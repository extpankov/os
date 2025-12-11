#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>

int main(void) {
    int channel[2];

    if (pipe(channel) != 0) {
        fprintf(stderr, "Не удалось создать канал\n");
        return 1;
    }

    pid_t child = fork();
    if (child < 0) {
        perror("fork failed");
        return 1;
    }

    if (child == 0) {
        close(channel[1]);

        char msg_buf[256];
        ssize_t nread = read(channel[0], msg_buf, sizeof(msg_buf) - 1);
        if (nread > 0) {
            msg_buf[nread] = '\0';

            printf("Потомок (PID=%d) получил:\n%s", (int)getpid(), msg_buf);

            time_t now = time(NULL);
            struct tm local_tm;
            localtime_r(&now, &local_tm);
            char time_str[64];
            asctime_r(&local_tm, time_str);
            time_str[strcspn(time_str, "\n")] = '\0';
            printf("Время в потомке: %s\n", time_str);
        }

        close(channel[0]);
        _exit(0);
    } else {
        close(channel[0]);

        time_t current_time = time(NULL);
        struct tm parent_tm;
        localtime_r(&current_time, &parent_tm);
        char parent_time[64];
        asctime_r(&parent_tm, parent_time);
        parent_time[strcspn(parent_time, "\n")] = '\0';

        char payload[300];
        int len = snprintf(payload, sizeof(payload),
                          "Привет от родителя!\n"
                          "PID родителя: %d\n"
                          "Время отправки: %s\n",
                          (int)getpid(), parent_time);

        if (write(channel[1], payload, len) != len) {
            perror("Ошибка записи в канал");
        }

        close(channel[1]);
        wait(NULL);
    }

    return 0;
}
