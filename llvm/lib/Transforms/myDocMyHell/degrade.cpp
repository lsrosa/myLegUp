#include  <cstdlib>
#include "ILPModuloScheduler.h"
#include <lp_lib.h>
#define steps 20
#define repetitions 30

// using namespace polly;
using namespace llvm;
using namespace legup;

void ILPModuloScheduler::printModuleSchedulerRow(int scheduler, bool suc){
  FILE * pFile = fopen("DegradedModuleSchedulerTimes", "a");
  sched_II = moduloScheduler.II;
  if(suc){
    fprintf(pFile, "%s\t%d\t%f\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%f\t%f\n", moduloScheduler.loopLabel.c_str(), scheduler, step, nIRlines, lp_nvars, lp_nconst, sched_latency, sched_II, moduloScheduler.tripCount, nsdcs, totaltime, solvetime);
  }
  else{
    fprintf(pFile, "%s\t%d\t%f\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%f\t%f\n", moduloScheduler.loopLabel.c_str(), scheduler, step, nIRlines, 0, 0, 0, 0, moduloScheduler.tripCount, 0, totaltime, solvetime);
  }
  fclose(pFile);
  return;
}

void ILPModuloScheduler::degradeSDC(){
  int numOps = BB->size();

  //---------------------------------------------------------
  // we will get the minimum nsdcs for the maximum II
  //---------------------------------------------------------
  resetCounters();
  initializeSDC(getMaxPossibleII(BB));
  bool suc = iterativeSchedule(10*numOps);
  std::cout << "nsdcs: " << nsdcs << '\n';
  assert(suc == true && "oh deer God, teach your son spelling");
  int max_solves = nsdcs;

  //---------------------------------------------------------
  // degrade from max_solves
  //---------------------------------------------------------
  int lower = max_solves-round(0.1*numOps);
  if(lower <= 1 ) lower = 1;
  int upper = max_solves+numOps;

  float budgetRatio;
  //for(float budgetRatio = lower; budgetRatio <= upper; budgetRatio+=(float)(upper-lower)/steps){
  for(int i = 0; i<steps; i++){
    budgetRatio = lower+i*(float)(upper-lower)/steps;
    int ii = moduloScheduler.II;
    step = budgetRatio;
    resetCounters();
    clock_t tictt = clock();
    std::cout << "II: " << moduloScheduler.II << '\n';
    initializeSDC(ii);
    bool success = true;
    int br = budgetRatio;//floor(budgetRatio*numOps);
    std::cout << "nsdcs: " << nsdcs << '\n';
    //std::cout << "budgetRatio: " << br << '\n';
    //float counter = budgetRatio*numOps;//*iiCandidate;
    //std::cout << "counter: " << counter << " - br: " << br << '\n';
    while (!iterativeSchedule(br)) {
      ii++;
      std::cout << "II: " << ii << '\n';
      //moduloScheduler.sanityCheckII(moduloScheduler.II);
      if(ii > (int)BB->size()){
        clock_t toctt = clock();
        totaltime += (double)(toctt - tictt) / CLOCKS_PER_SEC;
        printModuleSchedulerRow(0, false);
        success = false;
        std::cout << "solvetime: " << solvetime << '\n';
        break;
      }
      initializeSDC(ii);
    }
    std::cout << "nsdcs: " << nsdcs << '\n';

    if(!success){
      continue;
    }

    clock_t toctt = clock();
    totaltime += (double)(toctt - tictt) / CLOCKS_PER_SEC;

    lp_nvars = get_Ncolumns(sdcSolver.lp);
    lp_nconst = get_Nrows(sdcSolver.lp);

    sched_latency = 0;
    for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; i++) {
      int delay = Scheduler::getNumInstructionCycles(i);
      //std::cout << getLabel(i) << " - " << moduloScheduler.schedTime[i] << '\n';
      if(sched_latency < (moduloScheduler.schedTime[i]+delay)){
        sched_latency = moduloScheduler.schedTime[i]+delay;
      }
    }

    printModuleSchedulerRow(0, true);
  }//for(budgetRatio = 1, budgetRatio <= 10; budgetRatio+=0.5)

  return;
}

