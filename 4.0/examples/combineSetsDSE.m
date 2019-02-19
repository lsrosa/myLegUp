function [rn, radrs] = combineSetsDSE(constraints, pathSet, latticeSet, nreps = 1)
  totalInputSet = [pathSet; latticeSet];
  totalConstraintsSet = [constraints; constraints];

  %get the paretos over the combined sets
  [px, py, idx] = findPareto(totalInputSet(:,1), totalInputSet(:,2));

%------------------------------------------------------------------------

  %apply the path DSE on the first set
  [pathConfigs, pathMetrics, pathIdx, pathN] = pathDSE(totalConstraintsSet, totalInputSet);

  [pathParetoX, pathParetoY, pathParetoId] = findPareto(pathMetrics(:,1), pathMetrics(:,2));
  %get its pareto points for seeding the Lattice DSE
  seeds = pathConfigs(pathParetoId, :);

  pathADRS = adrs([px, py], [pathParetoX, pathParetoY]);
%------------------------------------------------------------------------

accLattice = 0;
accADRS = 0;
for cnt = 1:nreps

  %apply the seeded lattice DSE without seeds
  [latticeConfigs, latticeMetrics, latticeIdx, latticeN] = latticeDSE(totalConstraintsSet, totalInputSet);

  %get its pareto points
  [latticeParetoX, latticeParetoY, latticeParetoId] = findPareto(latticeMetrics(:,1), latticeMetrics(:,2));

  latticeADRS = adrs([px, py], [latticeParetoX, latticeParetoY]);

  accLattice = accLattice + latticeN;
  accADRS = accADRS + latticeADRS;
end

%make an average
latticeN = accLattice/nreps;
latticeADRS = accADRS/nreps;

%------------------------------------------------------------------------

  %apply the seeded lattice DSE with seeds
  [seedLatticeConfigs, seedLatticeMetrics, seedLatticeIdx, seedLatticeN] = latticeDSE(totalConstraintsSet, totalInputSet, seeds);

  %get its pareto points
  [seedLatticeParetoX, seedLatticeParetoY, seedLatticeParetoId] = findPareto(seedLatticeMetrics(:,1), seedLatticeMetrics(:,2));

  seedLatticeADRS = adrs([px, py], [seedLatticeParetoX, seedLatticeParetoY]);
%------------------------------------------------------------------------

  rn = [pathN; latticeN; seedLatticeN];
  radrs = [pathADRS; latticeADRS; seedLatticeADRS];
end
