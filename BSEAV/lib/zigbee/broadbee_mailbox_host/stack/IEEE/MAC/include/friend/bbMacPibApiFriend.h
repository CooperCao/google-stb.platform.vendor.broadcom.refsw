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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/friend/bbMacPibApiFriend.h $
*
* DESCRIPTION:
*   MAC-PIB API friend interface.
*
* $Revision: 2722 $
* $Date: 2014-06-24 19:37:15Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_PIB_API_FRIEND_H
#define _BB_MAC_PIB_API_FRIEND_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapPib.h"                /* MAC-PIB for MAC-SAP definitions. */
#include "friend/bbPhyPibApiFriend.h"   /* PHY-PIB API friend interface. */


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Validates an attempt to read the specified MAC-PIB public attribute.
 * \param[in]   attrId      Identifier of the specified MAC-PIB public attribute.
 * \return  One of the following status values:
 *  - IB_SUCCESS                    if the requested attribute is implemented by the
 *                                  MAC-PIB for the specified MAC Context or by the
 *                                  PHY-PIB (i.e. if it is a MAC-PIB public attribute),
 *  - IB_UNSUPPORTED_ATTRIBUTE      otherwise.
 *
 * \details
 *  All the MAC-PIB attributes listed in the IEEE 802.15.4-2006 are implemented at least
 *  as read-only attributes for both MAC Contexts, except the MAC security attributes that
 *  are not implemented.
 * \details
 *  For non-MAC-PIB private attributes this function calls the nested phyPibApiValidateGet
 *  function to discover if this attribute is implemented by the PHY-PIB and returns its
 *  answer to the caller.
*****************************************************************************************/
MAC_FRIEND IB_AccessStatus_t macPibApiValidateGet(const MAC_PibAttributeId_t attrId);


/*************************************************************************************//**
 * \brief   Reads the specified MAC-PIB public attribute value.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[in]   attrId              Identifier of the specified MAC-PIB public attribute.
 * \param[out]  value               Pointer to preallocated structure of type Variant to
 *  receive value of the requested scalar type attribute.
 * \param[out]  payload             Pointer to preallocated payload descriptor to receive
 *  value of variable size of the requested array type attribute.
 * \details
 *  This function assigns objects pointed either by \p value or \p payload argument with
 *  the requested attribute value, scalar or array respectively, according to the type of
 *  the requested attribute:
 *  - macBeaconPayload          is array type attribute - static payload,
 *  - phyChannelsSupported      is array type attribute - static payload,
 *  - all others                are scalar type attributes.
 *
 *  The second argument (that is unused for the specified attribute) may be set to NULL by
 *  the caller, but it also may be assigned with the pointer to existent empty object for
 *  uniformity. In the second case (i.e., if the unused argument is not NULL) the pointed
 *  object is zeroed (for scalars) or emptied (for arrays) by this function.
 * \details
 *  Call macPibApiValidateGet for the given attribute identifier to make sure that the
 *  specified attribute is implemented either by the MAC-PIB or the PHY-PIB and accessible
 *  for read. The result of validation does not depend on the MAC Context selection.
 * \details
 *  For non-MAC-PIB private attributes this function calls the nested phyPibApiGet
 *  function to obtain the attribute value from the PHY-PIB and transfers its value
 *  (scalar or array) to the caller.
 * \note
 *  The PHY-PIB attributes phyCurrentChannel and phyCurrentPage are overloaded by the
 *  MAC-PIB to make their values context-specific. Their names in the MAC-PIB private
 *  attributes set are macCurrentChannel and macCurrentPage respectively. Nevertheless
 *  these attributes are accessible in the MAC-PIB by their conventional identifiers just
 *  as they would be implemented by the PHY-PIB. Actual PHY-PIB attributes
 *  phyCurrentChannel and phyCurrentPage are hidden by the MAC-PIB friend API and are not
 *  accessible from the outside via this function. For the case of single-context MAC it
 *  makes no difference with the standard MAC and PHY behavior. But for the case of
 *  dual-context MAC these attributes may have different values for two MAC contexts. For
 *  dual-context MAC, when both contexts are enabled, the actual current channel and page
 *  are set according to the ZigBee PRO context MAC-PIB attributes; and values of these
 *  attributes from the RF4CE context MAC-PIB are used just for temporary channel hopping
 *  just for the time of transactions (transmission and corresponding acknowledgment)
 *  performed by the RF4CE MAC context. These MAC-PIB attributes unlike conventional ones
 *  are read-only if accessed via this function. In order to assign channel or channel
 *  page the higher layer shall use MLME-SET.request primitive.
*****************************************************************************************/
MAC_FRIEND void macPibApiGet(MAC_WITH_GIVEN_CONTEXT(
                             const MAC_PibAttributeId_t      attrId,
                             MAC_PibAttributeValue_t *const  value,
                             SYS_DataPointer_t *const        payload));


