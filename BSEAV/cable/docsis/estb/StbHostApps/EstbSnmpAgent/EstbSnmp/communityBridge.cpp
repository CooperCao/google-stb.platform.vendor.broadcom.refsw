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
//    Filename: communityBridge.cpp
//    Creation Date: Thursday, June 20, 2002 at 12:57:29
//    Created by Broadcom V2 Mib Compiler BCMIBC.EXE version 1.2.0b
//    from input file rfc2576.mib
//
//**************************************************************************

#include "MibObjs.h"
#include "communityBridge.h"

#include "BcmSnmpAgent.h"
#include "SnmpLog.h"
#include "SnmpApi.h"


/******************************************************************************\

SNMP-COMMUNITY-MIB DEFINITIONS ::= BEGIN

IMPORTS
    IpAddress,
    MODULE-IDENTITY,
    OBJECT-TYPE,
    Integer32,
    snmpModules
        FROM SNMPv2-SMI
    RowStatus,
    StorageType
        FROM SNMPv2-TC
    SnmpAdminString,
    SnmpEngineID
        FROM SNMP-FRAMEWORK-MIB
    SnmpTagValue,
    snmpTargetAddrEntry
        FROM SNMP-TARGET-MIB
    MODULE-COMPLIANCE,
    OBJECT-GROUP
        FROM SNMPv2-CONF;

snmpCommunityMIB MODULE-IDENTITY
    -- LAST-UPDATED "200003060000Z"          - - 6 Mar 2000, midnight
    LAST-UPDATED "0003060000Z"          -- 6 Mar 2000, midnight
    ORGANIZATION "SNMPv3 Working Group"
    CONTACT-INFO "WG-email:   snmpv3@lists.tislabs.com
                  Subscribe:  majordomo@lists.tislabs.com
                              In msg body:  subscribe snmpv3

                  Chair:      Russ Mundy
                              TIS Labs at Network Associates
                  Postal:     3060 Washington Rd
                              Glenwood MD 21738
                              USA
                  Email:      mundy@tislabs.com
                  Phone:      +1-301-854-6889

                  Co-editor:  Rob Frye
                              CoSine Communications
                  Postal:     1200 Bridge Parkway
                              Redwood City, CA 94065
                              USA
                  E-mail:     rfrye@cosinecom.com
                  Phone:      +1 703 725 1130

                  Co-editor:  David B. Levi
                              Nortel Networks
                  Postal:     3505 Kesterwood Drive
                              Knoxville, TN 37918
                  E-mail:     dlevi@nortelnetworks.com
                  Phone:      +1 423 686 0432

                  Co-editor:  Shawn A. Routhier
                              Integrated Systems Inc.
                  Postal:     333 North Ave 4th Floor
                              Wakefield, MA 01880
                  E-mail:     sar@epilogue.com
                  Phone:      +1 781 245 0804

                  Co-editor:  Bert Wijnen
                              Lucent Technologies
                  Postal:     Schagen 33
                              3461 GL Linschoten
                              Netherlands
                  Email:      bwijnen@lucent.com
                  Phone:      +31-348-407-775
                 "

        DESCRIPTION
            "This MIB module defines objects to help support coexistence
             between SNMPv1, SNMPv2c, and SNMPv3."
        -- REVISION "200003060000Z" - - 6 Mar 2000
        REVISION "0003060000Z" -- 6 Mar 2000
        DESCRIPTION "This version published as RFC 2576."
        -- REVISION "199905130000Z" - - 13 May 1999
        REVISION "9905130000Z" -- 13 May 1999
        DESCRIPTION "The Initial Revision"
    ::= { snmpModules 18 }

-- Administrative assignments ****************************************

snmpCommunityMIBObjects     OBJECT IDENTIFIER ::= { snmpCommunityMIB 1 }
snmpCommunityMIBConformance OBJECT IDENTIFIER ::= { snmpCommunityMIB 2 }

--
-- The snmpCommunityTable contains a database of community strings.
-- This table provides mappings between community strings, and the
-- parameters required for View-based Access Control.
--


\******************************************************************************/




/*\
 *
 *  snmpCommunityEntryBridge for bridging elements in the snmpCommunityTable
 *
\*/

/* 
snmpCommunityTable OBJECT-TYPE
    SYNTAX       SEQUENCE OF SnmpCommunityEntry
    MAX-ACCESS   not-accessible
    STATUS       current
    DESCRIPTION
        "The table of community strings configured in the SNMP
         engine's Local Configuration Datastore (LCD)."
    ::= { snmpCommunityMIBObjects 1 }

INDEX { snmpCommunityIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

snmpCommunityEntryBridge::snmpCommunityEntryBridge (const BcmString &snmpCommunityIndex, BcmSnmpV3Agent *pAgent)
  : TabularMibBridge ("1.3.6.1.6.3.18.1.1",
                      BcmObjectId(snmpCommunityIndex, BcmObjectId::kAscii),
                      pAgent)
{
  fRowStatus = SNMP_STATUS_INVISIBLE;

  // Initialize required objects for row creation:
  fRequiredObjects |= kBIT_snmpCommunityName;
  fRequiredObjects |= kBIT_snmpCommunitySecurityName;
  
  fStorageType = CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_VOLATILE;
  fEngineId = pAgent->GetEngineId();
  
  // Find the snmpTargetAddrExtTable
  fpTargetAddrExtTable = pAgent->FindTable (BcmString("snmpTargetAddrExtTable"));

  // Programatically created entries will be
  // initialized as if created by CREATE_AND_WAIT.
  // Change to CREATE_AND_GO if immediate activation
  // is desired.
  int Status = SNMP_STATUS_CREATE_AND_WAIT;
  Set_snmpCommunityStatus (Status);
}

/* Alternate form of constructor for use by snmpCommunityTable::NewDefaultEntry */
snmpCommunityEntryBridge::snmpCommunityEntryBridge (const BcmObjectId &Index, BcmSnmpV3Agent *pAgent)
  : TabularMibBridge ("1.3.6.1.6.3.18.1.1", Index, pAgent)
{
  fRowStatus = SNMP_STATUS_INVISIBLE;

  // Initialize required objects for row creation:
  fRequiredObjects |= kBIT_snmpCommunityName;
  fRequiredObjects |= kBIT_snmpCommunitySecurityName;
  
  // Find the snmpTargetAddrExtTable
  fpTargetAddrExtTable = pAgent->FindTable (BcmString("snmpTargetAddrExtTable"));
  
  fStorageType = CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_VOLATILE;
  fEngineId = pAgent->GetEngineId();
}

