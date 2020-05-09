arg_list = argv();
nreps = 1;
plotFlag = false;

nopipe = strcat(arg_list{1}, '/results_full.m')
pipe = strcat(arg_list{2}, '/results_full.m')

if(numel(arg_list) == 3)
  ilppipe = strcat(arg_list{3}, '/results_full.m')
end

plotFlag = true;

%loads the scripts in the legUP/4.0/examples folder
path = strrep(mfilename('fullpath'), mfilename(), '');
addpath(path);

load(pipe);
pipeConsraints = constraints;
pipeMetrics = metrics;
pipeConfigs = configFiles;
pc = [];
pm = [];

load(nopipe);
noPipeConsraints = constraints;
noPipeMetrics = metrics;
noPipeConfigs = configFiles;
nopm = [];

if(numel(arg_list) == 3)
  load(ilppipe);
  ilpPipeConsraints = constraints;
  ilpPipeMetrics = metrics;
  ilpPipeConfigs = configFiles;
  ilppm = [];
end

%guaratee the 1-1 correspondence
if(numel(arg_list) == 2)
  common = intersect(pipeConsraints, noPipeConsraints, 'rows');
elseif(numel(arg_list) == 3)
  %a = pipeConsraints(:, 1:2) == [1 7];
  %a = a(:,1) & a(:,2);
  %pipeConsraints(a,:)

  %b = noPipeConsraints(:, 1:2) == [1 7];
  %b = b(:,1) & b(:,2);
  %noPipeConsraints(b,:)

  %c = ilpPipeConsraints(:, 1:2) == [1 7];
  %c = c(:,1) & c(:,2);
  %ilpPipeConsraints(c,:)

  common = intersect(intersect(pipeConsraints, noPipeConsraints, 'rows'), ilpPipeConsraints, 'rows');
end

%size(pipeConsraints)
%size(noPipeConsraints)
%size(ilpPipeConsraints)
%size(common)

c1 = ismember(common, pipeConsraints, 'rows');
pipeConsraints = pipeConsraints(c1, :);
pipeMetrics = pipeMetrics(c1, :);
pipeConfigs = pipeConfigs(:,c1);

c2 = ismember(common, noPipeConsraints, 'rows');
noPipeConsraints = noPipeConsraints(c2, :);
noPipeMetrics = noPipeMetrics(c2, :);
noPipeConfigs = noPipeConfigs(:, c2);

if(numel(arg_list) == 3)
  c3 = ismember(common, ilpPipeConsraints, 'rows');
  ilpPipeConsraints = ilpPipeConsraints(c3, :);
  ilpPipeMetrics = ilpPipeMetrics(c3, :);
  ilpPipeConfigs = ilpPipeConfigs(:, c3);
end

constraints = common;


ndesigns = rows(constraints);
if (plotFlag)
  fighandle = figure(1); hold on;
  plot(noPipeMetrics(:,1), noPipeMetrics(:,2), '^k')
  plot(pipeMetrics(:,1), pipeMetrics(:,2), '*b')
  if(numel(arg_list) == 3)
    plot(ilpPipeMetrics(:,1), ilpPipeMetrics(:,2), 'or')
  end
  fs = 16;
  h = legend('no-pipe', 'NIS', 'ILPS')
  set (h, 'fontsize', fs);
  xlabel('Cycles', 'fontsize', fs);
  ylabel('ALMs', 'fontsize', fs);
  mkdir('./plots')
  graphname = strcat('plots/', arg_list{2}, 'dses.eps');
  print(fighandle, char(graphname), '-color');
  hold off;
  %pause
  %return;
end
return;
%---------------------------------------------------------------------------
%---------------------------------------------------------------------------
%---------------------------------------------------------------------------

% this compares cases where the path DSE with no pipe is used to seed the Lattice DSE with pipe and vice versa, also compares with path, lattice, and path + lattice DSE over the combined sets
seedingComparisons
return;

%---------------------------------------------------------------------------
%---------------------------------------------------------------------------
%---------------------------------------------------------------------------
%this runs the Path DSE, Lattice DSE, and Path+Lattice DSE over the config without pipe, with NIS pipe and with ILP pipe

pathLatticeComparisons;
return;
