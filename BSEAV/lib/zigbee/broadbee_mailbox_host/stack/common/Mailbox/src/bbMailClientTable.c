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
* FILENAME: $Workfile: branches/ext_xhuajun/MailboxIntegration/stack/common/Mailbox/src/bbMailClientTable.c $
*
* DESCRIPTION:
*         the mailbox client table implementation.
*
* $Revision: 3888 $
* $Date: 2014-10-04 00:03:46Z $
*
*****************************************************************************************/
/************************* INCLUDES *****************************************************/
#include "bbMailAPI.h"
#include "bbMailClientTable.h"

/************************* DEFINITIONS **************************************************/
#define MESSAGE_VERSION 1
#define GET_MESSAGE_TYPE(destinationHost, mType) \
    { \
        .version = MESSAGE_VERSION, \
        .subSystem = destinationHost, \
        .type = mType, \
    }

#define REQ_WITHOUT_DATA
#define REQ_WITH_DATA
#define REQ_WITH_NAMED_DATA
#define REQ_NO_PARAM
#define RESP_WITHOUT_DATA
#define RESP_WITH_DATA
#define IND_WITHOUT_DATA
#define IND_WITH_DATA
#define IND_WITHOUT_DATA_RESP
#define IND_WITH_DATA_RESP
#define IND_NO_PARAM_RESP
#define CLIENT_TABLE_ENTRY(entryType, ...) CLIENT_TABLE_ENTRY_##entryType(__VA_ARGS__)

#define CLIENT_TABLE_ENTRY_REQ_WITHOUT_DATA(fId, reqDescrType, reqParamsType, destinationHost) \
    {                                                                               \
        .id                 = fId,                                                  \
        .msgType            = GET_MESSAGE_TYPE(destinationHost, REQUEST_MSG_TYPE),  \
        .paramLength        = sizeof(reqParamsType),                                \
        .dataPointerOffset  = MAIL_INVALID_PAYLOAD_OFFSET,                          \
        .reqLength          = sizeof(reqDescrType),                                 \
        .paramOffset        = offsetof(reqDescrType, params),                       \
        .callbackOffset     = offsetof(reqDescrType, callback),                     \
    }

#define CLIENT_TABLE_ENTRY_REQ_WITH_DATA(fId, reqDescrType, reqParamsType, destinationHost) \
    {                                                                               \
        .id                 = fId,                                                  \
        .msgType            = GET_MESSAGE_TYPE(destinationHost, REQUEST_MSG_TYPE),  \
        .paramLength        = sizeof(reqParamsType),                                \
        .dataPointerOffset  = offsetof(reqParamsType, payload),                     \
        .reqLength          = sizeof(reqDescrType),                                 \
        .paramOffset        = offsetof(reqDescrType, params),                       \
        .callbackOffset     = offsetof(reqDescrType, callback),                     \
    }

#define CLIENT_TABLE_ENTRY_REQ_WITH_NAMED_DATA(fId, reqDescrType, reqParamsType, destinationHost, payloadName) \
    {                                                                               \
        .id                 = fId,                                                  \
        .msgType            = GET_MESSAGE_TYPE(destinationHost, REQUEST_MSG_TYPE),  \
        .paramLength        = sizeof(reqParamsType),                                \
        .dataPointerOffset  = offsetof(reqParamsType, payloadName),                 \
        .reqLength          = sizeof(reqDescrType),                                 \
        .paramOffset        = offsetof(reqDescrType, params),                       \
        .callbackOffset     = offsetof(reqDescrType, callback),                     \
    }

#define CLIENT_TABLE_ENTRY_REQ_NO_PARAM(fId, reqDescrType, reqParamsType, destinationHost) \
    {                                                                               \
        .id                 = fId,                                                  \
        .msgType            = GET_MESSAGE_TYPE(destinationHost, REQUEST_MSG_TYPE),  \
        .paramLength        = 0U,                                                   \
        .dataPointerOffset  = MAIL_INVALID_PAYLOAD_OFFSET,                          \
        .reqLength          = sizeof(reqDescrType),                                 \
        .paramOffset        = sizeof(reqDescrType),                                 \
        .callbackOffset     = offsetof(reqDescrType, callback),                     \
    }

#define CLIENT_TABLE_ENTRY_RESP_WITHOUT_DATA(fId, reqDescrType, reqParamsType, destinationHost) \
    {                                                                               \
        .id                 = fId,                                                  \
        .msgType            = GET_MESSAGE_TYPE(destinationHost, REQUEST_MSG_TYPE),  \
        .paramLength        = sizeof(reqParamsType),                                \
        .dataPointerOffset  = MAIL_INVALID_PAYLOAD_OFFSET,                          \
        .reqLength          = sizeof(reqDescrType),                                 \
        .paramOffset        = offsetof(reqDescrType, params),                       \
        .callbackOffset     = sizeof(reqDescrType),                                 \
    }

#define CLIENT_TABLE_ENTRY_RESP_WITH_DATA(fId, reqDescrType, reqParamsType, destinationHost) \
    {                                                                               \
        .id                 = fId,                                                  \
        .msgType            = GET_MESSAGE_TYPE(destinationHost, REQUEST_MSG_TYPE),  \
        .paramLength        = sizeof(reqParamsType),                                \
        .dataPointerOffset  = offsetof(reqParamsType, payload),                     \
        .reqLength          = sizeof(reqDescrType),                                 \
        .paramOffset        = offsetof(reqDescrType, params),                       \
        .callbackOffset     = sizeof(reqDescrType),                                 \
    }

#define CLIENT_TABLE_ENTRY_IND_WITHOUT_DATA(fId, reqDescrType, reqParamsType, destinationHost) \
    {                                                                               \
        .id                 = fId,                                                  \
        .msgType            = GET_MESSAGE_TYPE(destinationHost, INDICATION_MSG_TYPE), \
        .paramLength        = sizeof(reqParamsType),                                \
        .dataPointerOffset  = MAIL_INVALID_PAYLOAD_OFFSET,                          \
        .reqLength          = sizeof(reqParamsType),                                \
        .paramOffset        = 0U,                                                   \
        .callbackOffset     = sizeof(reqParamsType),                                \
    }

#define CLIENT_TABLE_ENTRY_IND_WITH_DATA(fId, reqDescrType, reqParamsType, destinationHost) \
    {                                                                               \
        .id                 = fId,                                                  \
        .msgType            = GET_MESSAGE_TYPE(destinationHost, INDICATION_MSG_TYPE), \
        .paramLength        = sizeof(reqParamsType),                                \
        .dataPointerOffset  = offsetof(reqParamsType, payload),                     \
        .reqLength          = sizeof(reqParamsType),                                \
        .paramOffset        = 0U,                                                   \
        .callbackOffset     = sizeof(reqParamsType),                                \
    }

#define CLIENT_TABLE_ENTRY_IND_WITHOUT_DATA_RESP(fId, reqDescrType, reqParamsType, destinationHost) \
    {                                                                               \
        .id                 = fId,                                                  \
        .msgType            = GET_MESSAGE_TYPE(destinationHost, INDICATION_MSG_TYPE),  \
        .paramLength        = sizeof(reqParamsType),                                \
        .dataPointerOffset  = MAIL_INVALID_PAYLOAD_OFFSET,                          \
        .reqLength          = sizeof(reqDescrType),                                 \
        .paramOffset        = offsetof(reqDescrType, params),                       \
        .callbackOffset     = offsetof(reqDescrType, callback),                     \
    }

#define CLIENT_TABLE_ENTRY_IND_WITH_DATA_RESP(fId, reqDescrType, reqParamsType, destinationHost) \
    {                                                                               \
        .id                 = fId,                                                  \
        .msgType            = GET_MESSAGE_TYPE(destinationHost, INDICATION_MSG_TYPE),  \
        .paramLength        = sizeof(reqParamsType),                                \
        .dataPointerOffset  = offsetof(reqParamsType, payload),                     \
        .reqLength          = sizeof(reqDescrType),                                 \
        .paramOffset        = offsetof(reqDescrType, params),                       \
        .callbackOffset     = offsetof(reqDescrType, callback),                     \
    }

#define CLIENT_TABLE_ENTRY_IND_NO_PARAM_RESP(fId, reqDescrType, reqParamsType, destinationHost) \
    {                                                                               \
        .id                 = fId,                                                  \
        .msgType            = GET_MESSAGE_TYPE(destinationHost, INDICATION_MSG_TYPE),  \
        .paramLength        = 0U,                                                   \
        .dataPointerOffset  = MAIL_INVALID_PAYLOAD_OFFSET,                          \
        .reqLength          = sizeof(reqDescrType),                                 \
        .paramOffset        = sizeof(reqDescrType),                                 \
        .callbackOffset     = offsetof(reqDescrType, callback),                     \
    }
typedef uint8_t NoAppropriateType_t;
static const MailClientParametersTableEntry_t clientTable[] =
{
/**********************************************************************************************************************/
/* Only for unit tests                                                                                                */
/**********************************************************************************************************************/
#ifdef MAILBOX_UNIT_TEST
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    MAIL_F1_FID,                    MailUnitTest_f1Descr_t,             MailUnitTest_f1ReqParams_t, HOST0),
    CLIENT_TABLE_ENTRY(REQ_NO_PARAM,        MAIL_F2_FID,                    MailUnitTest_f2Descr_t,             NoAppropriateType_t,        HOST0),
    CLIENT_TABLE_ENTRY(RESP_WITHOUT_DATA,   MAIL_F3_FID,                    MailUnitTest_f3Descr_t,             MailUnitTest_f3ReqParams_t, HOST0),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       MAIL_F4_FID,                    MailUnitTest_f4Descr_t,             MailUnitTest_f4ReqParams_t, HOST0),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    MAIL_F5_FID,                    MailUnitTest_f5Descr_t,             MailUnitTest_f5ReqParams_t, HOST0),
