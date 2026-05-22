#include<stdio.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<errno.h>
#include<signal.h>
#include<string.h>

volatile sig_atomic_t child_count = 0;

void receive(int sig) 
{
    int olderrno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0)
    {
        child_count++;
        char*s="child is recieved\n";
        write(1,s,strlen(s));
    }

    errno = olderrno;
}
int main()
{
    signal(SIGCHLD, receive);
    pid_t pid[2];
    pid[0]=fork();
    if(pid[0]==0)
    {
       sleep(100);
    }
    else
    {
       pid[1]=fork();
       if(pid[1]==0)
       {
           char*buf="sys_kill";
           int argc=2;
           char pid_str[16];
           snprintf(pid_str, sizeof(pid_str), "%d", pid[0]);
           char*argv[3]={"sys_kill",pid_str,NULL};
           execvp("./sys_kill",argv);
           _exit(0);
       }
       else
       {
           while (child_count < 2)
           {
               pause();//容易造成死锁
           }
           printf("all children are recieved\n");
       }
       
    }						    
}