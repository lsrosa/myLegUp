#include  <cstdlib>
#include "ILPModuloScheduler.h"
#include <lp_lib.h>

// using namespace polly;
using namespace llvm;
using namespace legup;


void ILPModuloScheduler::clearSM(){
  File().flush();
  std::cout << "here1" << std::endl;
  baseCongruenceClass.clear();
  std::cout << "here2" << std::endl;
  instWidth.clear();
  widthInst.clear();
  mappedInstWidth.clear();
  conflictSolvedCongruenceClass.clear();
  conflictDelay.clear();
  mappedInstDelay.clear();
  instLinkerIndex.clear();
  back_edge_rh_m_map.clear();
  back_edge_row_rh_map.clear();
  lateMinusSoonTimes.clear();

  std::cout << "here3" << std::endl;
  std::cout << "here4" << std::endl;
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

  cycleSlacks.clear();

  std::cout << "here5" << std::endl;
  if(NIindividual.first != NULL){
    NIindividual.first->clear();
    //delete NIindividual.first;
  }
  std::cout << "here6" << std::endl;
  if(solver.compare("gurobi")==0){
    if(NIGRBmodel!=NULL){
      std::cout << "here6.5" << std::endl;
      //delete NIGRBmodel;
      NIGRBmodel = NULL;
    }
  }

  std::cout << "here7" << std::endl;

  mappedRecMIISet.clear();
  std::cout << "here8" << std::endl;
  std::cout << "size:" << SMnodeMap->size() << " - maxSize: " << SMnodeMap->max_size() << std::endl;
  
  for(auto entry:*SMnodeMap){
    delete entry.second->second;
    delete entry.second;
  }
  delete SMnodeMap;

  std::cout << "here9" << std::endl;
  recMIISets.clear();
  O.clear();
  H.clear();
  D.clear();
  startVariableIndex.clear();

  //delete_lp(sdcSolver.lp);
  File().flush();
}

using Cycle = std::pair<std::deque<InstructionNode*>, int>;
using CycleVec = std::vector<Cycle*>;
using SMnode = std::pair<InstructionNode*, std::tuple<int, int, int>*>;


CycleVec * ILPModuloScheduler::getCycleAndRecMII(int II, InstructionNode* inode, InstructionNode *backEdgeNode, int prevDist, int backEdgeDist){
  CycleVec * retCycles = new CycleVec;
  int recmii = -1;

  for (InstructionNode::iterator i = inode->use_begin(), e = inode->use_end(); i != e; ++i) {
    InstructionNode * use = *i;
    if(NIdebug){
      File() << "inst:" << startVariableIndex[inode] << " is used by inst:" << startVariableIndex[use]<< "\n";
      File().flush();
    }

    if(use == backEdgeNode){
      Cycle * cycle = new Cycle;
      recmii = ceil((prevDist+latencyInstMap[inode])/backEdgeDist);

      assert(recmii > -1 && "slack should be >= 0 when find back edge node");
      if(NIdebug){
        File() << "inst:" << startVariableIndex[inode] << " leads to back edge node - slack: " << recmii << "\n";
        File().flush();
      }

      cycle->first.push_front(use);
      cycle->first.push_front(inode);
      cycle->second = recmii;
      retCycles->push_back(cycle);
    }else{
      CycleVec * tempCycles = getCycleAndRecMII(II, use, backEdgeNode, prevDist+latencyInstMap[inode], backEdgeDist);

      for(auto entry : *tempCycles){
        entry->first.push_front(inode);
      }

      retCycles->insert(retCycles->end(), tempCycles->begin(), tempCycles->end());

      if(NIdebug){
        if(tempCycles->size() != 0){
          File() << "there are paths between inst:" << startVariableIndex[inode] << " and the back edge node - prevDist: " << prevDist+latencyInstMap[inode] << "\n RecMII: ";
          for(auto entry : *tempCycles){
            File() << entry->second << ", " << "\n";
          }
        }else{
          File() << "there is NOT a path between inst:" << startVariableIndex[inode] << " and the back edge node\n";
        }
        File().flush();
      }
    }
  }//for (InstructionNode::iterator i = inode->use_begin(), e = inode->use_end(); i != e; ++i)

  for (InstructionNode::iterator i = inode->mem_use_begin(), e = inode->mem_use_end(); i != e; ++i) {
    InstructionNode * use = *i;
    if(NIdebug){
      File() << "inst:" << startVariableIndex[inode] << " is used by inst:" << startVariableIndex[use]<< "\n";
    }

    if(use == backEdgeNode){
      Cycle * cycle = new Cycle;
      recmii = ceil((prevDist+latencyInstMap[inode])/backEdgeDist);

      assert(recmii > -1 && "slack should be >= 0 when find back edge node");
      if(NIdebug){
        File() << "inst:" << startVariableIndex[inode] << " leads to back edge node - recMII: " << recmii << "\n";
        File().flush();
      }

      cycle->first.push_front(use);
      cycle->first.push_front(inode);
      cycle->second = recmii;
      retCycles->push_back(cycle);
    }else{
      CycleVec * tempCycles = getCycleAndRecMII(II, use, backEdgeNode, prevDist+latencyInstMap[inode], backEdgeDist);

      for(auto entry : *tempCycles){
        entry->first.push_front(inode);
      }

      retCycles->insert(retCycles->end(), tempCycles->begin(), tempCycles->end());

      if(NIdebug){
        if(tempCycles->size() != 0){
          File() << "there are paths between inst:" << startVariableIndex[inode] << " and the back edge node - prevDist: " << prevDist+latencyInstMap[inode] << "\n recMII: ";
          for(auto entry : *tempCycles){
            File() << entry->second << ", " << "\n";
          }
        }else{
          File() << "there is NOT a path between inst:" << startVariableIndex[inode] << " and the back edge node\n";
        }
        File().flush();
      }
    }
  }//for (InstructionNode::iterator i = inode->mem_use_begin(), e = inode->mem_use_end(); i != e; ++i)

  return retCycles;
}

