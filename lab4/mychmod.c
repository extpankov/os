#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s mode file\n", argv[0]);
        return 1;
    }

    struct stat st;
    if (stat(argv[2], &st) == -1) {
        perror("stat");
        return 1;
    }

    mode_t mode = st.st_mode;

    // если цифры (например "755")
    if (argv[1][0] >= '0' && argv[1][0] <= '9') {
        mode = strtol(argv[1], NULL, 8);
    } else {
        char *p = argv[1];
        int who = 0;
        while (*p=='u'||*p=='g'||*p=='o'||*p=='a') {
            if (*p=='u') who |= S_IRWXU;
            if (*p=='g') who |= S_IRWXG;
            if (*p=='o') who |= S_IRWXO;
            if (*p=='a') who |= S_IRWXU|S_IRWXG|S_IRWXO;
            p++;
        }
        if (!who) who = S_IRWXU|S_IRWXG|S_IRWXO; // <- по умолчанию "a"

        char op = *p++;
        int perms = 0;

        for (; *p; p++) {
            if (*p=='r') {
                if (who & S_IRWXU) perms |= S_IRUSR;
                if (who & S_IRWXG) perms |= S_IRGRP;
                if (who & S_IRWXO) perms |= S_IROTH;
            }
            if (*p=='w') {
                if (who & S_IRWXU) perms |= S_IWUSR;
                if (who & S_IRWXG) perms |= S_IWGRP;
                if (who & S_IRWXO) perms |= S_IWOTH;
            }
            if (*p=='x') {
                if (who & S_IRWXU) perms |= S_IXUSR;
                if (who & S_IRWXG) perms |= S_IXGRP;
                if (who & S_IRWXO) perms |= S_IXOTH;
            }
        }

        if (op=='+') mode |= perms;
        if (op=='-') mode &= ~perms;
    }

    if (chmod(argv[2], mode) == -1) {
        perror("chmod");
        return 1;
    }
    return 0;
}