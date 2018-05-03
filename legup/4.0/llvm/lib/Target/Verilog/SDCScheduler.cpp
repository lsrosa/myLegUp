/**
 * @file SDCScheduler.cpp
 * @author janders
 * @version
 *
 * @section LICENSE
 *
 * This file is distributed under the LegUp license. See LICENSE for details.
 *
 * @section DESCRIPTION
 *
 * This file implements the SDC-based scheduler.
 */

// if (LEGUP_CONFIG->getParameterInt("PIECEWISE_SDC"))
#include "SDCScheduler.h"
#include "llvm/IR/LLVMContext.h"
#include "LegupConfig.h"
#include "EdmondMatching.h"
#include "utils.h"
#include "Ram.h"
#include <lp_lib.h>
#include <map>
#include <algorithm>

using namespace llvm;
using namespace legup;

namespace legup {

std::map<Instruction*, unsigned> asap_control_step;
std::map<Instruction*, unsigned> alap_control_step;

SDCScheduler::SDCScheduler(Allocation *alloc) : lp(0), map(0), alloc(alloc) {
  chaining = false; // default is no chaining -- maximally pipelined
  clockPeriodConstraint = -1.0; // default is no clock period constraint
  numVars = 0;                  // LP isn't constructed yet
  numInst = 0; // the number of LLVM instructions to be scheduled
  SDCdebug = LEGUP_CONFIG->getParameterInt("SDC_DEBUG");
  PWSDCdebug = LEGUP_CONFIG->getParameterInt("DEBUG_PIECEWISE_SDC") &&
               LEGUP_CONFIG->getParameterInt("PIECEWISE_SDC");
}

void SDCScheduler::createLPVariables(Function *F) {
  int bb_numVars;
  // iterate over all BBs
  for (Function::iterator b = F->begin(), be = F->end(); b != be; b++) {
    //leandro - count the numb of vars in the building block
    bb_numVars = 0;
    // iterate over the instructions in a BB
    for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
        Instruction *inst = i;

        numInst++;

        // leandro - this is for the pw version
        if (LEGUP_CONFIG->getParameterInt("PIECEWISE_SDC")) {
            numInstVec[b]++;
        }

        InstructionNode *iNode = dag->getInstructionNode(i);
        startVariableIndex[iNode] = numVars;
        int latency = Scheduler::getNumInstructionCycles(i);
      endVariableIndex[iNode] = numVars + latency;

      if (LEGUP_CONFIG->getParameterInt("PIECEWISE_SDC")){
        //leandro - correct the start and end variable indexes
        pw_startVariableIndex[iNode] = bb_numVars;
        pw_endVariableIndex[iNode] = bb_numVars + latency;
      }

      // leandro -- print which indexes corresponds to each variable
      if (SDCdebug) {
          inst->dump();
          std::cout << '\n';
          // note that the variables range is from 0 to n-1, while the problem
          // goes from 1 to n
          std::cout << "instruction is assigned to variables "
                    << startVariableIndex[iNode] + 1 << " to "
                    << endVariableIndex[iNode] + 1 << "\n\n ";
      }
      bb_numVars += (1 + latency);
      numVars += (1 + latency);

      #if 0
            if (Scheduler::getNumInstructionCycles(i))
      	printf("OPCODE: %s CYCLES: %d\n", i->getOpcodeName(), Scheduler::getNumInstructionCycles(i));
      #endif
    }

    // leandro - create one problem for each BB
    if (LEGUP_CONFIG->getParameterInt("PIECEWISE_SDC")) {
        numVarsVec[b] = bb_numVars;
        lpvec[b] = make_lp(0, bb_numVars);
    }
    // print who many and number of variables
    if (PWSDCdebug) {
        std::cout << getLabel(b) << " BB -> lp ";
        std::cout << " with " << bb_numVars << " variables created" << '\n';
    }
  }

  if (SDCdebug){
    printf("SDC: # of variables: %d # of instructions: %d\n", numVars, numInst);
  }

  // build an empty LP instance with the right # of variables
  lp = make_lp(0, numVars);
  if (PWSDCdebug) {
      std::cout << "creating lp for Func " << F->getName().str() << " with "
                << numVars << " variables" << '\n';
  }
}

void SDCScheduler::addMulticycleConstraints(Function *F) {

    // leandro
    if (SDCdebug || PWSDCdebug) {
        std::cout << "\n\n--------- Multi cycle constraints ---------\n"
                  << '\n';
        std::cout << "SDC mulicycle constraints for function" << '\n';
    }

    int col[2];
  REAL val[2];

  for (std::map<InstructionNode*, unsigned>::iterator i = startVariableIndex.begin(), e = startVariableIndex.end(); i != e; i++) {

    InstructionNode* iNode = i->first;

    if (i->second == endVariableIndex[iNode])
      continue; // not a multicycle instruction

    // add constraints so that the variable corresponding to each
    // cycle of a multiple cycle instruction gets assigned to
    // contiguous states.
    if (SDCdebug || PWSDCdebug) {
        iNode->getInst()->dump();
    }

    for (unsigned j = i->second + 1; j <= endVariableIndex[iNode]; j++) {
      col[0] = 1 + j;  // variable indicies
      col[1] = 1 + (j-1);
      val[0] = 1.0; // variable coefficients
      val[1] = -1.0;
      // there must be EXACTLY 1 cycle delay between variable j and j-1
      // leandro - printing the problem step by step,
      if (SDCdebug || PWSDCdebug) {
          std::cout << "-C" << 1 + (j - 1) << " + C" << j + 1 << "= 1"
                    << "\n\n";
      }

      add_constraintex(lp, 2, val, col, EQ, 1.0);
    }
  }

  // it will be easier to get track of the building blocks iterating this way
  if (LEGUP_CONFIG->getParameterInt("PIECEWISE_SDC")) {
      if (PWSDCdebug) {
          std::cout << "------------ SDC mulicycle constraints for BB alone "
                       "------------ " << '\n';
      }

      for (Function::iterator bb = F->begin(), bbe = F->end(); bb != bbe;
           ++bb) {
          if (PWSDCdebug) {
              std::cout << getLabel(bb) << "  -- BB ------------ " << '\n';
          }
          for (BasicBlock::iterator inst = bb->begin(), ie = bb->end();
               inst != ie; ++inst) {
              InstructionNode *iNode = dag->getInstructionNode(inst);

              if (pw_startVariableIndex[iNode] == pw_endVariableIndex[iNode])
                  continue; // not a multicycle instruction

              if (PWSDCdebug) {
                  iNode->getInst()->dump();
              }

              for (unsigned j = pw_startVariableIndex[iNode] + 1;
                   j <= pw_endVariableIndex[iNode]; j++) {
                  col[0] = 1 + j; // variable indicies
                  col[1] = 1 + (j - 1);
                  val[0] = 1.0; // variable coefficients
                  val[1] = -1.0;

                  if (PWSDCdebug) {
                      std::cout << "-C" << 1 + (j - 1) << " + C" << j + 1
                                << "= 1"
                                << "\n\n";
                  }

                  // leandro - add constraints by BB
                  add_constraintex(lpvec[bb], 2, val, col, EQ, 1.0);
              }
          }
      }
  }

  // leandro - check if the LPs have the same constraints in overall
  if (PWSDCdebug) {
      write_LP(lp, stdout);
      std::cout << "\n\n";
      for (auto lpit : lpvec) {
          std::cout << getLabel(lpit.first) << " BB constraints: ";
          write_LP(lpit.second, stdout);
          std::cout << "\n\n";
      }
  }
}

