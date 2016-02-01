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
* FILENAME: $Workfile: branches/ext_xhuajun/MailboxIntegration/stack/common/Mailbox/src/bbMailAPI.c $
*
* DESCRIPTION:
*       implementation of wrappers for public functions
*
* $Revision: 3888 $
* $Date: 2014-10-04 00:03:46Z $
*
*****************************************************************************************/
/************************* INCLUDES *****************************************************/
#include "bbMailAPI.h"

/************************* DEFINITIONS **************************************************/
#ifdef _TEST_HARNESS_
/* NOTE: this is a stub for C++ engine (CppUnitTest)*/
#define CREATE_WRAPPER_CLIENT_FUNCTION(name, reqType, fId) \
    void name(reqType *req)                                \
    {                                                      \
        Mail_Serialize(mailDescriptorPtr, fId, req);       \
    }

#else /* STANDARD_DEVICE */
#define CREATE_WRAPPER_CLIENT_FUNCTION(name, reqType, fId) \
    void name(reqType *req)                                \
    {                                                      \
        Mail_Serialize(mailDescriptorPtr, fId, req);       \
    }
#endif /* STANDARD_DEVICE */
/************************* IMPLEMENTATION************************************************/
/**********************************************************************************************************************/
/* Only for unit tests                                                                                                */
/**********************************************************************************************************************/
#ifdef MAILBOX_UNIT_TEST
CREATE_WRAPPER_CLIENT_FUNCTION(MailUnitTest_f1_Call,        MailUnitTest_f1Descr_t,             MAIL_F1_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(MailUnitTest_f2_Call,        MailUnitTest_f2Descr_t,             MAIL_F2_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(MailUnitTest_f3_Call,        MailUnitTest_f3Descr_t,             MAIL_F3_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(MailUnitTest_f4_Call,        MailUnitTest_f4Descr_t,             MAIL_F4_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(MailUnitTest_f5_Call,        MailUnitTest_f5Descr_t,             MAIL_F5_FID);

/**********************************************************************************************************************/
/* Test engine API                                                                                                    */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_TEST_ENGINE_)
# ifdef MAILBOX_HOST_SIDE
    CREATE_WRAPPER_CLIENT_FUNCTION(Mail_TestEnginePing_Call,         TE_PingCommandReqDescr_t,           TE_PING_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(Mail_TestEngineEcho_Call,         TE_EchoCommandReqDescr_t,           TE_ECHO_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(Mail_TestEngineReset_Call,        TE_ResetCommandReqDescr_t,          TE_RESET_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(Mail_SetEchoDelay_Call,           TE_SetEchoDelayCommandReqDescr_t,   TE_ECHO_DELAY_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(Mail_Host2Uart1_Call,             TE_Host2Uart1ReqDescr_t,            TE_HOST_TO_UART1_FID);
# else
    CREATE_WRAPPER_CLIENT_FUNCTION(Mail_TestEngineHelloInd_Call,     TE_HelloCommandIndParams_t,         TE_HELLO_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(Mail_TestEngineAssertLogIdInd_Call, TE_AssertLogIdCommandIndParams_t, TE_ASSERT_LOGID_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(Mail_Uart1ToHostInd_Call,        TE_Uart1ToHostReqParams_t,          TE_UART1_TO_HOST_FID);
# endif
#endif

/* MAC API test */
// CREATE_WRAPPER_CLIENT_FUNCTION(MAC_BanTableSetDefaultAction_Call, MAC_BanTableSetDefaultActionReqDescr_t, MAC_REQ_BAN_SET_DEFAULT_ACTION_FID);
// CREATE_WRAPPER_CLIENT_FUNCTION(MAC_BanTableAddLink_Call,          MAC_BanTableAddLinkReqDescr_t,          MAC_REQ_BAN_LINK_FID);

#ifdef _ZBPRO_
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_DataReq_Call,            MAC_DataReqDescr_t,                     ZBPRO_MAC_REQ_DATA_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_PurgeReq_Call,           MAC_PurgeReqDescr_t,                    ZBPRO_MAC_REQ_PURGE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_AssociateReq_Call,       MAC_AssociateReqDescr_t,                ZBPRO_MAC_REQ_ASSOCIATE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_GetReq_Call,             MAC_GetReqDescr_t,                      ZBPRO_MAC_REQ_GET_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_ResetReq_Call,           MAC_ResetReqDescr_t,                    ZBPRO_MAC_REQ_RESET_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_RxEnableReq_Call,        MAC_RxEnableReqDescr_t,                 ZBPRO_MAC_REQ_RX_ENABLE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_ScanReq_Call,            MAC_ScanReqDescr_t,                     ZBPRO_MAC_REQ_SCAN_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_SetReq_Call,             MAC_SetReqDescr_t,                      ZBPRO_MAC_REQ_SET_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_StartReq_Call,           MAC_StartReqDescr_t,                    ZBPRO_MAC_REQ_START_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_AssociateResp_Call,      MAC_AssociateRespDescr_t,               ZBPRO_MAC_RESP_ASSOCIATE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_OrphanResp_Call,         MAC_OrphanRespDescr_t,                  ZBPRO_MAC_RESP_ORPHAN_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_DataInd_Call,            MAC_DataIndParams_t,                    ZBPRO_MAC_IND_DATA_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_AssociateInd_Call,       MAC_AssociateIndParams_t,               ZBPRO_MAC_IND_ASSOCIATE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_BeaconNotifyInd_Call,    MAC_BeaconNotifyIndParams_t,            ZBPRO_MAC_IND_BEACON_NOTIFY_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_CommStatusInd_Call,      MAC_CommStatusIndParams_t,              ZBPRO_MAC_IND_COMM_STATUS_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_OrphanInd_Call,          MAC_OrphanIndParams_t,                  ZBPRO_MAC_IND_ORPHAN_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_PollInd_Call,            MAC_PollIndParams_t,                    ZBPRO_MAC_IND_POLL_FID);
#endif /* _ZBPRO_ */

#ifdef _RF4CE_
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MAC_ResetReq_Call,           MAC_ResetReqDescr_t,                    RF4CE_MAC_REQ_RESET_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MAC_StartReq_Call,           MAC_StartReqDescr_t,                    RF4CE_MAC_REQ_START_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MAC_DataReq_Call,            MAC_DataReqDescr_t,                     RF4CE_MAC_REQ_DATA_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MAC_GetReq_Call,             MAC_GetReqDescr_t,                      RF4CE_MAC_REQ_GET_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MAC_SetReq_Call,             MAC_SetReqDescr_t,                      RF4CE_MAC_REQ_SET_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MAC_RxEnableReq_Call,        MAC_RxEnableReqDescr_t,                 RF4CE_MAC_REQ_RX_ENABLE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MAC_ScanReq_Call,            MAC_ScanReqDescr_t,                     RF4CE_MAC_REQ_SCAN_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MAC_DataInd_Call,            MAC_DataIndParams_t,                    RF4CE_MAC_IND_DATA_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MAC_BeaconNotifyInd_Call,    MAC_BeaconNotifyIndParams_t,            RF4CE_MAC_IND_BEACON_NOTIFY_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MAC_CommStatusInd_Call,      MAC_CommStatusIndParams_t,              RF4CE_MAC_IND_COMM_STATUS_FID);
#endif /* _RF4CE_ */

/* RF4CE NWK API unit test */
#ifdef _RF4CE_
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_ResetReq_Call,     RF4CE_NWK_ResetReqDescr_t,          RF4CE_NWK_REQ_RESET_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_StartReq_Call,     RF4CE_NWK_StartReqDescr_t,          RF4CE_NWK_REQ_START_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_DataReq_Call,      RF4CE_NWK_DataReqDescr_t,           RF4CE_NWK_REQ_DATA_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_GetReq_Call,       RF4CE_NWK_GetReqDescr_t,            RF4CE_NWK_REQ_GET_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_SetReq_Call,       RF4CE_NWK_SetReqDescr_t,            RF4CE_NWK_REQ_SET_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_DiscoveryReq_Call, RF4CE_NWK_DiscoveryReqDescr_t,      RF4CE_NWK_REQ_DISCOVERY_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_AutoDiscoveryReq_Call, RF4CE_NWK_AutoDiscoveryReqDescr_t, RF4CE_NWK_REQ_AUTODISCOVERY_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_PairReq_Call,      RF4CE_NWK_PairReqDescr_t,           RF4CE_NWK_REQ_PAIR_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_UnpairReq_Call,    RF4CE_NWK_UnpairReqDescr_t,         RF4CE_NWK_REQ_UNPAIR_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_RxEnableReq_Call,  RF4CE_NWK_RXEnableReqDescr_t,       RF4CE_NWK_REQ_RXENABLE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_UpdateKeyReq_Call, RF4CE_NWK_UpdateKeyReqDescr_t,      RF4CE_NWK_REQ_UPDATEKEY_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_DiscoveryResp_Call, RF4CE_NWK_DiscoveryRespDescr_t,    RF4CE_NWK_RESP_DISCOVERY_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_PairResp_Call,     RF4CE_NWK_PairRespDescr_t,          RF4CE_NWK_RESP_PAIR_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_UnpairResp_Call,   RF4CE_NWK_UnpairRespDescr_t,        RF4CE_NWK_RESP_UNPAIR_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_DataInd_Call,      RF4CE_NWK_DataIndParams_t,          RF4CE_NWK_IND_DATA_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_DiscoveryInd_Call, RF4CE_NWK_DiscoveryIndParams_t,     RF4CE_NWK_IND_DISCOVERY_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_COMMStatusInd_Call, RF4CE_NWK_COMMStatusIndParams_t,   RF4CE_NWK_IND_COMMSTATUS_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_PairInd_Call,      RF4CE_NWK_PairIndParams_t,          RF4CE_NWK_IND_PAIR_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_UnpairInd_Call,    RF4CE_NWK_UnpairIndParams_t,        RF4CE_NWK_IND_UNPAIR_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_MacStatsReq_Call,          RF4CE_NWK_MacStatsReqDescr_t,       RF4CE_NWK_MAC_STATS_REQ_FID);
# ifdef ENABLE_GU_KEY_SEED_IND
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_KeySeedStartInd_Call,      RF4CE_NWK_KeySeedStartIndParams_t,  RF4CE_NWK_KEY_SEED_START_IND_FID);
# endif /* ENABLE_GU_KEY_SEED_IND */
#endif /* _RF4CE_ */

#ifdef _ZBPRO_
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_NWK_PermitJoiningReq_Call, ZBPRO_NWK_PermitJoiningReqDescr_t,  ZBPRO_NWK_REQ_PERMIT_JOINING_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_NWK_LeaveReq_Call,             ZBPRO_NWK_LeaveReqDescr_t,          ZBPRO_NWK_REQ_LEAVE_FID);
// CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_NWK_GetReq_Call,               ZBPRO_NWK_GetReqDescr_t,            ZBPRO_NWK_REQ_GET_FID);
// CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_NWK_SetReq_Call,               ZBPRO_NWK_SetReqDescr_t,            ZBPRO_NWK_REQ_SET_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_NWK_GetKeyReq_Call,            ZBPRO_NWK_GetKeyReqDescr_t,         ZBPRO_NWK_REQ_GET_KEY_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_NWK_SetKeyReq_Call,            ZBPRO_NWK_SetKeyReqDescr_t,         ZBPRO_NWK_REQ_SET_KEY_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_NWK_RouteDiscoveryReq_Call,    ZBPRO_NWK_RouteDiscoveryReqDescr_t, ZBPRO_NWK_REQ_ROUTE_DISCOVERY_FID);


CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_EndpointRegisterReq_Call, ZBPRO_APS_EndpointRegisterReqDescr_t, ZBPRO_APS_REQ_ENDPOINTREGISTER_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_EndpointUnregisterReq_Call,ZBPRO_APS_EndpointUnregisterReqDescr_t, ZBPRO_APS_REQ_ENDPOINTUNREGISTER_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_DataReq_Call,          ZBPRO_APS_DataReqDescr_t,           ZBPRO_APS_REQ_DATA_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_BindReq_Call,          ZBPRO_APS_BindUnbindReqDescr_t,     ZBPRO_APS_REQ_BIND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_UnbindReq_Call,        ZBPRO_APS_BindUnbindReqDescr_t,     ZBPRO_APS_REQ_UNBIND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_GetReq_Call,           ZBPRO_APS_GetReqDescr_t,            ZBPRO_APS_REQ_GET_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_SetReq_Call,           ZBPRO_APS_SetReqDescr_t,            ZBPRO_APS_REQ_SET_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_GetKeyReq_Call,            ZBPRO_APS_GetKeyReqDescr_t,         ZBPRO_APS_REQ_GET_KEY_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_SetKeyReq_Call,            ZBPRO_APS_SetKeyReqDescr_t,         ZBPRO_APS_REQ_SET_KEY_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_AddGroupReq_Call,      ZBPRO_APS_AddGroupReqDescr_t,       ZBPRO_APS_REQ_ADDGROUP_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_RemoveGroupReq_Call,   ZBPRO_APS_RemoveGroupReqDescr_t,    ZBPRO_APS_REQ_REMOVEGROUP_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_RemoveAllGroupsReq_Call, ZBPRO_APS_RemoveAllGroupsReqDescr_t, ZBPRO_APS_REQ_REMOVEALLGROUPS_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_TransportKeyReq_Call,  ZBPRO_APS_TransportKeyReqDescr_t,   ZBPRO_APS_REQ_TRANSPORTKEY_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_UpdateDeviceReq_Call,  ZBPRO_APS_UpdateDeviceReqDescr_t,   ZBPRO_APS_REQ_UPDATEDEVICE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_RemoveDeviceReq_Call,  ZBPRO_APS_RemoveDeviceReqDescr_t,   ZBPRO_APS_REQ_REMOTEDEVICE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_RequestKeyReq_Call,    ZBPRO_APS_RequestKeyReqDescr_t,     ZBPRO_APS_REQ_REQUESTKEY_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_SwitchKeyReq_Call,     ZBPRO_APS_SwitchKeyReqDescr_t,      ZBPRO_APS_REQ_SWITCHKEY_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_DataInd_Call,          ZBPRO_APS_DataIndParams_t,          ZBPRO_APS_IND_DATA_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_TransportKeyInd_Call,  ZBPRO_APS_TransportKeyIndParams_t,  ZBPRO_APS_IND_TRANSPORTKEY_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_UpdateDeviceInd_Call,  ZBPRO_APS_UpdateDeviceIndParams_t,  ZBPRO_APS_IND_UPDATEDEVICE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_RemoveDeviceInd_Call,  ZBPRO_APS_RemoveDeviceIndParams_t,  ZBPRO_APS_IND_REMOTEDEVICE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_RequestKeyInd_Call,    ZBPRO_APS_RequestKeyIndParams_t,    ZBPRO_APS_IND_REQUESTKEY_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_SwitchKeyInd_Call,     ZBPRO_APS_SwitchKeyIndParams_t,     ZBPRO_APS_IND_SWITCHKEY_FID);

CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_AddrResolvingReq_Call,         ZBPRO_ZDO_AddrResolvingReqDescr_t,          ZBPRO_ZDO_REQ_ADDR_RESOLVING_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_NodeDescReq_Call,              ZBPRO_ZDO_NodeDescReqDescr_t,               ZBPRO_ZDO_REQ_NODE_DESC_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_PowerDescReq_Call,             ZBPRO_ZDO_PowerDescReqDescr_t,              ZBPRO_ZDO_REQ_POWER_DESC_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_SimpleDescReq_Call,            ZBPRO_ZDO_SimpleDescReqDescr_t,             ZBPRO_ZDO_REQ_SIMPLE_DESC_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_ActiveEpReq_Call,              ZBPRO_ZDO_ActiveEpReqDescr_t,               ZBPRO_ZDO_REQ_ACTIVE_EP_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_MatchDescReq_Call,             ZBPRO_ZDO_MatchDescReqDescr_t,              ZBPRO_ZDO_REQ_MATCH_DESC_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_DeviceAnnceReq_Call,           ZBPRO_ZDO_DeviceAnnceReqDescr_t,            ZBPRO_ZDO_REQ_DEVICE_ANNCE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_EndDeviceBindReq_Call,         ZBPRO_ZDO_EndDeviceBindReqDescr_t,          ZBPRO_ZDO_REQ_ED_BIND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_BindReq_Call,                  ZBPRO_ZDO_BindUnbindReqDescr_t,             ZBPRO_ZDO_REQ_BIND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_UnbindReq_Call,                ZBPRO_ZDO_BindUnbindReqDescr_t,             ZBPRO_ZDO_REQ_UNBIND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_StartNetworkReq_Call,          ZBPRO_ZDO_StartNetworkReqDescr_t,           ZBPRO_ZDO_REQ_START_NETWORK_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_MgmtLeaveReq_Call,             ZBPRO_ZDO_MgmtLeaveReqDescr_t,              ZBPRO_ZDO_REQ_MGMT_LEAVE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_MgmtPermitJoiningReq_Call,     ZBPRO_ZDO_MgmtPermitJoiningReqDescr_t,      ZBPRO_ZDO_REQ_MGMT_PERMIT_JOINING_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_MgmtNwkUpdateReq_Call,         ZBPRO_ZDO_MgmtNwkUpdateReqDescr_t,          ZBPRO_ZDO_REQ_MGMT_NWK_UPDATE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_MgmtNwkUpdateUnsolResp_Call,   ZBPRO_ZDO_MgmtNwkUpdateUnsolRespDescr_t,    ZBPRO_ZDO_RESP_MGMT_NWK_UPDATE_UNSOL_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_MgmtNwkUpdateUnsolInd_Call,    ZBPRO_ZDO_MgmtNwkUpdateUnsolIndParams_t,    ZBPRO_ZDO_IND_MGMT_NWK_UPDATE_UNSOL_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_MgmtLqiReq_Call,               ZBPRO_ZDO_MgmtLqiReqDescr_t,                ZBPRO_ZDO_REQ_MGMT_LQI_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_MgmtBindReq_Call,              ZBPRO_ZDO_MgmtBindReqDescr_t,               ZBPRO_ZDO_REQ_MGMT_BIND_FID);

CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_TC_NwkKeyUpdateReq_Call,       ZBPRO_TC_NwkKeyUpdateReqDescr_t,    ZBPRO_TC_REQ_NWK_KEY_UPDATE_FID);

CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SetPowerSourceReq_Call,                        ZBPRO_ZCL_SetPowerSourceReqDescr_t,                         ZBPRO_ZCL_REQ_SET_POWER_SOURCE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReq_Call,            ZBPRO_ZCL_ProfileWideCmdDiscoverAttrDescr_t,                ZBPRO_ZCL_PROFILE_WIDE_CMD_DISCOVER_ATTRIBUTE_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ProfileWideCmdReadAttributesReq_Call,          ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t,                 ZBPRO_ZCL_PROFILE_WIDE_CMD_READ_ATTRIBUTE_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ProfileWideCmdWriteAttributesReq_Call,         ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t,                ZBPRO_ZCL_PROFILE_WIDE_CMD_WRITE_ATTRIBUTE_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ProfileWideCmdConfigureReportingReq_Call,      ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t, ZBPRO_ZCL_PROFILE_WIDE_CMD_CONFIGURE_REPORTING_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReq_Call,    ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqDescr_t, ZBPRO_ZCL_PROFILE_WIDE_CMD_READ_REPORTING_CONFIGURATION_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ProfileWideCmdReportAttributesInd_Call,  ZBPRO_ZCL_ProfileWideCmdReportAttributesIndParams_t,     ZBPRO_ZCL_PROFILE_WIDE_CMD_REPORT_ATTRIBUTES_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IdentifyCmdIdentifyReq_Call,                   ZBPRO_ZCL_IdentifyCmdIdentifyReqDescr_t,                    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IdentifyCmdIdentifyQueryReq_Call,              ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqDescr_t,               ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseIndEB_Call,    ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseIndParams_t,      ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_RESPONSE_IND_FID);
# if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_IDENTIFY_RELAY_)
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IdentifyCmdIdentifyInd_Call,                   ZBPRO_ZCL_IdentifyCmdIdentifyIndParams_t,                   ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IdentifyCmdIdentifyResponseReq_Call,           ZBPRO_ZCL_IdentifyCmdIdentifyResponseReqDescr_t,            ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_RESPONSE_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IdentifyCmdIdentifyQueryInd_Call,              ZBPRO_ZCL_IdentifyCmdIdentifyQueryIndParams_t,              ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReq_Call,      ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReqDescr_t,       ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_RESPONSE_REQ_FID);
# else
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IndentifyInd_Call,                             ZBPRO_ZCL_IdentifyIndParams_t,                              ZBPRO_ZCL_IND_IDENTIFY_FID);
# endif
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_GroupsCmdAddGroupReq_Call,                     ZBPRO_ZCL_GroupsCmdAddGroupReqDescr_t,                      ZBPRO_ZCL_GROUPS_CMD_ADD_GROUP_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_GroupsCmdViewGroupReq_Call,                    ZBPRO_ZCL_GroupsCmdViewGroupReqDescr_t,                     ZBPRO_ZCL_GROUPS_CMD_VIEW_GROUP_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_GroupsCmdGetGroupMembershipReq_Call,           ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqDescr_t,            ZBPRO_ZCL_GROUPS_CMD_GET_GROUP_MEMBERSHIP_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_GroupsCmdGetGroupMembershipResponseInd_Call,   ZBPRO_ZCL_GroupsCmdGetGroupMembershipIndParams_t,           ZBPRO_ZCL_GROUPS_CMD_GET_GROUP_MEMBERSHIP_RESPONSE_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_GroupsCmdRemoveGroupReq_Call,                  ZBPRO_ZCL_GroupsCmdRemoveGroupReqDescr_t,                   ZBPRO_ZCL_GROUPS_CMD_REMOVE_GROUP_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReq_Call,              ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqDescr_t,               ZBPRO_ZCL_GROUPS_CMD_REMOVE_ALL_GROUPS_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReq_Call,           ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqDescr_t,            ZBPRO_ZCL_GROUPS_CMD_ADD_GROUP_IF_IDENTIFY_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ScenesCmdAddSceneReq_Call,                     ZBPRO_ZCL_ScenesCmdAddSceneReqDescr_t,                      ZBPRO_ZCL_SCENES_CMD_ADD_SCENE_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ScenesCmdViewSceneReq_Call,                    ZBPRO_ZCL_ScenesCmdViewSceneReqDescr_t,                     ZBPRO_ZCL_SCENES_CMD_VIEW_SCENE_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ScenesCmdStoreSceneReq_Call,                   ZBPRO_ZCL_ScenesCmdStoreSceneReqDescr_t,                    ZBPRO_ZCL_SCENES_CMD_STORE_SCENE_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ScenesCmdRecallSceneReq_Call,                  ZBPRO_ZCL_ScenesCmdRecallSceneReqDescr_t,                   ZBPRO_ZCL_SCENES_CMD_RECALL_SCENE_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ScenesCmdRemoveSceneReq_Call,                  ZBPRO_ZCL_ScenesCmdRemoveSceneReqDescr_t,                   ZBPRO_ZCL_SCENES_CMD_REMOVE_SCENE_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ScenesCmdRemoveAllScenesReq_Call,              ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqDescr_t,               ZBPRO_ZCL_SCENES_CMD_REMOVE_ALL_SCENES_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ScenesCmdGetSceneMembershipReq_Call,           ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqDescr_t,            ZBPRO_ZCL_SCENES_CMD_GET_SCENE_MEMBERSHIP_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseInd_Call,   ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseIndParams_t,   ZBPRO_ZCL_SCENES_CMD_GET_SCENE_MEMBERSHIP_RESPONSE_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_OnOffCmdOffReq_Call,                           ZBPRO_ZCL_OnOffCmdReqDescr_t,                               ZBPRO_ZCL_ONOFF_CMD_OFF_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_OnOffCmdOnReq_Call,                            ZBPRO_ZCL_OnOffCmdReqDescr_t,                               ZBPRO_ZCL_ONOFF_CMD_ON_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_OnOffCmdToggleReq_Call,                        ZBPRO_ZCL_OnOffCmdReqDescr_t,                               ZBPRO_ZCL_ONOFF_CMD_TOGGLE_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_LevelControlCmdMoveToLevelReq_Call,            ZBPRO_ZCL_LevelControlCmdMoveToLevelReqDescr_t,             ZBPRO_ZCL_LEVEL_CONTROL_CMD_MOVE_TO_LEVEL_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_LevelControlCmdMoveReq_Call,                   ZBPRO_ZCL_LevelControlCmdMoveReqDescr_t,                    ZBPRO_ZCL_LEVEL_CONTROL_CMD_MOVE_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_LevelControlCmdStepReq_Call,                   ZBPRO_ZCL_LevelControlCmdStepReqDescr_t,                    ZBPRO_ZCL_LEVEL_CONTROL_CMD_STEP_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_LevelControlCmdStopReq_Call,                   ZBPRO_ZCL_LevelControlCmdStopReqDescr_t,                    ZBPRO_ZCL_LEVEL_CONTROL_CMD_STOP_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_DoorLockCmdLockReq_Call,                       ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t,                  ZBPRO_ZCL_DOOR_LOCK_CMD_LOCK_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_DoorLockCmdUnlockReq_Call,                     ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t,                  ZBPRO_ZCL_DOOR_LOCK_CMD_UNLOCK_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_WindowCoveringCmdUpOpenReq_Call,               ZBPRO_ZCL_WindowCoveringCmdReqDescr_t,                      ZBPRO_ZCL_WINDOW_COVERING_CMD_UP_OPEN_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_WindowCoveringCmdDownCloseReq_Call,            ZBPRO_ZCL_WindowCoveringCmdReqDescr_t,                      ZBPRO_ZCL_WINDOW_COVERING_CMD_DOWN_CLOSE_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_WindowCoveringCmdStopReq_Call,                 ZBPRO_ZCL_WindowCoveringCmdReqDescr_t,                      ZBPRO_ZCL_WINDOW_COVERING_CMD_STOP_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_WindowCoveringCmdGotoLiftPecentageReq_Call,    ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t,       ZBPRO_ZCL_WINDOW_COVERING_CMD_GO_TO_LIFT_PERCENTAGE_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_WindowCoveringCmdGotoTiltPecentageReq_Call,    ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t,       ZBPRO_ZCL_WINDOW_COVERING_CMD_GO_TO_TILT_PERCENTAGE_REQ_FID);
// CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestInd_Call,           ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestIndParams_t,           ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_ENROLL_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationInd_Call, ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationIndParams_t, ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_STATUS_CHANGED_NOTIFICATION_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReq_Call,          ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqDescr_t,           ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_ENROLL_RESPONSE_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceArmInd_Call,                          ZBPRO_ZCL_SapIasAceArmIndParams_t,                          ZBPRO_ZCL_IAS_ACE_ARM_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceBypassInd_Call,                       ZBPRO_ZCL_SapIasAceBypassIndParams_t,                       ZBPRO_ZCL_IAS_ACE_BYPASS_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceEmergencyInd_Call,                    ZBPRO_ZCL_SapIasAceAlarmIndParams_t,                        ZBPRO_ZCL_IAS_ACE_EMERGENCY_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceFireInd_Call,                         ZBPRO_ZCL_SapIasAceAlarmIndParams_t,                        ZBPRO_ZCL_IAS_ACE_FIRE_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAcePanicInd_Call,                        ZBPRO_ZCL_SapIasAceAlarmIndParams_t,                        ZBPRO_ZCL_IAS_ACE_PANIC_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneIdMapInd_Call,                 ZBPRO_ZCL_SapIasAceGetZoneIdMapIndParams_t,                 ZBPRO_ZCL_IAS_ACE_GET_ZONE_ID_MAP_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneInfoInd_Call,                  ZBPRO_ZCL_SapIasAceGetZoneInfoIndParams_t,                  ZBPRO_ZCL_IAS_ACE_GET_ZONE_INFO_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceGetPanelStatusInd_Call,               ZBPRO_ZCL_SapIasAceGetPanelStatusIndParams_t,               ZBPRO_ZCL_IAS_ACE_GET_PANEL_STATUS_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceGetBypassedZoneListInd_Call,          ZBPRO_ZCL_SapIasAceGetBypassedZoneListIndParams_t,          ZBPRO_ZCL_IAS_ACE_GET_BYPASSED_ZONE_LIST_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneStatusInd_Call,                ZBPRO_ZCL_SapIasAceGetZoneStatusIndParams_t,                ZBPRO_ZCL_IAS_ACE_GET_ZONE_STATUS_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceArmRespReq_Call,                      ZBPRO_ZCL_SapIasAceArmRespReqDescr_t,                       ZBPRO_ZCL_IAS_ACE_ARM_RESP_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceBypassRespReq_Call,                   ZBPRO_ZCL_SapIasAceBypassRespReqDescr_t,                    ZBPRO_ZCL_IAS_ACE_BYPASS_RESP_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReq_Call,             ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqDescr_t,              ZBPRO_ZCL_IAS_ACE_GET_ZONE_ID_MAP_RESP_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneInfoRespReq_Call,              ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqDescr_t,               ZBPRO_ZCL_IAS_ACE_GET_ZONE_INFO_RESP_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceGetPanelStatusRespReq_Call,           ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqDescr_t,            ZBPRO_ZCL_IAS_ACE_GET_PANEL_STATUS_RESP_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReq_Call,      ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqDescr_t,       ZBPRO_ZCL_IAS_ACE_SET_BYPASSED_ZONE_LIST_RESP_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneStatusRespReq_Call,            ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqDescr_t,             ZBPRO_ZCL_IAS_ACE_GET_ZONE_STATUS_RESP_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceZoneStatusChangedReq_Call,            ZBPRO_ZCL_SapIasAceZoneStatusChangedReqDescr_t,             ZBPRO_ZCL_IAS_ACE_ZONE_STATUS_CHANGED_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAcePanelStatusChangedReq_Call,           ZBPRO_ZCL_SapIasAcePanelStatusChangedReqDescr_t,            ZBPRO_ZCL_IAS_ACE_PANEL_STATUS_CHANGED_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IASWDCmdStartWarningReq_Call,                  ZBPRO_ZCL_IASWDCmdStartWarningReqDescr_t,                   ZBPRO_ZCL_IAS_WD_CMD_START_WARNING_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IASWDCmdSquawkgReq_Call,                       ZBPRO_ZCL_IASWDCmdSquawkReqDescr_t,                         ZBPRO_ZCL_IAS_WD_CMD_SQUAWK_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ColorControlCmdMoveToColorReq_Call,            ZBPRO_ZCL_ColorControlCmdMoveToColorReqDescr_t,         ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_TO_COLOR_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ColorControlCmdMoveColorReq_Call,              ZBPRO_ZCL_ColorControlCmdMoveColorReqDescr_t,           ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_COLOR_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ColorControlCmdStepColorReq_Call,              ZBPRO_ZCL_ColorControlCmdStepColorReqDescr_t,           ZBPRO_ZCL_COLOR_CONTROL_CMD_STEP_COLOR_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReq_Call,      ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqDescr_t,   ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_TO_HUE_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReq_Call,        ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqDescr_t,     ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_HUE_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReq_Call,        ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqDescr_t,     ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_STEP_HUE_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReq_Call,      ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqDescr_t, ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_TO_HUE_AND_SATURATION_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ColorControlCmdColorLoopSetReq_Call,           ZBPRO_ZCL_ColorControlCmdColorLoopSetReqDescr_t,        ZBPRO_ZCL_COLOR_CONTROL_CMD_COLOR_LOOP_SET_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ColorControlCmdStopMoveStepReq_Call,           ZBPRO_ZCL_ColorControlCmdStopMoveStepReqDescr_t,        ZBPRO_ZCL_COLOR_CONTROL_CMD_STOP_MOVE_STEP_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReq_Call,   ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqDescr_t,    ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_COLOR_TEMPERATURE_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReq_Call,   ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqDescr_t,    ZBPRO_ZCL_COLOR_CONTROL_CMD_STEP_COLOR_TEMPERATURE_REQ_FID);


CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZHA_EzModeReq_Call,                ZBPRO_ZHA_EzModeReqDescr_t,             ZBPRO_ZHA_EZ_MODE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZHA_CieDeviceEnrollReq_Call,       ZBPRO_ZHA_CieEnrollReqDescr_t,          ZBPRO_ZHA_CIE_ENROLL_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZHA_CieDeviceSetPanelStatusReq_Call,    ZBPRO_ZHA_CieSetPanelStatusReqDescr_t,          ZBPRO_ZHA_CIE_SET_PANEL_STATUS_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZHA_CieZoneSetBypassStateReq_Call,    ZBPRO_ZHA_CieZoneSetBypassStateReqDescr_t,        ZBPRO_ZHA_CIE_ZONE_SET_BYPASS_STATE_REQ_FID);

#endif /* _ZBPRO_ */

CREATE_WRAPPER_CLIENT_FUNCTION(SYS_PrintInd_Call,                       SYS_PrintIndParams_t,  SYS_PRINT_FID);

#ifdef _RF4CE_
/* RF4CE Profile manager API unit test */
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_UnpairReq_Call,        RF4CE_UnpairReqDescr_t,             RF4CE_PROFILE_REQ_UNPAIR_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_StartReq_Call,         RF4CE_StartReqDescr_t,              RF4CE_PROFILE_REQ_START_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ResetReq_Call,         RF4CE_ResetReqDescr_t,              RF4CE_PROFILE_REQ_RESET_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_SetSupportedDevicesReq_Call, RF4CE_SetSupportedDevicesReqDescr_t, RF4CE_PROFILE_REQ_SET_SUP_DEVICES_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_CounterExpiredInd_Call, RF4CE_PairingReferenceIndParams_t, RF4CE_PROFILE_IND_COUNTER_EXPIRED_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_UnpairInd_Call,        RF4CE_PairingReferenceIndParams_t,  RF4CE_PROFILE_IND_UNPAIR_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_PairInd_Call,                  RF4CE_PairingIndParams_t,           RF4CE_PROFILE_IND_PAIR_FID);

