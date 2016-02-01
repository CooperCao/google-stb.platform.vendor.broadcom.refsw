//**************************************************************************
//
//    Copyright 2001 Broadcom Corporation
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
//    Filename: PktFiltr.cpp
//    Author:   Kevin O'Neal
//    Creation Date: 21-august-2001
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

#include "PktFilterController.h"
#include "IpFilter.h"
#include "NmAccess.h"
#include "EthernetPacket.h"
#include "EstbSnmpAgent.h"
#include "ifBridge.h"
#include "SnmpNonVolSettings.h"


BcmPacketFilterController *BcmPacketFilterController::fpThis = NULL;

BcmPacketFilterController::BcmPacketFilterController ()
{
  int Loop;

  for (Loop = 0; Loop < MAX_FILTER_INTERFACES; Loop++)
  {
    fInboundParseMask[Loop] = PARSE_NONE;
    fOutboundParseMask[Loop] = PARSE_NONE;
    fInboundIpFilters[Loop] = 0;
    fInboundIpTcpUdpFilters[Loop] = 0;
    fInboundIpMatchesAllFilters[Loop] = 0;
    fOutboundIpFilters[Loop] = 0;
    fOutboundIpTcpUdpFilters[Loop] = 0;
    fOutboundIpMatchesAllFilters[Loop] = 0;
  }
  
  
  fpIpFilterTable = NULL;
  
#ifdef NOTYET
  fpMibFilterSnoop = NULL;
#endif

  Mode (kOptimized);
  
  // Get implementation flags from non-vol, if available.
  BcmSnmpNonVolSettings *pSettings = BcmSnmpNonVolSettings::GetSingletonInstance();
  if (pSettings)
  {
    fImplMask = 0;
    
    if (pSettings->PktfilterEmptyIpDefaultIgnore() == true)
      fImplMask |= EMPTY_IP_DEFAULT_IGNORE;
    
    if (pSettings->PktfilterSingleIpFilterSufficient() == true)
      fImplMask |= SINGLE_IP_FILTER_SUFFICIENT;
  }
  else
  {
    fImplMask = EMPTY_IP_DEFAULT_IGNORE; 

    fImplMask |= SINGLE_IP_FILTER_SUFFICIENT;
  }
}

BcmPacketFilterController::~BcmPacketFilterController ()
{
}

int BcmPacketFilterController::Mode () const
{
  return fMode;
}

int BcmPacketFilterController::Mode (FilterMode NewMode)
{
  if (NewMode == kOptimized)
  {
    UpdateIpFilterOptimizations ();
  }
  else if (NewMode == kEnabled)
  {
    // Make sure we have all tables if we are going in to kEnabled mode.
    if (fpIpFilterTable == NULL)
      fpIpFilterTable = (docsDevFilterIpTable *) EstbSnmpAgent::Singleton().FindTable (BcmString ("docsDevFilterIpTable"));
    // If we don't find any of these, don't go into enabled mode lest we
    // dereference NULL...
    if (fpIpFilterTable == NULL)
    {
      SnmpLogFilt << "BcmPacketFilterController::Mode can't go kEnabled, missing tables." << endl;
      return Mode();
    }
  }
  
  fMode = NewMode;
    
  return Mode();
}

#ifdef NOTYET
void BcmPacketFilterController::MibFilterSnoop(BcmDocsisMibFilterSnoop *pSnoop)
{
    fpMibFilterSnoop = pSnoop;

    // Call this so that we will enable/disable the Snoop as appropriate.
    UpdateParseMasks();
}
#endif


/* Get/Set accessors for CableLabs required deviations from RFC-2669 */

bool BcmPacketFilterController::IgnoreIpDefaultWhenNoFilters () const
{
  return (fImplMask & EMPTY_IP_DEFAULT_IGNORE) != 0;
}

bool BcmPacketFilterController::SingleIpFilterSufficient () const
{
  return (fImplMask & SINGLE_IP_FILTER_SUFFICIENT) != 0;
}

