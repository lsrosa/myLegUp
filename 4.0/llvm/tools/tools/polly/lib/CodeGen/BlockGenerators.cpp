//===--- BlockGenerators.cpp - Generate code for statements -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the BlockGenerator and VectorBlockGenerator classes,
// which generate sequential code and vectorized code for a polyhedral
// statement, respectively.
//
//===----------------------------------------------------------------------===//

#include "polly/ScopInfo.h"
#include "isl/aff.h"
#include "isl/set.h"
#include "polly/CodeGen/BlockGenerators.h"
#include "polly/CodeGen/CodeGeneration.h"
#include "polly/Options.h"
#include "polly/Support/GICHelper.h"
#include "polly/Support/SCEVValidator.h"
#include "polly/Support/ScopHelper.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;
using namespace polly;

static cl::opt<bool>
    Aligned("enable-polly-aligned",
            cl::desc("Assumed aligned memory accesses."), cl::Hidden,
            cl::value_desc("OpenMP code generation enabled if true"),
            cl::init(false), cl::ZeroOrMore, cl::cat(PollyCategory));

static cl::opt<bool, true>
    SCEVCodegenF("polly-codegen-scev",
                 cl::desc("Use SCEV based code generation."), cl::Hidden,
                 cl::location(SCEVCodegen), cl::init(false), cl::ZeroOrMore,
                 cl::cat(PollyCategory));

bool polly::SCEVCodegen;

bool polly::canSynthesize(const Instruction *I, const llvm::LoopInfo *LI,
                          ScalarEvolution *SE, const Region *R) {
  if (SCEVCodegen) {
    if (!I || !SE->isSCEVable(I->getType()))
      return false;

    if (const SCEV *Scev = SE->getSCEV(const_cast<Instruction *>(I)))
      if (!isa<SCEVCouldNotCompute>(Scev))
        if (!hasScalarDepsInsideRegion(Scev, R))
          return true;

    return false;
  }

  Loop *L = LI->getLoopFor(I->getParent());
  return L && I == L->getCanonicalInductionVariable() && R->contains(L);
}

// Helper class to generate memory location.
namespace {
class IslGenerator {
public:
  IslGenerator(PollyIRBuilder &Builder, std::vector<Value *> &IVS)
      : Builder(Builder), IVS(IVS) {}
  Value *generateIslVal(__isl_take isl_val *Val);
  Value *generateIslAff(__isl_take isl_aff *Aff);
  Value *generateIslPwAff(__isl_take isl_pw_aff *PwAff);

private:
  typedef struct {
    Value *Result;
    class IslGenerator *Generator;
  } IslGenInfo;

  PollyIRBuilder &Builder;
  std::vector<Value *> &IVS;
  static int mergeIslAffValues(__isl_take isl_set *Set, __isl_take isl_aff *Aff,
                               void *User);
};
}

Value *IslGenerator::generateIslVal(__isl_take isl_val *Val) {
  Value *IntValue = Builder.getInt(APIntFromVal(Val));
  return IntValue;
}

Value *IslGenerator::generateIslAff(__isl_take isl_aff *Aff) {
  Value *Result;
  Value *ConstValue;
  isl_val *Val;

  Val = isl_aff_get_constant_val(Aff);
  ConstValue = generateIslVal(Val);
  Type *Ty = Builder.getInt64Ty();

  // FIXME: We should give the constant and coefficients the right type. Here
  // we force it into i64.
  Result = Builder.CreateSExtOrBitCast(ConstValue, Ty);

  unsigned int NbInputDims = isl_aff_dim(Aff, isl_dim_in);

  assert((IVS.size() == NbInputDims) &&
         "The Dimension of Induction Variables must match the dimension of the "
         "affine space.");

  for (unsigned int i = 0; i < NbInputDims; ++i) {
    Value *CoefficientValue;
    Val = isl_aff_get_coefficient_val(Aff, isl_dim_in, i);

    if (isl_val_is_zero(Val)) {
      isl_val_free(Val);
      continue;
    }

    CoefficientValue = generateIslVal(Val);
    CoefficientValue = Builder.CreateIntCast(CoefficientValue, Ty, true);
    Value *IV = Builder.CreateIntCast(IVS[i], Ty, true);
    Value *PAdd = Builder.CreateMul(CoefficientValue, IV, "p_mul_coeff");
    Result = Builder.CreateAdd(Result, PAdd, "p_sum_coeff");
  }

  isl_aff_free(Aff);

  return Result;
}

