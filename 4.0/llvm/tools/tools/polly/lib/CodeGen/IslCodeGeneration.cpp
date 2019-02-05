//===------ IslCodeGeneration.cpp - Code generate the Scops using ISL. ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// The IslCodeGeneration pass takes a Scop created by ScopInfo and translates it
// back to LLVM-IR using the ISL code generator.
//
// The Scop describes the high level memory behaviour of a control flow region.
// Transformation passes can update the schedule (execution order) of statements
// in the Scop. ISL is used to generate an abstract syntax tree that reflects
// the updated execution order. This clast is used to create new LLVM-IR that is
// computationally equivalent to the original control flow region, but executes
// its code in the new execution order defined by the changed scattering.
//
//===----------------------------------------------------------------------===//
#include "polly/Config/config.h"
#include "polly/CodeGen/BlockGenerators.h"
#include "polly/CodeGen/CodeGeneration.h"
#include "polly/CodeGen/IslAst.h"
#include "polly/CodeGen/LoopGenerators.h"
#include "polly/CodeGen/Utils.h"
#include "polly/Dependences.h"
#include "polly/LinkAllPasses.h"
#include "polly/ScopInfo.h"
#include "polly/Support/GICHelper.h"
#include "polly/Support/ScopHelper.h"
#include "polly/TempScopInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "isl/union_map.h"
#include "isl/list.h"
#include "isl/ast.h"
#include "isl/ast_build.h"
#include "isl/set.h"
#include "isl/map.h"
#include "isl/aff.h"

#include <map>

using namespace polly;
using namespace llvm;

#define DEBUG_TYPE "polly-codegen-isl"

/// @brief Insert function calls that print certain LLVM values at run time.
///
/// This class inserts libc function calls to print certain LLVM values at
/// run time.
class RuntimeDebugBuilder {
public:
  RuntimeDebugBuilder(PollyIRBuilder &Builder) : Builder(Builder) {}

  /// @brief Print a string to stdout.
  ///
  /// @param String The string to print.
  void createStrPrinter(std::string String);

  /// @brief Print an integer value to stdout.
  ///
  /// @param V The value to print.
  void createIntPrinter(Value *V);

private:
  PollyIRBuilder &Builder;

  /// @brief Add a call to the fflush function with no file pointer given.
  ///
  /// This call will flush all opened file pointers including stdout and stderr.
  void createFlush();

  /// @brief Get a reference to the 'printf' function.
  ///
  /// If the current module does not yet contain a reference to printf, we
  /// insert a reference to it. Otherwise the existing reference is returned.
  Function *getPrintF();
};

Function *RuntimeDebugBuilder::getPrintF() {
  Module *M = Builder.GetInsertBlock()->getParent()->getParent();
  const char *Name = "printf";
  Function *F = M->getFunction(Name);

  if (!F) {
    GlobalValue::LinkageTypes Linkage = Function::ExternalLinkage;
    FunctionType *Ty =
        FunctionType::get(Builder.getInt32Ty(), Builder.getInt8PtrTy(), true);
    F = Function::Create(Ty, Linkage, Name, M);
  }

  return F;
}

void RuntimeDebugBuilder::createFlush() {
  Module *M = Builder.GetInsertBlock()->getParent()->getParent();
  const char *Name = "fflush";
  Function *F = M->getFunction(Name);

  if (!F) {
    GlobalValue::LinkageTypes Linkage = Function::ExternalLinkage;
    FunctionType *Ty =
        FunctionType::get(Builder.getInt32Ty(), Builder.getInt8PtrTy(), false);
    F = Function::Create(Ty, Linkage, Name, M);
  }

  Builder.CreateCall(F, Constant::getNullValue(Builder.getInt8PtrTy()));
}

void RuntimeDebugBuilder::createStrPrinter(std::string String) {
  Function *F = getPrintF();
  Value *StringValue = Builder.CreateGlobalStringPtr(String);
  Builder.CreateCall(F, StringValue);

  createFlush();
}

void RuntimeDebugBuilder::createIntPrinter(Value *V) {
  IntegerType *Ty = dyn_cast<IntegerType>(V->getType());
  (void)Ty;
  assert(Ty && Ty->getBitWidth() == 64 &&
         "Cannot insert printer for this type.");

  Function *F = getPrintF();
  Value *String = Builder.CreateGlobalStringPtr("%ld");
  Builder.CreateCall2(F, String, V);
  createFlush();
}

/// @brief LLVM-IR generator for isl_ast_expr[essions]
///
/// This generator generates LLVM-IR that performs the computation described by
/// an isl_ast_expr[ession].
///
/// Example:
///
///   An isl_ast_expr[ession] can look like this:
///
///     (N + M) + 10
///
///   The IslExprBuilder could create the following LLVM-IR:
///
///     %tmp1 = add nsw i64 %N
///     %tmp2 = add nsw i64 %tmp1, %M
///     %tmp3 = add nsw i64 %tmp2, 10
///
/// The implementation of this class is mostly a mapping from isl_ast_expr
/// constructs to the corresponding LLVM-IR constructs.
///
/// The following decisions may need some explanation:
///
/// 1) Which data-type to choose
///
/// isl_ast_expr[essions] are untyped expressions that assume arbitrary
/// precision integer computations. LLVM-IR instead has fixed size integers.
/// When lowering to LLVM-IR we need to chose both the size of the data type and
/// the sign of the operations we use.
///
/// At the moment, we hardcode i64 bit signed computations. Our experience has
/// shown that 64 bit are generally large enough for the loop bounds that appear
/// in the wild. Signed computations are needed, as loop bounds may become
/// negative.
///
/// FIXME: Hardcoding sizes can cause issues:
///
///   a) Certain run-time checks that we may want to generate can involve the
///      size of the data types the computation is performed on. When code
///      generating these run-time checks to isl_ast_expr[essions], the
///      resulting computation may require more than 64 bit.
///
///   b) On embedded systems and especially for high-level-synthesis 64 bit
///      computations are very costly.
///
///   The right approach is to compute the minimal necessary bitwidth and
///   signedness for each subexpression during in the isl AST generation and
///   to use this information in our IslAstGenerator. Preliminary patches are
///   available, but have not been committed yet.
///
/// 2) We always flag computations with 'nsw'
///
/// As isl_ast_expr[essions] assume arbitrary precision, no wrapping should
/// ever occur in the generated LLVM-IR (assuming the data type chosen is large
/// enough).
class IslExprBuilder {
public:
  /// @brief Construct an IslExprBuilder.
  ///
  /// @param Builder The IRBuilder used to construct the isl_ast_expr[ession].
  ///                The insert location of this IRBuilder defines WHERE the
  ///                corresponding LLVM-IR is generated.
  ///
  /// @param IDToValue The isl_ast_expr[ession] may reference parameters or
  ///                  variables (identified by an isl_id). The IDTOValue map
  ///                  specifies the LLVM-IR Values that correspond to these
  ///                  parameters and variables.
  IslExprBuilder(PollyIRBuilder &Builder,
                 std::map<isl_id *, Value *> &IDToValue)
      : Builder(Builder), IDToValue(IDToValue) {}

