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
//    Filename: ocStbHostMib.h
//    Creation Date: Monday, June 13, 2011 at 13:53:49
//    Created by Broadcom BFC Mib Compiler BCMIBC.EXE version 3.6.1d
//    from input file OC-STB-HOST-MIB.mib
//
//****************************************************************************

#ifndef OCSTBHOSTMIB_H
#define OCSTBHOSTMIB_H

#include "MibObjs.h"
#include "MibBridge.h"

class ocStbHostHWIdentifiersGroupBridge;
class ocStbHostAVInterfaceEntryBridge;
class ocStbHostSPDIfEntryBridge;
class ocStbHostIEEE1394EntryBridge;
class ocStbHostIEEE1394ConnectedDevicesEntryBridge;
class ocStbHostDVIHDMIEntryBridge;
class ocStbHostDVIHDMIAvailableVideoFormatEntryBridge;
class ocStbHostComponentVideoEntryBridge;
class ocStbHostRFChannelOutEntryBridge;
class ocStbHostInBandTunerEntryBridge;
class ocStbHostProgramStatusEntryBridge;
class ocStbHostMpeg2ContentEntryBridge;
class ocStbHostAnalogVideoEntryBridge;
class ocStbHostQpskObjectsGroupBridge;
class ocStbHostEasCodesGroupBridge;
class ocStbHostSecuritySubSystemGroupBridge;
class ocStbHostDeviceSoftwareBaseGroupBridge;
class ocStbHostFirmwareDownloadStatusGroupBridge;
class ocStbHostSoftwareApplicationInfoEntryBridge;
class ocStbHostPowerGroupBridge;
// class ocStbHostUserSettingsGroupBridge;
// class ocStbHostSystemTempEntryBridge;
class ocStbHostSystemMemoryReportEntryBridge;
class ocStbCardInfoGroupBridge;
class ocStbHostCCAppInfoEntryBridge;
class ocStbHostSnmpProxyInfoGroupBridge;
class ocStbHostInfoGroupBridge;
class ocStbHostRebootInfoGroupBridge;
class ocStbHostMemoryInfoGroupBridge;
class ocStbHostJVMInfoGroupBridge;
// class ocStbHostSystemHomeNetworkEntryBridge;
class ocStbHostCardCpInfoGroupBridge;
class ocStbHostDumpTrapInfoGroupBridge;
class ocStbHostSpecificationsInfoGroupBridge;
class ocStbHostContentErrorSummaryInfoGroupBridge;
class ocStbHostSoftwareApplicationInfoGroupBridge;



/*\
 *  ocStbHostMibModuleGroup group
\*/

// OID constants
#define kOID_ocStbHostMibModuleGroup  "1.3.6.1.4.1.4491.2.3.1"
#define kOID_ocStbHostNotificationsGroup  "1.3.6.1.4.1.4491.2.3.1.0"
#define kOID_ocStbHostMibObjectsGroup  "1.3.6.1.4.1.4491.2.3.1.1"
#define kOID_ocStbHostConformanceGroup  "1.3.6.1.4.1.4491.2.3.1.2"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostMibModuleGroup  "ocStbHostMibModuleGroup"
  #define kNAME_ocStbHostNotificationsGroup  "ocStbHostNotificationsGroup"
  #define kNAME_ocStbHostMibObjectsGroup  "ocStbHostMibObjectsGroup"
  #define kNAME_ocStbHostConformanceGroup  "ocStbHostConformanceGroup"
#else
  #define kNAME_ocStbHostMibModuleGroup  kOID_ocStbHostMibModuleGroup
  #define kNAME_ocStbHostNotificationsGroup  kOID_ocStbHostNotificationsGroup
  #define kNAME_ocStbHostMibObjectsGroup  kOID_ocStbHostMibObjectsGroup
  #define kNAME_ocStbHostConformanceGroup  kOID_ocStbHostConformanceGroup
#endif

class ocStbHostMibModuleGroup : public BcmMibGroup
{
  public:
    ocStbHostMibModuleGroup ();

    bool Populate ();
};



/*\
 *  ocStbHostMibObjectsGroup group
\*/

// OID constants
#define kOID_ocStbHostMibObjectsGroup  "1.3.6.1.4.1.4491.2.3.1.1"
#define kOID_ocStbHostSystemGroup  "1.3.6.1.4.1.4491.2.3.1.1.1"
#define kOID_ocStbHostSecuritySubSystemGroup  "1.3.6.1.4.1.4491.2.3.1.1.2"
#define kOID_ocStbHostSoftwareGroup  "1.3.6.1.4.1.4491.2.3.1.1.3"
#define kOID_ocStbHostStatusGroup  "1.3.6.1.4.1.4491.2.3.1.1.4"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostMibObjectsGroup  "ocStbHostMibObjectsGroup"
  #define kNAME_ocStbHostSystemGroup  "ocStbHostSystemGroup"
  #define kNAME_ocStbHostSecuritySubSystemGroup  "ocStbHostSecuritySubSystemGroup"
  #define kNAME_ocStbHostSoftwareGroup  "ocStbHostSoftwareGroup"
  #define kNAME_ocStbHostStatusGroup  "ocStbHostStatusGroup"
#else
  #define kNAME_ocStbHostMibObjectsGroup  kOID_ocStbHostMibObjectsGroup
  #define kNAME_ocStbHostSystemGroup  kOID_ocStbHostSystemGroup
  #define kNAME_ocStbHostSecuritySubSystemGroup  kOID_ocStbHostSecuritySubSystemGroup
  #define kNAME_ocStbHostSoftwareGroup  kOID_ocStbHostSoftwareGroup
  #define kNAME_ocStbHostStatusGroup  kOID_ocStbHostStatusGroup
#endif

class ocStbHostMibObjectsGroup : public BcmMibGroup
{
  public:
    ocStbHostMibObjectsGroup ();

    bool Populate ();
};



/*\
 *  ocStbHostSystemGroup group
\*/

// OID constants
#define kOID_ocStbHostSystemGroup  "1.3.6.1.4.1.4491.2.3.1.1.1"
#define kOID_ocStbHostHWIdentifiersGroup  "1.3.6.1.4.1.4491.2.3.1.1.1.1"
#define kOID_ocStbHostInterfacesGroup  "1.3.6.1.4.1.4491.2.3.1.1.1.2"
#define kOID_ocStbHostEasObjectsGroup  "1.3.6.1.4.1.4491.2.3.1.1.1.3"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostSystemGroup  "ocStbHostSystemGroup"
  #define kNAME_ocStbHostHWIdentifiersGroup  "ocStbHostHWIdentifiersGroup"
  #define kNAME_ocStbHostInterfacesGroup  "ocStbHostInterfacesGroup"
  #define kNAME_ocStbHostEasObjectsGroup  "ocStbHostEasObjectsGroup"
#else
  #define kNAME_ocStbHostSystemGroup  kOID_ocStbHostSystemGroup
  #define kNAME_ocStbHostHWIdentifiersGroup  kOID_ocStbHostHWIdentifiersGroup
  #define kNAME_ocStbHostInterfacesGroup  kOID_ocStbHostInterfacesGroup
  #define kNAME_ocStbHostEasObjectsGroup  kOID_ocStbHostEasObjectsGroup
#endif

class ocStbHostSystemGroup : public BcmMibGroup
{
  public:
    ocStbHostSystemGroup ();

    bool Populate ();
};



/*\
 *  ocStbHostHWIdentifiersGroup group
\*/

// OID constants
#define kOID_ocStbHostHWIdentifiersGroup  "1.3.6.1.4.1.4491.2.3.1.1.1.1"
#define kOID_ocStbHostSerialNumber  "1.3.6.1.4.1.4491.2.3.1.1.1.1.1.0"
#define kOID_ocStbHostHostID  "1.3.6.1.4.1.4491.2.3.1.1.1.1.2.0"
#define kOID_ocStbHostCapabilities  "1.3.6.1.4.1.4491.2.3.1.1.1.1.3.0"
#define kOID_ocStbHostAvcSupport  "1.3.6.1.4.1.4491.2.3.1.1.1.1.4.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostHWIdentifiersGroup  "ocStbHostHWIdentifiersGroup"
  #define kNAME_ocStbHostSerialNumber  "ocStbHostSerialNumber"
  #define kNAME_ocStbHostHostID  "ocStbHostHostID"
  #define kNAME_ocStbHostCapabilities  "ocStbHostCapabilities"
  #define kNAME_ocStbHostAvcSupport  "ocStbHostAvcSupport"
#else
  #define kNAME_ocStbHostHWIdentifiersGroup  kOID_ocStbHostHWIdentifiersGroup
  #define kNAME_ocStbHostSerialNumber  kOID_ocStbHostSerialNumber
  #define kNAME_ocStbHostHostID  kOID_ocStbHostHostID
  #define kNAME_ocStbHostCapabilities  kOID_ocStbHostCapabilities
  #define kNAME_ocStbHostAvcSupport  kOID_ocStbHostAvcSupport
#endif

class ocStbHostHWIdentifiersGroup : public BcmMibGroup
{
  friend class ocStbHostHWIdentifiersGroupBridge;

  public:
    ocStbHostHWIdentifiersGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostHWIdentifiersGroupBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostSerialNumber = 1,
      kLEAF_ocStbHostHostID = 2,
      kLEAF_ocStbHostCapabilities = 3,
      kLEAF_ocStbHostAvcSupport = 4
    };
};



/*\
 *  ocStbHostInterfacesGroup group
\*/

// OID constants
#define kOID_ocStbHostInterfacesGroup  "1.3.6.1.4.1.4491.2.3.1.1.1.2"
#define kOID_ocStbHostDevInterfaceTypesGroup  "1.3.6.1.4.1.4491.2.3.1.1.1.2.1"
#define kOID_ocStbHostIEEE1394ObjectsGroup  "1.3.6.1.4.1.4491.2.3.1.1.1.2.3"
#define kOID_ocStbHostDigitalVideoOutputGroup  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4"
#define kOID_ocStbHostAnalogOutputGroup  "1.3.6.1.4.1.4491.2.3.1.1.1.2.5"
#define kOID_ocStbHostServiceProgramInfoGroup  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7"
#define kOID_ocStbHostQpskObjectsGroup  "1.3.6.1.4.1.4491.2.3.1.1.1.2.8"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostInterfacesGroup  "ocStbHostInterfacesGroup"
  #define kNAME_ocStbHostDevInterfaceTypesGroup  "ocStbHostDevInterfaceTypesGroup"
  #define kNAME_ocStbHostIEEE1394ObjectsGroup  "ocStbHostIEEE1394ObjectsGroup"
  #define kNAME_ocStbHostDigitalVideoOutputGroup  "ocStbHostDigitalVideoOutputGroup"
  #define kNAME_ocStbHostAnalogOutputGroup  "ocStbHostAnalogOutputGroup"
  #define kNAME_ocStbHostServiceProgramInfoGroup  "ocStbHostServiceProgramInfoGroup"
  #define kNAME_ocStbHostQpskObjectsGroup  "ocStbHostQpskObjectsGroup"
#else
  #define kNAME_ocStbHostInterfacesGroup  kOID_ocStbHostInterfacesGroup
  #define kNAME_ocStbHostDevInterfaceTypesGroup  kOID_ocStbHostDevInterfaceTypesGroup
  #define kNAME_ocStbHostIEEE1394ObjectsGroup  kOID_ocStbHostIEEE1394ObjectsGroup
  #define kNAME_ocStbHostDigitalVideoOutputGroup  kOID_ocStbHostDigitalVideoOutputGroup
  #define kNAME_ocStbHostAnalogOutputGroup  kOID_ocStbHostAnalogOutputGroup
  #define kNAME_ocStbHostServiceProgramInfoGroup  kOID_ocStbHostServiceProgramInfoGroup
  #define kNAME_ocStbHostQpskObjectsGroup  kOID_ocStbHostQpskObjectsGroup
#endif

class ocStbHostInterfacesGroup : public BcmMibGroup
{
  public:
    ocStbHostInterfacesGroup ();

    bool Populate ();
};


/* ocStbHostAVInterfaceTable and related objects */

// OID constants
#define kOID_ocStbHostAVInterfaceTable  "1.3.6.1.4.1.4491.2.3.1.1.1.2.2"
#define kOID_ocStbHostAVInterfaceEntry  "1.3.6.1.4.1.4491.2.3.1.1.1.2.2.1"
#define kOID_ocStbHostAVInterfaceIndex  "1.3.6.1.4.1.4491.2.3.1.1.1.2.2.1.1"
#define kOID_ocStbHostAVInterfaceType  "1.3.6.1.4.1.4491.2.3.1.1.1.2.2.1.2"
#define kOID_ocStbHostAVInterfaceDesc  "1.3.6.1.4.1.4491.2.3.1.1.1.2.2.1.3"
#define kOID_ocStbHostAVInterfaceStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.2.1.4"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostAVInterfaceTable  "ocStbHostAVInterfaceTable"
  #define kNAME_ocStbHostAVInterfaceEntry  "ocStbHostAVInterfaceEntry"
  #define kNAME_ocStbHostAVInterfaceIndex  "ocStbHostAVInterfaceIndex"
  #define kNAME_ocStbHostAVInterfaceType  "ocStbHostAVInterfaceType"
  #define kNAME_ocStbHostAVInterfaceDesc  "ocStbHostAVInterfaceDesc"
  #define kNAME_ocStbHostAVInterfaceStatus  "ocStbHostAVInterfaceStatus"
#else
  #define kNAME_ocStbHostAVInterfaceTable  kOID_ocStbHostAVInterfaceTable
  #define kNAME_ocStbHostAVInterfaceEntry  kOID_ocStbHostAVInterfaceEntry
  #define kNAME_ocStbHostAVInterfaceIndex  kOID_ocStbHostAVInterfaceIndex
  #define kNAME_ocStbHostAVInterfaceType  kOID_ocStbHostAVInterfaceType
  #define kNAME_ocStbHostAVInterfaceDesc  kOID_ocStbHostAVInterfaceDesc
  #define kNAME_ocStbHostAVInterfaceStatus  kOID_ocStbHostAVInterfaceStatus
#endif

class ocStbHostAVInterfaceTable : public BcmMibTable
{
  public:
    ocStbHostAVInterfaceTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ocStbHostAVInterfaceEntry : public BcmMibEntry
{
  friend class ocStbHostAVInterfaceEntryBridge;
  friend class ocStbHostAVInterfaceTable;

  public:
    ocStbHostAVInterfaceEntry (ocStbHostAVInterfaceTable *pTable, ocStbHostAVInterfaceEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostAVInterfaceEntryBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostAVInterfaceIndex = 1,
      kLEAF_ocStbHostAVInterfaceType = 2,
      kLEAF_ocStbHostAVInterfaceDesc = 3,
      kLEAF_ocStbHostAVInterfaceStatus = 4
    };
};


/* ocStbHostSPDIfTable and related objects */

// OID constants
#define kOID_ocStbHostSPDIfTable  "1.3.6.1.4.1.4491.2.3.1.1.1.2.6"
#define kOID_ocStbHostSPDIfEntry  "1.3.6.1.4.1.4491.2.3.1.1.1.2.6.1"
#define kOID_ocStbHostSPDIfAudioFormat  "1.3.6.1.4.1.4491.2.3.1.1.1.2.6.1.1"
#define kOID_ocStbHostSPDIfAudioMuteStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.6.1.2"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostSPDIfTable  "ocStbHostSPDIfTable"
  #define kNAME_ocStbHostSPDIfEntry  "ocStbHostSPDIfEntry"
  #define kNAME_ocStbHostSPDIfAudioFormat  "ocStbHostSPDIfAudioFormat"
  #define kNAME_ocStbHostSPDIfAudioMuteStatus  "ocStbHostSPDIfAudioMuteStatus"
#else
  #define kNAME_ocStbHostSPDIfTable  kOID_ocStbHostSPDIfTable
  #define kNAME_ocStbHostSPDIfEntry  kOID_ocStbHostSPDIfEntry
  #define kNAME_ocStbHostSPDIfAudioFormat  kOID_ocStbHostSPDIfAudioFormat
  #define kNAME_ocStbHostSPDIfAudioMuteStatus  kOID_ocStbHostSPDIfAudioMuteStatus
#endif

class ocStbHostSPDIfTable : public BcmMibTable
{
  public:
    ocStbHostSPDIfTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ocStbHostSPDIfEntry : public BcmMibEntry
{
  friend class ocStbHostSPDIfEntryBridge;
  friend class ocStbHostSPDIfTable;

  public:
    ocStbHostSPDIfEntry (ocStbHostSPDIfTable *pTable, ocStbHostSPDIfEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostSPDIfEntryBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostSPDIfAudioFormat = 1,
      kLEAF_ocStbHostSPDIfAudioMuteStatus = 2
    };
};



/*\
 *  ocStbHostIEEE1394ObjectsGroup group
\*/

// OID constants
#define kOID_ocStbHostIEEE1394ObjectsGroup  "1.3.6.1.4.1.4491.2.3.1.1.1.2.3"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostIEEE1394ObjectsGroup  "ocStbHostIEEE1394ObjectsGroup"
#else
  #define kNAME_ocStbHostIEEE1394ObjectsGroup  kOID_ocStbHostIEEE1394ObjectsGroup
#endif

class ocStbHostIEEE1394ObjectsGroup : public BcmMibGroup
{
  public:
    ocStbHostIEEE1394ObjectsGroup ();

