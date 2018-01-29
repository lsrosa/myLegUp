#include  <cstdlib>
#include "ILPModuloScheduler.h"
#include <algorithm>    // std::random_shuffle
#include <lp_lib.h>

// using namespace polly;
using namespace llvm;
using namespace legup;


void ILPModuloScheduler::initializeMRT(MRT * mrt, int ii){
  int m_i, r_i;
  bool * availableSlots;

  std::uniform_int_distribution<int> dist_ii(0, ii-1);

  //map instructions that are not resource constrained
  for (std::map<InstructionNode *, unsigned>::iterator i = startVariableIndex.begin(), e = startVariableIndex.end(); i != e; i++) {

    //if the inst is resource constrained
    if(constrained_insts[i->first])
      continue;

    m_i = dist_ii(generator);
    (*mrt)[i->first] = std::pair<int, int>(m_i, 0);
  }

  //for each unity type
  for(auto fu : FUlimit){
    int n_instances = fu.second;
    //File() << "instances: " << n_instances << '\n';
    std::uniform_int_distribution<int> dist_n_instances(0, n_instances-1);
    //File() << "mrtsize: " << ii*n_instances << '\n';
    availableSlots =  new bool[ii*n_instances];
    std::fill_n(availableSlots, ii*n_instances, false);

    //for each instruction of this type
    //File() << "nInsts: " << FUinstMap[fu.first].size() << '\n';
    for(auto inst : FUinstMap[fu.first]){
      //find randon values that are available in the slots list above created
      do {
        //randon m_i and r_i
        m_i = dist_ii(generator);
        r_i = dist_n_instances(generator);
      } while(availableSlots[m_i*n_instances+r_i]);
      //if it was Found
      availableSlots[m_i*n_instances+r_i] = true;

      //add instruction to slot in the MRT
      //File() << "final m: " << m_i << " - r: " << r_i << "\n";
      (*mrt)[inst] = std::pair<int, int>(m_i, r_i);
    }

    delete [] availableSlots;
  }//for(auto fu : FUlimit)

}

void ILPModuloScheduler::printMRT(MRT * mrt){
  for(auto inst : *mrt){
    File() << "[C" << startVariableIndex[inst.first] << ":m=" << inst.second.first << ":r=" << inst.second.second << "] " << '\n';
  }
  return;
}

void ILPModuloScheduler::cleanGA(){
  best_ever.first = NULL;
  best_ever.second = 0;

  for(std::map<unsigned, Specimina*>::iterator sp=population.begin(), spe=population.end(); sp!=spe; ++sp){
    Specimina * specimina = sp->second;
    for(std::vector<Individual*>::iterator in=specimina->begin(), ine=specimina->end(); in!=ine; ++in){
      //MRT * mrt = (*in)->first;
      delete (*in)->first;
      delete *in;
    }
    delete specimina;
  }
  population.clear();

  for(auto ii_lp : base_lps){
    delete_lp(ii_lp.second);
  }
  base_lps.clear();

  if(GRBsolution!=NULL){
     delete GRBsolution;
     GRBsolution = NULL;
   }
  if(GAGRBmodel!=NULL){
     delete GAGRBmodel;
     GAGRBmodel = NULL;
   }
  if(relaxedGRBsolution!=NULL){
     delete relaxedGRBsolution;
     relaxedGRBsolution = NULL;
   }
  if(relaxedGAGRBmodel!=NULL){
     delete relaxedGAGRBmodel;
     relaxedGAGRBmodel = NULL;
   }
}

void ILPModuloScheduler::createGAVariables(lprec **lp) {
    assert(BB);

    //leandro - variables start at 1 in lpSolver
    numVars = 0; // LP isn't constructed yet
    numInst = 0; // the number of LLVM instructions to be scheduled
    numConstraints = 0;

    startVariableIndex.clear();
    latencyInstMap.clear();
    baseCongruenceClass.clear();
    instLinkerIndex.clear();

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
        //numVars += delay;
        //endVariableIndex[iNode] = numVars;

        if (GAdebug){
          File() << "Start Index: " << startVariableIndex[iNode]
          << " with latency: " << latencyInstMap[iNode]
          << " I: " << *i << "\n";
        }
    }

    if (GAdebug){
      File() << "SDC: # of variables: " << numVars << " # of instructions: " << numInst << "\n";
    }

    *lp = make_lp(0, numVars);
}

bool ILPModuloScheduler::checkGAmrt(MRT * mrt, int II, bool verbose){
  if(verbose) std::cout << "\n--------- checking mrt ----------" << '\n';
  //clear available slots;
  std::map<std::string, bool*> AllBusySlots;

  //instanciate new available slots
  for(auto fulim : FUlimit){
    AllBusySlots[fulim.first] = new bool[II*fulim.second];
    std::fill_n(AllBusySlots[fulim.first], II*fulim.second, false);
  }
  //if(verbose) std::cout << std::boolalpha;
  for(BasicBlock::iterator i = BB->begin(), ie = BB->end(); i!=ie; ++i){

    InstructionNode * iNode = dag->getInstructionNode(i);

    if(!constrained_insts[iNode])
      continue;

    std::string FuName = LEGUP_CONFIG->getOpNameFromInst(i, moduloScheduler.alloc);
    bool * busySlots = AllBusySlots[FuName];
    int ninstances = FUlimit[FuName];
    int M = (*mrt)[iNode].first;
    int R = (*mrt)[iNode].second;

    int instance;

    if(verbose) std::cout << "\nm,r: (" << M << ", " << instance << ") at res: " << FuName << " - to inst: " << getLabel(i) << '\n';
    if(busySlots[M*ninstances+R]){
      if(verbose) std::cout << "conflict!!!!!!!!" << '\n';
      //assert(false);
      return false;
    }else{
      if(verbose) std::cout << "busySlots: " << busySlots[M*ninstances+R] << "  -  m,r: (" << M << ", " << R << ")" << '\n';
      busySlots[M*ninstances+R] = true;
    }

    //if(verbose) std::cout << "instance: " << instance << " - ninstances: " << ninstances << '\n';
    //assert(instance < ninstances && "ahhhh, this MRT is not valid, I guess");
  }
  if(verbose) std::cout << "NO conflict! =D" << '\n';
  return true;
}

void ILPModuloScheduler::modifyCongruenceLP(MRT *mrt, lprec *lp, int II, bool relax){
  if(GAdebug){
    File() << "\n ---- editing forward dependecy constraints " << '\n';
  }

  for(auto entry : rh_m_map){
    InstructionNode* i1 = std::get<0>(entry);
    InstructionNode* i2 = std::get<1>(entry);
    int row = std::get<2>(entry);
    int m1 = (*mrt)[i1].first;
    int m2 = (*mrt)[i2].first;

    REAL oldrh = get_rh(lp, row);

    if(GAdebug){
      File() << "C" << startVariableIndex[i1] << " - C" <<  startVariableIndex[i2] << " >= " << oldrh << " - (" << m1 << " - " << m2 << ")" << '\n';
    }

    if(relax){
      set_rh(lp, row, ceil((REAL)(oldrh-(m1-m2))));
    }else{
      set_rh(lp, row, ceil((REAL)(oldrh-(m1-m2))/II));
    }
  }
  if(GAdebug){
    File() << "\n ---- editing back edge dependency constraints" << '\n';
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

    if(GAdebug){
      File() << "C" << startVariableIndex[i1] << " - " <<  "C" << startVariableIndex[i2] << " >= " << oldrh << " - (" << m1 << " - " << m2 << ") - " << II << "*" << dist << '\n';
      File() << "cl: " << chainingLatency << " - dist: " << dist << " - latency" << latency << '\n';
    }

    //old_rh = chainingLatency-II*dist+(REAL)latency
    if(relax){
      set_rh(lp, row, ceil((REAL)((chainingLatency+latency-(m1-m2))-II*dist)));
    }else{
      set_rh(lp, row, ceil((REAL)((chainingLatency+latency-(m1-m2))-II*dist)/II));
    }
  }
}

int ILPModuloScheduler::legalizeMRT(MRT *mrt, int ii){
  std::uniform_int_distribution<int> dist_ii(0, ii-1);
  std::uniform_real_distribution<float> dist_coin(0, 1);

  unsigned m, r, mtemp, rtemp;
  std::pair<unsigned, unsigned> mr, mrtemp;

  int nouts = 0;

  for(auto fu : FUlimit){
    int n_instances = fu.second;
    std::uniform_int_distribution<int> dist_n_instances(0, n_instances-1);
    std::map<std::pair<unsigned, unsigned>, InstructionNode *> slots;

    //std::cout << "\n -- calculating available slots" << '\n';
    for (unsigned i = 0; i < (unsigned)ii; i++) {
      for (unsigned j = 0; j < (unsigned)n_instances; j++) {
        //std::cout << "(m,r) = (" << i  << ", " << j << ")" << '\n';
        slots[std::pair<unsigned, unsigned>(i, j)] = NULL;
      }
    }

    //std::cout << "\nres: " << fu.first << '\n';
    for(auto i=FUinstMap[fu.first].begin(), ie=FUinstMap[fu.first].end(); i!=ie;++i){
      if(GAdebug){
        File() << "checking inst: " << getLabel((*i)->getInst()) << '\n';
      }

      m = (*mrt)[*i].first;
      r = (*mrt)[*i].second;
      mr = std::pair<unsigned, unsigned>(m, r);


      if(slots[mr] == NULL){
        slots[mr] = *i;
        //std::cout << getLabel((*i)->getInst()) << " - erasing (m1,r1): (" << m1 << ", " << r1 << ")" << '\n';
      }else{//while already taken
        if(GAdebug){
          File() << "(m,r): (" << m << ", " << r << ") not available" << '\n';
        }
        //std::cout << getLabel((*i)->getInst()) << " - (m1,r1): (" << m1 << ", " << r1 << ") not available" << '\n';
        //new random values

        bool congClassAvailable = false;

        for(int tryr=0; tryr<n_instances; tryr++){
          std::pair<unsigned, unsigned> trymr = std::pair<unsigned, unsigned>(m, tryr);
          if(slots[trymr]==NULL){
            congClassAvailable = true;
            (*mrt)[*i] = trymr;
            slots[trymr] = *i;
            break;
          }
        }

        if(!congClassAvailable){
          //InstructionNode * inst_to_change = *i;
          InstructionNode * inst_to_change;
          if(dist_coin(generator) >= 0.5){
            inst_to_change = *i;
          }else{
            std::pair<unsigned, unsigned> to_change_mr = std::pair<unsigned, unsigned>(m, dist_n_instances(generator));
            inst_to_change = slots[to_change_mr];
            slots[to_change_mr] = *i;
            (*mrt)[*i] = to_change_mr;
          }

          do{
            //randon m_i and r_i
            mtemp = dist_ii(generator);
            rtemp = dist_n_instances(generator);
            mrtemp = std::pair<unsigned, unsigned>(mtemp, rtemp);
          }while(slots[mrtemp]!=NULL);

          //std::cout << getLabel((*i)->getInst()) << " - new (m,r): (" << mtemp << ", " << rtemp << ")" << '\n';
          (*mrt)[inst_to_change] = mrtemp;
          slots[mrtemp] = inst_to_change;
          nouts++;
        }
      }
    }//for(auto i=FUinstMap[fu.first].begin()+crossOverPoint+1, ie=FUinstMap[fu.first].end(); i!=ie;++i)
  }//for(auto fu : FUlimit)
  return nouts;
}

