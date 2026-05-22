/**
 * sigprocmask 防止父子进程竞态示例（使用 signal 注册）
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

volatile sig_atomic_t shared_data = 0;
volatile sig_atomic_t sig_flag = 0;

static void handler(int sig)
{
    sig_flag = 1;
    shared_data = 42;
}

int main(void)
{
    setbuf(stdout, NULL);

    /* --- 1. 使用 signal 注册 --- */
    if (signal(SIGUSR1, handler) == SIG_ERR) {
        perror("signal");
        exit(EXIT_FAILURE);
    }

    /* --- 2. fork 前阻塞 SIGUSR1 --- */
    sigset_t oldmask, newmask;
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) {
        perror("sigprocmask (block)");
        exit(EXIT_FAILURE);
    }

    printf("[parent] SIGUSR1 已阻塞，准备 fork\n");

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        printf("[child] 子进程启动，向父进程发送 SIGUSR1\n");
        kill(getppid(), SIGUSR1);
        printf("[child] 已发送，退出\n");
        exit(EXIT_SUCCESS);
    }

    /* ========== 父进程：初始化阶段 ========== */
    printf("[parent] 正在初始化...\n");
    sleep(1);
    printf("[parent] 初始化完成\n");

    /* --- 3. 解除 SIGUSR1 阻塞 --- */
    printf("[parent] 解除 SIGUSR1 阻塞\n");
    if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) {
        perror("sigprocmask (unblock)");
        exit(EXIT_FAILURE);
    }

    if (sig_flag)
        printf("[parent] handler 已执行，shared_data = %d\n",
               (int)shared_data);
    else
        printf("[parent] 信号未收到\n");

    printf("[parent] 等待子进程退出...\n");
    wait(NULL);
    printf("[parent] 结束\n");
    return 0;
}
