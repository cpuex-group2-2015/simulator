#ifndef __BREAKPOINT_H
#define __BREAKPOINT_H
#include "sim.h"

typedef struct breakpoint_t {
    int n;
    enum breakpoint_type_t {
        BREAKPOINT_TYPE_ADDR,
        BREAKPOINT_TYPE_SYMBOL
    } type;
    union target_t {
        unsigned int addr;
        int opcd;
    } target;
    struct breakpoint_t *next;
} BREAKPOINT;

void free_breakpoint(BREAKPOINT *bp);
BREAKPOINT *set_breakpoint_addr(unsigned int addr, BREAKPOINT *breakpoint);
BREAKPOINT *remove_breakpoint(int n, BREAKPOINT *bp);
int check_breakpoint(unsigned int addr, unsigned int ir, BREAKPOINT *bp);
void print_breakpoint_list(BREAKPOINT *bp);

#endif
