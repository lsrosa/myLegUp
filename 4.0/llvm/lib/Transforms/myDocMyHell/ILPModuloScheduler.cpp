#include "ILPModuloScheduler.h"
// using namespace polly;
using namespace llvm;
using namespace legup;

void ILPModuloScheduler::resetCounters(){
  // leandro time measing for individual steps
  // variables keep their values between loops
  lp_nvars = 0;
  lp_nconst = 0;
  sched_latency = 0;
  sched_II = 0;
  totaltime = 0;
  solvetime = 0;
  nsdcs = 0;
  timeout = 0;
  return;
}

void ILPModuloScheduler::printModuleSchedulerHeader(){
  FILE *pFile;
  std::string rptname("DetailedModuleSDCSchedulingTime");
  std::ifstream f(rptname);
  if (!f.good()) {
      pFile = fopen(rptname.c_str(), "w");
      fprintf(pFile, "label\ttimeout\tn_IRlines\t#vars\t#constraints\tlatency\tII\tTripCnt\tn_solves\tTotal\tSolving\ttotalcycles\n");
      fclose(pFile);
  }
  return;
}

void ILPModuloScheduler::printModuleSchedulerRow(){
  FILE * pFile = fopen("DetailedModuleSDCSchedulingTime", "a");
  sched_II = moduloScheduler.II;
  fprintf(pFile, "%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%f\t%f\t%d\n", moduloScheduler.loopLabel.c_str(), timeout, nIRlines, lp_nvars, lp_nconst, sched_latency, sched_II, moduloScheduler.tripCount, nsdcs, totaltime, solvetime, sched_latency+sched_II*( moduloScheduler.tripCount-1));
  fclose(pFile);
  return;
}

void ILPModuloScheduler::clearILP(){
  if(solver.compare("gurobi")==0){
    if(GRBsolution!=NULL) delete GRBsolution;
    if(ILPGRBmodel!=NULL) delete ILPGRBmodel;
  }
  return;
}

bool ILPModuloScheduler::runOnLoop(Loop *L, LPPassManager &LPM) {
    moduloScheduler.loop = L;

    resetCounters();
    printModuleSchedulerHeader();

    nIRlines = 0;
    for (Loop::block_iterator bb = L->block_begin(), e = L->block_end();
         bb != e; ++bb) {
        nIRlines += (*bb)->size();
    }

    BB = moduloScheduler.find_pipelined_bb();
    moduloScheduler.BB = BB;
    if (!BB) {
        // didn't find the loop or pipelining is turned off
        return false;
    }

    initLoop();

    int MII = getInitialMII();

    //TODO change this
    unsigned maxII = getMaxPossibleII(BB);

    if((unsigned)MII >= maxII)
      maxII = MII + 1;
    // ModuloSchedulo
    //MII = recmii;
    std::cout << "maxII:" << maxII << std::endl;
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

    if (SDCdebug || GAdebug || ILPdebug || NIdebug)
        File() << "Initial II = " << moduloScheduler.II << "\n";

    //leandro - setting an if for ILP
    std::string schedulerType = LEGUP_CONFIG->getParameter("MODULO_SCHEDULER");

    if(schedulerType.compare("DEGRADE") == 0){
      return degrade();
    }

    clock_t tictt = clock();

    if(schedulerType.compare("ILP") == 0){
      resetCounters();
      //change max II here
      //int maxII = 2*moduloScheduler.II;
      bool success;
      unsigned curII = moduloScheduler.II;

      std::cout << "currII: " << curII << "   --  maxII: " << maxII << '\n';
      //File() << "currII: " << curII << "   --  maxII: " << maxII << '\n';
      for(curII = moduloScheduler.II; curII <= maxII; curII++){
        // moduloScheduler.sanityCheckII(moduloScheduler.II);
        // re-create the SDC constraints with new II
        initializeILP(moduloScheduler.II);
        success = solveILP();
        if ( success ){
          File() << "Saporradeucerto  -- II: " << moduloScheduler.II << '\n';
          break;
        }
        moduloScheduler.II++;
      }
      if(curII == maxII && !success){
        std::cout << "FAIL: could not schedule loop "  << "in " << MII << " < II < " << maxII << '\n';
        assert(true == false && "ILP formulation fails");//quit the hell out of here
      }
      clearILP();
    }else if(schedulerType.compare("GA") == 0){
      resetCounters();
      int size = moduloScheduler.BB->size();
      nPop = (unsigned)size;
      if (LEGUP_CONFIG->getParameterInt("GA_POPULATION_SIZE")) {
          nPop = (unsigned)size*(unsigned)LEGUP_CONFIG->getParameterInt("GA_POPULATION_SIZE");
      }
      nPop = ceil(0.25*nPop);
      // yes, I like even numbers
      //Offspring creation does not require this, but it will generate an even number of individuals.
      if(nPop%2 != 0){
        nPop++;
      }

      maxGen = 1*(unsigned)size;
      if (LEGUP_CONFIG->getParameterInt("GA_MAXIMUM_GENERATIONS")) {
          maxGen = (unsigned)size*(unsigned)LEGUP_CONFIG->getParameterInt("GA_MAXIMUM_GENERATIONS");
      }
      maxGen = ceil(0.1*maxGen);
      //if(maxGen>10) {
      //  maxGen=10;
      //}

      mutationProb = 1;
      if (LEGUP_CONFIG->getParameterInt("GA_MUTATION_PROB")) {
          mutationProb = (unsigned)LEGUP_CONFIG->getParameterInt("GA_MUTATION_PROB");
      }

      offspringSize = nPop;
      if (LEGUP_CONFIG->getParameterInt("GA_OFFSPRING_SIZE")) {
          offspringSize = (unsigned)LEGUP_CONFIG->getParameterInt("GA_OFFSPRING_SIZE");
      }

      unsigned minII = moduloScheduler.II;
      //assure rand will be rand
      generator.seed(std::time(0));
      //std::srand(std::time(0));
      //std::cout << "minII: " << minII << " - maxII: " << maxII << '\n';
      //std::cout << "PopSize: " << nPop  << " - maxGen: " << maxGen << " - OffspringSize: " << offspringSize << " - mutationProb: " << mutationProb << '\n';
      //std::cout << "tripcount: " << moduloScheduler.tripCount << " - size: " << size << '\n';

      GA(&minII, &maxII);

    }else if(schedulerType.compare("NI") == 0){
      resetCounters();
      bool success;
      unsigned curII;

      for(curII = moduloScheduler.II; curII <= maxII; curII++){
        if(NIdebug){
            std::cout << "II: " << curII << '\n';
        }
        success = NI(curII);
        if ( success ){
          File() << "Saporradeucerto  -- II: " << curII << '\n';
          break;
        }
      }
      moduloScheduler.II = curII;

      if(curII == maxII && !success){
        std::cout << "FAIL: could not schedule loop "  << "in " << MII << " < II < " << maxII << '\n';
        assert(false && "NI scheduler fails");//quit the hell out of here
      }
    }else if(schedulerType.compare("SM") == 0){
      resetCounters();
      bool success;
      unsigned curII;

      for(curII = moduloScheduler.II; curII <= maxII; curII++){
        if(NIdebug){
            std::cout << "II: " << curII << '\n';
        }
        std::cout << "calling SM" << '\n';
        success = SM(curII);
        if ( success ){
          File() << "Saporradeucerto  -- II: " << curII << '\n';
          File().flush();
          break;
        }
      }
      moduloScheduler.II = curII;

      std::cout << "currII: " << curII << " - maxII: " << maxII << '\n';
      if(curII >= maxII && !success){
        std::cout << "FAIL: could not schedule loop "  << "in " << MII << " < II < " << maxII << " - curII: " << curII<< '\n';
        std::cout << "FAIL: could not schedule loop "  << "in " << MII << " < II < " << maxII << '\n';
        assert(false && "NI scheduler fails");//quit the hell out of here
        File().flush();
      }
    }else{
      resetCounters();
      initializeSDC(moduloScheduler.II);

      // int budgetRatio = 100;
      int budgetRatio = LEGUP_CONFIG->getParameterInt("SDC_BACKTRACKING_BUDGET_RATIO");
      assert(budgetRatio > 0);
      int numOps = BB->size();

      while (!iterativeSchedule(budgetRatio * numOps)) {
        printf("II = %d\n", moduloScheduler.II);
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
      }
    }

    clock_t toctt = clock();
    totaltime += (double)(toctt - tictt) / CLOCKS_PER_SEC;

    lp_nvars = get_Ncolumns(sdcSolver.lp);
    lp_nconst = get_Nrows(sdcSolver.lp);

    sched_latency = 0;
    for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; i++) {
      int delay = Scheduler::getNumInstructionCycles(i);
      //std::cout << getLabel(i) << " - " << moduloScheduler.schedTime[i] << " - delay: " << delay << '\n';
      if(sched_latency < (moduloScheduler.schedTime[i]+delay)){
        sched_latency = moduloScheduler.schedTime[i]+delay;
      }
      //std::cout << getLabel(i) << " - time " << moduloScheduler.schedTime[i] << " - M: " << moduloScheduler.schedTime[i]%moduloScheduler.II << '\n';
    }
    File() << "\n final sched latency: " << sched_latency<< '\n';

    printModuleSchedulerRow();

    if (SDCdebug || ILPdebug || GAdebug || NIdebug){
      File() << "Scheduled.\n";
      File() << "MII = " << MII << "\n";
      File() << "II = " << moduloScheduler.II << "\n";
    }
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
    if (SDCdebug){
      File() << "Final Modulo Reservation Table:\n";
      printModuloReservationTable();
      File() << "\n";
    }

    moduloScheduler.gather_pipeline_stats();
    moduloScheduler.print_pipeline_table();
    moduloScheduler.set_llvm_metadata();

    moduloScheduler.totalLoopsPipelined++;
    moduloScheduler.loopsPipelined.insert(moduloScheduler.loopLabel);

    //if(SDCdebug || ILPdebug || GAdebug || NIdebug){
      std::cout << "results:" << '\n';
      for(BasicBlock::iterator i = BB->begin(), ie = BB->end(); i!=ie; ++i){
        //i->dump();
        std::cout << " - " <<getLabel(i) << "_" << startVariableIndex[dag->getInstructionNode(i)] <<"_t=" << moduloScheduler.schedTime[i] << "\n";
      }
      std::cout << "\naaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n" << '\n';
    //}

    return true;
}