    bool Populate ();
};


/* ocStbHostIEEE1394Table and related objects */

// OID constants
#define kOID_ocStbHostIEEE1394Table  "1.3.6.1.4.1.4491.2.3.1.1.1.2.3.1"
#define kOID_ocStbHostIEEE1394Entry  "1.3.6.1.4.1.4491.2.3.1.1.1.2.3.1.1"
#define kOID_ocStbHostIEEE1394ActiveNodes  "1.3.6.1.4.1.4491.2.3.1.1.1.2.3.1.1.1"
#define kOID_ocStbHostIEEE1394DataXMission  "1.3.6.1.4.1.4491.2.3.1.1.1.2.3.1.1.2"
#define kOID_ocStbHostIEEE1394DTCPStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.3.1.1.3"
#define kOID_ocStbHostIEEE1394LoopStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.3.1.1.4"
#define kOID_ocStbHostIEEE1394RootStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.3.1.1.5"
#define kOID_ocStbHostIEEE1394CycleIsMaster  "1.3.6.1.4.1.4491.2.3.1.1.1.2.3.1.1.6"
#define kOID_ocStbHostIEEE1394IRMStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.3.1.1.7"
#define kOID_ocStbHostIEEE1394AudioMuteStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.3.1.1.8"
#define kOID_ocStbHostIEEE1394VideoMuteStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.3.1.1.9"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostIEEE1394Table  "ocStbHostIEEE1394Table"
  #define kNAME_ocStbHostIEEE1394Entry  "ocStbHostIEEE1394Entry"
  #define kNAME_ocStbHostIEEE1394ActiveNodes  "ocStbHostIEEE1394ActiveNodes"
  #define kNAME_ocStbHostIEEE1394DataXMission  "ocStbHostIEEE1394DataXMission"
  #define kNAME_ocStbHostIEEE1394DTCPStatus  "ocStbHostIEEE1394DTCPStatus"
  #define kNAME_ocStbHostIEEE1394LoopStatus  "ocStbHostIEEE1394LoopStatus"
  #define kNAME_ocStbHostIEEE1394RootStatus  "ocStbHostIEEE1394RootStatus"
  #define kNAME_ocStbHostIEEE1394CycleIsMaster  "ocStbHostIEEE1394CycleIsMaster"
  #define kNAME_ocStbHostIEEE1394IRMStatus  "ocStbHostIEEE1394IRMStatus"
  #define kNAME_ocStbHostIEEE1394AudioMuteStatus  "ocStbHostIEEE1394AudioMuteStatus"
  #define kNAME_ocStbHostIEEE1394VideoMuteStatus  "ocStbHostIEEE1394VideoMuteStatus"
#else
  #define kNAME_ocStbHostIEEE1394Table  kOID_ocStbHostIEEE1394Table
  #define kNAME_ocStbHostIEEE1394Entry  kOID_ocStbHostIEEE1394Entry
  #define kNAME_ocStbHostIEEE1394ActiveNodes  kOID_ocStbHostIEEE1394ActiveNodes
  #define kNAME_ocStbHostIEEE1394DataXMission  kOID_ocStbHostIEEE1394DataXMission
  #define kNAME_ocStbHostIEEE1394DTCPStatus  kOID_ocStbHostIEEE1394DTCPStatus
  #define kNAME_ocStbHostIEEE1394LoopStatus  kOID_ocStbHostIEEE1394LoopStatus
  #define kNAME_ocStbHostIEEE1394RootStatus  kOID_ocStbHostIEEE1394RootStatus
  #define kNAME_ocStbHostIEEE1394CycleIsMaster  kOID_ocStbHostIEEE1394CycleIsMaster
  #define kNAME_ocStbHostIEEE1394IRMStatus  kOID_ocStbHostIEEE1394IRMStatus
  #define kNAME_ocStbHostIEEE1394AudioMuteStatus  kOID_ocStbHostIEEE1394AudioMuteStatus
  #define kNAME_ocStbHostIEEE1394VideoMuteStatus  kOID_ocStbHostIEEE1394VideoMuteStatus
#endif

class ocStbHostIEEE1394Table : public BcmMibTable
{
  public:
    ocStbHostIEEE1394Table ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ocStbHostIEEE1394Entry : public BcmMibEntry
{
  friend class ocStbHostIEEE1394EntryBridge;
  friend class ocStbHostIEEE1394Table;

  public:
    ocStbHostIEEE1394Entry (ocStbHostIEEE1394Table *pTable, ocStbHostIEEE1394EntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostIEEE1394EntryBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostIEEE1394ActiveNodes = 1,
      kLEAF_ocStbHostIEEE1394DataXMission = 2,
      kLEAF_ocStbHostIEEE1394DTCPStatus = 3,
      kLEAF_ocStbHostIEEE1394LoopStatus = 4,
      kLEAF_ocStbHostIEEE1394RootStatus = 5,
      kLEAF_ocStbHostIEEE1394CycleIsMaster = 6,
      kLEAF_ocStbHostIEEE1394IRMStatus = 7,
      kLEAF_ocStbHostIEEE1394AudioMuteStatus = 8,
      kLEAF_ocStbHostIEEE1394VideoMuteStatus = 9
    };
};


/* ocStbHostIEEE1394ConnectedDevicesTable and related objects */

// OID constants
#define kOID_ocStbHostIEEE1394ConnectedDevicesTable  "1.3.6.1.4.1.4491.2.3.1.1.1.2.3.2"
#define kOID_ocStbHostIEEE1394ConnectedDevicesEntry  "1.3.6.1.4.1.4491.2.3.1.1.1.2.3.2.1"
#define kOID_ocStbHostIEEE1394ConnectedDevicesIndex  "1.3.6.1.4.1.4491.2.3.1.1.1.2.3.2.1.1"
#define kOID_ocStbHostIEEE1394ConnectedDevicesAVInterfaceIndex  "1.3.6.1.4.1.4491.2.3.1.1.1.2.3.2.1.2"
#define kOID_ocStbHostIEEE1394ConnectedDevicesSubUnitType  "1.3.6.1.4.1.4491.2.3.1.1.1.2.3.2.1.3"
#define kOID_ocStbHostIEEE1394ConnectedDevicesEui64  "1.3.6.1.4.1.4491.2.3.1.1.1.2.3.2.1.4"
#define kOID_ocStbHostIEEE1394ConnectedDevicesADSourceSelectSupport  "1.3.6.1.4.1.4491.2.3.1.1.1.2.3.2.1.5"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostIEEE1394ConnectedDevicesTable  "ocStbHostIEEE1394ConnectedDevicesTable"
  #define kNAME_ocStbHostIEEE1394ConnectedDevicesEntry  "ocStbHostIEEE1394ConnectedDevicesEntry"
  #define kNAME_ocStbHostIEEE1394ConnectedDevicesIndex  "ocStbHostIEEE1394ConnectedDevicesIndex"
  #define kNAME_ocStbHostIEEE1394ConnectedDevicesAVInterfaceIndex  "ocStbHostIEEE1394ConnectedDevicesAVInterfaceIndex"
  #define kNAME_ocStbHostIEEE1394ConnectedDevicesSubUnitType  "ocStbHostIEEE1394ConnectedDevicesSubUnitType"
  #define kNAME_ocStbHostIEEE1394ConnectedDevicesEui64  "ocStbHostIEEE1394ConnectedDevicesEui64"
  #define kNAME_ocStbHostIEEE1394ConnectedDevicesADSourceSelectSupport  "ocStbHostIEEE1394ConnectedDevicesADSourceSelectSupport"
#else
  #define kNAME_ocStbHostIEEE1394ConnectedDevicesTable  kOID_ocStbHostIEEE1394ConnectedDevicesTable
  #define kNAME_ocStbHostIEEE1394ConnectedDevicesEntry  kOID_ocStbHostIEEE1394ConnectedDevicesEntry
  #define kNAME_ocStbHostIEEE1394ConnectedDevicesIndex  kOID_ocStbHostIEEE1394ConnectedDevicesIndex
  #define kNAME_ocStbHostIEEE1394ConnectedDevicesAVInterfaceIndex  kOID_ocStbHostIEEE1394ConnectedDevicesAVInterfaceIndex
  #define kNAME_ocStbHostIEEE1394ConnectedDevicesSubUnitType  kOID_ocStbHostIEEE1394ConnectedDevicesSubUnitType
  #define kNAME_ocStbHostIEEE1394ConnectedDevicesEui64  kOID_ocStbHostIEEE1394ConnectedDevicesEui64
  #define kNAME_ocStbHostIEEE1394ConnectedDevicesADSourceSelectSupport  kOID_ocStbHostIEEE1394ConnectedDevicesADSourceSelectSupport
#endif

class ocStbHostIEEE1394ConnectedDevicesTable : public BcmMibTable
{
  public:
    ocStbHostIEEE1394ConnectedDevicesTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ocStbHostIEEE1394ConnectedDevicesEntry : public BcmMibEntry
{
  friend class ocStbHostIEEE1394ConnectedDevicesEntryBridge;
  friend class ocStbHostIEEE1394ConnectedDevicesTable;

  public:
    ocStbHostIEEE1394ConnectedDevicesEntry (ocStbHostIEEE1394ConnectedDevicesTable *pTable, ocStbHostIEEE1394ConnectedDevicesEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostIEEE1394ConnectedDevicesEntryBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostIEEE1394ConnectedDevicesIndex = 1,
      kLEAF_ocStbHostIEEE1394ConnectedDevicesAVInterfaceIndex = 2,
      kLEAF_ocStbHostIEEE1394ConnectedDevicesSubUnitType = 3,
      kLEAF_ocStbHostIEEE1394ConnectedDevicesEui64 = 4,
      kLEAF_ocStbHostIEEE1394ConnectedDevicesADSourceSelectSupport = 5
    };
};



/*\
 *  ocStbHostDigitalVideoOutputGroup group
\*/

// OID constants
#define kOID_ocStbHostDigitalVideoOutputGroup  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostDigitalVideoOutputGroup  "ocStbHostDigitalVideoOutputGroup"
#else
  #define kNAME_ocStbHostDigitalVideoOutputGroup  kOID_ocStbHostDigitalVideoOutputGroup
#endif

class ocStbHostDigitalVideoOutputGroup : public BcmMibGroup
{
  public:
    ocStbHostDigitalVideoOutputGroup ();

