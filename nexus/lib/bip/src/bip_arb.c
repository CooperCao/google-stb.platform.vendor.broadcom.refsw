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

#include "blst_queue.h"

BDBG_MODULE( bip_arb );


BDBG_OBJECT_ID( BIP_Arb );
BDBG_OBJECT_ID( BIP_ArbList );
BDBG_OBJECT_ID( BIP_ArbDclItem );


typedef struct BIP_ArbDclItem  /* BIP's API Request Block */
{
    BDBG_OBJECT( BIP_ArbDclItem )
    BIP_CallbackDesc        completionCallback;  /* Callback to be called at request completion.   */
    bool                    callbackBusy;        /* true=> callback has been called, but hasn't returned yet. */
    BLST_Q_ENTRY(BIP_ArbDclItem) dclItem_next;

} BIP_ArbDclItem;


typedef struct BIP_ArbList
{
    BDBG_OBJECT( BIP_ArbList )

    B_MutexHandle                               hArbListMutex;    /* Mutex for synchronizing access to the ArbList.  */
    BLST_Q_HEAD( arbNewListHead, BIP_Arb)       arbNewListHead;
    BLST_Q_HEAD( arbAcceptedListHead, BIP_Arb)  arbAcceptedListHead;
} BIP_ArbList;


typedef struct BIP_ArbFactory
{
    /* Deferred Callback List (DCL) mutex and listhead. */
    B_MutexHandle                             hDclMutex;    /* Mutex for accessing the Deferred Callback List.    */
    BLST_Q_HEAD(dclItem_head, BIP_ArbDclItem) dclItem_head;
} BIP_ArbFactory;

BIP_ArbFactory   g_BIP_ArbFactory;      /* Allocate a single instance of the ArbFactory. */

static BIP_Status BIP_Arb_CheckForBipInit(void)
{
    BIP_Status          rc;
    BIP_InitStatus  initStatus;

    rc = BIP_Init_GetStatus(&initStatus);
    BIP_CHECK_GOTO(( rc==BIP_SUCCESS), ( "BIP_Init_GetStatus Failed" ), error, rc, rc );
    BIP_CHECK_GOTO(( initStatus.refCount), ( "BIP not initialized! Call BIP_Init()" ), error, BIP_ERR_NOT_INITIALIZED, rc );

error:
    BDBG_ASSERT(rc==BIP_SUCCESS);     /* If debug build, assert when BIP is not initialized. */
    return (rc);
}

static BIP_Status  BIP_Arb_ReadyToDestroy(
    BIP_ArbHandle hArb
    )
{
    BIP_Status  rc = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hArb, BIP_Arb );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Entry..." BIP_MSG_PRE_ARG, hArb ));

    if ( ! hArb->destroyMe) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Not marked for delete, returning..." BIP_MSG_PRE_ARG, hArb ));
        return(BIP_INF_ARB_IN_PROGRESS);
    }

    if (hArb->state != BIP_ArbState_eIdle) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Arb is not Idle, returning..." BIP_MSG_PRE_ARG, hArb ));
        return(BIP_INF_ARB_IN_PROGRESS);
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Setting hDestroyEvent" BIP_MSG_PRE_ARG, hArb ));
    B_Event_Set(hArb->hDestroyEvent);

    return(rc);

} /* BIP_Arb_ReadyToDestroy */


void BIP_ArbFactory_GetDefaultInitSettings(
    BIP_ArbFactoryInitSettings *pSettings
    )
{
    BKNI_Memset( pSettings, 0, sizeof( *pSettings));
}


BIP_Status  BIP_ArbFactory_Init(
    BIP_ArbFactoryInitSettings *pSettings
    )
{
    BSTD_UNUSED(pSettings);
    BIP_ArbFactory *pArbFactory = &g_BIP_ArbFactory;
    BIP_Status        rc = BIP_SUCCESS;

    BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_ArbFactory: Starting Initialization" BIP_MSG_PRE_ARG ));

    /* Init the Deferred Callback List (DCL). */
    BLST_Q_INIT(&pArbFactory->dclItem_head);

    /* Create a mutex for the Deferred Callback List (DCL). */
    pArbFactory->hDclMutex = B_Mutex_Create( NULL );
    BIP_CHECK_GOTO(( pArbFactory->hDclMutex != NULL ), ( "B_Mutex_Create Failed" ), error, B_ERROR_UNKNOWN, rc );
    BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_ArbFactory: Created hDclMutex:%p" BIP_MSG_PRE_ARG, pArbFactory->hDclMutex ));

    BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_ArbFactory: Initialization Complete" BIP_MSG_PRE_ARG ));

    return(BIP_SUCCESS);

error:
    BIP_ArbFactory_Uninit();
    return(rc);
}


void BIP_ArbFactory_Uninit(void)
{
    BIP_ArbFactory *pArbFactory = &g_BIP_ArbFactory;

    if (pArbFactory->hDclMutex)
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_ArbFactory: Destroying hDclMutex:%p" BIP_MSG_PRE_ARG, pArbFactory->hDclMutex ));
        B_Mutex_Destroy(pArbFactory->hDclMutex);
    }
}


