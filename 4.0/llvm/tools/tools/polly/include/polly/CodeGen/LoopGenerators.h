//===- LoopGenerators.h - IR helper to create loops -------------*- C++ -*-===//
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
#ifndef POLLY_LOOP_GENERATORS_H
#define POLLY_LOOP_GENERATORS_H
#include "polly/CodeGen/IRBuilder.h"
#include "llvm/ADT/SetVector.h"

#include <map>

namespace llvm {
class Value;
class Pass;
class BasicBlock;
}

namespace polly {
using namespace llvm;

/// @brief Create a scalar loop.
///
/// @param LowerBound The starting value of the induction variable.
/// @param UpperBound The upper bound of the induction variable.
/// @param Stride     The value by which the induction variable is incremented.
///
/// @param Builder    The builder used to create the loop.
/// @param P          A pointer to the pass that uses this function. It is used
///                   to update analysis information.
/// @param ExitBlock  The block the loop will exit to.
/// @param Predicate  The predicate used to generate the upper loop bound.
/// @param Annotator  This function can (optionally) take a LoopAnnotator which
///                   tracks the loop structure.
/// @param Parallel   If this loop should be marked parallel in the Annotator.
/// @return Value*    The newly created induction variable for this loop.
Value *createLoop(Value *LowerBound, Value *UpperBound, Value *Stride,
                  PollyIRBuilder &Builder, Pass *P, BasicBlock *&ExitBlock,
                  ICmpInst::Predicate Predicate,
                  LoopAnnotator *Annotator = NULL, bool Parallel = false);

class OMPGenerator {
public:
  typedef std::map<Value *, Value *> ValueToValueMapTy;

  OMPGenerator(PollyIRBuilder &Builder, Pass *P) : Builder(Builder), P(P) {}

  /// @brief Create an OpenMP parallel loop.
  ///
  ///
  /// @param LowerBound  The starting value of the induction variable.
  /// @param UpperBound  The upper bound of the induction variable.
  /// @param Stride      The value by which the induction variable is
  ///                    incremented.
  ///
  /// @param UsedValues  A set of LLVM-IR Values that should be available to
  ///                    the new loop body.
  /// @param VMap        This map is filled by createParallelLoop(). It
  ///                    maps the values in UsedValues to Values through which
  ///                    their content is available within the loop body.
  /// @param LoopBody    A pointer to an iterator that is set to point to the
  ///                    body of the created loop. It should be used to insert
  ///                    instructions that form the actual loop body.
  ///
  /// @return Value*     The newly created induction variable for this loop.
  Value *createParallelLoop(Value *LowerBound, Value *UpperBound, Value *Stride,
                            SetVector<Value *> &UsedValues,
                            ValueToValueMapTy &VMap,
                            BasicBlock::iterator *LoopBody);

private:
  PollyIRBuilder &Builder;
  Pass *P;

  IntegerType *getIntPtrTy();
  Module *getModule();

  void createCallParallelLoopStart(Value *SubFunction, Value *SubfunctionParam,
                                   Value *NumberOfThreads, Value *LowerBound,
                                   Value *UpperBound, Value *Stride);
  Value *createCallLoopNext(Value *LowerBoundPtr, Value *UpperBoundPtr);
  void createCallParallelEnd();
  void createCallLoopEndNowait();

  Value *loadValuesIntoStruct(SetVector<Value *> &Values);
  void extractValuesFromStruct(SetVector<Value *> OldValues, Value *Struct,
                               ValueToValueMapTy &Map);

  /// @brief Create the OpenMP subfunction.
  ///
  /// @param Stride       The value by which the induction variable is
  ///                     incremented.
  /// @param Struct       The structure that is used to make Values available to
  ///                     the loop body.
  /// @param UsedValues   A set of LLVM-IR Values that should be available to
  ///                     the new loop body.
  /// @param VMap         This map that is filled by createSubfunction(). It
  ///                     maps the values in UsedValues to Values through which
  ///                     their content is available within the loop body.
  /// @param SubFunction  The newly created SubFunction is returned here.
  ///
  /// @return Value*      The newly created induction variable.
  Value *createSubfunction(Value *Stride, Value *Struct,
                           SetVector<Value *> UsedValues,
                           ValueToValueMapTy &VMap, Function **SubFunction);

  /// @brief Create the definition of the OpenMP subfunction.
  Function *createSubfunctionDefinition();
};
} // end namespace polly
#endif