//leandro - ilp verions
void ILPModuloScheduler::createILPVariables() {
    assert(BB);

    if (sdcSolver.lp != NULL)
        delete_lp(sdcSolver.lp);

    //leandro - variables start at 1 in lpSolver
    numVars = 0; // LP isn't constructed yet

    numInst = 0; // the number of LLVM instructions to be scheduled
    startVariableIndex.clear();
    endVariableIndex.clear();
    latencyInstMap.clear();

    FUinstMap.clear();
    FUlimit.clear();
    instMindex.clear();
    instRindex.clear();
    instYindex.clear();
    epsilon.clear();
    mu.clear();

    // iterate over the instructions in a BB
    for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; i++) {
        InstructionNode *iNode = dag->getInstructionNode(i);

        numInst++;
        numVars++;
        startVariableIndex[iNode] = numVars;
        //std::cout << "C" << numVars << " - startVariableIndex" << '\n';
        int delay = Scheduler::getNumInstructionCycles(i);
        if (isa<StoreInst>(i)) {
            // store, you need an extra cycle for the memory to be ready
            delay = 1;
        }

        latencyInstMap[iNode] = delay;
        //numVars += delay;
        //endVariableIndex[iNode] = numVars;

        if (ILPdebug){
          File() << "Start Index: " << startVariableIndex[iNode]
          << " delay: " << latencyInstMap[iNode]
          << " I: " << *i << "\n";
        }

        //leandro - create resource constraints variables
        std::string FuName = LEGUP_CONFIG->getOpNameFromInst(i, moduloScheduler.alloc);

        //check the constraint first
        int constraint;
        if (!LEGUP_CONFIG->getNumberOfFUsAllocated(FuName, &constraint))
          continue;

        std::map<std::string, std::vector<InstructionNode*>>::iterator it = FUinstMap.find(FuName);
        if (it != FUinstMap.end()){
          //std::cout << "input on existing: " << getLabel(iNode->getInst()) << '\n';
          it->second.push_back(iNode);
          continue;
        }

        FUlimit.insert(std::pair<std::string, int>(FuName, constraint));
        //std::cout << "input on non-existing: " << getLabel(iNode->getInst()) << '\n';
        std::vector<InstructionNode*> insts;
        insts.push_back(iNode);
        std::pair<std::string, std::vector<InstructionNode*>> newFU = std::pair<std::string, std::vector<InstructionNode*>>(FuName, insts);
        FUinstMap.insert(newFU);

        //addResourceConstraint(F, FuName, constraint);
    }

    if (ILPdebug) {
      for(auto imap :FUinstMap){
        File() << "--Fu name: " << imap.first << "  --constrained to: " << FUlimit[imap.first] << '\n';
        for(auto inst: imap.second){
          inst->getInst()->dump();
        }
      }
    }

    for(auto imap : FUinstMap){
      for(auto inst : imap.second){
        numVars++;
        instMindex[inst] = numVars;
        //std::cout << "C" << numVars << " - intMindex" << '\n';
        numVars++;
        instRindex[inst] = numVars;
        //std::cout << "C" << numVars << " - instRindex" << '\n';
        numVars++;
        instYindex[inst] = numVars;
        //std::cout << "C" << numVars << " - instYindex" << '\n';

        if (ILPdebug) {
          File() << "M = "<< instMindex[inst] << ", R = "<< instRindex[inst] << ", Y = "<< instYindex[inst] << ",  for : " << getLabel(inst->getInst()) << '\n';
          inst->getInst()->dump();
        }

        for(auto inst2 : imap.second){
          if(inst != inst2){
            std::pair<InstructionNode*, InstructionNode*> index(inst, inst2);
            numVars++;
            epsilon[index] = numVars;
            //std::cout << "C" << numVars << " - epsilon" << '\n';
            numVars++;
            mu[index] = numVars;
            //std::cout << "C" << numVars << " - mu" << '\n';
            if (ILPdebug) {
              File() << "inst 1: " << getLabel(inst->getInst()) << " -- inst2: " << getLabel(inst2->getInst()) <<
              " -- epsilon_" << getLabel(inst->getInst()) << getLabel(inst2->getInst()) << " = " << epsilon[index] <<
              " -- mu_" << getLabel(inst->getInst()) << getLabel(inst2->getInst()) << " = " << mu[index] << '\n';
            }
          }
        }
      }
    }
    //std::cout << "SDC: # of variables: " << numVars << " # of instructions: " << numInst << "\n";
    if (ILPdebug){
      File() << "SDC: # of variables: " << numVars << " # of instructions: " << numInst << "\n";
    }

    // IT IS REALLY IMPORTANT TO SET THE VARIABLES TO INTEGERS
    sdcSolver.lp = make_lp(0, numVars);
    if(solver.compare("lp_solve")==0){
      for (auto entry : startVariableIndex) {
        set_int(sdcSolver.lp, entry.second, TRUE);
      }
    }
    //some optimazions on ILP
    //set_break_at_first(sdcSolver.lp, TRUE);
    //these pre processing do not really help
    //set_presolve(sdcSolver.lp, PRESOLVE_ROWS | PRESOLVE_COLS | PRESOLVE_LINDEP | PRESOLVE_REDUCEMIP | PRESOLVE_KNAPSACK | PRESOLVE_ROWDOMINATE | PRESOLVE_COLDOMINATE | PRESOLVE_MERGEROWS, get_presolveloops(sdcSolver.lp));
}

