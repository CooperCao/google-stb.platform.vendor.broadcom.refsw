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
//    Filename: ocStbHostMib.cpp
//    Creation Date: Monday, June 13, 2011 at 13:53:49
//    Created by Broadcom BFC Mib Compiler BCMIBC.EXE version 3.6.1d
//    from input file OC-STB-HOST-MIB.mib
//
//****************************************************************************

#include "ocStbHostMib.h"
#include "ocStbHostBridge.h"
#include "SnmpLog.h" // for SnmpLog... macros
#include "BcmSnmpAgent.h"


/*\
 *  ocStbHostMibModuleGroup group
\*/

ocStbHostMibModuleGroup::ocStbHostMibModuleGroup ()
  : BcmMibGroup (kOID_ocStbHostMibModuleGroup, kNAME_ocStbHostMibModuleGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostMibModuleGroup::Populate ()
{
  BcmMibGroup *pGroup;


  if (BcmMibGroup::Populate() == false)
    return false;

  pGroup = new ocStbHostMibObjectsGroup();
  AddGroup (pGroup);

  return true;
}

/*\
 *  ocStbHostMibObjectsGroup group
\*/

ocStbHostMibObjectsGroup::ocStbHostMibObjectsGroup ()
  : BcmMibGroup (kOID_ocStbHostMibObjectsGroup, kNAME_ocStbHostMibObjectsGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostMibObjectsGroup::Populate ()
{
  BcmMibGroup *pGroup;


  if (BcmMibGroup::Populate() == false)
    return false;

  pGroup = new ocStbHostSystemGroup();
  AddGroup (pGroup);

  pGroup = new ocStbHostSecuritySubSystemGroup();
  AddGroup (pGroup);

  pGroup = new ocStbHostSoftwareGroup();
  AddGroup (pGroup);

  pGroup = new ocStbHostStatusGroup();
  AddGroup (pGroup);

  return true;
}

/*\
 *  ocStbHostSystemGroup group
\*/

ocStbHostSystemGroup::ocStbHostSystemGroup ()
  : BcmMibGroup (kOID_ocStbHostSystemGroup, kNAME_ocStbHostSystemGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostSystemGroup::Populate ()
{
  BcmMibGroup *pGroup;


  if (BcmMibGroup::Populate() == false)
    return false;

  pGroup = new ocStbHostHWIdentifiersGroup();
  AddGroup (pGroup);

  pGroup = new ocStbHostInterfacesGroup();
  AddGroup (pGroup);

  pGroup = new ocStbHostEasObjectsGroup();
  AddGroup (pGroup);

  return true;
}

/*\
 *  ocStbHostHWIdentifiersGroup group
\*/

ocStbHostHWIdentifiersGroup::ocStbHostHWIdentifiersGroup ()
  : BcmMibGroup (kOID_ocStbHostHWIdentifiersGroup, kNAME_ocStbHostHWIdentifiersGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostHWIdentifiersGroup::Populate ()
{
  BcmMibObject *pScalar;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new StringMibObject (kOID_ocStbHostSerialNumber, kNAME_ocStbHostSerialNumber, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new StringMibObject (kOID_ocStbHostHostID, kNAME_ocStbHostHostID, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostCapabilities, kNAME_ocStbHostCapabilities, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostAvcSupport, kNAME_ocStbHostAvcSupport, this, kReadOnly);
  AddScalar (pScalar);

  return true;
}

SNMP_STATUS ocStbHostHWIdentifiersGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ocStbHostHWIdentifiersGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ocStbHostHWIdentifiersGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostSerialNumber:
      fpBridge->Get_ocStbHostSerialNumber (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_ocStbHostHostID:
      fpBridge->Get_ocStbHostHostID (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_ocStbHostCapabilities:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostCapabilities ());
      break;

    case kLEAF_ocStbHostAvcSupport:
      Status = VB.SetBool (fpBridge->Get_ocStbHostAvcSupport ());
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

/*\
 *  ocStbHostInterfacesGroup group
\*/

ocStbHostInterfacesGroup::ocStbHostInterfacesGroup ()
  : BcmMibGroup (kOID_ocStbHostInterfacesGroup, kNAME_ocStbHostInterfacesGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostInterfacesGroup::Populate ()
{
  BcmMibTable *pTable;
  BcmMibGroup *pGroup;


  if (BcmMibGroup::Populate() == false)
    return false;

  pTable = new ocStbHostAVInterfaceTable();
  AddTable (pTable);

  pTable = new ocStbHostSPDIfTable();
  AddTable (pTable);

  pGroup = new ocStbHostIEEE1394ObjectsGroup();
  AddGroup (pGroup);

  pGroup = new ocStbHostDigitalVideoOutputGroup();
  AddGroup (pGroup);

  pGroup = new ocStbHostAnalogOutputGroup();
  AddGroup (pGroup);

  pGroup = new ocStbHostServiceProgramInfoGroup();
  AddGroup (pGroup);

  pGroup = new ocStbHostQpskObjectsGroup();
  AddGroup (pGroup);

  return true;
}


/*\ 
 *  ocStbHostAVInterfaceTable and related objects 
\*/

/* ocStbHostAVInterfaceTable */

ocStbHostAVInterfaceTable::ocStbHostAVInterfaceTable ()
  : BcmMibTable (kOID_ocStbHostAVInterfaceTable, kNAME_ocStbHostAVInterfaceTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ocStbHostAVInterfaceEntry::kLEAF_ocStbHostAVInterfaceType);
  fColumnMask |= (1 << ocStbHostAVInterfaceEntry::kLEAF_ocStbHostAVInterfaceDesc);
  fColumnMask |= (1 << ocStbHostAVInterfaceEntry::kLEAF_ocStbHostAVInterfaceStatus);
}

SNMP_STATUS ocStbHostAVInterfaceTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ocStbHostAVInterfaceEntry *pEntry;
  ocStbHostAVInterfaceEntryBridge *pMyBridge = (ocStbHostAVInterfaceEntryBridge *) pBridge;


  pEntry = new ocStbHostAVInterfaceEntry (this, pMyBridge);
  Status = AddEntry (pEntry);

  if (Status == SNMP_NO_ERROR)
  {
    // Remember bridge for destruction by base class later if specified.
    Status = pEntry->RegisterBridge (pBridge, DeleteUponDestruction);
  }
  else
    delete pEntry;

  return Status;
}

/* ocStbHostAVInterfaceEntry */

ocStbHostAVInterfaceEntry::ocStbHostAVInterfaceEntry (ocStbHostAVInterfaceTable *pTable, ocStbHostAVInterfaceEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ocStbHostAVInterfaceEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Uint32MibObject (kOID_ocStbHostAVInterfaceIndex, kNAME_ocStbHostAVInterfaceIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new OidMibObject (kOID_ocStbHostAVInterfaceType, kNAME_ocStbHostAVInterfaceType, this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_ocStbHostAVInterfaceDesc, kNAME_ocStbHostAVInterfaceDesc, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostAVInterfaceStatus, kNAME_ocStbHostAVInterfaceStatus, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ocStbHostAVInterfaceEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmObjectId Oid;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostAVInterfaceIndex:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostAVInterfaceIndex (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostAVInterfaceType:
      fpBridge->Get_ocStbHostAVInterfaceType (Oid);
      Status = VB.SetOid (Oid);
      break;

    case kLEAF_ocStbHostAVInterfaceDesc:
    {
      BcmString String;

      fpBridge->Get_ocStbHostAVInterfaceDesc (String);
      Status = VB.SetString (String);
      break;
    }

    case kLEAF_ocStbHostAVInterfaceStatus:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostAVInterfaceStatus ());
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}


/*\ 
 *  ocStbHostSPDIfTable and related objects 
\*/

/* ocStbHostSPDIfTable */

ocStbHostSPDIfTable::ocStbHostSPDIfTable ()
  : BcmMibTable (kOID_ocStbHostSPDIfTable, kNAME_ocStbHostSPDIfTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ocStbHostSPDIfEntry::kLEAF_ocStbHostSPDIfAudioFormat);
  fColumnMask |= (1 << ocStbHostSPDIfEntry::kLEAF_ocStbHostSPDIfAudioMuteStatus);
}

SNMP_STATUS ocStbHostSPDIfTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ocStbHostSPDIfEntry *pEntry;
  ocStbHostSPDIfEntryBridge *pMyBridge = (ocStbHostSPDIfEntryBridge *) pBridge;


  pEntry = new ocStbHostSPDIfEntry (this, pMyBridge);
  Status = AddEntry (pEntry);

  if (Status == SNMP_NO_ERROR)
  {
    // Remember bridge for destruction by base class later if specified.
    Status = pEntry->RegisterBridge (pBridge, DeleteUponDestruction);
  }
  else
    delete pEntry;

  return Status;
}

/* ocStbHostSPDIfEntry */

ocStbHostSPDIfEntry::ocStbHostSPDIfEntry (ocStbHostSPDIfTable *pTable, ocStbHostSPDIfEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ocStbHostSPDIfEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_ocStbHostSPDIfAudioFormat, kNAME_ocStbHostSPDIfAudioFormat, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostSPDIfAudioMuteStatus, kNAME_ocStbHostSPDIfAudioMuteStatus, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ocStbHostSPDIfEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostSPDIfAudioFormat:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostSPDIfAudioFormat ());
      break;

    case kLEAF_ocStbHostSPDIfAudioMuteStatus:
      Status = VB.SetBool (fpBridge->Get_ocStbHostSPDIfAudioMuteStatus ());
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

/*\
 *  ocStbHostIEEE1394ObjectsGroup group
\*/

ocStbHostIEEE1394ObjectsGroup::ocStbHostIEEE1394ObjectsGroup ()
  : BcmMibGroup (kOID_ocStbHostIEEE1394ObjectsGroup, kNAME_ocStbHostIEEE1394ObjectsGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostIEEE1394ObjectsGroup::Populate ()
{
  BcmMibTable *pTable;


  if (BcmMibGroup::Populate() == false)
    return false;

  pTable = new ocStbHostIEEE1394Table();
  AddTable (pTable);

  pTable = new ocStbHostIEEE1394ConnectedDevicesTable();
  AddTable (pTable);

  return true;
}


/*\ 
 *  ocStbHostIEEE1394Table and related objects 
\*/

/* ocStbHostIEEE1394Table */

ocStbHostIEEE1394Table::ocStbHostIEEE1394Table ()
  : BcmMibTable (kOID_ocStbHostIEEE1394Table, kNAME_ocStbHostIEEE1394Table)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ocStbHostIEEE1394Entry::kLEAF_ocStbHostIEEE1394ActiveNodes);
  fColumnMask |= (1 << ocStbHostIEEE1394Entry::kLEAF_ocStbHostIEEE1394DataXMission);
  fColumnMask |= (1 << ocStbHostIEEE1394Entry::kLEAF_ocStbHostIEEE1394DTCPStatus);
  fColumnMask |= (1 << ocStbHostIEEE1394Entry::kLEAF_ocStbHostIEEE1394LoopStatus);
  fColumnMask |= (1 << ocStbHostIEEE1394Entry::kLEAF_ocStbHostIEEE1394RootStatus);
  fColumnMask |= (1 << ocStbHostIEEE1394Entry::kLEAF_ocStbHostIEEE1394CycleIsMaster);
  fColumnMask |= (1 << ocStbHostIEEE1394Entry::kLEAF_ocStbHostIEEE1394IRMStatus);
  fColumnMask |= (1 << ocStbHostIEEE1394Entry::kLEAF_ocStbHostIEEE1394AudioMuteStatus);
  fColumnMask |= (1 << ocStbHostIEEE1394Entry::kLEAF_ocStbHostIEEE1394VideoMuteStatus);
}

SNMP_STATUS ocStbHostIEEE1394Table::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ocStbHostIEEE1394Entry *pEntry;
  ocStbHostIEEE1394EntryBridge *pMyBridge = (ocStbHostIEEE1394EntryBridge *) pBridge;


  pEntry = new ocStbHostIEEE1394Entry (this, pMyBridge);
  Status = AddEntry (pEntry);

  if (Status == SNMP_NO_ERROR)
  {
    // Remember bridge for destruction by base class later if specified.
    Status = pEntry->RegisterBridge (pBridge, DeleteUponDestruction);
  }
  else
    delete pEntry;

  return Status;
}

/* ocStbHostIEEE1394Entry */

ocStbHostIEEE1394Entry::ocStbHostIEEE1394Entry (ocStbHostIEEE1394Table *pTable, ocStbHostIEEE1394EntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ocStbHostIEEE1394Entry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_ocStbHostIEEE1394ActiveNodes, kNAME_ocStbHostIEEE1394ActiveNodes, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostIEEE1394DataXMission, kNAME_ocStbHostIEEE1394DataXMission, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostIEEE1394DTCPStatus, kNAME_ocStbHostIEEE1394DTCPStatus, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostIEEE1394LoopStatus, kNAME_ocStbHostIEEE1394LoopStatus, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostIEEE1394RootStatus, kNAME_ocStbHostIEEE1394RootStatus, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostIEEE1394CycleIsMaster, kNAME_ocStbHostIEEE1394CycleIsMaster, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostIEEE1394IRMStatus, kNAME_ocStbHostIEEE1394IRMStatus, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostIEEE1394AudioMuteStatus, kNAME_ocStbHostIEEE1394AudioMuteStatus, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostIEEE1394VideoMuteStatus, kNAME_ocStbHostIEEE1394VideoMuteStatus, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ocStbHostIEEE1394Entry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostIEEE1394ActiveNodes:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostIEEE1394ActiveNodes ());
      break;

    case kLEAF_ocStbHostIEEE1394DataXMission:
      Status = VB.SetBool (fpBridge->Get_ocStbHostIEEE1394DataXMission ());
      break;

    case kLEAF_ocStbHostIEEE1394DTCPStatus:
      Status = VB.SetBool (fpBridge->Get_ocStbHostIEEE1394DTCPStatus ());
      break;

    case kLEAF_ocStbHostIEEE1394LoopStatus:
      Status = VB.SetBool (fpBridge->Get_ocStbHostIEEE1394LoopStatus ());
      break;

    case kLEAF_ocStbHostIEEE1394RootStatus:
      Status = VB.SetBool (fpBridge->Get_ocStbHostIEEE1394RootStatus ());
      break;

    case kLEAF_ocStbHostIEEE1394CycleIsMaster:
      Status = VB.SetBool (fpBridge->Get_ocStbHostIEEE1394CycleIsMaster ());
      break;

    case kLEAF_ocStbHostIEEE1394IRMStatus:
      Status = VB.SetBool (fpBridge->Get_ocStbHostIEEE1394IRMStatus ());
      break;

    case kLEAF_ocStbHostIEEE1394AudioMuteStatus:
      Status = VB.SetBool (fpBridge->Get_ocStbHostIEEE1394AudioMuteStatus ());
      break;

    case kLEAF_ocStbHostIEEE1394VideoMuteStatus:
      Status = VB.SetBool (fpBridge->Get_ocStbHostIEEE1394VideoMuteStatus ());
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}


/*\ 
 *  ocStbHostIEEE1394ConnectedDevicesTable and related objects 
\*/

/* ocStbHostIEEE1394ConnectedDevicesTable */

ocStbHostIEEE1394ConnectedDevicesTable::ocStbHostIEEE1394ConnectedDevicesTable ()
  : BcmMibTable (kOID_ocStbHostIEEE1394ConnectedDevicesTable, kNAME_ocStbHostIEEE1394ConnectedDevicesTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ocStbHostIEEE1394ConnectedDevicesEntry::kLEAF_ocStbHostIEEE1394ConnectedDevicesAVInterfaceIndex);
  fColumnMask |= (1 << ocStbHostIEEE1394ConnectedDevicesEntry::kLEAF_ocStbHostIEEE1394ConnectedDevicesSubUnitType);
  fColumnMask |= (1 << ocStbHostIEEE1394ConnectedDevicesEntry::kLEAF_ocStbHostIEEE1394ConnectedDevicesEui64);
  fColumnMask |= (1 << ocStbHostIEEE1394ConnectedDevicesEntry::kLEAF_ocStbHostIEEE1394ConnectedDevicesADSourceSelectSupport);
}

SNMP_STATUS ocStbHostIEEE1394ConnectedDevicesTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ocStbHostIEEE1394ConnectedDevicesEntry *pEntry;
  ocStbHostIEEE1394ConnectedDevicesEntryBridge *pMyBridge = (ocStbHostIEEE1394ConnectedDevicesEntryBridge *) pBridge;


  pEntry = new ocStbHostIEEE1394ConnectedDevicesEntry (this, pMyBridge);
  Status = AddEntry (pEntry);

  if (Status == SNMP_NO_ERROR)
  {
    // Remember bridge for destruction by base class later if specified.
    Status = pEntry->RegisterBridge (pBridge, DeleteUponDestruction);
  }
  else
    delete pEntry;

  return Status;
}

