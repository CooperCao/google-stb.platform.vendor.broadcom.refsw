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
* FILENAME: $Workfile: trunk/stack/common/Mailbox/src/bbMailServerTable.c $
*
* DESCRIPTION:
*         the mailbox server table implementation.
*
* $Revision: 3955 $
* $Date: 2014-10-08 12:45:05Z $
*
*****************************************************************************************/
/************************* INCLUDES *****************************************************/
#include "bbMailAPI.h"
#include "bbMailServerTable.h"

/************************* DEFINITIONS **************************************************/
#define REQ_CONF
#define REQ_CONF_WITH_DATA
#define REQ_CONF_WITH_NAMED_DATA
#define REQ_NO_PARAM_CONF
#define REQ_NO_PARAM_CONF_WITH_DATA
#define RESP
#define IND_NO_RESP
#define IND_RESP
#define IND_RESP_WITH_DATA
#define IND_NO_PARAM_RESP
#define IND_NO_PARAM_RESP_WITH_DATA
#define SERVER_TABLE_ENTRY(entryType, ...) SERVER_TABLE_ENTRY_##entryType(__VA_ARGS__)

#define SERVER_TABLE_ENTRY_REQ_CONF(fId, reqDescrType, confParamsType, fPointer)    \
    {                                                                               \
        .id                     = fId,                                              \
        .function               = (MailPublicFunction_t)fPointer,                   \
        .reqLength              = sizeof(reqDescrType),                             \
        .reqParametersOffset    = offsetof(reqDescrType, params),                   \
        .callbackOffset         = offsetof(reqDescrType, callback),                 \
        .confParametersLength   = sizeof(confParamsType),                           \
        .confDataPointerOffset  = MAIL_INVALID_PAYLOAD_OFFSET,                      \
    }

#define SERVER_TABLE_ENTRY_REQ_CONF_WITH_DATA(fId, reqDescrType, confParamsType, fPointer) \
    {                                                                               \
        .id                     = fId,                                              \
        .function               = (MailPublicFunction_t)fPointer,                   \
        .reqLength              = sizeof(reqDescrType),                             \
        .reqParametersOffset    = offsetof(reqDescrType, params),                   \
        .callbackOffset         = offsetof(reqDescrType, callback),                 \
        .confParametersLength   = sizeof(confParamsType),                           \
        .confDataPointerOffset  = offsetof(confParamsType, payload),                \
    }

#define SERVER_TABLE_ENTRY_REQ_CONF_WITH_NAMED_DATA(fId, reqDescrType, confParamsType, fPointer, payloadName) \
    {                                                                               \
        .id                     = fId,                                              \
        .function               = (MailPublicFunction_t)fPointer,                   \
        .reqLength              = sizeof(reqDescrType),                             \
        .reqParametersOffset    = offsetof(reqDescrType, params),                   \
        .callbackOffset         = offsetof(reqDescrType, callback),                 \
        .confParametersLength   = sizeof(confParamsType),                           \
        .confDataPointerOffset  = offsetof(confParamsType, payloadName),            \
    }

#define SERVER_TABLE_ENTRY_REQ_NO_PARAM_CONF(fId, reqDescrType, confParamsType, fPointer) \
    {                                                                               \
        .id                     = fId,                                              \
        .function               = (MailPublicFunction_t)fPointer,                   \
        .reqLength              = sizeof(reqDescrType),                             \
        .reqParametersOffset    = sizeof(reqDescrType),                             \
        .callbackOffset         = offsetof(reqDescrType, callback),                 \
        .confParametersLength   = sizeof(confParamsType),                           \
        .confDataPointerOffset  = MAIL_INVALID_PAYLOAD_OFFSET,                      \
    }

#define SERVER_TABLE_ENTRY_REQ_NO_PARAM_CONF_WITH_DATA(fId, reqDescrType, confParamsType, fPointer) \
    {                                                                               \
        .id                     = fId,                                              \
        .function               = (MailPublicFunction_t)fPointer,                   \
        .reqLength              = sizeof(reqDescrType),                             \
        .reqParametersOffset    = sizeof(reqDescrType),                             \
        .callbackOffset         = offsetof(reqDescrType, callback),                 \
        .confParametersLength   = sizeof(confParamsType),                           \
        .confDataPointerOffset  = offsetof(confParamsType, payload),                \
    }

#define SERVER_TABLE_ENTRY_RESP(fId, reqDescrType, confParamsType, fPointer)        \
    {                                                                               \
        .id                     = fId,                                              \
        .function               = (MailPublicFunction_t)fPointer,                   \
        .reqLength              = sizeof(reqDescrType),                             \
        .reqParametersOffset    = offsetof(reqDescrType, params),                   \
        .callbackOffset         = sizeof(reqDescrType),                             \
        .confParametersLength   = 0U,                                               \
        .confDataPointerOffset  = MAIL_INVALID_PAYLOAD_OFFSET,                      \
    }

#define SERVER_TABLE_ENTRY_IND_NO_RESP(fId, reqDescrType, confParamsType, fPointer) \
    {                                                                               \
        .id                     = fId,                                              \
        .function               = (MailPublicFunction_t)fPointer,                   \
        .reqLength              = sizeof(reqDescrType),                             \
        .reqParametersOffset    = 0U,                                               \
        .callbackOffset         = sizeof(reqDescrType),                             \
        .confParametersLength   = 0U,                                               \
        .confDataPointerOffset  = MAIL_INVALID_PAYLOAD_OFFSET,                      \
    }

#define SERVER_TABLE_ENTRY_IND_RESP                     SERVER_TABLE_ENTRY_REQ_CONF
#define SERVER_TABLE_ENTRY_IND_RESP_WITH_DATA           SERVER_TABLE_ENTRY_REQ_CONF_WITH_DATA
#define SERVER_TABLE_ENTRY_IND_NO_PARAM_RESP            SERVER_TABLE_ENTRY_REQ_NO_PARAM_CONF
#define SERVER_TABLE_ENTRY_IND_NO_PARAM_RESP_WITH_DATA  SERVER_TABLE_ENTRY_REQ_NO_PARAM_CONF_WITH_DATA

typedef uint8_t NoAppropriateType_t;
static void NoAppropriateFunction(void)
{
    SYS_DbgHalt(MAILSERVER_UNEXPECTED_CALL);
};

static const MailServerParametersTableEntry_t serverTable[] =
{
/**********************************************************************************************************************/
/* Mailbox service function                                                                                           */
/**********************************************************************************************************************/
    SERVER_TABLE_ENTRY(REQ_NO_PARAM_CONF,   MAIL_ACK_FID,           MailAckDescr_t,                 MailAckConfParams_t,            NoAppropriateFunction),

/**********************************************************************************************************************/
/* Only for unit tests                                                                                                */
/**********************************************************************************************************************/
#ifdef MAILBOX_UNIT_TEST
    SERVER_TABLE_ENTRY(REQ_CONF,            MAIL_F1_FID,            MailUnitTest_f1Descr_t,         MailUnitTest_f1ConfParams_t,    MailUnitTest_f1),
    SERVER_TABLE_ENTRY(REQ_NO_PARAM_CONF,   MAIL_F2_FID,            MailUnitTest_f2Descr_t,         MailUnitTest_f2ConfParams_t,    MailUnitTest_f2),
    SERVER_TABLE_ENTRY(RESP,                MAIL_F3_FID,            MailUnitTest_f3Descr_t,         NoAppropriateType_t,            MailUnitTest_f3),
    SERVER_TABLE_ENTRY(REQ_CONF,            MAIL_F4_FID,            MailUnitTest_f4Descr_t,         MailUnitTest_f4ConfParams_t,    MailUnitTest_f4),
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_DATA,  MAIL_F5_FID,            MailUnitTest_f5Descr_t,         MailUnitTest_f5ConfParams_t,    MailUnitTest_f5),
#endif /* MAILBOX_UNIT_TEST */

/**********************************************************************************************************************/
/* Test engine API                                                                                                    */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_TEST_ENGINE_)
# ifdef MAILBOX_STACK_SIDE
    SERVER_TABLE_ENTRY(REQ_NO_PARAM_CONF,   TE_PING_FID,            TE_PingCommandReqDescr_t,       TE_PingCommandConfParams_t,     Mail_TestEnginePing),
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_DATA,  TE_ECHO_FID,            TE_EchoCommandReqDescr_t,       TE_EchoCommandConfParams_t,     Mail_TestEngineEcho),
    SERVER_TABLE_ENTRY(RESP,                TE_RESET_FID,           TE_ResetCommandReqDescr_t,      NoAppropriateType_t,            Mail_TestEngineReset),
    SERVER_TABLE_ENTRY(REQ_CONF,            TE_ECHO_DELAY_FID,      TE_SetEchoDelayCommandReqDescr_t, TE_SetEchoDelayCommandConfParams_t, Mail_SetEchoDelay),
    SERVER_TABLE_ENTRY(RESP,                TE_HOST_TO_UART1_FID,   TE_Host2Uart1ReqDescr_t,        NoAppropriateType_t,       MailUartRxInterruptHandler),
# endif /* MAILBOX_STACK_SIDE */

# ifdef MAILBOX_HOST_SIDE
    SERVER_TABLE_ENTRY(IND_NO_RESP,         TE_HELLO_FID,           TE_HelloCommandIndParams_t,     NoAppropriateType_t,            Mail_TestEngineHelloInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         TE_ASSERT_LOGID_FID,    TE_AssertLogIdCommandIndParams_t, NoAppropriateType_t,          Mail_TestEngineAssertLogIdInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         TE_UART1_TO_HOST_FID,   TE_Uart1ToHostReqParams_t,       NoAppropriateType_t,           Mail_Uart1ToHostInd),
# endif /* MAILBOX_HOST_SIDE */
#endif /* _MAILBOX_WRAPPERS_TEST_ENGINE_ */

/**********************************************************************************************************************/
/* Profiling Engine API                                                                                           */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_PROFILING_ENGINE_)
# ifdef MAILBOX_STACK_SIDE
    SERVER_TABLE_ENTRY(RESP,                PE_RESET_FID,           SYS_DbgPeResetReqDescr_t,       NoAppropriateType_t,            SYS_DbgPeResetReq),
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_DATA,  PE_GETDATA_FID,         SYS_DbgPeGetDataReqDescr_t,     SYS_DbgPeGetDataConfParams_t,   SYS_DbgPeGetDataReq),
# endif /* MAILBOX_STACK_SIDE */
#endif /* _MAILBOX_WRAPPERS_PROFILING_ENGINE_ */