void ILPModuloScheduler::addOverlapConstraints(int II){
  assert(BB);

  for(auto imap : FUinstMap){
    //get the name
    int a_k = FUlimit[imap.first]; //resources constraint

    for(auto i : imap.second){
      for(auto j : imap.second){
        if(i != j){
          std::pair<InstructionNode*, InstructionNode*> ij(i, j);
          std::pair<InstructionNode*, InstructionNode*> ji(j, i);

          int idx_order[2];
          int idx_theorem[3];
          int idx_glue[4];
          REAL coef_order[2];
          REAL coef_theorem[3];
          REAL coef_glue[4];

          //\epsilon_ij + \epsilon_ji <= 1
          idx_order[0] = epsilon[ij];
          idx_order[1] = epsilon[ji];
          coef_order[0] = 1.0;
          coef_order[1] = 1.0;
          add_constraintex(sdcSolver.lp, 2, coef_order, idx_order, LE, 1);
          //set_bounds(sdcSolver.lp, epsilon[ij], 0, 1);
          //set_int(sdcSolver.lp, epsilon[ij], TRUE);
          set_binary(sdcSolver.lp, epsilon[ij], TRUE);
          //set_bounds(sdcSolver.lp, epsilon[ji], 0, 1);
          //set_int(sdcSolver.lp, epsilon[ji], TRUE);
          set_binary(sdcSolver.lp, epsilon[ji], TRUE);


          //\mu_ij + \mu_ji <= 1
          idx_order[0] = mu[ij];
          idx_order[1] = mu[ji];
          add_constraintex(sdcSolver.lp, 2, coef_order, idx_order, LE, 1);
          //set_bounds(sdcSolver.lp, mu[ij], 0, 1);
          //set_int(sdcSolver.lp, mu[ij], TRUE);
          set_binary(sdcSolver.lp, mu[ij], TRUE);
          //set_bounds(sdcSolver.lp, mu[ji], 0, 1);
          //set_int(sdcSolver.lp, mu[ji], TRUE);
          set_binary(sdcSolver.lp, mu[ji], TRUE);

          idx_theorem[0] = instRindex[j];
          idx_theorem[1] = instRindex[i];
          //std::cout << "C" << instRindex[j] << "and C" <<  instRindex[i] << '\n';
          idx_theorem[2] = epsilon[ij];
          coef_theorem[0] = 1;
          coef_theorem[1] = -1;
          coef_theorem[2] = -a_k;
          // r_j - r_i - 1 - (epsilon_ij - 1)*a_k >= 0  --> r_j - r_i - epsilon_ij*a_k >= 1-a_k
          add_constraintex(sdcSolver.lp, 3, coef_theorem, idx_theorem, GE, 1-a_k);
          // r_j - r_i - epsilon_ij*ak <= 0
          add_constraintex(sdcSolver.lp, 3, coef_theorem, idx_theorem, LE, 0);

          idx_theorem[0] = instMindex[j];
          idx_theorem[1] = instMindex[i];
          idx_theorem[2] = mu[ij];
          coef_theorem[0] = 1;
          coef_theorem[1] = -1;
          coef_theorem[2] = -II;
          // m_j - m_i - 1 - (mu_ij - 1)*II >= 0  --> m_j - m_i - mu_ij*II >= 1-II
          add_constraintex(sdcSolver.lp, 3, coef_theorem, idx_theorem, GE, 1-II);
          // m_j - m_i - mu_ij*II <= 0
          add_constraintex(sdcSolver.lp, 3, coef_theorem, idx_theorem, LE, 0);

          // \epsilon_ij + \epsilon_ji + \mu_ij + \mu_ji >= 1
          idx_glue[0] = epsilon[ij];
          idx_glue[1] = epsilon[ji];
          idx_glue[2] = mu[ij];
          idx_glue[3] = mu[ji];
          coef_glue[0] = 1;
          coef_glue[1] = 1;
          coef_glue[2] = 1;
          coef_glue[3] = 1;
          add_constraintex(sdcSolver.lp, 4, coef_glue, idx_glue, GE, 1);
        }//if(i != j)
      }//for(auto j : imap.second)

      //add constraints to bound mi and ri, and constraints to link the helper vars y with t
      int idx_helper[3];
      REAL coef_helper[3];

      // t_i = y_i*II + m_i  --> t_i - II*y_i - m_i = 0
      idx_helper[0] = startVariableIndex[i];
      idx_helper[1] = instYindex[i];
      idx_helper[2] = instMindex[i];
      coef_helper[0] = 1;
      coef_helper[1] = -II;
      coef_helper[2] = -1;
      add_constraintex(sdcSolver.lp, 3, coef_helper, idx_helper, EQ, 0);

      // r_i <= a_k - 1
      // m_i <= II - 1

      if(solver.compare("gurobi")==0){
        double one[1];
        one[0] = 1;
        int r[1];
        r[0] = instRindex[i];
        add_constraintex(sdcSolver.lp, 1, one, r, GE, 0);
        add_constraintex(sdcSolver.lp, 1, one, r, LE, a_k-1);
        int m[1];
        m[0] = instMindex[i];
        add_constraintex(sdcSolver.lp, 1, one, m, GE, 0);
        add_constraintex(sdcSolver.lp, 1, one, m, LE, II-1);
      }else{
        set_bounds(sdcSolver.lp, instRindex[i], 0, a_k-1);
        set_bounds(sdcSolver.lp, instMindex[i], 0, II-1);

        set_int(sdcSolver.lp, instMindex[i], TRUE);
        set_int(sdcSolver.lp, instRindex[i], TRUE);
        set_int(sdcSolver.lp, instYindex[i], TRUE);
      }
    }//for(auto i : imap.second)
  }//for(auto imap : FUinstMap)

  if(ILPdebug){
    std::cout << "----- LP after overlap constraints ------" << '\n';
    write_LP(sdcSolver.lp, stdout);
  }
}

