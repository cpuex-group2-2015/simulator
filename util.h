#ifndef __UTIL_H
#define __UTIL_H

#define DOWNTO(l, n, m) (((l) & ((1 << ((n) + 1)) - 1)) >> (m))
#endif
