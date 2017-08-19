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
*       Server side ZCL OTA Upgrade cluster private interface.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZCL_CLUSTER_OTAU_H
#define _BB_ZBPRO_ZCL_CLUSTER_OTAU_H

/************************* INCLUDES *****************************************************/
#include "bbZbProZclSapClusterOTAU.h"

/************************* DEFINITIONS **************************************************/
/**//**
 * \brief Enumeration for Image Type values.
 *  See ZigBee Document 075123r05, subclause 11.4.2.6, table 11-4.
 */
typedef enum _ZBPRO_ZCL_OTAUImageTypeEnum_t {
    ZBPRO_ZCL_OTAU_IMAGE_TYPE_MANUF_SPEC_FIRST        = 0x0000,
    ZBPRO_ZCL_OTAU_IMAGE_TYPE_MANUF_SPEC_LAST         = 0xffbf,
    ZBPRO_ZCL_OTAU_IMAGE_TYPE_CLIENT_SEC_CREDENTIALS  = 0xffc0,
    ZBPRO_ZCL_OTAU_IMAGE_TYPE_CLIENT_CONFIGURATION    = 0xffc1,
    ZBPRO_ZCL_OTAU_IMAGE_TYPE_SERVER_LOG              = 0xffc2,
    ZBPRO_ZCL_OTAU_IMAGE_TYPE_WILD_CARD               = 0xffff
} ZBPRO_ZCL_OTAUImageTypeEnum_t;

/**//**
 * \brief Enumeration of client-to-server commands
 */
typedef enum _ZBPRO_ZCL_OTAUClientToServerCommandId_t
{
    ZBPRO_ZCL_SAP_OTAU_CMD_ID_QUERY_NEXT_IMAGE_REQUEST = 0x01,
    ZBPRO_ZCL_SAP_OTAU_CMD_ID_IMAGE_BLOCK_REQUEST      = 0x03,
    ZBPRO_ZCL_SAP_OTAU_CMD_ID_UPGRADE_END_REQUEST      = 0x06

} ZBPRO_ZCL_OTAUClientToServerCommandId_t;

/**//**
 * \brief Enumeration of server-to-client commands
 */
typedef enum _ZBPRO_ZCL_OTAUServerToClientCommandId_t
{
    ZBPRO_ZCL_SAP_OTAU_CMD_ID_QUERY_NEXT_IMAGE_RESPONSE = 0x02,
    ZBPRO_ZCL_SAP_OTAU_CMD_ID_IMAGE_BLOCK_RESPONSE      = 0x05,
    ZBPRO_ZCL_SAP_OTAU_CMD_ID_UPGRADE_END_RESPONSE      = 0x07

} ZBPRO_ZCL_OTAUServerToClientCommandId_t;


/**//**
 * \brief   Assembles OTA Upgrade cluster Command Uid.
 * \param[in]   direction       Either Client-to-Server (0x0) or Server-to-Client (0x1).
 * \param[in]   commandId       Numeric 8-bit value of Command Identifier local to Cluster
 *  and its Side.
 * \return  Numeric 64-bit of Command Uid.
 * \details
 *  Parameter \p direction specifies which side of cluster generates the command.
 */
#define ZBPRO_ZCL_MAKE_OTAU_CLUSTER_COMMAND_UID(direction, commandId)\
        (ZBPRO_ZCL_MAKE_CLUSTER_SPECIFIC_COMMAND_UID(\
                ZBPRO_ZCL_MAKE_COMMAND_UID_PREFIX(\
                        ZBPRO_ZCL_FRAME_TYPE_CLUSTER_SPECIFIC_COMMAND, ZBPRO_ZCL_CLUSTER_ID_OTA_UPGRADE,\
                        ZBPRO_ZCL_FRAME_DOMAIN_ZCL_STANDARD, /*manufCode*/ 0),\
                direction, commandId))


/**//**
 * \name Unique identifiers of server received cluster commands
 */
/**@{*/
#define ZBPRO_ZCL_SAP_OTAU_CMD_UID_QUERY_NEXT_IMAGE_REQUEST \
        (ZBPRO_ZCL_MAKE_OTAU_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER, \
                ZBPRO_ZCL_SAP_OTAU_CMD_ID_QUERY_NEXT_IMAGE_REQUEST))

