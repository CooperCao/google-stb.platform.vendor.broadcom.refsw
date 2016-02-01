/******************************************************************************
 * (c) 2007-2015 Broadcom Corporation
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

#ifndef BIP_ARB_H
#define BIP_ARB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bip_priv.h"

/**
 * Summary:
 *
 * Description:
 *
 * (The following is still under construction.)
 *
 * The term Arb refers to an "Action Request Block".  Arbs are used to implement
 * the Application Program Interface (API) to BIP.
 *
 * The BIP API uses a Request-Reponse pattern... with the Request being
 * originated from BIP API function calls.  The Request asks BIP (or some
 * BIP object) to perform some Action.  Once a Request has been started, it
 * becomes "in progress" and remains so until it is "complete". A request
 * is complete either when the Action is successfully performed, or when
 * the Action is aborted due to some error or exception condition.  The
 * completion of a Request generates a Response which contains (at least)
 * a completion status which indicates success or the reason for terminating
 * the Request.
 *
 * For example:
 *
 *      Request: Receive an HTTP message.
 *          (Request is in progress)
 *          (time passes)
 *          Finally an HTTP message arrives.
 *      Response: Status(=success) + received HTTP message
 *
 * Or, for a non-successful case:
 *
 *      Request: Receive an HTTP message.
 *          (Request is in progress)
 *          (time passes)
 *          Network connection is closed (no HTTP message!)
 *      Response: Status(=network peer aborted) no HTTP message
 *
 * BIP provides two methods for applications to implement these
 * time-consuming Requests-Response exchanges: Synchronous and Asynchronous
 *
 * Synchronous (the easy way):
 *
 *   With this method, the API function simply waits until the Request
 *   is completed before returning, even if it takes a long time.
 *
 *      App calls BIP_HttpSocket_RecvRequest()
 *                                              Request: Receive an HTTP message.
 *                                                  (Request is in progress)
 *                                                  (time passes)
 *                                                  Finally an HTTP message arrives.
 *                                                  Message is placed in caller's buffer.
 *                                              Response: Status(=success)
 *      BIP_HttpSocket_RecvRequest() returns to App with status and message.
 *
 * Asynchronous:
 *
 *  But often, applications want API functions to return quickly, so
 *  the API function merely starts the Request, then the Response is
 *  reported through a status variable and/or a completion callback.
 *
 *      App calls BIP_HttpSocket_RecvRequest()
 *                                              Request: Receive an HTTP message.
 *      BIP_HttpSocket_RecvRequest() returns
 *      ...to App with status(=in progress)
 *      App thread continues...
 *                                                  (Request is in progress)
 *                                                  (time passes)
 *                                                  Finally an HTTP message arrives.
 *                                                  Message is place in caller's buffer.
 *                                              Response: Status(=success) + received HTTP message
 *                                              Completion status variable is set to success.
 *                                              Completion callback is called.
 *
 * Async operation is used when either:
 * 1. A completion callback has been specified in pApiSettings->asyncCallback.callback or
 * 2. An async status pointer has been specifed in pApiSettings->pAsyncStatus.
 * Otherwise, the Arb will provide normal synchronous API operation.
 *
 * Synchronous Operation:
 * The BIP_Arb_Submit function will not return until the API request
 * has completed (with either success or failure). Synchronous operation
 * can be used for either "blocking" or "non-blocking" APIs.
 *
 *      Blocking APIs:
 *      In some use cases, an application may want an API to wait until
 *      something has happened, or some information is available before it
 *      returns.  For example, when BIP_HttpSocket_RecvRequest() is called
 *      with a non-zero timeout value, it will "block" until either an incoming
 *      request has been received, or the specified timeout has expired.
 *
 *      Non-blocking APIs:
 *      In other use cases, applications want APIs to return as quickly as
 *      possible so that their thread is always available to perform other
 *      activities.  For example, when BIP_HrecvRequestApittpSocket_RecvRequest() is called
 *      with a zero timeout value (the default), it always returns immediately,
 *      either with an incoming request, or with a BIP_INF_TIMEOUT status (if
 *      a request is not available)
 *
 * Asynchronous Operation:
 * This allows an API to start a time-consuming request, then return immediately
 * (with a status of BIP_IN_PROGRESS) without waiting for the request to complete.
 * Later, when the request does complete (either with success or failure), the
 * completion is reported to the application by either setting a completion
 * status variable (pApiSettings->pAsyncStatus), and/or calling a completion
 * callback (pApiSettings->asyncCallback.callback)
 *
 **/

