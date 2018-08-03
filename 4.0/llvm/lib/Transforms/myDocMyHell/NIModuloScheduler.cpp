#include  <cstdlib>
#include "ILPModuloScheduler.h"
#include <lp_lib.h>

// using namespace polly;
using namespace llvm;
using namespace legup;

void ILPModuloScheduler::clearNI(){
  for(auto slot : NIMRTvailableSlots){
    delete [] slot.second;
  }
  NIMRTvailableSlots.clear();
  baseCongruenceClass.clear();
  instWidth.clear();
  widthInst.clear();
  mappedInstWidth.clear();
  conflictSolvedCongruenceClass.clear();
  conflictDelay.clear();
  mappedInstDelay.clear();
  instLinkerIndex.clear();
  startVariableIndex.clear();
  back_edge_rh_m_map.clear();
  back_edge_row_rh_map.clear();

  lateMinusSoonTimes.clear();

  constrained_insts.clear();
  conflictDelay.clear();
  mappedInstDelay.clear();
  instLinkerIndex.clear();
  slacks.clear();
  pathLengths.clear();
  orderedLoopSlacks.clear();
  orderedPathLengths.clear();
  mappedInstSlack.clear();
  orderedSlacks.clear();
  isInstOnMRT.clear();
  sortedCycleSlack.clear();

  for(auto entry : cycleSlacks){
    entry->first.clear();
    delete entry;
  }

  NIindividual.first->clear();
  delete NIindividual.first;

  if(solver.compare("gurobi")==0){
    if(NIGRBmodel!=NULL){
      delete NIGRBmodel;
      NIGRBmodel = NULL;
    }
  }
  //delete_lp(sdcSolver.lp);
}

void ILPModuloScheduler::getWidths(InstructionNode *in){

  if(mappedInstWidth[in]){
    return;
  }

  //if this instruction has no dependencyies
  if((in->dep_begin() == in->dep_end()) && (in->mem_dep_begin() == in->mem_dep_end())){

    if(NIdebug){
      File() << "inst: " << getLabel(in->getInst()) << " has no dependencies - width 0;" << '\n';
    }

    instWidth[in] = 0;
    widthInst.insert(std::pair<int, InstructionNode*>(0, in));
    mappedInstWidth[in] = true;
  }else{
    if(NIdebug){
      File() << "------ inst: " << getLabel(in->getInst()) << '\n';
    }

    int max = 0;
    //check if all dependecies were already set
    for (InstructionNode::iterator i = in->dep_begin(), e = in->dep_end(); i != e; ++i) {
      //this inst were not set yet
      if(!mappedInstWidth[*i]){
        if(NIdebug){
          File() << "dependecy not set, inst: " << getLabel((*i)->getInst()) << '\n';
        }
        return;
      }else{
        if(NIdebug){
          File() << "dependecy with " << getLabel((*i)->getInst()) << " - width: " << instWidth[*i] << '\n';
        }
        //get max between the widths
        if(instWidth[*i]>=max){
          max = instWidth[*i]+1;
        }
      }
    }

    for (InstructionNode::iterator i = in->mem_dep_begin(), e = in->mem_dep_end(); i != e; ++i) {
      //this inst were not set yet
      if(!mappedInstWidth[*i]){
        if(NIdebug){
          File() << "dependecy not set, inst: " << getLabel((*i)->getInst()) << '\n';
        }
        return;
      }else{
        if(NIdebug){
          File() << "dependecy with " << getLabel((*i)->getInst()) << " - width: " << instWidth[*i] << '\n';
        }

        if(instWidth[*i]>=max){
          max = instWidth[*i]+1;
        }
      }
    }

    if(NIdebug){
      File() << "width: " << max << '\n';
    }
    instWidth[in] = max;
    widthInst.insert(std::pair<int, InstructionNode*>(max, in));
    mappedInstWidth[in] = true;
  }

  //add instructions that use this inst
  for (InstructionNode::iterator i = in->use_begin(), e = in->use_end(); i != e; ++i) {
    if(!mappedInstWidth[*i]){
      getWidths(*i);
    }
  }

  for (InstructionNode::iterator i = in->mem_use_begin(), e = in->mem_use_end(); i != e; ++i) {
    if(!mappedInstWidth[*i]){
      getWidths(*i);
    }
  }

  return;
}

void ILPModuloScheduler::conflictIncreaseCongruence(InstructionNode *in, int inc){
  //File() << "in: " << getLabel(in->getInst()) << " - delay: " << conflictDelay[in] << '\n';
  //std::cout << "late-soon: " << lateMinusSoonTimes[in] << '\n';
  if(inc > conflictDelay[in]){
    if(NIdebug){
      File() << "new delay: " << inc << " to instr: " << getLabel(in->getInst()) << '\n';
    }
    conflictDelay[in] = inc;
  }else{
    return;
  }

  int howLateIam;
  if(conflictDelay[in] >= lateMinusSoonTimes[in]){
    std::cout << "aaaaaaaaaaaaaaaaa!!" << '\n';
    howLateIam = conflictDelay[in] - lateMinusSoonTimes[in];
  }else{
    howLateIam = conflictDelay[in];
  }
  if(NIdebug){
    File() << "dep how late = conflictDelay - lateMinusSoonTimes  ---  " << howLateIam << " = " << conflictDelay[in] << " - " << lateMinusSoonTimes[in]<< '\n';
  }
  //add instructions that use this inst

  //this eliminates double entries
  std::vector<InstructionNode*> usesvec = std::vector<InstructionNode*>();
  for (InstructionNode::iterator i = in->use_begin(), e = in->use_end(); i != e; ++i) {
    usesvec.push_back(*i);
  }
  for (InstructionNode::iterator i = in->mem_use_begin(), e = in->mem_use_end(); i != e; ++i) {
    usesvec.push_back(*i);
  }
  //std::cout << "size: " << usesvec.size() << '\n';
  auto endit = std::unique(usesvec.begin(), usesvec.end());
  usesvec.resize(std::distance(usesvec.begin(), endit));

  for (auto i : usesvec) {
    if(NIdebug){
      File() << "C" << startVariableIndex[in] << " passing delay: " << inc << " to instr: C" << startVariableIndex[i] << '\n';
    }
    conflictIncreaseCongruence(i,howLateIam);

  }

  return;
}

void ILPModuloScheduler::WFSaddInstToMRT(InstructionNode *in, int II) {

  //skip non constraint instructions
  if(!constrained_insts[in]){
    return;
  }

  //MRT * mrt = NIindividual.first;

  std::string FuName = LEGUP_CONFIG->getOpNameFromInst(in->getInst(), moduloScheduler.alloc);
  bool * availableSlots = NIMRTvailableSlots[FuName];

  int ninstances = FUlimit[FuName];
  int baseM, currM;
  int inc=0;
  bool allocated = false;

  //if(mappedInstDelay[in]){
    baseM = (conflictSolvedCongruenceClass[in]+conflictDelay[in])%II;
  //}else{
    //baseM = conflictSolvedCongruenceClass[in];
  //}

  currM = baseM;

  if(NIdebug){
    File() << "\n ----- adding an mrt slot to inst: C" << startVariableIndex[in] << " - M: " << conflictSolvedCongruenceClass[in] << " - delay: " << conflictDelay[in] << '\n';
  }

  while(!allocated){
    for(int rcnt=0; rcnt < ninstances; rcnt++){
      if(availableSlots[ninstances*currM+rcnt]){

        if(NIdebug){
          File() << "found MRT slot for inst: " << getLabel(in->getInst()) << " - at" << FuName << " with " << ninstances << " instances - "<< " (m, r) = (" << currM << ", " << rcnt << ")" << '\n';
        }

        //(*mrt)[in] = std::pair<int, int>(currM, rcnt);
        availableSlots[ninstances*currM+rcnt] = false;
        allocated = true;

        conflictSolvedCongruenceClass[in] = currM;
        conflictDelay[in] += inc;
        mappedInstDelay[in] = true;
        break;
      }
    }
    //if could not allcate it in ninstances, increase the congruence class
    if(!allocated){
      //std::cout << "inc conflict" << '\n';
      inc++;
      currM = (currM+1)%II;
    }

    if(NIdebug){
      File() << "delay: " << inc << '\n';
    }
  }

  if(inc > 0){
    conflictIncreaseCongruence(in, inc);
  }

  return;
}

