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
#include "bbSysTypes.h"
#include "bbSysNvmManager.h"
#include "bbSysEvent.h"

#ifdef MAILBOX_UNIT_TEST
#   include "bbMailUnitTestFunctions.h"
#endif
#include "bbMailTestEngine.h"
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
# include "bbZbProZhaSapCieDevice.h"

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
# include "private/bbZbProZhaEzMode.h"
#endif /* _ZBPRO_ */

#ifdef _RF4CE_
# include "bbMacSapTypesBanTable.h" // for cert tests

# include "bbMacSapForRF4CE.h"
# include "bbMailPowerFilterKey.h"
# include "bbRF4CENWK.h"
# include "bbRF4CEPM.h"
# include "bbRF4CEZRC1.h"
# include "bbRF4CEZRC.h"
# include "bbRF4CEMSO.h"
#endif

#include "bbSysPrint.h"
#include "bbPhySapTest.h"
#include "bbRF4CEDiag.h"
#include "bbHalUsartMailboxAdapter.h"
/************************* TYPES ********************************************************/
/**//**
 * \brief Enumeration of API function's id
 */
typedef enum
{
    INCORRECT_REQ_ID                = 0x0000U, // 0
    FIRST_SERVICE_FID               = 0x0200U,
    MAIL_ACK_FID                    = FIRST_SERVICE_FID,
    TE_ASSERT_LOGID_FID             = 0x0201U,
    TE_ASSERT_ERRID_FID             = 0x0202U,
    TE_HOST_TO_UART1_FID            = 0x0203U,
    TE_UART1_TO_HOST_FID            = 0x0204U,
    /* MAILBOX_UNIT_TEST section start */
    MAIL_F1_FID                     = 0x0001U, // 1
    MAIL_F2_FID                     = 0x0002U, // 2
    MAIL_F3_FID                     = 0x0003U, // 3
    MAIL_F4_FID                     = 0x0004U, // 4
    MAIL_F5_FID                     = 0x0005U, // 5
    /* MAILBOX_UNIT_TEST section end */

    /* TEST_HARNESS section start */
    /* NOTE: may be better move this section to the service id area. Needs testing. */
    TE_PING_FID                     = 0x0006U, // 6
    TE_ECHO_FID                     = 0x0007U, // 7
    TE_RESET_FID                    = 0x0008U, // 8
    TE_HELLO_FID                    = 0x0009U, // 9
    /* TEST_HARNESS section end */

    /* MAC section start */
    MAC_REQ_BAN_SET_DEFAULT_ACTION_FID,
    MAC_REQ_BAN_LINK_FID,
    /* MAC ZBPRO context */
    ZBPRO_MAC_REQ_DATA_FID,
    ZBPRO_MAC_REQ_PURGE_FID,
    ZBPRO_MAC_REQ_ASSOCIATE_FID,
    ZBPRO_MAC_REQ_GET_FID,
    ZBPRO_MAC_REQ_RESET_FID,
    ZBPRO_MAC_REQ_RX_ENABLE_FID,
    ZBPRO_MAC_REQ_SCAN_FID,
    ZBPRO_MAC_REQ_SET_FID,
    ZBPRO_MAC_REQ_START_FID,
    ZBPRO_MAC_RESP_ASSOCIATE_FID,
    ZBPRO_MAC_RESP_ORPHAN_FID,
    ZBPRO_MAC_IND_DATA_FID,
    ZBPRO_MAC_IND_ASSOCIATE_FID,
    ZBPRO_MAC_IND_BEACON_NOTIFY_FID,
    ZBPRO_MAC_IND_COMM_STATUS_FID,
    ZBPRO_MAC_IND_ORPHAN_FID,
    ZBPRO_MAC_IND_POLL_FID,
    /* MAC RF4CE context */
    RF4CE_MAC_REQ_RESET_FID,
    RF4CE_MAC_REQ_START_FID,
    RF4CE_MAC_REQ_DATA_FID,
    RF4CE_MAC_REQ_GET_FID,
    RF4CE_MAC_REQ_SET_FID,
    RF4CE_MAC_REQ_RX_ENABLE_FID,
    RF4CE_MAC_REQ_SCAN_FID,
    RF4CE_MAC_IND_DATA_FID,
    RF4CE_MAC_IND_BEACON_NOTIFY_FID,
    RF4CE_MAC_IND_COMM_STATUS_FID,
    /* MAC section end */

    /* RF4CE NWK section start */
    RF4CE_NWK_REQ_RESET_FID,
    RF4CE_NWK_REQ_START_FID,
    RF4CE_NWK_REQ_DATA_FID,
    RF4CE_NWK_REQ_GET_FID,
    RF4CE_NWK_REQ_SET_FID,
    RF4CE_NWK_REQ_DISCOVERY_FID,
    RF4CE_NWK_REQ_AUTODISCOVERY_FID,
    RF4CE_NWK_REQ_PAIR_FID,
    RF4CE_NWK_REQ_UNPAIR_FID,
    RF4CE_NWK_REQ_RXENABLE_FID,
    RF4CE_NWK_REQ_UPDATEKEY_FID,

    RF4CE_NWK_RESP_DISCOVERY_FID,
    RF4CE_NWK_RESP_PAIR_FID,
    RF4CE_NWK_RESP_UNPAIR_FID,

    RF4CE_NWK_IND_DATA_FID,
    RF4CE_NWK_IND_DISCOVERY_FID,
    RF4CE_NWK_IND_COMMSTATUS_FID,
    RF4CE_NWK_IND_PAIR_FID,
    RF4CE_NWK_IND_UNPAIR_FID,
    /* RF4CE NWK section end */

    /* ZBPRO NWK section start */
    ZBPRO_NWK_REQ_PERMIT_JOINING_FID,
    ZBPRO_NWK_REQ_LEAVE_FID,
    ZBPRO_NWK_REQ_GET_FID,              /* just reserved, tests use only ZBPRO_APS_REQ_GET_FID */
    ZBPRO_NWK_REQ_SET_FID,              /* just reserved, tests use only ZBPRO_APS_REQ_SET_FID */
    ZBPRO_NWK_REQ_GET_KEY_FID,
    ZBPRO_NWK_REQ_SET_KEY_FID,
    ZBPRO_NWK_REQ_ROUTE_DISCOVERY_FID,
    /* ZBPRO NWK section end */

    /* ZBPRO APS section start */
    ZBPRO_APS_REQ_ENDPOINTREGISTER_FID,
    ZBPRO_APS_REQ_ENDPOINTUNREGISTER_FID,
    ZBPRO_APS_REQ_DATA_FID,
    ZBPRO_APS_REQ_BIND_FID,
    ZBPRO_APS_REQ_UNBIND_FID,
    ZBPRO_APS_REQ_GET_FID,
    ZBPRO_APS_REQ_SET_FID,
    ZBPRO_APS_REQ_GET_KEY_FID,
    ZBPRO_APS_REQ_SET_KEY_FID,
    ZBPRO_APS_REQ_ADDGROUP_FID,
    ZBPRO_APS_REQ_REMOVEGROUP_FID,
    ZBPRO_APS_REQ_REMOVEALLGROUPS_FID,

    ZBPRO_APS_IND_DATA_FID,

    /* APS Layer Security Primitives */
    ZBPRO_APS_REQ_TRANSPORTKEY_FID,
    ZBPRO_APS_REQ_UPDATEDEVICE_FID,
    ZBPRO_APS_REQ_REMOTEDEVICE_FID,
    ZBPRO_APS_REQ_REQUESTKEY_FID,
    ZBPRO_APS_REQ_SWITCHKEY_FID,

    ZBPRO_APS_IND_TRANSPORTKEY_FID,
    ZBPRO_APS_IND_UPDATEDEVICE_FID,
    ZBPRO_APS_IND_REMOTEDEVICE_FID,
    ZBPRO_APS_IND_REQUESTKEY_FID,
    ZBPRO_APS_IND_SWITCHKEY_FID,
    /* ZBPRO APS section end */

    /* ZBPRO ZDO section start */
    ZBPRO_ZDO_REQ_ADDR_RESOLVING_FID,
    ZBPRO_ZDO_REQ_NODE_DESC_FID,
    ZBPRO_ZDO_REQ_POWER_DESC_FID,
    ZBPRO_ZDO_REQ_SIMPLE_DESC_FID,
    ZBPRO_ZDO_REQ_ACTIVE_EP_FID,
    ZBPRO_ZDO_REQ_MATCH_DESC_FID,
    ZBPRO_ZDO_REQ_DEVICE_ANNCE_FID,
    ZBPRO_ZDO_REQ_SERVER_DISCOVERY_FID,
    ZBPRO_ZDO_REQ_ED_BIND_FID,
    ZBPRO_ZDO_REQ_BIND_FID,
    ZBPRO_ZDO_REQ_UNBIND_FID,
    ZBPRO_ZDO_REQ_START_NETWORK_FID,
    ZBPRO_ZDO_REQ_MGMT_LEAVE_FID,
    ZBPRO_ZDO_REQ_MGMT_LQI_FID,
    ZBPRO_ZDO_REQ_MGMT_PERMIT_JOINING_FID,
    ZBPRO_ZDO_REQ_MGMT_NWK_UPDATE_FID,
    ZBPRO_ZDO_RESP_MGMT_NWK_UPDATE_UNSOL_FID,
    ZBPRO_ZDO_IND_MGMT_NWK_UPDATE_UNSOL_FID,
    ZBPRO_ZDO_REQ_MGMT_BIND_FID ,
    /* ZBPRO ZDO section end */

    /* ZBPRO TC section start */
    ZBPRO_TC_REQ_NWK_KEY_UPDATE_FID,
    /* ZBPRO TC section end */

    /* ZBPRO ZCL section start */
    ZBPRO_ZCL_REQ_SET_POWER_SOURCE_FID,
    ZBPRO_ZCL_IND_IDENTIFY_FID,
    ZBPRO_ZCL_PROFILE_WIDE_CMD_READ_ATTRIBUTE_REQ_FID,
    ZBPRO_ZCL_PROFILE_WIDE_CMD_WRITE_ATTRIBUTE_REQ_FID,
    ZBPRO_ZCL_PROFILE_WIDE_CMD_DISCOVER_ATTRIBUTE_REQ_FID,
    ZBPRO_ZCL_PROFILE_WIDE_CMD_CONFIGURE_REPORTING_REQ_FID,
    ZBPRO_ZCL_PROFILE_WIDE_CMD_READ_REPORTING_CONFIGURATION_REQ_FID,
    ZBPRO_ZCL_PROFILE_WIDE_CMD_REPORT_ATTRIBUTES_IND_FID,
    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_REQ_FID,
    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_RESPONSE_REQ_FID,
    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_REQ_FID,
    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_RESPONSE_REQ_FID,
    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_IND_FID,
    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_IND_FID,
    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_RESPONSE_IND_FID,
    ZBPRO_ZCL_GROUPS_CMD_ADD_GROUP_REQ_FID,
    ZBPRO_ZCL_GROUPS_CMD_VIEW_GROUP_REQ_FID,
    ZBPRO_ZCL_GROUPS_CMD_GET_GROUP_MEMBERSHIP_REQ_FID,
    ZBPRO_ZCL_GROUPS_CMD_GET_GROUP_MEMBERSHIP_RESPONSE_IND_FID,
    ZBPRO_ZCL_GROUPS_CMD_REMOVE_GROUP_REQ_FID,
    ZBPRO_ZCL_GROUPS_CMD_REMOVE_ALL_GROUPS_REQ_FID,
    ZBPRO_ZCL_GROUPS_CMD_ADD_GROUP_IF_IDENTIFY_REQ_FID,
    ZBPRO_ZCL_SCENES_CMD_ADD_SCENE_REQ_FID,
    ZBPRO_ZCL_SCENES_CMD_VIEW_SCENE_REQ_FID,
    ZBPRO_ZCL_SCENES_CMD_STORE_SCENE_REQ_FID,
    ZBPRO_ZCL_SCENES_CMD_REMOVE_SCENE_REQ_FID,
    ZBPRO_ZCL_SCENES_CMD_REMOVE_ALL_SCENES_REQ_FID,
    ZBPRO_ZCL_SCENES_CMD_RECALL_SCENE_REQ_FID,
    ZBPRO_ZCL_SCENES_CMD_GET_SCENE_MEMBERSHIP_REQ_FID,
    ZBPRO_ZCL_SCENES_CMD_GET_SCENE_MEMBERSHIP_RESPONSE_IND_FID,
    ZBPRO_ZCL_ONOFF_CMD_OFF_REQ_FID,
    ZBPRO_ZCL_ONOFF_CMD_ON_REQ_FID,
    ZBPRO_ZCL_ONOFF_CMD_TOGGLE_REQ_FID,
    ZBPRO_ZCL_LEVEL_CONTROL_CMD_MOVE_TO_LEVEL_REQ_FID,
    ZBPRO_ZCL_LEVEL_CONTROL_CMD_MOVE_REQ_FID,
    ZBPRO_ZCL_LEVEL_CONTROL_CMD_STEP_REQ_FID,
    ZBPRO_ZCL_LEVEL_CONTROL_CMD_STOP_REQ_FID,
    ZBPRO_ZCL_DOOR_LOCK_CMD_LOCK_REQ_FID,
    ZBPRO_ZCL_DOOR_LOCK_CMD_UNLOCK_REQ_FID,
    ZBPRO_ZCL_WINDOW_COVERING_CMD_UP_OPEN_REQ_FID,
    ZBPRO_ZCL_WINDOW_COVERING_CMD_DOWN_CLOSE_REQ_FID,
    ZBPRO_ZCL_WINDOW_COVERING_CMD_STOP_REQ_FID,
    ZBPRO_ZCL_WINDOW_COVERING_CMD_GO_TO_LIFT_PERCENTAGE_REQ_FID,
    ZBPRO_ZCL_WINDOW_COVERING_CMD_GO_TO_TILT_PERCENTAGE_REQ_FID,
    ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_ENROLL_IND_FID,
    ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_ENROLL_RESPONSE_REQ_FID,
    ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_STATUS_CHANGED_NOTIFICATION_IND_FID,
    ZBPRO_ZCL_IAS_ACE_ARM_IND_FID,
    ZBPRO_ZCL_IAS_ACE_BYPASS_IND_FID,
    ZBPRO_ZCL_IAS_ACE_EMERGENCY_IND_FID,
    ZBPRO_ZCL_IAS_ACE_FIRE_IND_FID,
    ZBPRO_ZCL_IAS_ACE_PANIC_IND_FID,
    ZBPRO_ZCL_IAS_ACE_GET_ZONE_ID_MAP_IND_FID,
    ZBPRO_ZCL_IAS_ACE_GET_ZONE_INFO_IND_FID,
    ZBPRO_ZCL_IAS_ACE_GET_PANEL_STATUS_IND_FID,
    ZBPRO_ZCL_IAS_ACE_GET_BYPASSED_ZONE_LIST_IND_FID,
    ZBPRO_ZCL_IAS_ACE_GET_ZONE_STATUS_IND_FID,
    ZBPRO_ZCL_IAS_ACE_ARM_RESP_REQ_FID,
    ZBPRO_ZCL_IAS_ACE_BYPASS_RESP_REQ_FID,
    ZBPRO_ZCL_IAS_ACE_GET_ZONE_ID_MAP_RESP_REQ_FID,
    ZBPRO_ZCL_IAS_ACE_GET_ZONE_INFO_RESP_REQ_FID,
    ZBPRO_ZCL_IAS_ACE_GET_PANEL_STATUS_RESP_REQ_FID,
    ZBPRO_ZCL_IAS_ACE_SET_BYPASSED_ZONE_LIST_RESP_REQ_FID,
    ZBPRO_ZCL_IAS_ACE_GET_ZONE_STATUS_RESP_REQ_FID,
    ZBPRO_ZCL_IAS_ACE_ZONE_STATUS_CHANGED_REQ_FID,
    ZBPRO_ZCL_IAS_ACE_PANEL_STATUS_CHANGED_REQ_FID,
    ZBPRO_ZCL_IAS_WD_CMD_START_WARNING_REQ_FID,
    ZBPRO_ZCL_IAS_WD_CMD_SQUAWK_REQ_FID,
    ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_TO_COLOR_REQ_FID,
    ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_COLOR_REQ_FID,
    ZBPRO_ZCL_COLOR_CONTROL_CMD_STEP_COLOR_REQ_FID,
    ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_TO_HUE_REQ_FID,
    ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_HUE_REQ_FID,
    ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_STEP_HUE_REQ_FID,
    ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_TO_HUE_AND_SATURATION_REQ_FID,
    ZBPRO_ZCL_COLOR_CONTROL_CMD_COLOR_LOOP_SET_REQ_FID,
    ZBPRO_ZCL_COLOR_CONTROL_CMD_STOP_MOVE_STEP_REQ_FID,
    ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_COLOR_TEMPERATURE_REQ_FID,
    ZBPRO_ZCL_COLOR_CONTROL_CMD_STEP_COLOR_TEMPERATURE_REQ_FID,
    /* ZBPRO ZCL section end */

    /* ZBPRO NWK section start */
    ZBPRO_ZHA_EZ_MODE_FID,
    ZBPRO_ZHA_CIE_ENROLL_FID,
    ZBPRO_ZHA_CIE_ENROLL_IND_FID,
    /* ZBPRO NWK section end */

    /* ZHA device section start. */
    ZBPRO_ZHA_CIE_SET_PANEL_STATUS_REQ_FID,
    ZBPRO_ZHA_CIE_SET_PANEL_STATUS_IND_FID,
    ZBPRO_ZHA_CIE_ZONE_SET_BYPASS_STATE_REQ_FID,
    /* ZHA device section end. */

    /* RF4CE PROFILE MANAGER section start */
    RF4CE_PROFILE_REQ_UNPAIR_FID,
    RF4CE_PROFILE_REQ_START_FID,
    RF4CE_PROFILE_REQ_RESET_FID,
    RF4CE_PROFILE_REQ_SET_SUP_DEVICES_FID,

    RF4CE_PROFILE_IND_COUNTER_EXPIRED_FID,
    RF4CE_PROFILE_IND_UNPAIR_FID,
    RF4CE_PROFILE_IND_PAIR_FID,
    /* RF4CE PROFILE MANAGER section end */

    /* RF4CE ZRC 1.1 section start */
    RF4CE_ZRC1_REQ_GET_FID,
    RF4CE_ZRC1_REQ_SET_FID,
    RF4CE_ZRC1_REQ_CONTROLLER_BIND_FID,
    RF4CE_ZRC1_REQ_TARGET_BIND_FID,
    RF4CE_ZRC1_REQ_COMMANDDISCOVERY_FID,
    RF4CE_ZRC1_CONTROL_COMMAND_PRESSED_FID,
    RF4CE_ZRC1_CONTROL_COMMAND_RELEASED_FID,
    RF4CE_ZRC1_IND_CONTROLCOMMAND_FID,
    RF4CE_ZRC1_VENDORSPECIFIC_REQ_FID,
    RF4CE_ZRC1_VENDORSPECIFIC_IND_FID,
    /* RF4CE ZRC 1.1 section end */

    /* RF4CE GDP 2.0 & ZRC 2.0 section start */
    RF4CE_ZRC2_REQ_GET_FID,
    RF4CE_ZRC2_REQ_SET_FID,
    RF4CE_ZRC2_KEY_EXCHANGE_FID,
    RF4CE_ZRC2_BIND_FID,
    RF4CE_ZRC2_PROXY_BIND_FID,
    RF4CE_ZRC2_ENABLE_BINDING_FID,
    RF4CE_ZRC2_DISABLE_BINDING_FID,
    RF4CE_ZRC2_SET_PUSH_BUTTON_STIMULUS_FID,
    RF4CE_ZRC2_CLEAR_PUSH_BUTTON_STIMULUS_FID,
    RF4CE_ZRC2_CHECK_VALIDATION_RESP_FID,
    RF4CE_ZRC2_START_VALIDATION_FID,
    RF4CE_ZRC2_CHECK_VALIDATION_IND_FID,
    RF4CE_ZRC2_CONTROL_COMMAND_PRESS_REQ_FID,
    RF4CE_ZRC2_CONTROL_COMMAND_RELEASE_REQ_FID,
    RF4CE_ZRC2_CONTROL_COMMAND_IND_FID,
    RF4CE_GDP2_REQ_SET_POLL_CONSTRAINTS_FID,
    RF4CE_GDP2_REQ_POLL_NEGOTIATION_FID,
    RF4CE_GDP2_REQ_POLL_CLIENT_USER_EVENT_FID,
    RF4CE_GDP2_REQ_CLIENT_NOTIFICATION_FID,
    RF4CE_GDP2_REQ_IDENTIFY_CAP_ANNOUNCE_FID,
    RF4CE_GDP2_REQ_IDENTIFY_FID,
    RF4CE_GDP2_IND_POLL_NEGOTIATION_FID,
    RF4CE_GDP2_IND_HEARTBEAT_FID,
    RF4CE_GDP2_IND_CLIENT_NOTIFICATION_FID,
    RF4CE_GDP2_IND_IDENTIFY_CAP_ANNOUNCE_FID,
    RF4CE_GDP2_IND_IDENTIFY_FID,
    RF4CE_ZRC2_GET_SHARED_SECRET_IND_FID,
    RF4CE_ZRC2_PAIR_IND_FID,
    RF4CE_ZRC2_BINDING_FINISHED_IND_FID,
    RF4CE_ZRC2_IND_GET_RESP_FID,
    RF4CE_ZRC2_IND_PUSH_REQ_FID,
    /* RF4CE GDP 2.0 & ZRC 2.0 section end */

    /* Profiling Engine section start */
    PE_RESET_FID,
    PE_GETDATA_FID,
    /* Profiling Engine section end */

    /* NVM section start */
    NVM_READ_FILE_FID,
    NVM_WRITE_FILE_FID,
    NVM_OPEN_FILE_FID,
    NVM_CLOSE_FILE_FID,
    /* NVM section end */

    /* Event system section start */
    SYS_EVENT_SUBSCRIBE_FID,
    SYS_EVENT_RAISE_FID,
    SYS_EVENT_NOTIFY_FID,
    /* Event system section end */

    /* Power key section start */
    RF4CE_ZRC_SET_WAKEUP_ACTION_CODE_FID,
    RF4CE_ZRC_GET_WAKEUP_ACTION_CODE_FID,
    /* Power key section end */

    /* RF4CE NWK Key Seed Start indication */
    RF4CE_NWK_KEY_SEED_START_IND_FID,

    /* RF4CE MSO profile */
    RF4CE_MSO_GET_PROFILE_ATTRIBUTE_FID,
    RF4CE_MSO_SET_PROFILE_ATTRIBUTE_FID,
    RF4CE_MSO_GET_RIB_ATTRIBUTE_FID,
    RF4CE_MSO_SET_RIB_ATTRIBUTE_FID,
    RF4CE_MSO_BIND_FID,
    RF4CE_MSO_TARGET_ENABLE_BIND_FID, // TODO: remove later, unused
    RF4CE_MSO_TARGET_DISABLE_BIND_FID, // TODO: remove later, unused
    RF4CE_MSO_USER_CONTROL_PRESSED_FID,
    RF4CE_MSO_USER_CONTROL_RELEASED_FID,
    RF4CE_MSO_START_VALIDATION_IND_FID,
    RF4CE_MSO_CHECK_VALIDATION_IND_FID,
    RF4CE_MSO_VALIDATE_RESP_FID,
    RF4CE_MSO_USER_CONTROL_IND_FID,

    SYS_PRINT_FID,
    RF4CE_NWK_MAC_STATS_REQ_FID,

    /* Power key section end */
    TE_ECHO_DELAY_FID,

    /* Phy test stuff start */
    RF4CE_CTRL_TEST_GET_CAPS_FID,
    RF4CE_CTRL_TEST_SET_CHANNEL_FID,
    RF4CE_CTRL_TEST_CONTINUOUS_WAVE_START_FID,
    RF4CE_CTRL_TEST_CONTINUOUS_WAVE_STOP_FID,
    RF4CE_CTRL_TEST_TRANSMIT_START_FID,
    RF4CE_CTRL_TEST_TRANSMIT_STOP_FID,
    RF4CE_CTRL_TEST_RECEIVE_START_FID,
    RF4CE_CTRL_TEST_RECEIVE_STOP_FID,
    RF4CE_CTRL_TEST_ECHO_START_FID,
    RF4CE_CTRL_TEST_ECHO_STOP_FID,
    RF4CE_CTRL_TEST_ENERGY_DETECT_SCAN_FID,
    RF4CE_CTRL_TEST_GET_STATS_FID,
    RF4CE_CTRL_TEST_RESET_STATS_FID,
    RF4CE_CTRL_TEST_SET_TX_POWER_FID,
    RF4CE_CTRL_TEST_SELECT_ANTENNA_FID,
    RF4CE_CTRL_GET_DIAGNOSTICS_CAPS_FID,
    RF4CE_CTRL_GET_DIAGNOSTIC_FID,
    /* Phy test stuff end */
    PUBLIC_FUNCTIONS_AMOUNT
} MailFID_t;
SYS_DbgAssertStatic(PUBLIC_FUNCTIONS_AMOUNT < FIRST_SERVICE_FID);

