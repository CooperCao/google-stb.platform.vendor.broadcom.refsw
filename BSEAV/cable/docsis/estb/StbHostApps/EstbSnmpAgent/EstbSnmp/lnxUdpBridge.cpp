//**************************************************************************
//                                                                          
//    Copyright 2003 Broadcom Corporation                                   
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
//    Description:                                                          
//                                                                          
//**************************************************************************
//    Revision History:                                                     
//                                                                          
//**************************************************************************
//    Filename: udpBridge.cpp
//    Creation Date: 11/14/03
//    Created by Broadcom V2/BFC/BOSS Mib Compiler BCMIBC.EXE version 1.4.0c
//    from input file rfc2013.mib
//
//**************************************************************************

#include "lnxUdpBridge.h"
#include "BcmSnmpAgent.h"
#include "HalIf.h"
#include "lnxUdpPort0Snoop.h"
#include "SnmpLog.h"

#include <sys/time.h>

// PR 6694: Use a single static snoop for port 0 instead of a discrete one
// per object.  Otherwise we can wind up with several snoops hanging around
// in a multiple-agent environment.  Actually having several snoops still
// works OK, but is resource-inefficient.  And in a dual-forwarder environment,
// the multiple snoops would not necessarily count the same, which is not
// exactly correct either since the stats in this group apply to the whole
// machine, they're not per-IP stack.
lnxUdpPort0Snoop *lnxUdpGroupBridge::fpPort0Snoop = NULL;

/******************************************************************************\

-- NOTE: Various non-SMI text removed below (as indicated) for benefit of MIB
-- compiler.  Complete MIB text may be obtained at:
-- http://www.ietf.org/rfc/rfc2013.txt
-- Kevin O'Neal, 10/23/2001

-- 1.  Introduction: REMOVED
-- and we really start with...
-- 2.  Definitions

UDP-MIB DEFINITIONS ::= BEGIN

IMPORTS
    MODULE-IDENTITY, OBJECT-TYPE, Counter32,
    IpAddress, mib-2                   FROM SNMPv2-SMI
    MODULE-COMPLIANCE, OBJECT-GROUP    FROM SNMPv2-CONF;


udpMIB MODULE-IDENTITY
    LAST-UPDATED "9411010000Z"
    ORGANIZATION "IETF SNMPv2 Working Group"
    CONTACT-INFO
            "        Keith McCloghrie

             Postal: Cisco Systems, Inc.
                     170 West Tasman Drive
                     San Jose, CA  95134-1706
                     US

             Phone:  +1 408 526 5260
             Email:  kzm@cisco.com"
    DESCRIPTION
            "The MIB module for managing UDP implementations."
    REVISION      "9103310000Z"
    DESCRIPTION
            "The initial revision of this MIB module was part of MIB-
            II."
    ::= { mib-2 50 }

-- the UDP group

udp      OBJECT IDENTIFIER ::= { mib-2 7 }


\******************************************************************************/




/*\
 *
 *  lnxUdpGroupBridge for bridging scalars from the udpGroup group 
 *
\*/

/* 
udp      OBJECT IDENTIFIER ::= { mib-2 7 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

long unsigned int lnxUdpGroupBridge::UdpInDatagrams = 0;
long unsigned int lnxUdpGroupBridge::UdpNoPorts = 0;
long unsigned int lnxUdpGroupBridge::UdpInErrors = 0;
long unsigned int lnxUdpGroupBridge::UdpOutDatagrams = 0;
struct inpcb * lnxUdpGroupBridge::udp_inpcb_list = NULL;

lnxUdpGroupBridge::lnxUdpGroupBridge (BcmSnmpAgent *pAgent)
  : udpGroupBridge (pAgent)
{
  if (fpPort0Snoop == NULL)
    fpPort0Snoop = new lnxUdpPort0Snoop (false);
  
  // Iterate over the interfaces for our agent's packet switch and install
  // the snoop outbound on any IP stacks therein.
  
  BcmPacketSwitch *pSwitch = pAgent->GetPacketSwitch();
  
  if (pSwitch)
  {
    unsigned int Loop;
    BcmHalIf *pHal;
    
    for (Loop = 0; Loop < pSwitch->NumberOfRegisteredHalIf(); Loop++)
    {
      // Retrieve the interface.
      pHal = pSwitch->Find(Loop);

      // Slap a port 0 snoop on him if he's an IP stack.
      if (pHal->IanaType() == BcmHalIf::kIanaTypeIpStack)
      {
        // Make sure he's not already there.  Could be that another agent
        // already put him there if we're in a multi-agent environment.
        if (pHal->SnoopIsRegistered(fpPort0Snoop, BcmHalIf::kOutbound) == false)
          pHal->AddSnoop(fpPort0Snoop, BcmHalIf::kOutbound);
      }
    }
  }
}

lnxUdpGroupBridge::~lnxUdpGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
udpInDatagrams OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The total number of UDP datagrams delivered to UDP users."
    ::= { udp 1 }
*/ 
unsigned int lnxUdpGroupBridge::Get_udpInDatagrams ()
{
  linux_read_udp_stat();

  return UdpInDatagrams;
}


