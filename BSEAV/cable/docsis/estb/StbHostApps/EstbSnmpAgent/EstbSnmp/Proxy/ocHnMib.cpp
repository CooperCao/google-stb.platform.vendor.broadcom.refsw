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
//    Filename: ocHnMib.cpp
//    Creation Date: Friday, June 10, 2011 at 11:07:23
//    Created by Broadcom BFC Mib Compiler BCMIBC.EXE version 3.6.1d
//    from input file oc-home-network.mib
//
//****************************************************************************

#include "ocHnMib.h"
#include "ocHnBridge.h"
#include "SnmpLog.h" // for SnmpLog... macros
#include "BcmSnmpAgent.h"


/*\
 *  ocHnMibModuleGroup group
\*/

ocHnMibModuleGroup::ocHnMibModuleGroup ()
  : BcmMibGroup (kOID_ocHnMibModuleGroup, kNAME_ocHnMibModuleGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocHnMibModuleGroup::Populate ()
{
  BcmMibGroup *pGroup;


  if (BcmMibGroup::Populate() == false)
    return false;

  pGroup = new ocHnMibObjectsGroup();
  AddGroup (pGroup);

  return true;
}

/*\
 *  ocHnMibObjectsGroup group
\*/

ocHnMibObjectsGroup::ocHnMibObjectsGroup ()
  : BcmMibGroup (kOID_ocHnMibObjectsGroup, kNAME_ocHnMibObjectsGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocHnMibObjectsGroup::Populate ()
{
  BcmMibGroup *pGroup;


  if (BcmMibGroup::Populate() == false)
    return false;

  pGroup = new ocHnHomeNetStatusGroup();
  AddGroup (pGroup);

  pGroup = new ocHnDevInterfaceStatusGroup();
  AddGroup (pGroup);

  return true;
}

/*\
 *  ocHnHomeNetStatusGroup group
\*/

ocHnHomeNetStatusGroup::ocHnHomeNetStatusGroup ()
  : BcmMibGroup (kOID_ocHnHomeNetStatusGroup, kNAME_ocHnHomeNetStatusGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocHnHomeNetStatusGroup::Populate ()
{
  BcmMibTable *pTable;
  BcmMibGroup *pGroup;


  if (BcmMibGroup::Populate() == false)
    return false;

  pTable = new ocHnDevUpnpServiceTable();
  AddTable (pTable);

  pTable = new ocHnDevSupportedChannelTable();
  AddTable (pTable);

  pGroup = new ocHnRsdManagerStatusGroup();
  AddGroup (pGroup);

  pGroup = new ocHnDevPropertiesGroup();
  AddGroup (pGroup);

  return true;
}


/*\ 
 *  ocHnDevUpnpServiceTable and related objects 
\*/

/* ocHnDevUpnpServiceTable */

ocHnDevUpnpServiceTable::ocHnDevUpnpServiceTable ()
  : BcmMibTable (kOID_ocHnDevUpnpServiceTable, kNAME_ocHnDevUpnpServiceTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ocHnDevUpnpServiceEntry::kLEAF_ocHnDevUpnpServiceType);
  fColumnMask |= (1 << ocHnDevUpnpServiceEntry::kLEAF_ocHnDevUpnpServiceAvailable);
}

SNMP_STATUS ocHnDevUpnpServiceTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ocHnDevUpnpServiceEntry *pEntry;
  ocHnDevUpnpServiceEntryBridge *pMyBridge = (ocHnDevUpnpServiceEntryBridge *) pBridge;


  pEntry = new ocHnDevUpnpServiceEntry (this, pMyBridge);
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

/* ocHnDevUpnpServiceEntry */

ocHnDevUpnpServiceEntry::ocHnDevUpnpServiceEntry (ocHnDevUpnpServiceTable *pTable, ocHnDevUpnpServiceEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ocHnDevUpnpServiceEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Uint32MibObject (kOID_ocHnDevUpnpServiceIndex, kNAME_ocHnDevUpnpServiceIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocHnDevUpnpServiceType, kNAME_ocHnDevUpnpServiceType, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocHnDevUpnpServiceAvailable, kNAME_ocHnDevUpnpServiceAvailable, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ocHnDevUpnpServiceEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocHnDevUpnpServiceIndex:
      Status = VB.SetUint32 (fpBridge->Get_ocHnDevUpnpServiceIndex (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocHnDevUpnpServiceType:
      Status = VB.SetInt32 (fpBridge->Get_ocHnDevUpnpServiceType ());
      break;

    case kLEAF_ocHnDevUpnpServiceAvailable:
      Status = VB.SetBool (fpBridge->Get_ocHnDevUpnpServiceAvailable ());
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
 *  ocHnDevSupportedChannelTable and related objects 
\*/

/* ocHnDevSupportedChannelTable */

ocHnDevSupportedChannelTable::ocHnDevSupportedChannelTable ()
  : BcmMibTable (kOID_ocHnDevSupportedChannelTable, kNAME_ocHnDevSupportedChannelTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ocHnDevSupportedChannelEntry::kLEAF_ocHnDevSupportedChannelLastOperatingFreq);
  fColumnMask |= (1 << ocHnDevSupportedChannelEntry::kLEAF_ocHnDevSupportedChannelInUse);
  fColumnMask |= (1 << ocHnDevSupportedChannelEntry::kLEAF_ocHnDevSupportedChannelFrequency);
  fColumnMask |= (1 << ocHnDevSupportedChannelEntry::kLEAF_ocHnDevSupportedChannelEligible);
}

SNMP_STATUS ocHnDevSupportedChannelTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ocHnDevSupportedChannelEntry *pEntry;
  ocHnDevSupportedChannelEntryBridge *pMyBridge = (ocHnDevSupportedChannelEntryBridge *) pBridge;


  pEntry = new ocHnDevSupportedChannelEntry (this, pMyBridge);
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

/* ocHnDevSupportedChannelEntry */

ocHnDevSupportedChannelEntry::ocHnDevSupportedChannelEntry (ocHnDevSupportedChannelTable *pTable, ocHnDevSupportedChannelEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ocHnDevSupportedChannelEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Uint32MibObject (kOID_ocHnDevSupportedChannelIndex, kNAME_ocHnDevSupportedChannelIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocHnDevSupportedChannelLastOperatingFreq, kNAME_ocHnDevSupportedChannelLastOperatingFreq, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocHnDevSupportedChannelInUse, kNAME_ocHnDevSupportedChannelInUse, this, kReadWrite);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocHnDevSupportedChannelFrequency, kNAME_ocHnDevSupportedChannelFrequency, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocHnDevSupportedChannelEligible, kNAME_ocHnDevSupportedChannelEligible, this, kReadWrite);
  AddObject (pObject);
}

SNMP_STATUS ocHnDevSupportedChannelEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocHnDevSupportedChannelIndex:
      Status = VB.SetUint32 (fpBridge->Get_ocHnDevSupportedChannelIndex (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocHnDevSupportedChannelLastOperatingFreq:
      Status = VB.SetBool (fpBridge->Get_ocHnDevSupportedChannelLastOperatingFreq ());
      break;

    case kLEAF_ocHnDevSupportedChannelInUse:
      Status = VB.SetBool (fpBridge->Get_ocHnDevSupportedChannelInUse ());
      break;

    case kLEAF_ocHnDevSupportedChannelFrequency:
      Status = VB.SetUint32 (fpBridge->Get_ocHnDevSupportedChannelFrequency (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocHnDevSupportedChannelEligible:
      Status = VB.SetBool (fpBridge->Get_ocHnDevSupportedChannelEligible ());
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

SNMP_STATUS ocHnDevSupportedChannelEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocHnDevSupportedChannelIndex:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_ocHnDevSupportedChannelLastOperatingFreq:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocHnDevSupportedChannelInUse:
    {
      bool ocHnDevSupportedChannelInUse;

      Status = VB.GetBool (&ocHnDevSupportedChannelInUse);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ocHnDevSupportedChannelInUse (ocHnDevSupportedChannelInUse);
      break;
    }

    case kLEAF_ocHnDevSupportedChannelFrequency:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocHnDevSupportedChannelEligible:
    {
      bool ocHnDevSupportedChannelEligible;

      Status = VB.GetBool (&ocHnDevSupportedChannelEligible);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ocHnDevSupportedChannelEligible (ocHnDevSupportedChannelEligible);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}

/*\
 *  ocHnRsdManagerStatusGroup group
\*/

ocHnRsdManagerStatusGroup::ocHnRsdManagerStatusGroup ()
  : BcmMibGroup (kOID_ocHnRsdManagerStatusGroup, kNAME_ocHnRsdManagerStatusGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocHnRsdManagerStatusGroup::Populate ()
{
  BcmMibObject *pScalar;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new Uint32MibObject (kOID_ocHnRsdManagerImportanceNumber, kNAME_ocHnRsdManagerImportanceNumber, this, kReadWrite);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ocHnRsdManagerPreferredStatus, kNAME_ocHnRsdManagerPreferredStatus, this, kReadOnly);
  AddScalar (pScalar);

  return true;
}

SNMP_STATUS ocHnRsdManagerStatusGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ocHnRsdManagerStatusGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ocHnRsdManagerStatusGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocHnRsdManagerImportanceNumber:
      Status = VB.SetUint32 (fpBridge->Get_ocHnRsdManagerImportanceNumber (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocHnRsdManagerPreferredStatus:
      Status = VB.SetBool (fpBridge->Get_ocHnRsdManagerPreferredStatus ());
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

SNMP_STATUS ocHnRsdManagerStatusGroup::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocHnRsdManagerImportanceNumber:
    {
      unsigned int ocHnRsdManagerImportanceNumber;

      Status = VB.GetUint32 (&ocHnRsdManagerImportanceNumber);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ocHnRsdManagerImportanceNumber (ocHnRsdManagerImportanceNumber);
      break;
    }

    case kLEAF_ocHnRsdManagerPreferredStatus:
      Status = SNMP_NOT_WRITABLE;
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}

/*\
 *  ocHnDevPropertiesGroup group
\*/

ocHnDevPropertiesGroup::ocHnDevPropertiesGroup ()
  : BcmMibGroup (kOID_ocHnDevPropertiesGroup, kNAME_ocHnDevPropertiesGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocHnDevPropertiesGroup::Populate ()
{
  BcmMibObject *pScalar;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new StringMibObject (kOID_ocHnDevPropertiesFriendlyName, kNAME_ocHnDevPropertiesFriendlyName, this, kReadOnly);
  AddScalar (pScalar);

  return true;
}

SNMP_STATUS ocHnDevPropertiesGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ocHnDevPropertiesGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ocHnDevPropertiesGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocHnDevPropertiesFriendlyName:
    {
      BcmString String;

      fpBridge->Get_ocHnDevPropertiesFriendlyName (String);
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
 *  ocHnDevInterfaceStatusGroup group
\*/

ocHnDevInterfaceStatusGroup::ocHnDevInterfaceStatusGroup ()
  : BcmMibGroup (kOID_ocHnDevInterfaceStatusGroup, kNAME_ocHnDevInterfaceStatusGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ocHnDevInterfaceStatusGroup::Populate ()
{
  BcmMibTable *pTable;


  if (BcmMibGroup::Populate() == false)
    return false;

  pTable = new ocHnDevInterfaceConfigTable();
  AddTable (pTable);

  pTable = new ocHnDevInterfaceStatusTable();
  AddTable (pTable);

  pTable = new ocHnDevConnectionTable();
  AddTable (pTable);

  return true;
}


/*\ 
 *  ocHnDevInterfaceConfigTable and related objects 
\*/

/* ocHnDevInterfaceConfigTable */

ocHnDevInterfaceConfigTable::ocHnDevInterfaceConfigTable ()
  : BcmMibTable (kOID_ocHnDevInterfaceConfigTable, kNAME_ocHnDevInterfaceConfigTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ocHnDevInterfaceConfigEntry::kLEAF_ocHnDevInterfaceConfigMaxPowerLevel);
  fColumnMask |= (1 << ocHnDevInterfaceConfigEntry::kLEAF_ocHnDevInterfaceConfigPowerUnits);
  fColumnMask |= (1 << ocHnDevInterfaceConfigEntry::kLEAF_ocHnDevInterfaceConfigMaxParameterizedBandwidth);
  fColumnMask |= (1 << ocHnDevInterfaceConfigEntry::kLEAF_ocHnDevInterfaceConfigEnableEncryption);
  fColumnMask |= (1 << ocHnDevInterfaceConfigEntry::kLEAF_ocHnDevInterfaceConfigEncryptionPassphrase);
}

SNMP_STATUS ocHnDevInterfaceConfigTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ocHnDevInterfaceConfigEntry *pEntry;
  ocHnDevInterfaceConfigEntryBridge *pMyBridge = (ocHnDevInterfaceConfigEntryBridge *) pBridge;


  pEntry = new ocHnDevInterfaceConfigEntry (this, pMyBridge);
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

/* ocHnDevInterfaceConfigEntry */

ocHnDevInterfaceConfigEntry::ocHnDevInterfaceConfigEntry (ocHnDevInterfaceConfigTable *pTable, ocHnDevInterfaceConfigEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ocHnDevInterfaceConfigEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_ocHnDevInterfaceConfigMaxPowerLevel, kNAME_ocHnDevInterfaceConfigMaxPowerLevel, this, kReadWrite);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocHnDevInterfaceConfigPowerUnits, kNAME_ocHnDevInterfaceConfigPowerUnits, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocHnDevInterfaceConfigMaxParameterizedBandwidth, kNAME_ocHnDevInterfaceConfigMaxParameterizedBandwidth, this, kReadWrite);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocHnDevInterfaceConfigEnableEncryption, kNAME_ocHnDevInterfaceConfigEnableEncryption, this, kReadWrite);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_ocHnDevInterfaceConfigEncryptionPassphrase, kNAME_ocHnDevInterfaceConfigEncryptionPassphrase, this, kReadWrite);
  AddObject (pObject);
}

SNMP_STATUS ocHnDevInterfaceConfigEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocHnDevInterfaceConfigMaxPowerLevel:
      Status = VB.SetInt32 (fpBridge->Get_ocHnDevInterfaceConfigMaxPowerLevel ());
      break;

    case kLEAF_ocHnDevInterfaceConfigPowerUnits:
      Status = VB.SetInt32 (fpBridge->Get_ocHnDevInterfaceConfigPowerUnits ());
      break;

    case kLEAF_ocHnDevInterfaceConfigMaxParameterizedBandwidth:
      Status = VB.SetUint32 (fpBridge->Get_ocHnDevInterfaceConfigMaxParameterizedBandwidth (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocHnDevInterfaceConfigEnableEncryption:
      Status = VB.SetInt32 (fpBridge->Get_ocHnDevInterfaceConfigEnableEncryption ());
      break;

    case kLEAF_ocHnDevInterfaceConfigEncryptionPassphrase:
    {
      BcmString String;

      fpBridge->Get_ocHnDevInterfaceConfigEncryptionPassphrase (String);
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

SNMP_STATUS ocHnDevInterfaceConfigEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocHnDevInterfaceConfigMaxPowerLevel:
    {
      int ocHnDevInterfaceConfigMaxPowerLevel;

      Status = VB.GetInt32 (&ocHnDevInterfaceConfigMaxPowerLevel);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ocHnDevInterfaceConfigMaxPowerLevel (ocHnDevInterfaceConfigMaxPowerLevel);
      break;
    }

    case kLEAF_ocHnDevInterfaceConfigPowerUnits:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ocHnDevInterfaceConfigMaxParameterizedBandwidth:
    {
      unsigned int ocHnDevInterfaceConfigMaxParameterizedBandwidth;

      Status = VB.GetUint32 (&ocHnDevInterfaceConfigMaxParameterizedBandwidth);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ocHnDevInterfaceConfigMaxParameterizedBandwidth (ocHnDevInterfaceConfigMaxParameterizedBandwidth);
      break;
    }

    case kLEAF_ocHnDevInterfaceConfigEnableEncryption:
    {
      int ocHnDevInterfaceConfigEnableEncryption;

      Status = VB.GetInt32 (&ocHnDevInterfaceConfigEnableEncryption);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ocHnDevInterfaceConfigEnableEncryption (ocHnDevInterfaceConfigEnableEncryption);
      break;
    }

    case kLEAF_ocHnDevInterfaceConfigEncryptionPassphrase:
    {
      BcmString String;

      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ocHnDevInterfaceConfigEncryptionPassphrase (String);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}


/*\ 
 *  ocHnDevInterfaceStatusTable and related objects 
\*/

/* ocHnDevInterfaceStatusTable */

ocHnDevInterfaceStatusTable::ocHnDevInterfaceStatusTable ()
  : BcmMibTable (kOID_ocHnDevInterfaceStatusTable, kNAME_ocHnDevInterfaceStatusTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ocHnDevInterfaceStatusEntry::kLEAF_ocHnDevInterfaceStatusTxBroadcastRate);
  fColumnMask |= (1 << ocHnDevInterfaceStatusEntry::kLEAF_ocHnDevInterfaceStatusTxBroadcastLevel);
  fColumnMask |= (1 << ocHnDevInterfaceStatusEntry::kLEAF_ocHnDevInterfaceStatusMaxTxPowerLevel);
  fColumnMask |= (1 << ocHnDevInterfaceStatusEntry::kLEAF_ocHnDevInterfaceStatusPowerUnits);
  fColumnMask |= (1 << ocHnDevInterfaceStatusEntry::kLEAF_ocHnDevInterfaceStatusMaxParameterizedBandwidth);
  fColumnMask |= (1 << ocHnDevInterfaceStatusEntry::kLEAF_ocHnDevInterfaceStatusLayer2Scheduler);
}

SNMP_STATUS ocHnDevInterfaceStatusTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ocHnDevInterfaceStatusEntry *pEntry;
  ocHnDevInterfaceStatusEntryBridge *pMyBridge = (ocHnDevInterfaceStatusEntryBridge *) pBridge;


  pEntry = new ocHnDevInterfaceStatusEntry (this, pMyBridge);
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

/* ocHnDevInterfaceStatusEntry */

ocHnDevInterfaceStatusEntry::ocHnDevInterfaceStatusEntry (ocHnDevInterfaceStatusTable *pTable, ocHnDevInterfaceStatusEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ocHnDevInterfaceStatusEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Uint32MibObject (kOID_ocHnDevInterfaceStatusTxBroadcastRate, kNAME_ocHnDevInterfaceStatusTxBroadcastRate, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocHnDevInterfaceStatusTxBroadcastLevel, kNAME_ocHnDevInterfaceStatusTxBroadcastLevel, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocHnDevInterfaceStatusMaxTxPowerLevel, kNAME_ocHnDevInterfaceStatusMaxTxPowerLevel, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocHnDevInterfaceStatusPowerUnits, kNAME_ocHnDevInterfaceStatusPowerUnits, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocHnDevInterfaceStatusMaxParameterizedBandwidth, kNAME_ocHnDevInterfaceStatusMaxParameterizedBandwidth, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocHnDevInterfaceStatusLayer2Scheduler, kNAME_ocHnDevInterfaceStatusLayer2Scheduler, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ocHnDevInterfaceStatusEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocHnDevInterfaceStatusTxBroadcastRate:
      Status = VB.SetUint32 (fpBridge->Get_ocHnDevInterfaceStatusTxBroadcastRate (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocHnDevInterfaceStatusTxBroadcastLevel:
      Status = VB.SetInt32 (fpBridge->Get_ocHnDevInterfaceStatusTxBroadcastLevel ());
      break;

    case kLEAF_ocHnDevInterfaceStatusMaxTxPowerLevel:
      Status = VB.SetInt32 (fpBridge->Get_ocHnDevInterfaceStatusMaxTxPowerLevel ());
      break;

    case kLEAF_ocHnDevInterfaceStatusPowerUnits:
      Status = VB.SetInt32 (fpBridge->Get_ocHnDevInterfaceStatusPowerUnits ());
      break;

    case kLEAF_ocHnDevInterfaceStatusMaxParameterizedBandwidth:
      Status = VB.SetUint32 (fpBridge->Get_ocHnDevInterfaceStatusMaxParameterizedBandwidth (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocHnDevInterfaceStatusLayer2Scheduler:
      Status = VB.SetBool (fpBridge->Get_ocHnDevInterfaceStatusLayer2Scheduler ());
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
 *  ocHnDevConnectionTable and related objects 
\*/

/* ocHnDevConnectionTable */

ocHnDevConnectionTable::ocHnDevConnectionTable ()
  : BcmMibTable (kOID_ocHnDevConnectionTable, kNAME_ocHnDevConnectionTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ocHnDevConnectionEntry::kLEAF_ocHnDevConnectionDestMac);
  fColumnMask |= (1 << ocHnDevConnectionEntry::kLEAF_ocHnDevConnectionRxLinkRate);
  fColumnMask |= (1 << ocHnDevConnectionEntry::kLEAF_ocHnDevConnectionRxModulationType);
  fColumnMask |= (1 << ocHnDevConnectionEntry::kLEAF_ocHnDevConnectionRxBroadcastRate);
  fColumnMask |= (1 << ocHnDevConnectionEntry::kLEAF_ocHnDevConnectionRxSignalStrength);
  fColumnMask |= (1 << ocHnDevConnectionEntry::kLEAF_ocHnDevConnectionTxUnicastLevel);
  fColumnMask |= (1 << ocHnDevConnectionEntry::kLEAF_ocHnDevConnectionPowerUnits);
  fColumnMask |= (1 << ocHnDevConnectionEntry::kLEAF_ocHnDevConnectionTxLinkRate);
  fColumnMask |= (1 << ocHnDevConnectionEntry::kLEAF_ocHnDevConnectionTxModulationType);
}

SNMP_STATUS ocHnDevConnectionTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ocHnDevConnectionEntry *pEntry;
  ocHnDevConnectionEntryBridge *pMyBridge = (ocHnDevConnectionEntryBridge *) pBridge;


  pEntry = new ocHnDevConnectionEntry (this, pMyBridge);
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

/* ocHnDevConnectionEntry */

ocHnDevConnectionEntry::ocHnDevConnectionEntry (ocHnDevConnectionTable *pTable, ocHnDevConnectionEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ocHnDevConnectionEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Uint32MibObject (kOID_ocHnDevConnectionIndex, kNAME_ocHnDevConnectionIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new MacAddrMibObject (kOID_ocHnDevConnectionDestMac, kNAME_ocHnDevConnectionDestMac, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocHnDevConnectionRxLinkRate, kNAME_ocHnDevConnectionRxLinkRate, this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_ocHnDevConnectionRxModulationType, kNAME_ocHnDevConnectionRxModulationType, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocHnDevConnectionRxBroadcastRate, kNAME_ocHnDevConnectionRxBroadcastRate, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocHnDevConnectionRxSignalStrength, kNAME_ocHnDevConnectionRxSignalStrength, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocHnDevConnectionTxUnicastLevel, kNAME_ocHnDevConnectionTxUnicastLevel, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ocHnDevConnectionPowerUnits, kNAME_ocHnDevConnectionPowerUnits, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ocHnDevConnectionTxLinkRate, kNAME_ocHnDevConnectionTxLinkRate, this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_ocHnDevConnectionTxModulationType, kNAME_ocHnDevConnectionTxModulationType, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ocHnDevConnectionEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmMacAddress Mac;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ocHnDevConnectionIndex:
      Status = VB.SetUint32 (fpBridge->Get_ocHnDevConnectionIndex (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocHnDevConnectionDestMac:
      fpBridge->Get_ocHnDevConnectionDestMac (Mac);
      Status = VB.SetMacAddr (Mac);
      break;

    case kLEAF_ocHnDevConnectionRxLinkRate:
      Status = VB.SetUint32 (fpBridge->Get_ocHnDevConnectionRxLinkRate (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocHnDevConnectionRxModulationType:
      fpBridge->Get_ocHnDevConnectionRxModulationType (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_ocHnDevConnectionRxBroadcastRate:
      Status = VB.SetUint32 (fpBridge->Get_ocHnDevConnectionRxBroadcastRate (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocHnDevConnectionRxSignalStrength:
      Status = VB.SetInt32 (fpBridge->Get_ocHnDevConnectionRxSignalStrength ());
      break;

    case kLEAF_ocHnDevConnectionTxUnicastLevel:
      Status = VB.SetInt32 (fpBridge->Get_ocHnDevConnectionTxUnicastLevel ());
      break;

    case kLEAF_ocHnDevConnectionPowerUnits:
      Status = VB.SetInt32 (fpBridge->Get_ocHnDevConnectionPowerUnits ());
      break;

    case kLEAF_ocHnDevConnectionTxLinkRate:
      Status = VB.SetUint32 (fpBridge->Get_ocHnDevConnectionTxLinkRate (), VBT_UNSIGNED32);
      break;

    case kLEAF_ocHnDevConnectionTxModulationType:
      fpBridge->Get_ocHnDevConnectionTxModulationType (String);
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



