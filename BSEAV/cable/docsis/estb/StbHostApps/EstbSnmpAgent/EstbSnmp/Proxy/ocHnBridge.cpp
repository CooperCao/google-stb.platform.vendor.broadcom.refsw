//****************************************************************************
//
// Copyright (c) 2011 Broadcom Corporation
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
//****************************************************************************
//    Revision History:                                                     
//                                                                          
//****************************************************************************
//    Filename: ocHnBridge.cpp
//    Creation Date: Friday, June 10, 2011 at 11:07:23
//    Created by Broadcom BFC Mib Compiler BCMIBC.EXE version 3.6.1d
//    from input file oc-home-network.mib
//
//****************************************************************************

#include "MibObjs.h"
#include "SnmpLog.h"
#include "MergedNonVolDefaults.h"
#include "ocHnBridge.h"


/******************************************************************************\

OC-HOME-NETWORK-MIB DEFINITIONS ::= BEGIN

IMPORTS
   OBJECT-GROUP,
   MODULE-COMPLIANCE
       FROM SNMPv2-CONF              -- RFC 2580
   Integer32,
   Unsigned32,
   Counter32,
   OBJECT-TYPE,
   MODULE-IDENTITY
       FROM SNMPv2-SMI               -- RFC 2578
   TruthValue,
   RowStatus,
   MacAddress,
   TEXTUAL-CONVENTION
       FROM SNMPv2-TC                -- RFC 2579
   SnmpAdminString
       FROM SNMP-FRAMEWORK-MIB       -- RFC 3411
   InetAddressType,
   InetPortNumber,
   InetAddress
       FROM INET-ADDRESS-MIB         -- RFC 4001
   clabProjOpenCable
       FROM CLAB-DEF-MIB
   ifIndex
       FROM IF-MIB
       ;

ocHnMibModule MODULE-IDENTITY
   LAST-UPDATED "201009100000Z"        -- September 10, 2010
   ORGANIZATION "Cable Television Laboratories, Inc."
   CONTACT-INFO
        "Postal: Cable Television Laboratories, Inc.
        858 Coal Creek Circle
        Louisville, Colorado 80027-9750
        U.S.A.
        Phone: +1 303-661-9100
        Fax:     +1 303-661-9199
        E-mail: mibs@cablelabs.com"
   DESCRIPTION
           "This MIB module contains the objects for the
           management of an OpenCable Home Network. The
           MIB resides on the OpenCable Host Device if the
           Home Network Extension is implemented."
   REVISION "201009100000Z"            -- September 10, 2010
   DESCRIPTION
           "Added static PQoS configuration parameter and
           associated table."
   REVISION "201005070000Z"            -- May 7, 2010
   DESCRIPTION
           "Added device properties group."
   REVISION "200909210000Z"            -- September 21, 2009
   DESCRIPTION
           "This version is being published as part of the
           OpenCable Home Networking MIB specification I01."
   ::= { clabProjOpenCable 2 }

Tenths ::= TEXTUAL-CONVENTION
   DISPLAY-HINT "d-1"
   STATUS      current
   DESCRIPTION
           "This data type represents generic decimal units
            in Tenths. For example, a value of 5.1 will be
            represented as 51."
    SYNTAX      Integer32

PowerUnit ::= TEXTUAL-CONVENTION
    STATUS      current
    DESCRIPTION
            "Represents a well-known RSD power unit of
            measurement."
    SYNTAX      INTEGER {
                         other(1),
                         dBmV(2),
                         dBm(3),
                         mW(4)
                        }

--
-- Node definitions
--

ocHnNotifications OBJECT IDENTIFIER ::= { ocHnMibModule 1 }
ocHnMibObjects    OBJECT IDENTIFIER ::= { ocHnMibModule 2 }

ocHnHomeNetStatus OBJECT IDENTIFIER ::= { ocHnMibObjects 1 }

--
-- Reserved Services Domain Status
--

ocHnRsdManagerStatus OBJECT IDENTIFIER ::= { ocHnHomeNetStatus 1 }


\******************************************************************************/




/*\
 *
 *  ocHnDevUpnpServiceEntryBridge for bridging elements in the ocHnDevUpnpServiceTable
 *
\*/

