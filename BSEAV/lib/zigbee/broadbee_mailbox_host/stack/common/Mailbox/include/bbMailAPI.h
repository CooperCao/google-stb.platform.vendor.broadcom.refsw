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
* FILENAME: $Workfile: branches/ext_xhuajun/MailboxIntegration/stack/common/Mailbox/include/bbMailAPI.h $
*
* DESCRIPTION:
*       declaration of wrappers for public functions
*
* $Revision: 3612 $
* $Date: 2014-09-17 09:29:25Z $
*
*****************************************************************************************/
#ifndef _MAIL_API_H
#define _MAIL_API_H

/************************* INCLUDES *****************************************************/
#include "bbExtTestEngine.h"
#ifdef MAILBOX_UNIT_TEST
# include "bbExtMailboxUnitTest.h"
#endif

#include "bbSysNvmManager.h"
#include "bbSysEvent.h"
#include "bbSysPrint.h"
#include "bbSysDbgMm.h"
#include "bbSysDbgProfilingEngine.h"

#ifdef _ZBPRO_
# include "bbMacSapTypesBanTable.h"
# include "bbMacSapForZBPRO.h"

# include "bbZbProNwkSapTypesPermitJoining.h"
# include "bbZbProNwkSapTypesLeave.h"
# include "bbZbProNwkSapTypesGetSet.h"
# include "bbZbProNwkSapTypesRouteDiscovery.h"

# include "bbZbProApsData.h"
# include "bbZbProApsGroupManager.h"
# include "bbZbProApsSapBindUnbind.h"
# include "bbZbProApsSapGetSet.h"
# include "bbZbProApsSapSecurityServices.h"

# include "bbZbProZdoSapTypesDiscoveryManager.h"
# include "bbZbProZdoSapTypesGetNodeDescHandler.h"
# include "bbZbProZdoSapTypesGetPowerDescHandler.h"
# include "bbZbProZdoSapTypesGetSimpleDescHandler.h"
# include "bbZbProZdoSapTypesActiveEp.h"
# include "bbZbProZdoSapTypesMatchDesc.h"
# include "bbZbProZdoSapTypesDeviceAnnce.h"
# include "bbZbProZdoSapTypesBindingManager.h"
# include "bbZbProZdoSapTypesNetworkManager.h"
# include "bbZbProZdoSapTypesNodeManager.h"
# include "bbZbProZdoSapTypesMgmtNwkUpdate.h"
# include "bbZbProZdoSapTypesMgmtLqiHandler.h"
# include "bbZbProZdoSapTypesMgmtBind.h"

# include "bbZbProTcNwkKeyUpdate.h"

# include "bbZbProZclAttrLocal.h"
# include "bbZbProZclSapManagerClusterBasic.h"
# include "bbZbProZclSapManagerClusterIdentify.h"
# include "bbZbProZhaSapEzMode.h"

# include "bbZbProZclSapProfileWideAttributes.h"
# include "bbZbProZclSapProfileWideReporting.h"
# include "bbZbProZclSapClusterBasic.h"
# include "bbZbProZclSapClusterIdentify.h"
# include "bbZbProZclSapClusterGroups.h"
# include "bbZbProZclSapClusterScenes.h"
# include "bbZbProZclSapClusterOnOff.h"
# include "bbZbProZclSapClusterLevelControl.h"
# include "bbZbProZclSapClusterColorControl.h"
# include "bbZbProZclSapClusterDoorLock.h"
# include "bbZbProZclSapClusterWindowCovering.h"
# include "bbZbProZclSapClusterIasZone.h"
# include "bbZbProZclSapClusterIasAce.h"
# include "bbZbProZclSapClusterIASWD.h"
# include "bbZbProZclSapAttributesDiscover.h"

#ifdef _ZHA_PROFILE_CIE_DEVICE_IMPLEMENTATION_
# include "bbZbProZhaSapCieDevice.h"
#endif /* _ZHA_PROFILE_CIE_DEVICE_IMPLEMENTATION_ */

#endif /* _ZBPRO_ */

#ifdef _RF4CE_
# include "bbMacSapTypesBanTable.h"

# include "bbMacSapForRF4CE.h"
# include "bbExtPowerFilterKey.h"
# include "bbRF4CENWK.h"
# include "bbRF4CEPM.h"
# include "bbRF4CEZRC1.h"
# include "bbRF4CEZRC.h"
# include "bbRF4CEMSO.h"
#endif

#ifdef _PHY_TEST_HOST_INTERFACE_
#include "bbPhySapTest.h"
#include "bbPhyTest.h"
#include "bbRF4CEDiag.h"
#endif

#ifdef _PE_ENERGY_MEASUREMENT_
#include "bbExtPeScanEd.h"
#endif


#ifdef MAILBOX_UNIT_TEST
/* */
//extern void MailUnitTest_f1_Call(MailUnitTest_f1Descr_t *req);
//extern void MailUnitTest_f2_Call(MailUnitTest_f2Descr_t *req);
//extern void MailUnitTest_f3_Call(MailUnitTest_f3Descr_t *req);
//extern void MailUnitTest_f4_Call(MailUnitTest_f4Descr_t *req);
//extern void MailUnitTest_f5_Call(MailUnitTest_f5Descr_t *req);

