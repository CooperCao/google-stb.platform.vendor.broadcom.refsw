/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 ******************************************************************************/
//
//****************************************************************************
//
//  Filename:       EthernetPacket.cpp
//  Author:         David Pullen
//  Creation Date:  March 15, 2000
//
//****************************************************************************
//  Description:
//      This is a wrapper class for various forms of ethernet packets.  The
//      underlying data structures come from the HAL (ETHERNET_PACKET).  This
//      class is able to present the fields of the packet in a more usable form,
//      allowing get/set operations on them.
//
//      In general, you will hand an ETHERNET_PACKET to this class, which will
//      parse it into the data members.  Then, you can use the accessors to read
//      the fields out of the packet.
//
//      NOTE:  This class does not have the ability to create its own
//             ETHERNET_PACKET!  You must give it one before you can operate on
//             it!  This is especially important when using operator = ...
//
//      The accessors have both get and set versions.  The "gets" are fast
//      inline methods that simply return references to the data members of this
//      class.  The "sets" are real method calls that not only update the data
//      members, but also write through to the underlying ETHERNET_PACKET
//      memory, and potentially invalidate other members!
//
//      NOTE:  Before calling a "get" accessor, you must call the Parse...()
//             method for that group; e.g. before calling DsapParsed(), you
//             must call ParseLlc().  The Parse...() methods do the work of
//             parsing the packet data and storing them in the class members.
//
//             The "get" accessor names were changed to ensure that the client
//             is aware of this, though there is nothing that enforces that the
//             client has actually called Parse...().
//
//      This class handles all of the buffer chaining aspects, allowing clients
//      to treat the separate physical buffers a a single logical buffer.
//
//      See ftp://ftp.ed.ac.uk/pub/EdLAN/provan-enet and "TCP/IP Illustrated" 
//      by Stevens for more information on Ethernet and IP frame formats.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "EthernetPacket.h"

#include "MessageLog.h"
#include "OctetBuffer.h"

#include "BfcHal.h"

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <iomanip>
#else
#include <iomanip.h>
#endif
#include "sflfind.h"
//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

unsigned long  BcmEthernetPacket::gGlobalBcmPacketId = 0;

//********************** Local Functions *************************************

static uint16 CalculateIpHcs(const uint8 *pIpHeader,
                             unsigned int numberOfBytes);
static uint32 CalculatePseudoHeaderChecksum(const uint8 *pIpHeader,
                                            uint16 lastWord);
static uint32 CalculateIpV6PseudoHeaderChecksum(const uint8 *pIpHeader, 
                                                uint32 upperLayerLength);
static uint16 CalculateUdpChecksum(const uint8 *pIpHeader,
                                   const uint8 *pUdpHeader);

//********************** Class Method Implementations ************************


