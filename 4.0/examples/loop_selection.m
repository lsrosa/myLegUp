arg_list = argv();
nfiles = numel(arg_list);

if  nfiles == 0
  return;
end

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

configNames

%first iteration just to take the measures
load(arg_list{1})
%measures;
nmeasures = numel(measures);
vals = zeros(nfiles, nmeasures);
vals(1,:) = values;

%get the measures for the rest of the loops
for i=2:nfiles
  load(arg_list{i});
  vals(i,:) = values;
end

%vals
outFolder = strcat(strsplit(arg_list{i}, '/')(1), '/', 'plots')
mkdir(outFolder)

for i=1:numel(measures)-1
  fighandle = figure(i);
  plot(vals(:,1), vals(:, i+1), '.b');
  text(vals(:,1), vals(:, i+1), configNames);
  xlabel(measures(1));
  ylabel(measures(i+1));
  graphname = strcat(outFolder, '/', measures{i+1}, '.jpg')
  print(fighandle, char(graphname), '-djpg');
end