void ILPModuloScheduler::removeBackEdgeConstraints(lprec *lp){
  if(GAdebug){
    File() << "\n ---- removing back edge constraints " << '\n';
    File() << "deleting rows:\n";
  }

  for(auto rit= back_edge_rh_m_map.rbegin(); rit!=back_edge_rh_m_map.rend(); ++rit){
    int row = std::get<2>(*rit);

    if(GAdebug){
      File() << "row: " << row << " out of: " << get_Nrows(lp) << '\n';
    }

    del_constraint(lp, row);
  }
}

bool ILPModuloScheduler::evaluationTest(Individual * individual, int II){
  MRT * mrt = individual->first;
  lprec * lp = copy_lp(base_lps[II]);
  modifyCongruenceLP(mrt, lp, II, false);
  //removeBackEdgeConstraints(nonBackEdgelp);

  int *variableIndices = new int[numInst];
  REAL *variableCoefficients = new REAL[numInst];
  std::map<InstructionNode*, int> variablesM;
  std::map<InstructionNode*, int> variablesT;
  //std::cout << "numInst:" << numInst << '\n';

  int count = 0;
  for (std::map<InstructionNode *, unsigned>::iterator i = startVariableIndex.begin(), e = startVariableIndex.end(); i != e; i++) {
    unsigned varIndex = i->second;
    assert(count < numInst);
    variableIndices[count] = varIndex;
    variableCoefficients[count] = 1.0;
    variablesM[i->first] = (*mrt)[i->first].first;
    //std::cout << "variableIndices[" << count << "]=" << varIndex << '\n';
    count++;
  }
  assert(count == numInst);
  set_obj_fnex(lp, count, variableCoefficients, variableIndices);
  set_minim(lp);

  set_verbose(lp, 1);

  int ret;
  ret = solve(lp);
  return ret == 0;
}


