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
//  Filename:       EstbConfig.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  15 NOV 2006
//
//****************************************************************************
//  Description:
//      Class BcmEstbConfig encapsulates the information of an eSTB
//      configuration file.
//
//****************************************************************************

//********************** Include Files ***************************************
#include "EstbConfig.h"
#include <string.h>
#include "EstbTLVcodes.h"
#include "EstbTLV.h"
#include <stdio.h>
#include "EventLogApi.h"
#include "EstbEvents.h"

#include "MessageLog.h"

BcmEstbConfig::~BcmEstbConfig()
{
}

BcmEstbConfig::BcmEstbConfig()
  :
    fSnmpMibObjectList(),
    fSnmpMibObjTotal ( 0 ),
    // fSnmpV3KickList(),
    fSnmpV3NotifyList(),
    fSnmpV1V2CoexistenceList(),
    fSnmpV3AccessViewList(),
    fUsageContext( kEstbConfigFile ),
    fConfigSpecified (false)
{
}

BcmEstbConfig::BcmEstbConfig( const BcmEstbConfig& rhs )
  :
    fSnmpMibObjectList(),
    fSnmpMibObjTotal ( 0 ),
    // fSnmpV3KickList(),
    fSnmpV3NotifyList(),
    fSnmpV1V2CoexistenceList(),
    fSnmpV3AccessViewList(),
    fUsageContext( rhs.fUsageContext ),
    fConfigSpecified (false)
{

    // For STL containers, the Diab compiler doesn't support copy construction
    // in initializer lists.  Thus, we use the default constructor in the list
    // above, then use copy semantics here.
    fSnmpMibObjectList = rhs.fSnmpMibObjectList;
    // fSnmpV3KickList = rhs.fSnmpV3KickList;
    fSnmpV3NotifyList = rhs.fSnmpV3NotifyList;
    fSnmpV1V2CoexistenceList = rhs.fSnmpV1V2CoexistenceList;
    fSnmpV3AccessViewList = rhs.fSnmpV3AccessViewList;
}


BcmEstbConfig& BcmEstbConfig::operator=( const BcmEstbConfig& rhs )
{
    if( this == &rhs )
    {
        // special case: assignment to self
        return *this;
    }

    fSnmpMibObjectList = rhs.fSnmpMibObjectList;
    // fSnmpV3KickList = rhs.fSnmpV3KickList;
    fSnmpV3NotifyList = rhs.fSnmpV3NotifyList;
    fSnmpV1V2CoexistenceList = rhs.fSnmpV1V2CoexistenceList;
    fSnmpV3AccessViewList = rhs.fSnmpV3AccessViewList;

    fUsageContext = rhs.fUsageContext;
    fConfigSpecified = rhs.fConfigSpecified;
    return *this;
}



void BcmEstbConfig::Reset()
{

    fSnmpMibObjTotal = 0;
    fConfigSpecified = false;

    if (!fSnmpV3NotifyList.empty() )
    {
        fSnmpV3NotifyList.clear();
    }

    if (!fSnmpV1V2CoexistenceList.empty() )
    {
        fSnmpV1V2CoexistenceList.clear();
    }

    if (!fSnmpV3AccessViewList.empty() )
    {
        fSnmpV3AccessViewList.clear();
    }

    fUsageContext = kEstbConfigFile;
}


byte BcmEstbConfig::ParseFrom(const BcmOctetBuffer &octetBuffer,
    unsigned int &offset, BcmMessageLogSettings* log_sets )
{
    byte conf_code = 0;

    // unfortunately we have context specific parsing...
    switch( fUsageContext )
    {
        case kEstbConfigFile:
            conf_code =  ParseFromConfigFile( octetBuffer, offset, log_sets );
            break;

    }
    return conf_code;
}

