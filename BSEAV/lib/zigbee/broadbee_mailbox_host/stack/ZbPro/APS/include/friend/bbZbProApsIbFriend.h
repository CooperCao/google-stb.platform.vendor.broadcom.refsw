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
* FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/friend/bbZbProApsIbFriend.h $
*
* DESCRIPTION:
*   APS-IB API friend interface.
*
* $Revision: 2465 $
* $Date: 2014-05-21 11:21:20Z $
*
*****************************************************************************************/

#ifndef _ZBPRO_APS_IB_FRIEND_H
#define _ZBPRO_APS_IB_FRIEND_H


/************************* INCLUDES *****************************************************/
#include "bbZbProApsSapIb.h"
#include "friend/bbZbProNwkNibApiFriend.h"
#include "friend/bbZbProNwkSecurityMaterialFriend.h"

/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
  \brief Validates an attempt to access the specified APS-IB attribute.
  \param[in] attrId - attribute identifier.
*****************************************************************************************/
IB_AccessStatus_t zbProApsIbValidateGet(uint8_t attrId);

/*************************************************************************************//**
  \brief Reads the specified APS-IB attribute value.
  \param[in] attrId - attribute identifier.
  \result *attrValue - pointer to get the attribute value.
*****************************************************************************************/
void zbProApsIbGet(const uint8_t attrId, ZBPRO_APS_IbAttributeValue_t *attrValue, SYS_DataPointer_t *payload);

/**//**
 * \brief Syntactic sugar for zbProApsIbGet function
 */
#define ZBPRO_APS_GET_ATTR(id, dst) \
    zbProApsIbGet((id), (ZBPRO_APS_IbAttributeValue_t *)(void *)(dst), NULL)

/*************************************************************************************//**
  \brief Validates an attempt to write the new value to the specified APS-IB attribute.
  \param[in] attrId - attribute identifier.
  \result *newValue - pointer to the validated value of the attribute.
*****************************************************************************************/
IB_AccessStatus_t zbProApsIbValidateSet(uint8_t attrId, ZBPRO_APS_IbAttributeValue_t *newValue, SYS_DataPointer_t *payload);

/*************************************************************************************//**
  \brief Writes the new value to the specified NWK-NIB attribute.
  \param[in] attrId - attribute identifier.
  \result *newValue - pointer to the new value of the attribute.
*****************************************************************************************/
void *zbProApsIbSet(const uint8_t attrId, ZBPRO_APS_IbAttributeValue_t *newValue, SYS_DataPointer_t *payload);

/**//**
 * \brief Syntactic sugar for zbProApsIbSet function
 */
#define ZBPRO_APS_SET_ATTR(id, src) \
    zbProApsIbSet((id), (ZBPRO_APS_IbAttributeValue_t *)(void *)(src), NULL)


/*
 * Deletes all record from the Binding Table.
 */
void zbProApsBindingTableReset(void);

/*
 * Deletes all record from the Group Table.
 */
void zbProApsGroupTableReset(void);


#endif /* _ZBPRO_APS_IB_FRIEND_H */