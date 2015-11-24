#include <stdio.h>
#include "util.h"

void print_bin(unsigned int n) {
    int i;
    for (i = 31; i >= 0; i--) {
        putchar('0' + !!(n & (1 << i)));
    }
    putchar('\n');
}

float ui2f(uint32_t ui) {
    union uif {
        uint32_t ui;
        float f;
    } uif;

    uif.ui = ui;
    return uif.f;
}

uint32_t f2ui(float f) {
    union uif {
        uint32_t ui;
        float f;
    } uif;

    uif.f = f;
    return uif.ui;
}
