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
    " P  -- print always\n"
    " s  -- step into next instruction\n"
    " so -- step over into next instruction\n"
    " r  -- run\n"
    " d  -- disassemble current instruction\n"
    " D  -- disassemble always\n"
    " b  -- set breakpoint\n"
    " bl -- show breakpoint list\n"
    " br -- remove breakpoint\n"
    " w  -- watch (same as 'P')\n"
    " v  -- view memory data (ex. v r3)\n"
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

char *parse_int16(char *s, int *buf) {
    int n = 0;

    if (!(('0' <= *s && *s <= '9') || ('a' <= *s && *s <= 'f'))) {
        return s;
    }

    for (;;) {
        if ('0' <= *s && *s <= '9') {
            n = n * 16 + (*s - '0');
        } else if ('a' <= *s && *s <= 'f') {
            n = n * 16 + (*s - 'a' + 10);
        } else if (*s != 'x') {
            break;
        }
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
    int n;
    if (s[0] == 'r') {
        parse_int10(s + 1, &n);
        *buf = n;
    } else if (s[0] == 'f') {
        if (s[1] == 'r') {
            s = s + 1;
        }
        parse_int10(s + 1, &n);
        *buf = n + 32;
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

    if (c == 'p' || c == 'P' || c == 'w') {
        p->c = c == 'p' ? ICMD_PRINT : ICMD_WATCH;

        arg = command + 1;

        arg = parse_blank(arg);
        arg = parse_register(arg, &p->target);
    }
    else if (c == 's') {
        if (command[1] == 'o') {
            p->c = ICMD_STEPOVER;
        } else {
            p->c = ICMD_STEP;
        }
    }
    else if (c == 'r') p->c = ICMD_RUN;
    else if (c == 'd') p->c = ICMD_DISASM;
    else if (c == 'D') p->c = ICMD_DISASM_ALWAYS;
    else if (c == 'b') {
        if      (command[1] == 'l') p->c = ICMD_BPSHOW;
        else if (command[1] == 'r') p->c = ICMD_BPREMOVE;
        else                        p->c = ICMD_BPSET;

        if (p->c == ICMD_BPSET) {
            p->target = -1;
            arg = command + 2;
            arg = parse_blank(arg);
            arg = parse_int16(arg, &p->target);
        } else if (p->c == ICMD_BPREMOVE) {
            arg = command + 2;
            arg = parse_blank(arg);
            arg = parse_int10(arg, &p->target);
        }
    }
    else if (c == 'l') p->c = ICMD_BPSHOW;
    else if (c == 'v') {
        p->c = ICMD_VIEW_REGISTER;
        arg = command + 1;
        arg = parse_blank(arg);
        arg = parse_register(arg, &p->target);

    }
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
        printf("(R%d) = %d (0x%08x)\n", t, cpu->gpr[t], cpu->gpr[t]);
    } else if (32 <= t && t <= 63) {
        t = t - 32;
        printf("(FR%d) = %f (0x%08x)\n", t, ui2f(cpu->fpr[t]), cpu->fpr[t]);
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
    for (i = 0; i < 32; i++) {
        if (BIT(option->fpr_watch_list, i)) {
            interactive_print(cpu, i + 32);
        }
    }
}

void interactive_watch_add(OPTION *option, int t) {
    if (t == PRINT_TARGET_IR) {
    } else if (t == PRINT_TARGET_CR) {
    } else if (t == PRINT_TARGET_LR) {
    } else if (0  <= t && t <= 31) {
        option->gpr_watch_list |= 1 << t;
    } else if (32 <= t && t <= 63) {
        option->fpr_watch_list |= 1 << t;
    } else {
        printf("invalid target\n");
    }
}

void interactive_view_register(CPU *cpu, MEMORY *memory, int t, int n) {
    unsigned int addr;
    uint32_t data;

    if (t < 0 || 31 < t) {
        t = 3;
    }

    addr = cpu->gpr[t];
    load_from_sram(&data, memory, addr, sizeof(uint32_t));

    printf("MEM[R%d] = 0x%08x\n", t, data);
}

int interactive_prompt(CPU *cpu, MEMORY *m, OPTION *option) {
    int cont = 1;
    int res;
    char prompt_str[15];
    static PROMPT p;
    BREAKPOINT *bp;

    interactive_watch(cpu, option);
    if (option->disasm_always) {
        print_disasm_inst(cpu->pc, -1, 5, m, 1, option->breakpoint);
    }
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
            case ICMD_STEPOVER:
                cont = 0;
                option->mode = MODE_STEPOVER;
                option->stepover_addr = cpu->pc + 4;
                break;
            case ICMD_DISASM:
                print_disasm_inst(cpu->pc, -1, 5, m, 1, option->breakpoint);
                break;
            case ICMD_DISASM_ALWAYS:
                print_disasm_inst(cpu->pc, -1, 5, m, 1, option->breakpoint);
                option->disasm_always = 1;
                break;
            case ICMD_BPSET:
                if (p.target == -1) {
                    bp = set_breakpoint_addr(cpu->pc, option->breakpoint);
                } else {
                    bp = set_breakpoint_addr(p.target, option->breakpoint);
                }
                option->breakpoint = bp;
                printf("set breakpoint %d at 0x%06x\n", bp->n, bp->target.addr);
                break;
            case ICMD_BPSHOW:
                print_breakpoint_list(option->breakpoint);
                break;
            case ICMD_BPREMOVE:
                printf("remove breakpoint %d\n", p.target);
                bp = remove_breakpoint(p.target, bp);
                break;
            case ICMD_WATCH:
                interactive_watch_add(option, p.target);
                interactive_print(cpu, p.target);
                break;
            case ICMD_VIEW_REGISTER:
                interactive_view_register(cpu, m, p.target, 16);
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