    bool Populate ();
};


/* ocStbHostDVIHDMITable and related objects */

// OID constants
#define kOID_ocStbHostDVIHDMITable  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1"
#define kOID_ocStbHostDVIHDMIEntry  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1"
#define kOID_ocStbHostDVIHDMIOutputType  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.2"
#define kOID_ocStbHostDVIHDMIConnectionStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.3"
#define kOID_ocStbHostDVIHDMIRepeaterStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.4"
#define kOID_ocStbHostDVIHDMIVideoXmissionStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.5"
#define kOID_ocStbHostDVIHDMIHDCPStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.6"
#define kOID_ocStbHostDVIHDMIVideoMuteStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.7"
#define kOID_ocStbHostDVIHDMIOutputFormat  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.8"
#define kOID_ocStbHostDVIHDMIAspectRatio  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.9"
#define kOID_ocStbHostDVIHDMIHostDeviceHDCPStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.10"
#define kOID_ocStbHostDVIHDMIAudioFormat  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.11"
#define kOID_ocStbHostDVIHDMIAudioSampleRate  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.12"
#define kOID_ocStbHostDVIHDMIAudioChannelCount  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.13"
#define kOID_ocStbHostDVIHDMIAudioMuteStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.14"
#define kOID_ocStbHostDVIHDMIAudioSampleSize  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.15"
#define kOID_ocStbHostDVIHDMIColorSpace  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.16"
#define kOID_ocStbHostDVIHDMIFrameRate  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.17"
#define kOID_ocStbHostDVIHDMIAttachedDeviceType  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.18"
#define kOID_ocStbHostDVIHDMIEdid  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.19"
#define kOID_ocStbHostDVIHDMILipSyncDelay  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.20"
#define kOID_ocStbHostDVIHDMICecFeatures  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.21"
#define kOID_ocStbHostDVIHDMIFeatures  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.22"
#define kOID_ocStbHostDVIHDMIMaxDeviceCount  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.23"
#define kOID_ocStbHostDVIHDMIPreferredVideoFormat  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.24"
#define kOID_ocStbHostDVIHDMIEdidVersion  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.25"
#define kOID_ocStbHostDVIHDMI3DCompatibilityControl  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.26"
#define kOID_ocStbHostDVIHDMI3DCompatibilityMsgDisplay  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.1.1.27"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostDVIHDMITable  "ocStbHostDVIHDMITable"
  #define kNAME_ocStbHostDVIHDMIEntry  "ocStbHostDVIHDMIEntry"
  #define kNAME_ocStbHostDVIHDMIOutputType  "ocStbHostDVIHDMIOutputType"
  #define kNAME_ocStbHostDVIHDMIConnectionStatus  "ocStbHostDVIHDMIConnectionStatus"
  #define kNAME_ocStbHostDVIHDMIRepeaterStatus  "ocStbHostDVIHDMIRepeaterStatus"
  #define kNAME_ocStbHostDVIHDMIVideoXmissionStatus  "ocStbHostDVIHDMIVideoXmissionStatus"
  #define kNAME_ocStbHostDVIHDMIHDCPStatus  "ocStbHostDVIHDMIHDCPStatus"
  #define kNAME_ocStbHostDVIHDMIVideoMuteStatus  "ocStbHostDVIHDMIVideoMuteStatus"
  #define kNAME_ocStbHostDVIHDMIOutputFormat  "ocStbHostDVIHDMIOutputFormat"
  #define kNAME_ocStbHostDVIHDMIAspectRatio  "ocStbHostDVIHDMIAspectRatio"
  #define kNAME_ocStbHostDVIHDMIHostDeviceHDCPStatus  "ocStbHostDVIHDMIHostDeviceHDCPStatus"
  #define kNAME_ocStbHostDVIHDMIAudioFormat  "ocStbHostDVIHDMIAudioFormat"
  #define kNAME_ocStbHostDVIHDMIAudioSampleRate  "ocStbHostDVIHDMIAudioSampleRate"
  #define kNAME_ocStbHostDVIHDMIAudioChannelCount  "ocStbHostDVIHDMIAudioChannelCount"
  #define kNAME_ocStbHostDVIHDMIAudioMuteStatus  "ocStbHostDVIHDMIAudioMuteStatus"
  #define kNAME_ocStbHostDVIHDMIAudioSampleSize  "ocStbHostDVIHDMIAudioSampleSize"
  #define kNAME_ocStbHostDVIHDMIColorSpace  "ocStbHostDVIHDMIColorSpace"
  #define kNAME_ocStbHostDVIHDMIFrameRate  "ocStbHostDVIHDMIFrameRate"
  #define kNAME_ocStbHostDVIHDMIAttachedDeviceType  "ocStbHostDVIHDMIAttachedDeviceType"
  #define kNAME_ocStbHostDVIHDMIEdid  "ocStbHostDVIHDMIEdid"
  #define kNAME_ocStbHostDVIHDMILipSyncDelay  "ocStbHostDVIHDMILipSyncDelay"
  #define kNAME_ocStbHostDVIHDMICecFeatures  "ocStbHostDVIHDMICecFeatures"
  #define kNAME_ocStbHostDVIHDMIFeatures  "ocStbHostDVIHDMIFeatures"
  #define kNAME_ocStbHostDVIHDMIMaxDeviceCount  "ocStbHostDVIHDMIMaxDeviceCount"
  #define kNAME_ocStbHostDVIHDMIPreferredVideoFormat  "ocStbHostDVIHDMIPreferredVideoFormat"
  #define kNAME_ocStbHostDVIHDMIEdidVersion  "ocStbHostDVIHDMIEdidVersion"
  #define kNAME_ocStbHostDVIHDMI3DCompatibilityControl  "ocStbHostDVIHDMI3DCompatibilityControl"
  #define kNAME_ocStbHostDVIHDMI3DCompatibilityMsgDisplay  "ocStbHostDVIHDMI3DCompatibilityMsgDisplay"
#else
  #define kNAME_ocStbHostDVIHDMITable  kOID_ocStbHostDVIHDMITable
  #define kNAME_ocStbHostDVIHDMIEntry  kOID_ocStbHostDVIHDMIEntry
  #define kNAME_ocStbHostDVIHDMIOutputType  kOID_ocStbHostDVIHDMIOutputType
  #define kNAME_ocStbHostDVIHDMIConnectionStatus  kOID_ocStbHostDVIHDMIConnectionStatus
  #define kNAME_ocStbHostDVIHDMIRepeaterStatus  kOID_ocStbHostDVIHDMIRepeaterStatus
  #define kNAME_ocStbHostDVIHDMIVideoXmissionStatus  kOID_ocStbHostDVIHDMIVideoXmissionStatus
  #define kNAME_ocStbHostDVIHDMIHDCPStatus  kOID_ocStbHostDVIHDMIHDCPStatus
  #define kNAME_ocStbHostDVIHDMIVideoMuteStatus  kOID_ocStbHostDVIHDMIVideoMuteStatus
  #define kNAME_ocStbHostDVIHDMIOutputFormat  kOID_ocStbHostDVIHDMIOutputFormat
  #define kNAME_ocStbHostDVIHDMIAspectRatio  kOID_ocStbHostDVIHDMIAspectRatio
  #define kNAME_ocStbHostDVIHDMIHostDeviceHDCPStatus  kOID_ocStbHostDVIHDMIHostDeviceHDCPStatus
  #define kNAME_ocStbHostDVIHDMIAudioFormat  kOID_ocStbHostDVIHDMIAudioFormat
  #define kNAME_ocStbHostDVIHDMIAudioSampleRate  kOID_ocStbHostDVIHDMIAudioSampleRate
  #define kNAME_ocStbHostDVIHDMIAudioChannelCount  kOID_ocStbHostDVIHDMIAudioChannelCount
  #define kNAME_ocStbHostDVIHDMIAudioMuteStatus  kOID_ocStbHostDVIHDMIAudioMuteStatus
  #define kNAME_ocStbHostDVIHDMIAudioSampleSize  kOID_ocStbHostDVIHDMIAudioSampleSize
  #define kNAME_ocStbHostDVIHDMIColorSpace  kOID_ocStbHostDVIHDMIColorSpace
  #define kNAME_ocStbHostDVIHDMIFrameRate  kOID_ocStbHostDVIHDMIFrameRate
  #define kNAME_ocStbHostDVIHDMIAttachedDeviceType  kOID_ocStbHostDVIHDMIAttachedDeviceType
  #define kNAME_ocStbHostDVIHDMIEdid  kOID_ocStbHostDVIHDMIEdid
  #define kNAME_ocStbHostDVIHDMILipSyncDelay  kOID_ocStbHostDVIHDMILipSyncDelay
  #define kNAME_ocStbHostDVIHDMICecFeatures  kOID_ocStbHostDVIHDMICecFeatures
  #define kNAME_ocStbHostDVIHDMIFeatures  kOID_ocStbHostDVIHDMIFeatures
  #define kNAME_ocStbHostDVIHDMIMaxDeviceCount  kOID_ocStbHostDVIHDMIMaxDeviceCount
  #define kNAME_ocStbHostDVIHDMIPreferredVideoFormat  kOID_ocStbHostDVIHDMIPreferredVideoFormat
  #define kNAME_ocStbHostDVIHDMIEdidVersion  kOID_ocStbHostDVIHDMIEdidVersion
  #define kNAME_ocStbHostDVIHDMI3DCompatibilityControl  kOID_ocStbHostDVIHDMI3DCompatibilityControl
  #define kNAME_ocStbHostDVIHDMI3DCompatibilityMsgDisplay  kOID_ocStbHostDVIHDMI3DCompatibilityMsgDisplay
#endif

class ocStbHostDVIHDMITable : public BcmMibTable
{
  public:
    ocStbHostDVIHDMITable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ocStbHostDVIHDMIEntry : public BcmMibEntry
{
  friend class ocStbHostDVIHDMIEntryBridge;
  friend class ocStbHostDVIHDMITable;

  public:
    ocStbHostDVIHDMIEntry (ocStbHostDVIHDMITable *pTable, ocStbHostDVIHDMIEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostDVIHDMIEntryBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostDVIHDMIOutputType = 2,
      kLEAF_ocStbHostDVIHDMIConnectionStatus = 3,
      kLEAF_ocStbHostDVIHDMIRepeaterStatus = 4,
      kLEAF_ocStbHostDVIHDMIVideoXmissionStatus = 5,
      kLEAF_ocStbHostDVIHDMIHDCPStatus = 6,
      kLEAF_ocStbHostDVIHDMIVideoMuteStatus = 7,
      kLEAF_ocStbHostDVIHDMIOutputFormat = 8,
      kLEAF_ocStbHostDVIHDMIAspectRatio = 9,
      kLEAF_ocStbHostDVIHDMIHostDeviceHDCPStatus = 10,
      kLEAF_ocStbHostDVIHDMIAudioFormat = 11,
      kLEAF_ocStbHostDVIHDMIAudioSampleRate = 12,
      kLEAF_ocStbHostDVIHDMIAudioChannelCount = 13,
      kLEAF_ocStbHostDVIHDMIAudioMuteStatus = 14,
      kLEAF_ocStbHostDVIHDMIAudioSampleSize = 15,
      kLEAF_ocStbHostDVIHDMIColorSpace = 16,
      kLEAF_ocStbHostDVIHDMIFrameRate = 17,
      kLEAF_ocStbHostDVIHDMIAttachedDeviceType = 18,
      kLEAF_ocStbHostDVIHDMIEdid = 19,
      kLEAF_ocStbHostDVIHDMILipSyncDelay = 20,
      kLEAF_ocStbHostDVIHDMICecFeatures = 21,
      kLEAF_ocStbHostDVIHDMIFeatures = 22,
      kLEAF_ocStbHostDVIHDMIMaxDeviceCount = 23,
      kLEAF_ocStbHostDVIHDMIPreferredVideoFormat = 24,
      kLEAF_ocStbHostDVIHDMIEdidVersion = 25,
      kLEAF_ocStbHostDVIHDMI3DCompatibilityControl = 26,
      kLEAF_ocStbHostDVIHDMI3DCompatibilityMsgDisplay = 27
    };
};


/* ocStbHostDVIHDMIAvailableVideoFormatTable and related objects */

// OID constants
#define kOID_ocStbHostDVIHDMIAvailableVideoFormatTable  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.2"
#define kOID_ocStbHostDVIHDMIAvailableVideoFormatEntry  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.2.1"
#define kOID_ocStbHostDVIHDMIAvailableVideoFormatIndex  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.2.1.1"
#define kOID_ocStbHostDVIHDMIAvailableVideoFormat  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.2.1.2"
#define kOID_ocStbHostDVIHDMISupported3DStructures  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.2.1.3"
#define kOID_ocStbHostDVIHDMIActive3DStructure  "1.3.6.1.4.1.4491.2.3.1.1.1.2.4.2.1.4"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostDVIHDMIAvailableVideoFormatTable  "ocStbHostDVIHDMIAvailableVideoFormatTable"
  #define kNAME_ocStbHostDVIHDMIAvailableVideoFormatEntry  "ocStbHostDVIHDMIAvailableVideoFormatEntry"
  #define kNAME_ocStbHostDVIHDMIAvailableVideoFormatIndex  "ocStbHostDVIHDMIAvailableVideoFormatIndex"
  #define kNAME_ocStbHostDVIHDMIAvailableVideoFormat  "ocStbHostDVIHDMIAvailableVideoFormat"
  #define kNAME_ocStbHostDVIHDMISupported3DStructures  "ocStbHostDVIHDMISupported3DStructures"
  #define kNAME_ocStbHostDVIHDMIActive3DStructure  "ocStbHostDVIHDMIActive3DStructure"
#else
  #define kNAME_ocStbHostDVIHDMIAvailableVideoFormatTable  kOID_ocStbHostDVIHDMIAvailableVideoFormatTable
  #define kNAME_ocStbHostDVIHDMIAvailableVideoFormatEntry  kOID_ocStbHostDVIHDMIAvailableVideoFormatEntry
  #define kNAME_ocStbHostDVIHDMIAvailableVideoFormatIndex  kOID_ocStbHostDVIHDMIAvailableVideoFormatIndex
  #define kNAME_ocStbHostDVIHDMIAvailableVideoFormat  kOID_ocStbHostDVIHDMIAvailableVideoFormat
  #define kNAME_ocStbHostDVIHDMISupported3DStructures  kOID_ocStbHostDVIHDMISupported3DStructures
  #define kNAME_ocStbHostDVIHDMIActive3DStructure  kOID_ocStbHostDVIHDMIActive3DStructure
#endif

class ocStbHostDVIHDMIAvailableVideoFormatTable : public BcmMibTable
{
  public:
    ocStbHostDVIHDMIAvailableVideoFormatTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ocStbHostDVIHDMIAvailableVideoFormatEntry : public BcmMibEntry
{
  friend class ocStbHostDVIHDMIAvailableVideoFormatEntryBridge;
  friend class ocStbHostDVIHDMIAvailableVideoFormatTable;

  public:
    ocStbHostDVIHDMIAvailableVideoFormatEntry (ocStbHostDVIHDMIAvailableVideoFormatTable *pTable, ocStbHostDVIHDMIAvailableVideoFormatEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostDVIHDMIAvailableVideoFormatEntryBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostDVIHDMIAvailableVideoFormatIndex = 1,
      kLEAF_ocStbHostDVIHDMIAvailableVideoFormat = 2,
      kLEAF_ocStbHostDVIHDMISupported3DStructures = 3,
      kLEAF_ocStbHostDVIHDMIActive3DStructure = 4
    };
};



/*\
 *  ocStbHostAnalogOutputGroup group
\*/

// OID constants
#define kOID_ocStbHostAnalogOutputGroup  "1.3.6.1.4.1.4491.2.3.1.1.1.2.5"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostAnalogOutputGroup  "ocStbHostAnalogOutputGroup"
#else
  #define kNAME_ocStbHostAnalogOutputGroup  kOID_ocStbHostAnalogOutputGroup
#endif

class ocStbHostAnalogOutputGroup : public BcmMibGroup
{
  public:
    ocStbHostAnalogOutputGroup ();

    bool Populate ();
};


/* ocStbHostComponentVideoTable and related objects */

// OID constants
#define kOID_ocStbHostComponentVideoTable  "1.3.6.1.4.1.4491.2.3.1.1.1.2.5.1"
#define kOID_ocStbHostComponentVideoEntry  "1.3.6.1.4.1.4491.2.3.1.1.1.2.5.1.1"
#define kOID_ocStbHostComponentVideoConstrainedStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.5.1.1.1"
#define kOID_ocStbHostComponentOutputFormat  "1.3.6.1.4.1.4491.2.3.1.1.1.2.5.1.1.2"
#define kOID_ocStbHostComponentAspectRatio  "1.3.6.1.4.1.4491.2.3.1.1.1.2.5.1.1.3"
#define kOID_ocStbHostComponentVideoMuteStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.5.1.1.4"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostComponentVideoTable  "ocStbHostComponentVideoTable"
  #define kNAME_ocStbHostComponentVideoEntry  "ocStbHostComponentVideoEntry"
  #define kNAME_ocStbHostComponentVideoConstrainedStatus  "ocStbHostComponentVideoConstrainedStatus"
  #define kNAME_ocStbHostComponentOutputFormat  "ocStbHostComponentOutputFormat"
  #define kNAME_ocStbHostComponentAspectRatio  "ocStbHostComponentAspectRatio"
  #define kNAME_ocStbHostComponentVideoMuteStatus  "ocStbHostComponentVideoMuteStatus"
#else
  #define kNAME_ocStbHostComponentVideoTable  kOID_ocStbHostComponentVideoTable
  #define kNAME_ocStbHostComponentVideoEntry  kOID_ocStbHostComponentVideoEntry
  #define kNAME_ocStbHostComponentVideoConstrainedStatus  kOID_ocStbHostComponentVideoConstrainedStatus
  #define kNAME_ocStbHostComponentOutputFormat  kOID_ocStbHostComponentOutputFormat
  #define kNAME_ocStbHostComponentAspectRatio  kOID_ocStbHostComponentAspectRatio
  #define kNAME_ocStbHostComponentVideoMuteStatus  kOID_ocStbHostComponentVideoMuteStatus
#endif

class ocStbHostComponentVideoTable : public BcmMibTable
{
  public:
    ocStbHostComponentVideoTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ocStbHostComponentVideoEntry : public BcmMibEntry
{
  friend class ocStbHostComponentVideoEntryBridge;
  friend class ocStbHostComponentVideoTable;

  public:
    ocStbHostComponentVideoEntry (ocStbHostComponentVideoTable *pTable, ocStbHostComponentVideoEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostComponentVideoEntryBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostComponentVideoConstrainedStatus = 1,
      kLEAF_ocStbHostComponentOutputFormat = 2,
      kLEAF_ocStbHostComponentAspectRatio = 3,
      kLEAF_ocStbHostComponentVideoMuteStatus = 4
    };
};


/* ocStbHostRFChannelOutTable and related objects */

// OID constants
#define kOID_ocStbHostRFChannelOutTable  "1.3.6.1.4.1.4491.2.3.1.1.1.2.5.2"
#define kOID_ocStbHostRFChannelOutEntry  "1.3.6.1.4.1.4491.2.3.1.1.1.2.5.2.1"
#define kOID_ocStbHostRFChannelOut  "1.3.6.1.4.1.4491.2.3.1.1.1.2.5.2.1.2"
#define kOID_ocStbHostRFChannelOutAudioMuteStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.5.2.1.3"
#define kOID_ocStbHostRFChannelOutVideoMuteStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.5.2.1.4"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostRFChannelOutTable  "ocStbHostRFChannelOutTable"
  #define kNAME_ocStbHostRFChannelOutEntry  "ocStbHostRFChannelOutEntry"
  #define kNAME_ocStbHostRFChannelOut  "ocStbHostRFChannelOut"
  #define kNAME_ocStbHostRFChannelOutAudioMuteStatus  "ocStbHostRFChannelOutAudioMuteStatus"
  #define kNAME_ocStbHostRFChannelOutVideoMuteStatus  "ocStbHostRFChannelOutVideoMuteStatus"
#else
  #define kNAME_ocStbHostRFChannelOutTable  kOID_ocStbHostRFChannelOutTable
  #define kNAME_ocStbHostRFChannelOutEntry  kOID_ocStbHostRFChannelOutEntry
  #define kNAME_ocStbHostRFChannelOut  kOID_ocStbHostRFChannelOut
  #define kNAME_ocStbHostRFChannelOutAudioMuteStatus  kOID_ocStbHostRFChannelOutAudioMuteStatus
  #define kNAME_ocStbHostRFChannelOutVideoMuteStatus  kOID_ocStbHostRFChannelOutVideoMuteStatus
#endif

class ocStbHostRFChannelOutTable : public BcmMibTable
{
  public:
    ocStbHostRFChannelOutTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ocStbHostRFChannelOutEntry : public BcmMibEntry
{
  friend class ocStbHostRFChannelOutEntryBridge;
  friend class ocStbHostRFChannelOutTable;

  public:
    ocStbHostRFChannelOutEntry (ocStbHostRFChannelOutTable *pTable, ocStbHostRFChannelOutEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostRFChannelOutEntryBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostRFChannelOut = 2,
      kLEAF_ocStbHostRFChannelOutAudioMuteStatus = 3,
      kLEAF_ocStbHostRFChannelOutVideoMuteStatus = 4
    };
};



/*\
 *  ocStbHostServiceProgramInfoGroup group
\*/

// OID constants
#define kOID_ocStbHostServiceProgramInfoGroup  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostServiceProgramInfoGroup  "ocStbHostServiceProgramInfoGroup"
#else
  #define kNAME_ocStbHostServiceProgramInfoGroup  kOID_ocStbHostServiceProgramInfoGroup
#endif

class ocStbHostServiceProgramInfoGroup : public BcmMibGroup
{
  public:
    ocStbHostServiceProgramInfoGroup ();