byte BcmEstbConfig::ParseFromConfigFile(const BcmOctetBuffer &octetBuffer,
    unsigned int &offset, BcmMessageLogSettings* log_sets )
{
    byte conf_code;

	TLVTable tlvTable[] =
    {
        // settings common to config file 
        { kIpModeControl, 1, kTLV_int8, NULL, false,  false, (char *) "Ip Mode Control", NULL, true },
        { kSnmpMibObject, kUnknownLength, kTLV_OctetBuffer, NULL, true,  false, (char *) "SNMP MIB object", NULL, false },
        { kVendorSpecific, kUnknownLength, kTLV_nested, NULL, true,  false, (char *) "Vendor Specific",  NULL, false },
        { kSnmpV3NotifyReceiver, kUnknownLength, kTLV_nested, NULL, true,  false, (char *) "SNMP3 notify receiver",  NULL, false },
        { kSnmpV1V2Coexistence, kUnknownLength, kTLV_nested, NULL, true,  false, (char *) "SNMPv1v2 Coexistence",  NULL, false },
        { kSnmpV3AccessView, kUnknownLength, kTLV_nested, NULL, true,  false, (char *) "SNMPv3 Access View",  NULL, false },
        // this allows unstripped TLV 217s, which we probably don't want
        // { kEstbConfig, kUnknownLength, kTLV_nested, NULL, true,  false, (char *) "eSTB config (TLV 217)", NULL, false },

        // This must always be the last one in the table.
        kEndOfTLVTable 
        // kEndOfTLVTableStopParsing
    };

#ifdef TEST_TLVS
     if (TestTLVs())
        gLogMessageRaw << "Test TLV passed!" << endl;
     else
        gLogMessageRaw << "Test TLV failed!" << endl;
#endif

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

    // Check for an error during parsing.
    if( result >= kTLVParserError )
    {
      // Log this to the debugger...
      gLogMessageRaw << "ERROR:  Failed to parse basic EstbConfig TLV 217!" << endl;
      return kConfSyntaxError;
    }

    while (( result == kTLVParserStopParsingFound ) || 
           (result == kTLVParserDuplicateTLV))
    {

      // If we had duplicate TLVs, log the event, then continue
      if (result == kTLVParserDuplicateTLV) {
        EstbLogEvent(kEE_T45303);
      } else {
        uint8 typeField, lengthField;

        if (!octetBuffer.NetworkRead(offset, typeField) ||
            !octetBuffer.NetworkRead(offset, lengthField))
        {
          return kConfSyntaxError;
        }

        // printf("EstbConfig got TLV %d \n", typeField);
        // parser found a mutant type.  examine it.
        switch( typeField )
        {
            case kSnmpMibObject:
                fSnmpMibObjTotal++;
                // printf("EstbConfig got %d raw TLV 11s\n", fSnmpMibObjTotal);
                // call helper function.
                if( !ParseOctetBufferListItem( octetBuffer, offset, lengthField,
                    fSnmpMibObjectList, log_sets ) )
                {
                    // only a syntax error or memory allocation failure can cause
                    // parse failure for ParseOctetBufferListItem().
                    return kConfRejOther;
                }
                fConfigSpecified = true;
                break;

            case kVendorSpecific:
                // call helper function.
                if( !ParseVendorSpecific( octetBuffer, offset, lengthField, log_sets ) )
                {
                    // only a syntax error or memory allocation failure can cause
                    // parse failure for ParseVendorPecific().
                    return kConfRejOther;
                }
                fConfigSpecified = true;
                break;

            case kSnmpV3NotifyReceiver:
                // call helper function.
                if( !ParseSnmpV3NotifyReceiver( octetBuffer, offset, lengthField, log_sets ) )
                {
                    // only a syntax error or memory allocation failure can cause
                    // parse failure for ParseSnmpV3NotifyReceiver().
                    return kConfRejOther;
                }
                fConfigSpecified = true;
                break;

            case kSnmpV1V2Coexistence:
                // call helper function.
                if( !ParseSnmpV1V2Coexistence( octetBuffer, offset, lengthField, log_sets ) )
                {
                    // only a syntax error or memory allocation failure can cause
                    // parse failure for ParseSnmpV1V2Coexistence().
                    return kConfRejOther;
                }
                fConfigSpecified = true;
                break;

            case kSnmpV3AccessView:
                // call helper function.
                if( !ParseSnmpV3AccessView( octetBuffer, offset, lengthField, log_sets ) )
                {
                    // only a syntax error or memory allocation failure can cause
                    // parse failure for ParseSnmpV3AccessView().
                    return kConfRejOther;
                }
                fConfigSpecified = true;
                break;

          }
        }

        // start the parser once again
        // Parse the TLVs until end of input or special case exit.
        result = ParseTLVs(octetBuffer, offset, tlvTable, log_sets);
    }

    // validate the parsed configuration with rules for kEstbConfigFile context.
    conf_code = ValidateConfigFile( log_sets );
    return conf_code;
}


byte BcmEstbConfig::ValidateConfigFile( BcmMessageLogSettings* /*log_sets*/ )
{
  byte conf_code = kConfOK;

  if (!fConfigSpecified)
    conf_code = kConfMissingTLV217;

  return conf_code;
}



