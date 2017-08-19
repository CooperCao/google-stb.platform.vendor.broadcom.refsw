/*
 * win32-specific system services.
 *
 * Copyright 2000, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of Broadcom Corporation.
 *
 * $Id$
 */

#define	DBG	1

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
/*
#include <winbase.h>
*/

#include <typedefs.h>
#include <rts/debug_mem.h>
#include <utils.h>
#include <sys_xx.h>

/*
 * For thread scheduling and IRQL management
 * we model an SMP where each thread gets its own cpu and
 * irql's are maintained on a per-thread basis.
 * calling KeRaiseIrql() affects only the local cpu.
 * This increases the preemptability of the threads and better
 * exercises the driver locking.
 */
#define	MAXTHREADS	16
struct thirql {
	int 	threadid;
	int	irql;
};
static struct thirql thirqltab[MAXTHREADS];

/* Elapsed time counter: for performance profiling */
struct etc {
	char *s;		/* Name */
	uint tics;		/* Tics at last printing */
	uint nscheds;		/* Number of schedules */
	uint et;		/* Total elapsed time */
	LARGE_INTEGER start;		/* Starting time */
	LARGE_INTEGER stop;
};
struct etc etab[NUM_ETCS];

#define	BADCH	(int)'?'
#define	BADARG	(int)':'
#define	EMSG	""

static int uniq = 0;
static void *sys_lock;

void
sys_init(void)
{
	uniq = getticks() * 100;
	sys_lock = alloclock();
	ASSERT(sys_lock);
}

HANDLE
createthread(void *func, void *arg, int initial_irql)
{
	HANDLE h;
	int id;

	ASSERT(sys_lock);

	h = CreateThread(NULL, 16 * 1024, (LPTHREAD_START_ROUTINE) func,
		(LPVOID) arg, 0, &id);

	/* add thread-to-irql entry to our table */
	irql_add(id, initial_irql);

	return (h);
}

int
getcurrentthreadid()
{
	return (GetCurrentThreadId());
}

void
exitthread(uintptr code)
{
	irql_rem(GetCurrentThreadId());
	ExitThread(code);
}

void
msleep(int milliseconds)
{
	Sleep(milliseconds);
}


/* Cheesy usleep routine */

#define	US_COUNT	250

void
usleep(unsigned int useconds)
{
	volatile int count = US_COUNT;

	while (useconds-- > 0) {
		for (count = US_COUNT; count; count--)
			;
	}
}



void
nsleep (unsigned long ns)
{
	usleep((ns + 999) / 1000);
}

void *
alloclock()
{
	PCRITICAL_SECTION crt;

	crt = malloc(sizeof (CRITICAL_SECTION));
	ASSERT(crt);

	/* zero RecursionCount because the underlying impl may not */
	bzero(crt, sizeof (CRITICAL_SECTION));

	InitializeCriticalSection(crt);
	ASSERT(crt->RecursionCount == 0);

	return ((void *) crt);
}

void
freelock(void *lock)
{
	ASSERT(((PCRITICAL_SECTION)lock)->RecursionCount == 0);
	free((char*) lock);
}

void
lock(void *lock)
{
	EnterCriticalSection((PCRITICAL_SECTION) lock);
	ASSERT(((PCRITICAL_SECTION)lock)->RecursionCount == 1);
}

void
unlock(void *lock)
{
	ASSERT(((PCRITICAL_SECTION)lock)->RecursionCount == 1);
	LeaveCriticalSection((PCRITICAL_SECTION) lock);
}

long
allocevent(int manualreset, int initialstate)
{
	HANDLE h;
	char name[32];

	uniq++;

	sprintf(name, "%devent%d\n", GetCurrentProcessId(), uniq);

	h = CreateEvent(NULL, manualreset, initialstate, (LPTSTR) name);

	return ((long) h);
}

void
freeevent(long e)
{
	CloseHandle((HANDLE) e);
}

int
setevent(long e)
{
	return (SetEvent((HANDLE)e));
}