/* 
ocHnDevUpnpServiceTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF OcHnDevUpnpServiceEntry
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "This table defines each service offered
            by this device. One row should be created
            for each supported service."
    ::= { ocHnHomeNetStatus 2 }

INDEX { ocHnDevUpnpServiceIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocHnDevUpnpServiceEntryBridge::ocHnDevUpnpServiceEntryBridge (unsigned long ocHnDevUpnpServiceIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ocHnDevUpnpServiceTable,
                      BcmObjectId(ocHnDevUpnpServiceIndex),
                      pAgent)
{
}

ocHnDevUpnpServiceEntryBridge::~ocHnDevUpnpServiceEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocHnDevUpnpServiceIndex OBJECT-TYPE
   SYNTAX      Unsigned32 (1..4294967295)
   MAX-ACCESS not-accessible
   STATUS      current
   DESCRIPTION
           "Table Index."
   ::= { ocHnDevUpnpServiceEntry 1 }
*/ 
unsigned int ocHnDevUpnpServiceEntryBridge::Get_ocHnDevUpnpServiceIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevUpnpServiceIndex" << endl;
  return 0;
}


/* 
ocHnDevUpnpServiceType OBJECT-TYPE
   SYNTAX      INTEGER {
                        cds(1),
                        srs(2),
                        qosManager(3),
                        qosDevice(4),
                        qosPolicyHolder(5),
                        avt(6),
                        cm(7),
                        rcs(8)
                        }
   MAX-ACCESS read-only
   STATUS      current
   DESCRIPTION
           "Service options currently available."
   ::= { ocHnDevUpnpServiceEntry 2 }
*/ 
int ocHnDevUpnpServiceEntryBridge::Get_ocHnDevUpnpServiceType ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevUpnpServiceType" << endl;
  return 0;
}


/* 
ocHnDevUpnpServiceAvailable OBJECT-TYPE
   SYNTAX      TruthValue
   MAX-ACCESS read-only
   STATUS      current
   DESCRIPTION
           "Service is available (true) or unavailable (false)."
   ::= { ocHnDevUpnpServiceEntry 3 }
*/ 
bool ocHnDevUpnpServiceEntryBridge::Get_ocHnDevUpnpServiceAvailable ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevUpnpServiceAvailable" << endl;
  return true;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocHnDevSupportedChannelEntryBridge for bridging elements in the ocHnDevSupportedChannelTable
 *
\*/

/* 
ocHnDevSupportedChannelTable OBJECT-TYPE
   SYNTAX      SEQUENCE OF OcHnDevSupportedChannelEntry
   MAX-ACCESS not-accessible
   STATUS      current
   DESCRIPTION
           "This table identifies the RSD Technology channels
           for each interface supported by a particular RSD node.
           This table is required for RF-based technologies."
   ::= { ocHnHomeNetStatus 3 }

INDEX { ifIndex, ocHnDevSupportedChannelIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocHnDevSupportedChannelEntryBridge::ocHnDevSupportedChannelEntryBridge (int ifIndex, unsigned long ocHnDevSupportedChannelIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ocHnDevSupportedChannelTable,
                      BcmObjectId(ifIndex) + BcmObjectId(ocHnDevSupportedChannelIndex),
                      pAgent)
{
}

ocHnDevSupportedChannelEntryBridge::~ocHnDevSupportedChannelEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocHnDevSupportedChannelIndex OBJECT-TYPE
    SYNTAX      Unsigned32 (1..4294967295)
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "Index for each channel supported by the
            interface indicated by the ifIndex."
    ::= { ocHnDevSupportedChannelEntry 1 }
*/ 
unsigned int ocHnDevSupportedChannelEntryBridge::Get_ocHnDevSupportedChannelIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevSupportedChannelIndex" << endl;
  return 0;
}


/* 
ocHnDevSupportedChannelLastOperatingFreq OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Indicates this channel/frequency
            was tuned when last in the LinkUp state.
            The value is persistent through reboots."
    ::= { ocHnDevSupportedChannelEntry 2 }
*/ 
bool ocHnDevSupportedChannelEntryBridge::Get_ocHnDevSupportedChannelLastOperatingFreq ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevSupportedChannelLastOperatingFreq" << endl;
  return true;
}


/* 
ocHnDevSupportedChannelInUse OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-write
    STATUS      current
    DESCRIPTION
            "This channel is currently being used by
            this interface."
    ::= { ocHnDevSupportedChannelEntry 3 }
*/ 
bool ocHnDevSupportedChannelEntryBridge::Get_ocHnDevSupportedChannelInUse ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevSupportedChannelInUse" << endl;
  return true;
}


