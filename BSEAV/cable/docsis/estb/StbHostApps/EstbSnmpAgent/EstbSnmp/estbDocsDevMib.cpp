//**************************************************************************
//                                                                          
//    Copyright 2006 Broadcom Corporation                                   
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
//    Filename: estbDocsDevMib.cpp
//    Creation Date: 15 DEC 2006
//    Created by Broadcom BFC / V2 Mib Compiler BCMIBC.EXE version 3.0.0b
//    from input file ../docsis/rfc2669.mib
//
//**************************************************************************

#include "estbDocsDevBridge.h"
#include "SnmpLog.h" // for SnmpLog... macros
#include "BcmSnmpAgent.h"


#include "ifBridge.h"
#include "OperatingSystemFactory.h"
#include "PktFilterController.h"

#include "docsDevNmMib.h"
#include "docsDevEvMib.h"
#include "HalIf.h"


/*\
 *  docsDevGroup group
\*/

docsDevGroup::docsDevGroup ()
  : docsDevMibGroup ("1.3.6.1.2.1.69", "docsDevGroup")
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool docsDevGroup::Populate ()
{
  docsDevMibGroup *pGroup;


  if (docsDevMibGroup::Populate() == false)
    return false;

  pGroup = new docsDevMIBObjectsGroup();
  AddGroup (pGroup);

  return true;
}

/*\
 *  docsDevMIBObjectsGroup group
\*/

docsDevMIBObjectsGroup::docsDevMIBObjectsGroup ()
  : docsDevMibGroup ("1.3.6.1.2.1.69.1", "docsDevMIBObjectsGroup")
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool docsDevMIBObjectsGroup::Populate ()
{
  docsDevMibTable *pTable;
  docsDevMibGroup *pGroup;


  if (docsDevMibGroup::Populate() == false)
    return false;

  pTable = new docsDevNmAccessTable();
  AddTable (pTable);

  pGroup = new docsDevSoftwareGroup();
  AddGroup (pGroup);

  pGroup = new docsDevEventGroup();
  AddGroup (pGroup);

  pGroup = new docsDevFilterGroup();
  AddGroup (pGroup);

  return true;
}

/*\
 *  docsDevSoftwareGroup group
\*/

