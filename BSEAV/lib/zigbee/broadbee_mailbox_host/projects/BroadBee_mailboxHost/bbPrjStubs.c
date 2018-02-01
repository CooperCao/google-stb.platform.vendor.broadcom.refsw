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

/*****************************************************************************
*
* DESCRIPTION:
*       The project stubs implementation.
*
*****************************************************************************************/

/************************* INCLUDES *****************************************************/
#include "bbMailAPI.h"
#include "zigbee_common.h"
#include "zigbee_socket_server.h"
#include "zigbee_api.h"

#ifdef _ZBPRO_
#include "ha_registration.h"
#include "bbMacSapForZBPRO.h"
#include "bbZbProNwkSapTypesJoin.h"
#include "bbZbProNwkSapTypesLeave.h"
#include "bbZbProApsSapSecurityServices.h"
#endif

#ifdef _RF4CE_
#include "bbRF4CEConfig.h"
#include "bbRF4CENWK.h"
#include "bbRF4CEPM.h"
//#include "bbRF4CEGDPValidation.h"
//#include "bbRF4CEGDPExternalIndications.h"
#include "bbRF4CEZRCControlCommand.h"
#endif

#define STUB_DUMP_ERROR_MSG \
        do\
        {\
            printf("Warning:  \r\n"); \
            printf("Unexpected stub function %s is called, application should implement this\r\n\r\n", __FUNCTION__); \
        } while(0)

/* needed by server process */
//void RF4CE_UnpairReq(RF4CE_UnpairReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
//void RF4CE_StartReq(RF4CE_StartReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }

//void RF4CE_GDP_HeartbeatReq(RF4CE_GDP_HeartbeatReqDescr_t *request) {}
//void RF4CE_GDP_PushAttributesInd(RF4CE_GDP_HostAttributesReqDescr_t *reqDescr) {}
//void RF4CE_GDP_PullAttributesInd(RF4CE_GDP_HostAttributesReqDescr_t *reqDescr)  {}
//void RF4CE_GDP_GetSharedSecretInd(RF4CE_GDP_GetSharedSecretIndDescr_t *reqDescr) {}
//void RF4CE_GDP_CheckValidationInd(RF4CE_GDP_CheckValidationIndDescr_t *reqDescr) {}
//void RF4CE_ZRC_HeartbeatReq(RF4CE_GDP_HeartbeatReqDescr_t *request)  {}
//void RF4CE_ZRC_PushAttributesInd(RF4CE_GDP_HostAttributesReqDescr_t *reqDescr) {}
//void RF4CE_ZRC_PullAttributesInd(RF4CE_GDP_HostAttributesReqDescr_t *reqDescr) {}
//void RF4CE_ZRC_GetAttributesInd(RF4CE_GDP_HostAttributesReqDescr_t *reqDescr) {}
//void RF4CE_ZRC_SetAttributesInd(RF4CE_GDP_HostAttributesReqDescr_t *reqDescr) {}
//void RF4CE_ZRC_GetSharedSecretInd(RF4CE_GDP_GetSharedSecretIndDescr_t *reqDescr) {}
//void RF4CE_ZRC_CheckValidationInd(RF4CE_GDP_CheckValidationIndDescr_t *reqDescr) {}

