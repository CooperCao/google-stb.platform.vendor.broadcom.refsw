/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Description:
*   API name: Scheduler
*    Library routines for timer and thread event callback management
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "b_os_lib.h"
#include "b_os_priv.h"
#include "blst_slist.h"
#include "blst_list.h"
#include "blst_squeue.h"
#include "blst_queue.h"
#ifdef UCOS_DIAGS
	#include "ucos_ii.h"
#endif

BDBG_MODULE(b_os_scheduler);

typedef struct B_SchedulerEvent
{
    BLST_S_ENTRY(B_SchedulerEvent) node;    /* Entry into event list */
    B_EventHandle event;                    /* Event Handle */
    B_MutexHandle mutex;                    /* Mutex to lock for callback */
    void (*pCallback)(void *);              /* Callback routine */
    void *pContext;                         /* Callback parameter */
    bool deleted;                           /* Set to true when object is pending deletion */
} B_SchedulerEvent;

typedef struct B_SchedulerTimer
{
    BLST_S_ENTRY(B_SchedulerTimer) node;    /* Entry into timer list */
    B_Time time;                         /* Expiration time value */
    B_MutexHandle mutex;                    /* Mutex to lock for callback */
    void (*pCallback)(void *);              /* Callback routine */
    void *pContext;                         /* Callback parameter */
    bool deleted;                           /* Set to true when object is pending deletion */
} B_SchedulerTimer;

typedef struct B_SchedulerCallback
{
    struct B_Scheduler *pScheduler;                         /* Pointer back to parent */
    bool queued;                                            /* true if callback is in the active list */
    bool armed;                                             /* true if callback has been fired */
    bool stopped;                                           /* true if callback has been stopped */
    void *pInstanceHandle;                                  /* Object instance associated w/this callback */
    B_SchedulerCallbackFunction pFunction;                  /* Callback routine */
    void *pFuncParam1;                                      /* First parameter to callback */
    int funcParam2;                                         /* Second parameter to callback */
    BLST_S_ENTRY(B_SchedulerCallback) callbackNode;         /* Node into scheduler's callback list */
    BLST_SQ_ENTRY(B_SchedulerCallback) activeCallbackNode;  /* Node into scheduler's active callback list */
    bool deleted;                                           /* Set to true when object is pending deletion */
} B_SchedulerCallback;

typedef enum B_SchedulerState
{
    B_Scheduler_eRunning,
    B_Scheduler_eStopping,
    B_Scheduler_eStopped
} B_SchedulerState;

/***************************************************************************
Summary:
Scheduler Handle

Description:
A Scheduler is a higher-level OS construct.  A scheduler is responsible for
dispatching synchronized timer and event callbacks using a single thread.
***************************************************************************/
typedef struct B_Scheduler
{
    B_MutexHandle mutex;
    B_MutexHandle callbackMutex;
    B_EventHandle controlEvent;
    B_EventGroupHandle eventGroup;

    bool timerChanged;
    bool callbackChanged;

    B_SchedulerState state;

    BLST_S_HEAD(eventList, B_SchedulerEvent) eventList;
    BLST_S_HEAD(timerList, B_SchedulerTimer) timerList;
    BLST_S_HEAD(callbackList, B_SchedulerCallback) callbackList;
    BLST_SQ_HEAD(activeList, B_SchedulerCallback) activeCallbackList;
} B_Scheduler;

/***************************************************************************
Summary:
Get Default Scheduler Settings
***************************************************************************/
void B_Scheduler_GetDefaultSettings(
    B_SchedulerSettings *pSettings      /* [out] */
    )
{
    BSTD_UNUSED(pSettings);     /* Empty Structure */
}

