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
//  $Id$
//
//  Filename:       EstbSnmpV1V2Coexistence.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  15 NOV 2006
//
//****************************************************************************
//  Description:
//      Class BcmSnmpV1V2Coexistence encapsulates the eSTB TLV type 53 
//      configuration file setting.  
//
//****************************************************************************

#include "EstbSnmpV1V2Coexistence.h"
#include "EstbTLVcodes.h"
#include "EstbTLV.h"
#include "MessageLogSettings.h"
#include "MessageLog.h"
#include "EstbEventLog.h"
#include "EstbEvents.h"

BcmSnmpV1V2Coexistence::BcmSnmpV1V2Coexistence()
  :
    fCommunityName( 255 ),
    fViewName( 255 )
{
  fViewTypeSpecified = false;
  fViewType = 1; // ?
}


BcmSnmpV1V2Coexistence::BcmSnmpV1V2Coexistence( const BcmSnmpV1V2Coexistence& rhs )
  :
    fCommunityName (rhs.fCommunityName ),
    fViewName (rhs.fViewName )
{
  fViewTypeSpecified = rhs.fViewTypeSpecified;
  fViewType = rhs.fViewType;
}


BcmSnmpV1V2Coexistence& BcmSnmpV1V2Coexistence::operator=( 
    const BcmSnmpV1V2Coexistence& rhs )
{
    if( this == &rhs )
    {
        // special case: assignment to self
        return *this;
    }

    fViewTypeSpecified = rhs.fViewTypeSpecified;
    fCommunityName = rhs.fCommunityName;
    fViewType = rhs.fViewType;
    fViewName = rhs.fViewName;

    return *this;
}


bool BcmSnmpV1V2Coexistence::operator==( const BcmSnmpV1V2Coexistence& rhs ) const
{
    bool ret_val = false;
    
    
    if ((fViewTypeSpecified == rhs.fViewTypeSpecified)
    &&  (fCommunityName== rhs.fCommunityName)
    &&  (fViewType == rhs.fViewType)
    &&  (fViewName== rhs.fViewName))
    {
        ret_val = true;
    }
    
    return ret_val;
}


void BcmSnmpV1V2Coexistence::Reset()
{
    fViewTypeSpecified = false;
    fCommunityName.Empty();
    fViewType = 1; 
    fViewName.Empty();
}


ostream& BcmSnmpV1V2Coexistence::Print( ostream& ostrm ) const
{

#if !(BCM_REDUCED_IMAGE_SIZE)

    ostrm << "\nSNMP V1V2 Coexistence:"
          << "\n     Community Name: " << fCommunityName
          << "\n     Transport Tag: " << fTag
          << "\n     View Type: " << fViewType
          << ( fViewTypeSpecified ? spec : no_spec )
          << "\n     View Name: " << fViewName
          << endl;

#endif

    return ostrm;
}


bool BcmSnmpV1V2Coexistence::ParseV1V2Transport( const BcmOctetBuffer &obuf, unsigned int &offset,
    uint8 length, BcmMessageLogSettings* log_sets )
{
    // create temporary octet buf and offset.
    BcmOctetBuffer tmp_obuf(0);
    unsigned int tmp_offset = 0;

    // create parent obj.
    BcmSnmpV1V2Transport transport;

    // load an octet buf with the V bytes of the parent
    // (a.k.a. the child TLV's).
    tmp_obuf.Overlay( obuf, offset, length );
    tmp_offset = 0;

    // have parent obj load the kids from tempBuffer
    byte parse_conf_code = transport.ParseFrom( tmp_obuf, tmp_offset, log_sets );
    if( parse_conf_code == kConfOK )
    {
      // if all is well, store parent obj.
      if (transport.IsValid()) {

        fSnmpV1V2TransportList.push_back( transport );
        // gLogMessageRaw << "V1V2 Coexistence Transport valid " << transport << endl;

      } else {
        gLogMessageRaw << "V1V2 Coexistence Transport parsed OK, but invalid!" << endl;
      }

      // update offset of top level buffer.
      offset += tmp_offset;

      return true;
    }

    gLogMessageRaw << "V1V2 Coexistence Transport parse error!" << endl;
    return false;
}

