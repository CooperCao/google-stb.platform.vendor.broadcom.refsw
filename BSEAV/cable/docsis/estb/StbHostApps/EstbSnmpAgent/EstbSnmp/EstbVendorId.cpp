//****************************************************************************
//
// Copyright (c) 2010 Broadcom Corporation
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
//  Filename:       EstbVendorId.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  28 DEC 2010
//
//****************************************************************************
//  Description:
//      Class BcmVendorId encapsulates the sub-id 43.8 of
//      eSTB TLV type 43 configuration file setting.  
//
//****************************************************************************

#include "EstbVendorId.h"
#include "EstbTLVcodes.h"
#include "EstbTLV.h"
#include "MessageLogSettings.h"
#include "MessageLog.h"
#include "EstbEventLog.h"
#include "EstbEvents.h"


BcmVendorId::BcmVendorId()
{
  fVendorId.Empty();
}


BcmVendorId::BcmVendorId( const BcmVendorId& rhs )
  :
    fVendorId( rhs.fVendorId )
{
}


BcmVendorId& BcmVendorId::operator=( 
    const BcmVendorId& rhs )
{
    if( this == &rhs )
    {
        // special case: assignment to self
        return *this;
    }

    fVendorId = rhs.fVendorId;

    return *this;
}


bool BcmVendorId::operator==( const BcmVendorId& rhs ) const
{
    if (fVendorId == rhs.fVendorId)
    {
        return true;
    }
    
    return false;
}


void BcmVendorId::Reset()
{
    fVendorId.Empty();
}


ostream& BcmVendorId::Print( ostream& ostrm ) const
{

#if !(BCM_REDUCED_IMAGE_SIZE)

    ostrm << "\nVendor Id:"
          << "\n     Vendor Id: " << fVendorId
          << endl;

#endif

    return ostrm;
}

byte BcmVendorId::ParseFrom( const BcmOctetBuffer &octetBuffer, 
    unsigned int &offset, BcmMessageLogSettings* log_sets, bool lengthIsUint16 )
{
    TLVTable tlvTable[] =
    {   
	// TODO: Check for actual vendor stuff, if we ever need it in Estb SNMP Agent
        { kVendorId, 3, kTLV_OctetBuffer, &fVendorId, false, false, (char *) "VendorId", NULL, true },
        
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
    TLVParserResult result = ParseTLVs(octetBuffer, offset, tlvTable, log_sets,lengthIsUint16);

    // Check for an error during parsing.
    if( result >= kTLVParserError )
    {
      // Log this to the debugger...
      gLogMessageRaw << "ERROR:  Failed to parse VendorId TLVs!" << endl;

      return kConfSyntaxError;
    }
    
    while (( result == kTLVParserStopParsingFound ) ||
           (result == kTLVParserDuplicateTLV))
    {
      // If we had duplicate TLVs, log the event and keep going
      if (result == kTLVParserDuplicateTLV) {
        EstbLogEvent(kEE_T45303);
      } else {

      uint8 typeField, lengthField;

        if (!octetBuffer.NetworkRead(offset, typeField) ||
            !octetBuffer.NetworkRead(offset, lengthField))
        {
          return kConfSyntaxError;
        }
      }

      // start the parser once again

      // Parse the TLVs until end of input or special case exit.
      result = ParseTLVs(octetBuffer, offset, tlvTable, log_sets);
    }
 
    // Check for an error during parsing. 
    if( result >= kTLVParserError )
    {
        // Log this to the debugger...
        gLogMessageRaw << "ERROR:  Failed to parse EstbConfig Transport TLVs!" << endl;

        return kConfSyntaxError;
    }

    return kConfOK;

}

bool BcmVendorId::IsValid( void )                                  
{
  bool retVal = true;

  // Per CFR must have Vendor Id
  if (fVendorId.BytesUsed() == 0) {
    retVal = false;
    EstbLogEvent(kEE_T44301);
    gLogMessageRaw << "ERROR: No Vendor Id in Vendor Specific TLV!" << endl;
  }

  // If we got this far, it must be good!
  return retVal;
}
