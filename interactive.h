#ifndef __INTERACTIVE_H
#define __INTERACTIVE_H

#define MODE_INTERACTIVE 1
#define MODE_STEP 2
#define MODE_QUIT 3
#define MODE_RUN 0

#include "sim.h"

typedef struct prompt_t {
    char c;
    int target;
} PROMPT;

int interactive_prompt(CPU *cpu, RAM *ram, int *mode);
#endif
