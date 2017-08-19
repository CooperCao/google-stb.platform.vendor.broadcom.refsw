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
*       ZCL IAS Zone cluster SAP interface.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZCL_SAP_CLUSTER_IAS_ZONE_H
#define _BB_ZBPRO_ZCL_SAP_CLUSTER_IAS_ZONE_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZclCommon.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of attributes of IAS Zone ZCL cluster Server side.
 * \details
 *  These attributes are provided by Server side of the cluster.
 * \details
 *  IAS Zone ZCL cluster has no attributes provided by Client side.
 * \ingroup ZBPRO_ZCL_IASZoneAttr
 * \note
 *  This implementation of IAS WD ZCL cluster doesn't provide Server side. *
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 8.2.2.2, Table 8-3 and Table 8-7.
 */


typedef enum _ZBPRO_ZCL_SapIASZoneServerAttributeId_t
{
    ZBPRO_ZCL_SAP_IAS_ZONE_ATTR_ID_ZONE_STATE        =  0x0000,        /* ZoneState. */

    ZBPRO_ZCL_SAP_IAS_ZONE_ATTR_ID_ZONE_TYPE         =  0x0001,        /* ZoneType. */

    ZBPRO_ZCL_SAP_IAS_ZONE_ATTR_ID_ZONE_STATUS       =  0x0002,        /* ZoneStatus. */

    ZBPRO_ZCL_SAP_IAS_ZONE_ATTR_ID_IAS_CIE_ADDRESS   =  0x0010,        /* IAS_CIE_Address. */

    ZBPRO_ZCL_SAP_IAS_ZONE_ATTR_ID_ZONE_ID           =  0x0011         /* ZoneID. */

} ZBPRO_ZCL_SapIASZoneServerAttributeId_t;


/**//**
 * \brief   Enumerator for ZoneState Attribute.
 * \ingroup ZBPRO_ZCL_IASZoneAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 8.2.2.2.1.1, Table 8-4.
*/
typedef enum
{
     ZBPRO_ZCL_SAP_IAS_ZONE_ZONE_STATE_NOT_ENROLLED =   0x00,       /* Not enrolled. */

     ZBPRO_ZCL_SAP_IAS_ZONE_ZONE_STATE_ENROLLED     =   0x01        /* Enrolled (the client will react to
                                                                       Zone State Change Notification
                                                                       commands from the server). */
} ZBPRO_ZCL_SapIASZoneAttributeZoneState_t;


/**//**
 * \brief   Enumerator for ZoneType Attribute.
 * \ingroup ZBPRO_ZCL_IASZoneAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 8.2.2.2.1.2, Table 8-5.
*/
 typedef enum _ZBPRO_ZCL_SapIASZoneAttributeZoneType_t
{
    ZBPRO_ZCL_SAP_IAS_ZONE_ZONE_TYPE_STANDARD_CIE                  =   0x0000,      /* Standard CIE. */
    ZBPRO_ZCL_SAP_IAS_ZONE_ZONE_TYPE_MOTION_SENSOR                 =   0x000D,      /* Motion sensor. */
    ZBPRO_ZCL_SAP_IAS_ZONE_ZONE_TYPE_CONTACT_SWITCH                =   0x0015,      /* Contact switch. */
    ZBPRO_ZCL_SAP_IAS_ZONE_ZONE_TYPE_FIRE_SENSOR                   =   0x0028,      /* Fire sensor. */
    ZBPRO_ZCL_SAP_IAS_ZONE_ZONE_TYPE_WATER_SENSOR                  =   0x002A,      /* Water sensor. */
    ZBPRO_ZCL_SAP_IAS_ZONE_ZONE_TYPE_GAS_SENSOR                    =   0x002B,      /* Gas sensor. */
    ZBPRO_ZCL_SAP_IAS_ZONE_ZONE_TYPE_PERSONAL_EMERGENCY_DEVICE     =   0x002C,      /* Personal emergency device. */
    ZBPRO_ZCL_SAP_IAS_ZONE_ZONE_TYPE_VIBRATION_OR_MOVEMENT_SENSOR  =   0x002D,      /* Vibration/Movement sensor. */
    ZBPRO_ZCL_SAP_IAS_ZONE_ZONE_TYPE_REMOTE_CONTROL                =   0x010F,      /* Remote Control. */
    ZBPRO_ZCL_SAP_IAS_ZONE_ZONE_TYPE_KEY_FOB                       =   0x0115,      /* Key fob. */
    ZBPRO_ZCL_SAP_IAS_ZONE_ZONE_TYPE_KEYPAD                        =   0x021D,      /* Keypad.*/
    ZBPRO_ZCL_SAP_IAS_ZONE_ZONE_TYPE_STANDARD_WARNING_DEVICE       =   0x0225,      /* Standard Warning Device. */
    ZBPRO_ZCL_SAP_IAS_ZONE_ZONE_TYPE_INVALID_ZONE_TYPE             =   0xFFFF       /* Invalid Zone Type. */

} ZBPRO_ZCL_SapIASZoneAttributeZoneType_t;


