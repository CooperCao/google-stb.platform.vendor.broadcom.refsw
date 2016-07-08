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
* FILENAME: $Workfile: trunk/stack/IEEE/PHY/include/friend/bbPhyPibApiFriend.h $
*
* DESCRIPTION:
*   PHY-PIB API friend interface.
*
* $Revision: 2722 $
* $Date: 2014-06-24 19:37:15Z $
*
*****************************************************************************************/


#ifndef _BB_PHY_PIB_API_FRIEND_H
#define _BB_PHY_PIB_API_FRIEND_H


/************************* INCLUDES *****************************************************/
#include "bbPhySapPib.h"            /* PHY-PIB for PHY-SAP definitions. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of common access statuses to information bases (IB).
 * \details These statuses are returned by validate-access functions of IB friend APIs.
 *  These common statuses are introduced in order to make the APIs compatible between
 *  different layers and to provide ability for IB friend API nesting through the Stack.
 * \note    Corresponding layer-specific status identifiers numeric values are completely
 *  different for all layers. Here is the cross-reference for different layers:
 *  | Status                | PHY  | MAC  | RF4CE | NWK  | APS  |
 *  | --------------------- | ---- | ---- | ----- | ---- | ---- |
 *  | SUCCESS               | 0x07 | 0x00 | 0x00  | 0x00 | 0x00 |
 *  | UNSUPPORTED_ATTRIBUTE | 0x0A | 0xF4 | 0xF4  | 0xC9 | 0xB0 |
 *  | READ_ONLY             | 0x0B | 0xFB | -     | -    | -    |
 *  | INVALID_INDEX         | -    | 0xF9 | 0xF9  | -    | -    |
 *  | INVALID_PARAMETER     | 0x05 | 0xE8 | 0xE8  | 0xC1 | 0xA6 |
 */
typedef enum _IB_AccessStatus_t
{
    IB_SUCCESS,                 /*!< The requested operation was completed successfully. */

    IB_UNSUPPORTED_ATTRIBUTE,   /*!< A request was issued for an attribute that is not supported. */

    IB_READ_ONLY,               /*!< A SET request was issued for the read-only attribute. */

    IB_INVALID_INDEX,           /*!< The specified table index is out of range. */

    IB_INVALID_PARAMETER,       /*!< The specified parameter is either not supported or is out of the valid range. */

} IB_AccessStatus_t;


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Validates an attempt to read the specified PHY-PIB public attribute.
 * \param[in]   attrId      Identifier of the specified PHY-PIB public attribute.
 * \return  One of the following status values:
 *  - IB_SUCCESS                if the requested attribute is implemented by the PHY-PIB,
 *  - IB_UNSUPPORTED_ATTRIBUTE  otherwise.
 *
 * \note
 *  All the PHY-PIB attributes listed in the IEEE 802.15.4-2006 are implemented at least
 *  as read-only attributes.
*****************************************************************************************/
PHY_FRIEND IB_AccessStatus_t phyPibApiValidateGet(const PHY_PibAttributeId_t attrId);


/*************************************************************************************//**
 * \brief   Reads the specified PHY-PIB public attribute value.
 * \param[in]   attrId      Identifier of the specified PHY-PIB public attribute.
 * \param[out]  value       Pointer to preallocated structure of type Variant to receive
 *  value of the requested scalar type attribute.
 * \param[out]  payload     Pointer to preallocated payload descriptor to receive value of
 *  variable size of the requested array type attribute.
 * \note
 *  This function assigns objects pointed either by \p value or \p payload argument with
 *  the requested attribute value, scalar or array respectively, according to the type of
 *  the requested attribute:
 *  - phyChannelsSupported      is array type attribute - static payload,
 *  - all others                are scalar type attributes.
 *
 *  The second argument (that is unused for the specified attribute) may be set to NULL by
 *  the caller, but it also may be assigned with the pointer to existent empty object for
 *  uniformity. In the second case (i.e., if the unused argument is not NULL) the pointed
 *  object is zeroed (for scalars) or emptied (for arrays) by this function.
 * \note
 *  Call phyPibApiValidateGet for the given attribute identifier to make sure that the
 *  specified attribute is implemented by the PHY-PIB and accessible for read.
*****************************************************************************************/
PHY_FRIEND void phyPibApiGet(const PHY_PibAttributeId_t      attrId,
                             PHY_PibAttributeValue_t *const  value,
                             SYS_DataPointer_t *const        payload);


/*************************************************************************************//**
 * \brief   Validates an attempt to assign a new value to the specified PHY-PIB public
 *  attribute.
 * \param[in]   attrId      Identifier of the specified PHY-PIB public attribute.
 * \param[in]   value       Pointer to data structure of type Variant with the new value
 *  for the attribute to be assigned with.
 * \return  One of the following status values:
 *  - IB_SUCCESS                    if the written attribute is implemented by the
 *   PHY-PIB, write-enabled and the specified value is valid for it,
 *  - IB_UNSUPPORTED_ATTRIBUTE      if the attribute is not implemented by the PHY-PIB,
 *  - IB_READ_ONLY                  if the attribute is read-only,
 *  - IB_INVALID_PARAMETER          if the given value is not valid.
 * \note
 *  The PHY-PIB has no array type write-enabled attributes. All write-enabled attributes
 *  are scalar type.
*****************************************************************************************/
PHY_FRIEND IB_AccessStatus_t phyPibApiValidateSet(const PHY_PibAttributeId_t            attrId,
                                                  const PHY_PibAttributeValue_t *const  value);


/*************************************************************************************//**
 * \brief   Assigns the new value to the specified PHY-PIB public attribute.
 * \param[in]   attrId      Identifier of the specified MAC-PIB public attribute.
 * \param[in]   value       Pointer to data structure of type Variant with the new value
 *  for the attribute to be assigned with.
 * \note
 *  Call \c phyPibApiValidateSet for the given attribute identifier and its new value to
 *  make sure that the specified attribute is implemented and accessible for write and the
 *  new value is legal for it.
 * \note
 *  The PHY-PIB has no array type write-enabled attributes. All write-enabled attributes
 *  are scalar type.
 * \note
 *  The PHY-PIB attributes phyCurrentChannel and phyCurrentPage are unlike conventional
 *  ones are read-only if accessed via this function. In order to assign channel or
 *  channel page the higher layer shall use the nonstandard PLME-SET-CHANNEL.request
 *  primitive.
*****************************************************************************************/
PHY_FRIEND void phyPibApiSet(const PHY_PibAttributeId_t            attrId,
                             const PHY_PibAttributeValue_t *const  value);


#endif /* _BB_PHY_PIB_API_FRIEND_H */