snmpCommunityEntryBridge::~snmpCommunityEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
snmpCommunityIndex OBJECT-TYPE
    SYNTAX      SnmpAdminString (SIZE(1..32))
    MAX-ACCESS  not-accessible
    STATUS      current
    DESCRIPTION
        "The unique index value of a row in this table."
    ::= { snmpCommunityEntry 1 }
*/ 
int snmpCommunityEntryBridge::Get_snmpCommunityIndex (BcmString &snmpCommunityIndex)
{
  BcmObjectId Index;
  GetIndex(Index);
  Index.AsString(snmpCommunityIndex);

  return snmpCommunityIndex.length();
}


/* 
snmpCommunityName OBJECT-TYPE
    SYNTAX       OCTET STRING
    MAX-ACCESS   read-create
    STATUS       current
    DESCRIPTION
        "The community string for which a row in this table
         represents a configuration."
    ::= { snmpCommunityEntry 2 }
*/ 
int snmpCommunityEntryBridge::Get_snmpCommunityName (BcmString &snmpCommunityName)
{
  snmpCommunityName = fCommunityName;
  return snmpCommunityName.length();
}


/* 
snmpCommunitySecurityName OBJECT-TYPE
    SYNTAX       SnmpAdminString (SIZE(1..32))
    MAX-ACCESS   read-create
    STATUS       current
    DESCRIPTION
        "A human readable string representing the corresponding
         value of snmpCommunityName in a Security Model
         independent format."
    ::= { snmpCommunityEntry 3 }
*/ 
int snmpCommunityEntryBridge::Get_snmpCommunitySecurityName (BcmString &snmpCommunitySecurityName)
{
  snmpCommunitySecurityName = fSecurityName;
  return snmpCommunitySecurityName.length();
}


/* 
snmpCommunityContextEngineID OBJECT-TYPE
    SYNTAX       SnmpEngineID
    MAX-ACCESS   read-create
    STATUS       current
    DESCRIPTION
        "The contextEngineID indicating the location of the
         context in which management information is accessed
         when using the community string specified by the
         corresponding instance of snmpCommunityName.

         The default value is the snmpEngineID of the entity in
         which this object is instantiated."
    -- DEFVAL added for V2 mib compiler.  Not required for row creation.
    ::= { snmpCommunityEntry 4 }
*/ 
int snmpCommunityEntryBridge::Get_snmpCommunityContextEngineID (BcmString &snmpCommunityContextEngineID)
{
  snmpCommunityContextEngineID = fEngineId;
  return snmpCommunityContextEngineID.length();
}


/* 
snmpCommunityContextName OBJECT-TYPE
    SYNTAX       SnmpAdminString (SIZE(0..32))
    MAX-ACCESS   read-create
    STATUS       current
    DESCRIPTION
        "The context in which management information is accessed
         when using the community string specified by the corresponding
         instance of snmpCommunityName."
    DEFVAL      { ''H }   -- the empty string
    ::= { snmpCommunityEntry 5 }
*/ 
int snmpCommunityEntryBridge::Get_snmpCommunityContextName (BcmString &snmpCommunityContextName)
{
  snmpCommunityContextName = fContext;
  return snmpCommunityContextName.length();
}


/* 
snmpCommunityTransportTag OBJECT-TYPE
    SYNTAX       SnmpTagValue
    MAX-ACCESS   read-create
    STATUS       current
    DESCRIPTION
        "This object specifies a set of transport endpoints
         from which a command responder application will accept
         management requests.  If a management request containing
         this community is received on a transport endpoint other
         than the transport endpoints identified by this object,
         the request is deemed unauthentic.

         The transports identified by this object are specified
         in the snmpTargetAddrTable.  Entries in that table
         whose snmpTargetAddrTagList contains this tag value
         are identified.

         If the value of this object has zero-length, transport
         endpoints are not checked when authenticating messages
         containing this community string."
    DEFVAL      { ''H }   -- the empty string
    ::= { snmpCommunityEntry 6 }
*/ 
int snmpCommunityEntryBridge::Get_snmpCommunityTransportTag (BcmString &snmpCommunityTransportTag)
{
  snmpCommunityTransportTag = fTransportTag;
  return snmpCommunityTransportTag.length();
}


/* 
snmpCommunityStorageType OBJECT-TYPE
    SYNTAX       StorageType
    MAX-ACCESS   read-create
    STATUS       current
    DESCRIPTION
        "The storage type for this conceptual row in the
         snmpCommunityTable.  Conceptual rows having the value
         'permanent' need not allow write-access to any
         columnar object in the row."
    -- DEFVAL added for V2 mib compiler.  Not required for row creation.
    ::= { snmpCommunityEntry 7 }
*/ 
int snmpCommunityEntryBridge::Get_snmpCommunityStorageType ()
{
  return fStorageType;
}


/* 
snmpCommunityStatus OBJECT-TYPE
    SYNTAX       RowStatus
    MAX-ACCESS   read-create
    STATUS       current
    DESCRIPTION
        "The status of this conceptual row in the snmpCommunityTable.

         An entry in this table is not qualified for activation
         until instances of all corresponding columns have been
         initialized, either through default values, or through
         Set operations.  The snmpCommunityName and
         snmpCommunitySecurityName objects must be explicitly set.

         There is no restriction on setting columns in this table
         when the value of snmpCommunityStatus is active(1)."
    ::= { snmpCommunityEntry 8 }
*/ 
int snmpCommunityEntryBridge::Get_snmpCommunityStatus ()
{
  return fRowStatus;
}


/*\ 
 *  TEST methods
\*/

