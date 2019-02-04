//===- Dependency.cpp - Calculate dependency information for a Scop.  -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Calculate the data dependency relations for a Scop using ISL.
//
// The integer set library (ISL) from Sven, has a integrated dependency analysis
// to calculate data dependences. This pass takes advantage of this and
// calculate those dependences a Scop.
//
// The dependences in this pass are exact in terms that for a specific read
// statement instance only the last write statement instance is returned. In
// case of may writes a set of possible write instances is returned. This
// analysis will never produce redundant dependences.
//
//===----------------------------------------------------------------------===//
//
#include "polly/Dependences.h"
#include "polly/LinkAllPasses.h"
#include "polly/Options.h"
#include "polly/ScopInfo.h"
#include "polly/Support/GICHelper.h"
#include "llvm/Support/Debug.h"

#include <isl/aff.h>
#include <isl/ctx.h>
#include <isl/flow.h>
#include <isl/map.h>
#include <isl/options.h>
#include <isl/set.h>

using namespace polly;
using namespace llvm;

#define DEBUG_TYPE "polly-dependence"

static cl::opt<int> OptComputeOut(
    "polly-dependences-computeout",
    cl::desc("Bound the dependence analysis by a maximal amount of "
             "computational steps"),
    cl::Hidden, cl::init(250000), cl::ZeroOrMore, cl::cat(PollyCategory));

static cl::opt<bool> LegalityCheckDisabled(
    "disable-polly-legality", cl::desc("Disable polly legality check"),
    cl::Hidden, cl::init(false), cl::ZeroOrMore, cl::cat(PollyCategory));

enum AnalysisType { VALUE_BASED_ANALYSIS, MEMORY_BASED_ANALYSIS };

static cl::opt<enum AnalysisType> OptAnalysisType(
    "polly-dependences-analysis-type",
    cl::desc("The kind of dependence analysis to use"),
    cl::values(clEnumValN(VALUE_BASED_ANALYSIS, "value-based",
                          "Exact dependences without transitive dependences"),
               clEnumValN(MEMORY_BASED_ANALYSIS, "memory-based",
                          "Overapproximation of dependences"),
               clEnumValEnd),
    cl::Hidden, cl::init(VALUE_BASED_ANALYSIS), cl::ZeroOrMore,
    cl::cat(PollyCategory));

//===----------------------------------------------------------------------===//
Dependences::Dependences() : ScopPass(ID) { RAW = WAR = WAW = nullptr; }

void Dependences::collectInfo(Scop &S, isl_union_map **Read,
                              isl_union_map **Write, isl_union_map **MayWrite,
                              isl_union_map **AccessSchedule,
                              isl_union_map **StmtSchedule) {
  isl_space *Space = S.getParamSpace();
  *Read = isl_union_map_empty(isl_space_copy(Space));
  *Write = isl_union_map_empty(isl_space_copy(Space));
  *MayWrite = isl_union_map_empty(isl_space_copy(Space));
  *AccessSchedule = isl_union_map_empty(isl_space_copy(Space));
  *StmtSchedule = isl_union_map_empty(Space);

  SmallPtrSet<const Value *, 8> ReductionBaseValues;
  for (ScopStmt *Stmt : S)
    for (MemoryAccess *MA : *Stmt)
      if (MA->isReductionLike())
        ReductionBaseValues.insert(MA->getBaseAddr());

  for (ScopStmt *Stmt : S) {
    for (MemoryAccess *MA : *Stmt) {
      isl_set *domcp = Stmt->getDomain();
      isl_map *accdom = MA->getAccessRelation();

      accdom = isl_map_intersect_domain(accdom, domcp);

      if (ReductionBaseValues.count(MA->getBaseAddr())) {
        // Wrap the access domain and adjust the scattering accordingly.
        //
        // An access domain like
        //   Stmt[i0, i1] -> MemAcc_A[i0 + i1]
        // will be transformed into
        //   [Stmt[i0, i1] -> MemAcc_A[i0 + i1]] -> MemAcc_A[i0 + i1]
        //
        // The original scattering looks like
        //   Stmt[i0, i1] -> [0, i0, 2, i1, 0]
        // but as we transformed the access domain we need the scattering
        // to match the new access domains, thus we need
        //   [Stmt[i0, i1] -> MemAcc_A[i0 + i1]] -> [0, i0, 2, i1, 0]
        accdom = isl_map_range_map(accdom);

        isl_map *stmt_scatter = Stmt->getScattering();
        isl_set *scatter_dom = isl_map_domain(isl_map_copy(accdom));
        isl_set *scatter_ran = isl_map_range(stmt_scatter);
        isl_map *scatter =
            isl_map_from_domain_and_range(scatter_dom, scatter_ran);
        for (unsigned u = 0, e = Stmt->getNumIterators(); u != e; u++)
          scatter =
              isl_map_equate(scatter, isl_dim_out, 2 * u + 1, isl_dim_in, u);
        *AccessSchedule = isl_union_map_add_map(*AccessSchedule, scatter);
      }

      if (MA->isRead())
        *Read = isl_union_map_add_map(*Read, accdom);
      else
        *Write = isl_union_map_add_map(*Write, accdom);
    }
    *StmtSchedule = isl_union_map_add_map(*StmtSchedule, Stmt->getScattering());
  }
}

