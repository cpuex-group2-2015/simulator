#include <stdio.h>
#include <string.h>
#include "interactive.h"
#include "sim.h"
#include "util.h"
#include "disasm.h"
#include "breakpoint.h"

const char *help_string =
    "interactive mode commands\n"
    " p  -- print (ex. p r4 / p ir)\n"
    " s  -- step into next instruction\n"
    " r  -- run\n"
    " d  -- disassemble current instruction\n"
    " b  -- set breakpoint\n"
    " bl -- show breakpoint list\n"
    " br -- remove breakpoint\n"
    " w  -- watch (print register always)\n"
    " q  -- quit\n"
    " h  -- help\n";

#define PRINT_TARGET_IR -1
#define PRINT_TARGET_CR -2
#define PRINT_TARGET_LR -3

char *parse_int10(char *s, int *buf) {
    int n = 0;

    while ('0' <= *s && *s <= '9') {
        n = n * 10 + (*s - '0');
        s = s + 1;
    }

    *buf = n;
    return s;
}

char *parse_blank(char *s) {
    while (s[0] == ' ') s = s + 1;
    return s;
}

char *parse_register(char *s, int *buf) {
    if (s[0] == 'r') {
        if ('0' <= s[2] && s[2] <= '9') {
            *buf = (s[1] - '0') * 10 + (s[2] - '0');
        } else {
            *buf = (s[1] - '0');
        }
    } else if (s[0] == 'i' && s[1] == 'r') {
        *buf = PRINT_TARGET_IR;
    } else if (s[0] == 'c' && s[1] == 'r') {
        *buf = PRINT_TARGET_CR;
    } else if (s[0] == 'l' && s[1] == 'r') {
        *buf = PRINT_TARGET_LR;
    }
    return s;
}

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
    c = command[0];

    if (c == 'p' || c == 'w') {
        p->c = c == 'p' ? ICMD_PRINT : ICMD_WATCH;

        arg = command + 1;

        arg = parse_blank(arg);
        arg = parse_register(arg, &p->target);
    }
    else if (c == 's') p->c = ICMD_STEP;
    else if (c == 'r') p->c = ICMD_RUN;
    else if (c == 'd') p->c = ICMD_DISASM;
    else if (c == 'b') {
        if      (command[1] == 'l') p->c = ICMD_BPSHOW;
        else if (command[1] == 'r') p->c = ICMD_BPREMOVE;
        else                        p->c = ICMD_BPSET;
        printf("BREAKPOINT: %d\n", p->c);

        if (p->c == ICMD_BPREMOVE) {
            arg = command + 2;
            arg = parse_blank(arg);
            arg = parse_int10(arg, &p->target);
        }
    }
    else if (c == 'l') p->c = ICMD_BPSHOW;
    else if (c == 'q') p->c = ICMD_QUIT;
    else if (c == 'h') p->c = ICMD_HELP;
    else {
        p->c = ICMD_INVALID;
        p->target = c;
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
    } else if (0 <= t && t <= 31) {
        printf("(R%d) = %d\n", t, cpu->gpr[t]);
    } else {
        printf("invalid target\n");
    }
}

void interactive_watch(CPU *cpu, OPTION *option) {
    int i;

    for (i = 0; i < 32; i++) {
        if (BIT(option->gpr_watch_list, i)) {
            interactive_print(cpu, i);
        }
    }
}

void interactive_watch_add(OPTION *option, int t) {
    if (t == PRINT_TARGET_IR) {
    } else if (t == PRINT_TARGET_CR) {
    } else if (t == PRINT_TARGET_LR) {
    } else if (0  <= t && t <= 31) {
        option->gpr_watch_list |= 1 << t;
    } else {
        printf("invalid target\n");
    }
}

int interactive_prompt(CPU *cpu, MEMORY *m, OPTION *option) {
    int cont = 1;
    int res;
    char prompt_str[15];
    static PROMPT p;
    BREAKPOINT *bp;

    interactive_watch(cpu, option);
    sprintf(prompt_str, "0x%06x> ", cpu->pc);
    while (cont && (res = prompt(prompt_str, &p))) {
        switch (p.c) {
            case ICMD_PRINT: /* print */
                interactive_print(cpu, p.target);
                break;
            case ICMD_RUN: /* run */
                cont = 0; option->mode = MODE_RUN;
                break;
            case ICMD_STEP: /* step */
                cont = 0;
                break;
            case ICMD_DISASM:
                print_disasm_inst(cpu->pc, -1, 5, m, 1, option->breakpoint);
                break;
            case ICMD_BPSET:
                bp = set_breakpoint_addr(cpu->pc, option->breakpoint);
                option->breakpoint = bp;
                printf("set breakpoint %d at 0x%06x\n", bp->n, cpu->pc);
                break;
            case ICMD_BPSHOW:
                print_breakpoint_list(option->breakpoint);
                break;
            case ICMD_BPREMOVE:
                printf("remove breakpoint %d\n", p.target);
                bp = remove_breakpoint(p.target, bp);
                break;
            case ICMD_WATCH:
                printf("watch\n");
                interactive_watch_add(option, p.target);
                break;
            case ICMD_QUIT: /* quit */
                cont = 0; option->mode = MODE_QUIT;
                break;
            case ICMD_HELP: /* help */
                puts(help_string);
                break;
            case ICMD_NOP: /* do nothing */
                break;
            default: /* invalid command */
                printf("invalid command: %c\n", p.target);
        }
    }
    return 0;
}