void ILPModuloScheduler::LCaddInstToMRT(InstructionNode *in, int II){
  //skip non constraint instructions
  if(isInstOnMRT[in]){
    if(NIdebug){
      File() << "exiting on C" << startVariableIndex[in] << " - already on MRT\n";
    }
    return;
  }

  MRT *mrt = NIindividual.first;
  std::string FuName = LEGUP_CONFIG->getOpNameFromInst(in->getInst(), moduloScheduler.alloc);
  bool * availableSlots = NIMRTvailableSlots[FuName];

  int ninstances = FUlimit[FuName];
  int baseM, currM;
  int inc=0;
  bool allocated = false;

  if(loopheads[in]==true){
    if(NIdebug){
      File() << "\nC" << startVariableIndex[in] << " is a loop head, conflictDelay = 0";
    }
    conflictDelay[in] = 0;
  }

  //if(mappedInstOrder[in]){
  //  return;
  //}

  //if(mappedInstDelay[in]){
    baseM = (nonConstrainedASAP[in]+conflictDelay[in])%II;
  //}else{
    //baseM = conflictSolvedCongruenceClass[in];
  //}
  currM = baseM;

  //File() << "inst: " << getLabel(in->getInst()) << '\n';
  if(!constrained_insts[in]){
    if(NIdebug){
      File() << "\nnon resource constrained C" << startVariableIndex[in] << " inst: " << getLabel(in->getInst()) << " (m, -) = (" << baseM << ",-)" << " - delay: " << conflictDelay[in] << '\n';
    }
    conflictSolvedCongruenceClass[in] = baseM;
    mappedInstDelay[in] = true;
    isInstOnMRT[in] = true;
    (*mrt)[in] = std::pair<int, int>(currM, 0);
    return;
  }

  if(NIdebug){
    File() << "\n ----- adding an mrt slot to C" << startVariableIndex[in] << " inst: " << getLabel(in->getInst()) << " - M: " << baseM << " - delay: " << conflictDelay[in] << '\n';
  }

  while(!allocated){
    for(int rcnt=0; rcnt < ninstances; rcnt++){
      if(availableSlots[ninstances*currM+rcnt]){

        if(NIdebug){
          File() << "found MRT slot to C" << startVariableIndex[in] << " inst: " << getLabel(in->getInst()) << " - at" << FuName << " with " << ninstances << " instances - "<< " (m, r) = (" << currM << ", " << rcnt << ")" << '\n';
        }

        //(*mrt)[in] = std::pair<int, int>(currM, rcnt);
        availableSlots[ninstances*currM+rcnt] = false;
        allocated = true;

        conflictSolvedCongruenceClass[in] = currM;
        conflictDelay[in] += inc;
        mappedInstDelay[in] = true;
        mappedInstOrder[in] = true;
        isInstOnMRT[in] = true;
        (*mrt)[in] = std::pair<int, int>(currM, rcnt);
        break;
      }
    }
    //if could not allcate it in ninstances, increase the congruence class
    if(!allocated){
      //std::cout << "inc conflict" << '\n';
      inc++;
      currM = (currM+1)%II;
    }

    if(NIdebug){
      File() << "delay: " << inc << '\n';
    }
  }

  if(inc > 0){
    if(slacks.find(in) != slacks.end()){
      if(NIdebug){
        File() << "delay: " << inc << " - slack: " << slacks[in] << '\n';
      }
      //std::cout
      //assert(inc <= slacks[in]);
      conflictIncreaseCongruence(in, inc);
    }
  }
  return;
}

void ILPModuloScheduler::CPFaddInstToMRT(InstructionNode *in, int II){
  if(isInstOnMRT[in]){
    if(NIdebug){
      File() << "exiting on C" << startVariableIndex[in] << " - already on MRT\n";
    }
    return;
  }

  //std::cout << "\nahhhhhhhhhhhhhhh" << '\n';
  MRT *mrt = NIindividual.first;
  std::string FuName = LEGUP_CONFIG->getOpNameFromInst(in->getInst(), moduloScheduler.alloc);
  bool * availableSlots = NIMRTvailableSlots[FuName];

  int ninstances = FUlimit[FuName];
  int baseM, currM;
  int inc=0;
  bool allocated = false;

  //if(mappedInstDelay[in]){
    baseM = (conflictSolvedCongruenceClass[in]+conflictDelay[in])%II;
  //}else{
    //baseM = conflictSolvedCongruenceClass[in];
  //}

  currM = baseM;

  //skip non constraint instructions
  if(!constrained_insts[in]){
    //std::cout << "non resource constrained inst: C" << startVariableIndex[in] << " (m, -) = (" << (nonConstrainedASAP[in]+conflictDelay[in])%II << ", " << "-)" << '\n';
    if(NIdebug){
    }
    conflictSolvedCongruenceClass[in] = baseM;
    mappedInstDelay[in] = true;
    isInstOnMRT[in] = true;
    (*mrt)[in] = std::pair<int, int>(currM, 0);
    return;
  }

  if(NIdebug){
    File() << "\n ----- adding an mrt slot to inst: C" << startVariableIndex[in] << " - M: " << baseM << " - delay: " << conflictDelay[in] << '\n';
  }

  //std::cout << "/* message */" << '\n';
  while(!allocated){
    //std::cout << "tryinf new row" << '\n';
    for(int rcnt=0; rcnt < ninstances; rcnt++){
      if(availableSlots[ninstances*currM+rcnt]){

        if(NIdebug){
          File() << "found MRT slot for inst: C" << startVariableIndex[in] << " - at" << FuName << " with " << ninstances << " instances - "<< " (m, r) = (" << currM << ", " << rcnt << ")" << '\n';
        }

        //(*mrt)[in] = std::pair<int, int>(currM, rcnt);
        availableSlots[ninstances*currM+rcnt] = false;
        allocated = true;

        conflictSolvedCongruenceClass[in] = currM;
        conflictDelay[in] += inc;
        mappedInstDelay[in] = true;
        isInstOnMRT[in] = true;
        (*mrt)[in] = std::pair<int, int>(currM, rcnt);
        break;
      }
    }
    //if could not allcate it in ninstances, increase the congruence class
    if(!allocated){
      //std::cout << "inc conflict" << '\n';
      inc++;
      assert(inc < II && "oh boy seems like this MRT is full ");
      currM = (currM+1)%II;
    }

    if(NIdebug){
      File() << "delay: " << inc << '\n';
    }
  }

  //std::cout << "incrementing conflicts" << '\n';
  if(inc > 0){
    //File() << "in: " << getLabel(in->getInst()) << " - delay: " << conflictDelay[in] << '\n';
    conflictIncreaseCongruence(in, inc);
  }
  //std::cout << "incremented" << '\n';

  return;
}

using Cycle = std::pair<std::deque<InstructionNode*>, int>;
using CycleVec = std::vector<Cycle*>;

CycleVec * ILPModuloScheduler::getCycleAndSlacks(int II, InstructionNode* inode, InstructionNode *backEdgeNode, int prevDist, int backEdgeLength){
  CycleVec * retCycles = new CycleVec;

  if(nullPath[inode] == true){
    return retCycles;
  }

  int inodeSlack = -1;
  //std::cout << "inode: " << startVariableIndex[inode] << " - backedge: " << startVariableIndex[backEdgeNode] << '\n';

  std::vector<InstructionNode*> usesvec = std::vector<InstructionNode*>();

  //this eliminates double entries
  for (InstructionNode::iterator i = inode->use_begin(), e = inode->use_end(); i != e; ++i) {
    usesvec.push_back(*i);
  }
  for (InstructionNode::iterator i = inode->mem_use_begin(), e = inode->mem_use_end(); i != e; ++i) {
    usesvec.push_back(*i);
  }
  //std::cout << "size: " << usesvec.size() << '\n';
  auto endit = std::unique(usesvec.begin(), usesvec.end());
  usesvec.resize(std::distance(usesvec.begin(), endit));
  //std::cout << "size: " << usesvec.size() << "\n\n";

  for (auto i : usesvec) {
    InstructionNode * use = i;
    if(NIdebug){
      File() << "inst:" << startVariableIndex[inode] << " is used by inst:" << startVariableIndex[use]<< "\n";
      File().flush();
    }

    if(use == backEdgeNode){
      Cycle * cycle = new Cycle;
      inodeSlack = backEdgeLength - (prevDist+latencyInstMap[inode]);

      assert(inodeSlack > -1 && "slack should be >= 0 when find back edge node");
      if(NIdebug){
        File() << "inst:" << startVariableIndex[inode] << " leads to back edge node - slack: " << inodeSlack << "\n";
        File().flush();
      }

      cycle->first.push_front(use);
      cycle->first.push_front(inode);
      cycle->second = inodeSlack;
      retCycles->push_back(cycle);
    }else{
      CycleVec * tempCycles = getCycleAndSlacks(II, use, backEdgeNode, prevDist+latencyInstMap[inode], backEdgeLength);

      for(auto entry : *tempCycles){
        entry->first.push_front(inode);
      }

      retCycles->insert(retCycles->end(), tempCycles->begin(), tempCycles->end());

      if(NIdebug){
        if(tempCycles->size() != 0){
          File() << "there are paths between inst:" << startVariableIndex[inode] << " and the back edge node - prevDist: " << prevDist+latencyInstMap[inode] << "\n sclaks: ";
          for(auto entry : *tempCycles){
            File() << entry->second << ", " << "\n";
          }
        }else{
          File() << "there is NOT a path between inst:" << startVariableIndex[inode] << " and the back edge node\n";
        }
        File().flush();
      }
      delete tempCycles;
    }
  }//for (auto i : usesvec)
  //cin.get();

  if(retCycles->size() == 0){
    nullPath[inode] = true;
  }

  return retCycles;
}


