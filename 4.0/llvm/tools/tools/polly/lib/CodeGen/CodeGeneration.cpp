//===------ CodeGeneration.cpp - Code generate the Scops. -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// The CodeGeneration pass takes a Scop created by ScopInfo and translates it
// back to LLVM-IR using Cloog.
//
// The Scop describes the high level memory behaviour of a control flow region.
// Transformation passes can update the schedule (execution order) of statements
// in the Scop. Cloog is used to generate an abstract syntax tree (clast) that
// reflects the updated execution order. This clast is used to create new
// LLVM-IR that is computational equivalent to the original control flow region,
// but executes its code in the new execution order defined by the changed
// scattering.
//
//===----------------------------------------------------------------------===//

#include "polly/CodeGen/Cloog.h"
#ifdef CLOOG_FOUND

#include "polly/Dependences.h"
#include "polly/LinkAllPasses.h"
#include "polly/Options.h"
#include "polly/ScopInfo.h"
#include "polly/TempScopInfo.h"
#include "polly/CodeGen/CodeGeneration.h"
#include "polly/CodeGen/BlockGenerators.h"
#include "polly/CodeGen/LoopGenerators.h"
#include "polly/CodeGen/PTXGenerator.h"
#include "polly/CodeGen/Utils.h"
#include "polly/Support/GICHelper.h"
#include "polly/Support/ScopHelper.h"

#include "llvm/IR/Module.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#define CLOOG_INT_GMP 1
#include "cloog/cloog.h"
#include "cloog/isl/cloog.h"

#include "isl/aff.h"

#include <vector>
#include <utility>

using namespace polly;
using namespace llvm;

#define DEBUG_TYPE "polly-codegen"

struct isl_set;

namespace polly {
static cl::opt<bool>
    OpenMP("enable-polly-openmp", cl::desc("Generate OpenMP parallel code"),
           cl::value_desc("OpenMP code generation enabled if true"),
           cl::init(false), cl::ZeroOrMore, cl::cat(PollyCategory));

#ifdef GPU_CODEGEN
static cl::opt<bool>
    GPGPU("enable-polly-gpgpu", cl::desc("Generate GPU parallel code"),
          cl::Hidden, cl::value_desc("GPGPU code generation enabled if true"),
          cl::init(false), cl::ZeroOrMore, cl::cat(PollyCategory));

static cl::opt<std::string>
    GPUTriple("polly-gpgpu-triple",
              cl::desc("Target triple for GPU code generation"), cl::Hidden,
              cl::init(""), cl::cat(PollyCategory));
#endif /* GPU_CODEGEN */

typedef DenseMap<const char *, Value *> CharMapT;

/// Class to generate LLVM-IR that calculates the value of a clast_expr.
class ClastExpCodeGen {
  PollyIRBuilder &Builder;
  const CharMapT &IVS;

  Value *codegen(const clast_name *e, Type *Ty);
  Value *codegen(const clast_term *e, Type *Ty);
  Value *codegen(const clast_binary *e, Type *Ty);
  Value *codegen(const clast_reduction *r, Type *Ty);

public:
  // A generator for clast expressions.
  //
  // @param B The IRBuilder that defines where the code to calculate the
  //          clast expressions should be inserted.
  // @param IVMAP A Map that translates strings describing the induction
  //              variables to the Values* that represent these variables
  //              on the LLVM side.
  ClastExpCodeGen(PollyIRBuilder &B, CharMapT &IVMap);