#define ZBPRO_ZCL_SAP_OTAU_CMD_UID_IMAGE_BLOCK_REQUEST \
        (ZBPRO_ZCL_MAKE_OTAU_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER, \
                ZBPRO_ZCL_SAP_OTAU_CMD_ID_IMAGE_BLOCK_REQUEST))

#define ZBPRO_ZCL_SAP_OTAU_CMD_UID_UPGRADE_END_REQUEST \
        (ZBPRO_ZCL_MAKE_OTAU_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER, \
                ZBPRO_ZCL_SAP_OTAU_CMD_ID_UPGRADE_END_REQUEST))

/**//**
 * \name Unique identifiers of server generated cluster commands
 */
/**@{*/
#define ZBPRO_ZCL_SAP_OTAU_CMD_UID_QUERY_NEXT_IMAGE_RESPONSE \
        (ZBPRO_ZCL_MAKE_OTAU_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT, \
                ZBPRO_ZCL_SAP_OTAU_CMD_ID_QUERY_NEXT_IMAGE_RESPONSE))

#define ZBPRO_ZCL_SAP_OTAU_CMD_UID_IMAGE_BLOCK_RESPONSE \
        (ZBPRO_ZCL_MAKE_OTAU_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT, \
                ZBPRO_ZCL_SAP_OTAU_CMD_ID_IMAGE_BLOCK_RESPONSE))

#define ZBPRO_ZCL_SAP_OTAU_CMD_UID_UPGRADE_END_RESPONSE \
        (ZBPRO_ZCL_MAKE_OTAU_CLUSTER_COMMAND_UID( \
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT, \
                ZBPRO_ZCL_SAP_OTAU_CMD_ID_UPGRADE_END_RESPONSE))


/************************* PROTOTYPES ***************************************************/
/**//**
 * \name Serializing function of server received cluster commands
 */
/**@{*/
/**//**
  \brief OTAU Cluster Query Next Image Request command deserializer

  \param[in] zclFramePayload    - payload to deserialize from
  \param[in] indParams          - indication parameters to deserialize into
*/
void zbProZclOTAUQueryNextImageRequestCmdParse(
        SYS_DataPointer_t                       *const  zclFramePayload,
        ZbProZclLocalPrimitiveParamsPrototype_t *const  indParams);
/**//**
  \brief OTAU Cluster Image Block Request command deserializer

  \param[in] zclFramePayload    - payload to deserialize from
  \param[in] indParams          - indication parameters to deserialize into
*/
void zbProZclOTAUImageBlockRequestCmdParse(
        SYS_DataPointer_t                       *const  zclFramePayload,
        ZbProZclLocalPrimitiveParamsPrototype_t *const  indParams);
/**//**
  \brief OTAU Cluster Upgrade End Request command deserializer

  \param[in] zclFramePayload    - payload to deserialize from
  \param[in] indParams          - indication parameters to deserialize into
*/
void zbProZclOTAUUpgradeEndRequestCmdParse(
        SYS_DataPointer_t                       *const  zclFramePayload,
        ZbProZclLocalPrimitiveParamsPrototype_t *const  indParams);
/**@}*/

/**//**
 * \name Serializing function of server generated cluster commands
 */
/**@{*/
/**//**
  \brief ZCL OTAU Cluster Query Next Image Response command serializer

  \param[in] zclFramePayload    - payload to serialize into
  \param[in] reqParams          - request parameters to serialize from
*/
bool zbProZclOTAUQueryNextImageResponseCompose(
        SYS_DataPointer_t                             *const  zclFramePayload,
        const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);
/**//**
  \brief ZCL OTAU Cluster Image Block Response command serializer

  \param[in] zclFramePayload    - payload to serialize into
  \param[in] reqParams          - request parameters to serialize from
*/
bool zbProZclOTAUImageBlockResponseCompose(
        SYS_DataPointer_t                             *const  zclFramePayload,
        const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);
/**//**
  \brief ZCL OTAU Cluster Upgrade End Response command serializer

  \param[in] zclFramePayload    - payload to serialize into
  \param[in] reqParams          - request parameters to serialize from
*/
bool zbProZclOTAUUpgradeEndResponseCompose(
        SYS_DataPointer_t                             *const  zclFramePayload,
        const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);
/**@{*/

#endif /* _BB_ZBPRO_ZCL_CLUSTER_OTAU_H */

/* eof bbZbProZclHandlerClusterOTAU.h */