/* 
ocHnDevSupportedChannelFrequency OBJECT-TYPE
    SYNTAX      Unsigned32 (0..2000000000)
    UNITS       "hertz"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Channel center frequency. This channel must be available
            for selection as the operating frequency."
    ::= { ocHnDevSupportedChannelEntry 4 }
*/ 
unsigned int ocHnDevSupportedChannelEntryBridge::Get_ocHnDevSupportedChannelFrequency ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevSupportedChannelFrequency" << endl;
  return 0;
}


/* 
ocHnDevSupportedChannelEligible OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-write
    STATUS      current
    DESCRIPTION
            "This channel is currently eligible for use."
    ::= { ocHnDevSupportedChannelEntry 5 }
*/ 
bool ocHnDevSupportedChannelEntryBridge::Get_ocHnDevSupportedChannelEligible ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevSupportedChannelEligible" << endl;
  return true;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/

/* 
ocHnDevSupportedChannelInUse OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-write
    STATUS      current
    DESCRIPTION
            "This channel is currently being used by
            this interface."
    ::= { ocHnDevSupportedChannelEntry 3 }
*/ 
SNMP_STATUS ocHnDevSupportedChannelEntryBridge::Set_ocHnDevSupportedChannelInUse (bool ocHnDevSupportedChannelInUse)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;

  {
    if ((ocHnDevSupportedChannelInUse != true) && (ocHnDevSupportedChannelInUse != false))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ocHnDevSupportedChannelInUse" << endl;
  }

  return Status;
}


/* 
ocHnDevSupportedChannelEligible OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-write
    STATUS      current
    DESCRIPTION
            "This channel is currently eligible for use."
    ::= { ocHnDevSupportedChannelEntry 5 }
*/ 
SNMP_STATUS ocHnDevSupportedChannelEntryBridge::Set_ocHnDevSupportedChannelEligible (bool ocHnDevSupportedChannelEligible)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;

  {
    if ((ocHnDevSupportedChannelEligible != true) && (ocHnDevSupportedChannelEligible != false))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ocHnDevSupportedChannelEligible" << endl;
  }

  return Status;
}






/*\
 *
 *  ocHnRsdManagerStatusGroupBridge for bridging scalars from the ocHnRsdManagerStatusGroup group 
 *
\*/

/* 
ocHnRsdManagerStatus OBJECT IDENTIFIER ::= { ocHnHomeNetStatus 1 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocHnRsdManagerStatusGroupBridge::ocHnRsdManagerStatusGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_ocHnRsdManagerStatusGroup, pAgent)
{
}

ocHnRsdManagerStatusGroupBridge::~ocHnRsdManagerStatusGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocHnRsdManagerImportanceNumber OBJECT-TYPE
    SYNTAX      Unsigned32 (0..255)
    MAX-ACCESS read-write
    STATUS      current
    DESCRIPTION
            "Configurable importance number for the RSDManager in
            this device. Used to determine the preferred RSDManager
            on the home network."
    ::= { ocHnRsdManagerStatus 1 }
*/ 
unsigned int ocHnRsdManagerStatusGroupBridge::Get_ocHnRsdManagerImportanceNumber ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnRsdManagerImportanceNumber" << endl;
  return 0;
}


/* 
ocHnRsdManagerPreferredStatus OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "True indicates this is the preferred RSD Manager
            on the home network."
    ::= { ocHnRsdManagerStatus 2 }
*/ 
bool ocHnRsdManagerStatusGroupBridge::Get_ocHnRsdManagerPreferredStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnRsdManagerPreferredStatus" << endl;
  return true;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/

/* 
ocHnRsdManagerImportanceNumber OBJECT-TYPE
    SYNTAX      Unsigned32 (0..255)
    MAX-ACCESS read-write
    STATUS      current
    DESCRIPTION
            "Configurable importance number for the RSDManager in
            this device. Used to determine the preferred RSDManager
            on the home network."
    ::= { ocHnRsdManagerStatus 1 }
*/ 
SNMP_STATUS ocHnRsdManagerStatusGroupBridge::Set_ocHnRsdManagerImportanceNumber (unsigned int ocHnRsdManagerImportanceNumber)
{
  SNMP_STATUS Status = Uint32MibObject::TestDefaults (ocHnRsdManagerImportanceNumber);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ocHnRsdManagerImportanceNumber < MINVAL_OCHNRSDMANAGERIMPORTANCENUMBER)
    ||  (ocHnRsdManagerImportanceNumber > MAXVAL_OCHNRSDMANAGERIMPORTANCENUMBER))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ocHnRsdManagerImportanceNumber" << endl;
  }

  return Status;
}






