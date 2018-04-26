#define DEBUG_LEVEL  0
#include "llvm/IR/Dominators.h"
#include "AcceleratorAnalysis.h"

/*
#include <assert.h>
#include <iostream>

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/PassManager.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
*/

using namespace llvm;

std::string pass_name="accelerator-analysis";
#define DEBUG_TYPE "accelerator-analysis"
STATISTIC(LoopCount, "analyse possible accelerators");

namespace {
  class AcceleratorAnalysis : public ModulePass {
  public:
    static char ID; // Pass identification, replacement for typeid
    AcceleratorAnalysis() : ModulePass(ID) {}

    virtual void getAnalysisUsage(AnalysisUsage &AU) const{
      AU.addRequired<LoopInfo>();
    }
    //bool doInitialization(Loop *L, LPPassManager &lpm) override;

  bool runOnModule(Module &M) override;

  private:
    LoopData GetBestLoop(std::vector<LoopData> loopDataVector);
  };

}

char AcceleratorAnalysis::ID = 0;
static RegisterPass<AcceleratorAnalysis> X("accelerator-analysis", "accelerator analysis", false, false);

//this is a simple search for the loop with the maximum number of binary instructions
LoopData AcceleratorAnalysis::GetBestLoop(std::vector<LoopData> loopDataVector){
  LoopData best, ld;
  std::vector<LoopData>::iterator vec_iter, vec_iter_end;
  unsigned max=0;

  for(vec_iter = loopDataVector.begin(), vec_iter_end = loopDataVector.end(); vec_iter != vec_iter_end; ++vec_iter){
    ld = *vec_iter;
    if(ld.binaryInstCount > max){
      best = ld;
      max = ld.binaryInstCount;
    }
  }

  return best;
}

bool AcceleratorAnalysis::runOnModule(Module &M){
  ++LoopCount;

  Module::iterator func_iter, func_iter_end;
  LoopInfo::iterator loop_iter, loop_iter_end;
  Loop::block_iterator block_iter, block_iter_end;
  BasicBlock::iterator inst_iter, inst_iter_end;

  std::vector<LoopData> loopDataVector;
  LoopData loopData;

  int inLoopInstCount;
  unsigned opcode=0, line=0, binaryInstCount=0;
  const char *opcodeName;

  for (func_iter = M.begin(), func_iter_end = M.end(); func_iter != func_iter_end; ++func_iter) {
    Function &F = *func_iter; //masks some pointer calls

    if (!F.isDeclaration()){
      LoopInfo &LI = getAnalysis<LoopInfo>(F);
      errs() << "A Function starts !!------------------\n";

      //gets all loops in this function
      for(loop_iter = LI.begin(), loop_iter_end = LI.end(); loop_iter != loop_iter_end; ++loop_iter){
        Loop *L = *loop_iter; //masks some pointer calls

        //basic blocks composing the looop
        block_iter = L->block_begin();
        block_iter_end = L->block_end();

        //counts how many instructions in a loop
        inLoopInstCount = 0;
        //counts how many logic/arith instructions in a loop
        binaryInstCount = 0;

        //get loop starting line on source code
        line = L->getStartLoc().getLine();

        for( ; block_iter != block_iter_end; ++block_iter){
          BasicBlock *B = *block_iter; //masks some pointer calls

          inLoopInstCount += B->size();

          //instructions composing each block
          inst_iter = B->begin();
          inst_iter_end = B->end();

          for( ; inst_iter != inst_iter_end; ++inst_iter){
            Instruction &I = *inst_iter; //masks some pointer calls

            opcode = I.getOpcode();
            opcodeName = I.getOpcodeName();

            if(I.isBinaryOp()){
              binaryInstCount++;
              errs() << opcode << " " << opcodeName << ", ";
            }
          }
        }
        loopData.startLine = line;
        loopData.instCount = inLoopInstCount;
        loopData.binaryInstCount = binaryInstCount;
        loopData.l = L;
        loopDataVector.push_back(loopData);

        errs() << "\n found a loop in line "<< line << " with "<< inLoopInstCount << " instructions \n where " << binaryInstCount << " are binary (logic/arith) \n\n";
      }
    }/*else{
      errs() << "found function\n";
    }*/
  }

  LoopData best = GetBestLoop(loopDataVector);
  errs() << "\n the best loop has " << best.binaryInstCount << " binary instructions\n\n";

  return false;
}
