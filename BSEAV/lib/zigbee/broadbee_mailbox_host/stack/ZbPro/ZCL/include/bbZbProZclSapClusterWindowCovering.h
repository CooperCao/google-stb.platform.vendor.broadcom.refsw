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
* FILENAME: $Workfile: trunk/stack/ZbPro/ZCL/include/bbZbProZclSapClusterWindowCovering.h $
*
* DESCRIPTION:
*   ZCL Window Covering cluster SAP interface.
*
* $Revision: 7517 $
* $Date: 2015-07-15 18:34:05Z $
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZCL_SAP_CLUSTER_WINDOW_COVERING_H
#define _BB_ZBPRO_ZCL_SAP_CLUSTER_WINDOW_COVERING_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZclCommon.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of attributes of Window Covering ZCL cluster Server side.
 * \details
 *  These attributes are provided by Server side of the cluster.
 * \details
 *  Window Covering ZCL cluster has no attributes provided by Client side.
 * \note
 *  This implementation of Window Covering ZCL cluster doesn't provide Server side; and its Client
 *  side isn't able to access any of the optional attributes of the Server side, it's able
 *  to access only the following mandatory attributes of the Server side:
 *  - Window Covering.
 *
 *  Client is also able to receive reporting of this Window Covering attribute.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 7.4, tables 7-39, 7-40, 7-43.
 */
