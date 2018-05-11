//===------ SDCModuloScheduler.cpp ---------------------------------===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// See header
//
//===----------------------------------------------------------------------===//

/*
#define DEBUG_TYPE "polly-codegen"

#include "polly/LinkAllPasses.h"
#include "polly/Support/GICHelper.h"
#include "polly/Support/ScopHelper.h"
#include "polly/Cloog.h"
#include "polly/Dependences.h"
#include "polly/ScopInfo.h"
#include "polly/TempScopInfo.h"
*/
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Support/FileSystem.h"
#include "SchedulerDAG.h"
#include "Scheduler.h"
#include "utils.h"
#include "Ram.h"
#include "LegupConfig.h"
#include "SDCModuloScheduler.h"

//#define CLOOG_INT_GMP 1
//#include "cloog/cloog.h"
//#include "cloog/isl/cloog.h"

#include <sstream>
#include <iomanip>
#include <vector>
#include <utility>
#include <lp_lib.h>
#include <math.h>
#include <algorithm>

#include "SDCSolver.h"

// using namespace polly;
using namespace llvm;
using namespace legup;

long int fact(int n){
  long int res = 1;
  for(int i = 1; i<n; i++){
    res *= i;
  }
  return res;
}

bool SDCModuloScheduler::runOnLoop(Loop *L, LPPassManager &LPM) {
    moduloScheduler.loop = L;

    // leandro time measing for individual steps
    // variables keep their values between loops
    totaltime = 0;
    solvetime = 0;
    nsdcs = 0;
    double oppermannTime=0;
    clock_t tic_op, toc_op;

    FILE *pFile;
    std::string rptname("DetailedModuleSDCSchedulingTime");
    std::ifstream f(rptname);
    if (!f.good()) {
        pFile = fopen(rptname.c_str(), "w");
        fprintf(pFile, "label\tn_IRlines\tn_sdcs\tTotal\tSolving\n");
        fclose(pFile);
    }
    // std::cout << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" << '\n';
    // L->dump();
    // std::cout << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" << '\n';
    // std::cout << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" << '\n';
    int nIRlines = 0;
    for (Loop::block_iterator bb = L->block_begin(), e = L->block_end();
         bb != e; ++bb) {
        nIRlines += (*bb)->size();
    }
    // leandro - start measuring time

    BB = moduloScheduler.find_pipelined_bb();
    moduloScheduler.BB = BB;
    if (!BB) {
        // didn't find the loop or pipelining is turned off
        return false;
    }

    clock_t tictt = clock();
    initLoop();

    int MII = getInitialMII();

    // ModuloSchedulo
    moduloScheduler.II = MII;

    int userII = moduloScheduler.PipelineTclInfo.II;
    if (moduloScheduler.PipelineTclInfo.user_II) {
        if (SDCdebug)
            File() << "user II = " << userII << "\n";
        if (userII < moduloScheduler.II) {
            errs() << "ERROR: user specified II couldn't be achieved!\n";
            return false;
        }
        moduloScheduler.II = userII;
    }

    if (SDCdebug)
        File() << "Initial II = " << moduloScheduler.II << "\n";
    initializeSDC(moduloScheduler.II);

    // int budgetRatio = 100;
    int budgetRatio =
        LEGUP_CONFIG->getParameterInt("SDC_BACKTRACKING_BUDGET_RATIO");
    assert(budgetRatio > 0);
    int numOps = BB->size();

    //while (!iterativeSchedule(budgetRatio * numOps)) {
    oppermannTime = 0;
    while(oppermannTime < 10){
      printf("min II = %d\n", moduloScheduler.II);
      tic_op = clock();
      //if(!iterativeSchedule( fact(numOps)*numOps) ){
      if(iterativeSchedule( numOps*numOps*numOps) ){
        toc_op = clock();
        oppermannTime += (double)(toc_op - tic_op)/CLOCKS_PER_SEC;
        printf("\nmanage to schedule\n");
        printf("\nfinal oppermannTime = %f\tII=%d\n", oppermannTime,moduloScheduler.II);
        break;
      }

      if (moduloScheduler.PipelineTclInfo.user_II) {
          errs() << "ERROR: user specified II couldn't be achieved!\n";
          return false;
      }
      if (SDCdebug)
          File() << "Incrementing II\n";
      moduloScheduler.II++;
      if (SDCdebug)
          File() << "II = " << moduloScheduler.II << "\n";
      moduloScheduler.sanityCheckII(moduloScheduler.II);
      // re-create the SDC constraints with new II
      initializeSDC(moduloScheduler.II);
      toc_op = clock();
      oppermannTime += (double)(toc_op - tic_op)/CLOCKS_PER_SEC;
      printf("\nincremental oppermannTime = %f\tII=%d\n", oppermannTime,moduloScheduler.II );

    }
    //}

    if (SDCdebug)
        File() << "Scheduled.\n";
    if (SDCdebug)
        File() << "MII = " << MII << "\n";
    if (SDCdebug)
        File() << "II = " << moduloScheduler.II << "\n";
    if (LEGUP_CONFIG->getParameterInt("MODULO_DEBUG")) {
        errs() << "II = " << moduloScheduler.II << "\n";
    }

    Value *Elts[] = {
        MDString::get(M->getContext(), "II"),
        MDString::get(M->getContext(), utostr(moduloScheduler.II)),
        // MDString::get(M->getContext(), "BB"),
        // BB,
    };
    MDNode *Node = MDNode::get(M->getContext(), Elts);

    M->getOrInsertNamedMetadata("legup.pipeline")->addOperand(Node);

    assert(moduloScheduler.II > 0 && "II must be > 0");
    if (SDCdebug)
        File() << "Final Modulo Reservation Table:\n";
    printModuloReservationTable();
    if (SDCdebug)
        File() << "\n";

    moduloScheduler.gather_pipeline_stats();
    moduloScheduler.print_pipeline_table();
    moduloScheduler.set_llvm_metadata();

    moduloScheduler.totalLoopsPipelined++;
    moduloScheduler.loopsPipelined.insert(moduloScheduler.loopLabel);

    clock_t toctt = clock();
    totaltime += (double)(toctt - tictt) / CLOCKS_PER_SEC;

    pFile = fopen("DetailedModuleSDCSchedulingTime", "a");
    // leandro
    fprintf(pFile, "%s\t%d\t%ld\t%f\t%f\n", moduloScheduler.loopLabel.c_str(),
            nIRlines, nsdcs, totaltime, solvetime);
    fclose(pFile);
    return true;
}

SDCModuloScheduler::SDCModuloScheduler() {
    SDCdebug = LEGUP_CONFIG->getParameterInt("SDC_DEBUG");
    moduloScheduler.ranAlready = false;
    moduloScheduler.totalLoopsPipelined = 0;
    moduloScheduler.forceNoChain = false;
    sdcSolver.lp = NULL;
    sdcSolver.file = moduloScheduler.file;
    sdcSolver.SDCdebug = SDCdebug;
}

void SDCModuloScheduler::computeHeight(Instruction *I) {
    if (isa<PHINode>(I)) {
        // avoid a cycle when calculating the node heights
        // TODO: is this the correct height to give a phi?
        height[I] = 0;
        return;
    }

    if (height.find(I) != height.end()) {
        // already calculated height
        return;
    }
    // errs() << "computeHeight: I: " << *I << "\n";

    // no children ?
    if (noChildren(I)) {
        // op->mem_use_begin() == op->mem_use_end()) {
        if (SDCdebug)
            File() << "No children (height=0): " << *I << "\n";
        height[I] = 0;
    } else {
        // errs() << "children:\n";
        for (Value::user_iterator user = I->user_begin(), e = I->user_end();
             user != e; ++user) {
            if (Instruction *child = dyn_cast<Instruction>(*user)) {
                // errs() << "\t" << *child << "\n";
                updateHeight(I, child);
            }
        }

        InstructionNode *op = dag->getInstructionNode(I);
        for (InstructionNode::iterator i = op->mem_use_begin(),
                                       e = op->mem_use_end();
             i != e; ++i) {
            InstructionNode *child = *i;
            updateHeight(I, child->getInst());
        }
    }
}

