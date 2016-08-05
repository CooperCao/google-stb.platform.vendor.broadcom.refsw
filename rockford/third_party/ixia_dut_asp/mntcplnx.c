/*
  mntclnx.c: Abstraction for Linux OS calls made from mntcpapp.c

  Copyright (c) Ixia 2002-2009
  All rights reserved.
 
*/

/* This file contains function definitions (wrappers) for linux-specifc
   calls made from mntcpapp.c. */

/* System Includes */
#include <stdarg.h>
#include <sys/errno.h>
#include "fcntl.h"
#include <signal.h>
#include <assert.h>
#include <sys/ioctl.h>

#ifdef __GLIBC__
#include "net/if.h"
#include "netinet/in.h"
#include "netinet/tcp.h"
#else
#include "linux/socket.h"
#include "linux/in.h"
#endif /* __GLIBC__ */

/* Local Includes */ 

#include "./include/mntcpos.h"
#include "./include/mntcpapp.h"


#if 0
#include "asp_manager.h"
#else
#include "../../asp/emulation/dut/asp_manager_api.h"
ASP_ChannelMgrHandle ghAspChannelMgr = NULL; /* Global Asp channel manager handle.*/
int gFirstTimeFlag = 1;
#endif
extern int gUseAspSimulator;
extern char gAspProxyServerIp[];

/* 
   Prototypes of system functions we use (from /usr/include/sys).
   They are included here so that we know what they are and if they 
   change.  
*/
#ifdef __GLIBC__
extern int bind(int sockfd, __CONST_SOCKADDR_ARG my_addr, socklen_t addrlen); 
extern ssize_t sendto(int s, const void *msg, size_t len, int flags, 
				  __CONST_SOCKADDR_ARG to, socklen_t tolen); 
extern ssize_t recvfrom(int s, void *buf, size_t len, int flags,
					__SOCKADDR_ARG from, socklen_t *fromlen);
extern int connect(int sockfd, __CONST_SOCKADDR_ARG serv_addr, 
				   socklen_t addrlen);
extern int accept(int s, __SOCKADDR_ARG addr, socklen_t *addrlen);
extern int setsockopt(int s, int level, int optname, const void *optval, 
					  socklen_t optlen); 
extern int  getsockopt(int s, int level, int optname, void *optval, 
					   socklen_t *optlen);
extern ssize_t  send(int  s, const void *msg, size_t len, int flags);
extern ssize_t recv(int s, void *buf, size_t len, int flags);
#ifdef LIB_IPV6
extern const char *inet_ntop (int af, const void *cp,
                        char *buf, socklen_t len);

extern int inet_pton (int af, const char *cp,
                          void *buf);


#endif /* LIB_IPV6 */
#else
extern int bind(int sockfd,  struct sockaddr *my_addr, int addrlen); 
extern int sendto(int s, const void *msg, int len, unsigned int flags, 
				  const struct sockaddr *to, int tolen); 
extern int recvfrom(int s, void *buf, int len, unsigned int flags,
					struct sockaddr *from, int *fromlen);
extern int  connect(int sockfd, struct sockaddr *serv_addr, int addrlen);
extern int accept(int s, struct sockaddr *addr, int *addrlen);
extern int listen(int s, int backlog);
extern int setsockopt(int s, int level, int optname, const void *optval, 
					  int optlen); 
extern int  getsockopt(int s, int level, int optname, void *optval, 
					   int *optlen);
extern int  send(int  s,  const  void *msg, int len, unsigned int flags);
extern int recv(int s, void *buf, int len, unsigned int flags);
#endif /* __GLIBC__ */

extern int shutdown(int s, int how);
extern void exit(int status);
extern int fprintf(FILE *stream, const char *format, ... );
extern size_t strlen(const char *s);
extern int strcmp(const char *s1, const char *s2);
extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *dst, const char* src, size_t len);
extern int strncmp (const char *s1, const char *s2, size_t len);
extern void *memmove(void *s1, const char *s2, size_t n);
extern char *strcat(char *dest, const char *src);
extern int isdigit (int c);
extern void *memset(void *s, int c, size_t n);
extern int memcmp(void *s1, const char *s2, size_t n);
extern  void perror(const char *s);
extern int close(int sock);
extern int socket(int domain, int type, int protocol);
extern unsigned long int inet_addr(const char *cp);
extern int fcntl(int fd, int cmd, ...);
extern int atoi(const char *nptr);
extern  int atoi(const char *nptr);
extern char *strtok(char *s, const char *delim);
extern char *strchr (const char *, int);
extern void *malloc(size_t size);
extern void free(void *ptr);
/*extern void sleep (int seconds); */
/* Wrapped Functions */


/*>
    
  void OSInit();
  
  DESCRIPTION: 
  Init OS specific stuff here .
  
<*/

//#define __OS_DEBUG__

void 
OSInit(void)
{
  /* ignore signal SIGPIPE to avoid ending on broken pipe error */
  signal (SIGPIPE, SIG_IGN);
}

/*>
    
  void Exit(int code);
  
  DESCRIPTION: 
  This function terminates the calling process immediately.
  
  ARGS:
  code          process exit status
    
<*/
void 
Exit(int code)
{
  exit(code);
}

/*>
    
  (size_t) strLen = StrLen(const char *str);
  
  DESCRIPTION: 
  This function calculates the length of the string, not including the 
  terminating `\0' character.
  
  ARGS:
  str          string whose length will be calculated

  RETURNS:
  strLen       number of characters in str
    
<*/
size_t
StrLen(const char *str)
{
  return strlen(str);
}

char *
StrNCpy(char *dst, const char* src, size_t len)
{
  return strncpy(dst, src, len);
}

/*>
    
  (int) numChar = FPrintf (FILE *fp, const char *format, ...);
  
  DESCRIPTION: 
  This function writes output to the given output stream
  
  ARGS:
  fp         file pointer
  format     the string format to output to the stream

  RETURNS:
  numChar    number of characters printed
    
<*/
int
FPrintf(FILE *fp, const char *format, ...)
{
  va_list argPtr;
  int status = 0;

  va_start(argPtr, format);
  status = vfprintf(fp, format, argPtr);
  va_end(argPtr);
  return status;
}

/*>
    
  (int) numChar = SPrintf (char *str, const char *format, ...);
  
  DESCRIPTION: 
  This function writes output to the given string
  
  ARGS:
  fp         file pointer
  format     the string format to output to the string

  RETURNS:
  numChar    number of characters printed
    
<*/
int
SPrintf(char *str, const char *format, ...)
{
  va_list argPtr;
  int status = 0;
  
  va_start(argPtr, format);
  status = vsprintf(str, format, argPtr);
  va_end(argPtr);
  return status;
}

/*>
    
  (int) compare =  StrCmp(const char *str1, const char* str2);
  
  DESCRIPTION: 
  This function compares the two strings str1 and str2. It returns an 
  integer less than, equal to, or greater than zero if str1 is found, 
  to be less than, equal, or greater than str2 respectively.
  
  ARGS:
  str1          strings whose values are to be compared
  str2

  RETURNS:
  compare       integer less than, equal to, or greater than zero
  
<*/
int
StrCmp(const char *str1, const char* str2)
{
  return strcmp(str1,str2);
}