/* ocStbHostIEEE1394ConnectedDevicesEntry */

ocStbHostIEEE1394ConnectedDevicesEntry::ocStbHostIEEE1394ConnectedDevicesEntry (ocStbHostIEEE1394ConnectedDevicesTable *pTable, ocStbHostIEEE1394ConnectedDevicesEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ocStbHostIEEE1394ConnectedDevicesEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Uint32MibObject (kOID_ocStbHostIEEE1394ConnectedDevicesIndex, kNAME_ocStbHostIEEE1394ConnectedDevicesIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostIEEE1394ConnectedDevicesAVInterfaceIndex, kNAME_ocStbHostIEEE1394ConnectedDevicesAVInterfaceIndex, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostIEEE1394ConnectedDevicesSubUnitType, kNAME_ocStbHostIEEE1394ConnectedDevicesSubUnitType, this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_ocStbHostIEEE1394ConnectedDevicesEui64, kNAME_ocStbHostIEEE1394ConnectedDevicesEui64, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostIEEE1394ConnectedDevicesADSourceSelectSupport, kNAME_ocStbHostIEEE1394ConnectedDevicesADSourceSelectSupport, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ocStbHostIEEE1394ConnectedDevicesEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostIEEE1394ConnectedDevicesIndex:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostIEEE1394ConnectedDevicesIndex (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostIEEE1394ConnectedDevicesAVInterfaceIndex:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostIEEE1394ConnectedDevicesAVInterfaceIndex (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostIEEE1394ConnectedDevicesSubUnitType:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostIEEE1394ConnectedDevicesSubUnitType ());
      break;

    case kLEAF_ocStbHostIEEE1394ConnectedDevicesEui64:
    {
      BcmString String;

      fpBridge->Get_ocStbHostIEEE1394ConnectedDevicesEui64 (String);
      Status = VB.SetString (String);
      break;
    }

    case kLEAF_ocStbHostIEEE1394ConnectedDevicesADSourceSelectSupport:
      Status = VB.SetBool (fpBridge->Get_ocStbHostIEEE1394ConnectedDevicesADSourceSelectSupport ());
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

/*\
 *  ocStbHostDigitalVideoOutputGroup group
\*/

ocStbHostDigitalVideoOutputGroup::ocStbHostDigitalVideoOutputGroup ()
  : BcmMibGroup (kOID_ocStbHostDigitalVideoOutputGroup, kNAME_ocStbHostDigitalVideoOutputGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostDigitalVideoOutputGroup::Populate ()
{
  BcmMibTable *pTable;


  if (BcmMibGroup::Populate() == false)
    return false;

  pTable = new ocStbHostDVIHDMITable();
  AddTable (pTable);

  pTable = new ocStbHostDVIHDMIAvailableVideoFormatTable();
  AddTable (pTable);

  return true;
}


/*\ 
 *  ocStbHostDVIHDMITable and related objects 
\*/

/* ocStbHostDVIHDMITable */

ocStbHostDVIHDMITable::ocStbHostDVIHDMITable ()
  : BcmMibTable (kOID_ocStbHostDVIHDMITable, kNAME_ocStbHostDVIHDMITable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMIOutputType);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMIConnectionStatus);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMIRepeaterStatus);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMIVideoXmissionStatus);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMIHDCPStatus);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMIVideoMuteStatus);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMIOutputFormat);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMIAspectRatio);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMIHostDeviceHDCPStatus);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMIAudioFormat);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMIAudioSampleRate);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMIAudioChannelCount);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMIAudioMuteStatus);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMIAudioSampleSize);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMIColorSpace);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMIFrameRate);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMIAttachedDeviceType);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMIEdid);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMILipSyncDelay);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMICecFeatures);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMIFeatures);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMIMaxDeviceCount);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMIPreferredVideoFormat);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMIEdidVersion);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMI3DCompatibilityControl);
  fColumnMask |= (1 << ocStbHostDVIHDMIEntry::kLEAF_ocStbHostDVIHDMI3DCompatibilityMsgDisplay);
}

SNMP_STATUS ocStbHostDVIHDMITable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ocStbHostDVIHDMIEntry *pEntry;
  ocStbHostDVIHDMIEntryBridge *pMyBridge = (ocStbHostDVIHDMIEntryBridge *) pBridge;


  pEntry = new ocStbHostDVIHDMIEntry (this, pMyBridge);
  Status = AddEntry (pEntry);

  if (Status == SNMP_NO_ERROR)
  {
    // Remember bridge for destruction by base class later if specified.
    Status = pEntry->RegisterBridge (pBridge, DeleteUponDestruction);
  }
  else
    delete pEntry;

  return Status;
}

/* ocStbHostDVIHDMIEntry */

ocStbHostDVIHDMIEntry::ocStbHostDVIHDMIEntry (ocStbHostDVIHDMITable *pTable, ocStbHostDVIHDMIEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ocStbHostDVIHDMIEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMIOutputType, kNAME_ocStbHostDVIHDMIOutputType, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMIConnectionStatus, kNAME_ocStbHostDVIHDMIConnectionStatus, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMIRepeaterStatus, kNAME_ocStbHostDVIHDMIRepeaterStatus, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMIVideoXmissionStatus, kNAME_ocStbHostDVIHDMIVideoXmissionStatus, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMIHDCPStatus, kNAME_ocStbHostDVIHDMIHDCPStatus, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMIVideoMuteStatus, kNAME_ocStbHostDVIHDMIVideoMuteStatus, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMIOutputFormat, kNAME_ocStbHostDVIHDMIOutputFormat, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMIAspectRatio, kNAME_ocStbHostDVIHDMIAspectRatio, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMIHostDeviceHDCPStatus, kNAME_ocStbHostDVIHDMIHostDeviceHDCPStatus, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMIAudioFormat, kNAME_ocStbHostDVIHDMIAudioFormat, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMIAudioSampleRate, kNAME_ocStbHostDVIHDMIAudioSampleRate, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostDVIHDMIAudioChannelCount, kNAME_ocStbHostDVIHDMIAudioChannelCount, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMIAudioMuteStatus, kNAME_ocStbHostDVIHDMIAudioMuteStatus, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMIAudioSampleSize, kNAME_ocStbHostDVIHDMIAudioSampleSize, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMIColorSpace, kNAME_ocStbHostDVIHDMIColorSpace, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMIFrameRate, kNAME_ocStbHostDVIHDMIFrameRate, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMIAttachedDeviceType, kNAME_ocStbHostDVIHDMIAttachedDeviceType, this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_ocStbHostDVIHDMIEdid, kNAME_ocStbHostDVIHDMIEdid, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMILipSyncDelay, kNAME_ocStbHostDVIHDMILipSyncDelay, this, kReadOnly);
  AddObject (pObject);

  pObject = new BitsMibObject (kOID_ocStbHostDVIHDMICecFeatures, kNAME_ocStbHostDVIHDMICecFeatures, this, kReadOnly);
  AddObject (pObject);

  pObject = new BitsMibObject (kOID_ocStbHostDVIHDMIFeatures, kNAME_ocStbHostDVIHDMIFeatures, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMIMaxDeviceCount, kNAME_ocStbHostDVIHDMIMaxDeviceCount, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMIPreferredVideoFormat, kNAME_ocStbHostDVIHDMIPreferredVideoFormat, this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_ocStbHostDVIHDMIEdidVersion, kNAME_ocStbHostDVIHDMIEdidVersion, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMI3DCompatibilityControl, kNAME_ocStbHostDVIHDMI3DCompatibilityControl, this, kReadWrite);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMI3DCompatibilityMsgDisplay, kNAME_ocStbHostDVIHDMI3DCompatibilityMsgDisplay, this, kReadWrite);
  AddObject (pObject);
}