/**********************************************************************************************************************/
/* MAC API                                                                                                            */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_MAC_)
# ifdef MAILBOX_STACK_SIDE
#  ifdef _MAC_BAN_TABLE_
    SERVER_TABLE_ENTRY(REQ_CONF,           MAC_REQ_BAN_SET_DEFAULT_ACTION_FID,       MAC_BanTableSetDefaultActionReqDescr_t, MAC_BanTableConfParams_t,           MAC_BanTableSetDefaultAction),
    SERVER_TABLE_ENTRY(REQ_CONF,           MAC_REQ_BAN_LINK_FID,                     MAC_BanTableAddLinkReqDescr_t,          MAC_BanTableConfParams_t,           MAC_BanTableAddLink),
#  endif /* _MAC_BAN_TABLE_ */
# endif /* MAILBOX_STACK_SIDE */
/**** ZBPRO *****************************************************************************/
# ifdef _ZBPRO_
#  ifdef MAILBOX_STACK_SIDE
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_DATA, ZBPRO_MAC_REQ_GET_FID,                    MAC_GetReqDescr_t,                      MAC_GetConfParams_t,                ZBPRO_MAC_GetReq),
    SERVER_TABLE_ENTRY(REQ_CONF,           ZBPRO_MAC_REQ_SET_FID,                    MAC_SetReqDescr_t,                      MAC_SetConfParams_t,                ZBPRO_MAC_SetReq),
    SERVER_TABLE_ENTRY(REQ_CONF,           ZBPRO_MAC_REQ_DATA_FID,                   MAC_DataReqDescr_t,                     MAC_DataConfParams_t,               ZBPRO_MAC_DataReq),
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_MAC_)
    SERVER_TABLE_ENTRY(REQ_CONF,           ZBPRO_MAC_REQ_PURGE_FID,                  MAC_PurgeReqDescr_t,                    MAC_PurgeConfParams_t,              ZBPRO_MAC_PurgeReq),
    SERVER_TABLE_ENTRY(REQ_CONF,           ZBPRO_MAC_REQ_ASSOCIATE_FID,              MAC_AssociateReqDescr_t,                MAC_AssociateConfParams_t,          ZBPRO_MAC_AssociateReq),
    SERVER_TABLE_ENTRY(REQ_CONF,           ZBPRO_MAC_REQ_RESET_FID,                  MAC_ResetReqDescr_t,                    MAC_ResetConfParams_t,              ZBPRO_MAC_ResetReq),
    SERVER_TABLE_ENTRY(REQ_CONF,           ZBPRO_MAC_REQ_RX_ENABLE_FID,              MAC_RxEnableReqDescr_t,                 MAC_RxEnableConfParams_t,           ZBPRO_MAC_RxEnableReq),
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_DATA, ZBPRO_MAC_REQ_SCAN_FID,                   MAC_ScanReqDescr_t,                     MAC_ScanConfParams_t,               ZBPRO_MAC_ScanReq),
    SERVER_TABLE_ENTRY(REQ_CONF,           ZBPRO_MAC_REQ_START_FID,                  MAC_StartReqDescr_t,                    MAC_StartConfParams_t,              ZBPRO_MAC_StartReq),
    SERVER_TABLE_ENTRY(REQ_CONF,           ZBPRO_MAC_RESP_ASSOCIATE_FID,             MAC_AssociateRespDescr_t,               MAC_CommStatusAssociateIndParams_t, ZBPRO_MAC_AssociateResp),
    SERVER_TABLE_ENTRY(REQ_CONF,           ZBPRO_MAC_RESP_ORPHAN_FID,                MAC_OrphanRespDescr_t,                  MAC_CommStatusOrphanIndParams_t,    ZBPRO_MAC_OrphanResp),
#   endif /* WRAPPERS_ALL */
#  endif /* MAILBOX_STACK_SIDE */

# ifdef MAILBOX_HOST_SIDE
#  if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_MAC_)
    SERVER_TABLE_ENTRY(IND_NO_RESP,        ZBPRO_MAC_IND_DATA_FID,                   MAC_DataIndParams_t,                    NoAppropriateType_t,                ZBPRO_MAC_DataInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,        ZBPRO_MAC_IND_ASSOCIATE_FID,              MAC_AssociateIndParams_t,               NoAppropriateType_t,                ZBPRO_MAC_AssociateInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,        ZBPRO_MAC_IND_BEACON_NOTIFY_FID,          MAC_BeaconNotifyIndParams_t,            NoAppropriateType_t,                ZBPRO_MAC_BeaconNotifyInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,        ZBPRO_MAC_IND_COMM_STATUS_FID,            MAC_CommStatusIndParams_t,              NoAppropriateType_t,                ZBPRO_MAC_CommStatusInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,        ZBPRO_MAC_IND_ORPHAN_FID,                 MAC_OrphanIndParams_t,                  NoAppropriateType_t,                ZBPRO_MAC_OrphanInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,        ZBPRO_MAC_IND_POLL_FID,                   MAC_PollIndParams_t,                    NoAppropriateType_t,                ZBPRO_MAC_PollInd),
#  endif /* WRAPPERS_ALL */
# endif /* MAILBOX_HOST_SIDE */
# endif /* _ZBPRO_ */

/**** RF4CE *****************************************************************************/
# ifdef _RF4CE_
#  ifdef MAILBOX_STACK_SIDE
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_DATA, RF4CE_MAC_REQ_GET_FID,                    MAC_GetReqDescr_t,                      MAC_GetConfParams_t,                RF4CE_MAC_GetReq),
    SERVER_TABLE_ENTRY(REQ_CONF,           RF4CE_MAC_REQ_SET_FID,                    MAC_SetReqDescr_t,                      MAC_SetConfParams_t,                RF4CE_MAC_SetReq),
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_MAC_)
    SERVER_TABLE_ENTRY(REQ_CONF,           RF4CE_MAC_REQ_DATA_FID,                   MAC_DataReqDescr_t,                     MAC_DataConfParams_t,               RF4CE_MAC_DataReq),
    SERVER_TABLE_ENTRY(REQ_CONF,           RF4CE_MAC_REQ_RESET_FID,                  MAC_ResetReqDescr_t,                    MAC_ResetConfParams_t,              RF4CE_MAC_ResetReq),
    SERVER_TABLE_ENTRY(REQ_CONF,           RF4CE_MAC_REQ_RX_ENABLE_FID,              MAC_RxEnableReqDescr_t,                 MAC_RxEnableConfParams_t,           RF4CE_MAC_RxEnableReq),
    SERVER_TABLE_ENTRY(REQ_CONF,           RF4CE_MAC_REQ_START_FID,                  MAC_StartReqDescr_t,                    MAC_StartConfParams_t,              RF4CE_MAC_StartReq),
#  if defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_DATA, RF4CE_MAC_REQ_SCAN_FID,                   MAC_ScanReqDescr_t,                     MAC_ScanConfParams_t,               RF4CE_MAC_ScanReq),
#    endif /* RF4CE_TARGET */
#   endif /* WRAPPERS_ALL */
# endif /* MAILBOX_STACK_SIDE */

# ifdef MAILBOX_HOST_SIDE
#  if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_MAC_)
    SERVER_TABLE_ENTRY(IND_NO_RESP,        RF4CE_MAC_IND_DATA_FID,                   MAC_DataIndParams_t,                    NoAppropriateType_t,                RF4CE_MAC_DataInd),
#   if defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
    SERVER_TABLE_ENTRY(IND_NO_RESP,        RF4CE_MAC_IND_BEACON_NOTIFY_FID,          MAC_BeaconNotifyIndParams_t,            NoAppropriateType_t,                RF4CE_MAC_BeaconNotifyInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,        RF4CE_MAC_IND_COMM_STATUS_FID,            MAC_CommStatusIndParams_t,              NoAppropriateType_t,                RF4CE_MAC_CommStatusInd),
#    endif /* RF4CE_TARGET */
#   endif /* WRAPPERS_ALL */
#  endif /* MAILBOX_HOST_SIDE */

# endif /* _RF4CE_ */
#endif /* _MAILBOX_WRAPPERS_MAC_ */

/**********************************************************************************************************************/
/* NWK API                                                                                                            */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_NWK_)
/**** RF4CE *****************************************************************************/
# if defined(_RF4CE_)
# ifdef MAILBOX_STACK_SIDE
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_NWK_REQ_RESET_FID,        RF4CE_NWK_ResetReqDescr_t,          RF4CE_NWK_ResetConfParams_t,    RF4CE_NWK_ResetReq),
    SERVER_TABLE_ENTRY(REQ_NO_PARAM_CONF,   RF4CE_NWK_REQ_START_FID,        RF4CE_NWK_StartReqDescr_t,          RF4CE_NWK_StartConfParams_t,    RF4CE_NWK_StartReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_NWK_REQ_DATA_FID,         RF4CE_NWK_DataReqDescr_t,           RF4CE_NWK_DataConfParams_t,     RF4CE_NWK_DataReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_NWK_REQ_GET_FID,          RF4CE_NWK_GetReqDescr_t,            RF4CE_NWK_GetConfParams_t,      RF4CE_NWK_GetReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_NWK_REQ_SET_FID,          RF4CE_NWK_SetReqDescr_t,            RF4CE_NWK_SetConfParams_t,      RF4CE_NWK_SetReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_NWK_REQ_RXENABLE_FID,     RF4CE_NWK_RXEnableReqDescr_t,       RF4CE_NWK_RXEnableConfParams_t, RF4CE_NWK_RXEnableReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_NWK_REQ_UPDATEKEY_FID,    RF4CE_NWK_UpdateKeyReqDescr_t,      RF4CE_NWK_UpdateKeyConfParams_t, RF4CE_NWK_UpdateKeyReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_NWK_REQ_UNPAIR_FID,       RF4CE_NWK_UnpairReqDescr_t,         RF4CE_NWK_UnpairConfParams_t,   RF4CE_NWK_UnpairReq),
    SERVER_TABLE_ENTRY(RESP,                RF4CE_NWK_RESP_UNPAIR_FID,      RF4CE_NWK_UnpairRespDescr_t,        NoAppropriateType_t,            RF4CE_NWK_UnpairResp),
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_DATA,  RF4CE_NWK_REQ_DISCOVERY_FID,    RF4CE_NWK_DiscoveryReqDescr_t,      RF4CE_NWK_DiscoveryConfParams_t, RF4CE_NWK_DiscoveryReq),
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_DATA,  RF4CE_NWK_REQ_PAIR_FID,         RF4CE_NWK_PairReqDescr_t,           RF4CE_NWK_PairConfParams_t,     RF4CE_NWK_PairReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_NWK_MAC_STATS_REQ_FID,    RF4CE_NWK_MacStatsReqDescr_t,       RF4CE_NWK_MacStatsConfParams_t, RF4CE_NWK_MacStatsReq),
#   if defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_NWK_REQ_AUTODISCOVERY_FID, RF4CE_NWK_AutoDiscoveryReqDescr_t, RF4CE_NWK_AutoDiscoveryConfParams_t, RF4CE_NWK_AutoDiscoveryReq),
    SERVER_TABLE_ENTRY(RESP,                RF4CE_NWK_RESP_DISCOVERY_FID,   RF4CE_NWK_DiscoveryRespDescr_t,     NoAppropriateType_t,            RF4CE_NWK_DiscoveryResp),
    SERVER_TABLE_ENTRY(RESP,                RF4CE_NWK_RESP_PAIR_FID,        RF4CE_NWK_PairRespDescr_t,          NoAppropriateType_t,            RF4CE_NWK_PairResp),
