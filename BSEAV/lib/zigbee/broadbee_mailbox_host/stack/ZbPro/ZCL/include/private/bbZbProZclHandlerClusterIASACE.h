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
*       Server side ZCL IAS ACE cluster private interface.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZCL_CLUSTER_IAS_ACE_H
#define _BB_ZBPRO_ZCL_CLUSTER_IAS_ACE_H

/************************* INCLUDES *****************************************************/

#include "bbZbProZclSapClusterIasAce.h"
#include "private/bbZbProZclDispatcher.h"

/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Assembles IAS ACE cluster Command Uid.
 * \param[in]   direction       Either Client-to-Server (0x0) or Server-to-Client (0x1).
 * \param[in]   commandId       Numeric 8-bit value of Command Identifier local to Cluster
 *  and its Side.
 * \return  Numeric 64-bit of Command Uid.
 * \details
 *  Parameter \p direction specifies which side of cluster generates the command.
 */
#define ZBPRO_ZCL_MAKE_IAS_ACE_CLUSTER_COMMAND_UID(direction, commandId)\
        ZBPRO_ZCL_MAKE_CLUSTER_SPECIFIC_COMMAND_UID(\
                ZBPRO_ZCL_MAKE_COMMAND_UID_PREFIX(\
                        ZBPRO_ZCL_FRAME_TYPE_CLUSTER_SPECIFIC_COMMAND, ZBPRO_ZCL_CLUSTER_ID_IAS_ACE,\
                        ZBPRO_ZCL_FRAME_DOMAIN_ZCL_STANDARD, /*manufCode*/ 0),\
                direction, commandId)


/**//**
 * \name Unique identifiers of server received cluster commands
 */
/**@{*/
#define ZBPRO_ZCL_SAP_IAS_ACE_CMD_UID_ARM \
        ZBPRO_ZCL_MAKE_IAS_ACE_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER, \
                ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_ARM)

#define ZBPRO_ZCL_SAP_IAS_ACE_CMD_UID_BYPASS \
        ZBPRO_ZCL_MAKE_IAS_ACE_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER, \
                ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_BYPASS)

#define ZBPRO_ZCL_SAP_IAS_ACE_CMD_UID_EMERGENCY \
        ZBPRO_ZCL_MAKE_IAS_ACE_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER, \
                ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_EMERGENCY)

#define ZBPRO_ZCL_SAP_IAS_ACE_CMD_UID_FIRE \
        ZBPRO_ZCL_MAKE_IAS_ACE_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER, \
                ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_FIRE)

#define ZBPRO_ZCL_SAP_IAS_ACE_CMD_UID_PANIC \
        ZBPRO_ZCL_MAKE_IAS_ACE_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER, \
                ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_PANIC)

#define ZBPRO_ZCL_SAP_IAS_ACE_CMD_UID_GET_ZONE_ID_MAP \
        ZBPRO_ZCL_MAKE_IAS_ACE_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER, \
                ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_GET_ZONE_ID_MAP)

#define ZBPRO_ZCL_SAP_IAS_ACE_CMD_UID_GET_ZONE_INFORMATION \
        ZBPRO_ZCL_MAKE_IAS_ACE_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER, \
                ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_GET_ZONE_INFORMATION)

#define ZBPRO_ZCL_SAP_IAS_ACE_CMD_UID_GET_PANEL_STATUS \
        ZBPRO_ZCL_MAKE_IAS_ACE_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER, \
                ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_GET_PANEL_STATUS)

#define ZBPRO_ZCL_SAP_IAS_ACE_CMD_UID_GET_BYPASSED_ZONE_LIST \
        ZBPRO_ZCL_MAKE_IAS_ACE_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER, \
                ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_GET_BYPASSED_ZONE_LIST)

#define ZBPRO_ZCL_SAP_IAS_ACE_CMD_UID_GET_ZONE_STATUS \
        ZBPRO_ZCL_MAKE_IAS_ACE_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER, \
                ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_GET_ZONE_STATUS)
/**@}*/

/**//**
 * \name Unique identifiers of server generated cluster commands
 */
/**@{*/
#define ZBPRO_ZCL_SAP_IAS_ACE_CMD_UID_ARM_RESPONSE \
        ZBPRO_ZCL_MAKE_IAS_ACE_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT, \
                ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_ARM_RESPONSE)

#define ZBPRO_ZCL_SAP_IAS_ACE_CMD_UID_GET_ZONE_ID_MAP_RESPONSE \
        ZBPRO_ZCL_MAKE_IAS_ACE_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT, \
                ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_GET_ZONE_ID_MAP_RESPONSE)

#define ZBPRO_ZCL_SAP_IAS_ACE_CMD_UID_GET_ZONE_INFORMATION_RESPONSE \
        ZBPRO_ZCL_MAKE_IAS_ACE_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT, \
                ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_GET_ZONE_INFORMATION_RESPONSE)

#define ZBPRO_ZCL_SAP_IAS_ACE_CMD_UID_ZONE_STATUS_CHANGED \
        ZBPRO_ZCL_MAKE_IAS_ACE_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT, \
                ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_ZONE_STATUS_CHANGED)

#define ZBPRO_ZCL_SAP_IAS_ACE_CMD_UID_PANEL_STATUS_CHANGED \
        ZBPRO_ZCL_MAKE_IAS_ACE_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT, \
                ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_PANEL_STATUS_CHANGED)