#endif /* MAILBOX_UNIT_TEST */

/**********************************************************************************************************************/
/* Test engine API                                                                                                    */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_TEST_ENGINE_)
# ifdef MAILBOX_HOST_SIDE
    CLIENT_TABLE_ENTRY(REQ_NO_PARAM,        TE_PING_FID,                    TE_PingCommandReqDescr_t,           NoAppropriateType_t,        UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       TE_ECHO_FID,                    TE_EchoCommandReqDescr_t,           TE_EchoCommandReqParams_t,  UART),
    CLIENT_TABLE_ENTRY(RESP_WITHOUT_DATA,   TE_RESET_FID,                   TE_ResetCommandReqDescr_t,          TE_ResetCommandReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    TE_ECHO_DELAY_FID,              TE_SetEchoDelayCommandReqDescr_t,   TE_SetEchoDelayCommandReqParams_t, UART),
    CLIENT_TABLE_ENTRY(RESP_WITH_DATA,      TE_HOST_TO_UART1_FID,           TE_Host2Uart1ReqDescr_t,            TE_Host2Uart1ReqParams_t,   UART),
# endif /* MAILBOX_HOST_SIDE */

# ifdef MAILBOX_STACK_SIDE
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    TE_HELLO_FID,                   NoAppropriateType_t,                TE_HelloCommandIndParams_t, UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    TE_ASSERT_LOGID_FID,            NoAppropriateType_t,                TE_AssertLogIdCommandIndParams_t, UART),
    CLIENT_TABLE_ENTRY(IND_WITH_DATA,       TE_UART1_TO_HOST_FID,           NoAppropriateType_t,                TE_Uart1ToHostReqParams_t,   UART),
# endif /* MAILBOX_STACK_SIDE */
#endif /* _MAILBOX_WRAPPERS_TEST_ENGINE_ */

/**********************************************************************************************************************/
/* Profiling Engine API                                                                                           */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_PROFILING_ENGINE_)
# ifdef MAILBOX_HOST_SIDE
    CLIENT_TABLE_ENTRY(RESP_WITHOUT_DATA,   PE_RESET_FID,                   SYS_DbgPeResetReqDescr_t,           SYS_DbgPeResetReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    PE_GETDATA_FID,                 SYS_DbgPeGetDataReqDescr_t,         SYS_DbgPeGetDataReqParams_t, UART),
# endif /* MAILBOX_HOST_SIDE */
#endif /* _MAILBOX_WRAPPERS_PROFILING_ENGINE_ */

/**********************************************************************************************************************/
/* MAC API                                                                                                            */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_MAC_)
# ifdef MAILBOX_HOST_SIDE
#  ifdef _MAC_BAN_TABLE_
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, MAC_REQ_BAN_SET_DEFAULT_ACTION_FID,       MAC_BanTableSetDefaultActionReqDescr_t, MAC_BanTableSetDefaultActionReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, MAC_REQ_BAN_LINK_FID,                     MAC_BanTableAddLinkReqDescr_t,          MAC_BanTableAddLinkReqParams_t,          UART),
#  endif /* _MAC_BAN_TABLE_ */
# endif /* MAILBOX_HOST_SIDE */
/**** ZBPRO *****************************************************************************/
# ifdef _ZBPRO_
#  ifdef MAILBOX_HOST_SIDE
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, ZBPRO_MAC_REQ_GET_FID,                    MAC_GetReqDescr_t,                      MAC_GetReqParams_t,                      UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,    ZBPRO_MAC_REQ_SET_FID,                    MAC_SetReqDescr_t,                      MAC_SetReqParams_t,                      UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,    ZBPRO_MAC_REQ_DATA_FID,                   MAC_DataReqDescr_t,                     MAC_DataReqParams_t,                     UART),
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_MAC_)
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, ZBPRO_MAC_REQ_PURGE_FID,                  MAC_PurgeReqDescr_t,                    MAC_PurgeReqParams_t,                    UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, ZBPRO_MAC_REQ_ASSOCIATE_FID,              MAC_AssociateReqDescr_t,                MAC_AssociateReqParams_t,                UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, ZBPRO_MAC_REQ_RESET_FID,                  MAC_ResetReqDescr_t,                    MAC_ResetReqParams_t,                    UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, ZBPRO_MAC_REQ_RX_ENABLE_FID,              MAC_RxEnableReqDescr_t,                 MAC_RxEnableReqParams_t,                 UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, ZBPRO_MAC_REQ_SCAN_FID,                   MAC_ScanReqDescr_t,                     MAC_ScanReqParams_t,                     UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, ZBPRO_MAC_REQ_START_FID,                  MAC_StartReqDescr_t,                    MAC_StartReqParams_t,                    UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, ZBPRO_MAC_RESP_ASSOCIATE_FID,             MAC_AssociateRespDescr_t,               MAC_AssociateRespParams_t,               UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, ZBPRO_MAC_RESP_ORPHAN_FID,                MAC_OrphanRespDescr_t,                  MAC_OrphanRespParams_t,                  UART),
#   endif /* WRAPPERS_ALL */
#  endif /* MAILBOX_HOST_SIDE */

#  ifdef MAILBOX_STACK_SIDE
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_MAC_)
    CLIENT_TABLE_ENTRY(IND_WITH_DATA,    ZBPRO_MAC_IND_DATA_FID,                   NoAppropriateType_t,                    MAC_DataIndParams_t,                     UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA, ZBPRO_MAC_IND_ASSOCIATE_FID,              NoAppropriateType_t,                    MAC_AssociateIndParams_t,                UART),
    CLIENT_TABLE_ENTRY(IND_WITH_DATA,    ZBPRO_MAC_IND_BEACON_NOTIFY_FID,          NoAppropriateType_t,                    MAC_BeaconNotifyIndParams_t,             UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA, ZBPRO_MAC_IND_COMM_STATUS_FID,            NoAppropriateType_t,                    MAC_CommStatusIndParams_t,               UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA, ZBPRO_MAC_IND_ORPHAN_FID,                 NoAppropriateType_t,                    MAC_OrphanIndParams_t,                   UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA, ZBPRO_MAC_IND_POLL_FID,                   NoAppropriateType_t,                    MAC_PollIndParams_t,                     UART),
#   endif /* WRAPPERS_ALL */
#  endif /* MAILBOX_STACK_SIDE */
# endif /* _ZBPRO_ */

/**** RF4CE *****************************************************************************/
# ifdef _RF4CE_
#  ifdef MAILBOX_HOST_SIDE
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, RF4CE_MAC_REQ_GET_FID,                    MAC_GetReqDescr_t,                      MAC_GetReqParams_t,                      UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,    RF4CE_MAC_REQ_SET_FID,                    MAC_SetReqDescr_t,                      MAC_SetReqParams_t,                      UART),
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_MAC_)
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,    RF4CE_MAC_REQ_DATA_FID,                   MAC_DataReqDescr_t,                     MAC_DataReqParams_t,                     UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, RF4CE_MAC_REQ_RESET_FID,                  MAC_ResetReqDescr_t,                    MAC_ResetReqParams_t,                    UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, RF4CE_MAC_REQ_RX_ENABLE_FID,              MAC_RxEnableReqDescr_t,                 MAC_RxEnableReqParams_t,                 UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, RF4CE_MAC_REQ_START_FID,                  MAC_StartReqDescr_t,                    MAC_StartReqParams_t,                    UART),
#   if defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, RF4CE_MAC_REQ_SCAN_FID,                   MAC_ScanReqDescr_t,                     MAC_ScanReqParams_t,                     UART),
#    endif /* RF4CE_TARGET */
#   endif /* WRAPPERS_ALL */
#  endif /* MAILBOX_HOST_SIDE */

#  ifdef MAILBOX_STACK_SIDE
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_MAC_)
    CLIENT_TABLE_ENTRY(IND_WITH_DATA,    RF4CE_MAC_IND_DATA_FID,                   NoAppropriateType_t,                    MAC_DataIndParams_t,                     UART),
#    if defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
    CLIENT_TABLE_ENTRY(IND_WITH_DATA,    RF4CE_MAC_IND_BEACON_NOTIFY_FID,          NoAppropriateType_t,                    MAC_BeaconNotifyIndParams_t,             UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA, RF4CE_MAC_IND_COMM_STATUS_FID,            NoAppropriateType_t,                    MAC_CommStatusIndParams_t,               UART),
#    endif /* RF4CE_TARGET */
#   endif /* WRAPPERS_ALL */
#  endif /* MAILBOX_STACK_SIDE */
# endif /* _RF4CE_ */
#endif /* _MAILBOX_WRAPPERS_MAC_ */

# ifdef MAILBOX_STACK_SIDE
    CLIENT_TABLE_ENTRY(IND_WITH_DATA,       SYS_PRINT_FID,         NoAppropriateType_t,                SYS_PrintIndParams_t,   UART),
# endif /* MAILBOX_STACK_SIDE */

