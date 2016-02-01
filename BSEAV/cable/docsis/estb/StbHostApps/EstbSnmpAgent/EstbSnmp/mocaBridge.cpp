//****************************************************************************
//
// Copyright (c) 2009-2012 Broadcom Corporation
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
//    Filename: mocaBridge.cpp
//    Creation Date: Friday, October 16, 2009 at 10:25:11
//    Created by Broadcom BFC Mib Compiler BCMIBC.EXE version 3.6.1b
//    from input file MOCA11-MIB.mib
//
//****************************************************************************

#include "MibObjs.h"
#include "SnmpLog.h"
#include "MergedNonVolDefaults.h"
#include "mocaBridge.h"

#include <stdint.h>
#include "mocalib.h"
// #include "bcmmocacfg.h"


/******************************************************************************\

MOCA11-MIB DEFINITIONS ::= BEGIN

IMPORTS
    MODULE-IDENTITY,
    OBJECT-TYPE,
    NOTIFICATION-TYPE,
    Integer32,
    Unsigned32,
    Counter32
        FROM SNMPv2-SMI
    MODULE-COMPLIANCE,
    OBJECT-GROUP,
    NOTIFICATION-GROUP
        FROM SNMPv2-CONF
    TEXTUAL-CONVENTION,
    DisplayString,
    MacAddress,
    RowStatus,
    TruthValue
        FROM SNMPv2-TC
    ifIndex
        FROM IF-MIB
    mocaMIBs
        FROM MOCA-SMI;

moca11 MODULE-IDENTITY
    LAST-UPDATED    "200907110000Z"  -- July 11, 2009 00:00 AM
    ORGANIZATION    "Multimedia over Coax Alliance (MoCA) Technical
                    Working Group."
    CONTACT-INFO    "www.mocalliance.org

        Postal:     Multimedia over Coax Alliance
                    2400 Camino Ramon
                    Suite 375
                    San Ramon, CA 94583
                    USA
        Tel:        +1 925 275-6606
        Email:      help@mocalliance.org"
    DESCRIPTION
        "This MIB defines the objects used to remote configure, monitor,
        and diagnose the Multimedia over Coax Alliance (MoCA) network."

    REVISION        "200907110000Z"  -- July 11, 2009 00:00 AM
    DESCRIPTION
        "Second revision. The following two specifications are
        referenced in this revision of the MIB definition,
        1. MoCA MAC/PHY Specification 1.0, July 1, 2009.
        2. MoCA MAC/PHY Specification 1.1 Extensions, June 13, 2009.
        The key changes in this revision are,
        a. Updated textual convention 'MocaVersion' and 'MocaDB', and
           added 'MocaPacketAggrLevel'.
        b. Added 'mocaIfStatusChangeEnable' and
           'mocaIfNumNodesChangeEnable' to 'mocaIfConfigTable'.
        c. Renamed 'mocaIfBeaconVersion' to 'mocaIfNetworkVersion'.
        d. Renamed 'mocaIfTenPacketsAggrCapable' to
           'mocaIfPacketsAggrCapability'.
        e. Added 'mocaIfTxGcdRate' to 'mocaIfStatusTable'.
        f. Added 'mocaIfTxPacketsFlow' to 'mocaIfFlowStatsTable'.
        g. Renamed 'mocaNodeTenPacketsAggrCapable' to
           'mocaNodePacketsAggrCapability'.
        h. Added 'mocaNodeRxPackets', 'mocaNodeRxCorrected',
           'mocaNodeRxDrops' and 'mocaNodeSNR' to 'mocaNodeTable'.
        i. Added 'mocaBridgeTable' to 'mocaObjects'.
        j. Added 'mocaTrapIfStatusChange' and
           'mocaTrapIfNumNodesChange' to 'mocaNotifications'."
    REVISION        "200810030000Z"  -- October 3, 2008 00:00 AM
    DESCRIPTION
        "Initial version of this MIB module.  The following two
        specifications are referenced in this MIB definition,
        1. MoCA MAC/PHY Specification 1.0, April 5, 2007
        2. MoCA MAC/PHY Specification 1.1 Extensions, September 26,
           2007."
    ::= { mocaMIBs 1 }

--
-- Textual Conventions
--

MocaVersion ::= TEXTUAL-CONVENTION
    STATUS      current
    DESCRIPTION
        "Represent the MoCA Specification version.
        'moca10' represents MoCA Specification 1.0.
        'moca11' represents MoCA Specification 1.1."
    SYNTAX      INTEGER {
                moca1dot0 (10),
                moca1dot1 (11),
                moca1dot1ProTem (12) }

MocaNodeID ::= TEXTUAL-CONVENTION
    DISPLAY-HINT "d"
    STATUS      current
    DESCRIPTION
        "Represent a MoCA node ID.  MoCA 1.0 network can have a maximum
        of 8 nodes, so node ID is 0 to 7.  MoCA 1.1 network can have a
        maximum of 16 nodes, so node ID is 0 to 15."
    SYNTAX      Unsigned32 (0..15)

MocaChannel ::= TEXTUAL-CONVENTION
    STATUS      current
    DESCRIPTION
        "Represent the RF center frequency of a MoCA channel in MHz.
        A1      center frequency at 875 MHz
        B1      center frequency at 900 MHz
        C1      center frequency at 925 MHz
        C2      center frequency at 950 MHz
        C3      center frequency at 975 MHz
        C4      center frequency at 1000 MHz
        D1      center frequency at 1150 MHz
        D2      center frequency at 1200 MHz
        D3      center frequency at 1250 MHz
        D4      center frequency at 1300 MHz
        D5      center frequency at 1350 MHz
        D6      center frequency at 1400 MHz
        D7      center frequency at 1450 MHz
        D8      center frequency at 1500 MHz "
    REFERENCE   "MoCA MAC/PHY Specification 1.0, section 4.6."
    SYNTAX      INTEGER {
                unknown (0),
                a1 (875),
                b1 (900),
                c1 (925),
                c2 (950),
                c3 (975),
                c4 (1000),
                d1 (1150),
                d2 (1200),
                d3 (1250),
                d4 (1300),
                d5 (1350),
                d6 (1400),
                d7 (1450),
                d8 (1500) }

MocaChannelMask ::= TEXTUAL-CONVENTION
    DISPLAY-HINT "x"
    STATUS      current
    DESCRIPTION
        "Represent one or more RF center frequency of MoCA channels
        using a bitmask.  Bit 31 is the most significant bit, and bit 0
        is the least significant bit. Not all bits are valid MoCA
        channels.
        bit 0   center frequency at 800 MHz
        bit 1   center frequency at 825 MHz
        bit 2   center frequency at 850 MHz
        bit 3   center frequency at 875 MHz
        bit 4   center frequency at 900 MHz
        bit 5   center frequency at 925 MHz
        bit 6   center frequency at 950 MHz
        bit 7   center frequency at 975 MHz
        bit 8   center frequency at 1000 MHz
        bit 9   center frequency at 1025 MHz
        bit 10  center frequency at 1050 MHz
        bit 11  center frequency at 1050 MHz
        bit 12  center frequency at 1100 MHz
        bit 13  center frequency at 1125 MHz
        bit 14  center frequency at 1150 MHz
        bit 15  center frequency at 1175 MHz
        bit 16  center frequency at 1200 MHz
        bit 17  center frequency at 1225 MHz
        bit 18  center frequency at 1250 MHz
        bit 19  center frequency at 1275 MHz
        bit 20  center frequency at 1300 MHz
        bit 21  center frequency at 1325 MHz
        bit 22  center frequency at 1350 MHz
        bit 23  center frequency at 1375 MHz
        bit 24  center frequency at 1400 MHz
        bit 25  center frequency at 1425 MHz
        bit 26  center frequency at 1450 MHz
        bit 27  center frequency at 1475 MHz
        bit 28  center frequency at 1500 MHz
        bit 29  center frequency at 1525 MHz
        bit 30  center frequency at 1550 MHz
        bit 31  center frequency at 1575 MHz"
    SYNTAX      Unsigned32

MocaPhyRate ::= TEXTUAL-CONVENTION
    DISPLAY-HINT "d"
    STATUS      current
    DESCRIPTION
        "Represent the transmit PHY rate in Mbps."
    SYNTAX      Unsigned32 (0..1024)

MocaDB ::= TEXTUAL-CONVENTION
    DISPLAY-HINT "d"
    STATUS      current
    DESCRIPTION
        "Represent one decibel or 1 dB."
    SYNTAX      Unsigned32 (0..127)

MocaDBm ::= TEXTUAL-CONVENTION
    DISPLAY-HINT "d"
    STATUS      current
    DESCRIPTION
        "Represent a measure of power in mW expressed in decibels, and
        calculated as follows:
            power = 10*log10( Vrms^2 / R * 1000 )
        where Vrms is the root-mean-square Voltage of the received
        waveform and R is 75 ohms."
    SYNTAX      Integer32 (-100..27)

MocaPacketAggrLevel ::= TEXTUAL-CONVENTION
    STATUS      current
    DESCRIPTION
        "Represent the maximum number of Ethernet packets aggregated
        in a MoCA frame that is supported at transmit/receive.
        'none' represents packet aggregation is not supported,
        i.e. MoCA 1.0 or MoCA 1.1 ProTem devices.
        'aggr6' represents maximum aggregation of 6 packets.
        'aggr10' represents maximum aggregation of 10 packets."
    REFERENCE   "MoCA MAC/PHY Specification 1.1, section 10.2 and 12.2."
    SYNTAX      INTEGER {
                none (0),
                aggr6 (6),
                aggr10 (10) }

mocaNotifications   OBJECT IDENTIFIER ::= { moca11 0 }
mocaObjects         OBJECT IDENTIFIER ::= { moca11 1 }
mocaConformance     OBJECT IDENTIFIER ::= { moca11 2 }

--
-- MoCA Objects
--


\******************************************************************************/

/*\
 *
 *  mocaIfConfigEntryBridge for bridging elements in the mocaIfConfigTable
 *
\*/

/* 
mocaIfConfigTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF MocaIfConfigEntry
    MAX-ACCESS  not-accessible
    STATUS      current
    DESCRIPTION
        "MoCA interface configuration table.  This table supports the
        configuration of RF frequency, transmit power, link privacy, and
        traps related parameters.

        This table is indexed by 'ifIndex'. 

        Entries in this table cannot be created or deleted by the 
        network management system.  All entries are created or deleted 
        by the device software."
    ::= { mocaObjects 1 }

INDEX { ifIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

mocaIfConfigEntryBridge::mocaIfConfigEntryBridge (int ifIndex, struct moca_init_time *mocaInitTime, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_mocaIfConfigTable,
                      BcmObjectId(ifIndex),
                      pAgent)
{
}

mocaIfConfigEntryBridge::~mocaIfConfigEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
mocaIfEnable OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "Enable the MoCA interface if this value is 'true'.  Disable the
        MoCA interface if this value is 'false'.

        If this managed object is disabled, it may not be possible to
        enable this object via the SNMP protocol.  The mechanism to
        re-enable this object is out of the scope of this MIB definition
        draft."
    DEFVAL      { true }
    ::= { mocaIfConfigEntry 1 }
*/ 
bool mocaIfConfigEntryBridge::Get_mocaIfEnable ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_mocaIfEnable" << endl;
  return true;
}


/* 
mocaIfChannelMask OBJECT-TYPE
    SYNTAX      MocaChannelMask
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "Specify a list of RF center frequencies represented by a
        bitmask which this MoCA node is allowed to form or join a MoCA
        network.

        If the new list of frequencies does not contain the frequency
        this MoCA node is tuned to, this node must drop from the
        network.

        If this managed object is changed, it may not be possible to
        change this object again via the SNMP protocol.  The mechanism
        to change this object again is out of the scope of this MIB
        definition draft."
    REFERENCE   "MoCA MAC/PHY Specification 1.0, section 4.6.9."
    DEFVAL      { '15554000'h }
    ::= { mocaIfConfigEntry 2 }
*/ 
unsigned int mocaIfConfigEntryBridge::Get_mocaIfChannelMask ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_mocaIfChannelMask" << endl;
  return 0;
}


/* 
mocaIfPowerControl OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "Enable automatic power control if this value is 'true'.  Use a
        fixed transmit power level if this value is 'false'."
    DEFVAL      { true }
    ::= { mocaIfConfigEntry 3 }
*/ 
bool mocaIfConfigEntryBridge::Get_mocaIfPowerControl ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_mocaIfPowerControl" << endl;
  return true;
}


