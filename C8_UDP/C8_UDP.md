# 第8章 基本UDP套接字编程

无连接，每次发送和接收时制定对方网络地址

#### UDP应用

DNS NFS SNMP

### recvfrom和sendot

```c
#include<sys/socket.h>
ssize_t recvfrom(int sockfd,void* buff,size_t nbytes,int flags,struct sockaddr* from,socklen_t* addrlen);
ssize_t sendto(int sockfd,const void* buff,size_t nbytes,int flags,const struct sockaddr* to,socklen_t addrlen);
均返回：成功则读或写的字节数，出错-1
```

- 前三个参数，即描述符，读写缓冲区指针，读写字节数。
- flags参数暂不讨论，可置零。
- sendto中to指向目标地址，大小为addrlen。recvfrom的from指向源地址，addrlen参数返回长度
- sendto最后参数是整数，recvfrom是指针（值-结果）

返回读写长度，通常是发送和接收的数据量。

写一个长度为0的数据报是可行的。此时数据报中数据区长度为0.接收者返回0。而TCP接收read返回0表示对方终止连接。

如果recvfrom的from是空，则长度参数也必须为空，代表不关心数据发送者。

recvfrom和sendto可用于TCP，虽然没什么必要。

## 回射服务器

```c
#include "unp.h"
int main(){
	int sockfd;
  struct sockaddr_in servaddr,cliaddr;
  
  sockfd=Socket(AF_INET,SOCK_DGRAM,0);
  
  bzero(&servaddr,sizeof(servaddr));
  servaddr.sin_family=AF_INET;
  servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
  servaddr.sin_port=htons(SERV_PORT);
  
  Bind(sockfd,(SA*)&servaddr,sizeof(servaddr));
  
  dg_echo(sockfd,(SA*)&cliaddr,sizeof(cliaddr));
}
```

```c
void dg_echo(int sockfd,SA* pcliaddr,socklen_t clilen){
  int n;
  socklen_t len;
  char mesg[MAXLINE];
  for(;;){
    len=clilen;
    n=Recvfrom(sockfd,mesg,MAXLINE,0,pcliaddr,&len);
    Sendto(sockfd,mesg,n,0,pcliaddr,len);
  }
}
```

- 大多数TCP服务器是并发的，大多数UDP是迭代的
- 对于TCP服务器而言，有一个监听套接字和多个已连接套接字，每个已连接套接字与客户通信，且具有独立的缓冲区。而UDP只有一个套接字，顺次（而非同时）与多个客户通信，只有一个接收缓存区。因而更容易溢出。
- ​

## 客户函数

```c
#include "unp.h"
int main(){
  int sockfd;
  struct sockaddr_in servaddr;
  
  bzero(&servaddr,sizeof(servaddr));
  servaddr.sin_family=AF_INET;
  servaddr.sin_port=htons(SERV_PORT);
  Inet_pton(AF_INET,argv[1],&servaddr.sin_addr);
  
  sockfd=Socket(AF_INET,SOCK_DGRAM,0);
  
  dg_cli(stdin,sockfd,(SA*)&servaddr,sizeof(servaddr));
  exit(0);
}
```

```c
void dg_cli(FILE* fp,int sockfd,const SA* pservaddr,socklen_t servlen){
  int n;
  char sendline[MAXLINE],recvline[MAXLINE+1];
  while(Fgets(sendlien,MAXLINE,fp)!=NULL){
    Sendto(sockfd,sendline,strlen(sendline),0,pservaddr,servlen);
    
    n=Recvfrom(sockfd,recvline,MAXLINE,0,NULL,NULL);
    
    recvline[n]=0;
    Fputs(recvline,stdout);
  }
}
```

接收时，如果不指定发送方地址，则任何到达的数据报都会被接收

## 数据报的丢失

可能出现丢失，双方都不指定，对方还阻塞在接收上。

常用手段，设置定时。

对于其中一方，如果没有收到对方应答，不知道是对方回答丢失，还是本方的请求丢失。

## 验证收到的响应

服务器接收到对方的数据，登记其地址，并限定以后接收到的来源，避免数据混杂。

```c
void dg_cli(FILE* fp,int sockfd,const SA* pservaddr,socklen_t servlen){
  int n;
  char sendline[MAXLINE],recvline[MAXLINE+1];
  socklen_t len;
  struct sockaddr* preply_addr;
  
  preply_addr=Malloc(servlen);
  while(Fgets(sendline,MAXLINE,fp)!=NULL){
    Sendto(sockfd,sendline,strlen(sendline),0,pservaddr,servlen);
    len=servlen;
    n=Recvfrom(sockfd,recvline,MAXLINE,0,preply_addr,&len);
    if(len!=servlen || memcmp(pservaddr,preply_addr,len)!=0){
      printf("reply from %s (ignored)",Sock_ntop(preply_addr,len));
      contineu;
    }
    revline[n]=0;
    Fputs(recfline,stdout);
  }
}
```

