// I/O Library Development - main
// Author: Mathew Hall
// Date: May 23, 2014

#include "stdiov.h"

void __attribute__ ((noinline)) __attribute__ ((used)) midLevel(void) {
    putchar('c');
}

int main(void) {

    midLevel();
    return 0;
}
