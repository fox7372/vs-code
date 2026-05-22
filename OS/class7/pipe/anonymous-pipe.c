// Author: GPT-4-turbo
//
// 功能演示：匿名管道（Anonymous Pipe）用于父子进程间通信
// 父进程向管道写入 "Hello, world!"，子进程从管道读取并打印

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

// 父进程逻辑：向管道写入数据
void do_parent(int fd) {
    const char *msg = "Hello, world!";

    printf("[%d] Write: '%s'\n", getpid(), msg);
    write(fd, msg, strlen(msg) + 1);  // +1 包含字符串结尾的 '\0'

    close(fd);  // 写完后关闭写端，子进程的 read 才能返回 0 或正常结束

    // 等待子进程退出，避免僵尸进程
    wait(NULL);

    printf("[%d] Done.\n", getpid());
}

// 子进程逻辑：从管道读取数据
void do_child(int fd) {
    static char buf[1024];

    ssize_t num_read = read(fd, buf, sizeof(buf));
    if (num_read == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    printf("[%d] Got: '%s'\n", getpid(), buf);

    // 关闭读端
    close(fd);
}

int main() {
    int pipefd[2];  // pipefd[0] = 读端, pipefd[1] = 写端

    // 创建匿名管道
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // fork 创建子进程（管道文件描述符会复制到子进程）
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // 子进程：关闭不需要的写端，只保留读端
        close(pipefd[1]); // Close unused write end
        do_child(pipefd[0]);
    } else {
        // 父进程：关闭不需要的读端，只保留写端
        close(pipefd[0]); // Close unused read end
        do_parent(pipefd[1]);
    }

    return 0;
}
