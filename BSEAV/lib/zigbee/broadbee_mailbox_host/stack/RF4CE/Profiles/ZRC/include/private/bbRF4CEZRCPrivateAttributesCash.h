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
*       RF4CE ZRC 2.0 Attributes Cash private interface.
*
*******************************************************************************/

#ifndef _BB_RF4CE_ZRC_PRIVATE_ATTRIBUTES_CASH_H
#define _BB_RF4CE_ZRC_PRIVATE_ATTRIBUTES_CASH_H

/************************* INCLUDES ***********************************************************************************/
#include "bbRF4CEZRCAttributes.h"

/************************* PROTOTYPES *********************************************************************************/
/**//**
 * \brief   Initializes Attributes Cash at the firmware startup.
 * \details Must be called only once.
 */
void rf4cezrc2AttributesCashInit(void);

/**//**
 * \brief   Retrieves pointer to the origin and size of the desired ZRC 2.0 attribute stored in the Cash.
 * \param[in]   pAttrIdx        Pointer to the structure keeping the Attribute Id and the Attribute Element Index of the
 *  requested attribute or arrayed attribute element.
 * \param[out]  pAttrSize       Pointer to the variable that will be assigned with the size of the retrieved attribute
 *  (or attribute element). Must not be NULL.
 * \param[out]  ppAttrValue     Pointer to the variable that will be assigned with the pointer to the requested
 *  attribute (or attribute element) origin. Must not be NULL.
 * \param[in]   pairingRef      Pairing reference of the requested attribute, in the range from 0x00 to 0xFE, or 0xFF.
 *  The value 0xFF is used if this node own attribute is requested; values 0x00...0xFE are used if an attribute
 *  requested belongs to the corresponding paired node RIB disposed on this node.
 * \return  Status of the performed attempt to retrieve the requested attribute or arrayed attribute element - one of
 *  the following values: SUCCESS, UNSUPPORTED_ATTRIBUTE, ILLEGAL_REQUEST, INVALID_ENTRY, CASH_MISS.
 * \details If the Attribute Id denotes an attribute that is not implemented by either GDP 2.0 or ZRC 2.0 profile, the
 *  UNSUPPORTED_ATTRIBUTE status is returned.
 * \details The Attribute Index is used only in the case when the requested attribute is an arrayed attribute. Otherwise
 *  this parameter is ignored. If the Attribute Index falls outside the allowed range for the element index of the
 *  requested attribute, the INVALID_ENTRY status is returned.
 * \details The direct pointer to the memory within the Cash is returned with the \p ppAttrValue. The caller should not
 *  modify the attribute value instance disposed in the Cash, except the case of the SET method.
 * \details The Pairing Reference specified with the \p pairingRef must be in the range from 0x00 to 0xFE, or equal to
 *  0xFF. The value 0xFF is used if this node own attribute is requested; values 0x00...0xFE are used if the requested
 *  attribute belongs to the corresponding paired node RIB disposed on this node. If the specified pairing record is not
 *  valid (does not exist), except the special code 0xFF, the ILLEGAL_REQUEST status is returned.
 * \details If the requested attribute (or attribute element) potentially exists but does not found in the Cash, the
 *  CASH_MISS status is returned.
 */
RF4CE_ZRC2GDP2_GetAttributeStatus_t  rf4cezrc2AttributesCashRetrieveAttrEntry(
                const RF4CE_ZRC2_AttributeId_t *const  pAttrIdx,
                uint8_t *const  pAttrSize,
                void * *const  ppAttrValue,
                const uint8_t  pairingRef);

#endif /* _BB_RF4CE_ZRC_PRIVATE_ATTRIBUTES_CASH_H */

/* eof bbRF4CEZRCPrivateAttributesCash.h */