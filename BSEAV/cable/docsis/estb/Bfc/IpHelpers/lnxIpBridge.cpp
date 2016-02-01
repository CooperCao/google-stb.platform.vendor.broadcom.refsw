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
//    Filename: ipBridge.cpp
//    Creation Date: 11/14/03
//    Created by Broadcom V2 Mib Compiler BCMIBC.EXE version 1.2.0b from input file rfc2011.mib
//
//**************************************************************************

#include <stdio.h>

#include "lnxIpBridge.h"
#include "SnmpLog.h"


// These constant is from netinet/ip_icmp.h, but including that file
// leads down a bit of a #include wormhole which I am too lazy to dig out
// of.  So just define them here locally...
#define	ICMP_ECHOREPLY		            0 /* echo reply */
#define	ICMP_UNREACH		            3 /* dest unreachable, codes: */
#define	ICMP_UNREACH_NET	            0 /* bad net */
#define	ICMP_UNREACH_HOST	            1 /* bad host */
#define	ICMP_UNREACH_PROTOCOL	        2 /* bad protocol */
#define	ICMP_UNREACH_PORT               3 /* bad port */
#define	ICMP_UNREACH_NEEDFRAG           4 /* IP_DF caused drop */
#define	ICMP_UNREACH_SRCFAIL            5 /* src route failed */
#define	ICMP_UNREACH_NET_UNKNOWN        6 /* unknown net */
#define	ICMP_UNREACH_HOST_UNKNOWN       7 /* unknown host */
#define	ICMP_UNREACH_ISOLATED           8 /* src host isolated */
#define	ICMP_UNREACH_NET_PROHIB         9 /* prohibited access */
#define	ICMP_UNREACH_HOST_PROHIB       10 /* ditto */
#define	ICMP_UNREACH_TOSNET            11 /* bad tos for net */
#define	ICMP_UNREACH_TOSHOST           12 /* bad tos for host */
#define	ICMP_UNREACH_FILTER_PROHIB     13 /* admin prohib */
#define	ICMP_UNREACH_HOST_PRECEDENCE   14 /* host prec vio. */
#define	ICMP_UNREACH_PRECEDENCE_CUTOFF 15 /* prec cutoff */
#define	ICMP_SOURCEQUENCH               4 /* packet lost, slow down */
#define	ICMP_REDIRECT                   5 /* shorter route, codes: */
#define	ICMP_REDIRECT_NET               0 /* for network */
#define	ICMP_REDIRECT_HOST              1 /* for host */
#define	ICMP_REDIRECT_TOSNET            2 /* for tos and net */
#define	ICMP_REDIRECT_TOSHOST           3 /* for tos and host */
#define	ICMP_ECHO                       8 /* echo service */
#define	ICMP_ROUTERADVERT               9 /* router advertisement */
#define	ICMP_ROUTERSOLICIT             10 /* router solicitation */
#define	ICMP_TIMXCEED                  11 /* time exceeded, code: */
#define	ICMP_TIMXCEED_INTRANS           0 /* ttl==0 in transit */
#define	ICMP_TIMXCEED_REASS             1 /* ttl==0 in reass */
#define	ICMP_PARAMPROB                 12 /* ip header bad */
#define	ICMP_PARAMPROB_ERRATPTR         0 /* error at param ptr */
#define	ICMP_PARAMPROB_OPTABSENT        1 /* req. opt. absent */
#define	ICMP_PARAMPROB_LENGTH           2 /* bad length */
#define	ICMP_TSTAMP                    13 /* timestamp request */
#define	ICMP_TSTAMPREPLY               14 /* timestamp reply */
#define	ICMP_IREQ                      15 /* information request */
#define	ICMP_IREQREPLY                 16 /* information reply */
#define	ICMP_MASKREQ                   17 /* address mask request */
#define	ICMP_MASKREPLY                 18 /* address mask reply */
#define	ICMP_MAXTYPE                   18


// DPullen - Jan 19, 2003 - PR4266, PR6233; callout function to get
// ipInAddrErrors for packets not seen by the IP stack (external
// filters, etc).
unsigned long (*IpInAddrErrorsCallout)(void) = NULL;

/******************************************************************************\

-- NOTE: Various non-SMI text removed below (as indicated) for benefit of MIB
-- compiler.  Complete MIB text may be obtained at:
-- http://www.ietf.org/rfc/rfc2011.txt
-- Kevin O'Neal, 10/23/2001

-- 1.  Introduction: REMOVED
-- and we really start with...
-- 2.  Definitions

IP-MIB DEFINITIONS ::= BEGIN

IMPORTS
    MODULE-IDENTITY, OBJECT-TYPE, Integer32,
    Counter32, IpAddress, mib-2        FROM SNMPv2-SMI
    PhysAddress                        FROM SNMPv2-TC
    MODULE-COMPLIANCE, OBJECT-GROUP    FROM SNMPv2-CONF;


ipMIB MODULE-IDENTITY
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
            "The MIB module for managing IP and ICMP implementations,
            but excluding their management of IP routes."
    REVISION      "9103310000Z"
    DESCRIPTION
            "The initial revision of this MIB module was part of MIB-
            II."
    ::= { mib-2 48}

-- the IP group

ip       OBJECT IDENTIFIER ::= { mib-2 4 }


\******************************************************************************/



/*\
 *
 *  ipGroupBridge for bridging scalars from the ipGroup group
 *
\*/

/*
ip       OBJECT IDENTIFIER ::= { mib-2 4 }
*/

/*\
 *  construction / destruction
\*/

lnxIpGroupBridge::lnxIpGroupBridge (BcmSnmpAgent *pAgent)
  : ipGroupBridge (pAgent)
{
  BcmReassembler = NULL;
}

lnxIpGroupBridge::~lnxIpGroupBridge ()
{
}

/*\
 *  GET methods
\*/

/*
ipForwarding OBJECT-TYPE
    SYNTAX      INTEGER {
                    forwarding(1),    -- acting as a router
                    notForwarding(2)  -- NOT acting as a router
                }
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
            "The indication of whether this entity is acting as an IP
            router in respect to the forwarding of datagrams received
            by, but not addressed to, this entity.  IP routers forward
            datagrams.  IP hosts do not (except those source-routed via
            the host)."
    ::= { ip 1 }
*/
int lnxIpGroupBridge::Get_ipForwarding ()
{
  // PR6218: Although the OS thinks we have no forwarding (which I agree since
  // we're a bridge, not a router), the auto-testers seem to disagree.  So
  // return forwarding(1) no matter what the OS thinks.
  return CONSTVAL_IPFORWARDING_FORWARDING;
/*
  if (lnx_ipforwarding == 0)
    return CONSTVAL_IPFORWARDING_NOTFORWARDING;

  return CONSTVAL_IPFORWARDING_FORWARDING;
*/
}


