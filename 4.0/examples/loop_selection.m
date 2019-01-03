arg_list = argv();
nfiles = numel(arg_list);

if  nfiles < 2
  return;
end

%TODO this is a temporary workaround
%first argument should be "fast" or "full" just to distinguish the graphs
type = arg_list{end}
nfiles = nfiles - 1;

%get the name of each loop and config
for i=1:nfiles
  parts = strsplit(arg_list{i}, '/');
  configNames(i) = strcat(parts(3), parts(4));
  %makes nomes shorter for printing purposes
  configNames(i) = strrep(configNames(i), 'loop', 'l');
  configNames(i) = strrep(configNames(i), 'out.', '');
  configNames(i) = strrep(configNames(i), 'config', 'c');
  configNames(i) = strrep(configNames(i), '.tcl', '');
end

%configNames

%first iteration just to take the measures
load(arg_list{1});
%measures;
%nmeasures = numel(measures);
nmeasures = 2;
vals = zeros(nfiles, nmeasures);
vals(1,1:nmeasures) = values(1:nmeasures);

%get the measures for the rest of the loops
for i=2:nfiles
  load(arg_list{i});
  vals(i,1:nmeasures) = values(1:nmeasures);
end

function [paretoPoints] =  findPareto(x, y)
  paretoPoints = [];
  cond = zeros(numel(x), 1);
  x = x';
  y = y';

  for i=1:numel(x)
    c1 = x < x(i) & y < y(i);
    c2 = x == x(i) & y < y(i);
    c3 = x < x(i) & y == y(i);
    cond = sum(  c1 | c2 | c3 );
    if(cond == 0)
      paretoPoints = [paretoPoints; x(i) y(i)];
    end
  end

  paretoPoints = paretoPoints';
  return;
end

%vals
outFolder = strcat(strsplit(arg_list{i}, '/')(1), '/', 'plots');
mkdir(outFolder);

%eliminate duplicates but concatenate the string names
uniqueVals = [];
uniqueConfigNames = [];
rows = zeros(nfiles,1);

for i=1:nfiles
  if(rows(i) == 0)
    %comparing just cycles and ALMs for now -- change vals(i, 1:2) by  vals(i, :) to compare all metrics
    indexes = sum(vals(:, 1:nmeasures) == vals(i,1:nmeasures), 2) == nmeasures;
    uniqueVals = [uniqueVals; vals(i,1:nmeasures)]; %add only on instance

    processNames = configNames(indexes);

    %creates an array with unique names
    pairs = zeros(numel(processNames), 2);
    for j=1:numel(processNames)
      pn = strrep(processNames{j}, 'l', '');
      pn = strrep(pn, 'c', ' ');
      pairs(j,:) = strread(pn, '%d');
    end
    %pairs

    %pairs is a two integers array with loop and config number
    [C,~,ic] = unique(pairs(:,1));
    jointNames = cell();
    for j=1:numel(C)
      n = strrep(strcat(num2str(pairs(ic==1,2)')), '  ', ',');
      jointNames(j) = strcat('l', num2str(C(j)), 'c', n);
    end
    %jointNames
    %pause

    %join all names in a big string
    finalName = '';
    for j=1:numel(jointNames);
      finalName = strcat(finalName, jointNames{j});
    end
    %finalName

    %add only on instance
    uniqueConfigNames = [uniqueConfigNames; finalName];
    rows = rows+indexes;
  end
end

%verifications
%vals
%configNames
%uniqueVals
%uniqueConfigNames
finalValues = uniqueVals;
finalConfigNames = uniqueConfigNames;
%finalValues = vals
%finalConfigNames = configNames

finalValues
finalConfigNames
for i=1:nmeasures-1
  fighandle = figure(i); hold on;
  plot(finalValues(:,1), finalValues(:, i+1), '.b');
  %text(finalValues(:,1), finalValues(:, i+1), finalConfigNames);
  xlabel(measures(1));
  ylabel(measures(i+1));
  ppoints = findPareto(finalValues(:,1), finalValues(:, i+1))
  if(numel(ppoints) > 0)
    pspeed = finalValues(:,1) == ppoints(:,1)';
    parea = finalValues(:,i+1) == ppoints(:,2)';
    match = pspeed & parea;
    for cf = 1:columns(match)
      finalConfigNames(match(:,cf), :)
    end

    px = ppoints(:,1);
    py = ppoints(:,2);
    [px,idx] = sort(px);
    py = py(idx);
    plot(px, py, '*r-');
    text(px(1), py(1), 'speed');
    text(px(end), py(end), 'area');
  end

  graphname = strcat(outFolder, '/', measures{i+1}, '_', type, '.jpg');
  print(fighandle, char(graphname), '-djpg');
end