int IslGenerator::mergeIslAffValues(__isl_take isl_set *Set,
                                    __isl_take isl_aff *Aff, void *User) {
  IslGenInfo *GenInfo = (IslGenInfo *)User;

  assert((GenInfo->Result == nullptr) &&
         "Result is already set. Currently only single isl_aff is supported");
  assert(isl_set_plain_is_universe(Set) &&
         "Code generation failed because the set is not universe");

  GenInfo->Result = GenInfo->Generator->generateIslAff(Aff);

  isl_set_free(Set);
  return 0;
}

Value *IslGenerator::generateIslPwAff(__isl_take isl_pw_aff *PwAff) {
  IslGenInfo User;
  User.Result = nullptr;
  User.Generator = this;
  isl_pw_aff_foreach_piece(PwAff, mergeIslAffValues, &User);
  assert(User.Result && "Code generation for isl_pw_aff failed");

  isl_pw_aff_free(PwAff);
  return User.Result;
}

BlockGenerator::BlockGenerator(PollyIRBuilder &B, ScopStmt &Stmt, Pass *P)
    : Builder(B), Statement(Stmt), P(P), SE(P->getAnalysis<ScalarEvolution>()) {
}

Value *BlockGenerator::lookupAvailableValue(const Value *Old, ValueMapT &BBMap,
                                            ValueMapT &GlobalMap) const {
  // We assume constants never change.
  // This avoids map lookups for many calls to this function.
  if (isa<Constant>(Old))
    return const_cast<Value *>(Old);

  if (Value *New = GlobalMap.lookup(Old)) {
    if (Old->getType()->getScalarSizeInBits() <
        New->getType()->getScalarSizeInBits())
      New = Builder.CreateTruncOrBitCast(New, Old->getType());

    return New;
  }

  // Or it is probably a scop-constant value defined as global, function
  // parameter or an instruction not within the scop.
  if (isa<GlobalValue>(Old) || isa<Argument>(Old))
    return const_cast<Value *>(Old);

  if (const Instruction *Inst = dyn_cast<Instruction>(Old))
    if (!Statement.getParent()->getRegion().contains(Inst->getParent()))
      return const_cast<Value *>(Old);

  if (Value *New = BBMap.lookup(Old))
    return New;

  return nullptr;
}

Value *BlockGenerator::getNewValue(const Value *Old, ValueMapT &BBMap,
                                   ValueMapT &GlobalMap, LoopToScevMapT &LTS,
                                   Loop *L) {
  if (Value *New = lookupAvailableValue(Old, BBMap, GlobalMap))
    return New;

  if (SCEVCodegen && SE.isSCEVable(Old->getType()))
    if (const SCEV *Scev = SE.getSCEVAtScope(const_cast<Value *>(Old), L)) {
      if (!isa<SCEVCouldNotCompute>(Scev)) {
        const SCEV *NewScev = apply(Scev, LTS, SE);
        ValueToValueMap VTV;
        VTV.insert(BBMap.begin(), BBMap.end());
        VTV.insert(GlobalMap.begin(), GlobalMap.end());
        NewScev = SCEVParameterRewriter::rewrite(NewScev, SE, VTV);
        SCEVExpander Expander(SE, "polly");
        Value *Expanded = Expander.expandCodeFor(NewScev, Old->getType(),
                                                 Builder.GetInsertPoint());

        BBMap[Old] = Expanded;
        return Expanded;
      }
    }

  // Now the scalar dependence is neither available nor SCEVCodegenable, this
  // should never happen in the current code generator.
  llvm_unreachable("Unexpected scalar dependence in region!");
  return nullptr;
}

void BlockGenerator::copyInstScalar(const Instruction *Inst, ValueMapT &BBMap,
                                    ValueMapT &GlobalMap, LoopToScevMapT &LTS) {
  // We do not generate debug intrinsics as we did not investigate how to
  // copy them correctly. At the current state, they just crash the code
  // generation as the meta-data operands are not correctly copied.
  if (isa<DbgInfoIntrinsic>(Inst))
    return;

  Instruction *NewInst = Inst->clone();

  // Replace old operands with the new ones.
  for (Value *OldOperand : Inst->operands()) {
    Value *NewOperand =
        getNewValue(OldOperand, BBMap, GlobalMap, LTS, getLoopForInst(Inst));

    if (!NewOperand) {
      assert(!isa<StoreInst>(NewInst) &&
             "Store instructions are always needed!");
      delete NewInst;
      return;
    }

    NewInst->replaceUsesOfWith(OldOperand, NewOperand);
  }

  Builder.Insert(NewInst);
  BBMap[Inst] = NewInst;

  if (!NewInst->getType()->isVoidTy())
    NewInst->setName("p_" + Inst->getName());
}

