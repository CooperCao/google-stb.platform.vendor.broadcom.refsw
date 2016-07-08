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
* FILENAME: $Workfil$
*
* DESCRIPTION:
*   MAC-SAP security definitions.
*
* $Revision$
* $Date$
*
*****************************************************************************************/

#ifndef _BB_MAC_SAP_SECURITY_H
#define _BB_MAC_SAP_SECURITY_H

/************************* INCLUDES ***********************************************************************************/
#include "bbMacBasics.h"

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \name    Main security data types.
 */
/**@{*/
/**//**
 * \brief   Enumeration for the MAC Security Level parameter.
 * \details This data type is used for MAC-SAP Primitive Parameters, MAC Frame Format, and macAutoRequestSecurityLevel
 *  attribute.
 * \note    Only MCPS-DATA.request and MLME-POLL.request primitives of ZBPRO context of MAC implement MAC Security. All
 *  other primitives of ZBPRO context and all primitives of RF4CE context do not implement MAC Security. By default
 *  other primitives assume the SecurityLevel parameter to have 0x00 'None' value (in particular, the MCPS-DATA.request
 *  of RF4CE context, by default, ignores the SecurityLevel parameter and assumes it to be 0x00 'None').
 * \note    To be compliant with MAC Certification Tests, MAC shall be build with the _MAC_SAP_PROCESS_REDUNDANT_PARAMS_
 *  key that instructs all other primitives both of ZBPRO and RF4CE context to validate the SecurityLevel parameter.
 *  When built with this key, if a primitive, except MCPS-DATA.request and MLME-POLL.request primitives of ZBPRO
 *  context, is called with SecurityLevel different from 0x00 'None' the UNSUPPORTED_SECURITY status is confirmed (or
 *  reported with the MLME-COMM-STATUS.indication for the case of MLME-ASSOCIATION.response and MLME-ORPHAN.response).
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.6.2.2.1, table 95.
 */
typedef enum _MAC_SecurityLevel_t {
    MAC_SECURITY_LEVEL__NONE            = 0x0,      /*!< Security level 'None'. */
    MAC_SECURITY_LEVEL__MIC_32          = 0x1,      /*!< Security level 'MIC-32'. */
    MAC_SECURITY_LEVEL__MIC_64          = 0x2,      /*!< Security level 'MIC-64'. */
    MAC_SECURITY_LEVEL__MIC_128         = 0x3,      /*!< Security level 'MIC-128'. */
    MAC_SECURITY_LEVEL__ENC             = 0x4,      /*!< Security level 'ENC'. */
    MAC_SECURITY_LEVEL__ENC_MIC_32      = 0x5,      /*!< Security level 'ENC-MIC-32'. */
    MAC_SECURITY_LEVEL__ENC_MIC_64      = 0x6,      /*!< Security level 'ENC-MIC-64'. */
    MAC_SECURITY_LEVEL__ENC_MIC_128     = 0x7,      /*!< Security level 'ENC-MIC-128'. */
} MAC_SecurityLevel_t;

/**//**
 * \brief   Enumeration for the MAC Key Identifier Mode parameter.
 * \details This data type is used for MAC-SAP Primitive Parameters, MAC Frame Format, and macAutoRequestKeyIdMode
 *  attribute.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.6.2.2.2, table 96.
 */
typedef enum _MAC_KeyIdMode_t {
    MAC_KEY_ID_MODE__IMPLICIT           = 0x0,      /*!< Key is determined implicitly from the Address. */
    MAC_KEY_ID_MODE__DEFAULT            = 0x1,      /*!< Key is determined from the 1-octet Key Index in conjunction
                                                        with the macDefaultKeySource attribute. */
    MAC_KEY_ID_MODE__EXPLICIT_4B        = 0x2,      /*!< Key is determined explicitly from the 4-octet Key Source and
                                                        the 1-octet Key Index. */
    MAC_KEY_ID_MODE__EXPLICIT_8B        = 0x3,      /*!< Key is determined explicitly from the 8-octet Key Source and
                                                        the 1-octet Key Index. */
} MAC_KeyIdMode_t;

