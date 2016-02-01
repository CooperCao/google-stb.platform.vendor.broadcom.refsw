//**************************************************************************
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
//
//
//**************************************************************************
//  $Id$
//
//  Filename:       EstbConfig.h
//  Author:         Maurice Turcotte
//  Creation Date:  15 NOV 2006
//
//****************************************************************************
//  Description:
//      Class EstbConfig encapsulates the information of a binary ESTB
//      configuration file.
//
//****************************************************************************

//********************** Include Files ***************************************
#ifndef ESTBCONFIG_H
#define ESTBCONFIG_H

#include "typedefs.h"
#include "IpAddress.h"
#include <string.h>
#include "OctetBuffer.h"
#include "MacAddress.h"

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <iostream>
#include <iomanip>
#include <list>
#else
#include <iostream.h>
#include <iomanip.h>
#include <list.h>
#endif
// #include "EstbKickstartValue.h"
#include "EstbSnmpV3NotifyReceiver.h"
#include "EstbSnmpV1V2Coexistence.h"
#include "EstbSnmpV3AccessView.h"
#include "EstbVendorId.h"

// typedef list< BcmCosReqSettings > CosReqSetsList;
// typedef list< BcmCosRspSettings > CosRspSetsList;
// typedef list< BcmCosErrRsp > CosErrRspList;
typedef list< BcmMacAddress > MacAddrList;
typedef list< BcmOctetBuffer > OctetBufferList;
// typedef list< BcmSnmp3KickstartValue > Snmp3KickList;
typedef list< BcmSnmpV3NotifyReceiver > SnmpV3NotifyList;
typedef list< BcmSnmpV1V2Coexistence > SnmpV1V2CoexistenceList;
typedef list< BcmSnmpV3AccessView > SnmpV3AccessViewList;


/**# :[Description = "eSTB configuration file and settings."]
[Level = Analysis] */
class BcmEstbConfig
{
public:
    // begin functions required to be param for STL containers...
    //

    // Default constructor.
    //
    // Note:
    //      Needed to be type param for STL container.
    //
	BcmEstbConfig();

    // Copy constructor.
    //
    // Parameters:
    //      rhs - source object, a.k.a. right hand side.
    //
    // Note:
    //      Needed to be type param for STL container.
    //
	BcmEstbConfig( const BcmEstbConfig& rhs );

    // Destructor.
    //
    // Note:
    //      Needed to be type param for STL container.
    //
    ~BcmEstbConfig();

    // Assignment operator.
    //
    // Parameters:
    //      rhs - source object, a.k.a. right hand side.
    //
    // Returns:
    //      reference to this object
    //
    // Note:
    //      Needed to be type param for STL container.
    //
	BcmEstbConfig& operator=( const BcmEstbConfig& rhs );
    // end functions required to be param for STL containers...

    // Reset to default values.
    //
	void Reset();

    // This method causes the object to write itself out to the octet buffer in
    // TLV format.  
    //
    // Parameters:
    //      octetBuffer - the buffer into which the TLVs should be written.
    //
    //      log_sets - pointer to BcmMessageLogSettings object which
    //          specifies debug printing options.
    //
    // Returns:
    //      true if the TLVs were written correctly.
    //      false if there was a problem (not enough room in the buffer, etc.).
    //
    // Note: 
    //      Do NOT call this function when preparing a REG-REQ message.
    //      This function does not preserve CM config file settings
    //      order, which is a requirement of REG-REQ message.
    //
    bool WriteTo( BcmOctetBuffer &octetBuffer, BcmMessageLogSettings* log_sets = NULL );

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
    //      log_sets - pointer to BcmMessageLogSettings object which
    //          specifies debug printing options.
    //
    // Returns:
    //      byte - "Confirmation code" result, as described in DOCSIS RFI 
    //          v1.1-IO5 spec App C.4.  A result == 0 is success.  Non-zero 
    //          result is an error.
    //
    byte ParseFrom( const BcmOctetBuffer &octetBuffer, unsigned int &offset, 
        BcmMessageLogSettings* log_sets = NULL );

	// IsValid() - Returns true if config file contents pass validation tests.  
	//		Note:  validation criteria depends on usage context and sometimes
	//		wacky DOCSIS spec and ATP test requirements.
	//
	bool IsValid();

