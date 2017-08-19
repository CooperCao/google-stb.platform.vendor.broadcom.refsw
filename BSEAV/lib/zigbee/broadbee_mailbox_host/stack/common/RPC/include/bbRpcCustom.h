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
 *      RPC application-specific customization definitions.
 *
*******************************************************************************/

#ifndef _BB_RPC_CUSTOM_H
#define _BB_RPC_CUSTOM_H


/************************* INCLUDES *****************************************************/
#include "bbRpcPredefined.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Data type for application-specific function performing custom RPC Client
 *  Transaction Descriptor linking to Local Client Request.
 * \param[in]   transaction     Pointer to the service field of linked Client Transaction.
 * \note
 *  Pointer to application-specific service field of started Local Request is assigned to
 *  the \c localRequest field of \p transaction by RPC Dispatcher prior to call this
 *  function.
 * \details
 *  This application-provided function is called only once per each new Client Transaction
 *  after its base linking to the Local Client Request service field.
 */
typedef void RpcCustomLinkClientTrans_t(
                const RpcTransaction_t *const  transaction);


/**//**
 * \brief   Data type for application-specific function performing custom RPC Server
 *  Transaction Descriptor linking to Remote Client Request.
 * \param[in]   transaction     Pointer to the service field of linked Server Transaction.
 * \note
 *  Pointer to application-specific Parameters structure or Payload Descriptor of received
 *  Remote Request is assigned to the \c remoteRequest field of \p transaction by RPC
 *  Dispatcher prior to call this function.
 * \details
 *  This application-provided function is called only once per each new Transaction after
 *  its base linking to the Remote Client Request service field.
 */
typedef void RpcCustomLinkServerTrans_t(
                const RpcTransaction_t *const  transaction);


/**//**
 * \brief   Data type for application-specific function performing custom issue-data-
 *  request to the lower-level layer for the specified RPC Transaction.
 * \param[in]   transaction     Pointer to the service field of executed Transaction.
 * \details
 *  This application-provided function is used both by Client and Server types of RPC
 *  Transactions.
 * \details
 *  This application-provided function is called only once per each Client Transaction
 *  after its Client Request is successfully composed; and it may be called one or
 *  multiple times per each Server Transaction according to the number of Responses that
 *  must be sent on received Request within such a Transaction. This function shall
 *  arrange application-specific callback confirmation that will be triggered by the
 *  lower-level layer on data-transmission-confirmation. In turn it shall issue the
 *  CONFIRMATION event to the RPC Dispatcher for the corresponding Transaction.
 */
typedef void RpcCustomIssueDataReq_t(
                const RpcTransaction_t *const  transaction);


/**//**
 * \brief   Data type for application-specific function performing custom RPC Transaction
 *  Descriptor killing.
 * \param[in]   transaction     Pointer to the service field of killed Transaction.
 * \details
 *  This application-provided function is used both by Client and Server types of RPC
 *  Transactions.
 * \details
 *  This application-provided function is called only once per each Transaction being
 *  killed just prior to reinitialize its common RPC part and dismiss it.
 * \note
 *  Take into account that killing of common part of an RPC Transaction is performed by
 *  dedicated RPC function that may not be substituted by application.
 */
typedef void RpcCustomKillTrans_t(
                RpcTransaction_t *const  transaction);


#endif /* _BB_RPC_CUSTOM_H */

/* eof bbRpcCustom.h */