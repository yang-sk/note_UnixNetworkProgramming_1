## 第4章 基本TCP套接字编程
### 4.2 socket函数
此函数用来创建套接字.
```c
#include<sys/socket.h>
int socket(int family,int type,int protocol);
成功时返回一个较小的非负整数值(类似于文件描述符),出错返回负值
```
- family:协议族(协议域)
- type: 套接字类型
- protocol:协议类型. 若为0,则表示协议族和类型组合的默认值.
#### 常见的family常值
| family   | 说明      |
| -------- | ------- |
| AF_INET  | IPv4    |
| AF_INET6 | IPv6    |
| AF_LOCAL | Unix域协议 |
| AF_ROUTE | 路由套接字   |
| AF_KEY   | 密钥套接字   |

#### 常见的type常值
| type           | 说明      |
| -------------- | ------- |
| SOCK_STREAM    | 字节流套接字  |
| SOCK_DGRAM     | 数据报套接字  |
| SOCK_SEQPACKET | 有序分组套接字 |
| SOCK_RAW       | 原始套接字   |

####　socket函数AF_INET或AF_INET6的protocol常值
| protocol     | 说明       |
| ------------ | -------- |
| IPPROTO_TCP  | TCP传输协议  |
| IPPROTO_UDP  | UDP传输协议  |
| IPPROTO_SCTP | SCTP传输协议 |

#### 常见地址族和类型的组合
| 类型\地址族         | AF_INET  | AF_INET6 | AF_LOCAL | AF_ROUTE | AF_KEY |
| -------------- | -------- | -------- | -------- | -------- | ------ |
| SOCK_STEAM     | TCP/SCTP | TCP/STCP | yes      | no       | no     |
| SOCK_DGRAM     | UDP      | UDP      | yes      | no       | no     |
| SOCK_SEQPACKET | SCTP     | SCTP     | yes      | no       | no     |
| SOCK_RAW       | IPv4     | IPv6     | no       | yes      | yes    |

表格中的yes项表示该组合合法,但是没有具体命名.no表示该搭配不合法.
#### AF_xxxx和PF_xxxx
AF表示地址族,PF表示协议族.前者用来表征**地址**类型,后者用于表征**套接字**类型.
这是在早期系统中定义的,后期系统更倾向于用AF取代PF.
在```<sys/socket.h>```中定义两者总是等价.尽管如此,对很多现存代码进行改动替换依然可能崩溃.

### connect函数
此函数用来建立与TCP服务器的连接.
```c
#include<sys/socket.h>
int connect(int sockfd,const struct sockaddr* servaddr,socklen_t addrlen);
成功则返回0,出错-1
```
- sockfd: 操作的套接字主体
- servaddr: 远程服务器的网络地址结构体,内含IP地址和端口号
- socklen_t: 上述结构体的字节大小(通常为```unsigned int```)

客户端在调用connect之前不必调用bind,内核会确定源IP地址并选择一个临时端口作为源端口.若调用则会绑定套接字到指定的网络地址.
TCP套接字调用connect会激发三次握手,握手完毕后该函数才会返回(即该函数为阻塞式).
#### 出错返回
1. 若客户端没有收到服务器对SYN分节的响应,则返回ETIMEDOUT错误,常见于服务器崩溃的情况.举例来说,套接字发送SYN分节,若无响应则等待6s再发送,若仍无响应则继续发送,直至75s后返回错误.
2. 若服务器对客户端的响应是RST,则表明服务器在制定的端口上并无进程在等待连接.这是一种硬错误,会立即返回ECONNREFUSED错误.
     RST是一种表示错误的分节,发生于:
       - 上述情况
       - TCP想取消一个现有连接
       - TCP收到一个根本不存在的连接上的分节

3. 若客户端发出的SYN在中间的某个路由器上引发"destination unreachable"的ICMP错误,则被认为是一种软错误.这是路由器判定目标地址不可达产生的.
     客户端会继续多次尝试发送请求,直至超时(例如75s)后返回EHOSTUNREACH或ENETUNREACH.
     以下也可能产生该错误:一是按照本地的转发表找不到远程地址;二是connect调用根本不等待就返回(暂不明).
> 有些早期系统会错误认为这是一个不可修复的错误.
> ENETUNREACH已过时,应和EHOSTUNREACH视为一类.
> 思考: 软错误是可修复的网络错误.

