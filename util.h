#ifndef __UTIL_H
#define __UTIL_H

#include <stdint.h>

/* #define DOWNTO(l, n, m) (((l) & ((1UL << ((n) + 1)) - 1)) >> (m)) */

#define DOWNTO(l, n, m) (((0xffffffffU >> (31 - (n))) & l) >> (m))
#define DOWNTO64(l, n, m) (((0xffffffffffffffffULL >> (64 - (n))) & l) >> (m))
#define BIT(l, n) DOWNTO(l, n, n)
float ui2f(uint32_t ui);
uint32_t f2ui(float f);
double get_dtime();

#endif