void SDCModuloScheduler::updateHeight(Instruction *I, Instruction *child) {
    // errs() << "updateHeight: I: " << *I << " child: " << *child << "\n";

    computeHeight(child);
    assert(height.find(child) != height.end());

    // height[I] = std::max(height[I], height[C] + delay(I));
    height[I] = std::max(height[I], height[child] + moduloScheduler.delay(I) -
                                        moduloScheduler.II *
                                            moduloScheduler.distance(I, child));

    // errs() << "delay=" << moduloScheduler.delay(I) << "\n";
    // errs() << "height=" << height[I] << ": " << *I << "\n";
}

void SDCModuloScheduler::computeHeights() {
    printLineBreak();
    if (SDCdebug)
        File() << "Computing Heights\n";
    // errs() << "Loop:\n";
    for (BasicBlock::iterator instr = BB->begin(), ie = BB->end(); instr != ie;
         ++instr) {
        computeHeight(instr);
    }
    printLineBreak();
}

int SDCModuloScheduler::numIssueSlots(std::string FuName) {

    int constraint;
    if (LEGUP_CONFIG->getNumberOfFUsAllocated(FuName, &constraint)) {
        return constraint;
    } else {
        // no user-specified constraint
        return 0;
    }
}

bool SDCModuloScheduler::resourceConflict(Instruction *I, int timeSlot) {
    int moduloTimeSlot = timeSlot % moduloScheduler.II;

    if (moduloScheduler.isResourceConstrained(I)) {
        std::string FuName =
            LEGUP_CONFIG->getOpNameFromInst(I, moduloScheduler.alloc);
        for (int i = 0; i < numIssueSlots(FuName); i++) {
            if (moduloScheduler.getReservationTable(FuName, i,
                                                    moduloTimeSlot) == NULL) {
                // found a free slot
                return false;
            }
        }
        return true;
    }

    return false;
}

// select time where operation should be scheduled
int SDCModuloScheduler::findTimeSlot(Instruction *I, int minTime, int maxTime) {
    int curTime = minTime;
    int schedSlot = 0;
    bool found = false;
    while (!found && curTime <= maxTime) {
        if (resourceConflict(I, curTime)) {
            if (SDCdebug)
                File() << "Resource conflict at time " << curTime
                       << ". Incrementing time slot\n";
            // try the next time slot
            curTime++;
        } else {
            // no resource conflicts. Select this time slot.
            // Dependence constraints due to predecessors were honoured in the
            // computation of minTime
            schedSlot = curTime;
            found = true;
        }
    }

    // if a legal slot was not found, then pick the first option from the
    // following:
    // 1) minTime - if this is the first time the operation is being scheduled
    // or if minTime is greater than the time the operation was last scheduled
    // 2) previously scheduled time + 1
    if (!found) {
        if (SDCdebug)
            File() << "No legal slot found\n";
        if (neverScheduled[I] || minTime > prevSchedTime[I]) {
            if (SDCdebug)
                File() << "Forcing to minTime\n";
            schedSlot = minTime;
        } else {
            if (SDCdebug)
                File() << "Forcing to prev sched time + 1\n";
            schedSlot = prevSchedTime[I] + 1;
        }
    }

    return schedSlot;
}

bool SDCModuloScheduler::MRTSlotEmpty(int slot, std::string FuName) {
    for (int j = 0; j < numIssueSlots(FuName); j++) {
        if (moduloScheduler.getReservationTable(FuName, j, slot)) {
            return false;
        }
    }
    return true;
}

void SDCModuloScheduler::printModuloReservationTable() {

    for (std::set<std::string>::iterator
             i = moduloScheduler.constrainedFuNames.begin(),
             e = moduloScheduler.constrainedFuNames.end();
         i != e; ++i) {
        std::string FuName = *i;
        if (SDCdebug)
            File() << "FuName: " << FuName << "\n";

        for (int i = 0; i < moduloScheduler.II; i++) {
            if (SDCdebug)
                File() << "time slot: " << i;
            if (MRTSlotEmpty(i, FuName)) {
                if (SDCdebug)
                    File() << " empty\n";
                continue;
            }
            if (SDCdebug)
                File() << "\n";
            for (int j = 0; j < numIssueSlots(FuName); j++) {
                // TODO: LLVM 3.5 update: cannot print value if it is NULL so
                // check it here
                if (moduloScheduler.getReservationTable(FuName, j, i) == NULL) {
                    if (SDCdebug)
                        File() << "   issue slot: " << j << " instr: "
                               << "printing a <null> value"
                               << "\n";
                } else {
                    if (SDCdebug)
                        File() << "   issue slot: " << j << " instr: "
                               << *moduloScheduler.getReservationTable(
                                      FuName, j, i) << "\n";
                }
            }
        }
    }
}

void SDCModuloScheduler::unscheduleSDC(Instruction *I) {
    unschedule(I);
    sdcSolver.deleteAllInstrConstraints(I);
}

void SDCModuloScheduler::unschedule(Instruction *I) {
    assert(I);
    std::string FuName =
        LEGUP_CONFIG->getOpNameFromInst(I, moduloScheduler.alloc);

    if (SDCdebug)
        File() << "Unscheduling: " << *I << "\n";

    unscheduledInsts.insert(I);
    unscheduledInstsConstrained.insert(I);
    unscheduledInstsConstrainedQueue.push(I);
    unscheduledInstPriorityQueue.push_back(I);
    moduloScheduler.schedTime.erase(I);

    // remove from reservation table
    bool found = false;

    for (int i = 0; i < numIssueSlots(FuName); i++) {
        for (int j = 0; j < moduloScheduler.II; j++) {
            Instruction *prev =
                moduloScheduler.getReservationTable(FuName, i, j);
            if (I == prev) {
                // shouldn't exist in the reservation table twice
                assert(!found);
                found = true;
                moduloScheduler.setReservationTable(FuName, i, j, NULL);
            }
        }
    }
}

// schedule operation at time timeSlot. Displace previous scheduled
// operations that conflict with it either due to resource conflicts or
// dependence constraints
void SDCModuloScheduler::schedule(Instruction *I, int timeSlot) {

    moduloScheduler.schedTime[I] = timeSlot;
    std::string FuName =
        LEGUP_CONFIG->getOpNameFromInst(I, moduloScheduler.alloc);

    if (moduloScheduler.isResourceConstrained(I)) {
        int moduloTimeSlot = timeSlot % moduloScheduler.II;

        bool resourceConflict = true;
        for (int i = 0; i < numIssueSlots(FuName); i++) {
            // the instruction shouldn't already be in the table
            assert(
                I !=
                moduloScheduler.getReservationTable(FuName, i, moduloTimeSlot));
            // find an empty slot
            if (moduloScheduler.getReservationTable(FuName, i,
                                                    moduloTimeSlot) == NULL) {
                moduloScheduler.setReservationTable(FuName, i, moduloTimeSlot,
                                                    I);
                resourceConflict = false;
                break;
            }
        }

        if (resourceConflict) {
            // unschedule *all* potential resource conflicts
            for (int i = 0; i < numIssueSlots(FuName); i++) {
                Instruction *prev = moduloScheduler.getReservationTable(
                    FuName, i, moduloTimeSlot);
                assert(I != prev);
                if (prev) {
                    if (SDCdebug)
                        File() << "Resource conflict\n";
                    unschedule(prev);
                }
            }
            // use the first issue slot (arbitrary)
            moduloScheduler.setReservationTable(FuName, 0, moduloTimeSlot, I);
        }
    }

    // loop over successors and displace previous scheduled operations that
    // have a dependency conflict
    for (Value::use_iterator use = I->use_begin(), e = I->use_end(); use != e;
         ++use) {
        Instruction *succ = dyn_cast<Instruction>(*use);
        if (!succ)
            continue;

        // successor has been scheduled
        if (moduloScheduler.schedTime.find(succ) !=
            moduloScheduler.schedTime.end()) {
            int min = moduloScheduler.schedTime[I] + moduloScheduler.delay(I) -
                      moduloScheduler.II * distance(I, succ);
            if (moduloScheduler.schedTime[succ] < min) {
                assert(succ != I);
                if (SDCdebug)
                    File() << "Conflict with successor\n";
                unschedule(succ);
            }
        }
    }

    InstructionNode *op = dag->getInstructionNode(I);
    for (InstructionNode::iterator use = op->mem_use_begin(),
                                   e = op->mem_use_end();
         use != e; ++use) {
        Instruction *succ = (*use)->getInst();
        assert(succ);

        // successor has been scheduled
        if (moduloScheduler.schedTime.find(succ) !=
            moduloScheduler.schedTime.end()) {
            int min = moduloScheduler.schedTime[I] + moduloScheduler.delay(I) -
                      moduloScheduler.II * distance(I, succ);
            if (moduloScheduler.schedTime[succ] < min) {
                assert(succ != I);
                if (SDCdebug)
                    File() << "Conflict with successor\n";
                unschedule(succ);
            }
        }
    }

    neverScheduled[I] = false;
    unscheduledInsts.erase(I);
    prevSchedTime[I] = timeSlot;
}