docsDevSoftwareGroup::docsDevSoftwareGroup ()
  : docsDevMibGroup ("1.3.6.1.2.1.69.1.3", "docsDevSoftwareGroup")
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool docsDevSoftwareGroup::Populate ()
{
  BcmMibObject *pScalar;


  if (docsDevMibGroup::Populate() == false)
    return false;

  pScalar = new IpAddrMibObject ("1.3.6.1.2.1.69.1.3.1.0", "docsDevSwServer", this, kReadWrite);
  AddScalar (pScalar);

  pScalar = new StringMibObject ("1.3.6.1.2.1.69.1.3.2.0", "docsDevSwFilename", this, kReadWrite);
  AddScalar (pScalar);

  pScalar = new Int32MibObject ("1.3.6.1.2.1.69.1.3.3.0", "docsDevSwAdminStatus", this, kReadWrite);
  AddScalar (pScalar);

  pScalar = new Int32MibObject ("1.3.6.1.2.1.69.1.3.4.0", "docsDevSwOperStatus", this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new StringMibObject ("1.3.6.1.2.1.69.1.3.5.0", "docsDevSwCurrentVers", this, kReadOnly);
  AddScalar (pScalar);

  return true;
}

SNMP_STATUS docsDevSoftwareGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = docsDevMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (docsDevSoftwareGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS docsDevSoftwareGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;
  BcmIpAddress Ip;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_docsDevSwServer:
      fpBridge->Get_docsDevSwServer (Ip);
      Status = VB.SetIp (Ip);
      break;

    case kLEAF_docsDevSwFilename:
      fpBridge->Get_docsDevSwFilename (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_docsDevSwAdminStatus:
      Status = VB.SetInt32 (fpBridge->Get_docsDevSwAdminStatus ());
      break;

    case kLEAF_docsDevSwOperStatus:
      Status = VB.SetInt32 (fpBridge->Get_docsDevSwOperStatus ());
      break;

    case kLEAF_docsDevSwCurrentVers:
      fpBridge->Get_docsDevSwCurrentVers (String);
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

SNMP_STATUS docsDevSoftwareGroup::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;
  BcmIpAddress Ip;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_docsDevSwServer:
      Status = VB.GetIp (Ip);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevSwServer (Ip);
      break;

    case kLEAF_docsDevSwFilename:
      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevSwFilename (String);
      break;

    case kLEAF_docsDevSwAdminStatus:
    {
      int docsDevSwAdminStatus;

      Status = VB.GetInt32 (&docsDevSwAdminStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevSwAdminStatus (docsDevSwAdminStatus);
      break;
    }

    case kLEAF_docsDevSwOperStatus:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_docsDevSwCurrentVers:
      Status = SNMP_NOT_WRITABLE;
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}

/*\
 *  docsDevFilterGroup group
\*/

docsDevFilterGroup::docsDevFilterGroup ()
  : docsDevMibGroup ("1.3.6.1.2.1.69.1.6", "docsDevFilterGroup")
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool docsDevFilterGroup::Populate ()
{
  BcmMibObject *pScalar;
  docsDevMibTable *pTable;


  if (docsDevMibGroup::Populate() == false)
    return false;

  pScalar = new Int32MibObject ("1.3.6.1.2.1.69.1.6.3.0", "docsDevFilterIpDefault", this, kReadWrite);
  AddScalar (pScalar);

  pTable = new docsDevFilterIpTable();
  AddTable (pTable);

  return true;
}

SNMP_STATUS docsDevFilterGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = docsDevMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (docsDevFilterGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS docsDevFilterGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_docsDevFilterIpDefault:
      Status = VB.SetInt32 (fpBridge->Get_docsDevFilterIpDefault ());
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

SNMP_STATUS docsDevFilterGroup::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_docsDevFilterIpDefault:
    {
      int docsDevFilterIpDefault;

      Status = VB.GetInt32 (&docsDevFilterIpDefault);
      if (Status == SNMP_NO_ERROR)
      {
        Status = fpBridge->Set_docsDevFilterIpDefault (docsDevFilterIpDefault);
        BcmPacketFilterController::Singleton().UpdateIpFilterOptimizations ();
      }
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}



/*\
 *  docsDevFilterIpTable and related objects
\*/

/* docsDevFilterIpTable */

docsDevFilterIpTable::docsDevFilterIpTable ()
  : docsDevMibTable ("1.3.6.1.2.1.69.1.6.4", "docsDevFilterIpTable")
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << docsDevFilterIpEntry::kLEAF_docsDevFilterIpStatus);
  fColumnMask |= (1 << docsDevFilterIpEntry::kLEAF_docsDevFilterIpControl);
  fColumnMask |= (1 << docsDevFilterIpEntry::kLEAF_docsDevFilterIpIfIndex);
  fColumnMask |= (1 << docsDevFilterIpEntry::kLEAF_docsDevFilterIpDirection);
  fColumnMask |= (1 << docsDevFilterIpEntry::kLEAF_docsDevFilterIpBroadcast);
  fColumnMask |= (1 << docsDevFilterIpEntry::kLEAF_docsDevFilterIpSaddr);
  fColumnMask |= (1 << docsDevFilterIpEntry::kLEAF_docsDevFilterIpSmask);
  fColumnMask |= (1 << docsDevFilterIpEntry::kLEAF_docsDevFilterIpDaddr);
  fColumnMask |= (1 << docsDevFilterIpEntry::kLEAF_docsDevFilterIpDmask);
  fColumnMask |= (1 << docsDevFilterIpEntry::kLEAF_docsDevFilterIpProtocol);
  fColumnMask |= (1 << docsDevFilterIpEntry::kLEAF_docsDevFilterIpSourcePortLow);
  fColumnMask |= (1 << docsDevFilterIpEntry::kLEAF_docsDevFilterIpSourcePortHigh);
  fColumnMask |= (1 << docsDevFilterIpEntry::kLEAF_docsDevFilterIpDestPortLow);
  fColumnMask |= (1 << docsDevFilterIpEntry::kLEAF_docsDevFilterIpDestPortHigh);
  fColumnMask |= (1 << docsDevFilterIpEntry::kLEAF_docsDevFilterIpMatches);
  fColumnMask |= (1 << docsDevFilterIpEntry::kLEAF_docsDevFilterIpTos);
  fColumnMask |= (1 << docsDevFilterIpEntry::kLEAF_docsDevFilterIpTosMask);
  fColumnMask |= (1 << docsDevFilterIpEntry::kLEAF_docsDevFilterIpContinue);
  fColumnMask |= (1 << docsDevFilterIpEntry::kLEAF_docsDevFilterIpPolicyId);
}

SNMP_STATUS docsDevFilterIpTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  docsDevFilterIpEntry *pEntry;
  docsDevFilterIpEntryBridge *pMyBridge = (docsDevFilterIpEntryBridge *) pBridge;


  pEntry = new docsDevFilterIpEntry (this, pMyBridge);
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

BcmMibEntry *docsDevFilterIpTable::NewDefaultEntry (BcmObjectId &Index)
{
  if (fpAgent == NULL)
    return NULL;

  /* Verify index length. */

  // docsDevFilterIpIndex (numerical type): 1 element
  if (Index.Length() != 1)
    return NULL;

  /* Verify index value(s) */

  // docsDevFilterIpIndex (numerical type)
  if ((Index[0] < MINVAL_DOCSDEVFILTERIPINDEX)
  ||  (Index[0] > MAXVAL_DOCSDEVFILTERIPINDEX))
    return NULL;

  docsDevFilterIpEntryBridge *pBridge = new docsDevFilterIpEntryBridge (Index[0], fpAgent);

  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
  {
    SnmpLogInit << "docsDevFilterIpTable::NewDefaultEntry Bridge registration failed!  Deleting bridge object." << endl;
    delete (pBridge);
    return NULL;
  }

  return (docsDevMibEntry *)pBridge->fpObject;
}

SNMP_STATUS docsDevFilterIpTable::DestroyEntry (BcmObjectId &Index)
{
  docsDevFilterIpEntry *pEntry = NULL;
  MibEntryIterator Pos;


  SnmpLogInit << "docsDevFilterIpTable::DestroyEntry: destroying entry of index " << Index << endl;

  /* Actually destroy this entry */

  if (fpAgent)
    fpAgent->LockMib();

  // Find and remove entry from entry container
  pEntry = (docsDevFilterIpEntry *) FindEntry (Index, Pos);

  if (pEntry == NULL)
  {
    SnmpLogInit << "docsDevFilterIpTable::DestroyEntry failed to find Index " << Index << endl;

    if (fpAgent)
      fpAgent->UnlockMib();

    return SNMP_NO_SUCH_NAME;
  }

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
  fEntries.erase (Index);
#else
  fEntries.erase (Pos);
#endif

  // This is our protection against the possibility of a race
  // condition during packet filtering.  We aren't protected by the
  // mutex lock in this case because we don't want to take the
  // throughput hit on locking the mutex in the CmSnmpAgent::ApplyPacketFilters
  // method, which sits directly in the data path.  So, when destroying
  // any kind of filter entry, we'll simply remove the entry from the container,
  // then wait 50 ms before actually deleting it.  That way, if the filter
  // of interest is currently being considered by ApplyPacketFilters, it will
  // have time to finish with the filter before it's deleted.  Meanwhile, the
  // filter won't get returned in other iterations because it's been removed
  // from the table.  Kind of crude, but effective.
  BcmOperatingSystemFactory::ThreadSleep(50);

  // Set entry's fpTable pointer to NULL to avoid calling this again
  // in the entry's destructor.  Actually, re-calling wouldn't hurt us,
  // but it does take some time so this will speed things up a bit.
  pEntry->fpTable = NULL;

  delete (pEntry);

  if (fpAgent)
    fpAgent->UnlockMib();

  return SNMP_NO_ERROR;
}

/* docsDevFilterIpEntry */

docsDevFilterIpEntry::docsDevFilterIpEntry (docsDevFilterIpTable *pTable, docsDevFilterIpEntryBridge *pBridge)
  : docsDevMibEntry (pTable, "docsDevFilterIpEntry")
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject ("1.3.6.1.2.1.69.1.6.4.1.1", "docsDevFilterIpIndex", this, kNotAccessible);
  AddObject (pObject);

  pObject = new RowStatusMibObject (pTable, this, "1.3.6.1.2.1.69.1.6.4.1.2", "docsDevFilterIpStatus", kReadCreate);
  AddObject (pObject);

  pObject = new Int32MibObject ("1.3.6.1.2.1.69.1.6.4.1.3", "docsDevFilterIpControl", this, kReadCreate);
  AddObject (pObject);

  pObject = new Int32MibObject ("1.3.6.1.2.1.69.1.6.4.1.4", "docsDevFilterIpIfIndex", this, kReadCreate);
  AddObject (pObject);

  pObject = new Int32MibObject ("1.3.6.1.2.1.69.1.6.4.1.5", "docsDevFilterIpDirection", this, kReadCreate);
  AddObject (pObject);

  pObject = new Int32MibObject ("1.3.6.1.2.1.69.1.6.4.1.6", "docsDevFilterIpBroadcast", this, kReadCreate);
  AddObject (pObject);

  pObject = new IpAddrMibObject ("1.3.6.1.2.1.69.1.6.4.1.7", "docsDevFilterIpSaddr", this, kReadCreate);
  AddObject (pObject);

  pObject = new IpAddrMibObject ("1.3.6.1.2.1.69.1.6.4.1.8", "docsDevFilterIpSmask", this, kReadCreate);
  AddObject (pObject);

  pObject = new IpAddrMibObject ("1.3.6.1.2.1.69.1.6.4.1.9", "docsDevFilterIpDaddr", this, kReadCreate);
  AddObject (pObject);

  pObject = new IpAddrMibObject ("1.3.6.1.2.1.69.1.6.4.1.10", "docsDevFilterIpDmask", this, kReadCreate);
  AddObject (pObject);

  pObject = new Int32MibObject ("1.3.6.1.2.1.69.1.6.4.1.11", "docsDevFilterIpProtocol", this, kReadCreate);
  AddObject (pObject);

  pObject = new Int32MibObject ("1.3.6.1.2.1.69.1.6.4.1.12", "docsDevFilterIpSourcePortLow", this, kReadCreate);
  AddObject (pObject);

  pObject = new Int32MibObject ("1.3.6.1.2.1.69.1.6.4.1.13", "docsDevFilterIpSourcePortHigh", this, kReadCreate);
  AddObject (pObject);

  pObject = new Int32MibObject ("1.3.6.1.2.1.69.1.6.4.1.14", "docsDevFilterIpDestPortLow", this, kReadCreate);
  AddObject (pObject);

  pObject = new Int32MibObject ("1.3.6.1.2.1.69.1.6.4.1.15", "docsDevFilterIpDestPortHigh", this, kReadCreate);
  AddObject (pObject);

  pObject = new Uint32MibObject ("1.3.6.1.2.1.69.1.6.4.1.16", "docsDevFilterIpMatches", this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject ("1.3.6.1.2.1.69.1.6.4.1.17", "docsDevFilterIpTos", this, kReadCreate);
  AddObject (pObject);

  pObject = new StringMibObject ("1.3.6.1.2.1.69.1.6.4.1.18", "docsDevFilterIpTosMask", this, kReadCreate);
  AddObject (pObject);

  pObject = new Int32MibObject ("1.3.6.1.2.1.69.1.6.4.1.19", "docsDevFilterIpContinue", this, kReadCreate);
  AddObject (pObject);

  pObject = new Int32MibObject ("1.3.6.1.2.1.69.1.6.4.1.20", "docsDevFilterIpPolicyId", this, kReadCreate);
  AddObject (pObject);

  fpFilter = fpBridge->fpFilter;
}

SNMP_STATUS docsDevFilterIpEntry::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;

  Status = docsDevMibEntry::RegisterBridge(pBridge, DeleteUponDestruction);

  // Find the USB interface so we can conditionally block access to filters
  // in this table from USB queries.
  if (Status == SNMP_NO_ERROR)
    fpUsbIf = fpAgent->IfBridge().FirstIfEntryOfType (BcmHalIf::kIanaTypeUsb);

  return Status;
}

bool docsDevFilterIpEntry::IsAccessible (BcmMibRequest *pReq) const
{
  // First, check for plain old docsDevMibEntry accessibility
  if (docsDevMibEntry::IsAccessible (pReq) == false)
    return false;

#ifdef SUPPORT_USB_FILTER_HIDING
  if ((fpUsbIf == NULL) || (pReq == NULL) || (fpStatusObject == NULL))
    return true;

  if (pReq->fSrcIfIndex == fpUsbIf->Get_ifIndex())
  {
    // Support the special USB-only requirement:
    // "The host CPE MUST NOT be able to either access or alter MSO configured filters."
    if ((fpStatusObject->CreationMethod() == STATIC_CM_IFNUM)
    ||  (fpStatusObject->CreationMethod() == STATIC_CM_DS_IFNUM)
    ||  (fpStatusObject->CreationMethod() == STATIC_CM_US_IFNUM)
    ||  (fpStatusObject->CreationMethod() == RowStatusMibObject::kFromProvisioning))
    {
      SnmpLogReq << "docsDevFilterIpEntry::IsAccessible returning false due to USB query." << endl;
      return false;
    }
  }
#endif

  return true;
}

SNMP_STATUS docsDevFilterIpEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmIpAddress Ip;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_docsDevFilterIpIndex: // not-accessible
      Status = VB.SetInt32 (fpBridge->Get_docsDevFilterIpIndex ());
      break;

    case kLEAF_docsDevFilterIpStatus:
      Status = VB.SetInt32 (fpBridge->Get_docsDevFilterIpStatus ());
      break;

    case kLEAF_docsDevFilterIpControl:
      Status = VB.SetInt32 (fpBridge->Get_docsDevFilterIpControl ());
      break;

    case kLEAF_docsDevFilterIpIfIndex:
      Status = VB.SetInt32 (fpBridge->Get_docsDevFilterIpIfIndex ());
      break;

    case kLEAF_docsDevFilterIpDirection:
      Status = VB.SetInt32 (fpBridge->Get_docsDevFilterIpDirection ());
      break;

    case kLEAF_docsDevFilterIpBroadcast:
      Status = VB.SetBool (fpBridge->Get_docsDevFilterIpBroadcast ());
      break;

    case kLEAF_docsDevFilterIpSaddr:
      fpBridge->Get_docsDevFilterIpSaddr (Ip);
      Status = VB.SetIp (Ip);
      break;

    case kLEAF_docsDevFilterIpSmask:
      fpBridge->Get_docsDevFilterIpSmask (Ip);
      Status = VB.SetIp (Ip);
      break;

    case kLEAF_docsDevFilterIpDaddr:
      fpBridge->Get_docsDevFilterIpDaddr (Ip);
      Status = VB.SetIp (Ip);
      break;

    case kLEAF_docsDevFilterIpDmask:
      fpBridge->Get_docsDevFilterIpDmask (Ip);
      Status = VB.SetIp (Ip);
      break;

    case kLEAF_docsDevFilterIpProtocol:
      Status = VB.SetInt32 (fpBridge->Get_docsDevFilterIpProtocol ());
      break;

    case kLEAF_docsDevFilterIpSourcePortLow:
      Status = VB.SetInt32 (fpBridge->Get_docsDevFilterIpSourcePortLow ());
      break;

    case kLEAF_docsDevFilterIpSourcePortHigh:
      Status = VB.SetInt32 (fpBridge->Get_docsDevFilterIpSourcePortHigh ());
      break;

    case kLEAF_docsDevFilterIpDestPortLow:
      Status = VB.SetInt32 (fpBridge->Get_docsDevFilterIpDestPortLow ());
      break;

    case kLEAF_docsDevFilterIpDestPortHigh:
      Status = VB.SetInt32 (fpBridge->Get_docsDevFilterIpDestPortHigh ());
      break;

    case kLEAF_docsDevFilterIpMatches:
      Status = VB.SetUint32 (fpBridge->Get_docsDevFilterIpMatches (), VBT_COUNTER);
      break;

    case kLEAF_docsDevFilterIpTos:
      fpBridge->Get_docsDevFilterIpTos (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_docsDevFilterIpTosMask:
      fpBridge->Get_docsDevFilterIpTosMask (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_docsDevFilterIpContinue:
      Status = VB.SetBool (fpBridge->Get_docsDevFilterIpContinue ());
      break;

    case kLEAF_docsDevFilterIpPolicyId:
      Status = VB.SetInt32 (fpBridge->Get_docsDevFilterIpPolicyId ());
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

SNMP_STATUS docsDevFilterIpEntry::TestLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmIpAddress Ip;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_docsDevFilterIpIndex:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_docsDevFilterIpStatus:
    {
      int docsDevFilterIpStatus;

      Status = VB.GetInt32 (&docsDevFilterIpStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_docsDevFilterIpStatus (docsDevFilterIpStatus);
      break;
    }

    case kLEAF_docsDevFilterIpMatches:
      Status = SNMP_NOT_WRITABLE;
      break;

  }

  return Status;
}

SNMP_STATUS docsDevFilterIpEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmIpAddress Ip;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  // If this row has not yet been created, either reject the set or create the
  // row; either is allowed per RFC-2579.  See note in MibObjs.h for details.
  if ((fpBridge->Get_docsDevFilterIpStatus() == SNMP_STATUS_INVISIBLE)
  &&  ((fpBridge->fObjectMask & docsDevFilterIpEntryBridge::kBIT_docsDevFilterIpStatus) == 0))
  {
#ifdef SUPPORT_INDIRECT_ROW_CREATION
    int CreateStatus = SNMP_STATUS_CREATE_AND_WAIT;

    SnmpLogReq << "Indirect row creation supported, creating docsDevFilterIpEntry" << endl;
    Status = fpBridge->Set_docsDevFilterIpStatus (CreateStatus);
    if (Status != SNMP_NO_ERROR)
      return Status;
#else
    SnmpLogReq << "docsDevFilterIpStatus not set (" << (void *)fpBridge->fObjectMask << "), INCONSISTENT_VALUE" << endl;
    return SNMP_INCONSISTENT_VALUE | SNMP_ROW_NOT_CREATED;
#endif
  }

  switch (Leaf.Leaf())
  {
    case kLEAF_docsDevFilterIpIndex:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_docsDevFilterIpStatus:
    {
      int docsDevFilterIpStatus;

      Status = VB.GetInt32 (&docsDevFilterIpStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevFilterIpStatus (docsDevFilterIpStatus);
      break;
    }

    case kLEAF_docsDevFilterIpControl:
    {
      int docsDevFilterIpControl;

      Status = VB.GetInt32 (&docsDevFilterIpControl);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevFilterIpControl (docsDevFilterIpControl);
      break;
    }

    case kLEAF_docsDevFilterIpIfIndex:
    {
      int docsDevFilterIpIfIndex;

      Status = VB.GetInt32 (&docsDevFilterIpIfIndex);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevFilterIpIfIndex (docsDevFilterIpIfIndex);
      break;
    }

    case kLEAF_docsDevFilterIpDirection:
    {
      int docsDevFilterIpDirection;

      Status = VB.GetInt32 (&docsDevFilterIpDirection);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevFilterIpDirection (docsDevFilterIpDirection);
      break;
    }

    case kLEAF_docsDevFilterIpBroadcast:
    {
      bool docsDevFilterIpBroadcast;

      Status = VB.GetBool (&docsDevFilterIpBroadcast);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevFilterIpBroadcast (docsDevFilterIpBroadcast);
      break;
    }

    case kLEAF_docsDevFilterIpSaddr:
      Status = VB.GetIp (Ip);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevFilterIpSaddr (Ip);
      break;

    case kLEAF_docsDevFilterIpSmask:
      Status = VB.GetIp (Ip);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevFilterIpSmask (Ip);
      break;

    case kLEAF_docsDevFilterIpDaddr:
      Status = VB.GetIp (Ip);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevFilterIpDaddr (Ip);
      break;

    case kLEAF_docsDevFilterIpDmask:
      Status = VB.GetIp (Ip);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevFilterIpDmask (Ip);
      break;

    case kLEAF_docsDevFilterIpProtocol:
    {
      int docsDevFilterIpProtocol;

      Status = VB.GetInt32 (&docsDevFilterIpProtocol);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevFilterIpProtocol (docsDevFilterIpProtocol);
      break;
    }

    case kLEAF_docsDevFilterIpSourcePortLow:
    {
      int docsDevFilterIpSourcePortLow;

      Status = VB.GetInt32 (&docsDevFilterIpSourcePortLow);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevFilterIpSourcePortLow (docsDevFilterIpSourcePortLow);
      break;
    }

    case kLEAF_docsDevFilterIpSourcePortHigh:
    {
      int docsDevFilterIpSourcePortHigh;

      Status = VB.GetInt32 (&docsDevFilterIpSourcePortHigh);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevFilterIpSourcePortHigh (docsDevFilterIpSourcePortHigh);
      break;
    }

    case kLEAF_docsDevFilterIpDestPortLow:
    {
      int docsDevFilterIpDestPortLow;

      Status = VB.GetInt32 (&docsDevFilterIpDestPortLow);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevFilterIpDestPortLow (docsDevFilterIpDestPortLow);
      break;
    }

    case kLEAF_docsDevFilterIpDestPortHigh:
    {
      int docsDevFilterIpDestPortHigh;

      Status = VB.GetInt32 (&docsDevFilterIpDestPortHigh);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevFilterIpDestPortHigh (docsDevFilterIpDestPortHigh);
      break;
    }

    case kLEAF_docsDevFilterIpMatches:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_docsDevFilterIpTos:
      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevFilterIpTos (String);
      break;

    case kLEAF_docsDevFilterIpTosMask:
      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevFilterIpTosMask (String);
      break;

    case kLEAF_docsDevFilterIpContinue:
    {
      bool docsDevFilterIpContinue;

      Status = VB.GetBool (&docsDevFilterIpContinue);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevFilterIpContinue (docsDevFilterIpContinue);
      break;
    }

    case kLEAF_docsDevFilterIpPolicyId:
    {
      int docsDevFilterIpPolicyId;

      Status = VB.GetInt32 (&docsDevFilterIpPolicyId);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevFilterIpPolicyId (docsDevFilterIpPolicyId);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  if (Status == SNMP_NO_ERROR)
    BcmPacketFilterController::Singleton().UpdateIpFilterOptimizations ();

  return Status;
}


