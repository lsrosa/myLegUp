disp('applying the Path DSE on no-pipe to seed Lattice on NIS pipe')

disp('-')

[pathSeedPipeTotalConfigs, pathSeedPipeADRS] = pathSeedLattice(constraints, noPipeMetrics, pipeMetrics, 'path');

disp('-')

accN = 0;
accADRS = 0;
for cnt = 1:nreps
  [latticeSeedPipeTotalConfigs, latticeSeedPipeADRS] = pathSeedLattice(constraints, noPipeMetrics, pipeMetrics, 'lattice');
  accN = accN + latticeSeedPipeTotalConfigs;
  accADRS = accADRS + latticeSeedPipeADRS;
end
latticeSeedPipeTotalConfigs = accN/nreps;
latticeSeedPipeADRS = accADRS/nreps;

disp('-')

[pathLatticeSeedPipeTotalConfigs, pathLatticeSeedPipeADRS] = pathSeedLattice(constraints, noPipeMetrics, pipeMetrics, 'path+lattice');

disp('-')

[separatedTotalConfigs, separatedADRS] = separateSetsDSE(constraints, noPipeMetrics, pipeMetrics, nreps);

disp('-')

[combinedTotalConfigs, combinedADRS] = combineSetsDSE(constraints, noPipeMetrics, pipeMetrics, nreps);


rn = [pathSeedPipeTotalConfigs, latticeSeedPipeTotalConfigs, pathLatticeSeedPipeTotalConfigs, separatedTotalConfigs', combinedTotalConfigs'];
ra = 100*[pathSeedPipeADRS, latticeSeedPipeADRS, pathLatticeSeedPipeADRS, separatedADRS', combinedADRS'];

 num2str(rn)
 num2str(ra)

if(numel(arg_list) == 3)
  disp('applying the Path DSE on no-pipe to seed Lattice on ILPS pipe')

  disp('-')

  [pathSeedIlpPipeTotalConfigs, pathSeedIlpPipeADRS] = pathSeedLattice(constraints, noPipeMetrics, ilpPipeMetrics, 'path');

  disp('-')
  accN = 0;
  accADRS = 0;
  for cnt = 1:nreps
    [latticeSeedIlpPipeTotalConfigs, latticeSeedIlpPipeADRS] = pathSeedLattice(constraints, noPipeMetrics, ilpPipeMetrics, 'lattice');
    accN = accN + latticeSeedIlpPipeTotalConfigs;
    accADRS = accADRS + latticeSeedIlpPipeADRS;
  end
  latticeSeedIlpPipeTotalConfigs = accN/nreps;
  latticeSeedIlpPipeADRS = accADRS/nreps;

  disp('-')

  [pathLatticeSeedIlpPipeTotalConfigs, pathLatticeSeedIlpPipeADRS] = pathSeedLattice(constraints, noPipeMetrics, ilpPipeMetrics, 'path+lattice');

  disp('-')

  [separatedTotalConfigsIlp, separatedADRSIlp] = separateSetsDSE(constraints, noPipeMetrics, ilpPipeMetrics, nreps);

  disp('-')

  [combinedTotalConfigsIlp, combinedADRSIlp] = combineSetsDSE(constraints, noPipeMetrics, ilpPipeMetrics, nreps);

  rn = [pathSeedIlpPipeTotalConfigs, latticeSeedIlpPipeTotalConfigs, pathLatticeSeedIlpPipeTotalConfigs, separatedTotalConfigsIlp', combinedTotalConfigsIlp'];
  ra = 100*[pathSeedIlpPipeADRS, latticeSeedIlpPipeADRS, pathLatticeSeedIlpPipeADRS, separatedADRSIlp', combinedADRSIlp'];

   num2str(rn)
   num2str(ra)
end