#define ZCL_WINDOW_COVERING_MAKE_ATTRIBUTE(attributeSet, attributeId) (((attributeSet) << 4) | (attributeId))
typedef enum _ZBPRO_ZCL_SapWindowCoveringServerAttributeIdSet_t
{
    ZBPRO_ZCL_SAP_WC_ATTR_SET_INFO                            = 0x0000,       /* Window Covering Information attributes set */
    ZBPRO_ZCL_SAP_WC_ATTR_SET_SETTINGS                        = 0x0001        /* Window Covering Settings attributes set */
} ZBPRO_ZCL_SapWindowCoveringServerAttributeIdSet_t;
typedef enum _ZBPRO_ZCL_SapWindowCoveringServerAttributeId_t
{
    /*!< Window Covering Type (MANDATORY). */
    ZBPRO_ZCL_SAP_WC_ATTR_ID_TYPE                             = ZCL_WINDOW_COVERING_MAKE_ATTRIBUTE(ZBPRO_ZCL_SAP_WC_ATTR_SET_INFO, 0x0000),
    /*!< Window Covering Physical Closed Limit for Lift (OPTIONAL). */
    ZBPRO_ZCL_SAP_WC_ATTR_ID_PHYSICAL_CLOSED_LIMIT_LIFT       = ZCL_WINDOW_COVERING_MAKE_ATTRIBUTE(ZBPRO_ZCL_SAP_WC_ATTR_SET_INFO, 0x0001),
    /*!< Window Covering Physical Closed Limit for Tilt (OPTIONAL). */
    ZBPRO_ZCL_SAP_WC_ATTR_ID_PHYSICAL_CLOSED_LIMIT_TILT       = ZCL_WINDOW_COVERING_MAKE_ATTRIBUTE(ZBPRO_ZCL_SAP_WC_ATTR_SET_INFO, 0x0002),
    /*!< Window Covering Current Position for Lift (OPTIONAL). */
    ZBPRO_ZCL_SAP_WC_ATTR_ID_CURRENT_POSITION_LIFT            = ZCL_WINDOW_COVERING_MAKE_ATTRIBUTE(ZBPRO_ZCL_SAP_WC_ATTR_SET_INFO, 0x0003),
    /*!< Window Covering Current Position for Tilt (OPTIONAL). */
    ZBPRO_ZCL_SAP_WC_ATTR_ID_CURRENT_POSITION_TILT            = ZCL_WINDOW_COVERING_MAKE_ATTRIBUTE(ZBPRO_ZCL_SAP_WC_ATTR_SET_INFO, 0x0004),
    /*!< Window Covering Number of Actuations for Lift (OPTIONAL). */
    ZBPRO_ZCL_SAP_WC_ATTR_ID_NUMBER_ACTUATIONS_LIFT           = ZCL_WINDOW_COVERING_MAKE_ATTRIBUTE(ZBPRO_ZCL_SAP_WC_ATTR_SET_INFO, 0x0005),
    /*!< Window Covering Number of Actuations for Tilt (OPTIONAL). */
    ZBPRO_ZCL_SAP_WC_ATTR_ID_NUMBER_ACTUATIONS_TILT           = ZCL_WINDOW_COVERING_MAKE_ATTRIBUTE(ZBPRO_ZCL_SAP_WC_ATTR_SET_INFO, 0x0006),
    /*!< Window Covering Config/Status (MANDATORY). */
    ZBPRO_ZCL_SAP_WC_ATTR_ID_CONFIG_STATUS                    = ZCL_WINDOW_COVERING_MAKE_ATTRIBUTE(ZBPRO_ZCL_SAP_WC_ATTR_SET_INFO, 0x0007),
    /*!< Window Covering Current Position for Lift Percentage (OPTIONAL). */
    ZBPRO_ZCL_SAP_WC_ATTR_ID_CURRENT_POSITION_LIFT_PERCENTAGE = ZCL_WINDOW_COVERING_MAKE_ATTRIBUTE(ZBPRO_ZCL_SAP_WC_ATTR_SET_INFO, 0x0008),
    /*!< Window Covering Current Position for Tilt Percentage (OPTIONAL). */
    ZBPRO_ZCL_SAP_WC_ATTR_ID_CURRENT_POSITION_TILT_PERCENTAGE = ZCL_WINDOW_COVERING_MAKE_ATTRIBUTE(ZBPRO_ZCL_SAP_WC_ATTR_SET_INFO, 0x0009),
    /*!< Window Covering Installed Open Limit for Lift (MANDATORY). */
    ZBPRO_ZCL_SAP_WC_ATTR_ID_INSTALLED_OPEN_LIMIT_LIFT        = ZCL_WINDOW_COVERING_MAKE_ATTRIBUTE(ZBPRO_ZCL_SAP_WC_ATTR_SET_SETTINGS, 0x0000),
    /*!< Window Covering Installed Closed Limit for Lift (MANDATORY). */
    ZBPRO_ZCL_SAP_WC_ATTR_ID_INSTALLED_CLOSED_LIMIT_LIFT      = ZCL_WINDOW_COVERING_MAKE_ATTRIBUTE(ZBPRO_ZCL_SAP_WC_ATTR_SET_SETTINGS, 0x0001),
    /*!< Window Covering Installed Open Limit for Tilt (MANDATORY). */
    ZBPRO_ZCL_SAP_WC_ATTR_ID_INSTALLED_OPEN_LIMIT_TILT        = ZCL_WINDOW_COVERING_MAKE_ATTRIBUTE(ZBPRO_ZCL_SAP_WC_ATTR_SET_SETTINGS, 0x0002),
    /*!< Window Covering Installed Closed Limit for Tilt (MANDATORY). */
    ZBPRO_ZCL_SAP_WC_ATTR_ID_INSTALLED_CLOSED_LIMIT_TILT      = ZCL_WINDOW_COVERING_MAKE_ATTRIBUTE(ZBPRO_ZCL_SAP_WC_ATTR_SET_SETTINGS, 0x0003),
    /*!< Window Covering Velocity for Lift (OPTIONAL). */
    ZBPRO_ZCL_SAP_WC_ATTR_ID_VELOCITY_LIFT                    = ZCL_WINDOW_COVERING_MAKE_ATTRIBUTE(ZBPRO_ZCL_SAP_WC_ATTR_SET_SETTINGS, 0x0004),
    /*!< Window Covering Acceleration Time for Lift (OPTIONAL). */
    ZBPRO_ZCL_SAP_WC_ATTR_ID_ACCELERATION_TIME_LIFT           = ZCL_WINDOW_COVERING_MAKE_ATTRIBUTE(ZBPRO_ZCL_SAP_WC_ATTR_SET_SETTINGS, 0x0005),
    /*!< Window Covering Deceleration Time for Lift (OPTIONAL). */
    ZBPRO_ZCL_SAP_WC_ATTR_ID_DECELERATION_TIME_TILT           = ZCL_WINDOW_COVERING_MAKE_ATTRIBUTE(ZBPRO_ZCL_SAP_WC_ATTR_SET_SETTINGS, 0x0006),
    /*!< Window Covering Mode (MANDATORY). */
    ZBPRO_ZCL_SAP_WC_ATTR_ID_MODE                             = ZCL_WINDOW_COVERING_MAKE_ATTRIBUTE(ZBPRO_ZCL_SAP_WC_ATTR_SET_SETTINGS, 0x0007),
    /*!< Window Covering Intermediate Setponts for Lift (OPTIONAL). */
    ZBPRO_ZCL_SAP_WC_ATTR_ID_INTERMEDIATE_SETPOINTS_LIFT      = ZCL_WINDOW_COVERING_MAKE_ATTRIBUTE(ZBPRO_ZCL_SAP_WC_ATTR_SET_SETTINGS, 0x0008),
    /*!< Window Covering Intermediate Setponts for Tilt (OPTIONAL). */
    ZBPRO_ZCL_SAP_WC_ATTR_ID_INTERMEDIATE_SETPOINTS_TILT      = ZCL_WINDOW_COVERING_MAKE_ATTRIBUTE(ZBPRO_ZCL_SAP_WC_ATTR_SET_SETTINGS, 0x0009)
} ZBPRO_ZCL_SapWindowCoveringServerAttributeId_t;