/**//**
 * \brief   Data types shared by ZoneStatus attribute of IAS Zone cluster.
 * \ingroup ZBPRO_ZCL_IASZoneAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 8.2.2.2.1.3, Table 8-6.
 */
typedef  BitField16_t  ZBPRO_ZCL_SapIASZoneAttributeZoneStatus_t;       /*!< Shared data type for ZoneStatus attribute. */


/**//**
 * \brief   Data types shared by IAS_CIE_Address attribute of IAS Zone cluster.
 * \ingroup ZBPRO_ZCL_IASZoneAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 8.2.2.2.2, Table 8-7.
 */
typedef  uint64_t    ZBPRO_ZCL_SapIASZoneAttributeIASCIEAddress;        /*!< IAS_CIE_Address attribute. */


/**//**
 * \brief   Data types shared by ZoneID attribute of IAS Zone cluster.
 * \ingroup ZBPRO_ZCL_IASZoneAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 8.2.2.2.2, Table 8-7.
 */
typedef  uint8_t   ZBPRO_ZCL_SapIASZoneAttributeZoneID_t;                /*!< ZoneID attribute. */


/**//**
 * \brief   Enumeration of client-to-server commands specific to IAS Zone ZCL cluster.
 * \ingroup ZBPRO_ZCL_IASZoneAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 8.2.2.3, table 8-8.
 */
typedef enum _ZBPRO_ZCL_SapIASZoneClientToServerCommandId_t
{
    ZBPRO_ZCL_SAP_IAS_ZONE_CMD_ID_ZONE_ENROLL_RESPONSE                   =  0x00,     /*!< Zone Enroll Response. */

} ZBPRO_ZCL_SapIASZoneClientToServerCommandId_t;


/**//**
 * \brief   Enumeration of server-to-client commands specific to IAS Zone ZCL cluster.
 * \ingroup ZBPRO_ZCL_IASZoneAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 8.2.2.4, table 8-10.
 */
typedef enum _ZBPRO_ZCL_SapIASZoneServerToClientCommandId_t
{
    ZBPRO_ZCL_SAP_IAS_ZONE_CMD_ID_ZONE_STATUS_CHANGE_NOTIFICATION  =  0x00,   /*!< Zone Status Change Notification. */

    ZBPRO_ZCL_SAP_IAS_ZONE_CMD_ID_ZONE_ENROLL_REQUEST              =  0x01,   /*!< Zone Enroll Request. */

} ZBPRO_ZCL_SapIASZoneServerToClientCommandId_t;


/*-------------------------------- Zone Enroll request Cmd ------------------------------------------------------*/
/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Zone Enroll request command
 * specific to IAS Zone ZCL cluster.
 * \ingroup ZBPRO_ZCL_ZoneEnrollReq
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 8.2.2.4.2, figure 8-4.
 */
