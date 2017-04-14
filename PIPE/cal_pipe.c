#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
父进程通过管道传输两个数据给子进程
由子进程负责从管道中读取并输入
*/

int main(void)
{
    int fd[2];
    //创建管道
    if(pipe(fd)<0)
    {
        perror("pipe error");
        exit(1);
    }
    pid_t pid;
    if((pid=fork())<0)
    {
        perror("fork error");
        exit(1);
    }else if(pid>0)
    {
        //parent process
        close(fd[0]);//关闭读端，父进程写入数据
        int start=1,end=100;
        //往管道写入数据
        if(write(fd[1],&start,sizeof(int))!=sizeof(int))
        {
            perror("write error");
            exit(1);                
        }
        if(write(fd[1],&end,sizeof(int))
            !=sizeof(int))
        {
            perror("write error");
            exit(1);               
        }
        close(fd[1]);
        wait(0);
        
    }else
    {
        // child process
        close(fd[1]);//关闭写端，子进程读取数据
        int start,end;
        //往管道写入数据
        if(read(fd[0],&start,sizeof(int))
            !=sizeof(int))
        {
            perror("read error");
            exit(1);                
        }
        if(read(fd[0],&end,sizeof(int))
            !=sizeof(int))
        {
            perror("read error");
            exit(1);               
        }
        close(fd[0]);
        printf("child process read start:%d,end:%d\n",start,end);
    }
    exit(0);
}