void BcmPacketFilterController::IgnoreIpDefaultWhenNoFilters (bool Ignore)
{
  if (Ignore)
    fImplMask |= EMPTY_IP_DEFAULT_IGNORE;
  else
    fImplMask &= ~EMPTY_IP_DEFAULT_IGNORE;
}

void BcmPacketFilterController::SingleIpFilterSufficient (bool Sufficient)
{
  if (Sufficient)
    fImplMask |= SINGLE_IP_FILTER_SUFFICIENT;
  else
    fImplMask &= ~SINGLE_IP_FILTER_SUFFICIENT;
}

ostream &BcmPacketFilterController::Print(ostream &outStream) const
{
#ifdef FILTER_LOGGING_SUPPORT
  BcmString String;
  const char *pIpDefaultImpl;
  int NumIps;
  
  
  outStream << "======================== RFC-2669 Packet Filtering ============================\n"
            << "Controller: " << (void *)this << "\n"
            << "Table: IP=" << (void *)fpIpFilterTable << "\n"
            << "===============================================================================\n" << endl;
            
  outStream << "Implementation\n"
               "--------------" << endl;
  
  if (fImplMask & EMPTY_IP_DEFAULT_IGNORE)
    pIpDefaultImpl = "IGNORED (accept all)";
  else
    pIpDefaultImpl = "APPLIED";
    
  if (fImplMask & SINGLE_IP_FILTER_SUFFICIENT)
    NumIps = 1;
  else
    NumIps = 2;
  
  outStream << "Filter Mode (0=disabled, 1=enabled, 2=optimized):  " << (int)fMode << endl;
  outStream << "docsDevFilterIpDefault if no filters:              " << pIpDefaultImpl << endl;
  outStream << "IP filters required for default not to be applied: " << NumIps << endl;
  outStream << "" << endl;
               
  outStream << "Control Settings\n"
               "----------------" << endl;
  
  if (IpFilter::GetDefault() == CONSTVAL_DOCSDEVFILTERIPDEFAULT_ACCEPT)
    outStream << "IP default:  ACCEPT" << endl;
  else if (IpFilter::GetDefault() == CONSTVAL_DOCSDEVFILTERIPDEFAULT_DISCARD)
    outStream << "IP default:  DISCARD" << endl;
  else
    outStream << "IP default:  POLICY" << endl;
  
  outStream << "\nGlobal Filters\n"
                 "--------------\n"
                 "Per-Interface Filters\n"
                 "---------------------" << endl;
            
  // Iterate through all ifTable entries and indicate filters per interface:
  ifEntryBridge *pIf = EstbSnmpAgent::Singleton().IfBridge().FirstIfEntry();
  while (pIf)
  {
      pIf->Get_ifDescr(String);
      
      outStream << pIf->Get_ifIndex() << ": " << String << endl;
      outStream << "   IP In:   " << (int)fInboundIpFilters[pIf->Get_ifIndex()-1] << 
                   " (TCP/UDP:" << (int)fInboundIpTcpUdpFilters[pIf->Get_ifIndex()-1] << ", MatchesAll:" << (int)fInboundIpMatchesAllFilters[pIf->Get_ifIndex()-1] << ")\n"
                   "   IP Out:  " << (int)fOutboundIpFilters[pIf->Get_ifIndex()-1] <<
                   " (TCP/UDP:" << (int)fOutboundIpTcpUdpFilters[pIf->Get_ifIndex()-1] << ", MatchesAll:" << (int)fOutboundIpMatchesAllFilters[pIf->Get_ifIndex()-1] << ")\n"
                   "   Parse Mask (0x02=IP, 0x04=TCP/UDP)  IN:" << inHex(fInboundParseMask[pIf->Get_ifIndex()-1]) << " OUT:" << inHex(fOutboundParseMask[pIf->Get_ifIndex()-1])
                   << endl;
                   
      if ((fInboundIpFilters[pIf->Get_ifIndex()-1])
      ||  (fOutboundIpFilters[pIf->Get_ifIndex()-1])
      ||  ((fImplMask & EMPTY_IP_DEFAULT_IGNORE) == 0))
        outStream << "   IP default for this IF will be APPLIED to non-matching pkts" << endl;
      else
        outStream << "   IP default for this IF will be IGNORED (accept all)" << endl;
        
    pIf = EstbSnmpAgent::Singleton().IfBridge().NextIfEntry(pIf->Get_ifIndex());
  }
#endif
           
  return outStream;
}