/*\
 *
 *  ocHnDevPropertiesGroupBridge for bridging scalars from the ocHnDevPropertiesGroup group 
 *
\*/

/* 
ocHnDevProperties OBJECT IDENTIFIER ::= { ocHnHomeNetStatus 4 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocHnDevPropertiesGroupBridge::ocHnDevPropertiesGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_ocHnDevPropertiesGroup, pAgent)
{
}

ocHnDevPropertiesGroupBridge::~ocHnDevPropertiesGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocHnDevPropertiesFriendlyName OBJECT-TYPE
   SYNTAX      SnmpAdminString
   MAX-ACCESS read-only
   STATUS      current
   DESCRIPTION
           "Value from the <friendlyName> element in the OCAP Root
           Device Description of the device as defined in Annex A
           of [HNP2.0]."
   ::= { ocHnDevProperties 1 }
*/ 
int ocHnDevPropertiesGroupBridge::Get_ocHnDevPropertiesFriendlyName (BcmString &ocHnDevPropertiesFriendlyName)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevPropertiesFriendlyName" << endl;
  ocHnDevPropertiesFriendlyName = "(unfinished)";

  return ocHnDevPropertiesFriendlyName.length();
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocHnDevInterfaceConfigEntryBridge for bridging elements in the ocHnDevInterfaceConfigTable
 *
\*/

/* 
ocHnDevInterfaceConfigTable OBJECT-TYPE
   SYNTAX      SEQUENCE OF OcHnDevInterfaceConfigEntry
   MAX-ACCESS not-accessible
   STATUS      current
   DESCRIPTION
           "This table augments the ifTable for each HN interface.
           It provides the mechanism for modifying and displaying
           interface characteristics not addressed by the IETF
           ifTable. The index values will serve as an external index
           to the ocHnDevStatus tables."
   ::= { ocHnDevInterfaceStatus 1 }

INDEX { ifIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocHnDevInterfaceConfigEntryBridge::ocHnDevInterfaceConfigEntryBridge (int ifIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ocHnDevInterfaceConfigTable,
                      BcmObjectId(ifIndex),
                      pAgent)
{
}

ocHnDevInterfaceConfigEntryBridge::~ocHnDevInterfaceConfigEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocHnDevInterfaceConfigMaxPowerLevel OBJECT-TYPE
   SYNTAX      Integer32
   MAX-ACCESS read-write
   STATUS      current
   DESCRIPTION
           "Maximum power level used by an RSD Technology node.
           This level is combined with the measurement units
           defined in ocHnDevInterfaceConfigPowerUnits."
   ::= { ocHnDevInterfaceConfigEntry 2 }
*/ 
int ocHnDevInterfaceConfigEntryBridge::Get_ocHnDevInterfaceConfigMaxPowerLevel ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevInterfaceConfigMaxPowerLevel" << endl;
  return 0;
}


/* 
ocHnDevInterfaceConfigPowerUnits OBJECT-TYPE
   SYNTAX      PowerUnit
   MAX-ACCESS read-only
   STATUS     current
   DESCRIPTION
           "Reports the power units of measurement/reporting
           in ocHnDevInterfacePowerUnits used by the RSD
           technology on the interface being
           reported."
   ::= { ocHnDevInterfaceConfigEntry 3 }
*/ 
int ocHnDevInterfaceConfigEntryBridge::Get_ocHnDevInterfaceConfigPowerUnits ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevInterfaceConfigPowerUnits" << endl;
  return 0;
}


/* 
ocHnDevInterfaceConfigMaxParameterizedBandwidth OBJECT-TYPE
    SYNTAX     Unsigned32
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "This object requests the maximum bandwidth, as a
           percentage of total bandwidth, to be made available for
           parameterized traffic streams. The actual bandwidth made
           available for parameterized streams is represented by
           ocHnDevInterfaceStatusMaxParameterizedBandwidth object.
           This configuration takes effect only if the node
           is the Layer2Scheduler for the RSD Technology interface
           (i.e., if the ocHnDevInterfaceStatusLayer2Scheduler object
           is set to true on this node)."
    ::= { ocHnDevInterfaceConfigEntry 4 }
*/ 
unsigned int ocHnDevInterfaceConfigEntryBridge::Get_ocHnDevInterfaceConfigMaxParameterizedBandwidth ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevInterfaceConfigMaxParameterizedBandwidth" << endl;
  return 0;
}


