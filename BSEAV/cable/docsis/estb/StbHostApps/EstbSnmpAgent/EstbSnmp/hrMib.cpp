//**************************************************************************
//                                                                          
//    Copyright 2008 Broadcom Corporation                                   
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
//    Filename: hrMib.cpp
//    Creation Date: Wednesday, April 09, 2008 at 15:28:58
//    Created by Broadcom BFC / V2 Mib Compiler BCMIBC.EXE version 3.0.0k
//    from input file ocHostHr.mib
//
//**************************************************************************

#include "hrMib.h"
#include "hrBridge.h"
#include "SnmpLog.h" // for SnmpLog... macros
#include "BcmSnmpAgent.h"


/*\
 *  hostGroup group
\*/

hostGroup::hostGroup ()
  : BcmMibGroup (kOID_hostGroup, kNAME_hostGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool hostGroup::Populate ()
{
  BcmMibGroup *pGroup;


  if (BcmMibGroup::Populate() == false)
    return false;

  pGroup = new hrStorageGroup();
  AddGroup (pGroup);

  pGroup = new hrDeviceGroup();
  AddGroup (pGroup);

  pGroup = new hrSWRunGroup();
  AddGroup (pGroup);

  pGroup = new hrSWRunPerfGroup();
  AddGroup (pGroup);

  return true;
}

/*\
 *  hrStorageGroup group
\*/

hrStorageGroup::hrStorageGroup ()
  : BcmMibGroup (kOID_hrStorageGroup, kNAME_hrStorageGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool hrStorageGroup::Populate ()
{
  BcmMibObject *pScalar;
  BcmMibTable *pTable;
  BcmMibGroup *pGroup;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new Int32MibObject (kOID_hrMemorySize, kNAME_hrMemorySize, this, kReadOnly);
  AddScalar (pScalar);

  pTable = new hrStorageTable();
  AddTable (pTable);

  return true;
}

SNMP_STATUS hrStorageGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (hrStorageGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS hrStorageGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_hrMemorySize:
      Status = VB.SetInt32 (fpBridge->Get_hrMemorySize ());
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
 *  hrStorageTable and related objects 
\*/

/* hrStorageTable */

hrStorageTable::hrStorageTable ()
  : BcmMibTable (kOID_hrStorageTable, kNAME_hrStorageTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << hrStorageEntry::kLEAF_hrStorageIndex);
  fColumnMask |= (1 << hrStorageEntry::kLEAF_hrStorageType);
  fColumnMask |= (1 << hrStorageEntry::kLEAF_hrStorageDescr);
  fColumnMask |= (1 << hrStorageEntry::kLEAF_hrStorageAllocationUnits);
  fColumnMask |= (1 << hrStorageEntry::kLEAF_hrStorageSize);
  fColumnMask |= (1 << hrStorageEntry::kLEAF_hrStorageUsed);
  fColumnMask |= (1 << hrStorageEntry::kLEAF_hrStorageAllocationFailures);
}

SNMP_STATUS hrStorageTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  hrStorageEntry *pEntry;
  hrStorageEntryBridge *pMyBridge = (hrStorageEntryBridge *) pBridge;


  pEntry = new hrStorageEntry (this, pMyBridge);
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

/* hrStorageEntry */

hrStorageEntry::hrStorageEntry (hrStorageTable *pTable, hrStorageEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_hrStorageEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_hrStorageIndex, kNAME_hrStorageIndex, this, kReadOnly);
  AddObject (pObject);

  pObject = new OidMibObject (kOID_hrStorageType, kNAME_hrStorageType, this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_hrStorageDescr, kNAME_hrStorageDescr, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_hrStorageAllocationUnits, kNAME_hrStorageAllocationUnits, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_hrStorageSize, kNAME_hrStorageSize, this, kReadWrite);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_hrStorageUsed, kNAME_hrStorageUsed, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_hrStorageAllocationFailures, kNAME_hrStorageAllocationFailures, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS hrStorageEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmObjectId Oid;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_hrStorageIndex:
      Status = VB.SetInt32 (fpBridge->Get_hrStorageIndex ());
      break;

    case kLEAF_hrStorageType:
      fpBridge->Get_hrStorageType (Oid);
      Status = VB.SetOid (Oid);
      break;

    case kLEAF_hrStorageDescr:
    {
      BcmString String;

      fpBridge->Get_hrStorageDescr (String);
      Status = VB.SetString (String);
      break;
    }

    case kLEAF_hrStorageAllocationUnits:
      Status = VB.SetInt32 (fpBridge->Get_hrStorageAllocationUnits ());
      break;

    case kLEAF_hrStorageSize:
      Status = VB.SetInt32 (fpBridge->Get_hrStorageSize ());
      break;

    case kLEAF_hrStorageUsed:
      Status = VB.SetInt32 (fpBridge->Get_hrStorageUsed ());
      break;

    case kLEAF_hrStorageAllocationFailures:
      Status = VB.SetUint32 (fpBridge->Get_hrStorageAllocationFailures (), VBT_COUNTER);
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

SNMP_STATUS hrStorageEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmObjectId Oid;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_hrStorageIndex:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_hrStorageType:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_hrStorageDescr:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_hrStorageAllocationUnits:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_hrStorageSize:
    {
      int hrStorageSize;

      Status = VB.GetInt32 (&hrStorageSize);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_hrStorageSize (hrStorageSize);
      break;
    }

    case kLEAF_hrStorageUsed:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_hrStorageAllocationFailures:
      Status = SNMP_NOT_WRITABLE;
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}

/*\
 *  hrDeviceGroup group
\*/

hrDeviceGroup::hrDeviceGroup ()
  : BcmMibGroup (kOID_hrDeviceGroup, kNAME_hrDeviceGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool hrDeviceGroup::Populate ()
{
  BcmMibTable *pTable;
  BcmMibGroup *pGroup;


  if (BcmMibGroup::Populate() == false)
    return false;

  pTable = new hrDeviceTable();
  AddTable (pTable);

  pTable = new hrProcessorTable();
  AddTable (pTable);

  return true;
}


/*\ 
 *  hrDeviceTable and related objects 
\*/

/* hrDeviceTable */

hrDeviceTable::hrDeviceTable ()
  : BcmMibTable (kOID_hrDeviceTable, kNAME_hrDeviceTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << hrDeviceEntry::kLEAF_hrDeviceIndex);
  fColumnMask |= (1 << hrDeviceEntry::kLEAF_hrDeviceType);
  fColumnMask |= (1 << hrDeviceEntry::kLEAF_hrDeviceDescr);
  fColumnMask |= (1 << hrDeviceEntry::kLEAF_hrDeviceID);
  fColumnMask |= (1 << hrDeviceEntry::kLEAF_hrDeviceStatus);
  fColumnMask |= (1 << hrDeviceEntry::kLEAF_hrDeviceErrors);
}

SNMP_STATUS hrDeviceTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  hrDeviceEntry *pEntry;
  hrDeviceEntryBridge *pMyBridge = (hrDeviceEntryBridge *) pBridge;


  pEntry = new hrDeviceEntry (this, pMyBridge);
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

/* hrDeviceEntry */

hrDeviceEntry::hrDeviceEntry (hrDeviceTable *pTable, hrDeviceEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_hrDeviceEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_hrDeviceIndex, kNAME_hrDeviceIndex, this, kReadOnly);
  AddObject (pObject);

  pObject = new OidMibObject (kOID_hrDeviceType, kNAME_hrDeviceType, this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_hrDeviceDescr, kNAME_hrDeviceDescr, this, kReadOnly);
  AddObject (pObject);

  pObject = new OidMibObject (kOID_hrDeviceID, kNAME_hrDeviceID, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_hrDeviceStatus, kNAME_hrDeviceStatus, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_hrDeviceErrors, kNAME_hrDeviceErrors, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS hrDeviceEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmObjectId Oid;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_hrDeviceIndex:
      Status = VB.SetInt32 (fpBridge->Get_hrDeviceIndex ());
      break;

    case kLEAF_hrDeviceType:
      fpBridge->Get_hrDeviceType (Oid);
      Status = VB.SetOid (Oid);
      break;

    case kLEAF_hrDeviceDescr:
    {
      BcmString String;

      fpBridge->Get_hrDeviceDescr (String);
      Status = VB.SetString (String);
      break;
    }

    case kLEAF_hrDeviceID:
      fpBridge->Get_hrDeviceID (Oid);
      Status = VB.SetOid (Oid);
      break;

    case kLEAF_hrDeviceStatus:
      Status = VB.SetInt32 (fpBridge->Get_hrDeviceStatus ());
      break;

    case kLEAF_hrDeviceErrors:
      Status = VB.SetUint32 (fpBridge->Get_hrDeviceErrors (), VBT_COUNTER);
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
 *  hrProcessorTable and related objects 
\*/

/* hrProcessorTable */

hrProcessorTable::hrProcessorTable ()
  : BcmMibTable (kOID_hrProcessorTable, kNAME_hrProcessorTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << hrProcessorEntry::kLEAF_hrProcessorFrwID);
  fColumnMask |= (1 << hrProcessorEntry::kLEAF_hrProcessorLoad);
}

SNMP_STATUS hrProcessorTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  hrProcessorEntry *pEntry;
  hrProcessorEntryBridge *pMyBridge = (hrProcessorEntryBridge *) pBridge;


  pEntry = new hrProcessorEntry (this, pMyBridge);
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

/* hrProcessorEntry */

hrProcessorEntry::hrProcessorEntry (hrProcessorTable *pTable, hrProcessorEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_hrProcessorEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new OidMibObject (kOID_hrProcessorFrwID, kNAME_hrProcessorFrwID, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_hrProcessorLoad, kNAME_hrProcessorLoad, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS hrProcessorEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmObjectId Oid;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_hrProcessorFrwID:
      fpBridge->Get_hrProcessorFrwID (Oid);
      Status = VB.SetOid (Oid);
      break;

    case kLEAF_hrProcessorLoad:
      Status = VB.SetInt32 (fpBridge->Get_hrProcessorLoad ());
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
 *  hrSWRunGroup group
\*/

hrSWRunGroup::hrSWRunGroup ()
  : BcmMibGroup (kOID_hrSWRunGroup, kNAME_hrSWRunGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool hrSWRunGroup::Populate ()
{
  BcmMibObject *pScalar;
  BcmMibTable *pTable;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new Int32MibObject (kOID_hrSWOSIndex, kNAME_hrSWOSIndex, this, kReadOnly);
  AddScalar (pScalar);

  pTable = new hrSWRunTable();
  AddTable (pTable);

  return true;
}

SNMP_STATUS hrSWRunGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (hrSWRunGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS hrSWRunGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_hrSWOSIndex:
      Status = VB.SetInt32 (fpBridge->Get_hrSWOSIndex ());
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
 *  hrSWRunTable and related objects 
\*/

/* hrSWRunTable */

hrSWRunTable::hrSWRunTable ()
  : BcmMibTable (kOID_hrSWRunTable, kNAME_hrSWRunTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << hrSWRunEntry::kLEAF_hrSWRunIndex);
  fColumnMask |= (1 << hrSWRunEntry::kLEAF_hrSWRunName);
  fColumnMask |= (1 << hrSWRunEntry::kLEAF_hrSWRunID);
  fColumnMask |= (1 << hrSWRunEntry::kLEAF_hrSWRunPath);
  fColumnMask |= (1 << hrSWRunEntry::kLEAF_hrSWRunParameters);
  fColumnMask |= (1 << hrSWRunEntry::kLEAF_hrSWRunType);
  fColumnMask |= (1 << hrSWRunEntry::kLEAF_hrSWRunStatus);
}

SNMP_STATUS hrSWRunTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  hrSWRunEntry *pEntry;
  hrSWRunEntryBridge *pMyBridge = (hrSWRunEntryBridge *) pBridge;


  pEntry = new hrSWRunEntry (this, pMyBridge);
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

/* hrSWRunEntry */

hrSWRunEntry::hrSWRunEntry (hrSWRunTable *pTable, hrSWRunEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_hrSWRunEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_hrSWRunIndex, kNAME_hrSWRunIndex, this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_hrSWRunName, kNAME_hrSWRunName, this, kReadOnly);
  AddObject (pObject);

  pObject = new OidMibObject (kOID_hrSWRunID, kNAME_hrSWRunID, this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_hrSWRunPath, kNAME_hrSWRunPath, this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_hrSWRunParameters, kNAME_hrSWRunParameters, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_hrSWRunType, kNAME_hrSWRunType, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_hrSWRunStatus, kNAME_hrSWRunStatus, this, kReadWrite);
  AddObject (pObject);
}

SNMP_STATUS hrSWRunEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmObjectId Oid;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_hrSWRunIndex:
      Status = VB.SetInt32 (fpBridge->Get_hrSWRunIndex ());
      break;

    case kLEAF_hrSWRunName:
      fpBridge->Get_hrSWRunName (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_hrSWRunID:
      fpBridge->Get_hrSWRunID (Oid);
      Status = VB.SetOid (Oid);
      break;

    case kLEAF_hrSWRunPath:
      fpBridge->Get_hrSWRunPath (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_hrSWRunParameters:
      fpBridge->Get_hrSWRunParameters (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_hrSWRunType:
      Status = VB.SetInt32 (fpBridge->Get_hrSWRunType ());
      break;

    case kLEAF_hrSWRunStatus:
      Status = VB.SetInt32 (fpBridge->Get_hrSWRunStatus ());
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

SNMP_STATUS hrSWRunEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmObjectId Oid;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_hrSWRunIndex:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_hrSWRunName:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_hrSWRunID:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_hrSWRunPath:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_hrSWRunParameters:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_hrSWRunType:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_hrSWRunStatus:
    {
      int hrSWRunStatus;

      Status = VB.GetInt32 (&hrSWRunStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_hrSWRunStatus (hrSWRunStatus);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}

/*\
 *  hrSWRunPerfGroup group
\*/

hrSWRunPerfGroup::hrSWRunPerfGroup ()
  : BcmMibGroup (kOID_hrSWRunPerfGroup, kNAME_hrSWRunPerfGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool hrSWRunPerfGroup::Populate ()
{
  BcmMibTable *pTable;


  if (BcmMibGroup::Populate() == false)
    return false;

  pTable = new hrSWRunPerfTable();
  AddTable (pTable);

  return true;
}


/*\ 
 *  hrSWRunPerfTable and related objects 
\*/

/* hrSWRunPerfTable */

hrSWRunPerfTable::hrSWRunPerfTable ()
  : BcmMibTable (kOID_hrSWRunPerfTable, kNAME_hrSWRunPerfTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << hrSWRunPerfEntry::kLEAF_hrSWRunPerfCPU);
  fColumnMask |= (1 << hrSWRunPerfEntry::kLEAF_hrSWRunPerfMem);
}

SNMP_STATUS hrSWRunPerfTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  hrSWRunPerfEntry *pEntry;
  hrSWRunPerfEntryBridge *pMyBridge = (hrSWRunPerfEntryBridge *) pBridge;


  pEntry = new hrSWRunPerfEntry (this, pMyBridge);
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

/* hrSWRunPerfEntry */

hrSWRunPerfEntry::hrSWRunPerfEntry (hrSWRunPerfTable *pTable, hrSWRunPerfEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_hrSWRunPerfEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_hrSWRunPerfCPU, kNAME_hrSWRunPerfCPU, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_hrSWRunPerfMem, kNAME_hrSWRunPerfMem, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS hrSWRunPerfEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_hrSWRunPerfCPU:
      Status = VB.SetInt32 (fpBridge->Get_hrSWRunPerfCPU ());
      break;

    case kLEAF_hrSWRunPerfMem:
      Status = VB.SetInt32 (fpBridge->Get_hrSWRunPerfMem ());
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

