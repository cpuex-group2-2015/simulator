#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "sim.h"

#define BROM_SIZE 4194304
#define SRAM_SIZE   4194304

static char *__file;

size_t load_instructions_from_file(MEMORY *mem, char *filename, size_t max_size);

int main(int argc, char *argv[]) {
    int c;
    const char *optstring = "hi";
    char *filename;
    OPTION option;

    option.entry_point = 0;
    option.interactive = 0;
    option.fp = stdout;

    __file = argv[0];

    while ((c = getopt(argc, argv, optstring)) != -1) {
        switch (c) {
            case 'h':
                printf("usage: sim [options] file\n"
                       "options:\n"
                       "-i    interactive mode\n");
                return 0;
                break;
            case 'i':
                option.interactive = 1;
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

    if (ir_space_size > 0) {
        printf("loaded %lu byte\n", ir_space_size);
        sim_run(&cpu, &mem, &option);
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
        fprintf(stderr, "%s: too big file (%lu byte > %lu byte = ram)\n", __file, st.st_size, max_size);
        return 0;
    }

    FILE *fp;

    if ((fp = fopen(filename, "rb")) == NULL) {
        perror(__file);
        return 0;
    }

    fread(mem->brom, 1, size, fp);
    memset(mem->brom + size, 0xff, 4);

    /* ram->m[size] = 0xffffffff; *//* Guard */

    fclose(fp);

    return size;
}