void BcmPacketFilterController::UpdateIpFilterOptimizations ()
{
  FilterMode OldMode = fMode;
  

  SnmpLogFilt << "BcmPacketFilterController::UpdateIpFilterOptimizations" << endl;
  
  // Turn off optimizations until we are done.
  Mode (kEnabled);
  
  // Initialize to no filters
  for (int Loop = 0; Loop < MAX_FILTER_INTERFACES; Loop++)
  {
    fInboundIpFilters[Loop] = 0;
    fInboundIpTcpUdpFilters[Loop] = 0;
    fInboundIpMatchesAllFilters[Loop] = 0;
    fOutboundIpFilters[Loop] = 0;
    fOutboundIpTcpUdpFilters[Loop] = 0;
    fOutboundIpMatchesAllFilters[Loop] = 0;
  }
  
  // Iterate through IP filters and policy entries and process each one.
  MibEntryIterator Pos;
  EstbSnmpAgent::Singleton().LockMib();
  
  if (fpIpFilterTable != NULL)
  {
    docsDevFilterIpEntry *pEntry = (docsDevFilterIpEntry *)fpIpFilterTable->FirstEntry (Pos);
    while (pEntry)
    {
      ProcessIpFilter (pEntry->GetFilter());
      pEntry = (docsDevFilterIpEntry *)fpIpFilterTable->NextEntry (Pos);
    }
  }
  
  EstbSnmpAgent::Singleton().UnlockMib();
  
  // Update parse masks
  UpdateParseMasks ();
  
  // Turn optimizations back on.
  fMode = OldMode;
}

void BcmPacketFilterController::UpdateParseMasks ()
{
  int Loop;
  bool needToCallMe = false;
  
  SnmpLogFilt << "BcmPacketFilterController::UpdateParseMasks" << endl;
  
  for (Loop = 0; Loop < MAX_FILTER_INTERFACES; Loop++)
  {
    fInboundParseMask[Loop] = 0;
    fOutboundParseMask[Loop] = 0;
    
    if (fInboundIpFilters[Loop] - fInboundIpMatchesAllFilters[Loop] > 0)
    {
      fInboundParseMask[Loop] |= (PARSE_LLC | PARSE_IP);
      
      if (fInboundIpTcpUdpFilters[Loop] != 0)
        fInboundParseMask[Loop] |= PARSE_TCPUDP;
    }
    // If we have any IP filters we at least need to parse LLC so that
    // we can tell if a packet is an IP packet or not.
    else if (fInboundIpFilters[Loop] != 0)
    {
        fInboundParseMask[Loop] |= PARSE_LLC;
    }
    
    if (fOutboundIpFilters[Loop] - fOutboundIpMatchesAllFilters[Loop] > 0)
    {
      fOutboundParseMask[Loop] |= (PARSE_LLC | PARSE_IP);
      
      if (fOutboundIpTcpUdpFilters[Loop] != 0)
        fOutboundParseMask[Loop] |= PARSE_TCPUDP;
    }
    // If we have any IP filters we at least need to parse LLC so that
    // we can tell if a packet is an IP packet or not.
    else if (fOutboundIpFilters[Loop] != 0)
    {
        fOutboundParseMask[Loop] |= PARSE_LLC;
    }
    
    if ((fInboundParseMask[Loop] != 0) ||
        (fOutboundParseMask[Loop] != 0))
    {
        needToCallMe = true;
    }
  }

#ifdef NOTYET
  // If there is no interesting work to do (which is often the case), then
  // disable the Snoop so that it doesn't call me.  Otherwise, if I have to
  // check something or other, then enable the Snoop so that it does call me.
  if (fpMibFilterSnoop != NULL)
  {
      // If the opinion so far is that we don't need to call me, then check
      // one last thing - the default has to be accept in order for the Snoop
      // to be disabled.  
      if (needToCallMe == false)
      {
          if (IpFilter::GetDefault() != CONSTVAL_DOCSDEVFILTERIPDEFAULT_ACCEPT)
          {
              needToCallMe = true;
          }
      }

      //  Now update the Snoop.
      fpMibFilterSnoop->Enable(needToCallMe);
  }
#endif
}

