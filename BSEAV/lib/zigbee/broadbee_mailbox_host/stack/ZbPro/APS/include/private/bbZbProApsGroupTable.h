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
 * FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/private/bbZbProApsGroupTable.h $
 *
 * DESCRIPTION:
 *   Declarations of the Group Table component.
 *
 * $Revision: 2186 $
 * $Date: 2014-04-14 10:55:48Z $
 *
 ****************************************************************************************/

#ifndef _ZBPRO_APS_GROUPTABLE_H
#define _ZBPRO_APS_GROUPTABLE_H

/************************* INCLUDES ****************************************************/
#include "private/bbZbProApsMm.h"

/************************* TYPES *******************************************************/

typedef enum _ZbProApsGroupTablePresence_t
{
    ZBPRO_APS_GROUPTABLE_PRESENT    = 0,
    ZBPRO_APS_GROUPTABLE_ABSENT     = 1,
    ZBPRO_APS_GROUPTABLE_NOGROUP    = 2 /* there is no such group in the table */
} ZbProApsGroupTablePresence_t;

typedef enum _ZbProApsGroupTableResult_t
{
    ZBPRO_APS_GROUPTABLE_ADDED      = 0,
    ZBPRO_APS_GROUPTABLE_FULL       = 1,
    ZBPRO_APS_GROUPTABLE_REMOVED    = 2,
    ZBPRO_APS_GROUPTABLE_INVALID    = 3,
    ZBPRO_APS_GROUPTABLE_ALLREMOVED = 4,
} ZbProApsGroupTableResult_t;

/************************* INLINES *****************************************************/

/************************************************************************************//**
  \brief First entry of the APS Group Table.
****************************************************************************************/
INLINE ZbProApsGroupTableEntry_t *zbProApsGroupTableBegin(void)
{
    return &ZbProApsMmDescrGet()->groupTable[0];
}

/************************************************************************************//**
  \brief Returns pointer to memory area after last entry of the APS Group Table.
****************************************************************************************/
INLINE const ZbProApsGroupTableEntry_t *zbProApsGroupTableEnd(void)
{
    return zbProApsGroupTableBegin() + ZBPRO_APS_GROUP_TABLE_SIZE;
}

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
 \brief Returns presence status of the requested pair.
 \param[in] groupAddr - The 16-bit address of the group being tested.
 \param[in] endpoint - The endpoint to which the given group is being tested.
 \return The result of the presence.
 ***************************************************************************************/
APS_PRIVATE ZbProApsGroupTablePresence_t zbProApsGroupTableIncludes(
    ZBPRO_APS_GroupId_t groupAddr, ZBPRO_APS_EndpointId_t endpoint);

/************************************************************************************//**
 \brief To add a particular group for a particular endpoint.
 \param[in] groupAddr - The 16-bit address of the group being added.
 \param[in] endpoint - The endpoint to which the given group is being added.
 \return The result of operation.
 ***************************************************************************************/
APS_PRIVATE ZbProApsGroupTableResult_t zbProApsGroupTableAdd(
    ZBPRO_APS_GroupId_t groupAddr, ZBPRO_APS_EndpointId_t endpoint);

/************************************************************************************//**
 \brief To remove a particular group for a particular endpoint.
 \param[in] groupAddr - The 16-bit address of the group being removed.
 \param[in] endpoint - The endpoint to which the given group is being removed.
 \return The result of operation.
 ***************************************************************************************/
APS_PRIVATE ZbProApsGroupTableResult_t zbProApsGroupTableRemove(
    ZBPRO_APS_GroupId_t groupAddr, ZBPRO_APS_EndpointId_t endpoint);

/************************************************************************************//**
 \brief To remove membership in all groups for an endpoint.
 \param[in] endpoint - The endpoint to which all groups is being removed.
 \return The result of operation.
 ***************************************************************************************/
APS_PRIVATE ZbProApsGroupTableResult_t zbProApsGroupTableRemoveAll(
    ZBPRO_APS_EndpointId_t endpoint);

#endif /* _ZBPRO_APS_GROUPTABLE_H */