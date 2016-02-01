//****************************************************************************
//
// Copyright (c) 2004-2011 Broadcom Corporation
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
//  Filename:       DsgClientId.h
//  Author:         Pinar Taskiran
//  Creation Date:  Dec 1, 2004
//
//****************************************************************************
//  Description:
//      This is the implementation of the Client ID class.
//
//****************************************************************************

#ifndef DSGCLIENTID_H
#define DSGCLIENTID_H

//********************** Include Files ***************************************
#include "typedefs.h"
#include "OctetBuffer.h"
#include "MacAddress.h"
//********************** Global Types ****************************************

//********************** Global Constants ************************************
// These are only shared between BcmDsgClient and BcmDsgClientId classes.
// Merge BcmDsgClient and BcmDsgClientId
// Add kCardEntryClientType and rearrange enum to match the DSG client ID in spec
enum
{
	kCardEntryClientType	= 0, 

	kBroadcastType			= 1, // Application ID is uint16 or NULL
	kMacAddressType			= 2, // Application ID is in MAC ADDRESS format
	kCaSystemType			= 3, // Application ID is uint16
	kApplicationType		= 4, // Application ID is uint16

	kApplicationNameType	= 5, // Application ID is in text format
	kVendorType				= 6 // ?? uint16
};
// These are the potential values for fClientId for fIdType == kBroadcastType.
enum
{
	kBroadcastUnspecified		= 0, // there is no value specified (NULL)
	kBroadcastSCTE65		= 1, // SCTE-65 data
	kBroadcastSCTE18		= 2, // SCTE-18 data
	kBroadcastOcapCarousel		= 3, // OCAP object Carousel data
	kBroadcastCommonCarousel	= 4, // OpenCable Common Download Carousel data
	kBroadcastXAITData      	= 5  // OpenCable XAIT data

};
//********************** Global Variables ************************************

//********************** Class Declaration ***********************************
class BcmMessageLogSettings;
class BcmDsgClient;

class BcmDsgClientId
{
public:

    // Default Constructor.  Initializes the state of the object...
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmDsgClientId(void);

    // Copy Constructor.  Initializes the state of this object to match that
    // of the instance passed in.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmDsgClientId(const BcmDsgClientId &otherInstance);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmDsgClientId();

    // Assignment operator.  Copies the state of the instance passed in so that
    // this object is "identical".
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmDsgClientId & operator = (const BcmDsgClientId &otherInstance);

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
	bool operator==( const BcmDsgClient& rhs ) const;  

    // misc accessors.
    uint16 Id() const { return fClientId; }
    void Id( uint16 new_val ) { fClientId = new_val; }

    // misc accessors.
//    bool Active() const { return fActive; }
//    int ClientPort() const { return fClientPort; }

    // misc accessors.
    uint32 IdType() const { return fIdType; }
    void IdType( uint32 new_val ) { fIdType = new_val; }

    bool IsBroadcastSCTE65() const { if((fIdType == kBroadcastType) && (fClientId == kBroadcastSCTE65)) return true; return false; }
    bool IsBroadcast() const { if(fIdType == kBroadcastType) return true; return false; }

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
    BcmMessageLogSettings* log_sets );

	byte ParseBroadcastType( const BcmOctetBuffer &obuf, unsigned int &offset,
    BcmMessageLogSettings* log_sets );

	ostream& Print( ostream& ostrm ) const;
	const char* IdTypeName( uint32 value ) const;

    BcmMacAddress ClientAddress() const { return fClientAddr; }
	void ClientAddress(BcmMacAddress &id);

private:

    // Client ID
    uint16 fClientId;
	BcmMacAddress fClientAddr;

	//bool fActive;
	//int fClientPort;

	uint32 fIdType;
};


//********************** Inline Method Implementations ***********************

inline ostream& operator<<( ostream& ostrm, const BcmDsgClientId& rhs )
{
    return rhs.Print( ostrm );
}

#endif