  // Generates code to calculate a given clast expression.
  //
  // @param e The expression to calculate.
  // @return The Value that holds the result.
  Value *codegen(const clast_expr *e, Type *Ty);
};

Value *ClastExpCodeGen::codegen(const clast_name *e, Type *Ty) {
  CharMapT::const_iterator I = IVS.find(e->name);

  assert(I != IVS.end() && "Clast name not found");

  return Builder.CreateSExtOrBitCast(I->second, Ty);
}

static APInt APInt_from_MPZ(const mpz_t mpz) {
  uint64_t *p = nullptr;
  size_t sz;

  p = (uint64_t *)mpz_export(p, &sz, -1, sizeof(uint64_t), 0, 0, mpz);

  if (p) {
    APInt A((unsigned)mpz_sizeinbase(mpz, 2), (unsigned)sz, p);
    A = A.zext(A.getBitWidth() + 1);
    free(p);

    if (mpz_sgn(mpz) == -1)
      return -A;
    else
      return A;
  } else {
    uint64_t val = 0;
    return APInt(1, 1, &val);
  }
}

Value *ClastExpCodeGen::codegen(const clast_term *e, Type *Ty) {
  APInt a = APInt_from_MPZ(e->val);

  Value *ConstOne = ConstantInt::get(Builder.getContext(), a);
  ConstOne = Builder.CreateSExtOrBitCast(ConstOne, Ty);

  if (!e->var)
    return ConstOne;

  Value *var = codegen(e->var, Ty);
  return Builder.CreateMul(ConstOne, var);
}

Value *ClastExpCodeGen::codegen(const clast_binary *e, Type *Ty) {
  Value *LHS = codegen(e->LHS, Ty);

  APInt RHS_AP = APInt_from_MPZ(e->RHS);

  Value *RHS = ConstantInt::get(Builder.getContext(), RHS_AP);
  RHS = Builder.CreateSExtOrBitCast(RHS, Ty);

  switch (e->type) {
  case clast_bin_mod:
    return Builder.CreateSRem(LHS, RHS);
  case clast_bin_fdiv: {
    // floord(n,d) ((n < 0) ? (n - d + 1) : n) / d
    Value *One = ConstantInt::get(Ty, 1);
    Value *Zero = ConstantInt::get(Ty, 0);
    Value *Sum1 = Builder.CreateSub(LHS, RHS);
    Value *Sum2 = Builder.CreateAdd(Sum1, One);
    Value *isNegative = Builder.CreateICmpSLT(LHS, Zero);
    Value *Dividend = Builder.CreateSelect(isNegative, Sum2, LHS);
    return Builder.CreateSDiv(Dividend, RHS);
  }
  case clast_bin_cdiv: {
    // ceild(n,d) ((n < 0) ? n : (n + d - 1)) / d
    Value *One = ConstantInt::get(Ty, 1);
    Value *Zero = ConstantInt::get(Ty, 0);
    Value *Sum1 = Builder.CreateAdd(LHS, RHS);
    Value *Sum2 = Builder.CreateSub(Sum1, One);
    Value *isNegative = Builder.CreateICmpSLT(LHS, Zero);
    Value *Dividend = Builder.CreateSelect(isNegative, LHS, Sum2);
    return Builder.CreateSDiv(Dividend, RHS);
  }
  case clast_bin_div:
    return Builder.CreateSDiv(LHS, RHS);
  }

  llvm_unreachable("Unknown clast binary expression type");
}

Value *ClastExpCodeGen::codegen(const clast_reduction *r, Type *Ty) {
  assert((r->type == clast_red_min || r->type == clast_red_max ||
          r->type == clast_red_sum) &&
         "Clast reduction type not supported");
  Value *old = codegen(r->elts[0], Ty);

  for (int i = 1; i < r->n; ++i) {
    Value *exprValue = codegen(r->elts[i], Ty);

    switch (r->type) {
    case clast_red_min: {
      Value *cmp = Builder.CreateICmpSLT(old, exprValue);
      old = Builder.CreateSelect(cmp, old, exprValue);
      break;
    }
    case clast_red_max: {
      Value *cmp = Builder.CreateICmpSGT(old, exprValue);
      old = Builder.CreateSelect(cmp, old, exprValue);
      break;
    }
    case clast_red_sum:
      old = Builder.CreateAdd(old, exprValue);
      break;
    }
  }

  return old;
}

ClastExpCodeGen::ClastExpCodeGen(PollyIRBuilder &B, CharMapT &IVMap)
    : Builder(B), IVS(IVMap) {}

Value *ClastExpCodeGen::codegen(const clast_expr *e, Type *Ty) {
  switch (e->type) {
  case clast_expr_name:
    return codegen((const clast_name *)e, Ty);
  case clast_expr_term:
    return codegen((const clast_term *)e, Ty);
  case clast_expr_bin:
    return codegen((const clast_binary *)e, Ty);
  case clast_expr_red:
    return codegen((const clast_reduction *)e, Ty);
  }

  llvm_unreachable("Unknown clast expression!");
}

class ClastStmtCodeGen {
public:
  const std::vector<std::string> &getParallelLoops();

private:
  // The Scop we code generate.
  Scop *S;
  Pass *P;

  // The Builder specifies the current location to code generate at.
  PollyIRBuilder &Builder;

  // Map the Values from the old code to their counterparts in the new code.
  ValueMapT ValueMap;

  // Map the loops from the old code to expressions function of the induction
  // variables in the new code.  For example, when the code generator produces
  // this AST:
  //
  //   for (int c1 = 0; c1 <= 1023; c1 += 1)
  //     for (int c2 = 0; c2 <= 1023; c2 += 1)
  //       Stmt(c2 + 3, c1);
  //
  // LoopToScev is a map associating:
  //   "outer loop in the old loop nest" -> SCEV("c2 + 3"),
  //   "inner loop in the old loop nest" -> SCEV("c1").
  LoopToScevMapT LoopToScev;

  // clastVars maps from the textual representation of a clast variable to its
  // current *Value. clast variables are scheduling variables, original
  // induction variables or parameters. They are used either in loop bounds or
  // to define the statement instance that is executed.
  //
  //   for (s = 0; s < n + 3; ++i)
  //     for (t = s; t < m; ++j)
  //       Stmt(i = s + 3 * m, j = t);
  //
  // {s,t,i,j,n,m} is the set of clast variables in this clast.
  CharMapT ClastVars;

  // Codegenerator for clast expressions.
  ClastExpCodeGen ExpGen;

  // Do we currently generate parallel code?
  bool parallelCodeGeneration;

  std::vector<std::string> parallelLoops;

  void codegen(const clast_assignment *a);

