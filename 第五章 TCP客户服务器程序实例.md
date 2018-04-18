# 第五章 TCP客户服务器程序实例

## 5.1 概述

一个简单的回射程序结构示例

```c
#include "unp.h"
int main(){
  int listenfd,connfd;
  pid_t childpid;
  socklen_t clilen;
  struct sockaddr_in cliaddr,servaddr;
  
  listenfd=Socket(AF_INET,SOCK_STREAM,0);
  
  bzero(&servaddr,sizeof(servaddr));
  servaddr.sin_family=AF_INET;
  servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
  servaddr.sin_port=htons(SERV_PORT);
  
  Bind(listenfd,(SA*)&servaddr,sizeof(servaddr));
  
  Listen(listenfd,LISTENQ);
  for(;;){
    clilen=sizeof(cliaddr);
    connfd=Accept(listenfd,(SA*)&cliaddr,&clilen);
    if( (childpid=Fork())==0 ){
      Close(listenfd);
      str_echo(connfd);
      exit(0);
    }
    Close(connfd);
  }
}
```

```c
void str_echo(int sockfd){
  ssize_t n;
  char buf[MAXLINE];
  
again:
  while( (n=read(sockfd,buf,MAXLINE))>0)
    Writen(sockfd,buf,n);
  if(n<0 && errno==EINTR)
    goto again;
  else if(n<0)
    err_sys("str_echo:read error");
}
```

#### 客户程序

```c
#include"unp.h"
int main(){
  int sockfd;
  struct sockaddr_in servaddr;
  if(argc!=2)
    err_quit("usage: tcpcli <IPADDRESS>");
  
  sockfd=Socket(AF_INET,SOCK_STREAM,0);
  
  bzero(&servaddr,sizof(servaddr));
  servaddr.sin_family=AF_INET;
  servaddr.sin_port=htons(SERV_PORT);
  Inet_pton(AF)INET,argv[1],&servaddr.sin_addr);
  
  Connect(sockfd,(SA*)&servaddr,sizeof(servaddr));
  
  str_cli(stdin,sockfd);
  exit(0);
}
```

```c
void str_cli(FILE* fp,int sockfd){
  char sendline[MAXLINE],recvline[MAXLINE];
  
  while(Fgets(sendline,MAXLINE,fp)!=NULL){
    Writen(sockfd,sendline,strlen(sendline));
    
    if(Readline(sockfd,recvline,MAXLINE)==0)
      err_quit("str_cli:server terminated");
    Fputs(recvline,stdout);
  }
}
```

### 正常启动

服务器启动 -> socket,bind,listen -> accept(阻塞)

此时，服务器套接字处于LISTEN状态

客户端启动 -> socket -> connect(引发三次握手) -> 服务器accept和客户端connect均返回-> 连接建立

此时，客户端阻塞于fgets调用，服务器fork出子进程，子进程阻塞于read调用，父进程继续循环，并阻塞于accept调用

 TIP：使用netstat -a和ps -o pid,ppid,tty,stat,args,wchan命令查看套接字和进程状态。

客户进程输入-〉write->服务器read-〉服务器write-〉客户read （循环）

当客户EOF时，fgets返回NULL，str_cli函数终止，main终止。客户进程中描述符被关闭，发出FIN，开始四次挥手。服务器接收FIN，传送EOF给进程，回馈ACK给客户。

服务器进程接受EOF，read返回0，str——echo返回，main返回。进程终止，发出FIN。客户接受FIN并回复ACK并进入TIME-WAIT。服务器接受ACK进入CLOSED。

#### 进程与信号处理

子进程终止时发生SIGCHLD给父进程，父进程必须调用wait或waitpid给子进程收尸，才能正常结束，否则子进程陷入僵死。如果父进程结束，僵死子进程会过继给INIT进程处理。

信号也称软件中断，通常是异步发生的。信号可以由进程发送另一个进程或内核发送进程。

进程接受信号并调用处理函数的过程称为捕获。

对信号的处理分为：

1. 提供信号处理函数（signal handler）在信号捕获时自动调用。SIGKILL和SIGSTOP不可以被捕获。信号处理函数类型为

   ```c
   void handler(int signo);
   ```

   使用sigaction函数用来绑定信号类型和相应的处理函数。

2. 把信号处理设为SIG——IGN来忽略它。SIGKILL和SIGSTOP不可以忽略

3. 设为SIG——DFL启用默认处置。通常为收到信号后终止，有些是忽略

signal函数

POSIX中定义sigaction函数，然而较容易的方法是signal函数，它在很多厂家中都提供。如果没有提供，我们自己定义signal函数。