/**********************************************************************************************************************/
/* NWK API                                                                                                            */
/**********************************************************************************************************************/

/**** RF4CE *****************************************************************************/
#if defined(_RF4CE_) && (WRAPPERS_OFF != _MAILBOX_WRAPPERS_NWK_)
# ifdef MAILBOX_HOST_SIDE
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_NWK_REQ_RESET_FID,        RF4CE_NWK_ResetReqDescr_t,          RF4CE_NWK_ResetReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_NO_PARAM,        RF4CE_NWK_REQ_START_FID,        RF4CE_NWK_StartReqDescr_t,          NoAppropriateType_t,        UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       RF4CE_NWK_REQ_DATA_FID,         RF4CE_NWK_DataReqDescr_t,           RF4CE_NWK_DataReqParams_t,  UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_NWK_REQ_GET_FID,          RF4CE_NWK_GetReqDescr_t,            RF4CE_NWK_GetReqParams_t,   UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_NWK_REQ_SET_FID,          RF4CE_NWK_SetReqDescr_t,            RF4CE_NWK_SetReqParams_t,   UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       RF4CE_NWK_REQ_DISCOVERY_FID,    RF4CE_NWK_DiscoveryReqDescr_t,      RF4CE_NWK_DiscoveryReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       RF4CE_NWK_REQ_AUTODISCOVERY_FID, RF4CE_NWK_AutoDiscoveryReqDescr_t, RF4CE_NWK_AutoDiscoveryReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       RF4CE_NWK_REQ_PAIR_FID,         RF4CE_NWK_PairReqDescr_t,           RF4CE_NWK_PairReqParams_t,  UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_NWK_REQ_UNPAIR_FID,       RF4CE_NWK_UnpairReqDescr_t,         RF4CE_NWK_UnpairReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_NWK_REQ_RXENABLE_FID,     RF4CE_NWK_RXEnableReqDescr_t,       RF4CE_NWK_RXEnableReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_NWK_REQ_UPDATEKEY_FID,    RF4CE_NWK_UpdateKeyReqDescr_t,      RF4CE_NWK_UpdateKeyReqParams_t, UART),

    CLIENT_TABLE_ENTRY(RESP_WITH_DATA,      RF4CE_NWK_RESP_DISCOVERY_FID,   RF4CE_NWK_DiscoveryRespDescr_t,     RF4CE_NWK_DiscoveryRespParams_t, UART),
    CLIENT_TABLE_ENTRY(RESP_WITH_DATA,      RF4CE_NWK_RESP_PAIR_FID,        RF4CE_NWK_PairRespDescr_t,          RF4CE_NWK_PairRespParams_t,  UART),
    CLIENT_TABLE_ENTRY(RESP_WITHOUT_DATA,   RF4CE_NWK_RESP_UNPAIR_FID,      RF4CE_NWK_UnpairRespDescr_t,        RF4CE_NWK_UnpairRespParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_NWK_MAC_STATS_REQ_FID,    RF4CE_NWK_MacStatsReqDescr_t,       RF4CE_NWK_MacStatsReqParams_t, UART),
# endif /* MAILBOX_HOST_SIDE */

# ifdef MAILBOX_STACK_SIDE
#  if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_NWK_)
    CLIENT_TABLE_ENTRY(IND_WITH_DATA,       RF4CE_NWK_IND_DATA_FID,         NoAppropriateType_t,                RF4CE_NWK_DataIndParams_t,   UART),
    CLIENT_TABLE_ENTRY(IND_WITH_DATA,       RF4CE_NWK_IND_DISCOVERY_FID,    NoAppropriateType_t,                RF4CE_NWK_DiscoveryIndParams_t, UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    RF4CE_NWK_IND_COMMSTATUS_FID,   NoAppropriateType_t,                RF4CE_NWK_COMMStatusIndParams_t, UART),
    CLIENT_TABLE_ENTRY(IND_WITH_DATA,       RF4CE_NWK_IND_PAIR_FID,         NoAppropriateType_t,                RF4CE_NWK_PairIndParams_t,   UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    RF4CE_NWK_IND_UNPAIR_FID,       NoAppropriateType_t,                RF4CE_NWK_UnpairIndParams_t, UART),
#   ifdef ENABLE_GU_KEY_SEED_IND
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    RF4CE_NWK_KEY_SEED_START_IND_FID,   NoAppropriateType_t,                RF4CE_NWK_KeySeedStartIndParams_t, UART),
#   endif /* ENABLE_GU_KEY_SEED_IND */
#  endif /* WRAPPERS_ALL */
# endif /* MAILBOX_STACK_SIDE */
#endif /* defined(_RF4CE_) && (WRAPPERS_OFF != _MAILBOX_WRAPPERS_NWK_) */

/**** ZBPRO *****************************************************************************/
#ifdef _ZBPRO_
# if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_NWK_)
#  ifdef MAILBOX_HOST_SIDE
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_NWK_REQ_PERMIT_JOINING_FID, ZBPRO_NWK_PermitJoiningReqDescr_t, ZBPRO_NWK_PermitJoiningReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_NWK_REQ_LEAVE_FID,        ZBPRO_NWK_LeaveReqDescr_t,          ZBPRO_NWK_LeaveReqParams_t, UART),
    // reserved for ZBPRO_NWK_REQ_GET_FID
    // reserved for ZBPRO_NWK_REQ_SET_FID
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_NWK_REQ_GET_KEY_FID,      ZBPRO_NWK_GetKeyReqDescr_t,         ZBPRO_NWK_GetKeyReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_NWK_REQ_SET_KEY_FID,      ZBPRO_NWK_SetKeyReqDescr_t,         ZBPRO_NWK_SetKeyReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_NWK_REQ_ROUTE_DISCOVERY_FID, ZBPRO_NWK_RouteDiscoveryReqDescr_t, ZBPRO_NWK_RouteDiscoveryReqParams_t, UART),
#  endif /* MAILBOX_HOST_SIDE */
# endif /* _MAILBOX_WRAPPERS_NWK_ */
#endif /* _ZBPRO_ */

/**********************************************************************************************************************/
/* APS API                                                                                                            */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_APS_)
# ifdef _ZBPRO_
# ifdef MAILBOX_HOST_SIDE
    CLIENT_TABLE_ENTRY(REQ_WITH_NAMED_DATA, ZBPRO_APS_REQ_ENDPOINTREGISTER_FID, ZBPRO_APS_EndpointRegisterReqDescr_t, ZBPRO_APS_EndpointRegisterReqParams_t, UART, simpleDescriptor.inOutClusterList),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_APS_REQ_ENDPOINTUNREGISTER_FID, ZBPRO_APS_EndpointUnregisterReqDescr_t, ZBPRO_APS_EndpointUnregisterReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       ZBPRO_APS_REQ_DATA_FID,         ZBPRO_APS_DataReqDescr_t,           ZBPRO_APS_DataReqParams_t,  UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_APS_REQ_BIND_FID,         ZBPRO_APS_BindUnbindReqDescr_t,     ZBPRO_APS_BindUnbindReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_APS_REQ_UNBIND_FID,       ZBPRO_APS_BindUnbindReqDescr_t,     ZBPRO_APS_BindUnbindReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_APS_REQ_GET_FID,          ZBPRO_APS_GetReqDescr_t,            ZBPRO_APS_GetReqParams_t,   UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       ZBPRO_APS_REQ_SET_FID,          ZBPRO_APS_SetReqDescr_t,            ZBPRO_APS_SetReqParams_t,   UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_APS_REQ_GET_KEY_FID,      ZBPRO_APS_GetKeyReqDescr_t,         ZBPRO_APS_GetKeyReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_APS_REQ_SET_KEY_FID,      ZBPRO_APS_SetKeyReqDescr_t,         ZBPRO_APS_SetKeyReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_APS_REQ_ADDGROUP_FID,     ZBPRO_APS_AddGroupReqDescr_t,       ZBPRO_APS_AddGroupReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_APS_REQ_REMOVEGROUP_FID,  ZBPRO_APS_RemoveGroupReqDescr_t,    ZBPRO_APS_RemoveGroupReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_APS_REQ_REMOVEALLGROUPS_FID, ZBPRO_APS_RemoveAllGroupsReqDescr_t, ZBPRO_APS_RemoveAllGroupsReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_APS_REQ_TRANSPORTKEY_FID, ZBPRO_APS_TransportKeyReqDescr_t,   ZBPRO_APS_TransportKeyReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_APS_REQ_UPDATEDEVICE_FID, ZBPRO_APS_UpdateDeviceReqDescr_t,   ZBPRO_APS_UpdateDeviceReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_APS_REQ_REMOTEDEVICE_FID, ZBPRO_APS_RemoveDeviceReqDescr_t,   ZBPRO_APS_RemoveDeviceReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_APS_REQ_REQUESTKEY_FID,   ZBPRO_APS_RequestKeyReqDescr_t,     ZBPRO_APS_RequestKeyReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_APS_REQ_SWITCHKEY_FID,    ZBPRO_APS_SwitchKeyReqDescr_t,      ZBPRO_APS_SwitchKeyReqParams_t, UART),
# endif /* MAILBOX_HOST_SIDE */

# ifdef MAILBOX_STACK_SIDE
    CLIENT_TABLE_ENTRY(IND_WITH_DATA,       ZBPRO_APS_IND_DATA_FID,         NoAppropriateType_t,                ZBPRO_APS_DataIndParams_t,  UART),