在原文的测试中:
1. 指定192.168.1.100(子网中一个根本不存在的主机): 返回ETIMEDOUT
2. 指定一个未运行相关进程的端口: 返回ECONNREFUSED
3. 指定一个根本不可到达的IP地址(192.3.4.5): 返回EHOSTUNREACH

思考: 把路由比作邮局,一个邮局掌管一个街区.如果信封到达了正确的邮局,但在邮局派送到住户的过程出错(住户不存在或住户无响应),则是"超时"错误;如果信封在邮局之间传送时发送错误(如不存在的省市或街道),则是"不可达"错误.

connect函数调用时,套接字会从CLOSED状态转到SYN_SENT状态,若成功则转为ESTABLISHED状态;若发生错误,则不再可用,必须关闭再重新调用.

### bind函数
该函数绑定一个套接字到指定的网络地址.
```cpp
#include<sys/socket.h>
int bind(int sockfd,const struct sockaddr* addr,socklen_t addrlen);
成功则返回0,出错-1
```
- addr: 待绑定的网络地址结构对象
- addrlen: 上述地址结构体的字节大小
- bind的时候,IP地址和端口可以都指定,可以都不指定,或指定任何一个,都是合法的.

#### 指定网络地址
网络地址结构体中,可以不指定IP地址和端口,而是使用通配地址和通配端口.此时系统内核会自动选择地址和端口.

- 客户端通常不需指定端口, 服务器通常需要指定端口.

> 例外是远程过程调用(Remote Procedure Call,RPC)服务器,但在连接前需要进行端口告知.

- 一个主机可以有多个IP地址(多宿).
- 进程可以把套接字绑定一个IP地址,该地址必须归其所在主机所有.
- 绑定某些端口可能需要ROOT权限.

#### 客户端绑定IP地址
对于客户端来说,绑定IP地址意味着指定自身的源地址.
客户端通常不需要绑定IP地址,内核会根据所用外出网络接口来选择源IP地址,而所用外出接口取决于到达服务器所需路径.
即: 一个主机可以有多个IP地址,不同的地址可能接入到不同的网络.不指定IP地址会根据服务器路径自动选取.
#### 服务器绑定IP地址
对于服务器来说,绑定IP地址意味着只接收**目的地**为指定IP地址的报文.
一台主机上有多个IP地址,我们可以指定,只能有某个IP地址和外界通信,其他的IP地址一律不许接收信息.
> 报文的到达接口和目的接口可以是不同的(暂不明).后续将讨论弱端系统模型和强端系统模型.大多数实现都采用前者,这样报文只要其目的接口能够识别远程主机的某个接口就行,不必一定是它的到达接口.

如果服务器未指定自身IP地址, 那么在即将建立的连接中,服务器会把客户端SYN报文中的目标地址作为服务器地址.

#### 通配地址/端口号
通配端口号的数值为0.

- IPv4的通配地址为INADDR_ANY,其值一般为0. IP地址本身就是整数,可以简单赋值.

```c
struct sockaddr_in servaddr;
servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
```
尽管对于0来说,主机字节序和网络字节序一致,我们仍指明网络字节序.

- IPv6的通配地址为in6addr_any,不同于IPv4,它是一个结构体.

```c
#include<netinet/in.h>
struct sockaddr_in6 serv;
serv.sin6_addr = in6addr_any;
```
系统分配in6addr_any变量并初始化为常值IN6ADDR_ANY_INIT. 头文件`<netinet/in.h>`含有该变量的extern声明.

- 查看绑定的网络地址: getsockname函数.
- 出错: 常见的返回错误是EADDRINUSE(地址已使用).

### listen函数
listen函数仅由TCP服务器使用.
当创建一个套接字时,它被假设为一个主动套接字,即客户端套接字.listen函数把它转换为被动套接字,使其状态从CLOSED转为LISTEN状态.
```c
#include<sys/socket.h>
int listen(int sockfd,int backlog);
成功则返回0,出错-1
```
- 成功则返回0.

- backlog参数指定了套接字排队的最大连接个数.

服务器进程中:

- 用于接受连接请求的称作监听套接字;
- 用于直接和客户端对话的称作客户端套接字.

监听套接字维护两个队列:

