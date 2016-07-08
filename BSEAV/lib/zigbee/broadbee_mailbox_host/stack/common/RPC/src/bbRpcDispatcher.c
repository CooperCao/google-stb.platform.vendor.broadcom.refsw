/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 ******************************************************************************
/*****************************************************************************
*
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   RPC Dispatcher implementation.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


/************************* INCLUDES *****************************************************/
#include "private/bbRpcDispatcherPrivate.h"
#include "private/bbRpcTransactionPrivate.h"
#include "bbHalSystemTimer.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of identifiers of RPC Dispatcher task handlers.
 */
typedef enum _RpcDispatcherHandlerId_t
{
    /* Don't change the order of identifiers in the list. It will impact the RPC Dispatcher operation. */

    RPC_DISPATCHER_RESPONSE_TIMEOUT_EXPIRED_HANDLER_ID,     /*!< Process Response Waiting Timeout Expiration. */

    RPC_DISPATCHER_RETRY_CLIENT_TRANSACTION_HANDLER_ID,     /*!< Retry Client side Transaction processing. */

    RPC_DISPATCHER_START_NEW_LOCAL_REQUEST_HANDLER_ID,      /*!< Start New Local Client Request processing. */

    /* The following line must be the last one in the list. */

    RPC_DISPATCHER_HANDLERS_AMOUNT,                         /*!< Total number of task handlers. */

} RpcDispatcherHandlerId_t;


/**//**
 * \brief   Macro shortcut for posting RPC Dispatcher task.
 * \param[in]   dispatcher      Pointer to the Dispatcher descriptor.
 * \param[in]   handlerId       Identifier of the task handler to be posted.
 */
#define RPC_DISPATCHER_POST_TASK(dispatcher, handlerId)\
        SYS_SchedulerPostTask(&(dispatcher)->dispatcherTask, (handlerId))


/**//**
 * \brief   Enumeration of events to be processed within transaction.
 */
typedef enum _RpcDispatcherEvent_t
{
    /* Common events. */

    RPC_EVENT_SERVICE_COMPLETED,                /*!< Process Directive reported by the Service being completed. */

    RPC_EVENT_PROCESS_LOWERLEVEL_CONFIRM,       /*!< Arrange PROCESS_LOWERLEVEL_CONFIRM command. */

    /* Client Transactions events. */

    RPC_EVENT_COMPOSE_CLIENT_REQUEST,           /*!< Arrange COMPOSE_CLIENT_REQUEST command. */

    RPC_EVENT_PARSE_RESPONSE_CANDIDATE,         /*!< Arrange PARSE_RESPONSE_CANDIDATE command. */

    RPC_EVENT_PROCESS_TIMEOUT_EXPIRATION,       /*!< Arrange PROCESS_TIMEOUT_EXPIRATION command. */

    /* Server Transactions events. */

    RPC_EVENT_PARSE_CLIENT_REQUEST,             /*!< Arrange PARSE_CLIENT_REQUEST command. */

    /* Client Transactions events for Unsolicited response. */

    RPC_EVENT_COMPOSE_UNSOLICITED_RESPONSE,     /*!< Arrange COMPOSE_UNSOLICITED_RESPONSE command. */

    /* Server Transactions events for Unsolicited response. */

    RPC_EVENT_PARSE_UNSOLICITED_RESPONSE,       /*!< Arrange PARSE_UNSOLICITED_RESPONSE command. */

} RpcDispatcherEvent_t;


/************************* STATIC FUNCTIONS PROTOTYPES **********************************/
/**//**
 * \brief   Starts New Local Client Request processing.
 * \param[in]   task    Pointer to the task descriptor object embedded in the descriptor
 *  of the activated RPC Dispatcher.
 */
static void rpcDispatcherStartNewLocalRequestHandler(
                SYS_SchedulerTaskDescriptor_t *const  task);


/**//**
 * \brief   Retries Transaction processing.
 * \param[in]   task    Pointer to the task descriptor object embedded in the descriptor
 *  of the activated RPC Dispatcher.
 */
static void rpcDispatcherRetryClientTransactionHandler(
                SYS_SchedulerTaskDescriptor_t *const  task);


/**//**
 * \brief    Puts a Client Transaction into the Queue of Response Waiting Transactions and
 *  restarts the Timeout timer for it.
 * \param[in]   dispatcher      Pointer to the parent Dispatcher descriptor.
 * \param[in]   transaction     Pointer to the service field of executed Client
 *  Transaction.
 */
static void rpcDispatcherRestartResponseTimeout(
                RpcDispatcher_t  *const  dispatcher,
                RpcTransaction_t *const  transaction);


/**//**
 * \brief   Processes the Response Waiting Timeout Expiration.
 * \param[in]   task    Pointer to the task descriptor object embedded in the descriptor
 *  of the activated RPC Dispatcher.
 */
static void rpcDispatcherResponseTimeoutExpiredHandler(
                SYS_SchedulerTaskDescriptor_t *const  task);


/**//**
 * \brief   Performs step-by-step execution of a Client Transaction with the specified
 *  event.
 * \param[in]   dispatcher              Pointer to the parent Dispatcher descriptor.
 * \param[in]   transaction             Pointer to the service field of executed
 *  Client Transaction.
 * \param[in]   event                   Numeric identifier of an event to be processed.
 * \param[in]   completedDirective      Numeric identifier of the Directive reported by
 *  the Service being completed. Used only with the SERVICE_COMPLETED event. Otherwise
 *  should be set to NO_DIRECTIVE (0x0).
 * \return  Numeric identifier of the Directive returned by the Service.
 */
static RpcServiceDirective_t rpcDispatcherExecClientTransaction(
                RpcDispatcher_t     *const   dispatcher,
                RpcTransaction_t    *const   transaction,
                const RpcDispatcherEvent_t   event,
                const RpcServiceDirective_t  completedDirective);


/**//**
 * \brief   Performs step-by-step execution of a Server Transaction with the specified
 *  event.
 * \param[in]   dispatcher              Pointer to the parent Dispatcher descriptor.
 * \param[in]   transaction             Pointer to the service field of executed
 *  Server Transaction.
 * \param[in]   event                   Numeric identifier of an event to be processed.
 * \param[in]   completedDirective      Numeric identifier of the Directive reported by
 *  the Service being completed. Used only with the SERVICE_COMPLETED event. Otherwise
 *  should be set to NO_DIRECTIVE (0x0).
 * \return  Numeric identifier of the Directive returned by the Service.
 */
static RpcServiceDirective_t rpcDispatcherExecServerTransaction(
                RpcDispatcher_t     *const   dispatcher,
                RpcTransaction_t    *const   transaction,
                const RpcDispatcherEvent_t   event,
                const RpcServiceDirective_t  completedDirective);


/**//**
 * \brief   Kills and dismisses an RPC Transaction.
 * \param[in]   dispatcher      Pointer to the parent Dispatcher descriptor.
 * \param[in]   transaction     Pointer to the service field of killed Transaction.
 */
void rpcDispatcherKillTransaction(
                RpcDispatcher_t  *const  dispatcher,
                RpcTransaction_t *const  transaction);


/************************* STATIC CONSTANTS *********************************************/
/**//**
 * \brief   Array of RPC Dispatcher task handlers.
 */
static const SYS_SchedulerTaskHandler_t  rpcDispatcherTaskHandlers[RPC_DISPATCHER_HANDLERS_AMOUNT + 1] =
{
    [RPC_DISPATCHER_RESPONSE_TIMEOUT_EXPIRED_HANDLER_ID] = rpcDispatcherResponseTimeoutExpiredHandler,

    [RPC_DISPATCHER_RETRY_CLIENT_TRANSACTION_HANDLER_ID] = rpcDispatcherRetryClientTransactionHandler,

    [RPC_DISPATCHER_START_NEW_LOCAL_REQUEST_HANDLER_ID] = rpcDispatcherStartNewLocalRequestHandler,

    NULL,
};