int SDCModuloScheduler::predecessorStart(Instruction *I, Instruction *pred) {
    if (moduloScheduler.schedTime.find(pred) ==
        moduloScheduler.schedTime.end()) {
        // unscheduled
        return 0;
    } else {
        // immediate early start (estart)
        int start = moduloScheduler.schedTime[pred] +
                    moduloScheduler.delay(pred) -
                    moduloScheduler.II * moduloScheduler.distance(pred, I);
        return std::max(0, start);
    }
}

// calculate the earliest start time based on the immediate predecessors
// that have been scheduled already
int SDCModuloScheduler::calcEarlyStart(Instruction *I) {

    // if the branch depends on this state it should try be scheduled
    // in stage 0. this is to allow branching without predication
    /*
     * TODO: add this back in
    Instruction *branch = I->getParent()->getTerminator();
    for (User::op_iterator i = branch->op_begin(), e = branch->op_end(); i !=
            e; ++i) {
        Value *op = *i;
        if (op == I) {
            return 0;
        }
    }

    InstructionNode *branchNode = dag->getInstructionNode(branch);
    for (InstructionNode::iterator dep = branchNode->mem_dep_begin(),
            e = branchNode->mem_dep_end(); dep != e; ++dep) {
        // dependency from dep -> branchNode
        if ((*dep)->getInst() == I) {
            return 0;
        }
    }
      */

    int earlyStart = 0;
    for (User::op_iterator i = I->op_begin(), e = I->op_end(); i != e; ++i) {
        Instruction *pred = dyn_cast<Instruction>(i);
        if (!pred)
            continue;
        earlyStart = std::max(earlyStart, predecessorStart(I, pred));
    }

    InstructionNode *iNode = dag->getInstructionNode(I);
    for (InstructionNode::iterator dep = iNode->mem_dep_begin(),
                                   e = iNode->mem_dep_end();
         dep != e; ++dep) {
        // dependency from dep -> iNode
        earlyStart =
            std::max(earlyStart, predecessorStart(I, (*dep)->getInst()));
    }

    return earlyStart;
}

void SDCModuloScheduler::init() {
    neverScheduled.clear();
    height.clear();
    moduloScheduler.schedTime.clear();
    prevSchedTime.clear();
    unscheduledInsts.clear();
    unscheduledInstsConstrained.clear();

    while (!unscheduledInstsConstrainedQueue.empty()) {
        unscheduledInstsConstrainedQueue.pop();
    }
    unscheduledInstPriorityQueue.clear();

    moduloScheduler.initReservationTable();
}

// greater height = higher priority
Instruction *SDCModuloScheduler::getHighestPriorityInst() {
    // this should be reimplemented with a priority queue
    int maxHeight = -1;
    Instruction *highest = NULL;
    for (std::map<Instruction *, int>::iterator i = height.begin(),
                                                e = height.end();
         i != e; ++i) {
        Instruction *I = i->first;
        int height = i->second;
        // already scheduled
        if (unscheduledInsts.find(I) == unscheduledInsts.end())
            continue;
        if (height > maxHeight) {
            maxHeight = height;
            highest = I;
        }
    }
    // errs() << "maxHeight: " << maxHeight << "\n";
    assert(highest);
    return highest;
}

bool SDCModuloScheduler::iterativeClassic(int budget) {
    budget--;

    while (!unscheduledInsts.empty() && budget > 0) {
        Instruction *I = getHighestPriorityInst();
        if (SDCdebug)
            File() << "Scheduling: " << *I << "\n";

        int earlyStart = calcEarlyStart(I);

        int minTime = earlyStart;
        int maxTime = minTime + moduloScheduler.II - 1;

        int timeSlot = findTimeSlot(I, minTime, maxTime);

        if (SDCdebug)
            File() << "minTime: " << minTime << "\n";
        if (SDCdebug)
            File() << "maxTime: " << maxTime << "\n";
        if (SDCdebug)
            File() << "timeSlot: " << timeSlot << "\n";
        if (timeSlot != minTime) {
            if (SDCdebug)
                File()
                    << "Moved time slot away from minTime due to conflicts\n";
        }

        schedule(I, timeSlot);

        budget--;
    }
    return unscheduledInsts.empty();
}

// is there a conflict when scheduling operation at time timeSlot?
void SDCModuloScheduler::scheduleSDCInstruction(Instruction *I, int timeSlot) {
    // we only ever need to fix the position in the SDC formulation
    // for resource constrained instructions - these can't be
    // handled by the LP solver due to the modulo reservation table
    assert(moduloScheduler.isResourceConstrained(I));

    if (SDCdebug)
        File() << "Successfully scheduled (at time slot: " << timeSlot
               << "): " << *I << "\n";
    if (SDCdebug)
        File() << "TimeSlot: " << timeSlot << " Scheduling: " << *I << "\n";

    // make sure instruction stays at the same timeSlot when
    // solving the LP in the future
    constrainSDC(I, EQ, timeSlot);

    moduloScheduler.schedTime[I] = timeSlot;
    neverScheduled[I] = false;
    unscheduledInsts.erase(I);
    unscheduledInstsConstrained.erase(I);
    prevSchedTime[I] = timeSlot;

    if (moduloScheduler.isResourceConstrained(I)) {
        int port = -1;
        bool success = findEmptySlot(I, timeSlot, &port);
        assert(success);
        assert(port != -1);

        int moduloTimeSlot = timeSlot % moduloScheduler.II;
        /*
        errs() << "scheduling: " << *I << "\n";
        errs() << "moduloTimeSlot: " << moduloTimeSlot << "\n";
        errs() << "port: " << port << "\n";
        errs() << "size: " << reservationTable.size() << "\n";
        errs() << "size: " << reservationTable.at(0).size() << "\n";
        */
        std::string FuName =
            LEGUP_CONFIG->getOpNameFromInst(I, moduloScheduler.alloc);
        moduloScheduler.setReservationTable(FuName, port, moduloTimeSlot, I);
    }
}

bool SDCModuloScheduler::findEmptySlot(Instruction *I, int timeSlot,
                                       int *port) {
    assert(port);
    *port = 0;
    assert(moduloScheduler.isResourceConstrained(I));

    assert(timeSlot < 1000 && "Sanity Check");

    int moduloTimeSlot = timeSlot % moduloScheduler.II;

    std::string FuName =
        LEGUP_CONFIG->getOpNameFromInst(I, moduloScheduler.alloc);
    for (int i = 0; i < numIssueSlots(FuName); i++) {
        // the instruction shouldn't already be in the table
        assert(I !=
               moduloScheduler.getReservationTable(FuName, i, moduloTimeSlot));
        // find an empty slot
        if (moduloScheduler.getReservationTable(FuName, i, moduloTimeSlot) ==
            NULL) {
            *port = i;
            return true;
        }
    }
    return false;
}

// is there a conflict when scheduling operation at time timeSlot?
// includes both resource conflicts and dependency conflicts
bool SDCModuloScheduler::schedulingConflictSDC(Instruction *I, int timeSlot) {
    printLineBreak();
    if (SDCdebug)
        File() << "Is there a conflict (resource or dependency) "
               << "when scheduling at time slot: " << timeSlot << "?\n";

    if (moduloScheduler.isResourceConstrained(I)) {
        int port;
        if (!findEmptySlot(I, timeSlot, &port)) {
            if (SDCdebug)
                File() << "Resource conflict: No available issue slot at time "
                          "slot\n";
            return true;
        }
        if (SDCdebug)
            File() << "No resource conflict: found available issue slot\n";
    } else {
        if (SDCdebug)
            File() << "No resource conflict: not resource constrained\n";
    }

    bool depConflict = schedulingConflictSDCIgnoreResources(I, timeSlot);
    if (depConflict) {
        if (SDCdebug)
            File() << "Dependency conflict";
    } else {
        if (SDCdebug)
            File() << "No dependency conflict";
    }

    if (SDCdebug)
        File() << " when scheduling at time slot: " << timeSlot << ".\n";
    printLineBreak();
    return depConflict;
}

