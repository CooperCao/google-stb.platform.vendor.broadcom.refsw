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
* FILENAME: $Workfile: trunk/stack/ZbPro/ZCL/include/bbZbProZclSapClusterGroups.h $
*
* DESCRIPTION:
*   ZCL Groups cluster SAP interface.
*
* $Revision: 7600 $
* $Date: 2015-07-21 08:57:33Z $
*
*****************************************************************************************/

#ifndef _BB_ZBPRO_ZCL_SAP_CLUSTER_GROUPS_H_
#define _BB_ZBPRO_ZCL_SAP_CLUSTER_GROUPS_H_

/************************* INCLUDES *****************************************************/
#include "bbZbProZclCommon.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of attributes of Groups ZCL cluster Server side.
 * \details
 *  These attributes are provided by Server side of the cluster.
 * \details
 *  Groups ZCL cluster has no attributes provided by Client side.
 * \note
 *  This implementation of Groups ZCL cluster doesn't provide Server side. *
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.6.2.2, Table 3-33.
 */
typedef enum _ZBPRO_ZCL_SapGroupsServerAttributeId_t
{
    ZBPRO_ZCL_SAP_GROUPS_ATTR_ID_NAME_SUPPORT            = 0x0000,       /*!< NameSupport. */

    ZBPRO_ZCL_SAP_GROUPS_ATTR_ID_MAX                     = 0xFFFF,       /*!< Introduced only to make the
                                                                             enumeration 16-bit wide. */
} ZBPRO_ZCL_SapGroupsServerAttributeId_t;


/**//**
 * \name    Data types shared by attributes and command parameters of Groups cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.6.2.2, Table 3-33.
 */
typedef   BitField8_t   ZBPRO_ZCL_SapGroupsParamNameSupport_t;      /*!< Shared data type for NameSupport parameter. */

typedef   uint16_t      ZBPRO_ZCL_SapGroupsGroupID;                 /*!< Shared data type for GroupName parameter. */


/*-------------------------------- Add Group Cmd ------------------------------------------------------*/
/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Add Group command
 * specific to Groups ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.6.2.3.2, figure 3-9.
 */
typedef struct _ZBPRO_ZCL_GroupsCmdAddGroupReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;     /*!< Set of obligatory parameters of ZCL public
                                                                          interface to local application. */
    /* Custom parameters. */

    /* 32-bit data. */

    SYS_DataPointer_t                         payload;               /*!< 0-16 characters of the Group Name. This
                                                                          field contains characters only - without
                                                                          length byte. */
    /* 16-bit data. */

    ZBPRO_ZCL_SapGroupsGroupID                groupID;               /*!< GroupID to add to the Group Table. */

} ZBPRO_ZCL_GroupsCmdAddGroupReqParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_GroupsCmdAddGroupReqParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Add Group command
 * specific to Groups ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.6.2.4.1, figure 3-14.
 */
typedef struct _ZBPRO_ZCL_GroupsCmdAddGroupConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* Custom parameters. */

    /* 16-bit data. */
    ZBPRO_ZCL_SapGroupsGroupID              groupID;                    /*!< The Group ID field is set to the Group ID
                                                                             field of the received Add Group command */
    /* 8-bit data. */
    ZBPRO_ZCL_Status_t                      status;                     /*!< The Status field is set to SUCCESS,
                                                                             DUPLICATE_EXISTS, or INSUFFICIENT_SPACE
                                                                             as appropriate. */
} ZBPRO_ZCL_GroupsCmdAddGroupConfParams_t;


/*
 * Validate structure of ZCL Local Confirmation Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_GroupsCmdAddGroupConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Add Group command
 *  specific to Groups ZCL cluster
 */
typedef struct     _ZBPRO_ZCL_GroupsCmdAddGroupReqDescr_t     ZBPRO_ZCL_GroupsCmdAddGroupReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Add Group command
 *  specific to Groups ZCL cluster
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_GroupsCmdAddGroupConfCallback_t(
    ZBPRO_ZCL_GroupsCmdAddGroupReqDescr_t     *const  reqDescr,
    ZBPRO_ZCL_GroupsCmdAddGroupConfParams_t   *const  confParams);

/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Add Group command
 *  specific to Groups ZCL cluster
 */
struct _ZBPRO_ZCL_GroupsCmdAddGroupReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_GroupsCmdAddGroupConfCallback_t     *callback;       /*!< ZCL Confirmation callback handler entry
                                                                            point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t           service;        /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_GroupsCmdAddGroupReqParams_t         params;         /*!< ZCL Request parameters structure. */
};