//extern void Mail_TestEnginePing_Call(TE_PingCommandReqDescr_t *req);
extern void Mail_TestEngineEcho_Call(TE_EchoCommandReqDescr_t *const req);
extern void Mail_TestEngineReset_Call(TE_ResetCommandReqDescr_t *req);
//extern void Mail_SetEchoDelay_Call(TE_SetEchoDelayCommandReqDescr_t *req);
extern void sysEventSubscribeHostHandler_Call(SYS_EventHandlerMailParams_t *req);
//extern void Mail_Host2Uart1_Call(TE_Host2Uart1ReqDescr_t *req);
extern void RF4CE_ZRC_SetWakeUpActionCodeReq_Call(RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t *req);
extern void RF4CE_ZRC_GetWakeUpActionCodeReq_Call(RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t *req);

/* MAC API unit test */
extern void MAC_BanTableSetDefaultAction_Call(MAC_BanTableSetDefaultActionReqDescr_t *reqDescr);
extern void MAC_BanTableAddLink_Call(MAC_BanTableAddLinkReqDescr_t *reqDescr);

#ifdef _ZBPRO_
extern void ZBPRO_MAC_DataReq_Call(MAC_DataReqDescr_t *reqDescr);
extern void ZBPRO_MAC_DataInd_Call(MAC_DataIndParams_t *indParams);
extern void ZBPRO_MAC_PurgeReq_Call(MAC_PurgeReqDescr_t *reqDescr);
extern void ZBPRO_MAC_AssociateReq_Call(MAC_AssociateReqDescr_t *reqDescr);
extern void ZBPRO_MAC_AssociateInd_Call(MAC_AssociateIndParams_t *indParams);
extern void ZBPRO_MAC_AssociateResp_Call(MAC_AssociateRespDescr_t *respDescr);
extern void ZBPRO_MAC_BeaconNotifyInd_Call(MAC_BeaconNotifyIndParams_t *indParams);
extern void ZBPRO_MAC_CommStatusInd_Call(MAC_CommStatusIndParams_t *indParams);
extern void ZBPRO_MAC_GetReq_Call(MAC_GetReqDescr_t *reqDescr);
extern void ZBPRO_MAC_OrphanInd_Call(MAC_OrphanIndParams_t *indParams);
extern void ZBPRO_MAC_OrphanResp_Call(MAC_OrphanRespDescr_t *respDescr);
extern void ZBPRO_MAC_PollInd_Call(MAC_PollIndParams_t *indParams);
extern void ZBPRO_MAC_ResetReq_Call(MAC_ResetReqDescr_t *reqDescr);
extern void ZBPRO_MAC_RxEnableReq_Call(MAC_RxEnableReqDescr_t *reqDescr);
extern void ZBPRO_MAC_ScanReq_Call(MAC_ScanReqDescr_t *reqDescr);
extern void ZBPRO_MAC_SetReq_Call(MAC_SetReqDescr_t *reqDescr);
extern void ZBPRO_MAC_StartReq_Call(MAC_StartReqDescr_t *reqDescr);
#endif

extern void RF4CE_MAC_DataReq_Call(MAC_DataReqDescr_t *reqDescr);
extern void RF4CE_MAC_DataInd_Call(MAC_DataIndParams_t *indParams);
extern void RF4CE_MAC_BeaconNotifyInd_Call(MAC_BeaconNotifyIndParams_t *indParams);
extern void RF4CE_MAC_CommStatusInd_Call(MAC_CommStatusIndParams_t *indParams);
extern void RF4CE_MAC_GetReq_Call(MAC_GetReqDescr_t *reqDescr);
extern void RF4CE_MAC_ResetReq_Call(MAC_ResetReqDescr_t *reqDescr);
extern void RF4CE_MAC_RxEnableReq_Call(MAC_RxEnableReqDescr_t *reqDescr);
extern void RF4CE_MAC_ScanReq_Call(MAC_ScanReqDescr_t *reqDescr);
extern void RF4CE_MAC_SetReq_Call(MAC_SetReqDescr_t *reqDescr);
extern void RF4CE_MAC_StartReq_Call(MAC_StartReqDescr_t *reqDescr);

/* RF4CE NWK API unit test */
extern void RF4CE_NWK_ResetReq_Call(RF4CE_NWK_ResetReqDescr_t *reqDescr);
extern void RF4CE_NWK_StartReq_Call(RF4CE_NWK_StartReqDescr_t *reqDescr);
extern void RF4CE_NWK_DataReq_Call(RF4CE_NWK_DataReqDescr_t *reqDescr);
extern void RF4CE_NWK_GetReq_Call(RF4CE_NWK_GetReqDescr_t *reqDescr);
extern void RF4CE_NWK_SetReq_Call(RF4CE_NWK_SetReqDescr_t *reqDescr);
extern void RF4CE_NWK_DiscoveryReq_Call(RF4CE_NWK_DiscoveryReqDescr_t *reqDescr);
extern void RF4CE_NWK_AutoDiscoveryReq_Call(RF4CE_NWK_AutoDiscoveryReqDescr_t *reqDescr);
extern void RF4CE_NWK_PairReq_Call(RF4CE_NWK_PairReqDescr_t *reqDescr);
extern void RF4CE_NWK_UnpairReq_Call(RF4CE_NWK_UnpairReqDescr_t *reqDescr);
extern void RF4CE_NWK_RxEnableReq_Call(RF4CE_NWK_RXEnableReqDescr_t *reqDescr);
extern void RF4CE_NWK_UpdateKeyReq_Call(RF4CE_NWK_UpdateKeyReqDescr_t *reqDescr);
//extern void RF4CE_NWK_DiscoveryResp_Call(RF4CE_NWK_DiscoveryRespDescr_t *reqDescr);
//extern void RF4CE_NWK_PairResp_Call(RF4CE_NWK_PairRespDescr_t *reqDescr);
//extern void RF4CE_NWK_UnpairResp_Call(RF4CE_NWK_UnpairRespDescr_t *reqDescr);
extern void RF4CE_NWK_DataInd_Call(RF4CE_NWK_DataIndParams_t *indParams);
extern void RF4CE_NWK_DiscoveryInd_Call(RF4CE_NWK_DiscoveryIndParams_t *indParams);
extern void RF4CE_NWK_COMMStatusInd_Call(RF4CE_NWK_COMMStatusIndParams_t *indParams);
extern void RF4CE_NWK_PairInd_Call(RF4CE_NWK_PairIndParams_t *indParams);
extern void RF4CE_NWK_UnpairInd_Call(RF4CE_NWK_UnpairIndParams_t *indParams);
extern void RF4CE_NWK_MacStatsReq_Call(RF4CE_NWK_MacStatsReqDescr_t *request);
#ifdef ENABLE_GU_KEY_SEED_IND
extern void RF4CE_NWK_KeySeedStartInd_Call(RF4CE_NWK_KeySeedStartIndParams_t *indication);
#endif /* ENABLE_GU_KEY_SEED_IND */
extern void SYS_PrintInd_Call(SYS_PrintIndParams_t *indication);