/************************* IMPLEMENTATION ***********************************************/
/*
 * Initializes RPC Dispatcher on startup.
 */
void rpcDispatcherInit(
                RpcDispatcher_t                  *const  dispatcher,
                const SYS_SchedulerPriority_t            schedulerPriority,
                const RpcService_t               *const  servicesDescrTable,
                      RpcCustomLinkClientTrans_t *const  customLinkClientTrans,
                      RpcCustomLinkServerTrans_t *const  customLinkServerTrans,
                      RpcCustomIssueDataReq_t    *const  customIssueDataReq,
                      RpcCustomKillTrans_t       *const  customKillTrans,
                RpcTransaction_t                 *const  transactionsPullHead,
                const size_t                             transactionsPullSize,
                const size_t                             transactionDescrSize,
                const RpcDualQuotaLimit_t                transQuotaLimits[RPC_DUAL_QUOTA_CHANNELS_NUMBER],
                const RpcTransSeqNumMask_t               transSeqNumMask)
{
    SYS_DbgAssert(NULL != dispatcher, HALT_rpcDispatcherInit_NullDispatcher);
    SYS_DbgAssert(NULL != servicesDescrTable, HALT_rpcDispatcherInit_NullServicesDescrTable);
    SYS_DbgAssert(NULL != customIssueDataReq, HALT_rpcDispatcherInit_NullCustomIssueDataReq);
    SYS_DbgAssert(NULL != transactionsPullHead, HALT_rpcDispatcherInit_NullTransactionsPullHead);
    SYS_DbgAssert(transactionsPullSize > 0, HALT_rpcDispatcherInit_EmptyTransactionsPull);
    SYS_DbgAssert(transactionDescrSize >= sizeof(RpcTransaction_t), HALT_rpcDispatcherInit_InvalidTransactionDescrSize);
    SYS_DbgAssert(transSeqNumMask >= 0xFF, HALT_rpcDispatcherInit_TransSeqNumMaskTooShort);
    SYS_DbgAssertComplex(transQuotaLimits[RPC_TRANSACTION_TYPE_CLIENT] <= transactionsPullSize,
            HALT_rpcDispatcherInit_InvalidDualQuotaLimitForClientType);
    SYS_DbgAssertComplex(transQuotaLimits[RPC_TRANSACTION_TYPE_SERVER] <= transactionsPullSize,
            HALT_rpcDispatcherInit_InvalidDualQuotaLimitForServerType);
    SYS_DbgAssertComplex(
            transQuotaLimits[RPC_TRANSACTION_TYPE_CLIENT] + transQuotaLimits[RPC_TRANSACTION_TYPE_SERVER] >=
                    transactionsPullSize,
            HALT_rpcDispatcherInit_InvalidDualQuotaLimits);
    SYS_DbgAssertComplex(0x0 == (transSeqNumMask & (transSeqNumMask + 1)),
            HALT_rpcDispatcherInit_TransSeqNumMaskNotContinuous);

    SYS_DbgAssertComplex(0xDA != dispatcher->isInitialized, HALT_rpcDispatcherInit_AlreadyInitialized);

#ifdef _DEBUG_COMPLEX_
    dispatcher->isInitialized = 0xDA;
#endif

    /* Initialize simple data fields. */

    dispatcher->servicesDescrTable = servicesDescrTable;

    dispatcher->customLinkClientTrans = customLinkClientTrans;
    dispatcher->customLinkServerTrans = customLinkServerTrans;
    dispatcher->customIssueDataReq = customIssueDataReq;
    dispatcher->customKillTrans = customKillTrans;

    dispatcher->transSeqNumNext = 0;
    dispatcher->transSeqNumMask = transSeqNumMask;

    SYS_QueueResetQueue(&dispatcher->newLocalRequestsQueue);
    SYS_QueueResetQueue(&dispatcher->retryTransactionsQueue);
    SYS_QueueResetQueue(&dispatcher->waitingTransactionsQueue);

    /* Link Transaction elements within the Pull into the Unused Transactions Queue. */

    SYS_QueueDescriptor_t *unusedTransactionsQueue =        /* Pointer to the Queue of Unused Transactions. */
            &dispatcher->unusedTransactionsQueue;

    SYS_QueueResetQueue(unusedTransactionsQueue);

    uint8_t *eachTransactionElement =                       /* Pointer to each Transaction element service field in */
            (void*)&transactionsPullHead->element;          /* the Transactions Pull. */

    SYS_DbgAssertStatic(1 == sizeof(*eachTransactionElement));      /* Validate that eachTransactionElement points to */

    for (size_t i = 0; i < transactionsPullSize; i++)
    {
#ifdef _DEBUG_COMPLEX_
        RpcTransaction_t *const  transaction =              /* Pointer to the Transaction. */
                GET_PARENT_BY_FIELD(RpcTransaction_t, element, eachTransactionElement);

        transaction->parentDispatcher = NULL;
        transaction->serviceDescr = NULL;
        transaction->localRequest = NULL;

        transaction->remoteRequest.plain /* = [union with] transaction->remoteResponse.plain */ = 0;
        SYS_DbgAssertStatic(sizeof(transaction->remoteRequest) == sizeof(transaction->remoteResponse));
#endif

        SYS_QueuePutQueueElementToHead(unusedTransactionsQueue, (void*)eachTransactionElement);
        eachTransactionElement += transactionDescrSize;
    }

    /* Initialize the Dual Quota over the Unused Transactions Queue. */

    rpcDualQuotaInit(&dispatcher->transQuota, transQuotaLimits);

    /* Initialize the Dispatcher Task. */

    dispatcher->dispatcherTask =
            (const SYS_SchedulerTaskDescriptor_t){
                .qElem = { .nextElement = NULL },
                .handlers = rpcDispatcherTaskHandlers,
                .handlersMask = 0x0,
            };
    dispatcher->dispatcherTask.priority = schedulerPriority;

    /* Initialize the Response Waiting Timeout Timer. */

    dispatcher->timeoutTimer =
            (const SYS_TimeoutTask_t){
                .service = {
                    .queueElement = { .nextElement = NULL },
                    .flags = 0x0,
                    .counter = 0,
                },
                .timeout = 0,
                .handlerId = RPC_DISPATCHER_RESPONSE_TIMEOUT_EXPIRED_HANDLER_ID,
            };
    dispatcher->timeoutTimer.taskDescriptor = &dispatcher->dispatcherTask;

    /* Initialize all the linked Services. */

    const RpcService_t *eachServiceDescr =                  /* Pointer to each Service descriptor in the table. */
            servicesDescrTable;

    while (1)
    {
        if (RPC_IS_VALID_SERVICE_DESCRIPTOR(eachServiceDescr))          /* Initialize the whole Service. */
            RPC_GET_SERVICE_HANDLER(eachServiceDescr)(NULL, RPC_COMMAND_INITIALIZE_SERVICE);

        if (RPC_IS_SERVICES_TABLE_EOT(eachServiceDescr))
            break;

        eachServiceDescr++;
    };
}


/*
 * Accepts New Local Client Request for processing with the specified RPC Dispatcher.
 */
void rpcDispatcherAcceptNewLocalRequest(
                RpcDispatcher_t   *const  dispatcher,
                RpcLocalRequest_t *const  localRequest)
{
    SYS_DbgAssert(NULL != dispatcher, HALT_rpcDispatcherAcceptNewLocalRequest_NullDispatcher);
    SYS_DbgAssert(NULL != localRequest, HALT_rpcDispatcherAcceptNewLocalRequest_NullLocalRequest);

    SYS_DbgAssertComplex(0xDA == dispatcher->isInitialized, HALT_rpcDispatcherAcceptNewLocalRequest_NotInitialized);

    SYS_QueuePutQueueElementToTail(&dispatcher->newLocalRequestsQueue, &localRequest->element);
    RPC_DISPATCHER_POST_TASK(dispatcher, RPC_DISPATCHER_START_NEW_LOCAL_REQUEST_HANDLER_ID);
}