ostream& BcmEstbConfig::Print( ostream& ostrm ) const
{

#if !(BCM_REDUCED_IMAGE_SIZE)

    ostrm
        << "\n          usage context: " << inHex(fUsageContext) << " (" << UsageContextName( fUsageContext ) << ")";


    if( fUsageContext == kEstbConfigFile )
    {
        // only print these settings if context is eSTB config file...
        if( !fSnmpMibObjectList.empty() )
        {
            OctetBufferList::const_iterator mib_obj_it = fSnmpMibObjectList.begin();
            while( mib_obj_it != fSnmpMibObjectList.end() )
            {
                ostrm << "\n        SNMP MIB object:" << (*mib_obj_it);
                ++mib_obj_it;
            }
        }

    }

#endif

    return ostrm;
}



void BcmEstbConfig::UsageContext( int new_val )
{
    if( fUsageContext != new_val )
    {
        // new value changes current value...
        // store new value
        fUsageContext = new_val;

    }
}



bool BcmEstbConfig::IsValid()
{
  // if code gets here, all is well.
  return true;
}


#ifdef NOTYET
bool BcmEstbConfig::ParseSnmpV3KickstartValue( const BcmOctetBuffer &obuf, unsigned int &offset,
    uint8 length, BcmMessageLogSettings* log_sets )
{
    // create temporary octet buf and offset.
    BcmOctetBuffer tmp_obuf(0);
    unsigned int tmp_offset = 0;

    // create parent obj.
    BcmSnmpV3KickstartValue kick_val;

    // load an octet buf with the V bytes of the parent
    // (a.k.a. the child TLV's).
    tmp_obuf.Overlay( obuf, offset, length );
    tmp_offset = 0;

    // have parent obj load the kids from tempBuffer
    byte parse_conf_code = kick_val.ParseFrom( tmp_obuf, tmp_offset, log_sets );
    if( parse_conf_code == kConfOK )
    {
        // all is well.  store parent obj.
        fSnmpV3KickList.push_back( kick_val );

        // update offset of top level buffer.
        offset += tmp_offset;

        return true;
    }

    return false;
}

#endif


bool BcmEstbConfig::ParseVendorSpecific( const BcmOctetBuffer &obuf, unsigned int &offset,
    uint8 length, BcmMessageLogSettings* log_sets )
{
    // cout << "BcmEstbConfig::ParseSnmpV3NotifyReceiver" << endl;
    // create temporary octet buf and offset.
    BcmOctetBuffer tmp_obuf(0);
    unsigned int tmp_offset = 0;

    // create parent obj.
    BcmVendorId vendorId;

    // load an octet buf with the V bytes of the parent
    // (a.k.a. the child TLV's).
    tmp_obuf.Overlay( obuf, offset, length );
    tmp_offset = 0;

    // have parent obj load the kids from tempBuffer
    byte parse_conf_code = vendorId.ParseFrom( tmp_obuf, tmp_offset, log_sets );
    if( parse_conf_code == kConfOK )
    {
        // if all is well, store parent obj.
        if (vendorId.IsValid()) {
          // fVendorIdList.push_back( notify_rcvr );

        } else {
          gLogMessageRaw << "Vendor Id parsed OK, but invalid!" << endl;
        }

        // update offset of top level buffer.
        offset += tmp_offset;

        return true;
   }

    return false;
}


bool BcmEstbConfig::ParseSnmpV3NotifyReceiver( const BcmOctetBuffer &obuf, unsigned int &offset,
    uint8 length, BcmMessageLogSettings* log_sets )
{
    // cout << "BcmEstbConfig::ParseSnmpV3NotifyReceiver" << endl;
    // create temporary octet buf and offset.
    BcmOctetBuffer tmp_obuf(0);
    unsigned int tmp_offset = 0;

    // create parent obj.
    BcmSnmpV3NotifyReceiver notify_rcvr;

    // load an octet buf with the V bytes of the parent
    // (a.k.a. the child TLV's).
    tmp_obuf.Overlay( obuf, offset, length );
    tmp_offset = 0;

    // have parent obj load the kids from tempBuffer
    byte parse_conf_code = notify_rcvr.ParseFrom( tmp_obuf, tmp_offset, log_sets );
    if( parse_conf_code == kConfOK )
    {
        // if all is well, store parent obj.
        if (notify_rcvr.IsValid()) {
          fSnmpV3NotifyList.push_back( notify_rcvr );

        } else {
          gLogMessageRaw << "V3 Notify List parsed OK, but invalid!" << endl;
        }
        // update offset of top level buffer.
        offset += tmp_offset;

        return true;
        }

    return false;
}