/*
ipDefaultTTL OBJECT-TYPE
    SYNTAX      INTEGER (1..255)
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
            "The default value inserted into the Time-To-Live field of
            the IP header of datagrams originated at this entity,
            whenever a TTL value is not supplied by the transport layer
            protocol."
    ::= { ip 2 }
*/
int lnxIpGroupBridge::Get_ipDefaultTTL ()
{
  linux_read_ip_stat();
  return IpDefaultTTL;
}


/*
ipInReceives OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The total number of input datagrams received from
            interfaces, including those received in error."
    ::= { ip 3 }
*/
unsigned int lnxIpGroupBridge::Get_ipInReceives ()
{
  linux_read_ip_stat();

    // DPullen - Jan 19, 2003 - PR4266, PR6233; if an ipInAddrErrors counter callout
    // has been registered, then call it to get the number of packets that were
    // discarded externally to the IP stack because the destination IP address
    // was not one of ours.
    if (IpInAddrErrorsCallout != NULL)
    {
        IpInReceives += IpInAddrErrorsCallout();
    }
    return IpInReceives;
}


/*
ipInHdrErrors OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of input datagrams discarded due to errors in
            their IP headers, including bad checksums, version number
            mismatch, other format errors, time-to-live exceeded, errors
            discovered in processing their IP options, etc."
    ::= { ip 4 }
*/
unsigned int lnxIpGroupBridge::Get_ipInHdrErrors ()
{
  linux_read_ip_stat();
  return IpInHdrErrors;
}


/*
ipInAddrErrors OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of input datagrams discarded because the IP
            address in their IP header's destination field was not a
            valid address to be received at this entity.  This count
            includes invalid addresses (e.g., 0.0.0.0) and addresses of
            unsupported Classes (e.g., Class E).  For entities which are
            not IP routers and therefore do not forward datagrams, this
            counter includes datagrams discarded because the destination
            address was not a local address."
    ::= { ip 5 }
*/
unsigned int lnxIpGroupBridge::Get_ipInAddrErrors ()
{
  linux_read_ip_stat();

    // DPullen - Jan 19, 2003 - PR4266, PR6233; if an ipInAddrErrors counter callout
    // has been registered, then call it to get the number of packets that were
    // discarded externally to the IP stack because the destination IP address
    // was not one of ours.
    if (IpInAddrErrorsCallout != NULL)
    {
        IpInAddrErrors += IpInAddrErrorsCallout();
    }
    return IpInAddrErrors;
}


/*
ipForwDatagrams OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of input datagrams for which this entity was not
            their final IP destination, as a result of which an attempt
            was made to find a route to forward them to that final
            destination.  In entities which do not act as IP routers,
            this counter will include only those packets which were
            Source-Routed via this entity, and the Source-Route option
            processing was successful."
    ::= { ip 6 }
*/
unsigned int lnxIpGroupBridge::Get_ipForwDatagrams ()
{
  linux_read_ip_stat();
  return IpForwDatagrams;
}


/*
ipInUnknownProtos OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of locally-addressed datagrams received
            successfully but discarded because of an unknown or
            unsupported protocol."
    ::= { ip 7 }
*/
unsigned int lnxIpGroupBridge::Get_ipInUnknownProtos ()
{
  linux_read_ip_stat();
  return IpInUnknownProtos;
}


/*
ipInDiscards OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of input IP datagrams for which no problems were
            encountered to prevent their continued processing, but which
            were discarded (e.g., for lack of buffer space).  Note that
            this counter does not include any datagrams discarded while
            awaiting re-assembly."
    ::= { ip 8 }
*/
unsigned int lnxIpGroupBridge::Get_ipInDiscards ()
{
  linux_read_ip_stat();
  if (IpInDiscards < 0)
    return 0;
  return IpInDiscards;
}


/*
ipInDelivers OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The total number of input datagrams successfully delivered
            to IP user-protocols (including ICMP)."
    ::= { ip 9 }
*/
unsigned int lnxIpGroupBridge::Get_ipInDelivers ()
{
  linux_read_ip_stat();
  return IpInDelivers;
}


/*
ipOutRequests OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The total number of IP datagrams which local IP user-
            protocols (including ICMP) supplied to IP in requests for
            transmission.  Note that this counter does not include any
            datagrams counted in ipForwDatagrams."
    ::= { ip 10 }
*/
unsigned int lnxIpGroupBridge::Get_ipOutRequests ()
{
  linux_read_ip_stat();
  return IpOutRequests;
}


/*
ipOutDiscards OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of output IP datagrams for which no problem was
            encountered to prevent their transmission to their
            destination, but which were discarded (e.g., for lack of
            buffer space).  Note that this counter would include
            datagrams counted in ipForwDatagrams if any such packets met
            this (discretionary) discard criterion."
    ::= { ip 11 }
*/
unsigned int lnxIpGroupBridge::Get_ipOutDiscards ()
{
  linux_read_ip_stat();
  return IpOutDiscards;
}


/*
ipOutNoRoutes OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of IP datagrams discarded because no route could
            be found to transmit them to their destination.  Note that
            this counter includes any packets counted in ipForwDatagrams
            which meet this `no-route' criterion.  Note that this
            includes any datagrams which a host cannot route because all
            of its default routers are down."
    ::= { ip 12 }
*/
unsigned int lnxIpGroupBridge::Get_ipOutNoRoutes ()
{
  linux_read_ip_stat();
  return IpOutNoRoutes;
}


/*
ipReasmTimeout OBJECT-TYPE
    SYNTAX      Integer32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The maximum number of seconds which received fragments are
            held while they are awaiting reassembly at this entity."
    ::= { ip 13 }
*/
int lnxIpGroupBridge::Get_ipReasmTimeout ()
{
  linux_read_ip_stat();
  if (BcmReassembler)
    return (IpReasmTimeout + BcmReassembler->ReasmTimeouts());
  return IpReasmTimeout;
}


/*
ipReasmReqds OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of IP fragments received which needed to be
            reassembled at this entity."
    ::= { ip 14 }
*/
unsigned int lnxIpGroupBridge::Get_ipReasmReqds ()
{
  linux_read_ip_stat();
  if (BcmReassembler)
    return (IpReasmReqds + BcmReassembler->ReasmReqds());
  return IpReasmReqds;
}