/*-------------------------------- View Group Cmd ------------------------------------------------------*/
/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue View Group command
 * specific to Groups ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.6.2.3.3, figure 3-10.
 */
typedef struct _ZBPRO_ZCL_GroupsCmdViewGroupReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                           interface to local application. */
    /* Custom parameters. */

    /* 16-bit data. */
    ZBPRO_ZCL_SapGroupsGroupID                groupID;                /*!< GroupID to view. */

} ZBPRO_ZCL_GroupsCmdViewGroupReqParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_GroupsCmdViewGroupReqParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue View Group
 * command specific to Groups ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.6.2.4.1, figure 3-14.
 */
typedef struct _ZBPRO_ZCL_GroupsCmdViewGroupConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                           interface to local application. */
    /* Custom parameters. */

    /* 32-bit data. */
    SYS_DataPointer_t                         payload;                /*!< 0-16 characters of the Group Name. This
                                                                           field contains characters only - without
                                                                           length byte. */
    /* 16-bit data. */
    ZBPRO_ZCL_SapGroupsGroupID                groupID;                /*!< The Group ID field is set to the Group ID
                                                                           field of the received Add Group command. */
    /* 8-bit data. */
    ZBPRO_ZCL_Status_t                        status;                 /*!< The Status field is set to SUCCESS,
                                                                           DUPLICATE_EXISTS, or INSUFFICIENT_SPACE
                                                                           as appropriate. */
} ZBPRO_ZCL_GroupsCmdViewGroupConfParams_t;


/*
 * Validate structure of ZCL Local Confirmation Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_GroupsCmdViewGroupConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue View Group command
 * specific to Groups ZCL cluster.
 */
typedef struct     _ZBPRO_ZCL_GroupsCmdViewGroupReqDescr_t     ZBPRO_ZCL_GroupsCmdViewGroupReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of View Group command
 *  specific to Groups ZCL cluster
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_GroupsCmdViewGroupConfCallback_t(
    ZBPRO_ZCL_GroupsCmdViewGroupReqDescr_t     *const  reqDescr,
    ZBPRO_ZCL_GroupsCmdViewGroupConfParams_t   *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue View Group command
 *  specific to Groups ZCL cluster
 */
struct _ZBPRO_ZCL_GroupsCmdViewGroupReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_GroupsCmdViewGroupConfCallback_t     *callback;       /*!< ZCL Confirmation callback handler entry
                                                                         point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t            service;        /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_GroupsCmdViewGroupReqParams_t         params;         /*!< ZCL Request parameters structure. */
};



/*-------------------------------- Get Group Membership Cmd ------------------------------------------------------*/
/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Get Group Membership command
 * specific to Groups ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.6.2.3.4, figure 3-11.
 */
typedef struct _ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                           interface to local application. */
    /* Custom parameters. */

    /* 32-bit data. */
    SYS_DataPointer_t                         payload;                /*!< List of group ID. Number of groups contained
                                                                           in this list can be calculated as:
                                                    SYS_GetPayloadSize(payload) / sizeof(ZBPRO_ZCL_SapGroupsGroupID)*/

} ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqParams_t);

/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Get Group Membership command
 * specific to Groups ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.6.2.4.3, figure 3-16.
 */
typedef struct _ZBPRO_ZCL_GroupsCmdGetGroupMembershipIndParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;   /*!< Set of obligatory parameters of ZCL public
                                                                        interface to local application. */
    /* Custom parameters. */

    /* 32-bit data. */
    SYS_DataPointer_t                         payload;             /*!< The Group ID list field shall contain the
                                                                        identifiers either of all the groups in the
                                                                        group table or all the groups from the
                                                                        group list field of the received get group
                                                                        membership command which are in the
                                                                        group table. Number of groups contained
                                                                        in this list can be calculated as:
                                                    SYS_GetPayloadSize(payload) / sizeof(ZBPRO_ZCL_SapGroupsGroupID) */
    /* 8-bit data */
    uint8_t                                   capacity;            /*!< The Capacity field shall contain the
                                                                        remaining capacity of the group table
                                                                        of the device. */

} ZBPRO_ZCL_GroupsCmdGetGroupMembershipIndParams_t;


/*
 * Validate structure of ZCL Local Confirmation Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_GroupsCmdGetGroupMembershipIndParams_t);


/**//**
 * \brief  Data type for ZCL Local Confirmation parameters if Get Group Membership command
 *  specific to Groups ZCL cluster.
 *  \notr Confirmation parameters has no custom fields - only obligatory.
 */
typedef struct _ZBPRO_ZCL_GroupsCmdGetGroupMembershipConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;   /*!< Set of obligatory parameters of ZCL public
                                                                        interface to local application. */
    /* There no custom fields.*/

} ZBPRO_ZCL_GroupsCmdGetGroupMembershipConfParams_t;

