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

/*******************************************************************************
 *
 * DESCRIPTION:
 *      RF4CE NWK Frames related data.
 *
*******************************************************************************/

#ifndef _RF4CE_FRAME_H
#define _RF4CE_FRAME_H
/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbSysBasics.h"
#include "bbSysMemMan.h"

#include "bbRF4CENWKConstants.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief RF4CE NWK Frame Control definitions.
 */
#define RF4CE_NWK_FRAME_NO_CHANNEL 0x00
#define RF4CE_NWK_FRAME_CHANNEL_15 0x01
#define RF4CE_NWK_FRAME_CHANNEL_20 0x02
#define RF4CE_NWK_FRAME_CHANNEL_25 0x03
#define RF4CE_NWK_FRAME_GET_FRAME_TYPE(control) ((control) & 0x03)
#define RF4CE_NWK_FRAME_SET_FRAME_TYPE(control, value) (((control) & 0xfc) | ((value) & 0x03))
#define RF4CE_NWK_FRAME_GET_SECURITY(control) (((control) >> 2) & 0x01)
#define RF4CE_NWK_FRAME_SET_SECURITY(control, value) (((control) & 0xfb) | ((value) ? 0x04 : 0x00))
#define RF4CE_NWK_FRAME_GET_PROTOCOL(control) (((control) >> 3) & 0x03)
#define RF4CE_NWK_FRAME_SET_PROTOCOL(control, value) (((control) & 0xe7) | (((value) & 0x03) << 3))
#define RF4CE_NWK_FRAME_GET_RESERVED(control) (((control) >> 5) & 0x01)
#define RF4CE_NWK_FRAME_SET_RESERVED(control) ((control) | 0x20)
#define RF4CE_NWK_FRAME_GET_CHANNEL(control) (((control) >> 6) & 0x03)
#define RF4CE_NWK_FRAME_SET_CHANNEL(control, value) (((control) & 0x3f) | (((value) & 0x03) << 6))

typedef enum _RF4CE_NWK_Frame_type_t
{
    RF4CE_NWK_FRAME_RESERVED = 0x00,
    RF4CE_NWK_FRAME_STANDARD,
    RF4CE_NWK_FRAME_COMMAND,
    RF4CE_NWK_FRAME_VENDOR
} RF4CE_NWK_Frame_type_t;

/**//**
 * \brief RF4CE NWK commands supported.
 */
typedef enum _RF4CE_NWK_Commands_t
{
    RF4CE_NWK_CMD_DISCOVERY_REQUEST = 0x01,
    RF4CE_NWK_CMD_DISCOVERY_RESPONSE,
    RF4CE_NWK_CMD_PAIR_REQUEST,
    RF4CE_NWK_CMD_PAIR_RESPONSE,
    RF4CE_NWK_CMD_UNPAIR_REQUEST,
    RF4CE_NWK_CMD_KEY_SEED,
    RF4CE_NWK_CMD_PING_REQUEST,
    RF4CE_NWK_CMD_PING_RESPONSE
} RF4CE_NWK_Commands_t;

/**//**
 * \brief Key seed data size.
  */
#define KEY_SEED_SIZE 5

/************************* TYPES *******************************************************/
/**//**
 * \brief Basic RF4CE NWK data frame header.
 */
typedef struct PACKED _RF4CE_NWK_BasicFrameHeader_t
{
    uint8_t frameControl;     /*!< Frame control. */
    uint32_t frameCounter;    /*!< Frame counter. */
} RF4CE_NWK_BasicFrameHeader_t;

/**//**
 * \brief Command RF4CE NWK data frame header.
 */
typedef struct PACKED _RF4CE_NWK_CommandFrameHeader_t
{
    RF4CE_NWK_BasicFrameHeader_t basicHeader; /*!< Basic header is included. */
    uint8_t command;                          /*!< NWK command identification. */
} RF4CE_NWK_CommandFrameHeader_t;

/**//**
 * \brief Standard RF4CE NWK data frame header.
 */
typedef struct PACKED _RF4CE_NWK_StandardFrameHeader_t
{
    RF4CE_NWK_BasicFrameHeader_t basicHeader; /*!< Basic header is included. */
    uint8_t profileId;                        /*!< Profile identification. */
} RF4CE_NWK_StandardFrameHeader_t;

/**//**
 * \brief Vendor RF4CE NWK data frame header.
 */
