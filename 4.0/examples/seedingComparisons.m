disp('applying the Path DSE on no-pipe to seed Lattice on NIS pipe')

[noPipeSeedPipeTotalConfigs, noPipeSeedPipeADRS] = pathSeedLattice(constraints, noPipeMetrics, pipeMetrics)

[pipeSeedNoPipeTotalConfigs, pipeSeedNoPipeADRS] = pathSeedLattice(constraints, pipeMetrics, noPipeMetrics)

[combinedTotalConfigs, combinedADRS] = combineSetsDSE(constraints, noPipeMetrics, pipeMetrics, 10)

if(numel(arg_list) == 3)
  disp('applying the Path DSE on no-pipe to seed Lattice on ILPS pipe')

  [noPipeSeedIlpPipeTotalConfigs, noPipeSeedIlpPipeADRS] = pathSeedLattice(constraints, noPipeMetrics, ilpPipeMetrics)

  [ilpPipeSeedNoPipeTotalConfigs, ilpPipeSeedNoPipeADRS] = pathSeedLattice(constraints, ilpPipeMetrics, noPipeMetrics)

  [combinedTotalConfigs, combinedADRS] = combineSetsDSE(constraints, noPipeMetrics, ilpPipeMetrics, 10)
end
