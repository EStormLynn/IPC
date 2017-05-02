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