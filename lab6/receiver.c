#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

#define FIFO_NAME "/tmp/fifo_p2c"

int main() {
    int fd = open(FIFO_NAME, O_RDONLY);
    if (fd == -1) {
        perror("open FIFO for reading");
        exit(1);
    }

    char buffer[256];
    ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes <= 0) {
        perror("read from FIFO");
        close(fd);
        exit(1);
    }
    buffer[bytes] = '\0';

    printf("Дочерний процесс PID=%d\n", getpid());
    printf("%s", buffer);

    time_t t = time(NULL);
    struct tm tm;
    localtime_r(&t, &tm);
    char tstr[32];
    asctime_r(&tm, tstr);
    tstr[strcspn(tstr, "\n")] = '\0';
    printf("Время дочернего процесса: %s\n", tstr);

    close(fd);

    unlink(FIFO_NAME);

    return 0;
}
