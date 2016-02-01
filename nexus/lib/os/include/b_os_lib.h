/***************************************************************************
*     (c)2004-2015 Broadcom Corporation
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
*   API name: OS
*    Library routines for OS abstraction
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef B_OS_LIB_H__
#define B_OS_LIB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bstd.h"
#include "bkni.h"
#include "b_os_time.h"

#undef B_OS_LIB_USE_NATIVE_MALLOC_CALLS_INSTEAD_OF_BKNI

/***************************************************************************
Summary:
Generic Error Codes

Description:
These error codes will match those returned by nexus (NEXUS_Error) and
magnum (BERR_Code).  These may be used throughout application libraries
for consistency.
***************************************************************************/
typedef unsigned B_Error;

/**
Summary:
Standard Nexus error codes.
**/
#define B_ERROR_SUCCESS              0  /* success (always zero) */
#define B_ERROR_NOT_INITIALIZED      1  /* parameter not initialized */
#define B_ERROR_INVALID_PARAMETER    2  /* parameter is invalid */
#define B_ERROR_OUT_OF_MEMORY        3  /* out of heap memory */
#define B_ERROR_TIMEOUT              5  /* reached timeout limit */
#define B_ERROR_OS_ERROR             6  /* generic OS error */
#define B_ERROR_LEAKED_RESOURCE      7  /* resource being freed has attached resources that haven't been freed */
#define B_ERROR_NOT_SUPPORTED        8  /* requested feature is not supported */
#define B_ERROR_UNKNOWN              9  /* unknown */

/***************************************************************************
Summary:
Initialize the OS Library

Description:
It is permissible to call this routine multiple times.  Internally, a
reference count will be maintained.
***************************************************************************/
B_Error B_Os_Init(void);

/***************************************************************************
Summary:
Un-Initialize the OS Library

Description:
This routine must be called once per call to B_Os_Init
***************************************************************************/
B_Error B_Os_Uninit(void);

/***************************************************************************
Summary:
Lock the B_Os global general purpose semaphore.

Description:
***************************************************************************/
void B_Os_Lock(void);

/***************************************************************************
Summary:
Unlock the B_Os global general purpose semaphore.

Description:
***************************************************************************/
void B_Os_Unlock(void);

/***************************************************************************
Summary:
Mutex Handle
***************************************************************************/
typedef struct B_Mutex *B_MutexHandle;

/***************************************************************************
Summary:
Event Handle
***************************************************************************/
typedef void *B_EventHandle;

/***************************************************************************
Summary:
Event Group Handle
***************************************************************************/
typedef void *B_EventGroupHandle;

/***************************************************************************
Summary:
Message Queue Handle
***************************************************************************/
typedef struct B_MessageQueue *B_MessageQueueHandle;

/***************************************************************************
Summary:
Thread Handle
***************************************************************************/
typedef struct B_Thread *B_ThreadHandle;

/***************************************************************************
Summary:
Thread Main Loop Function
***************************************************************************/
typedef void (*B_ThreadFunc)(void *pParam);

/***************************************************************************
Summary:
Scheduler Handle

Description:
A Scheduler is a higher-level OS construct.  A scheduler is responsible for
dispatching synchronized timer and event callbacks using a single thread.
***************************************************************************/
typedef struct B_Scheduler *B_SchedulerHandle;

/***************************************************************************
Summary:
Scheduler Event Callback Prototype
***************************************************************************/
typedef void (*B_EventCallback)(void *pContext);

/***************************************************************************
Summary:
Scheduler Event Callback Handle
***************************************************************************/
typedef struct B_SchedulerEvent *B_SchedulerEventId;

/***************************************************************************
Summary:
Scheduler Timer Callback Prototype
***************************************************************************/
typedef void (*B_TimerCallback)(void *pContext);

/***************************************************************************
Summary:
Scheduler Timer Handle
***************************************************************************/
typedef struct B_SchedulerTimer *B_SchedulerTimerId;

/***************************************************************************
Summary:
Scheduler Callback Handle
***************************************************************************/
typedef struct B_SchedulerCallback *B_SchedulerCallbackHandle;

/***************************************************************************
Summary:
Scheduler Callback Function Prototype
***************************************************************************/
typedef void (*B_SchedulerCallbackFunction)(void *pParam1, int param2);

