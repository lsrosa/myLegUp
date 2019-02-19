function [rn, radrs] = pathSeedLattice(constraints, pathSet, latticeSet)
  %pathSet
  %latticeSet
  
  %apply the path DSE on the first set
  [pathSetConfigs, pathSetMetrics, pathSetIdx, pathSetN] = pathDSE(constraints, pathSet);

  %get its pareto points for seeding the Lattice DSE
  [pathSetParetoX, pathSetParetoY, pathSetParetoId] = findPareto(pathSetMetrics(:,1), pathSetMetrics(:,2));
  seeds = pathSetConfigs(pathSetParetoId, :);

  %apply the seeded lattice DSE
  [latticeSetConfigs, latticeSetMetrics, latticeSetIdx, latticeSetN] = latticeDSE(constraints, latticeSet, seeds);

  %concatenate de sets so we can measure the total n configs and ADRS
  totalInputSet = [pathSet; latticeSet];
  totalExploredSet = [pathSetMetrics; latticeSetMetrics];

  %get the paretos over the combined sets
  [px, py, idx] = findPareto(totalInputSet(:,1), totalInputSet(:,2));

  %get the paretos over the explored combined sets
  [pxx, pxy, xidx] = findPareto(totalExploredSet(:,1), totalExploredSet(:,2));

  rn = pathSetN + latticeSetN - rows(seeds);
  radrs = adrs([px, py], [pxx, pxy]);
end