    bool Populate ();
};


/* ocStbHostInBandTunerTable and related objects */

// OID constants
#define kOID_ocStbHostInBandTunerTable  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1"
#define kOID_ocStbHostInBandTunerEntry  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1.1"
#define kOID_ocStbHostInBandTunerModulationMode  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1.1.1"
#define kOID_ocStbHostInBandTunerFrequency  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1.1.2"
#define kOID_ocStbHostInBandTunerInterleaver  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1.1.3"
#define kOID_ocStbHostInBandTunerPower  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1.1.4"
#define kOID_ocStbHostInBandTunerAGCValue  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1.1.5"
#define kOID_ocStbHostInBandTunerSNRValue  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1.1.6"
#define kOID_ocStbHostInBandTunerUnerroreds  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1.1.7"
#define kOID_ocStbHostInBandTunerCorrecteds  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1.1.8"
#define kOID_ocStbHostInBandTunerUncorrectables  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1.1.9"
#define kOID_ocStbHostInBandTunerCarrierLockLost  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1.1.10"
#define kOID_ocStbHostInBandTunerPCRErrors  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1.1.11"
#define kOID_ocStbHostInBandTunerPTSErrors  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1.1.12"
#define kOID_ocStbHostInBandTunerState  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1.1.13"
#define kOID_ocStbHostInBandTunerBER  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1.1.14"
#define kOID_ocStbHostInBandTunerSecsSinceLock  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1.1.15"
#define kOID_ocStbHostInBandTunerEqGain  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1.1.16"
#define kOID_ocStbHostInBandTunerMainTapCoeff  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1.1.17"
#define kOID_ocStbHostInBandTunerTotalTuneCount  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1.1.18"
#define kOID_ocStbHostInBandTunerTuneFailureCount  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1.1.19"
#define kOID_ocStbHostInBandTunerTuneFailFreq  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1.1.20"
#define kOID_ocStbHostInBandTunerBandwidth  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.1.1.21"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostInBandTunerTable  "ocStbHostInBandTunerTable"
  #define kNAME_ocStbHostInBandTunerEntry  "ocStbHostInBandTunerEntry"
  #define kNAME_ocStbHostInBandTunerModulationMode  "ocStbHostInBandTunerModulationMode"
  #define kNAME_ocStbHostInBandTunerFrequency  "ocStbHostInBandTunerFrequency"
  #define kNAME_ocStbHostInBandTunerInterleaver  "ocStbHostInBandTunerInterleaver"
  #define kNAME_ocStbHostInBandTunerPower  "ocStbHostInBandTunerPower"
  #define kNAME_ocStbHostInBandTunerAGCValue  "ocStbHostInBandTunerAGCValue"
  #define kNAME_ocStbHostInBandTunerSNRValue  "ocStbHostInBandTunerSNRValue"
  #define kNAME_ocStbHostInBandTunerUnerroreds  "ocStbHostInBandTunerUnerroreds"
  #define kNAME_ocStbHostInBandTunerCorrecteds  "ocStbHostInBandTunerCorrecteds"
  #define kNAME_ocStbHostInBandTunerUncorrectables  "ocStbHostInBandTunerUncorrectables"
  #define kNAME_ocStbHostInBandTunerCarrierLockLost  "ocStbHostInBandTunerCarrierLockLost"
  #define kNAME_ocStbHostInBandTunerPCRErrors  "ocStbHostInBandTunerPCRErrors"
  #define kNAME_ocStbHostInBandTunerPTSErrors  "ocStbHostInBandTunerPTSErrors"
  #define kNAME_ocStbHostInBandTunerState  "ocStbHostInBandTunerState"
  #define kNAME_ocStbHostInBandTunerBER  "ocStbHostInBandTunerBER"
  #define kNAME_ocStbHostInBandTunerSecsSinceLock  "ocStbHostInBandTunerSecsSinceLock"
  #define kNAME_ocStbHostInBandTunerEqGain  "ocStbHostInBandTunerEqGain"
  #define kNAME_ocStbHostInBandTunerMainTapCoeff  "ocStbHostInBandTunerMainTapCoeff"
  #define kNAME_ocStbHostInBandTunerTotalTuneCount  "ocStbHostInBandTunerTotalTuneCount"
  #define kNAME_ocStbHostInBandTunerTuneFailureCount  "ocStbHostInBandTunerTuneFailureCount"
  #define kNAME_ocStbHostInBandTunerTuneFailFreq  "ocStbHostInBandTunerTuneFailFreq"
  #define kNAME_ocStbHostInBandTunerBandwidth  "ocStbHostInBandTunerBandwidth"
#else
  #define kNAME_ocStbHostInBandTunerTable  kOID_ocStbHostInBandTunerTable
  #define kNAME_ocStbHostInBandTunerEntry  kOID_ocStbHostInBandTunerEntry
  #define kNAME_ocStbHostInBandTunerModulationMode  kOID_ocStbHostInBandTunerModulationMode
  #define kNAME_ocStbHostInBandTunerFrequency  kOID_ocStbHostInBandTunerFrequency
  #define kNAME_ocStbHostInBandTunerInterleaver  kOID_ocStbHostInBandTunerInterleaver
  #define kNAME_ocStbHostInBandTunerPower  kOID_ocStbHostInBandTunerPower
  #define kNAME_ocStbHostInBandTunerAGCValue  kOID_ocStbHostInBandTunerAGCValue
  #define kNAME_ocStbHostInBandTunerSNRValue  kOID_ocStbHostInBandTunerSNRValue
  #define kNAME_ocStbHostInBandTunerUnerroreds  kOID_ocStbHostInBandTunerUnerroreds
  #define kNAME_ocStbHostInBandTunerCorrecteds  kOID_ocStbHostInBandTunerCorrecteds
  #define kNAME_ocStbHostInBandTunerUncorrectables  kOID_ocStbHostInBandTunerUncorrectables
  #define kNAME_ocStbHostInBandTunerCarrierLockLost  kOID_ocStbHostInBandTunerCarrierLockLost
  #define kNAME_ocStbHostInBandTunerPCRErrors  kOID_ocStbHostInBandTunerPCRErrors
  #define kNAME_ocStbHostInBandTunerPTSErrors  kOID_ocStbHostInBandTunerPTSErrors
  #define kNAME_ocStbHostInBandTunerState  kOID_ocStbHostInBandTunerState
  #define kNAME_ocStbHostInBandTunerBER  kOID_ocStbHostInBandTunerBER
  #define kNAME_ocStbHostInBandTunerSecsSinceLock  kOID_ocStbHostInBandTunerSecsSinceLock
  #define kNAME_ocStbHostInBandTunerEqGain  kOID_ocStbHostInBandTunerEqGain
  #define kNAME_ocStbHostInBandTunerMainTapCoeff  kOID_ocStbHostInBandTunerMainTapCoeff
  #define kNAME_ocStbHostInBandTunerTotalTuneCount  kOID_ocStbHostInBandTunerTotalTuneCount
  #define kNAME_ocStbHostInBandTunerTuneFailureCount  kOID_ocStbHostInBandTunerTuneFailureCount
  #define kNAME_ocStbHostInBandTunerTuneFailFreq  kOID_ocStbHostInBandTunerTuneFailFreq
  #define kNAME_ocStbHostInBandTunerBandwidth  kOID_ocStbHostInBandTunerBandwidth
#endif

class ocStbHostInBandTunerTable : public BcmMibTable
{
  public:
    ocStbHostInBandTunerTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ocStbHostInBandTunerEntry : public BcmMibEntry
{
  friend class ocStbHostInBandTunerEntryBridge;
  friend class ocStbHostInBandTunerTable;

  public:
    ocStbHostInBandTunerEntry (ocStbHostInBandTunerTable *pTable, ocStbHostInBandTunerEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostInBandTunerEntryBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostInBandTunerModulationMode = 1,
      kLEAF_ocStbHostInBandTunerFrequency = 2,
      kLEAF_ocStbHostInBandTunerInterleaver = 3,
      kLEAF_ocStbHostInBandTunerPower = 4,
      kLEAF_ocStbHostInBandTunerAGCValue = 5,
      kLEAF_ocStbHostInBandTunerSNRValue = 6,
      kLEAF_ocStbHostInBandTunerUnerroreds = 7,
      kLEAF_ocStbHostInBandTunerCorrecteds = 8,
      kLEAF_ocStbHostInBandTunerUncorrectables = 9,
      kLEAF_ocStbHostInBandTunerCarrierLockLost = 10,
      kLEAF_ocStbHostInBandTunerPCRErrors = 11,
      kLEAF_ocStbHostInBandTunerPTSErrors = 12,
      kLEAF_ocStbHostInBandTunerState = 13,
      kLEAF_ocStbHostInBandTunerBER = 14,
      kLEAF_ocStbHostInBandTunerSecsSinceLock = 15,
      kLEAF_ocStbHostInBandTunerEqGain = 16,
      kLEAF_ocStbHostInBandTunerMainTapCoeff = 17,
      kLEAF_ocStbHostInBandTunerTotalTuneCount = 18,
      kLEAF_ocStbHostInBandTunerTuneFailureCount = 19,
      kLEAF_ocStbHostInBandTunerTuneFailFreq = 20,
      kLEAF_ocStbHostInBandTunerBandwidth = 21
    };
};


/* ocStbHostProgramStatusTable and related objects */

// OID constants
#define kOID_ocStbHostProgramStatusTable  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.2"
#define kOID_ocStbHostProgramStatusEntry  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.2.1"
#define kOID_ocStbHostProgramIndex  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.2.1.1"
#define kOID_ocStbHostProgramAVSource  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.2.1.2"
#define kOID_ocStbHostProgramAVDestination  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.2.1.3"
#define kOID_ocStbHostProgramContentSource  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.2.1.4"
#define kOID_ocStbHostProgramContentDestination  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.2.1.5"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostProgramStatusTable  "ocStbHostProgramStatusTable"
  #define kNAME_ocStbHostProgramStatusEntry  "ocStbHostProgramStatusEntry"
  #define kNAME_ocStbHostProgramIndex  "ocStbHostProgramIndex"
  #define kNAME_ocStbHostProgramAVSource  "ocStbHostProgramAVSource"
  #define kNAME_ocStbHostProgramAVDestination  "ocStbHostProgramAVDestination"
  #define kNAME_ocStbHostProgramContentSource  "ocStbHostProgramContentSource"
  #define kNAME_ocStbHostProgramContentDestination  "ocStbHostProgramContentDestination"
#else
  #define kNAME_ocStbHostProgramStatusTable  kOID_ocStbHostProgramStatusTable
  #define kNAME_ocStbHostProgramStatusEntry  kOID_ocStbHostProgramStatusEntry
  #define kNAME_ocStbHostProgramIndex  kOID_ocStbHostProgramIndex
  #define kNAME_ocStbHostProgramAVSource  kOID_ocStbHostProgramAVSource
  #define kNAME_ocStbHostProgramAVDestination  kOID_ocStbHostProgramAVDestination
  #define kNAME_ocStbHostProgramContentSource  kOID_ocStbHostProgramContentSource
  #define kNAME_ocStbHostProgramContentDestination  kOID_ocStbHostProgramContentDestination
#endif

class ocStbHostProgramStatusTable : public BcmMibTable
{
  public:
    ocStbHostProgramStatusTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ocStbHostProgramStatusEntry : public BcmMibEntry
{
  friend class ocStbHostProgramStatusEntryBridge;
  friend class ocStbHostProgramStatusTable;

  public:
    ocStbHostProgramStatusEntry (ocStbHostProgramStatusTable *pTable, ocStbHostProgramStatusEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostProgramStatusEntryBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostProgramIndex = 1,
      kLEAF_ocStbHostProgramAVSource = 2,
      kLEAF_ocStbHostProgramAVDestination = 3,
      kLEAF_ocStbHostProgramContentSource = 4,
      kLEAF_ocStbHostProgramContentDestination = 5
    };
};


/* ocStbHostMpeg2ContentTable and related objects */

// OID constants
#define kOID_ocStbHostMpeg2ContentTable  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.3"
#define kOID_ocStbHostMpeg2ContentEntry  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.3.1"
#define kOID_ocStbHostMpeg2ContentIndex  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.3.1.1"
#define kOID_ocStbHostMpeg2ContentProgramNumber  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.3.1.2"
#define kOID_ocStbHostMpeg2ContentTransportStreamID  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.3.1.3"
#define kOID_ocStbHostMpeg2ContentTotalStreams  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.3.1.4"
#define kOID_ocStbHostMpeg2ContentSelectedVideoPID  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.3.1.5"
#define kOID_ocStbHostMpeg2ContentSelectedAudioPID  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.3.1.6"
#define kOID_ocStbHostMpeg2ContentOtherAudioPIDs  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.3.1.7"
#define kOID_ocStbHostMpeg2ContentCCIValue  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.3.1.8"
#define kOID_ocStbHostMpeg2ContentAPSValue  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.3.1.9"
#define kOID_ocStbHostMpeg2ContentCITStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.3.1.10"
#define kOID_ocStbHostMpeg2ContentBroadcastFlagStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.3.1.11"
#define kOID_ocStbHostMpeg2ContentEPNStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.3.1.12"
#define kOID_ocStbHostMpeg2ContentPCRPID  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.3.1.13"
#define kOID_ocStbHostMpeg2ContentPCRLockStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.3.1.14"
#define kOID_ocStbHostMpeg2ContentDecoderPTS  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.3.1.15"
#define kOID_ocStbHostMpeg2ContentDiscontinuities  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.3.1.16"
#define kOID_ocStbHostMpeg2ContentPktErrors  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.3.1.17"
#define kOID_ocStbHostMpeg2ContentPipelineErrors  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.3.1.18"
#define kOID_ocStbHostMpeg2ContentDecoderRestarts  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.3.1.19"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostMpeg2ContentTable  "ocStbHostMpeg2ContentTable"
  #define kNAME_ocStbHostMpeg2ContentEntry  "ocStbHostMpeg2ContentEntry"
  #define kNAME_ocStbHostMpeg2ContentIndex  "ocStbHostMpeg2ContentIndex"
  #define kNAME_ocStbHostMpeg2ContentProgramNumber  "ocStbHostMpeg2ContentProgramNumber"
  #define kNAME_ocStbHostMpeg2ContentTransportStreamID  "ocStbHostMpeg2ContentTransportStreamID"
  #define kNAME_ocStbHostMpeg2ContentTotalStreams  "ocStbHostMpeg2ContentTotalStreams"
  #define kNAME_ocStbHostMpeg2ContentSelectedVideoPID  "ocStbHostMpeg2ContentSelectedVideoPID"
  #define kNAME_ocStbHostMpeg2ContentSelectedAudioPID  "ocStbHostMpeg2ContentSelectedAudioPID"
  #define kNAME_ocStbHostMpeg2ContentOtherAudioPIDs  "ocStbHostMpeg2ContentOtherAudioPIDs"
  #define kNAME_ocStbHostMpeg2ContentCCIValue  "ocStbHostMpeg2ContentCCIValue"
  #define kNAME_ocStbHostMpeg2ContentAPSValue  "ocStbHostMpeg2ContentAPSValue"
  #define kNAME_ocStbHostMpeg2ContentCITStatus  "ocStbHostMpeg2ContentCITStatus"
  #define kNAME_ocStbHostMpeg2ContentBroadcastFlagStatus  "ocStbHostMpeg2ContentBroadcastFlagStatus"
  #define kNAME_ocStbHostMpeg2ContentEPNStatus  "ocStbHostMpeg2ContentEPNStatus"
  #define kNAME_ocStbHostMpeg2ContentPCRPID  "ocStbHostMpeg2ContentPCRPID"
  #define kNAME_ocStbHostMpeg2ContentPCRLockStatus  "ocStbHostMpeg2ContentPCRLockStatus"
  #define kNAME_ocStbHostMpeg2ContentDecoderPTS  "ocStbHostMpeg2ContentDecoderPTS"
  #define kNAME_ocStbHostMpeg2ContentDiscontinuities  "ocStbHostMpeg2ContentDiscontinuities"
  #define kNAME_ocStbHostMpeg2ContentPktErrors  "ocStbHostMpeg2ContentPktErrors"
  #define kNAME_ocStbHostMpeg2ContentPipelineErrors  "ocStbHostMpeg2ContentPipelineErrors"
  #define kNAME_ocStbHostMpeg2ContentDecoderRestarts  "ocStbHostMpeg2ContentDecoderRestarts"
#else
  #define kNAME_ocStbHostMpeg2ContentTable  kOID_ocStbHostMpeg2ContentTable
  #define kNAME_ocStbHostMpeg2ContentEntry  kOID_ocStbHostMpeg2ContentEntry
  #define kNAME_ocStbHostMpeg2ContentIndex  kOID_ocStbHostMpeg2ContentIndex
  #define kNAME_ocStbHostMpeg2ContentProgramNumber  kOID_ocStbHostMpeg2ContentProgramNumber
  #define kNAME_ocStbHostMpeg2ContentTransportStreamID  kOID_ocStbHostMpeg2ContentTransportStreamID
  #define kNAME_ocStbHostMpeg2ContentTotalStreams  kOID_ocStbHostMpeg2ContentTotalStreams
  #define kNAME_ocStbHostMpeg2ContentSelectedVideoPID  kOID_ocStbHostMpeg2ContentSelectedVideoPID
  #define kNAME_ocStbHostMpeg2ContentSelectedAudioPID  kOID_ocStbHostMpeg2ContentSelectedAudioPID
  #define kNAME_ocStbHostMpeg2ContentOtherAudioPIDs  kOID_ocStbHostMpeg2ContentOtherAudioPIDs
  #define kNAME_ocStbHostMpeg2ContentCCIValue  kOID_ocStbHostMpeg2ContentCCIValue
  #define kNAME_ocStbHostMpeg2ContentAPSValue  kOID_ocStbHostMpeg2ContentAPSValue
  #define kNAME_ocStbHostMpeg2ContentCITStatus  kOID_ocStbHostMpeg2ContentCITStatus
  #define kNAME_ocStbHostMpeg2ContentBroadcastFlagStatus  kOID_ocStbHostMpeg2ContentBroadcastFlagStatus
  #define kNAME_ocStbHostMpeg2ContentEPNStatus  kOID_ocStbHostMpeg2ContentEPNStatus
  #define kNAME_ocStbHostMpeg2ContentPCRPID  kOID_ocStbHostMpeg2ContentPCRPID
  #define kNAME_ocStbHostMpeg2ContentPCRLockStatus  kOID_ocStbHostMpeg2ContentPCRLockStatus
  #define kNAME_ocStbHostMpeg2ContentDecoderPTS  kOID_ocStbHostMpeg2ContentDecoderPTS
  #define kNAME_ocStbHostMpeg2ContentDiscontinuities  kOID_ocStbHostMpeg2ContentDiscontinuities
  #define kNAME_ocStbHostMpeg2ContentPktErrors  kOID_ocStbHostMpeg2ContentPktErrors
  #define kNAME_ocStbHostMpeg2ContentPipelineErrors  kOID_ocStbHostMpeg2ContentPipelineErrors
  #define kNAME_ocStbHostMpeg2ContentDecoderRestarts  kOID_ocStbHostMpeg2ContentDecoderRestarts
#endif

class ocStbHostMpeg2ContentTable : public BcmMibTable
{
  public:
    ocStbHostMpeg2ContentTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ocStbHostMpeg2ContentEntry : public BcmMibEntry
{
  friend class ocStbHostMpeg2ContentEntryBridge;
  friend class ocStbHostMpeg2ContentTable;

  public:
    ocStbHostMpeg2ContentEntry (ocStbHostMpeg2ContentTable *pTable, ocStbHostMpeg2ContentEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostMpeg2ContentEntryBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostMpeg2ContentIndex = 1,
      kLEAF_ocStbHostMpeg2ContentProgramNumber = 2,
      kLEAF_ocStbHostMpeg2ContentTransportStreamID = 3,
      kLEAF_ocStbHostMpeg2ContentTotalStreams = 4,
      kLEAF_ocStbHostMpeg2ContentSelectedVideoPID = 5,
      kLEAF_ocStbHostMpeg2ContentSelectedAudioPID = 6,
      kLEAF_ocStbHostMpeg2ContentOtherAudioPIDs = 7,
      kLEAF_ocStbHostMpeg2ContentCCIValue = 8,
      kLEAF_ocStbHostMpeg2ContentAPSValue = 9,
      kLEAF_ocStbHostMpeg2ContentCITStatus = 10,
      kLEAF_ocStbHostMpeg2ContentBroadcastFlagStatus = 11,
      kLEAF_ocStbHostMpeg2ContentEPNStatus = 12,
      kLEAF_ocStbHostMpeg2ContentPCRPID = 13,
      kLEAF_ocStbHostMpeg2ContentPCRLockStatus = 14,
      kLEAF_ocStbHostMpeg2ContentDecoderPTS = 15,
      kLEAF_ocStbHostMpeg2ContentDiscontinuities = 16,
      kLEAF_ocStbHostMpeg2ContentPktErrors = 17,
      kLEAF_ocStbHostMpeg2ContentPipelineErrors = 18,
      kLEAF_ocStbHostMpeg2ContentDecoderRestarts = 19
    };
};


/* ocStbHostAnalogVideoTable and related objects */

// OID constants
#define kOID_ocStbHostAnalogVideoTable  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.4"
#define kOID_ocStbHostAnalogVideoEntry  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.4.1"
#define kOID_ocStbHostAnalogVideoProtectionStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.7.4.1.1"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostAnalogVideoTable  "ocStbHostAnalogVideoTable"
  #define kNAME_ocStbHostAnalogVideoEntry  "ocStbHostAnalogVideoEntry"
  #define kNAME_ocStbHostAnalogVideoProtectionStatus  "ocStbHostAnalogVideoProtectionStatus"
#else
  #define kNAME_ocStbHostAnalogVideoTable  kOID_ocStbHostAnalogVideoTable
  #define kNAME_ocStbHostAnalogVideoEntry  kOID_ocStbHostAnalogVideoEntry
  #define kNAME_ocStbHostAnalogVideoProtectionStatus  kOID_ocStbHostAnalogVideoProtectionStatus
#endif

class ocStbHostAnalogVideoTable : public BcmMibTable
{
  public:
    ocStbHostAnalogVideoTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ocStbHostAnalogVideoEntry : public BcmMibEntry
{
  friend class ocStbHostAnalogVideoEntryBridge;
  friend class ocStbHostAnalogVideoTable;

  public:
    ocStbHostAnalogVideoEntry (ocStbHostAnalogVideoTable *pTable, ocStbHostAnalogVideoEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostAnalogVideoEntryBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostAnalogVideoProtectionStatus = 1
    };
};



/*\
 *  ocStbHostQpskObjectsGroup group
\*/

// OID constants
#define kOID_ocStbHostQpskObjectsGroup  "1.3.6.1.4.1.4491.2.3.1.1.1.2.8"
#define kOID_ocStbHostQpskFDCFreq  "1.3.6.1.4.1.4491.2.3.1.1.1.2.8.1.0"
#define kOID_ocStbHostQpskRDCFreq  "1.3.6.1.4.1.4491.2.3.1.1.1.2.8.2.0"
#define kOID_ocStbHostQpskFDCBer  "1.3.6.1.4.1.4491.2.3.1.1.1.2.8.3.0"
#define kOID_ocStbHostQpskFDCStatus  "1.3.6.1.4.1.4491.2.3.1.1.1.2.8.4.0"
#define kOID_ocStbHostQpskFDCBytesRead  "1.3.6.1.4.1.4491.2.3.1.1.1.2.8.5.0"
#define kOID_ocStbHostQpskFDCPower  "1.3.6.1.4.1.4491.2.3.1.1.1.2.8.6.0"
#define kOID_ocStbHostQpskFDCLockedTime  "1.3.6.1.4.1.4491.2.3.1.1.1.2.8.7.0"
#define kOID_ocStbHostQpskFDCSNR  "1.3.6.1.4.1.4491.2.3.1.1.1.2.8.8.0"
#define kOID_ocStbHostQpskAGC  "1.3.6.1.4.1.4491.2.3.1.1.1.2.8.9.0"
#define kOID_ocStbHostQpskRDCPower  "1.3.6.1.4.1.4491.2.3.1.1.1.2.8.10.0"
#define kOID_ocStbHostQpskRDCDataRate  "1.3.6.1.4.1.4491.2.3.1.1.1.2.8.11.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostQpskObjectsGroup  "ocStbHostQpskObjectsGroup"
  #define kNAME_ocStbHostQpskFDCFreq  "ocStbHostQpskFDCFreq"
  #define kNAME_ocStbHostQpskRDCFreq  "ocStbHostQpskRDCFreq"
  #define kNAME_ocStbHostQpskFDCBer  "ocStbHostQpskFDCBer"
  #define kNAME_ocStbHostQpskFDCStatus  "ocStbHostQpskFDCStatus"
  #define kNAME_ocStbHostQpskFDCBytesRead  "ocStbHostQpskFDCBytesRead"
  #define kNAME_ocStbHostQpskFDCPower  "ocStbHostQpskFDCPower"
  #define kNAME_ocStbHostQpskFDCLockedTime  "ocStbHostQpskFDCLockedTime"
  #define kNAME_ocStbHostQpskFDCSNR  "ocStbHostQpskFDCSNR"
  #define kNAME_ocStbHostQpskAGC  "ocStbHostQpskAGC"
  #define kNAME_ocStbHostQpskRDCPower  "ocStbHostQpskRDCPower"
  #define kNAME_ocStbHostQpskRDCDataRate  "ocStbHostQpskRDCDataRate"
#else
  #define kNAME_ocStbHostQpskObjectsGroup  kOID_ocStbHostQpskObjectsGroup
  #define kNAME_ocStbHostQpskFDCFreq  kOID_ocStbHostQpskFDCFreq
  #define kNAME_ocStbHostQpskRDCFreq  kOID_ocStbHostQpskRDCFreq
  #define kNAME_ocStbHostQpskFDCBer  kOID_ocStbHostQpskFDCBer
  #define kNAME_ocStbHostQpskFDCStatus  kOID_ocStbHostQpskFDCStatus
  #define kNAME_ocStbHostQpskFDCBytesRead  kOID_ocStbHostQpskFDCBytesRead
  #define kNAME_ocStbHostQpskFDCPower  kOID_ocStbHostQpskFDCPower
  #define kNAME_ocStbHostQpskFDCLockedTime  kOID_ocStbHostQpskFDCLockedTime
  #define kNAME_ocStbHostQpskFDCSNR  kOID_ocStbHostQpskFDCSNR
  #define kNAME_ocStbHostQpskAGC  kOID_ocStbHostQpskAGC
  #define kNAME_ocStbHostQpskRDCPower  kOID_ocStbHostQpskRDCPower
  #define kNAME_ocStbHostQpskRDCDataRate  kOID_ocStbHostQpskRDCDataRate
#endif

class ocStbHostQpskObjectsGroup : public BcmMibGroup
{
  friend class ocStbHostQpskObjectsGroupBridge;

