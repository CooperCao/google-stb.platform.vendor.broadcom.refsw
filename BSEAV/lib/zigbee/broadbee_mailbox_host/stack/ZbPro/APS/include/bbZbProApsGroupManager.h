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
 *      This header describes API for the ZigBee PRO APS Group Manager component.
 *
*******************************************************************************/

#ifndef _ZBPRO_APS_GROUP_MANAGER_H
#define _ZBPRO_APS_GROUP_MANAGER_H

/************************* INCLUDES ****************************************************/
#include "bbZbProApsCommon.h"

/************************* TYPES *******************************************************/

/**//**
 * \brief Group Manager request type.
 * \ingroup ZBPRO_APS_Misc
 */
typedef enum _ZbProApsGroupManagerReqType_t
{
    ZBPRO_APS_GROUPMANAGER_ADD           = 0,
    ZBPRO_APS_GROUPMANAGER_REMOVE        = 1,
    ZBPRO_APS_GROUPMANAGER_REMOVEALL     = 2
} ZbProApsGroupManagerReqType_t;

/**//**
 * \brief Service structure type.
 * \ingroup ZBPRO_APS_Misc
 */
typedef struct _ZbProApsGroupManagerServiceField_t
{
    SYS_QueueElement_t   queueElement;     /*!< Service queue field. */
    ZbProApsGroupManagerReqType_t reqType; /*!< Request type (Add/Remove/RemoveAll). */
} ZbProApsGroupManagerServiceField_t;

/**//**
 * \brief APSME-ADD-GROUP.request parameters structure, see ZigBee Specification r20, 2.2.4.5.1.
 * \ingroup ZBPRO_APS_AddGroupReq
 */
typedef struct _ZBPRO_APS_AddGroupReqParams_t
{
    ZBPRO_APS_GroupId_t groupAddr;         /*!< The 16-bit address of the group being added. */
    ZBPRO_APS_EndpointId_t endpoint;       /*!< The endpoint to which the given group is being added.
                                                Valid range for endpoint is 0x1..0xfe. */
} ZBPRO_APS_AddGroupReqParams_t;

/**//**
 * \brief APSME-ADD-GROUP.confirm parameters structure, see ZigBee Specification r20, 2.2.4.5.2.
 * \ingroup ZBPRO_APS_AddGroupConf
 */
typedef struct _ZBPRO_APS_AddGroupConfParams_t
{
    ZBPRO_APS_Status_t status;             /*!< Status of execution. */
    ZBPRO_APS_GroupId_t groupAddr;         /*!< The 16-bit address of the group being added. */
    ZBPRO_APS_EndpointId_t endpoint;       /*!< The endpoint to which the given group is being added. */
} ZBPRO_APS_AddGroupConfParams_t;

/**//**
 * \brief APSME-ADD-GROUP.request descriptor data type declaration.
 * \ingroup ZBPRO_APS_AddGroupReq
 */
typedef struct _ZBPRO_APS_AddGroupReqDescr_t  ZBPRO_APS_AddGroupReqDescr_t;

/**//**
 * \brief APSME-ADD-GROUP.confirm primitive callback function type.
 * \ingroup ZBPRO_APS_AddGroupConf
 */
typedef void ZBPRO_APS_AddGroupConfCallback_t(ZBPRO_APS_AddGroupReqDescr_t   *const reqDescr,
        ZBPRO_APS_AddGroupConfParams_t *const confParams);

/**//**
 * \brief APSME-ADD-GROUP.request primitive structure.
 * \ingroup ZBPRO_APS_AddGroupReq
 */
typedef struct _ZBPRO_APS_AddGroupReqDescr_t
{
    ZbProApsGroupManagerServiceField_t  service;        /*!< Request service field. */
    ZBPRO_APS_AddGroupReqParams_t       params;         /*!< Request parameters. */
    ZBPRO_APS_AddGroupConfCallback_t    *callback;      /*!< Confirmation callback. */
} ZBPRO_APS_AddGroupReqDescr_t;

/**//**
 * \brief APSME-REMOVE-GROUP.request parameters structure, see ZigBee Specification r20, 2.2.4.5.3.
 * \ingroup ZBPRO_APS_RemoveGroupReq
 */
typedef struct _ZBPRO_APS_RemoveGroupReqParams_t
{
    ZBPRO_APS_GroupId_t groupAddr;         /*!< The 16-bit address of the group being removed. */
    ZBPRO_APS_EndpointId_t endpoint;       /*!< The endpoint to which the given group is being removed.
                                                Valid range for endpoint is 0x1..0xfe. */
} ZBPRO_APS_RemoveGroupReqParams_t;

/**//**
 * \brief APSME-REMOVE-GROUP.confirm parameters structure, see ZigBee Specification r20, 2.2.4.5.4.
 * \ingroup ZBPRO_APS_RemoveGroupConf
 */
typedef struct _ZBPRO_APS_RemoveGroupConfParams_t
{
    ZBPRO_APS_Status_t status;             /*!< Status of execution. */
    ZBPRO_APS_GroupId_t groupAddr;         /*!< The 16-bit address of the group being removed. */
    ZBPRO_APS_EndpointId_t endpoint;       /*!< The endpoint which is to be removed from the group. */
} ZBPRO_APS_RemoveGroupConfParams_t;

