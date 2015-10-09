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

void simulate_io(int io, GPR *r, FILE *fp) {
    char buf[64];
    if (io == 0) { /* send */
        fprintf(fp, "%c", (char) DOWNTO(*r, 7, 0));
    } else { /* read */
        printf("\nrecv > ");
        if (fgets(buf, 63, stdin) == NULL) {
            *r = 0;
        } else {
            *r = buf[0];
        }
    }
}

int tick(CPU *cpu, RAM *ram, OPTION *option) {
    unsigned int ir, opcode, nia;

    ir = cpu->nir;
    opcode = OPCODE(ir);
    nia = cpu->pc + 4;

    /* halt */
    if (opcode == 0x3f) {
        return 0;
    }

    /* load and store */
    int rx, ry, rz;
    int16_t si;
    int opcd, f;
    int ea;
    GPR a, b;

    rx = DOWNTO(ir, 25, 21);
    ry = DOWNTO(ir, 20, 16);
    rz = DOWNTO(ir, 15, 11);
    si  = (int16_t) DOWNTO(ir, 15,  0);
    opcd = DOWNTO(ir, 10, 1);
    f  = ir & 1;

    switch (opcode) {
        /* lwz */
        case 32:
            ea = (ry == 0 ? 0 : cpu->gpr[ry]) + si;
            cpu->gpr[rx]= ram->m[ea];
            break;
        /* stw */
        case 36:
            ea = (ry == 0 ? 0 : cpu->gpr[ry]) + si;
            ram->m[ea] = cpu->gpr[rx];
            break;
        /* addi */
        case 14:
            cpu->gpr[rx] = (int16_t) si + (ry == 0 ? 0 : cpu->gpr[ry]);
            break;
        /* cmpi */
        case 11:
            a = cpu->gpr[rx];
            cpu->cr |= a < si ? 0x8 : (a > si ? 0x4 : 0x2);
            break;
        /* cmpw */
        case 30:
            a = cpu->gpr[rx];
            b = cpu->gpr[ry];
            cpu->cr |= a < b  ? 0x8 : (a > b  ? 0x4 : 0x2);
            break;
        /* branch */
        case 18:
            nia = cpu->pc + si;
            if (DOWNTO(ir, 25, 25)) {
                cpu->lr = cpu->pc + 4;
            }
            break;
        /* send / recv */
        case 0:
            simulate_io(DOWNTO(ir, 20, 20), &(cpu->gpr[rx]), option->fp);
            break;
        default:
            break;
    }

    cpu->pc = nia;
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
