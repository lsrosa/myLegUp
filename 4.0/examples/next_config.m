disp('I am on next_config.m');
disp(pwd)
tempConfigName = 'tempconfig.tcl';
dataFileName = 'partial_data.mat';

cond = exist(dataFileName, 'file');

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

  save(dataFileName, 'constraintsNames', 'constraintsValues', 'currentConfigNumber', 'state', 'compileQueue', 'searchQueue', 'partialMetricValues', 'partialConstraintsValues');

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

  %we need to treat dt == 0 and da == 0 cases
  for ix=1:rows(dt)
    %same or worse time, area improved
    if(dt(ix) <= 0 && da(ix) > 0)
      %gw(ix) = Inf;
      searchQueue = [searchQueue; nconfigs+ix];
    %time improved, same or worse area
    elseif (dt(ix) > 0 && da(ix) <= 0)
      %gw(ix) = Inf;
      searchQueue = [searchQueue; nconfigs+ix];
    %same time, same area
    elseif (dt(ix) == 0 && da(ix) == 0)
      %gw(ix) = Inf;
      searchQueue = [searchQueue; nconfigs+ix];
    %same time, area degraded
    elseif (dt(ix) == 0 && da(ix) < 0)
      %gw(ix) = -Inf;
    %time degraded, same area
    elseif (dt(ix) < 0 && da(ix) == 0)
      %gw(ix) = -Inf;
    %time degraded, area degraded
    elseif (dt(ix) < 0 && da(ix) < 0)
      %gw(ix) = -Inf;
    else
      %gw(ix) = da(ix)/dt(ix);
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

  %TODO might be in the wrong place
  %if searchQueue is empty we should stop
  if ( rows(searchQueue) == 0 )
    disp('removing file tempconfig.tcl');
    delete('tempconfig.tcl');
    return;
  end

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

    %add all possible designs to a compileQueue
    exploreResources = currConfig > 1
    for resIdx=1:numel(constraintsNames)
      %check if this resource is up to variation
      if(exploreResources(resIdx) == 1)
        newConstraint = currConfig;
        newConstraint(resIdx) = newConstraint(resIdx)-1;

        %check is the new constraint has not been searched before
        if(~ismember(newConstraint, constraintsValues, 'rows'))
          compileQueue = [compileQueue; newConstraint];
        end
      end
    end
  until(rows(compileQueue) > 0)
  compileQueue

  if(rows(compileQueue)>0)
    state = 'spread'
  else
    state = 'quit'
  end
  %pause
end

% spread subtracts one from each resource to evaluate the next step
if(strcmpi(state,'spread'))
  disp('I am at spread');

  compileQueue
  if(rows(compileQueue) > 0)
    %get first
    currConstraint = compileQueue(1,:);
    %pop the compileQueue
    if(rows(compileQueue) == 1)
      compileQueue = [];
    else
      compileQueue = compileQueue(2,:);
    end
  else
    disp('there is something wrong with the compileQueue')
    assert(rows(compileQueue) >= 0 && 'compileQueue size is not positive')
  end

  %if all spread configs were generated
  if(rows(compileQueue) == 0)
    state = 'nextStep'
  end

  constraintsValues
  compileQueue
  partialConstraintsValues = [partialConstraintsValues; currConstraint]

  nextConfigName = writeResources(tempConfigName, constraintsNames, currConstraint, rows(constraintsValues)+rows(partialConstraintsValues))

  %saving states and data for makefile
  %TODO I should do all this inside Octave
  save(dataFileName, 'constraintsNames', 'constraintsValues', 'currentConfigNumber', 'metrics', 'metricsValues', 'state', 'compileQueue', 'searchQueue', 'partialMetricValues', 'partialConstraintsValues');

  fid = fopen('nextconfig', 'w');
  fputs(fid, strcat(nextConfigName,"\n"));
  fclose(fid);

  %pause
  return;
end
