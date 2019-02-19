function [result] = adrs(set1, set2)
  uSet1 = unique(set1, 'rows');
  uSet2 = unique(set2, 'rows');

  s1 = rows(uSet1);
  s2 = rows(uSet2);
  sm = columns(uSet1);

  assert( columns(uSet2) == sm, 'matrices should have the same number of columns')

  acc = 0;
  for is1 = 1:s1
    m = inf;
    for is2 = 1:s2
      d = max( (uSet2(is2,:)-uSet1(is1,:))./uSet2(is2,:) );
      if(d < m)
        m = d;
      end
    end
    acc = acc + m;
  end

  result = acc/s1;
end