void ILPModuloScheduler::printSet(std::vector<SMnode*> in){
  if(in.size() == 0){
    File() << "empty set\n";
    return;
  }
  //std::cout << "printing set" << std::endl;
  for(auto it=in.begin(); it != in.end(); it++){
    SMnode * entry = *it;
    //std::cout << "end:" << entry->first << std::endl;
    File() << "C" << startVariableIndex[entry->first] << " - ";
  }
  //std::cout << "finised set" << std::endl;
  File() << "\n";

  File().flush();
  return;
}

std::vector<SMnode*> ILPModuloScheduler::pred(SMnode *in){
  std::vector<SMnode*> out;

  for(auto it = in->first->dep_begin(); it != in->first->dep_end(); it++){
    out.push_back((*SMnodeMap)[*it]);
  }

  for(auto it = in->first->mem_dep_begin(); it != in->first->mem_dep_end(); it++){
    out.push_back((*SMnodeMap)[*it]);
  }

  return out;
}

std::vector<SMnode*> ILPModuloScheduler::suc(SMnode *in){
  std::vector<SMnode*> out;

  for(auto it = in->first->use_begin(); it != in->first->use_end(); it++){
    out.push_back((*SMnodeMap)[*it]);
  }

  for(auto it = in->first->mem_use_begin(); it != in->first->mem_use_end(); it++){
    out.push_back((*SMnodeMap)[*it]);
  }

  return out;
}

std::vector<SMnode*> ILPModuloScheduler::predvec(){
  std::vector<SMnode*> out;

  for(auto node : O){
    InstructionNode * entry = node->first;

    for(auto it = entry->dep_begin(); it != entry->dep_end(); it++){
      InstructionNode *inst = * it;

      //check it inst is not in O
      bool found = false;
      for(auto oinst : O){
        if(oinst->first == inst){
          found = true;
        }
      }

      if(found == false){
        out.push_back((*SMnodeMap)[inst]);
      }
    }

    for(auto it = entry->mem_dep_begin(); it != entry->mem_dep_end(); it++){
      InstructionNode *inst = * it;

      //check it inst is not in O
      bool found = false;
      for(auto oinst : O){
        if(oinst->first == inst){
          found = true;
        }
      }

      if(found == false){
        out.push_back((*SMnodeMap)[inst]);
      }
    }
  }

  return out;
}

std::vector<SMnode*> ILPModuloScheduler::sucvec(){
  std::vector<SMnode*> out;

  for(auto node : O){
    InstructionNode * entry = node->first;

    for(auto it = entry->use_begin(); it != entry->use_end(); it++){
      InstructionNode *inst = * it;

      //check it inst is not in O
      bool found = false;
      for(auto oinst : O){
        if(oinst->first == inst){
          found = true;
        }
      }

      if(found == false){
        out.push_back((*SMnodeMap)[inst]);
      }
    }

    for(auto it = entry->mem_use_begin(); it != entry->mem_use_end(); it++){
      InstructionNode *inst = * it;

      //check it inst is not in O
      bool found = false;
      for(auto oinst : O){
        if(oinst->first == inst){
          found = true;
        }
      }

      if(found == false){
        out.push_back((*SMnodeMap)[inst]);
      }
    }
  }

  return out;
}

std::vector<SMnode*> ILPModuloScheduler::AminusB(std::vector<SMnode*> a, std::vector<SMnode*> b){
  std::vector<SMnode*> out;
  if(a.size() == 0)
    return out;

  //add elements from B that are not found in A
  for(auto aentry : a){
    bool found = false;
    for(auto bentry : b){
      if (aentry->first == bentry->first){
        found = true;
        break;
      }
    }
    if(found == false){
      out.push_back(aentry);
    }
  }

  return out;
}