/* 
snmpCommunityName OBJECT-TYPE
    SYNTAX       OCTET STRING
    MAX-ACCESS   read-create
    STATUS       current
    DESCRIPTION
        "The community string for which a row in this table
         represents a configuration."
    ::= { snmpCommunityEntry 2 }
*/ 
SNMP_STATUS snmpCommunityEntryBridge::Test_snmpCommunityName (const BcmString &snmpCommunityName)
{
  // Go ahead and use default string constraings: ASCII only, max 255 chars
  SNMP_STATUS Status = StringMibObject::TestDefaults (snmpCommunityName);

  if (Status == SNMP_NO_ERROR)
    fObjectMask |= kBIT_snmpCommunityName;

  return Status;
}

/* 
snmpCommunitySecurityName OBJECT-TYPE
    SYNTAX       SnmpAdminString (SIZE(1..32))
    MAX-ACCESS   read-create
    STATUS       current
    DESCRIPTION
        "A human readable string representing the corresponding
         value of snmpCommunityName in a Security Model
         independent format."
    ::= { snmpCommunityEntry 3 }
*/ 
SNMP_STATUS snmpCommunityEntryBridge::Test_snmpCommunitySecurityName (const BcmString &snmpCommunitySecurityName)
{
  SNMP_STATUS Status = StringMibObject::TestDefaults (snmpCommunitySecurityName);

  if (Status == SNMP_NO_ERROR)
  {
    if ((snmpCommunitySecurityName.size() < MINLEN_SNMPCOMMUNITYSECURITYNAME)
    ||  (snmpCommunitySecurityName.size() > MAXLEN_SNMPCOMMUNITYSECURITYNAME))
      return SNMP_WRONG_LENGTH;

    fObjectMask |= kBIT_snmpCommunitySecurityName;
  }

  return Status;
}

/* 
snmpCommunityContextEngineID OBJECT-TYPE
    SYNTAX       SnmpEngineID
    MAX-ACCESS   read-create
    STATUS       current
    DESCRIPTION
        "The contextEngineID indicating the location of the
         context in which management information is accessed
         when using the community string specified by the
         corresponding instance of snmpCommunityName.

         The default value is the snmpEngineID of the entity in
         which this object is instantiated."
    -- DEFVAL added for V2 mib compiler.  Not required for row creation.
    ::= { snmpCommunityEntry 4 }
*/ 
SNMP_STATUS snmpCommunityEntryBridge::Test_snmpCommunityContextEngineID (const BcmString &snmpCommunityContextEngineID)
{
  //tc - EngineId is not always printable - don't call this test.
  //SNMP_STATUS Status = StringMibObject::TestDefaults (snmpCommunityContextEngineID);
  SNMP_STATUS Status = SNMP_NO_ERROR;

  if (Status == SNMP_NO_ERROR)
  {
    if ((snmpCommunityContextEngineID.size() < MINLEN_SNMPCOMMUNITYCONTEXTENGINEID)
    ||  (snmpCommunityContextEngineID.size() > MAXLEN_SNMPCOMMUNITYCONTEXTENGINEID))
      return SNMP_WRONG_LENGTH;
  }

  return Status;
}

/* 
snmpCommunityStorageType OBJECT-TYPE
    SYNTAX       StorageType
    MAX-ACCESS   read-create
    STATUS       current
    DESCRIPTION
        "The storage type for this conceptual row in the
         snmpCommunityTable.  Conceptual rows having the value
         'permanent' need not allow write-access to any
         columnar object in the row."
    -- DEFVAL added for V2 mib compiler.  Not required for row creation.
    ::= { snmpCommunityEntry 7 }
*/ 
SNMP_STATUS snmpCommunityEntryBridge::Test_snmpCommunityStorageType (int snmpCommunityStorageType)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (snmpCommunityStorageType);
    
  if (Status == SNMP_NO_ERROR)
  {
    // First, check for general wrong values.
    if ((snmpCommunityStorageType != CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_OTHER)
    &&  (snmpCommunityStorageType != CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_VOLATILE)
    &&  (snmpCommunityStorageType != CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_NONVOLATILE)
    &&  (snmpCommunityStorageType != CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_PERMANENT)
    &&  (snmpCommunityStorageType != CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_READONLY))
      return SNMP_WRONG_VALUE;
      
    // We can't ever accept a set to 'other'.  This is not explicitly stated in 
    // the StorageType TC but it is tested by SilverCreek, etc.
    if (snmpCommunityStorageType == CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_OTHER)
      return SNMP_WRONG_VALUE;
      
    /*\ 
     * Additional rules from the StorageType textual convention defined
     * in SNMPv2-TC (RFC-2579):
     *
     * "If the value of an object with this syntax is either
     *  permanent(4) or readOnly(5), it cannot be written.
     *  Conversely, if the value is either other(1), volatile(2) or
     *  nonVolatile(3), it cannot be modified to be permanent(4) or
     *  readOnly(5).  (All illegal modifications result in a
     *  'wrongValue' error.)"
    \*/
    
    // If the value of an object with this syntax is either
    // permanent(4) or readOnly(5), it cannot be written.
    if ((Get_snmpCommunityStorageType() == CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_PERMANENT)
    ||  (Get_snmpCommunityStorageType() == CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_READONLY))
      return SNMP_NOT_WRITABLE;
      
    // Conversely, if the value is either other(1), volatile(2) or
    // nonVolatile(3), it cannot be modified to be permanent(4) or
    // readOnly(5).  (All illegal modifications result in a
    // 'wrongValue' error.)
    if ((Get_snmpCommunityStorageType() == CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_OTHER)
    ||  (Get_snmpCommunityStorageType() == CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_VOLATILE)
    ||  (Get_snmpCommunityStorageType() == CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_NONVOLATILE))
    {
      if ((snmpCommunityStorageType == CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_PERMANENT)
      ||  (snmpCommunityStorageType == CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_READONLY))
        return SNMP_WRONG_VALUE;
    }

    /* Finally, apply the DOCSIS rule that we only support VOLATILE */
    
    if (snmpCommunityStorageType != CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_VOLATILE)
      return SNMP_INCONSISTENT_VALUE;
  }

  return Status;
}