SNMP_STATUS ocStbHostDVIHDMIEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostDVIHDMIOutputType:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostDVIHDMIOutputType ());
      break;

    case kLEAF_ocStbHostDVIHDMIConnectionStatus:
      Status = VB.SetBool (fpBridge->Get_ocStbHostDVIHDMIConnectionStatus ());
      break;

    case kLEAF_ocStbHostDVIHDMIRepeaterStatus:
      Status = VB.SetBool (fpBridge->Get_ocStbHostDVIHDMIRepeaterStatus ());
      break;

    case kLEAF_ocStbHostDVIHDMIVideoXmissionStatus:
      Status = VB.SetBool (fpBridge->Get_ocStbHostDVIHDMIVideoXmissionStatus ());
      break;

    case kLEAF_ocStbHostDVIHDMIHDCPStatus:
      Status = VB.SetBool (fpBridge->Get_ocStbHostDVIHDMIHDCPStatus ());
      break;

    case kLEAF_ocStbHostDVIHDMIVideoMuteStatus:
      Status = VB.SetBool (fpBridge->Get_ocStbHostDVIHDMIVideoMuteStatus ());
      break;

    case kLEAF_ocStbHostDVIHDMIOutputFormat:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostDVIHDMIOutputFormat ());
      break;

    case kLEAF_ocStbHostDVIHDMIAspectRatio:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostDVIHDMIAspectRatio ());
      break;

    case kLEAF_ocStbHostDVIHDMIHostDeviceHDCPStatus:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostDVIHDMIHostDeviceHDCPStatus ());
      break;

    case kLEAF_ocStbHostDVIHDMIAudioFormat:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostDVIHDMIAudioFormat ());
      break;

    case kLEAF_ocStbHostDVIHDMIAudioSampleRate:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostDVIHDMIAudioSampleRate ());
      break;

    case kLEAF_ocStbHostDVIHDMIAudioChannelCount:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostDVIHDMIAudioChannelCount (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostDVIHDMIAudioMuteStatus:
      Status = VB.SetBool (fpBridge->Get_ocStbHostDVIHDMIAudioMuteStatus ());
      break;

    case kLEAF_ocStbHostDVIHDMIAudioSampleSize:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostDVIHDMIAudioSampleSize ());
      break;

    case kLEAF_ocStbHostDVIHDMIColorSpace:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostDVIHDMIColorSpace ());
      break;

    case kLEAF_ocStbHostDVIHDMIFrameRate:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostDVIHDMIFrameRate ());
      break;

    case kLEAF_ocStbHostDVIHDMIAttachedDeviceType:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostDVIHDMIAttachedDeviceType ());
      break;

    case kLEAF_ocStbHostDVIHDMIEdid:
      fpBridge->Get_ocStbHostDVIHDMIEdid (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_ocStbHostDVIHDMILipSyncDelay:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostDVIHDMILipSyncDelay ());
      break;

    case kLEAF_ocStbHostDVIHDMICecFeatures:
      // UNFINISHED: Make sure the number of octets argument is correct
      Status = VB.SetBits (fpBridge->Get_ocStbHostDVIHDMICecFeatures (), 2);
      break;

    case kLEAF_ocStbHostDVIHDMIFeatures:
      // UNFINISHED: Make sure the number of octets argument is correct
      Status = VB.SetBits (fpBridge->Get_ocStbHostDVIHDMIFeatures (), 1);
      break;

    case kLEAF_ocStbHostDVIHDMIMaxDeviceCount:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostDVIHDMIMaxDeviceCount ());
      break;

    case kLEAF_ocStbHostDVIHDMIPreferredVideoFormat:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostDVIHDMIPreferredVideoFormat ());
      break;

    case kLEAF_ocStbHostDVIHDMIEdidVersion:
      fpBridge->Get_ocStbHostDVIHDMIEdidVersion (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_ocStbHostDVIHDMI3DCompatibilityControl:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostDVIHDMI3DCompatibilityControl ());
      break;

    case kLEAF_ocStbHostDVIHDMI3DCompatibilityMsgDisplay:
      Status = VB.SetBool (fpBridge->Get_ocStbHostDVIHDMI3DCompatibilityMsgDisplay ());
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

SNMP_STATUS ocStbHostDVIHDMIEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostDVIHDMIOutputType:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMIConnectionStatus:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMIRepeaterStatus:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMIVideoXmissionStatus:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMIHDCPStatus:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMIVideoMuteStatus:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMIOutputFormat:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMIAspectRatio:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMIHostDeviceHDCPStatus:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMIAudioFormat:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMIAudioSampleRate:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMIAudioChannelCount:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMIAudioMuteStatus:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMIAudioSampleSize:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMIColorSpace:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMIFrameRate:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMIAttachedDeviceType:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMIEdid:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMILipSyncDelay:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMICecFeatures:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMIFeatures:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMIMaxDeviceCount:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMIPreferredVideoFormat:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMIEdidVersion:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostDVIHDMI3DCompatibilityControl:
    {
      int ocStbHostDVIHDMI3DCompatibilityControl;

      Status = VB.GetInt32 (&ocStbHostDVIHDMI3DCompatibilityControl);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ocStbHostDVIHDMI3DCompatibilityControl (ocStbHostDVIHDMI3DCompatibilityControl);
      break;
    }

    case kLEAF_ocStbHostDVIHDMI3DCompatibilityMsgDisplay:
    {
      bool ocStbHostDVIHDMI3DCompatibilityMsgDisplay;

      Status = VB.GetBool (&ocStbHostDVIHDMI3DCompatibilityMsgDisplay);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ocStbHostDVIHDMI3DCompatibilityMsgDisplay (ocStbHostDVIHDMI3DCompatibilityMsgDisplay);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}


/*\ 
 *  ocStbHostDVIHDMIAvailableVideoFormatTable and related objects 
\*/

/* ocStbHostDVIHDMIAvailableVideoFormatTable */

ocStbHostDVIHDMIAvailableVideoFormatTable::ocStbHostDVIHDMIAvailableVideoFormatTable ()
  : BcmMibTable (kOID_ocStbHostDVIHDMIAvailableVideoFormatTable, kNAME_ocStbHostDVIHDMIAvailableVideoFormatTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ocStbHostDVIHDMIAvailableVideoFormatEntry::kLEAF_ocStbHostDVIHDMIAvailableVideoFormat);
  fColumnMask |= (1 << ocStbHostDVIHDMIAvailableVideoFormatEntry::kLEAF_ocStbHostDVIHDMISupported3DStructures);
  fColumnMask |= (1 << ocStbHostDVIHDMIAvailableVideoFormatEntry::kLEAF_ocStbHostDVIHDMIActive3DStructure);
}

SNMP_STATUS ocStbHostDVIHDMIAvailableVideoFormatTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ocStbHostDVIHDMIAvailableVideoFormatEntry *pEntry;
  ocStbHostDVIHDMIAvailableVideoFormatEntryBridge *pMyBridge = (ocStbHostDVIHDMIAvailableVideoFormatEntryBridge *) pBridge;


  pEntry = new ocStbHostDVIHDMIAvailableVideoFormatEntry (this, pMyBridge);
  Status = AddEntry (pEntry);

  if (Status == SNMP_NO_ERROR)
  {
    // Remember bridge for destruction by base class later if specified.
    Status = pEntry->RegisterBridge (pBridge, DeleteUponDestruction);
  }
  else
    delete pEntry;

  return Status;
}

/* ocStbHostDVIHDMIAvailableVideoFormatEntry */

ocStbHostDVIHDMIAvailableVideoFormatEntry::ocStbHostDVIHDMIAvailableVideoFormatEntry (ocStbHostDVIHDMIAvailableVideoFormatTable *pTable, ocStbHostDVIHDMIAvailableVideoFormatEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ocStbHostDVIHDMIAvailableVideoFormatEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMIAvailableVideoFormatIndex, kNAME_ocStbHostDVIHDMIAvailableVideoFormatIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMIAvailableVideoFormat, kNAME_ocStbHostDVIHDMIAvailableVideoFormat, this, kReadOnly);
  AddObject (pObject);

  pObject = new BitsMibObject (kOID_ocStbHostDVIHDMISupported3DStructures, kNAME_ocStbHostDVIHDMISupported3DStructures, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostDVIHDMIActive3DStructure, kNAME_ocStbHostDVIHDMIActive3DStructure, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ocStbHostDVIHDMIAvailableVideoFormatEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostDVIHDMIAvailableVideoFormatIndex:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostDVIHDMIAvailableVideoFormatIndex ());
      break;

    case kLEAF_ocStbHostDVIHDMIAvailableVideoFormat:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostDVIHDMIAvailableVideoFormat ());
      break;

    case kLEAF_ocStbHostDVIHDMISupported3DStructures:
      // UNFINISHED: Make sure the number of octets argument is correct
      Status = VB.SetBits (fpBridge->Get_ocStbHostDVIHDMISupported3DStructures (), 2);
      break;

    case kLEAF_ocStbHostDVIHDMIActive3DStructure:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostDVIHDMIActive3DStructure ());
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

/*\
 *  ocStbHostAnalogOutputGroup group
\*/

ocStbHostAnalogOutputGroup::ocStbHostAnalogOutputGroup ()
  : BcmMibGroup (kOID_ocStbHostAnalogOutputGroup, kNAME_ocStbHostAnalogOutputGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostAnalogOutputGroup::Populate ()
{
  BcmMibTable *pTable;


  if (BcmMibGroup::Populate() == false)
    return false;

  pTable = new ocStbHostComponentVideoTable();
  AddTable (pTable);

  pTable = new ocStbHostRFChannelOutTable();
  AddTable (pTable);

  return true;
}


/*\ 
 *  ocStbHostComponentVideoTable and related objects 
\*/

/* ocStbHostComponentVideoTable */

ocStbHostComponentVideoTable::ocStbHostComponentVideoTable ()
  : BcmMibTable (kOID_ocStbHostComponentVideoTable, kNAME_ocStbHostComponentVideoTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ocStbHostComponentVideoEntry::kLEAF_ocStbHostComponentVideoConstrainedStatus);
  fColumnMask |= (1 << ocStbHostComponentVideoEntry::kLEAF_ocStbHostComponentOutputFormat);
  fColumnMask |= (1 << ocStbHostComponentVideoEntry::kLEAF_ocStbHostComponentAspectRatio);
  fColumnMask |= (1 << ocStbHostComponentVideoEntry::kLEAF_ocStbHostComponentVideoMuteStatus);
}

SNMP_STATUS ocStbHostComponentVideoTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ocStbHostComponentVideoEntry *pEntry;
  ocStbHostComponentVideoEntryBridge *pMyBridge = (ocStbHostComponentVideoEntryBridge *) pBridge;


  pEntry = new ocStbHostComponentVideoEntry (this, pMyBridge);
  Status = AddEntry (pEntry);

  if (Status == SNMP_NO_ERROR)
  {
    // Remember bridge for destruction by base class later if specified.
    Status = pEntry->RegisterBridge (pBridge, DeleteUponDestruction);
  }
  else
    delete pEntry;

  return Status;
}

/* ocStbHostComponentVideoEntry */

ocStbHostComponentVideoEntry::ocStbHostComponentVideoEntry (ocStbHostComponentVideoTable *pTable, ocStbHostComponentVideoEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ocStbHostComponentVideoEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_ocStbHostComponentVideoConstrainedStatus, kNAME_ocStbHostComponentVideoConstrainedStatus, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostComponentOutputFormat, kNAME_ocStbHostComponentOutputFormat, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostComponentAspectRatio, kNAME_ocStbHostComponentAspectRatio, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostComponentVideoMuteStatus, kNAME_ocStbHostComponentVideoMuteStatus, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ocStbHostComponentVideoEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostComponentVideoConstrainedStatus:
      Status = VB.SetBool (fpBridge->Get_ocStbHostComponentVideoConstrainedStatus ());
      break;

    case kLEAF_ocStbHostComponentOutputFormat:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostComponentOutputFormat ());
      break;

    case kLEAF_ocStbHostComponentAspectRatio:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostComponentAspectRatio ());
      break;

    case kLEAF_ocStbHostComponentVideoMuteStatus:
      Status = VB.SetBool (fpBridge->Get_ocStbHostComponentVideoMuteStatus ());
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}


/*\ 
 *  ocStbHostRFChannelOutTable and related objects 
\*/

/* ocStbHostRFChannelOutTable */

ocStbHostRFChannelOutTable::ocStbHostRFChannelOutTable ()
  : BcmMibTable (kOID_ocStbHostRFChannelOutTable, kNAME_ocStbHostRFChannelOutTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ocStbHostRFChannelOutEntry::kLEAF_ocStbHostRFChannelOut);
  fColumnMask |= (1 << ocStbHostRFChannelOutEntry::kLEAF_ocStbHostRFChannelOutAudioMuteStatus);
  fColumnMask |= (1 << ocStbHostRFChannelOutEntry::kLEAF_ocStbHostRFChannelOutVideoMuteStatus);
}

SNMP_STATUS ocStbHostRFChannelOutTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ocStbHostRFChannelOutEntry *pEntry;
  ocStbHostRFChannelOutEntryBridge *pMyBridge = (ocStbHostRFChannelOutEntryBridge *) pBridge;


  pEntry = new ocStbHostRFChannelOutEntry (this, pMyBridge);
  Status = AddEntry (pEntry);

  if (Status == SNMP_NO_ERROR)
  {
    // Remember bridge for destruction by base class later if specified.
    Status = pEntry->RegisterBridge (pBridge, DeleteUponDestruction);
  }
  else
    delete pEntry;

  return Status;
}

/* ocStbHostRFChannelOutEntry */

