#ifndef __CPU_H
#define __CPU_H

#include <stdint.h>

#define GPR_LEN 32    /* Number of GPRs */
#define FPR_LEN 32    /* Number of FPRs */

typedef uint32_t CR;  /* Condition Register */
typedef uint64_t LR;  /* Link Register */
typedef uint32_t CTR; /* Count Register */
typedef uint32_t GPR; /* General Purpose Register */
typedef uint32_t FPR; /* Floating-Point Register */

typedef struct cpu_t {
    CR  cr;
    LR  lr;
    CTR ctr;
    GPR gpr[32];
    FPR fpr[32];
} CPU;
#endif