void BIP_Arb_Destroy(
    BIP_ArbHandle hArb
    )
{
    BIP_Status  rc = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hArb, BIP_Arb );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Entry..." BIP_MSG_PRE_ARG, hArb ));

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Marking Arb for destruction!" BIP_MSG_PRE_ARG, hArb ));
    hArb->destroyMe = true;

    B_Event_Reset(hArb->hDestroyEvent);

    BIP_Arb_ReadyToDestroy(hArb);   /* Sets hDestroyEvent if Arb is ready to destroy. */

    BDBG_MSG(( BIP_MSG_PRE_FMT "Waiting to finish: "BIP_ARB_PRINTF_FMT
               BIP_MSG_PRE_ARG, BIP_ARB_PRINTF_ARG(hArb) ));
    rc = B_Event_Wait(hArb->hDestroyEvent, B_WAIT_FOREVER);
    BIP_CHECK_GOTO((rc == B_ERROR_SUCCESS), ( "B_Event_Wait failed, rc:0x%X", rc ), error, BIP_ERR_OS_CHECK_ERRNO, rc );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Arb is finished! It's okay to destroy!  Here we go..." BIP_MSG_PRE_ARG, hArb ));

    if (hArb->hDestroyEvent) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Destroying hDestroyEvent:%p" BIP_MSG_PRE_ARG, hArb, hArb->hDestroyEvent ));
        B_Event_Destroy( hArb->hDestroyEvent);
    }

    if (hArb->hArbEvent) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Destroying hArbEvent:%p" BIP_MSG_PRE_ARG, hArb, hArb->hArbEvent ));
        B_Event_Destroy( hArb->hArbEvent);
    }

    if (hArb->hApiMutex) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Destroying hApiMutex:%p" BIP_MSG_PRE_ARG, hArb, hArb->hApiMutex ));
        B_Mutex_Destroy( hArb->hApiMutex);
    }

    if (hArb->hArbMutex) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Destroying hArbMutex:%p" BIP_MSG_PRE_ARG, hArb, hArb->hArbMutex ));
        B_Mutex_Destroy( hArb->hArbMutex);
    }

    BDBG_OBJECT_DESTROY( hArb, BIP_Arb );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Freeing object memory" BIP_MSG_PRE_ARG, hArb ));
    B_Os_Free( hArb );

error:
    return;
} /* BIP_Arb_Destroy */