// note: source and dest must be in the same basic block
bool SDCScheduler::isDependent(InstructionNode *source, InstructionNode *dest) {
    // std::cout << "isDependent() s: " << *source->getInst() << "d: " <<
    // *dest->getInst() << "\n";

    assert(source->getInst()->getParent() == dest->getInst()->getParent());

    set <InstructionNode*> seen;
    queue <InstructionNode*> Q;
    Q.push(dest);

    while (!Q.empty()) {
        InstructionNode *cur;
        cur = Q.front();
        Q.pop();
        if (cur == source) {
            // std::cout << "Found dependency!\n";
            return true;
        }
        if (seen.find(cur) != seen.end())
            continue;
        seen.insert(cur);

        for (InstructionNode::iterator i = cur->dep_begin(), e = cur->dep_end(); i
                != e; ++i) {
            // dependency from depIn -> cur
            InstructionNode *depIn = *i;
            if(depIn->getInst()->getParent() != source->getInst()->getParent()) continue;
            Q.push(depIn);
        }
        for (InstructionNode::iterator i = cur->mem_dep_begin(),
                e = cur->mem_dep_end(); i != e; ++i) {
            // dependency from depIn -> cur
            InstructionNode *depIn = *i;
            if(depIn->getInst()->getParent() != source->getInst()->getParent()) continue;
            Q.push(depIn);
        }
    }

  return false;
}

void SDCScheduler::addDependencyConstraints(InstructionNode *in) {

  int col[2];
  REAL val[2];

  if(SDCdebug){
    in->getInst()->dump();
  }

  // First make sure each instruction is scheduled into a cycle >= 0
  col[0] = 1 + startVariableIndex[in];
  val[0] = 1.0;
  add_constraintex(lp, 1, val, col, GE, 0.0);
  if (LEGUP_CONFIG->getParameterInt("PIECEWISE_SDC")) {
      col[0] = 1 + pw_startVariableIndex[in];
      val[0] = 1.0;
      BasicBlock *bb = in->getInst()->getParent();
      add_constraintex(lpvec[bb], 1, val, col, GE, 0.0);
  }

  // Now handle the dependencies between instructions: producer/consumer
  // relationships

  for (InstructionNode::iterator i = in->dep_begin(), e = in->dep_end(); i != e;
       ++i) {
      // dependency from depIn -> in
      InstructionNode *depIn = *i;
      col[0] = 1 + startVariableIndex[in];
      val[0] = 1.0;
      col[1] = 1 + endVariableIndex[depIn];
      val[1] = -1.0;

      // leandro - printing the problem step by step
      if (SDCdebug || PWSDCdebug) {
          std::cout << "register dependency between" << '\n';
          in->getInst()->dump();
          std::cout << " and" << '\n';
          depIn->getInst()->dump();

          int inq = chaining ? 0 : 1;
          std::cout << "\n - C" << 1 + endVariableIndex[depIn] << "+ C"
                    << 1 + startVariableIndex[in] << ">= " << inq << "\n";
      }
      add_constraintex(lp, 2, val, col, GE,
                       chaining ? 0.0 : 1.0); // ensure the right ordering or
                                              // instructions based on
                                              // dependencies
      // if chaining is permitted, then the instructions can be in the SAME
      // cycle
      // if chaining is NOT permitted, a dependent instruction is moved to a
      // LATER cycle

      // leandro - adding the dependency for the picewise version
      if (LEGUP_CONFIG->getParameterInt("PIECEWISE_SDC")) {
          // dependency from depIn -> in
          InstructionNode *depIn = *i;
          col[0] = 1 + pw_startVariableIndex[in];
          val[0] = 1.0;
          col[1] = 1 + pw_endVariableIndex[depIn];
          val[1] = -1.0;
          BasicBlock *bb = in->getInst()->getParent();
          BasicBlock *depbb = depIn->getInst()->getParent();
          assert(bb == depbb && "basic blocks for consumer producer "
                                "dependencies should be the same");

          // leandro - printing the problem step by step
          if (PWSDCdebug) {
              int inq = chaining ? 0 : 1;
              std::cout << "bb wise version" << '\n';
              std::cout << " - C" << 1 + pw_endVariableIndex[depIn] << "+ C"
                        << 1 + pw_startVariableIndex[in] << ">= " << inq
                        << "\n\n";
          }
          add_constraintex(lpvec[bb], 2, val, col, GE, chaining ? 0.0 : 1.0);
      }
  }

  for (InstructionNode::iterator i = in->mem_dep_begin(), e = in->mem_dep_end();
       i != e; ++i) {

      // dependency from memDepIn -> in
      InstructionNode *memDepIn = *i;

    col[0] = 1 + startVariableIndex[in];
    val[0] = 1.0;
    col[1] = 1 + endVariableIndex[memDepIn];
    val[1] = -1.0;
    // leandro - printing the problem step by step, note that the variables
    // range
    // is from 0 to n-1, while the problem goes from 1 to n
    if (SDCdebug || PWSDCdebug) {
        std::cout << "memory dependency between" << '\n';
        in->getInst()->dump();
        std::cout << " and" << '\n';
        memDepIn->getInst()->dump();

        std::cout << " - C" << 1 + endVariableIndex[memDepIn] << "+ C"
                  << 1 + startVariableIndex[in] << ">= 0"
                  << "\n\n";
    }
    add_constraintex(lp, 2, val, col, GE, 0.0);

    // leandro - adding the dependency for the picewise version
    if (LEGUP_CONFIG->getParameterInt("PIECEWISE_SDC")) {
        // dependency from memDepIn -> in
        InstructionNode *memDepIn = *i;

        col[0] = 1 + pw_startVariableIndex[in];
        val[0] = 1.0;
        col[1] = 1 + pw_endVariableIndex[memDepIn];
        val[1] = -1.0;

        BasicBlock *bb = in->getInst()->getParent();
        BasicBlock *depbb = memDepIn->getInst()->getParent();
        assert(bb == depbb && "basic blocks for consumer producer dependencies "
                              "should be the same");

        if (PWSDCdebug) {
            std::cout << "bb wise version" << '\n';
            std::cout << " - C" << 1 + pw_endVariableIndex[memDepIn] << "+ C"
                      << 1 + pw_startVariableIndex[in] << ">= 0"
                      << "\n\n";
        }
        add_constraintex(lpvec[bb], 2, val, col, GE, 0.0);
    }
  }
}

void SDCScheduler::addDependencyConstraints(Function *F) {
    if (SDCdebug || PWSDCdebug) {
        std::cout << "\n\n--------- dependency constraints ---------\n" << '\n';
    }
    // iterate over all BBs
  for (Function::iterator b = F->begin(), be = F->end(); b != be; b++) {

    // iterate over the instructions in a BB
    for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {

      addDependencyConstraints(dag->getInstructionNode(i));
    }
  }
}

