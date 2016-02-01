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
#ifndef _ZIGBEE_API_H_
#define _ZIGBEE_API_H_

#include "zigbee_rf4ce_registration.h"

/******************************************************************************
    Zigbee Request functions runs from the caller thread
******************************************************************************/
typedef uint8_t NoAppropriateType_t;

#if 0 /* use the mailbox definition */

typedef unsigned int SYS_DataPointer_t;

/* 12 */
typedef struct _RF4CE_PairingReferenceIndParams_t
{
    /* The pairing reference. */
    uint8_t pairingRef;
} RF4CE_PairingReferenceIndParams_t;

/* 13 */
typedef struct _RF4CE_StartResetConfParams_t
{
    /* The result status of the operation. */
    uint8_t status;
} RF4CE_StartResetConfParams_t;

typedef struct _RF4CE_StartReqDescr_t
{
    /* Callback that is called upon finish of the Start operation */
    #ifdef SERVER
        void (*callback)(struct _RF4CE_StartReqDescr_t *req, RF4CE_StartResetConfParams_t *conf, int client_id);
    #else
        void (*callback)(struct _RF4CE_StartReqDescr_t *req, RF4CE_StartResetConfParams_t *conf);
    #endif
} RF4CE_StartReqDescr_t;

#if 0
/* 4.2.6 */
void RF4CE_CounterExpiredInd( RF4CE_PairingReferenceIndParams_t *indication) {}
void RF4CE_GDP_PullAttributesInd( RF4CE_GDP_HostAttributesReqDescr_t *request) {}
void RF4CE_GDP_PushAttributesInd( RF4CE_GDP_HostAttributesReqDescr_t *request) {}
void RF4CE_GDP_GetAttributesInd( RF4CE_GDP_HostAttributesReqDescr_t *request) {}
void RF4CE_GDP_SetAttributesInd( RF4CE_GDP_HostAttributesReqDescr_t *request) {}
void RF4CE_ZRC_UnpairInd( RF4CE_PairingReferenceIndParams_t *indication) {}
void RF4CE_ZRC_GetAttributesReq(RF4CE_GDP_AttributeDescr_t *request) {}
void RF4CE_ZRC_SetAttributesReq(RF4CE_GDP_AttributeDescr_t *request) {}
void RF4CE_ZRC_PushAttributesReq(RF4CE_GDP_AttributeDescr_t *request) {}
void RF4CE_ZRC_PullAttributesReq(RF4CE_GDP_AttributeDescr_t *request) {}
void RF4CE_UnpairReq(RF4CE_UnpairReqDescr_t *request) {}
void RF4CE_ZRC_KeyExchangeReq( RF4CE_GDP_KeyExchangeReqDescr_t *request) {}
#endif

#ifdef SERVER
    void RF4CE_StartReq(RF4CE_StartReqDescr_t *request, int client_id);
#else
    void RF4CE_StartReq(RF4CE_StartReqDescr_t *request);
#endif

#if 0
void RF4CE_ResetReq(RF4CE_ResetReqDescr_t *request) {}
#endif

/* 4.2.7 */

/* 1 */
typedef RF4CE_PairingReferenceIndParams_t RF4CE_GDP_CheckValidationIndParams_t;

/* 2*/
typedef struct _RF4CE_GDP_CheckValidationRespConfParams_t
{
    /* The result of the operation. */
    uint8_t status;
} RF4CE_GDP_CheckValidationRespConfParams_t;

/* 3 */
typedef struct _RF4CE_GDP_CheckValidationIndDescr_t
{
    /* The request parameters. */
    RF4CE_GDP_CheckValidationIndParams_t params;
    /* The request callback. */
    void (*callback)( struct _RF4CE_GDP_CheckValidationIndDescr_t *req, RF4CE_GDP_CheckValidationRespConfParams_t *conf);
} RF4CE_GDP_CheckValidationIndDescr_t;

#ifdef SERVER
    void RF4CE_ZRC_PairInd(RF4CE_PairingReferenceIndParams_t *indication, int client_id);
#else
    void RF4CE_ZRC_PairInd(RF4CE_PairingReferenceIndParams_t *indication);
#endif

#if 0
void RF4CE_ZRC_HeartbeatInd( RF4CE_PairingReferenceIndParams_t *indication) {}
#endif

#ifdef SERVER
    void RF4CE_ZRC_CheckValidationInd( RF4CE_GDP_CheckValidationIndDescr_t *indication, int client_id);
#else
    void RF4CE_ZRC_CheckValidationInd( RF4CE_GDP_CheckValidationIndDescr_t *indication);
#endif

#if 0
void RF4CE_ZRC_EnableBindingReq(RF4CE_GDP_BindingReqDescr_t *request) {}
void RF4CE_ZRC_DisableBindingReq(RF4CE_GDP_BindingReqDescr_t *request) {}
void RF4CE_ZRC_ButtonBindingReq( RF4CE_GDP_ButtonBindingReqDescr_t *request) {}
void RF4CE_ZRC_ClientNotificationReq( RF4CE_GDP_ClientNotificationReqDescr_t *request) {}
#endif

/* 4.2.8 */

/* 3 */
typedef struct _RF4CE_ZRC_ControlCommandIndParams_t
{
    /* Pairing reference */
    uint8_t pairingRef;
    /* Supplied payload consisting of one or more
    RF4CE_ZRC_ControlCommand_t structures. */
    SYS_DataPointer_t payload;
} RF4CE_ZRC_ControlCommandIndParams_t;

#ifdef SERVER
    void RF4CE_ZRC_ControlCommandInd( RF4CE_ZRC_ControlCommandIndParams_t *indication, int client_id);
#else
    void RF4CE_ZRC_ControlCommandInd( RF4CE_ZRC_ControlCommandIndParams_t *indication);
#endif

#if 0
/* 5.2.1 */
void RF4CE_MSO_GetProfileAttributeReq( RF4CE_MSO_GetProfileAttributeReqDescr_t *request) {}
void RF4CE_MSO_SetProfileAttributeReq( RF4CE_MSO_SetProfileAttributeReqDescr_t *request) {}

/* 5.2.2 */
void RF4CE_MSO_GetRIBInd(RF4CE_MSO_GetRIBAttributeReqDescr_t *request) {}
void RF4CE_MSO_SetRIBInd(RF4CE_MSO_SetRIBAttributeReqDescr_t *request) {}
void RF4CE_MSO_GetRIBAttributeReq( RF4CE_MSO_GetRIBAttributeReqDescr_t *request) {}
void RF4CE_MSO_SetRIBAttributeReq( RF4CE_MSO_SetRIBAttributeReqDescr_t *request) {}

/* 5.2.3 */
void RF4CE_MSO_CheckValidationInd( RF4CE_MSO_CheckValidationRespDescr_t *indication) {}

/* 5.2.4 */
void RF4CE_MSO_UserControlInd( RF4CE_MSO_UserControlIndParams_t *indication) {}

/* HA */

/* 6.4 */
void HA_RegisterEndpointReq(HA_RegisterEndpointReq_t *req) {}
void HA_UnregisterEndpointReq(HA_UnregisterEndpointReq_t *req) {}
void HA_ChangeSubscriptionForAttributeEventsReq( HA_ChangeSubscriptionForAttributeEventsReq_t *req) {}

/* 6.5 */
void HA_ReadAttributeReq(HA_ReadAttributeReq_t *req) {}
void HA_ReadAttributesStructuredReq(HA_ReadAttributesStructuredReq_t *req) {}
void HA_ReadAttributeRespInd(HA_ReadAttributeResp_t *resp) {}
void HA_ReadAttributeNtfyInd(HA_AttributeAccessNtfy_t *ntfy) {}
void HA_WriteAttributeReq(HA_WriteAttributeReq_t *req) {}
void HA_WriteAttributeUndividedReq(HA_WriteAttributeReq_t *req) {}
void HA_WriteAttributeNoResponseReq(HA_WriteAttributeReq_t *req) {}
void HA_WriteAttributesStructuredReq(HA_WriteAttributesStructuredReq_t *req) {}
void HA_WriteAttributeNtfyInd(HA_AttributeAccessNtfy_t *ind) {}
void HA_WriteAttributeRespInd(HA_WriteAttributeResp_t *resp) {}
void HA_WriteAttributesStructuredRespInd(HA_WriteAttributesStructuredResp_t *resp) {}
void HA_ConfigureReportingReq(HA_ConfigureReportingReq_t *req) {}
void HA_ConfigureReportingRespInd(HA_ConfigureReportingResp_t *resp) {}
void HA_ReadReportingConfigurationReq(HA_ReadReportingConfigurationReq_t *ind) {}
void HA_ReadReportingConfigurationRespInd(HA_ReadReportingConfigurationResp_t *resp) {}
void HA_ReportAttributeInd(HA_ReportAttributeInd_t *ind) {}
void HA_DefaultRespInd(HA_DefaultResp_t *resp) {}
void HA_DiscoverAttributesReq(HA_DiscoverAttributesReq_t *req) {}
void HA_DiscoverAttributesRespInd(HA_DiscoverAttributesResp_t *resp) {}

/* 6.7 */
void HA_OnOffClusterOnReq(HA_OnReq_t *req);
void HA_OnOffClusterOffReq(HA_OffReq_t *req);
void HA_OnOffClusterToggleReq(HA_ToggleReq_t *req);

/* 6.8 */
void HA_ScenesClusterAddSceneReq(HA_AddSceneReq_t *req);
void HA_ScenesClusterViewSceneReq(HA_ViewSceneReq_t *req);
void HA_ScenesClusterRemoveSceneReq(HA_RemoveSceneReq_t *req);
void HA_ScenesClusterRemoveAllScenesReq(HA_RemoveAllScenesReq_t *req);
void HA_ScenesClusterStoreSceneReq(HA_StoreSceneReq_t *req);
void HA_ScenesClusterRecallSceneReq(HA_RecallSceneReq_t *req);
void HA_ScenesClusterGetSceneMembershipReq(HA_GetSceneMembershipReq_t *req);
void HA_ScenesClusterAddSceneResponseInd(HA_AddSceneResponseInd_t *ind);
void HA_ScenesClusterViewSceneResponseInd( HA_ViewSceneResponseInd_t *ind);
void HA_ScenesClusterRemoveSceneResponseInd( HA_RemoveSceneResponseInd_t *ind);
void HA_ScenesClusterRemoveAllScenesResponseInd( HA_RemoveAllScenesResponseInd_t *ind);
void HA_ScenesClusterStoreSceneResponseInd( HA_StoreSceneResponseInd_t *ind);
void HA_ScenesClusterGetSceneMembershipResponseInd( HA_GetSceneMembershipResponseInd_t *ind);

