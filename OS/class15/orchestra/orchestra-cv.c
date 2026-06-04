// orchestra-cv.c — 条件变量版（高效等待）
// 用 cond_wait 替代忙等待，CPU 不会空转

#include <thread.h>
#include <thread-sync.h>

static mutex_t lk = MUTEX_INIT();   // 保护 conductor_beat 的互斥锁

extern int conductor_beat;           // 指挥的节拍计数（在 main.c 中定义）

// Condition variable for synchronizing beats
cond_t cv = COND_INIT();             // 条件变量：当节拍推进时唤醒等待的乐手

// 乐手等待节拍：若赶不上指挥就睡眠等待
void wait_for_beat(int current_beat) {
    mutex_lock(&lk);

    // To proceed only when current_beat falls behind.
    // And we don't need a "local copy" here.
    while (!(current_beat < conductor_beat)) {
        cond_wait(&cv, &lk);         // 释放锁并睡眠，被唤醒后重新获得锁
    }

    mutex_unlock(&lk);
}

// 指挥释放一个节拍：推进计数并唤醒所有等待的乐手
void release_beat() {
    mutex_lock(&lk);
    conductor_beat++;
    cond_broadcast(&cv);  // Wake up potential waiting threads.
    mutex_unlock(&lk);
}

// This is a bad hack; I'm lazy.
#include "main.c"