#   endif /* RF4CE_TARGET */
#  endif /* MAILBOX_STACK_SIDE */

# ifdef MAILBOX_HOST_SIDE
#  if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_NWK_)
    SERVER_TABLE_ENTRY(IND_NO_RESP,         RF4CE_NWK_IND_DATA_FID,         RF4CE_NWK_DataIndParams_t,          NoAppropriateType_t,            RF4CE_NWK_DataInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         RF4CE_NWK_IND_DISCOVERY_FID,    RF4CE_NWK_DiscoveryIndParams_t,     NoAppropriateType_t,            RF4CE_NWK_DiscoveryInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         RF4CE_NWK_IND_COMMSTATUS_FID,   RF4CE_NWK_COMMStatusIndParams_t,    NoAppropriateType_t,            RF4CE_NWK_COMMStatusInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         RF4CE_NWK_IND_PAIR_FID,         RF4CE_NWK_PairIndParams_t,          NoAppropriateType_t,            RF4CE_NWK_PairInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         RF4CE_NWK_IND_UNPAIR_FID,       RF4CE_NWK_UnpairIndParams_t,        NoAppropriateType_t,            RF4CE_NWK_UnpairInd),
#   ifdef ENABLE_GU_KEY_SEED_IND
    SERVER_TABLE_ENTRY(IND_NO_RESP,         RF4CE_NWK_KEY_SEED_START_IND_FID,   RF4CE_NWK_KeySeedStartIndParams_t,  NoAppropriateType_t,            RF4CE_NWK_KeySeedStartInd),
#    endif /* ENABLE_GU_KEY_SEED_IND */
#   endif /* WRAPPERS_ALL */
#  endif /* MAILBOX_HOST_SIDE */
# endif /* defined(_RF4CE_) */

/**** ZBPRO *****************************************************************************/
# ifdef _ZBPRO_
#  ifdef MAILBOX_STACK_SIDE
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_NWK_REQ_PERMIT_JOINING_FID, ZBPRO_NWK_PermitJoiningReqDescr_t, ZBPRO_NWK_PermitJoiningConfParams_t, ZBPRO_NWK_PermitJoiningReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_NWK_REQ_LEAVE_FID,        ZBPRO_NWK_LeaveReqDescr_t,          ZBPRO_NWK_LeaveConfParams_t, ZBPRO_NWK_LeaveReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_NWK_REQ_GET_KEY_FID,      ZBPRO_NWK_GetKeyReqDescr_t,         ZBPRO_NWK_GetKeyConfParams_t, ZBPRO_NWK_GetKeyReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_NWK_REQ_SET_KEY_FID,      ZBPRO_NWK_SetKeyReqDescr_t,         ZBPRO_NWK_SetKeyConfParams_t, ZBPRO_NWK_SetKeyReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_NWK_REQ_ROUTE_DISCOVERY_FID, ZBPRO_NWK_RouteDiscoveryReqDescr_t, ZBPRO_NWK_RouteDiscoveryConfParams_t, ZBPRO_NWK_RouteDiscoveryReq),
#  endif /* MAILBOX_STACK_SIDE */
# endif /* _ZBPRO_ */
#endif /* (WRAPPERS_OFF != _MAILBOX_WRAPPERS_NWK_) */

/**********************************************************************************************************************/
/* APS API                                                                                                            */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_APS_)
# ifdef _ZBPRO_
#  ifdef MAILBOX_STACK_SIDE
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_APS_REQ_ENDPOINTREGISTER_FID, ZBPRO_APS_EndpointRegisterReqDescr_t, ZBPRO_APS_EndpointRegisterConfParams_t,   ZBPRO_APS_EndpointRegisterReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_APS_REQ_ENDPOINTUNREGISTER_FID, ZBPRO_APS_EndpointUnregisterReqDescr_t, ZBPRO_APS_EndpointRegisterConfParams_t,   ZBPRO_APS_EndpointUnregisterReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_APS_REQ_DATA_FID,         ZBPRO_APS_DataReqDescr_t,           ZBPRO_APS_DataConfParams_t,                     ZBPRO_APS_DataReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_APS_REQ_BIND_FID,         ZBPRO_APS_BindUnbindReqDescr_t,     ZBPRO_APS_BindUnbindConfParams_t,               ZBPRO_APS_BindReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_APS_REQ_UNBIND_FID,       ZBPRO_APS_BindUnbindReqDescr_t,     ZBPRO_APS_BindUnbindConfParams_t,               ZBPRO_APS_UnbindReq),
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_DATA,  ZBPRO_APS_REQ_GET_FID,          ZBPRO_APS_GetReqDescr_t,            ZBPRO_APS_GetConfParams_t,                      ZBPRO_APS_GetReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_APS_REQ_SET_FID,          ZBPRO_APS_SetReqDescr_t,            ZBPRO_APS_SetConfParams_t,                      ZBPRO_APS_SetReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_APS_REQ_GET_KEY_FID,      ZBPRO_APS_GetKeyReqDescr_t,         ZBPRO_APS_GetKeyConfParams_t,                   ZBPRO_APS_GetKeyReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_APS_REQ_SET_KEY_FID,      ZBPRO_APS_SetKeyReqDescr_t,         ZBPRO_APS_SetKeyConfParams_t,                   ZBPRO_APS_SetKeyReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_APS_REQ_ADDGROUP_FID,     ZBPRO_APS_AddGroupReqDescr_t,       ZBPRO_APS_AddGroupConfParams_t,                 ZBPRO_APS_AddGroupReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_APS_REQ_REMOVEGROUP_FID,  ZBPRO_APS_RemoveGroupReqDescr_t,    ZBPRO_APS_RemoveGroupConfParams_t,              ZBPRO_APS_RemoveGroupReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_APS_REQ_REMOVEALLGROUPS_FID, ZBPRO_APS_RemoveAllGroupsReqDescr_t, ZBPRO_APS_RemoveAllGroupsConfParams_t,      ZBPRO_APS_RemoveAllGroupsReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_APS_REQ_TRANSPORTKEY_FID, ZBPRO_APS_TransportKeyReqDescr_t,   ZBPRO_APS_SecurityServicesConfParams_t,         ZBPRO_APS_TransportKeyReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_APS_REQ_UPDATEDEVICE_FID, ZBPRO_APS_UpdateDeviceReqDescr_t,   ZBPRO_APS_SecurityServicesConfParams_t,         ZBPRO_APS_UpdateDeviceReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_APS_REQ_REMOTEDEVICE_FID, ZBPRO_APS_RemoveDeviceReqDescr_t,   ZBPRO_APS_SecurityServicesConfParams_t,         ZBPRO_APS_RemoveDeviceReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_APS_REQ_REQUESTKEY_FID,   ZBPRO_APS_RequestKeyReqDescr_t,     ZBPRO_APS_SecurityServicesConfParams_t,         ZBPRO_APS_RequestKeyReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_APS_REQ_SWITCHKEY_FID,    ZBPRO_APS_SwitchKeyReqDescr_t,      ZBPRO_APS_SecurityServicesConfParams_t,         ZBPRO_APS_SwitchKeyReq),
#  endif /* MAILBOX_STACK_SIDE */

#  ifdef MAILBOX_HOST_SIDE
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_APS_)
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_APS_IND_DATA_FID,         ZBPRO_APS_DataIndParams_t,          NoAppropriateType_t,            ZBPRO_APS_DataInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_APS_IND_TRANSPORTKEY_FID, ZBPRO_APS_TransportKeyIndParams_t,  NoAppropriateType_t,            ZBPRO_APS_TransportKeyInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_APS_IND_UPDATEDEVICE_FID, ZBPRO_APS_UpdateDeviceIndParams_t,  NoAppropriateType_t,            ZBPRO_APS_UpdateDeviceInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_APS_IND_REMOTEDEVICE_FID, ZBPRO_APS_RemoveDeviceIndParams_t,  NoAppropriateType_t,            ZBPRO_APS_RemoveDeviceInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_APS_IND_REQUESTKEY_FID,   ZBPRO_APS_RequestKeyIndParams_t,    NoAppropriateType_t,            ZBPRO_APS_RequestKeyInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_APS_IND_SWITCHKEY_FID,    ZBPRO_APS_SwitchKeyIndParams_t,     NoAppropriateType_t,            ZBPRO_APS_SwitchKeyInd),
#   endif /* WRAPPERS_ALL */
#  endif /* MAILBOX_HOST_SIDE */
# endif /* _ZBPRO_ */
#endif /* _MAILBOX_WRAPPERS_APS_ */

