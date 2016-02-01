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
//    Filename: IpFiltr.cpp
//    Author:   Kevin O'Neal
//    Creation Date: 27-april-2000
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Code for IP filter objects
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include "IpFilter.h"
#include "MibObjs.h"
#include "SnmpLog.h"
#include "EstbSnmpAgent.h"


// Initialize static objects...
int IpFilter::fDefault = CONSTVAL_DOCSDEVFILTERIPDEFAULT_ACCEPT;


IpFilter::IpFilter (int Index)
{
  fIndex = Index;
  
  // Set up defaults per RFC-2669:
  fStatus = SNMP_STATUS_INVISIBLE;
  fControl = CONSTVAL_DOCSDEVFILTERIPCONTROL_DISCARD;
  fIfIndex = 1;
  fDirection = CONSTVAL_DOCSDEVFILTERIPDIRECTION_INBOUND;
  fBroadcast = false;
  fSrcIp.Set(0,0,0,0);
  fDstIp.Set(0,0,0,0);
  fSrcMask.Set(0,0,0,0);
  fDstMask.Set(0,0,0,0);
  fSrcIpMasked = fSrcIp;
  fSrcIpMasked &= fSrcMask;
  fDstIpMasked = fDstIp;
  fDstIpMasked &= fDstMask;
  fProtocol = 256; // 256 = any
  fSrcPortLo = 0;
  fSrcPortHi = 0xFFFF;
  fDstPortLo = 0;
  fDstPortHi = 0xFFFF;
  fMatches = 0;
  fTos = 0;
  fTosMask = 0;
  fContinue = false;
  fPolicyId = 0;
  fMatchesAll = false;
}

IpFilter::~IpFilter ()
{
}

ostream &IpFilter::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  outStream << "IP filter " << fIndex << endl;
#endif
  return outStream;
}

SNMP_STATUS IpFilter::SetDefault (int Default)
{
  if ((Default != CONSTVAL_DOCSDEVFILTERIPDEFAULT_ACCEPT)
  &&  (Default != CONSTVAL_DOCSDEVFILTERIPDEFAULT_DISCARD))
    return SNMP_WRONG_VALUE;

  fDefault = Default;
  return SNMP_NO_ERROR;
}

int IpFilter::GetIndex () const
{
  return fIndex;
}

SNMP_STATUS IpFilter::GetStatus () const
{
  return fStatus;
}

int IpFilter::GetControl () const
{
  return fControl;
}

int IpFilter::GetIfIndex () const
{
  return fIfIndex;
}

int IpFilter::GetDirection () const
{
  return fDirection;
}

bool IpFilter::GetBroadcast () const
{
  return fBroadcast;
}

const BcmIpAddress &IpFilter::GetSrcIp () const
{
  return fSrcIp;
}

const BcmIpAddress &IpFilter::GetDstIp () const
{
  return fDstIp;
}

const BcmIpAddress &IpFilter::GetSrcMask() const
{
  return fSrcMask;
}

const BcmIpAddress &IpFilter::GetDstMask () const
{
  return fDstMask;
}

int IpFilter::GetProtocol () const
{
  return fProtocol;
}

int IpFilter::GetSrcPortLo () const
{
  return fSrcPortLo;
}

int IpFilter::GetSrcPortHi () const
{
  return fSrcPortHi;
}

int IpFilter::GetDstPortLo () const
{
  return fDstPortLo;
}

int IpFilter::GetDstPortHi () const
{
  return fDstPortHi;
}

unsigned long IpFilter::GetMatches () const
{
  return fMatches;
}

unsigned char IpFilter::GetTos () const
{
  return fTos;
}

unsigned char IpFilter::GetTosMask () const
{
  return fTosMask;
}

bool IpFilter::GetContinue () const
{
  return fContinue;
}

int IpFilter::GetPolicyId () const
{
  return fPolicyId;
}

SNMP_STATUS IpFilter::SetStatus (SNMP_STATUS Status)
{
  fStatus = Status;
  UpdateMatchesAll ();
  return SNMP_NO_ERROR;
}

SNMP_STATUS IpFilter::SetControl (int Control)
{
  fControl = Control;
  UpdateMatchesAll ();
  return SNMP_NO_ERROR;
}

SNMP_STATUS IpFilter::SetIfIndex (int IfIndex)
{
  fIfIndex = IfIndex;
  UpdateMatchesAll ();
  return SNMP_NO_ERROR;
}

SNMP_STATUS IpFilter::SetDirection (int Direction)
{
  if (Direction != 1)
    return SNMP_BAD_VALUE;
  fDirection = Direction;
  UpdateMatchesAll ();
  return SNMP_NO_ERROR;
}

SNMP_STATUS IpFilter::SetBroadcast (bool Bcast)
{
  fBroadcast = Bcast;
  UpdateMatchesAll ();
  return SNMP_NO_ERROR;
}

