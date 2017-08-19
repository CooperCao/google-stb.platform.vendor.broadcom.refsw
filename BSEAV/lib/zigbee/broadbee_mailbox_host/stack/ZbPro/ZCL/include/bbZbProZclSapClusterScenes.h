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
*       ZCL Scenes cluster SAP interface.
*
*******************************************************************************/

#ifndef BBZBPROZCLSAPCLUSTERSCENES_H
#define BBZBPROZCLSAPCLUSTERSCENES_H

/************************* INCLUDES *****************************************************/
#include "bbZbProZclCommon.h"


/************************* DEFINITIONS **************************************************/
#define ZBPRO_ZCL_SCENE_MANAGEMENT_INFORMATION 0x0000
#define ZBPRO_ZCL_SCENE_MAKE_ATTRIBUTE_ID(set, id) (((set) << 4) | ((id) & 0xF))
/**//**
 * \brief   Enumeration of attributes of Scenes ZCL cluster Server side.
 * \details
 *  These attributes are provided by Server side of the cluster.
 * \details
 *  Identify ZCL cluster has no attributes provided by Client side.
 * \ingroup ZBPRO_ZCL_ScenesAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.7.2.2, 3.7.2.2.1, tables 3-36 3-37.
 */
typedef enum _ZBPRO_ZCL_SapScenesServerAttributeId_t
{
    ZBPRO_ZCL_SAP_SCENE_ATTR_ID_SCENE_COUNT        = ZBPRO_ZCL_SCENE_MAKE_ATTRIBUTE_ID(ZBPRO_ZCL_SCENE_MANAGEMENT_INFORMATION, 0x0000),       /*!< SceneCount. */
    ZBPRO_ZCL_SAP_SCENE_ATTR_ID_CURRENT_SCENE      = ZBPRO_ZCL_SCENE_MAKE_ATTRIBUTE_ID(ZBPRO_ZCL_SCENE_MANAGEMENT_INFORMATION, 0x0001),       /*!< CurrentScene. */
    ZBPRO_ZCL_SAP_SCENE_ATTR_ID_CURRENT_GROUP      = ZBPRO_ZCL_SCENE_MAKE_ATTRIBUTE_ID(ZBPRO_ZCL_SCENE_MANAGEMENT_INFORMATION, 0x0002),       /*!< CurrentGroup. */
    ZBPRO_ZCL_SAP_SCENE_ATTR_ID_SCENE_VALID        = ZBPRO_ZCL_SCENE_MAKE_ATTRIBUTE_ID(ZBPRO_ZCL_SCENE_MANAGEMENT_INFORMATION, 0x0003),       /*!< SceneValid. */
    ZBPRO_ZCL_SAP_SCENE_ATTR_ID_NAME_SUPPORT       = ZBPRO_ZCL_SCENE_MAKE_ATTRIBUTE_ID(ZBPRO_ZCL_SCENE_MANAGEMENT_INFORMATION, 0x0004),       /*!< NameSupport. */
    ZBPRO_ZCL_SAP_SCENE_ATTR_ID_LAST_CONFIGURED_BY = ZBPRO_ZCL_SCENE_MAKE_ATTRIBUTE_ID(ZBPRO_ZCL_SCENE_MANAGEMENT_INFORMATION, 0x0005),       /*!< LastConfiguredBy. */
    ZBPRO_ZCL_SAP_SCENE_ATTR_ID_MAX                = 0xFFFF,       /*!< Introduced only to make the enumeration 16-bit wide. */
} ZBPRO_ZCL_SapScenesServerAttributeId_t;


/**//**
 * \brief   Data types shared by attributes and command parameters of Scenes cluster.
 * \par     Documentation
 * \ingroup ZBPRO_ZCL_ScenesAttr
 *  See ZigBee Document 075123r05, subclause 3.7.2.2.1,
 */
typedef uint8_t  ZBPRO_ZCL_SapSceneParamSceneCount_t;

/**//**
 * \brief   Data types shared by attributes and command parameters of Scenes cluster.
 * \par     Documentation
 * \ingroup ZBPRO_ZCL_ScenesAttr
 *  See ZigBee Document 075123r05, subclause 3.7.2.2.1,
 */
typedef uint8_t  ZBPRO_ZCL_SapSceneParamCurrentScene_t;

