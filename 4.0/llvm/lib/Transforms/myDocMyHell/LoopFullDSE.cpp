#include "LoopFullDSE.h"
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include "ModuloScheduler.h"

using Constraint = std::tuple<std::string, int, int>;

void LoopFullDSE::clearLoopData(LoopData *ld){
  for(auto constr : ld->TCLConstraints){
    delete constr;
  }
  ld->TCLConstraints.clear();

  for(auto subloopdata : ld->subLoopsData){
    clearLoopData(subloopdata);
    delete subloopdata;
  }

  return;
}

void LoopFullDSE::clean(){
  //delete loop datas
  for(auto entry:loopDataMap){
    clearLoopData(entry.second);
    delete entry.second;
  }
  loopDataMap.clear();

  //delete the loop infos we have created
  for(auto loopinfo : loopInfoVec){
      loopinfo->~LoopInfoType();
  }

  alloc->~Allocation();

  bbMap.clear();
  return;

}

void LoopFullDSE::printLoopsData(){
  for(auto entry : loopDataMap){
    printLoopData(entry.second);
      std::cout << "\n----------------------------------\n\n";
  }
  return;
}

void LoopFullDSE::printLoopData(LoopData *ldin){
  std::cout << "\nloop: " << ldin->label << "\n";
  ldin->loop->dump();

  for(auto entry : ldin->nFUs){
    std::cout << "FU:" << entry.first << " - used by " << entry.second << " instructions\n";
  }

  for(auto subLoopData : ldin->subLoopsData){
    std::cout << "\nsubloop found:\n";
    printLoopData(subLoopData);
  }

  //skip if there is no constraints for this loop (a.k.a its a subloop)
  if(ldin->TCLConstraints.empty()){
    return;
  }

  std::cout << "\nconstraints:\n" << std::endl;
  std::cout << "size: " << ldin->TCLConstraints.size() << std::endl;
  for(auto cs : ldin->TCLConstraints){
    std::cout << std::get<0>(*cs) << " - min: " << std::get<1>(*cs) << " - max: " << std::get<2>(*cs) << "\n";
  }
  return;
}

void LoopFullDSE::analyzeLoops(){
  bbMap.clear();

  for(auto loop : loopVec){
    if(debug){
      std::cout << "found loop:\n";
      loop->dump();
    }

    loopDataMap[loop] = getLoopBasicMetrics(loop);
    saveLoopAsFunction(loop);
    //create config.tcl
    createTCLConfigs(loop);
    //create Makefile
    createMakefile(loop);
    //The execution of the designs with the several config.tcl files is automated in $(LEGUPHOME)/examples/Makefile.myDocMyHell and $(LEGUPHOME)/examples/Makefile.loops

  }

  bbMap.clear();
}

LoopData * LoopFullDSE::getLoopBasicMetrics(llvm::Loop * loop){
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

      //if(fuName.compare("signed_divide_32") == 0){
      //  ld->nFUs[fuName] += 2;
      //}else{
      ld->nFUs[fuName]++;
      //}
    }

    bbMap[bb] = true;
  }

  return ld;
}

void LoopFullDSE::addRAMConstraints(LoopData *ld){
  //local rams always improve performance
  Constraint *constraint = new Constraint("set_parameter LOCAL_RAMS", 2, 2);
  ld->TCLConstraints.push_back(constraint);

  if(debug){
    std::cout << "adding Local RAM constraint to loop: " << ld->label << "\n";
  }

  return;
}

void LoopFullDSE::addSolverConstraints(LoopData *ld){
  //local rams always improve performance
  Constraint *constraint = new Constraint(std::string("set_parameter SOLVER \"GUROBI\""), 0, 0);
  ld->TCLConstraints.push_back(constraint);

  if(debug){
    std::cout << "adding gurobi solver constraint to loop: " << ld->label << "\n";
  }

  return;
}

