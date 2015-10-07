#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sim.h"
#include "interactive.h"

void load_instruction(unsigned int *ir, RAM *ram, unsigned int pc) {
    memcpy(ir, ram->m + pc, sizeof(unsigned int));
    *ir = __builtin_bswap32(*ir);
}

void initialize_cpu(CPU *cpu, RAM *ram, OPTION *option) {
    cpu->cr = 0;
    cpu->lr = 0;
    cpu->ctr = 0;
    memset(cpu->gpr, 0, sizeof(GPR) * GPR_LEN);
    memset(cpu->fpr, 0, sizeof(GPR) * GPR_LEN);
    cpu->pc  = option->entry_point;
    load_instruction(&(cpu->nir), ram, cpu->pc);
}


int tick(CPU *cpu, RAM *ram, OPTION *option) {
    unsigned int ir, opcode;

    ir = cpu->nir;
    opcode = OPCODE(ir);

    /* halt */
    if (opcode == 0x3f) {
        return 0;
    }

    /* load and store */
    int rx, ry, rz;
    uint16_t v;
    int opcd, f;
    int ea;

    rx = DOWNTO(ir, 25, 21);
    ry = DOWNTO(ir, 20, 16);
    rz = DOWNTO(ir, 15, 11);
    v  = DOWNTO(ir, 15,  0);
    opcd = DOWNTO(ir, 10, 1);
    f  = ir & 1;

    switch (opcode) {
        /* addi */
        case 14:
            cpu->gpr[rx] = (int16_t) v + (ry == 0 ? 0 : cpu->gpr[ry]);
            break;
        /* lwz */
        case 32:
            ea = (ry == 0 ? 0 : cpu->gpr[ry]) + (int16_t) v;
            cpu->gpr[rx]= ram->m[ea];
            break;
        /* stw */
        case 36:
            ea = (ry == 0 ? 0 : cpu->gpr[ry]) + (int16_t) v;
            ram->m[ea] = cpu->gpr[rx];
            break;
        default:
            break;
    }

    cpu->pc = cpu->pc + 4;
    load_instruction(&(cpu->nir), ram, cpu->pc);

    return 1;
}


void sim_run(CPU *cpu, RAM *ram, OPTION *option) {
    unsigned int c = 0;
    int mode = option->interactive ? MODE_INTERACTIVE : MODE_RUN;
    initialize_cpu(cpu, ram, option);
    for (;;) {
        c++;
        if (mode == MODE_INTERACTIVE) {
            interactive_prompt(cpu, ram, &mode);
        }
        if (tick(cpu, ram, option) == 0) {
            break;
        }
        if (mode == MODE_STEP) {
            mode = MODE_INTERACTIVE;
        }
    }

    if (mode == MODE_QUIT) {
        printf("simulation aborted at %06x\n", cpu->pc);
    } else {
        printf("simulation completed at %06x\n", cpu->pc);
    }
    printf("%u instructions executed\n", c);
}