# if defined(RF4CE_ZRC_WAKEUP_ACTION_CODE_SUPPORT)
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC_SetWakeUpActionCodeReq_Call, RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t, RF4CE_ZRC_SET_WAKEUP_ACTION_CODE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC_GetWakeUpActionCodeReq_Call, RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t, RF4CE_ZRC_GET_WAKEUP_ACTION_CODE_FID);
# endif

/* RF4CE ZRC 1.1 Profile API unit test */
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC1_GetAttributesReq_Call,        RF4CE_ZRC1_GetAttributeDescr_t,             RF4CE_ZRC1_REQ_GET_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC1_SetAttributesReq_Call,        RF4CE_ZRC1_SetAttributeDescr_t,             RF4CE_ZRC1_REQ_SET_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC1_ControllerBindReq_Call,       RF4CE_ZRC1_BindReqDescr_t,                  RF4CE_ZRC1_REQ_CONTROLLER_BIND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC1_TargetBindReq_Call,           RF4CE_ZRC1_BindReqDescr_t,                  RF4CE_ZRC1_REQ_TARGET_BIND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC1_CommandDiscoveryReq_Call,     RF4CE_ZRC1_CommandDiscoveryReqDescr_t,  RF4CE_ZRC1_REQ_COMMANDDISCOVERY_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC1_ControlCommandPressedReq_Call,     RF4CE_ZRC1_ControlCommandReqDescr_t,  RF4CE_ZRC1_CONTROL_COMMAND_PRESSED_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC1_ControlCommandReleasedReq_Call,     RF4CE_ZRC1_ControlCommandReqDescr_t,  RF4CE_ZRC1_CONTROL_COMMAND_RELEASED_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC1_ControlCommandInd_Call,             RF4CE_ZRC1_ControlCommandIndParams_t,         RF4CE_ZRC1_IND_CONTROLCOMMAND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC1_VendorSpecificReq_Call,     RF4CE_ZRC1_VendorSpecificReqDescr_t,          RF4CE_ZRC1_VENDORSPECIFIC_REQ_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC1_VendorSpecificInd_Call,     RF4CE_ZRC1_VendorSpecificIndParams_t,         RF4CE_ZRC1_VENDORSPECIFIC_IND_FID);