/* 6.9 */
void HA_IdentifyClusterIdentifyReq(HA_IdentifyReq_t *req);
void HA_IdentifyClusterIdentifyQueryReq(HA_IdentifyQueryReq_t *req);
void HA_IdentifyClusterIdentifyQueryResponseInd(HA_IdentifyQueryResponseInd_t *ind);

/* 6.10 */
void HA_IdentifyClusterIdentifyQueryResponseReq(HA_IdentifyQueryResponseReq_t *req);
void HA_IdentifyClusterIdentifyInd(HA_IdentifyInd_t *ind);
void HA_IdentifyClusterIdentifyQueryInd(HA_IndicationAddressingInfo_t *addrInfo);

/* 6.11 */
void HA_GroupsClusterAddGroupReq(HA_AddGroupReq_t *req);
void HA_GroupsClusterViewGroupReq(HA_ViewGroupReq_t *req);
void HA_GroupsClusterGetGroupMembershipReq(HA_GetGroupMembershipReq_t *req);
void HA_GroupsClusterRemoveGroupReq(HA_RemoveGroupReq_t *req);
void HA_GroupsClusterRemoveAllGroupsReq( HA_RequestAddressingInfo_t *addrInfo);
void HA_GroupsClusterStoreGroupReq(HA_AddGroupReq_t *req);
void HA_GroupsClusterAddGroupResponseInd(HA_AddGroupResponseInd_t *ind);
void HA_GroupsClusterViewGroupResponseInd(HA_ViewGroupResponseInd_t *ind);
void HA_GroupsClusterGetGroupMembershipResponseInd(HA_GetGroupMembershipResponseInd_t *ind);
void HA_GroupsClusterRemoveGroupResponseInd(HA_RemoveGroupResponseInd_t *ind);

/* 6.12 */
void HA_DoorLockClusterLockDoorReq(HA_LockDoorReq_t *req);
void HA_DoorLockClusterUnlockDoorReq(HA_UnlockDoorReq_t *req);
void HA_DoorLockClusterLockDoorResponseInd(HA_LockDoorResponseInd_t *ind);
void HA_DoorLockClusterUnlockDoorResponseInd( HA_UnlockDoorResponseInd_t *ind);

/* 6.13 */
void HA_LevelControlClusterMoveToLevelReq(HA_MoveToLevelReq_t *req);
void HA_LevelControlClusterMoveReq(HA_MoveLevelReq_t *req);
void HA_LevelControlClusterStepReq(HA_Step_t *cmd);
void HA_LevelControlClusterStopReq(HA_StopReq_t *req);

/* 6.14 */
void HA_WindowCoveringClusterUpOpenReq(HA_UpOpenReq_t *req);
void HA_WindowCoveringClusterDownCloseReq(HA_DownCloseReq_t *req);
void HA_WindowCoveringClusterStopReq(HA_StopReq_t *req);

/* 6.15 */
void HA_ColorControlClusterMoveToColorReq(HA_MoveToColorReq_t *req);
void HA_ColorControlClusterMoveToColorReq(HA_MoveColorReq_t *req);
void HA_ColorControlClusterStepReq(HA_StepColorReq_t *req);

/* 6.17 */
void HA_IASZoneClusterZoneEnrollResponseReq(HA_ZoneEnrollResponseReq_t *req);
void HA_IASZoneClusterZoneStatusChangeNotificationInd(HA_ZoneStatusChangeNotificationInd_t *ind);
void HA_IASZoneClusterZoneEnrollRequestInd(HA_ZoneEnrollRequestInd_t *ind);

/* 6.18 */
void HA_IASWDClusterStartWarningReq(HA_StartWarningReq_t *req);
void HA_IASWDClusterSquawkReq(HA_SquawkReq_t *req);

/* 6.19 */
void HA_IASACEClusterArmResponseReq(HA_ArmResponseReq_t *req);
void HA_IASACEClusterGetZoneIDMapResponseReq(HA_GetZoneIDMapResponseReq_t *req);
void HA_IASACEClusterGetZoneInformationResponseReq(HA_GetZoneInformationResponseReq_t *req);
void HA_IASACEClusterZoneStatusChangedReq(HA_ZoneStatusChangedReq_t *req);
void HA_IASACEClusterPanelStatusChangedReq(HA_PanelStatusChangedReq_t *req);
void HA_IASACEClusterArmInd(HA_ArmInd_t *ind);
void HA_IASACEClusterBypassInd(HA_BypassInd_t *ind);
void HA_IASACEClusterEmergencyInd(HA_IndicationAddressingInfo_t * addrInfo);
void HA_IASACEClusterFireInd(HA_IndicationAddressingInfo_t *addrInfo);
void HA_IASACEClusterPanicInd(HA_IndicationAddressingInfo_t *addrInfo);
void HA_IASACEClusterGetZoneIdMapInd( HA_IndicationAddressingInfo_t *addrInfo);
void HA_IASACEClusterGetZoneInformationInd( HA_GetZoneInformationInd_t *ind);
#endif

/* 6.20 */

typedef int HA_Status_t;

/* 1 */
typedef struct _HA_EnterNetworkConf_t
{
    /* Logical channel number of the network. */
    uint8_t channel;
    /* Short address assigned to the device. */
    uint16_t shortAddress;
    /* PAN identifier of the entered network. */
    uint16_t PANId;
    /* Extended PAN identifier of the entered network. */
    uint64_t extPANId;
    /* Short address of the parent device. */
    uint16_t parentAddress;
    /* Result status of Enter Network procedure. If contains
    not successful status other fields shall be ignored. */
    HA_Status_t status;
} HA_EnterNetworkConf_t;

/* 2*/
typedef struct _HA_EnterNetworkReq_t
{
    /* Callback function to be called after the Enter Network request handling is done. */
    void (*HA_EnterNetworkConf)(struct _HA_EnterNetworkReq_t *origReq, HA_EnterNetworkConf_t *conf);
} HA_EnterNetworkReq_t;

void HA_EnterNetworkReq(HA_EnterNetworkReq_t *req);
void server_HA_EnterNetworkReq_callback(struct _HA_EnterNetworkReq_t *origReq, HA_EnterNetworkConf_t *conf);

#if 0
void HA_ResetToFactoryFreshReq(HA_ResetToFactoryFreshReq_t *req);
void HA_EZModeNetworkSteeringReq(HA_EZModeNetworkSteeringReq_t *req);
void HA_EZModeFindingAndBindingReq( HA_EZModeFindingAndBindingReq_t *req);
void HA_PermitJoiningReq(HA_PermitJoiningReq_t *req);
void HA_SetReq(HA_SetReq_t *req);
void HA_GetReq(HA_GetReq_t *req);
#endif

#endif  /* delete by xionghj */

/* 7.1 */
typedef enum
{
    /* Completed successfully. */
    BROADBEE_NO_ERROR,
    /* Address is out of range. */
    BROADBEE_ADDRESS_OUT_OF_RANGE,
    /* Access has been denied. */
    BROADBEE_ACCESS_DENIED,
    /* Access has taken too long time. */
    BROADBEE_ACCESS_TIME_OUT,
    /* Data is not available. */
    BROADBEE_DATA_NOT_AVAILABLE
} BroadBee_HostAccessResult_t;

/* 7.3 */

typedef struct _BroadBee_DataInHost_t
{
    /* Filename Index */
    uint8_t FilenameIndex;
    /* 32-bit address offset from the beginning of the file in Host */
    uint32_t address;
    /* Length to read in byte unit */
    uint32_t length;
    /* Pointer to actual data read from or to be written to Host */
    uint8_t *hostData;
} BroadBee_DataInHost_t;

typedef struct _BroadBee_ReadFileFromHostInd_t
{
    BroadBee_DataInHost_t dataFromHost;
    void (*BroadBee_ReadFromHostResp)( struct _BroadBee_ReadFileFromHostInd_t *readFileFromHostIndParam, BroadBee_DataInHost_t *returnData);
} BroadBee_ReadFileFromHostInd_t;

typedef struct _BroadBee_WriteFileToHostInd_t
{
    BroadBee_DataInHost_t dataToHost;
    void (*BroadBee_WriteToHostResp)( struct _BroadBee_WriteFileToHostInd_t *writeFileToHostIndParam, BroadBee_HostAccessResult_t *result);
} BroadBee_WriteFileToHostInd_t;

void BroadBee_ReadFileFromHostInd( BroadBee_ReadFileFromHostInd_t *readFileFromHostParam);
void BroadBee_WriteFileToHostInd( BroadBee_WriteFileToHostInd_t *writeFileToHostParam);