#ifdef _ZBPRO_
/* ZBPRO NWK API unit test */
extern void ZBPRO_NWK_PermitJoiningReq_Call(ZBPRO_NWK_PermitJoiningReqDescr_t *req);
extern void ZBPRO_NWK_LeaveReq_Call(ZBPRO_NWK_LeaveReqDescr_t *req);
// extern void ZBPRO_NWK_GetReq_Call(ZBPRO_NWK_GetReqDescr_t *req);
// extern void ZBPRO_NWK_SetReq_Call(ZBPRO_NWK_SetReqDescr_t *req);
extern void ZBPRO_NWK_GetKeyReq_Call(ZBPRO_NWK_GetKeyReqDescr_t *req);
extern void ZBPRO_NWK_SetKeyReq_Call(ZBPRO_NWK_SetKeyReqDescr_t *req);
extern void ZBPRO_NWK_RouteDiscoveryReq_Call(ZBPRO_NWK_RouteDiscoveryReqDescr_t *req);

/* ZBPRO APS API unit test */
extern void ZBPRO_APS_EndpointRegisterReq_Call(ZBPRO_APS_EndpointRegisterReqDescr_t *req);
extern void ZBPRO_APS_EndpointUnregisterReq_Call(ZBPRO_APS_EndpointUnregisterReqDescr_t *req);
extern void ZBPRO_APS_DataReq_Call(ZBPRO_APS_DataReqDescr_t *req);
extern void ZBPRO_APS_BindReq_Call(ZBPRO_APS_BindUnbindReqDescr_t *reqDescr);
extern void ZBPRO_APS_UnbindReq_Call(ZBPRO_APS_BindUnbindReqDescr_t *reqDescr);
extern void ZBPRO_APS_GetReq_Call(ZBPRO_APS_GetReqDescr_t *req);
extern void ZBPRO_APS_SetReq_Call(ZBPRO_APS_SetReqDescr_t *req);
extern void ZBPRO_APS_GetKeyReq_Call(ZBPRO_APS_GetKeyReqDescr_t *req);
extern void ZBPRO_APS_SetKeyReq_Call(ZBPRO_APS_SetKeyReqDescr_t *req);
extern void ZBPRO_APS_AddGroupReq_Call(ZBPRO_APS_AddGroupReqDescr_t *req);
extern void ZBPRO_APS_RemoveGroupReq_Call(ZBPRO_APS_RemoveGroupReqDescr_t *req);
extern void ZBPRO_APS_RemoveAllGroupsReq_Call(ZBPRO_APS_RemoveAllGroupsReqDescr_t *req);
extern void ZBPRO_APS_TransportKeyReq_Call(ZBPRO_APS_TransportKeyReqDescr_t *req);
extern void ZBPRO_APS_UpdateDeviceReq_Call(ZBPRO_APS_UpdateDeviceReqDescr_t *req);
extern void ZBPRO_APS_RemoveDeviceReq_Call(ZBPRO_APS_RemoveDeviceReqDescr_t *req);
extern void ZBPRO_APS_RequestKeyReq_Call(ZBPRO_APS_RequestKeyReqDescr_t *req);
extern void ZBPRO_APS_SwitchKeyReq_Call(ZBPRO_APS_SwitchKeyReqDescr_t *req);
extern void ZBPRO_APS_DataInd_Call(ZBPRO_APS_DataIndParams_t *indParams);
extern void ZBPRO_APS_TransportKeyInd_Call(ZBPRO_APS_TransportKeyIndParams_t *indParams);
extern void ZBPRO_APS_UpdateDeviceInd_Call(ZBPRO_APS_UpdateDeviceIndParams_t *indParams);
extern void ZBPRO_APS_RemoveDeviceInd_Call(ZBPRO_APS_RemoveDeviceIndParams_t *indParams);
extern void ZBPRO_APS_RequestKeyInd_Call(ZBPRO_APS_RequestKeyIndParams_t *indParams);
extern void ZBPRO_APS_SwitchKeyInd_Call(ZBPRO_APS_SwitchKeyIndParams_t *indParams);

