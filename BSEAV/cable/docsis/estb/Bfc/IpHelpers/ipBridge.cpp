//****************************************************************************
//
// Copyright (c) 2002-2009 Broadcom Corporation
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
//****************************************************************************
//    Description:                                                          
//                                                                          
//**************************************************************************
//    Revision History:                                                     
//                                                                          
//**************************************************************************
//    Filename: ipBridge.cpp
//    Creation Date: Thursday, April 19, 2007 at 16:18:15
//    Created by Broadcom BFC Mib Compiler BCMIBC.EXE version 3.6.0b
//    from input file ip-mib_rfc4293.mib
//
//**************************************************************************

#include "MibObjs.h"
#include "SnmpLog.h"
#include "ipBridge.h"

#include "HalIf.h"
#include "BcmSnmpAgent.h"
#include "BaseIpHalIf.h"

/******************************************************************************\

--
-- http://www.ietf.org/rfc/rfc4293.txt
--

IP-MIB DEFINITIONS ::= BEGIN

IMPORTS
    MODULE-IDENTITY, OBJECT-TYPE,
    Integer32, Counter32, IpAddress,
    mib-2, Unsigned32, Counter64,
    zeroDotZero                        FROM SNMPv2-SMI
    PhysAddress, TruthValue,
    TimeStamp, RowPointer,
    TEXTUAL-CONVENTION, TestAndIncr,
    RowStatus, StorageType             FROM SNMPv2-TC
    MODULE-COMPLIANCE, OBJECT-GROUP    FROM SNMPv2-CONF
    InetAddress, InetAddressType,
    InetAddressPrefixLength,
    InetVersion, InetZoneIndex         FROM INET-ADDRESS-MIB
    InterfaceIndex                     FROM IF-MIB;

ipMIB MODULE-IDENTITY
    LAST-UPDATED "200602020000Z"
    ORGANIZATION "IETF IPv6 MIB Revision Team"
    CONTACT-INFO
           "Editor:
            Shawn A. Routhier
            Interworking Labs
            108 Whispering Pines Dr. Suite 235
            Scotts Valley, CA 95066
            USA
            EMail: <sar@iwl.com>"
    DESCRIPTION
           "The MIB module for managing IP and ICMP implementations, but
            excluding their management of IP routes.

            Copyright (C) The Internet Society (2006).  This version of
            this MIB module is part of RFC 4293; see the RFC itself for
            full legal notices."

    REVISION      "200602020000Z"
    DESCRIPTION
           "The IP version neutral revision with added IPv6 objects for
            ND, default routers, and router advertisements.  As well as
            being the successor to RFC 2011, this MIB is also the
            successor to RFCs 2465 and 2466.  Published as RFC 4293."

    REVISION      "199411010000Z"
    DESCRIPTION
           "A separate MIB module (IP-MIB) for IP and ICMP management
            objects.  Published as RFC 2011."

    REVISION      "199103310000Z"
    DESCRIPTION
           "The initial revision of this MIB module was part of MIB-II,
            which was published as RFC 1213."
    ::= { mib-2 48}

--
-- The textual conventions we define and use in this MIB.
--

IpAddressOriginTC ::= TEXTUAL-CONVENTION
    STATUS     current
    DESCRIPTION
           "The origin of the address.

            manual(2) indicates that the address was manually configured
            to a specified address, e.g., by user configuration.

            dhcp(4) indicates an address that was assigned to this
            system by a DHCP server.

            linklayer(5) indicates an address created by IPv6 stateless
            auto-configuration.

            random(6) indicates an address chosen by the system at
            random, e.g., an IPv4 address within 169.254/16, or an RFC
            3041 privacy address."
    SYNTAX     INTEGER {
        other(1),
        manual(2),
        dhcp(4),
        linklayer(5),
        random(6)
    }

IpAddressStatusTC ::= TEXTUAL-CONVENTION
    STATUS     current
    DESCRIPTION
           "The status of an address.  Most of the states correspond to
            states from the IPv6 Stateless Address Autoconfiguration
            protocol.

            The preferred(1) state indicates that this is a valid
            address that can appear as the destination or source address
            of a packet.

            The deprecated(2) state indicates that this is a valid but
            deprecated address that should no longer be used as a source
            address in new communications, but packets addressed to such
            an address are processed as expected.

            The invalid(3) state indicates that this isn't a valid
            address and it shouldn't appear as the destination or source
            address of a packet.

            The inaccessible(4) state indicates that the address is not
            accessible because the interface to which this address is
            assigned is not operational.

            The unknown(5) state indicates that the status cannot be
            determined for some reason.

            The tentative(6) state indicates that the uniqueness of the
            address on the link is being verified.  Addresses in this
            state should not be used for general communication and
            should only be used to determine the uniqueness of the
            address.

            The duplicate(7) state indicates the address has been
            determined to be non-unique on the link and so must not be
            used.

            The optimistic(8) state indicates the address is available
            for use, subject to restrictions, while its uniqueness on
            a link is being verified.

            In the absence of other information, an IPv4 address is
            always preferred(1)."
    REFERENCE "RFC 2462"
    SYNTAX     INTEGER {
        preferred(1),
        deprecated(2),
        invalid(3),
        inaccessible(4),
        unknown(5),
        tentative(6),
        duplicate(7),
        optimistic(8)
    }

IpAddressPrefixOriginTC ::= TEXTUAL-CONVENTION
    STATUS     current
    DESCRIPTION
           "The origin of this prefix.

            manual(2) indicates a prefix that was manually configured.

            wellknown(3) indicates a well-known prefix, e.g., 169.254/16
            for IPv4 auto-configuration or fe80::/10 for IPv6 link-local
            addresses.  Well known prefixes may be assigned by IANA,
            the address registries, or by specification in a standards
            track RFC.

            dhcp(4) indicates a prefix that was assigned by a DHCP
            server.

            routeradv(5) indicates a prefix learned from a router
            advertisement.

            Note: while IpAddressOriginTC and IpAddressPrefixOriginTC
            are similar, they are not identical.  The first defines how
            an address was created, while the second defines how a
            prefix was found."
    SYNTAX     INTEGER {
        other(1),
        manual(2),
        wellknown(3),
        dhcp(4),
        routeradv(5)
    }

Ipv6AddressIfIdentifierTC ::= TEXTUAL-CONVENTION
     DISPLAY-HINT "2x:"
     STATUS       current
     DESCRIPTION
       "This data type is used to model IPv6 address
       interface identifiers.  This is a binary string
       of up to 8 octets in network byte-order."
     SYNTAX      OCTET STRING (SIZE (0..8))

--
-- the IP general group
-- some objects that affect all of IPv4
--

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

ipGroupBridge::ipGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_ipGroup, pAgent)
{
}

ipGroupBridge::~ipGroupBridge ()
{
}


/*\ 
 *  GET methods
\*/

/* 
ipForwarding OBJECT-TYPE
    SYNTAX     INTEGER {
                    forwarding(1),    -- acting as a router
                    notForwarding(2)  -- NOT acting as a router
               }
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "The indication of whether this entity is acting as an IPv4
            router in respect to the forwarding of datagrams received
            by, but not addressed to, this entity.  IPv4 routers forward
            datagrams.  IPv4 hosts do not (except those source-routed
            via the host).

            When this object is written, the entity should save the
            change to non-volatile storage and restore the object from
            non-volatile storage upon re-initialization of the system.
            Note: a stronger requirement is not used because this object
            was previously defined."
    ::= { ip 1 }
*/ 
int ipGroupBridge::Get_ipForwarding ()
{
  // Base class says we don't forward, since we're a bridge, not a router.  
  // Derived classes may override.
  return CONSTVAL_IPFORWARDING_NOTFORWARDING;
}


/* 
ipDefaultTTL OBJECT-TYPE
    SYNTAX     Integer32 (1..255)
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "The default value inserted into the Time-To-Live field of
            the IPv4 header of datagrams originated at this entity,
            whenever a TTL value is not supplied by the transport layer
            protocol.

            When this object is written, the entity should save the
            change to non-volatile storage and restore the object from
            non-volatile storage upon re-initialization of the system.
            Note: a stronger requirement is not used because this object
            was previously defined."
    ::= { ip 2 }
*/ 
int ipGroupBridge::Get_ipDefaultTTL ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipDefaultTTL" << endl;
  return 0;
}


/* 
ipReasmTimeout OBJECT-TYPE
    SYNTAX     Integer32
    UNITS      "seconds"
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The maximum number of seconds that received fragments are
            held while they are awaiting reassembly at this entity."
    ::= { ip 13 }
*/ 
int ipGroupBridge::Get_ipReasmTimeout ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipReasmTimeout" << endl;
  return 0;
}

#if (BCM_IPV6_SUPPORT)

/* 
ipv6IpForwarding OBJECT-TYPE
    SYNTAX     INTEGER {
                    forwarding(1),    -- acting as a router
                    notForwarding(2)  -- NOT acting as a router
               }
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "The indication of whether this entity is acting as an IPv6
            router on any interface in respect to the forwarding of
            datagrams received by, but not addressed to, this entity.
            IPv6 routers forward datagrams.  IPv6 hosts do not (except
            those source-routed via the host).

            When this object is written, the entity SHOULD save the
            change to non-volatile storage and restore the object from
            non-volatile storage upon re-initialization of the system."
    ::= { ip 25 }
*/ 
int ipGroupBridge::Get_ipv6IpForwarding ()
{
  // Return the same thing as ipForwarding  
  return Get_ipForwarding();
}


/* 
ipv6IpDefaultHopLimit OBJECT-TYPE
    SYNTAX     Integer32 (0..255)
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "The default value inserted into the Hop Limit field of the
            IPv6 header of datagrams originated at this entity whenever
            a Hop Limit value is not supplied by the transport layer
            protocol.

            When this object is written, the entity SHOULD save the
            change to non-volatile storage and restore the object from
            non-volatile storage upon re-initialization of the system."
    REFERENCE "RFC 2461 Section 6.3.2"
    ::= { ip 26 }
*/ 
int ipGroupBridge::Get_ipv6IpDefaultHopLimit ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6IpDefaultHopLimit" << endl;
  return 0;
}

#endif

/* 
ipv4InterfaceTableLastChange OBJECT-TYPE
    SYNTAX     TimeStamp
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The value of sysUpTime on the most recent occasion at which
            a row in the ipv4InterfaceTable was added or deleted, or
            when an ipv4InterfaceReasmMaxSize or an
            ipv4InterfaceEnableStatus object was modified.

            If new objects are added to the ipv4InterfaceTable that
            require the ipv4InterfaceTableLastChange to be updated when
            they are modified, they must specify that requirement in
            their description clause."
    ::= { ip 27 }
*/ 
unsigned long ipGroupBridge::Get_ipv4InterfaceTableLastChange ()
{
  // We don't support changes to this table so always return 0.
  return 0;
}

#if (BCM_IPV6_SUPPORT)

/* 
ipv6InterfaceTableLastChange OBJECT-TYPE
    SYNTAX     TimeStamp
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The value of sysUpTime on the most recent occasion at which
            a row in the ipv6InterfaceTable was added or deleted or when
            an ipv6InterfaceReasmMaxSize, ipv6InterfaceIdentifier,
            ipv6InterfaceEnableStatus, ipv6InterfaceReachableTime,
            ipv6InterfaceRetransmitTime, or ipv6InterfaceForwarding
            object was modified.

            If new objects are added to the ipv6InterfaceTable that
            require the ipv6InterfaceTableLastChange to be updated when
            they are modified, they must specify that requirement in
            their description clause."
    ::= { ip 29 }
*/ 
unsigned long ipGroupBridge::Get_ipv6InterfaceTableLastChange ()
{
  // We don't support changes to this table so always return 0.
  return 0;
}

#endif

/* 
ipAddressSpinLock OBJECT-TYPE
    SYNTAX     TestAndIncr
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "An advisory lock used to allow cooperating SNMP managers to
            coordinate their use of the set operation in creating or
            modifying rows within this table.

            In order to use this lock to coordinate the use of set
            operations, managers should first retrieve
            ipAddressTableSpinLock.  They should then determine the
            appropriate row to create or modify.  Finally, they should
            issue the appropriate set command, including the retrieved
            value of ipAddressSpinLock.  If another manager has altered
            the table in the meantime, then the value of
            ipAddressSpinLock will have changed, and the creation will
            fail as it will be specifying an incorrect value for
            ipAddressSpinLock.  It is suggested, but not required, that
            the ipAddressSpinLock be the first var bind for each set of
            objects representing a 'row' in a PDU."
    ::= { ip 33 }
*/ 
/*
// NOTE: normally, TestAndIncr objects don't need bridge
//       object methods for get, test, or set; the MIB
//       object will just return SNMP_NO_OPINION and let the
//       TestAndIncrMibObject do its thing.  However, this
//       method is provided here in case you need to provide
//       non-standard custom behaviour.
int ipGroupBridge::Get_ipAddressSpinLock ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipAddressSpinLock" << endl;
  return 0;
}
*/


/* 
ipv6RouterAdvertSpinLock OBJECT-TYPE
    SYNTAX     TestAndIncr
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "An advisory lock used to allow cooperating SNMP managers to
            coordinate their use of the set operation in creating or
            modifying rows within this table.

            In order to use this lock to coordinate the use of set
            operations, managers should first retrieve
            ipv6RouterAdvertSpinLock.  They should then determine the
            appropriate row to create or modify.  Finally, they should
            issue the appropriate set command including the retrieved
            value of ipv6RouterAdvertSpinLock.  If another manager has
            altered the table in the meantime, then the value of
            ipv6RouterAdvertSpinLock will have changed and the creation
            will fail as it will be specifying an incorrect value for
            ipv6RouterAdvertSpinLock.  It is suggested, but not
            required, that the ipv6RouterAdvertSpinLock be the first var
            bind for each set of objects representing a 'row' in a PDU."
    ::= { ip 38 }
*/ 
/*
// NOTE: normally, TestAndIncr objects don't need bridge
//       object methods for get, test, or set; the MIB
//       object will just return SNMP_NO_OPINION and let the
//       TestAndIncrMibObject do its thing.  However, this
//       method is provided here in case you need to provide
//       non-standard custom behaviour.
int ipGroupBridge::Get_ipv6RouterAdvertSpinLock ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6RouterAdvertSpinLock" << endl;
  return 0;
}
*/


/* 
ipInReceives OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The total number of input datagrams received from
            interfaces, including those received in error.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by
            ipSystemStatsInRecieves."
    ::= { ip 3 }
*/ 
unsigned int ipGroupBridge::Get_ipInReceives ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipInReceives" << endl;
  return 0;
}


/* 
ipInHdrErrors OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of input datagrams discarded due to errors in
            their IPv4 headers, including bad checksums, version number
            mismatch, other format errors, time-to-live exceeded, errors
            discovered in processing their IPv4 options, etc.

            This object has been deprecated as a new IP version-neutral
            table has been added.  It is loosely replaced by
            ipSystemStatsInHdrErrors."
    ::= { ip 4 }
*/ 
unsigned int ipGroupBridge::Get_ipInHdrErrors ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipInHdrErrors" << endl;
  return 0;
}


/* 
ipInAddrErrors OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of input datagrams discarded because the IPv4
            address in their IPv4 header's destination field was not a
            valid address to be received at this entity.  This count
            includes invalid addresses (e.g., 0.0.0.0) and addresses of
            unsupported Classes (e.g., Class E).  For entities which are
            not IPv4 routers, and therefore do not forward datagrams,
            this counter includes datagrams discarded because the
            destination address was not a local address.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by
            ipSystemStatsInAddrErrors."
    ::= { ip 5 }
*/ 
unsigned int ipGroupBridge::Get_ipInAddrErrors ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipInAddrErrors" << endl;
  return 0;
}


/* 
ipForwDatagrams OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of input datagrams for which this entity was not
            their final IPv4 destination, as a result of which an
            attempt was made to find a route to forward them to that
            final destination.  In entities which do not act as IPv4
            routers, this counter will include only those packets which
            were Source-Routed via this entity, and the Source-Route
            option processing was successful.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by
            ipSystemStatsInForwDatagrams."
    ::= { ip 6 }
*/ 
unsigned int ipGroupBridge::Get_ipForwDatagrams ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipForwDatagrams" << endl;
  return 0;
}


/* 
ipInUnknownProtos OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of locally-addressed datagrams received
            successfully but discarded because of an unknown or
            unsupported protocol.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by
            ipSystemStatsInUnknownProtos."
    ::= { ip 7 }
*/ 
unsigned int ipGroupBridge::Get_ipInUnknownProtos ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipInUnknownProtos" << endl;
  return 0;
}


/* 
ipInDiscards OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of input IPv4 datagrams for which no problems
            were encountered to prevent their continued processing, but
            which were discarded (e.g., for lack of buffer space).  Note
            that this counter does not include any datagrams discarded
            while awaiting re-assembly.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by
            ipSystemStatsInDiscards."
    ::= { ip 8 }
*/ 
unsigned int ipGroupBridge::Get_ipInDiscards ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipInDiscards" << endl;
  return 0;
}


/* 
ipInDelivers OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The total number of input datagrams successfully delivered
            to IPv4 user-protocols (including ICMP).

            This object has been deprecated as a new IP version neutral
            table has been added.  It is loosely replaced by
            ipSystemStatsIndelivers."
    ::= { ip 9 }
*/ 
unsigned int ipGroupBridge::Get_ipInDelivers ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipInDelivers" << endl;
  return 0;
}


/* 
ipOutRequests OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The total number of IPv4 datagrams which local IPv4 user
            protocols (including ICMP) supplied to IPv4 in requests for
            transmission.  Note that this counter does not include any
            datagrams counted in ipForwDatagrams.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by
            ipSystemStatsOutRequests."
    ::= { ip 10 }
*/ 
unsigned int ipGroupBridge::Get_ipOutRequests ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipOutRequests" << endl;
  return 0;
}


/* 
ipOutDiscards OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of output IPv4 datagrams for which no problem was
            encountered to prevent their transmission to their
            destination, but which were discarded (e.g., for lack of
            buffer space).  Note that this counter would include
            datagrams counted in ipForwDatagrams if any such packets met
            this (discretionary) discard criterion.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by
            ipSystemStatsOutDiscards."
    ::= { ip 11 }
*/ 
unsigned int ipGroupBridge::Get_ipOutDiscards ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipOutDiscards" << endl;
  return 0;
}


/* 
ipOutNoRoutes OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of IPv4 datagrams discarded because no route
            could be found to transmit them to their destination.  Note
            that this counter includes any packets counted in
            ipForwDatagrams which meet this `no-route' criterion.  Note
            that this includes any datagrams which a host cannot route
            because all of its default routers are down.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by
            ipSystemStatsOutNoRoutes."
    ::= { ip 12 }
*/ 
unsigned int ipGroupBridge::Get_ipOutNoRoutes ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipOutNoRoutes" << endl;
  return 0;
}


/* 
ipReasmReqds OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of IPv4 fragments received which needed to be
            reassembled at this entity.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by
            ipSystemStatsReasmReqds."
    ::= { ip 14 }
*/ 
unsigned int ipGroupBridge::Get_ipReasmReqds ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipReasmReqds" << endl;
  return 0;
}


/* 
ipReasmOKs OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of IPv4 datagrams successfully re-assembled.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by
            ipSystemStatsReasmOKs."
    ::= { ip 15 }
*/ 
unsigned int ipGroupBridge::Get_ipReasmOKs ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipReasmOKs" << endl;
  return 0;
}


/* 
ipReasmFails OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of failures detected by the IPv4 re-assembly
            algorithm (for whatever reason: timed out, errors, etc).
            Note that this is not necessarily a count of discarded IPv4
            fragments since some algorithms (notably the algorithm in
            RFC 815) can lose track of the number of fragments by
            combining them as they are received.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by
            ipSystemStatsReasmFails."
    ::= { ip 16 }
*/ 
unsigned int ipGroupBridge::Get_ipReasmFails ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipReasmFails" << endl;
  return 0;
}


/* 
ipFragOKs OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of IPv4 datagrams that have been successfully
            fragmented at this entity.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by
            ipSystemStatsOutFragOKs."
    ::= { ip 17 }
*/ 
unsigned int ipGroupBridge::Get_ipFragOKs ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipFragOKs" << endl;
  return 0;
}


/* 
ipFragFails OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of IPv4 datagrams that have been discarded
            because they needed to be fragmented at this entity but
            could not be, e.g., because their Don't Fragment flag was
            set.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by
            ipSystemStatsOutFragFails."
    ::= { ip 18 }
*/ 
unsigned int ipGroupBridge::Get_ipFragFails ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipFragFails" << endl;
  return 0;
}


/* 
ipFragCreates OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of IPv4 datagram fragments that have been
            generated as a result of fragmentation at this entity.

            This object has been deprecated as a new IP version neutral
            table has been added.  It is loosely replaced by
            ipSystemStatsOutFragCreates."
    ::= { ip 19 }
*/ 
unsigned int ipGroupBridge::Get_ipFragCreates ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipFragCreates" << endl;
  return 0;
}


/* 
ipRoutingDiscards OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of routing entries which were chosen to be
            discarded even though they are valid.  One possible reason
            for discarding such an entry could be to free-up buffer
            space for other routing entries.
            This object was defined in pre-IPv6 versions of the IP MIB.
            It was implicitly IPv4 only, but the original specifications
            did not indicate this protocol restriction.  In order to
            clarify the specifications, this object has been deprecated
            and a similar, but more thoroughly clarified, object has
            been added to the IP-FORWARD-MIB."
    ::= { ip 23 }
*/ 
unsigned int ipGroupBridge::Get_ipRoutingDiscards ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipRoutingDiscards" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/

/* 
ipForwarding OBJECT-TYPE
    SYNTAX     INTEGER {
                    forwarding(1),    -- acting as a router
                    notForwarding(2)  -- NOT acting as a router
               }
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "The indication of whether this entity is acting as an IPv4
            router in respect to the forwarding of datagrams received
            by, but not addressed to, this entity.  IPv4 routers forward
            datagrams.  IPv4 hosts do not (except those source-routed
            via the host).

            When this object is written, the entity should save the
            change to non-volatile storage and restore the object from
            non-volatile storage upon re-initialization of the system.
            Note: a stronger requirement is not used because this object
            was previously defined."
    ::= { ip 1 }
*/ 
SNMP_STATUS ipGroupBridge::Set_ipForwarding (int ipForwarding)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ipForwarding);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ipForwarding != CONSTVAL_IPFORWARDING_FORWARDING)
    &&  (ipForwarding != CONSTVAL_IPFORWARDING_NOTFORWARDING))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ipForwarding" << endl;
  }

  return Status;
}