/***************************************************************************
Shortcuts for event timeouts
***************************************************************************/
#define B_WAIT_NONE (0)             /* Avoid blocking while waiting for events */
#define B_WAIT_FOREVER (-1)         /* Block forever waiting for events */

/***************************************************************************
Summary:
Allocate Memory (malloc equivalent)
***************************************************************************/
#ifndef B_OS_LIB_USE_NATIVE_MALLOC_CALLS_INSTEAD_OF_BKNI
    #define B_Os_Malloc(numBytes)  BKNI_Malloc(numBytes)
#else
    #ifdef BDBG_DEBUG_BUILD
        #define B_Os_Malloc(numBytes) B_Os_Malloc_tagged((numBytes), __FUNCTION__, __LINE__)
    #else
        #define B_Os_Malloc(numBytes) B_Os_Malloc_tagged((numBytes), NULL, 0)
    #endif
    void *B_Os_Malloc_tagged(
        size_t numBytes,
        const char *pFunction,
        int line
        );
#endif /* #ifdef B_OS_LIB_USE_NATIVE_MALLOC_CALLS_INSTEAD_OF_BKNI */

/***************************************************************************
Summary:
Free Memory (free equivalent)
***************************************************************************/
#ifndef B_OS_LIB_USE_NATIVE_MALLOC_CALLS_INSTEAD_OF_BKNI
    #define B_Os_Free(pMemory) BKNI_Free(pMemory)
#else
    #ifdef BDBG_DEBUG_BUILD
        #define B_Os_Free(pMemory) B_Os_Free_tagged((pMemory), __FUNCTION__, __LINE__)
    #else
        #define B_Os_Free(pMemory) B_Os_Free_tagged((pMemory), NULL, 0)
    #endif
    void B_Os_Free_tagged(
        void *pMemory,
        const char *pFunction,
        int line
        );
#endif /* #ifdef B_OS_LIB_USE_NATIVE_MALLOC_CALLS_INSTEAD_OF_BKNI */

/***************************************************************************
Summary:
Allocate and Clear memory (calloc equivalent)
***************************************************************************/
#ifndef B_OS_LIB_USE_NATIVE_MALLOC_CALLS_INSTEAD_OF_BKNI
    #define  B_Os_Calloc(numMembers, numBytes)   B_Os_Memset(BKNI_Malloc(numBytes * numMembers),0,numBytes * numMembers)
#else
    #ifdef BDBG_DEBUG_BUILD
        #define B_Os_Calloc(numMembers, numBytes) B_Os_Calloc_tagged((numMembers), (numBytes), __FUNCTION__, __LINE__)
    #else
        #define B_Os_Calloc(numMembers, numBytes) B_Os_Calloc_tagged((numMembers), (numBytes), NULL, 0)
    #endif
    void *B_Os_Calloc_tagged(
        size_t numMembers,
        size_t memberSize,
        const char *pFunction,
        int line
        );
#endif /* #ifdef B_OS_LIB_USE_NATIVE_MALLOC_CALLS_INSTEAD_OF_BKNI */

/***************************************************************************
Summary:
Re-Allocate memory (realloc equivalent)

Note:   B_Os_Realloc_tagged() has been removed, since we don't have a
        way to implement it with the existing BKNI memory functions.
        We need a BKNI_Realloc() function.
***************************************************************************/
#ifdef B_OS_LIB_USE_NATIVE_MALLOC_CALLS_INSTEAD_OF_BKNI
    #ifdef BDBG_DEBUG_BUILD
        #define B_Os_Realloc(pMemory, numBytes) B_Os_Realloc_tagged((pMemory), (numBytes), __FUNCTION__, __LINE__)
    #else
        #define B_Os_Realloc(pMemory, numBytes) B_Os_Realloc_tagged((pMemory), (numBytes), NULL, 0)
    #endif
    void *B_Os_Realloc_tagged(
        void *pMemory,
        size_t numBytes,
        const char *pFunction,
        int line
        );
#endif /* #ifdef B_OS_LIB_USE_NATIVE_MALLOC_CALLS_INSTEAD_OF_BKNI */

/***************************************************************************
Summary:
Set a block of memory to a specified value (memset equivalent).
***************************************************************************/
void * B_Os_Memset(void *mem, int ch, size_t n);

/***************************************************************************
Summary:
Mutex settings.  Currently a placeholder.
***************************************************************************/
typedef struct { int notused; } B_MutexSettings;

