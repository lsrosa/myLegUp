arg_list = argv();
nfiles = numel(arg_list);

if  nfiles == 0
  return;
end

%get the name of each loop and config
for i=1:nfiles
  parts = strsplit(arg_list{i}, '/');
  configFiles(i) = strrep(arg_list(i), '/hardware_estimation.mat', '');
  configFiles(i) = strrep(configFiles(i), 'out.', '');

  configNames(i) = strcat(parts(3), parts(4));
  %makes nomes shorter for printing purposes
  configNames(i) = strrep(configNames(i), 'loop', 'l');
  configNames(i) = strrep(configNames(i), 'out.', '');
  configNames(i) = strrep(configNames(i), 'config', 'c');
  configNames(i) = strrep(configNames(i), '.tcl', '');
end
%configNames
%configFiles

%first iteration just to take the measures
load(arg_list{1});
%measures;
nmeasures = numel(measures);
metrics = zeros(nfiles, nmeasures);
metrics(1,:) = values;

%get the measures for the rest of the loops
for i=2:nfiles
  load(arg_list{i});
  metrics(i,:) = values;
end

%readConfigs
%read one of them to get the resources
fid = fopen(configFiles{i});
res = cell(); con = [];
while((line=fgets(fid)) != -1)
  parts = strsplit(line, ' ');
  %skip some constraints that are not resource
  if(numel(parts) != 3)
    continue;
  end
  %select only resource constraints
  if(strcmp(parts(1), 'set_resource_constraint')==1)
    res = [res, parts(2)];
    con = [con, str2num(parts{3})];
  end
end

resources = res;
constraints = zeros(nfiles, numel(con));
constraints(1,:) = con;
fclose(fid);

%read the rest
for i=2:nfiles
  fid = fopen(configFiles{i});
  con = [];
  while((line=fgets(fid)) != -1)
    parts = strsplit(line, ' ');
    %skip some constraints that are not resource
    if(numel(parts) != 3)
      continue;
    end
    %select only resource constraints
    if(strcmp(parts(1), 'set_resource_constraint')==1)
      con = [con, str2num(parts{3})];
    end
  end
  constraints(i,:) = con;
  fclose(fid);
end
nres = numel(resources);
%resources
%constraints

finalConstraints = [];
finalResources = cell();

for i=1:nres
  v = constraints(:,i);
  %are all elements the same?
  if( !all(v == v(1)) )
    finalConstraints = [finalConstraints, v];
    finalResources = [finalResources; resources(i)];
  end
end

%finalConstraints
%finalResources

outFolder = strcat(strsplit(arg_list{i}, '/')(1), '/', 'plots');
mkdir(outFolder);

finalNres = numel(finalResources)
winsize = get(0,'screensize');
winsize = [1 1 winsize(4) winsize(3)];
fighandle = figure(1,'position',winsize); hold on;

for i=nmeasures:-1:1%column
  for j=finalNres:-1:1;%row
    indexes = [i j j+(i-1)*finalNres]
    subplot(nmeasures, finalNres, j+(i-1)*finalNres);
    plot(finalConstraints(:,j), metrics(:,i), 'ob');
    xlabel(finalResources(j));
    ylabel(measures(i));
  end
end

graphname = strcat(outFolder, '/correlations.jpg');
print(fighandle, char(graphname), '-djpg');