// Default Constructor.  Sets the ETHERNET_PACKET pointer to NULL, and sets
// all of the members to 0's.
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmEthernetPacket::BcmEthernetPacket(void)
{
    pfEthernetPacket = NULL;
    fBcmPacketId = 0;

    // PR11290 set the buffer pointer to NULL.
    pfBuffer = NULL;

    // Prime this; the default MAC DA is all 0's, which is a unicast address,
    // so this is reasonable.
    fMacDaType = kUnicast;

	// Initialize member variables.
	fIpFragmentOffset = 0;
	fSourceTcpUdpPort = 0;
	fDestTcpUdpPort = 0;

    // Just let this do the rest of the work.
    ReleasePacket();
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmEthernetPacket::~BcmEthernetPacket()
{
    // Just let this do the work.
    if (pfEthernetPacket != NULL)
    {
        ReleasePacket();
    }
}


// *************************************************************************
// Note - for cache optimization, these functions have been moved here;
// please don't reorder these functions!
// *************************************************************************
// \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ 

// Causes the fields of the specified packet to be parsed into the data
// members.
//
// If a packet was previously stored, then the pointer to it is returned (we
// don't delete it).  You must then release/free it yourself.
//
// Parameters:
//      pPacket - the packet to be parsed.
//
// Returns:  Nothing.
//
void BcmEthernetPacket::SetPacket(ETHERNET_PACKET *pPacket)
{
    // Store a pointer to the new packet, and pull out the pointer to the data
    // buffer.
    pfEthernetPacket = pPacket;

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;

    fWasParsedMask = 0;

    // If the new packet is NULL, then just treat this like a request to release
    // the packet.
    if (pPacket != NULL)
    {
        // Pull out the pointer to the first data buffer from the enet packet.
        DATA_BUFFER *pDataBuffer = pPacket->pDataBufferHead;

        // PR12290 Now pull out the buffer pointer from the data buffer.
        pfBuffer = (uint8 *) pDataBuffer->pvStart;

        // Now, parse out the MAC DA and SA fields.  The rest will be deferred
        // until they are specifically requested by a client (on-demand parsing).
        fMacDa.Set(pfBuffer);
        fMacSa.Set(pfBuffer + 6);

        // Calculate the total packet size by stepping through each buffer.
        //
        // Buffer chaining not supported any more.
        fPacketLength = pDataBuffer->uiLen;

        // Calculate what type of MAC DA this is.
        fMacDaType = kUnicast;
        if (fMacDa.IsMulticast() == true)
        {
            // The lsb of the first byte is a 1; now I need to differentiate
            // between broadcast (which is a special class of multicast packet)
            // an plain old multicast.
            if (fMacDa == kBroadcastMac)
            {
                fMacDaType = kBroadcast;
            }
            else
            {
                fMacDaType = kMulticast;
            }
        }
    }
}


// These are the implementations for the packet parsing functions.  They are
// called by the public inline methods if necessary.
void BcmEthernetPacket::ParseIpAddrImpl(void)
{
    // Make sure the LLC header is parsed.  If not, then parse it; we need it
    // for some fields.
    ParseLlc();

    // Only do this if this is an IP packet.
    if (IsIpPktParsed())
    {
        // Remember that we have parsed the IP address fields now.
        fWasParsedMask |= kIpAddrWasParsed;

        // For now, only support IPv4.
        //
        // The IP source address is first.
        fSourceIpAddress.Set(pfBuffer + fIpHeaderOffset + 12);

        // Finally, the IP dest address.
        fDestIpAddress.Set(pfBuffer + fIpHeaderOffset + 16);
    }
}

// These are the implementations for the packet parsing functions.  They are
// called by the public inline methods if necessary.
void BcmEthernetPacket::ParseIpV6AddrImpl(void)
{
    // Make sure the LLC header is parsed.  If not, then parse it; we need it
    // for some fields.
    ParseLlc();

    // Only do this if this is an IP packet.
    if (IsIpV6PktParsed())
    {
        // Remember that we have parsed the IP address fields now.
        fWasParsedMask |= kIpV6AddrWasParsed;

        //
        // The IP source address is first.
        fSourceIpAddress.Set(pfBuffer + fIpHeaderOffset + 8, true);

        // Finally, the IP dest address.
        fDestIpAddress.Set(pfBuffer + fIpHeaderOffset + 24, true);
    }
}

void BcmEthernetPacket::ParseLlcImpl(void)
{
    uint8 offset=0;

    // Remember that we have parsed all of the LLC fields now.
    fWasParsedMask |= kLlcWasParsed;

    // Read out the EtherType/Length field, which is the 2 bytes after
    // the MAC SA.

    fEtherType = NetworkToHost(*((uint16 *) (pfBuffer + 12)));
    
    // Check for the existence of a VLAN header
    if (fEtherType == 0x8100)
    {
        offset = 4;
        fEtherType = NetworkToHost(*((uint16 *) (pfBuffer + 12 + offset)));
    }

    // If the type/length field is greater than 1500, then it is an EtherII
    // frame.  Otherwise, we have to keep parsing.
    if (fEtherType > 1500)
    {
        fFrameFormat = kEthernetII;
        fEtherTypeOffset = 12 + offset;

        // The IP Header (if any) starts 2 bytes after the EtherType field.
        fIpHeaderOffset = fEtherTypeOffset + 2;

        // Set the LLC fields to dummy values?  For now, don't do this.
        // The EtherII frame is the most common format, and we want it to
        // be as efficient as possible.  If the client doesn't check for
        // the frame type before reading these values, then they will
        // get bogus data anyway, so it doesn't really matter what bogus
        // data they get.
        fEtherLength = 1501; 
        
        // PR 12181:  Special optimized merged value for LLC filtering.  This 
        // consists of the lower 16 bits being the ethertype OR DSAP and the 
        // upper 16 bits containing a flag to indicate which protocol type is 
        // being used.
        fMergedLlcProtocol = kLlcProtocolHasEthertype | fEtherType;
    }
    else
    {
        // Not EtherII, so it was actually an EtherLength field.
        fEtherLength = fEtherType;

        // Pull out the DSAP, SSAP, and other LLC fields.
        fDsap = pfBuffer[14+offset];
        fSsap = pfBuffer[15+offset];
        fControl = pfBuffer[16+offset];
        fOrgCode[0] = pfBuffer[17+offset];
        fOrgCode[1] = pfBuffer[18+offset];
        fOrgCode[2] = pfBuffer[19+offset];
        
        // Now, see what the frame type really is.
        if ((fDsap == 0xaa) && (fSsap == 0xaa) && (fControl == 0x03))
        {
            // This is a SNAP frame.  The EtherType field is after the ORG
            // CODE.
            fFrameFormat = kSnap;
            fEtherTypeOffset = 20 + offset;

            // The IP Header (if any) starts 2 bytes after the EtherType
            // field.
            fIpHeaderOffset = fEtherTypeOffset + 2;

            // Pull out the EtherType field for real this time.
            fEtherType = NetworkToHost(*((uint16 *) (pfBuffer + 20 + offset)));
            
            // PR 12181:  Special optimized merged value for LLC filtering.  
            // This consists of the lower 16 bits being the ethertype OR DSAP
            // and the upper 16 bits containing a flag to indicate which 
            // protocol type is being used.
            fMergedLlcProtocol = kLlcProtocolHasEthertype | fEtherType;
        }
        else
        {
            // No other parsing can be done.  The remaining frame types
            // don't have an EtherType field, and can't carry IP packets.
            // Ensure that these fields are 0 for other frame types.  This
            // assumption is used by other methods.
            fEtherType = 0;
            fEtherTypeOffset = 0;
            fIpHeaderOffset = 0;

            if ((fDsap == 0xff) && (fSsap == 0xff))
            {
                fFrameFormat = kRaw802_3;

            }
            else
            {
                fFrameFormat = kIeee802_2;
            }
            
            // PR 12181:  Special optimized merged value for LLC filtering.  
            // This consists of the lower 16 bits being the ethertype OR DSAP
            // and the upper 16 bits containing a flag to indicate which 
            // protocol type is being used.
            fMergedLlcProtocol = kLlcProtocolHasDsap | fDsap;
        }
    }
}


void BcmEthernetPacket::ParseIpImpl(void)
{
    // Only do this if this is an IP packet, and we have successfully parsed
    // the IP addresses.
    if (ParseIpAddr())
    {
        // Remember that we have parsed all of the IP fields now.
        fWasParsedMask |= kIpWasParsed;

        // The IP Version and Header length fields are nibbles in the same
        // byte.  Pull them out, convert the length from dwords to bytes, and
        // shift the version out of the most significant bits.
        fIpVersion = pfBuffer[fIpHeaderOffset];

        fIpHeaderLengthBytes = fIpVersion & 0x0f;
        fIpHeaderLengthBytes *= 4;

        fIpVersion >>= 4;

        // Pull out the ToS byte.
        fIpTypeOfService = pfBuffer[fIpHeaderOffset + 1];

        // Pull out the total packet length.
        fIpTotalLengthBytes = NetworkToHost(*((uint16 *) (pfBuffer + fIpHeaderOffset + 2)));
        /*fIpTotalLengthBytes = pfBuffer[fIpHeaderOffset + 2];
        fIpTotalLengthBytes <<= 8;
        fIpTotalLengthBytes |= pfBuffer[fIpHeaderOffset + 3];*/


        // identification
        fIpIdentification = (pfBuffer[fIpHeaderOffset + 4] << 8) | pfBuffer[fIpHeaderOffset + 5];
        // flags
        fIpFlags = (pfBuffer[fIpHeaderOffset + 6] & 0xe0) >> 5;
        // fragment offset - 8-byte units
        fIpFragmentOffset = (((pfBuffer[fIpHeaderOffset + 6] & 0x1f) << 8) | pfBuffer[fIpHeaderOffset + 7])*8;

        // The protocol type is farther in to the packet.  Evaluate it to make
        // checking for TCP/UDP faster.
        fIpProtocolType = pfBuffer[fIpHeaderOffset + 9];

        if ((fIpProtocolType == kTcp) || (fIpProtocolType == kUdp))
        {
            fIpProtocolTypeIsTcpUdp = true;

            // The TCP/UDP header starts after the IP header.
            fTcpUdpHeaderOffset = fIpHeaderOffset + fIpHeaderLengthBytes;
        }
        else
        {
            fIpProtocolTypeIsTcpUdp = false;
            fTcpUdpHeaderOffset = 0;
        }
    }
}

void BcmEthernetPacket::ParseIpV6Impl(void)
{
    // Only do this if this is an IPV6 packet, and we have successfully parsed
    // the IP addresses.
    if (ParseIpV6Addr())
    {
        // Remember that we have parsed all of the IP fields now.
        // Get a pointer to the actual buffer we are parsing.
        uint8 *pBuffer = pfBuffer + fIpHeaderOffset;

        fWasParsedMask |= kIpV6WasParsed;

        // 4-bit Internet Protocol version number = 6.  Upper nibble of the first byte.
        fIpVersion = pBuffer[0];
        fIpVersion >>= 4;

        // pull the 8 bit traffic class out which spans 2 bytes
        fIpV6TrafficClass = (pBuffer[0] << 4) | (pBuffer[1] >> 4);

        // 20-bit flow label.
        fIpV6FlowLabel = pBuffer[1] & 0x0f;
        fIpV6FlowLabel <<= 8;
        fIpV6FlowLabel |= pBuffer[2];
        fIpV6FlowLabel <<= 8;
        fIpV6FlowLabel |= pBuffer[3];

        // 16-bit unsigned integer.  Length of the IPv6 payload, i.e., the rest 
        // of the packet following this IPv6 header, in octets.
        fIpV6PayloadLength = pBuffer[4];
        fIpV6PayloadLength <<= 8;
        fIpV6PayloadLength |= pBuffer[5];

        // Identifies the type of header immediately following the IPv6 header.  
        // Uses the same values as the IPv4 Protocol field 
        fIpV6NextHeader = pBuffer[6];

        // 8-bit unsigned integer.  Decremented by 1 by each node that forwards 
        // the packet. The packet if discarded if Hop Limit is decremented to zero.
        fIpV6HopLimit = pBuffer[7];

        // IPV6 header length is a fixed 40 bytes
        fIpHeaderLengthBytes = 40;

        // The IP source address is first.
        // This is not necessary, it was done by ParseIpV6Addr();
        //fSourceIpAddress.Set(&pBuffer[8], true);

        // Finally, the IP dest address.
        // This is not necessary, it was done by ParseIpV6Addr();
        //fDestIpAddress.Set(&pBuffer[24], true);

        // initialization
        fIpFlags = 0;
        fIpFragmentOffset = 0;
        fIpV6UnfragmentableLastHeaderOffset = 6;
        fIpV6UnfragmentableLengthBytes = fIpHeaderLengthBytes;

        bool HasRoutingHeader = false;
        bool DestOptionOccurBeforeRoutingHdr = false;
        unsigned int IntermediateDestOptionLength = 0;

        // Loop thru and process the extension headers if any
        while(  fIpV6NextHeader == kHopByHopOptionsHdr ||    // hop-by-hop options
                fIpV6NextHeader == kRoutingHdr         ||    // routing
                fIpV6NextHeader == kFragmentHdr        ||    // fragment
                fIpV6NextHeader == kEncapSecPayloadHdr ||    // encapsulating security payload
                fIpV6NextHeader == kAuthenticationHdr  ||    // authentication header
                fIpV6NextHeader == kDestinationOptionsHdr )  // destination options
        {
            if (fIpV6NextHeader == kEncapSecPayloadHdr)
            {
                // Everything is encrypted after this header up to its trailer. so exit now.
                break;
            }

            switch (fIpV6NextHeader)
            {
                case kHopByHopOptionsHdr:
                    fIpV6NextHeader = pBuffer[fIpHeaderLengthBytes];
                    fIpHeaderLengthBytes += (8 + (pBuffer[fIpHeaderLengthBytes + 1] * 8));
                    fIpV6UnfragmentableLastHeaderOffset = fIpV6UnfragmentableLengthBytes;
                    fIpV6UnfragmentableLengthBytes += (8 + (pBuffer[fIpHeaderLengthBytes + 1] * 8));
                    break;

                case kDestinationOptionsHdr:
                    fIpV6NextHeader = pBuffer[fIpHeaderLengthBytes];
                    fIpHeaderLengthBytes += (8 + (pBuffer[fIpHeaderLengthBytes + 1] * 8));
                    if (!HasRoutingHeader)
                    {
                        // Possibly an intermediate destination option hdr if routing occurred after it.
                        // Let store it first for later use if routing header occurs.  If not dont use it.
                        IntermediateDestOptionLength += (8 + (pBuffer[fIpHeaderLengthBytes + 1] * 8));
                        DestOptionOccurBeforeRoutingHdr = true;
                    }
                    break;

                case kRoutingHdr:
                    // Update fIpV6NextHeader and fIpHeaderLengthBytes 
                    HasRoutingHeader = true;
                    fIpV6NextHeader = pBuffer[fIpHeaderLengthBytes];
                    fIpHeaderLengthBytes += (8 + (pBuffer[fIpHeaderLengthBytes + 1] * 8));

                    // if there is(are) destination option header(s) occurred prior to routing header,
                    // they are final dest option header and must be included in unfragmentable part.
                    // Only do this for the first routing hdr.
                    if (DestOptionOccurBeforeRoutingHdr)
                    {
                        fIpV6UnfragmentableLengthBytes += IntermediateDestOptionLength;
                        DestOptionOccurBeforeRoutingHdr = false;
                    }

                    fIpV6UnfragmentableLastHeaderOffset = fIpV6UnfragmentableLengthBytes;
                    fIpV6UnfragmentableLengthBytes += (8 + (pBuffer[fIpHeaderLengthBytes + 1] * 8));
                    break;

                case kFragmentHdr:
                    // Parse fragmentation fields
                    
                    // identification
                    fIpV6Identification  = (pBuffer[fIpHeaderLengthBytes + 4] << 24);
                    fIpV6Identification |= (pBuffer[fIpHeaderLengthBytes + 5] << 16);
                    fIpV6Identification |= (pBuffer[fIpHeaderLengthBytes + 6] << 8);
                    fIpV6Identification |= (pBuffer[fIpHeaderLengthBytes + 7] << 0);
                    // flags
                    fIpFlags = (pBuffer[fIpHeaderLengthBytes + 3] & 0x01);
                    // fragment offset - 8-byte units
                    fIpFragmentOffset  = (pBuffer[fIpHeaderLengthBytes + 2]) << 8;
                    fIpFragmentOffset |= (pBuffer[fIpHeaderLengthBytes + 2]) << 0;
                    fIpFragmentOffset = *((uint16 *)(pBuffer + fIpHeaderLengthBytes + 2)) >> 3;

                    // Update parameters need for reassembly
                    fIpV6FragmentNextHeader   = pBuffer[fIpHeaderLengthBytes];

                    // Update fIpV6NextHeader and fIpHeaderLengthBytes 
                    fIpV6NextHeader = pBuffer[fIpHeaderLengthBytes];
                    fIpHeaderLengthBytes += 8;
                    break;
                
                case kAuthenticationHdr:
                    // Update fIpV6NextHeader and fIpHeaderLengthBytes 
                    fIpV6NextHeader = pBuffer[fIpHeaderLengthBytes];
                    fIpHeaderLengthBytes += (8 + (pBuffer[fIpHeaderLengthBytes + 1] * 4));
                    break;
            }
        }

        // copy the protocol type over to fIpProtocolType so IPv4 and IPv6 can be 
        // treated the same. 
        fIpProtocolType = fIpV6NextHeader;

        // done skipping over extension headers.  fIpV6NextHeader now
        // contains the transport layer protocol type and fIpHeaderLengthBytes
        // reflects the 40 byte IpV6 header plus extension headers. 
        if ((fIpV6NextHeader == kTcp) || (fIpV6NextHeader == kUdp))
        {
            fIpProtocolTypeIsTcpUdp = true;

            // The TCP/UDP header starts after the IP header.
            fTcpUdpHeaderOffset = fIpHeaderOffset + fIpHeaderLengthBytes;
        }
        else
        {
            fIpProtocolTypeIsTcpUdp = false;
            fTcpUdpHeaderOffset = 0;
        }
    }
}

void BcmEthernetPacket::ParseTcpUdpImpl(void)
{
    // Make sure the IP header is parsed.  If not, then parse it; we need it
    // for some fields.  If this isn't an IP packet, then we can bail.
    ParseLlc();

    if (EtherTypeParsed() == kEtherTypeIp)
    {
        if (ParseIp() == false)
        {
            return;
        }
    }
    else if (EtherTypeParsed() == kEtherTypeIpV6)
    {
        if (ParseIpV6() == false)
        {
            return;
        }
    }
    else
    {
        return;
    }

    // Only do this if this is a TCP/UDP packet.
    if (IpProtocolTypeTcpUdpParsed())
    {
        // Remember that we have parsed all of the TCP/UDP fields now.
        fWasParsedMask |= kTcpUdpWasParsed;

		// If the fragment offset is not zero, then this packet
		// is an IP fragment that does not contain a TCP/UDP header.
		if( fIpFragmentOffset != 0 )
		{		
			// Reset the TCP/UDP port member variables since these variables
			// are not initialized previously and the caller of this function might
			// read their values without checking the return value of this function.
			fSourceTcpUdpPort = 0;
			fDestTcpUdpPort = 0;

			return;
		}
	
        // The source port is first.
        fSourceTcpUdpPort = NetworkToHost(*((uint16 *) (pfBuffer + fTcpUdpHeaderOffset)));
        /*fSourceTcpUdpPort = pfBuffer[fTcpUdpHeaderOffset];
        fSourceTcpUdpPort <<= 8;
        fSourceTcpUdpPort |= pfBuffer[fTcpUdpHeaderOffset + 1];*/

        // The dest port is next.
        fDestTcpUdpPort = NetworkToHost(*((uint16 *) (pfBuffer + fTcpUdpHeaderOffset + 2)));
        /*fDestTcpUdpPort = pfBuffer[fTcpUdpHeaderOffset + 2];
        fDestTcpUdpPort <<= 8;
        fDestTcpUdpPort |= pfBuffer[fTcpUdpHeaderOffset + 3];*/
    }
}


void BcmEthernetPacket::ParseTcpImpl(void)
{
    // Make sure the TCP/UDP port numbers are parsed.  If not, then parse them;
    // if this isn't a TCP/IP packet, then we can bail.
    if ((ParseTcpUdp() == false) || (IpProtocolTypeParsed() != kTcp))
    {
        return;
    }

    // Remember that we have parsed all of the TCP fields now.
    fWasParsedMask |= kTcpWasParsed;

    // The sequence number is first.  We manually read this out from network
    // order to host order, because is the guaranteed not to be aligned on a
    // uint32 boundary, and we don't want to take an unaligned memory access
    // exception.
    fTcpSequenceNumber = pfBuffer[fTcpUdpHeaderOffset + 4];
    fTcpSequenceNumber <<= 8;
    fTcpSequenceNumber |= pfBuffer[fTcpUdpHeaderOffset + 5];
    fTcpSequenceNumber <<= 8;
    fTcpSequenceNumber |= pfBuffer[fTcpUdpHeaderOffset + 6];
    fTcpSequenceNumber <<= 8;
    fTcpSequenceNumber |= pfBuffer[fTcpUdpHeaderOffset + 7];

    // The ack number is next.  Again, manually do the net-to-host thing.
    fTcpAckNumber = pfBuffer[fTcpUdpHeaderOffset + 8];
    fTcpAckNumber <<= 8;
    fTcpAckNumber |= pfBuffer[fTcpUdpHeaderOffset + 9];
    fTcpAckNumber <<= 8;
    fTcpAckNumber |= pfBuffer[fTcpUdpHeaderOffset + 10];
    fTcpAckNumber <<= 8;
    fTcpAckNumber |= pfBuffer[fTcpUdpHeaderOffset + 11];

    // Then comes the header length, which we have to shift right 4 bits, then
    // convert to from 32-bit words to bytes.
    fTcpHeaderLengthBytes = *(pfBuffer + fTcpUdpHeaderOffset + 12);
    fTcpHeaderLengthBytes >>= 4;
    fTcpHeaderLengthBytes *= 4;

    // The flags.
    fTcpFlags = *(pfBuffer + fTcpUdpHeaderOffset + 13);

    // Window size.  We can safely use the macro here because this field is
    // pretty much guaranteed to be word aligned.
    fTcpWindowSize = NetworkToHost(*((uint16 *) (pfBuffer + fTcpUdpHeaderOffset + 14)));

    // Checksum.  We're skipping this for now...
    //fTcpChecksum = NetworkToHost(*((uint16 *) (pfBuffer + fTcpUdpHeaderOffset + 16)));

    // Finally the urgent pointer.  Again, this is safe because the field will
    // be word aligned.
    fTcpUrgentPointer = NetworkToHost(*((uint16 *) (pfBuffer + fTcpUdpHeaderOffset + 18)));
}


void BcmEthernetPacket::ParseArpImpl(void)
{
    // Make sure the LLC header is parsed.  If not, then parse it; we need it
    // for some fields.
    ParseLlc();

    // Only do this if this is an ARP packet.
    if (IsArpPktParsed())
    {
        // Calculate where the ARP payload starts.
        unsigned int arpOffset = fEtherTypeOffset + 2;
    
        // Remember that we have parsed all of the ARP fields now.
        fWasParsedMask |= kArpWasParsed;

        // Read the hardware type field.
        fArpHardwareType = NetworkToHost(*((uint16 *) (pfBuffer + arpOffset)));

        // Now the protocol type field.
        fArpProtocolType = NetworkToHost(*((uint16 *) (pfBuffer + arpOffset + 2)));

        // Next the hardware size field.
        fArpHardwareSize = pfBuffer[arpOffset + 4];

        // Then the protocol size field.
        fArpProtocolSize = pfBuffer[arpOffset + 5];

        // Now the opcode field.
        fArpOpcode = NetworkToHost(*((uint16 *) (pfBuffer + arpOffset + 6)));

        arpOffset += 8;

        // Next to last, we have the sender's hardware and protocol addresses.
        if (fArpHardwareType == kArpHwTypeEthernet)
        {
            fArpSenderHardwareAddress.Set(pfBuffer + arpOffset);
        }
        else
        {
            fArpSenderHardwareAddress = kAllZerosMac;
        }

        arpOffset += fArpHardwareSize;

        if (fArpProtocolType == kEtherTypeIp)
        {
            fArpSenderProtocolAddress.Set(pfBuffer + arpOffset);
        }
        else
        {
            fArpSenderProtocolAddress = kAllZerosIp;
        }

        arpOffset += fArpProtocolSize;

        // Finally we have the target's hardware and protocol addresses.
        if (fArpHardwareType == kArpHwTypeEthernet)
        {
            fArpTargetHardwareAddress.Set(pfBuffer + arpOffset);
        }
        else
        {
            fArpTargetHardwareAddress = kAllZerosMac;
        }

        arpOffset += fArpHardwareSize;

        if (fArpProtocolType == kEtherTypeIp)
        {
            fArpTargetProtocolAddress.Set(pfBuffer + arpOffset);
        }
        else
        {
            fArpTargetProtocolAddress = kAllZerosIp;
        }
    }
}


void BcmEthernetPacket::ParseIcmpImpl(void)
{
    unsigned int icmpHeaderOffset = fIpHeaderOffset + fIpHeaderLengthBytes;

    // Make sure the IP header is parsed.  If not, then parse it; we need it
    // for some fields.  If this isn't an ICMP packet, then we can bail.
    if ((ParseIp() == false) || (IpProtocolTypeParsed() != kIcmp))
    {
        return;
    }

    // Remember that we have parsed all of the TCP/UDP fields now.
    fWasParsedMask |= kIcmpWasParsed;

	// If the fragment offset is not zero, then this packet
	// is an IP fragment that does not contain an ICMP header.
	if( fIpFragmentOffset != 0 )
	{		
		// Reset the ICMP member variables since these variables
		// are not initialized previously and the caller of this function might
		// read their values without checking the return value of this function.
		// Set ICMP type and code to 0xff since zero is a valid value for both fields.
		fIcmpType = 0xff;
		fIcmpCode = 0xff;
		fIcmpIdentifier = 0;
		fIcmpSequenceNumber = 0;

		return;
	}

    // The Icmp Type is first.
    fIcmpType = NetworkToHost(*((uint8 *) (pfBuffer + icmpHeaderOffset)));

    // The Icmp Code is next.
    fIcmpCode = NetworkToHost(*((uint8 *) (pfBuffer + icmpHeaderOffset + 1)));

   // parse the identifier and sequence for packets which have it
    if ((fIcmpType == kIcmpTypeEchoRequest) || (fIcmpType == kIcmpTypeEchoReply) ||
        (fIcmpType == kIcmpTypeTimeStampRequest) || (fIcmpType == kIcmpTypeTimeStampReply) ||
        (fIcmpType == kIcmpTypeAddressMaskRequest) || (fIcmpType == kIcmpTypeAddressMaskReply))
    {
        fIcmpIdentifier     = NetworkToHost(*((uint16 *) (pfBuffer + icmpHeaderOffset + 4)));
        fIcmpSequenceNumber = NetworkToHost(*((uint16 *) (pfBuffer + icmpHeaderOffset + 6)));
    }
    else
    { 
        // Identifier and Sequence Number is not set in packet, just set to zero
        fIcmpIdentifier     = 0;
        fIcmpSequenceNumber = 0;
    }

}


void BcmEthernetPacket::ParseIcmpV6Impl(void)
{
    // Make sure the IP header is parsed.  If not, then parse it; we need it
    // for some fields.  If this isn't an ICMP packet, then we can bail.
    if ((ParseIpV6() == false) || (IpProtocolTypeParsed() != kIcmpV6))
    {
        return;
    }

    uint8 *pIcmpV6Header = (uint8 *) pfEthernetPacket->pDataBufferHead->pvStart + 
                                     EthernetHeaderLengthParsed() + 
                                     IpHeaderLengthBytesParsed();

    // Remember that we have parsed all of the TCP/UDP fields now.
    fWasParsedMask |= kIcmpV6WasParsed;

    // The Icmp Type is first.
    fIcmpType = pIcmpV6Header[0];

    // The Icmp Code is next.
    fIcmpCode = pIcmpV6Header[1];
}

// /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ .
// *************************************************************************
// Note - for cache optimization, these functions have been moved here;
// please don't reorder these functions!
// *************************************************************************


// Causes the currently stored packet (if any) to be released, and my data
// members are reset to an initial state.
//
// Parameters:  None.
//
// Returns:
//      A pointer to the packet that was previous owned by this class, or
//      NULL if there wasn't one.
//
ETHERNET_PACKET *BcmEthernetPacket::ReleasePacket(void)
{
    // Pull the old packet pointer aside.
    ETHERNET_PACKET *pPreviousPacket = pfEthernetPacket;

    // Reset the packet length.
    fPacketLength = 0;

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = false;

    fWasParsedMask = 0;

    // Set the packet pointer to NULL.
    pfEthernetPacket = NULL;

    // PR11290 also set the buffer pointer to NULL.
    pfBuffer = NULL;

    // Just return the previous packet that we were using.
    return pPreviousPacket;
}


// Allows a client to query/set the total number of bytes in the packet,
// starting at the MAC DA, going through the end of the PDU, but not
// inluding the CRC32 at the end.
//
// This is useful because the packet may not exist in a single buffer.
//
// Parameters:  None.
//
// Returns:
//      The total packet length.
//
void BcmEthernetPacket::SetPacketLength(unsigned int length)
{
    fPacketLength = length;
    pfEthernetPacket->pDataBufferHead->uiLen = (uint16) length;

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


// Allows the client to set/get the usage count associated with the packet.
// Since the same packet can be sent to multiple interfaces, it can only
// be released once the usage count goes to so.
//
// Parameters:
//      count - the usage count to set.
//
// Returns:
//      The current usage count.
//
void BcmEthernetPacket::UsageCount(unsigned int count)
{
    pfEthernetPacket->ulCount = count;
}


unsigned int BcmEthernetPacket::UsageCount(void) const
{
    return pfEthernetPacket->ulCount;
}


void BcmEthernetPacket::DecrementUsageCount(void)
{
    if (pfEthernetPacket->ulCount > 0)
    {
        pfEthernetPacket->ulCount--;
    }
}


void BcmEthernetPacket::IncrementUsageCount(void)
{
    pfEthernetPacket->ulCount++;
}


// This dumps the packet out in a human-readable form.
//
// Parameters:
//      outStream - the ostream to which the packet should be printed.
//
// Returns:
//      A reference to the ostream to support operation chaining.
//
ostream & BcmEthernetPacket::Print(ostream &outStream, bool verbose)
{
    static const char *gFormatNames[] =
    {
        "Unknown",
        "Ethernet II/DIX",
        "Raw 802.3",
        "IEEE 802.2",
        "SNAP"
    };

    if (pfEthernetPacket == NULL)
    {
        outStream << "\nThe packet is empty (NULL pointer)!" << endl;
    }

    else
    {
        // Use the pre-parse methods so that we don't get so much wasteful
        // inline expansion.  Only parse the TCP/UDP header if this is an
        // IP packet.
        ParseLlc();

        if (ParseIp())
        {
            ParseTcpUdp();
            ParseTcp();
        }
        else if ( ParseIpV6() )
        {
            ParseTcpUdp();
            ParseTcp();
        }
        else
        {
            // If not an IP packet, then try ARP.
            ParseArp();
        }

        // Note that I have to use my own accessors throughout this method;
        // this is because of deferred parsing.  I don't load my data members
        // from the actual packet until they are accessed.
        outStream 
            << "\nEthernet Frame Header:  (Format is " 
            << gFormatNames[GetFrameFormatParsed()] << ")\n";

        // Display the Ethernet header fields.
        outStream << "DestAddr=" << MacDa() << "; SrcAddr=" << MacSa();

        if (GetFrameFormatParsed() != kEthernetII)
        {
            outStream << "; EtherLen=" << EtherLengthParsed() << " bytes; DSAP=" 
                      << inHex(DsapParsed()) << "; SSAP=" << inHex(SsapParsed())
                      << "; Ctl=" << inHex(ControlParsed()) << "; OrgCode="
                      << inHex(OrgCodeParsed(0)) << " " << inHex(OrgCodeParsed(1))
                      << " " << inHex(OrgCodeParsed(2));
        }

        if ((GetFrameFormatParsed() == kEthernetII) || (GetFrameFormatParsed() == kSnap))
        {
            const char *pEtherTypeName;

            switch (EtherTypeParsed())
            {
                case kEtherTypeIp:
                    pEtherTypeName = "IP";
                    break;

                case kEtherTypeIpV6:
                    pEtherTypeName = "IPV6";
                    break;

                case kEtherTypeArp:
                    pEtherTypeName = "ARP";
                    break;

                case kEtherTypeRarp:
                    pEtherTypeName = "RARP";
                    break;

                case kEtherTypeIpx:
                    pEtherTypeName = "IPX";
                    break;

                default:
                    pEtherTypeName = "Unknown";
                    break;
            }

            outStream << "; EtherType=" << inHex(EtherTypeParsed()) << " (" << pEtherTypeName << ")";
            outStream << " Merged protocol = " << inHex(fMergedLlcProtocol) << endl;
        }

        outStream << '\n';

        // Display the IP header fields if this is an IP frame.
        if (EtherTypeParsed() == kEtherTypeIp || EtherTypeParsed() == kEtherTypeIpV6)
        {
            const char *pIpProtName;

            switch (IpProtocolTypeParsed())
            {
                case kIcmp:
                    pIpProtName = "ICMP";
                    break;

                case kIcmpV6:
                    pIpProtName = "ICMPv6";
                    break;

                case kIgmp:
                    pIpProtName = "IGMP";
                    break;

                case kTcp:
                    pIpProtName = "TCP";
                    break;

                case kUdp:
                    pIpProtName = "UDP";
                    break;

                default:
                    pIpProtName = "Unknown";
                    break;
            }

            outStream << "IP Frame Header:\n";

            outStream << "Ver=" << (unsigned int) IpVersionParsed()
                      //<< "; HdrLen=" << IpHeaderLengthBytesParsed()
                      //<< " bytes; ToS=" << inHex(IpTypeOfServiceParsed())
                      << "; ProtType=" << (unsigned int) IpProtocolTypeParsed() << " (" << pIpProtName
                      << "); DestIP=" << DestIpAddressParsed()
                      << "; SrcIP=" << SourceIpAddressParsed() << '\n';

            // Display the TCP/UDP header fields if this is a TCP/UDP IP frame.
            if (IpProtocolTypeTcpUdpParsed())
            {
                if (FragmentOffsetParsed() == 0)
                {
                    outStream << "\nTCP/UDP Frame Header:  DestPort=" 
                              << DestTcpUdpPortParsed() << " (" << inHex(DestTcpUdpPortParsed())
                              << "); SrcPort=" << SourceTcpUdpPortParsed() 
                              << " (" << inHex(SourceTcpUdpPortParsed()) << ")\n";
                
                    // Display the TCP header fields if this is a TCP/IP frame.
                    if (IpProtocolTypeParsed() == kTcp)
                    {
                        outStream << "\nTCP Frame Header:  SeqNum="
                                  << TcpSequenceNumberParsed() << "; AckNum="
                                  << TcpAckNumberParsed() << "; HdrLen="
                                  << TcpHeaderLengthBytesParsed() << "; Flags="
                                  << inHex(TcpFlagsParsed()) << "; WindSize="
                                  << TcpWindowSizeParsed() << "; UrgentPtr="
                                  << TcpUrgentPointerParsed() << '\n';
                    }
                }
            }
            // Display the TCP/UDP header fields if this is a TCP/UDP IP frame.
            if (IpProtocolTypeParsed() == kIcmpV6)
            {
                ParseIcmpV6Impl();
                typedef struct IcmpV6MessageTypeStrings
                {
                    uint8  type;
                    const char *string;
                } IcmpV6MessageTypeStrings;

                const IcmpV6MessageTypeStrings icmpV6MessageTypeStrings[] =
                {
                    {1, "DestinationUnreachable"},
                    {2, "PacketTooBig"},
                    {3, "TimeExceeded"},
                    {4, "ParameterProblem"},
                    {128, "EchoRequest"},
                    {129, "EchoReply"},
                    {130, "GroupMembershipQuery"},
                    {131, "GroupMembershipReport"},
                    {132, "GroupMembershipReduction"},
                    {133, "RouterSolicitation"},
                    {134, "RouterAdvertisement"},
                    {135, "NeighborSolicitation"},
                    {136, "NeighborAdvertisement"},
                    {137, "Redirect"}, 
                    {138, "RouterRenumbering"},
                    {139, "InformationQuery"},   
                    {140, "InformationResponse"},
                    {141, "InverseNeighborDiscoverySolicitation"},
                    {142, "InverseNeighborDiscoveryAdvertisement"},
                    {143, "Version2MulticastListenerReport"},      
                    {144, "HomeAgentAddressDiscoveryRequest"},    
                    {145, "HomeAgentAddressDiscoveryReply"},       
                    {146, "MobilePrefixSolicitation"},             
                    {147, "MobilePrefixAdvertisement"},          
                    {148, "CertificationPathSolicitation"},
                    {149, "CertificationPathAdvertisement"},       
                    {150, "ExperimentalMobilityProtocols"},        
                    {151, "MulticastRouterAdvertisement"},         
                    {152, "MulticastRouterSolicitation"},          
                    {153, "MulticastRouterTermination"},       
                    {0, "Unknown"}
                };
                
                const IcmpV6MessageTypeStrings * pIcmpV6MessageTypeStrings = icmpV6MessageTypeStrings;
                while( (pIcmpV6MessageTypeStrings->type != fIcmpType) &&
                       (pIcmpV6MessageTypeStrings->type != 0)
                     )
                {
                    pIcmpV6MessageTypeStrings++;
                }

                outStream << "ICMPv6 Type: " << pIcmpV6MessageTypeStrings->string << "\n";
            }
        }
        else if (EtherTypeParsed() == kEtherTypeArp)
        {
            const char *pHardTypeName;
            const char *pProtTypeName;
            const char *pOpCodeName;

            switch (ArpOpcodeParsed())
            {
                case 1:
                    pOpCodeName = "Request";
                    break;

                case 2:
                    pOpCodeName = "Reply";
                    break;

                default:
                    pOpCodeName = "Unknown";
                    break;
            }

            if (ArpHardwareTypeParsed() == kArpHwTypeEthernet)
            {
                pHardTypeName = "Ethernet";
            }
            else
            {
                pHardTypeName = "Unknown";
            }

            if (ArpProtocolTypeParsed() == kEtherTypeIp)
            {
                pProtTypeName = "IP";
            }
            else
            {
                pProtTypeName = "Unknown";
            }

            outStream << "\nARP Frame Contents:\n";

            outStream 
                << "HardType=" << ArpHardwareTypeParsed() << " (" << pHardTypeName
                << "); ProtType=" << inHex(ArpProtocolTypeParsed()) << " (" << pProtTypeName
                << "); HardSize=" << (int) ArpHardwareSizeParsed()
                << " bytes; ProtSize=" << (int) ArpProtocolSizeParsed()
                << " bytes; OpCode=" << ArpOpcodeParsed() << " (" << pOpCodeName
                << ")\nSender h/w addr=";
             
            if (ArpHardwareTypeParsed() == kArpHwTypeEthernet)
            {
                outStream << ArpSenderHardwareAddressParsed();
            }
            else
            {
                outStream << "????";
            }

            outStream << "; Sender prot addr=";

            if (ArpProtocolTypeParsed() == kEtherTypeIp)
            {
                outStream << ArpSenderProtocolAddressParsed();
            }
            else
            {
                outStream << "????";
            }
            
            outStream << "\nTarget h/w addr=";
            
            if (ArpHardwareTypeParsed() == kArpHwTypeEthernet)
            {
                outStream << ArpTargetHardwareAddressParsed();
            }
            else
            {
                outStream << "????";
            }

            outStream << "; Target prot addr=";
            
            if (ArpProtocolTypeParsed() == kEtherTypeIp)
            {
                outStream << ArpTargetProtocolAddressParsed();
            }
            else
            {
                outStream << "????";
            }

            outStream << '\n';
        }

        // Finally, dump the raw octets.
        if ( verbose == true )
        {
            DATA_BUFFER *pDataBuffer = pfEthernetPacket->pDataBufferHead;
            uint8 *pBuffer;
            unsigned int packetLength;
            BcmOctetBuffer octetBuffer(0);

            // The actual packet length depends on whether or not the PHS
            // sub-buffer suppression stuff was done.
            packetLength = pDataBuffer->uiLen - pDataBuffer->cmHalPhsInfo.mainBufferOffset + pDataBuffer->cmHalPhsInfo.numberOfBytes;

            outStream << "\nRaw buffer octets:  (" << packetLength << " bytes)\n\n";

            // Buffer chaining no longer supported.
            //while (1)
            {
                // Pull out the buffer pointer.
                pBuffer = (uint8 *) pDataBuffer->pvStart;

                // check for PHS Suppressed packet.
                if (pDataBuffer->cmHalPhsInfo.numberOfBytes || pDataBuffer->cmHalPhsInfo.mainBufferOffset)
                {
                    uint8 * pBufferSub;

                    // process the suppressed header portion
                    if (pDataBuffer->cmHalPhsInfo.numberOfBytes)
                    {
                        pBufferSub = (uint8 *) (pDataBuffer->cmHalPhsInfo.pPreBuffer);

                        // Let the octet buffer do the formatted hex and ASCII
                        // printing.
                        octetBuffer.AdoptBuffer(pBufferSub, 
                                                pDataBuffer->cmHalPhsInfo.numberOfBytes, 
                                                pDataBuffer->cmHalPhsInfo.numberOfBytes, 
                                                false);

                        outStream << octetBuffer;
                    }
                    // process remainder of the packet
                    if (pDataBuffer->cmHalPhsInfo.mainBufferOffset < pDataBuffer->uiLen)
                    {
                        unsigned int numberOfBytes = pDataBuffer->uiLen - pDataBuffer->cmHalPhsInfo.mainBufferOffset;

                        pBufferSub = pBuffer + pDataBuffer->cmHalPhsInfo.mainBufferOffset;
                        
                        // Let the octet buffer do the formatted hex and ASCII
                        // printing.
                        octetBuffer.AdoptBuffer(pBufferSub, numberOfBytes, 
                                                numberOfBytes, false);

                        outStream << octetBuffer;
                    }
                }
                else
                {
                    // Let the octet buffer do the formatted hex and ASCII
                    // printing.
                    octetBuffer.AdoptBuffer(pBuffer, pDataBuffer->uiLen, 
                                            pDataBuffer->uiLen, false);

                    outStream << octetBuffer;
                }

                // Buffer chaining no longer supported.
                /*
                // Stop if we just did the last buffer.
                if (pDataBuffer == pfEthernetPacket->pDataBufferTail)
                {
                    break;
                }

                // Otherwise, go to the next one.
                pDataBuffer = pDataBuffer->pNext;
                */
            }
        }
    }

    return outStream;
}


// This is useful for doing loopback operation.  It swaps the MAC SA with
// the MAC DA, allowing you to send the packet back to the originator.
// If this is an IP packet, you can also have it swap the dest and source
// IP address.  If it is a TCP or UDP IP packet, you can also swap the
// dest and source port.
//
// Parameters:
//      swapIp - tells it whether or not to swap the IP addresses.  This
//               only happens for IP packets.
//      swapPort - tells it whether or not to swap the TCP/UDP ports.  This
//                 only happens for TCP/UDP packets.
//
// Returns:  Nothing.
//
void BcmEthernetPacket::SwapAddresses(bool swapIp, bool swapPort)
{
    BcmMacAddress tempMac;

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;

    // Copy the MAC DA for later use.
    tempMac = fMacDa;

    // Store the MAC SA in the MAC DA.  I have to update both the member and
    // the buffer.
    MacDa(fMacSa);

    // Now store the old MAC DA in the MAC SA, updating both the member and the
    // buffer.
    MacSa(tempMac);

    // If this is an IP packet, then see if we need to swap the IP addresses.
    if ((fEtherType == kEtherTypeIp) && (swapIp == true))
    {
        BcmIpAddress tempIp;

        // Copy the Dest IP addr for later use.
        tempIp = fDestIpAddress;

        // Store the source IP address in the dest IP address, updating both the
        // member and the buffer.
        DestIpAddress(fSourceIpAddress);

        // Store the old dest IP addr in the source IP addr, updating both the
        // member and the buffer.
        SourceIpAddress(tempIp);

        // If this is a TCP/UDP packet, then see if we need to swap the port
        // numbers.
        if ((fIpProtocolTypeIsTcpUdp == true) && (swapPort == true))
        {
            uint16 tempPort;

            // Copy the dest port for later use.
            tempPort = fDestTcpUdpPort;

            // Store the source port in the dest port.
            DestTcpUdpPort(fSourceTcpUdpPort);

            // Store the old dest port in the source port.
            SourceTcpUdpPort(tempPort);
        }
    }
}


void BcmEthernetPacket::MacDa(const BcmMacAddress &macDa)
{
    // Update my member.
    fMacDa = macDa;

    // Update the buffer.
    fMacDa.Get(pfBuffer[0], pfBuffer[1], pfBuffer[2], 
               pfBuffer[3], pfBuffer[4], pfBuffer[5]);

    // Calculate what type of MAC DA this is.
    if (fMacDa.IsMulticast() == false)
    {
        fMacDaType = kUnicast;
    }
    else
    {
        // The lsb of the first byte is a 1; now I need to differentiate
        // between broadcast (which is a special class of multicast packet)
        // an plain old multicast.
        if (fMacDa == kBroadcastMac)
        {
            fMacDaType = kBroadcast;
        }
        else
        {
            fMacDaType = kMulticast;
        }
    }

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


void BcmEthernetPacket::MacSa(const BcmMacAddress &macSa)
{
    // Update my member.
    fMacSa = macSa;

    // Update the buffer.
    fMacSa.Get(pfBuffer[6], pfBuffer[7], pfBuffer[8], 
               pfBuffer[9], pfBuffer[10], pfBuffer[11]);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


// this is used when you are constructing an ethernet packet.  
// If we are starting from scratch, you must call this function to 
// tell me whether I am IEEE 802.2/802.3 (RFC 1042) or Ethernet (RFC894)
// before you call any other methods. 
void BcmEthernetPacket::SetFrameFormat(FrameFormat frameFormat)
{
    // set the frame format field
    fFrameFormat = frameFormat;

    // now we know the frame format, we know the fEtherTypeOffset
    switch (frameFormat)
    {
        case kEthernetII:
            fEtherTypeOffset = 12;
            fIpHeaderOffset = 14;
            break;

        case kIeee802_2:
            // this applies to IPX packets only.  There is no EtherType field. 
            fIpHeaderOffset = 22;
            break;

        case kSnap:
            fEtherTypeOffset = 20;
            fIpHeaderOffset = 22;
            break;

        case kRaw802_3:
        default:
            // no ether type and the payload is not IP
            break;
    }

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}

// the type field in the Ethernet header.  Use CommonEtherTypes enum.
void BcmEthernetPacket::EtherType(uint16 etherType)
{
    // Update my member.
    fEtherType = etherType;
    
    // PR 12181:  Special optimized merged value for LLC filtering.  
    // This consists of the lower 16 bits being the ethertype OR DSAP
    // and the upper 16 bits containing a flag to indicate which 
    // protocol type is being used.
    // PR 12614:  Added this here, overlooked in original PR.
    fMergedLlcProtocol = kLlcProtocolHasEthertype | fEtherType;

    // Update the buffer
    pfBuffer[fEtherTypeOffset] = (uint8) ((fEtherType >> 8) & 0xff);
    pfBuffer[fEtherTypeOffset + 1] = (uint8) (fEtherType & 0xff);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


void BcmEthernetPacket::EtherLength(uint16 etherLength)
{
    // Update my member.
    fEtherLength = etherLength;

    // Update the buffer
    pfBuffer[12] = (uint8) ((fEtherLength >> 8) & 0xff);
    pfBuffer[13] = (uint8) (fEtherLength & 0xff);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


void BcmEthernetPacket::Dsap(uint8 dsap)
{
    // Update my member.
    fDsap = dsap;
    
    // PR 12181:  Special optimized merged value for LLC filtering.  
    // This consists of the lower 16 bits being the ethertype OR DSAP
    // and the upper 16 bits containing a flag to indicate which 
    // protocol type is being used.
    // PR 12614:  Added this here, overlooked in original PR.
    fMergedLlcProtocol = kLlcProtocolHasDsap | fDsap;

    // Update the buffer
    pfBuffer[14] = fDsap;

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


void BcmEthernetPacket::Ssap(uint8 ssap)
{
    // Update my member.
    fSsap = ssap;

    // Update the buffer
    pfBuffer[15] = fSsap;

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}



// Returns a pointer to the first byte of the payload following the LLC
// header.  Note that if this isn't a Ethernet packet (or if the LLC header
// hasn't been parsed yet), then this returns NULL!
const uint8 *BcmEthernetPacket::LlcPdu(void) const
{
    uint8 *pPayload = NULL;

    if (fWasParsedMask & kLlcWasParsed)
    {
        // Get a pointer to the beginning of the packet.
        pPayload = (uint8 *) pfEthernetPacket->pDataBufferHead->pvStart;

        // Adjust it to point past the IP header, there may not be an Ip Hdr present,
        // and in that case ParseLlcImpl() will make the offset zero.
        pPayload += fIpHeaderOffset;
    }

    return pPayload;
}

// Same as above, but allows the PDU to be modified.  
//
uint8 *BcmEthernetPacket::LlcPduWritable(void)
{
    // Just use the implementation from the constipated version, then cast to
    // non-const.
    return (uint8 *) LlcPdu();
}

#ifdef BCM_STANDALONE_DHCP_CLIENT
const uint8 *BcmEthernetPacket::PduHeader(void) const
{
    uint8 *pPayload = NULL;

    if (fWasParsedMask & kLlcWasParsed)
    {
        // Get a pointer to the beginning of the packet.
        pPayload = (uint8 *) pfEthernetPacket->pDataBufferHead->pvStart;
    }

    return pPayload;
}

    // Same as above, but allows the PDU to be modified.  
uint8 *BcmEthernetPacket::PduHeaderWritable(void)
{
    return (uint8 *) PduHeader();

}
#endif

void BcmEthernetPacket::Control(uint8 control)
{
    // Update my member.
    fControl = control;

    // Update the buffer
    pfBuffer[16] = fControl;

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


void BcmEthernetPacket::OrgCode(unsigned int offset, uint8 orgCode)
{
    // Update my member.
    fOrgCode[offset] = orgCode;

    // Update the buffer
    pfBuffer[17 + offset] = orgCode;

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


void BcmEthernetPacket::ArpHardwareType(uint16 hardwareType)
{
    // Update my member.
    fArpHardwareType = hardwareType;

    // Update the buffer.
    pfBuffer[fEtherTypeOffset + 2] = (uint8) ((hardwareType >> 8) & 0xff);
    pfBuffer[fEtherTypeOffset + 3] = (uint8) (hardwareType & 0xff);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


void BcmEthernetPacket::ArpProtocolType(uint16 protocolType)
{
    // Update my member.
    fArpProtocolType = protocolType;

    // Update the buffer.
    pfBuffer[fEtherTypeOffset + 4] = (uint8) ((protocolType >> 8) & 0xff);
    pfBuffer[fEtherTypeOffset + 5] = (uint8) (protocolType & 0xff);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


void BcmEthernetPacket::ArpHardwareSize(uint8 hardwareSize)
{
    // Update my member.
    fArpHardwareSize = hardwareSize;

    // Update the buffer.
    pfBuffer[fEtherTypeOffset + 6] = hardwareSize;

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


void BcmEthernetPacket::ArpProtocolSize(uint8 protocolSize)
{
    // Update my member.
    fArpProtocolSize = protocolSize;

    // Update the buffer.
    pfBuffer[fEtherTypeOffset + 7] = protocolSize;

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


void BcmEthernetPacket::ArpOpcode(uint16 opcode)
{
    // Update my member.
    fArpOpcode = opcode;

    // Update the buffer.
    pfBuffer[fEtherTypeOffset + 8] = (uint8) ((opcode >> 8) & 0xff);
    pfBuffer[fEtherTypeOffset + 9] = (uint8) (opcode & 0xff);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


void BcmEthernetPacket::ArpSenderHardwareAddress(const BcmMacAddress &hardwareAddress)
{
    // PR12382 - don't modify the pfBuffer address, calculate an offset.
    unsigned int offset = fEtherTypeOffset + 10;

    // Update my member.
    fArpSenderHardwareAddress = hardwareAddress;

    // Update the buffer.
    // PR12382 - don't modify the pfBuffer address, calculate an offset.
    fArpSenderHardwareAddress.Get(pfBuffer[offset + 0], pfBuffer[offset + 1], pfBuffer[offset + 2],
                                  pfBuffer[offset + 3], pfBuffer[offset + 4], pfBuffer[offset + 5]);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


void BcmEthernetPacket::ArpSenderProtocolAddress(const BcmIpAddress &protocolAddress)
{
    // PR12382 - don't modify the pfBuffer address, calculate an offset.
    unsigned int offset = fEtherTypeOffset + 10 + fArpHardwareSize;

    // Update my member.
    fArpSenderProtocolAddress = protocolAddress;

    // Update the buffer.
    // PR12382 - don't modify the pfBuffer address, calculate an offset.
    fArpSenderProtocolAddress.Get(pfBuffer[offset + 0], pfBuffer[offset + 1], pfBuffer[offset + 2], pfBuffer[offset + 3]);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


void BcmEthernetPacket::ArpTargetHardwareAddress(const BcmMacAddress &hardwareAddress)
{
    // PR12382 - don't modify the pfBuffer address, calculate an offset.
    unsigned int offset = fEtherTypeOffset + 10 + fArpHardwareSize + fArpProtocolSize;

    // Update my member.
    fArpTargetHardwareAddress = hardwareAddress;

    // Update the buffer.
    // PR12382 - don't modify the pfBuffer address, calculate an offset.
    fArpTargetHardwareAddress.Get(pfBuffer[offset + 0], pfBuffer[offset + 1], pfBuffer[offset + 2],
                                  pfBuffer[offset + 3], pfBuffer[offset + 4], pfBuffer[offset + 5]);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


void BcmEthernetPacket::ArpTargetProtocolAddress(const BcmIpAddress &protocolAddress)
{
    // PR12382 - don't modify the pfBuffer address, calculate an offset.
    unsigned int offset = fEtherTypeOffset + 10 + fArpHardwareSize + fArpProtocolSize + fArpHardwareSize;

    // Update my member.
    fArpTargetProtocolAddress = protocolAddress;

    // Update the buffer.
    // PR12382 - don't modify the pfBuffer address, calculate an offset.
    fArpTargetProtocolAddress.Get(pfBuffer[offset + 0], pfBuffer[offset + 1], pfBuffer[offset + 2], pfBuffer[offset + 3]);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


void BcmEthernetPacket::IpVersion(uint8 ipVersion)
{
    // Update my member.
    fIpVersion = ipVersion & 0x0f;

    // Update the buffer
    // mask off the IP version bits in this byte
    pfBuffer[fIpHeaderOffset] &= 0x0f;

    // OR in the IP version to leave the header length intact
    pfBuffer[fIpHeaderOffset] |= (fIpVersion << 4);

    // I need to update the header checksum!
    // The calculation may not be correct if the header length is not
    // intialized yet, but if that is the case, it does not matter now anyway. 
    UpdateIpHcs();
}


void BcmEthernetPacket::IpTotalLengthBytes(uint16 totalLength)
{
    fIpTotalLengthBytes = totalLength;

    // Update the buffer
    pfBuffer[fIpHeaderOffset + 2] = (uint8)((totalLength >> 8) & 0xff);
    pfBuffer[fIpHeaderOffset + 3] = (uint8)(totalLength & 0xff);

    // I need to update the header checksum!
    UpdateIpHcs();
}


void BcmEthernetPacket::IpHeaderLengthBytes(unsigned int headerLength)
{
    // Update my member.
    fIpHeaderLengthBytes = headerLength & (unsigned int) (~0x03);

    // Update the buffer
    // mask off the header length bits
    pfBuffer[fIpHeaderOffset] &= 0xf0;

    // OR in the header length converted to dwords. 
    pfBuffer[fIpHeaderOffset] |= (fIpHeaderLengthBytes / 4);

    // now that we know the length of the IP header, we know the offset of 
    // the TCP or UDP header
    fTcpUdpHeaderOffset = fIpHeaderOffset + headerLength;

    // I need to update the header checksum!
    UpdateIpHcs();
}

void BcmEthernetPacket::Identification(uint16 ident)
{
    // Update my member
    fIpIdentification = ident;
    // Update the buffer
    pfBuffer[fIpHeaderOffset + 4] = (uint8)((ident >> 8) & 0xff);
    pfBuffer[fIpHeaderOffset + 5] = (uint8)((ident) & 0xff);

    // I need to update the header checksum!
    UpdateIpHcs();
}

void BcmEthernetPacket::Flags(uint8 flags)
{
    // Update my member
    fIpFlags = flags;

    // Update the buffer
    // mask off the old flags bits
    pfBuffer[fIpHeaderOffset + 6] &= 0x1f;

    // now OR the new ones in 
    pfBuffer[fIpHeaderOffset + 6] |= (flags << 5);

    // I need to update the header checksum!
    UpdateIpHcs();
}

void BcmEthernetPacket::FragmentOffset(uint16 offset)
{
    // Update my member
    fIpFragmentOffset = offset;

    // Update the buffer
    // mask off the old fragment offset
    pfBuffer[fIpHeaderOffset + 6] &= 0xe0;
    pfBuffer[fIpHeaderOffset + 7] &= 0;

    // now OR in the new offset
    pfBuffer[fIpHeaderOffset + 6] |= (uint8)((offset >> 8) & 0x1f);
    pfBuffer[fIpHeaderOffset + 7] |= (uint8)(offset & 0xff);

    // I need to update the header checksum!
    UpdateIpHcs();
}

uint8 BcmEthernetPacket::TimeToLive()
{
    // Update the buffer
    return pfBuffer[fIpHeaderOffset + 8];
}

void BcmEthernetPacket::TimeToLive(uint8 ttl)
{
    // Update the buffer
    pfBuffer[fIpHeaderOffset + 8] = ttl;

    // I need to update the header checksum!
    UpdateIpHcs();
}


void BcmEthernetPacket::IpTypeOfService(uint8 tos)
{
    // Update my member.
    fIpTypeOfService = tos;

    // Update the buffer
    pfBuffer[fIpHeaderOffset + 1] = fIpTypeOfService;

    // I need to update the header checksum!
    UpdateIpHcs();
}


void BcmEthernetPacket::IpProtocolType(uint8 protocolType)
{
    // Update my member.
    fIpProtocolType = protocolType;

    if ((fIpProtocolType == kTcp) || (fIpProtocolType == kUdp))
    {
        fIpProtocolTypeIsTcpUdp = true;
    }
    else
    {
        fIpProtocolTypeIsTcpUdp = false;
    }

    // Update the buffer
    pfBuffer[fIpHeaderOffset + 9] = fIpProtocolType;

    // I need to update the header checksum!
    UpdateIpHcs();
}


void BcmEthernetPacket::SourceIpAddress(const BcmIpAddress &sourceIp)
{
    // Update my member.
    fSourceIpAddress = sourceIp;

    // Update the buffer
    if( sourceIp.IsIpV6Address() )
    {
        fSourceIpAddress.Get(&pfBuffer[fIpHeaderOffset + 8], 16);
    }
    else
    {
        fSourceIpAddress.Get(pfBuffer[fIpHeaderOffset + 12],
                             pfBuffer[fIpHeaderOffset + 13],
                             pfBuffer[fIpHeaderOffset + 14],
                             pfBuffer[fIpHeaderOffset + 15]);

        // I need to update the header checksum!
        UpdateIpHcs();
    }
}


void BcmEthernetPacket::SourceIpAddressFixup(const BcmIpAddress &sourceIp)
{
    uint8 saveBuf[4];
    uint8 *pIpHeader;

    // Get a pointer to the IP header.
    pIpHeader = (uint8 *) (pfEthernetPacket->pDataBufferHead->pvStart);
    pIpHeader += fIpHeaderOffset;
    
    // save old data
    fSourceIpAddress.Get(saveBuf[0], saveBuf[1], saveBuf[2], saveBuf[3]);

    // Update my member.
    fSourceIpAddress = sourceIp;

    // Update the buffer
    fSourceIpAddress.Get(pfBuffer[fIpHeaderOffset + 12],
                         pfBuffer[fIpHeaderOffset + 13],
                         pfBuffer[fIpHeaderOffset + 14],
                         pfBuffer[fIpHeaderOffset + 15]);

    // We update the checksum the fast way by fixing up the data instead of recalculating the entire checksum
    ChecksumFixup(&pIpHeader[10], &saveBuf[0], 4,  (uint8 *) &pfBuffer[fIpHeaderOffset + 12], 4);

    // We also potentially need to update the TcpUdp checksum
    if (fIpProtocolTypeIsTcpUdp)
    {
        if (IpProtocolTypeParsed() == kUdp)
        {
            // if we are udp, and checksum is zero, then no reason to update
            if (pfBuffer[fTcpUdpHeaderOffset + 6] != 0 || pfBuffer[fTcpUdpHeaderOffset + 7])
            {
                //update the Udp checksum 
                ChecksumFixup(&pfBuffer[fTcpUdpHeaderOffset + 6], saveBuf, 4,  (uint8 *) &pfBuffer[fIpHeaderOffset + 12], 4);
            }
        }
        else
        {
            //update the Tcp checksum 
            ChecksumFixup(&pfBuffer[fTcpUdpHeaderOffset + 16], saveBuf, 4,  (uint8 *) &pfBuffer[fIpHeaderOffset + 12], 4);
        }
    }
    
    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}

// fixup the Source IP, IP Checksum
void BcmEthernetPacket::SourceIpAddressFixupNoCheckSum(const BcmIpAddress &sourceIp)
{
    uint8 saveBuf[4];
    uint8 *pIpHeader;

    // Get a pointer to the IP header.
    pIpHeader = (uint8 *) (pfEthernetPacket->pDataBufferHead->pvStart);
    pIpHeader += fIpHeaderOffset;
    
    // save old data
    fSourceIpAddress.Get(saveBuf[0], saveBuf[1], saveBuf[2], saveBuf[3]);

    // Update my member.
    fSourceIpAddress = sourceIp;

    // Update the buffer
    fSourceIpAddress.Get(pfBuffer[fIpHeaderOffset + 12],
                         pfBuffer[fIpHeaderOffset + 13],
                         pfBuffer[fIpHeaderOffset + 14],
                         pfBuffer[fIpHeaderOffset + 15]);

    // We update the checksum the fast way by fixing up the data instead of recalculating the entire checksum
    ChecksumFixup(&pIpHeader[10], &saveBuf[0], 4,  (uint8 *) &pfBuffer[fIpHeaderOffset + 12], 4);

    // no need to update the TCP or UDP packet
}

// Fixup Destination IP Address, Ip Checksum, and TCP or UDP checksum
void BcmEthernetPacket::DestIpAddressFixup(const BcmIpAddress &destIp)
{
    uint8 saveBuf[4];
    uint8 *pIpHeader;

    // Get a pointer to the IP header.
    pIpHeader = (uint8 *) (pfEthernetPacket->pDataBufferHead->pvStart);
    pIpHeader += fIpHeaderOffset;
    // save old data
    fDestIpAddress.Get(saveBuf[0], saveBuf[1], saveBuf[2], saveBuf[3]);

    // Update my member.
    fDestIpAddress = destIp;

    // Update the buffer
    fDestIpAddress.Get(pfBuffer[fIpHeaderOffset + 16],
                         pfBuffer[fIpHeaderOffset + 17],
                         pfBuffer[fIpHeaderOffset + 18],
                         pfBuffer[fIpHeaderOffset + 19]);

    // We update the checksum the fast way by fixing up the data instead of recalculating the entire checksum
    ChecksumFixup(&pIpHeader[10], &saveBuf[0], 4,  (uint8 *) &pfBuffer[fIpHeaderOffset + 16], 4);

    // We also potentially need to update the TcpUdp checksum
    if (fIpProtocolTypeIsTcpUdp)
    {

        if (IpProtocolTypeParsed() == kUdp)
        {
            // if we are udp, and checksum is zero, then no reason to update
            if (pfBuffer[fTcpUdpHeaderOffset + 6] != 0 || pfBuffer[fTcpUdpHeaderOffset + 7])
            {
                //update the Udp checksum 
                ChecksumFixup(&pfBuffer[fTcpUdpHeaderOffset + 6], saveBuf, 4,  (uint8 *) &pfBuffer[fIpHeaderOffset + 16], 4);
            }
        }
        else
        {
            //update the Tcp checksum 
            ChecksumFixup(&pfBuffer[fTcpUdpHeaderOffset + 16], saveBuf, 4,  (uint8 *) &pfBuffer[fIpHeaderOffset + 16], 4);
        }
    }

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}

// Fixup Destination IP Address, Ip Checksum 
void BcmEthernetPacket::DestIpAddressFixupNoCheckSum(const BcmIpAddress &destIp)
{
    uint8 saveBuf[4];
    uint8 *pIpHeader;

    // Get a pointer to the IP header.
    pIpHeader = (uint8 *) (pfEthernetPacket->pDataBufferHead->pvStart);
    pIpHeader += fIpHeaderOffset;
    // save old data
    fDestIpAddress.Get(saveBuf[0], saveBuf[1], saveBuf[2], saveBuf[3]);

    // Update my member.
    fDestIpAddress = destIp;

    // Update the buffer
    fDestIpAddress.Get(pfBuffer[fIpHeaderOffset + 16],
                         pfBuffer[fIpHeaderOffset + 17],
                         pfBuffer[fIpHeaderOffset + 18],
                         pfBuffer[fIpHeaderOffset + 19]);

    // We update the checksum the fast way by fixing up the data instead of recalculating the entire checksum
    ChecksumFixup(&pIpHeader[10], &saveBuf[0], 4,  (uint8 *) &pfBuffer[fIpHeaderOffset + 16], 4);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}
void BcmEthernetPacket::DestIpAddress(const BcmIpAddress &destIp)
{
    // Update my member.
    fDestIpAddress = destIp;

    // Update the buffer
    if( destIp.IsIpV6Address() )
    {
        fDestIpAddress.Get(&pfBuffer[fIpHeaderOffset + 24], 16);
    }
    else
    {
        fDestIpAddress.Get(pfBuffer[fIpHeaderOffset + 16],
                           pfBuffer[fIpHeaderOffset + 17],
                           pfBuffer[fIpHeaderOffset + 18],
                           pfBuffer[fIpHeaderOffset + 19]);

        // I need to update the header checksum!
        UpdateIpHcs();
    }
}

// Returns a pointer to the first byte of the IP options, if present
const uint8 *BcmEthernetPacket::IpOptions(void) const
{

    uint8 *pOptions = NULL;

    if ((fWasParsedMask & kIpWasParsed) && (IpHeaderLengthBytesParsed() > 20))
    {
        // Get a pointer to the beginning of the packet.
        pOptions = (uint8 *) pfEthernetPacket->pDataBufferHead->pvStart;

        // Adjust it to point past the IP header.
        pOptions += fIpHeaderOffset;

        // Adjust it to point past the fixed fields
        pOptions += 20;
    }

    return pOptions;
}

// Helper method to check the IP Header checksum.  Before calling this
// method, you must call ParseLlc(), but you don't have to call ParseIp().
//
// Parameters:  None.
//
// Returns:
//      true if the IP header checksum is valid.
//      false if the IP header checksum is not valid.
//
bool BcmEthernetPacket::IpHcsIsValid(void) const
{
    unsigned int numberOfBytes;
    uint8 *pIpHeader;
    uint16 checkSum;

    // Get a pointer to the IP header.
    pIpHeader = (uint8 *) (pfEthernetPacket->pDataBufferHead->pvStart);
    pIpHeader += fIpHeaderOffset;

    // Pull out the IP header length.  Since the user doesn't have to call
    // ParseIp(), I can't use the IpHeaderLengthBytesParsed() method.  This
    // is the same algorithm used when we parse the IP header.
    numberOfBytes = pIpHeader[0] & 0x0f;
    numberOfBytes *= 4;

    // Calculate the IP header checksum over the existing data.  If the result
    // is 0, then it is valid.
    checkSum = CalculateIpHcs(pIpHeader, numberOfBytes);

    if (checkSum == 0)
    {
        return true;
    }

    return false;
}


void BcmEthernetPacket::IpV6Init(void)
{
    fIpHeaderLengthBytes = 40;
    fTcpUdpHeaderOffset = fIpHeaderOffset + fIpHeaderLengthBytes;

    fIpVersion = 6;

    // Initialize the buffer
    memset(pfBuffer + fIpHeaderOffset, 0, fIpHeaderLengthBytes);

    // OR in the IP version
    pfBuffer[fIpHeaderOffset] |= (fIpVersion << 4);
}

// Accessors for IPv6 header fields
void BcmEthernetPacket::IpV6TrafficClass(uint8 trafficClass)
{
    // mask off the old traffic class
    pfBuffer[fIpHeaderOffset] &= 0xf0;
    pfBuffer[fIpHeaderOffset + 1] &= 0x0f;

    // mask in the new traffic class
    pfBuffer[fIpHeaderOffset] |= trafficClass >> 4;
    pfBuffer[fIpHeaderOffset + 1] |= trafficClass << 4;
}

uint8  BcmEthernetPacket::IpV6TrafficClass()
{
    return fIpV6TrafficClass;
}

void BcmEthernetPacket::IpV6FlowLabel(uint32 flowLabel)
{
    // mask off the old 20-bit flow label
    pfBuffer[fIpHeaderOffset + 1] &= 0xf0;
    pfBuffer[fIpHeaderOffset + 2] = 0;
    pfBuffer[fIpHeaderOffset + 3] = 0;

    // mask in the new traffic class
    pfBuffer[fIpHeaderOffset + 1] |= (uint8)((flowLabel >> 16) & 0x0f);
    pfBuffer[fIpHeaderOffset + 2] |= (uint8)((flowLabel >> 8) & 0xff);
    pfBuffer[fIpHeaderOffset + 3] |= (uint8)(flowLabel & 0xff);
}

uint32 BcmEthernetPacket::IpV6FlowLabel()
{
    return fIpV6FlowLabel;
}

void BcmEthernetPacket::IpV6PayloadLength(uint16 payloadLength)
{           
    pfBuffer[fIpHeaderOffset + 4] = (uint8)(payloadLength >> 8);
    pfBuffer[fIpHeaderOffset + 5] = (uint8)(payloadLength & 0xff);
}

uint16 BcmEthernetPacket::IpV6PayloadLength()
{
    return fIpV6PayloadLength;
}

void BcmEthernetPacket::UpdateIpV6ReassemblyNextHeaderField(void)
{
    pfBuffer[fIpHeaderOffset + fIpV6UnfragmentableLastHeaderOffset] = fIpV6FragmentNextHeader;
}

uint8 BcmEthernetPacket::IpV6UnfragmentableLastHeader(void)
{
    return (pfBuffer[fIpHeaderOffset + fIpV6UnfragmentableLastHeaderOffset]);
}

void BcmEthernetPacket::IpV6UnfragmentableLastHeader(uint8 nextHeader)
{
    pfBuffer[fIpHeaderOffset + fIpV6UnfragmentableLastHeaderOffset] = nextHeader;
}

void BcmEthernetPacket::IpV6NextHeader(uint8 nextHeader)
{
    pfBuffer[fIpHeaderOffset + 6] = nextHeader;
    fIpV6NextHeader = nextHeader;

    // copy the protocol type over to fIpProtocolType so IPv4 and IPv6 can be 
    // treated the same. 
    fIpProtocolType = fIpV6NextHeader;
    fIpFragmentOffset = 0;

    if ((fIpV6NextHeader == kTcp) || (fIpV6NextHeader == kUdp))
    {
        fIpProtocolTypeIsTcpUdp = true;
    }
    else
    {
        fIpProtocolTypeIsTcpUdp = false;
    }
}

uint8 BcmEthernetPacket::IpV6NextHeader()
{
    return fIpV6NextHeader;
}

uint8 BcmEthernetPacket::IpV6LastNextHeader()
{
    return fIpV6NextHeader;
}

void BcmEthernetPacket::IpV6HopLimit(uint8 hopLimit)
{
    pfBuffer[fIpHeaderOffset + 7] = hopLimit;
}

uint8 BcmEthernetPacket::IpV6HopLimit()
{
    return fIpV6HopLimit;
}

// This function fixes up the checksum. 
// Parameters:
//      bptr_checksum   - pointer to where new checksum is stored and original checksum
//      bptr_old_data   - pointer to the old data
//      old_data_length - old data length
//      bptr_new_data   - pointer to the new data
//      new_data_length - pointer to the new data length
//
// Returns:
//      none
void BcmEthernetPacket::ChecksumFixup (uint8 *bptr_checksum, uint8 *bptr_old_data,
		uint16 old_data_length, uint8 *bptr_new_data, uint16  new_data_length)
{
	unsigned long working_checksum;
	unsigned long old_data_word;
	unsigned long new_data_word;

	working_checksum = (unsigned long) ((bptr_checksum[0] * 256) + bptr_checksum[1]);

	working_checksum = (~working_checksum & 0x0000FFFF);

	while (old_data_length > 0x0000)
		{
		if (old_data_length == 0x00000001)
			{
			old_data_word = (unsigned long) ((bptr_old_data[0] * 256) + bptr_old_data[1]);

			working_checksum = working_checksum - (old_data_word & 0x0000FF00);

			if ((long) working_checksum <= 0x00000000L)
				{
				--working_checksum;

				working_checksum = working_checksum & 0x0000FFFF;
				}

			break;
			}
		else
			{
			old_data_word = (unsigned long) ((bptr_old_data[0] * 256) + bptr_old_data[1]);

			bptr_old_data = bptr_old_data + 2;

			working_checksum = working_checksum - (old_data_word & 0x0000FFFF);

			if ((long) working_checksum <= 0x00000000L)
				{
				--working_checksum;

				working_checksum = working_checksum & 0x0000FFFF;
				}

			old_data_length = (unsigned short) (old_data_length - 2);
			}
		}

	while (new_data_length > 0x0000)
		{
		if (new_data_length == 0x00000001)
			{
			new_data_word = (unsigned long) ((bptr_new_data[0] * 256) + bptr_new_data[1]);

			working_checksum = working_checksum + (new_data_word & 0x0000FF00);

			if (working_checksum & 0x00010000)
				{
				++working_checksum;

				working_checksum = working_checksum & 0x0000FFFF;
				}

			break;
			}
		else
			{
			new_data_word = (unsigned long) ((bptr_new_data[0] * 256) + bptr_new_data[1]);

			bptr_new_data = bptr_new_data + 2;

			working_checksum = working_checksum + (new_data_word & 0x0000FFFF);

			if (working_checksum & 0x00010000)
				{
				++working_checksum;

				working_checksum = working_checksum & 0x0000FFFF;
				}

			new_data_length = (unsigned short) (new_data_length - 2);
			}
		}

	working_checksum = ~working_checksum;

	bptr_checksum[0] = (unsigned char) (working_checksum/256);
	bptr_checksum[1] = (unsigned char) (working_checksum & 0x000000FF);

}

void BcmEthernetPacket::UpdateIpHcs (void)
{
    unsigned int numberOfBytes;
    uint8 *pIpHeader;
    uint16 checkSum;

    // Get a pointer to the IP header.
    pIpHeader = (uint8 *) (pfEthernetPacket->pDataBufferHead->pvStart);
    pIpHeader += fIpHeaderOffset;

    // Pull out the IP header length.  Since the user doesn't have to call
    // ParseIp(), I can't use the IpHeaderLengthBytesParsed() method.  This
    // is the same algorithm used when we parse the IP header.
    numberOfBytes = pIpHeader[0] & 0x0f;
    numberOfBytes *= 4;

    // Because we are calculating the checksum to be transmitted, I need to
    // prime the existing location with 0's.
    pIpHeader[10] = 0;
    pIpHeader[11] = 0;

    // Calculate the IP header checksum over the existing data.  If the result
    // is 0, then it is valid.
    checkSum = CalculateIpHcs(pIpHeader, numberOfBytes);

    // Now store the checksum.
    pIpHeader[10] = (uint8) ((checkSum >> 8) & 0xff);
    pIpHeader[11] = (uint8) (checkSum & 0xff);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


// Helper function that calculates the IP Header checksum.  The pIpHeader must
// not be NULL, and numberOfBytes must be a multiple of 4.
//
// If you want to calculate the checksum in order to transmit the packet, then
// you need to set the checksum bytes in the header to 0 before calling this
// function, then store the value returned in that location.
//
// If you want to verify an existing checksum, then just call this method with
// the existing checksum.  If the value returned is 0, then the checksum is
// valid; otherwise, the checksum is not valid.  Note that most documentation
// says that the verified checksum should be 0xffff, but since we're taking the
// one's complement, it will wind up being 0 if it is valid.
//
// Parameters:
//      pIpHeader - pointer to the beginning of the IP header.
//      numberOfBytes - the size of the IP header in bytes.
//
// Returns:
//      The IP header checksum value.
//
static uint16 CalculateIpHcs(const uint8 *pIpHeader, unsigned int numberOfBytes)
{
    uint16 checkSum;
    uint16 *pHeaderAsWords;
    uint32 totalValue = 0;

    // Since the IP header will always be aligned on a 16-bit boundary, we can
    // optimize reads by doing them as uint16's.
    pHeaderAsWords = (uint16 *) pIpHeader;

    while (numberOfBytes > 0)
    {
        // Read out two words of data and accumulate both of them.  This is a
        // minor optimization based on the fact that the IP header will always
        // be a multiple of 4 bytes.
        totalValue += NetworkToHost(*pHeaderAsWords);
        pHeaderAsWords++;

        totalValue += NetworkToHost(*pHeaderAsWords);
        pHeaderAsWords++;

        numberOfBytes -= 4;
    }

    // Now we need to fold the accumulator down to 16 bits.
    totalValue = (totalValue & 0xffff) + ((totalValue >> 16) & 0xffff);
    totalValue += ((totalValue >> 16) & 0xffff);

    // Bring this down to 16 bits.
    checkSum = (uint16) (totalValue & 0xffff);

    // Returns the 1's complement.
    return ~checkSum;
}


// Returns a pointer to the first byte of the payload following the UDP
// header.  Note that if this isn't a IP packet (or if the IP header
// hasn't been parsed yet), then this returns NULL!
const uint8 *BcmEthernetPacket::IpPdu(void) const
{
    uint8 *pPayload = NULL;

    if (fWasParsedMask & kIpWasParsed || fWasParsedMask & kIpV6WasParsed)
    {
        // Get a pointer to the beginning of the packet.
        pPayload = (uint8 *) pfEthernetPacket->pDataBufferHead->pvStart;

        // Adjust it to point past the IP header.
        pPayload += fIpHeaderOffset;

        // Adjust it to point past the UDP header (which is always 8 bytes).
        pPayload += fIpHeaderLengthBytes;
    }

    return pPayload;
}



// Same as above, but allows the PDU to be modified.  Note that you are
// responsible for updating the IP checksum after the PDU is modified.
uint8 *BcmEthernetPacket::IpPduWritable(void)
{
    // Just use the implementation from the constipated version, then cast to
    // non-const.
    return (uint8 *) IpPdu();
}


// Allows you to read/write the UDP length field if you are constructing
// your  own packet from scratch.  This field includes the UDP header and
// the data. 
uint16 BcmEthernetPacket::UdpLengthBytes(void) const
{
    uint16 length;

    length = pfBuffer[fTcpUdpHeaderOffset + 4];
    length <<= 8;
    length |= pfBuffer[fTcpUdpHeaderOffset + 5];

    return length;
}


void BcmEthernetPacket::UdpLengthBytes(uint16 totalLength)
{
    // Update the buffer
    pfBuffer[fTcpUdpHeaderOffset + 4] = (uint8) ((totalLength >> 8) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 5] = (uint8) (totalLength & 0xff);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}

void BcmEthernetPacket::SourceTcpUdpPortFixup(uint16 sourcePort)
{
    // Update my member.
    fSourceTcpUdpPort = sourcePort;

    // Update the buffer
    // We also potentially need to update the TcpUdp checksum
    if (IpProtocolTypeParsed() == kUdp)
    {
        // if we are udp, and checksum is zero, then no reason to update
        if (pfBuffer[fTcpUdpHeaderOffset + 6] != 0 || pfBuffer[fTcpUdpHeaderOffset + 7])
        {
             // I need to update the UDP checksum using the fixup method 
            ChecksumFixup(&pfBuffer[fTcpUdpHeaderOffset + 6], &pfBuffer[fTcpUdpHeaderOffset], 2,  (uint8 *) &fSourceTcpUdpPort, 2);
        }
    }
    else
    {
         // I need to update the TCP/UDP checksum using the fixup method 
        ChecksumFixup(&pfBuffer[fTcpUdpHeaderOffset + 16], &pfBuffer[fTcpUdpHeaderOffset], 2,  (uint8 *) &fSourceTcpUdpPort, 2);
    }

    pfBuffer[fTcpUdpHeaderOffset] = (uint8) ((fSourceTcpUdpPort >> 8) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 1] = (uint8) (fSourceTcpUdpPort & 0xff);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;

}

void BcmEthernetPacket::SourceTcpUdpPort(uint16 sourcePort)
{
    // Update my member.
    fSourceTcpUdpPort = sourcePort;

    // Update the buffer
    pfBuffer[fTcpUdpHeaderOffset] = (uint8) ((fSourceTcpUdpPort >> 8) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 1] = (uint8) (fSourceTcpUdpPort & 0xff);

    // I need to update the TCP/UDP checksum!

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


void BcmEthernetPacket::DestTcpUdpPort(uint16 destPort)
{
    // Update my member.
    fDestTcpUdpPort = destPort;

    // Update the buffer
    pfBuffer[fTcpUdpHeaderOffset + 2] = (uint8) ((fDestTcpUdpPort >> 8) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 3] = (uint8) (fDestTcpUdpPort & 0xff);

    // I need to update the TCP/UDP checksum!

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}

void BcmEthernetPacket::DestTcpUdpPortFixup(uint16 destPort)
{
    // Update my member.
    fDestTcpUdpPort = destPort;

    // Update the buffer
    // We also potentially need to update the TcpUdp checksum
    if (IpProtocolTypeParsed() == kUdp)
    {
        // if we are udp, and checksum is zero, then no reason to update
        if (pfBuffer[fTcpUdpHeaderOffset + 6] != 0 || pfBuffer[fTcpUdpHeaderOffset + 7])
        {
            // I need to update the TCP/UDP checksum using the fixup method 
            ChecksumFixup(&pfBuffer[fTcpUdpHeaderOffset + 6], &pfBuffer[fTcpUdpHeaderOffset+2], 2,  (uint8 *) &fDestTcpUdpPort, 2);
        }
    }
    else
    {
        // I need to update the TCP/UDP checksum using the fixup method 
        ChecksumFixup(&pfBuffer[fTcpUdpHeaderOffset + 16], &pfBuffer[fTcpUdpHeaderOffset+2], 2,  (uint8 *) &fDestTcpUdpPort, 2);
    }

    pfBuffer[fTcpUdpHeaderOffset+ 2]  = (uint8) ((fDestTcpUdpPort >> 8) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 3] = (uint8) (fDestTcpUdpPort & 0xff);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;

}

// used to zero out the UDP checksum field. 
void BcmEthernetPacket::UdpChecksum(uint16 checksum)
{
    // Update the buffer
    pfBuffer[fTcpUdpHeaderOffset + 6] = (uint8) ((checksum >> 8) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 7] = (uint8) (checksum & 0xff);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}

uint16 BcmEthernetPacket::UdpChecksum(void) const
{
	return ( (pfBuffer[fTcpUdpHeaderOffset + 6] << 8) | pfBuffer[fTcpUdpHeaderOffset + 7]  );
}

// Helper method to check the UDP Header checksum.  Before calling this
// method, you must call ParseLlc() and ParseIp() (verifying that this is
// a UDP/IP packet), but you don't have to call ParseTcpUdp().
//
// Note that the UDP checksum covers the UDP header, the UDP payload, and
// a pseudo header for some IP header fields.  Because the checksum covers
// the payload, it can be time consuming!
//
// If the sender didn't calculate a checksum (value == 0), then this will
// return true.
//
// Parameters:  None.
//
// Returns:
//      true if the UDP checksum is valid.
//      false if the UDP checksum is not valid.
//
bool BcmEthernetPacket::UdpChecksumIsValid(void) const
{
    // Get a pointer to the UDP header.
    // If the checksum is 0, then bail early.
    if ((pfBuffer[fTcpUdpHeaderOffset + 6] == 0) && 
        (pfBuffer[fTcpUdpHeaderOffset + 7] == 0))
    {
        return true;
    }

    // Call the helper to do the actual checksum calculations.  If the returned
    // value is 0, then it checks out.
    if (CalculateUdpChecksum(pfBuffer + fIpHeaderOffset,
                             pfBuffer + fTcpUdpHeaderOffset) == 0)
    {
        return true;
    }

    return false;
}

// Causes the UDP checksum to be calculated and stored.
void BcmEthernetPacket::UpdateUdpChecksum(void)
{
    uint16 checkSum;

    // Clear the existing checksum.
    pfBuffer[fTcpUdpHeaderOffset + 6] = 0;
    pfBuffer[fTcpUdpHeaderOffset + 7] = 0;

    // Calculate a new checksum.
    checkSum = CalculateUdpChecksum(pfBuffer + fIpHeaderOffset, 
                                    pfBuffer + fTcpUdpHeaderOffset);

    // What if the result is 0xffff?  The one's complement would be 0, which
    // is interprited to mean that the UDP checksum was not calculated.  In this
    // case we need to use 0xffff...
    if (checkSum == 0)
    {
        checkSum = 0xffff;
    }

    // Store the checksum.
    pfBuffer[fTcpUdpHeaderOffset + 6] = (uint8) ((checkSum >> 8) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 7] = (uint8) (checkSum & 0xff);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


// Helper function that calculates the first part of the TCP and UDP checksum
// over the so-called pseudo-header, which includes the source/dest IP
// addresses, the protocol, and a 16-bit value that is the UDP length for UDP
// packets, or ???? for TCP packets.
//
// Parameters:
//      pIpHeader - pointer to the beginning of the IP header.
//      lastWord - the value to be used as the last 16-bit field in the
//                 pseudo-header.
//
// Returns:
//      The 32-bit sum of the pseudo-header.
//
static uint32 CalculatePseudoHeaderChecksum(const uint8 *pIpHeader, 
                                            uint16 lastWord)
{
    uint32 totalValue = 0;
    uint16 *pBuffer16 = (uint16 *) pIpHeader;

    // Add values for the source IP address.
    totalValue += NetworkToHost(pBuffer16[6]);
    totalValue += NetworkToHost(pBuffer16[7]);

    // Add values for the dest IP address.
    totalValue += NetworkToHost(pBuffer16[8]);
    totalValue += NetworkToHost(pBuffer16[9]);

    // Add the value for the protocol.
    totalValue += pIpHeader[9];

    // Add the last 16-bit value.
    totalValue += lastWord;

    return totalValue;
}


// Helper function that calculates the first part of the TCP and UDP checksum
// over the IPv6 pseudo-header, which includes the source/dest IPv6
// addresses, the next header, and a 32-bit value that is the upper-layer
// packet length (e.g. the length of the UDP header plus UDP payload data).
//
// Parameters:
//      pIpHeader - pointer to the beginning of the IP header.
//      upperLayerLength - the value to be used as the 32-bit upper-layer
//                         packet length.
//
// Returns:
//      The 32-bit sum of the pseudo-header.
//
static uint32 CalculateIpV6PseudoHeaderChecksum(const uint8 *pIpHeader, 
                                                uint32 upperLayerLength)
{
    unsigned int offset;
    uint32 totalValue = 0;
    uint16 *pBuffer16 = (uint16 *) pIpHeader;

    // Add values for the source and dest IP addresses.
    for (offset = (8 / 2); offset < (40 / 2); offset++)
    {
        totalValue += NetworkToHost(pBuffer16[offset]);
    }

    // Add the upper-layer packet length value.
    totalValue += (upperLayerLength >> 16);
    totalValue += (upperLayerLength & 0xffff);

    // Add the value for the next header.
    totalValue += pIpHeader[6];

    return totalValue;
}


// Helper function that calculates the UDP checksum value.
//
// If you want to calculate the checksum in order to transmit the packet, then
// you need to set the checksum bytes in the header to 0 before calling this
// function, then store the value returned in that location.
//
// If you want to verify an existing checksum, then just call this method with
// the existing checksum.  If the value returned is 0, then the checksum is
// valid; otherwise, the checksum is not valid.  Note that most documentation
// says that the verified checksum should be 0xffff, but since we're taking the
// one's complement, it will wind up being 0 if it is valid.
//
// Parameters:
//      pIpHeader - pointer to the beginning of the IP header.
//      pUdpHeader - pointer to the beginning of the UDP header.
//
// Returns:
//      The UDP checksum value.
//
static uint16 CalculateUdpChecksum(const uint8 *pIpHeader, 
                                   const uint8 *pUdpHeader)
{
    uint32 totalValue;
    uint16 *pBuffer16 = (uint16 *) pUdpHeader;
    uint16 udpLength, checkSum;
    uint8 ipVersion;

    // Start off with the pseudo-header checksum, using the UDP length field as
    // the last 16-bit value.
    udpLength = NetworkToHost(pBuffer16[2]);

    // Read the IP version; the pseudo-header is different for IPv4 and IPv6.
    ipVersion = (pIpHeader[0] >> 4) & 0x0f;

    if (ipVersion == 4)
    {
        totalValue = CalculatePseudoHeaderChecksum(pIpHeader, udpLength);
    }
    else
    {
        totalValue = CalculateIpV6PseudoHeaderChecksum(pIpHeader, udpLength);
    }

    // Iterate over all full 16-bit words, spanning the UDP header and the
    // payload.
    while (udpLength > 1)
    {
        totalValue += NetworkToHost(*pBuffer16);
        pBuffer16++;

        udpLength -= 2;
    }

    // If there's an odd byte at the end, then pad with 0 and sum.
    if (udpLength == 1)
    {
        uint16 value = NetworkToHost(*pBuffer16);

        value &= 0xff00;

        totalValue += value;
    }

    // Now we need to fold the accumulator down to 16 bits.
    totalValue = (totalValue & 0xffff) + ((totalValue >> 16) & 0xffff);
    totalValue += ((totalValue >> 16) & 0xffff);

    // Bring this down to 16 bits.
    checkSum = (uint16) (totalValue & 0xffff);

    // Returns the 1's complement.
    return ~checkSum;
}


// Returns a pointer to the first byte of the payload following the UDP
// header.  Note that if this isn't a UDP/IP packet (or if the IP header
// hasn't been parsed yet), then this returns NULL!
const uint8 *BcmEthernetPacket::UdpPdu(void) const
{
    uint8 *pPayload = NULL;

    if (((fWasParsedMask & kIpWasParsed) && (fIpProtocolType == kUdp)) ||
        ((fWasParsedMask & kIpV6WasParsed) && (fIpV6NextHeader == kUdp)))
    {
        // Get a pointer to the beginning of the packet.
        pPayload = (uint8 *) pfEthernetPacket->pDataBufferHead->pvStart;

        // Adjust it to point past the IP header.
        pPayload += fTcpUdpHeaderOffset;

        // Adjust it to point past the UDP header (which is always 8 bytes).
        pPayload += 8;
    }

    return pPayload;
}



// Same as above, but allows the PDU to be modified.  Note that you are
// responsible for updating the UDP checksum after the PDU is modified (you
// can just set it to 0, since the checksum is not required for UDP).
uint8 *BcmEthernetPacket::UdpPduWritable(void)
{
    // Just use the implementation from the constipated version, then cast to
    // non-const.
    return (uint8 *) UdpPdu();
}


// Returns a pointer to the first byte of the TCP
// header.  Note that if this isn't a TCP/IP packet (or if the IP header
// hasn't been parsed yet), then this returns NULL!
const uint8 *BcmEthernetPacket::TcpHeader(void) const
{
    uint8 *pPayload = NULL;

    if ((fWasParsedMask & kIpWasParsed) && (fIpProtocolType == kTcp))
    {
        // Get a pointer to the beginning of the packet.
        pPayload = (uint8 *) pfEthernetPacket->pDataBufferHead->pvStart;

        // Adjust it to point past the IP header.
        pPayload += fTcpUdpHeaderOffset;
    }

    return pPayload;
}

void BcmEthernetPacket::TcpSequenceNumber(uint32 sequenceNumber)
{
    // Update my member.
    fTcpSequenceNumber = sequenceNumber;

    // Update the buffer
    pfBuffer[fTcpUdpHeaderOffset + 4] = (uint8) ((sequenceNumber >> 24) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 5] = (uint8) ((sequenceNumber >> 16) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 6] = (uint8) ((sequenceNumber >> 8) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 7] = (uint8) (sequenceNumber & 0xff);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}

void BcmEthernetPacket::TcpSequenceNumberFixup(uint32 sequenceNumber)
{
    // Update my member.
    fTcpSequenceNumber = sequenceNumber;

    // Update the buffer
     // I need to update the TCP/UDP checksum using the fixup method 
    ChecksumFixup(&pfBuffer[fTcpUdpHeaderOffset + 16], &pfBuffer[fTcpUdpHeaderOffset + 4], 4,  (uint8 *) &fTcpSequenceNumber,4);

    pfBuffer[fTcpUdpHeaderOffset + 4] = (uint8) ((sequenceNumber >> 24) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 5] = (uint8) ((sequenceNumber >> 16) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 6] = (uint8) ((sequenceNumber >> 8) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 7] = (uint8) (sequenceNumber & 0xff);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}

void BcmEthernetPacket::TcpAckNumber(uint32 ackNumber)
{
    // Update my member.
    fTcpAckNumber = ackNumber;

    // Update the buffer
    pfBuffer[fTcpUdpHeaderOffset + 8] = (uint8) ((ackNumber >> 24) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 9] = (uint8) ((ackNumber >> 16) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 10] = (uint8) ((ackNumber >> 8) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 11] = (uint8) (ackNumber & 0xff);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}

void BcmEthernetPacket::TcpAckNumberFixup(uint32 ackNumber)
{
    // Update my member.
    fTcpAckNumber = ackNumber;

    // Update the buffer
     // I need to update the TCP/UDP checksum using the fixup method 
    ChecksumFixup(&pfBuffer[fTcpUdpHeaderOffset + 16], &pfBuffer[fTcpUdpHeaderOffset + 8], 4,  (uint8 *) &fTcpAckNumber,4);

    pfBuffer[fTcpUdpHeaderOffset + 8] = (uint8) ((ackNumber >> 24) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 9] = (uint8) ((ackNumber >> 16) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 10] = (uint8) ((ackNumber >> 8) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 11] = (uint8) (ackNumber & 0xff);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}

void BcmEthernetPacket::TcpHeaderLengthBytes(unsigned int lengthBytes)
{
    // Update my member.
    fTcpHeaderLengthBytes = lengthBytes;

    // Convert from bytes to 32-bit words, and shift the value to the correct
    // bit position.
    lengthBytes /= 4;
    lengthBytes <<= 4;

    // Update the buffer
    pfBuffer[fTcpUdpHeaderOffset + 12] = (uint8) (lengthBytes & 0xff);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


void BcmEthernetPacket::TcpFlags(uint8 flags)
{
    // Update my member.
    fTcpFlags = flags;

    // Update the buffer
    pfBuffer[fTcpUdpHeaderOffset + 13] = flags;

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


void BcmEthernetPacket::SetTcpFlag(TcpFlagBits flag)
{
    TcpFlags(fTcpFlags | flag);
}


void BcmEthernetPacket::ClearTcpFlag(TcpFlagBits flag)
{
    TcpFlags(fTcpFlags & (~flag));
}


void BcmEthernetPacket::TcpWindowSize(uint16 windowSize)
{
    // Update my member.
    fTcpWindowSize = windowSize;

    // Update the buffer
    pfBuffer[fTcpUdpHeaderOffset + 14] = (uint8) ((windowSize >> 8) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 15] = (uint8) (windowSize & 0xff);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


uint16 BcmEthernetPacket::TcpChecksum(void) const
{
    // Get a pointer to the actual buffer we are parsing.
    return NetworkToHost(*((uint16 *) (pfBuffer + fTcpUdpHeaderOffset + 16)));
}


void BcmEthernetPacket::TcpChecksum(uint16 checksum)
{
    // Update the buffer
    pfBuffer[fTcpUdpHeaderOffset + 16] = (uint8) ((checksum >> 8) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 17] = (uint8) (checksum & 0xff);
}


void BcmEthernetPacket::TcpUrgentPointer(uint16 urgentPointer)
{
    // Update my member.
    fTcpUrgentPointer = urgentPointer;

    // Update the buffer
    pfBuffer[fTcpUdpHeaderOffset + 18] = (uint8) ((urgentPointer >> 8) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 19] = (uint8) (urgentPointer & 0xff);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


// Returns a pointer to the first byte of the payload following the TCP
// header.  Note that if this isn't a TCP/IP packet (or if the IP header
// hasn't been parsed yet), then this returns NULL!
const uint8 *BcmEthernetPacket::TcpPdu(void) const
{
    uint8 *pPayload = NULL;

    if ((fWasParsedMask & kIpWasParsed) && (fIpProtocolType == kTcp))
    {
        // Get a pointer to the beginning of the packet.
        pPayload = (uint8 *) pfEthernetPacket->pDataBufferHead->pvStart;

        // Adjust it to point past the IP header.
        pPayload += fTcpUdpHeaderOffset;

        // Adjust it to point past the TCP header.
        pPayload += fTcpHeaderLengthBytes;
    }

    return pPayload;
}



// Same as above, but allows the PDU to be modified.  Note that you are
// responsible for updating the TCP checksum after the PDU is modified.
uint8 *BcmEthernetPacket::TcpPduWritable(void)
{
    // Just use the implementation from the constipated version, then cast to
    // non-const.
    return (uint8 *) TcpPdu();
}


void BcmEthernetPacket::IcmpIdentifierFixup(uint16 identifier)
{
    unsigned int icmpHeaderOffset = fIpHeaderOffset + fIpHeaderLengthBytes;

    // Update my member.
    fIcmpIdentifier = identifier;

    // Update the checksum for the ICMP message
    ChecksumFixup(&pfBuffer[icmpHeaderOffset + 2], &pfBuffer[icmpHeaderOffset+4], 2,  (uint8 *) &fIcmpIdentifier, 2);

 
 
    pfBuffer[icmpHeaderOffset + 4] = (uint8) ((fIcmpIdentifier >> 8) & 0xff);
    pfBuffer[icmpHeaderOffset + 5] = (uint8) (fIcmpIdentifier & 0xff);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;

}

void BcmEthernetPacket::IcmpSequenceNumberFixup(uint16 sequenceNumber)
{
    unsigned int icmpHeaderOffset = fIpHeaderOffset + fIpHeaderLengthBytes;

    // Update my member.
    fIcmpSequenceNumber = sequenceNumber;

    // Update the checksum for the ICMP message
    ChecksumFixup(&pfBuffer[icmpHeaderOffset + 2], &pfBuffer[icmpHeaderOffset+6], 2,  (uint8 *) &fIcmpSequenceNumber, 2);

 
 
    pfBuffer[icmpHeaderOffset + 6] = (uint8) ((fIcmpSequenceNumber >> 8) & 0xff);
    pfBuffer[icmpHeaderOffset + 7] = (uint8) (fIcmpSequenceNumber & 0xff);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;

}

// Pre-parses the HTTP header so that you can call the ...Parsed() accessors.
//
// Returns false if this is not a HTTP packet.
bool BcmEthernetPacket::ParseHttp(void)
{
    // Only do this if we have not already parsed the HTTP header.
    if ((fWasParsedMask & kHttpWasParsed) == 0)
    {
        ParseHttpImpl();
    }

    // Return true if this is an HTTP packet.
    //
    // We can only call this if HTTP packet was parsed successfully,
    if (fWasParsedMask & kHttpWasParsed)
    {
		return true;
    }

    return false;
}

// Pre-parses the HTTP header so that you can call the ...Parsed() accessors.
//
// Returns false if this is not a HTTP packet.
void BcmEthernetPacket::ParseHttpImpl(void)
{
	// Make sure the TCP/UDP port numbers are parsed.  If not, then parse them;
    // if this isn't a HTTP packet, then we can bail.
    if ((ParseTcp() == false) || 
		(!( (DestTcpUdpPortParsed() == 80) || (DestTcpUdpPortParsed() == 8080) ||
		  (SourceTcpUdpPortParsed() == 80) || (SourceTcpUdpPortParsed() == 8080) )))
    {
        return;
    }

	// Set HTTP packet length.
	fHttpTotalLengthBytes=0;

    // determine the size of the HTTP request...
	if ( TcpPdu() != NULL )
	{
		fHttpTotalLengthBytes = PacketLength() - ( (char*)TcpPdu() - (char*)PacketStruct()->pDataBufferHead->pvStart);
	}

	// Now get the method type.
	fHttpMethodType = 0;
	fHttpMethodTypePointer = NULL;

	// Only proceed it there is an HTTP payload...
	// TCP could have some padding but not data
    if(fHttpTotalLengthBytes > kHttpMinGetRequestSize)
    {
		// Set HTTP Method.
		fHttpMethodTypePointer = (char*)memfind_r ((const void*) TcpPdu(), /* Block containing data */
				fHttpTotalLengthBytes, /* Size of block in bytes */
				(void *)"GET", /* Pattern to search for */
				strlen("GET"));
		
		// Currently we only search for method GET.
		if( fHttpMethodTypePointer != NULL )
		{
			fHttpMethodType = kGet;
		}
	}

    // Remember that we have parsed the necessary HTTP fields now.
    fWasParsedMask |= kHttpWasParsed;
}

 // Accessors for the HTTP packet length (in bytes).
unsigned int BcmEthernetPacket::HttpTotalLengthBytesParsed(void) const
{
	return fHttpTotalLengthBytes;
}

void BcmEthernetPacket::HttpTotalLengthBytes(unsigned int lengthBytes)
{
	fHttpTotalLengthBytes = lengthBytes;
}

// Accessor for the HTTP Method Type
uint8 BcmEthernetPacket::HttpMethodTypeParsed(void) const
{
	return fHttpMethodType;
}

char *BcmEthernetPacket::HttpMethodTypePointer(void) const
{
	return fHttpMethodTypePointer;
}

// Helper function that calculates the TCP checksum value.
//
// If you want to calculate the checksum in order to transmit the packet, then
// you need to set the checksum bytes in the header to 0 before calling this
// function, then store the value returned in that location.
//
// If you want to verify an existing checksum, then just call this method with
// the existing checksum.  If the value returned is 0, then the checksum is
// valid; otherwise, the checksum is not valid.  Note that most documentation
// says that the verified checksum should be 0xffff, but since we're taking the
// one's complement, it will wind up being 0 if it is valid.
//
// Parameters:
//      pIpHeader - pointer to the beginning of the IP header.
//      pUdpHeader - pointer to the beginning of the UDP header.
//
// Returns:
//      The UDP checksum value.
//
static uint16 CalculateTcpChecksum(const uint8 *pIpHeader, 
                                   const uint8 *pTcpHeader)
{
    uint32 totalValue;
    uint16 *pBuffer16 = (uint16 *) pTcpHeader;
    uint16 tcpLength, checkSum, ipTotalLengthBytes, ipHeaderLengthBytes;

    // Start off with calculating the TCP length
    ipHeaderLengthBytes = pIpHeader[0] & 0x0f;
    ipHeaderLengthBytes *= 4;
    ipTotalLengthBytes = NetworkToHost(*((uint16 *) (pIpHeader + 2)));

    tcpLength = ipTotalLengthBytes - ipHeaderLengthBytes;
	
	// Proceed with the pseudo-header checksum, using the TCP length field as
    // the last 16-bit value.
    totalValue = CalculatePseudoHeaderChecksum(pIpHeader, tcpLength);

    // Iterate over all full 16-bit words, spanning the TCP header and the
    // payload.
    while (tcpLength > 1)
    {
        totalValue += NetworkToHost(*pBuffer16);
        pBuffer16++;

        tcpLength -= 2;
    }

    // If there's an odd byte at the end, then pad with 0 and sum.
    if (tcpLength == 1)
    {
        uint16 value = NetworkToHost(*pBuffer16);

        value &= 0xff00;

        totalValue += value;
    }

    // Now we need to fold the accumulator down to 16 bits.
    totalValue = (totalValue & 0xffff) + ((totalValue >> 16) & 0xffff);
    totalValue += ((totalValue >> 16) & 0xffff);

    // Bring this down to 16 bits.
    checkSum = (uint16) (totalValue & 0xffff);

    // Returns the 1's complement.
    return ~checkSum;
}

// Helper method to check the TCP Header checksum.  Before calling this
// method, you must call ParseLlc() and ParseIp() (verifying that this is
// a TCP/IP packet), but you don't have to call ParseTcpUdp().
//
// Note that the TCP checksum covers the TCP header, the TCP payload, and
// a pseudo header for some IP header fields.  Because the checksum covers
// the payload, it can be time consuming!
//
// If the sender didn't calculate a checksum (value == 0), then this will
// return true.
//
// Parameters:  None.
//
// Returns:
//      true if the TCP checksum is valid.
//      false if the TCP checksum is not valid.
//
bool BcmEthernetPacket::TcpChecksumIsValid(void) const
{
    // Get a pointer to the frame.
    uint8 *pBuffer = (uint8 *) pfEthernetPacket->pDataBufferHead->pvStart;

    // Call the helper to do the actual checksum calculations.  If the returned
    // value is 0, then it checks out.
    if (CalculateTcpChecksum(pBuffer + fIpHeaderOffset, pBuffer + fTcpUdpHeaderOffset) == 0)
    {
        return true;
    }

    return false;
}

// Causes the TCP checksum to be calculated and stored.
void BcmEthernetPacket::UpdateTcpChecksum(void)
{
    uint16 checkSum;

    // Clear the existing checksum.
    pfBuffer[fTcpUdpHeaderOffset + 16] = 0;
    pfBuffer[fTcpUdpHeaderOffset + 17] = 0;

    // Calculate a new checksum.
    checkSum = CalculateTcpChecksum(pfBuffer + fIpHeaderOffset, 
                                    pfBuffer + fTcpUdpHeaderOffset);

    // What if the result is 0xffff?  The one's complement would be 0, which
    // is interprited to mean that the UDP checksum was not calculated.  In this
    // case we need to use 0xffff...
    if (checkSum == 0)
    {
        checkSum = 0xffff;
    }

    // Store the checksum.
    pfBuffer[fTcpUdpHeaderOffset + 16] = (uint8) ((checkSum >> 8) & 0xff);
    pfBuffer[fTcpUdpHeaderOffset + 17] = (uint8) (checkSum & 0xff);

    fBcmPacketId = ++gGlobalBcmPacketId;
    fPacketChanged = true;
}


