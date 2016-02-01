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
//    Description:                                                          
//                                                                          
//**************************************************************************
//    Revision History:                                                     
//                                                                          
//**************************************************************************
//    Filename: docsDevNmBridge.cpp
//    Creation Date: Monday, May 20, 2002 at 14:46:50
//    Created by Broadcom V2 Mib Compiler BCMIBC.EXE version 1.2.0b 
//    from input file rfc2669.mib
//
//**************************************************************************

#include "MibObjs.h"
#include "docsDevNmBridge.h"

#include "NmAccess.h"
#include "BcmSnmpAgent.h"
#include "SnmpLog.h"

/******************************************************************************\

-- NOTE: Various non-SMI text removed below (as indicated) for benefit of MIB
-- compiler.  Complete MIB text may be obtained at:
-- http://www.ietf.org/rfc/rfc2669.txt
-- Kevin O'Neal, 9/7/99

-- 1.  The SNMP Management Framework: REMOVED
-- 2.  Glossary: REMOVED
-- 3.  Overview: REMOVED

-- and we really start with...
-- 4.  Definitions

DOCS-CABLE-DEVICE-MIB DEFINITIONS ::= BEGIN

IMPORTS
        MODULE-IDENTITY,
        OBJECT-TYPE,
-- do not import        BITS,
        IpAddress,
        Unsigned32,
        Counter32,
        Integer32,
        zeroDotZero,
        mib-2
                FROM SNMPv2-SMI
        RowStatus,
        RowPointer,
        DateAndTime,
        TruthValue
                FROM SNMPv2-TC
        OBJECT-GROUP,
        MODULE-COMPLIANCE
                FROM SNMPv2-CONF
        SnmpAdminString
                FROM SNMP-FRAMEWORK-MIB -- RFC2571
        InterfaceIndexOrZero
                FROM IF-MIB;  -- RFC2233

docsDev MODULE-IDENTITY
        LAST-UPDATED    "9908190000Z" -- August 19, 1999
        ORGANIZATION    "IETF IPCDN Working Group"
        CONTACT-INFO
            "        Michael StJohns
             Postal: @Home Network
                     425 Broadway
                     Redwood City, CA 94063
                     U.S.A.
             Phone:  +1 650 569 5368
             E-mail: stjohns@corp.home.net"
        DESCRIPTION
            "This is the MIB Module for MCNS-compliant cable modems and
             cable-modem termination systems."
        REVISION "9908190000Z"
        DESCRIPTION
            "Initial Version, published as RFC 2669.
             Modified by Mike StJohns to add/revise filtering, TOS
             support, software version information objects."
        ::= { mib-2 69 }

docsDevMIBObjects  OBJECT IDENTIFIER ::= { docsDev 1 }
docsDevBase OBJECT IDENTIFIER ::= { docsDevMIBObjects 1 }

--
-- For the following object, there is no concept in the
-- RFI specification corresponding to a backup CMTS. The
-- enumeration is provided here in case someone is able
-- to define such a role or device.
--


\******************************************************************************/




/*\
 *
 *  docsDevNmAccessEntryBridge for bridging elements in the docsDevNmAccessTable
 *
\*/