byte BcmSnmpV1V2Coexistence::ParseFrom( const BcmOctetBuffer &octetBuffer, 
    unsigned int &offset, BcmMessageLogSettings* log_sets,bool lengthIsUint16 )
{
    TLVTable tlvTable[] =
    {   
        { kSnmpV1V2CommunityName, kUnknownLength, kTLV_OctetBuffer, &fCommunityName, false, false, (char *) "V1V2 community name", NULL, true },
        { kSnmpV1V2Transport, kUnknownLength, kTLV_nested, {NULL}, true, false, (char *) "V1V2 transport", NULL, false },
        { kSnmpV1V2ViewType, 1, kTLV_uint8, &fViewType, false, false, (char *) "V1V2 view type", &fViewTypeSpecified, true },
        { kSnmpV1V2ViewName, kUnknownLength, kTLV_OctetBuffer, &fViewName, false, false, (char *) "V1V2 view name", NULL, true },
        
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
      gLogMessageRaw << "ERROR:  Failed to parse V1V2Coexistence TLVs!" << endl;

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

        // parser found a mutant type.  examine it.
        switch( typeField )
        {
            case kSnmpV1V2Transport:
                // call helper function.
                if( !ParseV1V2Transport( octetBuffer, offset, lengthField, log_sets) )
                {
                    // only a syntax error or memory allocation failure can cause
                    // parse failure for ParseSnmpV1V2Transport().
                    return kConfRejOther;
                }
                break;

        }
      }

        // start the parser once again

        // Parse the TLVs until end of input or special case exit.
        result = ParseTLVs(octetBuffer, offset, tlvTable, log_sets);
    }

    // Now, create the Transport Tag, which is just all the transports jammed
    // into one buffer, separated by a space.
    char tagString[16];
    fTag.Empty();
    int tagCounter = 0;
    // printf ("Processing fSnmpV1V2TransportList\n");
    if (!fSnmpV1V2TransportList.empty()) {
      SnmpV1V2TransportList::const_iterator transport_it = fSnmpV1V2TransportList.begin();
      sprintf(tagString, "@STBconfigTag_%d", tagCounter++); 
      // printf ("Processing fSnmpV1V2TransportList tagString %s\n", tagString);
      fTag.AddToEnd(tagString, false);
      while( transport_it != fSnmpV1V2TransportList.end() ) {
        sprintf(tagString, " @STBconfigTag_%d", tagCounter++); 
      // printf ("Processing fSnmpV1V2TransportList tagString %s\n", tagString);
        fTag.AddToEnd(tagString, false);
        ++transport_it;
      } 
    }
    // cout << "Done Processing fSnmpV1V2TransportList. fTag :" << fTag << endl;
 
    return kConfOK;

}

bool BcmSnmpV1V2Coexistence::IsValid( void )
{
  bool retVal = true;

  // REQ 3684 - Must have community name
  if (fCommunityName.BytesUsed() == 0) {
    gLogMessageRaw << "No Community Name in V1V2 Coexistence TLV!" << endl;
    EstbLogEvent(kEE_T45301);
    retVal = false;
  }

  // REQ 3685 - Must have transport
  if (fSnmpV1V2TransportList.empty()) {
    gLogMessageRaw << "No Transport in V1V2 Coexistence TLV!" << endl;
    EstbLogEvent(kEE_T45302);
    retVal = false;
  } 

  if (fViewTypeSpecified) 
    if ((fViewType != 1) && (fViewType != 2)) {
      gLogMessageRaw << "SNMP Syntax Error in V1V2 Coexistence TLV!" << endl;
      EstbLogEvent(kEE_T45304);
      retVal = false;
    } 

  // If we got this far, it must be good!
  return retVal;
}

