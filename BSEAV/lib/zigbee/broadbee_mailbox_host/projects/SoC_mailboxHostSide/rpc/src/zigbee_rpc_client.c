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
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include "bbMailAPI.h"
#include "zigbee_api.h"
#include "zigbee_rpc.h"
#include "zigbee_common.h"
#include "zigbee_rpc_frame.h"
#include "zigbee_socket.h"
#include "zigbee_socket_client.h"

static zigbeeCallback *g_pZigbeeCallback = NULL;
static pthread_t zigbeeThread;

static void process_rx_message(unsigned int *message_rx)
{
    unsigned int *message_payload = message_rx+RPC_FRAME_PAYLOAD_OFFSET;
    unsigned int message_id = RPC_FRAME_GET_MSG_ID(message_rx[RPC_FRAME_HEADER_OFFSET]);

    switch(message_id) {

    case RPC_S2C_RF4CE_ZRC_PairInd:
        client_RF4CE_PairInd(message_payload);
        break;

    case RPC_S2C_RF4CE_ZRC2_CheckValidationInd:
        client_RF4CE_ZRC2_CheckValidationInd(message_payload);
        break;

    case RPC_S2C_RF4CE_ZRC2_ControlCommandInd: /* 4.2.8 */
        client_RF4CE_ZRC2_ControlCommandInd(message_payload);
        break;
    case RPC_S2C_RF4CE_ZRC1_ControlCommandInd: /* 4.2.8 */
        client_RF4CE_ZRC1_ControlCommandInd(message_payload);
        break;
    case (RPC_C2S_RF4CE_ZRC1_VendorSpecificReq | RPC_RESPONSE):
        client_RF4CE_ZRC1_VendorSpecificReq_callback(message_payload);
        break;
    case RPC_S2C_RF4CE_ZRC1_VendorSpecificInd:
        client_RF4CE_ZRC1_VendorSpecificInd(message_payload);
        break;
#ifdef TEST
    case RPC_S2C_ServerLoopbackInd:
        client_ServerLoopbackInd(g_pZigbeeCallback, message_payload);
        break;

    case (RPC_C2S_ClientRawBufferLoopback | RPC_RESPONSE):
        {
            unsigned int message_payload_size_in_words = message_rx[RPC_FRAME_LENGTH_OFFSET]-RPC_FRAME_HEADER_SIZE_IN_WORDS;
            client_ClientRawBufferLoopback_callback(g_pZigbeeCallback, message_payload, message_payload_size_in_words);
        }
        break;

    case (RPC_C2S_ClientCoreLoopbackReq | RPC_RESPONSE):
        client_ClientCoreLoopbackReq_callback(message_payload);
        break;

    case (RPC_C2S_ClientLoopbackReq | RPC_RESPONSE):
        client_ClientLoopback_callback(message_payload);
        break;
#endif
    case (RPC_C2S_RF4CE_MAC_GetReq | RPC_RESPONSE):
        client_RF4CE_MAC_GetReq_callback(message_payload);
        break;
    case (RPC_C2S_RF4CE_MAC_SetReq | RPC_RESPONSE):
        client_RF4CE_MAC_SetReq_callback(message_payload);
        break;
    case (RPC_C2S_RF4CE_ResetReq | RPC_RESPONSE):
        client_RF4CE_ResetReq_callback(message_payload);
        break;
    case (RPC_C2S_RF4CE_SetSupportedDevicesReq | RPC_RESPONSE):
        client_RF4CE_SetSupportedDevicesReq_callback(message_payload);
        break;
    case (RPC_C2S_RF4CE_StartReq | RPC_RESPONSE):
        client_RF4CE_StartReq_callback(message_payload);
        break;
    case (RPC_C2S_RF4CE_SetReq | RPC_RESPONSE):
        client_RF4CE_NWK_SetReq_callback(message_payload);
        break;
    case (RPC_C2S_RF4CE_GetReq | RPC_RESPONSE):
        client_RF4CE_NWK_GetReq_callback(message_payload);
        break;

    case (RPC_C2S_RF4CE_ZRC1_GetAttributesReq | RPC_RESPONSE):
        client_RF4CE_ZRC1_GetAttributesReq_callback(message_payload);
        break;

    case (RPC_C2S_RF4CE_ZRC1_SetAttributesReq | RPC_RESPONSE):
        client_RF4CE_ZRC1_SetAttributesReq_callback(message_payload);
        break;

    case (RPC_C2S_Mail_TestEngineEcho | RPC_RESPONSE):
        client_Mail_TestEngineEcho_callback(message_payload);
        break;
    case (RPC_C2S_Mail_SetEchoDelay | RPC_RESPONSE):
        //client_Mail_SetEchoDelay_callback(message_payload);
        break;
    case (RPC_C2S_RF4CE_ZRC_SetWakeUpActionCodeKey | RPC_RESPONSE):
        client_RF4CE_ZRC_SetWakeUpActionCodeReq_callback(message_payload);
        break;
    case (RPC_C2S_RF4CE_ZRC_GetWakeUpActionCodeKey | RPC_RESPONSE):
        client_RF4CE_ZRC_GetWakeUpActionCodeReq_callback(message_payload);
        break;
    case (RPC_C2S_RF4CE_ZRC1_TargetBindReq | RPC_RESPONSE):
        client_RF4CE_ZRC1_TargetBindReq_callback(message_payload);
        break;
    case (RPC_C2S_RF4CE_ZRC2_SetPushButtonStimulusReq | RPC_RESPONSE):
        client_RF4CE_ZRC2_SetPushButtonStimulusReq_callback(message_payload);
        break;
    case (RPC_C2S_RF4CE_ZRC2_EnableBindingReq | RPC_RESPONSE):
        client_RF4CE_ZRC2_EnableBindingReq_callback(message_payload);
        break;
    case (RPC_C2S_RF4CE_ZRC2_SetAttributesReq | RPC_RESPONSE):
        client_RF4CE_ZRC2_SetAttributesReq_callback(message_payload);
        break;
    case (RPC_C2S_RF4CE_ZRC2_CheckValidationResp | RPC_RESPONSE): // never be called since RF4CE_ZRC2_CheckValidationResp doesn't need callback
        break;
    case (RPC_C2S_HA_EnterNetworkReq | RPC_RESPONSE):
        //client_HA_EnterNetworkReq_callback(message_payload);
        break;

    case (RPC_C2S_RF4CE_RegisterVirtualDevice | RPC_RESPONSE):
        client_RF4CE_RegisterVirtualDevice_callback(message_payload);
        break;

    case (RPC_C2S_RF4CE_UnpairReq | RPC_RESPONSE):
        client_RF4CE_UnpairReq_callback(message_payload);
        break;
    case (RPC_C2S_TE_Host2Uart1Req | RPC_RESPONSE):
        //client_Mail_Host2Uart1_callback(message_payload);
        break;
    case (RPC_C2S_Phy_Test_Get_Caps_Req | RPC_RESPONSE):
        client_Phy_Test_Get_Caps_Req_callback(message_payload);
        break;
    case (RPC_C2S_Phy_Test_Set_Channel_Req | RPC_RESPONSE):
        client_Phy_Test_Set_Channel_Req_callback(message_payload);
        break;
    case (RPC_C2S_Phy_Test_Continuous_Wave_Start_Req | RPC_RESPONSE):
        client_Phy_Test_Continuous_Wave_Start_Req_callback(message_payload);
        break;
    case (RPC_C2S_Phy_Test_Continuous_Wave_Stop_Req | RPC_RESPONSE):
        client_Phy_Test_Continuous_Wave_Stop_Req_callback(message_payload);
        break;
    case (RPC_C2S_Phy_Test_Transmit_Start_Req | RPC_RESPONSE):
        client_Phy_Test_Transmit_Start_Req_callback(message_payload);
        break;
    case (RPC_C2S_Phy_Test_Transmit_Stop_Req | RPC_RESPONSE):
        client_Phy_Test_Transmit_Stop_Req_callback(message_payload);
        break;
    case (RPC_C2S_Phy_Test_Receive_Start_Req | RPC_RESPONSE):
        client_Phy_Test_Receive_Start_Req_callback(message_payload);
        break;
    case (RPC_C2S_Phy_Test_Receive_Stop_Req | RPC_RESPONSE):
        client_Phy_Test_Receive_Stop_Req_callback(message_payload);
        break;
    case (RPC_C2S_Phy_Test_Echo_Start_Req | RPC_RESPONSE):
        client_Phy_Test_Echo_Start_Req_callback(message_payload);
        break;
    case (RPC_C2S_Phy_Test_Echo_Stop_Req | RPC_RESPONSE):
        client_Phy_Test_Echo_Stop_Req_callback(message_payload);
        break;
    case (RPC_C2S_Phy_Test_Energy_Detect_Scan_Req | RPC_RESPONSE):
        client_Phy_Test_Energy_Detect_Scan_Req_callback(message_payload);
        break;
    case (RPC_C2S_Phy_Test_Get_Stats_Req | RPC_RESPONSE):
        client_Phy_Test_Get_Stats_Req_callback(message_payload);
        break;
    case (RPC_C2S_Phy_Test_Reset_Stats_Req | RPC_RESPONSE):
        client_Phy_Test_Reset_Stats_Req_callback(message_payload);
        break;
    case (RPC_C2S_Phy_Test_Set_TX_Power_Req | RPC_RESPONSE):
        client_Phy_Test_Set_TX_Power_Req_callback(message_payload);
        break;
    case (RPC_C2S_Phy_Test_SelectAntenna_Req | RPC_RESPONSE):
        client_Phy_Test_SelectAntenna_Req_callback(message_payload);
        break;
    case (RPC_C2S_RF4CE_Get_Diag_Caps_Req | RPC_RESPONSE):
        client_RF4CE_Get_Diag_Caps_Req_callback(message_payload);
        break;
    case (RPC_C2S_RF4CE_Get_Diag_Req | RPC_RESPONSE):
        client_RF4CE_Get_Diag_Req_callback(message_payload);
        break;
    case (RPC_S2C_SYS_EVENTNTFY):
        client_SYS_EventNtfy(message_payload);
        break;
#ifdef _ZBPRO_
    case (RPC_C2S_ZBPRO_NWK_PermitJoiningReq | RPC_RESPONSE):
        client_ZBPRO_NWK_PermitJoiningReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_NWK_LeaveReq | RPC_RESPONSE):
        client_ZBPRO_NWK_LeaveReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_NWK_GetKeyReq | RPC_RESPONSE):
        client_ZBPRO_NWK_GetKeyReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_NWK_SetKeyReq | RPC_RESPONSE):
        client_ZBPRO_NWK_SetKeyReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_NWK_RouteDiscoveryReq | RPC_RESPONSE):
        client_ZBPRO_NWK_RouteDiscoveryReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_APS_EndpointRegisterReq | RPC_RESPONSE):
        client_ZBPRO_APS_EndpointRegisterReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_APS_EndpointUnregisterReq | RPC_RESPONSE):
        client_ZBPRO_APS_EndpointUnregisterReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_APS_DataReq | RPC_RESPONSE):
        client_ZBPRO_APS_DataReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_APS_BindReq | RPC_RESPONSE):
        client_ZBPRO_APS_BindReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_APS_UnbindReq | RPC_RESPONSE):
        client_ZBPRO_APS_UnbindReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_APS_GetReq | RPC_RESPONSE):
        client_ZBPRO_APS_GetReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_APS_SetReq | RPC_RESPONSE):
        client_ZBPRO_APS_SetReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_APS_GetKeyReq | RPC_RESPONSE):
        client_ZBPRO_APS_GetKeyReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_APS_SetKeyReq | RPC_RESPONSE):
        client_ZBPRO_APS_SetKeyReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_APS_AddGroupReq | RPC_RESPONSE):
        client_ZBPRO_APS_AddGroupReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_APS_RemoveGroupReq | RPC_RESPONSE):
        client_ZBPRO_APS_RemoveGroupReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_APS_RemoveAllGroupsReq | RPC_RESPONSE):
        client_ZBPRO_APS_RemoveAllGroupsReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_APS_TransportKeyReq | RPC_RESPONSE):
        client_ZBPRO_APS_TransportKeyReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_APS_UpdateDeviceReq | RPC_RESPONSE):
        client_ZBPRO_APS_UpdateDeviceReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_APS_RemoveDeviceReq | RPC_RESPONSE):
        client_ZBPRO_APS_RemoveDeviceReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_APS_RequestKeyReq | RPC_RESPONSE):
        client_ZBPRO_APS_RequestKeyReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_APS_SwitchKeyReq | RPC_RESPONSE):
        client_ZBPRO_APS_SwitchKeyReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZDO_AddrResolvingReq | RPC_RESPONSE):
        client_ZBPRO_ZDO_AddrResolvingReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZDO_NodeDescReq | RPC_RESPONSE):
        client_ZBPRO_ZDO_NodeDescReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZDO_PowerDescReq | RPC_RESPONSE):
        client_ZBPRO_ZDO_PowerDescReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZDO_SimpleDescReq | RPC_RESPONSE):
        client_ZBPRO_ZDO_SimpleDescReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZDO_ActiveEpReq | RPC_RESPONSE):
        client_ZBPRO_ZDO_ActiveEpReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZDO_MatchDescReq | RPC_RESPONSE):
        client_ZBPRO_ZDO_MatchDescReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZDO_DeviceAnnceReq | RPC_RESPONSE):
        client_ZBPRO_ZDO_DeviceAnnceReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZDO_EndDeviceBindReq | RPC_RESPONSE):
        client_ZBPRO_ZDO_EndDeviceBindReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZDO_BindReq | RPC_RESPONSE):
        client_ZBPRO_ZDO_BindReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZDO_UnbindReq | RPC_RESPONSE):
        client_ZBPRO_ZDO_UnbindReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZDO_StartNetworkReq | RPC_RESPONSE):
        client_ZBPRO_ZDO_StartNetworkReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZDO_MgmtLeaveReq | RPC_RESPONSE):
        client_ZBPRO_ZDO_MgmtLeaveReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZDO_MgmtPermitJoiningReq | RPC_RESPONSE):
        client_ZBPRO_ZDO_MgmtPermitJoiningReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZDO_MgmtNwkUpdateReq | RPC_RESPONSE):
        client_ZBPRO_ZDO_MgmtNwkUpdateReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZDO_MgmtNwkUpdateUnsolResp | RPC_RESPONSE):
        client_ZBPRO_ZDO_MgmtNwkUpdateUnsolResp_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZDO_MgmtLqiReq | RPC_RESPONSE):
        client_ZBPRO_ZDO_MgmtLqiReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZDO_MgmtBindReq | RPC_RESPONSE):
        client_ZBPRO_ZDO_MgmtBindReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZDO_MgmtNwkUpdateUnsolInd):
        client_ZBPRO_ZDO_MgmtNwkUpdateUnsolInd(message_payload);
        break;
    case (RPC_C2S_ZBPRO_TC_NwkKeyUpdateReq | RPC_RESPONSE):
        client_ZBPRO_TC_NwkKeyUpdateReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SetPowerSourceReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_SetPowerSourceReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ProfileWideCmdReadAttributesReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ProfileWideCmdReadAttributesReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ProfileWideCmdWriteAttributesReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ProfileWideCmdWriteAttributesReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ProfileWideCmdConfigureReportingReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ProfileWideCmdConfigureReportingReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_IdentifyCmdIdentifyReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_IdentifyCmdIdentifyReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_IdentifyCmdIdentifyQueryReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_IdentifyCmdIdentifyQueryReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_GroupsCmdAddGroupReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_GroupsCmdAddGroupReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_GroupsCmdViewGroupReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_GroupsCmdViewGroupReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_GroupsCmdGetGroupMembershipReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_GroupsCmdGetGroupMembershipReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_GroupsCmdRemoveGroupReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_GroupsCmdRemoveGroupReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ScenesCmdAddSceneReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ScenesCmdAddSceneReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ScenesCmdViewSceneReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ScenesCmdViewSceneReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ScenesCmdStoreSceneReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ScenesCmdStoreSceneReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ScenesCmdRecallSceneReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ScenesCmdRecallSceneReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ScenesCmdRemoveSceneReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ScenesCmdRemoveSceneReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ScenesCmdRemoveAllScenesReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ScenesCmdRemoveAllScenesReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ScenesCmdGetSceneMembershipReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ScenesCmdGetSceneMembershipReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_OnOffCmdOffReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_OnOffCmdOffReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_OnOffCmdOnReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_OnOffCmdOnReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_OnOffCmdToggleReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_OnOffCmdToggleReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_LevelControlCmdMoveToLevelReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_LevelControlCmdMoveToLevelReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_LevelControlCmdMoveReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_LevelControlCmdMoveReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_LevelControlCmdStepReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_LevelControlCmdStepReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_LevelControlCmdStopReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_LevelControlCmdStopReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_DoorLockCmdLockReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_DoorLockCmdLockReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_DoorLockCmdUnlockReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_DoorLockCmdUnlockReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdUpOpenReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_WindowCoveringCmdUpOpenReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdDownCloseReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_WindowCoveringCmdDownCloseReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdStopReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_WindowCoveringCmdStopReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdGotoLiftPecentageReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_WindowCoveringCmdGotoLiftPecentageReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdGotoTiltPecentageReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_WindowCoveringCmdGotoTiltPecentageReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceArmRespReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_SapIasAceArmRespReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceBypassRespReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_SapIasAceBypassRespReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneInfoRespReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_SapIasAceGetZoneInfoRespReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceGetPanelStatusRespReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_SapIasAceGetPanelStatusRespReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneStatusRespReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_SapIasAceGetZoneStatusRespReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceZoneStatusChangedReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_SapIasAceZoneStatusChangedReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAcePanelStatusChangedReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_SapIasAcePanelStatusChangedReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_IASWDCmdStartWarningReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_IASWDCmdStartWarningReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_IASWDCmdSquawkgReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_IASWDCmdSquawkgReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_IdentifyInd):
        client_ZBPRO_ZCL_IdentifyInd(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ProfileWideCmdReportAttributesInd):
        client_ZBPRO_ZCL_ProfileWideCmdReportAttributesInd(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_GroupsCmdGetGroupMembershipResponseInd):
        client_ZBPRO_ZCL_GroupsCmdGetGroupMembershipResponseInd(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseInd):
        client_ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseInd(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestInd):
        client_ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestInd(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationInd):
        client_ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationInd(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceArmInd):
        client_ZBPRO_ZCL_SapIasAceArmInd(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceBypassInd):
        client_ZBPRO_ZCL_SapIasAceBypassInd(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceEmergencyInd):
        client_ZBPRO_ZCL_SapIasAceEmergencyInd(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceFireInd):
        client_ZBPRO_ZCL_SapIasAceFireInd(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAcePanicInd):
        client_ZBPRO_ZCL_SapIasAcePanicInd(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneIdMapInd):
        client_ZBPRO_ZCL_SapIasAceGetZoneIdMapInd(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneInfoInd):
        client_ZBPRO_ZCL_SapIasAceGetZoneInfoInd(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceGetPanelStatusInd):
        client_ZBPRO_ZCL_SapIasAceGetPanelStatusInd(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceGetBypassedZoneListInd):
        client_ZBPRO_ZCL_SapIasAceGetBypassedZoneListInd(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneStatusInd):
        client_ZBPRO_ZCL_SapIasAceGetZoneStatusInd(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ColorControlCmdMoveToColorReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ColorControlCmdMoveToColorReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ColorControlCmdMoveColorReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ColorControlCmdMoveColorReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ColorControlCmdStepColorReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ColorControlCmdStepColorReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ColorControlCmdColorLoopSetReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ColorControlCmdColorLoopSetReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ColorControlCmdStopMoveStepReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ColorControlCmdStopMoveStepReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReq | RPC_RESPONSE):
        client_ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZHA_EzModeReq | RPC_RESPONSE):
        client_ZBPRO_ZHA_EzModeReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZHA_CieDeviceEnrollReq | RPC_RESPONSE):
        client_ZBPRO_ZHA_CieDeviceEnrollReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZHA_CieDeviceSetPanelStatusReq | RPC_RESPONSE):
        client_ZBPRO_ZHA_CieDeviceSetPanelStatusReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZHA_CieZoneSetBypassStateReq | RPC_RESPONSE):
        client_ZBPRO_ZHA_CieZoneSetBypassStateReq_callback(message_payload);
        break;
    case (RPC_C2S_ZBPRO_ZHA_CieDeviceSetPanelStatusInd):
        client_ZBPRO_ZHA_CieDeviceSetPanelStatusInd(message_payload);
        break;
