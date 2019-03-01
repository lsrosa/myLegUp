function [constOut, metricsOut, idxOut, nCompiledDesigns] = pathDSE(configIn, metricsIn);
  global configsPath;
  global metricsPath;
  global alreadyCompiledPath;
  global alreadyDiscarded;
  global cycles = 1;
  global ALMs = 2;
  plot = false;

  configsPath = configIn;
  metricsPath = metricsIn;
  alreadyCompiledPath = [];
  alreadyDiscarded = [];

  assert(rows(configsPath) == rows(metricsPath), 'input configsPath and metricsPath numbers are different');
  ndesigns = rows(configsPath);

  %gets the maximum, minimum   and 10%diffrence
  %a = configsPath(:, 1:2) == [1 7]
  %a = a(:,1) & a(:,2)
  %configsPath(a,:)
  maxConfig = max(configsPath);
  oneConfig = min(configsPath);
  tenDiffConst = ceil(0.1*double(maxConfig - oneConfig));
  %pause

  %find the index of these configsPath
  [~, maxIdx] = ismember(maxConfig, configsPath, 'rows');
  [~, oneIdx] = ismember(oneConfig, configsPath, 'rows');
  %pause
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
    %currMetrics = compileOne(currIdx);;
    currMetrics = metricsPath(currIdx,:);

    %get the neighbors
    neighbors = getNegNeighbors(currIdx);

    %if there are no neighbors, we need to use the 10% rule
    if( numel(neighbors) > 0 )
      %currIdx
      %configsPath(currIdx,:)

      %compile the neighbors (can we do this in RL?)
      neighborsMetrics = compile(currIdx, neighbors);

      %analyze the neighbors directions
      dt = currMetrics(cycles) - neighborsMetrics(:, cycles);
      da = currMetrics(ALMs) - neighborsMetrics(:, ALMs);

      %this is an option for testing which approach is better
      discardQueue = false;
      if(discardQueue)
        for ix = 1:rows(dt)
          %same time, area improved
          if(dt(ix) == 0 && da(ix) > 0)
            searchQueue = [neighbors(ix)];
          %worse time, area improved
          elseif(dt(ix) < 0 && da(ix) > 0)
            searchQueue = [searchQueue; neighbors(ix)];
          %time improved, same area
          elseif (dt(ix) > 0 && da(ix) == 0)
            searchQueue = [neighbors(ix)];
          %time improved, worse area
          elseif (dt(ix) > 0 && da(ix) < 0)
            searchQueue = [searchQueue; neighbors(ix)];
          %same time, same area
          elseif (dt(ix) == 0 && da(ix) == 0)
            searchQueue = [neighbors(ix)];
          %better time and better area
          elseif (dt(ix) > 0 && da(ix) > 0)
            searchQueue = [neighbors(ix)];
          endif
        endfor
      else %if(discardQueue)
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
    end


    if(~discardQueue)
      %TODO - filter search queue and remove non pareto points between it
      %disp('discarding non pareto points in the search queue')
      if(rows(searchQueue) > 1)
        %prevsq = searchQueue
        %metricsPath(searchQueue, :)
        sqx = metricsPath(searchQueue, cycles);
        sqy = metricsPath(searchQueue, ALMs);
        [~, ~, sqIdx] = findPareto(sqx, sqy);
        searchQueue = searchQueue(sqIdx);

        %if(rows(prevsq) ~= rows(searchQueue))
        %  disp('Hey check out this new search queue');
        %  pause
        %end
      end
    end

    %searchQueue
    %pause
    %no neighbor and oneConfig not reached - we are trapped in a local cluster
    %numel(searchQueue) == 0
    %~ismember(oneConfig, alreadyCompiledPath, 'rows')
    if( numel(searchQueue) == 0 && ~ismember(oneConfig, alreadyCompiledPath, 'rows') )
        newConfig = [];

        %reduce 10% until find a non-compiled and non-discarded design or the oneConfig
        newConfig = configsPath(currIdx,:);
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
        c4 = ismember(newConfig, configsPath, 'rows');
        %notInConfigs = c4

        while( (c1 || c2 || ~c4) && ~c3)
          %tenDiffConst
          %oneConfig
          gt = newConfig > tenDiffConst;
          newConfig(~gt) = oneConfig(~gt);
          newConfig(gt) = newConfig(gt) - tenDiffConst(gt);
          %newConfig
          c1 = ismember(newConfig, alreadyCompiledPath, 'rows');
          %newconfigcompiled = c1
          c2 = ismember(newConfig, alreadyDiscarded, 'rows');
          %newConfigDiscarded = c2
          c3 = ismember(newConfig, oneConfig, 'rows');
          %newConfigIsOnes = c3
          c4 = ismember(newConfig, configsPath, 'rows');
          %notInConfigs = c4
          %disp('in a while');
          %newConfig
          %pause
        end
        %disp('after a while');

        %add the design to the seachQueue
        [~, ix] = ismember(newConfig, configsPath, 'rows');
        compileOne(ix);
        searchQueue = ix;
        %pause
    end %if( numel(neighbors) > 0 )

    %pause
    %searchQueue
    if(plot)
      [~, pltoIdx] = ismember(alreadyCompiledPath, configsPath, 'rows');
      plot(metricsPath(pltoIdx, 1), metricsPath(pltoIdx, 2), '*b');
      pause
    endif

    %alreadyCompiledPath
    %alreadyDiscarded
    %assert(sum(ismember(alreadyDiscarded, alreadyCompiledPath, 'rows')) == 0, 'aaaaaaaaaaaaa')
    %pause
  until( numel(searchQueue) == 0 ) %remember that until stops at true
  %until(true) %remember that until stops at true
  %disp(   strcat('total number of compiled designs = ', disp(rows(alreadyCompiledPath)) ) )

  nCompiledDesigns = rows(alreadyCompiledPath);

  if(plot)
    [~, pltoIdx] = ismember(alreadyCompiledPath, configsPath, 'rows');
    plot(metricsPath(pltoIdx, 1), metricsPath(pltoIdx, 2), '*b');
    figure(2)
    plot(metricsPath(:, 1), metricsPath(:, 2), '*b');
    pause
  endif

  constOut = alreadyCompiledPath;
  [~, idxOut] = ismember(alreadyCompiledPath, configsPath, 'rows');
  metricsOut = metricsPath(idxOut, :);
  %alreadyCompiledPath

