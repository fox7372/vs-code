#define _GNU_SOURCE
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define MAX_LOCKS 64
#define MAX_HELD  16

/* ---- real pthread functions ---- */
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

/* ---- lockdep global state (spinlock to avoid recursion) ---- */
static volatile int g_spin;

static void spin_lock(void) {
    while (__atomic_exchange_n(&g_spin, 1, __ATOMIC_ACQ_REL))
        ;
}
static void spin_unlock(void) {
    __atomic_store_n(&g_spin, 0, __ATOMIC_RELEASE);
}

static struct {
    pthread_mutex_t *addr;
    char name[32];
} locks[MAX_LOCKS];

static int nlocks;

/* adjacency matrix: edge[a][b] = "a held, then b acquired" */
static int edge[MAX_LOCKS][MAX_LOCKS];

/* per-thread: stack of currently held locks */
static __thread int held[MAX_HELD];
static __thread int nheld;

/* recursion guard */
static __thread int in_lockdep;

/* ---- internal helpers ---- */

static int register_mutex(pthread_mutex_t *m) {
    for (int i = 0; i < nlocks; i++)
        if (locks[i].addr == m) return i;
    if (nlocks >= MAX_LOCKS) return -1;
    int id = nlocks++;
    locks[id].addr = m;
    snprintf(locks[id].name, sizeof(locks[id].name), "mutex@%p", (void *)m);
    return id;
}

static int has_path(int cur, int target, int visited[]) {
    if (cur == target) return 1;
    visited[cur] = 1;
    for (int i = 0; i < nlocks; i++)
        if (edge[cur][i] && !visited[i])
            if (has_path(i, target, visited)) return 1;
    return 0;
}

/* ---- LD_PRELOAD replacement ---- */

int pthread_mutex_lock(pthread_mutex_t *m) {
    resolve_real();
    if (!real_lock || in_lockdep)
        return real_lock ? real_lock(m) : 0;
    in_lockdep = 1;

    spin_lock();
    int id = register_mutex(m);
    if (id >= 0) {
        for (int i = 0; i < nheld; i++) {
            int prev = held[i];
            int visited[MAX_LOCKS] = {};
            if (has_path(id, prev, visited)) {
                fprintf(stderr,
                    "\n*** LOCKDEP: potential deadlock! ***\n"
                    "  %s -> %s would create cycle\n"
                    "  currently held: ",
                    locks[prev].name, locks[id].name);
                for (int j = 0; j < nheld; j++)
                    fprintf(stderr, "%s ", locks[held[j]].name);
                fprintf(stderr, "\n\n");
            }
            edge[prev][id] = 1;
        }
        if (nheld < MAX_HELD) held[nheld++] = id;
    }
    spin_unlock();

    in_lockdep = 0;
    return real_lock(m);
}

int pthread_mutex_unlock(pthread_mutex_t *m) {
    resolve_real();
    if (!real_unlock || in_lockdep)
        return real_unlock ? real_unlock(m) : 0;
    in_lockdep = 1;

    int result = real_unlock(m);

    spin_lock();
    for (int i = nheld - 1; i >= 0; i--) {
        if (locks[held[i]].addr == m) {
            memmove(&held[i], &held[i + 1],
                    (nheld - i - 1) * sizeof(int));
            nheld--;
            break;
        }
    }
    spin_unlock();

    in_lockdep = 0;
    return result;
}