/* RF4CE GDP 2.0 & ZRC 2.0 Profile API unit test */
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_GetAttributesReq_Call,        RF4CE_ZRC2_GetAttributesReqDescr_t,         RF4CE_ZRC2_REQ_GET_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_SetAttributesReq_Call,        RF4CE_ZRC2_SetAttributesReqDescr_t,         RF4CE_ZRC2_REQ_SET_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_KeyExchangeReq_Call,          RF4CE_ZRC2_KeyExchangeReqDescr_t,           RF4CE_ZRC2_KEY_EXCHANGE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_BindReq_Call,                 RF4CE_ZRC2_BindReqDescr_t,                  RF4CE_ZRC2_BIND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_ProxyBindReq_Call,            RF4CE_ZRC2_ProxyBindReqDescr_t,             RF4CE_ZRC2_PROXY_BIND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_EnableBindingReq_Call,        RF4CE_ZRC2_BindingReqDescr_t,               RF4CE_ZRC2_ENABLE_BINDING_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_DisableBindingReq_Call,       RF4CE_ZRC2_BindingReqDescr_t,               RF4CE_ZRC2_DISABLE_BINDING_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_SetPushButtonStimulusReq_Call, RF4CE_ZRC2_ButtonBindingReqDescr_t,         RF4CE_ZRC2_SET_PUSH_BUTTON_STIMULUS_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_ClearPushButtonStimulusReq_Call, RF4CE_ZRC2_ButtonBindingReqDescr_t,         RF4CE_ZRC2_CLEAR_PUSH_BUTTON_STIMULUS_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_CheckValidationResp_Call,     RF4CE_ZRC2_CheckValidationRespDescr_t,      RF4CE_ZRC2_CHECK_VALIDATION_RESP_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_StartValidationInd_Call,      RF4CE_ZRC2_CheckValidationIndParams_t,      RF4CE_ZRC2_START_VALIDATION_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_CheckValidationInd_Call,      RF4CE_ZRC2_CheckValidationIndParams_t,      RF4CE_ZRC2_CHECK_VALIDATION_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_ControlCommandInd_Call,       RF4CE_ZRC2_ControlCommandIndParams_t,       RF4CE_ZRC2_CONTROL_COMMAND_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_GDP2_SetPollConstraintsReq_Call,   RF4CE_GDP2_SetPollConstraintsReqDescr_t,    RF4CE_GDP2_REQ_SET_POLL_CONSTRAINTS_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_GDP2_PollNegotiationReq_Call,      RF4CE_GDP2_PollNegotiationReqDescr_t,       RF4CE_GDP2_REQ_POLL_NEGOTIATION_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_GDP2_PollClientUserEventReq_Call,  RF4CE_GDP2_PollClientUserEventReqDescr_t,   RF4CE_GDP2_REQ_POLL_CLIENT_USER_EVENT_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_GDP2_ClientNotificationReq_Call,   RF4CE_GDP2_ClientNotificationReqDescr_t,    RF4CE_GDP2_REQ_CLIENT_NOTIFICATION_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_GDP2_IdentifyCapAnnounceReq_Call,  RF4CE_GDP2_IdentifyCapAnnounceReqDescr_t,   RF4CE_GDP2_REQ_IDENTIFY_CAP_ANNOUNCE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_GDP2_IdentifyReq_Call,             RF4CE_GDP2_IdentifyReqDescr_t,              RF4CE_GDP2_REQ_IDENTIFY_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_GDP2_PollNegotiationInd_Call,      RF4CE_GDP2_PollNegotiationIndParams_t,      RF4CE_GDP2_IND_POLL_NEGOTIATION_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_GDP2_HeartbeatInd_Call,            RF4CE_GDP2_HeartbeatIndParams_t,            RF4CE_GDP2_IND_HEARTBEAT_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_GDP2_ClientNotificationInd_Call,   RF4CE_GDP2_ClientNotificationIndParams_t,   RF4CE_GDP2_IND_CLIENT_NOTIFICATION_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_GDP2_IdentifyCapAnnounceInd_Call,  RF4CE_GDP2_IdentifyCapAnnounceIndParams_t,  RF4CE_GDP2_IND_IDENTIFY_CAP_ANNOUNCE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_GDP2_IdentifyInd_Call,             RF4CE_GDP2_IdentifyIndParams_t,             RF4CE_GDP2_IND_IDENTIFY_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_GetSharedSecretInd_Call,      RF4CE_ZRC2_GetSharedSecretIndDescr_t,       RF4CE_ZRC2_GET_SHARED_SECRET_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_PairNtfyInd_Call,             RF4CE_PairingIndParams_t,                   RF4CE_ZRC2_PAIR_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_BindingFinishedNtfyInd_Call,  RF4CE_ZRC2_BindingFinishedNtfyIndParams_t,  RF4CE_ZRC2_BINDING_FINISHED_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_GetAttrRespInd_Call,          RF4CE_ZRC2_SetAttributesReqParams_t,        RF4CE_ZRC2_IND_GET_RESP_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_PushAttrReqInd_Call,          RF4CE_ZRC2_SetAttributesReqParams_t,        RF4CE_ZRC2_IND_PUSH_REQ_FID);

/* RF4CE MSO Profile API unit test */
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MSO_GetProfileAttributeReq_Call,   RF4CE_MSO_GetProfileAttributeReqDescr_t,    RF4CE_MSO_GET_PROFILE_ATTRIBUTE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MSO_SetProfileAttributeReq_Call,   RF4CE_MSO_SetProfileAttributeReqDescr_t,    RF4CE_MSO_SET_PROFILE_ATTRIBUTE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MSO_GetRIBAttributeReq_Call,       RF4CE_MSO_GetRIBAttributeReqDescr_t,        RF4CE_MSO_GET_RIB_ATTRIBUTE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MSO_SetRIBAttributeReq_Call,       RF4CE_MSO_SetRIBAttributeReqDescr_t,        RF4CE_MSO_SET_RIB_ATTRIBUTE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MSO_BindReq_Call,                  RF4CE_MSO_BindReqDescr_t,                   RF4CE_MSO_BIND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MSO_UserControlPressedReq_Call,    RF4CE_MSO_UserControlReqDescr_t,            RF4CE_MSO_USER_CONTROL_PRESSED_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MSO_UserControlReleasedReq_Call,   RF4CE_MSO_UserControlReqDescr_t,            RF4CE_MSO_USER_CONTROL_RELEASED_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MSO_StartValidationInd_Call,       RF4CE_PairingReferenceIndParams_t,          RF4CE_MSO_START_VALIDATION_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MSO_CheckValidationInd_Call,       RF4CE_MSO_CheckValidationIndParams_t,       RF4CE_MSO_CHECK_VALIDATION_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MSO_UserControlInd_Call,           RF4CE_MSO_UserControlIndParams_t,           RF4CE_MSO_USER_CONTROL_IND_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MSO_WatchDogKickOrValidateReq_Call,RF4CE_MSO_WatchDogKickOrValidateReqDescr_t, RF4CE_MSO_VALIDATE_RESP_FID);

#endif /* _RF4CE_ */

