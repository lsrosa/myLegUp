function [constOut, metricsOut, idxOut, nCompiledDesigns] = latticeDSE(configIn, metricsIn, configSeeds = []);
  global configsLattice = configIn;
  global metricsLattice = metricsIn;
  global alreadyCompiledLattice = [];
  global cycles = 1;
  global ALMs = 2;
  plot = false;
  popCoeff = 0.01;
  alph = 0.05;
  global sigma = 0.1;
  bet = alph;

  configsLattice = configIn;
  metricsLattice = metricsIn;
  alreadyCompiledLattice = [];

  assert(rows(configsLattice) == rows(metricsIn), 'input configs and metrics numbers are different');
  ndesigns = rows(configsLattice);

  %gets the maximum, minimum and 10%diffrence configsLattice
  global maxConfig;
  global oneConfig;
  global radii;
  global radii2;

  maxConfig = max(configsLattice);
  oneConfig = min(configsLattice);
  radii = ceil(sigma*double(maxConfig-oneConfig));
  radii2 = radii.^2;

  %maxConfig
  %configsLattice
  %find the index of these configsLattice
  [~, maxIdx] = ismember(maxConfig, configsLattice, 'rows');
  [~, oneIdx] = ismember(oneConfig, configsLattice, 'rows');

  if(numel(configSeeds) > 0)
    configs2Compile = configSeeds;
    originalSize = rows(configSeeds);
  else
    originalSize = 0;
    %according to Ferretti:2018b Cluster-Based Heuristic  ...
    %configsLattice
    %pause
    nseeds = max(round(popCoeff*double(rows(configsLattice))), 1);
    configs2Compile = [];

    while(rows(configs2Compile) < nseeds)
      rang = double(maxConfig - oneConfig);
      probs = betainv(rand(size(oneConfig)), alph, bet);
      newconf = oneConfig + round(probs.*rang);
      if( ~ismember(newconf, configs2Compile, 'rows'))
        %newconf
        configs2Compile = [configs2Compile; newconf];
        %size(configs2Compile)
      end
      %if(mod(rows(configs2Compile), 10)==0)
      %  pause
      %end

    end
    %configs2Compile
  end


  while( numel(configs2Compile) ~= 0 )
    %compile designs
    [~, configs2CompileIdx] = ismember(configs2Compile, configsLattice, 'rows');
    copiledMetrics = compile(configs2CompileIdx(configs2CompileIdx > 0));
    [~, allCompiledIdxs] = ismember(alreadyCompiledLattice, configsLattice, 'rows');

    %get pareto points
    x = metricsLattice(allCompiledIdxs,cycles);
    y = metricsLattice(allCompiledIdxs,ALMs);
    %sx = size(x)
    %sy = size(y)
    [px, py, pIdx] = findPareto(x, y);
    %spx = size(px)
    %spy = size(py)
    %paretoConfigs = configsLattice(allCompiledIdxs(pIdx), :)

    %get pareto neighbors, which have not been compiled yet
    paretoIdx = allCompiledIdxs(pIdx);
    neighbors = getNeighbors(paretoIdx);
    configs2Compile = configsLattice(neighbors, :);

    %disp( strcat('partial number of compiled designs: ', disp(rows(allCompiledIdxs)-originalSize)));
    %allCompiledIdxs
    %pause
  end

  %until(true) %remember that until stops at true
  %disp(   strcat('total number of compiled designs = ', disp(rows(alreadyCompiledLattice)-originalSize) ) )
  nCompiledDesigns = rows(alreadyCompiledLattice);%-originalSize;

  constOut = alreadyCompiledLattice;
  [~, idxOut] = ismember(alreadyCompiledLattice, configsLattice, 'rows');
  metricsOut = metricsLattice(idxOut, :);

end %function

%return non-compiled and non-discarded 1-neighbors reducing the constraints
function rn = getNeighbors(idx)
  global configsLattice;
  global alreadyCompiledLattice;
  global alreadyDiscarded;
  global maxConfig;
  global oneConfig;
  global sigma;
  global radii;
  global radii2;

  %inputs = configsLattice(idx, :)
  %inputs = configsLattice(idx, radii2 > 0)
  %radii2
  %we only count the resources which vary
  vcols = radii2 > 0;
  rconfigs = [];

  %we will calculate a ellipsoid centered in the current config, and get all points inside this ellipsoid between all configs. This causes some redundancy, which will be elliminated later, but allows to use a lot o matrix algebra, which gives us better runtime.
  %we use an ellipsoid since we did not normalize the configuration spcae
  for ci = 1:numel(idx)
    centre = configsLattice(idx(ci), :);

    %creates a matrix with the center repeated
    centreMatrix = ones(size(configsLattice))*diag(centre);

    diffs = configsLattice - centreMatrix;
    diffs2 = diffs.^2;

    %d2 = diffs2(:, vcols)
    %r2 = radii2(:, vcols)

    %caculate the \sum{(x-x0)^2/(r^2)}
    coefs = sum(diffs2(:, vcols)./radii2(:, vcols), 2);

    %eliminates the same point (distance == 0) and only gets the ones inside the ellipsoid
    rconfigs = [rconfigs; configsLattice(coefs > 0 & coefs < 1, :)];
    %pause
  end

  %n = columns(configsLattice);
  %rconfigs = [];
  %increaseIdxs = maxConfig > 1;
  %for ridx = 1:numel(idx)
  %  config = configsLattice(idx(ridx), :);
  %  reduceIdxs = config > 1;
  %  for ci = 1:n
  %    if(rang(ci) > 0)
  %      for ri=1:rang(ci)
  %        newConfigUp = config;
  %        newConfigDown = config;
  %        newConfigUp(ci) = newConfigUp(ci)+ri;
  %        newConfigDown(ci) = newConfigDown(ci)-ri;
  %        if(newConfigDown(ci) == 0)
  %          newConfigDown(ci) = 1;
  %        end
  %        rconfigs = [rconfigs; newConfigUp; newConfigDown];
  %      end
  %    end
  %
  %    %if (increaseIdxs(ci) == 1 && config(ci) < maxConfig(ci))
  %    %  newConfig = config;
  %    %  newConfig(ci) = newConfig(ci)+1;
  %    %  c1 = ismember(newConfig, alreadyCompiledLattice, 'rows');
  %    %  if( ~c1 )
  %    %    rconfigs = [rconfigs; newConfig];
  %    %  end
  %    %end
  %  end
  %end

  %rconfigs
  %pause
  %removes configs out of the configuration space
  c1 = ismember(rconfigs, configsLattice, 'rows');
  rconfigs = rconfigs(c1 == 1,:);

  %removes configs that were already compiled
  c1 = ~ismember(rconfigs, alreadyCompiledLattice, 'rows');
  rconfigs = rconfigs(c1 == 1,:);

  %rconfigs
  %get the indexes of the configures, which will be the return of the function
  %I know this is redundant but it is faster
  [~, rnIdx] = ismember(rconfigs, configsLattice, 'rows');
  %this should exclude nonexisting configsLattice caused by compilation errors
  rn = unique(rnIdx(rnIdx > 0));

  %pause
end %function

%this function should be substituted by the compilation and report reads
function rMetrics = compile(c)
  global metricsLattice configsLattice;
  global alreadyCompiledLattice;

  alreadyCompiledLattice = [alreadyCompiledLattice; configsLattice(c,:)];
  rMetrics = metricsLattice(c,:);
end
