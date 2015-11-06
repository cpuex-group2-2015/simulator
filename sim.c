#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sim.h"
#include "instruction.h"
#include "interactive.h"
#include "breakpoint.h"

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
        case XO_LDX:
            ea = (ry == 0 ? 0 : cpu->gpr[ry]) + cpu->gpr[rz];
            load_from_sram(&(cpu->gpr[rx]), m, ea, sizeof(GPR));
            break;
        /* stx */
        case XO_STX:
            ea = (ry == 0 ? 0 : cpu->gpr[ry]) + cpu->gpr[rz];
            store_to_sram(&(cpu->gpr[rx]), m, ea, sizeof(GPR));
            break;
        /* add */
        case XO_ADD:
            cpu->gpr[rx] = cpu->gpr[ry] + cpu->gpr[rz];
            break;
        /* and */
        case XO_AND:
            cpu->gpr[rx] = cpu->gpr[ry] & cpu->gpr[rz];
            break;
        /* or  */
        case XO_OR:
            cpu->gpr[rx] = cpu->gpr[ry] | cpu->gpr[rz];
            break;
        /* mtlr, mtctr */
        case XO_MTSPR:
            if (rz == 0) {
                cpu->lr = cpu->gpr[rx];
            } else {
                cpu->ctr = cpu->gpr[rx];
            }
            break;
        /* mflr */
        case XO_MFSPR:
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
    if (opcode == OP_HALT) {
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
        case OP_XO:
            extended_op(cpu, m, rx, ry, rz, DOWNTO(ir, 10, 1));
            break;
        /* ld */
        case OP_LD:
            ea = (ry == 0 ? 0 : cpu->gpr[ry]) + si;
            load_from_sram(&(cpu->gpr[rx]), m, ea, sizeof(GPR));
            break;
        /* st */
        case OP_ST:
            ea = (ry == 0 ? 0 : cpu->gpr[ry]) + si;
            store_to_sram(&(cpu->gpr[rx]), m, ea, sizeof(GPR));
            break;
        /* addi */
        case OP_ADDI:
            cpu->gpr[rx] = (int16_t) si + (ry == 0 ? 0 : cpu->gpr[ry]);
            break;
        /* addis */
        case OP_ADDIS:
            cpu->gpr[rx] = ((int32_t) si << 16) + (ry == 0 ? 0 : cpu->gpr[ry]);
        /* cmpi */
            break;
        case OP_ANDI:
            cpu->gpr[rx] = (uint16_t) si & cpu->gpr[ry];
            break;
        case OP_ORI:
            cpu->gpr[rx] = (uint16_t) si | cpu->gpr[ry];
            break;
        case OP_CMPI:
            a = cpu->gpr[rx];
            cpu->cr = a < si ? 0x8 : (a > si ? 0x4 : 0x2);
            break;
        /* cmp */
        case OP_CMP:
            a = cpu->gpr[rx];
            b = cpu->gpr[ry];
            cpu->cr = a < b  ? 0x8 : (a > b  ? 0x4 : 0x2);
            break;
        /* b, bl */
        case OP_B:
            nia = (uint16_t) si;
            if (BIT(ir, 25)) {
                cpu->lr = cpu->pc + 4;
            }
            break;
        /* bc, bcl */
        case OP_BC:
            if (BIT(cpu->cr, 3 - DOWNTO(ir, 23, 22)) == BIT(ir, 24)) {
                nia = (uint16_t) si;
            }
            if (BIT(ir, 25)) {
                cpu->lr = cpu->pc + 4;
            }
            break;
        /* blr, bctr, bctrl */
        case OP_BSPR:
            if (BIT(ir, 22) == 0) {
                /* blr */
                nia = cpu->lr;
            } else {
                nia = cpu->ctr;
                if (BIT(ir, 25)) {
                    cpu->lr = cpu->pc + 4;
                }
            }
            break;
        /* send / recv */
        case OP_SENDRECV:
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
    int n;
    initialize_cpu(cpu, m, option);
    for(;;) {
        c++;
        if ((n = check_breakpoint(cpu->pc, cpu->nir, option->breakpoint)) > 0) {
            option->mode = MODE_INTERACTIVE;
            printf("stop at breakpoint %d\n", n);
        }
        if (option->mode == MODE_INTERACTIVE) {
            interactive_prompt(cpu, m, option);
        }
        if (option->mode == MODE_QUIT) {
            break;
        }
        if (tick(cpu, m, option) == 0) {
            break;
        }
    }

    if (option->mode == MODE_QUIT) {
        printf("simulation aborted at 0x%06x\n", cpu->pc);
    } else {
        printf("simulation completed at 0x%06x\n", cpu->pc);
    }
    printf("%u instructions executed\n", c);
    if (option->interactive && option->mode != MODE_QUIT) {
        interactive_prompt(cpu, m, option);
    }
}
