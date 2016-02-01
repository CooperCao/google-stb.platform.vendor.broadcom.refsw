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
/*****************************************************************************
*
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   ZCL IAS Zone cluster handler private interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/
#ifndef _BB_ZBPRO_ZCL_HANDLER_CLUSTER_IAS_ZONE_H_
#define _BB_ZBPRO_ZCL_HANDLER_CLUSTER_IAS_ZONE_H_

/************************* INCLUDES *****************************************************/
#include "bbZbProZclSapClusterIasZone.h"
#include "bbZbProZclCommon.h"
#include "private/bbZbProZclDispatcher.h"

/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Assembles IAS Zone cluster Command Uid.
 * \param[in]   direction       Either Client-to-Server (0x0) or Server-to-Client (0x1).
 * \param[in]   commandId       Numeric 8-bit value of Command Identifier local to Cluster
 *  and its Side.
 * \return  Numeric 64-bit of Command Uid.
 * \details
 *  Parameter \p direction specifies which side of cluster generates the command.
 */
#define ZBPRO_ZCL_MAKE_IAS_ZONE_CLUSTER_COMMAND_UID(direction, commandId)\
        ZBPRO_ZCL_MAKE_CLUSTER_SPECIFIC_COMMAND_UID(\
                ZBPRO_ZCL_MAKE_COMMAND_UID_PREFIX(\
                        ZBPRO_ZCL_FRAME_TYPE_CLUSTER_SPECIFIC_COMMAND, ZBPRO_ZCL_CLUSTER_ID_IAS_ZONE,\
                        ZBPRO_ZCL_FRAME_DOMAIN_ZCL_STANDARD, /*manufCode*/ 0),\
                direction, commandId)


/**//**
 * \brief   Unique identifiers of IAS Zone ZCL cluster commands.
*/
/**@{*/
#define ZBPRO_ZCL_IAS_ZONE_CMD_UID_ZONE_STATUS_CHANGE_NOTIFICATION\
        ZBPRO_ZCL_MAKE_IAS_ZONE_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT,\
                ZBPRO_ZCL_SAP_IAS_ZONE_CMD_ID_ZONE_STATUS_CHANGE_NOTIFICATION)     /*!< Zone Status Change Notification
                                                                                      command Uid. */
#define ZBPRO_ZCL_IAS_ZONE_CMD_UID_ZONE_ENROLL_REQUEST\
        ZBPRO_ZCL_MAKE_IAS_ZONE_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT,\
                ZBPRO_ZCL_SAP_IAS_ZONE_CMD_ID_ZONE_ENROLL_REQUEST)                 /*!< Zone Enroll Request. */


#define ZBPRO_ZCL_IAS_ZONE_CMD_UID_ZONE_ENROLL_RESPONSE\
        ZBPRO_ZCL_MAKE_IAS_ZONE_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER,\
                ZBPRO_ZCL_SAP_IAS_ZONE_CMD_ID_ZONE_ENROLL_RESPONSE)                /*!< Zone Enroll Response. */
/**@}*/


/**//**
 * \brief List of Confirmation parameters for ZclDispatcherTables
*/
#define ZBPRO_ZCL_IAS_ZONE_CONF_PARAMETERS_LIST  \
    ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseConfParams_t    iasZoneCmdZoneEnrollResponse

/**//**
 * \brief List of Indication parameters for ZclDispatcherTables
*/
#define ZBPRO_ZCL_IAS_ZONE_INDICATION_PARAMETERS_LIST  \
    ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestIndParams_t               iasZoneCmdZoneEnrollRequest;        \
    ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationIndParams_t    iasZoneCmdStatusChangeNotification

/************************* PROTOTYPES ***************************************************/
/*-------------------------------- Zone Enroll request Cmd ------------------------------------------------------*/
/**//**
 * \brief   Parses ZCL Frame Payload of Zone Enroll Request command
 *  specific to IAS Zone ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload.
 * \param[out]      indParams           Pointer to ZCL Local Indication Parameters
 *  Prototype.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 8.2.2.4.2, figure 8-4
*/

void zbProZclHandlerClusterIASZoneParseZoneEnrollRequestInd(
            SYS_DataPointer_t                             *const  zclFramePayload,
            ZbProZclLocalPrimitiveParamsPrototype_t       *const  indParams);


/*-------------------------------- Zone Enroll response Cmd ------------------------------------------------------*/
/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of Zone Enroll Response command
 *  specific to IAS Zone ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully;
 *          FALSE if it has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 8.2.2.3.1, figure 8-2.
 */
bool zbProZclHandlerClusterIASZoneComposeZoneEnrollResponseRequest(
            SYS_DataPointer_t                               *const   zclFramePayload,
            const ZbProZclLocalPrimitiveParamsPrototype_t   *const   reqParams);



/*-------------------------------- Zone Status Change Notification Cmd ---------------------------------------------*/
/**//**
 * \brief   Parses ZCL Frame Payload of Zone Status Change Notification comand
 *  specific to IAS Zone ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload.
 * \param[out]      indParams           Pointer to ZCL Local Indication Parameters
 *  Prototype.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 8.2.2.4.1, figure 8-3.
*/

void zbProZclHandlerClusterIASZoneParseZoneStatusChangeNotificationInd(
        SYS_DataPointer_t                             *const  zclFramePayload,
        ZbProZclLocalPrimitiveParamsPrototype_t       *const  indParams);


/**//**
 * \brief   Internal indication function that handles ZCL Local Indication on reception
 *          of Zone Status Change Notification command specific to IAS Zone ZCL cluster.
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \note
 *  The Zone Status Change Notification command is sent by server. Due
 *  to this reason client side service provides this function.
 */
void ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationInternalInd(
    ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationIndParams_t   *const   indParams);

#endif
