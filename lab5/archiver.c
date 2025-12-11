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

ssize_t read_all_or_fail(int fd, void *buf, size_t size) {
    size_t read_bytes = 0;
    while (read_bytes < size) {
        ssize_t res = read(fd, (char *)buf + read_bytes, size - read_bytes);
        if (res <= 0) return -1;
        read_bytes += res;
    }
    return read_bytes;
}

void extract_and_remove(const char *archive, const char *filename) {
    int arch_fd = open(archive, O_RDONLY);
    if (arch_fd < 0) {
        perror("open archive for reading");
        exit(1);
    }

    // Создаём временный файл
    char temp_name[] = "/tmp/archXXXXXX";
    int temp_fd = mkstemp(temp_name);
    if (temp_fd < 0) {
        perror("create temp file");
        close(arch_fd);
        exit(1);
    }

    struct FileInfo info;
    int found = 0;
    char *file_data = NULL;
    struct FileInfo target_info = {0};

    while (read(arch_fd, &info, sizeof(info)) == sizeof(info)) {
        if (!info.deleted && strcmp(info.name, filename) == 0) {
            found = 1;
            target_info = info;

            file_data = malloc(info.size);
            if (!file_data) {
                perror("malloc");
                close(arch_fd);
                close(temp_fd);
                unlink(temp_name);
                exit(1);
            }

            if (read_all_or_fail(arch_fd, file_data, info.size) != info.size) {
                fprintf(stderr, "Failed to read file data\n");
                free(file_data);
                close(arch_fd);
                close(temp_fd);
                unlink(temp_name);
                exit(1);
            }
            break;
        }
        lseek(arch_fd, info.size, SEEK_CUR);
    }

    if (!found) {
        printf("Файл '%s' не найден в архиве\n", filename);
        close(arch_fd);
        close(temp_fd);
        unlink(temp_name);
        exit(1);
    }

    int out_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (out_fd < 0) {
        perror("create output file");
        free(file_data);
        close(arch_fd);
        close(temp_fd);
        unlink(temp_name);
        exit(1);
    }
    write_all(out_fd, file_data, target_info.size);
    close(out_fd);

    chmod(filename, target_info.mode);
    chown(filename, target_info.uid, target_info.gid);
    struct timespec times[2] = {{target_info.mtime, 0}, {target_info.mtime, 0}};
    utimensat(AT_FDCWD, filename, times, 0);

    free(file_data);

    lseek(arch_fd, 0, SEEK_SET);
    while (read(arch_fd, &info, sizeof(info)) == sizeof(info)) {
        if (!info.deleted && strcmp(info.name, filename) == 0) {
            lseek(arch_fd, info.size, SEEK_CUR);
            continue;
        }

        write_all(temp_fd, &info, sizeof(info));

        char buf[BUF_SIZE];
        off_t left = info.size;
        while (left > 0) {
            ssize_t chunk = (left > BUF_SIZE) ? BUF_SIZE : left;
            read_all_or_fail(arch_fd, buf, chunk);
            write_all(temp_fd, buf, chunk);
            left -= chunk;
        }
    }

    close(arch_fd);
    close(temp_fd);

    if (rename(temp_name, archive) != 0) {
        perror("rename temp archive");
        unlink(temp_name);
        exit(1);
    }

    printf("Файл '%s' извлечён и удалён из архива\n", filename);
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
        extract_and_remove(archive, argv[3]);
    } else if (strcmp(flag, "-s") == 0) {
        show_archive(archive);
    } else {
        print_help();
    }

    return 0;
}