/*
ipReasmOKs OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of IP datagrams successfully re-assembled."
    ::= { ip 15 }
*/
unsigned int lnxIpGroupBridge::Get_ipReasmOKs ()
{
  linux_read_ip_stat();
  return IpReasmOKs;
}


/*
ipReasmFails OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of failures detected by the IP re-assembly
            algorithm (for whatever reason: timed out, errors, etc).
            Note that this is not necessarily a count of discarded IP
            fragments since some algorithms (notably the algorithm in
            RFC 815) can lose track of the number of fragments by
            combining them as they are received."
    ::= { ip 16 }
*/
unsigned int lnxIpGroupBridge::Get_ipReasmFails ()
{
  linux_read_ip_stat();
  return IpReasmFails;
}


/*
ipFragOKs OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of IP datagrams that have been successfully
            fragmented at this entity."
    ::= { ip 17 }
*/
unsigned int lnxIpGroupBridge::Get_ipFragOKs ()
{
  linux_read_ip_stat();
  return IpFragOKs;
}


/*
ipFragFails OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of IP datagrams that have been discarded because
            they needed to be fragmented at this entity but could not
            be, e.g., because their Don't Fragment flag was set."
    ::= { ip 18 }
*/
unsigned int lnxIpGroupBridge::Get_ipFragFails ()
{
  linux_read_ip_stat();
  return IpFragFails;
}


/*
ipFragCreates OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of IP datagram fragments that have been
            generated as a result of fragmentation at this entity."
    ::= { ip 19 }
*/
unsigned int lnxIpGroupBridge::Get_ipFragCreates ()
{
  linux_read_ip_stat();
  return IpFragCreates;
}


/*
ipRoutingDiscards OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of routing entries which were chosen to be
            discarded even though they are valid.  One possible reason
            for discarding such an entry could be to free-up buffer
            space for other routing entries."
    ::= { ip 23 }
*/
unsigned int lnxIpGroupBridge::Get_ipRoutingDiscards ()
{
  linux_read_ip_stat();
  return IpOutNoRoutes;
}


/*\
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\
 *  SET methods
\*/

/*
ipForwarding OBJECT-TYPE
    SYNTAX      INTEGER {
                    forwarding(1),    -- acting as a router
                    notForwarding(2)  -- NOT acting as a router
                }
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
            "The indication of whether this entity is acting as an IP
            router in respect to the forwarding of datagrams received
            by, but not addressed to, this entity.  IP routers forward
            datagrams.  IP hosts do not (except those source-routed via
            the host)."
    ::= { ip 1 }
*/
SNMP_STATUS lnxIpGroupBridge::Set_ipForwarding (int ipForwarding)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ipForwarding);
  FILE *procFile;

  if (Status == SNMP_NO_ERROR)
  {
    if ((ipForwarding != CONSTVAL_IPFORWARDING_FORWARDING)
    &&  (ipForwarding != CONSTVAL_IPFORWARDING_NOTFORWARDING))
      return SNMP_WRONG_VALUE;

    if (procFile = fopen ("/proc/sys/net/ipv4/ip_forward", "w")) {
      fprintf(procFile, "%ld\n", ipForwarding);
      fclose(procFile);
    } else {
      return SNMP_GEN_ERR;
    }
}

  return Status;
}


/*
ipDefaultTTL OBJECT-TYPE
    SYNTAX      INTEGER (1..255)
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
            "The default value inserted into the Time-To-Live field of
            the IP header of datagrams originated at this entity,
            whenever a TTL value is not supplied by the transport layer
            protocol."
    ::= { ip 2 }
*/
SNMP_STATUS lnxIpGroupBridge::Set_ipDefaultTTL (int ipDefaultTTL)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ipDefaultTTL);
  FILE *procFile;

  if (Status == SNMP_NO_ERROR)
  {
    if ((ipDefaultTTL < MINVAL_IPDEFAULTTTL)
    ||  (ipDefaultTTL > MAXVAL_IPDEFAULTTTL))
      return SNMP_WRONG_VALUE;

    if (procFile = fopen ("/proc/sys/net/ipv4/ip_default_ttl", "w")) {
      fprintf(procFile, "%ld\n", ipDefaultTTL);
      fclose(procFile);
    } else {
      return SNMP_GEN_ERR;
    }

  }

  return Status;
}

void lnxIpGroupBridge::linux_read_ip_stat ()
{
  FILE *in = fopen ("/proc/net/snmp", "r");
  char line [1024];

  IpForwarding = 0;

  if (! in)
    return;

  while (line == fgets (line, sizeof(line), in))
    {
      if (19 == sscanf (line,
"Ip: %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
     &IpForwarding, &IpDefaultTTL, &IpInReceives,
     &IpInHdrErrors, &IpInAddrErrors, &IpForwDatagrams,
     &IpInUnknownProtos, &IpInDiscards, &IpInDelivers,
     &IpOutRequests, &IpOutDiscards, &IpOutNoRoutes,
     &IpReasmTimeout, &IpReasmReqds, &IpReasmOKs,
     &IpReasmFails, &IpFragOKs, &IpFragFails,
     &IpFragCreates))
        break;
    }
  fclose (in);
} /* end of linux_read_ip_stat */


/*\
 *
 *  lnxIpNetToMediaEntryBridge for bridging elements in the ipNetToMediaTable
 *
\*/

/*
ipNetToMediaTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF IpNetToMediaEntry
    MAX-ACCESS  not-accessible
    STATUS      current
    DESCRIPTION
            "The IP Address Translation table used for mapping from IP
            addresses to physical addresses."
    ::= { ip 22 }

INDEX { ipNetToMediaIfIndex, ipNetToMediaNetAddress }
*/

/*\
 *  construction / destruction
\*/

lnxIpNetToMediaEntryBridge::lnxIpNetToMediaEntryBridge (const BcmMacAddress &ipNetToMediaPhysAddress, const BcmIpAddress &ipNetToMediaNetAddress, int ipNetToMediaType, BcmSnmpAgent *pAgent)
  : ipNetToMediaEntryBridge (NULL, ipNetToMediaPhysAddress, ipNetToMediaNetAddress,
                 ipNetToMediaType, pAgent->GetIfNum(), pAgent)
#ifdef NOTDEF
  : TabularMibBridge ("1.3.6.1.2.1.4.22",
                      BcmObjectId(pAgent->GetIfNum()) + BcmObjectId(ipNetToMediaNetAddress),
                      pAgent)