void LoopFullDSE::addResourcesConstraints(LoopData *ld){
  Constraint *constraint = NULL;
  for(auto entry : ld->nFUs){
    std::string constrName = std::string("set_resource_constraint ");
    constrName.append(entry.first);


    int ub = entry.second;
    //there is no need to try to generate many memories
    if(entry.first.compare("mem_dual_port") == 0 && ub >=1){
      ub = 1;
    }


    constraint = new Constraint(constrName, 1, ub);
    ld->TCLConstraints.push_back(constraint);
  }

  for(auto subloopdata:ld->subLoopsData){
    addResourcesConstraints(subloopdata);
  }

  return;
}

void LoopFullDSE::addPipelineConstraint(LoopData *ld){
  //if this loop has no sub-loops
  if(ld->subLoopsData.size() == 0){
    Constraint *constraint = NULL;
    
    std::string pipeFlag = LEGUP_CONFIG->getParameter("DSE_PIPE_OFF");
    std::string constrName;
    if(pipeFlag.compare("1")==0){
    	constrName = std::string("#loop_pipeline \"");
    }else{
    	constrName = std::string("loop_pipeline \"");
    }
    
    constrName.append(ld->label);
    constrName.append("\"");
    constraint = new Constraint(constrName, 0, 0);
    ld->TCLConstraints.push_back(constraint);

    //just to set the modulo scheduler
    std::string schedulerType = LEGUP_CONFIG->getParameter("MODULO_SCHEDULER");
    std::string constString;
    if(schedulerType.compare("ILP")==0){
	constString = std::string("set_parameter MODULO_SCHEDULER \"ILP\"");
    }else if(schedulerType.compare("NI")==0){
	constString = std::string("set_parameter MODULO_SCHEDULER \"NI\"");
    }else{
  	constString = std::string("set_parameter MODULO_SCHEDULER \"SDC\"");
    }

    constraint = new Constraint(constString, 0, 0);
    ld->TCLConstraints.push_back(constraint);
    return;
  }

  //if there are nested loops
  for(auto subloopdata:ld->subLoopsData){
    addPipelineConstraint(subloopdata);
  }

  return;
}

std::vector<Constraint*> LoopFullDSE::parseConstraints(LoopData *ld){
  std::vector<Constraint*> constraintsVec;
  std::map<std::string, Constraint*> constraintsMap;

  //concatenate the constraints of all subloops
  constraintsVec.insert(constraintsVec.end(), ld->TCLConstraints.begin(), ld->TCLConstraints.end());
  for(auto subloopdata : ld->subLoopsData){
    std::vector<Constraint*> subloopConstraintsVec = parseConstraints(subloopdata);
    constraintsVec.insert(constraintsVec.end(), subloopConstraintsVec.begin(), subloopConstraintsVec.end());
  }

  for(auto constraint : constraintsVec){
    std::string name = std::get<0>(*constraint);

    //if the constraints is not yet in the map
    if(constraintsMap.find(name) == constraintsMap.end()){
      constraintsMap[name] = constraint;
    }else{
      int invecMin = std::get<1>(*(constraintsMap[name]));
      int invecMax = std::get<2>(*(constraintsMap[name]));
      int currMin = std::get<1>(*constraint);
      int currMax = std::get<2>(*constraint);
      int min = std::min(invecMin, currMin);
      int max = std::max(invecMax, currMax);
      constraintsMap[name] = new Constraint(name, min, max);
    }
  }


  constraintsVec.clear();
  for(auto entry : constraintsMap){
    constraintsVec.push_back(entry.second);
  }

  //constraintsMap.clear();
  return constraintsVec;
}

