一般认为，服务器进程长期运行，主动请求由客户端发起。例外：异步回调(asynchronous callback)

通信层次：
![通信层次](https://github.com/yang-sk/note_unix_network_program_1/blob/master/C1_Introduction/img/net_communicate_levels.png?raw=true)
注:特别的，原始套接字(raw socket)可以绕过传输层，直接和网络层通信。

POSIX:Portable Operationg System Interface,可移植操作系统接口

一个简单的时间获取客户程序
```c

```
要点：TCP无消息边界。IP层掌管发送细节，因而发送结果可能与预想不同。可能出现：多次较短的报文被合并（粘包）；一次长的报文被拆分（拆包）。

包裹函数：用以自行处理错误，除非必须手动检测错误并做特别处理的情形。

一个简单的时间获取服务器程序
```c
#include "unp.h"
#include <time.h>
int main(int argc,char** argv){
  int listenfd,connfd;
  struct sockaddr_in servaddr;
  char buff[MAXLINE+1];
  time_t ticks;
  
  listenfd=Socket(AF_INET,SOCK_STREAM,0);
  
  bzero(&servaddr,sizeof(servaddr));
}
```
time函数返回自1970年的秒数；ctime把秒数转为日期字符串。

查看网络状态：netstat和ifonfig指令，提供接口状态和信息

服务器程序类型:迭代和并发。前者逐个处理客户请求，后者并行处理请求。

64位系统体系  
32位：称为ILP32模型  
64位：称为LP64模型。此时不能假设指针可以存放在一个整数中  

size_t类型：用于提供统一的接口，便于移植。它在32位中是32位的，在64位中是64位的。  
这意味着64位系统中可能隐含`typedef size_t unsigned long`定义  
套接字中设计socklen_t等类型，用于提供统一接口