/* 
ipDefaultTTL OBJECT-TYPE
    SYNTAX     Integer32 (1..255)
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "The default value inserted into the Time-To-Live field of
            the IPv4 header of datagrams originated at this entity,
            whenever a TTL value is not supplied by the transport layer
            protocol.

            When this object is written, the entity should save the
            change to non-volatile storage and restore the object from
            non-volatile storage upon re-initialization of the system.
            Note: a stronger requirement is not used because this object
            was previously defined."
    ::= { ip 2 }
*/ 
SNMP_STATUS ipGroupBridge::Set_ipDefaultTTL (int ipDefaultTTL)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ipDefaultTTL);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ipDefaultTTL < MINVAL_IPDEFAULTTTL)
    ||  (ipDefaultTTL > MAXVAL_IPDEFAULTTTL))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ipDefaultTTL" << endl;
  }

  return Status;
}

#if (BCM_IPV6_SUPPORT)

/* 
ipv6IpForwarding OBJECT-TYPE
    SYNTAX     INTEGER {
                    forwarding(1),    -- acting as a router
                    notForwarding(2)  -- NOT acting as a router
               }
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "The indication of whether this entity is acting as an IPv6
            router on any interface in respect to the forwarding of
            datagrams received by, but not addressed to, this entity.
            IPv6 routers forward datagrams.  IPv6 hosts do not (except
            those source-routed via the host).

            When this object is written, the entity SHOULD save the
            change to non-volatile storage and restore the object from
            non-volatile storage upon re-initialization of the system."
    ::= { ip 25 }
*/ 
SNMP_STATUS ipGroupBridge::Set_ipv6IpForwarding (int ipv6IpForwarding)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ipv6IpForwarding);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ipv6IpForwarding != CONSTVAL_IPV6IPFORWARDING_FORWARDING)
    &&  (ipv6IpForwarding != CONSTVAL_IPV6IPFORWARDING_NOTFORWARDING))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ipv6IpForwarding" << endl;
  }

  return Status;
}


/* 
ipv6IpDefaultHopLimit OBJECT-TYPE
    SYNTAX     Integer32 (0..255)
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "The default value inserted into the Hop Limit field of the
            IPv6 header of datagrams originated at this entity whenever
            a Hop Limit value is not supplied by the transport layer
            protocol.

            When this object is written, the entity SHOULD save the
            change to non-volatile storage and restore the object from
            non-volatile storage upon re-initialization of the system."
    REFERENCE "RFC 2461 Section 6.3.2"
    ::= { ip 26 }
*/ 
SNMP_STATUS ipGroupBridge::Set_ipv6IpDefaultHopLimit (int ipv6IpDefaultHopLimit)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ipv6IpDefaultHopLimit);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ipv6IpDefaultHopLimit < MINVAL_IPV6IPDEFAULTHOPLIMIT)
    ||  (ipv6IpDefaultHopLimit > MAXVAL_IPV6IPDEFAULTHOPLIMIT))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ipv6IpDefaultHopLimit" << endl;
  }

  return Status;
}

#endif

/* 
ipAddressSpinLock OBJECT-TYPE
    SYNTAX     TestAndIncr
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "An advisory lock used to allow cooperating SNMP managers to
            coordinate their use of the set operation in creating or
            modifying rows within this table.

            In order to use this lock to coordinate the use of set
            operations, managers should first retrieve
            ipAddressTableSpinLock.  They should then determine the
            appropriate row to create or modify.  Finally, they should
            issue the appropriate set command, including the retrieved
            value of ipAddressSpinLock.  If another manager has altered
            the table in the meantime, then the value of
            ipAddressSpinLock will have changed, and the creation will
            fail as it will be specifying an incorrect value for
            ipAddressSpinLock.  It is suggested, but not required, that
            the ipAddressSpinLock be the first var bind for each set of
            objects representing a 'row' in a PDU."
    ::= { ip 33 }
*/ 
// NOTE: normally, TestAndIncr objects don't need bridge
//       object methods for get, test, or set; the MIB
//       object will just return SNMP_NO_OPINION and let the
//       TestAndIncrMibObject do its thing.  However, this
//       method is provided here in case you need to provide
//       non-standard custom behaviour.
/*
SNMP_STATUS ipGroupBridge::Set_ipAddressSpinLock (int ipAddressSpinLock)
{
  SNMP_STATUS Status = TestAndIncrMibObject::TestDefaults (ipAddressSpinLock);

  if (Status == SNMP_NO_ERROR)
  {
    // UNFINISHED: no constraints specified in MIB for 
    // ipAddressSpinLock. Check if necessary.

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ipAddressSpinLock" << endl;
  }

  return Status;
}
*/


/* 
ipv6RouterAdvertSpinLock OBJECT-TYPE
    SYNTAX     TestAndIncr
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "An advisory lock used to allow cooperating SNMP managers to
            coordinate their use of the set operation in creating or
            modifying rows within this table.

            In order to use this lock to coordinate the use of set
            operations, managers should first retrieve
            ipv6RouterAdvertSpinLock.  They should then determine the
            appropriate row to create or modify.  Finally, they should
            issue the appropriate set command including the retrieved
            value of ipv6RouterAdvertSpinLock.  If another manager has
            altered the table in the meantime, then the value of
            ipv6RouterAdvertSpinLock will have changed and the creation
            will fail as it will be specifying an incorrect value for
            ipv6RouterAdvertSpinLock.  It is suggested, but not
            required, that the ipv6RouterAdvertSpinLock be the first var
            bind for each set of objects representing a 'row' in a PDU."
    ::= { ip 38 }
*/ 
// NOTE: normally, TestAndIncr objects don't need bridge
//       object methods for get, test, or set; the MIB
//       object will just return SNMP_NO_OPINION and let the
//       TestAndIncrMibObject do its thing.  However, this
//       method is provided here in case you need to provide
//       non-standard custom behaviour.
/*
SNMP_STATUS ipGroupBridge::Set_ipv6RouterAdvertSpinLock (int ipv6RouterAdvertSpinLock)
{
  SNMP_STATUS Status = TestAndIncrMibObject::TestDefaults (ipv6RouterAdvertSpinLock);

  if (Status == SNMP_NO_ERROR)
  {
    // UNFINISHED: no constraints specified in MIB for 
    // ipv6RouterAdvertSpinLock. Check if necessary.

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ipv6RouterAdvertSpinLock" << endl;
  }

  return Status;
}
*/






/*\
 *
 *  ipv4InterfaceEntryBridge for bridging elements in the ipv4InterfaceTable
 *
\*/

/* 
ipv4InterfaceTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF Ipv4InterfaceEntry
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The table containing per-interface IPv4-specific
            information."
    ::= { ip 28 }

INDEX { ipv4InterfaceIfIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ipv4InterfaceEntryBridge::ipv4InterfaceEntryBridge (int ipv4InterfaceIfIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ipv4InterfaceTable,
                      BcmObjectId(ipv4InterfaceIfIndex),
                      pAgent)
{
}

ipv4InterfaceEntryBridge::~ipv4InterfaceEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ipv4InterfaceIfIndex OBJECT-TYPE
    SYNTAX     InterfaceIndex
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The index value that uniquely identifies the interface to
            which this entry is applicable.  The interface identified by
            a particular value of this index is the same interface as
            identified by the same value of the IF-MIB's ifIndex."
    ::= { ipv4InterfaceEntry 1 }
*/ 
int ipv4InterfaceEntryBridge::Get_ipv4InterfaceIfIndex ()
{
  BcmObjectId Index;
  GetIndex(Index);
  return Index[0];
}


/* 
ipv4InterfaceReasmMaxSize OBJECT-TYPE
    SYNTAX     Integer32 (0..65535)
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The size of the largest IPv4 datagram that this entity can
            re-assemble from incoming IPv4 fragmented datagrams received
            on this interface."
    ::= { ipv4InterfaceEntry 2 }
*/ 
int ipv4InterfaceEntryBridge::Get_ipv4InterfaceReasmMaxSize ()
{
  // In the ipAddrTable this is what we return (see ipAddrTable::Update) so I 
  // guess it's correct here as well...
  return 65535;
}


/* 
ipv4InterfaceEnableStatus OBJECT-TYPE
    SYNTAX     INTEGER {
                 up(1),
                 down(2)
    }
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "The indication of whether IPv4 is enabled (up) or disabled
            (down) on this interface.  This object does not affect the
            state of the interface itself, only its connection to an
            IPv4 stack.  The IF-MIB should be used to control the state
            of the interface."
    ::= { ipv4InterfaceEntry 3 }
*/ 
int ipv4InterfaceEntryBridge::Get_ipv4InterfaceEnableStatus ()
{
  // We are always 'up'
  return CONSTVAL_IPV4INTERFACEENABLESTATUS_UP;
}


/* 
ipv4InterfaceRetransmitTime OBJECT-TYPE
    SYNTAX     Unsigned32
    UNITS      "milliseconds"
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The time between retransmissions of ARP requests to a
            neighbor when resolving the address or when probing the
            reachability of a neighbor."
    REFERENCE "RFC 1122"
    DEFVAL { 1000 }
    ::= { ipv4InterfaceEntry 4 }
*/ 
unsigned int ipv4InterfaceEntryBridge::Get_ipv4InterfaceRetransmitTime ()
{
  // Return the default.
  return 1000;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/

/* 
ipv4InterfaceEnableStatus OBJECT-TYPE
    SYNTAX     INTEGER {
                 up(1),
                 down(2)
    }
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "The indication of whether IPv4 is enabled (up) or disabled
            (down) on this interface.  This object does not affect the
            state of the interface itself, only its connection to an
            IPv4 stack.  The IF-MIB should be used to control the state
            of the interface."
    ::= { ipv4InterfaceEntry 3 }
*/ 
SNMP_STATUS ipv4InterfaceEntryBridge::Set_ipv4InterfaceEnableStatus (int ipv4InterfaceEnableStatus)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ipv4InterfaceEnableStatus);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ipv4InterfaceEnableStatus != CONSTVAL_IPV4INTERFACEENABLESTATUS_UP)
    &&  (ipv4InterfaceEnableStatus != CONSTVAL_IPV4INTERFACEENABLESTATUS_DOWN))
      return SNMP_WRONG_VALUE;

    // We don't support turning IPv4 on or off via this mechanism.
    if (ipv4InterfaceEnableStatus != Get_ipv4InterfaceEnableStatus())
      return SNMP_INCONSISTENT_VALUE;
  }

  return Status;
}




#if (BCM_IPV6_SUPPORT)

#include "NeighborDiscoveryThread.h"

/*\
 *
 *  ipv6InterfaceEntryBridge for bridging elements in the ipv6InterfaceTable
 *
\*/

/* 
ipv6InterfaceTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF Ipv6InterfaceEntry
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The table containing per-interface IPv6-specific
            information."
    ::= { ip 30 }

INDEX { ipv6InterfaceIfIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ipv6InterfaceEntryBridge::ipv6InterfaceEntryBridge (int ipv6InterfaceIfIndex, BcmSnmpAgent *pAgent, BcmNeighborDiscoveryThread *pNDThd, const BcmString &ipv6InterfaceIdentifier)
  : TabularMibBridge (kOID_ipv6InterfaceTable,
                      BcmObjectId(ipv6InterfaceIfIndex),
                      pAgent)
{
  fpNDThd = pNDThd;
  fInterfaceIdentifier = ipv6InterfaceIdentifier;
}

ipv6InterfaceEntryBridge::~ipv6InterfaceEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ipv6InterfaceIfIndex OBJECT-TYPE
    SYNTAX     InterfaceIndex
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The index value that uniquely identifies the interface to
            which this entry is applicable.  The interface identified by
            a particular value of this index is the same interface as
            identified by the same value of the IF-MIB's ifIndex."
    ::= { ipv6InterfaceEntry 1 }
*/ 
int ipv6InterfaceEntryBridge::Get_ipv6InterfaceIfIndex ()
{
  BcmObjectId Index;
  GetIndex(Index);
  return Index[0];
}


/* 
ipv6InterfaceReasmMaxSize OBJECT-TYPE
    SYNTAX     Unsigned32 (1500..65535)
    UNITS      "octets"
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The size of the largest IPv6 datagram that this entity can
            re-assemble from incoming IPv6 fragmented datagrams received
            on this interface."
    ::= { ipv6InterfaceEntry 2 }
*/ 
unsigned int ipv6InterfaceEntryBridge::Get_ipv6InterfaceReasmMaxSize ()
{
  // In the ipAddrTable this is what we return (see ipAddrTable::Update) so I 
  // guess it's correct here as well...
  return 65535;
}


/* 
ipv6InterfaceIdentifier OBJECT-TYPE
    SYNTAX     Ipv6AddressIfIdentifierTC
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The Interface Identifier for this interface.  The Interface
            Identifier is combined with an address prefix to form an
            interface address.

            By default, the Interface Identifier is auto-configured
            according to the rules of the link type to which this
            interface is attached.

            A zero length identifier may be used where appropriate.  One
            possible example is a loopback interface."
    ::= { ipv6InterfaceEntry 3 }
*/ 
int ipv6InterfaceEntryBridge::Get_ipv6InterfaceIdentifier (BcmString &ipv6InterfaceIdentifier)
{
  // Required to return the EUI-64 value derived from the MAC address of this
  // interface.  This gets passed in to the constructor.
  ipv6InterfaceIdentifier = fInterfaceIdentifier;
  return ipv6InterfaceIdentifier.length();
}


/* 
ipv6InterfaceEnableStatus OBJECT-TYPE
    SYNTAX     INTEGER {
                 up(1),
                 down(2)
    }
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "The indication of whether IPv6 is enabled (up) or disabled
            (down) on this interface.  This object does not affect the
            state of the interface itself, only its connection to an
            IPv6 stack.  The IF-MIB should be used to control the state
            of the interface.

            When this object is written, the entity SHOULD save the
            change to non-volatile storage and restore the object from
            non-volatile storage upon re-initialization of the system."
    ::= { ipv6InterfaceEntry 5 }
*/ 
int ipv6InterfaceEntryBridge::Get_ipv6InterfaceEnableStatus ()
{
  // We are always 'up'
  return CONSTVAL_IPV6INTERFACEENABLESTATUS_UP;
}


/* 
ipv6InterfaceReachableTime OBJECT-TYPE
    SYNTAX     Unsigned32
    UNITS      "milliseconds"
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The time a neighbor is considered reachable after receiving
            a reachability confirmation."
    REFERENCE "RFC 2461, Section 6.3.2"
    ::= { ipv6InterfaceEntry 6 }
*/ 
unsigned int ipv6InterfaceEntryBridge::Get_ipv6InterfaceReachableTime ()
{
  if (fpNDThd)
    return fpNDThd->DefaultRouterList().GetReachableTime();
    
  return 0;
}


/* 
ipv6InterfaceRetransmitTime OBJECT-TYPE
    SYNTAX     Unsigned32
    UNITS      "milliseconds"
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The time between retransmissions of Neighbor Solicitation
            messages to a neighbor when resolving the address or when
            probing the reachability of a neighbor."
    REFERENCE "RFC 2461, Section 6.3.2"
    ::= { ipv6InterfaceEntry 7 }
*/ 
unsigned int ipv6InterfaceEntryBridge::Get_ipv6InterfaceRetransmitTime ()
{
  // Default value
  unsigned int ipv6InterfaceRetransmitTime = RETRANS_TIMER_MS;

  if (fpNDThd)
  {
    ipv6InterfaceRetransmitTime = fpNDThd->DefaultRouterList().GetRetransTimer();
    
    // If we get a 0 back here then it means that this timeout was not
    // specified.  In that case we'll retry using a default value as
    // defined in NeighborCacheEntry.h.
    if (ipv6InterfaceRetransmitTime == 0)
      ipv6InterfaceRetransmitTime = RETRANS_TIMER_MS;
  }
    
  return ipv6InterfaceRetransmitTime;
}


/* 
ipv6InterfaceForwarding OBJECT-TYPE
    SYNTAX     INTEGER {
                    forwarding(1),    -- acting as a router
                    notForwarding(2)  -- NOT acting as a router
               }
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "The indication of whether this entity is acting as an IPv6
            router on this interface with respect to the forwarding of
            datagrams received by, but not addressed to, this entity.
            IPv6 routers forward datagrams.  IPv6 hosts do not (except
            those source-routed via the host).

            This object is constrained by ipv6IpForwarding and is
            ignored if ipv6IpForwarding is set to notForwarding.  Those
            systems that do not provide per-interface control of the
            forwarding function should set this object to forwarding for
            all interfaces and allow the ipv6IpForwarding object to
            control the forwarding capability.

            When this object is written, the entity SHOULD save the
            change to non-volatile storage and restore the object from
            non-volatile storage upon re-initialization of the system."
    ::= { ipv6InterfaceEntry 8 }
*/ 
int ipv6InterfaceEntryBridge::Get_ipv6InterfaceForwarding ()
{
  int ipv6InterfaceForwarding;

  // Return the same thing as ipForwarding  
  if (fpAgent->GetIntObject (BcmObjectId(kOID_ipForwarding), ipv6InterfaceForwarding) != SNMP_NO_ERROR)
    ipv6InterfaceForwarding = CONSTVAL_IPV6INTERFACEFORWARDING_NOTFORWARDING;

  return ipv6InterfaceForwarding;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/

/* 
ipv6InterfaceEnableStatus OBJECT-TYPE
    SYNTAX     INTEGER {
                 up(1),
                 down(2)
    }
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "The indication of whether IPv6 is enabled (up) or disabled
            (down) on this interface.  This object does not affect the
            state of the interface itself, only its connection to an
            IPv6 stack.  The IF-MIB should be used to control the state
            of the interface.

            When this object is written, the entity SHOULD save the
            change to non-volatile storage and restore the object from
            non-volatile storage upon re-initialization of the system."
    ::= { ipv6InterfaceEntry 5 }
*/ 
SNMP_STATUS ipv6InterfaceEntryBridge::Set_ipv6InterfaceEnableStatus (int ipv6InterfaceEnableStatus)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ipv6InterfaceEnableStatus);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ipv6InterfaceEnableStatus != CONSTVAL_IPV6INTERFACEENABLESTATUS_UP)
    &&  (ipv6InterfaceEnableStatus != CONSTVAL_IPV6INTERFACEENABLESTATUS_DOWN))
      return SNMP_WRONG_VALUE;

    // We don't support turning IPv6 on or off via this mechanism.
    if (ipv6InterfaceEnableStatus != Get_ipv6InterfaceEnableStatus())
      return SNMP_INCONSISTENT_VALUE;
  }

  return Status;
}


/* 
ipv6InterfaceForwarding OBJECT-TYPE
    SYNTAX     INTEGER {
                    forwarding(1),    -- acting as a router
                    notForwarding(2)  -- NOT acting as a router
               }
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "The indication of whether this entity is acting as an IPv6
            router on this interface with respect to the forwarding of
            datagrams received by, but not addressed to, this entity.
            IPv6 routers forward datagrams.  IPv6 hosts do not (except
            those source-routed via the host).

            This object is constrained by ipv6IpForwarding and is
            ignored if ipv6IpForwarding is set to notForwarding.  Those
            systems that do not provide per-interface control of the
            forwarding function should set this object to forwarding for
            all interfaces and allow the ipv6IpForwarding object to
            control the forwarding capability.

            When this object is written, the entity SHOULD save the
            change to non-volatile storage and restore the object from
            non-volatile storage upon re-initialization of the system."
    ::= { ipv6InterfaceEntry 8 }
*/ 
SNMP_STATUS ipv6InterfaceEntryBridge::Set_ipv6InterfaceForwarding (int ipv6InterfaceForwarding)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ipv6InterfaceForwarding);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ipv6InterfaceForwarding != CONSTVAL_IPV6INTERFACEFORWARDING_FORWARDING)
    &&  (ipv6InterfaceForwarding != CONSTVAL_IPV6INTERFACEFORWARDING_NOTFORWARDING))
      return SNMP_WRONG_VALUE;

    // We don't support turning IPv6 forwarding on or off via this mechanism.
    if (ipv6InterfaceForwarding != Get_ipv6InterfaceForwarding())
      return SNMP_INCONSISTENT_VALUE;
  }

  return Status;
}

#endif


#if 0

/*\
 *
 *  ipAddressPrefixEntryBridge for bridging elements in the ipAddressPrefixTable
 *
\*/

/* 
ipAddressPrefixTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF IpAddressPrefixEntry
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "This table allows the user to determine the source of an IP
            address or set of IP addresses, and allows other tables to
            share the information via pointer rather than by copying.

            For example, when the node configures both a unicast and
            anycast address for a prefix, the ipAddressPrefix objects
            for those addresses will point to a single row in this
            table.

            This table primarily provides support for IPv6 prefixes, and
            several of the objects are less meaningful for IPv4.  The
            table continues to allow IPv4 addresses to allow future
            flexibility.  In order to promote a common configuration,
            this document includes suggestions for default values for
            IPv4 prefixes.  Each of these values may be overridden if an
            object is meaningful to the node.

            All prefixes used by this entity should be included in this
            table independent of how the entity learned the prefix.
            (This table isn't limited to prefixes learned from router
            advertisements.)"
    ::= { ip 32 }

INDEX { ipAddressPrefixIfIndex, ipAddressPrefixType, ipAddressPrefixPrefix, ipAddressPrefixLength }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ipAddressPrefixEntryBridge::ipAddressPrefixEntryBridge (int ipAddressPrefixIfIndex, int ipAddressPrefixType, const BcmString &ipAddressPrefixPrefix, int ipAddressPrefixLength, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ipAddressPrefixTable,
                      BcmObjectId(ipAddressPrefixIfIndex) + BcmObjectId(ipAddressPrefixType) + BcmObjectId(ipAddressPrefixPrefix, BcmObjectId::kAscii) + BcmObjectId(ipAddressPrefixLength),
                      pAgent)
{
}

ipAddressPrefixEntryBridge::~ipAddressPrefixEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ipAddressPrefixIfIndex OBJECT-TYPE
    SYNTAX     InterfaceIndex
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The index value that uniquely identifies the interface on
            which this prefix is configured.  The interface identified
            by a particular value of this index is the same interface as
            identified by the same value of the IF-MIB's ifIndex."
    ::= { ipAddressPrefixEntry 1 }
*/ 
int ipAddressPrefixEntryBridge::Get_ipAddressPrefixIfIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipAddressPrefixIfIndex" << endl;
  return 0;
}