end %function

%return non-compiled and non-discarded 1-neighbors reducing the constraints
function rn = getNegNeighbors(idx)
  global configsPath;
  global alreadyCompiledPath;
  global alreadyDiscarded;

  n = columns(configsPath);
  config = configsPath(idx, :);

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
  [~, rn] = ismember(rconfigs, configsPath, 'rows');
  %this should exclude nonexisting configsPath caused by compilation errors
  rn = rn(rn > 0);
end %function

%this function should be substituted by the compilation and report reads
function rMetrics = compile(curr, configsToCompile)
  global metricsPath configsPath;
  global alreadyCompiledPath alreadyDiscarded;
  global cycles ALMs;

  %simulate compiling the current design
  currMetric = metricsPath(curr, :);
  %pause
  %alreadyCompiledPath = [alreadyCompiledPath; configsPath(curr, :)];
  %metricsPath(configsToCompile,:);
  %simulate the compilation of the other configsPath
  compiled = [];
  for c = 1:rows(configsToCompile)
    %simulate compile this config
    configsMetric = metricsPath(configsToCompile(c),:);
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
  alreadyCompiledPath = [alreadyCompiledPath; configsPath(compiled, :)];
  %discard the ones that were not compiled, happends when a dominant design is found
  discarded = setdiff(configsToCompile, compiled);
  alreadyDiscarded = [alreadyDiscarded; configsPath(discarded,:)];

  rMetrics = metricsPath(compiled,:);
  %pause
end

function rMetrics = compileOne(curr)
  global metricsPath configsPath;
  global alreadyCompiledPath alreadyDiscarded;

  %simulate compiling the current design
  currMetric = metricsPath(curr, :);
  alreadyCompiledPath = [alreadyCompiledPath; configsPath(curr, :)];

  rMetrics = currMetric;
  %pause
end
