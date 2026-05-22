#include"thread.h"
#include"thread-sync.h"

mutex_t lk=MUTEX_INIT();
cond_t cv=COND_INIT();

enum{A=1,B,C,D,E,F,};
struct rule{
	int from,ch,to;
}	rules[]={
	{A,'<',B},
	{B,'>',C},
	{C,'<',D},
	{A,'>',E},
	{E,'<',F},
	{F,'>',D},
	{D,'_',A},
};

int current=A;

void fish_thread(int id)
{
	while(1)
	{
		mutex_lock(&lk);
        while(rules[id%7].from!=current)cond_wait(&cv,&lk);
	    putchar(rules[id%7].ch);
	    current=rules[id%7].to;
	    cond_broadcast(&cv);
	    mutex_unlock(&lk);
	}
}

int main()
{
	setbuf(stdout, NULL);             // 关闭缓冲区，即时输出
    for (int i = 0; i <7; i++)
        spawn(fish_thread);           // 为每个角色创建一个线程
}
