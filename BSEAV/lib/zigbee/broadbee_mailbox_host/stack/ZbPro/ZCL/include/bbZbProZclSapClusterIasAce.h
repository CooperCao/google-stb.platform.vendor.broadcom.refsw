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

/******************************************************************************
*
* DESCRIPTION:
*       Server side ZCL IAS ACE cluster SAP interface.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZCL_SAP_CLUSTER_IAS_ACE_H
#define _BB_ZBPRO_ZCL_SAP_CLUSTER_IAS_ACE_H

/************************* INCLUDES *****************************************************/
#include "bbZbProZclCommon.h"

/************************* DEFINITIONS **************************************************/

/**//**
 * \brief Enumeration of client-to-server commands
 * \ingroup ZBPRO_ZCL_Misc
 */
typedef enum _ZBPRO_ZCL_SapIasAceClientToServerCommandId_t
{
    ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_ARM                            = 0x00,
    ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_BYPASS                         = 0x01,
    ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_EMERGENCY                      = 0x02,
    ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_FIRE                           = 0x03,
    ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_PANIC                          = 0x04,
    ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_GET_ZONE_ID_MAP                = 0x05,
    ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_GET_ZONE_INFORMATION           = 0x06,
    ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_GET_PANEL_STATUS               = 0x07,
    ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_GET_BYPASSED_ZONE_LIST         = 0x08,
    ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_GET_ZONE_STATUS                = 0x09
} ZBPRO_ZCL_SapIasAceClientToServerCommandId_t;

/**//**
 * \brief Enumeration of server-to-client commands
 * \ingroup ZBPRO_ZCL_Misc
 */
typedef enum _ZBPRO_ZCL_SapIasAceServerToClientCommandId_t
{
    ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_ARM_RESPONSE                   = 0x00,
    ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_GET_ZONE_ID_MAP_RESPONSE       = 0x01,
    ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_GET_ZONE_INFORMATION_RESPONSE  = 0x02,
    ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_ZONE_STATUS_CHANGED            = 0x03,
    ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_PANEL_STATUS_CHANGED           = 0x04,
    ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_GET_PANEL_STATUS_RESPONSE      = 0x05,
    ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_SET_BYPASSED_ZONE_LIST         = 0x06,
    ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_BYPASS_RESPONSE                = 0x07,
    ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_GET_ZONE_STATUS_RESPONSE       = 0x08
} ZBPRO_ZCL_SapIasAceServerToClientCommandId_t;

/**//**
 * \brief IAS ACE Get Zone ID Map Sections Number
 */
#define ZBPRO_ZCL_IAS_ACE_GET_ZONE_ID_MAP_SECTIONS_NUM          16

/**//**
 * \brief IAS ACE Get Zone status response Zone Status recored size
 */
#define ZBPRO_ZCL_IAS_ACE_GET_ZONE_STATUS_RESP_RECORD_SIZE          (1 + 2) /* zoneId + zoneStatus */

/*************************** TYPES ******************************************************/

/**//**
 * \brief IAS ACE Arm Command armMode field values
 * \ingroup ZBPRO_ZCL_IasAceArmInd
 */
typedef enum _ZBPRO_ZCL_IasAceArmMode_t
{
    ZBPRO_ZCL_IAS_ACE_ARM_MODE_DISARM           = 0x00,
    ZBPRO_ZCL_IAS_ACE_ARM_MODE_ARM_DAY          = 0x01,
    ZBPRO_ZCL_IAS_ACE_ARM_MODE_ARM_NIGHT        = 0x02,
    ZBPRO_ZCL_IAS_ACE_ARM_MODE_ARM_ALL          = 0x03
} ZBPRO_ZCL_IasAceArmMode_t;
SYS_DbgAssertStatic(sizeof(ZBPRO_ZCL_IasAceArmMode_t) == 1);    /* check of the actual size */

/**//**
 * \brief IAS ACE Arm Command indication parameters
 * \ingroup ZBPRO_ZCL_IasAceArmInd
 */
typedef struct _ZBPRO_ZCL_SapIasAceArmIndParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;
                                                      /*!< Set of obligatory parameters of ZCL public
                                                           interface to local application. */
    /* Custom parameters. */
    ZBPRO_ZCL_IasAceArmMode_t       armMode;          /*!< ARM Mode */
    uint8_t                         zoneId;           /*!< Zone ID */
    SYS_DataPointer_t               payload;          /*!< Arm/Disarm code as UTF-8 character string w/o leading length.
                                                           \note Refer to HA errata 1855 */
} ZBPRO_ZCL_SapIasAceArmIndParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_SapIasAceArmIndParams_t);

/**//**
 * \brief Notification field of the IAS ACE Arm Response Command
 * \ingroup ZBPRO_ZCL_IasAceArmReq
 */
