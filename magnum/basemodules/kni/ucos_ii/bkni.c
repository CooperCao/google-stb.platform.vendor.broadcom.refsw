/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description: ucos-ii bkni implementation. This module was implemented
 * to support ucos-ii v2.86. Use with version earlier than specified may lead
 * to undefined behavior and failure to compile.
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bkni_event_group.h"
#include "bdbg.h"
#include "blst_list.h"
#include "blst_slist.h"
#include "ucos_ii.h"
#include "os_cfg.h"
BDBG_MODULE(kni);

#define ASSERT_CRITICAL() (void)0
#define ASSERT_NOT_CRITICAL() (void)0
#define SET_CRITICAL() (void)0
#define CLEAR_CRITICAL() (void)0
extern unsigned char OSLockNesting;
extern unsigned char OSIntNesting;
#define CHECK_CRITICAL() ( bos_in_interrupt() || (OSLockNesting > 0) || (OSIntNesting > 0))
#define TICKS_TO_MS(ticks)	(ticks * 1000/OS_TICKS_PER_SEC)
#define MS_TO_TICKS(x)      ((x * OS_TICKS_PER_SEC)/ 1000)

/****************************************************************************
    Data Structures
****************************************************************************/
typedef struct BKNI_GroupObj {
    BLST_D_HEAD(group, BKNI_EventObj) members;
    OS_EVENT * pLock;
    BKNI_EventHandle hEvent;
} BKNI_GroupObj;

typedef struct BKNI_EventObj {
    BLST_S_ENTRY(BKNI_EventObj) link;
    BLST_D_ENTRY(BKNI_EventObj) list;
    OS_EVENT * pOsEvent;
    BKNI_GroupObj * group;
} BKNI_EventObj;

typedef struct BKNI_MutexObj {
    BLST_S_ENTRY(BKNI_MutexObj) link;
    OS_EVENT * pOsEvent;
} BKNI_MutexObj;

#define BKNI_MS_TO_TICKS(ms) ((ms + (1000/OS_TICKS_PER_SEC) - 1)/(1000/OS_TICKS_PER_SEC))
extern int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);

/* Create a new list head for our MUTEX free pool */
static BLST_S_HEAD(BKNI_P_MutexFreePool, BKNI_MutexObj) gMutexFreePool = BLST_S_INITIALIZER(gMutexFreePool); 
/* Create a new list head for our EVENT free pool */
static BLST_S_HEAD(BKNI_P_EventFreePool, BKNI_EventObj) gEventFreePool = BLST_S_INITIALIZER(gEventFreePool); 

uint32_t g_cp0_count_clocks_per_usec;
uint32_t g_cp0_count_clocks_per_sec;

/****************************************************************************
    Static function prototypes
****************************************************************************/
static void BKNI_P_PostSemaphore(OS_EVENT * pEvent);
static BERR_Code BKNI_P_GetTime(unsigned long *pulMsec);
/****************************************************************************
****************************************************************************/

BERR_Code BKNI_Init(void)
{
	return BERR_SUCCESS;
}

void BKNI_Uninit(void)
{
}

void *BKNI_Memset(void *mem, int ch, size_t n)
{
	return memset(mem, ch, n);
}

void *BKNI_Memcpy(void *dest, const void *src, size_t n)
{
	BDBG_ASSERT(src);
	BDBG_ASSERT(dest);
/*	BDBG_ASSERT(n); */
	return memcpy(dest, src, n);
}

int BKNI_Memcmp(
	const void *s1,		/* byte array to be compared */
	const void *s2,		/* byte array to be compared */
	size_t n			/* maximum number of bytes to be compared */
	)
{
	return memcmp(s1, s2, n);
}

/* Simple brute force implementation */
void *BKNI_Memchr(const void *mem, int ch, size_t n)
{
	while(!n--){
		if(*(unsigned char*)mem == (unsigned char)ch)
			return (void *)mem;
		mem = ((unsigned char*)mem) + 1;
	}
	/* not found */
	return 0;
}

