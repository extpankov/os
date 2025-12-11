#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include "common.h"

static int guard_fd = -1;

void terminate(int sig) {
    if (guard_fd != -1) {
        close(guard_fd);
        unlink(LOCK_PATH);
    }
    shm_unlink(MEM_REGION);
    exit(0);
}

int main() {
    guard_fd = open(LOCK_PATH, O_CREAT | O_EXCL | O_RDWR, 0644);
    if (guard_fd == -1) {
        if (errno == EEXIST)
            fprintf(stderr, "Instance already exists\n");
        else
            perror("Lock creation failed");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, terminate);
    signal(SIGTERM, terminate);

    int mem_fd = shm_open(MEM_REGION, O_CREAT | O_RDWR, 0666);
    if (mem_fd == -1) {
        perror("Shared memory creation");
        terminate(0);
        exit(EXIT_FAILURE);
    }

    if (ftruncate(mem_fd, sizeof(SegmentData)) == -1) {
        perror("Memory sizing");
        close(mem_fd);
        terminate(0);
        exit(EXIT_FAILURE);
    }

    SegmentData *mapped = mmap(NULL, sizeof(SegmentData), PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0);
    if (mapped == MAP_FAILED) {
        perror("Mapping failed");
        close(mem_fd);
        terminate(0);
        exit(EXIT_FAILURE);
    }

    mapped->owner = getpid();
    printf("Producer PID %d initialized\n", mapped->owner);

    while (1) {
        struct timespec now;
        clock_gettime(CLOCK_REALTIME, &now);
        struct tm local_tm;
        localtime_r(&now.tv_sec, &local_tm);
        char formatted[64];
        strftime(formatted, sizeof(formatted), "%Y-%m-%d %H:%M:%S", &local_tm);
        snprintf(mapped->content, MAX_BUF, "PID:%d TIME:%s", mapped->owner, formatted);
        sleep(2);
    }
}