  public:
    ocStbHostQpskObjectsGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostQpskObjectsGroupBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostQpskFDCFreq = 1,
      kLEAF_ocStbHostQpskRDCFreq = 2,
      kLEAF_ocStbHostQpskFDCBer = 3,
      kLEAF_ocStbHostQpskFDCStatus = 4,
      kLEAF_ocStbHostQpskFDCBytesRead = 5,
      kLEAF_ocStbHostQpskFDCPower = 6,
      kLEAF_ocStbHostQpskFDCLockedTime = 7,
      kLEAF_ocStbHostQpskFDCSNR = 8,
      kLEAF_ocStbHostQpskAGC = 9,
      kLEAF_ocStbHostQpskRDCPower = 10,
      kLEAF_ocStbHostQpskRDCDataRate = 11
    };
};



/*\
 *  ocStbHostEasObjectsGroup group
\*/

// OID constants
#define kOID_ocStbHostEasObjectsGroup  "1.3.6.1.4.1.4491.2.3.1.1.1.3"
#define kOID_ocStbHostEasCodesGroup  "1.3.6.1.4.1.4491.2.3.1.1.1.3.1"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostEasObjectsGroup  "ocStbHostEasObjectsGroup"
  #define kNAME_ocStbHostEasCodesGroup  "ocStbHostEasCodesGroup"
#else
  #define kNAME_ocStbHostEasObjectsGroup  kOID_ocStbHostEasObjectsGroup
  #define kNAME_ocStbHostEasCodesGroup  kOID_ocStbHostEasCodesGroup
#endif

class ocStbHostEasObjectsGroup : public BcmMibGroup
{
  public:
    ocStbHostEasObjectsGroup ();

    bool Populate ();
};



/*\
 *  ocStbHostEasCodesGroup group
\*/

// OID constants
#define kOID_ocStbHostEasCodesGroup  "1.3.6.1.4.1.4491.2.3.1.1.1.3.1"
#define kOID_ocStbEasMessageStateCode  "1.3.6.1.4.1.4491.2.3.1.1.1.3.1.1.0"
#define kOID_ocStbEasMessageCountyCode  "1.3.6.1.4.1.4491.2.3.1.1.1.3.1.2.0"
#define kOID_ocStbEasMessageCountySubdivisionCode  "1.3.6.1.4.1.4491.2.3.1.1.1.3.1.3.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostEasCodesGroup  "ocStbHostEasCodesGroup"
  #define kNAME_ocStbEasMessageStateCode  "ocStbEasMessageStateCode"
  #define kNAME_ocStbEasMessageCountyCode  "ocStbEasMessageCountyCode"
  #define kNAME_ocStbEasMessageCountySubdivisionCode  "ocStbEasMessageCountySubdivisionCode"
#else
  #define kNAME_ocStbHostEasCodesGroup  kOID_ocStbHostEasCodesGroup
  #define kNAME_ocStbEasMessageStateCode  kOID_ocStbEasMessageStateCode
  #define kNAME_ocStbEasMessageCountyCode  kOID_ocStbEasMessageCountyCode
  #define kNAME_ocStbEasMessageCountySubdivisionCode  kOID_ocStbEasMessageCountySubdivisionCode
#endif

class ocStbHostEasCodesGroup : public BcmMibGroup
{
  friend class ocStbHostEasCodesGroupBridge;

  public:
    ocStbHostEasCodesGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostEasCodesGroupBridge *fpBridge;

    enum
    {
      kLEAF_ocStbEasMessageStateCode = 1,
      kLEAF_ocStbEasMessageCountyCode = 2,
      kLEAF_ocStbEasMessageCountySubdivisionCode = 3
    };
};



/*\
 *  ocStbHostSecuritySubSystemGroup group
\*/

// OID constants
#define kOID_ocStbHostSecuritySubSystemGroup  "1.3.6.1.4.1.4491.2.3.1.1.2"
#define kOID_ocStbHostCASystemIdentifier  "1.3.6.1.4.1.4491.2.3.1.1.2.3.0"
#define kOID_ocStbHostCAType  "1.3.6.1.4.1.4491.2.3.1.1.2.4.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostSecuritySubSystemGroup  "ocStbHostSecuritySubSystemGroup"
  #define kNAME_ocStbHostCASystemIdentifier  "ocStbHostCASystemIdentifier"
  #define kNAME_ocStbHostCAType  "ocStbHostCAType"
#else
  #define kNAME_ocStbHostSecuritySubSystemGroup  kOID_ocStbHostSecuritySubSystemGroup
  #define kNAME_ocStbHostCASystemIdentifier  kOID_ocStbHostCASystemIdentifier
  #define kNAME_ocStbHostCAType  kOID_ocStbHostCAType
#endif

class ocStbHostSecuritySubSystemGroup : public BcmMibGroup
{
  friend class ocStbHostSecuritySubSystemGroupBridge;

  public:
    ocStbHostSecuritySubSystemGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostSecuritySubSystemGroupBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostCASystemIdentifier = 3,
      kLEAF_ocStbHostCAType = 4
    };
};



/*\
 *  ocStbHostSoftwareGroup group
\*/

// OID constants
#define kOID_ocStbHostSoftwareGroup  "1.3.6.1.4.1.4491.2.3.1.1.3"
#define kOID_ocStbHostDeviceSoftwareBaseGroup  "1.3.6.1.4.1.4491.2.3.1.1.3.1"
#define kOID_ocStbHostFirmwareDownloadStatusGroup  "1.3.6.1.4.1.4491.2.3.1.1.3.2"
#define kOID_ocStbHostSoftwareApplicationInfoGroup  "1.3.6.1.4.1.4491.2.3.1.1.3.3"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostSoftwareGroup  "ocStbHostSoftwareGroup"
  #define kNAME_ocStbHostDeviceSoftwareBaseGroup  "ocStbHostDeviceSoftwareBaseGroup"
  #define kNAME_ocStbHostFirmwareDownloadStatusGroup  "ocStbHostFirmwareDownloadStatusGroup"
  #define kNAME_ocStbHostSoftwareApplicationInfoGroup  "ocStbHostSoftwareApplicationInfoGroup"
#else
  #define kNAME_ocStbHostSoftwareGroup  kOID_ocStbHostSoftwareGroup
  #define kNAME_ocStbHostDeviceSoftwareBaseGroup  kOID_ocStbHostDeviceSoftwareBaseGroup
  #define kNAME_ocStbHostFirmwareDownloadStatusGroup  kOID_ocStbHostFirmwareDownloadStatusGroup
  #define kNAME_ocStbHostSoftwareApplicationInfoGroup  kOID_ocStbHostSoftwareApplicationInfoGroup
#endif

class ocStbHostSoftwareGroup : public BcmMibGroup
{
  public:
    ocStbHostSoftwareGroup ();

