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
*   MAC Security Procedures and Security PIB internals definitions.
*
* $Revision$
* $Date$
*
*****************************************************************************************/

#ifndef _BB_MAC_SECURITY_DEFS_H
#define _BB_MAC_SECURITY_DEFS_H

/************************* INCLUDES ***********************************************************************************/
#include "private/bbMacMpdu.h"

/************************* VALIDATIONS ********************************************************************************/
#if !defined(_MAC_CONTEXT_ZBPRO_)
# error This file requires the MAC Context for ZigBee PRO to be included into the build.
#endif

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \brief   Data type representing security key.
 * \details Security keys have 128-bit (16-bytes) size.
 * \note    Key is stored and transferred in the big-endian format.
 */
typedef uint8_t  MAC_SecurityKey_t[128 / 8];
SYS_DbgAssertStatic(sizeof(MAC_SecurityKey_t) == 16);

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \name    Data types for indices over security table-attributes and dynamic lists.
 * \details Identifiers may take values from 0 to (m-1), where m is the defined value of the corresponding configuration
 *  constant MAC_aMaxXxxxEntries.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.6.1, tables 88, 89.
 */
/**@{*/
typedef MAC_KeyTableEntries_t               MAC_KeyDescriptorId_t;              /*!< Index for macKeyTable. */
typedef MAC_DeviceTableEntries_t            MAC_DeviceDescriptorId_t;           /*!< Index for macDeviceTable. */
typedef MAC_SecurityLevelTableEntries_t     MAC_SecurityLevelDescriptorId_t;    /*!< Index for macSecurityLevelTable. */
typedef MAC_KeyIdLookupListEntries_t        MAC_KeyIdLookupDescriptorId_t;      /*!< Index for KeyIdLookupList. */
typedef MAC_KeyDeviceListEntries_t          MAC_KeyDeviceDescriptorId_t;        /*!< Index for KeyDeviceList. */
typedef MAC_KeyUsageListEntries_t           MAC_KeyUsageDescriptorId_t;         /*!< Index for KeyUsageList. */
/**@}*/

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Structure representing single packed KeyDescriptor item of macKeyTable security attribute.
 * \note    This format does not coincide with the serialized format in which the KeyDescriptor item is transferred over
 *  the external channel (with MLME-GET/SET primitives).
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.6.1, table 89.
 */
typedef struct PACKED _MAC_KeyDescriptor_t {
    MM_Link_t                       keyLink;                    /*!< Packed Block Id of linked KeyIdLookupDescriptor. */
    MM_Link_t                       keyXxxListsLink;            /*!< Packed Block Id of linked KeyUsageList,
                                                                    KeyDeviceList, and KeyIdLookupList. */
    MAC_KeyUsageListEntries_t       keyUsageListEntries;        /*!< The number of entries in KeyUsageList. */
    MAC_KeyDeviceListEntries_t      keyDeviceListEntries;       /*!< The number of entries in KeyDeviceList. */
    MAC_KeyIdLookupListEntries_t    keyIdLookupListEntries;     /*!< The number of entries in KeyIdLookupList. */
} MAC_KeyDescriptor_t;
SYS_DbgAssertStatic(OFFSETOF(MAC_KeyDescriptor_t, keyLink) == 0);
SYS_DbgAssertStatic(OFFSETOF(MAC_KeyDescriptor_t, keyXxxListsLink) == 1 * sizeof(MM_Link_t));
SYS_DbgAssertStatic(OFFSETOF(MAC_KeyDescriptor_t, keyUsageListEntries) == 2 * sizeof(MM_Link_t));
SYS_DbgAssertStatic(OFFSETOF(MAC_KeyDescriptor_t, keyDeviceListEntries) == 2 * sizeof(MM_Link_t) +
        sizeof(MAC_KeyUsageListEntries_t));
SYS_DbgAssertStatic(OFFSETOF(MAC_KeyDescriptor_t, keyIdLookupListEntries) == 2 * sizeof(MM_Link_t) +
        sizeof(MAC_KeyUsageListEntries_t) + sizeof(MAC_KeyDeviceListEntries_t));
SYS_DbgAssertStatic(sizeof(MAC_KeyDescriptor_t) == 2 * sizeof(MM_Link_t) +
        sizeof(MAC_KeyUsageListEntries_t) + sizeof(MAC_KeyDeviceListEntries_t) + sizeof(MAC_KeyIdLookupListEntries_t));
SYS_DbgAssertStatic(sizeof(MAC_KeyDescriptor_t) >= 5 && sizeof(MAC_KeyDescriptor_t) <= 9);

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Enumeration for DeviceLookupSize.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.5.8.2.4.
 */