/**//**
 * \brief   Data types shared by attributes and command parameters of Scenes cluster.
 * \par     Documentation
 * \ingroup ZBPRO_ZCL_ScenesAttr
 *  See ZigBee Document 075123r05, subclause 3.7.2.2.1,
 */
typedef uint16_t  ZBPRO_ZCL_SapSceneParamCurrentGroup_t;

/**//**
 * \brief   Data types shared by attributes and command parameters of Scenes cluster.
 * \par     Documentation
 * \ingroup ZBPRO_ZCL_ScenesAttr
 *  See ZigBee Document 075123r05, subclause 3.7.2.2.1,
 */
typedef Bool8_t  ZBPRO_ZCL_SapSceneParamSceneValid_t;

/**//**
 * \brief   Data types shared by attributes and command parameters of Scenes cluster.
 * \par     Documentation
 * \ingroup ZBPRO_ZCL_ScenesAttr
 *  See ZigBee Document 075123r05, subclause 3.7.2.2.1,
 */
typedef uint8_t  ZBPRO_ZCL_SapSceneParamNameSupport_t;

/**//**
 * \brief   Data types shared by attributes and command parameters of Scenes cluster.
 * \par     Documentation
 * \ingroup ZBPRO_ZCL_ScenesAttr
 *  See ZigBee Document 075123r05, subclause 3.7.2.2.1,
 */
typedef uint64_t  ZBPRO_ZCL_SapSceneParamLastConfiguredBy_t;


/**//**                                                                                                                 // TODO: Move to private header.
 * \brief   Enumeration of client-to-server commands specific to Scenes ZCL cluster.
 * \details
 *  These commands are generated by Scenes ZCL cluster Client side and received by
 *  Server side.
 * \ingroup ZBPRO_ZCL_ScenesAttr
 * \note
 *  This implementation of Scenes ZCL cluster doesn't provide generation and reception
 *  of any of the optional commands, it's able to generate and receive only the following
 *  mandatory commands:
 *  - AddScene,
 *  - ViewScene,
 *  - RemoveScene,
 *  - RemoveAllScenes,
 *  - StoreScene,
 *  - RecallScene,
 *  - GetSceneMembership.
 *
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.7.2.4, table 3-39.
 */
typedef enum _ZBPRO_ZCL_SapScenesClientToServerCommandId_t
{
    ZBPRO_ZCL_SAP_SCENE_CMD_ID_ADD_SCENE            = 0x00,     /*!< AddScene. */
    ZBPRO_ZCL_SAP_SCENE_CMD_ID_VIEW_SCENE           = 0x01,     /*!< ViewScene. */
    ZBPRO_ZCL_SAP_SCENE_CMD_ID_REMOVE_SCENE         = 0x02,     /*!< RemoveScene. */
    ZBPRO_ZCL_SAP_SCENE_CMD_ID_REMOVE_ALL_SCENES    = 0x03,     /*!< RemoveAllScenes. */
    ZBPRO_ZCL_SAP_SCENE_CMD_ID_STORE_SCENE          = 0x04,     /*!< StoreScene. */
    ZBPRO_ZCL_SAP_SCENE_CMD_ID_RECALL_SCENE         = 0x05,     /*!< RecallScene. */
    ZBPRO_ZCL_SAP_SCENE_CMD_ID_GET_SCENE_MEMBERSHIP = 0x06,     /*!< GetSceneMembership. */

    ZBPRO_ZCL_SAP_SCENE_CMD_ID_ENHANCED_ADD_SCENE   = 0x40,     /*!< EnhancedAddScene. */
    ZBPRO_ZCL_SAP_SCENE_CMD_ID_ENHANCED_VIEW_SCENE  = 0x41,     /*!< EnhancedViewScene. */
    ZBPRO_ZCL_SAP_SCENE_CMD_ID_COPY_SCENE           = 0x42,     /*!< CopyScene. */
} ZBPRO_ZCL_SapScenesClientToServerCommandId_t;


