#include <thread.h>
#include <thread-sync.h>
int n=1000000;
_Thread_local int x = 0;
int y=0;
mutex_t mtx = MUTEX_INIT();

void T_worker(int tid) {
  // thread_local int y;  <- Not compile.
    for(int i=0; i<n; i++)
    {   
        x++;
        if(x==100)
        {
            mutex_lock(&mtx);
            y+=x;
            mutex_unlock(&mtx);
            x=0;
        }
    }
}

int main() {
    for (int i = 0; i < 4; i++) {
        spawn(T_worker);
    }
    join();
    printf("Main thread: %p = %d\n", &y, y);
}
