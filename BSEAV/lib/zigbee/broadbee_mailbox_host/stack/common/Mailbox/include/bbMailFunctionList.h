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

/*******************************************************************************
 *
 * DESCRIPTION:
 *
 *
*******************************************************************************/

#ifndef MAILBOX_ENUM
#define MAILBOX_ENUM

#include "bbSysDbg.h"

/**//**
 * \brief Enumeration of API function's id
 */
typedef enum
{
    INCORRECT_REQ_ID                = 0x0000U, // This value must equal to zero. Please don't touch.

    /* Test engine section start */
    TE_ASSERT_ERRID_FID             = 0x03FFU, // This value used inside HAL code. Please don't touch.
    TE_RESET_FID                    = 0x0001U, // This value used inside HAL code. Please don't touch.
    TE_PING_FID,
    TE_HELLO_FID,
    TE_ASSERT_LOGID_FID,
    TE_MAILBOX_ACK_FID,
    TE_ECHO_FID,
    /* Test engine section end */

    /* MAILBOX_UNIT_TEST section start */
    MAIL_F1_FID,
    MAIL_F2_FID,
    MAIL_F3_FID,
    MAIL_F4_FID,
    MAIL_F5_FID,
    /* MAILBOX_UNIT_TEST section end */

    /* Event system section start */
    SYS_EVENT_SUBSCRIBE_FID,
    SYS_EVENT_RAISE_FID,
    SYS_EVENT_NOTIFY_FID,
    SYS_PRINT_FID,
    /* Event system section end */

    /* NVM section start */
    NVM_READ_FILE_FID,
    NVM_WRITE_FILE_FID,
    NVM_OPEN_FILE_FID,
    NVM_CLOSE_FILE_FID,
    /* NVM section end */

    /* Profiling Engine section start */
    PE_GET_FREE_BLOCKS_FID,
    PE_RESET_FID,
    PE_GETDATA_FID,
    PE_SCAN_START_FID,
    PE_SCAN_STOP_FID,
    PE_SCAN_GET_HISTORY_FID,
    PE_SCAN_GET_HISTORY_IND_FID,
    /* Profiling Engine section end */

    /* MAC section start */
    MAC_REQ_BAN_SET_DEFAULT_ACTION_FID,
    MAC_REQ_BAN_LINK_FID,

    /* MAC RF4CE context */
    RF4CE_MAC_REQ_GET_FID,
    RF4CE_MAC_REQ_SET_FID,
    RF4CE_MAC_REQ_DATA_FID,
    RF4CE_MAC_REQ_RESET_FID,
    RF4CE_MAC_REQ_RX_ENABLE_FID,
    RF4CE_MAC_REQ_START_FID,
    RF4CE_MAC_IND_DATA_FID,
    RF4CE_MAC_REQ_SCAN_FID,
    RF4CE_MAC_IND_BEACON_NOTIFY_FID,
    RF4CE_MAC_IND_COMM_STATUS_FID,
    /* MAC ZBPRO context */
    ZBPRO_MAC_REQ_GET_FID,
    ZBPRO_MAC_REQ_SET_FID,
    ZBPRO_MAC_REQ_DATA_FID,
    ZBPRO_MAC_REQ_PURGE_FID,
    ZBPRO_MAC_REQ_ASSOCIATE_FID,
    ZBPRO_MAC_REQ_RESET_FID,
    ZBPRO_MAC_REQ_RX_ENABLE_FID,
    ZBPRO_MAC_REQ_SCAN_FID,
    ZBPRO_MAC_REQ_START_FID,
    ZBPRO_MAC_RESP_ASSOCIATE_FID,
    ZBPRO_MAC_RESP_ORPHAN_FID,
    ZBPRO_MAC_REQ_POLL_FID,
    ZBPRO_MAC_IND_DATA_FID,
    ZBPRO_MAC_IND_ASSOCIATE_FID,
    ZBPRO_MAC_IND_BEACON_NOTIFY_FID,
    ZBPRO_MAC_IND_COMM_STATUS_FID,
    ZBPRO_MAC_IND_ORPHAN_FID,
    ZBPRO_MAC_IND_POLL_FID,
    /* MAC section end */

    /* RF4CE NWK section start */
    RF4CE_NWK_REQ_RESET_FID,
    RF4CE_NWK_REQ_START_FID,
    RF4CE_NWK_REQ_DATA_FID,
    RF4CE_NWK_REQ_GET_FID,
    RF4CE_NWK_REQ_SET_FID,
    RF4CE_NWK_REQ_DISCOVERY_FID,
    RF4CE_NWK_RESP_DISCOVERY_FID,
    RF4CE_NWK_REQ_AUTODISCOVERY_FID,
    RF4CE_NWK_REQ_RXENABLE_FID,
    RF4CE_NWK_REQ_UPDATEKEY_FID,
    RF4CE_NWK_REQ_PAIR_FID,
    RF4CE_NWK_RESP_PAIR_FID,
    RF4CE_NWK_REQ_UNPAIR_FID,
    RF4CE_NWK_RESP_UNPAIR_FID,
    RF4CE_NWK_IND_DATA_FID,
    RF4CE_NWK_IND_DISCOVERY_FID,
    RF4CE_NWK_IND_COMMSTATUS_FID,
    RF4CE_NWK_IND_PAIR_FID,
    RF4CE_NWK_IND_UNPAIR_FID,
    RF4CE_NWK_KEY_SEED_START_IND_FID,
    /* RF4CE NWK section end */

    /* ZBPRO NWK section start */
    ZBPRO_NWK_REQ_GET_FID,              /* just reserved, you can use ZBPRO_APS_REQ_GET_FID instead */
    ZBPRO_NWK_REQ_SET_FID,              /* just reserved, you can use ZBPRO_APS_REQ_SET_FID instead */
    ZBPRO_NWK_REQ_GET_KEY_FID,
    ZBPRO_NWK_REQ_SET_KEY_FID,
    ZBPRO_NWK_REQ_PERMIT_JOINING_FID,
    ZBPRO_NWK_REQ_LEAVE_FID,
    ZBPRO_NWK_REQ_ROUTE_DISCOVERY_FID,
    /* ZBPRO NWK section end */

    /* ZBPRO APS section start */
    ZBPRO_APS_REQ_GET_FID,
    ZBPRO_APS_REQ_SET_FID,
    ZBPRO_APS_REQ_GET_KEY_FID,
    ZBPRO_APS_REQ_SET_KEY_FID,
    ZBPRO_APS_REQ_ENDPOINTREGISTER_FID,
    ZBPRO_APS_REQ_ENDPOINTUNREGISTER_FID,
    ZBPRO_APS_REQ_DATA_FID,
    ZBPRO_APS_REQ_BIND_FID,
    ZBPRO_APS_REQ_UNBIND_FID,
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
    ZBPRO_ZDO_REQ_MGMT_PERMIT_JOINING_FID,
    ZBPRO_ZDO_REQ_MGMT_BIND_FID,
    ZBPRO_ZDO_REQ_MGMT_LQI_FID,
    ZBPRO_ZDO_REQ_MGMT_NWK_UPDATE_FID,
    ZBPRO_ZDO_RESP_MGMT_NWK_UPDATE_UNSOL_FID,
    ZBPRO_ZDO_IND_MGMT_NWK_UPDATE_UNSOL_FID,
    ZBPRO_ZDO_IND_DEVICE_ANNCE_FID,
    ZBPRO_ZDO_IND_READY_FID,
    /* ZBPRO ZDO section end */

    /* ZBPRO TC section start */
    ZBPRO_TC_REQ_NWK_KEY_UPDATE_FID,
    /* ZBPRO TC section end */


    /* ZBPRO ZCL section start */
    /*      wide commands           */
    ZBPRO_ZCL_PROFILE_WIDE_CMD_DISCOVER_ATTRIBUTE_REQ_FID,
    ZBPRO_ZCL_PROFILE_WIDE_CMD_READ_ATTRIBUTE_REQ_FID,
    ZBPRO_ZCL_PROFILE_WIDE_CMD_WRITE_ATTRIBUTE_REQ_FID,
    ZBPRO_ZCL_PROFILE_WIDE_CMD_CONFIGURE_REPORTING_REQ_FID,
    ZBPRO_ZCL_PROFILE_WIDE_CMD_READ_REPORTING_CONFIGURATION_REQ_FID,
    ZBPRO_ZCL_PROFILE_WIDE_CMD_REPORT_ATTRIBUTES_IND_FID,
    /*      basic cluster           */
    ZBPRO_ZCL_REQ_SET_POWER_SOURCE_FID,
    /*      identify cluster        */
    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_REQ_FID,
    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_REQ_FID,
    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_RESPONSE_IND_FID,
    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_RESPONSE_REQ_FID,
    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_RESPONSE_REQ_FID,
    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_IND_FID,
    ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_IND_FID,
    ZBPRO_ZCL_IND_IDENTIFY_FID,

    /*      group cluster           */
    ZBPRO_ZCL_GROUPS_CMD_ADD_GROUP_REQ_FID,
    ZBPRO_ZCL_GROUPS_CMD_VIEW_GROUP_REQ_FID,
    ZBPRO_ZCL_GROUPS_CMD_GET_GROUP_MEMBERSHIP_REQ_FID,
    ZBPRO_ZCL_GROUPS_CMD_REMOVE_GROUP_REQ_FID,
    ZBPRO_ZCL_GROUPS_CMD_REMOVE_ALL_GROUPS_REQ_FID,
    ZBPRO_ZCL_GROUPS_CMD_ADD_GROUP_IF_IDENTIFY_REQ_FID,
    ZBPRO_ZCL_GROUPS_CMD_GET_GROUP_MEMBERSHIP_RESPONSE_IND_FID,

    /*      scene cluster           */
    ZBPRO_ZCL_SCENES_CMD_ADD_SCENE_REQ_FID,
    ZBPRO_ZCL_SCENES_CMD_VIEW_SCENE_REQ_FID,
    ZBPRO_ZCL_SCENES_CMD_STORE_SCENE_REQ_FID,
    ZBPRO_ZCL_SCENES_CMD_RECALL_SCENE_REQ_FID,
    ZBPRO_ZCL_SCENES_CMD_REMOVE_SCENE_REQ_FID,
    ZBPRO_ZCL_SCENES_CMD_REMOVE_ALL_SCENES_REQ_FID,
    ZBPRO_ZCL_SCENES_CMD_GET_SCENE_MEMBERSHIP_REQ_FID,
    ZBPRO_ZCL_SCENES_CMD_GET_SCENE_MEMBERSHIP_RESPONSE_IND_FID,

    /*      on/off cluster          */
    ZBPRO_ZCL_ONOFF_CMD_OFF_REQ_FID,
    ZBPRO_ZCL_ONOFF_CMD_ON_REQ_FID,
    ZBPRO_ZCL_ONOFF_CMD_TOGGLE_REQ_FID,

    /*      level control cluster   */
    ZBPRO_ZCL_LEVEL_CONTROL_CMD_MOVE_TO_LEVEL_REQ_FID,
    ZBPRO_ZCL_LEVEL_CONTROL_CMD_MOVE_REQ_FID,
    ZBPRO_ZCL_LEVEL_CONTROL_CMD_STEP_REQ_FID,
    ZBPRO_ZCL_LEVEL_CONTROL_CMD_STOP_REQ_FID,

    /*      door lock cluster       */
    ZBPRO_ZCL_DOOR_LOCK_CMD_LOCK_REQ_FID,
    ZBPRO_ZCL_DOOR_LOCK_CMD_UNLOCK_REQ_FID,

    /*      window covering cluster */
    ZBPRO_ZCL_WINDOW_COVERING_CMD_UP_OPEN_REQ_FID,
    ZBPRO_ZCL_WINDOW_COVERING_CMD_DOWN_CLOSE_REQ_FID,
    ZBPRO_ZCL_WINDOW_COVERING_CMD_STOP_REQ_FID,
    ZBPRO_ZCL_WINDOW_COVERING_CMD_GO_TO_LIFT_PERCENTAGE_REQ_FID,
    ZBPRO_ZCL_WINDOW_COVERING_CMD_GO_TO_TILT_PERCENTAGE_REQ_FID,

    /*      ias zone cluster        */
    ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_ENROLL_RESPONSE_REQ_FID,
    ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_ENROLL_IND_FID,
    ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_STATUS_CHANGED_NOTIFICATION_IND_FID,

    /*      ias ace cluster         */
    ZBPRO_ZCL_IAS_ACE_ARM_RESP_REQ_FID,
    ZBPRO_ZCL_IAS_ACE_BYPASS_RESP_REQ_FID,
    ZBPRO_ZCL_IAS_ACE_GET_ZONE_ID_MAP_RESP_REQ_FID,
    ZBPRO_ZCL_IAS_ACE_GET_ZONE_INFO_RESP_REQ_FID,
    ZBPRO_ZCL_IAS_ACE_GET_PANEL_STATUS_RESP_REQ_FID,
    ZBPRO_ZCL_IAS_ACE_SET_BYPASSED_ZONE_LIST_RESP_REQ_FID,
    ZBPRO_ZCL_IAS_ACE_GET_ZONE_STATUS_RESP_REQ_FID,
    ZBPRO_ZCL_IAS_ACE_ZONE_STATUS_CHANGED_REQ_FID,
    ZBPRO_ZCL_IAS_ACE_PANEL_STATUS_CHANGED_REQ_FID,
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

    /*      ias wd cluster          */
    ZBPRO_ZCL_IAS_WD_CMD_START_WARNING_REQ_FID,
    ZBPRO_ZCL_IAS_WD_CMD_SQUAWK_REQ_FID,

    /*      color control cluster   */
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

    /*      OTA Upgrade cluster   */
    ZBPRO_ZCL_OTAU_CMD_QUERY_NEXT_IMAGE_REQUEST_IND_FID,
    ZBPRO_ZCL_OTAU_CMD_QUERY_NEXT_IMAGE_RESPONSE_REQ_FID,
    ZBPRO_ZCL_OTAU_CMD_IMAGE_BLOCK_REQUEST_IND_FID,
    ZBPRO_ZCL_OTAU_CMD_IMAGE_BLOCK_RESPONSE_REQ_FID,
    ZBPRO_ZCL_OTAU_CMD_UPGRADE_END_REQUEST_IND_FID,
    ZBPRO_ZCL_OTAU_CMD_UPGRADE_END_RESPONSE_REQ_FID,

    /* ZBPRO ZCL section end */

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
    RF4CE_ZRC1_REQ_COMMANDDISCOVERY_FID,
    RF4CE_ZRC1_CONTROL_COMMAND_PRESSED_FID,
    RF4CE_ZRC1_CONTROL_COMMAND_RELEASED_FID,
    RF4CE_ZRC1_REQ_CONTROLLER_BIND_FID,
    RF4CE_ZRC1_VENDORSPECIFIC_REQ_FID,
    RF4CE_ZRC1_REQ_TARGET_BIND_FID,
    RF4CE_ZRC1_VENDORSPECIFIC_IND_FID,
    RF4CE_ZRC1_IND_CONTROLCOMMAND_FID,
    /* RF4CE ZRC 1.1 section end */

    /* RF4CE GDP 2.0 */
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
    /* RF4CE GDP 2.0 end */

    /* ZRC 2.0 section start */
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
    RF4CE_ZRC2_CONTROL_COMMAND_PRESS_REQ_FID,
    RF4CE_ZRC2_CONTROL_COMMAND_RELEASE_REQ_FID,
    RF4CE_ZRC2_START_VALIDATION_FID,
    RF4CE_ZRC2_CHECK_VALIDATION_IND_FID,
    RF4CE_ZRC2_CONTROL_COMMAND_IND_FID,
    RF4CE_ZRC2_GET_SHARED_SECRET_IND_FID,
    RF4CE_ZRC2_PAIR_IND_FID,
    RF4CE_ZRC2_BINDING_FINISHED_IND_FID,
    RF4CE_ZRC2_IND_GET_RESP_FID,
    RF4CE_ZRC2_IND_PUSH_REQ_FID,
    /* ZRC 2.0 section end */

    /* RF4CE MSO profile section start */
    RF4CE_MSO_GET_PROFILE_ATTRIBUTE_FID,
    RF4CE_MSO_SET_PROFILE_ATTRIBUTE_FID,
    RF4CE_MSO_GET_RIB_ATTRIBUTE_FID,
    RF4CE_MSO_SET_RIB_ATTRIBUTE_FID,
    RF4CE_MSO_VALIDATE_RESP_FID,
    RF4CE_MSO_BIND_FID,
    RF4CE_MSO_USER_CONTROL_PRESSED_FID,
    RF4CE_MSO_USER_CONTROL_RELEASED_FID,
    RF4CE_MSO_CHECK_VALIDATION_IND_FID,
    RF4CE_MSO_USER_CONTROL_IND_FID,
    RF4CE_MSO_START_VALIDATION_IND_FID,
    /* RF4CE MSO profile section end */

    /* ZBPRO ZHA profile section start */
    ZBPRO_ZHA_EZ_MODE_FID,
    /*      CIE device custom implementation    */
    ZBPRO_ZHA_CIE_REGISTER_DEVICE_REQ_FID,
    ZBPRO_ZHA_CIE_UNREGISTER_DEVICE_REQ_FID,
    ZBPRO_ZHA_CIE_ENROLL_FID,
    ZBPRO_ZHA_CIE_SET_PANEL_STATUS_REQ_FID,
    ZBPRO_ZHA_CIE_ZONE_SET_BYPASS_STATE_REQ_FID,
    ZBPRO_ZHA_CIE_SET_PANEL_STATUS_IND_FID,
    ZBPRO_ZHA_CIE_ENROLL_IND_FID,
    /* ZBPRO ZHA profile section end */

    /* Addition API section start */
    /*      Power filter control                */
    RF4CE_ZRC_GET_WAKEUP_ACTION_CODE_FID,
    RF4CE_ZRC_SET_WAKEUP_ACTION_CODE_FID,

    /*      PHY test interface                  */
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
    RF4CE_MAC_IND_DATA_STUB_FID,

    TE_ROUTING_CHANGE_REQ_FID,

    /* Addition API section end */
    GET_FW_REV_FID,
    FINAL_PUBLIC_FID,

} MailFID_t;
SYS_DbgAssertStatic(0x0000U == INCORRECT_REQ_ID);       //NOTE: This is special value and must be zero.
SYS_DbgAssertStatic(0x0001U == TE_RESET_FID);           //NOTE: This value hardcoded inside HAL code.
SYS_DbgAssertStatic(0x03FFU == TE_ASSERT_ERRID_FID);    //NOTE: This value hardcoded inside HAL code.

