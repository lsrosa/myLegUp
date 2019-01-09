disp('I am on next_config.m');
disp(pwd)
tempConfigName = 'tempconfig.tcl';
dataFileName = 'partial_data.mat';
masterPause = false;

cond = exist(dataFileName, 'file');

function [paretoPoints, idx] =  findPareto(x, y)
  paretoPoints = [];
  idx = [];
  cond = zeros(numel(x), 1);
  %x = x';
  %y = y';

  for i=1:numel(x)
    c1 = x < x(i) & y < y(i);
    c2 = x == x(i) & y < y(i);
    c3 = x < x(i) & y == y(i);
    cond = sum(  c1 | c2 | c3 );
    if(cond == 0)
      paretoPoints = [paretoPoints; x(i) y(i)];
      idx = [idx; i];
    end
  end

  %paretoPoints = paretoPoints';
  return;
end

function [ names, vals ] = readResources( tempConfigName )
  fid = fopen(tempConfigName);

  line = fgets(fid);
  constraintsNames = cell();
  constraintsValues = [];
  while (line ~= -1)
    consStrings = strread(line, '%s');
    consValues = strread(line, '%d');

    %we are only interested in resources constraints now
    if ( strcmp(consStrings(1), 'set_resource_constraint') == 1 )
      constraintsValues = [constraintsValues,  consValues(end)];
      constraintsNames = [constraintsNames; consStrings(2)];
    end
    line = fgets(fid);
  end

  fclose(fid);
  names = constraintsNames;
  vals = constraintsValues;
end  % function

function [newConfigName] = writeResources( tempConfigName, names, vals, number )
  fid = fopen(tempConfigName);
  fidtemp = fopen('temp', 'w');

  line = fgets(fid);

  nConstraints = numel(names);
  cnt = 1;

  while (line ~= -1)
    consStrings = strread(line, '%s');
    consValues = strread(line, '%d');

    %we are only interested in resources constraints now
    if ( strcmp(consStrings{1}, 'set_resource_constraint') == 1 )

      if ( consValues(end) ~= vals(cnt) )
        %change the resource constraint and write in the temp file
        newConstraintString = cstrcat("set_resource_constraint ", names{cnt}, " ", mat2str(vals(cnt)), "\n");
        fputs(fidtemp, newConstraintString);
      else
        %a resource constraint line that does not change
        fputs(fidtemp, line);
      end

      cnt = cnt + 1;
    else
      %not a resource constrint line
      fputs(fidtemp, line);
    end

    line = fgets(fid);
  end

  assert(cnt == nConstraints+1, 'Should have written the same number of constraints');

  fclose(fid);
  fclose(fidtemp);
  % move the temp file to the temporary config
  movefile('temp', tempConfigName);

  %create a new confi#.tcl and out folder, copy .bc and config into the folder
  newConfigName = strcat('config', mat2str(number), '.tcl');
  copyfile(tempConfigName, newConfigName);
  newOutFolder = strcat('out.', newConfigName);
  mkdir(newOutFolder);
  copyfile(newConfigName, newOutFolder);
  copyfile('./module.c', newOutFolder);
end  % function


%this reads the very first config.tcl
if (cond == 0) %this means there is no file
  [constraintsNames, constraintsValues] = readResources(tempConfigName);
  currentConfigNumber = 1;
  state = 'spread';
  exploreResources = constraintsValues > 1;
  compileQueue = [];
  searchQueue = [];
  partialMetricValues = [];
  partialConstraintsValues = [];
  discardedConstraints = [];
  flag10 = false;
  maxResources = constraintsValues;

  %create a new confi#.tcl and out folder, copy .bc and config into the folder
  mkdir('out.config1.tcl');
  copyfile('config0.tcl', 'config1.tcl');
  copyfile('config0.tcl', 'out.config1.tcl/config1.tcl');
  copyfile('./module.c', 'out.config1.tcl');

  %write which config the makefile should build
  fid = fopen('nextconfig', 'w');
  fputs(fid, strcat('config1.tcl',"\n"));
  fclose(fid);

  currConfig = constraintsValues(currentConfigNumber,:)
  %add all possible designs to a queue
  disp('adding new configs to the compileQueue')
  for resIdx=1:numel(constraintsNames)
    %check if this resource is up to variation
    if(exploreResources(resIdx) == 1 && currConfig(resIdx) > 1)
      newConstraint = currConfig;
      newConstraint(resIdx) = newConstraint(resIdx)-1
      compileQueue = [compileQueue; newConstraint];
    end
  end

  save(dataFileName, 'constraintsNames', 'constraintsValues', 'currentConfigNumber', 'state', 'compileQueue', 'searchQueue', 'partialMetricValues', 'partialConstraintsValues', 'discardedConstraints', 'maxResources', 'flag10');

  compileQueue
  %pause
  return;
