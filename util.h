#ifndef __UTIL_H
#define __UTIL_H

/* #define DOWNTO(l, n, m) (((l) & ((1UL << ((n) + 1)) - 1)) >> (m)) */

#define DOWNTO(l, n, m) (((0xffffffffU >> (31 - (n))) & l) >> (m))
#endif
