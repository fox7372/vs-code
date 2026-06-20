#include "thread.h"

void T_worker(int T) {
    while (1) {
        sleep(10);
    }
}

int main(int argc, char *argv[]) {
    int N = atoi(argv[1]);
    for (int i = 0; i < N; i++) {
        spawn(T_worker);
    }
}
