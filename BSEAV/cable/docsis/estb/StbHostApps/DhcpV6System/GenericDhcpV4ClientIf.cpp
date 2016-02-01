//****************************************************************************
//
// Copyright (c) 2009 Broadcom Corporation
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
//  $Id$
//
//  Filename:       GenericDhcpV4ClientIf.cpp
//  Author:         Henry Shi
//  Creation Date:  Aug 14, 2009
//
//****************************************************************************

// Set this to 1 to enable simulated Reconfigure support (for testing when
// the DHCP server doesn't support it).
#define RECONFIGURE_SIM_SUPPORT 0

//********************** Include Files ***************************************

// My api and definitions...
#include "GenericDhcpV4ClientIf.h"

#if IPHALIF_ENABLED
#include "BaseIpHalIf.h"
#endif
#include "DhcpPacket.h"
#include "DhcpClientLease.h"

#include "SocketFactory.h"
#include "BcmSocket.h"

#include <stdlib.h>

ETHERNET_PACKET* AllocatePacket( char *buf, uint32 length )
{          
	char			*pPayload = NULL;
	DATA_BUFFER		*pDataBuffer = NULL;
	ETHERNET_PACKET *pEthernetPacketBuffer = NULL;

	// Send this packet to final destination such as the cablecard
	// Now wrap the data buffer in ETHERNET_PACKET struct to pass to eSTB.
	//
	// Allocate memory for the data structs.
	//
	// IMPORTANT: eSTB is RESPONSIBLE for releasing the buffers by calling
	//			  DsgFreeEthernetPkt()
	//
	
	// If 'buf' is NULL then the caller wants us to create a buffer of size 'length'.
	pPayload = buf;
	if( pPayload == NULL )
	{
		if( (pPayload = (char*)malloc( length)) == NULL)
		{
			gLogMessageRaw << " Failed to allocate ethernet buffer." << endl;
			return NULL;
		}

		memset(pPayload, 0, length);
	}

	if( ( pDataBuffer = (DATA_BUFFER *) malloc( sizeof( DATA_BUFFER )) ) == NULL)
	{
		// Ethernet driver will free the buffer 'buf' upon return.
		return NULL;
	}

	if( ( pEthernetPacketBuffer = (ETHERNET_PACKET *) malloc( sizeof( ETHERNET_PACKET )) ) == NULL)
	{
		free (pDataBuffer);

		// Ethernet driver will free the buffer 'buf' upon return.
		return NULL;
	}

	// reset buffer
	memset(pDataBuffer, 0, sizeof(pDataBuffer));
	memset(pEthernetPacketBuffer, 0, sizeof(pEthernetPacketBuffer));

	// Set DATA_BUFFER struct
	pDataBuffer->pvStart = (PVOID)pPayload; //buf;
	pDataBuffer->uiLen   = length;

	//Fill in the starting data buffer pointer address for the
	//Ethernet packet we are going to use for this data.
	pEthernetPacketBuffer->pDataBufferHead = pDataBuffer;

	//Use current FreeDataBuffPtr as last buffer in EthPktChain.
	pEthernetPacketBuffer->pDataBufferTail = pDataBuffer;
	pEthernetPacketBuffer->pNext = NULL;

	return pEthernetPacketBuffer;
}

  
//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Local Functions *************************************

//********************** Class Method Implementations ************************

BcmGenericDhcpV4ClientIf::BcmGenericDhcpV4ClientIf(BcmBaseIpHalIf &ipHalIf) :
	BcmDhcpClientIf(ipHalIf)
{
    fMessageLogSettings.SetModuleName("BcmGenericDhcpV4ClientIf");

	//Debug(true);

    {
        // Generate the instance name for display purposes.  My string can be
        // temporary because the message log settings creates its own memory.
        char *pInstanceName = new char[128];

        strcpy(pInstanceName, "Generic DHCPv4 ClientIf for ");
#if IPHALIF_ENABLED
        strcat(pInstanceName, ipHalIf.Description());
#endif
        fMessageLogSettings.SetInstanceName(pInstanceName);

        fMessageLogSettings.Register();
        delete pInstanceName;
    }
}


