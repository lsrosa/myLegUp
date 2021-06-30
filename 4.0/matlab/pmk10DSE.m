function [constOut, metricsOut, idxOut, nCompiledDesigns] = pmk10DSE(configIn, metricsIn);
  global configsPath;
  global metricsPath;
  global alreadyCompiledPath;
  global cycles = 1;
  global ALMs = 2;
  plot = false;

  configsPath = configIn;
  metricsPath = metricsIn;
  alreadyCompiledPath = [];
  alreadyDiscarded = [];

  assert(rows(configsPath) == rows(metricsPath), 'input configsPath and metricsPath numbers are different');
  ndesigns = rows(configsPath);

  %gets the maximum, minimum   and 10%diffrence
  maxConfig = max(configsPath);
  oneConfig = min(configsPath);
  tenDiffConst = ceil(0.1*double(maxConfig - oneConfig));
  %pause

  %find the index of these configsPath
  [~, maxIdx] = ismember(maxConfig, configsPath, 'rows');
  [~, oneIdx] = ismember(oneConfig, configsPath, 'rows');
  %pause
  %starts at max and walks toward one
  configsPath;
  currIdx = maxIdx;
  alreadyCompiledPath = [alreadyCompiledPath; configsPath(currIdx, :)];

  do
    %dont need to compile it, since the search queue comes from the compiled designs
    newConfig = floor(configsPath(currIdx,:)-tenDiffConst);
    newConfig = max(newConfig, 1);
    [~, ix] = ismember(newConfig, configsPath, 'rows');

    if(ix == 0)
      do
        %disp('not found reducing')
        newConfig = floor(newConfig-tenDiffConst);
        newConfig = max(newConfig, 1);
        [~, ix] = ismember(newConfig, configsPath, 'rows');
        %disp('-----')
      until(ix ~= 0 || all(newConfig) == 1)
    endif

    if(ix == 0 && all(newConfig) == 1)
      break
    endif

    alreadyCompiledPath = [alreadyCompiledPath; configsPath(ix, :)];
    currIdx = ix;

  until( all(configsPath(currIdx, :)==1) ) %remember that until stops at true

  % return variables
  nCompiledDesigns = rows(alreadyCompiledPath);
  constOut = alreadyCompiledPath;
  [~, idxOut] = ismember(alreadyCompiledPath, configsPath, 'rows');
  metricsOut = metricsPath(idxOut, :);
  %alreadyCompiledPath
end %function
