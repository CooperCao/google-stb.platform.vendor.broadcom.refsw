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
 *      MAC-SAP service data types.
 *
*******************************************************************************/

#ifndef _BB_MAC_SAP_SERVICE_H
#define _BB_MAC_SAP_SERVICE_H


/************************* INCLUDES *****************************************************/
#include "bbMacBasics.h"            /* Basic MAC set.           */
#include "bbSysQueue.h"             /* Queues engine interface. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief MAC-SAP supported request- and response-primitives identifiers enumeration.
 * \note Response-primitives are processed in the same way as request-primitives. For this
 *  reason their identifiers shall not overlay between responses and requests groups.
 */
typedef enum _MacReqPrimitiveId_t
{
    MAC_REQ_FREE = 0,           /*!< Special identifier to mark the internal MAC Request/Response Descriptor as free. */

    MAC_REQ_DATA,               /*!< Primitive MCPS-DATA.request. */
    MAC_REQ_PURGE,              /*!< Primitive MCPS-PURGE.request. */
    MAC_REQ_GET,                /*!< Primitive MLME-GET.request. */
    MAC_REQ_RESET,              /*!< Primitive MLME-RESET.request. */
    MAC_REQ_RX_ENABLE,          /*!< Primitive MLME-RX-ENABLE.request. */
    MAC_REQ_SET,                /*!< Primitive MLME-SET.request. */
    MAC_REQ_START,              /*!< Primitive MLME-START.request. */
    MAC_RESP_BEACON,            /*!< Primitive MLME-BEACON.response for internal usage in the MAC. */
    MAC_REQ_SCAN,               /*!< Primitive MLME-SCAN.request. */
    MAC_REQ_ASSOCIATE,          /*!< Primitive MLME-ASSOCIATE.request. */
    MAC_RESP_ASSOCIATE,         /*!< Primitive MLME-ASSOCIATE.response. */
    MAC_RESP_ORPHAN,            /*!< Primitive MLME-ORPHAN.response. */
    MAC_RESP_BEACON_TESTER,     /*!< Primitive MLME-BEACON.response for external usage by the higher layer. */
    MAC_REQ_POLL,               /*!< Primitive MLME-POLL.request. */
} MacReqPrimitiveId_t;


/**//**
 * \brief MAC-FE transactions handling, new-or-pending-request-to-process enumeration.
 * \details New indirect requests (i.e., if Indirect TX option of request parameters is
 *  set) are put into the transactions queue tail and their life timestamp is set
 *  according to the \e macTransactionPersistenceTime.
 * \details The pending requests are processed in a special way: (1) if the expired flag
 *  is also set in the request service field, then the request is not processed but the
 *  \c TRANSACTION_EXPIRED confirmation is issued, otherwise the request is processed as a
 *  pending request; (2) the Indirect TX option is ignored, i.e. a pending request with
 *  this option set, when assigned for processing, will be processed but not put into the
 *  transactions queue as for the new requests; (3) the number of retransmissions is set
 *  to zero instead of using the \e macMaxFrameRetries value, i.e. the pending requests
 *  are not retransmitted in the case of failed transmission attempt; (4) if a pending
 *  request is not successfully transmitted it is returned into the transactions queue,
 *  but the transactions dispatcher will not calculate the transaction life timestamp
 *  according to the \c macTransactionsPersistentTime value, it will preserve previously
 *  calculated life timestamp for this transaction; (5) and then the DSN value, with which
 *  the pending request was attempted to be transmitted firstly, will be stored for all
 *  consecutive retransmission attempts; (6) and then the destination address hash will
 *  not be recalculated, because it was calculated previously when the request was put
 *  into the transactions queue for the first time.
 */
typedef enum _MacPendingRequestFlag_t
{
    MAC_FE_TRANS_NEW_REQUEST      = 0,  /*!< New request to process. */
    MAC_FE_TRANS_NEW_RESPONSE     = 0,  /*!< New response to process. */
    MAC_FE_TRANS_PENDING_REQUEST  = 1,  /*!< Pending request to process. */
    MAC_FE_TRANS_PENDING_RESPONSE = 1,  /*!< Pending response to process. */
} MacTransPendingReqFlag_t;