void ILPModuloScheduler::evaluateIndividual(Individual * individual, int II){
  MRT * mrt = individual->first;
  lprec * relaxedlp = copy_lp(base_lps[II]);
  lprec * lp = copy_lp(base_lps[II]);
  bool relax = false;

  modifyCongruenceLP(mrt, lp, II, relax);
  modifyCongruenceLP(mrt, relaxedlp, II, !relax);
  //removeBackEdgeConstraints(nonBackEdgelp);

  int *variableIndices = new int[numInst];
  REAL *variableCoefficients = new REAL[numInst];
  std::map<InstructionNode*, int> variablesM;
  std::map<InstructionNode*, int> variablesT;
  //std::cout << "numInst:" << numInst << '\n';

  int count = 0;
  for (std::map<InstructionNode *, unsigned>::iterator i = startVariableIndex.begin(), e = startVariableIndex.end(); i != e; i++) {
    unsigned varIndex = i->second;
    assert(count < numInst);
    variableIndices[count] = varIndex;
    variableCoefficients[count] = 1.0;
    variablesM[i->first] = (*mrt)[i->first].first;
    //std::cout << "variableIndices[" << count << "]=" << varIndex << '\n';
    count++;
  }
  assert(count == numInst);
  set_obj_fnex(lp, count, variableCoefficients, variableIndices);
  set_minim(lp);
  set_obj_fnex(relaxedlp, count, variableCoefficients, variableIndices);
  set_minim(relaxedlp);

  set_verbose(lp, 1);
  set_verbose(relaxedlp, 1);

  int ret, ns=0;
  clock_t ticsv, tocsv;
  double elapsedTime=0;

  if(solver.compare("gurobi")==0){
    //std::cout << "\nhere1" << '\n';
    if(GAGRBmodel != NULL){
      //std::cout << "deleting model" << '\n';
      delete GAGRBmodel;
      GAGRBmodel = NULL;
    }

    if(relaxedGAGRBmodel != NULL){
      //std::cout << "deleting relaxed model" << '\n';
      delete relaxedGAGRBmodel;
      relaxedGAGRBmodel = NULL;
    }
    //std::cout << "here2" << '\n';

    char lpfile[15] = "lp.mps";
    write_freemps(lp, lpfile);
    GAGRBmodel = new GRBModel(env, "lp.mps");
    GAGRBmodel->set(GRB_IntParam_OutputFlag, 0);
    ticsv = clock();
    GAGRBmodel->optimize();
    tocsv = clock();
    ns++;
    elapsedTime += (double)(tocsv - ticsv) / CLOCKS_PER_SEC;
    GRBsolution = GAGRBmodel->getVars();

    int optimstatus = GAGRBmodel->get(GRB_IntAttr_Status);

    //std::cout << "optimstatus: " << optimstatus << '\n';
    if (optimstatus == GRB_OPTIMAL) {
      ret = 0;
      double objval = GAGRBmodel->get(GRB_DoubleAttr_ObjVal);
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

    if(ret != 0){
      char relaxedlpfile[15] = "relaxedlp.mps";
      write_freemps(relaxedlp, relaxedlpfile);
      relaxedGAGRBmodel = new GRBModel(env, "relaxedlp.mps");
      relaxedGAGRBmodel->set(GRB_IntParam_OutputFlag, 0);
      ticsv = clock();
      relaxedGAGRBmodel->optimize();
      tocsv = clock();
      ns++;
      elapsedTime += (double)(tocsv - ticsv) / CLOCKS_PER_SEC;
      relaxedGRBsolution = relaxedGAGRBmodel->getVars();
    }
    //std::cout << "here 5\n" << '\n';

    //SDCGRBmodel->write("grbmodel.mps");
    //assert(false);
  }else{
    ticsv = clock();
    ret = solve(lp);
    tocsv = clock();
    ns++;
    elapsedTime += (double)(tocsv - ticsv) / CLOCKS_PER_SEC;

    if(ret!=0){
      ticsv = clock();
      solve(relaxedlp);
      tocsv = clock();
      ns++;
      elapsedTime += (double)(tocsv - ticsv) / CLOCKS_PER_SEC;
    }
  }

  solvetime += elapsedTime;
  // printf("%f\n", solvetime);
  nsdcs += ns;

  //std::cout << "ret: " << ret << " - nonBackEdgeret: " << nonBackEdgeret << '\n';
  lprec * lpres;
  GRBVar * GRBres;
  if (ret < 2) {
    File() << "with back edges, LP solver returned: " << ret << "\n";
    lpres = lp;
    GRBres = GRBsolution;
  }else{
    File() << "using relaxed lp, LP solver returned: " << ret << "\n";
    lpres = relaxedlp;
    relax = true;
    GRBres = relaxedGRBsolution;
  }

  REAL *solution = new REAL[numInst];

  if(solver.compare("gurobi")==0){
    for(int i=0; i<numVars; i++){
      if(NIdebug){
        File() << "sol: " << GRBres[i].get(GRB_StringAttr_VarName) << " = " << GRBres[i].get(GRB_DoubleAttr_X) << '\n';
      }
      solution[i] = GRBres[i].get(GRB_DoubleAttr_X);
    }
  }else{
    get_variables(lpres, solution);
  }

  variablesT.clear();

  unsigned max = 0;

  if(relax){
    mrt->clear();
  }
  for(BasicBlock::iterator id = BB->begin(), ide = BB->end(); id!=ide; ++id){
    InstructionNode * i = dag->getInstructionNode(id);
    int idx = startVariableIndex[i]-1;

    assert(solution[idx] == round(solution[idx]) && "Solution was supposed to be integer");
    if(relax){
      variablesT[i] = variablesM[i]+solution[idx];
    }else{
      variablesT[i] = variablesM[i]+II*solution[idx];
    }

    if(GAdebug){
      if(relax){
        File() << "inst: " << getLabel(id) << " - variableM: " << variablesM[i] << " + solution: " << solution[idx] << " = " << variablesT[i] << '\n';
      }else{
        File() << "inst: " << getLabel(id) << " - variableM: " << variablesM[i] << " + II*solution: " << II*solution[idx] << " = " << variablesT[i] << '\n';
      }
    }

    if(variablesT[i]+latencyInstMap[i] > max){
      max = variablesT[i]+latencyInstMap[i];
    }

    if(relax){
      (*mrt)[i] = std::pair<unsigned, unsigned>(variablesT[i]%II, 0);
    }
  }

  if(!relax){
    individual->second = max + II*moduloScheduler.tripCount;
    feasible = true;
  }else{
    int nouts = legalizeMRT(mrt, II);
    //std::cout << "nouts: " << nouts << '\n';
    legalizeMRT(mrt, II);
    assert(checkGAmrt(mrt, II) && "at evaluatingIndividual");
    individual->second = max + II*moduloScheduler.tripCount + II*nouts;
  }
  //int worstcase = lat_acc*moduloScheduler.tripCount;
  //if (ret >= 2) {
  //  individual->second = worstcase;
  //}else{
    //individual->second = (int)max + II*moduloScheduler.tripCount;
    //individual->second = (int)max + II*moduloScheduler.tripCount+II*nouts;
    //std::cout << "fitness: " << individual->second << '\n';
    //std::cout << "worstcase: " << worstcase << " -  loop cycles: " << individual->second << '\n';
    //assert(worstcase >= individual->second && "ahhhh I suppose the worst case should be bigger than the loop total cycles");
    //if(nouts == 0){
    //  feasible = true;
    //}
    //std::cout << "feasible" << '\n';
  //}

  delete[] variableCoefficients;
  delete[] variableIndices;
  delete_lp(lp);
  delete_lp(relaxedlp);
  return;

}

void ILPModuloScheduler::extinctDeadSpecies(int fit, unsigned *minimunII, unsigned *maximumII){
  unsigned minII = *minimunII, maxII = *maximumII;
  //fit = max(t_i = y_i*II+m_i) + II*tc

  //can't kill anyone
  if(moduloScheduler.tripCount == 0){
    return;
  }

  unsigned lim = floor((float)fit/moduloScheduler.tripCount);
  //std::cout << "fit: " << fit << " - tc: " << moduloScheduler.tripCount << "  - lim: " << lim << " - best_everII: " << best_ever.second << '\n';

  if(GAdebug){
    File() << "fit: " << fit << " - tc: " << moduloScheduler.tripCount << "  - lim: " << lim << '\n';
  }
  assert(lim >= minII);

  //nothing to do in here
  if(lim > maxII)
    return;

  for(unsigned i=lim+1 ; i<=maxII; i++){
    std::cout << "say good bye to species II=" << i << '\n';
    if(GAdebug){
      File() << "say good bye to species II=" << i << '\n';
    }

    for(auto ind : *population[i]){
      ind->first->clear();//population[i].first.erase();
    }
    population[i]->clear();
    population.erase(i);
  }
  //std::cout << "TripCount: " << moduloScheduler.tripCount << '\n';
  *minimunII = minII;
  *maximumII = lim;

  return;
}

//legacy, do not use
/*
void ILPModuloScheduler::initializePopulation(unsigned * mininumII, unsigned * maximunII){
  //just in case
  if(GAdebug){
    File() << "-------------- initializePopulation ---------- \n";
  }

  unsigned minII = *mininumII, maxII = *maximunII;
  //std::cout << "creating specimina" << '\n';
  for (unsigned ii = minII; ii <= maxII; ii++) {
    Specimina * specimina = new Specimina;
    //add fellae (yes, I'm using latin plural) to population
    population[ii] = specimina;
    //std::cout << "IIinitilized: " << ii << '\n';
  }

  //std::cout << "creating population" << '\n';
  for (std::pair<unsigned, Specimina*> pop : population) {
    Specimina * specimina = pop.second;
    int ii = pop.first;
    //std::cout << "creatin ii pop" << ii << '\n';
    for (unsigned j = 0; j < nPop; j++) {
      MRT * mrt = new MRT();
      //std::cout << "initializeMRT" << '\n';
      initializeMRT(mrt, ii);
      if(GAdebug){
          File() << "\nnew MRT: II= " << ii << '\n';
          printMRT(mrt);
      }
      //pack the new mrt
      //std::cout << "creating individual" << '\n';
      Individual * individual = new Individual(mrt, -1);
      evaluateIndividual(individual, ii);
      //std::cout << "before: ninII: " << minII << " - maxII: " << maxII << '\n';
      extinctDeadSpecies(individual->second, &minII, &maxII);
      //std::cout << "after: ninII: " << minII << " - maxII: " << maxII << '\n';
      //add the bastard to its fellows
      specimina->push_back(individual);
    }
  }

  *mininumII = minII;
  *maximunII = maxII;
  return;
}
*/
int ILPModuloScheduler::correctPathNode(MRT * mrt, int II, InstructionNode * inode, InstructionNode * backEdgeNode, int inodeM){
  int m, r, m_new, r_new;
  bool inPath = false;
  int returnRelTime = -1;

  //I need to do something if the node is already fixed
  //if(fixCongruenceCorrectedNodes[inode]){}

  //get m,r and the mrt for this inst
  m = (*mrt)[inode].first;
  r = (*mrt)[inode].second;
  int min = inodeM;

  if(inode == backEdgeNode){
    inPath = true;
    if(GAdebug){
      File() << " leads to back edge node - inst: C" << startVariableIndex[inode] << "\n";
    }
    fixCongruenceCorrectedNodes[inode] = true;
    return 0;
  }

  int longestWayToBackEdge = returnRelTime;
  InstructionNode * longestNode = NULL;

  for (InstructionNode::iterator i = inode->use_begin(), e = inode->use_end(); i != e; ++i) {
    InstructionNode * use = *i;
    if(GAdebug){
      File() << "inst:" << startVariableIndex[inode] << " is used by inst:" << startVariableIndex[use]<< "\n";
    }

    int useLentgh = correctPathNode(mrt, II, use, backEdgeNode, (inodeM+latencyInstMap[inode])%II);

    if(useLentgh >= 0){
      inPath = true;
      if(useLentgh > longestWayToBackEdge){
        longestWayToBackEdge = useLentgh;
        longestNode = use;
      }
    }

    if(GAdebug){
      File() << "inpath: " << inPath << " - useLentgh: " << useLentgh  << '\n';
      if(inPath){
        File() << "there is a path between inst:" << startVariableIndex[inode] << " and the back edge node" << '\n';
      }else{
        File() << "there is NOT a path between inst:" << startVariableIndex[inode] << " and the back edge node\n";
      }
    }
  }//for (InstructionNode::iterator i = inode->use_begin(), e = inode->use_end(); i != e; ++i)

  for (InstructionNode::iterator i = inode->mem_use_begin(), e = inode->mem_use_end(); i != e; ++i) {
    InstructionNode * use = *i;
    if(GAdebug){
      File() << "inst:" << startVariableIndex[inode] << " is used by inst:" << startVariableIndex[use]<< "\n";
    }

    int useLentgh = correctPathNode(mrt, II, use, backEdgeNode, (inodeM+latencyInstMap[inode])%II);

    if(useLentgh >= 0){
      inPath = true;
      if(useLentgh > longestWayToBackEdge){
        longestWayToBackEdge = useLentgh;
        longestNode = use;
      }
    }

    if(GAdebug){
      File() << "inpath: " << inPath << " - useLentgh: " << useLentgh << '\n';
      if(inPath){
        File() << "there is a path between inst:" << startVariableIndex[inode] << " and the back edge node" << '\n';
      }else{
        File() << "there is NOT a path between inst:" << startVariableIndex[inode] << " and the back edge node\n";
      }
    }
  }//for (InstructionNode::iterator i = inode->mem_use_begin(), e = inode->mem_use_end(); i != e; ++i)

  int dec = 0;

  if(inPath){
    if(GAdebug){
      File() << "inst: " << getLabel(inode->getInst()) << " - index: " << startVariableIndex[inode] << " - assigned m: " << (*mrt)[inode].first << " - minM: " << inodeM << '\n';
    }
    int longestm = (*mrt)[longestNode].first;
    int shouldBem = (longestm - latencyInstMap[inode])%II;

    if(m != shouldBem){
      //if the instruction is resource constrained, we need to get the free slots
      if(constrained_insts[inode]){
        std::string fuName = LEGUP_CONFIG->getOpNameFromInst(inode->getInst(), moduloScheduler.alloc);
        int n_instances = FUlimit[fuName];
        bool * bs = fixCongruenceBusySlots[fuName];

        bool found = false;
        //iterate over the MRT slots which m > mMin
        for(m_new = shouldBem; m_new >= 0; m_new--){
          for(r_new = 0; r_new < n_instances; r_new++){
            //File() << "traying new m: " << m_new << " r: " << r_new << " - busy: " << bs[m_new*n_instances+r_new] << "\n";
            if(!bs[m_new*n_instances+r_new]){
              if(GAdebug){
                File() << "changing old m: " << m << " r: " << r << " to new m: " << m_new << " r: " << r_new << "\n";
              }
              //add instruction to slot in the MRT
              (*mrt)[inode] = std::pair<int, int>(m_new, r_new);
              bs[m_new*n_instances+r_new] = true;
              bs[m*n_instances+r] = false;
              found = true;
              fixCongruenceCorrectedNodes[inode] = true;
              //exit the loop
              break;
            }
          }
          //exit this loop as well
          if(found){
            break;
          }else{
            dec++;
          }
        }
        //assert(m_new*n_instances+r_new < II*n_instances && "Seems like there was no MRT slot available, need discrd this MRT and start over");
      }else{
        if(GAdebug){
          File() << "non constrained inst - changing old m: " << m << " r: " << r << " to new m: " << min << " r: " << 0 << "\n";
        }
        (*mrt)[inode] = std::pair<int, int>(shouldBem, 0);
        fixCongruenceCorrectedNodes[inode] = true;
      }
    }
  }//if(inPath)

  return longestWayToBackEdge+latencyInstMap[inode]+dec;

}

void ILPModuloScheduler::fixCongruenceDependecies(MRT * mrt, int II){
  if(GAdebug){
    File() << "--------- Fixing broken MRT Congruence dependency -------\n";
  }
  fixCongruenceCorrectedNodes.clear();

  std::map<InstructionNode*, int> minM;
  for(auto entry : back_edge_rh_m_map){
    InstructionNode* j = std::get<0>(entry);
    InstructionNode* i = std::get<1>(entry);
    int row = std::get<2>(entry);
    int mj = (*mrt)[j].first;
    int mi = (*mrt)[i].first;

    //int chainingLatency = std::get<0>(back_edge_row_rh_map[row]);
    //int dist = std::get<1>(back_edge_row_rh_map[row]);
    int latency = std::get<2>(back_edge_row_rh_map[row]);

    int lim = (mi+latency/*-dist*II does not make any difference*/)%II;

    if(GAdebug){
        File() << "inst: " << startVariableIndex[j] << " - m: " << mj << " - depends on inst: " << startVariableIndex[i] << " - m: " << mi << " - latency: " << latency << '\n';
    }

    if(lim > minM[j]){
      minM[j] = lim;
    }
  }

  if(GAdebug){
      File() << "\n --------- \n";
  }

  for(auto entry :  fixCongruenceBusySlots){
    delete [] entry.second;
  }
  fixCongruenceBusySlots.clear();

  int m, r;

  for(auto fu : FUlimit){
    int n_instances = fu.second;
    fixCongruenceBusySlots[fu.first] =  new bool[II*n_instances];
    bool * bs = fixCongruenceBusySlots[fu.first];
    std::fill_n(bs, II*n_instances, false);

    //maps occupied spots on MRTs
    for(auto i=FUinstMap[fu.first].begin(), ie=FUinstMap[fu.first].end(); i!=ie;++i){
      m = (*mrt)[*i].first;
      r = (*mrt)[*i].second;
      //std::cout << getLabel((*i)->getInst()) << '\n';
      //std::cout << "res: " << fu.first << " - (m,r): (" << m << ", " << r << ")" << '\n';
      assert(bs[n_instances*m+r] == false && "this slot should be empty");
      bs[n_instances*m+r] = true;
    }
  }

  //now check and correct

  for(auto entry : back_edge_rh_m_map){
    InstructionNode* j = std::get<0>(entry);
    InstructionNode* i = std::get<1>(entry);
    if(GAdebug){
      File() << "----------- correcting cycle path ------ \n";
      File() << "back node C" << startVariableIndex[i] << " m: " << (*mrt)[i].first << '\n';
    }
    correctPathNode(mrt, II, j, i, minM[j]);
  }

  fixCongruenceCorrectedNodes.clear();
  return;
}

std::pair<std::map<InstructionNode *, std::pair<int, int>>*, int> *  ILPModuloScheduler::NonResourceConstrainedASAPIndividual(int II){
  MRT * mrt = new MRT();
  Individual * individual = new Individual(mrt, -1);
  lprec * lp = copy_lp(base_lps[II]);
  modifyCongruenceLP(mrt, lp, II, /*relax=*/ true);

  int *variableIndices = new int[numInst];
  REAL *variableCoefficients = new REAL[numInst];
  std::map<InstructionNode*, int> variablesM;
  std::map<InstructionNode*, int> variablesT;
  //std::cout << "numInst:" << numInst << '\n';

  int count = 0;
  for (std::map<InstructionNode *, unsigned>::iterator i = startVariableIndex.begin(), e = startVariableIndex.end(); i != e; i++) {
    unsigned varIndex = i->second;
    assert(count < numInst);
    variableIndices[count] = varIndex;
    variableCoefficients[count] = 1.0;
    variablesM[i->first] = (*mrt)[i->first].first;
    //std::cout << "variableIndices[" << count << "]=" << varIndex << '\n';
    count++;
  }
  assert(count == numInst);
  set_obj_fnex(lp, count, variableCoefficients, variableIndices);
  set_minim(lp);
  set_verbose(lp, 1);
  clock_t ticsv, tocsv;
  if(solver.compare("gurobi")==0){
    if(GAGRBmodel != NULL){
      delete GAGRBmodel;
      GAGRBmodel = NULL;
    }

    char file[10] = "lp.mps";
    write_freemps(lp, file);
    GAGRBmodel = new GRBModel(env, "lp.mps");
    GAGRBmodel->set(GRB_IntParam_OutputFlag, 0);

    ticsv = clock();
    GAGRBmodel->optimize();
    tocsv = clock();
    GRBsolution = GAGRBmodel->getVars();
  }else{
    ticsv = clock();
    solve(lp);
    tocsv = clock();
  }

  solvetime += (double)(tocsv - ticsv) / CLOCKS_PER_SEC;
  nsdcs++;

  REAL *solution = new REAL[numInst];
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

  unsigned max = 0;

  variablesT.clear();
  mrt->clear();
  delete [] variableIndices;
  delete [] variableCoefficients;

  for(BasicBlock::iterator id = BB->begin(), ide = BB->end(); id!=ide; ++id){
    InstructionNode * i = dag->getInstructionNode(id);
    int idx = startVariableIndex[i]-1;

    assert(solution[idx] == round(solution[idx]) && "Solution was supposed to be integer");
    variablesT[i] = solution[idx];

    if(variablesT[i]+latencyInstMap[i] > max){
      max = variablesT[i]+latencyInstMap[i];
    }

    (*mrt)[i] = std::pair<unsigned, unsigned>(variablesT[i]%II, 0);
  }

  int nouts = legalizeMRT(mrt, II);
  //std::cout << "nouts: " << nouts << '\n';
  legalizeMRT(mrt, II);
  assert(checkGAmrt(mrt, II) && "at asap individual");
  individual->second = max + II*moduloScheduler.tripCount + II*nouts;

  return individual;
}

void ILPModuloScheduler::initializePopulation(unsigned * IIin){
  //just in case
  if(GAdebug){
    File() << "-------------- initializePopulation ---------- \n";
  }
  unsigned ii = *IIin;

  Specimina * specimina = new Specimina;
  population[ii] = specimina;

  Individual * asapindividual;
  int insemination = ceil(0.1*nPop);
  for(int i = 0; i < insemination; i++){
    asapindividual = NonResourceConstrainedASAPIndividual(ii);
    specimina->push_back(asapindividual);
    //std::cout << "asap individual fit: " << asapindividual->second << '\n';
  }

  /*
  bool found = false;
  int cnt = 0;
  while(!found){
    cnt++;
    MRT * mrt = new MRT();
    initializeMRT(mrt, ii);
    Individual * individual = new Individual(mrt, -1);
    found = evaluationTest(individual, ii);
    std::cout << "how many tries: " << cnt << '\n';
  }
  std::cout << "how many tries: " << cnt << '\n';
  assert(false);
  */
  //std::cout << "creating population" << '\n';
  for (std::pair<unsigned, Specimina*> pop : population) {
    Specimina * specimina = pop.second;
    int ii = pop.first;
    //std::cout << "creatin ii pop" << ii << '\n';
    for (unsigned j = 0; j < nPop; j++) {
      //std::cout << "asap individual fit: " << asapindividual->second << '\n';

      MRT * mrt = new MRT();
      //std::cout << "initializeMRT" << '\n';
      initializeMRT(mrt, ii);

      //checkGAmrt(mrt, ii);
      //fixCongruenceDependecies(mrt, ii);
      if(GAdebug){
          File() << "\nnew MRT: II= " << ii << '\n';
          printMRT(mrt);
      }
      //pack the new mrt
      //std::cout << "creating individual" << '\n';
      Individual * individual = new Individual(mrt, -1);
      evaluateIndividual(individual, ii);
      specimina->push_back(individual);
    }
  }

  *IIin = ii;
  return;
}

void ILPModuloScheduler::selection(unsigned ii){
  if(GAdebug){
    File() << "--------- Selection -------\n";
  }

  //for(auto spcm : population){
    //unsigned ii = spcm.first;
    Specimina * specimina = population[ii];//spcm.second;

    if(GAdebug){
      File() << "before sorting" << '\n';
      for(auto ind : *specimina){
        File() << ind->second << " ";
      }
      File() << '\n';
    }

    //this sort individuals according to their fitness in increasing order
    //if(feasible){
      std::sort(specimina->begin(), specimina->end(), [] (Individual const * i1, Individual const * i2) { return i1->second < i2->second; });
    //}else{
    //  std::random_shuffle(specimina->begin(), specimina->end());
    //}

    if(GAdebug){
      File() << "after sorting" << '\n';
      for(auto ind : *specimina){
        File() << ind->second << " ";
      }
      File() << '\n';
    }

    //dirty elitst selection - basically erease the individuals with wors fitness
    specimina->resize(nPop);

    if(GAdebug){
      File() << "after selection" << '\n';
      for(auto ind : *specimina){
        File() << ind->second << " ";
      }
      File() << '\n';
    }

    //std::cout << "after selection" << '\n';
    //for(auto ind : *specimina){
    //  std::cout << ind->second << " ";
    //}
    //std::cout << '\n';

    if(best_ever.first == (Individual*)NULL || (*(specimina->begin()))->second < best_ever.first->second){
      best_ever.first = (*(specimina->begin()));
      best_ever.second = ii;
    }

  //}
  return;
}

void ILPModuloScheduler::mutation(MRT * mrt, unsigned II){
  std::uniform_int_distribution<int> dist_percent(0, 99);
  std::uniform_int_distribution<int> dist_ii(0, II-1);

  //int m_i, r_i;
  bool * busySlots;

  //for each unity type
  for(auto fu : FUlimit){
    int n_instances = fu.second;
    //std::cout << "n_instances: " << n_instances << '\n';

    //quite if the MRT is full
    if(FUinstMap[fu.first].size() == II*n_instances){
      continue;
    }

    std::uniform_int_distribution<int> dist_n_instances(0, n_instances-1);
    busySlots = new bool[II*n_instances];
    std::fill_n(busySlots, II*n_instances, false);
    int m, r, m_new, r_new;

    //maps occupied spots at MRT
    for(auto i=FUinstMap[fu.first].begin(), ie=FUinstMap[fu.first].end(); i!=ie;++i){
      m = (*mrt)[*i].first;
      r = (*mrt)[*i].second;
      //std::cout << getLabel((*i)->getInst()) << '\n';
      busySlots[n_instances*m+r] = true;
    }
    //std::cout << busySlots << '\n';
    //now mutate
    for(auto i=FUinstMap[fu.first].begin(), ie=FUinstMap[fu.first].end(); i!=ie;++i){
      m = (*mrt)[*i].first;
      r = (*mrt)[*i].second;
      if((unsigned)dist_percent(generator) < mutationProb){
        if(GAdebug){
          File() << "--------- Mutation! -------\n";
        }
        do{
          //randon m_i and r_i
          m_new = dist_ii(generator);
          r_new = dist_n_instances(generator);
        } while(busySlots[m_new*n_instances+r_new]);
        //new m and r different from the old ones were found

        if(GAdebug){
          File() << "changing old m: " << m << " r: " << r << " to new m: " << m_new << " r: " << r_new << "\n";
        }

        busySlots[m_new*n_instances+r_new] = true;
        //note that the old values should be released
        busySlots[m*n_instances+r] = false;

        //add instruction to slot in the MRT
        (*mrt)[*i] = std::pair<int, int>(m_new, r_new);
      }
    }

    delete [] busySlots;
  }
  return;
}

//legacy do not use
/*
void ILPModuloScheduler::calculateNewPopulation(unsigned * minimunII, unsigned * maximumII){
  unsigned minII = *minimunII, maxII = *maximumII;
  std::uniform_int_distribution<int> dist_nPop(0, nPop-1);

  if(GAdebug){
    File() << "--------- Calculate new pop -------\n";
  }

  //selection();
  for(auto spcm : population){
    unsigned ii = spcm.first;
    Specimina * specimina = spcm.second;
    File() << "specimina: " << ii << '\n';
    Individual *parent1, *parent2, *cub1, *cub2;
    MRT * mrt_p1, * mrt_p2, * mrt_cub1, * mrt_cub2;
    unsigned p1, p2;
    //yes, they are baby lions
    for(unsigned cub=0; cub<offspringSize; cub+=2){
      //get parents
      //TODO change this dist and sort pop to favor individuals with better fit
      p1 = dist_nPop(generator);
      do {
        p2 =  dist_nPop(generator);
      } while(p2 == p1);

      parent1 = specimina->at(p1);
      parent2 = specimina->at(p2);

      if(GAdebug){
        File() << "\n\nparent1: " << p1 <<" fit: " << parent1->second << " - parent2 " << p2 << " fit: " << parent2->second << '\n';
      }

      //offspringGeneration();
      mrt_p1 = parent1->first;
      mrt_p2 = parent2->first;
      mrt_cub1 = new MRT(*mrt_p1);
      mrt_cub2 = new MRT(*mrt_p2);

      //std::map<std::pair<unsigned, unsigned>, InstructionNode*> *availableSlots1, *availableSlots2, *busySlots1, *busySlots2;
      std::uniform_int_distribution<int> dist_ii(0, ii-1);

      //for each unity type
      for(auto fu : FUlimit){
        int n_instances = fu.second;
        //std::uniform_int_distribution<int> dist_n_instances(0, n_instances-1);

        std::map<std::pair<unsigned, unsigned>, InstructionNode*> availableSlots1;
        std::map<std::pair<unsigned, unsigned>, InstructionNode*> availableSlots2;
        std::map<std::pair<unsigned, unsigned>, InstructionNode*> busySlots1;
        std::map<std::pair<unsigned, unsigned>, InstructionNode*> busySlots2;
        //std::fill_n(availableSlots1, ii*n_instances, (InstructionNode*)NULL);
        //std::fill_n(availableSlots2, ii*n_instances, (InstructionNode*)NULL);

        for (unsigned i = 0; i < ii; i++) {
          for (unsigned j = 0; j < (unsigned)n_instances; j++) {
            availableSlots1[std::pair<unsigned, unsigned>(i, j)] = NULL;
            availableSlots2[std::pair<unsigned, unsigned>(i, j)] = NULL;
          }
        }
        //each unit type has FUinstMap[fu.first].size() instructions, we want that the cross point select at least one instruction (either the first or the last) to be in each cub. The if handles if there is only one inst
        unsigned crossOverPoint;
        if(FUinstMap[fu.first].size() <= 2)//two insts or less
          crossOverPoint = 0;
        else{
          std::uniform_int_distribution<int> dist_cross_point(0, FUinstMap[fu.first].size()-2);
          crossOverPoint = dist_cross_point(generator);
        }

        if(GAdebug){
          File() << "unit: " << fu.first << " - instances: " << n_instances << '\n';
          for(auto i : FUinstMap[fu.first]){
            File() << getLabel(i->getInst()) << '\n';
          }
          File() << "crossOverPoint: " << crossOverPoint << '\n';
        }

        int m, r, mtemp, rtemp, m1, r1, m2, r2;

        //saves the instructions in the first part of the chromosome with the imformation
        for(auto i=FUinstMap[fu.first].begin(), ie=FUinstMap[fu.first].begin()+crossOverPoint+1; i!=ie;++i){
          m = (*mrt_cub1)[*i].first;
          r = (*mrt_cub1)[*i].second;
          availableSlots1.erase(std::pair<unsigned, unsigned>(m, r));
          busySlots1[std::pair<unsigned, unsigned>(m, r)] = *i;
          m = (*mrt_cub2)[*i].first;
          r = (*mrt_cub2)[*i].second;
          availableSlots2.erase(std::pair<unsigned, unsigned>(m, r));
          busySlots2[std::pair<unsigned, unsigned>(m, r)] = *i;
          if(GAdebug){
            File() << "copying " << getLabel((*i)->getInst()) << '\n';
          }
        }

        //instructions on the second part of the chromosome
        bool conflict;
        for(auto i=FUinstMap[fu.first].begin()+crossOverPoint+1, ie=FUinstMap[fu.first].end(); i!=ie;++i){
          if(GAdebug){
            File() << "changing " << getLabel((*i)->getInst()) << '\n';
          }

          m1 = (*mrt_cub1)[*i].first;
          r1 = (*mrt_cub1)[*i].second;
          m2 = (*mrt_cub2)[*i].first;
          r2 = (*mrt_cub2)[*i].second;

          //--------------------------------------------------------------------
          //--------------------consistency fix on cub1-------------------------
          //--------------------------------------------------------------------
          if(GAdebug){
            File() << "consistency check on cub 1" << '\n';
          }
          mtemp = m2;
          rtemp = r2;

          conflict = false;
          if(busySlots1[std::pair<unsigned, unsigned>(mtemp, rtemp)] != NULL){//while already taken
            //new random values
            if(GAdebug){
              File() << "conflict with: " << getLabel(availableSlots1[std::pair<unsigned, unsigned>(m2,r2)]->getInst()) << '\n';
            }
            std::uniform_int_distribution<int> dist_new_m_r(0, availableSlots1.size()-1);
            int pos = dist_new_m_r(generator);
            std::map<std::pair<unsigned, unsigned>, InstructionNode*>::iterator new_mr = availableSlots1.begin();
            std::advance(new_mr, pos);

            mtemp = new_mr->first.first;
            rtemp = new_mr->first.second;
            conflict = true;
          }

          //at this point we have an m and r that are cosistent

          //if there were no conflict we just need to add *i as m2,r2 = mtemp,rtemp
          if(!conflict || parent1->second <= parent2->second){
            //the instruction in the lower part gets a new random value instead of cub2 values
            (*mrt_cub1)[*i] = std::pair<int, int>(mtemp, rtemp);
            busySlots1[pair<unsigned, unsigned>(mtemp, rtemp)] = *i;
            availableSlots1.erase(pair<unsigned, unsigned>(mtemp, rtemp));

            if(GAdebug){
              if(conflict)
                File() << getLabel((*i)->getInst()) << "gets random m: " << mtemp << " r: " << rtemp << '\n';
              else
                File() << getLabel((*i)->getInst()) << "gets cub2 m: " << mtemp << " r: " << rtemp << '\n';
            }
          }else{
            //the instruction in the upper part gets random values
            InstructionNode * in1 = busySlots1[pair<unsigned, unsigned>(m2, r2)];
            (*mrt_cub1)[in1] = std::pair<int, int>(mtemp, rtemp);
            busySlots1[pair<unsigned, unsigned>(mtemp, rtemp)] = in1;
            availableSlots1.erase(pair<unsigned, unsigned>(mtemp, rtemp));

            if(GAdebug){
              File() << getLabel(in1->getInst()) << "gets random m: " << mtemp << " r: " << rtemp << '\n';
            }

            //the instruction in the lower parte get the values of cub 2
            (*mrt_cub1)[*i] = std::pair<int, int>(m2, r2);
            busySlots1[pair<unsigned, unsigned>(m2,r2)] = *i;
            if(GAdebug){
              File() << getLabel((*i)->getInst()) << "gets cub2 m: " << m2 << " r: " << r2 << '\n';
            }
          }

          //--------------------------------------------------------------------
          //--------------------consistency fix on cub2-------------------------
          //--------------------------------------------------------------------
          if(GAdebug){
            File() << "consistency check on cub 2" << '\n';
          }
          mtemp = m1;
          rtemp = r1;

          conflict = false;
          if(availableSlots2[pair<unsigned, unsigned>(mtemp, rtemp)] != NULL){//while already taken
            //new random values
            if(GAdebug){
              File() << "conflict with: " << getLabel(availableSlots2[pair<unsigned, unsigned>(m1, r2)]->getInst()) << '\n';
            }
            std::uniform_int_distribution<int> dist_new_m_r(0, availableSlots2.size()-1);
            int pos = dist_new_m_r(generator);
            std::map<std::pair<unsigned, unsigned>, InstructionNode*>::iterator new_mr = availableSlots2.begin();
            mtemp = dist_ii(generator);
            std::advance(new_mr, pos);

            mtemp = new_mr->first.first;
            rtemp = new_mr->first.second;
            conflict = true;
          }
          //at this point we have an m and r that are cosistent

          //if there were no conflict we just need to add *i as m2,r2 = mtemp,rtemp
          if(!conflict || parent2->second <= parent1->second){
            //the instruction in the lower part gets a new random value instead of cub2 values
            (*mrt_cub2)[*i] = std::pair<int, int>(mtemp, rtemp);
            busySlots2[pair<unsigned, unsigned>(mtemp, rtemp)] = *i;
            availableSlots2.erase(pair<unsigned, unsigned>(mtemp, rtemp));

            if(GAdebug){
              if(conflict)
              File() << getLabel((*i)->getInst()) << "gets random m: " << mtemp << " r: " << rtemp << '\n';
              else
              File() << getLabel((*i)->getInst()) << "gets cub1 m: " << mtemp << " r: " << rtemp << '\n';
            }
          }else{
            //the instruction in the upper part gets random values
            InstructionNode * in2 = busySlots2[pair<unsigned, unsigned>(m1, r1)];
            (*mrt_cub2)[in2] = std::pair<int, int>(mtemp, rtemp);
            busySlots2[pair<unsigned, unsigned>(mtemp, rtemp)] = in2;
            availableSlots2.erase(pair<unsigned, unsigned>(mtemp, rtemp));

            if(GAdebug){
              File() << getLabel(in2->getInst()) << "gets random m: " << mtemp << " r: " << rtemp << '\n';
            }

            //the instruction in the lower parte get the values of cub 2
            (*mrt_cub2)[*i] = std::pair<int, int>(m1, r1);
            busySlots2[pair<unsigned, unsigned>(m1,r1)] = *i;
            if(GAdebug){
              File() << getLabel((*i)->getInst()) << "gets cub1 m: " << m1 << " r: " << r1 << '\n';
            }
          }
        }//for(auto i=FUinstMap[fu.first].begin()+crossOverPoint+1, ie=FUinstMap[fu.first].end(); i!=ie;++i)
      }//for(auto fu : FUlimit)

      //TODO insert mutation mutation();
      //std::cout << "muatation1" << '\n';
      mutation(mrt_cub1, ii);
      //std::cout << "muatation2" << '\n';
      mutation(mrt_cub2, ii);
      cub1 = new Individual(mrt_cub1, -1);
      cub2 = new Individual(mrt_cub2, -1);
      //std::cout << "evaluate1" << '\n';
      evaluateIndividual(cub1, ii);
      extinctDeadSpecies(cub1->second, &minII, &maxII);
      //std::cout << "evaluate2" << '\n';
      evaluateIndividual(cub2, ii);
      extinctDeadSpecies(cub2->second, &minII, &maxII);
      specimina->push_back(cub1);
      specimina->push_back(cub2);

      if(GAdebug){
        File() << "\nmrt parent1 - fit: "<< parent1->second << ":\n";
        printMRT(mrt_p1);
        File() << "\nmrt parent2 - fit: "<< parent2->second << ":\n";
        printMRT(mrt_p2);
        File() << "\nmrt cub1 - fit: "<< cub1->second << ":\n";
        printMRT(mrt_cub1);
        File() << "\nmrt cub2 - fit: "<< cub2->second << ":\n";
        printMRT(mrt_cub2);
      }

    }//for(unsigned cub=0; cub<offspringSize; cub+=2)
  }//for(auto spcm : population)

  *minimunII = minII;
  *maximumII = maxII;

  return;
}
*/

//legacy
/*
void ILPModuloScheduler::calculateNewPopulation(unsigned * IIin){
  unsigned ii = *IIin;
  std::uniform_int_distribution<int> dist_nPop(0, nPop-1);

  if(GAdebug){
    File() << "--------- Calculate new pop -------\n";
  }

  //selection();
  for(auto spcm : population){
    assert(ii == spcm.first);
    Specimina * specimina = spcm.second;
    File() << "specimina: " << ii << '\n';
    Individual *parent1, *parent2, *cub1, *cub2;
    MRT * mrt_p1, * mrt_p2, * mrt_cub1, * mrt_cub2;
    unsigned p1, p2;
    //yes, they are baby lions
    for(unsigned cub=0; cub<offspringSize; cub+=2){
      //get parents
      //TODO change this dist and sort pop to favor individuals with better fit
      p1 = dist_nPop(generator);
      do {
        p2 =  dist_nPop(generator);
      } while(p2 == p1);

      parent1 = specimina->at(p1);
      parent2 = specimina->at(p2);

      if(GAdebug){
        File() << "\n\nparent1: " << p1 <<" fit: " << parent1->second << " - parent2 " << p2 << " fit: " << parent2->second << '\n';
      }

      //offspringGeneration();
      mrt_p1 = parent1->first;
      mrt_p2 = parent2->first;
      mrt_cub1 = new MRT(*mrt_p1);
      mrt_cub2 = new MRT(*mrt_p2);

      //std::map<std::pair<unsigned, unsigned>, InstructionNode*> *availableSlots1, *availableSlots2, *busySlots1, *busySlots2;
      std::uniform_int_distribution<int> dist_ii(0, ii-1);

      //for each unity type
      for(auto fu : FUlimit){
        int n_instances = fu.second;
        //std::uniform_int_distribution<int> dist_n_instances(0, n_instances-1);

        std::map<std::pair<unsigned, unsigned>, InstructionNode*> availableSlots1;
        std::map<std::pair<unsigned, unsigned>, InstructionNode*> availableSlots2;
        std::map<std::pair<unsigned, unsigned>, InstructionNode*> busySlots1;
        std::map<std::pair<unsigned, unsigned>, InstructionNode*> busySlots2;
        //std::fill_n(availableSlots1, ii*n_instances, (InstructionNode*)NULL);
        //std::fill_n(availableSlots2, ii*n_instances, (InstructionNode*)NULL);

        for (unsigned i = 0; i < ii; i++) {
          for (unsigned j = 0; j < (unsigned)n_instances; j++) {
            availableSlots1[std::pair<unsigned, unsigned>(i, j)] = NULL;
            availableSlots2[std::pair<unsigned, unsigned>(i, j)] = NULL;
          }
        }
        //each unit type has FUinstMap[fu.first].size() instructions, we want that the cross point select at least one instruction (either the first or the last) to be in each cub. The if handles if there is only one inst
        unsigned crossOverPoint;
        if(FUinstMap[fu.first].size() <= 2)//two insts or less
          crossOverPoint = 0;
        else{
          std::uniform_int_distribution<int> dist_cross_point(0, FUinstMap[fu.first].size()-2);
          crossOverPoint = dist_cross_point(generator);
        }

        if(GAdebug){
          File() << "unit: " << fu.first << " - instances: " << n_instances << '\n';
          for(auto i : FUinstMap[fu.first]){
            File() << getLabel(i->getInst()) << '\n';
          }
          File() << "crossOverPoint: " << crossOverPoint << '\n';
        }

        int m, r, mtemp, rtemp, m1, r1, m2, r2;

        //saves the instructions in the first part of the chromosome with the imformation
        for(auto i=FUinstMap[fu.first].begin(), ie=FUinstMap[fu.first].begin()+crossOverPoint+1; i!=ie;++i){
          m = (*mrt_cub1)[*i].first;
          r = (*mrt_cub1)[*i].second;
          availableSlots1.erase(std::pair<unsigned, unsigned>(m, r));
          busySlots1[std::pair<unsigned, unsigned>(m, r)] = *i;
          m = (*mrt_cub2)[*i].first;
          r = (*mrt_cub2)[*i].second;
          availableSlots2.erase(std::pair<unsigned, unsigned>(m, r));
          busySlots2[std::pair<unsigned, unsigned>(m, r)] = *i;
          if(GAdebug){
            File() << "copying " << getLabel((*i)->getInst()) << '\n';
          }
        }

        //instructions on the second part of the chromosome
        bool conflict;
        for(auto i=FUinstMap[fu.first].begin()+crossOverPoint+1, ie=FUinstMap[fu.first].end(); i!=ie;++i){
          if(GAdebug){
            File() << "changing " << getLabel((*i)->getInst()) << '\n';
          }

          m1 = (*mrt_cub1)[*i].first;
          r1 = (*mrt_cub1)[*i].second;
          m2 = (*mrt_cub2)[*i].first;
          r2 = (*mrt_cub2)[*i].second;

          //--------------------------------------------------------------------
          //--------------------consistency fix on cub1-------------------------
          //--------------------------------------------------------------------
          if(GAdebug){
            File() << "consistency check on cub 1" << '\n';
          }
          mtemp = m2;
          rtemp = r2;

          conflict = false;
          if(busySlots1[std::pair<unsigned, unsigned>(mtemp, rtemp)] != NULL){//while already taken
            //new random values
            if(GAdebug){
              File() << "conflict with: " << getLabel(availableSlots1[std::pair<unsigned, unsigned>(m2,r2)]->getInst()) << '\n';
            }
            std::uniform_int_distribution<int> dist_new_m_r(0, availableSlots1.size()-1);
            int pos = dist_new_m_r(generator);
            std::map<std::pair<unsigned, unsigned>, InstructionNode*>::iterator new_mr = availableSlots1.begin();
            std::advance(new_mr, pos);

            mtemp = new_mr->first.first;
            rtemp = new_mr->first.second;
            conflict = true;
          }

          //at this point we have an m and r that are cosistent

          //if there were no conflict we just need to add *i as m2,r2 = mtemp,rtemp
          if(!conflict || parent1->second <= parent2->second){
            //the instruction in the lower part gets a new random value instead of cub2 values
            (*mrt_cub1)[*i] = std::pair<int, int>(mtemp, rtemp);
            busySlots1[pair<unsigned, unsigned>(mtemp, rtemp)] = *i;
            availableSlots1.erase(pair<unsigned, unsigned>(mtemp, rtemp));

            if(GAdebug){
              if(conflict)
                File() << getLabel((*i)->getInst()) << "gets random m: " << mtemp << " r: " << rtemp << '\n';
              else
                File() << getLabel((*i)->getInst()) << "gets cub2 m: " << mtemp << " r: " << rtemp << '\n';
            }
          }else{
            //the instruction in the upper part gets random values
            InstructionNode * in1 = busySlots1[pair<unsigned, unsigned>(m2, r2)];
            (*mrt_cub1)[in1] = std::pair<int, int>(mtemp, rtemp);
            busySlots1[pair<unsigned, unsigned>(mtemp, rtemp)] = in1;
            availableSlots1.erase(pair<unsigned, unsigned>(mtemp, rtemp));

            if(GAdebug){
              File() << getLabel(in1->getInst()) << "gets random m: " << mtemp << " r: " << rtemp << '\n';
            }

            //the instruction in the lower parte get the values of cub 2
            (*mrt_cub1)[*i] = std::pair<int, int>(m2, r2);
            busySlots1[pair<unsigned, unsigned>(m2,r2)] = *i;
            if(GAdebug){
              File() << getLabel((*i)->getInst()) << "gets cub2 m: " << m2 << " r: " << r2 << '\n';
            }
          }

          //--------------------------------------------------------------------
          //--------------------consistency fix on cub2-------------------------
          //--------------------------------------------------------------------
          if(GAdebug){
            File() << "consistency check on cub 2" << '\n';
          }
          mtemp = m1;
          rtemp = r1;

          conflict = false;
          if(availableSlots2[pair<unsigned, unsigned>(mtemp, rtemp)] != NULL){//while already taken
            //new random values
            if(GAdebug){
              File() << "conflict with: " << getLabel(availableSlots2[pair<unsigned, unsigned>(m1, r2)]->getInst()) << '\n';
            }
            std::uniform_int_distribution<int> dist_new_m_r(0, availableSlots2.size()-1);
            int pos = dist_new_m_r(generator);
            std::map<std::pair<unsigned, unsigned>, InstructionNode*>::iterator new_mr = availableSlots2.begin();
            mtemp = dist_ii(generator);
            std::advance(new_mr, pos);

            mtemp = new_mr->first.first;
            rtemp = new_mr->first.second;
            conflict = true;
          }
          //at this point we have an m and r that are cosistent

          //if there were no conflict we just need to add *i as m2,r2 = mtemp,rtemp
          if(!conflict || parent2->second <= parent1->second){
            //the instruction in the lower part gets a new random value instead of cub2 values
            (*mrt_cub2)[*i] = std::pair<int, int>(mtemp, rtemp);
            busySlots2[pair<unsigned, unsigned>(mtemp, rtemp)] = *i;
            availableSlots2.erase(pair<unsigned, unsigned>(mtemp, rtemp));

            if(GAdebug){
              if(conflict)
              File() << getLabel((*i)->getInst()) << "gets random m: " << mtemp << " r: " << rtemp << '\n';
              else
              File() << getLabel((*i)->getInst()) << "gets cub1 m: " << mtemp << " r: " << rtemp << '\n';
            }
          }else{
            //the instruction in the upper part gets random values
            InstructionNode * in2 = busySlots2[pair<unsigned, unsigned>(m1, r1)];
            (*mrt_cub2)[in2] = std::pair<int, int>(mtemp, rtemp);
            busySlots2[pair<unsigned, unsigned>(mtemp, rtemp)] = in2;
            availableSlots2.erase(pair<unsigned, unsigned>(mtemp, rtemp));

            if(GAdebug){
              File() << getLabel(in2->getInst()) << "gets random m: " << mtemp << " r: " << rtemp << '\n';
            }

            //the instruction in the lower parte get the values of cub 2
            (*mrt_cub2)[*i] = std::pair<int, int>(m1, r1);
            busySlots2[pair<unsigned, unsigned>(m1,r1)] = *i;
            if(GAdebug){
              File() << getLabel((*i)->getInst()) << "gets cub1 m: " << m1 << " r: " << r1 << '\n';
            }
          }
        }//for(auto i=FUinstMap[fu.first].begin()+crossOverPoint+1, ie=FUinstMap[fu.first].end(); i!=ie;++i)
      }//for(auto fu : FUlimit)

      //TODO insert mutation mutation();
      //std::cout << "muatation1" << '\n';
      mutation(mrt_cub1, ii);
      //std::cout << "muatation2" << '\n';
      mutation(mrt_cub2, ii);
      cub1 = new Individual(mrt_cub1, -1);
      cub2 = new Individual(mrt_cub2, -1);
      //std::cout << "evaluate1" << '\n';
      evaluateIndividual(cub1, ii);
      //std::cout << "evaluate2" << '\n';
      evaluateIndividual(cub2, ii);
      specimina->push_back(cub1);
      specimina->push_back(cub2);

      if(GAdebug){
        File() << "\nmrt parent1 - fit: "<< parent1->second << ":\n";
        printMRT(mrt_p1);
        File() << "\nmrt parent2 - fit: "<< parent2->second << ":\n";
        printMRT(mrt_p2);
        File() << "\nmrt cub1 - fit: "<< cub1->second << ":\n";
        printMRT(mrt_cub1);
        File() << "\nmrt cub2 - fit: "<< cub2->second << ":\n";
        printMRT(mrt_cub2);
      }

    }//for(unsigned cub=0; cub<offspringSize; cub+=2)
  }//for(auto spcm : population)

  *IIin = ii;
  return;
}
*/
void ILPModuloScheduler::calculateNewPopulation(unsigned * IIin){
  unsigned ii = *IIin;
  std::uniform_int_distribution<int> dist_nPop(0, nPop-1);

  if(GAdebug){
    File() << "--------- Calculate new pop -------\n";
  }

  //selection();
    Specimina * specimina = population[ii];
    File() << "specimina: " << ii << '\n';
    Individual *parent1, *parent2, *cub1, *cub2;
    MRT * mrt_p1, * mrt_p2, * mrt_cub1, * mrt_cub2;
    //unsigned m1, r1, m2, r2, mtemp, rtemp;
    std::pair<unsigned, unsigned> mrtemp, mr1, mr2;

    unsigned p1, p2;
    //yes, they are baby lions
    for(unsigned cub=0; cub<offspringSize; cub+=2){
      //get parents
      //TODO change this dist and sort pop to favor individuals with better fit
      p1 = dist_nPop(generator);
      do {
        p2 =  dist_nPop(generator);
      } while(p2 == p1);

      parent1 = specimina->at(p1);
      parent2 = specimina->at(p2);

      if(GAdebug){
        File() << "\n\nparent1: " << p1 <<" fit: " << parent1->second << " - parent2 " << p2 << " fit: " << parent2->second << '\n';
      }

      //offspringGeneration();
      mrt_p1 = parent1->first;
      mrt_p2 = parent2->first;

      //copy the whole MRT
      mrt_cub1 = new MRT;
      mrt_cub2 = new MRT;

      //std::map<std::pair<unsigned, unsigned>, InstructionNode*> *availableSlots1, *availableSlots2, *busySlots1, *busySlots2;
      std::uniform_int_distribution<int> dist_cross_point(0, mrt_p1->size()-1-1);
      int crossOverPoint = dist_cross_point(generator);
      //int count = 0;

      if(GAdebug){
        File() << "size: " << mrt_p1->size() << " - crossOverPoint: " << crossOverPoint << " - numInst: " << numInst << '\n';
      }
      //cross over
      MRT::iterator cr1 = mrt_p1->begin();
      std::advance(cr1, crossOverPoint);
      MRT::iterator cr2 = mrt_p2->begin();
      std::advance(cr2, crossOverPoint);

      //std::cout << "copping c1" << '\n';
      mrt_cub1->insert(mrt_p1->begin(), cr1);
      //std::cout << "copping c2" << '\n';
      mrt_cub2->insert(mrt_p2->begin(), cr2);

      //std::cout << "copping c12" << '\n';
      mrt_cub1->insert(cr2, mrt_p2->end());
      //std::cout << "copping c22" << '\n';
      mrt_cub2->insert(cr1, mrt_p1->end());

      /*
      std::cout << "comparing MRT 1:" << '\n';
      std::deque<int> svi;
      for(auto entry : *mrt_cub1){
        svi.push_back(startVariableIndex[entry.first]);
        std::cout << "inst: C" << startVariableIndex[entry.first] << " - (m,r): (" << entry.second.first << ", " << entry.second.second << ")" << '\n';
      }

      std::cout << "comparing MRT 2:" << '\n';
      for(auto entry : *mrt_cub2){
        std::cout << "inst: C" << startVariableIndex[entry.first] << " - (m,r): (" << entry.second.first << ", " << entry.second.second << ")" << '\n';
        assert(startVariableIndex[entry.first] == svi.front() && "common!! they should be in the same order");
        svi.pop_front();
      }
      */

      if(GAdebug){
        File() << "\n\n------ after cross over ------ \n";
        printMRT(mrt_p1);
        File() << "\n" << '\n';
        printMRT(mrt_p2);
        File() << "\n" << '\n';
        printMRT(mrt_cub1);
        File() << "\n" << '\n';
        printMRT(mrt_cub2);
        File() << "\n" << '\n';
      }

      legalizeMRT(mrt_cub1, ii);
      //std::cout << "nouts: " << legalizeMRT(mrt_cub1, ii) << '\n';
      assert(checkGAmrt(mrt_cub1, ii, false) && "after legalize cub 1");
      legalizeMRT(mrt_cub2, ii);
      //std::cout << "nouts: " << legalizeMRT(mrt_cub2, ii) << '\n';
      assert(checkGAmrt(mrt_cub2, ii, false) && "after legalize cub 1");

      //TODO insert mutation mutation();
      //assert(checkGAmrt(mrt_cub1, ii, false) && "before mutation on cub 1");
      mutation(mrt_cub1, ii);
      //assert(checkGAmrt(mrt_cub1, ii, false) && "after mutation on cub 1");

      //assert(checkGAmrt(mrt_cub2, ii, false) && "before mutation on cub 2");
      mutation(mrt_cub2, ii);
      //assert(checkGAmrt(mrt_cub2, ii, false) && "after mutation on cub 2");

      //fixCongruenceDependecies(mrt_cub1, ii);
      //assert(checkGAmrt(mrt_cub1, ii, false) && "after congruence fix on cub 1");
      //fixCongruenceDependecies(mrt_cub2, ii);
      //assert(checkGAmrt(mrt_cub2, ii, false) && "after congruence fix on cub 1");

      cub1 = new Individual(mrt_cub1, -1);
      cub2 = new Individual(mrt_cub2, -1);
      //std::cout << "evaluate1" << '\n';
      evaluateIndividual(cub1, ii);
      //std::cout << "evaluate2" << '\n';
      evaluateIndividual(cub2, ii);
      specimina->push_back(cub1);
      specimina->push_back(cub2);

      if(GAdebug){
        File() << "\nmrt parent1 - fit: "<< parent1->second << ":\n";
        printMRT(mrt_p1);
        File() << "\nmrt parent2 - fit: "<< parent2->second << ":\n";
        printMRT(mrt_p2);
        File() << "\nmrt cub1 - fit: "<< cub1->second << ":\n";
        printMRT(mrt_cub1);
        File() << "\nmrt cub2 - fit: "<< cub2->second << ":\n";
        printMRT(mrt_cub2);
      }
    }//for(unsigned cub=0; cub<offspringSize; cub+=2)

  *IIin = ii;
  return;
}

void ILPModuloScheduler::getBestSolution(){

  if(GAdebug){
    File() << "--------- Saving Best Solution! -------\n";
  }

  assert(best_ever.first != NULL);

  unsigned II = best_ever.second;
  Individual * best_individual = best_ever.first;
  //flag = true;
  MRT * mrt = best_individual->first;

  assert(checkGAmrt(mrt, II, false) && "best individual's MRT");

  lprec * lp = copy_lp(base_lps[II]);
  //modifyGADependencyConstraintsForKernel(lp, mrt, II);
  modifyCongruenceLP(mrt, lp, II, false);
  //write_LP(lp, stderr);

  if(GAdebug){
    File() << "\n ----- best MRT: \n";
    printMRT(mrt);
  }

  int *variableIndices = new int[numInst];
  std::map<InstructionNode*, int> variablesM;
  std::map<InstructionNode*, int> variablesT;
  REAL *variableCoefficients = new REAL[numInst];
  //std::cout << "numInst:" << numInst << '\n';

  int count = 0;
  for (std::map<InstructionNode *, unsigned>::iterator i = startVariableIndex.begin(), e = startVariableIndex.end(); i != e; i++) {
    unsigned varIndex = i->second;
    assert(count < numInst);
    //i->first->getInst()->dump();
    variableIndices[count] = varIndex;
    variableCoefficients[count] = 1.0;
    variablesM[i->first] = (*mrt)[i->first].first;
    //std::cout << "variableIndices[" << count << "]=" << varIndex << " - m: " << (*mrt)[i->first].first << '\n';
    count++;
  }
  assert(count == numInst);

  set_obj_fnex(lp, count, variableCoefficients, variableIndices);
  set_minim(lp);

  if (GAdebug){
    write_LP(lp, stderr);
  }
  else{
    set_verbose(lp, 1);
  }

  int ret;
  clock_t ticsv, tocsv;
  if(solver.compare("gurobi")==0){
    if(GAGRBmodel != NULL){
      delete GAGRBmodel;
      GAGRBmodel = NULL;
    }

    char file[10] = "lp.mps";
    //FILE * file = fopen("lp.mps", "w+");
    write_freemps(lp, file);
    //fclose(file);
    GAGRBmodel = new GRBModel(env, "lp.mps");
    GAGRBmodel->set(GRB_IntParam_OutputFlag, 0);

    ticsv = clock();
    GAGRBmodel->optimize();
    tocsv = clock();

    GRBsolution = GAGRBmodel->getVars();

    int optimstatus = GAGRBmodel->get(GRB_IntAttr_Status);

    //std::cout << "optimstatus: " << optimstatus << '\n';
    if (optimstatus == GRB_OPTIMAL) {
      ret = 0;
      double objval = GAGRBmodel->get(GRB_DoubleAttr_ObjVal);
      std::cout << "Optimal objective: " << objval << '\n';
    } else if (optimstatus == GRB_INFEASIBLE) {
      ret = 2;
      std::cout << "Model is infeasible" << '\n';
    }else if (optimstatus == GRB_UNBOUNDED) {
      std::cout << "Model is unbounded" << '\n';
      ret = 3;
    }else {
      std::cout << "Optimization was stopped with status = "<< optimstatus << '\n';
    }

    //SDCGRBmodel->write("grbmodel.mps");
    //assert(false);
  }else{
    ticsv = clock();
    ret = solve(lp);
    tocsv = clock();
  }

  solvetime += (double)(tocsv - ticsv) / CLOCKS_PER_SEC;
  // printf("%f\n", solvetime);
  nsdcs++;

  if (ret != 0) {
    File() << "  LP solver returned: " << ret << "\n";
    File() << "  LP solver could not find an optimal solution\n";
    //report_fatal_error("LP solver could not find an optimal solution");
    //return;
  }
  std::cout << "/* message */" << '\n';
  REAL *solution = new REAL[numInst];
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


  int max = 0;
  for(BasicBlock::iterator id = BB->begin(), ide = BB->end(); id!=ide; ++id){
    InstructionNode * i = dag->getInstructionNode(id);
    int idx = startVariableIndex[i]-1;

    if(GAdebug){
      File() << "inst: " << getLabel(id) << " - variableM: " << variablesM[i] << " + II*solution: " << solution[idx] << " = " << (int )(variablesM[i]+II*solution[idx]) << '\n';
    }

    assert(solution[idx] == round(solution[idx]) && "Solution was supposed to be integer");
    variablesT[i] = variablesM[i]+II*solution[idx];
    if(variablesT[i] > max){
      max = variablesT[i];
    }

    //(*mrt)[i] = std::pair<unsigned, unsigned>(variablesT[i]%II, 0);
  }

  //legalizeMRT(mrt, II);
  assert(checkGAmrt(mrt, II) && "at evaluating best Individual");

  //assert(max > 0);

  if (GAdebug) {
      File() << "SDC solver status: " << ret << "\n";
  }

  variablesM.clear();
  delete[] variableCoefficients;
  delete[] variableIndices;
  sdcSolver.lp = lp;
  //delete_lp(lp);

  for(BasicBlock::iterator id = BB->begin(), ide = BB->end(); id!=ide; ++id){
    InstructionNode * i = dag->getInstructionNode(id);
    int idx = startVariableIndex[i]-1;

    assert(idx <= numVars);
    moduloScheduler.schedTime[id] = variablesT[i];
  }
  moduloScheduler.II = II;
  saveSchedule(/*lpSolve=*/true);
  variablesT.clear();
  return;
}

void ILPModuloScheduler::addGADependencyConstraints(lprec * lp, InstructionNode *in) {

    int col[2];
    REAL val[2];

    for (InstructionNode::iterator i = in->dep_begin(), e = in->dep_end(); i != e; ++i) {
      // Dependency: depIn -> in
      InstructionNode *depIn = *i;

      unsigned latency = latencyInstMap[depIn];

      //int min = (*mrt)[in].first;
      col[0] = startVariableIndex[in];
      val[0] = 1.0;
      //int mdepIn = (*mrt)[depIn].first;
      col[1] = startVariableIndex[depIn];
      val[1] = -1.0;

      // if chaining is permitted, then the instructions can be in the
      // SAME cycle
      // if chaining is NOT permitted, a dependent instruction is moved to
      // a  LATER cycle
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
      //int newb = b-(val[0]*min+val[1]*mdepIn);
      add_constraintex(lp, 2, val, col, GE, b);
      numConstraints++;
      //File() << "adding binary dep : C" << startVariableIndex[in] << " - C" << startVariableIndex[depIn] << ">=" << b << '\n';

      //if(constrained_insts[in] || constrained_insts[depIn]){
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
        add_constraintex(lp, 2, val, col, GE, b);
        numConstraints++;
        //File() << "adding mem dep : C" << startVariableIndex[in] << " - C" << startVariableIndex[memDepIn] << ">=" << b << '\n';
        //if(constrained_insts[in] || constrained_insts[memDepIn]){
          rh_m_map.push_back(std::tuple<InstructionNode*, InstructionNode*, int>(in, memDepIn, numConstraints));
        //}
    }
}

void ILPModuloScheduler::addGADependencyConstraintsForKernel(lprec *lp) {
  assert(BB);
  rh_m_map.clear();
  rh_ii_dist_map.clear();
  back_edge_rh_m_map.clear();
  back_edge_row_rh_map.clear();

  // iterate over the instructions in a BB
  for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; i++) {
    addGADependencyConstraints(lp, dag->getInstructionNode(i));
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
      //int mj = (*mrt)[inodej].first;
      col[0] = startVariableIndex[inodej];
      val[0] = 1.0;
      //int mi = (*mrt)[inodei].first;
      col[1] = startVariableIndex[inodei];
      val[1] = -1.0;

      // if chaining is permitted, then the instructions can be in the SAME cycle if chaining is NOT permitted, a dependent instruction is moved to a  LATER cycle
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

      int b = chainingLatency+latency;//-II*dist+latency;
      //int newb = b-(val[0]*mj+val[1]*mi);
      add_constraintex(lp, 2, val, col, GE, b);
      numConstraints++;

      rh_ii_dist_map.push_back(std::pair<int, int>(dist, numConstraints));
      //if(constrained_insts[inodej] || constrained_insts[inodei]){
        back_edge_rh_m_map.push_back(std::tuple<InstructionNode*, InstructionNode*, int>(inodej, inodei, numConstraints));
        back_edge_row_rh_map[numConstraints] = std::tuple<int, int, int>(chainingLatency, dist, latency);
      //}
    }
  }
}

