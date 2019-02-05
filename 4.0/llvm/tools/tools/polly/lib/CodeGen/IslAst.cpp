//===- IslAst.cpp - isl code generator interface --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// The isl code generator interface takes a Scop and generates a isl_ast. This
// ist_ast can either be returned directly or it can be pretty printed to
// stdout.
//
// A typical isl_ast output looks like this:
//
// for (c2 = max(0, ceild(n + m, 2); c2 <= min(511, floord(5 * n, 3)); c2++) {
//   bb2(c2);
// }
//
//===----------------------------------------------------------------------===//

#include "polly/CodeGen/CodeGeneration.h"
#include "polly/CodeGen/IslAst.h"
#include "polly/Dependences.h"
#include "polly/LinkAllPasses.h"
#include "polly/Options.h"
#include "polly/ScopInfo.h"
#include "llvm/Support/Debug.h"

#include "isl/union_map.h"
#include "isl/list.h"
#include "isl/ast_build.h"
#include "isl/set.h"
#include "isl/map.h"
#include "isl/aff.h"

using namespace llvm;
using namespace polly;

#define DEBUG_TYPE "polly-ast"

static cl::opt<bool> UseContext("polly-ast-use-context",
                                cl::desc("Use context"), cl::Hidden,
                                cl::init(false), cl::ZeroOrMore,
                                cl::cat(PollyCategory));

static cl::opt<bool> DetectParallel("polly-ast-detect-parallel",
                                    cl::desc("Detect parallelism"), cl::Hidden,
                                    cl::init(false), cl::ZeroOrMore,
                                    cl::cat(PollyCategory));

namespace polly {
class IslAst {
public:
  IslAst(Scop *Scop, Dependences &D);

  ~IslAst();

  /// Print a source code representation of the program.
  void pprint(llvm::raw_ostream &OS);

  __isl_give isl_ast_node *getAst();

  /// @brief Get the run-time conditions for the Scop.
  __isl_give isl_ast_expr *getRunCondition();

private:
  Scop *S;
  isl_ast_node *Root;
  isl_ast_expr *RunCondition;

  void buildRunCondition(__isl_keep isl_ast_build *Context);
};
} // End namespace polly.

// Temporary information used when building the ast.
struct AstBuildUserInfo {
  // The dependence information.
  Dependences *Deps;

  // We are inside a parallel for node.
  int InParallelFor;
};

// Print a loop annotated with OpenMP or vector pragmas.
static __isl_give isl_printer *
printParallelFor(__isl_keep isl_ast_node *Node, __isl_take isl_printer *Printer,
                 __isl_take isl_ast_print_options *PrintOptions,
                 IslAstUserPayload *Info) {
  if (Info) {
    if (Info->IsInnermostParallel) {
      Printer = isl_printer_start_line(Printer);
      Printer = isl_printer_print_str(Printer, "#pragma simd");
      if (Info->IsReductionParallel)
        Printer = isl_printer_print_str(Printer, " reduction");
      Printer = isl_printer_end_line(Printer);
    }
    if (Info->IsOutermostParallel) {
      Printer = isl_printer_start_line(Printer);
      Printer = isl_printer_print_str(Printer, "#pragma omp parallel for");
      if (Info->IsReductionParallel)
        Printer = isl_printer_print_str(Printer, " reduction");
      Printer = isl_printer_end_line(Printer);
    }
  }
  return isl_ast_node_for_print(Node, Printer, PrintOptions);
}

// Print an isl_ast_for.
static __isl_give isl_printer *
printFor(__isl_take isl_printer *Printer,
         __isl_take isl_ast_print_options *PrintOptions,
         __isl_keep isl_ast_node *Node, void *User) {
  isl_id *Id = isl_ast_node_get_annotation(Node);
  if (!Id)
    return isl_ast_node_for_print(Node, Printer, PrintOptions);

  struct IslAstUserPayload *Info =
      (struct IslAstUserPayload *)isl_id_get_user(Id);
  Printer = printParallelFor(Node, Printer, PrintOptions, Info);
  isl_id_free(Id);
  return Printer;
}

