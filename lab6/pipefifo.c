#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>

int main() {
    int p2c[2];
    int c2p[2];
    pid_t pid;
    char buffer[256];

    if (pipe(p2c) == -1 || pipe(c2p) == -1) {
        perror("pipe");
        exit(1);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        close(p2c[1]);
        close(c2p[0]);

        char ready = 1;
        if (write(c2p[1], &ready, 1) == -1) {
            perror("write ready");
            exit(1);
        }

        ssize_t bytes = read(p2c[0], buffer, sizeof(buffer) - 1);
        if (bytes <= 0) {
            perror("read");
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

        close(p2c[0]);
        close(c2p[1]);
        exit(0);
    } else {
        close(p2c[0]);
        close(c2p[1]);

        char ready;
        if (read(c2p[0], &ready, 1) <= 0) {
            perror("read ready");
            exit(1);
        }

        time_t t = time(NULL);
        struct tm tm;
        char tstr[32];
        localtime_r(&t, &tm);
        asctime_r(&tm, tstr);
        tstr[strcspn(tstr, "\n")] = 0;

        sleep(5);

        char message[256];
        snprintf(message, sizeof(message),
                 "Сообщение от родителя PID=%d\nВремя родителя: %s\n",
                 getpid(), tstr);

        if (write(p2c[1], message, strlen(message)) == -1) {
            perror("write msg");
            exit(1);
        }

        close(p2c[1]);
        close(c2p[0]);
        wait(NULL);
    }

    return 0;
}