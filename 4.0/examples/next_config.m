disp("I'm on next_config.m");
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
  currentConfigNumber = 0;
  save(dataFileName, 'constraintsNames', 'constraintsValues', 'currentConfigNumber');

  %create a new confi#.tcl and out folder, copy .bc and config into the folder
  mkdir('out.config0.tcl');
  copyfile('config0.tcl', 'out.config0.tcl');
  copyfile('./module.c', 'out.config0.tcl');

  %write which config the makefile should build
  fid = fopen('nextconfig', 'w');
  fputs(fid, strcat('config0.tcl',"\n"));
  fclose(fid);

  return;
else
  %if there is a file
  load(dataFileName);
  assert(cond == 2, 'there should be a file called partial_data.mat here');
end

%TODO here we define the heuristic to calculate the new config file
lastConstraints = constraintsValues(end,:)
[vmax,ix] = max(lastConstraints)

%heuristic reduces max value until all are one
if (vmax > 1)
  newConstraint = lastConstraints;
  newConstraint(ix) = newConstraint(ix)-1;
  constraintsValues = [constraintsValues; newConstraint];
  currentConfigNumber = currentConfigNumber+1;
  %write new constraint in the tempconfig
  nextConfigName = writeResources( tempConfigName, constraintsNames, newConstraint, currentConfigNumber );

  save(dataFileName, 'constraintsNames', 'constraintsValues', 'currentConfigNumber');

  fid = fopen('nextconfig', 'w');
  fputs(fid, strcat(nextConfigName,"\n"));
  fclose(fid);

  if (newConstraint(ix) == 1)
    disp('removing file tempconfig.tcl');
    delete('tempconfig.tcl');
  end
else
  %here we define the strop criteria
  disp('removing file tempconfig.tcl');
  delete('tempconfig.tcl');
end



return;
