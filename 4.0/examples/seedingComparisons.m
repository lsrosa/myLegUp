disp('applying the Path DSE on no-pipe to seed Lattice on NIS pipe')

disp('')

[pathSeedPipeTotalConfigs, pathSeedPipeADRS] = pathSeedLattice(constraints, noPipeMetrics, pipeMetrics, 'path')

disp('')

accN = 0;
accADRS = 0;
for cnt = 1:nreps
  [latticeSeedPipeTotalConfigs, latticeSeedPipeADRS] = pathSeedLattice(constraints, noPipeMetrics, pipeMetrics, 'lattice');
  accN = accN + latticeSeedPipeTotalConfigs;
  accADRS = accADRS + latticeSeedPipeADRS;
end
latticeSeedPipeTotalConfigs = accN/nreps
latticeSeedPipeADRS = accADRS/nreps

disp('')

[pathLatticeSeedPipeTotalConfigs, pathLatticeSeedPipeADRS] = pathSeedLattice(constraints, noPipeMetrics, pipeMetrics, 'path+lattice')

disp('')

[combinedTotalConfigs, combinedADRS] = combineSetsDSE(constraints, noPipeMetrics, pipeMetrics, 10)

if(numel(arg_list) == 3)
  disp('applying the Path DSE on no-pipe to seed Lattice on ILPS pipe')

  disp('')

  [pathSeedIlpPipeTotalConfigs, pathSeedIlpPipeADRS] = pathSeedLattice(constraints, noPipeMetrics, ilpPipeMetrics, 'path')

  disp('')
  accN = 0;
  accADRS = 0;
  for cnt = 1:nreps
    [latticeSeedIlpPipeTotalConfigs, latticeSeedIlpPipeADRS] = pathSeedLattice(constraints, noPipeMetrics, ilpPipeMetrics, 'lattice');
    accN = accN + latticeSeedIlpPipeTotalConfigs;
    accADRS = accADRS + latticeSeedIlpPipeADRS;
  end
  latticeSeedIlpPipeTotalConfigs = accN/nreps
  latticeSeedIlpPipeADRS = accADRS/nreps

  disp('')

  [pathLatticeSeedIlpPipeTotalConfigs, pathLatticeSeedIlpPipeADRS] = pathSeedLattice(constraints, noPipeMetrics, ilpPipeMetrics, 'path+lattice')

  disp('')

  [combinedTotalConfigsIlp, combinedADRSIlp] = combineSetsDSE(constraints, noPipeMetrics, ilpPipeMetrics, 10)
end
