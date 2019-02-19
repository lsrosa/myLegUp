function [constOut, metricsOut, idxOut, nCompiledDesigns] = pathDSE(configIn, metricsIn);
  global configs = configIn;
  global metrics = metricsIn;
  global alreadyCompiledPath = [];
  global alreadyDiscarded = [];
  global cycles = 1;
  global ALMs = 2;
  plot = false;

  configs = configIn;
  metrics = metricsIn;
  alreadyCompiledPath = [];
  alreadyDiscarded = [];

  assert(rows(configIn) == rows(metricsIn), 'input configs and metrics numbers are different');
  ndesigns = rows(configIn);

  %gets the maximum, minimum and 10%diffrence configs
  maxConfig = max(configIn);
  oneConfig = min(configIn);
  tenDiffConst = floor(0.1*double(maxConfig - oneConfig));

  %find the index of these configs
  [~, maxIdx] = ismember(maxConfig, configs, 'rows');
  [~, oneIdx] = ismember(oneConfig, configs, 'rows');

  %starts at max and walks toward one
  searchQueue = maxIdx;
  currMetrics = compileOne(maxIdx);

  if(plot)
    figure(1);
    hold on;
  end

  do
    %pop from search queue
    currIdx = searchQueue(1);
    searchQueue = searchQueue(2:end);

    %dont need to compile it, since the search queue comes from the compiled designs
    %currMetrics = compileOne(currIdx);
    currMetrics = metrics(currIdx,:);

    %get the neighbors
    neighbors = getNegNeighbors(currIdx);

    %if there are no neighbors, we need to use the 10% rule
    if( numel(neighbors) > 0 )
      %currIdx
      %configs(currIdx,:)

      %compile the neighbors (can we do this in RL?)
      neighborsMetrics = compile(currIdx, neighbors);

      %analyze the neighbors directions
      dt = currMetrics(cycles) - neighborsMetrics(:, cycles);
      da = currMetrics(ALMs) - neighborsMetrics(:, ALMs);

      for ix = 1:rows(dt)
        %same time, area improved
        if(dt(ix) == 0 && da(ix) > 0)
          searchQueue = [searchQueue; neighbors(ix)];
        %worse time, area improved
        elseif(dt(ix) < 0 && da(ix) > 0)
          searchQueue = [searchQueue; neighbors(ix)];
        %time improved, same area
        elseif (dt(ix) > 0 && da(ix) == 0)
          searchQueue = [searchQueue; neighbors(ix)];
        %time improved, worse area
        elseif (dt(ix) > 0 && da(ix) < 0)
          searchQueue = [searchQueue; neighbors(ix)];
        %same time, same area
        elseif (dt(ix) == 0 && da(ix) == 0)
          searchQueue = [searchQueue; neighbors(ix)];
        %better time and better area
        elseif (dt(ix) > 0 && da(ix) > 0)
          searchQueue = [searchQueue; neighbors(ix)];
        endif
      endfor
    end

    %searchQueue

    %no neighbor and oneConfig not reached - we are trapped in a local cluster
    %numel(searchQueue) == 0
    %~ismember(oneConfig, alreadyCompiledPath, 'rows')
    if( numel(searchQueue) == 0 && ~ismember(oneConfig, alreadyCompiledPath, 'rows') )
        newConfig = [];

        %reduce 10% until find a non-compiled and non-discarded design or the oneConfig
        newConfig = configs(currIdx,:);
        %tenDiffConst
        gt = newConfig > tenDiffConst;
        newConfig(~gt) = oneConfig(~gt);
        newConfig(gt) = newConfig(gt) - tenDiffConst(gt);
        c1 = ismember(newConfig, alreadyCompiledPath, 'rows');
        %newconfigcompiled = c1
        c2 = ismember(newConfig, alreadyDiscarded, 'rows');
        %newConfigDiscarded = c2
        c3 = ismember(newConfig, oneConfig, 'rows');
        %newConfigIsOnes = c3
        c4 = ismember(newConfig, configs, 'rows');
        %notInConfigs = c4

        while( (c1 || c2 || ~c4) && ~c3)
          gt = newConfig > tenDiffConst;
          newConfig(~gt) = oneConfig(~gt);
          newConfig(gt) = newConfig(gt) - tenDiffConst(gt);
          %oneConfig
          %newConfig
          c1 = ismember(newConfig, alreadyCompiledPath, 'rows');
          %newconfigcompiled = c1
          c2 = ismember(newConfig, alreadyDiscarded, 'rows');
          %newConfigDiscarded = c2
          c3 = ismember(newConfig, oneConfig, 'rows');
          %newConfigIsOnes = c3
          c4 = ismember(newConfig, configs, 'rows');
          %notInConfigs = c4
          %disp('in a while');
          %pause
        end
        %disp('after a while');

        %add the design to the seachQueue
        [~, ix] = ismember(newConfig, configs, 'rows');
        compileOne(ix);
        searchQueue = ix;
        %pause
    end %if( numel(neighbors) > 0 )

    %pause
    %searchQueue
    if(plot)
      [~, pltoIdx] = ismember(alreadyCompiledPath, configs, 'rows');
      plot(metrics(pltoIdx, 1), metrics(pltoIdx, 2), '*b');
      pause
    endif

    %alreadyCompiledPath
    %pause
  until( numel(searchQueue) == 0 ) %remember that until stops at true
  %until(true) %remember that until stops at true
  %disp(   strcat('total number of compiled designs = ', disp(rows(alreadyCompiledPath)) ) )

  nCompiledDesigns = rows(alreadyCompiledPath);

  if(plot)
    [~, pltoIdx] = ismember(alreadyCompiledPath, configs, 'rows');
    plot(metrics(pltoIdx, 1), metrics(pltoIdx, 2), '*b');
    figure(2)
    plot(metrics(:, 1), metrics(:, 2), '*b');
    pause
  endif

  constOut = alreadyCompiledPath;
  [~, idxOut] = ismember(alreadyCompiledPath, configs, 'rows');
  metricsOut = metrics(idxOut, :);
  %alreadyCompiledPath

