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
//  Filename:       DsgConfigSettings.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  Dec 1, 2004
//
//****************************************************************************
//  Description:
//      This is the implementation of the Dsg Configuration Settings class.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "DsgConfigSettings.h"

#include "TLVCodes.h"
#include "TLV.h"
#include "DcdTlvCodes.h"
#include "MessageLog.h"
#include "MessageLogSettings.h"
//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Default Constructor.  Initializes the state of the object...
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmDsgConfigSettings::BcmDsgConfigSettings(void) :
   
	fChannelListSpecified(false),
	fTdsg1TimeoutSpecified(false),
	fTdsg2TimeoutSpecified(false),
	fTdsg3TimeoutSpecified(false),
	fTdsg4TimeoutSpecified(false),
	fVendorParamsSpecified(false),
	fVendorSettings()
	
{
	Reset();
}

// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmDsgConfigSettings::~BcmDsgConfigSettings()
{
    // Clear list.
	fChannelList.clear();
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
BcmDsgConfigSettings & BcmDsgConfigSettings::operator = (const BcmDsgConfigSettings &otherInstance)
{
    // Copy each member.
    fTdsg1TimeoutSpecified = otherInstance.fTdsg1TimeoutSpecified;
    fTdsg2TimeoutSpecified = otherInstance.fTdsg2TimeoutSpecified;
    fTdsg3TimeoutSpecified = otherInstance.fTdsg3TimeoutSpecified;
    fTdsg4TimeoutSpecified = otherInstance.fTdsg4TimeoutSpecified;
    fTdsg1Timeout = otherInstance.fTdsg1Timeout;
    fTdsg2Timeout = otherInstance.fTdsg2Timeout;
    fTdsg3Timeout = otherInstance.fTdsg3Timeout;
    fTdsg4Timeout = otherInstance.fTdsg4Timeout;

    fChannelListSpecified = otherInstance.fChannelListSpecified;
    fChannelList =  otherInstance.fChannelList;

    fVendorParamsSpecified = otherInstance.fVendorParamsSpecified;
    fVendorSettings = otherInstance.fVendorSettings;

    return *this;
}

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
byte BcmDsgConfigSettings::ParseFrom( const BcmOctetBuffer &octetBuffer, unsigned int &offset,
    BcmMessageLogSettings* log_sets, bool strict_checking )
{
    // create temporary octet buf and offset.
    //BcmOctetBuffer tmp_obuf(0);
    //unsigned int tmp_offset = 0;

    TLVTable tlvTable[] =
    {
        // settings common to DCD top level.
		{ kChannelList,			kUnknownLength,	kTLV_nested, NULL, true,  false, (char *) "Channel List", NULL },   
        { kInitTimeout,			2,	kTLV_uint16, &fTdsg1Timeout, false, true, (char *) "Initialization Timeout",  &fTdsg1TimeoutSpecified },  
		{ kOperationalTimeout,	2,	kTLV_uint16, &fTdsg2Timeout, false, true, (char *) "Operational Timeout",  &fTdsg2TimeoutSpecified },
		{ kTwoWayRetryTimeout,  2,  kTLV_uint16, &fTdsg3Timeout, false, true, (char *) "Two-Way Retry Timeout",  &fTdsg3TimeoutSpecified },
        { kOneWayRetryTimeout,  2,	kTLV_uint16, &fTdsg4Timeout, false, true, (char *) "One-Way Retry Timeout",  &fTdsg4TimeoutSpecified },
        { kDsgVendorSpec,		kUnknownLength,  kTLV_nested,NULL, true,  false, (char *) "Vendor Specific", &fVendorParamsSpecified },

        // This must always be the last one in the table.
		kEndOfTLVTable
    };

    // Algorithm:
    //    Set up table to look for the top level Type codes.
    //    while top level type found...
    //      ...create new buffer, referring to the V bytes following top TL,
    //          going for length-1 bytes.
    //      create target object.  have it parse its TLVs.
    //      store target object.
    //    endwhile
    //


    // Parse the TLVs until end of input or special case exit.
    TLVParserResult result = ParseTLVs(octetBuffer, offset, tlvTable, log_sets);

    while( result == kTLVParserStopParsingFound )
    {
        // parser found a mutant type.  examine it.
        //
        // Note that this is set up such that if multiple instances of these
        // child types are encountered, then they will be applied to the
        // settings that have already been parsed.  The next effect is that
        // they will be treated as though they came across in one big lump.
        uint8 typeField, lengthField;

        if (!octetBuffer.NetworkRead(offset, typeField) ||
            !octetBuffer.NetworkRead(offset, lengthField))
        {
            return kRejMsgSyntaxError;
        }

        switch( typeField )
        {
            case kChannelList:
                {
					uint32 channel;
					if (!octetBuffer.NetworkRead(offset, channel))
					{
						return kRejMsgSyntaxError;
					}

					fChannelListSpecified = true;

					// Insert into the channel list.
					fChannelList.push_back(channel);
					//cout << setfill( ' ' ) << setw( 65 ) << dec << channel <<endl;
                }
                break;

            case kDsgVendorSpec:
                {
                    BcmOctetBuffer tempBuffer(0);
                    unsigned int tempOffset;
                    byte confCode;

                    // Overlay the V portion of this parent TLV.
                    tempBuffer.Overlay(octetBuffer, offset, lengthField);
                    tempOffset = 0;

                    // Parse the values.
                    confCode = fVendorSettings.ParseFrom(tempBuffer, tempOffset, log_sets);

                    if (confCode == kConfOk)
                    {
                        // Update the offset of the top level buffer.
                        offset += tempOffset;

						fVendorParamsSpecified = true;
                    }		 
                }
                break;

        }

        // start the parser once again
        
        // Parse the TLVs until end of input or special case exit.
        result = ParseTLVs(octetBuffer, offset, tlvTable, log_sets);
    }

    // Check for an error during parsing.  Do we skip over unrecognized type
    // codes, or do we error?????  We should skip over rather than error; this
    // is done for us by the parser.
    if( result >= kTLVParserError )
    {
        // Log this to the debugger...
        if (log_sets)
        {
            gErrorMsg(*log_sets, "ParseFrom") << "Failed to parse basic Classifier TLVs" << endl;
        }

        return kRejMsgSyntaxError;
    }

    if( !strict_checking )
    {
        // special case: client has specified that parsed values are NOT to
        // be validated w/strict error checking rules.
        return kConfOk;
    }

    /*if( !err_set_list.empty() )
    {
        // at least one error set is present.  
        // add rx_err_set_list to the errorEncodingList param passed to 
        // this function.
        return errorEncodingList.AddQosErrorEncoding( *this, err_set_list );
    }*/

    // NOTE:  It is assumed that some higher-layer entity is going to verify
    // that the required things are present, and that the combination of
    // settings that are present is allowed...

    return kConfOk;
}	

bool BcmDsgConfigSettings::AnythingSpecified(void) const
{
    if (fChannelListSpecified ||
        fTdsg1TimeoutSpecified ||
        fTdsg2TimeoutSpecified ||
        fTdsg3TimeoutSpecified ||
        fTdsg4TimeoutSpecified ||
		fVendorParamsSpecified
	   )
    {
        return true;
    }

    return false;
}

bool BcmDsgConfigSettings::Reset(void)
{
	// Clear channel list
	fChannelList.clear();

	// Reset member variables.
	fTdsg1Timeout = 0;
	fTdsg2Timeout = 0;
	fTdsg3Timeout = 0;
	fTdsg4Timeout = 0;

	fChannelListSpecified = false;
	fTdsg1TimeoutSpecified = false;
	fTdsg2TimeoutSpecified = false;
	fTdsg3TimeoutSpecified = false;
	fTdsg4TimeoutSpecified = false;

	fVendorParamsSpecified = false;
	return true;
}

ostream& BcmDsgConfigSettings::Print( ostream& ostrm ) const
{

#if !(BCM_REDUCED_IMAGE_SIZE)

    ostrm 
        << "\n========================="
        << "\nChannel List:\n";


    // consider phs settings
    if( !fChannelList.empty() )
    {
        ChanListConstIterator ch_it = fChannelList.begin();
        ChanListConstIterator ch_end = fChannelList.end();
        while( ch_it != ch_end )
        {
            // print channel
			ostrm << (*ch_it) << endl;
            
            // increment iterator.
            ++ch_it;
        }   
    }

    ostrm 
        << "\n========================="
        << "\n Tdsg1 Timeout: " 
            << fTdsg1Timeout <<" " << ( fTdsg1TimeoutSpecified ? "specified" : "not specified" )

        << "\n Tdsg2 Timeout: " 
            << fTdsg2Timeout <<" " << ( fTdsg2TimeoutSpecified ? "specified" : "not specified" ) 

        << "\n Tdsg3 Timeout: " 
            << fTdsg3Timeout <<" " << ( fTdsg3TimeoutSpecified ? "specified" : "not specified" )  

        << "\n Tdsg4 Timeout: " 
            << fTdsg4Timeout <<" " << ( fTdsg4TimeoutSpecified ? "specified" : "not specified" )  
		<< endl;

	if( fVendorParamsSpecified == true )
	{
		//ostrm << fVendorSettings << endl;
	}

#endif

    return ostrm;
}

uint32 BcmDsgConfigSettings::ChannelListSize( void ) const
{
	return fChannelList.size();
}

bool BcmDsgConfigSettings::WriteTo( BcmOctetBuffer &octetBuffer ) const
{
	octetBuffer.Fill(0);
	octetBuffer.Empty();

	ChanListConstIterator ch_it = fChannelList.begin();
	ChanListConstIterator ch_end = fChannelList.end();
	while( ch_it != ch_end )
	{
		uint32 channel = (uint32) (*ch_it);

		if( !octetBuffer.AddToEnd( channel ))
		{
			cout <<"Failed to add" << endl;
			return false;
		}
    	
		// increment iterator.
		++ch_it;
	}   

	return true;
}

// Accessor to read the Vendor Specific data that was previously stored or
// parsed.
//
// Parameters:  None.
//
// Returns:
//      A reference to the vendor specific data.
//
BcmType43VendorSettings &BcmDsgConfigSettings::VendorSettings(void) 
{
	return fVendorSettings;
}

// misc accessors.
bool BcmDsgConfigSettings::Tdsg1TimeoutSpecified( uint16 &timeout ) 
{ 
	if(fTdsg1TimeoutSpecified && fTdsg1Timeout)
		timeout = fTdsg1Timeout; 
	else
		timeout = kTdsg1NoDsgPacketPeriodSec;

	return fTdsg1TimeoutSpecified;
}
 
// misc accessors.
bool BcmDsgConfigSettings::Tdsg2TimeoutSpecified( uint16 &timeout ) 
{ 
	if(fTdsg2TimeoutSpecified && fTdsg2Timeout)
		timeout = fTdsg2Timeout; 
	else
		timeout = (uint16)kTdsg2NoDsgPacketPeriodSec;

	return fTdsg2TimeoutSpecified;
}

// misc accessors.
bool BcmDsgConfigSettings::Tdsg3TimeoutSpecified( uint16 &timeout ) 
{ 
	if(fTdsg3TimeoutSpecified)
		timeout = fTdsg3Timeout; 
	else
		timeout = (uint16) kTdsg3TwoWayRetryPeriodSec;

	return fTdsg3TimeoutSpecified;
}

// misc accessors.
bool BcmDsgConfigSettings::Tdsg4TimeoutSpecified( uint16 &timeout ) 
{ 
	if(fTdsg4TimeoutSpecified)
		timeout = fTdsg4Timeout; 
	else
		timeout = (uint16) kTdsg4OneWayRetryPeriodSec;

	return fTdsg4TimeoutSpecified;
}