/*
 * Validate structure of ZCL Local Confirmation Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_GroupsCmdGetGroupMembershipConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Get Group Membership command
 * specific to Groups ZCL cluster.
 */
typedef struct   _ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqDescr_t   ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Get Group Membership command
 *  specific to Groups ZCL cluster
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the empty confirmation parameters because responses will be
 oroceed in the Indication, not in the callback.
 */
typedef void ZBPRO_ZCL_GroupsCmdGetGroupMembershipConfCallback_t(
	ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqDescr_t        *const  reqDescr,
    ZBPRO_ZCL_GroupsCmdGetGroupMembershipConfParams_t      *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Get Group Membership command
 *  specific to Groups ZCL cluster
 */
struct _ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_GroupsCmdGetGroupMembershipConfCallback_t   *callback;   /*!< ZCL Confirmation callback handler entry
                                                                            point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                   service;    /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqParams_t       params;     /*!< ZCL Request parameters structure. */
};

/*-------------------------------- Remove group Cmd ------------------------------------------------------*/
/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Remove group command
 * specific to Groups ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.6.2.3.5, figure 3-12.
 */
typedef struct _ZBPRO_ZCL_GroupsCmdRemoveGroupReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                           interface to local application. */
    /* Custom parameters. */
    /* 16-bit data. */
    ZBPRO_ZCL_SapGroupsGroupID                groupID;                /*!< GroupID to view. */

} ZBPRO_ZCL_GroupsCmdRemoveGroupReqParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_GroupsCmdRemoveGroupReqParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Remove Group
 * command specific to Groups ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.6.2.4.4, figure 3-17.
 */
typedef struct _ZBPRO_ZCL_GroupsCmdRemoveGroupConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                           interface to local application. */
    /* Custom parameters. */

    /* 16-bit data. */
    ZBPRO_ZCL_SapGroupsGroupID                groupID;                /*!< The Group ID field is set to the Group ID
                                                                       field of the received Add Group command. */
    /* 8-bit data. */
    ZBPRO_ZCL_Status_t                        status;                 /*!< The Status field is set to SUCCESS,
                                                                           DUPLICATE_EXISTS, or INSUFFICIENT_SPACE
                                                                           as appropriate. */
} ZBPRO_ZCL_GroupsCmdRemoveGroupConfParams_t;


/*
 * Validate structure of ZCL Local Confirmation Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_GroupsCmdRemoveGroupConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Remove Group command
 * specific to Groups ZCL cluster.
 */
typedef struct   _ZBPRO_ZCL_GroupsCmdRemoveGroupReqDescr_t     ZBPRO_ZCL_GroupsCmdRemoveGroupReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Remove Group command
 *  specific to Groups ZCL cluster
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_GroupsCmdRemoveGroupConfCallback_t(
    ZBPRO_ZCL_GroupsCmdRemoveGroupReqDescr_t     *const  reqDescr,
    ZBPRO_ZCL_GroupsCmdRemoveGroupConfParams_t   *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Remove Group command
 *  specific to Groups ZCL cluster
 */
struct _ZBPRO_ZCL_GroupsCmdRemoveGroupReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_GroupsCmdRemoveGroupConfCallback_t   *callback;   /*!< ZCL Confirmation callback handler entry
                                                                            point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t            service;    /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_GroupsCmdRemoveGroupReqParams_t       params;     /*!< ZCL Request parameters structure. */
};


/*-------------------------------- Remove all groups Cmd ------------------------------------------------------*/
/**//**
 * \brief  Remove All Groups request has no custom fields.
 */
typedef struct _ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;   /*!< Set of obligatory parameters of ZCL public
                                                                        interface to local application. */
    /* There no custom fields.*/

} ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqParams_t;


/*
 * Validate structure of ZCL Local Confirmation Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Remove all groups command
 * specific to Groups ZCL cluster.
 */
typedef struct     _ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqDescr_t     ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqDescr_t;


/**//**
 * \brief  Remove All Groups Confirmation has no custom fields.
 */
typedef struct _ZBPRO_ZCL_GroupsCmdRemoveAllGroupsConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;   /*!< Set of obligatory parameters of ZCL public
                                                                        interface to local application. */
    /* There no custom fields.*/

} ZBPRO_ZCL_GroupsCmdRemoveAllGroupsConfParams_t;


/*
 * Validate structure of ZCL Local Confirmation Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_GroupsCmdRemoveAllGroupsConfParams_t);


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Remove All Groups command
 *  specific to Groups ZCL cluster
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 * \note
 * Remove All Groups Confirmation has no custom fields.
 */
