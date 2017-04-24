# 进程间通信概述
* 数据传输：一个进程需要将它的数据发送给另一个进程，发送的数据量在一个字节到几兆字节之间
* 共享数据：多个进程想要操作共享数据，一个进程对共享数据的修改，别的进程应该立刻看到。
* 通知时间：一个进程需要向另一个或一组进程发送消息，通知他们发生了某些事件（如进程终止时要通知父进程）
* 资源共享：多个进程之间共享同样的资源，为了做到这一点，需要内核提供锁和同步机制
* 进程控制：有些进程希望完全控制另一个进程的执行(如Debug进程)，此时控制进程希望能够拦截另一个进程的所有陷入的异常，并能够及时指导它的状态改变。

[TOC]
# 进程间通信方式
* 管道(pipe)，有名管道(FIFO)
* 信号(signal)
* 消息队列
* 共享内存
* 信号量
* 套接字(socket)

## 管道
* 管道针对本地计算机的两个进程之间的通信而设计的通信方式，管道建立后，实际获得两个文件描述符：一个用于读取另一个用于写入。
* 常见的IPC机制，通过pipe系统调用。
* 管道单工，数据只能向一个方向流动。双向通信时，需要建立两个管道。
* 数据的读出和写入：一个进程向管道中写的内容被管道另一端的进程读出。写入的内容每次都添加在管道的缓冲区的尾部，每次都是从缓冲区的头部读出数据。
### 管道的分类
* 匿名管道
    * 关系进程，父子或兄弟
    * 由pipe系统调用，管道由父进程建立
    * 管道位于内核空间，其实是一块缓存
* 有名管道（FIFO）
    * 两个没有任何关系的进程之间通信可通过有名管道进行数据传输
    * 通过系统调用mkfifo创建
### 管道创建

```
#include<unistd.h>
int pipe(int fd[2]);
//返回：0成功，-1出错
```
* 两个文件描述符数组
    * fd[0]:pipe的读端
    * fd[1]:pipe的写端


## 共享内存
* 共享内存区域是被多个进程共享的一部分物理内存。
* 多个进程都可以把共享内存**映射**到自己的虚拟空间。所有用户空间的进程要操作共享内存，都要将其映射到自己的虚拟空间，通过映射的虚拟内存空间地址去操作共享内存，从而达到进程间的数据通信。
* 共享内存是进程间共享数据的一种最快的方法，一个进程向共享内存区域写入了数据，共享这个内存区域的所有进程就可以立刻看到其中的内容
* 本身不提供同步机制，可通过信号量进行同步
* 提升数据处理效率，一种**效率最高**的IPC机制
### 共享内存属性信息
```C++
struct shmid_ds{
    struct ipc_perm shm_perm;
    size_t shm_semsz;//共享内存大小   
    pid_t shm_lpid;//最后一次调用系统pid
    pid_t shm_cpid;//pid的创建者的id
    shmatt_t shm_nattch;//和共享内存成功映射的数量
    time_t shm_atime;//最后一个成功映射的时间
    time_t shm_dtime;//最后断开映射的时间
    time_t shm_ctime;//最后改变的时间
}
```
### 共享内存使用步骤
* 使用shmget()函数创建共享内存
* 使用shmat()函数映射共享内存，将这段创建的共享内存映射到具体的进程虚拟内存空间

### 创建共享内存
```C++
#include <sys/shm.h>
itn shmget(key_t key,size_t size,int shmflag);
//返回，成功返回内核中共享内存的表示iD，失败返回-1.
```
* 参数
    * key:用户指定的共享内存键值
    * size:共享内存大小
    * shmflg:IPC_CREAT,IPC_EXCL等权限组合
* erron
    * EINVAL(无效的内存段大小)
    * EEXIST(内存段已经存在，无法创建)
    * EIDRM(内存段已经被删除)
    * ENOENT(内存段不存在)
    * EACCES(权限不够)
    * ENOMEM(没有足够内存创建内存段)

### 共享内存控制
```C++
#include<sys/shm.h>
int shmctl(int shmid,int cmd,struct shmid_ds *buf);
```
* 参数
    * shmid:共享内存ID
    * buf:共享内存属性指针
    * cmd
        * IPC_STAT 获取共享内存段属性
        * IPC_SET 设置共享内存段属性
        * IPC_RMID 删除共享内存段
        * SHM_LOCK 锁定共享内存段页面
        * SHM_UNLOCK 解锁锁定