std::vector<Value *> BlockGenerator::getMemoryAccessIndex(
    __isl_keep isl_map *AccessRelation, Value *BaseAddress, ValueMapT &BBMap,
    ValueMapT &GlobalMap, LoopToScevMapT &LTS, Loop *L) {
  assert((isl_map_dim(AccessRelation, isl_dim_out) == 1) &&
         "Only single dimensional access functions supported");

  std::vector<Value *> IVS;
  for (unsigned i = 0; i < Statement.getNumIterators(); ++i) {
    const Value *OriginalIV = Statement.getInductionVariableForDimension(i);
    Value *NewIV = getNewValue(OriginalIV, BBMap, GlobalMap, LTS, L);
    IVS.push_back(NewIV);
  }

  isl_pw_aff *PwAff = isl_map_dim_max(isl_map_copy(AccessRelation), 0);
  IslGenerator IslGen(Builder, IVS);
  Value *OffsetValue = IslGen.generateIslPwAff(PwAff);

  Type *Ty = Builder.getInt64Ty();
  OffsetValue = Builder.CreateIntCast(OffsetValue, Ty, true);

  std::vector<Value *> IndexArray;
  Value *NullValue = Constant::getNullValue(Ty);
  IndexArray.push_back(NullValue);
  IndexArray.push_back(OffsetValue);
  return IndexArray;
}

Value *BlockGenerator::getNewAccessOperand(
    __isl_keep isl_map *NewAccessRelation, Value *BaseAddress, ValueMapT &BBMap,
    ValueMapT &GlobalMap, LoopToScevMapT &LTS, Loop *L) {
  std::vector<Value *> IndexArray = getMemoryAccessIndex(
      NewAccessRelation, BaseAddress, BBMap, GlobalMap, LTS, L);
  Value *NewOperand =
      Builder.CreateGEP(BaseAddress, IndexArray, "p_newarrayidx_");
  return NewOperand;
}

Value *BlockGenerator::generateLocationAccessed(const Instruction *Inst,
                                                const Value *Pointer,
                                                ValueMapT &BBMap,
                                                ValueMapT &GlobalMap,
                                                LoopToScevMapT &LTS) {
  const MemoryAccess &Access = Statement.getAccessFor(Inst);
  isl_map *CurrentAccessRelation = Access.getAccessRelation();
  isl_map *NewAccessRelation = Access.getNewAccessRelation();

  assert(isl_map_has_equal_space(CurrentAccessRelation, NewAccessRelation) &&
         "Current and new access function use different spaces");

  Value *NewPointer;

  if (!NewAccessRelation) {
    NewPointer =
        getNewValue(Pointer, BBMap, GlobalMap, LTS, getLoopForInst(Inst));
  } else {
    Value *BaseAddress = const_cast<Value *>(Access.getBaseAddr());
    NewPointer = getNewAccessOperand(NewAccessRelation, BaseAddress, BBMap,
                                     GlobalMap, LTS, getLoopForInst(Inst));
  }

  isl_map_free(CurrentAccessRelation);
  isl_map_free(NewAccessRelation);
  return NewPointer;
}

Loop *BlockGenerator::getLoopForInst(const llvm::Instruction *Inst) {
  return P->getAnalysis<LoopInfo>().getLoopFor(Inst->getParent());
}

Value *BlockGenerator::generateScalarLoad(const LoadInst *Load,
                                          ValueMapT &BBMap,
                                          ValueMapT &GlobalMap,
                                          LoopToScevMapT &LTS) {
  const Value *Pointer = Load->getPointerOperand();
  const Instruction *Inst = dyn_cast<Instruction>(Load);
  Value *NewPointer =
      generateLocationAccessed(Inst, Pointer, BBMap, GlobalMap, LTS);
  Value *ScalarLoad =
      Builder.CreateLoad(NewPointer, Load->getName() + "_p_scalar_");
  return ScalarLoad;
}