int ILPModuloScheduler::getCycleSlacks(int II, InstructionNode* inode, InstructionNode *backEdgeNode, int prevDist, int backEdgeLength){
  bool inPath = false;
  int inodeSlack = -1;
  int tempSlack;

  for (InstructionNode::iterator i = inode->use_begin(), e = inode->use_end(); i != e; ++i) {
    InstructionNode * use = *i;
    if(NIdebug){
      File() << "inst:" << startVariableIndex[inode] << " is used by inst:" << startVariableIndex[use]<< "\n";
    }

    if(use == backEdgeNode){
      inPath = true;
      inodeSlack = backEdgeLength - (prevDist+latencyInstMap[inode]);

      assert(inodeSlack > -1 && "slack should be >= 0 when find back edge node");
      if(NIdebug){
        File() << "inst:" << startVariableIndex[inode] << " leads to back edge node - slack: " << inodeSlack << "\n";
      }

      // save the back edge slack as well
      if(slacks.find(use) == slacks.end()){
        slacks[use] = inodeSlack;
        if(NIdebug){
          File() << "first time backEdgeNode slack:" << slacks[use] << '\n';
        }
      }else if(inodeSlack < slacks[use]){
        if(NIdebug){
          File() << "backEdgeNode slack update:" << slacks[use] << '\n';
        }
        slacks[use] = inodeSlack;
      }else{
        if(NIdebug){
          File() << "backEdgeNode slack:" << slacks[use] << '\n';
        }
        inodeSlack = slacks[use];
      }
    }else{
      tempSlack = getCycleSlacks(II, use, backEdgeNode, prevDist+latencyInstMap[inode], backEdgeLength);

      if(tempSlack >= 0){
        if(inodeSlack == -1){//first time
          if(NIdebug){
            File() << "first time on path C" << startVariableIndex[inode] << " - tempslack: " << tempSlack << "\n";
          }
          inodeSlack = tempSlack;
        }else if(tempSlack < inodeSlack){
          if(NIdebug){
            File() << "update on path C" << startVariableIndex[inode] << " - tempslack: " << tempSlack << "\n";
          inodeSlack = tempSlack;
          }
        }
      }

      inPath = inPath || tempSlack >= 0;
      if(NIdebug){
        if(inPath){
          File() << "there is a path between inst:" << startVariableIndex[inode] << " and the back edge node - prevDist: " << prevDist+latencyInstMap[inode] << '\n';
        }else{
          File() << "there is NOT a path between inst:" << startVariableIndex[inode] << " and the back edge node\n";
        }
      }
    }
  }//for (InstructionNode::iterator i = inode->use_begin(), e = inode->use_end(); i != e; ++i)

  for (InstructionNode::iterator i = inode->mem_use_begin(), e = inode->mem_use_end(); i != e; ++i) {
    InstructionNode * use = *i;
    if(NIdebug){
      File() << "inst:" << startVariableIndex[inode] << " is used by inst:" << startVariableIndex[use]<< "\n";
    }

    if(use == backEdgeNode){
      inPath = true;
      inodeSlack = backEdgeLength - (prevDist+latencyInstMap[inode]);

      assert(inodeSlack > -1 && "slack should be >= 0 when find back edge node");
      if(NIdebug){
        File() << "inst:" << startVariableIndex[inode] << " leads to back edge node - slack: " << inodeSlack << "\n";
      }

      // save the back edge slack as well
      if(slacks.find(use) == slacks.end()){
        slacks[use] = inodeSlack;
        if(NIdebug){
          File() << "first time backEdgeNode slack:" << slacks[use] << '\n';
        }
      }else if(inodeSlack < slacks[use]){
        if(NIdebug){
          File() << "backEdgeNode slack update:" << slacks[use] << '\n';
        }
        slacks[use] = inodeSlack;
      }else{
        if(NIdebug){
          File() << "backEdgeNode slack:" << slacks[use] << '\n';
        }
        inodeSlack = slacks[use];
      }
    }
    else{
      tempSlack = getCycleSlacks(II, use, backEdgeNode, prevDist+latencyInstMap[inode], backEdgeLength);

      if(tempSlack >= 0){
        if(inodeSlack == -1){//first time
          if(NIdebug){
            File() << "first time on path C" << startVariableIndex[inode] << " - tempslack: " << tempSlack << "\n";
          }
          inodeSlack = tempSlack;
        }else if(tempSlack < inodeSlack){
          if(NIdebug){
            File() << "update on path C" << startVariableIndex[inode] << " - tempslack: " << tempSlack << "\n";
          }
          inodeSlack = tempSlack;
        }
      }

      inPath = inPath || tempSlack >= 0;
      if(NIdebug){
        if(inPath){
          File() << "there is a path between inst:" << startVariableIndex[inode] << " and the back edge node - prevDist: " << prevDist+latencyInstMap[inode] << '\n';
        }else{
          File() << "there is NOT a path between inst:" << startVariableIndex[inode] << " and the back edge node\n";
        }
      }
    }
  }//for (InstructionNode::iterator i = inode->mem_use_begin(), e = inode->mem_use_end(); i != e; ++i)
  if(NIdebug){
    File() << "C" << startVariableIndex[inode] << " - inodeSlack: " << inodeSlack << "\n";
  }
  if(inodeSlack >= 0){
    if(NIdebug){
      File() << "adding C" << startVariableIndex[inode] << " to slacks\n";
    }
    slacks[inode] = inodeSlack;
  }
  return inodeSlack;
}

int ILPModuloScheduler::getPathLenghts(InstructionNode *in, int prevDist){
  bool hasNoUse = (in->use_begin() == in->use_end()) && (in->mem_use_begin() == in->mem_use_end());

  if(pathLengths.find(in) != pathLengths.end()){
    return pathLengths[in];
  }

  //this is a last node
  if(hasNoUse || in->getInst()->isTerminator()){
    //save the length for this node.
    if(pathLengths.find(in) != pathLengths.end()){
      if(prevDist > pathLengths[in]){
        pathLengths[in] = prevDist;
      }
    }else{
      pathLengths[in] = prevDist;
    }
    //std::cout << "final var C" << startVariableIndex[in] << " associanted with path lenght: " << pathLengths[in] << '\n';
    if(NIdebug){
    }
    return prevDist;
  }

  int max = 0, uselenght;

  for(InstructionNode::iterator i = in->use_begin(), ie = in->use_end(); i != ie; i++){
    uselenght = getPathLenghts(*i, prevDist+latencyInstMap[in]);
    if(uselenght > max){
      max = uselenght;
    }
  }

  for(InstructionNode::iterator i = in->mem_use_begin(), ie = in->mem_use_end(); i != ie; i++){
    uselenght = getPathLenghts(*i, prevDist+latencyInstMap[in]);
    if(uselenght > max){
      max = uselenght;
    }
  }

  //save the length for this node.
  if(pathLengths.find(in) != pathLengths.end()){
    if(max > pathLengths[in]){
      pathLengths[in] = max;
    }
  }else{
    pathLengths[in] = max;
  }
  //std::cout << "var C" << startVariableIndex[in] << " associanted with path lenght: " << pathLengths[in] << '\n';
  if(NIdebug){
  }
  return pathLengths[in];
}

bool pathLengthOrder(const pair<InstructionNode*,std::pair<int,int>> &a,const pair<InstructionNode*,std::pair<int,int>> &b){

  if(a.second.second != b.second.second){
    return a.second.second < b.second.second;
  }else{
    return a.second.first > b.second.first;
  }
}

