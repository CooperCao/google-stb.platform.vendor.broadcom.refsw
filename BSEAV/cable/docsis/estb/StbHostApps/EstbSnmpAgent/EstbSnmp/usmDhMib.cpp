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
//    Filename: usmDhNewMib.cpp
//    Creation Date: Wednesday, February 04, 2004 at 15:34:16
//    Created by Broadcom BFC / V2 Mib Compiler BCMIBC.EXE version 3.0.0b
//    from input file rfc2786.mib
//
//**************************************************************************

#include "usmDhMib.h"
#include "usmDhBridge.h"
#include "SnmpLog.h" // for SnmpLog... macros
#include "BcmSnmpAgent.h"


/*\
 *  snmpUsmDHObjectsMIBGroup group
\*/

snmpUsmDHObjectsMIBGroup::snmpUsmDHObjectsMIBGroup ()
  : BcmMibGroup ("1.3.6.1.3.101", "snmpUsmDHObjectsMIBGroup")
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool snmpUsmDHObjectsMIBGroup::Populate ()
{
  BcmMibGroup *pGroup;


  if (BcmMibGroup::Populate() == false)
    return false;

  pGroup = new usmDHKeyObjectsGroup();
  AddGroup (pGroup);

  return true;
}

/*\
 *  usmDHKeyObjectsGroup group
\*/

usmDHKeyObjectsGroup::usmDHKeyObjectsGroup ()
  : BcmMibGroup ("1.3.6.1.3.101.1", "usmDHKeyObjectsGroup")
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool usmDHKeyObjectsGroup::Populate ()
{
  BcmMibGroup *pGroup;


  if (BcmMibGroup::Populate() == false)
    return false;

  pGroup = new usmDHPublicObjectsGroup();
  AddGroup (pGroup);

  pGroup = new usmDHKickstartGroup();
  AddGroup (pGroup);

  return true;
}

/*\
 *  usmDHPublicObjectsGroup group
\*/

usmDHPublicObjectsGroup::usmDHPublicObjectsGroup ()
  : BcmMibGroup ("1.3.6.1.3.101.1.1", "usmDHPublicObjectsGroup")
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool usmDHPublicObjectsGroup::Populate ()
{
  BcmMibObject *pScalar;
  BcmMibTable *pTable;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new StringMibObject ("1.3.6.1.3.101.1.1.1.0", "usmDHParameters", this, kReadWrite);
  AddScalar (pScalar);

  pTable = new usmDHUserKeyTable();
  AddTable (pTable);

  return true;
}

SNMP_STATUS usmDHPublicObjectsGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (usmDHPublicObjectsGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS usmDHPublicObjectsGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_usmDHParameters:
    {
      BcmString String;

      fpBridge->Get_usmDHParameters (String);
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

SNMP_STATUS usmDHPublicObjectsGroup::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_usmDHParameters:
    {
      BcmString String;

      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_usmDHParameters (String);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}


/*\ 
 *  usmDHUserKeyTable and related objects 
\*/

/* usmDHUserKeyTable */

usmDHUserKeyTable::usmDHUserKeyTable ()
  : BcmMibTable ("1.3.6.1.3.101.1.1.2", "usmDHUserKeyTable")
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << usmDHUserKeyEntry::kLEAF_usmDHUserAuthKeyChange);
  fColumnMask |= (1 << usmDHUserKeyEntry::kLEAF_usmDHUserOwnAuthKeyChange);
  fColumnMask |= (1 << usmDHUserKeyEntry::kLEAF_usmDHUserPrivKeyChange);
  fColumnMask |= (1 << usmDHUserKeyEntry::kLEAF_usmDHUserOwnPrivKeyChange);
}

SNMP_STATUS usmDHUserKeyTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  usmDHUserKeyEntry *pEntry;
  usmDHUserKeyEntryBridge *pMyBridge = (usmDHUserKeyEntryBridge *) pBridge;


  pEntry = new usmDHUserKeyEntry (this, pMyBridge);
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

/* usmDHUserKeyEntry */

