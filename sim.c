#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sim.h"

void initialize_cpu(CPU *cpu, RAM *ram, unsigned int entry_point) {
    cpu->cr = 0;
    cpu->lr = 0;
    cpu->ctr = 0;
    memset(cpu->gpr, 0, sizeof(GPR) * GPR_LEN);
    memset(cpu->fpr, 0, sizeof(GPR) * GPR_LEN);
    cpu->pc  = entry_point;
    cpu->nir = __builtin_bswap32(ram->m[entry_point]);
}


int tick(CPU *cpu, RAM *ram) {
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

    cpu->pc = cpu->pc + 1;
    cpu->nir = __builtin_bswap32(ram->m[cpu->pc]);

    return 1;
}

void sim_run(CPU *cpu, RAM *ram) {
    unsigned int c = 1;

    initialize_cpu(cpu, ram, 0);
    while (tick(cpu, ram) > 0) {
        c++;
    }

    printf("%u instructions executed\n", c);
}