### 共享内存映射和解除
```
#include<sys/shm.h>
void shmat(int shmid,char *shmaddr,int shmflg);
int shmdt(char  *shmaddr);
//失败返回-1
```
* 参数
    * shmid:共享内存ID
    * shmaddr:映射到进程虚拟内存空间的地址，系统自动分配
    * shmflg:弱shmaddr为0，shmflag也是0
        * SHM_RND
        * SHMLBA 地址为2的次方
        * SHM_RDONLY 只读方式连接
* errno
    * EINVAL 无效的IPC ID值或无效的地址
    * ENOMEM 没有足够的内存
    * EACCES 权限不够
* 子进程不继承父进程创建的共享内存，大家是共享的，子进程继承父进程映射的地址。

## 消息队列
* 消息队列是内核中的一个链表
* 用户进程将数据传输到内核后，内核重新添加一些如用户ID、组ID、读写进程的ID和优先级等相关信息后并打包成一个数据包称为消息
* 允许一个或多个进程往消息队列中读写消息，但一个消息只能被一个进程读取，读取完毕后**自动删除**
* 消息队列具有一定的**FIFO**的特性，消息可以按照顺序发送到队列中，也可以几种不同的方式从队列中读取。每一个消息队列在内核中用一个唯一的IPC标识ID表示
* 消息队列的实现包括**创建和打开队列**、**发送消息**、**读取消息**和**控制消息队列**四种操作。

### 消息队列属性
```C
struct msqid_ds
{
    struct ipc_perm msg_perm;
    msgqnum_t msg_qnum; //消息数量
    msglen_t msg_qbytes; //消息最大字节数
    pid_t msg_lspid; //最后一次发送消息进程的pid
    pid_t msg_lrpid; //最后一次接收消息的pid
    pid_t msg_stime; //最后一次消息发送的时间
    pid_t msg_ctime; //最后一次消息改变的时间
}；
```

### 打开或创建消息队列
```C
#include <sys/msg.h>
int msgget(key_t key,int flag);
//返回：如果成功，返回内核中消息队列的标识ID，出错返回-1
```
* 参数
    * key:用户指定的消息队列键值
    * flag:IPC CREAT,IPC EXCL等权限组合
* 若创建消息队列，key可以指定键值，也可以设置为IPC_PRIVATE(0)。若打开进行查询，则key不能为0，必须是一个非零的值，否则查询不到

### 消息队列控制
```C
#include <sys/msg.h>
int msgctl(int msgid,int cmd,struct msqid_ds *buf);
//返回：成功返回0,出错返回-1
```
* 参数
    * msgid：消息队列ID
    * buf：消息队列属性指针
    * cmd
        * IPC_STAT：获取消息队列的属性，取此队列的msqid_ds结构，并放在buf指向的结构中
        * IPC_SET：设置属性，按由buf只想的结构中的值，设置与此队列相关的结构中的字段
        * IPC_RMID：删除队列，从系统中删除该队列以及在队列上的所有数据。

### 发送消息
```C
#include <sys/msg.h>
int msgsnd(int msgqid,const void *ptr,size_t nbytes,int flag);
//成功返回0,出错返回-1

ptr:
struct mymesg
{
    long mtype;//消息类型
    char mtext[512];//消息数据本身
};
```
* nbytes 指定消息的大小，不包括mtype的大小
* mtype指消息的类型，由一个整数来表示，且大于0
* mtext消息数据本身
* 在Linux中，消息的最大长度是4056个字节，其中包括mtype，占4个字节
* 结构体mymesg用户可自定义，但第一个成员必须是mtype
* 参数flag
    * 0：阻塞
    * IPC_NOWAIT:类似文件I/O的非阻塞
    * 若消息队列满（或者是队列中的消息总数等于系统限制值，或队列中的字节数等于系统限制值），则指定IPC_NOWAIT使得msgsnd立即出错返回EAGAIN。如果指定0，则
        * 阻塞直到有空间可以容纳要发送的消息
        * 或从系统中删除了此队列
        * 或捕捉到一个信号，并从信号处理程序返回

### 接收消息
```C
#include <sys/msg.h>
ssize_t msgrcv(int msgqid,void *ptr,size_t nbytes,long type,int flag);
//成功返回消息数据部分的长度，出错返回-1
```

* 参数
    * magqid：消息队列的ID
    * ptr：指向存放消息的缓存
    * nbytes：消息缓存的大小，不包括mtype的大小。计算方式
        * nbytes=sizeof(struct mymesg)-sizeof(long)
    * type：消息类型
        * type==0：获得消息队列中的第一个消息
        * type>0：获得消息队列中类型type的第一个消息
        * type<0：获得消息队列中小于或等于type绝对值的消息
    * flag：0或者IPC_NOWAIT