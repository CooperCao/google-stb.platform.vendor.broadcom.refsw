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
//  $Id$
//
//  Filename:       EstbSnmpV3NotifyReceiver.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  15 NOV 2006
//
//****************************************************************************
//  Description:
//      Class BcmSnmpV3NotifyReceiver encapsulates the eSTB type 38 
//      configuration file setting.  
//
//****************************************************************************

#include "EstbSnmpV3NotifyReceiver.h"
#include "EstbTLVcodes.h"
#include "EstbTLV.h"
#include "MessageLogSettings.h"
#include "MessageLog.h"
#include "EventLogApi.h"
#include "EstbEvents.h"


BcmSnmpV3NotifyReceiver::BcmSnmpV3NotifyReceiver()
  :
    fIpAddress (0,0,0,0),
    fFilterParams( 255 ),
    fSecurityName( 255 )
{
  fPortSpecified = false;
  fIpAddressSpecified = false;
  fTypeSpecified = false;
  fPort = 162;
  fType; // ?
  fRetries = 3; 
  fTimeout = 15000; 
}


BcmSnmpV3NotifyReceiver::BcmSnmpV3NotifyReceiver( const BcmSnmpV3NotifyReceiver& rhs )
  :
    fIpAddress( rhs.fIpAddress ),
    fFilterParams( rhs.fFilterParams ) ,
    fSecurityName (rhs.fSecurityName )
{
  fPortSpecified = rhs.fPortSpecified;
  fIpAddressSpecified = rhs.fIpAddressSpecified;
  fTypeSpecified = rhs.fTypeSpecified;
  fPort = rhs.fPort;
  fType = rhs.fType;
  fTimeout = rhs.fTimeout;
  fRetries = rhs.fRetries;
}


BcmSnmpV3NotifyReceiver& BcmSnmpV3NotifyReceiver::operator=( 
    const BcmSnmpV3NotifyReceiver& rhs )
{
    if( this == &rhs )
    {
        // special case: assignment to self
        return *this;
    }

    fPortSpecified = rhs.fPortSpecified;
    fIpAddressSpecified = rhs.fIpAddressSpecified;
    fTypeSpecified = rhs.fTypeSpecified;
    fIpAddress = rhs.fIpAddress;
    fFilterParams = rhs.fFilterParams;
    fSecurityName = rhs.fSecurityName;
    fPort = rhs.fPort;
    fType = rhs.fType;
    fTimeout = rhs.fTimeout;
    fRetries = rhs.fRetries;

    return *this;
}


bool BcmSnmpV3NotifyReceiver::operator==( const BcmSnmpV3NotifyReceiver& rhs ) const
{
    bool ret_val = false;
    
    
    if ((fIpAddress == rhs.fIpAddress)
    &&  (fFilterParams == rhs.fFilterParams)
    &&  (fSecurityName== rhs.fSecurityName)
    &&  (fPort == rhs.fPort)
    &&  (fPortSpecified == rhs.fPortSpecified)
    &&  (fIpAddressSpecified == rhs.fIpAddressSpecified)
    &&  (fTypeSpecified == rhs.fTypeSpecified)
    &&  (fType == rhs.fType)
    &&  (fRetries == rhs.fRetries))
    {
        ret_val = true;
    }
    
    return ret_val;
}


void BcmSnmpV3NotifyReceiver::Reset()
{
    fPortSpecified = false;
    fIpAddressSpecified = false;
    fTypeSpecified = false;
    fIpAddress.reset();
    fPort = 162;
    fType = 0; // ?
    fRetries = 3; 
    fTimeout = 15000;
    fFilterParams.Empty();
    fSecurityName.Empty();
}


ostream& BcmSnmpV3NotifyReceiver::Print( ostream& ostrm ) const
{

#if !(BCM_REDUCED_IMAGE_SIZE)

    ostrm << "\nSNMP Notify Receiver:"
          << "\n     IP address: " << fIpAddress
          << ( fIpAddressSpecified ? spec : no_spec )
          << "\n     Port: " << fPort
          << ( fPortSpecified ? spec : no_spec )
          << "\n     Type: " << fType
          << ( fTypeSpecified ? spec : no_spec )
          << "\n     Timeout: " << fTimeout 
          << "\n     Retries: " << fRetries 
          << "\n     Filter Params: " << fFilterParams 
          << "\n     Security Name: " << fSecurityName
          << endl;

#endif

    return ostrm;
}



byte BcmSnmpV3NotifyReceiver::ParseFrom( const BcmOctetBuffer &octetBuffer, 
    unsigned int &offset, BcmMessageLogSettings* log_sets,bool lengthIsUint16 )
{
    TLVTable tlvTable[] =
    {   
		// TODO: Check while adding IPv6 capability
        { kSnmpNotifyIp, BcmIpV4Address::ADDR_LEN, kTLV_IpAddress, &fIpAddress, false, false, (char *) "notify IP address", &fIpAddressSpecified, true },
        { kSnmpNotifyPort, 2, kTLV_uint16, &fPort, false, false, (char *) "notify UDP port", &fPortSpecified, true },
        { kSnmpNotifyType, 2, kTLV_uint16, &fType, false, false, (char *) "notify type", &fTypeSpecified, true },
        { kSnmpNotifyTimeout, 2, kTLV_uint16, &fTimeout, false, false, (char *) "notify timeout", NULL, true },
        { kSnmpNotifyRetries, 2, kTLV_uint16, &fRetries, false, false, (char *) "notify retries", NULL, true },
        { kSnmpNotifyFilterParams, kUnknownLength, kTLV_OctetBuffer, &fFilterParams, false, false, (char *) "notify filter params", NULL, true },
        { kSnmpNotifySecurityName, kUnknownLength, kTLV_OctetBuffer, &fSecurityName, false, false, (char *) "notify security name", NULL, true },
        
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
        gLogMessageRaw << "ERROR:  Failed to parse SnmpV3NotifyReceiver TLVs!" << endl;

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
        gLogMessageRaw << "ERROR:  Failed to parse SnmpV3NotifyReceiver TLVs!" << endl;

        return kConfSyntaxError;
    }

 
    return kConfOK;
}

bool BcmSnmpV3NotifyReceiver::IsValid( void )
{
  bool retVal = true;

  // If we got this far, it must be good!
  return retVal;
}