/*>
    
  (char *) dstPtr = StrCpy(char *dst, const char* src);
  
  DESCRIPTION: 
  This function copies the string pointed to be src (including the terminating
  `\0' character) to the array pointed to by dest.
  
  ARGS:
  dst           string whose value will be copied to
  src           string whose value will be copied from

  RETURNS:
  dstPtr        pointer to dest string after copy
  
<*/
char *
StrCpy(char *dst, const char* src)
{
  return strcpy(dst, src);
}

#ifdef LIB_IPV6
/*>
    
  (int) compare =  StrNCmp(const char *str1, const char* str2, Size_t len);
  
  DESCRIPTION: 
  This function compares the 1st "len" characters of string str1 with string 
  str2. It returns an integer less than, equal to, or greater than zero if 
  str1 is found, respectively, to be less than, to match, or be greater than 
  str2.
  
  ARGS:
  str1          strings whose values are to be compared
  str2

  RETURNS:
  compare       integer less than, equal to, or greater than zero
  
<*/
int
StrNCmp(const char *str1, const char* str2, size_t len)
{
  return strncmp(str1, str2, len);
}

#endif /* LIB_IPV6 */

/*>
    
  (char *) dstPtr =  StrCat(char *dest, const char *src);
  
  DESCRIPTION: 
  This function appends the src string to the dest string, overwriting the 
  `\0' character at the end of dest, and then adds a terminating `\0' character.
  
  ARGS:
  dest          string whose value will be appended
  src           string whose value will be used to append to src
  
  RETURNS:
  dstPtr        result string after append

<*/
char *
StrCat(char *dest, const char *src)
{
  return strcat(dest, src);
}

char *
StrChr(const char *str, char c)
{
  return strchr(str, c);
}

/*>
    
  (int) isDigit = IsDigit(int c);
  
  DESCRIPTION: 
  This function checks whether c is a digit (0 through 9).

  ARGS:
  c             value to be checked

  RETURNS
  isDigit       nonzero if c is a digit
                otherwise zero
  
<*/
int 
IsDigit(int c)
{ 
  return isdigit(c); 
}

/*>
    
  void *MemSet(void *buff, int value, size_t length);
  
  DESCRIPTION: 
  This function fills the first length bytes of the memory area pointed to
  by buff with the constant value.

  ARGS:
  buff                   buffer to fill
  value                  value to repeat into buffer
  length                 number of bytes to fill in
  
<*/
void *
MemSet(void *buff, int value, size_t length)
{
  return (void *)memset(buff, value, length);
}

/*>
    
  void *MemMove(void *dst, const void *src, size_t length);
  
  DESCRIPTION: 
  The MemMove() function copies length bytes from memory area src
  to memory area dst.  The memory areas may overlap.
       
  ARGS:
  dst           string whose value will be copied to
  src           string whose value will be copied from
  length        number of bytes to fill in

  RETURNS:
  dstPtr        pointer to dest string after copy
  
<*/
void *
MemMove(void *dst, const void *src, size_t length)
{
  return (void *)memmove(dst, src, length);
}

int
MemCmp(const void *data1, const void *data2, size_t length)
{
  return memcmp(data1, data2, length);
}

/*>
    
  void PError(const char *usrDesc);
  
  DESCRIPTION: 
  This function produces a message on the standard error output, describing the
  last error that occured.

  ARGS:
  usrDesc            description prepended to the error message
  
<*/
void 
PError(const char *usrDesc)
{
  perror(usrDesc);  
}

int g_tcpSockNumberNeedToBeClosed = -1;

/*>
    
  (int) closed = OSSockClose(int sock);
  
  DESCRIPTION: 
  This function closes a socket decriptor.

  ARGS:
  sock              the socket descriptor to be closed

  RETURNS
  closed            zero on success, or -1 if an error occurred. 
  
<*/
int 
OSSockClose(int sock)
{

    /* If asp simulator is enabled then collect the reestablsihed socket back before closing the socket.*/
    if(gUseAspSimulator && (g_tcpSockNumberNeedToBeClosed == sock)) 
    {

        int rc=0;
        rc = ASP_ChannelMgr_Stop(ghAspChannelMgr, &sock);
        assert(rc == 0);

        /* TODO: Init and Channel Manager Open should be done only once so later add gFirstTimeFlag based call.*/
        ASP_ChannelMgr_Close( ghAspChannelMgr );

        ASP_Mgr_Uninit();
       
    }
    shutdown(sock, 2);

  if (sock < 0) {
	FPrintf(stderr, "! Invalid socket in OSSockClose\n");
	Exit(1);
  }

  if (close(sock) == 0 ) {
      fprintf(stdout, "\n%s: Clossssssssssssssssssssing ############################# sock=%d\n", __FUNCTION__, sock);
      sleep(1); /* TODO: Added this to avoid the bind issue. Later if bind issue is resolved then this can be removed. */
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockClose(sock == %u) OK\n", sock);
#endif
	return MNTCPAPP_API_RETCODE_OK;    
  }
  else {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockClose(sock == %u) *NOTOK*\n", sock);
#endif
	return MNTCPAPP_API_RETCODE_NOTOK;
  }
}

/*>
    
  (int) bound = OSSockBind(int sock, unsigned long int srcAddr, 
                           unsigned short srcPort);
  
  DESCRIPTION: 
  This function assigns a name to a socket decriptor.

  ARGS:
  sock              the socket descriptor to be bound
  srcAddr           local address 
  srcPort           local port

  RETURNS
  bound             zero on success, or -1 if an error occurred. 
  
<*/
int 
OSSockBind(int sock, unsigned long int srcAddr, unsigned short srcPort)   
{
  int i = 0, status = 0;

  /* We assume family = AF_INET, srcAddr == 0 means s_addr = INADDR_ANY */
  struct sockaddr_in src_name;

  if (sock < 0) {
	FPrintf(stderr, "! Invalid socket in OSSockBind\n");
	Exit(1);
  }

  if (srcAddr) {
	src_name.sin_addr.s_addr = htonl(srcAddr);
  }
  else {
	src_name.sin_addr.s_addr = INADDR_ANY;
  }
  src_name.sin_family = AF_INET;
  src_name.sin_port = htons(srcPort);

  /* Retry TCP_SOCK_BIND_RETRIES_NUM times to bind the TCP socket */
  for (i = 0, status = -1; status == -1 && i < TCP_SOCK_BIND_RETRIES_NUM; 
	   i++) {  
	status = bind(sock, (struct sockaddr *) &src_name, sizeof(src_name)); 

	if (status < 0) {
#ifdef __OS_DEBUG__
	  if (i >= (TCP_SOCK_BIND_RETRIES_NUM / 2)) {
		FPrintf(stdout, "OS_CALL --> OSSockBind: Bind Failed (Attempt #%u of %u) for sock=%d\n",
				i, TCP_SOCK_BIND_RETRIES_NUM, sock);
		FPrintf(stdout, "OS_CALL --> OSSockBind: Sleeping %u secs\n", 
				OS_SOCK_BIND_SLEEP_DURATION);
	  }
#endif
	  OSSleep(OS_SOCK_BIND_SLEEP_DURATION);
	}
  }

  if (status == 0) {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockBind(sock == %u, port == %u) OK\n", 
			sock, srcPort);
#endif
    return MNTCPAPP_API_RETCODE_OK;
  }
  else {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockBind(sock == %u, port == %u) *NOTOK*\n", 
			sock, srcPort);