// Ensure that llvm.dbg.value instructions (used for debugging)
// are scheduled to the same cycle as the previous, non-dummy
// instruction.
void SDCScheduler::addDbgValueInstConstraints(Function *F) {
    int col[2];
    REAL val[2];

    // iterate over all BBs
    for (Function::iterator b = F->begin(), be = F->end(); b != be; b++) {

        Instruction *lastInsn = NULL;

        // iterate over the instructions in a BB
        for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
            Instruction *insn = &*i;
            if (DbgValueInst *dbgVal = dyn_cast<DbgValueInst>(insn)) {
                if (lastInsn) {
                    InstructionNode *lastInsnNode =
                        dag->getInstructionNode(lastInsn);
                    col[0] = 1 + endVariableIndex[lastInsnNode];
                    val[0] = 1.0;

                    //					dbgs() << "Constrain" << *dbgVal << " to "
                    //<<
                    //*lastInsn << "\n";
                    col[1] =
                        1 + startVariableIndex[dag->getInstructionNode(dbgVal)];
                    val[1] = -1.0;

                    // We want the llvm.dbg.insn to be scheduled to the last
                    // cycle
                    // of the lastInsn.  If start==end for the lastInsn, then
                    // just
                    // schedule to end.  However, if end>start, then schedule to
                    // end-1.
                    if (startVariableIndex[lastInsnNode] ==
                        endVariableIndex[lastInsnNode]) {
                        add_constraintex(lp, 2, val, col, EQ, 0);
                    } else {
                        add_constraintex(lp, 2, val, col, EQ, 1.0);
                    }

                    // leandro -  piece wise version
                    if (LEGUP_CONFIG->getParameterInt("PIECEWISE_SDC")) {
                        std::cout << "!!!!!!!!! debug constraints" << '\n';
                        col[0] = 1 + pw_endVariableIndex[lastInsnNode];
                        val[0] = 1.0;
                        col[1] =
                            1 + pw_startVariableIndex[dag->getInstructionNode(
                                    dbgVal)];
                        val[1] = -1.0;
                        if (pw_startVariableIndex[lastInsnNode] ==
                            pw_endVariableIndex[lastInsnNode]) {
                            add_constraintex(lpvec[b], 2, val, col, EQ, 0);
                        } else {
                            add_constraintex(lpvec[b], 2, val, col, EQ, 1.0);
                        }
                    }
                }
            }

            if (!isaDummyCall(insn)) {
                lastInsn = i;
                //				int s = 1 +
                //startVariableIndex[dag->getInstructionNode(lastInsn)];
                //				int e = 1 +
                //endVariableIndex[dag->getInstructionNode(lastInsn)];
                //				dbgs() << "New last insn: " << *lastInsn << " " << s
                //<< " " << e <<"\n";
            }
        }
    }
}

void SDCScheduler::addTimingConstraints(InstructionNode *Root, InstructionNode *Curr, float PartialPathDelay) {
    int col[2];
    REAL val[2];

    if(SDCdebug){
      Root->getInst()->dump();
    }
    // don't constraint multi-cycle operations
    // dependency has more than 1 cycle latency, so this dependency will
    // already be in another cycle.
    if (Scheduler::getNumInstructionCycles(Root->getInst()) > 0)
        return;
    if (Scheduler::getNumInstructionCycles(Curr->getInst()) > 0)
        return;

    // leandro - make sure they are in the same BB
    if (LEGUP_CONFIG->getParameterInt("PIECEWISE_SDC")) {
        assert(Root->getInst()->getParent() == Curr->getInst()->getParent() &&
               "Instruction should be in the same BB for timing constraints");
    }

    // Walk through the dependencies
    for (InstructionNode::iterator i = Curr->dep_begin(), e = Curr->dep_end();
         i != e; ++i) {
        // dependency from depNode -> Curr
        InstructionNode *depNode = *i;
        if (Scheduler::getNumInstructionCycles(depNode->getInst()) > 0)
            continue;
        //else //leandro this shall be investigated later
        //  std::cout << "no delay" << '\n';

        float delay = PartialPathDelay + depNode->getDelay();

        //leandro this shall be investigated later
        if(SDCdebug){
          std::cout << PartialPathDelay << " + "<< depNode->getDelay() << '\n';
        }

        unsigned cycleConstraint = ceil(delay / clockPeriodConstraint);

        if (cycleConstraint > 0)
            cycleConstraint--;

        if (cycleConstraint > 0) {
            // if cycleConstraint == 0, we don't need to add the constraint.
            // the reason is that such constraints are ALREADY present in the LP
            // formulation, as they are depedency constraints  -- constraints
            // that express that an operation must happen AFTER the operations
            // producuing results that it depends on
            col[0] = 1 + startVariableIndex[Root];
            val[0] = 1.0;
            col[1] = 1 + endVariableIndex[depNode];
            val[1] = -1.0;
            // leandro - printing the problem step by step
            if (SDCdebug || PWSDCdebug) {
                std::cout << "timing constraint between" << '\n';
                Root->getInst()->dump();
                std::cout << " and" << '\n';
                depNode->getInst()->dump();

                std::cout << " - C" << 1 + endVariableIndex[depNode] << "+ C"
                          << 1 + startVariableIndex[Root]
                          << ">= " << cycleConstraint << "\n\n";
            }

            add_constraintex(lp, 2, val, col, GE, cycleConstraint);

            if (SDCdebug || PWSDCdebug)
                printf("CYCLE CONSTRAINT: %u BETWEEN %d %d (delay: %f period: "
                       "%f)\n",
                       cycleConstraint, col[1], col[0], delay,
                       clockPeriodConstraint);

            if (LEGUP_CONFIG->getParameterInt("PIECEWISE_SDC")) {
                col[0] = 1 + pw_startVariableIndex[Root];
                val[0] = 1.0;
                col[1] = 1 + pw_endVariableIndex[depNode];
                val[1] = -1.0;
                // leandro - printing the problem step by step
                if (PWSDCdebug) {
                    std::cout << "bb wise version" << '\n';
                    std::cout << " - C" << 1 + endVariableIndex[depNode]
                              << "+ C" << 1 + startVariableIndex[Root]
                              << ">= " << cycleConstraint << "\n\n";
                }

                add_constraintex(lpvec[Root->getInst()->getParent()], 2, val,
                                 col, GE, cycleConstraint);

                if (SDCdebug || PWSDCdebug)
                    printf("CYCLE CONSTRAINT: %u BETWEEN %d %d (delay: %f "
                           "period: %f)\n",
                           cycleConstraint, col[1], col[0], delay,
                           clockPeriodConstraint);
            }
        } else {
            assert(cycleConstraint == 0);
            // recursive call to discover other instructions
            addTimingConstraints(Root, depNode, delay);
        }

    }
}

void SDCScheduler::addTimingConstraints(Function *F) {
    if (SDCdebug || PWSDCdebug) {
        std::cout << "\n\n--------- timing constraints ---------\n" << '\n';
    }
    // iterate over all BBs
  for (Function::iterator b = F->begin(), be = F->end(); b != be; b++) {

    // iterate over the instructions in a BB
    for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {

      InstructionNode* iNode = dag->getInstructionNode(i);
      addTimingConstraints(iNode, iNode, iNode->getDelay());

    }
  }

  return;
}


static REAL *myVariables;
static std::map<InstructionNode*, unsigned> *starts;

// used as the compare function when sorting instructions into a list
// according to their ALAP scheduling
// this is used in resource constraint handling
bool ALAPPredicate(InstructionNode* d1, InstructionNode* d2){
  int diff = (int)myVariables[((*starts)[d1])] - (int)myVariables[((*starts)[d2])];
  if (diff < 0)
    return true;
  else if (diff == 0.0) { // instructions are in the same cycle in the ALAP schedule
    // in this case we use their variable index to determine the sorted order
    // this way, such "parallel" instructions will be sort-ordered in the same way
    // as they appear in the original basic block (rather than in some arbitrary order)

    // this is needed if there are resource constraints on multiple types of things, say adders and memory ports.
    // otherwise, an unsolveable LP formulation may result
    if (((int)((*starts)[d1]) - (int)((*starts)[d2])) < 0)
      return true;
    else
      return false;
  }
  else
    return false;
}

