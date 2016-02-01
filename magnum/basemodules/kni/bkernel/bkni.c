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
 *
 * Module Description:
 *
 * Implementatation of the Magnum KNI for bkernel.
 *
 * Revision History:
 *
 * $brcm_Log: $ *
 * 
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "../generic/bkni_multi.h"
#include "bkni_metrics.h"
#include "blst_list.h"
#include "bkernel.h"
#include "ministd.h"

BDBG_MODULE(kernelinterface);

BDBG_OBJECT_ID(BKNI_Event);
BDBG_OBJECT_ID(BKNI_Mutex);

struct BKNI_MutexObj
{
    BDBG_OBJECT(BKNI_Mutex)
    bool locked;
};

struct BKNI_EventObj
{
    BDBG_OBJECT(BKNI_Event)
    bool signal;
};

#define ASSERT_CRITICAL() (void)0
#define ASSERT_NOT_CRITICAL() (void)0
#define SET_CRITICAL() (void)0
#define CLEAR_CRITICAL() (void)0
#define CHECK_CRITICAL() 0

#define B_TRACK_ALLOC_LOCK()
#define B_TRACK_ALLOC_UNLOCK() 
#define B_TRACK_ALLOC_ALLOC(size) malloc(size)
#define B_TRACK_ALLOC_FREE(ptr) free(ptr)
#define B_TRACK_ALLOC_OS "bkernel"

#include "../linuxuser/bkni_track_mallocs.inc"


BERR_Code BKNI_Init(void)
{
    BKNI_P_TrackAlloc_Init();
    return BERR_SUCCESS;
}


/* coverity[+kill]  */
void BKNI_Fail(void)
{
    BKNI_Printf("BKNI_Fail is intentionally crashing. Please inspect any prior error messages or get a core dump stack trace to determine the cause of failure.\n");
    *(volatile unsigned char *)0;
}

#if 1
/* Mutex, Event and CriticalSection functions are deliberately not implemented, bkernel runs magnum code only in the single task context */
void
BKNI_EnterCriticalSection(void)
{
    return;
}

void
BKNI_LeaveCriticalSection(void)
{
    return;
}

#else
BERR_Code BKNI_CreateMutex(BKNI_MutexHandle *handle)
{
    ASSERT_NOT_CRITICAL();

    *handle = BKNI_Malloc(sizeof(struct BKNI_MutexObj));
    if ( NULL == *handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BDBG_OBJECT_SET(*handle, BKNI_Mutex);
    (*handle)->locked = false;
    return BERR_SUCCESS;
}

BERR_Code
BKNI_TryAcquireMutex(BKNI_MutexHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, BKNI_Mutex);

    if ( handle->locked )
    {
        return BERR_TIMEOUT;
    }
    else
    {
        handle->locked = true;
        return BERR_SUCCESS;
    }
}

BERR_Code
BKNI_AcquireMutex(BKNI_MutexHandle handle)
{
    ASSERT_NOT_CRITICAL();
    BDBG_OBJECT_ASSERT(handle, BKNI_Mutex);

    /* With no threads, we just need to make sure the object is not locked recursively. */
    BDBG_ASSERT(!handle->locked);
    handle->locked = true;

    return BERR_SUCCESS;
}

void
BKNI_ReleaseMutex(BKNI_MutexHandle handle)
{
    ASSERT_NOT_CRITICAL();
    BDBG_OBJECT_ASSERT(handle, BKNI_Mutex);

    /* With no threads, we just need to make sure the object is not locked recursively. */
    BDBG_ASSERT(handle->locked);
    handle->locked = false;
}

void
BKNI_DestroyMutex(BKNI_MutexHandle handle)
{
    ASSERT_NOT_CRITICAL();
    BDBG_OBJECT_ASSERT(handle, BKNI_Mutex);

    BKNI_Free(handle);
}



BERR_Code
BKNI_CreateEvent(BKNI_EventHandle *pEvent)
{
    ASSERT_NOT_CRITICAL();

    *pEvent = BKNI_Malloc(sizeof(struct BKNI_EventObj));
    if ( NULL == *pEvent )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BDBG_OBJECT_SET(*pEvent, BKNI_Event);
    (*pEvent)->signal = false;

    return BERR_SUCCESS;
}

void
BKNI_DestroyEvent(BKNI_EventHandle event)
{
    ASSERT_NOT_CRITICAL();
    BDBG_OBJECT_ASSERT(event, BKNI_Event);

    BKNI_Free(event);
}

BERR_Code
BKNI_WaitForEvent(BKNI_EventHandle event, int timeoutMsec)
{
    struct timeval target, now;

    /* The only thing that can set an event in this system is a prior task call or an isr.  There are no threads. */
    BDBG_OBJECT_ASSERT(event, BKNI_Event);
    
    if ( timeoutMsec == BKNI_INFINITE )
    {
        /* Wait effectively forever */
        target.tv_sec = 0x7fffffff;
        target.tv_usec = 999999;
    }
    else
    {
        gettimeofday(&target, NULL);
        target.tv_sec += timeoutMsec / 1000;
        target.tv_usec += (timeoutMsec % 1000) * 1000;
        if ( target.tv_usec > 1000000 )
        {
            target.tv_sec++;
            target.tv_usec -= 1000000;
        }
    }

    for ( ;; )
    {
        if ( event->signal )
        {
            event->signal = false;
            return BERR_SUCCESS;
        }
        
        gettimeofday(&now, NULL);

        /* Calc diff in ms */
        if ( now.tv_usec > target.tv_usec )
        {
            target.tv_sec -= 1;
            target.tv_usec += 1000000;
        }
        timeoutMsec = 1000 * (target.tv_sec - now.tv_sec);
        timeoutMsec += (999 + target.tv_usec - now.tv_usec)/1000;

        if ( timeoutMsec <= 0 )
        {
            return BERR_TIMEOUT;
        }

        B_Kernel_WaitForInterrupt();
    }
}

void
BKNI_SetEvent(BKNI_EventHandle event)
{
    BDBG_OBJECT_ASSERT(event, BKNI_Event);
    event->signal = true;
}

void
BKNI_ResetEvent(BKNI_EventHandle event)
{
    BDBG_OBJECT_ASSERT(event, BKNI_Event);
    event->signal = false;
}

void
BKNI_AssertIsrContext_isr(const char *filename, unsigned lineno)
{
    if ( !CHECK_CRITICAL() ) {
        BDBG_P_AssertFailed("Not in critical section", filename, lineno);
    }
}
#endif



void
BKNI_Delay(unsigned int microsec)
{
    B_Kernel_Delay(microsec);
}

BERR_Code
BKNI_Sleep(unsigned int millisec)
{
    ASSERT_NOT_CRITICAL();
    B_Kernel_Sleep(millisec);
    return BERR_SUCCESS;
}

void BKNI_Uninit(void)
{
    BKNI_P_TrackAlloc_Uninit();
    return;
}