/* 
mocaIfTxPowerLimit OBJECT-TYPE
    SYNTAX      MocaDB
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "Specify the transmit power BACKOFF in dB.  The transmit power
        SHALL only be changed when the device 'mocaIfStatus' object is
        not 'linkUp', which means the interface is not part of a MoCA
        network."
    REFERENCE   "MoCA MAC/PHY Specification 1.0, Section 3.6.1.2,
                Table 3.5."
    DEFVAL      { 0 }
    ::= { mocaIfConfigEntry 4 }
*/ 
unsigned int mocaIfConfigEntryBridge::Get_mocaIfTxPowerLimit ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_mocaIfTxPowerLimit" << endl;
  return 0;
}


/* 
mocaIfBeaconPowerLimit OBJECT-TYPE
    SYNTAX      MocaDB
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "Specify the beacon transmit power BACKOFF in dB.  The beacon
        transmit power SHALL only be changed when the device
        'mocaIfStatus' object is not 'linkUp', which means the
        interface is not part of a MoCA network."
    REFERENCE   "MoCA MAC/PHY Specification 1.0, Section 3.6.1.2,
                Table 3.5."
    DEFVAL      { 0 }
    ::= { mocaIfConfigEntry 5 }
*/ 
unsigned int mocaIfConfigEntryBridge::Get_mocaIfBeaconPowerLimit ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_mocaIfBeaconPowerLimit" << endl;
  return 0;
}


/* 
mocaIfPowerControlTargetRate OBJECT-TYPE
    SYNTAX      MocaPhyRate
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "Specify the target transmit PHY rate in Mbps for the power
        control algorithm.  This change should be used in the next
        maintenance cycle in the MoCA network."
    REFERENCE   "MoCA MAC/PHY Specification 1.0, section 8.1.9."
    DEFVAL      { 235 }
    ::= { mocaIfConfigEntry 6 }
*/ 
unsigned int mocaIfConfigEntryBridge::Get_mocaIfPowerControlTargetRate ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_mocaIfPowerControlTargetRate" << endl;
  return 0;
}


/* 
mocaIfPrivacyEnable OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "Enable link privacy if this value is 'true', and use the
        'mocaIfPassword' to generate the MAC management key and initial
        privacy management key.  Disable link privacy and do not perform
        link encryption if this value is 'false'.

        This node will drop from the network if this value changes.

        If this managed object is disabled, it may not be possible to
        enable this object via the SNMP protocol.  The mechanism to
        re-enable this object is out of the scope of this MIB definition
        draft."
    REFERENCE   "MoCA MAC/PHY Specification 1.0, section 6.0."
    DEFVAL      { false }
    ::= { mocaIfConfigEntry 7 }
*/ 
bool mocaIfConfigEntryBridge::Get_mocaIfPrivacyEnable ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_mocaIfPrivacyEnable" << endl;
  return true;
}


/* 
mocaIfPassword OBJECT-TYPE
    SYNTAX      DisplayString (SIZE (0..17))
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "Specify the MoCA password using an ASCII numeric string.

        This value must be 12 to 17 decimal digits long.  Access is only
        allowed when used with SNMPv3 protocol, but not allowed when
        used with SNMPv1 or SNMPv2c protocol.

        If this value changes, this node will drop from the network if
        'mocaIfPrivacyEnable' is 'true'.  If this managed object is
        changed, it may not be possible to change this object again via
        the SNMP protocol. The mechanism to change this object again is 
        out of the scope of this MIB definition draft."
    REFERENCE   "MoCA MAC/PHY Specification 1.0, section 6.3."
    DEFVAL      { "" }
    ::= { mocaIfConfigEntry 8 }
*/ 
int mocaIfConfigEntryBridge::Get_mocaIfPassword (BcmString &mocaIfPassword)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_mocaIfPassword" << endl;
  mocaIfPassword = "(unfinished)";

  return mocaIfPassword.length();
}


/* 
mocaIfPreferredNC OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "Set this node as preferred NC if this value is 'true'.  Do not
        set this node as preferred NC if this value is 'false'.

        This value can be ignored by this MoCA interface when operating
        in a MoCA 1.0 network."
    DEFVAL      { false }
    ::= { mocaIfConfigEntry 9 }
*/ 
bool mocaIfConfigEntryBridge::Get_mocaIfPreferredNC ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_mocaIfPreferredNC" << endl;
  return true;
}


/* 
mocaIfAccessEnable OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "Enable MAC address access control on the MoCA interface if this
        value is 'true'.  Disable MAC address access control on the MoCA
        interface if this value is 'false'.

        Access control applies to subsequent MoCA node admission."
    DEFVAL      { false }
    ::= { mocaIfConfigEntry 10 }
*/ 
bool mocaIfConfigEntryBridge::Get_mocaIfAccessEnable ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_mocaIfAccessEnable" << endl;
  return true;
}


/* 
mocaIfPhyThreshold OBJECT-TYPE
    SYNTAX      MocaPhyRate
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "Specify a minimum PHY rate.

        If transmit PHY rate between all pairs of MoCA nodes are greater
        than or equal to this value, then transmit PHY rate of one pair
        of MoCA node is less than this value, send
        'mocaTrapBelowPhyThreshold' if 'mocaIfPhyThresholdEnable' is
        'true'.

        If transmit PHY rate between one or more pairs of MoCA nodes are
        less than this value, then transmit PHY rate between all pairs
        of MoCA nodes are greater than or equal to this value, send
        'mocaTrapAbovePhyThreshold' if 'mocaIfPhyThresholdEnable' is
        'true'."
    REFERENCE   "MoCA MAC/PHY Specification 1.0, section 8.1.5."
    DEFVAL      { 95 }
    ::= { mocaIfConfigEntry 11 }
*/ 
unsigned int mocaIfConfigEntryBridge::Get_mocaIfPhyThreshold ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_mocaIfPhyThreshold" << endl;
  return 0;
}


/* 
mocaIfPhyThresholdEnable OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "If transmit PHY rate between all pairs of MoCA nodes are
        greater than or equal to 'mocaIfPhyThreshold', then transmit PHY
        rate of one pair of MoCA node is less than 'mocaIfPhyThreshold',
        send 'mocaTrapBelowPhyThreshold' if this value is 'true', do not
        send 'mocaTrapBelowPhyThreshold' if this value is 'false'.

        If transmit PHY rate between one or more pairs of MoCA nodes are
        less than 'mocaIfPhyThreshold', then transmit PHY rate between
        all pairs of MoCA nodes are greater than or equal to
        'mocaIfPhyThreshold', send 'mocaTrapAbovePhyThreshold' if this
        value is 'true', do not send 'mocaTrapAbovePhyThreshold' if this
        value is 'false'.

        Only one node in the MoCA network should set
        'mocaIfPhyThresholdEnable' to 'true'."
    DEFVAL      { false }
    ::= { mocaIfConfigEntry 12 }
*/ 
bool mocaIfConfigEntryBridge::Get_mocaIfPhyThresholdEnable ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_mocaIfPhyThresholdEnable" << endl;
  return true;
}


/* 
mocaIfStatusChangeEnable    OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "If 'mocaIfStatusChangeEnable' is 'true' and 'mocaIfStatus'
        changes, then the trap 'mocaTrapIfStatusChange' is sent."
    DEFVAL      { false }
::= { mocaIfConfigEntry 13 }
*/ 
bool mocaIfConfigEntryBridge::Get_mocaIfStatusChangeEnable ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_mocaIfStatusChangeEnable" << endl;
  return true;
}


/* 
mocaIfNumNodesChangeEnable  OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "If 'mocaIfNumNodesChangeEnable' is 'true' and 'mocaIfNumNodes'
        changes, then the trap 'mocaTrapIfNumNodesChange' is sent.

        Only one node in the MoCA network should set 'mocaIfNumNodes'
        to 'true'."
    DEFVAL      { false }
::= { mocaIfConfigEntry 14 }
*/ 
bool mocaIfConfigEntryBridge::Get_mocaIfNumNodesChangeEnable ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_mocaIfNumNodesChangeEnable" << endl;
  return true;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/

/* 
mocaIfEnable OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "Enable the MoCA interface if this value is 'true'.  Disable the
        MoCA interface if this value is 'false'.

        If this managed object is disabled, it may not be possible to
        enable this object via the SNMP protocol.  The mechanism to
        re-enable this object is out of the scope of this MIB definition
        draft."
    DEFVAL      { true }
    ::= { mocaIfConfigEntry 1 }
*/ 
SNMP_STATUS mocaIfConfigEntryBridge::Set_mocaIfEnable (bool mocaIfEnable)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;

  {
    if ((mocaIfEnable != true) && (mocaIfEnable != false))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_mocaIfEnable" << endl;
  }

  return Status;
}


/* 
mocaIfChannelMask OBJECT-TYPE
    SYNTAX      MocaChannelMask
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "Specify a list of RF center frequencies represented by a
        bitmask which this MoCA node is allowed to form or join a MoCA
        network.

        If the new list of frequencies does not contain the frequency
        this MoCA node is tuned to, this node must drop from the
        network.

        If this managed object is changed, it may not be possible to
        change this object again via the SNMP protocol.  The mechanism
        to change this object again is out of the scope of this MIB
        definition draft."
    REFERENCE   "MoCA MAC/PHY Specification 1.0, section 4.6.9."
    DEFVAL      { '15554000'h }
    ::= { mocaIfConfigEntry 2 }
*/ 
SNMP_STATUS mocaIfConfigEntryBridge::Set_mocaIfChannelMask (unsigned int mocaIfChannelMask)
{
  SNMP_STATUS Status = Uint32MibObject::TestDefaults (mocaIfChannelMask);

  if (Status == SNMP_NO_ERROR)
  {
    // UNFINISHED: no constraints specified in MIB for 
    // mocaIfChannelMask. Check if necessary.

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_mocaIfChannelMask" << endl;
  }

  return Status;
}


/* 
mocaIfPowerControl OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "Enable automatic power control if this value is 'true'.  Use a
        fixed transmit power level if this value is 'false'."
    DEFVAL      { true }
    ::= { mocaIfConfigEntry 3 }
*/ 
SNMP_STATUS mocaIfConfigEntryBridge::Set_mocaIfPowerControl (bool mocaIfPowerControl)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;

  {
    if ((mocaIfPowerControl != true) && (mocaIfPowerControl != false))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_mocaIfPowerControl" << endl;
  }

  return Status;
}


/* 
mocaIfTxPowerLimit OBJECT-TYPE
    SYNTAX      MocaDB
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "Specify the transmit power BACKOFF in dB.  The transmit power
        SHALL only be changed when the device 'mocaIfStatus' object is
        not 'linkUp', which means the interface is not part of a MoCA
        network."
    REFERENCE   "MoCA MAC/PHY Specification 1.0, Section 3.6.1.2,
                Table 3.5."
    DEFVAL      { 0 }
    ::= { mocaIfConfigEntry 4 }
*/ 
SNMP_STATUS mocaIfConfigEntryBridge::Set_mocaIfTxPowerLimit (unsigned int mocaIfTxPowerLimit)
{
  SNMP_STATUS Status = Uint32MibObject::TestDefaults (mocaIfTxPowerLimit);

  if (Status == SNMP_NO_ERROR)
  {
    if ((mocaIfTxPowerLimit < MINVAL_MOCAIFTXPOWERLIMIT)
    ||  (mocaIfTxPowerLimit > MAXVAL_MOCAIFTXPOWERLIMIT))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_mocaIfTxPowerLimit" << endl;
  }

  return Status;
}


/* 
mocaIfBeaconPowerLimit OBJECT-TYPE
    SYNTAX      MocaDB
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "Specify the beacon transmit power BACKOFF in dB.  The beacon
        transmit power SHALL only be changed when the device
        'mocaIfStatus' object is not 'linkUp', which means the
        interface is not part of a MoCA network."
    REFERENCE   "MoCA MAC/PHY Specification 1.0, Section 3.6.1.2,
                Table 3.5."
    DEFVAL      { 0 }
    ::= { mocaIfConfigEntry 5 }
*/ 
SNMP_STATUS mocaIfConfigEntryBridge::Set_mocaIfBeaconPowerLimit (unsigned int mocaIfBeaconPowerLimit)
{
  SNMP_STATUS Status = Uint32MibObject::TestDefaults (mocaIfBeaconPowerLimit);

  if (Status == SNMP_NO_ERROR)
  {
    if ((mocaIfBeaconPowerLimit < MINVAL_MOCAIFBEACONPOWERLIMIT)
    ||  (mocaIfBeaconPowerLimit > MAXVAL_MOCAIFBEACONPOWERLIMIT))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_mocaIfBeaconPowerLimit" << endl;
  }

  return Status;
}


