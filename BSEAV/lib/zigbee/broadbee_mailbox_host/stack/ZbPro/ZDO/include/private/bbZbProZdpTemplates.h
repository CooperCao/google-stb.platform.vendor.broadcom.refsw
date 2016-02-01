/******************************************************************************
* (c) 2014 Broadcom Corporation
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
******************************************************************************/
/*****************************************************************************
*
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   Templates of ZDP service functions
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZDP_TEMPLATE_H
#define _BB_ZBPRO_ZDP_TEMPLATE_H


/************************* INCLUDES *****************************************************/
#include "private/bbZbProZdpDispatcher.h"
#include "friend/bbZbProNwkNeighborTableFriend.h"

/****************************** TYPES ***************************************************/

/**//**
 * \brief Type of a Callback function which is used to issue fail status confirm
 */
typedef void ZbProZdpIssueFailConfirm_t(RpcTransaction_t *const rpcTransaction, ZBPRO_ZDO_Status_t status);

/************************* DEFINITIONS **************************************************/

/* Assert definitions used in Macro Templates */
#define SYS_DbgAssert_TEMPLATE              SYS_DbgAssert
#define SYS_DbgAssertComplex_TEMPLATE       SYS_DbgAssertComplex
#define SYS_DbgHalt_TEMPLATE                SYS_DbgHalt
#define SYS_DbgLog_TEMPLATE                 SYS_DbgLogId

/*************************** TEMPLATES **************************************************/

/*************************************************************************************//**
  \brief Common between services function to handle an APS Data confirm

  \param[in/out]    rpcTransaction  - Pointer to a Transaction
  \param[in]        failConfCb - callback pointer to issue confirmation to the NHL
  \return           directive to the dispatcher
*****************************************************************************************/
INLINE RpcServiceDirective_t zbProZdpHandleRpcLowLevelConfirm(
        RpcTransaction_t *const rpcTransaction,
        ZbProZdpIssueFailConfirm_t *failConfCb)
{
    if (RPC_TRANSACTION_TYPE_CLIENT == rpcTransaction->transType)
    {
        ZbProZdpTransaction_t *const  zdpTransaction =
            GET_PARENT_BY_FIELD(ZbProZdpTransaction_t, rpcTransactionDescr, rpcTransaction);

        SYS_DbgAssertComplex(NULL != zdpTransaction->apsDataConfParams,
                ZBPRO_ZDP_LOWERLEVEL_CONFIRM_APSDATACONFPARAMS_NULL);

        const ZBPRO_APS_Status_t  status =
            zdpTransaction->apsDataConfParams->status;

        if (zdpTransaction->isZdoConfIssued)
            return RPC_DIRECTIVE_DONT_WAIT_RESPONSE;

        if (ZBPRO_APS_SUCCESS_STATUS != status && ZBPRO_APS_NO_ACK_STATUS != status)
        {
            failConfCb(rpcTransaction, status);
            return RPC_DIRECTIVE_DONT_WAIT_RESPONSE;
        }

        SYS_DbgAssertComplex(rpcTransaction->isWaitingResponse,
                HALT_zbProZdpHandleRpcLowLevelConfirm_InconsistentFlags);

        return RPC_DIRECTIVE_RESTART_TIMEOUT;   /* Restart timeout when APS confirmed request transmission. */
    }
    else /* if (RPC_TRANSACTION_TYPE_SERVER == rpcTransaction->transType) */
    {
        /* Do nothing. When APSDE-DATA.confirm is received on ZDP Server Response, this Server Transaction is
         * fully completed. Embedded APSDE-DATA.request payload with ZDP Server Response frame will be freed by
         * the ZDP Dispatcher common kill-transaction function. */
        return RPC_DIRECTIVE_FINISH_TRANSACTION;
    }
}

