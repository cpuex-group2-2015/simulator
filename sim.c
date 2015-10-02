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
    cpu->nir = ram->m[entry_point];
}

int tick(CPU *cpu, RAM *ram) {
    int ir, opcode;

    ir = cpu->nir;
    opcode = OPCODE(ir);

    if (opcode == 0x3f) { /* 0b111111 -> halt*/
        return 0;
    }

    cpu->pc = cpu->pc + 1;
    cpu->nir = ram->m[cpu->pc];
}

void sim_run(CPU *cpu, RAM *ram) {
    while (tick(cpu, ram) > 0);
}