  /// @brief Create LLVM-IR for an isl_ast_expr[ession].
  ///
  /// @param Expr The ast expression for which we generate LLVM-IR.
  ///
  /// @return The llvm::Value* containing the result of the computation.
  Value *create(__isl_take isl_ast_expr *Expr);

  /// @brief Return the largest of two types.
  ///
  /// @param T1 The first type.
  /// @param T2 The second type.
  ///
  /// @return The largest of the two types.
  Type *getWidestType(Type *T1, Type *T2);

  /// @brief Return the type with which this expression should be computed.
  ///
  /// The type needs to be large enough to hold all possible input and all
  /// possible output values.
  ///
  /// @param Expr The expression for which to find the type.
  /// @return The type with which the expression should be computed.
  IntegerType *getType(__isl_keep isl_ast_expr *Expr);

private:
  PollyIRBuilder &Builder;
  std::map<isl_id *, Value *> &IDToValue;

  Value *createOp(__isl_take isl_ast_expr *Expr);
  Value *createOpUnary(__isl_take isl_ast_expr *Expr);
  Value *createOpBin(__isl_take isl_ast_expr *Expr);
  Value *createOpNAry(__isl_take isl_ast_expr *Expr);
  Value *createOpSelect(__isl_take isl_ast_expr *Expr);
  Value *createOpICmp(__isl_take isl_ast_expr *Expr);
  Value *createOpBoolean(__isl_take isl_ast_expr *Expr);
  Value *createId(__isl_take isl_ast_expr *Expr);
  Value *createInt(__isl_take isl_ast_expr *Expr);
};

Type *IslExprBuilder::getWidestType(Type *T1, Type *T2) {
  assert(isa<IntegerType>(T1) && isa<IntegerType>(T2));

  if (T1->getPrimitiveSizeInBits() < T2->getPrimitiveSizeInBits())
    return T2;
  else
    return T1;
}

Value *IslExprBuilder::createOpUnary(__isl_take isl_ast_expr *Expr) {
  assert(isl_ast_expr_get_op_type(Expr) == isl_ast_op_minus &&
         "Unsupported unary operation");

  Value *V;
  Type *MaxType = getType(Expr);

  V = create(isl_ast_expr_get_op_arg(Expr, 0));
  MaxType = getWidestType(MaxType, V->getType());

  if (MaxType != V->getType())
    V = Builder.CreateSExt(V, MaxType);

  isl_ast_expr_free(Expr);
  return Builder.CreateNSWNeg(V);
}

Value *IslExprBuilder::createOpNAry(__isl_take isl_ast_expr *Expr) {
  assert(isl_ast_expr_get_type(Expr) == isl_ast_expr_op &&
         "isl ast expression not of type isl_ast_op");
  assert(isl_ast_expr_get_op_n_arg(Expr) >= 2 &&
         "We need at least two operands in an n-ary operation");

  Value *V;

  V = create(isl_ast_expr_get_op_arg(Expr, 0));

  for (int i = 0; i < isl_ast_expr_get_op_n_arg(Expr); ++i) {
    Value *OpV;
    OpV = create(isl_ast_expr_get_op_arg(Expr, i));

    Type *Ty = getWidestType(V->getType(), OpV->getType());

    if (Ty != OpV->getType())
      OpV = Builder.CreateSExt(OpV, Ty);

    if (Ty != V->getType())
      V = Builder.CreateSExt(V, Ty);

    switch (isl_ast_expr_get_op_type(Expr)) {
    default:
      llvm_unreachable("This is no n-ary isl ast expression");

    case isl_ast_op_max: {
      Value *Cmp = Builder.CreateICmpSGT(V, OpV);
      V = Builder.CreateSelect(Cmp, V, OpV);
      continue;
    }
    case isl_ast_op_min: {
      Value *Cmp = Builder.CreateICmpSLT(V, OpV);
      V = Builder.CreateSelect(Cmp, V, OpV);
      continue;
    }
    }
  }

  // TODO: We can truncate the result, if it fits into a smaller type. This can
  // help in cases where we have larger operands (e.g. i67) but the result is
  // known to fit into i64. Without the truncation, the larger i67 type may
  // force all subsequent operations to be performed on a non-native type.
  isl_ast_expr_free(Expr);
  return V;
}