void ILPModuloScheduler::addILPMulticycleConstraints() {

    int col[2];
    REAL val[2];

    for (std::map<InstructionNode *, unsigned>::iterator i = startVariableIndex.begin(), e = startVariableIndex.end(); i != e; i++) {

        InstructionNode *iNode = i->first;
        // Instruction *I = iNode->getInst();
        unsigned startIndex = i->second;

        unsigned endIndex = endVariableIndex[iNode];
        unsigned latency = latencyInstMap[iNode];

        //if (startIndex == endIndex)
        if (latency > 0)
            continue; // not a multicycle instruction

        // add constraints so that the variable corresponding to each
        // cycle of a multiple cycle instruction gets assigned to
        // contiguous states.
        for (unsigned j = startIndex + 1; j <= endIndex; j++) {
            col[0] = j; // variable indicies
            col[1] = (j - 1);
            val[0] = 1.0; // variable coefficients
            val[1] = -1.0;

            if (ILPdebug) {
              File() << "Adding constraint: s(" << col[0] << ") == s(" << col[1] << ") + 1 cycle\n";
            }

            // there must be EXACTLY 1 cycle delay between variable j and j-1
            add_constraintex(sdcSolver.lp, 2, val, col, EQ, 1.0);
        }
    }
}

void ILPModuloScheduler::addILPTimingConstraints(InstructionNode *Root, InstructionNode *Curr, float PartialPathDelay, std::vector<Instruction *> path) {
    int col[2];
    REAL val[2];

    // don't constraint multi-cycle operations
    // dependency has more than 1 cycle latency, so this dependency will
    // already be in another cycle.
    if (Scheduler::getNumInstructionCycles(Root->getInst()) > 0)
        return;
    if (Scheduler::getNumInstructionCycles(Curr->getInst()) > 0)
        return;

    // Walk through the dependencies
    for (InstructionNode::iterator i = Curr->dep_begin(),
                                   e = Curr->dep_end();
         i != e; ++i) {
        // dependency from depNode -> Curr
        InstructionNode *depNode = *i;

        if (Scheduler::getNumInstructionCycles(depNode->getInst()) > 0)
            continue;

        float delay = PartialPathDelay + depNode->getDelay();
        unsigned cycleConstraint =
            ceil((float)delay / (float)clockPeriodConstraint);

        if (cycleConstraint > 0)
            cycleConstraint--;

        path.push_back(depNode->getInst());
        if (cycleConstraint > 0) {
            Instruction *I1 = depNode->getInst();
            Instruction *I2 = Root->getInst();

            if (SDCdebug) {
                File() << "Found path:\n";
                for (std::vector<Instruction *>::reverse_iterator
                         j = path.rbegin(),
                         je = path.rend();
                     j != je; ++j) {
                    Instruction *p = *j;
                    File() << "\tdelay: "
                           << dag->getInstructionNode(p)->getDelay() << " "
                           << *p << "\n";
                }
                File() << "Path delay:" << delay << "\n";
            }

            // don't add any timing constraints for nodes on recurrences,
            // we want to prioritize getting the best II over meeting
            // timing constraints...
            if (moduloScheduler.onCriticalPath(I1) &&
                moduloScheduler.onCriticalPath(I2)) {
                if (SDCdebug)
                    File() << "Skipping timing constraint due to loop "
                              "recurrence "
                           << "on this path\n";
                continue;
            }

            if (SDCdebug) {
                File() << "Adding cycle constraint: I2 >= I1 + "
                       << cycleConstraint
                       << " cycle(s). Detected delay: " << delay
                       << " (period: " << clockPeriodConstraint
                       << ") between I1 -> I2\n"
                       << "\tI1: " << *I1 << "\n"
                       << "\tI2: " << *I2 << "\n";
            }

            // if cycleConstraint == 0, we don't need to add the constraint.
            // the reason is that such constraints are ALREADY present in
            // the LP
            // formulation, as they are depedency constraints  --
            // constraints
            // that express that an operation must happen AFTER the
            // operations
            // producuing results that it depends on
            col[0] = 1 + startVariableIndex[Root];
            val[0] = 1.0;
            //col[1] = 1 + endVariableIndex[depNode];
            unsigned latency = latencyInstMap[depNode];
            col[1] = 1 + startVariableIndex[depNode];
            val[1] = -1.0;
            sdcSolver.addConstraintIncremental(sdcSolver.lp, 2, val, col,
                                               GE, cycleConstraint+latency);

        } else {
            addILPTimingConstraints(
                Root, depNode, delay,
                path); // recursive call to discover other instructions
        }
    }
}