- 未完成队列: 服务器收到其SYN请求,还未完成三次握手的客户端套接字队列, 它们的状态为SYN_RCVD状态.
- 已完成队列: 已完成三次握手的客户端套接字队列,状态为ESTABLISHED.


具体过程:

- 当客户端的SYN请求送达时,会在未完成队列末尾加入该客户端的排队,并完成三次握手; 成功握手后该客户端的排队移至已完成队列的末尾. 
- 当进程调用accept时, 返回已完成队列的首项给进程,若队列为空则投入睡眠,一直到出现可用项才被唤醒.

需要考虑的问题:

- 不要把backlog设置为0, 因为不同的系统可能有不同的解释,如果不需接受连接就关闭它.

- 收到客户端的SYN请求后,服务器会把它添加至未完成队列的末尾,并发出第二个握手. 若无意外,会在一个特定的时间内收到第三个握手,此时三次握手成功建立,该客户端加入已完成队列尾. 所以未完成队列的每项正常存活时间是一个RTT,约183ms.(除非超时失败)

- 若客户发起SYN请求时,未完成队列已满,则进程忽略该请求.(有些旧系统会回复RST,这是错误的,因为RST意味着硬错误,且易与"端口无进程监听"错误混淆)

- 在accept调用之前,就可以实现三次握手并收发数据.

- backlog参数曾规定为这已完成和未完成两个队列总和的最大值. 也有规定乘以模糊因子1.5后是未处理队列的最大长度. 但实际规定并不明确.我们可以指定一个较大值,但内核可能会截断至系统上限而不返回错误.

- 从历史上讲,backlog的初衷可能指的是已完成队列的最大长度,即服务器可以完成的最大连接数. 然而因为没有确切定义,不同系统的实现可能不同.

- 可以使用环境变量来指定backlog参数:
  ```c
  #include <stdlib.h>
  if( (ptr=getenv("LISTENQ")) != NULL )
    backlog=atoi(ptr);
  ```


### accept函数

由服务器调用,从已完成连接队列头返回一个连接.如果无可用项,则投入睡眠.

```c
#include<sys/socket.h>
int accept(int sockfd,struct sockaddr* addr,socklen_t* len);
//返回:成功则为描述符,失败为-1
```

可以简单理解为,服务器等待一个客户端连接,返回客户端的地址和地址结构体长度信息.如果对地址不关心,可以将后两个参数设置为空.

返回一个已连接套接字,作为与客户端的直接通信对象.

客户端地址结构体需要预先分配空间,并把空间大小作为第三个参数传入.第三个参数是"值-结果"参数.

### fork和exec函数

```c
#include<unistd.h>
pid_t fork(void);
//返回: 在子进程返回0,父进程返回子进程ID,出错返回-1
```

- 在fork的一瞬间,进程分身,子进程继承父进程的资源; 根据返回值,来"自我感知"当前处于父进程还是子进程.
- 子进程可以通过getppid获取父进程的ID
- 系统拥有套接字或文件,进程仅拥有描述符; 描述符是引用机制,如果指定某个描述符归子进程处理,那么父进程就需要关闭它.同理,子进程也要关闭父进程处理的描述符.

#### fork的典型用法

派生进程的唯一方法就是使用fork

- 创建进程的副本,处理指定的操作
- 创建进程的副本并调用exec把自身替换成新的程序

#### exec的用法

硬盘上的可执行文件被调用的唯一方法就是使用exec函数.具体包括6个:

```c
#include<unistd.h>
// 指定路径 + 参数列表(list)或向量(vector)
int execl(const char *path, const char *arg, ...);
int execv(const char *path, char *const argv[]);
// 指定路径 + 参数列表(list)或向量(vector) + 环境变量env
int execve(const char *path,char *const argv[],char* const envp[]);
int execle(const char *path, const char *arg, ..., char * const envp[]);
// 指定文件名 + 参数列表(list)或向量(vector)
int execlp(const char *file, const char *arg, ...);
int execvp(const char *file, char *const argv[]);
```

参数：

- path参数表示要启动程序的名称, 包括路径名

- arg参数表示启动程序所带的参数，一般第一个参数为要执行命令名(不带路径)

- 参数有两种形式:

  1. 列表, 不限定数量但需要以NULL元素作结尾
  2. 向量, 是一个指针数组,类型为```char* const *```,即是一个数组,数组原始是顶层const指针. 数组需要以NULL做结尾

  但环境变量仅有向量形式

