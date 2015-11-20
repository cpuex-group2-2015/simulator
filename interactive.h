#ifndef __INTERACTIVE_H
#define __INTERACTIVE_H

#define MODE_INTERACTIVE 1
#define MODE_STEPOVER 2
#define MODE_QUIT 3
#define MODE_RUN 0

#define ICMD_NOP           0
#define ICMD_PRINT         1
#define ICMD_STEP          2
#define ICMD_STEPOVER      3
#define ICMD_RUN           4
#define ICMD_DISASM        5
#define ICMD_DISASM_ALWAYS 6
#define ICMD_BPSET         7
#define ICMD_BPSHOW        8
#define ICMD_BPREMOVE      9
#define ICMD_WATCH         10
#define ICMD_VIEW_REGISTER 11
#define ICMD_DUMP          12
#define ICMD_QUIT          125
#define ICMD_HELP          126
#define ICMD_INVALID       127

#include "sim.h"

typedef struct prompt_t {
    int c;
    int target;
    int target2;
    int target3;
} PROMPT;

int interactive_prompt(CPU *cpu, MEMORY *m, OPTION *option);
#endif