/**//**
 * \name    WindowCoveringType attribute enumeration for Window Covering ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 7.4.2.1.2, table 7-41.
 */
typedef enum _ZBPRO_ZCL_WindowCoveringType_t
{
    ZBPRO_ZCL_WC_TYPE_ROLLERSHADE                             = 0x00,
    ZBPRO_ZCL_WC_TYPE_ROLLERSHADE_2_MOTOR                     = 0x01,
    ZBPRO_ZCL_WC_TYPE_ROLLERSHADE_EXTERIOR                    = 0x02,
    ZBPRO_ZCL_WC_TYPE_ROLLERSHADE_EXTERIOR_2_MOTOR            = 0x03,
    ZBPRO_ZCL_WC_TYPE_DRAPERY                                 = 0x04,
    ZBPRO_ZCL_WC_TYPE_AWNING                                  = 0x05,
    ZBPRO_ZCL_WC_TYPE_SHUTTER                                 = 0x06,
    ZBPRO_ZCL_WC_TYPE_TILT_BLIND_TILT_ONLY                    = 0x07,
    ZBPRO_ZCL_WC_TYPE_TILT_BLIND_LIFT_AND_TILT                = 0x08,
    ZBPRO_ZCL_WC_TYPE_PROJECTOR_SCREEN                        = 0x09
} ZBPRO_ZCL_WindowCoveringType_t;


/**//**
 * \name    Data types of attributes of Window Covering ZCL cluster.
 * \details
 *  All the listed attributes are transferred by value.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.8.2.2, table 3-42.
 */

/**@{*/

typedef ZBPRO_ZCL_WindowCoveringType_t ZBPRO_ZCL_SapWindowCoveringAttrType_t;

typedef uint16_t ZBPRO_ZCL_SapWindowCoveringAttrPhysicalClosedLimitLift_t;

typedef uint16_t ZBPRO_ZCL_SapWindowCoveringAttrPhysicalClosedLimitTilt_t;

typedef uint16_t ZBPRO_ZCL_SapWindowCoveringAttrCurrentPositionLift_t;

typedef uint16_t ZBPRO_ZCL_SapWindowCoveringAttrCurrentPositionTilt_t;

typedef uint16_t ZBPRO_ZCL_SapWindowCoveringAttrNumberActuationsLift_t;

typedef uint16_t ZBPRO_ZCL_SapWindowCoveringAttrNumberActuationsTilt_t;