    bool Populate ();
};



/*\
 *  ocStbHostDeviceSoftwareBaseGroup group
\*/

// OID constants
#define kOID_ocStbHostDeviceSoftwareBaseGroup  "1.3.6.1.4.1.4491.2.3.1.1.3.1"
#define kOID_ocStbHostSoftwareFirmwareVersion  "1.3.6.1.4.1.4491.2.3.1.1.3.1.1.0"
#define kOID_ocStbHostSoftwareOCAPVersion  "1.3.6.1.4.1.4491.2.3.1.1.3.1.2.0"
#define kOID_ocStbHostSoftwareFirmwareReleaseDate  "1.3.6.1.4.1.4491.2.3.1.1.3.1.3.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostDeviceSoftwareBaseGroup  "ocStbHostDeviceSoftwareBaseGroup"
  #define kNAME_ocStbHostSoftwareFirmwareVersion  "ocStbHostSoftwareFirmwareVersion"
  #define kNAME_ocStbHostSoftwareOCAPVersion  "ocStbHostSoftwareOCAPVersion"
  #define kNAME_ocStbHostSoftwareFirmwareReleaseDate  "ocStbHostSoftwareFirmwareReleaseDate"
#else
  #define kNAME_ocStbHostDeviceSoftwareBaseGroup  kOID_ocStbHostDeviceSoftwareBaseGroup
  #define kNAME_ocStbHostSoftwareFirmwareVersion  kOID_ocStbHostSoftwareFirmwareVersion
  #define kNAME_ocStbHostSoftwareOCAPVersion  kOID_ocStbHostSoftwareOCAPVersion
  #define kNAME_ocStbHostSoftwareFirmwareReleaseDate  kOID_ocStbHostSoftwareFirmwareReleaseDate
#endif

class ocStbHostDeviceSoftwareBaseGroup : public BcmMibGroup
{
  friend class ocStbHostDeviceSoftwareBaseGroupBridge;

  public:
    ocStbHostDeviceSoftwareBaseGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostDeviceSoftwareBaseGroupBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostSoftwareFirmwareVersion = 1,
      kLEAF_ocStbHostSoftwareOCAPVersion = 2,
      kLEAF_ocStbHostSoftwareFirmwareReleaseDate = 3
    };
};



/*\
 *  ocStbHostFirmwareDownloadStatusGroup group
\*/

// OID constants
#define kOID_ocStbHostFirmwareDownloadStatusGroup  "1.3.6.1.4.1.4491.2.3.1.1.3.2"
#define kOID_ocStbHostFirmwareImageStatus  "1.3.6.1.4.1.4491.2.3.1.1.3.2.1.0"
#define kOID_ocStbHostFirmwareCodeDownloadStatus  "1.3.6.1.4.1.4491.2.3.1.1.3.2.2.0"
#define kOID_ocStbHostFirmwareCodeObjectName  "1.3.6.1.4.1.4491.2.3.1.1.3.2.3.0"
#define kOID_ocStbHostFirmwareDownloadFailedStatus  "1.3.6.1.4.1.4491.2.3.1.1.3.2.4.0"
#define kOID_ocStbHostFirmwareDownloadFailedCount  "1.3.6.1.4.1.4491.2.3.1.1.3.2.5.0"
#define kOID_ocStbHostFirmwareDownloadGroupId  "1.3.6.1.4.1.4491.2.3.1.1.3.2.6.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostFirmwareDownloadStatusGroup  "ocStbHostFirmwareDownloadStatusGroup"
  #define kNAME_ocStbHostFirmwareImageStatus  "ocStbHostFirmwareImageStatus"
  #define kNAME_ocStbHostFirmwareCodeDownloadStatus  "ocStbHostFirmwareCodeDownloadStatus"
  #define kNAME_ocStbHostFirmwareCodeObjectName  "ocStbHostFirmwareCodeObjectName"
  #define kNAME_ocStbHostFirmwareDownloadFailedStatus  "ocStbHostFirmwareDownloadFailedStatus"
  #define kNAME_ocStbHostFirmwareDownloadFailedCount  "ocStbHostFirmwareDownloadFailedCount"
  #define kNAME_ocStbHostFirmwareDownloadGroupId  "ocStbHostFirmwareDownloadGroupId"
#else
  #define kNAME_ocStbHostFirmwareDownloadStatusGroup  kOID_ocStbHostFirmwareDownloadStatusGroup
  #define kNAME_ocStbHostFirmwareImageStatus  kOID_ocStbHostFirmwareImageStatus
  #define kNAME_ocStbHostFirmwareCodeDownloadStatus  kOID_ocStbHostFirmwareCodeDownloadStatus
  #define kNAME_ocStbHostFirmwareCodeObjectName  kOID_ocStbHostFirmwareCodeObjectName
  #define kNAME_ocStbHostFirmwareDownloadFailedStatus  kOID_ocStbHostFirmwareDownloadFailedStatus
  #define kNAME_ocStbHostFirmwareDownloadFailedCount  kOID_ocStbHostFirmwareDownloadFailedCount
  #define kNAME_ocStbHostFirmwareDownloadGroupId  kOID_ocStbHostFirmwareDownloadGroupId
#endif

class ocStbHostFirmwareDownloadStatusGroup : public BcmMibGroup
{
  friend class ocStbHostFirmwareDownloadStatusGroupBridge;

  public:
    ocStbHostFirmwareDownloadStatusGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostFirmwareDownloadStatusGroupBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostFirmwareImageStatus = 1,
      kLEAF_ocStbHostFirmwareCodeDownloadStatus = 2,
      kLEAF_ocStbHostFirmwareCodeObjectName = 3,
      kLEAF_ocStbHostFirmwareDownloadFailedStatus = 4,
      kLEAF_ocStbHostFirmwareDownloadFailedCount = 5,
      kLEAF_ocStbHostFirmwareDownloadGroupId = 6
    };
};



/*\
 *  ocStbHostSoftwareApplicationInfoGroup group
\*/

// OID constants
#define kOID_ocStbHostSoftwareApplicationInfoGroup  "1.3.6.1.4.1.4491.2.3.1.1.3.3"
#define kOID_ocStbHostSoftwareApplicationInfoSigLastReceivedTime  "1.3.6.1.4.1.4491.2.3.1.1.3.3.2.0"
#define kOID_ocStbHostSoftwareApplicationInfoSigLastReadStatus  "1.3.6.1.4.1.4491.2.3.1.1.3.3.3.0"
#define kOID_ocStbHostSoftwareApplicationInfoSigLastNetworkVersionRead  "1.3.6.1.4.1.4491.2.3.1.1.3.3.4.0"
#define kOID_ocStbHostSoftwareApplicationInfoSigVersionInUse  "1.3.6.1.4.1.4491.2.3.1.1.3.3.5.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostSoftwareApplicationInfoGroup  "ocStbHostSoftwareApplicationInfoGroup"
  #define kNAME_ocStbHostSoftwareApplicationInfoSigLastReceivedTime  "ocStbHostSoftwareApplicationInfoSigLastReceivedTime"
  #define kNAME_ocStbHostSoftwareApplicationInfoSigLastReadStatus  "ocStbHostSoftwareApplicationInfoSigLastReadStatus"
  #define kNAME_ocStbHostSoftwareApplicationInfoSigLastNetworkVersionRead  "ocStbHostSoftwareApplicationInfoSigLastNetworkVersionRead"
  #define kNAME_ocStbHostSoftwareApplicationInfoSigVersionInUse  "ocStbHostSoftwareApplicationInfoSigVersionInUse"
#else
  #define kNAME_ocStbHostSoftwareApplicationInfoGroup  kOID_ocStbHostSoftwareApplicationInfoGroup
  #define kNAME_ocStbHostSoftwareApplicationInfoSigLastReceivedTime  kOID_ocStbHostSoftwareApplicationInfoSigLastReceivedTime
  #define kNAME_ocStbHostSoftwareApplicationInfoSigLastReadStatus  kOID_ocStbHostSoftwareApplicationInfoSigLastReadStatus
  #define kNAME_ocStbHostSoftwareApplicationInfoSigLastNetworkVersionRead  kOID_ocStbHostSoftwareApplicationInfoSigLastNetworkVersionRead
  #define kNAME_ocStbHostSoftwareApplicationInfoSigVersionInUse  kOID_ocStbHostSoftwareApplicationInfoSigVersionInUse
#endif

class ocStbHostSoftwareApplicationInfoGroup : public BcmMibGroup
{
  friend class ocStbHostSoftwareApplicationInfoGroupBridge;

  public:
    ocStbHostSoftwareApplicationInfoGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostSoftwareApplicationInfoGroupBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostSoftwareApplicationInfoSigLastReceivedTime = 2,
      kLEAF_ocStbHostSoftwareApplicationInfoSigLastReadStatus = 3,
      kLEAF_ocStbHostSoftwareApplicationInfoSigLastNetworkVersionRead = 4,
      kLEAF_ocStbHostSoftwareApplicationInfoSigVersionInUse = 5
    };
};


/* ocStbHostSoftwareApplicationInfoTable and related objects */

// OID constants
#define kOID_ocStbHostSoftwareApplicationInfoTable  "1.3.6.1.4.1.4491.2.3.1.1.3.3.1"
#define kOID_ocStbHostSoftwareApplicationInfoEntry  "1.3.6.1.4.1.4491.2.3.1.1.3.3.1.1"
#define kOID_ocStbHostSoftwareAppNameString  "1.3.6.1.4.1.4491.2.3.1.1.3.3.1.1.1"
#define kOID_ocStbHostSoftwareAppVersionNumber  "1.3.6.1.4.1.4491.2.3.1.1.3.3.1.1.2"
#define kOID_ocStbHostSoftwareStatus  "1.3.6.1.4.1.4491.2.3.1.1.3.3.1.1.3"
#define kOID_ocStbHostSoftwareApplicationInfoIndex  "1.3.6.1.4.1.4491.2.3.1.1.3.3.1.1.4"
#define kOID_ocStbHostSoftwareOrganizationId  "1.3.6.1.4.1.4491.2.3.1.1.3.3.1.1.5"
#define kOID_ocStbHostSoftwareApplicationId  "1.3.6.1.4.1.4491.2.3.1.1.3.3.1.1.6"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostSoftwareApplicationInfoTable  "ocStbHostSoftwareApplicationInfoTable"
  #define kNAME_ocStbHostSoftwareApplicationInfoEntry  "ocStbHostSoftwareApplicationInfoEntry"
  #define kNAME_ocStbHostSoftwareAppNameString  "ocStbHostSoftwareAppNameString"
  #define kNAME_ocStbHostSoftwareAppVersionNumber  "ocStbHostSoftwareAppVersionNumber"
  #define kNAME_ocStbHostSoftwareStatus  "ocStbHostSoftwareStatus"
  #define kNAME_ocStbHostSoftwareApplicationInfoIndex  "ocStbHostSoftwareApplicationInfoIndex"
  #define kNAME_ocStbHostSoftwareOrganizationId  "ocStbHostSoftwareOrganizationId"
  #define kNAME_ocStbHostSoftwareApplicationId  "ocStbHostSoftwareApplicationId"
#else
  #define kNAME_ocStbHostSoftwareApplicationInfoTable  kOID_ocStbHostSoftwareApplicationInfoTable
  #define kNAME_ocStbHostSoftwareApplicationInfoEntry  kOID_ocStbHostSoftwareApplicationInfoEntry
  #define kNAME_ocStbHostSoftwareAppNameString  kOID_ocStbHostSoftwareAppNameString
  #define kNAME_ocStbHostSoftwareAppVersionNumber  kOID_ocStbHostSoftwareAppVersionNumber
  #define kNAME_ocStbHostSoftwareStatus  kOID_ocStbHostSoftwareStatus
  #define kNAME_ocStbHostSoftwareApplicationInfoIndex  kOID_ocStbHostSoftwareApplicationInfoIndex
  #define kNAME_ocStbHostSoftwareOrganizationId  kOID_ocStbHostSoftwareOrganizationId
  #define kNAME_ocStbHostSoftwareApplicationId  kOID_ocStbHostSoftwareApplicationId
#endif

class ocStbHostSoftwareApplicationInfoTable : public BcmMibTable
{
  public:
    ocStbHostSoftwareApplicationInfoTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ocStbHostSoftwareApplicationInfoEntry : public BcmMibEntry
{
  friend class ocStbHostSoftwareApplicationInfoEntryBridge;
  friend class ocStbHostSoftwareApplicationInfoTable;

  public:
    ocStbHostSoftwareApplicationInfoEntry (ocStbHostSoftwareApplicationInfoTable *pTable, ocStbHostSoftwareApplicationInfoEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostSoftwareApplicationInfoEntryBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostSoftwareAppNameString = 1,
      kLEAF_ocStbHostSoftwareAppVersionNumber = 2,
      kLEAF_ocStbHostSoftwareStatus = 3,
      kLEAF_ocStbHostSoftwareApplicationInfoIndex = 4,
      kLEAF_ocStbHostSoftwareOrganizationId = 5,
      kLEAF_ocStbHostSoftwareApplicationId = 6
    };
};



/*\
 *  ocStbHostStatusGroup group
\*/

// OID constants
#define kOID_ocStbHostStatusGroup  "1.3.6.1.4.1.4491.2.3.1.1.4"
#define kOID_ocStbHostPowerGroup  "1.3.6.1.4.1.4491.2.3.1.1.4.1"
#define kOID_ocStbHostSystemObjectsGroup  "1.3.6.1.4.1.4491.2.3.1.1.4.3"
#define kOID_ocStbCardInfoGroup  "1.3.6.1.4.1.4491.2.3.1.1.4.4"
#define kOID_ocStbHostInfoGroup  "1.3.6.1.4.1.4491.2.3.1.1.4.5"
#define kOID_ocStbHostRebootInfoGroup  "1.3.6.1.4.1.4491.2.3.1.1.4.6"
#define kOID_ocStbHostMemoryInfoGroup  "1.3.6.1.4.1.4491.2.3.1.1.4.7"
#define kOID_ocStbHostJVMInfoGroup  "1.3.6.1.4.1.4491.2.3.1.1.4.8"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostStatusGroup  "ocStbHostStatusGroup"
  #define kNAME_ocStbHostPowerGroup  "ocStbHostPowerGroup"
  #define kNAME_ocStbHostSystemObjectsGroup  "ocStbHostSystemObjectsGroup"
  #define kNAME_ocStbCardInfoGroup  "ocStbCardInfoGroup"
  #define kNAME_ocStbHostInfoGroup  "ocStbHostInfoGroup"
  #define kNAME_ocStbHostRebootInfoGroup  "ocStbHostRebootInfoGroup"
  #define kNAME_ocStbHostMemoryInfoGroup  "ocStbHostMemoryInfoGroup"
  #define kNAME_ocStbHostJVMInfoGroup  "ocStbHostJVMInfoGroup"
#else
  #define kNAME_ocStbHostStatusGroup  kOID_ocStbHostStatusGroup
  #define kNAME_ocStbHostPowerGroup  kOID_ocStbHostPowerGroup
  #define kNAME_ocStbHostSystemObjectsGroup  kOID_ocStbHostSystemObjectsGroup
  #define kNAME_ocStbCardInfoGroup  kOID_ocStbCardInfoGroup
  #define kNAME_ocStbHostInfoGroup  kOID_ocStbHostInfoGroup
  #define kNAME_ocStbHostRebootInfoGroup  kOID_ocStbHostRebootInfoGroup
  #define kNAME_ocStbHostMemoryInfoGroup  kOID_ocStbHostMemoryInfoGroup
  #define kNAME_ocStbHostJVMInfoGroup  kOID_ocStbHostJVMInfoGroup
#endif

class ocStbHostStatusGroup : public BcmMibGroup
{
  public:
    ocStbHostStatusGroup ();

    bool Populate ();
};



/*\
 *  ocStbHostPowerGroup group
\*/

// OID constants
#define kOID_ocStbHostPowerGroup  "1.3.6.1.4.1.4491.2.3.1.1.4.1"
#define kOID_ocStbHostPowerStatus  "1.3.6.1.4.1.4491.2.3.1.1.4.1.1.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostPowerGroup  "ocStbHostPowerGroup"
  #define kNAME_ocStbHostPowerStatus  "ocStbHostPowerStatus"
#else
  #define kNAME_ocStbHostPowerGroup  kOID_ocStbHostPowerGroup
  #define kNAME_ocStbHostPowerStatus  kOID_ocStbHostPowerStatus
#endif

class ocStbHostPowerGroup : public BcmMibGroup
{
  friend class ocStbHostPowerGroupBridge;

