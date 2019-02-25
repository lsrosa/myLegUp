disp('')
disp('appliying the Path DSE in the no-pipe space')

[noPipeConfigsPath, noPipeMetricsPath, noPipeIdxPath, noPipeDesignsPath] = pathDSE(constraints, noPipeMetrics);

[noPipeParetoX, noPipeParetoY, noPipeParetoId] = findPareto(noPipeMetrics(:,1), noPipeMetrics(:,2));

[noPipeParetoPathX,noPipeParetoPathY,noPipeParetoPathId] = findPareto(noPipeMetricsPath(:,1), noPipeMetricsPath(:,2));

disp('')
noPipeDesignsPath
noPipePathADRS = adrs([noPipeParetoX, noPipeParetoY], [noPipeParetoPathX, noPipeParetoPathY])

disp('')
disp('appliying the lattice DSE in the no-pipe space without using seeds')

accLattice = 0;
accADRS = 0;
for cnt = 1:nreps

  [noPipeConfigsLattice, noPipeMetricsLattice, noPipeIdxLattice, noPipeDesignsLattice] = latticeDSE(constraints, noPipeMetrics);

  [noPipeParetoLatticeX,noPipeParetoLatticeY,noPipeParetoLatticeId] = findPareto(noPipeMetricsLattice(:,1), noPipeMetricsLattice(:,2));

  accLattice = accLattice + noPipeDesignsLattice;
  noPipeLatticeADRS = adrs([noPipeParetoX, noPipeParetoY], [noPipeParetoLatticeX, noPipeParetoLatticeY]);
  accADRS = accADRS + noPipeLatticeADRS;
end

disp('')
noPipeDesignsLattice = accLattice/nreps
noPipeLatticeADRS = accADRS/nreps

disp('')
disp('appliying the lattice DSE in the no-pipe space using the pathDSE pareto points seeds')

%get the pareto points of the PathDSE as seeds for the latticeDSE
seeds = noPipeConfigsPath(noPipeParetoPathId, :);

[noPipeConstLatticePath, noPipeMetricsLatticePath, noPipeIdxLatticePath, noPipeDesignsLatticePath] = latticeDSE(constraints, noPipeMetrics, seeds);

[noPipeParetoLatticePathX, noPipeParetoLatticePathY, noPipeParetoLatticePathId] = findPareto(noPipeMetricsLatticePath(:,1), noPipeMetricsLatticePath(:,2));

disp('')
noPipeDesignsLatticePath = noPipeDesignsLatticePath - rows(seeds) + noPipeDesignsPath
noPipeLatticePathADRS = adrs([noPipeParetoX, noPipeParetoY], [noPipeParetoLatticePathX, noPipeParetoLatticePathY])

%---------------------------------------------------------------------------
%---------------------------------------------------------------------------
%---------------------------------------------------------------------------
disp('')
disp('appliying the Path DSE in the pipe space')

[pipeConfigsPath, pipeMetricsPath, pipeIdxPath, pipeDesignsPath] = pathDSE(constraints, pipeMetrics);

[pipeParetoX, pipeParetoY, pipeParetoId] = findPareto(pipeMetrics(:,1), pipeMetrics(:,2));

[pipeParetoPathX,pipeParetoPathY,pipeParetoPathId] = findPareto(pipeMetricsPath(:,1), pipeMetricsPath(:,2));

disp('')
pipeDesignsPath
pipePathADRS = adrs([pipeParetoX, pipeParetoY], [pipeParetoPathX, pipeParetoPathY])

disp('')
disp('appliying the lattice DSE in the pipe space without using seeds')

accLattice = 0;
accADRS = 0;
for cnt = 1:nreps
  [pipeConfigsLattice, pipeMetricsLattice, pipeIdxLattice, pipeDesignsLattice] = latticeDSE(constraints, pipeMetrics);

  [pipeParetoLatticeX,pipeParetoLatticeY,pipeParetoLatticeId] = findPareto(pipeMetricsLattice(:,1), pipeMetricsLattice(:,2));

  accLattice = accLattice + pipeDesignsLattice;
  pipeLatticeADRS = adrs([pipeParetoX, pipeParetoY], [pipeParetoLatticeX, pipeParetoLatticeY]);
  accADRS = accADRS + pipeLatticeADRS;