#endif
    return MNTCPAPP_API_RETCODE_NOTOK;
  } 
}

#ifdef LIB_IPV6
/*>
    
  (int) bound = OSSockBind6(int sock, unsigned char *srcAddr, 
                           unsigned short srcPort);
  
  DESCRIPTION: 
  This function assigns a name to a socket decriptor.

  ARGS:
  sock              the socket descriptor to be bound
  srcAddr           local address 
  srcPort           local port

  RETURNS
  bound             zero on success, or -1 if an error occurred. 
  
<*/
int 
OSSockBind6(int sock, unsigned char *srcAddr, unsigned short srcPort)   
{
  int i = 0, status = 0;

  /* We assume family = AF_INET6, srcAddr == 0 means sin6_addr = in6addr_any*/
  struct sockaddr_in6 src_name6;

  if (sock < 0) {
    FPrintf(stderr, "! Invalid socket in OSSockBind6\n");
    Exit(1);
  }
  
  MemSet((void *)&src_name6, 0, sizeof(src_name6));
  
  if (srcAddr) {
    /* IPv6 address is 16 bytes */
    MemMove((void *)&src_name6.sin6_addr, srcAddr, 16);
  }
  else {
    src_name6.sin6_addr = in6addr_any;
  }
  src_name6.sin6_family = AF_INET6;
  src_name6.sin6_port = htons(srcPort);

  /* Retry TCP_SOCK_BIND_RETRIES_NUM times to bind the TCP socket */
  for (i = 0, status = -1; status == -1 && 
       i < TCP_SOCK_BIND_RETRIES_NUM; i++) {  
    status = bind(sock, (struct sockaddr *) &src_name6, sizeof(src_name6)); 

	if (status < 0) {
#ifdef __OS_DEBUG__
	  if (i >= (TCP_SOCK_BIND_RETRIES_NUM / 2)) {
		FPrintf(stdout, "OS_CALL --> OSSockBind6: Bind Failed (Attempt #%u of %u)\n",
				i, TCP_SOCK_BIND_RETRIES_NUM); 
		FPrintf(stdout, "OS_CALL --> OSSockBind6: Sleeping %u secs\n", 
				OS_SOCK_BIND_SLEEP_DURATION);
	  }
#endif
	  OSSleep(OS_SOCK_BIND_SLEEP_DURATION);
	}
  }
  if (status == 0) {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockBind6(sock == %u) OK\n", sock);
#endif
    return MNTCPAPP_API_RETCODE_OK;
  }
  else {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockBind6(sock == %u) *NOTOK*\n", sock);
#endif
    return MNTCPAPP_API_RETCODE_NOTOK;
  } 
}

#endif /* LIB_IPV6 */

/*>
    
  (int) sockFD = OSSockSocket(unsigned short sType);
  
  DESCRIPTION: 
  This function creates an endpoint for communication and returns 
  a socket decriptor.

  ARGS:
  sType         the type of socket to create   

  RETURNS
  sockFD        the socket descriptor for created endpoint
                or -1 upon error
  
<*/

MNTCPAPP_OS_SOCKET   
OSSockSocket(unsigned short sType)
{
  MNTCPAPP_OS_SOCKET sock;
  
  switch (sType){
  
  case MNTCPAPP_SOCKTYPE_STREAM:
#ifdef LIB_IPV6
    if(gIPVersionForApp == MODE_MNTCPAPP_IPV6) {
      sock = socket(PF_INET6, SOCK_STREAM, 0);
    }
    else {
#endif /* LIB_IPV6 */
      sock = socket(PF_INET, SOCK_STREAM, 0);
#ifdef LIB_IPV6
    }
#endif /* LIB_IPV6 */
    {
        /*TODO: This is added to avoid the bind issue. Later if bind issue is found this can be removed if not required.*/
        int enable = 1;
        if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)))
        {
            FPrintf(stdout, "%s:Not able to set SO_REUSEADDR for SOCK_STREAM sock = %d", __FUNCTION__, sock);
        }
    }
  break;
  
  case MNTCPAPP_SOCKTYPE_DGRAM:
#ifdef LIB_IPV6
    if(gIPVersionForApp == MODE_MNTCPAPP_IPV6) {
      sock = socket(PF_INET6, SOCK_DGRAM, 0);
    }
    else {
#endif  /* LIB_IPV6 */
      sock = socket(PF_INET, SOCK_DGRAM, 0);
#ifdef LIB_IPV6
    }
#endif /* LIB_IPV6 */
  break;
  
  default:
    FPrintf(stdout, "Invalid Socket type requested\n");
    sock  = -1;
    break;
  }

  if (sock < 0) {
    FPrintf(stdout, "! socket descriptor value is less than 0\n"
			"! consider it as error\n");
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockSocket(sType == %u) *NOTOK*\n", sType);
#endif
    return MNTCPAPP_API_RETCODE_NOTOK;
  }
  else {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockSocket(sType == %u) OK\n", sType);
#endif
    return sock;
  }
}