  public:
    ocStbHostPowerGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostPowerGroupBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostPowerStatus = 1
    };
};



/*\
 *  ocStbHostSystemObjectsGroup group
\*/

// OID constants
#define kOID_ocStbHostSystemObjectsGroup  "1.3.6.1.4.1.4491.2.3.1.1.4.3"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostSystemObjectsGroup  "ocStbHostSystemObjectsGroup"
#else
  #define kNAME_ocStbHostSystemObjectsGroup  kOID_ocStbHostSystemObjectsGroup
#endif

class ocStbHostSystemObjectsGroup : public BcmMibGroup
{
  public:
    ocStbHostSystemObjectsGroup ();

    bool Populate ();
};


/* ocStbHostSystemMemoryReportTable and related objects */

// OID constants
#define kOID_ocStbHostSystemMemoryReportTable  "1.3.6.1.4.1.4491.2.3.1.1.4.3.3"
#define kOID_ocStbHostSystemMemoryReportEntry  "1.3.6.1.4.1.4491.2.3.1.1.4.3.3.1"
#define kOID_ocStbHostSystemMemoryReportIndex  "1.3.6.1.4.1.4491.2.3.1.1.4.3.3.1.1"
#define kOID_ocStbHostSystemMemoryReportMemoryType  "1.3.6.1.4.1.4491.2.3.1.1.4.3.3.1.2"
#define kOID_ocStbHostSystemMemoryReportMemorySize  "1.3.6.1.4.1.4491.2.3.1.1.4.3.3.1.3"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostSystemMemoryReportTable  "ocStbHostSystemMemoryReportTable"
  #define kNAME_ocStbHostSystemMemoryReportEntry  "ocStbHostSystemMemoryReportEntry"
  #define kNAME_ocStbHostSystemMemoryReportIndex  "ocStbHostSystemMemoryReportIndex"
  #define kNAME_ocStbHostSystemMemoryReportMemoryType  "ocStbHostSystemMemoryReportMemoryType"
  #define kNAME_ocStbHostSystemMemoryReportMemorySize  "ocStbHostSystemMemoryReportMemorySize"
#else
  #define kNAME_ocStbHostSystemMemoryReportTable  kOID_ocStbHostSystemMemoryReportTable
  #define kNAME_ocStbHostSystemMemoryReportEntry  kOID_ocStbHostSystemMemoryReportEntry
  #define kNAME_ocStbHostSystemMemoryReportIndex  kOID_ocStbHostSystemMemoryReportIndex
  #define kNAME_ocStbHostSystemMemoryReportMemoryType  kOID_ocStbHostSystemMemoryReportMemoryType
  #define kNAME_ocStbHostSystemMemoryReportMemorySize  kOID_ocStbHostSystemMemoryReportMemorySize
#endif

class ocStbHostSystemMemoryReportTable : public BcmMibTable
{
  public:
    ocStbHostSystemMemoryReportTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ocStbHostSystemMemoryReportEntry : public BcmMibEntry
{
  friend class ocStbHostSystemMemoryReportEntryBridge;
  friend class ocStbHostSystemMemoryReportTable;

  public:
    ocStbHostSystemMemoryReportEntry (ocStbHostSystemMemoryReportTable *pTable, ocStbHostSystemMemoryReportEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostSystemMemoryReportEntryBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostSystemMemoryReportIndex = 1,
      kLEAF_ocStbHostSystemMemoryReportMemoryType = 2,
      kLEAF_ocStbHostSystemMemoryReportMemorySize = 3
    };
};



/*\
 *  ocStbCardInfoGroup group
\*/

// OID constants
#define kOID_ocStbCardInfoGroup  "1.3.6.1.4.1.4491.2.3.1.1.4.4"
#define kOID_ocStbHostCardMacAddress  "1.3.6.1.4.1.4491.2.3.1.1.4.4.1.0"
#define kOID_ocStbHostCardIpAddressType  "1.3.6.1.4.1.4491.2.3.1.1.4.4.2.0"
#define kOID_ocStbHostCardIpAddress  "1.3.6.1.4.1.4491.2.3.1.1.4.4.3.0"
#define kOID_ocStbHostCardId  "1.3.6.1.4.1.4491.2.3.1.1.4.4.7.0"
#define kOID_ocStbHostCardBindingStatus  "1.3.6.1.4.1.4491.2.3.1.1.4.4.8.0"
#define kOID_ocStbHostCardOpenedGenericResource  "1.3.6.1.4.1.4491.2.3.1.1.4.4.9.0"
#define kOID_ocStbHostCardTimeZoneOffset  "1.3.6.1.4.1.4491.2.3.1.1.4.4.10.0"
#define kOID_ocStbHostCardDaylightSavingsTimeDelta  "1.3.6.1.4.1.4491.2.3.1.1.4.4.11.0"
#define kOID_ocStbHostCardDaylightSavingsTimeEntry  "1.3.6.1.4.1.4491.2.3.1.1.4.4.12.0"
#define kOID_ocStbHostCardDaylightSavingsTimeExit  "1.3.6.1.4.1.4491.2.3.1.1.4.4.13.0"
#define kOID_ocStbHostCardEaLocationCode  "1.3.6.1.4.1.4491.2.3.1.1.4.4.14.0"
#define kOID_ocStbHostCardVctId  "1.3.6.1.4.1.4491.2.3.1.1.4.4.15.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbCardInfoGroup  "ocStbCardInfoGroup"
  #define kNAME_ocStbHostCardMacAddress  "ocStbHostCardMacAddress"
  #define kNAME_ocStbHostCardIpAddressType  "ocStbHostCardIpAddressType"
  #define kNAME_ocStbHostCardIpAddress  "ocStbHostCardIpAddress"
  #define kNAME_ocStbHostCardId  "ocStbHostCardId"
  #define kNAME_ocStbHostCardBindingStatus  "ocStbHostCardBindingStatus"
  #define kNAME_ocStbHostCardOpenedGenericResource  "ocStbHostCardOpenedGenericResource"
  #define kNAME_ocStbHostCardTimeZoneOffset  "ocStbHostCardTimeZoneOffset"
  #define kNAME_ocStbHostCardDaylightSavingsTimeDelta  "ocStbHostCardDaylightSavingsTimeDelta"
  #define kNAME_ocStbHostCardDaylightSavingsTimeEntry  "ocStbHostCardDaylightSavingsTimeEntry"
  #define kNAME_ocStbHostCardDaylightSavingsTimeExit  "ocStbHostCardDaylightSavingsTimeExit"
  #define kNAME_ocStbHostCardEaLocationCode  "ocStbHostCardEaLocationCode"
  #define kNAME_ocStbHostCardVctId  "ocStbHostCardVctId"
#else
  #define kNAME_ocStbCardInfoGroup  kOID_ocStbCardInfoGroup
  #define kNAME_ocStbHostCardMacAddress  kOID_ocStbHostCardMacAddress
  #define kNAME_ocStbHostCardIpAddressType  kOID_ocStbHostCardIpAddressType
  #define kNAME_ocStbHostCardIpAddress  kOID_ocStbHostCardIpAddress
  #define kNAME_ocStbHostCardId  kOID_ocStbHostCardId
  #define kNAME_ocStbHostCardBindingStatus  kOID_ocStbHostCardBindingStatus
  #define kNAME_ocStbHostCardOpenedGenericResource  kOID_ocStbHostCardOpenedGenericResource
  #define kNAME_ocStbHostCardTimeZoneOffset  kOID_ocStbHostCardTimeZoneOffset
  #define kNAME_ocStbHostCardDaylightSavingsTimeDelta  kOID_ocStbHostCardDaylightSavingsTimeDelta
  #define kNAME_ocStbHostCardDaylightSavingsTimeEntry  kOID_ocStbHostCardDaylightSavingsTimeEntry
  #define kNAME_ocStbHostCardDaylightSavingsTimeExit  kOID_ocStbHostCardDaylightSavingsTimeExit
  #define kNAME_ocStbHostCardEaLocationCode  kOID_ocStbHostCardEaLocationCode
  #define kNAME_ocStbHostCardVctId  kOID_ocStbHostCardVctId
#endif

class ocStbCardInfoGroup : public BcmMibGroup
{
  friend class ocStbCardInfoGroupBridge;

  public:
    ocStbCardInfoGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbCardInfoGroupBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostCardMacAddress = 1,
      kLEAF_ocStbHostCardIpAddressType = 2,
      kLEAF_ocStbHostCardIpAddress = 3,
      kLEAF_ocStbHostCardId = 7,
      kLEAF_ocStbHostCardBindingStatus = 8,
      kLEAF_ocStbHostCardOpenedGenericResource = 9,
      kLEAF_ocStbHostCardTimeZoneOffset = 10,
      kLEAF_ocStbHostCardDaylightSavingsTimeDelta = 11,
      kLEAF_ocStbHostCardDaylightSavingsTimeEntry = 12,
      kLEAF_ocStbHostCardDaylightSavingsTimeExit = 13,
      kLEAF_ocStbHostCardEaLocationCode = 14,
      kLEAF_ocStbHostCardVctId = 15
    };
};



/*\
 *  ocStbHostCCMMIGroup group
\*/

// OID constants
#define kOID_ocStbHostCCMMIGroup  "1.3.6.1.4.1.4491.2.3.1.1.4.4.5"
#define kOID_ocStbHostCCApplicationsGroup  "1.3.6.1.4.1.4491.2.3.1.1.4.4.5.1"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostCCMMIGroup  "ocStbHostCCMMIGroup"
  #define kNAME_ocStbHostCCApplicationsGroup  "ocStbHostCCApplicationsGroup"
#else
  #define kNAME_ocStbHostCCMMIGroup  kOID_ocStbHostCCMMIGroup
  #define kNAME_ocStbHostCCApplicationsGroup  kOID_ocStbHostCCApplicationsGroup
#endif

class ocStbHostCCMMIGroup : public BcmMibGroup
{
  public:
    ocStbHostCCMMIGroup ();

    bool Populate ();
};



/*\
 *  ocStbHostCCApplicationsGroup group
\*/

// OID constants
#define kOID_ocStbHostCCApplicationsGroup  "1.3.6.1.4.1.4491.2.3.1.1.4.4.5.1"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostCCApplicationsGroup  "ocStbHostCCApplicationsGroup"
#else
  #define kNAME_ocStbHostCCApplicationsGroup  kOID_ocStbHostCCApplicationsGroup
#endif

class ocStbHostCCApplicationsGroup : public BcmMibGroup
{
  public:
    ocStbHostCCApplicationsGroup ();

    bool Populate ();
};


/* ocStbHostCCAppInfoTable and related objects */

// OID constants
#define kOID_ocStbHostCCAppInfoTable  "1.3.6.1.4.1.4491.2.3.1.1.4.4.5.1.1"
#define kOID_ocStbHostCCAppInfoEntry  "1.3.6.1.4.1.4491.2.3.1.1.4.4.5.1.1.1"
#define kOID_ocStbHostCCAppInfoIndex  "1.3.6.1.4.1.4491.2.3.1.1.4.4.5.1.1.1.1"
#define kOID_ocStbHostCCApplicationType  "1.3.6.1.4.1.4491.2.3.1.1.4.4.5.1.1.1.2"
#define kOID_ocStbHostCCApplicationName  "1.3.6.1.4.1.4491.2.3.1.1.4.4.5.1.1.1.3"
#define kOID_ocStbHostCCApplicationVersion  "1.3.6.1.4.1.4491.2.3.1.1.4.4.5.1.1.1.4"
#define kOID_ocStbHostCCAppInfoPage  "1.3.6.1.4.1.4491.2.3.1.1.4.4.5.1.1.1.5"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostCCAppInfoTable  "ocStbHostCCAppInfoTable"
  #define kNAME_ocStbHostCCAppInfoEntry  "ocStbHostCCAppInfoEntry"
  #define kNAME_ocStbHostCCAppInfoIndex  "ocStbHostCCAppInfoIndex"
  #define kNAME_ocStbHostCCApplicationType  "ocStbHostCCApplicationType"
  #define kNAME_ocStbHostCCApplicationName  "ocStbHostCCApplicationName"
  #define kNAME_ocStbHostCCApplicationVersion  "ocStbHostCCApplicationVersion"
  #define kNAME_ocStbHostCCAppInfoPage  "ocStbHostCCAppInfoPage"
#else
  #define kNAME_ocStbHostCCAppInfoTable  kOID_ocStbHostCCAppInfoTable
  #define kNAME_ocStbHostCCAppInfoEntry  kOID_ocStbHostCCAppInfoEntry
  #define kNAME_ocStbHostCCAppInfoIndex  kOID_ocStbHostCCAppInfoIndex
  #define kNAME_ocStbHostCCApplicationType  kOID_ocStbHostCCApplicationType
  #define kNAME_ocStbHostCCApplicationName  kOID_ocStbHostCCApplicationName
  #define kNAME_ocStbHostCCApplicationVersion  kOID_ocStbHostCCApplicationVersion
  #define kNAME_ocStbHostCCAppInfoPage  kOID_ocStbHostCCAppInfoPage
#endif

class ocStbHostCCAppInfoTable : public BcmMibTable
{
  public:
    ocStbHostCCAppInfoTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ocStbHostCCAppInfoEntry : public BcmMibEntry
{
  friend class ocStbHostCCAppInfoEntryBridge;
  friend class ocStbHostCCAppInfoTable;

  public:
    ocStbHostCCAppInfoEntry (ocStbHostCCAppInfoTable *pTable, ocStbHostCCAppInfoEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostCCAppInfoEntryBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostCCAppInfoIndex = 1,
      kLEAF_ocStbHostCCApplicationType = 2,
      kLEAF_ocStbHostCCApplicationName = 3,
      kLEAF_ocStbHostCCApplicationVersion = 4,
      kLEAF_ocStbHostCCAppInfoPage = 5
    };
};



/*\
 *  ocStbHostSnmpProxyInfoGroup group
\*/

// OID constants
#define kOID_ocStbHostSnmpProxyInfoGroup  "1.3.6.1.4.1.4491.2.3.1.1.4.4.6"
#define kOID_ocStbHostCardMfgId  "1.3.6.1.4.1.4491.2.3.1.1.4.4.6.1.0"
#define kOID_ocStbHostCardVersion  "1.3.6.1.4.1.4491.2.3.1.1.4.4.6.2.0"
#define kOID_ocStbHostCardRootOid  "1.3.6.1.4.1.4491.2.3.1.1.4.4.6.3.0"
#define kOID_ocStbHostCardSerialNumber  "1.3.6.1.4.1.4491.2.3.1.1.4.4.6.4.0"
#define kOID_ocStbHostCardSnmpAccessControl  "1.3.6.1.4.1.4491.2.3.1.1.4.4.6.5.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostSnmpProxyInfoGroup  "ocStbHostSnmpProxyInfoGroup"
  #define kNAME_ocStbHostCardMfgId  "ocStbHostCardMfgId"
  #define kNAME_ocStbHostCardVersion  "ocStbHostCardVersion"
  #define kNAME_ocStbHostCardRootOid  "ocStbHostCardRootOid"
  #define kNAME_ocStbHostCardSerialNumber  "ocStbHostCardSerialNumber"
  #define kNAME_ocStbHostCardSnmpAccessControl  "ocStbHostCardSnmpAccessControl"
#else
  #define kNAME_ocStbHostSnmpProxyInfoGroup  kOID_ocStbHostSnmpProxyInfoGroup
  #define kNAME_ocStbHostCardMfgId  kOID_ocStbHostCardMfgId
  #define kNAME_ocStbHostCardVersion  kOID_ocStbHostCardVersion
  #define kNAME_ocStbHostCardRootOid  kOID_ocStbHostCardRootOid
  #define kNAME_ocStbHostCardSerialNumber  kOID_ocStbHostCardSerialNumber
  #define kNAME_ocStbHostCardSnmpAccessControl  kOID_ocStbHostCardSnmpAccessControl
#endif

class ocStbHostSnmpProxyInfoGroup : public BcmMibGroup
{
  friend class ocStbHostSnmpProxyInfoGroupBridge;