void ILPModuloScheduler::modifyIIDistLP(unsigned ii){
  lprec *lp = base_lps[ii];

  for(auto entry : rh_ii_dist_map){
    REAL oldrh = get_rh(lp, entry.second);
    set_rh(lp, entry.second, oldrh-ii*entry.first);
  }
}

//legacy
/*
void ILPModuloScheduler::createBaseLPS(unsigned minII, unsigned maxII){
  printLineBreak();
  if (GAdebug)
      File() << "initializing BASE LP" << "\n";

  chaining = false; // default is no chaining -- maximally pipelined
  clockPeriodConstraint = -1.0; // default is no clock period constraint
  moduloScheduler.schedTime.clear();

  //std::cout << "create base LP" << '\n';
  lprec *base_lp;
  createGAVariables(&base_lp);

  chaining = true;
  // avoid chaining for now - along with timing constraints
  // chaining = false;
  if (LEGUP_CONFIG->getParameterInt("SDC_NO_CHAINING")) {
    chaining = false; // no chaining means that the design will be
    // pipelined as much as possible
  }

  //TODO add timing constraints later
  clockPeriodConstraint = 15; // 66 MHz
  if (LEGUP_CONFIG->getParameterInt("CLOCK_PERIOD")) {
    clockPeriodConstraint =
    (float)LEGUP_CONFIG->getParameterInt("CLOCK_PERIOD");
  }

  //std::cout << "adding dependencies for kernel" << '\n';
  addGADependencyConstraintsForKernel(base_lp);
  if (chaining && clockPeriodConstraint > 0) {
    if (!LEGUP_CONFIG->getParameterInt("SDC_NO_TIMING_CONSTRAINTS")) {
      //addTimingConstraintsForKernel();
    }
  }

  for(unsigned ii=minII; ii<=maxII; ii++){
    //std::cout << "modify for ii: " << ii << '\n';
    base_lps[ii] = copy_lp(base_lp);
    //modifyIIDistLP(ii);
  }

  //std::cout << "deleting base LP" << '\n';
  delete_lp(base_lp);
  printLineBreak();
}
*/
void ILPModuloScheduler::createBaseLPS(unsigned ii){
  printLineBreak();
  if (GAdebug)
      File() << "initializing BASE LP" << "\n";

  chaining = false; // default is no chaining -- maximally pipelined
  clockPeriodConstraint = -1.0; // default is no clock period constraint
  moduloScheduler.schedTime.clear();

  //std::cout << "create base LP" << '\n';
  lprec *base_lp;
  createGAVariables(&base_lp);

  chaining = true;
  // avoid chaining for now - along with timing constraints
  // chaining = false;
  if (LEGUP_CONFIG->getParameterInt("SDC_NO_CHAINING")) {
    chaining = false; // no chaining means that the design will be
    // pipelined as much as possible
  }

  //TODO add timing constraints later
  clockPeriodConstraint = 15; // 66 MHz
  if (LEGUP_CONFIG->getParameterInt("CLOCK_PERIOD")) {
    clockPeriodConstraint =
    (float)LEGUP_CONFIG->getParameterInt("CLOCK_PERIOD");
  }

  //std::cout << "adding dependencies for kernel" << '\n';
  addGADependencyConstraintsForKernel(base_lp);
  if (chaining && clockPeriodConstraint > 0) {
    if (!LEGUP_CONFIG->getParameterInt("SDC_NO_TIMING_CONSTRAINTS")) {
      //addTimingConstraintsForKernel();
    }
  }

  base_lps[ii] = copy_lp(base_lp);
  //modifyIIDistLP(ii);

  //std::cout << "deleting base LP" << '\n';
  delete_lp(base_lp);
  printLineBreak();
}