#  if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_APS_)
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    ZBPRO_APS_IND_TRANSPORTKEY_FID, NoAppropriateType_t,                ZBPRO_APS_TransportKeyIndParams_t, UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    ZBPRO_APS_IND_UPDATEDEVICE_FID, NoAppropriateType_t,                ZBPRO_APS_UpdateDeviceIndParams_t, UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    ZBPRO_APS_IND_REMOTEDEVICE_FID, NoAppropriateType_t,                ZBPRO_APS_RemoveDeviceIndParams_t, UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    ZBPRO_APS_IND_REQUESTKEY_FID,   NoAppropriateType_t,                ZBPRO_APS_RequestKeyIndParams_t, UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    ZBPRO_APS_IND_SWITCHKEY_FID,    NoAppropriateType_t,                ZBPRO_APS_SwitchKeyIndParams_t, UART),
#   endif /* WRAPPERS_ALL */
#  endif /* MAILBOX_STACK_SIDE */
# endif /* _ZBPRO_ */
#endif /* _MAILBOX_WRAPPERS_APS_ */

/**********************************************************************************************************************/
/* ZDO API                                                                                                            */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZDO_)
# ifdef _ZBPRO_
#  ifdef MAILBOX_HOST_SIDE
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZDO_REQ_ADDR_RESOLVING_FID,           ZBPRO_ZDO_AddrResolvingReqDescr_t,          ZBPRO_ZDO_AddrResolvingReqParams_t,         UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZDO_REQ_NODE_DESC_FID,                ZBPRO_ZDO_NodeDescReqDescr_t,               ZBPRO_ZDO_NodeDescReqParams_t,              UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZDO_REQ_POWER_DESC_FID,               ZBPRO_ZDO_PowerDescReqDescr_t,              ZBPRO_ZDO_PowerDescReqParams_t,             UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZDO_REQ_SIMPLE_DESC_FID,              ZBPRO_ZDO_SimpleDescReqDescr_t,             ZBPRO_ZDO_SimpleDescReqParams_t,            UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZDO_REQ_ACTIVE_EP_FID,                ZBPRO_ZDO_ActiveEpReqDescr_t,               ZBPRO_ZDO_ActiveEpReqParams_t,              UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_NAMED_DATA, ZBPRO_ZDO_REQ_MATCH_DESC_FID,               ZBPRO_ZDO_MatchDescReqDescr_t,              ZBPRO_ZDO_MatchDescReqParams_t,             UART, inOutClusterList),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZDO_REQ_DEVICE_ANNCE_FID,             ZBPRO_ZDO_DeviceAnnceReqDescr_t,            ZBPRO_ZDO_DeviceAnnceReqParams_t,           UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZDO_REQ_SERVER_DISCOVERY_FID,         ZBPRO_ZDO_ServerDiscoveryReqDescr_t,        ZBPRO_ZDO_ServerDiscoveryReqParams_t,       UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_NAMED_DATA, ZBPRO_ZDO_REQ_ED_BIND_FID,                  ZBPRO_ZDO_EndDeviceBindReqDescr_t,          ZBPRO_ZDO_EndDeviceBindReqParams_t,         UART, clusterList),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZDO_REQ_BIND_FID,                     ZBPRO_ZDO_BindUnbindReqDescr_t,             ZBPRO_ZDO_BindUnbindReqParams_t,            UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZDO_REQ_UNBIND_FID,                   ZBPRO_ZDO_BindUnbindReqDescr_t,             ZBPRO_ZDO_BindUnbindReqParams_t,            UART),
    CLIENT_TABLE_ENTRY(REQ_NO_PARAM,        ZBPRO_ZDO_REQ_START_NETWORK_FID,            ZBPRO_ZDO_StartNetworkReqDescr_t,           NoAppropriateType_t,                        UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZDO_REQ_MGMT_LEAVE_FID,               ZBPRO_ZDO_MgmtLeaveReqDescr_t,              ZBPRO_ZDO_MgmtLeaveReqParams_t,             UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZDO_REQ_MGMT_PERMIT_JOINING_FID,      ZBPRO_ZDO_MgmtPermitJoiningReqDescr_t,      ZBPRO_ZDO_MgmtPermitJoiningReqParams_t,     UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZDO_REQ_MGMT_NWK_UPDATE_FID,          ZBPRO_ZDO_MgmtNwkUpdateReqDescr_t,          ZBPRO_ZDO_MgmtNwkUpdateReqParams_t,         UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       ZBPRO_ZDO_RESP_MGMT_NWK_UPDATE_UNSOL_FID,   ZBPRO_ZDO_MgmtNwkUpdateUnsolRespDescr_t,    ZBPRO_ZDO_MgmtNwkUpdateUnsolRespParams_t,   UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZDO_REQ_MGMT_LQI_FID,                 ZBPRO_ZDO_MgmtLqiReqDescr_t,                ZBPRO_ZDO_MgmtLqiReqParams_t,               UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZDO_REQ_MGMT_BIND_FID,                ZBPRO_ZDO_MgmtBindReqDescr_t,               ZBPRO_ZDO_MgmtBindReqParams_t,              UART),
#  endif /* MAILBOX_HOST_SIDE */
#  ifdef MAILBOX_STACK_SIDE
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_ZDO_)
    CLIENT_TABLE_ENTRY(IND_WITH_DATA,       ZBPRO_ZDO_IND_MGMT_NWK_UPDATE_UNSOL_FID,    NoAppropriateType_t,                        ZBPRO_ZDO_MgmtNwkUpdateUnsolIndParams_t,    UART),
#   endif /* WRAPPERS_ALL */
#  endif /* MAILBOX_STACK_SIDE */
# endif /* _ZBPRO_ */
#endif /* _MAILBOX_WRAPPERS_ZDO_ */

/**********************************************************************************************************************/
/* TC API                                                                                                            */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_TC_)
# ifdef _ZBPRO_
#  ifdef MAILBOX_HOST_SIDE
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_TC_REQ_NWK_KEY_UPDATE_FID, ZBPRO_TC_NwkKeyUpdateReqDescr_t, ZBPRO_TC_NwkKeyUpdateReqParams_t, UART),
#  endif /* MAILBOX_HOST_SIDE */
# endif /* _ZBPRO_ */
#endif /* _MAILBOX_WRAPPERS_TC_ */