typedef void ZBPRO_ZCL_GroupsCmdRemoveAllGroupsConfCallback_t(
        ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqDescr_t     *const  reqDescr,
        ZBPRO_ZCL_GroupsCmdRemoveAllGroupsConfParams_t   *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Remove All Groups command
 *  specific to Groups ZCL cluster
 * \note
 * Remove All Groups Confirmation has no custom fields.
 */
struct _ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_GroupsCmdRemoveAllGroupsConfCallback_t   *callback;   /*!< ZCL Confirmation callback handler entry
                                                                            point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                service;    /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqParams_t       params;     /*!< ZCL Request parameters structure. */
};


/*-------------------------------- Add Group If Identifying Cmd ---------------------------------------------------*/
/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Add Group If Identifying command
 * specific to Groups ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.6.2.3.7, Figure 3-13.
 */
typedef struct _ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                           interface to local application. */
    /* Custom parameters. */

    /* 32-bit data. */
    SYS_DataPointer_t                         payload;                /*!< 0-16 characters of the Group Name. This
                                                                           field contains characters only - without
                                                                           length byte. */
    /* 16-bit data. */
    ZBPRO_ZCL_SapGroupsGroupID                groupID;                /*!< The Group ID field is set to the Group ID
                                                                           field of the received Add Group command. */
} ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqParams_t);


/**//**
 * \brief  Add Group If Identifying Confirmation has no custom parameters.
 */
typedef struct _ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;   /*!< Set of obligatory parameters of ZCL public
                                                                        interface to local application. */
    /* There no custom fields.*/

} ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyConfParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Add Group If Identifying command
 * specific to Groups ZCL cluster.
 */
typedef struct   _ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqDescr_t   ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Add Group If Identifying command
 *  specific to Groups ZCL cluster
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 *\note
 * Add Group If Identifying command have no response.
 */
typedef void ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyConfCallback_t(
        ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqDescr_t     *const  reqDescr,
        ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyConfParams_t   *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Add Group If Identifying command
 *  specific to Groups ZCL cluster
 */
struct _ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyConfCallback_t   *callback;   /*!< ZCL Confirmation callback handler entry
                                                                            point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                   service;    /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqParams_t       params;     /*!< ZCL Request parameters structure. */
};


/************************* PROTOTYPES ***************************************************/
/**//**
 * \name    Functions accept ZCL Local Requests to issue Add Group, View Group, Get Group
 *  Membership, Remove Group, Remove All Groups, Add Group If Identifying commands
 *  specific to Groups ZCL cluster.
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
 *  - clusterId             assigned to Groups Cluster Id,
 *  - manufCode             ignored because \c manufSpecific is assigned with FALSE,
 *  - commandId             assigned to Command Id according to particular Local Request,
 *  - transSeqNum           ignored because \c useSpecifiedTsn is assigned with FALSE,
 *  - overallStatus         just ignored,
 *  - direction             assigned to Client-to-Server,
 *  - clusterSpecific       assigned to TRUE (i.e. Cluster-Specific domain),
 *  - manufSpecific         assigned to FALSE (i.e., ZCL Standard type),
 *  - useSpecifiedTsn       assigned to FALSE (i.e., generate new TSN on transmission),
 *  - nonUnicastRequest     just ignored.
 */


/**//**
 * \brief   Accepts ZCL Local Request to issue Add Group
 *  command specific to Groups ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_GroupsCmdAddGroupReq(ZBPRO_ZCL_GroupsCmdAddGroupReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue View Group
 *  command specific to Groups ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_GroupsCmdViewGroupReq(ZBPRO_ZCL_GroupsCmdViewGroupReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Get Group Membership command
 * specific to Groups ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_GroupsCmdGetGroupMembershipReq(ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Remove Group command
 * specific to Groups ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_GroupsCmdRemoveGroupReq(ZBPRO_ZCL_GroupsCmdRemoveGroupReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Remove All Groups command
 * specific to Groups ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReq(ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Add Group If Identifying command
 * specific to Groups ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReq(ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqDescr_t *const  reqDescr);


/**//**
 * \brief   Handles ZCL Local Indication on reception of Get Group Membership response command
 *  specific to Groups ZCL cluster.
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \note
 *  The Get Group Membership response command is sent in response to Get Group Membership command but it
 *  is processed by the client unsolicitedly (because multiple response is allowed). Due
 *  to this reason client side service provides this function.
 */
void ZBPRO_ZCL_GroupsCmdGetGroupMembershipResponseInd(
    ZBPRO_ZCL_GroupsCmdGetGroupMembershipIndParams_t   *const   indParams);

#endif
