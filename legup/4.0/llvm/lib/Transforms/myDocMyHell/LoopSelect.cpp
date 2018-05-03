#include "LoopSelect.h"
#include <sys/stat.h>
#include "ModuloScheduler.h"

void LoopSelect::clean(){
  //delete loop datas
  for(auto entry:loopDataMap){
    delete entry.second;
  }

  //delete the loop infos we have created
  for(auto loopinfo : loopInfoVec){
      loopinfo->~LoopInfoType();
  }

  alloc->~Allocation();

  bbMap.clear();

  for(auto entry : TCLConstraints){
    delete entry;
  }
  TCLConstraints.clear();

  return;

}

void LoopSelect::printLoopsData(){
  for(auto entry : loopDataMap){
    printLoopData(entry.second);
  }
  std::cout << "\n\n";
  return;
}

void LoopSelect::printLoopData(LoopData *ldin){
  std::cout << "\nloop: " << ldin->label << "\n";
  ldin->loop->dump();
  for(auto entry : ldin->nFUs){
    std::cout << "FU:" << entry.first << " - used by " << entry.second << " instructions\n";
  }

  for(auto subLoopData : ldin->subLoopsData){
    std::cout << "\nsubloop found:\n";
    printLoopData(subLoopData);
  }

  return;
}

void LoopSelect::analyzeLoops(){
  bbMap.clear();
  for(auto loop : loopVec){
    std::cout << "found loop:\n";
    loop->dump();
    loopDataMap[loop] = getLoopBasicMetrics(loop);
    saveLoopAsFunction(loop);
    //create config.tcl
    createTCLConfigs(loop);
  }
  bbMap.clear();

  //TODO this
  //automate a make run with each different config.
}

LoopData * LoopSelect::getLoopBasicMetrics(llvm::Loop * loop){
  LoopData * ld = new LoopData();

  //because I love pointers
  ld->loop = loop;
  ModuloScheduler ms = ModuloScheduler();

  //get subloop data first
  if(loop->getSubLoopsVector().size() != 0){
    for(auto lit = loop->getSubLoopsVector().begin(); lit != loop->getSubLoopsVector().end(); lit++){
      llvm::Loop *subLoop = *lit;
      ld->subLoopsData.push_back(getLoopBasicMetrics(subLoop));
    }
  }

  for(llvm::Loop::block_iterator bit = loop->block_begin(); bit != loop->block_end(); bit++){
    llvm::BasicBlock *bb = *bit;
    //skip basic Blocks that have already been mapped in subloops.
    if(bbMap[bb] == true) continue;

    //bb->dump();
    //get label
    if(!ms.get_legup_label(bb).empty()){;
      ld->label = ms.get_legup_label(bb);
    }

    for(llvm::BasicBlock::iterator iit = bb->begin(); iit != bb->end(); iit++){
      std::string fuName = LEGUP_CONFIG->getOpNameFromInst(iit, alloc);

      //remove insts that are not assigned to any FU
      if(fuName.empty()) continue;

      ld->nFUs[fuName]++;
    }

    bbMap[bb] = true;
  }

  return ld;
}

using Constraint = std::tuple<std::string, int, int>;

void LoopSelect::addRAMConstraints(){
  Constraint *constraint = new Constraint("set_parameter LOCAL_RAMS", 0, 1);
  TCLConstraints.push_back(constraint);
  return;
}

void LoopSelect::createTCLConfigs(llvm::Loop *loop){
  //add the Local RAM constraints in the TCL constraints list
  addRAMConstraints();

  //TODO
  //ADD resource constraints
  //ADD pipeline constraints

  std::string filename = std::string("loops_out/");
  filename.append(loopDataMap[loop]->label);
  filename.append("/tcl.config");
  FILE * file = NULL;

  file = fopen(filename.c_str(), "w");
  if(file == NULL){
    std::cout << "Error creating tcl.config!\n";
    exit(1);
  }

  //TODO
  //write in the TCL file

  fclose(file);
  return;
}

void LoopSelect::saveLoopAsFunction(llvm::Loop *loop){

  std::string outdir = std::string("loops_out");
  int dir_err = mkdir(outdir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  if (-1 == dir_err && errno != EEXIST){
    std::cout << "Error creating loops_out directory!\n";
    exit(1);
  }

  outdir.append("/");
  outdir.append(loopDataMap[loop]->label);

  dir_err = mkdir(outdir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  if (-1 == dir_err && errno != EEXIST){
    std::cout << "Error creating" << outdir << "directory!\n";
    exit(1);
  }

  if(debug){
    std::cout << "-----------" << std::endl;
  }

  //TODO save as function

  return;
}

bool LoopSelect::runOnModule(Module &M){
    alloc = new Allocation(&M);
    //IMS.LI = &getAnalysis<LoopInfo>();
    //IMS.AA = &getAnalysis<AliasAnalysis>();
    //IMS.SE = &getAnalysis<ScalarEvolution>();
    //L->dump();

    for(llvm::Module::iterator F = M.begin(); F!= M.end(); F++){
      //skip declrations
      if(F->isDeclaration()){
        continue;
      }

      //this solves that scope problem where LoopInfo is deleted by the PassManager when invoked again for another function
      llvm::DominatorTree DT = llvm::DominatorTree();
      DT.recalculate(*F);
      LoopInfoType *loopInfo = new LoopInfoType();
      loopInfo->releaseMemory();
      loopInfo->Analyze(DT);
      loopInfoVec.push_back(loopInfo);

      for(llvm::LoopInfo::iterator lit = loopInfo->begin(); lit != loopInfo->end(); lit++){
        Loop * L = * lit;
        loopVec.push_back(L);
        loopFunctionMap[L] = F;
        //L->dump();
      }

    }//for all functions

    analyzeLoops();
    printLoopsData();
    //return true if IR has been modified

    clean();
    return true;
    //return IMS.runOnLoop(L, LPM);
}
