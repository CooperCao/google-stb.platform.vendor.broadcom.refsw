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
//  Filename:       DsgBroadcastTunnelHeader.h
//  Author:         Pinar Taskiran
//  Creation Date:  May 28, 2007
//
//****************************************************************************
//  Description:
//     
//
//****************************************************************************

#ifndef BCASTTUNNELHEADER_H
#define BCASTTUNNELHEADER_H

//********************** Include Files ***************************************

// Data container class (wrapper and utility methods for dealing with memory
// buffers).
#include "OctetBuffer.h"

// Wraps the 6-byte ethernet mac address, with useful methods.
#include "MacAddress.h"
#include "MessageLog.h"
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <iostream>
#else
#include <iostream.h>
#endif

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************


class BcmDsgBroadcastTunnelHeader
{
public:

    // Initializing Constructor.  Initializes the state of the object.
    //
    // The parameter values are specified as fields in the BT
    // header, and are defined in table D-1.  Since these are dependent on the
    // actual message subclass, they must be passed to this class.
    //
    // Parameters:
    //      btHeaderVersion - each message  knows what its version is.
    //
    // Returns:  N/A
    //
    BcmDsgBroadcastTunnelHeader(void);

    // Copy Constructor.  Initializes the state of this object to match that
    // of the instance passed in.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmDsgBroadcastTunnelHeader(const BcmDsgBroadcastTunnelHeader &otherInstance);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmDsgBroadcastTunnelHeader();

    // Assignment operator.  Copies the state of the instance passed in so that
    // this object is "identical".
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmDsgBroadcastTunnelHeader & operator = (const BcmDsgBroadcastTunnelHeader &otherInstance);

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
    bool WriteTo(BcmOctetBuffer &octetBuffer);

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
    bool ParseFrom(uint8* pBuffer);

    // Print formatted object contents to specified ostream.
    //
    // Parameters:
    //      ostream& ostrm - reference to the destination output stream.
    //
    // Returns:
    //      reference to the destination output stream.
    //
    virtual ostream& Print( ostream& ostrm ) const;

    uint8 BcastTunnelHeaderVersion() const { return fVersion; }
    void BcastTunnelHeaderVersion( uint8 newval ) { fVersion = newval; }

	// accessors for message log severity and field settings.
	//
	void MsgLogSeverities( uint32 new_val )
        { fMessageLogSettings.SetEnabledSeverities( new_val ); }
    uint32 MsgLogSeverities() 
        { return fMessageLogSettings.GetEnabledSeverities(); }

    void MsgLogFields( byte new_val )
        { fMessageLogSettings.SetEnabledFields( new_val ); }
    byte MsgLogFields() 
        { return fMessageLogSettings.GetEnabledFields(); }

	bool LastSegment(void);
	bool IsSegmented(void);
	inline uint8 SegmentNumber(void);
	inline uint16 IdNumber(void);

	enum
	{
		kMacMgtHdrLenBytes = 20
	};

	enum
	{
		kBtHeaderSize = 4 // 4 bytes
	};

protected:

    // Print formatted MAC management header items to specified ostream.
    //
    // Parameters:
    //      ostream& ostrm - reference to the destination output stream.
    //
    //      bool msgLenKnown - set to false to indicate that msg length
    //          item of the MAC MGT header is unknown at the time of 
    //          the call to this function.  defaults to true.  
    //
    // Returns:
    //      reference to the destination output stream.
    //
    ostream& PrintMacMgtHdr( ostream& ostrm, bool msgLenKnown = true ) const;

protected:

	uint8 fHeaderStart;
    uint8 fVersion;
	bool fLastSegment;
	uint8 fSegmentNumber;
	uint16 fIdNumber;

    // This value is loaded
    //      - with computed msg len in WriteTo()
    //      OR
    //      - with msg len parsed from received buffer in ParseFrom()
    //
    // It is stored here for use by the Print() debug function.
    uint16 fMessageLength;

    // This controls the level of message log output.
    BcmMessageLogSettings fMessageLogSettings;

};


//********************** Inline Method Implementations ***********************


inline ostream & operator << (ostream &outStream, const BcmDsgBroadcastTunnelHeader &message)
{
    return message.Print(outStream);
}

inline uint8 BcmDsgBroadcastTunnelHeader::SegmentNumber(void)
{
	return fSegmentNumber;
}

inline uint16 BcmDsgBroadcastTunnelHeader::IdNumber(void)
{
	return fIdNumber;
}

#endif