/* 
docsDevNmAccessTable OBJECT-TYPE
        SYNTAX      SEQUENCE OF DocsDevNmAccessEntry
        MAX-ACCESS  not-accessible
        STATUS      current
        DESCRIPTION
            "This table controls access to SNMP objects by network
             management stations. If the table is empty, access
             to SNMP objects is unrestricted.  This table exists only
             on SNMPv1 or v2c agents and does not exist on SNMPv3
             agents. See the conformance section for details.
             Specifically, for v3 agents, the appropriate MIBs and
             security models apply in lieu of this table."
        ::= { docsDevMIBObjects 2 }

INDEX { docsDevNmAccessIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

docsDevNmAccessEntryBridge::docsDevNmAccessEntryBridge (int docsDevNmAccessIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge ("1.3.6.1.2.1.69.1.2",
                      BcmObjectId(docsDevNmAccessIndex),
                      pAgent)
{
  fRowStatus = SNMP_STATUS_INVISIBLE;
  
  fpController = new NmAccessController (docsDevNmAccessIndex, pAgent);
  
  // Initialize 'test' values:
  fTestControl = Get_docsDevNmAccessControl();
  Get_docsDevNmAccessIpMask(fTestIpMask);

  // Programatically created entries will be
  // initialized as if created by CREATE_AND_WAIT.
  // Change to CREATE_AND_GO if immediate activation
  // is desired.
  int Status = SNMP_STATUS_CREATE_AND_WAIT;
  Set_docsDevNmAccessStatus (Status);
}

/* Alternate form of constructor for use by docsDevNmAccessTable::NewDefaultEntry */
docsDevNmAccessEntryBridge::docsDevNmAccessEntryBridge (const BcmObjectId &Index, BcmSnmpAgent *pAgent)
  : TabularMibBridge ("1.3.6.1.2.1.69.1.2", Index, pAgent)
{
  fRowStatus = SNMP_STATUS_INVISIBLE;
  
  fpController = new NmAccessController (Index[0], pAgent);
  
  // Initialize 'test' values:
  fTestControl = Get_docsDevNmAccessControl();
  Get_docsDevNmAccessIpMask(fTestIpMask);
}

docsDevNmAccessEntryBridge::~docsDevNmAccessEntryBridge ()
{
  delete fpController;
}

/*\ 
 *  GET methods
\*/

/* 
docsDevNmAccessIndex OBJECT-TYPE
        SYNTAX      Integer32 (1..2147483647)
        MAX-ACCESS  not-accessible
        STATUS      current
        DESCRIPTION
            "Index used to order the application of access
             entries."
        ::= { docsDevNmAccessEntry 1 }
*/ 
int docsDevNmAccessEntryBridge::Get_docsDevNmAccessIndex ()
{
  return fpController->GetIndex();
}


/* 
docsDevNmAccessIp OBJECT-TYPE
        SYNTAX      IpAddress
        MAX-ACCESS  read-create
        STATUS      current
        DESCRIPTION
            "The IP address (or subnet) of the network management
             station. The address 255.255.255.255 is defined to mean
             any NMS. If traps are enabled for this entry, then the
             value must be the address of a specific device."
        DEFVAL { 'ffffffff'h }
        ::= { docsDevNmAccessEntry 2 }
*/ 
void docsDevNmAccessEntryBridge::Get_docsDevNmAccessIp (BcmIpAddress &docsDevNmAccessIp)
{
  docsDevNmAccessIp = fpController->GetIpAddr();
}


/* 
docsDevNmAccessIpMask OBJECT-TYPE
        SYNTAX      IpAddress
        MAX-ACCESS  read-create
        STATUS      current
        DESCRIPTION
            "The IP subnet mask of the network management stations.
             If traps are enabled for this entry, then the value must
             be 255.255.255.255."
        DEFVAL { 'ffffffff'h }
        ::= { docsDevNmAccessEntry 3 }
*/ 
void docsDevNmAccessEntryBridge::Get_docsDevNmAccessIpMask (BcmIpAddress &docsDevNmAccessIpMask)
{
  docsDevNmAccessIpMask = fpController->GetIpMask();
}


/* 
docsDevNmAccessCommunity OBJECT-TYPE
        SYNTAX      OCTET STRING
        MAX-ACCESS  read-create
        STATUS      current
        DESCRIPTION
            "The community string to be matched for access by this
             entry. If set to a zero length string then any community
             string will match.  When read, this object SHOULD return
             a zero length string."
        DEFVAL { "public" }
        ::= { docsDevNmAccessEntry 4 }
*/ 
int docsDevNmAccessEntryBridge::Get_docsDevNmAccessCommunity (BcmString &docsDevNmAccessCommunity)
{
  // Per RFC-2669, this should (read: MUST) return an empty string when queried.
  docsDevNmAccessCommunity = "";
  return 0;
}