/*>
    
  (int) numBytes = OSSockSendTo(int sock, const void *msg, int msgLen, 
                                MNTCPAppSocketInfo_t dst);
  
  DESCRIPTION: 
  This function is used to transmit a message to another socket.

  ARGS:
  sock             local socket descriptor
  msg              message to be sent
  msgLen           length of message
  dst              information on destination socket

  RETURNS
  
<*/
int 
OSSockSendTo(int sock, const void *msg, int msgLen, MNTCPAppSocketInfo_t dst) 
{ 
  /* Assume flags = 0, family = AF_INET */ 
  struct sockaddr_in servAddr; 
  int                bytesSent = 0;
  const int          tryCnt = 10; /* Number of times to be tried to send the
                                     data before giving up with warning */
  int                i = 0; /* Index variable */
  unsigned char      retval = 0;
  fd_set             wfds;
  struct timeval     tv;
#ifdef LIB_IPV6
  /* Assume flags = 0, family = AF_INET6 */ 
  struct sockaddr_in6 servAddr6; 
#endif /* LIB_IPV6 */

  if (sock < 0) {
	FPrintf(stderr, "! Invalid socket in OSSockSendTo\n");
	Exit(1);
  }
  
#ifdef LIB_IPV6
  if(gIPVersionForApp == MODE_MNTCPAPP_IPV6) {
    MemSet(&servAddr6, 0, sizeof(servAddr6)); 
    servAddr6.sin6_family = AF_INET6; 
    /* IPv6 address is 16 bytes */
    MemMove((void *)&servAddr6.sin6_addr, dst.anvlIPv6Addr, 16);
    servAddr6.sin6_port = htons(dst.anvlPort); 

    /* Create socket state for doing select */
    FD_ZERO(&wfds);
    FD_SET(sock, &wfds);
    tv.tv_sec = 30; /* Wait till 30 seconds */
    tv.tv_usec = 0;

    for (i = 0; (i < tryCnt) && !retval; i++) {
      retval = select((sock + 1), 0, &wfds, 0, &tv);

      if (retval) {
        break;
      }

      /* Sleep for some time before retrying */
      sleep(5);
    }

    if (!retval) {
      FPrintf(stderr, 
          "\n! Even after %u attempts to poll the UDP socket\n"
          "! it was not writeable. This may be a serious OS wait\n"
          "! quesue processing problem. Tests may fail because of this.\n",
          tryCnt);
    }
    else {
      if (dst.delay) OSSleep(dst.delay);
      bytesSent = sendto(sock, msg, msgLen, 0,  
                 (struct sockaddr *)&servAddr6, sizeof(servAddr6));  
    }
  }
  else {
#endif /* LIB_IPV6 */
	MemSet(&servAddr, '\0', sizeof(servAddr)); 
	servAddr.sin_family = AF_INET; 
	servAddr.sin_addr.s_addr = htonl(dst.anvlIPAddr); 
	servAddr.sin_port = htons(dst.anvlPort); 
	
  /* Create socket state for doing select */
  FD_ZERO(&wfds);
  FD_SET(sock, &wfds);
  tv.tv_sec = 30; /* Wait till 30 seconds */
  tv.tv_usec = 0;

  for (i = 0; i < tryCnt; i++) {
#ifdef __OS_DEBUG__
    FPrintf(stdout, "\nOS_CALL ---> select (on sock == %u)\n", sock);
#endif /* __OS_DEBUG__ */
    retval = select((sock + 1), 0, &wfds, 0, &tv);

    if (retval) {
      break;
    }

    /* Sleep for some time before retrying */
    sleep(5);
  }

  if (!retval) {
    FPrintf(stderr, 
        "\n! Even after %u attempts to poll the UDP socket\n"
        "! it was not writeable. This may be a serious OS wait\n"
        "! quesue processing problem. Tests may fail because of this.\n",
        tryCnt);
  }
  else {
    if (dst.delay) OSSleep(dst.delay);
    bytesSent = sendto(sock, msg, msgLen, 0,  
               (struct sockaddr *)&servAddr, sizeof(servAddr));  
  }
#ifdef LIB_IPV6
  }
#endif /* LIB_IPV6 */

  if (bytesSent == 0) {
	FPrintf(stdout, "! 0 bytes sent.\n");
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockSendTo(sock == %u) *NOTOK*\n", sock);
#endif
	return MNTCPAPP_API_RETCODE_NOTOK;
  }
  else if (bytesSent < 0){
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockSendTo(sock == %u) *NOTOK*\n", sock);
#endif
	return MNTCPAPP_API_RETCODE_NOTOK;
  }
  else {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockSendTo: %u bytes sent.\n", bytesSent);
	FPrintf(stdout, "OS_CALL --> OSSockSendTo(sock == %u) OK\n", sock);
#endif
	return MNTCPAPP_API_RETCODE_OK;
  }
} 

/*>
    
  (int) numBytes = OSSockRecvFrom(int sock, void *buff, int bufflen, 
                                  MNTCPAppSocketInfo_t *from);
  
  DESCRIPTION: 
  This function is used to receive incoming data from another socket.

  ARGS:
  sock             local socket descriptor
  buff             buffer to fill with incoming data
  buffLen          length of the buffer
  from             information on source socket sending us data

  RETURNS
  numBytes         number of bytes read or -1 upon error
  
<*/
int 
OSSockRecvFrom(int sock, void *buff, int bufflen, MNTCPAppSocketInfo_t *from) 
{ 
  /* Assume flags = 0 , family = AF_INET */ 
  struct sockaddr_in fromAddr; 
  socklen_t          fromLen = 0;
  int                retCode = 0;
#ifdef LIB_IPV6
  /* Assume flags = 0, family = AF_INET6 */ 
  struct sockaddr_in6 fromAddr6; 
#endif /* LIB_IPV6 */

  if (sock < 0) {
	FPrintf(stderr, "! Invalid socket in OSSockRecvFrom\n");
	Exit(1);
  }
  
#ifdef LIB_IPV6
  if(gIPVersionForApp == MODE_MNTCPAPP_IPV6) {
    if ((from != 0) && (from->anvlPort != 0)) {
      MemSet(&fromAddr6, 0, sizeof(fromAddr6)); 
      fromAddr6.sin6_family = AF_INET6; 
      MemMove((void *)&fromAddr6.sin6_addr, from->anvlIPv6Addr, 16);
      fromAddr6.sin6_port = htons(from->anvlPort);
    }
    fromLen = sizeof(fromAddr6);

    retCode = recvfrom(sock, buff, bufflen, 0, 
					   (struct sockaddr *)&fromAddr6, &fromLen);
    
    if (retCode != -1) {    
      MemMove((void *)(from->anvlIPv6Addr), (void *)&fromAddr6.sin6_addr, 16);
      from->anvlPort = ntohs(fromAddr6.sin6_port);
    }
    else {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockRecvFrom(sock == %u) *NOTOK*\n, sock");
#endif
      retCode = MNTCPAPP_API_RETCODE_NOTOK;
    }
  }
  else {
#endif /* LIB_IPV6 */
  
	if ((from != 0) && (from->anvlIPAddr != 0) && (from->anvlPort != 0)) {
	  MemSet(&fromAddr, '\0', sizeof(fromAddr)); 
	  fromAddr.sin_family = AF_INET; 
	  fromAddr.sin_addr.s_addr = htonl(from->anvlIPAddr); 
	  fromAddr.sin_port = htons(from->anvlPort);
	}
	fromLen = sizeof(fromAddr);

	retCode = recvfrom(sock, buff, bufflen, 0, 
                       (struct sockaddr *)&fromAddr, &fromLen);
    
	if (retCode != -1) {    
	  from->anvlIPAddr = ntohl(fromAddr.sin_addr.s_addr);
	  from->anvlPort = ntohs(fromAddr.sin_port);
	}
	else {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockRecvFrom(sock == %u) *NOTOK*\n, sock");
#endif
	  retCode = MNTCPAPP_API_RETCODE_NOTOK;
	}
#ifdef LIB_IPV6
  }
#endif /* LIB_IPV6 */
  return retCode;
} 

