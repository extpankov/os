#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#define BLUE "\033[1;34m"
#define GREEN "\033[1;32m"
#define CYAN "\033[1;36m"
#define RESET "\033[0m"

typedef struct {
    int show_all, long_format;
} Options;

typedef struct {
    char name[256];
    struct stat info;
} FileEntry;

void print_colored(const char* name, mode_t mode) {
    const char *color = S_ISDIR(mode) ? BLUE : 
                       (mode & S_IXUSR) ? GREEN :
                       S_ISLNK(mode) ? CYAN : "";
    printf("%s%s" RESET, color, name);
}

void print_permissions(mode_t mode) {
    printf("%c%c%c%c%c%c%c%c%c%c ",
        S_ISDIR(mode) ? 'd' : '-',
        mode & S_IRUSR ? 'r' : '-',
        mode & S_IWUSR ? 'w' : '-',
        mode & S_IXUSR ? 'x' : '-',
        mode & S_IRGRP ? 'r' : '-',
        mode & S_IWGRP ? 'w' : '-',
        mode & S_IXGRP ? 'x' : '-',
        mode & S_IROTH ? 'r' : '-',
        mode & S_IWOTH ? 'w' : '-',
        mode & S_IXOTH ? 'x' : '-');
}

void print_long_format(FileEntry *files, int count) {
    long total = 0;
    for (int i = 0; i < count; total += files[i++].info.st_blocks);
    printf("total %ld\n", total);

    for (int i = 0; i < count; i++) {
        struct stat *info = &files[i].info;
        print_permissions(info->st_mode);
        
        struct passwd *pw = getpwuid(info->st_uid);
        struct group *gr = getgrgid(info->st_gid);
        
        printf("%ld %-8s %-8s %8ld %.12s ",
               (long)info->st_nlink,
               pw ? pw->pw_name : "?",
               gr ? gr->gr_name : "?",
               (long)info->st_size,
               ctime(&info->st_mtime) + 4);
               
        print_colored(files[i].name, info->st_mode);
        putchar('\n');
    }
}

void print_short_format(FileEntry *files, int count, int width) {
    int cols = 80 / (width + 2);
    cols = cols ? cols : 1;
    
    for (int i = 0; i < count; i++) {
        print_colored(files[i].name, files[i].info.st_mode);
        printf((i + 1) % cols && i != count - 1 ? "%*s" : "\n", 
               width - (int)strlen(files[i].name) + 2, "");
    }
}

int compare_files(const FileEntry *a, const FileEntry *b) {
    return strcasecmp(a->name, b->name);
}

void process_dir(const char *dirname, Options opts) {
    DIR *dir = opendir(dirname);
    if (!dir) { perror("opendir"); return; }

    FileEntry files[1024];
    int count = 0, max_width = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) && count < 1024) {
        if (!opts.show_all && entry->d_name[0] == '.') continue;

        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);
        if (lstat(path, &files[count].info) == -1) {
            perror("lstat"); 
            continue;
        }

        strncpy(files[count].name, entry->d_name, 255);
        files[count].name[255] = '\0';
        int len = strlen(entry->d_name);
        if (len > max_width) max_width = len;
        count++;
    }
    closedir(dir);

    qsort(files, count, sizeof(FileEntry), 
          (int (*)(const void*, const void*))compare_files);

    opts.long_format ? print_long_format(files, count) : 
                       print_short_format(files, count, max_width);
}

int main(int argc, char *argv[]) {
    Options opts = {0};
    int opt;
    
    while ((opt = getopt(argc, argv, "la")) != -1) {
        opts.long_format |= opt == 'l';
        opts.show_all |= opt == 'a';
    }

    if (optind >= argc) {
        process_dir(".", opts);
    } else {
        for (int i = optind; i < argc; i++) {
            if (argc > optind + 1) printf("%s:\n", argv[i]);
            process_dir(argv[i], opts);
            if (i < argc - 1) putchar('\n');
        }
    }
    return 0;
}