typedef struct _ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;     /*!< Set of obligatory parameters of ZCL public
                                                                          interface to local application. */
    /* Custom parameters. */

    /* 16-bit data. */

    ZBPRO_ZCL_SapIASZoneAttributeZoneType_t   zoneType;               /*!< Zone Type. */

    uint16_t                                  manufacturerCode;       /*!< Manufacturer Code. */

} ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestReqParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestReqParams_t);


/**//**
 * \brief  Structure for parameters of the local indication to the  Zone Enroll request command.
 * \note Parameters are the same as the parameters of the Zone Enroll request command.
 * \ingroup ZBPRO_ZCL_ZoneEnrollInd
 */
typedef   ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestReqParams_t   ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestIndParams_t;


ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestIndParams_t);


/*-------------------------------- Zone Enroll response Cmd ------------------------------------------------------*/
/**//**
 * \brief   Enumeration for Zone Enroll Response result codes.
 * \ingroup ZBPRO_ZCL_ZoneEnrollRespReq
 */
typedef enum _ZBPRO_ZCL_SapIASZoneEnrollResponseCodeType_t
{
    ZBPRO_ZCL_SAP_IAS_ZONE_ENROLL_RESPONSE_CODE_SUCCESS           =  0x00,      /*!< Success. */

    ZBPRO_ZCL_SAP_IAS_ZONE_ENROLL_RESPONSE_CODE_NOT_SUPPORTED     =  0x01,      /*!< Not supported. */

    ZBPRO_ZCL_SAP_IAS_ZONE_ENROLL_RESPONSE_CODE_NO_ENROLL_PERMIT  =  0x02,      /*!< No enroll permit. */

    ZBPRO_ZCL_SAP_IAS_ZONE_ENROLL_RESPONSE_CODE_TOO_MANY_ZONES    =  0x03       /*!< Too many zones. */

} ZBPRO_ZCL_SapIASZoneEnrollResponseCodeType_t;


SYS_DbgAssertStatic(1 == sizeof(ZBPRO_ZCL_SapIASZoneEnrollResponseCodeType_t));

/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Zone Enroll response command
 * specific to IAS Zone ZCL cluster.
 * \ingroup ZBPRO_ZCL_ZoneEnrollRespReq
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 8.2.2.3.1, figure 8-2.
 */
typedef struct _ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;     /*!< Set of obligatory parameters of ZCL public
                                                                          interface to local application. */
    /* Custom parameters. */

    /* 8-bit data. */

    ZBPRO_ZCL_SapIASZoneEnrollResponseCodeType_t   enrollResponseCode;     /*!< Enroll response code. */

    ZBPRO_ZCL_SapIASZoneAttributeZoneID_t          zoneID;                 /*!< ZoneID. */

} ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Zone Enroll response command
 * specific to IAS Zone ZCL cluster.
 * \ingroup ZBPRO_ZCL_ZoneEnrollRespConf
 * \note
 * Confirmation has no custom parameters.
 */
typedef struct _ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseConfParams_t
{
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */

    /* In specific requests there custom parameters must follow the obligatory ones. */

} ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseConfParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Zone Enroll response command
 * specific to IAS Zone ZCL cluster.
 * \ingroup ZBPRO_ZCL_ZoneEnrollRespReq
 */
typedef struct   _ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqDescr_t   ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Zone Enroll response command
 * specific to IAS Zone ZCL cluster.
 * \ingroup ZBPRO_ZCL_ZoneEnrollRespConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseConfCallback_t(
    ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqDescr_t     *const  reqDescr,
    ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseConfParams_t   *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Zone Enroll response command
 * specific to IAS Zone ZCL cluster.
 * \ingroup ZBPRO_ZCL_ZoneEnrollRespReq
 */
struct _ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseConfCallback_t     *callback;   /*!< ZCL Confirmation callback handler entry
                                                                               point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                      service;    /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqParams_t         params;     /*!< ZCL Request parameters structure. */
};


/*-------------------------------- Zone Status Change Notification Cmd ---------------------------------------------*/
/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Zone Status Change Notification command
 * specific to IAS Zone ZCL cluster.
 * \ingroup ZBPRO_ZCL_ZoneStatusChangeNotificationReq
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 8.2.2.4.1, figure 8-3.
 */