/* ZBPRO ZDO API unit test */
extern void ZBPRO_ZDO_AddrResolvingReq_Call(ZBPRO_ZDO_AddrResolvingReqDescr_t *req);
extern void ZBPRO_ZDO_NodeDescReq_Call(ZBPRO_ZDO_NodeDescReqDescr_t *req);
extern void ZBPRO_ZDO_PowerDescReq_Call(ZBPRO_ZDO_PowerDescReqDescr_t *req);
extern void ZBPRO_ZDO_SimpleDescReq_Call(ZBPRO_ZDO_SimpleDescReqDescr_t *req);
extern void ZBPRO_ZDO_ActiveEpReq_Call(ZBPRO_ZDO_ActiveEpReqDescr_t *req);
extern void ZBPRO_ZDO_MatchDescReq_Call(ZBPRO_ZDO_MatchDescReqDescr_t *req);
extern void ZBPRO_ZDO_DeviceAnnceReq_Call(ZBPRO_ZDO_DeviceAnnceReqDescr_t *req);
extern void ZBPRO_ZDO_ServerDiscoveryReq_Call(ZBPRO_ZDO_ServerDiscoveryReqDescr_t *req);
extern void ZBPRO_ZDO_EndDeviceBindReq_Call(ZBPRO_ZDO_EndDeviceBindReqDescr_t *req);
extern void ZBPRO_ZDO_BindReq_Call(ZBPRO_ZDO_BindUnbindReqDescr_t *req);
extern void ZBPRO_ZDO_UnbindReq_Call(ZBPRO_ZDO_BindUnbindReqDescr_t *req);
extern void ZBPRO_ZDO_StartNetworkReq_Call(ZBPRO_ZDO_StartNetworkReqDescr_t *req);
extern void ZBPRO_ZDO_MgmtPermitJoiningReq_Call(ZBPRO_ZDO_MgmtPermitJoiningReqDescr_t *req);
extern void ZBPRO_ZDO_MgmtNwkUpdateReq_Call(ZBPRO_ZDO_MgmtNwkUpdateReqDescr_t *req);
extern void ZBPRO_ZDO_MgmtNwkUpdateUnsolResp_Call(ZBPRO_ZDO_MgmtNwkUpdateUnsolRespDescr_t *resp);
extern void ZBPRO_ZDO_MgmtNwkUpdateUnsolInd_Call(ZBPRO_ZDO_MgmtNwkUpdateUnsolIndParams_t *indParams);
extern void ZBPRO_ZDO_MgmtLqiReq_Call(ZBPRO_ZDO_MgmtLqiReqDescr_t *req);
extern void ZBPRO_ZDO_MgmtBindReq_Call(ZBPRO_ZDO_MgmtBindReqDescr_t *req);
extern void ZBPRO_ZDO_MgmtLeaveReq_Call(ZBPRO_ZDO_MgmtLeaveReqDescr_t *const  reqDescr);

/* ZBPRO ZDO API unit test */

