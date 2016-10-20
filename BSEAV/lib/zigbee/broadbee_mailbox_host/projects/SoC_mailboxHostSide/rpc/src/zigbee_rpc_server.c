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
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
//#include "zigbee_types.h"
#include "bbMailAPI.h"
#include "zigbee_api.h"
#include "zigbee_common.h"
#include "zigbee_rpc.h"
#include "zigbee_rpc_frame.h"
#include "zigbee_ioctl.h"
#include "zigbee_socket_server.h"
#include "zigbee_rpc_server_priv.h"
#include "zigbee_socket.h"

socket_cb_t socket_cb[MAX_SOCKETS];

unsigned int Zigbee_Socket_Is_Idle(int socket)
{
    return socket_cb[socket].state == SOCKET_IDLE ? 1 : 0;
}

/* The server can receive frames from many clients.  Therefore, we have to collect frames into receive buffers based on socket number */
static void process_rx_message(unsigned int *message_rx, int socket)
{
    int message_id;
    int payload_size;

    if (socket >= MAX_SOCKETS) {
        printf("ZIGBEE_RPC_SERVER:  socket %d greater than supported...\n", socket); while(1);
    }

    message_id = RPC_FRAME_GET_MSG_ID(message_rx[RPC_FRAME_HEADER_OFFSET]);

    payload_size = message_rx[RPC_FRAME_LENGTH_OFFSET];
    payload_size -= RPC_FRAME_HEADER_SIZE_IN_WORDS;

    /* save message id */
    socket_cb[socket].message_id = message_id;

    /* copy incoming message from mailbox to local storage */
    memcpy((char *)&socket_cb[socket].message_rx[0], (char *)&message_rx[RPC_FRAME_PAYLOAD_OFFSET], payload_size*4);
    socket_cb[socket].message_rx_payload_size_in_words = payload_size;

    switch (message_id) {

#ifdef TEST
    case RPC_C2S_ClientRawBufferLoopback:
        server_ClientRawBufferLoopback(socket);
        break;

    case RPC_C2S_ClientCoreLoopbackReq:
        server_ClientCoreLoopbackReq(socket);
        break;

    case RPC_C2S_ClientLoopbackReq:
        server_ClientLoopbackReq(socket);
        break;

    case (RPC_S2C_ServerLoopbackInd | RPC_RESPONSE):
        server_ServerLoopbackInd_callback(&socket_cb[socket].message_rx[0], socket);
        break;

    case RPC_C2S_ServerLoopbackStart:
        server_ServerLoopbackStart(socket);
        break;
#endif

    case RPC_C2S_Open_Request:
        printf("ZIGBEE_RPC_SERVER:  RPC_C2M_Open_Request received for socket %d\n", socket);
        if (socket < MAX_SOCKETS) {
            socket_cb[socket].state = SOCKET_IDLE;
        } else {
            printf("ZIGBEE_RPC_SERVER:  Error:  Open request for socket %d outside of range supported\n", socket);
        }
        break;
   case RPC_C2S_RF4CE_MAC_GetReq:
        server_ZBPRO_MAC_GetReq(&socket_cb[socket].message_rx[0], socket);
        break;
   case RPC_C2S_RF4CE_MAC_SetReq:
        server_RF4CE_MAC_SetReq(&socket_cb[socket].message_rx[0], socket);
        break;
   case RPC_C2S_RF4CE_ResetReq:
        server_RF4CE_ResetReq(&socket_cb[socket].message_rx[0], socket);
        break;
   case RPC_C2S_RF4CE_SetSupportedDevicesReq:
        server_RF4CE_SetSupportedDevicesReq(&socket_cb[socket].message_rx[0], socket);
        break;
   case RPC_C2S_RF4CE_StartReq:
        /* Add some more stuff to adapt ZRC2.0 and ZRC1.1 */
        #if defined(USE_RF4CE_PROFILE_ZRC2)
        rf4ce_ZRC2_Set_Default_Check_Validation_Period(0);
        rf4ce_ZRC2_Set_Default_Check_Validation_Period(1);
        #endif
        server_RF4CE_StartReq(&socket_cb[socket].message_rx[0], socket);
        break;
   case RPC_C2S_RF4CE_ZRC1_VendorSpecificReq:
        server_RF4CE_ZRC1_VendorSpecificReq(&socket_cb[socket].message_rx[0], socket);
        break;
   case RPC_C2S_RF4CE_GetReq:
        server_RF4CE_NWK_GetReq(&socket_cb[socket].message_rx[0], socket);
        break;
   case RPC_C2S_RF4CE_SetReq:
        server_RF4CE_NWK_SetReq(&socket_cb[socket].message_rx[0], socket);
        break;
   case RPC_C2S_RF4CE_ZRC1_GetAttributesReq:
        server_RF4CE_ZRC1_GetAttributesReq(&socket_cb[socket].message_rx[0], socket);
        break;
   case RPC_C2S_RF4CE_ZRC1_SetAttributesReq:
        server_RF4CE_ZRC1_SetAttributesReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case RPC_C2S_Mail_TestEngineEcho:
        server_Mail_TestEngineEcho(&socket_cb[socket].message_rx[0], socket);
        break;
    case RPC_C2S_SYS_EventSubscribe:
        server_sysEventSubscribeHostHandler(&socket_cb[socket].message_rx[0], socket);
        //server_SYS_EventSubscribe(&socket_cb[socket].message_rx[0], socket);
        break;
    case RPC_C2S_Mail_SetEchoDelay:
        //server_Mail_SetEchoDelay(&socket_cb[socket].message_rx[0], socket);
        break;
    case RPC_C2S_RF4CE_ZRC_SetWakeUpActionCodeKey:
        server_RF4CE_ZRC_SetWakeUpActionCodeReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case RPC_C2S_RF4CE_ZRC_GetWakeUpActionCodeKey:
        server_RF4CE_ZRC_GetWakeUpActionCodeReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case RPC_C2S_RF4CE_ZRC1_TargetBindReq:
        server_RF4CE_ZRC1_TargetBindReq(&socket_cb[socket].message_rx[0], socket);
        break;
#if defined(USE_RF4CE_PROFILE_ZRC2)
    case RPC_C2S_RF4CE_ZRC2_SetPushButtonStimulusReq:
        server_RF4CE_ZRC2_SetPushButtonStimulusReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case RPC_C2S_RF4CE_ZRC2_EnableBindingReq:
        server_RF4CE_ZRC2_EnableBindingReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case RPC_C2S_RF4CE_ZRC2_SetAttributesReq:
        server_RF4CE_ZRC2_SetAttributesReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case RPC_C2S_RF4CE_ZRC2_CheckValidationResp:
        server_RF4CE_ZRC2_CheckValidationResp(&socket_cb[socket].message_rx[0], socket);
        break;
#endif
    case RPC_C2S_RF4CE_UnpairReq:
        server_RF4CE_UnpairReq(&socket_cb[socket].message_rx[0], socket);
        break;

#if 0
    case RPC_C2S_HA_EnterNetworkReq:
        server_HA_EnterNetworkReq(&socket_cb[socket].message_rx[0], socket);
        break;
#endif

    case (RPC_S2C_RF4CE_ZRC2_CheckValidationInd | RPC_RESPONSE):
        server_RF4CE_ZRC2_CheckValidationInd_callback(&socket_cb[socket].message_rx[0], socket);
        break;

    case (RPC_C2S_RF4CE_RegisterVirtualDevice):
        server_RF4CE_RegisterVirtualDevice(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_TE_Host2Uart1Req):
        //server_Mail_Host2Uart1(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_Phy_Test_Get_Caps_Req):
        server_Phy_Test_Get_Caps_Req(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_Phy_Test_Set_Channel_Req):
        server_Phy_Test_Set_Channel_Req(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_Phy_Test_Continuous_Wave_Start_Req):
        server_Phy_Test_Continuous_Wave_Start_Req(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_Phy_Test_Continuous_Wave_Stop_Req):
        server_Phy_Test_Continuous_Wave_Stop_Req(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_Phy_Test_Transmit_Start_Req):
        server_Phy_Test_Transmit_Start_Req(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_Phy_Test_Transmit_Stop_Req):
        server_Phy_Test_Transmit_Stop_Req(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_Phy_Test_Receive_Start_Req):
        server_Phy_Test_Receive_Start_Req(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_Phy_Test_Receive_Stop_Req):
        server_Phy_Test_Receive_Stop_Req(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_Phy_Test_Echo_Start_Req):
        server_Phy_Test_Echo_Start_Req(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_Phy_Test_Echo_Stop_Req):
        server_Phy_Test_Echo_Stop_Req(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_Phy_Test_Energy_Detect_Scan_Req):
        server_Phy_Test_Energy_Detect_Scan_Req(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_Phy_Test_Get_Stats_Req):
        server_Phy_Test_Get_Stats_Req(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_Phy_Test_Reset_Stats_Req):
        server_Phy_Test_Reset_Stats_Req(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_Phy_Test_Set_TX_Power_Req):
        server_Phy_Test_Set_TX_Power_Req(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_Phy_Test_SelectAntenna_Req):
        server_Phy_Test_SelectAntenna_Req(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_RF4CE_Get_Diag_Caps_Req):
        server_RF4CE_Get_Diag_Caps_Req(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_RF4CE_Get_Diag_Req):
        server_RF4CE_Get_Diag_Req(&socket_cb[socket].message_rx[0], socket);
        break;
    /* ZBPRO NWK */
    case (RPC_S2C_Mail_TestEngineReset):
        server_Mail_TestEngineReset(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_NWK_PermitJoiningReq):
        server_ZBPRO_NWK_PermitJoiningReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_NWK_LeaveReq):
        server_ZBPRO_NWK_LeaveReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_NWK_GetKeyReq):
        server_ZBPRO_NWK_GetKeyReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_NWK_SetKeyReq):
        server_ZBPRO_NWK_SetKeyReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_NWK_RouteDiscoveryReq):
        server_ZBPRO_NWK_RouteDiscoveryReq(&socket_cb[socket].message_rx[0], socket);
        break;
    /* ZBPRO APS */
    case (RPC_C2S_ZBPRO_APS_EndpointRegisterReq):
        server_ZBPRO_APS_EndpointRegisterReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_APS_EndpointUnregisterReq):
        server_ZBPRO_APS_EndpointUnregisterReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_APS_DataReq):
        server_ZBPRO_APS_DataReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_APS_BindReq):
        server_ZBPRO_APS_BindReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_APS_UnbindReq):
        server_ZBPRO_APS_UnbindReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_APS_GetReq):
        server_ZBPRO_APS_GetReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_APS_SetReq):
        server_ZBPRO_APS_SetReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_APS_GetKeyReq):
        server_ZBPRO_APS_GetKeyReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_APS_SetKeyReq):
        server_ZBPRO_APS_SetKeyReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_APS_AddGroupReq):
        server_ZBPRO_APS_AddGroupReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_APS_RemoveGroupReq):
        server_ZBPRO_APS_RemoveGroupReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_APS_RemoveAllGroupsReq):
        server_ZBPRO_APS_RemoveAllGroupsReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_APS_TransportKeyReq):
        server_ZBPRO_APS_TransportKeyReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_APS_UpdateDeviceReq):
        server_ZBPRO_APS_UpdateDeviceReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_APS_RemoveDeviceReq):
        server_ZBPRO_APS_RemoveDeviceReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_APS_RequestKeyReq):
        server_ZBPRO_APS_RequestKeyReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_APS_SwitchKeyReq):
        server_ZBPRO_APS_SwitchKeyReq(&socket_cb[socket].message_rx[0], socket);
        break;
    /* ZBPRO ZDO */
    case (RPC_C2S_ZBPRO_ZDO_AddrResolvingReq):
        server_ZBPRO_ZDO_AddrResolvingReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZDO_NodeDescReq):
        server_ZBPRO_ZDO_NodeDescReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZDO_PowerDescReq):
        server_ZBPRO_ZDO_PowerDescReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZDO_SimpleDescReq):
        server_ZBPRO_ZDO_SimpleDescReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZDO_ActiveEpReq):
        server_ZBPRO_ZDO_ActiveEpReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZDO_MatchDescReq):
        server_ZBPRO_ZDO_MatchDescReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZDO_DeviceAnnceReq):
        server_ZBPRO_ZDO_DeviceAnnceReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZDO_EndDeviceBindReq):
        server_ZBPRO_ZDO_EndDeviceBindReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZDO_BindReq):
        server_ZBPRO_ZDO_BindReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZDO_UnbindReq):
        server_ZBPRO_ZDO_UnbindReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZDO_StartNetworkReq):
        server_ZBPRO_ZDO_StartNetworkReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZDO_MgmtLeaveReq):
        server_ZBPRO_ZDO_MgmtLeaveReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZDO_MgmtPermitJoiningReq):
        server_ZBPRO_ZDO_MgmtPermitJoiningReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZDO_MgmtNwkUpdateReq):
        server_ZBPRO_ZDO_MgmtNwkUpdateReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZDO_MgmtNwkUpdateUnsolResp):
        server_ZBPRO_ZDO_MgmtNwkUpdateUnsolResp(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZDO_MgmtLqiReq):
        server_ZBPRO_ZDO_MgmtLqiReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZDO_MgmtBindReq):
        server_ZBPRO_ZDO_MgmtBindReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_TC_NwkKeyUpdateReq):
        server_ZBPRO_TC_NwkKeyUpdateReq(&socket_cb[socket].message_rx[0], socket);
        break;
    /* ZBPRO ZCL */
    case (RPC_C2S_ZBPRO_ZCL_SetPowerSourceReq):
        server_ZBPRO_ZCL_SetPowerSourceReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReq):
        server_ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ProfileWideCmdReadAttributesReq):
        server_ZBPRO_ZCL_ProfileWideCmdReadAttributesReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ProfileWideCmdWriteAttributesReq):
        server_ZBPRO_ZCL_ProfileWideCmdWriteAttributesReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ProfileWideCmdConfigureReportingReq):
        server_ZBPRO_ZCL_ProfileWideCmdConfigureReportingReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReq):
        server_ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_IdentifyCmdIdentifyReq):
        server_ZBPRO_ZCL_IdentifyCmdIdentifyReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_IdentifyCmdIdentifyQueryReq):
        server_ZBPRO_ZCL_IdentifyCmdIdentifyQueryReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_GroupsCmdAddGroupReq):
        server_ZBPRO_ZCL_GroupsCmdAddGroupReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_GroupsCmdViewGroupReq):
        server_ZBPRO_ZCL_GroupsCmdViewGroupReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_GroupsCmdGetGroupMembershipReq):
        server_ZBPRO_ZCL_GroupsCmdGetGroupMembershipReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_GroupsCmdRemoveGroupReq):
        server_ZBPRO_ZCL_GroupsCmdRemoveGroupReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReq):
        server_ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReq):
        server_ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ScenesCmdAddSceneReq):
        server_ZBPRO_ZCL_ScenesCmdAddSceneReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ScenesCmdViewSceneReq):
        server_ZBPRO_ZCL_ScenesCmdViewSceneReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ScenesCmdStoreSceneReq):
        server_ZBPRO_ZCL_ScenesCmdStoreSceneReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ScenesCmdRecallSceneReq):
        server_ZBPRO_ZCL_ScenesCmdRecallSceneReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ScenesCmdRemoveSceneReq):
        server_ZBPRO_ZCL_ScenesCmdRemoveSceneReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ScenesCmdRemoveAllScenesReq):
        server_ZBPRO_ZCL_ScenesCmdRemoveAllScenesReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ScenesCmdGetSceneMembershipReq):
        server_ZBPRO_ZCL_ScenesCmdGetSceneMembershipReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_OnOffCmdOffReq):
        server_ZBPRO_ZCL_OnOffCmdOffReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_OnOffCmdOnReq):
        server_ZBPRO_ZCL_OnOffCmdOnReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_OnOffCmdToggleReq):
        server_ZBPRO_ZCL_OnOffCmdToggleReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_LevelControlCmdMoveToLevelReq):
        server_ZBPRO_ZCL_LevelControlCmdMoveToLevelReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_LevelControlCmdMoveReq):
        server_ZBPRO_ZCL_LevelControlCmdMoveReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_LevelControlCmdStepReq):
        server_ZBPRO_ZCL_LevelControlCmdStepReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_LevelControlCmdStopReq):
        server_ZBPRO_ZCL_LevelControlCmdStopReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_DoorLockCmdLockReq):
        server_ZBPRO_ZCL_DoorLockCmdLockReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_DoorLockCmdUnlockReq):
        server_ZBPRO_ZCL_DoorLockCmdUnlockReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdUpOpenReq):
        server_ZBPRO_ZCL_WindowCoveringCmdUpOpenReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdDownCloseReq):
        server_ZBPRO_ZCL_WindowCoveringCmdDownCloseReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdStopReq):
        server_ZBPRO_ZCL_WindowCoveringCmdStopReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdGotoLiftPecentageReq):
        server_ZBPRO_ZCL_WindowCoveringCmdGotoLiftPecentageReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdGotoTiltPecentageReq):
        server_ZBPRO_ZCL_WindowCoveringCmdGotoTiltPecentageReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReq):
        server_ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceArmRespReq):
        server_ZBPRO_ZCL_SapIasAceArmRespReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceBypassRespReq):
        server_ZBPRO_ZCL_SapIasAceBypassRespReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReq):
        server_ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneInfoRespReq):
        server_ZBPRO_ZCL_SapIasAceGetZoneInfoRespReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceGetPanelStatusRespReq):
        server_ZBPRO_ZCL_SapIasAceGetPanelStatusRespReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReq):
        server_ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneStatusRespReq):
        server_ZBPRO_ZCL_SapIasAceGetZoneStatusRespReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAceZoneStatusChangedReq):
        server_ZBPRO_ZCL_SapIasAceZoneStatusChangedReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_SapIasAcePanelStatusChangedReq):
        server_ZBPRO_ZCL_SapIasAcePanelStatusChangedReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_IASWDCmdStartWarningReq):
        server_ZBPRO_ZCL_IASWDCmdStartWarningReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_IASWDCmdSquawkReq):
        server_ZBPRO_ZCL_IASWDCmdSquawkReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ColorControlCmdMoveToColorReq):
        server_ZBPRO_ZCL_ColorControlCmdMoveToColorReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ColorControlCmdMoveColorReq):
        server_ZBPRO_ZCL_ColorControlCmdMoveColorReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ColorControlCmdStepColorReq):
        server_ZBPRO_ZCL_ColorControlCmdStepColorReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReq):
        server_ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReq):
        server_ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReq):
        server_ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReq):
        server_ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ColorControlCmdColorLoopSetReq):
        server_ZBPRO_ZCL_ColorControlCmdColorLoopSetReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ColorControlCmdStopMoveStepReq):
        server_ZBPRO_ZCL_ColorControlCmdStopMoveStepReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReq):
        server_ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReq):
        server_ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZHA_EzModeReq):
        server_ZBPRO_ZHA_EzModeReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZHA_CieDeviceEnrollReq):
        server_ZBPRO_ZHA_CieDeviceEnrollReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZHA_CieDeviceSetPanelStatusReq):
        server_ZBPRO_ZHA_CieDeviceSetPanelStatusReq(&socket_cb[socket].message_rx[0], socket);
        break;
    case (RPC_C2S_ZBPRO_ZHA_CieZoneSetBypassStateReq):
        server_ZBPRO_ZHA_CieZoneSetBypassStateReq(&socket_cb[socket].message_rx[0], socket);
        break;
    default:
        printf("ZIGBEE_RPC_SERVER:  process_rx_message unknown message id:  0x%x\n", message_id);
        break;
    }
}