/**//**
 * \brief APSME-REMOVE-GROUP.request descriptor data type declaration.
 * \ingroup ZBPRO_APS_RemoveGroupReq
 */
typedef struct _ZBPRO_APS_RemoveGroupReqDescr_t  ZBPRO_APS_RemoveGroupReqDescr_t;

/**//**
 * \brief APSME-REMOVE-GROUP.confirm primitive callback function type.
 * \ingroup ZBPRO_APS_RemoveGroupConf
 */
typedef void ZBPRO_APS_RemoveGroupConfCallback_t(ZBPRO_APS_RemoveGroupReqDescr_t *const reqDescr,
        ZBPRO_APS_RemoveGroupConfParams_t *const confParams);

/**//**
 * \brief APSME-REMOVE-GROUP.request primitive structure.
 * \ingroup ZBPRO_APS_RemoveGroupReq
 */
typedef struct _ZBPRO_APS_RemoveGroupReqDescr_t
{
    ZbProApsGroupManagerServiceField_t   service;       /*!< Request service field. */
    ZBPRO_APS_RemoveGroupReqParams_t     params;        /*!< Request parameters. */
    ZBPRO_APS_RemoveGroupConfCallback_t  *callback;     /*!< Confirmation callback. */
} ZBPRO_APS_RemoveGroupReqDescr_t;

/**//**
 * \brief APSME-REMOVE-ALL-GROUPS.request parameters structure, see ZigBee Specification r20, 2.2.4.5.5.
 * \ingroup ZBPRO_APS_RemoveAllGroupsReq
 */
typedef struct _ZBPRO_APS_RemoveAllGroupsReqParams_t
{
    ZBPRO_APS_EndpointId_t endpoint;       /*!< The endpoint to which all groups are being removed.
                                                Valid range for endpoint is 0x1..0xfe. */
} ZBPRO_APS_RemoveAllGroupsReqParams_t;

/**//**
 * \brief APSME-REMOVE-ALL-GROUPS.confirm parameters structure, see ZigBee Specification r20, 2.2.4.5.6.
 * \ingroup ZBPRO_APS_RemoveAllGroupsConf
 */
typedef struct _ZBPRO_APS_RemoveAllGroupsConfParams_t
{
    ZBPRO_APS_Status_t status;             /*!< Status of execution. */
    ZBPRO_APS_EndpointId_t endpoint;       /*!< The endpoint which is to be removed from all groups. */
} ZBPRO_APS_RemoveAllGroupsConfParams_t;

/**//**
 * \brief APSME-REMOVE-ALL-GROUPS.request descriptor data type declaration.
 * \ingroup ZBPRO_APS_RemoveAllGroupsReq
 */
typedef struct _ZBPRO_APS_RemoveAllGroupsReqDescr_t  ZBPRO_APS_RemoveAllGroupsReqDescr_t;

/**//**
 * \brief APSME-REMOVE-ALL-GROUPS.confirm primitive callback function type.
 * \ingroup ZBPRO_APS_RemoveAllGroupsConf
 */
typedef void ZBPRO_APS_RemoveAllGroupsConfCallback_t(ZBPRO_APS_RemoveAllGroupsReqDescr_t *const reqDescr,
        ZBPRO_APS_RemoveAllGroupsConfParams_t *const confParams);

/**//**
 * \brief APSME-REMOVE-ALL-GROUPS.request primitive structure.
 * \ingroup ZBPRO_APS_RemoveAllGroupsReq
 */
typedef struct _ZBPRO_APS_RemoveAllGroupsReqDescr_t
{
    ZbProApsGroupManagerServiceField_t       service;   /*!< Request service field. */
    ZBPRO_APS_RemoveAllGroupsReqParams_t     params;    /*!< Request parameters. */
    ZBPRO_APS_RemoveAllGroupsConfCallback_t  *callback; /*!< Confirmation callback. */
} ZBPRO_APS_RemoveAllGroupsReqDescr_t;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
  \brief APSME-ADD-GROUP.request primitive function.
  \ingroup ZBPRO_APS_Functions

  \param[in] reqDescr - pointer to the request structure.
  \return Nothing.
****************************************************************************************/
void ZBPRO_APS_AddGroupReq(ZBPRO_APS_AddGroupReqDescr_t *reqDescr);

/************************************************************************************//**
  \brief APSME-REMOVE-GROUP.request primitive function.
  \ingroup ZBPRO_APS_Functions

  \param[in] reqDescr - pointer to the request structure.
  \return Nothing.
****************************************************************************************/
void ZBPRO_APS_RemoveGroupReq(ZBPRO_APS_RemoveGroupReqDescr_t *reqDescr);

/************************************************************************************//**
  \brief APSME-REMOVE-ALL-GROUPS.request primitive function.
  \ingroup ZBPRO_APS_Functions

  \param[in] reqDescr - pointer to the request structure.
  \return Nothing.
****************************************************************************************/
void ZBPRO_APS_RemoveAllGroupsReq(ZBPRO_APS_RemoveAllGroupsReqDescr_t *reqDescr);

#endif /* _ZBPRO_APS_GROUP_MANAGER_H */

/* eof bbZbProApsGroupManager.h */