bool SDCModuloScheduler::checkFeasible() {
    if (LEGUP_CONFIG->getParameterInt("INCREMENTAL_SDC")) {
        bool isFeasible = sdcSolver.unprocessed.empty();
        // bool success = scheduleSDC();
        // if ( success != isFeasible) {
        //    if(SDCdebug) File() << "UNEXPECTED mismatch\n";
        //}
        // assert(success == isFeasible);
        // return success;

        if (isFeasible) {
            // update the schedule times
            saveSchedule(/*lpSolve=*/false);
        }

        return isFeasible;
    } else {
        return scheduleSDC();
    }
}

SDCSolver::Constraints *SDCModuloScheduler::constrainSDC(Instruction *I,
                                                         int constr_type,
                                                         REAL constraint) {

    if (SDCdebug)
        File() << "Constraining " << lpConstraintStr(constr_type) << " "
               << ftostr(constraint) << ": " << *I << "\n";

    int col[1];
    REAL val[1];

    InstructionNode *in = dag->getInstructionNode(I);

    assert(startVariableIndex.find(in) != startVariableIndex.end());

    col[0] = 1 + startVariableIndex[in];
    val[0] = 1.0;

    SDCSolver::Constraints *C = new SDCSolver::Constraints;
    sdcSolver.addConstraintIncremental(sdcSolver.lp, 1, val, col, constr_type,
                                       constraint, C, I);

    return C;
}

bool SDCModuloScheduler::schedulingConflictSDCIgnoreResources(Instruction *I,
                                                              int timeSlot) {

    if (SDCdebug)
        File() << "Can we schedule instruction at time: " << timeSlot
               << " ignoring resource constraints?\n";
    SDCSolver::Constraints *C = constrainSDC(I, EQ, timeSlot);

    bool success = checkFeasible();

    if (success) {
        if (SDCdebug)
            File() << "Yes. Feasible";
    } else {
        if (SDCdebug)
            File() << "No. Not feasible";
    }
    if (SDCdebug)
        File() << " to schedule instruction at time: " << timeSlot
               << " ignoring resource constraints.\n";

    // remove the equality constraint added above
    sdcSolver.deleteConstraints(C);

    return !success;
}

bool SDCModuloScheduler::schedulingConflict(Instruction *I, int timeSlot) {

    if (moduloScheduler.isResourceConstrained(I)) {
        int port;
        if (!findEmptySlot(I, timeSlot, &port)) {
            return true;
        }
    }

    // NO - need to fix this to use the actual SDC solver.
    // can't use the delay() functions here due to potential chaining
    assert(0 && "Deprecated use schedulingConflictSDC");

    // loop over successors to find previous scheduled operations that
    // have a dependency conflict
    for (Value::use_iterator use = I->use_begin(), e = I->use_end(); use != e;
         ++use) {
        Instruction *succ = dyn_cast<Instruction>(*use);
        if (!succ)
            continue;

        // successor has been scheduled
        if (moduloScheduler.schedTime.find(succ) !=
            moduloScheduler.schedTime.end()) {
            int min = moduloScheduler.schedTime[I] + moduloScheduler.delay(I) -
                      moduloScheduler.II * moduloScheduler.distance(I, succ);
            if (moduloScheduler.schedTime[succ] < min) {
                assert(succ != I);
                return true;
            }
        }
    }

    InstructionNode *op = dag->getInstructionNode(I);
    for (InstructionNode::iterator use = op->mem_use_begin(),
                                   e = op->mem_use_end();
         use != e; ++use) {
        Instruction *succ = (*use)->getInst();
        assert(succ);

        // successor has been scheduled
        if (moduloScheduler.schedTime.find(succ) !=
            moduloScheduler.schedTime.end()) {
            int min = moduloScheduler.schedTime[I] + moduloScheduler.delay(I) -
                      moduloScheduler.II * moduloScheduler.distance(I, succ);
            if (moduloScheduler.schedTime[succ] < min) {
                assert(succ != I);
                return true;
            }
        }
    }

    // no conflicts. Safe to schedule
    return false;
}

void SDCModuloScheduler::findASAPTimeForEachInst(
    map<Instruction *, int> &instStepASAP) {
    instStepASAP.clear();
    printLineBreak();
    if (SDCdebug)
        File() << "Finding initial ASAP schedule\n";
    for (IntToInstMapTy::iterator j = sdcSchedInst.begin(),
                                  je = sdcSchedInst.end();
         j != je; ++j) {
        int asapCstep = j->first;
        for (std::list<Instruction *>::iterator
                 i = sdcSchedInst[asapCstep].begin(),
                 e = sdcSchedInst[asapCstep].end();
             i != e; ++i) {
            instStepASAP[*i] = asapCstep;
            if (SDCdebug)
                File() << "Time: " << asapCstep << " I: " << **i << "\n";
        }
    }
    printLineBreak();
}

void SDCModuloScheduler::backtracking(Instruction *I, int cstep) {

    // assert(0 && "backtracking!");
    if (LEGUP_CONFIG->getParameterInt("MODULO_DEBUG")) {
        errs() << "BACKTRACKING...\n";
    }
    if (SDCdebug)
        File() << "Backtracking...\n";

    // try *all* possibilities at this point
    // you just probably want to keep track of prev schedule
    // and always stay above that point...

    // try all possible locations from ASAP time to cstep to find
    // the first time slot that has *only* a resource conflict so
    // we can evict that already scheduled instruction and
    // replace with the current instruction
    int minTime;
    bool found = false;
    for (minTime = instStepASAP[I]; minTime <= (int)cstep; ++minTime) {
        if (!schedulingConflictSDCIgnoreResources(I, minTime)) {
            found = true;
            break;
        }
    }
    assert(found);

    // doesn't work:
    // int minTime = instStepASAP[I];

    // if a legal slot was not found, then pick the first option from the
    // following:
    // 1) minTime - if this is the first time the operation is being scheduled
    // or if minTime is greater than the time the operation was last scheduled
    // 2) previously scheduled time + 1
    int evictSlot;
    if (neverScheduled[I] || minTime > prevSchedTime[I]) {
        if (SDCdebug)
            File() << "Forcing to minTime\n";
        evictSlot = minTime;
    } else {
        if (SDCdebug)
            File() << "Forcing to prev sched time + 1\n";
        evictSlot = prevSchedTime[I] + 1;
    }

    /*
     * doesn't work - because there are fixed resource constrained
     * nodes already in the SDC formulation
     while (schedulingConflictSDCIgnoreResources(I, evictSlot)) {
     evictSlot++;
     }
     */

    if (SDCdebug)
        File() << "Forcing to time: " << evictSlot << "\n";

    assert(schedulingConflictSDC(I, evictSlot));
    if (resourceConflict(I, evictSlot)) {
        int moduloTimeSlot = evictSlot % moduloScheduler.II;
        std::string FuName =
            LEGUP_CONFIG->getOpNameFromInst(I, moduloScheduler.alloc);
        Instruction *evicted =
            moduloScheduler.getReservationTable(FuName, 0, moduloTimeSlot);
        assert(evicted);
        if (SDCdebug)
            File() << "Resource conflict. Evicting: " << *evicted << "\n";

        // unschedule the old instruction that has a resource conflict
        unscheduleSDC(evicted);
    }

    assert(!resourceConflict(I, evictSlot));

    if (schedulingConflictSDC(I, evictSlot)) {

        // when using the prevSchedTime + 1 there might still be
        // a dependency conflict at this slot
        // to handle that, we loop over all other resource
        // constrained instructions and try to evict them
        // individually until the SDC is feasible when scheduling
        // the current instruction at the eviction time slot

        // create a list of instructions to evict
        // todo: could make this smarter to only evict
        // instructions dependent on the current instruction
        std::list<Instruction *> evictionList;
        for (std::map<Instruction *, int>::iterator
                 j = moduloScheduler.schedTime.begin(),
                 je = moduloScheduler.schedTime.end();
             j != je; ++j) {
            Instruction *possibleEviction = j->first;
            evictionList.push_back(possibleEviction);
        }

        // now evict the instructions one at a time
        /*
           for (std::list<Instruction*>::iterator j =
           evictionList.begin(), je = evictionList.end(); j
           != je; ++j) {
           Instruction *possibleEviction = *j;
           int oldTime = moduloScheduler.schedTime[possibleEviction];
           unscheduleSDC(possibleEviction);
           if (!schedulingConflictSDC(I, evictSlot)) {
           if(SDCdebug) File() << "Dependency conflict. Evicting: " <<
         *possibleEviction << "\n";
         break;
         } else {
        // still a conflict.
        // re-schedule and try evicting the next one...
        scheduleSDCInstruction(possibleEviction, oldTime);
        }
        }
        */

        if (schedulingConflictSDC(I, evictSlot)) {
            // just evict *everything*
            if (SDCdebug)
                File() << "Evicting everything\n";
            for (std::list<Instruction *>::iterator j = evictionList.begin(),
                                                    je = evictionList.end();
                 j != je; ++j) {
                Instruction *possibleEviction = *j;
                if (SDCdebug)
                    File() << "Evicting: " << *possibleEviction << "\n";
                unscheduleSDC(possibleEviction);
            }
        }
    }

    // remove all the old GE constraints?
    // deleteAllConstraints(GEConstraints);
    // GEConstraints.clear();

    assert(!schedulingConflictSDC(I, evictSlot));

    // schedule the instruction in the evicted slot (add the EQ
    // constraint to sdc and add instr to modulo reservation
    // table)
    scheduleSDCInstruction(I, evictSlot);
}