/*
 * Starts New Local Client Request processing.
 */
void rpcDispatcherStartNewLocalRequestHandler(
                SYS_SchedulerTaskDescriptor_t *const  task)
{
    SYS_DbgAssert(NULL != task, HALT_rpcDispatcherStartNewLocalRequestHandler_NullTask);

    RpcDispatcher_t *const  dispatcher =                        /* Pointer to the Dispatcher descriptor. */
            GET_PARENT_BY_FIELD(RpcDispatcher_t, dispatcherTask, task);

    /* Take the first Local Client Request from the queue. */

    SYS_QueueDescriptor_t *const  newLocalRequestsQueue =       /* Pointer to the New Local Requests Queue. */
            &dispatcher->newLocalRequestsQueue;

    SYS_QueueElement_t *const  firstRequestElement =            /* Pointer to the first New Local Request service */
            SYS_QueueGetQueueHead(newLocalRequestsQueue);       /* field in the queue; or NULL if queue is empty. */

    if (NULL == firstRequestElement)
        return;

    const RpcLocalRequest_t *const  localRequest =              /* Pointer to the Client Request service field. */
            GET_PARENT_BY_FIELD(RpcLocalRequest_t, element, firstRequestElement);

    /* Try to allocate a new Client Transaction and assign the next (or forced) Sequence Number to it. */

    RpcTransaction_t *const  transaction =                      /* Pointer to the just allocated Transaction; or NULL */
            rpcTransactionAlloc(dispatcher, RPC_TRANSACTION_TYPE_CLIENT,        /* if Transaction was not allocated. */
                    localRequest->forceTsn, localRequest->forcedTsn);

    if (NULL == transaction)
        return;     /* Task will be reposted when the first busy Transaction is freed by another completed service. */

    /* Extract currently processed Local Client Request from the queue, and if there are more Requests to be processed,
     * repost task to proceed later with the next Request in the queue. */

    SYS_QueueRemoveHeadElement(newLocalRequestsQueue);
    if (!SYS_QueueIsEmpty(newLocalRequestsQueue))
        RPC_DISPATCHER_POST_TASK(dispatcher, RPC_DISPATCHER_START_NEW_LOCAL_REQUEST_HANDLER_ID);

    /* Link new Client Transaction to the Local Client Request; initialize Transaction. */

    const RpcServiceId_t  requestedServiceId =                  /* Unique identifier of the requested Service. */
            localRequest->serviceId;

    const RpcService_t *requestedServiceDescr =                 /* Pointer to the requested Service descriptor. Start */
            dispatcher->servicesDescrTable;                     /* searching from the first record in the table. */

    while (!RPC_IS_SERVICES_TABLE_EOT(requestedServiceDescr))
    {
        if (requestedServiceId == requestedServiceDescr->id)
            break;
        requestedServiceDescr++;
    }

    SYS_DbgAssertComplex(!RPC_IS_SERVICES_TABLE_EOT(requestedServiceDescr),
            HALT_rpcDispatcherStartNewLocalRequestHandler_RequestedServiceNotImplemented);
    SYS_DbgAssertComplex(RPC_IS_VALID_SERVICE_DESCRIPTOR(requestedServiceDescr),
            HALT_rpcDispatcherStartNewLocalRequestHandler_NullServiceHandler);

    SYS_DbgAssertComplex(NULL == transaction->serviceDescr,
            HALT_rpcDispatcherStartNewLocalRequestHandler_NonEmptyServiceDescr);
    SYS_DbgAssertComplex(NULL == transaction->localRequest,
            HALT_rpcDispatcherStartNewLocalRequestHandler_NonEmptyLocalRequest);
    SYS_DbgAssertComplex(0 == transaction->remoteRequest.plain,
            HALT_rpcDispatcherStartNewLocalRequestHandler_NonEmptyRemoteRequest);
    SYS_DbgAssertComplex(0 == transaction->remoteResponse.plain,
            HALT_rpcDispatcherStartNewLocalRequestHandler_NonEmptyRemoteResponse);

    transaction->serviceDescr = requestedServiceDescr;
    transaction->localRequest = localRequest;
    transaction->timeoutPeriod = 0;
    transaction->flags = 0x0;

    /* Link and initialize the application-specific part. */

    const RpcCustomLinkClientTrans_t *customLinkClientTrans =   /* Entry point to the application-specific link- */
            dispatcher->customLinkClientTrans;                  /* client-transaction function. */

    if (NULL != customLinkClientTrans)
        customLinkClientTrans(transaction);

    /* Start Client Transaction processing. */

    rpcDispatcherExecClientTransaction(dispatcher, transaction,
            !localRequest->isUnsolResp ? RPC_EVENT_COMPOSE_CLIENT_REQUEST : RPC_EVENT_COMPOSE_UNSOLICITED_RESPONSE,
            RPC_NO_DIRECTIVE);
}


/*
 * Reposts the Starts New Local Client Request processing when an RPC Transaction becomes
 * free.
 */
void rpcDispatcherFreeTransactionCallback(
                RpcDispatcher_t *const  dispatcher)
{
    SYS_DbgAssert(NULL != dispatcher, HALT_rpcDispatcherFreeTransactionCallback_NullDispatcher);
    SYS_DbgAssertComplex(!SYS_QueueIsEmpty(&dispatcher->unusedTransactionsQueue),
            HALT_rpcDispatcherFreeTransactionCallback_NoUnusedTransactions);

    if (!SYS_QueueIsEmpty(&dispatcher->newLocalRequestsQueue))
        RPC_DISPATCHER_POST_TASK(dispatcher, RPC_DISPATCHER_START_NEW_LOCAL_REQUEST_HANDLER_ID);
}


/*
 * Retries Transaction processing.
 */
void rpcDispatcherRetryClientTransactionHandler(
                SYS_SchedulerTaskDescriptor_t *const  task)
{
    SYS_DbgAssert(NULL != task, HALT_rpcDispatcherRetryClientTransactionHandler_NullTask);

    RpcDispatcher_t *const  dispatcher =                        /* Pointer to the Dispatcher descriptor. */
            GET_PARENT_BY_FIELD(RpcDispatcher_t, dispatcherTask, task);

    SYS_QueueDescriptor_t *const  retryTransactionsQueue =      /* Pointer to the Retry Transactions Queue. */
            &dispatcher->retryTransactionsQueue;

    SYS_QueueElement_t *const  firstTransactionElement =        /* Pointer to the first Retry Transaction service */
            SYS_QueueGetQueueHead(retryTransactionsQueue);      /* field in the queue. */

    SYS_DbgAssert(NULL != firstTransactionElement,
            HALT_rpcDispatcherRetryClientTransactionHandler_EmptyRetryTransactionsQueue);

    RpcTransaction_t *const  transaction =                      /* Pointer to the Transaction. */
            GET_PARENT_BY_FIELD(RpcTransaction_t, element, firstTransactionElement);

    SYS_QueueRemoveHeadElement(firstTransactionElement);
    if (!SYS_QueueIsEmpty(retryTransactionsQueue))
        RPC_DISPATCHER_POST_TASK(dispatcher, RPC_DISPATCHER_RETRY_CLIENT_TRANSACTION_HANDLER_ID);

    const RpcLocalRequest_t *const  localRequest =              /* Pointer to the Client Request service field. */
            transaction->localRequest;

    SYS_DbgAssert(NULL != localRequest, HALT_rpcDispatcherRetryClientTransactionHandler_NullLocalRequest);

    rpcDispatcherExecClientTransaction(dispatcher, transaction,
            !localRequest->isUnsolResp ? RPC_EVENT_COMPOSE_CLIENT_REQUEST : RPC_EVENT_COMPOSE_UNSOLICITED_RESPONSE,
            RPC_NO_DIRECTIVE);
}