void ILPModuloScheduler::initializeMaps(){
  FUinstMap.clear();
  FUlimit.clear();
  constrained_insts.clear();
  latencyInstMap.clear();
  baseCongruenceClass.clear();
  instLinkerIndex.clear();

  for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; i++) {
    //std::cout << getLabel(i) << '\n';
    InstructionNode *iNode = dag->getInstructionNode(i);
    std::string FuName = LEGUP_CONFIG->getOpNameFromInst(i, moduloScheduler.alloc);

    //fills the FUinstMap wich says which instruction uses which resource, will be useful in the individual generation
    //int delay = Scheduler::getNumInstructionCycles(i);

    int constraint;
    if (!LEGUP_CONFIG->getNumberOfFUsAllocated(FuName, &constraint)){
      //std::cout << "non res constrained inst: " << getLabel(i) << " - latency: " << delay << '\n';
      //assert(delay == 0 && "non res constrained has delay 0?");
      continue;
    }

    //std::cout << "res constrained inst: " << getLabel(i) << " - resource: " << FuName << " - latency: " << delay << '\n';
    constrained_insts[iNode] = true;

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
  }

  return;
}

void ILPModuloScheduler::GA(unsigned * minII, unsigned * maxII){
  //std::cout << "\n\ninitilize maps" << '\n';
  //nPop = 400;
  //offspringSize = nPop;
  //maxGen = 100;
  initializeMaps();

  bool incII = false;
  unsigned iireg = *minII;
  unsigned * ii = &iireg;
  do{
    if(incII){
      (*ii) = (*ii)+1;
    }

    //std::cout << "create base LPs" << '\n';
    //createBaseLPS(*minII, *maxII);
    createBaseLPS(*ii);
    feasible = false;

    //std::cout << "initialize pop" << '\n';
    //initializePopulation(minII, maxII);
    initializePopulation(ii);
    //std::cout << "popInitialized" << '\n';
    unsigned gen=0;
    //std::cout << "npop: " << nPop <<'\n';
    //std::cout << "gen: " << gen << " - maxGen: " << maxGen << '\n';
    do{
      std::cout << "\ngen: " << gen << '\n';
      //std::cout << "calculate new pop, gen: " << gen << '\n';
      //calculateNewPopulation(minII, maxII);
      calculateNewPopulation(ii);
      //std::cout << "selection" << '\n';
      selection(*ii);

      gen++;
      std::cout << "feasible: " << feasible << '\n';

      if(gen >= 2*maxGen && !feasible){
        std::cout << "incrementing II" << '\n';
        best_ever = std::pair<Individual *, unsigned>((Individual*)NULL, 0);
        incII = true;
        break;
      }
      //std::cout << "(gen>=maxGen && feasible): " << (gen>=maxGen && feasible) << '\n';
    }while(!(gen>=maxGen && feasible));
    //}while(!(gen>=maxGen));

    if(feasible){
      std::cout << "trying to get best sol" << '\n';
      getBestSolution();
      printMRT(best_ever.first->first);
      incII = false;
    }

  }while(incII);
  //std::cout << "trying to clean stuff" << '\n';
  cleanGA();
}