static void *rpc_server_thread(void *pParam)
{
    unsigned int message_buffer[MAX_MSG_SIZE_IN_WORDS];
    printf("ZIGBEE_RPC_SERVER:  rpc_server_thread, waiting for clients...\n");
    while (1) {
        int socket;
        int rc;
        if ((rc = Zigbee_Rpc_Receive(message_buffer, &socket, Zigbee_Socket_ServerRecv)) == -1) {
            // TODO delete the all the registration information.
            registerRemoveRegistrationInfoForClient(socket);
            socket_cb[socket].state = SOCKET_INACTIVE;
        } else {
            printf("ZIGBEE_RPC_SERVER:  received message from socket %d, message id=0x%x\n", socket, RPC_FRAME_GET_MSG_ID(message_buffer[0]));
            process_rx_message(message_buffer, socket);
        }
    }
}

void Zigbee_Rpc_ServerOpen(void)
{
    int rc;
    pthread_t thread;
    pthread_attr_t threadAttr;
    struct sched_param schedParam;

    memset((char *)&socket_cb, 0, sizeof(socket_cb[MAX_SOCKETS]));

    Zigbee_Socket_ServerOpen();

    /* Create thread for the mba_rx_handler*/
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setschedpolicy(&threadAttr, SCHED_FIFO);
    pthread_attr_getschedparam(&threadAttr, &schedParam);
    schedParam.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_attr_setschedparam(&threadAttr, &schedParam);
    pthread_attr_setstacksize(&threadAttr, 8*1024);
    rc = pthread_create(&thread,
                        &threadAttr,
                        rpc_server_thread,
                        NULL);
    if ( rc ) {
        printf("ZIGBEE_RPC_SERVER:  Unable to create thread");
        while (1);
    }

    return;
}

void Zigbee_Rpc_ServerClose(void)
{
    Zigbee_Socket_ServerClose();
}
