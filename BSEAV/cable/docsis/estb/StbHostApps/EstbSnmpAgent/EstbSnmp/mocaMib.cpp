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
//    Filename: mocaMib.cpp
//    Creation Date: Friday, October 16, 2009 at 10:25:11
//    Created by Broadcom BFC Mib Compiler BCMIBC.EXE version 3.6.1b
//    from input file MOCA11-MIB.mib
//
//****************************************************************************

#include "SnmpLog.h" // for SnmpLog... macros
#include "mocaMib.h"
#include "mocaBridge.h"
#include "BcmSnmpAgent.h"


/*\
 *  moca11Group group
\*/

moca11Group::moca11Group ()
  : BcmMibGroup (kOID_moca11Group, kNAME_moca11Group)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool moca11Group::Populate ()
{
  BcmMibGroup *pGroup;


  if (BcmMibGroup::Populate() == false)
    return false;

  pGroup = new mocaObjectsGroup();
  AddGroup (pGroup);

  return true;
}

/*\
 *  mocaObjectsGroup group
\*/

mocaObjectsGroup::mocaObjectsGroup ()
  : BcmMibGroup (kOID_mocaObjectsGroup, kNAME_mocaObjectsGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool mocaObjectsGroup::Populate ()
{
  BcmMibTable *pTable;


  if (BcmMibGroup::Populate() == false)
    return false;

  pTable = new mocaIfConfigTable();
  AddTable (pTable);

  pTable = new mocaIfAccessTable();
  AddTable (pTable);

  pTable = new mocaIfStatusTable();
  AddTable (pTable);

  pTable = new mocaIfStatsTable();
  AddTable (pTable);

  pTable = new mocaIfFlowStatsTable();
  AddTable (pTable);

  pTable = new mocaNodeTable();
  AddTable (pTable);

  pTable = new mocaMeshTable();
  AddTable (pTable);

  pTable = new mocaBridgeTable();
  AddTable (pTable);

  return true;
}


/*\ 
 *  mocaIfConfigTable and related objects 
\*/

/* mocaIfConfigTable */

mocaIfConfigTable::mocaIfConfigTable ()
  : BcmMibTable (kOID_mocaIfConfigTable, kNAME_mocaIfConfigTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << mocaIfConfigEntry::kLEAF_mocaIfEnable);
  fColumnMask |= (1 << mocaIfConfigEntry::kLEAF_mocaIfChannelMask);
  fColumnMask |= (1 << mocaIfConfigEntry::kLEAF_mocaIfPowerControl);
  fColumnMask |= (1 << mocaIfConfigEntry::kLEAF_mocaIfTxPowerLimit);
  fColumnMask |= (1 << mocaIfConfigEntry::kLEAF_mocaIfBeaconPowerLimit);
  fColumnMask |= (1 << mocaIfConfigEntry::kLEAF_mocaIfPowerControlTargetRate);
  fColumnMask |= (1 << mocaIfConfigEntry::kLEAF_mocaIfPrivacyEnable);
  fColumnMask |= (1 << mocaIfConfigEntry::kLEAF_mocaIfPassword);
  fColumnMask |= (1 << mocaIfConfigEntry::kLEAF_mocaIfPreferredNC);
  fColumnMask |= (1 << mocaIfConfigEntry::kLEAF_mocaIfAccessEnable);
  fColumnMask |= (1 << mocaIfConfigEntry::kLEAF_mocaIfPhyThreshold);
  fColumnMask |= (1 << mocaIfConfigEntry::kLEAF_mocaIfPhyThresholdEnable);
  fColumnMask |= (1 << mocaIfConfigEntry::kLEAF_mocaIfStatusChangeEnable);
  fColumnMask |= (1 << mocaIfConfigEntry::kLEAF_mocaIfNumNodesChangeEnable);
}

SNMP_STATUS mocaIfConfigTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  mocaIfConfigEntry *pEntry;
  mocaIfConfigEntryBridge *pMyBridge = (mocaIfConfigEntryBridge *) pBridge;


  pEntry = new mocaIfConfigEntry (this, pMyBridge);
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

/* mocaIfConfigEntry */

