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
* FILENAME: $Workfile: trunk/stack/ZbPro/ZCL/include/bbZbProZclSapClusterDoorLock.h $
*
* DESCRIPTION:
*   ZCL Door Lock cluster SAP interface.
*
* $Revision: 7517 $
* $Date: 2015-07-15 18:34:05Z $
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZCL_SAP_CLUSTER_DOOR_LOCK_H
#define _BB_ZBPRO_ZCL_SAP_CLUSTER_DOOR_LOCK_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZclCommon.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of attributes of Door Lock ZCL cluster Server side.
 */
typedef enum _ZBPRO_ZCL_SapDoorLockServerAttributeId_t
{
    /* 0x0000 – 0x000F Basic Information Attribute Set */
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_LOCK_STATE                                      = 0x0000, // M
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_LOCK_TYPE                                       = 0x0001, // M
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_ACTUATOR_ENABLED                                = 0x0002, // M
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_DOOR_STATE                                      = 0x0003,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_DOOR_OPEN_EVENTS                                = 0x0004,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_DOOR_CLOSED_EVENTS                              = 0x0005,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_OPEN_PERIOD                                     = 0x0006,

    /* 0x0010 – 0x001F User, PIN, Schedule Information Attribute Set */
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_NUMBER_OF_LOG_RECORDS_SUPPORTED                 = 0x0010,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_NUMBER_OF_TOTAL_USERS_SUPPORTED                 = 0x0011,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_NUMBER_OF_PIN_USERS_SUPPORTED                   = 0x0012,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_NUMBER_OF_RFID_USERS_SUPPORTED                  = 0x0013,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_NUMBER_OF_WEEK_DAY_SCHEDULES_SUPPORTED_PER_USER = 0x0014,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_NUMBER_OF_YEAR_DAY_SCHEDULES_SUPPORTED_PER_USER = 0x0015,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_NUMBER_OF_HOLIDAY_SCHEDULES_SUPPORTED           = 0x0016,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_MAX_PIN_CODE_LENGTH                             = 0x0017,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_MIN_PIN_CODE_LENGTH                             = 0x0018,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_MAX_RFID_CODE_LENGTH                            = 0x0019,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_MIN_RFID_CODE_LENGTH                            = 0x001A,

    /* 0x0020 – 0x002F Operational Settings Attribute Set */
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_ENABLE_LOGGING                                  = 0x0020,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_LANGUAGE                                        = 0x0021,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_LED_SETTINGS                                    = 0x0022,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_AUTO_RELOCK_TIME                                = 0x0023,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_SOUND_VOLUME                                    = 0x0024,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_OPERATING_MODE                                  = 0x0025,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_SUPPORTED_OPERATING_MODES                       = 0x0026,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_DEFAULT_CONFIGURATION_REGISTER                  = 0x0027,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_ENABLE_LOCAL_PROGRAMMING                        = 0x0028,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_ENABLE_ONE_TOUCH_LOCKING                        = 0x0029,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_ENABLE_INSIDE_STATUS_LED                        = 0x002A,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_ENABLE_PRIVACY_MODE_BUTTON                      = 0x002B,

    /* 0x0030 – 0x003F Security Settings Attribute Set */
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_WRONG_CODE_ENTRY_LIMIT                          = 0x0030,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_USER_CODE_TEMPORARY_DISABLE_TIME                = 0x0031,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_SEND_PIN_OVER_THE_AIR                           = 0x0032,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_REQUIRE_PIN_FOR_RF_OPERATION                    = 0x0033,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_ZIGBEE_SECURITY_LEVEL                           = 0x0034,

    /* 0x0040 – 0x004F Alarm and Event Masks Attribute Set */
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_ALARM_MASK                                      = 0x0040,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_KEYPAD_OPERATION_EVENT_MASK                     = 0x0041,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_RF_OPERATION_EVENT_MASK                         = 0x0042,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_MANUAL_OPERATION_EVENT_MASK                     = 0x0043,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_RFID_OPERATION_EVENT_MASK                       = 0x0044,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_KEYPAD_PROGRAMMING_EVENT_MASK                   = 0x0045,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_RF_PROGRAMMING_EVENT_MASK                       = 0x0046,
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_RFID_PROGRAMMING_EVENT_MASK                     = 0x0047,

    /* 0x8000 – 0xFFFF Reserved for vendor-specific attributes */
    ZBPRO_ZCL_SAP_DOOR_LOCK_ATTR_ID_MAX              = 0xFFFF,       /*!< Introduced only to make the enumeration 16-bit
                                                                        wide. */
} ZBPRO_ZCL_SapDoorLockServerAttributeId_t;


