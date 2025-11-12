#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>

#define MAX_NAME 256
#define BUF_SIZE 4096

struct FileInfo {
    char name[MAX_NAME];
    mode_t mode;
    uid_t uid;
    gid_t gid;
    off_t size;
    time_t mtime;
    int deleted;
};

void write_all(int fd, const void *buf, size_t size) {
    size_t written = 0;
    while (written < size) {
        ssize_t res = write(fd, (const char *)buf + written, size - written);
        if (res <= 0) {
            perror("write");
            exit(1);
        }
        written += res;
    }
}

void read_all(int fd, void *buf, size_t size) {
    size_t read_bytes = 0;
    while (read_bytes < size) {
        ssize_t res = read(fd, (char *)buf + read_bytes, size - read_bytes);
        if (res <= 0) {
            perror("read");
            exit(1);
        }
        read_bytes += res;
    }
}

void add_file(const char *archive, const char *filename) {
    int arch = open(archive, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (arch < 0) {
        perror("open archive");
        exit(1);
    }

    struct stat st;
    if (stat(filename, &st) == -1) {
        perror("stat");
        exit(1);
    }

    struct FileInfo info;
    memset(&info, 0, sizeof(info));
    strncpy(info.name, filename, MAX_NAME - 1);
    info.mode = st.st_mode;
    info.uid = st.st_uid;
    info.gid = st.st_gid;
    info.size = st.st_size;
    info.mtime = st.st_mtime;
    info.deleted = 0;

    write_all(arch, &info, sizeof(info));

    int in = open(filename, O_RDONLY);
    if (in < 0) {
        perror("open input");
        exit(1);
    }

    char buf[BUF_SIZE];
    ssize_t n;
    while ((n = read(in, buf, sizeof(buf))) > 0) {
        write_all(arch, buf, n);
    }

    close(in);
    close(arch);
    printf("Файл '%s' добавлен в архив '%s'\n", filename, archive);
}

void extract_file(const char *archive, const char *filename) {
    int arch = open(archive, O_RDWR);
    if (arch < 0) {
        perror("open archive");
        exit(1);
    }

    struct FileInfo info;
    off_t pos = 0;
    int found = 0;

    while (read(arch, &info, sizeof(info)) == sizeof(info)) {
        off_t data_pos = lseek(arch, 0, SEEK_CUR);

        if (!info.deleted && strcmp(info.name, filename) == 0) {
            found = 1;
            int out = open(info.name, O_WRONLY | O_CREAT | O_TRUNC, 0600);
            if (out < 0) {
                perror("create output");
                exit(1);
            }

            char buf[BUF_SIZE];
            off_t left = info.size;
            while (left > 0) {
                ssize_t chunk = (left > BUF_SIZE) ? BUF_SIZE : left;
                read_all(arch, buf, chunk);
                write_all(out, buf, chunk);
                left -= chunk;
            }

            close(out);

            chmod(info.name, info.mode);
            chown(info.name, info.uid, info.gid);
            struct timespec times[2];
            times[0].tv_sec = info.mtime;
            times[0].tv_nsec = 0;
            times[1] = times[0];
            utimensat(AT_FDCWD, info.name, times, 0);

            info.deleted = 1;
            lseek(arch, pos, SEEK_SET);
            write_all(arch, &info, sizeof(info));

            printf("Файл '%s' извлечён и удалён из архива\n", filename);
            close(arch);
            return;
        }

        lseek(arch, info.size, SEEK_CUR);
        pos = lseek(arch, 0, SEEK_CUR);
    }

    close(arch);
    if (!found)
        printf("Файл '%s' не найден в архиве\n", filename);
}

void show_archive(const char *archive) {
    int arch = open(archive, O_RDONLY);
    if (arch < 0) {
        perror("open archive");
        exit(1);
    }

    struct FileInfo info;
    printf("Содержимое архива '%s':\n", archive);
    printf("%-20s %-10s %-10s %-10s %-6s\n", "Имя", "Размер", "UID", "GID", "Del");
    while (read(arch, &info, sizeof(info)) == sizeof(info)) {
        printf("%-20s %-10ld %-10d %-10d %-6d\n",
               info.name, (long)info.size, info.uid, info.gid, info.deleted);
        lseek(arch, info.size, SEEK_CUR);
    }
    close(arch);
}

void print_help() {
    printf("Использование:\n");
    printf("  ./archiver archive -i file   добавить файл в архив\n");
    printf("  ./archiver archive -e file   извлечь файл из архива и удалить\n");
    printf("  ./archiver archive -s        показать содержимое архива\n");
    printf("  ./archiver -h                справка\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_help();
        return 1;
    }

    if (strcmp(argv[1], "-h") == 0) {
        print_help();
        return 0;
    }

    if (argc < 3) {
        print_help();
        return 1;
    }

    char *archive = argv[1];
    char *flag = argv[2];

    if (strcmp(flag, "-i") == 0 && argc >= 4) {
        add_file(archive, argv[3]);
    } else if (strcmp(flag, "-e") == 0 && argc >= 4) {
        extract_file(archive, argv[3]);
    } else if (strcmp(flag, "-s") == 0) {
        show_archive(archive);
    } else {
        print_help();
    }

    return 0;
}