  void codegen(const clast_assignment *a, ScopStmt *Statement,
               unsigned Dimension, int vectorDim,
               std::vector<ValueMapT> *VectorVMap = 0,
               std::vector<LoopToScevMapT> *VLTS = 0);

  void codegenSubstitutions(const clast_stmt *Assignment, ScopStmt *Statement,
                            int vectorDim = 0,
                            std::vector<ValueMapT> *VectorVMap = 0,
                            std::vector<LoopToScevMapT> *VLTS = 0);

  void codegen(const clast_user_stmt *u, std::vector<Value *> *IVS = nullptr,
               const char *iterator = nullptr,
               __isl_take isl_set *scatteringDomain = 0);

  void codegen(const clast_block *b);

  /// @brief Create a classical sequential loop.
  void codegenForSequential(const clast_for *f);

  /// @brief Create OpenMP structure values.
  ///
  /// Create a list of values that has to be stored into the OpenMP subfuncition
  /// structure.
  SetVector<Value *> getOMPValues(const clast_stmt *Body);

  /// @brief Update ClastVars and ValueMap according to a value map.
  ///
  /// @param VMap A map from old to new values.
  void updateWithValueMap(OMPGenerator::ValueToValueMapTy &VMap);

  /// @brief Create an OpenMP parallel for loop.
  ///
  /// This loop reflects a loop as if it would have been created by an OpenMP
  /// statement.
  void codegenForOpenMP(const clast_for *f);

#ifdef GPU_CODEGEN
  /// @brief Create GPGPU device memory access values.
  ///
  /// Create a list of values that will be set to be parameters of the GPGPU
  /// subfunction. These parameters represent device memory base addresses
  /// and the size in bytes.
  SetVector<Value *> getGPUValues(unsigned &OutputBytes);

  /// @brief Create a GPU parallel for loop.
  ///
  /// This loop reflects a loop as if it would have been created by a GPU
  /// statement.
  void codegenForGPGPU(const clast_for *F);

  /// @brief Get innermost for loop.
  const clast_stmt *getScheduleInfo(const clast_for *F,
                                    std::vector<int> &NumIters,
                                    unsigned &LoopDepth,
                                    unsigned &NonPLoopDepth);
#endif /* GPU_CODEGEN */

  /// @brief Check if a loop is parallel
  ///
  /// Detect if a clast_for loop can be executed in parallel.
  ///
  /// @param For The clast for loop to check.
  ///
  /// @return bool Returns true if the incoming clast_for statement can
  ///              execute in parallel.
  bool isParallelFor(const clast_for *For);

  bool isInnermostLoop(const clast_for *f);

  /// @brief Get the number of loop iterations for this loop.
  /// @param f The clast for loop to check.
  int getNumberOfIterations(const clast_for *f);

  /// @brief Create vector instructions for this loop.
  void codegenForVector(const clast_for *f);

  void codegen(const clast_for *f);

  Value *codegen(const clast_equation *eq);

  void codegen(const clast_guard *g);

  void codegen(const clast_stmt *stmt);

  void addParameters(const CloogNames *names);

  IntegerType *getIntPtrTy();

public:
  void codegen(const clast_root *r);

  ClastStmtCodeGen(Scop *scop, PollyIRBuilder &B, Pass *P);
};
}

IntegerType *ClastStmtCodeGen::getIntPtrTy() {
  return P->getAnalysis<DataLayoutPass>().getDataLayout().getIntPtrType(
      Builder.getContext());
}

const std::vector<std::string> &ClastStmtCodeGen::getParallelLoops() {
  return parallelLoops;
}

void ClastStmtCodeGen::codegen(const clast_assignment *a) {
  Value *V = ExpGen.codegen(a->RHS, getIntPtrTy());
  ClastVars[a->LHS] = V;
}

void ClastStmtCodeGen::codegen(const clast_assignment *A, ScopStmt *Stmt,
                               unsigned Dim, int VectorDim,
                               std::vector<ValueMapT> *VectorVMap,
                               std::vector<LoopToScevMapT> *VLTS) {
  Value *RHS;

  assert(!A->LHS && "Statement assignments do not have left hand side");

  RHS = ExpGen.codegen(A->RHS, Builder.getInt64Ty());

  const llvm::SCEV *URHS = S->getSE()->getUnknown(RHS);
  if (VLTS)
    (*VLTS)[VectorDim][Stmt->getLoopForDimension(Dim)] = URHS;
  LoopToScev[Stmt->getLoopForDimension(Dim)] = URHS;

  const PHINode *PN = Stmt->getInductionVariableForDimension(Dim);
  if (PN) {
    RHS = Builder.CreateTruncOrBitCast(RHS, PN->getType());

    if (VectorVMap)
      (*VectorVMap)[VectorDim][PN] = RHS;

    ValueMap[PN] = RHS;
  }
}

void ClastStmtCodeGen::codegenSubstitutions(const clast_stmt *Assignment,
                                            ScopStmt *Statement, int vectorDim,
                                            std::vector<ValueMapT> *VectorVMap,
                                            std::vector<LoopToScevMapT> *VLTS) {
  int Dimension = 0;

  while (Assignment) {
    assert(CLAST_STMT_IS_A(Assignment, stmt_ass) &&
           "Substitions are expected to be assignments");
    codegen((const clast_assignment *)Assignment, Statement, Dimension,
            vectorDim, VectorVMap, VLTS);
    Assignment = Assignment->next;
    Dimension++;
  }
}