Value *IslExprBuilder::createOpBin(__isl_take isl_ast_expr *Expr) {
  Value *LHS, *RHS, *Res;
  Type *MaxType;
  isl_ast_op_type OpType;

  assert(isl_ast_expr_get_type(Expr) == isl_ast_expr_op &&
         "isl ast expression not of type isl_ast_op");
  assert(isl_ast_expr_get_op_n_arg(Expr) == 2 &&
         "not a binary isl ast expression");

  OpType = isl_ast_expr_get_op_type(Expr);

  LHS = create(isl_ast_expr_get_op_arg(Expr, 0));
  RHS = create(isl_ast_expr_get_op_arg(Expr, 1));

  MaxType = LHS->getType();
  MaxType = getWidestType(MaxType, RHS->getType());

  // Take the result into account when calculating the widest type.
  //
  // For operations such as '+' the result may require a type larger than
  // the type of the individual operands. For other operations such as '/', the
  // result type cannot be larger than the type of the individual operand. isl
  // does not calculate correct types for these operations and we consequently
  // exclude those operations here.
  switch (OpType) {
  case isl_ast_op_pdiv_q:
  case isl_ast_op_pdiv_r:
  case isl_ast_op_div:
  case isl_ast_op_fdiv_q:
    // Do nothing
    break;
  case isl_ast_op_add:
  case isl_ast_op_sub:
  case isl_ast_op_mul:
    MaxType = getWidestType(MaxType, getType(Expr));
    break;
  default:
    llvm_unreachable("This is no binary isl ast expression");
  }

  if (MaxType != RHS->getType())
    RHS = Builder.CreateSExt(RHS, MaxType);

  if (MaxType != LHS->getType())
    LHS = Builder.CreateSExt(LHS, MaxType);

  switch (OpType) {
  default:
    llvm_unreachable("This is no binary isl ast expression");
  case isl_ast_op_add:
    Res = Builder.CreateNSWAdd(LHS, RHS);
    break;
  case isl_ast_op_sub:
    Res = Builder.CreateNSWSub(LHS, RHS);
    break;
  case isl_ast_op_mul:
    Res = Builder.CreateNSWMul(LHS, RHS);
    break;
  case isl_ast_op_div:
  case isl_ast_op_pdiv_q: // Dividend is non-negative
    Res = Builder.CreateSDiv(LHS, RHS);
    break;
  case isl_ast_op_fdiv_q: { // Round towards -infty
    // TODO: Review code and check that this calculation does not yield
    //       incorrect overflow in some bordercases.
    //
    // floord(n,d) ((n < 0) ? (n - d + 1) : n) / d
    Value *One = ConstantInt::get(MaxType, 1);
    Value *Zero = ConstantInt::get(MaxType, 0);
    Value *Sum1 = Builder.CreateSub(LHS, RHS);
    Value *Sum2 = Builder.CreateAdd(Sum1, One);
    Value *isNegative = Builder.CreateICmpSLT(LHS, Zero);
    Value *Dividend = Builder.CreateSelect(isNegative, Sum2, LHS);
    Res = Builder.CreateSDiv(Dividend, RHS);
    break;
  }
  case isl_ast_op_pdiv_r: // Dividend is non-negative
    Res = Builder.CreateSRem(LHS, RHS);
    break;
  }

  // TODO: We can truncate the result, if it fits into a smaller type. This can
  // help in cases where we have larger operands (e.g. i67) but the result is
  // known to fit into i64. Without the truncation, the larger i67 type may
  // force all subsequent operations to be performed on a non-native type.
  isl_ast_expr_free(Expr);
  return Res;
}

Value *IslExprBuilder::createOpSelect(__isl_take isl_ast_expr *Expr) {
  assert(isl_ast_expr_get_op_type(Expr) == isl_ast_op_select &&
         "Unsupported unary isl ast expression");
  Value *LHS, *RHS, *Cond;
  Type *MaxType = getType(Expr);

  Cond = create(isl_ast_expr_get_op_arg(Expr, 0));

  LHS = create(isl_ast_expr_get_op_arg(Expr, 1));
  RHS = create(isl_ast_expr_get_op_arg(Expr, 2));

  MaxType = getWidestType(MaxType, LHS->getType());
  MaxType = getWidestType(MaxType, RHS->getType());

  if (MaxType != RHS->getType())
    RHS = Builder.CreateSExt(RHS, MaxType);

  if (MaxType != LHS->getType())
    LHS = Builder.CreateSExt(LHS, MaxType);

  // TODO: Do we want to truncate the result?
  isl_ast_expr_free(Expr);
  return Builder.CreateSelect(Cond, LHS, RHS);
}

Value *IslExprBuilder::createOpICmp(__isl_take isl_ast_expr *Expr) {
  assert(isl_ast_expr_get_type(Expr) == isl_ast_expr_op &&
         "Expected an isl_ast_expr_op expression");

  Value *LHS, *RHS, *Res;

  LHS = create(isl_ast_expr_get_op_arg(Expr, 0));
  RHS = create(isl_ast_expr_get_op_arg(Expr, 1));

  Type *MaxType = LHS->getType();
  MaxType = getWidestType(MaxType, RHS->getType());

  if (MaxType != RHS->getType())
    RHS = Builder.CreateSExt(RHS, MaxType);

  if (MaxType != LHS->getType())
    LHS = Builder.CreateSExt(LHS, MaxType);

  switch (isl_ast_expr_get_op_type(Expr)) {
  default:
    llvm_unreachable("Unsupported ICmp isl ast expression");
  case isl_ast_op_eq:
    Res = Builder.CreateICmpEQ(LHS, RHS);
    break;
  case isl_ast_op_le:
    Res = Builder.CreateICmpSLE(LHS, RHS);
    break;
  case isl_ast_op_lt:
    Res = Builder.CreateICmpSLT(LHS, RHS);
    break;
  case isl_ast_op_ge:
    Res = Builder.CreateICmpSGE(LHS, RHS);
    break;
  case isl_ast_op_gt:
    Res = Builder.CreateICmpSGT(LHS, RHS);
    break;
  }

  isl_ast_expr_free(Expr);
  return Res;
}