//void RF4CE_ResetReq(RF4CE_ResetReqDescr_t *request)  { STUB_DUMP_ERROR_MSG; }
void RF4CE_ZRC2_GetAttributesReq(RF4CE_ZRC2_GetAttributesReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
//void RF4CE_ZRC2_SetAttributesReq(RF4CE_ZRC2_SetAttributesReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
void RF4CE_MAC_DataInd_Stub(MAC_DataIndParams_t *indication) { STUB_DUMP_ERROR_MSG; }
//void RF4CE_GDP_PullAttributesReq(RF4CE_GDP_AttributeDescr_t *request) {}
//void RF4CE_GDP_PushAttributesReq(RF4CE_GDP_AttributeDescr_t *request) {}
void RF4CE_ZRC2_KeyExchangeReq(RF4CE_ZRC2_KeyExchangeReqDescr_t *request) { STUB_DUMP_ERROR_MSG;}
void RF4CE_ZRC2_BindReq(RF4CE_ZRC2_BindReqDescr_t *request) { STUB_DUMP_ERROR_MSG;}
void RF4CE_ZRC2_ProxyBindReq(RF4CE_ZRC2_ProxyBindReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
//void RF4CE_ZRC2_EnableBindingReq(RF4CE_ZRC2_BindingReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
void RF4CE_ZRC2_DisableBindingReq(RF4CE_ZRC2_BindingReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
//void RF4CE_ZRC2_ButtonBindingReq(RF4CE_ZRC2_ButtonBindingReqDescr_t *request) {}
void RF4CE_ZRC2_SetPushButtonStimulusReq(RF4CE_ZRC2_ButtonBindingReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
void RF4CE_ZRC2_ClearPushButtonStimulusReq(RF4CE_ZRC2_ButtonBindingReqDescr_t *request) {STUB_DUMP_ERROR_MSG; }
//void RF4CE_ZRC2_CheckValidationResp(RF4CE_ZRC2_CheckValidationRespDescr_t *response) { STUB_DUMP_ERROR_MSG; }
void RF4CE_ZRC2_StartValidationInd(RF4CE_ZRC2_CheckValidationIndParams_t *indication) { STUB_DUMP_ERROR_MSG; }
void RF4CE_GDP2_IdentifyCapAnnounceReq(RF4CE_GDP2_IdentifyCapAnnounceReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
void RF4CE_GDP2_IdentifyCapAnnounceInd(RF4CE_GDP2_IdentifyCapAnnounceIndParams_t *indication) { STUB_DUMP_ERROR_MSG; }
//void RF4CE_ZRC2_PairNtfyInd(RF4CE_PairingIndParams_t *indication) {}
void RF4CE_ZRC2_BindingFinishedNtfyInd(RF4CE_ZRC2_BindingFinishedNtfyIndParams_t *indication) { STUB_DUMP_ERROR_MSG; }

//void RF4CE_GDP_ClientNotificationReq(RF4CE_GDP_ClientNotificationReqDescr_t *request) {}
//void RF4CE_ZRC_GetAttributesReq(RF4CE_GDP_AttributeDescr_t *request) {}
//void RF4CE_ZRC_SetAttributesReq(RF4CE_GDP_AttributeDescr_t *request) {}
//void RF4CE_ZRC_PullAttributesReq(RF4CE_GDP_AttributeDescr_t *request) {}
//void RF4CE_ZRC_PushAttributesReq(RF4CE_GDP_AttributeDescr_t *request) {}
//void RF4CE_ZRC_KeyExchangeReq(RF4CE_GDP_KeyExchangeReqDescr_t *request) {}
//void RF4CE_ZRC_EnableBindingReq(RF4CE_GDP_BindingReqDescr_t *request) {}
//void RF4CE_ZRC_DisableBindingReq(RF4CE_GDP_BindingReqDescr_t *request) {}
//void RF4CE_ZRC2_ButtonBindingReq(RF4CE_ZRC2_ButtonBindingReqDescr_t *request) {}
//void RF4CE_ZRC_ClientNotificationReq(RF4CE_GDP_ClientNotificationReqDescr_t *request) {}
//void RF4CE_ZRC2_CheckValidationInd(RF4CE_ZRC2_CheckValidationIndDescr_t *indication) {}

void Mail_TestEngineHelloInd(TE_HelloCommandIndParams_t *ind)
{
    SYS_EventHandlerMailParams_t eventMap = {0};
    BITMAP_SET(eventMap.subscribedEventsMap, APP_EVENT_00);
    BITMAP_SET(eventMap.subscribedEventsMap, APP_EVENT_01);
    BITMAP_SET(eventMap.subscribedEventsMap, APP_EVENT_02);
    BITMAP_SET(eventMap.subscribedEventsMap, APP_EVENT_03);
    BITMAP_SET(eventMap.subscribedEventsMap, APP_EVENT_04);
    BITMAP_SET(eventMap.subscribedEventsMap, APP_EVENT_05);
    BITMAP_SET(eventMap.subscribedEventsMap, APP_EVENT_06);
    BITMAP_SET(eventMap.subscribedEventsMap, APP_EVENT_07);
    BITMAP_SET(eventMap.subscribedEventsMap, ZBPRO_NEW_CHILD_EID);

    BITMAP_SET(eventMap.subscribedEventsMap, RF4CE_CTRL_EVENT_PAIRING_DEVICE_PAIRED);

    BITMAP_SET(eventMap.subscribedEventsMap, RF4CE_CTRL_EVENT_PAIRING_DEVICE_NOT_FOUND);
    BITMAP_SET(eventMap.subscribedEventsMap, RF4CE_CTRL_EVENT_PAIRING_MULTIPLE_DEVICES_FOUND);
    BITMAP_SET(eventMap.subscribedEventsMap, RF4CE_CTRL_EVENT_PAIRING_DUPLICATE_PAIR_FOUND);
    BITMAP_SET(eventMap.subscribedEventsMap, RF4CE_CTRL_EVENT_UNPAIRING_DEVICE_UNPAIRED);
    BITMAP_SET(eventMap.subscribedEventsMap, RF4CE_CTRL_EVENT_VENDOR_FRAME_RECEIVED);
    BITMAP_SET(eventMap.subscribedEventsMap, RF4CE_CTRL_EVENT_VENDOR_FRAME_SEND_OK);
    BITMAP_SET(eventMap.subscribedEventsMap, RF4CE_CTRL_EVENT_VENDOR_FRAME_SEND_FAILED);
    BITMAP_SET(eventMap.subscribedEventsMap, RF4CE_CTRL_EVENT_BAD_VENDOR_FRAME);
    BITMAP_SET(eventMap.subscribedEventsMap, RF4CE_CTRL_EVENT_PAIRING_GENERAL_FAILURE);
    BITMAP_SET(eventMap.subscribedEventsMap, RF4CE_CTRL_EVENT_TEST_TRANSMIT_END);

    void SYS_EventSubscribe_Call(SYS_EventHandlerMailParams_t*);
    sysEventSubscribeHostHandler_Call(&eventMap);
    //SYS_EventSubscribe_Call(&eventMap);
}
void Mail_TestEnginePing(TE_PingCommandReqDescr_t *const req) { STUB_DUMP_ERROR_MSG; }
void Mail_TestEngineMailboxAckReq(TE_MailboxAckDescr_t *const req) { STUB_DUMP_ERROR_MSG; }
void Mail_TestEngineReset(TE_ResetCommandReqDescr_t *const req) { STUB_DUMP_ERROR_MSG; }
void Mail_TestEngineHaltInd(TE_AssertLogIdCommandIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void TE_RoutingChangeReq(TE_RoutingChangeReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
void Mail_TestEngineSendHello(void) { STUB_DUMP_ERROR_MSG; }
void MailUnitTest_f1(MailUnitTest_f1Descr_t *req) { STUB_DUMP_ERROR_MSG; }
void MailUnitTest_f2(MailUnitTest_f2Descr_t *req) { STUB_DUMP_ERROR_MSG; }
void MailUnitTest_f3(MailUnitTest_f3Descr_t *req) { STUB_DUMP_ERROR_MSG; }
void MailUnitTest_f4(MailUnitTest_f4Descr_t *req) { STUB_DUMP_ERROR_MSG; }
void MailUnitTest_f5(MailUnitTest_f5Descr_t *req) { STUB_DUMP_ERROR_MSG; }
//void RF4CE_GDP_BindReq(RF4CE_GDP_BindReqDescr_t *request) {}
//void RF4CE_GDP_ProxyBindReq(RF4CE_GDP_ProxyBindReqDescr_t *request) {}
//void RF4CE_GDP_EnableHeartbeatReq(RF4CE_GDP_HeartbeatReqDescr_t *request) {}
//void RF4CE_GDP_DisableHeartbeatReq(RF4CE_GDP_HeartbeatReqDescr_t *request) {}
//void RF4CE_ZRC_EnableHeartbeatReq(RF4CE_GDP_HeartbeatReqDescr_t *request) {}
//void RF4CE_ZRC_DisableHeartbeatReq(RF4CE_GDP_HeartbeatReqDescr_t *request) {}
//void RF4CE_ZRC_ControlCommandReq(RF4CE_ZRC_ControlCommandReqDescr_t *request) {}

void MAC_BanTableSetDefaultAction(MAC_BanTableSetDefaultActionReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
void MAC_BanTableAddLink(MAC_BanTableAddLinkReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }

#ifdef _ZBPRO_
void ZBPRO_MAC_DataReq(MAC_DataReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_MAC_PurgeReq(MAC_PurgeReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_MAC_AssociateReq(MAC_AssociateReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_MAC_GetReq(MAC_GetReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_MAC_ResetReq(MAC_ResetReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_MAC_RxEnableReq(MAC_RxEnableReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_MAC_ScanReq(MAC_ScanReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_MAC_SetReq(MAC_SetReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_MAC_StartReq(MAC_StartReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_MAC_AssociateResp(MAC_AssociateRespDescr_t *const respDescr) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_MAC_OrphanResp(MAC_OrphanRespDescr_t *const respDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_NWK_LeaveReq(ZBPRO_NWK_LeaveReqDescr_t *reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_NWK_SetKeyReq(ZBPRO_NWK_SetKeyReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_NWK_GetKeyReq(ZBPRO_NWK_GetKeyReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_NWK_RouteDiscoveryReq(ZBPRO_NWK_RouteDiscoveryReqDescr_t *reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_SapIasAceAlarmInd(ZBPRO_ZCL_SapIasAceAlarmIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
//void RF4CE_NWK_MacStatsReq(RF4CE_NWK_MacStatsReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_APS_EndpointRegisterReq(ZBPRO_APS_EndpointRegisterReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_APS_EndpointUnregisterReq(ZBPRO_APS_EndpointUnregisterReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_APS_DataReq(ZBPRO_APS_DataReqDescr_t *const req) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_APS_DataInd(ZBPRO_APS_DataIndParams_t *indParams) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_APS_BindReq(ZBPRO_APS_BindUnbindReqDescr_t *reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_APS_UnbindReq(ZBPRO_APS_BindUnbindReqDescr_t *reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_APS_GetReq(ZBPRO_APS_GetReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_APS_SetReq(ZBPRO_APS_SetReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_APS_GetKeyReq(ZBPRO_APS_GetKeyReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_APS_SetKeyReq(ZBPRO_APS_SetKeyReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_APS_AddGroupReq(ZBPRO_APS_AddGroupReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_APS_RemoveGroupReq(ZBPRO_APS_RemoveGroupReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_APS_RemoveAllGroupsReq(ZBPRO_APS_RemoveAllGroupsReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_APS_TransportKeyReq(ZBPRO_APS_TransportKeyReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_APS_UpdateDeviceReq(ZBPRO_APS_UpdateDeviceReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_APS_RemoveDeviceReq(ZBPRO_APS_RemoveDeviceReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_APS_RequestKeyReq(ZBPRO_APS_RequestKeyReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_APS_SwitchKeyReq(ZBPRO_APS_SwitchKeyReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZDO_AddrResolvingReq(ZBPRO_ZDO_AddrResolvingReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZDO_NodeDescReq(ZBPRO_ZDO_NodeDescReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZDO_PowerDescReq(ZBPRO_ZDO_PowerDescReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZDO_SimpleDescReq(ZBPRO_ZDO_SimpleDescReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZDO_ActiveEpReq(ZBPRO_ZDO_ActiveEpReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZDO_MatchDescReq(ZBPRO_ZDO_MatchDescReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZDO_DeviceAnnceReq(ZBPRO_ZDO_DeviceAnnceReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZDO_ServerDiscoveryReq(ZBPRO_ZDO_ServerDiscoveryReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZDO_EndDeviceBindReq(ZBPRO_ZDO_EndDeviceBindReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZDO_BindReq(ZBPRO_ZDO_BindUnbindReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZDO_UnbindReq(ZBPRO_ZDO_BindUnbindReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZDO_StartNetworkReq(ZBPRO_ZDO_StartNetworkReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZDO_MgmtLeaveReq(ZBPRO_ZDO_MgmtLeaveReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZDO_MgmtPermitJoiningReq(ZBPRO_ZDO_MgmtPermitJoiningReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZDO_MgmtNwkUpdateReq(ZBPRO_ZDO_MgmtNwkUpdateReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZDO_MgmtBindReq(ZBPRO_ZDO_MgmtBindReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZDO_MgmtNwkUpdateUnsolResp(ZBPRO_ZDO_MgmtNwkUpdateUnsolRespDescr_t *const respDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZDO_MgmtLqiReq(ZBPRO_ZDO_MgmtLqiReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZDO_ReadyInd(ZBPRO_ZDO_ReadyIndParams_t *ind) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZDO_DeviceAnnceInd(ZBPRO_ZDO_DeviceAnnceIndParams_t *ind)
{
    int *sockets = NULL;
    int numSocket = HA_Registration_Get_All_Register_Id(&sockets);
    for(int i = 0; i < numSocket; i++){
        server_ZBPRO_ZDO_DeviceAnnceInd(ind, sockets[i]);
    }
    free(sockets);
}
void ZBPRO_ZDO_MgmtNwkUpdateUnsolInd(ZBPRO_ZDO_MgmtNwkUpdateUnsolIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_TC_NwkKeyUpdateReq(ZBPRO_TC_NwkKeyUpdateReqDescr_t *reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_SetPowerSourceReq(ZBPRO_ZCL_SetPowerSourceReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReq(ZBPRO_ZCL_ProfileWideCmdDiscoverAttrDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ProfileWideCmdReadAttributesReq(ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ProfileWideCmdWriteAttributesReq(ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ProfileWideCmdConfigureReportingReq(ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReq(ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_IdentifyCmdIdentifyReq(ZBPRO_ZCL_IdentifyCmdIdentifyReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_IdentifyCmdIdentifyQueryReq(ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_GroupsCmdAddGroupReq(ZBPRO_ZCL_GroupsCmdAddGroupReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_GroupsCmdViewGroupReq(ZBPRO_ZCL_GroupsCmdViewGroupReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_GroupsCmdGetGroupMembershipReq(ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqDescr_t *const  customReqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_GroupsCmdRemoveGroupReq(ZBPRO_ZCL_GroupsCmdRemoveGroupReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReq(ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReq(ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ScenesCmdAddSceneReq(ZBPRO_ZCL_ScenesCmdAddSceneReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ScenesCmdViewSceneReq(ZBPRO_ZCL_ScenesCmdViewSceneReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ScenesCmdStoreSceneReq(ZBPRO_ZCL_ScenesCmdStoreSceneReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ScenesCmdRecallSceneReq(ZBPRO_ZCL_ScenesCmdRecallSceneReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ScenesCmdRemoveSceneReq(ZBPRO_ZCL_ScenesCmdRemoveSceneReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ScenesCmdRemoveAllScenesReq(ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ScenesCmdGetSceneMembershipReq(ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_OnOffCmdOffReq(ZBPRO_ZCL_OnOffCmdReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_OnOffCmdOnReq(ZBPRO_ZCL_OnOffCmdReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_OnOffCmdToggleReq(ZBPRO_ZCL_OnOffCmdReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_LevelControlCmdMoveToLevelReq(ZBPRO_ZCL_LevelControlCmdMoveToLevelReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_LevelControlCmdMoveReq(ZBPRO_ZCL_LevelControlCmdMoveReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_LevelControlCmdStepReq(ZBPRO_ZCL_LevelControlCmdStepReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_LevelControlCmdStopReq(ZBPRO_ZCL_LevelControlCmdStopReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_DoorLockCmdLockReq(ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_DoorLockCmdUnlockReq(ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_WindowCoveringCmdUpOpenReq(ZBPRO_ZCL_WindowCoveringCmdReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_WindowCoveringCmdDownCloseReq(ZBPRO_ZCL_WindowCoveringCmdReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_WindowCoveringCmdStopReq(ZBPRO_ZCL_WindowCoveringCmdReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_WindowCoveringCmdGotoLiftPecentageReq(ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_WindowCoveringCmdGotoTiltPecentageReq(ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReq(ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_SapIasAceArmRespReq(ZBPRO_ZCL_SapIasAceArmRespReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_SapIasAceBypassRespReq(ZBPRO_ZCL_SapIasAceBypassRespReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReq(ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_SapIasAceGetZoneInfoRespReq(ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_SapIasAceGetPanelStatusRespReq(ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReq(ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_SapIasAceGetZoneStatusRespReq(ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_SapIasAceZoneStatusChangedReq(ZBPRO_ZCL_SapIasAceZoneStatusChangedReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_SapIasAcePanelStatusChangedReq(ZBPRO_ZCL_SapIasAcePanelStatusChangedReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_IASWDCmdStartWarningReq(ZBPRO_ZCL_IASWDCmdStartWarningReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_IASWDCmdSquawkgReq(ZBPRO_ZCL_IASWDCmdSquawkReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ColorControlCmdMoveToColorReq(ZBPRO_ZCL_ColorControlCmdMoveToColorReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ColorControlCmdMoveColorReq(ZBPRO_ZCL_ColorControlCmdMoveColorReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ColorControlCmdStepColorReq(ZBPRO_ZCL_ColorControlCmdStepColorReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReq(ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReq(ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReq(ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReq(ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ColorControlCmdColorLoopSetReq(ZBPRO_ZCL_ColorControlCmdColorLoopSetReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ColorControlCmdStopMoveStepReq(ZBPRO_ZCL_ColorControlCmdStopMoveStepReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReq(ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReq(ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }

void ZBPRO_ZCL_OTAUCmdQueryNextImageRequestInd(ZBPRO_ZCL_OTAUCmdQueryNextImageRequestIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_OTAUCmdQueryNextImageResponseReq(ZBPRO_ZCL_OTAUCmdQueryNextImageResponseReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_OTAUCmdImageBlockRequestInd(ZBPRO_ZCL_OTAUCmdImageBlockRequestIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_OTAUCmdImageBlockResponseReq(ZBPRO_ZCL_OTAUCmdImageBlockResponseReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_OTAUCmdUpgradeEndRequestInd(ZBPRO_ZCL_OTAUCmdUpgradeEndRequestIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_OTAUCmdUpgradeEndResponseReq(ZBPRO_ZCL_OTAUCmdUpgradeEndResponseReqDescr_t *const  reqDescr) { STUB_DUMP_ERROR_MSG; }

void ZBPRO_ZCL_IdentifyInd(ZBPRO_ZCL_IdentifyIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseIndEB(ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_GroupsCmdGetGroupMembershipResponseInd(ZBPRO_ZCL_GroupsCmdGetGroupMembershipIndParams_t   *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseInd(ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseIndParams_t *const  indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestInd(ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }

void ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationInd(ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationIndParams_t *const indParams)
{
    if(indParams->zclObligatoryPart.localEndpoint == 0xff){ /* broadcast endpoint*/
        int *sockets = NULL;
        int numSocket = HA_Registration_Get_All_Register_Id(&sockets);
        for(int i = 0; i < numSocket; i++){
            server_ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationInd(indParams, sockets[i]);
        }
        free(sockets);
    }else{
        int clientId = HA_Registration_Get_Register_Id(indParams->zclObligatoryPart.localEndpoint);
        if(clientId == HA_INVALID_REGISTER_ID){
            STUB_DUMP_ERROR_MSG;
        }
        else{
            server_ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationInd(indParams, clientId);
        }
    }
}

void ZBPRO_ZCL_SapIasAceArmInd(ZBPRO_ZCL_SapIasAceArmIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_SapIasAceBypassInd(ZBPRO_ZCL_SapIasAceBypassIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_SapIasAceGetZoneIdMapInd(ZBPRO_ZCL_SapIasAceGetZoneIdMapIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_SapIasAceGetZoneInfoInd(ZBPRO_ZCL_SapIasAceGetZoneInfoIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_SapIasAceGetPanelStatusInd(ZBPRO_ZCL_SapIasAceGetPanelStatusIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_SapIasAceGetBypassedZoneListInd(ZBPRO_ZCL_SapIasAceGetBypassedZoneListIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_SapIasAceGetZoneStatusInd(ZBPRO_ZCL_SapIasAceGetZoneStatusIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_ProfileWideCmdReportAttributesInd(ZBPRO_ZCL_ProfileWideCmdReportAttributesIndParams_t *const  indParams)
{
    int clientId = HA_Registration_Get_Register_Id(indParams->zclObligatoryPart.localEndpoint);
    if(clientId == HA_INVALID_REGISTER_ID){
        STUB_DUMP_ERROR_MSG;
    }
    else{
        server_ZBPRO_ZCL_ProfileWideCmdReportAttributesInd(indParams, clientId);
    }
}
void ZBPRO_ZCL_SapIasAceEmergencyInd(ZBPRO_ZCL_SapIasAceAlarmIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_SapIasAceFireInd(ZBPRO_ZCL_SapIasAceAlarmIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_SapIasAcePanicInd(ZBPRO_ZCL_SapIasAceAlarmIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZHA_CieDeviceEnrollInd(ZBPRO_ZHA_CieEnrollIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestForMailboxInd(ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationForMailboxInd(ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_SapIasAceArmForMailboxInd(
                   ZBPRO_ZCL_SapIasAceArmIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_SapIasAceBypassForMailboxInd(
                   ZBPRO_ZCL_SapIasAceBypassIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_SapIasAceEmergencyForMailboxInd(
                   ZBPRO_ZCL_SapIasAceAlarmIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_SapIasAceFireForMailboxInd(
                   ZBPRO_ZCL_SapIasAceAlarmIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_SapIasAcePanicForMailboxInd(
                   ZBPRO_ZCL_SapIasAceAlarmIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_SapIasAceGetZoneIdMapForMailboxInd(
                   ZBPRO_ZCL_SapIasAceGetZoneIdMapIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_SapIasAceGetZoneInfoForMailboxInd(
                   ZBPRO_ZCL_SapIasAceGetZoneInfoIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_SapIasAceGetPanelStatusForMailboxInd(
                   ZBPRO_ZCL_SapIasAceGetPanelStatusIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_SapIasAceGetBypassedZoneListForMailboxInd(
                   ZBPRO_ZCL_SapIasAceGetBypassedZoneListIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZCL_SapIasAceGetZoneStatusForMailboxInd(
                   ZBPRO_ZCL_SapIasAceGetZoneStatusIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZHA_CieDeviceRegisterReq(ZBPRO_ZHA_CieDeviceRegisterReqDescr_t * const descr)  { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZHA_CieDeviceUnregisterReq(ZBPRO_ZHA_CieDeviceUnregisterReqDescr_t * const descr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZHA_EzModeReq(ZBPRO_ZHA_EzModeReqDescr_t *reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZHA_CieDeviceEnrollReq(ZBPRO_ZHA_CieEnrollReqDescr_t *reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZHA_CieDeviceSetPanelStatusReq(ZBPRO_ZHA_CieSetPanelStatusReqDescr_t * const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_ZHA_CieZoneSetBypassStateReq(ZBPRO_ZHA_CieZoneSetBypassStateReqDescr_t   *const descr) { STUB_DUMP_ERROR_MSG; }
void ZBPRO_ZHA_CieDeviceSetPanelStatusInd(ZBPRO_ZHA_CieSetPanelStatusIndParams_t   *const indParams) { STUB_DUMP_ERROR_MSG; }
#endif

#ifdef _RF4CE_
void RF4CE_MAC_DataReq(MAC_DataReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void RF4CE_MAC_GetReq(MAC_GetReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
void RF4CE_MAC_ResetReq(MAC_ResetReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
void RF4CE_MAC_RxEnableReq(MAC_RxEnableReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
//void RF4CE_MAC_SetReq(MAC_SetReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
void RF4CE_MAC_StartReq(MAC_StartReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
void RF4CE_MAC_ScanReq(MAC_ScanReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
void RF4CE_NWK_ResetReq(RF4CE_NWK_ResetReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
void RF4CE_NWK_StartReq(RF4CE_NWK_StartReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
//void RF4CE_NWK_DataReq(RF4CE_NWK_DataReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
//void RF4CE_NWK_SetReq(RF4CE_NWK_SetReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
void RF4CE_MAC_DataInd(MAC_DataIndParams_t *const indParams)
{
    printf("MAC Data Indication dumping\n");
    uint8_t length = SYS_GetPayloadSize(&indParams->payload);
    uint8_t ch;
    for(uint8_t i = 0; i < length; i++){
        SYS_CopyFromPayload(&ch, &indParams->payload, i, 1);
        printf(" %02X ", ch);
    }
    printf("\n");
}

void RF4CE_NWK_RXEnableReq(RF4CE_NWK_RXEnableReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
void RF4CE_NWK_UpdateKeyReq(RF4CE_NWK_UpdateKeyReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
void RF4CE_NWK_UnpairReq(RF4CE_NWK_UnpairReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
void RF4CE_NWK_UnpairResp(RF4CE_NWK_UnpairRespDescr_t *response) { STUB_DUMP_ERROR_MSG; }
void RF4CE_NWK_DiscoveryReq(RF4CE_NWK_DiscoveryReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
void RF4CE_NWK_PairReq(RF4CE_NWK_PairReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
void RF4CE_NWK_AutoDiscoveryReq(RF4CE_NWK_AutoDiscoveryReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
void RF4CE_NWK_DiscoveryResp(RF4CE_NWK_DiscoveryRespDescr_t *response) { STUB_DUMP_ERROR_MSG; }
void RF4CE_NWK_PairResp(RF4CE_NWK_PairRespDescr_t *response) { STUB_DUMP_ERROR_MSG; }
//void ZBPRO_NWK_PermitJoiningReq(ZBPRO_NWK_PermitJoiningReqDescr_t *reqDescr) { STUB_DUMP_ERROR_MSG; }
//void RF4CE_SetSupportedDevicesReq(RF4CE_SetSupportedDevicesReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
void RF4CE_CounterExpiredInd(RF4CE_PairingReferenceIndParams_t *indication) { STUB_DUMP_ERROR_MSG; }
void RF4CE_UnpairInd(RF4CE_PairingReferenceIndParams_t *indication) { STUB_DUMP_ERROR_MSG; }
//void RF4CE_ZRC1_GetAttributesReq(RF4CE_ZRC1_GetAttributeDescr_t *request) { STUB_DUMP_ERROR_MSG; }
//void RF4CE_ZRC1_SetAttributesReq(RF4CE_ZRC1_SetAttributeDescr_t *request) { STUB_DUMP_ERROR_MSG; }
void RF4CE_ZRC1_CommandDiscoveryReq(RF4CE_ZRC1_CommandDiscoveryReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
void RF4CE_ZRC1_ControlCommandPressedReq(RF4CE_ZRC1_ControlCommandReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
void RF4CE_ZRC1_ControlCommandReleasedReq(RF4CE_ZRC1_ControlCommandReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
void RF4CE_ZRC1_ControllerBindReq(RF4CE_ZRC1_BindReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
//void RF4CE_ZRC1_VendorSpecificReq(RF4CE_ZRC1_VendorSpecificReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
//void RF4CE_ZRC1_TargetBindReq(RF4CE_ZRC1_BindReqDescr_t *request) {}
//void RF4CE_ZRC1_ControlCommandInd(RF4CE_ZRC1_ControlCommandIndParams_t *indication) {}
//void RF4CE_ZRC1_VendorSpecificInd(RF4CE_ZRC1_VendorSpecificIndParams_t *indication) { STUB_DUMP_ERROR_MSG; }
//void NVM_ReadFileInd(NVM_ReadFileIndDescr_t *indDescr) {}
//void NVM_WriteFileInd(NVM_WriteFileIndDescr_t *indDescr) {}
//void NVM_OpenFileInd(NVM_OpenFileIndDescr_t *indDescr) {}
//void NVM_CloseFileInd(NVM_CloseFileIndDescr_t *indDescr) {}
//void Mail_TestEngineSendHello(MailDescriptor_t *const mail) {}
void RF4CE_ZRC2_ControlCommandPressedReq(RF4CE_ZRC2_ControlCommandReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
void RF4CE_ZRC2_ControlCommandReleasedReq(RF4CE_ZRC2_ControlCommandReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
void RF4CE_ZRC2_GetAttrRespInd(RF4CE_ZRC2_SetAttributesReqParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void RF4CE_ZRC2_PushAttrReqInd(RF4CE_ZRC2_SetAttributesReqParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }

//void RF4CE_ZRC_SetWakeUpActionCodeReq(RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void RF4CE_ZRC_GetWakeUpActionCodeReq(RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
void Mail_TestEngineAssertLogIdInd(TE_AssertLogIdCommandIndParams_t *const indParams)
{
    printf("DebugLogId : 0x%08x\n", indParams->number);
}
//void RF4CE_CounterExpiredInd(RF4CE_PairingReferenceIndParams_t *indication) {}
//void RF4CE_UnpairInd(RF4CE_PairingReferenceIndParams_t *indication) {}
//void RF4CE_ZRC1_ControlCommandInd(RF4CE_ZRC1_ControlCommandIndParams_t *indication) {}
//void RF4CE_ZRC1_VendorSpecificInd(RF4CE_ZRC1_VendorSpecificIndParams_t *indication) {}
void RF4CE_GDP2_SetPollConstraintsReq(RF4CE_GDP2_SetPollConstraintsReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
void RF4CE_GDP2_PollNegotiationReq(RF4CE_GDP2_PollNegotiationReqDescr_t *const reqDescr) {}
void RF4CE_GDP2_PollClientUserEventReq(RF4CE_GDP2_PollClientUserEventReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
void RF4CE_GDP2_ClientNotificationReq(RF4CE_GDP2_ClientNotificationReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
void RF4CE_GDP2_PollNegotiationInd(RF4CE_GDP2_PollNegotiationIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void RF4CE_GDP2_HeartbeatInd(RF4CE_GDP2_HeartbeatIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void RF4CE_GDP2_ClientNotificationInd(RF4CE_GDP2_ClientNotificationIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
void RF4CE_GDP2_IdentifyReq(RF4CE_GDP2_IdentifyReqDescr_t *const reqDescr) { STUB_DUMP_ERROR_MSG; }
void RF4CE_GDP2_IdentifyInd(RF4CE_GDP2_IdentifyIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
/************************* STUBS for MAC ************************************************/

//void MailUartRxInterruptHandler(TE_Host2Uart1ReqDescr_t *const req) {}

/* void Mail_Uart1ToHostInd(TE_Uart1ToHostReqParams_t *ind){ */
/*     uint8_t length = SYS_GetPayloadSize(&ind->payload); */
/*     uint8_t ch; */
/*     for(uint8_t i = 0; i < length; i++){ */
/*         SYS_CopyFromPayload(&ch, &ind->payload, i, 1); */
/*         printf("%c", ch); */
/*     } */

/* } */

void RF4CE_GDP_StartValidationInd(RF4CE_PairingReferenceProfileIdIndParams_t *indication) { STUB_DUMP_ERROR_MSG; }
//void RF4CE_GDP_ClientNotificationInd(RF4CE_GDP_ClientNotificationIndParams_t *indication) {}
//void RF4CE_GDP_HeartbeatInd(RF4CE_GDP_HeartbeatIndParams_t *indication) {}
void RF4CE_ZRC_StartValidationInd(RF4CE_PairingReferenceProfileIdIndParams_t *indication) { STUB_DUMP_ERROR_MSG; }
//void RF4CE_ZRC_ClientNotificationInd(RF4CE_GDP_ClientNotificationIndParams_t *indication) {}
//void RF4CE_ZRC_HeartbeatInd(RF4CE_GDP_HeartbeatIndParams_t *indication) {}
//void RF4CE_NWK_GetReq(RF4CE_NWK_GetReqDescr_t *request) { STUB_DUMP_ERROR_MSG; }
#endif

void SYS_PrintInd(SYS_PrintIndParams_t *indication) { STUB_DUMP_ERROR_MSG; }
//void Mail_TestEngineHaltInd(TE_AssertLogIdCommandIndParams_t *const indParams) { STUB_DUMP_ERROR_MSG; }
#include <stdio.h>

static void dumpMessage(uint8_t *msgBuf)
{
    uint8_t size = msgBuf[0];
    for(int i = 0; i < size; i++)
        printf(" %02x ", msgBuf[i + 1]);
    printf("\n");
}
void SYS_EventNtfy(SYS_EventNotifyParams_t *const event)
{
    for (int i = 0; i < MAX_SOCKETS; i++){
        int socket = i;
        if(Zigbee_Socket_Is_Idle(socket))
            server_SYS_EventNtfy(event, socket);
    }
    switch(event->id){
        case APP_EVENT_00:
            break;
        case APP_EVENT_01:
            break;
        case APP_EVENT_02:
            break;
        case APP_EVENT_03:
            break;
        case APP_EVENT_04:
            break;
        case APP_EVENT_05:
            break;
        case APP_EVENT_06:
            break;
        default:
            printf("\nSYS_EventNtfy : EventId(%d)\n", event->id);
            break;

    }
}

void RF4CE_ZRC2_GetSharedSecretInd(RF4CE_ZRC2_GetSharedSecretIndDescr_t *request) { STUB_DUMP_ERROR_MSG; }

//void Phy_Test_Get_Caps_Req(Phy_Test_Get_Caps_ReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void Phy_Test_Set_Channel_Req(Phy_Test_Set_Channel_ReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void Phy_Test_Continuous_Wave_Start_Req(Phy_Test_Continuous_Wave_Start_ReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void Phy_Test_Continuous_Wave_Stop_Req(Phy_Test_Continuous_Wave_Stop_ReqDescr_t *req)  { STUB_DUMP_ERROR_MSG; }
//void Phy_Test_Transmit_Start_Req(Phy_Test_Transmit_Start_ReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void Phy_Test_Transmit_Stop_Req(Phy_Test_Transmit_Stop_ReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void Phy_Test_Receive_Start_Req(Phy_Test_Receive_Start_ReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void Phy_Test_Receive_Stop_Req(Phy_Test_Receive_Stop_ReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void Phy_Test_Echo_Start_Req(Phy_Test_Echo_Start_ReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void Phy_Test_Echo_Stop_Req(Phy_Test_Echo_Stop_ReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void Phy_Test_Energy_Detect_Scan_Req(Phy_Test_Energy_Detect_Scan_ReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void Phy_Test_Get_Stats_Req(Phy_Test_Get_Stats_ReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void Phy_Test_Reset_Stats_Req(Phy_Test_Reset_Stats_ReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void Phy_Test_Set_TX_Power_Req(Phy_Test_Set_TX_Power_ReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void Phy_Test_SelectAntenna_Req(Phy_Test_Select_Antenna_ReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void RF4CE_Get_Diag_Caps_Req(RF4CE_Diag_Caps_ReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }
//void RF4CE_Get_Diag_Req(RF4CE_Diag_ReqDescr_t *req) { STUB_DUMP_ERROR_MSG; }

#ifdef BYPASS_RPC
#include "zigbee.h"
#endif

void Mail_UartRecvInd(Mail_UartRecvIndDescr_t *const ind)
{
#ifdef BYPASS_RPC
    if(Zigbee_GetCallback()->Mail_UartRecvInd)
        Zigbee_GetCallback()->Mail_UartRecvInd(ind);
    //server_Mail_UartRecvInd(ind, 0);
#else
    for (int i = 0; i < MAX_SOCKETS; i++){
        int socket = i;
        if(Zigbee_Socket_Is_Idle(socket))
            server_Mail_UartRecvInd(ind, socket);
    }
#endif
}


/* eof bbPrjStubs.c */