typedef enum _MAC_DeviceLookupSize_t {
    MAC_DEVICE_LOOKUP_SIZE_4B = 0,                  /*!< DeviceLookupData size is 4 bytes. */
    MAC_DEVICE_LOOKUP_SIZE_8B = 1,                  /*!< DeviceLookupData size is 8 bytes. */
} MAC_DeviceLookupSize_t;
SYS_DbgAssertStatic(sizeof(MAC_DeviceLookupSize_t) == 1);

/**//**
 * \brief   Set of allowed DeviceLookupSize values, in bytes.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.5.8.2.4.
 */
/**@{*/
#define MAC_DEVICE_LOOKUP_SIZE_4B_VALUE     (4)     /*!< DeviceLookupData size is 4 bytes. */
#define MAC_DEVICE_LOOKUP_SIZE_8B_VALUE     (8)     /*!< DeviceLookupData size is 8 bytes. */
/**@}*/

/**//**
 * \brief   Type for the DeviceLookupData.
 * \note    The DeviceLookupData has variable size - either 4 or 8 bytes - depending on the addressing mode (short
 *  address with PAN Id, or extended address).
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.5.8.2.4.
 */
typedef union PACKED _MAC_DeviceLookupData_t {
    struct PACKED {
        MAC_PanId_t             panId;              /*!< The 16-bit PAN identifier of the device. */
        MAC_ShortAddress_t      shortAddress;       /*!< The 16-bit short address of the device. */
    };
    uint32_t                    panShortAddress;    /*!< Combined 32-bit value of ShortAddress and PAN Id. */
    MAC_ExtendedAddress_t       extAddress;         /*!< The 64-bit IEEE extended address of the device. */
} MAC_DeviceLookupData_t;
SYS_DbgAssertStatic(FIELD_SIZE(MAC_DeviceLookupData_t, panId) == 2 && OFFSETOF(MAC_DeviceLookupData_t, panId) == 0);
SYS_DbgAssertStatic(FIELD_SIZE(MAC_DeviceLookupData_t, shortAddress) == 2 &&
        OFFSETOF(MAC_DeviceLookupData_t, shortAddress) == 2);
SYS_DbgAssertStatic(FIELD_SIZE(MAC_DeviceLookupData_t, panShortAddress) == MAC_DEVICE_LOOKUP_SIZE_4B_VALUE &&
        OFFSETOF(MAC_DeviceLookupData_t, panShortAddress) == 0);
SYS_DbgAssertStatic(FIELD_SIZE(MAC_DeviceLookupData_t, extAddress) == MAC_DEVICE_LOOKUP_SIZE_8B_VALUE &&
        OFFSETOF(MAC_DeviceLookupData_t, extAddress) == 0);
SYS_DbgAssertStatic(sizeof(MAC_DeviceLookupData_t) == 8);

/**//**
 * \brief   Structure representing single packed DeviceDescriptor item of macDeviceTable security attribute.
 * \details This format coincides with the serialized format in which the DeviceDescriptor item is transferred over the
 *  external channel (with MLME-GET/SET primitives).
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.6.1, table 93.
 */
typedef struct PACKED _MAC_DeviceDescriptor_t {
    union PACKED {
        struct PACKED {
            MAC_PanId_t         panId;              /*!< The 16-bit PAN identifier of the device. */
            MAC_ShortAddress_t  shortAddress;       /*!< The 16-bit short address of the device. */
        };
        uint32_t                panShortAddress;    /*!< Combined 32-bit value of ShortAddress and PAN Id. */
    };
    MAC_ExtendedAddress_t       extAddress;         /*!< The 64-bit IEEE extended address of the device. */
    MAC_FrameCounter_t          frameCounter;       /*!< The incoming frame counter of the device. */
    Bool8_t                     exempt;             /*!< Indication of whether the device may override the minimum
                                                        security level settings defined in macSecurityLevelTable. */
} MAC_DeviceDescriptor_t;
SYS_DbgAssertStatic(FIELD_SIZE(MAC_DeviceDescriptor_t, panId) == 2 && OFFSETOF(MAC_DeviceDescriptor_t, panId) == 0);
SYS_DbgAssertStatic(FIELD_SIZE(MAC_DeviceDescriptor_t, shortAddress) == 2 &&
        OFFSETOF(MAC_DeviceDescriptor_t, shortAddress) == 2);
SYS_DbgAssertStatic(FIELD_SIZE(MAC_DeviceDescriptor_t, panShortAddress) == MAC_DEVICE_LOOKUP_SIZE_4B_VALUE &&
        OFFSETOF(MAC_DeviceDescriptor_t, panShortAddress) == 0);
