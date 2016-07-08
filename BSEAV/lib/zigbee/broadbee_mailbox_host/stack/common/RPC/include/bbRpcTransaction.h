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
*   RPC Transaction common definitions.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_RPC_TRANSACTION_H
#define _BB_RPC_TRANSACTION_H


/************************* INCLUDES *****************************************************/
#include "bbRpcRequest.h"
#include "bbSysTime.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Data type for binary Mask of Transaction Sequence Number.
 * \details
 *  When two Transaction Sequence Numbers compared only those bits are taken into account
 *  that equal to one in the Mask.
 */
typedef uint32_t  RpcTransSeqNumMask_t;


/**//**
 * \brief   Enumeration of Transaction Types.
 */
typedef enum _RpcTransType_t
{
    RPC_TRANSACTION_TYPE_CLIENT = 0,        /*!< CLIENT type Transaction. Used for Local Requests processing and waiting
                                                for Remote Responses. */

    RPC_TRANSACTION_TYPE_SERVER = 1,        /*!< SERVER type Transaction. Used for Remote Requests processing and
                                                issuing Server Responses on them. */
} RpcTransType_t;


/**//**
 * \brief   Structure for the service field of RPC Transaction object.
 * \details
 *  This structure shall be embedded as a service field into each application specific RPC
 *  Transaction Descriptor structure.
 */
struct _RpcTransaction_t
{
    /* 32-bit data. */

    SYS_QueueElement_t       element;                   /*!< Queue element. */

    RpcDispatcher_t         *parentDispatcher;          /*!< Pointer to the parent Dispatcher descriptor. */

    const RpcService_t      *serviceDescr;              /*!< Pointer to the linked Service descriptor. */

    const RpcLocalRequest_t *localRequest;              /*!< Pointer to the service field of linked Local Request. */

    union
    {
        RpcRemoteRequest_t   remoteRequest;             /*!< Union object assigned either with the pointer to the data-
                                                            indication parameters structure, or with the data-indication
                                                            payload descriptor of the received Remote Request. This data
                                                            is provided by the calling application which also chooses
                                                            what to save in the Transaction for purposes of Services.
                                                            This field may also be used by the linked Service for
                                                            storing (or referencing) arbitrary data on its own
                                                            discretion between periods of its activity. This field is
                                                            used only for Server Transactions. Service doesn't have to
                                                            reset this field to NULL after this Transaction completion;
                                                            it will be reset by the RPC Dispatcher when killing the
                                                            Transaction. */

        RpcRemoteResponse_t  remoteResponse;            /*!< The same with the \c remoteRequest but for the case of a
                                                            Client Transaction activated for processing of a Response
                                                            Candidate. The linked Service must process the Response
                                                            Candidate frame in single execution flow; and it must not
                                                            keep any private data in this field between periods of its
                                                            activity, because this field may be assigned with a new
                                                            Response Candidate at arbitrary moment (if reception of
                                                            responses is enabled). Service doesn't have to reset this
                                                            field to NULL after processing of each Response Candidate;
                                                            it will be reset by the RPC Dispatcher when killing the
                                                            Transaction. */
    };

    SYS_Timediff_t           timeoutPeriod;             /*!< Value of response waiting timeout period, in ms. This value
                                                            may be reassigned by the Service Handler on its discretion;
                                                            it will be accepted by the RPC Dispatcher next time when it
                                                            starts the Response Waiting Timeout Timer - i.e., the first
                                                            time just after issuing the Local Client Request, and each
                                                            time when it is requested to restart the timeout timer. */

    SYS_Time_t               expirationTimestamp;       /*!< Timestamp of the response waiting period end, in ms. */

    RpcTransSeqNum_t         transSeqNum;               /*!< Transaction Sequence Number. */

    /* 8-bit data. */

    union
    {
        BitField8_t          flags;                     /*!< Plain value of Transaction flags. Set to ZERO during
                                                            initialization. */
        struct
        {
            Bool8_t          isDataConfirmed   : 1;     /*!< TRUE if request to transmit data via the lower-level layer
                                                            is confirmed. Service must not reassign this field. */

            Bool8_t          isWaitingResponse : 1;     /*!< TRUE if shall wait Server Response. Set to FALSE during
                                                            initialization. Service must not reassign this field. */
        };
    };

    RpcTransType_t           transType;                 /*!< CLIENT or SERVER type. */
};


#endif /* _BB_RPC_TRANSACTION_H */
