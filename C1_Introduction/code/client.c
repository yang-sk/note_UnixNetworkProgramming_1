#include "unp.h" // for MAXLINE
int main(int argc, char** argv){
    int sockfd;                 //套接字
    ssize_t n;                  //接收数据计数
    char recvline[MAXLINE+1];   //接收信息并显示
    struct sockaddr_in servaddr;//服务器地址

    if(argc!=2)
        error_quit("need <IPaddress>");

    if( (sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
        error_quit("socket error");

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(13);
    if(inet_pton(AF_INET,argv[1],&servaddr.sin_addr)<=0)
        error_quit("inet_pton error for %s",argv[1]);

    if(connect(sockfd,(SA*)&servaddr,sizeof(servaddr))<0)
        error_quit("connect error");

    while( (n=read(sockfd,recvline,MAXLINE))>0){
        recvline[n]=0;
        if(fputs(recvline,stdout)==EOF)
            error_quit("fputs error");
    }
    if(n<0)
        error_quit("read error");
    exit(0);
}