/* 
docsDevNmAccessControl OBJECT-TYPE
        SYNTAX         INTEGER {
            none(1),
            read(2),
            readWrite(3),
            roWithTraps(4),
            rwWithTraps(5),
            trapsOnly(6)
        }
        MAX-ACCESS  read-create
        STATUS      current
        DESCRIPTION
            "Specifies the type of access allowed to this NMS. Setting
             this object to none(1) causes the table entry to be
             destroyed. Read(2) allows access by 'get' and 'get-next'
             PDUs. ReadWrite(3) allows access by 'set' as well.
             RoWithtraps(4), rwWithTraps(5), and trapsOnly(6)
             control distribution of Trap PDUs transmitted by this
             device."
        DEFVAL { read }
        ::= { docsDevNmAccessEntry 5 }
*/ 
int docsDevNmAccessEntryBridge::Get_docsDevNmAccessControl ()
{
  return fpController->GetControl();
}


/* 
docsDevNmAccessInterfaces OBJECT-TYPE
        SYNTAX      OCTET STRING
        MAX-ACCESS  read-create
        STATUS      current
        DESCRIPTION
            "Specifies the set of interfaces from which requests from
             this NMS will be accepted.
             Each octet within the value of this object specifies a set
             of eight interfaces, with the first octet specifying ports
             1 through 8, the second octet specifying interfaces 9
             through 16, etc.  Within each octet, the most significant
             bit represents the lowest numbered interface, and the least
             significant bit represents the highest numbered interface.
             Thus, each interface is represented by a single bit within
             the value of this object. If that bit has a value of '1'
             then that interface is included in the set.

             Note that entries in this table apply only to link-layer
             interfaces (e.g., Ethernet and CATV MAC). Upstream and
             downstream channel interfaces must not be specified."
--         DEFVAL is the bitmask corresponding to all interfaces
        ::= { docsDevNmAccessEntry 6 }
*/ 
unsigned long docsDevNmAccessEntryBridge::Get_docsDevNmAccessInterfaces ()
{
  return fpController->GetInterfaces();
}


/* 
docsDevNmAccessStatus OBJECT-TYPE
        SYNTAX      RowStatus
        MAX-ACCESS  read-create
        STATUS      current
        DESCRIPTION
            "Controls and reflects the status of rows in this
             table. Rows in this table may be created by either the
             create-and-go or create-and-wait paradigms.  There is no
             restriction on changing values in a row of this table while
             the row is active."
        ::= { docsDevNmAccessEntry 7 }
*/ 
int docsDevNmAccessEntryBridge::Get_docsDevNmAccessStatus ()
{
  return fpController->GetStatus();
}


/* 
docsDevNmAccessTrapVersion OBJECT-TYPE
        SYNTAX      INTEGER {
                disableSNMPv2trap(1),
                enableSNMPv2trap(2)
        }
        MAX-ACCESS  read-create
        STATUS      current
        DESCRIPTION
            "Specifies the TRAP version that is sent to this NMS. Setting this 
             object to DisableSNMPv2trap (1) causes the trap in SNMPv1 format 
             to be sent to particular NMS. Setting this object to 
             EnableSNMPv2trap (2) causes the trap in SNMPv2 format be sent to 
             particular NMS"
        DEFVAL { disableSNMPv2trap }
        ::= { docsDevNmAccessEntry 8 }
*/ 
int docsDevNmAccessEntryBridge::Get_docsDevNmAccessTrapVersion ()
{
  return fpController->GetTrapType();
}


/*\ 
 *  TEST methods
\*/

/* 
docsDevNmAccessIp OBJECT-TYPE
        SYNTAX      IpAddress
        MAX-ACCESS  read-create
        STATUS      current
        DESCRIPTION
            "The IP address (or subnet) of the network management
             station. The address 255.255.255.255 is defined to mean
             any NMS. If traps are enabled for this entry, then the
             value must be the address of a specific device."
        DEFVAL { 'ffffffff'h }
        ::= { docsDevNmAccessEntry 2 }
*/ 
SNMP_STATUS docsDevNmAccessEntryBridge::Test_docsDevNmAccessIp (const BcmIpAddress &docsDevNmAccessIp)
{
  return IpAddrMibObject::TestDefaults (docsDevNmAccessIp);
}


