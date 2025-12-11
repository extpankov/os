#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

static void show_usage(const char *prog) {
    fprintf(stderr, "Usage: %s <mode> <file>\n", prog);
    exit(1);
}

static int is_valid_octal(const char *s) {
    if (!*s) return 0;
    while (*s) {
        if (*s < '0' || *s > '7') return 0;
        s++;
    }
    return 1;
}

static int apply_octal_mode(const char *mode_str, const char *path) {
    char *tail;
    errno = 0;
    long val = strtol(mode_str, &tail, 8);

    if (errno != 0 || *tail != '\0' || val < 0 || val > 07777) {
        fprintf(stderr, "chmod: invalid mode '%s'\n", mode_str);
        return 1;
    }

    if (chmod(path, (mode_t)val) == -1) {
        perror(path);
        return 1;
    }
    return 0;
}

static int apply_symbolic_mode(const char *expr, const char *path) {
    struct stat sb;
    if (stat(path, &sb) == -1) {
        perror(path);
        return 1;
    }

    mode_t current = sb.st_mode;
    mode_t new_mode = current;

    const char *p = expr;

    int target = 0;
    while (*p == 'u' || *p == 'g' || *p == 'o' || *p == 'a') {
        switch (*p) {
            case 'u': target |= 4; break;
            case 'g': target |= 2; break;
            case 'o': target |= 1; break;
            case 'a': target |= 7; break;
        }
        p++;
    }

    if (target == 0) target = 7;

    if (*p != '+' && *p != '-' && *p != '=') {
        fprintf(stderr, "chmod: invalid operator in '%s'\n", expr);
        return 1;
    }

    char op = *p++;
    mode_t to_set = 0;

    while (*p) {
        switch (*p) {
            case 'r': to_set |= S_IRUSR | S_IRGRP | S_IROTH; break;
            case 'w': to_set |= S_IWUSR | S_IWGRP | S_IWOTH; break;
            case 'x': to_set |= S_IXUSR | S_IXGRP | S_IXOTH; break;
            default:
                fprintf(stderr, "chmod: invalid permission '%c' in '%s'\n", *p, expr);
                return 1;
        }
        p++;
    }

    mode_t final_bits = 0;
    if (target & 4) final_bits |= to_set & S_IRWXU;
    if (target & 2) final_bits |= to_set & S_IRWXG;
    if (target & 1) final_bits |= to_set & S_IRWXO;

    switch (op) {
        case '+': new_mode |= final_bits; break;
        case '-': new_mode &= ~final_bits; break;
        case '=':
            mode_t clear_mask = 0;
            if (target & 4) clear_mask |= S_IRWXU;
            if (target & 2) clear_mask |= S_IRWXG;
            if (target & 1) clear_mask |= S_IRWXO;
            new_mode = (new_mode & ~clear_mask) | final_bits;
            break;
    }

    if (chmod(path, new_mode) == -1) {
        perror(path);
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        show_usage(argv[0]);
    }

    const char *mode_arg = argv[1];
    const char *file_path = argv[2];

    if (is_valid_octal(mode_arg)) {
        return apply_octal_mode(mode_arg, file_path);
    } else {
        return apply_symbolic_mode(mode_arg, file_path);
    }
}