void ILPModuloScheduler::addILPTimingConstraintsForKernel() {
    assert(BB);

    // iterate over the instructions in a BB
    for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie;
         i++) {
        InstructionNode *iNode = dag->getInstructionNode(i);
        std::vector<Instruction *> path;
        path.push_back(i);
        addILPTimingConstraints(iNode, iNode, iNode->getDelay(), path);
    }
}

void ILPModuloScheduler::addILPDependencyConstraints(InstructionNode *in) {

    int col[2];
    REAL val[2];

    // First make sure each instruction is scheduled into a cycle >= 0
    //col[0] = startVariableIndex[in];
    //val[0] = 1.0;
    //sdcSolver.addConstraintIncremental(sdcSolver.lp, 1, val, col, GE, 0.0);

    // Now handle the dependencies between instructions: producer/consumer
    // relationships
    for (InstructionNode::iterator i = in->dep_begin(), e = in->dep_end();
         i != e; ++i) {
        // Dependency: depIn -> in
        InstructionNode *depIn = *i;

        col[0] = startVariableIndex[in];
        val[0] = 1.0;
        //col[1] = endVariableIndex[depIn];
        unsigned latency = latencyInstMap[depIn];
        col[1] = startVariableIndex[depIn];
        val[1] = -1.0;

        // if chaining is permitted, then the instructions can be in the
        // SAME cycle
        // if chaining is NOT permitted, a dependent instruction is moved to
        // a  LATER cycle
        int chainingLatency = chaining ? 0.0 : 1.0;

        if (LEGUP_CONFIG->getParameterInt("SDC_ONLY_CHAIN_CRITICAL")) {
            // if (moduloScheduler.onCriticalPath(i) &&
            // moduloScheduler.onCriticalPath(j)) {
            Instruction *I = depIn->getInst();
            if (moduloScheduler.onCriticalPath(I)) {
                // errs() << "sdc critical 1: " << *I << "\n";
                chainingLatency = 0;
            } else {
                // errs() << "sdc non-critical 1: " << *I << "\n";
                chainingLatency = 1;
            }
        }

        // ensure the right ordering or instructions based on data
        // dependency:
        //      depIn -> in
        // constraint:
        //      start of 'in' - end of 'depIn' >= chaining
        // equivalent to:
        //      start of 'in' >= end of 'depIn' + chaining
        int dist =
            moduloScheduler.distance(depIn->getInst(), in->getInst());
        assert(dist == 0);

        Instruction *I1 = in->getInst();
        Instruction *I2 = depIn->getInst();
        if (ILPdebug) {
            File()
                << "Adding dependency constraint: start(I1) >= end(I2) + "
                << chainingLatency << " cycle(s). \n"
                << "start(I1) - start(I2) >= "
                << chainingLatency << " cycle(s) + " << latency << " latency(I2)\n"
                << "\tI1: " << *I1 << "\n"
                << "\tI2: " << *I2 << "\n";
        }

        add_constraintex(sdcSolver.lp, 2, val, col, GE, chainingLatency+(REAL)latency);

        // sdcSolver.addConstraintIncremental(lp, 2, val, col, GE, chaining
        // ?
        // 0.0 : 1.0);
    }

    for (InstructionNode::iterator i = in->mem_dep_begin(),
                                   e = in->mem_dep_end();
         i != e; ++i) {

        // dependency from memDepIn -> in
        InstructionNode *memDepIn = *i;

        col[0] = startVariableIndex[in];
        val[0] = 1.0;
        //col[1] = endVariableIndex[memDepIn];
        unsigned latency = latencyInstMap[memDepIn];
        col[1] = startVariableIndex[memDepIn];
        val[1] = -1.0;

        Instruction *I1 = memDepIn->getInst();
        Instruction *I2 = in->getInst();

        if (ILPdebug) {
            File() << "Adding memory dependency constraint (I1->I2): "
                   << "start(I2) >= end(I1)\n"
                   << "start(I1) - start(I2) >=  0 cycle(s) + " << latency << " latency(I2)\n"
                   << "\tI1: " << *I1 << "\n"
                   << "\tI2: " << *I2 << "\n";
        }

        // cross-iteration constraints are handled elsewhere
        // TODO: refactor cross-iteration constraints to be handled here
        assert(moduloScheduler.dependent(I1, I2));
        if (moduloScheduler.distance(I1, I2)) {
            // if(SDCdebug) File() << "Skipping due to distance = " << dist
            // << "\n";
            continue;
        }

        add_constraintex(sdcSolver.lp, 2, val, col, GE,  0.0+(REAL)latency);
    }
}

