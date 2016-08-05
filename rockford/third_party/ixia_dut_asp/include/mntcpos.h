/*
  mntcpos.h: TCP Stub Server Application OS function include file.

  Copyright (c) Ixia 2002-2009
  All rights reserved.

*/

/* This file contains header for OS-specific functions called from
   mntcpapp.c  The function bodies are in a different file for each
   OS implemented.  (For example mntcplnx.c for Linux) */


#ifndef __MNTCPOS_H__
#define __MNTCPOS_H__

#include <stdio.h>
#include "stub.h"

#ifndef __MNTCPAPP_WINDOWS__
#define __MNTCPAPP_LINUX__
#endif /* __MNTCPAPP_WINDOWS__ */


#ifdef __MNTCPAPP_LINUX__
    #define MNTCPAPP_OS_SOCKET      int
    #define FALSE                      0
    #define TRUE                       !(FALSE)
    typedef int boolean;
#endif

/* Dclare socket for Win NT */
#ifdef __MNTCPAPP_WINDOWS__
    #include <winsock2.h>
    #define MNTCPAPP_OS_SOCKET      SOCKET
#endif

#define MNTCPAPP_SOCKTYPE_STREAM   1       /* type for Socket */
#define MNTCPAPP_SOCKTYPE_DGRAM    2       /* type for Socket */

#define LINGER_OFF                 0      /* linger on close if data present */
#define LINGER_ON                  1

#define OS_SOCK_BIND_SLEEP_DURATION 5

struct MNTCPAppSocketInfo_s {
  /* all are in host byte order */
  unsigned long int anvlIPAddr;
  unsigned short anvlPort;
  unsigned long int localIPAddr;
  unsigned short localPort;
  unsigned char anvlIPv6Addr[16]; /* ipv6 address is 128 bit long */
  unsigned char localIPv6Addr[16];

  int connSock;
  int delay; /* delay before writing on socket */
};
typedef struct MNTCPAppSocketInfo_s MNTCPAppSocketInfo_t;

enum FCntl_e {
  FCNTL_ILLEGAL = 0,

  /* these commands append to the existing flags */
  FCNTL_APPEND_NONBLOCKING,
  FCNTL_APPEND_BLOCKING
};
typedef enum FCntl_e FCntl_t;

enum SockOpt_e {
  SOCKOPT_ILLEGAL = 0,

  /* The following are of type SOL_SOCKET */
  SOCKOPT_SET_LINGER_TIME,
  SOCKOPT_SET_OOB_DATA_INLINE,
  SOCKOPT_SET_RECV_BUF_SIZE,
  SOCKOPT_SET_SEND_BUF_SIZE,

  /* The followings are of type IPPROTO_TCP */
  SOCKOPT_SET_NAGGLE_ALGO,
  SOCKOPT_SET_MSS,
  SOCKOPT_SET_DFBIT,
  SOCKOPT_SET_STDURG,
  SOCKOPT_SET_NOPUSH,

  /* The followings are of type IPPROTO_IP */
  SOCKOPT_SET_TTL,
#ifdef LIB_IPV6
  SOCKOPT_SET_HOPLIMIT,
#endif /* LIB_IPV6 */
  SOCKOPT_GET_ERROR,
  SOCKOPT_GET_RECV_BUF,
  SOCKOPT_GET_STDURG,
  SOCKOPT_GET_OOB_DATA_INLINE,

  SOCKOPT_SET_MD5,
  SOCKOPT_GET_MD5
};
typedef enum SockOpt_e SockOpt_t;

/* Declarations for wrapped socket call functions */
extern void OSInit(void);
extern int OSSockClose(MNTCPAPP_OS_SOCKET sock);
extern int OSSockBind(MNTCPAPP_OS_SOCKET sock, unsigned long int srcAddr,
                      unsigned short srcPort);
#ifdef LIB_IPV6
extern int OSSockBind6(MNTCPAPP_OS_SOCKET sock, unsigned char *srcAddr,
                       unsigned short srcPort);
#endif /* LIB_IPV6 */

extern MNTCPAPP_OS_SOCKET OSSockSocket(unsigned short sType);
extern int OSSockSendTo(MNTCPAPP_OS_SOCKET sock, const void *msg, int magLen,
                        MNTCPAppSocketInfo_t dst);
extern int OSSockRecvFrom(MNTCPAPP_OS_SOCKET sock, void *buff, int bufflen,
                          MNTCPAppSocketInfo_t *from);
extern int OSSockConnect(MNTCPAPP_OS_SOCKET sock, unsigned long int dstIPAddr,
                         unsigned short dstPort);
#ifdef LIB_IPV6
extern int OSSockConnect6(MNTCPAPP_OS_SOCKET sock, unsigned char *dstIPv6Addr,
                          unsigned short dstPort);
#endif /* LIB_IPV6 */
extern MNTCPAPP_OS_SOCKET OSSockAccept(MNTCPAPP_OS_SOCKET sock,
                                         MNTCPAppSocketInfo_t *nameInfo);
extern int OSSockFcntl(MNTCPAPP_OS_SOCKET sock, unsigned short cmd);
extern int OSSockListen(MNTCPAPP_OS_SOCKET sock, int backlog);
extern int OSSockSend(MNTCPAPP_OS_SOCKET sock, char *buff, unsigned int nbytes,
                      boolean urgent);
extern int OSSockRecv(MNTCPAPP_OS_SOCKET sock, char *buff, unsigned int nbytes);
extern int OSSockSetSockOpt(MNTCPAPP_OS_SOCKET sock, SockOpt_t opt, int value1, int value2);
extern int OSSockGetSockOpt(MNTCPAPP_OS_SOCKET sock, SockOpt_t opt, int *value);
extern int OSSockShutDn(int sock, int type);

/* Declarations for wrapped string functions */
extern size_t StrLen(const char *str);
extern int StrCmp(const char *str1, const char *str2);
extern char *StrCpy(char *dst, const char *src);
extern char *StrNCpy(char *dst, const char* src, size_t len);
extern char *StrCat(char *dest, const char *src);
extern char *StrTok(char *str, const char *delim);
extern char *StrChr(const char *str, char c);
#ifdef LIB_IPV6
extern int StrNCmp(const char *str1, const char *str2, size_t len);
#endif /* LIB_IPV6*/

/* Declarations for wrapped utility functions */
extern void Exit(int code);
extern int FPrintf(FILE *fp, const char *format, ...);
extern int SPrintf(char *str, const char *format, ...);
extern int IsDigit(int c);
extern void *MemSet(void *buff, int value, size_t length);
extern void *MemMove(void *dst, const void *src, size_t length);
extern int  MemCmp(const void *dst, const void *src, size_t length);
extern void PError(const char *usrDesc);
extern unsigned long OSIPAddrStrToLong(char *ipAddr);
extern int ErrNo(void);
extern boolean ConnInProgressCheck(void);
extern int ASCIIToInt(const char *ascii);
extern void *Malloc(unsigned int size);
extern void Free(void *ptr);
extern void OSSleep(int delay);
extern int OSErrorCodeToRFCError(int errCode);
extern void NetworkByteOrder(unsigned char *data, unsigned int count);

#ifdef LIB_IPV6
extern int OSIPv6AddrStrToByteArray(char *ipv6Addr, char *dst);
#endif  /* LIB_IPV6 */

#endif /*! __MNTCPOS_H__ */
