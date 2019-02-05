//===------ LoopGenerators.cpp -  IR helper to create loops ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains functions to create scalar and OpenMP parallel loops
// as LLVM-IR.
//
//===----------------------------------------------------------------------===//

#include "polly/ScopDetection.h"
#include "polly/CodeGen/LoopGenerators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;
using namespace polly;

// We generate a loop of the following structure
//
//              BeforeBB
//                 |
//                 v
//              GuardBB
//              /      |
//     __  PreHeaderBB  |
//    /  \    /         |
// latch  HeaderBB      |
//    \  /    \         /
//     <       \       /
//              \     /
//              ExitBB
//
// GuardBB checks if the loop is executed at least once. If this is the case
// we branch to PreHeaderBB and subsequently to the HeaderBB, which contains the
// loop iv 'polly.indvar', the incremented loop iv 'polly.indvar_next' as well
// as the condition to check if we execute another iteration of the loop. After
// the loop has finished, we branch to ExitBB.
//
// TODO: We currently always create the GuardBB. If we can prove the loop is
//       always executed at least once, we can get rid of this branch.
Value *polly::createLoop(Value *LB, Value *UB, Value *Stride,
                         PollyIRBuilder &Builder, Pass *P, BasicBlock *&ExitBB,
                         ICmpInst::Predicate Predicate,
                         LoopAnnotator *Annotator, bool Parallel) {
  DominatorTree &DT = P->getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  LoopInfo &LI = P->getAnalysis<LoopInfo>();
  Function *F = Builder.GetInsertBlock()->getParent();
  LLVMContext &Context = F->getContext();

  assert(LB->getType() == UB->getType() && "Types of loop bounds do not match");
  IntegerType *LoopIVType = dyn_cast<IntegerType>(UB->getType());
  assert(LoopIVType && "UB is not integer?");

  BasicBlock *BeforeBB = Builder.GetInsertBlock();
  BasicBlock *GuardBB = BasicBlock::Create(Context, "polly.loop_if", F);
  BasicBlock *HeaderBB = BasicBlock::Create(Context, "polly.loop_header", F);
  BasicBlock *PreHeaderBB =
      BasicBlock::Create(Context, "polly.loop_preheader", F);

  if (Annotator) {
    Annotator->Begin(HeaderBB);
    if (Parallel)
      Annotator->SetCurrentParallel();
  }

  // Update LoopInfo
  Loop *OuterLoop = LI.getLoopFor(BeforeBB);
  Loop *NewLoop = new Loop();

  if (OuterLoop) {
    OuterLoop->addChildLoop(NewLoop);
  } else {
    LI.addTopLevelLoop(NewLoop);
  }

  if (OuterLoop) {
    OuterLoop->addBasicBlockToLoop(GuardBB, LI.getBase());
    OuterLoop->addBasicBlockToLoop(PreHeaderBB, LI.getBase());
  }

  NewLoop->addBasicBlockToLoop(HeaderBB, LI.getBase());

  // ExitBB
  ExitBB = SplitBlock(BeforeBB, Builder.GetInsertPoint()++, P);
  ExitBB->setName("polly.loop_exit");

  // BeforeBB
  BeforeBB->getTerminator()->setSuccessor(0, GuardBB);

  // GuardBB
  DT.addNewBlock(GuardBB, BeforeBB);
  Builder.SetInsertPoint(GuardBB);
  Value *LoopGuard;
  LoopGuard = Builder.CreateICmp(Predicate, LB, UB);
  LoopGuard->setName("polly.loop_guard");
  Builder.CreateCondBr(LoopGuard, PreHeaderBB, ExitBB);

  // PreHeaderBB
  DT.addNewBlock(PreHeaderBB, GuardBB);
  Builder.SetInsertPoint(PreHeaderBB);
  Builder.CreateBr(HeaderBB);

  // HeaderBB
  DT.addNewBlock(HeaderBB, PreHeaderBB);
  Builder.SetInsertPoint(HeaderBB);
  PHINode *IV = Builder.CreatePHI(LoopIVType, 2, "polly.indvar");
  IV->addIncoming(LB, PreHeaderBB);
  Stride = Builder.CreateZExtOrBitCast(Stride, LoopIVType);
  Value *IncrementedIV = Builder.CreateNSWAdd(IV, Stride, "polly.indvar_next");
  Value *LoopCondition;
  UB = Builder.CreateSub(UB, Stride, "polly.adjust_ub");
  LoopCondition = Builder.CreateICmp(Predicate, IV, UB);
  LoopCondition->setName("polly.loop_cond");
  Builder.CreateCondBr(LoopCondition, HeaderBB, ExitBB);
  IV->addIncoming(IncrementedIV, HeaderBB);
  DT.changeImmediateDominator(ExitBB, GuardBB);

  // The loop body should be added here.
  Builder.SetInsertPoint(HeaderBB->getFirstNonPHI());
  return IV;
}