void ILPModuloScheduler::addILPDependencyConstraintsForKernel(int II) {
    assert(BB);

    // iterate over the instructions in a BB
    for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; i++) {
        addILPDependencyConstraints(dag->getInstructionNode(i));
    }

    for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie;
         ++i) {
        for (BasicBlock::iterator j = BB->begin(), je = BB->end(); j != je;
             ++j) {
            if (!moduloScheduler.dependent(i, j))
                continue;
            int dist = moduloScheduler.distance(i, j);
            if (!dist)
                continue;

            int col[2];
            REAL val[2];

            // cross iteration dependence from i -> j
            col[0] = startVariableIndex[dag->getInstructionNode(j)];
            //col[1] = endVariableIndex[dag->getInstructionNode(i)];
            unsigned latency = latencyInstMap[dag->getInstructionNode(i)];
            col[1] = startVariableIndex[dag->getInstructionNode(i)];
            val[0] = 1.0;
            val[1] = -1.0;

            // if chaining is permitted, then the instructions can be in the
            // SAME cycle
            // if chaining is NOT permitted, a dependent instruction is
            // moved to a
            // LATER cycle
            int chainingLatency = chaining ? 0.0 : 1.0;

            if (LEGUP_CONFIG->getParameterInt("SDC_ONLY_CHAIN_CRITICAL")) {
                // if (moduloScheduler.onCriticalPath(i) &&
                // moduloScheduler.onCriticalPath(j)) {
                if (moduloScheduler.onCriticalPath(i)) {
                    // errs() << "sdc critical delay=" <<
                    // moduloScheduler.delay(i) << " :" << *i << "\n";
                    chainingLatency = 0;
                    // assert(moduloScheduler.delay(i) == 0);
                } else {
                    // errs() << "sdc non-critical delay=" <<
                    // moduloScheduler.delay(i) << " :" << *i << "\n";
                    chainingLatency = 1;
                    // assert(moduloScheduler.delay(i) >= 1);
                }
            }

            if (isa<StoreInst>(i)) {
                // already looking at end state
                assert(chainingLatency == 0);
            }

            // ensure the right ordering or instructions based on
            // dependencies
            //      i -> j
            // constraint:
            //      start of 'j' - end of 'i' >= chaining - II*distance(i,
            //      j)
            // equivalent to:
            //      start of 'j' >= end of 'i' + chaining - II*distance(i,
            //      j)
            if (ILPdebug){
                File() << "Cross-iteration constraint: start of 'j' >= end of "
                       "'i' + chaining - II*distance(i, j)+latency\n";
                File() << "  chaining: " << chainingLatency << " II: " << II
                       << " distance: " << dist << " latency: " << latency << "\n";
                File() << "  i: " << *i << "\n";
                File() << "  j: " << *j << "\n";
            }

            add_constraintex(
                sdcSolver.lp, 2, val, col, GE, chainingLatency-II*dist+(REAL)latency);
        }
    }
}

void ILPModuloScheduler::initializeILP(int II) {

    printLineBreak();
    if (ILPdebug)
        File() << "initializing ILP constraints for II = " << II << "\n";

    chaining = false; // default is no chaining -- maximally pipelined
    clockPeriodConstraint = -1.0; // default is no clock period constraint
    moduloScheduler.schedTime.clear();
    createILPVariables();

    //addILPMulticycleConstraints();

    chaining = true;

    // avoid chaining for now - along with timing constraints
    // chaining = false;

    if (LEGUP_CONFIG->getParameterInt("SDC_NO_CHAINING")) {
        chaining = false; // no chaining means that the design will be
                          // pipelined as much as possible
    }

    addILPDependencyConstraintsForKernel(II);

    addOverlapConstraints(II);

    clockPeriodConstraint = 15; // 66 MHz
    if (LEGUP_CONFIG->getParameterInt("CLOCK_PERIOD")) {
        clockPeriodConstraint =
            (float)LEGUP_CONFIG->getParameterInt("CLOCK_PERIOD");
    }

    if (chaining && clockPeriodConstraint > 0) {
        if (!LEGUP_CONFIG->getParameterInt("SDC_NO_TIMING_CONSTRAINTS")) {
            //addTimingConstraintsForKernel();
        }
    }
    printLineBreak();
}

int ILPModuloScheduler::runILPLPSolver() {
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
    set_obj_fnex(sdcSolver.lp, count, variableCoefficients, variableIndices);
    set_minim(sdcSolver.lp);

    if (ILPdebug)
      write_LP(sdcSolver.lp, stderr);

    if (!ILPdebug){
      set_verbose(sdcSolver.lp, 1);
    }

    int ret;
    clock_t ticsv, tocsv;
    if(solver.compare("gurobi")==0){
      //FILE * lpfile = fopen("lpslp.lp", "w");
      //write_LP(sdcSolver.lp, lpfile);
      //fclose(lpfile);
      //if(GRBsolution != NULL){
      //  delete [] GRBsolution;
      //}

      if(NIGRBmodel != NULL){
        delete ILPGRBmodel;
        ILPGRBmodel = NULL;
      }

      char file[10] = "lp.mps";
      //FILE * file = fopen("lp.mps", "w");
      write_freemps(sdcSolver.lp, file);
      //fclose(file);
      ILPGRBmodel = new GRBModel(env, "lp.mps");
      ILPGRBmodel->set(GRB_IntParam_OutputFlag, 1);
      ILPGRBmodel->set(GRB_DoubleParam_TimeLimit, solver_time_budget*60.0);

      GRBsolution = ILPGRBmodel->getVars();
      //fix for integer variables
      for(auto imap : FUinstMap){
        for(auto i : imap.second){
          //std::cout << "M inst: C" << instMindex[i] << '\n';
          GRBsolution[instMindex[i]-1].set(GRB_CharAttr_VType, 'I');
          //std::cout << "R inst: C" << instRindex[i] << '\n';
          GRBsolution[instRindex[i]-1].set(GRB_CharAttr_VType, 'I');
          //std::cout << "Y inst: C" << instYindex[i] << '\n';
          GRBsolution[instYindex[i]-1].set(GRB_CharAttr_VType, 'I');
        }
      }
      for (auto entry : startVariableIndex) {
        //std::cout << "setting C" << entry.second << " as integer" << '\n';
        GRBsolution[entry.second-1].set(GRB_CharAttr_VType, 'I');
      }
      ticsv = clock();
      ILPGRBmodel->optimize();
      tocsv = clock();

      int optimstatus = ILPGRBmodel->get(GRB_IntAttr_Status);

      std::cout << "optimstatus: " << optimstatus << '\n';
      if (optimstatus == GRB_OPTIMAL) {
        ret = 0;
        double objval = ILPGRBmodel->get(GRB_DoubleAttr_ObjVal);
        File() << "Optimal objective: " << objval << '\n';
      }else if (optimstatus == GRB_TIME_LIMIT) {
        int nsol = ILPGRBmodel->get(GRB_IntAttr_SolCount);
        std::cout << "nsol: " << nsol << '\n';
        if(nsol >= 1){
          ret = 1;
        }else{
          ret = 2;
        }
      } else if (optimstatus == GRB_INFEASIBLE) {
        ret = 2;
        File() << "Model is infeasible" << '\n';
      }else if (optimstatus == GRB_UNBOUNDED) {
        File() << "Model is unbounded" << '\n';
        ret = 3;
      }else {
        File() << "Optimization was stopped with status = "<< optimstatus << '\n';
      }

      //ILPGRBmodel->write("grbmodel.mps");
      //assert(false);
    }else{
      float timlim = solver_time_budget*60;
      //std::cout << "time limit: " << timlim << '\n';
      set_timeout(sdcSolver.lp, timlim);
      set_break_at_first(sdcSolver.lp, TRUE);

      ticsv = clock();
      ret = solve(sdcSolver.lp);
      tocsv = clock();
    }

    solvetime += (double)(tocsv - ticsv) / CLOCKS_PER_SEC;
    nsdcs++;

    if (ILPdebug) {
        File() << "SDC solver status: " << ret << "\n";
    }

    delete[] variableCoefficients;
    delete[] variableIndices;
    return ret;
}