/* 
docsDevNmAccessIpMask OBJECT-TYPE
        SYNTAX      IpAddress
        MAX-ACCESS  read-create
        STATUS      current
        DESCRIPTION
            "The IP subnet mask of the network management stations.
             If traps are enabled for this entry, then the value must
             be 255.255.255.255."
        DEFVAL { 'ffffffff'h }
        ::= { docsDevNmAccessEntry 3 }
*/ 
SNMP_STATUS docsDevNmAccessEntryBridge::Test_docsDevNmAccessIpMask (const BcmIpAddress &docsDevNmAccessIpMask)
{
  SNMP_STATUS Status = IpAddrMibObject::TestDefaults (docsDevNmAccessIpMask);
  
  if (Status == SNMP_NO_ERROR)
    fTestIpMask = docsDevNmAccessIpMask;
    
  return Status;
}


/* 
docsDevNmAccessCommunity OBJECT-TYPE
        SYNTAX      OCTET STRING
        MAX-ACCESS  read-create
        STATUS      current
        DESCRIPTION
            "The community string to be matched for access by this
             entry. If set to a zero length string then any community
             string will match.  When read, this object SHOULD return
             a zero length string."
        DEFVAL { "public" }
        ::= { docsDevNmAccessEntry 4 }
*/ 
SNMP_STATUS docsDevNmAccessEntryBridge::Test_docsDevNmAccessCommunity (const BcmString &docsDevNmAccessCommunity)
{
  return StringMibObject::TestDefaults (docsDevNmAccessCommunity);
}


/* 
docsDevNmAccessControl OBJECT-TYPE
        SYNTAX         INTEGER {
            none(1),
            read(2),
            readWrite(3),
            roWithTraps(4),
            rwWithTraps(5),
            trapsOnly(6)
        }
        MAX-ACCESS  read-create
        STATUS      current
        DESCRIPTION
            "Specifies the type of access allowed to this NMS. Setting
             this object to none(1) causes the table entry to be
             destroyed. Read(2) allows access by 'get' and 'get-next'
             PDUs. ReadWrite(3) allows access by 'set' as well.
             RoWithtraps(4), rwWithTraps(5), and trapsOnly(6)
             control distribution of Trap PDUs transmitted by this
             device."
        DEFVAL { read }
        ::= { docsDevNmAccessEntry 5 }
*/ 
SNMP_STATUS docsDevNmAccessEntryBridge::Test_docsDevNmAccessControl (int docsDevNmAccessControl)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (docsDevNmAccessControl);

  if (Status == SNMP_NO_ERROR)
  {
    if ((docsDevNmAccessControl != CONSTVAL_DOCSDEVNMACCESSCONTROL_NONE)
    &&  (docsDevNmAccessControl != CONSTVAL_DOCSDEVNMACCESSCONTROL_READ)
    &&  (docsDevNmAccessControl != CONSTVAL_DOCSDEVNMACCESSCONTROL_READWRITE)
    &&  (docsDevNmAccessControl != CONSTVAL_DOCSDEVNMACCESSCONTROL_ROWITHTRAPS)
    &&  (docsDevNmAccessControl != CONSTVAL_DOCSDEVNMACCESSCONTROL_RWWITHTRAPS)
    &&  (docsDevNmAccessControl != CONSTVAL_DOCSDEVNMACCESSCONTROL_TRAPSONLY))
      return SNMP_WRONG_VALUE;
      
    fTestControl = docsDevNmAccessControl;
  }

  return Status;
}