```c
#include "unp.h"
Sigfunc* signal(int signo,Sigfunc *func){
  struct sigactionact,oact;
  
  act.sa_handler=func;
  sigemptyset(&act.sa_mask);
  act.sa_flags=0;
  if(signo==SIGALRM){
    #ifdef SA_INTERRUPT
    act.sa_flags |= SA_INTERRUPT;
    #endif
  }else{
    #ifdef SA_RESTART
    act.sa_flags |= SA_RESTART;
    #endif
  }
  if(sigaction(signo,&act,&oact)<0)
    return (SIG_ERR);
  return (oact.sa_handler);
}
```

- 函数signal的原型因为层次太多所以很复杂

  ```
  void (*signal(int signo,void (*func)(int))) (int);
  ```

  为了简便起见，定义`typedef void Sigfunc(int);`

- 设置处理函数

  sigaction结构中的sa——handler被设置为func参数

- 信号掩码

  信号的阻塞：表示信号不会被递交给进程。可以设置sa——mask为空，代表该信号函数运行期间，不阻塞额外类型的信号。信号在被处理函数执行期间，同种信号被阻塞。

- SA——RESTART标志

  该标志可选，信号捕获时，进程会中断，进入信号处理函数。如果设置，则内核会自动重启被中断的调用。如果系统支持SA——RESTART，我们对非SIGALRM信号设置该标志。SIGALRM通常是为IO操作设置超时使用，一般目的在于中断调用。早期系统默认自动重启并定义了互补的SA—INTERRUPT标志。此时我们就在SIGALRM时设置它。

POSIX信号语义

- 一旦安装了信号处理函数，则一直安装着。
- 某个信号处理函数运行期间，同种信号或sa——mask指定的信号会被阻塞。
- 如果一个信号在被阻塞期间产生了一次或多次，那么信号被解阻塞后通常只低级递交一次，即信号默认不排队。
- sigprocmask可以选择性地阻塞或解阻塞一族信号。这使得可以在一段临界区代码执行期间防止捕获某些信号，以保护这段代码

### 5.9 处理SIGCHLD信号

设置僵死状态的目的是维护子进程的信息，以便于父进程在以后某个时刻获取。如果

定义信号处理函数

```c
void sig_chld(int signo){
  pid_t pid;
  int stat;
  pid=wait(&stat);
  printf("child %d terminated.",pid);
}
```

因为信号是给父进程的，所以必须在fork前写入`Signal(SIGCHLD,sig_child)`ll来进行绑定，且只绑定一次。

> 信号处理函数中不宜调用printf这样标准IO函数

某些调用可能花费很长时间，这些调用称为慢调用。子进程终止时父进程阻塞于accept，SIGCHLD信号递交时，父进程会中断。此时accept调用会返回EINTR错误。

在编写健全的程序时，通常需要对被中断的慢调用进行处理。某些系统环境下的signal函数不会让内核自动重启被中断的调用，即SA——RESTART没有被设置。有些系统环境会自动重启。

某些系统不支持SA-RESTART标志，即使有，也不保证所有中断都可以自动重启。

满系统调用可适用于那些可能永远阻塞的系统调用。例如对管道和终端设备的读写。一个例外是磁盘IO，通常立即返回。

为了保证移植性，通常在慢调用外围加死循环或用goto来实现自重启。此时accept不应该使用包裹函数，因为我们需要做特别处理

```c
for(;;){
  chilen=sizeof(cliaddr);
  if( (connfd=accept(listenfd,(SA*)&cliaddr,&clilen))<0){
    if(errno==EINTR)
      continue;
    else
      err_sys("accept error");
  }
  break;
}
```

对于accept read write select oepn之类的函数，重启是合适的。但是connect例外，如果不能被内核自动重启，那么也不能被进程重启，它被中断后必须使用select来等待连接完成。

### wait和waitpid函数

```c
#include<sys/wait.h>
pid_t wait(int *statloc);
pid_t waitpid(pid_t pid,int* statloc, int options);
	均返回：若成功则为进程ID，否则-1
```

返回已终止子进程的ID号，以及通过statloc指针返回终止状态。

wait函数被调用时，如果当前没有已终止的子进程，不过有存在子进程在运行，则会阻塞到某个子进程终止为止。

waitpid可以指定特定的子进程，或-1表示等待任意子进程。其次可以有附加选项，WNOHANG表示不阻塞

假设当前服务器连接了5个客户端，客户端在几乎同一时刻终止，则服务器的5个子进程几乎在同一时刻终止，5个SIGCHLD被递交给父进程。某个信号触发了处理函数，在处理函数运作其间，后续的信号被阻塞。所以不是每个信号都得到了妥善处理。

> 有迹象说明，某个信号处理函数运作时，后续同种信号似乎都被丢弃了。

可以使用waitpid搭配非阻塞和循环来完成。

```c
void sig_child(int signo){
  pid_t pid;
  int stat;
  while( (pid=waitpid(-1,&stat,WNOHANG))>0)
    printf("child %d over.",pid);
}
```