/**//**                                                                                                                 // TODO: Move to private header.
 * \brief   Enumeration of server-to-client commands specific to Scenes ZCL cluster.
 * \details
 *  These commands are generated by Scenes ZCL cluster Server side and received by
 *  Client side.
 * \ingroup ZBPRO_ZCL_ScenesAttr
 * \note
 *  This implementation of Scenes ZCL cluster doesn't provide generation and reception
 *  of any of the optional commands, it's able to generate and receive only the following
 *  mandatory commands:
 *  - AddSceneResponse,
 *  - ViewSceneResponse,
 *  - RemoveSceneResponse,
 *  - RemoveAllScenesResponse,
 *  - StoreSceneResponse,
 *  - GetSceneMembershipResponse.
 *
 *  Note that currently all the server-to-client commands are mandatory, there are no
 *  optional commands in recent revision of ZCL Specification for Scenes cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.7.2.5, table 3-40.
 */
typedef enum _ZBPRO_ZCL_SapScenesServerToClientCommandId_t
{
    ZBPRO_ZCL_SAP_SCENE_CMD_ID_ADD_SCENE_RESPONSE            = 0x00,     /*!< AddSceneResponse. */
    ZBPRO_ZCL_SAP_SCENE_CMD_ID_VIEW_SCENE_RESPONSE           = 0x01,     /*!< ViewSceneResponse. */
    ZBPRO_ZCL_SAP_SCENE_CMD_ID_REMOVE_SCENE_RESPONSE         = 0x02,     /*!< RemoveSceneResponse. */
    ZBPRO_ZCL_SAP_SCENE_CMD_ID_REMOVE_ALL_SCENES_RESPONSE    = 0x03,     /*!< RemoveAllScenesResponse. */
    ZBPRO_ZCL_SAP_SCENE_CMD_ID_STORE_SCENE_RESPONSE          = 0x04,     /*!< StoreSceneResponse. */
    ZBPRO_ZCL_SAP_SCENE_CMD_ID_GET_SCENE_MEMBERSHIP_RESPONSE = 0x06,     /*!< GetSceneMembershipResponse. */

    ZBPRO_ZCL_SAP_SCENE_CMD_ID_ENHANCED_ADD_SCENE_RESPONSE   = 0x40,     /*!< EnhancedAddSceneResponse. */
    ZBPRO_ZCL_SAP_SCENE_CMD_ID_ENHANCED_VIEW_SCENE_RESPONSE  = 0x41,     /*!< EnhancedViewSceneResponse. */
    ZBPRO_ZCL_SAP_SCENE_CMD_ID_COPY_SCENE_RESPONSE           = 0x42,     /*!< CopySceneResponse. */
} ZBPRO_ZCL_SapScenesServerToClientCommandId_t;

/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Scenes command AddScene
 *  specific to Scenes ZCL cluster.
 * \ingroup ZBPRO_ZCL_AddSceneReq
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.7.2.4.2.1, figure 3-18.
 */
typedef struct _ZBPRO_ZCL_ScenesCmdAddSceneReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* Custom parameters. */

    uint16_t  groupId;                                                  /*!< The Group ID. */
    uint8_t  sceneId;                                                   /*!< The Scene ID. */
    uint16_t  transitionTime;                                           /*!< The Transition Time. */
    SYS_DataPointer_t payload;                                          /*!< The Variable part of the request:
                                                                             first goes the Scene Name Length byte
                                                                             followed by the Scene Name character string
                                                                             followed by the Cluster Specific Extension
                                                                             field set(s): 2 bytes Cluster ID followed
                                                                             by 1 byte of data length followed by the data
                                                                             itself, etc.*/

} ZBPRO_ZCL_ScenesCmdAddSceneReqParams_t;

/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Scenes commands ViewScene,
 *  RemoveScene, StoreScene, RecallScene specific to Scenes ZCL cluster.
 * \ingroup ZBPRO_ZCL_GroupIdSceneIdReq
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.7.2.4.3.1, 3.7.2.4.4.1, 3.7.2.4.6.1, 3.7.2.4.7.1,
 *  figures 3-19, 3-20, 3-22, 3-23.
 */
typedef struct _ZBPRO_ZCL_ScenesCmdGroupIdSceneIdReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* Custom parameters. */

    uint16_t  groupId;                                                  /*!< The Group ID. */
    uint8_t  sceneId;                                                   /*!< The Scene ID. */

} ZBPRO_ZCL_ScenesCmdGroupIdSceneIdReqParams_t;

/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue ViewScene command.
 * \ingroup ZBPRO_ZCL_ViewSceneReq
 */