/***************************************************************************
Summary:
Craete a scheduler
***************************************************************************/
B_SchedulerHandle B_Scheduler_Create(
    const B_SchedulerSettings *pSettings
    )
{
    B_Scheduler *pScheduler;
    B_Error errCode;

    BSTD_UNUSED(pSettings);     /* Empty Structure */

    pScheduler = B_Os_Calloc(1, sizeof(B_Scheduler));
    if ( NULL == pScheduler )
    {
        errCode = BERR_TRACE(B_ERROR_OUT_OF_MEMORY);
        goto err_malloc;
    }

    pScheduler->mutex = B_Mutex_Create(NULL);
    if ( NULL == pScheduler->mutex )
    {
        errCode = BERR_TRACE(B_ERROR_OS_ERROR);
        goto err_mutex;
    }

    pScheduler->callbackMutex = B_Mutex_Create(NULL);
    if ( NULL == pScheduler->callbackMutex )
    {
        errCode = BERR_TRACE(B_ERROR_OS_ERROR);
        goto err_callback_mutex;
    }

    pScheduler->controlEvent = B_Event_Create(NULL);
    if ( NULL == pScheduler->controlEvent )
    {
        errCode = BERR_TRACE(B_ERROR_OS_ERROR);
        goto err_event;
    }

    pScheduler->eventGroup = B_EventGroup_Create(NULL);
    if ( NULL == pScheduler->eventGroup )
    {
        errCode = BERR_TRACE(B_ERROR_OS_ERROR);
        goto err_group;
    }

    /* Add control event to group */
    B_EventGroup_AddEvent(pScheduler->eventGroup, pScheduler->controlEvent);

    /* Success */
    return pScheduler;

err_group:
    B_Event_Destroy(pScheduler->controlEvent);
err_event:
    B_Mutex_Destroy(pScheduler->callbackMutex);
err_callback_mutex:
    B_Mutex_Destroy(pScheduler->mutex);
err_mutex:
    B_Os_Free(pScheduler);
err_malloc:
    return NULL;
}


/***************************************************************************
Summary:
Destroy a scheduler
***************************************************************************/
void B_Scheduler_Destroy(
    B_SchedulerHandle scheduler
    )
{
    BDBG_ASSERT(NULL != scheduler);
    BDBG_ASSERT(B_Scheduler_eRunning != scheduler->state); /* App should always stop scheduler before destroying to avoid races */

    while (B_Scheduler_eStopped != scheduler->state)
    {
        B_Thread_Sleep(10); /* wait for scheduler thread to stop gracefully */
    }

    B_Mutex_Lock(scheduler->mutex);
    {
        B_SchedulerTimer *pTimer;
        B_SchedulerEvent *pEvent;
        B_SchedulerCallback *pCallback;

        while ( (pTimer = BLST_S_FIRST(&scheduler->timerList)) )
        {
            BLST_S_REMOVE_HEAD(&scheduler->timerList, node);
            BDBG_WRN(("Cleaning up timer %p", (void*)pTimer));
            B_Os_Free(pTimer);
        }
        while ( (pEvent = BLST_S_FIRST(&scheduler->eventList)) )
        {
            BLST_S_REMOVE_HEAD(&scheduler->eventList, node);
            if ( !pEvent->deleted )
            {
                BDBG_WRN(("Cleaning up event %p", (void*)pEvent));
                B_EventGroup_RemoveEvent(scheduler->eventGroup, pEvent->event);
            }
            B_Os_Free(pEvent);
        }
        while ( (pCallback = BLST_S_FIRST(&scheduler->callbackList)) )
        {
            BLST_S_REMOVE_HEAD(&scheduler->callbackList, callbackNode);
            BDBG_WRN(("Cleaning up callback %p", (void*)pCallback));
            B_Os_Free(pCallback);
        }
    }
    B_Mutex_Unlock(scheduler->mutex);
    B_EventGroup_RemoveEvent(scheduler->eventGroup, scheduler->controlEvent);
    B_EventGroup_Destroy(scheduler->eventGroup);
    B_Event_Destroy(scheduler->controlEvent);
    B_Mutex_Destroy(scheduler->callbackMutex);
    B_Mutex_Destroy(scheduler->mutex);
    B_Os_Free(scheduler);
}