BIP_ArbHandle BIP_Arb_Create(B_MutexHandle hApiMutex, B_MutexHandle hArbMutex)
{
    int                   rc;
    BIP_ArbHandle         hArb = NULL;

    BDBG_MSG(( BIP_MSG_PRE_FMT "Entry..." BIP_MSG_PRE_ARG ));

    BDBG_ASSERT(hApiMutex==NULL);
    BDBG_ASSERT(hArbMutex==NULL);

    /* Allocate memory for the object. */
    hArb = B_Os_Calloc(1, sizeof( *hArb ));
    BIP_CHECK_GOTO(( hArb !=NULL ), ( "Memory Allocation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BDBG_OBJECT_SET( hArb, BIP_Arb );
    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Allocated " BIP_MSG_PRE_ARG, hArb ));

    hArb->hArbMutex = B_Mutex_Create(NULL);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Created hArbMutex:%p" BIP_MSG_PRE_ARG, hArb, hArb->hArbMutex  ));
    BIP_CHECK_GOTO(( hArb->hArbMutex  !=NULL ), ( "B_Mutex_Create Failed" ), error, BIP_ERR_OS_CHECK_ERRNO, rc );

    hArb->hApiMutex = B_Mutex_Create(NULL);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Created hArb->hApiMutex:%p" BIP_MSG_PRE_ARG, hArb, hArb->hApiMutex ));
    BIP_CHECK_GOTO(( hArb->hApiMutex !=NULL ), ( "B_Mutex_Create Failed" ), error, BIP_ERR_OS_CHECK_ERRNO, rc );

    hArb->hArbEvent = B_Event_Create(NULL);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Created hArbEvent:%p" BIP_MSG_PRE_ARG, hArb, hArb->hArbEvent ));
    BIP_CHECK_GOTO(( hArb->hArbEvent !=NULL ), ( "B_Event_Create Failed" ), error, BIP_ERR_OS_CHECK_ERRNO, rc );

    hArb->hDestroyEvent = B_Event_Create(NULL);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Created hDestroyEvent:%p" BIP_MSG_PRE_ARG, hArb, hArb->hDestroyEvent ));
    BIP_CHECK_GOTO(( hArb->hDestroyEvent !=NULL ), ( "B_Event_Create Failed" ), error, BIP_ERR_OS_CHECK_ERRNO, rc );

    hArb->state = BIP_ArbState_eIdle;

    return hArb;

error:
    if (hArb) {BIP_Arb_Destroy( hArb ); }
    return( NULL );
} /* BIP_Arb_Create */




BIP_ArbHandle BIP_Arb_CreateAppArb(size_t appArbSize)
{
    int                      rc;
    BIP_ArbHandle         hArb = NULL;

    BDBG_MSG(( BIP_MSG_PRE_FMT "Entry..." BIP_MSG_PRE_ARG ));

    BDBG_ASSERT(appArbSize >= sizeof (BIP_Arb));

    /* Allocate memory for the object. */
    hArb = B_Os_Calloc(1, appArbSize );
    BIP_CHECK_GOTO(( hArb !=NULL ), ( "Memory Allocation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BDBG_OBJECT_SET( hArb, BIP_Arb );
    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Allocated " BIP_MSG_PRE_ARG, hArb ));

    hArb->hArbMutex = B_Mutex_Create(NULL);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Created hArbMutex:%p" BIP_MSG_PRE_ARG, hArb, hArb->hArbMutex  ));
    BIP_CHECK_GOTO(( hArb->hArbMutex  !=NULL ), ( "B_Mutex_Create Failed" ), error, BIP_ERR_OS_CHECK_ERRNO, rc );

    hArb->hApiMutex = B_Mutex_Create(NULL);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Created hArb->hApiMutex:%p" BIP_MSG_PRE_ARG, hArb, hArb->hApiMutex ));
    BIP_CHECK_GOTO(( hArb->hApiMutex !=NULL ), ( "B_Mutex_Create Failed" ), error, BIP_ERR_OS_CHECK_ERRNO, rc );

    hArb->hArbEvent = B_Event_Create(NULL);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Created hArbEvent:%p" BIP_MSG_PRE_ARG, hArb, hArb->hArbEvent ));
    BIP_CHECK_GOTO(( hArb->hArbEvent !=NULL ), ( "B_Event_Create Failed" ), error, BIP_ERR_OS_CHECK_ERRNO, rc );

    hArb->hDestroyEvent = B_Event_Create(NULL);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Created hDestroyEvent:%p" BIP_MSG_PRE_ARG, hArb, hArb->hDestroyEvent ));
    BIP_CHECK_GOTO(( hArb->hDestroyEvent !=NULL ), ( "B_Event_Create Failed" ), error, BIP_ERR_OS_CHECK_ERRNO, rc );

    hArb->state = BIP_ArbState_eIdle;

    return hArb;

error:
    if (hArb) {BIP_Arb_Destroy( hArb ); }
    return( NULL );
} /* BIP_Arb_Create */


BIP_Status BIP_Arb_Acquire(BIP_ArbHandle hArb)
{
    int     rc = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hArb, BIP_Arb );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Entry..." BIP_MSG_PRE_ARG, hArb ));

    rc = BIP_Arb_CheckForBipInit();
    BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP not initialized! Call BIP_Init()" ), error, rc, rc );

    B_Mutex_Lock(hArb->hApiMutex);
    B_Mutex_Lock(hArb->hArbMutex);

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Locked Api and Arb Mutexes" BIP_MSG_PRE_ARG, hArb ));

    if (hArb->state != BIP_ArbState_eIdle) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Arb is already busy, state: %d" BIP_MSG_PRE_ARG, hArb, hArb->state ));
        rc = BIP_ERR_ALREADY_IN_PROGRESS;
        goto error_unlock;
    }
    else if (hArb->destroyMe) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Arb is being destroyed, Acquire not allowed" BIP_MSG_PRE_ARG, hArb ));
        rc = BIP_ERR_ARB_BEING_DESTROYED;
        goto error_unlock   ;
    }
    else {
        /* Save the handles that we want to keep. */
        B_MutexHandle   hApiMutex     = hArb->hApiMutex;
        B_MutexHandle   hArbMutex     = hArb->hArbMutex ;
        B_EventHandle   hArbEvent     = hArb->hArbEvent;
        B_EventHandle   hDestroyEvent = hArb->hDestroyEvent;

        BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Resetting Arb..." BIP_MSG_PRE_ARG, hArb ));
        /* Init the whole Arb to zeros. */
        BKNI_Memset(hArb, 0 , sizeof(*hArb));

        BDBG_OBJECT_SET( hArb, BIP_Arb );

        /* Restore the handles. */
        hArb->hApiMutex       = hApiMutex;
        hArb->hArbMutex       = hArbMutex;
        hArb->hArbEvent       = hArbEvent;
        hArb->hDestroyEvent   = hDestroyEvent;

        /* Initialize misc other fields. */
        hArb->state = BIP_ArbState_eAcquired;
        hArb->apiBusy = true;
        hArb->completionStatus = BIP_INF_IN_PROGRESS;

        B_Mutex_Unlock(hArb->hArbMutex);
        /* Normal case... leave hApiMutex locked. */
    }
    /* coverity[missing_unlock] */
    return rc;

error_unlock:
        B_Mutex_Unlock(hArb->hArbMutex);
        B_Mutex_Unlock(hArb->hApiMutex);
error:
        return rc;
} /* BIP_Arb_Acquire */


void BIP_Arb_GetDefaultSubmitSettings(
    BIP_ArbSubmitSettings   *pSettings
    )
{
    BKNI_Memset( pSettings, 0, sizeof( *pSettings ));
    pSettings->waitIfBusy = true;
}


BIP_Status BIP_Arb_Submit(
   BIP_ArbHandle     hArb,
   BIP_ArbSubmitSettings   *pSettings,
   BIP_ApiSettings         *pApiSettings
   )
{
    BIP_ArbSubmitSettings    settings;
    int     rc = BIP_SUCCESS;
    bool    apiLocked = true;   /* hArb->hApiMutex is locked on entry. */
    bool    arbLocked = false;

    BDBG_OBJECT_ASSERT( hArb, BIP_Arb );

    rc = BIP_Arb_CheckForBipInit();
    BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP not initialized! Call BIP_Init()" ), error, rc, rc );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Entry..." BIP_MSG_PRE_ARG, hArb ));

    B_MUTEX_ASSERT_LOCKED(hArb->hApiMutex);

    B_Mutex_Lock(hArb->hArbMutex); arbLocked = true;
    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Verified that Api and Arb are locked" BIP_MSG_PRE_ARG, hArb ));


    if (hArb->state != BIP_ArbState_eAcquired) {
        BDBG_ERR(( BIP_MSG_PRE_FMT "hArb %p: Arb is not Acquired! state: %d  Did you call BIP_Arb_Acquire()?"
                   BIP_MSG_PRE_ARG, hArb, hArb->state ));
        BIP_CHECK_GOTO((false), ( "Arb is Busy/Idle but should be Acquired!" ), error, BIP_ERR_INVALID_API_SEQUENCE, rc );
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Verified that Arb state is Acquired" BIP_MSG_PRE_ARG, hArb ));

    if (!pSettings) {
        BIP_Arb_GetDefaultSubmitSettings(&settings);
        pSettings = &settings;
    }

    hArb->settings = *pSettings;

    if (pApiSettings) {
        if (pApiSettings->asyncCallback.callback) {
            hArb->completionCallback = pApiSettings->asyncCallback;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Set completionCallback to %p from ApiSettings" BIP_MSG_PRE_ARG, hArb, hArb->completionCallback));
        }

        if (pApiSettings->pAsyncStatus) {
            hArb->pCompletionStatus = pApiSettings->pAsyncStatus;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Set pCompletionStatus to %p from ApiSettings" BIP_MSG_PRE_ARG, hArb, hArb->pCompletionStatus));
        }
    }

    hArb->completionStatus = BIP_INF_IN_PROGRESS;
    if (hArb->pCompletionStatus) {
        *hArb->pCompletionStatus = hArb->completionStatus;
    }
    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Set *pCompletionStatus to BIP_INF_IN_PROGRESS" BIP_MSG_PRE_ARG, hArb ));

    if (hArb->hArbEvent) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Resetting Arb completionEvent" BIP_MSG_PRE_ARG, hArb ));
        B_Event_Reset(hArb->hArbEvent);
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Setting Arb state to Submitted" BIP_MSG_PRE_ARG, hArb ));
    hArb->state = BIP_ArbState_eSubmitted;  /* This hands the Arb to the server-side Arb-processor. */

    if (hArb->settings.arbProcessor) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Calling Arb's arbProcessor" BIP_MSG_PRE_ARG, hArb ));
        B_Mutex_Unlock(hArb->hArbMutex); arbLocked = false;
        hArb->settings.arbProcessor(hArb->settings.hObject, 0, BIP_Arb_ThreadOrigin_eArb);
        B_Mutex_Lock(hArb->hArbMutex); arbLocked = true;
    }

    /* Arb state should now be accepted or rejected. */

    B_MUTEX_ASSERT_LOCKED(hArb->hApiMutex);

    if ( !hArb->settings.waitIfBusy) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Unlocking ApiMutex because waitIfBusy is false" BIP_MSG_PRE_ARG, hArb ));
        B_Mutex_Unlock(hArb->hApiMutex); apiLocked = false;
    }

    if (hArb->state == BIP_ArbState_eCompleted && hArb->rejected) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Request was rejected. completionStatus:0x%X" BIP_MSG_PRE_ARG, hArb, hArb->completionStatus ));
        BDBG_ASSERT(hArb->completionStatus != BIP_INF_IN_PROGRESS);
        rc = hArb->completionStatus;
    }
    else if (hArb->completionCallback.callback) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Request has completionCallback, no need to wait" BIP_MSG_PRE_ARG, hArb ));
        rc = BIP_INF_IN_PROGRESS;   /* Async request has been successfully started, completion callback will be called when it completes. */
    }
    else if (hArb->pCompletionStatus) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Request has a pCompletionStatus pointer, no need to wait" BIP_MSG_PRE_ARG, hArb ));
        rc = BIP_INF_IN_PROGRESS;   /* Async request has been successfully started, completion status variable will get set when it completes. */
    }
    else if (hArb->hArbEvent)
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Waiting on *pCompletionEvent. completionStatus:0x%X" BIP_MSG_PRE_ARG, hArb, hArb->completionStatus ));
        B_Mutex_Unlock(hArb->hArbMutex); arbLocked = false;
        rc = B_Event_Wait(hArb->hArbEvent, B_WAIT_FOREVER);
        B_Mutex_Lock(hArb->hArbMutex); arbLocked = true;
        BIP_CHECK_GOTO((rc == B_ERROR_SUCCESS), ( "B_Event_Wait failed, rc:0x%X", rc ), error, BIP_ERR_B_OS_LIB, rc );

        BDBG_ASSERT(hArb->completionStatus != BIP_INF_IN_PROGRESS);
        BDBG_ASSERT(hArb->state == BIP_ArbState_eCompleted);
        rc = hArb->completionStatus;        /* API Request should be complete now. */
    }
    else
    {
        BIP_CHECK_GOTO((false), ( "Arb not finished, but no hArbEvent or completionCallback!" ), error, BIP_ERR_INVALID_PARAMETER, rc );
    }

    if ( hArb->settings.waitIfBusy) {
        B_MUTEX_ASSERT_LOCKED(hArb->hApiMutex);
        B_Mutex_Unlock(hArb->hApiMutex); apiLocked = false;
    }

    hArb->apiBusy = false;  /* Indicate that client-side is done with the Arb. */

    /* Print "return" message now, because Arb may be destroyed below. */
    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Returning: "BIP_STATUS_FMT"" BIP_MSG_PRE_ARG, hArb, BIP_STATUS_ARG(rc) ));

    if (hArb->state != BIP_ArbState_eCompleted) {
        B_Mutex_Unlock(hArb->hArbMutex);
    }
    else {  /* Arb's state is eCompleted. */
        hArb->state = BIP_ArbState_eIdle;
        if (hArb->hMyArbList) {     /* If dynamic Arb, destroy it. */
            BDBG_MSG(( BIP_MSG_PRE_FMT "hArbList %p: Destroying dynamic Arb: %p" BIP_MSG_PRE_ARG, hArb->hMyArbList, hArb));
            B_Mutex_Unlock(hArb->hArbMutex);
            BIP_Arb_Destroy(hArb); hArb = NULL;   /* Arb destroyed, hArb is now invalid. */
        }
        else {                      /* If static Arb, set back to idle. */
            BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Arb state is Completed, changeing to Idle" BIP_MSG_PRE_ARG, hArb ));
            B_Mutex_Unlock(hArb->hArbMutex);
            BIP_Arb_ReadyToDestroy(hArb);   /* Sets hArb->hDestroyEvent if Arb is ready to destroy. */
        }
    }
    return rc;