/* 
snmpCommunityStatus OBJECT-TYPE
    SYNTAX       RowStatus
    MAX-ACCESS   read-create
    STATUS       current
    DESCRIPTION
        "The status of this conceptual row in the snmpCommunityTable.

         An entry in this table is not qualified for activation
         until instances of all corresponding columns have been
         initialized, either through default values, or through
         Set operations.  The snmpCommunityName and
         snmpCommunitySecurityName objects must be explicitly set.

         There is no restriction on setting columns in this table
         when the value of snmpCommunityStatus is active(1)."
    ::= { snmpCommunityEntry 8 }
*/ 
SNMP_STATUS snmpCommunityEntryBridge::Test_snmpCommunityStatus (int snmpCommunityStatus)
{
  SNMP_STATUS Status = RowStatusMibObject::TestDefaults (snmpCommunityStatus, Get_snmpCommunityStatus());
  
  // From the StorageType TEXTUAL-CONVENTION defined in SNMPv2-TC (RFC-2579):
  // "A row which is permanent(4) can be changed but not deleted."
  if ((Get_snmpCommunityStorageType() == CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_PERMANENT)
  &&  (snmpCommunityStatus == CONSTVAL_SNMPCOMMUNITYSTATUS_DESTROY))
    return SNMP_WRONG_VALUE;

  if (Status == SNMP_NO_ERROR)
    fObjectMask |= kBIT_snmpCommunityStatus;

  return Status;
}


/*\ 
 *  SET methods
\*/

/* 
snmpCommunityName OBJECT-TYPE
    SYNTAX       OCTET STRING
    MAX-ACCESS   read-create
    STATUS       current
    DESCRIPTION
        "The community string for which a row in this table
         represents a configuration."
    ::= { snmpCommunityEntry 2 }
*/ 
SNMP_STATUS snmpCommunityEntryBridge::Set_snmpCommunityName (const BcmString &snmpCommunityName)
{
  fCommunityName = snmpCommunityName;

  fObjectMask |= kBIT_snmpCommunityName;

  // If we have just gotten all of our required objects for the
  // first time, transition from notReady to notInService.
  if (((fObjectMask & fRequiredObjects) == fRequiredObjects) 
  &&  (Get_snmpCommunityStatus() == SNMP_STATUS_NOT_READY))
  {
    int snmpCommunityStatus = SNMP_STATUS_NOT_IN_SERVICE;
    Set_snmpCommunityStatus (snmpCommunityStatus);
  }

  return SNMP_NO_ERROR;
}


/* 
snmpCommunitySecurityName OBJECT-TYPE
    SYNTAX       SnmpAdminString (SIZE(1..32))
    MAX-ACCESS   read-create
    STATUS       current
    DESCRIPTION
        "A human readable string representing the corresponding
         value of snmpCommunityName in a Security Model
         independent format."
    ::= { snmpCommunityEntry 3 }
*/ 
SNMP_STATUS snmpCommunityEntryBridge::Set_snmpCommunitySecurityName (const BcmString &snmpCommunitySecurityName)
{
  fSecurityName = snmpCommunitySecurityName;

  fObjectMask |= kBIT_snmpCommunitySecurityName;

  // If we have just gotten all of our required objects for the
  // first time, transition from notReady to notInService.
  if (((fObjectMask & fRequiredObjects) == fRequiredObjects) 
  &&  (Get_snmpCommunityStatus() == SNMP_STATUS_NOT_READY))
  {
    int snmpCommunityStatus = SNMP_STATUS_NOT_IN_SERVICE;
    Set_snmpCommunityStatus (snmpCommunityStatus);
  }

  return SNMP_NO_ERROR;
}


/* 
snmpCommunityContextEngineID OBJECT-TYPE
    SYNTAX       SnmpEngineID
    MAX-ACCESS   read-create
    STATUS       current
    DESCRIPTION
        "The contextEngineID indicating the location of the
         context in which management information is accessed
         when using the community string specified by the
         corresponding instance of snmpCommunityName.

         The default value is the snmpEngineID of the entity in
         which this object is instantiated."
    -- DEFVAL added for V2 mib compiler.  Not required for row creation.
    ::= { snmpCommunityEntry 4 }
*/ 
SNMP_STATUS snmpCommunityEntryBridge::Set_snmpCommunityContextEngineID (const BcmString &snmpCommunityContextEngineID)
{
  //tc - EngineId is not always printable - don't call this test.
  //SNMP_STATUS Status = StringMibObject::TestDefaults (snmpCommunityContextEngineID);
  SNMP_STATUS Status = SNMP_NO_ERROR;

  if (Status == SNMP_NO_ERROR)
  {
    fEngineId = snmpCommunityContextEngineID;
  }

  return Status;
}


/* 
snmpCommunityContextName OBJECT-TYPE
    SYNTAX       SnmpAdminString (SIZE(0..32))
    MAX-ACCESS   read-create
    STATUS       current
    DESCRIPTION
        "The context in which management information is accessed
         when using the community string specified by the corresponding
         instance of snmpCommunityName."
    DEFVAL      { ''H }   -- the empty string
    ::= { snmpCommunityEntry 5 }
*/ 
SNMP_STATUS snmpCommunityEntryBridge::Set_snmpCommunityContextName (const BcmString &snmpCommunityContextName)
{
  SNMP_STATUS Status = StringMibObject::TestDefaults (snmpCommunityContextName);

  if (Status == SNMP_NO_ERROR)
  {
    if ((snmpCommunityContextName.size() < MINLEN_SNMPCOMMUNITYCONTEXTNAME)
    ||  (snmpCommunityContextName.size() > MAXLEN_SNMPCOMMUNITYCONTEXTNAME))
      return SNMP_WRONG_LENGTH;

    fContext = snmpCommunityContextName;
  }

  return Status;
}