#endif // #ifndef MAILBOX_ENUM


#ifndef ANY_DIRECTION
# define ANY_DIRECTION(function_name, enum_name, desc_mod, desc_name, params_mod, parameters_name, param_payload, confirm_mod, confirm_name, confirm_payload)
#endif

#ifndef HOST_TO_STACK
# define HOST_TO_STACK(function_name, enum_name, desc_mod, desc_name, params_mod, parameters_name, param_payload, confirm_mod, confirm_name, confirm_payload)
#endif

#ifndef STACK_TO_HOST
# define STACK_TO_HOST(function_name, enum_name, desc_mod, desc_name, params_mod, parameters_name, param_payload, confirm_mod, confirm_name, confirm_payload)
#endif

// NOTE: Reserved words
#undef HAS_CB
#undef NO_CB
#undef NO_DATA
#undef HAS_DATA
#undef NO_PARAMS
#undef NO_CONFIRM
#undef NO_APPROPRIATE_TYPE
#undef NO_FIELD

/**********************************************************************************************************************/
/* Test engine API                                                                                                    */
/**********************************************************************************************************************/
/* NOTE: TE_RESET_FID must be on the first position. */
HOST_TO_STACK(Mail_TestEngineReset, TE_RESET_FID,
              NO_CB, TE_ResetCommandReqDescr_t,
              NO_DATA, TE_ResetCommandReqParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

HOST_TO_STACK(Mail_TestEnginePing, TE_PING_FID,
              HAS_CB, TE_PingCommandReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, TE_PingCommandConfParams_t, payload)

STACK_TO_HOST(Mail_TestEngineHelloInd, TE_HELLO_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, TE_HelloCommandIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(Mail_TestEngineAssertLogIdInd, TE_ASSERT_LOGID_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, TE_AssertLogIdCommandIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

ANY_DIRECTION(Mail_TestEngineMailboxAckReq, TE_MAILBOX_ACK_FID,
              HAS_CB, TE_MailboxAckDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, TE_MailboxAckConfParams_t, payload)

#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_TEST_ENGINE_)
HOST_TO_STACK(Mail_TestEngineEcho, TE_ECHO_FID,
              HAS_CB, TE_EchoCommandReqDescr_t,
              HAS_DATA, TE_EchoCommandReqParams_t, payload,
              HAS_DATA, TE_EchoCommandConfParams_t, payload)
#endif /* _MAILBOX_WRAPPERS_TEST_ENGINE_ */

#ifdef MAILBOX_UNIT_TEST
HOST_TO_STACK(MailUnitTest_f1, MAIL_F1_FID,
              HAS_CB, MailUnitTest_f1Descr_t,
              NO_DATA, MailUnitTest_f1ReqParams_t, payload,
              NO_DATA, MailUnitTest_f1ConfParams_t, payload)

HOST_TO_STACK(MailUnitTest_f2, MAIL_F2_FID,
              HAS_CB, MailUnitTest_f2Descr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, MailUnitTest_f2ConfParams_t, payload)

HOST_TO_STACK(MailUnitTest_f3, MAIL_F3_FID,
              NO_CB, MailUnitTest_f3Descr_t,
              NO_DATA, MailUnitTest_f3ReqParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

HOST_TO_STACK(MailUnitTest_f4, MAIL_F4_FID,
              HAS_CB, MailUnitTest_f4Descr_t,
              HAS_DATA, MailUnitTest_f4ReqParams_t, payload,
              NO_DATA, MailUnitTest_f4ConfParams_t, payload)

HOST_TO_STACK(MailUnitTest_f5, MAIL_F5_FID,
              HAS_CB, MailUnitTest_f5Descr_t,
              NO_DATA, MailUnitTest_f5ReqParams_t, payload,
              HAS_DATA, MailUnitTest_f5ConfParams_t, payload)
#endif /* MAILBOX_UNIT_TEST */

/**********************************************************************************************************************/
/* Common system API                                                                                                  */
/**********************************************************************************************************************/
HOST_TO_STACK(sysEventSubscribeHostHandler, SYS_EVENT_SUBSCRIBE_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, SYS_EventHandlerMailParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

HOST_TO_STACK(SYS_EventRaise, SYS_EVENT_RAISE_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, SYS_EventNotifyParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(SYS_EventNtfy, SYS_EVENT_NOTIFY_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, SYS_EventNotifyParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(SYS_PrintInd, SYS_PRINT_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, SYS_PrintIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

/**** NVM feature **************************/
STACK_TO_HOST(NVM_ReadFileInd, NVM_READ_FILE_FID,
              HAS_CB, NVM_ReadFileIndDescr_t,
              NO_DATA, NVM_ReadFileIndParams_t, payload,
              HAS_DATA, NVM_ReadFileRespParams_t, payload)

STACK_TO_HOST(NVM_WriteFileInd, NVM_WRITE_FILE_FID,
              HAS_CB, NVM_WriteFileIndDescr_t,
              HAS_DATA, NVM_WriteFileIndParams_t, payload,
              NO_DATA, NVM_WriteFileRespParams_t, payload)

STACK_TO_HOST(NVM_OpenFileInd, NVM_OPEN_FILE_FID,
              HAS_CB, NVM_OpenFileIndDescr_t,
              NO_DATA, NVM_OpenFileIndParams_t, payload,
              NO_DATA, NVM_OpenFileRespParams_t, payload)

STACK_TO_HOST(NVM_CloseFileInd, NVM_CLOSE_FILE_FID,
              HAS_CB, NVM_CloseFileIndDescr_t,
              NO_DATA, NVM_CloseFileIndParams_t, payload,
              NO_DATA, NVM_CloseFileRespParams_t, payload)


/**********************************************************************************************************************/
/* Profiling Engine API                                                                                               */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_PROFILING_ENGINE_)
HOST_TO_STACK(SYS_DbgMmGetFreeBlocks, PE_GET_FREE_BLOCKS_FID,
              HAS_CB, SYS_DbgMmGetFreeBlocksReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, SYS_DbgMmGetFreeBlocksConfParams_t, payload)

# if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_PROFILING_ENGINE_)
HOST_TO_STACK(SYS_DbgPeResetReq, PE_RESET_FID,
              NO_CB, SYS_DbgPeResetReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

HOST_TO_STACK(SYS_DbgPeGetDataReq, PE_GETDATA_FID,
              HAS_CB, SYS_DbgPeGetDataReqDescr_t,
              NO_DATA, SYS_DbgPeGetDataReqParams_t, payload,
              HAS_DATA, SYS_DbgPeGetDataConfParams_t, payload)
# endif /* WRAPPERS_ALL */
#ifdef _PE_ENERGY_MEASUREMENT_
HOST_TO_STACK(EXT_PeScanEdStartReq, PE_SCAN_START_FID,
              HAS_CB, EXT_PeScanEdStartReqDescr_t,
              NO_DATA, EXT_PeScanEdStartReqParams_t, payload,
              NO_DATA, EXT_PeScanEdStartConfParams_t, payload)

HOST_TO_STACK(EXT_PeScanEdStopReq, PE_SCAN_STOP_FID,
              HAS_CB, EXT_PeScanEdStopReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, EXT_PeScanEdStopConfParams_t, payload)

HOST_TO_STACK(EXT_PeScanEdGetHistoryReq, PE_SCAN_GET_HISTORY_FID,
              HAS_CB, EXT_PeScanEdGetHistoryReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              HAS_DATA, EXT_PeScanEdGetHistoryConfParams_t, payload)

STACK_TO_HOST(EXT_PeScanEdGetHistoryInd, PE_SCAN_GET_HISTORY_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, EXT_PeScanEdGetHistoryConfParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)
#endif //_PE_ENERGY_MEASUREMENT_
#endif /* _MAILBOX_WRAPPERS_PROFILING_ENGINE_ */

/**********************************************************************************************************************/
/* MAC API                                                                                                            */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_MAC_)
/**** MAC BAN Table *********************************************************************/
# ifdef _MAC_BAN_TABLE_
HOST_TO_STACK(MAC_BanTableSetDefaultAction, MAC_REQ_BAN_SET_DEFAULT_ACTION_FID,
              HAS_CB, MAC_BanTableSetDefaultActionReqDescr_t,
              NO_DATA, MAC_BanTableSetDefaultActionReqParams_t, payload,
              NO_DATA, MAC_BanTableConfParams_t, payload)

HOST_TO_STACK(MAC_BanTableAddLink, MAC_REQ_BAN_LINK_FID,
              HAS_CB, MAC_BanTableAddLinkReqDescr_t,
              NO_DATA, MAC_BanTableAddLinkReqParams_t, payload,
              NO_DATA, MAC_BanTableConfParams_t, payload)

# endif /* _MAC_BAN_TABLE_ */

/**** MAC RF4CE *************************************************************************/
# ifdef _RF4CE_
HOST_TO_STACK(RF4CE_MAC_GetReq, RF4CE_MAC_REQ_GET_FID,
              HAS_CB, MAC_GetReqDescr_t,
              NO_DATA, MAC_GetReqParams_t, payload,
              HAS_DATA, MAC_GetConfParams_t, payload)

HOST_TO_STACK(RF4CE_MAC_SetReq, RF4CE_MAC_REQ_SET_FID,
              HAS_CB, MAC_SetReqDescr_t,
              HAS_DATA, MAC_SetReqParams_t, payload,
              NO_DATA, MAC_SetConfParams_t, payload)

#  if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_MAC_)
HOST_TO_STACK(RF4CE_MAC_DataReq, RF4CE_MAC_REQ_DATA_FID,
              HAS_CB, MAC_DataReqDescr_t,
              HAS_DATA, MAC_DataReqParams_t, payload,
              NO_DATA, MAC_DataConfParams_t, payload)

HOST_TO_STACK(RF4CE_MAC_ResetReq, RF4CE_MAC_REQ_RESET_FID,
              HAS_CB, MAC_ResetReqDescr_t,
              NO_DATA, MAC_ResetReqParams_t, payload,
              NO_DATA, MAC_ResetConfParams_t, payload)

HOST_TO_STACK(RF4CE_MAC_RxEnableReq, RF4CE_MAC_REQ_RX_ENABLE_FID,
              HAS_CB, MAC_RxEnableReqDescr_t,
              NO_DATA, MAC_RxEnableReqParams_t, payload,
              NO_DATA, MAC_RxEnableConfParams_t, payload)

HOST_TO_STACK(RF4CE_MAC_StartReq, RF4CE_MAC_REQ_START_FID,
              HAS_CB, MAC_StartReqDescr_t,
              NO_DATA, MAC_StartReqParams_t, payload,
              NO_DATA, MAC_StartConfParams_t, payload)

STACK_TO_HOST(RF4CE_MAC_DataInd, RF4CE_MAC_IND_DATA_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, MAC_DataIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

#    if defined(RF4CE_TARGET)
HOST_TO_STACK(RF4CE_MAC_ScanReq, RF4CE_MAC_REQ_SCAN_FID,
              HAS_CB, MAC_ScanReqDescr_t,
              NO_DATA, MAC_ScanReqParams_t, payload,
              HAS_DATA, MAC_ScanConfParams_t, payload)

STACK_TO_HOST(RF4CE_MAC_BeaconNotifyInd, RF4CE_MAC_IND_BEACON_NOTIFY_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, MAC_BeaconNotifyIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(RF4CE_MAC_CommStatusInd, RF4CE_MAC_IND_COMM_STATUS_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, MAC_CommStatusIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

#    endif /* RF4CE_TARGET */
#  endif /* WRAPPERS_ALL */

# endif /* _RF4CE_ */

/**** MAC ZBPRO *************************************************************************/
# ifdef _ZBPRO_
HOST_TO_STACK(ZBPRO_MAC_GetReq, ZBPRO_MAC_REQ_GET_FID,
              HAS_CB, MAC_GetReqDescr_t,
              NO_DATA, MAC_GetReqParams_t, payload,
              HAS_DATA, MAC_GetConfParams_t, payload)

HOST_TO_STACK(ZBPRO_MAC_SetReq, ZBPRO_MAC_REQ_SET_FID,
              HAS_CB, MAC_SetReqDescr_t,
              HAS_DATA, MAC_SetReqParams_t, payload,
              NO_DATA, MAC_SetConfParams_t, payload)

HOST_TO_STACK(ZBPRO_MAC_DataReq, ZBPRO_MAC_REQ_DATA_FID,
              HAS_CB, MAC_DataReqDescr_t,
              HAS_DATA, MAC_DataReqParams_t, payload,
              NO_DATA, MAC_DataConfParams_t, payload)

#  if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_MAC_)
HOST_TO_STACK(ZBPRO_MAC_PurgeReq, ZBPRO_MAC_REQ_PURGE_FID,
              HAS_CB, MAC_PurgeReqDescr_t,
              NO_DATA, MAC_PurgeReqParams_t, payload,
              NO_DATA, MAC_PurgeConfParams_t, payload)

HOST_TO_STACK(ZBPRO_MAC_AssociateReq, ZBPRO_MAC_REQ_ASSOCIATE_FID,
              HAS_CB, MAC_AssociateReqDescr_t,
              NO_DATA, MAC_AssociateReqParams_t, payload,
              NO_DATA, MAC_AssociateConfParams_t, payload)

HOST_TO_STACK(ZBPRO_MAC_ResetReq, ZBPRO_MAC_REQ_RESET_FID,
              HAS_CB, MAC_ResetReqDescr_t,
              NO_DATA, MAC_ResetReqParams_t, payload,
              NO_DATA, MAC_ResetConfParams_t, payload)

HOST_TO_STACK(ZBPRO_MAC_RxEnableReq, ZBPRO_MAC_REQ_RX_ENABLE_FID,
              HAS_CB, MAC_RxEnableReqDescr_t,
              NO_DATA, MAC_RxEnableReqParams_t, payload,
              NO_DATA, MAC_RxEnableConfParams_t, payload)

HOST_TO_STACK(ZBPRO_MAC_ScanReq, ZBPRO_MAC_REQ_SCAN_FID,
              HAS_CB, MAC_ScanReqDescr_t,
              NO_DATA, MAC_ScanReqParams_t, payload,
              HAS_DATA, MAC_ScanConfParams_t, payload)

HOST_TO_STACK(ZBPRO_MAC_StartReq, ZBPRO_MAC_REQ_START_FID,
              HAS_CB, MAC_StartReqDescr_t,
              NO_DATA, MAC_StartReqParams_t, payload,
              NO_DATA, MAC_StartConfParams_t, payload)

HOST_TO_STACK(ZBPRO_MAC_AssociateResp, ZBPRO_MAC_RESP_ASSOCIATE_FID,
              HAS_CB, MAC_AssociateRespDescr_t,
              NO_DATA, MAC_AssociateRespParams_t, payload,
              NO_DATA, MAC_CommStatusAssociateIndParams_t, payload)

HOST_TO_STACK(ZBPRO_MAC_OrphanResp, ZBPRO_MAC_RESP_ORPHAN_FID,
              HAS_CB, MAC_OrphanRespDescr_t,
              NO_DATA, MAC_OrphanRespParams_t, payload,
              NO_DATA, MAC_CommStatusOrphanIndParams_t, payload)

HOST_TO_STACK(ZBPRO_MAC_PollReq, ZBPRO_MAC_REQ_POLL_FID,
              HAS_CB, MAC_PollReqDescr_t,
              NO_DATA, MAC_PollReqParams_t, payload,
              NO_DATA, MAC_PollConfParams_t, payload)

STACK_TO_HOST(ZBPRO_MAC_DataInd, ZBPRO_MAC_IND_DATA_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, MAC_DataIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_MAC_AssociateInd, ZBPRO_MAC_IND_ASSOCIATE_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, MAC_AssociateIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_MAC_BeaconNotifyInd, ZBPRO_MAC_IND_BEACON_NOTIFY_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, MAC_BeaconNotifyIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_MAC_CommStatusInd, ZBPRO_MAC_IND_COMM_STATUS_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, MAC_CommStatusIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_MAC_OrphanInd, ZBPRO_MAC_IND_ORPHAN_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, MAC_OrphanIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_MAC_PollInd, ZBPRO_MAC_IND_POLL_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, MAC_PollIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

#  endif /* WRAPPERS_ALL */
# endif /* _ZBPRO_ */
#endif /* _MAILBOX_WRAPPERS_MAC_ */

/**********************************************************************************************************************/
/* NWK API                                                                                                            */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_NWK_)
/**** NWK RF4CE *************************************************************************/
# ifdef _RF4CE_
HOST_TO_STACK(RF4CE_NWK_ResetReq, RF4CE_NWK_REQ_RESET_FID,
              HAS_CB, RF4CE_NWK_ResetReqDescr_t,
              NO_DATA, RF4CE_NWK_ResetReqParams_t, payload,
              NO_DATA, RF4CE_NWK_ResetConfParams_t, payload)

HOST_TO_STACK(RF4CE_NWK_StartReq, RF4CE_NWK_REQ_START_FID,
              HAS_CB, RF4CE_NWK_StartReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, RF4CE_NWK_StartConfParams_t, payload)

HOST_TO_STACK(RF4CE_NWK_DataReq, RF4CE_NWK_REQ_DATA_FID,
              HAS_CB, RF4CE_NWK_DataReqDescr_t,
              HAS_DATA, RF4CE_NWK_DataReqParams_t, payload,
              NO_DATA, RF4CE_NWK_DataConfParams_t, payload)

HOST_TO_STACK(RF4CE_NWK_GetReq, RF4CE_NWK_REQ_GET_FID,
              HAS_CB, RF4CE_NWK_GetReqDescr_t,
              NO_DATA, RF4CE_NWK_GetReqParams_t, payload,
              NO_DATA, RF4CE_NWK_GetConfParams_t, payload)

HOST_TO_STACK(RF4CE_NWK_SetReq, RF4CE_NWK_REQ_SET_FID,
              HAS_CB, RF4CE_NWK_SetReqDescr_t,
              NO_DATA, RF4CE_NWK_SetReqParams_t, payload,
              NO_DATA, RF4CE_NWK_SetConfParams_t, payload)

HOST_TO_STACK(RF4CE_NWK_DiscoveryReq, RF4CE_NWK_REQ_DISCOVERY_FID,
              HAS_CB, RF4CE_NWK_DiscoveryReqDescr_t,
              HAS_DATA, RF4CE_NWK_DiscoveryReqParams_t, payload,
              HAS_DATA, RF4CE_NWK_DiscoveryConfParams_t, payload)

#  if defined(RF4CE_TARGET)
HOST_TO_STACK(RF4CE_NWK_DiscoveryResp, RF4CE_NWK_RESP_DISCOVERY_FID,
              NO_CB, RF4CE_NWK_DiscoveryRespDescr_t,
              HAS_DATA, RF4CE_NWK_DiscoveryRespParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

HOST_TO_STACK(RF4CE_NWK_AutoDiscoveryReq, RF4CE_NWK_REQ_AUTODISCOVERY_FID,
              HAS_CB, RF4CE_NWK_AutoDiscoveryReqDescr_t,
              HAS_DATA, RF4CE_NWK_AutoDiscoveryReqParams_t, payload,
              NO_DATA, RF4CE_NWK_AutoDiscoveryConfParams_t, payload)
#  endif /* RF4CE_TARGET */

HOST_TO_STACK(RF4CE_NWK_RXEnableReq, RF4CE_NWK_REQ_RXENABLE_FID,
              HAS_CB, RF4CE_NWK_RXEnableReqDescr_t,
              NO_DATA, RF4CE_NWK_RXEnableReqParams_t, payload,
              NO_DATA, RF4CE_NWK_RXEnableConfParams_t, payload)

HOST_TO_STACK(RF4CE_NWK_UpdateKeyReq, RF4CE_NWK_REQ_UPDATEKEY_FID,
              HAS_CB, RF4CE_NWK_UpdateKeyReqDescr_t,
              NO_DATA, RF4CE_NWK_UpdateKeyReqParams_t, payload,
              NO_DATA, RF4CE_NWK_UpdateKeyConfParams_t, payload)

HOST_TO_STACK(RF4CE_NWK_PairReq, RF4CE_NWK_REQ_PAIR_FID,
              HAS_CB, RF4CE_NWK_PairReqDescr_t,
              HAS_DATA, RF4CE_NWK_PairReqParams_t, payload,
              HAS_DATA, RF4CE_NWK_PairConfParams_t, payload)

#  if defined(RF4CE_TARGET)
HOST_TO_STACK(RF4CE_NWK_PairResp, RF4CE_NWK_RESP_PAIR_FID,
              NO_CB, RF4CE_NWK_PairRespDescr_t,
              HAS_DATA, RF4CE_NWK_PairRespParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)
#  endif /* RF4CE_TARGET */

HOST_TO_STACK(RF4CE_NWK_UnpairReq, RF4CE_NWK_REQ_UNPAIR_FID,
              HAS_CB, RF4CE_NWK_UnpairReqDescr_t,
              NO_DATA, RF4CE_NWK_UnpairReqParams_t, payload,
              NO_DATA, RF4CE_NWK_UnpairConfParams_t, payload)

HOST_TO_STACK(RF4CE_NWK_UnpairResp, RF4CE_NWK_RESP_UNPAIR_FID,
              NO_CB, RF4CE_NWK_UnpairRespDescr_t,
              NO_DATA, RF4CE_NWK_UnpairRespParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

#  if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_NWK_)
STACK_TO_HOST(RF4CE_NWK_DataInd, RF4CE_NWK_IND_DATA_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, RF4CE_NWK_DataIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(RF4CE_NWK_DiscoveryInd, RF4CE_NWK_IND_DISCOVERY_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, RF4CE_NWK_DiscoveryIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(RF4CE_NWK_COMMStatusInd, RF4CE_NWK_IND_COMMSTATUS_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, RF4CE_NWK_COMMStatusIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(RF4CE_NWK_PairInd, RF4CE_NWK_IND_PAIR_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, RF4CE_NWK_PairIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(RF4CE_NWK_UnpairInd, RF4CE_NWK_IND_UNPAIR_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, RF4CE_NWK_UnpairIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

#   ifdef ENABLE_GU_KEY_SEED_IND
STACK_TO_HOST(RF4CE_NWK_KeySeedStartInd, RF4CE_NWK_KEY_SEED_START_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, RF4CE_NWK_KeySeedStartIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

#   endif /* ENABLE_GU_KEY_SEED_IND */
#  endif /* WRAPPERS_ALL */
# endif /* _RF4CE_ */

/**** NWK ZBPRO *************************************************************************/
# ifdef _ZBPRO_
HOST_TO_STACK(ZBPRO_NWK_GetKeyReq, ZBPRO_NWK_REQ_GET_KEY_FID,
              HAS_CB, ZBPRO_NWK_GetKeyReqDescr_t,
              NO_DATA, ZBPRO_NWK_GetKeyReqParams_t, payload,
              NO_DATA, ZBPRO_NWK_GetKeyConfParams_t, payload)

HOST_TO_STACK(ZBPRO_NWK_SetKeyReq, ZBPRO_NWK_REQ_SET_KEY_FID,
              HAS_CB, ZBPRO_NWK_SetKeyReqDescr_t,
              NO_DATA, ZBPRO_NWK_SetKeyReqParams_t, payload,
              NO_DATA, ZBPRO_NWK_SetKeyConfParams_t, payload)

HOST_TO_STACK(ZBPRO_NWK_PermitJoiningReq, ZBPRO_NWK_REQ_PERMIT_JOINING_FID,
              HAS_CB, ZBPRO_NWK_PermitJoiningReqDescr_t,
              NO_DATA, ZBPRO_NWK_PermitJoiningReqParams_t, payload,
              NO_DATA, ZBPRO_NWK_PermitJoiningConfParams_t, payload)

HOST_TO_STACK(ZBPRO_NWK_LeaveReq, ZBPRO_NWK_REQ_LEAVE_FID,
              HAS_CB, ZBPRO_NWK_LeaveReqDescr_t,
              NO_DATA, ZBPRO_NWK_LeaveReqParams_t, payload,
              NO_DATA, ZBPRO_NWK_LeaveConfParams_t, payload)

HOST_TO_STACK(ZBPRO_NWK_RouteDiscoveryReq, ZBPRO_NWK_REQ_ROUTE_DISCOVERY_FID,
              HAS_CB, ZBPRO_NWK_RouteDiscoveryReqDescr_t,
              NO_DATA, ZBPRO_NWK_RouteDiscoveryReqParams_t, payload,
              NO_DATA, ZBPRO_NWK_RouteDiscoveryConfParams_t, payload)

# endif /* _ZBPRO_ */
#endif /* _MAILBOX_WRAPPERS_NWK_ */

/**********************************************************************************************************************/
/* ZBPRO transport API                                                                                                */
/**********************************************************************************************************************/
#ifdef _ZBPRO_
/**** ZBPRO APS *************************************************************************/
# if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_APS_)
HOST_TO_STACK(ZBPRO_APS_GetReq, ZBPRO_APS_REQ_GET_FID,
              HAS_CB, ZBPRO_APS_GetReqDescr_t,
              NO_DATA, ZBPRO_APS_GetReqParams_t, payload,
              HAS_DATA, ZBPRO_APS_GetConfParams_t, payload)

HOST_TO_STACK(ZBPRO_APS_SetReq, ZBPRO_APS_REQ_SET_FID,
              HAS_CB, ZBPRO_APS_SetReqDescr_t,
              HAS_DATA, ZBPRO_APS_SetReqParams_t, payload,
              NO_DATA, ZBPRO_APS_SetConfParams_t, payload)

HOST_TO_STACK(ZBPRO_APS_GetKeyReq, ZBPRO_APS_REQ_GET_KEY_FID,
              HAS_CB, ZBPRO_APS_GetKeyReqDescr_t,
              NO_DATA, ZBPRO_APS_GetKeyReqParams_t, payload,
              NO_DATA, ZBPRO_APS_GetKeyConfParams_t, payload)

HOST_TO_STACK(ZBPRO_APS_SetKeyReq, ZBPRO_APS_REQ_SET_KEY_FID,
              HAS_CB, ZBPRO_APS_SetKeyReqDescr_t,
              NO_DATA, ZBPRO_APS_SetKeyReqParams_t, payload,
              NO_DATA, ZBPRO_APS_SetKeyConfParams_t, payload)

HOST_TO_STACK(ZBPRO_APS_EndpointRegisterReq, ZBPRO_APS_REQ_ENDPOINTREGISTER_FID,
              HAS_CB, ZBPRO_APS_EndpointRegisterReqDescr_t,
              HAS_DATA, ZBPRO_APS_EndpointRegisterReqParams_t, simpleDescriptor.inOutClusterList,
              NO_DATA, ZBPRO_APS_EndpointRegisterConfParams_t, payload)

HOST_TO_STACK(ZBPRO_APS_EndpointUnregisterReq, ZBPRO_APS_REQ_ENDPOINTUNREGISTER_FID,
              HAS_CB, ZBPRO_APS_EndpointUnregisterReqDescr_t,
              NO_DATA, ZBPRO_APS_EndpointUnregisterReqParams_t, payload,
              NO_DATA, ZBPRO_APS_EndpointRegisterConfParams_t, payload)

HOST_TO_STACK(ZBPRO_APS_DataReq, ZBPRO_APS_REQ_DATA_FID,
              HAS_CB, ZBPRO_APS_DataReqDescr_t,
              HAS_DATA, ZBPRO_APS_DataReqParams_t, payload,
              NO_DATA, ZBPRO_APS_DataConfParams_t, payload)

HOST_TO_STACK(ZBPRO_APS_BindReq, ZBPRO_APS_REQ_BIND_FID,
              HAS_CB, ZBPRO_APS_BindUnbindReqDescr_t,
              NO_DATA, ZBPRO_APS_BindUnbindReqParams_t, payload,
              NO_DATA, ZBPRO_APS_BindUnbindConfParams_t, payload)

HOST_TO_STACK(ZBPRO_APS_UnbindReq, ZBPRO_APS_REQ_UNBIND_FID,
              HAS_CB, ZBPRO_APS_BindUnbindReqDescr_t,
              NO_DATA, ZBPRO_APS_BindUnbindReqParams_t, payload,
              NO_DATA, ZBPRO_APS_BindUnbindConfParams_t, payload)

HOST_TO_STACK(ZBPRO_APS_AddGroupReq, ZBPRO_APS_REQ_ADDGROUP_FID,
              HAS_CB, ZBPRO_APS_AddGroupReqDescr_t,
              NO_DATA, ZBPRO_APS_AddGroupReqParams_t, payload,
              NO_DATA, ZBPRO_APS_AddGroupConfParams_t, payload)

HOST_TO_STACK(ZBPRO_APS_RemoveGroupReq, ZBPRO_APS_REQ_REMOVEGROUP_FID,
              HAS_CB, ZBPRO_APS_RemoveGroupReqDescr_t,
              NO_DATA, ZBPRO_APS_RemoveGroupReqParams_t, payload,
              NO_DATA, ZBPRO_APS_RemoveGroupConfParams_t, payload)

HOST_TO_STACK(ZBPRO_APS_RemoveAllGroupsReq, ZBPRO_APS_REQ_REMOVEALLGROUPS_FID,
              HAS_CB, ZBPRO_APS_RemoveAllGroupsReqDescr_t,
              NO_DATA, ZBPRO_APS_RemoveAllGroupsReqParams_t, payload,
              NO_DATA, ZBPRO_APS_RemoveAllGroupsConfParams_t, payload)

STACK_TO_HOST(ZBPRO_APS_DataInd, ZBPRO_APS_IND_DATA_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, ZBPRO_APS_DataIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

HOST_TO_STACK(ZBPRO_APS_TransportKeyReq, ZBPRO_APS_REQ_TRANSPORTKEY_FID,
              HAS_CB, ZBPRO_APS_TransportKeyReqDescr_t,
              NO_DATA, ZBPRO_APS_TransportKeyReqParams_t, payload,
              NO_DATA, ZBPRO_APS_SecurityServicesConfParams_t, payload)

HOST_TO_STACK(ZBPRO_APS_UpdateDeviceReq, ZBPRO_APS_REQ_UPDATEDEVICE_FID,
              HAS_CB, ZBPRO_APS_UpdateDeviceReqDescr_t,
              NO_DATA, ZBPRO_APS_UpdateDeviceReqParams_t, payload,
              NO_DATA, ZBPRO_APS_SecurityServicesConfParams_t, payload)

HOST_TO_STACK(ZBPRO_APS_RemoveDeviceReq, ZBPRO_APS_REQ_REMOTEDEVICE_FID,
              HAS_CB, ZBPRO_APS_RemoveDeviceReqDescr_t,
              NO_DATA, ZBPRO_APS_RemoveDeviceReqParams_t, payload,
              NO_DATA, ZBPRO_APS_SecurityServicesConfParams_t, payload)

HOST_TO_STACK(ZBPRO_APS_RequestKeyReq, ZBPRO_APS_REQ_REQUESTKEY_FID,
              HAS_CB, ZBPRO_APS_RequestKeyReqDescr_t,
              NO_DATA, ZBPRO_APS_RequestKeyReqParams_t, payload,
              NO_DATA, ZBPRO_APS_SecurityServicesConfParams_t, payload)

HOST_TO_STACK(ZBPRO_APS_SwitchKeyReq, ZBPRO_APS_REQ_SWITCHKEY_FID,
              HAS_CB, ZBPRO_APS_SwitchKeyReqDescr_t,
              NO_DATA, ZBPRO_APS_SwitchKeyReqParams_t, payload,
              NO_DATA, ZBPRO_APS_SecurityServicesConfParams_t, payload)

#  if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_APS_)
STACK_TO_HOST(ZBPRO_APS_TransportKeyInd, ZBPRO_APS_IND_TRANSPORTKEY_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_APS_TransportKeyIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_APS_UpdateDeviceInd, ZBPRO_APS_IND_UPDATEDEVICE_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_APS_UpdateDeviceIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_APS_RemoveDeviceInd, ZBPRO_APS_IND_REMOTEDEVICE_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_APS_RemoveDeviceIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_APS_RequestKeyInd, ZBPRO_APS_IND_REQUESTKEY_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_APS_RequestKeyIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_APS_SwitchKeyInd, ZBPRO_APS_IND_SWITCHKEY_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_APS_SwitchKeyIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

#  endif /* WRAPPERS_ALL */
# endif /* _MAILBOX_WRAPPERS_APS_ */

/**** ZBPRO ZDO *************************************************************************/
# if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZDO_)
HOST_TO_STACK(ZBPRO_ZDO_AddrResolvingReq, ZBPRO_ZDO_REQ_ADDR_RESOLVING_FID,
              HAS_CB, ZBPRO_ZDO_AddrResolvingReqDescr_t,
              NO_DATA, ZBPRO_ZDO_AddrResolvingReqParams_t, payload,
              HAS_DATA, ZBPRO_ZDO_AddrResolvingConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZDO_NodeDescReq, ZBPRO_ZDO_REQ_NODE_DESC_FID,
              HAS_CB, ZBPRO_ZDO_NodeDescReqDescr_t,
              NO_DATA, ZBPRO_ZDO_NodeDescReqParams_t, payload,
              NO_DATA, ZBPRO_ZDO_NodeDescConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZDO_PowerDescReq, ZBPRO_ZDO_REQ_POWER_DESC_FID,
              HAS_CB, ZBPRO_ZDO_PowerDescReqDescr_t,
              NO_DATA, ZBPRO_ZDO_PowerDescReqParams_t, payload,
              NO_DATA, ZBPRO_ZDO_PowerDescConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZDO_SimpleDescReq, ZBPRO_ZDO_REQ_SIMPLE_DESC_FID,
              HAS_CB, ZBPRO_ZDO_SimpleDescReqDescr_t,
              NO_DATA, ZBPRO_ZDO_SimpleDescReqParams_t, payload,
              HAS_DATA, ZBPRO_ZDO_SimpleDescConfParams_t, simpleDescriptor.inOutClusterList)

HOST_TO_STACK(ZBPRO_ZDO_ActiveEpReq, ZBPRO_ZDO_REQ_ACTIVE_EP_FID,
              HAS_CB, ZBPRO_ZDO_ActiveEpReqDescr_t,
              NO_DATA, ZBPRO_ZDO_ActiveEpReqParams_t, payload,
              HAS_DATA, ZBPRO_ZDO_ActiveEpConfParams_t, activeEpList)

HOST_TO_STACK(ZBPRO_ZDO_MatchDescReq, ZBPRO_ZDO_REQ_MATCH_DESC_FID,
              HAS_CB, ZBPRO_ZDO_MatchDescReqDescr_t,
              HAS_DATA, ZBPRO_ZDO_MatchDescReqParams_t, inOutClusterList,
              HAS_DATA, ZBPRO_ZDO_MatchDescConfParams_t, responseList)

HOST_TO_STACK(ZBPRO_ZDO_DeviceAnnceReq, ZBPRO_ZDO_REQ_DEVICE_ANNCE_FID,
              HAS_CB, ZBPRO_ZDO_DeviceAnnceReqDescr_t,
              NO_DATA, ZBPRO_ZDO_DeviceAnnceReqParams_t, payload,
              NO_DATA, ZBPRO_ZDO_DeviceAnnceConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZDO_ServerDiscoveryReq, ZBPRO_ZDO_REQ_SERVER_DISCOVERY_FID,
              HAS_CB, ZBPRO_ZDO_ServerDiscoveryReqDescr_t,
              NO_DATA, ZBPRO_ZDO_ServerDiscoveryReqParams_t, payload,
              HAS_DATA, ZBPRO_ZDO_ServerDiscoveryConfParams_t, serverList)

HOST_TO_STACK(ZBPRO_ZDO_EndDeviceBindReq, ZBPRO_ZDO_REQ_ED_BIND_FID,
              HAS_CB, ZBPRO_ZDO_EndDeviceBindReqDescr_t,
              HAS_DATA, ZBPRO_ZDO_EndDeviceBindReqParams_t, clusterList,
              NO_DATA, ZBPRO_ZDO_BindConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZDO_BindReq, ZBPRO_ZDO_REQ_BIND_FID,
              HAS_CB, ZBPRO_ZDO_BindUnbindReqDescr_t,
              NO_DATA, ZBPRO_ZDO_BindUnbindReqParams_t, payload,
              NO_DATA, ZBPRO_ZDO_BindConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZDO_UnbindReq, ZBPRO_ZDO_REQ_UNBIND_FID,
              HAS_CB, ZBPRO_ZDO_BindUnbindReqDescr_t,
              NO_DATA, ZBPRO_ZDO_BindUnbindReqParams_t, payload,
              NO_DATA, ZBPRO_ZDO_BindConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZDO_StartNetworkReq, ZBPRO_ZDO_REQ_START_NETWORK_FID,
              HAS_CB, ZBPRO_ZDO_StartNetworkReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, ZBPRO_ZDO_StartNetworkConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZDO_MgmtLeaveReq, ZBPRO_ZDO_REQ_MGMT_LEAVE_FID,
              HAS_CB, ZBPRO_ZDO_MgmtLeaveReqDescr_t,
              NO_DATA, ZBPRO_ZDO_MgmtLeaveReqParams_t, payload,
              NO_DATA, ZBPRO_ZDO_MgmtLeaveConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZDO_MgmtPermitJoiningReq, ZBPRO_ZDO_REQ_MGMT_PERMIT_JOINING_FID,
              HAS_CB, ZBPRO_ZDO_MgmtPermitJoiningReqDescr_t,
              NO_DATA, ZBPRO_ZDO_MgmtPermitJoiningReqParams_t, payload,
              NO_DATA, ZBPRO_ZDO_MgmtPermitJoiningConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZDO_MgmtBindReq, ZBPRO_ZDO_REQ_MGMT_BIND_FID,
              HAS_CB, ZBPRO_ZDO_MgmtBindReqDescr_t,
              NO_DATA, ZBPRO_ZDO_MgmtBindReqParams_t, payload,
              HAS_DATA, ZBPRO_ZDO_MgmtBindConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZDO_MgmtLqiReq, ZBPRO_ZDO_REQ_MGMT_LQI_FID,
              HAS_CB, ZBPRO_ZDO_MgmtLqiReqDescr_t,
              NO_DATA, ZBPRO_ZDO_MgmtLqiReqParams_t, payload,
              HAS_DATA, ZBPRO_ZDO_MgmtLqiConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZDO_MgmtNwkUpdateReq, ZBPRO_ZDO_REQ_MGMT_NWK_UPDATE_FID,
              HAS_CB, ZBPRO_ZDO_MgmtNwkUpdateReqDescr_t,
              NO_DATA, ZBPRO_ZDO_MgmtNwkUpdateReqParams_t, payload,
              HAS_DATA, ZBPRO_ZDO_MgmtNwkUpdateConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZDO_MgmtNwkUpdateUnsolResp, ZBPRO_ZDO_RESP_MGMT_NWK_UPDATE_UNSOL_FID,
              HAS_CB, ZBPRO_ZDO_MgmtNwkUpdateUnsolRespDescr_t,
              HAS_DATA, ZBPRO_ZDO_MgmtNwkUpdateUnsolRespParams_t, payload,
              NO_DATA, ZBPRO_ZDO_MgmtNwkUpdateUnsolConfParams_t, payload)

#  if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_ZDO_)
STACK_TO_HOST(ZBPRO_ZDO_MgmtNwkUpdateUnsolInd, ZBPRO_ZDO_IND_MGMT_NWK_UPDATE_UNSOL_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, ZBPRO_ZDO_MgmtNwkUpdateUnsolIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_ZDO_DeviceAnnceInd, ZBPRO_ZDO_IND_DEVICE_ANNCE_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_ZDO_DeviceAnnceIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_ZDO_ReadyInd, ZBPRO_ZDO_IND_READY_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_ZDO_ReadyIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

#  endif /* WRAPPERS_ALL */
# endif /* _MAILBOX_WRAPPERS_ZDO_ */

/**** ZBPRO TC **************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_TC_)
HOST_TO_STACK(ZBPRO_TC_NwkKeyUpdateReq, ZBPRO_TC_REQ_NWK_KEY_UPDATE_FID,
              HAS_CB, ZBPRO_TC_NwkKeyUpdateReqDescr_t,
              NO_DATA, ZBPRO_TC_NwkKeyUpdateReqParams_t, payload,
              NO_DATA, ZBPRO_TC_NwkKeyUpdateConfParams_t, payload)

#endif /* _MAILBOX_WRAPPERS_TC_ */

/**** ZBPRO ZCL *************************************************************************/
# if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_)
/**** wide commands ************************/
HOST_TO_STACK(ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReq, ZBPRO_ZCL_PROFILE_WIDE_CMD_DISCOVER_ATTRIBUTE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ProfileWideCmdDiscoverAttrDescr_t,
              NO_DATA, ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReqParams_t, payload,
              HAS_DATA, ZBPRO_ZCL_ProfileWideCmdDiscoverAttrConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_ProfileWideCmdReadAttributesReq, ZBPRO_ZCL_PROFILE_WIDE_CMD_READ_ATTRIBUTE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t,
              NO_DATA, ZBPRO_ZCL_ProfileWideCmdReadAttrReqParams_t, payload,
              HAS_DATA, ZBPRO_ZCL_ProfileWideCmdReadAttrConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_ProfileWideCmdWriteAttributesReq, ZBPRO_ZCL_PROFILE_WIDE_CMD_WRITE_ATTRIBUTE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t,
              HAS_DATA, ZBPRO_ZCL_ProfileWideCmdWriteAttrReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_ProfileWideCmdWriteAttrConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_ProfileWideCmdConfigureReportingReq, ZBPRO_ZCL_PROFILE_WIDE_CMD_CONFIGURE_REPORTING_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t,
              HAS_DATA, ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_ProfileWideCmdConfigureReportingConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReq, ZBPRO_ZCL_PROFILE_WIDE_CMD_READ_REPORTING_CONFIGURATION_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqDescr_t,
              NO_DATA, ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqParams_t, payload,
              HAS_DATA, ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationConfParams_t, payload)

STACK_TO_HOST(ZBPRO_ZCL_ProfileWideCmdReportAttributesInd, ZBPRO_ZCL_PROFILE_WIDE_CMD_REPORT_ATTRIBUTES_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, ZBPRO_ZCL_ProfileWideCmdReportAttributesIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)


#  if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_ZCL_)
/**** basic cluster  ********************/
HOST_TO_STACK(ZBPRO_ZCL_SetPowerSourceReq, ZBPRO_ZCL_REQ_SET_POWER_SOURCE_FID,
              HAS_CB, ZBPRO_ZCL_SetPowerSourceReqDescr_t,
              NO_DATA, ZBPRO_ZCL_SetPowerSourceReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_SetPowerSourceConfParams_t, payload)


/**** identify cluster  ********************/
HOST_TO_STACK(ZBPRO_ZCL_IdentifyCmdIdentifyReq, ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_REQ_FID,
              HAS_CB, ZBPRO_ZCL_IdentifyCmdIdentifyReqDescr_t,
              NO_DATA, ZBPRO_ZCL_IdentifyCmdIdentifyReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_IdentifyCmdConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_IdentifyCmdIdentifyQueryReq, ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_REQ_FID,
              HAS_CB, ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqDescr_t,
              NO_DATA, ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_IdentifyCmdConfParams_t, payload)

#   ifdef ZBPRO_ZHA_USE_0XEB_ENDPOINT_FOR_ZHA_CERT_TEST_543
STACK_TO_HOST(ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseIndEB, ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_RESPONSE_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)
#   endif

#   if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_IDENTIFY_RELAY_)
HOST_TO_STACK(ZBPRO_ZCL_IdentifyCmdIdentifyResponseReq, ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_RESPONSE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_IdentifyCmdIdentifyResponseReqDescr_t,
              NO_DATA, ZBPRO_ZCL_IdentifyCmdIdentifyResponseReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_IdentifyCmdConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReq, ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_RESPONSE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReqDescr_t,
              NO_DATA, ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_IdentifyCmdConfParams_t, payload)

STACK_TO_HOST(ZBPRO_ZCL_IdentifyCmdIdentifyInd, ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_ZCL_IdentifyCmdIdentifyIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_ZCL_IdentifyCmdIdentifyQueryInd, ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_ZCL_IdentifyCmdIdentifyQueryIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

#   else
STACK_TO_HOST(ZBPRO_ZCL_IdentifyInd, ZBPRO_ZCL_IND_IDENTIFY_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_ZCL_IdentifyIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)
#   endif

/**** group cluster  ***********************/
HOST_TO_STACK(ZBPRO_ZCL_GroupsCmdAddGroupReq, ZBPRO_ZCL_GROUPS_CMD_ADD_GROUP_REQ_FID,
              HAS_CB, ZBPRO_ZCL_GroupsCmdAddGroupReqDescr_t,
              HAS_DATA, ZBPRO_ZCL_GroupsCmdAddGroupReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_GroupsCmdAddGroupConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_GroupsCmdViewGroupReq, ZBPRO_ZCL_GROUPS_CMD_VIEW_GROUP_REQ_FID,
              HAS_CB, ZBPRO_ZCL_GroupsCmdViewGroupReqDescr_t,
              NO_DATA, ZBPRO_ZCL_GroupsCmdViewGroupReqParams_t, payload,
              HAS_DATA, ZBPRO_ZCL_GroupsCmdViewGroupConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_GroupsCmdGetGroupMembershipReq, ZBPRO_ZCL_GROUPS_CMD_GET_GROUP_MEMBERSHIP_REQ_FID,
              HAS_CB, ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqDescr_t,
              HAS_DATA, ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_GroupsCmdGetGroupMembershipConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_GroupsCmdRemoveGroupReq, ZBPRO_ZCL_GROUPS_CMD_REMOVE_GROUP_REQ_FID,
              HAS_CB, ZBPRO_ZCL_GroupsCmdRemoveGroupReqDescr_t,
              NO_DATA, ZBPRO_ZCL_GroupsCmdRemoveGroupReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_GroupsCmdRemoveGroupConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReq, ZBPRO_ZCL_GROUPS_CMD_REMOVE_ALL_GROUPS_REQ_FID,
              HAS_CB, ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqDescr_t,
              NO_DATA, ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_GroupsCmdRemoveAllGroupsConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReq, ZBPRO_ZCL_GROUPS_CMD_ADD_GROUP_IF_IDENTIFY_REQ_FID,
              HAS_CB, ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqDescr_t,
              HAS_DATA, ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyConfParams_t, payload)

STACK_TO_HOST(ZBPRO_ZCL_GroupsCmdGetGroupMembershipResponseInd, ZBPRO_ZCL_GROUPS_CMD_GET_GROUP_MEMBERSHIP_RESPONSE_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, ZBPRO_ZCL_GroupsCmdGetGroupMembershipIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)


/**** scene cluster  ***********************/
HOST_TO_STACK(ZBPRO_ZCL_ScenesCmdAddSceneReq, ZBPRO_ZCL_SCENES_CMD_ADD_SCENE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ScenesCmdAddSceneReqDescr_t,
              HAS_DATA, ZBPRO_ZCL_ScenesCmdAddSceneReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_ScenesCmdAddSceneConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_ScenesCmdViewSceneReq, ZBPRO_ZCL_SCENES_CMD_VIEW_SCENE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ScenesCmdViewSceneReqDescr_t,
              NO_DATA, ZBPRO_ZCL_ScenesCmdViewSceneReqParams_t, payload,
              HAS_DATA, ZBPRO_ZCL_ScenesCmdViewSceneConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_ScenesCmdStoreSceneReq, ZBPRO_ZCL_SCENES_CMD_STORE_SCENE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ScenesCmdStoreSceneReqDescr_t,
              NO_DATA, ZBPRO_ZCL_ScenesCmdStoreSceneReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_ScenesCmdStoreSceneConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_ScenesCmdRecallSceneReq, ZBPRO_ZCL_SCENES_CMD_RECALL_SCENE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ScenesCmdRecallSceneReqDescr_t,
              NO_DATA, ZBPRO_ZCL_ScenesCmdRecallSceneReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_ScenesCmdConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_ScenesCmdRemoveSceneReq, ZBPRO_ZCL_SCENES_CMD_REMOVE_SCENE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ScenesCmdRemoveSceneReqDescr_t,
              NO_DATA, ZBPRO_ZCL_ScenesCmdRemoveSceneReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_ScenesCmdRemoveSceneConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_ScenesCmdRemoveAllScenesReq, ZBPRO_ZCL_SCENES_CMD_REMOVE_ALL_SCENES_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqDescr_t,
              NO_DATA, ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_ScenesCmdRemoveAllScenesConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_ScenesCmdGetSceneMembershipReq, ZBPRO_ZCL_SCENES_CMD_GET_SCENE_MEMBERSHIP_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqDescr_t,
              NO_DATA, ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_ScenesCmdConfParams_t, payload)

STACK_TO_HOST(ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseInd, ZBPRO_ZCL_SCENES_CMD_GET_SCENE_MEMBERSHIP_RESPONSE_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)


/**** on/off cluster  **********************/
HOST_TO_STACK(ZBPRO_ZCL_OnOffCmdOffReq, ZBPRO_ZCL_ONOFF_CMD_OFF_REQ_FID,
              HAS_CB, ZBPRO_ZCL_OnOffCmdReqDescr_t,
              NO_DATA, ZBPRO_ZCL_OnOffCmdReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_OnOffCmdConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_OnOffCmdOnReq, ZBPRO_ZCL_ONOFF_CMD_ON_REQ_FID,
              HAS_CB, ZBPRO_ZCL_OnOffCmdReqDescr_t,
              NO_DATA, ZBPRO_ZCL_OnOffCmdReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_OnOffCmdConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_OnOffCmdToggleReq, ZBPRO_ZCL_ONOFF_CMD_TOGGLE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_OnOffCmdReqDescr_t,
              NO_DATA, ZBPRO_ZCL_OnOffCmdReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_OnOffCmdConfParams_t, payload)


/**** level control cluster  ***************/
HOST_TO_STACK(ZBPRO_ZCL_LevelControlCmdMoveToLevelReq, ZBPRO_ZCL_LEVEL_CONTROL_CMD_MOVE_TO_LEVEL_REQ_FID,
              HAS_CB, ZBPRO_ZCL_LevelControlCmdMoveToLevelReqDescr_t,
              NO_DATA, ZBPRO_ZCL_LevelControlCmdMoveToLevelReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_LevelControlCmdConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_LevelControlCmdMoveReq, ZBPRO_ZCL_LEVEL_CONTROL_CMD_MOVE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_LevelControlCmdMoveReqDescr_t,
              NO_DATA, ZBPRO_ZCL_LevelControlCmdMoveReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_LevelControlCmdConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_LevelControlCmdStepReq, ZBPRO_ZCL_LEVEL_CONTROL_CMD_STEP_REQ_FID,
              HAS_CB, ZBPRO_ZCL_LevelControlCmdStepReqDescr_t,
              NO_DATA, ZBPRO_ZCL_LevelControlCmdStepReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_LevelControlCmdConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_LevelControlCmdStopReq, ZBPRO_ZCL_LEVEL_CONTROL_CMD_STOP_REQ_FID,
              HAS_CB, ZBPRO_ZCL_LevelControlCmdStopReqDescr_t,
              NO_DATA, ZBPRO_ZCL_LevelControlCmdStopReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_LevelControlCmdConfParams_t, payload)


/**** door lock cluster  *******************/
HOST_TO_STACK(ZBPRO_ZCL_DoorLockCmdLockReq, ZBPRO_ZCL_DOOR_LOCK_CMD_LOCK_REQ_FID,
              HAS_CB, ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t,
              HAS_DATA, ZBPRO_ZCL_DoorLockCmdLockUnlockReqParams_t, codeString,
              NO_DATA, ZBPRO_ZCL_DoorLockCmdLockUnlockConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_DoorLockCmdUnlockReq, ZBPRO_ZCL_DOOR_LOCK_CMD_UNLOCK_REQ_FID,
              HAS_CB, ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t,
              HAS_DATA, ZBPRO_ZCL_DoorLockCmdLockUnlockReqParams_t, codeString,
              NO_DATA, ZBPRO_ZCL_DoorLockCmdLockUnlockConfParams_t, payload)

/**** window covering cluster  *************/
HOST_TO_STACK(ZBPRO_ZCL_WindowCoveringCmdUpOpenReq, ZBPRO_ZCL_WINDOW_COVERING_CMD_UP_OPEN_REQ_FID,
              HAS_CB, ZBPRO_ZCL_WindowCoveringCmdReqDescr_t,
              NO_DATA, ZBPRO_ZCL_WindowCoveringCmdReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_WindowCoveringCmdConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_WindowCoveringCmdDownCloseReq, ZBPRO_ZCL_WINDOW_COVERING_CMD_DOWN_CLOSE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_WindowCoveringCmdReqDescr_t,
              NO_DATA, ZBPRO_ZCL_WindowCoveringCmdReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_WindowCoveringCmdConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_WindowCoveringCmdStopReq, ZBPRO_ZCL_WINDOW_COVERING_CMD_STOP_REQ_FID,
              HAS_CB, ZBPRO_ZCL_WindowCoveringCmdReqDescr_t,
              NO_DATA, ZBPRO_ZCL_WindowCoveringCmdReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_WindowCoveringCmdConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_WindowCoveringCmdGotoLiftPecentageReq, ZBPRO_ZCL_WINDOW_COVERING_CMD_GO_TO_LIFT_PERCENTAGE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t,
              NO_DATA, ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_WindowCoveringCmdConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_WindowCoveringCmdGotoTiltPecentageReq, ZBPRO_ZCL_WINDOW_COVERING_CMD_GO_TO_TILT_PERCENTAGE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t,
              NO_DATA, ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_WindowCoveringCmdConfParams_t, payload)


/**** ias zone cluster  ********************/
HOST_TO_STACK(ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReq, ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_ENROLL_RESPONSE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqDescr_t,
              NO_DATA, ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseConfParams_t, payload)

STACK_TO_HOST(ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestInd, ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_ENROLL_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationInd, ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_STATUS_CHANGED_NOTIFICATION_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)


/**** ias ace cluster  *********************/
HOST_TO_STACK(ZBPRO_ZCL_SapIasAceArmRespReq, ZBPRO_ZCL_IAS_ACE_ARM_RESP_REQ_FID,
              HAS_CB, ZBPRO_ZCL_SapIasAceArmRespReqDescr_t,
              NO_DATA, ZBPRO_ZCL_SapIasAceArmRespReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_SapIasAceBypassRespReq, ZBPRO_ZCL_IAS_ACE_BYPASS_RESP_REQ_FID,
              HAS_CB, ZBPRO_ZCL_SapIasAceBypassRespReqDescr_t,
              HAS_DATA, ZBPRO_ZCL_SapIasAceBypassRespReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReq, ZBPRO_ZCL_IAS_ACE_GET_ZONE_ID_MAP_RESP_REQ_FID,
              HAS_CB, ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqDescr_t,
              NO_DATA, ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_SapIasAceGetZoneInfoRespReq, ZBPRO_ZCL_IAS_ACE_GET_ZONE_INFO_RESP_REQ_FID,
              HAS_CB, ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqDescr_t,
              HAS_DATA, ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_SapIasAceGetPanelStatusRespReq, ZBPRO_ZCL_IAS_ACE_GET_PANEL_STATUS_RESP_REQ_FID,
              HAS_CB, ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqDescr_t,
              NO_DATA, ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReq, ZBPRO_ZCL_IAS_ACE_SET_BYPASSED_ZONE_LIST_RESP_REQ_FID,
              HAS_CB, ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqDescr_t,
              HAS_DATA, ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_SapIasAceGetZoneStatusRespReq, ZBPRO_ZCL_IAS_ACE_GET_ZONE_STATUS_RESP_REQ_FID,
              HAS_CB, ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqDescr_t,
              HAS_DATA, ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_SapIasAceZoneStatusChangedReq, ZBPRO_ZCL_IAS_ACE_ZONE_STATUS_CHANGED_REQ_FID,
              HAS_CB, ZBPRO_ZCL_SapIasAceZoneStatusChangedReqDescr_t,
              HAS_DATA, ZBPRO_ZCL_SapIasAceZoneStatusChangedReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_SapIasAcePanelStatusChangedReq, ZBPRO_ZCL_IAS_ACE_PANEL_STATUS_CHANGED_REQ_FID,
              HAS_CB, ZBPRO_ZCL_SapIasAcePanelStatusChangedReqDescr_t,
              NO_DATA, ZBPRO_ZCL_SapIasAcePanelStatusChangedReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, payload)

STACK_TO_HOST(ZBPRO_ZCL_SapIasAceArmInd, ZBPRO_ZCL_IAS_ACE_ARM_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, ZBPRO_ZCL_SapIasAceArmIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_ZCL_SapIasAceBypassInd, ZBPRO_ZCL_IAS_ACE_BYPASS_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, ZBPRO_ZCL_SapIasAceBypassIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_ZCL_SapIasAceEmergencyInd, ZBPRO_ZCL_IAS_ACE_EMERGENCY_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_ZCL_SapIasAceAlarmIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_ZCL_SapIasAceFireInd, ZBPRO_ZCL_IAS_ACE_FIRE_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_ZCL_SapIasAceAlarmIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_ZCL_SapIasAcePanicInd, ZBPRO_ZCL_IAS_ACE_PANIC_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_ZCL_SapIasAceAlarmIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_ZCL_SapIasAceGetZoneIdMapInd, ZBPRO_ZCL_IAS_ACE_GET_ZONE_ID_MAP_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_ZCL_SapIasAceGetZoneIdMapIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_ZCL_SapIasAceGetZoneInfoInd, ZBPRO_ZCL_IAS_ACE_GET_ZONE_INFO_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_ZCL_SapIasAceGetZoneInfoIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_ZCL_SapIasAceGetPanelStatusInd, ZBPRO_ZCL_IAS_ACE_GET_PANEL_STATUS_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_ZCL_SapIasAceGetPanelStatusIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_ZCL_SapIasAceGetBypassedZoneListInd, ZBPRO_ZCL_IAS_ACE_GET_BYPASSED_ZONE_LIST_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_ZCL_SapIasAceGetBypassedZoneListIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_ZCL_SapIasAceGetZoneStatusInd, ZBPRO_ZCL_IAS_ACE_GET_ZONE_STATUS_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_ZCL_SapIasAceGetZoneStatusIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)


/**** ias wd cluster  **********************/
HOST_TO_STACK(ZBPRO_ZCL_IASWDCmdStartWarningReq, ZBPRO_ZCL_IAS_WD_CMD_START_WARNING_REQ_FID,
              HAS_CB, ZBPRO_ZCL_IASWDCmdStartWarningReqDescr_t,
              NO_DATA, ZBPRO_ZCL_IASWDCmdStartWarningReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_IASWDCmdStartWarningConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_IASWDCmdSquawkReq, ZBPRO_ZCL_IAS_WD_CMD_SQUAWK_REQ_FID,
              HAS_CB, ZBPRO_ZCL_IASWDCmdSquawkReqDescr_t,
              NO_DATA, ZBPRO_ZCL_IASWDCmdSquawkReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_IASWDCmdSquawkConfParams_t, payload)


/**** color control cluster ****************/
HOST_TO_STACK(ZBPRO_ZCL_ColorControlCmdMoveToColorReq, ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_TO_COLOR_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ColorControlCmdMoveToColorReqDescr_t,
              NO_DATA, ZBPRO_ZCL_ColorControlCmdMoveToColorReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_ColorControlCmdMoveToColorConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_ColorControlCmdMoveColorReq, ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_COLOR_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ColorControlCmdMoveColorReqDescr_t,
              NO_DATA, ZBPRO_ZCL_ColorControlCmdMoveColorReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_ColorControlCmdMoveColorConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_ColorControlCmdStepColorReq, ZBPRO_ZCL_COLOR_CONTROL_CMD_STEP_COLOR_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ColorControlCmdStepColorReqDescr_t,
              NO_DATA, ZBPRO_ZCL_ColorControlCmdStepColorReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_ColorControlCmdStepColorConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReq, ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_TO_HUE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqDescr_t,
              NO_DATA, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReq, ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_HUE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqDescr_t,
              NO_DATA, ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReq, ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_STEP_HUE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqDescr_t,
              NO_DATA, ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_ColorControlCmdEnhancedStepHueConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReq, ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_TO_HUE_AND_SATURATION_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqDescr_t,
              NO_DATA, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_ColorControlCmdColorLoopSetReq, ZBPRO_ZCL_COLOR_CONTROL_CMD_COLOR_LOOP_SET_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ColorControlCmdColorLoopSetReqDescr_t,
              NO_DATA, ZBPRO_ZCL_ColorControlCmdColorLoopSetReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_ColorControlCmdColorLoopSetConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_ColorControlCmdStopMoveStepReq, ZBPRO_ZCL_COLOR_CONTROL_CMD_STOP_MOVE_STEP_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ColorControlCmdStopMoveStepReqDescr_t,
              NO_DATA, ZBPRO_ZCL_ColorControlCmdStopMoveStepReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_ColorControlCmdStopMoveStepConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReq, ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_COLOR_TEMPERATURE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqDescr_t,
              NO_DATA, ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReq, ZBPRO_ZCL_COLOR_CONTROL_CMD_STEP_COLOR_TEMPERATURE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqDescr_t,
              NO_DATA, ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_ColorControlCmdStepColorTemperatureConfParams_t, payload)

/**** OTA Upgrade cluster ****************/
STACK_TO_HOST(ZBPRO_ZCL_OTAUCmdQueryNextImageRequestInd, ZBPRO_ZCL_OTAU_CMD_QUERY_NEXT_IMAGE_REQUEST_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_ZCL_OTAUCmdQueryNextImageRequestIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

HOST_TO_STACK(ZBPRO_ZCL_OTAUCmdQueryNextImageResponseReq, ZBPRO_ZCL_OTAU_CMD_QUERY_NEXT_IMAGE_RESPONSE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_OTAUCmdQueryNextImageResponseReqDescr_t,
              NO_DATA, ZBPRO_ZCL_OTAUCmdQueryNextImageResponseReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_OTAUCmdQueryNextImageResponseConfParams_t, payload)

STACK_TO_HOST(ZBPRO_ZCL_OTAUCmdImageBlockRequestInd, ZBPRO_ZCL_OTAU_CMD_IMAGE_BLOCK_REQUEST_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_ZCL_OTAUCmdImageBlockRequestIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

HOST_TO_STACK(ZBPRO_ZCL_OTAUCmdImageBlockResponseReq, ZBPRO_ZCL_OTAU_CMD_IMAGE_BLOCK_RESPONSE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_OTAUCmdImageBlockResponseReqDescr_t,
              HAS_DATA, ZBPRO_ZCL_OTAUCmdImageBlockResponseReqParams_t, payload,
              NO_DATA, ZBPRO_ZCL_OTAUCmdImageBlockResponseConfParams_t, payload)

STACK_TO_HOST(ZBPRO_ZCL_OTAUCmdUpgradeEndRequestInd, ZBPRO_ZCL_OTAU_CMD_UPGRADE_END_REQUEST_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_ZCL_OTAUCmdUpgradeEndRequestIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

HOST_TO_STACK(ZBPRO_ZCL_OTAUCmdUpgradeEndResponseReq, ZBPRO_ZCL_OTAU_CMD_UPGRADE_END_RESPONSE_REQ_FID,
              HAS_CB, ZBPRO_ZCL_OTAUCmdUpgradeEndResponseReqDescr_t,
              NO_DATA, ZBPRO_ZCL_OTAUCmdUpgradeEndResponseParams_t, payload,
              NO_DATA, ZBPRO_ZCL_OTAUCmdUpgradeEndResponseConfParams_t, payload)

#  endif /* WRAPPERS_ALL */
# endif /* _MAILBOX_WRAPPERS_ZCL_ */
#endif /* _ZBPRO_ */

/**********************************************************************************************************************/
/* PROFILES API                                                                                                       */
/**********************************************************************************************************************/
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_PROFILE_)
/**** RF4CE Profiles ********************************************************************/
# ifdef _RF4CE_
/**** RF4CE PROFILE MANAGER ****************/
HOST_TO_STACK(RF4CE_UnpairReq, RF4CE_PROFILE_REQ_UNPAIR_FID,
              HAS_CB, RF4CE_UnpairReqDescr_t,
              NO_DATA, RF4CE_UnpairReqParams_t, payload,
              NO_DATA, RF4CE_UnpairConfParams_t, payload)

HOST_TO_STACK(RF4CE_StartReq, RF4CE_PROFILE_REQ_START_FID,
              HAS_CB, RF4CE_StartReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, RF4CE_StartResetConfParams_t, payload)

HOST_TO_STACK(RF4CE_ResetReq, RF4CE_PROFILE_REQ_RESET_FID,
              HAS_CB, RF4CE_ResetReqDescr_t,
              NO_DATA, RF4CE_ResetReqParams_t, payload,
              NO_DATA, RF4CE_StartResetConfParams_t, payload)

HOST_TO_STACK(RF4CE_SetSupportedDevicesReq, RF4CE_PROFILE_REQ_SET_SUP_DEVICES_FID,
              HAS_CB, RF4CE_SetSupportedDevicesReqDescr_t,
              NO_DATA, RF4CE_SetSupportedDevicesReqParams_t, payload,
              NO_DATA, RF4CE_SetSupportedDevicesConfParams_t, payload)

#  if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_PROFILE_)
STACK_TO_HOST(RF4CE_CounterExpiredInd, RF4CE_PROFILE_IND_COUNTER_EXPIRED_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, RF4CE_PairingReferenceIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(RF4CE_UnpairInd, RF4CE_PROFILE_IND_UNPAIR_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, RF4CE_PairingReferenceIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

#   if defined(RF4CE_TARGET)
STACK_TO_HOST(RF4CE_PairInd, RF4CE_PROFILE_IND_PAIR_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, RF4CE_PairingIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)
#   endif /* RF4CE_TARGET */
#  endif /* WRAPPERS_ALL */

/**** RF4CE ZRC *****************************/
#  if (1 == USE_RF4CE_PROFILE_ZRC)

/**** RF4CE ZRC 1.1 *************************/
#   ifdef USE_RF4CE_PROFILE_ZRC1
HOST_TO_STACK(RF4CE_ZRC1_GetAttributesReq, RF4CE_ZRC1_REQ_GET_FID,
              HAS_CB, RF4CE_ZRC1_GetAttributeDescr_t,
              NO_DATA, RF4CE_ZRC1_GetAttributeReqParams_t, payload,
              NO_DATA, RF4CE_ZRC1_GetAttributeConfParams_t, payload)

HOST_TO_STACK(RF4CE_ZRC1_SetAttributesReq, RF4CE_ZRC1_REQ_SET_FID,
              HAS_CB, RF4CE_ZRC1_SetAttributeDescr_t,
              NO_DATA, RF4CE_ZRC1_SetAttributeReqParams_t, payload,
              NO_DATA, RF4CE_ZRC1_SetAttributeConfParams_t, payload)

HOST_TO_STACK(RF4CE_ZRC1_CommandDiscoveryReq, RF4CE_ZRC1_REQ_COMMANDDISCOVERY_FID,
              HAS_CB, RF4CE_ZRC1_CommandDiscoveryReqDescr_t,
              NO_DATA, RF4CE_ZRC1_CommandDiscoveryReqParams_t, payload,
              NO_DATA, RF4CE_ZRC1_CommandDiscoveryConfParams_t, payload)

HOST_TO_STACK(RF4CE_ZRC1_ControlCommandPressedReq, RF4CE_ZRC1_CONTROL_COMMAND_PRESSED_FID,
              HAS_CB, RF4CE_ZRC1_ControlCommandReqDescr_t,
              HAS_DATA, RF4CE_ZRC1_ControlCommandReqParams_t, payload,
              NO_DATA, RF4CE_ZRC_ControlCommandConfParams_t, payload)

HOST_TO_STACK(RF4CE_ZRC1_ControlCommandReleasedReq, RF4CE_ZRC1_CONTROL_COMMAND_RELEASED_FID,
              HAS_CB, RF4CE_ZRC1_ControlCommandReqDescr_t,
              HAS_DATA, RF4CE_ZRC1_ControlCommandReqParams_t, payload,
              NO_DATA, RF4CE_ZRC_ControlCommandConfParams_t, payload)

HOST_TO_STACK(RF4CE_ZRC1_ControllerBindReq, RF4CE_ZRC1_REQ_CONTROLLER_BIND_FID,
              HAS_CB, RF4CE_ZRC1_BindReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, RF4CE_ZRC1_BindConfParams_t, payload)

HOST_TO_STACK(RF4CE_ZRC1_VendorSpecificReq, RF4CE_ZRC1_VENDORSPECIFIC_REQ_FID,
              HAS_CB, RF4CE_ZRC1_VendorSpecificReqDescr_t,
              HAS_DATA, RF4CE_ZRC1_VendorSpecificReqParams_t, payload,
              NO_DATA, RF4CE_ZRC1_VendorSpecificConfParams_t, payload)

#    if defined(RF4CE_TARGET)
HOST_TO_STACK(RF4CE_ZRC1_TargetBindReq, RF4CE_ZRC1_REQ_TARGET_BIND_FID,
              HAS_CB, RF4CE_ZRC1_BindReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, RF4CE_ZRC1_BindConfParams_t, payload)

#    endif /* RF4CE_TARGET */
#    if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_PROFILE_)
STACK_TO_HOST(RF4CE_ZRC1_VendorSpecificInd, RF4CE_ZRC1_VENDORSPECIFIC_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, RF4CE_ZRC1_VendorSpecificIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

#     if defined(RF4CE_TARGET)
STACK_TO_HOST(RF4CE_ZRC1_ControlCommandInd, RF4CE_ZRC1_IND_CONTROLCOMMAND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, RF4CE_ZRC1_ControlCommandIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

#     endif /* RF4CE_TARGET */
#    endif /* WRAPPERS_ALL */
#   endif /* USE_RF4CE_PROFILE_ZRC1 */

/**** RF4CE GDP 2.0 *************************/
#   ifdef USE_RF4CE_PROFILE_ZRC2
HOST_TO_STACK(RF4CE_GDP2_SetPollConstraintsReq, RF4CE_GDP2_REQ_SET_POLL_CONSTRAINTS_FID,
              HAS_CB, RF4CE_GDP2_SetPollConstraintsReqDescr_t,
              NO_DATA, RF4CE_GDP2_SetPollConstraintsReqParams_t, payload,
              NO_DATA, RF4CE_GDP2_SetPollConstraintsConfParams_t, payload)

HOST_TO_STACK(RF4CE_GDP2_PollNegotiationReq, RF4CE_GDP2_REQ_POLL_NEGOTIATION_FID,
              HAS_CB, RF4CE_GDP2_PollNegotiationReqDescr_t,
              NO_DATA, RF4CE_GDP2_PollNegotiationReqParams_t, payload,
              NO_DATA, RF4CE_GDP2_PollNegotiationConfParams_t, payload)

HOST_TO_STACK(RF4CE_GDP2_PollClientUserEventReq, RF4CE_GDP2_REQ_POLL_CLIENT_USER_EVENT_FID,
              HAS_CB, RF4CE_GDP2_PollClientUserEventReqDescr_t,
              NO_DATA, RF4CE_GDP2_PollClientUserEventReqParams_t, payload,
              NO_DATA, RF4CE_GDP2_PollClientUserEventConfParams_t, payload)

HOST_TO_STACK(RF4CE_GDP2_ClientNotificationReq, RF4CE_GDP2_REQ_CLIENT_NOTIFICATION_FID,
              HAS_CB, RF4CE_GDP2_ClientNotificationReqDescr_t,
              HAS_DATA, RF4CE_GDP2_ClientNotificationReqParams_t, payload,
              NO_DATA, RF4CE_GDP2_ClientNotificationConfParams_t, payload)

HOST_TO_STACK(RF4CE_GDP2_IdentifyCapAnnounceReq, RF4CE_GDP2_REQ_IDENTIFY_CAP_ANNOUNCE_FID,
              HAS_CB, RF4CE_GDP2_IdentifyCapAnnounceReqDescr_t,
              NO_DATA, RF4CE_GDP2_IdentifyCapAnnounceReqParams_t, payload,
              NO_DATA, RF4CE_GDP2_IdentifyCapAnnounceConfParams_t, payload)

HOST_TO_STACK(RF4CE_GDP2_IdentifyReq, RF4CE_GDP2_REQ_IDENTIFY_FID,
              HAS_CB, RF4CE_GDP2_IdentifyReqDescr_t,
              NO_DATA, RF4CE_GDP2_IdentifyReqParams_t, payload,
              NO_DATA, RF4CE_GDP2_IdentifyConfParams_t, payload)

#    if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_PROFILE_)
STACK_TO_HOST(RF4CE_GDP2_PollNegotiationInd, RF4CE_GDP2_IND_POLL_NEGOTIATION_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, RF4CE_GDP2_PollNegotiationIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(RF4CE_GDP2_HeartbeatInd, RF4CE_GDP2_IND_HEARTBEAT_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, RF4CE_GDP2_HeartbeatIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(RF4CE_GDP2_ClientNotificationInd, RF4CE_GDP2_IND_CLIENT_NOTIFICATION_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, RF4CE_GDP2_ClientNotificationIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(RF4CE_GDP2_IdentifyCapAnnounceInd, RF4CE_GDP2_IND_IDENTIFY_CAP_ANNOUNCE_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, RF4CE_GDP2_IdentifyCapAnnounceIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(RF4CE_GDP2_IdentifyInd, RF4CE_GDP2_IND_IDENTIFY_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, RF4CE_GDP2_IdentifyIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

#    endif /* WRAPPERS_ALL */
#   endif /* USE_RF4CE_PROFILE_ZRC2 */

/**** RF4CE ZRC 2.0 *************************/
#   ifdef USE_RF4CE_PROFILE_ZRC2
HOST_TO_STACK(RF4CE_ZRC2_GetAttributesReq, RF4CE_ZRC2_REQ_GET_FID,
              HAS_CB, RF4CE_ZRC2_GetAttributesReqDescr_t,
              HAS_DATA, RF4CE_ZRC2_GetAttributesReqParams_t, payload,
              HAS_DATA, RF4CE_ZRC2_GetAttributesConfParams_t, payload)

HOST_TO_STACK(RF4CE_ZRC2_SetAttributesReq, RF4CE_ZRC2_REQ_SET_FID,
              HAS_CB, RF4CE_ZRC2_SetAttributesReqDescr_t,
              HAS_DATA, RF4CE_ZRC2_SetAttributesReqParams_t, payload,
              NO_DATA, RF4CE_ZRC2_SetAttributesConfParams_t, payload)

HOST_TO_STACK(RF4CE_ZRC2_KeyExchangeReq, RF4CE_ZRC2_KEY_EXCHANGE_FID,
              HAS_CB, RF4CE_ZRC2_KeyExchangeReqDescr_t,
              NO_DATA, RF4CE_ZRC2_KeyExchangeReqParams_t, payload,
              NO_DATA, RF4CE_ZRC2_KeyExchangeConfParams_t, payload)

HOST_TO_STACK(RF4CE_ZRC2_BindReq, RF4CE_ZRC2_BIND_FID,
              HAS_CB, RF4CE_ZRC2_BindReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, RF4CE_ZRC2_BindConfParams_t, payload)

HOST_TO_STACK(RF4CE_ZRC2_ProxyBindReq, RF4CE_ZRC2_PROXY_BIND_FID,
              HAS_CB, RF4CE_ZRC2_ProxyBindReqDescr_t,
              NO_DATA, RF4CE_ZRC2_ProxyBindReqParams_t, payload,
              NO_DATA, RF4CE_ZRC2_BindConfParams_t, payload)

HOST_TO_STACK(RF4CE_ZRC2_EnableBindingReq, RF4CE_ZRC2_ENABLE_BINDING_FID,
              HAS_CB, RF4CE_ZRC2_BindingReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, RF4CE_ZRC2_BindingConfParams_t, payload)

HOST_TO_STACK(RF4CE_ZRC2_DisableBindingReq, RF4CE_ZRC2_DISABLE_BINDING_FID,
              HAS_CB, RF4CE_ZRC2_BindingReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, RF4CE_ZRC2_BindingConfParams_t, payload)

HOST_TO_STACK(RF4CE_ZRC2_SetPushButtonStimulusReq, RF4CE_ZRC2_SET_PUSH_BUTTON_STIMULUS_FID,
              HAS_CB, RF4CE_ZRC2_ButtonBindingReqDescr_t,
              NO_DATA, RF4CE_ZRC2_ButtonBindingReqParams_t, payload,
              NO_DATA, RF4CE_ZRC2_BindingConfParams_t, payload)

HOST_TO_STACK(RF4CE_ZRC2_ClearPushButtonStimulusReq, RF4CE_ZRC2_CLEAR_PUSH_BUTTON_STIMULUS_FID,
              HAS_CB, RF4CE_ZRC2_ButtonBindingReqDescr_t,
              NO_DATA, RF4CE_ZRC2_ButtonBindingReqParams_t, payload,
              NO_DATA, RF4CE_ZRC2_BindingConfParams_t, payload)

HOST_TO_STACK(RF4CE_ZRC2_CheckValidationResp, RF4CE_ZRC2_CHECK_VALIDATION_RESP_FID,
              NO_CB, RF4CE_ZRC2_CheckValidationRespDescr_t,
              NO_DATA, RF4CE_ZRC2_CheckValidationRespParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

HOST_TO_STACK(RF4CE_ZRC2_ControlCommandPressedReq, RF4CE_ZRC2_CONTROL_COMMAND_PRESS_REQ_FID,
              HAS_CB, RF4CE_ZRC2_ControlCommandReqDescr_t,
              HAS_DATA, RF4CE_ZRC2_ControlCommandReqParams_t, payload,
              NO_DATA, RF4CE_ZRC2_ControlCommandConfParams_t, payload)

HOST_TO_STACK(RF4CE_ZRC2_ControlCommandReleasedReq, RF4CE_ZRC2_CONTROL_COMMAND_RELEASE_REQ_FID,
              HAS_CB, RF4CE_ZRC2_ControlCommandReqDescr_t,
              HAS_DATA, RF4CE_ZRC2_ControlCommandReqParams_t, payload,
              NO_DATA, RF4CE_ZRC2_ControlCommandConfParams_t, payload)

#    if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_PROFILE_)
STACK_TO_HOST(RF4CE_ZRC2_StartValidationInd, RF4CE_ZRC2_START_VALIDATION_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, RF4CE_ZRC2_CheckValidationIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(RF4CE_ZRC2_CheckValidationInd, RF4CE_ZRC2_CHECK_VALIDATION_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, RF4CE_ZRC2_CheckValidationIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(RF4CE_ZRC2_ControlCommandInd, RF4CE_ZRC2_CONTROL_COMMAND_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, RF4CE_ZRC2_ControlCommandIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(RF4CE_ZRC2_GetSharedSecretInd, RF4CE_ZRC2_GET_SHARED_SECRET_IND_FID,
              HAS_CB, RF4CE_ZRC2_GetSharedSecretIndDescr_t,
              NO_DATA, RF4CE_ZRC2_GetSharedSecretIndParams_t, payload,
              NO_DATA, RF4CE_ZRC2_GetSharedSecretRespParams_t, payload)

STACK_TO_HOST(RF4CE_ZRC2_PairNtfyInd, RF4CE_ZRC2_PAIR_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, RF4CE_PairingIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(RF4CE_ZRC2_BindingFinishedNtfyInd, RF4CE_ZRC2_BINDING_FINISHED_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, RF4CE_ZRC2_BindingFinishedNtfyIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(RF4CE_ZRC2_GetAttrRespInd, RF4CE_ZRC2_IND_GET_RESP_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, RF4CE_ZRC2_SetAttributesReqParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(RF4CE_ZRC2_PushAttrReqInd, RF4CE_ZRC2_IND_PUSH_REQ_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, RF4CE_ZRC2_SetAttributesReqParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

#    endif /* WRAPPERS_ALL */
#   endif /* USE_RF4CE_PROFILE_ZRC2 */
#  endif /* USE_RF4CE_PROFILE_ZRC */

/**** RF4CE MSO ****************************/
#  if (1 == USE_RF4CE_PROFILE_MSO)
HOST_TO_STACK(RF4CE_MSO_GetProfileAttributeReq, RF4CE_MSO_GET_PROFILE_ATTRIBUTE_FID,
              HAS_CB, RF4CE_MSO_GetProfileAttributeReqDescr_t,
              NO_DATA, RF4CE_MSO_GetProfileAttributeReqParams_t, payload,
              NO_DATA, RF4CE_MSO_GetProfileAttributeConfParams_t, payload)

HOST_TO_STACK(RF4CE_MSO_SetProfileAttributeReq, RF4CE_MSO_SET_PROFILE_ATTRIBUTE_FID,
              HAS_CB, RF4CE_MSO_SetProfileAttributeReqDescr_t,
              NO_DATA, RF4CE_MSO_SetProfileAttributeReqParams_t, payload,
              NO_DATA, RF4CE_MSO_SetProfileAttributeConfParams_t, payload)

HOST_TO_STACK(RF4CE_MSO_GetRIBAttributeReq, RF4CE_MSO_GET_RIB_ATTRIBUTE_FID,
              HAS_CB, RF4CE_MSO_GetRIBAttributeReqDescr_t,
              NO_DATA, RF4CE_MSO_GetRIBAttributeReqParams_t, payload,
              HAS_DATA, RF4CE_MSO_GetRIBAttributeConfParams_t, payload)

HOST_TO_STACK(RF4CE_MSO_SetRIBAttributeReq, RF4CE_MSO_SET_RIB_ATTRIBUTE_FID,
              HAS_CB, RF4CE_MSO_SetRIBAttributeReqDescr_t,
              HAS_DATA, RF4CE_MSO_SetRIBAttributeReqParams_t, payload,
              NO_DATA, RF4CE_MSO_SetRIBAttributeConfParams_t, payload)

#   if defined(RF4CE_TARGET)
HOST_TO_STACK(RF4CE_MSO_WatchDogKickOrValidateReq, RF4CE_MSO_VALIDATE_RESP_FID,
              NO_CB, RF4CE_MSO_WatchDogKickOrValidateReqDescr_t,
              NO_DATA, RF4CE_MSO_WatchDogKickOrValidateReqParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

#   endif /* RF4CE_TARGET */
HOST_TO_STACK(RF4CE_MSO_BindReq, RF4CE_MSO_BIND_FID,
              HAS_CB, RF4CE_MSO_BindReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, RF4CE_MSO_BindConfParams_t, payload)

HOST_TO_STACK(RF4CE_MSO_UserControlPressedReq, RF4CE_MSO_USER_CONTROL_PRESSED_FID,
              HAS_CB, RF4CE_MSO_UserControlReqDescr_t,
              NO_DATA, RF4CE_MSO_UserControlReqParams_t, payload,
              NO_DATA, RF4CE_MSO_UserControlConfParams_t, payload)

HOST_TO_STACK(RF4CE_MSO_UserControlReleasedReq, RF4CE_MSO_USER_CONTROL_RELEASED_FID,
              HAS_CB, RF4CE_MSO_UserControlReqDescr_t,
              NO_DATA, RF4CE_MSO_UserControlReqParams_t, payload,
              NO_DATA, RF4CE_MSO_UserControlConfParams_t, payload)

#   if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_PROFILE_)
#    if defined(RF4CE_TARGET)
STACK_TO_HOST(RF4CE_MSO_CheckValidationInd, RF4CE_MSO_CHECK_VALIDATION_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, RF4CE_MSO_CheckValidationIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(RF4CE_MSO_UserControlInd, RF4CE_MSO_USER_CONTROL_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, RF4CE_MSO_UserControlIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

#    endif /* RF4CE_TARGET */
STACK_TO_HOST(RF4CE_MSO_StartValidationInd, RF4CE_MSO_START_VALIDATION_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, RF4CE_PairingReferenceIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

#   endif /* WRAPPERS_ALL */
#  endif /* USE_RF4CE_PROFILE_MSO */
# endif /* _RF4CE_ */

/**** ZBPRO Profiles ********************************************************************/
# ifdef _ZBPRO_
/**** ZBPRO ZHA ****************************/
#  if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZHA_)
HOST_TO_STACK(ZBPRO_ZHA_EzModeReq, ZBPRO_ZHA_EZ_MODE_FID,
              HAS_CB, ZBPRO_ZHA_EzModeReqDescr_t,
              NO_DATA, ZBPRO_ZHA_EzModeReqParams_t, payload,
              NO_DATA, ZBPRO_ZHA_EzModeConfParams_t, payload)

#   ifdef _ZHA_PROFILE_CIE_DEVICE_IMPLEMENTATION_
HOST_TO_STACK(ZBPRO_ZHA_CieDeviceRegisterReq, ZBPRO_ZHA_CIE_REGISTER_DEVICE_REQ_FID,
              HAS_CB, ZBPRO_ZHA_CieDeviceRegisterReqDescr_t,
              NO_DATA, ZBPRO_ZHA_CieDeviceRegisterReqParams_t, payload,
              NO_DATA, ZBPRO_ZHA_CieDeviceRegisterConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZHA_CieDeviceUnregisterReq, ZBPRO_ZHA_CIE_UNREGISTER_DEVICE_REQ_FID,
              HAS_CB, ZBPRO_ZHA_CieDeviceUnregisterReqDescr_t,
              NO_DATA, ZBPRO_ZHA_CieDeviceUnregisterReqParams_t, payload,
              NO_DATA, ZBPRO_ZHA_CieDeviceUnregisterConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZHA_CieDeviceEnrollReq, ZBPRO_ZHA_CIE_ENROLL_FID,
              HAS_CB, ZBPRO_ZHA_CieEnrollReqDescr_t,
              NO_DATA, ZBPRO_ZHA_CieEnrollReqParams_t, payload,
              NO_DATA, ZBPRO_ZHA_CieEnrollConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZHA_CieDeviceSetPanelStatusReq, ZBPRO_ZHA_CIE_SET_PANEL_STATUS_REQ_FID,
              HAS_CB, ZBPRO_ZHA_CieSetPanelStatusReqDescr_t,
              NO_DATA, ZBPRO_ZHA_CieSetPanelStatusReqParams_t, payload,
              NO_DATA, ZBPRO_ZHA_CieSetPanelStatusConfParams_t, payload)

HOST_TO_STACK(ZBPRO_ZHA_CieZoneSetBypassStateReq, ZBPRO_ZHA_CIE_ZONE_SET_BYPASS_STATE_REQ_FID,
              HAS_CB, ZBPRO_ZHA_CieZoneSetBypassStateReqDescr_t,
              NO_DATA, ZBPRO_ZHA_CieZoneSetBypassStateReqParams_t, payload,
              NO_DATA, ZBPRO_ZHA_CieZoneSetBypassStateConfParams_t, payload)

#    if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_ZHA_)
STACK_TO_HOST(ZBPRO_ZHA_CieDeviceSetPanelStatusInd, ZBPRO_ZHA_CIE_SET_PANEL_STATUS_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_ZHA_CieSetPanelStatusIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

STACK_TO_HOST(ZBPRO_ZHA_CieDeviceEnrollInd, ZBPRO_ZHA_CIE_ENROLL_IND_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, ZBPRO_ZHA_CieEnrollIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)

#    endif /* WRAPPERS_ALL */
#   endif /* _ZHA_PROFILE_CIE_DEVICE_IMPLEMENTATION_ */
#  endif /* _MAILBOX_WRAPPERS_ZHA_ */
# endif /* _ZBPRO_ */
#endif /* _MAILBOX_WRAPPERS_PROFILE_ */

/**********************************************************************************************************************/
/* Addition API                                                                                                       */
/**********************************************************************************************************************/
#ifdef _RF4CE_
/**** Power filter control **************************************************************/
HOST_TO_STACK(RF4CE_ZRC_GetWakeUpActionCodeReq, RF4CE_ZRC_GET_WAKEUP_ACTION_CODE_FID,
              HAS_CB, RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, RF4CE_ZRC_GetWakeUpActionCodeConfParams_t, payload)

HOST_TO_STACK(RF4CE_ZRC_SetWakeUpActionCodeReq, RF4CE_ZRC_SET_WAKEUP_ACTION_CODE_FID,
              HAS_CB, RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t,
              NO_DATA, RF4CE_ZRC_SetWakeUpActionCodeReqParams_t, payload,
              NO_DATA, RF4CE_ZRC_SetWakeUpActionCodeConfParams_t, payload)
#endif

/**** PHY test interface ****************************************************************/
#if defined(_PHY_TEST_HOST_INTERFACE_)
HOST_TO_STACK(Phy_Test_Get_Caps_Req, RF4CE_CTRL_TEST_GET_CAPS_FID,
              HAS_CB, Phy_Test_Get_Caps_ReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, Phy_Test_Get_Caps_ConfParams_t, payload)

HOST_TO_STACK(Phy_Test_Set_Channel_Req, RF4CE_CTRL_TEST_SET_CHANNEL_FID,
              HAS_CB, Phy_Test_Set_Channel_ReqDescr_t,
              NO_DATA, Phy_Test_Set_Channel_ReqParams_t, payload,
              NO_DATA, Phy_Test_Set_Channel_ConfParams_t, payload)

HOST_TO_STACK(Phy_Test_Continuous_Wave_Start_Req, RF4CE_CTRL_TEST_CONTINUOUS_WAVE_START_FID,
              HAS_CB, Phy_Test_Continuous_Wave_Start_ReqDescr_t,
              NO_DATA, Phy_Test_Continuous_Wave_Start_ReqParams_t, payload,
              NO_DATA, Phy_Test_Continuous_Wave_StartStop_ConfParams_t, payload)

HOST_TO_STACK(Phy_Test_Continuous_Wave_Stop_Req, RF4CE_CTRL_TEST_CONTINUOUS_WAVE_STOP_FID,
              HAS_CB, Phy_Test_Continuous_Wave_Stop_ReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, Phy_Test_Continuous_Wave_StartStop_ConfParams_t, payload)

HOST_TO_STACK(Phy_Test_Transmit_Start_Req, RF4CE_CTRL_TEST_TRANSMIT_START_FID,
              HAS_CB, Phy_Test_Transmit_Start_ReqDescr_t,
              NO_DATA, Phy_Test_Transmit_Start_ReqParams_t, renamed_payload_field_just_to_avoid_error,
              NO_DATA, Phy_Test_Transmit_StartStop_ConfParams_t, payload)

HOST_TO_STACK(Phy_Test_Transmit_Stop_Req, RF4CE_CTRL_TEST_TRANSMIT_STOP_FID,
              HAS_CB, Phy_Test_Transmit_Stop_ReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, Phy_Test_Transmit_StartStop_ConfParams_t, payload)

HOST_TO_STACK(Phy_Test_Receive_Start_Req, RF4CE_CTRL_TEST_RECEIVE_START_FID,
              HAS_CB, Phy_Test_Receive_Start_ReqDescr_t,
              NO_DATA, Phy_Test_Receive_Start_ReqParams_t, payload,
              NO_DATA, Phy_Test_Receive_StartStop_ConfParams_t, payload)

HOST_TO_STACK(Phy_Test_Receive_Stop_Req, RF4CE_CTRL_TEST_RECEIVE_STOP_FID,
              HAS_CB, Phy_Test_Receive_Stop_ReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, Phy_Test_Receive_StartStop_ConfParams_t, payload)

HOST_TO_STACK(Phy_Test_Echo_Start_Req, RF4CE_CTRL_TEST_ECHO_START_FID,
              HAS_CB, Phy_Test_Echo_Start_ReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, Phy_Test_Echo_StartStop_ConfParams_t, payload)

HOST_TO_STACK(Phy_Test_Echo_Stop_Req, RF4CE_CTRL_TEST_ECHO_STOP_FID,
              HAS_CB, Phy_Test_Echo_Stop_ReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, Phy_Test_Echo_StartStop_ConfParams_t, payload)

HOST_TO_STACK(Phy_Test_Energy_Detect_Scan_Req, RF4CE_CTRL_TEST_ENERGY_DETECT_SCAN_FID,
              HAS_CB, Phy_Test_Energy_Detect_Scan_ReqDescr_t,
              NO_DATA, Phy_Test_Energy_Detect_Scan_ReqParams_t, payload,
              HAS_DATA, Phy_Test_Energy_Detect_Scan_ConfParams_t, payload)

HOST_TO_STACK(Phy_Test_Get_Stats_Req, RF4CE_CTRL_TEST_GET_STATS_FID,
              HAS_CB, Phy_Test_Get_Stats_ReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, Phy_Test_Get_Stats_ConfParams_t, payload)

HOST_TO_STACK(Phy_Test_Reset_Stats_Req, RF4CE_CTRL_TEST_RESET_STATS_FID,
              HAS_CB, Phy_Test_Reset_Stats_ReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, Phy_Test_Reset_Stats_ConfParams_t, payload)

HOST_TO_STACK(Phy_Test_Set_TX_Power_Req, RF4CE_CTRL_TEST_SET_TX_POWER_FID,
              HAS_CB, Phy_Test_Set_TX_Power_ReqDescr_t,
              NO_DATA, Phy_Test_Set_TX_Power_ReqParams_t, payload,
              NO_DATA, Phy_Test_Set_TX_Power_ConfParams_t, payload)

HOST_TO_STACK(Phy_Test_SelectAntenna_Req, RF4CE_CTRL_TEST_SELECT_ANTENNA_FID,
              HAS_CB, Phy_Test_Select_Antenna_ReqDescr_t,
              NO_DATA, Phy_Test_Select_Antenna_ReqParams_t, payload,
              NO_DATA, Phy_Test_Select_Antenna_ConfParams_t, payload)

HOST_TO_STACK(RF4CE_Get_Diag_Caps_Req, RF4CE_CTRL_GET_DIAGNOSTICS_CAPS_FID,
              HAS_CB, RF4CE_Diag_Caps_ReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, RF4CE_Diag_Caps_ConfParams_t, payload)

HOST_TO_STACK(RF4CE_Get_Diag_Req, RF4CE_CTRL_GET_DIAGNOSTIC_FID,
              HAS_CB, RF4CE_Diag_ReqDescr_t,
              NO_DATA, RF4CE_Diag_ReqParams_t, payload,
              NO_DATA, RF4CE_Diag_ConfParams_t, payload)

# if defined(_RF4CE_)
STACK_TO_HOST(RF4CE_MAC_DataInd_Stub, RF4CE_MAC_IND_DATA_STUB_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              HAS_DATA, MAC_DataIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)
# endif /* _RF4CE_ */

#endif /* _PHY_TEST_HOST_INTERFACE_ */


HOST_TO_STACK(TE_RoutingChangeReq, TE_ROUTING_CHANGE_REQ_FID,
              HAS_CB, TE_RoutingChangeReqDescr_t,
              NO_DATA, TE_RoutingChangeReqParams_t, payload,
              NO_DATA, TE_RoutingChangeConfParams_t, payload)

// To get the Firmware Revision numbers.
HOST_TO_STACK(Get_FW_Rev_Req, GET_FW_REV_FID,
              HAS_CB, Get_FW_Rev_ReqDescr_t,
              NO_PARAMS, NO_APPROPRIATE_TYPE, payload,
              NO_DATA, Get_FW_Rev_ConfParams_t, payload)


// NOTE: TE_ASSERT_ERRID_FID must be on the last position.
ANY_DIRECTION(Mail_TestEngineHaltInd, TE_ASSERT_ERRID_FID,
              NO_CB, NO_APPROPRIATE_TYPE,
              NO_DATA, TE_AssertLogIdCommandIndParams_t, payload,
              NO_CONFIRM, NO_APPROPRIATE_TYPE, payload)




#undef STACK_TO_HOST
#undef HOST_TO_STACK
#undef ANY_DIRECTION

/* eof bbMailFunctionList.h */