void OMPGenerator::createCallParallelLoopStart(
    Value *SubFunction, Value *SubfunctionParam, Value *NumberOfThreads,
    Value *LowerBound, Value *UpperBound, Value *Stride) {
  Module *M = getModule();
  const char *Name = "GOMP_parallel_loop_runtime_start";
  Function *F = M->getFunction(Name);

  // If F is not available, declare it.
  if (!F) {
    Type *LongTy = getIntPtrTy();
    GlobalValue::LinkageTypes Linkage = Function::ExternalLinkage;

    Type *Params[] = {PointerType::getUnqual(FunctionType::get(
                          Builder.getVoidTy(), Builder.getInt8PtrTy(), false)),
                      Builder.getInt8PtrTy(), Builder.getInt32Ty(), LongTy,
                      LongTy, LongTy};

    FunctionType *Ty = FunctionType::get(Builder.getVoidTy(), Params, false);
    F = Function::Create(Ty, Linkage, Name, M);
  }

  Value *Args[] = {SubFunction, SubfunctionParam, NumberOfThreads,
                   LowerBound,  UpperBound,       Stride};

  Builder.CreateCall(F, Args);
}

Value *OMPGenerator::createCallLoopNext(Value *LowerBoundPtr,
                                        Value *UpperBoundPtr) {
  Module *M = getModule();
  const char *Name = "GOMP_loop_runtime_next";
  Function *F = M->getFunction(Name);

  // If F is not available, declare it.
  if (!F) {
    Type *LongPtrTy = PointerType::getUnqual(getIntPtrTy());
    GlobalValue::LinkageTypes Linkage = Function::ExternalLinkage;

    Type *Params[] = {LongPtrTy, LongPtrTy};

    FunctionType *Ty = FunctionType::get(Builder.getInt8Ty(), Params, false);
    F = Function::Create(Ty, Linkage, Name, M);
  }

  Value *Args[] = {LowerBoundPtr, UpperBoundPtr};

  Value *Return = Builder.CreateCall(F, Args);
  Return = Builder.CreateICmpNE(
      Return, Builder.CreateZExt(Builder.getFalse(), Return->getType()));
  return Return;
}

void OMPGenerator::createCallParallelEnd() {
  const char *Name = "GOMP_parallel_end";
  Module *M = getModule();
  Function *F = M->getFunction(Name);

  // If F is not available, declare it.
  if (!F) {
    GlobalValue::LinkageTypes Linkage = Function::ExternalLinkage;

    FunctionType *Ty = FunctionType::get(Builder.getVoidTy(), false);
    F = Function::Create(Ty, Linkage, Name, M);
  }

  Builder.CreateCall(F);
}

void OMPGenerator::createCallLoopEndNowait() {
  const char *Name = "GOMP_loop_end_nowait";
  Module *M = getModule();
  Function *F = M->getFunction(Name);

  // If F is not available, declare it.
  if (!F) {
    GlobalValue::LinkageTypes Linkage = Function::ExternalLinkage;

    FunctionType *Ty = FunctionType::get(Builder.getVoidTy(), false);
    F = Function::Create(Ty, Linkage, Name, M);
  }

  Builder.CreateCall(F);
}

IntegerType *OMPGenerator::getIntPtrTy() {
  return P->getAnalysis<DataLayoutPass>().getDataLayout().getIntPtrType(
      Builder.getContext());
}

Module *OMPGenerator::getModule() {
  return Builder.GetInsertBlock()->getParent()->getParent();
}

Function *OMPGenerator::createSubfunctionDefinition() {
  Module *M = getModule();
  Function *F = Builder.GetInsertBlock()->getParent();
  std::vector<Type *> Arguments(1, Builder.getInt8PtrTy());
  FunctionType *FT = FunctionType::get(Builder.getVoidTy(), Arguments, false);
  Function *FN = Function::Create(FT, Function::InternalLinkage,
                                  F->getName() + ".omp_subfn", M);
  // Do not run any polly pass on the new function.
  FN->addFnAttr(PollySkipFnAttr);

  Function::arg_iterator AI = FN->arg_begin();
  AI->setName("omp.userContext");

  return FN;
}

Value *OMPGenerator::loadValuesIntoStruct(SetVector<Value *> &Values) {
  std::vector<Type *> Members;

  for (Value *V : Values)
    Members.push_back(V->getType());

  StructType *Ty = StructType::get(Builder.getContext(), Members);
  Value *Struct = Builder.CreateAlloca(Ty, 0, "omp.userContext");

  for (unsigned i = 0; i < Values.size(); i++) {
    Value *Address = Builder.CreateStructGEP(Struct, i);
    Builder.CreateStore(Values[i], Address);
  }

  return Struct;
}

void OMPGenerator::extractValuesFromStruct(SetVector<Value *> OldValues,
                                           Value *Struct,
                                           ValueToValueMapTy &Map) {
  for (unsigned i = 0; i < OldValues.size(); i++) {
    Value *Address = Builder.CreateStructGEP(Struct, i);
    Value *NewValue = Builder.CreateLoad(Address);
    Map.insert(std::make_pair(OldValues[i], NewValue));
  }
}