/*
 * Accepts COMPLETED signal from the Service being waited for completion.
 */
void rpcDispatcherAcceptServiceCompleted(
                RpcDispatcher_t      *const  dispatcher,
                RpcTransaction_t     *const  transaction,
                const RpcServiceDirective_t  completedDirective)
{
    SYS_DbgAssert(NULL != dispatcher, HALT_rpcDispatcherAcceptServiceCompleted_NullDispatcher);
    SYS_DbgAssert(NULL != transaction, HALT_rpcDispatcherAcceptServiceCompleted_NullTransaction);
    SYS_DbgAssertComplex(dispatcher == transaction->parentDispatcher,
            HALT_rpcDispatcherAcceptServiceCompleted_NotRelated);

    if (RPC_TRANSACTION_TYPE_CLIENT == transaction->transType)
        rpcDispatcherExecClientTransaction(dispatcher, transaction, RPC_EVENT_SERVICE_COMPLETED, completedDirective);
    else /* if (RPC_TRANSACTION_TYPE_SERVER == transaction->transType) */
        rpcDispatcherExecServerTransaction(dispatcher, transaction, RPC_EVENT_SERVICE_COMPLETED, completedDirective);
}


/*
 * Accepts confirmation on request to transmit data from the lower-level layer conducted
 * by the application specific callback function.
 */
void rpcDispatcherAcceptDataConf(
                RpcDispatcher_t  *const  dispatcher,
                RpcTransaction_t *const  transaction)
{
    SYS_DbgAssert(NULL != dispatcher, HALT_rpcDispatcherAcceptDataConf_NullDispatcher);
    SYS_DbgAssert(NULL != transaction, HALT_rpcDispatcherAcceptDataConf_NullTransaction);
    SYS_DbgAssertComplex(dispatcher == transaction->parentDispatcher, HALT_rpcDispatcherAcceptDataConf_NotRelated);

    if (RPC_TRANSACTION_TYPE_CLIENT == transaction->transType)
        rpcDispatcherExecClientTransaction(dispatcher, transaction, RPC_EVENT_PROCESS_LOWERLEVEL_CONFIRM,
                RPC_NO_DIRECTIVE);
    else /* if (RPC_TRANSACTION_TYPE_SERVER == transaction->transType) */
        rpcDispatcherExecServerTransaction(dispatcher, transaction, RPC_EVENT_PROCESS_LOWERLEVEL_CONFIRM,
                RPC_NO_DIRECTIVE);
}


/*
 * Accepts received data indication from the lower-level layer conducted by the
 * application specific handling function.
 */
bool rpcDispatcherAcceptDataInd(
                RpcDispatcher_t   *const  dispatcher,
                const RpcRemoteRequest_t  remoteRequest,
                const bool                mayBeRequest,
                const bool                mayBeResponse,
                const RpcServiceId_t      serviceId,
                const RpcTransSeqNum_t    transSeqNum)
{
    SYS_DbgAssert(NULL != dispatcher, HALT_rpcDispatcherAcceptDataInd_NullDispatcher);
    SYS_DbgAssertComplex(0xDA == dispatcher->isInitialized, HALT_rpcDispatcherAcceptDataInd_NotInitialized);
    SYS_DbgAssertComplex(mayBeRequest || mayBeResponse, HALT_rpcDispatcherAcceptDataInd_NeitherRequestNorResponse);

    /* If the indicated frame is a Server Response, try to find a corresponding Transaction in the queue of Transactions
     * Waiting for Response. If there is a Client Transaction with the same Service Identifier and Sequence Number, try
     * to process the received Server Response as a Candidate in Responses to the found Transaction. The linked Service
     * will investigate the indicated frame if it may be a Response being Waited. If it finds that the indicated Server
     * Response frame is indeed an Unsolicited Response, the Service will report UNSOLICITED_RESPONSE - in this case the
     * Response will be processed again but within new empty Server Transaction (just as for the case of Client Request
     * received). In the case when the indicated Response is not related to the found Client Transaction (in spite of
     * the same Service Identifier and Transaction Sequence Number) the Service reports ALIEN_RESPONSE - in this case
     * Dispatcher will try to find next Transaction in the queue with the appropriate Service Identifier and Sequence
     * Number; and if there is one, it will try to process the indicated Server Response within this next Transaction;
     * and so on until there are Transactions remaining in the queue and Service is returning the ALIEN_RESPONSE. If
     * finally no proper Transaction found and Service did not return other report than ALIEN_RESPONSE, Dispatcher will
     * try to process the indicated Server Response as an Unsolicited Response (see the second part of the function). In
     * the case when the indicated Response has completely invalid format the Service reports BROKEN_RESPONSE - in this
     * case Dispatcher will reject this frame, and this function returns FALSE. But if Service reports other status than
     * ALIEN_RESPONSE, or UNSOLICITED_RESPONSE, or BROKEN_RESPONSE - the indicated Server Response is considered to be
     * processed by the Service within the found Transaction. In this case function returns TRUE.
     */

    if (mayBeResponse)
    {
        const RpcTransSeqNum_t  transSeqNumMasked =             /* Actual Sequence Number of the indicated Response. */
                transSeqNum & (dispatcher->transSeqNumMask);

        const SYS_QueueElement_t *eachWaitingTransaction =                      /* Element service field of each */
                SYS_QueueGetQueueHead(&dispatcher->waitingTransactionsQueue);   /* Transaction Waiting for Response. */

        while (NULL != eachWaitingTransaction)
        {
            RpcTransaction_t *const  transaction =              /* Pointer to the Waiting Transaction service field. */
                    GET_PARENT_BY_FIELD(RpcTransaction_t, element, eachWaitingTransaction);

            SYS_DbgAssertComplex(transaction->isWaitingResponse, HALT_rpcDispatcherAcceptDataInd_NoResponseTransaction);
            SYS_DbgAssertComplex(NULL != transaction->serviceDescr, HALT_rpcDispatcherAcceptDataInd_EmptyTransaction);

            if (transSeqNumMasked == transaction->transSeqNum && serviceId == transaction->serviceDescr->id)
            {
                /* Link the candidate Client Transaction to the received Remote Response. */

                transaction->remoteResponse = remoteRequest;

                /* Process the Remote Response Candidate. */

                RpcServiceDirective_t  directive;               /* Directive returned by the Service. */

                directive = rpcDispatcherExecClientTransaction(dispatcher, transaction,
                        RPC_EVENT_PARSE_RESPONSE_CANDIDATE, RPC_NO_DIRECTIVE);

                if (RPC_DIRECTIVE_BROKEN_RESPONSE == directive)
                    return FALSE;

                if (RPC_DIRECTIVE_UNSOLICITED_RESPONSE == directive)
                    break;

                if (RPC_DIRECTIVE_ALIEN_RESPONSE != directive)
                    return TRUE;
            }

            eachWaitingTransaction = SYS_QueueGetNextQueueElement(eachWaitingTransaction);
        }
    }

    /* If the indicated frame is a Client Request, or if it is (or if it is supposed to be) a Unsolicited Server
     * Response, try to link the appropriate Service and allocate empty Server Transaction for this frame and start
     * processing it. If the requested Service is not implemented, try to find the Default Service. If finally neither
     * requested Service nor Default service are found, or if it fails to allocate an empty Server Transaction, the
     * frame is dropped, and this function returns FALSE (in this case the caller shall dismiss the dynamic memory
     * allocated by the lower-level layer for the indication payload). If Service is found and Transaction is allocated,
     * the received frame is put for processing. In both cases, for Client Request and Unsolicited Server Response, it
     * is processed in the same way - the linked Service decides how to process the frame and if to respond on it.
     * Anyway, in this case the indicated frame is considered to be processed, and this function returns TRUE.
     */

    /* Try to find the requested Service or the Default Service. If they are not implemented, then drop the frame and
     * return FALSE. */

    const RpcService_t *requestedServiceDescr =                 /* Pointer to the requested Service descriptor. Start */
            dispatcher->servicesDescrTable;                     /* searching from the first record in the table. */

    while (!RPC_IS_SERVICES_TABLE_EOT(requestedServiceDescr))
    {
        if (serviceId == requestedServiceDescr->id)
            break;
        requestedServiceDescr++;
    }

    if (RPC_IS_SERVICES_TABLE_EOT(requestedServiceDescr) && !RPC_IS_VALID_SERVICE_DESCRIPTOR(requestedServiceDescr))
        return FALSE;

    SYS_DbgAssertComplex(RPC_IS_VALID_SERVICE_DESCRIPTOR(requestedServiceDescr),
            HALT_rpcDispatcherAcceptDataInd_NullServiceHandler);

    /* Try to allocate new Server Transaction. If failed to allocate, then drop the frame and return FALSE. */

    RpcTransaction_t *const  transaction =                      /* Pointer to the just allocated Transaction; or NULL */
            rpcTransactionAlloc(dispatcher, RPC_TRANSACTION_TYPE_SERVER,        /* if Transaction was not allocated. */
                    /*forceTsn*/ TRUE, transSeqNum);

    if (NULL == transaction)
        return FALSE;

    /* Link new Server Transaction to the received Remote Request; initialize Transaction. */

    SYS_DbgAssertComplex(NULL == transaction->serviceDescr,
            HALT_rpcDispatcherAcceptDataInd_NonEmptyServiceDescr);
    SYS_DbgAssertComplex(NULL == transaction->localRequest,
            HALT_rpcDispatcherAcceptDataInd_NonEmptyLocalRequest);
    SYS_DbgAssertComplex(0 == transaction->remoteRequest.plain,
            HALT_rpcDispatcherAcceptDataInd_NonEmptyRemoteRequest);
    SYS_DbgAssertComplex(0 == transaction->remoteResponse.plain,
            HALT_rpcDispatcherAcceptDataInd_NonEmptyRemoteResponse);

    transaction->serviceDescr = requestedServiceDescr;
    transaction->remoteRequest = remoteRequest;
    transaction->timeoutPeriod = 0;
    transaction->flags = 0x0;

    /* Link and initialize the application-specific part. */

    const RpcCustomLinkServerTrans_t *customLinkServerTrans =   /* Entry point to the application-specific link- */
            dispatcher->customLinkServerTrans;                  /* server-transaction function. */

    if (NULL != customLinkServerTrans)
        customLinkServerTrans(transaction);

    /* Start Server Transaction processing. */

    rpcDispatcherExecServerTransaction(dispatcher, transaction,
            mayBeRequest ? RPC_EVENT_PARSE_CLIENT_REQUEST : RPC_EVENT_PARSE_UNSOLICITED_RESPONSE,
            RPC_NO_DIRECTIVE);

    return TRUE;
}


