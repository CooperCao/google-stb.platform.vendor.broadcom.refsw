//****************************************************************************
//
// Copyright (c) 2004-2009 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.  
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//****************************************************************************
//    Filename: EnetPkt.h
//    Author:   Kevin O'Neal (adapted by Pinar Taskiran)
//    Creation Date: 6-Dec-99
//
//**************************************************************************
//    Description:
//
//      Cablemodem V2 code
//		
//    Code for extracting stuff from ethernet packets
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#if ( (BFC) && !(BCM_ESTB_STANDALONE_APP_SUPPORT) )
// If this is a BFC build, just include BfcHal.h, where all this is 
// now defined.
#include "BfcHal.h"
#else

#ifndef ENET_PKT_H
#define ENET_PKT_H
#include <bcmtypes.h>

// Ethernet frame types:
#define UNKNOWN_FRAME         0
#define ETHERNET_II_FRAME     1
#define RAW_802_3_FRAME       2
#define IEEE_802_2_FRAME      3
#define SNAP_802_2_FRAME      4

// All frame types:
#define ENET_DST_ADDR_OFFSET   0
#define ENET_SRC_ADDR_OFFSET   6
#define ENET_TYPELEN_OFFSET    12

// 802.2 frames:
#define ENET_DSAP_OFFSET       14
#define ENET_SSAP_OFFSET       15
#define ENET_CTRL_OFFSET       16

// SNAP encapsulated 802.2 frames:
#define ENET_SNAP_ID_OFFSET    17

#define ENET_PROTO_TYPE_ETHERTYPE  1
#define ENET_PROTO_TYPE_DSAP       2

// Storage for various IP Header fields.  
typedef struct
{ 
    uint8 fIpVersion;
    unsigned int fIpHeaderLengthBytes;
    unsigned int fIpTotalLengthBytes;
	uint8 fIpFlags;
	uint16 fIpFragmentOffset;
	uint16 fIpIdentification;
    uint8 fIpTypeOfService;
    uint8 fIpProtocolType;

    uint32 fSourceIpAddress;
    uint32 fDestIpAddress;

    // Storage for TCP/UDP port numbers.  
    uint16 fSourceTcpUdpPort;
    uint16 fDestTcpUdpPort;
} IpHeaderFields;

#ifndef   CBLMODEM_H
typedef struct _CM_HAL_PHS_INFO
{
  // SW PHS support only:
  void *pPreBuffer;           // Buffer containing the unsuppressed portion of the header.
  uint16 numberOfBytes;       // The number of bytes of data in the pre-buffer.

  uint16 mainBufferOffset;    // The offset into the main buffer where the remainder of the packet starts.
  // The remainder length can be calculated (uiLen = mainBufferOffset).

  // HW PHS support only:
  byte hwPhsIndex;           // if > 0 specifies the phsIndex, if HW PHS is supported, otherwise = 0

} CM_HAL_PHS_INFO;

typedef struct _DATA_BUFFER_
{

  PVOID                   pvStart;         // Pointer to the packet's data buffer.
  UINT                    uiLen;           // The number of bytes of data in the buffer.

  CM_HAL_PHS_INFO         cmHalPhsInfo;    // NOTE:  Only for use by CM HAL (for PHS support)

  UINT                    ipsecOffset;     // Used by IPSEC to hold the offset needed to free the data

  UINT                    WiFi80211PsInfo;    //Used by 802.11 driver when enqueueing power save data.

  //Removed temporarily by MSN.
  //PVOID                   Cm80211Link;		//Used by 802.11 driver to save PETHERNET link.

  // For buffer chaining; this is no longer supported in most HALs, so it
  // should not be used.  We may remove this member in the future.
  struct _DATA_BUFFER_    *pNext;

} DATA_BUFFER, *PDATA_BUFFER;

/*******************************************************************************
*    DOCSIS Extended Header Packet structure.
*    (note: may contain multiple DOCSIS Ehdrs, uiLen represents total EHDR length)
*/
typedef struct _DOCSIS_EXTENDED_HEADER_
{
  PBYTE    pbStart;
  UINT     uiLen;
} DOCSIS_EXTENDED_HEADER, *PDOCSIS_EXTENDED_HEADER;

/*******************************************************************************
*    Ethernet packet structure.
*/
typedef struct _ETHERNET_PACKET_
{

  BcmHandle      hPacket;
  PDATA_BUFFER   pDataBufferHead;
  PDATA_BUFFER   pDataBufferTail;
  ULONG          ulCount;                // Packet usage counter
  USHORT         uiInterface;            // interface this packet came from
  USHORT         uiDestination;          // interface this packet is being sent to
  DOCSIS_EXTENDED_HEADER eHdr;           // DOCSIS Extended Header object (used for Downstream)
  struct _ETHERNET_PACKET_ *pNext;       // Reserved field for CM HAL

}    ETHERNET_PACKET, *PETHERNET_PACKET;
#endif

int CompareEnetAddr (unsigned char *Addr1, unsigned char *Addr2);

PUCHAR ExtractEnetHeader (PETHERNET_PACKET pEthPkt);

PUCHAR ExtractEnetSrcAddr (PETHERNET_PACKET pEthPkt);
PUCHAR ExtractEnetDstAddr (PETHERNET_PACKET pEthPkt);
USHORT ExtractEnetTypeLen (PETHERNET_PACKET pEthPkt);

UCHAR ExtractEnetDSAP (PETHERNET_PACKET pEthPkt);
UCHAR ExtractEnetSSAP (PETHERNET_PACKET pEthPkt);
UCHAR ExtractEnetCtrl (PETHERNET_PACKET pEthPkt);

USHORT ExtractEnetSnapType (PETHERNET_PACKET pEthPkt);

int ExtractEnetFrameType (PETHERNET_PACKET pEthPkt);

// Log an ethernet packet to the console.
void LogEnetPacket (PETHERNET_PACKET pEthPkt);

void ExtractIpHeaderFields (PETHERNET_PACKET pEthPkt, IpHeaderFields *pIpHeaderFields);
void SetIpAddress(uint8 *pBytes, uint32 *fU32Rep);
void PrintIpAddress(uint32 fU32Rep);
void GetIpAddress(uint32 fU32Rep, uint8 *ipAddress1, uint8 *ipAddress2, uint8 *ipAddress3, uint8 *ipAddress4);

#endif

#endif // !BFC