bool ILPModuloScheduler::AinB(std::vector<SMnode*> a, std::vector<SMnode*> b){
  bool result = true;
  for(auto aentry : a){
    bool found = false;
    for(auto bentry : b){
      if (aentry->first ==  bentry->first){
        found = true;
        break;
      }
    }
    if(found == false){
      result = false;
      break;
    }
  }
  return result;
}

std::vector<SMnode*> ILPModuloScheduler::AintersecB(std::vector<SMnode*> a, std::vector<SMnode*> b){
  std::vector<SMnode*> out;

  for(auto aentry : a){
    for(auto bentry : b){
      if (aentry->first ==  bentry->first){
        out.push_back(aentry);
        break;
      }
    }
  }

  return out;
}

std::vector<SMnode*> ILPModuloScheduler::AunionB(std::vector<SMnode*> a, std::vector<SMnode*> b){

  //add everyone from a
  std::vector<SMnode*> out(a);

  //add elements from B that are not found in A
  for(auto bentry : b){
    bool found = false;
    for(auto aentry : a){
      if (aentry->first ==  bentry->first){
        found = true;
        break;
      }
      if(found == false){
        out.push_back(bentry);
      }
    }
  }

  return out;
}

bool cycle_recmii_comparison(Cycle* a,Cycle* b){
  if(a->second == b->second){
    return a->first.size() > b->first.size();
  }
  return a->second > b->second;
}

bool height_mov_comparison(SMnode *ina, SMnode *inb){
  int Ha = std::get<1>(*(ina->second));
  int Hb = std::get<1>(*(inb->second));
  int La = std::get<0>(*(ina->second));
  int Lb = std::get<0>(*(inb->second));

  if(Ha == Hb){
    return La < Lb;
  }
  return Ha > Hb;
}

bool depth_mov_comparison(SMnode *ina, SMnode *inb){
  int Da = std::get<0>(*(ina->second));
  int Db = std::get<0>(*(inb->second));
  int La = std::get<2>(*(ina->second));
  int Lb = std::get<2>(*(inb->second));

  if(Da == Db){
    return La < Lb;
  }
  return Da > Db;
}

void ILPModuloScheduler::getDepth(InstructionNode *in, int prevDepth){
  if(prevDepth+1 > D[in]){
    D[in] = prevDepth + 1;
  }

  for(auto it = in->use_begin(); it != in->use_end(); it++){
    getDepth(*it, D[in]);
  }

  for(auto it = in->mem_use_begin(); it != in->mem_use_end(); it++){
    getDepth(*it, D[in]);
  }
  return;
}

void ILPModuloScheduler::getHeight(InstructionNode *in, int prevHeight){
  if(prevHeight+1 > H[in]){
    H[in] = prevHeight + 1;
  }

  for(auto it = in->dep_begin(); it != in->dep_end(); it++){
    getHeight(*it, H[in]);
  }

  for(auto it = in->mem_dep_begin(); it != in->mem_dep_end(); it++){
    getHeight(*it, H[in]);
  }
  return;
}

void ILPModuloScheduler::getHightAndDepth(){
  for(auto entry :  startVariableIndex){
    InstructionNode * inode = entry.first;
    if((inode->dep_begin() == inode->dep_end()) && inode->mem_dep_begin() == inode->mem_dep_end()){
      getDepth(inode, -1);
    }

    if(((inode->use_begin() == inode->use_end()) && inode->mem_use_begin() == inode->mem_use_end()) || inode->getInst()->isTerminator()){
      getHeight(inode, -1);
    }
  }
}

void ILPModuloScheduler::SMaddInstToMRT(InstructionNode *in, int II){
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

  baseM = (conflictSolvedCongruenceClass[in]+conflictDelay[in])%II;
  currM = baseM;

  //skip non constraint instructions
  if(!constrained_insts[in]){
    if(NIdebug){
      File() << "non resource constrained inst: C" << startVariableIndex[in] << " (m, -) = (" << (nonConstrainedASAP[in]+conflictDelay[in])%II << ", " << "-)" << '\n';
    }
    conflictSolvedCongruenceClass[in] = (nonConstrainedASAP[in]+conflictDelay[in])%II;
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
      inc++;
      currM = (currM+1)%II;
    }

    if(NIdebug){
      File() << "delay: " << inc << '\n';
    }
  }

  if(inc > 0){
    //File() << "in: " << getLabel(in->getInst()) << " - delay: " << conflictDelay[in] << '\n';
    conflictIncreaseCongruence(in, inc);
  }

  return;
}