void SDCScheduler::addResourceConstraint(Function *F, std::string constrainedFuName, unsigned constraint) {

  myVariables = new REAL[numVars];
  get_variables(lp, myVariables);

  starts = &startVariableIndex;

  std::vector<InstructionNode*> constrainedInstructions;

  // iterate over all BBs
  for (Function::iterator b = F->begin(), be = F->end(); b != be; b++) {

    constrainedInstructions.clear();

    // iterate over the instructions in a BB, collecting all the resource-constrained instructions in a container
    for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {

      InstructionNode* iNode = dag->getInstructionNode(i);
      Instruction*     inst = iNode->getInst();

      std::string FuName = LEGUP_CONFIG->getOpNameFromInst(inst, this->alloc);

      // continue if this is *not* the type of instruction being constrained
      if (FuName != constrainedFuName)
          continue;

      if (SDCdebug || PWSDCdebug) {
          std::cout << "Adding constraint for: specific FU " << FuName
                    << " I:" << getLabel(inst) << "\n";
      }

      constrainedInstructions.push_back(iNode);

    } // over the instructions

    // sort the resource-constrained instructions based on the ALAP scheduling result, as suggested in Cong's SDC paper
    std::sort(constrainedInstructions.begin(), constrainedInstructions.end(), ALAPPredicate);

    unsigned i;
    int col[2];
    REAL val[2];

    // constraints are added only between instructions within the SAME basic
    // block:
    // the reason for this is that instructions in different basic blocks
    // are guaranteed to be scheduled in different cycles by LegUp.

    /*
    #if 0
        if (constrainedInstructions.size() > 1) {
          printf("*ADD CONSTRAINT FOR ALAP: %d\n",
    (int)myVariables[startVariableIndex[constrainedInstructions[0]]]);
          printf("%s\n",
    constrainedInstructions[0]->getInst()->getOpcodeName());
        }
    #endif
    */
    for (i = constraint; i < constrainedInstructions.size(); i++) {
        // create the dependency
        // std::cout << "aaaaaaaaaahhhhhhhhhhhhhhhhh" << '\n';
        InstructionNode *i1 = constrainedInstructions[i];
        InstructionNode *i2 = constrainedInstructions[i - constraint];
        // instructions that are "constraint" indices apart in the sorted
        // order will be forced into separate cycles
        // constraint is the # of resources of the given type (e.g. # of
        // memory ports, # of dividers, etc)

        if (SDCdebug) {
            i2->getInst()->dump();
        }

        col[0] = 1 + startVariableIndex[i1];
        val[0] = 1.0;
        col[1] = 1 + startVariableIndex[i2];
        val[1] = -1;

        // there must be EXACTLY 1 cycle delay between variable j and j-1
        // leandro - printing the problem step by step,
        if (SDCdebug || PWSDCdebug) {
            int wait = Scheduler::getInitiationInterval(i1->getInst());
            std::cout << "-C" << 1 + startVariableIndex[i2] << " + C"
                      << 1 + startVariableIndex[i1] << ">= " << wait << "\n\n";
        }

        // force the two instructions to be N cycles apart (last parameter)
        // of the function call below,
        // where N is the initiation interval
        // of the instruction's shared functional unit.  The initiation
        // interval is 1 in
        // the typical case, however, it can be > 1 for certain types of
        // functional units
        // (any kind of unit that cannot accept new inputs every cycle, such
        // as a serial adder for example)
        add_constraintex(lp, 2, val, col, GE,
                         Scheduler::getInitiationInterval(i1->getInst()));
    }
  } // over the BBs of a function

  delete[] myVariables;

  return;
}

// leandro - PW version
void SDCScheduler::pw_addResourceConstraint(Function *F,
                                            std::string constrainedFuName,
                                            unsigned constraint) {

    std::map<BasicBlock *, REAL *> pw_myVariables; // = new REAL[numVars];
    starts = &pw_startVariableIndex;
    std::vector<InstructionNode *> constrainedInstructions;

    // iterate over all BBs
    for (Function::iterator b = F->begin(), be = F->end(); b != be; b++) {
        pw_myVariables[b] = new REAL[numVarsVec[b]];
        get_variables(lpvec[b], pw_myVariables[b]);

        // used by ALAPPredicate in the sorting function
        myVariables = pw_myVariables[b];

        constrainedInstructions.clear();

        // iterate over the instructions in a BB, collecting all the
        // resource-constrained instructions in a container
        for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {

            InstructionNode *iNode = dag->getInstructionNode(i);
            Instruction *inst = iNode->getInst();

            std::string FuName =
                LEGUP_CONFIG->getOpNameFromInst(inst, this->alloc);

            // continue if this is *not* the type of instruction being
            // constrained
            if (FuName != constrainedFuName)
                continue;

            if (PWSDCdebug)
                std::cout << "Adding constraint for: specific FU " << FuName
                          << " I:" << getLabel(inst) << "\n";

            constrainedInstructions.push_back(iNode);
        } // over the instructions

        // sort the resource-constrained instructions based on the ALAP
        // scheduling result, as suggested in Cong's SDC paper
        std::sort(constrainedInstructions.begin(),
                  constrainedInstructions.end(), ALAPPredicate);

        unsigned i;
        int col[2];
        REAL val[2];

        // constraints are added only between instructions within the SAME basic
        // block:
        // the reason for this is that instructions in different basic blocks
        // are guaranteed to be scheduled in different cycles by LegUp.
        /*
        #if 0
            if (constrainedInstructions.size() > 1) {
              printf("*ADD CONSTRAINT FOR ALAP: %d\n",
        (int)myVariables[startVariableIndex[constrainedInstructions[0]]]);
              printf("%s\n",
        constrainedInstructions[0]->getInst()->getOpcodeName());
            }
        #endif
        */
        for (i = constraint; i < constrainedInstructions.size(); i++) {
            // create the dependency

            InstructionNode *i1 = constrainedInstructions[i];
            InstructionNode *i2 = constrainedInstructions[i - constraint];
            // instructions that are "constraint" indices apart in the sorted
            // order will be forced into separate cycles
            // constraint is the # of resources of the given type (e.g. # of
            // memory ports, # of dividers, etc)

            if (PWSDCdebug) {
                i2->getInst()->dump();
            }

            col[0] = 1 + pw_startVariableIndex[i1];
            val[0] = 1.0;
            col[1] = 1 + pw_startVariableIndex[i2];
            val[1] = -1;

            // there must be EXACTLY 1 cycle delay between variable j and j-1
            // leandro - printing the problem step by step,
            if (PWSDCdebug) {
                int wait = Scheduler::getInitiationInterval(i1->getInst());
                std::cout << "-C" << 1 + pw_startVariableIndex[i2] << " + C"
                          << 1 + pw_startVariableIndex[i1] << ">= " << wait
                          << "\n\n";
            }

            // force the two instructions to be N cycles apart (last parameter)
            // of the function call below,
            // where N is the initiation interval
            // of the instruction's shared functional unit.  The initiation
            // interval is 1 in
            // the typical case, however, it can be > 1 for certain types of
            // functional units
            // (any kind of unit that cannot accept new inputs every cycle, such
            // as a serial adder for example)
            add_constraintex(lpvec[b], 2, val, col, GE,
                             Scheduler::getInitiationInterval(i1->getInst()));
        }
    } // over the BBs of a function

    for (auto bbit : pw_myVariables) {
        delete[] bbit.second;
    }

    return;
}

static std::map<Instruction*, unsigned> op2vertex;
static std::map<unsigned, Instruction*> vertex2op;
unsigned operationToVertex(Instruction *I) {
    if (op2vertex.find(I) != op2vertex.end()) return op2vertex[I];
    unsigned index = op2vertex.size()+1;
    op2vertex[I] = index;
    vertex2op[index] = I;
    return index;
}

Instruction *vertexToOperation(unsigned index) {
    assert (vertex2op.find(index) != vertex2op.end());
    return vertex2op[index];
}