int
resetevent(long e)
{
	return (ResetEvent((HANDLE)e));
}

void
waitforevent(long e)
{
	WaitForSingleObject((HANDLE)e, INFINITE);
}

int
wfetimeout(long e, int milliseconds)
{
	DWORD rc;
	rc = WaitForSingleObject((HANDLE)e, milliseconds);

	if (rc == WAIT_TIMEOUT)
		return 1;

	return 0;
}

static struct thirql*
getthirql(int id)
{
	int i;

	for (i = 0; i < MAXTHREADS; i++)
		if (thirqltab[i].threadid == id)
			return (&thirqltab[i]);
	return (NULL);
}

void
irql_add(int id, int initial_irql)
{
	struct thirql *p;

	lock(sys_lock);

	p = getthirql(0);
	ASSERT(p);

	p->threadid = id;
	p->irql = initial_irql;

	unlock(sys_lock);
}

void
irql_rem(int id)
{
	struct thirql *p;

	lock(sys_lock);

	p = getthirql(id);

	if (p)
		bzero(p, sizeof (struct thirql));

	unlock(sys_lock);
}

void
irql_set(int irql)
{
	struct thirql *p;
	int id;

	id = GetCurrentThreadId();

	lock(sys_lock);

	p = getthirql(id);
	ASSERT(p);

	p->irql = irql;

	unlock(sys_lock);
}

int
irql_get()
{
	struct thirql *p;
	int id;
	int irql;

	id = GetCurrentThreadId();

	lock(sys_lock);

	p = getthirql(id);

	irql = p->irql;

	unlock(sys_lock);

	return (irql);
}

int
getticks(void)
{
	return (GetTickCount());
}

/*
 * Returns integer number of hundred thousands of seconds elapsed.
 * NOTE: this function is no good if elapsed time or frequency is more than an integer.
 */
static uint
hts(LARGE_INTEGER *start, LARGE_INTEGER *stop)
{
	LARGE_INTEGER freq;
	uint diff, f1;

	QueryPerformanceFrequency(&freq);
	ASSERT(freq.HighPart == 0);

	f1 = freq.LowPart / 100000;
	
	/* Modulo arithmetic */
	diff = (uint) ((int) stop->LowPart - (int) start->LowPart);

	diff = diff / f1;
	return diff;
}

/* Small collection of time routines, for performance tracking. */
void
tr_clear(int i)
{
	ASSERT (i < NUM_ETCS);

	etab[i].et = 0;
	etab[i].nscheds = 0;
	etab[i].tics = GetTickCount();
}

int
tr_alloc(char *s)
{
	int i;

	for (i = 0; i < NUM_ETCS; i++) {
		if (etab[i].s == NULL) {
			etab[i].s = s;
			return i;
		}
	}
	ASSERT (0);
	return 0;
}

void
tr_start(int i)
{
	ASSERT(i < NUM_ETCS);
	QueryPerformanceCounter(&etab[i].start);
}

void
tr_stop(int i)
{
	uint elapsed;

	ASSERT(i < NUM_ETCS);
	QueryPerformanceCounter(&etab[i].stop);

	elapsed = hts (&etab[i].start, &etab[i].stop);

	etab[i].et += elapsed;
	ASSERT(etab[i].et >= elapsed);
	etab[i].nscheds++;
}

void
tr_print(int i)
{
	int tics;

	ASSERT(i < NUM_ETCS);

	if (etab[i].s == NULL)
		return;

	tics = GetTickCount();
	tics -= etab[i].tics;
	printf("Counter %s, tics elapsed %d, time consumed %d, calls %d\n",
			etab[i].s, tics, etab[i].et, etab[i].nscheds);
	if (tics != 0 && etab[i].nscheds != 0)
		printf("\t\ttime per tic %d, time per call %d\n",
			etab[i].et / tics, etab[i].et / etab[i].nscheds);
	etab[i].tics = GetTickCount();
}
