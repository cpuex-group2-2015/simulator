#include <stdio.h>
#include <string.h>
#include "sim.h"
#include "instruction.h"

int disasm_xo(unsigned int ir, char *buf, size_t n) {
    int xo = DOWNTO(ir, 10, 1);
    int rx = DOWNTO(ir, 25, 21);
    int ry = DOWNTO(ir, 20, 16);
    int rz = DOWNTO(ir, 15, 11);

    switch(xo) {
        /* ldx */
        case XO_LDX:
            snprintf(buf, n, "ldx   r%d, r%d, r%d", rx, ry, rz);
            break;
        /* stx */
        case XO_STX:
            snprintf(buf, n, "stx   r%d, r%d, r%d", rx, ry, rz);
            break;
        /* add */
        case XO_ADD:
            snprintf(buf, n, "add   r%d, r%d, r%d", rx, ry, rz);
            break;
        /* and */
        case XO_AND:
            snprintf(buf, n, "and   r%d, r%d, r%d", rx, ry, rz);
            break;
        /* or */
        case XO_OR:
            snprintf(buf, n, "or    r%d, r%d, r%d", rx, ry, rz);
            break;
        /* mtlr */
        case XO_MTSPR:
            if (rz == 0) {
                snprintf(buf, n, "mtlr  r%d", rx);
            } else {
                snprintf(buf, n, "mtctr r%d", rx);
            }
            break;
        /* mflr */
        case XO_MFSPR:
            snprintf(buf, n, "mflr  r%d", rx);
            break;
        default:
            strncpy(buf, "unknown", n);
            return -1;
    }
    return 0;
}

int disasm(unsigned int ir, char *buf, size_t n) {
    int opcode = OPCODE(ir);
    int rx = DOWNTO(ir, 25, 21);
    int ry = DOWNTO(ir, 20, 16);
    uint16_t d = DOWNTO(ir, 15, 0);
    const char *branch_s[] = {
        "bge", "ble", "bne", "bc ", "blt", "bgt", "beq", "bc "
    };


    switch(opcode) {
        /* ld */
        case OP_LD:
            if (d == 0) {
                snprintf(buf, n, "ld    r%d, r%d", rx, ry);
            } else {
                snprintf(buf, n, "ld    r%d, %d(r%d)", rx, d, ry);
            }
            break;
        /* st */
        case OP_ST:
            if (d == 0) {
                snprintf(buf, n, "st    r%d, r%d", rx, ry);
            } else {
                snprintf(buf, n, "st    r%d, %d(r%d)", rx, d, ry);
            }
            break;
        /* addi */
        case OP_ADDI:
            if (ry == 0) {
                snprintf(buf, n, "li    r%d, %d", rx, (int16_t) d);
            } else {
                snprintf(buf, n, "addi  r%d, r%d, %d", rx, ry, (int16_t) d);
            }
            break;
        /* addis */
        case OP_ADDIS:
            if (ry == 0) {
                snprintf(buf, n, "addis r%d, 0, %d", rx, (int16_t) d);
            } else {
                snprintf(buf, n, "addis r%d, r%d, %d", rx, ry, (int16_t) d);
            }
            break;
        /* andi */
        case OP_ANDI:
            snprintf(buf, n, "andi  r%d, r%d, %u", rx, ry, d);
            break;
        /* ori */
        case OP_ORI:
            if (rx == 0 && ry == 0 && d == 0) {
                strncpy(buf, "nop", n);
            } else {
                snprintf(buf, n, "ori   r%d, r%d, %u", rx, ry, d);
            }
            break;
        /* cmpi */
        case OP_CMPI:
            snprintf(buf, n, "cmpi  r%d, %d", rx, d);
            break;
        /* cmp */
        case OP_CMP:
            snprintf(buf, n, "cmp   r%d, r%d", rx, ry);
            break;
        /* b */
        case OP_B:
            if (BIT(ir, 25)) {
                snprintf(buf, n, "bl    0x%06x", (uint16_t) d);
            } else {
                snprintf(buf, n, "b     0x%06x", (uint16_t) d);
            }
            break;
        /* bc */
        case OP_BC:
            if (DOWNTO(ir, 24, 22) % 4 == 0) {
                snprintf(buf, n, "%s    %d, %d, 0x%06x",
                    BIT(ir, 25) ? "bcl" : "bc ",
                    BIT(ir, 24),
                    DOWNTO(ir, 23, 22),
                    (uint16_t) d);
            } else {
                snprintf(buf, n, "%s   0x%06x", branch_s[DOWNTO(ir, 24, 22)], (uint16_t) d);
            }
            break;
        /* blr, bctr, bctrl */
        case OP_BSPR:
            if (BIT(ir, 22) == 0) {
                strncpy(buf, "blr", n);
            } else {
                if (BIT(ir, 25) == 0) {
                    strncpy(buf, "bctr", n);
                } else {
                    strncpy(buf, "bctrl", n);
                }
            }
            break;
        /* halt */
        case OP_HALT:
            strncpy(buf, "halt", n);
            break;
        /* Extended Opcode */
        case OP_XO:
            return disasm_xo(ir, buf, n);
            break;
        default:
            strncpy(buf, "unknown", n);
            break;
    }
    return 0;
}

void print_disasm_inst(unsigned int addr, int before, int after, MEMORY *m, int ex_col, BREAKPOINT *bp) {
    char disasm_str[30];

    unsigned int s = addr + before * 4;
    unsigned int e = addr + after * 4;
    unsigned int ir;
    unsigned int i;

    if (s > addr) s = 0;
    if (e > m->ir_space_size - 4) e = m->ir_space_size - 4;

    for (i = s; i <= e; i = i + 4) {
        load_instruction(&ir, m, i);
        disasm(ir, disasm_str, sizeof(disasm_str));
        if (ex_col) {
            printf("%s %c 0x%06x: %s\n",
                i == addr ? "=>" : "  ",
                check_breakpoint(i, ir, bp) ? 'B' : ' ',
                i, disasm_str);
        } else {
            printf("0x%06x: %s\n",
                i, disasm_str);
        }

        memset(disasm_str, 0, sizeof(disasm_str));
    }

}