/*************************************************************************************//**
  \brief Template of functions the first of them issues the specified confirm parameters to the NHL
         the second one issues the confirm with the specified fail status to the NHL.

  \param[in]    reqDescType - type of a request descriptor
  \param[in]    confType - type of confirmation
  \param[in]    servName - Name of a Service
*****************************************************************************************/
#define ZBPRO_ZDP_CONFIRM(servName)             zbProZdp ## servName ## IssueConfirm
#define ZBPRO_ZDP_FAIL_CONFIRM(servName)        zbProZdp ## servName ## IssueFailConfirm
#define ZBPRO_ZDP_CONFIRM_FUNCTIONS_TEMPLATE(reqDescType, confType, servName)                           \
                                                                                                        \
static void ZBPRO_ZDP_CONFIRM(servName)(                                                                \
        RpcTransaction_t *const rpcTransaction,                                                         \
        confType *const confParams)                                                                     \
{                                                                                                       \
    ZbProZdpTransaction_t *const  zdpTransaction =                                                      \
        GET_PARENT_BY_FIELD(ZbProZdpTransaction_t, rpcTransactionDescr, rpcTransaction);                \
                                                                                                        \
    SYS_DbgAssert_TEMPLATE(!zdpTransaction->isZdoConfIssued++,                                          \
            ZBPRO_ZDP_ ## servName ## _ISSUECONFIRM_REENTER);                                           \
                                                                                                        \
    reqDescType *req = GET_PARENT_BY_FIELD(reqDescType, service, rpcTransaction->localRequest);         \
                                                                                                        \
    if (NULL != req->callback)                                                                          \
        req->callback(req, confParams);                                                                 \
}                                                                                                       \
                                                                                                        \
static void ZBPRO_ZDP_FAIL_CONFIRM(servName)(                                                           \
        RpcTransaction_t *const rpcTransaction,                                                         \
        ZBPRO_ZDO_Status_t status)                                                                      \
{                                                                                                       \
    const reqDescType *const req =                                                                      \
        GET_PARENT_BY_FIELD(reqDescType, service, rpcTransaction->localRequest);                        \
                                                                                                        \
    confType confParams =                                                                               \
    {                                                                                                   \
        .status = status,                                                                               \
        .nwkAddrOfInterest = req->params.nwkAddrOfInterest                                              \
        /* other field initialized using zeros by the C standard */                                     \
    };                                                                                                  \
                                                                                                        \
    ZBPRO_ZDP_CONFIRM(servName)(rpcTransaction, &confParams);                                           \
}

/*************************************************************************************//**
  \brief Composes the request frame
*****************************************************************************************/
#define ZBPRO_ZDP_COMPOSE_REQUEST_FRAME_UNICAST(servName) zbProZdp ## servName ## ComposeRequest
#define ZBPRO_ZDP_COMPOSE_REQUEST_FRAME_UNICAST_TEMPLATE(reqDescType, servName, reqCommandSize)                     \
                                                                                                                    \
static RpcServiceDirective_t ZBPRO_ZDP_COMPOSE_REQUEST_FRAME_UNICAST(servName)(                                     \
        RpcTransaction_t *const rpcTransaction)                                                                     \
{                                                                                                                   \
    const reqDescType *const reqDesc =                                                                              \
        GET_PARENT_BY_FIELD(reqDescType, service, rpcTransaction->localRequest);                                    \
                                                                                                                    \
    ZBPRO_APS_DataReqParams_t *const  apsDataReqParams =                                                            \
        &GET_PARENT_BY_FIELD(ZbProZdpTransaction_t, rpcTransactionDescr, rpcTransaction)->apsDataReqDescr.params;   \
                                                                                                                    \
    /* Are the request parameters valid for an unicast service? */                                                  \
    if (!ZBPRO_APS_IS_ADDRESS_DESCRIPTION_UNICAST(&reqDesc->params.zdpDstAddress) ||                                \
            !ZBPRO_NWK_IS_ADDR_UNICAST(reqDesc->params.nwkAddrOfInterest))                                          \
    {                                                                                                               \
        ZBPRO_ZDP_FAIL_CONFIRM(servName)(rpcTransaction, ZBPRO_ZDO_INVALID_REQUEST_TYPE);                           \
        return RPC_DIRECTIVE_LOCAL_FAILURE;                                                                         \
    }                                                                                                               \
                                                                                                                    \
    if (!SYS_MemAlloc(&apsDataReqParams->payload, ZBPRO_ZDP_FRAME_HEADER_SIZE + reqCommandSize))                    \
        return RPC_DIRECTIVE_NO_MEMORY;                                                                             \

#define ZBPRO_ZDP_COMPOSE_REQUEST_FRAME_UNICAST_TEMPLATE_END                                                        \
    apsDataReqParams->dstAddress = reqDesc->params.zdpDstAddress;                                                   \
    apsDataReqParams->txOptions.ack = true;                                                                         \
                                                                                                                    \
    if ( 0 != reqDesc->params.respWaitTimeout)                                                                      \
        rpcTransaction->timeoutPeriod = reqDesc->params.respWaitTimeout;                                            \
                                                                                                                    \
    return RPC_DIRECTIVE_WAIT_RESPONSE;                                                                             \
}

#define ZBPRO_ZDP_PARSE_RESPONSE_FRAME_UNICAST(servName) zbProZdp ## servName ## ParseResponse
#define ZBPRO_ZDP_PARSE_RESPONSE_FRAME_UNICAST_TEMPLATE(confType, servName, responseParsingFunc)                    \
                                                                                                                    \
static RpcServiceDirective_t ZBPRO_ZDP_PARSE_RESPONSE_FRAME_UNICAST(servName)(                                      \
        RpcTransaction_t *const rpcTransaction)                                                                     \
{                                                                                                                   \
    ZBPRO_APS_DataIndParams_t *apsDataIndParams = rpcTransaction->remoteResponse.params;                            \
    confType conf;                                                                                                  \
    RpcServiceDirective_t returnValue;                                                                              \
                                                                                                                    \
    SYS_DbgAssertComplex_TEMPLATE(NULL != apsDataIndParams, ZBPRO_ZDP_ ## servName ## _PARSERESPONSE_IND_NULL);     \
    SYS_DataPointer_t *apsDataIndPayload = &apsDataIndParams->payload;                                              \
                                                                                                                    \
    if (responseParsingFunc(apsDataIndPayload, &conf))                                                              \
    {                                                                                                               \
        ZBPRO_ZDP_CONFIRM(servName)(rpcTransaction, &conf);                                                         \
        returnValue = RPC_DIRECTIVE_DONT_WAIT_RESPONSE;                                                             \
    }                                                                                                               \
    else                                                                                                            \
        returnValue = RPC_DIRECTIVE_BROKEN_RESPONSE;                                                                \
                                                                                                                    \
    if (SYS_CheckPayload(apsDataIndPayload))                                                                        \
        SYS_FreePayload(apsDataIndPayload);                                                                         \
    return returnValue;                                                                                             \
}

/*************************************************************************************//**
  \brief Processes Commands from ZDP Dispatcher
*****************************************************************************************/
#define ZBPRO_ZDP_RPC_SERVICE_HANDLER(servName) zbProZdpRpc ## servName ## ServiceHandler
#define ZBPRO_ZDP_RPC_SERVICE_HANDLER_DECLARATION(servName)                                                         \
RpcServiceDirective_t zbProZdpRpc ## servName ## ServiceHandler(                                                    \
                RpcTransaction_t   *const  rpcTransaction,                                                          \
                const RpcServiceCommand_t  rpcCommand)
#define ZBPRO_ZDP_RPC_SERVICE_HANDLER_TEMPLATE(servName,                                                            \
        failConfirmFunction, composeRequestFrameHandler, parseResponseFrameHandler,                                 \
        parseRequestFrameHandler)                                                                                   \
                                                                                                                    \