bool SDCModuloScheduler::SDCWithBacktracking(int budget) {
    // the step the instruction would be scheduled to under ASAP scheduling
    initializeSDC(moduloScheduler.II);
    bool initialSolve = scheduleSDC();
    assert(initialSolve);
    map<Instruction *, int> instStepASAP;
    findASAPTimeForEachInst(instStepASAP);

    if (LEGUP_CONFIG->getParameterInt("SDC_PRIORITY")) {
        calculatePerturbation();
    }

    // make this a priority queue based on cstep?
    while (budget > 0) {

        Instruction *I = NULL;
        if (LEGUP_CONFIG->getParameterInt("SDC_BACKTRACKING_PRIORITY")) {
            if (unscheduledInstPriorityQueue.empty())
                break;
            I = unscheduledInstPriorityQueue_pop();
        } else {
            if (unscheduledInstsConstrainedQueue.empty())
                break;
            I = unscheduledInstsConstrainedQueue.front();
            unscheduledInstsConstrainedQueue.pop();
        }

        // control step in the schedule
        unsigned cstep = sdcSchedTime[I];

        if (SDCdebug)
            File() << "Control Step: " << cstep << "\n";
        if (SDCdebug)
            File() << "Scheduling: " << *I << "\n";

        // skip already scheduled instructions
        if (unscheduledInstsConstrained.find(I) ==
            unscheduledInstsConstrained.end()) {
            assert(0);
            continue;
        }

        // try scheduling I in the current time step (cstep) or earlier
        assert(instStepASAP[I] <= (int)cstep);
        /*
        bool feasible = false;
        for (int backtrackStep = cstep; backtrackStep >= instStepASAP[I];
                --backtrackStep) {
        //for (int backtrackStep = instStepASAP[I]; backtrackStep <=
        //        (int)cstep; ++backtrackStep) {

            // is this schedule feasible?
            if (!schedulingConflictSDC(I, backtrackStep)) {
                // constraint the instruction to timestep and update
                // modulo resource table
                scheduleSDCInstruction(I, backtrackStep);
                printModuloReservationTable();
                feasible = true;
                assert(backtrackStep == cstep);
                //continue;
                break;
            }

            if(SDCdebug) File() << "Scheduling conflict at time step: " <<
                backtrackStep << "\n";
        }
        */

        // is this schedule feasible?
        if (!resourceConflict(I, cstep)) {
            assert(!schedulingConflictSDC(I, cstep));
            // constraint the instruction to timestep and update
            // modulo resource table
            scheduleSDCInstruction(I, cstep);
            if (LEGUP_CONFIG->getParameterInt("DEBUG_MODULO_TABLE")) {
                printModuloReservationTable();
            }
            // feasible = true;
            // assert(backtrackStep == cstep);
            // continue;
            // break;
        } else {

            SDCSolver::Constraints *GEconstraint;

            // if (!feasible) {
            // backtracking failed:
            // add constraint that instruction must be scheduled
            // after current time step
            GEconstraint = constrainSDC(I, GE, cstep + 1);
            // GEConstraints.push_back(GEconstraint);

            // if (moduloScheduler.schedTime.find(I) !=
            // moduloScheduler.schedTime.end()) {
            // unscheduledInstsConstrainedQueue.push(I);
            //}

            //}

            // the scheduling SDC solution must be re-solved based
            // on the new constraint added above
            // technically we don't need to re-solve the SDC in the case
            // where instruction I was fixed to cstep using an equality
            // constraint - as this constraint is already met in the current
            // solution
            // bool success = checkFeasible();
            bool success = scheduleSDC();

            if (success) {
                unscheduledInstsConstrainedQueue.push(I);
                unscheduledInstPriorityQueue.push_back(I);
                assert(unscheduledInstsConstrained.find(I) !=
                       unscheduledInstsConstrained.end());
            } else {

                // remove the GE constraint that caused the failure
                // cstep represents the upper max of the timestep we
                // can reach with the current EQ constraints (assuming no
                // resource constraints)
                sdcSolver.deleteConstraints(GEconstraint);

                backtracking(I, cstep);

                if (SDCdebug)
                    File() << "Failed to schedule time >= " << (cstep + 1)
                           << " for: " << *I << "\n";

                // success = checkFeasible();
                success = scheduleSDC();

                // if backtracking worked then we should be successful
                // in finding a schedule
                assert(success);

                // reset the cstep to go back to the evicted instruction to
                // reschedule it
                // cstep = sdcSchedTime[evicted];
                // cstep = evictSlot;
                cstep = 0;
            }

            assert(success);
        }

        /*
        if (!success) {
            if(SDCdebug) File() << "Can't schedule at II = " << II << "\n";
            return false;
        }
        */

        budget--;
    }

    // assert(unscheduledInstsConstrainedQueue.empty());

    if (budget > 0) {
        assignUnconstrainedOperations();
    } else {
        if (SDCdebug)
            File() << "Budget exceeded. Giving up\n";
        if (SDCdebug)
            File() << "Failure to schedule at II = " << moduloScheduler.II
                   << "\n";
        if (LEGUP_CONFIG->getParameterInt("MODULO_DEBUG")) {
            errs() << "Failure to schedule at II = " << moduloScheduler.II
                   << "\n";
        }
        // assert(0);
    }

    return unscheduledInstsConstrained.empty();
}

Instruction *SDCModuloScheduler::unscheduledInstPriorityQueue_pop() {
    assert(!unscheduledInstPriorityQueue.empty());
    // find the instruction with the lowest control step
    // and the highest 'perturbation' of other operations
    int lowestStep = 1e6;
    int highestPerturbation = -1e6;
    Instruction *highestPriority = NULL;
    std::list<Instruction *>::iterator highestPriorityIt;

    for (std::list<Instruction *>::iterator
             i = unscheduledInstPriorityQueue.begin(),
             e = unscheduledInstPriorityQueue.end();
         i != e; ++i) {
        Instruction *I = *i;
        int cstep = sdcSchedTime[I];
        if (cstep < lowestStep) {
            lowestStep = cstep;
            highestPerturbation = perturbation[I];
            highestPriority = I;
            highestPriorityIt = i;
        } else if (cstep == lowestStep) {
            if (LEGUP_CONFIG->getParameterInt("SDC_PRIORITY")) {
                if (perturbation[I] > highestPerturbation) {
                    lowestStep = cstep;
                    highestPerturbation = perturbation[I];
                    highestPriority = I;
                    highestPriorityIt = i;
                }
            }
        }
    }
    assert(highestPriority);
    unscheduledInstPriorityQueue.erase(highestPriorityIt);
    return highestPriority;
}

