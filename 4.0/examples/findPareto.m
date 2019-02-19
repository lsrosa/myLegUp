function [rx, ry, idx] = findPareto(x, y)
  paretoPoints = [];
  idx = [];
  cond = zeros(numel(x), 1);
  %x = x';
  %y = y';

  for i=1:numel(x)
    c1 = x < x(i) & y < y(i);
    c2 = x == x(i) & y < y(i);
    c3 = x < x(i) & y == y(i);
    cond = sum(  c1 | c2 | c3 );
    if(cond == 0)
      paretoPoints = [paretoPoints; x(i) y(i)];
      idx = [idx; i];
    end
  end

  [rx, sortedIx] = sort(paretoPoints(:, 1));
  ry = paretoPoints(sortedIx, 2);
  idx = idx(sortedIx);
  %paretoPoints = paretoPoints';
  return;
end