/**********************************************************************************************************************/
/* ZCL API                                                                                                            */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_)
/**** ZBPRO *****************************************************************************/
# ifdef _ZBPRO_
#  ifdef MAILBOX_HOST_SIDE
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_REQ_SET_POWER_SOURCE_FID,                             ZBPRO_ZCL_SetPowerSourceReqDescr_t,                     ZBPRO_ZCL_SetPowerSourceReqParams_t,                        UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_PROFILE_WIDE_CMD_DISCOVER_ATTRIBUTE_REQ_FID,          ZBPRO_ZCL_ProfileWideCmdDiscoverAttrDescr_t,            ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReqParams_t,            UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_PROFILE_WIDE_CMD_READ_ATTRIBUTE_REQ_FID,              ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t,             ZBPRO_ZCL_ProfileWideCmdReadAttrReqParams_t,                UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       ZBPRO_ZCL_PROFILE_WIDE_CMD_WRITE_ATTRIBUTE_REQ_FID,             ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t,            ZBPRO_ZCL_ProfileWideCmdWriteAttrReqParams_t,               UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       ZBPRO_ZCL_PROFILE_WIDE_CMD_CONFIGURE_REPORTING_REQ_FID,     ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t,   ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqParams_t,  UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_PROFILE_WIDE_CMD_READ_REPORTING_CONFIGURATION_REQ_FID,  ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqDescr_t,  ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqParams_t,  UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_REQ_FID,                        ZBPRO_ZCL_IdentifyCmdIdentifyReqDescr_t,                ZBPRO_ZCL_IdentifyCmdIdentifyReqParams_t,                   UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_REQ_FID,                  ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqDescr_t,           ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqParams_t,              UART),
#   if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_IDENTIFY_RELAY_)
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_RESPONSE_REQ_FID,               ZBPRO_ZCL_IdentifyCmdIdentifyResponseReqDescr_t,        ZBPRO_ZCL_IdentifyCmdIdentifyResponseReqParams_t,           UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_RESPONSE_REQ_FID,         ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReqDescr_t,   ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReqParams_t,      UART),
#   endif
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       ZBPRO_ZCL_GROUPS_CMD_ADD_GROUP_REQ_FID,                         ZBPRO_ZCL_GroupsCmdAddGroupReqDescr_t,                  ZBPRO_ZCL_GroupsCmdAddGroupReqParams_t,                     UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_GROUPS_CMD_VIEW_GROUP_REQ_FID,                        ZBPRO_ZCL_GroupsCmdViewGroupReqDescr_t,                 ZBPRO_ZCL_GroupsCmdViewGroupReqParams_t,                    UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       ZBPRO_ZCL_GROUPS_CMD_GET_GROUP_MEMBERSHIP_REQ_FID,              ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqDescr_t,        ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqParams_t,           UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_GROUPS_CMD_REMOVE_GROUP_REQ_FID,                      ZBPRO_ZCL_GroupsCmdRemoveGroupReqDescr_t,               ZBPRO_ZCL_GroupsCmdRemoveGroupReqParams_t,                  UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_GROUPS_CMD_REMOVE_ALL_GROUPS_REQ_FID,                 ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqDescr_t,           ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqParams_t,              UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       ZBPRO_ZCL_GROUPS_CMD_ADD_GROUP_IF_IDENTIFY_REQ_FID,             ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqDescr_t,        ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqParams_t,           UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       ZBPRO_ZCL_SCENES_CMD_ADD_SCENE_REQ_FID,                         ZBPRO_ZCL_ScenesCmdAddSceneReqDescr_t,                  ZBPRO_ZCL_ScenesCmdAddSceneReqParams_t,                     UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_SCENES_CMD_VIEW_SCENE_REQ_FID,                        ZBPRO_ZCL_ScenesCmdViewSceneReqDescr_t,                 ZBPRO_ZCL_ScenesCmdViewSceneReqParams_t,                    UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_SCENES_CMD_STORE_SCENE_REQ_FID,                       ZBPRO_ZCL_ScenesCmdStoreSceneReqDescr_t,                ZBPRO_ZCL_ScenesCmdStoreSceneReqParams_t,                   UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_SCENES_CMD_RECALL_SCENE_REQ_FID,                      ZBPRO_ZCL_ScenesCmdRecallSceneReqDescr_t,               ZBPRO_ZCL_ScenesCmdRecallSceneReqParams_t,                  UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_SCENES_CMD_REMOVE_SCENE_REQ_FID,                      ZBPRO_ZCL_ScenesCmdRemoveSceneReqDescr_t,               ZBPRO_ZCL_ScenesCmdRemoveSceneReqParams_t,                  UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_SCENES_CMD_REMOVE_ALL_SCENES_REQ_FID,                 ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqDescr_t,           ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqParams_t,              UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_SCENES_CMD_GET_SCENE_MEMBERSHIP_REQ_FID,              ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqDescr_t,        ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqParams_t,           UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_ONOFF_CMD_OFF_REQ_FID,                                ZBPRO_ZCL_OnOffCmdReqDescr_t,                           ZBPRO_ZCL_OnOffCmdReqParams_t,                              UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_ONOFF_CMD_ON_REQ_FID,                                 ZBPRO_ZCL_OnOffCmdReqDescr_t,                           ZBPRO_ZCL_OnOffCmdReqParams_t,                              UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_ONOFF_CMD_TOGGLE_REQ_FID,                             ZBPRO_ZCL_OnOffCmdReqDescr_t,                           ZBPRO_ZCL_OnOffCmdReqParams_t,                              UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_LEVEL_CONTROL_CMD_MOVE_TO_LEVEL_REQ_FID,              ZBPRO_ZCL_LevelControlCmdMoveToLevelReqDescr_t,         ZBPRO_ZCL_LevelControlCmdMoveToLevelReqParams_t,            UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_LEVEL_CONTROL_CMD_MOVE_REQ_FID,                       ZBPRO_ZCL_LevelControlCmdMoveReqDescr_t,                ZBPRO_ZCL_LevelControlCmdMoveReqParams_t,                   UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_LEVEL_CONTROL_CMD_STEP_REQ_FID,                       ZBPRO_ZCL_LevelControlCmdStepReqDescr_t,                ZBPRO_ZCL_LevelControlCmdStepReqParams_t,                   UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_LEVEL_CONTROL_CMD_STOP_REQ_FID,                       ZBPRO_ZCL_LevelControlCmdStopReqDescr_t,                ZBPRO_ZCL_LevelControlCmdStopReqParams_t,                   UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_NAMED_DATA, ZBPRO_ZCL_DOOR_LOCK_CMD_LOCK_REQ_FID,                           ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t,              ZBPRO_ZCL_DoorLockCmdLockUnlockReqParams_t,                 UART, codeString),
    CLIENT_TABLE_ENTRY(REQ_WITH_NAMED_DATA, ZBPRO_ZCL_DOOR_LOCK_CMD_UNLOCK_REQ_FID,                         ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t,              ZBPRO_ZCL_DoorLockCmdLockUnlockReqParams_t,                 UART, codeString),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_WINDOW_COVERING_CMD_UP_OPEN_REQ_FID,                  ZBPRO_ZCL_WindowCoveringCmdReqDescr_t,                  ZBPRO_ZCL_WindowCoveringCmdReqParams_t,                     UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_WINDOW_COVERING_CMD_DOWN_CLOSE_REQ_FID,               ZBPRO_ZCL_WindowCoveringCmdReqDescr_t,                  ZBPRO_ZCL_WindowCoveringCmdReqParams_t,                     UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_WINDOW_COVERING_CMD_STOP_REQ_FID,                     ZBPRO_ZCL_WindowCoveringCmdReqDescr_t,                  ZBPRO_ZCL_WindowCoveringCmdReqParams_t,                     UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_WINDOW_COVERING_CMD_GO_TO_LIFT_PERCENTAGE_REQ_FID,    ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t,   ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqParams_t,      UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_WINDOW_COVERING_CMD_GO_TO_TILT_PERCENTAGE_REQ_FID,    ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t,   ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqParams_t,      UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_ENROLL_RESPONSE_REQ_FID,            ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqDescr_t,       ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqParams_t,      UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_IAS_ACE_ARM_RESP_REQ_FID,                             ZBPRO_ZCL_SapIasAceArmRespReqDescr_t,                   ZBPRO_ZCL_SapIasAceArmRespReqParams_t,                      UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       ZBPRO_ZCL_IAS_ACE_BYPASS_RESP_REQ_FID,                          ZBPRO_ZCL_SapIasAceBypassRespReqDescr_t,                ZBPRO_ZCL_SapIasAceBypassRespReqParams_t,                   UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_IAS_ACE_GET_ZONE_ID_MAP_RESP_REQ_FID,                 ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqDescr_t,          ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqParams_t,             UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       ZBPRO_ZCL_IAS_ACE_GET_ZONE_INFO_RESP_REQ_FID,                   ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqDescr_t,           ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqParams_t,              UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_IAS_ACE_GET_PANEL_STATUS_RESP_REQ_FID,                ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqDescr_t,        ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqParams_t,           UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       ZBPRO_ZCL_IAS_ACE_SET_BYPASSED_ZONE_LIST_RESP_REQ_FID,          ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqDescr_t,   ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqParams_t,      UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       ZBPRO_ZCL_IAS_ACE_GET_ZONE_STATUS_RESP_REQ_FID,                 ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqDescr_t,         ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqParams_t,            UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       ZBPRO_ZCL_IAS_ACE_ZONE_STATUS_CHANGED_REQ_FID,                  ZBPRO_ZCL_SapIasAceZoneStatusChangedReqDescr_t,         ZBPRO_ZCL_SapIasAceZoneStatusChangedReqParams_t,            UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_IAS_ACE_PANEL_STATUS_CHANGED_REQ_FID,                 ZBPRO_ZCL_SapIasAcePanelStatusChangedReqDescr_t,        ZBPRO_ZCL_SapIasAcePanelStatusChangedReqParams_t,           UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_IAS_WD_CMD_START_WARNING_REQ_FID,                     ZBPRO_ZCL_IASWDCmdStartWarningReqDescr_t,               ZBPRO_ZCL_IASWDCmdStartWarningReqParams_t,                  UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_IAS_WD_CMD_SQUAWK_REQ_FID,                            ZBPRO_ZCL_IASWDCmdSquawkReqDescr_t,                     ZBPRO_ZCL_IASWDCmdSquawkReqParams_t,                        UART),