/* 
docsDevNmAccessInterfaces OBJECT-TYPE
        SYNTAX      OCTET STRING
        MAX-ACCESS  read-create
        STATUS      current
        DESCRIPTION
            "Specifies the set of interfaces from which requests from
             this NMS will be accepted.
             Each octet within the value of this object specifies a set
             of eight interfaces, with the first octet specifying ports
             1 through 8, the second octet specifying interfaces 9
             through 16, etc.  Within each octet, the most significant
             bit represents the lowest numbered interface, and the least
             significant bit represents the highest numbered interface.
             Thus, each interface is represented by a single bit within
             the value of this object. If that bit has a value of '1'
             then that interface is included in the set.

             Note that entries in this table apply only to link-layer
             interfaces (e.g., Ethernet and CATV MAC). Upstream and
             downstream channel interfaces must not be specified."
--         DEFVAL is the bitmask corresponding to all interfaces
        ::= { docsDevNmAccessEntry 6 }
*/ 
SNMP_STATUS docsDevNmAccessEntryBridge::Test_docsDevNmAccessInterfaces (unsigned long docsDevNmAccessInterfaces)
{
  SNMP_STATUS Status = BitsMibObject::TestDefaults (docsDevNmAccessInterfaces);

  if (Status == SNMP_NO_ERROR)
  {
    // CableLabs continues to insist on setting this to invalid values, so I
    // will just accept anything.  Nevermind what RFC-2669 says, just take anything.
    // Upstream, downstream, interfaces that don't exist, anything.
#if 0
    IfTableBridge *pIf;
    unsigned long Allowable = 0;
    int Count = 0;
    
    // OK, I give up.  Those guys at CL just INSIST on constantly using 0xFF
    // for interfaces in the NM table, despite the fact that
    // the MIB clearly states that this is an invalid value.  So, go ahead
    // and accept 0xFF.
    if (IfMask == 0xFF)
      return SNMP_NO_ERROR;                       
    
    // Validate that the bitmask passed here is valid.
    pIf = fpAgent->IfBridge().FirstIfEntry ();
    while (pIf)
    {
      /*\
       *   - from RFC 2669:
       *   
       *  "Note that entries in this table apply only to link-layer
       *   interfaces (e.g., Ethernet and CATV MAC). Upstream and
       *   downstream channel interfaces must not be specified."
       *
       *   My own note: likewise w/ IP stack interface.
      \*/
      
      if ((pIf->Get_ifType() != BcmHalIf::kIanaTypeCmUpstream)
      &&  (pIf->Get_ifType() != BcmHalIf::kIanaTypeCmDownstream)
      &&  (pIf->Get_ifType() != BcmHalIf::kIanaTypeIpStack))
        Allowable |= (1 << Count);
      
      Count++;
      
      pIf = fpAgent->IfBridge().NextIfEntry (pIf->Get_ifIndex());
    }
    
    if (IfMask & ~Allowable)
      return SNMP_WRONG_VALUE;
#endif
  }

  return Status;
}


/* 
docsDevNmAccessStatus OBJECT-TYPE
        SYNTAX      RowStatus
        MAX-ACCESS  read-create
        STATUS      current
        DESCRIPTION
            "Controls and reflects the status of rows in this
             table. Rows in this table may be created by either the
             create-and-go or create-and-wait paradigms.  There is no
             restriction on changing values in a row of this table while
             the row is active."
        ::= { docsDevNmAccessEntry 7 }
*/ 
SNMP_STATUS docsDevNmAccessEntryBridge::Test_docsDevNmAccessStatus (int docsDevNmAccessStatus)
{
  SNMP_STATUS Status = RowStatusMibObject::TestDefaults (docsDevNmAccessStatus, Get_docsDevNmAccessStatus());

  if (Status == SNMP_NO_ERROR)
  {
    fObjectMask |= kBIT_docsDevNmAccessStatus;
  }

  return Status;
}


/* 
docsDevNmAccessTrapVersion OBJECT-TYPE
        SYNTAX      INTEGER {
                disableSNMPv2trap(1),
                enableSNMPv2trap(2)
        }
        MAX-ACCESS  read-create
        STATUS      current
        DESCRIPTION
            "Specifies the TRAP version that is sent to this NMS. Setting this 
             object to DisableSNMPv2trap (1) causes the trap in SNMPv1 format 
             to be sent to particular NMS. Setting this object to 
             EnableSNMPv2trap (2) causes the trap in SNMPv2 format be sent to 
             particular NMS"
        DEFVAL { disableSNMPv2trap }
        ::= { docsDevNmAccessEntry 8 }
*/ 
SNMP_STATUS docsDevNmAccessEntryBridge::Test_docsDevNmAccessTrapVersion (int docsDevNmAccessTrapVersion)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (docsDevNmAccessTrapVersion);

  if (Status == SNMP_NO_ERROR)
  {
    if ((docsDevNmAccessTrapVersion != CONSTVAL_DOCSDEVNMACCESSTRAPVERSION_DISABLESNMPV2TRAP)
    &&  (docsDevNmAccessTrapVersion != CONSTVAL_DOCSDEVNMACCESSTRAPVERSION_ENABLESNMPV2TRAP))
      return SNMP_WRONG_VALUE;
  }

  return Status;
}