else
  %if there is a file
  assert(cond == 2, 'there should be a file called partial_data.mat here');
  load(dataFileName);
end

% here we define the heuristic to calculate the new config file
% get the current config
currConfig = constraintsValues(currentConfigNumber,:)
currMetrics = metricsValues(currentConfigNumber,:)
nconfigs = rows(constraintsValues)

%we dont need to compile the next step since it has already been compiled in the spread, thus, we just need to fix the state data
if (strcmpi(state,'nextStep'))
  disp('I am at next step');

  partialConstraintsValues
  partialMetricValues

  %TODO get automatically those indexes
  cycles = 1;
  ALMs = 2; %TODO use all metrics distances
  dt = currMetrics(cycles) - partialMetricValues(:, cycles)
  da = currMetrics(ALMs) - partialMetricValues(:, ALMs)
  %gw = zeros(size(dt));

  %we need to treat dt == 0 and/or da == 0 cases
  for ix=1:rows(dt)
    %same time, area improved
    if(dt(ix) == 0 && da(ix) > 0)
      searchQueue = [searchQueue; nconfigs+ix];
    %worse time, area improved
    elseif(dt(ix) < 0 && da(ix) > 0)
      searchQueue = [searchQueue; nconfigs+ix];
    %time improved, same area
    elseif (dt(ix) > 0 && da(ix) == 0)
      searchQueue = [searchQueue; nconfigs+ix];
    %time improved, worse area
    elseif (dt(ix) > 0 && da(ix) < 0)
      searchQueue = [searchQueue; nconfigs+ix];
    %same time, same area
    elseif (dt(ix) == 0 && da(ix) == 0)
      searchQueue = [searchQueue; nconfigs+ix];
    %same time, area degraded
    elseif (dt(ix) == 0 && da(ix) < 0)
      disp('oro? 1');
    %time degraded, same area
    elseif (dt(ix) < 0 && da(ix) == 0)
      disp('oro? 2');
    %time degraded, area degraded
    elseif (dt(ix) < 0 && da(ix) < 0)
      disp('oro? 3');
    else
      disp('oro? 4');
    end
  end

  %gw
  % actually selects the next configs
  %vmax, imax] = max(gw)

  %put all the partial data in the main structs
  searchQueue
  constraintsValues = [constraintsValues; partialConstraintsValues]
  metricsValues = [metricsValues; partialMetricValues]

  partialConstraintsValues = [];
  partialMetricValues = [];

  %TODO evaluate conditions to add the 10% desing in the queue
  onesConstraint = ones(size(maxResources))
  c1 = ismember(onesConstraint, constraintsValues, 'rows')
  c2 = ismember(onesConstraint, discardedConstraints, 'rows')
  %pause
  notCOnes = c1 || c2
  %pause

  %might be in the wrong place , but its working
  %if searchQueue is empty we should stop
  if ( rows(searchQueue) == 0 )
    constraintsValues
    if(~notCOnes)

      %newConstraint = constraintsValues(currentConfigNumber,:)
      % here we get the pareto points between the points already covered and create a new constraint based on them.
      cycles = 1;
      ALMs = 2; %TODO use all metrics distances
      %find pareto
      [pp, pIdx] = findPareto(metricsValues(:, cycles), metricsValues(:, ALMs))
      %sort the points according to Cycles,  we want the point with less resources and more cycles (the last of the sorted pareto points)
      [~, sortedPIdx] = sort(pp(:, 1)) %1 is the first metric we selected in the paretoPoints function
      pIdx = pIdx(sortedPIdx)
      %TODO we should check if the point is already discarded or covered, in this case, we should try to generate another point
      newConstraint = constraintsValues(pIdx(end),:)
      %pause

      exploreResources = newConstraint > 1
      diff = ceil(0.1*newConstraint)

      for resIdx=1:numel(newConstraint)
        %disp('pause outside if')
        %pause
        if(exploreResources(resIdx) == 1)
          %disp('pause inside if')
          %pause
          newConstraint(resIdx) - diff(resIdx);
          newValue = newConstraint(resIdx) - diff(resIdx);
          if(newValue >= 1)
            newConstraint(resIdx) = newValue;
          else
            newConstraint(resIdx) = 1;
          end
          assert(newConstraint(resIdx) >= 1 && 'new constraint should be >= 1 when adding the 10% rule');
        end
      end
      newConstraint
      constraintsValues = [constraintsValues; newConstraint]
      searchQueue = rows(constraintsValues)

      %add all possible designs to a queue
      currConfig = newConstraint
      exploreResources = currConfig > 1
      for resIdx=1:numel(constraintsNames)
        %check if this resource is up to variation
        if(exploreResources(resIdx) == 1)
          newConstraint = currConfig;
          newConstraint(resIdx) = newConstraint(resIdx)-1
          compileQueue = [compileQueue; newConstraint];
        end
      end

      compileQueue
      nextConfigName = writeResources(tempConfigName, constraintsNames, newConstraint, rows(constraintsValues))
      currentConfigNumber = rows(constraintsValues)
      state = 'spread';
      flag10 = true

      save(dataFileName, 'constraintsNames', 'constraintsValues', 'currentConfigNumber', 'metrics', 'metricsValues', 'state', 'compileQueue', 'searchQueue', 'partialMetricValues', 'partialConstraintsValues', 'discardedConstraints', 'maxResources', 'flag10');

      fid = fopen('nextconfig', 'w');
      fputs(fid, strcat(nextConfigName,"\n"));
      fclose(fid);

      %pause
      return;
    else
      disp('removing file tempconfig.tcl');
      delete('tempconfig.tcl');
      return;
    end
  end

  %this loop pop the searchQueue and creates configs varying the constraints
  do
    disp('popping from searchQueue');

    if(rows(searchQueue) == 0)
      disp('nothing else to search');
      disp('removing file tempconfig.tcl');
      delete('tempconfig.tcl');
      return;
      %break;
    end

    currentConfigNumber = searchQueue(1)
    if(rows(searchQueue) > 1)
      searchQueue = searchQueue(2:end);
    else
      searchQueue = [];
    end

    currConfig = constraintsValues(currentConfigNumber,:)
    searchQueue

    %selects constraints to be reduced
    exploreResourcesDown = currConfig > 1
    %selects constraints to be increased
    exploreResourcesUp = currConfig < maxResources & maxResources > 1

    for resIdx=1:numel(constraintsNames)
      %check if this resource is up to variation
      if(exploreResourcesDown(resIdx) == 1)
        newConstraint = currConfig;
        newConstraint(resIdx) = newConstraint(resIdx)-1;

        newConstraintDown = newConstraint

        %check is the new constraint has not been searched before
        c1 = ~ismember(newConstraint, constraintsValues, 'rows');
        AlreadyCompiledDown = c1
        c2 = ~ismember(newConstraint, discardedConstraints, 'rows');
        AlreadydiscardedDown = c2
        if(c1 && c2)
          compileQueue = [compileQueue; newConstraint];
        end
      end

      %setting this false for some tests
      if(false)
        %this adds the increasing resources
        if(exploreResourcesUp(resIdx) == 1)
          newConstraint = currConfig;
          newConstraint(resIdx) = newConstraint(resIdx)+1;

          newConstraintUP = newConstraint

          %check is the new constraint has not been searched before
          c1 = ~ismember(newConstraint, constraintsValues, 'rows');
          AlreadyCompiledUp = c1
          c2 = ~ismember(newConstraint, discardedConstraints, 'rows');
          AlreadydiscardedUp = c2
          if(c1 && c2)
            compileQueue = [compileQueue; newConstraint];
          end
        end
      end
    end

    rows(compileQueue)
    %pause
  until(rows(compileQueue) > 0)

  compileQueue
  rows(compileQueue)

  if(rows(compileQueue) > 0)
    state = 'spread'
  else
    state = 'quit'
  end

  if (masterPause)
    pause
  end