BcmGenericDhcpV4ClientIf::~BcmGenericDhcpV4ClientIf()
{
}


ostream & BcmGenericDhcpV4ClientIf::Print(ostream &outputStream) const
{
    #if !(BCM_REDUCED_IMAGE_SIZE)

    outputStream 
        << fMessageLogSettings.GetInstanceName() 
	<< endl;

    #endif
    
    return outputStream;
}


void BcmGenericDhcpV4ClientIf::Debug(bool enable)
{
    uint32 severities = fMessageLogSettings.GetEnabledSeverities();

    // All the debugging stuff is tied to info messages, so this is the only
    // bit I need to twiddle.
    if (enable)
    {
        severities |= BcmMessageLogSettings::kInformationalMessages;
    }
    else
    {
        severities &= ~BcmMessageLogSettings::kInformationalMessages;
    }

    fMessageLogSettings.SetEnabledSeverities(severities);
}

bool BcmGenericDhcpV4ClientIf::SendDhcpPacket(BcmDhcpClientLease *pLease, 
                        BcmDhcpLeaseSettings *pSettings)
{
    bool result;
    uint16 numberOfSeconds;

    CallTrace("BcmDhcpClientIf", "SendDhcpPacket");
    BcmOctetBuffer buffer(0);
    BcmEthernetPacket packet;
    BcmIpAddress ipAddress;
	int buflen;
    // Get the IP HalIf that I'm associated with to allocate a buffer
    // for me.  Make it big enough for a large packet.
	//  if (!(IpHalIf()->AllocatePacket(&packet, 1518)))
	//  {
	//      gWarningMsg(fMessageLogSettings, "SendDhcpPacket")
	//         << "Failed to allocate a h/w buffer for the packet; can't send it!" << endl;

	//     return false;
	// }
	BcmIpAddress destIpAddress(0xff, 0xff, 0xff, 0xff);


	ETHERNET_PACKET* pEthernetBuffer = NULL;

    // Account for the 14 byte LLC header, the 40 byte IPv6 header, the 8 byte
    // UDP header, and the 4 byte CRC32, as well as the size of the DHCPv6
    // payload.
	unsigned int dhcpPacketSize = 1400;
	#warning "-DHCPv6 MODS - Pre-allocated DHCPv6 payload size - 1400 Bytes"
    unsigned int bufferSize = 14 + 40 + 8 + dhcpPacketSize + 4; //pDhcpPacket->Size() + 4;

    // Get the IP HalIf that I'm associated with to allocate a buffer
    // for me.
    //if (!pfIpHalIf->AllocatePacket(&packet, bufferSize))
	pEthernetBuffer = AllocatePacket(NULL, bufferSize);
	if (!pEthernetBuffer)
    {
        gLogMessageRaw //gWarningMsg(fMessageLogSettings, "SendDhcpV6Packet")
            << "Failed to allocate a h/w buffer for the packet ("
            << bufferSize << " bytes); can't send it!" << endl;

        return false;
    }

    // Store the allocated packet in the BcmEthernetPacket object.
    packet.SetPacket(pEthernetBuffer);



    // Let's fill in the LLC/IP/UDP headers.  If the lease state is renewing,
    // releasing or declining, then I need to use the unicast MAC DA of the
    // DHCP server; otherwise, I use the broadcast MAC DA.
    if ((pLease->State() == BcmDhcpClientLease::kStateRenewing) ||
        (pLease->State() == BcmDhcpClientLease::kStateDeclining) ||
        (pLease->State() == BcmDhcpClientLease::kStateReleasingSendPacket))
    {
        packet.MacDa(pLease->ServerMacAddresss());
    }
    else
    {
        packet.MacDa(kBroadcastMac);
    }

    // The MAC SA is always my IP stack's address.
    packet.MacSa(IpHalIf()->MacAddress());

    // We always send EtherII/IP.
    packet.SetFrameFormat(BcmEthernetPacket::kEthernetII);
    packet.EtherType(BcmEthernetPacket::kEtherTypeIp);

    // Tell the packet that this portion of the header is done.
    packet.LlcGenerated();

    // We always sent IPv4 and a standard 20-byte header.
    packet.IpVersion(4);
    packet.IpHeaderLengthBytes(20);

    // No ToS is requested.
    packet.IpTypeOfService(0);

    // The length will be filled in later.  I need to know how long the payload
    // is...
    //packet.IpTotalLengthBytes(?);

    // We can pick whatever Id we want; it doesn't matter.
    packet.Identification(0xdead);

    // No flags or fragments.
    packet.Flags(0);
    packet.FragmentOffset(0);

    // Use a standard TTL value.
    packet.TimeToLive(128);

    // Payload is UDP.
    packet.IpProtocolType(BcmEthernetPacket::kUdp);

    // The checksum is filled in on-the fly.  We don't fill it in.
    //packet.IpHcs(?);

    // If we are currently declining, renewing, releasing or rebinding, then
    // the source address is ours; otherwise the source address is 0.0.0.0.
    if ((pLease->State() == BcmDhcpClientLease::kStateRenewing) ||
        (pLease->State() == BcmDhcpClientLease::kStateRebinding) ||
        (pLease->State() == BcmDhcpClientLease::kStateDeclining) ||
        (pLease->State() == BcmDhcpClientLease::kStateReleasingSendPacket))
    {
        // Pull the requested address from the options.
        pSettings->GetOption(kDhcpRequestedIpAddress, ipAddress);

        packet.SourceIpAddress(ipAddress);
    }
    else
    {
        packet.SourceIpAddress(kAllZerosIp);
    }

    // If we are currently renewing, releasing or declining, then the dest
    // address is the DHCP server; otherwise the dest address is broadcast.
    if ((pLease->State() == BcmDhcpClientLease::kStateRenewing) ||
        (pLease->State() == BcmDhcpClientLease::kStateDeclining) ||
        (pLease->State() == BcmDhcpClientLease::kStateReleasingSendPacket))
    {
        // Pull the server's address from the options.
        pLease->ServerLeaseSettings().GetOption(kDhcpServerIdentifier, ipAddress);

        packet.DestIpAddress(ipAddress);
    }
    else
    {
        packet.DestIpAddress(kBroadcastIp);
    }

    // Tell the packet that this portion of the header is done.
    packet.IpGenerated();

    // We send to the DHCP server's port number (67) and from the DHCP client's
    // port number (68).
    packet.SourceTcpUdpPort(68);    // UNFINISHED - enum?  Allow to be configured?
    packet.DestTcpUdpPort(67);

    // The length will be filled in later.
    // The checksum will be calculated later.

    // Tell the packet that this portion of the header is done.
    packet.UdpGenerated();

    // Configure the DHCP packet object.
    pfDhcpPacket->DhcpLeaseSettings(pSettings);

    // Clamp the number of seconds in case it's very large.
    numberOfSeconds = (uint16) (pLease->CurrentTimeSec() & 0xffff);
    if (pLease->CurrentTimeSec() > 0xffff)
    {
        numberOfSeconds = 0xffff;
    }

    // For the decline and release message, this needs to be 0.
    if ((pLease->State() == BcmDhcpClientLease::kStateDeclining) ||
        (pLease->State() == BcmDhcpClientLease::kStateReleasingSendPacket))
    {
        numberOfSeconds = 0;
    }

    // Fill in the DHCP packet header info.  We can always set the same info;
    // the only thing that's variable is the XID, and that comes from the lease.
    pfDhcpPacket->PacketType(kDhcpBootRequest);
    pfDhcpPacket->Hops(0);
    pfDhcpPacket->Xid(pLease->Xid());
    pfDhcpPacket->Secs(numberOfSeconds);
    pfDhcpPacket->Flags(0);

    // This field will be 0.0.0.0 for Discover and Decline messages, and for
    // Request messages where the state is Selecting; it will be our IP address
    // for others.
    DhcpMessageTypeOption53 messageType = pSettings->GetDhcpMessageType();

    if ((messageType == kDhcpDiscover) || (messageType == kDhcpDecline))
    {
        pSettings->ClientIpAddress(kAllZerosIp);
    }
    else if ((messageType == kDhcpRequest) && 
             ((pLease->State() == BcmDhcpClientLease::kStateRequesting) || 
			 (pLease->State() == BcmDhcpClientLease::kStateRebooting)))
    {
        // RFC2131 isn't entirely clear on this, but testing has shown that
        // CNR behaves oddly if this isn't 0.0.0.0, so we'll do this.
        pSettings->ClientIpAddress(kAllZerosIp);
    }
    else
    {
        pSettings->GetOption(kDhcpRequestedIpAddress, ipAddress);

        pSettings->ClientIpAddress(ipAddress);
    }

    // These must always be 0.0.0.0
    pSettings->YourIpAddress(kAllZerosIp);
    pSettings->ServerIpAddress(kAllZerosIp);
    pSettings->RelayAgentIpAddress(kAllZerosIp);

    // These are always unused.
    pSettings->ServerName("");
    pSettings->FileName("");

    // Set the chaddr field to the MAC address of the IP stack.
    pSettings->ClientHardwareAddress(IpHalIf()->MacAddress());

    // For a DHCP Request packet in renewing or rebinding (or a Release packet),
    // we need to remove the requested IP address option.
    if ((messageType == kDhcpRelease) ||
        ((messageType == kDhcpRequest) && 
		(pLease->State() != BcmDhcpClientLease::kStateRequesting) && 
		(pLease->State() != BcmDhcpClientLease::kStateRebooting)))
    {
        pSettings->RemoveOption(kDhcpRequestedIpAddress);
    }

    // For DHCP Discover and Request packets, the client should include the
    // Maximum DHCP Message Size option (57), telling the server how large
    // of a DHCP packet we will accept.
    if ((messageType == kDhcpDiscover) || (messageType == kDhcpRequest))
    {
        // The maximum size we can support is an ethernet MTU (1518), minus
        // the LLC overhead (14 byte header and 4 byte CRC32).  The max message
        // size defaults to 576 bytes, and includes the IP, UDP, and DHCP
        // headers.
        pSettings->AddReplaceOption(kDhcpMaxDhcpMessageSize, (uint16) (1518 - 14 - 4));
    }

    gInfoMsg(fMessageLogSettings, "SendDhcpPacket")
        << "Generated DHCP settings:\n" << *pfDhcpPacket << endl;

    // Now overlay my buffer on the UDP payload.

    buffer.AdoptBuffer(packet.UdpPduWritable(), 1518 - 14 - 20 - 8, 0, false);

    result = pfDhcpPacket->WriteTo(buffer, &fMessageLogSettings);
    if (!result)
    {
        gErrorMsg(fMessageLogSettings, "SendDhcpPacket")
            << "BcmDhcpClientIf::SendDhcpPacket   failed to write DHCP packet to the buffer!" << endl;

        // Release the buffer back to the HAL.
        IpHalIf()->ReleasePacket(&packet);

        return false;
    }

    // Now that I know how big the UDP payload is, I can fill in the missing
    // length fields.
    packet.UdpLengthBytes(8 + buffer.BytesUsed());
    packet.IpTotalLengthBytes(20 + 8 + buffer.BytesUsed());

    // I have to calculate the UDP checksum manually.  The IP HCS is
    // automatically updated every time we change a field.
    packet.UpdateUdpChecksum();

    // Set the packet length in the underlying structure, not including the
    // 4-byte CRC..
    packet.SetPacketLength(14 + 20 + 8 + buffer.BytesUsed());
	buflen = 14 + 20 + 8 + buffer.BytesUsed();
    gInfoMsg(fMessageLogSettings, "SendDhcpPacket")
        << "Generated DHCP packet:\n" << packet << endl;

    // Send broadcast packets directly.   Send unicast packets using sockets. 
    //if( packet.IsBroadcast() == true )
	// Always use my interface 
    if( 1 ) 
    {
		//adopt buffer to start of IP header
    	//buffer.AdoptBuffer(packet.LlcPduWritable(), 1518 - 20 - 8, 0, false);
   		//buffer.SetBytesUsed(buflen);
		buffer.AdoptBuffer(packet.PduHeaderWritable(),1518, 0, false);
   		buffer.SetBytesUsed(buflen);

    	if (!result)
    	{
        	gErrorMsg(fMessageLogSettings, "SendDhcpPacket")
            	<< "BcmDhcpClientIf::SendDhcpPacket   failed to write DHCP packet to the buffer!" << endl;

        	// Release the buffer back to the HAL.
        	//IpHalIf()->ReleasePacket(&packet);
        	return false;
    	}
        //result = pfIpHalIf->ReceivePacket(&packet);
		//send out packet using Raw socket.
		result = IpHalIf()->TransmitViaNetworkSocket(buffer, destIpAddress, 67);
        if (!result)
        {
            gWarningMsg(fMessageLogSettings, "SendDhcpPacket")
                << "Failed to send packet!" << endl;

            // Release the buffer back to the HAL.
            IpHalIf()->ReleasePacket(&packet);

            return false;
        }
    }
    else
    {
        // Allocate socket used to communicate with the DHCP server for unicast packets.  
        // This is done because we cannot assume that the destination MAC address in our 
        // outgoing unicast packet (like Renew) will be the source MAC address in the Offer
        // packet.   In some configurations, the relay agent will send us the Offer
        // but it will expect the Renew to go directly to the DHCP server. 
        // 
        BcmIpAddress destIp = packet.DestIpAddressParsed();
        BcmIpAddress srcIp = packet.SourceIpAddressParsed();
        uint16 destPort = packet.DestTcpUdpPortParsed();
        uint16 srcPort = packet.SourceTcpUdpPortParsed();

        // Release the packet we allocated since we won't be sending it.  
        IpHalIf()->ReleasePacket(&packet);
        
        BcmSocket * pSocket;
        unsigned int ipStackNumber = IpHalIf()->InterfaceNumber();
        pSocket = BcmSocketFactory::NewSocket(AF_INET, SOCK_DGRAM, UDP, ipStackNumber);

        // This can fail if there aren't enough resources.
        if ( pSocket == NULL )
        {
            gWarningMsg(fMessageLogSettings, "SendDhcpPacket")
                << "Failed to allocate socket to send packet!" << endl;

            return false;
        }

        // bind to our IP address and source port 68
        pSocket->Bind( srcIp, srcPort );
        int status = pSocket->Sendto((char *)buffer.AccessBuffer(),
                                             buffer.BytesUsed(),
                                             0, 
                                             destIp, 
                                             destPort );

        delete pSocket;
        pSocket = NULL;

        if (status == -1 /*ERROR*/)
        {
            gErrorMsg(fMessageLogSettings, "Send")
                << "Sendto() failed to (" << destIp << ") on port (DHCP " 
                << (int)destPort << ")" << endl;

            return false;
        }
    }

    return true;

}