- 返回值:成功返回0,失败返回-1. 

- 如果调用成功,则直接替换为新程序,不会返回到原始程序.

![exec](exec.png)

调用过程:

1. 上面那行的3个函数使用列表(list)形式. 如果调用是参数列表形式,则转换为向量形式(vector),向量以NULL作结尾.
2. 左列2个函数指定filename参数. 如果该参数不包含路径分隔符"/", 则认为是文件名, 会从系统PATH路径中搜索该文件,找到之后转换文件名至路径名.(与命令行的命令调用同理)
3. 对于execv调用,系统会取用外部变量environ的值,来转换至execve调用.environ被认为是包含了当前环境变量的列表.
4. execve由系统直接调用, 会先找到可执行文件,再把环境变量输送到该文件的执行环境. 环境变量的设定不会对函数调用者所处的环境产生影响. 该过程并不是更改当前的环境变量,再去执行新程序.
5. 调用exec之前打开的文件描述符通常跨exec继续保持打开.不过可以用fcntl设置FD_CLOEXEC来禁止.
6. 调用结束后子进程直接终止,不会返回调用点.

```c
char * const argv[] = {"ls","-al",NULL};
execl("/bin/ls",argv);
execv("/bin/ls","ls","-al",NULL);
execlp("ls","ls","-al",NULL);
char * const envp[] = {"PATH=/bin", "A=0", NULL};
execl("./hello", "hello", NULL);
execle("./hello", "hello", NULL, envp);
```

### 4.8 并发服务器

并发服务器通常用派生子进程来实现. 常见的形式为

```c
pid_t pid;
int listenfd,connfd;
listenfd=Socket(...);
Bind(...);
Listen(...);
for(;;){  				//循环,不断接收连接请求并派生进程处理
  connfd=Accept(...);   //接受连接
  if( (pid=Fork())==0 ){ //派生子进程;如果当前是子进程
    Close(listenfd);	//关闭父进程中的套接字
    do_it(connfd);		//作处理
    Close(connfd);		//处理完毕以后的清理工作
    exit(0);
  }
  Close(connfd);		//如果当前是父进程,关闭子进程处理的套接字
}
```

[图]

关于文件描述符:

- 描述符仅仅是对文件的索引, 有一个计数引用机制. 当派生出子进程后,子进程继承父进程的描述符资源,此时各描述符的引用数均增加一. 当进程close该描述符(或正常终止), 描述符的引用数减一. 当引用数为0时, 内核关闭该文件.
- 如果父进程处理监听套接字,子进程处理已连接套接字,那么子进程内要关闭监听套接字,使得监听套接字的引用数变为1(只在父进程中被引用),这样父进程才可以成功关闭监听套接字.同理,父进程需要关闭子进程处理的已连接套接字.
- 原则:各进程保留需要操作的描述符,关闭其他的描述符.

### 4.9 close 函数

```c
#include<unistd.h>
int close(int fd); // 成功则0,否则-1
```

- 该函数把某个描述符标记为关闭,然后立即返回.
- 其机制是减少描述符引用数,关闭描述符是由内核自动完成的.可以使用shutdown来主动发送一个FIN分节终止连接.
- 被关闭后的套接字不可以使用read和write参数,但是TCP会尝试发送排队等待发送的数据,然后完成终止TCP连接的四次挥手.设定SO_LINGER选项可以更改这个过程.
- 如果需要保持半关闭状态下的单向通信,则可以使用shutdown函数.

### 4.10 getsockname和getpeername函数

返回与某个套接字关联的本地或对端地址.

```c
#include<sys/socket.h>
int getsockname(int sockfd,struct sockaddr* addr,socklen_t* len);
int getpeername(int sockfd,struct sockaddr* addr,socklen_t* len);
```

- 上述函数可以返回一个连接的本端/对端地址结构,包括协议族, IP地址和端口.
- 长度参数是"值-结果"参数
- 套接字对象必须是一个已建立好的连接,不能是监听套接字.
- 子进程可以继承父进程的相关数据,但进程通过exec执行程序时只能通过上述函数获取连接双方地址信息.
- 如果不确定地址类型,建议用新的通用地址结构,以容纳足够长的地址