arg_list = argv();
nfiles = numel(arg_list);

if  nfiles < 2
  return;
end

%TODO this is a temporary workaround
%first argument should be "fast" or "full" just to distinguish the graphs
type = arg_list{end};
nfiles = nfiles - 1;

%get the name of each loop and config
for i=1:nfiles
  parts = strsplit(arg_list{i}, '/');
  configname = strrep(parts(4), 'out.', '');
  configFiles(i) = strrep(arg_list(i), 'hardware_estimation.mat', configname);
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
configFiles{1}
fid = fopen(configFiles{1})
pwd
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
for i=1:nfiles
  readconfig=configFiles{i};
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

filename = strcat('results_', type, '.m')
save(filename, 'constraints', 'metrics', 'configFiles')
return;

variableConstraints = [];
variableResources = cell();

if (rows(constraints)==1)
  variableConstraints = constraints;
  variableResources = resources;
else
  for i=1:nres
    v = constraints(:,i);
    %are all elements the same?
    if( !all(v == v(1)) )
      variableConstraints = [variableConstraints, v];
      variableResources = [variableResources; resources(i)];
    end
  end
end
variableNres = numel(variableResources);

%variableConstraints
%variableResources

%create some clusters
metrics = metrics(:, 1:2);
nmeasures = 2;

resourcesK = ceil(max(max(variableConstraints))*1.1);
%resourcesK = 4;%this line helps to control the number of clusters
[h1 h2] = hist(metrics(:,1),resourcesK)
startCycles = h2(h1!=0)'
%startCycles = [442.27 543.36 695.00 947.73]'%that's cheating
cyclesK = numel(startCycles)

%resourcesK = 2;%this line helps to control the number of clusters
[h1 h2] = hist(metrics(:,2:end),resourcesK)
metricsK = ceil(mean(sum(h1!=0)))
%metricsK = 2;%this line helps to control the nuber of clusters
for i=2:nmeasures
  [h1 h2] = hist(metrics(:,i),metricsK)
  startMetrics(:,i-1) = h2(h1!=0)
end
[metricsK, ~] = size(startMetrics)
%metricsK = 2;%this line helps to control the number of clusters
%startCycles
%startMetrics
%cyclesK
%metricsK

k = cyclesK*metricsK
startA = zeros(k, nmeasures);
for i=1:metricsK
  startA((i-1)*cyclesK+1:(i)*cyclesK, :) = [startCycles, repmat(startMetrics(i,:), cyclesK, 1)];
end

function  [assignments, centers] = myKmeans(X, k, centers = 0, maxiter = 200)
	if (centers == 0)
		centerRows = randperm(size(X)(1));
		centers = X(centerRows(1:k), :);
	endif
	numOfRows = length(X(:,1));
	numOfFeatures = length(X(1,:));
	assignments = ones(1, numOfRows);

	for iter = 1:maxiter
		clusterTotals = zeros(k, numOfFeatures);
		clusterSizes = zeros(k, 1);
		for rowIx = 1:numOfRows
			minDist = realmax;
			assignTo = 0;
			for centerIx = 1:k
				% Euclidian distance is used.
				dist = sqrt(sum((X(rowIx, : ) - centers(centerIx, :)).^2));
				if dist < minDist
					minDist = dist;
					assignTo = centerIx;
				endif
			endfor
			assignments(rowIx) = assignTo;

			% Keep these information to calculate cluster centers.
			clusterTotals(assignTo, :) += X(rowIx, :);
			clusterSizes(assignTo)++;
		endfor

    % pushing close clusters far from each other
    d = zeros(k);
    %centers
    %get distances between centers
    for centerIx = 1:k
      d(:, centerIx) = distancePoints(centers, centers(centerIx, :));
    end

    maxD = max(max(d))
    %putting inf in distance to itself
    d(d==0) = Inf;
    %getting the smallest values
    minD = min(min(d))

    if (minD < 0.002*maxD)
      [minRow, minCol] = find(d==minD);
      %get just a pair os clusters that are close
      c1 = minRow(1);
      c2 = minCol(1);

      % assing all points of c2 to c1
      clusterTotals(c1, :) = clusterTotals(c1, :) + clusterTotals(c2, :);
      clusterSizes(c1) = clusterSizes(c1) + clusterSizes(c2);
      assignments(assignments==c2)=c1;
      clusterSizes(c2) = 0; %to trigger singleton
    end

    % This process is called 'singleton' in terms of Matlab.
		% If a cluster is empty choose a random data point as new
		% cluster cener.
    % modified to choose a point which is furthest from its center
		for clusterIx = 1:k
			if (clusterSizes(clusterIx) == 0)
        disp('singleton cluster!')
        distances = zeros(numOfRows, 1);
        for rowIx=1:numOfRows
          distances(rowIx) = distancePoints(X(rowIx), centers(assignments(rowIx)));
        end
        %distances
        [~, farRowIx] = max(distances);

				%randomRow = round(1 + rand() * (numOfRows - 1) );
        randomRow = farRowIx;
				clusterTotals(clusterIx, :) =  X(randomRow, :);
				clusterSizes(clusterIx) = 1;
        assignments(randomRow) = clusterIx;
			endif
		endfor

		newCenters = zeros(k, numOfFeatures);
		for centerIx = 1:k
			newCenters(centerIx, :) = clusterTotals(centerIx, : ) / clusterSizes(centerIx);
		endfor

		diff = sum(sum(abs(newCenters - centers)));

		if diff < eps
			disp('Centers are same, which means we converged before maxiteration count. This is a good thing!')
			break;
		endif

		centers = newCenters;
	endfor
	assignments = assignments';
	%printf('iter: %d, diff: %f\n', iter, diff);