SYS_DbgAssertStatic(FIELD_SIZE(MAC_DeviceDescriptor_t, extAddress) == MAC_DEVICE_LOOKUP_SIZE_8B_VALUE &&
        OFFSETOF(MAC_DeviceDescriptor_t, extAddress) == 4);
SYS_DbgAssertStatic(FIELD_SIZE(MAC_DeviceDescriptor_t, frameCounter) == 4 &&
        OFFSETOF(MAC_DeviceDescriptor_t, frameCounter) == 12);
SYS_DbgAssertStatic(FIELD_SIZE(MAC_DeviceDescriptor_t, exempt) == 1 && OFFSETOF(MAC_DeviceDescriptor_t, exempt) == 16);
SYS_DbgAssertStatic(sizeof(MAC_DeviceDescriptor_t) == 17);

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Structure representing single packed SecurityLevelDescriptor item of macSecurityLevelTable security
 *  attribute.
 * \details This format coincides with the serialized format in which the SecurityLevelDescriptor item is transferred
 *  over the external channel (with MLME-GET/SET primitives).
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.6.1, tables 79, 82, 92.
 */
typedef struct PACKED _MAC_SecurityLevelDescriptor_t {
    union PACKED {
        struct PACKED {
            MacMpduFrameType_t  frameType           : 3;    /*!< Frame Type identifier. */
            MacMpduCommandId_t  commandFrameId      : 5;    /*!< Command frame identifier. */
        };
        MacMpduSurrId_t         mpduSurrId;                 /*!< Combined 8-bit value of Frame Type and Command Id. */
    };
    MAC_SecurityLevel_t         securityMinimum     : 3;    /*!< The minimal required/expected security level for
                                                                incoming MAC frames having Frame Type and Command Id. */
    uint8_t                     reserved            : 4;    /*!< Reserved. */
    Bool8_t                     deviceOverrideSecurityMinimum : 1;      /*!< Indication of whether originating devices
                                                                            for which the Exempt flag is set may
                                                                            override the minimum security level. */
} MAC_SecurityLevelDescriptor_t;
SYS_DbgAssertStatic(FIELD_SIZE(MAC_SecurityLevelDescriptor_t, mpduSurrId) == 1 &&
        OFFSETOF(MAC_SecurityLevelDescriptor_t, mpduSurrId) == 0);
SYS_DbgAssertStatic(sizeof(MAC_SecurityLevelDescriptor_t) == 2);

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Enumeration for the KeyLookupSize.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclauses 7.5.8.2.4.
 */
typedef enum _MAC_KeyLookupSize_t
{
    MAC_KEY_LOOKUP_SIZE_5B = 0,                 /*!< KeyLookupData size is 5 bytes. */
    MAC_KEY_LOOKUP_SIZE_9B = 1,                 /*!< KeyLookupData size is 9 bytes. */
} MAC_KeyLookupSize_t;
SYS_DbgAssertStatic(sizeof(MAC_KeyLookupSize_t) == 1);

/**//**
 * \brief   Set of allowed KeyLookupSize values, in bytes.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.5.8.2.4.
 */
/**@{*/
#define MAC_KEY_LOOKUP_SIZE_5B_VALUE    (5)     /*!< KeyLookupData size is 5 bytes. */
#define MAC_KEY_LOOKUP_SIZE_9B_VALUE    (9)     /*!< KeyLookupData size is 9 bytes. */
/**@}*/

/**//**
 * \brief   Data type representing Key Lookup Data.
 * \details Key Lookup Data has either 9 or 5 bytes size.
 */
typedef uint8_t  MAC_KeyLookupData_t[MAC_KEY_LOOKUP_SIZE_9B_VALUE];
SYS_DbgAssertStatic(sizeof(MAC_KeyLookupData_t) == 9);

/**//**
 * \brief   Structure representing single packed KeyIdLookupDescriptor item of KeyIdLookupList.
 * \note    This structure describes the generalized format of KeyIdLookupDescriptor item. Indeed the lookupData field
 *  may have either 5 or 9 bytes size depending on the value of the lookupDataSize field. Formats of external
 *  transferring and internal storing coincide with each other.
 * \note    The lookupData value is stored and transferred in the big-endian format.
 * \note    This structure may be used when need to allocate a buffer for KeyIdLookupDescriptor item. Hence, during
 *  (de-)serialization actual size of the lookupData field must be taken.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.6.1, table 94.
 */
