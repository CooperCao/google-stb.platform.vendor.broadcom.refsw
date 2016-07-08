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
*
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   MAC Security PIB interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/

#ifndef _BB_MAC_SECURITY_PIB_H
#define _BB_MAC_SECURITY_PIB_H

/************************* INCLUDES ***********************************************************************************/
#include "private/bbMacSecurityDefs.h"
#include "private/bbMacMemory.h"


/************************* VALIDATIONS ********************************************************************************/
#if !defined(_MAC_CONTEXT_ZBPRO_)
# error This file requires the MAC Context for ZigBee PRO to be included into the build.
#endif

/************************* PROTOTYPES *********************************************************************************/
/**//**
 * \brief   Validates an attempt to read and reads the specified MAC-PIB Security public attribute.
 * \param[in]   attrId      Identifier of the specified attribute.
 * \param[in]   attrIndex   Index over items of the given table-attribute. Ignored for scalar type attributes.
 * \param[out]  pValue      Pointer to preallocated structure of type Variant to receive value of the requested scalar
 *  type attribute.
 * \param[out]  pPayload    Pointer to preallocated payload descriptor to receive value of variable size of the
 *  requested array type attribute.
 * \return  Status of operation.
 * \note    The payload must not be allocated by the caller. Only the payload descriptor is needed.
 */
MAC_PRIVATE MAC_Status_t macSecurityPibGetZBPRO(
        const MAC_PibAttributeId_t attrId,
        const MAC_PibAttributeIndex_t attrIndex,
        MAC_PibAttributeValue_t *const pValue,
        SYS_DataPointer_t *const pPayload);

/**//**
 * \brief   Validates an attempt to assign and assigns a new value to the specified MAC-PIB Security public attribute.
 * \param[in]       attrId      Identifier of the specified attribute.
 * \param[in]       attrIndex   Index over items of the given table-attribute. Ignored for scalar type attributes.
 * \param[in]       pValue      Pointer to data structure of type Variant with the new value for the scalar type
 *  attribute to be assigned with.
 * \param[in/out]   pPayload    Pointer to payload descriptor with the new value of variable size for the array type
 *  attribute to be assigned with.
 * \return  Status of operation.
 * \note    For the macKeyTable attribute the payload provided by the caller is captured by the MAC Security PIB. The
 *  caller shall not free this payload. The payload descriptor pointed with the \p pPayload is forced to EMPTY_PAYLOAD
 *  by this function in order to notify the caller of this fact.
 */
MAC_PRIVATE MAC_Status_t macSecurityPibSetZBPRO(
        const MAC_PibAttributeId_t attrId,
        const MAC_PibAttributeIndex_t attrIndex,
        const MAC_PibAttributeValue_t *const value,
        SYS_DataPointer_t *const payload);

/*--------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Cleanup links from the MAC-PIB security attribute macKeyDescriptor.
 * \param[in]   index    Index of cleanup of the macKeyDescriptor specified MAC-PIB attribute.
 * \return  Status of operation.
 */
MAC_PRIVATE void macSecurityPibCleanupLinksFromKeyDescriptorZBPRO(const MAC_KeyDescriptorId_t index);

/**//**
 * \brief   Assigns a new value to the MAC-PIB security attribute macKeyTableEntries.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
 * \return  Status of operation.
 */
MAC_PRIVATE MAC_Status_t macSecurityPibSetKeyTableEntriesZBPRO(const MAC_KeyTableEntries_t newValue);

/**//**
 * \brief   Returns value of single item of MAC-PIB security table-attribute macKeyTable.
 * \param[in]   index   Index over the table attribute.
 * \param[out]  pValue  Pointer to the buffer where to return current value.
 * \return  Status of operation.
 */
MAC_PRIVATE MAC_Status_t macSecurityPibGetKeyDescriptorZBPRO(
        const MAC_KeyDescriptorId_t index,
        MAC_KeyDescriptor_t *const pValue);

/**//**
 * \brief   Assigns a new value to the MAC-PIB security table-attribute macKeyTable.
 * \param[in]   index       Index over the table attribute.
 * \param[in]   pNewValue   Pointer to the buffer with new value.
 * \return  Status of operation.
 */
MAC_PRIVATE MAC_Status_t macSecurityPibSetKeyDescriptorZBPRO(
        const MAC_KeyDescriptorId_t index,
        const MAC_KeyDescriptor_t *const pNewValue);

/*--------------------------------------------------------------------------------------------------------------------*/

/**//**
 * \brief   Assigns a new value to the MAC-PIB security attribute macDeviceTableEntries.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
 * \return  Status of operation.
 */
MAC_PRIVATE MAC_Status_t macSecurityPibSetDeviceTableEntriesZBPRO(const MAC_DeviceTableEntries_t newValue);

/**//**
 * \brief   Returns value of single item of MAC-PIB security table-attribute macDeviceTable.
 * \param[in]   index   Index over the table attribute.
 * \param[out]  pValue  Pointer to the buffer where to return current value.
 * \return  Status of operation.
 */