/* ZBPRO ZCL API unit test */
extern void ZBPRO_ZCL_SetPowerSourceReq_Call(ZBPRO_ZCL_SetPowerSourceReqDescr_t *req);
extern void ZBPRO_ZCL_ProfileWideCmdReadAttributesReq_Call(ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t *req);
extern void ZBPRO_ZCL_ProfileWideCmdWriteAttributesReq_Call(ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t *req);
extern void ZBPRO_ZCL_ProfileWideCmdConfigureReportingReq_Call(ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t *const req);
extern void ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReq_Call(ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqDescr_t *const req);
extern void ZBPRO_ZCL_ProfileWideCmdReportAttributesInd_Call(ZBPRO_ZCL_ProfileWideCmdReportAttributesIndParams_t   *const   ind);
extern void ZBPRO_ZCL_IdentifyCmdIdentifyReq_Call(ZBPRO_ZCL_IdentifyCmdIdentifyReqDescr_t *req);
extern void ZBPRO_ZCL_IdentifyCmdIdentifyQueryReq_Call(ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqDescr_t *req);
extern void ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseIndEB_Call(ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseIndParams_t *ind);
# if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_IDENTIFY_RELAY_)
extern void ZBPRO_ZCL_IdentifyCmdIdentifyInd_Call(ZBPRO_ZCL_IdentifyCmdIdentifyIndParams_t *ind);
extern void ZBPRO_ZCL_IdentifyCmdIdentifyResponseReq_Call(ZBPRO_ZCL_IdentifyCmdIdentifyResponseReqDescr_t *req);
extern void ZBPRO_ZCL_IdentifyCmdIdentifyQueryInd_Call(ZBPRO_ZCL_IdentifyCmdIdentifyQueryIndParams_t *ind);
extern void ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReq_Call(ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReqDescr_t *req);
# else
extern void ZBPRO_ZCL_IdentifyInd(ZBPRO_ZCL_IdentifyIndParams_t *const indParams);
# endif
extern void ZBPRO_ZCL_GroupsCmdAddGroupReq_Call(ZBPRO_ZCL_GroupsCmdAddGroupReqDescr_t *req);
extern void ZBPRO_ZCL_GroupsCmdViewGroupReq_Call(ZBPRO_ZCL_GroupsCmdViewGroupReqDescr_t *req);
extern void ZBPRO_ZCL_GroupsCmdGetGroupMembershipReq_Call(ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqDescr_t *req);
extern void ZBPRO_ZCL_GroupsCmdGetGroupMembershipResponseInd_Call(ZBPRO_ZCL_GroupsCmdGetGroupMembershipIndParams_t *ind);
extern void ZBPRO_ZCL_GroupsCmdRemoveGroupReq_Call(ZBPRO_ZCL_GroupsCmdRemoveGroupReqDescr_t *req);
extern void ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReq_Call(ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqDescr_t *req);
extern void ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReq_Call(ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqDescr_t *req);
extern void ZBPRO_ZCL_ScenesCmdAddSceneReq_Call(ZBPRO_ZCL_ScenesCmdAddSceneReqDescr_t *req);
extern void ZBPRO_ZCL_ScenesCmdViewSceneReq_Call(ZBPRO_ZCL_ScenesCmdViewSceneReqDescr_t *req);
extern void ZBPRO_ZCL_ScenesCmdStoreSceneReq_Call(ZBPRO_ZCL_ScenesCmdStoreSceneReqDescr_t *req);
extern void ZBPRO_ZCL_ScenesCmdRecallSceneReq_Call(ZBPRO_ZCL_ScenesCmdRecallSceneReqDescr_t *req);
extern void ZBPRO_ZCL_ScenesCmdRemoveSceneReq_Call(ZBPRO_ZCL_ScenesCmdRemoveSceneReqDescr_t *req);
extern void ZBPRO_ZCL_ScenesCmdRemoveAllScenesReq_Call(ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqDescr_t *req);
extern void ZBPRO_ZCL_ScenesCmdGetSceneMembershipReq_Call(ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqDescr_t *req);
extern void ZBPRO_ZCL_ScenesCmdAddSceneResponseInd_Call(ZBPRO_ZCL_ScenesCmdAddSceneResponseIndParams_t *ind);
extern void ZBPRO_ZCL_ScenesCmdViewSceneResponseInd_Call(ZBPRO_ZCL_ScenesCmdViewSceneResponseIndParams_t *ind);
extern void ZBPRO_ZCL_ScenesCmdStoreSceneResponseInd_Call(ZBPRO_ZCL_ScenesCmdStoreSceneResponseIndParams_t *ind);
extern void ZBPRO_ZCL_ScenesCmdRemoveSceneResponseInd_Call(ZBPRO_ZCL_ScenesCmdRemoveSceneResponseIndParams_t *ind);
extern void ZBPRO_ZCL_ScenesCmdRemoveAllScenesResponseInd_Call(ZBPRO_ZCL_ScenesCmdRemoveAllScenesResponseIndParams_t *ind);
extern void ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseInd_Call(ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseIndParams_t *ind);
extern void ZBPRO_ZCL_OnOffCmdOffReq_Call(ZBPRO_ZCL_OnOffCmdReqDescr_t *req);
extern void ZBPRO_ZCL_OnOffCmdOnReq_Call(ZBPRO_ZCL_OnOffCmdReqDescr_t *req);
extern void ZBPRO_ZCL_OnOffCmdToggleReq_Call(ZBPRO_ZCL_OnOffCmdReqDescr_t *req);
extern void ZBPRO_ZCL_LevelControlCmdMoveToLevelReq_Call(ZBPRO_ZCL_LevelControlCmdMoveToLevelReqDescr_t *req);
extern void ZBPRO_ZCL_LevelControlCmdMoveReq_Call(ZBPRO_ZCL_LevelControlCmdMoveReqDescr_t *req);
extern void ZBPRO_ZCL_LevelControlCmdStepReq_Call(ZBPRO_ZCL_LevelControlCmdStepReqDescr_t *req);
extern void ZBPRO_ZCL_LevelControlCmdStopReq_Call(ZBPRO_ZCL_LevelControlCmdStopReqDescr_t *req);
extern void ZBPRO_ZCL_DoorLockCmdLockReq_Call(ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t *req);
extern void ZBPRO_ZCL_DoorLockCmdUnlockReq_Call(ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t *req);
extern void ZBPRO_ZCL_WindowCoveringCmdUpOpenReq_Call(ZBPRO_ZCL_WindowCoveringCmdReqDescr_t *req);
extern void ZBPRO_ZCL_WindowCoveringCmdDownCloseReq_Call(ZBPRO_ZCL_WindowCoveringCmdReqDescr_t *req);
extern void ZBPRO_ZCL_WindowCoveringCmdStopReq_Call(ZBPRO_ZCL_WindowCoveringCmdReqDescr_t *req);
extern void ZBPRO_ZCL_WindowCoveringCmdGotoLiftPecentageReq_Call(ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t *req);
extern void ZBPRO_ZCL_WindowCoveringCmdGotoTiltPecentageReq_Call(ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t *req);
extern void ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestInd_Call(ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestIndParams_t   *ind);
extern void ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationInd_Call(ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationIndParams_t   *ind);
extern void ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReq_Call(ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqDescr_t *  req);
extern void ZBPRO_ZCL_SapIasAceArmInd_Call(ZBPRO_ZCL_SapIasAceArmIndParams_t *const indParams);
extern void ZBPRO_ZCL_SapIasAceBypassInd_Call(ZBPRO_ZCL_SapIasAceBypassIndParams_t *const indParams);
extern void ZBPRO_ZCL_SapIasAceEmergencyInd_Call(ZBPRO_ZCL_SapIasAceAlarmIndParams_t *const indParams);
extern void ZBPRO_ZCL_SapIasAceFireInd_Call(ZBPRO_ZCL_SapIasAceAlarmIndParams_t *const indParams);
extern void ZBPRO_ZCL_SapIasAcePanicInd_Call(ZBPRO_ZCL_SapIasAceAlarmIndParams_t *const indParams);
extern void ZBPRO_ZCL_SapIasAceGetZoneIdMapInd_Call(ZBPRO_ZCL_SapIasAceGetZoneIdMapIndParams_t *const indParams);
extern void ZBPRO_ZCL_SapIasAceGetZoneInfoInd_Call(ZBPRO_ZCL_SapIasAceGetZoneInfoIndParams_t *const indParams);
extern void ZBPRO_ZCL_SapIasAceGetPanelStatusInd_Call(ZBPRO_ZCL_SapIasAceGetPanelStatusIndParams_t *const indParams);
extern void ZBPRO_ZCL_SapIasAceGetBypassedZoneListInd_Call(ZBPRO_ZCL_SapIasAceGetBypassedZoneListIndParams_t *const indParams);
extern void ZBPRO_ZCL_SapIasAceGetZoneStatusInd_Call(ZBPRO_ZCL_SapIasAceGetZoneStatusIndParams_t *const indParams);
extern void ZBPRO_ZCL_SapIasAceArmRespReq_Call(ZBPRO_ZCL_SapIasAceArmRespReqDescr_t *const reqDescr);
extern void ZBPRO_ZCL_SapIasAceBypassRespReq_Call(ZBPRO_ZCL_SapIasAceBypassRespReqDescr_t *const reqDescr);
extern void ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReq_Call(ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqDescr_t *const reqDescr);
extern void ZBPRO_ZCL_SapIasAceGetZoneInfoRespReq_Call(ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqDescr_t *const reqDescr);
extern void ZBPRO_ZCL_SapIasAceGetPanelStatusRespReq_Call(ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqDescr_t *const reqDescr);
extern void ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReq_Call(ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqDescr_t *const reqDescr);
extern void ZBPRO_ZCL_SapIasAceGetZoneStatusRespReq_Call(ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqDescr_t *const reqDescr);
extern void ZBPRO_ZCL_SapIasAceZoneStatusChangedReq_Call(ZBPRO_ZCL_SapIasAceZoneStatusChangedReqDescr_t *const reqDescr);
extern void ZBPRO_ZCL_SapIasAcePanelStatusChangedReq_Call(ZBPRO_ZCL_SapIasAcePanelStatusChangedReqDescr_t *const reqDescr);
extern void ZBPRO_ZCL_IASWDCmdStartWarningReq_Call(ZBPRO_ZCL_IASWDCmdStartWarningReqDescr_t *reqDescr);
extern void ZBPRO_ZCL_IASWDCmdSquawkgReq_Call(ZBPRO_ZCL_IASWDCmdSquawkReqDescr_t *reqDescr);
extern void ZBPRO_ZCL_ColorControlCmdMoveToColorReq_Call(ZBPRO_ZCL_ColorControlCmdMoveToColorReqDescr_t *req);
extern void ZBPRO_ZCL_ColorControlCmdMoveColorReq_Call(ZBPRO_ZCL_ColorControlCmdMoveColorReqDescr_t *req);
extern void ZBPRO_ZCL_ColorControlCmdStepColorReq_Call(ZBPRO_ZCL_ColorControlCmdStepColorReqDescr_t *req);
extern void ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReq_Call(ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqDescr_t * req);
extern void ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReq_Call(ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqDescr_t * req);
extern void ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReq_Call(ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqDescr_t * req);
extern void ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReq_Call(ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqDescr_t * req);
extern void ZBPRO_ZCL_ColorControlCmdColorLoopSetReq_Call(ZBPRO_ZCL_ColorControlCmdColorLoopSetReqDescr_t * req);
extern void ZBPRO_ZCL_ColorControlCmdStopMoveStepReq_Call(ZBPRO_ZCL_ColorControlCmdStopMoveStepReqDescr_t * req);
extern void ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReq_Call(ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqDescr_t * req);
extern void ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReq_Call(ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqDescr_t * req);
extern void ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReq_Call(ZBPRO_ZCL_ProfileWideCmdDiscoverAttrDescr_t *  req);
/* ZBPRO ZCL API unit test */

