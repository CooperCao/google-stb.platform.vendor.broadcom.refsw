/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
//
//****************************************************************************
//  $Id$
//
//  Filename:       EstbSnmpV3NotifyReceiver.h
//  Author:         Maurice Turcotte
//  Creation Date:  15 NOV 2006
//
//****************************************************************************
//  Description:
//      Class BcmSnmpV3NotifyReceiver encapsulates the eSTB type 38 
//      configuration file setting.  
//
//****************************************************************************

#ifndef ESTBSNMPV3NOTIFYRECEIVER_H
#define ESTBSNMPV3NOTIFYRECEIVER_H

//********************** Include Files ***************************************

#include "typedefs.h"
#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <iostream>
#else
#include <iostream.h>
#endif
#include "OctetBuffer.h"
#include "IpAddress.h"

class BcmMessageLogSettings;

class BcmSnmpV3NotifyReceiver
{
public:
    // begin functions required to be param for STL containers...
    //

    // Default constructor.
    //
	BcmSnmpV3NotifyReceiver();

    // Copy constructor.
    //
    // Parameters:
    //      rhs - source object, a.k.a. right hand side.
    //
	BcmSnmpV3NotifyReceiver( const BcmSnmpV3NotifyReceiver& rhs );

    // Destructor.
    //
    ~BcmSnmpV3NotifyReceiver() {}

    // Assignment operator.
    //
    // Parameters:
    //      rhs - source object, a.k.a. right hand side.
    //
    // Returns:
    //      reference to this object
    //
    BcmSnmpV3NotifyReceiver& operator=( const BcmSnmpV3NotifyReceiver& rhs );

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
	bool operator==( const BcmSnmpV3NotifyReceiver& rhs ) const;

   // Less than operator.
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
	bool operator<( const BcmSnmpV3NotifyReceiver& rhs ) const
   {
      if (this < &rhs)
         return true;
      else
         return false;
   }

    // end functions required to be param for STL containers...

    // Reset to default values.
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

	// Load object with the decoded TLV attributes contained in the specified
    // input buffer.
    //
    // Parameters:
    //      octetBuffer - input buffer.
    //
    //      offset - offset into input buffer at which to begin parsing.
    //
    //      log_sets - pointer to BcmMessageLogSettings object which are
    //          used to control debug output.
    //
    // Returns:
    //      "Confirmation code" result, as described in DOCSIS RFI v1.1-IO5
    //      spec App C.4.  A result == 0 is success.  Non-zero result is an
    //      error.
    //
    // Note:
    // It is assumed that the parent T and L codes have already been read
    // from the input buffer.  The starting offset for subsequent processing
    // will be the T code of the first sub-type.
    //
	byte ParseFrom( const BcmOctetBuffer &octetBuffer, unsigned int &offset, 
        BcmMessageLogSettings* log_sets = NULL, bool lengthIsUint16=false );

    bool IsValid();
        
    bool fPortSpecified;
    bool fIpAddressSpecified;
    bool fTypeSpecified;

    // Type 38.1: IP addr of trap receiver
    BcmIpAddress fIpAddress;
    
    // Type 38.2: UDP port number (defaults to 162 if not specified)
    unsigned short fPort;
    
    // Type 38.3: Trap type:
    // 1 = SNMPv1 trap
    // 2 = SNMPv2c trap
    // 3 = SNMP inform, v2c packet
    // 4 = SNMPv2c trap, v3 packet
    // 5 = SNMP inform, v3 packet
    unsigned short fType;
    
    // Type 38.4: inform timeout (milliseconds)
    unsigned short fTimeout;
    
    // Type 38.5: inform retries
    unsigned short fRetries;
    
    // Type 38.6: filter params, this is an ASN.1 encoded OID 
    BcmOctetBuffer fFilterParams;
    
    // Type 38.7: security name for SNMPv3 notification.
    // Defaults to "@config" if not specified
    BcmOctetBuffer fSecurityName;
};

inline ostream& operator<<( ostream& ostrm, const BcmSnmpV3NotifyReceiver& rhs )
{
    return rhs.Print( ostrm );
}

#endif