void LoopFullDSE::createTCLConfigs(llvm::Loop *loop){
  //create a new vector for this loop
  LoopData *ld = loopDataMap[loop];

  //add the Local RAM constraints in the TCL constraints list
  addRAMConstraints(ld);
  addSolverConstraints(ld);
  //ADD resource constraints
  addResourcesConstraints(ld);
  addPipelineConstraint(ld);
  //TODO
  //deal with unroll

  std::vector<Constraint*> constraintsVec = parseConstraints(ld);

  if(debug){
    std::cout << "\n--------------------------- \n Final Constraints for Loop Nest: \n";
    for(auto cs : constraintsVec){
      std::cout << std::get<0>(*cs) << " - min: " << std::get<1>(*cs) << " - max: " << std::get<2>(*cs) << "\n";
    }
    std::cout << "\n---------------------------\n";
  }

  //calculate the number of TCL config files
  unsigned nconfigs = 1;
  for(auto cs : constraintsVec){
    int min = std::get<1>(*cs), max = std::get<2>(*cs);
    if(min != 0 || max != 0){
      nconfigs *= max - min + 1;
    }
  }

  if(debug){
    std::cout << "There are " << nconfigs << " possible configs" << " for " << constraintsVec.size() << " constraints\n";
  }

  std::string basename = std::string("loops_out/");
  basename.append(loopDataMap[loop]->label);
  //nconfigs file pointers initialized as NULL

  ofstream file;
  unsigned m, m0 = nconfigs, nconstraints = constraintsVec.size();
  unsigned i, j;
  std::string filename;

  //matrix for debug mostly
  int * constmatrix = (int*)malloc(nconstraints*nconfigs*sizeof(int));

  for(i=0; i < nconfigs; i++){
    m = m0;
    filename = std::string(basename);
    filename.append(std::string("/config")+std::to_string(i)+std::string(".tcl"));
    file.open(filename.c_str());

    for(j=0; j != nconstraints; j++){
      int l = std::get<1>(*(constraintsVec[j]));
      int u = std::get<2>(*(constraintsVec[j]));
      int d = u-l+1;

      m = floor(m/d);
      constmatrix[j*nconfigs+i] = l+((int)floor(i/m))%d;

      //always on constraints as loop pipeline
      if(l==0 && u==0){
        file << std::get<0>(*(constraintsVec[j])) << "\n";
      }else{
        file << std::get<0>(*(constraintsVec[j])) << " " << constmatrix[j*nconfigs+i] << "\n";
      }
    }

    file.close();
  }

  /*
  if(debug){
    std::cout << "\nConstranint matrix:";
    for(j=0; j < constraintsVec.size(); j++){
      std::cout << "\n";
      for(i=0; i < nconfigs; i++){
        std::cout << constmatrix[j*nconfigs+i] << " ";
      }
    }
    std::cout << "\n";
  }
  */

  return;
}

void LoopFullDSE::createMakefile(llvm::Loop *loop){
  ofstream file;
  std::string basename = std::string("loops_out/");
  basename.append(loopDataMap[loop]->label);
  std::string filename = basename+std::string("/Makefile");
  file.open(filename.c_str());
  file << "NAME=module\n";
  file << "CUSTOM_MODULE_SCHEDULING = 1\n";
  file << "NO_OPT=0\n";
  file << "NO_INLINE=0\n";
  file << "LEVEL = $(LEGUPHOME)/examples\n";
  file << "#LOCAL_CONFIG = -legup-config=config1.tcl\n";
  file << "CONFIGS=$(shell ls config*.tcl)\n";
  file << "include $(LEGUPHOME)/examples/Makefile.loops\n";
  file.close();
  return;
}

void LoopFullDSE::saveLoopAsFunction(llvm::Loop *loop){

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

  //TODO save as function
  //for now we will just copy the source file
  std::error_code EC;
  std::string outfilename = std::string(outdir);
  outfilename.append("/module.bc");

  //create the file
  FILE *f = fopen(outfilename.c_str(), "w");
  fclose(f);

  int fd = open(outfilename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
  llvm::raw_fd_ostream OS(fd, true);
  llvm::WriteBitcodeToFile(module, OS);
  OS.flush();
  return;
}

bool LoopFullDSE::runOnModule(Module &M){
    module = &M;
    alloc = new Allocation(&M);
    //IMS.LI = &getAnalysis<LoopInfo>();
    //IMS.AA = &getAnalysis<AliasAnalysis>();
    //IMS.SE = &getAnalysis<ScalarEvolution>();

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