void ILPModuloScheduler::initializeSMMRT(int II){
  ///*NI with SM ordering
  NIindividual.first = new MRT;
  for(auto fulim : FUlimit){
    NIMRTvailableSlots[fulim.first] = new bool[II*fulim.second];
    std::fill_n(NIMRTvailableSlots[fulim.first], II*fulim.second, true);
  }
  //*/

  if(NIdebug){
    File() << "\n\n ----- calculating loop recMIIs---\n" << '\n';
  }

  mappedRecMIISet.clear();

  for(auto entry:cycleSlacks){
    //  delete entry;
  }
  cycleSlacks.clear();

  assert(recMIISets.size() == 0 && "recMIISets should be empty");

  H.clear();
  D.clear();
  O.clear();

  assert(SMnodeMap->size() == 0 && "SMnodeMap should be empty");

  conflictDelay.clear();
  std::cout << "here" << '\n';
  conflictSolvedCongruenceClass.clear();
  std::cout << "here2" << '\n';
  mappedInstDelay.clear();
  isInstOnMRT.clear();
  std::cout << "here2" << '\n';

  //std::cout << "after clear" << std::endl;
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
    //int latency = std::get<2>(back_edge_row_rh_map[row]);
    loopheads[j] = true;
    if(NIdebug){
      File() << "loop head C" << startVariableIndex[j] << "\n";
      File() << "getting paths between C" << startVariableIndex[i] << " -> C" << startVariableIndex[j] << '\n';
      File().flush();
    }
    //getCycleSlacks(II, j, i, 0, II*dist-latency);
    CycleVec * tempVec = getCycleAndRecMII(II, j, i, 0, dist);
    cycleSlacks.insert(cycleSlacks.end(), tempVec->begin(), tempVec->end());
  }
  NIdebug = temp;
  //std::cout << "after get cycle recMII" << std::endl;

  if(NIdebug){
    for(auto entry : cycleSlacks){
      File() << "cycle with recMII:" << entry->second << "\ninsts: ";
      for(auto inst : entry->first){
        File() << "C" << startVariableIndex[inst] << "-" << latencyInstMap[inst] << ", ";
      }
      File() << "\n";
    }
    File().flush();
  }

  std::cout << "middle debug" << std::endl;

  if(NIdebug){
    File() << "-------- recMIIs -------\n";
    for(auto entry: cycleSlacks){
      File() << "recMII: " << entry->second << "\n";
      for(auto i : entry->first){
        File() << "C" << startVariableIndex[i] << " - ";
      }
      File() << '\n';
    }
  }
  //std::cout << "after debug" << std::endl;

  sort(cycleSlacks.begin(),cycleSlacks.end(),cycle_recmii_comparison);
  if(NIdebug){
    File() << "-------- ordered recMIIs -------\n";
    for(auto entry: cycleSlacks){
      File() << "recMII: " << entry->second << "\n";
      for(auto i : entry->first){
        File() << "C" << startVariableIndex[i] << " - ";
      }
      File() << '\n';
    }
  }

  //get hight and depth
  getHightAndDepth();

  //creatnig RecMII sets
  for(auto entry : cycleSlacks){
    for(auto inst : entry->first){
      if(mappedRecMIISet[inst]==false){
        SMnode * node = new SMnode(inst, new std::tuple<int, int, int>(D[inst],H[inst],lateMinusSoonTimes[inst]));
        recMIISets[entry->second].push_back(node);
        mappedRecMIISet[inst]=true;
        (*SMnodeMap)[inst] = node;
      }
    }
  }
  //adding insts that do not belong to cycles to smallest priority sets
  for(auto entry : startVariableIndex){
    if(mappedRecMIISet[entry.first]==false){
      SMnode * node = new SMnode(entry.first, new std::tuple<int, int, int>(D[entry.first],H[entry.first],lateMinusSoonTimes[entry.first]));
      recMIISets[-1].push_back(node);
      mappedRecMIISet[entry.first]=true;
      (*SMnodeMap)[entry.first] = node;
    }
  }

  if(NIdebug){
    File() << "-------- recMII sets -------\n";
    //use reverse iterator to give the set with decreasing recMII order since map is an ordered struct
    for(auto entry = recMIISets.rbegin(); entry != recMIISets.rend(); entry++){
      File() << "recMII: " << entry->first << "\n";
      for(auto node : entry->second){
        InstructionNode *i = node->first;
        File() << "C" << startVariableIndex[i] << " - (D, H, Mov) = (" << std::get<0>(*(node->second)) << ", " << std::get<1>(*(node->second)) << ", " << std::get<2>(*(node->second)) << ")\n";
      }
      File() << '\n';
    }
    File().flush();
  }

  //create the bloody order
  //use reverse iterator to give the set with decreasing recMII order since map is an ordered struct

  std::vector<SMnode*> R;
  std::vector<SMnode*> predL, sucL;

  if(NIdebug){
    File() << "-------- Swing ordering -------\n";
  }

  for(auto it = recMIISets.rbegin(); it != recMIISets.rend(); it++){
    std::vector<SMnode*> S = it->second;
    if(NIdebug){
      std::cout << "new cycle" << std::endl;
      File() << "Initial S:\n";
      printSet(S);
    }

    bool bottomUpOrder = true;
    predL = predvec();
    sucL = sucvec();
    if(NIdebug){
      File() << "\npred and suc\n";
      printSet(predL);
      printSet(sucL);
    }
    if(predL.size() != 0 && AinB(predL, O)){
      //cin.get();
      R = AintersecB(predL, S);
      bottomUpOrder = true;
      if(NIdebug){
        File() << "bottom Up\n";
        printSet(R);
      }
    }else if (sucL.size() != 0 && AinB(sucL, O)){
      //cin.get();
      R = AintersecB(sucL, S);
      bottomUpOrder = false;
      if(NIdebug){
        File() << "Up Bottom\n";
        printSet(R);
      }
    }else{
      //cin.get();
      //calculate the maximum
      while(S.size() > 0){
        InstructionNode *instMax = NULL;
        int max = -1;
        for(SMnode* entry : S){
          if(nonConstrainedASAP[entry->first] == max){
            //if insts have the same ASAP time
            std::default_random_engine generator;
            std::uniform_real_distribution<double> distribution(0.0,1.0);
            if(rand() > 0.5){
              max = nonConstrainedASAP[entry->first];
              instMax = entry->first;
            }
          }else if(nonConstrainedASAP[entry->first] > max){
            //File() << "here" << std::endl;
            max = nonConstrainedASAP[entry->first];
            instMax = entry->first;
          }
        }
        R.push_back((*SMnodeMap)[instMax]);
        if(NIdebug){
          File() << "fisrt part partial R:\n";
          printSet(R);
        }
        S = AminusB(S, R);
        if(NIdebug){
          File() << "fisrt part partial S:\n";
          printSet(S);
        }
        //cin.get();
      }
      bottomUpOrder = true;
      //second part - consuming values in R
      while(R.size() != 0){
        if(bottomUpOrder == false){
          if(NIdebug){
            File() << "\n UP-Bottom\n";
            File() << "\n R size = " << R.size() << "\n";
          }
          while(R.size() != 0){

            if(NIdebug){
              File() << "sort\n";
            }
            std::sort(R.begin(), R.end(), height_mov_comparison);
            if(NIdebug){
              printSet(R);
              File() << "get v\n";
            }
            SMnode *v = R.front();
            if(NIdebug){
              File() << "add v to O\n";
            }
            O.push_back(v);
            if(NIdebug){
              printSet(O);
              File() << "erease v from R\n";
            }
            R.erase(R.begin());
            if(NIdebug){
              printSet(R);
              File() << "resulting set\n";
            }
            R = AunionB(R, AintersecB(suc(v), S));
            if(NIdebug){
              printSet(R);
            }
            //cin.get();
          }

          bottomUpOrder = true;
          if(NIdebug){
            File() << "new R\n";
          }
          R = AintersecB(predvec(), S);
          if(NIdebug){
            printSet(R);
          }
        }else{
          if(NIdebug){
            File() << "\n Bottom-UP\n";
            File() << "\n R size = " << R.size() << "\n";
          }
          while(R.size() != 0){
            if(NIdebug){
              File() << "sort\n";
            }
            std::sort(R.begin(), R.end(), depth_mov_comparison);
            if(NIdebug){
              printSet(R);
              File() << "get v\n";
            }
            SMnode *v = R.front();
            if(NIdebug){
              File() << "add v to O\n";
            }
            O.push_back(v);
            if(NIdebug){
              printSet(O);
              File() << "erease v from R\n";
            }
            R.erase(R.begin());
            if(NIdebug){
              printSet(R);
              File() << "resulting set\n";
            }
            R = AunionB(R, AintersecB(pred(v), S));
            if(NIdebug){
              printSet(R);
            }
            //cin.get();
          }

          bottomUpOrder = false;
          if(NIdebug){
            File() << "new R\n";
          }
          R = AintersecB(sucvec(), S);
          if(NIdebug){
            printSet(R);
          }
        }//endif
        ////cin.get();
      }//until R != empty
      if(NIdebug){
        File() << "after second part\n";
      }
      //cin.get();
    }
  }//end for

  if(NIdebug){
    File() << "\n---------------- ordered Insts ------------\n";
    for(auto entry : O){
      File() << "C" << startVariableIndex[entry->first] << " - ";
    }
    File() << "\n\n";
    File().flush();
  }

  ///*NI with SM ordering
  if(NIdebug){
    File() << " -----  filling mrt with insts ------ \n";
  }

  conflictSolvedCongruenceClass.insert(baseCongruenceClass.begin(), baseCongruenceClass.end());

  for(auto entry : O){
    SMaddInstToMRT(entry->first, II);
  }

  //MRT managment
  //clear available slots;
  for(auto slot : NIMRTvailableSlots){
    //delete [] slot.second;
  }
  NIMRTvailableSlots.clear();
  //*/

  return;
}

