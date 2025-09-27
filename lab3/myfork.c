#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void bye() {
    printf(">> atexit: процесс %d завершает работу\n", getpid());
}

void h1(int s) {
    printf("!! signal: процесс %d получил сигнал %d (SIGINT)\n", getpid(), s);
}

void h2(int sig, siginfo_t *inf, void *c) {
    printf("!! sigaction: процесс %d получил сигнал %d (SIGTERM)\n", getpid(), sig);
}

int main(){
    atexit(bye);
    signal(SIGINT, h1);
    struct sigaction sa;  
    sa.sa_sigaction = h2;  
    sa.sa_flags = SA_SIGINFO;  
    sigemptyset(&sa.sa_mask);  
    sigaction(SIGTERM,&sa,NULL);

    pid_t p = fork();

    if(p<0){ perror("fork error"); exit(1); }

    if(p==0){
        printf("child: PID=%d  PPID=%d\n", getpid(), getppid());
        sleep(2);
        printf("child: завершаюсь...\n");
        exit(42);
    } else {
        printf("parent: PID=%d  PPID=%d, child=%d\n", getpid(), getppid(), p);
        int st;
        waitpid(p,&st,0);
        if(WIFEXITED(st)) {
            printf("parent: ребенок вернул код %d\n", WEXITSTATUS(st));
        }
    }

    return 0; 
}