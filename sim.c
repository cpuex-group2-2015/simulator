#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
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
        /* mtlr */
        case XO_MTLR:
            cpu->lr = cpu->gpr[ry];
            break;
        case XO_MTCTR:
            cpu->ctr = cpu->gpr[ry];
            break;
        /* mflr */
        case XO_MFSPR:
            cpu->gpr[rx] = cpu->lr;
            break;
        /* ldx  */
        case XO_LFX:
            ea = (ry == 0 ? 0 : cpu->gpr[ry]) + cpu->gpr[rz];
            load_from_sram(&(cpu->fpr[rx]), m, ea, sizeof(FPR));
            break;
        /* stx */
        case XO_STFX:
            ea = (ry == 0 ? 0 : cpu->gpr[ry]) + cpu->gpr[rz];
            store_to_sram(&(cpu->fpr[rx]), m, ea, sizeof(FPR));
            break;
        /* sl */
        case XO_SL:
            cpu->gpr[rx] = cpu->gpr[ry] << (cpu->gpr[rz] & 0x1f);
            break;
        /* sr */
        case XO_SR:
            cpu->gpr[rx] = cpu->gpr[ry] >> (cpu->gpr[rz] & 0x1f);
            break;
        default:
            printf("invalid extend-op: %d\n", xo);
            break;
    }
}

void fp_op(CPU *cpu, int rx, int ry, int rz, uint16_t xo) {
    float fra = ui2f(cpu->fpr[ry]);
    float frb = ui2f(cpu->fpr[rz]);

    switch (xo) {
        case FP_MR:
            cpu->fpr[rx] = cpu->fpr[rz];
            break;
        case FP_ADD:
            cpu->fpr[rx] = f2ui(fra + frb);
            break;
        case FP_SUB:
            cpu->fpr[rx] = f2ui(fra - frb);
            break;
        case FP_MUL:
            cpu->fpr[rx] = f2ui(fra * frb);
            break;
        case FP_DIV:
            cpu->fpr[rx] = f2ui(fra / frb);
            break;
        case FP_CMP:
            if (isnan(fra) || isnan(frb)) cpu->cr = 0x2;
            else if (fra < frb)           cpu->cr = 0x8;
            else if (fra > frb)           cpu->cr = 0x4;
            else                          cpu->cr = 0x2;
            break;
        default:
            printf("invalid floating-point op: %d\n", xo);
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
            a = cpu->gpr[ry];
            cpu->cr = a < si ? 0x8 : (a > si ? 0x4 : 0x2);
            break;
        /* cmp */
        case OP_CMP:
            a = cpu->gpr[ry];
            b = cpu->gpr[rz];
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
        /* lf */
        case OP_LF:
            ea = (ry == 0 ? 0 : cpu->gpr[ry]) + si;
            load_from_sram(&(cpu->fpr[rx]), m, ea, sizeof(FPR));
            break;
        /* stf */
        case OP_STF:
            ea = (ry == 0 ? 0 : cpu->gpr[ry]) + si;
            store_to_sram(&(cpu->fpr[rx]), m, ea, sizeof(FPR));
            break;
        /* send */
        case OP_SEND:
            simulate_io(0, &(cpu->gpr[rx]), option->fp);
            break;
        /* recv */
        case OP_RECV:
            simulate_io(1, &(cpu->gpr[rx]), option->fp);
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
