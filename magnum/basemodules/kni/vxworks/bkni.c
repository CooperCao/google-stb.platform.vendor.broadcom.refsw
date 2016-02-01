/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * $brcm_Revision: $ *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bkni_metrics.h"
#include "bkni_event_group.h"
#include "blst_list.h"

/* VxWorks include files */
#include "vxWorks.h"
#include "fioLib.h"
#include "tickLib.h"
#include "semLib.h"
#include "sysLib.h"
#include "taskLib.h"
#include "intLib.h"
#include <timers.h>

#include <string.h>


BDBG_MODULE(kernelinterface);


/***************************************************************************
 *
 *	Defines, Typedefs, Structures, etc
 *
 ***************************************************************************/
#if BKNI_USE_TAGGED_API
static BKNI_Metrics g_metrics;
static BKNI_Metrics_LoggingState g_metricsLoggingState;

#define	TAG_GET_NAME(A,B)		A ## B

#define	TAG_PRINT_CALL(S,T)									\
do{																	\
	if( g_metricsLoggingState.TAG_GET_NAME(print##T,s) == true )	\
	{																\
		BKNI_Printf("%s() at %s, line %d", S, file, line ); 		\
	}																\
}while(0)

#define	TAG_PRINT_CALL_P1(S,T,PV)							\
do{																	\
	if( g_metricsLoggingState.TAG_GET_NAME(print##T,s) == true )	\
	{																\
		BKNI_Printf("%s(0x%lx) at %s, line %d", S, (unsigned long) PV, file, (int) line ); \
	}																\
}while(0)

#define	TAG_PRINT_CALL_P2(S,T,PV1,PV2)						\
{																	\
	if( g_metricsLoggingState.TAG_GET_NAME(print##T,s) == true )	\
	{																\
		BKNI_Printf("%s(0x%lx,0x%lx) at %s, line %d", S, (unsigned long) PV1, (unsigned long) PV2, file, line ); \
	}																\
}while(0)

#define	TAG_PRINT_CALL_START_TIME(S,T)								\
do{																	\
	g_metrics.TAG_GET_NAME(total##T,s)++;							\
	if( g_metricsLoggingState.TAG_GET_NAME(print##T,s) == true )	\
	{																\
		BKNI_Printf("%s() at %s, line %d", S, file, line );			\
	}																\
}while(0)

#define	TAG_PRINT_CALL_DELTA(S,T,PV)							\
do{																	\
	g_metrics.TAG_GET_NAME(total##T,Time) += PV;					\
	if( PV > g_metrics.TAG_GET_NAME(max##T,Time) )					\
	{																\
		g_metrics.TAG_GET_NAME(max##T,Time) = (uint32_t) PV;		\
	}																\
	if( g_metricsLoggingState.TAG_GET_NAME(print##T,s) == true )	\
	{																\
		BKNI_Printf("%s(0x%lx) at %s, line %d", S, (unsigned long) PV, file, line ); \
	}																\
}while(0)

#define	TAG_PRINT_CALL_DELAY_TIME(S,T,PV)							\
do{																	\
	g_metrics.TAG_GET_NAME(total##T,s)++;							\
	g_metrics.TAG_GET_NAME(total##T,Time) += PV;					\
	if( PV > g_metrics.TAG_GET_NAME(max##T,Time) )					\
	{																\
		g_metrics.TAG_GET_NAME(max##T,Time) = (uint32_t) PV;		\
	}																\
	if( g_metricsLoggingState.TAG_GET_NAME(print##T,s) == true )	\
	{																\
		BKNI_Printf("%s(0x%lx) at %s, line %d", S, (unsigned long) PV, file, line ); \
	}																\
}while(0)

#define	TAG_UPDATE_DELTA_TIME(T,PV)									\
do{																	\
	g_metrics.TAG_GET_NAME(total##T,Time) += PV;					\
	if( PV > g_metrics.TAG_GET_NAME(max##T,Time) )					\
	{																\
		g_metrics.TAG_GET_NAME(max##T,Time) = (uint32_t) PV;		\
	}																\
}while(0)

#define	TAG_BKNI_DELAY(A)											\
BKNI_DelayTagged(A, const char *file, unsigned line)

#define	TAG_BKNI_SLEEP(A)											\
BKNI_SleepTagged(A, const char *file, unsigned line)

#define	TAG_BKNI_CREATEEVENT(A)										\
BKNI_CreateEventTagged(A, const char *file, unsigned line)

#define	TAG_BKNI_SETEVENT(A)										\
BKNI_SetEventTagged(A, const char *file, unsigned line)

#define	TAG_BKNI_WAITFOREVENT(A, B)									\
BKNI_WaitForEventTagged(A, B, const char *file, unsigned line)

#define	TAG_BKNI_DESTROYEVENT(A)									\
BKNI_DestroyEventTagged(A, const char *file, unsigned line)

#define	TAG_BKNI_ENTERCRITICALSECTION(A)							\
BKNI_EnterCriticalSectionTagged(const char *file, unsigned line)

#define	TAG_BKNI_LEAVECRITICALSECTION(A)							\
BKNI_LeaveCriticalSectionTagged(const char *file, unsigned line)

#define	TAG_BKNI_ASSERTISRCONTEXT(A)								\
BKNI_AssertIsrContext_isr(const char *filename, unsigned lineno)

#define	TAG_BKNI_CREATEMUTEX(A)										\
BKNI_CreateMutexTagged(A, const char *file, unsigned line)

#define	TAG_BKNI_DESTROYMUTEX(A)									\
BKNI_DestroyMutexTagged(A, const char *file, unsigned line)

#define	TAG_BKNI_TRYACQUIREMUTEX(A)									\
BKNI_TryAcquireMutexTagged(A, const char *file, unsigned line)

#define	TAG_BKNI_ACQUIREMUTEX(A)									\
BKNI_AcquireMutexTagged(A, const char *file, unsigned line)

#define	TAG_BKNI_RELEASEMUTEX(A)									\
BKNI_ReleaseMutexTagged(A, const char *file, unsigned line)



#else
#define	TAG_PRINT_CALL(S,T)
#define	TAG_PRINT_CALL_P1(S,T,PV)
#define	TAG_PRINT_CALL_P2(S,T,PV1,PV2)
#define	TAG_PRINT_CALL_START_TIME(S,T)
#define	TAG_PRINT_CALL_DELTA(S,T,PV)
#define	TAG_PRINT_CALL_DELAY_TIME(S,T,PV)
#define	TAG_UPDATE_DELTA_TIME(T,PV)

#define	TAG_BKNI_DELAY(A)											\
BKNI_Delay(A)

#define	TAG_BKNI_SLEEP(A)											\
BKNI_Sleep(A)

#define	TAG_BKNI_CREATEEVENT(A)										\
BKNI_CreateEvent(A)

#define	TAG_BKNI_SETEVENT(A)										\
BKNI_SetEvent(A)

#define	TAG_BKNI_WAITFOREVENT(A, B)									\
BKNI_WaitForEvent(A, B)

#define	TAG_BKNI_DESTROYEVENT(A)									\
BKNI_DestroyEvent(A)

#define	TAG_BKNI_ENTERCRITICALSECTION(A)							\
BKNI_EnterCriticalSection(A)

#define	TAG_BKNI_LEAVECRITICALSECTION(A)							\
BKNI_LeaveCriticalSection(A)

#define	TAG_BKNI_ASSERTISRCONTEXT(A)								\
BKNI_AssertIsrContext_isr(A)

#define	TAG_BKNI_CREATEMUTEX(A)										\
BKNI_CreateMutex(A)

#define	TAG_BKNI_DESTROYMUTEX(A)									\
BKNI_DestroyMutex(A)

#define	TAG_BKNI_TRYACQUIREMUTEX(A)									\
BKNI_TryAcquireMutex(A)

#define	TAG_BKNI_ACQUIREMUTEX(A)									\
BKNI_AcquireMutex(A)

#define	TAG_BKNI_RELEASEMUTEX(A)									\
BKNI_ReleaseMutex(A)


#endif

#define	DEFAULT_STACK_SZ	(0x1000)	/* Default: 4K Bytes */

typedef struct BKNI_EventObj
{
	BLST_D_ENTRY(BKNI_EventObj) list;
	SEM_ID eventId;
	struct BKNI_GroupObj *group;
} BKNI_EventObj;

typedef struct BKNI_MutexObj
{
	SEM_ID mutexId;
	int ownerTaskId;
} BKNI_MutexObj;

struct BKNI_GroupObj 
{
	BLST_D_HEAD(group, BKNI_EventObj) members;
	SEM_ID lock;
	SEM_ID event;
};


static int g_init;
static int g_inCriticalSection;
static uint32_t g_criticalSectionStartTime;
static uint32_t g_mutexSectionStartTime;

#if BINT_NO_TASK_ISR
#error This mode of handling interrupts is no longer supportted. Please use Task context to service interrupts.
#else
static SEM_ID	g_bkni_crit_lock;
#endif


static void BKNI_GetTime(
	uint32_t *pulMsec)
{
	*pulMsec = (uint32_t) (tickGet() * 10);
	return;
}

static void BKNI_GetDeltaTime(
	uint32_t t1,
	uint32_t t2,
	uint32_t *pulDeltaTime)
{
	if( t1 > t2 )
	{
		*pulDeltaTime = 0xFFFFFFFF - t1;
		*pulDeltaTime += t2;
	}
	else
	{
		*pulDeltaTime = t2 - t1;
	}
	return;
}

static unsigned long BKNI_ConvertMillisecondsToTicks(
	unsigned long ulMilliseconds)
{
	unsigned long ticks;

	/* 1000 is represents the number of millseconds per second, since
	   sysClkRateGet() returns back number of ticks per second */
	ticks = ((((unsigned long)sysClkRateGet() * ulMilliseconds ) + 999)/ 1000);
	if( ticks == 0 )
	{
		/* to handle delays that are less then tick */
		ticks = 1;
	}
	return ticks;
}


/***************************************************************************
 *	General Functions
 ***************************************************************************/
BERR_Code BKNI_Init(
	void)
{
	BERR_Code retVal = BERR_SUCCESS;


	if (!g_init)
	{
		/* For VxWorks, the OS has to running by now, so nothing to do */
		g_init = 1;
		g_inCriticalSection = 0;
		g_criticalSectionStartTime = 0;
		g_mutexSectionStartTime = 0;
		BKNI_Metrics_Reset();
		retVal = BERR_SUCCESS;

#if BINT_NO_TASK_ISR
#error This mode of handling interrupts is no longer supportted. Please use Task context to service interrupts.
#else
        /* Note: this is a vxworks mutex semaphore which allows for nested locks from the same task.
        ** We don't allow nested critical locks so an additional test is included to detect nested calls.
        */
		g_bkni_crit_lock = semMCreate((SEM_Q_PRIORITY | SEM_DELETE_SAFE));
		if( !g_bkni_crit_lock )
		{
			retVal = BERR_OS_ERROR;
		}
#endif
	}
	else
	{
		retVal = BERR_OS_ERROR;
	}
	return(retVal);
}

void BKNI_Uninit(
	void)
{
	g_init = 0;
}


/* coverity[+kill]  */
void BKNI_Fail(
	void)
{
    volatile int i;

    /* These will cause a processor exception (and hopefully a stack crawl to show who called this). */
    i = *(int *)0; /* try to fail, take 1 */
    i = 0;
    i = 1/i;  /* try to fail, take 2 */

    for(;;) { } /* hang here */
}


/*
 * The following implementation of the snprintf() and vsnprintf() was taken
 * from VxWorks user group forum.
 *
 * A container of the following type is allocated from the stack
 * on entry to vsnprintf(), initialized with details about the
 * caller's output buffer, and passed to fioFormatV() for use by
 * sn_output_fn().
 */
typedef struct {
  char *s;        /* The user's output buffer */ 
  int nmax;       /* The length of the user's output buffer */
  int length;     /* The current length of the string in s[] */
} PrintData;

static STATUS sn_output_fn(char *buffer, int nc, int arg);

/*
 * I don't have access to an implementation of snprintf() so I am guessing
 * at the order and types of its arguments.
 */
int BKNI_Snprintf(char *s, size_t n, const char *fmt, ...)
{
	int nc;      /* The number of characters succesfully output (-1 on error) */
	va_list ap;  /* The variable argument list */
	va_start(ap, fmt);
	nc = BKNI_Vsnprintf(s, n, fmt, ap);
	va_end(ap);
	return nc;
}

int BKNI_Vsnprintf(char *s, size_t n, const char *fmt, va_list ap)
{
	PrintData data;  /* A container for details about the output buffer */
/*
 * Record details about the output buffer in 'data'. This can
 * then be passed to the fioFormatV() output function.
 */
	data.s = s;
	data.nmax = n;
	data.length = 0;
/*
 * Format output into data.s[] and record the accumulated length
 * of the string in data.length.
 */
	if(fioFormatV(fmt, ap, sn_output_fn, (int) &data) == ERROR)
		return ERROR;
/*
 * Terminate the accumulated string.
 * Note that sn_output_fn() guarantees that data.length is < data.nmax.
 */
	data.s[data.length] = '\0';
/*
 * Return a count of the number of characters in s[], excluding the
 * terminating '\0'. Note that this will be less than the number
 * expected, if the output had to be truncated.
 */
	return data.length;
}

/*.......................................................................
 * This is the function that fioFormatV() calls to render output into
 * the snprintf() caller's output buffer.
 */
static STATUS sn_output_fn(char *buffer, int nc, int arg)
{
	PrintData *data = (PrintData *) arg;
/*
 * Work out how many of the nc characters in buffer[] can be appended
 * to data->s[]. Leave room for a '\0' terminator at the end of
 * data->s[].
 */
	int nnew = nc;


	if(data->length + nnew + 1 >= data->nmax)
		nnew = data->nmax - data->length - 1;
/*
 * Append nnew characters from buffer[] to data.s[].
 */
	if(nnew > 0)
	{
		memcpy(data->s + data->length, buffer, nnew);
		data->length += nnew;
	}
	return OK;
}

/***************************************************************************
 *	Delay and Sleep Functions
 ***************************************************************************/

void TAG_BKNI_DELAY(
	unsigned int microsec)
{
    /* This uses a udelay function in the BSP to perform the delay */
    void Delayus(unsigned long n);
    if (!microsec) microsec=1; /* delay of zero would be bad */
    Delayus(microsec);
}


BERR_Code TAG_BKNI_SLEEP(
	unsigned int millisec)
{
	BERR_Code retVal = BERR_SUCCESS;
	unsigned long ticks;


	TAG_PRINT_CALL_DELAY_TIME("BKNI_Sleep", Sleep, millisec);

	/* multiply int value and int value in this function*/
	if( millisec == 0 )
	{
		/* VxWorks allows other tasks of same priority to run */
		ticks = NO_WAIT;
	}
	else
	{
		ticks = BKNI_ConvertMillisecondsToTicks((unsigned long) millisec);
	}
	if(taskDelay(ticks) == ERROR)
	{
		retVal = BERR_OS_ERROR;
	}

	return(retVal);
}


/***************************************************************************
 *	Event Functions
 ***************************************************************************/
BERR_Code TAG_BKNI_CREATEEVENT(
	BKNI_EventHandle *phEvent)
{
	BERR_Code retVal = BERR_OS_ERROR;
	BKNI_EventObj *pEventObj;


	*phEvent = NULL;
	pEventObj = (BKNI_EventObj *) BKNI_Malloc(sizeof(BKNI_EventObj));
	if(pEventObj != NULL)
	{
		/* VxWorks implementation of Kernel Interface uses SemB
		   feature of VxWorks.  This was done, since Events in VxWorks
		   are based on a task.  VxWorks does not implement Events
		   as independent object, outside the scope of a task.  Therefore,
		   to implement an Event as an independent object, SemB is used. */
		pEventObj->eventId = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
		if(pEventObj->eventId == NULL)
		{
			BKNI_Free((void *) pEventObj);
		}
		else
		{
			retVal = BERR_SUCCESS;
			pEventObj->group = NULL;
			*phEvent = (BKNI_EventHandle) pEventObj;
		}
	}
	return(retVal);
}

void TAG_BKNI_SETEVENT(
	BKNI_EventHandle hEvent)
{
	STATUS errCode;


	TAG_PRINT_CALL_P1("BKNI_SetEvent", Event, hEvent);
	BDBG_ASSERT(hEvent);

	if (hEvent->group) {
		errCode = semTake(hEvent->group->lock, WAIT_FOREVER);
        BDBG_ASSERT(errCode != ERROR);
    }

	errCode = semGive(hEvent->eventId);
	BDBG_ASSERT(errCode != ERROR);
	if (hEvent->group) { /* XXX it's possible race condition */
		errCode = semGive(hEvent->group->event);
		BDBG_ASSERT(errCode == OK);
		errCode = semGive(hEvent->group->lock);
        BDBG_ASSERT(errCode == OK);
	}
	return;
}

BERR_Code TAG_BKNI_WAITFOREVENT(
	BKNI_EventHandle hEvent,
	int millisec)
{
	BERR_Code retVal = BERR_SUCCESS;
	STATUS errCode;
	unsigned long tick;

	TAG_PRINT_CALL_P2("BKNI_WaitForEvent", Event, hEvent, millisec);
	BDBG_ASSERT(hEvent);

	if( millisec == BKNI_INFINITE )
	{
		tick = WAIT_FOREVER;
	}
	else
	{
		tick = BKNI_ConvertMillisecondsToTicks((unsigned long) millisec);
	}
	/*
	 * If you wait for 1 tick, the system may timeout from 0-10 msec, it cause the frontend
	 * locking failed occasionally. So, when timeout count is 1 tick, we increase to 2 to make
	 * sure the system to wait at least 10 ms as user expected.
	 */
	if (tick == 1) tick++;

	if( (errCode = semTake(hEvent->eventId, tick)) == ERROR )
	{
		/* Check if this is timeout or was it an error */
		switch( errno )
		{
			case S_intLib_NOT_ISR_CALLABLE:
			case S_objLib_OBJ_ID_ERROR:
			case S_objLib_OBJ_UNAVAILABLE:
				retVal = BERR_OS_ERROR;
				break;
			default:
				retVal = BERR_TIMEOUT;
				break;
		}
	}

	return(retVal);
}

void BKNI_ResetEvent(
	BKNI_EventHandle hEvent)
{

	
	/* Since VxWorks doesn't have Reset Event function, the following is
	   an implementation of one, where if an event exists, it will be retrieved */
	if( semTake(hEvent->eventId, 0) == ERROR )
	{
		/* Check if this is timeout or was it an error */
		switch( errno )
		{
			case S_intLib_NOT_ISR_CALLABLE:
			case S_objLib_OBJ_ID_ERROR:
				BDBG_ASSERT( false );
				break;
			case S_objLib_OBJ_UNAVAILABLE:
			default:
				break;
		}
	}
	return;
}

void TAG_BKNI_DESTROYEVENT(
	BKNI_EventHandle hEvent)
{
	STATUS errCode;


	TAG_PRINT_CALL_P1("BKNI_DestroyEvent", Event, hEvent);
	BDBG_ASSERT(hEvent);

	if (hEvent->group) {
		BDBG_WRN(("Event %#x still in the group %#x, removing it", (unsigned)hEvent, (unsigned)hEvent->group));
		errCode = semTake(hEvent->group->lock, WAIT_FOREVER);
		BDBG_ASSERT(errCode == OK);
		BLST_D_REMOVE(&hEvent->group->members, hEvent, list);
		errCode = semGive(hEvent->group->lock);
		BDBG_ASSERT(errCode == OK);
	}

	errCode = semDelete(hEvent->eventId);
	BDBG_ASSERT(errCode != ERROR);
	BKNI_Free((void *) hEvent);
	return;
}


/***************************************************************************
 *	Critical Section Functions
 ***************************************************************************/
void TAG_BKNI_ENTERCRITICALSECTION(
	void)
{
	TAG_PRINT_CALL_START_TIME("BKNI_EnterCriticalSection", CriticalSection);
	
	BKNI_GetTime(&g_criticalSectionStartTime);

#if BINT_NO_TASK_ISR
#error This mode of handling interrupts is no longer supportted. Please use Task context to service interrupts.
#else
{
	int errCode;

	errCode = semTake( g_bkni_crit_lock, WAIT_FOREVER );
	if( errCode != OK)
	{
		BDBG_ERR(("BKNI_EnterCriticalSection: failed to take lock"));
		BKNI_Fail();
	}
}
#endif

	/* The semaphore take above uses a mutext semaphore.  This will NOT block if there is no owner
	** OR the requesting task is the current owner (nested calls).  We do not allow for nested calls
	** so catch this situation with a entry flag.
	*/
	if( g_inCriticalSection )
	{
		BDBG_ERR(("BKNI_EnterCriticalSection detected nesting, which should be avoided."));
		BKNI_Fail();
	}
	g_inCriticalSection = 1;
	return;
}

void TAG_BKNI_LEAVECRITICALSECTION(
	void)
{
	uint32_t curTime;
	unsigned long totalTime;

	BKNI_GetTime(&curTime);
	BKNI_GetDeltaTime(g_criticalSectionStartTime, curTime, &totalTime);

	TAG_PRINT_CALL_DELTA("BKNI_LeaveCriticalSection", CriticalSection, totalTime);

#if BINT_NO_TASK_ISR
#error This mode of handling interrupts is no longer supportted. Please use Task context to service interrupts.
#else
{
	int errCode;

	/* The semaphore give below uses a mutext semaphore.
	** Use an entry flag that tells me if we Leave a Critical Section twice.
	*/
	if( !g_inCriticalSection )
	{
		BDBG_ERR(("BKNI_LeaveCriticalSection detected extra call."));
		BKNI_Fail();
	}

	g_inCriticalSection = 0;
	errCode = semGive( g_bkni_crit_lock );
	if( errCode == ERROR )
	{
		BDBG_ERR(("BKNI_LeaveCriticalSection: failed to return lock (errCode=%x)", errCode));
		BKNI_Fail();
	}
}
#endif
	return;
}

/* We really can't check if we're in ISR context, but we can check whether we're in a critical section */
/* We don't recommend that they do much of anything in an ISR, but instead use a high priority task to handle ISR operations. */
/* This task uses the critical section lock that we can check here. */
void TAG_BKNI_ASSERTISRCONTEXT(
	void)
{
	if( !g_inCriticalSection )
	{
		BDBG_ERR(("BKNI_AssertCriticalSection -- we are NOT in a critical section."));
		BKNI_Fail();
	}
}


/***************************************************************************
 *	Mutex Functions
 ***************************************************************************/
BERR_Code TAG_BKNI_CREATEMUTEX(
	BKNI_MutexHandle *phMutex)
{
	BERR_Code retVal = BERR_OS_ERROR;
	BKNI_MutexObj *pMutexObj;


	*phMutex = NULL;
	pMutexObj = (BKNI_MutexObj *) BKNI_Malloc(sizeof(BKNI_MutexObj));
	if( pMutexObj != NULL )
	{
		/* Use VxWorks Priority method to resolve which task to run */
		pMutexObj->mutexId = semMCreate((SEM_Q_PRIORITY | SEM_DELETE_SAFE));
		if( pMutexObj->mutexId == NULL )
		{
			BKNI_Free((void *) pMutexObj);
		}
		else
		{
			retVal = BERR_SUCCESS;
			*phMutex = (BKNI_MutexHandle) pMutexObj;
			pMutexObj->ownerTaskId = ERROR;
		}
	}
	return(retVal);
}

void TAG_BKNI_DESTROYMUTEX(
	BKNI_MutexHandle hMutex)
{
	STATUS errCode;


	errCode = semDelete(hMutex->mutexId);
	BDBG_ASSERT(errCode != ERROR);
	return;
}

BERR_Code TAG_BKNI_TRYACQUIREMUTEX(
	BKNI_MutexHandle hMutex)
{
	BERR_Code retVal = BERR_SUCCESS;


	TAG_PRINT_CALL_P1("BKNI_TryAcquireMutex", MutexSection, hMutex);
	BDBG_ASSERT(hMutex);

	BKNI_GetTime(&g_mutexSectionStartTime);

	/* VxWorks Mutex supports nested calls if the calls are from the same task,
	   but the Kernel Interface doesn't support nested calls for Mutex, so
	   to simulated not supporting nested calls, do the following */

	/* No need to use taskLock() and taskUnlock() when setting ownerTaskId since
	   once the VxWorks semaphore is taken, then modifying ownerTaskId variable
	   can be safely done. */
	if( semTake(hMutex->mutexId, NO_WAIT) == ERROR )
	{
		/* Check if this is timeout or was it an error */
		switch( errno )
		{
			case S_intLib_NOT_ISR_CALLABLE:
			case S_objLib_OBJ_ID_ERROR:
				retVal = BERR_OS_ERROR;
				break;
			case S_objLib_OBJ_UNAVAILABLE:
				/* VxWorks seems to return this error message when used with
				   timeout set to NO_WAIT under simulation on a PC.  Therefore
				   this is considered the same as TIMEOUT error */
			case S_objLib_OBJ_TIMEOUT:
			default:
				retVal = BERR_TIMEOUT;
				break;
		}
	}
	else
	{
		if( hMutex->ownerTaskId != ERROR )	/* Some task own's this mutex */
		{
			/* Check if same task that owns the mutex is trying to acquiring it again */
			if( hMutex->ownerTaskId == taskIdSelf() )
			{
				BDBG_ERR(("BKNI_AcquireMutex detected nesting, which should be avoided."));
				BKNI_Fail();
			}
		}
		hMutex->ownerTaskId = taskIdSelf();
	}
	return(retVal);
}

BERR_Code TAG_BKNI_ACQUIREMUTEX(
	BKNI_MutexHandle hMutex)
{
	BERR_Code retVal = BERR_SUCCESS;


	TAG_PRINT_CALL_P1("BKNI_AcquireMutex", MutexSection, hMutex);
	BDBG_ASSERT(hMutex);

	BKNI_GetTime(&g_mutexSectionStartTime);

	/* VxWorks Mutex supports nested calls if the calls are from the same task,
	   but the Kernel Interface doesn't support nested calls for Mutex, so
	   to simulated not supporting nested calls, do the following */
	/* No need to use taskLock() and taskUnlock() when setting ownerTaskId since
	   once the VxWorks semaphore is taken, then modifying ownerTaskId variable
	   can be safely done. */
	if( semTake(hMutex->mutexId, WAIT_FOREVER) == ERROR )
	{
		/* Check if this is timeout or was it an error */
		switch( errno )
		{
			case S_intLib_NOT_ISR_CALLABLE:
			case S_objLib_OBJ_ID_ERROR:
			case S_objLib_OBJ_UNAVAILABLE:
				retVal = BERR_OS_ERROR;
				break;
			default:
				retVal = BERR_TIMEOUT;
				break;
		}
	}
	else
	{
		if( hMutex->ownerTaskId != ERROR )	/* Some task own's this mutex */
		{
			/* Check if same task that owns the mutex is trying to acquiring it again */
			if( hMutex->ownerTaskId == taskIdSelf() )
			{
				BDBG_ERR(("BKNI_AcquireMutex detected nesting, which should be avoided."));
				BKNI_Fail();
			}
		}
		hMutex->ownerTaskId = taskIdSelf();
	}
	return(retVal);
}

void TAG_BKNI_RELEASEMUTEX(
	BKNI_MutexHandle hMutex)
{
	uint32_t curTime;
	unsigned long totalTime;
	STATUS errCode;


	TAG_PRINT_CALL_P1("BKNI_ReleaseMutex", MutexSection, hMutex);
	BDBG_ASSERT(hMutex);

	BKNI_GetTime(&curTime);
	BKNI_GetDeltaTime(g_mutexSectionStartTime, curTime, &totalTime);

	TAG_UPDATE_DELTA_TIME(MutexSection, totalTime);

	if (hMutex->ownerTaskId !=  taskIdSelf()) {
		BDBG_ERR(("BKNI_ReleaseMutex detected out of order call, which should be avoided."));
		BKNI_Fail();
        }
	hMutex->ownerTaskId = ERROR;
	errCode = semGive(hMutex->mutexId);
	BDBG_ASSERT(errCode != ERROR);
	return;
}


/***************************************************************************
 *	Metric Functions
 ***************************************************************************/
void BKNI_Metrics_Get(
	BKNI_Metrics *metrics)
{
#if BKNI_USE_TAGGED_API
	BKNI_Memcpy(metrics, &g_metrics, sizeof(BKNI_Metrics));
#else
	BKNI_Memset(metrics, 0, sizeof(BKNI_Metrics));
#endif
}

void BKNI_Metrics_Reset(
	void)
{
#if BKNI_USE_TAGGED_API
	BKNI_Memset(&g_metrics, 0, sizeof(BKNI_Metrics));
#endif
}

void BKNI_Metrics_GetLoggingState(
	BKNI_Metrics_LoggingState *logging)
{
#if BKNI_USE_TAGGED_API
	BKNI_Memcpy(logging, &g_metricsLoggingState, sizeof(BKNI_Metrics_LoggingState));
#else
	BKNI_Memset(logging, 0, sizeof(BKNI_Metrics_LoggingState));
#endif
}

void BKNI_Metrics_SetLoggingState(
	const BKNI_Metrics_LoggingState *logging)
{
#if BKNI_USE_TAGGED_API
	BKNI_Memcpy(&g_metricsLoggingState, logging, sizeof(BKNI_Metrics_LoggingState));
#else
    BSTD_UNUSED(logging);
#endif
}


/***************************************************************************
 *	Event Group Functions
 ***************************************************************************/
BERR_Code 
BKNI_CreateEventGroup(BKNI_EventGroupHandle *pGroup)
{
	BKNI_EventGroupHandle group;
	BERR_Code result;

	group = (BKNI_EventGroupHandle)BKNI_Malloc(sizeof(*group));
	if (!group) {
		result = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto err_no_memory;
	}
	BLST_D_INIT(&group->members);
	group->lock = semBCreate(SEM_Q_PRIORITY, SEM_FULL);
	if (group->lock == NULL) {
		result = BERR_TRACE(BERR_OS_ERROR);
		goto err_mutex;
	}
	group->event = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
	if (group->event == NULL) {
		result = BERR_TRACE(BERR_OS_ERROR);
		goto err_event;
	}

	*pGroup = group;
	return BERR_SUCCESS;

err_event:
	semDelete(group->lock);
err_mutex:
	BKNI_Free((void *)group);
err_no_memory:
	return result;
}

void
BKNI_DestroyEventGroup(BKNI_EventGroupHandle group)
{
	int rc;
	BKNI_EventHandle event;

	rc = semTake(group->lock, WAIT_FOREVER);
	if (rc!=OK) {
		BDBG_ERR(("semTake returned %d", rc));
		BDBG_ASSERT(false);
	}
	while( NULL != (event=BLST_D_FIRST(&group->members)) ) {
		event->group = NULL;
		BLST_D_REMOVE_HEAD(&group->members, list);
	}
	semGive(group->lock);
	semDelete(group->lock);
	semDelete(group->event);
	BKNI_Free((void *)group);
	return;
}

BERR_Code
BKNI_AddEventGroup(BKNI_EventGroupHandle group, BKNI_EventHandle event)
{
	int rc;
	BERR_Code result = BERR_SUCCESS;

	/* IMPORTANT: group lock shall be acquired before event lock */
	rc = semTake(group->lock, WAIT_FOREVER);
	if (rc!=OK) {
		BDBG_ERR(("semTake failed, rc=%d", rc));
		BDBG_ASSERT(false);
	}
	if (event->group != NULL) {
		BDBG_ERR(("Event %#x already connected to the group %#x", (unsigned)event, (unsigned)group));
		result = BERR_TRACE(BERR_OS_ERROR);
	} else {
		BLST_D_INSERT_HEAD(&group->members, event, list);
		event->group = group;
		rc = semTake(event->eventId, NO_WAIT);
		if (rc==OK) { /* potential race condition */
			semGive(event->eventId);
			semGive(group->event);
		}
	}
	semGive(group->lock);
	return result;
}

BERR_Code
BKNI_RemoveEventGroup(BKNI_EventGroupHandle group, BKNI_EventHandle event)
{
	int rc;
	BERR_Code result = BERR_SUCCESS;

	rc = semTake(group->lock, WAIT_FOREVER);
	if (rc!=OK) {
		BDBG_ERR(("semTake failed, rc=%d", rc));
		BDBG_ASSERT(false);
	}
	if (event->group != group) {
		BDBG_ERR(("Event %#x doesn't belong to the group %#x", event, group));
		result = BERR_TRACE(BERR_OS_ERROR);
	} else {
		BLST_D_REMOVE(&group->members, event, list);
		event->group = NULL;
	}
	semGive(group->lock);
	return result;
}

static unsigned 
group_get_events(BKNI_EventGroupHandle group, BKNI_EventHandle *events, unsigned max_events)
{
	BKNI_EventHandle ev;
	int rc;
	unsigned event;

	rc = semTake(group->lock, WAIT_FOREVER);
	BDBG_ASSERT(rc==OK);
	for(event=0, ev=BLST_D_FIRST(&group->members); ev && event<max_events ; ev=BLST_D_NEXT(ev, list)) {
			rc = semTake(ev->eventId, NO_WAIT);
			if (rc==OK) {
				events[event] = ev;
				event++;
			}
	}
	rc = semGive(group->lock);
	BDBG_ASSERT(rc==OK);
	return event;
}

BERR_Code 
BKNI_WaitForGroup(BKNI_EventGroupHandle group, int timeoutMsec, BKNI_EventHandle *events, unsigned max_events, unsigned *nevents)
{
	int rc;
	struct timespec target,now;
	unsigned long timeout = WAIT_FOREVER;
	bool forever=false;
	
	
	if (max_events<1) {
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}
	if (timeoutMsec==BKNI_INFINITE) {
		timeout = WAIT_FOREVER;
		forever = true;
	} else if (timeoutMsec==0) {
    	semTake(group->event, NO_WAIT); /* just clear event */
		*nevents = group_get_events(group, events, max_events);
		return *nevents ? BERR_SUCCESS:BERR_TIMEOUT;
	} else {
		rc = clock_gettime(CLOCK_REALTIME, &target);
		if (rc!=OK) {
			return BERR_TRACE(BERR_OS_ERROR);
		}
	}
    
	for(;;) {
		if (!forever) {
			timeout = BKNI_ConvertMillisecondsToTicks(timeoutMsec);
		}
		*nevents = group_get_events(group, events, max_events);
		if (*nevents) {
			return BERR_SUCCESS;
		}
    	rc = semTake(group->event, timeout);
		if (rc==OK) {
			continue;
		} else if (errno == S_objLib_OBJ_TIMEOUT ) {
			*nevents = 0;
			return BERR_TIMEOUT;
		} else {
			return BERR_TRACE(BERR_OS_ERROR);
		}
		/* false wakeup */
		if (!forever) {
			rc = clock_gettime(CLOCK_REALTIME, &now);
			if (rc!=OK) {
				return BERR_TRACE(BERR_OS_ERROR);
			}
            if (now.tv_sec > target.tv_sec || (now.tv_sec == target.tv_sec && now.tv_nsec >= target.tv_nsec)) {
                *nevents = 0;
                return BERR_TIMEOUT;
            }
            timeoutMsec = 1000*(target.tv_sec - now.tv_sec) + (target.tv_nsec/1000 - now.tv_nsec/1000)/1000000;
        }
    }
}