#  endif /* MAILBOX_HOST_SIDE */
#  ifdef MAILBOX_STACK_SIDE
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_ZCL_)
#    if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_IDENTIFY_RELAY_)
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_IND_FID,                        NoAppropriateType_t,                                    ZBPRO_ZCL_IdentifyCmdIdentifyIndParams_t,                   UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_IND_FID,                  NoAppropriateType_t,                                    ZBPRO_ZCL_IdentifyCmdIdentifyQueryIndParams_t,              UART),
#    else
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    ZBPRO_ZCL_IND_IDENTIFY_FID,                                     NoAppropriateType_t,                                    ZBPRO_ZCL_IdentifyIndParams_t,                              UART),
#    endif
    CLIENT_TABLE_ENTRY(IND_WITH_DATA,       ZBPRO_ZCL_PROFILE_WIDE_CMD_REPORT_ATTRIBUTES_IND_FID,           NoAppropriateType_t,                                    ZBPRO_ZCL_ProfileWideCmdReportAttributesIndParams_t,        UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_RESPONSE_IND_FID,         NoAppropriateType_t,                                    ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseIndParams_t,      UART),
    CLIENT_TABLE_ENTRY(IND_WITH_DATA,       ZBPRO_ZCL_GROUPS_CMD_GET_GROUP_MEMBERSHIP_RESPONSE_IND_FID,     NoAppropriateType_t,                                    ZBPRO_ZCL_GroupsCmdGetGroupMembershipIndParams_t,           UART),
    CLIENT_TABLE_ENTRY(IND_WITH_DATA,       ZBPRO_ZCL_SCENES_CMD_GET_SCENE_MEMBERSHIP_RESPONSE_IND_FID,     NoAppropriateType_t,                                    ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseIndParams_t,   UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_ENROLL_IND_FID,                       NoAppropriateType_t,                                  ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestIndParams_t,              UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_STATUS_CHANGED_NOTIFICATION_IND_FID,  NoAppropriateType_t,                                  ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationIndParams_t,   UART),
    CLIENT_TABLE_ENTRY(IND_WITH_DATA,       ZBPRO_ZCL_IAS_ACE_ARM_IND_FID,                                  NoAppropriateType_t,                                    ZBPRO_ZCL_SapIasAceArmIndParams_t,                          UART),
    CLIENT_TABLE_ENTRY(IND_WITH_DATA,       ZBPRO_ZCL_IAS_ACE_BYPASS_IND_FID,                               NoAppropriateType_t,                                    ZBPRO_ZCL_SapIasAceBypassIndParams_t,                       UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    ZBPRO_ZCL_IAS_ACE_EMERGENCY_IND_FID,                            NoAppropriateType_t,                                    ZBPRO_ZCL_SapIasAceAlarmIndParams_t,                        UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    ZBPRO_ZCL_IAS_ACE_FIRE_IND_FID,                                 NoAppropriateType_t,                                    ZBPRO_ZCL_SapIasAceAlarmIndParams_t,                        UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    ZBPRO_ZCL_IAS_ACE_PANIC_IND_FID,                                NoAppropriateType_t,                                    ZBPRO_ZCL_SapIasAceAlarmIndParams_t,                        UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    ZBPRO_ZCL_IAS_ACE_GET_ZONE_ID_MAP_IND_FID,                      NoAppropriateType_t,                                    ZBPRO_ZCL_SapIasAceGetZoneIdMapIndParams_t,                 UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    ZBPRO_ZCL_IAS_ACE_GET_ZONE_INFO_IND_FID,                        NoAppropriateType_t,                                    ZBPRO_ZCL_SapIasAceGetZoneInfoIndParams_t,                  UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    ZBPRO_ZCL_IAS_ACE_GET_PANEL_STATUS_IND_FID,                     NoAppropriateType_t,                                    ZBPRO_ZCL_SapIasAceGetPanelStatusIndParams_t,               UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    ZBPRO_ZCL_IAS_ACE_GET_BYPASSED_ZONE_LIST_IND_FID,               NoAppropriateType_t,                                    ZBPRO_ZCL_SapIasAceGetBypassedZoneListIndParams_t,          UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    ZBPRO_ZCL_IAS_ACE_GET_ZONE_STATUS_IND_FID,                      NoAppropriateType_t,                                    ZBPRO_ZCL_SapIasAceGetZoneStatusIndParams_t,                UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_TO_COLOR_REQ_FID,          ZBPRO_ZCL_ColorControlCmdMoveToColorReqDescr_t,         ZBPRO_ZCL_ColorControlCmdMoveToColorReqParams_t,        UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_COLOR_REQ_FID,             ZBPRO_ZCL_ColorControlCmdMoveColorReqDescr_t,           ZBPRO_ZCL_ColorControlCmdMoveColorReqParams_t,          UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_COLOR_CONTROL_CMD_STEP_COLOR_REQ_FID,             ZBPRO_ZCL_ColorControlCmdStepColorReqDescr_t,           ZBPRO_ZCL_ColorControlCmdStepColorReqParams_t,          UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_TO_HUE_REQ_FID,       ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqDescr_t,   ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqParams_t,  UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_HUE_REQ_FID,          ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqDescr_t,     ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqParams_t,     UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_STEP_HUE_REQ_FID,          ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqDescr_t,     ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqParams_t,     UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_TO_HUE_AND_SATURATION_REQ_FID,    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqDescr_t,    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqParams_t,   UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_COLOR_CONTROL_CMD_COLOR_LOOP_SET_REQ_FID,             ZBPRO_ZCL_ColorControlCmdColorLoopSetReqDescr_t,        ZBPRO_ZCL_ColorControlCmdColorLoopSetReqParams_t,        UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_COLOR_CONTROL_CMD_STOP_MOVE_STEP_REQ_FID,             ZBPRO_ZCL_ColorControlCmdStopMoveStepReqDescr_t,        ZBPRO_ZCL_ColorControlCmdStopMoveStepReqParams_t,        UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_COLOR_TEMPERATURE_REQ_FID,     ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqDescr_t,    ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqParams_t,    UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZCL_COLOR_CONTROL_CMD_STEP_COLOR_TEMPERATURE_REQ_FID,     ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqDescr_t,    ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqParams_t,    UART),

#   endif /* WRAPPERS_ALL */
#  endif /* MAILBOX_STACK_SIDE */
# endif /* _ZBPRO_ */
#endif

/**********************************************************************************************************************/
/* PROFILES API                                                                                                       */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_PROFILE_)
/**** ZBPRO *****************************************************************************/
# ifdef _ZBPRO_
/**** ZBPRO HA PROFILE *********************/
#  if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZHA_)
#   ifdef MAILBOX_HOST_SIDE
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZHA_EZ_MODE_FID,                      ZBPRO_ZHA_EzModeReqDescr_t,                 ZBPRO_ZHA_EzModeReqParams_t,                UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZHA_CIE_ENROLL_FID,                   ZBPRO_ZHA_CieEnrollReqDescr_t,              ZBPRO_ZHA_CieEnrollReqParams_t,             UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZHA_CIE_SET_PANEL_STATUS_REQ_FID,     ZBPRO_ZHA_CieSetPanelStatusReqDescr_t,      ZBPRO_ZHA_CieSetPanelStatusReqParams_t,     UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    ZBPRO_ZHA_CIE_ZONE_SET_BYPASS_STATE_REQ_FID,     ZBPRO_ZHA_CieZoneSetBypassStateReqDescr_t,      ZBPRO_ZHA_CieZoneSetBypassStateReqParams_t,     UART),
#   endif /* MAILBOX_HOST_SIDE */
#   ifdef MAILBOX_STACK_SIDE
#    if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_ZHA_)
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    ZBPRO_ZHA_CIE_SET_PANEL_STATUS_IND_FID,     NoAppropriateType_t,                        ZBPRO_ZHA_CieSetPanelStatusIndParams_t,     UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    ZBPRO_ZHA_CIE_ENROLL_IND_FID,               NoAppropriateType_t,                        ZBPRO_ZHA_CieEnrollIndParams_t,             UART),
#    endif /* WRAPPERS_ALL */
#   endif /* MAILBOX_STACK_SIDE */
#  endif /* _MAILBOX_WRAPPERS_ZHA_ */
# endif /* _ZBPRO_ */

/**** RF4CE *****************************************************************************/
# ifdef _RF4CE_

/**** RF4CE PROFILE MANAGER ****************/
# ifdef MAILBOX_HOST_SIDE
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_PROFILE_REQ_UNPAIR_FID,   RF4CE_UnpairReqDescr_t,             RF4CE_UnpairReqParams_t,    UART),
    CLIENT_TABLE_ENTRY(REQ_NO_PARAM,        RF4CE_PROFILE_REQ_START_FID,    RF4CE_StartReqDescr_t,              NoAppropriateType_t,        UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_PROFILE_REQ_RESET_FID,    RF4CE_ResetReqDescr_t,              RF4CE_ResetReqParams_t,     UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_PROFILE_REQ_SET_SUP_DEVICES_FID, RF4CE_SetSupportedDevicesReqDescr_t, RF4CE_SetSupportedDevicesReqParams_t, UART),
# endif /* MAILBOX_HOST_SIDE */

# ifdef MAILBOX_STACK_SIDE
#  if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_PROFILE_)
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    RF4CE_PROFILE_IND_COUNTER_EXPIRED_FID,      NoAppropriateType_t,                        RF4CE_PairingReferenceIndParams_t,          UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    RF4CE_PROFILE_IND_UNPAIR_FID,               NoAppropriateType_t,                        RF4CE_PairingReferenceIndParams_t,          UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    RF4CE_PROFILE_IND_PAIR_FID,                 NoAppropriateType_t,                        RF4CE_PairingIndParams_t,                   UART),
#  endif /* WRAPPERS_ALL */
# endif /* MAILBOX_STACK_SIDE */

/**** RF4CE PROFILE ZRC ********************/
#  if (1 == USE_RF4CE_PROFILE_ZRC)

/**** RF4CE PROFILE ZRC 1.1 ****************/
# ifdef USE_RF4CE_PROFILE_ZRC1
#  ifdef MAILBOX_HOST_SIDE
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_ZRC1_REQ_GET_FID,         RF4CE_ZRC1_GetAttributeDescr_t,         RF4CE_ZRC1_GetAttributeReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_ZRC1_REQ_SET_FID,         RF4CE_ZRC1_SetAttributeDescr_t,         RF4CE_ZRC1_SetAttributeReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_ZRC1_REQ_COMMANDDISCOVERY_FID, RF4CE_ZRC1_CommandDiscoveryReqDescr_t, RF4CE_ZRC1_CommandDiscoveryReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       RF4CE_ZRC1_CONTROL_COMMAND_PRESSED_FID, RF4CE_ZRC1_ControlCommandReqDescr_t, RF4CE_ZRC1_ControlCommandReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       RF4CE_ZRC1_CONTROL_COMMAND_RELEASED_FID, RF4CE_ZRC1_ControlCommandReqDescr_t, RF4CE_ZRC1_ControlCommandReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_NO_PARAM,        RF4CE_ZRC1_REQ_CONTROLLER_BIND_FID,    RF4CE_ZRC1_BindReqDescr_t,              NoAppropriateType_t,        UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       RF4CE_ZRC1_VENDORSPECIFIC_REQ_FID, RF4CE_ZRC1_VendorSpecificReqDescr_t,    RF4CE_ZRC1_VendorSpecificReqParams_t,                     UART),
#    if defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
        CLIENT_TABLE_ENTRY(REQ_NO_PARAM,        RF4CE_ZRC1_REQ_TARGET_BIND_FID,    RF4CE_ZRC1_BindReqDescr_t,              NoAppropriateType_t,        UART),
