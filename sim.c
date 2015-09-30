#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define RAM_SIZE 4194304

static char *__file;

size_t load_instructions_from_file(char *buf, char *filename, size_t max_size);

int main(int argc, char *argv[]) {
    int c;
    const char *optstring = "h";
    char *filename;

    __file = argv[0];

    while ((c = getopt(argc, argv, optstring)) != -1) {
        switch (c) {
            case 'h':
                printf("Usage: sim [options] file\n");
                return 0;
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

    char *ram;
    size_t ir_space_size;

    ram = malloc(RAM_SIZE);
    ir_space_size = load_instructions_from_file(ram, filename, RAM_SIZE);

    free(ram);
    return 0;
}

size_t load_instructions_from_file(char *buf, char *filename, size_t max_size) {
    size_t size;
    struct stat st;

    if (stat(filename, &st) == -1) {
        perror(__file);
        return 0;
    }

    size = st.st_size;
    if (size > max_size) {
        fprintf(stderr, "%s: too big file (%lu byte > %d byte = ram)\n", __file, st.st_size, RAM_SIZE);
        return 0;
    }

    FILE *fp;

    if ((fp = fopen(filename, "rb")) == NULL) {
        perror(__file);
        return 0;
    }

    fread(buf, 1, size, fp);

    fclose(fp);

    return size;
}
