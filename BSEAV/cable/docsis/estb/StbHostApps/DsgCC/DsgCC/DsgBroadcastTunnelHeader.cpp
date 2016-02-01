//****************************************************************************
//
// Copyright (c) 2007-2010 Broadcom Corporation
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
//
//  Filename:       DsgBroadcastTunnelHeader.cppp
//  Author:         Pinar Taskiran
//  Creation Date:  May 28, 2007
//
//****************************************************************************

//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "DsgBroadcastTunnelHeader.h"
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <iomanip>
#else
#include <iomanip.h>
#endif
#include "TLVCodes.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

// These are some of the values that go in the MAC Management Message header,
// and are specified in the DOCSIS spec.
const uint8 kDSAP = 0;
const uint8 kSSAP = 0;
const uint8 kControl = 0x03;
const uint8 kReserved = 0;

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Initializing Constructor.  Initializes the state of the object.
//
// The parameter values are specified as fields in the MAC Management
// header, and are defined in table 6-17.  Since these are dependent on the
// actual message subclass, they must be passed to this class.
//
// Parameters:
//
// Returns:  N/A
//
BcmDsgBroadcastTunnelHeader::BcmDsgBroadcastTunnelHeader() 
  :
    // Where do I get these from?  My MAC Address comes from NVRam or some other
    // storage; the CMTS comes from the first UCD I receive?
    fHeaderStart( 0x0 ),
    fVersion( 0x0 ),
    fLastSegment( false ),
    fSegmentNumber( 0x0 ),
	fIdNumber( 0x0 )
{
    fMessageLogSettings.SetEnabledFields( 0 );
    fMessageLogSettings.Register();
}