/* ZBPRO TC API unit test */
extern void ZBPRO_TC_NwkKeyUpdateReq_Call(ZBPRO_TC_NwkKeyUpdateReqDescr_t *req);
/* ZBPRO TC API unit test */

/* ZBPRO ZDO ZHA unit test */
#ifdef _ZHA_PROFILE_CIE_DEVICE_IMPLEMENTATION_
extern void ZBPRO_ZHA_EzModeReq_Call(ZBPRO_ZHA_EzModeReqDescr_t *reqDescr);
extern void ZBPRO_ZHA_CieDeviceEnrollReq_Call(ZBPRO_ZHA_CieEnrollReqDescr_t *reqDescr);
extern void ZBPRO_ZHA_CieDeviceSetPanelStatusReq_Call(ZBPRO_ZHA_CieSetPanelStatusReqDescr_t * const reqDescr);
extern void ZBPRO_ZHA_CieDeviceSetPanelStatusReqInd_Call(ZBPRO_ZHA_CieSetPanelStatusIndParams_t   *const   indParams);
extern void ZBPRO_ZHA_CieZoneSetBypassStateReq_Call(ZBPRO_ZHA_CieZoneSetBypassStateReqDescr_t   *const descr);
extern void ZBPRO_ZHA_CieDeviceEnrollInd_Call(ZBPRO_ZHA_CieEnrollIndParams_t *const indParams);
#endif