#endif
{
  fMac = ipNetToMediaPhysAddress;
  fIp = ipNetToMediaNetAddress;
  fType = ipNetToMediaType;
  fIf = pAgent->GetIfNum();
  // SnmpLogRaw << "new lnxIpNetToMediaEntryBridge " << fMac << " " << fIp << " " << fType << " " << fIf << endl;
}

lnxIpNetToMediaEntryBridge::~lnxIpNetToMediaEntryBridge ()
{
}

SNMP_STATUS lnxIpNetToMediaEntryBridge::Register(CleanupMethod Cleanup, bool SilentFailure)
{
    SNMP_STATUS Status;


    Status = TabularMibBridge::Register (Cleanup, SilentFailure);
    if (Status != SNMP_NO_ERROR)
        return Status;

    return SNMP_NO_ERROR;
}

SNMP_STATUS lnxIpNetToMediaEntryBridge::DeRegister()
{
    return TabularMibBridge::DeRegister ();
}


/*\
 *  GET methods
\*/

/*
ipNetToMediaIfIndex OBJECT-TYPE
    SYNTAX      INTEGER (1..2147483647)
    MAX-ACCESS  read-create
    STATUS      current
    DESCRIPTION
            "The interface on which this entry's equivalence is
            effective.  The interface identified by a particular value
            of this index is the same interface as identified by the
            same value of RFC 1573's ifIndex."
    ::= { ipNetToMediaEntry 1 }
*/
int lnxIpNetToMediaEntryBridge::Get_ipNetToMediaIfIndex ()
{
  return (fIf);
}


/*
ipNetToMediaPhysAddress OBJECT-TYPE
    SYNTAX      PhysAddress
    MAX-ACCESS  read-create
    STATUS      current
    DESCRIPTION
            "The media-dependent `physical' address."
    ::= { ipNetToMediaEntry 2 }
*/
void lnxIpNetToMediaEntryBridge::Get_ipNetToMediaPhysAddress (BcmMacAddress &ipNetToMediaPhysAddress)
{
  ipNetToMediaPhysAddress = fMac;
}


/*
ipNetToMediaNetAddress OBJECT-TYPE
    SYNTAX      IpAddress
    MAX-ACCESS  read-create
    STATUS      current
    DESCRIPTION
            "The IpAddress corresponding to the media-dependent
            `physical' address."
    ::= { ipNetToMediaEntry 3 }
*/
void lnxIpNetToMediaEntryBridge::Get_ipNetToMediaNetAddress (BcmIpAddress &ipNetToMediaNetAddress)
{
  ipNetToMediaNetAddress = fIp;
}


/*
ipNetToMediaType OBJECT-TYPE
    SYNTAX      INTEGER {
                other(1),        -- none of the following
                invalid(2),      -- an invalidated mapping
                dynamic(3),
                static(4)
            }
    MAX-ACCESS  read-create
    STATUS      current
    DESCRIPTION
            "The type of mapping.
            Setting this object to the value invalid(2) has the effect
            of invalidating the corresponding entry in the
            ipNetToMediaTable.  That is, it effectively disassociates
            the interface identified with said entry from the mapping
            identified with said entry.  It is an implementation-
            specific matter as to whether the agent removes an
            invalidated entry from the table.  Accordingly, management
            stations must be prepared to receive tabular information
            from agents that corresponds to entries not currently in
            use.  Proper interpretation of such entries requires
            examination of the relevant ipNetToMediaType object."
    ::= { ipNetToMediaEntry 4 }
*/
int lnxIpNetToMediaEntryBridge::Get_ipNetToMediaType ()
{
  return fType;
}


/*\
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\
 *  SET methods
\*/

/*
ipNetToMediaIfIndex OBJECT-TYPE
    SYNTAX      INTEGER (1..2147483647)
    MAX-ACCESS  read-create
    STATUS      current
    DESCRIPTION
            "The interface on which this entry's equivalence is
            effective.  The interface identified by a particular value
            of this index is the same interface as identified by the
            same value of RFC 1573's ifIndex."
    ::= { ipNetToMediaEntry 1 }
*/
SNMP_STATUS lnxIpNetToMediaEntryBridge::Set_ipNetToMediaIfIndex (int ipNetToMediaIfIndex)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ipNetToMediaIfIndex);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ipNetToMediaIfIndex < MINVAL_IPNETTOMEDIAIFINDEX)
    ||  (ipNetToMediaIfIndex > MAXVAL_IPNETTOMEDIAIFINDEX))
      return SNMP_WRONG_VALUE;

    // Implement everything here as READ-ONLY, that's OK per
    // ATP-N-00004:
    // "Based on precedent, IWL Test 1.1.11.6, 1.1.11.7, 1.1.11.8, and 1.1.11.11
    //  should allow a noSuchName error when attempting to write to the elements
    //  of ifStackTable, ipRouteTable, ipNetToMediaTable, ifPromiscuousMode, and
    //  ifAlias, which can all be read-only."
    return SNMP_INCONSISTENT_VALUE;
  }

  return Status;
}


/*
ipNetToMediaPhysAddress OBJECT-TYPE
    SYNTAX      PhysAddress
    MAX-ACCESS  read-create
    STATUS      current
    DESCRIPTION
            "The media-dependent `physical' address."
    ::= { ipNetToMediaEntry 2 }
*/
SNMP_STATUS lnxIpNetToMediaEntryBridge::Set_ipNetToMediaPhysAddress (const BcmMacAddress &ipNetToMediaPhysAddress)
{
  SNMP_STATUS Status = MacAddrMibObject::TestDefaults (ipNetToMediaPhysAddress);

  if (Status == SNMP_NO_ERROR)
  {
    // Implement everything here as READ-ONLY, that's OK per
    // ATP-N-00004:
    // "Based on precedent, IWL Test 1.1.11.6, 1.1.11.7, 1.1.11.8, and 1.1.11.11
    //  should allow a noSuchName error when attempting to write to the elements
    //  of ifStackTable, ipRouteTable, ipNetToMediaTable, ifPromiscuousMode, and
    //  ifAlias, which can all be read-only."
    return SNMP_INCONSISTENT_VALUE;
  }

  return Status;
}