/* 
ipAddressPrefixType OBJECT-TYPE
    SYNTAX     InetAddressType
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The address type of ipAddressPrefix."
    ::= { ipAddressPrefixEntry 2 }
*/ 
int ipAddressPrefixEntryBridge::Get_ipAddressPrefixType ()
{
  BcmIpAddress IpAddress;


  // UNFINISHED: assign meaningful value to IpAddress here!
  SnmpLogRaw << "UNFINISHED: Get_ipAddressPrefixType" << endl;

  #if (BCM_IPV6_SUPPORT)
  if (IpAddress.IsIpV6Address() == true)
  {
    if (IpAddress != kIPv6UnspecifiedAddress)
      return CONSTVAL_IPADDRESSPREFIXTYPE_IPV6;
    else
      return CONSTVAL_IPADDRESSPREFIXTYPE_UNKNOWN;
  }
  #endif

  return CONSTVAL_IPADDRESSPREFIXTYPE_IPV4;
}


/* 
ipAddressPrefixPrefix OBJECT-TYPE
    SYNTAX     InetAddress
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The address prefix.  The address type of this object is
            specified in ipAddressPrefixType.  The length of this object
            is the standard length for objects of that type (4 or 16
            bytes).  Any bits after ipAddressPrefixLength must be zero.

            Implementors need to be aware that, if the size of
            ipAddressPrefixPrefix exceeds 114 octets, then OIDS of
            instances of columns in this row will have more than 128
            sub-identifiers and cannot be accessed using SNMPv1,
            SNMPv2c, or SNMPv3."
    ::= { ipAddressPrefixEntry 3 }
*/ 
int ipAddressPrefixEntryBridge::Get_ipAddressPrefixPrefix (BcmString &ipAddressPrefixPrefix)
{
  BcmIpAddress IpAddress;


  // UNFINISHED: assign meaningful value to IpAddress here!
  SnmpLogRaw << "UNFINISHED: Get_ipAddressPrefixPrefix" << endl;

  ipAddressPrefixPrefix = IpAddress;

  return ipAddressPrefixPrefix.length();
}


/* 
ipAddressPrefixLength OBJECT-TYPE
    SYNTAX     InetAddressPrefixLength
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The prefix length associated with this prefix.

            The value 0 has no special meaning for this object.  It
            simply refers to address '::/0'."
    ::= { ipAddressPrefixEntry 4 }
*/ 
int ipAddressPrefixEntryBridge::Get_ipAddressPrefixLength ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipAddressPrefixLength" << endl;
  return 0;
}


/* 
ipAddressPrefixOrigin OBJECT-TYPE
    SYNTAX     IpAddressPrefixOriginTC
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The origin of this prefix."
    ::= { ipAddressPrefixEntry 5 }
*/ 
int ipAddressPrefixEntryBridge::Get_ipAddressPrefixOrigin ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipAddressPrefixOrigin" << endl;
  return 0;
}


/* 
ipAddressPrefixOnLinkFlag OBJECT-TYPE
    SYNTAX     TruthValue
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "This object has the value 'true(1)', if this prefix can be
            used for on-link determination; otherwise, the value is
            'false(2)'.

            The default for IPv4 prefixes is 'true(1)'."
    REFERENCE "For IPv6 RFC 2461, especially sections 2 and 4.6.2 and
               RFC 2462"
    ::= { ipAddressPrefixEntry 6 }
*/ 
bool ipAddressPrefixEntryBridge::Get_ipAddressPrefixOnLinkFlag ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipAddressPrefixOnLinkFlag" << endl;
  return true;
}


/* 
ipAddressPrefixAutonomousFlag OBJECT-TYPE
    SYNTAX     TruthValue
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "Autonomous address configuration flag.  When true(1),
            indicates that this prefix can be used for autonomous
            address configuration (i.e., can be used to form a local
            interface address).  If false(2), it is not used to auto-
            configure a local interface address.

            The default for IPv4 prefixes is 'false(2)'."
    REFERENCE "For IPv6 RFC 2461, especially sections 2 and 4.6.2 and
               RFC 2462"
    ::= { ipAddressPrefixEntry 7 }
*/ 
bool ipAddressPrefixEntryBridge::Get_ipAddressPrefixAutonomousFlag ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipAddressPrefixAutonomousFlag" << endl;
  return true;
}


/* 
ipAddressPrefixAdvPreferredLifetime OBJECT-TYPE
    SYNTAX     Unsigned32
    UNITS      "seconds"
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The remaining length of time, in seconds, that this prefix
            will continue to be preferred, i.e., time until deprecation.

            A value of 4,294,967,295 represents infinity.

            The address generated from a deprecated prefix should no
            longer be used as a source address in new communications,
            but packets received on such an interface are processed as
            expected.

            The default for IPv4 prefixes is 4,294,967,295 (infinity)."
    REFERENCE "For IPv6 RFC 2461, especially sections 2 and 4.6.2 and
               RFC 2462"
    ::= { ipAddressPrefixEntry 8 }
*/ 
unsigned int ipAddressPrefixEntryBridge::Get_ipAddressPrefixAdvPreferredLifetime ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipAddressPrefixAdvPreferredLifetime" << endl;
  return 0;
}


/* 
ipAddressPrefixAdvValidLifetime OBJECT-TYPE
    SYNTAX     Unsigned32
    UNITS       "seconds"
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The remaining length of time, in seconds, that this prefix
            will continue to be valid, i.e., time until invalidation.  A
            value of 4,294,967,295 represents infinity.

            The address generated from an invalidated prefix should not
            appear as the destination or source address of a packet.

            The default for IPv4 prefixes is 4,294,967,295 (infinity)."
    REFERENCE "For IPv6 RFC 2461, especially sections 2 and 4.6.2 and
               RFC 2462"
    ::= { ipAddressPrefixEntry 9 }
*/ 
unsigned int ipAddressPrefixEntryBridge::Get_ipAddressPrefixAdvValidLifetime ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipAddressPrefixAdvValidLifetime" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/

#endif



/*\
 *
 *  ipAddressEntryBridge for bridging elements in the ipAddressTable
 *
\*/

/* 
ipAddressTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF IpAddressEntry
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "This table contains addressing information relevant to the
            entity's interfaces.

            This table does not contain multicast address information.
            Tables for such information should be contained in multicast
            specific MIBs, such as RFC 3019.

            While this table is writable, the user will note that
            several objects, such as ipAddressOrigin, are not.  The
            intention in allowing a user to write to this table is to
            allow them to add or remove any entry that isn't
            permanent.  The user should be allowed to modify objects
            and entries when that would not cause inconsistencies
            within the table.  Allowing write access to objects, such
            as ipAddressOrigin, could allow a user to insert an entry
            and then label it incorrectly.

            Note well: When including IPv6 link-local addresses in this
            table, the entry must use an InetAddressType of 'ipv6z' in
            order to differentiate between the possible interfaces."
    ::= { ip 34 }

INDEX { ipAddressAddrType, ipAddressAddr }
*/ 

/*\ 
 *  construction / destruction
\*/ 

// This version of the constructor is called from ipAddressTable::Update
ipAddressEntryBridge::ipAddressEntryBridge (const BcmIpAddress &ipAddressAddr, int ipAddressIfIndex, int ipAddressType, const BcmObjectId &ipAddressPrefix, int ipAddressOrigin, int ipAddressStatus, unsigned long ipAddressCreated, unsigned long ipAddressLastChanged, unsigned long ipAddressStorageType, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ipAddressTable,
                      BcmObjectId(ipAddressAddr, true, true),
                      pAgent)
{
  fRowStatus = SNMP_STATUS_INVISIBLE;

  // Initialize required objects for row creation:
  fRequiredObjects |= kBIT_ipAddressIfIndex;
  
  // Set this via method to get the above bit set
  Set_ipAddressIfIndex (ipAddressIfIndex);

/*
  // Don't do this, per email from CableLabs 11/2008.  Use ipv6 for link-local.
    
  // From the ipAddressTable description:
  // "Note well: When including IPv6 link-local addresses in this table, 
  // the entry must use an InetAddressType of 'ipv6z' in order to 
  // differentiate between the possible interfaces."
  if (ipAddressAddr.IsIpV6Address() == true)
  {
    if (ipAddressAddr.GetIpV6Address().IsLinkLocalAddress() == true)
    {
      BcmObjectId Index;
      GetIndex(Index);
      Index[0] = CONSTVAL_IPADDRESSADDRTYPE_IPV6Z;
      SetIndex(Index); 
    }
  }
*/

  // Activate immediately.
  fObjectMask |= kBIT_ipAddressIfIndex;
  int Status = SNMP_STATUS_CREATE_AND_GO;
  
  Set_ipAddressRowStatus (Status);
  
  fIpAddressAddr = ipAddressAddr;
  fIpAddressType = ipAddressType;
  fIpAddressPrefix = ipAddressPrefix;
  fIpAddressOrigin = ipAddressOrigin;
  fIpAddressStatus = ipAddressStatus;
  fIpAddressCreated = ipAddressCreated;
  fIpAddressLastChanged = ipAddressLastChanged;
  fIpAddressStorageType = ipAddressStorageType;
}

ipAddressEntryBridge::~ipAddressEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ipAddressAddrType OBJECT-TYPE
    SYNTAX     InetAddressType
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The address type of ipAddressAddr."
    ::= { ipAddressEntry 1 }
*/ 
int ipAddressEntryBridge::Get_ipAddressAddrType ()
{
  BcmIpAddress IpAddress = fIpAddressAddr;


  #if (BCM_IPV6_SUPPORT)
  if (IpAddress.IsIpV6Address() == true)
  {
    if (IpAddress != kIPv6UnspecifiedAddress)
      return CONSTVAL_IPADDRESSADDRTYPE_IPV6;
    else
      return CONSTVAL_IPADDRESSADDRTYPE_UNKNOWN;
  }
  #endif

  return CONSTVAL_IPADDRESSADDRTYPE_IPV4;
}


/* 
ipAddressAddr OBJECT-TYPE
    SYNTAX     InetAddress
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The IP address to which this entry's addressing information
            pertains.  The address type of this object is specified in
            ipAddressAddrType.

            Implementors need to be aware that if the size of
            ipAddressAddr exceeds 116 octets, then OIDS of instances of
            columns in this row will have more than 128 sub-identifiers
            and cannot be accessed using SNMPv1, SNMPv2c, or SNMPv3."
    ::= { ipAddressEntry 2 }
*/ 
int ipAddressEntryBridge::Get_ipAddressAddr (BcmString &ipAddressAddr)
{
  if (Get_ipAddressAddrType() == CONSTVAL_IPADDRESSADDRTYPE_UNKNOWN)
    ipAddressAddr = "";
  else
    ipAddressAddr = fIpAddressAddr;

  return ipAddressAddr.length();
}


/* 
ipAddressIfIndex OBJECT-TYPE
    SYNTAX     InterfaceIndex
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The index value that uniquely identifies the interface to
            which this entry is applicable.  The interface identified by
            a particular value of this index is the same interface as
            identified by the same value of the IF-MIB's ifIndex."
    ::= { ipAddressEntry 3 }
*/ 
int ipAddressEntryBridge::Get_ipAddressIfIndex ()
{
  return fIpAddressIfIndex;
}


/* 
ipAddressType OBJECT-TYPE
    SYNTAX     INTEGER {
                 unicast(1),
                 anycast(2),
                 broadcast(3)
    }
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The type of address.  broadcast(3) is not a valid value for
            IPv6 addresses (RFC 3513)."
    DEFVAL { unicast }
    ::= { ipAddressEntry 4 }
*/ 
int ipAddressEntryBridge::Get_ipAddressType ()
{
  return fIpAddressType;
}


/* 
ipAddressPrefix OBJECT-TYPE
    SYNTAX     RowPointer
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "A pointer to the row in the prefix table to which this
            address belongs.  May be { 0 0 } if there is no such row."
    DEFVAL { zeroDotZero }
    ::= { ipAddressEntry 5 }
*/ 
int ipAddressEntryBridge::Get_ipAddressPrefix (BcmObjectId &ipAddressPrefix)
{
  ipAddressPrefix = fIpAddressPrefix;
  return ipAddressPrefix.Length();
}


/* 
ipAddressOrigin OBJECT-TYPE
    SYNTAX     IpAddressOriginTC
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The origin of the address."
    ::= { ipAddressEntry 6 }
*/ 
int ipAddressEntryBridge::Get_ipAddressOrigin ()
{
  return fIpAddressOrigin;
}


/* 
ipAddressStatus OBJECT-TYPE
    SYNTAX     IpAddressStatusTC
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The status of the address, describing if the address can be
            used for communication.

            In the absence of other information, an IPv4 address is
            always preferred(1)."
    DEFVAL { preferred }
    ::= { ipAddressEntry 7 }
*/ 
int ipAddressEntryBridge::Get_ipAddressStatus ()
{
  return fIpAddressStatus;
}


/* 
ipAddressCreated OBJECT-TYPE
    SYNTAX     TimeStamp
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The value of sysUpTime at the time this entry was created.
            If this entry was created prior to the last re-
            initialization of the local network management subsystem,
            then this object contains a zero value."
    ::= { ipAddressEntry 8 }
*/ 
unsigned long ipAddressEntryBridge::Get_ipAddressCreated ()
{
  return fIpAddressCreated;
}


/* 
ipAddressLastChanged OBJECT-TYPE
    SYNTAX     TimeStamp
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The value of sysUpTime at the time this entry was last
            updated.  If this entry was updated prior to the last re-
            initialization of the local network management subsystem,
            then this object contains a zero value."
    ::= { ipAddressEntry 9 }
*/ 
unsigned long ipAddressEntryBridge::Get_ipAddressLastChanged ()
{
  return fIpAddressLastChanged;
}


/* 
ipAddressRowStatus OBJECT-TYPE
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
            ipAddressIfIndex has been set to a valid index."
    ::= { ipAddressEntry 10 }
*/ 
int ipAddressEntryBridge::Get_ipAddressRowStatus ()
{
  return fRowStatus;
}


/* 
ipAddressStorageType OBJECT-TYPE
    SYNTAX     StorageType
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The storage type for this conceptual row.  If this object
            has a value of 'permanent', then no other objects are
            required to be able to be modified."
    DEFVAL { volatile }
    ::= { ipAddressEntry 11 }
*/ 
int ipAddressEntryBridge::Get_ipAddressStorageType ()
{
  return fIpAddressStorageType;
}


/*\ 
 *  TEST methods
\*/

/* 
ipAddressIfIndex OBJECT-TYPE
    SYNTAX     InterfaceIndex
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The index value that uniquely identifies the interface to
            which this entry is applicable.  The interface identified by
            a particular value of this index is the same interface as
            identified by the same value of the IF-MIB's ifIndex."
    ::= { ipAddressEntry 3 }
*/ 
SNMP_STATUS ipAddressEntryBridge::Test_ipAddressIfIndex (int ipAddressIfIndex)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ipAddressIfIndex);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ipAddressIfIndex < MINVAL_IPADDRESSIFINDEX)
    ||  (ipAddressIfIndex > MAXVAL_IPADDRESSIFINDEX))
      return SNMP_WRONG_VALUE;

    fObjectMask |= kBIT_ipAddressIfIndex;
  }

  return Status;
}

/* 
ipAddressRowStatus OBJECT-TYPE
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
            ipAddressIfIndex has been set to a valid index."
    ::= { ipAddressEntry 10 }
*/ 
SNMP_STATUS ipAddressEntryBridge::Test_ipAddressRowStatus (int ipAddressRowStatus)
{
  SNMP_STATUS Status = RowStatusMibObject::TestDefaults (ipAddressRowStatus, Get_ipAddressRowStatus());

  if (Status == SNMP_NO_ERROR)
  {
    // We don't support setting this object to anything other than
    // 'active'
    if (ipAddressRowStatus != SNMP_STATUS_ACTIVE)
      return SNMP_INCONSISTENT_VALUE;
  
    fObjectMask |= kBIT_ipAddressRowStatus;
  }

  return Status;
}


/*\ 
 *  SET methods
\*/

/* 
ipAddressIfIndex OBJECT-TYPE
    SYNTAX     InterfaceIndex
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The index value that uniquely identifies the interface to
            which this entry is applicable.  The interface identified by
            a particular value of this index is the same interface as
            identified by the same value of the IF-MIB's ifIndex."
    ::= { ipAddressEntry 3 }
*/ 
SNMP_STATUS ipAddressEntryBridge::Set_ipAddressIfIndex (int ipAddressIfIndex)
{
  // We don't support modification of this value beyond initialization.
  if ((fObjectMask & fRequiredObjects) == fRequiredObjects)
  {
    if (ipAddressIfIndex != Get_ipAddressIfIndex())
      return SNMP_INCONSISTENT_VALUE;
  }

  fIpAddressIfIndex = ipAddressIfIndex;
  
  fObjectMask |= kBIT_ipAddressIfIndex;

  // If we have just gotten all of our required objects for the
  // first time, transition from notReady to notInService.
  if (((fObjectMask & fRequiredObjects) == fRequiredObjects) 
  &&  (Get_ipAddressRowStatus() == SNMP_STATUS_NOT_READY))
  {
    int ipAddressRowStatus = SNMP_STATUS_NOT_IN_SERVICE;
    Set_ipAddressRowStatus (ipAddressRowStatus);
  }

  return SNMP_NO_ERROR;
}


/* 
ipAddressType OBJECT-TYPE
    SYNTAX     INTEGER {
                 unicast(1),
                 anycast(2),
                 broadcast(3)
    }
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The type of address.  broadcast(3) is not a valid value for
            IPv6 addresses (RFC 3513)."
    DEFVAL { unicast }
    ::= { ipAddressEntry 4 }
*/ 
SNMP_STATUS ipAddressEntryBridge::Set_ipAddressType (int ipAddressType)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ipAddressType);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ipAddressType != CONSTVAL_IPADDRESSTYPE_UNICAST)
    &&  (ipAddressType != CONSTVAL_IPADDRESSTYPE_ANYCAST)
    &&  (ipAddressType != CONSTVAL_IPADDRESSTYPE_BROADCAST))
      return SNMP_WRONG_VALUE;

    // We don't support modification of this value.
    if (ipAddressType != Get_ipAddressType())
      return SNMP_INCONSISTENT_VALUE;
  }

  return Status;
}


/* 
ipAddressStatus OBJECT-TYPE
    SYNTAX     IpAddressStatusTC
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The status of the address, describing if the address can be
            used for communication.

            In the absence of other information, an IPv4 address is
            always preferred(1)."
    DEFVAL { preferred }
    ::= { ipAddressEntry 7 }
*/ 
SNMP_STATUS ipAddressEntryBridge::Set_ipAddressStatus (int ipAddressStatus)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ipAddressStatus);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ipAddressStatus != CONSTVAL_IPADDRESSSTATUS_PREFERRED)
    &&  (ipAddressStatus != CONSTVAL_IPADDRESSSTATUS_DEPRECATED)
    &&  (ipAddressStatus != CONSTVAL_IPADDRESSSTATUS_INVALID)
    &&  (ipAddressStatus != CONSTVAL_IPADDRESSSTATUS_INACCESSIBLE)
    &&  (ipAddressStatus != CONSTVAL_IPADDRESSSTATUS_UNKNOWN)
    &&  (ipAddressStatus != CONSTVAL_IPADDRESSSTATUS_TENTATIVE)
    &&  (ipAddressStatus != CONSTVAL_IPADDRESSSTATUS_DUPLICATE)
    &&  (ipAddressStatus != CONSTVAL_IPADDRESSSTATUS_OPTIMISTIC))
      return SNMP_WRONG_VALUE;

    // We don't support modification of this value.
    if (ipAddressStatus != Get_ipAddressStatus())
      return SNMP_INCONSISTENT_VALUE;
  }

  return Status;
}


/* 
ipAddressRowStatus OBJECT-TYPE
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
            ipAddressIfIndex has been set to a valid index."
    ::= { ipAddressEntry 10 }
*/ 
SNMP_STATUS ipAddressEntryBridge::Set_ipAddressRowStatus (int &ipAddressRowStatus)
{
  SNMP_STATUS Status = SetRowStatus (ipAddressRowStatus);

  if (Status == SNMP_NO_ERROR)
  {
    fObjectMask |= kBIT_ipAddressRowStatus;
    fRowStatus = ipAddressRowStatus;
  }

  return Status;
}


/* 
ipAddressStorageType OBJECT-TYPE
    SYNTAX     StorageType
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The storage type for this conceptual row.  If this object
            has a value of 'permanent', then no other objects are
            required to be able to be modified."
    DEFVAL { volatile }
    ::= { ipAddressEntry 11 }
*/ 
SNMP_STATUS ipAddressEntryBridge::Set_ipAddressStorageType (int ipAddressStorageType)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ipAddressStorageType);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ipAddressStorageType != CONSTVAL_IPADDRESSSTORAGETYPE_OTHER)
    &&  (ipAddressStorageType != CONSTVAL_IPADDRESSSTORAGETYPE_VOLATILE)
    &&  (ipAddressStorageType != CONSTVAL_IPADDRESSSTORAGETYPE_NONVOLATILE)
    &&  (ipAddressStorageType != CONSTVAL_IPADDRESSSTORAGETYPE_PERMANENT)
    &&  (ipAddressStorageType != CONSTVAL_IPADDRESSSTORAGETYPE_READONLY))
      return SNMP_WRONG_VALUE;

    // We don't support modification of this value.
    if (ipAddressStorageType != Get_ipAddressStorageType())
      return SNMP_INCONSISTENT_VALUE;
  }

  return Status;
}