MAC_PRIVATE MAC_Status_t macSecurityPibGetDeviceDescriptorZBPRO(
        const MAC_DeviceDescriptorId_t index,
        MAC_DeviceDescriptor_t *const pValue);

/**//**
 * \brief   Assigns a new value to the MAC-PIB security table-attribute macDeviceTable.
 * \param[in]   index       Index over the table attribute.
 * \param[in]   pNewValue   Pointer to the buffer with new value.
 * \return  Status of operation.
 */
MAC_PRIVATE MAC_Status_t macSecurityPibSetDeviceDescriptorZBPRO(
        const MAC_DeviceDescriptorId_t index,
        const MAC_DeviceDescriptor_t *const pNewValue);

/*--------------------------------------------------------------------------------------------------------------------*/

/**//**
 * \brief   Assigns a new value to the MAC-PIB security attribute macSecurityLevelTableEntries.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
 * \return  Status of operation.
 */
MAC_PRIVATE MAC_Status_t macSecurityPibSetSecurityLevelTableEntriesZBPRO(const MAC_SecurityLevelTableEntries_t newValue);

/**//**
 * \brief   Returns value of single item of MAC-PIB security table-attribute macSecurityLevelTable.
 * \param[in]   index   Index over the table attribute.
 * \param[out]  pValue  Pointer to the buffer where to return current value.
 * \return  Status of operation.
 */
MAC_PRIVATE MAC_Status_t macSecurityPibGetSecurityLevelDescriptorZBPRO(
        const MAC_SecurityLevelDescriptorId_t index,
        MAC_SecurityLevelDescriptor_t *const pValue);

/**//**
 * \brief   Assigns a new value to the MAC-PIB security table-attribute macSecurityLevelTable.
 * \param[in]   index       Index over the table attribute.
 * \param[in]   pNewValue   Pointer to the buffer with new value.
 * \return  Status of operation.
 */
MAC_PRIVATE MAC_Status_t macSecurityPibSetSecurityLevelDescriptorZBPRO(
        const MAC_SecurityLevelDescriptorId_t index,
        const MAC_SecurityLevelDescriptor_t *const pNewValue);

/************************* INLINES ************************************************************************************/
/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Returns value of the MAC-PIB Security attribute macThreadMode.
 * \return  Value of the requested MAC-PIB attribute.
 */
INLINE MAC_ThreadMode_t macSecurityPibGetThreadModeZBPRO(void)
{
    return MAC_MEMORY_PIB_ZBPRO().macThreadMode;
}

/**//**
 * \brief   Assigns a new value to the ZigBee PRO MAC-PIB Security attribute macThreadMode.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
 */
