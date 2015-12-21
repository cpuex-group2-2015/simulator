#include <stdio.h>
#include <stdlib.h>
#include "instruction.h"
#include "stat.h"

STAT *stat_init() {
    STAT *s = NULL;
    s = stat_add(s, OP_LD   , 0, "ld");
    s = stat_add(s, OP_ADDI , 0, "addi");
    s = stat_add(s, OP_ADDIS, 0, "addis");
    s = stat_add(s, OP_ANDI , 0, "andi");
    s = stat_add(s, OP_ORI  , 0, "ori");
    s = stat_add(s, OP_CMPI , 0, "cmpi");
    s = stat_add(s, OP_CMP  , 0, "cmp");
    s = stat_add(s, OP_B    , 0, "b");
    s = stat_add(s, OP_BC   , 0, "bc");
    s = stat_add(s, OP_BLR  , 0, "blr");
    s = stat_add(s, OP_BCTR , 0, "bctr");
    s = stat_add(s, OP_LF   , 0, "lf");
    s = stat_add(s, OP_STF  , 0, "stf");
    s = stat_add(s, OP_MFGTF, 0, "mfgtf");
    s = stat_add(s, OP_MFFTG, 0, "mfftg");
    s = stat_add(s, OP_SEND , 0, "send");
    s = stat_add(s, OP_RECV , 0, "recv");
    s = stat_add(s, OP_XO   , XO_LDX  , "ldx");
    s = stat_add(s, OP_XO   , XO_STX  , "stx");
    s = stat_add(s, OP_XO   , XO_ADD  , "add");
    s = stat_add(s, OP_XO   , XO_NEG  , "neg");
    s = stat_add(s, OP_XO   , XO_AND  , "and");
    s = stat_add(s, OP_XO   , XO_OR   , "or");
    s = stat_add(s, OP_XO   , XO_MTLR , "mtlr");
    s = stat_add(s, OP_XO   , XO_MTCTR, "mtctr");
    s = stat_add(s, OP_XO   , XO_MFSPR, "mfspr");
    s = stat_add(s, OP_XO   , XO_LFX  , "lfx");
    s = stat_add(s, OP_XO   , XO_STFX , "stfx");
    s = stat_add(s, OP_XO   , XO_SL   , "sl");
    s = stat_add(s, OP_XO   , XO_SR   , "sr");
    s = stat_add(s, OP_FP   , FP_MR   , "fmr");
    s = stat_add(s, OP_FP   , FP_ADD  , "fadd");
    s = stat_add(s, OP_FP   , FP_SUB  , "fsub");
    s = stat_add(s, OP_FP   , FP_MUL  , "fmul");
    s = stat_add(s, OP_FP   , FP_INV  , "finv");
    s = stat_add(s, OP_FP   , FP_NEG  , "fneg");
    s = stat_add(s, OP_FP   , FP_ABS  , "fabs");
    s = stat_add(s, OP_FP   , FP_SQRT , "fsqrt");
    s = stat_add(s, OP_FP   , FP_CMP  , "fcmp");

    return s;
}

STAT *stat_add(STAT *s, int op, int xo, char *label) {
    STAT *stat;

    int opcd;

    if (op == OP_XO || op == OP_FP) {
        opcd = op << 10 | xo;
    } else {
        opcd = op << 10;
    }

    if (s == NULL) {
        if (label != NULL) {
            stat = malloc(sizeof(STAT));
            stat->opcd = opcd;
            stat->n = 0;
            stat->label = label;
            return stat;
        } else {
            return s;
        }
    } else {
        if (s->opcd == opcd) {
            s->n = s->n + 1;
        } else if (s->opcd < opcd) {
            s->r = stat_add(s->r, op, xo, label);
        } else {
            s->l = stat_add(s->l, op, xo, label);
        }
        return s;
    }
}

void stat_free(STAT *s) {
    if (s != NULL) {
        if (s->l != NULL) {
            stat_free(s->l);
        }
        if (s->r != NULL) {
            stat_free(s->r);
        }
        free(s);
    }
}

void stat_print(FILE *fp, STAT *s) {
    if (s != NULL) {
        fprintf(fp, "%-6s %llu\n", s->label, s->n);
        stat_print(fp, s->l);
        stat_print(fp, s->r);
    }
}