/*\
 *
 *  ipNetToPhysicalEntryBridge for bridging elements in the ipNetToPhysicalTable
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

ipNetToPhysicalEntryBridge::ipNetToPhysicalEntryBridge (ipNetToMediaEntryBridge *pN2M)
  : TabularMibBridge (kOID_ipNetToPhysicalTable,
                      BcmObjectId(pN2M->Get_ipNetToMediaIfIndex()),
                      pN2M->GetAgent())
{
  BcmObjectId Index;
  GetIndex (Index);
  
  // Append IP address to index
  BcmIpAddress ipNetToPhysicalNetAddress;
  pN2M->Get_ipNetToMediaNetAddress(ipNetToPhysicalNetAddress);
  
  // prepend type and length to IP address
  Index += BcmObjectId(ipNetToPhysicalNetAddress, true, true);
  
  // Re-index
  SetIndex (Index);
  
  fpN2M = pN2M;
  
  fRowStatus = SNMP_STATUS_INVISIBLE;

  // Nothing needed for row creation

  // Programatically created entries will be
  // initialized as if created by CREATE_AND_WAIT.
  // Change to CREATE_AND_GO if immediate activation
  // is desired.
  int Status = SNMP_STATUS_CREATE_AND_GO;
  Set_ipNetToPhysicalRowStatus (Status);
}

ipNetToPhysicalEntryBridge::~ipNetToPhysicalEntryBridge ()
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
int ipNetToPhysicalEntryBridge::Get_ipNetToPhysicalIfIndex ()
{
  return fpN2M->Get_ipNetToMediaIfIndex();
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
int ipNetToPhysicalEntryBridge::Get_ipNetToPhysicalNetAddressType ()
{
  BcmIpAddress IpAddress;


  fpN2M->Get_ipNetToMediaNetAddress(IpAddress);

  // 0.0.0.0 or :: addresses will be treated as 'unknown' w/ length 0
  if ((IpAddress == kIPv6UnspecifiedAddress) || (IpAddress == kAllZerosIp))
  {
      return CONSTVAL_IPNETTOPHYSICALNETADDRESSTYPE_UNKNOWN;
  }

  #if (BCM_IPV6_SUPPORT)
  else if (IpAddress.IsIpV6Address() == true)
  {
      return CONSTVAL_IPNETTOPHYSICALNETADDRESSTYPE_IPV6;
  }
  #endif

  return CONSTVAL_IPNETTOPHYSICALNETADDRESSTYPE_IPV4;
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
int ipNetToPhysicalEntryBridge::Get_ipNetToPhysicalNetAddress (BcmString &ipNetToPhysicalNetAddress)
{
  BcmIpAddress IpAddress;


  fpN2M->Get_ipNetToMediaNetAddress(IpAddress);

  // 0.0.0.0 or :: addresses will be treated as 'unknown' w/ length 0
  if ((IpAddress != kIPv6UnspecifiedAddress) && (IpAddress != kAllZerosIp))
  {
  ipNetToPhysicalNetAddress = IpAddress;
  }

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
void ipNetToPhysicalEntryBridge::Get_ipNetToPhysicalPhysAddress (BcmMacAddress &ipNetToPhysicalPhysAddress)
{
  fpN2M->Get_ipNetToMediaPhysAddress(ipNetToPhysicalPhysAddress);
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
unsigned long ipNetToPhysicalEntryBridge::Get_ipNetToPhysicalLastUpdated ()
{
  // Ask for our last update.  The 'true' argument here means tell us the last
  // time the entry changed, as opposed to the last time we heard from this
  // IP.
  return fpN2M->GetLastUpdate(true);
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
int ipNetToPhysicalEntryBridge::Get_ipNetToPhysicalType ()
{
  return fpN2M->Get_ipNetToMediaType();
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
int ipNetToPhysicalEntryBridge::Get_ipNetToPhysicalState ()
{
  BcmIpAddress NetAddress;
  
  fpN2M->Get_ipNetToMediaNetAddress(NetAddress);
  
  if (NetAddress.IsIpV6Address())
    return CONSTVAL_IPNETTOPHYSICALSTATE_REACHABLE;
  else
    return CONSTVAL_IPNETTOPHYSICALSTATE_UNKNOWN;
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
int ipNetToPhysicalEntryBridge::Get_ipNetToPhysicalRowStatus ()
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
SNMP_STATUS ipNetToPhysicalEntryBridge::Test_ipNetToPhysicalPhysAddress (const BcmMacAddress &ipNetToPhysicalPhysAddress)
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
SNMP_STATUS ipNetToPhysicalEntryBridge::Test_ipNetToPhysicalRowStatus (int ipNetToPhysicalRowStatus)
{
  SNMP_STATUS Status = RowStatusMibObject::TestDefaults (ipNetToPhysicalRowStatus, Get_ipNetToPhysicalRowStatus());

  if (Status == SNMP_NO_ERROR)
  {
    // We don't support setting this object to anything other than
    // 'active'
    if (ipNetToPhysicalRowStatus != SNMP_STATUS_ACTIVE)
      return SNMP_INCONSISTENT_VALUE;
  
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
SNMP_STATUS ipNetToPhysicalEntryBridge::Set_ipNetToPhysicalPhysAddress (const BcmMacAddress &ipNetToPhysicalPhysAddress)
{
  SNMP_STATUS Status = fpN2M->Set_ipNetToMediaPhysAddress(ipNetToPhysicalPhysAddress);
  
  if (Status != SNMP_NO_ERROR)
    return Status;

  fObjectMask |= kBIT_ipNetToPhysicalPhysAddress;

  // If we have just gotten all of our required objects for the
  // first time, transition from notReady to notInService.
  if (((fObjectMask & fRequiredObjects) == fRequiredObjects) 
  &&  (Get_ipNetToPhysicalRowStatus() == SNMP_STATUS_NOT_READY))
  {
    int ipNetToPhysicalRowStatus = SNMP_STATUS_NOT_IN_SERVICE;
    Set_ipNetToPhysicalRowStatus (ipNetToPhysicalRowStatus);
  }

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
SNMP_STATUS ipNetToPhysicalEntryBridge::Set_ipNetToPhysicalType (int ipNetToPhysicalType)
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

    Status = fpN2M->Set_ipNetToMediaType(ipNetToPhysicalType);
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
SNMP_STATUS ipNetToPhysicalEntryBridge::Set_ipNetToPhysicalRowStatus (int &ipNetToPhysicalRowStatus)
{
  SNMP_STATUS Status = SetRowStatus (ipNetToPhysicalRowStatus);

  if (Status == SNMP_NO_ERROR)
  {
    fObjectMask |= kBIT_ipNetToPhysicalRowStatus;
    fRowStatus = ipNetToPhysicalRowStatus;
  }

  return Status;
}




#if 0

/*\
 *
 *  ipv6ScopeZoneIndexEntryBridge for bridging elements in the ipv6ScopeZoneIndexTable
 *
\*/

/* 
ipv6ScopeZoneIndexTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF Ipv6ScopeZoneIndexEntry
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The table used to describe IPv6 unicast and multicast scope
            zones.

            For those objects that have names rather than numbers, the
            names were chosen to coincide with the names used in the
            IPv6 address architecture document. "
    REFERENCE "Section 2.7 of RFC 4291"
    ::= { ip 36 }

INDEX { ipv6ScopeZoneIndexIfIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ipv6ScopeZoneIndexEntryBridge::ipv6ScopeZoneIndexEntryBridge (int ipv6ScopeZoneIndexIfIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ipv6ScopeZoneIndexTable,
                      BcmObjectId(ipv6ScopeZoneIndexIfIndex),
                      pAgent)
{
}

ipv6ScopeZoneIndexEntryBridge::~ipv6ScopeZoneIndexEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ipv6ScopeZoneIndexIfIndex OBJECT-TYPE
    SYNTAX     InterfaceIndex
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The index value that uniquely identifies the interface to
            which these scopes belong.  The interface identified by a
            particular value of this index is the same interface as
            identified by the same value of the IF-MIB's ifIndex."
    ::= { ipv6ScopeZoneIndexEntry 1 }
*/ 
int ipv6ScopeZoneIndexEntryBridge::Get_ipv6ScopeZoneIndexIfIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6ScopeZoneIndexIfIndex" << endl;
  return 0;
}


/* 
ipv6ScopeZoneIndexLinkLocal OBJECT-TYPE
    SYNTAX     InetZoneIndex
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The zone index for the link-local scope on this interface."
    ::= { ipv6ScopeZoneIndexEntry 2 }
*/ 
unsigned int ipv6ScopeZoneIndexEntryBridge::Get_ipv6ScopeZoneIndexLinkLocal ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6ScopeZoneIndexLinkLocal" << endl;
  return 0;
}


/* 
ipv6ScopeZoneIndex3 OBJECT-TYPE
    SYNTAX     InetZoneIndex
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The zone index for scope 3 on this interface."
    ::= { ipv6ScopeZoneIndexEntry 3 }
*/ 
unsigned int ipv6ScopeZoneIndexEntryBridge::Get_ipv6ScopeZoneIndex3 ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6ScopeZoneIndex3" << endl;
  return 0;
}


/* 
ipv6ScopeZoneIndexAdminLocal OBJECT-TYPE
    SYNTAX     InetZoneIndex
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The zone index for the admin-local scope on this interface."
    ::= { ipv6ScopeZoneIndexEntry 4 }
*/ 
unsigned int ipv6ScopeZoneIndexEntryBridge::Get_ipv6ScopeZoneIndexAdminLocal ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6ScopeZoneIndexAdminLocal" << endl;
  return 0;
}


/* 
ipv6ScopeZoneIndexSiteLocal OBJECT-TYPE
    SYNTAX     InetZoneIndex
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The zone index for the site-local scope on this interface."
    ::= { ipv6ScopeZoneIndexEntry 5 }
*/ 
unsigned int ipv6ScopeZoneIndexEntryBridge::Get_ipv6ScopeZoneIndexSiteLocal ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6ScopeZoneIndexSiteLocal" << endl;
  return 0;
}


/* 
ipv6ScopeZoneIndex6 OBJECT-TYPE
    SYNTAX     InetZoneIndex
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The zone index for scope 6 on this interface."
    ::= { ipv6ScopeZoneIndexEntry 6 }
*/ 
unsigned int ipv6ScopeZoneIndexEntryBridge::Get_ipv6ScopeZoneIndex6 ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6ScopeZoneIndex6" << endl;
  return 0;
}


/* 
ipv6ScopeZoneIndex7 OBJECT-TYPE
    SYNTAX     InetZoneIndex
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The zone index for scope 7 on this interface."
    ::= { ipv6ScopeZoneIndexEntry 7 }
*/ 
unsigned int ipv6ScopeZoneIndexEntryBridge::Get_ipv6ScopeZoneIndex7 ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6ScopeZoneIndex7" << endl;
  return 0;
}


/* 
ipv6ScopeZoneIndexOrganizationLocal OBJECT-TYPE
    SYNTAX     InetZoneIndex
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The zone index for the organization-local scope on this
            interface."
    ::= { ipv6ScopeZoneIndexEntry 8 }
*/ 
unsigned int ipv6ScopeZoneIndexEntryBridge::Get_ipv6ScopeZoneIndexOrganizationLocal ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6ScopeZoneIndexOrganizationLocal" << endl;
  return 0;
}


/* 
ipv6ScopeZoneIndex9 OBJECT-TYPE
    SYNTAX     InetZoneIndex
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The zone index for scope 9 on this interface."
    ::= { ipv6ScopeZoneIndexEntry 9 }
*/ 
unsigned int ipv6ScopeZoneIndexEntryBridge::Get_ipv6ScopeZoneIndex9 ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6ScopeZoneIndex9" << endl;
  return 0;
}


/* 
ipv6ScopeZoneIndexA OBJECT-TYPE
    SYNTAX     InetZoneIndex
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The zone index for scope A on this interface."
    ::= { ipv6ScopeZoneIndexEntry 10 }
*/ 
unsigned int ipv6ScopeZoneIndexEntryBridge::Get_ipv6ScopeZoneIndexA ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6ScopeZoneIndexA" << endl;
  return 0;
}


/* 
ipv6ScopeZoneIndexB OBJECT-TYPE
    SYNTAX     InetZoneIndex
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The zone index for scope B on this interface."
    ::= { ipv6ScopeZoneIndexEntry 11 }
*/ 
unsigned int ipv6ScopeZoneIndexEntryBridge::Get_ipv6ScopeZoneIndexB ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6ScopeZoneIndexB" << endl;
  return 0;
}


/* 
ipv6ScopeZoneIndexC OBJECT-TYPE
    SYNTAX     InetZoneIndex
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The zone index for scope C on this interface."
    ::= { ipv6ScopeZoneIndexEntry 12 }
*/ 
unsigned int ipv6ScopeZoneIndexEntryBridge::Get_ipv6ScopeZoneIndexC ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6ScopeZoneIndexC" << endl;
  return 0;
}


/* 
ipv6ScopeZoneIndexD OBJECT-TYPE
    SYNTAX     InetZoneIndex
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The zone index for scope D on this interface."
    ::= { ipv6ScopeZoneIndexEntry 13 }
*/ 
unsigned int ipv6ScopeZoneIndexEntryBridge::Get_ipv6ScopeZoneIndexD ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6ScopeZoneIndexD" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/

#endif



/*\
 *
 *  ipDefaultRouterEntryBridge for bridging elements in the ipDefaultRouterTable
 *
\*/

/* 
ipDefaultRouterTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF IpDefaultRouterEntry
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The table used to describe the default routers known to this
            entity."
    ::= { ip 37 }

INDEX { ipDefaultRouterAddressType, ipDefaultRouterAddress, ipDefaultRouterIfIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ipDefaultRouterEntryBridge::ipDefaultRouterEntryBridge (const BcmIpAddress &ipDefaultRouterAddress, int ipDefaultRouterIfIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ipDefaultRouterTable,
                      BcmObjectId(ipDefaultRouterAddress, true, true) + BcmObjectId(ipDefaultRouterIfIndex),
                      pAgent)
{
  fIpAddress = ipDefaultRouterAddress;
}

ipDefaultRouterEntryBridge::~ipDefaultRouterEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ipDefaultRouterAddressType OBJECT-TYPE
    SYNTAX     InetAddressType
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The address type for this row."
    ::= { ipDefaultRouterEntry 1 }
*/ 
int ipDefaultRouterEntryBridge::Get_ipDefaultRouterAddressType ()
{
  // 0.0.0.0 or :: addresses will be treated as 'unknown' w/ length 0
  // Note that we would never expect to see this but still.
  if ((fIpAddress == kIPv6UnspecifiedAddress) || (fIpAddress == kAllZerosIp))
  {
      return CONSTVAL_IPDEFAULTROUTERADDRESSTYPE_UNKNOWN;
  }

  #if (BCM_IPV6_SUPPORT)
  else if (fIpAddress.IsIpV6Address() == true)
  {
      return CONSTVAL_IPDEFAULTROUTERADDRESSTYPE_IPV6;
  }
  #endif

  return CONSTVAL_IPDEFAULTROUTERADDRESSTYPE_IPV4;
}


/* 
ipDefaultRouterAddress OBJECT-TYPE
    SYNTAX     InetAddress
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The IP address of the default router represented by this
            row.  The address type of this object is specified in
            ipDefaultRouterAddressType.

            Implementers need to be aware that if the size of
            ipDefaultRouterAddress exceeds 115 octets, then OIDS of
            instances of columns in this row will have more than 128
            sub-identifiers and cannot be accessed using SNMPv1,
            SNMPv2c, or SNMPv3."
    ::= { ipDefaultRouterEntry 2 }
*/ 
int ipDefaultRouterEntryBridge::Get_ipDefaultRouterAddress (BcmString &ipDefaultRouterAddress)
{
  // 0.0.0.0 or :: addresses will be treated as 'unknown' w/ length 0
  // Note that we would never expect to see this but still.
  if ((fIpAddress != kIPv6UnspecifiedAddress) && (fIpAddress != kAllZerosIp))
  {
      ipDefaultRouterAddress = fIpAddress;
  }

  return ipDefaultRouterAddress.length();
}


/* 
ipDefaultRouterIfIndex OBJECT-TYPE
    SYNTAX     InterfaceIndex
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The index value that uniquely identifies the interface by
            which the router can be reached.  The interface identified
            by a particular value of this index is the same interface as
            identified by the same value of the IF-MIB's ifIndex."
    ::= { ipDefaultRouterEntry 3 }
*/ 
int ipDefaultRouterEntryBridge::Get_ipDefaultRouterIfIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipDefaultRouterIfIndex" << endl;
  return 0;
}


/* 
ipDefaultRouterLifetime OBJECT-TYPE
    SYNTAX     Unsigned32 (0..65535)
    UNITS      "seconds"
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The remaining length of time, in seconds, that this router
            will continue to be useful as a default router.  A value of
            zero indicates that it is no longer useful as a default
            router.  It is left to the implementer of the MIB as to
            whether a router with a lifetime of zero is removed from the
            list.

            For IPv6, this value should be extracted from the router
            advertisement messages."
    REFERENCE "For IPv6 RFC 2462 sections 4.2 and 6.3.4"
    ::= { ipDefaultRouterEntry 4 }
*/ 
unsigned int ipDefaultRouterEntryBridge::Get_ipDefaultRouterLifetime ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipDefaultRouterLifetime" << endl;
  return 0;
}


/* 
ipDefaultRouterPreference OBJECT-TYPE
    SYNTAX     INTEGER {
                     reserved (-2),
                     low (-1),
                     medium (0),
                     high (1)
                    }
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "An indication of preference given to this router as a
            default router as described in he Default Router
            Preferences document.  Treating the value as a
            2 bit signed integer allows for simple arithmetic
            comparisons.

            For IPv4 routers or IPv6 routers that are not using the
            updated router advertisement format, this object is set to
            medium (0)."
    REFERENCE "RFC 4291, section 2.1"
    ::= { ipDefaultRouterEntry 5 }
*/ 
int ipDefaultRouterEntryBridge::Get_ipDefaultRouterPreference ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipDefaultRouterPreference" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/



#if 0

/*\
 *
 *  ipv6RouterAdvertEntryBridge for bridging elements in the ipv6RouterAdvertTable
 *
\*/

/* 
ipv6RouterAdvertTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF Ipv6RouterAdvertEntry
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The table containing information used to construct router
            advertisements."
    ::= { ip 39 }

INDEX { ipv6RouterAdvertIfIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ipv6RouterAdvertEntryBridge::ipv6RouterAdvertEntryBridge (int ipv6RouterAdvertIfIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ipv6RouterAdvertTable,
                      BcmObjectId(ipv6RouterAdvertIfIndex),
                      pAgent)
{
  fRowStatus = SNMP_STATUS_INVISIBLE;

  // Initialize required objects for row creation:
  fRequiredObjects |= kBIT_ipv6RouterAdvertMinInterval;
  fRequiredObjects |= kBIT_ipv6RouterAdvertCurHopLimit;
  fRequiredObjects |= kBIT_ipv6RouterAdvertDefaultLifetime;

  // Programatically created entries will be
  // initialized as if created by CREATE_AND_WAIT.
  // Change to CREATE_AND_GO if immediate activation
  // is desired.
  int Status = SNMP_STATUS_CREATE_AND_WAIT;
  Set_ipv6RouterAdvertRowStatus (Status);
}

/* Alternate form of constructor for use by ipv6RouterAdvertTable::NewDefaultEntry */
ipv6RouterAdvertEntryBridge::ipv6RouterAdvertEntryBridge (const BcmObjectId &Index, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ipv6RouterAdvertTable, Index, pAgent)
{
  fRowStatus = SNMP_STATUS_INVISIBLE;

  // Initialize required objects for row creation:
  fRequiredObjects |= kBIT_ipv6RouterAdvertMinInterval;
  fRequiredObjects |= kBIT_ipv6RouterAdvertCurHopLimit;
  fRequiredObjects |= kBIT_ipv6RouterAdvertDefaultLifetime;
}

ipv6RouterAdvertEntryBridge::~ipv6RouterAdvertEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ipv6RouterAdvertIfIndex OBJECT-TYPE
    SYNTAX     InterfaceIndex
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The index value that uniquely identifies the interface on
            which router advertisements constructed with this
            information will be transmitted.  The interface identified
            by a particular value of this index is the same interface as
            identified by the same value of the IF-MIB's ifIndex."
    ::= { ipv6RouterAdvertEntry 1 }
*/ 
int ipv6RouterAdvertEntryBridge::Get_ipv6RouterAdvertIfIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6RouterAdvertIfIndex" << endl;
  return 0;
}


/* 
ipv6RouterAdvertSendAdverts OBJECT-TYPE
    SYNTAX     TruthValue
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "A flag indicating whether the router sends periodic
            router advertisements and responds to router solicitations
            on this interface."
    REFERENCE "RFC 2461 Section 6.2.1"
    DEFVAL { false }
    ::= { ipv6RouterAdvertEntry 2 }
*/ 
bool ipv6RouterAdvertEntryBridge::Get_ipv6RouterAdvertSendAdverts ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6RouterAdvertSendAdverts" << endl;
  return true;
}


/* 
ipv6RouterAdvertMaxInterval OBJECT-TYPE
    SYNTAX     Unsigned32 (4..1800)
    UNITS      "seconds"
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The maximum time allowed between sending unsolicited router
            advertisements from this interface."
    REFERENCE "RFC 2461 Section 6.2.1"
    DEFVAL { 600 }
    ::= { ipv6RouterAdvertEntry 3 }
*/ 
unsigned int ipv6RouterAdvertEntryBridge::Get_ipv6RouterAdvertMaxInterval ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6RouterAdvertMaxInterval" << endl;
  return 0;
}


/* 
ipv6RouterAdvertMinInterval OBJECT-TYPE
    SYNTAX     Unsigned32 (3..1350)
    UNITS      "seconds"
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The minimum time allowed between sending unsolicited router
            advertisements from this interface.

            The default is 0.33 * ipv6RouterAdvertMaxInterval, however,
            in the case of a low value for ipv6RouterAdvertMaxInterval,
            the minimum value for this object is restricted to 3."
    REFERENCE "RFC 2461 Section 6.2.1"
    ::= { ipv6RouterAdvertEntry 4 }
*/ 
unsigned int ipv6RouterAdvertEntryBridge::Get_ipv6RouterAdvertMinInterval ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6RouterAdvertMinInterval" << endl;
  return 0;
}


/* 
ipv6RouterAdvertManagedFlag OBJECT-TYPE
    SYNTAX     TruthValue
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The true/false value to be placed into the 'managed address
            configuration' flag field in router advertisements sent from
            this interface."
    REFERENCE "RFC 2461 Section 6.2.1"
    DEFVAL { false }
    ::= { ipv6RouterAdvertEntry 5 }
*/ 
bool ipv6RouterAdvertEntryBridge::Get_ipv6RouterAdvertManagedFlag ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6RouterAdvertManagedFlag" << endl;
  return true;
}


/* 
ipv6RouterAdvertOtherConfigFlag OBJECT-TYPE
    SYNTAX     TruthValue
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The true/false value to be placed into the 'other stateful
            configuration' flag field in router advertisements sent from
            this interface."
    REFERENCE "RFC 2461 Section 6.2.1"
    DEFVAL { false }
    ::= { ipv6RouterAdvertEntry 6 }
*/ 
bool ipv6RouterAdvertEntryBridge::Get_ipv6RouterAdvertOtherConfigFlag ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6RouterAdvertOtherConfigFlag" << endl;
  return true;
}