typedef enum _ZBPRO_ZCL_IasAceArmNotification_t
{
    ZBPRO_ZCL_IAS_ACE_ARM_NOTIFIED_DISARMED                     = ZBPRO_ZCL_IAS_ACE_ARM_MODE_DISARM,
    ZBPRO_ZCL_IAS_ACE_ARM_NOTIFIED_ARMED_DAY                    = ZBPRO_ZCL_IAS_ACE_ARM_MODE_ARM_DAY,
    ZBPRO_ZCL_IAS_ACE_ARM_NOTIFIED_ARMED_NIGHT                  = ZBPRO_ZCL_IAS_ACE_ARM_MODE_ARM_NIGHT,
    ZBPRO_ZCL_IAS_ACE_ARM_NOTIFIED_ARMED_ALL                    = ZBPRO_ZCL_IAS_ACE_ARM_MODE_ARM_ALL,
    ZBPRO_ZCL_IAS_ACE_ARM_NOTIFIED_INAVLID_CODE                 = 0x04,
    ZBPRO_ZCL_IAS_ACE_ARM_NOTIFIED_NOT_READY                    = 0x05, /*!< HA spec errata CCB 1781 */
    ZBPRO_ZCL_IAS_ACE_ARM_NOTIFIED_ALREADY_DISARMED             = 0x06  /*!< HA spec errata CCB 1781 */
} ZBPRO_ZCL_IasAceArmNotification_t;
SYS_DbgAssertStatic(sizeof(ZBPRO_ZCL_IasAceArmNotification_t) == 1);    /* check of the actual size */

/**//**
 * \brief IAS ACE Arm Response Command request parameters
 * \ingroup ZBPRO_ZCL_IasAceArmReq
 *
 * \note these parameters are based on fields of the Arm response command
 */
typedef struct _ZBPRO_ZCL_SapIasAceArmRespReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;
                                                      /*!< Set of obligatory parameters of ZCL public
                                                           interface to local application. */
    /* Custom parameters. */
    ZBPRO_ZCL_IasAceArmNotification_t   notification; /*!< Notification value */
} ZBPRO_ZCL_SapIasAceArmRespReqParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_SapIasAceArmRespReqParams_t);

/**//**
 * \brief IAS ACE Bypass Command indication parameters
 * \ingroup ZBPRO_ZCL_IasAceBypassInd
 */
typedef struct _ZBPRO_ZCL_SapIasAceBypassIndParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;
                                                      /*!< Set of obligatory parameters of ZCL public
                                                           interface to local application. */
    /* Custom parameters. */
    uint8_t                         zoneNum;          /*!< Number of Zones */
    SYS_DataPointer_t               payload;          /*!< List of ZoneIds followed by an Arm/Disarm code as UTF-8
                                                           character string w/o leading length
                                                           \note Refer to HA errata 1855 */
} ZBPRO_ZCL_SapIasAceBypassIndParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_SapIasAceBypassIndParams_t);

/**//**
 * \brief Result field of the IAS ACE Bypass response command
 * \ingroup ZBPRO_ZCL_IasAceBypassReq
 *
 * \note This field was introduced in HA spec errata CCB 1855
 */
typedef enum _ZBPRO_ZCL_IasAceBypassResult_t
{
    ZBPRO_ZCL_IAS_ACE_BYPASS_RESULT_BYPASSED            = 0x00,
    ZBPRO_ZCL_IAS_ACE_BYPASS_RESULT_NOT_BYPASSED        = 0x01,
    ZBPRO_ZCL_IAS_ACE_BYPASS_RESULT_NOT_ALLOWED         = 0x02,
    ZBPRO_ZCL_IAS_ACE_BYPASS_RESULT_INVALID_ZONE_ID     = 0x03,
    ZBPRO_ZCL_IAS_ACE_BYPASS_RESULT_UNKNOWN_ZONE_ID     = 0x04,
    ZBPRO_ZCL_IAS_ACE_BYPASS_RESULT_INVALID_ARM_CODE    = 0x05
} ZBPRO_ZCL_IasAceBypassResult_t;
SYS_DbgAssertStatic(sizeof(ZBPRO_ZCL_IasAceBypassResult_t) == 1);       /* check of the actual size */

/**//**
 * \brief IAS ACE Bypass Response Command request parameters
 * \ingroup ZBPRO_ZCL_IasAceBypassReq
 *
 * \note these parameters are based on fields of the Bypass response command
 *       which was introduced in HA spec errata CCB 1855
 */
typedef struct _ZBPRO_ZCL_SapIasAceBypassRespReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;
                                                      /*!< Set of obligatory parameters of ZCL public
                                                           interface to local application. */
    /* Custom parameters. */
    SYS_DataPointer_t               payload;          /*!< List of requested Zone Id results fields */
} ZBPRO_ZCL_SapIasAceBypassRespReqParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_SapIasAceBypassRespReqParams_t);

/**//**
 * \brief Alarm indication parameters for IAS ACE Emergency, Fire, and Panic Commands
 * \ingroup ZBPRO_ZCL_IasAceAlarmInd
 */
typedef struct _ZBPRO_ZCL_SapIasAceAlarmIndParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;
                                                      /*!< Set of obligatory parameters of ZCL public
                                                           interface to local application. */
    /* No custom parameters. */
} ZBPRO_ZCL_SapIasAceAlarmIndParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_SapIasAceAlarmIndParams_t);

/**//**
 * \brief IAS ACE Get Zone ID MAP Command indication parameters
 * \ingroup ZBPRO_ZCL_IasAceGetZoneIdMapInd
 *
 * \note Full declaration is provided for Python param parser
 */
typedef struct _ZBPRO_ZCL_SapIasAceGetZoneIdMapIndParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;
                                                      /*!< Set of obligatory parameters of ZCL public
                                                           interface to local application. */
    /* No custom parameters. */
} ZBPRO_ZCL_SapIasAceGetZoneIdMapIndParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_SapIasAceGetZoneIdMapIndParams_t);

