#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int agrc,char*agrv[])
{
    char*buf=agrv[1];
    int pipefd[2];                           // 管道文件描述符
    if (pipe(pipefd) == -1) {                // 创建管道
        perror("pipe");
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0)                            // 子进程
    {
        close(pipefd[1]);                    // 关闭写端
        dup2(pipefd[0], STDIN_FILENO);       // 将标准输入重定向到管道读端
        close(pipefd[0]);                    // 关闭原读端

        printf("child process is working\n");
        char *argv[] = {"fast_sort", NULL};
        execv(buf, argv);          // 执行 fast_sort
        perror("execv");                     // 只有 execv 失败才会执行到这里
        exit(1);
    }
    else                                     // 父进程
    {
        close(pipefd[0]);                    // 关闭读端

        /* 向管道写入数据，fast_sort 通过 scanf 从标准输入读取 */
        dprintf(pipefd[1], "6\n32 34 5 6 8 12\n");
        close(pipefd[1]);                    // 写完关闭写端，子进程 scanf 读到 EOF

        wait(NULL);                          // 等待子进程结束
        printf("child process is recieved by father process %d\n", getpid());
    }
}
