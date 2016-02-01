//****************************************************************************
//
// Copyright (c) 2007-2009 Broadcom Corporation
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

#include "estbUdpBridge.h"
#include "BcmSnmpAgent.h"
// #include "HalIf.h"
#include "SnmpLog.h"

#include <sys/time.h>

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
}

lnxUdpGroupBridge::~lnxUdpGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/



/*
udpHCInDatagrams OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of UDP datagrams delivered to UDP
            users, for devices that can receive more than 1
            million UDP datagrams per second.

            Discontinuities in the value of this counter can occur
            at re-initialization of the management system, and at
            other times as indicated by discontinuities in the
            value of sysUpTime."
    ::= { udp 8 }
*/
unsigned int lnxUdpGroupBridge::Get_udpHCInDatagrams (BcmCounter64 &udpHCInDatagrams)
{
  linux_read_udp_stat();

  udpHCInDatagrams = UdpInDatagrams;

  return udpHCInDatagrams.Low32();
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
udpHCOutDatagrams OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of UDP datagrams sent from this
            entity, for devices that can transmit more than 1
            million UDP datagrams per second.

            Discontinuities in the value of this counter can occur
            at re-initialization of the management system, and at
            other times as indicated by discontinuities in the
            value of sysUpTime."
    ::= { udp 9 }
*/
unsigned int lnxUdpGroupBridge::Get_udpHCOutDatagrams (BcmCounter64 &udpHCOutDatagrams)
{
  linux_read_udp_stat();

  udpHCOutDatagrams = UdpOutDatagrams;

  return udpHCOutDatagrams.Low32();
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