/**//**
 * \brief Union of all request descriptors.
 */
/* NOTE: used only on server side. Can be move to bbMailServerTable.h */
typedef union _MailReq_t
{
#ifdef MAILBOX_UNIT_TEST
    MailUnitTest_f1Descr_t          mailF1;
    MailUnitTest_f2Descr_t          mailF2;
    MailUnitTest_f3Descr_t          mailF3;
    MailUnitTest_f4Descr_t          mailF4;
    MailUnitTest_f5Descr_t          mailF5;
#endif /* MAILBOX_UNIT_TEST */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_TEST_ENGINE_)
    TE_PingCommandReqDescr_t        tePing;
    TE_EchoCommandReqDescr_t        teEcho;
    TE_ResetCommandReqDescr_t       teReset;
#endif /* _MAILBOX_WRAPPERS_TEST_ENGINE_ */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_PROFILING_ENGINE_)
    SYS_DbgPeResetReqDescr_t        peReset;
    SYS_DbgPeGetDataReqDescr_t      peGetData;
#endif /* _MAILBOX_WRAPPERS_PROFILING_ENGINE_ */

    // TODO: fill me as ^\}.*ReqDescr_t;$
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_MAC_)
# ifdef _MAC_BAN_TABLE_
    MAC_BanTableSetDefaultActionReqDescr_t macBanSetDefaultReq;
    MAC_BanTableAddLinkReqDescr_t   macBanAddLinkReq;
