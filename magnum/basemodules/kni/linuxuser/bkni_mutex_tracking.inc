/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
 * Implementatation of the Magnum KNI for user space Linux applications.
 *
 * Revision History:
 *
 * $brcm_Log: $ *
 * 
 ***************************************************************************/

/* 
 * Goal of this code is to provide least intrusive dead-lock detection, where deadlock detection based on mutex acquired for prolonged time
 * It based on adding minimal meta-data into each mutes and running the periodic 'heart-beat' routine that checks how long mutex was held.
 * It rellies of underlying OS code to provide the following OS specific primitives
    BKNI_P_MUTEXTRACKING_HEARTBEAT_TYPE         type heard beat data type
    BKNI_P_MUTEXTRACKING_HEARTBEAT_INIT(x)      intialization of heart beet
    BKNI_P_MUTEXTRACKING_HEARTBEAT_UNINIT(x)    un-intialization of heart beet
    BKNI_P_MUTEXTRACKING_HEARTBEAT_LOCK(x)      block heart beat from execution
    BKNI_P_MUTEXTRACKING_HEARTBEAT_UNLOCK(x)    resume hearr beat execution
    BKNI_P_MUTEXTRACKING_HEARTBEAT_DECLARE(x,heartBeat,delayms) declare function that to be executed as heart beat
    BKNI_P_MUTEXTRACKING_HEARTBEAT_START(x)     activate heart beat
    BKNI_P_MUTEXTRACKING_TICK_TYPE              type to compare the timer tick
    BKNI_P_MUTEXTRACKING_TICK_GET(x)            get current time tick
    BKNI_P_MUTEXTRACKING_TICK_DIFF_MS(x1,x2)    return difference between two timer ticks
    BKNI_P_MUTEXTRACKING_FORCE_RELEASE(x)       Force release of mutex that was acquired by different task/thread

And it defined the following data types and functions:
    BKNI_P_MutexTracking  data type that should be embedded into the mutex object
    BERR_Code BKNI_P_MutexTrackingState_Init(void) - used to initialize global state of the mutex tracking
    void BKNI_P_MutexTrackingState_Uninit(void) - used to unitialized global mutex tracking statre
    void BKNI_P_MutexTracking_Init(BKNI_P_MutexTracking  *mutex, const char *file, unsigned line) - used to initialize per mutex state
    void BKNI_P_MutexTracking_Uninit(BKNI_P_MutexTracking  *mutex) - used to uninitialize per mutex state
    void BKNI_P_MutexTracking_BeforeRelease(BKNI_P_MutexTracking *mutex) - should be called before mutex released
    void BKNI_P_MutexTracking_AfterAcquire(BKNI_P_MutexTracking  *mutex) - should be called after mutex was acquired
*/

#if BKNI_DEBUG_MUTEX_TRACKING
typedef struct BKNI_P_MutexTrackingState {
    BLST_D_HEAD(BKNI_P_MutexTrackingList, BKNI_P_MutexTracking) mutexList;
    BKNI_P_MUTEXTRACKING_HEARTBEAT_TYPE heartBeat; /* protects access to the list and updates to acquiredCountTick and timerTick members */
    bool timerStarted;
} BKNI_P_MutexTrackingState;

typedef struct BKNI_P_MutexTracking  {
    bool acquired;
    bool aborted;
    unsigned acquiredCount;
    unsigned acquiredCountTick; /* value of the acquiredCount when timerTick was captured */
    BKNI_P_MUTEXTRACKING_TICK_TYPE  timerTick; /* initialized by the heart-beat function */
    BKNI_MutexSettings settings;
    const char *file;
    unsigned line;
    BLST_D_ENTRY(BKNI_P_MutexTracking) link;
} BKNI_P_MutexTracking;

static BKNI_P_MutexTrackingState s_MutexTracking;

BERR_Code BKNI_P_MutexTrackingState_Init(void)
{
    BKNI_P_MutexTrackingState *tracking = &s_MutexTracking;
    tracking->timerStarted = false;
    return BERR_SUCCESS;
}