/**//**
 * \brief MAC-FE transactions handling, first-or-second-attempt-to-transmit enumeration.
 * \details For the pending requests it is meaningful if the current attempt to transmit
 *  is the first attempt or the second (the third, etc.) attempt. On the first attempt the
 *  DSN field of MPDU.MHR is assigned with the current value of \e macDSN (and this
 *  attribute is then incremented). And on all the consecutive attempts to transmit this
 *  request, if it was not successfully transmitted at the first attempt, the DSN value is
 *  not reassigned but the initially assigned value is used. The initially assigned DSN
 *  value is stored in the pending request descriptor service field.
 */
typedef enum
{
    MAC_FE_TRANS_FIRST_ATTEMPT  = 0,    /*!< This is the first attempt to transmit the pending request. */
    MAC_FE_TRANS_SECOND_ATTEMPT = 1,    /*!< This is the second (the third, etc.) attempt to transmit.  */
} MacTransSecondAttempFlag_t;


/**//**
 * \brief MAC-FE transactions handling, timeout-expiration enumeration.
 * \details If this flag is set for the pending request, the requests dispatcher shall
 *  issue the \c TRANSACTION_EXPIRED confirmation instead of processing this request.
 */
typedef enum _MacTransactionsExpiredFlag_t
{
    MAC_FE_TRANS_STILL_ACTUAL    = 0,   /*!< Shall process the pending request.    */
    MAC_FE_TRANS_TIMEOUT_EXPIRED = 1,   /*!< Shall confirm the timeout expiration. */
} MacTransExpiredReqFlag_t;


/**//**
 * \brief MAC frame sequence number data type.
 */
typedef uint8_t  MacSeqNum_t;


/**//**
 * \brief MAC DSN (Data Sequence Number) data type.
 */
typedef MacSeqNum_t  MAC_Dsn_t;


/**//**
 * \brief MAC BSN (Beacon Sequence Number) data type.
 */
typedef MacSeqNum_t  MAC_Bsn_t;


/**//**
 * \brief MAC-FE transactions handling, the destination address hash value.
 */
typedef uint8_t  MacAddrHash_t;


/**//**
 * \brief MAC requests service field data type.
 */
typedef struct _MacServiceField_t
{
    /* 32-bit data. */
    SYS_QueueElement_t     queueElement;    /*!< Service field to queue up elements. */

#if defined(_MAC_CONTEXT_ZBPRO_)
    /* 32-bit data. */
    HAL_Symbol__Tstamp_t   lifeTimestamp;   /*!< Life timestamp of a pending transaction. */
#endif

    /* 8-bit data.  */
    MacReqPrimitiveId_t    primitiveId;     /*!< Request- or response-primitive identifier. */

#if defined(_MAC_CONTEXT_ZBPRO_)
    /* 8-bit data.  */
    MacAddrHash_t          dstAddrHash;     /*!< Destination address and PAN Id hash value. */

    MAC_Dsn_t              storedDsn;       /*!< Stored MPDU.MHR.DSN. */
#endif

#if defined(_MAC_CONTEXT_ZBPRO_)
    /* Structured / 8-bit data.  */
    union
    {
        BitField8_t                     transFlags     : 3;         /*!< Transactions flags. */

        struct
        {
            MacTransPendingReqFlag_t    pendingRequest : 1;         /*!< Pending request flag. */

            MacTransSecondAttempFlag_t  secondAttempt  : 1;         /*!< Second transmission attempt flag. */

            MacTransExpiredReqFlag_t    timeoutExpired : 1;         /*!< Timeout expiration flag. */

            MAC_DECLARE_CONTEXT_ID(     contextId      : 1 );       /*!< MAC-SAP context identifier. */
        };
    };
#endif
} MacServiceField_t;


/**//**
 * \brief   Macro returns pointer to the Service field by the QueueElement subfield.
 * \param[in]   queueItem   Pointer to the QueueElement subfield.
 * \return  Pointer to the Service field.
 */
#define MAC_SERVICE_FIELD_BY_QUEUE_ELEMENT(queueItem)\
        GET_PARENT_BY_FIELD(MacServiceField_t, queueElement, (queueItem))


/**//**
 * \brief   Macro returns pointer to the Request Descriptor by the Request Service field.
 * \param[in]   reqType         Name of the structured type of the Request Descriptor.
 * \param[in]   reqService      Pointer to the Request Service field.
 * \return  Pointer to the Request Descriptor.
 */
#define MAC_REQ_DESCR_BY_SERVICE(reqType, reqService)\
        GET_PARENT_BY_FIELD(reqType, service, (reqService))


#endif /* _BB_MAC_SAP_SERVICE_H */

/* eof bbMacSapService.h */