// Copy Constructor.  Initializes the state of this object to match that
// of the instance passed in.
//
// Parameters:
//      otherInstance - reference to the object to copy.
//
// Returns:  N/A
//
BcmDsgBroadcastTunnelHeader::BcmDsgBroadcastTunnelHeader( const BcmDsgBroadcastTunnelHeader &otherInstance)
  :
    fHeaderStart( otherInstance.fHeaderStart ),
    fVersion( otherInstance.fVersion ),
    fLastSegment( otherInstance.fLastSegment ),
    fSegmentNumber( otherInstance.fSegmentNumber ),
    fIdNumber( otherInstance.fIdNumber ),
    fMessageLogSettings( otherInstance.fMessageLogSettings )
{
    fMessageLogSettings.Register();
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmDsgBroadcastTunnelHeader::~BcmDsgBroadcastTunnelHeader()
{
    fMessageLogSettings.Deregister();    
}


// Assignment operator.  Copies the state of the instance passed in so that
// this object is "identical".
//
// Parameters:
//      otherInstance - reference to the object to copy.
//
// Returns:
//      A reference to "this" so that operator = can be chained.
//
BcmDsgBroadcastTunnelHeader & BcmDsgBroadcastTunnelHeader::operator = (const BcmDsgBroadcastTunnelHeader &otherInstance)
{
    if( this == &otherInstance )
    {
        // special case: assignment to self
        return *this;
    }

    // Copy protected and private members of otherInstance to my members.
    fHeaderStart = otherInstance.fHeaderStart;
    fVersion = otherInstance.fVersion;
    fLastSegment = otherInstance.fLastSegment;
    fSegmentNumber = otherInstance.fSegmentNumber;
    fIdNumber = otherInstance.fIdNumber;
    fMessageLogSettings = otherInstance.fMessageLogSettings;

    return *this;
}


// This method causes the message to convert itself to a flat octet buffer
// (in network byte order) in preparation for being transmitted.
//
// Note that the message is appended to the octet buffer; any existing data
// is not lost.  This is useful when there is a header at the front of the
// message.
//
// Parameters:
//      octetBuffer - reference to the octet buffer into which the message
//                    should be written.
//
// Returns:
//      true if the message was stored in the buffer correctly.
//      false if there was a problem (buffer not big enough, incorrect or
//          invalid configuration, etc.).
//
/*
bool BcmDsgBroadcastTunnelHeader::WriteTo(BcmOctetBuffer &octetBuffer)
{
    unsigned int messageLengthOffset;
    uint16 messageLength = 0;
    uint16 bytes_used_entry = octetBuffer.BytesUsed();

    // Add the rest of the values for the header.
    if (!octetBuffer.NetworkAddToEnd(kDSAP) ||
        !octetBuffer.NetworkAddToEnd(kSSAP) ||
        !octetBuffer.NetworkAddToEnd(kControl) ||
        !octetBuffer.NetworkAddToEnd(fVersion) ||
        !octetBuffer.NetworkAddToEnd(fHeaderStart) ||
        !octetBuffer.NetworkAddToEnd(kReserved))
    {
        // Log this to the debugger...

        return false;
    }

    if( TlvLoggingEnabled() && pgMessageLogStream )
    {
        // TLV logging is enabled AND log stream exists.
        // print MAC MGT header info to log.
        PrintMacMgtHdr( *pgMessageLogStream );
        *pgMessageLogStream << "\n\nMSG PDU:";
    }

    // Now vector to the derived class to let it add its data to the buffer.
    if (!DerivedWriteTo(octetBuffer))
    {
        // Log this to the debugger...

        return false;
    }

    // Now, rewind and write the true message length in the header.  This
    // includes all bytes from the DSAP field to the end of the payload.
    messageLength = octetBuffer.BytesUsed() - messageLength;
    fLastSegment = messageLength;
    if (!octetBuffer.NetworkOverwrite(messageLengthOffset, messageLength))
    {
        // Log this to the debugger...

        return false;
    }

    // If the MAC hardware doesn't support CRC32 generation, then I need to do
    // it here.

    // Log the octet buffer to the debugger if that severity is enabled.
    gAppMsg(fMessageLogSettings, "WriteTo", kOctetLogging)
        << "\nMAC MGT msg buf, len=" 
        << (octetBuffer.BytesUsed() - bytes_used_entry)
        << octetBuffer  // raw mac mgt msg buffer
        << endl;

    return true;
}
*/

// This method parses the message from the flat octet buffer, converting
// from network byte order to machine byte order as necessary.
//
// Note that the return code from this method only implies success or
// failure in parsing of the message, not the contents or meaning of the
// message (i.e. whether or not the CMTS sent a SUCCESS or FAILURE
// confirmation code).
//
// Parameters:
//      octetBuffer - reference to the octet buffer from which the message
//                    should be parsed.  It is assumed that the first byte
//                    in the octet buffer corresponds to the first byte
//                    of the message.
//      offset - the offset into the buffer where the first byte of the
//               message is located (allows other header info to be skipped).
//
// Returns:
//      true if a valid message was parsed.
//      false if there was a problem (incomplete or incorrect message,
//          insufficient resources to parse the message, etc.).
//
bool BcmDsgBroadcastTunnelHeader::ParseFrom(uint8* pBuffer)
{
	BcmOctetBuffer octetBuffer(pBuffer, kBtHeaderSize, kBtHeaderSize, false);
 
    uint8 versionAndSegment = 0;
	unsigned int offset = 0;

    // read fHeaderStart, fVersion fields.  these are constant values.
    if( !octetBuffer.NetworkRead(offset, fHeaderStart) ||
		!octetBuffer.NetworkRead(offset, versionAndSegment))
	{
		return false;
	}

	fVersion = versionAndSegment & (0xE0);
	fVersion >>= 5;

	if( (fHeaderStart != 0xFF) || (fVersion != 0x01) )
	{
		cout <<"BT header is not valid - fHeaderStart: " << inHex(fHeaderStart) << endl;
		return false;
	}

	if( versionAndSegment & 0x10 )
	{
		fLastSegment = true;
	}

	fSegmentNumber = versionAndSegment & 0x0F;

    if( !octetBuffer.NetworkRead(offset, fIdNumber) )
	{
		return false;
	}

    if(0) // TlvLoggingEnabled() && pgMessageLogStream )
    {
        // TLV logging is enabled AND log stream exists.
        // print MAC MGT header info to log.
        PrintMacMgtHdr( *pgMessageLogStream );
        *pgMessageLogStream << "\n\nMSG PDU:";
    }

	return true;
}


ostream& BcmDsgBroadcastTunnelHeader::PrintMacMgtHdr( ostream& ostrm, bool msgLenKnown ) const
{
    ostrm
        << "\n         header start: " << inHex(fHeaderStart)
        << "\n              version: " << inHex(fVersion)
        << "\n             last segment: " << (fLastSegment?"true":"false")
        << "\n             segment number: " << inHex(fSegmentNumber)
        << "\n              id number: " << inHex(fIdNumber);

    return ostrm;
}

ostream& BcmDsgBroadcastTunnelHeader::Print( ostream& ostrm ) const
{
    // call helper function to print MAC mgt header items.
    PrintMacMgtHdr( ostrm );
	return ostrm;
}

/*
byte BcmDsgBroadcastTunnelHeader::BcastTunnelHeaderType( byte* buf, uint16 len )
{
    // msg type code is located 18 bytes into the mac mgt header.
    byte ret_val = 0;
    if( buf && (len >= 18) )
    {
        ret_val = *( buf + 18 );
    }
    return ret_val;
}
*/

bool BcmDsgBroadcastTunnelHeader::LastSegment(void) 
{
	return fLastSegment;
}

bool BcmDsgBroadcastTunnelHeader::IsSegmented(void) 
{
	if( (fSegmentNumber == 0) && (fLastSegment) )
	{
		return false;
	}

	return true;
}
