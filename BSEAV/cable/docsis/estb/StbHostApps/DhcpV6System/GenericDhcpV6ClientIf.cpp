//****************************************************************************
//
// Copyright (c) 2008 Broadcom Corporation
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
//  Filename:       DhcpV6ClientIf.cpp
//  Author:         David Pullen
//  Creation Date:  June 12, 2006
//
//****************************************************************************

// Set this to 1 to enable simulated Reconfigure support (for testing when
// the DHCP server doesn't support it).
#define RECONFIGURE_SIM_SUPPORT 0

//********************** Include Files ***************************************

// My api and definitions...
#include "GenericDhcpV6ClientIf.h"

#if IPHALIF_ENABLED
#include "BaseIpHalIf.h"
#endif


#include <stdlib.h>

  
#include "Portable.h"
//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Local Functions *************************************

//********************** Class Method Implementations ************************

BcmGenericDhcpV6ClientIf::BcmGenericDhcpV6ClientIf(BcmBaseIpHalIf &ipHalIf) :
	BcmDhcpV6ClientIf(ipHalIf)
{
    fMessageLogSettings.SetModuleName("BcmGenericDhcpV6ClientIf");

	//Debug(true);

    {
        // Generate the instance name for display purposes.  My string can be
        // temporary because the message log settings creates its own memory.
        char *pInstanceName = new char[128];

        strcpy(pInstanceName, "Generic DHCPv6 ClientIf for ");
#if IPHALIF_ENABLED
        strcat(pInstanceName, ipHalIf.Description());
#endif
        fMessageLogSettings.SetInstanceName(pInstanceName);

        fMessageLogSettings.Register();

        delete pInstanceName;
    }
}


BcmGenericDhcpV6ClientIf::~BcmGenericDhcpV6ClientIf()
{
}


ostream & BcmGenericDhcpV6ClientIf::Print(ostream &outputStream) const
{
    #if !(BCM_REDUCED_IMAGE_SIZE)

    outputStream 
        << fMessageLogSettings.GetInstanceName() 
	<< endl;

    #endif
    
    return outputStream;
}


void BcmGenericDhcpV6ClientIf::Debug(bool enable)
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

bool BcmGenericDhcpV6ClientIf::SendDhcpV6Packet(BcmDhcpV6Packet *pDhcpPacket)
{
    bool result;

    CallTrace("BcmGenericDhcpV6ClientIf", "SendDhcpV6Packet");

    BcmOctetBuffer buffer(0);
    BcmEthernetPacket packet;

    // Account for the 14 byte LLC header, the 40 byte IPv6 header, the 8 byte
    // UDP header, and the 4 byte CRC32, as well as the size of the DHCPv6
    // payload.
    //unsigned int bufferSize = 14 + 40 + 8 + pDhcpPacket->Size() + 4;

	BcmIpV6Address destIpAddress(kAllDhcpRelayAgentsAndServers);
	
	buffer.ResizeBuffer(pDhcpPacket->Size());
	gInfoMsg(fMessageLogSettings, "SendDhcpV6Packet")
		<< "Generating DHCPv6 packet with these settings: Size: " << pDhcpPacket->Size() << "\n" 
		<< *pDhcpPacket 
		<< endl;

	// Now overlay my buffer on the UDP payload.
	//buffer.AdoptBuffer(packet.UdpPduWritable(), pDhcpPacket->Size(), 0, false);

	// Write the the DHCPv6 packet data as the UDP payload.
	result = pDhcpPacket->WriteTo(buffer, &fMessageLogSettings);

	if (!result)
	{
		gErrorMsg(fMessageLogSettings, "SendDhcpV6Packet")
			<< "failed to write DHCP packet to the buffer!" << endl;

		// Release the buffer back to the HAL.
		//pfIpHalIf->ReleasePacket(&packet);

		return false;
	}

	result = IpHalIf()->TransmitViaNetworkSocket( buffer, destIpAddress, kDhcpV6UdpServerPort);

    if (!result)
    {
        gWarningMsg(fMessageLogSettings, "SendDhcpV6Packet")
            << "Failed to send packet!" << endl;

        return false;
    }
    
    return true;
}


