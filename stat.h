#ifndef __STAT_H
#define __STAT_H
#include <stdio.h>
#include <stdint.h>

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
int stat_logger_init(char *name);
void stat_logger_log(STAT *s, int op, int xo, uint32_t a, uint32_t b, uint32_t c);
void stat_logger_log_cmp(STAT *s, int op, int xo, uint32_t a, uint32_t b, uint32_t cr);
#endif