error:
    if (arbLocked) {
        B_Mutex_Unlock(hArb->hArbMutex);
    }
    if (apiLocked) {
        B_Mutex_Unlock(hArb->hApiMutex);
    }
    return rc;
} /* BIP_Arb_Submit */



void BIP_ArbList_Destroy(
    BIP_ArbListHandle hArbList
    )
{
    BDBG_OBJECT_ASSERT( hArbList, BIP_ArbList );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArbList %p: Entry..." BIP_MSG_PRE_ARG, hArbList ));

    /* Make sure that ArbList is empty before trying to destoy it.*/
    BDBG_ASSERT( BLST_Q_EMPTY( &hArbList->arbNewListHead));
    BDBG_ASSERT( BLST_Q_EMPTY( &hArbList->arbAcceptedListHead));

    if (hArbList->hArbListMutex) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArbList %p: Destroying hArbListMutex:%p" BIP_MSG_PRE_ARG, hArbList, hArbList->hArbListMutex ));
        B_Mutex_Destroy(hArbList->hArbListMutex);
    }

    BDBG_OBJECT_DESTROY( hArbList, BIP_ArbList );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArbList %p: Freeing object memory" BIP_MSG_PRE_ARG, hArbList ));
    B_Os_Free( hArbList );

    return;
} /* BIP_ArbList_Destroy */