/* 
mocaIfPowerControlTargetRate OBJECT-TYPE
    SYNTAX      MocaPhyRate
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "Specify the target transmit PHY rate in Mbps for the power
        control algorithm.  This change should be used in the next
        maintenance cycle in the MoCA network."
    REFERENCE   "MoCA MAC/PHY Specification 1.0, section 8.1.9."
    DEFVAL      { 235 }
    ::= { mocaIfConfigEntry 6 }
*/ 
SNMP_STATUS mocaIfConfigEntryBridge::Set_mocaIfPowerControlTargetRate (unsigned int mocaIfPowerControlTargetRate)
{
  SNMP_STATUS Status = Uint32MibObject::TestDefaults (mocaIfPowerControlTargetRate);

  if (Status == SNMP_NO_ERROR)
  {
    if ((mocaIfPowerControlTargetRate < MINVAL_MOCAIFPOWERCONTROLTARGETRATE)
    ||  (mocaIfPowerControlTargetRate > MAXVAL_MOCAIFPOWERCONTROLTARGETRATE))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_mocaIfPowerControlTargetRate" << endl;
  }

  return Status;
}


/* 
mocaIfPrivacyEnable OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "Enable link privacy if this value is 'true', and use the
        'mocaIfPassword' to generate the MAC management key and initial
        privacy management key.  Disable link privacy and do not perform
        link encryption if this value is 'false'.

        This node will drop from the network if this value changes.

        If this managed object is disabled, it may not be possible to
        enable this object via the SNMP protocol.  The mechanism to
        re-enable this object is out of the scope of this MIB definition
        draft."
    REFERENCE   "MoCA MAC/PHY Specification 1.0, section 6.0."
    DEFVAL      { false }
    ::= { mocaIfConfigEntry 7 }
*/ 
SNMP_STATUS mocaIfConfigEntryBridge::Set_mocaIfPrivacyEnable (bool mocaIfPrivacyEnable)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;

  {
    if ((mocaIfPrivacyEnable != true) && (mocaIfPrivacyEnable != false))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_mocaIfPrivacyEnable" << endl;
  }

  return Status;
}


/* 
mocaIfPassword OBJECT-TYPE
    SYNTAX      DisplayString (SIZE (0..17))
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "Specify the MoCA password using an ASCII numeric string.

        This value must be 12 to 17 decimal digits long.  Access is only
        allowed when used with SNMPv3 protocol, but not allowed when
        used with SNMPv1 or SNMPv2c protocol.

        If this value changes, this node will drop from the network if
        'mocaIfPrivacyEnable' is 'true'.  If this managed object is
        changed, it may not be possible to change this object again via
        the SNMP protocol. The mechanism to change this object again is 
        out of the scope of this MIB definition draft."
    REFERENCE   "MoCA MAC/PHY Specification 1.0, section 6.3."
    DEFVAL      { "" }
    ::= { mocaIfConfigEntry 8 }
*/ 
SNMP_STATUS mocaIfConfigEntryBridge::Set_mocaIfPassword (const BcmString &mocaIfPassword)
{
  SNMP_STATUS Status = StringMibObject::TestDefaults (mocaIfPassword);

  if (Status == SNMP_NO_ERROR)
  {
    if ((mocaIfPassword.size() < MINLEN_MOCAIFPASSWORD)
    ||  (mocaIfPassword.size() > MAXLEN_MOCAIFPASSWORD))
      return SNMP_WRONG_LENGTH;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_mocaIfPassword" << endl;
  }

  return Status;
}


/* 
mocaIfPreferredNC OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "Set this node as preferred NC if this value is 'true'.  Do not
        set this node as preferred NC if this value is 'false'.

        This value can be ignored by this MoCA interface when operating
        in a MoCA 1.0 network."
    DEFVAL      { false }
    ::= { mocaIfConfigEntry 9 }
*/ 
SNMP_STATUS mocaIfConfigEntryBridge::Set_mocaIfPreferredNC (bool mocaIfPreferredNC)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;

  {
    if ((mocaIfPreferredNC != true) && (mocaIfPreferredNC != false))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_mocaIfPreferredNC" << endl;
  }

  return Status;
}


/* 
mocaIfAccessEnable OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "Enable MAC address access control on the MoCA interface if this
        value is 'true'.  Disable MAC address access control on the MoCA
        interface if this value is 'false'.

        Access control applies to subsequent MoCA node admission."
    DEFVAL      { false }
    ::= { mocaIfConfigEntry 10 }
*/ 
SNMP_STATUS mocaIfConfigEntryBridge::Set_mocaIfAccessEnable (bool mocaIfAccessEnable)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;

  {
    if ((mocaIfAccessEnable != true) && (mocaIfAccessEnable != false))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_mocaIfAccessEnable" << endl;
  }

  return Status;
}


/* 
mocaIfPhyThreshold OBJECT-TYPE
    SYNTAX      MocaPhyRate
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "Specify a minimum PHY rate.

        If transmit PHY rate between all pairs of MoCA nodes are greater
        than or equal to this value, then transmit PHY rate of one pair
        of MoCA node is less than this value, send
        'mocaTrapBelowPhyThreshold' if 'mocaIfPhyThresholdEnable' is
        'true'.

        If transmit PHY rate between one or more pairs of MoCA nodes are
        less than this value, then transmit PHY rate between all pairs
        of MoCA nodes are greater than or equal to this value, send
        'mocaTrapAbovePhyThreshold' if 'mocaIfPhyThresholdEnable' is
        'true'."
    REFERENCE   "MoCA MAC/PHY Specification 1.0, section 8.1.5."
    DEFVAL      { 95 }
    ::= { mocaIfConfigEntry 11 }
*/ 
SNMP_STATUS mocaIfConfigEntryBridge::Set_mocaIfPhyThreshold (unsigned int mocaIfPhyThreshold)
{
  SNMP_STATUS Status = Uint32MibObject::TestDefaults (mocaIfPhyThreshold);

  if (Status == SNMP_NO_ERROR)
  {
    if ((mocaIfPhyThreshold < MINVAL_MOCAIFPHYTHRESHOLD)
    ||  (mocaIfPhyThreshold > MAXVAL_MOCAIFPHYTHRESHOLD))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_mocaIfPhyThreshold" << endl;
  }

  return Status;
}


/* 
mocaIfPhyThresholdEnable OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS  read-write
    STATUS      current
    DESCRIPTION
        "If transmit PHY rate between all pairs of MoCA nodes are
        greater than or equal to 'mocaIfPhyThreshold', then transmit PHY
        rate of one pair of MoCA node is less than 'mocaIfPhyThreshold',
        send 'mocaTrapBelowPhyThreshold' if this value is 'true', do not
        send 'mocaTrapBelowPhyThreshold' if this value is 'false'.

        If transmit PHY rate between one or more pairs of MoCA nodes are
        less than 'mocaIfPhyThreshold', then transmit PHY rate between
        all pairs of MoCA nodes are greater than or equal to
        'mocaIfPhyThreshold', send 'mocaTrapAbovePhyThreshold' if this
        value is 'true', do not send 'mocaTrapAbovePhyThreshold' if this
        value is 'false'.

        Only one node in the MoCA network should set
        'mocaIfPhyThresholdEnable' to 'true'."
    DEFVAL      { false }
    ::= { mocaIfConfigEntry 12 }
*/ 
SNMP_STATUS mocaIfConfigEntryBridge::Set_mocaIfPhyThresholdEnable (bool mocaIfPhyThresholdEnable)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;

  {
    if ((mocaIfPhyThresholdEnable != true) && (mocaIfPhyThresholdEnable != false))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_mocaIfPhyThresholdEnable" << endl;
  }

  return Status;
}






/*\
 *
 *  mocaIfAccessEntryBridge for bridging elements in the mocaIfAccessTable
 *
\*/

/* 
mocaIfAccessTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF MocaIfAccessEntry
    MAX-ACCESS  not-accessible
    STATUS      current
    DESCRIPTION
        "MoCA interface MAC access control table.  This table contains a
        list of MAC addresses that are allowed to connect to the MoCA
        network. In addition to being in this access control list, a
        MoCA node must have the proper 'mocaIfChannelMask',
        'mocaIfPrivacyEnable', and 'mocaIfPassword' before the node can
        form or join a MoCA network.

        This table is indexed by 'ifIndex' and 'mocaIfAccessIndex'."
    ::= { mocaObjects 2 }

INDEX { mocaIfAccessIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

mocaIfAccessEntryBridge::mocaIfAccessEntryBridge (int mocaIfAccessIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_mocaIfAccessTable,
                      BcmObjectId(mocaIfAccessIndex),
                      pAgent)
{
  fRowStatus = SNMP_STATUS_INVISIBLE;

  // Initialize required objects for row creation:
  fRequiredObjects |= kBIT_mocaIfAccessMacAddress;

  // Programatically created entries will be
  // initialized as if created by CREATE_AND_WAIT.
  // Change to CREATE_AND_GO if immediate activation
  // is desired.
  int Status = SNMP_STATUS_CREATE_AND_WAIT;
  Set_mocaIfAccessStatus (Status);
}

/* Alternate form of constructor for use by mocaIfAccessTable::NewDefaultEntry */
mocaIfAccessEntryBridge::mocaIfAccessEntryBridge (const BcmObjectId &Index, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_mocaIfAccessTable, Index, pAgent)
{
  fRowStatus = SNMP_STATUS_INVISIBLE;

  // Initialize required objects for row creation:
  fRequiredObjects |= kBIT_mocaIfAccessMacAddress;
}

mocaIfAccessEntryBridge::~mocaIfAccessEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
mocaIfAccessIndex OBJECT-TYPE
    SYNTAX      Integer32 (1..2147483647)
    MAX-ACCESS  not-accessible
    STATUS      current
    DESCRIPTION
        "This index is used to order the MoCA interface MAC access
        control table."
    ::= { mocaIfAccessEntry 1 }
*/ 
int mocaIfAccessEntryBridge::Get_mocaIfAccessIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_mocaIfAccessIndex" << endl;
  return 0;
}


/* 
mocaIfAccessMacAddress OBJECT-TYPE
    SYNTAX      MacAddress
    MAX-ACCESS  read-create
    STATUS      current
    DESCRIPTION
        "If 'mocaIfAccessEnable' is set to 'true', MoCA node with this
        MAC address is allowed to join the MoCA network. It is not
        guaranteed that the MoCA node with this MAC address can form or
        join a network."
    ::= { mocaIfAccessEntry 2 }
*/ 
void mocaIfAccessEntryBridge::Get_mocaIfAccessMacAddress (BcmMacAddress &mocaIfAccessMacAddress)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_mocaIfAccessMacAddress" << endl;
  mocaIfAccessMacAddress.Set (0,0,0,0,0,0);
}


/* 
mocaIfAccessStatus OBJECT-TYPE
    SYNTAX      RowStatus
    MAX-ACCESS  read-create
    STATUS      current
    DESCRIPTION
        "Control and reflect the status of a row in this table.
        Creation of a row can either be done with 'CreateAndWait' or
        'CreateAndGo'."
    ::= { mocaIfAccessEntry 3 }
*/ 
int mocaIfAccessEntryBridge::Get_mocaIfAccessStatus ()
{
  return fRowStatus;
}


/*\ 
 *  TEST methods
\*/

/* 
mocaIfAccessMacAddress OBJECT-TYPE
    SYNTAX      MacAddress
    MAX-ACCESS  read-create
    STATUS      current
    DESCRIPTION
        "If 'mocaIfAccessEnable' is set to 'true', MoCA node with this
        MAC address is allowed to join the MoCA network. It is not
        guaranteed that the MoCA node with this MAC address can form or
        join a network."
    ::= { mocaIfAccessEntry 2 }
*/ 
SNMP_STATUS mocaIfAccessEntryBridge::Test_mocaIfAccessMacAddress (const BcmMacAddress &mocaIfAccessMacAddress)
{
  SNMP_STATUS Status = MacAddrMibObject::TestDefaults (mocaIfAccessMacAddress);

  if (Status == SNMP_NO_ERROR)
  {
    // UNFINISHED: no constraints specified in MIB for 
    // mocaIfAccessMacAddress. Check if necessary.

    fObjectMask |= kBIT_mocaIfAccessMacAddress;
  }

  return Status;
}

