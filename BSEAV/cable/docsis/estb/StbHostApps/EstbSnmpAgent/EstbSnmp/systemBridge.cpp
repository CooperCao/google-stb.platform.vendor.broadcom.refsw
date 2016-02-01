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
//    Filename: systemBridge.cpp
//    Creation Date: Wednesday, June 18, 2003 at 10:46:34
//    Created by Broadcom V2/BFC/BOSS Mib Compiler BCMIBC.EXE version 1.4.0c
//    from input file rfc3418.mib
//
//**************************************************************************

#include "MibObjs.h"
#include "SnmpLog.h"
#include "systemBridge.h"


/******************************************************************************\

-- MIB edited for compilability.  Complete text available at
-- http://www.ietf.org/rfc/rfc3415.txt
-- Kevin O'Neal, 6/17/2003
--
-- Note that this MIB obsoletes & replaces RFC-1907

SNMPv2-MIB DEFINITIONS ::= BEGIN

IMPORTS
    MODULE-IDENTITY, OBJECT-TYPE, NOTIFICATION-TYPE,
    TimeTicks, Counter32, snmpModules, mib-2
        FROM SNMPv2-SMI
    DisplayString, TestAndIncr, TimeStamp
        FROM SNMPv2-TC
    MODULE-COMPLIANCE, OBJECT-GROUP, NOTIFICATION-GROUP
        FROM SNMPv2-CONF;

snmpMIB MODULE-IDENTITY
    LAST-UPDATED "200210160000Z"
    ORGANIZATION "IETF SNMPv3 Working Group"
    CONTACT-INFO
            "WG-EMail:   snmpv3@lists.tislabs.com
             Subscribe:  snmpv3-request@lists.tislabs.com

             Co-Chair:   Russ Mundy
                         Network Associates Laboratories
             postal:     15204 Omega Drive, Suite 300
                         Rockville, MD 20850-4601
                         USA
             EMail:      mundy@tislabs.com
             phone:      +1 301 947-7107

             Co-Chair:   David Harrington
                         Enterasys Networks
             postal:     35 Industrial Way
                         P. O. Box 5005
                         Rochester, NH 03866-5005
                         USA
             EMail:      dbh@enterasys.com
             phone:      +1 603 337-2614

             Editor:     Randy Presuhn
                         BMC Software, Inc.
             postal:     2141 North First Street
                         San Jose, CA 95131
                         USA
             EMail:      randy_presuhn@bmc.com
             phone:      +1 408 546-1006"
    DESCRIPTION
            "The MIB module for SNMP entities.

             Copyright (C) The Internet Society (2002). This
             version of this MIB module is part of RFC 3418;
             see the RFC itself for full legal notices.
            "
    REVISION      "200210160000Z"
    DESCRIPTION
            "This revision of this MIB module was published as
             RFC 3418."
    REVISION      "199511090000Z"
    DESCRIPTION
            "This revision of this MIB module was published as
             RFC 1907."
    REVISION      "199304010000Z"
    DESCRIPTION
            "The initial revision of this MIB module was published
            as RFC 1450."
    ::= { snmpModules 1 }

snmpMIBObjects OBJECT IDENTIFIER ::= { snmpMIB 1 }

--  ::= { snmpMIBObjects 1 }        this OID is obsolete
--  ::= { snmpMIBObjects 2 }        this OID is obsolete
--  ::= { snmpMIBObjects 3 }        this OID is obsolete

-- the System group
--
-- a collection of objects common to all managed systems.

system   OBJECT IDENTIFIER ::= { mib-2 1 }


\******************************************************************************/




/*\
 *
 *  systemGroupBridge for bridging scalars from the systemGroup group 
 *
\*/

/* 
system   OBJECT IDENTIFIER ::= { mib-2 1 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

systemGroupBridge::systemGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge ("1.3.6.1.2.1.1", pAgent)
{
}

systemGroupBridge::~systemGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
sysDescr OBJECT-TYPE
    SYNTAX      DisplayString (SIZE (0..255))
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "A textual description of the entity.  This value should
            include the full name and version identification of
            the system's hardware type, software operating-system,
            and networking software."
    ::= { system 1 }
*/ 
int systemGroupBridge::Get_sysDescr (BcmString &sysDescr)
{
  sysDescr = fpAgent->sysDescr();
  return sysDescr.length();
}


