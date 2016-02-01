/***************************************************************************
 *     (c)2007-2014 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/
#ifndef NEXUS_HDMI_INPUT_INFO_H__
#define NEXUS_HDMI_INPUT_INFO_H__

#include "bstd.h"
#include "nexus_hdmi_types.h"

/**
All the data structures in this header file are meant to pass-through low-level
data from the HDMI Rx to the application.
See magnum/commonutils/avc/CHIP/bavc_hdmi.h for the underlaying data structures and field-level documentation.
In order to preserve a simple pass-through, fields will be added and removed from these structures
to continue to match the underlying software layers.
**/

#ifdef __cplusplus
extern "C"
{
#endif

/**
Summary:
Type of packets that can be requested with NEXUS_HdmiInput_GetRawPacketData
**/
typedef enum NEXUS_HdmiInputPacketType
{
    NEXUS_HdmiInputPacketType_eNull                   = 0x00,
    NEXUS_HdmiInputPacketType_eAudioClockRegeneration = 0x01,
    NEXUS_HdmiInputPacketType_eAudioSample            = 0x02,
    NEXUS_HdmiInputPacketType_eGeneralControl         = 0x03,
    NEXUS_HdmiInputPacketType_eAudioContentProtection = 0x04,
    NEXUS_HdmiInputPacketType_eISRC1                  = 0x05,
    NEXUS_HdmiInputPacketType_eISRC2                  = 0x06,
    NEXUS_HdmiInputPacketType_eDirectStream           = 0x07,
    NEXUS_HdmiInputPacketType_eGamutMetadataPacket    = 0xA, /* HDMI 1.3 only */
    NEXUS_HdmiInputPacketType_eVendorSpecific         = 0x81,
    NEXUS_HdmiInputPacketType_eAviInfoFrame           = 0x82,
    NEXUS_HdmiInputPacketType_eSpdInfoFrame           = 0x83,
    NEXUS_HdmiInputPacketType_eAudioInfoFrame         = 0x84,
    NEXUS_HdmiInputPacketType_eMpegInfoFrame          = 0x85,
    NEXUS_HdmiInputPacketType_eUnused                 = 0xA0
    /* No eMax because these values are not sequential. */
} NEXUS_HdmiInputPacketType;

#define NEXUS_HDMI_INPUT_PB_LENGTH 28


typedef enum NEXUS_HdmiGamutFormat
{
    NEXUS_HdmiGamutFormat_eVerticesFacets,
    NEXUS_HdmiGamutFormat_eRange
} NEXUS_HdmiGamutFormat;


typedef enum NEXUS_HdmiGamutColorPrecision
{
    NEXUS_HdmiGamutColorPrecision_e8Bit,
    NEXUS_HdmiGamutColorPrecision_e10Bit,
    NEXUS_HdmiGamutColorPrecision_e12Bit,
    NEXUS_HdmiGamutColorPrecision_eUnknown
} NEXUS_HdmiGamutColorPrecision;


typedef enum NEXUS_HdmiGamutColorSpace
{
    NEXUS_HdmiGamutColorSpace_eItu709RGB,
    NEXUS_HdmiGamutColorSpace_exvYCC601SD,
    NEXUS_HdmiGamutColorSpace_exvYCC709HD,
    NEXUS_HdmiGamutColorSpace_eXZY /* not supported */
} NEXUS_HdmiGamutColorSpace;


typedef struct NEXUS_HdmiGcpData
{
    uint8_t PixelPacking;
    uint8_t ColorDepth;
    uint8_t DefaultPhase;
    NEXUS_HdmiPacketStatus packetStatus;
    NEXUS_HdmiPacket packet;
} NEXUS_HdmiGcpData;

typedef struct NEXUS_HdmiGamutPacket
{
    uint8_t type;
    uint8_t nextField;
    uint8_t profile;
    uint8_t affectedGamutSeqNumber;
    uint8_t noCurrentGamut;
    uint8_t packetSeq;
    uint8_t currentGamutSeqNumber;
    uint8_t dataBytes[NEXUS_HDMI_INPUT_PB_LENGTH];

    NEXUS_HdmiGamutFormat format;
    bool facets;
    NEXUS_HdmiGamutColorPrecision colorPrecision;
    NEXUS_HdmiGamutColorSpace colorSpace;
    uint16_t numberVertices;
    uint16_t numberFacets;

    /* for Gamut Vertice Data */
    uint16_t y[4];
    uint16_t cb[4];
    uint16_t cr[4];

    /* for Gamut Range Data */
    uint16_t minRedData;
    uint16_t maxRedData;
    uint16_t minGreenData;
    uint16_t maxGreenData;
    uint16_t minBlueData;
    uint16_t maxBlueData;

    NEXUS_HdmiPacketStatus packetStatus;

} NEXUS_HdmiGamutPacket;

typedef struct NEXUS_HdmiAudioContentProtection
{
    uint8_t   acpType;
    char      acpTypeDependent[NEXUS_HDMI_INPUT_PB_LENGTH];
    NEXUS_HdmiPacketStatus packetStatus;
    NEXUS_HdmiPacket packet;
} NEXUS_HdmiAudioContentProtection;

typedef struct NEXUS_HdmiAudioClockRegeneration
{
    uint32_t cts;
    uint32_t n;
    NEXUS_HdmiPacketStatus packetStatus;
    NEXUS_HdmiPacket packet;
} NEXUS_HdmiAudioClockRegeneration;

typedef struct NEXUS_HdmiGeneralControlPacket
{
    uint32_t muteStatus;
    uint32_t setAvMute;
    uint32_t clearAvMute;
    uint32_t pixelPacking;
    uint32_t colorDepth;
    uint32_t defaultPhase;
    NEXUS_HdmiPacketStatus packetStatus;
    NEXUS_HdmiPacket packet;
} NEXUS_HdmiGeneralControlPacket;


#define NEXUS_HDMI_ISRC_PACKET_BYTES 16
#define NEXUS_HDMI_IRSC2_PACKET_OFFSET 16
#define NEXUS_HDMI_ISRC_LENGTH 32
/***************************************************************************
Summary:
Structure containing the contents of ISRC Packet.  The contents of the ISRC packet *can*
actually be sent over two different packets (ISRC1 and ISRC2)
****************************************************************************/
typedef struct NEXUS_HdmiISRC
{
    bool  ISRC1_PacketReceived ;
    

    /* fields from ISRC Packet */
    bool  ISRC_Cont ;  /* ISRC data continued in next packet (ISRC2) */
    
    /* data located in ISRC_Status field and ISRC_UPC_EAN_xx fields are valid */
    bool  ISRC_Valid ; 
    uint8_t ISRC_Status ;
    /* contains concatenated data bytes from both ISRC1 and ISRC2 */
    uint8_t ISRC_UPC_EAN[NEXUS_HDMI_ISRC_LENGTH];  
    
    /* original raw packet data */
    NEXUS_HdmiPacket ISRC1_Packet ;
    NEXUS_HdmiPacket ISRC2_Packet ;
    
    NEXUS_HdmiPacketStatus packetStatus ;
} NEXUS_HdmiISRC ;




#ifdef __cplusplus
}
#endif

#endif
