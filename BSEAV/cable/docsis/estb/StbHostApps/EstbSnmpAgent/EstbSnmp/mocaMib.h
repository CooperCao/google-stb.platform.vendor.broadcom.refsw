//****************************************************************************
//
// Copyright (c) 2009-2010 Broadcom Corporation
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
//    Filename: mocaMib.h
//    Creation Date: Friday, October 16, 2009 at 10:25:11
//    Created by Broadcom BFC Mib Compiler BCMIBC.EXE version 3.6.1b
//    from input file MOCA11-MIB.mib
//
//****************************************************************************

#ifndef MOCAMIB_H
#define MOCAMIB_H

#include "MibObjs.h"
#include "MibBridge.h"

class mocaIfConfigEntryBridge;
class mocaIfAccessEntryBridge;
class mocaIfStatusEntryBridge;
class mocaIfStatsEntryBridge;
class mocaIfFlowStatsEntryBridge;
class mocaNodeEntryBridge;
class mocaMeshEntryBridge;
class mocaBridgeEntryBridge;


/*\
 *  moca11Group group
\*/

// OID constants
#define kOID_moca11Group  "1.3.6.1.4.1.31621.1.1"
#define kOID_mocaNotificationsGroup  "1.3.6.1.4.1.31621.1.1.0"
#define kOID_mocaObjectsGroup  "1.3.6.1.4.1.31621.1.1.1"
#define kOID_mocaConformanceGroup  "1.3.6.1.4.1.31621.1.1.2"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_moca11Group  "moca11Group"
  #define kNAME_mocaNotificationsGroup  "mocaNotificationsGroup"
  #define kNAME_mocaObjectsGroup  "mocaObjectsGroup"
  #define kNAME_mocaConformanceGroup  "mocaConformanceGroup"
#else
  #define kNAME_moca11Group  kOID_moca11Group
  #define kNAME_mocaNotificationsGroup  kOID_mocaNotificationsGroup
  #define kNAME_mocaObjectsGroup  kOID_mocaObjectsGroup
  #define kNAME_mocaConformanceGroup  kOID_mocaConformanceGroup
#endif

class moca11Group : public BcmMibGroup
{
  public:
    moca11Group ();

    bool Populate ();
};



/*\
 *  mocaObjectsGroup group
\*/

// OID constants
#define kOID_mocaObjectsGroup  "1.3.6.1.4.1.31621.1.1.1"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_mocaObjectsGroup  "mocaObjectsGroup"
#else
  #define kNAME_mocaObjectsGroup  kOID_mocaObjectsGroup
#endif

class mocaObjectsGroup : public BcmMibGroup
{
  public:
    mocaObjectsGroup ();

    bool Populate ();
};


/* mocaIfConfigTable and related objects */

// OID constants
#define kOID_mocaIfConfigTable  "1.3.6.1.4.1.31621.1.1.1.1"
#define kOID_mocaIfConfigEntry  "1.3.6.1.4.1.31621.1.1.1.1.1"
#define kOID_mocaIfEnable  "1.3.6.1.4.1.31621.1.1.1.1.1.1"
#define kOID_mocaIfChannelMask  "1.3.6.1.4.1.31621.1.1.1.1.1.2"
#define kOID_mocaIfPowerControl  "1.3.6.1.4.1.31621.1.1.1.1.1.3"
#define kOID_mocaIfTxPowerLimit  "1.3.6.1.4.1.31621.1.1.1.1.1.4"
#define kOID_mocaIfBeaconPowerLimit  "1.3.6.1.4.1.31621.1.1.1.1.1.5"
#define kOID_mocaIfPowerControlTargetRate  "1.3.6.1.4.1.31621.1.1.1.1.1.6"
#define kOID_mocaIfPrivacyEnable  "1.3.6.1.4.1.31621.1.1.1.1.1.7"
#define kOID_mocaIfPassword  "1.3.6.1.4.1.31621.1.1.1.1.1.8"
#define kOID_mocaIfPreferredNC  "1.3.6.1.4.1.31621.1.1.1.1.1.9"
#define kOID_mocaIfAccessEnable  "1.3.6.1.4.1.31621.1.1.1.1.1.10"
#define kOID_mocaIfPhyThreshold  "1.3.6.1.4.1.31621.1.1.1.1.1.11"
#define kOID_mocaIfPhyThresholdEnable  "1.3.6.1.4.1.31621.1.1.1.1.1.12"
#define kOID_mocaIfStatusChangeEnable  "1.3.6.1.4.1.31621.1.1.1.1.1.13"
#define kOID_mocaIfNumNodesChangeEnable  "1.3.6.1.4.1.31621.1.1.1.1.1.14"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_mocaIfConfigTable  "mocaIfConfigTable"
  #define kNAME_mocaIfConfigEntry  "mocaIfConfigEntry"
  #define kNAME_mocaIfEnable  "mocaIfEnable"
  #define kNAME_mocaIfChannelMask  "mocaIfChannelMask"
  #define kNAME_mocaIfPowerControl  "mocaIfPowerControl"
  #define kNAME_mocaIfTxPowerLimit  "mocaIfTxPowerLimit"
  #define kNAME_mocaIfBeaconPowerLimit  "mocaIfBeaconPowerLimit"
  #define kNAME_mocaIfPowerControlTargetRate  "mocaIfPowerControlTargetRate"
  #define kNAME_mocaIfPrivacyEnable  "mocaIfPrivacyEnable"
  #define kNAME_mocaIfPassword  "mocaIfPassword"
  #define kNAME_mocaIfPreferredNC  "mocaIfPreferredNC"
  #define kNAME_mocaIfAccessEnable  "mocaIfAccessEnable"
  #define kNAME_mocaIfPhyThreshold  "mocaIfPhyThreshold"
  #define kNAME_mocaIfPhyThresholdEnable  "mocaIfPhyThresholdEnable"
  #define kNAME_mocaIfStatusChangeEnable  "mocaIfStatusChangeEnable"
  #define kNAME_mocaIfNumNodesChangeEnable  "mocaIfNumNodesChangeEnable"