#endif
    default:
        printf("ZIGBEE_RPC_CLIENT:  process_rx_message unknown message id:  0x%x\n", message_id);
        break;
    }
}

static void *rpc_client_thread(void *pParam)
{
    unsigned int message_buffer[MAX_MSG_SIZE_IN_WORDS];
    int socket; /* not used for client */
    ((void)pParam); /* not used */

    while (1) {
        if (Zigbee_Rpc_Receive(message_buffer, &socket, Zigbee_Socket_ClientRecv) < 0) {
            g_pZigbeeCallback->ZigbeeError();
            break;
        } else {
            process_rx_message(message_buffer);
        }
    }
    return NULL;
}

/******************************************************************************
    Creates thread for callback function processing
******************************************************************************/
void Zigbee_Open(struct zigbeeCallback *p_zigbeeCallback, char *hostIpAddr)
{
    int rc;
    pthread_attr_t threadAttr;
    struct sched_param schedParam;

    while (1) {
        rc = Zigbee_Socket_ClientOpen(hostIpAddr);
        if (!rc) break;
        printf("waiting for server...\n");
        sleep(1);
    }

    if (g_pZigbeeCallback != NULL) {
        printf("ZIGBEE_RPC_CLIENT:  g_pZigbeeCallback != NULL\n");
    }

    g_pZigbeeCallback = (zigbeeCallback *)malloc(sizeof(zigbeeCallback));
    memcpy(g_pZigbeeCallback, p_zigbeeCallback, sizeof(zigbeeCallback));

    /* Create thread for RF4CE callbacks */
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setschedpolicy(&threadAttr, SCHED_FIFO);
    pthread_attr_getschedparam(&threadAttr, &schedParam);
    schedParam.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_attr_setschedparam(&threadAttr, &schedParam);
    pthread_attr_setstacksize(&threadAttr, 8*1024);
    rc = pthread_create(&zigbeeThread,
                        &threadAttr,
                        rpc_client_thread,
                        NULL);
    if ( rc ) {
        printf("ZIGBEE_RPC_CLIENT:  Unable to create thread");
        while (1);
    }

    /* Send request to Zigbee */
    Zigbee_Rpc_Send(NULL, 0, RPC_C2S_Open_Request, Zigbee_Socket_ClientSend, 0);
}

void Zigbee_Close(void)
{
    pthread_cancel(zigbeeThread);
    free(g_pZigbeeCallback);
    g_pZigbeeCallback = NULL;
    Zigbee_Socket_ClientClose();
}

void Zigbee_GetDefaultSettings(struct zigbeeCallback *zcb)
{
    memset(zcb, 0, sizeof(struct zigbeeCallback));
}

zigbeeCallback *Zigbee_GetCallback(void)
{
    return g_pZigbeeCallback;
}