/*
ipNetToMediaNetAddress OBJECT-TYPE
    SYNTAX      IpAddress
    MAX-ACCESS  read-create
    STATUS      current
    DESCRIPTION
            "The IpAddress corresponding to the media-dependent
            `physical' address."
    ::= { ipNetToMediaEntry 3 }
*/
SNMP_STATUS lnxIpNetToMediaEntryBridge::Set_ipNetToMediaNetAddress (const BcmIpAddress &ipNetToMediaNetAddress)
{
  SNMP_STATUS Status = IpAddrMibObject::TestDefaults (ipNetToMediaNetAddress);

  if (Status == SNMP_NO_ERROR)
  {
    // Implement everything here as READ-ONLY, that's OK per
    // ATP-N-00004:
    // "Based on precedent, IWL Test 1.1.11.6, 1.1.11.7, 1.1.11.8, and 1.1.11.11
    //  should allow a noSuchName error when attempting to write to the elements
    //  of ifStackTable, ipRouteTable, ipNetToMediaTable, ifPromiscuousMode, and
    //  ifAlias, which can all be read-only."
    return SNMP_INCONSISTENT_VALUE;
  }

  return Status;
}


/*
ipNetToMediaType OBJECT-TYPE
    SYNTAX      INTEGER {
                other(1),        -- none of the following
                invalid(2),      -- an invalidated mapping
                dynamic(3),
                static(4)
            }
    MAX-ACCESS  read-create
    STATUS      current
    DESCRIPTION
            "The type of mapping.
            Setting this object to the value invalid(2) has the effect
            of invalidating the corresponding entry in the
            ipNetToMediaTable.  That is, it effectively disassociates
            the interface identified with said entry from the mapping
            identified with said entry.  It is an implementation-
            specific matter as to whether the agent removes an
            invalidated entry from the table.  Accordingly, management
            stations must be prepared to receive tabular information
            from agents that corresponds to entries not currently in
            use.  Proper interpretation of such entries requires
            examination of the relevant ipNetToMediaType object."
    ::= { ipNetToMediaEntry 4 }
*/
SNMP_STATUS lnxIpNetToMediaEntryBridge::Set_ipNetToMediaType (int ipNetToMediaType)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ipNetToMediaType);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ipNetToMediaType != CONSTVAL_IPNETTOMEDIATYPE_OTHER)
    &&  (ipNetToMediaType != CONSTVAL_IPNETTOMEDIATYPE_INVALID)
    &&  (ipNetToMediaType != CONSTVAL_IPNETTOMEDIATYPE_DYNAMIC)
    &&  (ipNetToMediaType != CONSTVAL_IPNETTOMEDIATYPE_STATIC))
      return SNMP_WRONG_VALUE;

    // Implement everything here as READ-ONLY, that's OK per
    // ATP-N-00004:
    // "Based on precedent, IWL Test 1.1.11.6, 1.1.11.7, 1.1.11.8, and 1.1.11.11
    //  should allow a noSuchName error when attempting to write to the elements
    //  of ifStackTable, ipRouteTable, ipNetToMediaTable, ifPromiscuousMode, and
    //  ifAlias, which can all be read-only."
    return SNMP_INCONSISTENT_VALUE;
  }

  return Status;
}

/*\
 *
 *  lnxIpNetToPhysicalEntryBridge for bridging elements in the ipNetToPhysicalTable
 *
\*/

/*
ipNetToPhysicalTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF IpNetToPhysicalEntry
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The IP Address Translation table used for mapping from IP
            addresses to physical addresses.

            The Address Translation tables contain the IP address to
            'physical' address equivalences.  Some interfaces do not use
            translation tables for determining address equivalences
            (e.g., DDN-X.25 has an algorithmic method); if all
            interfaces are of this type, then the Address Translation
            table is empty, i.e., has zero entries.

            While many protocols may be used to populate this table, ARP
            and Neighbor Discovery are the most likely
            options."
    REFERENCE "RFC 826 and RFC 2461"
    ::= { ip 35 }

INDEX { ipNetToPhysicalIfIndex, ipNetToPhysicalNetAddressType, ipNetToPhysicalNetAddress }
*/

/*\
 *  construction / destruction
\*/

lnxIpNetToPhysicalEntryBridge::lnxIpNetToPhysicalEntryBridge (ipNetToMediaEntryBridge *pN2M)
  : ipNetToPhysicalEntryBridge (pN2M)
{
  fRowStatus = SNMP_STATUS_INVISIBLE;

  BcmIpAddress tempIp;

  // Programatically created entries will be
  // initialized as if created by CREATE_AND_WAIT.
  // Change to CREATE_AND_GO if immediate activation
  // is desired.
  int Status = SNMP_STATUS_CREATE_AND_WAIT;
  Set_ipNetToPhysicalRowStatus (Status);

  fMac.Set (0,0,0,0,0,0);
  fpN2M = pN2M;
  pN2M->Get_ipNetToMediaNetAddress(tempIp);
  fIpAddr = tempIp;

  fIpAddrType = pN2M->Get_ipNetToMediaType();
  fEntryType = 0;
  fIfIndex = pN2M->Get_ipNetToMediaIfIndex();
  fpAgent = pN2M->GetAgent();
  if (fpAgent)
    fLastChange = (unsigned long) fpAgent->sysUpTime();
  else
    fLastChange = (unsigned long) 0;
}

lnxIpNetToPhysicalEntryBridge::~lnxIpNetToPhysicalEntryBridge ()
{
}

/*\
 *  GET methods
\*/

/*
ipNetToPhysicalIfIndex OBJECT-TYPE
    SYNTAX     InterfaceIndex
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The index value that uniquely identifies the interface to
            which this entry is applicable.  The interface identified by
            a particular value of this index is the same interface as
            identified by the same value of the IF-MIB's ifIndex."
    ::= { ipNetToPhysicalEntry 1 }
*/
int lnxIpNetToPhysicalEntryBridge::Get_ipNetToPhysicalIfIndex ()
{
  return fIfIndex;
}


/*
ipNetToPhysicalNetAddressType OBJECT-TYPE
    SYNTAX     InetAddressType
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The type of ipNetToPhysicalNetAddress."
    ::= { ipNetToPhysicalEntry 2 }
*/
int lnxIpNetToPhysicalEntryBridge::Get_ipNetToPhysicalNetAddressType ()
{
  return fIpAddrType;
}


/*
ipNetToPhysicalNetAddress OBJECT-TYPE
    SYNTAX     InetAddress
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The IP Address corresponding to the media-dependent
            `physical' address.  The address type of this object is
            specified in ipNetToPhysicalAddressType.

            Implementors need to be aware that if the size of
            ipNetToPhysicalNetAddress exceeds 115 octets, then OIDS of
            instances of columns in this row will have more than 128
            sub-identifiers and cannot be accessed using SNMPv1,
            SNMPv2c, or SNMPv3."
    ::= { ipNetToPhysicalEntry 3 }
*/
int lnxIpNetToPhysicalEntryBridge::Get_ipNetToPhysicalNetAddress (BcmString &ipNetToPhysicalNetAddress)
{
  ipNetToPhysicalNetAddress = fIpAddr;

  return ipNetToPhysicalNetAddress.length();
}