typedef  ZBPRO_ZCL_ScenesCmdGroupIdSceneIdReqParams_t  ZBPRO_ZCL_ScenesCmdViewSceneReqParams_t;

/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue RemoveScene command.
 * \ingroup ZBPRO_ZCL_RemoveSceneReq
 */
typedef  ZBPRO_ZCL_ScenesCmdGroupIdSceneIdReqParams_t  ZBPRO_ZCL_ScenesCmdRemoveSceneReqParams_t;

/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue StoreScene command.
 * \ingroup ZBPRO_ZCL_StoreSceneReq
 */
typedef  ZBPRO_ZCL_ScenesCmdGroupIdSceneIdReqParams_t  ZBPRO_ZCL_ScenesCmdStoreSceneReqParams_t;

/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue RecallScene command.
 * \ingroup ZBPRO_ZCL_RecallSceneReq
 */
typedef  ZBPRO_ZCL_ScenesCmdGroupIdSceneIdReqParams_t  ZBPRO_ZCL_ScenesCmdRecallSceneReqParams_t;

/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Scenes commands RemoveAllScenes,
 *  GetSceneMembership specific to Scenes ZCL cluster.
 * \ingroup ZBPRO_ZCL_GroupIdReq
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.7.2.4.5.1, 3.7.2.4.8.1,
 *  figures 3-21, 3-24.
 */
typedef struct _ZBPRO_ZCL_ScenesCmdGroupIdReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* Custom parameters. */

    uint16_t  groupId;                                                  /*!< The Group ID. */

} ZBPRO_ZCL_ScenesCmdGroupIdReqParams_t;

/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Scenes commands RemoveAllScenes.
 * \ingroup ZBPRO_ZCL_RemoveAllScenesReq
 */
typedef  ZBPRO_ZCL_ScenesCmdGroupIdReqParams_t  ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqParams_t;

/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Scenes commands GetSceneMembership.
 * \ingroup ZBPRO_ZCL_GetSceneMembershipReq
 */
typedef  ZBPRO_ZCL_ScenesCmdGroupIdReqParams_t  ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqParams_t;

/**//**
 * \brief   Structure for parameters of ZCL Indication when AddSceneResponse command
 *  is issued.
 * \details
 * \p zclObligatoryPart.overhallStatus contains the status of the operation.
 * \details
 *  This structure takes its origin from ZCL AddScene Response profile-wide command.
 * \ingroup ZBPRO_ZCL_AddSceneInd
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.7.2.5.1.1, figure 3-27.
 */
typedef struct _ZBPRO_ZCL_ScenesCmdAddSceneResponseIndParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* Custom parameters. */

    uint16_t  groupId;                                                  /*!< The Group ID. */
    uint8_t  sceneId;                                                   /*!< The Scene ID. */

} ZBPRO_ZCL_ScenesCmdAddSceneResponseIndParams_t;

/**//**
 * \brief   Structure for parameters of ZCL Confirmation.
 * \ingroup ZBPRO_ZCL_AddSceneConf
 */
typedef  ZBPRO_ZCL_ScenesCmdAddSceneResponseIndParams_t  ZBPRO_ZCL_ScenesCmdAddSceneConfParams_t;

/**//**
 * \brief   Structure for parameters of ZCL Indication when ViewSceneResponse command
 *  is issued.
 * \details
 * \p zclObligatoryPart.overhallStatus contains the status of the operation.
 * \details
 *  This structure takes its origin from ZCL ViewScene Response profile-wide command.
 * \ingroup ZBPRO_ZCL_ViewSceneInd
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.7.2.5.2.1, figure 3-28.
 */
typedef struct _ZBPRO_ZCL_ScenesCmdViewSceneResponseIndParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* Custom parameters. */

    uint16_t  groupId;                                                  /*!< The Group ID. */
    uint8_t  sceneId;                                                   /*!< The Scene ID. */
    uint16_t  transitionTime;                                           /*!< The Transition Time. Valid only with the status SUCCESS. */
    SYS_DataPointer_t payload;                                          /*!< The Variable part of the response:
                                                                             first goes the Scene Name Length byte
                                                                             followed by the Scene Name character string
                                                                             followed by the Cluster Specific Extension
                                                                             field set(s): 2 bytes Cluster ID followed
                                                                             by 1 byte of data length followed by the data
                                                                             itself, etc. Valid only with the status SUCCESS. */

} ZBPRO_ZCL_ScenesCmdViewSceneResponseIndParams_t;