/* 
ocHnDevInterfaceConfigEnableEncryption OBJECT-TYPE
    SYNTAX     INTEGER
               {
               enabled(1),
               disabled(2)
               }
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "A Boolean parameter used to enable or disable privacy
           (link layer encryption) on the RSD Technology. The default
           is disabled(2)."
    ::= { ocHnDevInterfaceConfigEntry 5 }
*/ 
int ocHnDevInterfaceConfigEntryBridge::Get_ocHnDevInterfaceConfigEnableEncryption ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevInterfaceConfigEnableEncryption" << endl;
  return 0;
}


/* 
ocHnDevInterfaceConfigEncryptionPassphrase OBJECT-TYPE
    SYNTAX     SnmpAdminString (SIZE (24 | 32) )
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "A Password or a key used for encrypting data sent over
           the RSD Technology using technology-specific encryption
           algorithm."
    ::= { ocHnDevInterfaceConfigEntry 6 }
*/ 
int ocHnDevInterfaceConfigEntryBridge::Get_ocHnDevInterfaceConfigEncryptionPassphrase (BcmString &ocHnDevInterfaceConfigEncryptionPassphrase)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevInterfaceConfigEncryptionPassphrase" << endl;
  ocHnDevInterfaceConfigEncryptionPassphrase = "(unfinished)";

  return ocHnDevInterfaceConfigEncryptionPassphrase.length();
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/

/* 
ocHnDevInterfaceConfigMaxPowerLevel OBJECT-TYPE
   SYNTAX      Integer32
   MAX-ACCESS read-write
   STATUS      current
   DESCRIPTION
           "Maximum power level used by an RSD Technology node.
           This level is combined with the measurement units
           defined in ocHnDevInterfaceConfigPowerUnits."
   ::= { ocHnDevInterfaceConfigEntry 2 }
*/ 
SNMP_STATUS ocHnDevInterfaceConfigEntryBridge::Set_ocHnDevInterfaceConfigMaxPowerLevel (int ocHnDevInterfaceConfigMaxPowerLevel)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ocHnDevInterfaceConfigMaxPowerLevel);

  if (Status == SNMP_NO_ERROR)
  {
    // UNFINISHED: no constraints specified in MIB for 
    // ocHnDevInterfaceConfigMaxPowerLevel. Check if necessary.

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ocHnDevInterfaceConfigMaxPowerLevel" << endl;
  }

  return Status;
}


/* 
ocHnDevInterfaceConfigMaxParameterizedBandwidth OBJECT-TYPE
    SYNTAX     Unsigned32
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "This object requests the maximum bandwidth, as a
           percentage of total bandwidth, to be made available for
           parameterized traffic streams. The actual bandwidth made
           available for parameterized streams is represented by
           ocHnDevInterfaceStatusMaxParameterizedBandwidth object.
           This configuration takes effect only if the node
           is the Layer2Scheduler for the RSD Technology interface
           (i.e., if the ocHnDevInterfaceStatusLayer2Scheduler object
           is set to true on this node)."
    ::= { ocHnDevInterfaceConfigEntry 4 }
*/ 
SNMP_STATUS ocHnDevInterfaceConfigEntryBridge::Set_ocHnDevInterfaceConfigMaxParameterizedBandwidth (unsigned int ocHnDevInterfaceConfigMaxParameterizedBandwidth)
{
  SNMP_STATUS Status = Uint32MibObject::TestDefaults (ocHnDevInterfaceConfigMaxParameterizedBandwidth);

  if (Status == SNMP_NO_ERROR)
  {
    // UNFINISHED: no constraints specified in MIB for 
    // ocHnDevInterfaceConfigMaxParameterizedBandwidth. Check if necessary.

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ocHnDevInterfaceConfigMaxParameterizedBandwidth" << endl;
  }

  return Status;
}