/// @brief Fix all dimension of @p Zero to 0 and add it to @p user
static int fixSetToZero(__isl_take isl_set *Zero, void *user) {
  isl_union_set **User = (isl_union_set **)user;
  for (unsigned i = 0; i < isl_set_dim(Zero, isl_dim_set); i++)
    Zero = isl_set_fix_si(Zero, isl_dim_set, i, 0);
  *User = isl_union_set_add_set(*User, Zero);
  return 0;
}

/// @brief Compute the privatization dependences for a given dependency @p Map
///
/// Privatization dependences are widened original dependences which originate
/// or end in a reduction access. To compute them we apply the transitive close
/// of the reduction dependences (which maps each iteration of a reduction
/// statement to all following ones) on the RAW/WAR/WAW dependences. The
/// dependences which start or end at a reduction statement will be extended to
/// depend on all following reduction statement iterations as well.
/// Note: "Following" here means according to the reduction dependences.
///
/// For the input:
///
///  S0:   *sum = 0;
///        for (int i = 0; i < 1024; i++)
///  S1:     *sum += i;
///  S2:   *sum = *sum * 3;
///
/// we have the following dependences before we add privatization dependences:
///
///   RAW:
///     { S0[] -> S1[0]; S1[1023] -> S2[] }
///   WAR:
///     {  }
///   WAW:
///     { S0[] -> S1[0]; S1[1024] -> S2[] }
///   RED:
///     { S1[i0] -> S1[1 + i0] : i0 >= 0 and i0 <= 1022 }
///
/// and afterwards:
///
///   RAW:
///     { S0[] -> S1[i0] : i0 >= 0 and i0 <= 1023;
///       S1[i0] -> S2[] : i0 >= 0 and i0 <= 1023}
///   WAR:
///     {  }
///   WAW:
///     { S0[] -> S1[i0] : i0 >= 0 and i0 <= 1023;
///       S1[i0] -> S2[] : i0 >= 0 and i0 <= 1023}
///   RED:
///     { S1[i0] -> S1[1 + i0] : i0 >= 0 and i0 <= 1022 }
///
/// Note: This function also computes the (reverse) transitive closure of the
///       reduction dependences.
void Dependences::addPrivatizationDependences() {
  isl_union_map *PrivRAW, *PrivWAW, *PrivWAR;

  // The transitive closure might be over approximated, thus could lead to
  // dependency cycles in the privatization dependences. To make sure this
  // will not happen we remove all negative dependences after we computed
  // the transitive closure.
  TC_RED = isl_union_map_transitive_closure(isl_union_map_copy(RED), 0);

  // FIXME: Apply the current schedule instead of assuming the identity schedule
  //        here. The current approach is only valid as long as we compute the
  //        dependences only with the initial (identity schedule). Any other
  //        schedule could change "the direction of the backward depenendes" we
  //        want to eliminate here.
  isl_union_set *UDeltas = isl_union_map_deltas(isl_union_map_copy(TC_RED));
  isl_union_set *Universe = isl_union_set_universe(isl_union_set_copy(UDeltas));
  isl_union_set *Zero = isl_union_set_empty(isl_union_set_get_space(Universe));
  isl_union_set_foreach_set(Universe, fixSetToZero, &Zero);
  isl_union_map *NonPositive = isl_union_set_lex_le_union_set(UDeltas, Zero);

  TC_RED = isl_union_map_subtract(TC_RED, NonPositive);

  TC_RED = isl_union_map_union(
      TC_RED, isl_union_map_reverse(isl_union_map_copy(TC_RED)));
  TC_RED = isl_union_map_coalesce(TC_RED);

  isl_union_map **Maps[] = {&RAW, &WAW, &WAR};
  isl_union_map **PrivMaps[] = {&PrivRAW, &PrivWAW, &PrivWAR};
  for (unsigned u = 0; u < 3; u++) {
    isl_union_map **Map = Maps[u], **PrivMap = PrivMaps[u];

    *PrivMap = isl_union_map_apply_range(isl_union_map_copy(*Map),
                                         isl_union_map_copy(TC_RED));
    *PrivMap = isl_union_map_union(
        *PrivMap, isl_union_map_apply_range(isl_union_map_copy(TC_RED),
                                            isl_union_map_copy(*Map)));

    *Map = isl_union_map_union(*Map, *PrivMap);
  }

  isl_union_set_free(Universe);
}