typedef enum BIP_Arb_ThreadOrigin
{
    BIP_Arb_ThreadOrigin_eArb           = 0x01,     /* Thread entered through API (Arb) and may have upstream locks. */
    BIP_Arb_ThreadOrigin_eUnknown       = 0x02,     /* Not sure of thread origin, assume upstream locks. .           */

    BIP_Arb_ThreadOrigin_eIoChecker     = 0x10,     /* Callback from IoChecker, no upstream locks.                   */
    BIP_Arb_ThreadOrigin_eTimer         = 0x20,     /* Callback from BIP_Timer (B_Os_Scheduler), no upstream locks.  */
    BIP_Arb_ThreadOrigin_eBipCallback   = 0x40,     /* Callback from some other BIP class, no upstream locks.        */

    /* The following are only used for checking the origin for upstream locks. */
    BIP_Arb_ThreadOrigin_eMaybeUpstreamLocks  = BIP_Arb_ThreadOrigin_eArb |
                                                BIP_Arb_ThreadOrigin_eUnknown,

    BIP_Arb_ThreadOrigin_eNoUpstreamLocks     = BIP_Arb_ThreadOrigin_eIoChecker |
                                                BIP_Arb_ThreadOrigin_eTimer     |
                                                BIP_Arb_ThreadOrigin_eBipCallback
} BIP_Arb_ThreadOrigin;


typedef struct BIP_Arb      *BIP_ArbHandle;
typedef struct BIP_ArbList  *BIP_ArbListHandle;
BDBG_OBJECT_ID_DECLARE( BIP_Arb );


/* Define a typedef for the ArbProcessor (processState) function that gets called
 * when an Arb is submitted. */
typedef   void (*BIP_ArbProcessor)(void *hObject, int value, BIP_Arb_ThreadOrigin origin);

typedef struct BIP_ArbSettings  /* BIP's API Request Block */
{
    /* Variable fields that can be changed for each API request (must lock ARB before modifying) */
    void                *hObject;                /* Object handle to be passed to the arbProcessor() function. */
    BIP_ArbProcessor     arbProcessor;           /* Function to be called to process the request   */
    bool                 waitIfBusy;             /* true (default) => Future requests wait if Arb is busy */
                                                 /* false          => Future requests fail if Arb is busy */
} BIP_ArbSubmitSettings;

/* The states for a BIP_Arb object. */
typedef enum
{
    BIP_ArbState_eUninitialized = 0,
    BIP_ArbState_eIdle,
    BIP_ArbState_eAcquired,
    BIP_ArbState_eSubmitted,
    BIP_ArbState_eAccepted,
    BIP_ArbState_eCompleted
} BIP_Arb_State ;

/* Define the private data for a BIP_Arb object. */
typedef struct BIP_Arb  /* BIP's API Request Block */
{
    BDBG_OBJECT( BIP_Arb )

    /* Constant fields set at object creation. */
    B_MutexHandle           hApiMutex;              /* Mutex for serializing concurrent API requests.    */
    B_MutexHandle           hArbMutex;              /* Mutex for accessing Arb data.                     */
    B_EventHandle           hArbEvent;              /* Used for blocking APIs.                           */
    B_EventHandle           hDestroyEvent;          /* Used for waiting until ready to destroy.          */

    BIP_ArbSubmitSettings   settings;

    /* ARB state variable and data */
    BIP_Arb_State           state;
    bool                    apiBusy;                /* true => Arb still in use by API side Arb functions. */
    bool                    rejected;               /* true => BIP_Arb_RejectRequest() was called for this Arb. */
    bool                    destroyMe;              /* true => Arb is marked for delete. */
    BIP_Status             *pCompletionStatus;      /* Variable to receive request completion status. */
    BIP_CallbackDesc        completionCallback;     /* Callback to be called at request completion.   */
    BIP_Status              completionStatus;

    BIP_ArbListHandle       hMyArbList;             /* if non-null, then handle of ArbList holding this Arb. */
    BLST_Q_ENTRY(BIP_Arb)   arbNewListNext;         /* if Arb is in an ArbList, then link to next in New list. */
    BLST_Q_ENTRY(BIP_Arb)   arbAcceptedListNext;    /* if Arb is in an ArbList, then link to next in Accepted list. */
} BIP_Arb;