/***************************************************************************
Summary:
Get default mutex settings.
***************************************************************************/
void B_Mutex_GetDefaultSettings(
    B_MutexSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
Create a mutex using the specified settings.
***************************************************************************/
B_MutexHandle B_Mutex_Create(
    const B_MutexSettings *pSettings   /* Pass NULL for defaults */
    );

/***************************************************************************
Summary:
Destroy a mutex
***************************************************************************/
void B_Mutex_Destroy(
    B_MutexHandle mutex
    );

/***************************************************************************
Summary:
Lock a mutex
***************************************************************************/
void B_Mutex_Lock(
    B_MutexHandle mutex
    );

/***************************************************************************
Summary:
Try to lock a mutex.

Description:
This will attempt to lock the mutex without blocking.  If the mutex is
available, it will be locked and B_Error_eSuccess will be returned.  If
the mutex is unavailable, B_Error_eTimeout will be returned.
***************************************************************************/
B_Error B_Mutex_TryLock(
    B_MutexHandle mutex
    );

/***************************************************************************
Summary:
Unlock a mutex.

Description:
Mutexes must be unlocked by the same thread that successfully called
B_Mutex_Lock or B_Mutex_TryLock.
***************************************************************************/
void B_Mutex_Unlock(
    B_MutexHandle mutex
    );

/***************************************************************************
Summary:
Verify that a mutex is locked by the calling thread

Description:
This can be used to facilitate debugging
***************************************************************************/
#define B_MUTEX_ASSERT_LOCKED(mutex) BDBG_ASSERT(true == B_Mutex_P_IsLocked((mutex)))
bool B_Mutex_P_IsLocked(
    B_MutexHandle mutex
    );

/***************************************************************************
Summary:
Event settings.  Currently a placeholder.
***************************************************************************/
typedef struct { int notused; } B_EventSettings;

/***************************************************************************
Summary:
Get default event settings.
***************************************************************************/
void B_Event_GetDefaultSettings(
    B_EventSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
Create an event
***************************************************************************/
B_EventHandle B_Event_Create(
    const B_EventSettings *pSettings    /* Pass NULL for defaults */
    );

/***************************************************************************
Summary:
Destroy an event
***************************************************************************/
void B_Event_Destroy(
    B_EventHandle event
    );

/***************************************************************************
Summary:
Set an event
***************************************************************************/
void B_Event_Set(
    B_EventHandle event
    );

/***************************************************************************
Summary:
Wait for an event to be set

Description:
This routine will wait for the specified event to be set.  If the event
was set, B_Error_eSuccess will be returned.  If the event was not set,
B_Error_eTimeout will be returned.
***************************************************************************/
B_Error B_Event_Wait(
    B_EventHandle event,
    int timeoutMsec         /* Timeout in milliseconds.  Pass B_WAIT_FOREVER to wait
                               forever or B_WAIT_NONE to avoid blocking */
    );

/***************************************************************************
Summary:
Clear an event if it has been set
***************************************************************************/
void B_Event_Reset(
    B_EventHandle event
    );

/***************************************************************************
Summary:
Event Group Settings.  Currently a placeholder.
***************************************************************************/
typedef void B_EventGroupSettings;

/***************************************************************************
Summary:
Get default event group settings
***************************************************************************/
void B_EventGroup_GetDefaultSettings(
    B_EventGroupSettings *pSettings     /* [out] */
    );

/***************************************************************************
Summary:
Create an event group
***************************************************************************/
B_EventGroupHandle B_EventGroup_Create(
    const B_EventGroupSettings *pSettings
    );

/***************************************************************************
Summary:
Destroy an event group
***************************************************************************/
void B_EventGroup_Destroy(
    B_EventGroupHandle group
    );

/***************************************************************************
Summary:
Add an event to the group
***************************************************************************/
B_Error B_EventGroup_AddEvent(
    B_EventGroupHandle group,
    B_EventHandle event
    );

/***************************************************************************
Summary:
Remove an event from the group
***************************************************************************/
B_Error B_EventGroup_RemoveEvent(
    B_EventGroupHandle group,
    B_EventHandle event
    );

/***************************************************************************
Summary:
Wait for any event in the group to be set.

Description:
This routine will wait for the specified event to be set.  If the event
was set, B_Error_eSuccess will be returned.  If the event was not set,
B_Error_eTimeout will be returned.
***************************************************************************/
B_Error B_EventGroup_Wait(
    B_EventGroupHandle group,
    int timeoutMsec,                    /* Timeout in milliseconds.  Pass B_WAIT_FOREVER to wait
                                           forever or B_WAIT_NONE to avoid blocking */
    B_EventHandle *pTriggeredEvents,    /* Triggered events handles will be copied into this array */
    unsigned maxTriggeredEvents,        /* Size of triggered event array */
    unsigned *pNumTriggeredEvents       /* [out] Number of triggered events */
    );

/***************************************************************************
Summary:
Delay a thread for the specified time

Description:
This will busy-wait a thread for the specified time without yielding.
***************************************************************************/
void B_Thread_Delay(
    int microseconds
    );

/***************************************************************************
Summary:
Suspend a thread for the specified time

Description:
This will yield the CPU for the specified time.  May round up to the nearest
OS scheduler tick value.
***************************************************************************/
void B_Thread_Sleep(
    int milliseconds
    );

/***************************************************************************
Summary:
Thread Settings
***************************************************************************/
typedef struct B_ThreadSettings
{
    int priority;   /* 0 = maximum, 100=minimum */
    int stackSize;  /* In bytes, 0 will use an OS default */
} B_ThreadSettings;

/***************************************************************************
Summary:
Get Default Thread Settings
***************************************************************************/
void B_Thread_GetDefaultSettings(
    B_ThreadSettings *pSettings     /* [out] */
    );

/***************************************************************************
Summary:
Create a thread using the specified settings
***************************************************************************/
B_ThreadHandle B_Thread_Create(
    const char *pName,                  /* Thread Name - Optional */
    B_ThreadFunc threadFunction,        /* Thread Main Routine */
    void *pThreadParam,                 /* Parameter provided to threadFunction */
    const B_ThreadSettings *pSettings   /* Pass NULL for defaults */
    );

/***************************************************************************
Summary:
Destroy a thread.

Description:
This will signal the thread to exit and will wait for the thread to
terminate before returning.
***************************************************************************/
void B_Thread_Destroy(
    B_ThreadHandle thread
    );

/***************************************************************************
Summary:
Get a handle to the currently executing thread

Description:
This will return NULL if the thread was not created with B_Thread_Create
***************************************************************************/
B_ThreadHandle B_Thread_GetSelf(void);

/***************************************************************************
Summary:
Get a pointer to the name of the current thread.

Description:
This will return a pointer to the name (null-terminated, from inside
the B_Thread object) of the current thread.  For non-B_Threads, it will
return a pointer to an empty string ("").
***************************************************************************/
const char * B_Thread_GetMyName(void);

/***************************************************************************
Summary:
Get a handle to the currently executing thread

Description:
This will return a thread ID for any thread, not just those created by
B_Thread_Create.  This is useful for bprofile.
***************************************************************************/
B_Error B_Thread_GetId(
    B_ThreadHandle thread,  /* Pass NULL for the current thread */
    int *pId
    );

/***************************************************************************
Summary:
Get the name of a specified thread for debugging.

Description:
This will return the name of a thread specified, and will return
an error for those threads not created with B_ThreadCreate.
***************************************************************************/
B_Error B_Thread_GetName(
    B_ThreadHandle thread,  /* Pass NULL for current thread */
    char *pName,            /* Pointer to array for name */
    int maxNameLen          /* Size of name array */
    );

/***************************************************************************
Summary:
Get the name of a specified thread for debugging.

Description:
This will return the name of a thread specified, and will return
an error for those threads not created with B_ThreadCreate.  This is useful
for bprofile.
***************************************************************************/
B_Error B_Thread_GetNameFromId(
    int id,         /* ID returned by B_Thread_GetId */
    char *pName,    /* Pointer to array for name */
    int maxNameLen  /* Size of name array */
    );

/***************************************************************************
Summary:
Scheduler settings.  Currently a placeholder.
***************************************************************************/
typedef void B_SchedulerSettings;

/***************************************************************************
Summary:
Get Default Scheduler Settings
***************************************************************************/
void B_Scheduler_GetDefaultSettings(
    B_SchedulerSettings *pSettings      /* [out] */
    );

/***************************************************************************
Summary:
Craete a scheduler
***************************************************************************/
B_SchedulerHandle B_Scheduler_Create(
    const B_SchedulerSettings *pSettings
    );

/***************************************************************************
Summary:
Destroy a scheduler
***************************************************************************/
void B_Scheduler_Destroy(
    B_SchedulerHandle scheduler
    );

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
    );

/***************************************************************************
Summary:
Un-Register an event from the scheduler
***************************************************************************/
void B_Scheduler_UnregisterEvent(
    B_SchedulerHandle scheduler,
    B_SchedulerEventId eventId
    );

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
    );

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
    );