/* 
sysObjectID OBJECT-TYPE
    SYNTAX      OBJECT IDENTIFIER
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The vendor's authoritative identification of the
            network management subsystem contained in the entity.
            This value is allocated within the SMI enterprises
            subtree (1.3.6.1.4.1) and provides an easy and
            unambiguous means for determining `what kind of box' is
            being managed.  For example, if vendor `Flintstones,
            Inc.' was assigned the subtree 1.3.6.1.4.1.424242,
            it could assign the identifier 1.3.6.1.4.1.424242.1.1
            to its `Fred Router'."
    ::= { system 2 }
*/ 
int systemGroupBridge::Get_sysObjectID (BcmObjectId &sysObjectID)
{
  sysObjectID = fpAgent->sysObjectID();
  return sysObjectID.Length();
}


/* 
sysUpTime OBJECT-TYPE
    SYNTAX      TimeTicks
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "The time (in hundredths of a second) since the
            network management portion of the system was last
            re-initialized."
    ::= { system 3 }
*/ 
unsigned long systemGroupBridge::Get_sysUpTime ()
{
  return fpAgent->sysUpTime();
}


/* 
sysContact OBJECT-TYPE
    SYNTAX      DisplayString (SIZE (0..255))
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
            "The textual identification of the contact person for
            this managed node, together with information on how
            to contact this person.  If no contact information is
            known, the value is the zero-length string."
    ::= { system 4 }
*/ 
int systemGroupBridge::Get_sysContact (BcmString &sysContact)
{
  sysContact = fpAgent->sysContact();
  return sysContact.length();
}


/* 
sysName OBJECT-TYPE
    SYNTAX      DisplayString (SIZE (0..255))
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
            "An administratively-assigned name for this managed
            node.  By convention, this is the node's fully-qualified
            domain name.  If the name is unknown, the value is
            the zero-length string."
    ::= { system 5 }
*/ 
int systemGroupBridge::Get_sysName (BcmString &sysName)
{
  sysName = fpAgent->sysName();
  return sysName.length();
}


/* 
sysLocation OBJECT-TYPE
    SYNTAX      DisplayString (SIZE (0..255))
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
            "The physical location of this node (e.g., 'telephone
            closet, 3rd floor').  If the location is unknown, the
            value is the zero-length string."
    ::= { system 6 }
*/ 
int systemGroupBridge::Get_sysLocation (BcmString &sysLocation)
{
  sysLocation = fpAgent->sysLocation();
  return sysLocation.length();
}


/* 
sysServices OBJECT-TYPE
    SYNTAX      INTEGER (0..127)
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
            "A value which indicates the set of services that this
            entity may potentially offer.  The value is a sum.
            This sum initially takes the value zero. Then, for
            each layer, L, in the range 1 through 7, that this node
            performs transactions for, 2 raised to (L - 1) is added
            to the sum.  For example, a node which performs only
            routing functions would have a value of 4 (2^(3-1)).
            In contrast, a node which is a host offering application
            services would have a value of 72 (2^(4-1) + 2^(7-1)).
            Note that in the context of the Internet suite of
            protocols, values should be calculated accordingly:

                 layer      functionality
                   1        physical (e.g., repeaters)
                   2        datalink/subnetwork (e.g., bridges)
                   3        internet (e.g., supports the IP)
                   4        end-to-end  (e.g., supports the TCP)
                   7        applications (e.g., supports the SMTP)

            For systems including OSI protocols, layers 5 and 6
            may also be counted."
    ::= { system 7 }
*/ 
int systemGroupBridge::Get_sysServices ()
{
  return fpAgent->sysServices();
}