/**//**
 * \brief   Door Lock clustrt attribute types declarations.
 */
typedef uint8_t     ZBPRO_ZCL_SapDoorLockAttrLockState_t;
typedef uint8_t     ZBPRO_ZCL_SapDoorLockAttrLockType_t;
typedef Bool8_t     ZBPRO_ZCL_SapDoorLockAttrActuatorEnabled_t;
typedef uint8_t     ZBPRO_ZCL_SapDoorLockAttrDoorState_t;
typedef uint32_t    ZBPRO_ZCL_SapDoorLockAttrDoorOpenEvents_t;
typedef uint32_t    ZBPRO_ZCL_SapDoorLockAttrDoorClosedEvents_t;
typedef uint16_t    ZBPRO_ZCL_SapDoorLockAttrOpenPeriod_t;

typedef uint16_t    ZBPRO_ZCL_SapDoorLockAttrNumberOfLogRecordsSupported_t;
typedef uint16_t    ZBPRO_ZCL_SapDoorLockAttrNumberOfTotalUsersSupported_t;
typedef uint16_t    ZBPRO_ZCL_SapDoorLockAttrNumberOfPINUsersSupported_t;
typedef uint16_t    ZBPRO_ZCL_SapDoorLockAttrNumberOfRFIDUsersSupported_t;
typedef uint8_t     ZBPRO_ZCL_SapDoorLockAttrNumberOfWeekDaySchedulesSupportedPerUser_t;
typedef uint8_t     ZBPRO_ZCL_SapDoorLockAttrNumberOfYearDaySchedulesSupportedPerUser_t;
typedef uint8_t     ZBPRO_ZCL_SapDoorLockAttrNumberOfYearDaySchedulesSupportedPerUser_t;
typedef uint8_t     ZBPRO_ZCL_SapDoorLockAttrMaxPINCodeLength_t;
typedef uint8_t     ZBPRO_ZCL_SapDoorLockAttrMinPINCodeLength_t;
typedef uint8_t     ZBPRO_ZCL_SapDoorLockAttrMaxRFIDCodeLength_t;
typedef uint8_t     ZBPRO_ZCL_SapDoorLockAttrMinRFIDCodeLength_t;

typedef Bool8_t     ZBPRO_ZCL_SapDoorLockAttrEnableLogging_t;
typedef uint32_t    ZBPRO_ZCL_SapDoorLockAttrLanguage_t;
typedef uint8_t     ZBPRO_ZCL_SapDoorLockAttrLEDSettings_t;
typedef uint32_t    ZBPRO_ZCL_SapDoorLockAttrAutoRelockTime_t;
typedef uint8_t     ZBPRO_ZCL_SapDoorLockAttrSoundVolume_t;
typedef uint8_t     ZBPRO_ZCL_SapDoorLockAttrOperatingMode_t;
typedef uint16_t    ZBPRO_ZCL_SapDoorLockAttrSupportedOperatingModes_t;
typedef uint16_t    ZBPRO_ZCL_SapDoorLockAttrDefaultConfigurationRegister_t;
typedef Bool8_t     ZBPRO_ZCL_SapDoorLockAttrEnableLocalProgramming_t;
typedef Bool8_t     ZBPRO_ZCL_SapDoorLockAttrEnableOneTouchLocking_t;
typedef Bool8_t     ZBPRO_ZCL_SapDoorLockAttrEnableInsideStatusLED_t;
typedef Bool8_t     ZBPRO_ZCL_SapDoorLockAttrEnablePrivacyModeButton_t;