bool cycleSlackOrder(const pair<InstructionNode*,std::pair<int,int>> &a,const pair<InstructionNode*,std::pair<int,int>> &b){

  if(a.second.first != b.second.first){
    return a.second.first < b.second.first;
  }else{
    return a.second.second < b.second.second;
  }
}


bool inode_int_increasing_comparison(const pair<InstructionNode*,int> &a,const pair<InstructionNode*,int> &b){
  return a.second < b.second;
}

bool cycle_slack_comparison(Cycle* a,Cycle* b){
  if(a->second == b->second){
    return a->first.size() > b->first.size();
  }
  return a->second < b->second;
}

bool inode_int_decreasing_comparison(const pair<InstructionNode*,int> &a,const pair<InstructionNode*,int> &b){
  return a.second > b.second;
}

void ILPModuloScheduler::fillOrderedInsts(InstructionNode *in){
  bool alldeps = true;

  if(mappedInstOrder[in]){
    return;
  }

  for (InstructionNode::iterator i = in->dep_begin(), e = in->dep_end(); i != e; ++i){
    if(!mappedInstOrder[*i]){
      alldeps = false;
    }
  }

  for (InstructionNode::iterator i = in->mem_dep_begin(), e = in->mem_dep_end(); i != e; ++i) {
    if(!mappedInstOrder[*i]){
      alldeps = false;
    }
  }

  if(!alldeps){
    return;
  }

  for (InstructionNode::iterator i = in->use_begin(), e = in->use_end(); i != e; ++i){
    fillOrderedInsts(*i);
  }

  for (InstructionNode::iterator i = in->mem_use_begin(), e = in->mem_use_end(); i != e; ++i) {
    fillOrderedInsts(*i);
  }

  mappedInstOrder[in] = true;
  orderedInsts.push_back(in);
}

void ILPModuloScheduler::fillOrderedSlacks(InstructionNode *in){
  if(mappedInstSlack[in]){
    return;
  }

  for (InstructionNode::iterator i = in->dep_begin(), e = in->dep_end(); i != e; ++i){
    //if the instruction is in a cycle
    if(slacks.find(*i)!=slacks.end()){
      if(!mappedInstSlack[*i]){
        fillOrderedSlacks(*i);
      }
    }
  }

  for (InstructionNode::iterator i = in->mem_dep_begin(), e = in->mem_dep_end(); i != e; ++i) {
    //if the instruction is in a cycle
    if(slacks.find(*i)!=slacks.end()){
      if(!mappedInstSlack[*i]){
        fillOrderedSlacks(*i);
      }
    }
  }

  mappedInstSlack[in] = true;
  orderedSlacks.push_back(in);
  return;
}

void ILPModuloScheduler::getOrderedDataFlow(){
  if(NIdebug){
    File() << "\n---------- getting ordered data flow -----\n";
  }

  for(auto entry : orderedPathLengths){
    InstructionNode *in = entry.first;

    std::vector<std::pair<InstructionNode*, int>> uses;
    //if any deppendency was not mapped
    for (InstructionNode::iterator i = in->use_begin(), e = in->use_end(); i != e; ++i){
      uses.push_back(std::pair<InstructionNode*, int>(*i, pathLengths[*i]));
    }

    //if any memory deppendency was not mapped
    for (InstructionNode::iterator i = in->mem_use_begin(), e = in->mem_use_end(); i != e; ++i) {
      uses.push_back(std::pair<InstructionNode*, int>(*i, pathLengths[*i]));
    }

    sort(uses.begin(), uses.end(), inode_int_decreasing_comparison);
    orderedDataFlow[in] = uses;

    if(NIdebug){
      File() << "C" << startVariableIndex[in] << " is used by: ";
      for(auto entry : uses){
        File() << "C" << startVariableIndex[entry.first] << " , ";
      }
      File() << "\n";
    }
  }
  return;
}

void ILPModuloScheduler::initializeNIMRT(int II, std::string order){
  NIindividual.first = new MRT;

  for(auto fulim : FUlimit){
    NIMRTvailableSlots[fulim.first] = new bool[II*fulim.second];
    std::fill_n(NIMRTvailableSlots[fulim.first], II*fulim.second, true);
  }

  if(order.compare("loopPriorityCriticalFirst")==0){
    if(NIdebug){
      File() << "\n\n ----- calculating loop slacks---\n" << '\n';
    }

    slacks.clear();
    pathLengths.clear();
    orderedLoopSlacks.clear();
    orderedPathLengths.clear();
    orderedDataFlow.clear();
    orderedInsts.clear();
    mappedInstSlack.clear();
    orderedSlacks.clear();
    mappedInstOrder.clear();
    loopheads.clear();
    cycleSlacks.clear();
    sortedCycleSlack.clear();

    //std::cout << "/* message */" << std::endl;
    //File().flush();

    // dependency i-->j    - i is the back edge node source
    // this annotate the minimun path slack for each instruction
    //on var map<InstructionNode*, int> slacks
    bool temp = NIdebug;
    NIdebug = false;
    for(auto entry : back_edge_rh_m_map){
      InstructionNode* j = std::get<0>(entry);
      InstructionNode* i = std::get<1>(entry);
      int row = std::get<2>(entry);
      int dist = std::get<1>(back_edge_row_rh_map[row]);
      int latency = std::get<2>(back_edge_row_rh_map[row]);
      loopheads[j] = true;
      //std::cout << "getting paths between C" << startVariableIndex[i] << " -> C" << startVariableIndex[j] << '\n';
      if(NIdebug){
        File() << "loop head C" << startVariableIndex[j] << "\n";
        File().flush();
      }
      //getCycleSlacks(II, j, i, 0, II*dist-latency);
      nullPath.clear();
      CycleVec * tempVec = getCycleAndSlacks(II, j, i, 0, II*dist-latency);
      cycleSlacks.insert(cycleSlacks.end(), tempVec->begin(), tempVec->end());
      delete tempVec;
      //cin.get();
    }
    NIdebug = temp;

    if(NIdebug){
      for(auto entry : cycleSlacks){
        File() << "cycle with slack:" << entry->second << "\ninsts: ";
        for(auto inst : entry->first){
          File() << "C" << startVariableIndex[inst] << "-" << latencyInstMap[inst] << ", ";
        }
        File() << "\n";
      }
      File().flush();
    }

    std::cout << "slacks calculated" << std::endl;
    //File().flush();
    //cin.get();

    NIdebug = false;
    for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; i++) {
      if(NIdebug){
        File() << "--\n";
      }
      getWidths(dag->getInstructionNode(i));
    }
    NIdebug = temp;


    std::cout << "widths calculated" << '\n';
    /*
    for(auto entry: slacks){
      orderedLoopSlacks.push_back(std::pair<InstructionNode*, std::pair<int,int>>(entry.first, std::pair<int,int>(entry.second, instWidth[entry.first])));
    }
    sort(orderedLoopSlacks.begin(),orderedLoopSlacks.end(),cycleSlackOrder);
    */

    //order by loop slacks
    //for(auto entry: slacks){
    //  orderedLoopSlacks.push_back(std::pair<InstructionNode*, int>(entry.first, entry.second));
    //}
    //sort(orderedLoopSlacks.begin(),orderedLoopSlacks.end(),inode_int_increasing_comparison);
    sort(cycleSlacks.begin(),cycleSlacks.end(),cycle_slack_comparison);

    for(auto entry : startVariableIndex){
      sortedCycleSlack[entry.first] = false;
    }
    for(auto entry: cycleSlacks){
      for(auto inst: entry->first){
        if(sortedCycleSlack[inst] == false){
          orderedLoopSlacks.push_back(std::pair<InstructionNode*, int>(inst, entry->second));
          slacks[inst] = entry->second;
          sortedCycleSlack[inst] = true;
        }
      }
    }

    for(auto entry : orderedLoopSlacks){
      //orderedSlacks.push_back(entry.first);
      fillOrderedSlacks(entry.first);
    }

    std::cout << "odered slacks filled" << '\n';

    NIdebug = temp;

    if(NIdebug){
      File() << "-------- slacks -------\n";
      for(auto entry: slacks){
        File() << "C" << startVariableIndex[entry.first] << " - slack: " << entry.second << "\n";
      }
    }

    if(NIdebug){
      File() << "-------- ordered slacks -------\n";
      for(auto entry: orderedLoopSlacks){
        File() << "C" << startVariableIndex[entry.first] << " - slack: " << entry.second << "\n";
        //File() << "C" << startVariableIndex[entry.first] << " - slack: " << entry.second.first << " - width: " << entry.second.second << "\n";
      }
    }

    if(NIdebug){
      File() << "-------- topologically ordered insts with slack preference-------\n";
      for(auto entry: orderedSlacks){
        File() << "C" << startVariableIndex[entry] << " - slack: " << slacks[entry] << "\n";
      }
    }


    if(NIdebug){
      File() << "\n\n ----- calculating path lengths---\n" << '\n';
    }
    //this annotates the maximum path length in each instruction
    //on var map<InstructionNode*, int> pathLengths
    temp = NIdebug;
    NIdebug = false;
    for(auto entry : startVariableIndex){
      InstructionNode * i = entry.first;
      bool hasNoDep = (i->dep_begin() == i->dep_end()) && (i->mem_dep_begin() == i->mem_dep_end());

      //std::cout << "starting in instruction C" << startVariableIndex[i] << '\n';
      if(hasNoDep){
        getPathLenghts(i, 0);
      }
      //std::cout << "finished" << '\n';
    }
    NIdebug = temp;

    std::cout << "lenghts gotten" << '\n';

    if(NIdebug){
      File() << "-------- lenghts -------\n";
      for(auto entry: pathLengths){
        File() << "C" << startVariableIndex[entry.first] << " - lenght: " << entry.second << "\n";
      }
    }

    //order by path lengths
    for(auto entry: pathLengths){
      orderedPathLengths.push_back(std::pair<InstructionNode*, std::pair<int,int>>(entry.first, std::pair<int,int>(entry.second, instWidth[entry.first])));
    }
    sort(orderedPathLengths.begin(),orderedPathLengths.end(),pathLengthOrder);

    if(NIdebug){
      File() << "-------- ordered lenghts -------\n";
      for(auto entry: orderedPathLengths){
        File() << "C" << startVariableIndex[entry.first] << " - lenght: " << entry.second.first << " - width: " << entry.second.second << "\n";
      }
    }

    NIdebug = false;
    getOrderedDataFlow();

    std::cout << "ordered data-flow calculated" << '\n';

    for(auto entry : orderedPathLengths){
      fillOrderedInsts(entry.first);
    }
    NIdebug = temp;

    if(NIdebug){
      File() << "-------- topologically ordered insts with critical path preference-------\n";
      for(auto entry: orderedInsts){
        File() << "C" << startVariableIndex[entry] << "\n";
      }
    }

    //copy the base congruences so we can modify them without losing the information
    conflictSolvedCongruenceClass.insert(baseCongruenceClass.begin(), baseCongruenceClass.end());

    //std::cout << "order constructed" << std::endl;
    //File().flush();
    //cin.get();

    isInstOnMRT.clear();

    if(NIdebug){
      File() << " -----  filling mrt with cycles ------ \n";
    }

    for(auto inst : orderedSlacks){
      LCaddInstToMRT(inst, II);
    }

    std::cout << "loop insts added to the MRT" << '\n';

    if(NIdebug){
      File() << " -----  filling mrt with insts ------ \n";
    }

    for(auto inst : orderedInsts){
      CPFaddInstToMRT(inst, II);
    }

    std::cout << "other insts added to the MRT" << '\n';

  }else if(order.compare("widthFirst")==0){
    if(NIdebug){
      File() << "\n\n ----- calculating widths ---\n" << '\n';
    }

    for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; i++) {
      if(NIdebug){
        File() << "--\n";
      }
      getWidths(dag->getInstructionNode(i));
    }

    if(NIdebug){
      File() << "\n\n ----- inst widths ---\n" << '\n';
      for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; i++) {
        File() << "inst: " << getLabel(i) << " - width: " << instWidth[dag->getInstructionNode(i)] << '\n';
      }
      File() << "\n\n ----- widths insts ---\n" << '\n';
      for(auto width_inst : widthInst){
        File() << "width: " << width_inst.first << " - inst: " << getLabel(width_inst.second->getInst()) << '\n';
      }
    }

    if(NIdebug){
      File() << "\n\n ----- filling MRT ---\n" << '\n';
    }
    //copy the base congruences so we can modify them without losing the information
    conflictSolvedCongruenceClass.insert(baseCongruenceClass.begin(), baseCongruenceClass.end());

    for(auto width_inst : widthInst){
      WFSaddInstToMRT(width_inst.second, II);
    }

    MRT *mrt = NIindividual.first;
    for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; i++) {
      InstructionNode *in = dag->getInstructionNode(i);
      (*mrt)[in] = std::pair<int, int>(conflictSolvedCongruenceClass[in], 0);
    }
  }
  else{
    assert(false && "We do not recognize this order, sorry pal");
  }

  return;
}

