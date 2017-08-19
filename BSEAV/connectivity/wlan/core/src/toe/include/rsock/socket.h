/*
 * Remote Sockets client library definitions
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#ifndef __RSOCK_H__
#define __RSOCK_H__

#include <rsock/endian.h>
#include <rsock/types.h>

/* Customize byte order macros for target platform */

#ifndef htonl
#if RSOCK_LE
#define htonl(x)	rsock_swap32(x)
#define ntohl(x)	rsock_swap32(x)
#define htons(x)	rsock_swap16(x)
#define ntohs(x)	rsock_swap16(x)
#else
#define htonl(x)	(x)
#define ntohl(x)	(x)
#define htons(x)	(x)
#define ntohs(x)	(x)
#endif
#endif /* htonl */

int rsock_accept(int s, struct sockaddr *addr, socklen_t *addrlen);
int rsock_bind(int s, struct sockaddr *name, socklen_t namelen);
int rsock_shutdown(int s, int how);
int rsock_getpeername(int s, struct sockaddr *name, socklen_t *namelen);
int rsock_getsockname(int s, struct sockaddr *name, socklen_t *namelen);
int rsock_getsockopt(int s, int level, int optname,
                     void *optval, socklen_t *optlen);
int rsock_setsockopt(int s, int level, int optname,
                     const void *optval, socklen_t optlen);
int rsock_close(int s);
int rsock_connect(int s, struct sockaddr *name, socklen_t namelen);
int rsock_listen(int s, int backlog);
int rsock_recv(int s, void *mem, int len, unsigned int flags);
int rsock_read(int s, void *mem, int len);
int rsock_recvfrom(int s, void *mem, int len, unsigned int flags,
                   struct sockaddr *from, socklen_t *fromlen);
int rsock_send(int s, void *dataptr, int size, unsigned int flags);
int rsock_sendto(int s, void *dataptr, int size, unsigned int flags,
                 struct sockaddr *to, socklen_t tolen);
int rsock_socket(int domain, int type, int protocol);
int rsock_write(int s, void *dataptr, int size);
int rsock_ioctl(int s, long cmd, void *argp);
int rsock_fcntl(int s, int cmd, ...);

#ifdef RSOCK_BSD_COMPAT
#define accept		rsock_accept
#define bind		rsock_bind
#define shutdown	rsock_shutdown
#define getpeername	rsock_getpeername
#define getsockname	rsock_getsockname
#define getsockopt	rsock_getsockopt
#define setsockopt	rsock_setsockopt
#define close		rsock_close
#define connect		rsock_connect
#define listen		rsock_listen
#define recv		rsock_recv
#define read		rsock_read
#define recvfrom	rsock_recvfrom
#define send		rsock_send
#define sendto		rsock_sendto
#define socket		rsock_socket
#define write		rsock_write
#define ioctl		rsock_ioctl
#define fcntl		rsock_fcntl
#endif /* RSOCK_BSD_COMPAT */

#endif /* __RSOCK_H__ */
