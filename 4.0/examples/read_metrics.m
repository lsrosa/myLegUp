disp('I am supposed to read the compilation results and save them in the partial_data.mat file')

tempConfigName = 'tempconfig.tcl';
dataFileName = 'partial_data.mat';
%pwd
arg_list = argv();
metricsFileName = strcat(arg_list{1}, '/hardware_estimation.mat')
%pause

cond = exist(dataFileName, 'file');

assert(cond ~= 0, 'there should be a partial_data.mat file here');

%contains: constraintsNames, constraintsValues, currentConfigNumber
dataFile = load(dataFileName);
constraintsNames = dataFile.constraintsNames;
constraintsValues = dataFile.constraintsValues;
currentConfigNumber = dataFile.currentConfigNumber;
state = dataFile.state;
compileQueue = dataFile.compileQueue;
searchQueue = dataFile.searchQueue;
partialMetricValues = dataFile.partialMetricValues;
partialConstraintsValues = dataFile.partialConstraintsValues;
discardedConstraints = dataFile.discardedConstraints;
maxResources = dataFile.maxResources;
flag10 = dataFile.flag10;

%contains: measures, values
metricsFile = load(metricsFileName);
currentMetrics = metricsFile.measures;
currentMetricsValues = metricsFile.values;

%check if there is only one config
firstConfigCond = rows((dataFile.constraintsValues)) == 1;
%add conditions to check if we added the metrics already
isfield(dataFile,'metrics');
isfield(dataFile, 'metricsValues');
noMetricsCondition = ~isfield(dataFile,'metrics') && ~isfield(dataFile, 'metricsValues');


%if this is the first time and metrics are not in the file
if( firstConfigCond && noMetricsCondition )
  metrics = currentMetrics;
  metricsValues = currentMetricsValues;
else
  assert(~noMetricsCondition, 'there should be metrics in the partial_data.mat file');
  metrics = dataFile.metrics;
  metricsValues = dataFile.metricsValues;
  if(flag10)
    metricsValues = [metricsValues; currentMetricsValues];
    flag10 = false;
  else
    partialMetricValues = [partialMetricValues; currentMetricsValues];
  end
end

save(dataFileName, 'constraintsNames', 'constraintsValues', 'currentConfigNumber', 'metrics', 'metricsValues', 'state', 'compileQueue', 'searchQueue', 'partialMetricValues', 'partialConstraintsValues', 'discardedConstraints', 'maxResources', 'flag10');

%this is for debug only
data2 = load(dataFileName);


%pause
return