/* 
ocHnDevInterfaceConfigEnableEncryption OBJECT-TYPE
    SYNTAX     INTEGER
               {
               enabled(1),
               disabled(2)
               }
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "A Boolean parameter used to enable or disable privacy
           (link layer encryption) on the RSD Technology. The default
           is disabled(2)."
    ::= { ocHnDevInterfaceConfigEntry 5 }
*/ 
SNMP_STATUS ocHnDevInterfaceConfigEntryBridge::Set_ocHnDevInterfaceConfigEnableEncryption (int ocHnDevInterfaceConfigEnableEncryption)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ocHnDevInterfaceConfigEnableEncryption);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ocHnDevInterfaceConfigEnableEncryption != CONSTVAL_OCHNDEVINTERFACECONFIGENABLEENCRYPTION_ENABLED)
    &&  (ocHnDevInterfaceConfigEnableEncryption != CONSTVAL_OCHNDEVINTERFACECONFIGENABLEENCRYPTION_DISABLED))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ocHnDevInterfaceConfigEnableEncryption" << endl;
  }

  return Status;
}


/* 
ocHnDevInterfaceConfigEncryptionPassphrase OBJECT-TYPE
    SYNTAX     SnmpAdminString (SIZE (24 | 32) )
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
           "A Password or a key used for encrypting data sent over
           the RSD Technology using technology-specific encryption
           algorithm."
    ::= { ocHnDevInterfaceConfigEntry 6 }
*/ 
SNMP_STATUS ocHnDevInterfaceConfigEntryBridge::Set_ocHnDevInterfaceConfigEncryptionPassphrase (const BcmString &ocHnDevInterfaceConfigEncryptionPassphrase)
{
  SNMP_STATUS Status = SNMP_NO_ERROR; // Binary string, don't check for ASCII

  if (Status == SNMP_NO_ERROR)
  {
    if (ocHnDevInterfaceConfigEncryptionPassphrase.size() != CONSTLEN_OCHNDEVINTERFACECONFIGENCRYPTIONPASSPHRASE)
      return SNMP_WRONG_LENGTH;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ocHnDevInterfaceConfigEncryptionPassphrase" << endl;
  }

  return Status;
}






/*\
 *
 *  ocHnDevInterfaceStatusEntryBridge for bridging elements in the ocHnDevInterfaceStatusTable
 *
\*/

/* 
ocHnDevInterfaceStatusTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF OcHnDevInterfaceStatusEntry
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
           "This table tracks and reports current network
           related attributes for each interface (e.g., MAC)."
    ::= { ocHnDevInterfaceStatus 2 }

INDEX { ifIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocHnDevInterfaceStatusEntryBridge::ocHnDevInterfaceStatusEntryBridge (int ifIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ocHnDevInterfaceStatusTable,
                      BcmObjectId(ifIndex),
                      pAgent)
{
}

ocHnDevInterfaceStatusEntryBridge::~ocHnDevInterfaceStatusEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocHnDevInterfaceStatusTxBroadcastRate OBJECT-TYPE
   SYNTAX      Unsigned32
   UNITS       "bits/sec"
   MAX-ACCESS read-only
   STATUS      current
   DESCRIPTION
           "Broadcast transmit rate (bits/sec) for this interface."
   ::= { ocHnDevInterfaceStatusEntry 1 }
*/ 
unsigned int ocHnDevInterfaceStatusEntryBridge::Get_ocHnDevInterfaceStatusTxBroadcastRate ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevInterfaceStatusTxBroadcastRate" << endl;
  return 0;
}


/* 
ocHnDevInterfaceStatusTxBroadcastLevel OBJECT-TYPE
   SYNTAX      Tenths
   MAX-ACCESS read-only
   STATUS      current
   DESCRIPTION
           "The power level used for transmission of broadcast
           messages by the RSD node reported in units defined by
           ocHnDevInterfaceStatusPowerUnits."
   ::= { ocHnDevInterfaceStatusEntry 2 }
*/ 
int ocHnDevInterfaceStatusEntryBridge::Get_ocHnDevInterfaceStatusTxBroadcastLevel ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevInterfaceStatusTxBroadcastLevel" << endl;
  return 0;
}


/* 
ocHnDevInterfaceStatusMaxTxPowerLevel OBJECT-TYPE
   SYNTAX      Tenths
   MAX-ACCESS read-only
   STATUS      current
   DESCRIPTION
           "The absolute maximum power level that a node
           is capable of transmitting. The power units
           are defined in ocHnDevInterfaceStatusPowerUnits."
   ::= { ocHnDevInterfaceStatusEntry 3 }
*/ 
int ocHnDevInterfaceStatusEntryBridge::Get_ocHnDevInterfaceStatusMaxTxPowerLevel ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevInterfaceStatusMaxTxPowerLevel" << endl;
  return 0;
}