/***************************************************************************
Summary:
Run a scheduler

Description:
This routine will drive the scheduler execution.  It will not return until
B_Scheduler_Stop is called.
***************************************************************************/
void B_Scheduler_Run(
    B_SchedulerHandle scheduler
    );

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
    );


/***************************************************************************
Summary:
Settings for a scheduler callback instance
***************************************************************************/
typedef void B_SchedulerCallbackSettings; /* Placeholder */

/***************************************************************************
Summary:
Get default settings for a scheduler callback instance
***************************************************************************/
void B_SchedulerCallback_GetDefaultSettings(
    B_SchedulerCallbackSettings *pSettings  /* [out] */
    );

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
    );

/***************************************************************************
Summary:
Set the function to be used in an asynchronous callback
***************************************************************************/
B_Error B_SchedulerCallback_SetFunction(
    B_SchedulerCallbackHandle callbackHandle,
    B_SchedulerCallbackFunction callbackFunction,   /* Function to be called */
    void *pParam1,                                  /* First function parameter */
    int param2                                      /* Second function parameter */
    );

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
    );

/***************************************************************************
Summary:
Destroying an asynchronous callback
***************************************************************************/
void B_SchedulerCallback_Destroy(
    B_SchedulerCallbackHandle callback
    );

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
    );