typedef struct PACKED _MAC_KeyIdLookupDescriptor_t {
    MAC_KeyLookupSize_t     lookupDataSize;     /*!< A value of 0x00 indicates a set of 5 octets; a value of 0x01
                                                    indicates a set of 9 octets. */
    MAC_KeyLookupData_t     lookupData;         /*!< Data used to identify the key. Either 9 or 5 least significant
                                                    bytes. */
} MAC_KeyIdLookupDescriptor_t;
SYS_DbgAssertStatic(FIELD_SIZE(MAC_KeyIdLookupDescriptor_t, lookupDataSize) == 1 &&
        OFFSETOF(MAC_KeyIdLookupDescriptor_t, lookupDataSize) == 0);
SYS_DbgAssertStatic(FIELD_SIZE(MAC_KeyIdLookupDescriptor_t, lookupData) == MAC_KEY_LOOKUP_SIZE_9B_VALUE &&
        OFFSETOF(MAC_KeyIdLookupDescriptor_t, lookupData) == 1);
SYS_DbgAssertStatic(sizeof(MAC_KeyIdLookupDescriptor_t) == 10);


/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Structure representing single packed KeyDeviceDescriptor item of KeyDeviceList.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.6.1, table 91.
 */
typedef struct PACKED _MAC_KeyDeviceDescriptor_t {
    MAC_DeviceDescriptorId_t    deviceDescriptorHandle;     /*!< Handle to the DeviceDescriptor corresponding to the
                                                                device. */
    union PACKED {
        struct PACKED {
            Bool8_t     uniqueDevice    : 1;    /*!< Indication of whether the device indicated by
                                                    DeviceDescriptorHandle is uniquely associated with the
                                                    KeyDescriptor, i.e., it is a link key as opposed to a group key. */
            Bool8_t     blacklisted     : 1;    /*!< Indication of whether the device indicated by
                                                    DeviceDescriptorHandle previously communicated with this key prior
                                                    to the exhaustion of the frame counter. */
            uint8_t     reserved        : 6;    /*!< Reserved. */
        };
        uint8_t         plain;                  /*!< Flattened value of UniqueDevice and Blacklisted. */
    };
} MAC_KeyDeviceDescriptor_t;
SYS_DbgAssertStatic(FIELD_SIZE(MAC_KeyDeviceDescriptor_t, deviceDescriptorHandle) == sizeof(MAC_DeviceDescriptorId_t) &&
        OFFSETOF(MAC_KeyDeviceDescriptor_t, deviceDescriptorHandle) == 0);
SYS_DbgAssertStatic(FIELD_SIZE(MAC_KeyDeviceDescriptor_t, plain) == 1 &&
        OFFSETOF(MAC_KeyDeviceDescriptor_t, plain) == sizeof(MAC_DeviceDescriptorId_t));
#if (MAC_aMaxDeviceTableEntries < 256)
SYS_DbgAssertStatic(sizeof(MAC_KeyDeviceDescriptor_t) == 2);
#else
SYS_DbgAssertStatic(sizeof(MAC_KeyDeviceDescriptor_t) == 3);
#endif

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Structure representing single packed KeyUsageDescriptor item of KeyUsageList.
 * \details This format coincides with the serialized format in which the SecurityLevelDescriptor item is transferred
 *  over the external channel (with MLME-GET/SET primitives).
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.6.1, tables 79, 82, 90.
 */
typedef struct PACKED _MAC_KeyUsageDescriptor_t {
    union PACKED {
        struct PACKED {
            MacMpduFrameType_t  frameType       : 3;    /*!< Frame Type identifier. */
            MacMpduCommandId_t  commandFrameId  : 5;    /*!< Command frame identifier. */
        };
        MacMpduSurrId_t         mpduSurrId;             /*!< Combined 8-bit value of Frame Type and Command Id. */
    };
} MAC_KeyUsageDescriptor_t;
SYS_DbgAssertStatic(FIELD_SIZE(MAC_KeyUsageDescriptor_t, mpduSurrId) == 1 &&
        OFFSETOF(MAC_KeyUsageDescriptor_t, mpduSurrId) == 0);
SYS_DbgAssertStatic(sizeof(MAC_KeyUsageDescriptor_t) == 1);

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \name    Parameters of generic CCM* mode of operation used by MAC.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause B.3.2
 */
/**@{*/
#define MAC_CCM_L           (2)                     /*!< Value of the L. */
#define MAC_CCM_N_SIZE      (15 - MAC_CCM_L)        /*!< Size of the Nonce (N). */
/**@}*/


/**//**
 * \brief   Data type representing CCM* Nonce.
 * \details CCM* Nonce has 13-bytes size.
 *  See IEEE 802.15.4-2006, subclause 7.6.3.2.
 */
typedef uint8_t  MAC_SecurityNonce_t[MAC_CCM_N_SIZE];
SYS_DbgAssertStatic(sizeof(MAC_SecurityNonce_t) == 13);

#endif /* _BB_MAC_SECURITY_DEFS_H */