#else
  #define kNAME_mocaIfConfigTable  kOID_mocaIfConfigTable
  #define kNAME_mocaIfConfigEntry  kOID_mocaIfConfigEntry
  #define kNAME_mocaIfEnable  kOID_mocaIfEnable
  #define kNAME_mocaIfChannelMask  kOID_mocaIfChannelMask
  #define kNAME_mocaIfPowerControl  kOID_mocaIfPowerControl
  #define kNAME_mocaIfTxPowerLimit  kOID_mocaIfTxPowerLimit
  #define kNAME_mocaIfBeaconPowerLimit  kOID_mocaIfBeaconPowerLimit
  #define kNAME_mocaIfPowerControlTargetRate  kOID_mocaIfPowerControlTargetRate
  #define kNAME_mocaIfPrivacyEnable  kOID_mocaIfPrivacyEnable
  #define kNAME_mocaIfPassword  kOID_mocaIfPassword
  #define kNAME_mocaIfPreferredNC  kOID_mocaIfPreferredNC
  #define kNAME_mocaIfAccessEnable  kOID_mocaIfAccessEnable
  #define kNAME_mocaIfPhyThreshold  kOID_mocaIfPhyThreshold
  #define kNAME_mocaIfPhyThresholdEnable  kOID_mocaIfPhyThresholdEnable
  #define kNAME_mocaIfStatusChangeEnable  kOID_mocaIfStatusChangeEnable
  #define kNAME_mocaIfNumNodesChangeEnable  kOID_mocaIfNumNodesChangeEnable
#endif

