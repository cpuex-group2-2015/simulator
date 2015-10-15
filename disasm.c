#include <stdio.h>
#include <string.h>
#include "sim.h"

int disasm_xo(unsigned int ir, char *buf, size_t n) {
    int xo = DOWNTO(ir, 10, 1);
    int rx = DOWNTO(ir, 25, 21);
    int ry = DOWNTO(ir, 20, 16);
    int rz = DOWNTO(ir, 15, 11);

    switch(xo) {
        /* ldx */
        case 23:
            snprintf(buf, n, "ldx   r%d, r%d, r%d", rx, ry, rz);
            break;
        /* stx */
        case 151:
            snprintf(buf, n, "stx   r%d, r%d, r%d", rx, ry, rz);
            break;
        /* add */
        case 266:
            snprintf(buf, n, "add   r%d, r%d, r%d", rx, ry, rz);
            break;
        /* and */
        case 28:
            snprintf(buf, n, "and   r%d, r%d, r%d", ry, rx, rz);
            break;
        /* or */
        case 444:
            snprintf(buf, n, "or    r%d, r%d, r%d", ry, rx, rz);
            break;
        /* mtlr */
        case 467:
            snprintf(buf, n, "mtlr  r%d", rx);
            break;
        /* mflr */
        case 339:
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
        case 32:
            if (d == 0) {
                snprintf(buf, n, "ld    r%d, r%d", rx, ry);
            } else {
                snprintf(buf, n, "ld    r%d, %d(r%d)", rx, d, ry);
            }
            break;
        /* st */
        case 36:
            if (d == 0) {
                snprintf(buf, n, "st    r%d, r%d", rx, ry);
            } else {
                snprintf(buf, n, "st    r%d, %d(r%d)", rx, d, ry);
            }
            break;
        /* addi */
        case 14:
            if (ry == 0) {
                snprintf(buf, n, "li    r%d, %d", rx, (int16_t) d);
            } else {
                snprintf(buf, n, "addi  r%d, r%d, %d", rx, ry, (int16_t) d);
            }
            break;
        /* addis */
        case 15:
            if (ry == 0) {
                snprintf(buf, n, "addis r%d, 0, %d", rx, (int16_t) d);
            } else {
                snprintf(buf, n, "addis r%d, r%d, %d", rx, ry, (int16_t) d);
            }
            break;
        /* andi */
        case 28:
            snprintf(buf, n, "andi  r%d, r%d, %u", ry, rx, d);
            break;
        /* ori */
        case 25:
            if (rx == 0 && ry == 0 && d == 0) {
                strncpy(buf, "nop", n);
            } else {
                snprintf(buf, n, "ori   r%d, r%d, %u", ry, rx, d);
            }
            break;
        /* cmpi */
        case 11:
            snprintf(buf, n, "cmpi  r%d, %d", rx, d);
            break;
        /* cmp */
        case 30:
            snprintf(buf, n, "cmp   r%d, r%d", rx, ry);
            break;
        /* b */
        case 18:
            if (BIT(ir, 25)) {
                snprintf(buf, n, "bl    0x%06x", (uint16_t) d);
            } else {
                snprintf(buf, n, "b     0x%06x", (uint16_t) d);
            }
            break;
        /* bc */
        case 16:
            if (DOWNTO(ir, 24, 22) % 4 == 0) {
                snprintf(buf, n, "%s    %d, %d, 0x%06x",
                    BIT(ir, 25) ? "bcl" : "bc ",
                    BIT(ir, 24),
                    DOWNTO(ir, 23, 22),
                    (uint16_t) d);
            } else {
                snprintf(buf, n, "%s    0x%06x", branch_s[DOWNTO(ir, 24, 22)], (uint16_t) d);
            }
            break;
        /* Extended Opcode */
        case 31:
            return disasm_xo(ir, buf, n);
            break;
        default:
            strncpy(buf, "unknown", n);
            break;
    }
    return 0;
}