/**//**
 * \brief IAS ACE Get Zone ID Map Section
 * \ingroup ZBPRO_ZCL_IasAceGetZoneIdMapReq
 */
typedef BitField16_t ZBPRO_ZCL_IasAceGetZoneIdMapSection_t;

/**//**
 * \brief IAS ACE Get Zone ID Map Section
 * \ingroup ZBPRO_ZCL_IasAceGetZoneIdMapResp
 */
typedef ZBPRO_ZCL_IasAceGetZoneIdMapSection_t
        ZBPRO_ZCL_IasAceGetZoneIdMap_t[ZBPRO_ZCL_IAS_ACE_GET_ZONE_ID_MAP_SECTIONS_NUM];

/**//**
 * \brief IAS ACE Get Zone ID Map Response Command request parameters
 * \ingroup ZBPRO_ZCL_IasAceGetZoneIdMapReq
 */
typedef struct _ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;
                                                      /*!< Set of obligatory parameters of ZCL public
                                                           interface to local application. */
    /* Custom parameters. */
    ZBPRO_ZCL_IasAceGetZoneIdMap_t          map;      /*!< Zone ID Map */
} ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqParams_t);

/**//**
 * \brief IAS ACE Get Zone Information Command indication parameters
 * \ingroup ZBPRO_ZCL_IasAceGetZoneInfoInd
 */
typedef struct _ZBPRO_ZCL_SapIasAceGetZoneInfoIndParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;
                                                      /*!< Set of obligatory parameters of ZCL public
                                                           interface to local application. */
    /* Custom parameters. */
    uint8_t                         zoneId;           /*!< Zone ID */
} ZBPRO_ZCL_SapIasAceGetZoneInfoIndParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_SapIasAceGetZoneInfoIndParams_t);

/**//**
 * \brief IAS ACE Get Zone Information Response Command request parameters
 * \ingroup ZBPRO_ZCL_IasAceGetZoneInfoReq
 */
typedef struct _ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;
                                                      /*!< Set of obligatory parameters of ZCL public
                                                           interface to local application. */
    /* Custom parameters. */
    uint8_t                         zoneId;           /*!< Zone ID */
    uint16_t                        zoneType;         /*!< IAS Zone cluster type */
    ZBPRO_APS_ExtAddr_t             ieeeAddress;      /*!< Extended device Address */
    SYS_DataPointer_t               payload;          /*!< Zone Label. UTF-8 ZigBee character string.
                                                           \note Refer to HA errata 1855 */
} ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqParams_t);

/**//**
 * \brief IAS ACE Get Panel Status Command indication parameters
 * \ingroup ZBPRO_ZCL_IasAceGetPanelStatusInd
 *
 * \par see docs-13-0553-41 HA spec errata, CCB 1781
 * \note Full declaration is provided for Python param parser
 */
typedef struct _ZBPRO_ZCL_SapIasAceGetPanelStatusIndParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;
                                                      /*!< Set of obligatory parameters of ZCL public
                                                           interface to local application. */
    /* No custom parameters. */
} ZBPRO_ZCL_SapIasAceGetPanelStatusIndParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_SapIasAceGetPanelStatusIndParams_t);

/**//**
 * \brief Panel status field of the IAS ACE Get Panel status response command
 * \ingroup ZBPRO_ZCL_IasAceGetPanelStatusReq
 */
typedef enum _ZBPRO_ZCL_SapIasAcePanelStatus_t
{
    ZBPRO_ZCL_IAS_ACE_PANEL_STATUS_DISARMED          = 0x00,
    ZBPRO_ZCL_IAS_ACE_PANEL_STATUS_ARMED_STAY        = 0x01,
    ZBPRO_ZCL_IAS_ACE_PANEL_STATUS_ARMED_NIGHT       = 0x02,
    ZBPRO_ZCL_IAS_ACE_PANEL_STATUS_ARMED_AWAY        = 0x03,
    ZBPRO_ZCL_IAS_ACE_PANEL_STATUS_EXIT_DELAY        = 0x04,
    ZBPRO_ZCL_IAS_ACE_PANEL_STATUS_ENTRY_DELAY       = 0x05,
    ZBPRO_ZCL_IAS_ACE_PANEL_STATUS_NOT_READY         = 0x06,
    ZBPRO_ZCL_IAS_ACE_PANEL_STATUS_IN_ALARM          = 0x07,
    ZBPRO_ZCL_IAS_ACE_PANEL_STATUS_ARMING_STAY       = 0x08,
    ZBPRO_ZCL_IAS_ACE_PANEL_STATUS_ARMING_NIGHT      = 0x09,
    ZBPRO_ZCL_IAS_ACE_PANEL_STATUS_ARMING_AWAY       = 0x0a
} ZBPRO_ZCL_SapIasAcePanelStatus_t;
SYS_DbgAssertStatic(sizeof(ZBPRO_ZCL_SapIasAcePanelStatus_t) == 1);       /* check of the actual size */

/**//**
 * \brief IAS ACE Audible Notification
 * \ingroup ZBPRO_ZCL_Misc
 */