BIP_ArbListHandle BIP_ArbList_Create(void * unused)
{
    int                      rc;
    BIP_ArbListHandle        hArbList = NULL;

    BSTD_UNUSED(unused);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Entry..." BIP_MSG_PRE_ARG ));

    rc = BIP_Arb_CheckForBipInit();
    BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP not initialized! Call BIP_Init()" ), error, rc, rc );

    /* Allocate memory for the object. */
    hArbList = B_Os_Calloc(1, sizeof( *hArbList ));
    BIP_CHECK_GOTO(( hArbList !=NULL ), ( "Memory Allocation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BDBG_OBJECT_SET( hArbList, BIP_ArbList );
    BDBG_MSG(( BIP_MSG_PRE_FMT "hArbList %p: Allocated " BIP_MSG_PRE_ARG, hArbList ));

    hArbList->hArbListMutex = B_Mutex_Create(NULL);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hArbList %p: Created hArbListMutex:%p" BIP_MSG_PRE_ARG, hArbList, hArbList->hArbListMutex ));
    BIP_CHECK_GOTO(( hArbList->hArbListMutex !=NULL ), ( "B_Mutex_Create Failed" ), error, BIP_ERR_OS_CHECK_ERRNO, rc );

    BLST_Q_INIT(&hArbList->arbNewListHead);
    BLST_Q_INIT(&hArbList->arbAcceptedListHead);

    return hArbList;

error:
    if (hArbList) {BIP_ArbList_Destroy( hArbList ); }
    return( NULL );
} /* BIP_ArbList_Create */


BIP_Status BIP_ArbList_Submit(
    BIP_ArbListHandle       hArbList,
    BIP_ArbHandle           hArb,
    BIP_ArbSubmitSettings  *pSettings,
    BIP_ApiSettings        *pApiSettings    /* Generic API settings. */
    )
{
    int                     rc = BIP_SUCCESS;

    BDBG_ASSERT(hArb);
    hArb = (BIP_ArbHandle) hArb;  /* First element in the AppArb should be a BIP_Arb struct. */

    rc = BIP_Arb_CheckForBipInit();
    BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP not initialized! Call BIP_Init()" ), error, rc, rc );

    BDBG_OBJECT_ASSERT( hArbList, BIP_ArbList );
    BDBG_OBJECT_ASSERT( hArb, BIP_Arb );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArbList %p: Entry... Locking ArbList Mutex..." BIP_MSG_PRE_ARG, hArbList ));

    B_Mutex_Lock(hArbList->hArbListMutex);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hArbList %p: Inserting hArb:%p at tail of arbNewList" BIP_MSG_PRE_ARG, hArbList, hArb ));
    BLST_Q_INSERT_TAIL(&hArbList->arbNewListHead, hArb, arbNewListNext);
    hArb->hMyArbList = hArbList;

    B_Mutex_Unlock(hArbList->hArbListMutex);

    /* Now just submit the Arb as with static Arbs. */
    rc = BIP_Arb_Submit(hArb, pSettings, pApiSettings);

error:
    return (rc);
}


BIP_ArbHandle BIP_ArbList_GetNewArb(
    BIP_ArbListHandle   hArbList
    )
{
    BIP_ArbHandle   hArb;

    BDBG_OBJECT_ASSERT( hArbList, BIP_ArbList );

    B_Mutex_Lock(hArbList->hArbListMutex);
    hArb = BLST_Q_FIRST(&hArbList->arbNewListHead);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hArbList %p: Getting oldest hArb:%p" BIP_MSG_PRE_ARG, hArbList, hArb ));
    B_Mutex_Unlock(hArbList->hArbListMutex);

    return hArb;
}


bool BIP_Arb_IsIdle(
    BIP_ArbHandle     hArb
    )
{
    bool    rc = false;

    BDBG_OBJECT_ASSERT( hArb, BIP_Arb );

    if (hArb->state == BIP_ArbState_eIdle     ||
        hArb->state == BIP_ArbState_eCompleted )
    {
        rc = true;
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Returning %s" BIP_MSG_PRE_ARG, hArb, rc?"TRUE":"FALSE" ));

    return rc;

} /* BIP_Arb_IsIdle */


bool BIP_Arb_IsNew(
    BIP_ArbHandle     hArb
    )
{
    bool    rc = false;

    BDBG_OBJECT_ASSERT( hArb, BIP_Arb );

    if (hArb->state == BIP_ArbState_eSubmitted)
    {
        rc = true;
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Returning %s" BIP_MSG_PRE_ARG, hArb, rc?"TRUE":"FALSE" ));

    return rc;

} /* BIP_Arb_IsNew */


bool BIP_Arb_IsBusy(
    BIP_ArbHandle     hArb
    )
{
    bool    rc = false;

    BDBG_OBJECT_ASSERT( hArb, BIP_Arb );

    if (hArb->state == BIP_ArbState_eAccepted)
    {
        rc = true;
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Returning %s" BIP_MSG_PRE_ARG, hArb, rc?"TRUE":"FALSE" ));

    return rc;

} /* BIP_Arb_IsBusy */


BIP_Status BIP_Arb_GetRequest(
   BIP_ArbHandle     hArb
   )
{
    BIP_Status     rc = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hArb, BIP_Arb );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Entry..." BIP_MSG_PRE_ARG, hArb ));

    if (hArb->state != BIP_ArbState_eSubmitted) {
        rc = BIP_INF_END_OF_STREAM;
    }

    return rc;

} /* BIP_Arb_GetRequest*/


bool BIP_ArbList_IsEmpty(
    BIP_ArbListHandle     hArbList
    )
{
    bool    rc = false;

    BDBG_OBJECT_ASSERT( hArbList, BIP_ArbList );

    if (BLST_Q_EMPTY(&hArbList->arbNewListHead)) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArbList %p: arbNewList NOT empty" BIP_MSG_PRE_ARG, hArbList ));
        rc = true;
    }

    if (BLST_Q_EMPTY(&hArbList->arbAcceptedListHead)) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArbList %p: arbAcceptedList NOT empty" BIP_MSG_PRE_ARG, hArbList ));
        rc = true;
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArbList %p: Returning %s" BIP_MSG_PRE_ARG, hArbList, rc?"TRUE":"FALSE" ));

    return rc;

} /* BIP_ArbList_IsEmpty */


BIP_Status BIP_Arb_AcceptRequest(
   BIP_ArbHandle     hArb
   )
{
    BIP_Status     rc = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hArb, BIP_Arb );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Entry..." BIP_MSG_PRE_ARG, hArb ));

    B_Mutex_Lock(hArb->hArbMutex);

    BIP_CHECK_GOTO((hArb->state == BIP_ArbState_eSubmitted),
                   ( "Can't accept Arb! Arb state is %d (should be %d)", hArb->state, BIP_ArbState_eSubmitted ),
                    error, BIP_ERR_ARB_INTERNAL, rc );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Arb state is Submitted, changing to Accepted" BIP_MSG_PRE_ARG, hArb ));
    hArb->state = BIP_ArbState_eAccepted;

    if (hArb->hMyArbList) {
        BIP_ArbListHandle   hArbList = hArb->hMyArbList;

        B_Mutex_Lock(hArbList->hArbListMutex);
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArbList %p: Removing Arb: %p from New list" BIP_MSG_PRE_ARG, hArbList, hArb));
        BLST_Q_REMOVE(&hArbList->arbNewListHead, hArb, arbNewListNext);

        BLST_Q_INSERT_TAIL(&hArbList->arbAcceptedListHead, hArb, arbAcceptedListNext);
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArbList %p: Adding Arb: %p to Accepted list" BIP_MSG_PRE_ARG, hArbList, hArb));
        B_Mutex_Unlock(hArbList->hArbListMutex);
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Returning: "BIP_STATUS_FMT"" BIP_MSG_PRE_ARG, hArb, BIP_STATUS_ARG(rc) ));
    B_Mutex_Unlock(hArb->hArbMutex);
    return rc;

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: [Error] Returning: "BIP_STATUS_FMT"" BIP_MSG_PRE_ARG, hArb, BIP_STATUS_ARG(rc) ));
    B_Mutex_Unlock(hArb->hArbMutex);
    return rc;

} /* BIP_Arb_AcceptRequest */


BIP_Status BIP_Arb_RejectRequest(
   BIP_ArbHandle     hArb,
   BIP_Status         completionStatus
   )
{
    BIP_Status   rc = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hArb, BIP_Arb );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Entry..." BIP_MSG_PRE_ARG, hArb ));

    B_Mutex_Lock(hArb->hArbMutex);

    BIP_CHECK_GOTO((hArb->state == BIP_ArbState_eSubmitted),
                   ( "Can't reject Arb! Arb state is %d (should be %d)", hArb->state, BIP_ArbState_eSubmitted ),
                    error, BIP_ERR_ARB_INTERNAL, rc );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Setting completionStatus to 0x%X" BIP_MSG_PRE_ARG, hArb, completionStatus ));
    hArb->completionStatus = completionStatus;
    if (hArb->pCompletionStatus) {
        *hArb->pCompletionStatus = hArb->completionStatus;
    }

    if (hArb->hMyArbList) {
        BIP_ArbListHandle   hArbList = hArb->hMyArbList;

        B_Mutex_Lock(hArbList->hArbListMutex);
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArbList %p: Removing Arb: %p from New list" BIP_MSG_PRE_ARG, hArbList, hArb));
        BLST_Q_REMOVE(&hArbList->arbNewListHead, hArb, arbNewListNext);
        B_Mutex_Unlock(hArbList->hArbListMutex);
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Arb state is Submitted, changing to Completed" BIP_MSG_PRE_ARG, hArb ));
    hArb->rejected = true;
    hArb->state = BIP_ArbState_eCompleted;

    if (!hArb->apiBusy)
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Arb state is Completed and !apiBusy, changing to Idle" BIP_MSG_PRE_ARG, hArb ));
        hArb->state = BIP_ArbState_eIdle;
        B_Mutex_Unlock(hArb->hArbMutex);
        BIP_Arb_ReadyToDestroy(hArb);       /* Sets hArb->hDestroyEvent if Arb is ready to destroy. */
        return rc;                          /* If ready to destroy, don't dereference hArb anymore. */
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Returning: "BIP_STATUS_FMT"" BIP_MSG_PRE_ARG, hArb, BIP_STATUS_ARG(rc) ));
    B_Mutex_Unlock(hArb->hArbMutex);
    return rc;

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: [Error] Returning: "BIP_STATUS_FMT"" BIP_MSG_PRE_ARG, hArb, BIP_STATUS_ARG(rc) ));
    B_Mutex_Unlock(hArb->hArbMutex);
    return rc;

} /* BIP_Arb_AcceptRequest */


BIP_Status BIP_Arb_AddDeferredCallback(
   BIP_ArbHandle     hArb,
   BIP_CallbackDesc *pCallbackDesc
   )
{
    BIP_ArbFactory      *pArbFactory = &g_BIP_ArbFactory;
    BIP_ArbDclItem      *pDclItem;
    int                  rc = BIP_SUCCESS;

    BSTD_UNUSED(hArb);
    BDBG_ASSERT(pCallbackDesc != NULL);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Entry... pCallbackDesc: %p, callback: %p, context: %p, param: %d" BIP_MSG_PRE_ARG,
               pCallbackDesc, pCallbackDesc->callback, pCallbackDesc->context, pCallbackDesc->param ));

    /* Allocate memory for the object. */
    pDclItem  = B_Os_Calloc(1, sizeof( *pDclItem ));
    BIP_CHECK_GOTO(( pDclItem !=NULL ), ( "Memory Allocation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BDBG_OBJECT_SET( pDclItem, BIP_ArbDclItem );
    BDBG_MSG(( BIP_MSG_PRE_FMT "pDclItem %p: Allocated (%d bytes)" BIP_MSG_PRE_ARG, pDclItem, sizeof *pDclItem ));

    pDclItem->completionCallback = *pCallbackDesc;

    B_Mutex_Lock(pArbFactory->hDclMutex);
    BLST_Q_INSERT_TAIL(&pArbFactory->dclItem_head, pDclItem, dclItem_next);
    B_Mutex_Unlock(pArbFactory->hDclMutex);

    BDBG_MSG(( BIP_MSG_PRE_FMT "pDclItem %p: Returning: "BIP_STATUS_FMT"" BIP_MSG_PRE_ARG, pDclItem, BIP_STATUS_ARG(rc) ));
    return rc;

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "pDclItem %p: [Error] Returning: "BIP_STATUS_FMT"" BIP_MSG_PRE_ARG, pDclItem, BIP_STATUS_ARG(rc) ));
    return rc;
} /* BIP_Arb_AddDeferredCallback */


BIP_Status BIP_Arb_CompleteRequest(
   BIP_ArbHandle     hArb,
   BIP_Status         completionStatus
   )
{
    int     rc = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hArb, BIP_Arb );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Entry..." BIP_MSG_PRE_ARG, hArb ));

    B_Mutex_Lock(hArb->hArbMutex);

    BIP_CHECK_GOTO((hArb->state == BIP_ArbState_eAccepted),
                   ( "Can't complete Arb! Arb state is %d (should be %d)", hArb->state, BIP_ArbState_eAccepted ),
                    error, BIP_ERR_ARB_INTERNAL, rc );

    if (hArb->hMyArbList) {
        BIP_ArbListHandle   hArbList = hArb->hMyArbList;

        B_Mutex_Lock(hArbList->hArbListMutex);
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArbList %p: Removing Arb: %p from Accepted list" BIP_MSG_PRE_ARG, hArbList, hArb));
        BLST_Q_REMOVE(&hArbList->arbAcceptedListHead, hArb, arbAcceptedListNext);
        B_Mutex_Unlock(hArbList->hArbListMutex);
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Setting completionStatus to 0x%X" BIP_MSG_PRE_ARG, hArb, completionStatus ));

    hArb->completionStatus = completionStatus;
    if (hArb->pCompletionStatus) {
        *hArb->pCompletionStatus = hArb->completionStatus;
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Arb state is Accepted, changing to Completed" BIP_MSG_PRE_ARG, hArb ));
    hArb->state = BIP_ArbState_eCompleted;

    if (hArb->completionCallback.callback) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Queuing completionCallback" BIP_MSG_PRE_ARG, hArb ));
        BIP_Arb_AddDeferredCallback(hArb, &hArb->completionCallback);
    }

    if (hArb->hArbEvent) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Setting hArbEvent" BIP_MSG_PRE_ARG, hArb ));
        B_Event_Set(hArb->hArbEvent);
    }

    if (!hArb->apiBusy)
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Arb state is Completed and !apiBusy, changing to Idle" BIP_MSG_PRE_ARG, hArb ));
        hArb->state = BIP_ArbState_eIdle;
        if (hArb->hMyArbList) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hArbList %p: Destroying dynamic Arb: %p" BIP_MSG_PRE_ARG, hArb->hMyArbList, hArb));
            B_Mutex_Unlock(hArb->hArbMutex);
            BIP_Arb_Destroy(hArb);
        }
        else {
            B_Mutex_Unlock(hArb->hArbMutex);
            BIP_Arb_ReadyToDestroy(hArb);     /* Sets hArb->hDestroyEvent if Arb is ready to destroy. */
        }
        return rc;                        /* If ready to destroy, don't dereference hArb anymore. */
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: Returning: "BIP_STATUS_FMT"" BIP_MSG_PRE_ARG, hArb, BIP_STATUS_ARG(rc)));
    B_Mutex_Unlock(hArb->hArbMutex);
    return rc;

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "hArb %p: [Error] Returning: "BIP_STATUS_FMT"" BIP_MSG_PRE_ARG, hArb, BIP_STATUS_ARG(rc)));
    B_Mutex_Unlock(hArb->hArbMutex);
    return rc;
} /* BIP_Arb_CompleteRequest */