/*
 * Puts a Client Transaction into the Queue of Response Waiting Transactions and restarts
 * the Timeout timer for it.
 */
void rpcDispatcherRestartResponseTimeout(
                RpcDispatcher_t  *const  dispatcher,
                RpcTransaction_t *const  transaction)
{
    SYS_DbgAssert(NULL != dispatcher, HALT_rpcDispatcherRestartResponseTimeout_NullDispatcher);
    SYS_DbgAssert(NULL != transaction, HALT_rpcDispatcherRestartResponseTimeout_NullTransaction);
    SYS_DbgAssertComplex(dispatcher == transaction->parentDispatcher,
            HALT_rpcDispatcherRestartResponseTimeout_NotRelated);
    SYS_DbgAssertComplex(RPC_TRANSACTION_TYPE_CLIENT == transaction->transType,
            HALT_rpcDispatcherRestartResponseTimeout_NotClientType);

    if (!transaction->isWaitingResponse)
    {
        SYS_DbgAssertComplex(NULL == SYS_QueueFindParentElement(&dispatcher->waitingTransactionsQueue,
                                                                &transaction->element),
                HALT_rpcDispatcherRestartResponseTimeout_TransactionAlreadyInQueue);

        SYS_QueuePutQueueElementToTail(&dispatcher->waitingTransactionsQueue, &transaction->element);

        transaction->isWaitingResponse = TRUE;
    }
    else
    {
        SYS_DbgAssertComplex(NULL != SYS_QueueFindParentElement(&dispatcher->waitingTransactionsQueue,
                                                                &transaction->element),
                HALT_rpcDispatcherRestartResponseTimeout_TransactionNotInQueue);
    }

    const SYS_Timediff_t  timeoutPeriod =                       /* Value of this Transaction timeout period. */
            transaction->timeoutPeriod;

    SYS_DbgAssert(timeoutPeriod > 0, HALT_rpcDispatcherRestartResponseTimeout_ZeroTimeoutPeriod);

    transaction->expirationTimestamp = HAL_GetSystemTime() + timeoutPeriod;

    /* Engage the timeout timer for the whole timeout period of this Transaction, but only if such a period is shorter
     * (closer to the current moment) than the current value of the remainder to expiration of the originally assigned
     * period, or if the timer is currently disabled (or just has expired). */

    SYS_TimeoutTask_t *const  timeoutTimer =                    /* Pointer to the Timeout timer descriptor. */
            &dispatcher->timeoutTimer;

    const SYS_Timediff_t  remainPeriod =                        /* Value of remaining timeout period. */
            SYS_TimeoutRemain(&timeoutTimer->service);

    /* When the remaining period is zero, it is not clear either the timer is disabled or it is enabled but just has
     * expired. For this particular case post the timeout task in order not to loose possible timeout event. */
    if (0 == remainPeriod)
        RPC_DISPATCHER_POST_TASK(dispatcher, RPC_DISPATCHER_RESPONSE_TIMEOUT_EXPIRED_HANDLER_ID);

    if (0 == remainPeriod || timeoutPeriod < remainPeriod)
    {
        SYS_TimeoutTaskRemove(timeoutTimer);
        timeoutTimer->timeout = timeoutPeriod;
        SYS_TimeoutTaskPost(timeoutTimer, TIMEOUT_TASK_ONE_SHOT_MODE);
    }
}


/*
 * Processes the Response Waiting Timeout Expiration.
 */