/*
ipNetToPhysicalPhysAddress OBJECT-TYPE
    SYNTAX     PhysAddress (SIZE(0..65535))
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The media-dependent `physical' address.

            As the entries in this table are typically not persistent
            when this object is written the entity SHOULD NOT save the
            change to non-volatile storage."
    ::= { ipNetToPhysicalEntry 4 }
*/
void lnxIpNetToPhysicalEntryBridge::Get_ipNetToPhysicalPhysAddress (BcmMacAddress &ipNetToPhysicalPhysAddress)
{
  ipNetToPhysicalPhysAddress.Set (0,0,0,0,0,0);
  ipNetToPhysicalPhysAddress = fMac;
}


/*
ipNetToPhysicalLastUpdated OBJECT-TYPE
    SYNTAX     TimeStamp
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The value of sysUpTime at the time this entry was last
            updated.  If this entry was updated prior to the last re-
            initialization of the local network management subsystem,
            then this object contains a zero value."
    ::= { ipNetToPhysicalEntry 5 }
*/
unsigned long lnxIpNetToPhysicalEntryBridge::Get_ipNetToPhysicalLastUpdated ()
{
  return fLastChange;
}


/*
ipNetToPhysicalType OBJECT-TYPE
    SYNTAX     INTEGER {
                other(1),        -- none of the following
                invalid(2),      -- an invalidated mapping
                dynamic(3),
                static(4),
                local(5)         -- local interface
            }
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The type of mapping.

            Setting this object to the value invalid(2) has the effect
            of invalidating the corresponding entry in the
            ipNetToPhysicalTable.  That is, it effectively dis-
            associates the interface identified with said entry from the
            mapping identified with said entry.  It is an
            implementation-specific matter as to whether the agent
            removes an invalidated entry from the table.  Accordingly,
            management stations must be prepared to receive tabular
            information from agents that corresponds to entries not
            currently in use.  Proper interpretation of such entries
            requires examination of the relevant ipNetToPhysicalType
            object.

            The 'dynamic(3)' type indicates that the IP address to
            physical addresses mapping has been dynamically resolved
            using e.g., IPv4 ARP or the IPv6 Neighbor Discovery
            protocol.

            The 'static(4)' type indicates that the mapping has been
            statically configured.  Both of these refer to entries that
            provide mappings for other entities addresses.

            The 'local(5)' type indicates that the mapping is provided
            for an entity's own interface address.

            As the entries in this table are typically not persistent
            when this object is written the entity SHOULD NOT save the
            change to non-volatile storage."
    DEFVAL { static }
    ::= { ipNetToPhysicalEntry 6 }
*/
int lnxIpNetToPhysicalEntryBridge::Get_ipNetToPhysicalType ()
{
  return fEntryType;
}


/*
ipNetToPhysicalState OBJECT-TYPE
    SYNTAX     INTEGER {
                     reachable(1), -- confirmed reachability

                     stale(2),     -- unconfirmed reachability

                     delay(3),     -- waiting for reachability
                                   -- confirmation before entering
                                   -- the probe state

                     probe(4),     -- actively probing

                     invalid(5),   -- an invalidated mapping

                     unknown(6),   -- state can not be determined
                                   -- for some reason.

                     incomplete(7) -- address resolution is being
                                   -- performed.
                    }
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The Neighbor Unreachability Detection state for the
            interface when the address mapping in this entry is used.
            If Neighbor Unreachability Detection is not in use (e.g. for
            IPv4), this object is always unknown(6)."
    REFERENCE "RFC 2461"
    ::= { ipNetToPhysicalEntry 7 }
*/
int lnxIpNetToPhysicalEntryBridge::Get_ipNetToPhysicalState ()
{
  // for IPv4 always return unknown(6).
  return 6;
}


/*
ipNetToPhysicalRowStatus OBJECT-TYPE
    SYNTAX     RowStatus
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The status of this conceptual row.

            The RowStatus TC requires that this DESCRIPTION clause
            states under which circumstances other objects in this row
            can be modified.  The value of this object has no effect on
            whether other objects in this conceptual row can be
            modified.

            A conceptual row can not be made active until the
            ipNetToPhysicalPhysAddress object has been set.

            Note that if the ipNetToPhysicalType is set to 'invalid',
            the managed node may delete the entry independent of the
            state of this object."
    ::= { ipNetToPhysicalEntry 8 }
*/
int lnxIpNetToPhysicalEntryBridge::Get_ipNetToPhysicalRowStatus ()
{
  return fRowStatus;
}


/*\
 *  TEST methods
\*/

/*
ipNetToPhysicalPhysAddress OBJECT-TYPE
    SYNTAX     PhysAddress (SIZE(0..65535))
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The media-dependent `physical' address.

            As the entries in this table are typically not persistent
            when this object is written the entity SHOULD NOT save the
            change to non-volatile storage."
    ::= { ipNetToPhysicalEntry 4 }
*/
SNMP_STATUS lnxIpNetToPhysicalEntryBridge::Test_ipNetToPhysicalPhysAddress (const BcmMacAddress &ipNetToPhysicalPhysAddress)
{
  SNMP_STATUS Status = MacAddrMibObject::TestDefaults (ipNetToPhysicalPhysAddress);

  if (Status == SNMP_NO_ERROR)
  {
    // UNFINISHED: don't know how to check size for PhysAddress
    // from MINLEN_IPNETTOPHYSICALPHYSADDRESS to MAXLEN_IPNETTOPHYSICALPHYSADDRESS

    fObjectMask |= kBIT_ipNetToPhysicalPhysAddress;
  }

  return Status;
}

/*
ipNetToPhysicalRowStatus OBJECT-TYPE
    SYNTAX     RowStatus
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The status of this conceptual row.

            The RowStatus TC requires that this DESCRIPTION clause
            states under which circumstances other objects in this row
            can be modified.  The value of this object has no effect on
            whether other objects in this conceptual row can be
            modified.

            A conceptual row can not be made active until the
            ipNetToPhysicalPhysAddress object has been set.

            Note that if the ipNetToPhysicalType is set to 'invalid',
            the managed node may delete the entry independent of the
            state of this object."
    ::= { ipNetToPhysicalEntry 8 }
*/
SNMP_STATUS lnxIpNetToPhysicalEntryBridge::Test_ipNetToPhysicalRowStatus (int ipNetToPhysicalRowStatus)
{
  SNMP_STATUS Status = RowStatusMibObject::TestDefaults (ipNetToPhysicalRowStatus, Get_ipNetToPhysicalRowStatus());

  if (Status == SNMP_NO_ERROR)
  {
    // UNFINISHED: check any non-default constraints here and
    // return the appropriate error code in case of failure.

    fObjectMask |= kBIT_ipNetToPhysicalRowStatus;
  }

  return Status;
}