/*>
    
  (int) connected = OSSockConnect(int sock, unsigned long int dstIPAddr, 
                                  unsigned short dstPort);

  DESCRIPTION: 
  This function creates a connection between a local and remote socket.

  ARGS:
  sock             local socket descriptor
  dstIPAddr        IP Address of remote socket
  dstPort          port value of remote socket

  RETURNS
  connected        MNTCPAPP_API_RETCODE_OK             upon success
                   MNTCPAPP_API_RETCODE_NOTOK          upon error
  
<*/
int 
OSSockConnect(int sock, unsigned long int dstIPAddr, unsigned short dstPort)
{
  struct sockaddr_in dst_name;    /* socket name structure used by connect */

  if (sock < 0) {
	FPrintf(stderr, "! Invalid socket in OSSockConnect\n");
	Exit(1);
  }

  /* set connection info */
  dst_name.sin_addr.s_addr = htonl(dstIPAddr);
  dst_name.sin_family = AF_INET;
  dst_name.sin_port = htons(dstPort);
  
  if ( connect(sock, (struct sockaddr *) &dst_name, sizeof(dst_name)) < 0 ) {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockConnect(sock == %u) *NOTOK*\n", sock);
#endif
    return MNTCPAPP_API_RETCODE_NOTOK;
  }
  else {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockConnect(sock == %u) OK\n", sock);
#endif
    return MNTCPAPP_API_RETCODE_OK;
  }
}

#ifdef LIB_IPV6 
/*>
  (int) connected = OSSockConnect6(int sock, unsigned char *dstIPv6Addr, 
                                  unsigned short dstPort);

  DESCRIPTION: 
  This function creates a connection between a local and remote socket.

  ARGS:
  sock             local socket descriptor
  dstIPAddr        IP Address of remote socket
  dstPort          port value of remote socket

  RETURNS
  connected        MNTCPAPP_API_RETCODE_OK             upon success
                   MNTCPAPP_API_RETCODE_NOTOK          upon error
  
<*/
int 
OSSockConnect6(int sock, unsigned char *dstIPv6Addr, unsigned short dstPort)
{
  struct sockaddr_in6 dst_name;    /* socket name structure used by connect */

  if (sock < 0) {
    FPrintf(stderr, "! Invalid socket in OSSockConnect6\n");
    Exit(1);
  }
  MemSet((void *)&dst_name, 0, sizeof(dst_name));
  /* set connection info */
  
  /* IPv6 address is 16 bytes */
  MemMove((void *)&dst_name.sin6_addr, dstIPv6Addr, 16);
  
  dst_name.sin6_family = AF_INET6;
  dst_name.sin6_port = htons(dstPort);
  
  if ( connect(sock, (struct sockaddr *) &dst_name, sizeof(dst_name)) < 0 ) {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockConnect6(sock == %u) *NOTOK*\n", sock);
#endif
    return MNTCPAPP_API_RETCODE_NOTOK;
  }
  else {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockConnect6(sock == %u) OK\n", sock);
#endif
    return MNTCPAPP_API_RETCODE_OK;
  }
}
#endif /* LIB_IPV6 */

/*>
    
  (unsigned long) addr = OSIPAddrStrToLong(char *ipAddr);

  DESCRIPTION: 
  This function converts an IP address to host byte order.

  ARGS:
  ipAddr             IP address to be put in host byte order

  RETURNS
  addr               IP address in host byte order
  
<*/
unsigned long OSIPAddrStrToLong(char *ipAddr)
{
  unsigned long addr = 0;
  if (!ipAddr) {
	FPrintf(stdout, "NULL address given programming error\n");
	Exit(-1);
  }
    
  addr = inet_addr(ipAddr);

  return ntohl(addr); 
}

#ifdef LIB_IPV6 
/*>
    
  (int) addrOK = OSIPv6AddrStrToByteArray(char *ipv6Addr, char *byteArray);

  DESCRIPTION: 

  This function converts the character string(IPv6 address) 
  ipv6Addr into a network address structure in the AF_INET6 family, 
  then copies the network address structure to dst.
  
  This function converts an IPv6 address to host byte order.

  ARGS:
  ipv6Addr         IPv6 address to be put in host byte order
  byteArray        Array of bytes to hold the IPv6 address.

  RETURNS
  addrOK           MNTCPAPP_API_RETCODE_OK             upon success
                   MNTCPAPP_API_RETCODE_NOTOK          upon error
  
<*/
int
OSIPv6AddrStrToByteArray(char *ipv6Addr, char *byteArray)
{
  if (!ipv6Addr) {
  FPrintf(stdout, "NULL address given programming error\n");
  Exit(-1);
  }
    
  if(inet_pton(AF_INET6, ipv6Addr, byteArray) < 1) {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSIPv6AddrStrToByteArray *NOTOK*\n");
#endif
    return MNTCPAPP_API_RETCODE_NOTOK;
  }
  else {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSIPv6AddrStrToByteArray OK\n");
#endif
    return MNTCPAPP_API_RETCODE_OK;
  }
}

#endif /* LIB_IPV6 */
/*>
    
  (int) acceptedSock = int OSSockAccept(int sock, 
                                        MNTCPAppSocketInfo_t *nameInfo);

  DESCRIPTION: 
  This function accepts a connection request that comes in on the
  listening socket and creates a new socket desciptor for the accepted
  request. 

  ARGS:
  sock             socket descriptor of listening socket
  nameInfo         name information for accepted socket

  RETURNS
  acceptedSock     socket descriptor of accepted socket
                   MNTCPAPP_API_RETCODE_NOTOK upon error
  
<*/


MNTCPAPP_OS_SOCKET
OSSockAccept(MNTCPAPP_OS_SOCKET sock, MNTCPAppSocketInfo_t *nameInfo)
{
#ifdef LIB_IPV6
  struct sockaddr_in6 accept_name6;   /* socket name structure used by accept */
#endif /* LIB_IPV6 */
  struct sockaddr_in accept_name;   /* socket name structure used by accept */
  socklen_t accept_len = sizeof(accept_name);
  int ret = 0;
  
  if (sock < 0) {
	FPrintf(stderr, "! Invalid socket in OSSockAccept\n");
	Exit(1);
  }

#ifdef LIB_IPV6
  if(gIPVersionForApp == MODE_MNTCPAPP_IPV6) {
    MemSet((void *)&accept_name6, 0, sizeof(accept_name6));
    accept_len = sizeof(accept_name6);
    ret = accept(sock, (struct sockaddr *) &accept_name6, &accept_len); 
  }
  else {
#endif /* LIB_IPV6 */
  ret = accept(sock, (struct sockaddr *) &accept_name, &accept_len); 
#ifdef LIB_IPV6
  }
#endif /* LIB_IPV6 */
  if (ret) {
#ifdef LIB_IPV6
  if(gIPVersionForApp == MODE_MNTCPAPP_IPV6) {
      MemMove(nameInfo->anvlIPv6Addr, &accept_name6.sin6_addr, 16);

      nameInfo->anvlPort = (unsigned short) ntohs(accept_name6.sin6_port);
    }
    else {
#endif /* LIB_IPV6 */  
  nameInfo->anvlIPAddr = 
    (unsigned long int) ntohl(accept_name.sin_addr.s_addr);

  nameInfo->anvlPort = (unsigned short) ntohs(accept_name.sin_port);
#ifdef LIB_IPV6
  }
#endif /* LIB_IPV6 */
  }
  
  printf("%s: =======================> going to call startAspManager, ret = %d,  gUseAspSimulator = %d \n", __FUNCTION__,ret, gUseAspSimulator);
  if (ret) {

    /* If asp simulator is enabled then hijack the connection to simulator.*/
    if(gUseAspSimulator) 
    {
        int rc;
        int sockFd = ret;
        
        /* later change this to get it thru cmd arg.*/

        /* TODO: Init and Channel Manager Open should be done only once so later add gFirstTimeFlag based call.*/
        rc = ASP_Mgr_Init(gAspProxyServerIp);
        assert(rc ==0);

        /* Open a ASP Channel Mananger. */
        ghAspChannelMgr = ASP_ChannelMgr_Open();
        assert(ghAspChannelMgr);

        /* Offload Connection to a ASP Channel Manager. */
        fprintf(stdout, "%s: Offload Connection: Host --> ASP...\n", __FUNCTION__);
        rc = ASP_ChannelMgr_Start(ghAspChannelMgr, sockFd);
        assert(rc == 0);

        g_tcpSockNumberNeedToBeClosed = sockFd; /* Preserving the socket number which will be closed in the for this tcp connection. */
    }

    return ret;
  }
  else {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockAccept(sock == %u) *NOTOK*\n", sock);
#endif
    return MNTCPAPP_API_RETCODE_NOTOK;
  }

}