Value *OMPGenerator::createSubfunction(Value *Stride, Value *StructData,
                                       SetVector<Value *> Data,
                                       ValueToValueMapTy &Map,
                                       Function **SubFunction) {
  Function *FN = createSubfunctionDefinition();

  BasicBlock *PrevBB, *HeaderBB, *ExitBB, *CheckNextBB, *LoadIVBoundsBB,
      *AfterBB;
  Value *LowerBoundPtr, *UpperBoundPtr, *UserContext, *Ret1, *HasNextSchedule,
      *LowerBound, *UpperBound, *IV;
  Type *IntPtrTy = getIntPtrTy();
  LLVMContext &Context = FN->getContext();

  // Store the previous basic block.
  PrevBB = Builder.GetInsertBlock();

  // Create basic blocks.
  HeaderBB = BasicBlock::Create(Context, "omp.setup", FN);
  ExitBB = BasicBlock::Create(Context, "omp.exit", FN);
  CheckNextBB = BasicBlock::Create(Context, "omp.checkNext", FN);
  LoadIVBoundsBB = BasicBlock::Create(Context, "omp.loadIVBounds", FN);

  DominatorTree &DT = P->getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  DT.addNewBlock(HeaderBB, PrevBB);
  DT.addNewBlock(ExitBB, HeaderBB);
  DT.addNewBlock(CheckNextBB, HeaderBB);
  DT.addNewBlock(LoadIVBoundsBB, HeaderBB);

  // Fill up basic block HeaderBB.
  Builder.SetInsertPoint(HeaderBB);
  LowerBoundPtr = Builder.CreateAlloca(IntPtrTy, 0, "omp.lowerBoundPtr");
  UpperBoundPtr = Builder.CreateAlloca(IntPtrTy, 0, "omp.upperBoundPtr");
  UserContext = Builder.CreateBitCast(FN->arg_begin(), StructData->getType(),
                                      "omp.userContext");

  extractValuesFromStruct(Data, UserContext, Map);
  Builder.CreateBr(CheckNextBB);

  // Add code to check if another set of iterations will be executed.
  Builder.SetInsertPoint(CheckNextBB);
  Ret1 = createCallLoopNext(LowerBoundPtr, UpperBoundPtr);
  HasNextSchedule = Builder.CreateTrunc(Ret1, Builder.getInt1Ty(),
                                        "omp.hasNextScheduleBlock");
  Builder.CreateCondBr(HasNextSchedule, LoadIVBoundsBB, ExitBB);

  // Add code to to load the iv bounds for this set of iterations.
  Builder.SetInsertPoint(LoadIVBoundsBB);
  LowerBound = Builder.CreateLoad(LowerBoundPtr, "omp.lowerBound");
  UpperBound = Builder.CreateLoad(UpperBoundPtr, "omp.upperBound");

  // Subtract one as the upper bound provided by openmp is a < comparison
  // whereas the codegenForSequential function creates a <= comparison.
  UpperBound = Builder.CreateSub(UpperBound, ConstantInt::get(IntPtrTy, 1),
                                 "omp.upperBoundAdjusted");

  Builder.CreateBr(CheckNextBB);
  Builder.SetInsertPoint(--Builder.GetInsertPoint());
  IV = createLoop(LowerBound, UpperBound, Stride, Builder, P, AfterBB,
                  ICmpInst::ICMP_SLE);

  BasicBlock::iterator LoopBody = Builder.GetInsertPoint();
  Builder.SetInsertPoint(AfterBB->begin());

  // Add code to terminate this openmp subfunction.
  Builder.SetInsertPoint(ExitBB);
  createCallLoopEndNowait();
  Builder.CreateRetVoid();

  Builder.SetInsertPoint(LoopBody);
  *SubFunction = FN;

  return IV;
}

Value *OMPGenerator::createParallelLoop(Value *LowerBound, Value *UpperBound,
                                        Value *Stride,
                                        SetVector<Value *> &Values,
                                        ValueToValueMapTy &Map,
                                        BasicBlock::iterator *LoopBody) {
  Value *Struct, *IV, *SubfunctionParam, *NumberOfThreads;
  Function *SubFunction;

  Struct = loadValuesIntoStruct(Values);

  BasicBlock::iterator PrevInsertPoint = Builder.GetInsertPoint();
  IV = createSubfunction(Stride, Struct, Values, Map, &SubFunction);
  *LoopBody = Builder.GetInsertPoint();
  Builder.SetInsertPoint(PrevInsertPoint);

  // Create call for GOMP_parallel_loop_runtime_start.
  SubfunctionParam =
      Builder.CreateBitCast(Struct, Builder.getInt8PtrTy(), "omp_data");

  NumberOfThreads = Builder.getInt32(0);

  // Add one as the upper bound provided by openmp is a < comparison
  // whereas the codegenForSequential function creates a <= comparison.
  UpperBound =
      Builder.CreateAdd(UpperBound, ConstantInt::get(getIntPtrTy(), 1));

  createCallParallelLoopStart(SubFunction, SubfunctionParam, NumberOfThreads,
                              LowerBound, UpperBound, Stride);
  Builder.CreateCall(SubFunction, SubfunctionParam);
  createCallParallelEnd();

  return IV;
}