ocStbHostRFChannelOutEntry::ocStbHostRFChannelOutEntry (ocStbHostRFChannelOutTable *pTable, ocStbHostRFChannelOutEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ocStbHostRFChannelOutEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Uint32MibObject (kOID_ocStbHostRFChannelOut, kNAME_ocStbHostRFChannelOut, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostRFChannelOutAudioMuteStatus, kNAME_ocStbHostRFChannelOutAudioMuteStatus, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostRFChannelOutVideoMuteStatus, kNAME_ocStbHostRFChannelOutVideoMuteStatus, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ocStbHostRFChannelOutEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostRFChannelOut:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostRFChannelOut (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostRFChannelOutAudioMuteStatus:
      Status = VB.SetBool (fpBridge->Get_ocStbHostRFChannelOutAudioMuteStatus ());
      break;

    case kLEAF_ocStbHostRFChannelOutVideoMuteStatus:
      Status = VB.SetBool (fpBridge->Get_ocStbHostRFChannelOutVideoMuteStatus ());
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

/*\
 *  ocStbHostServiceProgramInfoGroup group
\*/

ocStbHostServiceProgramInfoGroup::ocStbHostServiceProgramInfoGroup ()
  : BcmMibGroup (kOID_ocStbHostServiceProgramInfoGroup, kNAME_ocStbHostServiceProgramInfoGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostServiceProgramInfoGroup::Populate ()
{
  BcmMibTable *pTable;


  if (BcmMibGroup::Populate() == false)
    return false;

  pTable = new ocStbHostInBandTunerTable();
  AddTable (pTable);

  pTable = new ocStbHostProgramStatusTable();
  AddTable (pTable);

  pTable = new ocStbHostMpeg2ContentTable();
  AddTable (pTable);

  pTable = new ocStbHostAnalogVideoTable();
  AddTable (pTable);

  return true;
}


/*\ 
 *  ocStbHostInBandTunerTable and related objects 
\*/

/* ocStbHostInBandTunerTable */

ocStbHostInBandTunerTable::ocStbHostInBandTunerTable ()
  : BcmMibTable (kOID_ocStbHostInBandTunerTable, kNAME_ocStbHostInBandTunerTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ocStbHostInBandTunerEntry::kLEAF_ocStbHostInBandTunerModulationMode);
  fColumnMask |= (1 << ocStbHostInBandTunerEntry::kLEAF_ocStbHostInBandTunerFrequency);
  fColumnMask |= (1 << ocStbHostInBandTunerEntry::kLEAF_ocStbHostInBandTunerInterleaver);
  fColumnMask |= (1 << ocStbHostInBandTunerEntry::kLEAF_ocStbHostInBandTunerPower);
  fColumnMask |= (1 << ocStbHostInBandTunerEntry::kLEAF_ocStbHostInBandTunerAGCValue);
  fColumnMask |= (1 << ocStbHostInBandTunerEntry::kLEAF_ocStbHostInBandTunerSNRValue);
  fColumnMask |= (1 << ocStbHostInBandTunerEntry::kLEAF_ocStbHostInBandTunerUnerroreds);
  fColumnMask |= (1 << ocStbHostInBandTunerEntry::kLEAF_ocStbHostInBandTunerCorrecteds);
  fColumnMask |= (1 << ocStbHostInBandTunerEntry::kLEAF_ocStbHostInBandTunerUncorrectables);
  fColumnMask |= (1 << ocStbHostInBandTunerEntry::kLEAF_ocStbHostInBandTunerCarrierLockLost);
  fColumnMask |= (1 << ocStbHostInBandTunerEntry::kLEAF_ocStbHostInBandTunerPCRErrors);
  fColumnMask |= (1 << ocStbHostInBandTunerEntry::kLEAF_ocStbHostInBandTunerPTSErrors);
  fColumnMask |= (1 << ocStbHostInBandTunerEntry::kLEAF_ocStbHostInBandTunerState);
  fColumnMask |= (1 << ocStbHostInBandTunerEntry::kLEAF_ocStbHostInBandTunerBER);
  fColumnMask |= (1 << ocStbHostInBandTunerEntry::kLEAF_ocStbHostInBandTunerSecsSinceLock);
  fColumnMask |= (1 << ocStbHostInBandTunerEntry::kLEAF_ocStbHostInBandTunerEqGain);
  fColumnMask |= (1 << ocStbHostInBandTunerEntry::kLEAF_ocStbHostInBandTunerMainTapCoeff);
  fColumnMask |= (1 << ocStbHostInBandTunerEntry::kLEAF_ocStbHostInBandTunerTotalTuneCount);
  fColumnMask |= (1 << ocStbHostInBandTunerEntry::kLEAF_ocStbHostInBandTunerTuneFailureCount);
  fColumnMask |= (1 << ocStbHostInBandTunerEntry::kLEAF_ocStbHostInBandTunerTuneFailFreq);
  fColumnMask |= (1 << ocStbHostInBandTunerEntry::kLEAF_ocStbHostInBandTunerBandwidth);
}

SNMP_STATUS ocStbHostInBandTunerTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ocStbHostInBandTunerEntry *pEntry;
  ocStbHostInBandTunerEntryBridge *pMyBridge = (ocStbHostInBandTunerEntryBridge *) pBridge;


  pEntry = new ocStbHostInBandTunerEntry (this, pMyBridge);
  Status = AddEntry (pEntry);

  if (Status == SNMP_NO_ERROR)
  {
    // Remember bridge for destruction by base class later if specified.
    Status = pEntry->RegisterBridge (pBridge, DeleteUponDestruction);
  }
  else
    delete pEntry;

  return Status;
}

/* ocStbHostInBandTunerEntry */

ocStbHostInBandTunerEntry::ocStbHostInBandTunerEntry (ocStbHostInBandTunerTable *pTable, ocStbHostInBandTunerEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ocStbHostInBandTunerEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_ocStbHostInBandTunerModulationMode, kNAME_ocStbHostInBandTunerModulationMode, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostInBandTunerFrequency, kNAME_ocStbHostInBandTunerFrequency, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostInBandTunerInterleaver, kNAME_ocStbHostInBandTunerInterleaver, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostInBandTunerPower, kNAME_ocStbHostInBandTunerPower, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostInBandTunerAGCValue, kNAME_ocStbHostInBandTunerAGCValue, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostInBandTunerSNRValue, kNAME_ocStbHostInBandTunerSNRValue, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostInBandTunerUnerroreds, kNAME_ocStbHostInBandTunerUnerroreds, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostInBandTunerCorrecteds, kNAME_ocStbHostInBandTunerCorrecteds, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostInBandTunerUncorrectables, kNAME_ocStbHostInBandTunerUncorrectables, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostInBandTunerCarrierLockLost, kNAME_ocStbHostInBandTunerCarrierLockLost, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostInBandTunerPCRErrors, kNAME_ocStbHostInBandTunerPCRErrors, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostInBandTunerPTSErrors, kNAME_ocStbHostInBandTunerPTSErrors, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostInBandTunerState, kNAME_ocStbHostInBandTunerState, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostInBandTunerBER, kNAME_ocStbHostInBandTunerBER, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostInBandTunerSecsSinceLock, kNAME_ocStbHostInBandTunerSecsSinceLock, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostInBandTunerEqGain, kNAME_ocStbHostInBandTunerEqGain, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostInBandTunerMainTapCoeff, kNAME_ocStbHostInBandTunerMainTapCoeff, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostInBandTunerTotalTuneCount, kNAME_ocStbHostInBandTunerTotalTuneCount, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostInBandTunerTuneFailureCount, kNAME_ocStbHostInBandTunerTuneFailureCount, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostInBandTunerTuneFailFreq, kNAME_ocStbHostInBandTunerTuneFailFreq, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostInBandTunerBandwidth, kNAME_ocStbHostInBandTunerBandwidth, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ocStbHostInBandTunerEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostInBandTunerModulationMode:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostInBandTunerModulationMode ());
      break;

    case kLEAF_ocStbHostInBandTunerFrequency:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostInBandTunerFrequency (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostInBandTunerInterleaver:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostInBandTunerInterleaver ());
      break;

    case kLEAF_ocStbHostInBandTunerPower:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostInBandTunerPower ());
      break;

    case kLEAF_ocStbHostInBandTunerAGCValue:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostInBandTunerAGCValue (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostInBandTunerSNRValue:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostInBandTunerSNRValue ());
      break;

    case kLEAF_ocStbHostInBandTunerUnerroreds:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostInBandTunerUnerroreds (), VBT_COUNTER);
      break;

    case kLEAF_ocStbHostInBandTunerCorrecteds:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostInBandTunerCorrecteds (), VBT_COUNTER);
      break;

    case kLEAF_ocStbHostInBandTunerUncorrectables:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostInBandTunerUncorrectables (), VBT_COUNTER);
      break;

    case kLEAF_ocStbHostInBandTunerCarrierLockLost:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostInBandTunerCarrierLockLost (), VBT_COUNTER);
      break;

    case kLEAF_ocStbHostInBandTunerPCRErrors:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostInBandTunerPCRErrors (), VBT_COUNTER);
      break;

    case kLEAF_ocStbHostInBandTunerPTSErrors:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostInBandTunerPTSErrors (), VBT_COUNTER);
      break;

    case kLEAF_ocStbHostInBandTunerState:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostInBandTunerState ());
      break;

    case kLEAF_ocStbHostInBandTunerBER:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostInBandTunerBER ());
      break;

    case kLEAF_ocStbHostInBandTunerSecsSinceLock:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostInBandTunerSecsSinceLock (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostInBandTunerEqGain:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostInBandTunerEqGain ());
      break;

    case kLEAF_ocStbHostInBandTunerMainTapCoeff:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostInBandTunerMainTapCoeff ());
      break;

    case kLEAF_ocStbHostInBandTunerTotalTuneCount:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostInBandTunerTotalTuneCount (), VBT_COUNTER);
      break;

    case kLEAF_ocStbHostInBandTunerTuneFailureCount:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostInBandTunerTuneFailureCount (), VBT_COUNTER);
      break;

    case kLEAF_ocStbHostInBandTunerTuneFailFreq:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostInBandTunerTuneFailFreq (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostInBandTunerBandwidth:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostInBandTunerBandwidth ());
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}


/*\ 
 *  ocStbHostProgramStatusTable and related objects 
\*/

/* ocStbHostProgramStatusTable */

ocStbHostProgramStatusTable::ocStbHostProgramStatusTable ()
  : BcmMibTable (kOID_ocStbHostProgramStatusTable, kNAME_ocStbHostProgramStatusTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ocStbHostProgramStatusEntry::kLEAF_ocStbHostProgramAVSource);
  fColumnMask |= (1 << ocStbHostProgramStatusEntry::kLEAF_ocStbHostProgramAVDestination);
  fColumnMask |= (1 << ocStbHostProgramStatusEntry::kLEAF_ocStbHostProgramContentSource);
  fColumnMask |= (1 << ocStbHostProgramStatusEntry::kLEAF_ocStbHostProgramContentDestination);
}

SNMP_STATUS ocStbHostProgramStatusTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ocStbHostProgramStatusEntry *pEntry;
  ocStbHostProgramStatusEntryBridge *pMyBridge = (ocStbHostProgramStatusEntryBridge *) pBridge;


  pEntry = new ocStbHostProgramStatusEntry (this, pMyBridge);
  Status = AddEntry (pEntry);

  if (Status == SNMP_NO_ERROR)
  {
    // Remember bridge for destruction by base class later if specified.
    Status = pEntry->RegisterBridge (pBridge, DeleteUponDestruction);
  }
  else
    delete pEntry;

  return Status;
}

/* ocStbHostProgramStatusEntry */