# endif /* _MAC_BAN_TABLE_ */
    MAC_DataReqDescr_t              macDataReq;
    MAC_GetReqDescr_t               macGetReq;
    MAC_ResetReqDescr_t             macResetReq;
    MAC_RxEnableReqDescr_t          macRxenableReq;
    MAC_SetReqDescr_t               macSetReq;
    MAC_StartReqDescr_t             macStartReq;
# if defined(_ZBPRO_) || defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
    MAC_ScanReqDescr_t              macScanReq;
# endif
# if defined(_ZBPRO_) || defined(MAILBOX_UNIT_TEST)
    MAC_PurgeReqDescr_t             macPurgeReq;
    MAC_AssociateReqDescr_t         macAssociateReq;
    MAC_AssociateRespDescr_t        macAssociateResp;
    MAC_OrphanRespDescr_t           macOrphanResp;
# endif
#endif /* _MAILBOX_WRAPPERS_MAC_ */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_NWK_)
# ifdef _RF4CE_
    RF4CE_NWK_ResetReqDescr_t       rNwkResetReq;
    RF4CE_NWK_StartReqDescr_t       rNwkStartReq;
    RF4CE_NWK_DataReqDescr_t        rNwkDataReq;
    RF4CE_NWK_GetReqDescr_t         rNwkGetReq;
    RF4CE_NWK_SetReqDescr_t         rNwkSetReq;
    RF4CE_NWK_DiscoveryReqDescr_t   rNwkDiscReq;
    RF4CE_NWK_AutoDiscoveryReqDescr_t rNwkAutoDiscReq;
    RF4CE_NWK_PairReqDescr_t        rNwkPairReq;
    RF4CE_NWK_UnpairReqDescr_t      rNwkUnpairReq;
    RF4CE_NWK_RXEnableReqDescr_t    rNwkRxEnableReq;
    RF4CE_NWK_UpdateKeyReqDescr_t   rNwkUpdateKeyReq;
    RF4CE_NWK_DiscoveryRespDescr_t  rNwkDiscResp;
    RF4CE_NWK_PairRespDescr_t       rNwkPairResp;
    RF4CE_NWK_UnpairRespDescr_t     rNwkUnpairResp;
    RF4CE_NWK_MacStatsReqDescr_t    rNwkMacStatsReq;
# endif /* _RF4CE_ */
#endif /* _MAILBOX_WRAPPERS_NWK_ */

#ifdef _ZBPRO_
# if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_NWK_)
    ZBPRO_NWK_PermitJoiningReqDescr_t   zNwkPermitJoining;
    ZBPRO_NWK_LeaveReqDescr_t           zNwkLeaveReq;
    // ZBPRO_NWK_GetReqDescr_t          zNwkGetReq;     // reserved
    // ZBPRO_NWK_SetReqDescr_t          zNwkSetReq;     // reserved
    ZBPRO_NWK_GetKeyReqDescr_t          zNwkGetKeyReq;
    ZBPRO_NWK_SetKeyReqDescr_t          zNwkSetKeyReq;
    ZBPRO_NWK_RouteDiscoveryReqDescr_t  zNwkRouteDiscoveryReq;
# endif /* _MAILBOX_WRAPPERS_NWK_ */
#endif /* _ZBPRO_ */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_APS_)
# ifdef _ZBPRO_
    ZBPRO_APS_EndpointRegisterReqDescr_t    zApsEndpointRegisterReq;
    ZBPRO_APS_EndpointUnregisterReqDescr_t  zApsEndpointUnregisterReq;
    ZBPRO_APS_DataReqDescr_t            zApsDataReq;
    ZBPRO_APS_BindUnbindReqDescr_t      zApsBindReq;
    ZBPRO_APS_BindUnbindReqDescr_t      zApsUnbindReq;
    ZBPRO_APS_GetReqDescr_t             zApsGetReq;
    ZBPRO_APS_SetReqDescr_t             zApsSetReq;
    ZBPRO_APS_GetKeyReqDescr_t          zApsGetKeyReq;
    ZBPRO_APS_SetKeyReqDescr_t          zApsSetKeyReq;
    ZBPRO_APS_AddGroupReqDescr_t        zApsAddGroupReq;
    ZBPRO_APS_RemoveGroupReqDescr_t     zApsRemoveGroupReq;
    ZBPRO_APS_RemoveAllGroupsReqDescr_t zApsremoveAllGroupReq;
    ZBPRO_APS_TransportKeyReqDescr_t    zApsTranspostKeyReq;
    ZBPRO_APS_UpdateDeviceReqDescr_t    zApsUpdateKeyReq;
    ZBPRO_APS_RemoveDeviceReqDescr_t    zApsRemoveDeviceReq;
    ZBPRO_APS_RequestKeyReqDescr_t      zApsRequestKeyReq;
    ZBPRO_APS_SwitchKeyReqDescr_t       zApsSwitchKeyReq;
# endif /* _ZBPRO_ */
#endif /* _MAILBOX_WRAPPERS_APS_ */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZDO_)
# ifdef _ZBPRO_
    ZBPRO_ZDO_AddrResolvingReqDescr_t           zZdoAddrResolvingReq;
    ZBPRO_ZDO_NodeDescReqDescr_t                zZdoNodeDescReq;
    ZBPRO_ZDO_PowerDescReqDescr_t               zZdoPowerDescReq;
    ZBPRO_ZDO_SimpleDescReqDescr_t              zZdoSimpleDescReq;
    ZBPRO_ZDO_ActiveEpReqDescr_t                zZdoActiveEpReq;
    ZBPRO_ZDO_MatchDescReqDescr_t               zZdoMatchDescReq;
    ZBPRO_ZDO_DeviceAnnceReqDescr_t             zZdoDeviceAnnceReq;
    ZBPRO_ZDO_ServerDiscoveryReqDescr_t         zZdoServerDiscoveryReq;
    ZBPRO_ZDO_EndDeviceBindReqDescr_t           zZdoEndDeviceBindReq;
    ZBPRO_ZDO_BindUnbindReqDescr_t              zZdoBindUnbindReq;
    ZBPRO_ZDO_StartNetworkReqDescr_t            zZdoStartNetworkReq;
    ZBPRO_ZDO_MgmtLeaveReqDescr_t               zZdoMgmtLeaveReq;
    ZBPRO_ZDO_MgmtPermitJoiningReqDescr_t       zZdoMgmtPermitJoiningReq;
    ZBPRO_ZDO_MgmtNwkUpdateReqDescr_t           zZdoMgmtNwkUpdateReq;
    ZBPRO_ZDO_MgmtNwkUpdateUnsolRespDescr_t     zZdoMgmtNwkUpdateUnsolResp;
    ZBPRO_ZDO_MgmtLqiReqDescr_t                 zZdoMgmtLqiReq;
    ZBPRO_ZDO_MgmtBindReqDescr_t                zZdoMgmtBindReqDescr;
# endif /* _ZBPRO_ */
#endif /* _MAILBOX_WRAPPERS_ZDO_ */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_TC_)
# ifdef _ZBPRO_
    ZBPRO_TC_NwkKeyUpdateReqDescr_t             zTcNwkKeyUpdateReq;
# endif /* _ZBPRO_ */
#endif /* _MAILBOX_WRAPPERS_TC_ */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_)
# ifdef _ZBPRO_
    ZBPRO_ZCL_SetPowerSourceReqDescr_t                      zZclSetPowerSourceReq;
    ZBPRO_ZCL_ProfileWideCmdDiscoverAttrDescr_t             zZclProfileWideCmdDiscoverAttrReq;
    ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t              zZclProfileWideCmdReadAttrReq;
    ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t             zZclProfileWideCmdWriteAttrReq;
    ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t    zZclProfileWideCmdConfigureReportingReq;
    ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqDescr_t  zZclProfileWideCmdReadReportingConfigurationReq;
    ZBPRO_ZCL_IdentifyCmdIdentifyReqDescr_t                 zZclIdentifyCmdIdentifyReq;
    ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqDescr_t            zZclIdentifyCmdIdentifyQueryReq;
#  if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_IDENTIFY_RELAY_)
    ZBPRO_ZCL_IdentifyCmdIdentifyResponseReqDescr_t         zZclIdentifyCmdIdentifyResponseReq;
    ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReqDescr_t    zZclIdentifyCmdIdentifyQueryResponseReq;
