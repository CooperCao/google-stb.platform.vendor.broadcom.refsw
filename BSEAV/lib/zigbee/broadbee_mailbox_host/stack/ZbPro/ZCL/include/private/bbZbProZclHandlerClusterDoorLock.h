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
*       ZCL Door Lock cluster handler private interface.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZCL_HANDLER_CLUSTER_DOOR_LOCK_H
#define _BB_ZBPRO_ZCL_HANDLER_CLUSTER_DOOR_LOCK_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZclSapClusterDoorLock.h"
#include "private/bbZbProZclDispatcher.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Assembles Door Lock cluster Command Uid.
 * \param[in]   direction       Either Client-to-Server (0x0) or Server-to-Client (0x1).
 * \param[in]   commandId       Numeric 8-bit value of Command Identifier local to Cluster
 *  and its Side.
 * \return  Numeric 64-bit of Command Uid.
 * \details
 *  Parameter \p direction specifies which side of cluster generates the command.
 */
#define ZBPRO_ZCL_MAKE_DOOR_LOCK_CLUSTER_COMMAND_UID(direction, commandId)\
        ZBPRO_ZCL_MAKE_CLUSTER_SPECIFIC_COMMAND_UID(\
                ZBPRO_ZCL_MAKE_COMMAND_UID_PREFIX(\
                        ZBPRO_ZCL_FRAME_TYPE_CLUSTER_SPECIFIC_COMMAND, ZBPRO_ZCL_CLUSTER_ID_DOOR_LOCK,\
                        ZBPRO_ZCL_FRAME_DOMAIN_ZCL_STANDARD, /*manufCode*/ 0),\
                direction, commandId)


/**//**
 * \name    Unique identifiers of Door Lock ZCL cluster commands.
 */
/**@{*/
#define ZBPRO_ZCL_DOOR_LOCK_CMD_UID_LOCK\
        ZBPRO_ZCL_MAKE_DOOR_LOCK_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER,\
                ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_LOCK)                    /*!< Lock command Id. */

#define ZBPRO_ZCL_DOOR_LOCK_CMD_UID_UNLOCK\
        ZBPRO_ZCL_MAKE_DOOR_LOCK_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER,\
                ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_UNLOCK)                  /*!< Unlock command Id. */

#define ZBPRO_ZCL_DOOR_LOCK_CMD_UID_LOCK_RESPONSE\
        ZBPRO_ZCL_MAKE_DOOR_LOCK_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT,\
                ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_LOCK_RESPONSE)           /*!< Lock Response command Id. */

#define ZBPRO_ZCL_DOOR_LOCK_CMD_UID_UNLOCK_RESPONSE\
        ZBPRO_ZCL_MAKE_DOOR_LOCK_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT,\
                ZBPRO_ZCL_SAP_DOOR_LOCK_CMD_ID_UNLOCK_RESPONSE)         /*!< Unlock Response command Id. */
/**@}*/


/************************* PROTOTYPES ***************************************************/
/*
 * Allocates dynamic memory and composes ZCL Frame Payload of lock/unlock command specific to
 * DoorLock ZCL cluster.
 */
bool zbProZclHandlerClusterDoorLockComposeLock(
    SYS_DataPointer_t                             *const  zclFramePayload,
    const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);

/*
 * Parses ZCL Frame Payload and dismisses dynamic memory of lock/unlock response command
 * specific to Door Lock ZCL cluster.
 */
void zbProZclHandlerClusterDoorLockParseLockResponse(
    SYS_DataPointer_t                             *const  zclFramePayload,
    ZbProZclLocalPrimitiveParamsPrototype_t       *const  indParams,
    const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);

#endif /* _BB_ZBPRO_ZCL_HANDLER_CLUSTER_DOOR_LOCK_H */

/* eof bbZbProZclHandlerClusterDoorLock.h */