void *BKNI_Memmove(
	void *dest,			/* destination byte array */
	const void *src,	/* source byte array */
	size_t n			/* number of bytes to copy */
	)
{
	/* our memcpy can copy overlapping memory */
	return memcpy(dest, src, n);
}
#ifndef _BKNI_PRINT_H_
int BKNI_Printf(
	const char *fmt, /* format string */
	...					/* variable arguments */
	)
{
	int ret;
	va_list args;
	va_start(args,fmt);
	ret = vprintf(fmt, args);
	va_end(args);
	return ret;
}

int BKNI_Vprintf(const char *fmt, va_list ap)
{
	return vprintf(fmt, ap);
}
#endif /* _BKNI_PRINT_H_ */

int BKNI_Snprintf(
	char *s,			/* destination string */
	size_t n,			/* size of memory that can be used. It should include
							space for the trailing null byte. */
	const char *fmt,	/* format string */
	...					/* variable arguments */
	)
{
	int ret;
	va_list args;
	va_start(args,fmt);
	ret =vsnprintf(s, n, fmt, args);
	va_end(args);
	return ret;
}

int BKNI_Vsnprintf(char *s, size_t n, const char *fmt, va_list ap)
{
	return vsnprintf(s, n, fmt, ap);
}

void BKNI_Delay(unsigned int microsec)
{
#define MICROSEC_PER_SEC 1000000
    // We can only handle a granularity of a timer tick, for now.

    int microsec_per_os_tick = MICROSEC_PER_SEC / OS_TICKS_PER_SEC;
    int timer_ticks = microsec / microsec_per_os_tick;
    int start, end;
    if (timer_ticks==0) timer_ticks=1;
    
    start = OSTimeGet();
    end = start + timer_ticks;
    while (1) {
        if (OSTimeGet() >= end) break;
    }
}

#if BDBG_DEBUG_BUILD
void *BKNI_Malloc_tagged(
	size_t size,
	const char *file,
	int line
	)
{
	return malloc_dbg(size,(char*)file,line);
}

void BKNI_Free_tagged(
	void *mem,			/* Pointer to memory allocated by BKNI_Malloc */
	const char *file,
	int line
	)
{
	free_dbg(mem, (char*)file, line);
	return;
}

void BKNI_DumpMallocs(void)
{
/*	m_report(); */
}
#else
void *BKNI_Malloc(int size)
{
	return malloc(size);
}

void BKNI_Free(	void *mem)
{
	return free(mem);
}
#endif

BERR_Code BKNI_Sleep(
	unsigned int millisec	/* minimum number of milliseconds to sleep */
	)
{
    unsigned int ticks = MS_TO_TICKS(millisec);

    if (ticks <= 0)
    {
        ticks = 1;
    }
    OSTimeDly(ticks);
	return BERR_SUCCESS;
}