void ILPModuloScheduler::createNIVariables() {
  assert(BB);
  if (sdcSolver.lp != NULL)
      delete_lp(sdcSolver.lp);

  //variables start at 1 in lpSolver
  numVars = 0; // LP isn't constructed yet
  numInst = 0; // the number of LLVM instructions to be scheduled
  numConstraints = 0;

  startVariableIndex.clear();
  std::cout << "here1" << '\n';
  latencyInstMap.clear();
  std::cout << "here2" << '\n';
  baseCongruenceClass.clear();
  std::cout << "here3" << '\n';
  constrained_insts.clear();
  FUinstMap.clear();
  FUlimit.clear();
  instLinkerIndex.clear();

  std::map<std::string, int> instCounts;

  // iterate over the instructions in a BB
  for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; i++) {
      InstructionNode *iNode = dag->getInstructionNode(i);

      numInst++;
      numVars++;
      startVariableIndex[iNode] = numVars;

      int delay = Scheduler::getNumInstructionCycles(i);
      if (isa<StoreInst>(i)) {
          // store, you need an extra cycle for the memory to be ready
          delay = 1;
      }

      latencyInstMap[iNode] = delay;

      if (NIdebug){
        File() << "Start Index: " << startVariableIndex[iNode]
        << " delay: " << latencyInstMap[iNode]
        << " I: " << *i << "\n";
      }

    std::string FuName = LEGUP_CONFIG->getOpNameFromInst(i, moduloScheduler.alloc);

    //check the constraint first
    int constraint;
    if (!LEGUP_CONFIG->getNumberOfFUsAllocated(FuName, &constraint)){
      constrained_insts[iNode] = false;
      continue;
    }

    //if(NIdebug){
      instCounts[FuName]++;
    //}

    constrained_insts[iNode] = true;
    if(NIdebug){
      File() << "adding constrained_inst C" << startVariableIndex[iNode] << "\n";
    }


    std::map<std::string, std::vector<InstructionNode*>>::iterator it = FUinstMap.find(FuName);
    if (it != FUinstMap.end()){
      //File() << "input on existing: " << getLabel(iNode->getInst()) << '\n';
      it->second.push_back(iNode);
      continue;
    }

    FUlimit.insert(std::pair<std::string, int>(FuName, constraint));
    //File() << "input on non-existing: " << getLabel(iNode->getInst()) << '\n';
    std::vector<InstructionNode*> insts;
    insts.push_back(iNode);
    std::pair<std::string, std::vector<InstructionNode*>> newFU = std::pair<std::string, std::vector<InstructionNode*>>(FuName, insts);
    FUinstMap.insert(newFU);
  }
  //NIdebug = true;
  /*
  if(NIdebug){
    File() << "\n\n" << '\n';
    for(auto entry:instCounts){
      File() << "resource: " << entry.first << " - limited to: " << FUlimit[entry.first] << " - is used by # insts: " << entry.second << '\n';
    }
  }
  */
  instCounts.clear();
  //NIdebug = false;
  //std::cout << "\n\naaaaaaaaaaaaaaaaaaaaaa" << '\n';

  if(link){
    for(auto imap : FUinstMap){
      for(auto inst : imap.second){
        numVars++;
        numInst++;
        instLinkerIndex[inst] = numVars;

        if (NIdebug) {
          File() << "T = "<< instLinkerIndex[inst] << ",  for : " << getLabel(inst->getInst()) << '\n';
          //inst->getInst()->dump();
        }
      }
    }

    if (NIdebug){
      File() << "SDC: # of variables: " << numVars << " # of instructions: " << numInst << "\n";
    }
  }

  sdcSolver.lp = make_lp(0, numVars);
}