endfunction

%[idx, centers, sumd, dist] = kmeans(metrics, k, 'Replicates', 1000000000, 'MaxIter', 10000000, 'emptyaction', 'singleton');
[idx, centers] = myKmeans(metrics, k, startA, 50000);


%row is cluste and column is resource type
constraintsMean = zeros(k, variableNres);
constraintsStd = zeros(k, variableNres);

clusterText = cell();
for i=1:k
  c = variableConstraints(idx==i, :);
  if (numel(c)==0)
    constraintsMean(i, :) = NaN;
    constraintsStd(i, :) = NaN;
  elseif (rows(c) ==1)
    constraintsMean(i, :) = c;
    constraintsStd(i, :) = zeros(size(c));
  else
    constraintsMean(i, :) = mean(c);
    constraintsStd(i, :) = std(c);
  end
  %constraintsMean(i, :)
  %constraintsStd(i, :)

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

outFolder = strcat(strsplit(arg_list{1}, '/')(1), '/', 'plots');
mkdir(outFolder);

%winsize = get(0,'screensize');
%winsize = [1 1 winsize(4) winsize(3)];
marks = cellstr(['or'; 'ob'; 'ok'; 'sb'; 'sk'; '*b'; '*k'; '.b'; '.k'; '^b'; '^k'; 'or'; 'ob'; 'ok'; 'sb'; 'sk'; '*b'; '*k'; '.b'; '.k'; '^b'; '^k']);

[~, sortedCyclesIdx] = sort(centers(:,1));

plotcond = 1;
if(plotcond == 1)
  %[metrics idx]
  %sort(unique(idx))
  %sortedCenterIdx

  for i=1:nmeasures-1
    fighandle = figure(i); hold on;

    for j=sortedCyclesIdx'
      %j
      plot(metrics(idx==j,1), metrics(idx==j, i+1), '.b');
      dp = distancePoints([metrics(idx==j,1), metrics(idx==j, i+1)], [centers(j, 1), centers(j, i+1)] );
      radius = max(dp);
      if (numel(radius) != 0)
        drawCircle (centers(j, 1), centers(j, i+1), radius, 'r');
        plot(centers(j, 1), centers(j, i+1), '*r');
      end
      %text(centers(j, 1), centers(j, i+1), num2cell(j));
      title(legendText);
    end

    figure(2)
    plot(metrics(:,1), metrics(:,2), '.b');

    %pause

    xlabel(measures(1));
    ylabel(measures(i+1));

    graphname = strcat(outFolder, '/Corr', measures{i+1}, '_', type, '.jpg');
    print(fighandle, char(graphname), '-djpg');
  end
end

%print table as latex
filename = strcat(outFolder, '/clusterAvgConstraints.tex');
fid = fopen(filename{1}, "w");

%\begin{tabulary}{\textwidth}{|c|c|c|c|c|c|}
fprintf(fid, "\\begin\{tabulary\}\{\\textwidth\}\{|c|");
for j=1:variableNres
fprintf(fid, "c|");
end
fprintf(fid, "\}\n");

%table header
fprintf(fid, "\\hline\nCluster ");
for j=1:variableNres
  tx = strrep(variableResources{j}, '_', ' ');
  tx = strrep(tx, 'unsigned', 'u ');
  tx = strrep(tx, 'signed', 's ');
  fprintf(fid, " & %s", tx);
endfor
fprintf(fid, " \\\\\\hline\n");

%printing latex table with average constraints for clusters
%sotting according to cycles
%constraintsMean
%constraintsStd
for j=sortedCyclesIdx'
  if (!isnan(constraintsMean(j, 1)))
    fprintf(fid, "%.2f, %.2f", centers(j, 1), centers(j, 2));
    for l=1:variableNres
      fprintf(fid, " & %.2f $\\pm$ %.2f", constraintsMean(j, l), constraintsStd(j, l));
    endfor
    fprintf(fid, " \\\\\\hline \n");
  endif
endfor

%\end{tabulary}
fprintf(fid, "\\end\{tabulary\}");
fclose(fid);

close all
clear all