/****************************************************************************
****************************************************************************/
BERR_Code 
BKNI_CreateEvent(BKNI_EventHandle * event)
{
    BKNI_EventObj * pEventObj;
    bool bAllocNewOsEvent = false;
	OS_CPU_SR  cpu_sr = 0;

    ASSERT_NOT_CRITICAL();

    *event = NULL;

    /* First, check our free pool */
    OS_ENTER_CRITICAL();
    pEventObj = BLST_S_FIRST(&gEventFreePool);
    if (pEventObj == NULL) {
        OS_EXIT_CRITICAL();
        bAllocNewOsEvent = true;
        pEventObj = (BKNI_EventObj *)BKNI_Malloc(sizeof(BKNI_EventObj));
    } else {
        BLST_S_REMOVE_HEAD(&gEventFreePool, link);
        OS_EXIT_CRITICAL();
    }

    if (pEventObj == NULL) {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    if (bAllocNewOsEvent == true) {
        pEventObj->pOsEvent = OSSemCreate(0);
        if (pEventObj->pOsEvent == NULL) {
            BKNI_Free(pEventObj);
            return BERR_TRACE(BERR_OS_ERROR);
        }
    } else {
        /* Initialize the Sem count for this event. We don't have to do this
           in a critical section because the sem isn't being used right now 
        */
        pEventObj->pOsEvent->OSEventCnt = 0;
    }

    pEventObj->group = NULL;

    *event = pEventObj;
    
    return BERR_SUCCESS;
}

/****************************************************************************
****************************************************************************/
void
BKNI_DestroyEvent(BKNI_EventHandle event)
{
	OS_CPU_SR  cpu_sr = 0;
    ASSERT_NOT_CRITICAL();

    if (event) {
        OS_ENTER_CRITICAL();
        BLST_S_INSERT_HEAD(&gEventFreePool, (BKNI_EventObj *)event, link);
        OS_EXIT_CRITICAL();
    }
    
    return;
}

/****************************************************************************
****************************************************************************/
BERR_Code 
BKNI_WaitForEvent(BKNI_EventHandle event, int timeoutMsec)
{
    BERR_Code   errCode = BERR_SUCCESS;
    unsigned char       ucosErr;
	OS_CPU_SR  cpu_sr = 0;
    
    int ticks;

    if (!event || !(event->pOsEvent)) {
        return BERR_INVALID_PARAMETER;
    }
    
    if (timeoutMsec != 0) {
        ASSERT_NOT_CRITICAL();
    }

    /* According to PI documentation, a timeout of 0 tells us to 
       check for an event signal and return immediately. We're not supposed
       to let it get scheduled because an ISR is allowed to call this
       ONLY if the timeout is 0.
    */
    if (timeoutMsec == 0) {
        OS_ENTER_CRITICAL();
        if (event->pOsEvent->OSEventCnt) {
            event->pOsEvent->OSEventCnt--;
            errCode = BERR_SUCCESS;
        } else {
            errCode = BERR_TIMEOUT;
        }
        OS_EXIT_CRITICAL();
        return errCode;
    }

    if (timeoutMsec == BKNI_INFINITE) {
        ticks = 0;
    } else {
        if (timeoutMsec < 30) {
            /* NOTE: The Linux kernel kni interface a 30ms minimum timeout -- why? */
            timeoutMsec = 30;
        }
        /* This macro forces us to round up to the next 10ms */
        ticks = BKNI_MS_TO_TICKS(timeoutMsec);
    }
    OSSemPend(event->pOsEvent, ticks, &ucosErr); 

    switch( ucosErr ) {
        case OS_ERR_NONE: 
            break;
        case OS_ERR_TIMEOUT:
            errCode = BERR_TIMEOUT;
            break;
        default:
            errCode = BERR_OS_ERROR;
            break;
    }

    return errCode;
}

/****************************************************************************
    By current definition, BKNI_SetEvent can be called from both task and 
    interrupt context (interrupt means ISR or CriticalSection). Unfortunately,
    BKNI_SetEvent_isr is permanently mapped to BKNI_SetEvent.
    
    Since this is the case, we need to add logic to know whether or not we are 
    inside a critical section so we can process accordingly. 
    
    NOTE: I do not expect BKNI_SetEvent to be called from real interrupt 
    context (as opposed to our high-priority ISR task) because I have no way
    of knowing we're actually running in interrupt context. I only know we're
    in a CriticalSection, which we enter before we call Magnum ISRs. 
****************************************************************************/
static void
BKNI_P_SetEvent_isr(BKNI_EventHandle event)
{
    INT8U ucosStatus;

    if (event && event->pOsEvent) {
        BKNI_GroupObj *group = event->group;

        /* We're in ISR context (either a real ISR or a CriticalSection), so 
           the OS scheduler is already being held. Don't attempt to lock it again.
        */
        /* prevent the event count saturating */
        ucosStatus = OSSemPost(event->pOsEvent);
        if (event->pOsEvent->OSEventCnt > 1) {
            event->pOsEvent->OSEventCnt = 1;
        }

        if (ucosStatus != OS_ERR_NONE) {
            ucosStatus = BERR_TRACE(ucosStatus);
        }
        /* If this event is part of a group, signal the group event */
        if (group) {
            BKNI_P_SetEvent_isr(group->hEvent);
        }
    }
}

void
BKNI_SetEvent(BKNI_EventHandle event)
{
    unsigned char ucosError;
    INT8U ucosStatus;

    /* */
    if (CHECK_CRITICAL()) {
        BKNI_P_SetEvent_isr(event);
        return;
    }
    
    if (event && event->pOsEvent) {
        BKNI_GroupObj *group = event->group;
        if (group) {
            OSSemPend(group->pLock, 0, &ucosError); 
            if (ucosError != OS_ERR_NONE) {
                ucosError = BERR_TRACE(ucosError);
            }
        }
        /* Hold the the scheduler while we prevent the event count saturating */
        OSSchedLock();
        ucosStatus = OSSemPost(event->pOsEvent);
        if (event->pOsEvent->OSEventCnt > 1) {
            event->pOsEvent->OSEventCnt = 1;
        }
        OSSchedUnlock();
        /* event may now be stale.... */
        if (ucosStatus != OS_ERR_NONE) {
            ucosStatus = BERR_TRACE(ucosStatus);
        }
        if (group) {
            BKNI_SetEvent(group->hEvent);
            BKNI_P_PostSemaphore(group->pLock);
        }
    }
}

/****************************************************************************
****************************************************************************/
void
BKNI_ResetEvent(BKNI_EventHandle event)
{
    BDBG_ASSERT((((int) event) % 4) == 0);
    BDBG_ASSERT((((int) event->pOsEvent) % 4) == 0);
    if (event && event->pOsEvent) {
        event->pOsEvent->OSEventCnt = 0;
    }
}
/*
  We implement BKNI_Fail by executing jtag or software breakpoint
  instruction, causing system to drop in to the debugger. 
 */
#define BREAK_TYPE_SOFT 1
#define BREAK_TYPE_JTAG 2
#define BREAK_TYPE BREAK_TYPE_JTAG
void BKNI_Fail(void)
{
#ifdef GHS
        /* todo */
#else    
#if BREAK_TYPE == BREAK_TYPE_SOFT
	__asm__("break");
#else
	__asm__("sdbbp");
#endif
#endif
}

struct critical_stance_t {
	OS_CPU_SR cpu_sr;
	unsigned int ref_count;
};

struct critical_stance_t cs = {OS_ENABLE, 0};

void BKNI_EnterCriticalSection(void)
{
	OS_CPU_SR  cpu_sr = 0;
	OSSchedLock();
	OS_ENTER_CRITICAL();
	cs.cpu_sr = cpu_sr;
	cs.ref_count++;
}

void BKNI_LeaveCriticalSection(void)
{
	OS_CPU_SR  cpu_sr = cs.cpu_sr;
	cs.ref_count--;
	OS_EXIT_CRITICAL();
	OSSchedUnlock();
}

void BKNI_AssertIsrContext_isr(const char *filename, unsigned lineno)
{
	BSTD_UNUSED(filename);
	BSTD_UNUSED(lineno);
	if((cs.ref_count == 0) && !bos_in_interrupt()){
		BKNI_Fail();
	}
}

/****************************************************************************
    Don't use BDBG_ messages in BKNI_CreateMutex -- we get called during
    KNI & DBG init.
****************************************************************************/
BERR_Code 
BKNI_CreateMutex(BKNI_MutexHandle *mutex)
{
    BKNI_MutexObj * pMutexObj;
    bool bAllocNewOsEvent = false;
	OS_CPU_SR  cpu_sr = 0;

    ASSERT_NOT_CRITICAL();

    *mutex = NULL;

    /* First, check our free pool */
    OS_ENTER_CRITICAL();
    pMutexObj = BLST_S_FIRST(&gMutexFreePool);
    if (pMutexObj == NULL) {
        OS_EXIT_CRITICAL();
        bAllocNewOsEvent = true;
        pMutexObj = (BKNI_MutexObj *)BKNI_Malloc(sizeof(BKNI_MutexObj));
    } else {
        BLST_S_REMOVE_HEAD(&gMutexFreePool, link);
        OS_EXIT_CRITICAL();
    }

    if (pMutexObj == NULL) {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    if (bAllocNewOsEvent == true) {
        pMutexObj->pOsEvent = OSSemCreate(1);
        if (pMutexObj->pOsEvent == NULL) {
            BKNI_Free(pMutexObj);
            return BERR_TRACE(BERR_OS_ERROR); 
        }
    } else {
        /* Initialize the Sem count for this mutex. We don't have to do this
           in a critical section because the sem isn't being used right now 
        */
        pMutexObj->pOsEvent->OSEventCnt = 1;
    }

    *mutex = (BKNI_MutexHandle)pMutexObj;

    return BERR_SUCCESS;
}

/****************************************************************************
****************************************************************************/
void
BKNI_DestroyMutex(BKNI_MutexHandle mutex)
{
	OS_CPU_SR  cpu_sr = 0;
    ASSERT_NOT_CRITICAL();

    /* uCos doesn't have a way to destroy the semaphore. Add it to our free pool */
    if (mutex) {
        OS_ENTER_CRITICAL();
        BLST_S_INSERT_HEAD(&gMutexFreePool, (BKNI_MutexObj *)mutex, link);
        OS_EXIT_CRITICAL();
        mutex = NULL;
    }

    return;
}

/****************************************************************************
****************************************************************************/
BERR_Code 
BKNI_TryAcquireMutex(BKNI_MutexHandle mutex)
{
    BERR_Code errCode = BERR_SUCCESS;
	OS_CPU_SR  cpu_sr = 0;

    ASSERT_NOT_CRITICAL();

    if (!mutex || !(mutex->pOsEvent)) {
        return BERR_INVALID_PARAMETER;
    }

    /* See if we can acquire this mutex. If so, grab it and go. 
       If not, return a timeout error. Don't block.
    */
    OS_ENTER_CRITICAL();
    if (mutex->pOsEvent->OSEventCnt) {
        /* We know it's available -- grab it */
        mutex->pOsEvent->OSEventCnt--;
        errCode = BERR_SUCCESS;
    } else {
        errCode = BERR_TIMEOUT;
    }
    OS_EXIT_CRITICAL();

    return errCode;
}

/****************************************************************************
****************************************************************************/
BERR_Code 
BKNI_AcquireMutex(BKNI_MutexHandle mutex)
{
    unsigned char       ucosErr;
    BERR_Code   errCode = BERR_SUCCESS;

    ASSERT_NOT_CRITICAL();

    if (!mutex || !(mutex->pOsEvent)) {
        return BERR_INVALID_PARAMETER;
    }

    /* Wait forever on this mutex */
    OSSemPend(mutex->pOsEvent, 0, &ucosErr); 

    switch( ucosErr ) {
        case OS_ERR_NONE: 
            break;
        case OS_ERR_TIMEOUT:
            errCode = BERR_TIMEOUT;
            break;
        default:
            errCode = BERR_OS_ERROR;
            break;
    }
    
    return errCode;
}

/****************************************************************************
****************************************************************************/
void
BKNI_ReleaseMutex(BKNI_MutexHandle mutex)
{
    ASSERT_NOT_CRITICAL();

    if (mutex && mutex->pOsEvent) {
        BKNI_P_PostSemaphore(mutex->pOsEvent);
    }
}

/****************************************************************************
    Event Group Functions
****************************************************************************/
BERR_Code 
BKNI_CreateEventGroup(BKNI_EventGroupHandle *pGroup)
{
    BKNI_EventGroupHandle group;
    BERR_Code result = BERR_SUCCESS;

    group = (BKNI_EventGroupHandle)BKNI_Malloc(sizeof(*group));
    if (!group) {
        result = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_no_memory;
    }
    BLST_D_INIT(&group->members);
    
    group->pLock = OSSemCreate(1);
    if (group->pLock == NULL) {
        result = BERR_TRACE(BERR_OS_ERROR);
        goto err_mutex;
    }
    result = BKNI_CreateEvent(&(group->hEvent));
    if (result != BERR_SUCCESS) {
        result = BERR_TRACE(result);
        goto err_event;
    }

    *pGroup = group;

    return BERR_SUCCESS;

err_event:
    /* uCOS does not have a method to destroy a semaphore */
    BDBG_ERR(("%s: uCOS Event Leak - no method to destroy semaphore", BSTD_FUNCTION));
err_mutex:
    BKNI_Free((void *)group);
err_no_memory:
    return result;
}

/****************************************************************************
****************************************************************************/
void
BKNI_DestroyEventGroup(BKNI_EventGroupHandle group)
{
    unsigned char ucosError;
    BKNI_EventHandle event;

    OSSemPend(group->pLock, 0, &ucosError); 
    if (ucosError != OS_ERR_NONE) {
        ucosError = BERR_TRACE(ucosError);
        BDBG_ASSERT(false);
    }
    while( NULL != (event=BLST_D_FIRST(&group->members)) ) {
        event->group = NULL;
        BLST_D_REMOVE_HEAD(&group->members, list);
    }
    BKNI_P_PostSemaphore(group->pLock);
    BDBG_ERR(("%s: uCOS Event Leak - no method to destroy semaphore", BSTD_FUNCTION));
    BKNI_DestroyEvent(group->hEvent);
    BKNI_Free((void *)group);

    return;
}

/****************************************************************************
****************************************************************************/
BERR_Code
BKNI_AddEventGroup(BKNI_EventGroupHandle group, BKNI_EventHandle event)
{
    unsigned char ucosError;
    BERR_Code result = BERR_SUCCESS;
	OS_CPU_SR  cpu_sr = 0;

    /* IMPORTANT: group lock shall be acquired before event lock */
    OSSemPend(group->pLock, 0, &ucosError); 
    if (ucosError != OS_ERR_NONE) {
        ucosError = BERR_TRACE(ucosError);
        BDBG_ASSERT(false);
    }
    if (event->group != NULL) {
        BDBG_ERR(("Event %#x already connected to group %#x", (unsigned)event, (unsigned)group));
        result = BERR_TRACE(BERR_OS_ERROR);
    } else {
        bool bIsSet;
        BLST_D_INSERT_HEAD(&group->members, event, list);
        event->group = group;
        OS_ENTER_CRITICAL();
        bIsSet = event->pOsEvent->OSEventCnt > 0;
        OS_EXIT_CRITICAL();
        if (bIsSet) { /* kinda racey */
            BKNI_SetEvent(group->hEvent);
        }
    }
    BKNI_P_PostSemaphore(group->pLock);

    return result;
}

/****************************************************************************
****************************************************************************/
BERR_Code
BKNI_RemoveEventGroup(BKNI_EventGroupHandle group, BKNI_EventHandle event)
{
    unsigned char ucosError;
    BERR_Code result = BERR_SUCCESS;

    OSSemPend(group->pLock, 0, &ucosError); 
    if (ucosError != OS_ERR_NONE) {
        ucosError = BERR_TRACE(ucosError);
        BDBG_ASSERT(false);
    }
    if (event->group != group) {
        BDBG_ERR(("Event %#x doesn't belong to group %#x", event, group));
        result = BERR_TRACE(BERR_OS_ERROR);
    } else {
        BLST_D_REMOVE(&group->members, event, list);
        event->group = NULL;
    }
    BKNI_P_PostSemaphore(group->pLock);

    return result;
}

/****************************************************************************
****************************************************************************/
static unsigned
group_get_events(BKNI_EventGroupHandle group, BKNI_EventHandle *events, unsigned max_events)
{
    unsigned char ucosError;
    BKNI_EventHandle ev;
    unsigned event;
	OS_CPU_SR  cpu_sr = 0;

    OSSemPend(group->pLock, 0, &ucosError); 
    if (ucosError != OS_ERR_NONE) {
        ucosError = BERR_TRACE(ucosError);
        BDBG_ASSERT(false);
    }
    for(event=0, ev=BLST_D_FIRST(&group->members); ev && event<max_events ; ev=BLST_D_NEXT(ev, list)) {
        bool bIsSet;
        OS_ENTER_CRITICAL();
        if ((bIsSet = (ev->pOsEvent->OSEventCnt > 0)) == true) {
            ev->pOsEvent->OSEventCnt = 0;
        }
        OS_EXIT_CRITICAL();
        if (bIsSet) {
            events[event] = ev;
            event++;
        }
    }
    BKNI_P_PostSemaphore(group->pLock);

    return event;
}

/****************************************************************************
****************************************************************************/
BERR_Code
BKNI_WaitForGroup(BKNI_EventGroupHandle group, int timeoutMsec, BKNI_EventHandle *events, unsigned max_events, unsigned *nevents)
{
    BERR_Code result;
    unsigned long target, now;
    bool forever = false;

    ASSERT_NOT_CRITICAL();

    if (max_events < 1) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if (timeoutMsec == BKNI_INFINITE) {
        forever = true;
    } else if (timeoutMsec == 0) {
        BKNI_WaitForEvent(group->hEvent, 0); /* just clear event */
        *nevents = group_get_events(group, events, max_events);
        return *nevents ? BERR_SUCCESS : BERR_TIMEOUT;
    } else {
        BKNI_P_GetTime(&target);
        target += timeoutMsec;
    }

    for(;;) {
        /* Make sure we clear the group's event status by calling WaitForEvent.
           Other OS ports check their event list immediately, but they don't 
           use an Event for the group itself.
        */
        result = BKNI_WaitForEvent(group->hEvent, forever ? BKNI_INFINITE : timeoutMsec);
        if (result == BERR_SUCCESS) {
            *nevents = group_get_events(group, events, max_events);
            if (*nevents) {
                return BERR_SUCCESS;
            }
        } else if (result == BERR_TIMEOUT) {
            *nevents = 0;
            return BERR_TIMEOUT;
        } else {
            return BERR_TRACE(BERR_OS_ERROR);
        }
        /* false wakeup */
        if (!forever) {
            result = BKNI_P_GetTime(&now);
            if (result != BERR_SUCCESS) {
                return BERR_TRACE(BERR_OS_ERROR);
            }
            if ((int)(now - target) >= 0) {
                *nevents = 0;
                return BERR_TIMEOUT;
            }
            timeoutMsec = target - now;
        }
    }
}

/****************************************************************************
    Provide an internal method for releasing a "mutex" (really a uCOS
    semaphore). Internal functions can't use the external BKNI interface
    now that we've added additional checks for validity.
    
    Since we're using a semaphore, make sure the count never gets higher 
    than 1. If it does, it means we released the mutex more times than it 
    was acquired
****************************************************************************/
static void
BKNI_P_PostSemaphore(OS_EVENT * pEvent)
{
    uint32_t mutexCount;
	OS_CPU_SR  cpu_sr = 0;

    /* Since we're using a semaphore, make sure the count never
       gets higher than 1. If it does, it means we released the
       mutex more times than it was acquired
    */
    OS_ENTER_CRITICAL();
    mutexCount = pEvent->OSEventCnt;
    OS_EXIT_CRITICAL();
    if (mutexCount > 0) {
        /* Mutex already posted. Don't do it again */
        mutexCount = BERR_TRACE(mutexCount);
    } else {
        OSSemPost(pEvent);
    }
}


/****************************************************************************
****************************************************************************/
static BERR_Code 
BKNI_P_GetTime(unsigned long *pulMsec)
{
    unsigned long  ticks;
    ticks = (unsigned long)OSTimeGet();
    *pulMsec  = (unsigned long)TICKS_TO_MS(ticks);
    return BERR_SUCCESS;
}