#  endif
    ZBPRO_ZCL_GroupsCmdAddGroupReqDescr_t                   zZclGroupsCmdAddGroupReq;
    ZBPRO_ZCL_GroupsCmdViewGroupReqDescr_t                  zZclGroupsCmdViewGroupReq;
    ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqDescr_t         zZclGroupsCmdGetGroupMembershipReq;
    ZBPRO_ZCL_GroupsCmdRemoveGroupReqDescr_t                zZclGroupsCmdRemoveGroupReq;
    ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqDescr_t            zZclGroupsCmdRemoveAllGroupsReq;
    ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqDescr_t         zZclGroupsCmdAddGroupIfIdentifyReq;
    ZBPRO_ZCL_ScenesCmdAddSceneReqDescr_t                   zZclScenesCmdAddSceneReq;
    ZBPRO_ZCL_ScenesCmdViewSceneReqDescr_t                  zZclScenesCmdViewSceneReq;
    ZBPRO_ZCL_ScenesCmdStoreSceneReqDescr_t                 zZclScenesCmdStoreSceneReq;
    ZBPRO_ZCL_ScenesCmdRecallSceneReqDescr_t                zZclScenesCmdRecallSceneReq;
    ZBPRO_ZCL_ScenesCmdRemoveSceneReqDescr_t                zZclScenesCmdRemoveSceneReq;
    ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqDescr_t            zZclScenesCmdRemoveAllScenesReq;
    ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqDescr_t         zZclScenesCmdGetSceneMembershipReq;
    ZBPRO_ZCL_OnOffCmdReqDescr_t                            zZclOnOffCmdReq;
    ZBPRO_ZCL_LevelControlCmdMoveToLevelReqDescr_t          zZclLevelControlCmdMoveToLevelReq;
    ZBPRO_ZCL_LevelControlCmdMoveReqDescr_t                 zZclLevelControlCmdMoveReq;
    ZBPRO_ZCL_LevelControlCmdStepReqDescr_t                 zZclLevelControlCmdStepReq;
    ZBPRO_ZCL_LevelControlCmdStopReqDescr_t                 zZclLevelControlCmdStopReq;
    ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t               zZclDoorLockCmdLockReq;
    ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t               zZclDoorLockCmdUnlockReq;
    ZBPRO_ZCL_WindowCoveringCmdReqDescr_t                   zZclWindowCoveringCmdReq;
    ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t    zZclWindowCoveringLiftTiltPercentCmdReq;
    ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqDescr_t        zZclIASZoneCmdZoneEnrollResponseReq;
    ZBPRO_ZCL_SapIasAceArmRespReqDescr_t                    zZclIasAceArmRespReq;
    ZBPRO_ZCL_SapIasAceBypassRespReqDescr_t                 zZclIasAceBypassRespReq;
    ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqDescr_t           zZclIasAceGetZoneIdMapRespReq;
    ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqDescr_t            zZclIasAceGetZoneInfoRespReq;
    ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqDescr_t         zZclIasAceGetPanelStatusRespReq;
    ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqDescr_t    zZclIasAceSetBypassedZoneListRespReq;
    ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqDescr_t          zZclIasAceGetZoneStatusRespReq;
    ZBPRO_ZCL_SapIasAceZoneStatusChangedReqDescr_t          zZclIasAceZoneStatusChangedReq;
    ZBPRO_ZCL_SapIasAcePanelStatusChangedReqDescr_t         zZclIasAcePanelStatusChangedReq;
    ZBPRO_ZCL_IASWDCmdStartWarningReqDescr_t                zZclIASWDCmdStartWarningReq;
    ZBPRO_ZCL_IASWDCmdSquawkReqDescr_t                      zZclIASWDCmdSquawkReq;
    ZBPRO_ZCL_ColorControlCmdMoveToColorReqDescr_t          zZclColorControlCmdMoveToColorReq;
    ZBPRO_ZCL_ColorControlCmdMoveColorReqDescr_t            zZclColorControlCmdMoveColorReq;
    ZBPRO_ZCL_ColorControlCmdStepColorReqDescr_t            zZclColorControlCmdStepColorReq;
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqDescr_t    zZclColorControlCmdEnhancedMoveToHueReq;
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqDescr_t      zZclColorControlCmdEnhancedMoveHueReq;
    ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqDescr_t      zZclColorControlCmdEnhancedStepHueReq;
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqDescr_t  zZclColorControlCmdEnhancedMoveToHueAndSaturationReq;
    ZBPRO_ZCL_ColorControlCmdColorLoopSetReqDescr_t         zZclColorControlCmdColorLoopSetReq;
    ZBPRO_ZCL_ColorControlCmdStopMoveStepReqDescr_t         zZclColorControlCmdStopMoveStepReq;
    ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqDescr_t     zZclColorControlCmdMoveColorTemperatureReq;
    ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqDescr_t     zZclColorControlCmdStepColorTemperatureReq;
# endif /* _ZBPRO_ */
#endif

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_PROFILE_)
# ifdef _ZBPRO_
#  if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZHA_)
    ZBPRO_ZHA_EzModeReqDescr_t                  zZhaEzModeReq;
    ZBPRO_ZHA_CieEnrollReqDescr_t               zZhaCieEnrollReq;
    ZBPRO_ZHA_CieSetPanelStatusReqDescr_t       zZhaCieSetPanelStatus;
    ZBPRO_ZHA_CieZoneSetBypassStateReqDescr_t   zZhaCieSetZoneBypassState;
#  endif
# endif /* _ZBPRO_ */
# ifdef _RF4CE_
    RF4CE_UnpairReqDescr_t          rProfileUnpairReq;
    RF4CE_StartReqDescr_t           rProfileStartReq;
    RF4CE_ResetReqDescr_t           rProfileResetReq;
    RF4CE_SetSupportedDevicesReqDescr_t rProfileSetSupportedDevicesReq;

#  if (1 == USE_RF4CE_PROFILE_ZRC)
#   if defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
    RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t     rZrcSetWakupActionCodeReq;
    RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t     rZrcGetWakupActionCodeReq;
#   endif
#  ifdef USE_RF4CE_PROFILE_ZRC1
    RF4CE_ZRC1_GetAttributeDescr_t      rZrc1GetReq;
    RF4CE_ZRC1_SetAttributeDescr_t      rZrc1SetReq;
    RF4CE_ZRC1_CommandDiscoveryReqDescr_t rZrc1CommandDiscoveryReq;
    RF4CE_ZRC1_ControlCommandReqDescr_t rZrc1ControlCommandPressedReq;
    RF4CE_ZRC1_ControlCommandReqDescr_t rZrc1ControlCommandReleasedReq;
    RF4CE_ZRC1_BindReqDescr_t           rZrc1ControllerBindReq;
    RF4CE_ZRC1_BindReqDescr_t           rZrc1TargetBindReq;
    RF4CE_ZRC1_VendorSpecificReqDescr_t rZrc1VendorSpecificReq;
#  endif /* USE_RF4CE_PROFILE_ZRC1 */
#  ifdef USE_RF4CE_PROFILE_ZRC2
    RF4CE_ZRC2_GetAttributesReqDescr_t  rZrc2GetReq;
    RF4CE_ZRC2_SetAttributesReqDescr_t  rZrc2SetReq;
    RF4CE_ZRC2_KeyExchangeReqDescr_t    rZcr2KeyExchangeReq;
    RF4CE_ZRC2_BindReqDescr_t           rZrc2BindReq;
    RF4CE_ZRC2_ProxyBindReqDescr_t      rZrc2ProxyBindReq;
    RF4CE_ZRC2_BindingReqDescr_t        rZrc2EnableBindReq;
    RF4CE_ZRC2_BindingReqDescr_t        rZrc2DisableBindReq;
    RF4CE_ZRC2_ButtonBindingReqDescr_t          rZrc2SetPushButtonStimulusReq;
    RF4CE_ZRC2_ButtonBindingReqDescr_t          rZrc2ClearPushButtonStimulusReq;
    RF4CE_ZRC2_CheckValidationRespDescr_t rZrc2CheckValidationReq;
    RF4CE_ZRC2_ControlCommandReqDescr_t rZrc2ControlCommandReq;
    RF4CE_GDP2_SetPollConstraintsReqDescr_t     rGdp2SetPollConstraintsReq;
    RF4CE_GDP2_PollNegotiationReqDescr_t        rGdp2PollNegotiationReq;
    RF4CE_GDP2_PollClientUserEventReqDescr_t    rGdp2PollClientUserEventReq;
    RF4CE_GDP2_ClientNotificationReqDescr_t     rGdp2ClientNotificationReq;
    RF4CE_GDP2_IdentifyCapAnnounceReqDescr_t    rGdp2IdentifyCapAnnounceReq;
    RF4CE_GDP2_IdentifyReqDescr_t               rGdp2IdentifyReq;
    RF4CE_ZRC2_GetSharedSecretIndDescr_t        rZrc2GetSharedSecretInd;
#  endif /* USE_RF4CE_PROFILE_ZRC2 */
#  endif /* USE_RF4CE_PROFILE_ZRC */
#  if (1 == USE_RF4CE_PROFILE_MSO)
    RF4CE_MSO_GetProfileAttributeReqDescr_t rMsoGetProfileAttributeReq;
    RF4CE_MSO_SetProfileAttributeReqDescr_t rMsoSetProfileAttributeReq;
    RF4CE_MSO_GetRIBAttributeReqDescr_t rMsoGetRIBAttributeReq;
    RF4CE_MSO_SetRIBAttributeReqDescr_t rMsoSetRIBAttributeReq;
    RF4CE_MSO_BindReqDescr_t rMsoBindReq;
    RF4CE_MSO_UserControlReqDescr_t rMsoUserControlReq;
    RF4CE_MSO_WatchDogKickOrValidateReqDescr_t rMsoValidateReq;
#  endif /* (1 == USE_RF4CE_PROFILE_MSO) */
# endif /* _RF4CE_ */
#endif /* _MAILBOX_WRAPPERS_PROFILE_ */

    NVM_ReadFileIndDescr_t       rNvmReadFileReq;
    NVM_OpenFileIndDescr_t       rNvmOpenFileReq;
    NVM_WriteFileIndDescr_t      rNvmWriteFileReq;
    NVM_CloseFileIndDescr_t      rNvmCloseFileReq;

#if defined(_PHY_TEST_HOST_INTERFACE_)
    Phy_Test_Get_Caps_ReqDescr_t   rPhyTestGetCapsReq;
    Phy_Test_Set_Channel_ReqDescr_t    rPhyTestSetChannelReq;
    Phy_Test_Continuous_Wave_Start_ReqDescr_t  rPhyTestContinuousWaveStartReq;
    Phy_Test_Continuous_Wave_Stop_ReqDescr_t   rPhyTestContinuousWaveStopReq;
    Phy_Test_Transmit_Start_ReqDescr_t rPhyTestTransmitStartReq;
    Phy_Test_Transmit_Stop_ReqDescr_t  rPhyTestTransmitStopReq;
    Phy_Test_Receive_Start_ReqDescr_t  rPhyTestReceiveStartReq;
    Phy_Test_Receive_Stop_ReqDescr_t   rPhyTestReceiveStopReq;
    Phy_Test_Echo_Start_ReqDescr_t     rPhyTestEchoStartReq;
    Phy_Test_Echo_Stop_ReqDescr_t      rPhyTestEchoStopReq;
    Phy_Test_Energy_Detect_Scan_ReqDescr_t rPhyTestEnergyDetectScanReq;
    Phy_Test_Get_Stats_ReqDescr_t      rPhyTestGetStatsReq;
    Phy_Test_Reset_Stats_ReqDescr_t    rPhyTestResetStatsReq;
    Phy_Test_Set_TX_Power_ReqDescr_t   rPhyTestSetTXPowerReq;
    Phy_Test_Select_Antenna_ReqDescr_t rPhyTestSelectAntennaReq;
    RF4CE_Diag_Caps_ReqDescr_t         rRF4CEDiagCapsReq;
    RF4CE_Diag_ReqDescr_t              rRF4CEDiagReq;
#endif
} MailReq_t;


/**//**
 * \brief Union of all request parameters.
 */
typedef union _MailReqParams_t
{
#ifdef MAILBOX_UNIT_TEST
    MailUnitTest_f1ReqParams_t      mailF1;
    MailUnitTest_f3ReqParams_t      mailF3;
    MailUnitTest_f4ReqParams_t      mailF4;
    MailUnitTest_f5ReqParams_t      mailF5;
#endif /* MAILBOX_UNIT_TEST */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_TEST_ENGINE_)
    TE_EchoCommandReqParams_t       teEcho;
    TE_ResetCommandReqParams_t      teReset;
#endif /* _MAILBOX_WRAPPERS_TEST_ENGINE_ */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_PROFILING_ENGINE_)
    SYS_DbgPeResetReqParams_t       peReset;
    SYS_DbgPeGetDataReqParams_t     peGetData;
#endif /* _MAILBOX_WRAPPERS_PROFILING_ENGINE_ */

    // TODO: fill me as .*ReqParams_t;$
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_MAC_)
# ifdef _MAC_BAN_TABLE_
    MAC_BanTableSetDefaultActionReqParams_t macBanSetActionReq;
    MAC_BanTableAddLinkReqParams_t  macBanAddLinkReq;
