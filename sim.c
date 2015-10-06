#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sim.h"

void initialize_cpu(CPU *cpu, RAM *ram, OPTION *option) {
    cpu->cr = 0;
    cpu->lr = 0;
    cpu->ctr = 0;
    memset(cpu->gpr, 0, sizeof(GPR) * GPR_LEN);
    memset(cpu->fpr, 0, sizeof(GPR) * GPR_LEN);
    cpu->pc  = option->entry_point;
    cpu->nir = __builtin_bswap32(ram->m[option->entry_point]);
}


int tick(CPU *cpu, RAM *ram, OPTION *option) {
    unsigned int ir, opcode;

    ir = cpu->nir;
    opcode = OPCODE(ir);

    /* halt */
    if (opcode == 0x3f) {
        return 0;
    }

    /* load and store */
    int rx, ry, rz;
    uint16_t v;
    int opcd, f;
    int ea;

    rx = DOWNTO(ir, 25, 21);
    ry = DOWNTO(ir, 20, 16);
    rz = DOWNTO(ir, 15, 11);
    v  = DOWNTO(ir, 15,  0);
    opcd = DOWNTO(ir, 10, 1);
    f  = ir & 1;

    switch (opcode) {
        /* lwz */
        case 32:
            ea = (ry == 0 ? 0 : cpu->gpr[ry]) + (int16_t) v;
            cpu->gpr[rx]= ram->m[ea];
            break;
        /* stw */
        case 36:
            ea = (ry == 0 ? 0 : cpu->gpr[ry]) + (int16_t) v;
            ram->m[ea] = cpu->gpr[rx];
            break;
        default:
            break;
    }

    cpu->pc = cpu->pc + 1;
    cpu->nir = __builtin_bswap32(ram->m[cpu->pc]);

    return 1;
}

int prompt(char *s, PROMPT *p) {
    char c, bufs[64], *arg;
    int target;

    printf("%s", s);
    if (fgets(bufs, 63, stdin) == NULL) {
        bufs[0] = 'r';
        printf("r\n");
    }

    c = bufs[0];
    if (c == '\n') {
        return 1;
    }
    p->command = c;
    arg = bufs + 2;

    if (c == 'p') {
        if (arg[0] == 'r') {
            if ('0' <= arg[2] && arg[2] <= '9') {
                target = (arg[1] - '0') * 10 + (arg[2] - '0');
            } else {
                target = (arg[1] - '0');
            }
            if (target < 0 || 31 < target) {
                return -1;
            } else {
                p->target = target;
            }
        }
    }
    return 1;
}

#define MODE_RUN 0
#define MODE_INTERACTIVE 1
#define MODE_STEP 2
#define MODE_QUIT 3

void sim_run(CPU *cpu, RAM *ram, OPTION *option) {
    unsigned int c = 0;
    int mode = option->interactive ? MODE_INTERACTIVE : MODE_STEP;
    PROMPT p;

    p.command = '\n';

    initialize_cpu(cpu, ram, option);
    int res = 0;
    for (;;) {
        while (mode == MODE_INTERACTIVE && (res = prompt("> ", &p))) {
            switch (p.command) {
                /* print*/
                case 'p':
                    printf("(R%d) = %d\n", p.target, cpu->gpr[p.target]);
                    break;
                /* run */
                case 'r':
                    mode = MODE_RUN; break;
                /* step */
                case 's':
                    mode = MODE_STEP; break;
                case 'q':
                    mode = MODE_QUIT; break;
                case '\n':
                    break;
                default:
                    printf("invalid command: %c(0x%x)\n", p.command, p.command);
            }
        }
        if (mode == MODE_QUIT) {
            break;
        }
        c++;
        if (tick(cpu, ram, option) == 0) {
            break;
        }
        if (mode == MODE_STEP) {
            mode = MODE_INTERACTIVE;
        }
    }

    if (mode == MODE_QUIT) {
        printf("simulation aborted\n");
    } else {
        printf("simulation completed\n");
    }
    printf("%u instructions executed\n", c);
}