void rpcDispatcherResponseTimeoutExpiredHandler(
                SYS_SchedulerTaskDescriptor_t *const  task)
{
    SYS_DbgAssert(NULL != task, HALT_rpcDispatcherTimeoutExpiredHandler_NullTask);

    RpcDispatcher_t *const  dispatcher =                        /* Pointer to the Dispatcher descriptor. */
            GET_PARENT_BY_FIELD(RpcDispatcher_t, dispatcherTask, task);

    SYS_QueueDescriptor_t *const  waitingTransactionsQueue =    /* Pointer to the Response Waiting Transaction Queue. */
            &dispatcher->waitingTransactionsQueue;

    /* Assure that queue is not empty. Otherwise just exit. */

    if (SYS_QueueIsEmpty(waitingTransactionsQueue))
        return;

    /* Look for the first expired Transaction in the queue of Transactions waiting for Response. Queue is not empty, and
     * there are two cases:
     * 1) As soon as the first one expired Transaction is found in the queue, then:
     *  - if this Transaction is not the last one in the queue, then repost the timeout expired task in order to process
     *      the queue again immediately, but in a different execution flow,
     *  - remove this expired Transaction from the queue,
     *  - disable indication of new received Responses to this expired Transaction,
     *  - call the linked Service with the PROCESS_TIMEOUT_EXPIRATION command,
     *  - exit. The rest of the queue will be processed immediately but in a different execution flow;
     * 2) If no expired transactions found in the queue (queue is not empty), then:
     *  - during the queue processing the shortest reminder of timeout periods of all Transactions in the queue is
     *      found,
     *  - engage the timeout timer for the period equal to the shortest reminder found.
     */

    const SYS_Time_t  currentTimestamp =                        /* Current timestamp, in ms. */
            HAL_GetSystemTime();

    SYS_Timediff_t  shortestReminder =                          /* Shortest timeout reminder, in ms. Finally valid if */
            INT32_MAX;                                          /* no expired Transactions are found. */

    SYS_DbgAssertStatic(4 == sizeof(SYS_Timediff_t));           /* Validate if it's legal to use the INT32_MAX. */

    SYS_QueueElement_t *firstExpiredTransaction =               /* Element service field of the first Transaction */
            SYS_QueueGetQueueHead(waitingTransactionsQueue);    /* Waiting for Response found to be Expired. */

    while (NULL != firstExpiredTransaction)
    {
        SYS_QueueElement_t *const  nextTransaction =            /* Element service field of the next Transaction.*/
                SYS_QueueGetNextQueueElement(firstExpiredTransaction);

        RpcTransaction_t *const  transaction =                  /* Pointer to the Waiting Transaction service field. */
                GET_PARENT_BY_FIELD(RpcTransaction_t, element, firstExpiredTransaction);

        SYS_DbgAssertComplex(transaction->isWaitingResponse,
                HALT_rpcDispatcherResponseTimeoutExpiredHandler_NoResponseTransaction);

        const SYS_Timediff_t  reminder =                        /* The reminder of the timeout period, signed, in ms. */
                transaction->expirationTimestamp - currentTimestamp;

        if (reminder <= 0)
        {
            if (NULL != nextTransaction)
                RPC_DISPATCHER_POST_TASK(dispatcher, RPC_DISPATCHER_RESPONSE_TIMEOUT_EXPIRED_HANDLER_ID);

            SYS_QueueRemoveQueueElement(waitingTransactionsQueue, firstExpiredTransaction);

            transaction->isWaitingResponse = FALSE;

            rpcDispatcherExecClientTransaction(dispatcher, transaction, RPC_EVENT_PROCESS_TIMEOUT_EXPIRATION,
                    RPC_NO_DIRECTIVE);

            return;     /* Proceed with the rest of the queue, if needed, in a different execution flow. */
        }
        else if (reminder < shortestReminder)
        {
            shortestReminder = reminder;
        }

        firstExpiredTransaction = nextTransaction;
    }

    /* Engage the timeout timer for the shortest remainder period. */

    SYS_TimeoutTask_t *const  timeoutTimer =                    /* Pointer to the Timeout timer descriptor. */
            &dispatcher->timeoutTimer;

    /* Recall the timer prior to engage it again, to prevent possible error in the case when a task with priority higher
     * then the current one might already engage it. */
    SYS_TimeoutTaskRemove(timeoutTimer);

    timeoutTimer->timeout = shortestReminder;
    SYS_TimeoutTaskPost(timeoutTimer, TIMEOUT_TASK_ONE_SHOT_MODE);
}


/*
 * Performs step-by-step execution of a Client Transaction with the specified event.
 */