void ILPModuloScheduler::addNIDependencyConstraints(InstructionNode *in) {

    int col[2];
    REAL val[2];

    for (InstructionNode::iterator i = in->dep_begin(), e = in->dep_end(); i != e; ++i) {
        // Dependency: depIn -> in
        InstructionNode *depIn = *i;
        unsigned latency = latencyInstMap[depIn];
        col[0] = startVariableIndex[in];
        val[0] = 1.0;
        col[1] = startVariableIndex[depIn];
        val[1] = -1.0;

        int chainingLatency = chaining ? 0.0 : 1.0;

        if (LEGUP_CONFIG->getParameterInt("SDC_ONLY_CHAIN_CRITICAL")) {
            Instruction *I = depIn->getInst();
            if (moduloScheduler.onCriticalPath(I)) {
                chainingLatency = 0;
            } else {
                chainingLatency = 1;
            }
        }

        int dist = moduloScheduler.distance(depIn->getInst(), in->getInst());
        assert(dist == 0);

        int b = chainingLatency+latency;
        add_constraintex(sdcSolver.lp, 2, val, col, GE, b);
        numConstraints++;

        //if(constrained_insts[in] || constrained_insts[depIn]){
          //File() << "inputting row: " << numConstraints << '\n';
          rh_m_map.push_back(std::tuple<InstructionNode*, InstructionNode*, int>(in, depIn, numConstraints));
        //}
    }

    for (InstructionNode::iterator i = in->mem_dep_begin(), e = in->mem_dep_end(); i != e; ++i) {

        // dependency from memDepIn -> in
        InstructionNode *memDepIn = *i;

        unsigned latency = latencyInstMap[memDepIn];
        //int min = (*mrt)[in].first;
        col[0] = startVariableIndex[in];
        val[0] = 1.0;
        //int mmemDepIn = (*mrt)[memDepIn].first;
        col[1] = startVariableIndex[memDepIn];
        val[1] = -1.0;

        Instruction *I1 = memDepIn->getInst();
        Instruction *I2 = in->getInst();

        // cross-iteration constraints are handled elsewhere
        // TODO: refactor cross-iteration constraints to be handled here
        assert(moduloScheduler.dependent(I1, I2));
        if (moduloScheduler.distance(I1, I2)) {
          // if(SDCdebug) File() << "Skipping due to distance = " << dist
          // << "\n";
          continue;
        }

        int b = 0+latency;
        //int newb = b-(val[0]*min+val[1]*mmemDepIn);
        add_constraintex(sdcSolver.lp, 2, val, col, GE, b);
        numConstraints++;

        //if(constrained_insts[in] || constrained_insts[memDepIn]){
          //File() << "inputting row: " << numConstraints << '\n';
          rh_m_map.push_back(std::tuple<InstructionNode*, InstructionNode*, int>(in, memDepIn, numConstraints));
        //}
    }
}

void ILPModuloScheduler::addNIDependencyConstraintsForKernel(int II) {
  assert(BB);
  rh_m_map.clear();
  back_edge_rh_m_map.clear();
  back_edge_row_rh_map.clear();
  rh_ii_dist_map.clear();

  for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; i++) {
      addNIDependencyConstraints(dag->getInstructionNode(i));
  }

  for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; ++i) {
    for (BasicBlock::iterator j = BB->begin(), je = BB->end(); j != je; ++j) {
      if (!moduloScheduler.dependent(i, j))
        continue;
      int dist = moduloScheduler.distance(i, j);
      if (!dist)
        continue;

      int col[2];
      REAL val[2];

      // cross iteration dependence from i -> j
      InstructionNode * inodei = dag->getInstructionNode(i);
      InstructionNode * inodej = dag->getInstructionNode(j);

      unsigned latency = latencyInstMap[inodei];
      col[0] = startVariableIndex[inodej];
      val[0] = 1.0;
      col[1] = startVariableIndex[inodei];
      val[1] = -1.0;

      int chainingLatency = chaining ? 0.0 : 1.0;

      if (LEGUP_CONFIG->getParameterInt("SDC_ONLY_CHAIN_CRITICAL")) {
        if (moduloScheduler.onCriticalPath(i)) {
          chainingLatency = 0;
        } else {
          chainingLatency = 1;
        }
      }

      if (isa<StoreInst>(i)) {
        // already looking at end state
        assert(chainingLatency == 0);
      }

      if (NIdebug){
        File() << "Cross-iteration constraint: start of 'j' >= end of "
               "'i' + chaining - II*distance(i, j)+latency\n";
        File() << "  chaining: " << chainingLatency << " II: " << II
               << " distance: " << dist << " latency: " << latency << "\n";
        File() << "  i: C" << startVariableIndex[inodei] << "\n";
        File() << "  j: C" << startVariableIndex[inodej] << "\n";
      }

      add_constraintex(sdcSolver.lp, 2, val, col, GE, chainingLatency-II*dist+(REAL)latency);

      numConstraints++;

      rh_ii_dist_map.push_back(std::pair<int, int>(dist, numConstraints));
      //if(constrained_insts[inodej] || constrained_insts[inodei]){
        //File() << "inputting row: " << numConstraints << '\n';
        back_edge_rh_m_map.push_back(std::tuple<InstructionNode*, InstructionNode*, int>(inodej, inodei, numConstraints));
        back_edge_row_rh_map[numConstraints] = std::tuple<int, int, int>(chainingLatency, dist, latency);
      //}
      //std::cout << "\nedge: C" << startVariableIndex[inodei] << " -> C" << startVariableIndex[inodej] << "\ndist: " << dist << " - II: " << II << " - latency: " << latency << '\n';
    }
  }
}

int ILPModuloScheduler::runNILPSolver(bool asap) {

    int *variableIndices = new int[numInst];
    REAL *variableCoefficients = new REAL[numInst];

    int count = 0;

    for (std::map<InstructionNode *, unsigned>::iterator i = startVariableIndex.begin(), e = startVariableIndex.end(); i != e; i++) {
        unsigned varIndex = i->second;
        assert(count < numInst);
        variableIndices[count] = varIndex;
        variableCoefficients[count] = 1.0;
        count++;
    }

    assert(count == numInst);
    lprec * lp;
    if(asap){
      lp = sdcSolver.lp;
    }else{
      lp = ALAPlp;
    }

    set_obj_fnex(lp, count, variableCoefficients, variableIndices);

    if(asap){
      set_minim(lp);
    }else{
      set_maxim(lp);
    }
    //std::cout << "\n\nbefore solving \n\n" << std::endl;
    //write_LP(sdcSolver.lp, stderr);
    //std::cout << "\n\n after solving \n\n" << std::endl;
    if (!NIdebug){
      set_verbose(lp, 1);
    }

    int ret;
    clock_t ticsv, tocsv;
    //std::cout << "solver: " << solver << '\n';
    if(solver.compare("gurobi")==0){
      //write_LP(lp, stderr);
      //if(GRBsolution != NULL){
      //  delete [] GRBsolution;
      //}

      if(NIGRBmodel != NULL){
        delete NIGRBmodel;
        NIGRBmodel = NULL;
      }

      char file[10] = "lp.mps";
      write_mps(lp, file);

      NIGRBmodel = new GRBModel(env, "lp.mps");
      NIGRBmodel->set(GRB_IntParam_OutputFlag, 0);

      //handles the bug when gurobi imports a Max problem from cplex
      if(!asap){
        NIGRBmodel->setObjective(-NIGRBmodel->getObjective(), GRB_MAXIMIZE);
      }

      ticsv = clock();
      NIGRBmodel->optimize();
      tocsv = clock();

      int optimstatus = NIGRBmodel->get(GRB_IntAttr_Status);

      //std::cout << "optimstatus: " << optimstatus << '\n';
      if (optimstatus == GRB_OPTIMAL) {
        ret = 0;
        double objval = NIGRBmodel->get(GRB_DoubleAttr_ObjVal);
        File() << "Optimal objective: " << objval << '\n';
      } else if (optimstatus == GRB_INFEASIBLE) {
        ret = 2;
        File() << "Model is infeasible" << '\n';
      }else if (optimstatus == GRB_UNBOUNDED) {
        File() << "Model is unbounded" << '\n';
        ret = 3;
      }else {
        File() << "Optimization was stopped with status = "<< optimstatus << '\n';
      }

      GRBsolution = NIGRBmodel->getVars();
      //NIGRBmodel->write("grbmodel.lp");
    }else{
      ticsv = clock();
      ret = solve(lp);
      tocsv = clock();
    }

    solvetime += (double)(tocsv - ticsv) / CLOCKS_PER_SEC;
    nsdcs++;

    if (NIdebug) {
      std::cout << "SDC solver status: " << ret << "\n";
    }

    delete[] variableCoefficients;
    delete[] variableIndices;
    return ret;
}

