arg_list = argv();

%loads the scripts in the legUP/4.0/examples folder
path = strrep(mfilename('fullpath'), mfilename(), '');
addpath(path);

ilp = strcat(arg_list{1}, '-ilp/results_full.m');
sdc = strcat(arg_list{1}, '-sdc/results_full.m');
gas = strcat(arg_list{1}, '-gas/results_full.m');
nis = strcat(arg_list{1}, '-nis/results_full.m');

load(ilp);
ilpConstraints = constraints;
ilpMetrics = metrics;

load(sdc);
sdcConsraints = constraints;
sdcMetrics = metrics;

load(gas);
gasConsraints = constraints;
gasMetrics = metrics;

load(nis);
nisConsraints = constraints;
nisMetrics = metrics;

common = intersect(ilpConstraints, sdcConsraints, 'rows');
common = intersect(common, gasConsraints, 'rows');
common = intersect(common, nisConsraints, 'rows');

[~, id] = ismember(common, ilpConstraints, 'rows');
ilpConstraints = ilpConstraints(id, :);
ilpMetrics = ilpMetrics(id, :);
[~,~,pid] = findPareto(ilpMetrics(:, 1), ilpMetrics(:, 2));
ilpParetoConstraints = ilpConstraints(pid,:);
ilpParetoMetrics = ilpMetrics(pid, :);

[~, id] = ismember(common, sdcConsraints, 'rows');
sdcConsraints = sdcConsraints(id, :);
sdcMetrics = sdcMetrics(id, :);
[~,~,pid] = findPareto(sdcMetrics(:, 1), sdcMetrics(:, 2));
sdcParetoConsraints = sdcConsraints(pid,:);
[~, ilpId] = ismember(sdcParetoConsraints, ilpConstraints, 'rows');
sdcParetoMetrics = ilpMetrics(ilpId, :);

[~, id] = ismember(common, gasConsraints, 'rows');
gasConsraints = gasConsraints(id, :);
gasMetrics = gasMetrics(id, :);
[~,~,pid] = findPareto(gasMetrics(:, 1), gasMetrics(:, 2));
gasParetoConsraints = gasConsraints(pid,:);
[~, ilpId] = ismember(gasParetoConsraints, ilpConstraints, 'rows');
gasParetoMetrics = ilpMetrics(ilpId, :);

[~, id] = ismember(common, nisConsraints, 'rows');
nisConsraints = nisConsraints(id, :);
nisMetrics = nisMetrics(id, :);
[~,~,pid] = findPareto(nisMetrics(:, 1), nisMetrics(:, 2));
nisParetoConsraints = nisConsraints(pid,:);
[~, ilpId] = ismember(nisParetoConsraints, ilpConstraints, 'rows');
nisParetoMetrics = ilpMetrics(ilpId, :);

tp = 100*[
  adrs(ilpParetoMetrics, sdcParetoMetrics);
  adrs(ilpParetoMetrics, gasParetoMetrics);
  adrs(ilpParetoMetrics, nisParetoMetrics);
]