// Takes the cloog specific domain and translates it into a map Statement ->
// PartialSchedule, where the PartialSchedule contains all the dimensions that
// have been code generated up to this point.
static __isl_give isl_map *extractPartialSchedule(ScopStmt *Statement,
                                                  __isl_take isl_set *Domain) {
  isl_map *Schedule = Statement->getScattering();
  int ScheduledDimensions = isl_set_dim(Domain, isl_dim_set);
  int UnscheduledDimensions =
      isl_map_dim(Schedule, isl_dim_out) - ScheduledDimensions;

  isl_set_free(Domain);

  return isl_map_project_out(Schedule, isl_dim_out, ScheduledDimensions,
                             UnscheduledDimensions);
}

void ClastStmtCodeGen::codegen(const clast_user_stmt *u,
                               std::vector<Value *> *IVS, const char *iterator,
                               __isl_take isl_set *Domain) {
  ScopStmt *Statement = (ScopStmt *)u->statement->usr;

  if (u->substitutions)
    codegenSubstitutions(u->substitutions, Statement);

  int VectorDimensions = IVS ? IVS->size() : 1;

  if (VectorDimensions == 1) {
    BlockGenerator::generate(Builder, *Statement, ValueMap, LoopToScev, P);
    isl_set_free(Domain);
    return;
  }

  VectorValueMapT VectorMap(VectorDimensions);
  std::vector<LoopToScevMapT> VLTS(VectorDimensions);

  if (IVS) {
    assert(u->substitutions && "Substitutions expected!");
    int i = 0;
    for (std::vector<Value *>::iterator II = IVS->begin(), IE = IVS->end();
         II != IE; ++II) {
      ClastVars[iterator] = *II;
      codegenSubstitutions(u->substitutions, Statement, i, &VectorMap, &VLTS);
      i++;
    }
  }

  // Copy the current value map into all vector maps if the key wasn't
  // available yet. This is needed in case vector codegen is performed in
  // OpenMP subfunctions.
  for (auto KV : ValueMap)
    for (int i = 0; i < VectorDimensions; ++i)
      VectorMap[i].insert(KV);

  isl_map *Schedule = extractPartialSchedule(Statement, Domain);
  VectorBlockGenerator::generate(Builder, *Statement, VectorMap, VLTS, Schedule,
                                 P);
  isl_map_free(Schedule);
}

void ClastStmtCodeGen::codegen(const clast_block *b) {
  if (b->body)
    codegen(b->body);
}

void ClastStmtCodeGen::codegenForSequential(const clast_for *f) {
  Value *LowerBound, *UpperBound, *IV, *Stride;
  BasicBlock *ExitBlock;
  Type *IntPtrTy = getIntPtrTy();

  LowerBound = ExpGen.codegen(f->LB, IntPtrTy);
  UpperBound = ExpGen.codegen(f->UB, IntPtrTy);
  Stride = Builder.getInt(APInt_from_MPZ(f->stride));

  IV = createLoop(LowerBound, UpperBound, Stride, Builder, P, ExitBlock,
                  CmpInst::ICMP_SLE);

  // Add loop iv to symbols.
  ClastVars[f->iterator] = IV;

  if (f->body)
    codegen(f->body);

  // Loop is finished, so remove its iv from the live symbols.
  ClastVars.erase(f->iterator);
  Builder.SetInsertPoint(ExitBlock->begin());
}

// Helper class to determine all scalar parameters used in the basic blocks of a
// clast. Scalar parameters are scalar variables defined outside of the SCoP.
class ParameterVisitor : public ClastVisitor {
  std::set<Value *> Values;

public:
  ParameterVisitor() : ClastVisitor(), Values() {}

  void visitUser(const clast_user_stmt *Stmt) {
    const ScopStmt *S = static_cast<const ScopStmt *>(Stmt->statement->usr);
    const BasicBlock *BB = S->getBasicBlock();

    // Check all the operands of instructions in the basic block.
    for (const Instruction &Inst : *BB) {
      for (Value *SrcVal : Inst.operands()) {
        if (Instruction *OpInst = dyn_cast<Instruction>(SrcVal))
          if (S->getParent()->getRegion().contains(OpInst))
            continue;

        if (isa<Instruction>(SrcVal) || isa<Argument>(SrcVal))
          Values.insert(SrcVal);
      }
    }
  }

  // Iterator to iterate over the values found.
  typedef std::set<Value *>::const_iterator const_iterator;
  inline const_iterator begin() const { return Values.begin(); }
  inline const_iterator end() const { return Values.end(); }
};