Value *IslExprBuilder::createOpBoolean(__isl_take isl_ast_expr *Expr) {
  assert(isl_ast_expr_get_type(Expr) == isl_ast_expr_op &&
         "Expected an isl_ast_expr_op expression");

  Value *LHS, *RHS, *Res;
  isl_ast_op_type OpType;

  OpType = isl_ast_expr_get_op_type(Expr);

  assert((OpType == isl_ast_op_and || OpType == isl_ast_op_or) &&
         "Unsupported isl_ast_op_type");

  LHS = create(isl_ast_expr_get_op_arg(Expr, 0));
  RHS = create(isl_ast_expr_get_op_arg(Expr, 1));

  // Even though the isl pretty printer prints the expressions as 'exp && exp'
  // or 'exp || exp', we actually code generate the bitwise expressions
  // 'exp & exp' or 'exp | exp'. This forces the evaluation of both branches,
  // but it is, due to the use of i1 types, otherwise equivalent. The reason
  // to go for bitwise operations is, that we assume the reduced control flow
  // will outweight the overhead introduced by evaluating unneeded expressions.
  // The isl code generation currently does not take advantage of the fact that
  // the expression after an '||' or '&&' is in some cases not evaluated.
  // Evaluating it anyways does not cause any undefined behaviour.
  //
  // TODO: Document in isl itself, that the unconditionally evaluating the
  // second part of '||' or '&&' expressions is safe.
  assert(LHS->getType() == Builder.getInt1Ty() && "Expected i1 type");
  assert(RHS->getType() == Builder.getInt1Ty() && "Expected i1 type");

  switch (OpType) {
  default:
    llvm_unreachable("Unsupported boolean expression");
  case isl_ast_op_and:
    Res = Builder.CreateAnd(LHS, RHS);
    break;
  case isl_ast_op_or:
    Res = Builder.CreateOr(LHS, RHS);
    break;
  }

  isl_ast_expr_free(Expr);
  return Res;
}

Value *IslExprBuilder::createOp(__isl_take isl_ast_expr *Expr) {
  assert(isl_ast_expr_get_type(Expr) == isl_ast_expr_op &&
         "Expression not of type isl_ast_expr_op");
  switch (isl_ast_expr_get_op_type(Expr)) {
  case isl_ast_op_error:
  case isl_ast_op_cond:
  case isl_ast_op_and_then:
  case isl_ast_op_or_else:
  case isl_ast_op_call:
  case isl_ast_op_member:
  case isl_ast_op_access:
    llvm_unreachable("Unsupported isl ast expression");
  case isl_ast_op_max:
  case isl_ast_op_min:
    return createOpNAry(Expr);
  case isl_ast_op_add:
  case isl_ast_op_sub:
  case isl_ast_op_mul:
  case isl_ast_op_div:
  case isl_ast_op_fdiv_q: // Round towards -infty
  case isl_ast_op_pdiv_q: // Dividend is non-negative
  case isl_ast_op_pdiv_r: // Dividend is non-negative
    return createOpBin(Expr);
  case isl_ast_op_minus:
    return createOpUnary(Expr);
  case isl_ast_op_select:
    return createOpSelect(Expr);
  case isl_ast_op_and:
  case isl_ast_op_or:
    return createOpBoolean(Expr);
  case isl_ast_op_eq:
  case isl_ast_op_le:
  case isl_ast_op_lt:
  case isl_ast_op_ge:
  case isl_ast_op_gt:
    return createOpICmp(Expr);
  }

  llvm_unreachable("Unsupported isl_ast_expr_op kind.");
}

Value *IslExprBuilder::createId(__isl_take isl_ast_expr *Expr) {
  assert(isl_ast_expr_get_type(Expr) == isl_ast_expr_id &&
         "Expression not of type isl_ast_expr_ident");

  isl_id *Id;
  Value *V;

  Id = isl_ast_expr_get_id(Expr);

  assert(IDToValue.count(Id) && "Identifier not found");

  V = IDToValue[Id];

  isl_id_free(Id);
  isl_ast_expr_free(Expr);

  return V;
}

IntegerType *IslExprBuilder::getType(__isl_keep isl_ast_expr *Expr) {
  // XXX: We assume i64 is large enough. This is often true, but in general
  //      incorrect. Also, on 32bit architectures, it would be beneficial to
  //      use a smaller type. We can and should directly derive this information
  //      during code generation.
  return IntegerType::get(Builder.getContext(), 64);
}

Value *IslExprBuilder::createInt(__isl_take isl_ast_expr *Expr) {
  assert(isl_ast_expr_get_type(Expr) == isl_ast_expr_int &&
         "Expression not of type isl_ast_expr_int");
  isl_val *Val;
  Value *V;
  APInt APValue;
  IntegerType *T;

  Val = isl_ast_expr_get_val(Expr);
  APValue = APIntFromVal(Val);
  T = getType(Expr);
  APValue = APValue.sextOrSelf(T->getBitWidth());
  V = ConstantInt::get(T, APValue);

  isl_ast_expr_free(Expr);
  return V;
}

Value *IslExprBuilder::create(__isl_take isl_ast_expr *Expr) {
  switch (isl_ast_expr_get_type(Expr)) {
  case isl_ast_expr_error:
    llvm_unreachable("Code generation error");
  case isl_ast_expr_op:
    return createOp(Expr);
  case isl_ast_expr_id:
    return createId(Expr);
  case isl_ast_expr_int:
    return createInt(Expr);
  }

  llvm_unreachable("Unexpected enum value");
}

class IslNodeBuilder {
public:
  IslNodeBuilder(PollyIRBuilder &Builder, LoopAnnotator &Annotator, Pass *P)
      : Builder(Builder), Annotator(Annotator), ExprBuilder(Builder, IDToValue),
        P(P) {}