/* 
sysORLastChange OBJECT-TYPE
    SYNTAX     TimeStamp
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
            "The value of sysUpTime at the time of the most recent
            change in state or value of any instance of sysORID."
    ::= { system 8 }
*/ 
unsigned long systemGroupBridge::Get_sysORLastChange ()
{
  return fpAgent->sysORLastChange();
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/

/* 
sysContact OBJECT-TYPE
    SYNTAX      DisplayString (SIZE (0..255))
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
            "The textual identification of the contact person for
            this managed node, together with information on how
            to contact this person.  If no contact information is
            known, the value is the zero-length string."
    ::= { system 4 }
*/ 
SNMP_STATUS systemGroupBridge::Set_sysContact (const BcmString &sysContact)
{
  SNMP_STATUS Status = StringMibObject::TestDefaults (sysContact);

  if (Status == SNMP_NO_ERROR)
  {
    if ((sysContact.size() < MINLEN_SYSCONTACT)
    ||  (sysContact.size() > MAXLEN_SYSCONTACT))
      return SNMP_WRONG_LENGTH;

    Status = fpAgent->sysContact (sysContact);
  }

  return Status;
}


/* 
sysName OBJECT-TYPE
    SYNTAX      DisplayString (SIZE (0..255))
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
            "An administratively-assigned name for this managed
            node.  By convention, this is the node's fully-qualified
            domain name.  If the name is unknown, the value is
            the zero-length string."
    ::= { system 5 }
*/ 
SNMP_STATUS systemGroupBridge::Set_sysName (const BcmString &sysName)
{
  SNMP_STATUS Status = StringMibObject::TestDefaults (sysName);

  if (Status == SNMP_NO_ERROR)
  {
    if ((sysName.size() < MINLEN_SYSNAME)
    ||  (sysName.size() > MAXLEN_SYSNAME))
      return SNMP_WRONG_LENGTH;

    Status = fpAgent->sysName (sysName);
  }

  return Status;
}


/* 
sysLocation OBJECT-TYPE
    SYNTAX      DisplayString (SIZE (0..255))
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
            "The physical location of this node (e.g., 'telephone
            closet, 3rd floor').  If the location is unknown, the
            value is the zero-length string."
    ::= { system 6 }
*/ 
SNMP_STATUS systemGroupBridge::Set_sysLocation (const BcmString &sysLocation)
{
  SNMP_STATUS Status = StringMibObject::TestDefaults (sysLocation);

  if (Status == SNMP_NO_ERROR)
  {
    if ((sysLocation.size() < MINLEN_SYSLOCATION)
    ||  (sysLocation.size() > MAXLEN_SYSLOCATION))
      return SNMP_WRONG_LENGTH;

    Status = fpAgent->sysLocation (sysLocation);
  }

  return Status;
}






/*\
 *
 *  sysOREntryBridge for bridging elements in the sysORTable
 *
\*/

/* 
sysORTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF SysOREntry
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
            "The (conceptual) table listing the capabilities of
            the local SNMP application acting as a command
            responder with respect to various MIB modules.
            SNMP entities having dynamically-configurable support
            of MIB modules will have a dynamically-varying number
            of conceptual rows."
    ::= { system 9 }

INDEX { sysORIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

sysOREntryBridge::sysOREntryBridge (int sysORIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge ("1.3.6.1.2.1.1.9",
                      BcmObjectId(sysORIndex),
                      pAgent)
{
}

sysOREntryBridge::~sysOREntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
sysORIndex OBJECT-TYPE
    SYNTAX     INTEGER (1..2147483647)
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
            "The auxiliary variable used for identifying instances
            of the columnar objects in the sysORTable."
    ::= { sysOREntry 1 }
*/ 
int sysOREntryBridge::Get_sysORIndex ()
{
  BcmObjectId Index;
  GetIndex (Index);
  return Index[0];
}


/* 
sysORID OBJECT-TYPE
    SYNTAX     OBJECT IDENTIFIER
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
            "An authoritative identification of a capabilities
            statement with respect to various MIB modules supported
            by the local SNMP application acting as a command
            responder."
    ::= { sysOREntry 2 }
*/ 
int sysOREntryBridge::Get_sysORID (BcmObjectId &sysORID)
{
  sysORID = fpAgent->sysORID(Get_sysORIndex());
  return sysORID.Length();
}


/* 
sysORDescr OBJECT-TYPE
    SYNTAX     DisplayString
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
            "A textual description of the capabilities identified
            by the corresponding instance of sysORID."
    ::= { sysOREntry 3 }
*/ 
int sysOREntryBridge::Get_sysORDescr (BcmString &sysORDescr)
{
  sysORDescr = fpAgent->sysORDescr(Get_sysORIndex());
  return sysORDescr.length();
}


/* 
sysORUpTime OBJECT-TYPE
    SYNTAX     TimeStamp
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
            "The value of sysUpTime at the time this conceptual
            row was last instantiated."
    ::= { sysOREntry 4 }
*/ 
unsigned long sysOREntryBridge::Get_sysORUpTime ()
{
  return fpAgent->sysORUpTime(Get_sysORIndex());
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/


