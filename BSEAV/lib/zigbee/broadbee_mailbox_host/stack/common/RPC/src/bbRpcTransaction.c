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
*   RPC Transaction manipulation implementation.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


/************************* INCLUDES *****************************************************/
#include "private/bbRpcTransactionPrivate.h"
#include "private/bbRpcDispatcherPrivate.h"


/************************* IMPLEMENTATION ***********************************************/
/*
 * Allocates new RPC Transaction.
 */
RpcTransaction_t* rpcTransactionAlloc(
                RpcDispatcher_t *const  dispatcher,
                const RpcTransType_t    transType,
                const bool              forceTsn,
                const RpcTransSeqNum_t  forcedTsn)
{
    SYS_DbgAssert(NULL != dispatcher, HALT_rpcTransactionAlloc_NullDispatcher);

    RpcDualQuota_t *const  transQuota =                         /* Pointer to the Transactions Quota descriptor. */
            &dispatcher->transQuota;

    if (FALSE == rpcDualQuotaTry(transQuota, transType))
        return NULL;

    SYS_QueueDescriptor_t *const  unusedTransactionsQueue =     /* Pointer to the Unused Transactions Queue. */
            &dispatcher->unusedTransactionsQueue;

    SYS_QueueElement_t *const  allocatedTransactionElement =    /* Pointer to the service field of the allocated */
            SYS_QueueGetQueueHead(unusedTransactionsQueue);     /* Transaction; or NULL if it was not allocated. */

    if (NULL == allocatedTransactionElement)
    {
        rpcDualQuotaFree(transQuota, transType);
        return NULL;
    }

    SYS_QueueRemoveHeadElement(unusedTransactionsQueue);

    RpcTransaction_t *const allocatedTransaction =              /* Pointer to the allocated Transaction. */
            GET_PARENT_BY_FIELD(RpcTransaction_t, element, allocatedTransactionElement);

    SYS_DbgAssertComplex(NULL == allocatedTransaction->parentDispatcher,
            HALT_rpcTransactionAlloc_NonEmptyParentDispatcher);

    allocatedTransaction->parentDispatcher = dispatcher;
    allocatedTransaction->transType = transType;

    SYS_DbgAssertComplex(IMP(RPC_TRANSACTION_TYPE_CLIENT != transType, FALSE != forceTsn),
            HALT_rpcTransactionAlloc_ServerTransactionMustHaveForcedTsn);

    allocatedTransaction->transSeqNum =
            ((RPC_TRANSACTION_TYPE_CLIENT == transType && FALSE == forceTsn) ?
                    (dispatcher->transSeqNumNext++) : forcedTsn) & (dispatcher->transSeqNumMask);

    return allocatedTransaction;
}


/*
 * Frees RPC Transaction.
 */
void rpcTransactionFree(
                RpcDispatcher_t  *const  dispatcher,
                RpcTransaction_t *const  transaction)
{
    SYS_DbgAssert(NULL != dispatcher, HALT_rpcTransactionFree_NullDispatcher);
    SYS_DbgAssert(NULL != transaction, HALT_rpcTransactionFree_NullTransaction);
    SYS_DbgAssertComplex(dispatcher == transaction->parentDispatcher, HALT_rpcTransactionFree_NotRelated);

#ifdef _DEBUG_COMPLEX_
    transaction->parentDispatcher = NULL;
#endif

    rpcDualQuotaFree(&dispatcher->transQuota, transaction->transType);
    SYS_QueuePutQueueElementToHead(&dispatcher->unusedTransactionsQueue, &transaction->element);
    rpcDispatcherFreeTransactionCallback(dispatcher);
}


/* eof bbRpcTransaction.c */
