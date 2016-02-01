//****************************************************************************
//
// Copyright (c) 2007-2011 Broadcom Corporation
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
//  Filename:       DsgBroadcastTunnelPktReassembler.h
//  Author:         Pinar Taskiran
//  Creation Date:  May 28, 2007
//
//****************************************************************************
//  Description:
//     
//
//****************************************************************************


#ifndef DSGBCASTTUNNELPKTREASSEMBLER_H
#define DSGBCASTTUNNELPKTREASSEMBLER_H

//********************** Include Files ***************************************

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <map>
#else
#include <map.h>
#endif
#include "OctetBuffer.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************
class BcmEthernetPacket;
class BcmMessageLogSettings;
class BcmDsgBroadcastTunnelHeader;


class BcmDsgBroadcastTunnelPktReassemblyBuffer
{
public:
    // Default Constructor.
    BcmDsgBroadcastTunnelPktReassemblyBuffer(BcmEthernetPacket &ipDatagramBtFragment);

    // Destructor.
    ~BcmDsgBroadcastTunnelPktReassemblyBuffer();

    // return code for ProcessFragment.  Based on the code, appripriate action must be taken. 
    typedef enum
    {
		kNotFragmented,							 // Datagram is not fragmented.
        kAllFragmentsReceived,                   // The fragment was processed successfully, 
                                                 // and the complete datagram is ready.
        kNotAllFragmentsReceived,                // The fragment was processed successfully,
                                                 // but not all fragments have been received.
        kIllegalFragment,                        // Malformned fragment (bad header, etc).
        kOverlappingFragment,                    // Fragment with offset & length overlaps with a fragment
                                                 // already received (Teardrop attack, etc).
        kPayloadTooLarge                         // Payload is larger than what is allowed by IP spec
    }fragmentCode;


    //
    //
    // Parameters:
    //      ipDatagramFragment - the fragment to be processed
    //
    // Returns:
    //      true if the packet has been completely reassembled into reassembledPacket
    //      false if the last packet has not been found and reassembledPacket is unchanged
    //
    uint32 ProcessFragment( BcmEthernetPacket &ipDatagramFragment, BcmDsgBroadcastTunnelHeader &btHeader );

    //
    // Parameters:
    //
    // Returns:
    //      true if the fragment buffer has timed out
    //      false if the fragment buffer has not timed out
    //
    bool TimedOut();

    // the total number of fragments received for this fragment set
    unsigned int NumberOfFragmentsReceived(void);

    ostream& Print( ostream& ostrm ) const;

	BcmOctetBuffer* PayloadBuffer();
	void BufferInUse( bool value );

	enum
	{
		kBroadcastTunnelMpegTwoSectionMaxSize		= 4096,
		kMaximumReassembleyBufferLifespanSeconds	= 60 // No such value is specified in the DSG spec. 
														 // Might be required in the future.
	};

private:

    // buffer for payload
    BcmOctetBuffer *pfPayloadBuffer;

    // when this buffer was created
    unsigned long fCreationTimeMS;

    unsigned int fMaxPacketSize;

    unsigned int fNumberOfFragmentsReceived;

	bool fBufferInUse;

    //BcmMessageLogSettings   fMessageLogSettings;

private:

    // Copy Constructor.  Not supported.
    BcmDsgBroadcastTunnelPktReassemblyBuffer(const BcmDsgBroadcastTunnelPktReassemblyBuffer &otherInstance);

    // Assignment operator.  Not supported.
    BcmDsgBroadcastTunnelPktReassemblyBuffer & operator = (const BcmDsgBroadcastTunnelPktReassemblyBuffer &otherInstance);
};

// Insertion operator for this class.  It allows you to do the following:
//
//    cout << commandParameter << endl;
//
inline ostream & operator << (ostream &outputStream, const BcmDsgBroadcastTunnelPktReassemblyBuffer &buffer)
{
    return buffer.Print(outputStream);
}
//****************************************************************************
typedef map< uint16, BcmDsgBroadcastTunnelPktReassemblyBuffer *, std::less<uint16> > ReassemblyBufferMap;

typedef ReassemblyBufferMap::iterator ReassemblyBufferMapIt;
typedef ReassemblyBufferMap::const_iterator ReassemblyBufferMapConstIt;

class BcmDsgBroadcastTunnelPktReassembler 
{
public:
    // begin functions required to be param for STL containers...
    //
    
    // Constructor. 
    //
    BcmDsgBroadcastTunnelPktReassembler();
     
    // Destructor.  
    //
    ~BcmDsgBroadcastTunnelPktReassembler();

	// Parameters:
	//      ipDatagramBtFragment - the fragment to be processed
	//      pReassembledPacket - if true returned, the reassembled packet is here
	//
	// Returns:
	//      true if the packet has been completely reassembled into pReassembledPacket
	//      false if the last packet has not been found
	//
	uint32 ProcessBtFragment( BcmEthernetPacket &ipDatagramBtFragment, BcmOctetBuffer** pReassembledPacket );

    //
    // end functions required to be param for STL containers...

    // Reset() - Reset to initial conditions.
    //
    void Reset();

    // Print formatted object contents to specified ostream.
    //
    // Parameters:
    //      ostream& ostrm - reference to the destination output stream.
    //
    // Returns:
    //      reference to the destination output stream.
    //
	ostream& Print( ostream& ostrm ) const;

    // PrintSummary() - Print summary of formatted object contents 
    //      to specified ostream.
    //
    // Parameters:
    //      ostream& ostrm - reference to the destination output stream.
    //
    // Returns:
    //      reference to the destination output stream.
    //
	ostream& PrintSummary( ostream& ostrm ) const;

    // PrintDetailed() - Print detailed formatted object contents 
    //      to specified ostream.
    //
    // Parameters:
    //      ostream& ostrm - reference to the destination output stream.
    //
    // Returns:
    //      reference to the destination output stream.
    //
	ostream& PrintDetailed( ostream& ostrm ) const;



// data members are public for now because it's a hassle to manipulate
// the various <xxxx>SetsList data members via accessor functions.
public:    

    ReassemblyBufferMap fReassemblyBufferMap;

private:
    // Copy Constructor. Not supported.
    //
    BcmDsgBroadcastTunnelPktReassembler( const BcmDsgBroadcastTunnelPktReassembler& rhs );

    // Assignment operator. Not supported.
    //
    BcmDsgBroadcastTunnelPktReassembler& operator=( const BcmDsgBroadcastTunnelPktReassembler& rhs );

    // Equality operator. Not supported.
	bool operator==( const BcmDsgBroadcastTunnelPktReassembler& rhs ) const;    
};

inline ostream& operator<<( ostream& ostrm, const BcmDsgBroadcastTunnelPktReassembler& rhs )
{
    return rhs.Print( ostrm );
}

#endif