bool ILPModuloScheduler::createSMschedule(int II){
  if(NIdebug){
    File() << "\n\n--------------- Creating Schedule ----------\n";
    File().flush();
  }

  std::vector<SMnode*> predu, sucu;
  //partial schedule
  std::map<InstructionNode*, int> ps;
  //intersections for conditions
  std::vector<SMnode*> intersecPred, intersecSuc;

  //gety starting and ending nodes
  // dependency i-->j    - i is the back edge node source
  std::map<std::pair<InstructionNode*, InstructionNode*>, int> backEdge_dist_map;

  for(auto entry : back_edge_rh_m_map){
    InstructionNode* j = std::get<0>(entry);
    InstructionNode* i = std::get<1>(entry);
    int row = std::get<2>(entry);
    int dist = std::get<1>(back_edge_row_rh_map[row]);

    std::pair<InstructionNode*, InstructionNode*> key = std::pair<InstructionNode*, InstructionNode*>(j,i);

    backEdge_dist_map[key] = dist;
  }

  if(NIdebug){
    for(auto entry : backEdge_dist_map){
      File() << "back-edge: C" << startVariableIndex[entry.first.first] << " --> C" << startVariableIndex[entry.first.second] << " - dist: " << entry.second << "\n";
    }
    File().flush();
  }

  //NIMRTvailableSlots.clear();

  //instanciate new available slots
  for(auto fulim : FUlimit){
    NIMRTvailableSlots[fulim.first] = new bool[II*fulim.second];
    std::fill_n(NIMRTvailableSlots[fulim.first], II*fulim.second, true);
  }

  // now to the business - scheduler begins here
  for(auto u : O){
    predu.clear();
    sucu.clear();
    intersecPred.clear();
    intersecSuc.clear();

    predu = pred(u);
    sucu = suc(u);

    std::vector<SMnode*> psvec;
    for(auto entry : ps){
      SMnode * node = new SMnode(entry.first, new std::tuple<int, int, int>(-1,-1,-1));
      psvec.push_back(node);
    }

    intersecPred = AintersecB(predu, psvec);
    intersecSuc = AintersecB(sucu, psvec);

    //MRT for u
    std::string FuName = LEGUP_CONFIG->getOpNameFromInst(u->first->getInst(), moduloScheduler.alloc);
    bool * availableSlots = NIMRTvailableSlots[FuName];
    int ninstances = FUlimit[FuName];

    if(NIdebug){
      File() << "\n\nu = C" << startVariableIndex[u->first] << " - FuName: " << FuName << " - instances: " << ninstances << " - constrained: " << constrained_insts[u->first] << "\n";
      File() << "Pred: ";
      printSet(predu);
      File() << "Suc: ";
      printSet(sucu);
      File() << "PS set: ";
      printSet(psvec);
      File() << "pred-u intersec ps: ";
      printSet(intersecPred);
      File() << "suc-u intesec ps: ";
      printSet(intersecSuc);
      File().flush();
    }

    //if only predecessors
    if(intersecPred.size() != 0 && intersecSuc.size() == 0){
      if(NIdebug){
        File() << "only pred\n";
        File().flush();
      }
      int earlyStart = -1;

      //get early time
      for(auto entry : intersecPred){
        int t = ps[entry->first] + latencyInstMap[entry->first];
        File() << "C" << startVariableIndex[entry->first] << " - t: " << ps[entry->first] << " - pred_latency: " << latencyInstMap[entry->first];
        std::pair<InstructionNode*, InstructionNode*> key = std::pair<InstructionNode*, InstructionNode*>(entry->first, u->first);
        //if this pair of nodes is a back-edge
        if(backEdge_dist_map.find(key) != backEdge_dist_map.end()){
          t = t - II*backEdge_dist_map[key];
          File() << " - backedge dist: " << backEdge_dist_map[key];
        }
        File() << "\n";
        if(t >= earlyStart){
          earlyStart = t;
        }
      }

      //only for constrained intstructions
      if(constrained_insts[u->first]){
        //find time respecting MRT
        bool allocated = false;
        int currM = earlyStart%II;

        for(int inc=0; inc < II; inc++){
          for(int r=0; r<ninstances; r++){
            if(availableSlots[ninstances*currM+r] == true){
              availableSlots[ninstances*currM+r] = false;
              allocated = true;
              break;
            }
          }
          //if could not allcate it in ninstances, increase the congruence class
          if(allocated){
            break;
          }
          currM = (currM+1)%II;
          earlyStart++;
        }

        if(!allocated){
          if(NIdebug){
            File() << "could not find slot\n";
          }
          return false;
        }
      }

      ps[u->first] = earlyStart;
      if(NIdebug){
        File() << "scheduler with t: " << ps[u->first];
      }
    }else
    //in only successors
    if(intersecPred.size() == 0 && intersecSuc.size() != 0){
      if(NIdebug){
        File() << "only suc\n";
        File().flush();
      }
      int lateStart = BB->size()*II;

      //get late time
      for(auto entry : intersecSuc){
        int t = ps[entry->first] - latencyInstMap[u->first];

        File() << "C" << startVariableIndex[entry->first] << " - predt: " << ps[entry->first] << " - u_latency: " << latencyInstMap[u->first] << " - t: " << t;

        std::pair<InstructionNode*, InstructionNode*> key = std::pair<InstructionNode*, InstructionNode*>(entry->first,u->first);
        //if this pair of nodes is a back-edge
        if(backEdge_dist_map.find(key) != backEdge_dist_map.end()){
          t += II*backEdge_dist_map[key];
          File() << " - backedge dist: " << backEdge_dist_map[key] << " - II:" << II << " - T:" << t;
        }
        File() << " - prev lateStart:" << lateStart << "\n";
        if(t <= lateStart){
          lateStart = t;
        }
      }

      //only for resource contrained instructions
      if(constrained_insts[u->first]){
        //find time respecting MRT
        bool allocated = false;
        int currM = lateStart%II;

        for(int dec=0; dec < II && lateStart>=0; dec++){
          for(int r=0; r<ninstances; r++){
            if(availableSlots[ninstances*currM+r] == true){
              availableSlots[ninstances*currM+r] = false;
              allocated = true;
              break;
            }
          }
          //if could not allcate it in ninstances, increase the congruence class
          if(allocated){
            break;
          }
          currM = (currM-1)%II;
          lateStart--;
        }

        if(!allocated){
          if(NIdebug){
            File() << "could not find slot\n";
          }
          //cin.get();
          return false;
        }
      }
      ps[u->first] = lateStart;
      if(NIdebug){
        File() << "scheduler with t: " << ps[u->first];
      }
    }else
    //if both predecessors and successors
    if (intersecPred.size() != 0 && intersecSuc.size() != 0){
      if(NIdebug){
        File() << "both pred and suc\n";
        File().flush();
      }
      int earlyStart = -1;
      int lateStart = BB->size()*II;

      //get early time
      for(auto entry : intersecPred){
        int t = ps[entry->first] +  latencyInstMap[entry->first];
        File() << "C" << startVariableIndex[entry->first] << " - t: " << ps[entry->first] << " - pred_latency: " << latencyInstMap[entry->first];
        std::pair<InstructionNode*, InstructionNode*> key = std::pair<InstructionNode*, InstructionNode*>(entry->first,u->first);
        //if this pair of nodes is a back-edge
        if(backEdge_dist_map.find(key) != backEdge_dist_map.end()){
          t = t - II*backEdge_dist_map[key];
          File() << " - backedge dist: " << backEdge_dist_map[key];
        }
        File() << "\n";
        if(t >= earlyStart){
          earlyStart = t;
        }
      }

      //get late time
      for(auto entry : intersecSuc){
        int t = ps[entry->first] - latencyInstMap[u->first];
        File() << "C" << startVariableIndex[entry->first] << " - t: " << ps[entry->first] << " - u_latency: " << latencyInstMap[u->first];
        std::pair<InstructionNode*, InstructionNode*> key = std::pair<InstructionNode*, InstructionNode*>(entry->first, u->first);
        //if this pair of nodes is a back-edge
        if(backEdge_dist_map.find(key) != backEdge_dist_map.end()){
          t = t + II*backEdge_dist_map[key];
          File() << " - backedge dist: " << backEdge_dist_map[key];
        }

        File() << "\n";
        if(t <= lateStart){
          lateStart = t;
        }
      }

      if(earlyStart+II-1 < lateStart){
        lateStart = earlyStart+II-1;
      }

      int t = earlyStart;
      //only for resource contrained instructions
      if(constrained_insts[u->first]){
        //find time respecting MRT
        bool allocated = false;

        if(NIdebug){
          File() << "earlyStart: " << earlyStart << " - lateStart: " << lateStart << "\n";
        }
        File().flush();
        for(t=earlyStart; t <=lateStart; t++){
          int m = t%II;
          File() << "m: " << m << "\n";
          for(int r=0; r<ninstances; r++){
            if(availableSlots[ninstances*m+r] == true){
              availableSlots[ninstances*m+r] = false;
              allocated = true;
              break;
            }
          }
          //if could not allcate it in ninstances, increase the congruence class
          if(allocated){
            break;
          }
        }
        File().flush();
        if(!allocated){
          if(NIdebug){
            File() << "could not find slot\n";
          }
          //cin.get();
          return false;
        }
      }
      File().flush();
      assert(t > -1 && "something went wrong on creating SM schedule");
      ps[u->first] = t;
      if(NIdebug){
        File() << "scheduler with t: " << ps[u->first];
        File().flush();
      }
    }
    //neither predecessors or successors in the schedule
    else{
      if(NIdebug){
        File() << "neither pred or suc\n";
        File().flush();
      }

      int earlyStart = nonConstrainedASAP[u->first];
      int lateStart = earlyStart+II-1;
      int t = earlyStart;

      if(NIdebug){
        File() << "earlyStart: " << earlyStart << " - lateStart: " << lateStart << "\n";
        File().flush();
      }

      //only for resource contrained instructions
      if(constrained_insts[u->first]){
        //find time respecting MRT
        bool allocated = false;

        for(t=earlyStart; t <= lateStart; t++){
          int m = t%II;
          for(int r=0; r<ninstances; r++){
            if(availableSlots[ninstances*m+r] == true){
              availableSlots[ninstances*m+r] = false;
              allocated = true;
              break;
            }
          }
          //if could not allcate it in ninstances, increase the congruence class
          if(allocated){
            break;
          }
        }

        if(!allocated){
          if(NIdebug){
            File() << "could not find slot\n";
          }
          return false;
        }
      }
      assert(t > -1 && "something went wrong on creating SM schedule");
      ps[u->first] = t;
      if(NIdebug){
        File() << "scheduler with t: " << ps[u->first];
      }
    }

  }

  for(BasicBlock::iterator id = BB->begin(), ide = BB->end(); id!=ide; ++id){
    InstructionNode * i = dag->getInstructionNode(id);
    assert(ps.find(i) != ps.end() && "checking before saving SM schedule");
    moduloScheduler.schedTime[id] = ps[i];
    if(NIdebug){
      File() << "\n saving t: " << ps[i];
    }
    //std::cout << "ps[C" << startVariableIndex[i] << "]:" << ps[i] << '\n';
  }

  //MRT managment
  //clear available slots;
  //for(auto e:NIMRTvailableSlots){
  //  delete [] e.second;
  //}
  NIMRTvailableSlots.clear();


  moduloScheduler.II = II;
  saveSchedule(/*lpSolve=*/true);

  return true;
}

