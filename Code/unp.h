#ifndef UNP_H_    
#define UNP_H_    
  
#include <stdio.h>    
#include <stdarg.h>    
#include <stdlib.h>    
#include <string.h>    
#include <time.h>    
  
#include <unistd.h>    
#include <errno.h>    
#include <pthread.h>  
#include <semaphore.h>  
#include <sys/socket.h>    
#include <sys/wait.h>    
#include <netinet/in.h>    
#include <arpa/inet.h>    
#include <sys/select.h>    
#include <sys/epoll.h>    
#include <sys/poll.h>    
#include <sys/file.h>    
#include <sys/mman.h>  
  
#define MAXLINE 1024    
#define LISTENQ 1024    
  
#define MAXNITEMS 1000000  
#define MAXNTHREADS 100  
  
#define SERV_PORT 9877    
#define SERV_PORT_STR "9877"    
  
#define SA struct sockaddr    
typedef void Sigfunc(int);    
  
#define min(a,b)    ((a) < (b) ? (a) : (b))    
#define max(a,b)    ((a) > (b) ? (a) : (b))    
  
  
//错误处理函数，输出错误信息后退出程序    
void error_quit(char *fmt, ...);    
  
//为了适应网络的慢速IO而编写的读写函数  
ssize_t readn(int fd, void *vptr, size_t n);    
ssize_t writen(int fd, const void *vptr, size_t n);    
ssize_t readline(int fd, void *vptr, size_t maxlen);    
  
//各类读写包裹函数    
void Write(int fd, void *ptr, size_t nbytes);    
ssize_t Read(int fd, void *ptr, size_t nbytes);    
ssize_t Readn(int fd, void *ptr, size_t nbytes);    
void Writen(int fd, void *ptr, size_t nbytes);    
ssize_t Readline(int fd, void *ptr, size_t maxlen);    
void Fputs(const char *ptr, FILE *stream);    
char *Fgets(char *ptr, int n, FILE *stream);     
  
//各类标准包裹函数    
int Open(const char *pathname, int flags, mode_t mode);  
void Close(int fd);   
Sigfunc *Signal(int signo, Sigfunc *func);    
void *Malloc(size_t size);    
void *Calloc(size_t n, size_t size);  
void Pipe(int *fds);    
pid_t Fork(void);    
pid_t Waitpid(pid_t pid, int *iptr, int options);   
void Dup2(int fd1, int fd2);  
  
//各类网络包裹函数    
int Socket(int family, int type, int protocol);    
void Inet_pton(int family, const char *strptr, void *addrptr);    
void Connect(int fd, const struct sockaddr *sa, socklen_t salen);    
void Listen(int fd, int backlog);    
void Bind(int fd, const struct sockaddr *sa, socklen_t salen);    
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr);    
const char *Inet_ntop(int family, const void *addrptr, char *strptr, size_t len);    
int Select(int nfds, fd_set *readfds, fd_set *writefds,     
           fd_set *exceptfds, struct timeval *timeout);    
int Poll(struct pollfd *fdarray, unsigned long nfds, int timeout);    
void Shutdown(int fd, int how);    
int Epoll_create(int size);    
void Epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);    
int Epoll_wait(int epfd, struct epoll_event *events,    
               int maxevents, int timeout);    
void Sendto(int fd, const void *ptr, size_t nbytes, int flags,    
            const struct sockaddr *sa, socklen_t salen);    
ssize_t Recvfrom(int fd, void *ptr, size_t nbytes, int flags,   
                struct sockaddr *sa, socklen_t *salenptr);    
void Setsockopt(int fd, int level, int optname,   
                const void *optval, socklen_t optlen);  
void *Mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);  
void Munmap(void *addr, size_t len);  
void Ftruncate(int fd, off_t length);  
  
//各类和线程操作相关的包裹函数    
void Pthread_create(pthread_t *tid, const pthread_attr_t *attr,  
                    void * (*func)(void *), void *arg);  
void Pthread_detach(pthread_t tid);  
void Pthread_join(pthread_t tid, void **status);  
void Pthread_kill(pthread_t tid, int signo);  
void Pthread_mutex_lock(pthread_mutex_t *mptr);  
void Pthread_mutex_unlock(pthread_mutex_t *mptr);  
//此函数相当于UNP书上的set_concurrency函数  
void Pthread_setconcurrency(int level);  
void Pthread_cond_signal(pthread_cond_t *cptr);  
void Pthread_cond_wait(pthread_cond_t *cptr, pthread_mutex_t *mptr);  
  
//各类和信号量相关的包裹函数  
sem_t *Sem_open(const char *name, int oflag,  
                mode_t mode, unsigned int value);  
void Sem_close(sem_t *sem);  
void Sem_unlink(const char *pathname);  
void Sem_init(sem_t *sem, int pshared, unsigned int value);  
void Sem_destroy(sem_t *sem);  
void Sem_wait(sem_t *sem);  
void Sem_post(sem_t *sem);  
void Sem_getvalue(sem_t *sem, int *valp);  
  
#endif   