/* 
ipv6RouterAdvertLinkMTU OBJECT-TYPE
    SYNTAX     Unsigned32
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The value to be placed in MTU options sent by the router on
            this interface.

            A value of zero indicates that no MTU options are sent."
    REFERENCE "RFC 2461 Section 6.2.1"
    DEFVAL { 0 }
    ::= { ipv6RouterAdvertEntry 7 }
*/ 
unsigned int ipv6RouterAdvertEntryBridge::Get_ipv6RouterAdvertLinkMTU ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6RouterAdvertLinkMTU" << endl;
  return 0;
}


/* 
ipv6RouterAdvertReachableTime OBJECT-TYPE
    SYNTAX     Unsigned32 (0..3600000)
    UNITS      "milliseconds"
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The value to be placed in the reachable time field in router
            advertisement messages sent from this interface.

            A value of zero in the router advertisement indicates that
            the advertisement isn't specifying a value for reachable
            time."
    REFERENCE "RFC 2461 Section 6.2.1"
    DEFVAL { 0 }
    ::= { ipv6RouterAdvertEntry 8 }
*/ 
unsigned int ipv6RouterAdvertEntryBridge::Get_ipv6RouterAdvertReachableTime ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6RouterAdvertReachableTime" << endl;
  return 0;
}


/* 
ipv6RouterAdvertRetransmitTime OBJECT-TYPE
    SYNTAX     Unsigned32
    UNITS      "milliseconds"
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The value to be placed in the retransmit timer field in
            router advertisements sent from this interface.

            A value of zero in the router advertisement indicates that
            the advertisement isn't specifying a value for retrans
            time."
    REFERENCE "RFC 2461 Section 6.2.1"
    DEFVAL { 0 }
    ::= { ipv6RouterAdvertEntry 9 }
*/ 
unsigned int ipv6RouterAdvertEntryBridge::Get_ipv6RouterAdvertRetransmitTime ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6RouterAdvertRetransmitTime" << endl;
  return 0;
}


/* 
ipv6RouterAdvertCurHopLimit OBJECT-TYPE
    SYNTAX     Unsigned32 (0..255)
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The default value to be placed in the current hop limit
            field in router advertisements sent from this interface.
            The value should be set to the current diameter of the
            Internet.

            A value of zero in the router advertisement indicates that
            the advertisement isn't specifying a value for curHopLimit.

            The default should be set to the value specified in the IANA
            web pages (www.iana.org) at the time of implementation."
    REFERENCE "RFC 2461 Section 6.2.1"
    ::= { ipv6RouterAdvertEntry 10 }
*/ 
unsigned int ipv6RouterAdvertEntryBridge::Get_ipv6RouterAdvertCurHopLimit ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6RouterAdvertCurHopLimit" << endl;
  return 0;
}


/* 
ipv6RouterAdvertDefaultLifetime OBJECT-TYPE
    SYNTAX     Unsigned32 (0|4..9000)
    UNITS      "seconds"
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The value to be placed in the router lifetime field of
            router advertisements sent from this interface.  This value
            MUST be either 0 or between ipv6RouterAdvertMaxInterval and
            9000 seconds.

            A value of zero indicates that the router is not to be used
            as a default router.

            The default is 3 * ipv6RouterAdvertMaxInterval."
    REFERENCE "RFC 2461 Section 6.2.1"
    ::= { ipv6RouterAdvertEntry 11 }
*/ 
unsigned int ipv6RouterAdvertEntryBridge::Get_ipv6RouterAdvertDefaultLifetime ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipv6RouterAdvertDefaultLifetime" << endl;
  return 0;
}


/* 
ipv6RouterAdvertRowStatus OBJECT-TYPE
    SYNTAX     RowStatus
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The status of this conceptual row.

            As all objects in this conceptual row have default values, a
            row can be created and made active by setting this object
            appropriately.

            The RowStatus TC requires that this DESCRIPTION clause
            states under which circumstances other objects in this row
            can be modified.  The value of this object has no effect on
            whether other objects in this conceptual row can be
            modified."
    ::= { ipv6RouterAdvertEntry 12 }
*/ 
int ipv6RouterAdvertEntryBridge::Get_ipv6RouterAdvertRowStatus ()
{
  return fRowStatus;
}


/*\ 
 *  TEST methods
\*/

/* 
ipv6RouterAdvertMinInterval OBJECT-TYPE
    SYNTAX     Unsigned32 (3..1350)
    UNITS      "seconds"
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The minimum time allowed between sending unsolicited router
            advertisements from this interface.

            The default is 0.33 * ipv6RouterAdvertMaxInterval, however,
            in the case of a low value for ipv6RouterAdvertMaxInterval,
            the minimum value for this object is restricted to 3."
    REFERENCE "RFC 2461 Section 6.2.1"
    ::= { ipv6RouterAdvertEntry 4 }
*/ 
SNMP_STATUS ipv6RouterAdvertEntryBridge::Test_ipv6RouterAdvertMinInterval (unsigned int ipv6RouterAdvertMinInterval)
{
  SNMP_STATUS Status = Uint32MibObject::TestDefaults (ipv6RouterAdvertMinInterval);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ipv6RouterAdvertMinInterval < MINVAL_IPV6ROUTERADVERTMININTERVAL)
    ||  (ipv6RouterAdvertMinInterval > MAXVAL_IPV6ROUTERADVERTMININTERVAL))
      return SNMP_WRONG_VALUE;

    fObjectMask |= kBIT_ipv6RouterAdvertMinInterval;
  }

  return Status;
}

/* 
ipv6RouterAdvertCurHopLimit OBJECT-TYPE
    SYNTAX     Unsigned32 (0..255)
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The default value to be placed in the current hop limit
            field in router advertisements sent from this interface.
            The value should be set to the current diameter of the
            Internet.

            A value of zero in the router advertisement indicates that
            the advertisement isn't specifying a value for curHopLimit.

            The default should be set to the value specified in the IANA
            web pages (www.iana.org) at the time of implementation."
    REFERENCE "RFC 2461 Section 6.2.1"
    ::= { ipv6RouterAdvertEntry 10 }
*/ 
SNMP_STATUS ipv6RouterAdvertEntryBridge::Test_ipv6RouterAdvertCurHopLimit (unsigned int ipv6RouterAdvertCurHopLimit)
{
  SNMP_STATUS Status = Uint32MibObject::TestDefaults (ipv6RouterAdvertCurHopLimit);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ipv6RouterAdvertCurHopLimit < MINVAL_IPV6ROUTERADVERTCURHOPLIMIT)
    ||  (ipv6RouterAdvertCurHopLimit > MAXVAL_IPV6ROUTERADVERTCURHOPLIMIT))
      return SNMP_WRONG_VALUE;

    fObjectMask |= kBIT_ipv6RouterAdvertCurHopLimit;
  }

  return Status;
}

/* 
ipv6RouterAdvertDefaultLifetime OBJECT-TYPE
    SYNTAX     Unsigned32 (0|4..9000)
    UNITS      "seconds"
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The value to be placed in the router lifetime field of
            router advertisements sent from this interface.  This value
            MUST be either 0 or between ipv6RouterAdvertMaxInterval and
            9000 seconds.

            A value of zero indicates that the router is not to be used
            as a default router.

            The default is 3 * ipv6RouterAdvertMaxInterval."
    REFERENCE "RFC 2461 Section 6.2.1"
    ::= { ipv6RouterAdvertEntry 11 }
*/ 
SNMP_STATUS ipv6RouterAdvertEntryBridge::Test_ipv6RouterAdvertDefaultLifetime (unsigned int ipv6RouterAdvertDefaultLifetime)
{
  SNMP_STATUS Status = Uint32MibObject::TestDefaults (ipv6RouterAdvertDefaultLifetime);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ipv6RouterAdvertDefaultLifetime < MINVAL_IPV6ROUTERADVERTDEFAULTLIFETIME)
    ||  (ipv6RouterAdvertDefaultLifetime > MAXVAL_IPV6ROUTERADVERTDEFAULTLIFETIME))
      return SNMP_WRONG_VALUE;

    fObjectMask |= kBIT_ipv6RouterAdvertDefaultLifetime;
  }

  return Status;
}

/* 
ipv6RouterAdvertRowStatus OBJECT-TYPE
    SYNTAX     RowStatus
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The status of this conceptual row.

            As all objects in this conceptual row have default values, a
            row can be created and made active by setting this object
            appropriately.

            The RowStatus TC requires that this DESCRIPTION clause
            states under which circumstances other objects in this row
            can be modified.  The value of this object has no effect on
            whether other objects in this conceptual row can be
            modified."
    ::= { ipv6RouterAdvertEntry 12 }
*/ 
SNMP_STATUS ipv6RouterAdvertEntryBridge::Test_ipv6RouterAdvertRowStatus (int ipv6RouterAdvertRowStatus)
{
  SNMP_STATUS Status = RowStatusMibObject::TestDefaults (ipv6RouterAdvertRowStatus, Get_ipv6RouterAdvertRowStatus());

  if (Status == SNMP_NO_ERROR)
  {
    // UNFINISHED: check any non-default constraints here and
    // return the appropriate error code in case of failure.
    SnmpLogRaw << "UNFINISHED: Test_ipv6RouterAdvertRowStatus" << endl;

    fObjectMask |= kBIT_ipv6RouterAdvertRowStatus;
  }

  return Status;
}


/*\ 
 *  SET methods
\*/

/* 
ipv6RouterAdvertSendAdverts OBJECT-TYPE
    SYNTAX     TruthValue
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "A flag indicating whether the router sends periodic
            router advertisements and responds to router solicitations
            on this interface."
    REFERENCE "RFC 2461 Section 6.2.1"
    DEFVAL { false }
    ::= { ipv6RouterAdvertEntry 2 }
*/ 
SNMP_STATUS ipv6RouterAdvertEntryBridge::Set_ipv6RouterAdvertSendAdverts (bool ipv6RouterAdvertSendAdverts)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;

  {
    if ((ipv6RouterAdvertSendAdverts != true) && (ipv6RouterAdvertSendAdverts != false))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ipv6RouterAdvertSendAdverts" << endl;
  }

  return Status;
}


/* 
ipv6RouterAdvertMaxInterval OBJECT-TYPE
    SYNTAX     Unsigned32 (4..1800)
    UNITS      "seconds"
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The maximum time allowed between sending unsolicited router
            advertisements from this interface."
    REFERENCE "RFC 2461 Section 6.2.1"
    DEFVAL { 600 }
    ::= { ipv6RouterAdvertEntry 3 }
*/ 
SNMP_STATUS ipv6RouterAdvertEntryBridge::Set_ipv6RouterAdvertMaxInterval (unsigned int ipv6RouterAdvertMaxInterval)
{
  SNMP_STATUS Status = Uint32MibObject::TestDefaults (ipv6RouterAdvertMaxInterval);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ipv6RouterAdvertMaxInterval < MINVAL_IPV6ROUTERADVERTMAXINTERVAL)
    ||  (ipv6RouterAdvertMaxInterval > MAXVAL_IPV6ROUTERADVERTMAXINTERVAL))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ipv6RouterAdvertMaxInterval" << endl;
  }

  return Status;
}


/* 
ipv6RouterAdvertMinInterval OBJECT-TYPE
    SYNTAX     Unsigned32 (3..1350)
    UNITS      "seconds"
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The minimum time allowed between sending unsolicited router
            advertisements from this interface.

            The default is 0.33 * ipv6RouterAdvertMaxInterval, however,
            in the case of a low value for ipv6RouterAdvertMaxInterval,
            the minimum value for this object is restricted to 3."
    REFERENCE "RFC 2461 Section 6.2.1"
    ::= { ipv6RouterAdvertEntry 4 }
*/ 
SNMP_STATUS ipv6RouterAdvertEntryBridge::Set_ipv6RouterAdvertMinInterval (unsigned int ipv6RouterAdvertMinInterval)
{
  // UNFINISHED: really set something here.
  SnmpLogRaw << "UNFINISHED: Set_ipv6RouterAdvertMinInterval" << endl;

  fObjectMask |= kBIT_ipv6RouterAdvertMinInterval;

  // If we have just gotten all of our required objects for the
  // first time, transition from notReady to notInService.
  if (((fObjectMask & fRequiredObjects) == fRequiredObjects) 
  &&  (Get_ipv6RouterAdvertRowStatus() == SNMP_STATUS_NOT_READY))
  {
    int ipv6RouterAdvertRowStatus = SNMP_STATUS_NOT_IN_SERVICE;
    Set_ipv6RouterAdvertRowStatus (ipv6RouterAdvertRowStatus);
  }

  return SNMP_NO_ERROR;
}


/* 
ipv6RouterAdvertManagedFlag OBJECT-TYPE
    SYNTAX     TruthValue
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The true/false value to be placed into the 'managed address
            configuration' flag field in router advertisements sent from
            this interface."
    REFERENCE "RFC 2461 Section 6.2.1"
    DEFVAL { false }
    ::= { ipv6RouterAdvertEntry 5 }
*/ 
SNMP_STATUS ipv6RouterAdvertEntryBridge::Set_ipv6RouterAdvertManagedFlag (bool ipv6RouterAdvertManagedFlag)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;

  {
    if ((ipv6RouterAdvertManagedFlag != true) && (ipv6RouterAdvertManagedFlag != false))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ipv6RouterAdvertManagedFlag" << endl;
  }

  return Status;
}


/* 
ipv6RouterAdvertOtherConfigFlag OBJECT-TYPE
    SYNTAX     TruthValue
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The true/false value to be placed into the 'other stateful
            configuration' flag field in router advertisements sent from
            this interface."
    REFERENCE "RFC 2461 Section 6.2.1"
    DEFVAL { false }
    ::= { ipv6RouterAdvertEntry 6 }
*/ 
SNMP_STATUS ipv6RouterAdvertEntryBridge::Set_ipv6RouterAdvertOtherConfigFlag (bool ipv6RouterAdvertOtherConfigFlag)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;

  {
    if ((ipv6RouterAdvertOtherConfigFlag != true) && (ipv6RouterAdvertOtherConfigFlag != false))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ipv6RouterAdvertOtherConfigFlag" << endl;
  }

  return Status;
}


/* 
ipv6RouterAdvertLinkMTU OBJECT-TYPE
    SYNTAX     Unsigned32
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The value to be placed in MTU options sent by the router on
            this interface.

            A value of zero indicates that no MTU options are sent."
    REFERENCE "RFC 2461 Section 6.2.1"
    DEFVAL { 0 }
    ::= { ipv6RouterAdvertEntry 7 }
*/ 
SNMP_STATUS ipv6RouterAdvertEntryBridge::Set_ipv6RouterAdvertLinkMTU (unsigned int ipv6RouterAdvertLinkMTU)
{
  SNMP_STATUS Status = Uint32MibObject::TestDefaults (ipv6RouterAdvertLinkMTU);

  if (Status == SNMP_NO_ERROR)
  {
    // UNFINISHED: no constraints specified in MIB for 
    // ipv6RouterAdvertLinkMTU. Check if necessary.

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ipv6RouterAdvertLinkMTU" << endl;
  }

  return Status;
}


/* 
ipv6RouterAdvertReachableTime OBJECT-TYPE
    SYNTAX     Unsigned32 (0..3600000)
    UNITS      "milliseconds"
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The value to be placed in the reachable time field in router
            advertisement messages sent from this interface.

            A value of zero in the router advertisement indicates that
            the advertisement isn't specifying a value for reachable
            time."
    REFERENCE "RFC 2461 Section 6.2.1"
    DEFVAL { 0 }
    ::= { ipv6RouterAdvertEntry 8 }
*/ 
SNMP_STATUS ipv6RouterAdvertEntryBridge::Set_ipv6RouterAdvertReachableTime (unsigned int ipv6RouterAdvertReachableTime)
{
  SNMP_STATUS Status = Uint32MibObject::TestDefaults (ipv6RouterAdvertReachableTime);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ipv6RouterAdvertReachableTime < MINVAL_IPV6ROUTERADVERTREACHABLETIME)
    ||  (ipv6RouterAdvertReachableTime > MAXVAL_IPV6ROUTERADVERTREACHABLETIME))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ipv6RouterAdvertReachableTime" << endl;
  }

  return Status;
}


/* 
ipv6RouterAdvertRetransmitTime OBJECT-TYPE
    SYNTAX     Unsigned32
    UNITS      "milliseconds"
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The value to be placed in the retransmit timer field in
            router advertisements sent from this interface.

            A value of zero in the router advertisement indicates that
            the advertisement isn't specifying a value for retrans
            time."
    REFERENCE "RFC 2461 Section 6.2.1"
    DEFVAL { 0 }
    ::= { ipv6RouterAdvertEntry 9 }
*/ 
SNMP_STATUS ipv6RouterAdvertEntryBridge::Set_ipv6RouterAdvertRetransmitTime (unsigned int ipv6RouterAdvertRetransmitTime)
{
  SNMP_STATUS Status = Uint32MibObject::TestDefaults (ipv6RouterAdvertRetransmitTime);

  if (Status == SNMP_NO_ERROR)
  {
    // UNFINISHED: no constraints specified in MIB for 
    // ipv6RouterAdvertRetransmitTime. Check if necessary.

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ipv6RouterAdvertRetransmitTime" << endl;
  }

  return Status;
}


/* 
ipv6RouterAdvertCurHopLimit OBJECT-TYPE
    SYNTAX     Unsigned32 (0..255)
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The default value to be placed in the current hop limit
            field in router advertisements sent from this interface.
            The value should be set to the current diameter of the
            Internet.

            A value of zero in the router advertisement indicates that
            the advertisement isn't specifying a value for curHopLimit.

            The default should be set to the value specified in the IANA
            web pages (www.iana.org) at the time of implementation."
    REFERENCE "RFC 2461 Section 6.2.1"
    ::= { ipv6RouterAdvertEntry 10 }
*/ 
SNMP_STATUS ipv6RouterAdvertEntryBridge::Set_ipv6RouterAdvertCurHopLimit (unsigned int ipv6RouterAdvertCurHopLimit)
{
  // UNFINISHED: really set something here.
  SnmpLogRaw << "UNFINISHED: Set_ipv6RouterAdvertCurHopLimit" << endl;

  fObjectMask |= kBIT_ipv6RouterAdvertCurHopLimit;

  // If we have just gotten all of our required objects for the
  // first time, transition from notReady to notInService.
  if (((fObjectMask & fRequiredObjects) == fRequiredObjects) 
  &&  (Get_ipv6RouterAdvertRowStatus() == SNMP_STATUS_NOT_READY))
  {
    int ipv6RouterAdvertRowStatus = SNMP_STATUS_NOT_IN_SERVICE;
    Set_ipv6RouterAdvertRowStatus (ipv6RouterAdvertRowStatus);
  }

  return SNMP_NO_ERROR;
}


/* 
ipv6RouterAdvertDefaultLifetime OBJECT-TYPE
    SYNTAX     Unsigned32 (0|4..9000)
    UNITS      "seconds"
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The value to be placed in the router lifetime field of
            router advertisements sent from this interface.  This value
            MUST be either 0 or between ipv6RouterAdvertMaxInterval and
            9000 seconds.

            A value of zero indicates that the router is not to be used
            as a default router.

            The default is 3 * ipv6RouterAdvertMaxInterval."
    REFERENCE "RFC 2461 Section 6.2.1"
    ::= { ipv6RouterAdvertEntry 11 }
*/ 
SNMP_STATUS ipv6RouterAdvertEntryBridge::Set_ipv6RouterAdvertDefaultLifetime (unsigned int ipv6RouterAdvertDefaultLifetime)
{
  // UNFINISHED: really set something here.
  SnmpLogRaw << "UNFINISHED: Set_ipv6RouterAdvertDefaultLifetime" << endl;

  fObjectMask |= kBIT_ipv6RouterAdvertDefaultLifetime;

  // If we have just gotten all of our required objects for the
  // first time, transition from notReady to notInService.
  if (((fObjectMask & fRequiredObjects) == fRequiredObjects) 
  &&  (Get_ipv6RouterAdvertRowStatus() == SNMP_STATUS_NOT_READY))
  {
    int ipv6RouterAdvertRowStatus = SNMP_STATUS_NOT_IN_SERVICE;
    Set_ipv6RouterAdvertRowStatus (ipv6RouterAdvertRowStatus);
  }

  return SNMP_NO_ERROR;
}


/* 
ipv6RouterAdvertRowStatus OBJECT-TYPE
    SYNTAX     RowStatus
    MAX-ACCESS read-create
    STATUS     current
    DESCRIPTION
           "The status of this conceptual row.

            As all objects in this conceptual row have default values, a
            row can be created and made active by setting this object
            appropriately.

            The RowStatus TC requires that this DESCRIPTION clause
            states under which circumstances other objects in this row
            can be modified.  The value of this object has no effect on
            whether other objects in this conceptual row can be
            modified."
    ::= { ipv6RouterAdvertEntry 12 }
*/ 
SNMP_STATUS ipv6RouterAdvertEntryBridge::Set_ipv6RouterAdvertRowStatus (int &ipv6RouterAdvertRowStatus)
{
  SNMP_STATUS Status = SetRowStatus (ipv6RouterAdvertRowStatus);

  if (Status == SNMP_NO_ERROR)
  {
    fObjectMask |= kBIT_ipv6RouterAdvertRowStatus;
    fRowStatus = ipv6RouterAdvertRowStatus;

    // UNFINISHED: really set something here!
    SnmpLogRaw << "UNFINISHED: Set_ipv6RouterAdvertRowStatus" << endl;
  }

  return Status;
}

#endif




/*\
 *
 *  ipAddrEntryBridge for bridging elements in the ipAddrTable
 *
\*/

/* 
ipAddrTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF IpAddrEntry
    MAX-ACCESS not-accessible
    STATUS     deprecated
    DESCRIPTION
           "The table of addressing information relevant to this
            entity's IPv4 addresses.

            This table has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by the
            ipAddressTable although several objects that weren't deemed
            useful weren't carried forward while another
            (ipAdEntReasmMaxSize) was moved to the ipv4InterfaceTable."
    ::= { ip 20 }

INDEX { ipAdEntAddr }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ipAddrEntryBridge::ipAddrEntryBridge (const BcmIpAddress &ipAdEntAddr, 
                   int ipAdEntIfIndex,
                   const BcmIpAddress &ipAdEntNetMask,
                   int ipAdEntBcastAddr,
                   int ipAdEntReasmMaxSize, 
                   BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ipAddrTable,
                      BcmObjectId(ipAdEntAddr),
                      pAgent)
{
  fIpAdEntAddr = ipAdEntAddr; 
  fIpAdEntIfIndex = ipAdEntIfIndex;
  fIpAdEntNetMask = ipAdEntNetMask;
  fIpAdEntBcastAddr = ipAdEntBcastAddr;
  fIpAdEntReasmMaxSize = ipAdEntReasmMaxSize;
}

ipAddrEntryBridge::~ipAddrEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ipAdEntAddr OBJECT-TYPE
    SYNTAX     IpAddress
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The IPv4 address to which this entry's addressing
            information pertains."
    ::= { ipAddrEntry 1 }
*/ 
void ipAddrEntryBridge::Get_ipAdEntAddr (BcmIpAddress &ipAdEntAddr)
{
  ipAdEntAddr = fIpAdEntAddr;
}