bool BcmEstbConfig::ParseSnmpV1V2Coexistence( const BcmOctetBuffer &obuf, unsigned int &offset,
    uint8 length, BcmMessageLogSettings* log_sets )
{
    // cout << "BcmEstbConfig::ParseSnmpV1V2Coexistence" << endl;
    // create temporary octet buf and offset.
    BcmOctetBuffer tmp_obuf(0);
    unsigned int tmp_offset = 0;

    // create parent obj.
    BcmSnmpV1V2Coexistence coexistence;

    // load an octet buf with the V bytes of the parent
    // (a.k.a. the child TLV's).
    tmp_obuf.Overlay( obuf, offset, length );
    tmp_offset = 0;

    // have parent obj load the kids from tempBuffer
    byte parse_conf_code = coexistence.ParseFrom( tmp_obuf, tmp_offset, log_sets );
    if( parse_conf_code == kConfOK )
    {
        // if all is well, store parent obj.
        if (coexistence.IsValid()) {
          fSnmpV1V2CoexistenceList.push_back( coexistence );

        } else {
          gLogMessageRaw << "V1V2 Coexistence parsed OK, but invalid!" << endl;
        }
        // update offset of top level buffer.
        offset += tmp_offset;

        return true;
    }

    return false;
}


bool BcmEstbConfig::ParseSnmpV3AccessView( const BcmOctetBuffer &obuf, unsigned int &offset,
    uint8 length, BcmMessageLogSettings* log_sets )
{
    // cout << "BcmEstbConfig::ParseSnmpV3AccessView" << endl;
    // create temporary octet buf and offset.
    BcmOctetBuffer tmp_obuf(0);
    unsigned int tmp_offset = 0;

    // create parent obj.
    BcmSnmpV3AccessView access_view;

    // load an octet buf with the V bytes of the parent
    // (a.k.a. the child TLV's).
    tmp_obuf.Overlay( obuf, offset, length );
    tmp_offset = 0;

    // have parent obj load the kids from tempBuffer
    byte parse_conf_code = access_view.ParseFrom( tmp_obuf, tmp_offset, log_sets );
    if( parse_conf_code == kConfOK )
    {
        // if all is well, store parent obj.
        if (access_view.IsValid()) {
          fSnmpV3AccessViewList.push_back( access_view );
        } else {
          gLogMessageRaw << "V3 AccessView parsed OK, but invalid!" << endl;
        }
        // update offset of top level buffer.
        offset += tmp_offset;

        return true;
        }

    return false;
}


bool BcmEstbConfig::ParseOctetBufferListItem( const BcmOctetBuffer &obuf,
    unsigned int &offset, uint8 length, OctetBufferList& dest_obuf_list,
    BcmMessageLogSettings* log_sets )
{
    // create temporary octet buf and offset.
    BcmOctetBuffer tmp_obuf( 256 );

    if( tmp_obuf.Append( obuf, offset, length ) )
    {
        // all is well.  store obuf.
        dest_obuf_list.push_back( tmp_obuf );

        // log the V...
        if (log_sets)
        {
            gInfoMsgNoFields(*log_sets) << tmp_obuf;
        }

        // update offset of top level buffer.
        offset += length;

        return true;
    }

    // Log this to the debugger...
    return false;
}




bool BcmEstbConfig::ParseMacAddrAndAddToList( MacAddrList& dest_list,
    const BcmOctetBuffer &obuf, unsigned int &offset, uint8 length,
    BcmMessageLogSettings* log_sets )
{
    // create temporary octet buf and offset.
    BcmOctetBuffer tmp_obuf(0);
    unsigned int tmp_offset = 0;

    // create parent obj.
    BcmMacAddress mac_addr;

    // load an octet buf with the V bytes of the parent
    // (a.k.a. the child TLV's).
    tmp_obuf.Overlay( obuf, offset, length );
    tmp_offset = 0;

    // have parent obj load the kids from tempBuffer
    if( mac_addr.ReadFrom( tmp_obuf, tmp_offset ) )
    {
        // all is well.
        // store parent obj.
        dest_list.push_back( mac_addr );

        // log the V...
        if (log_sets)
        {
            gInfoMsgNoFields(*log_sets) << mac_addr;
        }

        // update offset of top level buffer.
        offset += tmp_offset;

        return true;
    }

    return false;
}