RpcServiceDirective_t rpcDispatcherExecClientTransaction(
                RpcDispatcher_t     *const   dispatcher,
                RpcTransaction_t    *const   transaction,
                const RpcDispatcherEvent_t   event,
                const RpcServiceDirective_t  completedDirective)
{
    SYS_DbgAssert(NULL != dispatcher, HALT_rpcDispatcherExecClientTransaction_NullDispatcher);
    SYS_DbgAssert(NULL != transaction, HALT_rpcDispatcherExecClientTransaction_NullTransaction);
    SYS_DbgAssertComplex(RPC_TRANSACTION_TYPE_CLIENT == transaction->transType,
            HALT_rpcDispatcherExecClientTransaction_InvalidTransactionType);
    SYS_DbgAssertComplex(dispatcher == transaction->parentDispatcher,
            HALT_rpcDispatcherExecClientTransaction_NotRelated);
    SYS_DbgAssertComplex(NULL != transaction->serviceDescr, HALT_rpcDispatcherExecClientTransaction_EmptyTransaction);

    const RpcLocalRequest_t *const  localRequest =          /* Pointer to the Client Request service field. */
            transaction->localRequest;

    SYS_DbgAssert(NULL != localRequest, HALT_rpcDispatcherExecClientTransaction_NullLocalRequest);

    const Bool8_t  isUnsolResp =                            /* TRUE for server unsolicited response, FALSE for client */
            localRequest->isUnsolResp;                      /* request. */

    const RpcServiceHandler_t *const serviceHandler =       /* Entry point to the linked Service handler function. */
            RPC_GET_SERVICE_HANDLER(transaction->serviceDescr);

    SYS_DbgAssertComplex(NULL != serviceHandler, HALT_rpcDispatcherExecClientTransaction_NullServiceHandler);

    RpcServiceDirective_t  directive =                      /* Directive returned by the Service. */
            RPC_DIRECTIVE_PROHIBITED_CODE;

    switch (event)
    {
        case RPC_EVENT_COMPOSE_CLIENT_REQUEST:
            SYS_DbgAssert(FALSE == isUnsolResp,
                    HALT_rpcDispatcherExecClientTransaction_COMPOSE_CLIENT_REQUEST_InvalidRequestType);
            directive = serviceHandler(transaction, RPC_COMMAND_COMPOSE_CLIENT_REQUEST);
            goto case_RPC_DIRECTIVE_FROM_COMPOSE_CLIENT_REQUEST;

        case RPC_EVENT_COMPOSE_UNSOLICITED_RESPONSE:
            SYS_DbgAssert(FALSE != isUnsolResp,
                    HALT_rpcDispatcherExecClientTransaction_COMPOSE_UNSOLICITED_RESPONSE_InvalidRequestType);
            directive = serviceHandler(transaction, RPC_COMMAND_COMPOSE_UNSOLICITED_RESPONSE);
            goto case_RPC_DIRECTIVE_FROM_COMPOSE_UNSOLICITED_RESPONSE;

        case RPC_EVENT_SERVICE_COMPLETED:
            directive = completedDirective;
            goto case_RPC_DIRECTIVE_FROM_COMPOSE_CLIENT_REQUEST;    /* and _FROM_COMPOSE_UNSOLICITED_RESPONSE as well */

        case_RPC_DIRECTIVE_FROM_COMPOSE_CLIENT_REQUEST:
        case_RPC_DIRECTIVE_FROM_COMPOSE_UNSOLICITED_RESPONSE:
            SYS_DbgAssertComplex(!transaction->isDataConfirmed,
                    HALT_rpcDispatcherExecClientTransaction_COMPOSE_CLIENT_REQUEST_InvalidDataConfirmedFlag);
            SYS_DbgAssertComplex(!transaction->isWaitingResponse,
                    HALT_rpcDispatcherExecClientTransaction_COMPOSE_CLIENT_REQUEST_InvalidWaitResponseFlag);
            switch (directive)
            {
                case RPC_DIRECTIVE_WAIT_SERVICE:
                    /* Just return and wait for COMPLETED signal from the Service. */
                    break;

                case RPC_DIRECTIVE_NO_MEMORY:
                    SYS_QueuePutQueueElementToTail(&dispatcher->retryTransactionsQueue, &transaction->element);
                    RPC_DISPATCHER_POST_TASK(dispatcher, RPC_DISPATCHER_RETRY_CLIENT_TRANSACTION_HANDLER_ID);
                    break;

                case RPC_DIRECTIVE_ABORT_REQUEST:
                    goto case_RPC_EVENT_KILL_CLIENT_TRANSACTION;

                case RPC_DIRECTIVE_WAIT_RESPONSE:
                    SYS_DbgAssert(FALSE == isUnsolResp,
                            HALT_rpcDispatcherExecClientTransaction_DIRECTIVE_WAIT_RESPONSE_InvalidRequestType);
                    rpcDispatcherRestartResponseTimeout(dispatcher, transaction);
                    goto case_RPC_EVENT_CLIENT_REQUEST_SEND_DATA;

                case RPC_DIRECTIVE_DONT_WAIT_RESPONSE:
                    goto case_RPC_EVENT_CLIENT_REQUEST_SEND_DATA;

                case_RPC_EVENT_CLIENT_REQUEST_SEND_DATA:
                    dispatcher->customIssueDataReq(transaction);
                    break;

                default:
                    SYS_DbgHalt(HALT_rpcDispatcherExecClientTransaction_COMPOSE_CLIENT_REQUEST_InvalidDirective);
                    break;
            }
            break; /* case_RPC_DIRECTIVE_FROM_COMPOSE_CLIENT_REQUEST: */

        case RPC_EVENT_PROCESS_LOWERLEVEL_CONFIRM:
            SYS_DbgAssertComplex(!transaction->isDataConfirmed,
                    HALT_rpcDispatcherExecClientTransaction_PROCESS_LOWERLEVEL_CONFIRM_AlreadyConfirmed);
            transaction->isDataConfirmed = TRUE;
            directive = serviceHandler(transaction, RPC_COMMAND_PROCESS_LOWERLEVEL_CONFIRM);
            goto case_RPC_DIRECTIVE_FROM_CONFIRM_RESPONSE_TIMEOUT;

        case RPC_EVENT_PARSE_RESPONSE_CANDIDATE:
            SYS_DbgAssert(FALSE == isUnsolResp,
                    HALT_rpcDispatcherExecClientTransaction_PARSE_RESPONSE_CANDIDATE_InvalidRequestType);
            SYS_DbgAssertComplex(transaction->isWaitingResponse,
                    HALT_rpcDispatcherExecClientTransaction_PARSE_RESPONSE_CANDIDATE_NoResponseTransaction);
            directive = serviceHandler(transaction, RPC_COMMAND_PARSE_RESPONSE_CANDIDATE);
            goto case_RPC_DIRECTIVE_FROM_CONFIRM_RESPONSE_TIMEOUT;

        case RPC_EVENT_PROCESS_TIMEOUT_EXPIRATION:
            SYS_DbgAssert(FALSE == isUnsolResp,
                    HALT_rpcDispatcherExecClientTransaction_PROCESS_TIMEOUT_EXPIRATION_InvalidRequestType);
            /* transaction->isWaitingResponse = FALSE;      was already performed by the timeout task handler. */
            SYS_DbgAssertComplex(!transaction->isWaitingResponse,
                    HALT_rpcDispatcherExecClientTransaction_PROCESS_TIMEOUT_EXPIRATION_WaitingTransaction);
            directive = serviceHandler(transaction, RPC_COMMAND_PROCESS_TIMEOUT_EXPIRATION);
            goto case_RPC_DIRECTIVE_FROM_CONFIRM_RESPONSE_TIMEOUT;

        case_RPC_DIRECTIVE_FROM_CONFIRM_RESPONSE_TIMEOUT:
            switch (directive)
            {
                case RPC_DIRECTIVE_ALIEN_RESPONSE:
                case RPC_DIRECTIVE_UNSOLICITED_RESPONSE:
                case RPC_DIRECTIVE_BROKEN_RESPONSE:
                    SYS_DbgAssert(FALSE == isUnsolResp,
                            HALT_rpcDispatcherExecClientTransaction_DIRECTIVE_BROKEN_RESPONSE_InvalidRequestType);
                    SYS_DbgAssert(RPC_EVENT_PARSE_RESPONSE_CANDIDATE == event,
                            HALT_rpcDispatcherExecClientTransaction_PARSE_RESPONSE_CANDIDATE_ProhibitedDirective);
                    /* Just return directive to the caller, it will process these cases outside the Transaction. */
                    break;

                case RPC_DIRECTIVE_PROCEED:
                    SYS_DbgAssert(FALSE == isUnsolResp,
                            HALT_rpcDispatcherExecClientTransaction_DIRECTIVE_PROCEED_InvalidRequestType);
                    SYS_DbgAssert(RPC_EVENT_PROCESS_TIMEOUT_EXPIRATION != event,
                            HALT_rpcDispatcherExecClientTransaction_PROCESS_TIMEOUT_EXPIRATION_ProhibitedDirective);
                    SYS_DbgAssertComplex(IMP(RPC_EVENT_PROCESS_LOWERLEVEL_CONFIRM == event,
                                             transaction->isWaitingResponse),
                            HALT_rpcDispatcherExecClientTransaction_PROCESS_LOWERLEVEL_CONFIRM_LostOfEvents);
                    goto case_RPC_DIRECTIVE_PROCEED;

                case RPC_DIRECTIVE_DONT_WAIT_RESPONSE:
                    if (transaction->isWaitingResponse)
                    {
#ifdef _DEBUG_COMPLEX_
                        SYS_QueueElement_t *removedElement =        /* Pointer to removed element; or NULL. */
#endif
                        SYS_QueueRemoveQueueElement(&dispatcher->waitingTransactionsQueue, &transaction->element);
                        SYS_DbgAssertComplex(NULL != removedElement,
                                HALT_rpcDispatcherExecClientTransaction_DONT_WAIT_RESPONSE_NotInWaitingQueue);
                        transaction->isWaitingResponse = FALSE;
                    }
                    else /* !(transaction->isWaitingResponse) */
                    {
                        SYS_DbgAssertComplex(NULL == SYS_QueueFindParentElement(&dispatcher->waitingTransactionsQueue,
                                                                                &transaction->element),
                                HALT_rpcDispatcherExecClientTransaction_DONT_WAIT_RESPONSE_StillInWaitingQueue);
                    }
                    goto case_RPC_DIRECTIVE_PROCEED;

                case_RPC_DIRECTIVE_PROCEED:
                    SYS_DbgAssertComplex(IMP(isUnsolResp,
                                             !transaction->isWaitingResponse && transaction->isDataConfirmed),
                            HALT_rpcDispatcherExecClientTransaction_DIRECTIVE_PROCEED_InvalidUnsolicitedFlags);
                    if (!transaction->isWaitingResponse && transaction->isDataConfirmed)
                        goto case_RPC_EVENT_KILL_CLIENT_TRANSACTION;
                    break;

                case RPC_DIRECTIVE_RESTART_TIMEOUT:
                    SYS_DbgAssert(FALSE == isUnsolResp,
                            HALT_rpcDispatcherExecClientTransaction_DIRECTIVE_RESTART_TIMEOUT_InvalidRequestType);
                    rpcDispatcherRestartResponseTimeout(dispatcher, transaction);
                    break;

                default:
                    SYS_DbgHalt(HALT_rpcDispatcherExecClientTransaction_CONFIRM_RESPONSE_TIMEOUT_InvalidDirective);
                    break;
            }
            break; /* case_RPC_DIRECTIVE_FROM_CONFIRM_RESPONSE_TIMEOUT: */

        case_RPC_EVENT_KILL_CLIENT_TRANSACTION:
            serviceHandler(transaction, RPC_COMMAND_KILL_TRANSACTION);
            rpcDispatcherKillTransaction(dispatcher, transaction);
            break;

        default:
            SYS_DbgHalt(HALT_rpcDispatcherExecClientTransaction_UnknownEvent);
            break;
    }

    return directive;
}