    // accessors
    int UsageContext() const { return fUsageContext; }
    void UsageContext( int new_val );

protected:
    // helper functions called by ParseFrom()...
    //
    // ParseFromConfigFile - Load this object from the specified buffer at the 
    //      specified offset and length, assuming a kCmConfigFile usage context.  
    //
    // Parameters:
    //      obuf - BcmOctetBuffer& from which the new object is to be loaded.
    //
    //      offset - unsigned int& offset into obuf where data for the object 
    //          begins.
    //
    //      log_sets - pointer to BcmMessageLogSettings object which
    //          specifies debug printing options.
    //
    // Returns:
    //      byte - Appendix C. "Confirmation Code."  obuf and offset
    //          reference params are updated by this function.
    //
    byte ParseFromConfigFile( const BcmOctetBuffer &obuf, unsigned int &offset,
        BcmMessageLogSettings* log_sets = NULL );
    //
    //
    //      length - number of bytes to read from obuf.
    //
    //      log_sets - pointer to BcmMessageLogSettings object which
    //          specifies debug printing options.
    //
    // Returns:
    //      bool - true if successful, otherwise false.  obuf and offset
    //          reference params are updated by this function.
    //
    bool ParseMacAddrAndAddToList( MacAddrList& dest_list, 
        const BcmOctetBuffer &obuf, unsigned int &offset,
        uint8 length, BcmMessageLogSettings* log_sets = NULL );
    //
    // ParseSnmp3KickstartValue - Load a new BcmSnmp3KickstartValue object 
    //      from the specified buffer at the specified offset and length.  
    //      Add the resulting BcmSnmp3KickstartValue object to fSnmp3KickList.
    //
    // Parameters:
    //
    //      obuf - BcmOctetBuffer& from which the new object is to be loaded.
    //
    //      offset - unsigned int& offset into obuf where data for the object 
    //          begins.
    //
    //      length - number of bytes to read from obuf.
    //
    //      log_sets - pointer to BcmMessageLogSettings object which
    //          specifies debug printing options.
    //
    // Returns:
    //      bool - true if successful, otherwise false.  obuf and offset
    //          reference params are updated by this function.
    //
    // bool ParseSnmp3KickstartValue( const BcmOctetBuffer &obuf, unsigned int &offset,
    //     uint8 length, BcmMessageLogSettings* log_sets = NULL );
        
    //
    // ParseVendorSpecific - Load a new BcmVendorSpecific object 
    //      from the specified buffer at the specified offset and length.  
    //
    // Parameters:
    //
    //      obuf - BcmOctetBuffer& from which the new object is to be loaded.
    //
    //      offset - unsigned int& offset into obuf where data for the object 
    //          begins.
    //
    //      length - number of bytes to read from obuf.
    //
    //      log_sets - pointer to BcmMessageLogSettings object which
    //          specifies debug printing options.
    //
    // Returns:
    //      bool - true if successful, otherwise false.  obuf and offset
    //          reference params are updated by this function.
    //
    bool ParseVendorSpecific( const BcmOctetBuffer &obuf, unsigned int &offset,
        uint8 length, BcmMessageLogSettings* log_sets = NULL );
        
    //
    // ParseSnmpV3NotifyReceiver - Load a new BcmSnmp3NotifyReceiver object 
    //      from the specified buffer at the specified offset and length.  
    //      Add the resulting BcmSnmp3KickstartValue object to fSnmp3NotifyList.
    //
    // Parameters:
    //
    //      obuf - BcmOctetBuffer& from which the new object is to be loaded.
    //
    //      offset - unsigned int& offset into obuf where data for the object 
    //          begins.
    //
    //      length - number of bytes to read from obuf.
    //
    //      log_sets - pointer to BcmMessageLogSettings object which
    //          specifies debug printing options.
    //
    // Returns:
    //      bool - true if successful, otherwise false.  obuf and offset
    //          reference params are updated by this function.
    //
    bool ParseSnmpV3NotifyReceiver( const BcmOctetBuffer &obuf, unsigned int &offset,
        uint8 length, BcmMessageLogSettings* log_sets = NULL );
        