// Allocate an AstNodeInfo structure and initialize it with default values.
static struct IslAstUserPayload *allocateIslAstUser() {
  struct IslAstUserPayload *NodeInfo;
  NodeInfo =
      (struct IslAstUserPayload *)malloc(sizeof(struct IslAstUserPayload));
  NodeInfo->Context = 0;
  NodeInfo->IsOutermostParallel = 0;
  NodeInfo->IsInnermostParallel = 0;
  NodeInfo->IsReductionParallel = false;
  return NodeInfo;
}

// Free the AstNodeInfo structure.
static void freeIslAstUser(void *Ptr) {
  struct IslAstUserPayload *UserStruct = (struct IslAstUserPayload *)Ptr;
  isl_ast_build_free(UserStruct->Context);
  free(UserStruct);
}

// Check if the current scheduling dimension is parallel.
//
// We check for parallelism by verifying that the loop does not carry any
// dependences.
//
// Parallelism test: if the distance is zero in all outer dimensions, then it
// has to be zero in the current dimension as well.
//
// Implementation: first, translate dependences into time space, then force
// outer dimensions to be equal. If the distance is zero in the current
// dimension, then the loop is parallel. The distance is zero in the current
// dimension if it is a subset of a map with equal values for the current
// dimension.
static bool astScheduleDimIsParallel(__isl_keep isl_ast_build *Build,
                                     __isl_take isl_union_map *Deps) {
  isl_union_map *Schedule;
  isl_map *ScheduleDeps, *Test;
  isl_space *ScheduleSpace;
  unsigned Dimension, IsParallel;

  Schedule = isl_ast_build_get_schedule(Build);
  ScheduleSpace = isl_ast_build_get_schedule_space(Build);

  Dimension = isl_space_dim(ScheduleSpace, isl_dim_out) - 1;

  Deps = isl_union_map_apply_range(Deps, isl_union_map_copy(Schedule));
  Deps = isl_union_map_apply_domain(Deps, Schedule);

  if (isl_union_map_is_empty(Deps)) {
    isl_union_map_free(Deps);
    isl_space_free(ScheduleSpace);
    return true;
  }

  ScheduleDeps = isl_map_from_union_map(Deps);

  for (unsigned i = 0; i < Dimension; i++)
    ScheduleDeps = isl_map_equate(ScheduleDeps, isl_dim_out, i, isl_dim_in, i);

  Test = isl_map_universe(isl_map_get_space(ScheduleDeps));
  Test = isl_map_equate(Test, isl_dim_out, Dimension, isl_dim_in, Dimension);
  IsParallel = isl_map_is_subset(ScheduleDeps, Test);

  isl_space_free(ScheduleSpace);
  isl_map_free(Test);
  isl_map_free(ScheduleDeps);

  return IsParallel;
}

/// @brief Check if the current scheduling dimension is parallel
///
/// In case the dimension is parallel we also check if any reduction
/// dependences is broken when we exploit this parallelism. If so,
/// @p IsReductionParallel will be set to true. The reduction dependences we use
/// to check are actually the union of the transitive closure of the initial
/// reduction dependences together with their reveresal. Even though these
/// dependences connect all iterations with each other (thus they are cyclic)
/// we can perform the parallelism check as we are only interested in a zero
/// (or non-zero) dependence distance on the dimension in question.
static bool astScheduleDimIsParallel(__isl_keep isl_ast_build *Build,
                                     Dependences *D,
                                     bool &IsReductionParallel) {
  if (!D->hasValidDependences())
    return false;

  isl_union_map *Deps = D->getDependences(
      Dependences::TYPE_RAW | Dependences::TYPE_WAW | Dependences::TYPE_WAR);
  if (!astScheduleDimIsParallel(Build, Deps))
    return false;

  isl_union_map *RedDeps = D->getDependences(Dependences::TYPE_TC_RED);
  if (!astScheduleDimIsParallel(Build, RedDeps))
    IsReductionParallel = true;

  return true;
}