/***************************************************************************
Summary:
Register an event with a scheduler

Description:
This will cause the scheduler to call the sepecified EventCallback function
when the specified event has been set.  Prior to calling the callback,
the specified mutex will be automatically locked.  After the callback
returns, the mutex will be unlocked.
***************************************************************************/
B_SchedulerEventId B_Scheduler_RegisterEvent(
    B_SchedulerHandle scheduler,
    B_MutexHandle mutex,                        /* Mutex to lock prior to calling callback */
    B_EventHandle event,                        /* Event that will trigger the callback */
    B_EventCallback callback,                   /* Callback routine to execute when event is set */
    void *pContext                              /* Value passed to callback routine */
    )
{
    B_Error errCode;
    B_SchedulerEvent *pEvent;

    BDBG_ASSERT(NULL != scheduler);
    BDBG_ASSERT(NULL != mutex);
    BDBG_ASSERT(NULL != event);
    BDBG_ASSERT(NULL != callback);

    pEvent = B_Os_Malloc(sizeof(B_SchedulerEvent));
    if ( NULL == pEvent )
    {
        errCode = BERR_TRACE(B_ERROR_OUT_OF_MEMORY);
        return NULL;
    }

    pEvent->event = event;
    pEvent->mutex = mutex;
    pEvent->pCallback = callback;
    pEvent->pContext = pContext;
    pEvent->deleted = false;

    B_Mutex_Lock(scheduler->mutex);
    #if BDBG_DEBUG_BUILD
    /* Scan for duplicates */
    /* TODO: Should this be supported as it was in nexus? */
    {
        B_SchedulerEvent *pNode;
        for ( pNode = BLST_S_FIRST(&scheduler->eventList);
              NULL != pNode;
              pNode = BLST_S_NEXT(pNode, node) )
        {
            if ( pNode->event == event && !pNode->deleted )
            {
                BDBG_ERR(("Duplicate event registration.  An event can only be registered once per scheduler."));
                B_Mutex_Unlock(scheduler->mutex);
                B_Os_Free(pEvent);
                return NULL;
            }
        }
    }
    #endif
    B_EventGroup_AddEvent(scheduler->eventGroup, event);
    BLST_S_INSERT_HEAD(&scheduler->eventList, pEvent, node);
    B_Mutex_Unlock(scheduler->mutex);

    return pEvent;
}

/***************************************************************************
Summary:
Un-Register an event from the scheduler
***************************************************************************/
void B_Scheduler_UnregisterEvent(
    B_SchedulerHandle scheduler,
    B_SchedulerEventId eventId
    )
{
    B_SchedulerEvent *pNode;

    BDBG_ASSERT(NULL != scheduler);
    BDBG_ASSERT(NULL != eventId);

    B_Mutex_Lock(scheduler->mutex);

    for ( pNode = BLST_S_FIRST(&scheduler->eventList);
          NULL != pNode;
          pNode = BLST_S_NEXT(pNode, node) )
    {
        if ( pNode == eventId )
        {
            break;
        }
    }

    if ( NULL == pNode || pNode->deleted )
    {
        BDBG_ERR(("Event not registered with this scheduler"));
        BDBG_ASSERT(NULL != pNode);
        B_Mutex_Unlock(scheduler->mutex);
        return;
    }

    /* Remove from group */
    /* TODO: Should duplicate event registration be supported? */
    B_EventGroup_RemoveEvent(scheduler->eventGroup, eventId->event);

    /* Notify scheduler of change */
    eventId->deleted = true;
    B_Mutex_Unlock(scheduler->mutex);
    B_Event_Set(scheduler->controlEvent);
}