  void addParameters(__isl_take isl_set *Context);
  void create(__isl_take isl_ast_node *Node);
  IslExprBuilder &getExprBuilder() { return ExprBuilder; }

private:
  PollyIRBuilder &Builder;
  LoopAnnotator &Annotator;
  IslExprBuilder ExprBuilder;
  Pass *P;

  // This maps an isl_id* to the Value* it has in the generated program. For now
  // on, the only isl_ids that are stored here are the newly calculated loop
  // ivs.
  std::map<isl_id *, Value *> IDToValue;

  // Extract the upper bound of this loop
  //
  // The isl code generation can generate arbitrary expressions to check if the
  // upper bound of a loop is reached, but it provides an option to enforce
  // 'atomic' upper bounds. An 'atomic upper bound is always of the form
  // iv <= expr, where expr is an (arbitrary) expression not containing iv.
  //
  // This function extracts 'atomic' upper bounds. Polly, in general, requires
  // atomic upper bounds for the following reasons:
  //
  // 1. An atomic upper bound is loop invariant
  //
  //    It must not be calculated at each loop iteration and can often even be
  //    hoisted out further by the loop invariant code motion.
  //
  // 2. OpenMP needs a loop invarient upper bound to calculate the number
  //    of loop iterations.
  //
  // 3. With the existing code, upper bounds have been easier to implement.
  __isl_give isl_ast_expr *getUpperBound(__isl_keep isl_ast_node *For,
                                         CmpInst::Predicate &Predicate);

  unsigned getNumberOfIterations(__isl_keep isl_ast_node *For);

  void createFor(__isl_take isl_ast_node *For);
  void createForVector(__isl_take isl_ast_node *For, int VectorWidth);
  void createForSequential(__isl_take isl_ast_node *For);

  /// Generate LLVM-IR that computes the values of the original induction
  /// variables in function of the newly generated loop induction variables.
  ///
  /// Example:
  ///
  ///   // Original
  ///   for i
  ///     for j
  ///       S(i)
  ///
  ///   Schedule: [i,j] -> [i+j, j]
  ///
  ///   // New
  ///   for c0
  ///     for c1
  ///       S(c0 - c1, c1)
  ///
  /// Assuming the original code consists of two loops which are
  /// transformed according to a schedule [i,j] -> [c0=i+j,c1=j]. The resulting
  /// ast models the original statement as a call expression where each argument
  /// is an expression that computes the old induction variables from the new
  /// ones, ordered such that the first argument computes the value of induction
  /// variable that was outermost in the original code.
  ///
  /// @param Expr The call expression that represents the statement.
  /// @param Stmt The statement that is called.
  /// @param VMap The value map into which the mapping from the old induction
  ///             variable to the new one is inserted. This mapping is used
  ///             for the classical code generation (not scev-based) and
  ///             gives an explicit mapping from an original, materialized
  ///             induction variable. It consequently can only be expressed
  ///             if there was an explicit induction variable.
  /// @param LTS  The loop to SCEV map in which the mapping from the original
  ///             loop to a SCEV representing the new loop iv is added. This
  ///             mapping does not require an explicit induction variable.
  ///             Instead, we think in terms of an implicit induction variable
  ///             that counts the number of times a loop is executed. For each
  ///             original loop this count, expressed in function of the new
  ///             induction variables, is added to the LTS map.
  void createSubstitutions(__isl_take isl_ast_expr *Expr, ScopStmt *Stmt,
                           ValueMapT &VMap, LoopToScevMapT &LTS);
  void createSubstitutionsVector(__isl_take isl_ast_expr *Expr, ScopStmt *Stmt,
                                 VectorValueMapT &VMap,
                                 std::vector<LoopToScevMapT> &VLTS,
                                 std::vector<Value *> &IVS,
                                 __isl_take isl_id *IteratorID);
  void createIf(__isl_take isl_ast_node *If);
  void createUserVector(__isl_take isl_ast_node *User,
                        std::vector<Value *> &IVS,
                        __isl_take isl_id *IteratorID,
                        __isl_take isl_union_map *Schedule);
  void createUser(__isl_take isl_ast_node *User);
  void createBlock(__isl_take isl_ast_node *Block);
};

__isl_give isl_ast_expr *
IslNodeBuilder::getUpperBound(__isl_keep isl_ast_node *For,
                              ICmpInst::Predicate &Predicate) {
  isl_id *UBID, *IteratorID;
  isl_ast_expr *Cond, *Iterator, *UB, *Arg0;
  isl_ast_op_type Type;

  Cond = isl_ast_node_for_get_cond(For);
  Iterator = isl_ast_node_for_get_iterator(For);
  Type = isl_ast_expr_get_op_type(Cond);

  assert(isl_ast_expr_get_type(Cond) == isl_ast_expr_op &&
         "conditional expression is not an atomic upper bound");

  switch (Type) {
  case isl_ast_op_le:
    Predicate = ICmpInst::ICMP_SLE;
    break;
  case isl_ast_op_lt:
    Predicate = ICmpInst::ICMP_SLT;
    break;
  default:
    llvm_unreachable("Unexpected comparision type in loop conditon");
  }

  Arg0 = isl_ast_expr_get_op_arg(Cond, 0);

  assert(isl_ast_expr_get_type(Arg0) == isl_ast_expr_id &&
         "conditional expression is not an atomic upper bound");

  UBID = isl_ast_expr_get_id(Arg0);

  assert(isl_ast_expr_get_type(Iterator) == isl_ast_expr_id &&
         "Could not get the iterator");

  IteratorID = isl_ast_expr_get_id(Iterator);

  assert(UBID == IteratorID &&
         "conditional expression is not an atomic upper bound");

  UB = isl_ast_expr_get_op_arg(Cond, 1);

  isl_ast_expr_free(Cond);
  isl_ast_expr_free(Iterator);
  isl_ast_expr_free(Arg0);
  isl_id_free(IteratorID);
  isl_id_free(UBID);

  return UB;
}