// test for overlap. ie. Whether there exists a number C in both ranges:
// x1 <= C <= x2
// and
// y1 <= C <= y2
bool test_overlap(int x1, int x2, int y1, int y2) {
    assert(x1 <= x2);
    assert(y1 <= y2);

    return (x1 <= y2 && y1 <= x2);
}

bool overlap_start(int x1, int x2, int y1, int y2) {
    assert(test_overlap(x1, x2, y1, y2));
    return std::min(x1, y1);
}

void SDCScheduler::addMultipumpConstraints(Function *F) {

    // iterate over all BBs
    std::vector<Instruction *> multipliers;
    for (Function::iterator b = F->begin(), be = F->end(); b != be; b++) {
        // iterate over the instructions in a BB
        for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {

              if (!alloc->useExplicitDSP(i)) continue;

              multipliers.push_back(i);

              // if (asap_control_step[i] == alap_control_step[i]) continue;
              // std::cout << "I: " << *i << "\n";
              // std::cout << "\tasap: " << asap_control_step[i] << "\n";
              // std::cout << "\talap: " << alap_control_step[i] << "\n";
        }
    }

    // std::cout << "Found " << multipliers.size() << " multipliers\n";
    EdmondMatching EM;
    for (unsigned i = 0; i < multipliers.size(); i++) {
        Instruction *A = multipliers.at(i);
        // std::cout << "A: " << *A << "\n";

        for (unsigned j = i + 1; j < multipliers.size(); j++) {
            Instruction *B = multipliers.at(j);
            assert(B != A);

            // are these two multiplier compatible?

            // not in the same basic block
            if (B->getParent() != A->getParent()) continue;

            if (isDependent(dag->getInstructionNode(A),
                              dag->getInstructionNode(B))) continue;
            if (isDependent(dag->getInstructionNode(B),
                            dag->getInstructionNode(A)))
                continue;

            // do the intervals overlap?
            // std::cout << "B: " << *B << "\n";
            if (test_overlap(asap_control_step[A], alap_control_step[A],
                             asap_control_step[B], alap_control_step[B])) {
                // std::cout << "overlap\n";
                EM.addEdge(operationToVertex(A), operationToVertex(B));
            }
        }
    }

    EM.solveEdmondMatching();

    // calculate equality constraints
    int col[2];
    REAL val[2];

    unsigned matches = 0;
    for (EdmondMatching::iterator i = EM.begin(), ie = EM.end();
            i != ie; ++i) {
        EdmondMatching::EdgeType edge = *i;
        Instruction *A = vertexToOperation(edge.first);
        Instruction *B = vertexToOperation(edge.second);
        // std::cout << "Paired:\n";
        // std::cout << "A: " << *A << "\n";
        // std::cout << "B: " << *B << "\n";
        /*
        unsigned cycleConstraint =
            overlap_start(asap_control_step[A], alap_control_step[A],
                          asap_control_step[B], alap_control_step[B]);
        */

        matches++;

        // set an equality constraint to put both multipliers in
        // the same state
        col[0] = 1 + startVariableIndex[dag->getInstructionNode(A)];
        val[0] = 1.0;
        col[1] = 1 + startVariableIndex[dag->getInstructionNode(B)];
        val[1] = -1.0;
        add_constraintex(lp, 2, val, col, EQ, 0);

        col[0] = 1 + endVariableIndex[dag->getInstructionNode(A)];
        val[0] = 1.0;
        col[1] = 1 + endVariableIndex[dag->getInstructionNode(B)];
        val[1] = -1.0;
        add_constraintex(lp, 2, val, col, EQ, 0);

        if (SDCdebug) {
            printf("Multipumping. inst: %s (idx: %u) and inst: %s (idx: %u) "
                   "constrained to same cycle\n", A->getOpcodeName(),
                   col[0], B->getOpcodeName(), col[1]);
        }
    }

    // std::cout << "Found " << matches << " pairs\n";
}

void SDCScheduler::addALAPConstraints(Function *F) {

  REAL* variables = new REAL[numVars];
  get_variables(lp, variables);

  // maxCycle maps a basic block index to the number of cycles
  // in that BB (according to ASAP scheduling done previously)
  // note: the map will default to 0
  std::map<BasicBlock *, int> maxCycle;

  for (Function::iterator bb = F->begin(), be = F->end(); bb != be; bb++) {
      for (BasicBlock::iterator i = bb->begin(), ie = bb->end(); i != ie; i++) {
          int idx = endVariableIndex[dag->getInstructionNode(i)];
          int stateAssigned = variables[idx];
          maxCycle[bb] = std::max(maxCycle[bb], stateAssigned);
      }
      if (SDCdebug || PWSDCdebug) {
          printf("After ASAP scheduling BB: %s takes %d cycles\n",
                 getLabel(bb).c_str(), maxCycle[bb]);
      }
  }

  // now add the constraints
  int col[1];
  REAL val[1];

  if (SDCdebug || PWSDCdebug) {
      std::cout << "\n\n--------- ALAP constraints ---------\n" << '\n';
  }

  for (Function::iterator bb = F->begin(), be = F->end(); bb != be; bb++) {
      for (BasicBlock::iterator i = bb->begin(), ie = bb->end(); i != ie; i++) {

          // leandro
          if (SDCdebug || PWSDCdebug) {
              i->dump();
          }

          InstructionNode* iNode = dag->getInstructionNode(i);
          int startIdx = startVariableIndex[iNode];
          int endIdx = endVariableIndex[iNode];

          col[0] = 1 + endIdx;
          val[0] = 1.0;

          if (!i->isTerminator() && (iNode->dep_begin() == iNode->dep_end()) && ((unsigned)variables[startIdx] == 0)) {
              // this instruction isn't a terminator
              // and it has no dependencies
              REAL endState = endIdx - startIdx;
              int latency = Scheduler::getNumInstructionCycles(i);
              assert(endState == latency);

              // leandro - instructions without dependencies are schedule in the
              // begin
              if (SDCdebug || PWSDCdebug) {
                  std::cout << "C" << 1 + endIdx << "= " << endState << "\n\n";
              }

              add_constraintex(lp, 1, val, col, EQ, endState);
              if (SDCdebug || PWSDCdebug) {
                  printf("INST: %s (IDX: %u) CONSTRAINED TO CYCLE 0.\n",
                         i->getOpcodeName(), col[0]);
              }
          } else {

              // leandro - other instructions are schedule just before the bb
              // ends
              if (SDCdebug || PWSDCdebug) {
                  std::cout << "C" << 1 + endIdx << "<= " << (REAL)maxCycle[bb]
                            << "\n\n";
              }

              add_constraintex(lp, 1, val, col, LE, (REAL)maxCycle[bb]);
              if (SDCdebug || PWSDCdebug) {
                  printf("INST: %s (IDX: %u) CONSTRAINED TO CYCLE LEQ: %u\n",
                         i->getOpcodeName(), col[0], maxCycle[bb]);
              }
          }
      }
  }

  delete [] variables;

  return;
}

