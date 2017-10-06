/*
 * RSock User Mode Operating System Abstraction
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

#ifndef _OS_H
#define _OS_H

#include <stdio.h>			/* For debug printf */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <assert.h>

#define ASSERT assert

typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef int int32;
typedef short int16;
typedef char int8;

typedef uint32 os_msec_t;

#define OS_DEBUG			if (0) printf
#define OS_ERROR			printf

#define os_errno			errno
#define os_errno_set(errno)		(os_errno = (errno))

#define OS_MSEC_FOREVER			((os_msec_t)0xffffffff)

typedef sem_t os_sem_t;

#define os_printf			printf
#define os_sprintf			sprintf

extern int os_printerr(const char *fmt, ...)
	__attribute__ ((__format__ (__printf__, 1, 2)));

#define os_sem_init(sem_ptr, count)	sem_init((sem_ptr), 0, (count))
#define os_sem_give(sem_ptr)		sem_post(sem_ptr)

#define os_memcpy(dst, src, len)	memcpy((dst), (src), (len))
#define os_memset(dst, val, len)	memset((dst), (val), (len))

#define os_strlen(s)			strlen(s)
#define os_strcmp(s1, s2)		strcmp((s1), (s2))
#define os_strncpy(dst, src, maxlen)	strncpy((dst), (src), (maxlen))

#define os_strerror(eno)		strerror(eno)

#define os_malloc(size)			malloc(size)
#define os_free(ptr)			free(ptr)

extern int os_msleep(os_msec_t dur);
extern os_msec_t os_msec(void);
extern int os_sem_take(os_sem_t *sem, os_msec_t tmout, int sig);

typedef struct os_pkt_opaque os_pkt_t;

#define os_pkt_alloc(len)		user_pkt_alloc(len)
#define os_pkt_free(pkt)		user_pkt_free(pkt)
#define os_pkt_data(pkt)		user_pkt_data(pkt)
#define os_pkt_len(pkt)			user_pkt_len(pkt)
#define os_pkt_output(pkt)		user_pkt_output(pkt)

extern os_pkt_t *user_pkt_alloc(int len);
extern void user_pkt_free(os_pkt_t *pkt);
extern void *user_pkt_data(os_pkt_t *pkt);
extern int user_pkt_len(os_pkt_t *pkt);
extern int user_pkt_output(os_pkt_t *pkt);

#endif /* _OS_H */
