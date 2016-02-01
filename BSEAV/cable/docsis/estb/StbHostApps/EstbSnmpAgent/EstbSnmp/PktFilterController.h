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
//    Filename: PktFiltr.h
//    Author:   Kevin O'Neal
//    Creation Date: 27-april-2000
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Common definitions for packet filters
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef PKTFILTERCONTROLLER_H
#define PKTFILTERCONTROLLER_H

#include <stdio.h> // for NULL definition

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <iostream>
#else
#include <iostream.h>
#endif

#include "PktFilter.h"
#include "BcmSnmpAgent.h"
#include "SnmpLog.h"

#define MAX_FILTER_INTERFACES   32
// PR 7470: eDOCSIS-N-04.0153-2 requires that we only apply filters
// for ifIndex 1 to interfaces 5-15, not to 16+ which are reserved
// for eSAFEs.
#define ESAFE_RESERVED_IF_START 16
#define ESAFE_RESERVED_IF_END   31
#define PARSE_NONE              0x00
#define PARSE_LLC               0x01
#define PARSE_IP                0x02
#define PARSE_TCPUDP            0x04

// Forward references:
class BcmEthernetPacket;
class IpFilter;
class docsDevFilterIpTable;
// class BcmDocsisMibFilterSnoop;

// fImplMask Bits to indicate which spec interpretations to apply.
#define EMPTY_IP_DEFAULT_IGNORE     0x0002
#define SINGLE_IP_FILTER_SUFFICIENT 0x0004

class BcmPacketFilterController
{
  public:
    typedef enum
    {
      kDisabled = 0,
      kEnabled,
      kOptimized
    } FilterMode;
  
    static inline BcmPacketFilterController &Singleton();
    
    inline PKTFILTER_STATUS ApplyPacketFilters (BcmEthernetPacket &Pkt, unsigned int InFrom, unsigned int OutTo);
    
    int Mode () const;
    int Mode (FilterMode NewMode);
    
    // void MibFilterSnoop(BcmDocsisMibFilterSnoop *pSnoop);

    ostream &Print(ostream &outStream) const;
    
  protected:
    void UpdateIpFilterOptimizations ();
    
    /* Get/Set accessors for different RFC-2669 interpretations */
    bool IgnoreIpDefaultWhenNoFilters () const;
    bool SingleIpFilterSufficient () const;
    void IgnoreIpDefaultWhenNoFilters (bool Ignore);
    void SingleIpFilterSufficient (bool Sufficient);
    
  private:
    BcmPacketFilterController ();
   ~BcmPacketFilterController ();
   
    inline PKTFILTER_STATUS ApplyInboundIpFilters (BcmEthernetPacket &Pkt, unsigned int IfNum, unsigned int OutIfNum) const;
    inline PKTFILTER_STATUS ApplyOutboundIpFilters (BcmEthernetPacket &Pkt, unsigned int IfNum) const;
    inline PKTFILTER_STATUS ApplyIpFilterResults (BcmEthernetPacket &Pkt, unsigned int InFromIx, PKTFILTER_STATUS InIpStatus, unsigned int OutToIx, PKTFILTER_STATUS OutIpStatus) const;
    PKTFILTER_STATUS ApplyPacketFiltersImpl (BcmEthernetPacket &Pkt, unsigned int InFrom, unsigned int OutTo);
    
    void ProcessIpFilter (const IpFilter *pFilter);
    void UpdateParseMasks ();
    
    // Debug printing
    void LogFilt (BcmEthernetPacket &Pkt, unsigned int InFrom, unsigned int OutTo) const;
    void LogIpFilterResults (unsigned int InFromIx, PKTFILTER_STATUS InIpStatus, unsigned int OutToIx, PKTFILTER_STATUS OutIpStatus) const;
    
  private:
    static BcmPacketFilterController *fpThis; // for inline singleton access
    FilterMode fMode;
    unsigned long fImplMask; // for supporting several spec interpretations
    unsigned short fInboundIpFilters[MAX_FILTER_INTERFACES];
    unsigned short fInboundIpTcpUdpFilters[MAX_FILTER_INTERFACES];
    unsigned short fInboundIpMatchesAllFilters[MAX_FILTER_INTERFACES];
    unsigned short fOutboundIpFilters[MAX_FILTER_INTERFACES];
    unsigned short fOutboundIpTcpUdpFilters[MAX_FILTER_INTERFACES];
    unsigned short fOutboundIpMatchesAllFilters[MAX_FILTER_INTERFACES];
    unsigned short fInboundParseMask[MAX_FILTER_INTERFACES];
    unsigned short fOutboundParseMask[MAX_FILTER_INTERFACES];
    docsDevFilterIpTable  *fpIpFilterTable;
  
    // BcmDocsisMibFilterSnoop *fpMibFilterSnoop;

  // Popular guy, this BcmPacketFilterController...  
  friend class docsDevFilterGroup;
  friend class docsDevFilterIpEntry;
  friend class EstbSnmpNonVolSettings;
  friend class BcmSnmpNonVolSettings;
};

inline BcmPacketFilterController &BcmPacketFilterController::Singleton ()
{
  if (fpThis == NULL)
    fpThis = new BcmPacketFilterController ();
    
  return *fpThis;
}

inline PKTFILTER_STATUS BcmPacketFilterController::ApplyPacketFilters (BcmEthernetPacket &Pkt, unsigned int InFrom, unsigned int OutTo)
{
  if (fMode != kDisabled)
  {
    return ApplyPacketFiltersImpl (Pkt, InFrom, OutTo);
  }
  
  SnmpLogFilt << "BcmPacketFilterController::ApplyPacketFilters - all filters disabled, passing EVERYTHING!" << endl;
  return PKTFILTER_NO_OPINION;
}

inline ostream & operator << (ostream &outStream, const BcmPacketFilterController &Filters)
{
    return Filters.Print (outStream);
}

#endif