/*
std::priority_queue<Instruction*, std::list<Instruction*>, myComp>
unscheduledInstPriorityQueue;

// first we prioritize by SDC step - instructions scheduled earlier should
struct myComp {
  bool operator() (const Instruction* a, const Instruction* b) const {
      return a->delay_ < b->delay_;
  }
};

void SDCModuloScheduler::updatePriorityQueue() {
  unscheduledInstPriorityQueue.clear();
  for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; i++) {
      unscheduledInstPriorityQueue.push(i);

  }
}

*/

//
void SDCModuloScheduler::calculatePerturbation() {
    printLineBreak();
    if (SDCdebug)
        File() << "Calculating perturbation priority function\n";
    std::map<Instruction *, int> origSdcSchedTime = sdcSchedTime;
    for (std::list<Instruction *>::iterator
             i = unscheduledInstPriorityQueue.begin(),
             e = unscheduledInstPriorityQueue.end();
         i != e; ++i) {
        Instruction *I = *i;

        int cstep = sdcSchedTime[I];

        SDCSolver::Constraints *C1 = NULL;
        if (LEGUP_CONFIG->getParameterInt("INCREMENTAL_SDC")) {
            InstructionNode *in = dag->getInstructionNode(I);
            assert(startVariableIndex.find(in) != startVariableIndex.end());
            int idx = 1 + startVariableIndex[in];
            int before = sdcSolver.getD(sdcSolver.FeasibleSoln, idx);

            // tentatively add a GE constraint
            C1 = constrainSDC(I, GE, cstep + 1);
            // Constraint *C2 = constrainSDC(I, LE, cstep-1);

            int after = sdcSolver.getD(sdcSolver.FeasibleSoln, idx);

            // if (LEGUP_CONFIG->getParameterInt("INCREMENTAL_SDC")) {
            // bool success = checkFeasible();
            // bool isFeasible = sdcSolver.unprocessed.empty();
            // assert(success);

            if (SDCdebug)
                File() << "Perturbation from incr SDC: "
                       << sdcSolver.affected.size() << "\n";
            if (SDCdebug)
                File() << "Before: " << before << "\n";
            if (SDCdebug)
                File() << "After: " << after << "\n";
        } else {
            // tentatively add a GE constraint
            C1 = constrainSDC(I, GE, cstep + 1);
        }

        // bool success = checkFeasible();
        bool success = scheduleSDC();

        // remove the equality constraint added above
        sdcSolver.deleteConstraints(C1);
        // deleteConstraint(C2);

        // assert(success);
        if (!success) {

            if (SDCdebug)
                File() << "Couldn't perturb operation. Giving it 1000\n";
            perturbation[I] = 1000;
            continue;
        }

        // count how many operations had to move...
        int numberOfChanges = 0;
        if (LEGUP_CONFIG->getParameterInt("DEBUG_PERTURBATION")) {
            if (SDCdebug)
                File() << "How many instruction changed?\n";
        }
        for (std::map<Instruction *, int>::iterator j = sdcSchedTime.begin(),
                                                    je = sdcSchedTime.end();
             j != je; j++) {
            Instruction *J = j->first;
            if (sdcSchedTime[J] != origSdcSchedTime[J]) {
                if (LEGUP_CONFIG->getParameterInt("DEBUG_PERTURBATION")) {
                    if (SDCdebug)
                        File() << "\tChanged (" << origSdcSchedTime[J] << " -> "
                               << sdcSchedTime[J] << "): " << *J << "\n";
                }
                numberOfChanges++;
            }
            // else {
            //    if(SDCdebug) File() << "\tUnchanged (" << origSdcSchedTime[J]
            //    << " -> "
            //        << sdcSchedTime[J] << "): " << *J << "\n";
            //}
        }

        if (SDCdebug)
            File() << "Perturbation: " << numberOfChanges << " for: " << *I
                   << "\n";
        perturbation[I] = numberOfChanges;
    }

    // reset the SDC scheduler back to baseline
    bool success = checkFeasible(); // scheduleSDC();
    assert(success);
    printLineBreak();
}

bool SDCModuloScheduler::SDCGreedy() {

    // TODO: refactor duplicate code
    // the step the instruction would be scheduled to under ASAP scheduling
    initializeSDC(moduloScheduler.II);
    bool initialSolve = scheduleSDC();
    assert(initialSolve);
    map<Instruction *, int> instStepASAP;
    findASAPTimeForEachInst(instStepASAP);

    if (LEGUP_CONFIG->getParameterInt("SDC_PRIORITY")) {
        calculatePerturbation();
    }

    // while(!unscheduledInsts.empty()) {
    while (!unscheduledInstPriorityQueue.empty()) {

        // if (cstep > maxCstep) {
        //        break;
        //}

        Instruction *I = unscheduledInstPriorityQueue_pop();

        // control step in the schedule
        unsigned cstep = sdcSchedTime[I];

        // loop over every instruction scheduled at this control step
        // note: the sdcSchedInst map is modified inside this loop - because
        // of rescheduling caused by calling scheduleSDC()
        // std::list<Instruction*>::iterator i = sdcSchedInst[cstep].begin(),
        // e
        //= sdcSchedInst[cstep].end();
        // while (i != e) {

        // Instruction *I = *i;
        //++i;

        if (SDCdebug)
            File() << "Control Step: " << cstep << "\n";

        if (unscheduledInsts.find(I) == unscheduledInsts.end()) {
            // the instruction may have already been scheduled
            assert(0);
            continue;
        }

        if (SDCdebug)
            File() << "Attempting to schedule (at cstep = " << cstep
                   << "): " << *I << "\n";

        // try scheduling I in the current time step (cstep) or earlier
        bool feasible = false;
        for (int backtrackStep = cstep; backtrackStep >= instStepASAP[I];
             --backtrackStep) {

            // is this schedule feasible?
            if (!schedulingConflictSDC(I, backtrackStep)) {
                // constraint the instruction to timestep and update
                // modulo resource table
                scheduleSDCInstruction(I, backtrackStep);
                feasible = true;
                // continue;
                break;
            }
        }

        if (!feasible) {
            // backtracking failed:
            // add constraint that instruction must be scheduled
            // after current time step
            constrainSDC(I, GE, cstep + 1);
            unscheduledInstPriorityQueue.push_back(I);
        }

        // the scheduling SDC solution must be re-solved based
        // on the new constraint added above
        // technically we don't need to re-solve the SDC in the case
        // where instruction I was fixed to cstep using an equality
        // constraint - as this constraint is already met in the current
        // solution
        // bool success = checkFeasible();
        bool success = scheduleSDC();

        if (!success) {
            if (SDCdebug)
                File() << "FAILURE!\nCan't schedule at II = "
                       << moduloScheduler.II << "\n";
            return false;
        }

        // after re-solving the SDC,  the data-structure sdcSchedInst has
        // now been modified, so we need to refresh the loop iterators
        // note: we will look at some instructions twice in this time
        // step, but we skip any instructions already scheduled
        // at the start of the loop
        // i = sdcSchedInst[cstep].begin();
        // e = sdcSchedInst[cstep].end();

        //}
        // cstep++;
    }

    // scheduleSDC();
    assignUnconstrainedOperations();

    if (SDCdebug)
        File() << "SDC-based IMS successful\n";
    return true;
}

void SDCModuloScheduler::assignUnconstrainedOperations() {
    for (std::set<Instruction *>::iterator i = unscheduledInsts.begin(),
                                           e = unscheduledInsts.end();
         i != e; ++i) {
        Instruction *I = *i;
        if (moduloScheduler.schedTime.find(I) ==
            moduloScheduler.schedTime.end()) {
            int time = sdcSchedTime[I];
            if (SDCdebug)
                File() << "Assigning timeslot: " << time << " to " << *I
                       << "\n";
            assert(!moduloScheduler.isResourceConstrained(I) &&
                   "Instruction should have already been scheduled");
            moduloScheduler.schedTime[I] = time;
        }
    }
}