// Mark a for node openmp parallel, if it is the outermost parallel for node.
static void markOpenmpParallel(__isl_keep isl_ast_build *Build,
                               struct AstBuildUserInfo *BuildInfo,
                               struct IslAstUserPayload *NodeInfo) {
  if (BuildInfo->InParallelFor)
    return;

  if (astScheduleDimIsParallel(Build, BuildInfo->Deps,
                               NodeInfo->IsReductionParallel)) {
    BuildInfo->InParallelFor = 1;
    NodeInfo->IsOutermostParallel = 1;
  }
}

// This method is executed before the construction of a for node. It creates
// an isl_id that is used to annotate the subsequently generated ast for nodes.
//
// In this function we also run the following analyses:
//
// - Detection of openmp parallel loops
//
static __isl_give isl_id *astBuildBeforeFor(__isl_keep isl_ast_build *Build,
                                            void *User) {
  struct AstBuildUserInfo *BuildInfo = (struct AstBuildUserInfo *)User;
  struct IslAstUserPayload *NodeInfo = allocateIslAstUser();
  isl_id *Id = isl_id_alloc(isl_ast_build_get_ctx(Build), "", NodeInfo);
  Id = isl_id_set_free_user(Id, freeIslAstUser);

  markOpenmpParallel(Build, BuildInfo, NodeInfo);

  return Id;
}

// Returns 0 when Node contains loops, otherwise returns -1. This search
// function uses ISL's way to iterate over lists of isl_ast_nodes with
// isl_ast_node_list_foreach. Please use the single argument wrapper function
// that returns a bool instead of using this function directly.
static int containsLoops(__isl_take isl_ast_node *Node, void *User) {
  if (!Node)
    return -1;

  switch (isl_ast_node_get_type(Node)) {
  case isl_ast_node_for:
    isl_ast_node_free(Node);
    return 0;
  case isl_ast_node_block: {
    isl_ast_node_list *List = isl_ast_node_block_get_children(Node);
    int Res = isl_ast_node_list_foreach(List, &containsLoops, nullptr);
    isl_ast_node_list_free(List);
    isl_ast_node_free(Node);
    return Res;
  }
  case isl_ast_node_if: {
    int Res = -1;
    if (0 == containsLoops(isl_ast_node_if_get_then(Node), nullptr) ||
        (isl_ast_node_if_has_else(Node) &&
         0 == containsLoops(isl_ast_node_if_get_else(Node), nullptr)))
      Res = 0;
    isl_ast_node_free(Node);
    return Res;
  }
  case isl_ast_node_user:
  default:
    isl_ast_node_free(Node);
    return -1;
  }
}

// Returns true when Node contains loops.
static bool containsLoops(__isl_take isl_ast_node *Node) {
  return 0 == containsLoops(Node, nullptr);
}

// This method is executed after the construction of a for node.
//
// It performs the following actions:
//
// - Reset the 'InParallelFor' flag, as soon as we leave a for node,
//   that is marked as openmp parallel.
//
static __isl_give isl_ast_node *
astBuildAfterFor(__isl_take isl_ast_node *Node, __isl_keep isl_ast_build *Build,
                 void *User) {
  isl_id *Id = isl_ast_node_get_annotation(Node);
  if (!Id)
    return Node;
  struct IslAstUserPayload *Info =
      (struct IslAstUserPayload *)isl_id_get_user(Id);
  struct AstBuildUserInfo *BuildInfo = (struct AstBuildUserInfo *)User;

  if (Info) {
    if (Info->IsOutermostParallel)
      BuildInfo->InParallelFor = 0;
    if (!containsLoops(isl_ast_node_for_get_body(Node)))
      if (astScheduleDimIsParallel(Build, BuildInfo->Deps,
                                   Info->IsReductionParallel))
        Info->IsInnermostParallel = 1;
    if (!Info->Context)
      Info->Context = isl_ast_build_copy(Build);
  }

  isl_id_free(Id);
  return Node;
}