#endif

/* RF4CE Profile manager API unit test */
extern void RF4CE_UnpairReq_Call(RF4CE_UnpairReqDescr_t *request);
extern void RF4CE_StartReq_Call(RF4CE_StartReqDescr_t *request);
extern void RF4CE_ResetReq_Call(RF4CE_ResetReqDescr_t *request);
extern void RF4CE_SetSupportedDevicesReq_Call(RF4CE_SetSupportedDevicesReqDescr_t *request);
extern void RF4CE_CounterExpiredInd_Call(RF4CE_PairingReferenceIndParams_t *indication);
extern void RF4CE_UnpairInd_Call(RF4CE_PairingReferenceIndParams_t *indication);
extern void RF4CE_PairInd_Call(RF4CE_PairingIndParams_t *indication);

/* RF4CE ZRC 1.1 API unit test */
extern void RF4CE_ZRC1_GetAttributesReq_Call(RF4CE_ZRC1_GetAttributeDescr_t *request);
extern void RF4CE_ZRC1_SetAttributesReq_Call(RF4CE_ZRC1_SetAttributeDescr_t *request);
extern void RF4CE_ZRC1_ControllerBindReq_Call(RF4CE_ZRC1_BindReqDescr_t *request);
extern void RF4CE_ZRC1_TargetBindReq_Call(RF4CE_ZRC1_BindReqDescr_t *request);
extern void RF4CE_ZRC1_CommandDiscoveryReq_Call(RF4CE_ZRC1_CommandDiscoveryReqDescr_t *request);
extern void RF4CE_ZRC1_ControlCommandPressedReq_Call(RF4CE_ZRC1_ControlCommandReqDescr_t *request);
extern void RF4CE_ZRC1_ControlCommandReleasedReq_Call(RF4CE_ZRC1_ControlCommandReqDescr_t *request);
extern void RF4CE_ZRC1_ControlCommandInd_Call(RF4CE_ZRC1_ControlCommandIndParams_t *indication);
extern void RF4CE_ZRC1_VendorSpecificReq_Call(RF4CE_ZRC1_VendorSpecificReqDescr_t *request);
extern void RF4CE_ZRC1_VendorSpecificInd_Call(RF4CE_ZRC1_VendorSpecificIndParams_t *indication);

/* RF4CE GDP 2.0 & ZRC 2.0 API unit test */
extern void RF4CE_ZRC2_GetAttributesReq_Call(RF4CE_ZRC2_GetAttributesReqDescr_t *request);
extern void RF4CE_ZRC2_SetAttributesReq_Call(RF4CE_ZRC2_SetAttributesReqDescr_t *request);
extern void RF4CE_ZRC2_KeyExchangeReq_Call(RF4CE_ZRC2_KeyExchangeReqDescr_t *request);
extern void RF4CE_ZRC2_BindReq_Call(RF4CE_ZRC2_BindReqDescr_t *request);
extern void RF4CE_ZRC2_ProxyBindReq_Call(RF4CE_ZRC2_ProxyBindReqDescr_t *request);
extern void RF4CE_ZRC2_EnableBindingReq_Call(RF4CE_ZRC2_BindingReqDescr_t *request);
extern void RF4CE_ZRC2_DisableBindingReq_Call(RF4CE_ZRC2_BindingReqDescr_t *request);
extern void RF4CE_ZRC2_SetPushButtonStimulusReq_Call(RF4CE_ZRC2_ButtonBindingReqDescr_t *request);
extern void RF4CE_ZRC2_ClearPushButtonStimulusReq_Call(RF4CE_ZRC2_ButtonBindingReqDescr_t *request);
extern void RF4CE_ZRC2_CheckValidationResp_Call(RF4CE_ZRC2_CheckValidationRespDescr_t *response);
extern void RF4CE_ZRC2_StartValidationInd_Call(RF4CE_ZRC2_CheckValidationIndParams_t *indication);
extern void RF4CE_ZRC2_CheckValidationInd_Call(RF4CE_ZRC2_CheckValidationIndParams_t *indication);
extern void RF4CE_ZRC2_ControlCommandInd_Call(RF4CE_ZRC2_ControlCommandIndParams_t *indication);
extern void RF4CE_GDP2_SetPollConstraintsReq_Call(RF4CE_GDP2_SetPollConstraintsReqDescr_t *const reqDescr);
extern void RF4CE_GDP2_PollNegotiationReq_Call(RF4CE_GDP2_PollNegotiationReqDescr_t *const reqDescr);
extern void RF4CE_GDP2_PollClientUserEventReq_Call(RF4CE_GDP2_PollClientUserEventReqDescr_t *const reqDescr);
extern void RF4CE_GDP2_ClientNotificationReq_Call(RF4CE_GDP2_ClientNotificationReqDescr_t *const reqDescr);
extern void RF4CE_GDP2_IdentifyCapAnnounceReq_Call(RF4CE_GDP2_IdentifyCapAnnounceReqDescr_t *const reqDescr);
extern void RF4CE_GDP2_IdentifyReq_Call(RF4CE_GDP2_IdentifyReqDescr_t *const reqDescr);
extern void RF4CE_GDP2_PollNegotiationInd_Call(RF4CE_GDP2_PollNegotiationIndParams_t *const indParams);
extern void RF4CE_GDP2_HeartbeatInd_Call(RF4CE_GDP2_HeartbeatIndParams_t *const indParams);
extern void RF4CE_GDP2_ClientNotificationInd_Call(RF4CE_GDP2_ClientNotificationIndParams_t *const indParams);
extern void RF4CE_GDP2_IdentifyCapAnnounceInd_Call(RF4CE_GDP2_IdentifyCapAnnounceIndParams_t *const indParams);
extern void RF4CE_GDP2_IdentifyInd_Call(RF4CE_GDP2_IdentifyIndParams_t *const indParams);
extern void RF4CE_ZRC2_GetSharedSecretInd_Call(RF4CE_ZRC2_GetSharedSecretIndDescr_t *indDescr);
extern void RF4CE_ZRC2_PairNtfyInd_Call(RF4CE_PairingIndParams_t *indication);
extern void RF4CE_ZRC2_BindingFinishedNtfyInd_Call(RF4CE_ZRC2_BindingFinishedNtfyIndParams_t *indication);
extern void RF4CE_ZRC2_GetAttrRespInd_Call(RF4CE_ZRC2_SetAttributesReqParams_t *const indParams);
extern void RF4CE_ZRC2_PushAttrReqInd_Call(RF4CE_ZRC2_SetAttributesReqParams_t *const indParams);