SNMP_STATUS IpFilter::SetSrcIp (const BcmIpAddress &Ip)
{
  fSrcIp = Ip;
  
  fSrcIpMasked = fSrcIp;
  fSrcIpMasked &= fSrcMask;
  
  UpdateMatchesAll ();
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS IpFilter::SetDstIp (const BcmIpAddress &Ip)
{
  fDstIp = Ip;
  
  fDstIpMasked = fDstIp;
  fDstIpMasked &= fDstMask;
  
  UpdateMatchesAll ();
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS IpFilter::SetSrcMask (const BcmIpAddress &Mask)
{
  fSrcMask = Mask;
  
  fSrcIpMasked = fSrcIp;
  fSrcIpMasked &= fSrcMask;
  
  UpdateMatchesAll ();
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS IpFilter::SetDstMask (const BcmIpAddress &Mask)
{
  fDstMask = Mask;
  
  fDstIpMasked = fDstIp;
  fDstIpMasked &= fDstMask;
  
  UpdateMatchesAll ();
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS IpFilter::SetProtocol (int Protocol)
{
  fProtocol = Protocol;
  UpdateMatchesAll ();
  return SNMP_NO_ERROR;
}

SNMP_STATUS IpFilter::SetSrcPortLo (int Port)
{
  fSrcPortLo = Port;
  UpdateMatchesAll ();
  return SNMP_NO_ERROR;
}

SNMP_STATUS IpFilter::SetSrcPortHi (int Port)
{
  fSrcPortHi = Port;
  UpdateMatchesAll ();
  return SNMP_NO_ERROR;
}

SNMP_STATUS IpFilter::SetDstPortLo (int Port)
{
  fDstPortLo = Port;
  UpdateMatchesAll ();
  return SNMP_NO_ERROR;
}

SNMP_STATUS IpFilter::SetDstPortHi (int Port)
{
  fDstPortHi = Port;
  UpdateMatchesAll ();
  return SNMP_NO_ERROR;
}

SNMP_STATUS IpFilter::SetTos (unsigned char Tos)
{
  fTos = Tos;
  UpdateMatchesAll ();
  return SNMP_NO_ERROR;
}

SNMP_STATUS IpFilter::SetTosMask (unsigned char Mask)
{
  fTosMask = Mask;
  UpdateMatchesAll ();
  return SNMP_NO_ERROR;
}

SNMP_STATUS IpFilter::SetContinue (bool Continue)
{
  fContinue = Continue;
  UpdateMatchesAll ();
  return SNMP_NO_ERROR;
}

SNMP_STATUS IpFilter::SetPolicyId (int Policy)
{
  return SNMP_BAD_VALUE;
}

// This method will check all filter constraints and see if they match
// everything.  This will allow us to avoid checking all packet fields for
// a given filter if we know ahead of time that it will match ALL packets.
// Note that we don't consider status, ifIndex, or direction when making
// this decision.  This is because if we do, then our MatchesAll optimization
// will only help if the filter is for both directions and all interfaces.
void IpFilter::UpdateMatchesAll ()
{
  bool MatchesAll = true;
  
  
  if (GetBroadcast() == true)
    MatchesAll = false;
    
  else if (GetSrcMask() != kAllZerosIp)
    MatchesAll = false;
    
  else if (GetDstMask() != kAllZerosIp)
    MatchesAll = false;
    
  else if (GetProtocol() != 256)
    MatchesAll = false;
    
  else if (GetSrcPortLo() != 0)
    MatchesAll = false;
    
  else if (GetSrcPortHi() != 0xFFFF)
    MatchesAll = false;
    
  else if (GetDstPortLo() != 0)
    MatchesAll = false;
    
  else if (GetDstPortHi() != 0xFFFF)
    MatchesAll = false;
    
  else if (GetTos() != 0)
    MatchesAll = false;
    
  else if (GetTosMask() != 0)
    MatchesAll = false;
  
  fMatchesAll = MatchesAll;
}

void IpFilter::LogMatch (const BcmEthernetPacket &Pkt, int IfNum, int Direction, PKTFILTER_STATUS Matched) const
{
#ifdef FILTER_LOGGING_SUPPORT
  bool Mismatch = false;

  if (Direction == PKTFILTER_INBOUND)
    SnmpLogFilt << "Checking IP filter " << fIndex << " for INBOUND match." << endl;
  else
    SnmpLogFilt << "Checking IP filter " << fIndex << " for OUTBOUND match." << endl;
    
  if (Matched & NOMATCH_IP_NOT_ACTIVE)
  {
    SnmpLogFilt << "  Status is not ACTIVE (" << fStatus << "): NO MATCH\n";
    Mismatch = true;
  }
  
  // ifIndex
  SnmpLogFilt << "  ifIndex: P=" << IfNum << ", F=" << fIfIndex << ": ";
  if (Matched & NOMATCH_IP_IFINDEX)
  {
    SnmpLogFilt << "NO MATCH" << endl;
    Mismatch = true;
  }
  else if (Mismatch)
    SnmpLogFilt << "(not checked)" << endl;
  else
    SnmpLogFilt << "MATCH" << endl;
    
  // direction
  SnmpLogFilt << "  direction: P=" << Direction << ", F=" << fDirection << ": ";
  if (Matched & NOMATCH_IP_DIRECTION)
  {
    SnmpLogFilt << "NO MATCH" << endl;
    Mismatch = true;
  }
  else if (Mismatch)
    SnmpLogFilt << "(not checked)" << endl;
  else
    SnmpLogFilt << "MATCH" << endl;
    
  SnmpLogFilt << "  matches all: " << fMatchesAll << endl;
  
  // broadcast
  SnmpLogFilt << "  broadcast: P=" << Pkt.MacDa() << " (" << Pkt.MacDa().IsMulticast() << "), F=" << fBroadcast << ": ";
  if (Matched & NOMATCH_IP_BROADCAST)
  {
    SnmpLogFilt << "NO MATCH" << endl;
    Mismatch = true;
  }
  else if (Mismatch)
    SnmpLogFilt << "(not checked)" << endl;
  else
    SnmpLogFilt << "MATCH" << endl;
  
  // src IP
  SnmpLogFilt << "  src IP: P=" << Pkt.SourceIpAddressParsed() << "; F=" << fSrcIp << ", M=" << fSrcMask << ": ";
  if (Matched & NOMATCH_IP_SRCIP)
  {
    SnmpLogFilt << "NO MATCH" << endl;
    Mismatch = true;
  }
  else if (Mismatch)
    SnmpLogFilt << "(not checked)" << endl;
  else
    SnmpLogFilt << "MATCH" << endl;
  
  // dst IP
  SnmpLogFilt << "  dst IP: P=" << Pkt.DestIpAddressParsed() << "; F=" << fDstIp << ", M=" << fDstMask << ": ";
  if (Matched & NOMATCH_IP_DSTIP)
  {
    SnmpLogFilt << "NO MATCH" << endl;
    Mismatch = true;
  }
  else if (Mismatch)
    SnmpLogFilt << "(not checked)" << endl;
  else
    SnmpLogFilt << "MATCH" << endl;
  
  // protocol
  SnmpLogFilt << "  protocol: P=" << (int)Pkt.IpProtocolTypeParsed() << ", F=" << fProtocol << ": ";
  if (Matched & NOMATCH_IP_PROTOCOL)
  {
    SnmpLogFilt << "NO MATCH" << endl;
    Mismatch = true;
  }
  else if (Mismatch)
    SnmpLogFilt << "(not checked)" << endl;
  else
    SnmpLogFilt << "MATCH" << endl;
    
  // tos
  SnmpLogFilt << "  tos: P=" << inHex(Pkt.IpTypeOfServiceParsed()) << "; F=" << inHex(fTos) << ", M=" << inHex(fTosMask) << ": ";
  if (Matched & NOMATCH_IP_TOS)
  {
    SnmpLogFilt << "NO MATCH" << endl;
    Mismatch = true;
  }
  else if (Mismatch)
    SnmpLogFilt << "(not checked)" << endl;
  else
    SnmpLogFilt << "MATCH" << endl;
  
  // src ports
  if (Pkt.IpProtocolTypeTcpUdpParsed())
    SnmpLogFilt << "  src ports: P=" << Pkt.SourceTcpUdpPortParsed() << ", F=" << fSrcPortLo << "<->" << fSrcPortHi << ": ";
  else
    SnmpLogFilt << "  src ports: packet is not TCP or UDP, ignored: ";
  if (Matched & NOMATCH_IP_SRCPORTS)
  {
    SnmpLogFilt << "NO MATCH" << endl;
    Mismatch = true;
  }
  else if (Mismatch)
    SnmpLogFilt << "(not checked)" << endl;
  else
    SnmpLogFilt << "MATCH" << endl;
  
  // dst ports
  if (Pkt.IpProtocolTypeTcpUdpParsed())
    SnmpLogFilt << "  dst ports: P=" << Pkt.SourceTcpUdpPortParsed() << ", F=" << fDstPortLo << "<->" << fDstPortHi << ": ";
  else
    SnmpLogFilt << "  dst ports: packet is not TCP or UDP, ignored: ";
  if (Matched & NOMATCH_IP_DSTPORTS)
  {
    SnmpLogFilt << "NO MATCH" << endl;
    Mismatch = true;
  }
  else if (Mismatch)
    SnmpLogFilt << "(not checked)" << endl;
  else
    SnmpLogFilt << "MATCH" << endl;
  
  if (Matched == PKTFILTER_MATCH)
  {
    
    SnmpLogFilt << "  +++ MATCH +++ " ;
    
    if (fControl == PKTFILTER_PASS)
      SnmpLogFilt << "control=ACCEPT ";
    else if (fControl == PKTFILTER_DISCARD)
      SnmpLogFilt << "control=DISCARD ";
    else
      SnmpLogFilt << "control=POLICY" << fPolicyId << " ";
    
    if (fContinue == true)
      SnmpLogFilt << "continue=TRUE" << endl;
    else
      SnmpLogFilt << "continue=FALSE" << endl;
  }
  else
    SnmpLogFilt << "  -- NO MATCH --" << endl;
#endif
}

