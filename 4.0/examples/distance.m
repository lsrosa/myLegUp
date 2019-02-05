arg_list = argv();
nopipe = strcat(arg_list{1}, '/results_full.m')
pipe = strcat(arg_list{2}, '/results_full.m')

load(pipe);
pipeConsraints = constraints;
pipeMetrics = metrics;
pipeConfigs = configFiles;

load(nopipe);
nopipeConsraints = constraints;
nopipeMetrics = metrics;
nopipeConfigs = configFiles;

%guaratee the 1-1 correspondence
pc = [];
pm = [];
nopm = [];
for row = 1:rows(pipeConsraints)
  if(ismember(pipeConsraints(row, :), nopipeConsraints, 'rows'))
    pc = [pc; pipeConsraints(row, :)];
    pm = [pm; pipeMetrics(row, :)];
    nopm = [nopm; nopipeMetrics(row, :)];
  end
end
pipeConsraints = pc;
pipeMetrics = pm;
nopipeMetrics = nopm;

ndesigns = rows(pipeConsraints);

% count = 0;
nmetrics = columns(pipeMetrics)
% nmetrics = 2;
partiald = zeros(nmetrics, 1);
d = 0;

for ci=1:ndesigns
  for cj=ci:ndesigns
    temp = 1;
    %count = count + 1;
    for metric=1:nmetrics
      c = (pipeMetrics(ci, metric)-pipeMetrics(cj, metric))*(nopipeMetrics(ci, metric)-nopipeMetrics(cj, metric)) >= 0;
      partiald(metric) = partiald(metric) + c;
      temp = temp*c;
    end
    %partiald
    %temp
    d = d + temp;
  end
end

%count
ndistances = ndesigns*(ndesigns+1)/2

ARPD = d/ndistances
partialARPD = partiald/ndistances
