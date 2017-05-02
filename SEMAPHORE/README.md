## 信号量
* 本质上是共享资源的数目，用来控制对共享资源的访问。
* 用于进程间的互斥和同步
* 每种恭喜那个资源对应一个信号量，为了便于大量共享资源的操作引入了**信号量集**，可对多有信号量一次性操作。对信号量集中所有的操作可以要求全部成功，也可以部分成功
* 二元信号量(信号灯)值为0和1
* 对信号量做PV操作

### 信号量集属性
```C
#include<sysm.h>

struct semid_ds
{
    struct ipc_perm sem_perm;
    unsigned short sem_nseme; //信号灯的数量
    time_t sem_otime; //最后一次操作的时间
    time_t sem_ctime; //最后一次改变的时间
};
```

### 创建信号量集
```C
#include <sysm.h>
int semget(key_t key,int nsems,int flag);
//返回：如果成功，返回信号量集ID，出错返回-1
```
* 参数
    * key:用户指定的信号量集键值
    * nsems:信号量集中信号量的个数
    * flag:IPC CREAT,IPC EXCL等权限组合
* 若创建消息队列，key可以指定键值，也可以设置为IPC_PRIVATE(0)。若打开进行查询，则key不能为0，必须是一个非零的值，否则查询不到

### 信号量集控制
```C
#include <sys/msg.h>
int semctl(int semid,int semnum,int cmd,...//信号量集数组);
//返回：成功返回0,出错返回-1
union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
}
```
* 参数
    * semid：信号量集ID
    * semnum：0表示对所有信号量操作，信号量标号从0开始。
    * val：防止获取或设置信号量集中某个信号量的值
    * buf：信号量属性指针
    * array：防止获取或设置信号量集中所有信号量的值
    * cmd
        * IPC_STAT：获取信号量的属性，取此队列的semid_ds结构，并放在buf指向的结构中
        * IPC_SET：设置属性，按由buf只想的结构中的值，设置与此信号量相关的结构中的字段
        * IPC_RMID：删除信号量，从系统中删除该信号量以及信号量上的所有数据。

### 信号量集操作
```C
#include<sysm.h>
int semop(int semid,struct sembuf *sops,size_t nsops);
//成功返回0，失败返回-1

struct sembuf
{
    unsigned short sem_num; //信号量集标号
    short sem_op; //操作
    short sem_flg; //
}
```
* 参数
    * semid：信号量集ID
    * sops：sembuf结构体数组指针
    * nsops：第二个参数中结构体数组的长度
    * sem_num：信号量集中信号量的编号
    * sep_op：正数为v操作，负数为p操作，0表示对共享资源是否已用完的测试
    * sem_flg：SEM_UNDO标志，表示进程结束时，相应的操作将被取消，如果设置了该标志，那么在进程没有释放共享资源就退出时，内核将代为释放。
* 用于信号量集中信号量的PV操作
* 可用于进程见的互斥和同步