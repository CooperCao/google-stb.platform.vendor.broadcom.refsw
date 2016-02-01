//**************************************************************************
//                                                                          
//    Copyright 2004 Broadcom Corporation                                   
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
//    Filename: systemNewMib.cpp
//    Creation Date: Wednesday, February 04, 2004 at 15:32:40
//    Created by Broadcom BFC / V2 Mib Compiler BCMIBC.EXE version 3.0.0b
//    from input file rfc3418.mib
//
//**************************************************************************

#include "systemMib.h"
#include "systemBridge.h"
#include "SnmpLog.h" // for SnmpLog... macros
#include "BcmSnmpAgent.h"


/*\
 *  systemGroup group
\*/

systemGroup::systemGroup ()
  : BcmMibGroup ("1.3.6.1.2.1.1", "systemGroup")
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool systemGroup::Populate ()
{
  BcmMibObject *pScalar;
  BcmMibTable *pTable;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new StringMibObject ("1.3.6.1.2.1.1.1.0", "sysDescr", this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new OidMibObject ("1.3.6.1.2.1.1.2.0", "sysObjectID", this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new TimeticksMibObject ("1.3.6.1.2.1.1.3.0", "sysUpTime", this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new StringMibObject ("1.3.6.1.2.1.1.4.0", "sysContact", this, kReadWrite);
  AddScalar (pScalar);

  pScalar = new StringMibObject ("1.3.6.1.2.1.1.5.0", "sysName", this, kReadWrite);
  AddScalar (pScalar);

  pScalar = new StringMibObject ("1.3.6.1.2.1.1.6.0", "sysLocation", this, kReadWrite);
  AddScalar (pScalar);

  pScalar = new Int32MibObject ("1.3.6.1.2.1.1.7.0", "sysServices", this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new TimeticksMibObject ("1.3.6.1.2.1.1.8.0", "sysORLastChange", this, kReadOnly);
  AddScalar (pScalar);

  pTable = new sysORTable();
  AddTable (pTable);

  return true;
}

SNMP_STATUS systemGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (systemGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS systemGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;
  BcmObjectId Oid;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_sysDescr:
      fpBridge->Get_sysDescr (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_sysObjectID:
      fpBridge->Get_sysObjectID (Oid);
      Status = VB.SetOid (Oid);
      break;

    case kLEAF_sysUpTime:
      Status = VB.SetTimeticks (fpBridge->Get_sysUpTime ());
      break;

    case kLEAF_sysContact:
      fpBridge->Get_sysContact (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_sysName:
      fpBridge->Get_sysName (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_sysLocation:
      fpBridge->Get_sysLocation (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_sysServices:
      Status = VB.SetInt32 (fpBridge->Get_sysServices ());
      break;

    case kLEAF_sysORLastChange:
      Status = VB.SetTimeticks (fpBridge->Get_sysORLastChange ());
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

SNMP_STATUS systemGroup::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;
  BcmObjectId Oid;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_sysDescr:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_sysObjectID:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_sysUpTime:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_sysContact:
      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_sysContact (String);
      break;

    case kLEAF_sysName:
      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_sysName (String);
      break;

    case kLEAF_sysLocation:
      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_sysLocation (String);
      break;

    case kLEAF_sysServices:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_sysORLastChange:
      Status = SNMP_NOT_WRITABLE;
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}


/*\ 
 *  sysORTable and related objects 
\*/

/* sysORTable */

sysORTable::sysORTable ()
  : BcmMibTable ("1.3.6.1.2.1.1.9", "sysORTable")
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << sysOREntry::kLEAF_sysORID);
  fColumnMask |= (1 << sysOREntry::kLEAF_sysORDescr);
  fColumnMask |= (1 << sysOREntry::kLEAF_sysORUpTime);
}

SNMP_STATUS sysORTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  sysOREntry *pEntry;
  sysOREntryBridge *pMyBridge = (sysOREntryBridge *) pBridge;


  pEntry = new sysOREntry (this, pMyBridge);
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

/* sysOREntry */

sysOREntry::sysOREntry (sysORTable *pTable, sysOREntryBridge *pBridge)
  : BcmMibEntry (pTable, "sysOREntry")
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject ("1.3.6.1.2.1.1.9.1.1", "sysORIndex", this, kNotAccessible);
  AddObject (pObject);

  pObject = new OidMibObject ("1.3.6.1.2.1.1.9.1.2", "sysORID", this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject ("1.3.6.1.2.1.1.9.1.3", "sysORDescr", this, kReadOnly);
  AddObject (pObject);

  pObject = new TimeticksMibObject ("1.3.6.1.2.1.1.9.1.4", "sysORUpTime", this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS sysOREntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmObjectId Oid;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_sysORIndex: // not-accessible
      Status = VB.SetInt32 (fpBridge->Get_sysORIndex ());
      break;

    case kLEAF_sysORID:
      fpBridge->Get_sysORID (Oid);
      Status = VB.SetOid (Oid);
      break;

    case kLEAF_sysORDescr:
    {
      BcmString String;

      fpBridge->Get_sysORDescr (String);
      Status = VB.SetString (String);
      break;
    }

    case kLEAF_sysORUpTime:
      Status = VB.SetTimeticks (fpBridge->Get_sysORUpTime ());
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


