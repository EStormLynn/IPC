#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

char *cmd1[3]={"/bin/cat","/etc/passwd",NULL};
char *cmd2[3]={"/bin/grep","root",NULL};

int main(void)
{
    int fd[2];
    if(pipe(fd)<0)
    {
        perror("pipe error");
        exit(1);
    }
    int i=1;
    pid_t pid;
    for(;i<=2;i++)
    {
        pid=fork();
        if(pid<0)
        {
            perror("fork error");
            exit(1);
        }
        else if(pid==0)
        {
            //child process
            if(i==1)
            {
                //子进程1，负责写入数据
                close(fd[0]);//关闭读端
                
                //将标准输出重定向到管段的写端
                if(dup2(fd[1],STDOUT_FILENO)
                            !=STDOUT_FILENO)
                {
                    perror("dup2 error");
                }
                close(fd[1]);

                //调用exec函数执行cat命令
                if(execvp(cmd1[0],cmd1<0))
                {
                    perror("excvp error");
                    exit(1);
                }
                break;
            }
            if(i==2)
            {
                //子进程2，负责从管道读取数据
                close(fd[1]);//关闭写端
                
                //将标准输入重定向到管道的读端
                if(dup2(fd[0],STDIN_FILENO)
                                !=STDIN_FILENO)
                {
                    perror("dup2 error");
                }
                close(fd[0]);

                //调用exec函数执行grep命令
                if(execvp(cmd2[0],cmd2)<0)
                {
                    perror("execvp error");
                    exit(1);
                }
                break;

            }
        }else
        {
            //parent process
            if(i==2)
            {
                //父进程要等到子进程全部创建完毕才去回收
                close(fd[0]);
                close(fd[1]);
                wait(0);
                wait(0);
            }
        }
    }


    
}