#    endif /* RF4CE_TARGET */
#  endif /* MAILBOX_HOST_SIDE */
#  ifdef MAILBOX_STACK_SIDE
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_PROFILE_)
#    if defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
        CLIENT_TABLE_ENTRY(IND_WITH_DATA,    RF4CE_ZRC1_IND_CONTROLCOMMAND_FID,    NoAppropriateType_t,                RF4CE_ZRC1_ControlCommandIndParams_t, UART),
#    endif /* RF4CE_TARGET */
     CLIENT_TABLE_ENTRY(IND_WITH_DATA,       RF4CE_ZRC1_VENDORSPECIFIC_IND_FID, NoAppropriateType_t,    RF4CE_ZRC1_VendorSpecificIndParams_t,                     UART),
#   endif /* WRAPPERS_ALL */
#  endif /* MAILBOX_STACK_SIDE */
# endif /* USE_RF4CE_PROFILE_ZRC1 */

/**** RF4CE PROFILE GDP 2.0 & ZRC 2.0 ******/
# ifdef USE_RF4CE_PROFILE_ZRC2
#  ifdef MAILBOX_HOST_SIDE
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       RF4CE_ZRC2_REQ_GET_FID,         RF4CE_ZRC2_GetAttributesReqDescr_t,     RF4CE_ZRC2_GetAttributesReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       RF4CE_ZRC2_REQ_SET_FID,                     RF4CE_ZRC2_SetAttributesReqDescr_t,         RF4CE_ZRC2_SetAttributesReqParams_t,        UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_ZRC2_KEY_EXCHANGE_FID,                RF4CE_ZRC2_KeyExchangeReqDescr_t,           RF4CE_ZRC2_KeyExchangeReqParams_t,          UART),
    CLIENT_TABLE_ENTRY(REQ_NO_PARAM,        RF4CE_ZRC2_BIND_FID,            RF4CE_ZRC2_BindReqDescr_t,              NoAppropriateType_t,                UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_ZRC2_PROXY_BIND_FID,      RF4CE_ZRC2_ProxyBindReqDescr_t,         RF4CE_ZRC2_ProxyBindReqParams_t,    UART),
    CLIENT_TABLE_ENTRY(REQ_NO_PARAM,        RF4CE_ZRC2_ENABLE_BINDING_FID,  RF4CE_ZRC2_BindingReqDescr_t,           NoAppropriateType_t,                UART),
    CLIENT_TABLE_ENTRY(REQ_NO_PARAM,        RF4CE_ZRC2_DISABLE_BINDING_FID, RF4CE_ZRC2_BindingReqDescr_t,           NoAppropriateType_t,                UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,      RF4CE_ZRC2_SET_PUSH_BUTTON_STIMULUS_FID,    RF4CE_ZRC2_ButtonBindingReqDescr_t,         RF4CE_ZRC2_ButtonBindingReqParams_t,        UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,      RF4CE_ZRC2_CLEAR_PUSH_BUTTON_STIMULUS_FID,  RF4CE_ZRC2_ButtonBindingReqDescr_t,         RF4CE_ZRC2_ButtonBindingReqParams_t,        UART),
    CLIENT_TABLE_ENTRY(RESP_WITHOUT_DATA,   RF4CE_ZRC2_CHECK_VALIDATION_RESP_FID, RF4CE_ZRC2_CheckValidationRespDescr_t, RF4CE_ZRC2_CheckValidationRespParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       RF4CE_ZRC2_CONTROL_COMMAND_PRESS_REQ_FID, RF4CE_ZRC2_ControlCommandReqDescr_t, RF4CE_ZRC2_ControlCommandReqParams_t,  UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       RF4CE_ZRC2_CONTROL_COMMAND_RELEASE_REQ_FID, RF4CE_ZRC2_ControlCommandReqDescr_t, RF4CE_ZRC2_ControlCommandReqParams_t,  UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_GDP2_REQ_SET_POLL_CONSTRAINTS_FID,    RF4CE_GDP2_SetPollConstraintsReqDescr_t,    RF4CE_GDP2_SetPollConstraintsReqParams_t,   UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_GDP2_REQ_POLL_NEGOTIATION_FID,        RF4CE_GDP2_PollNegotiationReqDescr_t,       RF4CE_GDP2_PollNegotiationReqParams_t,      UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_GDP2_REQ_POLL_CLIENT_USER_EVENT_FID,  RF4CE_GDP2_PollClientUserEventReqDescr_t,   RF4CE_GDP2_PollClientUserEventReqParams_t,  UART),
    CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       RF4CE_GDP2_REQ_CLIENT_NOTIFICATION_FID,     RF4CE_GDP2_ClientNotificationReqDescr_t,    RF4CE_GDP2_ClientNotificationReqParams_t,   UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,      RF4CE_GDP2_REQ_IDENTIFY_CAP_ANNOUNCE_FID,   RF4CE_GDP2_IdentifyCapAnnounceReqDescr_t,   RF4CE_GDP2_IdentifyCapAnnounceReqParams_t,  UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,      RF4CE_GDP2_REQ_IDENTIFY_FID,                RF4CE_GDP2_IdentifyReqDescr_t,              RF4CE_GDP2_IdentifyReqParams_t,             UART),
#  endif /* MAILBOX_HOST_SIDE */

#  ifdef MAILBOX_STACK_SIDE
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_PROFILE_)
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    RF4CE_ZRC2_START_VALIDATION_FID, NoAppropriateType_t,                   RF4CE_ZRC2_CheckValidationIndParams_t, UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    RF4CE_ZRC2_CHECK_VALIDATION_IND_FID, NoAppropriateType_t,               RF4CE_ZRC2_CheckValidationIndParams_t, UART),
    CLIENT_TABLE_ENTRY(IND_WITH_DATA,       RF4CE_ZRC2_CONTROL_COMMAND_IND_FID, NoAppropriateType_t,                RF4CE_ZRC2_ControlCommandIndParams_t, UART),
    CLIENT_TABLE_ENTRY(IND_WITH_DATA,       RF4CE_GDP2_IND_POLL_NEGOTIATION_FID,        NoAppropriateType_t,                        RF4CE_GDP2_PollNegotiationIndParams_t,      UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    RF4CE_GDP2_IND_HEARTBEAT_FID,               NoAppropriateType_t,                        RF4CE_GDP2_HeartbeatIndParams_t,            UART),
    CLIENT_TABLE_ENTRY(IND_WITH_DATA,       RF4CE_GDP2_IND_CLIENT_NOTIFICATION_FID,     NoAppropriateType_t,                        RF4CE_GDP2_ClientNotificationIndParams_t,   UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,      RF4CE_GDP2_IND_IDENTIFY_CAP_ANNOUNCE_FID,   NoAppropriateType_t,                        RF4CE_GDP2_IdentifyCapAnnounceIndParams_t,  UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,      RF4CE_GDP2_IND_IDENTIFY_FID,                NoAppropriateType_t,                        RF4CE_GDP2_IdentifyIndParams_t,             UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA_RESP, RF4CE_ZRC2_GET_SHARED_SECRET_IND_FID, RF4CE_ZRC2_GetSharedSecretIndDescr_t, RF4CE_ZRC2_GetSharedSecretIndParams_t, UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,      RF4CE_ZRC2_PAIR_IND_FID,                    NoAppropriateType_t,                        RF4CE_PairingIndParams_t,                   UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,      RF4CE_ZRC2_BINDING_FINISHED_IND_FID,        NoAppropriateType_t,                        RF4CE_ZRC2_BindingFinishedNtfyIndParams_t,  UART),
    CLIENT_TABLE_ENTRY(IND_WITH_DATA,         RF4CE_ZRC2_IND_GET_RESP_FID,                NoAppropriateType_t,                        RF4CE_ZRC2_SetAttributesReqParams_t,        UART),
    CLIENT_TABLE_ENTRY(IND_WITH_DATA,         RF4CE_ZRC2_IND_PUSH_REQ_FID,                NoAppropriateType_t,                        RF4CE_ZRC2_SetAttributesReqParams_t,        UART),
#   endif /* WRAPPERS_ALL */
#  endif /* MAILBOX_STACK_SIDE */
# endif /* USE_RF4CE_PROFILE_ZRC2 */
/**********************************************************************************************************************/
/* The new api to set the power filter key                                                                            */
/**********************************************************************************************************************/
#  ifdef MAILBOX_HOST_SIDE
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_ZRC_SET_WAKEUP_ACTION_CODE_FID, RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t,  RF4CE_ZRC_SetWakeUpActionCodeReqParams_t,  UART),
    CLIENT_TABLE_ENTRY(REQ_NO_PARAM,        RF4CE_ZRC_GET_WAKEUP_ACTION_CODE_FID, RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t,  NoAppropriateType_t,  UART),
#  endif
# endif /* USE_RF4CE_PROFILE_ZRC */