bool BcmGenericDhcpV4ClientIf::SendArpPacket(uint16 arpType, const BcmIpAddress &ipAddress)
{
    CallTrace("BcmDhcpClientIf", "SendArpPacket");

    bool result;
    BcmEthernetPacket packet;
    BcmOctetBuffer buffer(0);

    // Get the IP HalIf that I'm associated with to allocate a buffer
    // for me.  Make it big enough for an ARP packet, which is small.
    //if (!IpHalIf()->AllocatePacket(&packet, 64))
    //{
        //gWarningMsg(fMessageLogSettings, "SendArpPacket")
        //    << "Failed to allocate a h/w buffer for the packet; can't send it!" << endl;

        //return false;
    //}

	ETHERNET_PACKET* pEthernetBuffer = NULL;

    // Account for the 14 byte LLC header, the 40 byte IPv6 header, the 8 byte
    // UDP header, and the 4 byte CRC32, as well as the size of the DHCPv6
    // payload.
	unsigned int dhcpPacketSize = 1400;
	#warning "-DHCPv6 MODS - Pre-allocated DHCPv6 payload size - 1400 Bytes"
    unsigned int bufferSize = 14 + 40 + 8 + dhcpPacketSize + 4; //pDhcpPacket->Size() + 4;

    // Get the IP HalIf that I'm associated with to allocate a buffer
    // for me.
    //if (!pfIpHalIf->AllocatePacket(&packet, bufferSize))
	pEthernetBuffer = AllocatePacket(NULL, bufferSize);
	if (!pEthernetBuffer)
    {
        gLogMessageRaw //gWarningMsg(fMessageLogSettings, "SendDhcpV6Packet")
            << "Failed to allocate a h/w buffer for the packet ("
            << bufferSize << " bytes); can't send it!" << endl;

        return false;
    }
    // Store the allocated packet in the BcmEthernetPacket object.
    packet.SetPacket(pEthernetBuffer);

    // Let's fill in the LLC header.  I always use the broadcast address for
    // the MAC DA.
    packet.MacDa(kBroadcastMac);

    // The MAC SA is always my IP stack's address.
    packet.MacSa(IpHalIf()->MacAddress());

    // We always send EtherII/ARP.
    packet.SetFrameFormat(BcmEthernetPacket::kEthernetII);
    packet.EtherType(BcmEthernetPacket::kEtherTypeArp);

    // Tell the packet that this portion of the header is done.
    packet.LlcGenerated();

    // Fill in the ARP header.  There are 2 modes of operation for DHCP clients:
    //  - ARP request, where source protocol is all 0's and target protocol is
    //    the IP address.
    //  - ARP reply, where the source and target are me.
    packet.ArpHardwareType(BcmEthernetPacket::kArpHwTypeEthernet);
    packet.ArpProtocolType(BcmEthernetPacket::kEtherTypeIp);
    packet.ArpHardwareSize(6);
    packet.ArpProtocolSize(4);

    packet.ArpOpcode(arpType);

    // Always use my IP stack's MAC address as the source h/w address.
    packet.ArpSenderHardwareAddress(IpHalIf()->MacAddress());

    // If this is an ARP request, then I use 0.0.0.0 as the source protocol
    // address; otherwise, I use the IP address that was passed in.
    if (arpType == BcmEthernetPacket::kArpRequest)
    {
        packet.ArpSenderProtocolAddress(kAllZerosIp);
    }
    else
    {
        packet.ArpSenderProtocolAddress(ipAddress);
    }

    // If this is an ARP request, then I use all 0's as the target h/w address;
    // otherwise, I use my IP stack's address as the target.
    if (arpType == BcmEthernetPacket::kArpRequest)
    {
        packet.ArpTargetHardwareAddress(kAllZerosMac);
    }
    else
    {
        packet.ArpTargetHardwareAddress(IpHalIf()->MacAddress());
    }

    // Always use the specified IP address as the target.
    packet.ArpTargetProtocolAddress(ipAddress);

    // Tell the packet that this portion of the header is done.
    packet.ArpGenerated();

    // Set the packet length in the underlying structure, not including the
    // 4-byte CRC..
    packet.SetPacketLength(60);

    gInfoMsg(fMessageLogSettings, "SendArpPacket")
        << "Generated ARP Packet:\n" << packet << endl;

    //result = pfIpHalIf->ReceivePacket(&packet);
	//adopt buffer to start of IP header
    //buffer.AdoptBuffer(packet.LlcPdu(), 1518 - 20 - 8, 0, false);

	buffer.AdoptBuffer(packet.PduHeaderWritable(),1518, 0, false);
   	buffer.SetBytesUsed(60);

    // Send the packet using raw socket.
	// send out packet using Raw socket.
	result = IpHalIf()->TransmitViaNetworkSocket(buffer, ipAddress, 67);

    if (!result)
    {
        gWarningMsg(fMessageLogSettings, "SendArpPacket")
            << "Failed to send packet!" << endl;

        // Release the buffer back to the HAL.
        IpHalIf()->ReleasePacket(&packet);

        return false;
    }
 
    return true;

}