end
pipeDesignsLattice = accLattice/nreps
pipeLatticeADRS = accADRS/nreps


disp('appliying the lattice DSE in the pipe space using the pathDSE pareto points seeds')

%get the pareto points of the PathDSE as seeds for the latticeDSE
seeds = pipeConfigsPath(pipeParetoPathId, :);

[pipeConstLatticePath, pipeMetricsLatticePath, pipeIdxLatticePath, pipeDesignsLatticePath] = latticeDSE(constraints, pipeMetrics, seeds);

[pipeParetoLatticePathX, pipeParetoLatticePathY, pipeParetoLatticePathId] = findPareto(pipeMetricsLatticePath(:,1), pipeMetricsLatticePath(:,2));

disp('')
pipeDesignsLatticePath = pipeDesignsLatticePath - rows(seeds) + pipeDesignsPath
pipeLatticePathADRS = adrs([pipeParetoX, pipeParetoY], [pipeParetoLatticePathX, pipeParetoLatticePathY])

if(numel(arg_list) < 3)
  return;
end
%---------------------------------------------------------------------------
%---------------------------------------------------------------------------
%---------------------------------------------------------------------------
disp('')
disp('appliying the Path DSE in the ilp-pipe space')

[ilpPipeConfigsPath, ilpPipeMetricsPath, ilpPipeIdxPath, ilpPipeDesignsPath] = pathDSE(constraints, ilpPipeMetrics);

[ilpPipeParetoX, ilpPipeParetoY, ilpPipeParetoId] = findPareto(ilpPipeMetrics(:,1), ilpPipeMetrics(:,2));

[ilpPipeParetoPathX,ilpPipeParetoPathY,ilpPipeParetoPathId] = findPareto(ilpPipeMetricsPath(:,1), ilpPipeMetricsPath(:,2));

disp('')
ilpPipeDesignsPath
ilpPipePathADRS = adrs([ilpPipeParetoX, ilpPipeParetoY], [ilpPipeParetoPathX, ilpPipeParetoPathY])

disp('')
disp('appliying the lattice DSE in the ilp-pipe space without using seeds')

accLattice = 0;
accADRS = 0;
for cnt = 1:nreps
  [ilpPipeConfigsLattice, ilpPipeMetricsLattice, ilpPipeIdxLattice, ilpPipeDesignsLattice] = latticeDSE(constraints, ilpPipeMetrics);

  [ilpPipeParetoLatticeX,ilpPipeParetoLatticeY,ilpPipeParetoLatticeId] = findPareto(ilpPipeMetricsLattice(:,1), ilpPipeMetricsLattice(:,2));

  accLattice = accLattice + ilpPipeDesignsLattice;
  ilpPipeLatticeADRS = adrs([ilpPipeParetoX, ilpPipeParetoY], [ilpPipeParetoLatticeX, ilpPipeParetoLatticeY]);
  accADRS = accADRS + ilpPipeLatticeADRS;

end

disp('')
ilpPipeDesignsLattice = accLattice/nreps
ilpPipeLatticeADRS = accADRS/nreps


disp('')
disp('appliying the lattice DSE in the ilp-pipe space using the pathDSE pareto points seeds')

%get the pareto points of the PathDSE as seeds for the latticeDSE
seeds = ilpPipeConfigsPath(ilpPipeParetoPathId, :);

[ilpPipeConstLatticePath, ilpPipeMetricsLatticePath, ilpPipeIdxLatticePath, ilpPipeDesignsLatticePath] = latticeDSE(constraints, ilpPipeMetrics, seeds);

[ilpPipeParetoLatticePathX, ilpPipeParetoLatticePathY, ilpPipeParetoLatticePathId] = findPareto(ilpPipeMetricsLatticePath(:,1), ilpPipeMetricsLatticePath(:,2));

disp('')
ilpPipeDesignsLatticePath = ilpPipeDesignsLatticePath - rows(seeds) + ilpPipeDesignsPath
ilpPipeLatticePathADRS = adrs([ilpPipeParetoX, ilpPipeParetoY], [ilpPipeParetoLatticePathX, ilpPipeParetoLatticePathY])