/* 
mocaIfAccessStatus OBJECT-TYPE
    SYNTAX      RowStatus
    MAX-ACCESS  read-create
    STATUS      current
    DESCRIPTION
        "Control and reflect the status of a row in this table.
        Creation of a row can either be done with 'CreateAndWait' or
        'CreateAndGo'."
    ::= { mocaIfAccessEntry 3 }
*/ 
SNMP_STATUS mocaIfAccessEntryBridge::Test_mocaIfAccessStatus (int mocaIfAccessStatus)
{
  SNMP_STATUS Status = RowStatusMibObject::TestDefaults (mocaIfAccessStatus, Get_mocaIfAccessStatus());

  if (Status == SNMP_NO_ERROR)
  {
    // UNFINISHED: check any non-default constraints here and
    // return the appropriate error code in case of failure.
    SnmpLogRaw << "UNFINISHED: Test_mocaIfAccessStatus" << endl;

    fObjectMask |= kBIT_mocaIfAccessStatus;
  }

  return Status;
}


/*\ 
 *  SET methods
\*/

/* 
mocaIfAccessMacAddress OBJECT-TYPE
    SYNTAX      MacAddress
    MAX-ACCESS  read-create
    STATUS      current
    DESCRIPTION
        "If 'mocaIfAccessEnable' is set to 'true', MoCA node with this
        MAC address is allowed to join the MoCA network. It is not
        guaranteed that the MoCA node with this MAC address can form or
        join a network."
    ::= { mocaIfAccessEntry 2 }
*/ 
SNMP_STATUS mocaIfAccessEntryBridge::Set_mocaIfAccessMacAddress (const BcmMacAddress &mocaIfAccessMacAddress)
{
  // UNFINISHED: really set something here.
  SnmpLogRaw << "UNFINISHED: Set_mocaIfAccessMacAddress" << endl;

  fObjectMask |= kBIT_mocaIfAccessMacAddress;

  // If we have just gotten all of our required objects for the
  // first time, transition from notReady to notInService.
  if (((fObjectMask & fRequiredObjects) == fRequiredObjects) 
  &&  (Get_mocaIfAccessStatus() == SNMP_STATUS_NOT_READY))
  {
    int mocaIfAccessStatus = SNMP_STATUS_NOT_IN_SERVICE;
    Set_mocaIfAccessStatus (mocaIfAccessStatus);
  }

  return SNMP_NO_ERROR;
}


/* 
mocaIfAccessStatus OBJECT-TYPE
    SYNTAX      RowStatus
    MAX-ACCESS  read-create
    STATUS      current
    DESCRIPTION
        "Control and reflect the status of a row in this table.
        Creation of a row can either be done with 'CreateAndWait' or
        'CreateAndGo'."
    ::= { mocaIfAccessEntry 3 }
*/ 
SNMP_STATUS mocaIfAccessEntryBridge::Set_mocaIfAccessStatus (int &mocaIfAccessStatus)
{
  SNMP_STATUS Status = SetRowStatus (mocaIfAccessStatus);

  if (Status == SNMP_NO_ERROR)
  {
    fObjectMask |= kBIT_mocaIfAccessStatus;
    fRowStatus = mocaIfAccessStatus;

    // UNFINISHED: really set something here!
    SnmpLogRaw << "UNFINISHED: Set_mocaIfAccessStatus" << endl;
  }

  return Status;
}






/*\
 *
 *  mocaIfStatusEntryBridge for bridging elements in the mocaIfStatusTable
 *
\*/

/* 
mocaIfStatusTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF MocaIfStatusEntry
    MAX-ACCESS  not-accessible
    STATUS      current
    DESCRIPTION
        "MoCA interface status information table.  This table provides
        features supported and operation parameters of the MoCA
        interface.

        This table is indexed by 'ifIndex'.

        Entries in this table cannot be created or deleted by the
        network management system. All entries are created or deleted by
        the device software."
    ::= { mocaObjects 3 }

INDEX { ifIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 


static void pqos_create_response_status_cb(void *arg, struct moca_pqos_create_response *in)
{
    mocaIfStatusEntryBridge *x = (mocaIfStatusEntryBridge *)arg;
    
    x->Assign_mocaIfMaxIngressNodeBw(in->maxpeakdatarate);
    x->Assign_mocaIfMaxEgressNodeBw(in->maxpeakdatarate); 

    moca_cancel_event_loop(x->MocaHandle);
}



mocaIfStatusEntryBridge::mocaIfStatusEntryBridge (int ifIndex, 
                                   void *mocaHandle, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_mocaIfStatusTable,
                      BcmObjectId(ifIndex),
                      pAgent)
{
  struct moca_gen_status MocaGenStatus;
  struct moca_drv_info MocaDrvInfo;
  struct moca_init_time MocaInitTime;
  struct moca_tx_profile MocaTxProfile;
  MoCA_NODE_STATUS_ENTRY MocaNodeStatusEntry;
  char swVers[81] = "";
  uint8_t macAddr[6];
  uint32_t versMajor, versMinor, versBuild ;
  int count;


  // mocaHandle is needed for the callback, so save it!
  MocaHandle = mocaHandle;

  bzero(&MocaGenStatus, sizeof(MocaGenStatus));
  bzero(&MocaDrvInfo, sizeof(MocaDrvInfo));
  bzero(&MocaInitTime, sizeof(MocaInitTime));
  bzero(&MocaTxProfile, sizeof(MocaTxProfile));
  bzero(&MocaNodeStatusEntry, sizeof(MocaNodeStatusEntry));

  // Set our fixed values
  lMocaIfPacketsAggrCapability = 10;

  // Default all other values values
  lMocaIfStatus = lMocaIfMocaVersion = lMocaIfNetworkVersion = 
  lMocaIfNodeID = lMocaIfNumNodes = lMocaIfNC = lMocaIfBackupNC = 
  lMocaIfRFChannel = lMocaIfTabooChannelMask = lMocaIfTxGcdPowerReduction = 
  lMocaIfQAM256Capable = lMocaIfLinkUpTime = lMocaIfLOF = 
  lMocaIfNodeTabooChannelMask = lMocaIfCapabilityMask = 
  lMocaIfTxGcdRate = lMocaIfMaxIngressNodeBw = lMocaIfMaxEgressNodeBw = 0;
  lMocaIfSoftwareVersion = ""; lMocaIfName = "";
  lMocaIfMacAddress = kAllZerosMac;

  // cout << "In mocaIfStatusEntryBridge calling moca_get_gen_status" << endl;
  if (!moca_get_gen_status(mocaHandle, &MocaGenStatus)) {
    if (MocaGenStatus.operation_status)
      if (MocaGenStatus.link_status)
        lMocaIfStatus = CONSTVAL_MOCAIFSTATUS_LINKUP;
      else
        lMocaIfStatus = CONSTVAL_MOCAIFSTATUS_NOLINK;
    else
      lMocaIfStatus = CONSTVAL_MOCAIFSTATUS_DISABLED;
    versMajor = MocaGenStatus.moca_sw_version >> 28 ;
    versMinor = (MocaGenStatus.moca_sw_version << 4) >> 28 ;
    versBuild = (MocaGenStatus.moca_sw_version << 8) >> 28 ;
    sprintf(swVers, "SW %d.%d.%d HW %x", versMajor, versMinor, versBuild,
                                MocaGenStatus.moca_hw_version);
    lMocaIfSoftwareVersion = swVers;
    lMocaIfMocaVersion = MocaGenStatus.self_moca_version;
    lMocaIfNetworkVersion = MocaGenStatus.network_moca_version;
    lMocaIfNodeID = MocaGenStatus.node_id;
    lMocaIfNC = MocaGenStatus.nc_node_id;
    lMocaIfBackupNC = MocaGenStatus.backup_nc_id;
    lMocaIfRFChannel = MocaGenStatus.rf_channel;
    lMocaIfTabooChannelMask = MocaGenStatus.network_taboo_mask;
    lMocaIfTxGcdPowerReduction = MocaGenStatus.tx_gcd_power_reduction;
    lMocaIfQAM256Capable = MocaGenStatus.qam_256_support;
    for (count = 0; count < MOCA_MAX_NODES; count++)
      if (MocaGenStatus.connected_nodes & (0x1 << count))
        lMocaIfNumNodes++;
  }
  // cout << "In mocaIfStatusEntryBridge calling moca_get_drv_info" << endl;
  if (!moca_get_drv_info(mocaHandle, &MocaDrvInfo)) {
    lMocaIfLinkUpTime = MocaDrvInfo.link_uptime;
    lMocaIfName = (char *) (MocaDrvInfo.ifname);
  }

  // cout << "In mocaIfStatusEntryBridge calling moca_get_init_time" << endl;
  if (!moca_get_init_time(mocaHandle, &MocaInitTime)) {
    moca_u32_to_mac(macAddr, MocaInitTime.mac_addr_hi, 
                    MocaInitTime.mac_addr_lo);
    lMocaIfMacAddress = macAddr;
    lMocaIfLOF = MocaInitTime.lof;
    lMocaIfNodeTabooChannelMask = MocaInitTime.taboo_fixed_channel_mask;
    lMocaIfCapabilityMask = MocaInitTime.freq_mask;
  }

  // cout << "In mocaIfStatusEntryBridge calling moca_get_tx_profile" << endl;
  if (!moca_get_tx_profile(mocaHandle, (uint32_t) lMocaIfNodeID, &MocaTxProfile)) {
    lMocaIfTxGcdRate = moca_phy_rate(MocaTxProfile.nbas, MocaTxProfile.cp, MocaTxProfile.turbo_status);
  }

  MocaNodeStatusEntry.nodeId = lMocaIfNodeID;
  // cout << "In mocaIfStatusEntryBridge calling MoCACtl2_GetNodeStatus" << endl;
  if (!MoCACtl2_GetNodeStatus(mocaHandle, &MocaNodeStatusEntry)) {
    lMocaIfMaxIngressNodeBw = MocaNodeStatusEntry.maxPhyRates.rxUcPhyRate;
    lMocaIfMaxEgressNodeBw = MocaNodeStatusEntry.maxPhyRates.txUcPhyRate;
  }

//  cout << "In mocaIfStatusEntryBridge calling moca_register_pqos_create_response_cb" << endl;
//  moca_register_pqos_create_response_cb(mocaHandle, 
//                                        &pqos_create_response_status_cb, 
//                                        (void *)this);
//  cout << "In mocaIfStatusEntryBridge calling MoCACtl2_GetPQoSStatus" << endl;
//  MoCACtl2_GetPQoSStatus(mocaHandle);
//  cout << "In mocaIfStatusEntryBridge calling moca_event_loop" << endl;
//    moca_event_loop(mocaHandle);
  
//  cout << "In mocaIfStatusEntryBridge calling moca_register_pqos_create_response_cb" << endl;
//  moca_register_pqos_create_response_cb(mocaHandle, NULL, NULL);  
}

mocaIfStatusEntryBridge::~mocaIfStatusEntryBridge ()
{
  if (MocaHandle)
    moca_close( MocaHandle );
  MocaHandle = NULL;
}

/*\ 
 *  GET methods
\*/

/* 
mocaIfStatus OBJECT-TYPE
    SYNTAX      INTEGER {
                disabled (1),
                noLink (2),
                linkUp (3) }
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the current status of the MoCA interface.

        'disabled' indicates interface is disabled.  It will not be
        possible to read the 'disabled' status of the MoCA interface if
        this managed node does not have another local port
        (e.g. Ethernet) where SNMP objects can be accessed.
        'noLink' indicates interface is enabled but not part of a
        network.
        'linkUp' indicates interface is enabled and in a network."
    ::= { mocaIfStatusEntry 1 }
*/ 
int mocaIfStatusEntryBridge::Get_mocaIfStatus ()
{
  return lMocaIfStatus;
}