/* 
snmpCommunityTransportTag OBJECT-TYPE
    SYNTAX       SnmpTagValue
    MAX-ACCESS   read-create
    STATUS       current
    DESCRIPTION
        "This object specifies a set of transport endpoints
         from which a command responder application will accept
         management requests.  If a management request containing
         this community is received on a transport endpoint other
         than the transport endpoints identified by this object,
         the request is deemed unauthentic.

         The transports identified by this object are specified
         in the snmpTargetAddrTable.  Entries in that table
         whose snmpTargetAddrTagList contains this tag value
         are identified.

         If the value of this object has zero-length, transport
         endpoints are not checked when authenticating messages
         containing this community string."
    DEFVAL      { ''H }   -- the empty string
    ::= { snmpCommunityEntry 6 }
*/ 
SNMP_STATUS snmpCommunityEntryBridge::Set_snmpCommunityTransportTag (const BcmString &snmpCommunityTransportTag)
{
  SNMP_STATUS Status = StringMibObject::TestDefaults (snmpCommunityTransportTag);

  if (Status == SNMP_NO_ERROR)
  {
    if ((snmpCommunityTransportTag.size() < MINLEN_SNMPCOMMUNITYTRANSPORTTAG)
    ||  (snmpCommunityTransportTag.size() > MAXLEN_SNMPCOMMUNITYTRANSPORTTAG))
      return SNMP_WRONG_LENGTH;

    fTransportTag = snmpCommunityTransportTag;
  }

  return Status;
}


/* 
snmpCommunityStorageType OBJECT-TYPE
    SYNTAX       StorageType
    MAX-ACCESS   read-create
    STATUS       current
    DESCRIPTION
        "The storage type for this conceptual row in the
         snmpCommunityTable.  Conceptual rows having the value
         'permanent' need not allow write-access to any
         columnar object in the row."
    -- DEFVAL added for V2 mib compiler.  Not required for row creation.
    ::= { snmpCommunityEntry 7 }
*/ 
SNMP_STATUS snmpCommunityEntryBridge::Set_snmpCommunityStorageType (int snmpCommunityStorageType)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (snmpCommunityStorageType);
    
  if (Status == SNMP_NO_ERROR)
  {
    fStorageType = snmpCommunityStorageType;
  }

  return Status;
}


/* 
snmpCommunityStatus OBJECT-TYPE
    SYNTAX       RowStatus
    MAX-ACCESS   read-create
    STATUS       current
    DESCRIPTION
        "The status of this conceptual row in the snmpCommunityTable.

         An entry in this table is not qualified for activation
         until instances of all corresponding columns have been
         initialized, either through default values, or through
         Set operations.  The snmpCommunityName and
         snmpCommunitySecurityName objects must be explicitly set.

         There is no restriction on setting columns in this table
         when the value of snmpCommunityStatus is active(1)."
    ::= { snmpCommunityEntry 8 }
*/ 
SNMP_STATUS snmpCommunityEntryBridge::Set_snmpCommunityStatus (int &snmpCommunityStatus)
{
  SNMP_STATUS Status = SetRowStatus (snmpCommunityStatus);

  if (Status == SNMP_NO_ERROR)
  {
    fObjectMask |= kBIT_snmpCommunityStatus;
    fRowStatus = snmpCommunityStatus;
  }

  return Status;
}


bool snmpCommunityEntryBridge::MatchesCommunity (const BcmString &Community)
{
  if (Get_snmpCommunityStatus() != SNMP_STATUS_ACTIVE) 
  {
    SnmpLogNm << "  Entry is not active, NO MATCH." << endl;
    return false;
  }
  
  if (Community != fCommunityName)
  {
    SnmpLogNm << "  Req cmty='" << Community << "', snmpCommunityName='" << fCommunityName << "'.  NO MATCH." << endl;
    return false;
  }
    
  return true;
}

bool snmpCommunityEntryBridge::MatchesIp (const BcmIpAddress &Ip, int SrcPort, bool ApplyTMask)
{
  // First, see if we're active.
  if (Get_snmpCommunityStatus() != SNMP_STATUS_ACTIVE)
  {
    SnmpLogNm << "  Entry is not active, NO MATCH." << endl;
    return false;
  }

  // If we have no transport tag, then we match ANY old IP
  // address.
  if (fTransportTag == "")
  {
    SnmpLogNm << "  Transport tag is empty.  MATCH." << endl;
    return true;
  }
  
  // This should never happen.  But check anyway.
  if (fpTargetAddrExtTable == NULL)
  {
    SnmpLogNm << "  snmpTargetAddrExtTable is NULL! Can't map transport tag! NO MATCH." << endl;
    return false;
  }
    
  // If we do have a transport tag, then we must iterate through all
  // entries in the snmpTargetAddrTable and check the snmpTargetAddrTAddr
  // for each active one that has our tag in his snmpTargetAddrTagList.
  // Also, we must mask snmpTargetAddrTAddr with snmpTargetAddrTMask from
  // the snmpTargetAddrExtTable.  Note that since the snmpTargetAddrTable is
  // supported by our non-portable agent and the snmpTargetAddrExtTable is
  // supported by our V2 agent, we will have the snmpTargetAddrExtTable proxy
  // for the snmpTargetAddrTable.
  
  BcmObjectId Index;
  snmpTargetAddrExtEntry *pEntry = (snmpTargetAddrExtEntry *) fpTargetAddrExtTable->FirstEntry ();
  snmpTargetAddrExtEntryBridge *pTAddr;
  
  while (pEntry)
  {
    pEntry->GetIndex(Index);
    SnmpLogNm << "  Checking snmpTargetAddrExtTable." << Index << " for IP match..." << endl;
    
    pTAddr = pEntry->fpBridge;
    if (pTAddr->MatchesIp (fTransportTag, Ip, SrcPort, ApplyTMask) == true)
    {
      SnmpLogNm << "  IP address OK.  MATCH." << endl;
      return true;
    }
    
    pEntry = (snmpTargetAddrExtEntry *) fpTargetAddrExtTable->NextEntry (Index);
  }
  
  SnmpLogNm << "  No match found for " << fTransportTag << "/" << Ip << ", NO MATCH." << endl;
  
  return false;
}

bool snmpCommunityEntryBridge::MatchesSecurityName (const BcmString &SecurityName)
{
  if (Get_snmpCommunityStatus() != SNMP_STATUS_ACTIVE) 
  {
    SnmpLogNm << "  Entry is not active, NO MATCH." << endl;
    return false;
  }
  
  if (SecurityName != fSecurityName)
  {
    SnmpLogNm << "  Req secName='" << SecurityName << "', snmpSecurityName='" << fSecurityName << "'.  NO MATCH." << endl;
    return false;
  }
    
  return true;
}


/*\
 *
 *  snmpTargetAddrExtEntryBridge for bridging elements in the snmpTargetAddrExtTable
 *
\*/