CREATE_WRAPPER_CLIENT_FUNCTION(NVM_ReadFileInd_Call, NVM_ReadFileIndDescr_t, NVM_READ_FILE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(NVM_OpenFileInd_Call, NVM_OpenFileIndDescr_t, NVM_OPEN_FILE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(NVM_WriteFileInd_Call, NVM_WriteFileIndDescr_t, NVM_WRITE_FILE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(NVM_CloseFileInd_Call, NVM_CloseFileIndDescr_t, NVM_CLOSE_FILE_FID);

CREATE_WRAPPER_CLIENT_FUNCTION(SYS_EventSubscribe_Call,  SYS_EventHandlerParams_t,      SYS_EVENT_SUBSCRIBE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(SYS_EventRaise_Call,                 SYS_EventNotifyParams_t,            SYS_EVENT_RAISE_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(SYS_EventNtfy_Call,       SYS_EventNotifyParams_t,       SYS_EVENT_NOTIFY_FID);

CREATE_WRAPPER_CLIENT_FUNCTION(DirectTV_Test_Get_Caps_Req_Call,              DirectTV_Test_Get_Caps_ReqDescr_t,   RF4CE_CTRL_TEST_GET_CAPS_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(DirectTV_Test_Set_Channel_Req_Call,           DirectTV_Test_Set_Channel_ReqDescr_t,   RF4CE_CTRL_TEST_SET_CHANNEL_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(DirectTV_Test_Continuous_Wave_Start_Req_Call, DirectTV_Test_Continuous_Wave_Start_ReqDescr_t,   RF4CE_CTRL_TEST_CONTINUOUS_WAVE_START_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(DirectTV_Test_Continuous_Wave_Stop_Req_Call,  DirectTV_Test_Continuous_Wave_Stop_ReqDescr_t,   RF4CE_CTRL_TEST_CONTINUOUS_WAVE_STOP_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(DirectTV_Test_Transmit_Start_Req_Call,        DirectTV_Test_Transmit_Start_ReqDescr_t,   RF4CE_CTRL_TEST_TRANSMIT_START_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(DirectTV_Test_Transmit_Stop_Req_Call,         DirectTV_Test_Transmit_Stop_ReqDescr_t,   RF4CE_CTRL_TEST_TRANSMIT_STOP_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(DirectTV_Test_Receive_Start_Req_Call,         DirectTV_Test_Receive_Start_ReqDescr_t,   RF4CE_CTRL_TEST_RECEIVE_START_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(DirectTV_Test_Receive_Stop_Req_Call,          DirectTV_Test_Receive_Stop_ReqDescr_t,   RF4CE_CTRL_TEST_RECEIVE_STOP_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(DirectTV_Test_Echo_Start_Req_Call,            DirectTV_Test_Echo_Start_ReqDescr_t,   RF4CE_CTRL_TEST_ECHO_START_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(DirectTV_Test_Echo_Stop_Req_Call,             DirectTV_Test_Echo_Stop_ReqDescr_t,   RF4CE_CTRL_TEST_ECHO_STOP_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(DirectTV_Test_Energy_Detect_Scan_Req_Call,    DirectTV_Test_Energy_Detect_Scan_ReqDescr_t,   RF4CE_CTRL_TEST_ENERGY_DETECT_SCAN_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(DirectTV_Test_Get_Stats_Req_Call,             DirectTV_Test_Get_Stats_ReqDescr_t,   RF4CE_CTRL_TEST_GET_STATS_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(DirectTV_Test_Reset_Stats_Req_Call,           DirectTV_Test_Reset_Stats_ReqDescr_t,   RF4CE_CTRL_TEST_RESET_STATS_FID);
CREATE_WRAPPER_CLIENT_FUNCTION(DirectTV_Test_Set_TX_Power_Req_Call,          DirectTV_Test_Set_TX_Power_ReqDescr_t,   RF4CE_CTRL_TEST_SET_TX_POWER_FID);
#else /* NOT MAILBOX_UNIT_TEST */

/**********************************************************************************************************************/
/* Test engine API                                                                                                    */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_TEST_ENGINE_)
# ifdef MAILBOX_HOST_SIDE
    CREATE_WRAPPER_CLIENT_FUNCTION(Mail_TestEnginePing,         TE_PingCommandReqDescr_t,           TE_PING_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(Mail_TestEngineEcho,         TE_EchoCommandReqDescr_t,           TE_ECHO_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(Mail_TestEngineReset,        TE_ResetCommandReqDescr_t,          TE_RESET_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(Mail_SetEchoDelay,           TE_SetEchoDelayCommandReqDescr_t,   TE_ECHO_DELAY_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(Mail_Host2Uart1,             TE_Host2Uart1ReqDescr_t,            TE_HOST_TO_UART1_FID);
# else
    CREATE_WRAPPER_CLIENT_FUNCTION(Mail_TestEngineHelloInd,     TE_HelloCommandIndParams_t,         TE_HELLO_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(Mail_TestEngineAssertLogIdInd, TE_AssertLogIdCommandIndParams_t, TE_ASSERT_LOGID_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(Mail_Uart1ToHostInd_Call,    TE_Uart1ToHostReqParams_t,          TE_UART1_TO_HOST_FID);
# endif
#endif

/**********************************************************************************************************************/
/* Profiling Engine API                                                                                           */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_PROFILING_ENGINE_)
# ifdef MAILBOX_HOST_SIDE
    CREATE_WRAPPER_CLIENT_FUNCTION(SYS_DbgPeResetReq,           SYS_DbgPeResetReqDescr_t,           PE_RESET_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(SYS_DbgPeGetDataReq,         SYS_DbgPeGetDataReqDescr_t,         PE_GETDATA_FID);
# endif
#endif

/**********************************************************************************************************************/
/* MAC API                                                                                                            */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_MAC_)
# ifdef MAILBOX_HOST_SIDE
#  ifdef _MAC_BAN_TABLE_
    CREATE_WRAPPER_CLIENT_FUNCTION(MAC_BanTableSetDefaultAction, MAC_BanTableSetDefaultActionReqDescr_t, MAC_REQ_BAN_SET_DEFAULT_ACTION_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(MAC_BanTableAddLink,          MAC_BanTableAddLinkReqDescr_t,          MAC_REQ_BAN_LINK_FID);
#  endif /* _MAC_BAN_TABLE_ */
# endif /* MAILBOX_HOST_SIDE */
/**** ZBPRO *****************************************************************************/
# ifdef _ZBPRO_
#  ifdef MAILBOX_HOST_SIDE
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_GetReq,            MAC_GetReqDescr_t,                      ZBPRO_MAC_REQ_GET_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_SetReq,            MAC_SetReqDescr_t,                      ZBPRO_MAC_REQ_SET_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_DataReq,            MAC_DataReqDescr_t,                     ZBPRO_MAC_REQ_DATA_FID);
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_MAC_)
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_PurgeReq,          MAC_PurgeReqDescr_t,                    ZBPRO_MAC_REQ_PURGE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_AssociateReq,      MAC_AssociateReqDescr_t,                ZBPRO_MAC_REQ_ASSOCIATE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_ResetReq,          MAC_ResetReqDescr_t,                    ZBPRO_MAC_REQ_RESET_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_RxEnableReq,       MAC_RxEnableReqDescr_t,                 ZBPRO_MAC_REQ_RX_ENABLE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_ScanReq,           MAC_ScanReqDescr_t,                     ZBPRO_MAC_REQ_SCAN_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_StartReq,          MAC_StartReqDescr_t,                    ZBPRO_MAC_REQ_START_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_AssociateResp,     MAC_AssociateRespDescr_t,               ZBPRO_MAC_RESP_ASSOCIATE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_OrphanResp,        MAC_OrphanRespDescr_t,                  ZBPRO_MAC_RESP_ORPHAN_FID);
#   endif /* WRAPPERS_ALL  */
#  else /* ! MAILBOX_HOST_SIDE */
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_MAC_)
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_DataInd,            MAC_DataIndParams_t,                    ZBPRO_MAC_IND_DATA_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_AssociateInd,       MAC_AssociateIndParams_t,               ZBPRO_MAC_IND_ASSOCIATE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_BeaconNotifyInd,    MAC_BeaconNotifyIndParams_t,            ZBPRO_MAC_IND_BEACON_NOTIFY_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_CommStatusInd,      MAC_CommStatusIndParams_t,              ZBPRO_MAC_IND_COMM_STATUS_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_OrphanInd,          MAC_OrphanIndParams_t,                  ZBPRO_MAC_IND_ORPHAN_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_MAC_PollInd,            MAC_PollIndParams_t,                    ZBPRO_MAC_IND_POLL_FID);
#   endif /* WRAPPERS_ALL */
#  endif /* ! MAILBOX_HOST_SIDE */
# endif /* _ZBPRO_ */

/**** RF4CE *****************************************************************************/
# ifdef _RF4CE_
#  ifdef MAILBOX_HOST_SIDE
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MAC_DataReq,            MAC_DataReqDescr_t,                     RF4CE_MAC_REQ_DATA_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MAC_GetReq,             MAC_GetReqDescr_t,                      RF4CE_MAC_REQ_GET_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MAC_ResetReq,           MAC_ResetReqDescr_t,                    RF4CE_MAC_REQ_RESET_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MAC_RxEnableReq,        MAC_RxEnableReqDescr_t,                 RF4CE_MAC_REQ_RX_ENABLE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MAC_SetReq,             MAC_SetReqDescr_t,                      RF4CE_MAC_REQ_SET_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MAC_StartReq,           MAC_StartReqDescr_t,                    RF4CE_MAC_REQ_START_FID);
#   if defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MAC_ScanReq,            MAC_ScanReqDescr_t,                     RF4CE_MAC_REQ_SCAN_FID);
#   endif /* RF4CE_TARGET */
#  else /* ! MAILBOX_HOST_SIDE */
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_MAC_)
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MAC_DataInd,            MAC_DataIndParams_t,                    RF4CE_MAC_IND_DATA_FID);
#    if defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MAC_BeaconNotifyInd,    MAC_BeaconNotifyIndParams_t,            RF4CE_MAC_IND_BEACON_NOTIFY_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MAC_CommStatusInd,      MAC_CommStatusIndParams_t,              RF4CE_MAC_IND_COMM_STATUS_FID);
#    endif /* RF4CE_TARGET */
#   endif /* WRAPPERS_ALL */
#  endif /* ! MAILBOX_HOST_SIDE */
# endif /* _RF4CE_ */
#endif

/**********************************************************************************************************************/
/* NWK API                                                                                                            */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_NWK_)
/**** ZBPRO *****************************************************************************/
# ifdef _ZBPRO_
#  ifdef MAILBOX_HOST_SIDE
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_NWK_PermitJoiningReq,     ZBPRO_NWK_PermitJoiningReqDescr_t,  ZBPRO_NWK_REQ_PERMIT_JOINING_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_NWK_LeaveReq,             ZBPRO_NWK_LeaveReqDescr_t,          ZBPRO_NWK_REQ_LEAVE_FID);
    // CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_NWK_GetReq,            ZBPRO_NWK_GetReqDescr_t,            ZBPRO_NWK_REQ_GET_FID);
    // CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_NWK_SetReq,            ZBPRO_NWK_SetReqDescr_t,            ZBPRO_NWK_REQ_SET_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_NWK_GetKeyReq,            ZBPRO_NWK_GetKeyReqDescr_t,         ZBPRO_NWK_REQ_GET_KEY_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_NWK_SetKeyReq,            ZBPRO_NWK_SetKeyReqDescr_t,         ZBPRO_NWK_REQ_SET_KEY_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_NWK_RouteDiscoveryReq,    ZBPRO_NWK_RouteDiscoveryReqDescr_t, ZBPRO_NWK_REQ_ROUTE_DISCOVERY_FID);
#  else /* ! MAILBOX_HOST_SIDE */
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_NWK_)
        /* NOTE: insert indications here */
#   endif /* WRAPPERS_ALL */
#  endif /* ! MAILBOX_HOST_SIDE */
# endif /* _ZBPRO_ */

#ifndef MAILBOX_HOST_SIDE
    CREATE_WRAPPER_CLIENT_FUNCTION(SYS_PrintInd,                       SYS_PrintIndParams_t,  SYS_PRINT_FID);
#endif

/**** RF4CE *****************************************************************************/
# ifdef _RF4CE_
#  ifdef MAILBOX_HOST_SIDE
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_ResetReq,          RF4CE_NWK_ResetReqDescr_t,          RF4CE_NWK_REQ_RESET_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_StartReq,          RF4CE_NWK_StartReqDescr_t,          RF4CE_NWK_REQ_START_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_DataReq,           RF4CE_NWK_DataReqDescr_t,           RF4CE_NWK_REQ_DATA_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_GetReq,            RF4CE_NWK_GetReqDescr_t,            RF4CE_NWK_REQ_GET_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_SetReq,            RF4CE_NWK_SetReqDescr_t,            RF4CE_NWK_REQ_SET_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_DiscoveryReq,      RF4CE_NWK_DiscoveryReqDescr_t,      RF4CE_NWK_REQ_DISCOVERY_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_AutoDiscoveryReq,  RF4CE_NWK_AutoDiscoveryReqDescr_t,  RF4CE_NWK_REQ_AUTODISCOVERY_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_PairReq,           RF4CE_NWK_PairReqDescr_t,           RF4CE_NWK_REQ_PAIR_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_UnpairReq,         RF4CE_NWK_UnpairReqDescr_t,         RF4CE_NWK_REQ_UNPAIR_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_RXEnableReq,       RF4CE_NWK_RXEnableReqDescr_t,       RF4CE_NWK_REQ_RXENABLE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_UpdateKeyReq,      RF4CE_NWK_UpdateKeyReqDescr_t,      RF4CE_NWK_REQ_UPDATEKEY_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_DiscoveryResp,     RF4CE_NWK_DiscoveryRespDescr_t,     RF4CE_NWK_RESP_DISCOVERY_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_PairResp,          RF4CE_NWK_PairRespDescr_t,          RF4CE_NWK_RESP_PAIR_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_UnpairResp,        RF4CE_NWK_UnpairRespDescr_t,        RF4CE_NWK_RESP_UNPAIR_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_MacStatsReq,       RF4CE_NWK_MacStatsReqDescr_t,       RF4CE_NWK_MAC_STATS_REQ_FID);
#  else /* ! MAILBOX_HOST_SIDE */
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_NWK_)
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_DataInd,           RF4CE_NWK_DataIndParams_t,          RF4CE_NWK_IND_DATA_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_DiscoveryInd,      RF4CE_NWK_DiscoveryIndParams_t,     RF4CE_NWK_IND_DISCOVERY_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_COMMStatusInd,     RF4CE_NWK_COMMStatusIndParams_t,    RF4CE_NWK_IND_COMMSTATUS_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_PairInd,           RF4CE_NWK_PairIndParams_t,          RF4CE_NWK_IND_PAIR_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_UnpairInd,         RF4CE_NWK_UnpairIndParams_t,        RF4CE_NWK_IND_UNPAIR_FID);
#    ifdef ENABLE_GU_KEY_SEED_IND
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_NWK_KeySeedStartInd,   RF4CE_NWK_KeySeedStartIndParams_t,  RF4CE_NWK_KEY_SEED_START_IND_FID);
#    endif /* ENABLE_GU_KEY_SEED_IND */
#   endif /* WRAPPERS_ALL */
#  endif /* ! MAILBOX_HOST_SIDE */
# endif /* _RF4CE_ */
#endif

/**********************************************************************************************************************/
/* APS API                                                                                                            */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_APS_)
/**** ZBPRO *****************************************************************************/
# ifdef _ZBPRO_
#  ifdef MAILBOX_HOST_SIDE
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_EndpointRegisterReq, ZBPRO_APS_EndpointRegisterReqDescr_t, ZBPRO_APS_REQ_ENDPOINTREGISTER_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_EndpointUnregisterReq,ZBPRO_APS_EndpointUnregisterReqDescr_t, ZBPRO_APS_REQ_ENDPOINTUNREGISTER_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_DataReq,           ZBPRO_APS_DataReqDescr_t,           ZBPRO_APS_REQ_DATA_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_BindReq,           ZBPRO_APS_BindUnbindReqDescr_t,     ZBPRO_APS_REQ_BIND_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_UnbindReq,         ZBPRO_APS_BindUnbindReqDescr_t,     ZBPRO_APS_REQ_UNBIND_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_GetReq,            ZBPRO_APS_GetReqDescr_t,            ZBPRO_APS_REQ_GET_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_SetReq,            ZBPRO_APS_SetReqDescr_t,            ZBPRO_APS_REQ_SET_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_GetKeyReq,         ZBPRO_APS_GetKeyReqDescr_t,         ZBPRO_APS_REQ_GET_KEY_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_SetKeyReq,         ZBPRO_APS_SetKeyReqDescr_t,         ZBPRO_APS_REQ_SET_KEY_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_AddGroupReq,       ZBPRO_APS_AddGroupReqDescr_t,       ZBPRO_APS_REQ_ADDGROUP_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_RemoveGroupReq,    ZBPRO_APS_RemoveGroupReqDescr_t,    ZBPRO_APS_REQ_REMOVEGROUP_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_RemoveAllGroupsReq, ZBPRO_APS_RemoveAllGroupsReqDescr_t, ZBPRO_APS_REQ_REMOVEALLGROUPS_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_TransportKeyReq,   ZBPRO_APS_TransportKeyReqDescr_t,   ZBPRO_APS_REQ_TRANSPORTKEY_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_UpdateDeviceReq,   ZBPRO_APS_UpdateDeviceReqDescr_t,   ZBPRO_APS_REQ_UPDATEDEVICE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_RemoveDeviceReq,   ZBPRO_APS_RemoveDeviceReqDescr_t,   ZBPRO_APS_REQ_REMOTEDEVICE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_RequestKeyReq,     ZBPRO_APS_RequestKeyReqDescr_t,     ZBPRO_APS_REQ_REQUESTKEY_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_SwitchKeyReq,      ZBPRO_APS_SwitchKeyReqDescr_t,      ZBPRO_APS_REQ_SWITCHKEY_FID);
#  else /* ! MAILBOX_HOST_SIDE */
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_DataInd,           ZBPRO_APS_DataIndParams_t,          ZBPRO_APS_IND_DATA_FID);
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_APS_)
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_TransportKeyInd,   ZBPRO_APS_TransportKeyIndParams_t,  ZBPRO_APS_IND_TRANSPORTKEY_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_UpdateDeviceInd,   ZBPRO_APS_UpdateDeviceIndParams_t,  ZBPRO_APS_IND_UPDATEDEVICE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_RemoveDeviceInd,   ZBPRO_APS_RemoveDeviceIndParams_t,  ZBPRO_APS_IND_REMOTEDEVICE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_RequestKeyInd,     ZBPRO_APS_RequestKeyIndParams_t,    ZBPRO_APS_IND_REQUESTKEY_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_APS_SwitchKeyInd,      ZBPRO_APS_SwitchKeyIndParams_t,     ZBPRO_APS_IND_SWITCHKEY_FID);
#   endif /* WRAPPERS_ALL */
#  endif /* ! MAILBOX_HOST_SIDE */
# endif /* _ZBPRO_ */
#endif

/**********************************************************************************************************************/
/* ZDO API                                                                                                            */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZDO_)
/**** ZBPRO *****************************************************************************/
# ifdef _ZBPRO_
#  ifdef MAILBOX_HOST_SIDE
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_AddrResolvingReq,          ZBPRO_ZDO_AddrResolvingReqDescr_t,          ZBPRO_ZDO_REQ_ADDR_RESOLVING_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_NodeDescReq,               ZBPRO_ZDO_NodeDescReqDescr_t,               ZBPRO_ZDO_REQ_NODE_DESC_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_PowerDescReq,              ZBPRO_ZDO_PowerDescReqDescr_t,              ZBPRO_ZDO_REQ_POWER_DESC_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_SimpleDescReq,             ZBPRO_ZDO_SimpleDescReqDescr_t,             ZBPRO_ZDO_REQ_SIMPLE_DESC_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_ActiveEpReq,               ZBPRO_ZDO_ActiveEpReqDescr_t,               ZBPRO_ZDO_REQ_ACTIVE_EP_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_MatchDescReq,              ZBPRO_ZDO_MatchDescReqDescr_t,              ZBPRO_ZDO_REQ_MATCH_DESC_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_DeviceAnnceReq,            ZBPRO_ZDO_DeviceAnnceReqDescr_t,            ZBPRO_ZDO_REQ_DEVICE_ANNCE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_EndDeviceBindReq,          ZBPRO_ZDO_EndDeviceBindReqDescr_t,          ZBPRO_ZDO_REQ_ED_BIND_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_BindReq,                   ZBPRO_ZDO_BindUnbindReqDescr_t,             ZBPRO_ZDO_REQ_BIND_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_UnbindReq,                 ZBPRO_ZDO_BindUnbindReqDescr_t,             ZBPRO_ZDO_REQ_UNBIND_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_StartNetworkReq,           ZBPRO_ZDO_StartNetworkReqDescr_t,           ZBPRO_ZDO_REQ_START_NETWORK_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_MgmtLeaveReq,              ZBPRO_ZDO_MgmtLeaveReqDescr_t,              ZBPRO_ZDO_REQ_MGMT_LEAVE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_MgmtPermitJoiningReq,      ZBPRO_ZDO_MgmtPermitJoiningReqDescr_t,      ZBPRO_ZDO_REQ_MGMT_PERMIT_JOINING_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_MgmtNwkUpdateReq,          ZBPRO_ZDO_MgmtNwkUpdateReqDescr_t,          ZBPRO_ZDO_REQ_MGMT_NWK_UPDATE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_MgmtNwkUpdateUnsolResp,    ZBPRO_ZDO_MgmtNwkUpdateUnsolRespDescr_t,    ZBPRO_ZDO_RESP_MGMT_NWK_UPDATE_UNSOL_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_MgmtLqiReq_Call,           ZBPRO_ZDO_MgmtLqiReqDescr_t,                ZBPRO_ZDO_REQ_MGMT_LQI_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_MgmtBindReq,               ZBPRO_ZDO_MgmtBindReqDescr_t,               ZBPRO_ZDO_REQ_MGMT_BIND_FID);
#  else /* ! MAILBOX_HOST_SIDE */
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_ZDO_)
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZDO_MgmtNwkUpdateUnsolInd,     ZBPRO_ZDO_MgmtNwkUpdateUnsolIndParams_t,    ZBPRO_ZDO_IND_MGMT_NWK_UPDATE_UNSOL_FID);
#   endif /* WRAPPERS_ALL */
#  endif /* ! MAILBOX_HOST_SIDE */
# endif /* _ZBPRO_ */
#endif