typedef uint8_t     ZBPRO_ZCL_SapDoorLockAttrWrongCodeEntryLimit_t;
typedef uint8_t     ZBPRO_ZCL_SapDoorLockAttrUserCodeTemporaryDisableTime_t;
typedef Bool8_t     ZBPRO_ZCL_SapDoorLockAttrSendPINOverTheAir_t;
typedef Bool8_t     ZBPRO_ZCL_SapDoorLockAttrRequirePINforRFOperation_t;
typedef uint8_t     ZBPRO_ZCL_SapDoorLockAttrZigBeeSecurityLevel_t;

typedef uint16_t    ZBPRO_ZCL_SapDoorLockAttrAlarmMask_t;
typedef uint16_t    ZBPRO_ZCL_SapDoorLockAttrKeypadOperationEventMask_t;
typedef uint16_t    ZBPRO_ZCL_SapDoorLockAttrRFOperationEventMask_t;
typedef uint16_t    ZBPRO_ZCL_SapDoorLockAttrManualOperationEventMask_t;
typedef uint16_t    ZBPRO_ZCL_SapDoorLockAttrRFIDOperationEventMask_t;
typedef uint16_t    ZBPRO_ZCL_SapDoorLockAttrKeypadProgrammingEventMask_t;
typedef uint16_t    ZBPRO_ZCL_SapDoorLockAttrRFProgrammingEventMask_t;
typedef uint16_t    ZBPRO_ZCL_SapDoorLockAttrRFIDProgrammingEventMask_t;


/**//**                                                                                                                 // TODO: Move to private header.
 * \brief   Enumeration of client-to-server commands specific to Door Lock ZCL cluster.
 */
typedef enum _ZBPRO_ZCL_SapDoorLockClientToServerCommandId_t
{
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_LOCK                     = 0x00,     // M
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_UNLOCK                   = 0x01,     // M
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_TOGGLE                   = 0x02,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_UNLOCK_WITH_TIMEOUT      = 0x03,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_GET_LOG_RECORD           = 0x04,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_SET_PIN_CODE             = 0x05,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_GET_PIN_CODE             = 0x06,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_CLEAR_PIN_CODE           = 0x07,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_CLEAR_ALL_PIN_CODES      = 0x08,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_SET_USER_STATUS          = 0x09,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_GET_USER_STATUS          = 0x0A,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_SET_WEEKDAY_SCHEDULE     = 0x0B,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_GET_WEEKDAY_SCHEDULE     = 0x0C,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_CLEAR_WEEKDAY_SCHEDULE   = 0x0D,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_SET_YEAR_DAY_SCHEDULE    = 0x0E,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_GET_YEAR_DAY_SCHEDULE    = 0x0F,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_CLEAR_YEAR_DAY_SCHEDULE  = 0x10,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_SET_HOLIDAY_SCHEDULE     = 0x11,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_GET_HOLIDAY_SCHEDULE     = 0x12,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_CLEAR_HOLIDAY_SCHEDULE   = 0x13,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_SET_USER_TYPE            = 0x14,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_GET_USER_TYPE            = 0x15,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_SET_RFID_CODE            = 0x16,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_GET_RFID_CODE            = 0x17,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_CLEAR_RFID_CODE          = 0x18,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_CLEAR_ALL_RFID_CODES     = 0x19,
} ZBPRO_ZCL_SapDoorLockClientToServerCommandId_t;


/**//**                                                                                                                 // TODO: Move to private header.
 * \brief   Enumeration of server-to-client commands specific to Door Lock ZCL cluster.
 */
