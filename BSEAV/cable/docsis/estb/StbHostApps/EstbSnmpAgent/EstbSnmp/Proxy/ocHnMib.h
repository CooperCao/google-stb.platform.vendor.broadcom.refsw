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
//    Filename: ocHnMib.h
//    Creation Date: Friday, June 10, 2011 at 11:07:23
//    Created by Broadcom BFC Mib Compiler BCMIBC.EXE version 3.6.1d
//    from input file oc-home-network.mib
//
//****************************************************************************

#ifndef OCHNMIB_H
#define OCHNMIB_H

#include "MibObjs.h"
#include "MibBridge.h"


class ocHnMibModuleGroupBridge;
class ocHnMibObjectsGroupBridge;
class ocHnHomeNetStatusGroupBridge;
class ocHnDevUpnpServiceEntryBridge;
class ocHnDevSupportedChannelEntryBridge;
class ocHnRsdManagerStatusGroupBridge;
class ocHnDevPropertiesGroupBridge;
class ocHnROConfigurationGroupBridge;
class ocHnDevInterfaceStatusGroupBridge;
class ocHnDevInterfaceConfigEntryBridge;
class ocHnDevInterfaceStatusEntryBridge;
class ocHnDevConnectionEntryBridge;


/*\
 *  ocHnMibModuleGroup group
\*/

// OID constants
#define kOID_ocHnMibModuleGroup  "1.3.6.1.4.1.4491.2.3.2"
#define kOID_ocHnNotificationsGroup  "1.3.6.1.4.1.4491.2.3.2.1"
#define kOID_ocHnMibObjectsGroup  "1.3.6.1.4.1.4491.2.3.2.2"
#define kOID_ocHnConformanceGroup  "1.3.6.1.4.1.4491.2.3.2.3"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocHnMibModuleGroup  "ocHnMibModuleGroup"
  #define kNAME_ocHnNotificationsGroup  "ocHnNotificationsGroup"
  #define kNAME_ocHnMibObjectsGroup  "ocHnMibObjectsGroup"
  #define kNAME_ocHnConformanceGroup  "ocHnConformanceGroup"
#else
  #define kNAME_ocHnMibModuleGroup  kOID_ocHnMibModuleGroup
  #define kNAME_ocHnNotificationsGroup  kOID_ocHnNotificationsGroup
  #define kNAME_ocHnMibObjectsGroup  kOID_ocHnMibObjectsGroup
  #define kNAME_ocHnConformanceGroup  kOID_ocHnConformanceGroup
#endif

class ocHnMibModuleGroup : public BcmMibGroup
{
  public:
    ocHnMibModuleGroup ();

    bool Populate ();
};



/*\
 *  ocHnMibObjectsGroup group
\*/

// OID constants
#define kOID_ocHnMibObjectsGroup  "1.3.6.1.4.1.4491.2.3.2.2"
#define kOID_ocHnHomeNetStatusGroup  "1.3.6.1.4.1.4491.2.3.2.2.1"
#define kOID_ocHnDevInterfaceStatusGroup  "1.3.6.1.4.1.4491.2.3.2.2.4"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocHnMibObjectsGroup  "ocHnMibObjectsGroup"
  #define kNAME_ocHnHomeNetStatusGroup  "ocHnHomeNetStatusGroup"
  #define kNAME_ocHnDevInterfaceStatusGroup  "ocHnDevInterfaceStatusGroup"
#else
  #define kNAME_ocHnMibObjectsGroup  kOID_ocHnMibObjectsGroup
  #define kNAME_ocHnHomeNetStatusGroup  kOID_ocHnHomeNetStatusGroup
  #define kNAME_ocHnDevInterfaceStatusGroup  kOID_ocHnDevInterfaceStatusGroup
#endif

class ocHnMibObjectsGroup : public BcmMibGroup
{
  public:
    ocHnMibObjectsGroup ();

    bool Populate ();
};



/*\
 *  ocHnHomeNetStatusGroup group
\*/