typedef struct PACKED _RF4CE_NWK_VendorFrameHeader_t
{
    RF4CE_NWK_StandardFrameHeader_t standardHeader; /*!< Standard header is included. */
    uint16_t vendorId;                       /*!< Vendor identification. */
} RF4CE_NWK_VendorFrameHeader_t;

/**//**
 * \brief Vendor specific data frame body.
  */
typedef struct PACKED _VendorFrameBody_t
{
    uint8_t profileId;                        /*!< Profile identification. */
    uint16_t vendorId;                       /*!< Vendor identification. */
} VendorFrameBody_t;

/**//**
 * \brief Discovery request frame header.
  */
typedef struct PACKED _DiscoveryRequestFrameHeader_t
{
   RF4CE_NWK_CommandFrameHeader_t header; /*!< NWK command header is included. */
   uint8_t nodeCapabilities;              /*!< Node capabilities. */
   uint16_t vendorId;                     /*!< Vendor ID. */
   uint8_t vendorString[RF4CE_NWK_VENDOR_STRING_LENGTH]; /*!< Vendor string. */
   uint8_t applicationCapabilities;       /*!< Application capabilities. */
} DiscoveryRequestFrameHeader_t;

/**//**
 * \brief Discovery request frame body.
  */
typedef struct PACKED _DiscoveryRequestFrameBody_t
{
   uint8_t command;                       /*!< NWK command identification. */
   uint8_t nodeCapabilities;              /*!< Node capabilities. */
   uint16_t vendorId;                     /*!< Vendor ID. */
   uint8_t vendorString[RF4CE_NWK_VENDOR_STRING_LENGTH]; /*!< Vendor string. */
   uint8_t applicationCapabilities;       /*!< Application capabilities. */
} DiscoveryRequestFrameBody_t;

/**//**
 * \brief Discovery request frame footer.
  */
typedef struct PACKED _DiscoveryRequestFrameFooter_t
{
    uint8_t requestedDeviceType;          /*!< Discovery requested device type. Can be 0xff as a wildcard. */
} DiscoveryRequestFrameFooter_t;

/**//**
 * \brief Discovery response frame header.
  */
typedef struct PACKED _DiscoveryResponseFrameHeader_t
{
   RF4CE_NWK_CommandFrameHeader_t header; /*!< NWK command header is included. */
   uint8_t status;                        /*!< Node status. */
   uint8_t nodeCapabilities;              /*!< Node capabilities. */
   uint16_t vendorId;                     /*!< Vendor ID. */
   uint8_t vendorString[RF4CE_NWK_VENDOR_STRING_LENGTH]; /*!< Vendor string. */
   uint8_t applicationCapabilities;       /*!< Application capabilities. */
} DiscoveryResponseFrameHeader_t;

/**//**
 * \brief Discovery response frame body.
  */
typedef struct PACKED _DiscoveryResponseFrameBody_t
{
   uint8_t command;                       /*!< NWK command identification. */
   uint8_t status;                        /*!< Node status. */
   uint8_t nodeCapabilities;              /*!< Node capabilities. */
   uint16_t vendorId;                     /*!< Vendor ID. */
   uint8_t vendorString[RF4CE_NWK_VENDOR_STRING_LENGTH]; /*!< Vendor string. */
   uint8_t applicationCapabilities;       /*!< Application capabilities. */
} DiscoveryResponseFrameBody_t;

/**//**
 * \brief Discovery response frame footer.
  */
typedef struct PACKED _DiscoveryResponseFrameFooter_t
{
    uint8_t discoveryRequestLQI;          /*!< Discovery request registered LQI. */
} DiscoveryResponseFrameFooter_t;

/**//**
 * \brief Pair request frame header.
  */
typedef struct PACKED _PairRequestFrameHeader_t
{
   RF4CE_NWK_CommandFrameHeader_t header; /*!< NWK command header is included. */
   MAC_Addr16bit_t shortAddress;          /*!< Short address. For controller must be 0xfffe */
   uint8_t nodeCapabilities;              /*!< Node capabilities. */
   uint16_t vendorId;                     /*!< Vendor ID. */
   uint8_t vendorString[RF4CE_NWK_VENDOR_STRING_LENGTH]; /*!< Vendor string. */
   uint8_t applicationCapabilities;       /*!< Application capabilities. */
} PairRequestFrameHeader_t;