typedef struct _ZBPRO_ZCL_SapWindowCoveringAttrConfigStatus_t
{
    uint8_t isOperational           : 1;
    uint8_t isOnline                : 1;
    uint8_t areCommandsReversed     : 1;
    uint8_t isLiftControlClosedLoop : 1;
    uint8_t isTiltControlClosedLoop : 1;
    uint8_t isEncoderControlledLift : 1;
    uint8_t isEncoderControlledTilt : 1;
    uint8_t reserved                : 1;
} ZBPRO_ZCL_SapWindowCoveringAttrConfigStatus_t;

typedef uint8_t ZBPRO_ZCL_SapWindowCoveringAttrCurrentPositionLiftPercentage_t; /* 0 - 100% */

typedef uint8_t ZBPRO_ZCL_SapWindowCoveringAttrCurrentPositionTiltPercentage_t; /* 0 - 100% */

typedef uint16_t ZBPRO_ZCL_SapWindowCoveringAttrInstalledOpenLimitLift_t;

typedef uint16_t ZBPRO_ZCL_SapWindowCoveringAttrInstalledClosedLimitLift_t;

typedef uint16_t ZBPRO_ZCL_SapWindowCoveringAttrInstalledOpenLimitTilt_t;

typedef uint16_t ZBPRO_ZCL_SapWindowCoveringAttrInstalledClosedLimitTilt_t;

typedef uint16_t ZBPRO_ZCL_SapWindowCoveringAttrVelocityLift_t;

typedef uint16_t ZBPRO_ZCL_SapWindowCoveringAttrAccelerationTimeLift_t;

typedef uint16_t ZBPRO_ZCL_SapWindowCoveringAttrDecelerationTimeLift_t;

typedef struct _ZBPRO_ZCL_SapWindowCoveringAttrMode_t
{
    uint8_t isMotorDirectionReversed   : 1;
    uint8_t isRunInCalibrationMode     : 1;
    uint8_t isRunningInMaintenanceMode : 1;
    uint8_t areLEDSDisplayFeedback     : 1;
    uint8_t reserved                   : 4;
} ZBPRO_ZCL_SapWindowCoveringAttrMode_t;

typedef SYS_DataPointer_t ZBPRO_ZCL_SapWindowCoveringAttrIntermediateSetpointsLift_t;

typedef SYS_DataPointer_t ZBPRO_ZCL_SapWindowCoveringAttrIntermediateSetpointsTilt_t;

/**@}*/


/**//**                                                                                                                 // TODO: Move to private header.
 * \brief   Enumeration of client-to-server commands specific to Window Covering ZCL cluster.
 * \details
 *  These commands are generated by Window Covering ZCL cluster Client side and received by Server
 *  side.
 * \details
 *  On/Off ZCL cluster has no commands generated by Server side and received by Client
 *  side.
 * \note
 *  This implementation of Window Covering ZCL cluster doesn't provide Server side; and its Client
 *  side doesn't implement generation of any of the optional commands, it's able to
 *  generate only the following mandatory commands:
 *  - UpOpen,
 *  - DownClose,
 *  - Stop,
 *  - GoToLiftPercentage,
 *  - GoToTiltPercentage.
 *
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 7.4.2.2, table
 *  7-45.
 */
typedef enum _ZBPRO_ZCL_SapWindowCoveringClientToServerCommandId_t
{
    ZBPRO_ZCL_SAP_WC_CMD_ID_UP_OPEN                         = 0x00,     /*!< Up/Open. */
    ZBPRO_ZCL_SAP_WC_CMD_ID_DOWN_CLOSE                      = 0x01,     /*!< Down/Close. */
    ZBPRO_ZCL_SAP_WC_CMD_ID_STOP                            = 0x02,     /*!< Stop. */
    ZBPRO_ZCL_SAP_WC_CMD_ID_GO_TO_LIFT_VALUE                = 0x04,     /*!< Go to Lift value. */
    ZBPRO_ZCL_SAP_WC_CMD_ID_GO_TO_LIFT_PERCENTAGE           = 0x05,     /*!< Go to Lift percentage. */
    ZBPRO_ZCL_SAP_WC_CMD_ID_GO_TO_TILT_VALUE                = 0x07,     /*!< Go to Tilt value. */
    ZBPRO_ZCL_SAP_WC_CMD_ID_GO_TO_TILT_PERCENTAGE           = 0x08,     /*!< Go to Tilt percentage. */
} ZBPRO_ZCL_SapWindowCoveringClientToServerCommandId_t;