end %function

%return non-compiled and non-discarded 1-neighbors reducing the constraints
function rn = getNegNeighbors(idx)
  global configs;
  global alreadyCompiledPath;
  global alreadyDiscarded;

  n = columns(configs);
  config = configs(idx, :);

  rconfigs = [];
  reduceIdxs = config > 1;
  for ci = 1:n
    if (reduceIdxs(ci) == 1)
      newConfig = config;
      newConfig(ci) = newConfig(ci)-1;
      c1 = ismember(newConfig, alreadyCompiledPath, 'rows');
      c2 = ismember(newConfig, alreadyDiscarded, 'rows');
      if( ~c1 && ~c2 )
        rconfigs = [rconfigs; newConfig];
      end
    end
  end

  %get the indexes of the configures, which will be the return of the function
  [~, rn] = ismember(rconfigs, configs, 'rows');
  %this should exclude nonexisting configs caused by compilation errors
  rn = rn(rn > 0);
end %function

%this function should be substituted by the compilation and report reads
function rMetrics = compile(curr, configsToCompile)
  global metrics configs;
  global alreadyCompiledPath alreadyDiscarded;
  global cycles ALMs;

  %simulate compiling the current design
  currMetric = metrics(curr, :);
  %pause
  %alreadyCompiledPath = [alreadyCompiledPath; configs(curr, :)];

  %simulate the compilation of the other configs
  compiled = [];
  for c = 1:rows(configsToCompile)
    %simulate compile this config
    configsMetric = metrics(configsToCompile(c),:);
    dt = currMetric(cycles) - configsMetric(cycles);
    da = currMetric(ALMs) - configsMetric(  ALMs);

    %check if the new design dominates the current one
    c1 = dt == 0 && da >  0;
    %paretoAreaDominant = c1
    c2 = dt >  0 && da == 0;
    %paretoCyclesDominant = c2
    c3 = dt == 0 && da == 0;
    %SamePoint = c3
    c4 = dt > 0 && da > 0;
    %fullDominant = c4

    %annotate which ones were compiled
    compiled = [compiled; configsToCompile(c)];

    if(c1 || c2 || c3 || c4)
      break;
    end
  end

  %alreadyCompiledPath
  alreadyCompiledPath = [alreadyCompiledPath; configs(compiled, :)];
  %discard the ones that were not compiled, happends when a dominant design is found
  discarded = setdiff(configsToCompile, compiled);
  alreadyDiscarded = [alreadyDiscarded; configs(discarded,:)];

  rMetrics = metrics(compiled,:);
  %pause
end

function rMetrics = compileOne(curr)
  global metrics configs;
  global alreadyCompiledPath alreadyDiscarded;

  %simulate compiling the current design
  currMetric = metrics(curr, :);
  alreadyCompiledPath = [alreadyCompiledPath; configs(curr, :)];

  rMetrics = currMetric;
  %pause
end