/* 
udpNoPorts OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The total number of received UDP datagrams for which there
            was no application at the destination port."
    ::= { udp 2 }
*/ 
unsigned int lnxUdpGroupBridge::Get_udpNoPorts ()
{
  linux_read_udp_stat();

  return UdpNoPorts; 
}


/* 
udpInErrors OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of received UDP datagrams that could not be
            delivered for reasons other than the lack of an application
            at the destination port."
    ::= { udp 3 }
*/ 
unsigned int lnxUdpGroupBridge::Get_udpInErrors ()
{
  linux_read_udp_stat();

  return UdpInErrors;
}


/* 
udpOutDatagrams OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The total number of UDP datagrams sent from this entity."
    ::= { udp 4 }
*/ 
unsigned int lnxUdpGroupBridge::Get_udpOutDatagrams ()
{
  linux_read_udp_stat();

  return UdpOutDatagrams;
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/

struct inpcb *lnxUdpGroupBridge::UDP_Scan_Init(void)
{
    FILE *in;
    char line [256];
    struct inpcb **pp;
    struct timeval now;
    static unsigned long Time_Of_Last_Reload = 0;

    /*
     * save some cpu-cycles, and reload after 5 secs...
     */
    gettimeofday (&now, (struct timezone *) 0);
    if (Time_Of_Last_Reload + 5 > now.tv_sec)
      {
        return udp_inpcb_list;
      }
    Time_Of_Last_Reload = now.tv_sec;


    if (! (in = fopen ("/proc/net/udp", "r")))
      {
        return udp_inpcb_list;
      }

    /* free old chain: */
    while (udp_inpcb_list)
      {
        struct inpcb *p = udp_inpcb_list;
        udp_inpcb_list = udp_inpcb_list->inp_next;
        free (p);
      }

    /* scan proc-file and append: */

    pp = &udp_inpcb_list;

    while (line == fgets (line, sizeof(line), in))
      {
        struct inpcb pcb, *nnew;
        unsigned int state, lport, fport;
        unsigned long int laddr;

        if (4 != sscanf (line, "%*d: %x:%x %*x:%x %x",
                         &laddr, &lport, &fport, &state))
          continue;

        if (state != 7)         /* fix me:  UDP_LISTEN ??? */
          continue;

        pcb.inp_laddr.s_addr = htonl (laddr); // MRT PR7559
        pcb.inp_lport = (unsigned short) (lport);
        pcb.inp_fport = (unsigned short) (fport);

        nnew = (struct inpcb *) malloc (sizeof (struct inpcb));
        if (nnew == NULL) break;
        *nnew = pcb;
        nnew->inp_next = 0;

        *pp = nnew;
        pp = & nnew->inp_next;
      }

    fclose (in);

    /* first entry to go: */
    return udp_inpcb_list;
}

struct inpcb *lnxUdpGroupBridge::UDP_Scan_Next(struct inpcb *TargetInPcb)
{
        if (TargetInPcb == NULL)
          return NULL;

        return(TargetInPcb->inp_next);     
}



bool lnxUdpGroupBridge::UdpEntryExists (const BcmIpAddress &udpLocalAddress, int udpLocalPort)
{
  struct inpcb *pUdp;
  BcmIpAddress Ip;
  
  pUdp = udp_inpcb_list;
  
  while (pUdp)
  {
    // Create an IP address object from the linux udpTable info
    Ip.Set (pUdp->inp_laddr.s_addr);
    
    // SnmpLogReq << "UDP: " << Ip << ":" << pUdp->inp_lport << endl;
    
    if ((pUdp->inp_lport == udpLocalPort)
    &&  (Ip == udpLocalAddress))
      return true;
      
    pUdp = UDP_Scan_Next(pUdp);
  };

  return false;
}


void lnxUdpGroupBridge::UpdateUdpTable (BcmSnmpAgent *pAgent)
{
  static bool Updating = false;
  
  // Prevent reentrancy issues
  if (Updating)
    return;
    
  BcmMibTable *pTable;
  BcmMibEntry *pEntry;
  MibEntryIterator Pos;
  udpEntryBridge *pBridge;
  BcmIpAddress udpIp;
  int udpLocalPort;
  BcmObjectId Index;
  struct inpcb *pUdp;
  
  // Query the OS and see if the udpTable has changed.  If it has, 
  // take the necessary action.
  
  // First, go through the V2/BFC idea of the udpTable and delete any entries 
  // which the OS doesn't have.
  pTable = pAgent->FindTable(BcmString("udpTable"));
  if (pTable == NULL)
    return;
    
  Updating = true;
  
  SnmpLogReq << "Checking existing udpEntries..." << endl;
  
  pAgent->LockMib();    

  // Go out and do the kernel dive and get a completely new list
  udp_inpcb_list = UDP_Scan_Init();

  pEntry = pTable->FirstEntry(Pos);
  while (pEntry)
  {
    pBridge = (udpEntryBridge *)pEntry->fpMibBridge;
    
    // Populate linux udpEntry with the items from this V2 entry
    pBridge->Get_udpLocalAddress(udpIp);
    pEntry->GetIndex(Index);

    // If this entry doesn't exist in the linux table, flush the table and start
    // over.
    if (UdpEntryExists (udpIp, pBridge->Get_udpLocalPort()) == false)
    {
      SnmpLogReq << "  Found stale entry " << Index << ", rebuilding table." << endl;
      pTable->Flush();
      break;
    }
    else
      SnmpLogReq << "  Entry " << Index << " OK" <<endl;
    
    pEntry = pTable->NextEntry(Pos);
  }
  
  // Next, iterate through the OS's idea of the udpTable and create any new
  // entries that we need.
  SnmpLogReq << "Checking for new udpEntries..." << endl;
  
  for (pUdp = udp_inpcb_list; pUdp != NULL; pUdp = pUdp->inp_next)
  {
    // Create an IP address object from the linux udpTable info
    udpIp.Set (pUdp->inp_laddr.s_addr);
    udpLocalPort = pUdp->inp_lport;

    // Note that linux puts a 0.0.0.0:0 entry in the table for some reason.
    // Skip that sucker.
    if ((udpIp != kAllZerosIp) || (udpLocalPort != 0))    
    {
      Index = udpIp;
      Index += udpLocalPort;
      
      if (pTable->FindEntry (Index) == NULL)
      {
        SnmpLogReq << "  Creating new entry " << Index << endl;
        pBridge = new udpEntryBridge (udpIp, udpLocalPort, pAgent);
        if (pBridge->Register(kDestroyMibObject) != SNMP_NO_ERROR)
          delete pBridge;
      }
      else
        SnmpLogReq << "  Entry " << Index << " already exists." << endl;
    }
      
  }
  
  pAgent->UnlockMib();
  
  Updating = false;
}

void lnxUdpGroupBridge::linux_read_udp_stat ()
{
  FILE *in = fopen ("/proc/net/snmp", "r");
  char line [1024];

  if (! in)
    return;

  while (line == fgets (line, sizeof(line), in))
    {
      if (4 == sscanf (line, "Udp: %lu %lu %lu %lu\n",
                        &UdpInDatagrams, &UdpNoPorts,
                        &UdpInErrors, &UdpOutDatagrams))
        break;
    }
  fclose (in);
}