SetVector<Value *> ClastStmtCodeGen::getOMPValues(const clast_stmt *Body) {
  SetVector<Value *> Values;

  // The clast variables
  for (CharMapT::iterator I = ClastVars.begin(), E = ClastVars.end(); I != E;
       I++)
    Values.insert(I->second);

  // Find the temporaries that are referenced in the clast statements'
  // basic blocks but are not defined by these blocks (e.g., references
  // to function arguments or temporaries defined before the start of
  // the SCoP).
  ParameterVisitor Params;
  Params.visit(Body);

  for (ParameterVisitor::const_iterator PI = Params.begin(), PE = Params.end();
       PI != PE; ++PI) {
    Value *V = *PI;
    Values.insert(V);
    DEBUG(dbgs() << "Adding temporary for OMP copy-in: " << *V << "\n");
  }

  return Values;
}

void
ClastStmtCodeGen::updateWithValueMap(OMPGenerator::ValueToValueMapTy &VMap) {
  std::set<Value *> Inserted;

  for (CharMapT::iterator I = ClastVars.begin(), E = ClastVars.end(); I != E;
       I++) {
    ClastVars[I->first] = VMap[I->second];
    Inserted.insert(I->second);
  }

  for (OMPGenerator::ValueToValueMapTy::iterator I = VMap.begin(),
                                                 E = VMap.end();
       I != E; ++I) {
    if (Inserted.count(I->first))
      continue;

    ValueMap[I->first] = I->second;
  }
}

static void clearDomtree(Function *F, DominatorTree &DT) {
  DomTreeNode *N = DT.getNode(&F->getEntryBlock());
  std::vector<BasicBlock *> Nodes;
  for (po_iterator<DomTreeNode *> I = po_begin(N), E = po_end(N); I != E; ++I)
    Nodes.push_back(I->getBlock());

  for (std::vector<BasicBlock *>::iterator I = Nodes.begin(), E = Nodes.end();
       I != E; ++I)
    DT.eraseNode(*I);
}

void ClastStmtCodeGen::codegenForOpenMP(const clast_for *For) {
  Value *Stride, *LB, *UB, *IV;
  BasicBlock::iterator LoopBody;
  IntegerType *IntPtrTy = getIntPtrTy();
  SetVector<Value *> Values;
  OMPGenerator::ValueToValueMapTy VMap;
  OMPGenerator OMPGen(Builder, P);

  Stride = Builder.getInt(APInt_from_MPZ(For->stride));
  Stride = Builder.CreateSExtOrBitCast(Stride, IntPtrTy);
  LB = ExpGen.codegen(For->LB, IntPtrTy);
  UB = ExpGen.codegen(For->UB, IntPtrTy);

  Values = getOMPValues(For->body);

  IV = OMPGen.createParallelLoop(LB, UB, Stride, Values, VMap, &LoopBody);
  BasicBlock::iterator AfterLoop = Builder.GetInsertPoint();
  Builder.SetInsertPoint(LoopBody);

  // Save the current values.
  const ValueMapT ValueMapCopy = ValueMap;
  const CharMapT ClastVarsCopy = ClastVars;

  updateWithValueMap(VMap);
  ClastVars[For->iterator] = IV;

  if (For->body)
    codegen(For->body);

  // Restore the original values.
  ValueMap = ValueMapCopy;
  ClastVars = ClastVarsCopy;

  clearDomtree((*LoopBody).getParent()->getParent(),
               P->getAnalysis<DominatorTreeWrapperPass>().getDomTree());

  Builder.SetInsertPoint(AfterLoop);
}

#ifdef GPU_CODEGEN
static unsigned getArraySizeInBytes(const ArrayType *AT) {
  unsigned Bytes = AT->getNumElements();
  if (const ArrayType *T = dyn_cast<ArrayType>(AT->getElementType()))
    Bytes *= getArraySizeInBytes(T);
  else
    Bytes *= AT->getElementType()->getPrimitiveSizeInBits() / 8;

  return Bytes;
}

SetVector<Value *> ClastStmtCodeGen::getGPUValues(unsigned &OutputBytes) {
  SetVector<Value *> Values;
  OutputBytes = 0;

  // Record the memory reference base addresses.
  for (ScopStmt *Stmt : *S) {
    for (MemoryAccess *MA : *Stmt) {
      Value *BaseAddr = const_cast<Value *>(MA->getBaseAddr());
      Values.insert((BaseAddr));

      // FIXME: we assume that there is one and only one array to be written
      // in a SCoP.
      int NumWrites = 0;
      if (MA->isWrite()) {
        ++NumWrites;
        assert(NumWrites <= 1 &&
               "We support at most one array to be written in a SCoP.");
        if (const PointerType *PT =
                dyn_cast<PointerType>(BaseAddr->getType())) {
          Type *T = PT->getArrayElementType();
          const ArrayType *ATy = dyn_cast<ArrayType>(T);
          OutputBytes = getArraySizeInBytes(ATy);
        }
      }
    }
  }

  return Values;
}

