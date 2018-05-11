// PR1602
// RUN: %dragonegg -S %s -o - -O3 | not grep ptrtoint
// RUN: %dragonegg -S %s -o - -O3 | grep getelementptr | count 1


struct S { virtual void f(); };

typedef void (S::*P)(void);

const P p = &S::f; 

void g(S s) {
   (s.*p)();
 }