/*
 * Performs step-by-step execution of a Server Transaction with the specified event.
 */
RpcServiceDirective_t rpcDispatcherExecServerTransaction(
                RpcDispatcher_t     *const   dispatcher,
                RpcTransaction_t    *const   transaction,
                const RpcDispatcherEvent_t   event,
                const RpcServiceDirective_t  completedDirective)
{
    SYS_DbgAssert(NULL != dispatcher, HALT_rpcDispatcherExecServerTransaction_NullDispatcher);
    SYS_DbgAssert(NULL != transaction, HALT_rpcDispatcherExecServerTransaction_NullTransaction);
    SYS_DbgAssertComplex(RPC_TRANSACTION_TYPE_SERVER == transaction->transType,
            HALT_rpcDispatcherExecServerTransaction_InvalidTransactionType);
    SYS_DbgAssertComplex(dispatcher == transaction->parentDispatcher,
            HALT_rpcDispatcherExecServerTransaction_NotRelated);
    SYS_DbgAssertComplex(NULL != transaction->serviceDescr, HALT_rpcDispatcherExecServerTransaction_EmptyTransaction);

    const RpcServiceHandler_t *const serviceHandler =       /* Entry point to the linked Service handler function. */
            RPC_GET_SERVICE_HANDLER(transaction->serviceDescr);

    SYS_DbgAssertComplex(NULL != serviceHandler, HALT_rpcDispatcherExecServerTransaction_NullServiceHandler);

    RpcServiceDirective_t  directive =                      /* Directive returned by the Service. */
            RPC_DIRECTIVE_PROHIBITED_CODE;

    switch (event)
    {
        case RPC_EVENT_PARSE_CLIENT_REQUEST:
            directive = serviceHandler(transaction, RPC_COMMAND_PARSE_CLIENT_REQUEST);
            goto case_RPC_DIRECTIVE_FROM_PARSE_CLIENT_REQUEST;

        case_RPC_DIRECTIVE_FROM_PARSE_CLIENT_REQUEST:
        case_RPC_DIRECTIVE_FROM_PROCESS_LOWERLEVEL_CONFIRM:
            if (RPC_DIRECTIVE_FINISH_TRANSACTION == directive)
                goto case_RPC_EVENT_KILL_SERVER_TRANSACTION;
            else if (RPC_DIRECTIVE_WAIT_SERVICE == directive)
            {
                SYS_DbgAssert(RPC_EVENT_PROCESS_LOWERLEVEL_CONFIRM == event,
                        HALT_rpcDispatcherExecServerTransaction_UNEXPECTED_WAIT_SERVICE);
                /* Just return and wait for COMPLETED signal from the Service. */
                break;
            }
            else if (RPC_DIRECTIVE_MAKE_RESPONSE_NOW != directive)
            {
                SYS_DbgAssert(RPC_DIRECTIVE_PROCEED == directive,
                        HALT_rpcDispatcherExecServerTransaction_MAKE_RESPONSE_InvalidDirective);
                directive = serviceHandler(transaction, RPC_COMMAND_COMPOSE_SERVER_RESPONSE);
            }
            else /* if (RPC_DIRECTIVE_MAKE_RESPONSE_NOW == directive) */
            {
                SYS_DbgAssert(RPC_EVENT_PARSE_CLIENT_REQUEST == event,
                        HALT_rpcDispatcherExecServerTransaction_UNEXPECTED_MAKE_RESPONSE_NOW);
            }
            goto case_RPC_DIRECTIVE_FROM_COMPOSE_SERVER_RESPONSE;

        case RPC_EVENT_SERVICE_COMPLETED:
            directive = completedDirective;
            goto case_RPC_DIRECTIVE_FROM_COMPOSE_SERVER_RESPONSE;

        case_RPC_DIRECTIVE_FROM_COMPOSE_SERVER_RESPONSE:
            switch (directive)
            {
                case RPC_DIRECTIVE_WAIT_SERVICE:
                    /* Just return and wait for COMPLETED signal from the Service. */
                    break;

                case RPC_DIRECTIVE_DONT_MAKE_RESPONSE:
                    goto case_RPC_EVENT_KILL_SERVER_TRANSACTION;

                case RPC_DIRECTIVE_MAKE_RESPONSE_NOW:
                case RPC_DIRECTIVE_MAKE_RESPONSE:
                    dispatcher->customIssueDataReq(transaction);
                    break;

                default:
                    SYS_DbgHalt(HALT_rpcDispatcherExecServerTransaction_COMPOSE_SERVER_RESPONSE_InvalidDirective);
                    break;
            }
            break; /* case_RPC_DIRECTIVE_FROM_COMPOSE_SERVER_RESPONSE: */

        case RPC_EVENT_PROCESS_LOWERLEVEL_CONFIRM:
            directive = serviceHandler(transaction, RPC_COMMAND_PROCESS_LOWERLEVEL_CONFIRM);
            goto case_RPC_DIRECTIVE_FROM_PROCESS_LOWERLEVEL_CONFIRM;

        case RPC_EVENT_PARSE_UNSOLICITED_RESPONSE:
            directive = serviceHandler(transaction, RPC_COMMAND_PARSE_UNSOLICITED_RESPONSE);
            goto case_RPC_DIRECTIVE_FROM_PARSE_UNSOLICITED_RESPONSE;

        case_RPC_DIRECTIVE_FROM_PARSE_UNSOLICITED_RESPONSE:
            SYS_DbgAssert(RPC_DIRECTIVE_FINISH_TRANSACTION == directive,
                    HALT_rpcDispatcherExecServerTransaction_DIRECTIVE_FROM_PARSE_UNSOLICITED_RESPONSE_InvalidDirective);
            goto case_RPC_EVENT_KILL_SERVER_TRANSACTION;

        case_RPC_EVENT_KILL_SERVER_TRANSACTION:
            serviceHandler(transaction, RPC_COMMAND_KILL_TRANSACTION);
            rpcDispatcherKillTransaction(dispatcher, transaction);
            break;

        default:
            SYS_DbgHalt(HALT_rpcDispatcherExecServerTransaction_UnknownEvent);
            break;
    }

    return directive;
}


/*
 * Kills and dismisses an RPC Transaction.
 */
void rpcDispatcherKillTransaction(
                RpcDispatcher_t  *const  dispatcher,
                RpcTransaction_t *const  transaction)
{
    SYS_DbgAssert(NULL != dispatcher, HALT_rpcDispatcherKillTransaction_NullDispatcher);
    SYS_DbgAssert(NULL != transaction, HALT_rpcDispatcherKillTransaction_NullTransaction);
    SYS_DbgAssertComplex(dispatcher == transaction->parentDispatcher, HALT_rpcDispatcherKillTransaction_NotRelated);

    /* Reinitialize the application-specific part. */

    const RpcCustomKillTrans_t *customKillTrans =       /* Entry point to the application-specific kill-transaction */
            dispatcher->customKillTrans;                /* function. */

    if (NULL != customKillTrans)
        customKillTrans(transaction);

    /* Reinitialize common part. */

#ifdef _DEBUG_COMPLEX_
    transaction->serviceDescr = NULL;
    transaction->localRequest = NULL;

    transaction->remoteRequest.plain /* = [union with] transaction->remoteResponse.plain */ = 0;
    SYS_DbgAssertStatic(sizeof(transaction->remoteRequest) == sizeof(transaction->remoteResponse));
#endif

    /* Dismiss RPC Transaction. */

    rpcTransactionFree(dispatcher, transaction);
}


/* eof bbRpcDispatcher.c */
