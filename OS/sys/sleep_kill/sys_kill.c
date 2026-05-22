#include<sys/types.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<signal.h>

int main(int argc,char*argv[])
{
    pid_t pid=atoi(argv[1]);

    kill(pid,SIGKILL);
    printf("%d process is killed\n",pid);
}
    