#define BIP_ARB_PRINTF_FMT  \
    "[hArb=%p State=%s apiBusy=%s rejected=%s destroyMe=%s completionStatus=%s hArbList=%p]"

#define BIP_ARB_PRINTF_ARG(pObj)                                    \
    (pObj),                                                         \
    (pObj)->state==BIP_ArbState_eUninitialized   ? "Uninit"    :    \
    (pObj)->state==BIP_ArbState_eIdle            ? "Idle"      :    \
    (pObj)->state==BIP_ArbState_eAcquired        ? "Acquired"  :    \
    (pObj)->state==BIP_ArbState_eSubmitted       ? "Submitted" :    \
    (pObj)->state==BIP_ArbState_eAccepted        ? "Accepted"  :    \
    (pObj)->state==BIP_ArbState_eCompleted       ? "Completed" :    \
                                                   "<Undefined>",   \
    (pObj)->apiBusy   ? "Y" : "n",                                  \
    (pObj)->rejected  ? "Y" : "n",                                  \
    (pObj)->destroyMe ? "Y" : "n",                                  \
    BIP_StatusGetText((pObj)->completionStatus),                    \
    (pObj)->hMyArbList

/**
 * Summary:
 * API to initialize the BIP_ArbFactory that creates and destroys the BIP_Arbs.
 *
 * Description:
 *
 * Called once at BIP_Initialization.
 **/
typedef struct BIP_ArbFactoryInitSettings
{
    int unused;
}BIP_ArbFactoryInitSettings;

void BIP_ArbFactory_GetDefaultInitSettings(
    BIP_ArbFactoryInitSettings *pSettings
    );

BIP_Status  BIP_ArbFactory_Init(BIP_ArbFactoryInitSettings *pSettings);


/**
 * Summary:
 * API to uninitialize (shutdown) the BIP_ArbFactory.
 *
 * Description:
 *
 * Called once at BIP_Uninitialization (shutdown).
 **/

void BIP_ArbFactory_Uninit(void);


/**
 * Summary:
 * Macros to convert between BIP_ArbHandle and AppArb pointers.
 *
 * Description:
 *      Get the AppArb of the specified type from the specified BIP_ArbHandle (hArb).
 **/
#define BIP_APPARB_FROM_ARB(hArb, appArbType)      (BDBG_OBJECT_ASSERT((appArbType *)hArb, appArbType), (appArbType *)hArb)

/**
 * Description:
 *      Get the BIP_ArbHandle from the specified AppArb pointer. *
 **/
#define BIP_ARB_FROM_APPARB(pAppArb)      (BDBG_OBJECT_ASSERT((BIP_ArbHandle)pAppArb, BIP_Arb), (BIP_ArbHandle)pAppArb)



/*****************************************************************************
 * Client-side Arb APIs:
 *****************************************************************************/
/**
 * Summary:
 * Create an Arb object.
 *
 * Description:
 *
 **/
BIP_ArbHandle BIP_Arb_Create(
    B_MutexHandle hApiMutex,
    B_MutexHandle hArbMutex
    );

