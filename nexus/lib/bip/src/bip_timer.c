/******************************************************************************
 * (c) 2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/
#include "bip_priv.h"

BDBG_MODULE( bip_timer );
BDBG_OBJECT_ID( BIP_Timer );

/* Each BIP_Timer is really just a B_Scheduler_Timer (from the b_os library).
 * But to simplify, we'll create a BIP_TimerFactory that holds the Scheduler,
 * Thread, and Mutex that the B_Scheduler_Timers require.  */

typedef struct BIP_TimerFactory
{
    BDBG_OBJECT( BIP_TimerFactory )

    B_SchedulerHandle hScheduler;
    B_ThreadHandle    hSchedulerThread;        /* Thread used to run scheduler */
    B_MutexHandle     hSchedulerTimerMutex;

} BIP_TimerFactory;

BIP_TimerFactory   g_BIP_TimerFactory;      /* Allocate a single instance of the TimerFactory. */


void BIP_TimerFactory_GetDefaultInitSettings(
    BIP_TimerFactoryInitSettings *pSettings
    )
{
    BKNI_Memset( pSettings, 0, sizeof( *pSettings));
}


BIP_Status  BIP_TimerFactory_Init(
    BIP_TimerFactoryInitSettings *pSettings
    )
{
    BSTD_UNUSED(pSettings);
    BIP_TimerFactory *pTimerFactory = &g_BIP_TimerFactory;
    BIP_Status        rc = BIP_SUCCESS;


    BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_TimerFactory: Starting Initialization" BIP_MSG_PRE_ARG ));

    /* Create the scheduler. */
    pTimerFactory->hScheduler = B_Scheduler_Create(NULL);
    BIP_CHECK_GOTO(( pTimerFactory->hScheduler !=NULL ), ( "B_Scheduler_Create Failed" ), error, B_ERROR_UNKNOWN, rc );
    BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_TimerFactory: Created hScheduler:%p" BIP_MSG_PRE_ARG, pTimerFactory->hScheduler ));

    /* Create thread to run scheduler. */
    pTimerFactory->hSchedulerThread = B_Thread_Create("BipTmr",
                                   (B_ThreadFunc)B_Scheduler_Run,
                                   (void *) pTimerFactory->hScheduler,
                                   NULL);   /* Use default settings. */
    BIP_CHECK_GOTO(( pTimerFactory->hSchedulerThread !=NULL ), ( "B_Thread_Create Failed" ), error, B_ERROR_UNKNOWN, rc );
    BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_TimerFactory: Created hSchedulerThread:%p" BIP_MSG_PRE_ARG, pTimerFactory->hSchedulerThread ));

    /* Create a mutex for the scheduler timers. */
    pTimerFactory->hSchedulerTimerMutex = B_Mutex_Create( NULL );
    BIP_CHECK_GOTO(( pTimerFactory->hSchedulerTimerMutex !=NULL ), ( "B_Mutex_Create Failed" ), error, B_ERROR_UNKNOWN, rc );
    BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_TimerFactory: Created hSchedulerTimerMutex:%p" BIP_MSG_PRE_ARG, pTimerFactory->hSchedulerTimerMutex ));

    BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_TimerFactory: Initialization Complete" BIP_MSG_PRE_ARG ));

    return(BIP_SUCCESS);

error:
    BIP_TimerFactory_Uninit();
    return(rc);
}


void BIP_TimerFactory_Uninit(void)
{
    BIP_TimerFactory *pTimerFactory = &g_BIP_TimerFactory;

    if (pTimerFactory->hScheduler) {

        if (pTimerFactory->hSchedulerThread )
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_TimerFactory: Stopping hScheduler:%p" BIP_MSG_PRE_ARG, pTimerFactory->hScheduler));
            B_Scheduler_Stop(pTimerFactory->hScheduler);

            BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_TimerFactory: Destroying hSchedulerThread:%p" BIP_MSG_PRE_ARG, pTimerFactory->hSchedulerThread ));
            B_Thread_Destroy(pTimerFactory->hSchedulerThread);
        }

        BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_TimerFactory: Destroying hScheduler:%p" BIP_MSG_PRE_ARG, pTimerFactory->hScheduler ));
        B_Scheduler_Destroy(pTimerFactory->hScheduler);
    }

    if (pTimerFactory->hSchedulerTimerMutex)
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_TimerFactory: Destroying hSchedulerTimerMutex:%p" BIP_MSG_PRE_ARG, pTimerFactory->hSchedulerTimerMutex ));
        B_Mutex_Destroy(pTimerFactory->hSchedulerTimerMutex);
    }
}


void BIP_Timer_GetDefaultCreateSettings(
   BIP_TimerCreateSettings *pSettings
   )
{
    BKNI_Memset( pSettings, 0, sizeof( *pSettings ));
}


BIP_TimerHandle BIP_Timer_Create(
   BIP_TimerCreateSettings *pSettings
   )
{
    BIP_TimerHandle     hTimer;
    BIP_TimerFactory   *pTimerFactory = &g_BIP_TimerFactory;

    /* Start a B_Scheduler timer to do the timing.  But the B_Scheduler timers have an issue
     * that causes them to appear to fire early by up to 1 millisecond.  For example:
     *
     *   1. App calls B_Scheduler_StartTimer() at: 0.600800  with duration of 0.200
     *   2. B_Scheduler_StartTimer sets expiration time to 0.600800 + 0.200 = 0.800800
     *   3. At 0.800100, B_Scheduler_Run compares current time with timer time: 0.800100 - 0.800800 = -0.000700 (actually 700 us early, but truncates to zero ms, timer expired!)
     *   4. At 0.800100, App measures elapsed time: 0.800100 - 0.600800 = 0.199300  Truncates to 0.199, but original duration was specified as 0.200
     *
     * For the BIP_Timers, the desired behavior is that the callback be called at or after the requested timer duration, BUT NEVER BEFORE!
     * So we'll work around this by adding a millisecond to the requested BIP_Timer's duration when we start the B_Scheduler timer.
     *
     * But if the time duration is specified as zero, then don't bother adding the millisecond and just leave at zero so
     * the timer callback will be called as quickly as possible.
     *
     * */

    hTimer = B_Scheduler_StartTimer(
       pTimerFactory->hScheduler,
       pTimerFactory->hSchedulerTimerMutex,
       pSettings->input.timeoutInMs==0 ? 0 : pSettings->input.timeoutInMs +1,  /* "+1" to work around B_Scheduler timer early expiration. */
       pSettings->input.callback,
       pSettings->input.pContext);

    BDBG_MSG(( BIP_MSG_PRE_FMT "hTimer %p: Creation complete, timeoutInMs:%ld context:%p" BIP_MSG_PRE_ARG, hTimer, pSettings->input.timeoutInMs, pSettings->input.pContext ));

    return hTimer;
}

void BIP_Timer_Destroy(BIP_TimerHandle  hTimer)
{
    BIP_TimerFactory   *pTimerFactory = &g_BIP_TimerFactory;

    B_Scheduler_CancelTimer( pTimerFactory->hScheduler, hTimer);
}