INLINE void macSecurityPibSetThreadModeZBPRO(const MAC_ThreadMode_t newValue)
{
    MAC_MEMORY_PIB_ZBPRO().macThreadMode = newValue;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Returns value of the MAC-PIB security attribute macKeyTableEntries.
 * \return  Value of the requested MAC-PIB attribute.
 */
INLINE MAC_KeyTableEntries_t macSecurityPibGetKeyTableEntriesZBPRO(void)
{
    return macKeyTableEntries;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Returns value of the MAC-PIB security attribute macDeviceTableEntries.
 * \return  Value of the requested MAC-PIB attribute.
 */
INLINE MAC_DeviceTableEntries_t macSecurityPibGetDeviceTableEntriesZBPRO(void)
{
    return macDeviceTableEntries;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Returns value of the MAC-PIB security attribute macSecurityLevelTableEntries.
 * \return  Value of the requested MAC-PIB attribute.
 */
INLINE MAC_SecurityLevelTableEntries_t macSecurityPibGetSecurityLevelTableEntriesZBPRO(void)
{
    return macSecurityLevelTableEntries;
}


/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Returns value of the MAC-PIB security attribute macFrameCounter.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
 * \return  Value of the requested MAC-PIB attribute.
 */
INLINE MAC_FrameCounter_t macSecurityPibGetFrameCounterZBPRO(void)
{
    return macFrameCounter;
}

/**//**
 * \brief   Assigns a new value to the MAC-PIB security attribute macFrameCounter.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
 */
INLINE void macSecurityPibSetFrameCounterZBPRO(const MAC_FrameCounter_t newValue)
{
    macFrameCounter = newValue;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Returns value of the MAC-PIB security attribute macAutoRequestSecurityLevel.
 * \return  Value of the requested MAC-PIB attribute.
 */
INLINE MAC_SecurityLevel_t macSecurityPibGetAutoRequestSecurityLevelZBPRO(void)
{
    return macAutoRequestSecurityLevel;
}

/**//**
 * \brief   Assigns a new value to the MAC-PIB security attribute macAutoRequestSecurityLevel.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
 * \return  Status of operation.
 */
INLINE MAC_Status_t macSecurityPibSetAutoRequestSecurityLevelZBPRO(const MAC_SecurityLevel_t newValue)
{
    if (newValue > MAC_SECURITY_LEVEL__ENC_MIC_128)
        return MAC_INVALID_PARAMETER;

    macAutoRequestSecurityLevel = newValue;
    return MAC_SUCCESS;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Returns value of the MAC-PIB security attribute macAutoRequestKeyIdMode.
 * \return  Value of the requested MAC-PIB attribute.
 */
INLINE MAC_KeyIdMode_t macSecurityPibGetAutoRequestKeyIdModeZBPRO(void)
{
    return macAutoRequestIdMode;
}

/**//**
 * \brief   Assigns a new value to the MAC-PIB security attribute macAutoRequestKeyIdMode.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
 * \return  Status of operation.
 */
INLINE MAC_Status_t macSecurityPibSetAutoRequestKeyIdModeZBPRO(const MAC_KeyIdMode_t newValue)
{
    if (newValue > MAC_KEY_ID_MODE__EXPLICIT_8B)
        return MAC_INVALID_PARAMETER;

    macAutoRequestIdMode = newValue;
    return MAC_SUCCESS;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Returns value of the MAC-PIB security attribute macAutoRequestKeySource.
 * \return  Value of the requested MAC-PIB attribute.
 */
INLINE MAC_KeySource_t macSecurityPibGetAutoRequestKeySourceZBPRO(void)
{
    return macAutoRequestKeySource;
}

/**//**
 * \brief   Assigns a new value to the MAC-PIB security attribute macAutoRequestKeySource.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
 */
INLINE void macSecurityPibSetAutoRequestKeySourceZBPRO(const MAC_KeySource_t newValue)
{
    macAutoRequestKeySource = newValue;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Returns value of the MAC-PIB security attribute macAutoRequestKeyIndex.
 * \return  Value of the requested MAC-PIB attribute.
 */
INLINE MAC_KeyIndex_t macSecurityPibGetAutoRequestKeyIndexZBPRO(void)
{
    return macAutoRequestKeyIndex;
}

/**//**
 * \brief   Assigns a new value to the MAC-PIB security attribute macAutoRequestKeyIndex.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
 * \return  Status of operation.
 */
INLINE MAC_Status_t macSecurityPibSetAutoRequestKeyIndexZBPRO(const MAC_KeyIndex_t newValue)
{
    if (newValue == 0x00)
            return MAC_INVALID_PARAMETER;

    macAutoRequestKeyIndex = newValue;
    return MAC_SUCCESS;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Returns value of the MAC-PIB security attribute macDefaultKeySource.
 * \return  Value of the requested MAC-PIB attribute.
 */
INLINE MAC_KeySource_t macSecurityPibGetDefaultKeySourceZBPRO(void)
{
    return macDefaultKeySource;
}

/**//**
 * \brief   Assigns a new value to the MAC-PIB security attribute macDefaultKeySource.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
 */
INLINE void macSecurityPibSetDefaultKeySourceZBPRO(const MAC_KeySource_t newValue)
{
    macDefaultKeySource = newValue;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Returns value of the MAC-PIB security attribute macPANCoordExtendedAddress.
 * \return  Value of the requested MAC-PIB attribute.
 */
INLINE MAC_ExtendedAddress_t macSecurityPibGetPANCoordExtendedAddressZBPRO(void)
{
    return macPANCoordExtendedAddress;
}

/**//**
 * \brief   Assigns a new value to the MAC-PIB security attribute macPANCoordExtendedAddress.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
 */
INLINE void macSecurityPibSetPANCoordExtendedAddressZBPRO(const MAC_ExtendedAddress_t newValue)
{
    macPANCoordExtendedAddress = newValue;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Returns value of the MAC-PIB security attribute macPANCoordShortAddress.
 * \return  Value of the requested MAC-PIB attribute.
 */
INLINE MAC_ShortAddress_t macSecurityPibGetPANCoordShortAddressZBPRO(void)
{
    return macPANCoordShortAddress;
}

/**//**
 * \brief   Assigns a new value to the MAC-PIB security attribute macPANCoordShortAddress.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
 */
INLINE void macSecurityPibSetPANCoordShortAddressZBPRO(const MAC_ShortAddress_t newValue)
{
    macPANCoordShortAddress = newValue;
}

/**//**
 * \brief   Assigns a new value to the Frame Counter field of the Device Decriptor in the
 *          Device Table.
 * \param[in]   index         Index over the table attribute.
 * \param[in]   frameCounter  Value to assign.
 */
MAC_Status_t macSecurityPibDeviceDesciptorSetFrameCounter(
                        const MAC_DeviceDescriptorId_t index,
                        const MAC_FrameCounter_t frameCounter);

/**//**
 * \brief   Assigns a new value to the Blacklisted field of the Key Device Decriptor in the
 *          Key Descriptor in the Key Table.
 * \param[in]   index         Index over the Key Table.
 * \param[in]   index         Index over the Key Device List.
 * \param[in]   blacklisted   Value to assign.
 */
MAC_Status_t macSecurityPibKeyDeviceDescriptorSetBlacklisted(
                        const MAC_KeyDescriptorId_t keyDescrId,
                        const MAC_KeyDeviceDescriptorId_t keyDeviceDescrId,
                        const Bool8_t blacklisted);

#endif /* _BB_MAC_SECURITY_PIB_H */