/*\ 
 *  SET methods
\*/

/* 
docsDevNmAccessIp OBJECT-TYPE
        SYNTAX      IpAddress
        MAX-ACCESS  read-create
        STATUS      current
        DESCRIPTION
            "The IP address (or subnet) of the network management
             station. The address 255.255.255.255 is defined to mean
             any NMS. If traps are enabled for this entry, then the
             value must be the address of a specific device."
        DEFVAL { 'ffffffff'h }
        ::= { docsDevNmAccessEntry 2 }
*/ 
SNMP_STATUS docsDevNmAccessEntryBridge::Set_docsDevNmAccessIp (const BcmIpAddress &docsDevNmAccessIp)
{
  SNMP_STATUS Status = IpAddrMibObject::TestDefaults (docsDevNmAccessIp);

  if (Status == SNMP_NO_ERROR)
  {
    return fpController->SetIpAddr (docsDevNmAccessIp);
  }

  return Status;
}


/* 
docsDevNmAccessIpMask OBJECT-TYPE
        SYNTAX      IpAddress
        MAX-ACCESS  read-create
        STATUS      current
        DESCRIPTION
            "The IP subnet mask of the network management stations.
             If traps are enabled for this entry, then the value must
             be 255.255.255.255."
        DEFVAL { 'ffffffff'h }
        ::= { docsDevNmAccessEntry 3 }
*/ 
SNMP_STATUS docsDevNmAccessEntryBridge::Set_docsDevNmAccessIpMask (const BcmIpAddress &docsDevNmAccessIpMask)
{
  SNMP_STATUS Status = IpAddrMibObject::TestDefaults (docsDevNmAccessIpMask);
  
  // PR 6100: enforce the 'no traps unless mask is 255.255.255.255' rule.
  if ((fTestControl == CONSTVAL_DOCSDEVNMACCESSCONTROL_ROWITHTRAPS)
  ||  (fTestControl == CONSTVAL_DOCSDEVNMACCESSCONTROL_RWWITHTRAPS)
  ||  (fTestControl == CONSTVAL_DOCSDEVNMACCESSCONTROL_TRAPSONLY))
  {
    if (docsDevNmAccessIpMask != kBroadcastIp)
    {
      SnmpLogReq << "Control has traps, mask != 255.255.255.255, INCONSISTENT_VALUE." << endl;
      return SNMP_INCONSISTENT_VALUE;
    }
  }

  if (Status == SNMP_NO_ERROR)
  {
    return fpController->SetIpMask (docsDevNmAccessIpMask);
  }

  return Status;
}


/* 
docsDevNmAccessCommunity OBJECT-TYPE
        SYNTAX      OCTET STRING
        MAX-ACCESS  read-create
        STATUS      current
        DESCRIPTION
            "The community string to be matched for access by this
             entry. If set to a zero length string then any community
             string will match.  When read, this object SHOULD return
             a zero length string."
        DEFVAL { "public" }
        ::= { docsDevNmAccessEntry 4 }
*/ 
SNMP_STATUS docsDevNmAccessEntryBridge::Set_docsDevNmAccessCommunity (const BcmString &docsDevNmAccessCommunity)
{
  SNMP_STATUS Status = StringMibObject::TestDefaults (docsDevNmAccessCommunity);

  if (Status == SNMP_NO_ERROR)
  {
    return fpController->SetCommunity (docsDevNmAccessCommunity);
  }

  return Status;
}


