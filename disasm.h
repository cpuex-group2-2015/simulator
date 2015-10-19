#ifndef __DISASM_H
#define __DISASM_H
#include <stdio.h>
#include "sim.h"
int disasm(unsigned int ir, char *buf, size_t n);
void print_disasm_inst(unsigned int addr, int before, int after, MEMORY *m, int ex_col, BREAKPOINT *bp);
#endif