  public:
    ocStbHostSnmpProxyInfoGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostSnmpProxyInfoGroupBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostCardMfgId = 1,
      kLEAF_ocStbHostCardVersion = 2,
      kLEAF_ocStbHostCardRootOid = 3,
      kLEAF_ocStbHostCardSerialNumber = 4,
      kLEAF_ocStbHostCardSnmpAccessControl = 5
    };
};



/*\
 *  ocStbHostCardCpInfoGroup group
\*/

// OID constants
#define kOID_ocStbHostCardCpInfoGroup  "1.3.6.1.4.1.4491.2.3.1.1.4.4.16"
#define kOID_ocStbHostCardCpAuthKeyStatus  "1.3.6.1.4.1.4491.2.3.1.1.4.4.16.1.0"
#define kOID_ocStbHostCardCpCertificateCheck  "1.3.6.1.4.1.4491.2.3.1.1.4.4.16.2.0"
#define kOID_ocStbHostCardCpCciChallengeCount  "1.3.6.1.4.1.4491.2.3.1.1.4.4.16.3.0"
#define kOID_ocStbHostCardCpKeyGenerationReqCount  "1.3.6.1.4.1.4491.2.3.1.1.4.4.16.4.0"
#define kOID_ocStbHostCardCpIdList  "1.3.6.1.4.1.4491.2.3.1.1.4.4.16.5.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostCardCpInfoGroup  "ocStbHostCardCpInfoGroup"
  #define kNAME_ocStbHostCardCpAuthKeyStatus  "ocStbHostCardCpAuthKeyStatus"
  #define kNAME_ocStbHostCardCpCertificateCheck  "ocStbHostCardCpCertificateCheck"
  #define kNAME_ocStbHostCardCpCciChallengeCount  "ocStbHostCardCpCciChallengeCount"
  #define kNAME_ocStbHostCardCpKeyGenerationReqCount  "ocStbHostCardCpKeyGenerationReqCount"
  #define kNAME_ocStbHostCardCpIdList  "ocStbHostCardCpIdList"
#else
  #define kNAME_ocStbHostCardCpInfoGroup  kOID_ocStbHostCardCpInfoGroup
  #define kNAME_ocStbHostCardCpAuthKeyStatus  kOID_ocStbHostCardCpAuthKeyStatus
  #define kNAME_ocStbHostCardCpCertificateCheck  kOID_ocStbHostCardCpCertificateCheck
  #define kNAME_ocStbHostCardCpCciChallengeCount  kOID_ocStbHostCardCpCciChallengeCount
  #define kNAME_ocStbHostCardCpKeyGenerationReqCount  kOID_ocStbHostCardCpKeyGenerationReqCount
  #define kNAME_ocStbHostCardCpIdList  kOID_ocStbHostCardCpIdList
#endif

class ocStbHostCardCpInfoGroup : public BcmMibGroup
{
  friend class ocStbHostCardCpInfoGroupBridge;

  public:
    ocStbHostCardCpInfoGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostCardCpInfoGroupBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostCardCpAuthKeyStatus = 1,
      kLEAF_ocStbHostCardCpCertificateCheck = 2,
      kLEAF_ocStbHostCardCpCciChallengeCount = 3,
      kLEAF_ocStbHostCardCpKeyGenerationReqCount = 4,
      kLEAF_ocStbHostCardCpIdList = 5
    };
};



/*\
 *  ocStbHostInfoGroup group
\*/

// OID constants
#define kOID_ocStbHostInfoGroup  "1.3.6.1.4.1.4491.2.3.1.1.4.5"
#define kOID_ocStbHostIpAddressType  "1.3.6.1.4.1.4491.2.3.1.1.4.5.1.0"
#define kOID_ocStbHostIpSubNetMask  "1.3.6.1.4.1.4491.2.3.1.1.4.5.2.0"
#define kOID_ocStbHostOobMessageMode  "1.3.6.1.4.1.4491.2.3.1.1.4.5.3.0"
#define kOID_ocStbHostBootStatus  "1.3.6.1.4.1.4491.2.3.1.1.4.5.6.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostInfoGroup  "ocStbHostInfoGroup"
  #define kNAME_ocStbHostIpAddressType  "ocStbHostIpAddressType"
  #define kNAME_ocStbHostIpSubNetMask  "ocStbHostIpSubNetMask"
  #define kNAME_ocStbHostOobMessageMode  "ocStbHostOobMessageMode"
  #define kNAME_ocStbHostBootStatus  "ocStbHostBootStatus"
#else
  #define kNAME_ocStbHostInfoGroup  kOID_ocStbHostInfoGroup
  #define kNAME_ocStbHostIpAddressType  kOID_ocStbHostIpAddressType
  #define kNAME_ocStbHostIpSubNetMask  kOID_ocStbHostIpSubNetMask
  #define kNAME_ocStbHostOobMessageMode  kOID_ocStbHostOobMessageMode
  #define kNAME_ocStbHostBootStatus  kOID_ocStbHostBootStatus
#endif

class ocStbHostInfoGroup : public BcmMibGroup
{
  friend class ocStbHostInfoGroupBridge;

  public:
    ocStbHostInfoGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostInfoGroupBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostIpAddressType = 1,
      kLEAF_ocStbHostIpSubNetMask = 2,
      kLEAF_ocStbHostOobMessageMode = 3,
      kLEAF_ocStbHostBootStatus = 6
    };
};



/*\
 *  ocStbHostDumpTrapInfoGroup group
\*/

// OID constants
#define kOID_ocStbHostDumpTrapInfoGroup  "1.3.6.1.4.1.4491.2.3.1.1.4.5.4"
#define kOID_ocStbHostDumpEventCount  "1.3.6.1.4.1.4491.2.3.1.1.4.5.4.1.0"
#define kOID_ocStbHostDumpNow  "1.3.6.1.4.1.4491.2.3.1.1.4.5.4.2.0"
#define kOID_ocStbHostDumpEventTimeout  "1.3.6.1.4.1.4491.2.3.1.1.4.5.4.3.0"
#define kOID_ocStbHostDumpFilePath  "1.3.6.1.4.1.4491.2.3.1.1.4.5.4.4.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostDumpTrapInfoGroup  "ocStbHostDumpTrapInfoGroup"
  #define kNAME_ocStbHostDumpEventCount  "ocStbHostDumpEventCount"
  #define kNAME_ocStbHostDumpNow  "ocStbHostDumpNow"
  #define kNAME_ocStbHostDumpEventTimeout  "ocStbHostDumpEventTimeout"
  #define kNAME_ocStbHostDumpFilePath  "ocStbHostDumpFilePath"
#else
  #define kNAME_ocStbHostDumpTrapInfoGroup  kOID_ocStbHostDumpTrapInfoGroup
  #define kNAME_ocStbHostDumpEventCount  kOID_ocStbHostDumpEventCount
  #define kNAME_ocStbHostDumpNow  kOID_ocStbHostDumpNow
  #define kNAME_ocStbHostDumpEventTimeout  kOID_ocStbHostDumpEventTimeout
  #define kNAME_ocStbHostDumpFilePath  kOID_ocStbHostDumpFilePath
#endif

class ocStbHostDumpTrapInfoGroup : public BcmMibGroup
{
  friend class ocStbHostDumpTrapInfoGroupBridge;

  public:
    ocStbHostDumpTrapInfoGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostDumpTrapInfoGroupBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostDumpEventCount = 1,
      kLEAF_ocStbHostDumpNow = 2,
      kLEAF_ocStbHostDumpEventTimeout = 3,
      kLEAF_ocStbHostDumpFilePath = 4
    };
};



/*\
 *  ocStbHostSpecificationsInfoGroup group
\*/

// OID constants
#define kOID_ocStbHostSpecificationsInfoGroup  "1.3.6.1.4.1.4491.2.3.1.1.4.5.5"
#define kOID_ocStbHostCfrSpecificationIssue  "1.3.6.1.4.1.4491.2.3.1.1.4.5.5.1.0"
#define kOID_ocStbHostMibSpecificationIssue  "1.3.6.1.4.1.4491.2.3.1.1.4.5.5.2.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostSpecificationsInfoGroup  "ocStbHostSpecificationsInfoGroup"
  #define kNAME_ocStbHostCfrSpecificationIssue  "ocStbHostCfrSpecificationIssue"
  #define kNAME_ocStbHostMibSpecificationIssue  "ocStbHostMibSpecificationIssue"
#else
  #define kNAME_ocStbHostSpecificationsInfoGroup  kOID_ocStbHostSpecificationsInfoGroup
  #define kNAME_ocStbHostCfrSpecificationIssue  kOID_ocStbHostCfrSpecificationIssue
  #define kNAME_ocStbHostMibSpecificationIssue  kOID_ocStbHostMibSpecificationIssue
#endif

class ocStbHostSpecificationsInfoGroup : public BcmMibGroup
{
  friend class ocStbHostSpecificationsInfoGroupBridge;

  public:
    ocStbHostSpecificationsInfoGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostSpecificationsInfoGroupBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostCfrSpecificationIssue = 1,
      kLEAF_ocStbHostMibSpecificationIssue = 2
    };
};



/*\
 *  ocStbHostContentErrorSummaryInfoGroup group
\*/

// OID constants
#define kOID_ocStbHostContentErrorSummaryInfoGroup  "1.3.6.1.4.1.4491.2.3.1.1.4.5.7"
#define kOID_ocStbHostPatTimeoutCount  "1.3.6.1.4.1.4491.2.3.1.1.4.5.7.1.0"
#define kOID_ocStbHostPmtTimeoutCount  "1.3.6.1.4.1.4491.2.3.1.1.4.5.7.2.0"
#define kOID_ocStbHostOobCarouselTimeoutCount  "1.3.6.1.4.1.4491.2.3.1.1.4.5.7.3.0"
#define kOID_ocStbHostInbandCarouselTimeoutCount  "1.3.6.1.4.1.4491.2.3.1.1.4.5.7.4.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostContentErrorSummaryInfoGroup  "ocStbHostContentErrorSummaryInfoGroup"
  #define kNAME_ocStbHostPatTimeoutCount  "ocStbHostPatTimeoutCount"
  #define kNAME_ocStbHostPmtTimeoutCount  "ocStbHostPmtTimeoutCount"
  #define kNAME_ocStbHostOobCarouselTimeoutCount  "ocStbHostOobCarouselTimeoutCount"
  #define kNAME_ocStbHostInbandCarouselTimeoutCount  "ocStbHostInbandCarouselTimeoutCount"
#else
  #define kNAME_ocStbHostContentErrorSummaryInfoGroup  kOID_ocStbHostContentErrorSummaryInfoGroup
  #define kNAME_ocStbHostPatTimeoutCount  kOID_ocStbHostPatTimeoutCount
  #define kNAME_ocStbHostPmtTimeoutCount  kOID_ocStbHostPmtTimeoutCount
  #define kNAME_ocStbHostOobCarouselTimeoutCount  kOID_ocStbHostOobCarouselTimeoutCount
  #define kNAME_ocStbHostInbandCarouselTimeoutCount  kOID_ocStbHostInbandCarouselTimeoutCount
#endif

class ocStbHostContentErrorSummaryInfoGroup : public BcmMibGroup
{
  friend class ocStbHostContentErrorSummaryInfoGroupBridge;

  public:
    ocStbHostContentErrorSummaryInfoGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostContentErrorSummaryInfoGroupBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostPatTimeoutCount = 1,
      kLEAF_ocStbHostPmtTimeoutCount = 2,
      kLEAF_ocStbHostOobCarouselTimeoutCount = 3,
      kLEAF_ocStbHostInbandCarouselTimeoutCount = 4
    };
};



/*\
 *  ocStbHostRebootInfoGroup group
\*/

// OID constants
#define kOID_ocStbHostRebootInfoGroup  "1.3.6.1.4.1.4491.2.3.1.1.4.6"
#define kOID_ocStbHostRebootType  "1.3.6.1.4.1.4491.2.3.1.1.4.6.1.0"
#define kOID_ocStbHostRebootReset  "1.3.6.1.4.1.4491.2.3.1.1.4.6.2.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostRebootInfoGroup  "ocStbHostRebootInfoGroup"
  #define kNAME_ocStbHostRebootType  "ocStbHostRebootType"
  #define kNAME_ocStbHostRebootReset  "ocStbHostRebootReset"
#else
  #define kNAME_ocStbHostRebootInfoGroup  kOID_ocStbHostRebootInfoGroup
  #define kNAME_ocStbHostRebootType  kOID_ocStbHostRebootType
  #define kNAME_ocStbHostRebootReset  kOID_ocStbHostRebootReset
#endif

class ocStbHostRebootInfoGroup : public BcmMibGroup
{
  friend class ocStbHostRebootInfoGroupBridge;

  public:
    ocStbHostRebootInfoGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostRebootInfoGroupBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostRebootType = 1,
      kLEAF_ocStbHostRebootReset = 2
    };
};



/*\
 *  ocStbHostMemoryInfoGroup group
\*/

// OID constants
#define kOID_ocStbHostMemoryInfoGroup  "1.3.6.1.4.1.4491.2.3.1.1.4.7"
#define kOID_ocStbHostLargestAvailableBlock  "1.3.6.1.4.1.4491.2.3.1.1.4.7.1.0"
#define kOID_ocStbHostTotalVideoMemory  "1.3.6.1.4.1.4491.2.3.1.1.4.7.2.0"
#define kOID_ocStbHostAvailableVideoMemory  "1.3.6.1.4.1.4491.2.3.1.1.4.7.3.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostMemoryInfoGroup  "ocStbHostMemoryInfoGroup"
  #define kNAME_ocStbHostLargestAvailableBlock  "ocStbHostLargestAvailableBlock"
  #define kNAME_ocStbHostTotalVideoMemory  "ocStbHostTotalVideoMemory"
  #define kNAME_ocStbHostAvailableVideoMemory  "ocStbHostAvailableVideoMemory"
#else
  #define kNAME_ocStbHostMemoryInfoGroup  kOID_ocStbHostMemoryInfoGroup
  #define kNAME_ocStbHostLargestAvailableBlock  kOID_ocStbHostLargestAvailableBlock
  #define kNAME_ocStbHostTotalVideoMemory  kOID_ocStbHostTotalVideoMemory
  #define kNAME_ocStbHostAvailableVideoMemory  kOID_ocStbHostAvailableVideoMemory
#endif

class ocStbHostMemoryInfoGroup : public BcmMibGroup
{
  friend class ocStbHostMemoryInfoGroupBridge;

  public:
    ocStbHostMemoryInfoGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostMemoryInfoGroupBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostLargestAvailableBlock = 1,
      kLEAF_ocStbHostTotalVideoMemory = 2,
      kLEAF_ocStbHostAvailableVideoMemory = 3
    };
};



/*\
 *  ocStbHostJVMInfoGroup group
\*/

// OID constants
#define kOID_ocStbHostJVMInfoGroup  "1.3.6.1.4.1.4491.2.3.1.1.4.8"
#define kOID_ocStbHostJVMHeapSize  "1.3.6.1.4.1.4491.2.3.1.1.4.8.1.0"
#define kOID_ocStbHostJVMAvailHeap  "1.3.6.1.4.1.4491.2.3.1.1.4.8.2.0"
#define kOID_ocStbHostJVMLiveObjects  "1.3.6.1.4.1.4491.2.3.1.1.4.8.3.0"
#define kOID_ocStbHostJVMDeadObjects  "1.3.6.1.4.1.4491.2.3.1.1.4.8.4.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ocStbHostJVMInfoGroup  "ocStbHostJVMInfoGroup"
  #define kNAME_ocStbHostJVMHeapSize  "ocStbHostJVMHeapSize"
  #define kNAME_ocStbHostJVMAvailHeap  "ocStbHostJVMAvailHeap"
  #define kNAME_ocStbHostJVMLiveObjects  "ocStbHostJVMLiveObjects"
  #define kNAME_ocStbHostJVMDeadObjects  "ocStbHostJVMDeadObjects"
#else
  #define kNAME_ocStbHostJVMInfoGroup  kOID_ocStbHostJVMInfoGroup
  #define kNAME_ocStbHostJVMHeapSize  kOID_ocStbHostJVMHeapSize
  #define kNAME_ocStbHostJVMAvailHeap  kOID_ocStbHostJVMAvailHeap
  #define kNAME_ocStbHostJVMLiveObjects  kOID_ocStbHostJVMLiveObjects
  #define kNAME_ocStbHostJVMDeadObjects  kOID_ocStbHostJVMDeadObjects
#endif

class ocStbHostJVMInfoGroup : public BcmMibGroup
{
  friend class ocStbHostJVMInfoGroupBridge;

  public:
    ocStbHostJVMInfoGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ocStbHostJVMInfoGroupBridge *fpBridge;

    enum
    {
      kLEAF_ocStbHostJVMHeapSize = 1,
      kLEAF_ocStbHostJVMAvailHeap = 2,
      kLEAF_ocStbHostJVMLiveObjects = 3,
      kLEAF_ocStbHostJVMDeadObjects = 4
    };
};



#endif


