/***************************************************************************
*  Copyright (C) 2003-2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
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
***************************************************************************/

/* 
 * Goal of this code is to provide least intrusive dead-lock detection, where deadlock detection based on mutex acquired for prolonged time
 * It based on adding minimal meta-data into each mutex and running the periodic 'heart-beat' routine that checks how long mutex was held.
 * It relies of underlying OS code to provide the following OS specific primitives
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
            if(acquiredCountTick != acquiredCount) {
                mutex->timerTick = now;
                mutex->acquiredCountTick = acquiredCount;
            } else {
                long diff = BKNI_P_MUTEXTRACKING_TICK_DIFF_MS(&now, &mutex->timerTick);
                if(diff > 2*1000) {
                    BDBG_WRN(("Mutex:%p(%s:%u) was acquired for %ld msec", (void *)mutex, mutex->file?mutex->file:"unknown",mutex->line, diff));
                }
                if(diff > 30*1000) {
                    BDBG_P_PrintString("Mutex:%p(%s:%u) deadlock detected, try to force release\n", (void *)mutex, mutex->file?mutex->file:"unknown",mutex->line);
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
        BDBG_P_PrintString("BKNI_AcquireMutex:%p was aborted, perhaps due to deadlock\n", (void *)mutex);
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