bool ILPModuloScheduler::NonResourceConstrainedASAP(int II){

  int status = runNILPSolver();

  if (status == 0) {
    REAL *variables = new REAL[numVars];

    if(solver.compare("gurobi")==0){
      for(int i=0; i<numVars; i++){
        if(NIdebug){
          File() << "sol: " << GRBsolution[i].get(GRB_StringAttr_VarName) << " = " << GRBsolution[i].get(GRB_DoubleAttr_X) << '\n';
        }
        variables[i] = GRBsolution[i].get(GRB_DoubleAttr_X);
      }
    }else{
      get_variables(sdcSolver.lp, variables);
    }

    for(auto entry: startVariableIndex){
      assert(entry.second <= (unsigned)numVars);
      nonConstrainedASAP[entry.first] = variables[entry.second-1];
    }
    return true;
  }
  File() << "Something is wrong with this problem false" << '\n';
  return false;
}

bool ILPModuloScheduler::NonResourceConstrainedALAP(int II){
  if(NIdebug){
    File() << "------------ALAP scheduling ------------" << '\n';
  }

  //just in case
  if(ALAPlp != NULL){
    delete_lp(ALAPlp);
  }

  //we will mess with this copy
  ALAPlp = copy_lp(sdcSolver.lp);
  int col[1];
  REAL val[1];

  //---- add ALAP constraints
  for(auto entry : startVariableIndex){
    InstructionNode *i = entry.first;
    int startIndex = entry.second;
    int asaptime = nonConstrainedASAP[i];

    bool hasNoUse = (i->use_begin() == i->use_end()) && (i->mem_use_begin() == i->mem_use_end());

    if(i->getInst()->isTerminator() || hasNoUse){
      if(NIdebug){
        File() << "Adding inst asap time: C" << startVariableIndex[i] << " = " << asaptime << '\n';
      }
      col[0] = startIndex;
      val[0] = 1.0;

      add_constraintex(ALAPlp, 1, val, col, EQ, asaptime);
    }
  }

  //write_LP(sdcSolver.lp, stderr);
  //write_LP(ALAPlp, stderr);

  int status = runNILPSolver(/*ASAP=*/false);

  if (status == 0) {
    REAL *variables = new REAL[numVars];

    if(solver.compare("gurobi")==0){
      for(int i=0; i<numVars; i++){
        if(NIdebug){
          File() << "sol: " << GRBsolution[i].get(GRB_StringAttr_VarName) << " = " << GRBsolution[i].get(GRB_DoubleAttr_X) << '\n';
        }
        variables[i] = GRBsolution[i].get(GRB_DoubleAttr_X);
      }
    }else{
      get_variables(ALAPlp, variables);
    }
    //get_variables(ALAPlp, variables);

    //if (NIdebug){
    //  print_solution(sdcSolver.lp, numVars);
    //}

    for(BasicBlock::iterator i = BB->begin(), ie = BB->end(); i!=ie; ++i){
      InstructionNode * inode = dag->getInstructionNode(i);
      int idx = startVariableIndex[inode];

      assert(idx <= numVars);
      nonConstrainedALAP[inode] = variables[idx-1];
      //moduloScheduler.schedTime[i] = variables[idx-1];
      lateMinusSoonTimes[inode] = variables[idx-1] - nonConstrainedASAP[inode];
    }

    if (NIdebug){
      File() << "  Found solution to ILP problem  **pleonasm detected**\n";
    }

    saveSchedule(/*lpSolve=*/true);
    return true;
  }
  File() << "Something is wrong with this problem false" << '\n';
  return false;
}

void ILPModuloScheduler::getBaseCongruenceClasses(int II){

  if(NIdebug){
    File() << "\n" << '\n';
  }

  for(auto entry : startVariableIndex){
    baseCongruenceClass[entry.first] = nonConstrainedASAP[entry.first]%II;

    //if(NIdebug){
    //  File() << "inst: C" << startVariableIndex[entry.first] << " - ASAPt: " << mNonResourceConstrainedASAP[entry.first] << " - BaseM: " << baseCongruenceClass[entry.first] << '\n';
    //}
  }
}

void ILPModuloScheduler::addLinkerConstraints(MRT *mrt, lprec *lp, int II){
  if(NIdebug){
    File() << "\n\n-------- adding linker constraints ---\n\n";
  }

  int col[2];
  REAL val[2];

  for(auto imap : FUinstMap){
    for(auto inst : imap.second){
      col[0] = startVariableIndex[inst];// varT
      val[0] = 1.0;
      col[1] = instLinkerIndex[inst];//vat x
      val[1] = -II;
      int M = (*mrt)[inst].first;
      add_constraintex(lp, 2, val, col, EQ, 0);

      // IT IS REALLY IMPORTANT TO SET THE VARIABLES TO INTEGERS
      set_int(sdcSolver.lp, instLinkerIndex[inst], TRUE);

      if(NIdebug){
        File() << "inst: " << getLabel(inst->getInst()) << " - C" << startVariableIndex[inst] << " - "<< II << "*C" << instLinkerIndex[inst] << " = " << M << '\n';
      }
    }
  }

  File() << "\n\n";
}

void ILPModuloScheduler::modifyNICongruenceLP(MRT *mrt, lprec *lp, int II){
  if(NIdebug){
    File() << "foward edges ----" << '\n';
  }

  for(auto entry : rh_m_map){
    InstructionNode* i1 = std::get<0>(entry);
    InstructionNode* i2 = std::get<1>(entry);
    int row = std::get<2>(entry);
    int m1 = (*mrt)[i1].first;
    int m2 = (*mrt)[i2].first;

    REAL oldrh = get_rh(lp, row);

    if(NIdebug){
      File() << "C" << startVariableIndex[i1] << " - " <<  "C" << startVariableIndex[i2] << " >= (" << oldrh << " - (" << m1 << " - " << m2 << "))/" << II << '\n';
    }

    set_rh(lp, row, ceil((REAL)(oldrh-(m1-m2))/II));
  }

  if(NIdebug){
    File() << "back edges ----" << '\n';
  }

  for(auto entry : back_edge_rh_m_map){
    InstructionNode* i1 = std::get<0>(entry);
    InstructionNode* i2 = std::get<1>(entry);
    int row = std::get<2>(entry);
    int m1 = (*mrt)[i1].first;
    int m2 = (*mrt)[i2].first;

    int chainingLatency = std::get<0>(back_edge_row_rh_map[row]);
    int dist = std::get<1>(back_edge_row_rh_map[row]);
    int latency = std::get<2>(back_edge_row_rh_map[row]);

    REAL oldrh = get_rh(lp, row);

    if(NIdebug){
      File() << "C" << startVariableIndex[i1] << " - " <<  "C" << startVariableIndex[i2] << " >= (" << oldrh << " - (" << m1 << " - " << m2 << "))/" << II << '\n';
      File() << "cl: " << chainingLatency << " - dist: " << dist << " - latency" << latency << '\n';

    }

    //old_rh = chainingLatency-II*dist+(REAL)latency
    set_rh(lp, row, ceil((chainingLatency+latency-dist*II-(m1-m2))/II));
  }

  //write_LP(lp, stderr);
  /*
  int ncols = get_Ncolumns(lp);
  REAL row[ncols+1];
  for(int i = 1; i<=get_Nrows(lp); i++){
    std::cout << "old_rh: " << get_rh(lp, i)/II ;
    REAL rh = get_rh(lp, i)/II;
    rh = ceil(rh);

    std::cout << "    ----   new_rh: " << rh << '\n';
    set_rh(lp, i, rh);
  }
  */

  //for(auto var : startVariableIndex){
  ///  set_int(sdcSolver.lp, var.second, TRUE);
  //}

  //write_LP(lp, stderr);
}

