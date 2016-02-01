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
//  Filename:       EstbSnmpV3AccessView.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  15 NOV 2006
//
//****************************************************************************
//  Description:
//      Class BcmSnmpV3AccessView encapsulates the eSTB TLV type 54 
//      configuration file setting.  
//
//****************************************************************************

#include "EstbSnmpV3AccessView.h"
#include "EstbTLVcodes.h"
#include "EstbTLV.h"
#include "MessageLogSettings.h"
#include "MessageLog.h"
#include "BcmString.h"
#include "BerObjs.h"
#include "EstbEventLog.h"
#include "EstbEvents.h"


BcmSnmpV3AccessView::BcmSnmpV3AccessView()
{
  // AsnBerOid tmpAsnOid(BcmObjectId(1, 3, 6));
  // BcmString tmpString;

  fName.Empty();

  /* Boy, we have to jump through hoops to get a default OID 
   * so we can just pluck it out again. Yeesh
   */
  // tmpAsnOid.Encode (tmpString);
  // tmpString.Set(fSubtree);

  fSubtree.Empty();
  unsigned long defaultOID[] = {1, 3, 6};
  fSubtreeOID.Set(defaultOID, sizeof(defaultOID)/sizeof(unsigned long));

  fMask.Empty();
  fType = 1; 
  fSubtreeSpecified = false;
  fMaskSpecified = false;
  fTypeSpecified = false;
}


BcmSnmpV3AccessView::BcmSnmpV3AccessView( const BcmSnmpV3AccessView& rhs )
  :
    fName (rhs.fName ),
    fSubtree( rhs.fSubtree ),
    fSubtreeOID( rhs.fSubtreeOID ),
    fMask( rhs.fMask )
{
  fType = rhs.fType;
  fSubtreeSpecified = rhs.fSubtreeSpecified;
  fMaskSpecified = rhs.fMaskSpecified;
  fTypeSpecified = rhs.fTypeSpecified;
}


BcmSnmpV3AccessView& BcmSnmpV3AccessView::operator=( 
    const BcmSnmpV3AccessView& rhs )
{
    if( this == &rhs )
    {
        // special case: assignment to self
        return *this;
    }

    fName = rhs.fName;
    fSubtree = rhs.fSubtree;
    fSubtreeOID = rhs.fSubtreeOID;
    fMask = rhs.fMask;
    fType = rhs.fType;
    fSubtreeSpecified = rhs.fSubtreeSpecified;
    fMaskSpecified = rhs.fMaskSpecified;
    fTypeSpecified = rhs.fTypeSpecified;

    return *this;
}


bool BcmSnmpV3AccessView::operator==( const BcmSnmpV3AccessView& rhs ) const
{
    bool ret_val = false;
    
    
    if ((fName == rhs.fName)
    &&  (fSubtree == rhs.fSubtree)
    &&  (fSubtreeOID == rhs.fSubtreeOID)
    &&  (fSubtreeSpecified == rhs.fSubtreeSpecified)
    &&  (fMask == rhs.fMask)
    &&  (fMaskSpecified == rhs.fMaskSpecified)
    &&  (fType == rhs.fType)
    &&  (fTypeSpecified == rhs.fTypeSpecified))
    {
        ret_val = true;
    }
    
    return ret_val;
}


void BcmSnmpV3AccessView::Reset()
{
  // AsnBerOid tmpAsnOid(BcmObjectId(1, 3, 6));
  // BcmString tmpString;

    fSubtreeSpecified = false;
    fMaskSpecified = false;
    fTypeSpecified = false;
    fName.Empty();

    /* Boy, we have to jump through hoops to get a default OID 
     * so we can just pluck it out again. Yeesh
     */
    // tmpAsnOid.Encode (tmpString);
    // tmpString.Set(fSubtree);

    fSubtree.Empty();
    unsigned long defaultOID[] = {1, 3, 6};
    fSubtreeOID.Set(defaultOID, sizeof(defaultOID)/sizeof(unsigned long));
    fMask.Empty();
    fType = 1; 
}


ostream& BcmSnmpV3AccessView::Print( ostream& ostrm ) const
{

#if !(BCM_REDUCED_IMAGE_SIZE)

    ostrm << "\nSNMP Access View:"
          << "\n     Name: " << fName
          << "\n     Subtree: " << fSubtree
          << "\n     SubtreeOID: " << fSubtreeOID
          << ( fSubtreeSpecified ? spec : no_spec )
          << "\n     Mask: " << fMask
          << ( fMaskSpecified ? spec : no_spec )
          << "\n     Type: " << fType
          << ( fTypeSpecified ? spec : no_spec )
          << endl;

#endif

    return ostrm;
}


byte BcmSnmpV3AccessView::ParseFrom( const BcmOctetBuffer &octetBuffer, 
    unsigned int &offset, BcmMessageLogSettings* log_sets,bool lengthIsUint16 )
{
    TLVTable tlvTable[] =
    {   
        { kSnmpV3AccessViewName, kUnknownLength, kTLV_OctetBuffer, &fName, false, false, (char *) "access view name", NULL, true },
        { kSnmpV3AccessViewSubtree, kUnknownLength, kTLV_OctetBuffer, &fSubtree, false, false, (char *) "access view subtree", &fSubtreeSpecified, true },
        { kSnmpV3AccessViewMask, kUnknownLength, kTLV_OctetBuffer, &fMask, false, false, (char *) "access view mask", &fMaskSpecified, true },
        { kSnmpV3AccessViewType, sizeof(uint8), kTLV_uint8, &fType, false, false, (char *) "access view type", &fTypeSpecified, true },
        
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
        gLogMessageRaw << "ERROR:  Failed to parse V3AccessView TLVs!" << endl;

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
        gLogMessageRaw << "ERROR:  Failed to parse V3AccessView TLVs!" << endl;

        return kConfSyntaxError;
    }


    if (fSubtreeSpecified) {
      if (fSubtree.BytesUsed() > MAX_OID_SIZE)
        return kConfSyntaxError;

      // fSubtree is a BcmOctetBuffer that gets converted to
      // berOid, an AsnBerOid, which decodes the OID and gets converted to
      // fSubtreeOid, a BcmObjectID, what ProvisionVacmViewTreeFamilyEntry() needs
      // Oh yeah, and ProvisionVacmViewTreeFamilyEntry() converts to ObjectId!!

      AsnBerOid berOid(fSubtree.AccessBuffer(), fSubtree.BufferSize());

      if (berOid.Decode() != SNMP_NO_ERROR)
        return kConfSyntaxError;

      berOid.GetValue(fSubtreeOID);   //get the parsed oid into a BcmObjectId object
    } else {
      // just go ahead and use the default oid 1.3.6
      fSubtreeSpecified = true;
    }

    return kConfOK;
}

bool BcmSnmpV3AccessView::IsValid( void )
{
  bool retVal = true;

  // REQ 3695 - Must have view name
  if (fName.BytesUsed() == 0) {
    EstbLogEvent(kEE_T45401); 
    retVal = false;
    gLogMessageRaw << "ERROR: No view name in SNMPv3 Access View TLV!" << endl;
  }

  // If we got this far, it must be good!
  return retVal;
} 