void Dependences::calculateDependences(Scop &S) {
  isl_union_map *Read, *Write, *MayWrite, *AccessSchedule, *StmtSchedule,
      *Schedule;

  DEBUG(dbgs() << "Scop: \n" << S << "\n");

  collectInfo(S, &Read, &Write, &MayWrite, &AccessSchedule, &StmtSchedule);

  Schedule =
      isl_union_map_union(AccessSchedule, isl_union_map_copy(StmtSchedule));

  Read = isl_union_map_coalesce(Read);
  Write = isl_union_map_coalesce(Write);
  MayWrite = isl_union_map_coalesce(MayWrite);

  long MaxOpsOld = isl_ctx_get_max_operations(S.getIslCtx());
  isl_ctx_set_max_operations(S.getIslCtx(), OptComputeOut);
  isl_options_set_on_error(S.getIslCtx(), ISL_ON_ERROR_CONTINUE);

  DEBUG(dbgs() << "Read: " << Read << "\n";
        dbgs() << "Write: " << Write << "\n";
        dbgs() << "MayWrite: " << MayWrite << "\n";
        dbgs() << "Schedule: " << Schedule << "\n");

  // The pointers below will be set by the subsequent calls to
  // isl_union_map_compute_flow.
  RAW = WAW = WAR = RED = nullptr;

  if (OptAnalysisType == VALUE_BASED_ANALYSIS) {
    isl_union_map_compute_flow(
        isl_union_map_copy(Read), isl_union_map_copy(Write),
        isl_union_map_copy(MayWrite), isl_union_map_copy(Schedule), &RAW,
        nullptr, nullptr, nullptr);

    isl_union_map_compute_flow(
        isl_union_map_copy(Write), isl_union_map_copy(Write),
        isl_union_map_copy(Read), isl_union_map_copy(Schedule), &WAW, &WAR,
        nullptr, nullptr);
  } else {
    isl_union_map *Empty;

    Empty = isl_union_map_empty(isl_union_map_get_space(Write));
    Write = isl_union_map_union(Write, isl_union_map_copy(MayWrite));

    isl_union_map_compute_flow(
        isl_union_map_copy(Read), isl_union_map_copy(Empty),
        isl_union_map_copy(Write), isl_union_map_copy(Schedule), nullptr, &RAW,
        nullptr, nullptr);

    isl_union_map_compute_flow(
        isl_union_map_copy(Write), isl_union_map_copy(Empty),
        isl_union_map_copy(Read), isl_union_map_copy(Schedule), nullptr, &WAR,
        nullptr, nullptr);

    isl_union_map_compute_flow(
        isl_union_map_copy(Write), isl_union_map_copy(Empty),
        isl_union_map_copy(Write), isl_union_map_copy(Schedule), nullptr, &WAW,
        nullptr, nullptr);
    isl_union_map_free(Empty);
  }

  isl_union_map_free(MayWrite);
  isl_union_map_free(Write);
  isl_union_map_free(Read);
  isl_union_map_free(Schedule);

  RAW = isl_union_map_coalesce(RAW);
  WAW = isl_union_map_coalesce(WAW);
  WAR = isl_union_map_coalesce(WAR);

  if (isl_ctx_last_error(S.getIslCtx()) == isl_error_quota) {
    isl_union_map_free(RAW);
    isl_union_map_free(WAW);
    isl_union_map_free(WAR);
    RAW = WAW = WAR = nullptr;
    isl_ctx_reset_error(S.getIslCtx());
  }
  isl_options_set_on_error(S.getIslCtx(), ISL_ON_ERROR_ABORT);
  isl_ctx_reset_operations(S.getIslCtx());
  isl_ctx_set_max_operations(S.getIslCtx(), MaxOpsOld);

  isl_union_map *STMT_RAW, *STMT_WAW, *STMT_WAR;
  STMT_RAW = isl_union_map_intersect_domain(
      isl_union_map_copy(RAW),
      isl_union_map_domain(isl_union_map_copy(StmtSchedule)));
  STMT_WAW = isl_union_map_intersect_domain(
      isl_union_map_copy(WAW),
      isl_union_map_domain(isl_union_map_copy(StmtSchedule)));
  STMT_WAR = isl_union_map_intersect_domain(isl_union_map_copy(WAR),
                                            isl_union_map_domain(StmtSchedule));
  DEBUG(dbgs() << "Wrapped Dependences:\n"; printScop(dbgs()); dbgs() << "\n");

  // To handle reduction dependences we proceed as follows:
  // 1) Aggregate all possible reduction dependences, namely all self
  //    dependences on reduction like statements.
  // 2) Intersect them with the actual RAW & WAW dependences to the get the
  //    actual reduction dependences. This will ensure the load/store memory
  //    addresses were __identical__ in the two iterations of the statement.
  // 3) Relax the original RAW and WAW dependences by substracting the actual
  //    reduction dependences. Binary reductions (sum += A[i]) cause both, and
  //    the same, RAW and WAW dependences.
  // 4) Add the privatization dependences which are widened versions of
  //    already present dependences. They model the effect of manual
  //    privatization at the outermost possible place (namely after the last
  //    write and before the first access to a reduction location).

  // Step 1)
  RED = isl_union_map_empty(isl_union_map_get_space(RAW));
  for (ScopStmt *Stmt : S) {
    for (MemoryAccess *MA : *Stmt) {
      if (!MA->isReductionLike())
        continue;
      isl_set *AccDomW = isl_map_wrap(MA->getAccessRelation());
      isl_map *Identity =
          isl_map_from_domain_and_range(isl_set_copy(AccDomW), AccDomW);
      RED = isl_union_map_add_map(RED, Identity);
    }
  }

  // Step 2)
  RED = isl_union_map_intersect(RED, isl_union_map_copy(RAW));
  RED = isl_union_map_intersect(RED, isl_union_map_copy(WAW));

  if (!isl_union_map_is_empty(RED)) {

    // Step 3)
    RAW = isl_union_map_subtract(RAW, isl_union_map_copy(RED));
    WAW = isl_union_map_subtract(WAW, isl_union_map_copy(RED));

    // Step 4)
    addPrivatizationDependences();
  }

  DEBUG(dbgs() << "Final Wrapped Dependences:\n"; printScop(dbgs());
        dbgs() << "\n");

  RAW = isl_union_map_zip(RAW);
  WAW = isl_union_map_zip(WAW);
  WAR = isl_union_map_zip(WAR);
  RED = isl_union_map_zip(RED);
  TC_RED = isl_union_map_zip(TC_RED);

  DEBUG(dbgs() << "Zipped Dependences:\n"; printScop(dbgs()); dbgs() << "\n");

  RAW = isl_union_set_unwrap(isl_union_map_domain(RAW));
  WAW = isl_union_set_unwrap(isl_union_map_domain(WAW));
  WAR = isl_union_set_unwrap(isl_union_map_domain(WAR));
  RED = isl_union_set_unwrap(isl_union_map_domain(RED));
  TC_RED = isl_union_set_unwrap(isl_union_map_domain(TC_RED));

  DEBUG(dbgs() << "Unwrapped Dependences:\n"; printScop(dbgs());
        dbgs() << "\n");

  RAW = isl_union_map_union(RAW, STMT_RAW);
  WAW = isl_union_map_union(WAW, STMT_WAW);
  WAR = isl_union_map_union(WAR, STMT_WAR);

  RAW = isl_union_map_coalesce(RAW);
  WAW = isl_union_map_coalesce(WAW);
  WAR = isl_union_map_coalesce(WAR);
  RED = isl_union_map_coalesce(RED);
  TC_RED = isl_union_map_coalesce(TC_RED);

  DEBUG(printScop(dbgs()));
}