/* 
mocaIfLinkUpTime OBJECT-TYPE
    SYNTAX      Unsigned32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the total number of seconds that this interface is
        part of a MoCA network.

        This may be used with 'sysUpTime' to determine the link
        availability in the MoCA interface. Note that 'sysUpTime' is in
        10 milliseconds and 'mocaIfLinkUpTime' is in seconds."
    ::= { mocaIfStatusEntry 2 }
*/ 
unsigned int mocaIfStatusEntryBridge::Get_mocaIfLinkUpTime ()
{
  return lMocaIfLinkUpTime;
}


/* 
mocaIfSoftwareVersion OBJECT-TYPE
    SYNTAX      DisplayString (SIZE (0..80))
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the software version of the MoCA device.  This should
        be the same as the product software version in the MoCA
        certificate."
    ::= { mocaIfStatusEntry 3 }
*/ 
int mocaIfStatusEntryBridge::Get_mocaIfSoftwareVersion (BcmString &mocaIfSoftwareVersion)
{
  mocaIfSoftwareVersion = lMocaIfSoftwareVersion;

  return lMocaIfSoftwareVersion.length();
}


/* 
mocaIfMocaVersion OBJECT-TYPE
    SYNTAX      MocaVersion
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the MoCA version supported by this MoCA interface."
    ::= { mocaIfStatusEntry 4 }
*/ 
int mocaIfStatusEntryBridge::Get_mocaIfMocaVersion ()
{
  return lMocaIfMocaVersion;
}


/* 
mocaIfNetworkVersion OBJECT-TYPE
    SYNTAX      MocaVersion
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the MoCA version used in this MoCA network.  If this
        interface is not part of a MoCA network, report
        'mocaIfMocaVersion'.

        This value uses the MOCA_VERSION in the Beacon Frame, but
        requires determining whether any nodes in the network are
        MoCA 1.1 ProTem, which distinguishes a MoCA 1.1 network,
        or 'moca1dot1', from a MoCA 1.1 ProTem network, or
        'moca1dot1ProTem'."
    REFERENCE   "MoCA MAC/PHY Specification 1.0, section 3.3.6."
    ::= { mocaIfStatusEntry 5 }
*/ 
int mocaIfStatusEntryBridge::Get_mocaIfNetworkVersion ()
{
  return lMocaIfNetworkVersion;
}


/* 
mocaIfMacAddress OBJECT-TYPE
    SYNTAX      MacAddress
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the MAC address of this MoCA interface.  This MAC
        address is encoded in the first six bytes of the Globally Unique
        Identifier (GUID).

        For example, MoCA interface with MAC address aa:bb:cc:dd:ee:ff
        will have a GUID of aa:bb:cc:dd:ee:ff:00:00."
    ::= { mocaIfStatusEntry 6 }
*/ 
void mocaIfStatusEntryBridge::Get_mocaIfMacAddress (BcmMacAddress &mocaIfMacAddress)
{
  mocaIfMacAddress = lMocaIfMacAddress;
}


/* 
mocaIfNodeID OBJECT-TYPE
    SYNTAX      MocaNodeID
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the node ID of this MoCA interface.  If this interface
        is not part of a MoCA network, report a value of '0'."
    ::= { mocaIfStatusEntry 7 }
*/ 
unsigned int mocaIfStatusEntryBridge::Get_mocaIfNodeID ()
{
  return lMocaIfNodeID;
}


/* 
mocaIfName OBJECT-TYPE
    SYNTAX      DisplayString (SIZE (0..16))
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the textual name of this MoCA interface.  The value of
        this object should be the name of the interface as assigned by
        the MoCA device.

        Since MoCA has an Ethernet convergence layer, this name may be
        'ethN' where 'N' is the port number."
    ::= { mocaIfStatusEntry 8 }
*/ 
int mocaIfStatusEntryBridge::Get_mocaIfName (BcmString &mocaIfName)
{
  mocaIfName = lMocaIfName;

  return mocaIfName.length();
}


/* 
mocaIfNumNodes OBJECT-TYPE
    SYNTAX      Unsigned32 (0..16)
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the numbers of 1's in the GCD_BITMASK field reported
        in Type I Probe Reports.  This value corresponds to the number
        of nodes that this node communicates to in the MoCA network.

        This value may be smaller than the number of nodes reported by
        the NC node."
    REFERENCE   "MoCA MAC/PHY Specification 1.0, section 3.6.3.2."
    ::= { mocaIfStatusEntry 9 }
*/ 
unsigned int mocaIfStatusEntryBridge::Get_mocaIfNumNodes ()
{
  return lMocaIfNumNodes;
}


/* 
mocaIfNC OBJECT-TYPE
    SYNTAX      MocaNodeID
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the node ID of the network coordinator.  If this
        interface is not part of a MoCA network, report a value of '0'."
    ::= { mocaIfStatusEntry 10 }
*/ 
unsigned int mocaIfStatusEntryBridge::Get_mocaIfNC ()
{
  return lMocaIfNC;
}


/* 
mocaIfBackupNC OBJECT-TYPE
    SYNTAX      MocaNodeID
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the node ID of the backup network coordinator.  If
        this interface is not part of a MoCA network, report a value of
        '0'."
    ::= { mocaIfStatusEntry 11 }
*/ 
unsigned int mocaIfStatusEntryBridge::Get_mocaIfBackupNC ()
{
  return lMocaIfBackupNC;
}


/* 
mocaIfRFChannel OBJECT-TYPE
    SYNTAX      MocaChannel
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the MoCA channel in MHz this interface is tuned to
        when part of a MoCA network.  When not part of a MoCA network
        this value may not reflect the actual tuned channel.  If
        'mocaIfEnable' is 'false', report a value of 'unknown'."
    ::= { mocaIfStatusEntry 12 }
*/ 
int mocaIfStatusEntryBridge::Get_mocaIfRFChannel ()
{
  return lMocaIfRFChannel * 25;
}


/* 
mocaIfLOF OBJECT-TYPE
    SYNTAX      MocaChannel
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the MoCA channel in MHz this interface is tuned to
        when it was last in the 'linkUp' state.

        If this interface is never part of a MoCA network, report the
        factory default Last Operational Frequency (LOF)."
    REFERENCE   "MoCA MAC/PHY Specification 1.0, section 3.4.1."
    ::= { mocaIfStatusEntry 13 }
*/ 
int mocaIfStatusEntryBridge::Get_mocaIfLOF ()
{
  return lMocaIfLOF;
}


/* 
mocaIfTabooChannelMask OBJECT-TYPE
    SYNTAX      MocaChannelMask
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the list of taboo channels in this MoCA network
        represented as a bitmask.

        This value is derived from TABOO_MASK_START and
        TABOO_CHANNEL_MASK in the beacon, but has a different data
        representation.  E.g. If taboo channels consists of 1300, 1350
        and 1400 MHz, TABOO_MASK_START is 52, TABOO_CHANNEL_MASK is
        'A8000000'h, and 'mocaIfTabooChannelMask' is '01500000'h.

        Note that in 'mocaIfTabooChannelMask', the lowest represented
        frequency of 800 MHz is represented in bit 0, and increasing bit
        position represents increasingly higher frequency. While in
        TABOO_CHANNEL_MASK, the taboo channel with the lowest frequency
        is represented in bit 31, and decreasing bit position represents
        increasingly higher frequency.

        If this interface is not sending or receiving beacon, or there
        is no taboo channel in this MoCA network, report a value of
        '0'."
    REFERENCE   "MoCA MAC/PHY Specification 1.0, section 3.3.4."
    ::= { mocaIfStatusEntry 14 }
*/ 
unsigned int mocaIfStatusEntryBridge::Get_mocaIfTabooChannelMask ()
{
  return lMocaIfTabooChannelMask;
}


/* 
mocaIfNodeTabooChannelMask OBJECT-TYPE
    SYNTAX      MocaChannelMask
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the list of taboo channels for this MoCA node as
        reported in the TABOO_MASK_START and TABOO_CHANNEL_MASK fields
        in this node's Admission Request frame."
    REFERENCE   "MoCA MAC/PHY Specification 1.0, section 3.6.1.1."
    ::= { mocaIfStatusEntry 15 }
*/ 
unsigned int mocaIfStatusEntryBridge::Get_mocaIfNodeTabooChannelMask ()
{
  return lMocaIfNodeTabooChannelMask;
}


/* 
mocaIfCapabilityMask OBJECT-TYPE
    SYNTAX      MocaChannelMask
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the list of RF frequency channels that this device can
        support represented as a bitmask."
    ::= { mocaIfStatusEntry 16 }
*/ 
unsigned int mocaIfStatusEntryBridge::Get_mocaIfCapabilityMask ()
{
  return lMocaIfCapabilityMask;
}


/* 
mocaIfTxGcdPowerReduction OBJECT-TYPE
    SYNTAX      MocaDB
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "The Transmit Power Control back-off used for broadcast
        transmissions from this node.  'mocaIfTxGcdPowerReduction' is
        identical to the TPC back-off utilized for transmission and
        determined from the TPC backoff parameters TPC_BACKOFF_MAJOR and
        TPC_BACKOFF_MINOR as follows:

        mocaIfTxGcdPowerReduction = TPC_BACKOFF_MAJOR * 3 +
                                    TPC_BACKOFF_MINOR"
    ::= { mocaIfStatusEntry 17 }
*/ 
unsigned int mocaIfStatusEntryBridge::Get_mocaIfTxGcdPowerReduction ()
{
  return lMocaIfTxGcdPowerReduction;
}


/* 
mocaIfQAM256Capable OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate this MoCA node supports QAM256 if the value is 'true'.
        Indicate this MoCA node does not support QAM256 if this value is
        'false'."
    ::= { mocaIfStatusEntry 18 }
*/ 
bool mocaIfStatusEntryBridge::Get_mocaIfQAM256Capable ()
{
  return lMocaIfQAM256Capable;
}


/* 
mocaIfPacketsAggrCapability OBJECT-TYPE
    SYNTAX      MocaPacketAggrLevel
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Represent the maximum number of Ethernet packets aggregated in 
        a MoCA frame that this MoCA interface supports."
    ::= { mocaIfStatusEntry 19 }
*/ 
int mocaIfStatusEntryBridge::Get_mocaIfPacketsAggrCapability ()
{
  return lMocaIfPacketsAggrCapability;
}


/* 
mocaIfMaxIngressNodeBw OBJECT-TYPE
    SYNTAX      MocaPhyRate
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the maximum bandwidth of this MoCA interface for
        admission of flows if this node is an ingress node.  This value
        may be obtained from REM_NODE_CAPACITY field in the Response
        L2ME Frame.  For example, when the node is trying to create
        extremely large Flow, resulting in a failed flow creation."
    REFERENCE   "MoCA MAC/PHY Specification 1.1, section 4.2.2.2."
    ::= { mocaIfStatusEntry 20 }
*/ 
unsigned int mocaIfStatusEntryBridge::Get_mocaIfMaxIngressNodeBw ()
{
  return lMocaIfMaxIngressNodeBw;
}


/* 
mocaIfMaxEgressNodeBw OBJECT-TYPE
    SYNTAX      MocaPhyRate
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the maximum bandwidth of this MoCA interface for
        admission of Flows if this node is an egress node.  This value
        may be obtained from REM_NODE_CAPACITY field in the Response
        L2ME Frame.  For example, when the node is trying to create
        extremely large Flow, resulting in a failed flow creation."
    REFERENCE   "MoCA MAC/PHY Specification 1.1, section 4.2.2.2."
    ::= { mocaIfStatusEntry 21 }
*/ 
unsigned int mocaIfStatusEntryBridge::Get_mocaIfMaxEgressNodeBw ()
{
  return lMocaIfMaxEgressNodeBw;
}