typedef enum _ZBPRO_ZCL_SapIasAceAudibleNotification_t
{
    ZBPRO_ZCL_IAS_ACE_AUDIBLE_NOTIFICATION_MUTE                         = 0X00,
    ZBPRO_ZCL_IAS_ACE_AUDIBLE_NOTIFICATION_DEFAULT_SOUND                = 0X01,
    ZBPRO_ZCL_IAS_ACE_AUDIBLE_NOTIFICATION_MANUFACTURER_SPECIFIC_START  = 0X80
} ZBPRO_ZCL_SapIasAceAudibleNotification_t;
SYS_DbgAssertStatic(sizeof(ZBPRO_ZCL_SapIasAceAudibleNotification_t) == 1);       /* check of the actual size */

/**//**
 * \brief IAS ACE Alarm Status
 * \ingroup ZBPRO_ZCL_Misc
 *
 * \par see docs-13-0553-41 HA spec errata, CCB 1781
 */
typedef enum _ZBPRO_ZCL_IasAceAlarmStatus_t
{
    ZBPRO_ZCL_IAS_ACE_ALARM_STATUS_NO_ALARM         = 0x00,
    ZBPRO_ZCL_IAS_ACE_ALARM_STATUS_BURGLAR          = 0x01,
    ZBPRO_ZCL_IAS_ACE_ALARM_STATUS_FIRE             = 0x02,
    ZBPRO_ZCL_IAS_ACE_ALARM_STATUS_EMERGENCY        = 0x03,
    ZBPRO_ZCL_IAS_ACE_ALARM_STATUS_POLICE_PANIC     = 0x04,
    ZBPRO_ZCL_IAS_ACE_ALARM_STATUS_FIRE_PANICIC     = 0x05,
    ZBPRO_ZCL_IAS_ACE_ALARM_STATUS_EMERGENCY_PANIC  = 0x06
} ZBPRO_ZCL_IasAceAlarmStatus_t;
SYS_DbgAssertStatic(sizeof(ZBPRO_ZCL_IasAceAlarmStatus_t) == 1);        /* check of the actual size */

/**//**
 * \brief IAS ACE Get Panel Status Response Command request parameters
 * \ingroup ZBPRO_ZCL_IasAceGetPanelStatusReq
 *
 * \par see docs-13-0553-41 HA spec errata, CCB 1781
 */
typedef struct _ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;
                                                      /*!< Set of obligatory parameters of ZCL public
                                                           interface to local application. */
    /* Custom parameters. */
    ZBPRO_ZCL_SapIasAcePanelStatus_t            panelStatus;
                                                      /*!< Panel status */
    uint8_t                                     secondsRemaining;
                                                      /*!< Remaining seconds */
    ZBPRO_ZCL_SapIasAceAudibleNotification_t    audibleNotification;
                                                      /*!< Notification type */
    ZBPRO_ZCL_IasAceAlarmStatus_t               alarmStatus;
                                                      /*!< Alarm status */
} ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqParams_t);

/**//**
 * \brief IAS ACE Get Bypassed Zone List Command indication parameters
 * \ingroup ZBPRO_ZCL_IasAceGetBypassedZoneListInd
 *
 * \par see docs-13-0553-41 HA spec errata, CCB 1855
 * \note Full declaration is provided for Python param parser
 */
typedef struct _ZBPRO_ZCL_SapIasAceGetBypassedZoneListIndParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;
                                                      /*!< Set of obligatory parameters of ZCL public
                                                           interface to local application. */
    /* No custom parameters. */
} ZBPRO_ZCL_SapIasAceGetBypassedZoneListIndParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_SapIasAceGetBypassedZoneListIndParams_t);

/**//**
 * \brief IAS ACE Set Bypassed Zone List Command request parameters.
 *        This command is the response to Get Bypassed Zone List Command.
 * \ingroup ZBPRO_ZCL_IasAceSetBypassedZoneListReq
 *
 * \par see docs-13-0553-41 HA spec errata, CCB 1855
 * \note Full declaration is provided for Python param parser
 */
typedef struct _ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;
                                                      /*!< Set of obligatory parameters of ZCL public
                                                           interface to local application. */
    /* Custom parameters. */
    SYS_DataPointer_t                       payload;  /*!< List of bypassed Zone Ids in ascending order */
} ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqParams_t);

/**//**
 * \brief IAS ACE Get Zone Status Command indication parameters
 * \ingroup ZBPRO_ZCL_IasAceGetZoneStatusInd
 *
 * \par see docs-13-0553-41 HA spec errata, CCB 1781
 */
typedef struct _ZBPRO_ZCL_SapIasAceGetZoneStatusIndParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;
                                                      /*!< Set of obligatory parameters of ZCL public
                                                           interface to local application. */
    /* Custom parameters. */
    uint8_t                         startingZoneId;   /*!< Starting Zone ID */
    uint8_t                         zoneIdNumMax;     /*!< Max Zone ID */
    Bool8_t                         zoneStatusMaskFlag; /*!< Zone status Mask flag  */
    BitField16_t                    zoneStatusMask;   /*!< Zone status Mask */
} ZBPRO_ZCL_SapIasAceGetZoneStatusIndParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_SapIasAceGetZoneStatusIndParams_t);

/**//**
 * \brief IAS ACE Get Zone Status Response Command request parameters
 * \ingroup ZBPRO_ZCL_IasAceGetZoneStatusReq
 *
 * \par see docs-13-0553-41 HA spec errata, CCB 1781
 */