/***************************************************************************
Summary:
Schedule a timer

Description:
This will start a timer that will lock the specified mutex and call the
specified callback routine when the timer expires.  Timers are one-shot,
and they must be rescheduled to repeat.  Re-scheduling the timer from the
callback itself is allowed.  After the timer expires, the TimerId value
returned becomes invalid and should be discarded.
***************************************************************************/
B_SchedulerTimerId B_Scheduler_StartTimer(
    B_SchedulerHandle scheduler,
    B_MutexHandle mutex,                    /* Mutex to lock prior to calling callback */
    int timeoutMsec,                        /* Timer expiration time in msec */
    B_TimerCallback callback,               /* Callback to call when timer expires */
    void *pContext                          /* Value passed to callback routine */
    )
{
    B_Error errCode;
    B_SchedulerTimer *pTimer, *pNode, *pPrev=NULL;

    BDBG_ASSERT(NULL != scheduler);
    BDBG_ASSERT(NULL != mutex);
    BDBG_ASSERT(timeoutMsec >= 0);
    BDBG_ASSERT(NULL != callback);

    pTimer = B_Os_Malloc(sizeof(B_SchedulerTimer));
    if ( NULL == pTimer )
    {
        errCode = BERR_TRACE(B_ERROR_OUT_OF_MEMORY);
        return NULL;
    }

    pTimer->mutex = mutex;
    B_Time_Get(&pTimer->time);
    B_Time_Add(&pTimer->time, timeoutMsec);
    pTimer->pCallback = callback;
    pTimer->pContext = pContext;
    pTimer->deleted = false;

    B_Mutex_Lock(scheduler->mutex);
    {
        scheduler->timerChanged = true;

        for ( pNode = BLST_S_FIRST(&scheduler->timerList);
              NULL != pNode;
              pNode = BLST_S_NEXT(pNode, node) )
        {
            if ( B_Time_Diff(&pNode->time, &pTimer->time) > 0 )
            {
                break;
            }
            pPrev = pNode;
        }

        if ( NULL == pPrev )
        {
            BLST_S_INSERT_HEAD(&scheduler->timerList, pTimer, node);
            B_Event_Set(scheduler->controlEvent);   /* Immediately wake thread, timeout will change */
        }
        else
        {
            BLST_S_INSERT_AFTER(&scheduler->timerList, pPrev, pTimer, node);
        }
    }
    B_Mutex_Unlock(scheduler->mutex);

    return pTimer;
}

/***************************************************************************
Summary:
Cancel a timer

Description:
This will cancel a timer that was previously started.  If the timer has
already expired, this call is safe.  If the callback has actually been
called, this will generate a warning message.  If the timer has expired and
the callback is pending on the mutex, the timer will be discarded and no
warning will occur.
***************************************************************************/
void B_Scheduler_CancelTimer(
    B_SchedulerHandle scheduler,
    B_SchedulerTimerId timerId
    )
{
    B_SchedulerTimer *pNode, *pPrev=NULL;

    BDBG_ASSERT(NULL != scheduler);
    BDBG_ASSERT(NULL != timerId);

    B_Mutex_Lock(scheduler->mutex);
    {
        for ( pNode = BLST_S_FIRST(&scheduler->timerList);
              NULL != pNode;
              pNode = BLST_S_NEXT(pNode, node) )
        {
            if ( pNode == timerId )
            {
                break;
            }
            pPrev = pNode;
        }
        if ( NULL == pNode || pNode->deleted == true )
        {
            BDBG_ERR(("Cancelling stale timer %p", (void*)timerId));
            B_Mutex_Unlock(scheduler->mutex);
            return;
        }

        scheduler->timerChanged = true;
        timerId->deleted = true;            /* Actual work must be done by main thread to avoid races */
    }
    B_Mutex_Unlock(scheduler->mutex);
    B_Event_Set(scheduler->controlEvent);
}