const clast_stmt *ClastStmtCodeGen::getScheduleInfo(const clast_for *F,
                                                    std::vector<int> &NumIters,
                                                    unsigned &LoopDepth,
                                                    unsigned &NonPLoopDepth) {
  clast_stmt *Stmt = (clast_stmt *)F;
  const clast_for *Result;
  bool NonParaFlag = false;
  LoopDepth = 0;
  NonPLoopDepth = 0;

  while (Stmt) {
    if (CLAST_STMT_IS_A(Stmt, stmt_for)) {
      const clast_for *T = (clast_for *)Stmt;
      if (isParallelFor(T)) {
        if (!NonParaFlag) {
          NumIters.push_back(getNumberOfIterations(T));
          Result = T;
        }
      } else
        NonParaFlag = true;

      Stmt = T->body;
      LoopDepth++;
      continue;
    }
    Stmt = Stmt->next;
  }

  assert(NumIters.size() == 4 &&
         "The loops should be tiled into 4-depth parallel loops and an "
         "innermost non-parallel one (if exist).");
  NonPLoopDepth = LoopDepth - NumIters.size();
  assert(NonPLoopDepth <= 1 &&
         "We support only one innermost non-parallel loop currently.");
  return (const clast_stmt *)Result->body;
}

void ClastStmtCodeGen::codegenForGPGPU(const clast_for *F) {
  BasicBlock::iterator LoopBody;
  SetVector<Value *> Values;
  SetVector<Value *> IVS;
  std::vector<int> NumIterations;
  PTXGenerator::ValueToValueMapTy VMap;

  assert(!GPUTriple.empty() &&
         "Target triple should be set properly for GPGPU code generation.");
  PTXGenerator PTXGen(Builder, P, GPUTriple);

  // Get original IVS and ScopStmt
  unsigned TiledLoopDepth, NonPLoopDepth;
  const clast_stmt *InnerStmt =
      getScheduleInfo(F, NumIterations, TiledLoopDepth, NonPLoopDepth);
  const clast_stmt *TmpStmt;
  const clast_user_stmt *U;
  const clast_for *InnerFor;
  if (CLAST_STMT_IS_A(InnerStmt, stmt_for)) {
    InnerFor = (const clast_for *)InnerStmt;
    TmpStmt = InnerFor->body;
  } else
    TmpStmt = InnerStmt;
  U = (const clast_user_stmt *)TmpStmt;
  ScopStmt *Statement = (ScopStmt *)U->statement->usr;
  for (unsigned i = 0; i < Statement->getNumIterators() - NonPLoopDepth; i++) {
    const Value *IV = Statement->getInductionVariableForDimension(i);
    IVS.insert(const_cast<Value *>(IV));
  }

  unsigned OutBytes;
  Values = getGPUValues(OutBytes);
  PTXGen.setOutputBytes(OutBytes);
  PTXGen.startGeneration(Values, IVS, VMap, &LoopBody);

  BasicBlock::iterator AfterLoop = Builder.GetInsertPoint();
  Builder.SetInsertPoint(LoopBody);

  BasicBlock *AfterBB = 0;
  if (NonPLoopDepth) {
    Value *LowerBound, *UpperBound, *IV, *Stride;
    Type *IntPtrTy = getIntPtrTy();
    LowerBound = ExpGen.codegen(InnerFor->LB, IntPtrTy);
    UpperBound = ExpGen.codegen(InnerFor->UB, IntPtrTy);
    Stride = Builder.getInt(APInt_from_MPZ(InnerFor->stride));
    IV = createLoop(LowerBound, UpperBound, Stride, Builder, P, AfterBB,
                    CmpInst::ICMP_SLE);
    const Value *OldIV_ = Statement->getInductionVariableForDimension(2);
    Value *OldIV = const_cast<Value *>(OldIV_);
    VMap.insert(std::make_pair(OldIV, IV));
  }

  updateWithValueMap(VMap);

  BlockGenerator::generate(Builder, *Statement, ValueMap, LoopToScev, P);

  if (AfterBB)
    Builder.SetInsertPoint(AfterBB->begin());

  // FIXME: The replacement of the host base address with the parameter of ptx
  // subfunction should have been done by updateWithValueMap. We use the
  // following codes to avoid affecting other parts of Polly. This should be
  // fixed later.
  Function *FN = Builder.GetInsertBlock()->getParent();
  for (unsigned j = 0; j < Values.size(); j++) {
    Value *baseAddr = Values[j];
    for (Function::iterator B = FN->begin(); B != FN->end(); ++B) {
      for (BasicBlock::iterator I = B->begin(); I != B->end(); ++I)
        I->replaceUsesOfWith(baseAddr, ValueMap[baseAddr]);
    }
  }
  Builder.SetInsertPoint(AfterLoop);
  PTXGen.setLaunchingParameters(NumIterations[0], NumIterations[1],
                                NumIterations[2], NumIterations[3]);
  PTXGen.finishGeneration(FN);
}
#endif

bool ClastStmtCodeGen::isInnermostLoop(const clast_for *f) {
  const clast_stmt *stmt = f->body;

  while (stmt) {
    if (!CLAST_STMT_IS_A(stmt, stmt_user))
      return false;

    stmt = stmt->next;
  }

  return true;
}

int ClastStmtCodeGen::getNumberOfIterations(const clast_for *For) {
  isl_set *LoopDomain = isl_set_copy(isl_set_from_cloog_domain(For->domain));
  int NumberOfIterations = polly::getNumberOfIterations(LoopDomain);
  if (NumberOfIterations == -1)
    return -1;
  return NumberOfIterations / mpz_get_si(For->stride) + 1;
}

