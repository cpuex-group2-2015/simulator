#ifndef __CPU_H
#define __CPU_H

#include <stdio.h>
#include <stdint.h>
#include "util.h"

#define GPR_LEN 32    /* Number of GPRs */
#define FPR_LEN 32    /* Number of FPRs */

#define OPCODE(ir) DOWNTO((ir), 31, 26)

typedef uint8_t  CR;  /* Condition Register */
typedef uint32_t LR;  /* Link Register */
typedef uint32_t CTR; /* Count Register */
typedef int32_t GPR; /* General Purpose Register */
typedef uint32_t FPR; /* Floating-Point Register */

typedef struct cpu_t {
    CR  cr;
    LR  lr;
    CTR ctr;
    GPR gpr[GPR_LEN];
    FPR fpr[FPR_LEN];
    unsigned int pc;
    unsigned int nir;
} CPU;

typedef struct memory_t {
    uint8_t *brom;
    uint8_t *sram;
    unsigned int brom_size;
    unsigned int sram_size;
} MEMORY;

typedef struct option_t {
    unsigned int entry_point;
    int interactive;
    FILE *fp;
} OPTION;

#endif

void sim_run(CPU *cpu, MEMORY *mem, OPTION *option);
