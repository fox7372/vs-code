// 作者: GPT-4-turbo

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

// UNIX 域套接字也用于本地进程间通信，它们在文件系统中也有一个名字，
// 比如 "/var/run/docker.sock"。这与命名管道类似。
#define PIPE_NAME "/tmp/my_pipe"

/** 
 * @brief 从命名管道中读取数据
 */
void pipe_read() {
    // 以只读方式打开命名管道
    int fd = open(PIPE_NAME, O_RDONLY);
    char buffer[1024];

    if (fd == -1) {
        perror("打开管道失败");
        exit(1);
    }

    // 从管道中读取数据
    int num_read = read(fd, buffer, sizeof(buffer));
    if (num_read > 0) {
        printf("收到: %s\n", buffer);
    } else {
        printf("未收到数据。\n");
    }
    close(fd);
}

/**
 * @brief 向命名管道中写入数据
 * @param content 要写入的消息内容
 */
void pipe_write(const char *content) {
    // 以只写方式打开命名管道
    int fd = open(PIPE_NAME, O_WRONLY);

    if (fd == -1) {
        perror("打开管道失败");
        exit(1);
    }

    // 将消息写入管道（包含字符串结束符）
    write(fd, content, strlen(content) + 1);
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "用法: %s read|write [message]\n", argv[0]);
        return 1;
    }

    // 如果命名管道不存在则创建它
    if (mkfifo(PIPE_NAME, 0666) == -1) {
        if (errno != EEXIST) {
            perror("创建命名管道失败");
            return 1;
        }
    } else {
        printf("已创建 " PIPE_NAME "\n");
    }

    if (strcmp(argv[1], "read") == 0) {
        pipe_read();
    } else if (strcmp(argv[1], "write") == 0) {
        pipe_write(argv[2]);
    } else {
        fprintf(stderr, "无效命令，请使用 'read' 或 'write'。\n");
        return 1;
    }

    return 0;
}