unsigned IslNodeBuilder::getNumberOfIterations(__isl_keep isl_ast_node *For) {
  isl_id *Annotation = isl_ast_node_get_annotation(For);
  if (!Annotation)
    return -1;

  struct IslAstUserPayload *Info =
      (struct IslAstUserPayload *)isl_id_get_user(Annotation);
  if (!Info) {
    isl_id_free(Annotation);
    return -1;
  }

  isl_union_map *Schedule = isl_ast_build_get_schedule(Info->Context);
  isl_set *LoopDomain = isl_set_from_union_set(isl_union_map_range(Schedule));
  isl_id_free(Annotation);
  int NumberOfIterations = polly::getNumberOfIterations(LoopDomain);
  if (NumberOfIterations == -1)
    return -1;
  return NumberOfIterations + 1;
}

void IslNodeBuilder::createUserVector(__isl_take isl_ast_node *User,
                                      std::vector<Value *> &IVS,
                                      __isl_take isl_id *IteratorID,
                                      __isl_take isl_union_map *Schedule) {
  isl_ast_expr *Expr = isl_ast_node_user_get_expr(User);
  isl_ast_expr *StmtExpr = isl_ast_expr_get_op_arg(Expr, 0);
  isl_id *Id = isl_ast_expr_get_id(StmtExpr);
  isl_ast_expr_free(StmtExpr);
  ScopStmt *Stmt = (ScopStmt *)isl_id_get_user(Id);
  VectorValueMapT VectorMap(IVS.size());
  std::vector<LoopToScevMapT> VLTS(IVS.size());

  isl_union_set *Domain = isl_union_set_from_set(Stmt->getDomain());
  Schedule = isl_union_map_intersect_domain(Schedule, Domain);
  isl_map *S = isl_map_from_union_map(Schedule);

  createSubstitutionsVector(Expr, Stmt, VectorMap, VLTS, IVS, IteratorID);
  VectorBlockGenerator::generate(Builder, *Stmt, VectorMap, VLTS, S, P);

  isl_map_free(S);
  isl_id_free(Id);
  isl_ast_node_free(User);
}

void IslNodeBuilder::createForVector(__isl_take isl_ast_node *For,
                                     int VectorWidth) {
  isl_ast_node *Body = isl_ast_node_for_get_body(For);
  isl_ast_expr *Init = isl_ast_node_for_get_init(For);
  isl_ast_expr *Inc = isl_ast_node_for_get_inc(For);
  isl_ast_expr *Iterator = isl_ast_node_for_get_iterator(For);
  isl_id *IteratorID = isl_ast_expr_get_id(Iterator);

  Value *ValueLB = ExprBuilder.create(Init);
  Value *ValueInc = ExprBuilder.create(Inc);

  Type *MaxType = ExprBuilder.getType(Iterator);
  MaxType = ExprBuilder.getWidestType(MaxType, ValueLB->getType());
  MaxType = ExprBuilder.getWidestType(MaxType, ValueInc->getType());

  if (MaxType != ValueLB->getType())
    ValueLB = Builder.CreateSExt(ValueLB, MaxType);
  if (MaxType != ValueInc->getType())
    ValueInc = Builder.CreateSExt(ValueInc, MaxType);

  std::vector<Value *> IVS(VectorWidth);
  IVS[0] = ValueLB;

  for (int i = 1; i < VectorWidth; i++)
    IVS[i] = Builder.CreateAdd(IVS[i - 1], ValueInc, "p_vector_iv");

  isl_id *Annotation = isl_ast_node_get_annotation(For);
  assert(Annotation && "For statement is not annotated");

  struct IslAstUserPayload *Info =
      (struct IslAstUserPayload *)isl_id_get_user(Annotation);
  assert(Info && "For statement annotation does not contain info");

  isl_union_map *Schedule = isl_ast_build_get_schedule(Info->Context);
  assert(Schedule && "For statement annotation does not contain its schedule");

  IDToValue[IteratorID] = ValueLB;

  switch (isl_ast_node_get_type(Body)) {
  case isl_ast_node_user:
    createUserVector(Body, IVS, isl_id_copy(IteratorID),
                     isl_union_map_copy(Schedule));
    break;
  case isl_ast_node_block: {
    isl_ast_node_list *List = isl_ast_node_block_get_children(Body);

    for (int i = 0; i < isl_ast_node_list_n_ast_node(List); ++i)
      createUserVector(isl_ast_node_list_get_ast_node(List, i), IVS,
                       isl_id_copy(IteratorID), isl_union_map_copy(Schedule));

    isl_ast_node_free(Body);
    isl_ast_node_list_free(List);
    break;
  }
  default:
    isl_ast_node_dump(Body);
    llvm_unreachable("Unhandled isl_ast_node in vectorizer");
  }

  IDToValue.erase(IteratorID);
  isl_id_free(IteratorID);
  isl_id_free(Annotation);
  isl_union_map_free(Schedule);

  isl_ast_node_free(For);
  isl_ast_expr_free(Iterator);
}

