#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

int child_count = 0;

void receive(int sig) {
    int olderrno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0) {
        child_count++;
    }

    errno = olderrno;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "用法: %s <数量>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);

    signal(SIGCHLD, receive);

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            printf("子进程 %d (PID=%d)\n", i, getpid());
            sleep(1);
            exit(0);
        }
    }

    while (child_count < n)
        sleep(1);

    printf("全部已回收 (共 %d 个子进程)\n", child_count);
    return 0;
}