/**********************************************************************************************************************/
/* ZDO API                                                                                                            */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZDO_)
# ifdef _ZBPRO_
#  ifdef MAILBOX_STACK_SIDE
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_DATA,          ZBPRO_ZDO_REQ_ADDR_RESOLVING_FID,           ZBPRO_ZDO_AddrResolvingReqDescr_t,          ZBPRO_ZDO_AddrResolvingConfParams_t,        ZBPRO_ZDO_AddrResolvingReq),
    SERVER_TABLE_ENTRY(REQ_CONF,                    ZBPRO_ZDO_REQ_NODE_DESC_FID,                ZBPRO_ZDO_NodeDescReqDescr_t,               ZBPRO_ZDO_NodeDescConfParams_t,             ZBPRO_ZDO_NodeDescReq),
    SERVER_TABLE_ENTRY(REQ_CONF,                    ZBPRO_ZDO_REQ_POWER_DESC_FID,               ZBPRO_ZDO_PowerDescReqDescr_t,              ZBPRO_ZDO_PowerDescConfParams_t,            ZBPRO_ZDO_PowerDescReq),
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_NAMED_DATA,    ZBPRO_ZDO_REQ_SIMPLE_DESC_FID,              ZBPRO_ZDO_SimpleDescReqDescr_t,             ZBPRO_ZDO_SimpleDescConfParams_t,           ZBPRO_ZDO_SimpleDescReq, simpleDescriptor.inOutClusterList),
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_NAMED_DATA,    ZBPRO_ZDO_REQ_ACTIVE_EP_FID,                ZBPRO_ZDO_ActiveEpReqDescr_t,               ZBPRO_ZDO_ActiveEpConfParams_t,             ZBPRO_ZDO_ActiveEpReq,   activeEpList),
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_NAMED_DATA,    ZBPRO_ZDO_REQ_MATCH_DESC_FID,               ZBPRO_ZDO_MatchDescReqDescr_t,              ZBPRO_ZDO_MatchDescConfParams_t,            ZBPRO_ZDO_MatchDescReq,  responseList),
    SERVER_TABLE_ENTRY(REQ_CONF,                    ZBPRO_ZDO_REQ_DEVICE_ANNCE_FID,             ZBPRO_ZDO_DeviceAnnceReqDescr_t,            ZBPRO_ZDO_DeviceAnnceConfParams_t,          ZBPRO_ZDO_DeviceAnnceReq),
    SERVER_TABLE_ENTRY(REQ_CONF,                    ZBPRO_ZDO_REQ_ED_BIND_FID,                  ZBPRO_ZDO_EndDeviceBindReqDescr_t,          ZBPRO_ZDO_BindConfParams_t,                 ZBPRO_ZDO_EndDeviceBindReq),
    SERVER_TABLE_ENTRY(REQ_CONF,                    ZBPRO_ZDO_REQ_BIND_FID,                     ZBPRO_ZDO_BindUnbindReqDescr_t,             ZBPRO_ZDO_BindConfParams_t,                 ZBPRO_ZDO_BindReq),
    SERVER_TABLE_ENTRY(REQ_CONF,                    ZBPRO_ZDO_REQ_UNBIND_FID,                   ZBPRO_ZDO_BindUnbindReqDescr_t,             ZBPRO_ZDO_BindConfParams_t,                 ZBPRO_ZDO_UnbindReq),
    SERVER_TABLE_ENTRY(REQ_NO_PARAM_CONF,           ZBPRO_ZDO_REQ_START_NETWORK_FID,            ZBPRO_ZDO_StartNetworkReqDescr_t,           ZBPRO_ZDO_StartNetworkConfParams_t,         ZBPRO_ZDO_StartNetworkReq),
    SERVER_TABLE_ENTRY(REQ_CONF,                    ZBPRO_ZDO_REQ_MGMT_LEAVE_FID,               ZBPRO_ZDO_MgmtLeaveReqDescr_t,              ZBPRO_ZDO_MgmtLeaveConfParams_t,            ZBPRO_ZDO_MgmtLeaveReq),
    SERVER_TABLE_ENTRY(REQ_CONF,                    ZBPRO_ZDO_REQ_MGMT_PERMIT_JOINING_FID,      ZBPRO_ZDO_MgmtPermitJoiningReqDescr_t,      ZBPRO_ZDO_MgmtPermitJoiningConfParams_t,    ZBPRO_ZDO_MgmtPermitJoiningReq),
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_DATA,          ZBPRO_ZDO_REQ_MGMT_NWK_UPDATE_FID,          ZBPRO_ZDO_MgmtNwkUpdateReqDescr_t,          ZBPRO_ZDO_MgmtNwkUpdateConfParams_t,        ZBPRO_ZDO_MgmtNwkUpdateReq),
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_DATA,          ZBPRO_ZDO_REQ_MGMT_BIND_FID,                ZBPRO_ZDO_MgmtBindReqDescr_t,               ZBPRO_ZDO_MgmtBindConfParams_t,             ZBPRO_ZDO_MgmtBindReq),
    SERVER_TABLE_ENTRY(REQ_CONF,                    ZBPRO_ZDO_RESP_MGMT_NWK_UPDATE_UNSOL_FID,   ZBPRO_ZDO_MgmtNwkUpdateUnsolRespDescr_t,    ZBPRO_ZDO_MgmtNwkUpdateUnsolConfParams_t,   ZBPRO_ZDO_MgmtNwkUpdateUnsolResp),
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_DATA,          ZBPRO_ZDO_REQ_MGMT_LQI_FID,                 ZBPRO_ZDO_MgmtLqiReqDescr_t,                ZBPRO_ZDO_MgmtLqiConfParams_t,              ZBPRO_ZDO_MgmtLqiReq),
#  endif /* MAILBOX_STACK_SIDE */
#  ifdef MAILBOX_HOST_SIDE
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_ZDO_)
    SERVER_TABLE_ENTRY(IND_NO_RESP,                 ZBPRO_ZDO_IND_MGMT_NWK_UPDATE_UNSOL_FID,    ZBPRO_ZDO_MgmtNwkUpdateUnsolIndParams_t,    NoAppropriateType_t,                        ZBPRO_ZDO_MgmtNwkUpdateUnsolInd),
#   endif /* WRAPPERS_ALL */
#  endif /* MAILBOX_HOST_SIDE */
# endif /* _ZBPRO_ */
#endif /* _MAILBOX_WRAPPERS_ZDO_ */

/**********************************************************************************************************************/
/* TC API                                                                                                            */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_TC_)
# ifdef _ZBPRO_
#  ifdef MAILBOX_STACK_SIDE
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_TC_REQ_NWK_KEY_UPDATE_FID,    ZBPRO_TC_NwkKeyUpdateReqDescr_t,      ZBPRO_TC_NwkKeyUpdateConfParams_t,         ZBPRO_TC_NwkKeyUpdateReq),
#  endif /* MAILBOX_STACK_SIDE */
# endif /* _ZBPRO_ */
#endif /* _MAILBOX_WRAPPERS_TC_ */