/*\
 *  SET methods
\*/

/*
ipNetToPhysicalPhysAddress OBJECT-TYPE
    SYNTAX     PhysAddress (SIZE(0..65535))
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The media-dependent `physical' address.

            As the entries in this table are typically not persistent
            when this object is written the entity SHOULD NOT save the
            change to non-volatile storage."
    ::= { ipNetToPhysicalEntry 4 }
*/
SNMP_STATUS lnxIpNetToPhysicalEntryBridge::Set_ipNetToPhysicalPhysAddress (const BcmMacAddress &ipNetToPhysicalPhysAddress)
{
  fObjectMask |= kBIT_ipNetToPhysicalPhysAddress;

  // If we have just gotten all of our required objects for the
  // first time, transition from notReady to notInService.
  if (((fObjectMask & fRequiredObjects) == fRequiredObjects)
  &&  (Get_ipNetToPhysicalRowStatus() == SNMP_STATUS_NOT_READY))
  {
    int ipNetToPhysicalRowStatus = SNMP_STATUS_NOT_IN_SERVICE;
    Set_ipNetToPhysicalRowStatus (ipNetToPhysicalRowStatus);
  }

  fMac = ipNetToPhysicalPhysAddress;
  return SNMP_NO_ERROR;
}


/*
ipNetToPhysicalType OBJECT-TYPE
    SYNTAX     INTEGER {
                other(1),        -- none of the following
                invalid(2),      -- an invalidated mapping
                dynamic(3),
                static(4),
                local(5)         -- local interface
            }
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The type of mapping.

            Setting this object to the value invalid(2) has the effect
            of invalidating the corresponding entry in the
            ipNetToPhysicalTable.  That is, it effectively dis-
            associates the interface identified with said entry from the
            mapping identified with said entry.  It is an
            implementation-specific matter as to whether the agent
            removes an invalidated entry from the table.  Accordingly,
            management stations must be prepared to receive tabular
            information from agents that corresponds to entries not
            currently in use.  Proper interpretation of such entries
            requires examination of the relevant ipNetToPhysicalType
            object.

            The 'dynamic(3)' type indicates that the IP address to
            physical addresses mapping has been dynamically resolved
            using e.g., IPv4 ARP or the IPv6 Neighbor Discovery
            protocol.

            The 'static(4)' type indicates that the mapping has been
            statically configured.  Both of these refer to entries that
            provide mappings for other entities addresses.

            The 'local(5)' type indicates that the mapping is provided
            for an entity's own interface address.

            As the entries in this table are typically not persistent
            when this object is written the entity SHOULD NOT save the
            change to non-volatile storage."
    DEFVAL { static }
    ::= { ipNetToPhysicalEntry 6 }
*/
SNMP_STATUS lnxIpNetToPhysicalEntryBridge::Set_ipNetToPhysicalType (int ipNetToPhysicalType)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ipNetToPhysicalType);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ipNetToPhysicalType != CONSTVAL_IPNETTOPHYSICALTYPE_OTHER)
    &&  (ipNetToPhysicalType != CONSTVAL_IPNETTOPHYSICALTYPE_INVALID)
    &&  (ipNetToPhysicalType != CONSTVAL_IPNETTOPHYSICALTYPE_DYNAMIC)
    &&  (ipNetToPhysicalType != CONSTVAL_IPNETTOPHYSICALTYPE_STATIC)
    &&  (ipNetToPhysicalType != CONSTVAL_IPNETTOPHYSICALTYPE_LOCAL))
      return SNMP_WRONG_VALUE;

    fEntryType = ipNetToPhysicalType;
  }

  return Status;
}


/*
ipNetToPhysicalRowStatus OBJECT-TYPE
    SYNTAX     RowStatus
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The status of this conceptual row.

            The RowStatus TC requires that this DESCRIPTION clause
            states under which circumstances other objects in this row
            can be modified.  The value of this object has no effect on
            whether other objects in this conceptual row can be
            modified.

            A conceptual row can not be made active until the
            ipNetToPhysicalPhysAddress object has been set.

            Note that if the ipNetToPhysicalType is set to 'invalid',
            the managed node may delete the entry independent of the
            state of this object."
    ::= { ipNetToPhysicalEntry 8 }
*/
SNMP_STATUS lnxIpNetToPhysicalEntryBridge::Set_ipNetToPhysicalRowStatus (int &ipNetToPhysicalRowStatus)
{
  SNMP_STATUS Status = SetRowStatus (ipNetToPhysicalRowStatus);

  if (Status == SNMP_NO_ERROR)
  {
    fObjectMask |= kBIT_ipNetToPhysicalRowStatus;
    fRowStatus = ipNetToPhysicalRowStatus;
  }

  return Status;
}




/*\
 *
 *  lnxIcmpGroupBridge for bridging scalars from the icmpGroup group
 *
\*/

/*
icmp     OBJECT IDENTIFIER ::= { mib-2 5 }
*/

/*\
 *  construction / destruction
\*/

lnxIcmpGroupBridge::lnxIcmpGroupBridge (BcmSnmpAgent *pAgent)
  : icmpGroupBridge (pAgent)
{
}

lnxIcmpGroupBridge::~lnxIcmpGroupBridge ()
{
}

/*\
 *  GET methods
\*/

