#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "sim.h"
#include "interactive.h"
#include "disasm.h"
#include "stat.h"

#define BROM_SIZE 4194304
#define SRAM_SIZE 4194304

static char *__file;

size_t load_instructions_from_file(MEMORY *mem, char *filename, size_t max_size);
size_t load_init_data_from_file(MEMORY *mem, char *filename, size_t max_size);

static const char *logo = "\n"
"  ___  __ \\_  __ \\_ |     / /__  ____/__  __ \\__  /___  ____/_  ___/_  ___/    ___  __ \\_  ____/\n"
"  __  /_/ /  / / /_ | /| / /__  __/  __  /_/ /_  / __  __/  _____ \\_____ \\     __  /_/ /  /     \n"
"  _  ____// /_/ /__ |/ |/ / _  /___  _  _, _/_  /___  /___  ____/ /____/ /     _  ____// /___   \n"
"  /_/     \\____/ ____/|__/  /_____/  /_/ |_| /_____/_____/  /____/ /____/      /_/     \\____/   \n\n";

static const char *help_string =
    "usage: sim [OPTIONS] FILE.bin\n"
    "options:\n"
    "-i               interactive mode\n"
    "-e <entry_point> set entry_point (default=0)\n"
    "-d <file_name>   inital data file (default: FILE.data) \n"
    "-a               disassemble file and exit\n"
    "-o <file_name>   output file (default: stdout)\n"
    "-s               output statistics\n";

int main(int argc, char *argv[]) {
    int c;
    int disassemble_mode = 0;
    const char *optstring = "hie:d:ao:s";
    char *filename;
    char *init_data_filename = NULL;
    OPTION option;

    option.entry_point = 0;
    option.mode = MODE_RUN;
    option.interactive = 0;
    option.fp = stdout;
    option.breakpoint = NULL;
    option.gpr_watch_list = 0;
    option.fpr_watch_list = 0;
    option.spr_watch_list = 0;
    option.disasm_always = 0;
    option.stat = NULL;

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
            case 'o':
                option.fp = fopen(optarg, "wb");
                if (option.fp == NULL) {
                    perror(__file);
                    return -1;
                }
                break;
            case 's':
                option.stat = stat_init();
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
    size_t data_space_size = 0;

    mem.brom = malloc(BROM_SIZE);
    mem.brom_size = BROM_SIZE;
    mem.sram = malloc(SRAM_SIZE);
    mem.sram_size = SRAM_SIZE;
    ir_space_size = load_instructions_from_file(&mem, filename, BROM_SIZE);

    if (init_data_filename == NULL) {
        int filename_len = strlen(filename);

        init_data_filename = malloc(filename_len + 5);
        strncpy(init_data_filename, filename, filename_len);
        if (strstr(init_data_filename + filename_len - 4, ".bin")) {
            strcpy(init_data_filename + filename_len - 4, ".data");
        } else {
            strncat(init_data_filename, ".data", filename_len + 5);
        }
    }
    data_space_size = load_init_data_from_file(&mem, init_data_filename, SRAM_SIZE);

    long long unsigned int count;
    double elapsed_time;

    if (ir_space_size > 0) { mem.ir_space_size = ir_space_size;
        if (disassemble_mode) {
            print_disasm_inst(0, 0, ir_space_size / 4, &mem, 0, NULL);
        } else {
            printf("* loaded instructions [byte]: %lu\n", ir_space_size);
            if (data_space_size > 0) {
                printf("* loaded initial data [byte]: %lu\n", data_space_size);
            }
            elapsed_time = get_dtime();
            count = sim_run(&cpu, &mem, &option);
            elapsed_time = get_dtime() - elapsed_time;

            putchar('\n');
            if (option.mode == MODE_QUIT) {
                printf("* simulation aborted at 0x%06x\n", cpu.pc);
            } else {
                printf("* simulation completed at 0x%06x\n", cpu.pc);
            }
            printf("* total executed instructions: %llu\n", count);
            printf("* elapsed time [s]: %f\n", elapsed_time);
        }
    }

    if (option.fp != stdout && option.fp != NULL) {
        fclose(option.fp);
    }
    free(mem.brom);
    free(mem.sram);
    if (option.stat != NULL) {
        stat_print(stdout, option.stat);
        stat_free(option.stat);
    }
    return 0;
}

size_t load_instructions_from_file(MEMORY *mem, char *filename, size_t max_size) {
    size_t size;
    struct stat st;

    if (stat(filename, &st) == -1) {
        fprintf(stderr, "%s: %s - ", __file, filename);
        perror("");
        return 0;
    }

    size = st.st_size;
    if (size > max_size - 4) {
        fprintf(stderr, "%s: too big file (%lu byte > %lu byte = brom)\n", __file, (unsigned long) st.st_size, max_size);
        return 0;
    }

    FILE *fp;

    if ((fp = fopen(filename, "rb")) == NULL) {
        perror(__file);
        return 0;
    }

    fread(mem->brom, 1, size, fp);
    memset(mem->brom + size, 0x00, 4);

    fclose(fp);
    return size;
}

size_t load_init_data_from_file(MEMORY *mem, char *filename, size_t max_size) {
    size_t size;
    struct stat st;

    if (stat(filename, &st) == -1) {
        fprintf(stderr, "%s: %s - ", __file, filename);
        perror("");
        return 0;
    }

    size = st.st_size;
    if (size > max_size - 4) {
        fprintf(stderr, "%s: too big file (%lu byte > %lu byte = sram)\n", __file, (unsigned long) st.st_size, max_size);
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