typedef enum _ZBPRO_ZCL_SapDoorLockServerToClientCommandId_t
{
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_LOCK_RESPONSE                    = 0x00,     // M
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_UNLOCK_RESPONSE                  = 0x01,     // M
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_TOGGLE_RESPONSE                  = 0x02,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_UNLOCK_WITH_TIMEOUT_RESPONSE     = 0x03,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_GET_LOG_RECORD_RESPONSE          = 0x04,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_SET_PIN_CODE_RESPONSE            = 0x05,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_GET_PIN_CODE_RESPONSE            = 0x06,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_CLEAR_PIN_CODE_RESPONSE          = 0x07,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_CLEAR_ALL_PIN_CODES_RESPONSE     = 0x08,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_SET_USER_STATUS_RESPONSE         = 0x09,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_GET_USER_STATUS_RESPONSE         = 0x0A,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_SET_WEEKDAY_SCHEDULE_RESPONSE    = 0x0B,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_GET_WEEKDAY_SCHEDULE_RESPONSE    = 0x0C,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_CLEAR_WEEKDAY_SCHEDULE_RESPONSE  = 0x0D,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_SET_YEAR_DAY_SCHEDULE_RESPONSE   = 0x0E,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_GET_YEAR_DAY_SCHEDULE_RESPONSE   = 0x0F,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_CLEAR_YEAR_DAY_SCHEDULE_RESPONSE = 0x10,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_SET_HOLIDAY_SCHEDULE_RESPONSE    = 0x11,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_GET_HOLIDAY_SCHEDULE_RESPONSE    = 0x12,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_CLEAR_HOLIDAY_SCHEDULE_RESPONSE  = 0x13,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_SET_USER_TYPE_RESPONSE           = 0x14,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_GET_USER_TYPE_RESPONSE           = 0x15,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_SET_RFID_CODE_RESPONSE           = 0x16,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_GET_RFID_CODE_RESPONSE           = 0x17,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_CLEAR_RFID_CODE_RESPONSE         = 0x18,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_CLEAR_ALL_RFID_CODES_RESPONSE    = 0x19,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_OPERATING_EVENT_NOTIFICATION     = 0x20,
    ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_PROGRAMMING_EVENT_NOTIFICATION   = 0x21,
} ZBPRO_ZCL_SapDoorLockServerToClientCommandId_t;


/**//**
 * \brief   Lock/unlock command parameters type
 */
typedef struct _ZBPRO_ZCL_DoorLockCmdLockUnlockReqParams_t
{
    ZbProZclLocalPrimitiveObligatoryPart_t zclObligatoryPart;
    SYS_DataPointer_t                      codeString;
} ZBPRO_ZCL_DoorLockCmdLockUnlockReqParams_t;

/**//**
 * \brief   Lock/unlock confirmation parameters type
 */
typedef struct _ZBPRO_ZCL_DoorLockCmdLockUnlockConfParams_t
{
    ZbProZclLocalPrimitiveObligatoryPart_t zclObligatoryPart;
} ZBPRO_ZCL_DoorLockCmdLockUnlockConfParams_t;


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Lock Door command
 *  specific to Door Lock ZCL cluster.
 */
typedef struct _ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t;


/**//**
 * \brief   Data type for the callback function of Identify cluster command
 */
typedef void ZBPRO_ZCL_DoorLockCmdLockConfCallback_t(
    ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t   *const reqDescr,
    ZBPRO_ZCL_DoorLockCmdLockUnlockConfParams_t *const confParams);

/**//**
 * \brief   Lock/unlock command descriptor type.
 */
struct _ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t
{
    ZBPRO_ZCL_DoorLockCmdLockConfCallback_t    *callback;
    ZbProZclLocalPrimitiveDescrService_t        service;
    ZBPRO_ZCL_DoorLockCmdLockUnlockReqParams_t  params;
};

/**//**
 * \brief   Lock Door command request.
 */
void ZBPRO_ZCL_DoorLockCmdLockReq(ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t *const reqDescr);

/**//**
 * \brief   Unlock Door command request.
 */
void ZBPRO_ZCL_DoorLockCmdUnlockReq(ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t *const reqDescr);

#endif /* _BB_ZBPRO_ZCL_SAP_CLUSTER_DOOR_LOCK_H */
