function [rn, radrs] = separateSetsDSE(constraints, set1, set2, nreps = 1)
  totalInputSet = [set1; set2];

  %get the paretos over the combined sets
  [px, py, idx] = findPareto(totalInputSet(:,1), totalInputSet(:,2));
%------------------------------------------------------------------------

  %apply the path DSE on both sets
  [pc1, pm1, pid1, pn1] = pathDSE(constraints, set1);
  [pc2, pm2, pid2, pn2] = pathDSE(constraints, set2);

  [ppx, ppy, ppid] = findPareto([pm1(:,1); pm2(:,1)], [pm1(:,2); pm2(:,2)]);

  pathN = pn1 + pn2;
  pathADRS = adrs([px, py], [ppx, ppy]);
%------------------------------------------------------------------------

accLattice = 0;
accADRS = 0;
for cnt = 1:nreps
  %apply the lattice DSE on both sets
  [c1, m1, id1, n1] = latticeDSE(constraints, set1);
  [c2, m2, id2, n2] = latticeDSE(constraints, set2);

  [ppx, ppy, ppid] = findPareto([m1(:,1); m2(:,1)], [m1(:,2); m2(:,2)]);

  accLattice = accLattice + n1 + n2;
  accADRS = accADRS + adrs([px, py], [ppx, ppy]);
  %pause
end

%make an average
latticeN = accLattice/nreps;
latticeADRS = accADRS/nreps;

%------------------------------------------------------------------------
  %get seeds from path on set 1 and 2
  [ppx1, ppy1, ppid1] = findPareto(pm1(:,1), pm1(:,2));
  [ppx2, ppy2, ppid2] = findPareto(pm2(:,1), pm2(:,2));
  seeds1 = pc1(ppid1, :);
  seeds2 = pc2(ppid2, :);

  %apply the seeded lattice DSE with seeds on both sets
  [c1, m1, id1, n1] = latticeDSE(constraints, set1, seeds1);
  [c2, m2, id2, n2] = latticeDSE(constraints, set2, seeds2);

  [ppx, ppy, ppid] = findPareto([m1(:,1); m2(:,1)], [m1(:,2); m2(:,2)]);

  seedLatticeN = n1 + n2 - rows(seeds1) - rows(seeds2) + pathN;
  seedLatticeADRS = adrs([px, py], [ppx, ppy]);

%------------------------------------------------------------------------

  rn = [pathN; latticeN; seedLatticeN];
  radrs = [pathADRS; latticeADRS; seedLatticeADRS];
end