RpcServiceDirective_t ZBPRO_ZDP_RPC_SERVICE_HANDLER(servName)(                                                      \
                RpcTransaction_t   *const  rpcTransaction,                                                          \
                const RpcServiceCommand_t  rpcCommand)                                                              \
{                                                                                                                   \
    SYS_DbgAssertComplex_TEMPLATE((NULL == rpcTransaction) == (RPC_COMMAND_INITIALIZE_SERVICE == rpcCommand),       \
            ZBPRO_ZDP_ ## servName ## _SERVICEHANDLER_NULLRPCTRANSACTION);                                          \
                                                                                                                    \
    if (RPC_COMMAND_INITIALIZE_SERVICE != rpcCommand && RPC_TRANSACTION_TYPE_CLIENT == rpcTransaction->transType)   \
        SYS_DbgAssert_TEMPLATE(NULL != rpcTransaction->localRequest,                                                \
                ZBPRO_ZDP_ ## servName ## _SERVICEHANDLER_NULLREQ);                                                 \
                                                                                                                    \
    switch (rpcCommand)                                                                                             \
    {                                                                                                               \
        case RPC_COMMAND_INITIALIZE_SERVICE:                                                                        \
            /* nothing to do */                                                                                     \
            break;                                                                                                  \
                                                                                                                    \
        case RPC_COMMAND_COMPOSE_CLIENT_REQUEST:                                                                    \
            SYS_DbgAssertComplex_TEMPLATE(RPC_TRANSACTION_TYPE_CLIENT == rpcTransaction->transType,                 \
                    ZBPRO_ZDP_ ## servName ## _SERVICEHANDLER_UNEXP_TRANS_TYPE_ON_COMPOSE);                         \
                                                                                                                    \
            return composeRequestFrameHandler(rpcTransaction);                                                      \
                                                                                                                    \
        case RPC_COMMAND_PARSE_RESPONSE_CANDIDATE:                                                                  \
            SYS_DbgAssertComplex_TEMPLATE(RPC_TRANSACTION_TYPE_CLIENT == rpcTransaction->transType,                 \
                    ZBPRO_ZDP_ ## servName ## _SERVICEHANDLER_UNEXP_TRANS_TYPE_ON_RESP_IND);                        \
                                                                                                                    \
            return parseResponseFrameHandler(rpcTransaction);                                                       \
                                                                                                                    \
        case RPC_COMMAND_PROCESS_TIMEOUT_EXPIRATION:                                                                \
            SYS_DbgAssertComplex_TEMPLATE(RPC_TRANSACTION_TYPE_CLIENT == rpcTransaction->transType,                 \
                    ZBPRO_ZDP_ ## servName ## _SERVICEHANDLER_UNEXP_TRANS_TYPE_ON_TIMEOUT);                         \
                                                                                                                    \
            if (FALSE == rpcTransaction->isDataConfirmed)                                                           \
                return RPC_DIRECTIVE_RESTART_TIMEOUT;                                                               \
                                                                                                                    \
            failConfirmFunction(rpcTransaction, ZBPRO_ZDO_TIMEOUT);                                                 \
                                                                                                                    \
            return RPC_DIRECTIVE_DONT_WAIT_RESPONSE;                                                                \
                                                                                                                    \
        case RPC_COMMAND_PARSE_CLIENT_REQUEST:                                                                      \
                                                                                                                    \
            SYS_DbgAssertComplex_TEMPLATE(RPC_TRANSACTION_TYPE_SERVER == rpcTransaction->transType,                 \
                    ZBPRO_ZDP_ ## servName ## _SERVICEHANDLER_UNEXP_TRANS_TYPE_ON_PARSE_CLIENT_REQUEST);            \
                                                                                                                    \
            return parseRequestFrameHandler(rpcTransaction);                                                        \
                                                                                                                    \
        case RPC_COMMAND_PROCESS_LOWERLEVEL_CONFIRM:                                                                \
                                                                                                                    \
            return zbProZdpHandleRpcLowLevelConfirm(rpcTransaction, failConfirmFunction);                           \
                                                                                                                    \
        case RPC_COMMAND_PARSE_UNSOLICITED_RESPONSE:                                                                \
            /* Reject unsolicited Node_Desc_rsp, Power_Desc_rsp, Simple_Desc_rsp. */                                \
            return RPC_DIRECTIVE_FINISH_TRANSACTION;                                                                \
                                                                                                                    \
        case RPC_COMMAND_KILL_TRANSACTION:                                                                          \
            /* nothing to do */                                                                                     \
            break;                                                                                                  \
                                                                                                                    \
        default:                                                                                                    \
            SYS_DbgLog_TEMPLATE(ZBPRO_ZDP_ ## servName ## _SERVICEHANDLER_INVALID_RPCCOMMAND);                      \
            return RPC_DIRECTIVE_FINISH_TRANSACTION;                                                                \
    }                                                                                                               \
                                                                                                                    \
    return RPC_DIRECTIVE_PROHIBITED_CODE;                                                                           \
}

/*************************************************************************************//**
    \brief Performs ZDP Node/Power Desc request serialization.
 ****************************************************************************************/
INLINE void zbProZdoNodePowerDescSerializeReq(SYS_DataPointer_t *const payload, ZBPRO_ZDO_NwkAddr_t nwkAddrOfInterest)
{
    SYS_START_SERIALIZATION(SYS_GetPayloadSize(payload));
    SYS_SERIALIZE_SKIP(ZBPRO_ZDP_FRAME_HEADER_SIZE);
    SYS_SERIALIZE(&nwkAddrOfInterest);

    SYS_DbgAssert(SYS_IS_SERIALIZATION_VALID(), ZBPROZDONODEPOWERDESC_SERIALIZEREQ);
    SYS_FINISH_SERIALIZATION(payload, 0);
}

/*************************************************************************************//**
    \brief Performs ZDP Node/Power Desc request deserialization.
 ****************************************************************************************/
INLINE bool zbProZdoNodePowerDescDeserializeReq(
        SYS_DataPointer_t *const payload,
        ZBPRO_ZDO_NwkAddr_t *const nwkAddrOfInterest)
{
    SYS_DataLength_t payloadSize = SYS_GetPayloadSize(payload);

    if (ZBPRO_ZDP_FRAME_HEADER_SIZE + sizeof(ZBPRO_ZDO_NwkAddr_t) != payloadSize)
    {
        SYS_DbgLogId(ZBPROZDONODEPOWERDESC_DESERIALIZEREQ);
        return false;
    }

    SYS_START_DESERIALIZATION(payload, 0, payloadSize);
    SYS_DESERIALIZE_SKIP(ZBPRO_ZDP_FRAME_HEADER_SIZE);
    SYS_DESERIALIZE(nwkAddrOfInterest);
    SYS_FINISH_DESERIALIZATION();

    return true;
}

INLINE bool zbProZdpCheckAddrOfInterest(ZBPRO_ZDO_NwkAddr_t nwkAddrOfInterest, ZBPRO_ZDO_Status_t *const status)
{
    ZBPRO_ZDO_NwkAddr_t myNwkAddr;
    ZBPRO_ZDO_DeviceType_t deviceType;

    ZBPRO_APS_GET_ATTR(ZBPRO_NWK_NIB_DEVICE_TYPE, &deviceType);
    ZBPRO_APS_GET_ATTR(ZBPRO_NWK_NIB_NETWORK_ADDRESS, &myNwkAddr);

    if (myNwkAddr != nwkAddrOfInterest)
    {
        if (ZBPRO_DEVICE_TYPE_END_DEVICE == deviceType)
            *status = ZBPRO_ZDO_INVALID_REQUEST_TYPE;
        else if (zbProNwkIsChild(nwkAddrOfInterest))
            *status = ZBPRO_ZDO_NO_DESCRIPTOR;
        else
            *status = ZBPRO_ZDO_DEVICE_NOT_FOUND;
        return false;
    }
    else
        return true;
}

#endif /* _BB_ZBPRO_ZDP_TEMPLATE_H */
