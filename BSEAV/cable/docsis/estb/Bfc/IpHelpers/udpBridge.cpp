//**************************************************************************
//                                                                          
//    Copyright 2007 Broadcom Corporation                                   
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
//    Creation Date: Monday, April 16, 2007 at 15:13:40
//    Created by Broadcom BFC Mib Compiler BCMIBC.EXE version 3.6.0b
//    from input file udp-mib_rfc4113.mib
//
//**************************************************************************

#include "MibObjs.h"
#include "SnmpLog.h"
#include "udpBridge.h"


/******************************************************************************\

UDP-MIB DEFINITIONS ::= BEGIN

IMPORTS
    MODULE-IDENTITY, OBJECT-TYPE, Integer32, Counter32, Counter64,
    Unsigned32, IpAddress, mib-2       FROM SNMPv2-SMI
    MODULE-COMPLIANCE, OBJECT-GROUP    FROM SNMPv2-CONF
    InetAddress, InetAddressType,
    InetPortNumber                     FROM INET-ADDRESS-MIB;

udpMIB MODULE-IDENTITY
    LAST-UPDATED "200505200000Z"  -- May 20, 2005
    ORGANIZATION
           "IETF IPv6 Working Group
            http://www.ietf.org/html.charters/ipv6-charter.html"
    CONTACT-INFO
           "Bill Fenner (editor)

            AT&T Labs -- Research
            75 Willow Rd.
            Menlo Park, CA 94025

            Phone: +1 650 330-7893
            Email: <fenner@research.att.com>

            John Flick (editor)

            Hewlett-Packard Company
            8000 Foothills Blvd. M/S 5557
            Roseville, CA 95747

            Phone: +1 916 785 4018
            Email: <john.flick@hp.com>

            Send comments to <ipv6@ietf.org>"
    DESCRIPTION
           "The MIB module for managing UDP implementations.
            Copyright (C) The Internet Society (2005).  This
            version of this MIB module is part of RFC 4113;
            see the RFC itself for full legal notices."
    REVISION      "200505200000Z"  -- May 20, 2005
    DESCRIPTION
           "IP version neutral revision, incorporating the
            following revisions:

            - Added udpHCInDatagrams and udpHCOutDatagrams in order
              to provide high-capacity counters for fast networks.
            - Added text to the descriptions of all counter objects
              to indicate how discontinuities are detected.
            - Deprecated the IPv4-specific udpTable and replaced it
              with the version neutral udpEndpointTable.  This
              table includes support for connected UDP endpoints
              and support for identification of the operating
              system process associated with a UDP endpoint.
            - Deprecated the udpGroup and replaced it with object
              groups representing the current set of objects.
            - Deprecated udpMIBCompliance and replaced it with
              udpMIBCompliance2, which includes the compliance
              information for the new object groups.

            This version published as RFC 4113."
    REVISION      "199411010000Z"    -- November 1, 1994
    DESCRIPTION
           "Initial SMIv2 version, published as RFC 2013."
    REVISION      "199103310000Z"    -- March 31, 1991
    DESCRIPTION
           "The initial revision of this MIB module was part of
            MIB-II, published as RFC 1213."
    ::= { mib-2 50 }

-- the UDP group

udp      OBJECT IDENTIFIER ::= { mib-2 7 }


\******************************************************************************/




/*\
 *
 *  udpGroupBridge for bridging scalars from the udpGroup group 
 *
\*/

/* 
udp      OBJECT IDENTIFIER ::= { mib-2 7 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

udpGroupBridge::udpGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_udpGroup, pAgent)
{
}

udpGroupBridge::~udpGroupBridge ()
{
}

void udpGroupBridge::UpdateUdpTable ()
{
}

void udpGroupBridge::UpdateUdpEndpointTable ()
{
}

/*\ 
 *  GET methods
\*/

/* 
udpInDatagrams OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of UDP datagrams delivered to UDP
            users.

            Discontinuities in the value of this counter can occur
            at re-initialization of the management system, and at
            other times as indicated by discontinuities in the
            value of sysUpTime."
    ::= { udp 1 }
*/ 
unsigned int udpGroupBridge::Get_udpInDatagrams ()
{
  BcmCounter64 Value(0,0);

  // We can just return the low 32 bits of the 64 bit counter.
  Get_udpHCInDatagrams (Value);

  return Value.Low32();
}


