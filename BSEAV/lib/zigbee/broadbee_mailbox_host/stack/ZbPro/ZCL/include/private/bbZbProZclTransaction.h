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
*   ZCL Transaction common definitions.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZCL_TRANSACTION_H
#define _BB_ZBPRO_ZCL_TRANSACTION_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZclCommon.h"
#include "bbZbProApsData.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for descriptor of Outgoing ZCL Command.
 */
typedef struct _ZbProZclOutgoingCommandDescr_t  ZbProZclOutgoingCommandDescr_t;


/**//**
 * \brief   Structure for descriptor of Incoming ZCL Command.
 */
typedef struct _ZbProZclIncomingCommandDescr_t  ZbProZclIncomingCommandDescr_t;


/**//**
 * \brief   Structure for ZCL Transaction descriptor.
 */
typedef struct _ZbProZclTransaction_t
{
    /* Structured data with 32-bit alignment. */

    RpcTransaction_t                  rpcTransactionDescr;
            /*!< Embedded descriptor of the RPC Transaction. */

#if !defined(_DEBUG_COMPLEX_)
    union
    {       /* Provide separated fields for the debug build. */
#endif
        const ZbProZclOutgoingCommandDescr_t *outgoingCommandDescr;
            /*!< Pointer to the linked Outgoing Command descriptor. */

        const ZbProZclIncomingCommandDescr_t *incomingCommandDescr;
            /*!< Pointer to the linked Incoming Command descriptor. */

#if !defined(_DEBUG_COMPLEX_)
    };
#endif

    ZBPRO_APS_DataReqDescr_t          apsDataReqDescr;
            /*!< Embedded descriptor of the APSDE-DATA.request. ZCL Dispatcher performs final freeing of payload on
                transaction killing. If inside single Server Transaction multiple data frames (Server Responses) are
                sent, the corresponding Service Handler shall decide if to free the previous payload and allocate the
                next one for each data frame sent, or to reuse the same payload for all of them. */

    /* 32-bit data. */

    const ZBPRO_APS_DataConfParams_t *apsDataConfParams;
            /*!< Pointer to the APSDE-DATA.confirm parameters. Service doesn't have to reset this field to NULL after
                processing of each confirmation in the case of multiple Server Responses sent by it within single Server
                Transaction. */

    /* 16-bit data. */

    ZBPRO_ZCL_ClusterId_t             clusterId;
            /*!< Identifier of Cluster this transaction is opened for. Notice that all frames (commands) issued within a
                transaction belong to the same cluster, domain and manufacturer as the original frame does. */

    ZBPRO_ZCL_ManufCode_t             manufCode;
            /*!< Manufacturer Code this transaction is opened for. Assigned only if \c frameDomain specifies
                Manufacturer Specific transaction. Notice that all frames (commands) issued within a transaction belong
                to the same cluster, domain and manufacturer as the original frame does.*/

    /* 8-bit data. */

    ZBPRO_ZCL_FrameDomain_t           frameDomain       : 1;
            /*!< Frame domain of the transaction: either ZCL Standard (0x0) or Manufacturer Specific (0x1). Notice that
                all frames (commands) issued within a transaction belong to the same cluster, domain and manufacturer as
                the original frame does. */

    ZBPRO_ZCL_ClusterSide_t           originatorSide    : 1;
            /*!< Side of cluster that originated the distributed transaction: either ZCL Client (0x0) or ZCL Server
                (0x1). Notice that distributed transaction may be opened by ZLC Server as well as by ZCL Client; and the
                term ZCL Client is not necessarily bound to RPC Client. All outgoing frames of RPC Client Transaction
                and all incoming frames to RPC Server Transaction have the \c direction field equal to this parameter,
                and inversely, all incoming frames to RPC Client Transaction and all outgoing frames of RPC Server
                Transaction must have \c direction field opposite to this parameter. */

    Bool8_t                           isZclConfIssued   : 1;
            /*!< TRUE if Local ZCL Confirmation was already issued; FALSE if it wasn't yet. This field shall be set to
                TRUE by the linked Service Handler as soon as Local ZCL Confirmation is issued by it. Next time on
                attempt to issue Local ZCL Confirmation it will be canceled because this flag is equal to TRUE.
                Validation and assignment of this flag shall be implemented directly inside the function responsible for
                issuing the Local ZCL Confirmation. */

    Bool8_t                           unicastApsTx      : 1;
            /*!< TRUE if request command within this RPC Client Transaction was sent by APS unicastly; FALSE otherwise
                (i.e., nonunicastly) or APS transmission is not confirmed yet. This flag is used only for RPC Client
                Transaction for resolving whether the request transmission was unicast or nonunicast in the case of
                response waiting timeout expiration. If request transmission was unicast and default response for
                successful processing was not disabled, but there were no response in the specified timeout, the TIMEOUT
                status is confirmed to the local higher-level layer. If APS transmission is not completed yet (this flag
                stays FALSE in this case), but response waiting timeout has already expired, then ZCL Dispatcher checks
                if request transmission was unicast or not just according to the request destination address and
                addressing mode. Notice that indirect addressing (i.e., transmission via the APS binding table) is
                considered nonunicast. */

} ZbProZclTransaction_t;


#endif /* _BB_ZBPRO_ZCL_TRANSACTION_H */