bool Dependences::runOnScop(Scop &S) {
  releaseMemory();
  calculateDependences(S);

  return false;
}

bool Dependences::isValidScattering(StatementToIslMapTy *NewScattering) {
  Scop &S = getCurScop();

  if (LegalityCheckDisabled)
    return true;

  isl_union_map *Dependences = getDependences(TYPE_RAW | TYPE_WAW | TYPE_WAR);
  isl_space *Space = S.getParamSpace();
  isl_union_map *Scattering = isl_union_map_empty(Space);

  isl_space *ScatteringSpace = 0;

  for (ScopStmt *Stmt : S) {
    isl_map *StmtScat;

    if (NewScattering->find(Stmt) == NewScattering->end())
      StmtScat = Stmt->getScattering();
    else
      StmtScat = isl_map_copy((*NewScattering)[Stmt]);

    if (!ScatteringSpace)
      ScatteringSpace = isl_space_range(isl_map_get_space(StmtScat));

    Scattering = isl_union_map_add_map(Scattering, StmtScat);
  }

  Dependences =
      isl_union_map_apply_domain(Dependences, isl_union_map_copy(Scattering));
  Dependences = isl_union_map_apply_range(Dependences, Scattering);

  isl_set *Zero = isl_set_universe(isl_space_copy(ScatteringSpace));
  for (unsigned i = 0; i < isl_set_dim(Zero, isl_dim_set); i++)
    Zero = isl_set_fix_si(Zero, isl_dim_set, i, 0);

  isl_union_set *UDeltas = isl_union_map_deltas(Dependences);
  isl_set *Deltas = isl_union_set_extract_set(UDeltas, ScatteringSpace);
  isl_union_set_free(UDeltas);

  isl_map *NonPositive = isl_set_lex_le_set(Deltas, Zero);
  bool IsValid = isl_map_is_empty(NonPositive);
  isl_map_free(NonPositive);

  return IsValid;
}

