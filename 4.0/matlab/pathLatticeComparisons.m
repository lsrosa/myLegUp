
[refParetoX, refParetoY, refParetoId] = findPareto(noPipeMetrics(:,1), noPipeMetrics(:,2));
[pmkConfigsPMK, pmkMetricsPMK, pmkIdxPMK, nDesignsPMK] = pmk10DSE(constraints, noPipeMetrics);
[pmkParetoX,pmkParetoY,pmkParetoId] = findPareto(pmkMetricsPMK(:,1), pmkMetricsPMK(:,2));
pmkADRS = adrs([refParetoX, refParetoY], [pmkParetoX, pmkParetoY]);
disp(strcat('no pipe: nDesigns: ', num2str(nDesignsPMK), ' adrs: ', num2str(pmkADRS)))

[refParetoX, refParetoY, refParetoId] = findPareto(pipeMetrics(:,1), pipeMetrics(:,2));
[pmkConfigsPMK, pmkMetricsPMK, pmkIdxPMK, nDesignsPMK] = pmk10DSE(constraints, pipeMetrics);
[pmkParetoX,pmkParetoY,pmkParetoId] = findPareto(pmkMetricsPMK(:,1), pmkMetricsPMK(:,2));
pmkADRS = adrs([refParetoX, refParetoY], [pmkParetoX, pmkParetoY]);
disp(strcat('   pipe: nDesigns: ', num2str(nDesignsPMK), ' adrs: ', num2str(pmkADRS)))


disp('appliying the lattice DSE in the pipe space using the PMK pareto points seeds')
[pmkConfigsPMK, pmkMetricsPMK, pmkIdxPMK, nDesignsPMK] = pmk10DSE(constraints, noPipeMetrics);
[pmkParetoX,pmkParetoY,pmkParetoId] = findPareto(pmkMetricsPMK(:,1), pmkMetricsPMK(:,2));

seeds = pmkConfigsPMK(pmkParetoId, :);
[LatticepmkConfigsPMK, LatticepmkMetricsPMK, LatticepmkIdxPMK, LatticenDesignsPMK] = latticeDSE(constraints, pipeMetrics, seeds);

[refParetoX, refParetoY, refParetoId] = findPareto([pipeMetrics(:,1); noPipeMetrics(:,1)], [pipeMetrics(:,2); noPipeMetrics(:,2)]);
[pmkParetoX,pmkParetoY,pmkParetoId] = findPareto([pmkMetricsPMK(:,1); LatticepmkMetricsPMK(:,1)], [pmkMetricsPMK(:,2); LatticepmkMetricsPMK(:,2)]);
pmkADRS = adrs([refParetoX, refParetoY], [pmkParetoX, pmkParetoY]);
disp(strcat(' seeded: nDesigns: ', num2str(nDesignsPMK-rows(seeds)+LatticenDesignsPMK), ' adrs: ', num2str(pmkADRS)))

disp('appliying the PMK in the pipe and no pipe spaces')
[noPipepmkConfigsPMK, noPipepmkMetricsPMK, noPipepmkIdxPMK, noPipenDesignsPMK] = pmk10DSE(constraints, noPipeMetrics);
[pipepmkConfigsPMK, pipepmkMetricsPMK, pipepmkIdxPMK, pipenDesignsPMK] = pmk10DSE(constraints, pipeMetrics);

[refParetoX, refParetoY, refParetoId] = findPareto([pipeMetrics(:,1); noPipeMetrics(:,1)], [pipeMetrics(:,2); noPipeMetrics(:,2)]);
[pmkParetoX,pmkParetoY,pmkParetoId] = findPareto([noPipepmkMetricsPMK(:,1); pipepmkMetricsPMK(:,1)], [noPipepmkMetricsPMK(:,2); pipepmkMetricsPMK(:,2)]);
pmkADRS = adrs([refParetoX, refParetoY], [pmkParetoX, pmkParetoY]);
disp(strcat('  union: nDesigns: ', num2str(noPipenDesignsPMK+pipenDesignsPMK), ' adrs: ', num2str(pmkADRS)))

return;

disp('appliying the Path DSE in the no-pipe space')

[noPipeConfigsPath, noPipeMetricsPath, noPipeIdxPath, noPipeDesignsPath] = pathDSE(constraints, noPipeMetrics);

[noPipeParetoX, noPipeParetoY, noPipeParetoId] = findPareto(noPipeMetrics(:,1), noPipeMetrics(:,2));

[noPipeParetoPathX,noPipeParetoPathY,noPipeParetoPathId] = findPareto(noPipeMetricsPath(:,1), noPipeMetricsPath(:,2));

noPipeDesignsPath;
noPipePathADRS = adrs([noPipeParetoX, noPipeParetoY], [noPipeParetoPathX, noPipeParetoPathY]);

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


noPipeDesignsLattice = accLattice/nreps;
noPipeLatticeADRS = accADRS/nreps;


disp('appliying the lattice DSE in the no-pipe space using the pathDSE pareto points seeds')

%get the pareto points of the PathDSE as seeds for the latticeDSE
seeds = noPipeConfigsPath(noPipeParetoPathId, :);

[noPipeConstLatticePath, noPipeMetricsLatticePath, noPipeIdxLatticePath, noPipeDesignsLatticePath] = latticeDSE(constraints, noPipeMetrics, seeds);