void ClastStmtCodeGen::codegenForVector(const clast_for *F) {
  DEBUG(dbgs() << "Vectorizing loop '" << F->iterator << "'\n";);
  int VectorWidth = getNumberOfIterations(F);

  Value *LB = ExpGen.codegen(F->LB, getIntPtrTy());

  APInt Stride = APInt_from_MPZ(F->stride);
  IntegerType *LoopIVType = dyn_cast<IntegerType>(LB->getType());
  Stride = Stride.zext(LoopIVType->getBitWidth());
  Value *StrideValue = ConstantInt::get(LoopIVType, Stride);

  std::vector<Value *> IVS(VectorWidth);
  IVS[0] = LB;

  for (int i = 1; i < VectorWidth; i++)
    IVS[i] = Builder.CreateAdd(IVS[i - 1], StrideValue, "p_vector_iv");

  isl_set *Domain = isl_set_copy(isl_set_from_cloog_domain(F->domain));

  // Add loop iv to symbols.
  ClastVars[F->iterator] = LB;

  const clast_stmt *Stmt = F->body;

  while (Stmt) {
    codegen((const clast_user_stmt *)Stmt, &IVS, F->iterator,
            isl_set_copy(Domain));
    Stmt = Stmt->next;
  }

  // Loop is finished, so remove its iv from the live symbols.
  isl_set_free(Domain);
  ClastVars.erase(F->iterator);
}

bool ClastStmtCodeGen::isParallelFor(const clast_for *f) {
  isl_set *Domain = isl_set_copy(isl_set_from_cloog_domain(f->domain));
  assert(Domain && "Cannot access domain of loop");

  Dependences &D = P->getAnalysis<Dependences>();

  return D.isParallelDimension(Domain, isl_set_n_dim(Domain));
}

void ClastStmtCodeGen::codegen(const clast_for *f) {
  bool Vector = PollyVectorizerChoice != VECTORIZER_NONE;
  if ((Vector || OpenMP) && isParallelFor(f)) {
    if (Vector && isInnermostLoop(f) && (-1 != getNumberOfIterations(f)) &&
        (getNumberOfIterations(f) <= 16)) {
      codegenForVector(f);
      return;
    }

    if (OpenMP && !parallelCodeGeneration) {
      parallelCodeGeneration = true;
      parallelLoops.push_back(f->iterator);
      codegenForOpenMP(f);
      parallelCodeGeneration = false;
      return;
    }
  }

#ifdef GPU_CODEGEN
  if (GPGPU && isParallelFor(f)) {
    if (!parallelCodeGeneration) {
      parallelCodeGeneration = true;
      parallelLoops.push_back(f->iterator);
      codegenForGPGPU(f);
      parallelCodeGeneration = false;
      return;
    }
  }
#endif

  codegenForSequential(f);
}

Value *ClastStmtCodeGen::codegen(const clast_equation *eq) {
  Value *LHS = ExpGen.codegen(eq->LHS, getIntPtrTy());
  Value *RHS = ExpGen.codegen(eq->RHS, getIntPtrTy());
  CmpInst::Predicate P;

  if (eq->sign == 0)
    P = ICmpInst::ICMP_EQ;
  else if (eq->sign > 0)
    P = ICmpInst::ICMP_SGE;
  else
    P = ICmpInst::ICMP_SLE;

  return Builder.CreateICmp(P, LHS, RHS);
}

void ClastStmtCodeGen::codegen(const clast_guard *g) {
  Function *F = Builder.GetInsertBlock()->getParent();
  LLVMContext &Context = F->getContext();

  BasicBlock *CondBB =
      SplitBlock(Builder.GetInsertBlock(), Builder.GetInsertPoint(), P);
  CondBB->setName("polly.cond");
  BasicBlock *MergeBB = SplitBlock(CondBB, CondBB->begin(), P);
  MergeBB->setName("polly.merge");
  BasicBlock *ThenBB = BasicBlock::Create(Context, "polly.then", F);

  DominatorTree &DT = P->getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  DT.addNewBlock(ThenBB, CondBB);
  DT.changeImmediateDominator(MergeBB, CondBB);

  CondBB->getTerminator()->eraseFromParent();

  Builder.SetInsertPoint(CondBB);

  Value *Predicate = codegen(&(g->eq[0]));

  for (int i = 1; i < g->n; ++i) {
    Value *TmpPredicate = codegen(&(g->eq[i]));
    Predicate = Builder.CreateAnd(Predicate, TmpPredicate);
  }

  Builder.CreateCondBr(Predicate, ThenBB, MergeBB);
  Builder.SetInsertPoint(ThenBB);
  Builder.CreateBr(MergeBB);
  Builder.SetInsertPoint(ThenBB->begin());

  LoopInfo &LI = P->getAnalysis<LoopInfo>();
  Loop *L = LI.getLoopFor(CondBB);
  if (L)
    L->addBasicBlockToLoop(ThenBB, LI.getBase());

  codegen(g->then);

  Builder.SetInsertPoint(MergeBB->begin());
}

