#include <stdio.h>
#include <stdlib.h>
#include <regex.h>

void grep_file(regex_t *pattern, FILE *file) {
    char *line = NULL;
    size_t len = 0;
    
    while (getline(&line, &len, file) != -1) {
        if (regexec(pattern, line, 0, NULL, 0) == 0) {
            fputs(line, stdout);
        }
    }
    free(line);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Используйте: %s pattern [file...]\n", argv[0]);
        return 1;
    }

    regex_t pattern;
    int ret = regcomp(&pattern, argv[1], REG_EXTENDED | REG_ICASE);
    if (ret) {
        char errbuf[100];
        regerror(ret, &pattern, errbuf, sizeof(errbuf));
        fprintf(stderr, "Regex error: %s\n", errbuf);
        return 1;
    }

    if (argc == 2) {
        grep_file(&pattern, stdin);
    } else {
        for (int i = 2; i < argc; i++) {
            FILE *file = fopen(argv[i], "r");
            if (!file) {
                perror(argv[i]);
                continue;
            }
            grep_file(&pattern, file);
            fclose(file);
        }
    }

    regfree(&pattern);
    return 0;
}