/**//**
 * \brief   Structure for parameters of ZCL Confirmation.
 * \ingroup ZBPRO_ZCL_ViewSceneConf
 */
typedef  ZBPRO_ZCL_ScenesCmdViewSceneResponseIndParams_t  ZBPRO_ZCL_ScenesCmdViewSceneConfParams_t;

/**//**
 * \brief   Structure for parameters of ZCL Indication when RemoveSceneResponse command
 *  is issued.
 * \details
 * \p zclObligatoryPart.overhallStatus contains the status of the operation.
 * \details
 *  This structure takes its origin from ZCL RemoveScene Response profile-wide command.
 * \ingroup ZBPRO_ZCL_RemoveSceneInd
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.7.2.5.3.1, figure 3-29.
 */
typedef ZBPRO_ZCL_ScenesCmdAddSceneResponseIndParams_t ZBPRO_ZCL_ScenesCmdRemoveSceneResponseIndParams_t;

/**//**
 * \brief   Structure for parameters of ZCL Confirmation when RemoveSceneResponse command
 *  is issued.
 * \ingroup ZBPRO_ZCL_RemoveSceneConf
 */
typedef ZBPRO_ZCL_ScenesCmdAddSceneResponseIndParams_t ZBPRO_ZCL_ScenesCmdRemoveSceneConfParams_t;

/**//**
 * \brief   Structure for parameters of ZCL Indication when RemoveAllScenesResponse command
 *  is issued.
 * \details
 * \p zclObligatoryPart.overhallStatus contains the status of the operation.
 * \details
 *  This structure takes its origin from ZCL RemoveAllScenes Response profile-wide command.
 * \ingroup ZBPRO_ZCL_RemoveAllScenesInd
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.7.2.5.4.1, figure 3-30.
 */
typedef struct _ZBPRO_ZCL_ScenesCmdRemoveAllScenesResponseIndParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* Custom parameters. */

    uint16_t  groupId;                                                  /*!< The Group ID. */

} ZBPRO_ZCL_ScenesCmdRemoveAllScenesResponseIndParams_t;

/**//**
 * \brief   Structure for parameters of ZCL Confirmation when RemoveAllScenesResponse command
 *  is issued.
 * \ingroup ZBPRO_ZCL_RemoveAllScenesConf
 */
typedef  ZBPRO_ZCL_ScenesCmdRemoveAllScenesResponseIndParams_t  ZBPRO_ZCL_ScenesCmdRemoveAllScenesConfParams_t;

/**//**
 * \brief   Structure for parameters of ZCL Indication when StoreSceneResponse command
 *  is issued.
 * \details
 * \p zclObligatoryPart.overhallStatus contains the status of the operation.
 * \details
 *  This structure takes its origin from ZCL StoreScene Response profile-wide command.
 * \ingroup ZBPRO_ZCL_StoreSceneInd
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.7.2.5.5.1, figure 3-31.
 */
typedef ZBPRO_ZCL_ScenesCmdAddSceneResponseIndParams_t ZBPRO_ZCL_ScenesCmdStoreSceneResponseIndParams_t;

/**//**
 * \brief   Structure for parameters of ZCL Confirmation when StoreScenesResponse command
 *  is issued.
 * \ingroup ZBPRO_ZCL_StoreSceneConf
 */
typedef ZBPRO_ZCL_ScenesCmdAddSceneResponseIndParams_t ZBPRO_ZCL_ScenesCmdStoreSceneConfParams_t;

/**//**
 * \brief   Structure for parameters of ZCL Indication when GetSceneMembershipResponse command
 *  is issued.
 * \details
 * \p zclObligatoryPart.overhallStatus contains the status of the operation.
 * \details
 *  This structure takes its origin from ZCL GetSceneMembership Response profile-wide command.
 * \ingroup ZBPRO_ZCL_GetSceneMembershipInd
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.7.2.5.6.1, figure 3-32.
 */
typedef struct _ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseIndParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* Custom parameters. */

    uint8_t  capacity;                                                  /*!< The Capacity. */
    uint16_t  groupId;                                                  /*!< The Group ID. */
    SYS_DataPointer_t payload;                                          /*!< The Variable part of the response contains the array of the scene IDs. Valid only with the status SUCCESS. */

} ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseIndParams_t;