/**********************************************************************************************************************/
/* TC API                                                                                                            */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_TC_)
/**** ZBPRO *****************************************************************************/
# ifdef _ZBPRO_
#  ifdef MAILBOX_HOST_SIDE
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_TC_NwkKeyUpdateReq, ZBPRO_TC_NwkKeyUpdateReqDescr_t, ZBPRO_TC_REQ_NWK_KEY_UPDATE_FID);
#  else /* ! MAILBOX_HOST_SIDE */
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_TC_)
#   endif /* WRAPPERS_ALL */
#  endif /* ! MAILBOX_HOST_SIDE */
# endif /* _ZBPRO_ */
#endif

/**********************************************************************************************************************/
/* ZCL API                                                                                                            */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_)
/**** ZBPRO *****************************************************************************/
# ifdef _ZBPRO_
#  ifdef MAILBOX_HOST_SIDE
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SetPowerSourceReq,                         ZBPRO_ZCL_SetPowerSourceReqDescr_t,                         ZBPRO_ZCL_REQ_SET_POWER_SOURCE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReq,             ZBPRO_ZCL_ProfileWideCmdDiscoverAttrDescr_t,                ZBPRO_ZCL_PROFILE_WIDE_CMD_DISCOVER_ATTRIBUTE_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ProfileWideCmdReadAttributesReq,           ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t,                 ZBPRO_ZCL_PROFILE_WIDE_CMD_READ_ATTRIBUTE_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ProfileWideCmdWriteAttributesReq,          ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t,                ZBPRO_ZCL_PROFILE_WIDE_CMD_WRITE_ATTRIBUTE_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ProfileWideCmdConfigureReportingReq,   ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t,   ZBPRO_ZCL_PROFILE_WIDE_CMD_CONFIGURE_REPORTING_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReq,   ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqDescr_t, ZBPRO_ZCL_PROFILE_WIDE_CMD_READ_REPORTING_CONFIGURATION_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IdentifyCmdIdentifyReq,                    ZBPRO_ZCL_IdentifyCmdIdentifyReqDescr_t,                    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IdentifyCmdIdentifyQueryReq,               ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqDescr_t,               ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_REQ_FID);
#   if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_IDENTIFY_RELAY_)
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IdentifyCmdIdentifyResponseReq,            ZBPRO_ZCL_IdentifyCmdIdentifyResponseReqDescr_t,            ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_RESPONSE_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReq,       ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReqDescr_t,       ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_RESPONSE_REQ_FID);
#   endif
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_GroupsCmdAddGroupReq,                      ZBPRO_ZCL_GroupsCmdAddGroupReqDescr_t,                      ZBPRO_ZCL_GROUPS_CMD_ADD_GROUP_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_GroupsCmdViewGroupReq,                     ZBPRO_ZCL_GroupsCmdViewGroupReqDescr_t,                     ZBPRO_ZCL_GROUPS_CMD_VIEW_GROUP_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_GroupsCmdGetGroupMembershipReq,            ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqDescr_t,            ZBPRO_ZCL_GROUPS_CMD_GET_GROUP_MEMBERSHIP_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_GroupsCmdRemoveGroupReq,                   ZBPRO_ZCL_GroupsCmdRemoveGroupReqDescr_t,                   ZBPRO_ZCL_GROUPS_CMD_REMOVE_GROUP_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReq,               ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqDescr_t,               ZBPRO_ZCL_GROUPS_CMD_REMOVE_ALL_GROUPS_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReq,            ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqDescr_t,            ZBPRO_ZCL_GROUPS_CMD_ADD_GROUP_IF_IDENTIFY_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ScenesCmdAddSceneReq,                      ZBPRO_ZCL_ScenesCmdAddSceneReqDescr_t,                      ZBPRO_ZCL_SCENES_CMD_ADD_SCENE_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ScenesCmdViewSceneReq,                     ZBPRO_ZCL_ScenesCmdViewSceneReqDescr_t,                     ZBPRO_ZCL_SCENES_CMD_VIEW_SCENE_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ScenesCmdStoreSceneReq,                    ZBPRO_ZCL_ScenesCmdStoreSceneReqDescr_t,                    ZBPRO_ZCL_SCENES_CMD_STORE_SCENE_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ScenesCmdRecallSceneReq,                   ZBPRO_ZCL_ScenesCmdRecallSceneReqDescr_t,                   ZBPRO_ZCL_SCENES_CMD_RECALL_SCENE_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ScenesCmdRemoveSceneReq,                   ZBPRO_ZCL_ScenesCmdRemoveSceneReqDescr_t,                   ZBPRO_ZCL_SCENES_CMD_REMOVE_SCENE_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ScenesCmdRemoveAllScenesReq,               ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqDescr_t,               ZBPRO_ZCL_SCENES_CMD_REMOVE_ALL_SCENES_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ScenesCmdGetSceneMembershipReq,            ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqDescr_t,            ZBPRO_ZCL_SCENES_CMD_GET_SCENE_MEMBERSHIP_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_OnOffCmdOffReq,                            ZBPRO_ZCL_OnOffCmdReqDescr_t,                               ZBPRO_ZCL_ONOFF_CMD_OFF_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_OnOffCmdOnReq,                             ZBPRO_ZCL_OnOffCmdReqDescr_t,                               ZBPRO_ZCL_ONOFF_CMD_ON_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_OnOffCmdToggleReq,                         ZBPRO_ZCL_OnOffCmdReqDescr_t,                               ZBPRO_ZCL_ONOFF_CMD_TOGGLE_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_LevelControlCmdMoveToLevelReq,             ZBPRO_ZCL_LevelControlCmdMoveToLevelReqDescr_t,             ZBPRO_ZCL_LEVEL_CONTROL_CMD_MOVE_TO_LEVEL_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_LevelControlCmdMoveReq,                    ZBPRO_ZCL_LevelControlCmdMoveReqDescr_t,                    ZBPRO_ZCL_LEVEL_CONTROL_CMD_MOVE_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_LevelControlCmdStepReq,                    ZBPRO_ZCL_LevelControlCmdStepReqDescr_t,                    ZBPRO_ZCL_LEVEL_CONTROL_CMD_STEP_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_LevelControlCmdStopReq,                    ZBPRO_ZCL_LevelControlCmdStopReqDescr_t,                    ZBPRO_ZCL_LEVEL_CONTROL_CMD_STOP_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_DoorLockCmdLockReq,                        ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t,                  ZBPRO_ZCL_DOOR_LOCK_CMD_LOCK_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_DoorLockCmdUnlockReq,                      ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t,                  ZBPRO_ZCL_DOOR_LOCK_CMD_UNLOCK_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_WindowCoveringCmdUpOpenReq,                ZBPRO_ZCL_WindowCoveringCmdReqDescr_t,                      ZBPRO_ZCL_WINDOW_COVERING_CMD_UP_OPEN_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_WindowCoveringCmdDownCloseReq,             ZBPRO_ZCL_WindowCoveringCmdReqDescr_t,                      ZBPRO_ZCL_WINDOW_COVERING_CMD_DOWN_CLOSE_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_WindowCoveringCmdStopReq,                  ZBPRO_ZCL_WindowCoveringCmdReqDescr_t,                      ZBPRO_ZCL_WINDOW_COVERING_CMD_STOP_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_WindowCoveringCmdGotoLiftPecentageReq,     ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t,       ZBPRO_ZCL_WINDOW_COVERING_CMD_GO_TO_LIFT_PERCENTAGE_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_WindowCoveringCmdGotoTiltPecentageReq,     ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t,       ZBPRO_ZCL_WINDOW_COVERING_CMD_GO_TO_TILT_PERCENTAGE_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReq,           ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqDescr_t,           ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_ENROLL_RESPONSE_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceArmRespReq,                       ZBPRO_ZCL_SapIasAceArmRespReqDescr_t,                       ZBPRO_ZCL_IAS_ACE_ARM_RESP_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceBypassRespReq,                    ZBPRO_ZCL_SapIasAceBypassRespReqDescr_t,                    ZBPRO_ZCL_IAS_ACE_BYPASS_RESP_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReq,              ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqDescr_t,              ZBPRO_ZCL_IAS_ACE_GET_ZONE_ID_MAP_RESP_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneInfoRespReq,               ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqDescr_t,               ZBPRO_ZCL_IAS_ACE_GET_ZONE_INFO_RESP_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceGetPanelStatusRespReq,            ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqDescr_t,            ZBPRO_ZCL_IAS_ACE_GET_PANEL_STATUS_RESP_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReq,       ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqDescr_t,       ZBPRO_ZCL_IAS_ACE_SET_BYPASSED_ZONE_LIST_RESP_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneStatusRespReq,             ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqDescr_t,             ZBPRO_ZCL_IAS_ACE_GET_ZONE_STATUS_RESP_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceZoneStatusChangedReq,             ZBPRO_ZCL_SapIasAceZoneStatusChangedReqDescr_t,             ZBPRO_ZCL_IAS_ACE_ZONE_STATUS_CHANGED_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAcePanelStatusChangedReq,            ZBPRO_ZCL_SapIasAcePanelStatusChangedReqDescr_t,            ZBPRO_ZCL_IAS_ACE_PANEL_STATUS_CHANGED_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IASWDCmdStartWarningReq,                   ZBPRO_ZCL_IASWDCmdStartWarningReqDescr_t,                   ZBPRO_ZCL_IAS_WD_CMD_START_WARNING_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IASWDCmdSquawkgReq,                        ZBPRO_ZCL_IASWDCmdSquawkReqDescr_t,                         ZBPRO_ZCL_IAS_WD_CMD_SQUAWK_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ColorControlCmdMoveToColorReq,             ZBPRO_ZCL_ColorControlCmdMoveToColorReqDescr_t,         ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_TO_COLOR_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ColorControlCmdMoveColorReq,               ZBPRO_ZCL_ColorControlCmdMoveColorReqDescr_t,           ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_COLOR_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ColorControlCmdStepColorReq,               ZBPRO_ZCL_ColorControlCmdStepColorReqDescr_t,           ZBPRO_ZCL_COLOR_CONTROL_CMD_STEP_COLOR_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReq,      ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqDescr_t,   ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_TO_HUE_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReq,        ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqDescr_t,     ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_HUE_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReq,        ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqDescr_t,     ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_STEP_HUE_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReq,      ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqDescr_t,   ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_TO_HUE_AND_SATURATION_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ColorControlCmdColorLoopSetReq,           ZBPRO_ZCL_ColorControlCmdColorLoopSetReqDescr_t,        ZBPRO_ZCL_COLOR_CONTROL_CMD_COLOR_LOOP_SET_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ColorControlCmdStopMoveStepReq,           ZBPRO_ZCL_ColorControlCmdStopMoveStepReqDescr_t,        ZBPRO_ZCL_COLOR_CONTROL_CMD_STOP_MOVE_STEP_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReq,   ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqDescr_t,    ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_COLOR_TEMPERATURE_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReq,   ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqDescr_t,    ZBPRO_ZCL_COLOR_CONTROL_CMD_STEP_COLOR_TEMPERATURE_REQ_FID);
#  else /* ! MAILBOX_HOST_SIDE */
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_ZCL_)
#    if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_IDENTIFY_RELAY_)
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IdentifyCmdIdentifyInd,                    ZBPRO_ZCL_IdentifyCmdIdentifyIndParams_t,                   ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_IND_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IdentifyCmdIdentifyQueryInd,               ZBPRO_ZCL_IdentifyCmdIdentifyQueryIndParams_t,              ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_IND_FID);
#    else
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IdentifyInd,                               ZBPRO_ZCL_IdentifyIndParams_t,                              ZBPRO_ZCL_IND_IDENTIFY_FID);
#    endif
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseIndEB,     ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseIndParams_t,      ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_RESPONSE_IND_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_GroupsCmdGetGroupMembershipResponseInd,    ZBPRO_ZCL_GroupsCmdGetGroupMembershipIndParams_t,           ZBPRO_ZCL_GROUPS_CMD_GET_GROUP_MEMBERSHIP_RESPONSE_IND_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseInd,    ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseIndParams_t,   ZBPRO_ZCL_SCENES_CMD_GET_SCENE_MEMBERSHIP_RESPONSE_IND_FID);
    // CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestInd,            ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestIndParams_t,            ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_ENROLL_IND_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationInd, ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationIndParams_t, ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_STATUS_CHANGED_NOTIFICATION_IND_FID);
    // CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceArmInd,                           ZBPRO_ZCL_SapIasAceArmIndParams_t,                          ZBPRO_ZCL_IAS_ACE_ARM_IND_FID);
    // CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceBypassInd,                        ZBPRO_ZCL_SapIasAceBypassIndParams_t,                       ZBPRO_ZCL_IAS_ACE_BYPASS_IND_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceEmergencyInd,                     ZBPRO_ZCL_SapIasAceAlarmIndParams_t,                        ZBPRO_ZCL_IAS_ACE_EMERGENCY_IND_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceFireInd,                          ZBPRO_ZCL_SapIasAceAlarmIndParams_t,                        ZBPRO_ZCL_IAS_ACE_FIRE_IND_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAcePanicInd,                         ZBPRO_ZCL_SapIasAceAlarmIndParams_t,                        ZBPRO_ZCL_IAS_ACE_PANIC_IND_FID);
    // CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneIdMapInd,                  ZBPRO_ZCL_SapIasAceGetZoneIdMapIndParams_t,                 ZBPRO_ZCL_IAS_ACE_GET_ZONE_ID_MAP_IND_FID);
    // CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneInfoInd,                   ZBPRO_ZCL_SapIasAceGetZoneInfoIndParams_t,                  ZBPRO_ZCL_IAS_ACE_GET_ZONE_INFO_IND_FID);
    // CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceGetPanelStatusInd,                ZBPRO_ZCL_SapIasAceGetPanelStatusIndParams_t,               ZBPRO_ZCL_IAS_ACE_GET_PANEL_STATUS_IND_FID);
    // CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceGetBypassedZoneListInd,           ZBPRO_ZCL_SapIasAceGetBypassedZoneListIndParams_t,          ZBPRO_ZCL_IAS_ACE_GET_BYPASSED_ZONE_LIST_IND_FID);
    // CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneStatusInd,                 ZBPRO_ZCL_SapIasAceGetZoneStatusIndParams_t,                ZBPRO_ZCL_IAS_ACE_GET_ZONE_STATUS_IND_FID);
    // CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZHA_CieDeviceSetPanelStatusInd,                ZBPRO_ZHA_CieSetPanelStatusIndParams_t,                     ZBPRO_ZHA_CIE_SET_PANEL_STATUS_IND_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZCL_ProfileWideCmdReportAttributesInd,         ZBPRO_ZCL_ProfileWideCmdReportAttributesIndParams_t,        ZBPRO_ZCL_PROFILE_WIDE_CMD_REPORT_ATTRIBUTES_IND_FID);