/**********************************************************************************************************************/
/* ZCL API                                                                                                            */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_)
/**** ZBPRO *****************************************************************************/
# ifdef _ZBPRO_
#  ifdef MAILBOX_STACK_SIDE
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_REQ_SET_POWER_SOURCE_FID,                             ZBPRO_ZCL_SetPowerSourceReqDescr_t,                         ZBPRO_ZCL_SetPowerSourceConfParams_t,               ZBPRO_ZCL_SetPowerSourceReq),
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_DATA,  ZBPRO_ZCL_PROFILE_WIDE_CMD_DISCOVER_ATTRIBUTE_REQ_FID,          ZBPRO_ZCL_ProfileWideCmdDiscoverAttrDescr_t,                ZBPRO_ZCL_ProfileWideCmdDiscoverAttrConfParams_t,   ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReq),
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_DATA,  ZBPRO_ZCL_PROFILE_WIDE_CMD_READ_ATTRIBUTE_REQ_FID,              ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t,                 ZBPRO_ZCL_ProfileWideCmdReadAttrConfParams_t,       ZBPRO_ZCL_ProfileWideCmdReadAttributesReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_PROFILE_WIDE_CMD_WRITE_ATTRIBUTE_REQ_FID,             ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t,                ZBPRO_ZCL_ProfileWideCmdWriteAttrConfParams_t,      ZBPRO_ZCL_ProfileWideCmdWriteAttributesReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_PROFILE_WIDE_CMD_CONFIGURE_REPORTING_REQ_FID,    ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t,   ZBPRO_ZCL_ProfileWideCmdConfigureReportingConfParams_t,  ZBPRO_ZCL_ProfileWideCmdConfigureReportingReq),
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_DATA,  ZBPRO_ZCL_PROFILE_WIDE_CMD_READ_REPORTING_CONFIGURATION_REQ_FID,  ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqDescr_t,  ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationConfParams_t,  ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_REQ_FID,                        ZBPRO_ZCL_IdentifyCmdIdentifyReqDescr_t,                    ZBPRO_ZCL_IdentifyCmdConfParams_t,                  ZBPRO_ZCL_IdentifyCmdIdentifyReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_REQ_FID,                  ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqDescr_t,               ZBPRO_ZCL_IdentifyCmdConfParams_t,                  ZBPRO_ZCL_IdentifyCmdIdentifyQueryReq),
#   if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_IDENTIFY_RELAY_)
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_RESPONSE_REQ_FID,               ZBPRO_ZCL_IdentifyCmdIdentifyResponseReqDescr_t,            ZBPRO_ZCL_IdentifyCmdConfParams_t,                  ZBPRO_ZCL_IdentifyCmdIdentifyResponseReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_RESPONSE_REQ_FID,         ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReqDescr_t,       ZBPRO_ZCL_IdentifyCmdConfParams_t,                  ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReq),
#   endif
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_GROUPS_CMD_ADD_GROUP_REQ_FID,                         ZBPRO_ZCL_GroupsCmdAddGroupReqDescr_t,                      ZBPRO_ZCL_GroupsCmdAddGroupConfParams_t,            ZBPRO_ZCL_GroupsCmdAddGroupReq),
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_DATA,  ZBPRO_ZCL_GROUPS_CMD_VIEW_GROUP_REQ_FID,                        ZBPRO_ZCL_GroupsCmdViewGroupReqDescr_t,                     ZBPRO_ZCL_GroupsCmdViewGroupConfParams_t,           ZBPRO_ZCL_GroupsCmdViewGroupReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_GROUPS_CMD_GET_GROUP_MEMBERSHIP_REQ_FID,              ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqDescr_t,            ZBPRO_ZCL_GroupsCmdGetGroupMembershipConfParams_t,  ZBPRO_ZCL_GroupsCmdGetGroupMembershipReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_GROUPS_CMD_REMOVE_GROUP_REQ_FID,                      ZBPRO_ZCL_GroupsCmdRemoveGroupReqDescr_t,                   ZBPRO_ZCL_GroupsCmdRemoveGroupConfParams_t,         ZBPRO_ZCL_GroupsCmdRemoveGroupReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_GROUPS_CMD_REMOVE_ALL_GROUPS_REQ_FID,                 ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqDescr_t,               ZBPRO_ZCL_GroupsCmdRemoveAllGroupsConfParams_t,     ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_GROUPS_CMD_ADD_GROUP_IF_IDENTIFY_REQ_FID,             ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqDescr_t,            ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyConfParams_t,  ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_SCENES_CMD_ADD_SCENE_REQ_FID,                         ZBPRO_ZCL_ScenesCmdAddSceneReqDescr_t,                      ZBPRO_ZCL_ScenesCmdAddSceneConfParams_t,            ZBPRO_ZCL_ScenesCmdAddSceneReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_SCENES_CMD_VIEW_SCENE_REQ_FID,                        ZBPRO_ZCL_ScenesCmdViewSceneReqDescr_t,                     ZBPRO_ZCL_ScenesCmdViewSceneConfParams_t,           ZBPRO_ZCL_ScenesCmdViewSceneReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_SCENES_CMD_STORE_SCENE_REQ_FID,                       ZBPRO_ZCL_ScenesCmdStoreSceneReqDescr_t,                    ZBPRO_ZCL_ScenesCmdStoreSceneConfParams_t,          ZBPRO_ZCL_ScenesCmdStoreSceneReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_SCENES_CMD_RECALL_SCENE_REQ_FID,                      ZBPRO_ZCL_ScenesCmdRecallSceneReqDescr_t,                   ZBPRO_ZCL_ScenesCmdConfParams_t,                    ZBPRO_ZCL_ScenesCmdRecallSceneReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_SCENES_CMD_REMOVE_SCENE_REQ_FID,                      ZBPRO_ZCL_ScenesCmdRemoveSceneReqDescr_t,                   ZBPRO_ZCL_ScenesCmdRemoveSceneConfParams_t,         ZBPRO_ZCL_ScenesCmdRemoveSceneReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_SCENES_CMD_REMOVE_ALL_SCENES_REQ_FID,                 ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqDescr_t,               ZBPRO_ZCL_ScenesCmdRemoveAllScenesConfParams_t,     ZBPRO_ZCL_ScenesCmdRemoveAllScenesReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_SCENES_CMD_GET_SCENE_MEMBERSHIP_REQ_FID,              ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqDescr_t,            ZBPRO_ZCL_ScenesCmdConfParams_t,                    ZBPRO_ZCL_ScenesCmdGetSceneMembershipReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_ONOFF_CMD_OFF_REQ_FID,                                ZBPRO_ZCL_OnOffCmdReqDescr_t,                               ZBPRO_ZCL_OnOffCmdConfParams_t,                     ZBPRO_ZCL_OnOffCmdOffReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_ONOFF_CMD_ON_REQ_FID,                                 ZBPRO_ZCL_OnOffCmdReqDescr_t,                               ZBPRO_ZCL_OnOffCmdConfParams_t,                     ZBPRO_ZCL_OnOffCmdOnReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_ONOFF_CMD_TOGGLE_REQ_FID,                             ZBPRO_ZCL_OnOffCmdReqDescr_t,                               ZBPRO_ZCL_OnOffCmdConfParams_t,                     ZBPRO_ZCL_OnOffCmdToggleReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_LEVEL_CONTROL_CMD_MOVE_TO_LEVEL_REQ_FID,              ZBPRO_ZCL_LevelControlCmdMoveToLevelReqDescr_t,             ZBPRO_ZCL_LevelControlCmdConfParams_t,              ZBPRO_ZCL_LevelControlCmdMoveToLevelReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_LEVEL_CONTROL_CMD_MOVE_REQ_FID,                       ZBPRO_ZCL_LevelControlCmdMoveReqDescr_t,                    ZBPRO_ZCL_LevelControlCmdConfParams_t,              ZBPRO_ZCL_LevelControlCmdMoveReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_LEVEL_CONTROL_CMD_STEP_REQ_FID,                       ZBPRO_ZCL_LevelControlCmdStepReqDescr_t,                    ZBPRO_ZCL_LevelControlCmdConfParams_t,              ZBPRO_ZCL_LevelControlCmdStepReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_LEVEL_CONTROL_CMD_STOP_REQ_FID,                       ZBPRO_ZCL_LevelControlCmdStopReqDescr_t,                    ZBPRO_ZCL_LevelControlCmdConfParams_t,              ZBPRO_ZCL_LevelControlCmdStopReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_DOOR_LOCK_CMD_LOCK_REQ_FID,                           ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t,                  ZBPRO_ZCL_DoorLockCmdLockUnlockConfParams_t,        ZBPRO_ZCL_DoorLockCmdLockReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_DOOR_LOCK_CMD_UNLOCK_REQ_FID,                         ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t,                  ZBPRO_ZCL_DoorLockCmdLockUnlockConfParams_t,        ZBPRO_ZCL_DoorLockCmdUnlockReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_WINDOW_COVERING_CMD_UP_OPEN_REQ_FID,                  ZBPRO_ZCL_WindowCoveringCmdReqDescr_t,                      ZBPRO_ZCL_WindowCoveringCmdConfParams_t,            ZBPRO_ZCL_WindowCoveringCmdUpOpenReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_WINDOW_COVERING_CMD_DOWN_CLOSE_REQ_FID,               ZBPRO_ZCL_WindowCoveringCmdReqDescr_t,                      ZBPRO_ZCL_WindowCoveringCmdConfParams_t,            ZBPRO_ZCL_WindowCoveringCmdDownCloseReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_WINDOW_COVERING_CMD_STOP_REQ_FID,                     ZBPRO_ZCL_WindowCoveringCmdReqDescr_t,                      ZBPRO_ZCL_WindowCoveringCmdConfParams_t,            ZBPRO_ZCL_WindowCoveringCmdStopReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_WINDOW_COVERING_CMD_GO_TO_LIFT_PERCENTAGE_REQ_FID,    ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t,       ZBPRO_ZCL_WindowCoveringCmdConfParams_t,            ZBPRO_ZCL_WindowCoveringCmdGotoLiftPecentageReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_WINDOW_COVERING_CMD_GO_TO_TILT_PERCENTAGE_REQ_FID,    ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t,       ZBPRO_ZCL_WindowCoveringCmdConfParams_t,            ZBPRO_ZCL_WindowCoveringCmdGotoTiltPecentageReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_ENROLL_RESPONSE_REQ_FID,            ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqDescr_t,           ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseConfParams_t, ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_IAS_ACE_ARM_RESP_REQ_FID,                             ZBPRO_ZCL_SapIasAceArmRespReqDescr_t,                       ZBPRO_ZCL_SapIasAceRespReqConfParams_t,             ZBPRO_ZCL_SapIasAceArmRespReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_IAS_ACE_BYPASS_RESP_REQ_FID,                          ZBPRO_ZCL_SapIasAceBypassRespReqDescr_t,                    ZBPRO_ZCL_SapIasAceRespReqConfParams_t,             ZBPRO_ZCL_SapIasAceBypassRespReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_IAS_ACE_GET_ZONE_ID_MAP_RESP_REQ_FID,                 ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqDescr_t,              ZBPRO_ZCL_SapIasAceRespReqConfParams_t,             ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_IAS_ACE_GET_ZONE_INFO_RESP_REQ_FID,                   ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqDescr_t,               ZBPRO_ZCL_SapIasAceRespReqConfParams_t,             ZBPRO_ZCL_SapIasAceGetZoneInfoRespReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_IAS_ACE_GET_PANEL_STATUS_RESP_REQ_FID,                ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqDescr_t,            ZBPRO_ZCL_SapIasAceRespReqConfParams_t,             ZBPRO_ZCL_SapIasAceGetPanelStatusRespReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_IAS_ACE_SET_BYPASSED_ZONE_LIST_RESP_REQ_FID,          ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqDescr_t,       ZBPRO_ZCL_SapIasAceRespReqConfParams_t,             ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_IAS_ACE_GET_ZONE_STATUS_RESP_REQ_FID,                 ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqDescr_t,             ZBPRO_ZCL_SapIasAceRespReqConfParams_t,             ZBPRO_ZCL_SapIasAceGetZoneStatusRespReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_IAS_ACE_ZONE_STATUS_CHANGED_REQ_FID,                  ZBPRO_ZCL_SapIasAceZoneStatusChangedReqDescr_t,             ZBPRO_ZCL_SapIasAceRespReqConfParams_t,             ZBPRO_ZCL_SapIasAceZoneStatusChangedReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_IAS_ACE_PANEL_STATUS_CHANGED_REQ_FID,                 ZBPRO_ZCL_SapIasAcePanelStatusChangedReqDescr_t,            ZBPRO_ZCL_SapIasAceRespReqConfParams_t,             ZBPRO_ZCL_SapIasAcePanelStatusChangedReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_IAS_WD_CMD_START_WARNING_REQ_FID,                     ZBPRO_ZCL_IASWDCmdStartWarningReqDescr_t,                   ZBPRO_ZCL_IASWDCmdStartWarningConfParams_t,         ZBPRO_ZCL_IASWDCmdStartWarningReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_IAS_WD_CMD_SQUAWK_REQ_FID,                            ZBPRO_ZCL_IASWDCmdSquawkReqDescr_t,                         ZBPRO_ZCL_IASWDCmdSquawkConfParams_t,               ZBPRO_ZCL_IASWDCmdSquawkgReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_TO_COLOR_REQ_FID,          ZBPRO_ZCL_ColorControlCmdMoveToColorReqDescr_t,         ZBPRO_ZCL_ColorControlCmdMoveToColorConfParams_t,       ZBPRO_ZCL_ColorControlCmdMoveToColorReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_COLOR_REQ_FID,             ZBPRO_ZCL_ColorControlCmdMoveColorReqDescr_t,           ZBPRO_ZCL_ColorControlCmdMoveColorConfParams_t,         ZBPRO_ZCL_ColorControlCmdMoveColorReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_COLOR_CONTROL_CMD_STEP_COLOR_REQ_FID,             ZBPRO_ZCL_ColorControlCmdStepColorReqDescr_t,           ZBPRO_ZCL_ColorControlCmdStepColorConfParams_t,         ZBPRO_ZCL_ColorControlCmdStepColorReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_TO_HUE_REQ_FID,       ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqDescr_t,   ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueConfParams_t,  ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_HUE_REQ_FID,          ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqDescr_t,     ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueConfParams_t,     ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_STEP_HUE_REQ_FID,          ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqDescr_t,     ZBPRO_ZCL_ColorControlCmdEnhancedStepHueConfParams_t,     ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_TO_HUE_AND_SATURATION_REQ_FID,    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqDescr_t,    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationConfParams_t,   ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_COLOR_CONTROL_CMD_COLOR_LOOP_SET_REQ_FID,             ZBPRO_ZCL_ColorControlCmdColorLoopSetReqDescr_t,        ZBPRO_ZCL_ColorControlCmdColorLoopSetConfParams_t,        ZBPRO_ZCL_ColorControlCmdColorLoopSetReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_COLOR_CONTROL_CMD_STOP_MOVE_STEP_REQ_FID,             ZBPRO_ZCL_ColorControlCmdStopMoveStepReqDescr_t,        ZBPRO_ZCL_ColorControlCmdStopMoveStepConfParams_t,        ZBPRO_ZCL_ColorControlCmdStopMoveStepReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_COLOR_TEMPERATURE_REQ_FID,     ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqDescr_t,    ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureConfParams_t,    ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZCL_COLOR_CONTROL_CMD_STEP_COLOR_TEMPERATURE_REQ_FID,     ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqDescr_t,    ZBPRO_ZCL_ColorControlCmdStepColorTemperatureConfParams_t,    ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReq),