/* 
ipAdEntIfIndex OBJECT-TYPE
    SYNTAX     INTEGER (1..2147483647)
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The index value which uniquely identifies the interface to
            which this entry is applicable.  The interface identified by
            a particular value of this index is the same interface as
            identified by the same value of the IF-MIB's ifIndex."
    ::= { ipAddrEntry 2 }
*/ 
int ipAddrEntryBridge::Get_ipAdEntIfIndex ()
{
  return fIpAdEntIfIndex;
}


/* 
ipAdEntNetMask OBJECT-TYPE
    SYNTAX     IpAddress
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The subnet mask associated with the IPv4 address of this
            entry.  The value of the mask is an IPv4 address with all
            the network bits set to 1 and all the hosts bits set to 0."
    ::= { ipAddrEntry 3 }
*/ 
void ipAddrEntryBridge::Get_ipAdEntNetMask (BcmIpAddress &ipAdEntNetMask)
{
  ipAdEntNetMask = fIpAdEntNetMask;
}


/* 
ipAdEntBcastAddr OBJECT-TYPE
    SYNTAX     INTEGER (0..1)
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The value of the least-significant bit in the IPv4 broadcast
            address used for sending datagrams on the (logical)
            interface associated with the IPv4 address of this entry.
            For example, when the Internet standard all-ones broadcast
            address is used, the value will be 1.  This value applies to
            both the subnet and network broadcast addresses used by the
            entity on this (logical) interface."
    ::= { ipAddrEntry 4 }
*/ 
int ipAddrEntryBridge::Get_ipAdEntBcastAddr ()
{
  return fIpAdEntBcastAddr;
}


