#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "sim.h"
#include "disasm.h"

#define BROM_SIZE 4194304
#define SRAM_SIZE   4194304

static char *__file;

size_t load_instructions_from_file(MEMORY *mem, char *filename, size_t max_size);
size_t load_init_data_from_file(MEMORY *mem, char *filename, size_t max_size);

static const char *logo = "\n"
"  ___  __ \\_  __ \\_ |     / /__  ____/__  __ \\__  /___  ____/_  ___/_  ___/    ___  __ \\_  ____/\n"
"  __  /_/ /  / / /_ | /| / /__  __/  __  /_/ /_  / __  __/  _____ \\_____ \\     __  /_/ /  /     \n"
"  _  ____// /_/ /__ |/ |/ / _  /___  _  _, _/_  /___  /___  ____/ /____/ /     _  ____// /___   \n"
"  /_/     \\____/ ____/|__/  /_____/  /_/ |_| /_____/_____/  /____/ /____/      /_/     \\____/   \n\n";

static const char *help_string =
    "usage: sim [options] file\n"
    "options:\n"
    "-i               interactive mode\n"
    "-e <entry_point> set entry_point (default=0)\n"
    "-d <file_name>   inital data file\n"
    "-a               disassemble file and exit\n";

int main(int argc, char *argv[]) {
    int c;
    int disassemble_mode = 0;
    const char *optstring = "hie:d:a";
    char *filename;
    char *init_data_filename = NULL;
    OPTION option;

    option.entry_point = 0;
    option.mode = MODE_RUN;
    option.interactive = 0;
    option.fp = stdout;
    option.breakpoint = NULL;

    __file = argv[0];

    while ((c = getopt(argc, argv, optstring)) != -1) {
        switch (c) {
            case 'h':
                puts(logo);
                puts(help_string);
                return 0;
                break;
            case 'i':
                option.interactive = 1;
                option.mode = MODE_INTERACTIVE;
                break;
            case 'e':
                option.entry_point = atoi(optarg);
                break;
            case 'd':
                init_data_filename = optarg;
                break;
            case 'a':
                disassemble_mode = 1;
                break;
            case '?':
                return -1;
                break;
            default:
                break;
        }
    }

    filename = argv[optind];
    if (filename == NULL) {
        fprintf(stderr, "%s: No input file\n", __file);
        return -1;
    }

    CPU cpu;
    MEMORY mem;
    size_t ir_space_size;

    mem.brom = malloc(BROM_SIZE);
    mem.brom_size = BROM_SIZE;
    mem.sram = malloc(SRAM_SIZE);
    mem.sram_size = SRAM_SIZE;
    ir_space_size = load_instructions_from_file(&mem, filename, BROM_SIZE);
    if (init_data_filename != NULL) {
        load_init_data_from_file(&mem, init_data_filename, SRAM_SIZE);
    }

    if (ir_space_size > 0) {
        mem.ir_space_size = ir_space_size;
        if (disassemble_mode) {
            print_disasm_inst(0, 0, ir_space_size / 4, &mem, 0, NULL);
        } else {
            printf("loaded %lu byte\n", ir_space_size);
            sim_run(&cpu, &mem, &option);
        }
    }

    free(mem.brom);
    free(mem.sram);
    return 0;
}

size_t load_instructions_from_file(MEMORY *mem, char *filename, size_t max_size) {
    size_t size;
    struct stat st;

    if (stat(filename, &st) == -1) {
        perror(__file);
        return 0;
    }

    size = st.st_size;
    if (size > max_size - 4) {
        fprintf(stderr, "%s: too big file (%lu byte > %lu byte = brom)\n", __file, st.st_size, max_size);
        return 0;
    }

    FILE *fp;

    if ((fp = fopen(filename, "rb")) == NULL) {
        perror(__file);
        return 0;
    }

    fread(mem->brom, 1, size, fp);
    memset(mem->brom + size, 0xff, 4);

    fclose(fp);
    return size;
}

size_t load_init_data_from_file(MEMORY *mem, char *filename, size_t max_size) {
    size_t size;
    struct stat st;

    if (stat(filename, &st) == -1) {
        perror(__file);
        return 0;
    }

    size = st.st_size;
    if (size > max_size - 4) {
        fprintf(stderr, "%s: too big file (%lu byte > %lu byte = sram)\n", __file, st.st_size, max_size);
        return 0;
    }

    FILE *fp;

    if ((fp = fopen(filename, "rb")) == NULL) {
        perror(__file);
        return 0;
    }

    fread(mem->sram, 1, size, fp);

    fclose(fp);
    return size;
}