/* 
ocHnDevInterfaceStatusPowerUnits OBJECT-TYPE
   SYNTAX      PowerUnit
   MAX-ACCESS read-only
   STATUS      current
   DESCRIPTION
           "Reports the power units of measurement/reporting
           in ocHnDevInterfacePowerUnits used by the RSD
           technology on the interface being reported."
   ::= { ocHnDevInterfaceStatusEntry 4 }
*/ 
int ocHnDevInterfaceStatusEntryBridge::Get_ocHnDevInterfaceStatusPowerUnits ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevInterfaceStatusPowerUnits" << endl;
  return 0;
}


/* 
ocHnDevInterfaceStatusMaxParameterizedBandwidth OBJECT-TYPE
   SYNTAX      Unsigned32
   MAX-ACCESS read-only
   STATUS      current
   DESCRIPTION
            "The actual bandwidth, as a percentage of total, that can
            be utilized for reservation-based (parameterized)
            traffic streams. A value of '0' should be reported if
            this node is not a layer2scheduler for the RSD Technology
            interface (i.e., if the
            ocHnDevInterfaceStatusLayer2Scheduler
            object is set to false on this node)."
    ::= { ocHnDevInterfaceStatusEntry 5 }
*/ 
unsigned int ocHnDevInterfaceStatusEntryBridge::Get_ocHnDevInterfaceStatusMaxParameterizedBandwidth ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevInterfaceStatusMaxParameterizedBandwidth" << endl;
  return 0;
}


/* 
ocHnDevInterfaceStatusLayer2Scheduler OBJECT-TYPE
    SYNTAX     TruthValue
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
            "This Boolean parameter indicates whether this particular
            node is a layer-2 scheduler."
    ::= { ocHnDevInterfaceStatusEntry 6 }
*/ 
bool ocHnDevInterfaceStatusEntryBridge::Get_ocHnDevInterfaceStatusLayer2Scheduler ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevInterfaceStatusLayer2Scheduler" << endl;
  return true;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocHnDevConnectionEntryBridge for bridging elements in the ocHnDevConnectionTable
 *
\*/

/* 
ocHnDevConnectionTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF OcHnDevConnectionEntry
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
            "This table defines the logical connections, by MAC,
            of all the devices known to this interface."
    ::= { ocHnDevInterfaceStatus 3 }

INDEX { ifIndex, ocHnDevConnectionIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocHnDevConnectionEntryBridge::ocHnDevConnectionEntryBridge (int ifIndex, unsigned long ocHnDevConnectionIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ocHnDevConnectionTable,
                      BcmObjectId(ifIndex) + BcmObjectId(ocHnDevConnectionIndex),
                      pAgent)
{
}

ocHnDevConnectionEntryBridge::~ocHnDevConnectionEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocHnDevConnectionIndex OBJECT-TYPE
   SYNTAX     Unsigned32 (1..4294967295)
   MAX-ACCESS not-accessible
   STATUS     current
   DESCRIPTION
           "Secondary index of the connection table."
   ::= { ocHnDevConnectionEntry 1 }
*/ 
unsigned int ocHnDevConnectionEntryBridge::Get_ocHnDevConnectionIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevConnectionIndex" << endl;
  return 0;
}


/* 
ocHnDevConnectionDestMac OBJECT-TYPE
   SYNTAX      MacAddress
   MAX-ACCESS read-only
   STATUS      current
   DESCRIPTION
           "MAC of other device if visible to
           this device."
   ::= { ocHnDevConnectionEntry 2 }
*/ 
void ocHnDevConnectionEntryBridge::Get_ocHnDevConnectionDestMac (BcmMacAddress &ocHnDevConnectionDestMac)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevConnectionDestMac" << endl;
  ocHnDevConnectionDestMac.Set (0,0,0,0,0,0);
}


/* 
ocHnDevConnectionRxLinkRate OBJECT-TYPE
   SYNTAX      Unsigned32
   UNITS       "bits/sec"
   MAX-ACCESS read-only
   STATUS      current
   DESCRIPTION
           "PHY reception rate (bits/sec) on this interface
           from the node identified by its MAC address."
   ::= { ocHnDevConnectionEntry 3 }
*/ 
unsigned int ocHnDevConnectionEntryBridge::Get_ocHnDevConnectionRxLinkRate ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevConnectionRxLinkRate" << endl;
  return 0;
}


