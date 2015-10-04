#include <stdio.h>
#include "util.h"

void print_bin(unsigned int n) {
    int i;
    for (i = 31; i >= 0; i--) {
        putchar('0' + !!(n & (1 << i)));
    }
    putchar('\n');
}
