#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <stdatomic.h>

extern long sum, N;

/* 0=unlocked, 1=locked */
static _Atomic int klock = 0;

static inline int futex_wait_int(int *uaddr, int expected) {
    return (int)syscall(SYS_futex, uaddr, FUTEX_WAIT, expected, NULL, NULL, 0);
}
static inline int futex_wake_int(int *uaddr, int n) {
    return (int)syscall(SYS_futex, uaddr, FUTEX_WAKE, n, NULL, NULL, 0);
}

/* 强制每次 acquire 都进内核：成功拿到锁后再做一次“必然 EAGAIN 的 futex_wait” */
static inline void lock_force_kernel(void) {
    for (;;) {
        int expected = 0;
        if (atomic_compare_exchange_weak_explicit(
                &klock, &expected, 1,
                memory_order_acquire, memory_order_relaxed)) {

            /* 强制进内核：此时 klock==1，expected=2 不匹配 => 立即 -EAGAIN 返回，但已 trap */
            (void)futex_wait_int((int*)&klock, 2);
            return;
        }

        /* 竞争时：在内核里睡眠等待（同样会 trap） */
        (void)futex_wait_int((int*)&klock, 1);
    }
}

/* 强制每次 release 都进内核：无条件 futex_wake（即使没人等，也会 trap） */
static inline void unlock_force_kernel(void) {
    atomic_store_explicit(&klock, 0, memory_order_release);
    (void)futex_wake_int((int*)&klock, 1);
}

void T_sum(void) {
    for (long i = 0; i < N; i++) {
        lock_force_kernel();
        sum++;
        unlock_force_kernel();
    }
}
