    #include "unp.h"    
      
    //此函数是在程序发生错误时被调用    
    //先输出字符串fmt，再根据errno输出错误原因（如果有的话），最后退出程序    
    //注：在多线程程序中，错误原因可能不正确    
    void error_quit(char *fmt, ...)    
    {    
        int res;    
        va_list list;    
        va_start(list, fmt);    
        res = vfprintf(stderr, fmt, list);    
        if( errno != 0 )    
            fprintf(stderr, " : %s", strerror(errno));    
        fprintf(stderr, "\n");    
        va_end(list);    
        exit(1);    
    }    
      
    //字节流套接字上调用read时，输入的字节数可能比请求的数量少，    
    //但这不是出错的状态，原因是内核中用于套接字的缓冲区可能已经达到了极限，    
    //此时需要调用者再次调用read函数    
    ssize_t readn(int fd, void *vptr, size_t n)    
    {    
        size_t  nleft;    
        ssize_t nread;    
        char    *ptr;    
      
        ptr = vptr;    
        nleft = n;    
        while (nleft > 0)    
        {    
            if ( (nread = read(fd, ptr, nleft)) < 0)    
            {    
                if (errno == EINTR)    
                    nread = 0;      /* and call read() again */    
                else    
                    return(-1);    
            }     
            else if (nread == 0)    
                break;              /* EOF */    
            nleft -= nread;    
            ptr   += nread;    
        }    
        return (n - nleft);     /* return >= 0 */    
    }    
      
    //字节流套接字上调用write时，输出的字节数可能比请求的数量少，    
    //但这不是出错的状态，原因是内核中用于套接字的缓冲区可能已经达到了极限，    
    //此时需要调用者再次调用write函数    
    ssize_t writen(int fd, const void *vptr, size_t n)    
    {    
        size_t      nleft;    
        ssize_t     nwritten;    
        const char  *ptr;    
      
        ptr = vptr;    
        nleft = n;    
        while (nleft > 0)     
        {    
            if ( (nwritten = write(fd, ptr, nleft)) <= 0)     
            {    
                if (nwritten < 0 && errno == EINTR)    
                    nwritten = 0;       /* and call write() again */    
                else    
                    return(-1);         /* error */    
            }    
            nleft -= nwritten;    
            ptr   += nwritten;    
        }    
        return n;    
    }    
      
    static int  read_cnt;    
    static char *read_ptr;    
    static char read_buf[MAXLINE];    
      
    //内部函数my_read每次最多读MAXLINE个字节，然后每次返回一个字节    
    static ssize_t my_read(int fd, char *ptr)    
    {    
        if (read_cnt <= 0)     
        {    
    again:    
            if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0)     
            {    
                if (errno == EINTR)    
                    goto again;    
                return(-1);    
            }     
            else if (read_cnt == 0)    
                return(0);    
            read_ptr = read_buf;    
        }    
      
        read_cnt--;    
        *ptr = *read_ptr++;    
        return 1;    
    }    
      
    //从描述符中读取文本行    
    ssize_t readline(int fd, void *vptr, size_t maxlen)    
    {    
        ssize_t n, rc;    
        char    c, *ptr;    
      
        ptr = vptr;    
        for (n = 1; n < maxlen; n++)    
        {    
            if ( (rc = my_read(fd, &c)) == 1)     
            {    
                *ptr++ = c;    
                if (c == '\n')    
                    break;  /* newline is stored, like fgets() */    
            }     
            else if (rc == 0)    
            {    
                *ptr = 0;    
                return(n - 1);  /* EOF, n - 1 bytes were read */    
            }    
            else    
                return(-1);     /* error, errno set by read() */    
        }    
        *ptr = 0;   /* null terminate like fgets() */    
        return n;    
    }    
      
    ssize_t Readn(int fd, void *ptr, size_t nbytes)    
    {    
        ssize_t n = readn(fd, ptr, nbytes);    
        if ( n < 0)    
            error_quit("readn error");    
        return n;    
    }    
      
    void Writen(int fd, void *ptr, size_t nbytes)    
    {    
        if ( writen(fd, ptr, nbytes) != nbytes )    
            error_quit("writen error");    
    }    
      
    ssize_t Readline(int fd, void *ptr, size_t maxlen)    
    {    
        ssize_t n = readline(fd, ptr, maxlen);    
        if ( n < 0)    
            error_quit("readline error");    
        return n;    
    }    
      
    ssize_t Read(int fd, void *ptr, size_t nbytes)    
    {    
        ssize_t n = read(fd, ptr, nbytes);    
        if ( n == -1)    
            error_quit("read error");    
        return n;    
    }    
      
    void Write(int fd, void *ptr, size_t nbytes)    
    {    
        if (write(fd, ptr, nbytes) != nbytes)    
            error_quit("write error");    
    }    
      
    int Open(const char *pathname, int flags, mode_t mode)  
    {  
        int fd = open(pathname, flags, mode);  
        if( -1 == fd )  
            error_quit("open file %s error", pathname);  
        return fd;  
    }  
      
    void Close(int fd)    
    {    
        if (close(fd) == -1)    
            error_quit("close error");    
    }    
      
    void Fputs(const char *ptr, FILE *stream)    
    {    
        if (fputs(ptr, stream) == EOF)    
            error_quit("fputs error");    
    }    
      
    char *Fgets(char *ptr, int n, FILE *stream)    
    {    
        char *rptr = fgets(ptr, n, stream);    
        if ( rptr == NULL && ferror(stream) )    
            error_quit("fgets error");    
        return rptr;    
    }    
      
    int Socket(int family, int type, int protocol)    
    {    
        int n = socket(family, type, protocol);    
        if( n < 0)    
            error_quit("socket error");    
        return n;    
    }    
      
    void Inet_pton(int family, const char *strptr, void *addrptr)    
    {    
        int n = inet_pton(family, strptr, addrptr);    
        if( n < 0)    
            error_quit("inet_pton error for %s", strptr);    
    }    
      
    void Connect(int fd, const struct sockaddr *sa, socklen_t salen)    
    {    
        if (connect(fd, sa, salen) < 0)    
            error_quit("connect error");    
    }    
      
    void Listen(int fd, int backlog)    
    {    
        if (listen(fd, backlog) < 0)    
            error_quit("listen error");    
    }    
      
    void Bind(int fd, const struct sockaddr *sa, socklen_t salen)    
    {    
        if (bind(fd, sa, salen) < 0)    
            error_quit("bind error");    
    }    
      
    int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)    
    {    
        int n = accept(fd, sa, salenptr);    
        if ( n < 0)    
            error_quit("accept error");    
        return n;    
    }    
      
    const char *Inet_ntop(int family, const void *addrptr, char *strptr, size_t len)    
    {    
        const char  *ptr = inet_ntop(family, addrptr, strptr, len);    
        if ( ptr == NULL)    
            error_quit("inet_ntop error");    
        return ptr;    
    }    
      
    pid_t Fork(void)    
    {    
        pid_t pid = fork();    
        if ( pid == -1)    
            error_quit("fork error");    
        return pid;    
    }    
      
    Sigfunc *Signal(int signo, Sigfunc *func)    
    {    
        Sigfunc *sigfunc = signal(signo, func);    
        if ( sigfunc == SIG_ERR)    
            error_quit("signal error");    
        return sigfunc;    
    }    
      
    int Select(int nfds, fd_set *readfds, fd_set *writefds,     
               fd_set *exceptfds, struct timeval *timeout)    
    {    
        int n = select(nfds, readfds, writefds, exceptfds, timeout);    
        if ( n < 0 )    
            error_quit("select error");    
        return n;       /* can return 0 on timeout */    
    }    
      
    int Poll(struct pollfd *fdarray, unsigned long nfds, int timeout)    
    {    
        int n = poll(fdarray, nfds, timeout);    
        if ( n < 0 )    
            error_quit("poll error");    
        return n;    
    }    
      
    void Shutdown(int fd, int how)    
    {    
        if (shutdown(fd, how) < 0)    
            error_quit("shutdown error");    
    }    
      
    int Epoll_create(int size)    
    {    
        int n = epoll_create(size);    
        if( n < 0 )    
            error_quit("epoll create error");    
        return n;    
    }    
      
    void Epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)    
    {    
        if( epoll_ctl(epfd, op, fd, event) < 0 )    
            error_quit("epoll ctl error");    
    }    
      
    int Epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)    
    {    
        int n = epoll_wait(epfd, events, maxevents, timeout);    
        if( n < 0 )    
            error_quit("epoll wait error");    
        return n;    
    }    
      
    void Sendto(int fd, const void *ptr, size_t nbytes, int flags,    
                const struct sockaddr *sa, socklen_t salen)    
    {    
        if (sendto(fd, ptr, nbytes, flags, sa, salen) != (ssize_t)nbytes)    
            error_quit("sendto error");    
    }    
      
    ssize_t Recvfrom(int fd, void *ptr, size_t nbytes, int flags,    
    struct sockaddr *sa, socklen_t *salenptr)    
    {    
        ssize_t n = recvfrom(fd, ptr, nbytes, flags, sa, salenptr);    
        if ( n < 0 )    
            error_quit("recvfrom error");    
        return n;    
    }    
      
    ssize_t Recvmsg(int fd, struct msghdr *msg, int flags)    
    {    
        ssize_t n = recvmsg(fd, msg, flags);    
        if ( n < 0 )    
            error_quit("recvmsg error");    
        return(n);    
    }    
      
    void *Malloc(size_t size)    
    {    
        void *ptr = malloc(size);    
        if ( ptr == NULL )    
            error_quit("malloc error");    
        return ptr;    
    }    
      
    void *Calloc(size_t n, size_t size)  
    {  
        void *ptr = calloc(n, size);  
        if ( ptr == NULL)  
            error_quit("calloc error");  
        return ptr;  
    }  
      
    void Pipe(int *fds)    
    {    
        if ( pipe(fds) < 0 )    
            error_quit("pipe error");    
    }    
      
    pid_t Waitpid(pid_t pid, int *iptr, int options)    
    {    
        pid_t   retpid = waitpid(pid, iptr, options);    
        if ( retpid == -1)    
            error_quit("waitpid error");    
        return retpid;    
    }  
      
    void Setsockopt(int fd, int level, int optname,   
                    const void *optval, socklen_t optlen)  
    {  
        if (setsockopt(fd, level, optname, optval, optlen) < 0)  
            error_quit("setsockopt error");  
    }  
      
    void Socketpair(int family, int type, int protocol, int *fd)  
    {  
        int n = socketpair(family, type, protocol, fd);  
        if ( n < 0 )  
            error_quit("socketpair error");  
    }  
      
    void Dup2(int fd1, int fd2)  
    {  
        if (dup2(fd1, fd2) == -1)  
            error_quit("dup2 error");  
    }  
      
    void *Mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)  
    {  
        void *ptr = mmap(addr, len, prot, flags, fd, offset);  
        if ( ptr == MAP_FAILED )  
            error_quit("mmap error");  
        return ptr;  
    }  
      
    void Munmap(void *addr, size_t len)  
    {  
        if (munmap(addr, len) == -1)  
            error_quit("munmap error");  
    }  
      
    void Ftruncate(int fd, off_t length)  
    {  
        if (ftruncate(fd, length) == -1)  
            error_quit("ftruncate error");  
    }  