/*>
   
  (int) retVal = OSSockFcntl(int sock, unsigned short cmd);

  DESCRIPTION: 
  This function performs the operations on a socket file descriptor.
  The following operations are supported.  In addition some operations
  are supported in combination with others.

  O_NONBLOCK,     ~O_NONBLOCK
  FASYNC,         ~FASYNC
  MSG_OOB

  ARGS:
  sock             local socket file descriptor
  cmd              the operation to perform on file descriptor

  RETURNS
  retVal           MNTCPAPP_API_RETCODE_NOTOK upon error
                   MNTCPAPP_API_RETCODE_OK    on success

<*/
int
OSSockFcntl(int sock, unsigned short cmd)
{
  int flags = 0;
  int ret = 0;

  if (sock < 0) {
	FPrintf(stderr, "! Invalid socket in OSSockFcntl\n");
	Exit(1);
  }

  flags = fcntl(sock, F_GETFL);

  if (flags < 0) {
	FPrintf(stderr, "! Could not retrieve socket flags\n");
  }
  
  switch (cmd){
	
	/* The following cases appends to the existing flag settings */
  case FCNTL_APPEND_NONBLOCKING:
	/* append the non blocking flag to current flags */
	ret = fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    break;

  case FCNTL_APPEND_BLOCKING:
	/* append the blocking flag to current flags */
	flags &= ~O_NONBLOCK;
	ret = fcntl(sock, F_SETFL, flags);
    break;
			
	
  default:
	FPrintf(stderr, "! Invalid Fcntl command\n");
	ret = -1;    /* return -1 if error (invalid cmd) */
  }
  
  if (ret < 0 ) {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockFcntl(sock == %u) *NOTOK*\n", sock);
#endif
    return MNTCPAPP_API_RETCODE_NOTOK;
  }
  else {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockFcntl(sock == %u) OK\n", sock);
#endif
    return MNTCPAPP_API_RETCODE_OK;
  }
}

/*>
    
  (int) success = OSSockListen(int sock, int backlog);

  DESCRIPTION: 
  This function is used to indicate a willingness to accept incoming 
  connections.

  ARGS:
  sock             local socket descriptor
  backlog          limit on queue for incoming connections

  RETURNS
  success          MNTCPAPP_API_RETCODE_OK upon success
                   MNTCPAPP_API_RETCODE_NOTOK upon error
  
<*/
int
OSSockListen(int sock, int backlog)
{
  if (sock < 0) {
	FPrintf(stderr, "! Invalid socket in OSSockListen\n");
	Exit(1);
  }
 
  if (listen(sock, backlog) < 0) {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockListen(sock == %u) *NOTOK*\n", sock);
#endif
	return MNTCPAPP_API_RETCODE_NOTOK;
  }
  else {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockFcntl(sock == %u) OK\n", sock);
#endif
	return MNTCPAPP_API_RETCODE_OK;
  }
}

/*>
  (int) retVal = OSSockSetSockOpt(int sock, SockOpt_t opt, int value1, 
                                  int value2);

  DESCRIPTION:
  This function is used to set the following socket options:

  SOL_SOCKET, SO_LINGER,    (int, int)
  SOL_SOCKET, SO_OOBINLINE  (int)
  SOL_SOCKET, SO_RCVBUF     (int)          done

  IPPROTO_TCP, TCP_NODELAY  (int)          done
  IPPROTO_TCP, TCP_MAXSEG   (int)
  IPPROTO_TCP, TCP_STDURG   (int)

  IPPROTO_IP, IP_TTL        (int)

  ARGS:
  sock             local socket descriptor
  opt              socket option to be set
  value1           value to set the option to
  value2           used for SO_LINGER 

  RETURNS:
  retVal          MNTCPAPP_API_RETCODE_OK    upon success
                  MNTCPAPP_API_RETCODE_NOTOK upon failure
<*/
int 
OSSockSetSockOpt(int sock, SockOpt_t opt, int value1, int value2)
{
  struct linger ling;
  int           val1 = 0;
  socklen_t     optlen = 0;
  int           recvbuff = 0;
  int           ret = 0;
  int           newRecvBuff = 0;
  int           sendbuff = 0;
  int           newSendBuff = 0;

  if (sock < 0) {
	FPrintf(stderr, "! Invalid socket in OSSockSetSockOpt\n");
	Exit(1);
  }

  val1 = value1;
  newRecvBuff = value1;
  newSendBuff = value1;
  
  switch (opt) {
	  
  case SOCKOPT_SET_LINGER_TIME:
	ling.l_onoff = value1;
	ling.l_linger = value2;
	ret = setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *) &ling, 
					 sizeof(ling));
	break;

  case SOCKOPT_SET_OOB_DATA_INLINE:
	ret = setsockopt(sock, SOL_SOCKET, SO_OOBINLINE, &val1, sizeof(val1));
	break;

  case SOCKOPT_SET_RECV_BUF_SIZE:
	/* get old buffer size */
	optlen = sizeof(recvbuff);
	ret = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *) &recvbuff, 
					 &optlen);
	if (ret	< 0) {
	  FPrintf(stdout, "! Could not get current buffer size\n");
	  PError("!OSSockSetSockOpt : reason");
	  return MNTCPAPP_API_RETCODE_NOTOK; 
	}

	FPrintf(stdout, "recv buffer size was %d, "
			"changing to %d\n", recvbuff, newRecvBuff);
	/* set requested buffer size */
	ret = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *) &newRecvBuff,
					 sizeof(newRecvBuff));

    break;


  case SOCKOPT_SET_SEND_BUF_SIZE:
	/* get old buffer size */
	optlen = sizeof(sendbuff);
	ret = getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *) &sendbuff, 
					 &optlen);
	if (ret	< 0) {
	  FPrintf(stdout, "! Could not get current buffer size\n");
	  PError("!OSSockSetSockOpt : reason");
	  return MNTCPAPP_API_RETCODE_NOTOK;  
	}
	
	FPrintf(stdout, "send buffer size was %d, changing to %d\n", sendbuff, 
			newSendBuff);

	/* set requested buffer size */
	ret = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *) &newSendBuff,
					 sizeof(newSendBuff));
    break;


  case SOCKOPT_SET_NAGGLE_ALGO:
	ret = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *) &val1, 
					 sizeof(val1));
    break;

  case SOCKOPT_SET_DFBIT:
    if (0 < val1) {
      val1 = IP_PMTUDISC_DO;
    }
    else {
      val1 = IP_PMTUDISC_DONT;
    }
    ret = setsockopt(sock, SOL_IP, IP_MTU_DISCOVER, (char *) &val1, 
             sizeof(val1));
    break;

  case SOCKOPT_SET_MSS:
	ret = setsockopt(sock, IPPROTO_TCP, TCP_MAXSEG, (char *) &val1, 
					 sizeof(val1));
    break;