bool ILPModuloScheduler::SM(int II){

  bool tmp = NIdebug;
  SMnodeMap = new std::map<InstructionNode*, SMnode*>;
  NIdebug = false;
  initiateNI(II);
  std::cout << "II: " << II << '\n';
  // Create variables
  bool success;
  File().flush();
  std::cout << "ASAP" << std::endl;
  success = NonResourceConstrainedASAP(II);
  assert(success && "something went wrong with non res. constrained ASAP");
  File().flush();
  std::cout << "ALAP" << std::endl;
  //success = NonResourceConstrainedALAP(II);
  assert(success && "something went wrong with non res. constrained ALAP");
  File().flush();

  if(NIdebug){
    File() << " ------ lateMinusSoonTimes ----- \n";File().flush();
    for(auto entry : lateMinusSoonTimes){
      File() << "C" << startVariableIndex[entry.first] << " : " << nonConstrainedALAP[entry.first] << " - " << nonConstrainedASAP[entry.first] << " = " << entry.second << '\n';
      File().flush();
    }
    File().flush();
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
  NIdebug=false;
  std::cout << "getting order" << std::endl;
  initializeSMMRT(II);
  NIdebug = tmp;

  ///*NI with SM ordering
  if(NIdebug){
    File() << "\n\n----- Initialize MRT ---- \n\n";
    printMRT(NIindividual.first);
    File().flush();
    assert(checkNImrt(II) && "this MRT should be at least valid");
    File().flush();

    File() << "\n\n------conflict solved congruence class:" << '\n';
    for(auto entry: startVariableIndex){
      File() << "inst: C" << entry.second << " - newM: " << conflictSolvedCongruenceClass[entry.first] << '\n';
    }
    File().flush();
  }

  std::cout << "creating schedule" << std::endl;
  //success = evaluateNIIndividual(II);

  success = createSMschedule(II);
  //NIdebug=true;

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
  clearSM();
  std::cout << "returning " << success << '\n';
  //cin.get();
  //assert(success==true && "returning");
  return success;
}
