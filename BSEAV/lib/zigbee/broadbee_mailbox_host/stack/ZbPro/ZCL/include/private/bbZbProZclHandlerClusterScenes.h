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
* FILENAME: $Workfile: trunk/stack/ZbPro/ZCL/include/private/bbZbProZclHandlerClusterScenes.h $
*
* DESCRIPTION:
*   ZCL Scenes cluster handler private interface.
*
* $Revision: 7831 $
* $Date: 2015-07-31 13:31:38Z $
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZCL_HANDLER_CLUSTER_SCENES_H
#define _BB_ZBPRO_ZCL_HANDLER_CLUSTER_SCENES_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZclSapClusterScenes.h"
#include "private/bbZbProZclDispatcher.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Assembles Scenes cluster Command Uid.
 * \param[in]   direction       Either Client-to-Server (0x0) or Server-to-Client (0x1).
 * \param[in]   commandId       Numeric 8-bit value of Command Identifier local to Cluster
 *  and its Side.
 * \return  Numeric 64-bit of Command Uid.
 * \details
 *  Parameter \p direction specifies which side of cluster generates the command.
 */
#define ZBPRO_ZCL_MAKE_SCENES_CLUSTER_COMMAND_UID(direction, commandId)\
        ZBPRO_ZCL_MAKE_CLUSTER_SPECIFIC_COMMAND_UID(\
                ZBPRO_ZCL_MAKE_COMMAND_UID_PREFIX(\
                        ZBPRO_ZCL_FRAME_TYPE_CLUSTER_SPECIFIC_COMMAND, ZBPRO_ZCL_CLUSTER_ID_SCENES,\
                        ZBPRO_ZCL_FRAME_DOMAIN_ZCL_STANDARD, /*manufCode*/ 0),\
                direction, commandId)


/**//**
 * \name    Unique identifiers of Scenes ZCL cluster commands.
 */
/**@{*/
#define ZBPRO_ZCL_SCENES_CMD_UID_ADD_SCENE\
        ZBPRO_ZCL_MAKE_SCENES_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER,\
                ZBPRO_ZCL_SAP_SCENE_CMD_ID_ADD_SCENE)                       /*!< AddScene Command Uid. */

#define ZBPRO_ZCL_SCENES_CMD_UID_VIEW_SCENE\
        ZBPRO_ZCL_MAKE_SCENES_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER,\
                ZBPRO_ZCL_SAP_SCENE_CMD_ID_VIEW_SCENE)                      /*!< ViewScene Command Uid. */

#define ZBPRO_ZCL_SCENES_CMD_UID_REMOVE_SCENE\
        ZBPRO_ZCL_MAKE_SCENES_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER,\
                ZBPRO_ZCL_SAP_SCENE_CMD_ID_REMOVE_SCENE)                    /*!< RemoveScene Command Uid. */

#define ZBPRO_ZCL_SCENES_CMD_UID_REMOVE_ALL_SCENES\
        ZBPRO_ZCL_MAKE_SCENES_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER,\
                ZBPRO_ZCL_SAP_SCENE_CMD_ID_REMOVE_ALL_SCENES)               /*!< RemoveAllScenes Command Uid. */

#define ZBPRO_ZCL_SCENES_CMD_UID_STORE_SCENE\
        ZBPRO_ZCL_MAKE_SCENES_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER,\
                ZBPRO_ZCL_SAP_SCENE_CMD_ID_STORE_SCENE)                     /*!< StoreScene Command Uid. */

#define ZBPRO_ZCL_SCENES_CMD_UID_RECALL_SCENE\
        ZBPRO_ZCL_MAKE_SCENES_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER,\
                ZBPRO_ZCL_SAP_SCENE_CMD_ID_RECALL_SCENE)                    /*!< RecallScene Command Uid. */

#define ZBPRO_ZCL_SCENES_CMD_UID_GET_SCENE_MEMBERSHIP\
        ZBPRO_ZCL_MAKE_SCENES_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER,\
                ZBPRO_ZCL_SAP_SCENE_CMD_ID_GET_SCENE_MEMBERSHIP)            /*!< GetSceneMembership Command Uid. */


#define ZBPRO_ZCL_SCENES_CMD_UID_ADD_SCENE_RESPONSE\
        ZBPRO_ZCL_MAKE_SCENES_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT,\
                ZBPRO_ZCL_SAP_SCENE_CMD_ID_ADD_SCENE_RESPONSE)              /*!< AddScene Response Command Uid. */

#define ZBPRO_ZCL_SCENES_CMD_UID_VIEW_SCENE_RESPONSE\
        ZBPRO_ZCL_MAKE_SCENES_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT,\
                ZBPRO_ZCL_SAP_SCENE_CMD_ID_VIEW_SCENE_RESPONSE)             /*!< ViewScene Response Command Uid. */

#define ZBPRO_ZCL_SCENES_CMD_UID_STORE_SCENE_RESPONSE\
        ZBPRO_ZCL_MAKE_SCENES_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT,\
                ZBPRO_ZCL_SAP_SCENE_CMD_ID_STORE_SCENE_RESPONSE)            /*!< StoreScene Response Command Uid. */