[noPipeParetoLatticePathX, noPipeParetoLatticePathY, noPipeParetoLatticePathId] = findPareto(noPipeMetricsLatticePath(:,1), noPipeMetricsLatticePath(:,2));


noPipeDesignsLatticePath = noPipeDesignsLatticePath - rows(seeds) + noPipeDesignsPath;
noPipeLatticePathADRS = adrs([noPipeParetoX, noPipeParetoY], [noPipeParetoLatticePathX, noPipeParetoLatticePathY]);

%---------------------------------------------------------------------------
%---------------------------------------------------------------------------
%---------------------------------------------------------------------------

disp('appliying the Path DSE in the pipe space')

[pipeConfigsPath, pipeMetricsPath, pipeIdxPath, pipeDesignsPath] = pathDSE(constraints, pipeMetrics);

[pipeParetoX, pipeParetoY, pipeParetoId] = findPareto(pipeMetrics(:,1), pipeMetrics(:,2));

[pipeParetoPathX,pipeParetoPathY,pipeParetoPathId] = findPareto(pipeMetricsPath(:,1), pipeMetricsPath(:,2));


pipeDesignsPath;
pipePathADRS = adrs([pipeParetoX, pipeParetoY], [pipeParetoPathX, pipeParetoPathY]);


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
pipeDesignsLattice = accLattice/nreps;
pipeLatticeADRS = accADRS/nreps;


disp('appliying the lattice DSE in the pipe space using the pathDSE pareto points seeds')

%get the pareto points of the PathDSE as seeds for the latticeDSE
seeds = pipeConfigsPath(pipeParetoPathId, :);

[pipeConstLatticePath, pipeMetricsLatticePath, pipeIdxLatticePath, pipeDesignsLatticePath] = latticeDSE(constraints, pipeMetrics, seeds);

[pipeParetoLatticePathX, pipeParetoLatticePathY, pipeParetoLatticePathId] = findPareto(pipeMetricsLatticePath(:,1), pipeMetricsLatticePath(:,2));


pipeDesignsLatticePath = pipeDesignsLatticePath - rows(seeds) + pipeDesignsPath;
pipeLatticePathADRS = adrs([pipeParetoX, pipeParetoY], [pipeParetoLatticePathX, pipeParetoLatticePathY]);

if(numel(arg_list) < 3)
  return;
end
%---------------------------------------------------------------------------
%---------------------------------------------------------------------------
%---------------------------------------------------------------------------

disp('appliying the Path DSE in the ilp-pipe space')

[ilpPipeConfigsPath, ilpPipeMetricsPath, ilpPipeIdxPath, ilpPipeDesignsPath] = pathDSE(constraints, ilpPipeMetrics);

[ilpPipeParetoX, ilpPipeParetoY, ilpPipeParetoId] = findPareto(ilpPipeMetrics(:,1), ilpPipeMetrics(:,2));

[ilpPipeParetoPathX,ilpPipeParetoPathY,ilpPipeParetoPathId] = findPareto(ilpPipeMetricsPath(:,1), ilpPipeMetricsPath(:,2));


ilpPipeDesignsPath;
ilpPipePathADRS = adrs([ilpPipeParetoX, ilpPipeParetoY], [ilpPipeParetoPathX, ilpPipeParetoPathY]);


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


ilpPipeDesignsLattice = accLattice/nreps;
ilpPipeLatticeADRS = accADRS/nreps;



disp('appliying the lattice DSE in the ilp-pipe space using the pathDSE pareto points seeds')

%get the pareto points of the PathDSE as seeds for the latticeDSE
seeds = ilpPipeConfigsPath(ilpPipeParetoPathId, :);

[ilpPipeConstLatticePath, ilpPipeMetricsLatticePath, ilpPipeIdxLatticePath, ilpPipeDesignsLatticePath] = latticeDSE(constraints, ilpPipeMetrics, seeds);

[ilpPipeParetoLatticePathX, ilpPipeParetoLatticePathY, ilpPipeParetoLatticePathId] = findPareto(ilpPipeMetricsLatticePath(:,1), ilpPipeMetricsLatticePath(:,2));


ilpPipeDesignsLatticePath = ilpPipeDesignsLatticePath - rows(seeds) + ilpPipeDesignsPath;
ilpPipeLatticePathADRS = adrs([ilpPipeParetoX, ilpPipeParetoY], [ilpPipeParetoLatticePathX, ilpPipeParetoLatticePathY]);


%---------------------------------------------------------------------------
%---------------------------------------------------------------------------
%---------------------------------------------------------------------------
rnpath = [noPipeDesignsPath; pipeDesignsPath; ilpPipeDesignsPath]
radrspath = [noPipePathADRS; pipePathADRS ; ilpPipePathADRS]

rnlattice = [noPipeDesignsLattice; pipeDesignsLattice; ilpPipeDesignsLattice]
radrslattice = [noPipeLatticeADRS; pipeLatticeADRS; ilpPipeLatticeADRS]

rnpathlattice = [noPipeDesignsLatticePath; pipeDesignsLatticePath; ilpPipeDesignsLatticePath]
radrspathlattice = [noPipeLatticePathADRS; pipeLatticePathADRS; ilpPipeLatticePathADRS]

figure(1); hold on;
plot(rnpath,radrspath, '*b')
plot(rnlattice,radrslattice, 'or')
plot(rnpathlattice,radrspathlattice, 'sk')
pause