bool ILPModuloScheduler::evaluateNIIndividual(int II){
  MRT * mrt = NIindividual.first;
  lprec * lp = sdcSolver.lp;
  //this is where we save the schedule solution
  std::map<InstructionNode*, int> variablesT;

  modifyNICongruenceLP(mrt, lp, II);

  //if(link){
  //  addLinkerConstraints(mrt, lp, II);
  //}

  int ret = runNILPSolver(/*asap=*/ true);

  if (ret != 0) {
    std::cout << "  LP solver returned: " << ret << "\n";
    //File() << "  LP solver returned: " << ret << "\n";
    //File() << "  LP solver could not find an optimal solution\n";
    //report_fatal_error("LP solver could not find an optimal solution");
    return false;
  }
  //std::cout << "\n\n problem solved \n" << '\n';

  REAL *solution = new REAL[numVars];

  if(solver.compare("gurobi")==0){
    for(int i=0; i<numVars; i++){
      if(NIdebug){
        File() << "sol: " << GRBsolution[i].get(GRB_StringAttr_VarName) << " = " << GRBsolution[i].get(GRB_DoubleAttr_X) << '\n';
      }
      solution[i] = GRBsolution[i].get(GRB_DoubleAttr_X);
    }
  }else{
    get_variables(lp, solution);
  }
  //std::cout << "\n\n getting solution\n" << '\n';
  variablesT.clear();

  unsigned max = 0;
  for(BasicBlock::iterator id = BB->begin(), ide = BB->end(); id!=ide; ++id){
    InstructionNode * i = dag->getInstructionNode(id);
    int idx = startVariableIndex[i]-1;
    int m = conflictSolvedCongruenceClass[i];

    if(NIdebug){
      File() << "inst: C" << startVariableIndex[i] << " - variableM: " << m << " + II*solution: " << solution[idx] << " = " << (int )(m+II*solution[idx]) << '\n';
    }

    assert(solution[idx] == round(solution[idx]) && "Solution was supposed to be integer");
    variablesT[i] = m+II*solution[idx];

    if(variablesT[i]+latencyInstMap[i] > max){
      max = variablesT[i]+latencyInstMap[i];
    }
  }

  /*
  if(link){
    for(auto imap : FUinstMap){
      for(auto inst : imap.second){
        int idx = instLinkerIndex[inst]-1;

        variablesLink[inst] = solution[idx];
        //assert(solution[idx] == round(solution[idx]) && "Solution was supposed to be integer");

        File() << "inst: " << getLabel(inst->getInst()) << " ----   t = x+m = IIy + m: " << variablesT[inst] << " = " << II*variablesLink[inst] + variablesM[inst] << " = " << II << "*" << variablesLink[inst] << " + " << variablesM[inst] << '\n';
      }
    }
  }
  */

  NIindividual.second = (int)max + II*moduloScheduler.tripCount;

  if (GAdebug) {
      File() << "SDC solver status: " << ret << "\n";
  }

  delete[] solution;

  for(BasicBlock::iterator id = BB->begin(), ide = BB->end(); id!=ide; ++id){
    InstructionNode * i = dag->getInstructionNode(id);
    moduloScheduler.schedTime[id] = variablesT[i];
  }

  moduloScheduler.II = II;
  saveSchedule(/*lpSolve=*/true);
  variablesT.clear();

  return true;
}

bool ILPModuloScheduler::checkNImrt(int II){
  //clear available slots;
  for(auto slot : NIMRTvailableSlots){
    delete [] slot.second;
  }
  NIMRTvailableSlots.clear();

  //instanciate new available slots
  for(auto fulim : FUlimit){
    NIMRTvailableSlots[fulim.first] = new bool[II*fulim.second];
    std::fill_n(NIMRTvailableSlots[fulim.first], II*fulim.second, true);
  }
  //std::cout << std::boolalpha;
  for(BasicBlock::iterator i = BB->begin(), ie = BB->end(); i!=ie; ++i){

    InstructionNode * iNode = dag->getInstructionNode(i);

    if(!constrained_insts[iNode]){
      //std::cout << "non res contrained inst C" << startVariableIndex[iNode] << '\n';
      continue;
    }

    MRT *mrt = NIindividual.first;
    std::string FuName = LEGUP_CONFIG->getOpNameFromInst(i, moduloScheduler.alloc);
    bool * availableSlots = NIMRTvailableSlots[FuName];
    int ninstances = FUlimit[FuName];
    //int M = moduloScheduler.schedTime[i]%II;
    int M = (*mrt)[iNode].first;

    int instance;
    for(instance = 0; instance<ninstances; instance++){
      std::cout << "\navailableSlot: " << availableSlots[M*II+instance] << "  -  m,r: (" << M << ", " << instance << ")" << '\n';
      if(availableSlots[M*ninstances+instance]){
        std::cout << "m,r: (" << M << ", " << instance << ") at res: " << FuName << " - to inst: C" << startVariableIndex[iNode] << '\n';
        availableSlots[M*ninstances+instance] = false;
        break;
      }
    }
    std::cout << "instance: " << instance << " - ninstances: " << ninstances << '\n';
    if(instance == ninstances){
      std::cout << "conflict!!!!!!!!" << '\n';
      std::cout << "m,r: (" << M << ", " << instance << ") at res: " << FuName << " - to inst: C" << startVariableIndex[iNode] << '\n';
      return false;
    }
    //assert(instance < ninstances && "ahhhh, this MRT is not valid, I guess");
  }

  return true;
}

void ILPModuloScheduler::calculateScheduleWithDelay(int II){
  // This generates a schedule that does not satisfy the dependency constraints.
  // Use for debug and analysis only

  //MRT * mrt = NIindividual.first;
  //lprec * lp = sdcSolver.lp;

  std::map<InstructionNode*, int> variablesT;

  //REAL *solution = new REAL[numVars];
  unsigned max = 0;
  for(BasicBlock::iterator id = BB->begin(), ide = BB->end(); id!=ide; ++id){
    InstructionNode * i = dag->getInstructionNode(id);
    variablesT[i] = moduloScheduler.schedTime[id]+conflictDelay[i];

    if(NIdebug){
      File() << "inst: " << getLabel(id) << " - baseT: " << moduloScheduler.schedTime[id] << " - delay: " << conflictDelay[i] << "\n";
    }

    if(variablesT[i]+latencyInstMap[i] > max){
      max = variablesT[i];
    }
  }

  NIindividual.second = (int)max + II*moduloScheduler.tripCount;

  for(BasicBlock::iterator id = BB->begin(), ide = BB->end(); id!=ide; ++id){
    InstructionNode * i = dag->getInstructionNode(id);
    moduloScheduler.schedTime[id] = variablesT[i];
  }

  moduloScheduler.II = II;
  saveSchedule(/*lpSolve=*/true);
  variablesT.clear();

  return;
}

void ILPModuloScheduler::initiateNI(int II){
  createNIVariables();
  addNIDependencyConstraintsForKernel(II);
  lateMinusSoonTimes.clear();

  return;
}

bool ILPModuloScheduler::NI(int II){

  bool tmp = NIdebug;
  NIdebug = false;

  std::cout << "initiate" << std::endl;
  //cin.get();
  initiateNI(II);
  std::cout << "II: " << II << '\n';
  // Create variables
  bool success;
  success = NonResourceConstrainedASAP(II);
  if(success == false){
    return false;
  }
  assert(success && "something went wrong with non res. constrained ASAP");
  success = NonResourceConstrainedALAP(II);
  assert(success && "something went wrong with non res. constrained ALAP");

  std::cout << "asap and alap ready" << std::endl;
  //cin.get();

  if(NIdebug){
    File() << " ------ lateMinusSoonTimes ----- \n";
    for(auto entry : lateMinusSoonTimes){
      File() << "C" << startVariableIndex[entry.first] << " : " << nonConstrainedALAP[entry.first] << " - " << nonConstrainedASAP[entry.first] << " = " << entry.second << '\n';
      File().flush();
    }
  }

  getBaseCongruenceClasses(II);

  if(NIdebug){
    File() << "\n\n------base congruence class:" << '\n';
    for(auto entry: startVariableIndex){
      File() << "inst: C" << entry.second << " - baseM: " << baseCongruenceClass[entry.first] << '\n';
    }
    File().flush();
  }
  NIdebug = tmp;

  std::cout << "Create MRT" << std::endl;
  //cin.get();
  File().flush();
  initializeNIMRT(II);
  //NIdebug = !NIdebug;

  std::cout << "MRT created" << '\n';

  if(NIdebug){
    File() << "\n\n----- banana ---- \n\n";
    printMRT(NIindividual.first);
    File().flush();
    assert(checkNImrt(II) && "this MRT should be at least valid");
  }

  if(NIdebug){
    File() << "\n\n------conflict solved congruence class:" << '\n';
    for(auto entry: startVariableIndex){
      File() << "inst: C" << entry.second << " - newM: " << conflictSolvedCongruenceClass[entry.first] << '\n';
    }
  }

  std::cout << "solve problem" << std::endl;
  //cin.get();
  success = evaluateNIIndividual(II);
  //calculateScheduleWithDelay(II);

  if(NIdebug){
    bool validmrt;
    validmrt = checkNImrt(II);
    if(!validmrt){
      std::cout << "\nThis MRT is invalid =(\n" << '\n';
    }else{
      std::cout << "\nThis MRT is valid =)\n" << '\n';
    }
  }

  std::cout << "cleaning" << '\n';
  clearNI();
  std::cout << "returning " << success << '\n';
  //cin.get();
  //assert(success==true && "returning");
  return success;
}