typedef struct _ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t       zclObligatoryPart; /*!< Set of obligatory parameters of ZCL public
                                                                          interface to local application. */
    /* Custom parameters. */

    /* 16-bit data. */

    ZBPRO_ZCL_SapIASZoneAttributeZoneStatus_t     zoneStatus;      /*!< The Zone Status field shall be the current
                                                                        value of the ZoneStatus attribute. */

    uint16_t                                      delay;           /*!< The Delay field is defined as the amount of
                                                                        time in quarter-seconds, from the moment when
                                                                        a change takes place in one or more bits of
                                                                        the Zone Status attribute and the successful
                                                                        transmission of the Zone Status
                                                                        Change Notification. */

    /* 8-bit data. */
    uint8_t                                       extendedStatus;  /*!< The Extended Status field is reserved for
                                                                        additional status information and shall
                                                                        be set to zero. */

    ZBPRO_ZCL_SapIASZoneAttributeZoneID_t         zoneID;          /*!< Zone ID is the index of the Zone in the
                                                                        CIE's zone table. If none is programmed,
                                                                        the Zone ID default value SHALL be indicated
                                                                        in this field. */

} ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationReqParams_t;


ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationReqParams_t);

/**//**
 * \brief Command Zone Status Change Notification send from server to client. The request parameters on the server
 * side are the indication parameters on the client side.
 * \ingroup ZBPRO_ZCL_ZoneStatusChangeNotificationInd
 */

typedef   ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationReqParams_t
    ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationIndParams_t;


ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationIndParams_t);


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Function accept ZCL Local Requests to issue Zone Enroll response command.
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \details
 *  The caller shall specify the following obligatory parameters of request:
 *  - callback                              assign with ZCL Local Confirm handler function,
 *  - remoteApsAddress.addrMode             specify remote (destination) addressing mode,
 *  - respWaitTimeout                       specify timeout for waiting for response, in seconds.
 *      Use value 0xFFFF if default ZCL timeout shall be accepted. Use value 0x0000 if ZCL
 *      layer shall not wait for response and issue confirmation right on confirmation
 *      from APS layer on request to send the command,
 *  - localEndpoint                 specify endpoint on this node with ZCL-based profile
 *      which will be used as the source endpoint,
 *  - disableDefaultResp            set to TRUE if ZCL Default Response is necessarily
 *      needed even for the case of successful command processing on the remote node; set
 *      to FALSE if it's enough to have ZCL Default Response only for the case of failure
 *      on the remote node.
 *   - useSpecifiedTsn       TRUE,
 *   - transSeqNum           set TSN from server Zone Enroll request command,
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
 *  - clusterId             assigned to Groups Cluster Id,
 *  - manufCode             ignored because \c manufSpecific is assigned with FALSE,
 *  - commandId             assigned to Command Id
 *  - overhallStatus        just ignored,
 *  - direction             assigned to Client-to-Server,
 *  - clusterSpecific       assigned to TRUE,
 *  - manufSpecific         assigned to FALSE (i.e., ZCL Standard type),
 *  - nonUnicastRequest     just ignored.
 */


/**//**
 * \brief   Accepts ZCL Local Request to issue Zone Enroll response command
 * specific to IAS Zone ZCL cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReq(ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqDescr_t *const  reqDescr);


/**//**
 * \brief   Handles ZCL Local Indication on reception of Zone Status Change Notification command
 * specific to IAS Zone ZCL cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \note
 *  The Zone Status Change Notification command is sent by server. Due
 *  to this reason client side service provides this function.
 * \return Nothing.
 */
void ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationInd(
    ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationIndParams_t   *const   indParams);


/**//**
 * \brief   Handles ZCL Local Indication on reception of Zone Enroll request command
 * specific to IAS Zone ZCL cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \note
 *  The Zone Enroll request command is sent by server. Due
 *  to this reason client side service provides this function.
 * \return Nothing.
 */
void ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestInd(
    ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestIndParams_t   *const   indParams);


#endif

/* eof bbZbProZclSapClusterIasZone.h */