ocStbHostProgramStatusEntry::ocStbHostProgramStatusEntry (ocStbHostProgramStatusTable *pTable, ocStbHostProgramStatusEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ocStbHostProgramStatusEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Uint32MibObject (kOID_ocStbHostProgramIndex, kNAME_ocStbHostProgramIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new OidMibObject (kOID_ocStbHostProgramAVSource, kNAME_ocStbHostProgramAVSource, this, kReadOnly);
  AddObject (pObject);

  pObject = new OidMibObject (kOID_ocStbHostProgramAVDestination, kNAME_ocStbHostProgramAVDestination, this, kReadOnly);
  AddObject (pObject);

  pObject = new OidMibObject (kOID_ocStbHostProgramContentSource, kNAME_ocStbHostProgramContentSource, this, kReadOnly);
  AddObject (pObject);

  pObject = new OidMibObject (kOID_ocStbHostProgramContentDestination, kNAME_ocStbHostProgramContentDestination, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ocStbHostProgramStatusEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmObjectId Oid;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostProgramIndex:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostProgramIndex (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostProgramAVSource:
      fpBridge->Get_ocStbHostProgramAVSource (Oid);
      Status = VB.SetOid (Oid);
      break;

    case kLEAF_ocStbHostProgramAVDestination:
      fpBridge->Get_ocStbHostProgramAVDestination (Oid);
      Status = VB.SetOid (Oid);
      break;

    case kLEAF_ocStbHostProgramContentSource:
      fpBridge->Get_ocStbHostProgramContentSource (Oid);
      Status = VB.SetOid (Oid);
      break;

    case kLEAF_ocStbHostProgramContentDestination:
      fpBridge->Get_ocStbHostProgramContentDestination (Oid);
      Status = VB.SetOid (Oid);
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}


/*\ 
 *  ocStbHostMpeg2ContentTable and related objects 
\*/

/* ocStbHostMpeg2ContentTable */

ocStbHostMpeg2ContentTable::ocStbHostMpeg2ContentTable ()
  : BcmMibTable (kOID_ocStbHostMpeg2ContentTable, kNAME_ocStbHostMpeg2ContentTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ocStbHostMpeg2ContentEntry::kLEAF_ocStbHostMpeg2ContentProgramNumber);
  fColumnMask |= (1 << ocStbHostMpeg2ContentEntry::kLEAF_ocStbHostMpeg2ContentTransportStreamID);
  fColumnMask |= (1 << ocStbHostMpeg2ContentEntry::kLEAF_ocStbHostMpeg2ContentTotalStreams);
  fColumnMask |= (1 << ocStbHostMpeg2ContentEntry::kLEAF_ocStbHostMpeg2ContentSelectedVideoPID);
  fColumnMask |= (1 << ocStbHostMpeg2ContentEntry::kLEAF_ocStbHostMpeg2ContentSelectedAudioPID);
  fColumnMask |= (1 << ocStbHostMpeg2ContentEntry::kLEAF_ocStbHostMpeg2ContentOtherAudioPIDs);
  fColumnMask |= (1 << ocStbHostMpeg2ContentEntry::kLEAF_ocStbHostMpeg2ContentCCIValue);
  fColumnMask |= (1 << ocStbHostMpeg2ContentEntry::kLEAF_ocStbHostMpeg2ContentAPSValue);
  fColumnMask |= (1 << ocStbHostMpeg2ContentEntry::kLEAF_ocStbHostMpeg2ContentCITStatus);
  fColumnMask |= (1 << ocStbHostMpeg2ContentEntry::kLEAF_ocStbHostMpeg2ContentBroadcastFlagStatus);
  fColumnMask |= (1 << ocStbHostMpeg2ContentEntry::kLEAF_ocStbHostMpeg2ContentEPNStatus);
  fColumnMask |= (1 << ocStbHostMpeg2ContentEntry::kLEAF_ocStbHostMpeg2ContentPCRPID);
  fColumnMask |= (1 << ocStbHostMpeg2ContentEntry::kLEAF_ocStbHostMpeg2ContentPCRLockStatus);
  fColumnMask |= (1 << ocStbHostMpeg2ContentEntry::kLEAF_ocStbHostMpeg2ContentDecoderPTS);
  fColumnMask |= (1 << ocStbHostMpeg2ContentEntry::kLEAF_ocStbHostMpeg2ContentDiscontinuities);
  fColumnMask |= (1 << ocStbHostMpeg2ContentEntry::kLEAF_ocStbHostMpeg2ContentPktErrors);
  fColumnMask |= (1 << ocStbHostMpeg2ContentEntry::kLEAF_ocStbHostMpeg2ContentPipelineErrors);
  fColumnMask |= (1 << ocStbHostMpeg2ContentEntry::kLEAF_ocStbHostMpeg2ContentDecoderRestarts);
}

SNMP_STATUS ocStbHostMpeg2ContentTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ocStbHostMpeg2ContentEntry *pEntry;
  ocStbHostMpeg2ContentEntryBridge *pMyBridge = (ocStbHostMpeg2ContentEntryBridge *) pBridge;


  pEntry = new ocStbHostMpeg2ContentEntry (this, pMyBridge);
  Status = AddEntry (pEntry);

  if (Status == SNMP_NO_ERROR)
  {
    // Remember bridge for destruction by base class later if specified.
    Status = pEntry->RegisterBridge (pBridge, DeleteUponDestruction);
  }
  else
    delete pEntry;

  return Status;
}

/* ocStbHostMpeg2ContentEntry */

ocStbHostMpeg2ContentEntry::ocStbHostMpeg2ContentEntry (ocStbHostMpeg2ContentTable *pTable, ocStbHostMpeg2ContentEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ocStbHostMpeg2ContentEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Uint32MibObject (kOID_ocStbHostMpeg2ContentIndex, kNAME_ocStbHostMpeg2ContentIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostMpeg2ContentProgramNumber, kNAME_ocStbHostMpeg2ContentProgramNumber, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostMpeg2ContentTransportStreamID, kNAME_ocStbHostMpeg2ContentTransportStreamID, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostMpeg2ContentTotalStreams, kNAME_ocStbHostMpeg2ContentTotalStreams, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostMpeg2ContentSelectedVideoPID, kNAME_ocStbHostMpeg2ContentSelectedVideoPID, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostMpeg2ContentSelectedAudioPID, kNAME_ocStbHostMpeg2ContentSelectedAudioPID, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostMpeg2ContentOtherAudioPIDs, kNAME_ocStbHostMpeg2ContentOtherAudioPIDs, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostMpeg2ContentCCIValue, kNAME_ocStbHostMpeg2ContentCCIValue, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostMpeg2ContentAPSValue, kNAME_ocStbHostMpeg2ContentAPSValue, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostMpeg2ContentCITStatus, kNAME_ocStbHostMpeg2ContentCITStatus, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostMpeg2ContentBroadcastFlagStatus, kNAME_ocStbHostMpeg2ContentBroadcastFlagStatus, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostMpeg2ContentEPNStatus, kNAME_ocStbHostMpeg2ContentEPNStatus, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostMpeg2ContentPCRPID, kNAME_ocStbHostMpeg2ContentPCRPID, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostMpeg2ContentPCRLockStatus, kNAME_ocStbHostMpeg2ContentPCRLockStatus, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostMpeg2ContentDecoderPTS, kNAME_ocStbHostMpeg2ContentDecoderPTS, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostMpeg2ContentDiscontinuities, kNAME_ocStbHostMpeg2ContentDiscontinuities, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostMpeg2ContentPktErrors, kNAME_ocStbHostMpeg2ContentPktErrors, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostMpeg2ContentPipelineErrors, kNAME_ocStbHostMpeg2ContentPipelineErrors, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostMpeg2ContentDecoderRestarts, kNAME_ocStbHostMpeg2ContentDecoderRestarts, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ocStbHostMpeg2ContentEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostMpeg2ContentIndex:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostMpeg2ContentIndex (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostMpeg2ContentProgramNumber:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostMpeg2ContentProgramNumber (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostMpeg2ContentTransportStreamID:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostMpeg2ContentTransportStreamID (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostMpeg2ContentTotalStreams:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostMpeg2ContentTotalStreams (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostMpeg2ContentSelectedVideoPID:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostMpeg2ContentSelectedVideoPID ());
      break;

    case kLEAF_ocStbHostMpeg2ContentSelectedAudioPID:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostMpeg2ContentSelectedAudioPID ());
      break;

    case kLEAF_ocStbHostMpeg2ContentOtherAudioPIDs:
      Status = VB.SetBool (fpBridge->Get_ocStbHostMpeg2ContentOtherAudioPIDs ());
      break;

    case kLEAF_ocStbHostMpeg2ContentCCIValue:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostMpeg2ContentCCIValue ());
      break;

    case kLEAF_ocStbHostMpeg2ContentAPSValue:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostMpeg2ContentAPSValue ());
      break;

    case kLEAF_ocStbHostMpeg2ContentCITStatus:
      Status = VB.SetBool (fpBridge->Get_ocStbHostMpeg2ContentCITStatus ());
      break;

    case kLEAF_ocStbHostMpeg2ContentBroadcastFlagStatus:
      Status = VB.SetBool (fpBridge->Get_ocStbHostMpeg2ContentBroadcastFlagStatus ());
      break;

    case kLEAF_ocStbHostMpeg2ContentEPNStatus:
      Status = VB.SetBool (fpBridge->Get_ocStbHostMpeg2ContentEPNStatus ());
      break;

    case kLEAF_ocStbHostMpeg2ContentPCRPID:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostMpeg2ContentPCRPID ());
      break;

    case kLEAF_ocStbHostMpeg2ContentPCRLockStatus:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostMpeg2ContentPCRLockStatus ());
      break;

    case kLEAF_ocStbHostMpeg2ContentDecoderPTS:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostMpeg2ContentDecoderPTS ());
      break;

    case kLEAF_ocStbHostMpeg2ContentDiscontinuities:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostMpeg2ContentDiscontinuities (), VBT_COUNTER);
      break;

    case kLEAF_ocStbHostMpeg2ContentPktErrors:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostMpeg2ContentPktErrors (), VBT_COUNTER);
      break;

    case kLEAF_ocStbHostMpeg2ContentPipelineErrors:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostMpeg2ContentPipelineErrors (), VBT_COUNTER);
      break;

    case kLEAF_ocStbHostMpeg2ContentDecoderRestarts:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostMpeg2ContentDecoderRestarts (), VBT_COUNTER);
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}


/*\ 
 *  ocStbHostAnalogVideoTable and related objects 
\*/

/* ocStbHostAnalogVideoTable */

ocStbHostAnalogVideoTable::ocStbHostAnalogVideoTable ()
  : BcmMibTable (kOID_ocStbHostAnalogVideoTable, kNAME_ocStbHostAnalogVideoTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ocStbHostAnalogVideoEntry::kLEAF_ocStbHostAnalogVideoProtectionStatus);
}

SNMP_STATUS ocStbHostAnalogVideoTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ocStbHostAnalogVideoEntry *pEntry;
  ocStbHostAnalogVideoEntryBridge *pMyBridge = (ocStbHostAnalogVideoEntryBridge *) pBridge;


  pEntry = new ocStbHostAnalogVideoEntry (this, pMyBridge);
  Status = AddEntry (pEntry);

  if (Status == SNMP_NO_ERROR)
  {
    // Remember bridge for destruction by base class later if specified.
    Status = pEntry->RegisterBridge (pBridge, DeleteUponDestruction);
  }
  else
    delete pEntry;

  return Status;
}

/* ocStbHostAnalogVideoEntry */

ocStbHostAnalogVideoEntry::ocStbHostAnalogVideoEntry (ocStbHostAnalogVideoTable *pTable, ocStbHostAnalogVideoEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ocStbHostAnalogVideoEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_ocStbHostAnalogVideoProtectionStatus, kNAME_ocStbHostAnalogVideoProtectionStatus, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ocStbHostAnalogVideoEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostAnalogVideoProtectionStatus:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostAnalogVideoProtectionStatus ());
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

/*\
 *  ocStbHostQpskObjectsGroup group
\*/

ocStbHostQpskObjectsGroup::ocStbHostQpskObjectsGroup ()
  : BcmMibGroup (kOID_ocStbHostQpskObjectsGroup, kNAME_ocStbHostQpskObjectsGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostQpskObjectsGroup::Populate ()
{
  BcmMibObject *pScalar;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new Uint32MibObject (kOID_ocStbHostQpskFDCFreq, kNAME_ocStbHostQpskFDCFreq, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ocStbHostQpskRDCFreq, kNAME_ocStbHostQpskRDCFreq, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostQpskFDCBer, kNAME_ocStbHostQpskFDCBer, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostQpskFDCStatus, kNAME_ocStbHostQpskFDCStatus, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ocStbHostQpskFDCBytesRead, kNAME_ocStbHostQpskFDCBytesRead, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostQpskFDCPower, kNAME_ocStbHostQpskFDCPower, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ocStbHostQpskFDCLockedTime, kNAME_ocStbHostQpskFDCLockedTime, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostQpskFDCSNR, kNAME_ocStbHostQpskFDCSNR, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ocStbHostQpskAGC, kNAME_ocStbHostQpskAGC, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostQpskRDCPower, kNAME_ocStbHostQpskRDCPower, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostQpskRDCDataRate, kNAME_ocStbHostQpskRDCDataRate, this, kReadOnly);
  AddScalar (pScalar);

  return true;
}

SNMP_STATUS ocStbHostQpskObjectsGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ocStbHostQpskObjectsGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ocStbHostQpskObjectsGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostQpskFDCFreq:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostQpskFDCFreq (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostQpskRDCFreq:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostQpskRDCFreq (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostQpskFDCBer:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostQpskFDCBer ());
      break;

    case kLEAF_ocStbHostQpskFDCStatus:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostQpskFDCStatus ());
      break;

    case kLEAF_ocStbHostQpskFDCBytesRead:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostQpskFDCBytesRead (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostQpskFDCPower:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostQpskFDCPower ());
      break;

    case kLEAF_ocStbHostQpskFDCLockedTime:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostQpskFDCLockedTime (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostQpskFDCSNR:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostQpskFDCSNR ());
      break;

    case kLEAF_ocStbHostQpskAGC:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostQpskAGC (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostQpskRDCPower:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostQpskRDCPower ());
      break;

    case kLEAF_ocStbHostQpskRDCDataRate:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostQpskRDCDataRate ());
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

/*\
 *  ocStbHostEasObjectsGroup group
\*/

