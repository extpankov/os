#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

typedef struct {
    unsigned number_lines   : 1;
    unsigned number_nonblank: 1;
    unsigned show_ends      : 1;
} Options;

void print_usage(const char *prog) {
    fprintf(stderr,
        "Использование: %s [ОПЦИЯ]... [ФАЙЛ]...\n"
        "  -n    нумеровать все строки\n"
        "  -b    нумеровать только непустые строки\n"
        "  -E    показывать $ в конце строк\n"
        "  -h    помощь\n", prog);
}

void process_stream(FILE *file, Options *opts, int *line_number) {
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), file)) {
        size_t len = strlen(buffer);
        int empty = (len == 0 || (len == 1 && buffer[0] == '\n'));

        if ((opts->number_nonblank && !empty) || opts->number_lines)
            printf("%6d\t", (*line_number)++);

        if (opts->show_ends && buffer[len-1] == '\n') {
            buffer[len-1] = '$';
            buffer[len]   = '\n';
            buffer[len+1] = '\0';
        }
        fputs(buffer, stdout);
    }
}

int main(int argc, char *argv[]) {
    Options opts = {0};
    int opt, line_number = 1;

    while ((opt = getopt(argc, argv, "nbEh")) != -1) {
        switch (opt) {
            case 'n': opts.number_lines = 1; break;
            case 'b': opts.number_nonblank = 1; break;
            case 'E': opts.show_ends = 1; break;
            case 'h': print_usage(argv[0]); return 0;
            default : print_usage(argv[0]); return 1;
        }
    }

    if (optind == argc) {
        process_stream(stdin, &opts, &line_number);
    } else {
        for (int i = optind; i < argc; i++) {
            FILE *file = strcmp(argv[i], "-") ? fopen(argv[i], "r") : stdin;
            if (!file) {
                fprintf(stderr, "%s: %s: %s\n", argv[0], argv[i], strerror(errno));
                continue;
            }
            process_stream(file, &opts, &line_number);
            if (file != stdin) fclose(file);
        }
    }
    return 0;
}