#  endif /* MAILBOX_STACK_SIDE */
#  ifdef MAILBOX_HOST_SIDE
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_ZCL_)
#    if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_IDENTIFY_RELAY_)
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_IND_FID,                        ZBPRO_ZCL_IdentifyCmdIdentifyIndParams_t,                   NoAppropriateType_t,                                ZBPRO_ZCL_IdentifyCmdIdentifyInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_IND_FID,                  ZBPRO_ZCL_IdentifyCmdIdentifyQueryIndParams_t,              NoAppropriateType_t,                                ZBPRO_ZCL_IdentifyCmdIdentifyQueryInd),
#    else
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_ZCL_IND_IDENTIFY_FID,                                     ZBPRO_ZCL_IdentifyIndParams_t,                              NoAppropriateType_t,                                ZBPRO_ZCL_IdentifyInd),
#    endif
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_RESPONSE_IND_FID,         ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseIndParams_t,      NoAppropriateType_t,                                ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseIndEB),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_ZCL_GROUPS_CMD_GET_GROUP_MEMBERSHIP_RESPONSE_IND_FID,     ZBPRO_ZCL_GroupsCmdGetGroupMembershipIndParams_t,           NoAppropriateType_t,                                ZBPRO_ZCL_GroupsCmdGetGroupMembershipResponseInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_ZCL_SCENES_CMD_GET_SCENE_MEMBERSHIP_RESPONSE_IND_FID,     ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseIndParams_t,   NoAppropriateType_t,                                ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_ENROLL_IND_FID,                      ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestIndParams_t,            NoAppropriateType_t,                              ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_STATUS_CHANGED_NOTIFICATION_IND_FID, ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationIndParams_t, NoAppropriateType_t,                              ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_ZCL_IAS_ACE_ARM_IND_FID,                                  ZBPRO_ZCL_SapIasAceArmIndParams_t,                          NoAppropriateType_t,                                ZBPRO_ZCL_SapIasAceArmInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_ZCL_IAS_ACE_BYPASS_IND_FID,                               ZBPRO_ZCL_SapIasAceBypassIndParams_t,                       NoAppropriateType_t,                                ZBPRO_ZCL_SapIasAceBypassInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_ZCL_IAS_ACE_EMERGENCY_IND_FID,                            ZBPRO_ZCL_SapIasAceAlarmIndParams_t,                        NoAppropriateType_t,                                ZBPRO_ZCL_SapIasAceEmergencyInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_ZCL_IAS_ACE_FIRE_IND_FID,                                 ZBPRO_ZCL_SapIasAceAlarmIndParams_t,                        NoAppropriateType_t,                                ZBPRO_ZCL_SapIasAceFireInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_ZCL_IAS_ACE_PANIC_IND_FID,                                ZBPRO_ZCL_SapIasAceAlarmIndParams_t,                        NoAppropriateType_t,                                ZBPRO_ZCL_SapIasAcePanicInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_ZCL_IAS_ACE_GET_ZONE_ID_MAP_IND_FID,                      ZBPRO_ZCL_SapIasAceGetZoneIdMapIndParams_t,                 NoAppropriateType_t,                                ZBPRO_ZCL_SapIasAceGetZoneIdMapInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_ZCL_IAS_ACE_GET_ZONE_INFO_IND_FID,                        ZBPRO_ZCL_SapIasAceGetZoneInfoIndParams_t,                  NoAppropriateType_t,                                ZBPRO_ZCL_SapIasAceGetZoneInfoInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_ZCL_IAS_ACE_GET_PANEL_STATUS_IND_FID,                     ZBPRO_ZCL_SapIasAceGetPanelStatusIndParams_t,               NoAppropriateType_t,                                ZBPRO_ZCL_SapIasAceGetPanelStatusInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_ZCL_IAS_ACE_GET_BYPASSED_ZONE_LIST_IND_FID,               ZBPRO_ZCL_SapIasAceGetBypassedZoneListIndParams_t,          NoAppropriateType_t,                                ZBPRO_ZCL_SapIasAceGetBypassedZoneListInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_ZCL_IAS_ACE_GET_ZONE_STATUS_IND_FID,                      ZBPRO_ZCL_SapIasAceGetZoneStatusIndParams_t,                NoAppropriateType_t,                                ZBPRO_ZCL_SapIasAceGetZoneStatusInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,       ZBPRO_ZCL_PROFILE_WIDE_CMD_REPORT_ATTRIBUTES_IND_FID,             ZBPRO_ZCL_ProfileWideCmdReportAttributesIndParams_t,        NoAppropriateType_t,                                ZBPRO_ZCL_ProfileWideCmdReportAttributesInd),
#   endif /* WRAPPERS_ALL */
#  endif /* MAILBOX_HOST_SIDE */
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
#   ifdef MAILBOX_STACK_SIDE
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZHA_EZ_MODE_FID,                      ZBPRO_ZHA_EzModeReqDescr_t,                 ZBPRO_ZHA_EzModeConfParams_t,               ZBPRO_ZHA_EzModeReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZHA_CIE_ENROLL_FID,                   ZBPRO_ZHA_CieEnrollReqDescr_t,              ZBPRO_ZHA_CieEnrollConfParams_t,            ZBPRO_ZHA_CieDeviceEnrollReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZHA_CIE_SET_PANEL_STATUS_REQ_FID,     ZBPRO_ZHA_CieSetPanelStatusReqDescr_t,      ZBPRO_ZHA_CieSetPanelStatusConfParams_t,    ZBPRO_ZHA_CieDeviceSetPanelStatusReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            ZBPRO_ZHA_CIE_ZONE_SET_BYPASS_STATE_REQ_FID,     ZBPRO_ZHA_CieZoneSetBypassStateReqDescr_t,      ZBPRO_ZHA_CieZoneSetBypassStateConfParams_t,    ZBPRO_ZHA_CieZoneSetBypassStateReq),
#   endif /* MAILBOX_STACK_SIDE */
#   ifdef MAILBOX_HOST_SIDE
#    if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_ZHA_)
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_ZHA_CIE_SET_PANEL_STATUS_IND_FID,     ZBPRO_ZHA_CieSetPanelStatusIndParams_t,     NoAppropriateType_t,                        ZBPRO_ZHA_CieDeviceSetPanelStatusInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         ZBPRO_ZHA_CIE_ENROLL_IND_FID,               ZBPRO_ZHA_CieEnrollIndParams_t,             NoAppropriateType_t,                        ZBPRO_ZHA_CieDeviceEnrollInd),
#    endif /* WRAPPERS_ALL */
#   endif /* MAILBOX_HOST_SIDE */
#  endif /* _MAILBOX_WRAPPERS_ZHA_ */
# endif /* _ZBPRO_ */

/**** RF4CE *****************************************************************************/
# ifdef _RF4CE_

/**** RF4CE PROFILE MANAGER ****************/
# ifdef MAILBOX_STACK_SIDE
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_PROFILE_REQ_UNPAIR_FID,           RF4CE_UnpairReqDescr_t,                 RF4CE_UnpairConfParams_t,           RF4CE_UnpairReq),
    SERVER_TABLE_ENTRY(REQ_NO_PARAM_CONF,   RF4CE_PROFILE_REQ_START_FID,            RF4CE_StartReqDescr_t,                  RF4CE_StartResetConfParams_t,       RF4CE_StartReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_PROFILE_REQ_RESET_FID,            RF4CE_ResetReqDescr_t,                  RF4CE_StartResetConfParams_t,       RF4CE_ResetReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_PROFILE_REQ_SET_SUP_DEVICES_FID,  RF4CE_SetSupportedDevicesReqDescr_t,    RF4CE_SetSupportedDevicesConfParams_t, RF4CE_SetSupportedDevicesReq),
# endif /* MAILBOX_STACK_SIDE */
# ifdef MAILBOX_HOST_SIDE
#  if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_PROFILE_)
    SERVER_TABLE_ENTRY(IND_NO_RESP,         RF4CE_PROFILE_IND_COUNTER_EXPIRED_FID,      RF4CE_PairingReferenceIndParams_t,          NoAppropriateType_t,                        RF4CE_CounterExpiredInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         RF4CE_PROFILE_IND_UNPAIR_FID,               RF4CE_PairingReferenceIndParams_t,          NoAppropriateType_t,                        RF4CE_UnpairInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         RF4CE_PROFILE_IND_PAIR_FID,                 RF4CE_PairingIndParams_t,                   NoAppropriateType_t,                        RF4CE_PairInd),
#   endif /* WRAPPERS_ALL */
#  endif /* MAILBOX_HOST_SIDE */

/**** RF4CE PROFILE ZRC ********************/
# if (1 == USE_RF4CE_PROFILE_ZRC)

/**** RF4CE PROFILE ZRC 1.1 ****************/
# ifdef USE_RF4CE_PROFILE_ZRC1
#  ifdef MAILBOX_STACK_SIDE
        SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_ZRC1_REQ_GET_FID,                 RF4CE_ZRC1_GetAttributeDescr_t,         RF4CE_ZRC1_GetAttributeConfParams_t,    RF4CE_ZRC1_GetAttributesReq),
        SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_ZRC1_REQ_SET_FID,                 RF4CE_ZRC1_SetAttributeDescr_t,         RF4CE_ZRC1_SetAttributeConfParams_t,    RF4CE_ZRC1_SetAttributesReq),
        SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_ZRC1_REQ_COMMANDDISCOVERY_FID,    RF4CE_ZRC1_CommandDiscoveryReqDescr_t,  RF4CE_ZRC1_CommandDiscoveryConfParams_t, RF4CE_ZRC1_CommandDiscoveryReq),
        SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_ZRC1_CONTROL_COMMAND_PRESSED_FID, RF4CE_ZRC1_ControlCommandReqDescr_t,    RF4CE_ZRC_ControlCommandConfParams_t,    RF4CE_ZRC1_ControlCommandPressedReq),
        SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_ZRC1_CONTROL_COMMAND_RELEASED_FID,RF4CE_ZRC1_ControlCommandReqDescr_t,    RF4CE_ZRC_ControlCommandConfParams_t,    RF4CE_ZRC1_ControlCommandReleasedReq),
        SERVER_TABLE_ENTRY(REQ_NO_PARAM_CONF,   RF4CE_ZRC1_REQ_CONTROLLER_BIND_FID,            RF4CE_ZRC1_BindReqDescr_t,                  RF4CE_ZRC1_BindConfParams_t,       RF4CE_ZRC1_ControllerBindReq),
        SERVER_TABLE_ENTRY(REQ_CONF,   RF4CE_ZRC1_VENDORSPECIFIC_REQ_FID,            RF4CE_ZRC1_VendorSpecificReqDescr_t,                  RF4CE_ZRC1_VendorSpecificConfParams_t,       RF4CE_ZRC1_VendorSpecificReq),
#    if defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
            SERVER_TABLE_ENTRY(REQ_NO_PARAM_CONF,   RF4CE_ZRC1_REQ_TARGET_BIND_FID,            RF4CE_ZRC1_BindReqDescr_t,                  RF4CE_ZRC1_BindConfParams_t,       RF4CE_ZRC1_TargetBindReq),
#    endif /* RF4CE_TARGET */
#  endif /* MAILBOX_STACK_SIDE */

#  ifdef MAILBOX_HOST_SIDE
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_PROFILE_)
#    if defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
        SERVER_TABLE_ENTRY(IND_NO_RESP,         RF4CE_ZRC1_IND_CONTROLCOMMAND_FID,       RF4CE_ZRC1_ControlCommandIndParams_t,    NoAppropriateType_t,                RF4CE_ZRC1_ControlCommandInd),
