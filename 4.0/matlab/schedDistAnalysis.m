pkg load statistics;
arg_list = argv();
nreps = 50;
plotFlag = false;
nmetrics = 2;
cycles = 1;
alms = 2;

basename = strsplit(arg_list{1}, '-')(2);
for ns=2:numel(arg_list)
  name = strsplit(arg_list{ns}, '-')(2);
  leg(ns-1) = strcat(name);
end
%leg
%pause}

benchname = strsplit(arg_list{1}, '-')(1);
filenames(1) = strcat(benchname, '-ilp');
filenames(2) = strcat(benchname, '-sdc');
filenames(3) = strcat(benchname, '-gas');
filenames(4) = strcat(benchname, '-nis');
%filenames

%loads the scripts in the legUP/4.0/examples folder
%path = strrep(mfilename('fullpath'), mfilename(), '');
%addpath(path);

file = strcat(filenames{1}, '/results_full.m');
load(file);
baseConstraints = constraints;
baseMetrics = metrics;
baseConfigs = configFiles;
common = baseConstraints;

file = cell();
constraints = cell();
metrics = cell();
configs = cell();
for ns=2:numel(filenames)
  files{ns-1} = strcat(filenames{ns}, '/results_full.m');
  load(files{ns-1});
  otherConstraints{ns-1} = constraints;
  otherMetrics{ns-1} = metrics;
  otherConfigs{ns-1} = configFiles;

  %makes the intersection of all benchmarks
  common = intersect(common, otherConstraints{ns-1}, 'rows');
end

%final values for the constraints
constraints = common;

c = ismember(constraints, baseConstraints, 'rows');
baseMetrics = baseMetrics(c,:);
baseConfigs = baseConfigs(c);

for ns=2:numel(filenames)
  c = ismember(constraints, otherConstraints{ns-1}, 'rows');
  otherMetrics{ns-1} = otherMetrics{ns-1}(c, :);
  otherConfigs{ns-1} = otherConfigs{ns-1}(c);
end

%otherMetrics
%otherConfigs
dists = zeros(rows(constraints), numel(filenames)-1);

for ns=1:numel(filenames)-1
  filenames{ns+1}
  %dists(:,ns) = geomean(double(baseMetrics(:,:) - otherMetrics{ns}(:,:)+1));
  %idx = prod(otherMetrics{ns}(:,1:2), 2) < prod(baseMetrics(:,1:2), 2);
  %[otherMetrics{ns}(idx,1:2) baseMetrics(idx,1:2)]
  dists(:,ns) = prod(otherMetrics{ns}(:,1:2), 2)./prod(baseMetrics(:,1:2), 2);
  %r = [baseMetrics(:, 1:2)'; otherMetrics{ns}(:, 1:2)'; dists(:, ns)'];
  %r(:, r(end,:)<1);
end


% get the len of the configs
%dists
%constraints

x = sum(constraints, 2)
s1 = ['.b'; '.r'; '.k']
s2 = ['-b'; '-r'; '-k']

figure(1); hold on;

c = dists
%c(:, 3) = c(:, 3)/1.4

yy = 4
for sche = 1:3
  pp = polyfit(x,c(:,sche),3);
  ux = unique(x);
  plot(ux, polyval(pp, ux), s2(sche,:))
end
ylim([0 yy])
for sche = 1:3
  s1(sche,:)
  plot(x, c(:,sche), s1(sche,:), "markersize", 15)
end

legend('sdcs', 'gas', 'nis')

pause
return;



[y, x]= hist(dists)
fighandle = figure(1); hold on;
c = x>0.8
%sort(y(c), 'descend')
y
%y(2:9,3) = y(9:-1:2,3)
%y(2:9,2) = y(4:-1:2,2)
%y(c, 3) = sort(y(c,3), 'descend')
%y(3,3) = 109
%y(4,3) = 23
%x = 0.9*x

tickIdx = any(y, 2);
bar(x, y);
set(gca,'xtick',x(tickIdx));
%num2str(round(100*x)/100)
fs = 14;
set(gca,'xticklabel',num2str(round(100*x(tickIdx))/100), 'fontsize', fs);
h = legend(leg);
set (h, 'fontsize', fs);
l = xlabel('AL', 'fontsize', fs);
ylabel('# designs', 'fontsize', fs);

%xlim([0.99 1.098]) %mt
%xlim([0.42 3.3]) %dv
%xlim([0.5 max(x)*1.1]) %fat
%xlim([0.42 2.9]) %cp
xlim([.2 5]) %ac
%pause
mkdir('./plots');
graphname = strcat('plots/', strsplit(filenames{ns}, '-')(1), '-schedDistHist.eps');
print(fighandle, char(graphname), '-color');
hold off;

pause;
return;

fighandle = figure(2); hold on;
marks = {'*', 'o', '^'};
for ns=1:numel(filenames)-1
  d2{ns} = dists(:,ns);
  plot(d2{ns}(:), otherMetrics{ns}(:,2), marks(ns));
end

h = legend(leg, 'location', 'northwest');
set (h, 'fontsize', fs);
xlabel('AL', 'fontsize', fs);
ylabel('area', 'fontsize', fs);
graphname = strcat('plots/', strsplit(filenames{ns}, '-')(1), '-cycle-alms-corr.eps');
print(fighandle, char(graphname), '-color');
hold off;

pause
return;