/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue
 *  Scenes Cluster commands.
 * \details
 *  This structure takes its origin from ZCL Default Response profile-wide command.
 * \ingroup ZBPRO_ZCL_ScenesConf
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.4.12, figure 2-25.
 */
typedef struct _ZBPRO_ZCL_ScenesCmdConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* No custom parameters exist for this type of ZCL Local Confirm. */

} ZBPRO_ZCL_ScenesCmdConfParams_t;

ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ScenesCmdAddSceneReqParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ScenesCmdViewSceneReqParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ScenesCmdStoreSceneReqParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ScenesCmdRecallSceneReqParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ScenesCmdRemoveSceneReqParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ScenesCmdAddSceneResponseIndParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ScenesCmdViewSceneResponseIndParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ScenesCmdRemoveSceneResponseIndParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ScenesCmdRemoveAllScenesResponseIndParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ScenesCmdStoreSceneResponseIndParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseIndParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ScenesCmdConfParams_t);

/**//**
 * \brief   Typedef for AddScene command.
 * \ingroup ZBPRO_ZCL_AddSceneReq
 */
typedef struct _ZBPRO_ZCL_ScenesCmdAddSceneReqDescr_t ZBPRO_ZCL_ScenesCmdAddSceneReqDescr_t;

/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of AddScene command
 *  specific to Scenes ZCL cluster.
 * \ingroup ZBPRO_ZCL_AddSceneConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_ScenesCmdAddSceneConfCallback_t(
                ZBPRO_ZCL_ScenesCmdAddSceneReqDescr_t   *const  reqDescr,
                ZBPRO_ZCL_ScenesCmdAddSceneConfParams_t *const  confParams);

/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue AddScene command
 *  specific to Scenes ZCL cluster.
 * \ingroup ZBPRO_ZCL_AddSceneReq
 */
struct _ZBPRO_ZCL_ScenesCmdAddSceneReqDescr_t
{
    ZBPRO_ZCL_ScenesCmdAddSceneConfCallback_t    *callback;      /*!< ZCL Confirmation callback handler entry point. */
    ZbProZclLocalPrimitiveDescrService_t          service;       /*!< ZCL Request Descriptor service field. */
    ZBPRO_ZCL_ScenesCmdAddSceneReqParams_t        params;        /*!< ZCL Request parameters structure. */
};

/**//**
 * \brief   Typedef for ViewScene command.
 * \ingroup ZBPRO_ZCL_ViewSceneReq
 */
typedef struct _ZBPRO_ZCL_ScenesCmdViewSceneReqDescr_t ZBPRO_ZCL_ScenesCmdViewSceneReqDescr_t;

/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of ViewScene command
 *  specific to Scenes ZCL cluster.
 * \ingroup ZBPRO_ZCL_ViewSceneConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_ScenesCmdViewSceneConfCallback_t(
                ZBPRO_ZCL_ScenesCmdViewSceneReqDescr_t   *const  reqDescr,
                ZBPRO_ZCL_ScenesCmdViewSceneConfParams_t *const  confParams);

/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue ViewScene command
 *  specific to Scenes ZCL cluster.
 * \ingroup ZBPRO_ZCL_ViewSceneReq
 */
struct _ZBPRO_ZCL_ScenesCmdViewSceneReqDescr_t
{
    ZBPRO_ZCL_ScenesCmdViewSceneConfCallback_t   *callback;      /*!< ZCL Confirmation callback handler entry point. */
    ZbProZclLocalPrimitiveDescrService_t          service;       /*!< ZCL Request Descriptor service field. */
    ZBPRO_ZCL_ScenesCmdViewSceneReqParams_t       params;        /*!< ZCL Request parameters structure. */
};

/**//**
 * \brief   Typedef for StoreScene command.
 * \ingroup ZBPRO_ZCL_StoreSceneReq
 */
typedef struct _ZBPRO_ZCL_ScenesCmdStoreSceneReqDescr_t ZBPRO_ZCL_ScenesCmdStoreSceneReqDescr_t;