    //
    // ParseSnmpV1V2Coexistence - Load a new BcmSnmpV1V2Coexistence object 
    //      from the specified buffer at the specified offset and length.  
    //
    // Parameters:
    //
    //      obuf - BcmOctetBuffer& from which the new object is to be loaded.
    //
    //      offset - unsigned int& offset into obuf where data for the object 
    //          begins.
    //
    //      length - number of bytes to read from obuf.
    //
    //      log_sets - pointer to BcmMessageLogSettings object which
    //          specifies debug printing options.
    //
    // Returns:
    //      bool - true if successful, otherwise false.  obuf and offset
    //          reference params are updated by this function.
    //
    bool ParseSnmpV1V2Coexistence( const BcmOctetBuffer &obuf, unsigned int &offset,
        uint8 length, BcmMessageLogSettings* log_sets = NULL );
        
    //
    // ParseSnmpV3AccessView - Load a new BcmSnmp3AccessView object 
    //      from the specified buffer at the specified offset and length.  
    //
    // Parameters:
    //
    //      obuf - BcmOctetBuffer& from which the new object is to be loaded.
    //
    //      offset - unsigned int& offset into obuf where data for the object 
    //          begins.
    //
    //      length - number of bytes to read from obuf.
    //
    //      log_sets - pointer to BcmMessageLogSettings object which
    //          specifies debug printing options.
    //
    // Returns:
    //      bool - true if successful, otherwise false.  obuf and offset
    //          reference params are updated by this function.
    //
    bool ParseSnmpV3AccessView( const BcmOctetBuffer &obuf, unsigned int &offset,
        uint8 length, BcmMessageLogSettings* log_sets = NULL );
        
    //
    // ValidateConfigFile - Validate the parsed configuration with rules  
    //      kCmConfigFile context.  
    //
    // Parameters:
    //      log_sets - pointer to BcmMessageLogSettings object which
    //          specifies debug printing options.
    //
    // Returns:
    //      byte - Appendix C. "Confirmation Code."  
    //
    byte ValidateConfigFile( BcmMessageLogSettings* log_sets = NULL );
    //
    //
    // ParseOctetBufferListItem - Load a new BcmOctetBuffer object from the 
    //      specified buffer at the specified offset and length.  Add the 
    //      resulting BcmOctetBuffer object to the specified OctetBufferList
    //      object.
    //
    // Parameters:
    //
    //      obuf - BcmOctetBuffer& from which the new object is to be loaded.
    //
    //      offset - unsigned int& offset into obuf where data for the object 
    //          begins.
    //
    //      length - number of bytes to read from obuf.
    //
    //      dest_obuf_list - reference to destination OctetBufferList object
    //          to which the new BcmOctetBuffer object shall be added.
    //
    //      log_sets - pointer to BcmMessageLogSettings object which
    //          specifies debug printing options.
    //
    // Returns:
    //      bool - true if successful, otherwise false.  obuf and offset
    //          reference params are updated by this function.
    //
    bool ParseOctetBufferListItem( const BcmOctetBuffer &obuf, unsigned int &offset,
        uint8 length, OctetBufferList& dest_obuf_list, BcmMessageLogSettings* log_sets = NULL );


// UNFINISHED - make data members public until we understand what sort
// of accessers are needed for the list< ??? > data members.
//protected:
public:

    // snmp mib object (optional). TLV type 11.
    OctetBufferList fSnmpMibObjectList;
    int fSnmpMibObjTotal;

    // list of snmp v3 kickstart values (optional). TLV type 34.
    // Snmp3KickList fSnmp3KickList;
    
    // list of snmp v3 notify receiver values (optional). TLV type 38.
    SnmpV3NotifyList fSnmpV3NotifyList;

    // list of snmp v1/v2 coexistence values (optional). TLV type 53.
    SnmpV1V2CoexistenceList fSnmpV1V2CoexistenceList;

    // list of snmp v3 access view values (optional). TLV type 54.
    SnmpV3AccessViewList fSnmpV3AccessViewList;


    // Used by clients to specify context info which determines some print
    // formatting decisions.  Possible values defined in EstbTLVcodes.h
    int fUsageContext;

    // Indicates whether we got any TLV217s at all
    bool fConfigSpecified;
};

inline ostream& operator<<( ostream& ostrm, const BcmEstbConfig& rhs )
{
    return rhs.Print( ostrm );
}

#endif