/* 
docsDevNmAccessControl OBJECT-TYPE
        SYNTAX         INTEGER {
            none(1),
            read(2),
            readWrite(3),
            roWithTraps(4),
            rwWithTraps(5),
            trapsOnly(6)
        }
        MAX-ACCESS  read-create
        STATUS      current
        DESCRIPTION
            "Specifies the type of access allowed to this NMS. Setting
             this object to none(1) causes the table entry to be
             destroyed. Read(2) allows access by 'get' and 'get-next'
             PDUs. ReadWrite(3) allows access by 'set' as well.
             RoWithtraps(4), rwWithTraps(5), and trapsOnly(6)
             control distribution of Trap PDUs transmitted by this
             device."
        DEFVAL { read }
        ::= { docsDevNmAccessEntry 5 }
*/ 
SNMP_STATUS docsDevNmAccessEntryBridge::Set_docsDevNmAccessControl (int docsDevNmAccessControl)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (docsDevNmAccessControl);

  if (Status == SNMP_NO_ERROR)
  {
    if ((docsDevNmAccessControl != CONSTVAL_DOCSDEVNMACCESSCONTROL_NONE)
    &&  (docsDevNmAccessControl != CONSTVAL_DOCSDEVNMACCESSCONTROL_READ)
    &&  (docsDevNmAccessControl != CONSTVAL_DOCSDEVNMACCESSCONTROL_READWRITE)
    &&  (docsDevNmAccessControl != CONSTVAL_DOCSDEVNMACCESSCONTROL_ROWITHTRAPS)
    &&  (docsDevNmAccessControl != CONSTVAL_DOCSDEVNMACCESSCONTROL_RWWITHTRAPS)
    &&  (docsDevNmAccessControl != CONSTVAL_DOCSDEVNMACCESSCONTROL_TRAPSONLY))
      return SNMP_WRONG_VALUE;
      
    // PR 6100: handle the implied requirement that if control includes traps,
    // mask must be all 255's
    if ((docsDevNmAccessControl == CONSTVAL_DOCSDEVNMACCESSCONTROL_ROWITHTRAPS)
    ||  (docsDevNmAccessControl == CONSTVAL_DOCSDEVNMACCESSCONTROL_RWWITHTRAPS)
    ||  (docsDevNmAccessControl == CONSTVAL_DOCSDEVNMACCESSCONTROL_TRAPSONLY))
    {
      if (fTestIpMask != kBroadcastIp)
      {
        SnmpLogReq << "Control has traps, mask != 255.255.255.255, INCONSISTENT_VALUE." << endl;
        return SNMP_INCONSISTENT_VALUE;
      }
    }

    fpController->SetControl (docsDevNmAccessControl);
    
    // If this is set to NONE, destroy the entry.
    if (docsDevNmAccessControl == CONSTVAL_DOCSDEVNMACCESSCONTROL_NONE)
    {
      SnmpLogNm << "docsDevNmAccessControl set to NONE, destroying entry." << endl;
      if (fpObject)
        delete fpObject;
    }
  }

  return Status;
}


