/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

/*******************************************************************************
 *
 * DESCRIPTION:
 *      RPC Transaction manipulation private interface.
 *
*******************************************************************************/

#ifndef _BB_RPC_TRANSACTION_PRIVATE_H
#define _BB_RPC_TRANSACTION_PRIVATE_H


/************************* INCLUDES *****************************************************/
#include "bbRpcTransaction.h"


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Allocates new RPC Transaction.
 * \param[in]   dispatcher      Pointer to the parent Dispatcher descriptor.
 * \param[in]   transType       Either CLIENT or SERVER.
 * \param[in]   forceTsn        TRUE to force TSN for CLIENT transaction to te value given
 *  with \p forcedTsn.
 * \param[in]   forcedTsn       Sequence Number to be assigned to the newly allocated
 *  Transaction of the SERVER type, or CLIENT type when \p forceTsn is TRUE.
 * \return  Pointer to the service field of the allocated Transaction; on NULL if
 *  Transaction was not allocated.
 * \details
 *  Prior to allocate a new Transaction this function validates if the quota is not
 *  exceeded for the desired Transaction Type. If so, new Transaction is not allocated
 *  even if there are unused Transactions (in this case they are hold for different
 *  Transaction Types).
 */
RpcTransaction_t* rpcTransactionAlloc(
                RpcDispatcher_t *const  dispatcher,
                const RpcTransType_t    transType,
                const bool              forceTsn,
                const RpcTransSeqNum_t  forcedTsn);


/**//**
 * \brief   Frees RPC Transaction.
 * \param[in]   dispatcher      Pointer to the parent Dispatcher descriptor.
 * \param[in]   transaction     Pointer to the freed Transaction.
 */
void rpcTransactionFree(
                RpcDispatcher_t  *const  dispatcher,
                RpcTransaction_t *const  transaction);


#endif /* _BB_RPC_TRANSACTION_PRIVATE_H */

/* eof bbRpcTransactionPrivate.h */