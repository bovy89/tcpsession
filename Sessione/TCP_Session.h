#ifndef __TCP_SESSION_H__
#define __TCP_SESSION_H__

#ifdef __TCP_SESSION_C__
/* dentro il modulo .c non sono extern */
#ifdef MY_EXTERN
#undef MY_EXTERN
#endif
#define MY_EXTERN
#else
/* fuori dal modulo .c sono extern */
#define MY_EXTERN extern
#endif

#define  INACTIVITY_TIMEOUT_SECONDS 60
#define  RETRY_TIMEOUT_SECONDS 600
#define  RETRY_INTERVAL_SECONDS 10
#define MAX_TENTATIVI (RETRY_TIMEOUT_SECONDS/RETRY_INTERVAL_SECONDS)

#ifndef SOCKET_ERROR
#define SOCKET_ERROR   ((int)-1)
#endif

#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/time.h> 

MY_EXTERN int	Init_TCP_Session_Module(void *ptr);
MY_EXTERN int	Socket(int domain, int type, int protocol);
MY_EXTERN int	TCP_Session_IPv4_Socket(void);
MY_EXTERN int	SetsockoptReuseAddr(int s);
MY_EXTERN int	GetsockoptReuseAddr(int s, int *pFlag);
MY_EXTERN int	SetNoBlocking(int s);
MY_EXTERN int	SetBlocking(int s);
MY_EXTERN int	IsBlocking(int s, int *pIsBlocking);
MY_EXTERN int	Bind(int  sockfd, struct sockaddr *my_addr, socklen_t addrlen);
MY_EXTERN int	Listen(int s, int backlog);
MY_EXTERN int	Accept(int   s,  struct  sockaddr  *addr,  socklen_t *addrlen);
MY_EXTERN int	Connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen);
MY_EXTERN ssize_t	Read(int fd, void *buf, size_t count);
MY_EXTERN ssize_t	Write(int fd, const void *buf, size_t count);
MY_EXTERN int	Send(int s, const void *msg, size_t len, int flags);
MY_EXTERN int	AvailableBytes(int s, int *pnum);
MY_EXTERN int	AvailableSpace(int s, int *pnum);
MY_EXTERN int	Select(int  n,  fd_set  *readfds,  fd_set  *writefds, fd_set *exceptfds, struct timeval *timeout);
MY_EXTERN int	Close(int fd);
MY_EXTERN int	CloseWait(int fd, int seconds);
MY_EXTERN int	CloseWait_TCP_Session_Module(void *ptr, struct timeval *timeout);
MY_EXTERN int	Close_TCP_Session_Module(void *ptr);


#endif   /* __TCP_SESSION_H__  */