/**//**
 * \brief Pair request frame body.
  */
typedef struct PACKED _PairRequestFrameBody_t
{
   uint8_t command;                       /*!< NWK command identification. */
   MAC_Addr16bit_t shortAddress;          /*!< Short address. For controller must be 0xfffe */
   uint8_t nodeCapabilities;              /*!< Node capabilities. */
   uint16_t vendorId;                     /*!< Vendor ID. */
   uint8_t vendorString[RF4CE_NWK_VENDOR_STRING_LENGTH]; /*!< Vendor string. */
   uint8_t applicationCapabilities;       /*!< Application capabilities. */
} PairRequestFrameBody_t;

/**//**
 * \brief Pair request frame footer.
  */
typedef struct PACKED _PairRequestFrameFooter_t
{
   uint8_t keyExchangeTransferCount;     /*!< Count of expected key transfer commands. */
} PairRequestFrameFooter_t;

/**//**
 * \brief Pair response frame header.
  */
typedef struct PACKED _PairResponseFrameHeader_t
{
   RF4CE_NWK_CommandFrameHeader_t header; /*!< NWK command header is included. */
   uint8_t status;                        /*!< Status of the corresponding request */
   MAC_Addr16bit_t allocatedShortAddress; /*!< Allocated by the issuer short address for the node */
   MAC_Addr16bit_t shortAddress;          /*!< Short address of the issuer. */
   uint8_t nodeCapabilities;              /*!< Node capabilities. */
   uint16_t vendorId;                     /*!< Vendor ID. */
   uint8_t vendorString[RF4CE_NWK_VENDOR_STRING_LENGTH]; /*!< Vendor string. */
   uint8_t applicationCapabilities;       /*!< Application capabilities. */
} PairResponseFrameHeader_t;

/**//**
 * \brief Pair response frame body.
  */
typedef struct PACKED _PairResponseFrameBody_t
{
   uint8_t command;                       /*!< NWK command identification. */
   uint8_t status;                        /*!< Status of the corresponding request */
   MAC_Addr16bit_t allocatedShortAddress; /*!< Allocated by the issuer short address for the node */
   MAC_Addr16bit_t shortAddress;          /*!< Short address of the issuer. */
   uint8_t nodeCapabilities;              /*!< Node capabilities. */
   uint16_t vendorId;                     /*!< Vendor ID. */
   uint8_t vendorString[RF4CE_NWK_VENDOR_STRING_LENGTH]; /*!< Vendor string. */
   uint8_t applicationCapabilities;       /*!< Application capabilities. */
} PairResponseFrameBody_t;

/**//**
 * \brief Key seed frame.
  */
typedef struct PACKED _KeySeedFrame_t
{
   RF4CE_NWK_CommandFrameHeader_t header; /*!< NWK command header is included. */
   uint8_t sequenceNumber;                /*!< Key seed sequence number */
   SecurityKey_t seedData[KEY_SEED_SIZE]; /*!< Key seed data */
} KeySeedFrame_t;

/**//**
 * \brief Key seed frame body.
  */
typedef struct PACKED _KeySeedFrameBody_t
{
   uint8_t command;                       /*!< NWK command identification. */
   uint8_t sequenceNumber;                /*!< Key seed sequence number */
   SecurityKey_t seedData[KEY_SEED_SIZE]; /*!< Key seed data */
} KeySeedFrameBody_t;


/**//**
 * \brief Ping request/response frame header.
  */
typedef struct PACKED _PingRequestResponseFrameHeader_t
{
   RF4CE_NWK_CommandFrameHeader_t header; /*!< NWK command header is included. */
   uint8_t pingOptions;                   /*!< Ping options */
} PingRequestFrameHeader_t, PingResponseFrameHeader_t;

/**//**
 * \brief Ping request/response frame body.
  */
typedef struct PACKED _PingRequestResponseFrameBody_t
{
   uint8_t command;                       /*!< NWK command identification. */
   uint8_t pingOptions;                   /*!< Ping options */
} PingRequestFrameBody_t, PingResponseFrameBody_t;

/**//**
 * \brief Ping response frame footer.
  */
typedef struct PACKED _EncryptedFrameFooter_t
{
   uint32_t nfr;                  /*!< Ping NFR fields */
} EncryptedFrameFooter_t;

#endif /* _RF4CE_FRAME_H */

/* eof bbRF4CEFrame.h */