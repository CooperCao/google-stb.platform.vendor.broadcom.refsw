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
* FILENAME: $Workfile: trunk/stack/ZbPro/ZCL/include/private/bbZbProZclHandlerClusterGroups.h $
*
* DESCRIPTION:
*   ZCL Groups cluster handler private interface.
*
* $Revision: 7831 $
* $Date: 2015-07-31 13:31:38Z $
*
*****************************************************************************************/
#ifndef _BB_ZBPRO_ZCL_HANDLER_CLUSTER_GROUPS_H_
#define _BB_ZBPRO_ZCL_HANDLER_CLUSTER_GROUPS_H_

/************************* INCLUDES *****************************************************/
#include "bbZbProZclSapClusterGroups.h"
#include "bbZbProZclCommon.h"
#include "private/bbZbProZclDispatcher.h"

/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of client-to-server commands specific to Groups ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.6.2.3, table 3-34.
 */
typedef enum _ZBPRO_ZCL_SapGroupsClientToServerCommandId_t
{
    ZBPRO_ZCL_SAP_GROUPS_CMD_ID_ADD_GROUP                   =  0x00,     /*!< Add Group. */

    ZBPRO_ZCL_SAP_GROUPS_CMD_ID_VIEW_GROUP                  =  0x01,     /*!< View group. */

    ZBPRO_ZCL_SAP_GROUPS_CMD_ID_GET_GROUP_MEMBERSHIP        =  0x02,     /*!< Get group membership. */

    ZBPRO_ZCL_SAP_GROUPS_CMD_ID_REMOVE_GROUP                =  0x03,     /*!< Remove group. */

    ZBPRO_ZCL_SAP_GROUPS_CMD_ID_REMOVE_ALL_GROUPS           =  0x04,     /*!< Remove all groups. */

    ZBPRO_ZCL_SAP_GROUPS_CMD_ID_ADD_GROUP_IF_IDENTIFYING    =  0x05      /*!< Add Group if identifying. */

} ZBPRO_ZCL_SapGroupsClientToServerCommandId_t;


/**//**
 * \brief   Enumeration of server-to-client commands specific to Groups ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.6.2.4, table 3-35.
 */
typedef enum _ZBPRO_ZCL_SapGroupsServerToClientCommandId_t
{
    ZBPRO_ZCL_SAP_GROUPS_CMD_ID_ADD_GROUP_RESPONSE             =  0x00,     /*!< Add Group response. */

    ZBPRO_ZCL_SAP_GROUPS_CMD_ID_VIEW_GROUP_RESPONSE            =  0x01,     /*!< View group response. */

    ZBPRO_ZCL_SAP_GROUPS_CMD_ID_GET_GROUP_MEMBERSHIP_RESPONSE  =  0x02,     /*!< Get group membership response. */

    ZBPRO_ZCL_SAP_GROUPS_CMD_ID_REMOVE_GROUP_RESPONSE          =  0x03,     /*!< Remove group response. */

} ZBPRO_ZCL_SapGroupsServerToClientCommandId_t;


/**//**
 * \brief   Assembles Groups cluster Command Uid.
 * \param[in]   direction       Either Client-to-Server (0x0) or Server-to-Client (0x1).
 * \param[in]   commandId       Numeric 8-bit value of Command Identifier local to Cluster
 *  and its Side.
 * \return  Numeric 64-bit of Command Uid.
 * \details
 *  Parameter \p direction specifies which side of cluster generates the command.
 */
#define ZBPRO_ZCL_MAKE_GROUPS_CLUSTER_COMMAND_UID(direction, commandId)\
        ZBPRO_ZCL_MAKE_CLUSTER_SPECIFIC_COMMAND_UID(\
                ZBPRO_ZCL_MAKE_COMMAND_UID_PREFIX(\
                        ZBPRO_ZCL_FRAME_TYPE_CLUSTER_SPECIFIC_COMMAND, ZBPRO_ZCL_CLUSTER_ID_GROUPS,\
                        ZBPRO_ZCL_FRAME_DOMAIN_ZCL_STANDARD, /*manufCode*/ 0),\
                direction, commandId)


/**//**
 * \brief   Unique identifiers of Groups ZCL cluster commands.
 */
/**@{*/
#define ZBPRO_ZCL_GROUPS_CMD_UID_ADD_GROUP\
        ZBPRO_ZCL_MAKE_GROUPS_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER,\
                ZBPRO_ZCL_SAP_GROUPS_CMD_ID_ADD_GROUP)                      /*!< Add Group command Uid. */