void BcmPacketFilterController::ProcessIpFilter (const IpFilter *pFilter)
{
  bool TcpUdp = true;

  
  if (pFilter->GetIfIndex() > MAX_FILTER_INTERFACES)
    return;
    
  if (pFilter->GetStatus() != SNMP_STATUS_ACTIVE)
    return;
    
  if ((pFilter->GetSrcPortLo() == 0)
  &&  (pFilter->GetSrcPortHi() == 65535)
  &&  (pFilter->GetDstPortLo() == 0)
  &&  (pFilter->GetDstPortHi() == 65535))
    TcpUdp = false;
    
  // Special case #1: 0 applies to all interfaces
  if (pFilter->GetIfIndex() == 0)
  {
    for (int Loop = 0; Loop < MAX_FILTER_INTERFACES; Loop++)
    {
      if ((pFilter->GetDirection() == PKTFILTER_INBOUND)
      ||  (pFilter->GetDirection() == PKTFILTER_BOTH))
      {
        fInboundIpFilters[Loop]++;
        if (TcpUdp)
          fInboundIpTcpUdpFilters[Loop]++;
          
        if (pFilter->MatchesAll() == true)
          fInboundIpMatchesAllFilters[Loop]++;
      }
        
      if ((pFilter->GetDirection() == PKTFILTER_OUTBOUND)
      ||  (pFilter->GetDirection() == PKTFILTER_BOTH))
      {
        fOutboundIpFilters[Loop]++;
        if (TcpUdp)
          fOutboundIpTcpUdpFilters[Loop]++;
          
        if (pFilter->MatchesAll() == true)
          fOutboundIpMatchesAllFilters[Loop]++;
      }
    }
  }
  
#ifdef SUPPORT_CMCI_IF_CONVERSION
  // Special case #2: 1 applies to all CPE interfaces (anything but 2,3,4)
  else if (pFilter->GetIfIndex() == STATIC_ALL_CPES_IFNUM)
  {
    if ((pFilter->GetDirection() == PKTFILTER_INBOUND)
    ||  (pFilter->GetDirection() == PKTFILTER_BOTH))
    {
      fInboundIpFilters[0]++;
      if (TcpUdp)
        fInboundIpTcpUdpFilters[0]++;
        
      if (pFilter->MatchesAll() == true)
        fInboundIpMatchesAllFilters[0]++;
    }
      
    if ((pFilter->GetDirection() == PKTFILTER_OUTBOUND)
    ||  (pFilter->GetDirection() == PKTFILTER_BOTH))
    {
      fOutboundIpFilters[0]++;
      if (TcpUdp)
        fOutboundIpTcpUdpFilters[0]++;
        
      if (pFilter->MatchesAll() == true)
        fOutboundIpMatchesAllFilters[0]++;
    }
    
    for (int Loop = 4; Loop < MAX_FILTER_INTERFACES; Loop++)
    {
      // PR 7470: eDOCSIS-N-04.0153-2 requires that we only apply filters
      // for ifIndex 1 to interfaces 5-15 and 32+, not to 16-31 which are 
      // reserved for eSAFEs.
      if ((Loop >= (ESAFE_RESERVED_IF_START-1))
      &&  (Loop <   ESAFE_RESERVED_IF_END))
        continue;
      
      if ((pFilter->GetDirection() == PKTFILTER_INBOUND)
      ||  (pFilter->GetDirection() == PKTFILTER_BOTH))
      {
        fInboundIpFilters[Loop]++;
        if (TcpUdp)
          fInboundIpTcpUdpFilters[Loop]++;
          
        if (pFilter->MatchesAll() == true)
          fInboundIpMatchesAllFilters[Loop]++;
      }
        
      if ((pFilter->GetDirection() == PKTFILTER_OUTBOUND)
      ||  (pFilter->GetDirection() == PKTFILTER_BOTH))
      {
        fOutboundIpFilters[Loop]++;
        if (TcpUdp)
          fOutboundIpTcpUdpFilters[Loop]++;
          
        if (pFilter->MatchesAll() == true)
          fOutboundIpMatchesAllFilters[Loop]++;
      }
    }
  }
#endif
  
  else 
  {
    if ((pFilter->GetDirection() == PKTFILTER_INBOUND)
    ||  (pFilter->GetDirection() == PKTFILTER_BOTH))
    {
      fInboundIpFilters[pFilter->GetIfIndex()-1]++;
      if (TcpUdp)
        fInboundIpTcpUdpFilters[pFilter->GetIfIndex()-1]++;
        
      if (pFilter->MatchesAll() == true)
        fInboundIpMatchesAllFilters[pFilter->GetIfIndex()-1]++;
    }
      
    if ((pFilter->GetDirection() == PKTFILTER_OUTBOUND)
    ||  (pFilter->GetDirection() == PKTFILTER_BOTH))
    {
      fOutboundIpFilters[pFilter->GetIfIndex()-1]++;
      if (TcpUdp)
        fOutboundIpTcpUdpFilters[pFilter->GetIfIndex()-1]++;
        
      if (pFilter->MatchesAll() == true)
        fOutboundIpMatchesAllFilters[pFilter->GetIfIndex()-1]++;
    }
  }
}

