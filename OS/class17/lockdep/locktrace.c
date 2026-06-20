#define _GNU_SOURCE
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static int (*real_lock)(pthread_mutex_t *);
static int (*real_unlock)(pthread_mutex_t *);
static volatile int resolving;

static void resolve_real(void) {
    if (real_lock) return;
    if (__atomic_exchange_n(&resolving, 1, __ATOMIC_ACQ_REL)) return;
    real_lock   = dlsym(RTLD_NEXT, "pthread_mutex_lock");
    real_unlock = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
    __atomic_store_n(&resolving, 0, __ATOMIC_RELEASE);
}

static void trace(const char *msg, pthread_mutex_t *m) {
    char buf[256];
    int n = snprintf(buf, sizeof(buf), "[trace] thread %lu: %s %p\n",
                     (unsigned long)pthread_self(), msg, (void *)m);
    write(STDERR_FILENO, buf, n);
}

int pthread_mutex_lock(pthread_mutex_t *m) {
    resolve_real();
    if (!real_lock)
        return 0;

    trace("acquire", m);
    int ret = real_lock(m);
    trace("acquired", m);

    return ret;
}

int pthread_mutex_unlock(pthread_mutex_t *m) {
    resolve_real();
    if (!real_unlock)
        return 0;

    trace("release", m);
    return real_unlock(m);
}