Value *BlockGenerator::generateScalarStore(const StoreInst *Store,
                                           ValueMapT &BBMap,
                                           ValueMapT &GlobalMap,
                                           LoopToScevMapT &LTS) {
  const Value *Pointer = Store->getPointerOperand();
  Value *NewPointer =
      generateLocationAccessed(Store, Pointer, BBMap, GlobalMap, LTS);
  Value *ValueOperand = getNewValue(Store->getValueOperand(), BBMap, GlobalMap,
                                    LTS, getLoopForInst(Store));

  return Builder.CreateStore(ValueOperand, NewPointer);
}

void BlockGenerator::copyInstruction(const Instruction *Inst, ValueMapT &BBMap,
                                     ValueMapT &GlobalMap,
                                     LoopToScevMapT &LTS) {
  // Terminator instructions control the control flow. They are explicitly
  // expressed in the clast and do not need to be copied.
  if (Inst->isTerminator())
    return;

  if (canSynthesize(Inst, &P->getAnalysis<LoopInfo>(), &SE,
                    &Statement.getParent()->getRegion()))
    return;

  if (const LoadInst *Load = dyn_cast<LoadInst>(Inst)) {
    Value *NewLoad = generateScalarLoad(Load, BBMap, GlobalMap, LTS);
    // Compute NewLoad before its insertion in BBMap to make the insertion
    // deterministic.
    BBMap[Load] = NewLoad;
    return;
  }

  if (const StoreInst *Store = dyn_cast<StoreInst>(Inst)) {
    Value *NewStore = generateScalarStore(Store, BBMap, GlobalMap, LTS);
    // Compute NewStore before its insertion in BBMap to make the insertion
    // deterministic.
    BBMap[Store] = NewStore;
    return;
  }

  copyInstScalar(Inst, BBMap, GlobalMap, LTS);
}

void BlockGenerator::copyBB(ValueMapT &GlobalMap, LoopToScevMapT &LTS) {
  BasicBlock *BB = Statement.getBasicBlock();
  BasicBlock *CopyBB =
      SplitBlock(Builder.GetInsertBlock(), Builder.GetInsertPoint(), P);
  CopyBB->setName("polly.stmt." + BB->getName());
  Builder.SetInsertPoint(CopyBB->begin());

  ValueMapT BBMap;

  for (Instruction &Inst : *BB)
    copyInstruction(&Inst, BBMap, GlobalMap, LTS);
}

VectorBlockGenerator::VectorBlockGenerator(PollyIRBuilder &B,
                                           VectorValueMapT &GlobalMaps,
                                           std::vector<LoopToScevMapT> &VLTS,
                                           ScopStmt &Stmt,
                                           __isl_keep isl_map *Schedule,
                                           Pass *P)
    : BlockGenerator(B, Stmt, P), GlobalMaps(GlobalMaps), VLTS(VLTS),
      Schedule(Schedule) {
  assert(GlobalMaps.size() > 1 && "Only one vector lane found");
  assert(Schedule && "No statement domain provided");
}

Value *VectorBlockGenerator::getVectorValue(const Value *Old,
                                            ValueMapT &VectorMap,
                                            VectorValueMapT &ScalarMaps,
                                            Loop *L) {
  if (Value *NewValue = VectorMap.lookup(Old))
    return NewValue;

  int Width = getVectorWidth();

  Value *Vector = UndefValue::get(VectorType::get(Old->getType(), Width));

  for (int Lane = 0; Lane < Width; Lane++)
    Vector = Builder.CreateInsertElement(
        Vector,
        getNewValue(Old, ScalarMaps[Lane], GlobalMaps[Lane], VLTS[Lane], L),
        Builder.getInt32(Lane));

  VectorMap[Old] = Vector;

  return Vector;
}

Type *VectorBlockGenerator::getVectorPtrTy(const Value *Val, int Width) {
  PointerType *PointerTy = dyn_cast<PointerType>(Val->getType());
  assert(PointerTy && "PointerType expected");

  Type *ScalarType = PointerTy->getElementType();
  VectorType *VectorType = VectorType::get(ScalarType, Width);

  return PointerType::getUnqual(VectorType);
}

