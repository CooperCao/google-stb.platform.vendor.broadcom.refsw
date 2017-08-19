/*
 * RServ Simulation Operating System Abstraction
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

#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <errno.h>
#include <assert.h>

#define ASSERT assert

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef int int32;
typedef short int16;
typedef char int8;

typedef sem_t os_sem_t;

#define os_sem_init(sem_ptr, count)	sem_init((sem_ptr), 0, (count))
#define os_sem_give(sem_ptr)		sem_post(sem_ptr)

extern int os_sem_take(os_sem_t *sem, struct timeval *tmout);

#define tv_cmp(a, b)					\
	(((a)->tv_sec < (b)->tv_sec) ? -1 :		\
	 ((a)->tv_sec > (b)->tv_sec) ? 1 :		\
	 ((a)->tv_usec < (b)->tv_usec) ? -1 :		\
	 ((a)->tv_usec > (b)->tv_usec) ? 1 : 0)

#define tv_sub(dst, src) {				\
	(dst)->tv_sec -= (src)->tv_sec;			\
	(dst)->tv_usec -= (src)->tv_usec;		\
	if ((dst)->tv_usec < 0) {			\
	    (dst)->tv_sec--;				\
	    (dst)->tv_usec += 1000000;			\
	}						\
}

#define tv_add(dst, src) {				\
	(dst)->tv_sec += (src)->tv_sec;			\
	(dst)->tv_usec += (src)->tv_usec;		\
	if ((dst)->tv_usec >= 1000000) {		\
	    (dst)->tv_sec++;				\
	    (dst)->tv_usec -= 1000000;			\
	}						\
}

#define tv_copy(dst, src) {				\
	(dst)->tv_sec = (src)->tv_sec;			\
	(dst)->tv_usec = (src)->tv_usec;		\
}

#endif /* _OS_H */