typedef struct _ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;
                                                      /*!< Set of obligatory parameters of ZCL public
                                                           interface to local application. */
    /* Custom parameters. */
    Bool8_t                         zoneStatusComplete; /*!< Zone status complete flag  */
    SYS_DataPointer_t               payload;          /*!< List of (ZoneId, ZoneStatus) records each of
                                                           ZBPRO_ZCL_IAS_ACE_GET_ZONE_STATUS_RESP_RECORD_SIZE bytes */
} ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqParams_t);

/**//**
 * \brief IAS ACE Zone Status Changed Command request parameters
 * \ingroup ZBPRO_ZCL_IasAceZoneStatusChangedReq
 *
 * \par see docs-13-0553-41 HA spec errata, CCB 1781
 */
typedef struct _ZBPRO_ZCL_SapIasAceZoneStatusChangedReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;
                                                      /*!< Set of obligatory parameters of ZCL public
                                                           interface to local application. */
    /* Custom parameters. */
    uint8_t                                  zoneId;  /*!< Zone ID */
    uint16_t                                 zoneStatus;
                                                      /*!< Zone status */
    ZBPRO_ZCL_SapIasAceAudibleNotification_t audibleNotification;
                                                      /*!< Notification type */
    SYS_DataPointer_t                        payload; /*!< as zoneLabel - UTF-8 ZigBee character string
                                                           w/o leading size byte */
} ZBPRO_ZCL_SapIasAceZoneStatusChangedReqParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_SapIasAceZoneStatusChangedReqParams_t);

/**//**
 * \brief IAS ACE Panel Status Changed Command request parameters
 * \ingroup ZBPRO_ZCL_IasAcePanelStatusChangedReq
 *
 * \par see docs-13-0553-41 HA spec errata, CCB 1781
 */
typedef struct _ZBPRO_ZCL_SapIasAcePanelStatusChangedReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;
                                                      /*!< Set of obligatory parameters of ZCL public
                                                           interface to local application. */
    /* Custom parameters. */
    ZBPRO_ZCL_SapIasAcePanelStatus_t         panelStatus;
                                                      /*!< Panel status */
    uint8_t                                  secondsRemaining;
                                                      /*!< Remaining seconds */
    ZBPRO_ZCL_SapIasAceAudibleNotification_t audibleNotification;
                                                      /*!< Notification type */
    ZBPRO_ZCL_IasAceAlarmStatus_t            alarmStatus;
                                                      /*!< Alarm status */
} ZBPRO_ZCL_SapIasAcePanelStatusChangedReqParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_SapIasAcePanelStatusChangedReqParams_t);

/**//**
 * \brief IAS ACE common response request confirm parameters
 * \ingroup ZBPRO_ZCL_IasAceCommonConf
 */
typedef struct _ZBPRO_ZCL_SapIasAceRespReqConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;
                                                      /*!< Set of obligatory parameters of ZCL public
                                                           interface to local application. */
    /* No custom parameters */
} ZBPRO_ZCL_SapIasAceRespReqConfParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_SapIasAceRespReqConfParams_t);

/**//**
 * \brief Notification field of the IAS ACE Arm Response Command
 * \ingroup ZBPRO_ZCL_IasAceArmReq
 */
typedef struct _ZBPRO_ZCL_SapIasAceArmRespReqDescr_t ZBPRO_ZCL_SapIasAceArmRespReqDescr_t;

/**//**
 * \brief Notification field of the IAS ACE Bypass Response Command
 * \ingroup ZBPRO_ZCL_IasAceBypassReq
 */
typedef struct _ZBPRO_ZCL_SapIasAceBypassRespReqDescr_t ZBPRO_ZCL_SapIasAceBypassRespReqDescr_t;

/**//**
 * \brief Notification field of the IAS ACE Get Zone ID Map Response Command
 * \ingroup ZBPRO_ZCL_IasAceGetZoneIdMapReq
 */
typedef struct _ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqDescr_t ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqDescr_t;

/**//**
 * \brief Notification field of the IAS ACE Get Zone Info Response Command
 * \ingroup ZBPRO_ZCL_IasAceGetZoneInfoReq
 */
typedef struct _ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqDescr_t ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqDescr_t;

/**//**
 * \brief Notification field of the IAS ACE Panel Status Response Command
 * \ingroup ZBPRO_ZCL_IasAcePanelStatusReq
 */
typedef struct _ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqDescr_t ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqDescr_t;

/**//**
 * \brief Notification field of the IAS ACE Set Bypassed Zone List Response Command
 * \ingroup ZBPRO_ZCL_IasAceSetBypassedZoneListReq
 */
typedef struct _ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqDescr_t ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqDescr_t;

/**//**
 * \brief Notification field of the IAS ACE Get Zone Status Response Command
 * \ingroup ZBPRO_ZCL_IasAceGetZoneStatusReq
 */
typedef struct _ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqDescr_t ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqDescr_t;

/**//**
 * \brief Notification field of the IAS ACE Zone Status Changed Response Command
 * \ingroup ZBPRO_ZCL_IasAceZoneStatusChangedReq
 */
typedef struct _ZBPRO_ZCL_SapIasAceZoneStatusChangedReqDescr_t ZBPRO_ZCL_SapIasAceZoneStatusChangedReqDescr_t;