/* 
ipAdEntReasmMaxSize OBJECT-TYPE
    SYNTAX     INTEGER (0..65535)
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The size of the largest IPv4 datagram which this entity can
            re-assemble from incoming IPv4 fragmented datagrams received
            on this interface."
    ::= { ipAddrEntry 5 }
*/ 
int ipAddrEntryBridge::Get_ipAdEntReasmMaxSize ()
{
  return fIpAdEntReasmMaxSize;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ipNetToMediaEntryBridge for bridging elements in the ipNetToMediaTable
 *
\*/

/* 
ipNetToMediaTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF IpNetToMediaEntry
    MAX-ACCESS not-accessible
    STATUS     deprecated
    DESCRIPTION
           "The IPv4 Address Translation table used for mapping from
            IPv4 addresses to physical addresses.

            This table has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by the
            ipNetToPhysicalTable."
    ::= { ip 22 }

INDEX { ipNetToMediaIfIndex, ipNetToMediaNetAddress }
*/ 

/*\ 
 *  construction / destruction
\*/ 

// NOTE: The pHal argument in this constructor points to the BcmHalIf object
//       which is connected to the IP/MAC address mapping being performed.
//       The ipNetToMediaIfIndex value for this mapping will be the
//       ifIndex value for this HAL.  For entries which are learned via
//       the BcmNetToMediaSnoop, this is the interface which sent the
//       incoming packet to the IP stack, or to which the IP stack sent the
//       outgoing packet (eg ethernet).
//
//       The IpStackInterface argument, on the other hand, indicates which
//       IP stack this entry is associated with.  For entries which are
//       learned via the BcmNetToMediaSnoop, this is the interface
//       number (NOT ifIndex) of the IP stack which sent or received the 
//       packet from which we learn the association.
//
//       The reason for the IpStackInterface argument is because we might
//       have a single snoop servicing multiple agents, should we have a
//       master agent situation.  In that case, when one agent's IP address
//       changes, for example (see PR 5814), we don't want to lose all the
//       entries for the other agent.  This argument gives us a way to 
//       keep track of which entry is related to which IP stack.
ipNetToMediaEntryBridge::ipNetToMediaEntryBridge (const BcmHalIf *pHal,
                                                  const BcmMacAddress &ipNetToMediaPhysAddress, 
                                                  const BcmIpAddress &ipNetToMediaNetAddress, 
                                                  int ipNetToMediaType, 
                                                  int IpStackInterface, 
                                                  BcmSnmpAgent *pAgent)
#ifdef BCM_STANDALONE_SNMP_AGENT
  : TabularMibBridge (kOID_ipNetToMediaTable,
                      BcmObjectId(pAgent->GetIfNum()) + BcmObjectId(ipNetToMediaNetAddress),
                      pAgent)
#else
  : TabularMibBridge (kOID_ipNetToMediaTable,
                      BcmObjectId(pHal->IfTableIndex(pAgent)) + BcmObjectId(ipNetToMediaNetAddress),
                      pAgent)
#endif
{
  fpHal = pHal;
  fIp = ipNetToMediaNetAddress;
  fIpStackIf = IpStackInterface;
  
  Update (ipNetToMediaPhysAddress, ipNetToMediaType);
  
  fpN2P = NULL;
}

ipNetToMediaEntryBridge::~ipNetToMediaEntryBridge ()
{
  if (fpN2P)
  {
    delete fpN2P;
    fpN2P = NULL;
  }
}

SNMP_STATUS ipNetToMediaEntryBridge::Register(CleanupMethod Cleanup, bool SilentFailure)
{
  SNMP_STATUS Status = TabularMibBridge::Register (Cleanup, SilentFailure);
  if (Status == SNMP_NO_ERROR)
  {
    fpN2P = new ipNetToPhysicalEntryBridge(this);
    if (fpN2P->Register (kDeleteBridge, SilentFailure) != SNMP_NO_ERROR)
    {
      delete fpN2P;
      fpN2P = NULL;
    }
  }
    
  return Status;
}

void ipNetToMediaEntryBridge::Update (const BcmMacAddress &Mac, int Type)
{
  // Update last update time.  This is to help us identify the oldest
  // entry in the table should we have to start dropping entries due to the
  // table getting too big.  Typically this would only happen as a result
  // of an intentional denial of service attack (like targa2 oshare).
  fLastUpdate = fpAgent->sysUpTime();
  
  // Distinguish between 'updated' and 'changed' since they are used
  // differently.
  if ((fMac != Mac) || (fType != Type))
    fLastChange = fLastUpdate;
  
  fMac = Mac;
  fType = Type;
}

/*\ 
 *  GET methods
\*/

/* 
ipNetToMediaIfIndex OBJECT-TYPE
    SYNTAX     INTEGER (1..2147483647)
    MAX-ACCESS read-create
    STATUS     deprecated
    DESCRIPTION
           "The interface on which this entry's equivalence is
            effective.  The interface identified by a particular value
            of this index is the same interface as identified by the
            same value of the IF-MIB's ifIndex.

            This object predates the rule limiting index objects to a
            max access value of 'not-accessible' and so continues to use
            a value of 'read-create'."
    ::= { ipNetToMediaEntry 1 }
*/ 
int ipNetToMediaEntryBridge::Get_ipNetToMediaIfIndex ()
{
#ifdef BCM_STANDALONE_SNMP_AGENT
  return fIpStackIf;
#else
  return fpHal->IfTableIndex(fpAgent);
#endif
}


/* 
ipNetToMediaPhysAddress OBJECT-TYPE
    SYNTAX     PhysAddress (SIZE(0..65535))
    MAX-ACCESS read-create
    STATUS     deprecated
    DESCRIPTION
           "The media-dependent `physical' address.  This object should
            return 0 when this entry is in the 'incomplete' state.

            As the entries in this table are typically not persistent
            when this object is written the entity should not save the
            change to non-volatile storage.  Note: a stronger
            requirement is not used because this object was previously
            defined."
    ::= { ipNetToMediaEntry 2 }
*/ 
void ipNetToMediaEntryBridge::Get_ipNetToMediaPhysAddress (BcmMacAddress &ipNetToMediaPhysAddress)
{
  ipNetToMediaPhysAddress = fMac;
}


/* 
ipNetToMediaNetAddress OBJECT-TYPE
    SYNTAX     IpAddress
    MAX-ACCESS read-create
    STATUS     deprecated
    DESCRIPTION
           "The IpAddress corresponding to the media-dependent
            `physical' address.

            This object predates the rule limiting index objects to a
            max access value of 'not-accessible' and so continues to use
            a value of 'read-create'."
    ::= { ipNetToMediaEntry 3 }
*/ 
void ipNetToMediaEntryBridge::Get_ipNetToMediaNetAddress (BcmIpAddress &ipNetToMediaNetAddress)
{
  ipNetToMediaNetAddress = fIp;
}


/* 
ipNetToMediaType OBJECT-TYPE
    SYNTAX     INTEGER {
                other(1),        -- none of the following
                invalid(2),      -- an invalidated mapping
                dynamic(3),
                static(4)
            }
    MAX-ACCESS read-create
    STATUS     deprecated
    DESCRIPTION
           "The type of mapping.

            Setting this object to the value invalid(2) has the effect
            of invalidating the corresponding entry in the
            ipNetToMediaTable.  That is, it effectively dis-associates
            the interface identified with said entry from the mapping
            identified with said entry.  It is an implementation-
            specific matter as to whether the agent removes an
            invalidated entry from the table.  Accordingly, management
            stations must be prepared to receive tabular information
            from agents that corresponds to entries not currently in
            use.  Proper interpretation of such entries requires
            examination of the relevant ipNetToMediaType object.

            As the entries in this table are typically not persistent
            when this object is written the entity should not save the
            change to non-volatile storage.  Note: a stronger
            requirement is not used because this object was previously
            defined."
    ::= { ipNetToMediaEntry 4 }
*/ 
int ipNetToMediaEntryBridge::Get_ipNetToMediaType ()
{
  return fType;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/

/* 
ipNetToMediaIfIndex OBJECT-TYPE
    SYNTAX     INTEGER (1..2147483647)
    MAX-ACCESS read-create
    STATUS     deprecated
    DESCRIPTION
           "The interface on which this entry's equivalence is
            effective.  The interface identified by a particular value
            of this index is the same interface as identified by the
            same value of the IF-MIB's ifIndex.

            This object predates the rule limiting index objects to a
            max access value of 'not-accessible' and so continues to use
            a value of 'read-create'."
    ::= { ipNetToMediaEntry 1 }
*/ 
SNMP_STATUS ipNetToMediaEntryBridge::Set_ipNetToMediaIfIndex (int ipNetToMediaIfIndex)
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
    SYNTAX     PhysAddress (SIZE(0..65535))
    MAX-ACCESS read-create
    STATUS     deprecated
    DESCRIPTION
           "The media-dependent `physical' address.  This object should
            return 0 when this entry is in the 'incomplete' state.

            As the entries in this table are typically not persistent
            when this object is written the entity should not save the
            change to non-volatile storage.  Note: a stronger
            requirement is not used because this object was previously
            defined."
    ::= { ipNetToMediaEntry 2 }
*/ 
SNMP_STATUS ipNetToMediaEntryBridge::Set_ipNetToMediaPhysAddress (const BcmMacAddress &ipNetToMediaPhysAddress)
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
    SYNTAX     IpAddress
    MAX-ACCESS read-create
    STATUS     deprecated
    DESCRIPTION
           "The IpAddress corresponding to the media-dependent
            `physical' address.

            This object predates the rule limiting index objects to a
            max access value of 'not-accessible' and so continues to use
            a value of 'read-create'."
    ::= { ipNetToMediaEntry 3 }
*/ 
SNMP_STATUS ipNetToMediaEntryBridge::Set_ipNetToMediaNetAddress (const BcmIpAddress &ipNetToMediaNetAddress)
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
    SYNTAX     INTEGER {
                other(1),        -- none of the following
                invalid(2),      -- an invalidated mapping
                dynamic(3),
                static(4)
            }
    MAX-ACCESS read-create
    STATUS     deprecated
    DESCRIPTION
           "The type of mapping.

            Setting this object to the value invalid(2) has the effect
            of invalidating the corresponding entry in the
            ipNetToMediaTable.  That is, it effectively dis-associates
            the interface identified with said entry from the mapping
            identified with said entry.  It is an implementation-
            specific matter as to whether the agent removes an
            invalidated entry from the table.  Accordingly, management
            stations must be prepared to receive tabular information
            from agents that corresponds to entries not currently in
            use.  Proper interpretation of such entries requires
            examination of the relevant ipNetToMediaType object.

            As the entries in this table are typically not persistent
            when this object is written the entity should not save the
            change to non-volatile storage.  Note: a stronger
            requirement is not used because this object was previously
            defined."
    ::= { ipNetToMediaEntry 4 }
*/ 
SNMP_STATUS ipNetToMediaEntryBridge::Set_ipNetToMediaType (int ipNetToMediaType)
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




#if 0

/*\
 *
 *  ipTrafficStatsGroupBridge for bridging scalars from the ipTrafficStatsGroup group 
 *
\*/

/* 
ipTrafficStats OBJECT IDENTIFIER ::= { ip 31 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ipTrafficStatsGroupBridge::ipTrafficStatsGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_ipTrafficStatsGroup, pAgent)
{
}

ipTrafficStatsGroupBridge::~ipTrafficStatsGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ipIfStatsTableLastChange OBJECT-TYPE
    SYNTAX     TimeStamp
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The value of sysUpTime on the most recent occasion at which
            a row in the ipIfStatsTable was added or deleted.

            If new objects are added to the ipIfStatsTable that require
            the ipIfStatsTableLastChange to be updated when they are
            modified, they must specify that requirement in their
            description clause."
    ::= { ipTrafficStats 2 }
*/ 
unsigned long ipTrafficStatsGroupBridge::Get_ipIfStatsTableLastChange ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsTableLastChange" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ipSystemStatsEntryBridge for bridging elements in the ipSystemStatsTable
 *
\*/

/* 
ipSystemStatsTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF IpSystemStatsEntry
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The table containing system wide, IP version specific
            traffic statistics.  This table and the ipIfStatsTable
            contain similar objects whose difference is in their
            granularity.  Where this table contains system wide traffic
            statistics, the ipIfStatsTable contains the same statistics
            but counted on a per-interface basis."
    ::= { ipTrafficStats 1 }

INDEX { ipSystemStatsIPVersion }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ipSystemStatsEntryBridge::ipSystemStatsEntryBridge (int ipSystemStatsIPVersion, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ipSystemStatsTable,
                      BcmObjectId(ipSystemStatsIPVersion),
                      pAgent)
{
}

ipSystemStatsEntryBridge::~ipSystemStatsEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ipSystemStatsIPVersion OBJECT-TYPE
    SYNTAX     InetVersion
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The IP version of this row."
    ::= { ipSystemStatsEntry 1 }
*/ 
int ipSystemStatsEntryBridge::Get_ipSystemStatsIPVersion ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsIPVersion" << endl;
  return 0;
}


/* 
ipSystemStatsInReceives OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of input IP datagrams received, including
            those received in error.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 3 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsInReceives ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsInReceives" << endl;
  return 0;
}


/* 
ipSystemStatsHCInReceives OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of input IP datagrams received, including
            those received in error.  This object counts the same
            datagrams as ipSystemStatsInReceives, but allows for larger
            values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 4 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsHCInReceives (BcmCounter64 &ipSystemStatsHCInReceives)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsHCInReceives" << endl;
  ipSystemStatsHCInReceives = Value;

  return ipSystemStatsHCInReceives.Low32();
}


/* 
ipSystemStatsInOctets OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of octets received in input IP datagrams,
            including those received in error.  Octets from datagrams
            counted in ipSystemStatsInReceives MUST be counted here.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 5 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsInOctets ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsInOctets" << endl;
  return 0;
}


/* 
ipSystemStatsHCInOctets OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of octets received in input IP datagrams,
            including those received in error.  This object counts the
            same octets as ipSystemStatsInOctets, but allows for larger
            values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 6 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsHCInOctets (BcmCounter64 &ipSystemStatsHCInOctets)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsHCInOctets" << endl;
  ipSystemStatsHCInOctets = Value;

  return ipSystemStatsHCInOctets.Low32();
}


/* 
ipSystemStatsInHdrErrors OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of input IP datagrams discarded due to errors in
            their IP headers, including version number mismatch, other
            format errors, hop count exceeded, errors discovered in
            processing their IP options, etc.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 7 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsInHdrErrors ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsInHdrErrors" << endl;
  return 0;
}


/* 
ipSystemStatsInNoRoutes OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of input IP datagrams discarded because no route
            could be found to transmit them to their destination.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 8 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsInNoRoutes ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsInNoRoutes" << endl;
  return 0;
}


/* 
ipSystemStatsInAddrErrors OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of input IP datagrams discarded because the IP
            address in their IP header's destination field was not a
            valid address to be received at this entity.  This count
            includes invalid addresses (e.g., ::0).  For entities
            that are not IP routers and therefore do not forward
            datagrams, this counter includes datagrams discarded
            because the destination address was not a local address.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 9 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsInAddrErrors ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsInAddrErrors" << endl;
  return 0;
}


/* 
ipSystemStatsInUnknownProtos OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of locally-addressed IP datagrams received
            successfully but discarded because of an unknown or
            unsupported protocol.

            When tracking interface statistics, the counter of the
            interface to which these datagrams were addressed is
            incremented.  This interface might not be the same as the
            input interface for some of the datagrams.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 10 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsInUnknownProtos ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsInUnknownProtos" << endl;
  return 0;
}


/* 
ipSystemStatsInTruncatedPkts OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of input IP datagrams discarded because the
            datagram frame didn't carry enough data.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 11 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsInTruncatedPkts ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsInTruncatedPkts" << endl;
  return 0;
}


/* 
ipSystemStatsInForwDatagrams OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of input datagrams for which this entity was not
            their final IP destination and for which this entity
            attempted to find a route to forward them to that final
            destination.  In entities that do not act as IP routers,
            this counter will include only those datagrams that were
            Source-Routed via this entity, and the Source-Route
            processing was successful.

            When tracking interface statistics, the counter of the
            incoming interface is incremented for each datagram.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 12 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsInForwDatagrams ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsInForwDatagrams" << endl;
  return 0;
}


/* 
ipSystemStatsHCInForwDatagrams OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of input datagrams for which this entity was not
            their final IP destination and for which this entity
            attempted to find a route to forward them to that final
            destination.  This object counts the same packets as
            ipSystemStatsInForwDatagrams, but allows for larger values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 13 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsHCInForwDatagrams (BcmCounter64 &ipSystemStatsHCInForwDatagrams)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsHCInForwDatagrams" << endl;
  ipSystemStatsHCInForwDatagrams = Value;

  return ipSystemStatsHCInForwDatagrams.Low32();
}


/* 
ipSystemStatsReasmReqds OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP fragments received that needed to be
            reassembled at this interface.

            When tracking interface statistics, the counter of the
            interface to which these fragments were addressed is
            incremented.  This interface might not be the same as the
            input interface for some of the fragments.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 14 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsReasmReqds ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsReasmReqds" << endl;
  return 0;
}


/* 
ipSystemStatsReasmOKs OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP datagrams successfully reassembled.

            When tracking interface statistics, the counter of the
            interface to which these datagrams were addressed is
            incremented.  This interface might not be the same as the
            input interface for some of the datagrams.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 15 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsReasmOKs ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsReasmOKs" << endl;
  return 0;
}


/* 
ipSystemStatsReasmFails OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of failures detected by the IP re-assembly
            algorithm (for whatever reason: timed out, errors, etc.).
            Note that this is not necessarily a count of discarded IP
            fragments since some algorithms (notably the algorithm in
            RFC 815) can lose track of the number of fragments by
            combining them as they are received.

            When tracking interface statistics, the counter of the
            interface to which these fragments were addressed is
            incremented.  This interface might not be the same as the
            input interface for some of the fragments.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 16 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsReasmFails ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsReasmFails" << endl;
  return 0;
}


/* 
ipSystemStatsInDiscards OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of input IP datagrams for which no problems were
            encountered to prevent their continued processing, but
            were discarded (e.g., for lack of buffer space).  Note that
            this counter does not include any datagrams discarded while
            awaiting re-assembly.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 17 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsInDiscards ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsInDiscards" << endl;
  return 0;
}


/* 
ipSystemStatsInDelivers OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of datagrams successfully delivered to IP
            user-protocols (including ICMP).

            When tracking interface statistics, the counter of the
            interface to which these datagrams were addressed is
            incremented.  This interface might not be the same as the
            input interface for some of the datagrams.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 18 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsInDelivers ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsInDelivers" << endl;
  return 0;
}


/* 
ipSystemStatsHCInDelivers OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of datagrams successfully delivered to IP
            user-protocols (including ICMP).  This object counts the
            same packets as ipSystemStatsInDelivers, but allows for
            larger values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 19 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsHCInDelivers (BcmCounter64 &ipSystemStatsHCInDelivers)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsHCInDelivers" << endl;
  ipSystemStatsHCInDelivers = Value;

  return ipSystemStatsHCInDelivers.Low32();
}


/* 
ipSystemStatsOutRequests OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of IP datagrams that local IP user-
            protocols (including ICMP) supplied to IP in requests for
            transmission.  Note that this counter does not include any
            datagrams counted in ipSystemStatsOutForwDatagrams.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 20 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsOutRequests ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsOutRequests" << endl;
  return 0;
}


/* 
ipSystemStatsHCOutRequests OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of IP datagrams that local IP user-
            protocols (including ICMP) supplied to IP in requests for
            transmission.  This object counts the same packets as
            ipSystemStatsOutRequests, but allows for larger values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 21 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsHCOutRequests (BcmCounter64 &ipSystemStatsHCOutRequests)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsHCOutRequests" << endl;
  ipSystemStatsHCOutRequests = Value;

  return ipSystemStatsHCOutRequests.Low32();
}


/* 
ipSystemStatsOutNoRoutes OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of locally generated IP datagrams discarded
            because no route could be found to transmit them to their
            destination.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 22 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsOutNoRoutes ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsOutNoRoutes" << endl;
  return 0;
}


/* 
ipSystemStatsOutForwDatagrams OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of datagrams for which this entity was not their
            final IP destination and for which it was successful in
            finding a path to their final destination.  In entities
            that do not act as IP routers, this counter will include
            only those datagrams that were Source-Routed via this
            entity, and the Source-Route processing was successful.

            When tracking interface statistics, the counter of the
            outgoing interface is incremented for a successfully
            forwarded datagram.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 23 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsOutForwDatagrams ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsOutForwDatagrams" << endl;
  return 0;
}


/* 
ipSystemStatsHCOutForwDatagrams OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of datagrams for which this entity was not their
            final IP destination and for which it was successful in
            finding a path to their final destination.  This object
            counts the same packets as ipSystemStatsOutForwDatagrams,
            but allows for larger values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 24 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsHCOutForwDatagrams (BcmCounter64 &ipSystemStatsHCOutForwDatagrams)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsHCOutForwDatagrams" << endl;
  ipSystemStatsHCOutForwDatagrams = Value;

  return ipSystemStatsHCOutForwDatagrams.Low32();
}


/* 
ipSystemStatsOutDiscards OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of output IP datagrams for which no problem was
            encountered to prevent their transmission to their
            destination, but were discarded (e.g., for lack of
            buffer space).  Note that this counter would include
            datagrams counted in ipSystemStatsOutForwDatagrams if any
            such datagrams met this (discretionary) discard criterion.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 25 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsOutDiscards ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsOutDiscards" << endl;
  return 0;
}


/* 
ipSystemStatsOutFragReqds OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP datagrams that would require fragmentation
            in order to be transmitted.

            When tracking interface statistics, the counter of the
            outgoing interface is incremented for a successfully
            fragmented datagram.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 26 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsOutFragReqds ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsOutFragReqds" << endl;
  return 0;
}


/* 
ipSystemStatsOutFragOKs OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP datagrams that have been successfully
            fragmented.

            When tracking interface statistics, the counter of the
            outgoing interface is incremented for a successfully
            fragmented datagram.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 27 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsOutFragOKs ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsOutFragOKs" << endl;
  return 0;
}


/* 
ipSystemStatsOutFragFails OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP datagrams that have been discarded because
            they needed to be fragmented but could not be.  This
            includes IPv4 packets that have the DF bit set and IPv6
            packets that are being forwarded and exceed the outgoing
            link MTU.

            When tracking interface statistics, the counter of the
            outgoing interface is incremented for an unsuccessfully
            fragmented datagram.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 28 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsOutFragFails ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsOutFragFails" << endl;
  return 0;
}


/* 
ipSystemStatsOutFragCreates OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of output datagram fragments that have been
            generated as a result of IP fragmentation.

            When tracking interface statistics, the counter of the
            outgoing interface is incremented for a successfully
            fragmented datagram.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 29 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsOutFragCreates ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsOutFragCreates" << endl;
  return 0;
}


/* 
ipSystemStatsOutTransmits OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of IP datagrams that this entity supplied
            to the lower layers for transmission.  This includes
            datagrams generated locally and those forwarded by this
            entity.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 30 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsOutTransmits ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsOutTransmits" << endl;
  return 0;
}


/* 
ipSystemStatsHCOutTransmits OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of IP datagrams that this entity supplied
            to the lower layers for transmission.  This object counts
            the same datagrams as ipSystemStatsOutTransmits, but allows
            for larger values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 31 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsHCOutTransmits (BcmCounter64 &ipSystemStatsHCOutTransmits)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsHCOutTransmits" << endl;
  ipSystemStatsHCOutTransmits = Value;

  return ipSystemStatsHCOutTransmits.Low32();
}


/* 
ipSystemStatsOutOctets OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of octets in IP datagrams delivered to the
            lower layers for transmission.  Octets from datagrams
            counted in ipSystemStatsOutTransmits MUST be counted here.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 32 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsOutOctets ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsOutOctets" << endl;
  return 0;
}


/* 
ipSystemStatsHCOutOctets OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of octets in IP datagrams delivered to the
            lower layers for transmission.  This objects counts the same
            octets as ipSystemStatsOutOctets, but allows for larger
            values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 33 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsHCOutOctets (BcmCounter64 &ipSystemStatsHCOutOctets)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsHCOutOctets" << endl;
  ipSystemStatsHCOutOctets = Value;

  return ipSystemStatsHCOutOctets.Low32();
}


/* 
ipSystemStatsInMcastPkts OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP multicast datagrams received.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 34 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsInMcastPkts ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsInMcastPkts" << endl;
  return 0;
}


/* 
ipSystemStatsHCInMcastPkts OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP multicast datagrams received.  This object
            counts the same datagrams as ipSystemStatsInMcastPkts but
            allows for larger values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 35 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsHCInMcastPkts (BcmCounter64 &ipSystemStatsHCInMcastPkts)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsHCInMcastPkts" << endl;
  ipSystemStatsHCInMcastPkts = Value;

  return ipSystemStatsHCInMcastPkts.Low32();
}


/* 
ipSystemStatsInMcastOctets OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of octets received in IP multicast
            datagrams.  Octets from datagrams counted in
            ipSystemStatsInMcastPkts MUST be counted here.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 36 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsInMcastOctets ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsInMcastOctets" << endl;
  return 0;
}


/* 
ipSystemStatsHCInMcastOctets OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of octets received in IP multicast
            datagrams.  This object counts the same octets as
            ipSystemStatsInMcastOctets, but allows for larger values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 37 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsHCInMcastOctets (BcmCounter64 &ipSystemStatsHCInMcastOctets)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsHCInMcastOctets" << endl;
  ipSystemStatsHCInMcastOctets = Value;

  return ipSystemStatsHCInMcastOctets.Low32();
}


/* 
ipSystemStatsOutMcastPkts OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP multicast datagrams transmitted.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 38 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsOutMcastPkts ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsOutMcastPkts" << endl;
  return 0;
}


/* 
ipSystemStatsHCOutMcastPkts OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP multicast datagrams transmitted.  This
            object counts the same datagrams as
            ipSystemStatsOutMcastPkts, but allows for larger values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 39 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsHCOutMcastPkts (BcmCounter64 &ipSystemStatsHCOutMcastPkts)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsHCOutMcastPkts" << endl;
  ipSystemStatsHCOutMcastPkts = Value;

  return ipSystemStatsHCOutMcastPkts.Low32();
}


/* 
ipSystemStatsOutMcastOctets OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of octets transmitted in IP multicast
            datagrams.  Octets from datagrams counted in
            ipSystemStatsOutMcastPkts MUST be counted here.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 40 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsOutMcastOctets ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsOutMcastOctets" << endl;
  return 0;
}


/* 
ipSystemStatsHCOutMcastOctets OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of octets transmitted in IP multicast
            datagrams.  This object counts the same octets as
            ipSystemStatsOutMcastOctets, but allows for larger values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 41 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsHCOutMcastOctets (BcmCounter64 &ipSystemStatsHCOutMcastOctets)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsHCOutMcastOctets" << endl;
  ipSystemStatsHCOutMcastOctets = Value;

  return ipSystemStatsHCOutMcastOctets.Low32();
}


/* 
ipSystemStatsInBcastPkts OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP broadcast datagrams received.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 42 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsInBcastPkts ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsInBcastPkts" << endl;
  return 0;
}


/* 
ipSystemStatsHCInBcastPkts OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP broadcast datagrams received.  This object
            counts the same datagrams as ipSystemStatsInBcastPkts but
            allows for larger values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 43 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsHCInBcastPkts (BcmCounter64 &ipSystemStatsHCInBcastPkts)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsHCInBcastPkts" << endl;
  ipSystemStatsHCInBcastPkts = Value;

  return ipSystemStatsHCInBcastPkts.Low32();
}


/* 
ipSystemStatsOutBcastPkts OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP broadcast datagrams transmitted.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 44 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsOutBcastPkts ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsOutBcastPkts" << endl;
  return 0;
}


/* 
ipSystemStatsHCOutBcastPkts OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP broadcast datagrams transmitted.  This
            object counts the same datagrams as
            ipSystemStatsOutBcastPkts, but allows for larger values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipSystemStatsDiscontinuityTime."
    ::= { ipSystemStatsEntry 45 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsHCOutBcastPkts (BcmCounter64 &ipSystemStatsHCOutBcastPkts)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsHCOutBcastPkts" << endl;
  ipSystemStatsHCOutBcastPkts = Value;

  return ipSystemStatsHCOutBcastPkts.Low32();
}


/* 
ipSystemStatsDiscontinuityTime OBJECT-TYPE
    SYNTAX     TimeStamp
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The value of sysUpTime on the most recent occasion at which
            any one or more of this entry's counters suffered a
            discontinuity.

            If no such discontinuities have occurred since the last re-
            initialization of the local management subsystem, then this
            object contains a zero value."
    ::= { ipSystemStatsEntry 46 }
*/ 
unsigned long ipSystemStatsEntryBridge::Get_ipSystemStatsDiscontinuityTime ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsDiscontinuityTime" << endl;
  return 0;
}


/* 
ipSystemStatsRefreshRate OBJECT-TYPE
    SYNTAX     Unsigned32
    UNITS      "milli-seconds"
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The minimum reasonable polling interval for this entry.
            This object provides an indication of the minimum amount of
            time required to update the counters in this entry."
    ::= { ipSystemStatsEntry 47 }
*/ 
unsigned int ipSystemStatsEntryBridge::Get_ipSystemStatsRefreshRate ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipSystemStatsRefreshRate" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ipIfStatsEntryBridge for bridging elements in the ipIfStatsTable
 *
\*/

/* 
ipIfStatsTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF IpIfStatsEntry
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The table containing per-interface traffic statistics.  This
            table and the ipSystemStatsTable contain similar objects
            whose difference is in their granularity.  Where this table
            contains per-interface statistics, the ipSystemStatsTable
            contains the same statistics, but counted on a system wide
            basis."
    ::= { ipTrafficStats 3 }

INDEX { ipIfStatsIPVersion, ipIfStatsIfIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ipIfStatsEntryBridge::ipIfStatsEntryBridge (int ipIfStatsIPVersion, int ipIfStatsIfIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ipIfStatsTable,
                      BcmObjectId(ipIfStatsIPVersion) + BcmObjectId(ipIfStatsIfIndex),
                      pAgent)
{
}

ipIfStatsEntryBridge::~ipIfStatsEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ipIfStatsIPVersion OBJECT-TYPE
    SYNTAX     InetVersion
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The IP version of this row."
    ::= { ipIfStatsEntry 1 }
*/ 
int ipIfStatsEntryBridge::Get_ipIfStatsIPVersion ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsIPVersion" << endl;
  return 0;
}


/* 
ipIfStatsIfIndex OBJECT-TYPE
    SYNTAX     InterfaceIndex
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The index value that uniquely identifies the interface to
            which this entry is applicable.  The interface identified by
            a particular value of this index is the same interface as
            identified by the same value of the IF-MIB's ifIndex."
    ::= { ipIfStatsEntry 2 }
*/ 
int ipIfStatsEntryBridge::Get_ipIfStatsIfIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsIfIndex" << endl;
  return 0;
}


/* 
ipIfStatsInReceives OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of input IP datagrams received, including
            those received in error.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 3 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsInReceives ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsInReceives" << endl;
  return 0;
}


/* 
ipIfStatsHCInReceives OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of input IP datagrams received, including
            those received in error.  This object counts the same
            datagrams as ipIfStatsInReceives, but allows for larger
            values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 4 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsHCInReceives (BcmCounter64 &ipIfStatsHCInReceives)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsHCInReceives" << endl;
  ipIfStatsHCInReceives = Value;

  return ipIfStatsHCInReceives.Low32();
}


/* 
ipIfStatsInOctets OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of octets received in input IP datagrams,
            including those received in error.  Octets from datagrams
            counted in ipIfStatsInReceives MUST be counted here.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 5 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsInOctets ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsInOctets" << endl;
  return 0;
}


/* 
ipIfStatsHCInOctets OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of octets received in input IP datagrams,
            including those received in error.  This object counts the
            same octets as ipIfStatsInOctets, but allows for larger
            values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 6 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsHCInOctets (BcmCounter64 &ipIfStatsHCInOctets)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsHCInOctets" << endl;
  ipIfStatsHCInOctets = Value;

  return ipIfStatsHCInOctets.Low32();
}


/* 
ipIfStatsInHdrErrors OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of input IP datagrams discarded due to errors in
            their IP headers, including version number mismatch, other
            format errors, hop count exceeded, errors discovered in
            processing their IP options, etc.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 7 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsInHdrErrors ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsInHdrErrors" << endl;
  return 0;
}


/* 
ipIfStatsInNoRoutes OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of input IP datagrams discarded because no route
            could be found to transmit them to their destination.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 8 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsInNoRoutes ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsInNoRoutes" << endl;
  return 0;
}


/* 
ipIfStatsInAddrErrors OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of input IP datagrams discarded because the IP
            address in their IP header's destination field was not a
            valid address to be received at this entity.  This count
            includes invalid addresses (e.g., ::0).  For entities that
            are not IP routers and therefore do not forward datagrams,
            this counter includes datagrams discarded because the
            destination address was not a local address.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 9 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsInAddrErrors ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsInAddrErrors" << endl;
  return 0;
}


/* 
ipIfStatsInUnknownProtos OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of locally-addressed IP datagrams received
            successfully but discarded because of an unknown or
            unsupported protocol.

            When tracking interface statistics, the counter of the
            interface to which these datagrams were addressed is
            incremented.  This interface might not be the same as the
            input interface for some of the datagrams.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 10 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsInUnknownProtos ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsInUnknownProtos" << endl;
  return 0;
}


/* 
ipIfStatsInTruncatedPkts OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of input IP datagrams discarded because the
            datagram frame didn't carry enough data.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 11 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsInTruncatedPkts ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsInTruncatedPkts" << endl;
  return 0;
}


/* 
ipIfStatsInForwDatagrams OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of input datagrams for which this entity was not
            their final IP destination and for which this entity
            attempted to find a route to forward them to that final
            destination.  In entities that do not act as IP routers,
            this counter will include only those datagrams that were
            Source-Routed via this entity, and the Source-Route
            processing was successful.

            When tracking interface statistics, the counter of the
            incoming interface is incremented for each datagram.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 12 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsInForwDatagrams ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsInForwDatagrams" << endl;
  return 0;
}


/* 
ipIfStatsHCInForwDatagrams OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of input datagrams for which this entity was not
            their final IP destination and for which this entity
            attempted to find a route to forward them to that final
            destination.  This object counts the same packets as
            ipIfStatsInForwDatagrams, but allows for larger values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 13 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsHCInForwDatagrams (BcmCounter64 &ipIfStatsHCInForwDatagrams)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsHCInForwDatagrams" << endl;
  ipIfStatsHCInForwDatagrams = Value;

  return ipIfStatsHCInForwDatagrams.Low32();
}


/* 
ipIfStatsReasmReqds OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP fragments received that needed to be
            reassembled at this interface.

            When tracking interface statistics, the counter of the
            interface to which these fragments were addressed is
            incremented.  This interface might not be the same as the
            input interface for some of the fragments.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 14 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsReasmReqds ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsReasmReqds" << endl;
  return 0;
}


/* 
ipIfStatsReasmOKs OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP datagrams successfully reassembled.

            When tracking interface statistics, the counter of the
            interface to which these datagrams were addressed is
            incremented.  This interface might not be the same as the
            input interface for some of the datagrams.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 15 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsReasmOKs ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsReasmOKs" << endl;
  return 0;
}


/* 
ipIfStatsReasmFails OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of failures detected by the IP re-assembly
            algorithm (for whatever reason: timed out, errors, etc.).
            Note that this is not necessarily a count of discarded IP
            fragments since some algorithms (notably the algorithm in
            RFC 815) can lose track of the number of fragments by
            combining them as they are received.

            When tracking interface statistics, the counter of the
            interface to which these fragments were addressed is
            incremented.  This interface might not be the same as the
            input interface for some of the fragments.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 16 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsReasmFails ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsReasmFails" << endl;
  return 0;
}


/* 
ipIfStatsInDiscards OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of input IP datagrams for which no problems were
            encountered to prevent their continued processing, but
            were discarded (e.g., for lack of buffer space).  Note that
            this counter does not include any datagrams discarded while
            awaiting re-assembly.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 17 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsInDiscards ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsInDiscards" << endl;
  return 0;
}


/* 
ipIfStatsInDelivers OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of datagrams successfully delivered to IP
            user-protocols (including ICMP).

            When tracking interface statistics, the counter of the
            interface to which these datagrams were addressed is
            incremented.  This interface might not be the same as the
            input interface for some of the datagrams.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 18 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsInDelivers ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsInDelivers" << endl;
  return 0;
}


/* 
ipIfStatsHCInDelivers OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of datagrams successfully delivered to IP
            user-protocols (including ICMP).  This object counts the
            same packets as ipIfStatsInDelivers, but allows for larger
            values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 19 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsHCInDelivers (BcmCounter64 &ipIfStatsHCInDelivers)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsHCInDelivers" << endl;
  ipIfStatsHCInDelivers = Value;

  return ipIfStatsHCInDelivers.Low32();
}


/* 
ipIfStatsOutRequests OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of IP datagrams that local IP user-
            protocols (including ICMP) supplied to IP in requests for
            transmission.  Note that this counter does not include any
            datagrams counted in ipIfStatsOutForwDatagrams.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 20 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsOutRequests ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsOutRequests" << endl;
  return 0;
}


/* 
ipIfStatsHCOutRequests OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of IP datagrams that local IP user-
            protocols (including ICMP) supplied to IP in requests for
            transmission.  This object counts the same packets as
            ipIfStatsOutRequests, but allows for larger values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 21 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsHCOutRequests (BcmCounter64 &ipIfStatsHCOutRequests)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsHCOutRequests" << endl;
  ipIfStatsHCOutRequests = Value;

  return ipIfStatsHCOutRequests.Low32();
}


/* 
ipIfStatsOutForwDatagrams OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of datagrams for which this entity was not their
            final IP destination and for which it was successful in
            finding a path to their final destination.  In entities
            that do not act as IP routers, this counter will include
            only those datagrams that were Source-Routed via this
            entity, and the Source-Route processing was successful.

            When tracking interface statistics, the counter of the
            outgoing interface is incremented for a successfully
            forwarded datagram.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 23 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsOutForwDatagrams ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsOutForwDatagrams" << endl;
  return 0;
}


/* 
ipIfStatsHCOutForwDatagrams OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of datagrams for which this entity was not their
            final IP destination and for which it was successful in
            finding a path to their final destination.  This object
            counts the same packets as ipIfStatsOutForwDatagrams, but
            allows for larger values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 24 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsHCOutForwDatagrams (BcmCounter64 &ipIfStatsHCOutForwDatagrams)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsHCOutForwDatagrams" << endl;
  ipIfStatsHCOutForwDatagrams = Value;

  return ipIfStatsHCOutForwDatagrams.Low32();
}


/* 
ipIfStatsOutDiscards OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of output IP datagrams for which no problem was
            encountered to prevent their transmission to their
            destination, but were discarded (e.g., for lack of
            buffer space).  Note that this counter would include
            datagrams counted in ipIfStatsOutForwDatagrams if any such
            datagrams met this (discretionary) discard criterion.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 25 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsOutDiscards ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsOutDiscards" << endl;
  return 0;
}


/* 
ipIfStatsOutFragReqds OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP datagrams that would require fragmentation
            in order to be transmitted.

            When tracking interface statistics, the counter of the
            outgoing interface is incremented for a successfully
            fragmented datagram.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 26 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsOutFragReqds ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsOutFragReqds" << endl;
  return 0;
}


/* 
ipIfStatsOutFragOKs OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP datagrams that have been successfully
            fragmented.

            When tracking interface statistics, the counter of the
            outgoing interface is incremented for a successfully
            fragmented datagram.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 27 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsOutFragOKs ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsOutFragOKs" << endl;
  return 0;
}


/* 
ipIfStatsOutFragFails OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP datagrams that have been discarded because
            they needed to be fragmented but could not be.  This
            includes IPv4 packets that have the DF bit set and IPv6
            packets that are being forwarded and exceed the outgoing
            link MTU.

            When tracking interface statistics, the counter of the
            outgoing interface is incremented for an unsuccessfully
            fragmented datagram.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 28 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsOutFragFails ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsOutFragFails" << endl;
  return 0;
}


/* 
ipIfStatsOutFragCreates OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of output datagram fragments that have been
            generated as a result of IP fragmentation.

            When tracking interface statistics, the counter of the
            outgoing interface is incremented for a successfully
            fragmented datagram.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 29 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsOutFragCreates ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsOutFragCreates" << endl;
  return 0;
}


/* 
ipIfStatsOutTransmits OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of IP datagrams that this entity supplied
            to the lower layers for transmission.  This includes
            datagrams generated locally and those forwarded by this
            entity.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 30 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsOutTransmits ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsOutTransmits" << endl;
  return 0;
}


/* 
ipIfStatsHCOutTransmits OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of IP datagrams that this entity supplied
            to the lower layers for transmission.  This object counts
            the same datagrams as ipIfStatsOutTransmits, but allows for
            larger values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 31 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsHCOutTransmits (BcmCounter64 &ipIfStatsHCOutTransmits)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsHCOutTransmits" << endl;
  ipIfStatsHCOutTransmits = Value;

  return ipIfStatsHCOutTransmits.Low32();
}


/* 
ipIfStatsOutOctets OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of octets in IP datagrams delivered to the
            lower layers for transmission.  Octets from datagrams
            counted in ipIfStatsOutTransmits MUST be counted here.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 32 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsOutOctets ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsOutOctets" << endl;
  return 0;
}


/* 
ipIfStatsHCOutOctets OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of octets in IP datagrams delivered to the
            lower layers for transmission.  This objects counts the same
            octets as ipIfStatsOutOctets, but allows for larger values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 33 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsHCOutOctets (BcmCounter64 &ipIfStatsHCOutOctets)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsHCOutOctets" << endl;
  ipIfStatsHCOutOctets = Value;

  return ipIfStatsHCOutOctets.Low32();
}


/* 
ipIfStatsInMcastPkts OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP multicast datagrams received.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 34 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsInMcastPkts ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsInMcastPkts" << endl;
  return 0;
}


/* 
ipIfStatsHCInMcastPkts OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP multicast datagrams received.  This object
            counts the same datagrams as ipIfStatsInMcastPkts, but
            allows for larger values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 35 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsHCInMcastPkts (BcmCounter64 &ipIfStatsHCInMcastPkts)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsHCInMcastPkts" << endl;
  ipIfStatsHCInMcastPkts = Value;

  return ipIfStatsHCInMcastPkts.Low32();
}


/* 
ipIfStatsInMcastOctets OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of octets received in IP multicast
            datagrams.  Octets from datagrams counted in
            ipIfStatsInMcastPkts MUST be counted here.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 36 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsInMcastOctets ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsInMcastOctets" << endl;
  return 0;
}


/* 
ipIfStatsHCInMcastOctets OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of octets received in IP multicast
            datagrams.  This object counts the same octets as
            ipIfStatsInMcastOctets, but allows for larger values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 37 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsHCInMcastOctets (BcmCounter64 &ipIfStatsHCInMcastOctets)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsHCInMcastOctets" << endl;
  ipIfStatsHCInMcastOctets = Value;

  return ipIfStatsHCInMcastOctets.Low32();
}


/* 
ipIfStatsOutMcastPkts OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP multicast datagrams transmitted.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 38 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsOutMcastPkts ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsOutMcastPkts" << endl;
  return 0;
}


/* 
ipIfStatsHCOutMcastPkts OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP multicast datagrams transmitted.  This
            object counts the same datagrams as ipIfStatsOutMcastPkts,
            but allows for larger values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 39 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsHCOutMcastPkts (BcmCounter64 &ipIfStatsHCOutMcastPkts)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsHCOutMcastPkts" << endl;
  ipIfStatsHCOutMcastPkts = Value;

  return ipIfStatsHCOutMcastPkts.Low32();
}


/* 
ipIfStatsOutMcastOctets OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of octets transmitted in IP multicast
            datagrams.  Octets from datagrams counted in
            ipIfStatsOutMcastPkts MUST be counted here.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 40 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsOutMcastOctets ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsOutMcastOctets" << endl;
  return 0;
}


/* 
ipIfStatsHCOutMcastOctets OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of octets transmitted in IP multicast
            datagrams.  This object counts the same octets as
            ipIfStatsOutMcastOctets, but allows for larger values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 41 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsHCOutMcastOctets (BcmCounter64 &ipIfStatsHCOutMcastOctets)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsHCOutMcastOctets" << endl;
  ipIfStatsHCOutMcastOctets = Value;

  return ipIfStatsHCOutMcastOctets.Low32();
}


/* 
ipIfStatsInBcastPkts OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP broadcast datagrams received.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 42 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsInBcastPkts ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsInBcastPkts" << endl;
  return 0;
}


/* 
ipIfStatsHCInBcastPkts OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP broadcast datagrams received.  This object
            counts the same datagrams as ipIfStatsInBcastPkts, but
            allows for larger values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 43 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsHCInBcastPkts (BcmCounter64 &ipIfStatsHCInBcastPkts)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsHCInBcastPkts" << endl;
  ipIfStatsHCInBcastPkts = Value;

  return ipIfStatsHCInBcastPkts.Low32();
}


/* 
ipIfStatsOutBcastPkts OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP broadcast datagrams transmitted.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 44 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsOutBcastPkts ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsOutBcastPkts" << endl;
  return 0;
}


/* 
ipIfStatsHCOutBcastPkts OBJECT-TYPE
    SYNTAX     Counter64
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of IP broadcast datagrams transmitted.  This
            object counts the same datagrams as ipIfStatsOutBcastPkts,
            but allows for larger values.

            Discontinuities in the value of this counter can occur at
            re-initialization of the management system, and at other
            times as indicated by the value of
            ipIfStatsDiscontinuityTime."
    ::= { ipIfStatsEntry 45 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsHCOutBcastPkts (BcmCounter64 &ipIfStatsHCOutBcastPkts)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsHCOutBcastPkts" << endl;
  ipIfStatsHCOutBcastPkts = Value;

  return ipIfStatsHCOutBcastPkts.Low32();
}


/* 
ipIfStatsDiscontinuityTime OBJECT-TYPE
    SYNTAX     TimeStamp
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The value of sysUpTime on the most recent occasion at which
            any one or more of this entry's counters suffered a
            discontinuity.

            If no such discontinuities have occurred since the last re-
            initialization of the local management subsystem, then this
            object contains a zero value."
    ::= { ipIfStatsEntry 46 }
*/ 
unsigned long ipIfStatsEntryBridge::Get_ipIfStatsDiscontinuityTime ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsDiscontinuityTime" << endl;
  return 0;
}


/* 
ipIfStatsRefreshRate OBJECT-TYPE
    SYNTAX     Unsigned32
    UNITS "milli-seconds"
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The minimum reasonable polling interval for this entry.
            This object provides an indication of the minimum amount of
            time required to update the counters in this entry."
    ::= { ipIfStatsEntry 47 }
*/ 
unsigned int ipIfStatsEntryBridge::Get_ipIfStatsRefreshRate ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ipIfStatsRefreshRate" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/

#endif



/*\
 *
 *  icmpGroupBridge for bridging scalars from the icmpGroup group 
 *
\*/

/* 
icmp     OBJECT IDENTIFIER ::= { mib-2 5 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

icmpGroupBridge::icmpGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_icmpGroup, pAgent)
{
}

icmpGroupBridge::~icmpGroupBridge ()
{
}

SNMP_STATUS icmpGroupBridge::Register (CleanupMethod Cleanup, bool SilentFailure)
{
  SNMP_STATUS Status = ScalarMibBridge::Register (Cleanup, SilentFailure);
  if (Status != SNMP_NO_ERROR)
    return Status;
    
  /* Create entries in the icmpStatsTable and icmpMsgStatsTable */
  
  BcmMibBridge *pBridge = new icmpStatsEntryBridge (CONSTVAL_ICMPSTATSIPVERSION_IPV4, this, fpAgent);
  if (pBridge->Register (Cleanup, SilentFailure) != SNMP_NO_ERROR)
    delete pBridge;
        
  #if (BCM_IPV6_SUPPORT)
  pBridge = new icmpStatsEntryBridge (CONSTVAL_ICMPSTATSIPVERSION_IPV6, this, fpAgent);
  if (pBridge->Register (Cleanup, SilentFailure) != SNMP_NO_ERROR)
    delete pBridge;
  #endif

  // -- The table entries are created in the Update() method as necessary.

  return Status;
}

/*\ 
 *  GET methods
\*/

/* 
icmpInMsgs OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The total number of ICMP messages which the entity received.
            Note that this counter includes all those counted by
            icmpInErrors.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by
            icmpStatsInMsgs."
    ::= { icmp 1 }
*/ 
unsigned int icmpGroupBridge::Get_icmpInMsgs ()
{
  unsigned int icmpInMsgs;

  // Answer is the sum of all ICMP message types, including errors.
  // Note that the IcmpInMsgs method includes errors so we don't need to
  // explicitly add IcmpInErrors here.
  icmpInMsgs = IcmpInMsgs();
  
  return icmpInMsgs;
}


/* 
icmpInErrors OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP messages which the entity received but
            determined as having ICMP-specific errors (bad ICMP
            checksums, bad length, etc.).

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by
            icmpStatsInErrors."
    ::= { icmp 2 }
*/ 
unsigned int icmpGroupBridge::Get_icmpInErrors ()
{
  return IcmpInErrors();
}


/* 
icmpInDestUnreachs OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP Destination Unreachable messages
            received.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by a column in
            the icmpMsgStatsTable."
    ::= { icmp 3 }
*/ 
unsigned int icmpGroupBridge::Get_icmpInDestUnreachs ()
{
  return IcmpInMsgs(CONSTVAL_ICMPMSGTYPE_DESTUNREACH);
}


/* 
icmpInTimeExcds OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP Time Exceeded messages received.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by a column in
            the icmpMsgStatsTable."
    ::= { icmp 4 }
*/ 
unsigned int icmpGroupBridge::Get_icmpInTimeExcds ()
{
  return IcmpInMsgs(CONSTVAL_ICMPMSGTYPE_TIMEEXCD);
}


/* 
icmpInParmProbs OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP Parameter Problem messages received.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by a column in
            the icmpMsgStatsTable."
    ::= { icmp 5 }
*/ 
unsigned int icmpGroupBridge::Get_icmpInParmProbs ()
{
  return IcmpInMsgs(CONSTVAL_ICMPMSGTYPE_PARMPROB);
}


/* 
icmpInSrcQuenchs OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP Source Quench messages received.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by a column in
            the icmpMsgStatsTable."
    ::= { icmp 6 }
*/ 
unsigned int icmpGroupBridge::Get_icmpInSrcQuenchs ()
{
  return IcmpInMsgs(CONSTVAL_ICMPMSGTYPE_SRCQUENCH);
}


/* 
icmpInRedirects OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP Redirect messages received.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by a column in
            the icmpMsgStatsTable."
    ::= { icmp 7 }
*/ 
unsigned int icmpGroupBridge::Get_icmpInRedirects ()
{
  return IcmpInMsgs(CONSTVAL_ICMPMSGTYPE_REDIRECT);
}


/* 
icmpInEchos OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP Echo (request) messages received.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by a column in
            the icmpMsgStatsTable."
    ::= { icmp 8 }
*/ 
unsigned int icmpGroupBridge::Get_icmpInEchos ()
{
  return IcmpInMsgs(CONSTVAL_ICMPMSGTYPE_ECHO);
}


/* 
icmpInEchoReps OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP Echo Reply messages received.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by a column in
            the icmpMsgStatsTable."
    ::= { icmp 9 }
*/ 
unsigned int icmpGroupBridge::Get_icmpInEchoReps ()
{
  return IcmpInMsgs(CONSTVAL_ICMPMSGTYPE_ECHOREP);
}


/* 
icmpInTimestamps OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP Timestamp (request) messages received.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by a column in
            the icmpMsgStatsTable."
    ::= { icmp 10 }
*/ 
unsigned int icmpGroupBridge::Get_icmpInTimestamps ()
{
  return IcmpInMsgs(CONSTVAL_ICMPMSGTYPE_TIMESTAMP);
}


/* 
icmpInTimestampReps OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP Timestamp Reply messages received.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by a column in
            the icmpMsgStatsTable."
    ::= { icmp 11 }
*/ 
unsigned int icmpGroupBridge::Get_icmpInTimestampReps ()
{
  return IcmpInMsgs(CONSTVAL_ICMPMSGTYPE_TIMESTAMPREP);
}


/* 
icmpInAddrMasks OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP Address Mask Request messages received.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by a column in
            the icmpMsgStatsTable."
    ::= { icmp 12 }
*/ 
unsigned int icmpGroupBridge::Get_icmpInAddrMasks ()
{
  return IcmpInMsgs(CONSTVAL_ICMPMSGTYPE_ADDRMASK);
}


/* 
icmpInAddrMaskReps OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP Address Mask Reply messages received.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by a column in
            the icmpMsgStatsTable."
    ::= { icmp 13 }
*/ 
unsigned int icmpGroupBridge::Get_icmpInAddrMaskReps ()
{
  return IcmpInMsgs(CONSTVAL_ICMPMSGTYPE_ADDRMASKREP);
}


/* 
icmpOutMsgs OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The total number of ICMP messages which this entity
            attempted to send.  Note that this counter includes all
            those counted by icmpOutErrors.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by
            icmpStatsOutMsgs."
    ::= { icmp 14 }
*/ 
unsigned int icmpGroupBridge::Get_icmpOutMsgs ()
{
  unsigned int icmpOutMsgs;

  // Answer is the sum of all ICMP message types
  icmpOutMsgs = IcmpOutMsgs();
  
  return icmpOutMsgs;
}


/* 
icmpOutErrors OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP messages which this entity did not send
            due to problems discovered within ICMP, such as a lack of
            buffers.  This value should not include errors discovered
            outside the ICMP layer, such as the inability of IP to route
            the resultant datagram.  In some implementations, there may
            be no types of error which contribute to this counter's
            value.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by
            icmpStatsOutErrors."
    ::= { icmp 15 }
*/ 
unsigned int icmpGroupBridge::Get_icmpOutErrors ()
{
  return IcmpOutErrors();
}


/* 
icmpOutDestUnreachs OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP Destination Unreachable messages sent.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by a column in
            the icmpMsgStatsTable."
    ::= { icmp 16 }
*/ 
unsigned int icmpGroupBridge::Get_icmpOutDestUnreachs ()
{
  return IcmpOutMsgs(CONSTVAL_ICMPMSGTYPE_DESTUNREACH);
}


/* 
icmpOutTimeExcds OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP Time Exceeded messages sent.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by a column in
            the icmpMsgStatsTable."
    ::= { icmp 17 }
*/ 
unsigned int icmpGroupBridge::Get_icmpOutTimeExcds ()
{
  return IcmpOutMsgs(CONSTVAL_ICMPMSGTYPE_TIMEEXCD);
}


/* 
icmpOutParmProbs OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP Parameter Problem messages sent.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by a column in
            the icmpMsgStatsTable."
    ::= { icmp 18 }
*/ 
unsigned int icmpGroupBridge::Get_icmpOutParmProbs ()
{
  return IcmpOutMsgs(CONSTVAL_ICMPMSGTYPE_PARMPROB);
}


/* 
icmpOutSrcQuenchs OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP Source Quench messages sent.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by a column in
            the icmpMsgStatsTable."
    ::= { icmp 19 }
*/ 
unsigned int icmpGroupBridge::Get_icmpOutSrcQuenchs ()
{
  return IcmpOutMsgs(CONSTVAL_ICMPMSGTYPE_SRCQUENCH);
}


/* 
icmpOutRedirects OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP Redirect messages sent.  For a host, this
            object will always be zero, since hosts do not send
            redirects.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by a column in
            the icmpMsgStatsTable."
    ::= { icmp 20 }
*/ 
unsigned int icmpGroupBridge::Get_icmpOutRedirects ()
{
  return IcmpOutMsgs(CONSTVAL_ICMPMSGTYPE_REDIRECT);
}


/* 
icmpOutEchos OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP Echo (request) messages sent.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by a column in
            the icmpMsgStatsTable."
    ::= { icmp 21 }
*/ 
unsigned int icmpGroupBridge::Get_icmpOutEchos ()
{
  return IcmpOutMsgs(CONSTVAL_ICMPMSGTYPE_ECHO);
}


/* 
icmpOutEchoReps OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP Echo Reply messages sent.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by a column in
            the icmpMsgStatsTable."
    ::= { icmp 22 }
*/ 
unsigned int icmpGroupBridge::Get_icmpOutEchoReps ()
{
  return IcmpOutMsgs(CONSTVAL_ICMPMSGTYPE_ECHOREP);
}


/* 
icmpOutTimestamps OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP Timestamp (request) messages sent.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by a column in
            the icmpMsgStatsTable."
    ::= { icmp 23 }
*/ 
unsigned int icmpGroupBridge::Get_icmpOutTimestamps ()
{
  return IcmpOutMsgs(CONSTVAL_ICMPMSGTYPE_TIMESTAMP);
}


/* 
icmpOutTimestampReps OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP Timestamp Reply messages sent.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by a column in
            the icmpMsgStatsTable."
    ::= { icmp 24 }
*/ 
unsigned int icmpGroupBridge::Get_icmpOutTimestampReps ()
{
  return IcmpOutMsgs(CONSTVAL_ICMPMSGTYPE_TIMESTAMPREP);
}


/* 
icmpOutAddrMasks OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP Address Mask Request messages sent.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by a column in
            the icmpMsgStatsTable."
    ::= { icmp 25 }
*/ 
unsigned int icmpGroupBridge::Get_icmpOutAddrMasks ()
{
  return IcmpOutMsgs(CONSTVAL_ICMPMSGTYPE_ADDRMASK);
}


/* 
icmpOutAddrMaskReps OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The number of ICMP Address Mask Reply messages sent.

            This object has been deprecated, as a new IP version-neutral
            table has been added.  It is loosely replaced by a column in
            the icmpMsgStatsTable."
    ::= { icmp 26 }
*/ 
unsigned int icmpGroupBridge::Get_icmpOutAddrMaskReps ()
{
  return IcmpOutMsgs(CONSTVAL_ICMPMSGTYPE_ADDRMASKREP);
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  icmpStatsEntryBridge for bridging elements in the icmpStatsTable
 *
\*/

/* 
icmpStatsTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF IcmpStatsEntry
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The table of generic system-wide ICMP counters."
    ::= { icmp 29 }

INDEX { icmpStatsIPVersion }
*/ 

/*\ 
 *  construction / destruction
\*/ 

icmpStatsEntryBridge::icmpStatsEntryBridge (int icmpStatsIPVersion, icmpGroupBridge *pIcmpBridge, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_icmpStatsTable,
                      BcmObjectId(icmpStatsIPVersion),
                      pAgent)
{
  fpIcmpBridge = pIcmpBridge;
}

icmpStatsEntryBridge::~icmpStatsEntryBridge ()
{
  fpIcmpBridge = NULL;
}

/*\ 
 *  GET methods
\*/

/* 
icmpStatsIPVersion OBJECT-TYPE
    SYNTAX     InetVersion
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The IP version of the statistics."
    ::= { icmpStatsEntry 1 }
*/ 
int icmpStatsEntryBridge::Get_icmpStatsIPVersion ()
{
  BcmObjectId Index;
  GetIndex (Index);
  return Index[0];
}


/* 
icmpStatsInMsgs OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of ICMP messages that the entity received.
            Note that this counter includes all those counted by
            icmpStatsInErrors."
    ::= { icmpStatsEntry 2 }
*/ 
unsigned int icmpStatsEntryBridge::Get_icmpStatsInMsgs ()
{
  #if (BCM_IPV6_SUPPORT)
  if (Get_icmpStatsIPVersion() == CONSTVAL_ICMPSTATSIPVERSION_IPV6)
    return fpIcmpBridge->Icmp6InMsgs() + fpIcmpBridge->Icmp6InErrors();
  #endif
  
  return fpIcmpBridge->IcmpInMsgs();
}


/* 
icmpStatsInErrors OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of ICMP messages that the entity received but
            determined as having ICMP-specific errors (bad ICMP
            checksums, bad length, etc.)."
    ::= { icmpStatsEntry 3 }
*/ 
unsigned int icmpStatsEntryBridge::Get_icmpStatsInErrors ()
{
  #if (BCM_IPV6_SUPPORT)
  if (Get_icmpStatsIPVersion() == CONSTVAL_ICMPSTATSIPVERSION_IPV6)
    return fpIcmpBridge->Icmp6InErrors();
  #endif
  
  return fpIcmpBridge->IcmpInErrors();
}


/* 
icmpStatsOutMsgs OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of ICMP messages that the entity attempted
            to send.  Note that this counter includes all those counted
            by icmpStatsOutErrors."
    ::= { icmpStatsEntry 4 }
*/ 
unsigned int icmpStatsEntryBridge::Get_icmpStatsOutMsgs ()
{
  #if (BCM_IPV6_SUPPORT)
  if (Get_icmpStatsIPVersion() == CONSTVAL_ICMPSTATSIPVERSION_IPV6)
    return fpIcmpBridge->Icmp6OutMsgs();
  #endif
  
  return fpIcmpBridge->IcmpOutMsgs();
}


/* 
icmpStatsOutErrors OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of ICMP messages that this entity did not send
            due to problems discovered within ICMP, such as a lack of
            buffers.  This value should not include errors discovered
            outside the ICMP layer, such as the inability of IP to route
            the resultant datagram.  In some implementations, there may
            be no types of error that contribute to this counter's
            value."
    ::= { icmpStatsEntry 5 }
*/ 
unsigned int icmpStatsEntryBridge::Get_icmpStatsOutErrors ()
{
  #if (BCM_IPV6_SUPPORT)
  if (Get_icmpStatsIPVersion() == CONSTVAL_ICMPSTATSIPVERSION_IPV6)
    return fpIcmpBridge->Icmp6OutErrors();
  #endif
  
  return fpIcmpBridge->IcmpOutErrors();
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  icmpMsgStatsEntryBridge for bridging elements in the icmpMsgStatsTable
 *
\*/

/* 
icmpMsgStatsTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF IcmpMsgStatsEntry
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The table of system-wide per-version, per-message type ICMP
            counters."
    ::= { icmp 30 }

INDEX { icmpMsgStatsIPVersion, icmpMsgStatsType }
*/ 

/*\ 
 *  construction / destruction
\*/ 

icmpMsgStatsEntryBridge::icmpMsgStatsEntryBridge (int icmpMsgStatsIPVersion, int icmpMsgStatsType, icmpGroupBridge *pIcmpBridge, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_icmpMsgStatsTable,
                      BcmObjectId(icmpMsgStatsIPVersion) + BcmObjectId(icmpMsgStatsType),
                      pAgent)
{
  fpIcmpBridge = pIcmpBridge;
}

icmpMsgStatsEntryBridge::~icmpMsgStatsEntryBridge ()
{
  fpIcmpBridge = NULL;
}

/*\ 
 *  GET methods
\*/

/* 
icmpMsgStatsIPVersion OBJECT-TYPE
    SYNTAX     InetVersion
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The IP version of the statistics."
    ::= { icmpMsgStatsEntry 1 }
*/ 
int icmpMsgStatsEntryBridge::Get_icmpMsgStatsIPVersion ()
{
  BcmObjectId Index;
  GetIndex (Index);
  return Index[0];
}


/* 
icmpMsgStatsType OBJECT-TYPE
    SYNTAX     Integer32 (0..255)
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The ICMP type field of the message type being counted by
            this row.

            Note that ICMP message types are scoped by the address type
            in use."
    REFERENCE "http://www.iana.org/assignments/icmp-parameters and
               http://www.iana.org/assignments/icmpv6-parameters"
    ::= { icmpMsgStatsEntry 2 }
*/ 
int icmpMsgStatsEntryBridge::Get_icmpMsgStatsType ()
{
  BcmObjectId Index;
  GetIndex (Index);
  return Index[1];
}


/* 
icmpMsgStatsInPkts OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of input packets for this AF and type."
    ::= { icmpMsgStatsEntry 3 }
*/ 
unsigned int icmpMsgStatsEntryBridge::Get_icmpMsgStatsInPkts ()
{
  #if (BCM_IPV6_SUPPORT)
  if (Get_icmpMsgStatsIPVersion() == CONSTVAL_ICMPMSGSTATSIPVERSION_IPV6)
    return fpIcmpBridge->Icmp6InMsgs(Get_icmpMsgStatsType());
  #endif
  
  return fpIcmpBridge->IcmpInMsgs(Get_icmpMsgStatsType());
}


/* 
icmpMsgStatsOutPkts OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of output packets for this AF and type."
    ::= { icmpMsgStatsEntry 4 }
*/ 
unsigned int icmpMsgStatsEntryBridge::Get_icmpMsgStatsOutPkts ()
{
  #if (BCM_IPV6_SUPPORT)
  if (Get_icmpMsgStatsIPVersion() == CONSTVAL_ICMPMSGSTATSIPVERSION_IPV6)
    return fpIcmpBridge->Icmp6OutMsgs(Get_icmpMsgStatsType());
  #endif

  return fpIcmpBridge->IcmpOutMsgs(Get_icmpMsgStatsType());
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/