bool ILPModuloScheduler::solveILP(){
  int status = runILPLPSolver();
  //std::cout << "result from lpsolver: " << status << '\n';
  File() << "result from lpsolver: " << status << '\n';
  if(status == 7){
    timeout = 1;
    return false;
  }else if (status == 0 || status == 1) {
    REAL *variables = new REAL[numVars];

    //std::cout << "numVars: " << numVars << '\n';
    if(solver.compare("gurobi")==0){
      for(int i =0; i<numVars; i++){
        //std::cout << "i: " << i << " - sol: " << GRBsolution[i].get(GRB_StringAttr_VarName) << " = " << GRBsolution[i].get(GRB_DoubleAttr_X) << '\n';
        if(NIdebug){
          File() << "sol: " << GRBsolution[i].get(GRB_StringAttr_VarName) << " = " << GRBsolution[i].get(GRB_DoubleAttr_X) << '\n';
        }
        variables[i] = GRBsolution[i].get(GRB_DoubleAttr_X);
      }
    }else{
      get_variables(sdcSolver.lp, variables);
    }

    //write_LP(sdcSolver.lp, stdout);
    //std::cout << "LP with " << get_Ncolumns(sdcSolver.lp) << "  - numVars = " << numVars << '\n';

    //for(int i=0; i<numVars; i++){
    //  std::cout << "variables[" << i << "]: " << variables[i] << '\n';
    //}
    for(BasicBlock::iterator i = BB->begin(), ie = BB->end(); i!=ie; ++i){
      int idx = startVariableIndex[dag->getInstructionNode(i)];

      assert(idx <= numVars);
      //std::cout << "before assignment: " << moduloScheduler.schedTime[i] << " variables[i] = " << variables[idx-1] << '\n';
      moduloScheduler.schedTime[i] = (int)variables[idx-1];
      //if( moduloScheduler.schedTime[i] !=   variables[idx-1] ){
      //  i->dump();
      //  printf("\n%f\n", moduloScheduler.schedTime[i] - variables[idx-1]);
      //}
      //std::cout << "after assignment: " << moduloScheduler.schedTime[i] << " variables[i] = " << variables[idx-1] << '\n';

      if(ILPdebug){
        std::cout << "results:" << '\n';
        std::cout << " - " <<getLabel(i) << "_" << idx << "_" << idx-1 <<"_t=" << variables[idx-1] << "_" << moduloScheduler.schedTime[i] << "\n\n";
      }

      assert(moduloScheduler.schedTime[i] == (int)variables[idx-1] && "weird int-float casting bug");
    }

    if (ILPdebug){
      File() << "  Found solution to ILP problem  **pleonasm detected**\n";
    }
    saveSchedule(/*lpSolve=*/true);
    return true;
  }
  std::cout << "returning false" << '\n';
  return false;
}

ILPModuloScheduler::ILPModuloScheduler() {
    SDCdebug = LEGUP_CONFIG->getParameterInt("SDC_DEBUG");
    ILPdebug = LEGUP_CONFIG->getParameterInt("ILP_DEBUG");
    GAdebug = LEGUP_CONFIG->getParameterInt("GA_DEBUG");
    NIdebug = LEGUP_CONFIG->getParameterInt("NI_DEBUG");
    moduloScheduler.ranAlready = false;
    moduloScheduler.totalLoopsPipelined = 0;
    moduloScheduler.forceNoChain = false;
    sdcSolver.lp = NULL;
    sdcSolver.file = moduloScheduler.file;
    sdcSolver.SDCdebug = SDCdebug;

    std::string slv = LEGUP_CONFIG->getParameter("SOLVER");
    if(slv.compare("GUROBI")==0){
      solver = "gurobi";
    }else{
      solver = "lpsolve";
    }

    env.set(GRB_IntParam_OutputFlag, 0);
}

//---------------------------------------------------------------------
//------------------------SDCS Stuff ----------------------------------
//---------------------------------------------------------------------