/* 
mocaIfTxGcdRate OBJECT-TYPE
    SYNTAX      MocaPhyRate
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the PHY rate in Mbps for the transmit traffic
        broadcast from this node."
    ::= { mocaIfStatusEntry 22 }
*/ 
unsigned int mocaIfStatusEntryBridge::Get_mocaIfTxGcdRate ()
{
  return lMocaIfTxGcdRate;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  mocaIfStatsEntryBridge for bridging elements in the mocaIfStatsTable
 *
\*/

/* 
mocaIfStatsTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF MocaIfStatsEntry
    MAX-ACCESS  not-accessible
    STATUS      current
    DESCRIPTION
        "MoCA interface statistics table.  This table provides
        statistics of the MoCA interface.

        This table is indexed by 'ifIndex'.

        Entries in this table cannot be created or deleted by the
        network management system. All entries are created or deleted by
        the device software."
    ::= { mocaObjects 4 }

INDEX { ifIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

mocaIfStatsEntryBridge::mocaIfStatsEntryBridge (int ifIndex, void *mocaHandle,
                                                BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_mocaIfStatsTable,
                      BcmObjectId(ifIndex),
                      pAgent)
{
  struct moca_gen_stats MocaGenStats;
  struct moca_gen_status MocaGenStatus;

  bzero(&MocaGenStats, sizeof(MocaGenStats));
  bzero(&MocaGenStatus, sizeof(MocaGenStatus));

  // Clear our values
  lMocaIfTxPackets = lMocaIfTxDrops = lMocaIfRxPackets = 
  lMocaIfRxCorrectedErrors = lMocaIfRxDrops = 
  lMocaIfEgressNodeNumFlows = lMocaIfIngressNodeNumFlows = 0;

  // mocaHandle is only valid within the constructor, so don't save it!
  if (!moca_get_gen_stats(mocaHandle, &MocaGenStats)) {
    lMocaIfTxPackets = MocaGenStats.in_uc_packets +
                       MocaGenStats.in_mc_packets +
                       MocaGenStats.in_bc_packets +
                       MocaGenStats.in_discard_packets_ecl +
                       MocaGenStats.in_discard_packets_mac;
    lMocaIfTxDrops = MocaGenStats.in_discard_packets_ecl +
                     MocaGenStats.in_discard_packets_mac;
    lMocaIfRxPackets = MocaGenStats.out_uc_packets +
                       MocaGenStats.out_bc_packets;
    lMocaIfRxDrops = MocaGenStats.out_discard_packets;
  }
  if (!moca_get_gen_status(mocaHandle, &MocaGenStatus)) {
    lMocaIfEgressNodeNumFlows = MocaGenStatus.pqos_egress_numflows;
  }
}

mocaIfStatsEntryBridge::~mocaIfStatsEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
mocaIfTxPackets OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the number of Ethernet packets transmitted by this
        MoCA interface."
    ::= { mocaIfStatsEntry 1 }
*/ 
unsigned int mocaIfStatsEntryBridge::Get_mocaIfTxPackets ()
{
  return lMocaIfTxPackets;
}


/* 
mocaIfTxDrops OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the number of transmitted Ethernet packets dropped by
        this MoCA interface."
    ::= { mocaIfStatsEntry 2 }
*/ 
unsigned int mocaIfStatsEntryBridge::Get_mocaIfTxDrops ()
{
  return lMocaIfTxDrops;
}


/* 
mocaIfRxPackets OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the number of good Ethernet packets received by this
        MoCA interface."
    ::= { mocaIfStatsEntry 3 }
*/ 
unsigned int mocaIfStatsEntryBridge::Get_mocaIfRxPackets ()
{
  return lMocaIfRxPackets;
}


/* 
mocaIfRxCorrectedErrors OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the number of received Ethernet packets that have
        errors and are corrected by this MoCA interface."
    ::= { mocaIfStatsEntry 4 }
*/ 
unsigned int mocaIfStatsEntryBridge::Get_mocaIfRxCorrectedErrors ()
{
  return lMocaIfRxCorrectedErrors;
}


/* 
mocaIfRxDrops OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the number of scheduled MoCA frames that are not
        detected or have uncorrectable errors and are dropped by this
        MoCA interface."
    ::= { mocaIfStatsEntry 5 }
*/ 
unsigned int mocaIfStatsEntryBridge::Get_mocaIfRxDrops ()
{
  return lMocaIfRxDrops;
}


/* 
mocaIfEgressNodeNumFlows OBJECT-TYPE
    SYNTAX      Unsigned32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the number of PQoS flows in which this MoCA interface
        is an egress node for these PQoS flows."
    ::= { mocaIfStatsEntry 6 }
*/ 
unsigned int mocaIfStatsEntryBridge::Get_mocaIfEgressNodeNumFlows ()
{
  return lMocaIfEgressNodeNumFlows;
}


/* 
mocaIfIngressNodeNumFlows OBJECT-TYPE
    SYNTAX      Unsigned32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the number of PQoS flows in which this MoCA interface
        is an ingress node for these PQoS flows."
    ::= { mocaIfStatsEntry 7 }
*/ 
unsigned int mocaIfStatsEntryBridge::Get_mocaIfIngressNodeNumFlows ()
{
  return lMocaIfIngressNodeNumFlows;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  mocaIfFlowStatsEntryBridge for bridging elements in the mocaIfFlowStatsTable
 *
\*/

/* 
mocaIfFlowStatsTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF MocaIfFlowStatsEntry
    MAX-ACCESS  not-accessible
    STATUS      current
    DESCRIPTION
        "MoCA interface flow statistics table.  This table provides
        statistics of ingress PQoS flow in the MoCA interface.

        This table is indexed by 'ifIndex' and 'mocaIfFlowIndex'.

        Entries in this table cannot be created or deleted by the
        network management system. All entries are created or deleted by
        the device software."
    ::= { mocaObjects 5 }

INDEX { ifIndex, mocaIfFlowIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

mocaIfFlowStatsEntryBridge::mocaIfFlowStatsEntryBridge (int ifIndex, int mocaIfFlowIndex, 
                                                        void *mocaHandle, 
                                                        struct moca_pqos_query_response *PqosRsp,
                                                        BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_mocaIfFlowStatsTable,
                      BcmObjectId(ifIndex) + BcmObjectId(mocaIfFlowIndex),
                      pAgent)
{
  struct moca_gen_stats MocaGenStats;
  uint8_t macAddr[6];

  bzero(&MocaGenStats, sizeof(MocaGenStats));
  bzero(&macAddr, sizeof(macAddr));

  lIfIndex = ifIndex;
  lMocaIfFlowIndex = mocaIfFlowIndex;
  lMocaIfFlowID = PqosRsp->flowid;
  lMocaIfPacketDA = PqosRsp->packetda;
  lMocaIfPeakDataRate = PqosRsp->tpeakdatarate; 
  lMocaIfBurstSize =  PqosRsp->tburstsize; 
  lMocaIfLeaseTime = PqosRsp->tleasetime; 
  lMocaIfFlowTag = PqosRsp->flowtag; 
  lMocaIfLeaseTimeLeft = lMocaIfTxPacketsOnePacketAggr = 
  lMocaIfTxPacketsTwoPacketsAggr = lMocaIfTxPacketsThreePacketsAggr = 
  lMocaIfTxPacketsFourPacketsAggr = lMocaIfTxPacketsFivePacketsAggr = 
  lMocaIfTxPacketsSixPacketsAggr = lMocaIfTxPacketsSevenPacketsAggr = 
  lMocaIfTxPacketsEightPacketsAggr = lMocaIfTxPacketsNinePacketsAggr = 
  lMocaIfTxPacketsTenPacketsAggr = lMocaIfTxPacketsFlow = 0;

  // mocaHandle is only valid within the constructor, so don't save it!
  if (!moca_get_gen_stats(mocaHandle, &MocaGenStats)) {
    lMocaIfTxPacketsOnePacketAggr = MocaGenStats.aggr_pkt_stats_tx[0];
    lMocaIfTxPacketsTwoPacketsAggr = MocaGenStats.aggr_pkt_stats_tx[1];
    lMocaIfTxPacketsThreePacketsAggr = MocaGenStats.aggr_pkt_stats_tx[2];
    lMocaIfTxPacketsFourPacketsAggr = MocaGenStats.aggr_pkt_stats_tx[3];
    lMocaIfTxPacketsFivePacketsAggr = MocaGenStats.aggr_pkt_stats_tx[4];
    lMocaIfTxPacketsSixPacketsAggr = MocaGenStats.aggr_pkt_stats_tx[5];
    lMocaIfTxPacketsSevenPacketsAggr = MocaGenStats.aggr_pkt_stats_tx[6];
    lMocaIfTxPacketsEightPacketsAggr = MocaGenStats.aggr_pkt_stats_tx[7];
    lMocaIfTxPacketsNinePacketsAggr = MocaGenStats.aggr_pkt_stats_tx[8];
    lMocaIfTxPacketsTenPacketsAggr = MocaGenStats.aggr_pkt_stats_tx[9];
  }

}

mocaIfFlowStatsEntryBridge::~mocaIfFlowStatsEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
mocaIfFlowIndex OBJECT-TYPE
    SYNTAX      Integer32 (1..2147483647)
    MAX-ACCESS  not-accessible
    STATUS      current
    DESCRIPTION
        "This index is used to order the MoCA interface flow statistics
        table."
    ::= { mocaIfFlowStatsEntry 1 }
*/ 
int mocaIfFlowStatsEntryBridge::Get_mocaIfFlowIndex ()
{
  return lMocaIfFlowIndex;
}


/* 
mocaIfFlowID OBJECT-TYPE
    SYNTAX      MacAddress
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the flow ID of a PQoS flow. The flow ID is a valid
        multicast Ethernet MAC address that uniquely identifies a PQoS
        flow in a MoCA Network.

        The recommended rules for generation of the flow ID are
        specified in MoCA Spec 1.1."
    REFERENCE   "MoCA MAC/PHY Specification 1.1, appendix D."
    ::= { mocaIfFlowStatsEntry 2 }
*/ 
void mocaIfFlowStatsEntryBridge::Get_mocaIfFlowID (BcmMacAddress &mocaIfFlowID)
{
  mocaIfFlowID = lMocaIfFlowID;
}


/* 
mocaIfPacketDA OBJECT-TYPE
    SYNTAX      MacAddress
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the Destination Address (DA) of Ethernet packets of
        the PQoS Flow for which this node is the ingress node as
        specified in Section 4.2.1 of MoCA specification v1.1
        extensions."
    REFERENCE   "MoCA MAC/PHY Specification 1.1, section 4.2.1."
    ::= { mocaIfFlowStatsEntry 3 }
*/ 
void mocaIfFlowStatsEntryBridge::Get_mocaIfPacketDA (BcmMacAddress &mocaIfPacketDA)
{
  mocaIfPacketDA = lMocaIfPacketDA;
}


/* 
mocaIfPeakDataRate OBJECT-TYPE
    SYNTAX      Unsigned32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the T_PEAK_DATA_RATE of the PQoS flow identified by
        'mocaIfFlowID' in which this MoCA interface is an ingress node
        for the PQoS flow."
    REFERENCE   "MoCA MAC/PHY Specification 1.1, section 4.2.1."
    ::= { mocaIfFlowStatsEntry 4 }
*/ 
unsigned int mocaIfFlowStatsEntryBridge::Get_mocaIfPeakDataRate ()
{
  return lMocaIfPeakDataRate;
}


/* 
mocaIfBurstSize OBJECT-TYPE
    SYNTAX      Unsigned32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the T_BURST_SIZE of the PQoS flow identified by
        'mocaIfFlowID' in which this MoCA interface is an ingress node
        for this PQoS flow."
    REFERENCE   "MoCA MAC/PHY Specification 1.1, section 4.2.1."
    ::= { mocaIfFlowStatsEntry 5 }
*/ 
unsigned int mocaIfFlowStatsEntryBridge::Get_mocaIfBurstSize ()
{
  return lMocaIfBurstSize;
}


/* 
mocaIfLeaseTime OBJECT-TYPE
    SYNTAX      Unsigned32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the T_LEASE_TIME of the PQoS flow identified by
        'mocaIfFlowID' in which this MoCA interface is an ingress node
        for this PQoS flow."
    REFERENCE   "MoCA MAC/PHY Specification 1.1, section 4.2.1."
    ::= { mocaIfFlowStatsEntry 6 }
*/ 
unsigned int mocaIfFlowStatsEntryBridge::Get_mocaIfLeaseTime ()
{
  return lMocaIfLeaseTime;
}


/* 
mocaIfFlowTag OBJECT-TYPE
    SYNTAX      Unsigned32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the FLOW_TAG of the PQoS flow identified by
        'mocaIfFlowID' in which this MoCA interface is an ingress node
        for this PQoS flow. The FLOW_TAG carries application specific
        content of this PQoS flow."
    REFERENCE   "MoCA MAC/PHY Specification 1.1, section 4.2.1."
    ::= { mocaIfFlowStatsEntry 7 }
*/ 
unsigned int mocaIfFlowStatsEntryBridge::Get_mocaIfFlowTag ()
{
  return lMocaIfFlowTag;
}


/* 
mocaIfLeaseTimeLeft OBJECT-TYPE
    SYNTAX      Unsigned32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the LEASE_TIME_LEFT of the PQoS flow identified by
        'mocaIfFlowID' in which this MoCA interface is an ingress node
        for this PQoS flow."
    REFERENCE   "MoCA MAC/PHY Specification 1.1, section 4.2.1."
    ::= { mocaIfFlowStatsEntry 8 }
*/ 
unsigned int mocaIfFlowStatsEntryBridge::Get_mocaIfLeaseTimeLeft ()
{
  return lMocaIfLeaseTimeLeft;
}


/* 
mocaIfTxPacketsOnePacketAggr OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the number of MoCA frames with no aggregation level
        transmitted by this MoCA interface."
    ::= { mocaIfFlowStatsEntry 9 }
*/ 
unsigned int mocaIfFlowStatsEntryBridge::Get_mocaIfTxPacketsOnePacketAggr ()
{
  return lMocaIfTxPacketsOnePacketAggr;
}


/* 
mocaIfTxPacketsTwoPacketsAggr OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the number of MoCA frames with two-packets
        aggregation level transmitted by this MoCA interface."
    ::= { mocaIfFlowStatsEntry 10 }
*/ 
unsigned int mocaIfFlowStatsEntryBridge::Get_mocaIfTxPacketsTwoPacketsAggr ()
{
  return lMocaIfTxPacketsTwoPacketsAggr;
}


/* 
mocaIfTxPacketsThreePacketsAggr OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the number of MoCA frames with three-packets
        aggregation level transmitted by this MoCA interface."
    ::= { mocaIfFlowStatsEntry 11 }
*/ 
unsigned int mocaIfFlowStatsEntryBridge::Get_mocaIfTxPacketsThreePacketsAggr ()
{
  return lMocaIfTxPacketsThreePacketsAggr;
}


/* 
mocaIfTxPacketsFourPacketsAggr OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the number of MoCA frames with four-packets
        aggregation level transmitted by this MoCA interface."
    ::= { mocaIfFlowStatsEntry 12 }
*/ 
unsigned int mocaIfFlowStatsEntryBridge::Get_mocaIfTxPacketsFourPacketsAggr ()
{
  return lMocaIfTxPacketsFourPacketsAggr;
}


/* 
mocaIfTxPacketsFivePacketsAggr OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the number of MoCA frames with five-packets
        aggregation level transmitted by this MoCA interface."
    ::= { mocaIfFlowStatsEntry 13 }
*/ 
unsigned int mocaIfFlowStatsEntryBridge::Get_mocaIfTxPacketsFivePacketsAggr ()
{
  return lMocaIfTxPacketsFivePacketsAggr;
}


/* 
mocaIfTxPacketsSixPacketsAggr OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the number of MoCA frames with six-packets
        aggregation level transmitted by this MoCA interface."
    ::= { mocaIfFlowStatsEntry 14 }
*/ 
unsigned int mocaIfFlowStatsEntryBridge::Get_mocaIfTxPacketsSixPacketsAggr ()
{
  return lMocaIfTxPacketsSixPacketsAggr;
}


/* 
mocaIfTxPacketsSevenPacketsAggr OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the number of MoCA frames with seven-packets
        aggregation level transmitted by this MoCA interface."
    ::= { mocaIfFlowStatsEntry 15 }
*/ 
unsigned int mocaIfFlowStatsEntryBridge::Get_mocaIfTxPacketsSevenPacketsAggr ()
{
  return lMocaIfTxPacketsSevenPacketsAggr;
}


/* 
mocaIfTxPacketsEightPacketsAggr OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the number of MoCA frames with eight-packets
        aggregation level transmitted by this MoCA interface."
    ::= { mocaIfFlowStatsEntry 16 }
*/ 
unsigned int mocaIfFlowStatsEntryBridge::Get_mocaIfTxPacketsEightPacketsAggr ()
{
  return lMocaIfTxPacketsEightPacketsAggr;
}


/* 
mocaIfTxPacketsNinePacketsAggr OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the number of MoCA frames with nine-packets
        aggregation level transmitted by this MoCA interface."
    ::= { mocaIfFlowStatsEntry 17 }
*/ 
unsigned int mocaIfFlowStatsEntryBridge::Get_mocaIfTxPacketsNinePacketsAggr ()
{
  return lMocaIfTxPacketsNinePacketsAggr;
}


/* 
mocaIfTxPacketsTenPacketsAggr OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the number of MoCA frames with ten-packets
        aggregation level transmitted by this MoCA interface."
    ::= { mocaIfFlowStatsEntry 18 }
*/ 
unsigned int mocaIfFlowStatsEntryBridge::Get_mocaIfTxPacketsTenPacketsAggr ()
{
  return lMocaIfTxPacketsTenPacketsAggr;
}


/* 
mocaIfTxPacketsFlow OBJECT-TYPE
    SYNTAX      Unsigned32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the total number of Ethernet packets transmitted by
        this MoCA interface on the PQoS flow indentified by
        'mocaIfFlowID'."
    ::= { mocaIfFlowStatsEntry 19 }
*/ 
unsigned int mocaIfFlowStatsEntryBridge::Get_mocaIfTxPacketsFlow ()
{
  return lMocaIfTxPacketsFlow;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  mocaNodeEntryBridge for bridging elements in the mocaNodeTable
 *
\*/

/* 
mocaNodeTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF MocaNodeEntry
    MAX-ACCESS  not-accessible
    STATUS      current
    DESCRIPTION
        "MoCA node status information table.  This table contains an
        entry for every node in the MoCA network, excluding this MoCA
        interface.

        This table is indexed by 'ifIndex' and 'mocaNodeIndex'.

        Entries in this table cannot be created or deleted by the
        network management system. All entries are created or deleted by
        the device software."
    ::= { mocaObjects 6 }

INDEX { ifIndex, mocaNodeIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

mocaNodeEntryBridge::mocaNodeEntryBridge (int ifIndex, unsigned long mocaNodeIndex, 
                                                void *mocaHandle, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_mocaNodeTable,
                      BcmObjectId(ifIndex) + BcmObjectId(mocaNodeIndex),
                      pAgent)
{
  struct moca_gen_status MocaGenStatus;
  struct moca_gen_node_status MocaGenNodeStatus;
  struct moca_drv_info MocaDrvInfo;
  struct moca_init_time MocaInitTime;
  struct moca_tx_profile MocaTxProfile;
  MoCA_NODE_STATUS_ENTRY MocaNodeStatusEntry;
  MoCA_NODE_STATISTICS_ENTRY MocaNodeStatsEntry;
  char swVers[81] = "";
  uint8_t macAddr[6];
  uint32_t versMajor, versMinor, versBuild ;


  bzero(&MocaGenStatus, sizeof(MocaGenStatus));
  bzero(&MocaGenNodeStatus, sizeof(MocaGenNodeStatus));
  bzero(&MocaDrvInfo, sizeof(MocaDrvInfo));
  bzero(&MocaInitTime, sizeof(MocaInitTime));
  bzero(&MocaTxProfile, sizeof(MocaTxProfile));
  bzero(&MocaNodeStatusEntry, sizeof(MocaNodeStatusEntry));
  bzero(&MocaNodeStatsEntry, sizeof(MocaNodeStatsEntry));

  // Set our fixed values
  lMocaNodeIndex = mocaNodeIndex;
  lMocaNodePacketsAggrCapability = 10;
  // These are all optional, so we always return 0
  lMocaNodeRxGcdPower = lMocaNodeTxPowerReduction
  = lMocaNodeRxPower = lMocaNodeRxPackets
  = lMocaNodeRxCorrected = lMocaNodeRxDrops
  = lMocaNodeSNR = 0;

  // Default all other values before getting the real values
  lMocaNodeMocaVersion = 0;
  lMocaNodeMacAddress = kAllZerosMac;
  lMocaNodeTxGcdRate = 0;
  lMocaNodePreferredNC = false;
  lMocaNodeQAM256Capable = false;

  if (!(moca_get_gen_node_status(mocaHandle, mocaNodeIndex, &MocaGenNodeStatus))) {
    moca_u32_to_mac(macAddr, MocaGenNodeStatus.eui_hi, 
                    MocaGenNodeStatus.eui_lo);
    lMocaNodeMacAddress = macAddr;
  }
  if (!(moca_get_gen_status(mocaHandle, &MocaGenStatus))) {
    lMocaNodeMocaVersion = MocaGenStatus.network_moca_version;
    lMocaNodeQAM256Capable = (MocaGenStatus.qam_256_support ? true : false);

  }
  if (!moca_get_init_time(mocaHandle, &MocaInitTime)) {
    lMocaNodePreferredNC = (MocaInitTime.preferred_nc ? true : false);
  }
  if (!moca_get_tx_profile(mocaHandle, mocaNodeIndex, &MocaTxProfile)) {
    lMocaNodeTxGcdRate = moca_phy_rate(MocaTxProfile.nbas, MocaTxProfile.cp, MocaTxProfile.turbo_status);
  }
  MocaNodeStatusEntry.nodeId = mocaNodeIndex;

  MoCACtl2_GetNodeStatus(mocaHandle, &MocaNodeStatusEntry);

    lMocaNodeSNR = MocaNodeStatusEntry.rxUc.avgSnr/2.0;

    lMocaNodeRxGcdPower = MocaNodeStatusEntry.rxBc.rxGain/4.0;
    lMocaNodeTxPowerReduction = MocaNodeStatusEntry.txUc.txPower;
    lMocaNodeRxPower = MocaNodeStatusEntry.rxUc.rxGain/4.0;

  MocaNodeStatsEntry.nodeId = mocaNodeIndex;
  MoCACtl2_GetNodeStatistics(mocaHandle, &MocaNodeStatsEntry, 0);

    lMocaNodeRxPackets = MocaNodeStatsEntry.rxPkts;
    lMocaNodeRxCorrected = MocaNodeStatsEntry.rxCwCorrected;  
    lMocaNodeRxDrops = MocaNodeStatsEntry.rxCwUncorrected;
  
}

mocaNodeEntryBridge::~mocaNodeEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
mocaNodeIndex OBJECT-TYPE
    SYNTAX      MocaNodeID
    MAX-ACCESS  not-accessible
    STATUS      current
    DESCRIPTION
        "The node ID is used as an index to order the MoCA node status
        information table."
    ::= { mocaNodeEntry 1 }
*/ 
unsigned int mocaNodeEntryBridge::Get_mocaNodeIndex ()
{
  return lMocaNodeIndex;
}


/* 
mocaNodeMocaVersion OBJECT-TYPE
    SYNTAX      MocaVersion
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the MoCA version supported by the MoCA node identified
        by 'mocaNodeIndex'."
    ::= { mocaNodeEntry 2 }
*/ 
int mocaNodeEntryBridge::Get_mocaNodeMocaVersion ()
{
  return lMocaNodeMocaVersion;
}


/* 
mocaNodeMacAddress OBJECT-TYPE
    SYNTAX      MacAddress
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the MAC address of the MoCA node identified by
        'mocaNodeIndex'. This MAC address is encoded in the first six
        bytes of the Globally Unique Identifier (GUID).

        For example, MoCA node with MAC address aa:bb:cc:dd:ee:ff will
        have a GUID of aa:bb:cc:dd:ee:ff:00:00."
    ::= { mocaNodeEntry 3 }
*/ 
void mocaNodeEntryBridge::Get_mocaNodeMacAddress (BcmMacAddress &mocaNodeMacAddress)
{
  mocaNodeMacAddress = lMocaNodeMacAddress;
}


/* 
mocaNodeTxGcdRate OBJECT-TYPE
    SYNTAX      MocaPhyRate
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the PHY rate in Mbps for the transmit traffic
        broadcasted from this MoCA interface.

        This value is the broadcast PHY rate from this MoCA interface to
        the node identified by 'mocaNodeIndex'."
    ::= { mocaNodeEntry 4 }
*/ 
unsigned int mocaNodeEntryBridge::Get_mocaNodeTxGcdRate ()
{
  return lMocaNodeTxGcdRate;
}


/* 
mocaNodeRxGcdPower OBJECT-TYPE
    SYNTAX      MocaDBm
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "The power measurement should cover the preamble and/or at
        least a symbol time.  The averaging method employed for this
        measurement is implementation specific and not specified.  It
        is recommended but not required to utilize the LMO power
        control probes for this measurement."
    ::= { mocaNodeEntry 5 }
*/ 
int mocaNodeEntryBridge::Get_mocaNodeRxGcdPower ()
{
  return lMocaNodeRxGcdPower;
}


/* 
mocaNodeTxPowerReduction OBJECT-TYPE
    SYNTAX      MocaDB
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "The Transmit Power Control back-off in dB used for unicast
        transmissions from the managed node to a node identified by
        'mocaNodeIndex'.  'mocaNodeTxPowerReduction' is identical to the
        TPC back-off utilized for transmission and determined from the
        TPC backoff parameters TPC_BACKOFF_MAJOR and TPC_BACKOFF_MINOR
        as follows:

        mocaNodeTxPowerReduction = TPC_BACKOFF_MAJOR * 3 +
                                   TPC_BACKOFF_MINOR"
    ::= { mocaNodeEntry 6 }
*/ 
unsigned int mocaNodeEntryBridge::Get_mocaNodeTxPowerReduction ()
{
  return lMocaNodeTxPowerReduction;
}


/* 
mocaNodeRxPower OBJECT-TYPE
    SYNTAX      MocaDBm
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "The power measurement should cover the preamble and/or at
        least a symbol time.  The averaging method employed for this
        measurement is implementation specific and not specified.  It
        is recommended but not required to utilize the LMO power
        control probes for this measurement."
    ::= { mocaNodeEntry 7 }
*/ 
int mocaNodeEntryBridge::Get_mocaNodeRxPower ()
{
  return lMocaNodeRxPower;
}


/* 
mocaNodePreferredNC OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the MoCA node identified by 'mocaNodeIndex' is set as
        preferred NC if this value is 'true'.  Indicate the MoCA node
        identified by 'mocaNodeIndex' is not set as preferred NC if this
        value is 'false'."
    ::= { mocaNodeEntry 8 }
*/ 
bool mocaNodeEntryBridge::Get_mocaNodePreferredNC ()
{
  return lMocaNodePreferredNC;
}


/* 
mocaNodeQAM256Capable OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the MoCA node identified by 'mocaNodeIndex' supports
        QAM256 if the value is 'true'.  Indicate the MoCA node
        identified by 'mocaNodeIndex' does not support QAM256 if this
        value is 'false'."
    ::= { mocaNodeEntry 9 }
*/ 
bool mocaNodeEntryBridge::Get_mocaNodeQAM256Capable ()
{
  return lMocaNodeQAM256Capable;
}


/* 
mocaNodePacketsAggrCapability OBJECT-TYPE
    SYNTAX      MocaPacketAggrLevel
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Represent the maximum number of Ethernet packets aggregated in
        a MoCA frame that this MoCA node supports."
    ::= { mocaNodeEntry 10 }
*/ 
int mocaNodeEntryBridge::Get_mocaNodePacketsAggrCapability ()
{
  return lMocaNodePacketsAggrCapability;
}


/* 
mocaNodeRxPackets   OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the number of good Ethernet packets received by this
        MoCA interface from the MoCA node identified by
        'mocaNodeIndex'."
    ::= { mocaNodeEntry 11 }
*/ 
unsigned int mocaNodeEntryBridge::Get_mocaNodeRxPackets ()
{
  return lMocaNodeRxPackets;
}


/* 
mocaNodeRxCorrected OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the number of Ethernet packets received by this MoCA
        interface that have errors and are corrected from the MoCA node
        identified by 'mocaNodeIndex'."
    ::= { mocaNodeEntry 12 }
*/ 
unsigned int mocaNodeEntryBridge::Get_mocaNodeRxCorrected ()
{
  return lMocaNodeRxCorrected;
}


/* 
mocaNodeRxDrops OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the number of scheduled MoCA frames that are not
        detected or have uncorrectable errors and are dropped by this
        MoCA interface from the MoCA node identified by
        'mocaNodeIndex'."
    ::= { mocaNodeEntry 13 }
*/ 
unsigned int mocaNodeEntryBridge::Get_mocaNodeRxDrops ()
{
  return lMocaNodeRxDrops;
}


/* 
mocaNodeSNR OBJECT-TYPE
    SYNTAX      MocaDB
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "A dB measure of the Signal to Noise Ratio (SNR) based on the
        Type 1 (EVM) probe from a node identified by 'mocaNodeIndex'."
    REFERENCE   "MoCA MAC/PHY Specification 1.0, section 4.5.1."
    ::= { mocaNodeEntry 14 }
*/ 
unsigned int mocaNodeEntryBridge::Get_mocaNodeSNR ()
{
  return lMocaNodeSNR;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  mocaMeshEntryBridge for bridging elements in the mocaMeshTable
 *
\*/

/* 
mocaMeshTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF MocaMeshEntry
    MAX-ACCESS  not-accessible
    STATUS      current
    DESCRIPTION
        "MoCA mesh PHY rate table.  This table contains the unicast
        transmit PHY rate between all pair of nodes in the MoCA Network.

        This table is indexed by 'ifIndex', 'mocaMeshTxNodeIndex', and
        'mocaMeshRxNodeIndex'.

        Since 'mocaMeshTxRate' is the transmit PHY rate from
        'mocaMeshTxNodeIndex' to 'mocaMeshRxNodeIndex', this table does
        not contain any entries with 'mocaMeshTxNodeIndex' equals
        'mocaMeshRxNodeIndex'.  In addition, a MoCA network can have
        less than 16 nodes, hence some values of 'mocaMeshTxNodeIndex'
        and 'mocaMeshRxNodeIndex' in the range of 0 to 15 may not exist.

        For example, in a three nodes MoCA 1.1 network with node ID 2, 4
        and 6, the 'mocaMeshTable' for MoCA interface with 'ifIndex' 1
        contains only six entries,
            mocaMeshTxRate.1.2.4 in which
                'ifIndex' = 1
                'mocaMeshTxNodeIndex' = 2
                'mocaMeshRxNodeIndex' = 4
            mocaMeshTxRate.1.2.6
            mocaMeshTxRate.1.4.2
            mocaMeshTxRate.1.4.6
            mocaMeshTxRate.1.6.2
            mocaMeshTxRate.1.6.4

        In a three nodes MoCA 1.0 network with node ID 2, 4 and 6, the
        'mocaMeshTable' for MoCA interface with 'ifIndex' 1 and node ID
        2 contains only four entries,
            mocaMeshTxRate.1.2.4
            mocaMeshTxRate.1.2.6
            mocaMeshTxRate.1.4.2
            mocaMeshTxRate.1.6.2

        Entries in this table cannot be created or deleted by the
        network management system. All entries are created or deleted by
        the device software."
    ::= { mocaObjects 7 }

INDEX { ifIndex, mocaMeshTxNodeIndex, mocaMeshRxNodeIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

mocaMeshEntryBridge::mocaMeshEntryBridge (int ifIndex, unsigned long mocaMeshTxNodeIndex, 
                                          unsigned long mocaMeshRxNodeIndex, void *mocaHandle,
                                          BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_mocaMeshTable,
                      BcmObjectId(ifIndex) + BcmObjectId(mocaMeshTxNodeIndex) + BcmObjectId(mocaMeshRxNodeIndex),
                      pAgent)
{
  struct moca_tx_profile MocaTxProfile;
  
  bzero(&MocaTxProfile, sizeof(MocaTxProfile));


  lMocaMeshTxNodeIndex = mocaMeshTxNodeIndex;
  lMocaMeshRxNodeIndex = mocaMeshRxNodeIndex;
  lMocaMeshTxRate = 0;
  if (!moca_get_tx_profile(mocaHandle, (uint32_t) mocaMeshTxNodeIndex, &MocaTxProfile)) {
    lMocaMeshTxRate = moca_phy_rate(MocaTxProfile.nbas, MocaTxProfile.cp, MocaTxProfile.turbo_status);
  }
}

mocaMeshEntryBridge::~mocaMeshEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
mocaMeshTxNodeIndex OBJECT-TYPE
    SYNTAX      MocaNodeID
    MAX-ACCESS  not-accessible
    STATUS      current
    DESCRIPTION
        "The node ID of the transmit MoCA node is used as one of the
        index to order the mesh PHY rate table."
    ::= { mocaMeshEntry 1 }
*/ 
unsigned int mocaMeshEntryBridge::Get_mocaMeshTxNodeIndex ()
{
  return lMocaMeshTxNodeIndex;
}


/* 
mocaMeshRxNodeIndex OBJECT-TYPE
    SYNTAX      MocaNodeID
    MAX-ACCESS  not-accessible
    STATUS      current
    DESCRIPTION
        "The node ID of the receive MoCA node is used as one of the
        index to order the mesh PHY rate table."
    ::= { mocaMeshEntry 2 }
*/ 
unsigned int mocaMeshEntryBridge::Get_mocaMeshRxNodeIndex ()
{
  return lMocaMeshRxNodeIndex;
}


/* 
mocaMeshTxRate OBJECT-TYPE
    SYNTAX      MocaPhyRate
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "Indicate the transmit PHY rate in Mbps from the MoCA node
        identified by 'mocaMeshTxNodeIndex' to the MoCA node identified
        by 'mocaMeshRxNodeIndex'."
    ::= { mocaMeshEntry 3 }
*/ 
unsigned int mocaMeshEntryBridge::Get_mocaMeshTxRate ()
{
  return lMocaMeshTxRate;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  mocaBridgeEntryBridge for bridging elements in the mocaBridgeTable
 *
\*/

/* 
mocaBridgeTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF MocaBridgeEntry
    MAX-ACCESS  not-accessible
    STATUS      current
    DESCRIPTION
        "MoCA bridge Ethernet MAC address table.  This table contains
        an entry for every node in the MoCA network.

        This table is indexed by 'ifIndex', 'mocaBridgeNodeIndex' and
        'mocaBridgeMacIndex'.

        Entries in this table cannot be created or deleted by the
        network management system.  All entries are created or deleted
        by the device software."
::= { mocaObjects 8 }

INDEX { ifIndex, mocaBridgeNodeIndex, mocaBridgeMacIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

mocaBridgeEntryBridge::mocaBridgeEntryBridge (int ifIndex, unsigned long mocaBridgeNodeIndex, int mocaBridgeMacIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_mocaBridgeTable,
                      BcmObjectId(ifIndex) + BcmObjectId(mocaBridgeNodeIndex) + BcmObjectId(mocaBridgeMacIndex),
                      pAgent)
{
/*  MoCA_UC_FWD_ENTRY ucFwdTbl [MoCA_MAX_UC_FWD_ENTRIES] ;
  unsigned int tblSize;
  unsigned int fwdEntries;

  if (!MoCACtl2_GetUcFwdTbl( g_mocaHandle, &ucFwdTbl [0], &tblSize )) {
    for (fwdEntries = 0; fwdEntries < tblSize; fwdEntries++) {
      return these two values:
       ucFwdTbl[fwdEntries].ucDestNodeId
       ucFwdTbl[fwdEntries].nodeMacAddr
    }
  }
*/
}

mocaBridgeEntryBridge::~mocaBridgeEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
mocaBridgeNodeIndex OBJECT-TYPE
    SYNTAX      MocaNodeID
    MAX-ACCESS  not-accessible
    STATUS      current
    DESCRIPTION
        "The node ID is used as an index to order the MAC entries 
        in the MoCA bridge table."
    ::= { mocaBridgeEntry 1 }
*/ 
unsigned int mocaBridgeEntryBridge::Get_mocaBridgeNodeIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_mocaBridgeNodeIndex" << endl;
  return 0;
}


/* 
mocaBridgeMacIndex  OBJECT-TYPE
    SYNTAX      Integer32 (1..2147483647)
    MAX-ACCESS  not-accessible
    STATUS      current
    DESCRIPTION
        "This index is used to order the MAC entries for each node in
        the MoCA bridge table."
    ::= { mocaBridgeEntry 2 }
*/ 
int mocaBridgeEntryBridge::Get_mocaBridgeMacIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_mocaBridgeMacIndex" << endl;
  return 0;
}


/* 
mocaBridgeMacAddress    OBJECT-TYPE
    SYNTAX      MacAddress
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION
        "The Ethernet MAC address identified by the node ID and MAC
        index."
    ::= { mocaBridgeEntry 3 }
*/ 
void mocaBridgeEntryBridge::Get_mocaBridgeMacAddress (BcmMacAddress &mocaBridgeMacAddress)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_mocaBridgeMacAddress" << endl;
  mocaBridgeMacAddress.Set (0,0,0,0,0,0);
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





