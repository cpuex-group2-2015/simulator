#include <stdio.h>
#include "interactive.h"
#include "sim.h"

const char *help_string =
    "interactive mode commands\n"
    " p -- print (ex. p r4 / p ir)\n"
    " s -- step into next instruction\n"
    " r -- run\n"
    " q -- quit\n"
    " h -- help\n";

int prompt(char *s, PROMPT *p) {
    char command[64], *arg;

    printf("%s", s);
    if (fgets(command, 63, stdin) == NULL) {
        command[0] = 'r';
        printf("r\n");
    }

    if (command[0] == '\n') { /* repeat */
        return 1;
    }
    p->c = command[0];
    arg = command + 2;

    if (command[0] == 'p') {
        if (arg[0] == 'r') {
            if ('0' <= arg[2] && arg[2] <= '9') {
                p->target = (arg[1] - '0') * 10 + (arg[2] - '0');
            } else {
                p->target = (arg[1] - '0');
            }
        } else if (arg[0] == 'i' && arg[1] == 'r') {
            p->target = -1;
        }
    }
    return 1;
}

void interactive_print(CPU *cpu, int t) {
    if (t == -1) {
        printf("(IR) = %08x\n", cpu->nir);
    } else if (0 <= t && t <= 32) {
        printf("(R%d) = %d\n", t, cpu->gpr[t]);
    } else {
        printf("invalid target\n");
    }
}

int interactive_prompt(CPU *cpu, MEMORY *m, int *mode) {
    int cont = 1;
    int nextmode = *mode;
    int res;
    char prompt_str[15];
    static PROMPT p;

    sprintf(prompt_str, "0x%06x> ", cpu->pc);
    while (cont && (res = prompt(prompt_str, &p))) {
        switch (p.c) {
            case 'p': /* print */
                interactive_print(cpu, p.target);
                break;
            case 'r': /* run */
                cont = 0; nextmode = MODE_RUN;
                break;
            case 's': /* step */
                cont = 0; nextmode = MODE_INTERACTIVE;
                break;
            case 'q': /* quit */
                cont = 0; nextmode = MODE_QUIT;
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
    *mode = nextmode;
    return 0;
}