#ifdef __TCP_STDURG__
  case SOCKOPT_SET_STDURG:
	ret = setsockopt(sock, IPPROTO_TCP, TCP_STDURG, (char *) &val1, 
					 sizeof(val1));
    break;
#endif

  case SOCKOPT_SET_NOPUSH:
	/* Linux implements equivalent functionality using TCP_CORK */
	ret = setsockopt(sock, IPPROTO_TCP, TCP_CORK, (char *) &val1, 
					 sizeof(val1));
    break;

  case SOCKOPT_SET_TTL:
	ret = setsockopt(sock, IPPROTO_IP, IP_TTL, (char *) &val1, 
					 sizeof(val1));
    break;
	
#ifdef LIB_IPV6
  case SOCKOPT_SET_HOPLIMIT:
	/* Linux uses IPV6_UNICAST_HOPS, IPV6_HOPLIMIT option is for DGRAMs */
    ret = setsockopt(sock, IPPROTO_IPV6, IPV6_UNICAST_HOPS, (char *) &val1, 
           sizeof(val1));
    break;
#endif /* LIB_IPV6 */

  case SOCKOPT_SET_MD5:
	FPrintf(stdout, "MD5 SetSockOpt option not currenly supported under "
			"Linux\n");
	return MNTCPAPP_API_RETCODE_NOTOK;
	break;

  default:
	FPrintf(stdout, "Invalid SetSockOpt option\n");
	return MNTCPAPP_API_RETCODE_NOTOK;
    break;
  }

  if (ret == 0) {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockSetSockOpt(sock == %u) OK\n", sock);
#endif
    return MNTCPAPP_API_RETCODE_OK;
  }
  else {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockSetSockOpt(sock == %u) *NOTOK*\n", sock);
#endif
    return MNTCPAPP_API_RETCODE_NOTOK;
  }

}

/*>
    
  (int) numBytes = OSSockSend(int sock, char *buff, unsigned int nBytes,
                              boolean urgent);

  DESCRIPTION: 
  This function transmits a message to another socket.

  ARGS:
  sock              local socket descriptor
  buff              buffer containing message to be sent
  nBytes            length of message
  urgent            whether or not message should be sent as 
                    out of band data
  RETURNS
  numBytes          number of characters sent( > 0), otherwise
                    MNTCPAPP_API_RETCODE_NOTOK upon error
  
<*/
int   
OSSockSend(int sock, char *buff, unsigned int nBytes, boolean urgent)
{
  int val = 0;
 
  if (sock < 0) {
	FPrintf(stderr, "! Invalid socket in OSSockSend\n");
	Exit(1);
  }
 
  if(gUseAspSimulator) 
  {
      int rc;
      /* FPrintf(stdout, "payload data before feeding simulator is |%s|",buff); */
      rc = ASP_ChannelMgr_SendPayload(ghAspChannelMgr, buff, nBytes);
      assert(rc == 0);

      val = nBytes; /* If ASP_ChannelMgr_SendPayload executes successfuly then we assume that nBytesa are transmitted successfuly.*/
  }
  else
  {
      if (urgent) {
        val = send(sock, buff, nBytes, MSG_OOB);
      }
      else {
        val = send(sock, buff, nBytes, 0);
      }
  }
  
  if (val <= 0) {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockSend(sock == %u) *NOTOK*\n", sock);
#endif
	return MNTCPAPP_API_RETCODE_NOTOK;
  }
  else {
	return val;
  }
}


/*>
    
  (int) numBytes = int OSSockRecv(int sock, char *buff, unsigned int nbytes);

  DESCRIPTION: 
  This function receives a message from a connected socket.

  ARGS:
  sock              local socket descriptor
  buff              buffer where message data should be filled in
  nBytes            length of buffer

  RETURNS
  numBytes          number of characters read, otherwise
                   MNTCPAPP_API_RETCODE_NOTOK upon error
  
<*/
int   
OSSockRecv(int sock, char *buff, unsigned int nBytes)
{
  int val = 0;

  if (sock < 0) {
	FPrintf(stderr, "! Invalid socket in OSSockRecv\n");
	Exit(1);
  }
    
  val = recv(sock, buff, nBytes, 0);

  if (val < 0) {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockRecv(sock == %u) *NOTOK*\n", sock);
#endif
	return MNTCPAPP_API_RETCODE_NOTOK;
  }
  else {
	return val;
  }
}

/*>
  (int) retVal = OSSockGetSockOpt(int sock, SockOpt_t opt, int *value);

  DESCRIPTION:
  This function is used to retrieve the following socket options:

  SOL_SOCKET, SO_ERROR
  SOL_SOCKET, SO_OOBINLINE
  SOL_SOCKET, SO_RCVBUF

  IPPROTO_TCP, TCP_STDURG

  ARGS:
  sock             local socket descriptor
  opt              socket option whose value will be retrieved
  value            location to store returned value

  RETURNS:
  retVal           MNTCPAPP_API_RETCODE_OK     upon success
                   MNTCPAPP_API_RETCODE_NOTOK  upon error

<*/
int 
OSSockGetSockOpt(int sock, SockOpt_t opt, int *value)
{
  socklen_t valLen = sizeof(int);
  int ret = 0;

  if (sock < 0) {
	FPrintf(stderr, "! Invalid socket in OSSockGetSockOpt\n");
	Exit(1);
  }

  switch (opt) {

  case SOCKOPT_GET_ERROR:
	ret = getsockopt(sock, SOL_SOCKET, SO_ERROR, value, &valLen);
	break;
  
  case SOCKOPT_GET_RECV_BUF:
	ret = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, value, &valLen);
	break;
  
  case SOCKOPT_GET_OOB_DATA_INLINE: 
	ret = getsockopt(sock, SOL_SOCKET, SO_OOBINLINE, value, &valLen);
	break;
  
#ifdef __TCP_STDURG__
  case SOCKOPT_GET_STDURG:
	ret = getsockopt(sock, IPPROTO_TCP, TCP_STDURG, value, &valLen);
#endif

  case SOCKOPT_GET_MD5:
	FPrintf(stdout, "MD5 GetSockOpt option is not currently supported under "
            "Linux\n");
	ret = -1;
	break;

  default:
	FPrintf(stdout, "Invalid SetSockOpt option\n");
	ret = -1;        /* return -1 if error (invalid opt) */
  }

  if (ret < 0) {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockGetSockOpt(sock == %u) *NOTOK*\n", sock);
#endif
    return MNTCPAPP_API_RETCODE_NOTOK;
  }
  else {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockGetSockOpt(sock == %u) *NOTOK*\n", sock);
#endif
    return MNTCPAPP_API_RETCODE_OK;
  }

}

