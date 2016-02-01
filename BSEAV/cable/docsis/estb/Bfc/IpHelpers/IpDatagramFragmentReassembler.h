//**************************************************************************
//
// Copyright (c) 2002-2010 Broadcom Corporation
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
//  Filename:       IpDatagramFragmentReassembler.h
//  Author:         Sean Dunlap
//  Creation Date:  Sept 26, 2002
//
//****************************************************************************
//  Description:
//      This class implements an IP datagram fragment reassembly algorithm 
//      described in RFC815.  Fragments can arrive in any order.  Duplicate, 
//      and even overlapping fragments are handled.  
//      After 60 seconds, if all fragments are not received, the fragment 
//      buffer expires and resources are freed. 
//
//****************************************************************************
#ifndef IPDATAGRMFRAGMENTREASSEMBLER_H
#define IPDATAGRMFRAGMENTREASSEMBLER_H


//********************** Include Files ***************************************

#include <stdio.h>

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <iostream>
#include <iomanip>
#include <list>
#else
#include <iostream.h>
#include <iomanip.h>
#include <list.h>
#endif
#include "MessageLogSettings.h"
#include "IpAddress.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************
#define  kPACKETMTU 1514   // (including Ethernet Header 1500 + 14 (ethernet header) + 4 Byte Checksum + 4 byte pad = 1522

//********************** Local Variables *************************************

//********************** Forward Declaration *********************************
class BcmReassemblyBuffer;
class BcmEthernetPacket;

//********************** Global instances ************************************

//********************** Class Method Implementations ************************

class BcmIpDatagramFragmentReassembler
{
public:

    // Initializing Constructor.  Stores the information specified.
    //
    // Parameters:
    //      pDescription - text string describing the HalIf, for debugging.
    //
    // Returns:  N/A
    //
    BcmIpDatagramFragmentReassembler(const char *pDescription = NULL,
                                     int maxPacketSize = 65535,
                                     int maxNumberOfFragmentSets = 8,
                                     bool adjustIpStackCounters = false );


    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    ~BcmIpDatagramFragmentReassembler();


    // return code for ProcessIpFragment.  Based on the code, appripriate action must be taken. 
    typedef enum
    {
        NotAllFragmentsReceived,                 // Packet contents were copied.  Packet has been released.
        AllFragmentsReceived,                    // All fragments have been received. 
        IllegalFragment,                         // Malformned fragment (bad header, etc).
        OverlappingFragment,                     // Fragment with offset & length overlaps with a fragment
                                                 // already received (Teardrop attack, etc).
        PayloadTooLarge,                         // Payload is larger than allowed by IP spec.
        MaxNumberOfReassembleyBuffersExceeded    // Too many reassembley buffers active.  DOS attack?
                                                 // Packet has NOT been released.  You should drop the packet.  
                                                
    } tProcessIpFragment_ReturnCodes;

    // The caller does not need to free the announced packet.  
    //
    // return true if the packet has been completely reassembled
    //        false if the last packet has not been found
    // 
    tProcessIpFragment_ReturnCodes ProcessIpFragment( BcmEthernetPacket &ipDatagramFragment, BcmEthernetPacket * reassembledPacket );

    // print the object
    ostream& Print( ostream& ostrm ) const;

    // this is a list of preferred source IP addresses, which will be given preference over non-preferred
    // addresses when it is necessary to discard a fragment set due to fMaxNumberOfFragmentSets being
    // exceeded.  This is a defense against DoS attack fragments which might cause legitimate fragments
    // to otherwise be dropped.  Your application may or may not have the information available
    // to take advantage of this feature. 
    void AddPreferredSourceIPAddress( BcmIpAddress & preferredSourceIpAddress );
    void DeletePreferredSourceIPAddress( BcmIpAddress & preferredSourceIpAddress );
    inline unsigned int ReasmReqds(void);
    inline unsigned int ReasmTimeouts(void);
private:

    // list of preferred source IP addresses
    list< BcmIpAddress > fPreferredSourceIPAddressList;

    // some debug counters
    unsigned int fCompletePacketsReassembled;
    unsigned int fFragmentsProcessed;
    unsigned int fPacketsTimedOut;
    unsigned int fOverlappingFragments;
    unsigned int fPayloadTooLarge;
    unsigned int fIllegalFragment;
    unsigned int fSingleFragmentBuffersKilled;
    unsigned int fFragmentBuffersKilledEarly;

    unsigned int fMaxPacketSize;
    unsigned int fMaxNumberOfFragmentSets;

    bool fAdjustIpStackCounters;

    // list of active reassembly buffers
    list< BcmReassemblyBuffer * > fReassemblyBufferList;

    BcmMessageLogSettings fMessageLogSettings;
};

inline ostream & operator << (ostream &outputStream, const BcmIpDatagramFragmentReassembler &instance)
{
    return instance.Print(outputStream);
}

inline unsigned int BcmIpDatagramFragmentReassembler::ReasmReqds(void) { return (fFragmentsProcessed);}
inline unsigned int BcmIpDatagramFragmentReassembler::ReasmTimeouts(void) { return (fPacketsTimedOut + fFragmentBuffersKilledEarly + fSingleFragmentBuffersKilled);}
#endif