/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Up/Open, Down/Close, or Stop
 *  command specific to Window Covering ZCL cluster.
 * \details
 *  All the implemented commands - Up/Open, Down/Close, Stop - have the same format and differ only
 *  by the Command Id which is determined by using different primitives.
 * \details
 *  Commands Up/Open, Down/Close, Stop have no custom parameters.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 7.4.2.2.1, 7.4.2.2.2, 7.4.2.2.3.
 */
typedef struct _ZBPRO_ZCL_WindowCoveringCmdReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* No custom parameters exist for this type of ZCL Local Request. */

} ZBPRO_ZCL_WindowCoveringCmdReqParams_t;

/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_WindowCoveringCmdReqParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Up/Open,
 * Down/Close or Stop command specific to Window Covering ZCL cluster.
 * \details
 *  This structure takes its origin from ZCL Default Response profile-wide command.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.4.12, figure 2-25.
 */
typedef struct _ZBPRO_ZCL_WindowCoveringCmdConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* No custom parameters exist for this type of ZCL Local Confirm. */

} ZBPRO_ZCL_WindowCoveringCmdConfParams_t;

/*
 * Validate structure of ZCL Local Confirm Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_WindowCoveringCmdConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Up/Open, Down/Close, or Stop
 *  command specific to Window Covering ZCL cluster.
 */
typedef struct _ZBPRO_ZCL_WindowCoveringCmdReqDescr_t  ZBPRO_ZCL_WindowCoveringCmdReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Up/Open, Down/Close, and Stop
 *  commands specific to Window Covering ZCL cluster.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_WindowCoveringCmdConfCallback_t(
                ZBPRO_ZCL_WindowCoveringCmdReqDescr_t   *const  reqDescr,
                ZBPRO_ZCL_WindowCoveringCmdConfParams_t *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Up/Open, Down/Close, or Stop
 *  command specific to Window Covering ZCL cluster.
 */
struct _ZBPRO_ZCL_WindowCoveringCmdReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_WindowCoveringCmdConfCallback_t   *callback;  /*!< ZCL Confirmation callback handler entry point. */

    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t         service;   /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_WindowCoveringCmdReqParams_t         params;  /*!< ZCL Request parameters structure. */
};

/*
 * Validate structure of ZCL Local Request Descriptor.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_WindowCoveringCmdReqDescr_t);

/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue GotoLiftPecentage, GotoTiltPecentage
 *  command specific to Window Covering ZCL cluster.
 * \details
 *  All the implemented commands - GotoLiftPecentage, GotoTiltPecentage - have the same format and differ only
 *  by the Command Id which is determined by using different primitives.
 * \details
 *  Commands GotoLiftPecentage, GotoTiltPecentage have one custom parameter: the pecent to go to.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 7.4.2.2.1, 7.4.2.2.2, 7.4.2.2.3.
 */
typedef struct _ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    uint8_t percent;                                                    /*!< The percent to ift or tilt to. */
} ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqParams_t;

/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqParams_t);

/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue GotoLiftPecentage, GotoTiltPecentage
 *  command specific to Window Covering ZCL cluster.
 */
typedef struct _ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t  ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of GotoLiftPecentage, GotoTiltPecentage
 *  commands specific to Window Covering ZCL cluster.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdConfCallback_t(
                ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t   *const  reqDescr,
                ZBPRO_ZCL_WindowCoveringCmdConfParams_t *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue GotoLiftPecentage, GotoTiltPecentage
 *  command specific to Window Covering ZCL cluster.
 */
struct _ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdConfCallback_t   *callback;  /*!< ZCL Confirmation callback handler entry point. */

    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t         service;   /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqParams_t         params;  /*!< ZCL Request parameters structure. */
};