isl_union_map *getCombinedScheduleForSpace(Scop *scop, unsigned dimLevel) {
  isl_space *Space = scop->getParamSpace();
  isl_union_map *schedule = isl_union_map_empty(Space);

  for (ScopStmt *Stmt : *scop) {
    unsigned remainingDimensions = Stmt->getNumScattering() - dimLevel;
    isl_map *Scattering = isl_map_project_out(
        Stmt->getScattering(), isl_dim_out, dimLevel, remainingDimensions);
    schedule = isl_union_map_add_map(schedule, Scattering);
  }

  return schedule;
}

bool Dependences::isParallelDimension(__isl_take isl_set *ScheduleSubset,
                                      unsigned ParallelDim) {
  // To check if a loop is parallel, we perform the following steps:
  //
  // o Move dependences from 'Domain -> Domain' to 'Schedule -> Schedule' space.
  // o Limit dependences to the schedule space enumerated by the loop.
  // o Calculate distances of the dependences.
  // o Check if one of the distances is invalid in presence of parallelism.

  isl_union_map *Schedule, *Deps;
  isl_map *ScheduleDeps;
  Scop *S = &getCurScop();

  if (!hasValidDependences()) {
    isl_set_free(ScheduleSubset);
    return false;
  }

  // FIXME: We can remove ignore reduction dependences in case we privatize the
  //        memory locations the reduction statements reduce into.
  Deps = getDependences(TYPE_RAW | TYPE_WAW | TYPE_WAR | TYPE_RED);

  if (isl_union_map_is_empty(Deps)) {
    isl_union_map_free(Deps);
    isl_set_free(ScheduleSubset);
    return true;
  }

  Schedule = getCombinedScheduleForSpace(S, ParallelDim);
  Deps = isl_union_map_apply_range(Deps, isl_union_map_copy(Schedule));
  Deps = isl_union_map_apply_domain(Deps, Schedule);

  if (isl_union_map_is_empty(Deps)) {
    isl_union_map_free(Deps);
    isl_set_free(ScheduleSubset);
    return true;
  }

  ScheduleDeps = isl_map_from_union_map(Deps);
  ScheduleDeps =
      isl_map_intersect_domain(ScheduleDeps, isl_set_copy(ScheduleSubset));
  ScheduleDeps = isl_map_intersect_range(ScheduleDeps, ScheduleSubset);

  isl_set *Distances = isl_map_deltas(ScheduleDeps);
  isl_space *Space = isl_set_get_space(Distances);
  isl_set *Invalid = isl_set_universe(Space);

  // [0, ..., 0, +] - All zeros and last dimension larger than zero
  for (unsigned i = 0; i < ParallelDim - 1; i++)
    Invalid = isl_set_fix_si(Invalid, isl_dim_set, i, 0);

  Invalid = isl_set_lower_bound_si(Invalid, isl_dim_set, ParallelDim - 1, 1);
  Invalid = isl_set_intersect(Invalid, Distances);

  bool IsParallel = isl_set_is_empty(Invalid);
  isl_set_free(Invalid);

  return IsParallel;
}