void ILPModuloScheduler::degradeILP(){
  int numOps = BB->size();
  int MII = moduloScheduler.II;
  unsigned maxII = getMaxPossibleII(BB);
  if((unsigned)MII > maxII)
    maxII = MII + 1;

  int safeii  = moduloScheduler.II;
  //std::cout << "safeII: " << safeii << '\n';
  int cnt = 0;
  //for(solver_time_budget = 0.0001; solver_time_budget <= 5; solver_time_budget += (5-0.0001)/steps){
  float lower = ((float)1.0/60)*(numOps/82);
  float upper = ((float)1.0/60)*(5*numOps/82);
  int ii = moduloScheduler.II;

  for(int i = 0; i<steps; i++){
    solver_time_budget = lower + i*(float)(upper-lower)/steps;
    step = solver_time_budget;
    resetCounters();
    std::cout << "ILP iteration: " << cnt++ << '\n';
    clock_t tictt = clock();

    bool success;
    unsigned curII;

    for(curII = ii; curII <= maxII; curII++){
      std::cout << "curII: " << curII << '\n';
      initializeILP(curII);
      success = solveILP();
      if ( success ){
        break;
      }
      //ii++;
    }
    if(curII == maxII && !success){
      std::cout << "FAIL: could not schedule loop "  << "in " << MII << " < II < " << maxII << '\n';
      assert(true == false && "ILP formulation fails");//quit the hell out of here
    }

    clock_t toctt = clock();
    totaltime += (double)(toctt - tictt) / CLOCKS_PER_SEC;
    lp_nvars = get_Ncolumns(sdcSolver.lp);
    lp_nconst = get_Nrows(sdcSolver.lp);

    sched_latency = 0;
    for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; i++) {
      int delay = Scheduler::getNumInstructionCycles(i);
      //std::cout << getLabel(i) << " - " << moduloScheduler.schedTime[i] << '\n';
      if(sched_latency < (moduloScheduler.schedTime[i]+delay)){
        sched_latency = moduloScheduler.schedTime[i]+delay;
      }
    }
    moduloScheduler.II = curII;
    printModuleSchedulerRow(1, success);
  }
  moduloScheduler.II = safeii;
  return;
}

void ILPModuloScheduler::degradeGA(){
  unsigned minII = getInitialMII();
  unsigned maxII = getMaxPossibleII(BB);
  if(minII > maxII)
    maxII = minII + 1;

  generator.seed(std::time(0));
  float alpha;
  float beta; //0.5, 0.05

  int cnt = 0;
  float coef;
  //for(float coef = 0.05; coef <= 25; coef +=(25-0.05)/steps){
  float lower=0.1;
  float upper=1;
  for(int i = 0; i<steps; i++){
    coef = lower + i*(upper-lower)/steps;
    resetCounters();
    step = coef;
    std::cout << "GA iteration: " << cnt++ << '\n';

    for(int rep = 0; rep < repetitions; rep++){
      std::cout << "repetition: " << rep << '\n';
      clock_t tictt = clock();
      alpha = coef*0.5;//0.1;
      beta = coef*0.01;

      int size = moduloScheduler.BB->size();
      nPop = ceil(alpha*size);
      if(nPop%2 != 0){
        nPop++;
      }

      maxGen = ceil(coef*3);

      offspringSize = nPop;
      mutationProb = 1;

      std::cout << "minII: " << minII << " - maxII: " << maxII << '\n';
      std::cout << "PopSize: " << nPop  << " - maxGen: " << maxGen << " - OffspringSize: " << offspringSize << " - mutationProb: " << mutationProb << '\n';
      std::cout << "tripcount: " << moduloScheduler.tripCount << " - size: " << size << '\n';
      GA(&minII, &maxII);

      clock_t toctt = clock();
      totaltime += (double)(toctt - tictt) / CLOCKS_PER_SEC;

      lp_nvars = get_Ncolumns(sdcSolver.lp);
      lp_nconst = get_Nrows(sdcSolver.lp);

      int max_lat = 0;
      for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; i++) {
        int delay = Scheduler::getNumInstructionCycles(i);
        //std::cout << getLabel(i) << " - " << moduloScheduler.schedTime[i] << '\n';
        if(max_lat < (moduloScheduler.schedTime[i]+delay)){
          max_lat = moduloScheduler.schedTime[i]+delay;
        }
      }
      sched_latency += max_lat;
    }//for(int rep = 0; rep < repetitions; rep++)
    totaltime /= repetitions;
    solvetime /= repetitions;
    sched_latency = round((float)sched_latency/repetitions);
    nsdcs = round((float)nsdcs/repetitions);
    printModuleSchedulerRow(2, true);
  }

  return;
}

bool ILPModuloScheduler::degrade(){

  FILE *pFile;
  std::string rptname("DegradedModuleSchedulerTimes");
  std::ifstream f(rptname);
  if (!f.good()) {
      pFile = fopen(rptname.c_str(), "w");
      fprintf(pFile, "label\tscheduler\tstep\tn_IRlines\t#vars\t#constraints\tlatency\tII\tTripCnt\tn_solves\tTotal\tSolving\n");
      fclose(pFile);
  }

  degradeSDC();
  degradeILP();
  degradeGA();


  return false;
}