Value *
VectorBlockGenerator::generateStrideOneLoad(const LoadInst *Load,
                                            VectorValueMapT &ScalarMaps,
                                            bool NegativeStride = false) {
  unsigned VectorWidth = getVectorWidth();
  const Value *Pointer = Load->getPointerOperand();
  Type *VectorPtrType = getVectorPtrTy(Pointer, VectorWidth);
  unsigned Offset = NegativeStride ? VectorWidth - 1 : 0;

  Value *NewPointer = nullptr;
  NewPointer = getNewValue(Pointer, ScalarMaps[Offset], GlobalMaps[Offset],
                           VLTS[Offset], getLoopForInst(Load));
  Value *VectorPtr =
      Builder.CreateBitCast(NewPointer, VectorPtrType, "vector_ptr");
  LoadInst *VecLoad =
      Builder.CreateLoad(VectorPtr, Load->getName() + "_p_vec_full");
  if (!Aligned)
    VecLoad->setAlignment(8);

  if (NegativeStride) {
    SmallVector<Constant *, 16> Indices;
    for (int i = VectorWidth - 1; i >= 0; i--)
      Indices.push_back(ConstantInt::get(Builder.getInt32Ty(), i));
    Constant *SV = llvm::ConstantVector::get(Indices);
    Value *RevVecLoad = Builder.CreateShuffleVector(
        VecLoad, VecLoad, SV, Load->getName() + "_reverse");
    return RevVecLoad;
  }

  return VecLoad;
}

Value *VectorBlockGenerator::generateStrideZeroLoad(const LoadInst *Load,
                                                    ValueMapT &BBMap) {
  const Value *Pointer = Load->getPointerOperand();
  Type *VectorPtrType = getVectorPtrTy(Pointer, 1);
  Value *NewPointer =
      getNewValue(Pointer, BBMap, GlobalMaps[0], VLTS[0], getLoopForInst(Load));
  Value *VectorPtr = Builder.CreateBitCast(NewPointer, VectorPtrType,
                                           Load->getName() + "_p_vec_p");
  LoadInst *ScalarLoad =
      Builder.CreateLoad(VectorPtr, Load->getName() + "_p_splat_one");

  if (!Aligned)
    ScalarLoad->setAlignment(8);

  Constant *SplatVector = Constant::getNullValue(
      VectorType::get(Builder.getInt32Ty(), getVectorWidth()));

  Value *VectorLoad = Builder.CreateShuffleVector(
      ScalarLoad, ScalarLoad, SplatVector, Load->getName() + "_p_splat");
  return VectorLoad;
}

Value *
VectorBlockGenerator::generateUnknownStrideLoad(const LoadInst *Load,
                                                VectorValueMapT &ScalarMaps) {
  int VectorWidth = getVectorWidth();
  const Value *Pointer = Load->getPointerOperand();
  VectorType *VectorType = VectorType::get(
      dyn_cast<PointerType>(Pointer->getType())->getElementType(), VectorWidth);

  Value *Vector = UndefValue::get(VectorType);

  for (int i = 0; i < VectorWidth; i++) {
    Value *NewPointer = getNewValue(Pointer, ScalarMaps[i], GlobalMaps[i],
                                    VLTS[i], getLoopForInst(Load));
    Value *ScalarLoad =
        Builder.CreateLoad(NewPointer, Load->getName() + "_p_scalar_");
    Vector = Builder.CreateInsertElement(
        Vector, ScalarLoad, Builder.getInt32(i), Load->getName() + "_p_vec_");
  }

  return Vector;
}

void VectorBlockGenerator::generateLoad(const LoadInst *Load,
                                        ValueMapT &VectorMap,
                                        VectorValueMapT &ScalarMaps) {
  if (PollyVectorizerChoice >= VECTORIZER_FIRST_NEED_GROUPED_UNROLL ||
      !VectorType::isValidElementType(Load->getType())) {
    for (int i = 0; i < getVectorWidth(); i++)
      ScalarMaps[i][Load] =
          generateScalarLoad(Load, ScalarMaps[i], GlobalMaps[i], VLTS[i]);
    return;
  }

  const MemoryAccess &Access = Statement.getAccessFor(Load);

  // Make sure we have scalar values available to access the pointer to
  // the data location.
  extractScalarValues(Load, VectorMap, ScalarMaps);

  Value *NewLoad;
  if (Access.isStrideZero(isl_map_copy(Schedule)))
    NewLoad = generateStrideZeroLoad(Load, ScalarMaps[0]);
  else if (Access.isStrideOne(isl_map_copy(Schedule)))
    NewLoad = generateStrideOneLoad(Load, ScalarMaps);
  else if (Access.isStrideX(isl_map_copy(Schedule), -1))
    NewLoad = generateStrideOneLoad(Load, ScalarMaps, true);
  else
    NewLoad = generateUnknownStrideLoad(Load, ScalarMaps);

  VectorMap[Load] = NewLoad;
}