inline PKTFILTER_STATUS BcmPacketFilterController::ApplyInboundIpFilters (BcmEthernetPacket &Pkt, 
                         unsigned int IfNum, unsigned int OutIfNum) const
{
  if ((fMode == kOptimized) && (fInboundIpFilters[IfNum-1] == 0))
    return PKTFILTER_NO_OPINION;
      
  MibEntryIterator Pos;
  PKTFILTER_STATUS Action = PKTFILTER_NO_OPINION;
  
  docsDevFilterIpEntry *pEntry = (docsDevFilterIpEntry *)fpIpFilterTable->FirstEntry (Pos);
  while (pEntry)
  {
    if (pEntry->GetFilter()->Match (Pkt, IfNum, PKTFILTER_INBOUND) == PKTFILTER_MATCH)
    {
      Action = pEntry->GetFilter()->Apply (Pkt);
      
      // PR 7648: Special case here.  If this filter is inbound for a CPE
      // interface and we just matched where the OutIfNum is an eSAFE (if 16-31)
      // and the packet is multicast we must decrement the matches counter on 
      // the filter.  This is because we now are required to filter among
      // CPE devices if one of the CPEs is an eSAFE.  This breaks our logic
      // in IpFiltr.h about how to avoid multiple-matching on inbound
      // filters when we have a multicast packet.
      if ((OutIfNum > 15)             // Out an eSAFE
      &&  (OutIfNum < 32)             // Out an eSAFE
      &&  (IfNum != STATIC_CM_IFNUM)  // In a CPE
      &&  (Pkt.IsUnicast() == false)) // Broadcast or multicast pkt
      {
        //SnmpLogRaw << "M--" << endl;
        pEntry->GetFilter()->fMatches--;
      }
      
      // If the filter says DISCARD, then drop the packet and we're done, even
      // if Continue is TRUE.
      if (Action & PKTFILTER_DISCARD)
        return Action;
      
      // If it's pass or policy and don't continue, we're also done.
      if (pEntry->GetFilter()->GetContinue() == false)
        return Action;
    }
      
    pEntry = (docsDevFilterIpEntry *)fpIpFilterTable->NextEntry (Pos);
  }
  
  return Action;
}