# endif /* _MAC_BAN_TABLE_ */
    MAC_DataReqParams_t             macDataReq;
    MAC_GetReqParams_t              macGetReq;
    MAC_ResetReqParams_t            macResetReq;
    MAC_RxEnableReqParams_t         macRxEnableReq;
    MAC_SetReqParams_t              macSetReq;
    MAC_StartReqParams_t            macStartReq;
# if defined(_ZBPRO_) || defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
    MAC_ScanReqParams_t             macScanReq;
# endif
# if defined(_ZBPRO_) || defined(MAILBOX_UNIT_TEST)
    MAC_PurgeReqParams_t            macPurgeReq;
    MAC_AssociateReqParams_t        macAssociateReq;
    MAC_AssociateRespParams_t       macAssociateResp;
    MAC_OrphanRespParams_t          macOrphanResp;
# endif
#endif /* _MAILBOX_WRAPPERS_MAC_ */

#ifdef _RF4CE_
# if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_NWK_)
    RF4CE_NWK_ResetReqParams_t      rNwkReset;
    RF4CE_NWK_DataReqParams_t       rNwkData;
    RF4CE_NWK_GetReqParams_t        rNwkGet;
    RF4CE_NWK_SetReqParams_t        rNwkSet;
    RF4CE_NWK_DiscoveryReqParams_t  rNwkDiscReq;
    RF4CE_NWK_AutoDiscoveryReqParams_t rNwkAutoDisc;
    RF4CE_NWK_PairReqParams_t       rNwkPairReq;
    RF4CE_NWK_UnpairReqParams_t     rNwkUnpairReq;
    RF4CE_NWK_RXEnableReqParams_t   rNwkRxEnable;
    RF4CE_NWK_UpdateKeyReqParams_t  rNwkUpdateKey;
    RF4CE_NWK_DiscoveryRespParams_t rNwkDiscResp;
    RF4CE_NWK_PairRespParams_t      rNwkPairResp;
    RF4CE_NWK_UnpairRespParams_t    rNwkUnpairResp;
    RF4CE_NWK_MacStatsReqParams_t   rNwkMacStatsReq;
# endif /* _MAILBOX_WRAPPERS_NWK_ */
#endif

#ifdef _ZBPRO_
# if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_NWK_)
    ZBPRO_NWK_PermitJoiningReqParams_t  zNwkPermitJoining;
    ZBPRO_NWK_LeaveReqParams_t          zNwkLeaveReqParams;
    // ZBPRO_NWK_GetReqParams_t            zNwkGetReqParams;   //reserved
    // ZBPRO_NWK_SetReqParams_t            zNwkSetReqParams;   //reserved
    ZBPRO_NWK_GetKeyReqParams_t         zNwkGetKeyReqParams;
    ZBPRO_NWK_SetKeyReqParams_t         zNwkSetKeyReqParams;
    ZBPRO_NWK_RouteDiscoveryReqParams_t zNwkRouteDiscoveryParams;
# endif /* _MAILBOX_WRAPPERS_NWK_ */
#endif /* _ZBPRO_ */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_APS_)
# ifdef _ZBPRO_
    ZBPRO_APS_EndpointRegisterReqParams_t   zApsEndpointRegister;
    ZBPRO_APS_EndpointUnregisterReqParams_t zApsEndpointUnregister;
    ZBPRO_APS_DataReqParams_t           zApsData;
    ZBPRO_APS_BindUnbindReqParams_t     zApsBind;
    ZBPRO_APS_BindUnbindReqParams_t     zApsUnbind;
    ZBPRO_APS_GetReqParams_t            zApsGet;
    ZBPRO_APS_SetReqParams_t            zApsSet;
    ZBPRO_APS_GetKeyReqParams_t         zApsGetKey;
    ZBPRO_APS_SetKeyReqParams_t         zApsSetKey;
    ZBPRO_APS_AddGroupReqParams_t       zApsAddGroup;
    ZBPRO_APS_RemoveGroupReqParams_t    zApsRemoveGroup;
    ZBPRO_APS_RemoveAllGroupsReqParams_t zApsRemoveAllGroup;
    ZBPRO_APS_TransportKeyReqParams_t   zApsTransportKey;
    ZBPRO_APS_UpdateDeviceReqParams_t   zApsUpdateDevice;
    ZBPRO_APS_RemoveDeviceReqParams_t   zApsRemoveDevice;
    ZBPRO_APS_RequestKeyReqParams_t     zApsRequestKey;
    ZBPRO_APS_SwitchKeyReqParams_t      zApsSwitchKey;
# endif /* _ZBPRO_ */
#endif /* _MAILBOX_WRAPPERS_APS_ */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZDO_)
# ifdef _ZBPRO_
    ZBPRO_ZDO_AddrResolvingReqParams_t          zZdoAddrResolving;
    ZBPRO_ZDO_NodeDescReqParams_t               zZdoNodeDesc;
    ZBPRO_ZDO_PowerDescReqParams_t              zZdoPowerDesc;
    ZBPRO_ZDO_SimpleDescReqParams_t             zZdoSimpleDesc;
    ZBPRO_ZDO_ActiveEpReqParams_t               zZdoActiveEp;
    ZBPRO_ZDO_MatchDescReqParams_t              zZdoMatchDesc;
    ZBPRO_ZDO_DeviceAnnceReqParams_t            zZdoDeviceAnnce;
    ZBPRO_ZDO_ServerDiscoveryReqParams_t        zZdoServerDiscovery;
    ZBPRO_ZDO_EndDeviceBindReqParams_t          zZdoEndDeviceBind;
    ZBPRO_ZDO_BindUnbindReqParams_t             zZdoBindUnbind;
    ZBPRO_ZDO_MgmtLeaveReqParams_t              zZdoMgmtLeave;
    ZBPRO_ZDO_MgmtPermitJoiningReqParams_t      zZdoMgmtPermitJoining;
    ZBPRO_ZDO_MgmtNwkUpdateReqParams_t          zZdoMgmtNwkUpdate;
    ZBPRO_ZDO_MgmtNwkUpdateUnsolRespParams_t    zZdoMgmtNwkUpdateUnsol;
    ZBPRO_ZDO_MgmtLqiReqParams_t                zZdoMgmtLqiReqParams;
    ZBPRO_ZDO_MgmtBindReqParams_t               zZdoMgmtBindReqParams;
# endif /* _ZBPRO_ */
#endif /* _MAILBOX_WRAPPERS_ZDO_ */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_TC_)
# ifdef _ZBPRO_
    ZBPRO_TC_NwkKeyUpdateReqParams_t            zTcNwkKeyUpdate;
# endif /* _ZBPRO_ */
#endif /* _MAILBOX_WRAPPERS_TC_ */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_)
# ifdef _ZBPRO_
    ZBPRO_ZCL_SetPowerSourceReqParams_t                     zZclSetPowerSourceReq;
    ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReqParams_t         zZclProfileWideCmdDiscoverAttrReq;
    ZBPRO_ZCL_ProfileWideCmdReadAttrReqParams_t             zZclProfileWideCmdReadAttrReq;
    ZBPRO_ZCL_ProfileWideCmdWriteAttrReqParams_t            zZclProfileWideCmdWriteAttrReq;
    ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqParams_t   zZclProfileWideCmdConfigureReportingReq;
    ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqParams_t  zZclProfileWideCmdReadReportingConfigurationReq;
    ZBPRO_ZCL_IdentifyCmdIdentifyReqParams_t                zZclIdentifyCmdIdentifyReq;
    ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqParams_t           zZclIdentifyCmdIdentifyQueryReq;
#  if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_IDENTIFY_RELAY_)
    ZBPRO_ZCL_IdentifyCmdIdentifyResponseReqParams_t        zZclIdentifyCmdIdentifyResponseReq;
    ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReqParams_t   zZclIdentifyCmdIdentifyQueryResponseReq;
#  endif
    ZBPRO_ZCL_GroupsCmdAddGroupReqParams_t                  zZclGroupsCmdAddGroupReq;
    ZBPRO_ZCL_GroupsCmdViewGroupReqParams_t                 zZclGroupsCmdViewGroupReq;
    ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqParams_t        zZclGroupsCmdGetGroupMembershipReq;
    ZBPRO_ZCL_GroupsCmdRemoveGroupReqParams_t               zZclGroupsCmdRemoveGroupReq;
    ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqParams_t           zZclGroupsCmdRemoveAllGroupsReq;
    ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqParams_t        zZclGroupsCmdAddGroupIfIdentifyReq;
    ZBPRO_ZCL_ScenesCmdAddSceneReqParams_t                  zZclScenesCmdAddSceneReq;
    ZBPRO_ZCL_ScenesCmdViewSceneReqParams_t                 zZclScenesCmdViewSceneReq;
    ZBPRO_ZCL_ScenesCmdStoreSceneReqParams_t                zZclScenesCmdStoreSceneReq;
    ZBPRO_ZCL_ScenesCmdRemoveSceneReqParams_t               zZclScenesCmdRemoveSceneReq;
    ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqParams_t           zZclScenesCmdRemoveAllScenesReq;
    ZBPRO_ZCL_ScenesCmdRecallSceneReqParams_t               zZclScenesCmdRecallSceneReq;
    ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqParams_t        zZclScenesCmdGetSceneMembershipReq;
    ZBPRO_ZCL_OnOffCmdReqParams_t                           zZclOnOffCmdReq;
    ZBPRO_ZCL_LevelControlCmdMoveToLevelReqParams_t         zZclLevelControlCmdMoveToLevelReq;
    ZBPRO_ZCL_LevelControlCmdMoveReqParams_t                zZclLevelControlCmdMoveReq;
    ZBPRO_ZCL_LevelControlCmdStepReqParams_t                zZclLevelControlCmdStepReq;
    ZBPRO_ZCL_LevelControlCmdStopReqParams_t                zZclLevelControlCmdStopReq;
    ZBPRO_ZCL_WindowCoveringCmdReqParams_t                  zZclWindowCoveringCmdReq;
    ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqParams_t   zZclWindowCoveringLiftTiltPercentCmdReq;
    ZBPRO_ZCL_DoorLockCmdLockUnlockReqParams_t              zZclDoorLockCmdLockReq;
    ZBPRO_ZCL_DoorLockCmdLockUnlockReqParams_t              zZclDoorLockCmdUnlockReq;
    ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqParams_t       zZclIASZoneCmdZoneEnrollResponseReq;
    ZBPRO_ZCL_SapIasAceArmRespReqParams_t                   zZclIasAceArmRespReqParams;
    ZBPRO_ZCL_SapIasAceBypassRespReqParams_t                zZclIasAceBypassRespReqParams;
    ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqParams_t          zZclIasAceGetZoneIdMapRespReqParams;
    ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqParams_t           zZclIasAceGetZoneInfoRespReqParams;
    ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqParams_t        zZclIasAceGetPanelStatusRespReqParams;
    ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqParams_t   zZclIasAceSetBypassedZoneListRespReqParams;
    ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqParams_t         zZclIasAceGetZoneStatusRespReqParams;
    ZBPRO_ZCL_SapIasAceZoneStatusChangedReqParams_t         zZclIasAceZoneStatusChangedReqParams;
    ZBPRO_ZCL_SapIasAcePanelStatusChangedReqParams_t        zZclIasAcePanelStatusChangedReqParams;
    ZBPRO_ZCL_IASWDCmdStartWarningReqParams_t               zZclIASWDCmdStartWarningReq;
    ZBPRO_ZCL_IASWDCmdSquawkReqParams_t                     zZclIASWDCmdSquawkReq;
    ZBPRO_ZCL_ColorControlCmdMoveToColorReqParams_t         zZclColorControlCmdMoveToColorReq;
    ZBPRO_ZCL_ColorControlCmdMoveColorReqParams_t           zZclColorControlCmdMoveColorReq;
    ZBPRO_ZCL_ColorControlCmdStepColorReqParams_t           zZclColorControlCmdStepColorReq;
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqParams_t   zZclColorControlCmdEnhancedMoveToHueReq;
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqParams_t     zZclColorControlCmdEnhancedMoveHueReq;
    ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqParams_t     zZclColorControlCmdEnhancedStepHueReq;
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqParams_t  zZclColorControlCmdEnhancedMoveToHueAndSaturationReq;
    ZBPRO_ZCL_ColorControlCmdColorLoopSetReqParams_t           zZclColorControlCmdColorLoopSetReq;
    ZBPRO_ZCL_ColorControlCmdStopMoveStepReqParams_t           zZclColorControlCmdStopMoveStepReq;
    ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqParams_t   zZclColorControlCmdMoveColorTemperatureReq;
    ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqParams_t   zZclColorControlCmdStepColorTemperatureReq;