class mocaIfConfigTable : public BcmMibTable
{
  public:
    mocaIfConfigTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class mocaIfConfigEntry : public BcmMibEntry
{
  friend class mocaIfConfigEntryBridge;
  friend class mocaIfConfigTable;

  public:
    mocaIfConfigEntry (mocaIfConfigTable *pTable, mocaIfConfigEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    mocaIfConfigEntryBridge *fpBridge;

    enum
    {
      kLEAF_mocaIfEnable = 1,
      kLEAF_mocaIfChannelMask = 2,
      kLEAF_mocaIfPowerControl = 3,
      kLEAF_mocaIfTxPowerLimit = 4,
      kLEAF_mocaIfBeaconPowerLimit = 5,
      kLEAF_mocaIfPowerControlTargetRate = 6,
      kLEAF_mocaIfPrivacyEnable = 7,
      kLEAF_mocaIfPassword = 8,
      kLEAF_mocaIfPreferredNC = 9,
      kLEAF_mocaIfAccessEnable = 10,
      kLEAF_mocaIfPhyThreshold = 11,
      kLEAF_mocaIfPhyThresholdEnable = 12,
      kLEAF_mocaIfStatusChangeEnable = 13,
      kLEAF_mocaIfNumNodesChangeEnable = 14
    };
};


/* mocaIfAccessTable and related objects */

// OID constants
#define kOID_mocaIfAccessTable  "1.3.6.1.4.1.31621.1.1.1.2"
#define kOID_mocaIfAccessEntry  "1.3.6.1.4.1.31621.1.1.1.2.1"
#define kOID_mocaIfAccessIndex  "1.3.6.1.4.1.31621.1.1.1.2.1.1"
#define kOID_mocaIfAccessMacAddress  "1.3.6.1.4.1.31621.1.1.1.2.1.2"
#define kOID_mocaIfAccessStatus  "1.3.6.1.4.1.31621.1.1.1.2.1.3"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_mocaIfAccessTable  "mocaIfAccessTable"
  #define kNAME_mocaIfAccessEntry  "mocaIfAccessEntry"
  #define kNAME_mocaIfAccessIndex  "mocaIfAccessIndex"
  #define kNAME_mocaIfAccessMacAddress  "mocaIfAccessMacAddress"
  #define kNAME_mocaIfAccessStatus  "mocaIfAccessStatus"
#else
  #define kNAME_mocaIfAccessTable  kOID_mocaIfAccessTable
  #define kNAME_mocaIfAccessEntry  kOID_mocaIfAccessEntry
  #define kNAME_mocaIfAccessIndex  kOID_mocaIfAccessIndex
  #define kNAME_mocaIfAccessMacAddress  kOID_mocaIfAccessMacAddress
  #define kNAME_mocaIfAccessStatus  kOID_mocaIfAccessStatus
#endif

class mocaIfAccessTable : public BcmMibTable
{
  public:
    mocaIfAccessTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
    BcmMibEntry *NewDefaultEntry (BcmObjectId &Index);
};

class mocaIfAccessEntry : public BcmMibEntry
{
  friend class mocaIfAccessEntryBridge;
  friend class mocaIfAccessTable;

  public:
    mocaIfAccessEntry (mocaIfAccessTable *pTable, mocaIfAccessEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS TestLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    mocaIfAccessEntryBridge *fpBridge;

    enum
    {
      kLEAF_mocaIfAccessIndex = 1,
      kLEAF_mocaIfAccessMacAddress = 2,
      kLEAF_mocaIfAccessStatus = 3
    };
};


/* mocaIfStatusTable and related objects */

// OID constants
#define kOID_mocaIfStatusTable  "1.3.6.1.4.1.31621.1.1.1.3"
#define kOID_mocaIfStatusEntry  "1.3.6.1.4.1.31621.1.1.1.3.1"
#define kOID_mocaIfStatus  "1.3.6.1.4.1.31621.1.1.1.3.1.1"
#define kOID_mocaIfLinkUpTime  "1.3.6.1.4.1.31621.1.1.1.3.1.2"
#define kOID_mocaIfSoftwareVersion  "1.3.6.1.4.1.31621.1.1.1.3.1.3"
#define kOID_mocaIfMocaVersion  "1.3.6.1.4.1.31621.1.1.1.3.1.4"
#define kOID_mocaIfNetworkVersion  "1.3.6.1.4.1.31621.1.1.1.3.1.5"
#define kOID_mocaIfMacAddress  "1.3.6.1.4.1.31621.1.1.1.3.1.6"
#define kOID_mocaIfNodeID  "1.3.6.1.4.1.31621.1.1.1.3.1.7"
#define kOID_mocaIfName  "1.3.6.1.4.1.31621.1.1.1.3.1.8"
#define kOID_mocaIfNumNodes  "1.3.6.1.4.1.31621.1.1.1.3.1.9"
#define kOID_mocaIfNC  "1.3.6.1.4.1.31621.1.1.1.3.1.10"
#define kOID_mocaIfBackupNC  "1.3.6.1.4.1.31621.1.1.1.3.1.11"
#define kOID_mocaIfRFChannel  "1.3.6.1.4.1.31621.1.1.1.3.1.12"
#define kOID_mocaIfLOF  "1.3.6.1.4.1.31621.1.1.1.3.1.13"
#define kOID_mocaIfTabooChannelMask  "1.3.6.1.4.1.31621.1.1.1.3.1.14"
#define kOID_mocaIfNodeTabooChannelMask  "1.3.6.1.4.1.31621.1.1.1.3.1.15"
#define kOID_mocaIfCapabilityMask  "1.3.6.1.4.1.31621.1.1.1.3.1.16"
#define kOID_mocaIfTxGcdPowerReduction  "1.3.6.1.4.1.31621.1.1.1.3.1.17"
#define kOID_mocaIfQAM256Capable  "1.3.6.1.4.1.31621.1.1.1.3.1.18"
#define kOID_mocaIfPacketsAggrCapability  "1.3.6.1.4.1.31621.1.1.1.3.1.19"
#define kOID_mocaIfMaxIngressNodeBw  "1.3.6.1.4.1.31621.1.1.1.3.1.20"
#define kOID_mocaIfMaxEgressNodeBw  "1.3.6.1.4.1.31621.1.1.1.3.1.21"
#define kOID_mocaIfTxGcdRate  "1.3.6.1.4.1.31621.1.1.1.3.1.22"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_mocaIfStatusTable  "mocaIfStatusTable"
  #define kNAME_mocaIfStatusEntry  "mocaIfStatusEntry"
  #define kNAME_mocaIfStatus  "mocaIfStatus"
  #define kNAME_mocaIfLinkUpTime  "mocaIfLinkUpTime"
  #define kNAME_mocaIfSoftwareVersion  "mocaIfSoftwareVersion"
  #define kNAME_mocaIfMocaVersion  "mocaIfMocaVersion"
  #define kNAME_mocaIfNetworkVersion  "mocaIfNetworkVersion"
  #define kNAME_mocaIfMacAddress  "mocaIfMacAddress"
  #define kNAME_mocaIfNodeID  "mocaIfNodeID"
  #define kNAME_mocaIfName  "mocaIfName"
  #define kNAME_mocaIfNumNodes  "mocaIfNumNodes"
  #define kNAME_mocaIfNC  "mocaIfNC"
  #define kNAME_mocaIfBackupNC  "mocaIfBackupNC"
  #define kNAME_mocaIfRFChannel  "mocaIfRFChannel"
  #define kNAME_mocaIfLOF  "mocaIfLOF"
  #define kNAME_mocaIfTabooChannelMask  "mocaIfTabooChannelMask"
  #define kNAME_mocaIfNodeTabooChannelMask  "mocaIfNodeTabooChannelMask"
  #define kNAME_mocaIfCapabilityMask  "mocaIfCapabilityMask"
  #define kNAME_mocaIfTxGcdPowerReduction  "mocaIfTxGcdPowerReduction"
  #define kNAME_mocaIfQAM256Capable  "mocaIfQAM256Capable"
  #define kNAME_mocaIfPacketsAggrCapability  "mocaIfPacketsAggrCapability"
  #define kNAME_mocaIfMaxIngressNodeBw  "mocaIfMaxIngressNodeBw"
  #define kNAME_mocaIfMaxEgressNodeBw  "mocaIfMaxEgressNodeBw"
  #define kNAME_mocaIfTxGcdRate  "mocaIfTxGcdRate"
#else
  #define kNAME_mocaIfStatusTable  kOID_mocaIfStatusTable
  #define kNAME_mocaIfStatusEntry  kOID_mocaIfStatusEntry
  #define kNAME_mocaIfStatus  kOID_mocaIfStatus
  #define kNAME_mocaIfLinkUpTime  kOID_mocaIfLinkUpTime
  #define kNAME_mocaIfSoftwareVersion  kOID_mocaIfSoftwareVersion
  #define kNAME_mocaIfMocaVersion  kOID_mocaIfMocaVersion
  #define kNAME_mocaIfNetworkVersion  kOID_mocaIfNetworkVersion
  #define kNAME_mocaIfMacAddress  kOID_mocaIfMacAddress
  #define kNAME_mocaIfNodeID  kOID_mocaIfNodeID
  #define kNAME_mocaIfName  kOID_mocaIfName
  #define kNAME_mocaIfNumNodes  kOID_mocaIfNumNodes
  #define kNAME_mocaIfNC  kOID_mocaIfNC
  #define kNAME_mocaIfBackupNC  kOID_mocaIfBackupNC
  #define kNAME_mocaIfRFChannel  kOID_mocaIfRFChannel
  #define kNAME_mocaIfLOF  kOID_mocaIfLOF
  #define kNAME_mocaIfTabooChannelMask  kOID_mocaIfTabooChannelMask
  #define kNAME_mocaIfNodeTabooChannelMask  kOID_mocaIfNodeTabooChannelMask
  #define kNAME_mocaIfCapabilityMask  kOID_mocaIfCapabilityMask
  #define kNAME_mocaIfTxGcdPowerReduction  kOID_mocaIfTxGcdPowerReduction
  #define kNAME_mocaIfQAM256Capable  kOID_mocaIfQAM256Capable
  #define kNAME_mocaIfPacketsAggrCapability  kOID_mocaIfPacketsAggrCapability
  #define kNAME_mocaIfMaxIngressNodeBw  kOID_mocaIfMaxIngressNodeBw
  #define kNAME_mocaIfMaxEgressNodeBw  kOID_mocaIfMaxEgressNodeBw
  #define kNAME_mocaIfTxGcdRate  kOID_mocaIfTxGcdRate
#endif

class mocaIfStatusTable : public BcmMibTable
{
  public:
    mocaIfStatusTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class mocaIfStatusEntry : public BcmMibEntry
{
  friend class mocaIfStatusEntryBridge;
  friend class mocaIfStatusTable;

  public:
    mocaIfStatusEntry (mocaIfStatusTable *pTable, mocaIfStatusEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    mocaIfStatusEntryBridge *fpBridge;

    enum
    {
      kLEAF_mocaIfStatus = 1,
      kLEAF_mocaIfLinkUpTime = 2,
      kLEAF_mocaIfSoftwareVersion = 3,
      kLEAF_mocaIfMocaVersion = 4,
      kLEAF_mocaIfNetworkVersion = 5,
      kLEAF_mocaIfMacAddress = 6,
      kLEAF_mocaIfNodeID = 7,
      kLEAF_mocaIfName = 8,
      kLEAF_mocaIfNumNodes = 9,
      kLEAF_mocaIfNC = 10,
      kLEAF_mocaIfBackupNC = 11,
      kLEAF_mocaIfRFChannel = 12,
      kLEAF_mocaIfLOF = 13,
      kLEAF_mocaIfTabooChannelMask = 14,
      kLEAF_mocaIfNodeTabooChannelMask = 15,
      kLEAF_mocaIfCapabilityMask = 16,
      kLEAF_mocaIfTxGcdPowerReduction = 17,
      kLEAF_mocaIfQAM256Capable = 18,
      kLEAF_mocaIfPacketsAggrCapability = 19,
      kLEAF_mocaIfMaxIngressNodeBw = 20,
      kLEAF_mocaIfMaxEgressNodeBw = 21,
      kLEAF_mocaIfTxGcdRate = 22
    };
};


/* mocaIfStatsTable and related objects */

// OID constants
#define kOID_mocaIfStatsTable  "1.3.6.1.4.1.31621.1.1.1.4"
#define kOID_mocaIfStatsEntry  "1.3.6.1.4.1.31621.1.1.1.4.1"
#define kOID_mocaIfTxPackets  "1.3.6.1.4.1.31621.1.1.1.4.1.1"
#define kOID_mocaIfTxDrops  "1.3.6.1.4.1.31621.1.1.1.4.1.2"
#define kOID_mocaIfRxPackets  "1.3.6.1.4.1.31621.1.1.1.4.1.3"
#define kOID_mocaIfRxCorrectedErrors  "1.3.6.1.4.1.31621.1.1.1.4.1.4"
#define kOID_mocaIfRxDrops  "1.3.6.1.4.1.31621.1.1.1.4.1.5"
#define kOID_mocaIfEgressNodeNumFlows  "1.3.6.1.4.1.31621.1.1.1.4.1.6"
#define kOID_mocaIfIngressNodeNumFlows  "1.3.6.1.4.1.31621.1.1.1.4.1.7"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_mocaIfStatsTable  "mocaIfStatsTable"
  #define kNAME_mocaIfStatsEntry  "mocaIfStatsEntry"
  #define kNAME_mocaIfTxPackets  "mocaIfTxPackets"
  #define kNAME_mocaIfTxDrops  "mocaIfTxDrops"
  #define kNAME_mocaIfRxPackets  "mocaIfRxPackets"
  #define kNAME_mocaIfRxCorrectedErrors  "mocaIfRxCorrectedErrors"
  #define kNAME_mocaIfRxDrops  "mocaIfRxDrops"
  #define kNAME_mocaIfEgressNodeNumFlows  "mocaIfEgressNodeNumFlows"
  #define kNAME_mocaIfIngressNodeNumFlows  "mocaIfIngressNodeNumFlows"
#else
  #define kNAME_mocaIfStatsTable  kOID_mocaIfStatsTable
  #define kNAME_mocaIfStatsEntry  kOID_mocaIfStatsEntry
  #define kNAME_mocaIfTxPackets  kOID_mocaIfTxPackets
  #define kNAME_mocaIfTxDrops  kOID_mocaIfTxDrops
  #define kNAME_mocaIfRxPackets  kOID_mocaIfRxPackets
  #define kNAME_mocaIfRxCorrectedErrors  kOID_mocaIfRxCorrectedErrors
  #define kNAME_mocaIfRxDrops  kOID_mocaIfRxDrops
  #define kNAME_mocaIfEgressNodeNumFlows  kOID_mocaIfEgressNodeNumFlows
  #define kNAME_mocaIfIngressNodeNumFlows  kOID_mocaIfIngressNodeNumFlows
#endif

class mocaIfStatsTable : public BcmMibTable
{
  public:
    mocaIfStatsTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class mocaIfStatsEntry : public BcmMibEntry
{
  friend class mocaIfStatsEntryBridge;
  friend class mocaIfStatsTable;

  public:
    mocaIfStatsEntry (mocaIfStatsTable *pTable, mocaIfStatsEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    mocaIfStatsEntryBridge *fpBridge;

    enum
    {
      kLEAF_mocaIfTxPackets = 1,
      kLEAF_mocaIfTxDrops = 2,
      kLEAF_mocaIfRxPackets = 3,
      kLEAF_mocaIfRxCorrectedErrors = 4,
      kLEAF_mocaIfRxDrops = 5,
      kLEAF_mocaIfEgressNodeNumFlows = 6,
      kLEAF_mocaIfIngressNodeNumFlows = 7
    };
};


/* mocaIfFlowStatsTable and related objects */

// OID constants
#define kOID_mocaIfFlowStatsTable  "1.3.6.1.4.1.31621.1.1.1.5"
#define kOID_mocaIfFlowStatsEntry  "1.3.6.1.4.1.31621.1.1.1.5.1"
#define kOID_mocaIfFlowIndex  "1.3.6.1.4.1.31621.1.1.1.5.1.1"
#define kOID_mocaIfFlowID  "1.3.6.1.4.1.31621.1.1.1.5.1.2"
#define kOID_mocaIfPacketDA  "1.3.6.1.4.1.31621.1.1.1.5.1.3"
#define kOID_mocaIfPeakDataRate  "1.3.6.1.4.1.31621.1.1.1.5.1.4"
#define kOID_mocaIfBurstSize  "1.3.6.1.4.1.31621.1.1.1.5.1.5"
#define kOID_mocaIfLeaseTime  "1.3.6.1.4.1.31621.1.1.1.5.1.6"
#define kOID_mocaIfFlowTag  "1.3.6.1.4.1.31621.1.1.1.5.1.7"
#define kOID_mocaIfLeaseTimeLeft  "1.3.6.1.4.1.31621.1.1.1.5.1.8"
#define kOID_mocaIfTxPacketsOnePacketAggr  "1.3.6.1.4.1.31621.1.1.1.5.1.9"
#define kOID_mocaIfTxPacketsTwoPacketsAggr  "1.3.6.1.4.1.31621.1.1.1.5.1.10"
#define kOID_mocaIfTxPacketsThreePacketsAggr  "1.3.6.1.4.1.31621.1.1.1.5.1.11"
#define kOID_mocaIfTxPacketsFourPacketsAggr  "1.3.6.1.4.1.31621.1.1.1.5.1.12"
#define kOID_mocaIfTxPacketsFivePacketsAggr  "1.3.6.1.4.1.31621.1.1.1.5.1.13"
#define kOID_mocaIfTxPacketsSixPacketsAggr  "1.3.6.1.4.1.31621.1.1.1.5.1.14"
#define kOID_mocaIfTxPacketsSevenPacketsAggr  "1.3.6.1.4.1.31621.1.1.1.5.1.15"
#define kOID_mocaIfTxPacketsEightPacketsAggr  "1.3.6.1.4.1.31621.1.1.1.5.1.16"
#define kOID_mocaIfTxPacketsNinePacketsAggr  "1.3.6.1.4.1.31621.1.1.1.5.1.17"
#define kOID_mocaIfTxPacketsTenPacketsAggr  "1.3.6.1.4.1.31621.1.1.1.5.1.18"
#define kOID_mocaIfTxPacketsFlow  "1.3.6.1.4.1.31621.1.1.1.5.1.19"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_mocaIfFlowStatsTable  "mocaIfFlowStatsTable"
  #define kNAME_mocaIfFlowStatsEntry  "mocaIfFlowStatsEntry"
  #define kNAME_mocaIfFlowIndex  "mocaIfFlowIndex"
  #define kNAME_mocaIfFlowID  "mocaIfFlowID"
  #define kNAME_mocaIfPacketDA  "mocaIfPacketDA"
  #define kNAME_mocaIfPeakDataRate  "mocaIfPeakDataRate"
  #define kNAME_mocaIfBurstSize  "mocaIfBurstSize"
  #define kNAME_mocaIfLeaseTime  "mocaIfLeaseTime"
  #define kNAME_mocaIfFlowTag  "mocaIfFlowTag"
  #define kNAME_mocaIfLeaseTimeLeft  "mocaIfLeaseTimeLeft"
  #define kNAME_mocaIfTxPacketsOnePacketAggr  "mocaIfTxPacketsOnePacketAggr"
  #define kNAME_mocaIfTxPacketsTwoPacketsAggr  "mocaIfTxPacketsTwoPacketsAggr"
  #define kNAME_mocaIfTxPacketsThreePacketsAggr  "mocaIfTxPacketsThreePacketsAggr"
  #define kNAME_mocaIfTxPacketsFourPacketsAggr  "mocaIfTxPacketsFourPacketsAggr"
  #define kNAME_mocaIfTxPacketsFivePacketsAggr  "mocaIfTxPacketsFivePacketsAggr"
  #define kNAME_mocaIfTxPacketsSixPacketsAggr  "mocaIfTxPacketsSixPacketsAggr"
  #define kNAME_mocaIfTxPacketsSevenPacketsAggr  "mocaIfTxPacketsSevenPacketsAggr"
  #define kNAME_mocaIfTxPacketsEightPacketsAggr  "mocaIfTxPacketsEightPacketsAggr"
  #define kNAME_mocaIfTxPacketsNinePacketsAggr  "mocaIfTxPacketsNinePacketsAggr"
  #define kNAME_mocaIfTxPacketsTenPacketsAggr  "mocaIfTxPacketsTenPacketsAggr"
  #define kNAME_mocaIfTxPacketsFlow  "mocaIfTxPacketsFlow"
#else
  #define kNAME_mocaIfFlowStatsTable  kOID_mocaIfFlowStatsTable
  #define kNAME_mocaIfFlowStatsEntry  kOID_mocaIfFlowStatsEntry
  #define kNAME_mocaIfFlowIndex  kOID_mocaIfFlowIndex
  #define kNAME_mocaIfFlowID  kOID_mocaIfFlowID
  #define kNAME_mocaIfPacketDA  kOID_mocaIfPacketDA
  #define kNAME_mocaIfPeakDataRate  kOID_mocaIfPeakDataRate
  #define kNAME_mocaIfBurstSize  kOID_mocaIfBurstSize
  #define kNAME_mocaIfLeaseTime  kOID_mocaIfLeaseTime
  #define kNAME_mocaIfFlowTag  kOID_mocaIfFlowTag
  #define kNAME_mocaIfLeaseTimeLeft  kOID_mocaIfLeaseTimeLeft
  #define kNAME_mocaIfTxPacketsOnePacketAggr  kOID_mocaIfTxPacketsOnePacketAggr
  #define kNAME_mocaIfTxPacketsTwoPacketsAggr  kOID_mocaIfTxPacketsTwoPacketsAggr
  #define kNAME_mocaIfTxPacketsThreePacketsAggr  kOID_mocaIfTxPacketsThreePacketsAggr
  #define kNAME_mocaIfTxPacketsFourPacketsAggr  kOID_mocaIfTxPacketsFourPacketsAggr
  #define kNAME_mocaIfTxPacketsFivePacketsAggr  kOID_mocaIfTxPacketsFivePacketsAggr
  #define kNAME_mocaIfTxPacketsSixPacketsAggr  kOID_mocaIfTxPacketsSixPacketsAggr
  #define kNAME_mocaIfTxPacketsSevenPacketsAggr  kOID_mocaIfTxPacketsSevenPacketsAggr
  #define kNAME_mocaIfTxPacketsEightPacketsAggr  kOID_mocaIfTxPacketsEightPacketsAggr
  #define kNAME_mocaIfTxPacketsNinePacketsAggr  kOID_mocaIfTxPacketsNinePacketsAggr
  #define kNAME_mocaIfTxPacketsTenPacketsAggr  kOID_mocaIfTxPacketsTenPacketsAggr
  #define kNAME_mocaIfTxPacketsFlow  kOID_mocaIfTxPacketsFlow
#endif

class mocaIfFlowStatsTable : public BcmMibTable
{
  public:
    mocaIfFlowStatsTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class mocaIfFlowStatsEntry : public BcmMibEntry
{
  friend class mocaIfFlowStatsEntryBridge;
  friend class mocaIfFlowStatsTable;

  public:
    mocaIfFlowStatsEntry (mocaIfFlowStatsTable *pTable, mocaIfFlowStatsEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    mocaIfFlowStatsEntryBridge *fpBridge;

    enum
    {
      kLEAF_mocaIfFlowIndex = 1,
      kLEAF_mocaIfFlowID = 2,
      kLEAF_mocaIfPacketDA = 3,
      kLEAF_mocaIfPeakDataRate = 4,
      kLEAF_mocaIfBurstSize = 5,
      kLEAF_mocaIfLeaseTime = 6,
      kLEAF_mocaIfFlowTag = 7,
      kLEAF_mocaIfLeaseTimeLeft = 8,
      kLEAF_mocaIfTxPacketsOnePacketAggr = 9,
      kLEAF_mocaIfTxPacketsTwoPacketsAggr = 10,
      kLEAF_mocaIfTxPacketsThreePacketsAggr = 11,
      kLEAF_mocaIfTxPacketsFourPacketsAggr = 12,
      kLEAF_mocaIfTxPacketsFivePacketsAggr = 13,
      kLEAF_mocaIfTxPacketsSixPacketsAggr = 14,
      kLEAF_mocaIfTxPacketsSevenPacketsAggr = 15,
      kLEAF_mocaIfTxPacketsEightPacketsAggr = 16,
      kLEAF_mocaIfTxPacketsNinePacketsAggr = 17,
      kLEAF_mocaIfTxPacketsTenPacketsAggr = 18,
      kLEAF_mocaIfTxPacketsFlow = 19
    };
};


/* mocaNodeTable and related objects */

// OID constants
#define kOID_mocaNodeTable  "1.3.6.1.4.1.31621.1.1.1.6"
#define kOID_mocaNodeEntry  "1.3.6.1.4.1.31621.1.1.1.6.1"
#define kOID_mocaNodeIndex  "1.3.6.1.4.1.31621.1.1.1.6.1.1"
#define kOID_mocaNodeMocaVersion  "1.3.6.1.4.1.31621.1.1.1.6.1.2"
#define kOID_mocaNodeMacAddress  "1.3.6.1.4.1.31621.1.1.1.6.1.3"
#define kOID_mocaNodeTxGcdRate  "1.3.6.1.4.1.31621.1.1.1.6.1.4"
#define kOID_mocaNodeRxGcdPower  "1.3.6.1.4.1.31621.1.1.1.6.1.5"
#define kOID_mocaNodeTxPowerReduction  "1.3.6.1.4.1.31621.1.1.1.6.1.6"
#define kOID_mocaNodeRxPower  "1.3.6.1.4.1.31621.1.1.1.6.1.7"
#define kOID_mocaNodePreferredNC  "1.3.6.1.4.1.31621.1.1.1.6.1.8"
#define kOID_mocaNodeQAM256Capable  "1.3.6.1.4.1.31621.1.1.1.6.1.9"
#define kOID_mocaNodePacketsAggrCapability  "1.3.6.1.4.1.31621.1.1.1.6.1.10"
#define kOID_mocaNodeRxPackets  "1.3.6.1.4.1.31621.1.1.1.6.1.11"
#define kOID_mocaNodeRxCorrected  "1.3.6.1.4.1.31621.1.1.1.6.1.12"
#define kOID_mocaNodeRxDrops  "1.3.6.1.4.1.31621.1.1.1.6.1.13"
#define kOID_mocaNodeSNR  "1.3.6.1.4.1.31621.1.1.1.6.1.14"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_mocaNodeTable  "mocaNodeTable"
  #define kNAME_mocaNodeEntry  "mocaNodeEntry"
  #define kNAME_mocaNodeIndex  "mocaNodeIndex"
  #define kNAME_mocaNodeMocaVersion  "mocaNodeMocaVersion"
  #define kNAME_mocaNodeMacAddress  "mocaNodeMacAddress"
  #define kNAME_mocaNodeTxGcdRate  "mocaNodeTxGcdRate"
  #define kNAME_mocaNodeRxGcdPower  "mocaNodeRxGcdPower"
  #define kNAME_mocaNodeTxPowerReduction  "mocaNodeTxPowerReduction"
  #define kNAME_mocaNodeRxPower  "mocaNodeRxPower"
  #define kNAME_mocaNodePreferredNC  "mocaNodePreferredNC"
  #define kNAME_mocaNodeQAM256Capable  "mocaNodeQAM256Capable"
  #define kNAME_mocaNodePacketsAggrCapability  "mocaNodePacketsAggrCapability"
  #define kNAME_mocaNodeRxPackets  "mocaNodeRxPackets"
  #define kNAME_mocaNodeRxCorrected  "mocaNodeRxCorrected"
  #define kNAME_mocaNodeRxDrops  "mocaNodeRxDrops"
  #define kNAME_mocaNodeSNR  "mocaNodeSNR"
#else
  #define kNAME_mocaNodeTable  kOID_mocaNodeTable
  #define kNAME_mocaNodeEntry  kOID_mocaNodeEntry
  #define kNAME_mocaNodeIndex  kOID_mocaNodeIndex
  #define kNAME_mocaNodeMocaVersion  kOID_mocaNodeMocaVersion
  #define kNAME_mocaNodeMacAddress  kOID_mocaNodeMacAddress
  #define kNAME_mocaNodeTxGcdRate  kOID_mocaNodeTxGcdRate
  #define kNAME_mocaNodeRxGcdPower  kOID_mocaNodeRxGcdPower
  #define kNAME_mocaNodeTxPowerReduction  kOID_mocaNodeTxPowerReduction
  #define kNAME_mocaNodeRxPower  kOID_mocaNodeRxPower
  #define kNAME_mocaNodePreferredNC  kOID_mocaNodePreferredNC
  #define kNAME_mocaNodeQAM256Capable  kOID_mocaNodeQAM256Capable
  #define kNAME_mocaNodePacketsAggrCapability  kOID_mocaNodePacketsAggrCapability
  #define kNAME_mocaNodeRxPackets  kOID_mocaNodeRxPackets
  #define kNAME_mocaNodeRxCorrected  kOID_mocaNodeRxCorrected
  #define kNAME_mocaNodeRxDrops  kOID_mocaNodeRxDrops
  #define kNAME_mocaNodeSNR  kOID_mocaNodeSNR
#endif

class mocaNodeTable : public BcmMibTable
{
  public:
    mocaNodeTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class mocaNodeEntry : public BcmMibEntry
{
  friend class mocaNodeEntryBridge;
  friend class mocaNodeTable;

  public:
    mocaNodeEntry (mocaNodeTable *pTable, mocaNodeEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    mocaNodeEntryBridge *fpBridge;

    enum
    {
      kLEAF_mocaNodeIndex = 1,
      kLEAF_mocaNodeMocaVersion = 2,
      kLEAF_mocaNodeMacAddress = 3,
      kLEAF_mocaNodeTxGcdRate = 4,
      kLEAF_mocaNodeRxGcdPower = 5,
      kLEAF_mocaNodeTxPowerReduction = 6,
      kLEAF_mocaNodeRxPower = 7,
      kLEAF_mocaNodePreferredNC = 8,
      kLEAF_mocaNodeQAM256Capable = 9,
      kLEAF_mocaNodePacketsAggrCapability = 10,
      kLEAF_mocaNodeRxPackets = 11,
      kLEAF_mocaNodeRxCorrected = 12,
      kLEAF_mocaNodeRxDrops = 13,
      kLEAF_mocaNodeSNR = 14
    };
};


/* mocaMeshTable and related objects */

// OID constants
#define kOID_mocaMeshTable  "1.3.6.1.4.1.31621.1.1.1.7"
#define kOID_mocaMeshEntry  "1.3.6.1.4.1.31621.1.1.1.7.1"
#define kOID_mocaMeshTxNodeIndex  "1.3.6.1.4.1.31621.1.1.1.7.1.1"
#define kOID_mocaMeshRxNodeIndex  "1.3.6.1.4.1.31621.1.1.1.7.1.2"
#define kOID_mocaMeshTxRate  "1.3.6.1.4.1.31621.1.1.1.7.1.3"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_mocaMeshTable  "mocaMeshTable"
  #define kNAME_mocaMeshEntry  "mocaMeshEntry"
  #define kNAME_mocaMeshTxNodeIndex  "mocaMeshTxNodeIndex"
  #define kNAME_mocaMeshRxNodeIndex  "mocaMeshRxNodeIndex"
  #define kNAME_mocaMeshTxRate  "mocaMeshTxRate"
#else
  #define kNAME_mocaMeshTable  kOID_mocaMeshTable
  #define kNAME_mocaMeshEntry  kOID_mocaMeshEntry
  #define kNAME_mocaMeshTxNodeIndex  kOID_mocaMeshTxNodeIndex
  #define kNAME_mocaMeshRxNodeIndex  kOID_mocaMeshRxNodeIndex
  #define kNAME_mocaMeshTxRate  kOID_mocaMeshTxRate
#endif

class mocaMeshTable : public BcmMibTable
{
  public:
    mocaMeshTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class mocaMeshEntry : public BcmMibEntry
{
  friend class mocaMeshEntryBridge;
  friend class mocaMeshTable;

  public:
    mocaMeshEntry (mocaMeshTable *pTable, mocaMeshEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    mocaMeshEntryBridge *fpBridge;

    enum
    {
      kLEAF_mocaMeshTxNodeIndex = 1,
      kLEAF_mocaMeshRxNodeIndex = 2,
      kLEAF_mocaMeshTxRate = 3
    };
};


/* mocaBridgeTable and related objects */

// OID constants
#define kOID_mocaBridgeTable  "1.3.6.1.4.1.31621.1.1.1.8"
#define kOID_mocaBridgeEntry  "1.3.6.1.4.1.31621.1.1.1.8.1"
#define kOID_mocaBridgeNodeIndex  "1.3.6.1.4.1.31621.1.1.1.8.1.1"
#define kOID_mocaBridgeMacIndex  "1.3.6.1.4.1.31621.1.1.1.8.1.2"
#define kOID_mocaBridgeMacAddress  "1.3.6.1.4.1.31621.1.1.1.8.1.3"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_mocaBridgeTable  "mocaBridgeTable"
  #define kNAME_mocaBridgeEntry  "mocaBridgeEntry"
  #define kNAME_mocaBridgeNodeIndex  "mocaBridgeNodeIndex"
  #define kNAME_mocaBridgeMacIndex  "mocaBridgeMacIndex"
  #define kNAME_mocaBridgeMacAddress  "mocaBridgeMacAddress"
#else
  #define kNAME_mocaBridgeTable  kOID_mocaBridgeTable
  #define kNAME_mocaBridgeEntry  kOID_mocaBridgeEntry
  #define kNAME_mocaBridgeNodeIndex  kOID_mocaBridgeNodeIndex
  #define kNAME_mocaBridgeMacIndex  kOID_mocaBridgeMacIndex
  #define kNAME_mocaBridgeMacAddress  kOID_mocaBridgeMacAddress
#endif

class mocaBridgeTable : public BcmMibTable
{
  public:
    mocaBridgeTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class mocaBridgeEntry : public BcmMibEntry
{
  friend class mocaBridgeEntryBridge;
  friend class mocaBridgeTable;

  public:
    mocaBridgeEntry (mocaBridgeTable *pTable, mocaBridgeEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    mocaBridgeEntryBridge *fpBridge;

    enum
    {
      kLEAF_mocaBridgeNodeIndex = 1,
      kLEAF_mocaBridgeMacIndex = 2,
      kLEAF_mocaBridgeMacAddress = 3
    };
};



#endif