bool SDCModuloScheduler::iterativeSchedule(int budget) {
    init();

    // calculate height priority
    computeHeights();

    // mark all operations as never scheduled
    // add all operations into unscheduled list
    for (BasicBlock::iterator instr = BB->begin(), ie = BB->end(); instr != ie;
         ++instr) {
        neverScheduled[instr] = true;
        unscheduledInsts.insert(instr);
        if (moduloScheduler.isResourceConstrained(instr)) {
            unscheduledInstsConstrained.insert(instr);
            unscheduledInstsConstrainedQueue.push(instr);
            unscheduledInstPriorityQueue.push_back(instr);
        }
        if (SDCdebug)
            File() << "Height: " << height[instr] << ": " << *instr << "\n";
    }

    std::string schedulerType = LEGUP_CONFIG->getParameter("MODULO_SCHEDULER");
    if (SDCdebug)
        File() << "Modulo Scheduler Type: " << schedulerType << "\n";
    if (schedulerType == "SDC_BACKTRACKING") {
        return SDCWithBacktracking(budget);
    } else if (schedulerType == "SDC_GREEDY") {
        return SDCGreedy();
    } else if (schedulerType == "ITERATIVE") {
        return iterativeClassic(budget);
    } else {
        assert(0 && "Unrecognzied modulo scheduler type");
    }
}

static void printNodeLabel(raw_ostream &out, Instruction *I) { out << *I; }

void SDCModuloScheduler::printMinDistDot(int II) {
    std::string FileError;
    std::string FileName = "mindist." + utostr(II) + ".dot";
    raw_fd_ostream moduloDotFile(FileName.c_str(), FileError,
                                 llvm::sys::fs::F_None);
    assert(FileError.empty() && "Error opening log files");
    formatted_raw_ostream out(moduloDotFile);

    dotGraph<Instruction> graph(out, printNodeLabel);
    graph.setLabelLimit(20);

    for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; ++i) {
        for (BasicBlock::iterator j = BB->begin(), je = BB->end(); j != je;
             ++j) {
            // -inf: no path from i to j in dependency graph
            if (minDist[i][j] < -1000)
                continue;

            std::string str;
            raw_string_ostream stream(str);
            stream << minDist[i][j];

            std::string label = "label=\"" + stream.str();
            if (moduloScheduler.dependent(i, j)) {
                assert(moduloScheduler.distance(i, j) >= 0);
                label += " (" + utostr(moduloScheduler.distance(i, j)) + ")";
            }
            label += "\"";
            if ((i == j) && (minDist[i][j] > 0)) {
                label += ",color=red";
            } else if (minDist[i][j] < 0) {
                label += ",color=green";
            } else if (minDist[i][j] > 0) {
                label += ",color=orange";
            }

            graph.connectDot(out, i, j, label);
        }
    }
}

int SDCModuloScheduler::recurrenceMII_SDC(int resourceMII) {
    printLineBreak();
    if (SDCdebug)
        File() << "Calculating recurrence MII using SDC scheduler\n";
    int recMII = resourceMII;
    // int recMII = 0;

    // first solve initial SDC
    bool success = false;

    do {
        if (SDCdebug)
            File() << "Trying recMII (SDC) = " << recMII << "\n";
        moduloScheduler.II = recMII;
        initializeSDC(recMII);
        success = checkFeasible(); // scheduleSDC(); //
        if (!success) {
            if (SDCdebug)
                File() << "Scheduling failed. Incrementing recMII\n";
            recMII++;
            moduloScheduler.sanityCheckII(recMII);
        } else {
            break;
        }
    } while (1);

    if (SDCdebug)
        File() << "recMII = " << recMII << " using SDC scheduler\n";
    printLineBreak();
    return recMII;
}

int SDCModuloScheduler::recurrenceMII(int resourceMII) {
    if (SDCdebug)
        File() << "Calculating recurrence MII using IMS technique\n";
    int recMII = resourceMII;
    // find smallest II with no positive cycle
    bool positiveCycle;
    do {
        if (SDCdebug)
            File() << "Trying recMII = " << recMII << "\n";
        positiveCycle = computeMinDist(recMII);
        printMinDistDot(recMII);
        if (!positiveCycle) {
            break;
        } else {
            if (SDCdebug)
                File() << "Positive cycle detected. Incrementing recMII\n";
            recMII++;
            moduloScheduler.sanityCheckII(recMII);
        }
    } while (1);

    return recMII;
}

void SDCModuloScheduler::initMinDist(int II) {
    int negInf = -100000;

    for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; ++i) {
        for (BasicBlock::iterator j = BB->begin(), je = BB->end(); j != je;
             ++j) {
            minDist[i][j] = negInf;
            if (!moduloScheduler.dependent(i, j))
                continue;
            minDist[i][j] = std::max(minDist[i][j],
                                     moduloScheduler.delay(i) -
                                         II * moduloScheduler.distance(i, j));
            if (i == j)
                assert(minDist[i][j] <= 0);
        }
        // if i has no children then it connects to the STOP pseudo node
        heightR[i] = negInf;
        if (noChildren(i)) {
            // minDist[i][STOP]
            heightR[i] = 0;
        }
    }
}

bool SDCModuloScheduler::computeMinDist(int II) {

    initMinDist(II);

    // now consider all paths via vertex k as well: O(n^3)
    for (BasicBlock::iterator k = BB->begin(), ke = BB->end(); k != ke; ++k) {
        for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie;
             ++i) {
            for (BasicBlock::iterator j = BB->begin(), je = BB->end(); j != je;
                 ++j) {
                int dist = minDist[i][k] + minDist[k][j];
                if (dist > minDist[i][j]) {
                    minDist[i][j] = dist;
                    if ((i == j) && (dist > 0)) {
                        // i must be scheduled after itself - impossible
                        // positive cycle detected
                        if (SDCdebug)
                            File() << "Positive Cycle Detected:\n";
                        if (SDCdebug)
                            File() << "   i == j: " << *j << "\n";
                        if (SDCdebug)
                            File() << "   minDist[i][j] = min[i][k] + "
                                      "minDist[k][j]\n";
                        if (SDCdebug)
                            File() << "   minDist[i][k]: " << minDist[i][k]
                                   << "\n";
                        if (SDCdebug)
                            File() << "   minDist[k][j]: " << minDist[k][j]
                                   << "\n";
                        if (SDCdebug)
                            File() << "   i/j: " << *i << "\n";
                        if (SDCdebug)
                            File() << "   k: " << *k << "\n";
                        if (SDCdebug)
                            File() << "   dist: " << dist << "\n";
                        if (SDCdebug)
                            File() << "   Instruction i must be scheduled "
                                   << "after itself. Impossible!\n";
                        if (SDCdebug)
                            File() << "   Check the mindist." << utostr(II)
                                   << ".dot graph for the red connection\n";
                        return true;
                    }
                }
            }
        }
    }

    /*
    if (moduloScheduler.forceNoChain) {
        // copy
        // minDistWithChaining = minDist;
        for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie;
    ++i) {
            for (BasicBlock::iterator j = BB->begin(), je = BB->end(); j !=
    je; ++j) {
                minDistWithChaining[i][j] = minDist[i][j];
            }
        }
    }
    */

    // no positive cycle
    return false;
}

void SDCModuloScheduler::resetMinDistForDetectingRecurrences() {
    int negInf = -100000;
    for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; ++i) {
        for (BasicBlock::iterator j = BB->begin(), je = BB->end(); j != je;
             ++j) {
            minDistCopy[i][j] = negInf;
        }
    }
}

void SDCModuloScheduler::restructureLoopRecurrences(int resMII) {
    if (LEGUP_CONFIG->getParameterInt("MODULO_DEBUG")) {
        errs() << "Restructuring expression tree to minimize recurrences\n";
    }

    moduloScheduler.forceNoChain = true;

    if (SDCdebug)
        File() << "Calculating minDis with no chaining\n";
    int MII = recurrenceMII(resMII);
    moduloScheduler.findLoopRecurrences();
    if (SDCdebug)
        File() << "recMII (mindist): " << MII << "\n";
    saveMinDistForDetectingRecurrences(MII);

    moduloScheduler.restructureDFG();

    resetMinDistForDetectingRecurrences();

    if (SDCdebug)
        File() << "Regenerating dependency DAG\n";
    dag->runOnFunction(*F, moduloScheduler.alloc);

    moduloScheduler.localMemDistances.clear();
    moduloScheduler.addLocalMemConstraints();

    moduloScheduler.forceNoChain = false;
}