#   endif /* WRAPPERS_ALL */
#  endif /* ! MAILBOX_HOST_SIDE */
# endif /* _ZBPRO_ */
#endif

/**********************************************************************************************************************/
/* PROFILES API                                                                                                       */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_PROFILE_)
/**** ZBPRO *****************************************************************************/
# ifdef _ZBPRO_
#  if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZHA_)
#   ifdef MAILBOX_HOST_SIDE
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZHA_EzModeReq,                 ZBPRO_ZHA_EzModeReqDescr_t,             ZBPRO_ZHA_EZ_MODE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZHA_CieDeviceEnrollReq,        ZBPRO_ZHA_CieEnrollReqDescr_t,          ZBPRO_ZHA_CIE_ENROLL_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZHA_CieDeviceSetPanelStatusReq,    ZBPRO_ZHA_CieSetPanelStatusReqDescr_t,    ZBPRO_ZHA_CIE_SET_PANEL_STATUS_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZHA_CieZoneSetBypassStateReq,      ZBPRO_ZHA_CieZoneSetBypassStateReqDescr_t,    ZBPRO_ZHA_CIE_ZONE_SET_BYPASS_STATE_REQ_FID);
#   else /* ! MAILBOX_HOST_SIDE */
#    if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_ZHA_)
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZHA_CieDeviceSetPanelStatusInd,    ZBPRO_ZHA_CieSetPanelStatusIndParams_t,    ZBPRO_ZHA_CIE_SET_PANEL_STATUS_IND_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(ZBPRO_ZHA_CieDeviceEnrollInd,            ZBPRO_ZHA_CieEnrollIndParams_t,            ZBPRO_ZHA_CIE_ENROLL_IND_FID);
#    endif /* WRAPPERS_ALL */
#   endif /* ! MAILBOX_HOST_SIDE */
#  endif /* _MAILBOX_WRAPPERS_ZHA_ */
# endif /* _ZBPRO_ */

/**** RF4CE *****************************************************************************/
# ifdef _RF4CE_
/**** RF4CE PROFILE MANAGER ****************/
# ifdef MAILBOX_HOST_SIDE
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_UnpairReq,             RF4CE_UnpairReqDescr_t,             RF4CE_PROFILE_REQ_UNPAIR_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_StartReq,              RF4CE_StartReqDescr_t,              RF4CE_PROFILE_REQ_START_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ResetReq,              RF4CE_ResetReqDescr_t,              RF4CE_PROFILE_REQ_RESET_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_SetSupportedDevicesReq, RF4CE_SetSupportedDevicesReqDescr_t, RF4CE_PROFILE_REQ_SET_SUP_DEVICES_FID);

#  else /* ! MAILBOX_HOST_SIDE */
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_PROFILE_)
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_CounterExpiredInd,     RF4CE_PairingReferenceIndParams_t,  RF4CE_PROFILE_IND_COUNTER_EXPIRED_FID);
#       if !defined(USE_RF4CE_PROFILE_ZRC2)
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_UnpairInd,             RF4CE_PairingReferenceIndParams_t,  RF4CE_PROFILE_IND_UNPAIR_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_PairInd,               RF4CE_PairingIndParams_t,           RF4CE_PROFILE_IND_PAIR_FID);
#       endif /* !defined(USE_RF4CE_PROFILE_ZRC2) */
#   endif /* WRAPPERS_ALL */
#  endif /* ! MAILBOX_HOST_SIDE */

/**** RF4CE PROFILE ZRC ********************/
# if (1 == USE_RF4CE_PROFILE_ZRC)
/**** RF4CE PROFILE ZRC 1.1 ****************/
# ifdef USE_RF4CE_PROFILE_ZRC1
#  ifdef MAILBOX_HOST_SIDE
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC1_GetAttributesReq,        RF4CE_ZRC1_GetAttributeDescr_t,             RF4CE_ZRC1_REQ_GET_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC1_SetAttributesReq,        RF4CE_ZRC1_SetAttributeDescr_t,             RF4CE_ZRC1_REQ_SET_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC1_ControllerBindReq,        RF4CE_ZRC1_BindReqDescr_t,             RF4CE_ZRC1_REQ_CONTROLLER_BIND_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC1_VendorSpecificReq,        RF4CE_ZRC1_VendorSpecificReqDescr_t,   RF4CE_ZRC1_VENDORSPECIFIC_REQ_FID);
#    if defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
         CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC1_TargetBindReq,        RF4CE_ZRC1_BindReqDescr_t,             RF4CE_ZRC1_REQ_TARGET_BIND_FID);