#define ZBPRO_ZCL_SCENES_CMD_UID_REMOVE_SCENE_RESPONSE\
        ZBPRO_ZCL_MAKE_SCENES_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT,\
                ZBPRO_ZCL_SAP_SCENE_CMD_ID_REMOVE_SCENE_RESPONSE)           /*!< RemoveScene Response Command Uid. */

#define ZBPRO_ZCL_SCENES_CMD_UID_REMOVE_ALL_SCENES_RESPONSE\
        ZBPRO_ZCL_MAKE_SCENES_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT,\
                ZBPRO_ZCL_SAP_SCENE_CMD_ID_REMOVE_ALL_SCENES_RESPONSE)      /*!< RemoveAllScenes Response Command Uid. */

#define ZBPRO_ZCL_SCENES_CMD_UID_GET_SCENE_MEMBERSHIP_RESPONSE\
        ZBPRO_ZCL_MAKE_SCENES_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT,\
                ZBPRO_ZCL_SAP_SCENE_CMD_ID_GET_SCENE_MEMBERSHIP_RESPONSE)   /*!< GetSceneMembership Response Command Uid. */
/**@}*/


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of AddScene command
 *  specific to Scenes ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully; FALSE if it
 *  has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.7.2.4.2.1, figure 3-18.
 */
bool zbProZclHandlerClusterScenesComposeAddScene(
                SYS_DataPointer_t                             *const  zclFramePayload,
                const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);


/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of ViewScene command
 *  specific to Scenes ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully; FALSE if it
 *  has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.7.2.4.3.1, figure 3-19.
 */
bool zbProZclHandlerClusterScenesComposeViewScene(
                SYS_DataPointer_t                             *const  zclFramePayload,
                const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);

/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of RemoveAllScenes command
 *  specific to Scenes ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully; FALSE if it
 *  has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.7.2.4.5.1, figure 3-21.
 */
bool zbProZclHandlerClusterScenesComposeRemoveAllScenes(
                SYS_DataPointer_t                             *const  zclFramePayload,
                const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);

/**//**
 * \brief   Parses ZCL Frame Payload and dismisses dynamic memory of AddSceneResponse
 *  command specific to Scenes ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload and dismissed finally.
 * \param[out]      indParams           Pointer to ZCL Local Indication Parameters
 *  Prototype.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.5.2.5.1.1, figure 3-27.
 */
void zbProZclHandlerClusterScenesParseAddSceneResponse(
                SYS_DataPointer_t                             *const  zclFramePayload,
                ZbProZclLocalPrimitiveParamsPrototype_t       *const  indParams,
                const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);

/**//**
 * \brief   Parses ZCL Frame Payload and dismisses dynamic memory of ViewSceneResponse
 *  command specific to Scenes ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload and dismissed finally.
 * \param[out]      indParams           Pointer to ZCL Local Indication Parameters
 *  Prototype.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.5.2.5.2.1, figure 3-28.
 */
void zbProZclHandlerClusterScenesParseViewSceneResponse(
                SYS_DataPointer_t                             *const  zclFramePayload,
                ZbProZclLocalPrimitiveParamsPrototype_t       *const  indParams,
                const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);

/**//**
 * \brief   Parses ZCL Frame Payload and dismisses dynamic memory of RemoveAllScenesResponse
 *  command specific to Scenes ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload and dismissed finally.
 * \param[out]      indParams           Pointer to ZCL Local Indication Parameters
 *  Prototype.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.5.2.5.4.1, figure 3-30.
 */
void zbProZclHandlerClusterScenesParseRemoveAllScenesResponse(
                SYS_DataPointer_t                       *const  zclFramePayload,
                ZbProZclLocalPrimitiveParamsPrototype_t *const  indParams);

/**//**
 * \brief   Parses ZCL Frame Payload and dismisses dynamic memory of RemoveAllScenesResponse
 *  command specific to Scenes ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload and dismissed finally.
 * \param[out]      indParams           Pointer to ZCL Local Indication Parameters
 *  Prototype.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.5.2.5.4.1, figure 3-30.
 */
void zbProZclHandlerClusterScenesParseRemoveAllScenesResponseForConf(
                SYS_DataPointer_t                             *const  zclFramePayload,
                ZbProZclLocalPrimitiveParamsPrototype_t       *const  indParams,
                const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);

/**//**
 * \brief   Parses ZCL Frame Payload and dismisses dynamic memory of GetSceneMembershipResponse
 *  command specific to Scenes ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload and dismissed finally.
 * \param[out]      indParams           Pointer to ZCL Local Indication Parameters
 *  Prototype.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.5.2.5.6.1, figure 3-32.
 */
void zbProZclHandlerClusterScenesParseGetSceneMembershipResponse(
                SYS_DataPointer_t                       *const  zclFramePayload,
                ZbProZclLocalPrimitiveParamsPrototype_t *const  indParams);

#endif /* _BB_ZBPRO_ZCL_HANDLER_CLUSTER_SCENES_H */