inline PKTFILTER_STATUS BcmPacketFilterController::ApplyOutboundIpFilters (BcmEthernetPacket &Pkt, unsigned int IfNum) const
{
  if ((fMode == kOptimized) && (fOutboundIpFilters[IfNum-1] == 0))
    return PKTFILTER_NO_OPINION;
    
  MibEntryIterator Pos;
  PKTFILTER_STATUS Action = PKTFILTER_NO_OPINION;
  
  docsDevFilterIpEntry *pEntry = (docsDevFilterIpEntry *)fpIpFilterTable->FirstEntry (Pos);
  while (pEntry)
  {
    if (pEntry->GetFilter()->Match (Pkt, IfNum, PKTFILTER_OUTBOUND) == PKTFILTER_MATCH)
    {
      Action = pEntry->GetFilter()->Apply (Pkt);
    
      // If the filter says DISCARD, then drop the packet and we're done, even
      // if Continue is TRUE.
      if (Action & PKTFILTER_DISCARD)
        return Action;
      
      // If it's pass and don't continue, we're also done.
      if (pEntry->GetFilter()->GetContinue() == false)
        return PKTFILTER_PASS;
    }
      
    pEntry = (docsDevFilterIpEntry *)fpIpFilterTable->NextEntry (Pos);
  }
  
  return Action;
}


/*\ 
 * Consider the results of the IP filter processing, subject to 
 * our spec interpretation flags.  In any case, the IP filter methods
 * can return 1 of 3 values:
 * 
 * NO OPINION: didn't match any filters
 * PASS: matched a filter, told to accept the packet
 * DISCARD: matched a filter, told to discard the packet
\*/
inline PKTFILTER_STATUS BcmPacketFilterController::ApplyIpFilterResults (BcmEthernetPacket &Pkt, 
                  unsigned int InFromIx, PKTFILTER_STATUS InIpStatus, 
                  unsigned int OutToIx, PKTFILTER_STATUS OutIpStatus) const
{
#ifdef FILTER_LOGGING_SUPPORT
  if (BcmSnmpAgent::gMessageLogSettings.GetEnabledSeverities () & BcmSnmpAgent::kPktFilterMessages)
      LogIpFilterResults (InFromIx, InIpStatus, OutToIx, OutIpStatus);
#endif

  // Mask off continue bits, we don't care about them at this point and they
  // may confuse things below.
  InIpStatus  &= 0x0003;
  OutIpStatus &= 0x0003;

  /* One possibility: didn't match a filter inbound or outbound. */
  
  if ((InIpStatus == PKTFILTER_NO_OPINION) && (OutIpStatus == PKTFILTER_NO_OPINION))
  {
    // If we are using the ignore default when the table is empty
    // impl flag, return NO OPINION if we have no filters at all.
    if ((fImplMask & EMPTY_IP_DEFAULT_IGNORE)
    &&  (fInboundIpFilters[InFromIx-1] == 0)
    &&  (fOutboundIpFilters[OutToIx-1] == 0))
      return PKTFILTER_NO_OPINION;
      
    // ...and return the default.
    return IpFilter::GetDefault();
  }
  
  
  /* Second possibility: matched in both directions, same results for both. */
  
  if (InIpStatus == OutIpStatus)
  {
    // Return the value, no need to consider implementation flags.
    return InIpStatus;
  }
  
  
  /* Third possibility: matched both directions, different results */
  
  if ((InIpStatus & PKTFILTER_DISCARD) || (OutIpStatus & PKTFILTER_DISCARD))
  {
    // If either is DISCARD, return DISCARD.  No need to consider implementation flags.
    return PKTFILTER_DISCARD;
  }
  
  
  /* Fourth possibility: if we make it here, we have matched only one direction */
  
  // If we aren't using the SINGLE FILTER flag, then matching only one
  // direction is no dice.  Apply policy group 0 and return the default.
  if ((fImplMask & SINGLE_IP_FILTER_SUFFICIENT) == 0)
  {
    // ...and return the default.
    return IpFilter::GetDefault();
  }

  // If we ARE using the SINGLE FILTER flag, then matching just one interface
  // is good enough.  Return whichever one is not NO OPINION.  
  if (InIpStatus != PKTFILTER_NO_OPINION)
    return InIpStatus;
  
  return OutIpStatus;
}

