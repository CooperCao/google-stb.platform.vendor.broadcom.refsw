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
*   RPC Dispatcher interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_RPC_DISPATCHER_H
#define _BB_RPC_DISPATCHER_H


/************************* INCLUDES *****************************************************/
#include "bbRpcCustom.h"
#include "bbRpcService.h"
#include "bbRpcTransaction.h"
#include "bbRpcDualQuota.h"
#include "bbSysTimeoutTask.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for descriptor of RPC Dispatcher.
 */
struct _RpcDispatcher_t
{
    /* 32-bit data. */

    const RpcService_t               *servicesDescrTable;           /*!< Pointer to the beginning of the null-
                                                                        terminating services' descriptors table. */

    SYS_QueueDescriptor_t             newLocalRequestsQueue;        /*!< Queue of New Local Requests. */

    SYS_QueueDescriptor_t             unusedTransactionsQueue;      /*!< Queue of Unused Transactions. */

    SYS_QueueDescriptor_t             retryTransactionsQueue;       /*!< Queue of Client Transactions to be Retried. */

    SYS_QueueDescriptor_t             waitingTransactionsQueue;     /*!< Queue of Client Transaction Waiting for
                                                                        Response. */

    RpcCustomLinkClientTrans_t       *customLinkClientTrans;        /*!< Entry point to the application-specific link-
                                                                        client-transaction function. Called once. */

    RpcCustomLinkServerTrans_t       *customLinkServerTrans;        /*!< Entry point to the application-specific link-
                                                                        server-transaction function. Called once. */

    RpcCustomIssueDataReq_t          *customIssueDataReq;           /*!< Entry point to the application-specific issue-
                                                                        data-request function. Called once. */

    RpcCustomKillTrans_t             *customKillTrans;              /*!< Entry point to the application-specific kill-
                                                                        transaction function. Called once. */

    RpcTransSeqNum_t                  transSeqNumNext;              /*!< Stored Sequence Number for the next Client
                                                                        Transaction. */

    RpcTransSeqNumMask_t              transSeqNumMask;              /*!< Binary Mask for Transaction Sequence Number
                                                                        comparison. */

    /* Structured data, aligned at 32-bit. */

    SYS_SchedulerTaskDescriptor_t     dispatcherTask;               /*!< This Dispatcher Task descriptor. */

    SYS_TimeoutTask_t                 timeoutTimer;                 /*!< Response Waiting Timeout Timer descriptor. */

    /* Structured data, aligned at 16-bit. */

    RpcDualQuota_t                    transQuota;                   /*!< Descriptor of Transactions Quota. */

#ifdef _DEBUG_COMPLEX_

    /* 8-bit data. */

    uint8_t                           isInitialized;                /*!< 0xDA if initialized; other value if not
                                                                        initialized. */
#endif
};


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Initializes RPC Dispatcher on startup.
 * \param[out]  dispatcher              Pointer to the initialized Dispatcher descriptor.
 * \param[in]   schedulerPriority       Dispatcher task priority when extracted for
 *  execution.
 * \param[in]   servicesDescrTable      Pointer to the first element of null-terminated
 *  table of services' descriptors.
 * \param[in]   customLinkClientTrans   Entry point to the application-specific link-
 *  client-transaction function.
 * \param[in]   customLinkServerTrans   Entry point to the application-specific link-
 *  server-transaction function.
 * \param[in]   customIssueDataRequest  Entry point to the application-specific issue-
 *  data-request function.
 * \param[in]   customKillTrans         Entry point to the application-specific kill-
 *  transaction function.
 * \param[in]   transactionsPullHead    Pointer to the first element of Transactions Pull.
 * \param[in]   transactionsPullSize    Size of Transactions Pull, in elements.
 * \param[in]   transactionDescrSize    Size of a single application-specific Transaction
 *  Descriptor in the Pull, in bytes.
 * \param[in]   transQuotaLimits        Array of two Limit values for Dual Quota over the
 *  Transactions Pull between CLIENT and SERVER types of Transactions.
 * \param[in]   transSeqNumMask         Binary Mask for Transaction Sequence Number
 *  comparison.
 * \note
 *  Do not ever call this function during the RPC Dispatcher work even if it is itself and
 *  all its linked Services are temporarily idle; use this function only during the
 *  startup sequence for memory initialization.
 * \note
 *  The last object in the table pointed with \p servicesDescrTable shall have \c handler
 *  field equal to NULL; it will serve as the end-of-table marker.
 * \note
 *  All Transactions in the Pull pointed with \p transactionsPullHead will be linked into
 *  a single queue with their \c element.next field during this function work.
 * \note
 *  Transaction Sequence Number Mask must be of the form (2^m-1), where m=8...32.
 */
