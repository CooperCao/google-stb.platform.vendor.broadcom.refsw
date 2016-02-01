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
//  Filename:       DsgClientId.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  Dec 1, 2004
//
//****************************************************************************
//  Description:
//      This is the implementation of the Client ID class.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "DsgClientId.h"

#include "TLV.h"
#include "MessageLog.h"
#include "MessageLogSettings.h"
#include "MacAddress.h"
#include "DsgClient.h"
#include "DcdTlvCodes.h"
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
BcmDsgClientId::BcmDsgClientId(void) :
   
    fClientId( 0 ),
	fIdType (0),
	fClientAddr(kAllZerosMac)
//	fClientPort( 0 ),
//	fActive(false)
{
}


// Copy Constructor.  Initializes the state of this object to match that
// of the instance passed in.
//
// Parameters:
//      otherInstance - reference to the object to copy.
//
// Returns:  N/A
//
BcmDsgClientId::BcmDsgClientId(const BcmDsgClientId &otherInstance) 
{
    // Copy the members from the other instance.
    fClientId = otherInstance.fClientId;
	fIdType = otherInstance.fIdType;
	fClientAddr = otherInstance.fClientAddr;
//	fClientPort = otherInstance.fClientPort;
//	fActive = otherInstance.fActive;
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmDsgClientId::~BcmDsgClientId()
{
    // Nothing to do.
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
BcmDsgClientId & BcmDsgClientId::operator = (const BcmDsgClientId &otherInstance)
{
    // Copy protected and private members of otherInstance to my members.
    fClientId = otherInstance.fClientId;
	fIdType = otherInstance.fIdType;
	fClientAddr = otherInstance.fClientAddr;
//	fClientPort = otherInstance.fClientPort;
//	fActive = otherInstance.fActive;

    return *this;
}

bool BcmDsgClientId::operator==(
  const BcmDsgClient& rhs ) const
{
    if( fIdType != (uint16)rhs.IdType() )
    {
		#if 0
		cout
				<<" Tunnel's client ID TYPE = " << IdTypeName((uint32)fIdType)
				<<" did not match client's ID TYPE = " << IdTypeName(rhs.IdType()) <<endl;
		#endif
        return false;
    }	

	if( fIdType == kMacAddressType )
	{
		if( fClientAddr != (rhs.ClientAddress()) )
		{
			#if 0
			//gAlwaysMsgNoFields(fMessageLogSettings)
			cout
				<<" Tunnel's client ADDRESS = " << fClientAddr
				<<" did not match client's ADDRESS = " << rhs.ClientAddress() <<endl;
			#endif
			return false;
		}
	}
	else
	{
		if( fClientId != (uint16)rhs.Id() )
		{
			#if 0
			//gAlwaysMsgNoFields(fMessageLogSettings)
			cout
				<<" Tunnel's client ID = " << fClientId
				<<" did not match client's ID = " << rhs.Id() <<endl;
			#endif
			return false;
		}
	}

    return true;
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
byte BcmDsgClientId::ParseFrom( const BcmOctetBuffer &obuf, unsigned int &offset,
    BcmMessageLogSettings* log_sets )

{
    // create temporary octet buf and offset.
    //BcmOctetBuffer tmp_obuf(0);
    //unsigned int tmp_offset = 0;

	bool fMacAddressTypeSpecified = false;
	bool fCaSystemTypeSpecified = false;
	bool fApplicationTypeSpecified = false;

    TLVTable tlvTable[] =
    {
        // settings common to DCD top level.
		{ kBroadcast,			kUnknownLength,	kTLV_nested, NULL, true, false, (char *) "Broadcast Id", NULL },   
        { kWellKnownMacAddress,				 6,	kTLV_MacAddress, &fClientAddr, false, false, (char *) "MAC Address", &fMacAddressTypeSpecified },    
		{ kCaSystemId,						 2, kTLV_uint16, &fClientId, false, false, (char *) "Cas Id", &fCaSystemTypeSpecified }, 
		{ kApplicationId,					 2, kTLV_uint16, &fClientId, false, false, (char *) "App Id", &fApplicationTypeSpecified }, 

        // This must always be the last one in the table.
		kEndOfTLVTable
    };

    // Parse the TLVs until end of input or special case exit.
	TLVParserResult result = kTLVParserEndOfData;

	// Parse type and length because Broadcast type is handled differently.
    uint8 typeField, lengthField;

    if (!obuf.NetworkRead(offset, typeField) ||
        !obuf.NetworkRead(offset, lengthField))
    {
        return kRejMsgSyntaxError;
    }

    if( typeField == kBroadcast)
    {                    
		// create temporary octet buf and offset.
		BcmOctetBuffer tmp_obuf(0);
		unsigned int tmp_offset = 0;

        // load an octet buf with the V bytes of the parent 
        // (a.k.a. the child TLV's).
		// Make sure to include type and length fields
		// when passing it to the child buffer below.
		// That is why child buffer points to (offset-2).
		offset = offset - 2;
        tmp_obuf.Overlay( obuf, offset, (lengthField + 2) );
        tmp_offset = 0;

		byte confCode;
		confCode = ParseBroadcastType( tmp_obuf, tmp_offset, log_sets );

        if (confCode == kConfOk)              
        {                    
            // update offset of top level buffer.
            offset += tmp_offset;
        }
        else
        {						
            // error.
            if (log_sets)
            {
                gErrorMsg(*log_sets, "ParseFrom") 
				<< "Failed to parse Dsg Rule Client ID TLVs" << endl;
            }

            return kRejOther;
        }                      
    }
	else
	{
		// Move the offset back to point to Type.
		offset = offset - 2;

		result = ParseTLVs(obuf, offset, tlvTable, log_sets);
	}
    
    // Check for an error during parsing.  Do we skip over unrecognized type
    // codes, or do we error?????  We should skip over rather than error; this
    // is done for us by the parser.
    if( result >= kTLVParserError )
    {
        // Log this to the debugger...      
		if(log_sets)
			gErrorMsg(*log_sets, "ParseFrom")  <<"kRejMsgSyntaxError" << endl;
        return kRejMsgSyntaxError;
    }

	// Check that ID type is specified.
	// We check for broadcast ID type in ParseBroadcastType().
    if( (fIdType != kBroadcastType) )
	{
		if( fMacAddressTypeSpecified )
		{
			fIdType = kMacAddressType;
		}
		else if( fCaSystemTypeSpecified )
		{
			fIdType = kCaSystemType;
		}
		else if( fApplicationTypeSpecified )
		{
			fIdType = kApplicationType;
		}
		else
		{
			if(log_sets)
				gErrorMsg(*log_sets, "ParseFrom")  << "kRejectRequiredSettingNotPresent" << endl;
			
			// no types specified?? error
			// Then no client ID is found.
			return kRejectRequiredSettingNotPresent;
		}
	}

    return kConfOk;
}	

ostream& BcmDsgClientId::Print( ostream& ostrm ) const
{    

#if !(BCM_REDUCED_IMAGE_SIZE)

    ostrm 
        << " Client ID Settings:\n";
        

	if( fIdType == kMacAddressType )
	{
		ostrm
			<< "\tType = " << IdTypeName((uint32)fIdType) <<" ID = " << fClientAddr << endl;
	}
	else
	{
		ostrm
			<< "\tType = " << IdTypeName((uint32)fIdType)  <<" ID = " << (uint32)fClientId << endl;
	}


#endif

    return ostrm;
}

const char* BcmDsgClientId::IdTypeName( uint32 value ) const
{
	char* names[] = 
	{
		(char *) "[ CableCard Client Type ]",	// = 0
		(char *) "[ Broadcast Type ]",			// = 1
		(char *) "[ Mac Address Type ]",		// = 2 
		(char *) "[ Ca System Type ]",			// = 3
		(char *) "[ Application Type ]",		// = 4
		(char *) "[ Application Name Type ]",	// = 5
		(char *) "[ Vendor Type ]"              // = 6
	};

	return names[ value ];
}


byte BcmDsgClientId::ParseBroadcastType( const BcmOctetBuffer &obuf, unsigned int &offset,
    BcmMessageLogSettings* log_sets )
{
	bool fBroadcastTypeSpecified = false;
	TLVParserResult result = kTLVParserError;
	uint8 lengthField = obuf.BytesUsed() - 2;

	switch( lengthField )
	{
		case 0:
			{
				TLVTable tlvTable[] =
				{
					// settings common to DCD top level.
					{ kBroadcast,            0,	kTLV_uint8, &fClientId, false, false, (char *) "Broadcast", &fBroadcastTypeSpecified },   

					// This must always be the last one in the table.
					kEndOfTLVTable
				};

				// Parse the TLVs until end of input or special case exit.
				result = ParseTLVs(obuf, offset, tlvTable, log_sets);

			}
			break;

		case 2:
			{
				TLVTable tlvTable[] =
				{
					// settings common to DCD top level.
					{ kBroadcast,            2,	kTLV_uint16, &fClientId, false, false, (char *) "Broadcast", &fBroadcastTypeSpecified },   

					// This must always be the last one in the table.
					kEndOfTLVTable
				};

				// Parse the TLVs until end of input or special case exit.
				result = ParseTLVs(obuf, offset, tlvTable, log_sets);
			}
			break;

		default:
			{
				if(log_sets)
					gErrorMsg(*log_sets, "ParseBroadcastType")  << "Broadcast Type Client ID Length Value " << lengthField <<" is invalid" << endl;
				
				return kRejMsgSyntaxError;
			}
			break;

	}

    // Check for an error during parsing.  Do we skip over unrecognized type
    // codes, or do we error?????  We should skip over rather than error; this
    // is done for us by the parser.
    if( result >= kTLVParserError )
    {
        // Log this to the debugger...
        if (log_sets)
        {
            gErrorMsg(*log_sets, "ParseBroadcastType") << "Failed to parse Broadcast Type Client ID TLVs" << endl;
        }

        return kRejMsgSyntaxError;
    }

	if( (lengthField == 2) && (fClientId == 0))
	{
		if(log_sets)
			gErrorMsg(*log_sets, "ParseBroadcastType")  << "The DCD contains a DSG Broadcast ID TLV of Length 2 and Value 0." << endl;

		return kRejOutOfRangeError;
	}

	// Set type
	if( fBroadcastTypeSpecified )
	{
		fIdType = kBroadcastType;
	}
	else
	{
		// no types specified?? error
		// Then no client ID is found.
		return kRejectRequiredSettingNotPresent;
	}

	return kConfOk;
}

void BcmDsgClientId::ClientAddress(BcmMacAddress &id)
{
	fClientAddr = id;
}