#define ZBPRO_ZCL_GROUPS_CMD_UID_VIEW_GROUP\
        ZBPRO_ZCL_MAKE_GROUPS_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER,\
                ZBPRO_ZCL_SAP_GROUPS_CMD_ID_VIEW_GROUP)                     /*!< View Group command Uid. */

#define ZBPRO_ZCL_GROUPS_CMD_UID_GET_GROUP_MEMBERSHIP\
        ZBPRO_ZCL_MAKE_GROUPS_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER,\
                 ZBPRO_ZCL_SAP_GROUPS_CMD_ID_GET_GROUP_MEMBERSHIP)          /*!< Get Group Membership Command Uid. */

#define ZBPRO_ZCL_GROUPS_CMD_UID_REMOVE_GROUP\
        ZBPRO_ZCL_MAKE_GROUPS_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER,\
                ZBPRO_ZCL_SAP_GROUPS_CMD_ID_REMOVE_GROUP)                   /*!< Remove Group Command Uid. */

#define ZBPRO_ZCL_GROUPS_CMD_UID_REMOVE_ALL_GROUPS\
        ZBPRO_ZCL_MAKE_GROUPS_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER,\
                ZBPRO_ZCL_SAP_GROUPS_CMD_ID_REMOVE_ALL_GROUPS)              /*!< Remove  All Groups Command Uid. */

#define ZBPRO_ZCL_GROUPS_CMD_UID_ADD_GROUP_IF_IDENTIFYING\
        ZBPRO_ZCL_MAKE_GROUPS_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER,\
                ZBPRO_ZCL_SAP_GROUPS_CMD_ID_ADD_GROUP_IF_IDENTIFYING)       /*!< Add Group if identifying Command Uid. */


#define ZBPRO_ZCL_GROUPS_CMD_UID_ADD_GROUP_RESPONSE\
        ZBPRO_ZCL_MAKE_GROUPS_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT,\
                ZBPRO_ZCL_SAP_GROUPS_CMD_ID_ADD_GROUP_RESPONSE)             /*!< Add Group response Uid. */

#define ZBPRO_ZCL_GROUPS_CMD_UID_VIEW_GROUP_RESPONSE\
        ZBPRO_ZCL_MAKE_GROUPS_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT,\
                ZBPRO_ZCL_SAP_GROUPS_CMD_ID_VIEW_GROUP_RESPONSE)            /*!< View Group response Uid. */

#define ZBPRO_ZCL_GROUPS_CMD_UID_GET_GROUP_MEMBERSHIP_RESPONSE\
        ZBPRO_ZCL_MAKE_GROUPS_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT,\
                ZBPRO_ZCL_SAP_GROUPS_CMD_ID_GET_GROUP_MEMBERSHIP_RESPONSE)  /*!< Get Group Membership response Uid. */

#define ZBPRO_ZCL_GROUPS_CMD_UID_REMOVE_GROUP_RESPONSE\
        ZBPRO_ZCL_MAKE_GROUPS_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT,\
                ZBPRO_ZCL_SAP_GROUPS_CMD_ID_REMOVE_GROUP_RESPONSE)          /*!< Remove group response Uid. */
/**@}*/


/**//**
 * \brief List of Indication parameters for bbZbProZclDispatcherTables.h
 */
#define ZBPRO_ZCL_GROUPS_INDICATION_PARAMETERS_LIST      \
        ZBPRO_ZCL_GroupsCmdGetGroupMembershipIndParams_t    ZBPRO_ZCL_GroupsCmdGetGroupMembershipIndParams

/**//**
 * \brief List of Confirmation parameters for bbZbProZclDispatcherTables.h
 */
#define ZBPRO_ZCL_GROUPS_CONF_PARAMETERS_LIST \
        ZBPRO_ZCL_GroupsCmdAddGroupConfParams_t              ZBPRO_ZCL_GroupsCmdAddGroupConfParams;            \
        ZBPRO_ZCL_GroupsCmdViewGroupConfParams_t             ZBPRO_ZCL_GroupsCmdViewGroupConfParams;           \
        ZBPRO_ZCL_GroupsCmdGetGroupMembershipConfParams_t    ZBPRO_ZCL_GroupsCmdGetGroupMembershipConfParams;  \
        ZBPRO_ZCL_GroupsCmdRemoveGroupConfParams_t           ZBPRO_ZCL_GroupsCmdRemoveGroupConfParams;         \
        ZBPRO_ZCL_GroupsCmdRemoveAllGroupsConfParams_t       ZBPRO_ZCL_GroupsCmdRemoveAllGroupsConfParams;     \
        ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyConfParams_t    ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyConfParams


/************************* PROTOTYPES ***************************************************/

/*-------------------------------- Add Group Cmd ------------------------------------------------------*/
/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of Add Group command
 *  specific to Groups ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully;
 *          FALSE if it has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.6.2.3.2, figure 3-9.
 */
bool zbProZclHandlerClusterGroupsComposeAddGroup(
            SYS_DataPointer_t                               *const  zclFramePayload,
            const ZbProZclLocalPrimitiveParamsPrototype_t   *const  reqParams);


/**//**
 * \brief   Parses ZCL Frame Payload of Add Group response command
 *  specific to Groups ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload.
 * \param[out]      indParams           Pointer to ZCL Local Indication Parameters
 *  Prototype.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.6.2.4.1, figure 3-14.
 */
void zbProZclHandlerClusterGroupsParseAddGroupResponse(
            SYS_DataPointer_t                             *const  zclFramePayload,
            ZbProZclLocalPrimitiveParamsPrototype_t       *const  indParams,
            const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);


/*-------------------------------- View Group Cmd ------------------------------------------------------*/
/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of View Group command
 *  specific to Groups ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully;
 *          FALSE if it has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.6.2.3.3, figure 3-10.
 */
bool zbProZclHandlerClusterGroupsComposeViewGroup(
            SYS_DataPointer_t                               *const  zclFramePayload,
            const ZbProZclLocalPrimitiveParamsPrototype_t   *const  reqParams);


/**//**
 * \brief   Parses ZCL Frame Payload of View Group response command
 *  specific to Groups ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload.
 * \param[out]      indParams           Pointer to ZCL Local Indication Parameters
 *  Prototype.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.6.2.4.2, figure 3-15.
 */
void zbProZclHandlerClusterGroupsParseViewGroupResponse(
            SYS_DataPointer_t                             *const  zclFramePayload,
            ZbProZclLocalPrimitiveParamsPrototype_t       *const  indParams,
            const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);


/*-------------------------------- Get Group Membership Cmd ------------------------------------------------------*/
/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of Get Group Membership command
 *  specific to Groups ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully;
 *          FALSE if it has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.6.2.3.4, figure 3-11.
 */
bool zbProZclHandlerClusterGroupsComposeGetGroupMembership(
            SYS_DataPointer_t                               *const  zclFramePayload,
            const ZbProZclLocalPrimitiveParamsPrototype_t   *const  reqParams);


/**//**
 * \brief   Parses ZCL Frame Payload of Get Group Membership response command
 *  specific to Groups ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload.
 * \param[out]      indParams           Pointer to ZCL Local Indication Parameters
 *  Prototype.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.6.2.4.3 figure 3-16.
 */
void zbProZclHandlerClusterGroupsParseGetGroupMembershipResponse(
            SYS_DataPointer_t                           *const  zclFramePayload,
            ZbProZclLocalPrimitiveParamsPrototype_t     *const  indParams);


/*-------------------------------- Remove group Cmd ------------------------------------------------------*/
/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of Remove group command
 *  specific to Groups ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully;
 *          FALSE if it has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.6.2.3.5, figure 3-12.
 */
bool zbProZclHandlerClusterGroupsComposeRemoveGroup(
            SYS_DataPointer_t                               *const  zclFramePayload,
            const ZbProZclLocalPrimitiveParamsPrototype_t   *const  reqParams);


/**//**
 * \brief   Parses ZCL Frame Payload of Remove group response command
 *  specific to Groups ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload.
 * \param[out]      indParams           Pointer to ZCL Local Indication Parameters
 *  Prototype.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.6.2.4.4 figure 3-17.
 */
void zbProZclHandlerClusterGroupsParseRemoveGroupResponse(
            SYS_DataPointer_t                             *const  zclFramePayload,
            ZbProZclLocalPrimitiveParamsPrototype_t       *const  indParams,
            const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);


/*-------------------------------- Remove all groups Cmd ------------------------------------------------------*/

/* The Remove all groups command has no payload in the request and has no respond.
 * So there nothing to implement here. */

/*-------------------------------- Add Group If Identifying Cmd ---------------------------------------------------*/
/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of Add Group If Identifying command
 *  specific to Groups ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully;
 *          FALSE if it has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.6.2.3.7, figure 3-13.
 */
bool zbProZclHandlerClusterGroupsComposeAddGroupIfIdentifying(
            SYS_DataPointer_t                               *const  zclFramePayload,
            const ZbProZclLocalPrimitiveParamsPrototype_t   *const  reqParams);


/* The Add Group If Identifying command has no respond. */



#endif