PKTFILTER_STATUS BcmPacketFilterController::ApplyPacketFiltersImpl (BcmEthernetPacket &Pkt, 
               unsigned int InFromIx, unsigned int OutToIx)
{
#ifdef FILTER_LOGGING_SUPPORT
  if (BcmSnmpAgent::gMessageLogSettings.GetEnabledSeverities () & BcmSnmpAgent::kPktFilterMessages)
    LogFilt (Pkt, InFromIx, OutToIx);
#endif
  
  /* OK, this is a bridged packet.  Do whatever filtering we need to. */
  
  if (fMode == kOptimized)
  {
    // What kind of parsing should we do on this packet?
    if ((fInboundParseMask[InFromIx-1] & PARSE_TCPUDP)
    ||  (fOutboundParseMask[OutToIx-1] & PARSE_TCPUDP))
      Pkt.ParseTcpUdp();
    
    else if ((fInboundParseMask[InFromIx-1] & PARSE_IP)
    ||  (fOutboundParseMask[OutToIx-1] & PARSE_IP))
      Pkt.ParseIp();
    
    // If we are implementing ignore IP defaults if the tables
    // are empty, and if we have no IP filters, then we can return now.  If we
    // do have IP filters, then it means that the filters match all IP
    // traffic because our parse masks are 0, which means we must
    // proceed.
    else if ((fInboundIpFilters[InFromIx-1] == 0)
    && (fOutboundIpFilters[OutToIx-1] == 0)
    && (fImplMask & EMPTY_IP_DEFAULT_IGNORE))
      return PKTFILTER_NO_OPINION;
  }
  else
    Pkt.ParseTcpUdp ();
  
  // Apply IP (inbound then outbound), only if this is an IP packet.
  if (Pkt.IsIpPktParsed())
  {
    PKTFILTER_STATUS InIpStatus = ApplyInboundIpFilters (Pkt, InFromIx, OutToIx);
    PKTFILTER_STATUS OutIpStatus = ApplyOutboundIpFilters (Pkt, OutToIx);
    
    // Apply the results of the inbound & outbound IP filter parsing  
    PKTFILTER_STATUS Status = ApplyIpFilterResults (Pkt, InFromIx, InIpStatus, OutToIx, OutIpStatus);
    if (Status & PKTFILTER_DISCARD)
      return Status;
  }
    
  return PKTFILTER_NO_OPINION;
}

void BcmPacketFilterController::LogFilt (BcmEthernetPacket &Pkt, unsigned int InFromIx, unsigned int OutToIx) const
{
#ifdef FILTER_LOGGING_SUPPORT

  SnmpLogFilt << "\n****************************************************\n"
                   " PACKET from IF " << InFromIx << " to IF " << OutToIx << "\n"
                   "****************************************************\n" << Pkt << endl;
                   
  if (fMode == kOptimized)
  {
    // What kind of parsing should we do on this packet?
    if ((fInboundParseMask[InFromIx-1] & PARSE_TCPUDP)
    ||  (fOutboundParseMask[OutToIx-1] & PARSE_TCPUDP))
      SnmpLogFilt << "Optimized: parsing TCP & UDP" << endl;
    
    else if ((fInboundParseMask[InFromIx-1] & PARSE_IP)
    ||  (fOutboundParseMask[OutToIx-1] & PARSE_IP))
      SnmpLogFilt << "Optimized: parsing IP" << endl;
    
    else if ((fInboundParseMask[InFromIx-1] & PARSE_LLC)
    ||  (fOutboundParseMask[OutToIx-1] & PARSE_LLC))
      SnmpLogFilt << "Optimized: parsing LLC" << endl;
      
    else
      SnmpLogFilt << "Optimized: parsing nothing" << endl;
    
    // If we are implementing ignore both LLC and IP defaults if the tables
    // are empty, and if we have no IP filters, then we can return now.  If we
    // do have IP filters, then it means that the filters match all IP
    // traffic because our parse masks are 0, which means we must
    // proceed.
    if ((fInboundIpFilters[InFromIx-1] == 0)
    && (fOutboundIpFilters[OutToIx-1] == 0)
    && (fImplMask & EMPTY_IP_DEFAULT_IGNORE))
      SnmpLogFilt << "  Impl: no IP filters, ignoring defaults: NO OPINION" << endl;
      
    else
      SnmpLogFilt << "  Continuing - MATCH ALL filters or not ignoring defaults." << endl;
  }
  else
    SnmpLogFilt << "  Not optimized - parsing TCP & UDP" << endl;
#endif
}