/* 
snmpTargetAddrExtTable OBJECT-TYPE
    SYNTAX       SEQUENCE OF SnmpTargetAddrExtEntry
    MAX-ACCESS   not-accessible
    STATUS       current
    DESCRIPTION
        "The table of mask and mms values associated with the
         snmpTargetAddrTable.

         The snmpTargetAddrExtTable augments the
         snmpTargetAddrTable with a transport address mask value
         and a maximum message size value.  The transport address
         mask allows entries in the snmpTargetAddrTable to define
         a set of addresses instead of just a single address.
         The maximum message size value allows the maximum
         message size of another SNMP entity to be configured for
         use in SNMPv1 (and SNMPv2c) transactions, where the
         message format does not specify a maximum message size."
    ::= { snmpCommunityMIBObjects 2 }

INDEX {  }
*/ 

/*\ 
 *  construction / destruction
\*/ 

snmpTargetAddrExtEntryBridge::snmpTargetAddrExtEntryBridge (const BcmString &snmpTargetAddrName, // index of snmpTargetAddrTable that this extends
                                  BcmSnmpV3Agent *pAgent)
  : TabularMibBridge ("1.3.6.1.6.3.18.1.2",
                      BcmObjectId (snmpTargetAddrName, BcmObjectId::kAscii),
                      pAgent)
{
  fMMS = 484;
  fTMaskIp = kAllZerosIp;
  fTMaskPort = 0;
  
  fTAddrName = snmpTargetAddrName;
}

/* Alternate form of constructor for use by snmpTargetAddrExtTable::NewDefaultEntry */
snmpTargetAddrExtEntryBridge::snmpTargetAddrExtEntryBridge (const BcmObjectId &Index, BcmSnmpV3Agent *pAgent)
  : TabularMibBridge ("1.3.6.1.6.3.18.1.2", Index, pAgent)
{
  fMMS = 484;
  fTMaskIp = kAllZerosIp;
  fTMaskPort = 0;
    
  // Stringify our index:
  for (int Loop = 0; Loop < Index.Length(); Loop++)
    fTAddrName += (char)Index[Loop];
}

snmpTargetAddrExtEntryBridge::~snmpTargetAddrExtEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
snmpTargetAddrTMask OBJECT-TYPE
    SYNTAX      OCTET STRING (SIZE (0..255))
    MAX-ACCESS  read-create
    STATUS      current
    DESCRIPTION
        "The mask value associated with an entry in the
         snmpTargetAddrTable.  The value of this object must
         have the same length as the corresponding instance of
         snmpTargetAddrTAddress, or must have length 0.  An
         attempt to set it to any other value will result in
         an inconsistentValue error.

         The value of this object allows an entry in the
         snmpTargetAddrTable to specify multiple addresses.
         The mask value is used to select which bits of
         a transport address must match bits of the corresponding
         instance of snmpTargetAddrTAddress, in order for the
         transport address to match a particular entry in the
         snmpTargetAddrTable.  Bits which are 1 in the mask
         value indicate bits in the transport address which
         must match bits in the snmpTargetAddrTAddress value.

         Bits which are 0 in the mask indicate bits in the
         transport address which need not match.  If the
         length of the mask is 0, the mask should be treated
         as if all its bits were 1 and its length were equal
         to the length of the corresponding value of
         snmpTargetAddrTable.

         This object may not be modified while the value of the
         corresponding instance of snmpTargetAddrRowStatus is
         active(1).  An attempt to set this object in this case
         will result in an inconsistentValue error."
    DEFVAL { ''H }
    ::= { snmpTargetAddrExtEntry 1 }
*/ 
int snmpTargetAddrExtEntryBridge::Get_snmpTargetAddrTMask (BcmString &snmpTargetAddrTMask)
{
  if ((fTMaskIp != kAllZerosIp)
  ||  (fTMaskPort != 0))
  {
    snmpTargetAddrTMask = fTMaskIp;
    snmpTargetAddrTMask += (unsigned char) ((fTMaskPort >> 8) & 0xFF);
    snmpTargetAddrTMask += (unsigned char) ((fTMaskPort >> 0) & 0xFF);
  }
  
  return snmpTargetAddrTMask.length();
}


