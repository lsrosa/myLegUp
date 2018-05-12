/* Generated from ../../../git/cloog/test/./reservoir/lim-lam2.cloog by CLooG 0.14.0-72-gefe2fc2 gmp bits in 0.01s. */
extern void hash(int);

/* Useful macros. */
#define floord(n,d) (((n)<0) ? -((-(n)+(d)-1)/(d)) : (n)/(d))
#define ceild(n,d)  (((n)<0) ? -((-(n))/(d)) : ((n)+(d)-1)/(d))
#define max(x,y)    ((x) > (y) ? (x) : (y))
#define min(x,y)    ((x) < (y) ? (x) : (y))

#define S1(i) { hash(1); hash(i); }
#define S2(i,j) { hash(2); hash(i); hash(j); }
#define S3(i,j) { hash(3); hash(i); hash(j); }

void test(int M, int N)
{
  /* Scattering iterators. */
  int c2, c4;
  /* Original iterators. */
  int i, j;
  for (c2=1;c2<=M;c2++) {
    S1(c2) ;
  }
  if (N >= 2) {
    for (c2=1;c2<=M;c2++) {
      for (c4=2;c4<=N;c4++) {
        S2(c2,c4) ;
      }
    }
  }
  if (N >= 2) {
    for (c2=1;c2<=M;c2++) {
      for (c4=1;c4<=N-1;c4++) {
        S3(c2,c4) ;
      }
    }
  }
}