void BcmPacketFilterController::LogIpFilterResults (unsigned int InFromIx, PKTFILTER_STATUS InIpStatus, unsigned int OutToIx, PKTFILTER_STATUS OutIpStatus) const
{
#ifdef FILTER_LOGGING_SUPPORT
  // Little shortcut for printing here:
  static const char *StatStr[] = {"NO OPINION", "DISCARD", "ACCEPT", "POLICY"};

  SnmpLogFilt << "IP filter results: Inbound=" << InIpStatus << ", Outbound=" << OutIpStatus;
  
  InIpStatus  &= 0x0003;
  OutIpStatus &= 0x0003;
  
  SnmpLogFilt << " (" << StatStr[InIpStatus] << " / " << StatStr[OutIpStatus] << ")" << endl;

  /* One possibility: didn't match a filter inbound or outbound. */
  
  if ((InIpStatus == PKTFILTER_NO_OPINION) && (OutIpStatus == PKTFILTER_NO_OPINION))
  {
    SnmpLogFilt << "  No match, inbound or outbound." << endl;
    
    // If we are using the ignore default when the table is empty
    // impl flag, return NO OPINION if we have no filters at all.
    if ((fInboundIpFilters[InFromIx-1] == 0)
    &&  (fOutboundIpFilters[OutToIx-1] == 0))
    {
      SnmpLogFilt << "  No filters, inbound or outbound." << endl;
    
      if (fImplMask & EMPTY_IP_DEFAULT_IGNORE)
        SnmpLogFilt << "    Impl: ignore default for no filters - NO OPINION" << endl;
      else
        SnmpLogFilt << "    Impl: default applies if no filters - " << StatStr[IpFilter::GetDefault() & 0x0003] << endl;
        
      return;
    }
      
    // Otherwise, apply any policies from group 0 and return the default.
    SnmpLogFilt << "  Filters: " << fInboundIpFilters[InFromIx-1] << " inbound, " << fOutboundIpFilters[OutToIx-1] << " outbound: apply default - " << StatStr[IpFilter::GetDefault() & 0x0003] << endl;
    return;
  }
  
  
  /* Second possibility: matched in both directions, same results for both. */
  
  if (InIpStatus == OutIpStatus)
  {
    SnmpLogFilt << "  Matched both directions the same - " << StatStr[InIpStatus] << endl;
    return;
  }
  
  
  /* Third possibility: matched both directions, one is discard */
  
  if ((InIpStatus & PKTFILTER_DISCARD) || (OutIpStatus & PKTFILTER_DISCARD))
  {
    SnmpLogFilt << "  Matched both directions, one is discard - DISCARD" << endl;
    return;
  }
  
  
  /* Fourth possibility: if we make it here, we have matched only one direction */
  
  SnmpLogFilt << "  Matched one direction only." << endl;
  
  // If we aren't using the SINGLE FILTER flag, then matching only one
  // direction is no dice.  Apply policy group 0 and return the default.
  if ((fImplMask & SINGLE_IP_FILTER_SUFFICIENT) == 0)
  {
    SnmpLogFilt << "    Impl: single filter gets default: " << StatStr[IpFilter::GetDefault() & 0x0003] << endl;
    return;
  }

  // If we ARE using the SINGLE FILTER flag, then matching just one interface
  // is good enough.  Return whichever one is not NO OPINION.
  if (InIpStatus != PKTFILTER_NO_OPINION)
  {
    SnmpLogFilt << "    Impl: single filter is sufficient: " << StatStr[InIpStatus] << endl;
    return;
  }
  
  SnmpLogFilt << "    Impl: single filter is sufficient: " << StatStr[OutIpStatus] << endl;
#endif
}