/**//**
 * \brief   Data type for the MAC Key Source parameter.
 * \details This data type is used for:
 *  - representation of the KeySource parameter of different MAC-SAP Primitives,
 *  - representation of MAC Security attributes macAutoRequestKeySource and macDefaultKeySource,
 *  - different internal needs of security procedures.
 *
 * \details This data type is not used for:
 *  - representation of the KeySource field of the MAC Auxiliary Security Header,
 *  - representation of the LookupData field of KeyIdLookupDescriptor included into the MAC Security attribute
 *      macKeyTable.
 *
 * \details The Key Source entity according to the official Standard is a set of 0, 4 or 8 octets. Hence, in this
 *  implementation of the MAC it is introduced as a 64-bit integer. Due to this reason, at least the information about
 *  the size of Key Source is lost when it is represented as fixed width 64-bit integer. And also the endianness must be
 *  treated properly.
 * \details The length of the Key Source is not stored in the Stack and is not transferred with this attribute, but it
 *  may be easily recovered in all cases. It is either a constant or it is defined by the linked Key Id Mode value:
 *  - in the case of MAC-SAP Primitive, the KeySource parameter length is specified by the KeyIdMode parameter that is
 *      provided obligatory in the same set of parameters. For KeyIdMode 0x0 and 0x1, the KeySource length is zero; and
 *      for KeyIdMode 0x2 or 0x3, the KeySource length is 4 or 8 octets respectively,
 *  - in the case of macAutoRequestKeySource attribute, its length is specified by the macAutoRequestKeyIdMode attribute
 *      in the same way as described in the previous item for KeySource and KeyIdMode parameters,
 *  - in the case of macDefaultKeySource attribute, its length is fixed to 8 octets.
 *
 * \details The Key Source, as a 64-bit integer value, when transferred by the higher-level layer to/from the Stack
 *  either within MAC-SAP primitive parameters or as one of security attributes value, shall have the little-endian
 *  representation as for all integer values exchanged with the Stack. When used for Key Id Lookup security procedure,
 *  and when packed into the KeySource field of the MAC Auxiliary Security Header it is converted into the big-endian
 *  format as for all other octet sets. For example, if KeySource is assigned with the integer value 0xFF00000000000033,
 *  if will be matched with the Key LookupData 'FF 00 00 00 00 00 00 03 XX' (where XX is the Key Index), and it will be
 *  (de-)serialized to the MAC Auxiliary Security Header as a subset of octets 'FF 00 00 00 00 00 00 03' (the leftmost
 *  octet is transferred first).
 * \details For the case when the Key Source has 8 octets (according to the Key Id Mode), the whole 64-bit integer value
 *  is used. For the case when the Key Source has 4 octets (again according to the Key Id Mode), all 8 bytes of its
 *  64-bit integer representation are accepted by the Stack, stored in its memory, and reported back as-is when
 *  retrieved, but only the four least-significant bytes of the 64-bit integer are used for the Stack internal needs
 *  while the four most-significant bytes are ignored. And for the case when the Key Source has 0 octets, its integer
 *  value is also accepted, stored and reported as-is as a 64-bit integer, but its fully ignored by the Stack for its
 *  internal needs. For example, if KeyIdMode equals 0x2, and the KeySource is assigned with 0x1122334455667788, the
 *  0x1122334455667788 will be stored in the Stack memory, and reported back when retrieved, but it will be treated as
 *  '55 66 77 88' octet string when used for internal needs.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 7.6.2.4.1, B.2.2.
 */
typedef uint64_t  MAC_KeySource_t;