/**
 * Summary:
 * Create an AppArb object (subclass of a BIP_Arb), which is an Arb followed by application-specific data.
 *
 * Description:
 *
 * An AppArb (application-specific Arb) is defined like this:
 *
 *      typedef struct BIP_SocketRecvArb  \* AppArb for BIP_Socket_Recv() *\
 *      {
 *          BIP_Arb    arb;     \* Arb "base class". Must be the first field in struct! *\
 *          BDBG_OBJECT( BIP_SocketRecvArb )
 *
 *          \* Add application-specific fields here... *\
 *          BIP_SocketRecvSettings  settings;   \* Copy of Recv settings passed by caller. *\
 *          BLST_Q_ENTRY(BIP_SocketRecvArb) recvArbQueueNext;
 *
 *      } BIP_SocketRecvArb;
 *
 * Then an instance of an AppArb (for example a BIP_SocketRecvArb) is created like this:
 *
 *      BIP_SocketRecvArb      *pAppArb;    \* pointer to the BIP_SocketRecvArb. *\
 *      BIP_ArbHandle           hArb;       \* Handle of the associated BIP_Arb. *\
 *
 *      \* Create a BIP_SocketRecvArb AppArb. *\
 *      pAppArb =  BIP_ARB_CREATE_APPARB( BIP_SocketRecvArb );
 *      BIP_CHECK_GOTO(( pAppArb !=NULL ), ( "Memory Allocation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
 *
 *      BDBG_OBJECT_SET( pAppArb, BIP_SocketRecvArb);   \* Set the AppArb type (mandatory). *\
 *
 *      hArb = BIP_ARB_FROM_APPARB(pAppArb);            \* Convert AppArb pointer to BIP_Arb handle. *\
 *      BDBG_OBJECT_ASSERT( hArb, BIP_Arb );
 *
 *
 * And then acquired like this:
 *
 *     rc = BIP_Arb_Acquire(hArb);
 *
 * And populated with app-specific data like this:
 *
 *     pAppArb->settings = *pSocketRecvSettings;
 *     pAppArb->otherData = otherValue;
 *     ...
 *
 * And finally... submitted like this:
 *
 *     BIP_Arb_GetDefaultSubmitSettings(&arbSettings);
 *     arbSettings.hObject           = hSocket;            \* Object handle. *\
 *     arbSettings.arbProcessor      = processState;       \* Function to be called to process the Arb. *\
 *     arbSettings.waitIfBusy        = false;              \* Should simultaneous calls to this API be serialized. *\
 *
 *     \* Send the Arb to the state machine. *\
 *     rc = BIP_ArbList_Submit(hArbList, hArb, &arbSettings, &pSocketRecvSettings->api);
 *     return rc;
 *
 */
#define   BIP_ARB_CREATE_APPARB(appArbType)    ((appArbType *)BIP_Arb_CreateAppArb(sizeof (appArbType)))
BIP_ArbHandle BIP_Arb_CreateAppArb(size_t appArbSize); /* Don't call this... use BIP_ARB_CREATE_APPARB() macro. */


/**
 * Summary:
 * Destroy an Arb object.
 *
 * Description:
 *
 **/
void BIP_Arb_Destroy(
    BIP_ArbHandle hArb
    );

/**
 * Summary:
 * Take ownership of an Arb and start an API request/response cycle.
 *
 * Description:
 *
 **/
BIP_Status BIP_Arb_Acquire(
    BIP_ArbHandle hArb
    );

/**
 * Summary:
 * Get the default settings for submitting an Arb.
 *
 * Description:
 *
 **/
void BIP_Arb_GetDefaultSubmitSettings(
    BIP_ArbSubmitSettings *pSubmitSettings
    );

/**
 * Summary:
 * Submit an Arb to be processed by its server (state machine function).
 *
 * Description:
 *
 **/
BIP_Status BIP_Arb_Submit(
   BIP_ArbHandle     hArb,
   BIP_ArbSubmitSettings   *pSettings,
   BIP_ApiSettings         *pApiSettings
   );


/**
 * Summary:
 * Create an ArbList object that can hold lots of Arbs.
 *
 * Description:
 *
 **/
BIP_ArbListHandle BIP_ArbList_Create(
    void *unused
    );

/**
 * Summary:
 * Destroy an ArbList object.
 *
 * Description:
 *
 **/
void BIP_ArbList_Destroy(
    BIP_ArbListHandle   hArbList
    );

/**
 * Summary:
 * Submit an AppArb to an ArbList.
 *
 * Description:
 *
 **/
BIP_Status BIP_ArbList_Submit(
    BIP_ArbListHandle       hArbList,
    BIP_ArbHandle           hArb,
    BIP_ArbSubmitSettings  *pSettings,
    BIP_ApiSettings        *pApiSettings    /* Generic API settings. */
    );

/*****************************************************************************
 * Server-side Arb APIs:
 *****************************************************************************/

/**
 * Summary:
 * Check to see if an Arb is "Idle"... meaning that there is no API request
 * being processed for that Arb.
 *
 * Description:
 * When an Arb is "Idle", it will remain Idle until BIP_Arb_Submit() is called
 * for that Arb, at which point it will become "New".
 *
 **/
bool BIP_Arb_IsIdle(
    BIP_ArbHandle     hArb
    );