static __isl_give isl_ast_node *AtEachDomain(__isl_take isl_ast_node *Node,
                                             __isl_keep isl_ast_build *Context,
                                             void *User) {
  struct IslAstUserPayload *Info = nullptr;
  isl_id *Id = isl_ast_node_get_annotation(Node);

  if (Id)
    Info = (struct IslAstUserPayload *)isl_id_get_user(Id);

  if (!Info) {
    // Allocate annotations once: parallel for detection might have already
    // allocated the annotations for this node.
    Info = allocateIslAstUser();
    Id = isl_id_alloc(isl_ast_node_get_ctx(Node), nullptr, Info);
    Id = isl_id_set_free_user(Id, &freeIslAstUser);
  }

  if (!Info->Context)
    Info->Context = isl_ast_build_copy(Context);

  return isl_ast_node_set_annotation(Node, Id);
}

void IslAst::buildRunCondition(__isl_keep isl_ast_build *Context) {
  // The conditions that need to be checked at run-time for this scop are
  // available as an isl_set in the AssumedContext. We generate code for this
  // check as follows. First, we generate an isl_pw_aff that is 1, if a certain
  // combination of parameter values fulfills the conditions in the assumed
  // context, and that is 0 otherwise. We then translate this isl_pw_aff into
  // an isl_ast_expr. At run-time this expression can be evaluated and the
  // optimized scop can be executed conditionally according to the result of the
  // run-time check.

  isl_aff *Zero =
      isl_aff_zero_on_domain(isl_local_space_from_space(S->getParamSpace()));
  isl_aff *One =
      isl_aff_zero_on_domain(isl_local_space_from_space(S->getParamSpace()));

  One = isl_aff_add_constant_si(One, 1);

  isl_pw_aff *PwZero = isl_pw_aff_from_aff(Zero);
  isl_pw_aff *PwOne = isl_pw_aff_from_aff(One);

  PwOne = isl_pw_aff_intersect_domain(PwOne, S->getAssumedContext());
  PwZero = isl_pw_aff_intersect_domain(
      PwZero, isl_set_complement(S->getAssumedContext()));

  isl_pw_aff *Cond = isl_pw_aff_union_max(PwOne, PwZero);

  RunCondition = isl_ast_build_expr_from_pw_aff(Context, Cond);
}

IslAst::IslAst(Scop *Scop, Dependences &D) : S(Scop) {
  isl_ctx *Ctx = S->getIslCtx();
  isl_options_set_ast_build_atomic_upper_bound(Ctx, true);
  isl_ast_build *Context;
  struct AstBuildUserInfo BuildInfo;

  if (UseContext)
    Context = isl_ast_build_from_context(S->getContext());
  else
    Context = isl_ast_build_from_context(isl_set_universe(S->getParamSpace()));

  Context = isl_ast_build_set_at_each_domain(Context, AtEachDomain, nullptr);

  isl_union_map *Schedule =
      isl_union_map_intersect_domain(S->getSchedule(), S->getDomains());

  Function *F = Scop->getRegion().getEntry()->getParent();
  (void)F;

  DEBUG(dbgs() << ":: isl ast :: " << F->getName()
               << " :: " << Scop->getRegion().getNameStr() << "\n");

  DEBUG(dbgs() << S->getContextStr() << "\n"; isl_union_map_dump(Schedule));

  if (DetectParallel || PollyVectorizerChoice != VECTORIZER_NONE) {
    BuildInfo.Deps = &D;
    BuildInfo.InParallelFor = 0;

    Context = isl_ast_build_set_before_each_for(Context, &astBuildBeforeFor,
                                                &BuildInfo);
    Context = isl_ast_build_set_after_each_for(Context, &astBuildAfterFor,
                                               &BuildInfo);
  }

  buildRunCondition(Context);

  Root = isl_ast_build_ast_from_schedule(Context, Schedule);

  isl_ast_build_free(Context);

  DEBUG(pprint(dbgs()));
}

IslAst::~IslAst() {
  isl_ast_node_free(Root);
  isl_ast_expr_free(RunCondition);
}

