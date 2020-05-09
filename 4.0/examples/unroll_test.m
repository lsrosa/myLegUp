arg_list = argv();

base = strcat(arg_list{1}, '/results_full.m')
unr = strcat(arg_list{2}, '/results_full.m')

load(base);
baseConstraints = constraints;
baseMetrics = metrics;

load(unr);
unrConsraints = constraints;
unrMetrics = metrics;

[~, coveredIdx] = ismember(baseConstraints, unrConsraints, 'rows');
coveredMetrics = unrMetrics(coveredIdx, :);

uncoveredConstraints = setdiff(unrConsraints, baseConstraints, 'rows');
[~, uncoveredIdx] = ismember(uncoveredConstraints, unrConsraints, 'rows');
uncoveredMetrics = unrMetrics(uncoveredIdx, :);

fighandle = figure(1); hold on;
ms = 12;
plot(baseMetrics(:,1), baseMetrics(:,2), '^k', "markersize", ms);
plot(coveredMetrics(:,1), coveredMetrics(:,2), '*b', "markersize", ms);
plot(uncoveredMetrics(:,1), uncoveredMetrics(:,2), 'or', "markersize", ms);

fs = 16;
h = legend('uf=1', 'covered uf=2', 'missed uf=2')
set (h, 'fontsize', fs);
xlabel('Cycles', 'fontsize', fs);
ylabel('ALMs', 'fontsize', fs);
mkdir('./plots')
graphname = strcat('plots/unrollTest.eps');
print(fighandle, char(graphname), '-color');
hold off;
pause;
return;
