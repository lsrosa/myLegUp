for (int c1 = -99; c1 <= 100; c1 += 1) {
  if (c1 <= 0)
    S1(1, -c1 + 1);
  for (int c3 = max(1, -2 * c1 + 3); c3 <= min(199, -2 * c1 + 199); c3 += 2) {
    S2(((c3 - 1) / 2) + c1, (c3 + 1) / 2);
    S1(((c3 + 1) / 2) + c1, (c3 + 1) / 2);
  }
  if (c1 >= 1)
    S2(100, -c1 + 101);
}
