function [rn, radrs] = pathSeedLattice(constraints, seedsSet, latticeSet, seedsAlgo = 'path')
  %pathSet
  %latticeSet

  %apply the path DSE on the first set

  if(strcmp(seedsAlgo, 'path'))
    [seedsSetConfigs, seedsSetMetrics, seedsSetIdx, seedsSetN] = pathDSE(constraints, seedsSet);
  elseif(strcmp(seedsAlgo, 'lattice'))
    [seedsSetConfigs, seedsSetMetrics, seedsSetIdx, seedsSetN] = latticeDSE(constraints, seedsSet);
  elseif(strcmp(seedsAlgo, 'path+lattice'))
    [c, m, id, n] = pathDSE(constraints, seedsSet);
    [px, py, pid] = findPareto(m(:,1), m(:,2));
    s = c(pid, :);
    [seedsSetConfigs, seedsSetMetrics, seedsSetIdx, seedsSetN] = latticeDSE(constraints, seedsSet, s);
    seedsSetN = seedsSetN + n - rows(pid);
  else
    assert(false, 'choose path, lattice, or path+lattice');
  end

  %get its pareto points for seeding the Lattice DSE
  [seedsSetParetoX, seedsSetParetoY, seedsSetParetoId] = findPareto(seedsSetMetrics(:,1), seedsSetMetrics(:,2));
  seeds = seedsSetConfigs(seedsSetParetoId, :);

  %apply the seeded lattice DSE
  [latticeSetConfigs, latticeSetMetrics, latticeSetIdx, latticeSetN] = latticeDSE(constraints, latticeSet, seeds);

  %concatenate de sets so we can measure the total n configs and ADRS
  totalInputSet = [seedsSet; latticeSet];
  totalExploredSet = [seedsSetMetrics; latticeSetMetrics];

  %get the paretos over the combined sets
  [px, py, idx] = findPareto(totalInputSet(:,1), totalInputSet(:,2));

  %get the paretos over the explored combined sets
  [pxx, pxy, xidx] = findPareto(totalExploredSet(:,1), totalExploredSet(:,2));

  rn = seedsSetN + latticeSetN - rows(seeds);
  radrs = adrs([px, py], [pxx, pxy]);
end
