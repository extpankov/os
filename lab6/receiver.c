#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#define FIFO_P2C "/tmp/fifo_p2c"
#define FIFO_C2P "/tmp/fifo_c2p"

int main() {
    mkfifo(FIFO_P2C, 0666);
    mkfifo(FIFO_C2P, 0666);

    int fd_p2c = open(FIFO_P2C, O_RDONLY);
    if (fd_p2c == -1) {
        perror("open FIFO_P2C");
        exit(1);
    }

    int fd_c2p = open(FIFO_C2P, O_WRONLY);
    if (fd_c2p == -1) {
        perror("open FIFO_C2P");
        exit(1);
    }

    char ready = 1;
    if (write(fd_c2p, &ready, 1) == -1) {
        perror("write ready");
        exit(1);
    }

    char buffer[256];
    ssize_t bytes = read(fd_p2c, buffer, sizeof(buffer) - 1);
    if (bytes <= 0) {
        perror("read message");
        exit(1);
    }
    buffer[bytes] = '\0';

    printf("Дочерний процесс PID=%d\n", getpid());
    printf("%s", buffer);

    time_t t = time(NULL);
    struct tm tm;
    char tstr[32];
    localtime_r(&t, &tm);
    asctime_r(&tm, tstr);
    tstr[strcspn(tstr, "\n")] = 0;

    printf("Время дочернего процесса: %s\n", tstr);

    close(fd_p2c);
    close(fd_c2p);

    unlink(FIFO_P2C);
    unlink(FIFO_C2P);

    return 0;
}