# endif /* _ZBPRO_ */
#endif

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_PROFILE_)
# ifdef _ZBPRO_
#  if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZHA_)
    ZBPRO_ZHA_EzModeReqParams_t                 zZhaEzMode;
    ZBPRO_ZHA_CieEnrollReqParams_t              zZhaCieEnroll;
    ZBPRO_ZHA_CieSetPanelStatusReqParams_t      zZhaCieSetPanelStatus;
    ZBPRO_ZHA_CieZoneSetBypassStateReqParams_t  zZhaCieZoneSetBypassState;
#  endif
# endif /* _ZBPRO_ */
# ifdef _RF4CE_
    RF4CE_UnpairReqParams_t                     rProfileUnpair;
    RF4CE_ResetReqParams_t          rProfileReset;
    RF4CE_SetSupportedDevicesReqParams_t rProfileSetSupportedDevices;

#  if (1 == USE_RF4CE_PROFILE_ZRC)
#  ifdef USE_RF4CE_PROFILE_ZRC1
    RF4CE_ZRC1_GetAttributeReqParams_t      rZrc1GetReq;
    RF4CE_ZRC1_SetAttributeReqParams_t      rZrc1SetReq;
    RF4CE_ZRC1_CommandDiscoveryReqParams_t rZrc1CommandDiscoveryReq;
    RF4CE_ZRC1_ControlCommandReqParams_t rZrc1ControlCommandPressedReq;
    RF4CE_ZRC1_ControlCommandReqParams_t rZrc1ControlCommandReleasedReq;
    RF4CE_ZRC1_VendorSpecificReqParams_t rZrc1VendorSpecificReq;
#  endif /* USE_RF4CE_PROFILE_ZRC1 */
#   ifdef USE_RF4CE_PROFILE_ZRC2
    RF4CE_ZRC2_GetAttributesReqParams_t    rZrc2GetReq;
    RF4CE_ZRC2_SetAttributesReqParams_t    rZrc2SetReq;
    RF4CE_ZRC2_KeyExchangeReqParams_t      rZrc2KeyExchangeReq;
    RF4CE_ZRC2_ProxyBindReqParams_t        rZrc2ProxyBindReq;
    RF4CE_ZRC2_ButtonBindingReqParams_t         rZrc2SetPushButtonStimulusReq;
    RF4CE_ZRC2_ButtonBindingReqParams_t         rZrc2ClearPushButtonStimulusReq;
    RF4CE_ZRC2_ControlCommandReqParams_t   rZrc2ControlCommandReqParams;
    RF4CE_GDP2_SetPollConstraintsReqParams_t    rGdp2SetPollConstraintsReq;
    RF4CE_GDP2_PollNegotiationReqParams_t       rGdp2PollNegotiationReq;
    RF4CE_GDP2_PollClientUserEventReqParams_t   rGdp2PollClientUserEventReq;
    RF4CE_GDP2_ClientNotificationReqParams_t    rGdp2ClientNotificationReq;
    RF4CE_GDP2_IdentifyCapAnnounceReqParams_t   rGdp2IdentifyCapAnnounceReq;
    RF4CE_GDP2_IdentifyReqParams_t              rGdp2IdentifyReq;
    RF4CE_ZRC2_GetSharedSecretIndParams_t       rZrc2GetSharedSecretInd;
#   endif /* USE_RF4CE_PROFILE_ZRC2 */
#  endif /* USE_RF4CE_PROFILE_ZRC */
#  if (1 == USE_RF4CE_PROFILE_MSO)
    RF4CE_MSO_GetProfileAttributeReqParams_t rMsoGetProfileAttributeReqParams;
    RF4CE_MSO_SetProfileAttributeReqParams_t rMsoSetProfileAttributeReqParams;
    RF4CE_MSO_GetRIBAttributeReqParams_t rMsoGetRIBAttributeReqParams;
    RF4CE_MSO_SetRIBAttributeReqParams_t rMsoSetRIBAttributeReqParams;
    RF4CE_MSO_UserControlReqParams_t rMsoUserControlReqParams;
    RF4CE_MSO_WatchDogKickOrValidateReqParams_t rMsoValidateReqParams;
#  endif /* (1 == USE_RF4CE_PROFILE_MSO) */
# endif /* _RF4CE_ */
#endif

    NVM_ReadFileIndParams_t     rNvmReadFileReq;
    NVM_OpenFileIndParams_t     rNvmOpenFileReq;
    NVM_WriteFileIndParams_t    rNvmWriteFileReq;
    NVM_CloseFileIndParams_t    rNvmCloseFileReq;

#if defined(_PHY_TEST_HOST_INTERFACE_)
    Phy_Test_Set_Channel_ReqParams_t    rPhyTestSetChannelReqParams;
    Phy_Test_Continuous_Wave_Start_ReqParams_t  rPhyTestContinuousWaveStartReqParams;
    Phy_Test_Transmit_Start_ReqParams_t rPhyTestTransmitStartReqParams;
    Phy_Test_Receive_Start_ReqParams_t  rPhyTestReceiveStart_ReqParams;
    Phy_Test_Energy_Detect_Scan_ReqParams_t rPhyTestEnergyDetectScanReqParams;
    Phy_Test_Set_TX_Power_ReqParams_t   rPhyTestSetTXPowerReqParams;
    Phy_Test_Select_Antenna_ReqParams_t rPhyTestSelectAntennaReqParams;
    RF4CE_Diag_ReqParams_t              rRF4CEDiagReqParams;
#endif
} MailReqParams_t;


/**//**
 * \brief Union of all confirm parameters.
 */
typedef union _MailConfParams_t
{
#ifdef MAILBOX_UNIT_TEST
    MailUnitTest_f1ConfParams_t     mailF1;
    MailUnitTest_f2ConfParams_t     mailF2;
    MailUnitTest_f4ConfParams_t     mailF4;
    MailUnitTest_f5ConfParams_t     mailF5;
#endif /* MAILBOX_UNIT_TEST */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_TEST_ENGINE_)
    TE_PingCommandConfParams_t      tePing;
    TE_EchoCommandConfParams_t      teEcho;
#endif /* _MAILBOX_WRAPPERS_TEST_ENGINE_ */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_PROFILING_ENGINE_)
    SYS_DbgPeGetDataConfParams_t    peGetData;
#endif /* _MAILBOX_WRAPPERS_PROFILING_ENGINE_ */

    // TODO: fill me as .*ConfParams_t;$
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_MAC_)
# ifdef _MAC_BAN_TABLE_
    MAC_BanTableConfParams_t        macBanSetActionConf;
    MAC_BanTableConfParams_t        macBanAddLinkConf;
# endif /* _MAC_BAN_TABLE_ */
    MAC_DataConfParams_t            macDataConf;
    MAC_GetConfParams_t             macGetConf;
    MAC_ResetConfParams_t           macResetConf;
    MAC_RxEnableConfParams_t        macRxEnableConf;
    MAC_SetConfParams_t             macSetConf;
    MAC_StartConfParams_t           macStartConf;
# if defined(_ZBPRO_) || defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
    MAC_ScanConfParams_t            macScanConf;
# endif
# if defined(_ZBPRO_) || defined(MAILBOX_UNIT_TEST)
    MAC_PurgeConfParams_t           macPurgeConf;
    MAC_AssociateConfParams_t       macAssociateConf;
    MAC_CommStatusAssociateIndParams_t macCommStatusAssociateInd;
    MAC_CommStatusOrphanIndParams_t macCommStatusOrphanInd;
# endif
#endif /* _MAILBOX_WRAPPERS_MAC_ */

#ifdef _RF4CE_
# if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_NWK_)
    RF4CE_NWK_ResetConfParams_t     rNwkReset;
    RF4CE_NWK_StartConfParams_t     rNwkStart;
    RF4CE_NWK_DataConfParams_t      rNwkData;
    RF4CE_NWK_GetConfParams_t       rNwkGet;
    RF4CE_NWK_SetConfParams_t       rNwkSet;
    RF4CE_NWK_DiscoveryConfParams_t rNwkDisc;
    RF4CE_NWK_AutoDiscoveryConfParams_t rNwkAutoDisc;
    RF4CE_NWK_PairConfParams_t      rNwkPair;
    RF4CE_NWK_UnpairConfParams_t    rNwkUnpair;
    RF4CE_NWK_RXEnableConfParams_t  rNwkRxEnable;
    RF4CE_NWK_UpdateKeyConfParams_t rNwkUpdateKey;
    RF4CE_NWK_MacStatsConfParams_t  rNwkMacStats;
# endif /* _MAILBOX_WRAPPERS_NWK_ */
#endif


#ifdef _ZBPRO_
# if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_NWK_)
    ZBPRO_NWK_PermitJoiningConfParams_t     zNwkPermitJoining;
    ZBPRO_NWK_LeaveConfParams_t             zNwkLeaveConfParams_t;
# endif /* _MAILBOX_WRAPPERS_NWK_ */
#endif /* _ZBPRO_ */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_APS_)
# ifdef _ZBPRO_
    ZBPRO_APS_EndpointRegisterConfParams_t  zApsEndpointRegister;
    ZBPRO_APS_DataConfParams_t              zApsData;
    ZBPRO_APS_BindUnbindConfParams_t        zApsBind;
    ZBPRO_APS_BindUnbindConfParams_t        zApsUnbind;
    ZBPRO_APS_GetConfParams_t               zApsGet;
    ZBPRO_APS_SetConfParams_t               zApsSet;
    ZBPRO_APS_AddGroupConfParams_t          zApsAddGroup;
    ZBPRO_APS_RemoveGroupConfParams_t       zApsRemoveGroup;
    ZBPRO_APS_RemoveAllGroupsConfParams_t   zApsRemoveAllGroup;
    ZBPRO_APS_SecurityServicesConfParams_t  zApsTransportKey;
    ZBPRO_APS_SecurityServicesConfParams_t  zApsUpdateDevice;
    ZBPRO_APS_SecurityServicesConfParams_t  zApsRemoveDevice;
    ZBPRO_APS_SecurityServicesConfParams_t  zApsRequestKey;
    ZBPRO_APS_SecurityServicesConfParams_t  zApsSwitchKey;
# endif /* _ZBPRO_ */
#endif /* _MAILBOX_WRAPPERS_APS_ */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZDO_)
# ifdef _ZBPRO_
    ZBPRO_ZDO_AddrResolvingConfParams_t         zZdoAddrResolving;
    ZBPRO_ZDO_NodeDescConfParams_t              zZdoNodeDesc;
    ZBPRO_ZDO_PowerDescConfParams_t             zZdoPowerDesc;
    ZBPRO_ZDO_SimpleDescConfParams_t            zZdoSimpleDesc;
    ZBPRO_ZDO_ActiveEpConfParams_t              zZdoActiveEp;
    ZBPRO_ZDO_MatchDescConfParams_t             zZdoMatchDesc;
    ZBPRO_ZDO_DeviceAnnceConfParams_t           zZdoDeviceAnnce;
    ZBPRO_ZDO_ServerDiscoveryConfParams_t       zZdoServerDiscovery;
    ZBPRO_ZDO_BindConfParams_t                  zZdoBindUnbind;
    ZBPRO_ZDO_StartNetworkConfParams_t          zZdoStartNetwork;
    ZBPRO_ZDO_MgmtLeaveConfParams_t             zZdoMgmtLeave;
    ZBPRO_ZDO_MgmtPermitJoiningConfParams_t     zZdoMgmtPermitJoining;
    ZBPRO_ZDO_MgmtNwkUpdateConfParams_t         zZdoMgmtNwkUpdate;
    ZBPRO_ZDO_MgmtNwkUpdateUnsolConfParams_t    zZdoMgmtNwkUpdateUnsol;
    ZBPRO_ZDO_MgmtLqiConfParams_t               zZdoMgmtLqiConfParams;
    ZBPRO_ZDO_MgmtBindConfParams_t              zZdoMgmtBindConfParams;
