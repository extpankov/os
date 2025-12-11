#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <string.h>
#include "common.h"

int main() {
    int mem_fd = shm_open(MEM_REGION, O_RDONLY, 0666);
    if (mem_fd == -1) {
        perror("shm_open failed");
        exit(EXIT_FAILURE);
    }

    SegmentData *mapped = mmap(NULL, sizeof(SegmentData), PROT_READ, MAP_SHARED, mem_fd, 0);
    if (mapped == MAP_FAILED) {
        perror("mmap error");
        close(mem_fd);
        exit(EXIT_FAILURE);
    }

    pid_t current = getpid();
    char previous[MAX_BUF] = {0};
    printf("Consumer PID %d active\n", current);

    while (1) {
        if (mapped->content[0] != '\0' && strcmp(mapped->content, previous) != 0) {
            strncpy(previous, mapped->content, MAX_BUF - 1);
            previous[MAX_BUF - 1] = '\0';

            struct timespec now;
            clock_gettime(CLOCK_REALTIME, &now);
            struct tm local_tm;
            localtime_r(&now.tv_sec, &local_tm);
            char timestamp[64];
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &local_tm);

            printf("[Consumer %d | %s] Data: %s\n", current, timestamp, previous);
        }
        sleep(1);
    }

    munmap(mapped, sizeof(SegmentData));
    close(mem_fd);
    return 0;
}