/* 
snmpTargetAddrMMS OBJECT-TYPE
    SYNTAX      Integer32 (0|484..2147483647)
    MAX-ACCESS  read-create
    STATUS      current
    DESCRIPTION
        "The maximum message size value associated with an entry
         in the snmpTargetAddrTable."
    DEFVAL { 484 }
    ::= { snmpTargetAddrExtEntry 2 }
*/ 
int snmpTargetAddrExtEntryBridge::Get_snmpTargetAddrMMS ()
{
  return fMMS;
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/* 
snmpTargetAddrTMask OBJECT-TYPE
    SYNTAX      OCTET STRING (SIZE (0..255))
    MAX-ACCESS  read-create
    STATUS      current
    DESCRIPTION
        "The mask value associated with an entry in the
         snmpTargetAddrTable.  The value of this object must
         have the same length as the corresponding instance of
         snmpTargetAddrTAddress, or must have length 0.  An
         attempt to set it to any other value will result in
         an inconsistentValue error.

         The value of this object allows an entry in the
         snmpTargetAddrTable to specify multiple addresses.
         The mask value is used to select which bits of
         a transport address must match bits of the corresponding
         instance of snmpTargetAddrTAddress, in order for the
         transport address to match a particular entry in the
         snmpTargetAddrTable.  Bits which are 1 in the mask
         value indicate bits in the transport address which
         must match bits in the snmpTargetAddrTAddress value.

         Bits which are 0 in the mask indicate bits in the
         transport address which need not match.  If the
         length of the mask is 0, the mask should be treated
         as if all its bits were 1 and its length were equal
         to the length of the corresponding value of
         snmpTargetAddrTable.

         This object may not be modified while the value of the
         corresponding instance of snmpTargetAddrRowStatus is
         active(1).  An attempt to set this object in this case
         will result in an inconsistentValue error."
    DEFVAL { ''H }
    ::= { snmpTargetAddrExtEntry 1 }
*/ 
SNMP_STATUS snmpTargetAddrExtEntryBridge::Test_snmpTargetAddrTMask (const BcmString &snmpTargetAddrTMask)
{
  if ((snmpTargetAddrTMask.size() < MINLEN_SNMPTARGETADDRTMASK)
  ||  (snmpTargetAddrTMask.size() > MAXLEN_SNMPTARGETADDRTMASK))
    return SNMP_WRONG_LENGTH;

  // PR 7073: Don't allow this object to be modified whilst active. 
  BcmString CurMask;
  Get_snmpTargetAddrTMask(CurMask);
  
  // I guess "modified" means actually changing the value, so a set to what it
  // is already should be OK.
  if (snmpTargetAddrTMask != CurMask)
  {
    BcmMibRequest *pReq = LastRequestInfo();
    
    // Note: allow 'internal' sets to modify this value when the row is active.
    // Otherwise, certain kinds of programatic configuration (specifically
    // provisioning notify receivers - eg DOCSIS TLV-38) can create rows in
    // the table, then activate them, then more provisioning (eg DOCSIS
    // TLV-11) might come along and want to modify the TMask.
    if ((pReq != NULL) && (pReq->fSrcIp != kSoftwareLoopbackIp))
    {
      TargetAddr TAddr;
      if (AgentFindTargetAddr (fTAddrName.data(), fTAddrName.length(), &TAddr) == SNMP_NO_ERROR)
      {
        if (TAddr.Status == SNMP_STATUS_ACTIVE)
        {
          SnmpLogReq << "Attempt to modify snmpTargetAddrTMask while snmpTargetAddrRowStatus = ACTIVE.  INCONSISTENT_VALUE." << endl;
          return SNMP_INCONSISTENT_VALUE;
        }
      }
      else
        SnmpLogReq << "Couldn't find my snmpTargetAddrTable entry!  Proceed w/o checking RowStatus." << endl;
    }
  }
    
  // We only like IPv4 addresses.  Reject anything that's not either empty
  // or 6 bytes long.  6 bytes is because this is a TAddr value, which is
  // 4 bytes for the IP address, 2 bytes for the port number.
  if ((snmpTargetAddrTMask.size() != 0)
  &&  (snmpTargetAddrTMask.size() != 6))
  {
    SnmpLogReq << "Rejecting snmpTargetTMask that's neither 6 nor 0 bytes (IPv4 only).  INCONSISTENT_VALUE." << endl;
    return SNMP_INCONSISTENT_VALUE;
  }

  return SNMP_NO_ERROR;
}

/*\ 
 *  SET methods
\*/

/* 
snmpTargetAddrTMask OBJECT-TYPE
    SYNTAX      OCTET STRING (SIZE (0..255))
    MAX-ACCESS  read-create
    STATUS      current
    DESCRIPTION
        "The mask value associated with an entry in the
         snmpTargetAddrTable.  The value of this object must
         have the same length as the corresponding instance of
         snmpTargetAddrTAddress, or must have length 0.  An
         attempt to set it to any other value will result in
         an inconsistentValue error.

         The value of this object allows an entry in the
         snmpTargetAddrTable to specify multiple addresses.
         The mask value is used to select which bits of
         a transport address must match bits of the corresponding
         instance of snmpTargetAddrTAddress, in order for the
         transport address to match a particular entry in the
         snmpTargetAddrTable.  Bits which are 1 in the mask
         value indicate bits in the transport address which
         must match bits in the snmpTargetAddrTAddress value.

         Bits which are 0 in the mask indicate bits in the
         transport address which need not match.  If the
         length of the mask is 0, the mask should be treated
         as if all its bits were 1 and its length were equal
         to the length of the corresponding value of
         snmpTargetAddrTable.

         This object may not be modified while the value of the
         corresponding instance of snmpTargetAddrRowStatus is
         active(1).  An attempt to set this object in this case
         will result in an inconsistentValue error."
    DEFVAL { ''H }
    ::= { snmpTargetAddrExtEntry 1 }
*/ 
SNMP_STATUS snmpTargetAddrExtEntryBridge::Set_snmpTargetAddrTMask (const BcmString &snmpTargetAddrTMask)
{
  if ((snmpTargetAddrTMask.size() < MINLEN_SNMPTARGETADDRTMASK)
  ||  (snmpTargetAddrTMask.size() > MAXLEN_SNMPTARGETADDRTMASK))
    return SNMP_WRONG_LENGTH;

  if (snmpTargetAddrTMask.size() == 0)
  {
    fTMaskIp.Set (0,0,0,0);
    fTMaskPort = 0;
  }
  else if (snmpTargetAddrTMask.size() == 6)
  {
    fTMaskIp.Set(snmpTargetAddrTMask[0], snmpTargetAddrTMask[1], snmpTargetAddrTMask[2], snmpTargetAddrTMask[3]);
    fTMaskPort = (snmpTargetAddrTMask[4] << 8);
    fTMaskPort |= snmpTargetAddrTMask[5];
  }
    
  // We only like IPv4 addresses.  Reject anything that's not either empty
  // or 6 bytes long.  6 bytes is because this is a TAddr value, which is
  // 4 bytes for the IP address, 2 bytes for the port number.
  else
  {
    SnmpLogReq << "Rejecting snmpTargetTMask that's neither 6 nor 0 bytes (IPv4 only).  INCONSISTENT_VALUE." << endl;
    return SNMP_INCONSISTENT_VALUE;
  }

  return SNMP_NO_ERROR;
}


/* 
snmpTargetAddrMMS OBJECT-TYPE
    SYNTAX      Integer32 (0|484..2147483647)
    MAX-ACCESS  read-create
    STATUS      current
    DESCRIPTION
        "The maximum message size value associated with an entry
         in the snmpTargetAddrTable."
    DEFVAL { 484 }
    ::= { snmpTargetAddrExtEntry 2 }
*/ 
SNMP_STATUS snmpTargetAddrExtEntryBridge::Set_snmpTargetAddrMMS (int snmpTargetAddrMMS)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (snmpTargetAddrMMS);

  if (Status == SNMP_NO_ERROR)
  {
    if ((snmpTargetAddrMMS < MINVAL_SNMPTARGETADDRMMS)
    ||  (snmpTargetAddrMMS > MAXVAL_SNMPTARGETADDRMMS))
    {
      if (snmpTargetAddrMMS != 0)
        return SNMP_WRONG_VALUE;
    }

    fMMS = snmpTargetAddrMMS;
  }

  return Status;
}