/* 
ocHnDevConnectionRxModulationType OBJECT-TYPE
   SYNTAX      SnmpAdminString
   MAX-ACCESS read-only
   STATUS      current
   DESCRIPTION
           "Type of modulation currently being used by an
           interface on this node for reception from the
           node identified by MAC address. A value is required
           only if this is an RF technology-based connection.
           Non-RF interfaces should report 'n/a'."
   ::= { ocHnDevConnectionEntry 4 }
*/ 
int ocHnDevConnectionEntryBridge::Get_ocHnDevConnectionRxModulationType (BcmString &ocHnDevConnectionRxModulationType)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevConnectionRxModulationType" << endl;
  ocHnDevConnectionRxModulationType = "(unfinished)";

  return ocHnDevConnectionRxModulationType.length();
}


/* 
ocHnDevConnectionRxBroadcastRate OBJECT-TYPE
   SYNTAX      Unsigned32
   UNITS       "bits/sec"
   MAX-ACCESS read-only
   STATUS      current
   DESCRIPTION
           "Receive rate (bits/sec) at this RSD Interface for
           broadcasts initiated by the logically connected node
           identified by the destination MAC Address in
           this table entry."
   ::= { ocHnDevConnectionEntry 5 }
*/ 
unsigned int ocHnDevConnectionEntryBridge::Get_ocHnDevConnectionRxBroadcastRate ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevConnectionRxBroadcastRate" << endl;
  return 0;
}


/* 
ocHnDevConnectionRxSignalStrength OBJECT-TYPE
   SYNTAX      Tenths
   MAX-ACCESS read-only
   STATUS      current
   DESCRIPTION
           "The signal strength received at this
           interface from the associated node identified by
           the MAC address. The power units are defined
           in ocHnDevConnectionPowerUnits "
   ::= { ocHnDevConnectionEntry 6 }
*/ 
int ocHnDevConnectionEntryBridge::Get_ocHnDevConnectionRxSignalStrength ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevConnectionRxSignalStrength" << endl;
  return 0;
}


/* 
ocHnDevConnectionTxUnicastLevel OBJECT-TYPE
   SYNTAX      Tenths
   MAX-ACCESS read-only
   STATUS      current
   DESCRIPTION
           "The power level used for transmission of unicast
           messages by the RSD node reported in units
           defined in ocHnDevConnectionPowerUnits for
           this interface."
    ::= { ocHnDevConnectionEntry 7 }
*/ 
int ocHnDevConnectionEntryBridge::Get_ocHnDevConnectionTxUnicastLevel ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevConnectionTxUnicastLevel" << endl;
  return 0;
}


/* 
ocHnDevConnectionPowerUnits OBJECT-TYPE
    SYNTAX     PowerUnit
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "Reports the power units of measurement/reporting
           in ocHnDevInterfacePowerUnits used by the RSD
           technology on the interface being
           reported."
    ::= { ocHnDevConnectionEntry 8 }
*/ 
int ocHnDevConnectionEntryBridge::Get_ocHnDevConnectionPowerUnits ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevConnectionPowerUnits" << endl;
  return 0;
}


/* 
ocHnDevConnectionTxLinkRate OBJECT-TYPE
    SYNTAX     Unsigned32
    UNITS      "bits/sec"
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "PHY transmission rate (bits/sec) from this interface
           to the node identified by MAC addresses."
    ::= { ocHnDevConnectionEntry 9 }
*/ 
unsigned int ocHnDevConnectionEntryBridge::Get_ocHnDevConnectionTxLinkRate ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevConnectionTxLinkRate" << endl;
  return 0;
}


/* 
ocHnDevConnectionTxModulationType OBJECT-TYPE
    SYNTAX     SnmpAdminString
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
           "The modulation type currently employed by this
           interface while transmitting to the node
           identified by its MAC address. A value is required
           only if this is an RF technology-based connection.
           Non-RF interfaces should report 'n/a'."
    ::= { ocHnDevConnectionEntry 10 }
*/ 
int ocHnDevConnectionEntryBridge::Get_ocHnDevConnectionTxModulationType (BcmString &ocHnDevConnectionTxModulationType)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocHnDevConnectionTxModulationType" << endl;
  ocHnDevConnectionTxModulationType = "(unfinished)";

  return ocHnDevConnectionTxModulationType.length();
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