# if (1 == USE_RF4CE_PROFILE_MSO)
#  ifdef MAILBOX_HOST_SIDE
     CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_MSO_GET_PROFILE_ATTRIBUTE_FID,         RF4CE_MSO_GetProfileAttributeReqDescr_t,         RF4CE_MSO_GetProfileAttributeReqParams_t, UART),
     CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_MSO_SET_PROFILE_ATTRIBUTE_FID,         RF4CE_MSO_SetProfileAttributeReqDescr_t,         RF4CE_MSO_SetProfileAttributeReqParams_t, UART),
     CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_MSO_GET_RIB_ATTRIBUTE_FID,             RF4CE_MSO_GetRIBAttributeReqDescr_t,             RF4CE_MSO_GetRIBAttributeReqParams_t,     UART),
     CLIENT_TABLE_ENTRY(REQ_WITH_DATA,       RF4CE_MSO_SET_RIB_ATTRIBUTE_FID,             RF4CE_MSO_SetRIBAttributeReqDescr_t,             RF4CE_MSO_SetRIBAttributeReqParams_t,     UART),
#    if defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
     CLIENT_TABLE_ENTRY(RESP_WITHOUT_DATA,   RF4CE_MSO_VALIDATE_RESP_FID,                 RF4CE_MSO_WatchDogKickOrValidateReqDescr_t,      RF4CE_MSO_WatchDogKickOrValidateReqParams_t, UART),
#    endif /* RF4CE_TARGET */
#    if defined(RF4CE_CONTROLLER) || defined(MAILBOX_UNIT_TEST)
      CLIENT_TABLE_ENTRY(REQ_NO_PARAM,        RF4CE_MSO_BIND_FID,                         RF4CE_MSO_BindReqDescr_t,                        NoAppropriateType_t,                      UART),
      CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_MSO_USER_CONTROL_PRESSED_FID,         RF4CE_MSO_UserControlReqDescr_t,                 RF4CE_MSO_UserControlReqParams_t,         UART),
      CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA,    RF4CE_MSO_USER_CONTROL_RELEASED_FID,        RF4CE_MSO_UserControlReqDescr_t,                 RF4CE_MSO_UserControlReqParams_t,         UART),
#    endif /* RF4CE_CONTROLLER */
#  endif /* MAILBOX_HOST_SIDE */
#  ifdef MAILBOX_STACK_SIDE
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_PROFILE_)
#    if defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
      CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,    RF4CE_MSO_CHECK_VALIDATION_IND_FID,         NoAppropriateType_t,                             RF4CE_MSO_CheckValidationIndParams_t,     UART),
      CLIENT_TABLE_ENTRY(IND_WITH_DATA,       RF4CE_MSO_USER_CONTROL_IND_FID,             NoAppropriateType_t,                             RF4CE_MSO_UserControlIndParams_t,         UART),
#     endif /* RF4CE_TARGET */
     CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,     RF4CE_MSO_START_VALIDATION_IND_FID,         NoAppropriateType_t,                             RF4CE_PairingReferenceIndParams_t,        UART),
#    endif /* WRAPPERS_ALL */
#   endif /* MAILBOX_STACK_SIDE */
#  endif /* USE_RF4CE_PROFILE_MSO */
# endif /* _RF4CE_ */
#endif /* _MAILBOX_WRAPPERS_RF4CE_PROFILE_ */

/**********************************************************************************************************************/
/* NVM API                                                                                                            */
/**********************************************************************************************************************/
#ifdef MAILBOX_STACK_SIDE
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA_RESP,   NVM_READ_FILE_FID,   NVM_ReadFileIndDescr_t,  NVM_ReadFileIndParams_t,  UART),
    CLIENT_TABLE_ENTRY(IND_WITH_DATA_RESP,      NVM_WRITE_FILE_FID,  NVM_WriteFileIndDescr_t, NVM_WriteFileIndParams_t, UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA_RESP,   NVM_OPEN_FILE_FID,   NVM_OpenFileIndDescr_t,  NVM_OpenFileIndParams_t,  UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA_RESP,   NVM_CLOSE_FILE_FID,  NVM_CloseFileIndDescr_t, NVM_CloseFileIndParams_t, UART),
#endif /* MAILBOX_STACK_SIDE */

/**********************************************************************************************************************/
/* SYS events                                                                                                         */
/**********************************************************************************************************************/
#ifdef MAILBOX_HOST_SIDE
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,        SYS_EVENT_SUBSCRIBE_FID, NoAppropriateType_t,   SYS_EventHandlerMailParams_t, UART),
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,        SYS_EVENT_RAISE_FID,    NoAppropriateType_t,    SYS_EventNotifyParams_t, UART),
#endif /* MAILBOX_HOST_SIDE */
#ifdef MAILBOX_STACK_SIDE
    CLIENT_TABLE_ENTRY(IND_WITHOUT_DATA,        SYS_EVENT_NOTIFY_FID, NoAppropriateType_t,      SYS_EventNotifyParams_t, UART),
#endif /* MAILBOX_STACK_SIDE */

/**********************************************************************************************************************/
/* Phy API                                                                                                       */
/**********************************************************************************************************************/
#ifdef MAILBOX_HOST_SIDE
#  if defined(_PHY_TEST_HOST_INTERFACE_)
    CLIENT_TABLE_ENTRY(REQ_NO_PARAM,     RF4CE_CTRL_TEST_GET_CAPS_FID,              Phy_Test_Get_Caps_ReqDescr_t,   NoAppropriateType_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, RF4CE_CTRL_TEST_SET_CHANNEL_FID,           Phy_Test_Set_Channel_ReqDescr_t,   Phy_Test_Set_Channel_ReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, RF4CE_CTRL_TEST_CONTINUOUS_WAVE_START_FID, Phy_Test_Continuous_Wave_Start_ReqDescr_t,   Phy_Test_Continuous_Wave_Start_ReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_NO_PARAM,     RF4CE_CTRL_TEST_CONTINUOUS_WAVE_STOP_FID,  Phy_Test_Continuous_Wave_Stop_ReqDescr_t,   NoAppropriateType_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, RF4CE_CTRL_TEST_TRANSMIT_START_FID,        Phy_Test_Transmit_Start_ReqDescr_t,   Phy_Test_Transmit_Start_ReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_NO_PARAM,     RF4CE_CTRL_TEST_TRANSMIT_STOP_FID,         Phy_Test_Transmit_Stop_ReqDescr_t,   NoAppropriateType_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, RF4CE_CTRL_TEST_RECEIVE_START_FID,         Phy_Test_Receive_Start_ReqDescr_t,   Phy_Test_Receive_Start_ReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_NO_PARAM,     RF4CE_CTRL_TEST_RECEIVE_STOP_FID,          Phy_Test_Receive_Stop_ReqDescr_t,   NoAppropriateType_t, UART),
    CLIENT_TABLE_ENTRY(REQ_NO_PARAM,     RF4CE_CTRL_TEST_ECHO_START_FID,            Phy_Test_Echo_Start_ReqDescr_t,   NoAppropriateType_t, UART),
    CLIENT_TABLE_ENTRY(REQ_NO_PARAM,     RF4CE_CTRL_TEST_ECHO_STOP_FID,             Phy_Test_Echo_Stop_ReqDescr_t,   NoAppropriateType_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, RF4CE_CTRL_TEST_ENERGY_DETECT_SCAN_FID,    Phy_Test_Energy_Detect_Scan_ReqDescr_t,   Phy_Test_Energy_Detect_Scan_ReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_NO_PARAM,     RF4CE_CTRL_TEST_GET_STATS_FID,             Phy_Test_Get_Stats_ReqDescr_t,   NoAppropriateType_t, UART),
    CLIENT_TABLE_ENTRY(REQ_NO_PARAM,     RF4CE_CTRL_TEST_RESET_STATS_FID,           Phy_Test_Reset_Stats_ReqDescr_t,   NoAppropriateType_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, RF4CE_CTRL_TEST_SET_TX_POWER_FID,          Phy_Test_Set_TX_Power_ReqDescr_t,   Phy_Test_Set_TX_Power_ReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, RF4CE_CTRL_TEST_SELECT_ANTENNA_FID,        Phy_Test_Select_Antenna_ReqDescr_t,   Phy_Test_Select_Antenna_ReqParams_t, UART),
    CLIENT_TABLE_ENTRY(REQ_NO_PARAM,     RF4CE_CTRL_GET_DIAGNOSTICS_CAPS_FID,       RF4CE_Diag_Caps_ReqDescr_t,   NoAppropriateType_t, UART),
    CLIENT_TABLE_ENTRY(REQ_WITHOUT_DATA, RF4CE_CTRL_GET_DIAGNOSTIC_FID,             RF4CE_Diag_ReqDescr_t,        RF4CE_Diag_ReqParams_t, UART),
#  endif
#endif /* MAILBOX_HOST_SIDE */
};

/************************* IMPLEMENTATION ***********************************************/
/*************************************************************************************//**
    \brief Gets information related to the request id from the client table.
    \param[in] id - request number.

    \return request information.
 ****************************************************************************************/
const MailClientParametersTableEntry_t *mailClientTableGetAppropriateEntry(uint16_t id)
{
    const MailClientParametersTableEntry_t *entry = clientTable;
    const MailClientParametersTableEntry_t *const tableEnd = clientTable + ARRAY_SIZE(clientTable);
    while (id != entry->id)
    {
        SYS_DbgAssertComplex(tableEnd != entry, MAILCLIENTTABLE_GETREQUESTINFORMATION_0);
        ++entry;
    }
    return entry;
}