/***************************************************************************
Summary:
Get current system time

Description:
The value returned in this call is only useful in the B_Time_Add and
B_Time_Diff routines, it is not meant for direct access to a system time
value.
***************************************************************************/
void B_Time_Get(
    B_Time *pTime
    );

/***************************************************************************
Summary:
Return the delta between two time values in milliseconds
***************************************************************************/
long B_Time_Diff(
    const B_Time *pFuture,
    const B_Time *pPast
    );

/***************************************************************************
Summary:
Return the delta between two time values as a 64-bit number of microseconds
***************************************************************************/
uint64_t B_Time_DiffMicroseconds(
   const B_Time *pFuture,
   const B_Time *pPast
   );

/***************************************************************************
Summary:
Add a specified number of milliseconds to a time value
***************************************************************************/
void B_Time_Add(
    B_Time *pTime,
    long deltaMsec
    );

/***************************************************************************
Summary:
Message Queue Settings
***************************************************************************/
typedef struct B_MessageQueueSettings
{
    size_t maxMessageSize;  /* Maximum size of an individual message */
    size_t maxMessages;     /* Maximum messages in the queue at a time */
} B_MessageQueueSettings;

/***************************************************************************
Summary:
Get Default Message Queue Settings
***************************************************************************/
void B_MessageQueue_GetDefaultSettings(
    B_MessageQueueSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
Create a message queue
***************************************************************************/
B_MessageQueueHandle B_MessageQueue_Create(
    const B_MessageQueueSettings *pSettings
    );

/***************************************************************************
Summary:
Destroy a message queue
***************************************************************************/
void B_MessageQueue_Destroy(
    B_MessageQueueHandle handle
    );

/***************************************************************************
Summary:
Post (Send) a message to a queue

Description:
This function will return B_ERROR_SUCCESS if the message was sent
successfully or B_ERROR_TIMEOUT if the message queue is full.
***************************************************************************/
B_Error B_MessageQueue_Post(
    B_MessageQueueHandle handle,
    const void *pMessageData,
    size_t messageSize
    );

/***************************************************************************
Summary:
Receive a message from a queue

Description:
This function will return B_ERROR_SUCCESS if a message was successfully
received or B_ERROR_TIMEOUT if the specified timeout value was reached.
***************************************************************************/
B_Error B_MessageQueue_Wait(
    B_MessageQueueHandle handle,
    void *pMessageBuffer,
    size_t messageBufferSize,
    size_t *pMessageSizeReceived,
    int timeoutMsec                 /* Values B_WAIT_NONE and B_WAIT_FOREVER may be used */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef B_OS_LIB_H__ */