bool snmpTargetAddrExtEntryBridge::MatchesIp (const BcmString &TransportTag, const BcmIpAddress &Ip, int SrcPort, bool ApplyTMask)
{
  TargetAddr TAddr;

  if (AgentFindTargetAddr (fTAddrName.data(), fTAddrName.length(), &TAddr) != SNMP_NO_ERROR)
  {
    SnmpLogNm << "  Couldn't find snmpTargetAddrEntry." << fTAddrName << "  NO MATCH." << endl;
    return false;
  }
  
  // Is the target addr we're proxying for active?
  if (TAddr.Status != SNMP_STATUS_ACTIVE)
  {
    SnmpLogNm << "  TAddr not active.  NO MATCH." << endl;
    return false;
  }
  
  // Check the domain for snmpUDPDomain
  BcmObjectId Domain (TAddr.DomainLen, TAddr.pDomain);
  if (Domain != BcmObjectId ("1.3.6.1.6.1.1"))
  {
    SnmpLogNm << "  TAddr domain " << Domain << " != snmpUDPDomain.  NO MATCH." << endl;
    return false;
  }
  else
    SnmpLogNm << "  TAddr domain=snmpUDPDomain (" << Domain << "), OK..." << endl;
  
  // Is the supplied transport tag present in our taglist? 
  char *pTag = strstr (TAddr.pTagList, TransportTag.c_str());
  if (pTag == NULL)
  {
    SnmpLogNm << "  Tag " << TransportTag << " not found in TagList " << BcmString (TAddr.pTagList, TAddr.TagListLen).c_str() << ". NO MATCH" << endl;
    return false;
  }
  else
  {
    SnmpLogNm << "  Tag " << TransportTag << " found in TagList " << BcmString (TAddr.pTagList, TAddr.TagListLen).c_str() << ", checking delimeters." << endl;
    
    // Check right before the tag and see if it's a delimiter.  Unless the
    // tag is the first thing in the list, indicated by the check below.
    if (pTag != TAddr.pTagList)
    {
      char *pBefore = pTag-1;
      
      // Check for valid delimeters from RFC-2573
      if ((*pBefore != ' ')  && (*pBefore != '\t')
      &&  (*pBefore != '\r') && (*pBefore != '\n'))
      {
        SnmpLogNm << "  Tag not properly delimited at start, NO MATCH." << endl;
        return false;
      }
    }
    
    // Check right after the tag and see if it's a delimter.  Unless the tag
    // is the last thing in the list, indicated by the check below.
    if ((pTag + TransportTag.length()) < (TAddr.pTagList + TAddr.TagListLen))
    {
      char *pAfter = pTag + TransportTag.length();
      
      // Check for valid delimeters from RFC-2573
      if ((*pAfter != ' ')  && (*pAfter != '\t')
      &&  (*pAfter != '\r') && (*pAfter != '\n'))
      {
        SnmpLogNm << "  Tag not properly delimited at end, NO MATCH." << endl;
        return false;
      }
    }
    
    SnmpLogNm << "  Tag delimited in TagList OK..." << endl;
  }
  
  // Now verify the addr & mask.
  BcmIpAddress TAddrIp (TAddr.pAddr[0],TAddr.pAddr[1],TAddr.pAddr[2],TAddr.pAddr[3]);
  unsigned short TAddrPort = (TAddr.pAddr[4] << 8) | TAddr.pAddr[5];
  
  // Special case: if the snmpTargetAddrTMask is an empty string, we must
  // match everything.  OR, if we are instructed to not mask.  This would be
  // in the case of matching community strings for traps.
  if ( ((fTMaskIp == kAllZerosIp) && (fTMaskPort == 0))
  ||   (ApplyTMask == false))
  {
    if (ApplyTMask)
      SnmpLogNm << "  snmpTargetAddrTMask = empty, verify IP & port" << endl;
    else
      SnmpLogNm << "  Ignoring snmpTargetAddrTMask, verify IP & port" << endl;
    
    if (Ip != TAddrIp)
    {
      SnmpLogNm << "  IP check failed.  " << Ip << " != " << TAddrIp << ".  NO MATCH." << endl;
      return false;
    }
    else
      SnmpLogNm << "  " << Ip << " == " << TAddrIp << ", OK..." << endl;
    
    if (TAddrPort != SrcPort)
    {
      SnmpLogNm << "  Port check failed.  " << TAddrPort << " != " << SrcPort << ".  NO MATCH." << endl;
      return false;
    }
    else
      SnmpLogNm << "  Port = " << SrcPort << ", OK..." << endl;
  }
  else
  {
    unsigned long ulIp, ulTAddrIp, ulTMaskIp;
    
    Ip.Get(ulIp);
    TAddrIp.Get(ulTAddrIp);
    fTMaskIp.Get(ulTMaskIp);
    
    // This doesn't work quite like you'd think, it doesn't apply the mask
    // to the comparator.  Do it by hand instead.
    // if (Ip.IsEqualWithMask (TAddrIp, fTMaskIp) == false)
    if ((ulIp & ulTMaskIp) != (ulTAddrIp &  ulTMaskIp))
    {
      SnmpLogNm << "  IP check failed. " << TAddrIp << " & " << fTMaskIp << " != " << Ip << " & " << fTMaskIp << ".  NO MATCH." << endl;
      return false;
    }
    else
      SnmpLogNm << "  " << TAddrIp << " & " << fTMaskIp << " == " << Ip << " & " << fTMaskIp << ", OK..." << endl;
    
    if ((TAddrPort & fTMaskPort) != (SrcPort & fTMaskPort))
    {
      SnmpLogNm << "  Port check failed. " << TAddrPort << " & " << fTMaskPort << " != " << SrcPort << " & " << fTMaskPort << ".  NO MATCH." << endl;
      return false;
    }
    else
      SnmpLogNm << "  Port  " << TAddrPort << " & " << fTMaskPort << " == " << SrcPort << " & " << fTMaskPort << ", OK..." << endl;
  }

  SnmpLogNm << "  Tag IP and port check out!  MATCH." << endl;
  return true;
}
