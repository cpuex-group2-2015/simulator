#ifndef __CPU_H
#define __CPU_H

#include <stdint.h>
#include "util.h"

#define GPR_LEN 32    /* Number of GPRs */
#define FPR_LEN 32    /* Number of FPRs */

#define OPCODE(ir) DOWNTO((ir), 31, 26)

typedef uint32_t inst_t;

typedef uint8_t  CR;  /* Condition Register */
typedef uint64_t LR;  /* Link Register */
typedef uint32_t CTR; /* Count Register */
typedef uint32_t GPR; /* General Purpose Register */
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

typedef struct ram_t {
    inst_t *m;
    unsigned int size;
} RAM;

typedef struct option_t {
    unsigned int entry_point;
    int interactive;
} OPTION;

typedef struct prompt_t {
    char command;
    int target;
} PROMPT;

#endif

void sim_run(CPU *cpu, RAM *ram, OPTION *option);
