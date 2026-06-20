#include <thread.h>
#include <thread-sync.h>

mutex_t lock = MUTEX_INIT();

void T_lock(void *arg) {
    mutex_lock(&lock);
    printf("Thread %ld acquired lock\n", (long)arg);

    // 同一个线程再次获取同一把锁 → 死锁
    mutex_lock(&lock);
    printf("Thread %ld acquired lock again (never reached)\n", (long)arg);

    mutex_unlock(&lock);
    mutex_unlock(&lock);
}

int main() {
    spawn(T_lock);
}
