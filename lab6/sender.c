#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <errno.h>


#define FIFO_NAME "/tmp/fifo_p2c"

int main() {
    if (mkfifo(FIFO_NAME, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo");
        exit(1);
    }

    int fd = open(FIFO_NAME, O_WRONLY);
    if (fd == -1) {
        perror("open FIFO for writing");
        exit(1);
    }

    time_t t = time(NULL);
    struct tm tm;
    localtime_r(&t, &tm);
    char tstr[32];
    asctime_r(&tm, tstr);
    tstr[strcspn(tstr, "\n")] = '\0';

    char message[256];
    snprintf(message, sizeof(message),
             "Сообщение от родителя PID=%d\nВремя родителя: %s\n",
             (int)getpid(), tstr);

    if (write(fd, message, strlen(message)) == -1) {
        perror("write to FIFO");
        close(fd);
        exit(1);
    }

    close(fd);

    unlink(FIFO_NAME);

    printf("[Sender] Сообщение отправлено.\n");
    return 0;
}
