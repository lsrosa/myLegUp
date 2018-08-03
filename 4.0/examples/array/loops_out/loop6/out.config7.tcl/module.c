// Multi-dimensional arrays
// Expected result: 156
#include <stdio.h>
#define N 100
int array[N];

int fct(int *array, int size) {
    int result = 0;
    int i;
    loop4: for (i = 0; i < size; i++) {
        result += array[i];
    }
    return result;
}

int main() {
    int result = 0;
    int a, b, c;

    loop6: for(int i=1; i<N; i++){
      array[i] = i*i+3*i-19;
    }

    loop3: for (a = 0; a < 2; a++) {
        loop2: for (b = 0; b < 2; b++) {
            loop1: for (c = 0; c < 3; c++) {
                result += array[a*2+b*2+c];
            }
        }
    }

    loop5: for(int i=1; i<N-1; i++){
      array[i] = array[i+1]*array[i-1];
    }

    result += fct((int *)array, 12);

    printf("Result: %d\n", result);
    if (result == 39) {
        printf("RESULT: PASS\n");
    } else {
        printf("RESULT: FAIL\n");
    }
    return result;
}