// leandro - functions were getting to messy start implementing pw version on
// separated functions
void SDCScheduler::pw_addALAPConstraints(Function *F) {
    //    PWSDCdebug
    std::map<BasicBlock *, REAL *> variables;
    // maxCycle maps a basic block index to the number of cycles
    // in that BB (according to ASAP scheduling done previously)
    // note: the map will default to 0
    std::map<BasicBlock *, int> maxCycle;

    for (Function::iterator bb = F->begin(), be = F->end(); bb != be; bb++) {
        variables[bb] = new REAL[numVarsVec[bb]];
        get_variables(lpvec[bb], variables[bb]);

        for (BasicBlock::iterator i = bb->begin(), ie = bb->end(); i != ie;
             i++) {
            int idx = pw_endVariableIndex[dag->getInstructionNode(i)];
            int stateAssigned = variables[bb][idx];
            maxCycle[bb] = std::max(maxCycle[bb], stateAssigned);
        }
        if (PWSDCdebug) {
            printf("After PW ASAP scheduling BB: %s takes %d cycles\n",
                   getLabel(bb).c_str(), maxCycle[bb]);
        }
    }

    // now add the constraints
    int col[1];
    REAL val[1];

    if (PWSDCdebug) {
        std::cout << "\n\n--------- PW ALAP constraints ---------\n" << '\n';
    }

    for (Function::iterator bb = F->begin(), be = F->end(); bb != be; bb++) {
        for (BasicBlock::iterator i = bb->begin(), ie = bb->end(); i != ie;
             i++) {

            // leandro
            if (PWSDCdebug) {
                i->dump();
            }

            InstructionNode *iNode = dag->getInstructionNode(i);
            int startIdx = pw_startVariableIndex[iNode];
            int endIdx = pw_endVariableIndex[iNode];

            col[0] = 1 + endIdx;
            val[0] = 1.0;

            if (!i->isTerminator() &&
                (iNode->dep_begin() == iNode->dep_end()) &&
                ((unsigned)variables[bb][startIdx] == 0)) {
                // this instruction isn't a terminator
                // and it has no dependencies
                REAL endState = endIdx - startIdx;
                int latency = Scheduler::getNumInstructionCycles(i);
                assert(endState == latency);

                // leandro - instructions without dependencies are schedule in
                // the begin
                if (PWSDCdebug) {
                    std::cout << "C" << 1 + endIdx << "= " << endState
                              << "\n\n";
                }

                add_constraintex(lpvec[bb], 1, val, col, EQ, endState);
                if (PWSDCdebug) {
                    printf("INST: %s (IDX: %u) CONSTRAINED TO CYCLE 0.\n",
                           i->getOpcodeName(), col[0]);
                }
            } else {

                // leandro - other instructions are schedule just before the bb
                // ends
                if (PWSDCdebug) {
                    std::cout << "C" << 1 + endIdx
                              << "<= " << (REAL)maxCycle[bb] << "\n\n";
                }

                add_constraintex(lpvec[bb], 1, val, col, LE,
                                 (REAL)maxCycle[bb]);
                if (PWSDCdebug) {
                    printf("INST: %s (IDX: %u) CONSTRAINED TO CYCLE LEQ: %u\n",
                           i->getOpcodeName(), col[0], maxCycle[bb]);
                }
            }
        }
    }

    for (auto bbit : variables) {
        delete[] bbit.second;
    }

    return;
}

void SDCScheduler::depositSchedule(Function *F) {
    REAL *variables = new REAL[numVars];
    get_variables(lp, variables);

  int count = 0;

  // iterate over all BBs
  for (Function::iterator b = F->begin(), be = F->end(); b != be; b++) {
      unsigned numStatesInBB = 0;
      int pipelined = getMetadataInt(b->getTerminator(), "legup.pipelined");

      if (SDCdebug || PWSDCdebug) {
          std::cout << "BasicBlock" << getLabel(b) << '\n';
      }

      // iterate over the instructions in a BB
      for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
          unsigned idx = startVariableIndex[dag->getInstructionNode(i)];
          unsigned stateAssigned = (unsigned)variables[idx];

          if (SDCdebug || PWSDCdebug) {
              std::cout << "comparing: " << idx << " and: " << stateAssigned
                        << '\n';
              // printf("BB: %d INSTR OPCODE: %s (IDX: %d) CLOCK CYCLE ASSIGNED:
              // %u\n", count, i->getOpcodeName(), idx+1, stateAssigned);
          }

          map->setState(dag->getInstructionNode(i), stateAssigned);

      // add extra cycles for multi-cycle instructions
      // except for pipelining where iterative modulo scheduling has already
      // handled multi-cycle instructions
      if (!pipelined) {
          stateAssigned += Scheduler::getNumInstructionCycles(i);
      }

      if (stateAssigned > numStatesInBB) {
          numStatesInBB = stateAssigned;
      }

    }
    count++;

    map->setNumStates(b, numStatesInBB);

    // ??? janders -- not sure if the code below is NEEDED
    // this was borrowed from the previous implementation
    Instruction *inst = b->getTerminator();
    if (isa<ReturnInst>(inst)) {
      InstructionNode *in = dag->getInstructionNode(inst);
      map->setState(in, numStatesInBB);
    }
  }

  delete[] variables;
}

void SDCScheduler::pw_depositSchedule(Function *F) {
    std::map<BasicBlock *, REAL *> variables; // = new REAL[numVars];
    int count = 0;

    // iterate over all BBs
    for (Function::iterator b = F->begin(), be = F->end(); b != be; b++) {
        variables[b] = new REAL[numVarsVec[b]];
        get_variables(lpvec[b], variables[b]);

        unsigned numStatesInBB = 0;
        int pipelined = getMetadataInt(b->getTerminator(), "legup.pipelined");

        if (PWSDCdebug) {
            std::cout << "BasicBlock" << getLabel(b) << '\n';
        }
        // iterate over the instructions in a BB
        for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
            unsigned idx = pw_startVariableIndex[dag->getInstructionNode(i)];
            unsigned stateAssigned = (unsigned)variables[b][idx];
            if (PWSDCdebug) {
                std::cout << "comparing: " << idx << " and: " << stateAssigned
                          << '\n';
                // printf("BB: %d INSTR OPCODE: %s (IDX: %d) CLOCK CYCLE
                // ASSIGNED: %u\n", count, i->getOpcodeName(), idx+1,
                // stateAssigned);
            }

            pw_map->setState(dag->getInstructionNode(i), stateAssigned);

            // add extra cycles for multi-cycle instructions
            // except for pipelining where iterative modulo scheduling has
            // already
            // handled multi-cycle instructions
            if (!pipelined) {
                stateAssigned += Scheduler::getNumInstructionCycles(i);
            }

            if (stateAssigned > numStatesInBB) {
                numStatesInBB = stateAssigned;
            }
        }
        count++;

        pw_map->setNumStates(b, numStatesInBB);

        // ??? janders -- not sure if the code below is NEEDED
        // this was borrowed from the previous implementation
        Instruction *inst = b->getTerminator();
        if (isa<ReturnInst>(inst)) {
            InstructionNode *in = dag->getInstructionNode(inst);
            pw_map->setState(in, numStatesInBB);
        }
    }

    for (auto bbit : variables) {
        delete[] bbit.second;
    }
}

void SDCScheduler::scheduleASAP() { scheduleAXAP(/*ASAP=*/true); }

void SDCScheduler::scheduleALAP() {
    scheduleAXAP(/*ASAP=*/false);
}

