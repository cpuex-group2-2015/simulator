#ifndef __INSTURCTION_H
#define __INSTURCTION_H

#define OP_SIM       0
#define OP_XO       31
#define OP_FP       63
#define OP_LD       32
#define OP_ST       36
#define OP_ADDI     14
#define OP_ADDIS    15
#define OP_ANDI     28
#define OP_ORI      25
#define OP_CMPI     11
#define OP_CMP      30
#define OP_B        18
#define OP_BC       16
#define OP_BLR      19
#define OP_BCTR     20
#define OP_LF       50
#define OP_STF      52
#define OP_MFGTF    21
#define OP_MFFTG    22
#define OP_SEND      1
#define OP_RECV      2

#define XO_LDX      23
#define XO_STX      151
#define XO_ADD      266
#define XO_NEG      104
#define XO_AND      28
#define XO_OR       444
#define XO_MTLR     467
#define XO_MTCTR    468
#define XO_MFSPR    339
#define XO_LFX      599
#define XO_STFX     663
#define XO_SL       24
#define XO_SR       536

#define FP_MR       72
#define FP_ADD      21
#define FP_SUB      20
#define FP_MUL      25
#define FP_INV      18
#define FP_NEG      40
#define FP_ABS      264
#define FP_SQRT     22
#define FP_CMP      0

#endif
