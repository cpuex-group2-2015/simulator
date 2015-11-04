#ifndef __INSTURCTION_H
#define __INSTURCTION_H

#define OP_HALT     63
#define OP_XO       31
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
#define OP_SENDRECV 0

#define XO_LDX      23
#define XO_STX      151
#define XO_ADD      266
#define XO_AND      28
#define XO_OR       444
#define XO_MTLR     467
#define XO_MFLR     339
#endif