static void BIP_Arb_SchedulerCallbackWrapper(void *pContext)
{
    BIP_ArbDclItem      *pDclItem = (BIP_ArbDclItem *)pContext;
    BIP_CallbackDesc    *pCallbackDesc;
    BIP_ArbFactory      *pArbFactory = &g_BIP_ArbFactory;

    BDBG_OBJECT_ASSERT( pDclItem, BIP_ArbDclItem );

    pCallbackDesc = &pDclItem->completionCallback;

    BDBG_MSG(( BIP_MSG_PRE_FMT "pDclItem %p: Calling callback via Scheduler..." BIP_MSG_PRE_ARG, pDclItem));
    pCallbackDesc->callback(pCallbackDesc->context, pCallbackDesc->param);
    BDBG_MSG(( BIP_MSG_PRE_FMT "pDclItem %p: Returned from callback for Deferred Completion List item" BIP_MSG_PRE_ARG, pDclItem ));

    BDBG_MSG(( BIP_MSG_PRE_FMT "pDclItem %p: Removing item from Deferred Completion List" BIP_MSG_PRE_ARG, pDclItem ));
    B_Mutex_Lock(pArbFactory->hDclMutex);
    BLST_Q_REMOVE(&pArbFactory->dclItem_head, pDclItem, dclItem_next);
    B_Mutex_Unlock(pArbFactory->hDclMutex);

    BDBG_OBJECT_DESTROY( pDclItem, BIP_ArbDclItem );

    BDBG_MSG(( BIP_MSG_PRE_FMT "pDclItem %p: Freeing DCL item memory" BIP_MSG_PRE_ARG, pDclItem ));
    B_Os_Free( pDclItem );
}