#define ZBPRO_ZCL_SAP_IAS_ACE_CMD_UID_GET_PANEL_STATUS_RESPONSE \
        ZBPRO_ZCL_MAKE_IAS_ACE_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT, \
                ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_GET_PANEL_STATUS_RESPONSE)

#define ZBPRO_ZCL_SAP_IAS_ACE_CMD_UID_SET_BYPASSED_ZONE_LIST \
        ZBPRO_ZCL_MAKE_IAS_ACE_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT, \
                ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_SET_BYPASSED_ZONE_LIST)

#define ZBPRO_ZCL_SAP_IAS_ACE_CMD_UID_BYPASS_RESPONSE \
        ZBPRO_ZCL_MAKE_IAS_ACE_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT, \
                ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_BYPASS_RESPONSE)

#define ZBPRO_ZCL_SAP_IAS_ACE_CMD_UID_GET_ZONE_STATUS_RESPONSE \
        ZBPRO_ZCL_MAKE_IAS_ACE_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT, \
                ZBPRO_ZCL_SAP_IAS_ACE_CMD_ID_GET_ZONE_STATUS_RESPONSE)
/**@}*/

/**//**
 * \brief List of Indication parameters
 */
#define ZBPRO_ZCL_SAP_IAS_ACE_INDICATION_PARAMETERS_LIST \
    ZBPRO_ZCL_SapIasAceArmIndParams_t                   ZBPRO_ZCL_SapIasAceArmIndParams; \
    ZBPRO_ZCL_SapIasAceBypassIndParams_t                ZBPRO_ZCL_SapIasAceBypassIndParams; \
    ZBPRO_ZCL_SapIasAceAlarmIndParams_t                 ZBPRO_ZCL_SapIasAceAlarmIndParams; \
    ZBPRO_ZCL_SapIasAceGetZoneIdMapIndParams_t          ZBPRO_ZCL_SapIasAceGetZoneIdMapIndParams; \
    ZBPRO_ZCL_SapIasAceGetZoneInfoIndParams_t           ZBPRO_ZCL_SapIasAceGetZoneInfoIndParams; \
    ZBPRO_ZCL_SapIasAceGetPanelStatusIndParams_t        ZBPRO_ZCL_SapIasAceGetPanelStatusIndParams; \
    ZBPRO_ZCL_SapIasAceGetBypassedZoneListIndParams_t   ZBPRO_ZCL_SapIasAceGetBypassedZoneListIndParams; \
    ZBPRO_ZCL_SapIasAceGetZoneStatusIndParams_t         ZBPRO_ZCL_SapIasAceGetZoneStatusIndParams

/**//**
 * \brief List of Confirmation parameters
 */
#define ZBPRO_ZCL_SAP_IAS_ACE_CONF_PARAMETERS_LIST \
    ZBPRO_ZCL_SapIasAceRespReqConfParams_t              ZBPRO_ZCL_SapIasAceRespReqConfParams_t

/************************* PROTOTYPES ***************************************************/

/**//**
 * \name Serializing function of server received cluster commands
 */
/**@{*/
void zbProZclIasAceArmCmdParse(
        SYS_DataPointer_t                       *const  zclFramePayload,
        ZbProZclLocalPrimitiveParamsPrototype_t *const  indParams);
void zbProZclIasAceBypassCmdParse(
        SYS_DataPointer_t                       *const  zclFramePayload,
        ZbProZclLocalPrimitiveParamsPrototype_t *const  indParams);
void zbProZclIasAceGetZoneInfoCmdParse(
        SYS_DataPointer_t                       *const  zclFramePayload,
        ZbProZclLocalPrimitiveParamsPrototype_t *const  indParams);
void zbProZclIasAceGetZoneStatusCmdParse(
        SYS_DataPointer_t                       *const  zclFramePayload,
        ZbProZclLocalPrimitiveParamsPrototype_t *const  indParams);
/**@}*/

/**//**
 * \name Serializing function of server generated cluster commands
 */
/**@{*/
bool zbProZclIasAceArmRespCompose(
        SYS_DataPointer_t                             *const  zclFramePayload,
        const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);
bool zbProZclIasAceBypassRespCompose(
        SYS_DataPointer_t                             *const  zclFramePayload,
        const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);
bool zbProZclIasAceGetZoneIdMapRespCompose(
        SYS_DataPointer_t                             *const  zclFramePayload,
        const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);
bool zbProZclIasAceGetZoneInfoRespCompose(
        SYS_DataPointer_t                             *const  zclFramePayload,
        const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);
bool zbProZclIasAceGetPanelStatusRespCompose(
        SYS_DataPointer_t                             *const  zclFramePayload,
        const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);
bool zbProZclIasAceSetBypassedZoneListRespCompose(
        SYS_DataPointer_t                             *const  zclFramePayload,
        const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);
bool zbProZclIasAceGetZoneStatusRespCompose(
        SYS_DataPointer_t                             *const  zclFramePayload,
        const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);
bool zbProZclIasAceZoneStatusChangedCompose(
        SYS_DataPointer_t                             *const  zclFramePayload,
        const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);
bool zbProZclIasAcePanelStatusChangedCompose(
        SYS_DataPointer_t                             *const  zclFramePayload,
        const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);
/**@}*/

#endif /* _BB_ZBPRO_ZCL_CLUSTER_IAS_ACE_H */

/* eof bbZbProZclHandlerClusterIASACE.h */