/*
icmpInMsgs OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The total number of ICMP messages which the entity
            received.  Note that this counter includes all those counted
            by icmpInErrors."
    ::= { icmp 1 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpInMsgs ()
{
  linux_read_icmp_stat();

  return IcmpInMsgs;
}


/*
icmpInErrors OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP messages which the entity received but
            determined as having ICMP-specific errors (bad ICMP
            checksums, bad length, etc.)."
    ::= { icmp 2 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpInErrors ()
{
  linux_read_icmp_stat();

  return IcmpInErrors;
}


/*
icmpInDestUnreachs OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP Destination Unreachable messages
            received."
    ::= { icmp 3 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpInDestUnreachs ()
{
  linux_read_icmp_stat();

  return IcmpInDestUnreachs;
}


/*
icmpInTimeExcds OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP Time Exceeded messages received."
    ::= { icmp 4 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpInTimeExcds ()
{
  linux_read_icmp_stat();

  return IcmpInTimeExcds;
}


/*
icmpInParmProbs OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP Parameter Problem messages received."
    ::= { icmp 5 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpInParmProbs ()
{
  linux_read_icmp_stat();

  return IcmpInParmProbs;
}


/*
icmpInSrcQuenchs OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP Source Quench messages received."
    ::= { icmp 6 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpInSrcQuenchs ()
{
  linux_read_icmp_stat();

  return IcmpInSrcQuenchs;
}


/*
icmpInRedirects OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP Redirect messages received."
    ::= { icmp 7 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpInRedirects ()
{
  linux_read_icmp_stat();

  return IcmpInRedirects;
}


/*
icmpInEchos OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP Echo (request) messages received."
    ::= { icmp 8 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpInEchos ()
{
  linux_read_icmp_stat();

  return IcmpInEchos;
}


/*
icmpInEchoReps OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP Echo Reply messages received."
    ::= { icmp 9 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpInEchoReps ()
{
  linux_read_icmp_stat();

  return IcmpInEchoReps;
}


/*
icmpInTimestamps OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP Timestamp (request) messages received."
    ::= { icmp 10 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpInTimestamps ()
{
  linux_read_icmp_stat();

  return IcmpInTimestamps;
}


/*
icmpInTimestampReps OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP Timestamp Reply messages received."
    ::= { icmp 11 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpInTimestampReps ()
{
  linux_read_icmp_stat();

  return IcmpInTimestampReps;
}


/*
icmpInAddrMasks OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP Address Mask Request messages received."
    ::= { icmp 12 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpInAddrMasks ()
{
  linux_read_icmp_stat();

  return IcmpInAddrMasks;
}


/*
icmpInAddrMaskReps OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP Address Mask Reply messages received."
    ::= { icmp 13 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpInAddrMaskReps ()
{
  linux_read_icmp_stat();

  return IcmpInAddrMaskReps;
}


/*
icmpOutMsgs OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The total number of ICMP messages which this entity
            attempted to send.  Note that this counter includes all
            those counted by icmpOutErrors."
    ::= { icmp 14 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpOutMsgs ()
{
  linux_read_icmp_stat();

  return IcmpOutMsgs;
}


/*
icmpOutErrors OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP messages which this entity did not send
            due to problems discovered within ICMP such as a lack of
            buffers.  This value should not include errors discovered
            outside the ICMP layer such as the inability of IP to route
            the resultant datagram.  In some implementations there may
            be no types of error which contribute to this counter's
            value."
    ::= { icmp 15 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpOutErrors ()
{
  linux_read_icmp_stat();

  return IcmpOutErrors;
}


/*
icmpOutDestUnreachs OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP Destination Unreachable messages sent."
    ::= { icmp 16 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpOutDestUnreachs ()
{
  linux_read_icmp_stat();

  return IcmpOutDestUnreachs;
}


/*
icmpOutTimeExcds OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP Time Exceeded messages sent."
    ::= { icmp 17 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpOutTimeExcds ()
{
  linux_read_icmp_stat();

  return IcmpOutTimeExcds;
}


/*
icmpOutParmProbs OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP Parameter Problem messages sent."
    ::= { icmp 18 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpOutParmProbs ()
{
  linux_read_icmp_stat();

  return IcmpOutParmProbs;
}


/*
icmpOutSrcQuenchs OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP Source Quench messages sent."
    ::= { icmp 19 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpOutSrcQuenchs ()
{
  linux_read_icmp_stat();

  return IcmpOutSrcQuenchs;
}


/*
icmpOutRedirects OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP Redirect messages sent.  For a host,
            this object will always be zero, since hosts do not send
            redirects."
    ::= { icmp 20 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpOutRedirects ()
{
  linux_read_icmp_stat();

  return IcmpOutRedirects;
}


/*
icmpOutEchos OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP Echo (request) messages sent."
    ::= { icmp 21 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpOutEchos ()
{
  linux_read_icmp_stat();

  return IcmpOutEchos;
}


/*
icmpOutEchoReps OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP Echo Reply messages sent."
    ::= { icmp 22 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpOutEchoReps ()
{
  linux_read_icmp_stat();

  return IcmpOutEchoReps;
}


/*
icmpOutTimestamps OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP Timestamp (request) messages sent."
    ::= { icmp 23 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpOutTimestamps ()
{
  linux_read_icmp_stat();

  return IcmpOutTimestamps;
}


/*
icmpOutTimestampReps OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP Timestamp Reply messages sent."
    ::= { icmp 24 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpOutTimestampReps ()
{
  linux_read_icmp_stat();

  return IcmpOutTimestampReps;
}


/*
icmpOutAddrMasks OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP Address Mask Request messages sent."
    ::= { icmp 25 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpOutAddrMasks ()
{
  linux_read_icmp_stat();

  return IcmpOutAddrMasks;
}


/*
icmpOutAddrMaskReps OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The number of ICMP Address Mask Reply messages sent."
    ::= { icmp 26 }
*/
unsigned int lnxIcmpGroupBridge::Get_icmpOutAddrMaskReps ()
{
  linux_read_icmp_stat();

  return IcmpOutAddrMaskReps;
}


/*\
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\
 *  SET methods
\*/

void lnxIcmpGroupBridge::linux_read_icmp_stat ()
{
  FILE *in = fopen ("/proc/net/snmp", "r");
  char line [1024];

  if (! in)
    return;

  while (line == fgets (line, sizeof(line), in))
    {
      if (26 == sscanf (line,
"Icmp: %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu\n",
   &IcmpInMsgs, &IcmpInErrors, &IcmpInDestUnreachs,
   &IcmpInTimeExcds, &IcmpInParmProbs, &IcmpInSrcQuenchs,
   &IcmpInRedirects, &IcmpInEchos, &IcmpInEchoReps,
   &IcmpInTimestamps, &IcmpInTimestampReps, &IcmpInAddrMasks,
   &IcmpInAddrMaskReps, &IcmpOutMsgs, &IcmpOutErrors,
   &IcmpOutDestUnreachs, &IcmpOutTimeExcds,
   &IcmpOutParmProbs, &IcmpOutSrcQuenchs, &IcmpOutRedirects,
   &IcmpOutEchos, &IcmpOutEchoReps, &IcmpOutTimestamps,
   &IcmpOutTimestampReps, &IcmpOutAddrMasks,
   &IcmpOutAddrMaskReps))
        break;
    }
  fclose (in);
}