// OID constants
#define kOID_ocHnHomeNetStatusGroup  "1.3.6.1.4.1.4491.2.3.2.2.1"
#define kOID_ocHnRsdManagerStatusGroup  "1.3.6.1.4.1.4491.2.3.2.2.1.1"
#define kOID_ocHnDevPropertiesGroup  "1.3.6.1.4.1.4491.2.3.2.2.1.4"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocHnHomeNetStatusGroup  "ocHnHomeNetStatusGroup"
  #define kNAME_ocHnRsdManagerStatusGroup  "ocHnRsdManagerStatusGroup"
  #define kNAME_ocHnDevPropertiesGroup  "ocHnDevPropertiesGroup"
#else
  #define kNAME_ocHnHomeNetStatusGroup  kOID_ocHnHomeNetStatusGroup
  #define kNAME_ocHnRsdManagerStatusGroup  kOID_ocHnRsdManagerStatusGroup
  #define kNAME_ocHnDevPropertiesGroup  kOID_ocHnDevPropertiesGroup
#endif

class ocHnHomeNetStatusGroup : public BcmMibGroup
{
  public:
    ocHnHomeNetStatusGroup ();

    bool Populate ();
};


/* ocHnDevUpnpServiceTable and related objects */

// OID constants
#define kOID_ocHnDevUpnpServiceTable  "1.3.6.1.4.1.4491.2.3.2.2.1.2"
#define kOID_ocHnDevUpnpServiceEntry  "1.3.6.1.4.1.4491.2.3.2.2.1.2.1"
#define kOID_ocHnDevUpnpServiceIndex  "1.3.6.1.4.1.4491.2.3.2.2.1.2.1.1"
#define kOID_ocHnDevUpnpServiceType  "1.3.6.1.4.1.4491.2.3.2.2.1.2.1.2"
#define kOID_ocHnDevUpnpServiceAvailable  "1.3.6.1.4.1.4491.2.3.2.2.1.2.1.3"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocHnDevUpnpServiceTable  "ocHnDevUpnpServiceTable"
  #define kNAME_ocHnDevUpnpServiceEntry  "ocHnDevUpnpServiceEntry"
  #define kNAME_ocHnDevUpnpServiceIndex  "ocHnDevUpnpServiceIndex"
  #define kNAME_ocHnDevUpnpServiceType  "ocHnDevUpnpServiceType"
  #define kNAME_ocHnDevUpnpServiceAvailable  "ocHnDevUpnpServiceAvailable"
#else
  #define kNAME_ocHnDevUpnpServiceTable  kOID_ocHnDevUpnpServiceTable
  #define kNAME_ocHnDevUpnpServiceEntry  kOID_ocHnDevUpnpServiceEntry
  #define kNAME_ocHnDevUpnpServiceIndex  kOID_ocHnDevUpnpServiceIndex
  #define kNAME_ocHnDevUpnpServiceType  kOID_ocHnDevUpnpServiceType
  #define kNAME_ocHnDevUpnpServiceAvailable  kOID_ocHnDevUpnpServiceAvailable
#endif

