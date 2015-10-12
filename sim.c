#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sim.h"
#include "interactive.h"

void load_instruction(unsigned int *ir, MEMORY *m, unsigned int pc) {
    memcpy(ir, m->brom + pc, sizeof(unsigned int));
    *ir = __builtin_bswap32(*ir);
}

void initialize_cpu(CPU *cpu, MEMORY *m, OPTION *option) {
    cpu->cr = 0;
    cpu->lr = 0;
    cpu->ctr = 0;
    memset(cpu->gpr, 0, sizeof(GPR) * GPR_LEN);
    memset(cpu->fpr, 0, sizeof(GPR) * GPR_LEN);
    cpu->pc  = option->entry_point;
    load_instruction(&(cpu->nir), m, cpu->pc);
}

void simulate_io(int io, GPR *r, FILE *fp) {
    char buf[64];
    if (io == 0) { /* send */
        fprintf(fp, "%c", (char) DOWNTO(*r, 7, 0));
    } else { /* read */
        printf("\nrecv> ");
        if (fgets(buf, 63, stdin) == NULL) {
            *r = 0;
        } else {
            *r = buf[0];
        }
    }
}

void load_from_sram(void *reg, MEMORY *m, unsigned int addr, size_t size) {
    memcpy(reg, m->sram + addr, size);
}

void store_to_sram(void *reg, MEMORY *m, unsigned int addr, size_t size) {
    memcpy(m->sram + addr, reg, size);
}

void extended_op(CPU *cpu, MEMORY *m, int rx, int ry, int rz, uint16_t xo) {
    int ea;
    switch (xo) {
        /* ldx  */
        case 23:
            ea = (ry == 0 ? 0 : cpu->gpr[ry]) + cpu->gpr[rz];
            load_from_sram(&(cpu->gpr[rx]), m, ea, sizeof(GPR));
            break;
        /* stx */
        case 151:
            ea = (ry == 0 ? 0 : cpu->gpr[ry]) + cpu->gpr[rz];
            store_to_sram(&(cpu->gpr[rx]), m, ea, sizeof(GPR));
            break;
        /* add */
        /* and */
        case 28:
            cpu->gpr[ry] = cpu->gpr[rx] & cpu->gpr[rz];
            break;
        /* or  */
        case 444:
            cpu->gpr[ry] = cpu->gpr[rx] | cpu->gpr[rz];
            break;
        /* mtlr */
        case 467:
            cpu->lr = cpu->gpr[rx];
            break;
        /* mflr */
        case 339:
            cpu->gpr[rx] = cpu->lr;
            break;
        default:
            printf("invalid extend-op: %d\n", xo);
            break;
    }
}

int tick(CPU *cpu, MEMORY *m, OPTION *option) {
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
    int ea;
    GPR a, b;

    rx = DOWNTO(ir, 25, 21);
    ry = DOWNTO(ir, 20, 16);
    rz = DOWNTO(ir, 15, 11);
    si  = (int16_t) DOWNTO(ir, 15,  0);

    switch (opcode) {
        /* extended_opcode */
        case 31:
            extended_op(cpu, m, rx, ry, rz, DOWNTO(ir, 10, 1));
            break;
        /* ld */
        case 32:
            ea = (ry == 0 ? 0 : cpu->gpr[ry]) + si;
            load_from_sram(&(cpu->gpr[rx]), m, ea, sizeof(GPR));
            break;
        /* st */
        case 36:
            ea = (ry == 0 ? 0 : cpu->gpr[ry]) + si;
            store_to_sram(&(cpu->gpr[rx]), m, ea, sizeof(GPR));
            break;
        /* addi */
        case 14:
            cpu->gpr[rx] = (int16_t) si + (ry == 0 ? 0 : cpu->gpr[ry]);
            break;
        /* addis */
        case 15:
            cpu->gpr[rx] = ((int32_t) si << 16) + (ry == 0 ? 0 : cpu->gpr[ry]);
        /* cmpi */
        case 11:
            a = cpu->gpr[rx];
            cpu->cr = a < si ? 0x8 : (a > si ? 0x4 : 0x2);
            break;
        /* cmp */
        case 30:
            a = cpu->gpr[rx];
            b = cpu->gpr[ry];
            cpu->cr = a < b  ? 0x8 : (a > b  ? 0x4 : 0x2);
            break;
        /* b, bl */
        case 18:
            nia = cpu->pc + si;
            if (BIT(ir, 25)) {
                cpu->lr = cpu->pc + 4;
            }
            break;
        /* bc, bcl */
        case 16:
            if (BIT(cpu->cr, 3 - DOWNTO(ir, 23, 22)) == BIT(ir, 24)) {
                nia = cpu->pc + si;
            }
            if (BIT(ir, 25)) {
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
    load_instruction(&(cpu->nir), m, cpu->pc);

    return 1;
}


void sim_run(CPU *cpu, MEMORY *m, OPTION *option) {
    unsigned int c = 0;
    int mode = option->interactive ? MODE_INTERACTIVE : MODE_RUN;
    initialize_cpu(cpu, m, option);
    for (;;) {
        c++;
        if (mode == MODE_INTERACTIVE) {
            interactive_prompt(cpu, m, &mode);
        }
        if (tick(cpu, m, option) == 0) {
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