ocStbHostEasObjectsGroup::ocStbHostEasObjectsGroup ()
  : BcmMibGroup (kOID_ocStbHostEasObjectsGroup, kNAME_ocStbHostEasObjectsGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostEasObjectsGroup::Populate ()
{
  BcmMibGroup *pGroup;


  if (BcmMibGroup::Populate() == false)
    return false;

  pGroup = new ocStbHostEasCodesGroup();
  AddGroup (pGroup);

  return true;
}

/*\
 *  ocStbHostEasCodesGroup group
\*/

ocStbHostEasCodesGroup::ocStbHostEasCodesGroup ()
  : BcmMibGroup (kOID_ocStbHostEasCodesGroup, kNAME_ocStbHostEasCodesGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostEasCodesGroup::Populate ()
{
  BcmMibObject *pScalar;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new Uint32MibObject (kOID_ocStbEasMessageStateCode, kNAME_ocStbEasMessageStateCode, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ocStbEasMessageCountyCode, kNAME_ocStbEasMessageCountyCode, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ocStbEasMessageCountySubdivisionCode, kNAME_ocStbEasMessageCountySubdivisionCode, this, kReadOnly);
  AddScalar (pScalar);

  return true;
}

SNMP_STATUS ocStbHostEasCodesGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ocStbHostEasCodesGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ocStbHostEasCodesGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbEasMessageStateCode:
      Status = VB.SetUint32 (fpBridge->Get_ocStbEasMessageStateCode (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbEasMessageCountyCode:
      Status = VB.SetUint32 (fpBridge->Get_ocStbEasMessageCountyCode (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbEasMessageCountySubdivisionCode:
      Status = VB.SetUint32 (fpBridge->Get_ocStbEasMessageCountySubdivisionCode (), VBT_UNSIGNED32);
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

/*\
 *  ocStbHostSecuritySubSystemGroup group
\*/

ocStbHostSecuritySubSystemGroup::ocStbHostSecuritySubSystemGroup ()
  : BcmMibGroup (kOID_ocStbHostSecuritySubSystemGroup, kNAME_ocStbHostSecuritySubSystemGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostSecuritySubSystemGroup::Populate ()
{
  BcmMibObject *pScalar;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new StringMibObject (kOID_ocStbHostCASystemIdentifier, kNAME_ocStbHostCASystemIdentifier, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostCAType, kNAME_ocStbHostCAType, this, kReadOnly);
  AddScalar (pScalar);

  return true;
}

SNMP_STATUS ocStbHostSecuritySubSystemGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ocStbHostSecuritySubSystemGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ocStbHostSecuritySubSystemGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostCASystemIdentifier:
    {
      BcmString String;

      fpBridge->Get_ocStbHostCASystemIdentifier (String);
      Status = VB.SetString (String);
      break;
    }

    case kLEAF_ocStbHostCAType:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostCAType ());
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

/*\
 *  ocStbHostSoftwareGroup group
\*/

ocStbHostSoftwareGroup::ocStbHostSoftwareGroup ()
  : BcmMibGroup (kOID_ocStbHostSoftwareGroup, kNAME_ocStbHostSoftwareGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostSoftwareGroup::Populate ()
{
  BcmMibGroup *pGroup;


  if (BcmMibGroup::Populate() == false)
    return false;

  pGroup = new ocStbHostDeviceSoftwareBaseGroup();
  AddGroup (pGroup);

  pGroup = new ocStbHostFirmwareDownloadStatusGroup();
  AddGroup (pGroup);

  pGroup = new ocStbHostSoftwareApplicationInfoGroup();
  AddGroup (pGroup);

  return true;
}

/*\
 *  ocStbHostDeviceSoftwareBaseGroup group
\*/

ocStbHostDeviceSoftwareBaseGroup::ocStbHostDeviceSoftwareBaseGroup ()
  : BcmMibGroup (kOID_ocStbHostDeviceSoftwareBaseGroup, kNAME_ocStbHostDeviceSoftwareBaseGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostDeviceSoftwareBaseGroup::Populate ()
{
  BcmMibObject *pScalar;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new StringMibObject (kOID_ocStbHostSoftwareFirmwareVersion, kNAME_ocStbHostSoftwareFirmwareVersion, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new StringMibObject (kOID_ocStbHostSoftwareOCAPVersion, kNAME_ocStbHostSoftwareOCAPVersion, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new DateTimeMibObject (kOID_ocStbHostSoftwareFirmwareReleaseDate, kNAME_ocStbHostSoftwareFirmwareReleaseDate, this, kReadOnly);
  AddScalar (pScalar);

  return true;
}

SNMP_STATUS ocStbHostDeviceSoftwareBaseGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ocStbHostDeviceSoftwareBaseGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ocStbHostDeviceSoftwareBaseGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostSoftwareFirmwareVersion:
      fpBridge->Get_ocStbHostSoftwareFirmwareVersion (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_ocStbHostSoftwareOCAPVersion:
      fpBridge->Get_ocStbHostSoftwareOCAPVersion (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_ocStbHostSoftwareFirmwareReleaseDate:
      Status = VB.SetDateAndTime (fpBridge->Get_ocStbHostSoftwareFirmwareReleaseDate ());
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

/*\
 *  ocStbHostFirmwareDownloadStatusGroup group
\*/

ocStbHostFirmwareDownloadStatusGroup::ocStbHostFirmwareDownloadStatusGroup ()
  : BcmMibGroup (kOID_ocStbHostFirmwareDownloadStatusGroup, kNAME_ocStbHostFirmwareDownloadStatusGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostFirmwareDownloadStatusGroup::Populate ()
{
  BcmMibObject *pScalar;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new Int32MibObject (kOID_ocStbHostFirmwareImageStatus, kNAME_ocStbHostFirmwareImageStatus, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostFirmwareCodeDownloadStatus, kNAME_ocStbHostFirmwareCodeDownloadStatus, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new StringMibObject (kOID_ocStbHostFirmwareCodeObjectName, kNAME_ocStbHostFirmwareCodeObjectName, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostFirmwareDownloadFailedStatus, kNAME_ocStbHostFirmwareDownloadFailedStatus, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ocStbHostFirmwareDownloadFailedCount, kNAME_ocStbHostFirmwareDownloadFailedCount, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ocStbHostFirmwareDownloadGroupId, kNAME_ocStbHostFirmwareDownloadGroupId, this, kReadOnly);
  AddScalar (pScalar);

  return true;
}

SNMP_STATUS ocStbHostFirmwareDownloadStatusGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ocStbHostFirmwareDownloadStatusGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ocStbHostFirmwareDownloadStatusGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostFirmwareImageStatus:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostFirmwareImageStatus ());
      break;

    case kLEAF_ocStbHostFirmwareCodeDownloadStatus:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostFirmwareCodeDownloadStatus ());
      break;

    case kLEAF_ocStbHostFirmwareCodeObjectName:
    {
      BcmString String;

      fpBridge->Get_ocStbHostFirmwareCodeObjectName (String);
      Status = VB.SetString (String);
      break;
    }

    case kLEAF_ocStbHostFirmwareDownloadFailedStatus:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostFirmwareDownloadFailedStatus ());
      break;

    case kLEAF_ocStbHostFirmwareDownloadFailedCount:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostFirmwareDownloadFailedCount (), VBT_COUNTER);
      break;

    case kLEAF_ocStbHostFirmwareDownloadGroupId:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostFirmwareDownloadGroupId (), VBT_UNSIGNED32);
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

/*\
 *  ocStbHostSoftwareApplicationInfoGroup group
\*/

ocStbHostSoftwareApplicationInfoGroup::ocStbHostSoftwareApplicationInfoGroup ()
  : BcmMibGroup (kOID_ocStbHostSoftwareApplicationInfoGroup, kNAME_ocStbHostSoftwareApplicationInfoGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostSoftwareApplicationInfoGroup::Populate ()
{
  BcmMibObject *pScalar;
  BcmMibTable *pTable;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new DateTimeMibObject (kOID_ocStbHostSoftwareApplicationInfoSigLastReceivedTime, kNAME_ocStbHostSoftwareApplicationInfoSigLastReceivedTime, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostSoftwareApplicationInfoSigLastReadStatus, kNAME_ocStbHostSoftwareApplicationInfoSigLastReadStatus, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostSoftwareApplicationInfoSigLastNetworkVersionRead, kNAME_ocStbHostSoftwareApplicationInfoSigLastNetworkVersionRead, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostSoftwareApplicationInfoSigVersionInUse, kNAME_ocStbHostSoftwareApplicationInfoSigVersionInUse, this, kReadOnly);
  AddScalar (pScalar);

  pTable = new ocStbHostSoftwareApplicationInfoTable();
  AddTable (pTable);

  return true;
}

SNMP_STATUS ocStbHostSoftwareApplicationInfoGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ocStbHostSoftwareApplicationInfoGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ocStbHostSoftwareApplicationInfoGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostSoftwareApplicationInfoSigLastReceivedTime:
      Status = VB.SetDateAndTime (fpBridge->Get_ocStbHostSoftwareApplicationInfoSigLastReceivedTime ());
      break;

    case kLEAF_ocStbHostSoftwareApplicationInfoSigLastReadStatus:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostSoftwareApplicationInfoSigLastReadStatus ());
      break;

    case kLEAF_ocStbHostSoftwareApplicationInfoSigLastNetworkVersionRead:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostSoftwareApplicationInfoSigLastNetworkVersionRead ());
      break;

    case kLEAF_ocStbHostSoftwareApplicationInfoSigVersionInUse:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostSoftwareApplicationInfoSigVersionInUse ());
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}


/*\ 
 *  ocStbHostSoftwareApplicationInfoTable and related objects 
\*/

/* ocStbHostSoftwareApplicationInfoTable */

ocStbHostSoftwareApplicationInfoTable::ocStbHostSoftwareApplicationInfoTable ()
  : BcmMibTable (kOID_ocStbHostSoftwareApplicationInfoTable, kNAME_ocStbHostSoftwareApplicationInfoTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ocStbHostSoftwareApplicationInfoEntry::kLEAF_ocStbHostSoftwareAppNameString);
  fColumnMask |= (1 << ocStbHostSoftwareApplicationInfoEntry::kLEAF_ocStbHostSoftwareAppVersionNumber);
  fColumnMask |= (1 << ocStbHostSoftwareApplicationInfoEntry::kLEAF_ocStbHostSoftwareStatus);
  fColumnMask |= (1 << ocStbHostSoftwareApplicationInfoEntry::kLEAF_ocStbHostSoftwareOrganizationId);
  fColumnMask |= (1 << ocStbHostSoftwareApplicationInfoEntry::kLEAF_ocStbHostSoftwareApplicationId);
}

SNMP_STATUS ocStbHostSoftwareApplicationInfoTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ocStbHostSoftwareApplicationInfoEntry *pEntry;
  ocStbHostSoftwareApplicationInfoEntryBridge *pMyBridge = (ocStbHostSoftwareApplicationInfoEntryBridge *) pBridge;


  pEntry = new ocStbHostSoftwareApplicationInfoEntry (this, pMyBridge);
  Status = AddEntry (pEntry);

  if (Status == SNMP_NO_ERROR)
  {
    // Remember bridge for destruction by base class later if specified.
    Status = pEntry->RegisterBridge (pBridge, DeleteUponDestruction);
  }
  else
    delete pEntry;

  return Status;
}

/* ocStbHostSoftwareApplicationInfoEntry */

ocStbHostSoftwareApplicationInfoEntry::ocStbHostSoftwareApplicationInfoEntry (ocStbHostSoftwareApplicationInfoTable *pTable, ocStbHostSoftwareApplicationInfoEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ocStbHostSoftwareApplicationInfoEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new StringMibObject (kOID_ocStbHostSoftwareAppNameString, kNAME_ocStbHostSoftwareAppNameString, this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_ocStbHostSoftwareAppVersionNumber, kNAME_ocStbHostSoftwareAppVersionNumber, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostSoftwareStatus, kNAME_ocStbHostSoftwareStatus, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostSoftwareApplicationInfoIndex, kNAME_ocStbHostSoftwareApplicationInfoIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_ocStbHostSoftwareOrganizationId, kNAME_ocStbHostSoftwareOrganizationId, this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_ocStbHostSoftwareApplicationId, kNAME_ocStbHostSoftwareApplicationId, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ocStbHostSoftwareApplicationInfoEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostSoftwareAppNameString:
      fpBridge->Get_ocStbHostSoftwareAppNameString (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_ocStbHostSoftwareAppVersionNumber:
      fpBridge->Get_ocStbHostSoftwareAppVersionNumber (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_ocStbHostSoftwareStatus:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostSoftwareStatus ());
      break;

    case kLEAF_ocStbHostSoftwareApplicationInfoIndex:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostSoftwareApplicationInfoIndex (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostSoftwareOrganizationId:
      fpBridge->Get_ocStbHostSoftwareOrganizationId (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_ocStbHostSoftwareApplicationId:
      fpBridge->Get_ocStbHostSoftwareApplicationId (String);
      Status = VB.SetString (String);
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

/*\
 *  ocStbHostStatusGroup group
\*/

ocStbHostStatusGroup::ocStbHostStatusGroup ()
  : BcmMibGroup (kOID_ocStbHostStatusGroup, kNAME_ocStbHostStatusGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostStatusGroup::Populate ()
{
  BcmMibGroup *pGroup;


  if (BcmMibGroup::Populate() == false)
    return false;

  pGroup = new ocStbHostPowerGroup();
  AddGroup (pGroup);

  pGroup = new ocStbHostSystemObjectsGroup();
  AddGroup (pGroup);

  pGroup = new ocStbCardInfoGroup();
  AddGroup (pGroup);

  pGroup = new ocStbHostInfoGroup();
  AddGroup (pGroup);

  pGroup = new ocStbHostRebootInfoGroup();
  AddGroup (pGroup);

  pGroup = new ocStbHostMemoryInfoGroup();
  AddGroup (pGroup);

  pGroup = new ocStbHostJVMInfoGroup();
  AddGroup (pGroup);

  return true;
}

/*\
 *  ocStbHostPowerGroup group
\*/

ocStbHostPowerGroup::ocStbHostPowerGroup ()
  : BcmMibGroup (kOID_ocStbHostPowerGroup, kNAME_ocStbHostPowerGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostPowerGroup::Populate ()
{
  BcmMibObject *pScalar;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new Int32MibObject (kOID_ocStbHostPowerStatus, kNAME_ocStbHostPowerStatus, this, kReadOnly);
  AddScalar (pScalar);

  return true;
}

SNMP_STATUS ocStbHostPowerGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ocStbHostPowerGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ocStbHostPowerGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostPowerStatus:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostPowerStatus ());
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

/*\
 *  ocStbHostSystemObjectsGroup group
\*/

ocStbHostSystemObjectsGroup::ocStbHostSystemObjectsGroup ()
  : BcmMibGroup (kOID_ocStbHostSystemObjectsGroup, kNAME_ocStbHostSystemObjectsGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostSystemObjectsGroup::Populate ()
{
  BcmMibTable *pTable;


  if (BcmMibGroup::Populate() == false)
    return false;

  pTable = new ocStbHostSystemMemoryReportTable();
  AddTable (pTable);

  return true;
}


/*\ 
 *  ocStbHostSystemMemoryReportTable and related objects 
\*/

/* ocStbHostSystemMemoryReportTable */

ocStbHostSystemMemoryReportTable::ocStbHostSystemMemoryReportTable ()
  : BcmMibTable (kOID_ocStbHostSystemMemoryReportTable, kNAME_ocStbHostSystemMemoryReportTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ocStbHostSystemMemoryReportEntry::kLEAF_ocStbHostSystemMemoryReportMemoryType);
  fColumnMask |= (1 << ocStbHostSystemMemoryReportEntry::kLEAF_ocStbHostSystemMemoryReportMemorySize);
}

SNMP_STATUS ocStbHostSystemMemoryReportTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ocStbHostSystemMemoryReportEntry *pEntry;
  ocStbHostSystemMemoryReportEntryBridge *pMyBridge = (ocStbHostSystemMemoryReportEntryBridge *) pBridge;


  pEntry = new ocStbHostSystemMemoryReportEntry (this, pMyBridge);
  Status = AddEntry (pEntry);

  if (Status == SNMP_NO_ERROR)
  {
    // Remember bridge for destruction by base class later if specified.
    Status = pEntry->RegisterBridge (pBridge, DeleteUponDestruction);
  }
  else
    delete pEntry;

  return Status;
}

/* ocStbHostSystemMemoryReportEntry */

ocStbHostSystemMemoryReportEntry::ocStbHostSystemMemoryReportEntry (ocStbHostSystemMemoryReportTable *pTable, ocStbHostSystemMemoryReportEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ocStbHostSystemMemoryReportEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Uint32MibObject (kOID_ocStbHostSystemMemoryReportIndex, kNAME_ocStbHostSystemMemoryReportIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostSystemMemoryReportMemoryType, kNAME_ocStbHostSystemMemoryReportMemoryType, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocStbHostSystemMemoryReportMemorySize, kNAME_ocStbHostSystemMemoryReportMemorySize, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ocStbHostSystemMemoryReportEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostSystemMemoryReportIndex:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostSystemMemoryReportIndex (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostSystemMemoryReportMemoryType:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostSystemMemoryReportMemoryType ());
      break;

    case kLEAF_ocStbHostSystemMemoryReportMemorySize:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostSystemMemoryReportMemorySize ());
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

/*\
 *  ocStbCardInfoGroup group
\*/

ocStbCardInfoGroup::ocStbCardInfoGroup ()
  : BcmMibGroup (kOID_ocStbCardInfoGroup, kNAME_ocStbCardInfoGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbCardInfoGroup::Populate ()
{
  BcmMibObject *pScalar;
  BcmMibGroup *pGroup;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new MacAddrMibObject (kOID_ocStbHostCardMacAddress, kNAME_ocStbHostCardMacAddress, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostCardIpAddressType, kNAME_ocStbHostCardIpAddressType, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new StringMibObject (kOID_ocStbHostCardIpAddress, kNAME_ocStbHostCardIpAddress, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new StringMibObject (kOID_ocStbHostCardId, kNAME_ocStbHostCardId, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostCardBindingStatus, kNAME_ocStbHostCardBindingStatus, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new StringMibObject (kOID_ocStbHostCardOpenedGenericResource, kNAME_ocStbHostCardOpenedGenericResource, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostCardTimeZoneOffset, kNAME_ocStbHostCardTimeZoneOffset, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new StringMibObject (kOID_ocStbHostCardDaylightSavingsTimeDelta, kNAME_ocStbHostCardDaylightSavingsTimeDelta, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ocStbHostCardDaylightSavingsTimeEntry, kNAME_ocStbHostCardDaylightSavingsTimeEntry, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ocStbHostCardDaylightSavingsTimeExit, kNAME_ocStbHostCardDaylightSavingsTimeExit, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new StringMibObject (kOID_ocStbHostCardEaLocationCode, kNAME_ocStbHostCardEaLocationCode, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new StringMibObject (kOID_ocStbHostCardVctId, kNAME_ocStbHostCardVctId, this, kReadOnly);
  AddScalar (pScalar);

  pGroup = new ocStbHostCCMMIGroup();
  AddGroup (pGroup);

  pGroup = new ocStbHostSnmpProxyInfoGroup();
  AddGroup (pGroup);

  pGroup = new ocStbHostCardCpInfoGroup();
  AddGroup (pGroup);

  return true;
}

SNMP_STATUS ocStbCardInfoGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ocStbCardInfoGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ocStbCardInfoGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;
  BcmMacAddress Mac;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostCardMacAddress:
      fpBridge->Get_ocStbHostCardMacAddress (Mac);
      Status = VB.SetMacAddr (Mac);
      break;

    case kLEAF_ocStbHostCardIpAddressType:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostCardIpAddressType ());
      break;

    case kLEAF_ocStbHostCardIpAddress:
      fpBridge->Get_ocStbHostCardIpAddress (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_ocStbHostCardId:
      fpBridge->Get_ocStbHostCardId (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_ocStbHostCardBindingStatus:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostCardBindingStatus ());
      break;

    case kLEAF_ocStbHostCardOpenedGenericResource:
      fpBridge->Get_ocStbHostCardOpenedGenericResource (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_ocStbHostCardTimeZoneOffset:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostCardTimeZoneOffset ());
      break;

    case kLEAF_ocStbHostCardDaylightSavingsTimeDelta:
      fpBridge->Get_ocStbHostCardDaylightSavingsTimeDelta (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_ocStbHostCardDaylightSavingsTimeEntry:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostCardDaylightSavingsTimeEntry (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostCardDaylightSavingsTimeExit:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostCardDaylightSavingsTimeExit (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostCardEaLocationCode:
      fpBridge->Get_ocStbHostCardEaLocationCode (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_ocStbHostCardVctId:
      fpBridge->Get_ocStbHostCardVctId (String);
      Status = VB.SetString (String);
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

/*\
 *  ocStbHostCCMMIGroup group
\*/

ocStbHostCCMMIGroup::ocStbHostCCMMIGroup ()
  : BcmMibGroup (kOID_ocStbHostCCMMIGroup, kNAME_ocStbHostCCMMIGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostCCMMIGroup::Populate ()
{
  BcmMibGroup *pGroup;


  if (BcmMibGroup::Populate() == false)
    return false;

  pGroup = new ocStbHostCCApplicationsGroup();
  AddGroup (pGroup);

  return true;
}

/*\
 *  ocStbHostCCApplicationsGroup group
\*/

ocStbHostCCApplicationsGroup::ocStbHostCCApplicationsGroup ()
  : BcmMibGroup (kOID_ocStbHostCCApplicationsGroup, kNAME_ocStbHostCCApplicationsGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostCCApplicationsGroup::Populate ()
{
  BcmMibTable *pTable;


  if (BcmMibGroup::Populate() == false)
    return false;

  pTable = new ocStbHostCCAppInfoTable();
  AddTable (pTable);

  return true;
}


/*\ 
 *  ocStbHostCCAppInfoTable and related objects 
\*/

/* ocStbHostCCAppInfoTable */

ocStbHostCCAppInfoTable::ocStbHostCCAppInfoTable ()
  : BcmMibTable (kOID_ocStbHostCCAppInfoTable, kNAME_ocStbHostCCAppInfoTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ocStbHostCCAppInfoEntry::kLEAF_ocStbHostCCAppInfoIndex);
  fColumnMask |= (1 << ocStbHostCCAppInfoEntry::kLEAF_ocStbHostCCApplicationType);
  fColumnMask |= (1 << ocStbHostCCAppInfoEntry::kLEAF_ocStbHostCCApplicationName);
  fColumnMask |= (1 << ocStbHostCCAppInfoEntry::kLEAF_ocStbHostCCApplicationVersion);
  fColumnMask |= (1 << ocStbHostCCAppInfoEntry::kLEAF_ocStbHostCCAppInfoPage);
}

SNMP_STATUS ocStbHostCCAppInfoTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ocStbHostCCAppInfoEntry *pEntry;
  ocStbHostCCAppInfoEntryBridge *pMyBridge = (ocStbHostCCAppInfoEntryBridge *) pBridge;


  pEntry = new ocStbHostCCAppInfoEntry (this, pMyBridge);
  Status = AddEntry (pEntry);

  if (Status == SNMP_NO_ERROR)
  {
    // Remember bridge for destruction by base class later if specified.
    Status = pEntry->RegisterBridge (pBridge, DeleteUponDestruction);
  }
  else
    delete pEntry;

  return Status;
}

/* ocStbHostCCAppInfoEntry */

ocStbHostCCAppInfoEntry::ocStbHostCCAppInfoEntry (ocStbHostCCAppInfoTable *pTable, ocStbHostCCAppInfoEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ocStbHostCCAppInfoEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Uint32MibObject (kOID_ocStbHostCCAppInfoIndex, kNAME_ocStbHostCCAppInfoIndex, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostCCApplicationType, kNAME_ocStbHostCCApplicationType, this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_ocStbHostCCApplicationName, kNAME_ocStbHostCCApplicationName, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocStbHostCCApplicationVersion, kNAME_ocStbHostCCApplicationVersion, this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_ocStbHostCCAppInfoPage, kNAME_ocStbHostCCAppInfoPage, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ocStbHostCCAppInfoEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostCCAppInfoIndex:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostCCAppInfoIndex (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostCCApplicationType:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostCCApplicationType (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostCCApplicationName:
      fpBridge->Get_ocStbHostCCApplicationName (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_ocStbHostCCApplicationVersion:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostCCApplicationVersion (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostCCAppInfoPage:
      fpBridge->Get_ocStbHostCCAppInfoPage (String);
      Status = VB.SetString (String);
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

/*\
 *  ocStbHostSnmpProxyInfoGroup group
\*/

ocStbHostSnmpProxyInfoGroup::ocStbHostSnmpProxyInfoGroup ()
  : BcmMibGroup (kOID_ocStbHostSnmpProxyInfoGroup, kNAME_ocStbHostSnmpProxyInfoGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostSnmpProxyInfoGroup::Populate ()
{
  BcmMibObject *pScalar;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new StringMibObject (kOID_ocStbHostCardMfgId, kNAME_ocStbHostCardMfgId, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new StringMibObject (kOID_ocStbHostCardVersion, kNAME_ocStbHostCardVersion, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new OidMibObject (kOID_ocStbHostCardRootOid, kNAME_ocStbHostCardRootOid, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new StringMibObject (kOID_ocStbHostCardSerialNumber, kNAME_ocStbHostCardSerialNumber, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostCardSnmpAccessControl, kNAME_ocStbHostCardSnmpAccessControl, this, kReadWrite);
  AddScalar (pScalar);

  return true;
}

SNMP_STATUS ocStbHostSnmpProxyInfoGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ocStbHostSnmpProxyInfoGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ocStbHostSnmpProxyInfoGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;
  BcmObjectId Oid;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostCardMfgId:
      fpBridge->Get_ocStbHostCardMfgId (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_ocStbHostCardVersion:
      fpBridge->Get_ocStbHostCardVersion (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_ocStbHostCardRootOid:
      fpBridge->Get_ocStbHostCardRootOid (Oid);
      Status = VB.SetOid (Oid);
      break;

    case kLEAF_ocStbHostCardSerialNumber:
      fpBridge->Get_ocStbHostCardSerialNumber (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_ocStbHostCardSnmpAccessControl:
      Status = VB.SetBool (fpBridge->Get_ocStbHostCardSnmpAccessControl ());
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

SNMP_STATUS ocStbHostSnmpProxyInfoGroup::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;
  BcmObjectId Oid;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostCardMfgId:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostCardVersion:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostCardRootOid:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostCardSerialNumber:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostCardSnmpAccessControl:
    {
      bool ocStbHostCardSnmpAccessControl;

      Status = VB.GetBool (&ocStbHostCardSnmpAccessControl);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ocStbHostCardSnmpAccessControl (ocStbHostCardSnmpAccessControl);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}

/*\
 *  ocStbHostCardCpInfoGroup group
\*/

ocStbHostCardCpInfoGroup::ocStbHostCardCpInfoGroup ()
  : BcmMibGroup (kOID_ocStbHostCardCpInfoGroup, kNAME_ocStbHostCardCpInfoGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostCardCpInfoGroup::Populate ()
{
  BcmMibObject *pScalar;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new Int32MibObject (kOID_ocStbHostCardCpAuthKeyStatus, kNAME_ocStbHostCardCpAuthKeyStatus, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostCardCpCertificateCheck, kNAME_ocStbHostCardCpCertificateCheck, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ocStbHostCardCpCciChallengeCount, kNAME_ocStbHostCardCpCciChallengeCount, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ocStbHostCardCpKeyGenerationReqCount, kNAME_ocStbHostCardCpKeyGenerationReqCount, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new StringMibObject (kOID_ocStbHostCardCpIdList, kNAME_ocStbHostCardCpIdList, this, kReadOnly);
  AddScalar (pScalar);

  return true;
}

SNMP_STATUS ocStbHostCardCpInfoGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ocStbHostCardCpInfoGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ocStbHostCardCpInfoGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostCardCpAuthKeyStatus:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostCardCpAuthKeyStatus ());
      break;

    case kLEAF_ocStbHostCardCpCertificateCheck:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostCardCpCertificateCheck ());
      break;

    case kLEAF_ocStbHostCardCpCciChallengeCount:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostCardCpCciChallengeCount (), VBT_COUNTER);
      break;

    case kLEAF_ocStbHostCardCpKeyGenerationReqCount:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostCardCpKeyGenerationReqCount (), VBT_COUNTER);
      break;

    case kLEAF_ocStbHostCardCpIdList:
    {
      BcmString String;

      fpBridge->Get_ocStbHostCardCpIdList (String);
      Status = VB.SetString (String);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

/*\
 *  ocStbHostInfoGroup group
\*/

ocStbHostInfoGroup::ocStbHostInfoGroup ()
  : BcmMibGroup (kOID_ocStbHostInfoGroup, kNAME_ocStbHostInfoGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostInfoGroup::Populate ()
{
  BcmMibObject *pScalar;
  BcmMibGroup *pGroup;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new Int32MibObject (kOID_ocStbHostIpAddressType, kNAME_ocStbHostIpAddressType, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new StringMibObject (kOID_ocStbHostIpSubNetMask, kNAME_ocStbHostIpSubNetMask, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostOobMessageMode, kNAME_ocStbHostOobMessageMode, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostBootStatus, kNAME_ocStbHostBootStatus, this, kReadOnly);
  AddScalar (pScalar);

  pGroup = new ocStbHostDumpTrapInfoGroup();
  AddGroup (pGroup);

  pGroup = new ocStbHostSpecificationsInfoGroup();
  AddGroup (pGroup);

  pGroup = new ocStbHostContentErrorSummaryInfoGroup();
  AddGroup (pGroup);

  return true;
}

SNMP_STATUS ocStbHostInfoGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ocStbHostInfoGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ocStbHostInfoGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostIpAddressType:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostIpAddressType ());
      break;

    case kLEAF_ocStbHostIpSubNetMask:
    {
      BcmString String;

      fpBridge->Get_ocStbHostIpSubNetMask (String);
      Status = VB.SetString (String);
      break;
    }

    case kLEAF_ocStbHostOobMessageMode:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostOobMessageMode ());
      break;

    case kLEAF_ocStbHostBootStatus:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostBootStatus ());
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

/*\
 *  ocStbHostDumpTrapInfoGroup group
\*/

ocStbHostDumpTrapInfoGroup::ocStbHostDumpTrapInfoGroup ()
  : BcmMibGroup (kOID_ocStbHostDumpTrapInfoGroup, kNAME_ocStbHostDumpTrapInfoGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostDumpTrapInfoGroup::Populate ()
{
  BcmMibObject *pScalar;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new Int32MibObject (kOID_ocStbHostDumpEventCount, kNAME_ocStbHostDumpEventCount, this, kReadWrite);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostDumpNow, kNAME_ocStbHostDumpNow, this, kReadWrite);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ocStbHostDumpEventTimeout, kNAME_ocStbHostDumpEventTimeout, this, kReadWrite);
  AddScalar (pScalar);

  pScalar = new StringMibObject (kOID_ocStbHostDumpFilePath, kNAME_ocStbHostDumpFilePath, this, kReadOnly);
  AddScalar (pScalar);

  return true;
}

SNMP_STATUS ocStbHostDumpTrapInfoGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ocStbHostDumpTrapInfoGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ocStbHostDumpTrapInfoGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostDumpEventCount:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostDumpEventCount ());
      break;

    case kLEAF_ocStbHostDumpNow:
      Status = VB.SetBool (fpBridge->Get_ocStbHostDumpNow ());
      break;

    case kLEAF_ocStbHostDumpEventTimeout:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostDumpEventTimeout (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocStbHostDumpFilePath:
    {
      BcmString String;

      fpBridge->Get_ocStbHostDumpFilePath (String);
      Status = VB.SetString (String);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

SNMP_STATUS ocStbHostDumpTrapInfoGroup::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostDumpEventCount:
    {
      int ocStbHostDumpEventCount;

      Status = VB.GetInt32 (&ocStbHostDumpEventCount);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ocStbHostDumpEventCount (ocStbHostDumpEventCount);
      break;
    }

    case kLEAF_ocStbHostDumpNow:
    {
      bool ocStbHostDumpNow;

      Status = VB.GetBool (&ocStbHostDumpNow);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ocStbHostDumpNow (ocStbHostDumpNow);
      break;
    }

    case kLEAF_ocStbHostDumpEventTimeout:
    {
      unsigned int ocStbHostDumpEventTimeout;

      Status = VB.GetUint32 (&ocStbHostDumpEventTimeout);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ocStbHostDumpEventTimeout (ocStbHostDumpEventTimeout);
      break;
    }

    case kLEAF_ocStbHostDumpFilePath:
      Status = SNMP_NOT_WRITABLE;
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}

/*\
 *  ocStbHostSpecificationsInfoGroup group
\*/

ocStbHostSpecificationsInfoGroup::ocStbHostSpecificationsInfoGroup ()
  : BcmMibGroup (kOID_ocStbHostSpecificationsInfoGroup, kNAME_ocStbHostSpecificationsInfoGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostSpecificationsInfoGroup::Populate ()
{
  BcmMibObject *pScalar;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new StringMibObject (kOID_ocStbHostCfrSpecificationIssue, kNAME_ocStbHostCfrSpecificationIssue, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new StringMibObject (kOID_ocStbHostMibSpecificationIssue, kNAME_ocStbHostMibSpecificationIssue, this, kReadOnly);
  AddScalar (pScalar);

  return true;
}

SNMP_STATUS ocStbHostSpecificationsInfoGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ocStbHostSpecificationsInfoGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ocStbHostSpecificationsInfoGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostCfrSpecificationIssue:
      fpBridge->Get_ocStbHostCfrSpecificationIssue (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_ocStbHostMibSpecificationIssue:
      fpBridge->Get_ocStbHostMibSpecificationIssue (String);
      Status = VB.SetString (String);
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

/*\
 *  ocStbHostContentErrorSummaryInfoGroup group
\*/

ocStbHostContentErrorSummaryInfoGroup::ocStbHostContentErrorSummaryInfoGroup ()
  : BcmMibGroup (kOID_ocStbHostContentErrorSummaryInfoGroup, kNAME_ocStbHostContentErrorSummaryInfoGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostContentErrorSummaryInfoGroup::Populate ()
{
  BcmMibObject *pScalar;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new Uint32MibObject (kOID_ocStbHostPatTimeoutCount, kNAME_ocStbHostPatTimeoutCount, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ocStbHostPmtTimeoutCount, kNAME_ocStbHostPmtTimeoutCount, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ocStbHostOobCarouselTimeoutCount, kNAME_ocStbHostOobCarouselTimeoutCount, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ocStbHostInbandCarouselTimeoutCount, kNAME_ocStbHostInbandCarouselTimeoutCount, this, kReadOnly);
  AddScalar (pScalar);

  return true;
}

SNMP_STATUS ocStbHostContentErrorSummaryInfoGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ocStbHostContentErrorSummaryInfoGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ocStbHostContentErrorSummaryInfoGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostPatTimeoutCount:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostPatTimeoutCount (), VBT_COUNTER);
      break;

    case kLEAF_ocStbHostPmtTimeoutCount:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostPmtTimeoutCount (), VBT_COUNTER);
      break;

    case kLEAF_ocStbHostOobCarouselTimeoutCount:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostOobCarouselTimeoutCount (), VBT_COUNTER);
      break;

    case kLEAF_ocStbHostInbandCarouselTimeoutCount:
      Status = VB.SetUint32 (fpBridge->Get_ocStbHostInbandCarouselTimeoutCount (), VBT_COUNTER);
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

/*\
 *  ocStbHostRebootInfoGroup group
\*/

ocStbHostRebootInfoGroup::ocStbHostRebootInfoGroup ()
  : BcmMibGroup (kOID_ocStbHostRebootInfoGroup, kNAME_ocStbHostRebootInfoGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostRebootInfoGroup::Populate ()
{
  BcmMibObject *pScalar;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new Int32MibObject (kOID_ocStbHostRebootType, kNAME_ocStbHostRebootType, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostRebootReset, kNAME_ocStbHostRebootReset, this, kReadWrite);
  AddScalar (pScalar);

  return true;
}

SNMP_STATUS ocStbHostRebootInfoGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ocStbHostRebootInfoGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ocStbHostRebootInfoGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostRebootType:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostRebootType ());
      break;

    case kLEAF_ocStbHostRebootReset:
      Status = VB.SetBool (fpBridge->Get_ocStbHostRebootReset ());
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

SNMP_STATUS ocStbHostRebootInfoGroup::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostRebootType:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocStbHostRebootReset:
    {
      bool ocStbHostRebootReset;

      Status = VB.GetBool (&ocStbHostRebootReset);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ocStbHostRebootReset (ocStbHostRebootReset);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}

/*\
 *  ocStbHostMemoryInfoGroup group
\*/

ocStbHostMemoryInfoGroup::ocStbHostMemoryInfoGroup ()
  : BcmMibGroup (kOID_ocStbHostMemoryInfoGroup, kNAME_ocStbHostMemoryInfoGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostMemoryInfoGroup::Populate ()
{
  BcmMibObject *pScalar;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new Int32MibObject (kOID_ocStbHostLargestAvailableBlock, kNAME_ocStbHostLargestAvailableBlock, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostTotalVideoMemory, kNAME_ocStbHostTotalVideoMemory, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostAvailableVideoMemory, kNAME_ocStbHostAvailableVideoMemory, this, kReadOnly);
  AddScalar (pScalar);

  return true;
}

SNMP_STATUS ocStbHostMemoryInfoGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ocStbHostMemoryInfoGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ocStbHostMemoryInfoGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostLargestAvailableBlock:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostLargestAvailableBlock ());
      break;

    case kLEAF_ocStbHostTotalVideoMemory:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostTotalVideoMemory ());
      break;

    case kLEAF_ocStbHostAvailableVideoMemory:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostAvailableVideoMemory ());
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}

/*\
 *  ocStbHostJVMInfoGroup group
\*/

ocStbHostJVMInfoGroup::ocStbHostJVMInfoGroup ()
  : BcmMibGroup (kOID_ocStbHostJVMInfoGroup, kNAME_ocStbHostJVMInfoGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocStbHostJVMInfoGroup::Populate ()
{
  BcmMibObject *pScalar;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new Int32MibObject (kOID_ocStbHostJVMHeapSize, kNAME_ocStbHostJVMHeapSize, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostJVMAvailHeap, kNAME_ocStbHostJVMAvailHeap, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostJVMLiveObjects, kNAME_ocStbHostJVMLiveObjects, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocStbHostJVMDeadObjects, kNAME_ocStbHostJVMDeadObjects, this, kReadOnly);
  AddScalar (pScalar);

  return true;
}

SNMP_STATUS ocStbHostJVMInfoGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ocStbHostJVMInfoGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ocStbHostJVMInfoGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocStbHostJVMHeapSize:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostJVMHeapSize ());
      break;

    case kLEAF_ocStbHostJVMAvailHeap:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostJVMAvailHeap ());
      break;

    case kLEAF_ocStbHostJVMLiveObjects:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostJVMLiveObjects ());
      break;

    case kLEAF_ocStbHostJVMDeadObjects:
      Status = VB.SetInt32 (fpBridge->Get_ocStbHostJVMDeadObjects ());
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  // If this leaf is 'not-accessible' return noAccess here.  We will, however,
  // still populate the VB so that programatic access is still possible.
  if ((Leaf.Access() == kNotAccessible)
  ||  (Leaf.Access() == kAccessibleForNotify))
    Status = SNMP_NO_ACCESS;

  return Status;
}