/*
 * Validate structure of ZCL Local Request Descriptor.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t);


/************************* PROTOTYPES ***************************************************/
/**//**
 * \name    Functions accept ZCL Local Requests to issue Up/Open, Down/Close, Stop, GotoLiftPecentage, GotoTiltPecentage commands specific
 *  to Window Covering ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 * \details
 *  The caller shall specify the following obligatory parameters of request:
 *  - callback                      assign with ZCL Local Confirm handler function,
 *  - remoteApsAddress.addrMode     specify remote (destination) addressing mode,
 *  - respWaitTimeout               specify timeout of waiting for response, in seconds.
 *      Use value 0x0000 or 0xFFFF as well if default ZCL timeout shall be accepted. Note
 *      that the default ZCL timeout may be different for different commands,
 *  - localEndpoint                 specify endpoint on this node with ZCL-based profile
 *      which will be used as the source endpoint,
 *  - disableDefaultResp            set to TRUE if ZCL Default Response is necessarily
 *      needed even for the case of successful command processing on the remote node; set
 *      to FALSE if it's enough to have ZCL Default Response only for the case of failure
 *      on the remote node.
 *
 * \details
 *  For the case when remote (destination) node is bound to this (source) node, one may
 *  set the Remote Addressing Mode to NOT_PRESENT and specify only the Local Endpoint and
 *  Cluster on it. APS layer will then assume Remote node Address (extended or group) and
 *  Endpoint corresponding to the specified Local Endpoint according to the Binding Table.
 *  Otherwise, for direct addressing mode, the caller shall also specify the following
 *  parameters:
 *  - remoteApsAddress      specify destination address (either short, extended or group),
 *  - remoteEndpoint        specify destination endpoint on the remote node with the same
 *      ZCL-based profile as for the Local Endpoint.
 *
 * \details
 *  Following parameters are ignored even if specified by the caller and reassigned by
 *  this command handler:
 *  - localApsAddress       assigned by APS layer with this node address,
 *  - clusterId             assigned to On/Off Cluster Id,
 *  - manufCode             ignored because \c manufSpecific is assigned with FALSE,
 *  - commandId             assigned to Command Id of either Up/Open, Down/Close, or Stop command,
 *  - transSeqNum           ignored because \c useSpecifiedTsn is assigned with FALSE,
 *  - overhallStatus        just ignored,
 *  - direction             assigned to Client-to-Server,
 *  - clusterSpecific       assigned to TRUE (i.e. Cluster-Specific domain),
 *  - manufSpecific         assigned to FALSE (i.e., ZCL Standard type),
 *  - useSpecifiedTsn       assigned to FALSE (i.e., generate new TSN on transmission),
 *  - nonUnicastRequest     just ignored.
 */

/**@{*/

/**//**
 * \brief   Accepts ZCL Local Request to issue Up/Open command specific to Window Covering ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_WindowCoveringCmdUpOpenReq(
                ZBPRO_ZCL_WindowCoveringCmdReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Down/Close command specific to Window Covering ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_WindowCoveringCmdDownCloseReq(
                ZBPRO_ZCL_WindowCoveringCmdReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Stop command specific to Window Covering ZCL
 *  cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_WindowCoveringCmdStopReq(
                ZBPRO_ZCL_WindowCoveringCmdReqDescr_t *const  reqDescr);

/**//**
 * \brief   Accepts ZCL Local Request to issue GotoLiftPecentage command specific to Window Covering ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_WindowCoveringCmdGotoLiftPecentageReq(
                ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue GotoTiltPecentage command specific to Window Covering ZCL
 *  cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_WindowCoveringCmdGotoTiltPecentageReq(
                ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t *const  reqDescr);

/**@}*/


#endif /* _BB_ZBPRO_ZCL_SAP_CLUSTER_WINDOW_COVERING_H */