当某个信号触发处理函数后，处理函数会不断地搜索当前所有已终止子进程，并进行处理。当搜索不到已终止子进程时会返回。因为设置非阻塞，所以每轮搜索会很快。这样的话，可以应对（几乎）同时到达的多个信号。

最终版本

- 信号及处理函数
- accept手动重启
- waitpid函数

```c
#include "unp.h"
int main(){
  int listenfd,connfd;
  pid_t childpid;
  socklen_t clilen;
  struct sockaddr_in cliaddr,servaddr;
  void sig_chld(int);
  
  listenfd=Socket(AF_INET,SOCK_STREAM,0);
  
  bzero(&servaddr,sizeof(servaddr));
  servaddr.sin_family=AF_INET;
  servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
  servaddr.sin_port=htons(SERV_PORT);
  
  Bind(listenfd,(SA*)&servaddr,sizeof(servaddr));
  
  Listen(listenfd,LISTENQ);
  
  Signal(SIGCHLD,sig_chld);
  for(;;){
    clilen=sizeof(cliaddr);
    if( (connfd=accept(listenfd,(SA*)&cliaddr,&clilen))<0){
      if(errno==EINTR)
        continue;
      else
        err_sys("accpet error");
    }
    if( (childpid=Fork())==0){
      Close(listenfd);
      str_echo(connfd);
      exit(0);
    }
    Close(connfd);
  }
}
```

## accept返回前连接终止

accept函数除了被信号中断外，还有可能被以下错误中断。此时只需再次调用accept即可

在三次握手时，服务器在listen之后，不需accept即可接受SYN。三次握手完毕后，可能在某些情况下，客户发出RST

【图】

不同的实现处理不同。源于Berkeley的实现在内核处理，进程不可见。SVR4返回错误给进程作为accept的返回结果，某些返回EPROTO，POSIX规定返回ECONNABORTED

### 服务器子进程终止

服务器子进程被终止，描述符关闭，发送FIN。客户响应ACK。

SIGCHLD被父进程捕获并正常处理

此时客户响应ACK是内核自动完成的，客户进程还阻塞在fgets上

此时前两次挥手已经完成。FIN—WAIT2 Close——WAIT

重申：套接字不隶属于进程。进程结束了，套接字还在，它接受了FIN ACK转为了FINwait2状态？？

客户接受键盘输入，并发送至对端。允许写入一个已经发出FIn的套接字。

服务器接受报文，但是没有进程来操作套接字，因此回复RST

客户在调用write后立即调用read，此时FIn已被接受，【RST???】read会读到EOF并返回0，。进程并不察觉接受到了RST，RST是很重要的。

客户终止，关闭描述符

本例中问题在于，对方发出FIn时，本方阻塞在fgets上，不能实时响应。

### SIGPIPE信号

当进程向某个已收到RST的套接字执行写操作时，内核向进程发送SIGPIPE信号，默认行为是终止进程。如果不期望，可以绑定处理函数来自行处理。但无论捕获信号还是忽略信号，写操作都会引发EPIPE错误。

写一个已接收FIn的套接字可以；但是RST会是错误。【读？】

处理SIGPIPE如果没有特别的事情可做，可设为SIG_IGN，并假设后续的write捕捉到EPIPE并终止。

注意：如果使用多个套接字，将无法确认是哪个出的错。如果需要得知，则必须忽略SIGPIPE并从write返回错误处判断。

## 5.14 服务器主机崩溃

主机崩溃时无法发送FIN

 主机崩溃不是系统关闭

客户发送数据时，因为得不到对方确认，会重发数次直至失败。但是write是调用成功，因为它仅仅表示拷贝进入发送缓存区。数据不可达的错误返回进程时，进程在read调用上，将返回一个错误ETIMEDOUT（数据到达主机时无应答）或EHOSTUNREACH ENETUNREACH（在某路由上被判定不可达）

所以可以给read设定一个超神

只有向服务器发送数据才能检测到连接断开。SO——KEEPALIVE选项用于自动检测

## 5。15主机崩溃重启

主机崩溃，客户不知道。主机重启后，客户发送数据，主机没有进程来处理该套接字的数据，会回复RST。此时客户正read，返回CONNRESET错误

## 服务器主机关机

关机时，Init进程给所有进程发送SIGTERM信号，该信号可被捕获；等待5-20秒后发送SIGKILL（不能捕获）。终止进程。



## 数据格式

### 文本串

支持性极好，因为字节流和文本串有直接对应关系。

【图】

### 二进制结构

非文本串的数据类型，以字节流传输。不保证准确

- 大端模式小端模式
- 32位64位
- 对齐限制，short，double等类型规范不统一等

解决方法

- 字符串
- 显式规定格式



### 关于

如果把标准输入重定向到二进制文件：二进制文件有```\0```这样读入到字符串中，发送时使用了strlen，此时会从中间0截断作为字符串。所以会少发









s

