//**************************************************************************
//
//    Copyright 1999  Broadcom Corporation
//    All Rights Reserved
//    No portions of this material may be reproduced in any form without the
//    written permission of:
//             Broadcom Corporation
//             16251 Laguna Canyon Road
//             Irvine, California  92618
//    All information contained in this document is Broadcom Corporation
//    company private, proprietary, and trade secret.
//
//
//
//**************************************************************************
//    Filename: IpFilter.h
//    Author:   Kevin O'Neal
//    Creation Date: 27-april-2000
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Header file for IP filter objects
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef IPFILTER_H
#define IPFILTER_H

#include "PktFilter.h"
#include "MibObjs.h"
#include "EthernetPacket.h"
#include "estbDocsDevBridge.h"

#include "BcmSnmpAgent.h"
#include "SnmpLog.h"

#define NOMATCH_IP_IFINDEX    0x00000001
#define NOMATCH_IP_DIRECTION  0x00000002
#define NOMATCH_IP_BROADCAST  0x00000004
#define NOMATCH_IP_SRCIP      0x00000008
#define NOMATCH_IP_DSTIP      0x00000010
#define NOMATCH_IP_PROTOCOL   0x00000020
#define NOMATCH_IP_SRCPORTS   0x00000040
#define NOMATCH_IP_DSTPORTS   0x00000080
#define NOMATCH_IP_TOS        0x00000100
#define NOMATCH_IP_NOT_ACTIVE 0x80000000

class IpFilter
{
  public:
    IpFilter (int Index);
   ~IpFilter ();
   
    static inline int GetDefault ();
    static SNMP_STATUS SetDefault (int Default);
   
    inline bool MatchesAll () const;
    inline PKTFILTER_STATUS Match (const BcmEthernetPacket &Pkt, int IfNum, int Direction);
    inline PKTFILTER_STATUS Apply (BcmEthernetPacket &Pkt) const; // Assumes we've already Matched!! Call Match() fisrt!
    
    ostream &Print(ostream &outStream) const;
    
    int GetIndex () const;
    SNMP_STATUS GetStatus () const;
    int GetControl () const;
    int GetIfIndex () const;
    int GetDirection () const;
    bool GetBroadcast () const;
    const BcmIpAddress &GetSrcIp () const;
    const BcmIpAddress &GetDstIp () const;
    const BcmIpAddress &GetSrcMask() const;
    const BcmIpAddress &GetDstMask () const;
    int GetProtocol () const;
    int GetSrcPortLo () const;
    int GetSrcPortHi () const;
    int GetDstPortLo () const;
    int GetDstPortHi () const;
    unsigned long GetMatches () const;
    unsigned char GetTos () const;
    unsigned char GetTosMask () const;
    bool GetContinue () const;
    int GetPolicyId () const;
    
    SNMP_STATUS SetStatus (SNMP_STATUS Status);
    SNMP_STATUS SetControl (int Control);
    SNMP_STATUS SetIfIndex (int IfIndex);
    SNMP_STATUS SetDirection (int Direction);
    SNMP_STATUS SetBroadcast (bool Bcast);
    SNMP_STATUS SetSrcIp (const BcmIpAddress &Ip);
    SNMP_STATUS SetDstIp (const BcmIpAddress &Ip);
    SNMP_STATUS SetSrcMask (const BcmIpAddress &Mask);
    SNMP_STATUS SetDstMask (const BcmIpAddress &Mask);
    SNMP_STATUS SetProtocol (int Protocol);
    SNMP_STATUS SetSrcPortLo (int Port);
    SNMP_STATUS SetSrcPortHi (int Port);
    SNMP_STATUS SetDstPortLo (int Port);
    SNMP_STATUS SetDstPortHi (int Port);
    SNMP_STATUS SetTos (unsigned char Tos);
    SNMP_STATUS SetTosMask (unsigned char Mask);
    SNMP_STATUS SetContinue (bool Continue);
    SNMP_STATUS SetPolicyId (int Policy);
    
  protected:
    static int fDefault;
    int fIndex;
    SNMP_STATUS fStatus;
    int fControl;
    int fIfIndex;
    int fDirection;
    bool fBroadcast;
    BcmIpAddress fSrcIp, fDstIp, fSrcMask, fDstMask, fSrcIpMasked, fDstIpMasked;
    int fProtocol;
    int fSrcPortLo, fSrcPortHi;
    int fDstPortLo, fDstPortHi;
    unsigned long fMatches;
    unsigned char fTos, fTosMask;
    bool fContinue;
    int fPolicyId;
    bool fMatchesAll;
    
    void UpdateMatchesAll ();
    void LogMatch (const BcmEthernetPacket &Pkt, int IfNum, int Direction, PKTFILTER_STATUS Matched) const;
    
    friend class DocsDevFilterIpTableBridge;
    friend class BcmPacketFilterController;
};

inline bool IpFilter::MatchesAll () const
{
  return fMatchesAll;
}

inline PKTFILTER_STATUS IpFilter::Apply (BcmEthernetPacket &Pkt) const
{
  if (fControl == CONSTVAL_DOCSDEVFILTERIPCONTROL_DISCARD)
    return PKTFILTER_DISCARD;
    
  if (fControl == CONSTVAL_DOCSDEVFILTERIPCONTROL_ACCEPT)
    return PKTFILTER_PASS;
    
  return PKTFILTER_PASS;
}