void SDCScheduler::scheduleAXAP(bool ASAP) {

    if (SDCdebug || PWSDCdebug) {
        std::cout << "------------ Schedule AXAP --------------" << '\n';
    }

    // In this case, we simply minimize the sum of the starting cycles
    // for all instructions
    // ticmt = clock();
    int *variableIndices = new int[numInst];
    REAL *variableCoefficients = new REAL[numInst];

    int count = 0;
    for (std::map<InstructionNode *, unsigned>::iterator
             i = startVariableIndex.begin(),
             e = startVariableIndex.end();
         i != e; i++) {

        unsigned varIndex = i->second;
        variableIndices[count] = 1 + varIndex;
        variableCoefficients[count] = 1.0;
        count++;
    }
    assert(count == numInst);

    set_obj_fnex(lp, count, variableCoefficients, variableIndices);
    if (ASAP)
        set_minim(lp);
    else
      set_maxim(lp);

    // leandro -- pw version
    if (LEGUP_CONFIG->getParameterInt("PIECEWISE_SDC")) {
        // one vector for each basic block
        std::map<BasicBlock *, int *>
            pw_variableIndices; // = new int[numInstVec[b]];
        std::map<BasicBlock *, REAL *>
            pw_variableCoefficients; // = new REAL[numInstVec[b]];

        for (auto lpit : lpvec) {
            BasicBlock *b = lpit.first;
            assert(numInstVec[b]);
            // std::cout << "comparing: " << numInstVec[b] << " and: " <<
            // b->size() << '\n';
            pw_variableIndices[b] = new int[numInstVec[b]];
            pw_variableCoefficients[b] = new REAL[numInstVec[b]];

            // leandro -- pw version
            unsigned pw_count = 0;
            for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie;
                 ++i) {
                pw_variableIndices[b][pw_count] =
                    pw_startVariableIndex[dag->getInstructionNode(i)] + 1;
                pw_variableCoefficients[b][pw_count] = 1.0;
                pw_count++;
            }
            assert(pw_count == numInstVec[b]);
            set_obj_fnex(lpvec[b], pw_count, pw_variableCoefficients[b],
                         pw_variableIndices[b]);
            if (ASAP)
                set_minim(lpvec[b]);
            else
                set_maxim(lpvec[b]);
        }
    }

    if (SDCdebug || PWSDCdebug) {
        write_LP(lp, stdout);
    } else {
        set_verbose(lp, 1);
    }

    if (PWSDCdebug) {
        for (auto lpit : lpvec) {
          std::cout << getLabel(lpit.first) << " BB constraints: ";
          write_LP(lpit.second, stdout);
          std::cout << "\n\n";
        }
    } else {
        for (auto lpit : lpvec) {
            set_verbose(lpit.second, 1);
        }
    }

    // leandro - we stop measing the mapping time just before the solve
    // tocmt = clock();
    // mappingtime += (double)(tocmt - ticmt)/CLOCKS_PER_SEC;

  // leandro - measure time to lpsolve to find a solution
  ticst = clock();
  if (SDCdebug || PWSDCdebug) {
      std::cout << " --- trying to solve the function LP" << '\n';
  }
  int ret = solve(lp);
  // leandro -- just to make things fair
  assert(ret == 0);

  // leandro - pw version
  if (LEGUP_CONFIG->getParameterInt("PIECEWISE_SDC")) {
      int pw_ret;
      for (auto lpit : lpvec) {
          if (SDCdebug || PWSDCdebug) {
              std::cout << getLabel(lpit.first)
                        << "  ---  trying to solve the BB LP" << '\n';
          }
          pw_ret = solve(lpit.second);
          assert(pw_ret == 0);
      }
  }

  tocst = clock();
  solvetime += (double)(tocst - ticst) / CLOCKS_PER_SEC;

  if (SDCdebug) {
    printf("SDC solver status: %d\n", ret);
  }

  if (ret != 0) {
      std::cout << "LP solver returned: " << ret << "\n";
      report_fatal_error("LP solver could not find an optimal solution");
  }

  delete[] variableCoefficients;
  delete[] variableIndices;
}

void SDCScheduler::add_lp_constraints_for_sdc_multipump(Function *F) {
    addALAPConstraints(F);
    if (LEGUP_CONFIG->getParameterInt("PIECEWISE_SDC")) {
        pw_addALAPConstraints(F);
    }

    if (SDCdebug) {
        printf("SOLVING ALAP\n");
    }
    scheduleALAP();

    REAL* variables = new REAL[numVars];
    get_variables(lp, variables);
    for (Function::iterator b = F->begin(), be = F->end(); b != be; b++) {
        for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
            if (!alloc->useExplicitDSP(i))
                continue;
            alap_control_step[i] = (unsigned)
                variables[endVariableIndex[dag->getInstructionNode(i)]];
            // std::cout << "I: " << *i << "\n";
            // std::cout << "\tstateAssigned: " << alap_control_step[i] << "\n";
        }
    }
    delete[] variables;

    /*
    for (Function::iterator b = F->begin(), be = F->end(); b != be; b++) {
        for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
            if (!alloc->useExplicitDSP(i)) continue;
            if (asap_control_step[i] == alap_control_step[i]) continue;
            std::cout << "I: " << *i << "\n";
            std::cout << "\tasap: " << asap_control_step[i] << "\n";
            std::cout << "\talap: " << alap_control_step[i] << "\n";
        }
    }
    */

    // todo: refactor this (copied from above)
    delete_lp(lp);
    numVars = 0;
    numInst = 0;
    createLPVariables(F);
    addMulticycleConstraints(F);
    addDependencyConstraints(F);
    if (chaining && clockPeriodConstraint > 0) {
        addTimingConstraints(F);
    }
    //      addResourceConstraint(F); ??? janders: Andrew check if you want to call new res constraints handling code in ur multi-pump stuff

    addMultipumpConstraints(F);
}

