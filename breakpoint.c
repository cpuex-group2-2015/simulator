#include <stdlib.h>
#include "sim.h"
#include "breakpoint.h"

void free_breakpoint(BREAKPOINT *bp) {
    BREAKPOINT *next;
    while (bp != NULL) {
        next = bp->next;
        free(bp);
        bp = next;
    }
}

BREAKPOINT *set_breakpoint_addr(unsigned int addr, BREAKPOINT *breakpoint) {
    BREAKPOINT *bp;

    bp = malloc(sizeof(BREAKPOINT));
    bp->n = breakpoint == NULL ? 1 : breakpoint->n + 1;
    bp->type = BREAKPOINT_TYPE_ADDR;
    bp->target.addr = addr;
    bp->next = breakpoint;

    return bp;
}

int check_breakpoint(unsigned int addr, unsigned int ir, BREAKPOINT *bp) {
    while (bp != NULL) {
        if (bp->type == BREAKPOINT_TYPE_ADDR) {
            if (bp->target.addr == addr) {
                return bp->n;
            }
        }
        bp = bp->next;
    }
    return 0;
}

void print_breakpoint_list(BREAKPOINT *bp) {
    if (bp == NULL) {
        printf("No breakpoints\n");
        return;
    }
    printf("ID  TARGET\n");
    while (bp != NULL) {
        if (bp->type == BREAKPOINT_TYPE_ADDR) {
            printf("%2d  0x%06x\n", bp->n, bp->target.addr);
        }
        bp = bp->next;
    }
}