static void BKNI_P_MutexTracking_HeartBeat_locked(void)
{
    BKNI_P_MutexTrackingState *tracking = &s_MutexTracking;
    BKNI_P_MutexTracking *mutex;
    BKNI_P_MUTEXTRACKING_TICK_TYPE  now;

    BKNI_P_MUTEXTRACKING_TICK_GET(&now);
    for(mutex = BLST_D_FIRST(&tracking->mutexList); mutex ; mutex = BLST_D_NEXT(mutex, link)) {
        if(mutex->acquired) {
            unsigned acquiredCount = mutex->acquiredCount;
            unsigned acquiredCountTick = mutex->acquiredCountTick;
            if(mutex->settings.suspended) {
                return;
            }
            if(acquiredCountTick != acquiredCount) {
                mutex->timerTick = now;
                mutex->acquiredCountTick = acquiredCount;
            } else {
                long diff = BKNI_P_MUTEXTRACKING_TICK_DIFF_MS(&now, &mutex->timerTick);
                if(diff > 2*1000) {
                    BDBG_WRN(("Mutex:%p(%s:%u) was acquired for %ld msec", mutex, mutex->file?mutex->file:"unknown",mutex->line, diff));
                }
                if(diff > 30*1000) {
                    BDBG_P_PrintString("Mutex:%p(%s:%u) deadlock detected, try to force release\n", mutex, mutex->file?mutex->file:"unknown",mutex->line);
                    mutex->aborted = true;
                    BKNI_P_MUTEXTRACKING_FORCE_RELEASE(mutex);
                    break;
                }
            }
        }
    }
    return;
}

BKNI_P_MUTEXTRACKING_HEARTBEAT_DECLARE(BKNI_P_MutexTracking_HeartBeat_locked,&s_MutexTracking.heartBeat,1000)

static void BKNI_P_MutexTrackingState_Uninit(void)
{
    BKNI_P_MutexTrackingState *tracking = &s_MutexTracking;
    BLST_D_INIT(&tracking->mutexList);
    if(tracking->timerStarted) {
        tracking->timerStarted = false;
        BKNI_P_MUTEXTRACKING_HEARTBEAT_UNINIT(&tracking->heartBeat);
    }
    return;
}

static void BKNI_P_MutexTracking_Init(BKNI_P_MutexTracking  *mutex, const char *file, unsigned line)
{
    BKNI_P_MutexTrackingState *tracking = &s_MutexTracking;
    mutex->acquired = false;
    mutex->aborted = false;
    mutex->acquiredCount = 1;
    mutex->acquiredCountTick = 0;
    mutex->timerTick = 0;
    mutex->file = file;
    mutex->line = line;
    mutex->settings.suspended = false;
    if(!tracking->timerStarted) {
        tracking->timerStarted = true;
        BKNI_P_MUTEXTRACKING_HEARTBEAT_INIT(&tracking->heartBeat);
        BKNI_P_MUTEXTRACKING_HEARTBEAT_START(&tracking->heartBeat);
    }
    BKNI_P_MUTEXTRACKING_HEARTBEAT_LOCK(&tracking->heartBeat);
    BLST_D_INSERT_HEAD(&tracking->mutexList, mutex, link);
    BKNI_P_MUTEXTRACKING_HEARTBEAT_UNLOCK(&tracking->heartBeat);
    return;
}

static void BKNI_P_MutexTracking_Uninit(BKNI_P_MutexTracking  *mutex)
{
    BKNI_P_MutexTrackingState *tracking = &s_MutexTracking;

    BKNI_P_MUTEXTRACKING_HEARTBEAT_LOCK(&tracking->heartBeat);
    BLST_D_REMOVE(&tracking->mutexList, mutex, link);
    BKNI_P_MUTEXTRACKING_HEARTBEAT_UNLOCK(&tracking->heartBeat);
    return;
}

static void BKNI_P_MutexTracking_BeforeRelease(BKNI_P_MutexTracking *mutex)
{
    mutex->acquired = false;
    return;
}

static void BKNI_P_MutexTracking_AfterAcquire(BKNI_P_MutexTracking  *mutex, const char *file, int line)
{
    if(mutex->aborted) {
        BDBG_P_PrintString("BKNI_AcquireMutex:%p was aborted, perhaps due to deadlock\n", mutex);
        BKNI_Fail();
    }
    mutex->acquired = true;
    mutex->acquiredCount ++;
    mutex->file = file;
    mutex->line = line;
    return;
}
#else /* #if BKNI_DEBUG_MUTEX_TRACKING */
typedef unsigned BKNI_P_MutexTracking;
#define BKNI_P_MutexTrackingState_Init() (BERR_SUCCESS)
#define BKNI_P_MutexTrackingState_Uninit() 
#define BKNI_P_MutexTracking_Init(mutex, file, line) 
#define BKNI_P_MutexTracking_Uninit(mutex)
#define BKNI_P_MutexTracking_BeforeRelease(mutex) 
#define BKNI_P_MutexTracking_AfterAcquire(mutex, file, line) do { BSTD_UNUSED(file); BSTD_UNUSED(line); } while(0)
#endif /* #else /# #if BKNI_DEBUG_MUTEX_TRACKING #/ */