void SDCModuloScheduler::saveMinDistForDetectingRecurrences(int recMII) {
    for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; ++i) {
        for (BasicBlock::iterator j = BB->begin(), je = BB->end(); j != je;
             ++j) {
            minDistCopy[i][j] = minDist[i][j];
        }
    }
    origRecMII = recMII;
}

int SDCModuloScheduler::resourceMII() {
    printLineBreak();
    if (SDCdebug)
        File() << "\nCalculating resource MII using IMS technique\n";

    int resMII = 1;
    for (std::set<std::string>::iterator
             i = moduloScheduler.constrainedFuNames.begin(),
             e = moduloScheduler.constrainedFuNames.end();
         i != e; ++i) {
        std::string FuName = *i;
        int issueSlots = numIssueSlots(FuName);
        if (SDCdebug)
            File() << "Constraints from FuName: " << FuName
                   << " Issue Slots: " << issueSlots << "\n";
        int opMII = 1;
        int numOps = 0;
        for (BasicBlock::iterator instr = BB->begin(), ie = BB->end();
             instr != ie; ++instr) {
            std::string opName =
                LEGUP_CONFIG->getOpNameFromInst(instr, moduloScheduler.alloc);
            if (opName == FuName) {
                if (SDCdebug)
                    File() << "-" << *instr << "\n";
                numOps++;
            }
        }
        assert(issueSlots);
        opMII = std::max(opMII, (int)ceil((float)numOps / (float)issueSlots));
        if (SDCdebug)
            File() << "resMII (due to " << FuName << "): " << opMII << "\n";
        resMII = std::max(resMII, opMII);
    }
    if (SDCdebug)
        File() << "Overall resMII: " << resMII << "\n";

    moduloScheduler.sanityCheckII(resMII);

    if (SDCdebug)
        File() << "resMII: " << resMII << "\n";
    printLineBreak();
    return resMII;
}

void SDCModuloScheduler::RemapInstruction(Instruction *I,
                                          ValueToValueMapTy &VMap) {
    assert(I);
    for (unsigned op = 0, E = I->getNumOperands(); op != E; ++op) {
        Value *Op = I->getOperand(op);
        assert(Op);
        ValueToValueMapTy::iterator It = VMap.find(Op);
        if (It != VMap.end()) {
            assert(It->second);
            // errs() << "Changing op " << op << " to " << *It->second <<
            // "\n";
            I->setOperand(op, It->second);
        }
    }
}

void SDCModuloScheduler::printMap(map<int, ValueToValueMapTy> &valueMapIter,
                                  Value *v, int iter) {
    for (int j = 0; j <= iter; j++) {
        if (SDCdebug)
            File() << "i=" << j << ": " << *v << " -> " << *valueMapIter[j][v]
                   << "\n";
    }
}

void SDCModuloScheduler::initLoop() {

    // errs() << "BB: " << *BB << "\n";
    // Dependences *DP = &getAnalysis<Dependences>();
    // DP->printScop(errs());

    moduloScheduler.loopPreheader = moduloScheduler.loop->getLoopPreheader();
    assert(moduloScheduler.loopPreheader);
    F = moduloScheduler.loopPreheader->getParent();
    M = F->getParent();

    // errs() << "Depth: " << moduloScheduler.loop->getLoopDepth() << "\n";
    // only handle loops with one BB?
    // assert(moduloScheduler.loop->getLoopDepth() == 1);

    /*
       DT = &getAnalysis<DominatorTree>();
       C = &getAnalysis<CloogInfo>();
       SD = &getAnalysis<ScopDetection>();
       TD = &getAnalysis<DataLayout>();
       */

    /*
    S = &scop;
    region = &S->getRegion();
    R = region;

    F = R->getEntry()->getParent();
    BB = R->getEntry();
    //moduloScheduler.loop = LI->getLoopFor(BB);
    */

    // AliasAnalysis *AA = &getAnalysis<AliasAnalysis>();

    moduloScheduler.alloc = new Allocation(M);
    moduloScheduler.alloc->addAA(AA);
    Scheduler::alloc = moduloScheduler.alloc;

    if (!moduloScheduler.ranAlready) {
        if (SDCdebug)
            File() << getFileHeader();

        moduloScheduler.verify_can_find_all_loop_labels();
    }
    moduloScheduler.ranAlready = true;

    if (SDCdebug)
        File() << "Found Loop: " << *moduloScheduler.loop << "\n";
    if (SDCdebug)
        File() << "Label: " << moduloScheduler.loopLabel << "\n";
    // moduloScheduler.tripCount = L->getSmallConstantTripCount();
    // TODO: this may be wrong, just needed to do something to make it compile
    moduloScheduler.tripCount = SE->getSmallConstantTripCount(
        moduloScheduler.loop, moduloScheduler.loop->getExitingBlock());

    // trip count might not be constant -> in which case the trip count is 0
    // assert(moduloScheduler.tripCount);
    if (SDCdebug)
        File() << "Trip count: " << moduloScheduler.tripCount << "\n";

    /*
    BasicBlock *LatchBlock = L->getLoopLatch();
    if (LatchBlock) {
        SE = &getAnalysis<ScalarEvolution>();
        unsigned TripCount = SE->getSmallConstantTripCount(L, LatchBlock);
        unsigned TripMultiple = SE->getSmallConstantTripMultiple(L,
    LatchBlock);
        if(SDCdebug) File() << "Trip count: " << TripCount << "\n";
        if(SDCdebug) File() << "Trip multiple: " << TripMultiple << "\n";
    }
    */

    PHINode *induction = moduloScheduler.loop->getCanonicalInductionVariable();
    if (!induction) {
        if (SDCdebug)
            File()
                << "Error: Couldn't canonicalize induction variable! Skipping "
                   "pipelining\n";
        return;
    }

    assert(induction);

    //  canonical induction variable: an integer recurrence that starts at 0
    //  and increments by one each time through the loop.
    if (SDCdebug)
        File() << "Induction variable: " << *induction << "\n";
    setMetadataInt(induction, "legup.canonical_induction", 1);

    // the loop body should only have a single predecessor
    assert(moduloScheduler.loopPreheader);

    dag = new SchedulerDAG;
    moduloScheduler.dag = dag;
    sdcSolver.dag = dag;
    dag->runOnFunction(*F, moduloScheduler.alloc);

    moduloScheduler.addLocalMemConstraints();

    moduloScheduler.printDFGFile("pipelineDFG.dot");

    // canonical induction variable starts at 0 and increments by 1
    moduloScheduler.inductionVar =
        moduloScheduler.loop->getCanonicalInductionVariable();
    assert(moduloScheduler.inductionVar);

    if (SDCdebug)
        File() << "Loop preheader: " << moduloScheduler.loopPreheader->getName()
               << "\n";

    moduloScheduler.II = 1;
    moduloScheduler.initReservationTable();
    init();
}

int SDCModuloScheduler::getInitialMII() {
    // int MII = std::max(resourceMII(), recurrenceMII());
    int resMII = resourceMII();

    int MII = resMII;

    if (LEGUP_CONFIG->getParameterInt("RESTRUCTURE_LOOP_RECURRENCES")) {
        restructureLoopRecurrences(resMII);

        resMII = resourceMII();

        moduloScheduler.printDFGFile("pipelineDFG.after.dot");

        MII = recurrenceMII(resMII);
        saveMinDistForDetectingRecurrences(MII);
    }

    int elemRecMII = moduloScheduler.findLoopRecurrences();

    // recurrenceMII starts from resourceII
    int recMII = 0;

    if (LEGUP_CONFIG->getParameter("MODULO_SCHEDULER") == "ITERATIVE") {

        recMII = recurrenceMII(elemRecMII);
    } else {

        // with SDC, we can't use iterative modulo scheduling to detect
        // recurrences because of chaining
        recMII = recurrenceMII_SDC(elemRecMII);
    }

    assert(
        recMII >= elemRecMII &&
        "Sanity check. The recMII should never be less than the one calculated"
        "from elementary recurrence analysis");

    if (recMII != elemRecMII) {
        errs() << "WARNING: elemRecMII: " << elemRecMII << " != MII: " << recMII
               << "\n";
        errs() << "Is there a problem with the SDC formulation?\n";
    }

    MII = max(recMII, resMII);

    return MII;
}