void ClastStmtCodeGen::codegen(const clast_stmt *stmt) {
  if (CLAST_STMT_IS_A(stmt, stmt_root))
    assert(false && "No second root statement expected");
  else if (CLAST_STMT_IS_A(stmt, stmt_ass))
    codegen((const clast_assignment *)stmt);
  else if (CLAST_STMT_IS_A(stmt, stmt_user))
    codegen((const clast_user_stmt *)stmt);
  else if (CLAST_STMT_IS_A(stmt, stmt_block))
    codegen((const clast_block *)stmt);
  else if (CLAST_STMT_IS_A(stmt, stmt_for))
    codegen((const clast_for *)stmt);
  else if (CLAST_STMT_IS_A(stmt, stmt_guard))
    codegen((const clast_guard *)stmt);

  if (stmt->next)
    codegen(stmt->next);
}

void ClastStmtCodeGen::addParameters(const CloogNames *names) {
  SCEVExpander Rewriter(P->getAnalysis<ScalarEvolution>(), "polly");

  int i = 0;
  for (Scop::param_iterator PI = S->param_begin(), PE = S->param_end();
       PI != PE; ++PI) {
    assert(i < names->nb_parameters && "Not enough parameter names");

    const SCEV *Param = *PI;
    Type *Ty = Param->getType();

    Instruction *insertLocation = --(Builder.GetInsertBlock()->end());
    Value *V = Rewriter.expandCodeFor(Param, Ty, insertLocation);
    ClastVars[names->parameters[i]] = V;

    ++i;
  }
}

void ClastStmtCodeGen::codegen(const clast_root *r) {
  addParameters(r->names);

  parallelCodeGeneration = false;

  const clast_stmt *stmt = (const clast_stmt *)r;
  if (stmt->next)
    codegen(stmt->next);
}

ClastStmtCodeGen::ClastStmtCodeGen(Scop *scop, PollyIRBuilder &B, Pass *P)
    : S(scop), P(P), Builder(B), ExpGen(Builder, ClastVars) {}

namespace {
class CodeGeneration : public ScopPass {
  std::vector<std::string> ParallelLoops;

public:
  static char ID;

  CodeGeneration() : ScopPass(ID) {}

  bool runOnScop(Scop &S) {
    ParallelLoops.clear();

    assert(!S.getRegion().isTopLevelRegion() &&
           "Top level regions are not supported");

    simplifyRegion(&S, this);

    BasicBlock *StartBlock = executeScopConditionally(S, this);

    PollyIRBuilder Builder(StartBlock->begin());

    ClastStmtCodeGen CodeGen(&S, Builder, this);
    CloogInfo &C = getAnalysis<CloogInfo>();
    CodeGen.codegen(C.getClast());

    ParallelLoops.insert(ParallelLoops.begin(),
                         CodeGen.getParallelLoops().begin(),
                         CodeGen.getParallelLoops().end());
    return true;
  }

  virtual void printScop(raw_ostream &OS) const {
    for (std::vector<std::string>::const_iterator PI = ParallelLoops.begin(),
                                                  PE = ParallelLoops.end();
         PI != PE; ++PI)
      OS << "Parallel loop with iterator '" << *PI << "' generated\n";
  }

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<CloogInfo>();
    AU.addRequired<Dependences>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<RegionInfoPass>();
    AU.addRequired<ScalarEvolution>();
    AU.addRequired<ScopDetection>();
    AU.addRequired<ScopInfo>();
    AU.addRequired<DataLayoutPass>();
    AU.addRequired<LoopInfo>();

    AU.addPreserved<CloogInfo>();
    AU.addPreserved<Dependences>();
    AU.addPreserved<LoopInfo>();
    AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addPreserved<ScopDetection>();
    AU.addPreserved<ScalarEvolution>();

    // FIXME: We do not yet add regions for the newly generated code to the
    //        region tree.
    AU.addPreserved<RegionInfoPass>();
    AU.addPreserved<TempScopInfo>();
    AU.addPreserved<ScopInfo>();
    AU.addPreservedID(IndependentBlocksID);
  }
};
}

char CodeGeneration::ID = 1;

Pass *polly::createCodeGenerationPass() { return new CodeGeneration(); }

INITIALIZE_PASS_BEGIN(CodeGeneration, "polly-codegen",
                      "Polly - Create LLVM-IR from SCoPs", false, false);
INITIALIZE_PASS_DEPENDENCY(CloogInfo);
INITIALIZE_PASS_DEPENDENCY(Dependences);
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass);
INITIALIZE_PASS_DEPENDENCY(RegionInfoPass);
INITIALIZE_PASS_DEPENDENCY(ScalarEvolution);
INITIALIZE_PASS_DEPENDENCY(ScopDetection);
INITIALIZE_PASS_DEPENDENCY(DataLayoutPass);
INITIALIZE_PASS_END(CodeGeneration, "polly-codegen",
                    "Polly - Create LLVM-IR from SCoPs", false, false)

#endif // CLOOG_FOUND
