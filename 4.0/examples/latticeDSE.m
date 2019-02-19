function [constOut, metricsOut, idxOut, nCompiledDesigns] = latticeDSE(configIn, metricsIn, configSeeds = []);
  global configsLattice = configIn;
  global metricsLattice = metricsIn;
  global alreadyCompiledLattice = [];
  global cycles = 1;
  global ALMs = 2;
  plot = false;
  global sigma = 0.25;

  configsLattice = configIn;
  metricsLattice = metricsIn;
  alreadyCompiledLattice = [];

  assert(rows(configIn) == rows(metricsIn), 'input configs and metrics numbers are different');
  ndesigns = rows(configIn);

  %gets the maximum, minimum and 10%diffrence configsLattice
  global maxConfig = max(configIn);
  global oneConfig = min(configIn);

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
    nseeds = ceil(0.05*double(rows(configsLattice)));
    configs2Compile = [];

    while(rows(configs2Compile) < nseeds)
      range = double(maxConfig - oneConfig);
      alpha = 0.5;
      beta = alpha;
      probs = betainv(rand(size(oneConfig)), alpha, beta);
      newconf = oneConfig + round(probs.*range);
      if( ~ismember(newconf, configs2Compile, 'rows'))
        %newconf
        configs2Compile = [configs2Compile; newconf];
        %size(configs2Compile)
      end
      %if(mod(rows(configs2Compile), 10)==0)
      %  pause
      %end

    end
  end

  %configs2Compile

  while( numel(configs2Compile) ~= 0 ) %remember that until stops at true

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

  range = ceil(sigma*double(maxConfig-oneConfig));

  n = columns(configsLattice);

  rconfigs = [];
  increaseIdxs = maxConfig > 1;

  for ridx = 1:numel(idx)
    config = configsLattice(idx(ridx), :);
    reduceIdxs = config > 1;
    for ci = 1:n
      if(range(ci) > 0)
        for ri=1:range(ci)
          newConfigUp = config;
          newConfigDown = config;
          newConfigUp(ci) = newConfigUp(ci)+ri;
          newConfigDown(ci) = newConfigDown(ci)-ri;
          rconfigs = [rconfigs; newConfigUp; newConfigDown];
        end
      end

      %if (increaseIdxs(ci) == 1 && config(ci) < maxConfig(ci))
      %  newConfig = config;
      %  newConfig(ci) = newConfig(ci)+1;
      %  c1 = ismember(newConfig, alreadyCompiledLattice, 'rows');
      %  if( ~c1 )
      %    rconfigs = [rconfigs; newConfig];
      %  end
      %end
    end
  end

  %rconfigs
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
