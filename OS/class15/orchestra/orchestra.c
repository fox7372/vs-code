// orchestra.c — 自旋等待版（忙等待）
// 通过忙等待同步指挥和乐手，存在 CPU 空转浪费

#include <thread.h>
#include <thread-sync.h>

static mutex_t lk = MUTEX_INIT();   // 保护 conductor_beat 的互斥锁

extern int conductor_beat;           // 指挥的节拍计数（在 main.c 中定义）

// 乐手等待节拍：当前拍子落后于指挥时才继续
void wait_for_beat(int current_beat) {
retry:
    // Reads should be protected by a mutex.
    mutex_lock(&lk);
    int conductor_beat_ = conductor_beat;  // 取一份本地快照
    mutex_unlock(&lk);

    if (current_beat >= conductor_beat_) {
        // 还没到拍子，继续忙等待（浪费 CPU）
        // There is a pattern here: we "wait" until something
        // (a condition) happens. This is the idea behind the
        // condition variable.
        goto retry;
    }
}

// 指挥释放一个节拍：推进节拍计数
void release_beat() {
    mutex_lock(&lk);
    conductor_beat++;
    mutex_unlock(&lk);
}

// This is a bad hack; I'm lazy.
#include "main.c"