/***************************************************************************
Summary:
Run a scheduler

Description:
This routine will drive the scheduler execution.  It will not return until
B_Scheduler_Stop is called.
***************************************************************************/
void B_Scheduler_Run(
    B_SchedulerHandle scheduler
    )
{
    BDBG_ASSERT(NULL != scheduler);

    B_Mutex_Lock(scheduler->mutex);

    while ( B_Scheduler_eRunning == scheduler->state )
    {
        int i;
        B_Error errCode;
        B_EventHandle pEvents[5];
        B_SchedulerEvent *pEvent;
        unsigned numEvents, eventsLeft;
        const int maxTries=4;
        long timeout = B_WAIT_FOREVER;

        /* First order of business.  Handle Expired timers */
        /* Mutex is held */
        for ( i = 0; i < maxTries; i++ )
        {
            B_Time currentTime;
            /* coverity[use_after_free] */
            B_SchedulerTimer *pFirstTimer = BLST_S_FIRST(&scheduler->timerList);
            if ( NULL == pFirstTimer )
            {
                timeout = B_WAIT_FOREVER;
                break;
            }
            B_Time_Get(&currentTime);
            timeout = B_Time_Diff(&pFirstTimer->time, &currentTime);
            if ( timeout<=0 || pFirstTimer->deleted )
            {
                if( !pFirstTimer->deleted )
                {
                    B_Mutex_Unlock(scheduler->mutex);
                    B_Mutex_Lock(pFirstTimer->mutex);
                    if ( !pFirstTimer->deleted )
                    {
                        pFirstTimer->pCallback(pFirstTimer->pContext);
                    }
                    B_Mutex_Unlock(pFirstTimer->mutex);
                    B_Mutex_Lock(scheduler->mutex);
                }
                if ( !scheduler->timerChanged )
                {
                    BDBG_ASSERT(pFirstTimer == BLST_S_FIRST(&scheduler->timerList));
                    BLST_S_REMOVE_HEAD(&scheduler->timerList, node);
                }
                else
                {
                    scheduler->timerChanged = false;
                    BLST_S_REMOVE(&scheduler->timerList, pFirstTimer, B_SchedulerTimer, node);
                }
                B_Os_Free(pFirstTimer);
            }
            else if ( timeout > 0 )
            {
                break;
            }
        }
        if ( i >= maxTries )
        {
            /* Clear timeout if we exceeded the max number of timers */
            timeout = 0;
        }

        /* Mutex still held here */
        for ( i = 0; i < maxTries; i++ )
        {
            B_SchedulerCallback *pCallback;
            pCallback = BLST_SQ_FIRST(&scheduler->activeCallbackList);
            if( NULL == pCallback )
            {
                break;
            }
            BDBG_ASSERT(pCallback->queued);
            pCallback->queued = false;
            BLST_SQ_REMOVE_HEAD(&scheduler->activeCallbackList, activeCallbackNode);
            if ( pCallback->deleted )
            {   
                /* coverity[use] */
                BLST_S_REMOVE(&scheduler->callbackList, pCallback, B_SchedulerCallback, callbackNode);
                B_Os_Free(pCallback);
                continue;
            }
            B_Mutex_Unlock(scheduler->mutex);       /* Unlock before acquiring other mutex */
            B_Mutex_Lock(scheduler->callbackMutex); /* This prevents racing with StopCallbacks */
            if ( pCallback->armed && !pCallback->stopped )
            {
                pCallback->armed = false;
                pCallback->pFunction(pCallback->pFuncParam1, pCallback->funcParam2);
            }
            B_Mutex_Unlock(scheduler->callbackMutex);
            B_Mutex_Lock(scheduler->mutex);
        }
        if( i == maxTries )
        {
            timeout = 0;
        }

        /* Mutex still held here -- need to yield before waiting on event group */
        B_Mutex_Unlock(scheduler->mutex);

        /* Wait for any event or a timeout */
        errCode = B_EventGroup_Wait(scheduler->eventGroup, timeout, pEvents, sizeof(pEvents)/sizeof(*pEvents), &numEvents);

        /* Acquire scheduler lock */
        B_Mutex_Lock(scheduler->mutex);
        if ( B_Scheduler_eRunning != scheduler->state )
        {
            break;
        }
        if ( errCode == B_ERROR_TIMEOUT )
        {
            #ifdef NO_OS_DIAGS
            return;
            #else
            continue;
            #endif
        }
        else if ( errCode != B_ERROR_SUCCESS )
        {
            BDBG_ERR(("Unexpected event group error %u.  Terminating.", errCode));
            break;
        }

        /* Event Processing */
        for ( eventsLeft=numEvents, pEvent=BLST_S_FIRST(&scheduler->eventList);
              NULL != pEvent && eventsLeft > 0;
              )
        {
            unsigned j;

            if ( pEvent->deleted )
            {
                B_SchedulerEvent *pDeletedEvent = pEvent;
                pEvent = BLST_S_NEXT(pEvent, node);
                /* coverity[use_after_free] */
                BLST_S_REMOVE(&scheduler->eventList, pDeletedEvent, B_SchedulerEvent, node);    /* TODO: This is inefficient but rarely called - consider saving prev */
                B_Os_Free(pDeletedEvent);
                continue;
            }
            /* For each node in the list, test if any of the set events match */
            for ( j=0; j<numEvents; j++ )
            {
                if ( pEvents[j] == pEvent->event )
                {
                    /* TODO: Consider allowing duplicate events as nexus did */
                    /* TODO: move "hot" entries to head of the list */
                    B_Mutex_Unlock(scheduler->mutex);
                    B_Mutex_Lock(pEvent->mutex);
                    if ( !pEvent->deleted )
                    {
                        pEvent->pCallback(pEvent->pContext);
                    }
                    B_Mutex_Unlock(pEvent->mutex);
                    B_Mutex_Lock(scheduler->mutex);
                    /* This value is decremented only for matching events.  */
                    /* As a side-effect, if another event (e.g. controlEvent) is set, the */
                    /* entire list will be walked and deleted items will be purged */
                    eventsLeft--;
                    /* Found a match, no need to continue searching this node.  Advance to the next */
                    break;
                }
            }
            /* TODO: Why was this conditional? -- seems inefficient if we've already found a match */
            /*if ( j==numEvents ) */
            {
                pEvent = BLST_S_NEXT(pEvent, node);
            }
        }
        /* End event Processing */

        /* Loop will continue.  Mutex is still locked */
        #ifdef NO_OS_DIAGS
        return;
        #endif
    }

    scheduler->state = B_Scheduler_eStopped;
    B_Mutex_Unlock(scheduler->mutex);
    #ifdef UCOS_DIAGS
        OSTaskDel(OS_PRIO_SELF);
    #endif
    /* Done */
}