#    endif /* RF4CE_TARGET */
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC1_CommandDiscoveryReq,     RF4CE_ZRC1_CommandDiscoveryReqDescr_t,  RF4CE_ZRC1_REQ_COMMANDDISCOVERY_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC1_ControlCommandPressedReq,     RF4CE_ZRC1_ControlCommandReqDescr_t,  RF4CE_ZRC1_CONTROL_COMMAND_PRESSED_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC1_ControlCommandReleasedReq,     RF4CE_ZRC1_ControlCommandReqDescr_t,  RF4CE_ZRC1_CONTROL_COMMAND_RELEASED_FID);
#  else /* MAILBOX_HOST_SIDE */
#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_PROFILE_)
#    if defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
#     ifndef RF4CE_ZRC_WAKEUP_ACTION_CODE_SUPPORT
      CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC1_ControlCommandInd,             RF4CE_ZRC1_ControlCommandIndParams_t,         RF4CE_ZRC1_IND_CONTROLCOMMAND_FID);
#     endif
#    endif /* RF4CE_TARGET */
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC1_VendorSpecificInd,             RF4CE_ZRC1_VendorSpecificIndParams_t,         RF4CE_ZRC1_VENDORSPECIFIC_IND_FID);
#   endif /* WRAPPERS_ALL */
#  endif /* MAILBOX_HOST_SIDE */
# endif /* USE_RF4CE_PROFILE_ZRC1 */
/**** RF4CE PROFILE GDP 2.0 & ZRC 2.0 ******/
#  ifdef USE_RF4CE_PROFILE_ZRC2
#   ifdef MAILBOX_HOST_SIDE
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_GetAttributesReq,             RF4CE_ZRC2_GetAttributesReqDescr_t,     RF4CE_ZRC2_REQ_GET_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_SetAttributesReq,             RF4CE_ZRC2_SetAttributesReqDescr_t,         RF4CE_ZRC2_REQ_SET_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_KeyExchangeReq,               RF4CE_ZRC2_KeyExchangeReqDescr_t,       RF4CE_ZRC2_KEY_EXCHANGE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_BindReq,                      RF4CE_ZRC2_BindReqDescr_t,              RF4CE_ZRC2_BIND_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_ProxyBindReq,                 RF4CE_ZRC2_ProxyBindReqDescr_t,         RF4CE_ZRC2_PROXY_BIND_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_EnableBindingReq,             RF4CE_ZRC2_BindingReqDescr_t,           RF4CE_ZRC2_ENABLE_BINDING_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_DisableBindingReq,            RF4CE_ZRC2_BindingReqDescr_t,           RF4CE_ZRC2_DISABLE_BINDING_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_SetPushButtonStimulusReq,     RF4CE_ZRC2_ButtonBindingReqDescr_t,         RF4CE_ZRC2_SET_PUSH_BUTTON_STIMULUS_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_ClearPushButtonStimulusReq,   RF4CE_ZRC2_ButtonBindingReqDescr_t,         RF4CE_ZRC2_CLEAR_PUSH_BUTTON_STIMULUS_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_CheckValidationResp,          RF4CE_ZRC2_CheckValidationRespDescr_t,  RF4CE_ZRC2_CHECK_VALIDATION_RESP_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_ControlCommandPressedReq,     RF4CE_ZRC2_ControlCommandReqDescr_t,    RF4CE_ZRC2_CONTROL_COMMAND_PRESS_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_ControlCommandReleasedReq,    RF4CE_ZRC2_ControlCommandReqDescr_t,    RF4CE_ZRC2_CONTROL_COMMAND_RELEASE_REQ_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_GDP2_SetPollConstraintsReq,        RF4CE_GDP2_SetPollConstraintsReqDescr_t,    RF4CE_GDP2_REQ_SET_POLL_CONSTRAINTS_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_GDP2_PollNegotiationReq,           RF4CE_GDP2_PollNegotiationReqDescr_t,       RF4CE_GDP2_REQ_POLL_NEGOTIATION_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_GDP2_PollClientUserEventReq,       RF4CE_GDP2_PollClientUserEventReqDescr_t,   RF4CE_GDP2_REQ_POLL_CLIENT_USER_EVENT_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_GDP2_ClientNotificationReq,        RF4CE_GDP2_ClientNotificationReqDescr_t,    RF4CE_GDP2_REQ_CLIENT_NOTIFICATION_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_GDP2_IdentifyCapAnnounceReq,       RF4CE_GDP2_IdentifyCapAnnounceReqDescr_t,   RF4CE_GDP2_REQ_IDENTIFY_CAP_ANNOUNCE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_GDP2_IdentifyReq,                  RF4CE_GDP2_IdentifyReqDescr_t,              RF4CE_GDP2_REQ_IDENTIFY_FID);
#   else /* MAILBOX_HOST_SIDE */
#    if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_PROFILE_)
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_PairNtfyInd,                  RF4CE_PairingIndParams_t,                   RF4CE_ZRC2_PAIR_IND_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_BindingFinishedNtfyInd,       RF4CE_ZRC2_BindingFinishedNtfyIndParams_t,  RF4CE_ZRC2_BINDING_FINISHED_IND_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_StartValidationInd,           RF4CE_ZRC2_CheckValidationIndParams_t,  RF4CE_ZRC2_START_VALIDATION_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_CheckValidationInd,           RF4CE_ZRC2_CheckValidationIndParams_t,  RF4CE_ZRC2_CHECK_VALIDATION_IND_FID);
#   ifndef RF4CE_ZRC_WAKEUP_ACTION_CODE_SUPPORT
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_ControlCommandInd,            RF4CE_ZRC2_ControlCommandIndParams_t,   RF4CE_ZRC2_CONTROL_COMMAND_IND_FID);
#   endif
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_GDP2_PollNegotiationInd,           RF4CE_GDP2_PollNegotiationIndParams_t,      RF4CE_GDP2_IND_POLL_NEGOTIATION_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_GDP2_HeartbeatInd,                 RF4CE_GDP2_HeartbeatIndParams_t,            RF4CE_GDP2_IND_HEARTBEAT_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_GDP2_ClientNotificationInd,        RF4CE_GDP2_ClientNotificationIndParams_t,   RF4CE_GDP2_IND_CLIENT_NOTIFICATION_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_GDP2_IdentifyCapAnnounceInd,       RF4CE_GDP2_IdentifyCapAnnounceIndParams_t,  RF4CE_GDP2_IND_IDENTIFY_CAP_ANNOUNCE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_GDP2_IdentifyInd,                  RF4CE_GDP2_IdentifyIndParams_t,             RF4CE_GDP2_IND_IDENTIFY_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_GetSharedSecretInd,           RF4CE_ZRC2_GetSharedSecretIndDescr_t,   RF4CE_ZRC2_GET_SHARED_SECRET_IND_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_GetAttrRespInd,               RF4CE_ZRC2_SetAttributesReqParams_t,        RF4CE_ZRC2_IND_GET_RESP_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC2_PushAttrReqInd,               RF4CE_ZRC2_SetAttributesReqParams_t,        RF4CE_ZRC2_IND_PUSH_REQ_FID);
#     endif /* WRAPPERS_ALL */
#    endif /* ! MAILBOX_HOST_SIDE */
#   endif /* USE_RF4CE_PROFILE_ZRC2 */

/**** RF4CE addons *************************/
#   if defined(MAILBOX_HOST_SIDE)
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC_SetWakeUpActionCodeReq,        RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t,    RF4CE_ZRC_SET_WAKEUP_ACTION_CODE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_ZRC_GetWakeUpActionCodeReq,        RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t,    RF4CE_ZRC_GET_WAKEUP_ACTION_CODE_FID);
#   endif

#  endif /* USE_RF4CE_PROFILE_ZRC */

#  if (1 == USE_RF4CE_PROFILE_MSO)
#   ifdef MAILBOX_HOST_SIDE
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MSO_GetProfileAttributeReq,             RF4CE_MSO_GetProfileAttributeReqDescr_t,  RF4CE_MSO_GET_PROFILE_ATTRIBUTE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MSO_SetProfileAttributeReq,             RF4CE_MSO_SetProfileAttributeReqDescr_t,  RF4CE_MSO_SET_PROFILE_ATTRIBUTE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MSO_GetRIBAttributeReq,                 RF4CE_MSO_GetRIBAttributeReqDescr_t,      RF4CE_MSO_GET_RIB_ATTRIBUTE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MSO_SetRIBAttributeReq,                 RF4CE_MSO_SetRIBAttributeReqDescr_t,      RF4CE_MSO_SET_RIB_ATTRIBUTE_FID);
#    if defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MSO_WatchDogKickOrValidateReq,          RF4CE_MSO_WatchDogKickOrValidateReqDescr_t,RF4CE_MSO_VALIDATE_RESP_FID);
#    endif /* RF4CE_TARGET */
#    if defined(RF4CE_CONTROLLER) || defined(MAILBOX_UNIT_TEST)
         CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MSO_BindReq,                       RF4CE_MSO_BindReqDescr_t,                 RF4CE_MSO_BIND_FID);
         CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MSO_UserControlPressedReq,         RF4CE_MSO_UserControlReqDescr_t,          RF4CE_MSO_USER_CONTROL_PRESSED_FID);
         CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MSO_UserControlReleasedReq,        RF4CE_MSO_UserControlReqDescr_t,          RF4CE_MSO_USER_CONTROL_RELEASED_FID);
#    endif /* RF4CE_CONTROLLER */
#   else /* MAILBOX_HOST_SIDE */
#    if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_PROFILE_)
#     if defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
      CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MSO_CheckValidationInd,               RF4CE_MSO_CheckValidationIndParams_t,     RF4CE_MSO_CHECK_VALIDATION_IND_FID);
      CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MSO_UserControlInd,                   RF4CE_MSO_UserControlIndParams_t,         RF4CE_MSO_USER_CONTROL_IND_FID);
#     endif /* RF4CE_TARGET */
    CREATE_WRAPPER_CLIENT_FUNCTION(RF4CE_MSO_StartValidationInd,                 RF4CE_PairingReferenceIndParams_t,        RF4CE_MSO_START_VALIDATION_IND_FID);
#    endif /* WRAPPERS_ALL */
#   endif /* MAILBOX_HOST_SIDE */
#  endif /* (1 == USE_RF4CE_PROFILE_MSO) */

# endif /* _RF4CE_ */
#endif /* _MAILBOX_WRAPPERS_RF4CE_PROFILE_ */

#ifdef MAILBOX_STACK_SIDE
    CREATE_WRAPPER_CLIENT_FUNCTION(NVM_ReadFileInd, NVM_ReadFileIndDescr_t, NVM_READ_FILE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(NVM_OpenFileInd, NVM_OpenFileIndDescr_t, NVM_OPEN_FILE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(NVM_WriteFileInd, NVM_WriteFileIndDescr_t, NVM_WRITE_FILE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(NVM_CloseFileInd, NVM_CloseFileIndDescr_t, NVM_CLOSE_FILE_FID);
#endif /* MAILBOX_STACK_SIDE */

#ifdef MAILBOX_HOST_SIDE
    CREATE_WRAPPER_CLIENT_FUNCTION(SYS_EventSubscribe, SYS_EventHandlerParams_t,        SYS_EVENT_SUBSCRIBE_FID);
    CREATE_WRAPPER_CLIENT_FUNCTION(SYS_EventRaise,              SYS_EventNotifyParams_t,            SYS_EVENT_RAISE_FID);
#endif /* MAILBOX_HOST_SIDE */
#ifdef MAILBOX_STACK_SIDE
    CREATE_WRAPPER_CLIENT_FUNCTION(SYS_EventNtfy,      SYS_EventNotifyParams_t,         SYS_EVENT_NOTIFY_FID);
#endif /* MAILBOX_STACK_SIDE */
#endif /* NOT MAILBOX_UNIT_TEST */