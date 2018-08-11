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

variableConstraints = [];
variableResources = cell();

for i=1:nres
  v = constraints(:,i);
  %are all elements the same?
  if( !all(v == v(1)) )
    variableConstraints = [variableConstraints, v];
    variableResources = [variableResources; resources(i)];
  end
end
variableNres = numel(variableResources);
%variableConstraints
%variableResources

%create some clusters
k = ceil(max(max(variableConstraints))*1.1);
[h1 h2] = hist(metrics(:,1),k);
startA = h2(h1!=0)';
k = numel(startA);

%[idx, centers, sumd, dist] = kmeans(metrics(:,1), k, 'Start', 'cluster', 'emptyaction', 'singleton', 'Replicates', k, 'MaxIter', 200, 'Distance', 'correlation');
[idx, centers, sumd, dist] = kmeans(metrics(:,1), k, 'A', startA, 'emptyaction', 'singleton', 'MaxIter', 200, 'Distance', 'correlation');
centers
minDist = min(dist')';

threshhold = 1;
maxIterations=10;

cnt = 0;
min(centers)
while( any(minDist > threshhold*min(centers)) )
  ping = strcat("kmeans iteration", num2str(cnt))
  [idx, centers, sumd, dist] = kmeans(metrics(:,1), k, 'Start', 'cluster', 'emptyaction', 'singleton', 'Replicates', k, 'MaxIter', 200, 'Distance', 'correlation');
  %[idx, centers, sumd, dist] = kmeans(metrics(:,1), k, 'A', startA, 'emptyaction', 'singleton', 'MaxIter', 200, 'Distance', 'correlation');
  centers
  minDist = min(dist')';

  %security exit
  cnt = cnt+1;
  if(cnt >= maxIterations)
    break;
  end
end

%row is cluste and column is resource type
constraintsMean = zeros(k, variableNres);
constraintsStd = zeros(k, variableNres);
%variableConstraints
clusterText = cell();
for i=1:k
  c = variableConstraints(idx==i, :)
  if(numel(c)==0)
    constraintsMean(i, :) = NaN;
    constraintsStd(i, :) = NaN;
  else
    constraintsMean(i, :) = mean(c);
    constraintsStd(i, :) = std(c);
  end
  text = '';
  for j=1:variableNres
     text = strcat(text, 'r', num2str(j), ': ', num2str(constraintsMean(i, j)), '\pm', num2str(constraintsStd(i, j)), "\n");
  end
  clusterText(i) = text;
end

legendText = '';
for i=1:variableNres
  legendText = strcat(legendText, 'r', num2str(i), ': ', variableResources(i), "\n");
end
legendText = strrep(legendText, '_', ' ');
%legendText
%clusterText
%variableResources
%centers
%constraintsMean
%constraintsStd

outFolder = strcat(strsplit(arg_list{i}, '/')(1), '/', 'plots');
mkdir(outFolder);

%winsize = get(0,'screensize');
%winsize = [1 1 winsize(4) winsize(3)];
marks = cellstr(['or'; 'ob'; 'ok'; 'sr'; 'sb'; 'sk'; '*r'; '*b'; '*k'; '.r'; '.b'; '.k'; '^r'; '^b'; '^k']);

plotcond = 1;
if(plotcond == 1)
for i=1:numel(measures)-1
  fighandle = figure(i); hold on;
  miny = 0.9*min(min(metrics(:, i+1)));
  maxy = 1.1*max(max(metrics(:, i+1)));

  for j=1:k
    plot(metrics(idx==j,1), metrics(idx==j, i+1), marks(j));
    plot([centers(j), centers(j)], [miny, maxy], '-r');
    xp = centers;
    yp = maxy*ones(size(centers));
    %text(xp(1), yp(1), clusterText(1));
    %gtext(clusterText{j});
    title(legendText);
  end

  xlabel(measures(1));
  ylabel(measures(i+1));

  graphname = strcat(outFolder, '/Corr', measures{i+1}, '.jpg');
  print(fighandle, char(graphname), '-djpg');
end
end

%print table as latex
[~, idxs] = sort(centers);
filename = strcat(outFolder, '/clusterAvgConstraints.txt');
fid = fopen(filename{1}, "w");

fprintf(fid, "Cluster ");
for j=1:variableNres
  fprintf(fid, " & %s", variableResources{j});
end
fprintf(fid, " ///hline\n");

%constraintsMean
for j=idxs'
  fprintf(fid, "%.2f", centers(j));
  for l=1:variableNres
     fprintf(fid, " & %.2f \\pm %.2f", constraintsMean(j, l), constraintsStd(j, l));
  end
  fprintf(fid, " ///hline \n");
end
fclose(fid);
