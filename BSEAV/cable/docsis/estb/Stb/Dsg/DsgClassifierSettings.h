//****************************************************************************
//  
// Copyright (c) 2004-2009 Broadcom Corporation
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
//****************************************************************************
//  Filename:       DsgClassifierSettings.h
//  Author:         Pinar Taskiran 
//				    (adapted from ClassifierSettings.cpp by Cliff Danielson)
//  Creation Date:  Sep 2, 2004
//
//****************************************************************************
//  Description:
//      
//
//****************************************************************************

#ifndef DSG_CLASSIFIERSETTINGS_H
#define DSG_CLASSIFIERSETTINGS_H

//********************** Include Files ***************************************

#include "ClassifierSettings.h"
#include "typedefs.h"

#include "TLVCodes.h"
#include "MacAddress.h"
#include "IpAddress.h"
#include "ErrorSetList.h"

#include <iostream>
#include <iomanip>

class BcmOctetBuffer;


class BcmDsgClassifierSettings 
{
public:

    // begin functions required to be param for STL containers...
    //

    // Default constructor.
    //
	BcmDsgClassifierSettings( byte starting_type_code = 0 );

    // Copy constructor.
    //
    // Parameters:
    //      rhs - source object, a.k.a. right hand side.
    //
	BcmDsgClassifierSettings( const BcmDsgClassifierSettings& rhs );

    // Destructor.
    //
    ~BcmDsgClassifierSettings() {}

    // Assignment operator.
    //
    // Parameters:
    //      rhs - source object, a.k.a. right hand side.
    //
    // Returns:
    //      reference to this object
    //
    BcmDsgClassifierSettings& operator=( const BcmDsgClassifierSettings& rhs );

    // Less than operator.
    //
    // Parameters:
    //      rhs - comparison object, a.k.a. right hand side.
    //
    // Returns:
    //      bool - true if this object is less than the comparison object.
    //
    // Note:
    //      Some compilers (like Borland C++ v5.02) require operator<
    //      for lists even if functions which require it are never
    //      called.  It is used for ordering lists.
    //
	bool operator<( const BcmDsgClassifierSettings& rhs ) const;

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
	bool operator==( const BcmDsgClassifierSettings& rhs ) const;
    // end functions required to be param for STL containers...

    // Print formatted object contents to specified ostream.
    //
    // Parameters:
    //      ostream& ostrm - reference to the destination output stream.
    //
    // Returns:
    //      reference to the destination output stream.
    //
    ostream& Print( ostream& ostrm ) const;

	byte ParseFrom(const BcmOctetBuffer &octetBuffer,
		unsigned int &offset, ErrorSetList& err_set_list, 
		BcmMessageLogSettings* log_sets, bool strict_checking );

	// This method fills the provided buffer in the following
	// order:
	//	typedef struct
	// { 
	//	uint16 cfr_id;
	//	uint8 priority;
	//	uint8 unused;

	//	uint8 source_ip_used;
	//	uint8 source_ip_mask_used;
	//	uint8 dest_ip_used;
	//	uint8 port_used;
	//	uint32 source_ip_adr;
	//	uint32 source_ip_mask;
	//	uint32 dest_ip_adr;
	//	uint16 port_start;
	//	uint16 port_end; 
	// } t_dsg_classifier;

	bool WriteTo( BcmOctetBuffer &octetBuffer ) const;	

public:
    //
    uint16 CmtsClassifierId() const { return fCmtsClassifierId; }
    void CmtsClassifierId( uint16 new_val ) { fCmtsClassifierId = new_val; 
											  fClassifierIdSpecified = true;}
    //


    // accessor for rule priority [22/23].5 - default value 64
    byte RulePriority() const { return fRulePriority; }
    void RulePriority( byte new_val ) 
    { 
        fRulePriority = new_val; 
        fRulePrioritySpecified = true;
    }

    // accessors for IP Classifier Settings
    //
    // accessor for IP protocol type.  Type [22/23].9.2  no default value.
    uint16 ProtocolType() const { return fIpClassifierSets.ProtocolType(); };
    void ProtocolType(uint16 type) { fIpClassifierSets.ProtocolType(type);
                                     fIpClassifierSetsSpecified = true;
                                   };

    // accessor for source IP address.  Type [22/23].9.3  no default value.
    BcmIpAddress SourceIpAddr() const { return fIpClassifierSets.SourceIpAddr(); };
    void SourceIpAddr(BcmIpAddress addr) { fIpClassifierSets.SourceIpAddr(addr);
                                           fIpClassifierSetsSpecified = true;
                                         };

    // accessor for source IP mask.  Type [22/23].9.4  default = 255.255.255.255
    BcmIpAddress SourceIpMask() const { return fIpClassifierSets.SourceIpMask(); };
    void SourceIpMask(BcmIpAddress mask) { fIpClassifierSets.SourceIpMask(mask);
                                            fIpClassifierSetsSpecified = true;
                                          };

    // accessor for dest IP address.  Type [22/23].9.5  no default value.
    BcmIpAddress DestIpAddr() const { return fIpClassifierSets.DestIpAddr(); };
    void DestIpAddr(BcmIpAddress addr) { fIpClassifierSets.DestIpAddr(addr);
                                          fIpClassifierSetsSpecified = true;
                                        };

    // accessor for dest IP mask.  Type [22/23].9.6  default = 255.255.255.255
    BcmIpAddress DestIpMask() const { return fIpClassifierSets.DestIpMask(); };
    void DestIpMask(BcmIpAddress addr) { fIpClassifierSets.DestIpMask(addr);
                                         fIpClassifierSetsSpecified = true;
                                       };

    // accessor for source TCP/UDP port start.  Type [22/23].9.7  default value = 0
    uint16 SourcePortStart() const { return fIpClassifierSets.SourcePortStart(); };
    void SourcePortStart(uint16 port) { fIpClassifierSets.SourcePortStart(port);
                                        fIpClassifierSetsSpecified = true;
                                      };

    // accessor for source TCP/UDP port end.  Type [22/23].9.8  default value = 65535
    uint16 SourcePortEnd() const { return fIpClassifierSets.SourcePortEnd(); };
    void SourcePortEnd(uint16 port) { fIpClassifierSets.SourcePortEnd(port);
                                      fIpClassifierSetsSpecified = true;
                                    };

    // accessor for dest TCP/UDP port start.  Type [22/23].9.9  default value = 0
    uint16 DestPortStart() const { return fIpClassifierSets.DestPortStart(); };
    void DestPortStart(uint16 port) { fIpClassifierSets.DestPortStart(port);
                                      fIpClassifierSetsSpecified = true;
                                    };

    // accessor for dest TCP/UDP port end.  Type [22/23].9.10  default value = 65535
    uint16 DestPortEnd() const { return fIpClassifierSets.DestPortEnd(); };
    void DestPortEnd(uint16 port) { fIpClassifierSets.DestPortEnd(port);
                                    fIpClassifierSetsSpecified = true;
                                  };

	bool MustHaveSetsSpecified(void) const;

	// CMTS assigned classifier id, 
    // Type [22/23].2  Default = 0.
	uint16 fCmtsClassifierId;
	bool fClassifierIdSpecified;

    // Rule priority.  Type [22/23].5  Context sensitive default(sic).
    byte fRulePriority;
    bool fRulePrioritySpecified;

    // IP classifier settings.  Type [22/23].9  no default.
    BcmIpClassifierSettings fIpClassifierSets;
    bool fIpClassifierSetsSpecified;

};

inline ostream& operator<<( ostream& ostrm, const BcmDsgClassifierSettings& rhs )
{
    return rhs.Print( ostrm );
}

#endif