PKTFILTER_STATUS IpFilter::Match (const BcmEthernetPacket &Pkt, int IfNum, int Direction)
{
  PKTFILTER_STATUS Matched = PKTFILTER_MATCH;
  int PktIf = IfNum; // used below to avoid multi-matching bcast pkts
  
  
#ifdef SUPPORT_CMCI_IF_CONVERSION  
  // Treat IF #1 as all CPE devices
  if (fIfIndex == STATIC_ALL_CPES_IFNUM)
  {
    if ((IfNum > STATIC_CM_US_IFNUM) 
    // PR 7470: eDOCSIS-N-04.0153-2 requires that we only apply filters
    // for ifIndex 1 to interfaces 5-15, not to 16+ which are reserved
    // for eSAFEs.  Also PR 7648, missed this one when doing 7470.
    &&  ((IfNum < 16) || (IfNum > 31)))
      IfNum = 1;
  }
#endif

  // Check that this filter is active
  if (fStatus != SNMP_STATUS_ACTIVE)
    Matched |= NOMATCH_IP_NOT_ACTIVE;
  
  // Check ifIndex.
  else if ((fIfIndex != 0) // IF 0 Matched anything
  &&  (fIfIndex != IfNum)) // Otherwise compare
    Matched |= NOMATCH_IP_IFINDEX;
    
  // Check direction.
  else if ((fDirection != CONSTVAL_DOCSDEVFILTERIPDIRECTION_BOTH)
  && (fDirection != Direction))
    Matched |= NOMATCH_IP_DIRECTION;
    
  // Do we know ahead of time that this filter matches
  // all kinds of IP traffic? If so then we can skip remaining comparisons.
  else if (fMatchesAll == true);
    
  // Check broadcast.  Just check the DA for broadcast.  Note that if fBroadcast
  // is false, we should apply the filter to ALL packets, including broadcast!
  // This is from RFC-2669: "If set to true(1), the filter only applies to multicast
  // and broadcast traffic. If set to false(2), the filter applies to all traffic."
  else if ((fBroadcast == true) && (Pkt.IsUnicast() == true))
    Matched |= NOMATCH_IP_BROADCAST;
    
  // Check SRC IP
  else if (Pkt.SourceIpAddressParsed().IsEqualWithMask(fSrcIpMasked, fSrcMask) == 0)
    Matched |= NOMATCH_IP_SRCIP;
    
  // Check DST IP
  else if (Pkt.DestIpAddressParsed().IsEqualWithMask(fDstIpMasked, fDstMask) == 0)
    Matched |= NOMATCH_IP_DSTIP;
    
  // Check protocol
  else if ((fProtocol != 256) // 256 Matched anything
  && (Pkt.IpProtocolTypeParsed() != fProtocol))
    Matched |= NOMATCH_IP_PROTOCOL;
    
  // Check type of service
  else if ((Pkt.IpTypeOfServiceParsed() & fTosMask) != (fTos & fTosMask))
    Matched |= NOMATCH_IP_TOS;
    
  else if (Pkt.IpProtocolTypeTcpUdpParsed())
  {
    // Check source ports
    if ((Pkt.SourceTcpUdpPortParsed() < fSrcPortLo)
    || (Pkt.SourceTcpUdpPortParsed() > fSrcPortHi))
      Matched |= NOMATCH_IP_SRCPORTS;
    
    // Check dest ports  
    if ((Pkt.DestTcpUdpPortParsed() < fDstPortLo)
    || (Pkt.DestTcpUdpPortParsed() > fDstPortHi))
      Matched |= NOMATCH_IP_DSTPORTS;
  }
    
  // Matched it! Increment counter.
  if (Matched == PKTFILTER_MATCH)
  {
    // Special case: don't match a filter multiple times for a single
    // packet.  This can happen if the packet is multicast & DS, the
    // direction is OUTBOUND, and ifIndex = 1 (ALL_CPES) or 0 (ALL_IFS).  
    // In that case, if we match one interface we will match them all, so only 
    // increment the counter a single time.
    // INCREMENT MATCHES IF ANY ARE TRUE:
    if ((Direction == PKTFILTER_INBOUND)   // direction is inbound
    ||  (PktIf < STATIC_CM_DS_IFNUM)       // interface is primary CPE or CM
#ifdef SUPPORT_CMCI_IF_CONVERSION
    ||  (fIfIndex > STATIC_ALL_CPES_IFNUM) // filter not multi-match (1 or 0)
#else
    ||  (fIfIndex > 0)                     // filter not all interfaces
#endif
    ||  (Pkt.IsUnicast() == true))         // unicast packet
    {
      fMatches++;
//    SnmpLogFilt << "Incrementing match counter: " 
//                << "Direction=" << Direction << ", PktIF=" << PktIf << ", FltIF=" << fIfIndex << endl;
    }
/*
    else
    {
      // DEBUG
      SnmpLogFilt << "NOT Incrementing match counter" << endl;
    }
*/
  }
    
  // If filter logging is on, debug.
  if (BcmSnmpAgent::gMessageLogSettings.GetEnabledSeverities () & BcmSnmpAgent::kPktFilterMessages)
    LogMatch (Pkt, PktIf, Direction, Matched);
  
  return Matched;
}

inline int IpFilter::GetDefault ()
{
  return fDefault;
}

inline ostream & operator << (ostream &outStream, const IpFilter &Filter)
{
    return Filter.Print(outStream);
}

#endif