class ocHnDevUpnpServiceTable : public BcmMibTable
{
  public:
    ocHnDevUpnpServiceTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ocHnDevUpnpServiceEntry : public BcmMibEntry
{
  friend class ocHnDevUpnpServiceEntryBridge;
  friend class ocHnDevUpnpServiceTable;

  public:
    ocHnDevUpnpServiceEntry (ocHnDevUpnpServiceTable *pTable, ocHnDevUpnpServiceEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocHnDevUpnpServiceEntryBridge *fpBridge;

    enum
    {
      kLEAF_ocHnDevUpnpServiceIndex = 1,
      kLEAF_ocHnDevUpnpServiceType = 2,
      kLEAF_ocHnDevUpnpServiceAvailable = 3
    };
};


/* ocHnDevSupportedChannelTable and related objects */

// OID constants
#define kOID_ocHnDevSupportedChannelTable  "1.3.6.1.4.1.4491.2.3.2.2.1.3"
#define kOID_ocHnDevSupportedChannelEntry  "1.3.6.1.4.1.4491.2.3.2.2.1.3.1"
#define kOID_ocHnDevSupportedChannelIndex  "1.3.6.1.4.1.4491.2.3.2.2.1.3.1.1"
#define kOID_ocHnDevSupportedChannelLastOperatingFreq  "1.3.6.1.4.1.4491.2.3.2.2.1.3.1.2"
#define kOID_ocHnDevSupportedChannelInUse  "1.3.6.1.4.1.4491.2.3.2.2.1.3.1.3"
#define kOID_ocHnDevSupportedChannelFrequency  "1.3.6.1.4.1.4491.2.3.2.2.1.3.1.4"
#define kOID_ocHnDevSupportedChannelEligible  "1.3.6.1.4.1.4491.2.3.2.2.1.3.1.5"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocHnDevSupportedChannelTable  "ocHnDevSupportedChannelTable"
  #define kNAME_ocHnDevSupportedChannelEntry  "ocHnDevSupportedChannelEntry"
  #define kNAME_ocHnDevSupportedChannelIndex  "ocHnDevSupportedChannelIndex"
  #define kNAME_ocHnDevSupportedChannelLastOperatingFreq  "ocHnDevSupportedChannelLastOperatingFreq"
  #define kNAME_ocHnDevSupportedChannelInUse  "ocHnDevSupportedChannelInUse"
  #define kNAME_ocHnDevSupportedChannelFrequency  "ocHnDevSupportedChannelFrequency"
  #define kNAME_ocHnDevSupportedChannelEligible  "ocHnDevSupportedChannelEligible"
#else
  #define kNAME_ocHnDevSupportedChannelTable  kOID_ocHnDevSupportedChannelTable
  #define kNAME_ocHnDevSupportedChannelEntry  kOID_ocHnDevSupportedChannelEntry
  #define kNAME_ocHnDevSupportedChannelIndex  kOID_ocHnDevSupportedChannelIndex
  #define kNAME_ocHnDevSupportedChannelLastOperatingFreq  kOID_ocHnDevSupportedChannelLastOperatingFreq
  #define kNAME_ocHnDevSupportedChannelInUse  kOID_ocHnDevSupportedChannelInUse
  #define kNAME_ocHnDevSupportedChannelFrequency  kOID_ocHnDevSupportedChannelFrequency
  #define kNAME_ocHnDevSupportedChannelEligible  kOID_ocHnDevSupportedChannelEligible
#endif

class ocHnDevSupportedChannelTable : public BcmMibTable
{
  public:
    ocHnDevSupportedChannelTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ocHnDevSupportedChannelEntry : public BcmMibEntry
{
  friend class ocHnDevSupportedChannelEntryBridge;
  friend class ocHnDevSupportedChannelTable;

  public:
    ocHnDevSupportedChannelEntry (ocHnDevSupportedChannelTable *pTable, ocHnDevSupportedChannelEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocHnDevSupportedChannelEntryBridge *fpBridge;

    enum
    {
      kLEAF_ocHnDevSupportedChannelIndex = 1,
      kLEAF_ocHnDevSupportedChannelLastOperatingFreq = 2,
      kLEAF_ocHnDevSupportedChannelInUse = 3,
      kLEAF_ocHnDevSupportedChannelFrequency = 4,
      kLEAF_ocHnDevSupportedChannelEligible = 5
    };
};



/*\
 *  ocHnRsdManagerStatusGroup group
\*/

// OID constants
#define kOID_ocHnRsdManagerStatusGroup  "1.3.6.1.4.1.4491.2.3.2.2.1.1"
#define kOID_ocHnRsdManagerImportanceNumber  "1.3.6.1.4.1.4491.2.3.2.2.1.1.1.0"
#define kOID_ocHnRsdManagerPreferredStatus  "1.3.6.1.4.1.4491.2.3.2.2.1.1.2.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocHnRsdManagerStatusGroup  "ocHnRsdManagerStatusGroup"
  #define kNAME_ocHnRsdManagerImportanceNumber  "ocHnRsdManagerImportanceNumber"
  #define kNAME_ocHnRsdManagerPreferredStatus  "ocHnRsdManagerPreferredStatus"
#else
  #define kNAME_ocHnRsdManagerStatusGroup  kOID_ocHnRsdManagerStatusGroup
  #define kNAME_ocHnRsdManagerImportanceNumber  kOID_ocHnRsdManagerImportanceNumber
  #define kNAME_ocHnRsdManagerPreferredStatus  kOID_ocHnRsdManagerPreferredStatus
#endif

class ocHnRsdManagerStatusGroup : public BcmMibGroup
{
  friend class ocHnRsdManagerStatusGroupBridge;

  public:
    ocHnRsdManagerStatusGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocHnRsdManagerStatusGroupBridge *fpBridge;

    enum
    {
      kLEAF_ocHnRsdManagerImportanceNumber = 1,
      kLEAF_ocHnRsdManagerPreferredStatus = 2
    };
};



/*\
 *  ocHnDevPropertiesGroup group
\*/

// OID constants
#define kOID_ocHnDevPropertiesGroup  "1.3.6.1.4.1.4491.2.3.2.2.1.4"
#define kOID_ocHnDevPropertiesFriendlyName  "1.3.6.1.4.1.4491.2.3.2.2.1.4.1.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocHnDevPropertiesGroup  "ocHnDevPropertiesGroup"
  #define kNAME_ocHnDevPropertiesFriendlyName  "ocHnDevPropertiesFriendlyName"
#else
  #define kNAME_ocHnDevPropertiesGroup  kOID_ocHnDevPropertiesGroup
  #define kNAME_ocHnDevPropertiesFriendlyName  kOID_ocHnDevPropertiesFriendlyName
#endif

class ocHnDevPropertiesGroup : public BcmMibGroup
{
  friend class ocHnDevPropertiesGroupBridge;

  public:
    ocHnDevPropertiesGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocHnDevPropertiesGroupBridge *fpBridge;

    enum
    {
      kLEAF_ocHnDevPropertiesFriendlyName = 1
    };
};



/*\
 *  ocHnDevInterfaceStatusGroup group
\*/

// OID constants
#define kOID_ocHnDevInterfaceStatusGroup  "1.3.6.1.4.1.4491.2.3.2.2.4"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocHnDevInterfaceStatusGroup  "ocHnDevInterfaceStatusGroup"
#else
  #define kNAME_ocHnDevInterfaceStatusGroup  kOID_ocHnDevInterfaceStatusGroup
#endif

class ocHnDevInterfaceStatusGroup : public BcmMibGroup
{
  public:
    ocHnDevInterfaceStatusGroup ();

    bool Populate ();
};


/* ocHnDevInterfaceConfigTable and related objects */

// OID constants
#define kOID_ocHnDevInterfaceConfigTable  "1.3.6.1.4.1.4491.2.3.2.2.4.1"
#define kOID_ocHnDevInterfaceConfigEntry  "1.3.6.1.4.1.4491.2.3.2.2.4.1.1"
#define kOID_ocHnDevInterfaceConfigMaxPowerLevel  "1.3.6.1.4.1.4491.2.3.2.2.4.1.1.2"
#define kOID_ocHnDevInterfaceConfigPowerUnits  "1.3.6.1.4.1.4491.2.3.2.2.4.1.1.3"
#define kOID_ocHnDevInterfaceConfigMaxParameterizedBandwidth  "1.3.6.1.4.1.4491.2.3.2.2.4.1.1.4"
#define kOID_ocHnDevInterfaceConfigEnableEncryption  "1.3.6.1.4.1.4491.2.3.2.2.4.1.1.5"
#define kOID_ocHnDevInterfaceConfigEncryptionPassphrase  "1.3.6.1.4.1.4491.2.3.2.2.4.1.1.6"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocHnDevInterfaceConfigTable  "ocHnDevInterfaceConfigTable"
  #define kNAME_ocHnDevInterfaceConfigEntry  "ocHnDevInterfaceConfigEntry"
  #define kNAME_ocHnDevInterfaceConfigMaxPowerLevel  "ocHnDevInterfaceConfigMaxPowerLevel"
  #define kNAME_ocHnDevInterfaceConfigPowerUnits  "ocHnDevInterfaceConfigPowerUnits"
  #define kNAME_ocHnDevInterfaceConfigMaxParameterizedBandwidth  "ocHnDevInterfaceConfigMaxParameterizedBandwidth"
  #define kNAME_ocHnDevInterfaceConfigEnableEncryption  "ocHnDevInterfaceConfigEnableEncryption"
  #define kNAME_ocHnDevInterfaceConfigEncryptionPassphrase  "ocHnDevInterfaceConfigEncryptionPassphrase"
#else
  #define kNAME_ocHnDevInterfaceConfigTable  kOID_ocHnDevInterfaceConfigTable
  #define kNAME_ocHnDevInterfaceConfigEntry  kOID_ocHnDevInterfaceConfigEntry
  #define kNAME_ocHnDevInterfaceConfigMaxPowerLevel  kOID_ocHnDevInterfaceConfigMaxPowerLevel
  #define kNAME_ocHnDevInterfaceConfigPowerUnits  kOID_ocHnDevInterfaceConfigPowerUnits
  #define kNAME_ocHnDevInterfaceConfigMaxParameterizedBandwidth  kOID_ocHnDevInterfaceConfigMaxParameterizedBandwidth
  #define kNAME_ocHnDevInterfaceConfigEnableEncryption  kOID_ocHnDevInterfaceConfigEnableEncryption
  #define kNAME_ocHnDevInterfaceConfigEncryptionPassphrase  kOID_ocHnDevInterfaceConfigEncryptionPassphrase
#endif

class ocHnDevInterfaceConfigTable : public BcmMibTable
{
  public:
    ocHnDevInterfaceConfigTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ocHnDevInterfaceConfigEntry : public BcmMibEntry
{
  friend class ocHnDevInterfaceConfigEntryBridge;
  friend class ocHnDevInterfaceConfigTable;

  public:
    ocHnDevInterfaceConfigEntry (ocHnDevInterfaceConfigTable *pTable, ocHnDevInterfaceConfigEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocHnDevInterfaceConfigEntryBridge *fpBridge;

    enum
    {
      kLEAF_ocHnDevInterfaceConfigMaxPowerLevel = 2,
      kLEAF_ocHnDevInterfaceConfigPowerUnits = 3,
      kLEAF_ocHnDevInterfaceConfigMaxParameterizedBandwidth = 4,
      kLEAF_ocHnDevInterfaceConfigEnableEncryption = 5,
      kLEAF_ocHnDevInterfaceConfigEncryptionPassphrase = 6
    };
};


/* ocHnDevInterfaceStatusTable and related objects */

// OID constants
#define kOID_ocHnDevInterfaceStatusTable  "1.3.6.1.4.1.4491.2.3.2.2.4.2"
#define kOID_ocHnDevInterfaceStatusEntry  "1.3.6.1.4.1.4491.2.3.2.2.4.2.1"
#define kOID_ocHnDevInterfaceStatusTxBroadcastRate  "1.3.6.1.4.1.4491.2.3.2.2.4.2.1.1"
#define kOID_ocHnDevInterfaceStatusTxBroadcastLevel  "1.3.6.1.4.1.4491.2.3.2.2.4.2.1.2"
#define kOID_ocHnDevInterfaceStatusMaxTxPowerLevel  "1.3.6.1.4.1.4491.2.3.2.2.4.2.1.3"
#define kOID_ocHnDevInterfaceStatusPowerUnits  "1.3.6.1.4.1.4491.2.3.2.2.4.2.1.4"
#define kOID_ocHnDevInterfaceStatusMaxParameterizedBandwidth  "1.3.6.1.4.1.4491.2.3.2.2.4.2.1.5"
#define kOID_ocHnDevInterfaceStatusLayer2Scheduler  "1.3.6.1.4.1.4491.2.3.2.2.4.2.1.6"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocHnDevInterfaceStatusTable  "ocHnDevInterfaceStatusTable"
  #define kNAME_ocHnDevInterfaceStatusEntry  "ocHnDevInterfaceStatusEntry"
  #define kNAME_ocHnDevInterfaceStatusTxBroadcastRate  "ocHnDevInterfaceStatusTxBroadcastRate"
  #define kNAME_ocHnDevInterfaceStatusTxBroadcastLevel  "ocHnDevInterfaceStatusTxBroadcastLevel"
  #define kNAME_ocHnDevInterfaceStatusMaxTxPowerLevel  "ocHnDevInterfaceStatusMaxTxPowerLevel"
  #define kNAME_ocHnDevInterfaceStatusPowerUnits  "ocHnDevInterfaceStatusPowerUnits"
  #define kNAME_ocHnDevInterfaceStatusMaxParameterizedBandwidth  "ocHnDevInterfaceStatusMaxParameterizedBandwidth"
  #define kNAME_ocHnDevInterfaceStatusLayer2Scheduler  "ocHnDevInterfaceStatusLayer2Scheduler"
#else
  #define kNAME_ocHnDevInterfaceStatusTable  kOID_ocHnDevInterfaceStatusTable
  #define kNAME_ocHnDevInterfaceStatusEntry  kOID_ocHnDevInterfaceStatusEntry
  #define kNAME_ocHnDevInterfaceStatusTxBroadcastRate  kOID_ocHnDevInterfaceStatusTxBroadcastRate
  #define kNAME_ocHnDevInterfaceStatusTxBroadcastLevel  kOID_ocHnDevInterfaceStatusTxBroadcastLevel
  #define kNAME_ocHnDevInterfaceStatusMaxTxPowerLevel  kOID_ocHnDevInterfaceStatusMaxTxPowerLevel
  #define kNAME_ocHnDevInterfaceStatusPowerUnits  kOID_ocHnDevInterfaceStatusPowerUnits
  #define kNAME_ocHnDevInterfaceStatusMaxParameterizedBandwidth  kOID_ocHnDevInterfaceStatusMaxParameterizedBandwidth
  #define kNAME_ocHnDevInterfaceStatusLayer2Scheduler  kOID_ocHnDevInterfaceStatusLayer2Scheduler
#endif

class ocHnDevInterfaceStatusTable : public BcmMibTable
{
  public:
    ocHnDevInterfaceStatusTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ocHnDevInterfaceStatusEntry : public BcmMibEntry
{
  friend class ocHnDevInterfaceStatusEntryBridge;
  friend class ocHnDevInterfaceStatusTable;

  public:
    ocHnDevInterfaceStatusEntry (ocHnDevInterfaceStatusTable *pTable, ocHnDevInterfaceStatusEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocHnDevInterfaceStatusEntryBridge *fpBridge;

    enum
    {
      kLEAF_ocHnDevInterfaceStatusTxBroadcastRate = 1,
      kLEAF_ocHnDevInterfaceStatusTxBroadcastLevel = 2,
      kLEAF_ocHnDevInterfaceStatusMaxTxPowerLevel = 3,
      kLEAF_ocHnDevInterfaceStatusPowerUnits = 4,
      kLEAF_ocHnDevInterfaceStatusMaxParameterizedBandwidth = 5,
      kLEAF_ocHnDevInterfaceStatusLayer2Scheduler = 6
    };
};


/* ocHnDevConnectionTable and related objects */

// OID constants
#define kOID_ocHnDevConnectionTable  "1.3.6.1.4.1.4491.2.3.2.2.4.3"
#define kOID_ocHnDevConnectionEntry  "1.3.6.1.4.1.4491.2.3.2.2.4.3.1"
#define kOID_ocHnDevConnectionIndex  "1.3.6.1.4.1.4491.2.3.2.2.4.3.1.1"
#define kOID_ocHnDevConnectionDestMac  "1.3.6.1.4.1.4491.2.3.2.2.4.3.1.2"
#define kOID_ocHnDevConnectionRxLinkRate  "1.3.6.1.4.1.4491.2.3.2.2.4.3.1.3"
#define kOID_ocHnDevConnectionRxModulationType  "1.3.6.1.4.1.4491.2.3.2.2.4.3.1.4"
#define kOID_ocHnDevConnectionRxBroadcastRate  "1.3.6.1.4.1.4491.2.3.2.2.4.3.1.5"
#define kOID_ocHnDevConnectionRxSignalStrength  "1.3.6.1.4.1.4491.2.3.2.2.4.3.1.6"
#define kOID_ocHnDevConnectionTxUnicastLevel  "1.3.6.1.4.1.4491.2.3.2.2.4.3.1.7"
#define kOID_ocHnDevConnectionPowerUnits  "1.3.6.1.4.1.4491.2.3.2.2.4.3.1.8"
#define kOID_ocHnDevConnectionTxLinkRate  "1.3.6.1.4.1.4491.2.3.2.2.4.3.1.9"
#define kOID_ocHnDevConnectionTxModulationType  "1.3.6.1.4.1.4491.2.3.2.2.4.3.1.10"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocHnDevConnectionTable  "ocHnDevConnectionTable"
  #define kNAME_ocHnDevConnectionEntry  "ocHnDevConnectionEntry"
  #define kNAME_ocHnDevConnectionIndex  "ocHnDevConnectionIndex"
  #define kNAME_ocHnDevConnectionDestMac  "ocHnDevConnectionDestMac"
  #define kNAME_ocHnDevConnectionRxLinkRate  "ocHnDevConnectionRxLinkRate"
  #define kNAME_ocHnDevConnectionRxModulationType  "ocHnDevConnectionRxModulationType"
  #define kNAME_ocHnDevConnectionRxBroadcastRate  "ocHnDevConnectionRxBroadcastRate"
  #define kNAME_ocHnDevConnectionRxSignalStrength  "ocHnDevConnectionRxSignalStrength"
  #define kNAME_ocHnDevConnectionTxUnicastLevel  "ocHnDevConnectionTxUnicastLevel"
  #define kNAME_ocHnDevConnectionPowerUnits  "ocHnDevConnectionPowerUnits"
  #define kNAME_ocHnDevConnectionTxLinkRate  "ocHnDevConnectionTxLinkRate"
  #define kNAME_ocHnDevConnectionTxModulationType  "ocHnDevConnectionTxModulationType"
#else
  #define kNAME_ocHnDevConnectionTable  kOID_ocHnDevConnectionTable
  #define kNAME_ocHnDevConnectionEntry  kOID_ocHnDevConnectionEntry
  #define kNAME_ocHnDevConnectionIndex  kOID_ocHnDevConnectionIndex
  #define kNAME_ocHnDevConnectionDestMac  kOID_ocHnDevConnectionDestMac
  #define kNAME_ocHnDevConnectionRxLinkRate  kOID_ocHnDevConnectionRxLinkRate
  #define kNAME_ocHnDevConnectionRxModulationType  kOID_ocHnDevConnectionRxModulationType
  #define kNAME_ocHnDevConnectionRxBroadcastRate  kOID_ocHnDevConnectionRxBroadcastRate
  #define kNAME_ocHnDevConnectionRxSignalStrength  kOID_ocHnDevConnectionRxSignalStrength
  #define kNAME_ocHnDevConnectionTxUnicastLevel  kOID_ocHnDevConnectionTxUnicastLevel
  #define kNAME_ocHnDevConnectionPowerUnits  kOID_ocHnDevConnectionPowerUnits
  #define kNAME_ocHnDevConnectionTxLinkRate  kOID_ocHnDevConnectionTxLinkRate
  #define kNAME_ocHnDevConnectionTxModulationType  kOID_ocHnDevConnectionTxModulationType
#endif

class ocHnDevConnectionTable : public BcmMibTable
{
  public:
    ocHnDevConnectionTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ocHnDevConnectionEntry : public BcmMibEntry
{
  friend class ocHnDevConnectionEntryBridge;
  friend class ocHnDevConnectionTable;

  public:
    ocHnDevConnectionEntry (ocHnDevConnectionTable *pTable, ocHnDevConnectionEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocHnDevConnectionEntryBridge *fpBridge;

    enum
    {
      kLEAF_ocHnDevConnectionIndex = 1,
      kLEAF_ocHnDevConnectionDestMac = 2,
      kLEAF_ocHnDevConnectionRxLinkRate = 3,
      kLEAF_ocHnDevConnectionRxModulationType = 4,
      kLEAF_ocHnDevConnectionRxBroadcastRate = 5,
      kLEAF_ocHnDevConnectionRxSignalStrength = 6,
      kLEAF_ocHnDevConnectionTxUnicastLevel = 7,
      kLEAF_ocHnDevConnectionPowerUnits = 8,
      kLEAF_ocHnDevConnectionTxLinkRate = 9,
      kLEAF_ocHnDevConnectionTxModulationType = 10
    };
};



#endif