void IslNodeBuilder::createForSequential(__isl_take isl_ast_node *For) {
  isl_ast_node *Body;
  isl_ast_expr *Init, *Inc, *Iterator, *UB;
  isl_id *IteratorID;
  Value *ValueLB, *ValueUB, *ValueInc;
  Type *MaxType;
  BasicBlock *ExitBlock;
  Value *IV;
  CmpInst::Predicate Predicate;
  bool Parallel;

  Parallel = IslAstInfo::isInnermostParallel(For);

  Body = isl_ast_node_for_get_body(For);

  // isl_ast_node_for_is_degenerate(For)
  //
  // TODO: For degenerated loops we could generate a plain assignment.
  //       However, for now we just reuse the logic for normal loops, which will
  //       create a loop with a single iteration.

  Init = isl_ast_node_for_get_init(For);
  Inc = isl_ast_node_for_get_inc(For);
  Iterator = isl_ast_node_for_get_iterator(For);
  IteratorID = isl_ast_expr_get_id(Iterator);
  UB = getUpperBound(For, Predicate);

  ValueLB = ExprBuilder.create(Init);
  ValueUB = ExprBuilder.create(UB);
  ValueInc = ExprBuilder.create(Inc);

  MaxType = ExprBuilder.getType(Iterator);
  MaxType = ExprBuilder.getWidestType(MaxType, ValueLB->getType());
  MaxType = ExprBuilder.getWidestType(MaxType, ValueUB->getType());
  MaxType = ExprBuilder.getWidestType(MaxType, ValueInc->getType());

  if (MaxType != ValueLB->getType())
    ValueLB = Builder.CreateSExt(ValueLB, MaxType);
  if (MaxType != ValueUB->getType())
    ValueUB = Builder.CreateSExt(ValueUB, MaxType);
  if (MaxType != ValueInc->getType())
    ValueInc = Builder.CreateSExt(ValueInc, MaxType);

  IV = createLoop(ValueLB, ValueUB, ValueInc, Builder, P, ExitBlock, Predicate,
                  &Annotator, Parallel);
  IDToValue[IteratorID] = IV;

  create(Body);

  Annotator.End();

  IDToValue.erase(IteratorID);

  Builder.SetInsertPoint(ExitBlock->begin());

  isl_ast_node_free(For);
  isl_ast_expr_free(Iterator);
  isl_id_free(IteratorID);
}

void IslNodeBuilder::createFor(__isl_take isl_ast_node *For) {
  bool Vector = PollyVectorizerChoice != VECTORIZER_NONE;

  if (Vector && IslAstInfo::isInnermostParallel(For)) {
    int VectorWidth = getNumberOfIterations(For);
    if (1 < VectorWidth && VectorWidth <= 16) {
      createForVector(For, VectorWidth);
      return;
    }
  }
  createForSequential(For);
}

void IslNodeBuilder::createIf(__isl_take isl_ast_node *If) {
  isl_ast_expr *Cond = isl_ast_node_if_get_cond(If);

  Function *F = Builder.GetInsertBlock()->getParent();
  LLVMContext &Context = F->getContext();

  BasicBlock *CondBB =
      SplitBlock(Builder.GetInsertBlock(), Builder.GetInsertPoint(), P);
  CondBB->setName("polly.cond");
  BasicBlock *MergeBB = SplitBlock(CondBB, CondBB->begin(), P);
  MergeBB->setName("polly.merge");
  BasicBlock *ThenBB = BasicBlock::Create(Context, "polly.then", F);
  BasicBlock *ElseBB = BasicBlock::Create(Context, "polly.else", F);

  DominatorTree &DT = P->getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  DT.addNewBlock(ThenBB, CondBB);
  DT.addNewBlock(ElseBB, CondBB);
  DT.changeImmediateDominator(MergeBB, CondBB);

  LoopInfo &LI = P->getAnalysis<LoopInfo>();
  Loop *L = LI.getLoopFor(CondBB);
  if (L) {
    L->addBasicBlockToLoop(ThenBB, LI.getBase());
    L->addBasicBlockToLoop(ElseBB, LI.getBase());
  }

  CondBB->getTerminator()->eraseFromParent();

  Builder.SetInsertPoint(CondBB);
  Value *Predicate = ExprBuilder.create(Cond);
  Builder.CreateCondBr(Predicate, ThenBB, ElseBB);
  Builder.SetInsertPoint(ThenBB);
  Builder.CreateBr(MergeBB);
  Builder.SetInsertPoint(ElseBB);
  Builder.CreateBr(MergeBB);
  Builder.SetInsertPoint(ThenBB->begin());

  create(isl_ast_node_if_get_then(If));

  Builder.SetInsertPoint(ElseBB->begin());

  if (isl_ast_node_if_has_else(If))
    create(isl_ast_node_if_get_else(If));

  Builder.SetInsertPoint(MergeBB->begin());

  isl_ast_node_free(If);
}

void IslNodeBuilder::createSubstitutions(isl_ast_expr *Expr, ScopStmt *Stmt,
                                         ValueMapT &VMap, LoopToScevMapT &LTS) {
  assert(isl_ast_expr_get_type(Expr) == isl_ast_expr_op &&
         "Expression of type 'op' expected");
  assert(isl_ast_expr_get_op_type(Expr) == isl_ast_op_call &&
         "Opertation of type 'call' expected");
  for (int i = 0; i < isl_ast_expr_get_op_n_arg(Expr) - 1; ++i) {
    isl_ast_expr *SubExpr;
    Value *V;

    SubExpr = isl_ast_expr_get_op_arg(Expr, i + 1);
    V = ExprBuilder.create(SubExpr);
    ScalarEvolution *SE = Stmt->getParent()->getSE();
    LTS[Stmt->getLoopForDimension(i)] = SE->getUnknown(V);

    // CreateIntCast can introduce trunc expressions. This is correct, as the
    // result will always fit into the type of the original induction variable
    // (because we calculate a value of the original induction variable).
    const Value *OldIV = Stmt->getInductionVariableForDimension(i);
    if (OldIV) {
      V = Builder.CreateIntCast(V, OldIV->getType(), true);
      VMap[OldIV] = V;
    }
  }

  isl_ast_expr_free(Expr);
}

void IslNodeBuilder::createSubstitutionsVector(
    __isl_take isl_ast_expr *Expr, ScopStmt *Stmt, VectorValueMapT &VMap,
    std::vector<LoopToScevMapT> &VLTS, std::vector<Value *> &IVS,
    __isl_take isl_id *IteratorID) {
  int i = 0;

  Value *OldValue = IDToValue[IteratorID];
  for (Value *IV : IVS) {
    IDToValue[IteratorID] = IV;
    createSubstitutions(isl_ast_expr_copy(Expr), Stmt, VMap[i], VLTS[i]);
    i++;
  }

  IDToValue[IteratorID] = OldValue;
  isl_id_free(IteratorID);
  isl_ast_expr_free(Expr);
}