/*************************************************************************************//**
 * \brief   Validates an attempt to assign a new value to the specified MAC-PIB public
 *   attribute.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[in]   attrId              Identifier of the specified MAC-PIB public attribute.
 * \param[in]   value               Pointer to data structure of type Variant with the new
 *  value for the scalar type attribute to be assigned with.
 * \param[in]   payload             Pointer to payload descriptor with the new value of
 *  variable size for the array type attribute to be assigned with.
 * \return  One of the following status values:
 *  - IB_SUCCESS                    if the written attribute is implemented by the MAC-PIB
 *                                  or by the PHY-PIB, is write-enabled, and the specified
 *                                  value is valid for it for the specified MAC Context,
 *  - IB_UNSUPPORTED_ATTRIBUTE      if the attribute is not implemented in the context
 *                                  neither by the MAC-PIB nor by the PHY-PIB,
 *  - IB_READ_ONLY                  if the attribute is read-only for the MAC Context,
 *  - IB_INVALID_PARAMETER          if the given value is not valid for the MAC Context.
 *
 * \note
 *  This function uses either \p value or \p payload argument according to the type of the
 *  validated attribute; the second argument (that is unused for the specified  attribute)
 *  may even not be provided by the caller (i.e., it is allowed to specify NULL instead of
 *  the unused attribute).
 * \note
 *  For non-MAC-PIB private attributes this function calls the nested phyPibApiValidateSet
 *  function to discover if and how this attribute is implemented by the PHY-PIB and
 *  transfers its answer to the caller.
*****************************************************************************************/
MAC_FRIEND IB_AccessStatus_t macPibApiValidateSet(MAC_WITH_GIVEN_CONTEXT(
                                                  const MAC_PibAttributeId_t            attrId,
                                                  const MAC_PibAttributeValue_t *const  value,
                                                  const SYS_DataPointer_t *const        payload));


/*************************************************************************************//**
 * \brief   Assigns the new value to the specified MAC-PIB public attribute.
 * \param[in]   __givenContextId    Identifier of the specified context.
 * \param[in]   attrId              Identifier of the specified MAC-PIB public attribute.
 * \param[in]   value               Pointer to data structure of type Variant with the new
 *  value for the scalar type attribute to be assigned with.
 * \param[in]   payload             Pointer to payload descriptor with the new value of
 *  variable size for the array type attribute to be assigned with.
 * \note
 *  This function uses either \p value or \p payload argument according to the type of the
 *  assigned attribute; the second argument (that is unused for the specified  attribute)
 *  may even not be provided by the caller (i.e., it is allowed to specify NULL instead of
 *  the unused attribute).
 * \note
 *  Call macPibApiValidateSet for the given attribute identifier, its new value and MAC
 *  Context to make sure that the specified attribute is implemented either by the MAC-PIB
 *  or the PHY-PIB and accessible for write and the new value is legal for it. The result
 *  of validation depends on MAC Context selection.
 * \note
 *  For non-MAC-PIB private attributes this function calls the nested phyPibApiSet
 *  function to accept this attribute value with the PHY-PIB.
*****************************************************************************************/
MAC_FRIEND void macPibApiSet(MAC_WITH_GIVEN_CONTEXT(
                             const MAC_PibAttributeId_t            attrId,
                             const MAC_PibAttributeValue_t *const  value,
                             const SYS_DataPointer_t *const        payload));


#endif /* _BB_MAC_PIB_API_FRIEND_H */