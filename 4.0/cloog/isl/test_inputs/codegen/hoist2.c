for (int c0 = 1; c0 <= 5; c0 += 1)
  for (int c1 = max(t1, t1 - 64 * b + 64); c1 <= min(70, -((c0 - 1) % 2) - c0 + 73); c1 += 64)
    A(c0, 64 * b + c1 - 8);