static BIP_Status BIP_Arb_FireCallbackWithScheduler(
   BIP_ArbDclItem      *pDclItem
   )
{
    int                 rc = BIP_SUCCESS;
    BIP_TimerCreateSettings timerCreateSettings;
    BIP_TimerHandle     hTimer;

    BDBG_MSG(( BIP_MSG_PRE_FMT "pDclItem %p: Entry..." BIP_MSG_PRE_ARG, pDclItem ));

    BDBG_MSG(( BIP_MSG_PRE_FMT "pDclItem %p: Creating timer to expire immediately" BIP_MSG_PRE_ARG, pDclItem ));
    timerCreateSettings.input.callback    = BIP_Arb_SchedulerCallbackWrapper;
    timerCreateSettings.input.pContext    = pDclItem;
    timerCreateSettings.input.timeoutInMs = 0;      /* Fire callback immediately. */
    hTimer = BIP_Timer_Create(&timerCreateSettings);
    BIP_CHECK_GOTO(( hTimer !=NULL ), ( "BIP_Timer_Create() failed" ), error, BIP_ERR_INTERNAL, rc );
    BDBG_MSG(( BIP_MSG_PRE_FMT "pDclItem %p: Timer created" BIP_MSG_PRE_ARG, pDclItem ));

    BDBG_MSG(( BIP_MSG_PRE_FMT "pDclItem %p: Returning: "BIP_STATUS_FMT"" BIP_MSG_PRE_ARG, pDclItem, BIP_STATUS_ARG(rc) ));
    return rc;

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "pDclItem %p: [Error] Returning: "BIP_STATUS_FMT"" BIP_MSG_PRE_ARG, pDclItem, BIP_STATUS_ARG(rc) ));
    return rc;
} /* BIP_Arb_FireCallbackWithScheduler */