void rpcDispatcherInit(
                RpcDispatcher_t                  *const  dispatcher,
                const SYS_SchedulerPriority_t            schedulerPriority,
                const RpcService_t               *const  servicesDescrTable,
                      RpcCustomLinkClientTrans_t *const  customLinkClientTrans,
                      RpcCustomLinkServerTrans_t *const  customLinkServerTrans,
                      RpcCustomIssueDataReq_t    *const  customIssueDataReq,
                      RpcCustomKillTrans_t       *const  customKillTrans,
                      RpcTransaction_t           *const  transactionsPullHead,
                const size_t                             transactionsPullSize,
                const size_t                             transactionDescrSize,
                const RpcDualQuotaLimit_t                transQuotaLimits[RPC_DUAL_QUOTA_CHANNELS_NUMBER],
                const RpcTransSeqNumMask_t               transSeqNumMask);


/**//**
 * \brief   Accepts New Local Client Request for processing with the specified RPC
 *  Dispatcher.
 * \param[in]   dispatcher      Pointer to the dedicated Dispatcher descriptor.
 * \param[in]   localRequest    Pointer to Local Client Request service field.
 */
void rpcDispatcherAcceptNewLocalRequest(
                RpcDispatcher_t   *const  dispatcher,
                RpcLocalRequest_t *const  localRequest);


/**//**
 * \brief   Accepts COMPLETED signal from the Service being waited for completion.
 * \param[in]   dispatcher              Pointer to the dedicated Dispatcher descriptor.
 * \param[in]   transaction             Pointer to the service field of executed
 *  Transaction.
 * \param[in]   completedDirective      Numeric identifier of the Directive reported by
 *  the Service being completed.
 * \details
 *  Pointer to the dispatcher Descriptor is considered to be well known to the Service
 *  having application-specific implementation. If the same Service is shared between two
 *  ore more applications (i.e., Dispatchers), implement dedicated wrappers of such single
 *  Service one per each Dispatcher and provide each one with the pointer to descriptor of
 *  its personal Dispatcher.
 */
void rpcDispatcherAcceptServiceCompleted(
                RpcDispatcher_t      *const  dispatcher,
                RpcTransaction_t     *const  transaction,
                const RpcServiceDirective_t  completedDirective);


/**//**
 * \brief   Accepts confirmation on request to transmit data from the lower-level layer
 *  conducted by the application specific callback function.
 * \param[in]   dispatcher      Pointer to the dedicated Dispatcher descriptor.
 * \param[in]   transaction     Pointer to the service field of executed Transaction.
 * \details
 *  Originally request to transmit data via the lower-level layer was issued by the
 *  application-specific function specified to the RPC Dispatcher with
 *  \c customIssueDataReq during initialization. That function should assign appropriate
 *  callback handler for further confirmation to be called by the lower-level layer.
 * \details
 *  The pointer to the Transaction service field must be recovered from the pointer to the
 *  confirmed application-specific lower-level layer request descriptor that is embedded
 *  into the same Transaction Descriptor with the Transaction service field. Pointer to
 *  the dispatcher Descriptor is considered to be well known to the calling application.
 * \details
 *  The calling application-specific RPC Dispatcher Wrapper shall assign the application-
 *  specific part of the Transaction Descriptor pointed here with \p transaction with the
 *  received data-confirmation parameters prior to call this function in order to provide
 *  the application-specific Service with these confirmation parameters.
 */
void rpcDispatcherAcceptDataConf(
                RpcDispatcher_t  *const  dispatcher,
                RpcTransaction_t *const  transaction);


/**//**
 * \brief   Accepts received data indication from the lower-level layer conducted by the
 *  application specific handling function.
 * \param[in]   dispatcher      Pointer to the dedicated Dispatcher descriptor.
 * \param[in]   remoteRequest   Union object assigned either with the pointer to the data-
 *  indication parameters structure, or with the data-indication payload descriptor of the
 *  received Remote Request, or application-specific payload descriptor. This data is
 *  provided by the calling application which also chooses what to save in the Transaction
 *  for purposes of Services.
 * \param[in]   isResponse      TRUE if the frame described with \p remoteRequest is a
 *  Server Response indeed but not a Client Request; FALSE if it is a Client Request.
 * \param[in]   serviceId       Unique numeric identifier of the corresponding Service.
 * \param[in]   transSeqNum     Transaction Sequence Number reported by the lower-level
 *  layer.
 * \return  TRUE if indicated data was processed; FALSE if it was rejected.
 * \note
 *  In the case when this function returns FALSE (i.e., indicated data is rejected) the
 *  caller shall dismiss the dynamic memory allocated for the indication payload. If TRUE
 *  is returned, it means that the Service finally called by this RPC Dispatcher is
 *  responsible for that.
 * \note
 *  When two Transaction Sequence Numbers compared, only those bits are taken into account
 *  that equal to one in the Mask saved in the \c transSeqNumMask field of \p dispatcher.
 */
bool rpcDispatcherAcceptDataInd(
                RpcDispatcher_t   *const  dispatcher,
                const RpcRemoteRequest_t  remoteRequest,
                const bool                mayBeRequest,
                const bool                mayBeResponse,
                const RpcServiceId_t      serviceId,
                const RpcTransSeqNum_t    transSeqNum);


#endif /* _BB_RPC_DISPATCHER_H */
