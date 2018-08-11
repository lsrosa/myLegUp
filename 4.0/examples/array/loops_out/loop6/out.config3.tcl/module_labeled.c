extern void __legup_label(char* label);
// Multi-dimensional as
// Ebpected result: 156
#include <stdio.h>
#define N 100
int a[N], b[N];

int main() {
    int result = 0;

    loop6: for(int i=1; i<N; i++){
__legup_label("loop6");
      a[i] /= (i/(a[i]+1))/(b[i]+1);
      //b[i] /= (i/(b[i]+1))/(a[i]+1);
    }

    //printf("Result: %d\n", result);
    if (result == 0) {
        printf("RESULT: PASS\n");
    } else {
        printf("RESULT: FAIL\n");
    }
    return result;
}