/**//**
 * \brief Notification field of the IAS ACE Panel Status Changed Response Command
 * \ingroup ZBPRO_ZCL_IasAcePanelStatusChangedReq
 */
typedef struct _ZBPRO_ZCL_SapIasAcePanelStatusChangedReqDescr_t ZBPRO_ZCL_SapIasAcePanelStatusChangedReqDescr_t;

/**//**
 * \brief IAS ACE Arm declaration of confirmation callback
 * \ingroup ZBPRO_ZCL_IasAceArmConf
 */
typedef void ZBPRO_ZCL_SapIasAceArmRespReqConfCallback_t(
        ZBPRO_ZCL_SapIasAceArmRespReqDescr_t *const  reqDescr,
        ZBPRO_ZCL_SapIasAceRespReqConfParams_t *const  confParams);

/**//**
 * \brief IAS ACE Bypass declaration of confirmation callback
 * \ingroup ZBPRO_ZCL_IasAceBypassConf
 */
typedef void ZBPRO_ZCL_SapIasAceBypassRespReqConfCallback_t(
        ZBPRO_ZCL_SapIasAceBypassRespReqDescr_t *const  reqDescr,
        ZBPRO_ZCL_SapIasAceRespReqConfParams_t *const  confParams);

/**//**
 * \brief IAS ACE Get Zone ID Map declaration of confirmation callback
 * \ingroup ZBPRO_ZCL_IasAceGetZoneIdMapConf
 */
typedef void ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqConfCallback_t(
        ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqDescr_t *const  reqDescr,
        ZBPRO_ZCL_SapIasAceRespReqConfParams_t *const  confParams);

/**//**
 * \brief IAS ACE Get Zone Info declaration of confirmation callback
 * \ingroup ZBPRO_ZCL_IasAceGetZoneInfoConf
 */
typedef void ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqConfCallback_t(
        ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqDescr_t *const  reqDescr,
        ZBPRO_ZCL_SapIasAceRespReqConfParams_t *const  confParams);

/**//**
 * \brief IAS ACE Get Panel Status declaration of confirmation callback
 * \ingroup ZBPRO_ZCL_IasAceGetPanelStatusConf
 */
typedef void ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqConfCallback_t(
        ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqDescr_t *const  reqDescr,
        ZBPRO_ZCL_SapIasAceRespReqConfParams_t *const  confParams);

/**//**
 * \brief IAS ACE Set Bypassed Zone List declaration of confirmation callback
 * \ingroup ZBPRO_ZCL_IasAceSetBypassedZoneListConf
 */
typedef void ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqConfCallback_t(
        ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqDescr_t *const  reqDescr,
        ZBPRO_ZCL_SapIasAceRespReqConfParams_t *const  confParams);

/**//**
 * \brief IAS ACE Get Zone Status declaration of confirmation callback
 * \ingroup ZBPRO_ZCL_IasAceGetZoneStatusConf
 */
typedef void ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqConfCallback_t(
        ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqDescr_t *const  reqDescr,
        ZBPRO_ZCL_SapIasAceRespReqConfParams_t *const  confParams);

/**//**
 * \brief IAS ACE Zone Status Changed declaration of confirmation callback
 * \ingroup ZBPRO_ZCL_IasAceZoneStatusChangedConf
 */
typedef void ZBPRO_ZCL_SapIasAceZoneStatusChangedReqConfCallback_t(
        ZBPRO_ZCL_SapIasAceZoneStatusChangedReqDescr_t *const  reqDescr,
        ZBPRO_ZCL_SapIasAceRespReqConfParams_t *const  confParams);

/**//**
 * \brief IAS ACE Panel Status Changed declaration of confirmation callback
 * \ingroup ZBPRO_ZCL_IasAcePanelStatusChangedConf
 */
typedef void ZBPRO_ZCL_SapIasAcePanelStatusChangedReqConfCallback_t(
        ZBPRO_ZCL_SapIasAcePanelStatusChangedReqDescr_t *const  reqDescr,
        ZBPRO_ZCL_SapIasAceRespReqConfParams_t *const  confParams);

/**//**
 * \brief Structure declarations of the request descriptor
 * \ingroup ZBPRO_ZCL_IasAceArmReq
 */
struct _ZBPRO_ZCL_SapIasAceArmRespReqDescr_t
{
    ZBPRO_ZCL_SapIasAceArmRespReqConfCallback_t         *callback;         /*!< Confirmation callback */
    ZbProZclLocalPrimitiveDescrService_t                service;           /*!< Request service structure */
    ZBPRO_ZCL_SapIasAceArmRespReqParams_t               params;            /*!< Request parameters */
};

/**//**
 * \brief Structure declarations of the request descriptor
 * \ingroup ZBPRO_ZCL_IasAceBypassReq
 */
struct _ZBPRO_ZCL_SapIasAceBypassRespReqDescr_t
{
    ZBPRO_ZCL_SapIasAceBypassRespReqConfCallback_t      *callback;         /*!< Confirmation callback */
    ZbProZclLocalPrimitiveDescrService_t                service;           /*!< Request service structure */
    ZBPRO_ZCL_SapIasAceBypassRespReqParams_t            params;            /*!< Request parameters */
};

/**//**
 * \brief Structure declarations of the request descriptor
 * \ingroup ZBPRO_ZCL_IasAceGetZoneIdMapReq
 */