/**//**
 * \brief   Data type for the MAC Key Index parameter.
 * \details This data type is used for MAC-SAP Primitive Parameters, MAC Frame Format, and macAutoRequestKeyIndex
 *  attribute.
 * \details The Key Index takes values from the range 0x01 to 0xFF.
 * \note    It is the responsibility of the key originator to make sure that the Key Index is different from 0x00.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.6.2.4.2.
 */
typedef uint8_t  MAC_KeyIndex_t;

/**//**
 * \brief   Value 0x00 used instead of the KeyIndex if KeyIndex is omitted. The case of KeyIdMode = 0x00.
 */
#define MAC_KEY_INDEX_OMITTED   (0x00)
/**@}*/

/**//**
 * \name    Data type and constants for Security Frame Counter.
 */
/**@{*/
/**//**
 * \brief   Data type representing security frame counter.
 * \details This data type is used both for outgoing frame counter of this local device, and for incoming frame counters
 *  stored on this device from all linked remote devices. Also used for macFrameCounter attribute.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.6.1, tables 88, 89.
 */
typedef uint32_t  MAC_FrameCounter_t;

/**//**
 * \brief   Constant for expired Security Frame Counter.
 */
#define MAC_FRAME_COUNTER_EXPIRED   (0xFFFFFFFF)
/**@}*/

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \name    Constants defining maximum allowed numbers of entries in corresponding security table-attributes and dynamic
 *  lists.
 */
/**@{*/
/**//**
 * \brief   The maximum allowed number of entries in macKeyTable attribute.
 * \details Value must be in the range from 1 to 65535.
 */
#define MAC_aMaxKeyTableEntries             (8)
SYS_DbgAssertStatic(MAC_aMaxKeyTableEntries >= 1 && MAC_aMaxKeyTableEntries <= 65535);

/**//**
 * \brief   The maximum allowed number of entries in macDeviceTable attribute.
 * \details Value must be in the range from 1 to 65535.
 */
#define MAC_aMaxDeviceTableEntries          (8)
SYS_DbgAssertStatic(MAC_aMaxDeviceTableEntries >= 1 && MAC_aMaxDeviceTableEntries <= 65535);

/**//**
 * \brief   The maximum allowed number of entries in macSecurityLevelTable attribute.
 * \details Value must be in the range from 1 to 11.
 * \note    In the current implementation of MAC only Data frame and Data Request Command frame may be (un-)secured. Due
 *  to this reason the defined constant is set to 2.
 */
#define MAC_aMaxSecurityLevelTableEntries   (2)
SYS_DbgAssertStatic(MAC_aMaxSecurityLevelTableEntries >= 1 && MAC_aMaxSecurityLevelTableEntries <= 11);

/**//**
 * \brief   The maximum allowed number of entries in KeyIdLookupList field of KeyDescriptor of macKeyTable attribute.
 * \details Value must be in the range from 1 to 65535.
 */
#define MAC_aMaxKeyIdLookupListEntries      (8)
SYS_DbgAssertStatic(MAC_aMaxKeyIdLookupListEntries >= 1 && MAC_aMaxKeyIdLookupListEntries <= 65535);

/**//**
 * \brief   The maximum allowed number of entries in KeyDeviceList field of KeyDescriptor of macKeyTable attribute.
 * \details Value must be in the range from 1 to 65535.
 */
#define MAC_aMaxKeyDeviceListEntries        (8)
SYS_DbgAssertStatic(MAC_aMaxKeyDeviceListEntries >= 1 && MAC_aMaxKeyDeviceListEntries <= 65535);

/**//**
 * \brief   The maximum allowed number of entries in KeyUsageList field of KeyDescriptor of macKeyTable attribute.
 * \details Value must be in the range from 1 to 11.
 * \note    In the current implementation of MAC only Data frame and Data Request Command frame may be (un-)secured. Due
 *  to this reason the defined constant is set to 2.
 */
#define MAC_aMaxKeyUsageListEntries         (2)
SYS_DbgAssertStatic(MAC_aMaxKeyUsageListEntries >= 1 && MAC_aMaxKeyUsageListEntries <= 11);
/**@}*/

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \name    Data types for number of entries in security table-attributes and dynamic lists.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.6.1, tables 88, 89.
 */