BIP_Status BIP_Arb_DoDeferred(
   BIP_ArbHandle            hArb,
   BIP_Arb_ThreadOrigin     threadOrigin
   )
{
    int     rc = BIP_SUCCESS;
    BIP_ArbDclItem      *pDclItem;
    BIP_ArbDclItem      *pDclItemNext;
    BIP_ArbFactory      *pArbFactory = &g_BIP_ArbFactory;

    BSTD_UNUSED(hArb);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Entry... threadOrigin:0x%X" BIP_MSG_PRE_ARG, threadOrigin ));

    for (;;) {
        B_Mutex_Lock(pArbFactory->hDclMutex);
        for (pDclItem = BLST_Q_FIRST(&pArbFactory->dclItem_head) ; pDclItem != NULL ; pDclItem = BLST_Q_NEXT(pDclItem, dclItem_next)) {

            BDBG_MSG(( BIP_MSG_PRE_FMT "pDclItem %p: Found Deferred Completion List item: callbackBusy: %d" BIP_MSG_PRE_ARG, pDclItem, pDclItem->callbackBusy ));

            if (!pDclItem->callbackBusy) {
                pDclItem->callbackBusy = true;
                break;
            }
        }
        if (pDclItem) {
            pDclItemNext = BLST_Q_NEXT(pDclItem, dclItem_next);
        }
        B_Mutex_Unlock(pArbFactory->hDclMutex);

        if (!pDclItem) {
            break;
        }

        BIP_CallbackDesc    *pCallbackDesc = &pDclItem->completionCallback;

        BDBG_MSG(( BIP_MSG_PRE_FMT "pDclItem %p: Calling callback for Deferred Completion List item" BIP_MSG_PRE_ARG, pDclItem ));

        if ((threadOrigin & BIP_Arb_ThreadOrigin_eMaybeUpstreamLocks) != 0) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "pDclItem %p: Calling callback using B_Os_Scheduler..." BIP_MSG_PRE_ARG, pDclItem ));
            rc = BIP_Arb_FireCallbackWithScheduler(pDclItem);
        }
        else
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "pDclItem %p: Calling callback Directly..." BIP_MSG_PRE_ARG, pDclItem ));
            pCallbackDesc->callback(pCallbackDesc->context,pCallbackDesc->param);
            BDBG_MSG(( BIP_MSG_PRE_FMT "pDclItem %p: Returned from callback for Deferred Completion List item" BIP_MSG_PRE_ARG, pDclItem ));

            BDBG_MSG(( BIP_MSG_PRE_FMT "pDclItem %p: Removing Deferred Completion List item" BIP_MSG_PRE_ARG, pDclItem ));
            B_Mutex_Lock(pArbFactory->hDclMutex);
            BLST_Q_REMOVE(&pArbFactory->dclItem_head, pDclItem, dclItem_next);
            B_Mutex_Unlock(pArbFactory->hDclMutex);

            BDBG_OBJECT_DESTROY( pDclItem, BIP_ArbDclItem );

            BDBG_MSG(( BIP_MSG_PRE_FMT "pDclItem %p: Freeing DCL item memory" BIP_MSG_PRE_ARG, pDclItem ));
            B_Os_Free( pDclItem );
        }
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "Returning: "BIP_STATUS_FMT"" BIP_MSG_PRE_ARG, BIP_STATUS_ARG(rc) ));
    return rc;
} /* BIP_Arb_DoDeferred*/