#    endif /* RF4CE_TARGET */
     SERVER_TABLE_ENTRY(IND_NO_RESP,         RF4CE_ZRC1_VENDORSPECIFIC_IND_FID,       RF4CE_ZRC1_VendorSpecificIndParams_t,    NoAppropriateType_t,                RF4CE_ZRC1_VendorSpecificInd),
#   endif /* WRAPPERS_ALL */
#  endif /* MAILBOX_HOST_SIDE */
# endif /* USE_RF4CE_PROFILE_ZRC1 */

/**** RF4CE PROFILE GDP 2.0 & ZRC 2.0 ******/
# ifdef USE_RF4CE_PROFILE_ZRC2
#  ifdef MAILBOX_STACK_SIDE
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_DATA,  RF4CE_ZRC2_REQ_GET_FID,         RF4CE_ZRC2_GetAttributesReqDescr_t,     RF4CE_ZRC2_GetAttributesConfParams_t,   RF4CE_ZRC2_GetAttributesReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_ZRC2_REQ_SET_FID,                     RF4CE_ZRC2_SetAttributesReqDescr_t,         RF4CE_ZRC2_SetAttributesConfParams_t,       RF4CE_ZRC2_SetAttributesReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_ZRC2_KEY_EXCHANGE_FID,    RF4CE_ZRC2_KeyExchangeReqDescr_t,       RF4CE_ZRC2_KeyExchangeConfParams_t,     RF4CE_ZRC2_KeyExchangeReq),
    SERVER_TABLE_ENTRY(REQ_NO_PARAM_CONF,   RF4CE_ZRC2_BIND_FID,            RF4CE_ZRC2_BindReqDescr_t,              RF4CE_ZRC2_BindConfParams_t,            RF4CE_ZRC2_BindReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_ZRC2_PROXY_BIND_FID,      RF4CE_ZRC2_ProxyBindReqDescr_t,         RF4CE_ZRC2_BindConfParams_t,            RF4CE_ZRC2_ProxyBindReq),
    SERVER_TABLE_ENTRY(REQ_NO_PARAM_CONF,   RF4CE_ZRC2_ENABLE_BINDING_FID,  RF4CE_ZRC2_BindingReqDescr_t,           RF4CE_ZRC2_BindingConfParams_t,         RF4CE_ZRC2_EnableBindingReq),
    SERVER_TABLE_ENTRY(REQ_NO_PARAM_CONF,   RF4CE_ZRC2_DISABLE_BINDING_FID, RF4CE_ZRC2_BindingReqDescr_t,           RF4CE_ZRC2_BindingConfParams_t,         RF4CE_ZRC2_DisableBindingReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_ZRC2_SET_PUSH_BUTTON_STIMULUS_FID,    RF4CE_ZRC2_ButtonBindingReqDescr_t,         RF4CE_ZRC2_BindingConfParams_t,             RF4CE_ZRC2_SetPushButtonStimulusReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_ZRC2_CLEAR_PUSH_BUTTON_STIMULUS_FID,  RF4CE_ZRC2_ButtonBindingReqDescr_t,         RF4CE_ZRC2_BindingConfParams_t,             RF4CE_ZRC2_ClearPushButtonStimulusReq),
    SERVER_TABLE_ENTRY(RESP,                RF4CE_ZRC2_CHECK_VALIDATION_RESP_FID, RF4CE_ZRC2_CheckValidationRespDescr_t, NoAppropriateType_t,               RF4CE_ZRC2_CheckValidationResp),
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_ZRC2_CONTROL_COMMAND_PRESS_REQ_FID, RF4CE_ZRC2_ControlCommandReqDescr_t, RF4CE_ZRC2_ControlCommandConfParams_t, RF4CE_ZRC2_ControlCommandPressedReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_ZRC2_CONTROL_COMMAND_RELEASE_REQ_FID, RF4CE_ZRC2_ControlCommandReqDescr_t, RF4CE_ZRC2_ControlCommandConfParams_t, RF4CE_ZRC2_ControlCommandReleasedReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_GDP2_REQ_SET_POLL_CONSTRAINTS_FID,    RF4CE_GDP2_SetPollConstraintsReqDescr_t,    RF4CE_GDP2_SetPollConstraintsConfParams_t,  RF4CE_GDP2_SetPollConstraintsReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_GDP2_REQ_POLL_NEGOTIATION_FID,        RF4CE_GDP2_PollNegotiationReqDescr_t,       RF4CE_GDP2_PollNegotiationConfParams_t,     RF4CE_GDP2_PollNegotiationReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_GDP2_REQ_POLL_CLIENT_USER_EVENT_FID,  RF4CE_GDP2_PollClientUserEventReqDescr_t,   RF4CE_GDP2_PollClientUserEventConfParams_t, RF4CE_GDP2_PollClientUserEventReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_GDP2_REQ_CLIENT_NOTIFICATION_FID,     RF4CE_GDP2_ClientNotificationReqDescr_t,    RF4CE_GDP2_ClientNotificationConfParams_t,  RF4CE_GDP2_ClientNotificationReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_GDP2_REQ_IDENTIFY_CAP_ANNOUNCE_FID,   RF4CE_GDP2_IdentifyCapAnnounceReqDescr_t,   RF4CE_GDP2_IdentifyCapAnnounceConfParams_t, RF4CE_GDP2_IdentifyCapAnnounceReq),
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_GDP2_REQ_IDENTIFY_FID,                RF4CE_GDP2_IdentifyReqDescr_t,              RF4CE_GDP2_IdentifyConfParams_t,            RF4CE_GDP2_IdentifyReq),
#  endif /* MAILBOX_STACK_SIDE */
#  ifdef MAILBOX_HOST_SIDE
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_PROFILE_)
    SERVER_TABLE_ENTRY(IND_NO_RESP,    RF4CE_ZRC2_START_VALIDATION_FID,     RF4CE_ZRC2_CheckValidationIndParams_t,  NoAppropriateType_t,                    RF4CE_ZRC2_StartValidationInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,    RF4CE_ZRC2_CHECK_VALIDATION_IND_FID, RF4CE_ZRC2_CheckValidationIndParams_t,  NoAppropriateType_t,                    RF4CE_ZRC2_CheckValidationInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,    RF4CE_ZRC2_CONTROL_COMMAND_IND_FID,  RF4CE_ZRC2_ControlCommandIndParams_t,   NoAppropriateType_t,                    RF4CE_ZRC2_ControlCommandInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         RF4CE_GDP2_IND_POLL_NEGOTIATION_FID,        RF4CE_GDP2_PollNegotiationIndParams_t,      NoAppropriateType_t,                        RF4CE_GDP2_PollNegotiationInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         RF4CE_GDP2_IND_HEARTBEAT_FID,               RF4CE_GDP2_HeartbeatIndParams_t,            NoAppropriateType_t,                        RF4CE_GDP2_HeartbeatInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         RF4CE_GDP2_IND_CLIENT_NOTIFICATION_FID,     RF4CE_GDP2_ClientNotificationIndParams_t,   NoAppropriateType_t,                        RF4CE_GDP2_ClientNotificationInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         RF4CE_GDP2_IND_IDENTIFY_CAP_ANNOUNCE_FID,   RF4CE_GDP2_IdentifyCapAnnounceIndParams_t,  NoAppropriateType_t,                        RF4CE_GDP2_IdentifyCapAnnounceInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         RF4CE_GDP2_IND_IDENTIFY_FID,                RF4CE_GDP2_IdentifyIndParams_t,             NoAppropriateType_t,                        RF4CE_GDP2_IdentifyInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         RF4CE_ZRC2_PAIR_IND_FID,                    RF4CE_PairingIndParams_t,                   NoAppropriateType_t,                        RF4CE_ZRC2_PairNtfyInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         RF4CE_ZRC2_BINDING_FINISHED_IND_FID,        RF4CE_ZRC2_BindingFinishedNtfyIndParams_t,  NoAppropriateType_t,                        RF4CE_ZRC2_BindingFinishedNtfyInd),
    SERVER_TABLE_ENTRY(IND_RESP,            RF4CE_ZRC2_GET_SHARED_SECRET_IND_FID,       RF4CE_ZRC2_GetSharedSecretIndDescr_t,       RF4CE_ZRC2_GetSharedSecretRespParams_t,     RF4CE_ZRC2_GetSharedSecretInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         RF4CE_ZRC2_IND_GET_RESP_FID,                RF4CE_ZRC2_SetAttributesReqParams_t,        NoAppropriateType_t,                        RF4CE_ZRC2_GetAttrRespInd),
    SERVER_TABLE_ENTRY(IND_NO_RESP,         RF4CE_ZRC2_IND_PUSH_REQ_FID,                RF4CE_ZRC2_SetAttributesReqParams_t,        NoAppropriateType_t,                        RF4CE_ZRC2_PushAttrReqInd),
#   endif /* WRAPPERS_ALL */
#  endif /* MAILBOX_HOST_SIDE */
#   endif /* USE_RF4CE_PROFILE_ZRC2 */

#   ifdef MAILBOX_STACK_SIDE
#    ifdef RF4CE_TARGET
/**********************************************************************************************************************/
/* The new api to set the power filter key                                                                            */
/**********************************************************************************************************************/
    SERVER_TABLE_ENTRY(REQ_CONF,            RF4CE_ZRC_SET_WAKEUP_ACTION_CODE_FID,    RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t,  RF4CE_ZRC_SetWakeUpActionCodeConfParams_t, RF4CE_ZRC_SetWakeUpActionCodeReq),
    SERVER_TABLE_ENTRY(REQ_NO_PARAM_CONF,   RF4CE_ZRC_GET_WAKEUP_ACTION_CODE_FID,    RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t,  RF4CE_ZRC_GetWakeUpActionCodeConfParams_t, RF4CE_ZRC_GetWakeUpActionCodeReq),
#    endif
#   endif

#  endif /* USE_RF4CE_PROFILE_ZRC */

#   if (1 == USE_RF4CE_PROFILE_MSO)
#    ifdef MAILBOX_STACK_SIDE
         SERVER_TABLE_ENTRY(REQ_CONF,           RF4CE_MSO_GET_PROFILE_ATTRIBUTE_FID,         RF4CE_MSO_GetProfileAttributeReqDescr_t,        RF4CE_MSO_GetProfileAttributeConfParams_t, RF4CE_MSO_GetProfileAttributeReq),
         SERVER_TABLE_ENTRY(REQ_CONF,           RF4CE_MSO_SET_PROFILE_ATTRIBUTE_FID,         RF4CE_MSO_SetProfileAttributeReqDescr_t,        RF4CE_MSO_SetProfileAttributeConfParams_t, RF4CE_MSO_SetProfileAttributeReq),
         SERVER_TABLE_ENTRY(REQ_CONF_WITH_DATA, RF4CE_MSO_GET_RIB_ATTRIBUTE_FID,             RF4CE_MSO_GetRIBAttributeReqDescr_t,            RF4CE_MSO_GetRIBAttributeConfParams_t,     RF4CE_MSO_GetRIBAttributeReq),
         SERVER_TABLE_ENTRY(REQ_CONF,           RF4CE_MSO_SET_RIB_ATTRIBUTE_FID,             RF4CE_MSO_SetRIBAttributeReqDescr_t,            RF4CE_MSO_SetRIBAttributeConfParams_t,     RF4CE_MSO_SetRIBAttributeReq),