void ILPModuloScheduler::computeHeight(Instruction *I) {
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

void ILPModuloScheduler::updateHeight(Instruction *I, Instruction *child) {
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

void ILPModuloScheduler::computeHeights() {
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

int ILPModuloScheduler::numIssueSlots(std::string FuName) {

    int constraint;
    if (LEGUP_CONFIG->getNumberOfFUsAllocated(FuName, &constraint)) {
        return constraint;
    } else {
        // no user-specified constraint
        return 0;
    }
}

bool ILPModuloScheduler::resourceConflict(Instruction *I, int timeSlot) {
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
int ILPModuloScheduler::findTimeSlot(Instruction *I, int minTime, int maxTime) {
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

bool ILPModuloScheduler::MRTSlotEmpty(int slot, std::string FuName) {
    for (int j = 0; j < numIssueSlots(FuName); j++) {
        if (moduloScheduler.getReservationTable(FuName, j, slot)) {
            return false;
        }
    }
    return true;
}

void ILPModuloScheduler::printModuloReservationTable() {

    for (std::set<std::string>::iterator
             i = moduloScheduler.constrainedFuNames.begin(),
             e = moduloScheduler.constrainedFuNames.end();
         i != e; ++i) {
        std::string FuName = *i;
            File() << "FuName: " << FuName << "\n";

        for (int i = 0; i < moduloScheduler.II; i++) {
                File() << "time slot: " << i;
            if (MRTSlotEmpty(i, FuName)) {
                    File() << " empty\n";
                continue;
            }
                File() << "\n";
            for (int j = 0; j < numIssueSlots(FuName); j++) {
                // TODO: LLVM 3.5 update: cannot print value if it is NULL so
                // check it here
                if (moduloScheduler.getReservationTable(FuName, j, i) == NULL) {
                        File() << "   issue slot: " << j << " instr: "
                               << "printing a <null> value"
                               << "\n";
                } else {
                        File() << "   issue slot: " << j << " instr: "
                               << *moduloScheduler.getReservationTable(
                                      FuName, j, i) << "\n";
                }
            }
        }
    }
}

void ILPModuloScheduler::unscheduleSDC(Instruction *I) {
    unschedule(I);
    sdcSolver.deleteAllInstrConstraints(I);
}

void ILPModuloScheduler::unschedule(Instruction *I) {
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
void ILPModuloScheduler::schedule(Instruction *I, int timeSlot) {

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

int ILPModuloScheduler::predecessorStart(Instruction *I, Instruction *pred) {
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
int ILPModuloScheduler::calcEarlyStart(Instruction *I) {

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

void ILPModuloScheduler::init() {
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
Instruction *ILPModuloScheduler::getHighestPriorityInst() {
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

bool ILPModuloScheduler::iterativeClassic(int budget) {
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
void ILPModuloScheduler::scheduleSDCInstruction(Instruction *I, int timeSlot) {
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

bool ILPModuloScheduler::findEmptySlot(Instruction *I, int timeSlot,
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
bool ILPModuloScheduler::schedulingConflictSDC(Instruction *I, int timeSlot) {
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


bool ILPModuloScheduler::checkFeasible() {
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
            saveSchedule2(/*lpSolve=*/false);
        }

        return isFeasible;
    } else {
        //std::cout << "here1" << '\n';
        return scheduleSDC();
        //std::cout << "here17" << '\n';
    }
}

SDCSolver::Constraints *ILPModuloScheduler::constrainSDC(Instruction *I,
                                                         int constr_type,
                                                         REAL constraint) {

    if (SDCdebug)
        File() << "Constraining " << lpConstraintStr(constr_type) << " "
               << ftostr(constraint) << ": " << *I << "\n";

    int idx[1];
    REAL val[1];

    InstructionNode *in = dag->getInstructionNode(I);

    assert(startVariableIndex.find(in) != startVariableIndex.end());

    idx[0] = 1 + startVariableIndex[in];
    val[0] = 1.0;

    SDCSolver::Constraints *C = new SDCSolver::Constraints;
    sdcSolver.addConstraintIncremental(sdcSolver.lp, 1, val, idx, constr_type,
                                       constraint, C, I);

    return C;
}

bool ILPModuloScheduler::schedulingConflictSDCIgnoreResources(Instruction *I,
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

bool ILPModuloScheduler::schedulingConflict(Instruction *I, int timeSlot) {

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

void ILPModuloScheduler::findASAPTimeForEachInst(
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

void ILPModuloScheduler::backtracking(Instruction *I, int cstep) {

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

bool ILPModuloScheduler::SDCWithBacktracking(int budget) {
    // the step the instruction would be scheduled to under ASAP scheduling
    initializeSDC(moduloScheduler.II);
    bool initialSolve = scheduleSDC();
    if(initialSolve==false) return false;
    assert(initialSolve);
    map<Instruction *, int> instStepASAP;
    findASAPTimeForEachInst(instStepASAP);

    if (LEGUP_CONFIG->getParameterInt("SDC_PRIORITY")) {
        calculatePerturbation();
    }
    //std::cout << "nsdcs: " << nsdcs << '\n';
    std::string schedulerType = LEGUP_CONFIG->getParameter("MODULO_SCHEDULER");
    if(schedulerType.compare("DEGRADE") == 0){
      nsdcs = 0;
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

Instruction *ILPModuloScheduler::unscheduledInstPriorityQueue_pop() {
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

void ILPModuloScheduler::updatePriorityQueue() {
  unscheduledInstPriorityQueue.clear();
  for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; i++) {
      unscheduledInstPriorityQueue.push(i);

  }
}

*/

//
void ILPModuloScheduler::calculatePerturbation() {
    printLineBreak();
    if (SDCdebug)
        File() << "Calculating perturbation priority function\n";
    std::map<Instruction *, int> origSdcSchedTime = sdcSchedTime;
    for (std::list<Instruction *>::iterator i = unscheduledInstPriorityQueue.begin(), e = unscheduledInstPriorityQueue.end(); i != e; ++i) {
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

bool ILPModuloScheduler::SDCGreedy() {

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

void ILPModuloScheduler::assignUnconstrainedOperations() {
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

bool ILPModuloScheduler::iterativeSchedule(int budget) {
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
    if (schedulerType == "SDC_BACKTRACKING" || schedulerType == "DEGRADE") {
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

void ILPModuloScheduler::printMinDistDot(int II) {
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
                label += ",idxor=red";
            } else if (minDist[i][j] < 0) {
                label += ",idxor=green";
            } else if (minDist[i][j] > 0) {
                label += ",idxor=orange";
            }

            graph.connectDot(out, i, j, label);
        }
    }
}

int ILPModuloScheduler::recurrenceMII_SDC(int resourceMII) {
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
            std::cout << "HERE!!!" << '\n';
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

int ILPModuloScheduler::recurrenceMII(int resourceMII) {
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

void ILPModuloScheduler::initMinDist(int II) {
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

bool ILPModuloScheduler::computeMinDist(int II) {

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

void ILPModuloScheduler::resetMinDistForDetectingRecurrences() {
    int negInf = -100000;
    for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; ++i) {
        for (BasicBlock::iterator j = BB->begin(), je = BB->end(); j != je;
             ++j) {
            minDistCopy[i][j] = negInf;
        }
    }
}

void ILPModuloScheduler::restructureLoopRecurrences(int resMII) {
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

void ILPModuloScheduler::saveMinDistForDetectingRecurrences(int recMII) {
    for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; ++i) {
        for (BasicBlock::iterator j = BB->begin(), je = BB->end(); j != je;
             ++j) {
            minDistCopy[i][j] = minDist[i][j];
        }
    }
    origRecMII = recMII;
}

int ILPModuloScheduler::resourceMII() {
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

void ILPModuloScheduler::RemapInstruction(Instruction *I,
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

void ILPModuloScheduler::printMap(map<int, ValueToValueMapTy> &valueMapIter,
                                  Value *v, int iter) {
    for (int j = 0; j <= iter; j++) {
        if (SDCdebug)
            File() << "i=" << j << ": " << *v << " -> " << *valueMapIter[j][v]
                   << "\n";
    }
}

void ILPModuloScheduler::initLoop() {

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

int ILPModuloScheduler::getInitialMII() {
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
    std::cout << "\nRecMII: " << recMII << " - resMII: " << resMII << " - MII: " << MII << '\n';
    recmii = resMII;
    return MII;
    //return resMII;
}