/* 
docsDevNmAccessInterfaces OBJECT-TYPE
        SYNTAX      OCTET STRING
        MAX-ACCESS  read-create
        STATUS      current
        DESCRIPTION
            "Specifies the set of interfaces from which requests from
             this NMS will be accepted.
             Each octet within the value of this object specifies a set
             of eight interfaces, with the first octet specifying ports
             1 through 8, the second octet specifying interfaces 9
             through 16, etc.  Within each octet, the most significant
             bit represents the lowest numbered interface, and the least
             significant bit represents the highest numbered interface.
             Thus, each interface is represented by a single bit within
             the value of this object. If that bit has a value of '1'
             then that interface is included in the set.

             Note that entries in this table apply only to link-layer
             interfaces (e.g., Ethernet and CATV MAC). Upstream and
             downstream channel interfaces must not be specified."
--         DEFVAL is the bitmask corresponding to all interfaces
        ::= { docsDevNmAccessEntry 6 }
*/ 
SNMP_STATUS docsDevNmAccessEntryBridge::Set_docsDevNmAccessInterfaces (unsigned long docsDevNmAccessInterfaces)
{
  SNMP_STATUS Status = BitsMibObject::TestDefaults (docsDevNmAccessInterfaces);

  if (Status == SNMP_NO_ERROR)
  {
    // CableLabs continues to insist on setting this to invalid values, so I
    // will just accept anything.  Nevermind what RFC-2669 says, just take anything.
    // Upstream, downstream, interfaces that don't exist, anything.
#if 0
    IfTableBridge *pIf;
    unsigned long Allowable = 0;
    int Count = 0;
    
    // OK, I give up.  Those guys at CL just INSIST on constantly using 0xFF
    // for interfaces in the NM table, despite the fact that
    // the MIB clearly states that this is an invalid value.  So, go ahead
    // and accept 0xFF.
    if (IfMask == 0xFF)
      return SNMP_NO_ERROR;                       
    
    // Validate that the bitmask passed here is valid.
    pIf = fpAgent->IfBridge().FirstIfEntry ();
    while (pIf)
    {
      /*\
       *   - from RFC 2669:
       *   
       *  "Note that entries in this table apply only to link-layer
       *   interfaces (e.g., Ethernet and CATV MAC). Upstream and
       *   downstream channel interfaces must not be specified."
       *
       *   My own note: likewise w/ IP stack interface.
      \*/
      
      if ((pIf->Get_ifType() != BcmHalIf::kIanaTypeCmUpstream)
      &&  (pIf->Get_ifType() != BcmHalIf::kIanaTypeCmDownstream)
      &&  (pIf->Get_ifType() != BcmHalIf::kIanaTypeIpStack))
        Allowable |= (1 << Count);
      
      Count++;
      
      pIf = fpAgent->IfBridge().NextIfEntry (pIf->Get_ifIndex());
    }
    
    if (IfMask & ~Allowable)
      return SNMP_WRONG_VALUE;
#endif
    
    return fpController->SetInterfaces (docsDevNmAccessInterfaces);
  }

  return Status;
}


/* 
docsDevNmAccessStatus OBJECT-TYPE
        SYNTAX      RowStatus
        MAX-ACCESS  read-create
        STATUS      current
        DESCRIPTION
            "Controls and reflects the status of rows in this
             table. Rows in this table may be created by either the
             create-and-go or create-and-wait paradigms.  There is no
             restriction on changing values in a row of this table while
             the row is active."
        ::= { docsDevNmAccessEntry 7 }
*/ 
SNMP_STATUS docsDevNmAccessEntryBridge::Set_docsDevNmAccessStatus (int &docsDevNmAccessStatus)
{
  SNMP_STATUS Status = SetRowStatus (docsDevNmAccessStatus);

  if (Status == SNMP_NO_ERROR)
  {
    fObjectMask |= kBIT_docsDevNmAccessStatus;
    fRowStatus = docsDevNmAccessStatus;

    Status = fpController->SetStatus (docsDevNmAccessStatus);
  }

  return Status;
}


/* 
docsDevNmAccessTrapVersion OBJECT-TYPE
        SYNTAX      INTEGER {
                disableSNMPv2trap(1),
                enableSNMPv2trap(2)
        }
        MAX-ACCESS  read-create
        STATUS      current
        DESCRIPTION
            "Specifies the TRAP version that is sent to this NMS. Setting this 
             object to DisableSNMPv2trap (1) causes the trap in SNMPv1 format 
             to be sent to particular NMS. Setting this object to 
             EnableSNMPv2trap (2) causes the trap in SNMPv2 format be sent to 
             particular NMS"
        DEFVAL { disableSNMPv2trap }
        ::= { docsDevNmAccessEntry 8 }
*/ 
SNMP_STATUS docsDevNmAccessEntryBridge::Set_docsDevNmAccessTrapVersion (int docsDevNmAccessTrapVersion)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (docsDevNmAccessTrapVersion);

  if (Status == SNMP_NO_ERROR)
  {
    if ((docsDevNmAccessTrapVersion != CONSTVAL_DOCSDEVNMACCESSTRAPVERSION_DISABLESNMPV2TRAP)
    &&  (docsDevNmAccessTrapVersion != CONSTVAL_DOCSDEVNMACCESSTRAPVERSION_ENABLESNMPV2TRAP))
      return SNMP_WRONG_VALUE;

    return fpController->SetTrapType (docsDevNmAccessTrapVersion);
  }

  return Status;
}