struct _ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqDescr_t
{
    ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqConfCallback_t *callback;        /*!< Confirmation callback */
    ZbProZclLocalPrimitiveDescrService_t                service;           /*!< Request service structure */
    ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqParams_t      params;            /*!< Request parameters */
};

/**//**
 * \brief Structure declarations of the request descriptor
 * \ingroup ZBPRO_ZCL_IasAceGetZoneInfoReq
 */
struct _ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqDescr_t
{
    ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqConfCallback_t *callback;         /*!< Confirmation callback */
    ZbProZclLocalPrimitiveDescrService_t                service;           /*!< Request service structure */
    ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqParams_t       params;            /*!< Request parameters */
};

/**//**
 * \brief Structure declarations of the request descriptor
 * \ingroup ZBPRO_ZCL_IasAceGetPanelStatusReq
 */
struct _ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqDescr_t
{
    ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqConfCallback_t *callback;      /*!< Confirmation callback */
    ZbProZclLocalPrimitiveDescrService_t                service;           /*!< Request service structure */
    ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqParams_t    params;            /*!< Request parameters */
};

/**//**
 * \brief Structure declarations of the request descriptor
 * \ingroup ZBPRO_ZCL_IasAceSetBypassedZoneListReq
 */
struct _ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqDescr_t
{
    ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqConfCallback_t *callback; /*!< Confirmation callback */
    ZbProZclLocalPrimitiveDescrService_t                service;           /*!< Request service structure */
    ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqParams_t params;          /*!< Request parameters */
};

/**//**
 * \brief Structure declarations of the request descriptor
 * \ingroup ZBPRO_ZCL_IasAceGetZoneStatusReq
 */
struct _ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqDescr_t
{
    ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqConfCallback_t *callback;       /*!< Confirmation callback */
    ZbProZclLocalPrimitiveDescrService_t                service;           /*!< Request service structure */
    ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqParams_t     params;            /*!< Request parameters */
};

/**//**
 * \brief Structure declarations of the request descriptor
 * \ingroup ZBPRO_ZCL_IasAceZoneStatusChangedReq
 */
struct _ZBPRO_ZCL_SapIasAceZoneStatusChangedReqDescr_t
{
    ZBPRO_ZCL_SapIasAceZoneStatusChangedReqConfCallback_t *callback;       /*!< Confirmation callback */
    ZbProZclLocalPrimitiveDescrService_t                service;           /*!< Request service structure */
    ZBPRO_ZCL_SapIasAceZoneStatusChangedReqParams_t     params;            /*!< Request parameters */

};

/**//**
 * \brief Structure declarations of the request descriptor
 * \ingroup ZBPRO_ZCL_IasAcePanelStatusChangedReq
 */
struct _ZBPRO_ZCL_SapIasAcePanelStatusChangedReqDescr_t
{
    ZBPRO_ZCL_SapIasAcePanelStatusChangedReqConfCallback_t *callback;      /*!< Confirmation callback */
    ZbProZclLocalPrimitiveDescrService_t                service;           /*!< Request service structure */
    ZBPRO_ZCL_SapIasAcePanelStatusChangedReqParams_t    params;            /*!< Request parameters */
};

ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_SapIasAceArmRespReqDescr_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_SapIasAceBypassRespReqDescr_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqDescr_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqDescr_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqDescr_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqDescr_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqDescr_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_SapIasAceZoneStatusChangedReqDescr_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_SapIasAcePanelStatusChangedReqDescr_t);

/************************* PROTOTYPES ***************************************************/

/**//**
 * \brief   Handles ZCL Local Indication on reception of IAS ACE Arm Command
 * specific to IAS ACE ZCL cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \note
 *  The IAS ACE Arm Command is sent by client. Due
 *  to this reason server side service provides this function.
 * \return Nothing.
 */
void ZBPRO_ZCL_SapIasAceArmInd(ZBPRO_ZCL_SapIasAceArmIndParams_t *const indParams);

/**//**
 * \brief   Handles ZCL Local Indication on reception of IAS ACE Bypass Command
 * specific to IAS ACE ZCL cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \note
 *  The IAS ACE Bypass Command is sent by client. Due
 *  to this reason server side service provides this function.
 * \return Nothing.
 */
void ZBPRO_ZCL_SapIasAceBypassInd(ZBPRO_ZCL_SapIasAceBypassIndParams_t *const indParams);

/**//**
 * \brief   Handles ZCL Local Indication on reception of IAS ACE Emergency Command
 * specific to IAS ACE ZCL cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \note
 *  The IAS ACE Emergency Command is sent by client. Due
 *  to this reason server side service provides this function.
 * \return Nothing.
 */
void ZBPRO_ZCL_SapIasAceEmergencyInd(ZBPRO_ZCL_SapIasAceAlarmIndParams_t *const indParams);

/**//**
 * \brief   Handles ZCL Local Indication on reception of IAS ACE Fire Command
 * specific to IAS ACE ZCL cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \note
 *  The IAS ACE Fire Command is sent by client. Due
 *  to this reason server side service provides this function.
 * \return Nothing.
 */
void ZBPRO_ZCL_SapIasAceFireInd(ZBPRO_ZCL_SapIasAceAlarmIndParams_t *const indParams);

/**//**
 * \brief   Handles ZCL Local Indication on reception of IAS ACE Panic Command
 * specific to IAS ACE ZCL cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \note
 *  The IAS ACE Panic Command is sent by client. Due
 *  to this reason server side service provides this function.
 * \return Nothing.
 */
