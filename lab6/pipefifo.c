#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>

int main() {
    int pipefd[2];
    pid_t pid;
    char buffer[256];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {  
        close(pipefd[1]);

        ssize_t bytes;
        // printf("Дочерний процесс PID=%d\n", getpid());

        while ((bytes = read(pipefd[0], buffer, sizeof(buffer)-1)) > 0) {
            buffer[bytes] = 0;
            printf("%s", buffer);
        }

        // текущее время ребёнка
        time_t t = time(NULL);
        char tstr[32];
        struct tm tm;
        localtime_r(&t, &tm);
        asctime_r(&tm, tstr);
        tstr[strcspn(tstr, "\n")] = 0;

        printf("Время дочернего процесса: %s\n", tstr);

        close(pipefd[0]);
        exit(0);
    } else {
        // PARENT
        close(pipefd[0]);

        sleep(5);

        time_t t = time(NULL);
        struct tm tm;
        char tstr[32];

        localtime_r(&t, &tm);
        asctime_r(&tm, tstr);
        tstr[strcspn(tstr, "\n")] = 0;

        char message[256];
        snprintf(message, sizeof(message),
                 "Сообщение от родителя PID=%d\nВремя родителя: %s\n",
                 getpid(), tstr);

        if (write(pipefd[1], message, strlen(message)) == -1)
            perror("write");

        close(pipefd[1]);
        wait(NULL);
    }

    return 0;
}