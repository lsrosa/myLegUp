extern void __legup_label(char* label);
// Multi-dimensional arrays
// Expected result: 156

#include <stdio.h>
volatile int array[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
volatile int a1[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
volatile int a2[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
volatile int array2[12];

int fct(int size) {
    int result = 0;
    int i;

    loop4: for (int i = 0; i < 12; i++) {
__legup_label("loop4");
        array2[i] = a1[i]*a2[i-2];
    }

    loop5: for (i = 0; i < size; i++) {
__legup_label("loop5");
        result += array2[i];
    }
    return result;
}

int main() {
    int result = 0;
    int a, b, c;


    loop1: for (a = 0; a < 2; a++) {
__legup_label("loop1");
        loop2: for (b = 0; b < 2; b++) {
__legup_label("loop2");
            loop3: for (c = 0; c < 3; c++) {
__legup_label("loop3");
                result += array[a*2+b*2+c];
            }
        }
    }



    result += fct(12);

    printf("Result: %d\n", result);
    if (result == 156) {
        printf("RESULT: PASS\n");
    } else {
        printf("RESULT: FAIL\n");
    }
    return result;
}