# endif /* _ZBPRO_ */
#endif /* _MAILBOX_WRAPPERS_ZDO_ */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_TC_)
# ifdef _ZBPRO_
    ZBPRO_TC_NwkKeyUpdateConfParams_t       zTcNwkKeyUpdate;
# endif /* _ZBPRO_ */
#endif /* _MAILBOX_WRAPPERS_TC_ */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_)
# ifdef _ZBPRO_
    ZBPRO_ZCL_SetPowerSourceConfParams_t                        zZclSetPowerSourceConf;
    ZBPRO_ZCL_ProfileWideCmdDiscoverAttrConfParams_t            zZclProfileWideCmdDiscoverAttrConf;
    ZBPRO_ZCL_ProfileWideCmdReadAttrConfParams_t                zZclProfileWideCmdReadAttrConf;
    ZBPRO_ZCL_ProfileWideCmdWriteAttrConfParams_t               zZclProfileWideCmdWriteAttrConf;
    ZBPRO_ZCL_ProfileWideCmdConfigureReportingConfParams_t  zZclProfileWideCmdConfigureReportingConf;
    ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationConfParams_t  zZclProfileWideCmdReadReportingConfigurationConf;
    ZBPRO_ZCL_IdentifyCmdConfParams_t                           zZclIdentifyCmdConf;
    ZBPRO_ZCL_GroupsCmdAddGroupConfParams_t                     zZclGroupsCmdAddGroupConf;
    ZBPRO_ZCL_GroupsCmdViewGroupConfParams_t                    zZclGroupsCmdViewGroupConf;
    ZBPRO_ZCL_GroupsCmdRemoveGroupConfParams_t                  zZclGroupsCmdRemoveGroupConf;
    ZBPRO_ZCL_ScenesCmdConfParams_t                             zZclScenesCmdConf;
    ZBPRO_ZCL_ScenesCmdAddSceneConfParams_t                     zZclScenesAddSceneCmdConf;
    ZBPRO_ZCL_ScenesCmdViewSceneConfParams_t                    zZclScenesViewSceneCmdConf;
    ZBPRO_ZCL_ScenesCmdStoreSceneConfParams_t                   zZclScenesStoreSceneCmdConf;
    ZBPRO_ZCL_ScenesCmdRemoveSceneConfParams_t                  zZclScenesRemoveSceneCmdConf;
    ZBPRO_ZCL_ScenesCmdRemoveAllScenesConfParams_t              zZclScenesRemoveAllScenesCmdConf;
    ZBPRO_ZCL_OnOffCmdConfParams_t                              zZclOnOffCmdConf;
    ZBPRO_ZCL_LevelControlCmdConfParams_t                       zZclLevelControlCmdConf;
    ZBPRO_ZCL_DoorLockCmdLockUnlockConfParams_t                 zZclDoorLockCmdLockConf;
    ZBPRO_ZCL_DoorLockCmdLockUnlockConfParams_t                 zZclDoorLockCmdUnlockConf;
    ZBPRO_ZCL_WindowCoveringCmdConfParams_t                     zZclWindowCoveringCmdConf;
    ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseConfParams_t          zZclIASZoneCmdEnrollResponseConf;
    ZBPRO_ZCL_SapIasAceRespReqConfParams_t                      zZclIasAceRespReqConfParams;
    ZBPRO_ZCL_IASWDCmdStartWarningConfParams_t                  zZclIASWDCmdStartWarningConf;
    ZBPRO_ZCL_IASWDCmdSquawkConfParams_t                        zZclIASWDCmdSquawkConf;
    ZBPRO_ZCL_ColorControlCmdMoveToColorConfParams_t    zZclColorControlCmdMoveToColorConf;
    ZBPRO_ZCL_ColorControlCmdMoveColorConfParams_t      zZclColorControlCmdMoveColorConf;
    ZBPRO_ZCL_ColorControlCmdStepColorConfParams_t      zZclColorControlCmdStepColorConf;
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueConfParams_t    zZclColorControlCmdEnhancedMoveToHueConf;
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueConfParams_t      zZclColorControlCmdEnhancedMoveHueConf;
    ZBPRO_ZCL_ColorControlCmdEnhancedStepHueConfParams_t      zZclColorControlCmdEnhancedStepHueConf;
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationConfParams_t  zZclColorControlCmdEnhancedMoveToHueAndSaturationConf;
    ZBPRO_ZCL_ColorControlCmdColorLoopSetConfParams_t         zZclColorControlCmdColorLoopSetConf;
    ZBPRO_ZCL_ColorControlCmdStopMoveStepConfParams_t         zZclColorControlCmdStopMoveStepConf;
    ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureConfParams_t     zZclColorControlCmdMoveColorTemperatureConf;
    ZBPRO_ZCL_ColorControlCmdStepColorTemperatureConfParams_t     zZclColorControlCmdStepColorTemperatureConf;
# endif /* _ZBPRO_ */
#endif

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_PROFILE_)
# ifdef _ZBPRO_
#  if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZHA_)
    ZBPRO_ZHA_EzModeConfParams_t                zZhaEzMode;
    ZBPRO_ZHA_CieEnrollConfParams_t             zZhaCieEnroll;
    ZBPRO_ZHA_CieSetPanelStatusConfParams_t     zZhaCieSetPanelStatus;
    ZBPRO_ZHA_CieZoneSetBypassStateConfParams_t zZhaCieSetBypassState;
#  endif
# endif /* _ZBPRO_ */

# ifdef _RF4CE_
    RF4CE_UnpairConfParams_t                    rProfileUnpair;
    RF4CE_StartResetConfParams_t                rProfileStart;
    RF4CE_StartResetConfParams_t                rProfileReset;
    RF4CE_SetSupportedDevicesConfParams_t       rProfileSetSupportedDeviceReq;

#  if (1 == USE_RF4CE_PROFILE_ZRC)
#   ifdef USE_RF4CE_PROFILE_ZRC1
    RF4CE_ZRC1_GetAttributeConfParams_t      rZrc1GetReq;
    RF4CE_ZRC1_SetAttributeConfParams_t      rZrc1SetReq;
    RF4CE_ZRC1_CommandDiscoveryConfParams_t rZrc1CommandDiscoveryReq;
    RF4CE_ZRC_ControlCommandConfParams_t    rZrc1ControlCommandPressedReq;
    RF4CE_ZRC_ControlCommandConfParams_t    rZrc1ControlCommandReleasedReq;
    RF4CE_ZRC1_BindConfParams_t             rZrc1ControllerBindReq;
    RF4CE_ZRC1_BindConfParams_t             rZrc1TargetBindReq;
    RF4CE_ZRC1_VendorSpecificConfParams_t   rZrc1VendorSpecificReq;
#  endif /* USE_RF4CE_PROFILE_ZRC1 */
#   ifdef USE_RF4CE_PROFILE_ZRC2
    RF4CE_ZRC2_GetAttributesConfParams_t    rZrc2GetReq;
    RF4CE_ZRC2_SetAttributesConfParams_t    rZrc2SetReq;
    RF4CE_ZRC2_KeyExchangeConfParams_t      rZrc2KeyExchangeReq;
    RF4CE_ZRC2_BindConfParams_t             rZrc2BindReq;
    RF4CE_ZRC2_BindConfParams_t             rZrc2ProxyBindReq;
    RF4CE_ZRC2_BindingConfParams_t          rZrc2EnableBindingReq;
    RF4CE_ZRC2_BindingConfParams_t          rZrc2DisableBindingReq;
    RF4CE_ZRC2_BindingConfParams_t          rZrc2BottonBindingReq;
    RF4CE_ZRC2_ControlCommandConfParams_t   rZrc2ControlCommandConfParams;
    RF4CE_GDP2_SetPollConstraintsConfParams_t   rGdp2SetPollConstraintsConf;
    RF4CE_GDP2_PollNegotiationConfParams_t      rGdp2PollNegotiationConf;
    RF4CE_GDP2_PollClientUserEventConfParams_t  rGdp2PollClientUserEventConf;
    RF4CE_GDP2_ClientNotificationConfParams_t   rGdp2ClientNotificationConf;
    RF4CE_GDP2_IdentifyCapAnnounceConfParams_t  rGdp2IdentifyCapAnnounceConf;
    RF4CE_GDP2_IdentifyConfParams_t             rGdp2IdentifyConf;
    RF4CE_ZRC2_GetSharedSecretRespParams_t      rZrc2GetSharedSecretInd;
#   endif /* USE_RF4CE_PROFILE_ZRC2 */
#  endif /* USE_RF4CE_PROFILE_ZRC */
#  if (1 == USE_RF4CE_PROFILE_MSO)
    RF4CE_MSO_GetProfileAttributeConfParams_t rMsoGetProfileAttributeConfParams;
    RF4CE_MSO_SetProfileAttributeConfParams_t rMsoSetProfileAttributeConfParams;
    RF4CE_MSO_GetRIBAttributeConfParams_t rMsoGetRIBAttributeConfParams;
    RF4CE_MSO_SetRIBAttributeConfParams_t rMsoSetRIBAttributeConfParams;
    RF4CE_MSO_BindConfParams_t rMsoBindConfParams;
    RF4CE_MSO_UserControlConfParams_t rMsoUserControlConfParams;
#  endif /* (1 == USE_RF4CE_PROFILE_MSO) */
# endif /* _RF4CE_ */
#endif

    NVM_ReadFileRespParams_t    rNvmReadFileReq;
    NVM_OpenFileRespParams_t    rNvmOpenFileReq;
    NVM_WriteFileRespParams_t   rNvmWriteFileReq;
    NVM_CloseFileRespParams_t   rNvmCloseFileReq;

#if defined(_PHY_TEST_HOST_INTERFACE_)
    Phy_Test_Get_Caps_ConfParams_t rPhyTestGetCapsConfParams;
    Phy_Test_Set_Channel_ConfParams_t rPhyTestSetChannelConfParams;
    Phy_Test_Continuous_Wave_StartStop_ConfParams_t rPhyTestContinuousWaveStartConfParams;
    Phy_Test_Continuous_Wave_StartStop_ConfParams_t rPhyTestContinuousWaveStopConfParams;
    Phy_Test_Transmit_StartStop_ConfParams_t rPhyTestTransmitStartConfParams;
    Phy_Test_Transmit_StartStop_ConfParams_t rPhyTestTransmitStopConfParams;
    Phy_Test_Receive_StartStop_ConfParams_t rPhyTestReceiveStartConfParams;
    Phy_Test_Receive_StartStop_ConfParams_t  rPhyTestReceiveStopConfParams;
    Phy_Test_Echo_StartStop_ConfParams_t rPhyTestEchoStartConfParams;
    Phy_Test_Echo_StartStop_ConfParams_t rPhyTestEchoStopConfParams;
    Phy_Test_Energy_Detect_Scan_ConfParams_t rPhyTestEnergyDetectScanConfParams;
    Phy_Test_Get_Stats_ConfParams_t rPhyTestGetStatsConfParams;
    Phy_Test_Reset_Stats_ConfParams_t rPhyTestResetStatsConfParams;
    Phy_Test_Set_TX_Power_ConfParams_t rPhyTestSetTXPowerConfParams;
    RF4CE_Diag_Caps_ConfParams_t  rRF4CEDiagCapsConfParams;
    RF4CE_Diag_ConfParams_t       rRF4CEDiagConfParams;
