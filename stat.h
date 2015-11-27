#ifndef __STAT_H
#define __STAT_H
#include <stdio.h>

typedef struct stat_t {
    struct stat_t *l;
    struct stat_t *r;
    int opcd;
    long long unsigned int n;
    char *label;
} STAT;

STAT *stat_init();
STAT *stat_add(STAT *s, int opcd, int xo, char *label);
void stat_free(STAT *s);
void stat_print(FILE *fp, STAT *s);
#endif
