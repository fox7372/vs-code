#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void handler(int sig) {
    printf("捕获信号 %d\n", sig);
}

int main() {
    sigset_t set, old;

    signal(SIGINT, handler);

    printf("按 Ctrl+C 试试 — 将被阻塞 10 秒\n");

    // 阻塞 SIGINT
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, &old);

    sleep(10);  // 这期间按 Ctrl+C 不会生效，信号处于 pending

    printf("解除阻塞，积压的信号会立即送达\n");
    sigprocmask(SIG_SETMASK, &old, NULL);  // 恢复

    pause();  // 再按一次 Ctrl+C 就退出了
    return 0;
}