typedef struct zigbeeCallback {
#ifdef _RF4CE_
    void (*RF4CE_PairInd)(RF4CE_PairingIndParams_t *indication);
    void (*RF4CE_ZRC2_CheckValidationInd)(RF4CE_ZRC2_CheckValidationIndParams_t *indication);
    void (*RF4CE_ZRC2_ControlCommandInd)(RF4CE_ZRC2_ControlCommandIndParams_t *indication);
    void (*RF4CE_ZRC1_ControlCommandInd)(RF4CE_ZRC1_ControlCommandIndParams_t *indication);
#endif
    void (*SYS_EventNtfy)(SYS_EventNotifyParams_t *event);
    void (*ZBPRO_ZDO_MgmtNwkUpdateUnsolInd)(ZBPRO_ZDO_MgmtNwkUpdateUnsolIndParams_t *indication);
    void (*ZBPRO_ZCL_IdentifyInd)(ZBPRO_ZCL_IdentifyIndParams_t *indication);
    void (*ZBPRO_ZCL_ProfileWideCmdReportAttributesInd)(ZBPRO_ZCL_ProfileWideCmdReportAttributesIndParams_t *indication);
    void (*ZBPRO_ZCL_GroupsCmdGetGroupMembershipResponseInd)(ZBPRO_ZCL_GroupsCmdGetGroupMembershipIndParams_t *indication);
    void (*ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseInd)(ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseIndParams_t *indication);
    void (*ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestInd)(ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestIndParams_t *indication);
    void (*ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationInd)(ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationIndParams_t *indication);
    void (*ZBPRO_ZCL_SapIasAceArmInd)(ZBPRO_ZCL_SapIasAceArmIndParams_t *indication);
    void (*ZBPRO_ZCL_SapIasAceBypassInd)(ZBPRO_ZCL_SapIasAceBypassIndParams_t *indication);
    void (*ZBPRO_ZCL_SapIasAceEmergencyInd)(ZBPRO_ZCL_SapIasAceAlarmIndParams_t *indication);
    void (*ZBPRO_ZCL_SapIasAceFireInd)(ZBPRO_ZCL_SapIasAceAlarmIndParams_t *indication);
    void (*ZBPRO_ZCL_SapIasAcePanicInd)(ZBPRO_ZCL_SapIasAceAlarmIndParams_t *indication);
    void (*ZBPRO_ZCL_SapIasAceGetZoneIdMapInd)(ZBPRO_ZCL_SapIasAceGetZoneIdMapIndParams_t *indication);
    void (*ZBPRO_ZCL_SapIasAceGetZoneInfoInd)(ZBPRO_ZCL_SapIasAceGetZoneInfoIndParams_t *indication);
    void (*ZBPRO_ZCL_SapIasAceGetPanelStatusInd)(ZBPRO_ZCL_SapIasAceGetPanelStatusIndParams_t *indication);
    void (*ZBPRO_ZCL_SapIasAceGetBypassedZoneListInd)(ZBPRO_ZCL_SapIasAceGetBypassedZoneListIndParams_t *indication);
    void (*ZBPRO_ZCL_SapIasAceGetZoneStatusInd)(ZBPRO_ZCL_SapIasAceGetZoneStatusIndParams_t *indication);
    void (*ZBPRO_ZHA_CieDeviceSetPanelStatusInd)(ZBPRO_ZHA_CieSetPanelStatusIndParams_t *indication);
    void (*ServerLoopbackInd)(unsigned int *tx_buffer, unsigned int *rx_buffer, unsigned int num_of_words, void (*callback)(unsigned int *tx_buffer, unsigned int *rx_buffer, unsigned int num_of_words));
    void (*BufferCompare)(unsigned int *buf, unsigned int num_of_words);
    void (*ZigbeeError)(void);
} zigbeeCallback;


#define DECLARE_SERVER_REQUEST_API_FUNCTION(name, reqType, confType)    \
    extern void server_##name(unsigned int *buf, int socket);           \
    extern void server_##name##_callback(reqType  *request, confType *conf);

#define DECLARE_SERVER_INDICATION_API_FUNCTION(name, indType, respType) \
    extern void server_##name(indType *indication, int socket);         \
    extern void server_##name##_callback(unsigned int *buf, int socket);

#define DECLARE_CLIENT_REQUEST_API_FUNCTION(name, reqType, confType)    \
    extern void name(reqType *request);                                 \
    extern void client_##name##_callback(unsigned int *message_rx);

#define DECLARE_CLIENT_INDICATION_API_FUNCTION(name, indType, respType) \
    extern void client_##name(unsigned int *message_rx);                \
    extern void client_##name##_callback(indType *ind, respType *resp);

#define DECLARE_SERVER_LOCAL_REQUEST_API_FUNCTION(name, reqType, confType)      \
    extern void server_##name##_local_call(reqType *req);                       \
    extern void server_##name##_local_callback(reqType  *request, confType *conf);

#ifdef SERVER

void rf4ce_ZRC2_Set_Default_Check_Validation_Period(uint8_t pairingRef);