void SDCScheduler::do_post_scheduling_steps(Function *F) {
    if (SDCdebug || PWSDCdebug) {
        std::cout << "\n\n ------------ post scheduling steps -----------"
                  << '\n';
    }

    REAL *variables = new REAL[numVars];
    get_variables(lp, variables);

    // TODO leandro - not supporting multipumping yet
    for (Function::iterator b = F->begin(), be = F->end(); b != be; b++) {
        for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {

            unsigned idx = endVariableIndex[dag->getInstructionNode(i)];
            int stateAssigned = variables[idx];

            if (SDCdebug || PWSDCdebug) {
                printf("ASAP BB: %s INSTR: %s OPCODE: %s (IDX: %d) CLOCK "
                       "END CYCLE ASSIGNED: %u\n",
                       getLabel(b).c_str(), getLabel(i).c_str(),
                       i->getOpcodeName(), idx + 1, stateAssigned);
          }

          if (LEGUP_CONFIG->getParameterInt("SDC_MULTIPUMP")) {
              asap_control_step[i] = stateAssigned;
              // leandro - yes I was screaming
              // std::cout << "aaaaaaaaaaaaaahhhhhhhhhhhhh" << '\n';
          }
        }
    }
    // the number of constraints in the LP formulation before we add constraints
    // for ALAP scheduling
    int Nrows = 0;

    // leandro - too much problems to remove this from the scope now
    std::map<BasicBlock *, int> pw_Nrows;

    if (LEGUP_CONFIG->getParameterInt("SDC_RES_CONSTRAINTS")) {

        // ALAP scheduling, if desired, must be done
        // after ASAP scheduling.  The ASAP scheduling
        // is used to derive a MAXIMUM cycle bound
        // on the schedule

    // we do ALAP scheduling in two cases: when the user actually WANTS it, and also when
        // we are doing resource-constrained scheduling

        Nrows = get_Nrows(lp);
        if (SDCdebug || PWSDCdebug) {
            std::cout << "\n ---ALAP constraints" << '\n';
        }

        addALAPConstraints(F);

        // leandro - pw version
        if (LEGUP_CONFIG->getParameterInt("PIECEWISE_SDC")) {
            for (auto lpit : lpvec) {
                pw_Nrows[lpit.first] = get_Nrows(lpit.second);
            }

            if (PWSDCdebug) {
                std::cout << "\n----PW ALAP constraints" << '\n';
            }
            pw_addALAPConstraints(F);
        }

        if (SDCdebug || PWSDCdebug) {
            printf("SOLVING ALAP to determine ordering for resource "
                   "constraints\n");
        }

        scheduleALAP();
    }

    if (LEGUP_CONFIG->getParameterInt("SDC_RES_CONSTRAINTS")) {

        // delete the ALAP constraints (the most recently added rows of the LP
        // formulation)
        // this returns the LP formulation back to ASAP

        // the reason we are doing is that the resource constraints will (most
        // likely)
        // dilate the schedule making it longer since the original ASAP
        // scheduling
        // was done without considering such constraints.

        // so we cannot expect the resource-constrained schedule to be as short
        // as the
        // unconstrained schedule.  we therefore must delete the ALAP
        // constraints.
        for (int i = get_Nrows(lp); i > Nrows; i--) {
            del_constraint(lp, i);
        }

        // leandro - pw version
        if (LEGUP_CONFIG->getParameterInt("PIECEWISE_SDC")) {
            for (auto lpit : lpvec) {
                for (int i = get_Nrows(lpit.second); i > pw_Nrows[lpit.first];
                     i--) {
                    del_constraint(lpit.second, i);
                }
            }
        }

        // leandro
        if (SDCdebug || PWSDCdebug) {
            std::cout << "\n\n--------- Resource constraints ---------\n"
                      << '\n';
        }

        // get all unique FU types in the function
        std::set<std::string> seenFUType;
        for (Function::iterator b = F->begin(), be = F->end(); b != be; b++) {
            for (BasicBlock::iterator I = b->begin(), ie = b->end(); I != ie;
                 I++) {
                std::string FuName =
                    LEGUP_CONFIG->getOpNameFromInst(I, this->alloc);

                if (seenFUType.find(FuName) != seenFUType.end())
                    continue;

                seenFUType.insert(FuName);

                int constraint;

                if (!LEGUP_CONFIG->getNumberOfFUsAllocated(FuName, &constraint))
                    continue;

                if (SDCdebug || PWSDCdebug) {
                    std::cout
                        << "\n\nAdding constraint for specific FU: " << FuName
                        << " of: " << constraint << "\n";
                    std::cout << " -- Function version\n";
                }
                addResourceConstraint(F, FuName, constraint);
                if (LEGUP_CONFIG->getParameterInt("PIECEWISE_SDC")) {
                    if (PWSDCdebug) {
                        std::cout << " -- BB " << getLabel(b) << " version\n";
                    }
                    pw_addResourceConstraint(F, FuName, constraint);
                }
            }
        }
        // invoke ASAP scheduling (with added resource constraints)
        scheduleASAP();
    }

    // must do ALAP after resource constrained ASAP scheduling because the
    // maximum cycle bound of the schedule might have changed

    // this is only if the user WANTS ALAP scheduling (via TCL file settings)
    if (LEGUP_CONFIG->getParameterInt("SDC_ALAP")) {
        if (SDCdebug || PWSDCdebug) {
            std::cout << "\n\n--------------- ALAP final schedule (alap "
                         "specified)------------a\n" << '\n';
        }

        Nrows = get_Nrows(lp);
        addALAPConstraints(F);

        // leandro - pw version
        if (LEGUP_CONFIG->getParameterInt("PIECEWISE_SDC")) {
            for (auto lpit : lpvec) {
                pw_Nrows[lpit.first] = get_Nrows(lpit.second);
            }
            pw_addALAPConstraints(F);
        }

        if (SDCdebug) {
            printf("SOLVING ALAP\n");
        }
        scheduleALAP();
    }

    // for multipumping we want to group multipliers into the same cycle
    // if their min (ASAP) and max (ALAP) cycle constraints overlap
    // TODO - leandro - not yet suported
    if (LEGUP_CONFIG->getParameterInt("SDC_MULTIPUMP")) {
        if (SDCdebug || PWSDCdebug) {
            std::cout
                << "\n\n--------------- MultiPump schedule ------------a\n"
                << '\n';
        }
        add_lp_constraints_for_sdc_multipump(F);

        // already has ALAP constraints so we have to solve as an ALAP schedule
        // or can i...?
        // scheduleALAP();
        scheduleASAP();
    }
}

SchedulerMapping *SDCScheduler::createMapping(Function *F, SchedulerDAG *dag) {
    map = new SchedulerMapping();
    if (LEGUP_CONFIG->getParameterInt("PIECEWISE_SDC")) {
        pw_map = new SchedulerMapping();
    }

    this->dag = dag;

    // leandro - start measuring time here
  ticmt = clock();

  // create variables in the LP formulation for
  // each of the instructions to be scheduled in the LLVM IR
  createLPVariables(F);

  // add multi-cycle instruction constraints
  // these are the the math constraints for the instructions
  // that take multiple cycles to complete, for example, the multiply instruction
  // in the Cyclone II FPGA
  addMulticycleConstraints(F);

  chaining = true;

  if (LEGUP_CONFIG->getParameterInt("SDC_NO_CHAINING")) {
    chaining = false; // no chaining means that the design will be pipelined as much as possible
  }

  // add the constraints to the LP so that instructions
  // are scheduled in cycles >= the cycles where their
  // dependencies are computed
  addDependencyConstraints(F);

  // scheduling of llvm.dbg.value instructions (used for debugging)
  addDbgValueInstConstraints(F);

  clockPeriodConstraint = 15; // 66 MHz
  if (LEGUP_CONFIG->getParameterInt("CLOCK_PERIOD")) {
      clockPeriodConstraint =
          (float)LEGUP_CONFIG->getParameterInt("CLOCK_PERIOD");
  }

  if (chaining && clockPeriodConstraint > 0) {
    addTimingConstraints(F);
  }

  //leandro stop the mapping here and continue inside the scheduleAXAP()
  //tocmt = clock();
  //mappingtime += (double)(tocmt - ticmt)/CLOCKS_PER_SEC;

  // do the scheduling
  scheduleAXAP(true);

  // leandro - start measuring time here for the modifications in
  // ticmt = clock();

  // Make updates to the scheduling just produced
  do_post_scheduling_steps(F);

  // deposit schedule into the schedule map data struction
  if (SDCdebug || PWSDCdebug) {
      std::cout
          << "\n\n------------------deposit schedule for Function ------\n\n";
  }
  depositSchedule(F);

  if (LEGUP_CONFIG->getParameterInt("PIECEWISE_SDC")) {
      if (PWSDCdebug) {
          std::cout << "\n\n------------------deposit schedule for PW Function "
                       "------\n\n";
      }
      pw_depositSchedule(F);
  }

  if (PWSDCdebug) {
      if (map->compare(pw_map)) {
          std::cout << "\n\n!!!!!!!!!!!!!!!!!!!\n    SUCCESS!! "
                       "\n!!!!!!!!!!!!!!!!!!!\n" << '\n';
      }
  }

  // The current schedule has been stored to "map". Now, check
  // whether data paths should be multi-cycled in this compilation.
  // If so, remove the registers from data paths here.
  // Then, if software profiling was performed for this compilation,
  // check whether data path latencies should be extended for infrequent BB.
  // If they should be, a second scheduling phase is also performed here.

  // Leandro TODO - modify for pw
  multicycle_and_modify_schedule(F);

  // leandro -- add the time modifications in the scheduling
  tocmt = clock();
  mappingtime += (double)(tocmt - ticmt) / CLOCKS_PER_SEC;

  { // keep these together
      delete_lp(lp);

      // leandro - creaning the mess
      for (auto lpit : lpvec) {
          delete_lp(lpit.second);
      }
  }

  if (LEGUP_CONFIG->getParameterInt("PIECEWISE_SDC")) {
      return pw_map;
  }
  return map;
}

} // legup namespace
