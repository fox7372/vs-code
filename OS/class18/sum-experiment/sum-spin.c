#include <thread.h>
#include <thread-sync.h>

extern long sum, N;

spinlock_t lk = SPIN_INIT();

void T_sum() {
    for (int i = 0; i < N; i++) {
        spin_lock(&lk);
        sum++;
        spin_unlock(&lk);
    }
}