/***************************************************************************
Summary:
Stop a scheduler

Description:
This routine will stop the scheduler execution.  After this is called,
B_Scheduler_Run will return.  This routine can be called by any thread,
including the scheduler thread itself.  It is not synchronized, so this call
may return before B_Scheduler_Run returns.
***************************************************************************/
void B_Scheduler_Stop(
    B_SchedulerHandle scheduler
    )
{
    BDBG_ASSERT(NULL != scheduler);
    scheduler->state = B_Scheduler_eStopping;
    B_Event_Set(scheduler->controlEvent);
}

/***************************************************************************
Summary:
Get default settings for a scheduler callback instance
***************************************************************************/
void B_SchedulerCallback_GetDefaultSettings(
    B_SchedulerCallbackSettings *pSettings  /* [out] */
    )
{
    BSTD_UNUSED(pSettings); /* Empty for now */
}

/***************************************************************************
Summary:
Register an asynchronous callback

Description:
This will cancel a timer that was previously started.  If the timer has
already expired, this call is safe.  If the callback has actually been
called, this will generate a warning message.  If the timer has expired and
the callback is pending on the mutex, the timer will be discarded and no
warning will occur.
***************************************************************************/
B_SchedulerCallbackHandle B_SchedulerCallback_Create(
    B_SchedulerHandle scheduler,
    void *pInstanceHandle,                          /* Pass the handle to the application object's instance here */
    const B_SchedulerCallbackSettings *pSettings    /* Pass NULL for defaults */
    )
{
    B_Error errCode;
    B_SchedulerCallback *pCallback;

    BDBG_ASSERT(NULL != scheduler);
    BDBG_ASSERT(NULL != pInstanceHandle);

    BSTD_UNUSED(pSettings); /* Empty for now */

    pCallback = B_Os_Calloc(1, sizeof(B_SchedulerCallback));
    if ( NULL == pCallback )
    {
        errCode = BERR_TRACE(B_ERROR_OUT_OF_MEMORY);
        return NULL;
    }

    pCallback->pScheduler = scheduler;
    pCallback->pInstanceHandle = pInstanceHandle;

    B_Mutex_Lock(scheduler->mutex);
    {
        BLST_S_INSERT_HEAD(&scheduler->callbackList, pCallback, callbackNode);
    }
    B_Mutex_Unlock(scheduler->mutex);

    return pCallback;
}