/* RF4CE MSO */
extern void RF4CE_MSO_GetProfileAttributeReq_Call(RF4CE_MSO_GetProfileAttributeReqDescr_t *request);
extern void RF4CE_MSO_SetProfileAttributeReq_Call(RF4CE_MSO_SetProfileAttributeReqDescr_t *request);
extern void RF4CE_MSO_GetRIBAttributeReq_Call(RF4CE_MSO_GetRIBAttributeReqDescr_t *request);
extern void RF4CE_MSO_SetRIBAttributeReq_Call(RF4CE_MSO_SetRIBAttributeReqDescr_t *request);
extern void RF4CE_MSO_BindReq_Call(RF4CE_MSO_BindReqDescr_t *request);
extern void RF4CE_MSO_UserControlPressedReq_Call(RF4CE_MSO_UserControlReqDescr_t *request);
extern void RF4CE_MSO_UserControlReleasedReq_Call(RF4CE_MSO_UserControlReqDescr_t *request);
extern void RF4CE_MSO_StartValidationInd_Call(RF4CE_PairingReferenceIndParams_t *indication);
extern void RF4CE_MSO_CheckValidationInd_Call(RF4CE_MSO_CheckValidationIndParams_t *indication);
extern void RF4CE_MSO_UserControlInd_Call(RF4CE_MSO_UserControlIndParams_t *indication);
extern void RF4CE_MSO_WatchDogKickOrValidateReq_Call(RF4CE_MSO_WatchDogKickOrValidateReqDescr_t *request);

/* NVM */
extern void NVM_ReadFileInd_Call(NVM_ReadFileIndDescr_t *indDescr);
extern void NVM_OpenFileInd_Call(NVM_OpenFileIndDescr_t *indDescr);
extern void NVM_WriteFileInd_Call(NVM_WriteFileIndDescr_t *indDescr);
extern void NVM_CloseFileInd_Call(NVM_CloseFileIndDescr_t *indDescr);

# if defined(_PHY_TEST_HOST_INTERFACE_)
extern void Phy_Test_Get_Caps_Req_Call(Phy_Test_Get_Caps_ReqDescr_t  *request);
extern void Phy_Test_Set_Channel_Req_Call(Phy_Test_Set_Channel_ReqDescr_t *request);
extern void Phy_Test_Continuous_Wave_Start_Req_Call(Phy_Test_Continuous_Wave_Start_ReqDescr_t *request);
extern void Phy_Test_Continuous_Wave_Stop_Req_Call(Phy_Test_Continuous_Wave_Stop_ReqDescr_t *request);
extern void Phy_Test_Transmit_Start_Req_Call(Phy_Test_Transmit_Start_ReqDescr_t *request);
extern void Phy_Test_Transmit_Stop_Req_Call(Phy_Test_Transmit_Stop_ReqDescr_t *request);
extern void Phy_Test_Receive_Start_Req_Call(Phy_Test_Receive_Start_ReqDescr_t *request);
extern void Phy_Test_Receive_Stop_Req_Call(Phy_Test_Receive_Stop_ReqDescr_t *request);
extern void Phy_Test_Echo_Start_Req_Call(Phy_Test_Echo_Start_ReqDescr_t *request);
extern void Phy_Test_Echo_Stop_Req_Call(Phy_Test_Echo_Stop_ReqDescr_t *request);
extern void Phy_Test_Energy_Detect_Scan_Req_Call(Phy_Test_Energy_Detect_Scan_ReqDescr_t *request);
extern void Phy_Test_Get_Stats_Req_Call(Phy_Test_Get_Stats_ReqDescr_t *request);
extern void Phy_Test_Reset_Stats_Req_Call(Phy_Test_Reset_Stats_ReqDescr_t *request);
extern void Phy_Test_Set_TX_Power_Req_Call(Phy_Test_Set_TX_Power_ReqDescr_t *request);
extern void Phy_Test_SelectAntenna_Req_Call(Phy_Test_Select_Antenna_ReqDescr_t *request);
extern void RF4CE_Get_Diag_Caps_Req_Call(RF4CE_Diag_Caps_ReqDescr_t *request);
extern void RF4CE_Get_Diag_Req_Call(RF4CE_Diag_ReqDescr_t *request);
# endif

#endif /* MAILBOX_UNIT_TEST */
#endif /* _MAIL_API_H */
