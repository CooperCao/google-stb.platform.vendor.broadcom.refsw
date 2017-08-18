/*
 * sys_xx.h - some portable system services (threads, locks, events)
 *
 * Copyright 1999, Broadcom Corporation.
 * $Id$
 */

#ifndef _sys_xx_h_
#define	_sys_xx_h_

extern void sys_init(void);
extern void* createthread(void *func, void *arg, int initial_irql);
extern int getcurrentthreadid(void);
extern void exitthread(uintptr code);
extern void msleep(int milliseconds);
#if (__GLIBC__ < 2) || (__GLIBC_MINOR__ < 2)
extern void usleep(unsigned int useconds);
#endif
extern void *alloclock(void);
extern void lock(void *lock);
extern void unlock(void *lock);
extern void freelock(void *lock);
extern long allocevent(int manualreset, int initialstate);
extern void freeevent(long e);
extern int setevent(long e);
extern int resetevent(long e);
extern void waitforevent(long e);
extern int wfetimeout(long e, int milliseconds);
extern void irql_add(int id, int initial_irql);
extern void irql_rem(int id);
extern void irql_set(int irql);
extern int irql_get(void);
extern int getticks(void);

#endif	/* _sys_xx_h_ */