static void printDependencyMap(raw_ostream &OS, __isl_keep isl_union_map *DM) {
  if (DM)
    OS << DM << "\n";
  else
    OS << "n/a\n";
}

void Dependences::printScop(raw_ostream &OS) const {
  OS << "\tRAW dependences:\n\t\t";
  printDependencyMap(OS, RAW);
  OS << "\tWAR dependences:\n\t\t";
  printDependencyMap(OS, WAR);
  OS << "\tWAW dependences:\n\t\t";
  printDependencyMap(OS, WAW);
  OS << "\tReduction dependences:\n\t\t";
  printDependencyMap(OS, RED);
  OS << "\tTransitive closure of reduction dependences:\n\t\t";
  printDependencyMap(OS, TC_RED);
}

void Dependences::releaseMemory() {
  isl_union_map_free(RAW);
  isl_union_map_free(WAR);
  isl_union_map_free(WAW);
  isl_union_map_free(RED);
  isl_union_map_free(TC_RED);

  RED = RAW = WAR = WAW = TC_RED = nullptr;
}

isl_union_map *Dependences::getDependences(int Kinds) {
  assert(hasValidDependences() && "No valid dependences available");
  isl_space *Space = isl_union_map_get_space(RAW);
  isl_union_map *Deps = isl_union_map_empty(Space);

  if (Kinds & TYPE_RAW)
    Deps = isl_union_map_union(Deps, isl_union_map_copy(RAW));

  if (Kinds & TYPE_WAR)
    Deps = isl_union_map_union(Deps, isl_union_map_copy(WAR));

  if (Kinds & TYPE_WAW)
    Deps = isl_union_map_union(Deps, isl_union_map_copy(WAW));

  if (Kinds & TYPE_RED)
    Deps = isl_union_map_union(Deps, isl_union_map_copy(RED));

  if (Kinds & TYPE_TC_RED)
    Deps = isl_union_map_union(Deps, isl_union_map_copy(TC_RED));

  Deps = isl_union_map_coalesce(Deps);
  Deps = isl_union_map_detect_equalities(Deps);
  return Deps;
}

bool Dependences::hasValidDependences() {
  return (RAW != nullptr) && (WAR != nullptr) && (WAW != nullptr);
}

void Dependences::getAnalysisUsage(AnalysisUsage &AU) const {
  ScopPass::getAnalysisUsage(AU);
}

char Dependences::ID = 0;

Pass *polly::createDependencesPass() { return new Dependences(); }

INITIALIZE_PASS_BEGIN(Dependences, "polly-dependences",
                      "Polly - Calculate dependences", false, false);
INITIALIZE_PASS_DEPENDENCY(ScopInfo);
INITIALIZE_PASS_END(Dependences, "polly-dependences",
                    "Polly - Calculate dependences", false, false)
