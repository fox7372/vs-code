#include <thread.h>
#include <thread-sync.h>

#define N 10000000
int sum;

spinlock_t lk = SPIN_INIT();

void T_sum(int tid) {
    for (int i = 0; i < N / 10; i++) {
        spin_lock(&lk);
        for (int i = 0; i < 10; i++) {
            sum++;
            asm volatile("" : : : "memory");
        }
        spin_unlock(&lk);
    }
}

int main() {
    spawn(T_sum);
    spawn(T_sum);
    join();

    printf("sum = %ld\n", sum);
    printf("2*n = %ld\n", 2 * N);
}