void ZBPRO_ZCL_SapIasAcePanicInd(ZBPRO_ZCL_SapIasAceAlarmIndParams_t *const indParams);

/**//**
 * \brief   Handles ZCL Local Indication on reception of IAS ACE Get Zone ID MAP Command
 * specific to IAS ACE ZCL cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \note
 *  The IAS ACE Get Zone ID MAP Command is sent by client. Due
 *  to this reason server side service provides this function.
 * \return Nothing.
 */
void ZBPRO_ZCL_SapIasAceGetZoneIdMapInd(ZBPRO_ZCL_SapIasAceGetZoneIdMapIndParams_t *const indParams);

/**//**
 * \brief   Handles ZCL Local Indication on reception of IAS ACE Get Zone Information Command
 * specific to IAS ACE ZCL cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \note
 *  The IAS ACE Get Zone Information Command is sent by client. Due
 *  to this reason server side service provides this function.
 * \return Nothing.
 */
void ZBPRO_ZCL_SapIasAceGetZoneInfoInd(ZBPRO_ZCL_SapIasAceGetZoneInfoIndParams_t *const indParams);

/**//**
 * \brief   Handles ZCL Local Indication on reception of IAS ACE Get Panel Status Command
 * specific to IAS ACE ZCL cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \note
 *  The IAS ACE Get Panel Status Command is sent by client. Due
 *  to this reason server side service provides this function.
 * \return Nothing.
 */
void ZBPRO_ZCL_SapIasAceGetPanelStatusInd(ZBPRO_ZCL_SapIasAceGetPanelStatusIndParams_t *const indParams);

/**//**
 * \brief   Handles ZCL Local Indication on reception of IAS ACE Get Bypassed Zone List Command
 * specific to IAS ACE ZCL cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \note
 *  The IAS ACE Get Bypassed Zone List Command is sent by client. Due
 *  to this reason server side service provides this function.
 * \return Nothing.
 */
void ZBPRO_ZCL_SapIasAceGetBypassedZoneListInd(ZBPRO_ZCL_SapIasAceGetBypassedZoneListIndParams_t *const indParams);

/**//**
 * \brief   Handles ZCL Local Indication on reception of IAS ACE Get Zone Status Command
 * specific to IAS ACE ZCL cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \note
 *  The IAS ACE Get Zone Status Command is sent by client. Due
 *  to this reason server side service provides this function.
 * \return Nothing.
 */
void ZBPRO_ZCL_SapIasAceGetZoneStatusInd(ZBPRO_ZCL_SapIasAceGetZoneStatusIndParams_t *const indParams);

/**//**
 * \brief IAS ACE Arm Response Command request
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr       Pointer to the request parameters.
 * \return Nothing.
 */
void ZBPRO_ZCL_SapIasAceArmRespReq(ZBPRO_ZCL_SapIasAceArmRespReqDescr_t *const reqDescr);

/**//**
 * \brief IAS ACE Bypass Response Command request
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr       Pointer to the request parameters.
 * \return Nothing.
 */
void ZBPRO_ZCL_SapIasAceBypassRespReq(ZBPRO_ZCL_SapIasAceBypassRespReqDescr_t *const reqDescr);

/**//**
 * \brief IAS ACE Get Zone ID MAP Response Command request
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr       Pointer to the request parameters.
 * \return Nothing.
 */
void ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReq(ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqDescr_t *const reqDescr);

/**//**
 * \brief IAS ACE Get Zone Information Response Command request
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr       Pointer to the request parameters.
 * \return Nothing.
 */
void ZBPRO_ZCL_SapIasAceGetZoneInfoRespReq(ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqDescr_t *const reqDescr);

/**//**
 * \brief IAS ACE Get Panel Status Response Command request
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr       Pointer to the request parameters.
 * \return Nothing.
 */
void ZBPRO_ZCL_SapIasAceGetPanelStatusRespReq(ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqDescr_t *const reqDescr);

/**//**
 * \brief IAS ACE Get Bypassed Zone List Response Command request
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr       Pointer to the request parameters.
 * \return Nothing.
 */
void ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReq(ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqDescr_t *const reqDescr);

/**//**
 * \brief IAS ACE Get Zone Status Response Command request
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr       Pointer to the request parameters.
 * \return Nothing.
 */
void ZBPRO_ZCL_SapIasAceGetZoneStatusRespReq(ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqDescr_t *const reqDescr);

/**//**
 * \brief IAS ACE Zone Status Changed Command request
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr       Pointer to the request parameters.
 * \return Nothing.
 */
void ZBPRO_ZCL_SapIasAceZoneStatusChangedReq(ZBPRO_ZCL_SapIasAceZoneStatusChangedReqDescr_t *const reqDescr);

/**//**
 * \brief IAS ACE Panel Status Changed Command request
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr       Pointer to the request parameters.
 * \return Nothing.
 */
void ZBPRO_ZCL_SapIasAcePanelStatusChangedReq(ZBPRO_ZCL_SapIasAcePanelStatusChangedReqDescr_t *const reqDescr);

#endif /* _BB_ZBPRO_ZCL_SAP_CLUSTER_IAS_ACE_H */

/* eof bbZbProZclSapClusterIasAce.h */