void IslNodeBuilder::createUser(__isl_take isl_ast_node *User) {
  ValueMapT VMap;
  LoopToScevMapT LTS;
  isl_id *Id;
  ScopStmt *Stmt;

  isl_ast_expr *Expr = isl_ast_node_user_get_expr(User);
  isl_ast_expr *StmtExpr = isl_ast_expr_get_op_arg(Expr, 0);
  Id = isl_ast_expr_get_id(StmtExpr);
  isl_ast_expr_free(StmtExpr);

  Stmt = (ScopStmt *)isl_id_get_user(Id);
  createSubstitutions(Expr, Stmt, VMap, LTS);
  BlockGenerator::generate(Builder, *Stmt, VMap, LTS, P);

  isl_ast_node_free(User);
  isl_id_free(Id);
}

void IslNodeBuilder::createBlock(__isl_take isl_ast_node *Block) {
  isl_ast_node_list *List = isl_ast_node_block_get_children(Block);

  for (int i = 0; i < isl_ast_node_list_n_ast_node(List); ++i)
    create(isl_ast_node_list_get_ast_node(List, i));

  isl_ast_node_free(Block);
  isl_ast_node_list_free(List);
}

void IslNodeBuilder::create(__isl_take isl_ast_node *Node) {
  switch (isl_ast_node_get_type(Node)) {
  case isl_ast_node_error:
    llvm_unreachable("code  generation error");
  case isl_ast_node_for:
    createFor(Node);
    return;
  case isl_ast_node_if:
    createIf(Node);
    return;
  case isl_ast_node_user:
    createUser(Node);
    return;
  case isl_ast_node_block:
    createBlock(Node);
    return;
  }

  llvm_unreachable("Unknown isl_ast_node type");
}

void IslNodeBuilder::addParameters(__isl_take isl_set *Context) {
  SCEVExpander Rewriter(P->getAnalysis<ScalarEvolution>(), "polly");

  for (unsigned i = 0; i < isl_set_dim(Context, isl_dim_param); ++i) {
    isl_id *Id;
    const SCEV *Scev;
    IntegerType *T;
    Instruction *InsertLocation;

    Id = isl_set_get_dim_id(Context, isl_dim_param, i);
    Scev = (const SCEV *)isl_id_get_user(Id);
    T = dyn_cast<IntegerType>(Scev->getType());
    InsertLocation = --(Builder.GetInsertBlock()->end());
    Value *V = Rewriter.expandCodeFor(Scev, T, InsertLocation);
    IDToValue[Id] = V;

    isl_id_free(Id);
  }

  isl_set_free(Context);
}

namespace {
class IslCodeGeneration : public ScopPass {
public:
  static char ID;

  IslCodeGeneration() : ScopPass(ID) {}

  bool runOnScop(Scop &S) {
    IslAstInfo &AstInfo = getAnalysis<IslAstInfo>();

    assert(!S.getRegion().isTopLevelRegion() &&
           "Top level regions are not supported");

    simplifyRegion(&S, this);

    BasicBlock *StartBlock = executeScopConditionally(S, this);
    isl_ast_node *Ast = AstInfo.getAst();
    LoopAnnotator Annotator;
    PollyIRBuilder Builder(StartBlock->getContext(), llvm::ConstantFolder(),
                           polly::IRInserter(Annotator));
    Builder.SetInsertPoint(StartBlock->begin());

    IslNodeBuilder NodeBuilder(Builder, Annotator, this);

    Builder.SetInsertPoint(StartBlock->getSinglePredecessor()->begin());
    NodeBuilder.addParameters(S.getContext());
    // Build condition that evaluates at run-time if all assumptions taken
    // for the scop hold. If we detect some assumptions do not hold, the
    // original code is executed.
    Value *V = NodeBuilder.getExprBuilder().create(AstInfo.getRunCondition());
    Value *Zero = ConstantInt::get(V->getType(), 0);
    V = Builder.CreateICmp(CmpInst::ICMP_NE, Zero, V);
    BasicBlock *PrevBB = StartBlock->getUniquePredecessor();
    BranchInst *Branch = dyn_cast<BranchInst>(PrevBB->getTerminator());
    Branch->setCondition(V);
    Builder.SetInsertPoint(StartBlock->begin());

    NodeBuilder.create(Ast);
    return true;
  }

  virtual void printScop(raw_ostream &OS) const {}

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<IslAstInfo>();
    AU.addRequired<RegionInfoPass>();
    AU.addRequired<ScalarEvolution>();
    AU.addRequired<ScopDetection>();
    AU.addRequired<ScopInfo>();
    AU.addRequired<LoopInfo>();

    AU.addPreserved<Dependences>();

    AU.addPreserved<LoopInfo>();
    AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addPreserved<IslAstInfo>();
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

char IslCodeGeneration::ID = 1;

Pass *polly::createIslCodeGenerationPass() { return new IslCodeGeneration(); }

INITIALIZE_PASS_BEGIN(IslCodeGeneration, "polly-codegen-isl",
                      "Polly - Create LLVM-IR from SCoPs", false, false);
INITIALIZE_PASS_DEPENDENCY(Dependences);
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass);
INITIALIZE_PASS_DEPENDENCY(LoopInfo);
INITIALIZE_PASS_DEPENDENCY(RegionInfoPass);
INITIALIZE_PASS_DEPENDENCY(ScalarEvolution);
INITIALIZE_PASS_DEPENDENCY(ScopDetection);
INITIALIZE_PASS_END(IslCodeGeneration, "polly-codegen-isl",
                    "Polly - Create LLVM-IR from SCoPs", false, false)