/**
 * Summary:
 * Check to see if an Arb is "New"... meaning that BIP_Arb_Submit() was called
 * to submit a new request for that Arb... and that the new request is waiting
 * to be "Accepted" (by BIP_Arb_AcceptRequest) or "Rejected" (by BIP_Arb_RejectRequest).
 *
 * Description:
 * When an Arb is "New", it will remain New until:
 * 1. BIP_Arb_AcceptRequest() is called, at which point it will become "Busy", or
 * 2. BIP_Arb_RejectRequest() is called, at which point it will become "Idle".
 *
 **/
bool BIP_Arb_IsNew(
    BIP_ArbHandle     hArb
    );

/**
 * Summary:
 * Check to see if an Arb is "Busy"... meaning that BIP_Arb_AcceptRequest() was called
 * to "Accept" a new request for that Arb... and that the new request is in progress
 * until it becomes completed by BIP_Arb_Complete().
 *
 * Description:
 * When an Arb is "Busy", it will remain Busy until BIP_Arb_CompleteRequest() is called,
 * at which point it will become "Idle".
 *
 **/
bool BIP_Arb_IsBusy(
    BIP_ArbHandle     hArb
    );

/**
 * Summary:
 * Check to see if an Arb contains a request to be processed.
 *
 * Description:
 *
 **/
BIP_Status BIP_Arb_GetRequest(
    BIP_ArbHandle     hArb
    );

/**
 * Summary:
 * Check to see if an ArbList is empty"... meaning that any Arbs that were in the
 * list have been completed and set back to the Idle state.
 *
 * Description:
 * When an Arb is "Idle", it will remain Idle until BIP_Arb_Submit() is called
 * for that Arb, at which point it will become "New".
 *
 **/

bool BIP_ArbList_IsEmpty(
    BIP_ArbListHandle
    );

/**
 * Summary:
 * Get the oldest "new" Arb from an ArbList.  A "new" Arb is one that hasn't been
 * Accepted or Rejected.
 *
 * Description:
 *
 **/

BIP_ArbHandle BIP_ArbList_GetNewArb(
    BIP_ArbListHandle   hArbList
    );

/**
 * Summary:
 * Get the oldest "accepted" Arb from an ArbList.
 *
 * Description:
 *
 **/

BIP_ArbHandle BIP_ArbList_GetAcceptedArb(
    BIP_ArbListHandle   hArbList
    );

/**
 * Summary:
 * Indicate that an Arb has begun its processing.
 *
 * Description:
 *
 **/
BIP_Status BIP_Arb_AcceptRequest(
    BIP_ArbHandle     hArb
    );

/**
 * Summary:
 * Indicate that an Arb cannot be accepted and must be failed.
 *
 * Description:
 *
 **/
BIP_Status BIP_Arb_RejectRequest(
    BIP_ArbHandle     hArb,
    BIP_Status         completionStatus
    );

/**
 * Summary:
 * Indicate that an Arb has completed, either successfully or with an error or exception.
 *
 * Description:
 *
 **/
BIP_Status BIP_Arb_CompleteRequest(
    BIP_ArbHandle     hArb,
    BIP_Status         completionStatus
    );

/**
 * Summary:
 * Add a BIP_Callback_Desc to the list of deferred callbacks for an Arb. This queues the callback
 * descriptor for later processing when BIP_Arb_DoDeferred() is called for that Arb.
 *
 * Description:
 *
 **/
BIP_Status BIP_Arb_AddDeferredCallback(
   BIP_ArbHandle     hArb,
   BIP_CallbackDesc *pCallbackDesc
   );

/**
 * Summary:
 * Do any deferred processing (e.g., calling completion callbacks) that needs to be done.
 * This API is intended to be called by the arbProcessor function after it has released it's
 * object lock(s).
 *
 * Description:
 * The "threadOrigin" argument is used to determine whether any queued callback descriptors
 * can be called directly, or whether they need to be called by a separate B_Os_Scheduler
 * (in order to prevent deadlocks).
 *
 **/
BIP_Status BIP_Arb_DoDeferred(
   BIP_ArbHandle            hArb,
   BIP_Arb_ThreadOrigin     threadOrigin
   );

/* ********************************************************************************************** */
#ifdef __cplusplus
}
#endif

#endif /* BIP_ARB_H */