#     if defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
         SERVER_TABLE_ENTRY(RESP,                RF4CE_MSO_VALIDATE_RESP_FID,                RF4CE_MSO_WatchDogKickOrValidateReqDescr_t,     NoAppropriateType_t,                       RF4CE_MSO_WatchDogKickOrValidateReq),
#     endif /* RF4CE_TARGET */
#     if defined(RF4CE_CONTROLLER) || defined(MAILBOX_UNIT_TEST)
          SERVER_TABLE_ENTRY(REQ_NO_PARAM_CONF, RF4CE_MSO_BIND_FID,                         RF4CE_MSO_BindReqDescr_t,                        RF4CE_MSO_BindConfParams_t,                RF4CE_MSO_BindReq),
          SERVER_TABLE_ENTRY(REQ_CONF,          RF4CE_MSO_USER_CONTROL_PRESSED_FID,         RF4CE_MSO_UserControlReqDescr_t,                 RF4CE_MSO_UserControlConfParams_t,         RF4CE_MSO_UserControlPressedReq),
          SERVER_TABLE_ENTRY(REQ_CONF,          RF4CE_MSO_USER_CONTROL_RELEASED_FID,        RF4CE_MSO_UserControlReqDescr_t,                 RF4CE_MSO_UserControlConfParams_t,         RF4CE_MSO_UserControlReleasedReq),
#     endif /* RF4CE_CONTROLLER */
#    endif /* MAILBOX_STACK_SIDE */
#    ifdef MAILBOX_HOST_SIDE
#     if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_PROFILE_)
#      if defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
          SERVER_TABLE_ENTRY(IND_NO_RESP,       RF4CE_MSO_CHECK_VALIDATION_IND_FID,         RF4CE_MSO_CheckValidationIndParams_t,              NoAppropriateType_t,                       RF4CE_MSO_CheckValidationInd),
          SERVER_TABLE_ENTRY(IND_NO_RESP,       RF4CE_MSO_USER_CONTROL_IND_FID,             RF4CE_MSO_UserControlIndParams_t,                  NoAppropriateType_t,                       RF4CE_MSO_UserControlInd),
#      endif /* RF4CE_TARGET */
         SERVER_TABLE_ENTRY(IND_NO_RESP,        RF4CE_MSO_START_VALIDATION_IND_FID,         RF4CE_PairingReferenceIndParams_t,                 NoAppropriateType_t,                       RF4CE_MSO_StartValidationInd),
#     endif /* WRAPPERS_ALL */
#    endif /* MAILBOX_HOST_SIDE */
#   endif /* USE_RF4CE_PROFILE_MSO */

# endif /* _RF4CE_ */
#endif /* _MAILBOX_WRAPPERS_RF4CE_PROFILE_ */

/**********************************************************************************************************************/
/* NVM API                                                                                                            */
/**********************************************************************************************************************/
#ifdef MAILBOX_HOST_SIDE
    SERVER_TABLE_ENTRY(IND_RESP_WITH_DATA, NVM_READ_FILE_FID,  NVM_ReadFileIndDescr_t,  NVM_ReadFileRespParams_t,  NVM_ReadFileInd),
    SERVER_TABLE_ENTRY(IND_RESP,           NVM_WRITE_FILE_FID, NVM_WriteFileIndDescr_t, NVM_WriteFileRespParams_t, NVM_WriteFileInd),
    SERVER_TABLE_ENTRY(IND_RESP,           NVM_OPEN_FILE_FID,  NVM_OpenFileIndDescr_t,  NVM_OpenFileRespParams_t,  NVM_OpenFileInd),
    SERVER_TABLE_ENTRY(IND_RESP,           NVM_CLOSE_FILE_FID, NVM_CloseFileIndDescr_t, NVM_CloseFileRespParams_t, NVM_CloseFileInd),
#endif /* MAILBOX_HOST_SIDE */

/**********************************************************************************************************************/
/* SYS events                                                                                                         */
/**********************************************************************************************************************/
#ifdef MAILBOX_STACK_SIDE
    SERVER_TABLE_ENTRY(IND_NO_RESP,     SYS_EVENT_SUBSCRIBE_FID,    SYS_EventHandlerMailParams_t,   NoAppropriateType_t, sysEventSubscribeHostHandler),
    SERVER_TABLE_ENTRY(IND_NO_RESP,     SYS_EVENT_RAISE_FID,        SYS_EventNotifyParams_t,        NoAppropriateType_t, SYS_EventRaise),
#endif /* MAILBOX_STACK_SIDE */
#ifdef MAILBOX_HOST_SIDE
    SERVER_TABLE_ENTRY(IND_NO_RESP,     SYS_EVENT_NOTIFY_FID,       SYS_EventNotifyParams_t,        NoAppropriateType_t, SYS_EventNtfy),
#endif /* MAILBOX_HOST_SIDE */

/**********************************************************************************************************************/
/* DirectTV API                                                                                                       */
/**********************************************************************************************************************/
#ifdef MAILBOX_STACK_SIDE
    SERVER_TABLE_ENTRY(REQ_NO_PARAM_CONF, RF4CE_CTRL_TEST_GET_CAPS_FID,              DirectTV_Test_Get_Caps_ReqDescr_t,   DirectTV_Test_Get_Caps_ConfParams_t, DirectTV_Test_Get_Caps_Req),
    SERVER_TABLE_ENTRY(REQ_CONF,          RF4CE_CTRL_TEST_SET_CHANNEL_FID,           DirectTV_Test_Set_Channel_ReqDescr_t,   DirectTV_Test_Set_Channel_ConfParams_t, DirectTV_Test_Set_Channel_Req),
    SERVER_TABLE_ENTRY(REQ_CONF,          RF4CE_CTRL_TEST_CONTINUOUS_WAVE_START_FID, DirectTV_Test_Continuous_Wave_Start_ReqDescr_t,   DirectTV_Test_Continuous_Wave_StartStop_ConfParams_t, DirectTV_Test_Continuous_Wave_Start_Req),
    SERVER_TABLE_ENTRY(REQ_NO_PARAM_CONF, RF4CE_CTRL_TEST_CONTINUOUS_WAVE_STOP_FID,  DirectTV_Test_Continuous_Wave_Stop_ReqDescr_t,   DirectTV_Test_Continuous_Wave_StartStop_ConfParams_t, DirectTV_Test_Continuous_Wave_Stop_Req),
    SERVER_TABLE_ENTRY(REQ_CONF,          RF4CE_CTRL_TEST_TRANSMIT_START_FID,        DirectTV_Test_Transmit_Start_ReqDescr_t,   DirectTV_Test_Transmit_StartStop_ConfParams_t, DirectTV_Test_Transmit_Start_Req),
    SERVER_TABLE_ENTRY(REQ_NO_PARAM_CONF, RF4CE_CTRL_TEST_TRANSMIT_STOP_FID,         DirectTV_Test_Transmit_Stop_ReqDescr_t,   DirectTV_Test_Transmit_StartStop_ConfParams_t, DirectTV_Test_Transmit_Stop_Req),
    SERVER_TABLE_ENTRY(REQ_CONF,          RF4CE_CTRL_TEST_RECEIVE_START_FID,         DirectTV_Test_Receive_Start_ReqDescr_t,   DirectTV_Test_Receive_StartStop_ConfParams_t, DirectTV_Test_Receive_Start_Req),
    SERVER_TABLE_ENTRY(REQ_NO_PARAM_CONF, RF4CE_CTRL_TEST_RECEIVE_STOP_FID,          DirectTV_Test_Receive_Stop_ReqDescr_t,   DirectTV_Test_Receive_StartStop_ConfParams_t, DirectTV_Test_Receive_Stop_Req),
    SERVER_TABLE_ENTRY(REQ_NO_PARAM_CONF, RF4CE_CTRL_TEST_ECHO_START_FID,            DirectTV_Test_Echo_Start_ReqDescr_t,   DirectTV_Test_Echo_StartStop_ConfParams_t, DirectTV_Test_Echo_Start_Req),
    SERVER_TABLE_ENTRY(REQ_NO_PARAM_CONF, RF4CE_CTRL_TEST_ECHO_STOP_FID,             DirectTV_Test_Echo_Stop_ReqDescr_t,   DirectTV_Test_Echo_StartStop_ConfParams_t, DirectTV_Test_Echo_Stop_Req),
    SERVER_TABLE_ENTRY(REQ_CONF_WITH_DATA,RF4CE_CTRL_TEST_ENERGY_DETECT_SCAN_FID,    DirectTV_Test_Energy_Detect_Scan_ReqDescr_t,   DirectTV_Test_Energy_Detect_Scan_ConfParams_t, DirectTV_Test_Energy_Detect_Scan_Req),
    SERVER_TABLE_ENTRY(REQ_NO_PARAM_CONF, RF4CE_CTRL_TEST_GET_STATS_FID,             DirectTV_Test_Get_Stats_ReqDescr_t,   DirectTV_Test_Get_Stats_ConfParams_t, DirectTV_Test_Get_Stats_Req),
    SERVER_TABLE_ENTRY(REQ_NO_PARAM_CONF, RF4CE_CTRL_TEST_RESET_STATS_FID,           DirectTV_Test_Reset_Stats_ReqDescr_t,   DirectTV_Test_Reset_Stats_ConfParams_t, DirectTV_Test_Reset_Stats_Req),
    SERVER_TABLE_ENTRY(REQ_CONF,          RF4CE_CTRL_TEST_SET_TX_POWER_FID,          DirectTV_Test_Set_TX_Power_ReqDescr_t,   DirectTV_Test_Set_TX_Power_ConfParams_t, DirectTV_Test_Set_TX_Power_Req),
#endif
};
/************************* IMPLEMENTATION ***********************************************/
const MailServerParametersTableEntry_t *mailServerTableGetAppropriateEntry(uint16_t id)
{
    const MailServerParametersTableEntry_t *entry = serverTable;
    const MailServerParametersTableEntry_t *tableEnd = serverTable + ARRAY_SIZE(serverTable);

    while (id != entry->id)
        SYS_DbgAssert(tableEnd != entry++, MAILSERVER_GETPUBLICFUNCTIONBYID_0);
    return entry;
}
/* eof bbMailServerTable.c */
