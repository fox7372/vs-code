/**
 * 对比版本：不使用 sigprocmask，展示父子进程间的数据竞争
 *
 * 与正确版本唯一区别：fork 前没有阻塞 SIGUSR1。
 * 子进程 fork 后立即 kill() 发送信号给父进程。
 * 由于父进程此时还在 sleep（模拟初始化），信号处理函数
 * 可能在 shared_data = 42 之前执行，读到 0 或不确定的值。
 *
 * 多次运行可观察到 handler 输出 shared_data = 0 的错误情况。
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

volatile sig_atomic_t shared_data = 0;

static void handler(int sig)
{
    int saved = errno;
    char buf[64];
    int n = snprintf(buf, sizeof(buf),
                     "[handler] SIGUSR1 到达, shared_data = %d  <%s>\n",
                     (int)shared_data,
                     shared_data == 42 ? "正确" : "数据竞争！");
    write(STDOUT_FILENO, buf, n);
    errno = saved;
}

int main(void)
{
    setbuf(stdout, NULL);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGUSR1, &sa, NULL) < 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    /* 注意：这里没有 sigprocmask 阻塞信号 */

    printf("[parent] 准备 fork（无信号保护）\n");

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        printf("[child]  子进程启动，发送 SIGUSR1\n");
        kill(getppid(), SIGUSR1);
        printf("[child]  已发送，退出\n");
        exit(EXIT_SUCCESS);
    }

    /* 父进程"初始化" */
    printf("[parent] 正在初始化...\n");
    sleep(1);
    shared_data = 42;
    printf("[parent] 初始化完成 (shared_data = %d)\n", (int)shared_data);

    printf("[parent] 等待子进程...\n");
    wait(NULL);
    printf("[parent] 结束\n");
    return 0;
}