void VectorBlockGenerator::copyUnaryInst(const UnaryInstruction *Inst,
                                         ValueMapT &VectorMap,
                                         VectorValueMapT &ScalarMaps) {
  int VectorWidth = getVectorWidth();
  Value *NewOperand = getVectorValue(Inst->getOperand(0), VectorMap, ScalarMaps,
                                     getLoopForInst(Inst));

  assert(isa<CastInst>(Inst) && "Can not generate vector code for instruction");

  const CastInst *Cast = dyn_cast<CastInst>(Inst);
  VectorType *DestType = VectorType::get(Inst->getType(), VectorWidth);
  VectorMap[Inst] = Builder.CreateCast(Cast->getOpcode(), NewOperand, DestType);
}

void VectorBlockGenerator::copyBinaryInst(const BinaryOperator *Inst,
                                          ValueMapT &VectorMap,
                                          VectorValueMapT &ScalarMaps) {
  Loop *L = getLoopForInst(Inst);
  Value *OpZero = Inst->getOperand(0);
  Value *OpOne = Inst->getOperand(1);

  Value *NewOpZero, *NewOpOne;
  NewOpZero = getVectorValue(OpZero, VectorMap, ScalarMaps, L);
  NewOpOne = getVectorValue(OpOne, VectorMap, ScalarMaps, L);

  Value *NewInst = Builder.CreateBinOp(Inst->getOpcode(), NewOpZero, NewOpOne,
                                       Inst->getName() + "p_vec");
  VectorMap[Inst] = NewInst;
}

void VectorBlockGenerator::copyStore(const StoreInst *Store,
                                     ValueMapT &VectorMap,
                                     VectorValueMapT &ScalarMaps) {
  int VectorWidth = getVectorWidth();

  const MemoryAccess &Access = Statement.getAccessFor(Store);

  const Value *Pointer = Store->getPointerOperand();
  Value *Vector = getVectorValue(Store->getValueOperand(), VectorMap,
                                 ScalarMaps, getLoopForInst(Store));

  // Make sure we have scalar values available to access the pointer to
  // the data location.
  extractScalarValues(Store, VectorMap, ScalarMaps);

  if (Access.isStrideOne(isl_map_copy(Schedule))) {
    Type *VectorPtrType = getVectorPtrTy(Pointer, VectorWidth);
    Value *NewPointer = getNewValue(Pointer, ScalarMaps[0], GlobalMaps[0],
                                    VLTS[0], getLoopForInst(Store));

    Value *VectorPtr =
        Builder.CreateBitCast(NewPointer, VectorPtrType, "vector_ptr");
    StoreInst *Store = Builder.CreateStore(Vector, VectorPtr);

    if (!Aligned)
      Store->setAlignment(8);
  } else {
    for (unsigned i = 0; i < ScalarMaps.size(); i++) {
      Value *Scalar = Builder.CreateExtractElement(Vector, Builder.getInt32(i));
      Value *NewPointer = getNewValue(Pointer, ScalarMaps[i], GlobalMaps[i],
                                      VLTS[i], getLoopForInst(Store));
      Builder.CreateStore(Scalar, NewPointer);
    }
  }
}

bool VectorBlockGenerator::hasVectorOperands(const Instruction *Inst,
                                             ValueMapT &VectorMap) {
  for (Value *Operand : Inst->operands())
    if (VectorMap.count(Operand))
      return true;
  return false;
}

