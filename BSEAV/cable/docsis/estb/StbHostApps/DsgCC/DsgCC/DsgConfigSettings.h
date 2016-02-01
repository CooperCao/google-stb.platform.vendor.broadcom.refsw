//****************************************************************************
//
// Copyright (c) 2004-2012 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized Licens, Broadcom grants no license
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
//****************************************************************************
//
//  Filename:       DsgConfigSettings.h
//  Author:         Pinar Taskiran
//  Creation Date:  Dec 1, 2004
//
//****************************************************************************
//  Description:
//      This is the implementation of the DSG Configuration Settings class.
//
//****************************************************************************

#ifndef DSGCONFIGSETTINGS_H
#define DSGCONFIGSETTINGS_H

//********************** Include Files ***************************************
#include "typedefs.h"
#include "OctetBuffer.h"
#include "Type43VendorSettings.h"

#include <list>
//********************** Global Types ****************************************
 // Pseudonym for the queue class type.
typedef list<uint32> ChannelList;

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************
class BcmMessageLogSettings;

class BcmDsgConfigSettings
{
public:

    // Default Constructor.  Initializes the state of the object...
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmDsgConfigSettings(void);

    // Copy Constructor.  Initializes the state of this object to match that
    // of the instance passed in.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmDsgConfigSettings(const BcmDsgConfigSettings &otherInstance);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmDsgConfigSettings();

    // Assignment operator.  Copies the state of the instance passed in so that
    // this object is "identical".
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmDsgConfigSettings & operator = (const BcmDsgConfigSettings &otherInstance);

    // Equality operator.
    //
    // Parameters:
    //      rhs - comparison object, a.k.a. right hand side.
    //
    // Returns:
    //      bool - true if objects are equal, else false.
    //
    // Note:
    //      Some compilers (like Borland C++ v5.02) require operator<
    //      for lists even if functions which require it are never
    //      called.  
    //
	bool operator==( const BcmDsgConfigSettings& rhs ) const;  

	// Default timer values
    enum
    {
        kTdsg1NoDsgPacketPeriodSec = 2,
        kTdsg2NoDsgPacketPeriodSec = 600,
        kTdsg3TwoWayRetryPeriodSec = 300,
        kTdsg4OneWayRetryPeriodSec = 1800
    };

    // misc accessors.
    bool Tdsg1TimeoutSpecified( uint16 &timeout );
    // misc accessors.
    bool Tdsg2TimeoutSpecified( uint16 &timeout );
    // misc accessors.
    bool Tdsg3TimeoutSpecified( uint16 &timeout );
    // misc accessors.
    bool Tdsg4TimeoutSpecified( uint16 &timeout );


    // This is called by ParseFrom() after the MAC Mgmt header has been
    // parsed from the buffer.  It gives the derived class a chance to parse
    // its message-specific data from the buffer.
    //
    // Note that the return code from this method only implies success or
    // failure in parsing of the message, not the contents or meaning of the
    // message (i.e. whether or not the CMTS sent a SUCCESS or FAILURE
    // confirmation code).
    //
    // Parameters:
    //      octetBuffer - reference to the octet buffer from which the message
    //                    should be parsed.  If is assumed that the first byte
    //                    in the buffer corresponds to the first byte of the
    //                    payload data.
    //      offset - the offset into the buffer where the first byte of the
    //               message is located (allows other header info to be skipped).
    //
    // Returns:
    //      true if the message was parsed correctly.
    //      false if there was a problem (incomplete message, invalid data,
    //          etc.).
    //
	byte ParseFrom( const BcmOctetBuffer &obuf, unsigned int &offset,
    BcmMessageLogSettings* log_sets, bool strict_checking = true );

	bool AnythingSpecified(void) const;

	bool Reset(void);

	// DCD object needs to know whether this tunnel has any classifier ID specified.
	bool VendorParamsSpecified(void) const { return fVendorParamsSpecified;}
    // misc accessors.
    bool ChannelListSpecified(void) const { return fChannelListSpecified;}


	// DCD object needs to know whether this tunnel has any classifier ID specified.
	BcmType43VendorSettings &VendorSettings(void) ;

    // Print() - Print detailed formatted object contents 
    //      to specified ostream.
    //
    // Parameters:
    //      ostream& ostrm - reference to the destination output stream.
    //
    // Returns:
    //      reference to the destination output stream.
    //
	ostream& Print( ostream& ostrm ) const;

	uint32 ChannelListSize( void ) const;
	bool WriteTo( BcmOctetBuffer &octetBuffer ) const;

    //
    // Added for outside entity to set various component of this object
    //
    void SetFreqChannelList( uint32 ChanFreq ) 
    { 
        fChannelListSpecified = true;
        fChannelList.push_back( ChanFreq );
    }

    void SetTdsg1Timeout( uint16 timeout ) 
    { 
        fTdsg1TimeoutSpecified = true;
        fTdsg1Timeout = timeout;
    }

    void SetTdsg2Timeout( uint16 timeout )
    {
        fTdsg2TimeoutSpecified = true;
        fTdsg2Timeout = timeout;
    }

    void SetTdsg3Timeout( uint16 timeout ) 
    { 
        fTdsg3TimeoutSpecified = true;
        fTdsg3Timeout = timeout;
    }

    void SetTdsg4Timeout( uint16 timeout )
    {
        fTdsg4TimeoutSpecified = true;
        fTdsg4Timeout = timeout;
    }

private:

	uint16 fTdsg1Timeout;
	uint16 fTdsg2Timeout;
	uint16 fTdsg3Timeout;
	uint16 fTdsg4Timeout;
	
	bool fChannelListSpecified;
	typedef ChannelList::const_iterator ChanListConstIterator;
	ChannelList fChannelList;

	bool fTdsg1TimeoutSpecified;
	bool fTdsg2TimeoutSpecified;
	bool fTdsg3TimeoutSpecified;
	bool fTdsg4TimeoutSpecified;

	bool fVendorParamsSpecified;

	// Vendor Specific Parameters
	BcmType43VendorSettings fVendorSettings;
};


//********************** Inline Method Implementations ***********************
inline ostream& operator<<( ostream& ostrm, const BcmDsgConfigSettings& rhs )
{
    return rhs.Print( ostrm );
}
#endif


