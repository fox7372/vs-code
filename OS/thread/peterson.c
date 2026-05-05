#include "thread.h"
#include <stdatomic.h>


//在这个循环里，当前线程（比如 A）没有修改 turn 和 B 这两个变量。那我从内存读一次放到 CPU 寄存器里就行了，没必要每次都去内存查
//就会导致当pcB到while时会缓存数据，当pcA到1时结果B的while中的A还是1

/*#define N 10000000
volatile int a=0;
volatile int A=0;
volatile int B=0;
volatile char turn ='A';
void TA(int i)
{
    for(int j=0;j<N;j++)
    {
        A=1;
        turn='B';
        while(turn =='B'&&B==1){}
        a++;
        A=0;
    }
}
void TB(int i)
{
    for(int j=0;j<N;j++)
    {
        B=1;
        turn='A'; 
        while(turn=='A'&&A==1){}
        a++;
        B=0;
    }
}
*/
//骗人的。。。。。。在现在的cpu大多数多核的也就会导致很多指令的顺序有问题

#include <emmintrin.h> 

#define N 10000000

atomic_int a = 0;
atomic_int A = 0; 
atomic_int B = 0;
atomic_int turn = 0; //0->A

void* TA(void* arg) {
    for (int j = 0; j < N; j++) {
        // 【修改点 1】使用 seq_cst，最强屏障
        atomic_store_explicit(&A, 1, memory_order_seq_cst);
        atomic_store_explicit(&turn, 1, memory_order_seq_cst);
        
        while (1) {
            // 【修改点 2】读取也用 seq_cst
            int t = atomic_load_explicit(&turn, memory_order_seq_cst);
            int b_val = atomic_load_explicit(&B, memory_order_seq_cst);
            
            if (!(t == 1 && b_val == 1)) {
                break;
            }
            // _mm_pause(); // 可以保留，帮助减少总线压力
        }

        // 临界区
        // 既然用了 seq_cst 做锁，这里的 a++ 即使是非原子的也是安全的
        // 但为了绝对保险，我们还是用原子加法
        atomic_fetch_add_explicit(&a, 1, memory_order_seq_cst);

        atomic_store_explicit(&A, 0, memory_order_seq_cst);
    }
    return NULL;
}

void* TB(void* arg) {
    for (int j = 0; j < N; j++) {
        atomic_store_explicit(&B, 1, memory_order_seq_cst);
        atomic_store_explicit(&turn, 0, memory_order_seq_cst);
        while (1) {
            int t = atomic_load_explicit(&turn, memory_order_seq_cst);
            int a_val = atomic_load_explicit(&A, memory_order_seq_cst);
            
            if (!(t == 0 && a_val == 1)) {
                break;
            }
            // _mm_pause();
        }

        atomic_fetch_add_explicit(&a, 1, memory_order_seq_cst);

        atomic_store_explicit(&B, 0, memory_order_seq_cst);
    }
    return NULL;
}

int main()
{
    create(TA);
    create(TB);
    join();
    printf("%d\n",a);
}