/***************************************************************************
Summary:
Set the function to be used in an asynchronous callback
***************************************************************************/
B_Error B_SchedulerCallback_SetFunction(
    B_SchedulerCallbackHandle callbackHandle,
    B_SchedulerCallbackFunction callbackFunction,   /* Function to be called */
    void *pParam1,                                  /* First function parameter */
    int param2                                      /* Second function parameter */
    )
{
    BDBG_ASSERT(NULL != callbackHandle);
    BDBG_ASSERT(NULL != callbackFunction);

    B_Mutex_Lock(callbackHandle->pScheduler->mutex);
    {
        callbackHandle->pFunction = callbackFunction;
        callbackHandle->pFuncParam1 = pParam1;
        callbackHandle->funcParam2 = param2;
    }
    B_Mutex_Unlock(callbackHandle->pScheduler->mutex);

    return B_ERROR_SUCCESS;
}

/***************************************************************************
Summary:
Fire an asynchronous callback

Description:
This will schedule the callback to be executed when the scheduler becomes
idle.  If the same callback is fired multiple times before it is able to
be called, only a single application callback will be made.
***************************************************************************/
void B_SchedulerCallback_Fire(
    B_SchedulerCallbackHandle callback
    )
{
    BDBG_ASSERT(NULL != callback);

    B_Mutex_Lock(callback->pScheduler->mutex);
    {
        if ( !callback->queued )
        {
            BLST_SQ_INSERT_TAIL(&callback->pScheduler->activeCallbackList, callback, activeCallbackNode);
            callback->queued = true;
        }
        callback->armed = true;
    }
    B_Mutex_Unlock(callback->pScheduler->mutex);
    B_Event_Set(callback->pScheduler->controlEvent);
}

/***************************************************************************
Summary:
Destroying an asynchronous callback
***************************************************************************/
void B_SchedulerCallback_Destroy(
    B_SchedulerCallbackHandle callback
    )
{
    BDBG_ASSERT(NULL != callback);

    B_Mutex_Lock(callback->pScheduler->mutex);
    {
        if ( !callback->queued )
        {
            BLST_SQ_INSERT_TAIL(&callback->pScheduler->activeCallbackList, callback, activeCallbackNode);
            callback->queued = true;
        }
        callback->deleted = true;
    }
    B_Mutex_Unlock(callback->pScheduler->mutex);
}

/***************************************************************************
Summary:
Cancel all pending callbacks for this object instance

Description:
Because of the asynchronous nature of scheduler callbacks, race conditions
can occur when closing objects that have pending callbacks.  In order to
guarantee that all pending callbacks have been stopped for an instance,
this function should be called as part of closing the object instance.

NOTE: It is important to call this routine before acquiring your instance's
mutex to avoid a race condition where a callback into your module may still
be pending due to contention on the module's mutex.  As an example:

void B_Object_Close(B_ObjectHandle handle)
{
   BDBG_ASSERT(NULL != handle);

   B_Scheduler_StopCallbacks(handle->scheduler);

   // Now it's guaranteed that no callbacks are pending.

   B_Mutex_Lock(handle->mutex);

   // Actually clean up now
}
***************************************************************************/
void B_Scheduler_StopCallbacks(
    B_SchedulerHandle scheduler,
    void *pInstanceHandle
    )
{
    B_SchedulerCallback *pCallback;

    BDBG_ASSERT(NULL != scheduler);
    BDBG_ASSERT(NULL != pInstanceHandle);

    B_Mutex_Lock(scheduler->mutex);
    {
        /* Mark any callbacks matching the instance handle as stopped. */
        for ( pCallback = BLST_S_FIRST(&scheduler->callbackList);
              NULL != pCallback;
              pCallback = BLST_S_NEXT(pCallback, callbackNode) )
        {
            if ( pCallback->pInstanceHandle == pInstanceHandle )
            {
                pCallback->stopped = true;
            }
        }
    }
    B_Mutex_Unlock(scheduler->mutex);

    /* Sync with callback execution.  After this barrier, no further callbacks will be issued */
    B_Mutex_Lock(scheduler->callbackMutex);
    B_Mutex_Unlock(scheduler->callbackMutex);
}