/**@{*/
/**//**
 * \brief   Data type for number of entries in macKeyTable attribute.
 */
#if (MAC_aMaxKeyTableEntries < 256)
typedef uint8_t     MAC_KeyTableEntries_t;
#else
typedef uint16_t    MAC_KeyTableEntries_t;
SYS_DbgAssertStatic(MAC_aMaxKeyTableEntries <= UINT16_MAX);
#endif

/**//**
 * \brief   Data type for number of entries in macDeviceTable attribute.
 */
#if (MAC_aMaxDeviceTableEntries < 256)
typedef uint8_t     MAC_DeviceTableEntries_t;
#else
typedef uint16_t    MAC_DeviceTableEntries_t;
SYS_DbgAssertStatic(MAC_aMaxDeviceTableEntries <= UINT16_MAX);
#endif

/**//**
 * \brief   Data type for number of entries in macSecurityLevelTable attribute.
 */
typedef uint8_t     MAC_SecurityLevelTableEntries_t;
SYS_DbgAssertStatic(MAC_aMaxSecurityLevelTableEntries <= UINT8_MAX);

/**//**
 * \brief   Data type for number of entries in KeyIdLookupList field of KeyDescriptor of macKeyTable attribute.
 */
#if (MAC_aMaxKeyIdLookupListEntries < 256)
typedef uint8_t     MAC_KeyIdLookupListEntries_t;
#else
typedef uint16_t    MAC_KeyIdLookupListEntries_t;
SYS_DbgAssertStatic(MAC_aMaxKeyIdLookupListEntries <= UINT16_MAX);
#endif

/**//**
 * \brief   Data type for number of entries in KeyDeviceList field of KeyDescriptor of macKeyTable attribute.
 */
#if (MAC_aMaxKeyDeviceListEntries < 256)
typedef uint8_t     MAC_KeyDeviceListEntries_t;
#else
typedef uint16_t    MAC_KeyDeviceListEntries_t;
SYS_DbgAssertStatic(MAC_aMaxKeyDeviceListEntries <= UINT16_MAX);
#endif

/**//**
 * \brief   Data type for number of entries in KeyUsageList field of KeyDescriptor of macKeyTable attribute.
 */
typedef uint8_t     MAC_KeyUsageListEntries_t;
SYS_DbgAssertStatic(MAC_aMaxKeyUsageListEntries <= UINT8_MAX);
/**@}*/

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Data type for the PIBAttributeIndex parameter of MLME-GET/SET primitives.
 * \details Attribute index, in general, allows to iterate up to 65536 table entries.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.6.1, table 88.
 */
typedef uint16_t  MAC_PibAttributeIndex_t;

/**//**
 * \brief   Common structure for security parameters of MAC primitives.
 * \note    It is using for the next primitives:
 *  - MCPS-DATA.request, MCPS-DATA.indication,
 *  - MLME-POLL.request,
 *  - MLME-COMM-STATUS.indication.
 *
 * \details See comments to the MAC_KeySource_t data type definition on how to treat the KeySource value.
 * \details This set of parameters is ignored when the SecurityLevel parameter for is set to 0x00. The KeySource field
 *  is ignored when KeyIdMode is set to 0x0 or 0x1. The KeyIndex field is ignored when KeyIdMode is set to 0x0.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 7.6.2.4.
*/
typedef struct _MAC_SecurityParams_t {
    /* 64-bit data. */
    MAC_KeySource_t     keySource;      /*!< The originator of the key to be used. */
    /* 8-bit data. */
    MAC_KeyIdMode_t     keyIdMode;      /*!< The mode used to identify the key to be used. */
    MAC_KeyIndex_t      keyIndex;       /*!< The index of the key to be used. */
} MAC_SecurityParams_t;

#endif /* _BB_MAC_SAP_SECURITY_H */
