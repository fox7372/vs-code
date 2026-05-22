#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<signal.h>
void receive(int sig)
{
    char* n="alarm is working\n";
    write(1,n,strlen(n));
}
int main()
{
    alarm(5);
    sleep(3);
    signal(SIGALRM,receive);
    char*buf="alarm sin't working\n";
    write(1,buf,strlen(buf));
    pause();
}