#endif
} MailConfParams_t;

/**//**
 * \brief Union of all indication parameters.
 */
typedef union _MailIndParams_t
{
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_TEST_ENGINE_)
    TE_HelloCommandIndParams_t      teHello;
    TE_AssertLogIdCommandIndParams_t teAssertLogId;
#endif /* _MAILBOX_WRAPPERS_TEST_ENGINE_ */

    // TODO: fill me as .*IndParams_t;$
#if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_MAC_)
    MAC_DataIndParams_t             macDataInd;
# if defined(_ZBPRO_) || defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
    MAC_BeaconNotifyIndParams_t     macBeaconNotifyInd;
    MAC_CommStatusIndParams_t       macCommStatusInd;
# endif
# if defined(_ZBPRO_) || defined(MAILBOX_UNIT_TEST)
    MAC_AssociateIndParams_t        macAssociateInd;
    MAC_OrphanIndParams_t           macOrphanInd;
    MAC_PollIndParams_t             macPollInd;
# endif
#endif /* _MAILBOX_WRAPPERS_MAC_ */

#if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_NWK_)
# ifdef _RF4CE_
    RF4CE_NWK_DataIndParams_t       rNwkData;
    RF4CE_NWK_DiscoveryIndParams_t  rnwkDiscovery;
    RF4CE_NWK_COMMStatusIndParams_t rNwkCommStatus;
    RF4CE_NWK_PairIndParams_t       rNwkPair;
    RF4CE_NWK_UnpairIndParams_t     rNwkUnpair;
#  ifdef ENABLE_GU_KEY_SEED_IND
    RF4CE_NWK_KeySeedStartIndParams_t rNwkIndKeySeed;
#  endif /* ENABLE_GU_KEY_SEED_IND */
# endif
#endif /* _MAILBOX_WRAPPERS_NWK_ */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_APS_)
# ifdef _ZBPRO_
    ZBPRO_APS_DataIndParams_t           zApsData;
    ZBPRO_APS_TransportKeyIndParams_t   zApsTransport;
    ZBPRO_APS_UpdateDeviceIndParams_t   zApsUpdateDevice;
    ZBPRO_APS_RemoveDeviceIndParams_t   zApsRemoveDevice;
    ZBPRO_APS_RequestKeyIndParams_t     zApsRequestKey;
    ZBPRO_APS_SwitchKeyIndParams_t      zApsSwitchKey;
# endif /* _ZBPRO_ */
#endif /* _MAILBOX_WRAPPERS_APS_ */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZDO_)
# ifdef _ZBPRO_
    ZBPRO_ZDO_MgmtNwkUpdateUnsolIndParams_t     zZdoMgmtNwkUpdateUnsol;
# endif /* _ZBPRO_ */
#endif /* _MAILBOX_WRAPPERS_ZDO_ */

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_)
# ifdef _ZBPRO_
    ZBPRO_ZCL_ProfileWideCmdReportAttributesIndParams_t         zZclProfileWideCmdReportAttributesInd;
#  if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_IDENTIFY_RELAY_)
    ZBPRO_ZCL_IdentifyCmdIdentifyIndParams_t                    zZclIdentifyCmdIdentifyInd;
    ZBPRO_ZCL_IdentifyCmdIdentifyQueryIndParams_t               zZclIdentifyCmdIdentifyQueryInd;
#  else
    ZBPRO_ZCL_IdentifyIndParams_t                               zZclIdentifyInd;
#  endif
    ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseIndParams_t       zZclIdentifyCmdIdentifyQueryResponseInd;
    ZBPRO_ZCL_GroupsCmdGetGroupMembershipIndParams_t            zZclGroupsCmdGetGroupMembershipInd;
    ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseIndParams_t    zZclScenesCmdGetSceneMembershipResponseInd;
    ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationIndParams_t    zZclIASZoneCmdZoneStatusChangeNotificationInd;
    ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestIndParams_t               zZclIASZoneCmdZoneEnrollRequestInd;
    ZBPRO_ZCL_SapIasAceArmIndParams_t                           zZclIasAceArmIndParams;
    ZBPRO_ZCL_SapIasAceBypassIndParams_t                        zZclIasAceBypassIndParams;
    ZBPRO_ZCL_SapIasAceAlarmIndParams_t                         zZclIasAceAlarmIndParams;
    ZBPRO_ZCL_SapIasAceGetZoneIdMapIndParams_t                  zZclIasAceGetZoneIdMapIndParams;
    ZBPRO_ZCL_SapIasAceGetZoneInfoIndParams_t                   zZclIasAceGetZoneInfoIndParams;
    ZBPRO_ZCL_SapIasAceGetPanelStatusIndParams_t                zZclIasAceGetPanelStatusIndParams;
    ZBPRO_ZCL_SapIasAceGetBypassedZoneListIndParams_t           zZclIasAceGetBypassedZoneListIndParams;
    ZBPRO_ZCL_SapIasAceGetZoneStatusIndParams_t                 zZclIasAceGetZoneStatusIndParams;
# endif /* _ZBPRO_ */
#endif

#if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_PROFILE_)
# ifdef _RF4CE_
    RF4CE_PairingReferenceIndParams_t rProfileCounterExpired;
    RF4CE_PairingReferenceIndParams_t rProfileUnpair;
    RF4CE_PairingIndParams_t          rProfilePair;

#  if (1 == USE_RF4CE_PROFILE_ZRC)
#   ifdef USE_RF4CE_PROFILE_ZRC1
#    if defined(RF4CE_TARGET) || defined(MAILBOX_UNIT_TEST)
    RF4CE_ZRC1_ControlCommandIndParams_t rZrc1ControlCommand;
#    endif /* RF4CE_TARGET */
    RF4CE_ZRC1_VendorSpecificIndParams_t rZrc1VendorSpecific;
#   endif /* USE_RF4CE_PROFILE_ZRC1 */
#   ifdef USE_RF4CE_PROFILE_ZRC2
    RF4CE_ZRC2_CheckValidationIndParams_t rZrc2StartValidation;
    RF4CE_ZRC2_CheckValidationIndParams_t rZrc2CheckValidation;
    RF4CE_ZRC2_ControlCommandIndParams_t  rZrc2ControlCommandIndParams;
    RF4CE_GDP2_PollNegotiationIndParams_t       rGdp2PollNegotiationInd;
    RF4CE_GDP2_HeartbeatIndParams_t             rGdp2HeartbeatInd;
    RF4CE_GDP2_IdentifyCapAnnounceIndParams_t   rGdp2IdentifyCapAnnounceInd;
    RF4CE_GDP2_IdentifyIndParams_t              rGdp2IdentifyInd;
    RF4CE_GDP2_ClientNotificationIndParams_t    rGdp2ClientNotificationInd;
    RF4CE_PairingIndParams_t                    rZrc2Pair;
    RF4CE_ZRC2_BindingFinishedNtfyIndParams_t   rZrc2BindingFinished;
    RF4CE_ZRC2_SetAttributesReqParams_t         rZrc2GetAttrRespInd;
    RF4CE_ZRC2_SetAttributesReqParams_t         rZrc2PushAttrReqInd;
#   endif /* USE_RF4CE_PROFILE_ZRC2 */
#  endif /* USE_RF4CE_PROFILE_ZRC */
#  if (1 == USE_RF4CE_PROFILE_MSO)
    RF4CE_PairingReferenceIndParams_t rMsoStartValidationIndParams;
    RF4CE_MSO_CheckValidationIndParams_t rMsoCheckValidationIndParams;
    RF4CE_MSO_UserControlIndParams_t rMsoUserControlIndParams;
#  endif /* (1 == USE_RF4CE_PROFILE_MSO) */
# endif /* _RF4CE_ */
#endif

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_PROFILE_)
# ifdef _ZBPRO_
#  if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZHA_)
    ZBPRO_ZHA_CieSetPanelStatusIndParams_t      zZhaCieSetPanelStatus;
    ZBPRO_ZHA_CieEnrollIndParams_t              zZhaCieEnrollInd;
#  endif /* (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZHA_) */
# endif /* _ZBPRO */
#endif /* (WRAPPERS_OFF != _MAILBOX_WRAPPERS_PROFILE_) */

    SYS_PrintIndParams_t                zSysPrintIndParams;

    SYS_EventHandlerMailParams_t        eventSubscribeCommand;
    SYS_EventNotifyParams_t             eventRaiseCommand;
    SYS_EventNotifyParams_t             eventNotifyCommand;
} MailIndParams_t;

/**//**
 * \brief Union of all service parameters.
 */
typedef union _MailServiceReq_t
{
    MailAckDescr_t                  ack;
} MailServiceReq_t;

typedef union _MailServiceConfParams_t
{
    MailAckConfParams_t             ack;
} MailServiceConfParams_t;


typedef struct _MailDescriptor_t MailDescriptor_t;
extern MailDescriptor_t *mailDescriptorPtr;
extern void Mail_Serialize(MailDescriptor_t *const mail, uint16_t fId, void *req);

#ifdef MAILBOX_UNIT_TEST
/* */
extern void MailUnitTest_f1_Call(MailUnitTest_f1Descr_t *req);
extern void MailUnitTest_f2_Call(MailUnitTest_f2Descr_t *req);
extern void MailUnitTest_f3_Call(MailUnitTest_f3Descr_t *req);
extern void MailUnitTest_f4_Call(MailUnitTest_f4Descr_t *req);
extern void MailUnitTest_f5_Call(MailUnitTest_f5Descr_t *req);

extern void Mail_TestEnginePing_Call(TE_PingCommandReqDescr_t *req);
extern void Mail_TestEngineEcho_Call(TE_EchoCommandReqDescr_t *req);
extern void Mail_TestEngineReset_Call(TE_ResetCommandReqDescr_t *req);
extern void Mail_SetEchoDelay_Call(TE_SetEchoDelayCommandReqDescr_t *req);
extern void SYS_EventSubscribe_Call(SYS_EventHandlerParams_t*);
extern void Mail_Host2Uart1_Call(TE_Host2Uart1ReqDescr_t *req);
extern void RF4CE_ZRC_SetWakeUpActionCodeReq_Call(RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t *req);
extern void RF4CE_ZRC_GetWakeUpActionCodeReq_Call(RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t *req);

/* MAC API unit test */
extern void MAC_BanTableSetDefaultAction_Call(MAC_BanTableSetDefaultActionReqDescr_t *reqDescr);
extern void MAC_BanTableAddLink_Call(MAC_BanTableAddLinkReqDescr_t *reqDescr);

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
extern void RF4CE_NWK_DiscoveryResp_Call(RF4CE_NWK_DiscoveryRespDescr_t *reqDescr);
extern void RF4CE_NWK_PairResp_Call(RF4CE_NWK_PairRespDescr_t *reqDescr);
extern void RF4CE_NWK_UnpairResp_Call(RF4CE_NWK_UnpairRespDescr_t *reqDescr);
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
extern void ZBPRO_ZHA_EzModeReq_Call(ZBPRO_ZHA_EzModeReqDescr_t *reqDescr);
extern void ZBPRO_ZHA_CieDeviceEnrollReq_Call(ZBPRO_ZHA_CieEnrollReqDescr_t *reqDescr);
extern void ZBPRO_ZHA_CieDeviceSetPanelStatusReq_Call(ZBPRO_ZHA_CieSetPanelStatusReqDescr_t * const reqDescr);
extern void ZBPRO_ZHA_CieDeviceSetPanelStatusReqInd_Call(ZBPRO_ZHA_CieSetPanelStatusIndParams_t   *const   indParams);
extern void ZBPRO_ZHA_CieZoneSetBypassStateReq_Call(ZBPRO_ZHA_CieZoneSetBypassStateReqDescr_t   *const descr);
extern void ZBPRO_ZHA_CieDeviceEnrollInd_Call(ZBPRO_ZHA_CieEnrollIndParams_t *const indParams);

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