/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of StoreScene command
 *  specific to Scenes ZCL cluster.
 * \ingroup ZBPRO_ZCL_StoreSceneConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_ScenesCmdStoreSceneConfCallback_t(
                ZBPRO_ZCL_ScenesCmdStoreSceneReqDescr_t   *const  reqDescr,
                ZBPRO_ZCL_ScenesCmdStoreSceneConfParams_t *const  confParams);

/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue StoreScene command
 *  specific to Scenes ZCL cluster.
 * \ingroup ZBPRO_ZCL_StoreSceneReq
 */
struct _ZBPRO_ZCL_ScenesCmdStoreSceneReqDescr_t
{
    ZBPRO_ZCL_ScenesCmdStoreSceneConfCallback_t  *callback;      /*!< ZCL Confirmation callback handler entry point. */
    ZbProZclLocalPrimitiveDescrService_t          service;       /*!< ZCL Request Descriptor service field. */
    ZBPRO_ZCL_ScenesCmdStoreSceneReqParams_t      params;        /*!< ZCL Request parameters structure. */
};

/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of RemoveScene command
 *  specific to Scenes ZCL cluster.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef struct _ZBPRO_ZCL_ScenesCmdRemoveSceneReqDescr_t ZBPRO_ZCL_ScenesCmdRemoveSceneReqDescr_t;
typedef void ZBPRO_ZCL_ScenesCmdRemoveSceneConfCallback_t(
                ZBPRO_ZCL_ScenesCmdRemoveSceneReqDescr_t   *const  reqDescr,
                ZBPRO_ZCL_ScenesCmdRemoveSceneConfParams_t *const  confParams);

/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue RemoveScene command
 *  specific to Scenes ZCL cluster.
 */
struct _ZBPRO_ZCL_ScenesCmdRemoveSceneReqDescr_t
{
    ZBPRO_ZCL_ScenesCmdRemoveSceneConfCallback_t    *callback;      /*!< ZCL Confirmation callback handler entry point. */
    ZbProZclLocalPrimitiveDescrService_t             service;       /*!< ZCL Request Descriptor service field. */
    ZBPRO_ZCL_ScenesCmdRemoveSceneReqParams_t        params;        /*!< ZCL Request parameters structure. */
};

/**//**
 * \brief   Typedef for RemoveAllScenes command.
 * \ingroup ZBPRO_ZCL_RemoveAllScenesReq
 */
typedef struct _ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqDescr_t ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqDescr_t;

/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of RemoveAllScenes command
 *  specific to Scenes ZCL cluster.
 * \ingroup ZBPRO_ZCL_RemoveAllScenesConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_ScenesCmdRemoveAllScenesConfCallback_t(
                ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqDescr_t   *const  reqDescr,
                ZBPRO_ZCL_ScenesCmdRemoveAllScenesConfParams_t *const  confParams);

/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue RemoveAllScenes command
 *  specific to Scenes ZCL cluster.
 * \ingroup ZBPRO_ZCL_RemoveAllScenesReq
 */
struct _ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqDescr_t
{
    ZBPRO_ZCL_ScenesCmdRemoveAllScenesConfCallback_t    *callback;      /*!< ZCL Confirmation callback handler entry point. */
    ZbProZclLocalPrimitiveDescrService_t                 service;       /*!< ZCL Request Descriptor service field. */
    ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqParams_t        params;        /*!< ZCL Request parameters structure. */
};

/**//**
 * \brief   Typedef for RecallScene command.
 * \ingroup ZBPRO_ZCL_RecallSceneReq
 */
typedef struct _ZBPRO_ZCL_ScenesCmdRecallSceneReqDescr_t ZBPRO_ZCL_ScenesCmdRecallSceneReqDescr_t;

/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of RecallScene command
 *  specific to Scenes ZCL cluster.
 * \ingroup ZBPRO_ZCL_RecallSceneConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_ScenesCmdRecallSceneConfCallback_t(
                ZBPRO_ZCL_ScenesCmdRecallSceneReqDescr_t *const  reqDescr,
                ZBPRO_ZCL_ScenesCmdConfParams_t          *const  confParams);

/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue RecallScene command
 *  specific to Scenes ZCL cluster.
 * \ingroup ZBPRO_ZCL_RecallSceneReq
 */