usmDHUserKeyEntry::usmDHUserKeyEntry (usmDHUserKeyTable *pTable, usmDHUserKeyEntryBridge *pBridge)
  : BcmMibEntry (pTable, "usmDHUserKeyEntry")
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new StringMibObject ("1.3.6.1.3.101.1.1.2.1.1", "usmDHUserAuthKeyChange", this, kReadCreate);
  AddObject (pObject);

  pObject = new StringMibObject ("1.3.6.1.3.101.1.1.2.1.2", "usmDHUserOwnAuthKeyChange", this, kReadCreate);
  AddObject (pObject);

  pObject = new StringMibObject ("1.3.6.1.3.101.1.1.2.1.3", "usmDHUserPrivKeyChange", this, kReadCreate);
  AddObject (pObject);

  pObject = new StringMibObject ("1.3.6.1.3.101.1.1.2.1.4", "usmDHUserOwnPrivKeyChange", this, kReadCreate);
  AddObject (pObject);
}

SNMP_STATUS usmDHUserKeyEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_usmDHUserAuthKeyChange:
      fpBridge->Get_usmDHUserAuthKeyChange (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_usmDHUserOwnAuthKeyChange:
      fpBridge->Get_usmDHUserOwnAuthKeyChange (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_usmDHUserPrivKeyChange:
      fpBridge->Get_usmDHUserPrivKeyChange (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_usmDHUserOwnPrivKeyChange:
      fpBridge->Get_usmDHUserOwnPrivKeyChange (String);
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

SNMP_STATUS usmDHUserKeyEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_usmDHUserAuthKeyChange:
      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_usmDHUserAuthKeyChange (String);
      break;

    case kLEAF_usmDHUserOwnAuthKeyChange:
      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_usmDHUserOwnAuthKeyChange (String);
      break;

    case kLEAF_usmDHUserPrivKeyChange:
      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_usmDHUserPrivKeyChange (String);
      break;

    case kLEAF_usmDHUserOwnPrivKeyChange:
      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_usmDHUserOwnPrivKeyChange (String);
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}

/*\
 *  usmDHKickstartGroup group
\*/

usmDHKickstartGroup::usmDHKickstartGroup ()
  : BcmMibGroup ("1.3.6.1.3.101.1.2", "usmDHKickstartGroup")
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool usmDHKickstartGroup::Populate ()
{
  BcmMibTable *pTable;


  if (BcmMibGroup::Populate() == false)
    return false;

  pTable = new usmDHKickstartTable();
  AddTable (pTable);

  return true;
}


/*\ 
 *  usmDHKickstartTable and related objects 
\*/

/* usmDHKickstartTable */

usmDHKickstartTable::usmDHKickstartTable ()
  : BcmMibTable ("1.3.6.1.3.101.1.2.1", "usmDHKickstartTable")
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << usmDHKickstartEntry::kLEAF_usmDHKickstartMyPublic);
  fColumnMask |= (1 << usmDHKickstartEntry::kLEAF_usmDHKickstartMgrPublic);
  fColumnMask |= (1 << usmDHKickstartEntry::kLEAF_usmDHKickstartSecurityName);
}

SNMP_STATUS usmDHKickstartTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  usmDHKickstartEntry *pEntry;
  usmDHKickstartEntryBridge *pMyBridge = (usmDHKickstartEntryBridge *) pBridge;


  pEntry = new usmDHKickstartEntry (this, pMyBridge);
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

/* usmDHKickstartEntry */

usmDHKickstartEntry::usmDHKickstartEntry (usmDHKickstartTable *pTable, usmDHKickstartEntryBridge *pBridge)
  : BcmMibEntry (pTable, "usmDHKickstartEntry")
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject ("1.3.6.1.3.101.1.2.1.1.1", "usmDHKickstartIndex", this, kNotAccessible);
  AddObject (pObject);

  pObject = new StringMibObject ("1.3.6.1.3.101.1.2.1.1.2", "usmDHKickstartMyPublic", this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject ("1.3.6.1.3.101.1.2.1.1.3", "usmDHKickstartMgrPublic", this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject ("1.3.6.1.3.101.1.2.1.1.4", "usmDHKickstartSecurityName", this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS usmDHKickstartEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_usmDHKickstartIndex: // not-accessible
      Status = VB.SetInt32 (fpBridge->Get_usmDHKickstartIndex ());
      break;

    case kLEAF_usmDHKickstartMyPublic:
      fpBridge->Get_usmDHKickstartMyPublic (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_usmDHKickstartMgrPublic:
      fpBridge->Get_usmDHKickstartMgrPublic (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_usmDHKickstartSecurityName:
      fpBridge->Get_usmDHKickstartSecurityName (String);
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