/* 
udpNoPorts OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of received UDP datagrams for which
            there was no application at the destination port.

            Discontinuities in the value of this counter can occur
            at re-initialization of the management system, and at
            other times as indicated by discontinuities in the
            value of sysUpTime."
    ::= { udp 2 }
*/ 
unsigned int udpGroupBridge::Get_udpNoPorts ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_udpNoPorts" << endl;
  return 0;
}


/* 
udpInErrors OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The number of received UDP datagrams that could not be
            delivered for reasons other than the lack of an
            application at the destination port.

            Discontinuities in the value of this counter can occur
            at re-initialization of the management system, and at
            other times as indicated by discontinuities in the
            value of sysUpTime."
    ::= { udp 3 }
*/ 
unsigned int udpGroupBridge::Get_udpInErrors ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_udpInErrors" << endl;
  return 0;
}


/* 
udpOutDatagrams OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The total number of UDP datagrams sent from this
            entity.

            Discontinuities in the value of this counter can occur
            at re-initialization of the management system, and at
            other times as indicated by discontinuities in the
            value of sysUpTime."
    ::= { udp 4 }
*/ 
unsigned int udpGroupBridge::Get_udpOutDatagrams ()
{
  BcmCounter64 Value(0,0);

  // We can just return the low 32 bits of the 64 bit counter.
  Get_udpHCOutDatagrams (Value);

  return Value.Low32();
}


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
unsigned int udpGroupBridge::Get_udpHCInDatagrams (BcmCounter64 &udpHCInDatagrams)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED:  Return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_udpHCInDatagrams" << endl;
  udpHCInDatagrams = Value;

  return udpHCInDatagrams.Low32();
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
unsigned int udpGroupBridge::Get_udpHCOutDatagrams (BcmCounter64 &udpHCOutDatagrams)
{
  BcmCounter64 Value(0,0);

  // UNFINISHED:  Return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_udpHCOutDatagrams" << endl;
  udpHCOutDatagrams = Value;

  return udpHCOutDatagrams.Low32();
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  udpEndpointEntryBridge for bridging elements in the udpEndpointTable
 *
\*/

/* 
udpEndpointTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF UdpEndpointEntry
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "A table containing information about this entity's UDP
            endpoints on which a local application is currently
            accepting or sending datagrams.

            The address type in this table represents the address
            type used for the communication, irrespective of the
            higher-layer abstraction.  For example, an application
            using IPv6 'sockets' to communicate via IPv4 between
            ::ffff:10.0.0.1 and ::ffff:10.0.0.2 would use
            InetAddressType ipv4(1).

            Unlike the udpTable in RFC 2013, this table also allows
            the representation of an application that completely
            specifies both local and remote addresses and ports.  A
            listening application is represented in three possible
            ways:

            1) An application that is willing to accept both IPv4
               and IPv6 datagrams is represented by a
               udpEndpointLocalAddressType of unknown(0) and a
               udpEndpointLocalAddress of ''h (a zero-length
               octet-string).

            2) An application that is willing to accept only IPv4
               or only IPv6 datagrams is represented by a
               udpEndpointLocalAddressType of the appropriate
               address type and a udpEndpointLocalAddress of
               '0.0.0.0' or '::' respectively.

            3) An application that is listening for datagrams only
               for a specific IP address but from any remote
               system is represented by a
               udpEndpointLocalAddressType of the appropriate
               address type, with udpEndpointLocalAddress
               specifying the local address.

            In all cases where the remote is a wildcard, the
            udpEndpointRemoteAddressType is unknown(0), the
            udpEndpointRemoteAddress is ''h (a zero-length
            octet-string), and the udpEndpointRemotePort is 0.

            If the operating system is demultiplexing UDP packets
            by remote address and port, or if the application has
            'connected' the socket specifying a default remote
            address and port, the udpEndpointRemote* values should
            be used to reflect this."
    ::= { udp 7 }

INDEX { udpEndpointLocalAddressType, udpEndpointLocalAddress, udpEndpointLocalPort, udpEndpointRemoteAddressType, udpEndpointRemoteAddress, udpEndpointRemotePort, udpEndpointInstance }
*/ 

/*\ 
 *  construction / destruction
\*/ 

udpEndpointEntryBridge::udpEndpointEntryBridge (BcmSnmpAgent *pAgent, const BcmIpAddress &udpEndpointLocalAddress, unsigned long udpEndpointLocalPort, const BcmIpAddress &udpEndpointRemoteAddress, unsigned long udpEndpointRemotePort, unsigned long udpEndpointInstance)
  : TabularMibBridge (kOID_udpEndpointTable,
                      // Index will be assigned below...
                      BcmObjectId(),
                      pAgent)
{
  fLocalAddr = udpEndpointLocalAddress;
  fLocalPort = udpEndpointLocalPort;
  fRemoteAddr = udpEndpointRemoteAddress;
  fRemotePort = udpEndpointRemotePort;
  fInstance = udpEndpointInstance;
  
  BcmObjectId Index;
  
  /* "Local" part of index */
  
  // Local address type
  Index = Get_udpEndpointLocalAddressType();
  
  // Length of local address: 0, 4, or 16 depending on type
  if (Get_udpEndpointLocalAddressType() == CONSTVAL_UDPENDPOINTLOCALADDRESSTYPE_UNKNOWN)
    Index += (unsigned long) CONSTLEN_UDPENDPOINTLOCALADDRESS_UNKNOWN;
  else
  {
    if (Get_udpEndpointLocalAddressType() == CONSTVAL_UDPENDPOINTLOCALADDRESSTYPE_IPV4)
      Index += (unsigned long) CONSTLEN_UDPENDPOINTLOCALADDRESS_IPV4;
    if (Get_udpEndpointLocalAddressType() == CONSTVAL_UDPENDPOINTLOCALADDRESSTYPE_IPV6)
      Index += (unsigned long) CONSTLEN_UDPENDPOINTLOCALADDRESS_IPV6;
      
    // Local address only included for IPv4 or IPv6.  Unknown is empty string.
    Index += udpEndpointLocalAddress;
  }
  
  // Local port
  Index += udpEndpointLocalPort;
  
  
  /* "Remote" part of index */
  
  // Remote address type
  Index += Get_udpEndpointRemoteAddressType();
  
  // Length of remote address: 0, 4, or 16 depending on type
  if (Get_udpEndpointRemoteAddressType() == CONSTVAL_UDPENDPOINTREMOTEADDRESSTYPE_UNKNOWN)
    Index += (unsigned long) CONSTLEN_UDPENDPOINTREMOTEADDRESS_UNKNOWN;
  else
  {
    if (Get_udpEndpointRemoteAddressType() == CONSTVAL_UDPENDPOINTREMOTEADDRESSTYPE_IPV4)
      Index += (unsigned long) CONSTLEN_UDPENDPOINTREMOTEADDRESS_IPV4;
    if (Get_udpEndpointRemoteAddressType() == CONSTVAL_UDPENDPOINTREMOTEADDRESSTYPE_IPV6)
      Index += (unsigned long) CONSTLEN_UDPENDPOINTREMOTEADDRESS_IPV6;
      
    // Remote address only included for IPv4 or IPv6.  Unknown is empty string.
    Index += udpEndpointRemoteAddress;
  }
  Index += udpEndpointRemotePort;
  
  /* Instance */
  
  Index += udpEndpointInstance;
  
  SetIndex (Index);
}

udpEndpointEntryBridge::~udpEndpointEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
udpEndpointLocalAddressType OBJECT-TYPE
    SYNTAX     InetAddressType
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The address type of udpEndpointLocalAddress.  Only
            IPv4, IPv4z, IPv6, and IPv6z addresses are expected, or
            unknown(0) if datagrams for all local IP addresses are
            accepted."
    ::= { udpEndpointEntry 1 }
*/ 
int udpEndpointEntryBridge::Get_udpEndpointLocalAddressType ()
{
  if (fLocalAddr.IsIpV6Address() == true)
  {
    if (fLocalAddr != kIPv6UnspecifiedAddress)
      return CONSTVAL_UDPENDPOINTLOCALADDRESSTYPE_IPV6;
    else
      return CONSTVAL_UDPENDPOINTLOCALADDRESSTYPE_UNKNOWN;
  }

  return CONSTVAL_UDPENDPOINTLOCALADDRESSTYPE_IPV4;
}


/* 
udpEndpointLocalAddress OBJECT-TYPE
    SYNTAX     InetAddress
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The local IP address for this UDP endpoint.

            The value of this object can be represented in three
            possible ways, depending on the characteristics of the
            listening application:

            1. For an application that is willing to accept both
               IPv4 and IPv6 datagrams, the value of this object
               must be ''h (a zero-length octet-string), with
               the value of the corresponding instance of the
               udpEndpointLocalAddressType object being unknown(0).

            2. For an application that is willing to accept only IPv4
               or only IPv6 datagrams, the value of this object
               must be '0.0.0.0' or '::', respectively, while the
               corresponding instance of the
               udpEndpointLocalAddressType object represents the
               appropriate address type.

            3. For an application that is listening for data
               destined only to a specific IP address, the value
               of this object is the specific IP address for which
               this node is receiving packets, with the
               corresponding instance of the
               udpEndpointLocalAddressType object representing the
               appropriate address type.

            As this object is used in the index for the
            udpEndpointTable, implementors of this table should be
            careful not to create entries that would result in OIDs
            with more than 128 subidentifiers; else the information
            cannot be accessed using SNMPv1, SNMPv2c, or SNMPv3."
    ::= { udpEndpointEntry 2 }
*/ 
int udpEndpointEntryBridge::Get_udpEndpointLocalAddress (BcmString &udpEndpointLocalAddress)
{
  if (Get_udpEndpointLocalAddressType() != CONSTVAL_UDPENDPOINTLOCALADDRESSTYPE_UNKNOWN)
    udpEndpointLocalAddress = fLocalAddr;

  return udpEndpointLocalAddress.length();
}


/* 
udpEndpointLocalPort OBJECT-TYPE
    SYNTAX     InetPortNumber
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The local port number for this UDP endpoint."
    ::= { udpEndpointEntry 3 }
*/ 
unsigned int udpEndpointEntryBridge::Get_udpEndpointLocalPort ()
{
  return fLocalPort;
}


/* 
udpEndpointRemoteAddressType OBJECT-TYPE
    SYNTAX     InetAddressType
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The address type of udpEndpointRemoteAddress.  Only
            IPv4, IPv4z, IPv6, and IPv6z addresses are expected, or
            unknown(0) if datagrams for all remote IP addresses are
            accepted.  Also, note that some combinations of
            udpEndpointLocalAdressType and
            udpEndpointRemoteAddressType are not supported.  In
            particular, if the value of this object is not
            unknown(0), it is expected to always refer to the
            same IP version as udpEndpointLocalAddressType."
    ::= { udpEndpointEntry 4 }
*/ 
int udpEndpointEntryBridge::Get_udpEndpointRemoteAddressType ()
{
  if (fRemoteAddr.IsIpV6Address() == true)
  {
    if (fRemoteAddr != kIPv6UnspecifiedAddress)
      return CONSTVAL_UDPENDPOINTLOCALADDRESSTYPE_IPV6;
    else
      return CONSTVAL_UDPENDPOINTLOCALADDRESSTYPE_UNKNOWN;
  }
  
  return CONSTVAL_UDPENDPOINTLOCALADDRESSTYPE_IPV4;
}


/* 
udpEndpointRemoteAddress OBJECT-TYPE
    SYNTAX     InetAddress
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The remote IP address for this UDP endpoint.  If
            datagrams from any remote system are to be accepted,
            this value is ''h (a zero-length octet-string).
            Otherwise, it has the type described by
            udpEndpointRemoteAddressType and is the address of the
            remote system from which datagrams are to be accepted
            (or to which all datagrams will be sent).

            As this object is used in the index for the
            udpEndpointTable, implementors of this table should be
            careful not to create entries that would result in OIDs
            with more than 128 subidentifiers; else the information
            cannot be accessed using SNMPv1, SNMPv2c, or SNMPv3."
    ::= { udpEndpointEntry 5 }
*/ 
int udpEndpointEntryBridge::Get_udpEndpointRemoteAddress (BcmString &udpEndpointRemoteAddress)
{
  if (Get_udpEndpointRemoteAddressType() != CONSTVAL_UDPENDPOINTREMOTEADDRESSTYPE_UNKNOWN)
    udpEndpointRemoteAddress = fRemoteAddr;

  return udpEndpointRemoteAddress.length();
}


/* 
udpEndpointRemotePort OBJECT-TYPE
    SYNTAX     InetPortNumber
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The remote port number for this UDP endpoint.  If
            datagrams from any remote system are to be accepted,
            this value is zero."
    ::= { udpEndpointEntry 6 }
*/ 
unsigned int udpEndpointEntryBridge::Get_udpEndpointRemotePort ()
{
  return fRemotePort;
}


/* 
udpEndpointInstance OBJECT-TYPE
    SYNTAX     Unsigned32 (1..'ffffffff'h)
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "The instance of this tuple.  This object is used to
            distinguish among multiple processes 'connected' to
            the same UDP endpoint.  For example, on a system
            implementing the BSD sockets interface, this would be
            used to support the SO_REUSEADDR and SO_REUSEPORT
            socket options."
    ::= { udpEndpointEntry 7 }
*/ 
unsigned int udpEndpointEntryBridge::Get_udpEndpointInstance ()
{
  return fInstance;
}


/* 
udpEndpointProcess OBJECT-TYPE
    SYNTAX     Unsigned32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The system's process ID for the process associated with
            this endpoint, or zero if there is no such process.
            This value is expected to be the same as
            HOST-RESOURCES-MIB::hrSWRunIndex or SYSAPPL-MIB::
            sysApplElmtRunIndex for some row in the appropriate
            tables."
    ::= { udpEndpointEntry 8 }
*/ 
unsigned int udpEndpointEntryBridge::Get_udpEndpointProcess ()
{
  // We have only one process, and we don't support the hrSWRunTable.
  // So return 0.
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
 *  udpEntryBridge for bridging elements in the udpTable
 *
\*/

/* 
udpTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF UdpEntry
    MAX-ACCESS not-accessible
    STATUS     deprecated
    DESCRIPTION
           "A table containing IPv4-specific UDP listener
            information.  It contains information about all local
            IPv4 UDP end-points on which an application is
            currently accepting datagrams.  This table has been
            deprecated in favor of the version neutral
            udpEndpointTable."
    ::= { udp 5 }

INDEX { udpLocalAddress, udpLocalPort }
*/ 

/*\ 
 *  construction / destruction
\*/ 

udpEntryBridge::udpEntryBridge (const BcmIpAddress &udpLocalAddress, int udpLocalPort, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_udpTable,
                      BcmObjectId(udpLocalAddress) + BcmObjectId(udpLocalPort),
                      pAgent)
{
  fAddr = udpLocalAddress;
  fPort = udpLocalPort;
}

udpEntryBridge::~udpEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
udpLocalAddress OBJECT-TYPE
    SYNTAX     IpAddress
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The local IP address for this UDP listener.  In the
            case of a UDP listener that is willing to accept
            datagrams for any IP interface associated with the
            node, the value 0.0.0.0 is used."
    ::= { udpEntry 1 }
*/ 
void udpEntryBridge::Get_udpLocalAddress (BcmIpAddress &udpLocalAddress)
{
  udpLocalAddress = fAddr;
}


/* 
udpLocalPort OBJECT-TYPE
    SYNTAX     Integer32 (0..65535)
    MAX-ACCESS read-only
    STATUS     deprecated
    DESCRIPTION
           "The local port number for this UDP listener."
    ::= { udpEntry 2 }
*/ 
int udpEntryBridge::Get_udpLocalPort ()
{
  return fPort;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/