struct _ZBPRO_ZCL_ScenesCmdRecallSceneReqDescr_t
{
    ZBPRO_ZCL_ScenesCmdRecallSceneConfCallback_t    *callback;      /*!< ZCL Confirmation callback handler entry point. */
    ZbProZclLocalPrimitiveDescrService_t             service;       /*!< ZCL Request Descriptor service field. */
    ZBPRO_ZCL_ScenesCmdRecallSceneReqParams_t        params;        /*!< ZCL Request parameters structure. */
};

/**//**
 * \brief   Typedef for GetSceneMembership command.
 * \ingroup ZBPRO_ZCL_GetSceneMembershipReq
 */
typedef struct _ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqDescr_t ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqDescr_t;

/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of GetSceneMembership command
 *  specific to Scenes ZCL cluster.
 * \ingroup ZBPRO_ZCL_GetSceneMembershipConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_ScenesCmdGetSceneMembershipConfCallback_t(
                ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqDescr_t *const  reqDescr,
                ZBPRO_ZCL_ScenesCmdConfParams_t                 *const  confParams);

/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue GetSceneMembership command
 *  specific to Scenes ZCL cluster.
 * \ingroup ZBPRO_ZCL_GetSceneMembershipReq
 */
struct _ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqDescr_t
{
    ZBPRO_ZCL_ScenesCmdGetSceneMembershipConfCallback_t    *callback;      /*!< ZCL Confirmation callback handler entry point. */
    ZbProZclLocalPrimitiveDescrService_t                    service;       /*!< ZCL Request Descriptor service field. */
    ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqParams_t        params;        /*!< ZCL Request parameters structure. */
};

/*
 * Validate structures of ZCL Local Request Descriptors.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_ScenesCmdAddSceneReqDescr_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_ScenesCmdViewSceneReqDescr_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_ScenesCmdRemoveSceneReqDescr_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqDescr_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_ScenesCmdStoreSceneReqDescr_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_ScenesCmdRecallSceneReqDescr_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqDescr_t);


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief  Functions accept ZCL Local Requests to issue AddScene, ViewScene, StoreScene,
 *  RemoveScene, RemoveAllScenes, RecalleScene and GetSceneMembership commands specific
 *  to Scenes ZCL cluster.
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
 */

/**//**
 * \brief   Accepts ZCL Local Request to issue AddScene command specific to Scenes ZCL
 *  cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_ScenesCmdAddSceneReq(
                ZBPRO_ZCL_ScenesCmdAddSceneReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue ViewScene command specific to Scenes ZCL
 *  cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_ScenesCmdViewSceneReq(
                ZBPRO_ZCL_ScenesCmdViewSceneReqDescr_t *const  reqDescr);

/**//**
 * \brief   Accepts ZCL Local Request to issue StoreScene command specific to Scenes ZCL
 *  cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_ScenesCmdStoreSceneReq(
                ZBPRO_ZCL_ScenesCmdStoreSceneReqDescr_t *const  reqDescr);

/**//**
 * \brief   Accepts ZCL Local Request to issue RecallScene command specific to Scenes ZCL
 *  cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_ScenesCmdRecallSceneReq(
                ZBPRO_ZCL_ScenesCmdRecallSceneReqDescr_t *const  reqDescr);

/**//**
 * \brief   Accepts ZCL Local Request to issue RemoveScene command specific to Scenes ZCL
 *  cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_ScenesCmdRemoveSceneReq(
                ZBPRO_ZCL_ScenesCmdRemoveSceneReqDescr_t *const  reqDescr);

/**//**
 * \brief   Accepts ZCL Local Request to issue RemoveAllScenes command specific to Scenes ZCL
 *  cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_ScenesCmdRemoveAllScenesReq(
                ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqDescr_t *const  reqDescr);

/**//**
 * \brief   Accepts ZCL Local Request to issue GetSceneMembership command specific to Scenes ZCL
 *  cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_ScenesCmdGetSceneMembershipReq(
                ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqDescr_t *const  reqDescr);

/**//**
 * \brief   Handles ZCL Local Scenes on reception of GetSceneMembershipResponse command specific to
 *  Identify ZCL cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \return Nothing.
 */
void ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseInd(
                ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseIndParams_t *const  indParams);

#endif // BBZBPROZCLSAPCLUSTERSCENES_H

/* eof bbZbProZclSapClusterScenes.h */