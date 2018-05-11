#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Module.h"
#include "utils.h"
#include "LegupConfig.h"
#include "llvm/IR/Dominators.h"

#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

#include <sstream>
#include <iomanip>
#include <vector>
#include <utility>
#include <math.h>
#include <algorithm>

#include "Allocation.h"

// using namespace polly;
using namespace llvm;
using namespace legup;

namespace llvm {
void initializeLoopSelectPass(llvm::PassRegistry &);
}

namespace {

//string = name and operand, min, and max
using Constraint=std::tuple<std::string, int, int>;

typedef struct loopData{
  llvm::Loop *loop = NULL;
  //llvm::Loop *outmostLoop = NULL;
  //llvm::Loop *parentLoop = NULL;
  std::string label;
  std::map<std::string, int> nFUs;
  std::vector<loopData*> subLoopsData;
  std::vector<Constraint*> TCLConstraints;
}LoopData;

class LoopSelect : public ModulePass {
  // Region *region;
  // Scop *S;
  DominatorTree *DT;
  ScalarEvolution *SE;
  // ScopDetection *SD;
  // CloogInfo *C;
  LoopInfo *LI;
  DataLayout *TD;

  private:
    bool debug = LEGUP_CONFIG->getParameterInt("DEBUG_LOOP_SELECT");

    legup::Allocation *alloc;
    using LoopInfoType=llvm::LoopInfoBase<llvm::BasicBlock, llvm::Loop>;

    std::vector<llvm::Loop*> loopVec;
    std::vector<LoopInfoType*> loopInfoVec;

    std::map<llvm::Loop*, llvm::Function*> loopFunctionMap;
    std::map<llvm::Loop*, LoopData*> loopDataMap;
    std::map<llvm::BasicBlock*, bool> bbMap;

    void clean();
    void clearLoopData(LoopData *ld);
    void analyzeLoops();
    LoopData * getLoopBasicMetrics(llvm::Loop * loop);

    void addRAMConstraints(LoopData *ld);
    void addResourcesConstraints(LoopData *ld);
    void addPipelineConstraint(LoopData *ld);
    std::vector<Constraint*> parseConstraints(LoopData *ld);
    void createTCLConfigs(llvm::Loop *loop);

    void saveLoopAsFunction(llvm::Loop *loop);
    void printLoopsData();
    void printLoopData(LoopData* loop);


  public:
    static char ID;
    // SDCModuloScheduler() : ScopPass(ID) {
    LoopSelect() : ModulePass(ID) {}

    bool runOnModule(Module &M) override;

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
        AU.addRequired<LoopInfo>();
        AU.addRequired<AliasAnalysis>();
        AU.addRequired<ScalarEvolution>();
        // AU.setPreservesAll();
        // does not preserve loopinfo?
        // AU.addPreserved<MemoryDependenceAnalysis>();
        // AU.addPreserved<AliasAnalysis>();

        /*
        AU.addRequired<CloogInfo>();
        AU.addRequired<Dependences>();
        AU.addRequired<DominatorTree>();
        AU.addRequired<RegionInfo>();
        AU.addRequired<ScopDetection>();
        AU.addRequired<ScopInfo>();
        AU.addRequired<DataLayout>();

        AU.addPreserved<CloogInfo>();
        AU.addPreserved<Dependences>();
        AU.addPreserved<DominatorTree>();
        AU.addPreserved<PostDominatorTree>();
        AU.addPreserved<ScopDetection>();
        AU.addPreserved<ScalarEvolution>();
        AU.addPreserved<RegionInfo>();
        AU.addPreserved<TempScopInfo>();
        AU.addPreserved<ScopInfo>();
        AU.addPreservedID(IndependentBlocksID);
        */
    }
};
}

/*
*/
char LoopSelect::ID = 1;

INITIALIZE_PASS_BEGIN(LoopSelect, "loop-select", "Leandro's Loop Selection",false, false)

// INITIALIZE_PASS_DEPENDENCY(SchedulerDAG)
INITIALIZE_PASS_END(LoopSelect, "loop-slect", "Leandro's Loop Selection",false, false)

static RegisterPass<LoopSelect> Z("loop-select", "Leandro's Loop Selection");

/*
using namespace llvm;
char LoopPipeline::ID = 0;
INITIALIZE_PASS(LoopPipeline, "modulo-schedule",
                "LegUp Iterative Modulo Scheduling PrePass",
                false, false)
namespace llvm {

LoopPass *createSDCModuloSchedulerPass() {
  return new LoopPipeline();
}
*/

//}
/*
namespace polly {
Pass* createModuloSchedulePass() {
    //return 0;
  return new LoopPipeline();
}
}
*/