end

% spread subtracts one from each resource to evaluate the next step
if(strcmpi(state,'spread'))
  disp('I am at spread');

  %the logic to unschedule nodes should comes there.
  %lastConstraint = the last compiled config
  %compare with currConfig  and currMetrics

  dominanceFlag = false;
  if( rows(partialMetricValues) > 0 )
    %TODO get automatically those indexes
    cycles = 1;
    ALMs = 2; %TODO use all metrics distances
    currMetrics
    partialMetricValues
    dt = currMetrics(cycles) - partialMetricValues(end, cycles)
    da = currMetrics(ALMs) - partialMetricValues(end, ALMs)

    %These are the cases where we should unschedule the parent and other children points
    c1 = dt == 0 && da >  0;
    paretoAreaDominant = c1
    c2 = dt >  0 && da == 0;
    paretoCyclesDominant = c2
    c3 = dt == 0 && da == 0;
    SamePoint = c3
    %same time, area improved
    if(c1 || c2 || c3)
      %this flag will control the next_config generation
      dominanceFlag = true;

      %this serves to check if configs were discarded up in "nextStep"
      discardedConstraints = [discardedConstraints; compileQueue]

      %unschedule by emptying the compile queue
      compileQueue = [];
      disp('emptying compileQueue since dominant or same point has been found');
      %TODO at this point a compilation will occur in the upper makefile, thus we need to go back at "nextStep" -- GOD forbbids me to use a GOTO =(
    end
    %pause
  end
  discardedConstraints
  compileQueue

  if(rows(compileQueue) > 0)
    %get first
    lastConstraint = compileQueue(1,:);
    %pop the compileQueue
    if(rows(compileQueue) == 1)
      compileQueue = [];
    else
      compileQueue = compileQueue(2:end,:);
    end
  else
    disp('there is something wrong with the compileQueue')
    %assert(rows(compileQueue) >= 0 && 'compileQueue size is not positive')
  end
  %if all spread configs were generated
  compileQueue
  rows(compileQueue)

  if(rows(compileQueue) == 0)
    state = 'nextStep'
  end

  %if we emptyied the compileQueue we skip creating a new config.tcl
  if(~dominanceFlag)
    %pause
    constraintsValues
    compileQueue
    partialConstraintsValues = [partialConstraintsValues; lastConstraint]
    nextConfigName = writeResources(tempConfigName, constraintsNames, lastConstraint, rows(constraintsValues)+rows(partialConstraintsValues))
  end

  if (masterPause)
    pause
  end
  %saving states and data for makefile
  %TODO I should do all this inside Octave
  save(dataFileName, 'constraintsNames', 'constraintsValues', 'currentConfigNumber', 'metrics', 'metricsValues', 'state', 'compileQueue', 'searchQueue', 'partialMetricValues', 'partialConstraintsValues', 'discardedConstraints', 'maxResources', 'flag10');

  %if we emptyied the compileQueue we write a dummy for the upper level Makefile skip compilation
  if(dominanceFlag)
    fid = fopen('dummy', 'w');
    fputs(fid, strcat("dummy","\n"));
  else
    fid = fopen('nextconfig', 'w');
    fputs(fid, strcat(nextConfigName,"\n"));
  end
  fclose(fid);

  if (masterPause)
    pause
  end
  return;
end