bool VectorBlockGenerator::extractScalarValues(const Instruction *Inst,
                                               ValueMapT &VectorMap,
                                               VectorValueMapT &ScalarMaps) {
  bool HasVectorOperand = false;
  int VectorWidth = getVectorWidth();

  for (Value *Operand : Inst->operands()) {
    ValueMapT::iterator VecOp = VectorMap.find(Operand);

    if (VecOp == VectorMap.end())
      continue;

    HasVectorOperand = true;
    Value *NewVector = VecOp->second;

    for (int i = 0; i < VectorWidth; ++i) {
      ValueMapT &SM = ScalarMaps[i];

      // If there is one scalar extracted, all scalar elements should have
      // already been extracted by the code here. So no need to check for the
      // existance of all of them.
      if (SM.count(Operand))
        break;

      SM[Operand] =
          Builder.CreateExtractElement(NewVector, Builder.getInt32(i));
    }
  }

  return HasVectorOperand;
}

void VectorBlockGenerator::copyInstScalarized(const Instruction *Inst,
                                              ValueMapT &VectorMap,
                                              VectorValueMapT &ScalarMaps) {
  bool HasVectorOperand;
  int VectorWidth = getVectorWidth();

  HasVectorOperand = extractScalarValues(Inst, VectorMap, ScalarMaps);

  for (int VectorLane = 0; VectorLane < getVectorWidth(); VectorLane++)
    copyInstScalar(Inst, ScalarMaps[VectorLane], GlobalMaps[VectorLane],
                   VLTS[VectorLane]);

  if (!VectorType::isValidElementType(Inst->getType()) || !HasVectorOperand)
    return;

  // Make the result available as vector value.
  VectorType *VectorType = VectorType::get(Inst->getType(), VectorWidth);
  Value *Vector = UndefValue::get(VectorType);

  for (int i = 0; i < VectorWidth; i++)
    Vector = Builder.CreateInsertElement(Vector, ScalarMaps[i][Inst],
                                         Builder.getInt32(i));

  VectorMap[Inst] = Vector;
}

int VectorBlockGenerator::getVectorWidth() { return GlobalMaps.size(); }

void VectorBlockGenerator::copyInstruction(const Instruction *Inst,
                                           ValueMapT &VectorMap,
                                           VectorValueMapT &ScalarMaps) {
  // Terminator instructions control the control flow. They are explicitly
  // expressed in the clast and do not need to be copied.
  if (Inst->isTerminator())
    return;

  if (canSynthesize(Inst, &P->getAnalysis<LoopInfo>(), &SE,
                    &Statement.getParent()->getRegion()))
    return;

  if (const LoadInst *Load = dyn_cast<LoadInst>(Inst)) {
    generateLoad(Load, VectorMap, ScalarMaps);
    return;
  }

  if (hasVectorOperands(Inst, VectorMap)) {
    if (const StoreInst *Store = dyn_cast<StoreInst>(Inst)) {
      copyStore(Store, VectorMap, ScalarMaps);
      return;
    }

    if (const UnaryInstruction *Unary = dyn_cast<UnaryInstruction>(Inst)) {
      copyUnaryInst(Unary, VectorMap, ScalarMaps);
      return;
    }

    if (const BinaryOperator *Binary = dyn_cast<BinaryOperator>(Inst)) {
      copyBinaryInst(Binary, VectorMap, ScalarMaps);
      return;
    }

    // Falltrough: We generate scalar instructions, if we don't know how to
    // generate vector code.
  }

  copyInstScalarized(Inst, VectorMap, ScalarMaps);
}

void VectorBlockGenerator::copyBB() {
  BasicBlock *BB = Statement.getBasicBlock();
  BasicBlock *CopyBB =
      SplitBlock(Builder.GetInsertBlock(), Builder.GetInsertPoint(), P);
  CopyBB->setName("polly.stmt." + BB->getName());
  Builder.SetInsertPoint(CopyBB->begin());

  // Create two maps that store the mapping from the original instructions of
  // the old basic block to their copies in the new basic block. Those maps
  // are basic block local.
  //
  // As vector code generation is supported there is one map for scalar values
  // and one for vector values.
  //
  // In case we just do scalar code generation, the vectorMap is not used and
  // the scalarMap has just one dimension, which contains the mapping.
  //
  // In case vector code generation is done, an instruction may either appear
  // in the vector map once (as it is calculating >vectorwidth< values at a
  // time. Or (if the values are calculated using scalar operations), it
  // appears once in every dimension of the scalarMap.
  VectorValueMapT ScalarBlockMap(getVectorWidth());
  ValueMapT VectorBlockMap;

  for (Instruction &Inst : *BB)
    copyInstruction(&Inst, VectorBlockMap, ScalarBlockMap);
}