/// Print a C like representation of the program.
void IslAst::pprint(llvm::raw_ostream &OS) {
  isl_ast_node *Root;
  isl_ast_print_options *Options;

  Options = isl_ast_print_options_alloc(S->getIslCtx());
  Options = isl_ast_print_options_set_print_for(Options, &printFor, nullptr);

  isl_printer *P = isl_printer_to_str(S->getIslCtx());
  P = isl_printer_set_output_format(P, ISL_FORMAT_C);

  P = isl_printer_print_ast_expr(P, RunCondition);
  char *result = isl_printer_get_str(P);
  P = isl_printer_flush(P);

  OS << "\nif (" << result << ")\n\n";
  P = isl_printer_indent(P, 4);

  Root = getAst();
  P = isl_ast_node_print(Root, P, Options);
  result = isl_printer_get_str(P);
  OS << result << "\n";
  OS << "else\n";
  OS << "    {  /* original code */ }\n\n";
  isl_printer_free(P);
  isl_ast_node_free(Root);
}

__isl_give isl_ast_node *IslAst::getAst() { return isl_ast_node_copy(Root); }
__isl_give isl_ast_expr *IslAst::getRunCondition() {
  return isl_ast_expr_copy(RunCondition);
}

void IslAstInfo::pprint(llvm::raw_ostream &OS) { Ast->pprint(OS); }

void IslAstInfo::releaseMemory() {
  if (Ast) {
    delete Ast;
    Ast = 0;
  }
}

bool IslAstInfo::runOnScop(Scop &Scop) {
  if (Ast)
    delete Ast;

  S = &Scop;

  Dependences &D = getAnalysis<Dependences>();

  Ast = new IslAst(&Scop, D);

  return false;
}

__isl_give isl_ast_node *IslAstInfo::getAst() { return Ast->getAst(); }
__isl_give isl_ast_expr *IslAstInfo::getRunCondition() {
  return Ast->getRunCondition();
}

void IslAstInfo::printScop(raw_ostream &OS) const {
  Function *F = S->getRegion().getEntry()->getParent();

  OS << F->getName() << "():\n";

  Ast->pprint(OS);
}

IslAstUserPayload *IslAstInfo::getNodePayload(__isl_keep isl_ast_node *Node) {
  isl_id *Id = isl_ast_node_get_annotation(Node);
  if (!Id)
    return nullptr;
  IslAstUserPayload *Payload = (IslAstUserPayload *)isl_id_get_user(Id);
  isl_id_free(Id);
  return Payload;
}

bool IslAstInfo::isParallel(__isl_keep isl_ast_node *Node) {
  return (isInnermostParallel(Node) || isOuterParallel(Node)) &&
         !isReductionParallel(Node);
}

bool IslAstInfo::isInnermostParallel(__isl_keep isl_ast_node *Node) {
  IslAstUserPayload *Payload = getNodePayload(Node);
  return Payload && Payload->IsInnermostParallel &&
         !Payload->IsReductionParallel;
}

bool IslAstInfo::isOuterParallel(__isl_keep isl_ast_node *Node) {
  IslAstUserPayload *Payload = getNodePayload(Node);
  return Payload && Payload->IsOutermostParallel &&
         !Payload->IsReductionParallel;
}

bool IslAstInfo::isReductionParallel(__isl_keep isl_ast_node *Node) {
  IslAstUserPayload *Payload = getNodePayload(Node);
  return Payload && Payload->IsReductionParallel;
}

void IslAstInfo::getAnalysisUsage(AnalysisUsage &AU) const {
  // Get the Common analysis usage of ScopPasses.
  ScopPass::getAnalysisUsage(AU);
  AU.addRequired<ScopInfo>();
  AU.addRequired<Dependences>();
}

char IslAstInfo::ID = 0;

Pass *polly::createIslAstInfoPass() { return new IslAstInfo(); }

INITIALIZE_PASS_BEGIN(IslAstInfo, "polly-ast",
                      "Polly - Generate an AST of the SCoP (isl)", false,
                      false);
INITIALIZE_PASS_DEPENDENCY(ScopInfo);
INITIALIZE_PASS_DEPENDENCY(Dependences);
INITIALIZE_PASS_END(IslAstInfo, "polly-ast",
                    "Polly - Generate an AST from the SCoP (isl)", false, false)
