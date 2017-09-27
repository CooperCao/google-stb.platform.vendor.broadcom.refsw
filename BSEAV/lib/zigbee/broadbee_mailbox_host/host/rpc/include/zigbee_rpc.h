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
 ******************************************************************************/

#ifndef _ZIGBEE_RPC_H_
#define _ZIGBEE_RPC_H_

/* 10 bit code, used in message header */
typedef enum zigbeeRpcCode {
    RPC_C2S_Open_Request = 0,

    RPC_C2S_FW_Rev_GetReq,

    /* 4.2.7 */
    RPC_S2C_RF4CE_ZRC_PairInd,
    RPC_S2C_RF4CE_ZRC2_CheckValidationInd,

    /* 4.2.8 */
    RPC_S2C_RF4CE_ZRC2_ControlCommandInd,

    /* MAC */
    RPC_C2S_RF4CE_MAC_GetReq,

    RPC_C2S_RF4CE_MAC_SetReq,
    /* 6.1 */
    RPC_C2S_RF4CE_ResetReq,

    RPC_C2S_RF4CE_SetSupportedDevicesReq,

    RPC_C2S_RF4CE_StartReq,

    RPC_C2S_RF4CE_DataReq,

    RPC_C2S_RF4CE_GetReq,

    RPC_C2S_RF4CE_SetReq,

    RPC_C2S_RF4CE_ZRC1_GetAttributesReq,

    RPC_C2S_RF4CE_ZRC1_SetAttributesReq,

    /* 6.20 */
    RPC_C2S_HA_EnterNetworkReq,

    RPC_C2S_Mail_TestEngineEcho,

    RPC_C2S_SYS_EventSubscribe,

    RPC_C2S_Mail_SetEchoDelay,

    RPC_C2S_RF4CE_RegisterVirtualDevice,

    RPC_C2S_RF4CE_ZRC_SetWakeUpActionCodeKey,

    RPC_C2S_RF4CE_ZRC_GetWakeUpActionCodeKey,

    RPC_C2S_RF4CE_ZRC1_TargetBindReq,

    RPC_C2S_RF4CE_ZRC2_SetPushButtonStimulusReq,

    RPC_C2S_RF4CE_ZRC2_EnableBindingReq,

    RPC_C2S_RF4CE_ZRC2_SetAttributesReq,

    RPC_C2S_RF4CE_ZRC2_CheckValidationResp,

    RPC_S2C_RF4CE_ZRC1_ControlCommandInd,

    RPC_C2S_RF4CE_ZRC1_VendorSpecificReq,

    RPC_S2C_RF4CE_ZRC1_VendorSpecificInd,

    RPC_C2S_RF4CE_UnpairReq,

    RPC_C2S_TE_Host2Uart1Req,

    RPC_C2S_Phy_Test_Get_Caps_Req,
    RPC_C2S_Phy_Test_Set_Channel_Req,
    RPC_C2S_Phy_Test_Continuous_Wave_Start_Req,
    RPC_C2S_Phy_Test_Continuous_Wave_Stop_Req,
    RPC_C2S_Phy_Test_Transmit_Start_Req,
    RPC_C2S_Phy_Test_Transmit_Stop_Req,
    RPC_C2S_Phy_Test_Receive_Start_Req,
    RPC_C2S_Phy_Test_Receive_Stop_Req,
    RPC_C2S_Phy_Test_Echo_Start_Req,
    RPC_C2S_Phy_Test_Echo_Stop_Req,
    RPC_C2S_Phy_Test_Energy_Detect_Scan_Req,
    RPC_C2S_Phy_Test_Get_Stats_Req,
    RPC_C2S_Phy_Test_Reset_Stats_Req,
    RPC_C2S_Phy_Test_Set_TX_Power_Req,
    RPC_C2S_Phy_Test_SelectAntenna_Req,
    RPC_C2S_RF4CE_Get_Diag_Caps_Req,
    RPC_C2S_RF4CE_Get_Diag_Req,

    RPC_S2C_Mail_TestEngineReset,
    RPC_S2C_SYS_EVENTNTFY,
    /* NWK */
    RPC_C2S_ZBPRO_NWK_PermitJoiningReq,
    RPC_C2S_ZBPRO_NWK_LeaveReq,
    RPC_C2S_ZBPRO_NWK_GetKeyReq,
    RPC_C2S_ZBPRO_NWK_SetKeyReq,
    RPC_C2S_ZBPRO_NWK_RouteDiscoveryReq,

    /* APS */
    RPC_C2S_ZBPRO_APS_EndpointRegisterReq,
    RPC_C2S_ZBPRO_APS_EndpointUnregisterReq,
    RPC_C2S_ZBPRO_APS_DataReq,
    RPC_C2S_ZBPRO_APS_BindReq,
    RPC_C2S_ZBPRO_APS_UnbindReq,
    RPC_C2S_ZBPRO_APS_GetReq,
    RPC_C2S_ZBPRO_APS_SetReq,
    RPC_C2S_ZBPRO_APS_GetKeyReq,
    RPC_C2S_ZBPRO_APS_SetKeyReq,
    RPC_C2S_ZBPRO_APS_AddGroupReq,
    RPC_C2S_ZBPRO_APS_RemoveGroupReq,
    RPC_C2S_ZBPRO_APS_RemoveAllGroupsReq,
    RPC_C2S_ZBPRO_APS_TransportKeyReq,
    RPC_C2S_ZBPRO_APS_UpdateDeviceReq,
    RPC_C2S_ZBPRO_APS_RemoveDeviceReq,
    RPC_C2S_ZBPRO_APS_RequestKeyReq,
    RPC_C2S_ZBPRO_APS_SwitchKeyReq,
    /* ZDO */
    RPC_C2S_ZBPRO_ZDO_AddrResolvingReq,
    RPC_C2S_ZBPRO_ZDO_NodeDescReq,
    RPC_C2S_ZBPRO_ZDO_PowerDescReq,
    RPC_C2S_ZBPRO_ZDO_SimpleDescReq,
    RPC_C2S_ZBPRO_ZDO_ActiveEpReq,
    RPC_C2S_ZBPRO_ZDO_MatchDescReq,
    RPC_C2S_ZBPRO_ZDO_DeviceAnnceReq,
    RPC_C2S_ZBPRO_ZDO_EndDeviceBindReq,
    RPC_C2S_ZBPRO_ZDO_BindReq,
    RPC_C2S_ZBPRO_ZDO_UnbindReq,
    RPC_C2S_ZBPRO_ZDO_StartNetworkReq,
    RPC_C2S_ZBPRO_ZDO_MgmtLeaveReq,
    RPC_C2S_ZBPRO_ZDO_MgmtPermitJoiningReq,
    RPC_C2S_ZBPRO_ZDO_MgmtNwkUpdateReq,
    RPC_C2S_ZBPRO_ZDO_MgmtNwkUpdateUnsolResp,
    RPC_C2S_ZBPRO_ZDO_MgmtLqiReq,
    RPC_C2S_ZBPRO_ZDO_MgmtBindReq,
    RPC_C2S_ZBPRO_ZDO_MgmtNwkUpdateUnsolInd,
    RPC_C2S_ZBPRO_ZDO_DeviceAnnceInd,
    /* ZCL */
    RPC_C2S_ZBPRO_TC_NwkKeyUpdateReq,
    RPC_C2S_ZBPRO_ZCL_SetPowerSourceReq,
    RPC_C2S_ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReq,
    RPC_C2S_ZBPRO_ZCL_ProfileWideCmdReadAttributesReq,
    RPC_C2S_ZBPRO_ZCL_ProfileWideCmdWriteAttributesReq,
    RPC_C2S_ZBPRO_ZCL_ProfileWideCmdConfigureReportingReq,
    RPC_C2S_ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReq,
    RPC_C2S_ZBPRO_ZCL_IdentifyCmdIdentifyReq,
    RPC_C2S_ZBPRO_ZCL_IdentifyCmdIdentifyQueryReq,
    RPC_C2S_ZBPRO_ZCL_GroupsCmdAddGroupReq,
    RPC_C2S_ZBPRO_ZCL_GroupsCmdViewGroupReq,
    RPC_C2S_ZBPRO_ZCL_GroupsCmdGetGroupMembershipReq,
    RPC_C2S_ZBPRO_ZCL_GroupsCmdRemoveGroupReq,
    RPC_C2S_ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReq,
    RPC_C2S_ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReq,
    RPC_C2S_ZBPRO_ZCL_ScenesCmdAddSceneReq,
    RPC_C2S_ZBPRO_ZCL_ScenesCmdViewSceneReq,
    RPC_C2S_ZBPRO_ZCL_ScenesCmdStoreSceneReq,
    RPC_C2S_ZBPRO_ZCL_ScenesCmdRecallSceneReq,
    RPC_C2S_ZBPRO_ZCL_ScenesCmdRemoveSceneReq,
    RPC_C2S_ZBPRO_ZCL_ScenesCmdRemoveAllScenesReq,
    RPC_C2S_ZBPRO_ZCL_ScenesCmdGetSceneMembershipReq,
    RPC_C2S_ZBPRO_ZCL_OnOffCmdOffReq,
    RPC_C2S_ZBPRO_ZCL_OnOffCmdOnReq,
    RPC_C2S_ZBPRO_ZCL_OnOffCmdToggleReq,
    RPC_C2S_ZBPRO_ZCL_LevelControlCmdMoveToLevelReq,
    RPC_C2S_ZBPRO_ZCL_LevelControlCmdMoveReq,
    RPC_C2S_ZBPRO_ZCL_LevelControlCmdStepReq,
    RPC_C2S_ZBPRO_ZCL_LevelControlCmdStopReq,
    RPC_C2S_ZBPRO_ZCL_DoorLockCmdLockReq,
    RPC_C2S_ZBPRO_ZCL_DoorLockCmdUnlockReq,
    RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdUpOpenReq,
    RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdDownCloseReq,
    RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdStopReq,
    RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdGotoLiftPecentageReq,
    RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdGotoTiltPecentageReq,
    RPC_C2S_ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReq,
    RPC_C2S_ZBPRO_ZCL_SapIasAceArmRespReq,
    RPC_C2S_ZBPRO_ZCL_SapIasAceBypassRespReq,
    RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReq,
    RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneInfoRespReq,
    RPC_C2S_ZBPRO_ZCL_SapIasAceGetPanelStatusRespReq,
    RPC_C2S_ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReq,
    RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneStatusRespReq,
    RPC_C2S_ZBPRO_ZCL_SapIasAceZoneStatusChangedReq,
    RPC_C2S_ZBPRO_ZCL_SapIasAcePanelStatusChangedReq,
    RPC_C2S_ZBPRO_ZCL_IASWDCmdStartWarningReq,
    RPC_C2S_ZBPRO_ZCL_IASWDCmdSquawkReq,
    RPC_C2S_ZBPRO_ZCL_IdentifyInd,
    RPC_C2S_ZBPRO_ZCL_ProfileWideCmdReportAttributesInd,
    RPC_C2S_ZBPRO_ZCL_GroupsCmdGetGroupMembershipResponseInd,
    RPC_C2S_ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseInd,
    RPC_C2S_ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestInd,
    RPC_C2S_ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationInd,
    RPC_C2S_ZBPRO_ZCL_SapIasAceArmInd,
    RPC_C2S_ZBPRO_ZCL_SapIasAceBypassInd,
    RPC_C2S_ZBPRO_ZCL_SapIasAceEmergencyInd,
    RPC_C2S_ZBPRO_ZCL_SapIasAceFireInd,
    RPC_C2S_ZBPRO_ZCL_SapIasAcePanicInd,
    RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneIdMapInd,
    RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneInfoInd,
    RPC_C2S_ZBPRO_ZCL_SapIasAceGetPanelStatusInd,
    RPC_C2S_ZBPRO_ZCL_SapIasAceGetBypassedZoneListInd,
    RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneStatusInd,
    RPC_C2S_ZBPRO_ZCL_ColorControlCmdMoveToColorReq,
    RPC_C2S_ZBPRO_ZCL_ColorControlCmdMoveColorReq,
    RPC_C2S_ZBPRO_ZCL_ColorControlCmdStepColorReq,
    RPC_C2S_ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReq,
    RPC_C2S_ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReq,
    RPC_C2S_ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReq,
    RPC_C2S_ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReq,
    RPC_C2S_ZBPRO_ZCL_ColorControlCmdColorLoopSetReq,
    RPC_C2S_ZBPRO_ZCL_ColorControlCmdStopMoveStepReq,
    RPC_C2S_ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReq,
    RPC_C2S_ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReq,
    RPC_C2S_ZBPRO_ZHA_EzModeReq,
    RPC_C2S_ZBPRO_ZHA_CieDeviceEnrollReq,
    RPC_C2S_ZBPRO_ZHA_CieDeviceSetPanelStatusReq,
    RPC_C2S_ZBPRO_ZHA_CieZoneSetBypassStateReq,
    RPC_C2S_ZBPRO_ZHA_CieDeviceSetPanelStatusInd,
#ifdef TEST
    RPC_C2S_ClientRawBufferLoopback,    /* Send raw data (no parameters) to the server and back */
    RPC_C2S_ClientCoreLoopbackReq,    /* Send buffer to the core and back */
    RPC_C2S_ClientLoopbackReq,
    RPC_C2S_ServerLoopbackStart,
    RPC_S2C_ServerLoopbackInd,
#endif
    RPC_C2S_Mail_UartSendReq,
    RPC_C2S_Mail_UartRecvInd,
    RPC_C2S_ZBPRO_MAC_GetReq = RPC_C2S_RF4CE_MAC_GetReq
} zigbeeRpcCode;

#define RPC_RESPONSE 0x200

void Zigbee_Rpc_Send(unsigned int *pData, int size_in_words, uint32_t code, void (*sendBuffer)(unsigned int *, int, int), int socket);
int Zigbee_Rpc_Receive(unsigned int *message_buffer, int *socket, int (*recvBuffer)(unsigned int *, int *, int));

#endif /*_ZIGBEE_RPC_H_*/

/* eof zigbee_rpc.h */