DECLARE_SERVER_REQUEST_API_FUNCTION(RF4CE_MAC_GetReq, MAC_GetReqDescr_t, MAC_GetConfParams_t)
DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_MAC_GetReq, MAC_GetReqDescr_t, MAC_GetConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(RF4CE_MAC_SetReq, MAC_SetReqDescr_t, MAC_SetConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(RF4CE_ResetReq, RF4CE_ResetReqDescr_t, RF4CE_StartResetConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(RF4CE_RegisterVirtualDevice, RF4CE_RegisterVirtualDeviceReqDescr_t, RF4CE_RegisterVirtualDeviceConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(RF4CE_StartReq, RF4CE_StartReqDescr_t, RF4CE_StartResetConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(RF4CE_NWK_DataReq, RF4CE_NWK_DataReqDescr_t, RF4CE_NWK_DataConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(RF4CE_NWK_GetReq, RF4CE_NWK_GetReqDescr_t, RF4CE_NWK_GetConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(RF4CE_NWK_SetReq, RF4CE_NWK_SetReqDescr_t, RF4CE_NWK_SetConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(RF4CE_ZRC1_GetAttributesReq, RF4CE_ZRC1_GetAttributeDescr_t, RF4CE_ZRC1_GetAttributeConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(Mail_TestEngineEcho, TE_EchoCommandReqDescr_t, TE_EchoCommandConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(Mail_SetEchoDelay, TE_SetEchoDelayCommandReqDescr_t, TE_SetEchoDelayCommandConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(RF4CE_ZRC_SetWakeUpActionCodeReq, RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t, RF4CE_ZRC_SetWakeUpActionCodeConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(RF4CE_ZRC_GetWakeUpActionCodeReq, RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t, RF4CE_ZRC_GetWakeUpActionCodeConfParams_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(RF4CE_PairInd, RF4CE_PairingIndParams_t, NoAppropriateType_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(RF4CE_ZRC2_CheckValidationInd, RF4CE_ZRC2_CheckValidationIndParams_t, NoAppropriateType_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(RF4CE_ZRC2_ControlCommandInd, RF4CE_ZRC2_ControlCommandIndParams_t, NoAppropriateType_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(RF4CE_ZRC1_ControlCommandInd, RF4CE_ZRC1_ControlCommandIndParams_t, NoAppropriateType_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(RF4CE_ZRC2_SetPushButtonStimulusReq, RF4CE_ZRC2_ButtonBindingReqDescr_t, RF4CE_ZRC2_BindingConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(RF4CE_ZRC2_EnableBindingReq, RF4CE_ZRC2_BindingReqDescr_t, RF4CE_ZRC2_BindingConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(RF4CE_ZRC2_SetAttributesReq, RF4CE_ZRC2_SetAttributesReqDescr_t, RF4CE_ZRC2_SetAttributesConfParams_t)

DECLARE_SERVER_LOCAL_REQUEST_API_FUNCTION(RF4CE_ZRC2_SetAttributesReq, RF4CE_ZRC2_SetAttributesReqDescr_t, RF4CE_ZRC2_SetAttributesConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(RF4CE_ZRC2_CheckValidationResp, RF4CE_ZRC2_CheckValidationRespDescr_t, NoAppropriateType_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(RF4CE_ZRC1_TargetBindReq, RF4CE_ZRC1_BindReqDescr_t, RF4CE_ZRC1_BindConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(RF4CE_UnpairReq, RF4CE_UnpairReqDescr_t, RF4CE_UnpairConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(Mail_Host2Uart1, TE_Host2Uart1ReqDescr_t,   NoAppropriateType_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(DirectTV_Test_Get_Caps_Req,                             DirectTV_Test_Get_Caps_ReqDescr_t,   DirectTV_Test_Get_Caps_ConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(DirectTV_Test_Set_Channel_Req,                       DirectTV_Test_Set_Channel_ReqDescr_t,   DirectTV_Test_Set_Channel_ConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(DirectTV_Test_Continuous_Wave_Start_Req,   DirectTV_Test_Continuous_Wave_Start_ReqDescr_t,   DirectTV_Test_Continuous_Wave_StartStop_ConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(DirectTV_Test_Continuous_Wave_Stop_Req,     DirectTV_Test_Continuous_Wave_Stop_ReqDescr_t,   DirectTV_Test_Continuous_Wave_StartStop_ConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(DirectTV_Test_Transmit_Start_Req,                 DirectTV_Test_Transmit_Start_ReqDescr_t,   DirectTV_Test_Transmit_StartStop_ConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(DirectTV_Test_Transmit_Stop_Req,                   DirectTV_Test_Transmit_Stop_ReqDescr_t,   DirectTV_Test_Transmit_StartStop_ConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(DirectTV_Test_Receive_Start_Req,                   DirectTV_Test_Receive_Start_ReqDescr_t,   DirectTV_Test_Receive_StartStop_ConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(DirectTV_Test_Receive_Stop_Req,                     DirectTV_Test_Receive_Stop_ReqDescr_t,   DirectTV_Test_Receive_StartStop_ConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(DirectTV_Test_Echo_Start_Req,                         DirectTV_Test_Echo_Start_ReqDescr_t,   DirectTV_Test_Echo_StartStop_ConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(DirectTV_Test_Echo_Stop_Req,                           DirectTV_Test_Echo_Stop_ReqDescr_t,   DirectTV_Test_Echo_StartStop_ConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(DirectTV_Test_Energy_Detect_Scan_Req,         DirectTV_Test_Energy_Detect_Scan_ReqDescr_t,   DirectTV_Test_Energy_Detect_Scan_ConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(DirectTV_Test_Get_Stats_Req,                           DirectTV_Test_Get_Stats_ReqDescr_t,   DirectTV_Test_Get_Stats_ConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(DirectTV_Test_Reset_Stats_Req,                       DirectTV_Test_Reset_Stats_ReqDescr_t,   DirectTV_Test_Reset_Stats_ConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(DirectTV_Test_Set_TX_Power_Req,                     DirectTV_Test_Set_TX_Power_ReqDescr_t,   DirectTV_Test_Set_TX_Power_ConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(Mail_TestEngineReset, TE_ResetCommandReqDescr_t, NoAppropriateType_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(SYS_EventNtfy, SYS_EventNotifyParams_t, NoAppropriateType_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_NWK_PermitJoiningReq, ZBPRO_NWK_PermitJoiningReqDescr_t, ZBPRO_NWK_PermitJoiningConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_NWK_LeaveReq, ZBPRO_NWK_LeaveReqDescr_t, ZBPRO_NWK_LeaveConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_NWK_GetKeyReq, ZBPRO_NWK_GetKeyReqDescr_t, ZBPRO_NWK_GetKeyConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_NWK_SetKeyReq, ZBPRO_NWK_SetKeyReqDescr_t, ZBPRO_NWK_SetKeyConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_NWK_RouteDiscoveryReq, ZBPRO_NWK_RouteDiscoveryReqDescr_t, ZBPRO_NWK_RouteDiscoveryConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_EndpointRegisterReq, ZBPRO_APS_EndpointRegisterReqDescr_t, ZBPRO_APS_EndpointRegisterConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_EndpointUnregisterReq, ZBPRO_APS_EndpointUnregisterReqDescr_t, ZBPRO_APS_EndpointRegisterConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_DataReq, ZBPRO_APS_DataReqDescr_t, ZBPRO_APS_DataConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_BindReq, ZBPRO_APS_BindUnbindReqDescr_t, ZBPRO_APS_BindUnbindConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_UnbindReq, ZBPRO_APS_BindUnbindReqDescr_t, ZBPRO_APS_BindUnbindConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_GetReq, ZBPRO_APS_GetReqDescr_t, ZBPRO_APS_GetConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_SetReq, ZBPRO_APS_SetReqDescr_t, ZBPRO_APS_SetConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_GetKeyReq, ZBPRO_APS_GetKeyReqDescr_t, ZBPRO_APS_GetKeyConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_SetKeyReq, ZBPRO_APS_SetKeyReqDescr_t, ZBPRO_APS_SetKeyConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_AddGroupReq, ZBPRO_APS_AddGroupReqDescr_t, ZBPRO_APS_AddGroupConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_RemoveGroupReq, ZBPRO_APS_RemoveGroupReqDescr_t, ZBPRO_APS_RemoveGroupConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_RemoveAllGroupsReq, ZBPRO_APS_RemoveAllGroupsReqDescr_t, ZBPRO_APS_RemoveAllGroupsConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_TransportKeyReq, ZBPRO_APS_TransportKeyReqDescr_t, ZBPRO_APS_SecurityServicesConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_UpdateDeviceReq, ZBPRO_APS_UpdateDeviceReqDescr_t, ZBPRO_APS_SecurityServicesConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_RemoveDeviceReq, ZBPRO_APS_RemoveDeviceReqDescr_t, ZBPRO_APS_SecurityServicesConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_RequestKeyReq, ZBPRO_APS_RequestKeyReqDescr_t, ZBPRO_APS_SecurityServicesConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_SwitchKeyReq, ZBPRO_APS_SwitchKeyReqDescr_t, ZBPRO_APS_SecurityServicesConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_AddrResolvingReq, ZBPRO_ZDO_AddrResolvingReqDescr_t, ZBPRO_ZDO_AddrResolvingConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_NodeDescReq, ZBPRO_ZDO_NodeDescReqDescr_t, ZBPRO_ZDO_NodeDescConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_PowerDescReq, ZBPRO_ZDO_PowerDescReqDescr_t, ZBPRO_ZDO_PowerDescConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_SimpleDescReq, ZBPRO_ZDO_SimpleDescReqDescr_t, ZBPRO_ZDO_SimpleDescConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_ActiveEpReq, ZBPRO_ZDO_ActiveEpReqDescr_t, ZBPRO_ZDO_ActiveEpConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_MatchDescReq, ZBPRO_ZDO_MatchDescReqDescr_t, ZBPRO_ZDO_MatchDescConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_DeviceAnnceReq, ZBPRO_ZDO_DeviceAnnceReqDescr_t, ZBPRO_ZDO_DeviceAnnceConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_EndDeviceBindReq, ZBPRO_ZDO_EndDeviceBindReqDescr_t, ZBPRO_ZDO_BindConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_BindReq, ZBPRO_ZDO_BindUnbindReqDescr_t, ZBPRO_ZDO_BindConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_UnbindReq, ZBPRO_ZDO_BindUnbindReqDescr_t, ZBPRO_ZDO_BindConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_StartNetworkReq, ZBPRO_ZDO_StartNetworkReqDescr_t, ZBPRO_ZDO_StartNetworkConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_MgmtLeaveReq, ZBPRO_ZDO_MgmtLeaveReqDescr_t, ZBPRO_ZDO_MgmtLeaveConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_MgmtPermitJoiningReq, ZBPRO_ZDO_MgmtPermitJoiningReqDescr_t, ZBPRO_ZDO_MgmtPermitJoiningConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_MgmtNwkUpdateReq, ZBPRO_ZDO_MgmtNwkUpdateReqDescr_t, ZBPRO_ZDO_MgmtNwkUpdateConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_MgmtNwkUpdateUnsolResp, ZBPRO_ZDO_MgmtNwkUpdateUnsolRespDescr_t, ZBPRO_ZDO_MgmtNwkUpdateUnsolConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_MgmtLqiReq, ZBPRO_ZDO_MgmtLqiReqDescr_t, ZBPRO_ZDO_MgmtLqiConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_MgmtBindReq, ZBPRO_ZDO_MgmtBindReqDescr_t, ZBPRO_ZDO_MgmtBindConfParams_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZDO_MgmtNwkUpdateUnsolInd, ZBPRO_ZDO_MgmtNwkUpdateUnsolIndParams_t, NoAppropriateType_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_TC_NwkKeyUpdateReq, ZBPRO_TC_NwkKeyUpdateReqDescr_t, ZBPRO_TC_NwkKeyUpdateConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_SetPowerSourceReq, ZBPRO_ZCL_SetPowerSourceReqDescr_t, ZBPRO_ZCL_SetPowerSourceConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReq, ZBPRO_ZCL_ProfileWideCmdDiscoverAttrDescr_t, ZBPRO_ZCL_ProfileWideCmdDiscoverAttrConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ProfileWideCmdReadAttributesReq, ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t, ZBPRO_ZCL_ProfileWideCmdReadAttrConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ProfileWideCmdWriteAttributesReq, ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t, ZBPRO_ZCL_ProfileWideCmdWriteAttrConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ProfileWideCmdConfigureReportingReq, ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t, ZBPRO_ZCL_ProfileWideCmdConfigureReportingConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReq, ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqDescr_t, ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_IdentifyCmdIdentifyReq, ZBPRO_ZCL_IdentifyCmdIdentifyReqDescr_t, ZBPRO_ZCL_IdentifyCmdConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_IdentifyCmdIdentifyQueryReq, ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqDescr_t, ZBPRO_ZCL_IdentifyCmdConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_GroupsCmdAddGroupReq, ZBPRO_ZCL_GroupsCmdAddGroupReqDescr_t, ZBPRO_ZCL_GroupsCmdAddGroupConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_GroupsCmdViewGroupReq, ZBPRO_ZCL_GroupsCmdViewGroupReqDescr_t, ZBPRO_ZCL_GroupsCmdViewGroupConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_GroupsCmdGetGroupMembershipReq, ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqDescr_t, ZBPRO_ZCL_GroupsCmdGetGroupMembershipConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_GroupsCmdRemoveGroupReq, ZBPRO_ZCL_GroupsCmdRemoveGroupReqDescr_t, ZBPRO_ZCL_GroupsCmdRemoveGroupConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReq, ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqDescr_t, ZBPRO_ZCL_GroupsCmdRemoveAllGroupsConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReq, ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqDescr_t, ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ScenesCmdAddSceneReq, ZBPRO_ZCL_ScenesCmdAddSceneReqDescr_t, ZBPRO_ZCL_ScenesCmdAddSceneConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ScenesCmdViewSceneReq, ZBPRO_ZCL_ScenesCmdViewSceneReqDescr_t, ZBPRO_ZCL_ScenesCmdViewSceneConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ScenesCmdStoreSceneReq, ZBPRO_ZCL_ScenesCmdStoreSceneReqDescr_t, ZBPRO_ZCL_ScenesCmdStoreSceneConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ScenesCmdRecallSceneReq, ZBPRO_ZCL_ScenesCmdRecallSceneReqDescr_t, ZBPRO_ZCL_ScenesCmdConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ScenesCmdRemoveSceneReq, ZBPRO_ZCL_ScenesCmdRemoveSceneReqDescr_t, ZBPRO_ZCL_ScenesCmdRemoveSceneConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ScenesCmdRemoveAllScenesReq, ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqDescr_t, ZBPRO_ZCL_ScenesCmdRemoveAllScenesConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ScenesCmdGetSceneMembershipReq, ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqDescr_t, ZBPRO_ZCL_ScenesCmdConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_OnOffCmdOffReq, ZBPRO_ZCL_OnOffCmdReqDescr_t, ZBPRO_ZCL_OnOffCmdConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_OnOffCmdOnReq, ZBPRO_ZCL_OnOffCmdReqDescr_t, ZBPRO_ZCL_OnOffCmdConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_OnOffCmdToggleReq, ZBPRO_ZCL_OnOffCmdReqDescr_t, ZBPRO_ZCL_OnOffCmdConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_LevelControlCmdMoveToLevelReq, ZBPRO_ZCL_LevelControlCmdMoveToLevelReqDescr_t, ZBPRO_ZCL_LevelControlCmdConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_LevelControlCmdMoveReq, ZBPRO_ZCL_LevelControlCmdMoveReqDescr_t, ZBPRO_ZCL_LevelControlCmdConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_LevelControlCmdStepReq, ZBPRO_ZCL_LevelControlCmdStepReqDescr_t, ZBPRO_ZCL_LevelControlCmdConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_LevelControlCmdStopReq, ZBPRO_ZCL_LevelControlCmdStopReqDescr_t, ZBPRO_ZCL_LevelControlCmdConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_DoorLockCmdLockReq, ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t, ZBPRO_ZCL_DoorLockCmdLockUnlockConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_DoorLockCmdUnlockReq, ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t, ZBPRO_ZCL_DoorLockCmdLockUnlockConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_WindowCoveringCmdUpOpenReq, ZBPRO_ZCL_WindowCoveringCmdReqDescr_t, ZBPRO_ZCL_WindowCoveringCmdConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_WindowCoveringCmdDownCloseReq, ZBPRO_ZCL_WindowCoveringCmdReqDescr_t, ZBPRO_ZCL_WindowCoveringCmdConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_WindowCoveringCmdStopReq, ZBPRO_ZCL_WindowCoveringCmdReqDescr_t, ZBPRO_ZCL_WindowCoveringCmdConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_WindowCoveringCmdGotoLiftPecentageReq, ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t, ZBPRO_ZCL_WindowCoveringCmdConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_WindowCoveringCmdGotoTiltPecentageReq, ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t, ZBPRO_ZCL_WindowCoveringCmdConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReq, ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqDescr_t, ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_SapIasAceArmRespReq, ZBPRO_ZCL_SapIasAceArmRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_SapIasAceBypassRespReq, ZBPRO_ZCL_SapIasAceBypassRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReq, ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneInfoRespReq, ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_SapIasAceGetPanelStatusRespReq, ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReq, ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneStatusRespReq, ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_SapIasAceZoneStatusChangedReq, ZBPRO_ZCL_SapIasAceZoneStatusChangedReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_SapIasAcePanelStatusChangedReq, ZBPRO_ZCL_SapIasAcePanelStatusChangedReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_IASWDCmdStartWarningReq, ZBPRO_ZCL_IASWDCmdStartWarningReqDescr_t, ZBPRO_ZCL_IASWDCmdStartWarningConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_IASWDCmdSquawkgReq, ZBPRO_ZCL_IASWDCmdSquawkReqDescr_t, ZBPRO_ZCL_IASWDCmdSquawkConfParams_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_IdentifyInd, ZBPRO_ZCL_IdentifyIndParams_t, NoAppropriateType_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_ProfileWideCmdReportAttributesInd, ZBPRO_ZCL_ProfileWideCmdReportAttributesIndParams_t, NoAppropriateType_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_GroupsCmdGetGroupMembershipResponseInd, ZBPRO_ZCL_GroupsCmdGetGroupMembershipIndParams_t, NoAppropriateType_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseInd, ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseIndParams_t, NoAppropriateType_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestInd, ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestIndParams_t, NoAppropriateType_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationInd, ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationIndParams_t, NoAppropriateType_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_SapIasAceArmInd, ZBPRO_ZCL_SapIasAceArmIndParams_t, NoAppropriateType_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_SapIasAceBypassInd, ZBPRO_ZCL_SapIasAceBypassIndParams_t, NoAppropriateType_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_SapIasAceEmergencyInd, ZBPRO_ZCL_SapIasAceAlarmIndParams_t, NoAppropriateType_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_SapIasAceFireInd, ZBPRO_ZCL_SapIasAceAlarmIndParams_t, NoAppropriateType_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_SapIasAcePanicInd, ZBPRO_ZCL_SapIasAceAlarmIndParams_t, NoAppropriateType_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneIdMapInd, ZBPRO_ZCL_SapIasAceGetZoneIdMapIndParams_t, NoAppropriateType_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneInfoInd, ZBPRO_ZCL_SapIasAceGetZoneInfoIndParams_t, NoAppropriateType_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_SapIasAceGetPanelStatusInd, ZBPRO_ZCL_SapIasAceGetPanelStatusIndParams_t, NoAppropriateType_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_SapIasAceGetBypassedZoneListInd, ZBPRO_ZCL_SapIasAceGetBypassedZoneListIndParams_t, NoAppropriateType_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneStatusInd, ZBPRO_ZCL_SapIasAceGetZoneStatusIndParams_t, NoAppropriateType_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ColorControlCmdMoveToColorReq, ZBPRO_ZCL_ColorControlCmdMoveToColorReqDescr_t, ZBPRO_ZCL_ColorControlCmdMoveToColorConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ColorControlCmdMoveColorReq, ZBPRO_ZCL_ColorControlCmdMoveColorReqDescr_t, ZBPRO_ZCL_ColorControlCmdMoveColorConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ColorControlCmdStepColorReq, ZBPRO_ZCL_ColorControlCmdStepColorReqDescr_t, ZBPRO_ZCL_ColorControlCmdStepColorConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReq, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqDescr_t, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReq, ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqDescr_t, ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReq, ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqDescr_t, ZBPRO_ZCL_ColorControlCmdEnhancedStepHueConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReq, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqDescr_t, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ColorControlCmdColorLoopSetReq, ZBPRO_ZCL_ColorControlCmdColorLoopSetReqDescr_t, ZBPRO_ZCL_ColorControlCmdColorLoopSetConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ColorControlCmdStopMoveStepReq, ZBPRO_ZCL_ColorControlCmdStopMoveStepReqDescr_t, ZBPRO_ZCL_ColorControlCmdStopMoveStepConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReq, ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqDescr_t, ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReq, ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqDescr_t, ZBPRO_ZCL_ColorControlCmdStepColorTemperatureConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZHA_EzModeReq, ZBPRO_ZHA_EzModeReqDescr_t, ZBPRO_ZHA_EzModeConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZHA_CieDeviceEnrollReq, ZBPRO_ZHA_CieEnrollReqDescr_t, ZBPRO_ZHA_CieEnrollConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZHA_CieDeviceSetPanelStatusReq, ZBPRO_ZHA_CieSetPanelStatusReqDescr_t, ZBPRO_ZHA_CieSetPanelStatusConfParams_t)

DECLARE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZHA_CieZoneSetBypassStateReq, ZBPRO_ZHA_CieZoneSetBypassStateReqDescr_t, ZBPRO_ZHA_CieZoneSetBypassStateConfParams_t)

DECLARE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZHA_CieDeviceSetPanelStatusInd, ZBPRO_ZHA_CieSetPanelStatusIndParams_t, NoAppropriateType_t)

#else

DECLARE_CLIENT_REQUEST_API_FUNCTION(RF4CE_MAC_GetReq, MAC_GetReqDescr_t, MAC_GetConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(RF4CE_MAC_SetReq, MAC_SetReqDescr_t, MAC_SetConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(RF4CE_ResetReq, RF4CE_ResetReqDescr_t, RF4CE_StartResetConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(RF4CE_RegisterVirtualDevice, RF4CE_RegisterVirtualDeviceReqDescr_t, RF4CE_RegisterVirtualDeviceConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(RF4CE_StartReq, RF4CE_StartReqDescr_t, RF4CE_StartResetConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(RF4CE_NWK_DataReq, RF4CE_NWK_DataReqDescr_t, RF4CE_NWK_DataConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(RF4CE_NWK_GetReq, RF4CE_NWK_GetReqDescr_t, RF4CE_NWK_GetConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(RF4CE_NWK_SetReq, RF4CE_NWK_SetReqDescr_t, RF4CE_NWK_SetConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(RF4CE_ZRC1_GetAttributesReq, RF4CE_ZRC1_GetAttributeDescr_t, RF4CE_ZRC1_GetAttributeConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(Mail_TestEngineEcho, TE_EchoCommandReqDescr_t, TE_EchoCommandConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(Mail_SetEchoDelay, TE_SetEchoDelayCommandReqDescr_t, TE_SetEchoDelayCommandConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(RF4CE_ZRC_SetWakeUpActionCodeReq, RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t, RF4CE_ZRC_SetWakeUpActionCodeConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(RF4CE_ZRC_GetWakeUpActionCodeReq, RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t, RF4CE_ZRC_GetWakeUpActionCodeConfParams_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(RF4CE_PairInd, RF4CE_PairingIndParams_t, NoAppropriateType_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(RF4CE_ZRC2_CheckValidationInd, RF4CE_ZRC2_CheckValidationIndParams_t, NoAppropriateType_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(RF4CE_ZRC2_ControlCommandInd, RF4CE_ZRC2_ControlCommandIndParams_t, NoAppropriateType_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(RF4CE_ZRC1_ControlCommandInd, RF4CE_ZRC1_ControlCommandIndParams_t, NoAppropriateType_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(RF4CE_ZRC2_SetPushButtonStimulusReq, RF4CE_ZRC2_ButtonBindingReqDescr_t, RF4CE_ZRC2_BindingConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(RF4CE_ZRC2_EnableBindingReq, RF4CE_ZRC2_BindingReqDescr_t, RF4CE_ZRC2_BindingConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(RF4CE_ZRC2_SetAttributesReq, RF4CE_ZRC2_SetAttributesReqDescr_t, RF4CE_ZRC2_SetAttributesConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(RF4CE_ZRC1_TargetBindReq, RF4CE_ZRC1_BindReqDescr_t, RF4CE_ZRC1_BindConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(RF4CE_UnpairReq, RF4CE_UnpairReqDescr_t, RF4CE_UnpairConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(Mail_Host2Uart1, TE_Host2Uart1ReqDescr_t,   NoAppropriateType_t)
DECLARE_CLIENT_REQUEST_API_FUNCTION(DirectTV_Test_Get_Caps_Req,                             DirectTV_Test_Get_Caps_ReqDescr_t,   DirectTV_Test_Get_Caps_ConfParams_t)
DECLARE_CLIENT_REQUEST_API_FUNCTION(DirectTV_Test_Set_Channel_Req,                       DirectTV_Test_Set_Channel_ReqDescr_t,   DirectTV_Test_Set_Channel_ConfParams_t)
DECLARE_CLIENT_REQUEST_API_FUNCTION(DirectTV_Test_Continuous_Wave_Start_Req,   DirectTV_Test_Continuous_Wave_Start_ReqDescr_t,   DirectTV_Test_Continuous_Wave_StartStop_ConfParams_t)
DECLARE_CLIENT_REQUEST_API_FUNCTION(DirectTV_Test_Continuous_Wave_Stop_Req,     DirectTV_Test_Continuous_Wave_Stop_ReqDescr_t,   DirectTV_Test_Continuous_Wave_StartStop_ConfParams_t)
DECLARE_CLIENT_REQUEST_API_FUNCTION(DirectTV_Test_Transmit_Start_Req,                 DirectTV_Test_Transmit_Start_ReqDescr_t,   DirectTV_Test_Transmit_StartStop_ConfParams_t)
DECLARE_CLIENT_REQUEST_API_FUNCTION(DirectTV_Test_Transmit_Stop_Req,                   DirectTV_Test_Transmit_Stop_ReqDescr_t,   DirectTV_Test_Transmit_StartStop_ConfParams_t)
DECLARE_CLIENT_REQUEST_API_FUNCTION(DirectTV_Test_Receive_Start_Req,                   DirectTV_Test_Receive_Start_ReqDescr_t,   DirectTV_Test_Receive_StartStop_ConfParams_t)
DECLARE_CLIENT_REQUEST_API_FUNCTION(DirectTV_Test_Receive_Stop_Req,                     DirectTV_Test_Receive_Stop_ReqDescr_t,   DirectTV_Test_Receive_StartStop_ConfParams_t)
DECLARE_CLIENT_REQUEST_API_FUNCTION(DirectTV_Test_Echo_Start_Req,                         DirectTV_Test_Echo_Start_ReqDescr_t,   DirectTV_Test_Echo_StartStop_ConfParams_t)
DECLARE_CLIENT_REQUEST_API_FUNCTION(DirectTV_Test_Echo_Stop_Req,                           DirectTV_Test_Echo_Stop_ReqDescr_t,   DirectTV_Test_Echo_StartStop_ConfParams_t)
DECLARE_CLIENT_REQUEST_API_FUNCTION(DirectTV_Test_Energy_Detect_Scan_Req,         DirectTV_Test_Energy_Detect_Scan_ReqDescr_t,   DirectTV_Test_Energy_Detect_Scan_ConfParams_t)
DECLARE_CLIENT_REQUEST_API_FUNCTION(DirectTV_Test_Get_Stats_Req,                           DirectTV_Test_Get_Stats_ReqDescr_t,   DirectTV_Test_Get_Stats_ConfParams_t)
DECLARE_CLIENT_REQUEST_API_FUNCTION(DirectTV_Test_Reset_Stats_Req,                       DirectTV_Test_Reset_Stats_ReqDescr_t,   DirectTV_Test_Reset_Stats_ConfParams_t)
DECLARE_CLIENT_REQUEST_API_FUNCTION(DirectTV_Test_Set_TX_Power_Req,                     DirectTV_Test_Set_TX_Power_ReqDescr_t,   DirectTV_Test_Set_TX_Power_ConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(Mail_TestEngineReset, TE_ResetCommandReqDescr_t, NoAppropriateType_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(SYS_EventNtfy, SYS_EventNotifyParams_t, NoAppropriateType_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_NWK_PermitJoiningReq, ZBPRO_NWK_PermitJoiningReqDescr_t, ZBPRO_NWK_PermitJoiningConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_NWK_LeaveReq, ZBPRO_NWK_LeaveReqDescr_t, ZBPRO_NWK_LeaveConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_NWK_GetKeyReq, ZBPRO_NWK_GetKeyReqDescr_t, ZBPRO_NWK_GetKeyConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_NWK_SetKeyReq, ZBPRO_NWK_SetKeyReqDescr_t, ZBPRO_NWK_SetKeyConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_NWK_RouteDiscoveryReq, ZBPRO_NWK_RouteDiscoveryReqDescr_t, ZBPRO_NWK_RouteDiscoveryConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_EndpointRegisterReq, ZBPRO_APS_EndpointRegisterReqDescr_t, ZBPRO_APS_EndpointRegisterConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_EndpointUnregisterReq, ZBPRO_APS_EndpointUnregisterReqDescr_t, ZBPRO_APS_EndpointRegisterConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_DataReq, ZBPRO_APS_DataReqDescr_t, ZBPRO_APS_DataConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_BindReq, ZBPRO_APS_BindUnbindReqDescr_t, ZBPRO_APS_BindUnbindConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_UnbindReq, ZBPRO_APS_BindUnbindReqDescr_t, ZBPRO_APS_BindUnbindConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_GetReq, ZBPRO_APS_GetReqDescr_t, ZBPRO_APS_GetConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_SetReq, ZBPRO_APS_SetReqDescr_t, ZBPRO_APS_SetConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_GetKeyReq, ZBPRO_APS_GetKeyReqDescr_t, ZBPRO_APS_GetKeyConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_SetKeyReq, ZBPRO_APS_SetKeyReqDescr_t, ZBPRO_APS_SetKeyConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_AddGroupReq, ZBPRO_APS_AddGroupReqDescr_t, ZBPRO_APS_AddGroupConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_RemoveGroupReq, ZBPRO_APS_RemoveGroupReqDescr_t, ZBPRO_APS_RemoveGroupConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_RemoveAllGroupsReq, ZBPRO_APS_RemoveAllGroupsReqDescr_t, ZBPRO_APS_RemoveAllGroupsConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_TransportKeyReq, ZBPRO_APS_TransportKeyReqDescr_t, ZBPRO_APS_SecurityServicesConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_UpdateDeviceReq, ZBPRO_APS_UpdateDeviceReqDescr_t, ZBPRO_APS_SecurityServicesConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_RemoveDeviceReq, ZBPRO_APS_RemoveDeviceReqDescr_t, ZBPRO_APS_SecurityServicesConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_RequestKeyReq, ZBPRO_APS_RequestKeyReqDescr_t, ZBPRO_APS_SecurityServicesConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_SwitchKeyReq, ZBPRO_APS_SwitchKeyReqDescr_t, ZBPRO_APS_SecurityServicesConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_AddrResolvingReq, ZBPRO_ZDO_AddrResolvingReqDescr_t, ZBPRO_ZDO_AddrResolvingConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_NodeDescReq, ZBPRO_ZDO_NodeDescReqDescr_t, ZBPRO_ZDO_NodeDescConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_PowerDescReq, ZBPRO_ZDO_PowerDescReqDescr_t, ZBPRO_ZDO_PowerDescConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_SimpleDescReq, ZBPRO_ZDO_SimpleDescReqDescr_t, ZBPRO_ZDO_SimpleDescConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_ActiveEpReq, ZBPRO_ZDO_ActiveEpReqDescr_t, ZBPRO_ZDO_ActiveEpConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_MatchDescReq, ZBPRO_ZDO_MatchDescReqDescr_t, ZBPRO_ZDO_MatchDescConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_DeviceAnnceReq, ZBPRO_ZDO_DeviceAnnceReqDescr_t, ZBPRO_ZDO_DeviceAnnceConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_EndDeviceBindReq, ZBPRO_ZDO_EndDeviceBindReqDescr_t, ZBPRO_ZDO_BindConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_BindReq, ZBPRO_ZDO_BindUnbindReqDescr_t, ZBPRO_ZDO_BindConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_UnbindReq, ZBPRO_ZDO_BindUnbindReqDescr_t, ZBPRO_ZDO_BindConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_StartNetworkReq, ZBPRO_ZDO_StartNetworkReqDescr_t, ZBPRO_ZDO_StartNetworkConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_MgmtLeaveReq, ZBPRO_ZDO_MgmtLeaveReqDescr_t, ZBPRO_ZDO_MgmtLeaveConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_MgmtPermitJoiningReq, ZBPRO_ZDO_MgmtPermitJoiningReqDescr_t, ZBPRO_ZDO_MgmtPermitJoiningConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_MgmtNwkUpdateReq, ZBPRO_ZDO_MgmtNwkUpdateReqDescr_t, ZBPRO_ZDO_MgmtNwkUpdateConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_MgmtNwkUpdateUnsolResp, ZBPRO_ZDO_MgmtNwkUpdateUnsolRespDescr_t, ZBPRO_ZDO_MgmtNwkUpdateUnsolConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_MgmtLqiReq, ZBPRO_ZDO_MgmtLqiReqDescr_t, ZBPRO_ZDO_MgmtLqiConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_MgmtBindReq, ZBPRO_ZDO_MgmtBindReqDescr_t, ZBPRO_ZDO_MgmtBindConfParams_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZDO_MgmtNwkUpdateUnsolInd, ZBPRO_ZDO_MgmtNwkUpdateUnsolIndParams_t, NoAppropriateType_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_TC_NwkKeyUpdateReq, ZBPRO_TC_NwkKeyUpdateReqDescr_t, ZBPRO_TC_NwkKeyUpdateConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_SetPowerSourceReq, ZBPRO_ZCL_SetPowerSourceReqDescr_t, ZBPRO_ZCL_SetPowerSourceConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReq, ZBPRO_ZCL_ProfileWideCmdDiscoverAttrDescr_t, ZBPRO_ZCL_ProfileWideCmdDiscoverAttrConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ProfileWideCmdReadAttributesReq, ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t, ZBPRO_ZCL_ProfileWideCmdReadAttrConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ProfileWideCmdWriteAttributesReq, ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t, ZBPRO_ZCL_ProfileWideCmdWriteAttrConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ProfileWideCmdConfigureReportingReq, ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t, ZBPRO_ZCL_ProfileWideCmdConfigureReportingConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReq, ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqDescr_t, ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_IdentifyCmdIdentifyReq, ZBPRO_ZCL_IdentifyCmdIdentifyReqDescr_t, ZBPRO_ZCL_IdentifyCmdConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_IdentifyCmdIdentifyQueryReq, ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqDescr_t, ZBPRO_ZCL_IdentifyCmdConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_GroupsCmdAddGroupReq, ZBPRO_ZCL_GroupsCmdAddGroupReqDescr_t, ZBPRO_ZCL_GroupsCmdAddGroupConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_GroupsCmdViewGroupReq, ZBPRO_ZCL_GroupsCmdViewGroupReqDescr_t, ZBPRO_ZCL_GroupsCmdViewGroupConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_GroupsCmdGetGroupMembershipReq, ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqDescr_t, ZBPRO_ZCL_GroupsCmdGetGroupMembershipConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_GroupsCmdRemoveGroupReq, ZBPRO_ZCL_GroupsCmdRemoveGroupReqDescr_t, ZBPRO_ZCL_GroupsCmdRemoveGroupConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReq, ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqDescr_t, ZBPRO_ZCL_GroupsCmdRemoveAllGroupsConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReq, ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqDescr_t, ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ScenesCmdAddSceneReq, ZBPRO_ZCL_ScenesCmdAddSceneReqDescr_t, ZBPRO_ZCL_ScenesCmdAddSceneConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ScenesCmdViewSceneReq, ZBPRO_ZCL_ScenesCmdViewSceneReqDescr_t, ZBPRO_ZCL_ScenesCmdViewSceneConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ScenesCmdStoreSceneReq, ZBPRO_ZCL_ScenesCmdStoreSceneReqDescr_t, ZBPRO_ZCL_ScenesCmdStoreSceneConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ScenesCmdRecallSceneReq, ZBPRO_ZCL_ScenesCmdRecallSceneReqDescr_t, ZBPRO_ZCL_ScenesCmdConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ScenesCmdRemoveSceneReq, ZBPRO_ZCL_ScenesCmdRemoveSceneReqDescr_t, ZBPRO_ZCL_ScenesCmdRemoveSceneConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ScenesCmdRemoveAllScenesReq, ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqDescr_t, ZBPRO_ZCL_ScenesCmdRemoveAllScenesConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ScenesCmdGetSceneMembershipReq, ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqDescr_t, ZBPRO_ZCL_ScenesCmdConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_OnOffCmdOffReq, ZBPRO_ZCL_OnOffCmdReqDescr_t, ZBPRO_ZCL_OnOffCmdConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_OnOffCmdOnReq, ZBPRO_ZCL_OnOffCmdReqDescr_t, ZBPRO_ZCL_OnOffCmdConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_OnOffCmdToggleReq, ZBPRO_ZCL_OnOffCmdReqDescr_t, ZBPRO_ZCL_OnOffCmdConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_LevelControlCmdMoveToLevelReq, ZBPRO_ZCL_LevelControlCmdMoveToLevelReqDescr_t, ZBPRO_ZCL_LevelControlCmdConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_LevelControlCmdMoveReq, ZBPRO_ZCL_LevelControlCmdMoveReqDescr_t, ZBPRO_ZCL_LevelControlCmdConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_LevelControlCmdStepReq, ZBPRO_ZCL_LevelControlCmdStepReqDescr_t, ZBPRO_ZCL_LevelControlCmdConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_LevelControlCmdStopReq, ZBPRO_ZCL_LevelControlCmdStopReqDescr_t, ZBPRO_ZCL_LevelControlCmdConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_DoorLockCmdLockReq, ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t, ZBPRO_ZCL_DoorLockCmdLockUnlockConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_DoorLockCmdUnlockReq, ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t, ZBPRO_ZCL_DoorLockCmdLockUnlockConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_WindowCoveringCmdUpOpenReq, ZBPRO_ZCL_WindowCoveringCmdReqDescr_t, ZBPRO_ZCL_WindowCoveringCmdConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_WindowCoveringCmdDownCloseReq, ZBPRO_ZCL_WindowCoveringCmdReqDescr_t, ZBPRO_ZCL_WindowCoveringCmdConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_WindowCoveringCmdStopReq, ZBPRO_ZCL_WindowCoveringCmdReqDescr_t, ZBPRO_ZCL_WindowCoveringCmdConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_WindowCoveringCmdGotoLiftPecentageReq, ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t, ZBPRO_ZCL_WindowCoveringCmdConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_WindowCoveringCmdGotoTiltPecentageReq, ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t, ZBPRO_ZCL_WindowCoveringCmdConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReq, ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqDescr_t, ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_SapIasAceArmRespReq, ZBPRO_ZCL_SapIasAceArmRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_SapIasAceBypassRespReq, ZBPRO_ZCL_SapIasAceBypassRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReq, ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneInfoRespReq, ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_SapIasAceGetPanelStatusRespReq, ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReq, ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneStatusRespReq, ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_SapIasAceZoneStatusChangedReq, ZBPRO_ZCL_SapIasAceZoneStatusChangedReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_SapIasAcePanelStatusChangedReq, ZBPRO_ZCL_SapIasAcePanelStatusChangedReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_IASWDCmdStartWarningReq, ZBPRO_ZCL_IASWDCmdStartWarningReqDescr_t, ZBPRO_ZCL_IASWDCmdStartWarningConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_IASWDCmdSquawkgReq, ZBPRO_ZCL_IASWDCmdSquawkReqDescr_t, ZBPRO_ZCL_IASWDCmdSquawkConfParams_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_IdentifyInd, ZBPRO_ZCL_IdentifyIndParams_t, NoAppropriateType_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_ProfileWideCmdReportAttributesInd, ZBPRO_ZCL_ProfileWideCmdReportAttributesIndParams_t, NoAppropriateType_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_GroupsCmdGetGroupMembershipResponseInd, ZBPRO_ZCL_GroupsCmdGetGroupMembershipIndParams_t, NoAppropriateType_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseInd, ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseIndParams_t, NoAppropriateType_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestInd, ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestIndParams_t, NoAppropriateType_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationInd, ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationIndParams_t, NoAppropriateType_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_SapIasAceArmInd, ZBPRO_ZCL_SapIasAceArmIndParams_t, NoAppropriateType_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_SapIasAceBypassInd, ZBPRO_ZCL_SapIasAceBypassIndParams_t, NoAppropriateType_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_SapIasAceEmergencyInd, ZBPRO_ZCL_SapIasAceAlarmIndParams_t, NoAppropriateType_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_SapIasAceFireInd, ZBPRO_ZCL_SapIasAceAlarmIndParams_t, NoAppropriateType_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_SapIasAcePanicInd, ZBPRO_ZCL_SapIasAceAlarmIndParams_t, NoAppropriateType_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneIdMapInd, ZBPRO_ZCL_SapIasAceGetZoneIdMapIndParams_t, NoAppropriateType_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneInfoInd, ZBPRO_ZCL_SapIasAceGetZoneInfoIndParams_t, NoAppropriateType_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_SapIasAceGetPanelStatusInd, ZBPRO_ZCL_SapIasAceGetPanelStatusIndParams_t, NoAppropriateType_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_SapIasAceGetBypassedZoneListInd, ZBPRO_ZCL_SapIasAceGetBypassedZoneListIndParams_t, NoAppropriateType_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_SapIasAceGetZoneStatusInd, ZBPRO_ZCL_SapIasAceGetZoneStatusIndParams_t, NoAppropriateType_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ColorControlCmdMoveToColorReq, ZBPRO_ZCL_ColorControlCmdMoveToColorReqDescr_t, ZBPRO_ZCL_ColorControlCmdMoveToColorConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ColorControlCmdMoveColorReq, ZBPRO_ZCL_ColorControlCmdMoveColorReqDescr_t, ZBPRO_ZCL_ColorControlCmdMoveColorConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ColorControlCmdStepColorReq, ZBPRO_ZCL_ColorControlCmdStepColorReqDescr_t, ZBPRO_ZCL_ColorControlCmdStepColorConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReq, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqDescr_t, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReq, ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqDescr_t, ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReq, ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqDescr_t, ZBPRO_ZCL_ColorControlCmdEnhancedStepHueConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReq, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqDescr_t, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ColorControlCmdColorLoopSetReq, ZBPRO_ZCL_ColorControlCmdColorLoopSetReqDescr_t, ZBPRO_ZCL_ColorControlCmdColorLoopSetConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ColorControlCmdStopMoveStepReq, ZBPRO_ZCL_ColorControlCmdStopMoveStepReqDescr_t, ZBPRO_ZCL_ColorControlCmdStopMoveStepConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReq, ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqDescr_t, ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReq, ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqDescr_t, ZBPRO_ZCL_ColorControlCmdStepColorTemperatureConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZHA_EzModeReq, ZBPRO_ZHA_EzModeReqDescr_t, ZBPRO_ZHA_EzModeConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZHA_CieDeviceEnrollReq, ZBPRO_ZHA_CieEnrollReqDescr_t, ZBPRO_ZHA_CieEnrollConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZHA_CieDeviceSetPanelStatusReq, ZBPRO_ZHA_CieSetPanelStatusReqDescr_t, ZBPRO_ZHA_CieSetPanelStatusConfParams_t)

DECLARE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZHA_CieZoneSetBypassStateReq, ZBPRO_ZHA_CieZoneSetBypassStateReqDescr_t, ZBPRO_ZHA_CieZoneSetBypassStateConfParams_t)

DECLARE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZHA_CieDeviceSetPanelStatusInd, ZBPRO_ZHA_CieSetPanelStatusIndParams_t, NoAppropriateType_t)

#endif

#if 0  /* deleted by huajun */
/* MAC API unit test */
/* RF4CE NWK API unit test */
extern void RF4CE_NWK_StartReq_Call(RF4CE_StartReqDescr_t *reqDescr, int socket);
extern void RF4CE_NWK_DataReq_Call(RF4CE_DataReqDescr_t *reqDescr, int socket);
extern void RF4CE_NWK_GetReq_Call(RF4CE_GetReqDescr_t *reqDescr, int socket);
extern void RF4CE_NWK_SetReq_Call(RF4CE_SetReqDescr_t *reqDescr, int socket);
extern void RF4CE_NWK_DiscoveryReq_Call(RF4CE_DiscoveryReqDescr_t *reqDescr, int socket);
extern void RF4CE_NWK_AutoDiscoveryReq_Call(RF4CE_AutoDiscoveryReqDescr_t *reqDescr, int socket);
extern void RF4CE_NWK_PairReq_Call(RF4CE_PairReqDescr_t *reqDescr, int socket);
extern void RF4CE_NWK_UnpairReq_Call(RF4CE_UnpairReqDescr_t *reqDescr, int socket);
extern void RF4CE_NWK_RxEnableReq_Call(RF4CE_RXEnableReqDescr_t *reqDescr, int socket);
extern void RF4CE_NWK_UpdateKeyReq_Call(RF4CE_UpdateKeyReqDescr_t *reqDescr, int socket);
extern void RF4CE_NWK_DiscoveryResp_Call(RF4CE_DiscoveryRespDescr_t *reqDescr, int socket);
extern void RF4CE_NWK_PairResp_Call(RF4CE_PairRespDescr_t *reqDescr, int socket);
extern void RF4CE_NWK_UnpairResp_Call(RF4CE_UnpairRespDescr_t *reqDescr, int socket);
extern void RF4CE_NWK_DataInd_Call(RF4CE_DataIndParams_t *indParams, int socket);
extern void RF4CE_NWK_DiscoveryInd_Call(RF4CE_DiscoveryIndParams_t *indParams, int socket);
extern void RF4CE_NWK_COMMStatusInd_Call(RF4CE_COMMStatusIndParams_t *indParams, int socket);
extern void RF4CE_NWK_PairInd_Call(RF4CE_PairIndParams_t *indParams, int socket);
extern void RF4CE_NWK_UnpairInd_Call(RF4CE_UnpairIndParams_t *indParams, int socket);

/* ZBPRO APS API unit test */
extern void ZBPRO_APS_DataReq_Call(ZBPRO_APS_DataReqDescr_t *req, int socket);
extern void ZBPRO_APS_BindReq_Call(ZBPRO_APS_BindUnbindReqDescr_t *reqDescr, int socket);
extern void ZBPRO_APS_UnbindReq_Call(ZBPRO_APS_BindUnbindReqDescr_t *reqDescr, int socket);
extern void ZBPRO_APS_GetReq_Call(ZBPRO_APS_GetReqDescr_t *req, int socket);
extern void ZBPRO_APS_SetReq_Call(ZBPRO_APS_SetReqDescr_t *req, int socket);
extern void ZBPRO_APS_AddGroupReq_Call(ZBPRO_APS_AddGroupReqDescr_t *req, int socket);
extern void ZBPRO_APS_RemoveGroupReq_Call(ZBPRO_APS_RemoveGroupReqDescr_t *req, int socket);
extern void ZBPRO_APS_RemoveAllGroupsReq_Call(ZBPRO_APS_RemoveAllGroupsReqDescr_t *req, int socket);
extern void ZBPRO_APS_TransportKeyReq_Call(ZBPRO_APS_TransportKeyReqDescr_t *req, int socket);
extern void ZBPRO_APS_UpdateDeviceReq_Call(ZBPRO_APS_UpdateDeviceReqDescr_t *req, int socket);
extern void ZBPRO_APS_RemoveDeviceReq_Call(ZBPRO_APS_RemoveDeviceReqDescr_t *req, int socket);
extern void ZBPRO_APS_RequestKeyReq_Call(ZBPRO_APS_RequestKeyReqDescr_t *req, int socket);
extern void ZBPRO_APS_SwitchKeyReq_Call(ZBPRO_APS_SwitchKeyReqDescr_t *req, int socket);
extern void ZBPRO_APS_DataInd_Call(ZBPRO_APS_DataIndParams_t *indParams, int socket);
extern void ZBPRO_APS_TransportKeyInd_Call(ZBPRO_APS_TransportKeyIndParams_t *indParams, int socket);
extern void ZBPRO_APS_UpdateDeviceInd_Call(ZBPRO_APS_UpdateDeviceIndParams_t *indParams, int socket);
extern void ZBPRO_APS_RemoveDeviceInd_Call(ZBPRO_APS_RemoveDeviceIndParams_t *indParams, int socket);
extern void ZBPRO_APS_RequestKeyInd_Call(ZBPRO_APS_RequestKeyIndParams_t *indParams, int socket);
extern void ZBPRO_APS_SwitchKeyInd_Call(ZBPRO_APS_SwitchKeyIndParams_t *indParams, int socket);

/* RF4CE Profile manager API unit test */
extern void RF4CE_UnpairReq_Call(RF4CE_UnpairReqDescr_t *request, int socket);
extern void RF4CE_StartReq_Call(RF4CE_StartReqDescr_t *request, int socket);
extern void RF4CE_ResetReq_Call(RF4CE_ResetReqDescr_t *request, int socket);
extern void RF4CE_CounterExpiredInd_Call(RF4CE_PairingReferenceIndParams_t *indication, int socket);
extern void RF4CE_UnpairInd_Call(RF4CE_PairingReferenceIndParams_t *indication, int socket);
extern void RF4CE_PairInd_Call(RF4CE_PairingReferenceIndParams_t *indication, int socket);

/* RF4CE GDP Profile API unit test */
extern void RF4CE_GDP_GetAttributesReq_Call(RF4CE_GDP_AttributeDescr_t *request, int socket);
extern void RF4CE_GDP_SetAttributesReq_Call(RF4CE_GDP_AttributeDescr_t *request, int socket);
extern void RF4CE_GDP_PushAttributesReq_Call(RF4CE_GDP_AttributeDescr_t *request, int socket);
extern void RF4CE_GDP_PullAttributesReq_Call(RF4CE_GDP_AttributeDescr_t *request, int socket);
extern void RF4CE_GDP_KeyExchangeReq_Call(RF4CE_GDP_KeyExchangeReqDescr_t *request, int socket);
extern void RF4CE_GDP_BindReq_Call(RF4CE_GDP_BindReqDescr_t *request, int socket);
extern void RF4CE_GDP_ProxyBindReq_Call(RF4CE_GDP_ProxyBindReqDescr_t *request, int socket);
extern void RF4CE_GDP_HeartbeatReq_Call(RF4CE_GDP_HeartbeatReqDescr_t *request, int socket);
extern void RF4CE_GDP_EnableBindingReq_Call(RF4CE_GDP_BindingReqDescr_t *request, int socket);
extern void RF4CE_GDP_DisableBindingReq_Call(RF4CE_GDP_BindingReqDescr_t *request, int socket);
extern void RF4CE_GDP_ButtonBindingReq_Call(RF4CE_GDP_ButtonBindingReqDescr_t *request, int socket);
extern void RF4CE_GDP_ClientNotificationReq_Call(RF4CE_GDP_ClientNotificationReqDescr_t *request, int socket);
extern void RF4CE_GDP_PushAttributesInd_Call(RF4CE_GDP_HostAttributesReqDescr_t *request, int socket);
extern void RF4CE_GDP_PullAttributesInd_Call(RF4CE_GDP_HostAttributesReqDescr_t *request, int socket);
extern void RF4CE_GDP_GetSharedSecretInd_Call(RF4CE_GDP_GetSharedSecretIndDescr_t *request, int socket);
extern void RF4CE_GDP_HeartbeatInd_Call(RF4CE_GDP_HeartbeatIndParams_t *indication, int socket);
extern void RF4CE_GDP_CheckValidationInd_Call(RF4CE_GDP_CheckValidationIndDescr_t *indication, int socket);
extern void RF4CE_GDP_ClientNotificationInd_Call(RF4CE_GDP_ClientNotificationIndParams_t *indication, int socket);

/* RF4CE ZRC Profile API unit test */
extern void RF4CE_ZRC_GetAttributesReq_Call(RF4CE_GDP_AttributeDescr_t *request, int socket);
extern void RF4CE_ZRC_SetAttributesReq_Call(RF4CE_GDP_AttributeDescr_t *request, int socket);
extern void RF4CE_ZRC_PushAttributesReq_Call(RF4CE_GDP_AttributeDescr_t *request, int socket);
extern void RF4CE_ZRC_PullAttributesReq_Call(RF4CE_GDP_AttributeDescr_t *request, int socket);
extern void RF4CE_ZRC_KeyExchangeReq_Call(RF4CE_GDP_KeyExchangeReqDescr_t *request, int socket);
extern void RF4CE_ZRC_BindReq_Call(RF4CE_GDP_BindReqDescr_t *request, int socket);
extern void RF4CE_ZRC_ProxyBindReq_Call(RF4CE_GDP_ProxyBindReqDescr_t *request, int socket);
extern void RF4CE_ZRC_HeartbeatReq_Call(RF4CE_GDP_HeartbeatReqDescr_t *request, int socket);
extern void RF4CE_ZRC_ControlCommandReq_Call(RF4CE_ZRC_ControlCommandReqDescr_t *request, int socket);
extern void RF4CE_ZRC_EnableBindingReq_Call(RF4CE_GDP_BindingReqDescr_t *request, int socket);
extern void RF4CE_ZRC_DisableBindingReq_Call(RF4CE_GDP_BindingReqDescr_t *request, int socket);
extern void RF4CE_ZRC_ButtonBindingReq_Call(RF4CE_GDP_ButtonBindingReqDescr_t *request, int socket);
extern void RF4CE_ZRC_ClientNotificationReq_Call(RF4CE_GDP_ClientNotificationReqDescr_t *request, int socket);
extern void RF4CE_ZRC_PushAttributesInd_Call(RF4CE_GDP_HostAttributesReqDescr_t *request, int socket);
extern void RF4CE_ZRC_PullAttributesInd_Call(RF4CE_GDP_HostAttributesReqDescr_t *request, int socket);
extern void RF4CE_ZRC_GetAttributesInd_Call(RF4CE_GDP_HostAttributesReqDescr_t *request, int socket);
extern void RF4CE_ZRC_SetAttributesInd_Call(RF4CE_GDP_HostAttributesReqDescr_t *request, int socket);
extern void RF4CE_ZRC_GetSharedSecretInd_Call(RF4CE_GDP_GetSharedSecretIndDescr_t *request, int socket);
extern void RF4CE_ZRC_CheckValidationInd_Call(RF4CE_GDP_CheckValidationIndDescr_t *indication, int socket);
extern void RF4CE_ZRC_ClientNotificationInd_Call(RF4CE_GDP_ClientNotificationIndParams_t *indication, int socket);
extern void RF4CE_ZRC_HeartbeatInd_Call(RF4CE_GDP_HeartbeatIndParams_t *indication, int socket);
extern void RF4CE_ZRC_ControlCommandInd_Call(RF4CE_ZRC_ControlCommandIndParams_t *indication, int socket);


#ifdef TEST
    int ClientLoopbackReq(unsigned int *tx_buffer, unsigned int num_of_words, void (*callback)(unsigned int *rx_buffer, unsigned int num_of_words), unsigned int *rx_buffer);
    int ClientCoreLoopbackReq(unsigned int *tx_buffer, unsigned int num_of_words, void (*callback)(unsigned int *rx_buffer, unsigned int num_of_words), unsigned int *rx_buffer);
    void ServerLoopbackInd(int socket, unsigned int *tx_buffer, unsigned int *rx_buffer, unsigned int num_of_words, void (*callback)(unsigned int *tx_buffer, unsigned int *rx_buffer, unsigned int num_of_words));
#endif

#endif
#endif /*_ZIGBEE_API_H_*/
