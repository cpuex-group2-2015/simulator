#include <stdio.h>
#include "interactive.h"
#include "sim.h"
#include "util.h"
#include "disasm.h"

const char *help_string =
    "interactive mode commands\n"
    " p -- print (ex. p r4 / p ir)\n"
    " s -- step into next instruction\n"
    " d -- disassemble current instruction\n"
    " r -- run\n"
    " q -- quit\n"
    " h -- help\n";

#define PRINT_TARGET_IR -1
#define PRINT_TARGET_CR -2
#define PRINT_TARGET_LR -3

int prompt(char *s, PROMPT *p) {
    char c, command[64], *arg;

    printf("%s", s);
    if (fgets(command, 63, stdin) == NULL) {
        command[0] = 'q';
        printf("q\n");
    }

    if (command[0] == '\n') { /* repeat */
        return 1;
    }
    p->c = c = command[0];

    arg = command + 1;
    while (arg[0] == ' ') arg = arg + 1;

    if (c == 'p') {
        if (arg[0] == 'r') {
            if ('0' <= arg[2] && arg[2] <= '9') {
                p->target = (arg[1] - '0') * 10 + (arg[2] - '0');
            } else {
                p->target = (arg[1] - '0');
            }
        } else if (arg[0] == 'i' && arg[1] == 'r') {
            p->target = PRINT_TARGET_IR;
        } else if (arg[0] == 'c' && arg[1] == 'r') {
            p->target = PRINT_TARGET_CR;
        } else if (arg[0] == 'l' && arg[1] == 'r') {
            p->target = PRINT_TARGET_LR;
        }
    }
    return 1;
}

void interactive_print(CPU *cpu, int t) {
    if (t == PRINT_TARGET_IR) {
        printf("(IR) = %08x\n", cpu->nir);
    } else if (t == PRINT_TARGET_CR) {
        printf("(CR) = %d%d%d%d\n",
                BIT(cpu->cr, 3), BIT(cpu->cr, 2),
                BIT(cpu->cr, 1), BIT(cpu->cr, 0));
    } else if (t == PRINT_TARGET_LR) {
        printf("(LR) = 0x%06x\n", cpu->lr);
    } else if (0 <= t && t <= 32) {
        printf("(R%d) = %d\n", t, cpu->gpr[t]);
    } else {
        printf("invalid target\n");
    }
}

int interactive_prompt(CPU *cpu, MEMORY *m, OPTION *option) {
    int cont = 1;
    int res;
    char disasm_str[30];
    char prompt_str[15];
    static PROMPT p;

    sprintf(prompt_str, "0x%06x> ", cpu->pc);
    while (cont && (res = prompt(prompt_str, &p))) {
        switch (p.c) {
            case 'p': /* print */
                interactive_print(cpu, p.target);
                break;
            case 'r': /* run */
                cont = 0; option->mode = MODE_RUN;
                break;
            case 's': /* step */
                cont = 0;
                break;
            case 'd':
                disasm(cpu->nir, disasm_str, sizeof(disasm_str));
                printf("   0x%06x: %s\n", cpu->pc, disasm_str);
                break;
            case 'q': /* quit */
                cont = 0; option->mode = MODE_QUIT;
                break;
            case 'h': /* help */
                puts(help_string);
                break;
            case '\0': /* do nothing */
            case '\n': /* do nothing */
                break;
            default: /* invalid command */
                printf("invalid command: %c\n", p.c);
        }
    }
    return 0;
}