/*>
    
  (int) numBytes = int OSSockShutDn(int sock, int type);

  DESCRIPTION: 
  Causes all or part of a full-duplex connection on the socket 
  to be shut down.

  ARGS:
  sock              local socket descriptor
  type              type of shutdown to perform

  RETURNS
  ret              MNTCPAPP_API_RETCODE_OK     upon success
                   MNTCPAPP_API_RETCODE_NOTOK  upon error
  
<*/
int
OSSockShutDn(int sock, int type)
{
  int how = 0;
  int ret = 0;

  if (sock < 0) {
	FPrintf(stderr, "! Invalid socket in OSSockShutDn\n");
	Exit(1);
  }

  ret = MNTCPAPP_API_RETCODE_OK;
  
  /*
	The shutdown call causes all or part of a full-duplex con-
	nection on the socket associated with s to be  shut  down.
	If  how is 0, further receives will be disallowed.  If how
	is 1, further sends will be disallowed.  If how is 2, fur-
	ther sends and receives will be disallowed.
  */
  
  
  switch (type) {
	
  case  SHUT_READ:
	how = 0;
	break;
	
  case SHUT_WRITE:
	how = 1;
	break;
	
  case SHUT_RD_WT:
	how = 2;
	break;
  }
  
  /*
	On  success,  zero is returned.  On error, -1 is returned,
	and errno is set appropriately.
  */
  if (shutdown(sock, how) < 0 ) {
#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSSockShutDn(sock == %u) *NOTOK*\n", sock);
#endif
	ret = MNTCPAPP_API_RETCODE_NOTOK;
  }
  
  return ret;
}

/*>
  (int) errorNum = ErrNo(void);

  DESCRIPTION:
  This function is used to retrieve the error code from the last error.

  RETURNS:
  errorNum         error code number

<*/
int 
ErrNo(void)
{
  return errno;
}

/*>

  (boolean) inProgress = ConnInProgressCheck(void);

  DESCRIPTION:
  This function determines whether or not the connection is currently 
  in progress or not.

  RETURNS:
  inProgress          TRUE if connection is in progress
                      FALSE otherwise
<*/
boolean 
ConnInProgressCheck(void)
{
  return (errno == EINPROGRESS) ? TRUE : FALSE;
}

/*>

  (boolean) value = ASCIIToInt(const char *ascii);

  DESCRIPTION:
  This function converts the string pointed to by ascii to an integer.

  ARGS:
  ascii        the ASCII character to convert to integer

  RETURNS:
  value        the converted value
  
<*/
int 
ASCIIToInt(const char *ascii)
{
  return atoi(ascii);
}

/*>

  (char *) token = StrTok(char *str, const char *delim);

  DESCRIPTION:
  This function creates a nonempty string of characters up to, but not
  including a delimiter string.  This function is used to parse str
  into these nonempty strings called tokens.

  ARGS:
  str          string to be parsed
  delim        delimiter string

  RETURNS:
  token        pointer to next token
               NULL if no more tokens exist
  
<*/
char *
StrTok(char *str, const char *delim)
{
  return strtok(str, delim);
}

/*>

  (void *) ptr = Malloc(unsigned int size);

  DESCRIPTION:
  This function returns a pointer to memory allocated by the OS.

  ARGS:
  size         size of memory block to be returned

  RETURNS:
  ptr          pointer to newly allocated memory

<*/
void *
Malloc(unsigned int size)
{
  return malloc(size);
}

/*>

  (void) Free(void *ptr);

  DESCRIPTION:
  This function returns a pointer to memory allocated by the OS.

  ARGS:
  ptr          pointer to memory to be freed

  RETURNS:
  NONE

<*/
void
Free(void *ptr)
{
  free(ptr);
}

/*>

  void OSSleep(int delay);

  DESCRIPTION:
  Tells the OS dependent layer to sleep for the specified
  number of seconds.

  ARGS:
  delay     number of seconds to delay

  RETURNS:

<*/
void 
OSSleep(int delay)
{
  sleep(delay);  /* delay is in seconds */
}

/*>

  int rfcCodeBitMask = OSErrorCodeToRFCError(int errorCode);

  DESCRIPTION:
  This function translates the error codes, returned by calls to OS layer depenent  
  socket functions, into a bitmask of generic code values representing the generic 
  error messages expected by RFC 793.  Note that some OS error codes may map to more 
  than one of the generic error messages in RFC 793 depending upon the context in 
  which the error code was received.

  ARGS:
  errorCode      -  OS layer dependent socket function error return value

  RETURNS:
  rfcCodeBitMask -  bitmask of error codes for generic RFC 793 error messages

<*/
int 
OSErrorCodeToRFCError(int errorCode)
{
  TCPErrorCode_t rfcCodeBitMask = TCP_ERROR_UNKNOWN;

#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSErrorCodeToRFCError(errorCode == %d)\n", 
			errorCode);
#endif

  switch (errorCode) {
  case 0:
	break;

  case EADDRINUSE:
	rfcCodeBitMask = TCP_ERROR_CONN_ILLEGAL_FOR_PROCESS;
	break;

  case EALREADY:
  case EISCONN:
	rfcCodeBitMask = TCP_ERROR_CONN_ALREADY_EXISTS;
	break;

  case EPIPE:
	rfcCodeBitMask = (TCP_ERROR_CONN_CLOSING | TCP_ERROR_CONN_DOES_NOT_EXIST |
					  TCP_ERROR_CONN_FOREIGN_UNSPECIFIED);
	break;

  case ENXIO:
	rfcCodeBitMask = (TCP_ERROR_CONN_CLOSING | TCP_ERROR_CONN_RESET);
	break;

  case ENOTCONN:
	rfcCodeBitMask = (TCP_ERROR_CONN_DOES_NOT_EXIST | TCP_ERROR_CONN_CLOSING);
	break;

  case ECONNREFUSED:
	rfcCodeBitMask = (TCP_ERROR_CONN_REFUSED | TCP_ERROR_CONN_FOREIGN_UNSPECIFIED);
	break;

  case ECONNRESET:
	rfcCodeBitMask = (TCP_ERROR_CONN_RESET | TCP_ERROR_CONN_REFUSED);
	break;

  case MNTCPAPP_API_RETCODE_NOTOK:
	rfcCodeBitMask = TCP_ERROR_CONN_DOES_NOT_EXIST;	
	break;

  default:
	/* use TCP_ERROR_ANY only if backwards compatibility is an issue */
	break;
  }

#ifdef __OS_DEBUG__
	FPrintf(stdout, "OS_CALL --> OSErrorCodeToRFCError: rfcCodeBitMask == 0x%02x\n\n", 
			(int)rfcCodeBitMask);
#endif
	
  return (int)rfcCodeBitMask;
}
