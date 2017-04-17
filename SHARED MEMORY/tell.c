#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tell.h"

static int fd[2];

//管道初始化
void init()
{
    if(pipe(fd)<0)
    {
        perror("pipe error");    
    }

}

//利用管道进行等待
void wait_pipe()
{
    char c;
    //管道读写默认是阻塞的
    if(read(fd[0],&c,1)<0)
    {
        perror("wait pipe error");
    }
}

//利用管道进行通知
void notify_pipe()
{
    char c='c';
    if(write(fd[1],&c,1)!=1)
    {
        perror("notify pipe error");
    }
}

//销毁管道
void destory_pipe()
{
    close(fd[0]);
    close(fd[1]);
}