mocaIfConfigEntry::mocaIfConfigEntry (mocaIfConfigTable *pTable, mocaIfConfigEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_mocaIfConfigEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_mocaIfEnable, kNAME_mocaIfEnable, this, kReadWrite);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfChannelMask, kNAME_mocaIfChannelMask, this, kReadWrite);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_mocaIfPowerControl, kNAME_mocaIfPowerControl, this, kReadWrite);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfTxPowerLimit, kNAME_mocaIfTxPowerLimit, this, kReadWrite);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfBeaconPowerLimit, kNAME_mocaIfBeaconPowerLimit, this, kReadWrite);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfPowerControlTargetRate, kNAME_mocaIfPowerControlTargetRate, this, kReadWrite);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_mocaIfPrivacyEnable, kNAME_mocaIfPrivacyEnable, this, kReadWrite);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_mocaIfPassword, kNAME_mocaIfPassword, this, kReadWrite);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_mocaIfPreferredNC, kNAME_mocaIfPreferredNC, this, kReadWrite);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_mocaIfAccessEnable, kNAME_mocaIfAccessEnable, this, kReadWrite);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfPhyThreshold, kNAME_mocaIfPhyThreshold, this, kReadWrite);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_mocaIfPhyThresholdEnable, kNAME_mocaIfPhyThresholdEnable, this, kReadWrite);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_mocaIfStatusChangeEnable, kNAME_mocaIfStatusChangeEnable, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_mocaIfNumNodesChangeEnable, kNAME_mocaIfNumNodesChangeEnable, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS mocaIfConfigEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_mocaIfEnable:
      Status = VB.SetBool (fpBridge->Get_mocaIfEnable ());
      break;

    case kLEAF_mocaIfChannelMask:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfChannelMask (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaIfPowerControl:
      Status = VB.SetBool (fpBridge->Get_mocaIfPowerControl ());
      break;

    case kLEAF_mocaIfTxPowerLimit:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfTxPowerLimit (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaIfBeaconPowerLimit:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfBeaconPowerLimit (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaIfPowerControlTargetRate:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfPowerControlTargetRate (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaIfPrivacyEnable:
      Status = VB.SetBool (fpBridge->Get_mocaIfPrivacyEnable ());
      break;

    case kLEAF_mocaIfPassword:
    {
      BcmString String;

      fpBridge->Get_mocaIfPassword (String);
      Status = VB.SetString (String);
      break;
    }

    case kLEAF_mocaIfPreferredNC:
      Status = VB.SetBool (fpBridge->Get_mocaIfPreferredNC ());
      break;

    case kLEAF_mocaIfAccessEnable:
      Status = VB.SetBool (fpBridge->Get_mocaIfAccessEnable ());
      break;

    case kLEAF_mocaIfPhyThreshold:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfPhyThreshold (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaIfPhyThresholdEnable:
      Status = VB.SetBool (fpBridge->Get_mocaIfPhyThresholdEnable ());
      break;

    case kLEAF_mocaIfStatusChangeEnable:
      Status = VB.SetBool (fpBridge->Get_mocaIfStatusChangeEnable ());
      break;

    case kLEAF_mocaIfNumNodesChangeEnable:
      Status = VB.SetBool (fpBridge->Get_mocaIfNumNodesChangeEnable ());
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

SNMP_STATUS mocaIfConfigEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_mocaIfEnable:
    {
      bool mocaIfEnable;

      Status = VB.GetBool (&mocaIfEnable);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_mocaIfEnable (mocaIfEnable);
      break;
    }

    case kLEAF_mocaIfChannelMask:
    {
      unsigned int mocaIfChannelMask;

      Status = VB.GetUint32 (&mocaIfChannelMask);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_mocaIfChannelMask (mocaIfChannelMask);
      break;
    }

    case kLEAF_mocaIfPowerControl:
    {
      bool mocaIfPowerControl;

      Status = VB.GetBool (&mocaIfPowerControl);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_mocaIfPowerControl (mocaIfPowerControl);
      break;
    }

    case kLEAF_mocaIfTxPowerLimit:
    {
      unsigned int mocaIfTxPowerLimit;

      Status = VB.GetUint32 (&mocaIfTxPowerLimit);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_mocaIfTxPowerLimit (mocaIfTxPowerLimit);
      break;
    }

    case kLEAF_mocaIfBeaconPowerLimit:
    {
      unsigned int mocaIfBeaconPowerLimit;

      Status = VB.GetUint32 (&mocaIfBeaconPowerLimit);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_mocaIfBeaconPowerLimit (mocaIfBeaconPowerLimit);
      break;
    }

    case kLEAF_mocaIfPowerControlTargetRate:
    {
      unsigned int mocaIfPowerControlTargetRate;

      Status = VB.GetUint32 (&mocaIfPowerControlTargetRate);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_mocaIfPowerControlTargetRate (mocaIfPowerControlTargetRate);
      break;
    }

    case kLEAF_mocaIfPrivacyEnable:
    {
      bool mocaIfPrivacyEnable;

      Status = VB.GetBool (&mocaIfPrivacyEnable);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_mocaIfPrivacyEnable (mocaIfPrivacyEnable);
      break;
    }

    case kLEAF_mocaIfPassword:
    {
      BcmString String;

      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_mocaIfPassword (String);
      break;
    }

    case kLEAF_mocaIfPreferredNC:
    {
      bool mocaIfPreferredNC;

      Status = VB.GetBool (&mocaIfPreferredNC);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_mocaIfPreferredNC (mocaIfPreferredNC);
      break;
    }

    case kLEAF_mocaIfAccessEnable:
    {
      bool mocaIfAccessEnable;

      Status = VB.GetBool (&mocaIfAccessEnable);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_mocaIfAccessEnable (mocaIfAccessEnable);
      break;
    }

    case kLEAF_mocaIfPhyThreshold:
    {
      unsigned int mocaIfPhyThreshold;

      Status = VB.GetUint32 (&mocaIfPhyThreshold);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_mocaIfPhyThreshold (mocaIfPhyThreshold);
      break;
    }

    case kLEAF_mocaIfPhyThresholdEnable:
    {
      bool mocaIfPhyThresholdEnable;

      Status = VB.GetBool (&mocaIfPhyThresholdEnable);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_mocaIfPhyThresholdEnable (mocaIfPhyThresholdEnable);
      break;
    }

    case kLEAF_mocaIfStatusChangeEnable:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_mocaIfNumNodesChangeEnable:
      Status = SNMP_NOT_WRITABLE;
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}


/*\ 
 *  mocaIfAccessTable and related objects 
\*/

/* mocaIfAccessTable */

mocaIfAccessTable::mocaIfAccessTable ()
  : BcmMibTable (kOID_mocaIfAccessTable, kNAME_mocaIfAccessTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << mocaIfAccessEntry::kLEAF_mocaIfAccessMacAddress);
  fColumnMask |= (1 << mocaIfAccessEntry::kLEAF_mocaIfAccessStatus);
}

SNMP_STATUS mocaIfAccessTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  mocaIfAccessEntry *pEntry;
  mocaIfAccessEntryBridge *pMyBridge = (mocaIfAccessEntryBridge *) pBridge;


  pEntry = new mocaIfAccessEntry (this, pMyBridge);
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

BcmMibEntry *mocaIfAccessTable::NewDefaultEntry (BcmObjectId &Index)
{
  if (fpAgent == NULL)
    return NULL;

  /* Verify index length. */

  // mocaIfAccessIndex (numerical type): 1 element
  if (Index.Length() != 1)
    return NULL;

  /* Verify index value(s) */

  // mocaIfAccessIndex (numerical type)
  if ((Index[0] < MINVAL_MOCAIFACCESSINDEX)
  ||  (Index[0] > MAXVAL_MOCAIFACCESSINDEX))
    return NULL;

  SnmpLogRaw << "UNFINISHED: double-check index values in mocaIfAccessTable::NewDefaultEntry" << endl;

  mocaIfAccessEntryBridge *pBridge = new mocaIfAccessEntryBridge (Index, fpAgent);

  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
  {
    SnmpLogInit << "mocaIfAccessTable::NewDefaultEntry Bridge registration failed!  Deleting bridge object." << endl;
    delete (pBridge);
    return NULL;
  }
  
  return (BcmMibEntry *)pBridge->fpObject;
}

/* mocaIfAccessEntry */

mocaIfAccessEntry::mocaIfAccessEntry (mocaIfAccessTable *pTable, mocaIfAccessEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_mocaIfAccessEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_mocaIfAccessIndex, kNAME_mocaIfAccessIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new MacAddrMibObject (kOID_mocaIfAccessMacAddress, kNAME_mocaIfAccessMacAddress, this, kReadCreate);
  AddObject (pObject);

  pObject = new RowStatusMibObject (pTable, this, kOID_mocaIfAccessStatus, kNAME_mocaIfAccessStatus, kReadCreate);
  AddObject (pObject);
}

SNMP_STATUS mocaIfAccessEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmMacAddress Mac;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_mocaIfAccessIndex:
      Status = VB.SetInt32 (fpBridge->Get_mocaIfAccessIndex ());
      break;

    case kLEAF_mocaIfAccessMacAddress:
      fpBridge->Get_mocaIfAccessMacAddress (Mac);
      Status = VB.SetMacAddr (Mac);
      break;

    case kLEAF_mocaIfAccessStatus:
      Status = VB.SetInt32 (fpBridge->Get_mocaIfAccessStatus ());
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

SNMP_STATUS mocaIfAccessEntry::TestLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmMacAddress Mac;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_mocaIfAccessIndex:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_mocaIfAccessMacAddress:
      Status = VB.GetMacAddr (Mac);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_mocaIfAccessMacAddress (Mac);
      break;

    case kLEAF_mocaIfAccessStatus:
    {
      int mocaIfAccessStatus;

      Status = VB.GetInt32 (&mocaIfAccessStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_mocaIfAccessStatus (mocaIfAccessStatus);
      break;
    }

  }

  return Status;
}

SNMP_STATUS mocaIfAccessEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmMacAddress Mac;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  // If this row has not yet been created, either reject the set or create the
  // row; either is allowed per RFC-2579.  See note in MibObjs.h for details.
  if ((fpBridge->Get_mocaIfAccessStatus() == SNMP_STATUS_INVISIBLE)
  &&  ((fpBridge->fObjectMask & mocaIfAccessEntryBridge::kBIT_mocaIfAccessStatus) == 0))
  {
#ifdef SUPPORT_INDIRECT_ROW_CREATION
    int CreateStatus = SNMP_STATUS_CREATE_AND_WAIT;

    SnmpLogReq << "Indirect row creation supported, creating mocaIfAccessEntry" << endl;
    Status = fpBridge->Set_mocaIfAccessStatus (CreateStatus);
    if (Status != SNMP_NO_ERROR)
      return Status;
#else
    SnmpLogReq << "mocaIfAccessStatus not set (" << (void *)fpBridge->fObjectMask << "), INCONSISTENT_VALUE" << endl;
    return SNMP_INCONSISTENT_VALUE | SNMP_ROW_NOT_CREATED;
#endif
  }

  switch (Leaf.Leaf())
  {
    case kLEAF_mocaIfAccessIndex:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_mocaIfAccessMacAddress:
      Status = VB.GetMacAddr (Mac);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_mocaIfAccessMacAddress (Mac);
      break;

    case kLEAF_mocaIfAccessStatus:
    {
      int mocaIfAccessStatus;

      Status = VB.GetInt32 (&mocaIfAccessStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_mocaIfAccessStatus (mocaIfAccessStatus);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}


/*\ 
 *  mocaIfStatusTable and related objects 
\*/

/* mocaIfStatusTable */

mocaIfStatusTable::mocaIfStatusTable ()
  : BcmMibTable (kOID_mocaIfStatusTable, kNAME_mocaIfStatusTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << mocaIfStatusEntry::kLEAF_mocaIfStatus);
  fColumnMask |= (1 << mocaIfStatusEntry::kLEAF_mocaIfLinkUpTime);
  fColumnMask |= (1 << mocaIfStatusEntry::kLEAF_mocaIfSoftwareVersion);
  fColumnMask |= (1 << mocaIfStatusEntry::kLEAF_mocaIfMocaVersion);
  fColumnMask |= (1 << mocaIfStatusEntry::kLEAF_mocaIfNetworkVersion);
  fColumnMask |= (1 << mocaIfStatusEntry::kLEAF_mocaIfMacAddress);
  fColumnMask |= (1 << mocaIfStatusEntry::kLEAF_mocaIfNodeID);
  fColumnMask |= (1 << mocaIfStatusEntry::kLEAF_mocaIfName);
  fColumnMask |= (1 << mocaIfStatusEntry::kLEAF_mocaIfNumNodes);
  fColumnMask |= (1 << mocaIfStatusEntry::kLEAF_mocaIfNC);
  fColumnMask |= (1 << mocaIfStatusEntry::kLEAF_mocaIfBackupNC);
  fColumnMask |= (1 << mocaIfStatusEntry::kLEAF_mocaIfRFChannel);
  fColumnMask |= (1 << mocaIfStatusEntry::kLEAF_mocaIfLOF);
  fColumnMask |= (1 << mocaIfStatusEntry::kLEAF_mocaIfTabooChannelMask);
  fColumnMask |= (1 << mocaIfStatusEntry::kLEAF_mocaIfNodeTabooChannelMask);
  fColumnMask |= (1 << mocaIfStatusEntry::kLEAF_mocaIfCapabilityMask);
  fColumnMask |= (1 << mocaIfStatusEntry::kLEAF_mocaIfTxGcdPowerReduction);
  fColumnMask |= (1 << mocaIfStatusEntry::kLEAF_mocaIfQAM256Capable);
  fColumnMask |= (1 << mocaIfStatusEntry::kLEAF_mocaIfPacketsAggrCapability);
  fColumnMask |= (1 << mocaIfStatusEntry::kLEAF_mocaIfMaxIngressNodeBw);
  fColumnMask |= (1 << mocaIfStatusEntry::kLEAF_mocaIfMaxEgressNodeBw);
  fColumnMask |= (1 << mocaIfStatusEntry::kLEAF_mocaIfTxGcdRate);
}

SNMP_STATUS mocaIfStatusTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  mocaIfStatusEntry *pEntry;
  mocaIfStatusEntryBridge *pMyBridge = (mocaIfStatusEntryBridge *) pBridge;


  pEntry = new mocaIfStatusEntry (this, pMyBridge);
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

/* mocaIfStatusEntry */

mocaIfStatusEntry::mocaIfStatusEntry (mocaIfStatusTable *pTable, mocaIfStatusEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_mocaIfStatusEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_mocaIfStatus, kNAME_mocaIfStatus, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfLinkUpTime, kNAME_mocaIfLinkUpTime, this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_mocaIfSoftwareVersion, kNAME_mocaIfSoftwareVersion, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_mocaIfMocaVersion, kNAME_mocaIfMocaVersion, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_mocaIfNetworkVersion, kNAME_mocaIfNetworkVersion, this, kReadOnly);
  AddObject (pObject);

  pObject = new MacAddrMibObject (kOID_mocaIfMacAddress, kNAME_mocaIfMacAddress, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfNodeID, kNAME_mocaIfNodeID, this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_mocaIfName, kNAME_mocaIfName, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfNumNodes, kNAME_mocaIfNumNodes, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfNC, kNAME_mocaIfNC, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfBackupNC, kNAME_mocaIfBackupNC, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_mocaIfRFChannel, kNAME_mocaIfRFChannel, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_mocaIfLOF, kNAME_mocaIfLOF, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfTabooChannelMask, kNAME_mocaIfTabooChannelMask, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfNodeTabooChannelMask, kNAME_mocaIfNodeTabooChannelMask, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfCapabilityMask, kNAME_mocaIfCapabilityMask, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfTxGcdPowerReduction, kNAME_mocaIfTxGcdPowerReduction, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_mocaIfQAM256Capable, kNAME_mocaIfQAM256Capable, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_mocaIfPacketsAggrCapability, kNAME_mocaIfPacketsAggrCapability, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfMaxIngressNodeBw, kNAME_mocaIfMaxIngressNodeBw, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfMaxEgressNodeBw, kNAME_mocaIfMaxEgressNodeBw, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfTxGcdRate, kNAME_mocaIfTxGcdRate, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS mocaIfStatusEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmMacAddress Mac;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_mocaIfStatus:
      Status = VB.SetInt32 (fpBridge->Get_mocaIfStatus ());
      break;

    case kLEAF_mocaIfLinkUpTime:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfLinkUpTime (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaIfSoftwareVersion:
      fpBridge->Get_mocaIfSoftwareVersion (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_mocaIfMocaVersion:
      Status = VB.SetInt32 (fpBridge->Get_mocaIfMocaVersion ());
      break;

    case kLEAF_mocaIfNetworkVersion:
      Status = VB.SetInt32 (fpBridge->Get_mocaIfNetworkVersion ());
      break;

    case kLEAF_mocaIfMacAddress:
      fpBridge->Get_mocaIfMacAddress (Mac);
      Status = VB.SetMacAddr (Mac);
      break;

    case kLEAF_mocaIfNodeID:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfNodeID (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaIfName:
      fpBridge->Get_mocaIfName (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_mocaIfNumNodes:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfNumNodes (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaIfNC:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfNC (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaIfBackupNC:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfBackupNC (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaIfRFChannel:
      Status = VB.SetInt32 (fpBridge->Get_mocaIfRFChannel ());
      break;

    case kLEAF_mocaIfLOF:
      Status = VB.SetInt32 (fpBridge->Get_mocaIfLOF ());
      break;

    case kLEAF_mocaIfTabooChannelMask:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfTabooChannelMask (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaIfNodeTabooChannelMask:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfNodeTabooChannelMask (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaIfCapabilityMask:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfCapabilityMask (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaIfTxGcdPowerReduction:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfTxGcdPowerReduction (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaIfQAM256Capable:
      Status = VB.SetBool (fpBridge->Get_mocaIfQAM256Capable ());
      break;

    case kLEAF_mocaIfPacketsAggrCapability:
      Status = VB.SetInt32 (fpBridge->Get_mocaIfPacketsAggrCapability ());
      break;

    case kLEAF_mocaIfMaxIngressNodeBw:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfMaxIngressNodeBw (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaIfMaxEgressNodeBw:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfMaxEgressNodeBw (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaIfTxGcdRate:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfTxGcdRate (), VBT_UNSIGNED32);
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
 *  mocaIfStatsTable and related objects 
\*/

/* mocaIfStatsTable */

mocaIfStatsTable::mocaIfStatsTable ()
  : BcmMibTable (kOID_mocaIfStatsTable, kNAME_mocaIfStatsTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << mocaIfStatsEntry::kLEAF_mocaIfTxPackets);
  fColumnMask |= (1 << mocaIfStatsEntry::kLEAF_mocaIfTxDrops);
  fColumnMask |= (1 << mocaIfStatsEntry::kLEAF_mocaIfRxPackets);
  fColumnMask |= (1 << mocaIfStatsEntry::kLEAF_mocaIfRxCorrectedErrors);
  fColumnMask |= (1 << mocaIfStatsEntry::kLEAF_mocaIfRxDrops);
  fColumnMask |= (1 << mocaIfStatsEntry::kLEAF_mocaIfEgressNodeNumFlows);
  fColumnMask |= (1 << mocaIfStatsEntry::kLEAF_mocaIfIngressNodeNumFlows);
}

SNMP_STATUS mocaIfStatsTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  mocaIfStatsEntry *pEntry;
  mocaIfStatsEntryBridge *pMyBridge = (mocaIfStatsEntryBridge *) pBridge;


  pEntry = new mocaIfStatsEntry (this, pMyBridge);
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

/* mocaIfStatsEntry */

mocaIfStatsEntry::mocaIfStatsEntry (mocaIfStatsTable *pTable, mocaIfStatsEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_mocaIfStatsEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Uint32MibObject (kOID_mocaIfTxPackets, kNAME_mocaIfTxPackets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfTxDrops, kNAME_mocaIfTxDrops, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfRxPackets, kNAME_mocaIfRxPackets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfRxCorrectedErrors, kNAME_mocaIfRxCorrectedErrors, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfRxDrops, kNAME_mocaIfRxDrops, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfEgressNodeNumFlows, kNAME_mocaIfEgressNodeNumFlows, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfIngressNodeNumFlows, kNAME_mocaIfIngressNodeNumFlows, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS mocaIfStatsEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_mocaIfTxPackets:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfTxPackets (), VBT_COUNTER);
      break;

    case kLEAF_mocaIfTxDrops:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfTxDrops (), VBT_COUNTER);
      break;

    case kLEAF_mocaIfRxPackets:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfRxPackets (), VBT_COUNTER);
      break;

    case kLEAF_mocaIfRxCorrectedErrors:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfRxCorrectedErrors (), VBT_COUNTER);
      break;

    case kLEAF_mocaIfRxDrops:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfRxDrops (), VBT_COUNTER);
      break;

    case kLEAF_mocaIfEgressNodeNumFlows:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfEgressNodeNumFlows (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaIfIngressNodeNumFlows:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfIngressNodeNumFlows (), VBT_UNSIGNED32);
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
 *  mocaIfFlowStatsTable and related objects 
\*/

/* mocaIfFlowStatsTable */

mocaIfFlowStatsTable::mocaIfFlowStatsTable ()
  : BcmMibTable (kOID_mocaIfFlowStatsTable, kNAME_mocaIfFlowStatsTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << mocaIfFlowStatsEntry::kLEAF_mocaIfFlowID);
  fColumnMask |= (1 << mocaIfFlowStatsEntry::kLEAF_mocaIfPacketDA);
  fColumnMask |= (1 << mocaIfFlowStatsEntry::kLEAF_mocaIfPeakDataRate);
  fColumnMask |= (1 << mocaIfFlowStatsEntry::kLEAF_mocaIfBurstSize);
  fColumnMask |= (1 << mocaIfFlowStatsEntry::kLEAF_mocaIfLeaseTime);
  fColumnMask |= (1 << mocaIfFlowStatsEntry::kLEAF_mocaIfFlowTag);
  fColumnMask |= (1 << mocaIfFlowStatsEntry::kLEAF_mocaIfLeaseTimeLeft);
  fColumnMask |= (1 << mocaIfFlowStatsEntry::kLEAF_mocaIfTxPacketsOnePacketAggr);
  fColumnMask |= (1 << mocaIfFlowStatsEntry::kLEAF_mocaIfTxPacketsTwoPacketsAggr);
  fColumnMask |= (1 << mocaIfFlowStatsEntry::kLEAF_mocaIfTxPacketsThreePacketsAggr);
  fColumnMask |= (1 << mocaIfFlowStatsEntry::kLEAF_mocaIfTxPacketsFourPacketsAggr);
  fColumnMask |= (1 << mocaIfFlowStatsEntry::kLEAF_mocaIfTxPacketsFivePacketsAggr);
  fColumnMask |= (1 << mocaIfFlowStatsEntry::kLEAF_mocaIfTxPacketsSixPacketsAggr);
  fColumnMask |= (1 << mocaIfFlowStatsEntry::kLEAF_mocaIfTxPacketsSevenPacketsAggr);
  fColumnMask |= (1 << mocaIfFlowStatsEntry::kLEAF_mocaIfTxPacketsEightPacketsAggr);
  fColumnMask |= (1 << mocaIfFlowStatsEntry::kLEAF_mocaIfTxPacketsNinePacketsAggr);
  fColumnMask |= (1 << mocaIfFlowStatsEntry::kLEAF_mocaIfTxPacketsTenPacketsAggr);
  fColumnMask |= (1 << mocaIfFlowStatsEntry::kLEAF_mocaIfTxPacketsFlow);
}

SNMP_STATUS mocaIfFlowStatsTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  mocaIfFlowStatsEntry *pEntry;
  mocaIfFlowStatsEntryBridge *pMyBridge = (mocaIfFlowStatsEntryBridge *) pBridge;


  pEntry = new mocaIfFlowStatsEntry (this, pMyBridge);
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

/* mocaIfFlowStatsEntry */

mocaIfFlowStatsEntry::mocaIfFlowStatsEntry (mocaIfFlowStatsTable *pTable, mocaIfFlowStatsEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_mocaIfFlowStatsEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_mocaIfFlowIndex, kNAME_mocaIfFlowIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new MacAddrMibObject (kOID_mocaIfFlowID, kNAME_mocaIfFlowID, this, kReadOnly);
  AddObject (pObject);

  pObject = new MacAddrMibObject (kOID_mocaIfPacketDA, kNAME_mocaIfPacketDA, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfPeakDataRate, kNAME_mocaIfPeakDataRate, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfBurstSize, kNAME_mocaIfBurstSize, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfLeaseTime, kNAME_mocaIfLeaseTime, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfFlowTag, kNAME_mocaIfFlowTag, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfLeaseTimeLeft, kNAME_mocaIfLeaseTimeLeft, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfTxPacketsOnePacketAggr, kNAME_mocaIfTxPacketsOnePacketAggr, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfTxPacketsTwoPacketsAggr, kNAME_mocaIfTxPacketsTwoPacketsAggr, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfTxPacketsThreePacketsAggr, kNAME_mocaIfTxPacketsThreePacketsAggr, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfTxPacketsFourPacketsAggr, kNAME_mocaIfTxPacketsFourPacketsAggr, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfTxPacketsFivePacketsAggr, kNAME_mocaIfTxPacketsFivePacketsAggr, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfTxPacketsSixPacketsAggr, kNAME_mocaIfTxPacketsSixPacketsAggr, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfTxPacketsSevenPacketsAggr, kNAME_mocaIfTxPacketsSevenPacketsAggr, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfTxPacketsEightPacketsAggr, kNAME_mocaIfTxPacketsEightPacketsAggr, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfTxPacketsNinePacketsAggr, kNAME_mocaIfTxPacketsNinePacketsAggr, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfTxPacketsTenPacketsAggr, kNAME_mocaIfTxPacketsTenPacketsAggr, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaIfTxPacketsFlow, kNAME_mocaIfTxPacketsFlow, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS mocaIfFlowStatsEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmMacAddress Mac;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_mocaIfFlowIndex:
      Status = VB.SetInt32 (fpBridge->Get_mocaIfFlowIndex ());
      break;

    case kLEAF_mocaIfFlowID:
      fpBridge->Get_mocaIfFlowID (Mac);
      Status = VB.SetMacAddr (Mac);
      break;

    case kLEAF_mocaIfPacketDA:
      fpBridge->Get_mocaIfPacketDA (Mac);
      Status = VB.SetMacAddr (Mac);
      break;

    case kLEAF_mocaIfPeakDataRate:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfPeakDataRate (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaIfBurstSize:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfBurstSize (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaIfLeaseTime:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfLeaseTime (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaIfFlowTag:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfFlowTag (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaIfLeaseTimeLeft:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfLeaseTimeLeft (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaIfTxPacketsOnePacketAggr:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfTxPacketsOnePacketAggr (), VBT_COUNTER);
      break;

    case kLEAF_mocaIfTxPacketsTwoPacketsAggr:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfTxPacketsTwoPacketsAggr (), VBT_COUNTER);
      break;

    case kLEAF_mocaIfTxPacketsThreePacketsAggr:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfTxPacketsThreePacketsAggr (), VBT_COUNTER);
      break;

    case kLEAF_mocaIfTxPacketsFourPacketsAggr:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfTxPacketsFourPacketsAggr (), VBT_COUNTER);
      break;

    case kLEAF_mocaIfTxPacketsFivePacketsAggr:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfTxPacketsFivePacketsAggr (), VBT_COUNTER);
      break;

    case kLEAF_mocaIfTxPacketsSixPacketsAggr:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfTxPacketsSixPacketsAggr (), VBT_COUNTER);
      break;

    case kLEAF_mocaIfTxPacketsSevenPacketsAggr:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfTxPacketsSevenPacketsAggr (), VBT_COUNTER);
      break;

    case kLEAF_mocaIfTxPacketsEightPacketsAggr:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfTxPacketsEightPacketsAggr (), VBT_COUNTER);
      break;

    case kLEAF_mocaIfTxPacketsNinePacketsAggr:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfTxPacketsNinePacketsAggr (), VBT_COUNTER);
      break;

    case kLEAF_mocaIfTxPacketsTenPacketsAggr:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfTxPacketsTenPacketsAggr (), VBT_COUNTER);
      break;

    case kLEAF_mocaIfTxPacketsFlow:
      Status = VB.SetUint32 (fpBridge->Get_mocaIfTxPacketsFlow (), VBT_UNSIGNED32);
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
 *  mocaNodeTable and related objects 
\*/

/* mocaNodeTable */

mocaNodeTable::mocaNodeTable ()
  : BcmMibTable (kOID_mocaNodeTable, kNAME_mocaNodeTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << mocaNodeEntry::kLEAF_mocaNodeMocaVersion);
  fColumnMask |= (1 << mocaNodeEntry::kLEAF_mocaNodeMacAddress);
  fColumnMask |= (1 << mocaNodeEntry::kLEAF_mocaNodeTxGcdRate);
  fColumnMask |= (1 << mocaNodeEntry::kLEAF_mocaNodeRxGcdPower);
  fColumnMask |= (1 << mocaNodeEntry::kLEAF_mocaNodeTxPowerReduction);
  fColumnMask |= (1 << mocaNodeEntry::kLEAF_mocaNodeRxPower);
  fColumnMask |= (1 << mocaNodeEntry::kLEAF_mocaNodePreferredNC);
  fColumnMask |= (1 << mocaNodeEntry::kLEAF_mocaNodeQAM256Capable);
  fColumnMask |= (1 << mocaNodeEntry::kLEAF_mocaNodePacketsAggrCapability);
  fColumnMask |= (1 << mocaNodeEntry::kLEAF_mocaNodeRxPackets);
  fColumnMask |= (1 << mocaNodeEntry::kLEAF_mocaNodeRxCorrected);
  fColumnMask |= (1 << mocaNodeEntry::kLEAF_mocaNodeRxDrops);
  fColumnMask |= (1 << mocaNodeEntry::kLEAF_mocaNodeSNR);
}

SNMP_STATUS mocaNodeTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  mocaNodeEntry *pEntry;
  mocaNodeEntryBridge *pMyBridge = (mocaNodeEntryBridge *) pBridge;


  pEntry = new mocaNodeEntry (this, pMyBridge);
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

/* mocaNodeEntry */

mocaNodeEntry::mocaNodeEntry (mocaNodeTable *pTable, mocaNodeEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_mocaNodeEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Uint32MibObject (kOID_mocaNodeIndex, kNAME_mocaNodeIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_mocaNodeMocaVersion, kNAME_mocaNodeMocaVersion, this, kReadOnly);
  AddObject (pObject);

  pObject = new MacAddrMibObject (kOID_mocaNodeMacAddress, kNAME_mocaNodeMacAddress, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaNodeTxGcdRate, kNAME_mocaNodeTxGcdRate, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_mocaNodeRxGcdPower, kNAME_mocaNodeRxGcdPower, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaNodeTxPowerReduction, kNAME_mocaNodeTxPowerReduction, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_mocaNodeRxPower, kNAME_mocaNodeRxPower, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_mocaNodePreferredNC, kNAME_mocaNodePreferredNC, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_mocaNodeQAM256Capable, kNAME_mocaNodeQAM256Capable, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_mocaNodePacketsAggrCapability, kNAME_mocaNodePacketsAggrCapability, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaNodeRxPackets, kNAME_mocaNodeRxPackets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaNodeRxCorrected, kNAME_mocaNodeRxCorrected, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaNodeRxDrops, kNAME_mocaNodeRxDrops, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaNodeSNR, kNAME_mocaNodeSNR, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS mocaNodeEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmMacAddress Mac;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_mocaNodeIndex:
      Status = VB.SetUint32 (fpBridge->Get_mocaNodeIndex (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaNodeMocaVersion:
      Status = VB.SetInt32 (fpBridge->Get_mocaNodeMocaVersion ());
      break;

    case kLEAF_mocaNodeMacAddress:
      fpBridge->Get_mocaNodeMacAddress (Mac);
      Status = VB.SetMacAddr (Mac);
      break;

    case kLEAF_mocaNodeTxGcdRate:
      Status = VB.SetUint32 (fpBridge->Get_mocaNodeTxGcdRate (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaNodeRxGcdPower:
      Status = VB.SetInt32 (fpBridge->Get_mocaNodeRxGcdPower ());
      break;

    case kLEAF_mocaNodeTxPowerReduction:
      Status = VB.SetUint32 (fpBridge->Get_mocaNodeTxPowerReduction (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaNodeRxPower:
      Status = VB.SetInt32 (fpBridge->Get_mocaNodeRxPower ());
      break;

    case kLEAF_mocaNodePreferredNC:
      Status = VB.SetBool (fpBridge->Get_mocaNodePreferredNC ());
      break;

    case kLEAF_mocaNodeQAM256Capable:
      Status = VB.SetBool (fpBridge->Get_mocaNodeQAM256Capable ());
      break;

    case kLEAF_mocaNodePacketsAggrCapability:
      Status = VB.SetInt32 (fpBridge->Get_mocaNodePacketsAggrCapability ());
      break;

    case kLEAF_mocaNodeRxPackets:
      Status = VB.SetUint32 (fpBridge->Get_mocaNodeRxPackets (), VBT_COUNTER);
      break;

    case kLEAF_mocaNodeRxCorrected:
      Status = VB.SetUint32 (fpBridge->Get_mocaNodeRxCorrected (), VBT_COUNTER);
      break;

    case kLEAF_mocaNodeRxDrops:
      Status = VB.SetUint32 (fpBridge->Get_mocaNodeRxDrops (), VBT_COUNTER);
      break;

    case kLEAF_mocaNodeSNR:
      Status = VB.SetUint32 (fpBridge->Get_mocaNodeSNR (), VBT_UNSIGNED32);
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
 *  mocaMeshTable and related objects 
\*/

/* mocaMeshTable */

mocaMeshTable::mocaMeshTable ()
  : BcmMibTable (kOID_mocaMeshTable, kNAME_mocaMeshTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << mocaMeshEntry::kLEAF_mocaMeshTxRate);
}

SNMP_STATUS mocaMeshTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  mocaMeshEntry *pEntry;
  mocaMeshEntryBridge *pMyBridge = (mocaMeshEntryBridge *) pBridge;


  pEntry = new mocaMeshEntry (this, pMyBridge);
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

/* mocaMeshEntry */

mocaMeshEntry::mocaMeshEntry (mocaMeshTable *pTable, mocaMeshEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_mocaMeshEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Uint32MibObject (kOID_mocaMeshTxNodeIndex, kNAME_mocaMeshTxNodeIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaMeshRxNodeIndex, kNAME_mocaMeshRxNodeIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_mocaMeshTxRate, kNAME_mocaMeshTxRate, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS mocaMeshEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_mocaMeshTxNodeIndex:
      Status = VB.SetUint32 (fpBridge->Get_mocaMeshTxNodeIndex (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaMeshRxNodeIndex:
      Status = VB.SetUint32 (fpBridge->Get_mocaMeshRxNodeIndex (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaMeshTxRate:
      Status = VB.SetUint32 (fpBridge->Get_mocaMeshTxRate (), VBT_UNSIGNED32);
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
 *  mocaBridgeTable and related objects 
\*/

/* mocaBridgeTable */

mocaBridgeTable::mocaBridgeTable ()
  : BcmMibTable (kOID_mocaBridgeTable, kNAME_mocaBridgeTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << mocaBridgeEntry::kLEAF_mocaBridgeMacAddress);
}

SNMP_STATUS mocaBridgeTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  mocaBridgeEntry *pEntry;
  mocaBridgeEntryBridge *pMyBridge = (mocaBridgeEntryBridge *) pBridge;


  pEntry = new mocaBridgeEntry (this, pMyBridge);
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

/* mocaBridgeEntry */

mocaBridgeEntry::mocaBridgeEntry (mocaBridgeTable *pTable, mocaBridgeEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_mocaBridgeEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Uint32MibObject (kOID_mocaBridgeNodeIndex, kNAME_mocaBridgeNodeIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_mocaBridgeMacIndex, kNAME_mocaBridgeMacIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new MacAddrMibObject (kOID_mocaBridgeMacAddress, kNAME_mocaBridgeMacAddress, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS mocaBridgeEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmMacAddress Mac;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_mocaBridgeNodeIndex:
      Status = VB.SetUint32 (fpBridge->Get_mocaBridgeNodeIndex (), VBT_UNSIGNED32);
      break;

    case kLEAF_mocaBridgeMacIndex:
      Status = VB.SetInt32 (fpBridge->Get_mocaBridgeMacIndex ());
      break;

    case kLEAF_mocaBridgeMacAddress:
      fpBridge->Get_mocaBridgeMacAddress (Mac);
      Status = VB.SetMacAddr (Mac);
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



