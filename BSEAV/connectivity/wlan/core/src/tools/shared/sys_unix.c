/*
 * UNIX/pthreads-specific system routines (threads, locks, events).
 * 
 * $Id$
 */

#define	DBG	1

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <typedefs.h>
#include <utils.h>
#include <sys_xx.h>

struct event {
	pthread_mutex_t	lock;
	pthread_cond_t cond;
	int wake;
};

void
sys_init(void)
{
}

/* ARGSUSED2 */
void *
createthread(void *func, void *arg, int initial_irql)
{
#ifndef BCMSIM_STDALONE
	pthread_t t;

	pthread_create(&t, NULL, func, arg);
	ASSERT(t);

	return ((void*)t);
#else
	return NULL;
#endif
}

int
getcurrentthreadid()
{
#ifndef BCMSIM_STDALONE
	return ((int)pthread_self());
#else
	return 0;
#endif
}

void
exitthread(uintptr code)
{
#ifndef BCMSIM_STDALONE
	pthread_exit((void*) code);
#else
	return;
#endif
}

void
msleep(int milliseconds)
{
	usleep(milliseconds * 1000);
}

/*
 * pthread mutexes don't support recursion, don't have ref counts,
 * and don't check for recursion.  We add a ref count in order to
 * assert on attempts to recurse.
 */
struct ulock {
	int ref;
	pthread_mutex_t mutex;
};

void *
alloclock()
{
	struct ulock *ulock;

	/*
	 * pthread mutexes don't support recursion nor do they check for it
	 * so we add an extra int of bookkeeping info to be able
	 * to assert on recursion attempts.
	 */
	ulock = malloc(sizeof (struct ulock));
	ASSERT(ulock);
	bzero(ulock, sizeof (struct ulock));

#ifndef BCMSIM_STDALONE
	pthread_mutex_init(&ulock->mutex, NULL);
#endif

	return ((void *) ulock);
}

void
freelock(void *lock)
{
	struct ulock *ulock;

	ulock = (struct ulock*) lock;

	ASSERT(ulock);
	ASSERT(ulock->ref == 0);

#ifndef BCMSIM_STDALONE
	pthread_mutex_destroy(&ulock->mutex);
#endif

	free((char*) ulock);
}

void
lock(void *lock)
{
	struct ulock *ulock = (struct ulock*) lock;

	ASSERT(ulock);
#ifndef BCMSIM_STDALONE
	pthread_mutex_lock(&ulock->mutex);
#endif
	ulock->ref++;
	ASSERT(ulock->ref == 1);
}

void
unlock(void *lock)
{
	struct ulock *ulock = (struct ulock*) lock;

	ASSERT(ulock);
	ASSERT(ulock->ref == 1);
	ulock->ref--;
#ifndef BCMSIM_STDALONE
	pthread_mutex_unlock(&ulock->mutex);
#endif
}

long
allocevent(int manualreset, int initialstate)
{
	struct event *ev;

	/* we only support auto-reset mode for now */
	ASSERT(manualreset == 0);
	ASSERT(initialstate == 0);

	ev = (struct event*) malloc(sizeof (struct event));
	ASSERT(ev);

#ifndef BCMSIM_STDALONE
	pthread_mutex_init(&ev->lock, NULL);
	pthread_cond_init(&ev->cond, NULL);
#endif
	ev->wake = 0;

	return ((long) ev);
}

void
freeevent(long e)
{
	struct event *ev;

	ASSERT(e);

	ev = (struct event*) e;

#ifndef BCMSIM_STDALONE
	pthread_mutex_destroy(&ev->lock);
	pthread_cond_destroy(&ev->cond);
#endif
	free((char*) ev);
}

int
setevent(long e)
{
	struct event *ev;

	ASSERT(e);

	ev = (struct event*) e;

#ifndef BCMSIM_STDALONE
	pthread_mutex_lock(&ev->lock);
	pthread_cond_broadcast(&ev->cond);
#endif
	ev->wake = 1;
#ifndef BCMSIM_STDALONE
	pthread_mutex_unlock(&ev->lock);
#endif

	return (1);
}

int
resetevent(long e)
{
	/* we only support auto-reset */
	ASSERT(0);
	return (1);
}

void
waitforevent(long e)
{
	struct event *ev;

	ASSERT(e);

	ev = (struct event*) e;

#ifndef BCMSIM_STDALONE
	pthread_mutex_lock(&ev->lock);
	if (ev->wake == 0)
		pthread_cond_wait(&ev->cond, &ev->lock);
#endif
	ASSERT(ev->wake);
	ev->wake = 0;
#ifndef BCMSIM_STDALONE
	pthread_mutex_unlock(&ev->lock);
#endif
}

int
getticks(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000000) + tv.tv_usec);
}

void
irql_add(int id, int initial_irql)
{
	ASSERT(0);
}

void irql_rem(int id)
{
	ASSERT(0);
}

void
irql_set(int irql)
{
	ASSERT(0);
}

int
irql_get(void)
{
	ASSERT(0);
	return (0);
}
