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
//    Filename: ifMib.cpp
//    Creation Date: Thursday, September 16, 2004 at 16:26:56
//    Created by Broadcom BFC / V2 Mib Compiler BCMIBC.EXE version 3.0.0j
//    from input file rfc2863.mib
//
//**************************************************************************

#include "ifMib.h"
#include "ifBridge.h"
#include "HalIf.h"
#include "SnmpLog.h" // for SnmpLog... macros
#include "BcmSnmpAgent.h"


/*\
 *  ifMIBGroup group
\*/

ifMIBGroup::ifMIBGroup ()
  : BcmMibGroup (kOID_ifMIBGroup, kNAME_ifMIBGroup)
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ifMIBGroup::Populate ()
{
  BcmMibGroup *pGroup;


  if (BcmMibGroup::Populate() == false)
    return false;

  pGroup = new ifMIBObjectsGroup();
  AddGroup (pGroup);

  return true;
}

/*\
 *  interfacesGroup group
\*/

interfacesGroup::interfacesGroup ()
  : BcmMibGroup (kOID_interfacesGroup, kNAME_interfacesGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool interfacesGroup::Populate ()
{
  BcmMibObject *pScalar;
  BcmMibTable *pTable;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new Int32MibObject (kOID_ifNumber, kNAME_ifNumber, this, kReadOnly);
  AddScalar (pScalar);

  pTable = new ifTable();
  AddTable (pTable);

  return true;
}

SNMP_STATUS interfacesGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (interfacesGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS interfacesGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ifNumber:
      Status = VB.SetInt32 (fpBridge->Get_ifNumber ());
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
 *  ifTable and related objects 
\*/

/* ifTable */

ifTable::ifTable ()
  : BcmMibTable (kOID_ifTable, kNAME_ifTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ifEntry::kLEAF_ifIndex);
  fColumnMask |= (1 << ifEntry::kLEAF_ifDescr);
  fColumnMask |= (1 << ifEntry::kLEAF_ifType);
  fColumnMask |= (1 << ifEntry::kLEAF_ifMtu);
  fColumnMask |= (1 << ifEntry::kLEAF_ifSpeed);
  fColumnMask |= (1 << ifEntry::kLEAF_ifPhysAddress);
  fColumnMask |= (1 << ifEntry::kLEAF_ifAdminStatus);
  fColumnMask |= (1 << ifEntry::kLEAF_ifOperStatus);
  fColumnMask |= (1 << ifEntry::kLEAF_ifLastChange);
  fColumnMask |= (1 << ifEntry::kLEAF_ifInOctets);
  fColumnMask |= (1 << ifEntry::kLEAF_ifInUcastPkts);
  fColumnMask |= (1 << ifEntry::kLEAF_ifInNUcastPkts);
  fColumnMask |= (1 << ifEntry::kLEAF_ifInDiscards);
  fColumnMask |= (1 << ifEntry::kLEAF_ifInErrors);
  fColumnMask |= (1 << ifEntry::kLEAF_ifInUnknownProtos);
  fColumnMask |= (1 << ifEntry::kLEAF_ifOutOctets);
  fColumnMask |= (1 << ifEntry::kLEAF_ifOutUcastPkts);
  fColumnMask |= (1 << ifEntry::kLEAF_ifOutNUcastPkts);
  fColumnMask |= (1 << ifEntry::kLEAF_ifOutDiscards);
  fColumnMask |= (1 << ifEntry::kLEAF_ifOutErrors);
  fColumnMask |= (1 << ifEntry::kLEAF_ifOutQLen);
  fColumnMask |= (1 << ifEntry::kLEAF_ifSpecific);
}

SNMP_STATUS ifTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ifEntry *pEntry;
  ifEntryBridge *pMyBridge = (ifEntryBridge *) pBridge;


  pEntry = new ifEntry (this, pMyBridge);
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

/* ifEntry */

ifEntry::ifEntry (ifTable *pTable, ifEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ifEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_ifIndex, kNAME_ifIndex, this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_ifDescr, kNAME_ifDescr, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ifType, kNAME_ifType, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ifMtu, kNAME_ifMtu, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ifSpeed, kNAME_ifSpeed, this, kReadOnly);
  AddObject (pObject);

  MacAddrMibObject *pMacObject = new MacAddrMibObject (kOID_ifPhysAddress, kNAME_ifPhysAddress, this, kReadOnly);
  pMacObject->SetSyntax (MacAddrMibObject::kPhysAddrSyntax); // return empty string if all 0's
  AddObject (pMacObject);

  pObject = new Int32MibObject (kOID_ifAdminStatus, kNAME_ifAdminStatus, this, kReadWrite);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ifOperStatus, kNAME_ifOperStatus, this, kReadOnly);
  AddObject (pObject);

  pObject = new TimeticksMibObject (kOID_ifLastChange, kNAME_ifLastChange, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ifInOctets, kNAME_ifInOctets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ifInUcastPkts, kNAME_ifInUcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ifInNUcastPkts, kNAME_ifInNUcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ifInDiscards, kNAME_ifInDiscards, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ifInErrors, kNAME_ifInErrors, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ifInUnknownProtos, kNAME_ifInUnknownProtos, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ifOutOctets, kNAME_ifOutOctets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ifOutUcastPkts, kNAME_ifOutUcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ifOutNUcastPkts, kNAME_ifOutNUcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ifOutDiscards, kNAME_ifOutDiscards, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ifOutErrors, kNAME_ifOutErrors, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ifOutQLen, kNAME_ifOutQLen, this, kReadOnly);
  AddObject (pObject);

  pObject = new OidMibObject (kOID_ifSpecific, kNAME_ifSpecific, this, kReadOnly);
  AddObject (pObject);
}

ostream &ifEntry::Print(ostream &outStream)
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  MibObjectIterator Pos;
  BcmMibObject *pObject;
  BcmObjectId Index;


  GetIndex (Index);

  outStream << "-------------------------------------------------------------------------------" << endl;

  if (fpAgent)
  {
    if (IsAccessible() == true)
      outStream << Name() << "." << Index <<  endl;
    else
      outStream << Name() << "." << Index << "  < INACCESSIBLE >" <<  endl;
  }
  else
    outStream << Name() << "." << Index << " agent=NULL" << endl;
    
  /* Add some special info here about our HAL interface and other things. */
  
  if (fpBridge)
  {
    outStream << "  driver index: " << fpBridge->Get_ifNumber() << "  " << endl;
    
    if (fpBridge->Get_ifTypeImpl() == BcmHalIf::kIanaTypeOther)
      outStream << "  ifType other(1), internal type: " << fpBridge->Get_ifType() << endl;
       
    outStream << "  counters: " << (void *)fpBridge->InterfaceCounters() << endl;
    
    if (fpBridge->InterfaceCounters())
    {
      outStream << "  HAL: " << (void *)fpBridge->InterfaceCounters()->Interface() << endl;
      if (fpBridge->InterfaceCounters()->Interface())
      {
        outStream << "    short name: " << (void *)fpBridge->InterfaceCounters()->Interface()->ShortName() << endl;
        outStream << "    description: '" << fpBridge->InterfaceCounters()->Interface()->Description() << "'" << endl;
      }
    }
    
    /* PR 8164: print ifStack above/below info */
    
    BcmString Description;

	#ifndef BCM_STANDALONE_SNMP_AGENT
    // Above (no more than one interface can be directly 'above' me)
    outStream << "  Above me: ";
    if (fpBridge->fpStackAbove)
    {
      fpBridge->fpStackAbove->Get_ifDescr(Description);
      outStream << fpBridge->fpStackAbove->Get_ifIndex() << " (" << Description << ")";
    }
    outStream << endl;
    
    // Below (zero or more interfaces may be 'below' me)
    outStream << "  Below me: ";
    list <ifEntryBridge *>::iterator SPos = fpBridge->fStackBelow.begin();
    
    if (SPos == fpBridge->fStackBelow.end())
      outStream << endl;
      
    while (SPos != fpBridge->fStackBelow.end())
    {
      (*SPos)->Get_ifDescr(Description);
      outStream << (*SPos)->Get_ifIndex() << " (" << Description << ")" << endl;
      SPos++;
      
      if (SPos != fpBridge->fStackBelow.end())
        outStream << "            ";
    }
	#endif
  }
  
  outStream << "-------------------------------------------------------------------------------" << endl;

  // Iterate through all objects and print 'em
  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
    pObject = (*Pos).second;
#else
    pObject = *Pos;
#endif

    outStream << *pObject;

    Pos++;
  }
#endif

  return outStream;
}

SNMP_STATUS ifEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmMacAddress Mac;
  BcmObjectId Oid;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ifIndex:
      Status = VB.SetInt32 (fpBridge->Get_ifIndex ());
      break;

    case kLEAF_ifDescr:
    {
      BcmString String;

      fpBridge->Get_ifDescr (String);
      Status = VB.SetString (String);
      break;
    }

    case kLEAF_ifType:
      // true argument here means translate the type to 'other' if the type
      // is unassigned.
      Status = VB.SetInt32 (fpBridge->Get_ifType (true));
      break;

    case kLEAF_ifMtu:
      Status = VB.SetInt32 (fpBridge->Get_ifMtu ());
      break;

    case kLEAF_ifSpeed:
      Status = VB.SetUint32 (fpBridge->Get_ifSpeed (), VBT_UNSIGNED32);
      break;

    case kLEAF_ifPhysAddress:
      fpBridge->Get_ifPhysAddress (Mac);
      Status = VB.SetMacAddr (Mac);
      break;

    case kLEAF_ifAdminStatus:
      Status = VB.SetInt32 (fpBridge->Get_ifAdminStatus ());
      break;

    case kLEAF_ifOperStatus:
      Status = VB.SetInt32 (fpBridge->Get_ifOperStatus ());
      break;

    case kLEAF_ifLastChange:
      Status = VB.SetTimeticks (fpBridge->Get_ifLastChange ());
      break;

    case kLEAF_ifInOctets:
      Status = VB.SetUint32 (fpBridge->Get_ifInOctets (), VBT_COUNTER);
      break;

    case kLEAF_ifInUcastPkts:
      Status = VB.SetUint32 (fpBridge->Get_ifInUcastPkts (), VBT_COUNTER);
      break;

    case kLEAF_ifInNUcastPkts:
      Status = VB.SetUint32 (fpBridge->Get_ifInNUcastPkts (), VBT_COUNTER);
      break;

    case kLEAF_ifInDiscards:
      Status = VB.SetUint32 (fpBridge->Get_ifInDiscards (), VBT_COUNTER);
      break;

    case kLEAF_ifInErrors:
      Status = VB.SetUint32 (fpBridge->Get_ifInErrors (), VBT_COUNTER);
      break;

    case kLEAF_ifInUnknownProtos:
      Status = VB.SetUint32 (fpBridge->Get_ifInUnknownProtos (), VBT_COUNTER);
      break;

    case kLEAF_ifOutOctets:
      Status = VB.SetUint32 (fpBridge->Get_ifOutOctets (), VBT_COUNTER);
      break;

    case kLEAF_ifOutUcastPkts:
      Status = VB.SetUint32 (fpBridge->Get_ifOutUcastPkts (), VBT_COUNTER);
      break;

    case kLEAF_ifOutNUcastPkts:
      Status = VB.SetUint32 (fpBridge->Get_ifOutNUcastPkts (), VBT_COUNTER);
      break;

    case kLEAF_ifOutDiscards:
      Status = VB.SetUint32 (fpBridge->Get_ifOutDiscards (), VBT_COUNTER);
      break;

    case kLEAF_ifOutErrors:
      Status = VB.SetUint32 (fpBridge->Get_ifOutErrors (), VBT_COUNTER);
      break;

    case kLEAF_ifOutQLen:
      Status = VB.SetUint32 (fpBridge->Get_ifOutQLen (), VBT_GAUGE);
      break;

    case kLEAF_ifSpecific:
      fpBridge->Get_ifSpecific (Oid);
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

SNMP_STATUS ifEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmMacAddress Mac;
  BcmObjectId Oid;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ifIndex:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifDescr:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifType:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifMtu:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifSpeed:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifPhysAddress:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifAdminStatus:
    {
      int ifAdminStatus;

      Status = VB.GetInt32 (&ifAdminStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ifAdminStatus (ifAdminStatus);
      break;
    }

    case kLEAF_ifOperStatus:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifLastChange:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifInOctets:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifInUcastPkts:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifInNUcastPkts:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifInDiscards:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifInErrors:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifInUnknownProtos:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifOutOctets:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifOutUcastPkts:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifOutNUcastPkts:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifOutDiscards:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifOutErrors:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifOutQLen:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifSpecific:
      Status = SNMP_NOT_WRITABLE;
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}


/*\
 *  ifMIBObjectsGroup group
\*/

ifMIBObjectsGroup::ifMIBObjectsGroup ()
  : BcmMibGroup (kOID_ifMIBObjectsGroup, kNAME_ifMIBObjectsGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ifMIBObjectsGroup::Populate ()
{
  BcmMibObject *pScalar;
  BcmMibTable *pTable;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new TimeticksMibObject (kOID_ifTableLastChange, kNAME_ifTableLastChange, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new TimeticksMibObject (kOID_ifStackLastChange, kNAME_ifStackLastChange, this, kReadOnly);
  AddScalar (pScalar);

  pTable = new ifXTable();
  AddTable (pTable);

  pTable = new ifStackTable();
  AddTable (pTable);
  
#if 0

  pTable = new ifRcvAddressTable();
  AddTable (pTable);

  pTable = new ifTestTable();
  AddTable (pTable);
  
#endif

  return true;
}

SNMP_STATUS ifMIBObjectsGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ifMIBObjectsGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ifMIBObjectsGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ifTableLastChange:
      Status = VB.SetTimeticks (fpBridge->Get_ifTableLastChange ());
      break;

    case kLEAF_ifStackLastChange:
      Status = VB.SetTimeticks (fpBridge->Get_ifStackLastChange ());
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
 *  ifXTable and related objects 
\*/

/* ifXTable */

ifXTable::ifXTable ()
  : BcmMibTable (kOID_ifXTable, kNAME_ifXTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ifXEntry::kLEAF_ifName);
  fColumnMask |= (1 << ifXEntry::kLEAF_ifInMulticastPkts);
  fColumnMask |= (1 << ifXEntry::kLEAF_ifInBroadcastPkts);
  fColumnMask |= (1 << ifXEntry::kLEAF_ifOutMulticastPkts);
  fColumnMask |= (1 << ifXEntry::kLEAF_ifOutBroadcastPkts);
  fColumnMask |= (1 << ifXEntry::kLEAF_ifHCInOctets);
  fColumnMask |= (1 << ifXEntry::kLEAF_ifHCInUcastPkts);
  fColumnMask |= (1 << ifXEntry::kLEAF_ifHCInMulticastPkts);
  fColumnMask |= (1 << ifXEntry::kLEAF_ifHCInBroadcastPkts);
  fColumnMask |= (1 << ifXEntry::kLEAF_ifHCOutOctets);
  fColumnMask |= (1 << ifXEntry::kLEAF_ifHCOutUcastPkts);
  fColumnMask |= (1 << ifXEntry::kLEAF_ifHCOutMulticastPkts);
  fColumnMask |= (1 << ifXEntry::kLEAF_ifHCOutBroadcastPkts);
  fColumnMask |= (1 << ifXEntry::kLEAF_ifLinkUpDownTrapEnable);
  fColumnMask |= (1 << ifXEntry::kLEAF_ifHighSpeed);
  fColumnMask |= (1 << ifXEntry::kLEAF_ifPromiscuousMode);
  fColumnMask |= (1 << ifXEntry::kLEAF_ifConnectorPresent);
  fColumnMask |= (1 << ifXEntry::kLEAF_ifAlias);
  fColumnMask |= (1 << ifXEntry::kLEAF_ifCounterDiscontinuityTime);
}

SNMP_STATUS ifXTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ifXEntry *pEntry;
  ifXEntryBridge *pMyBridge = (ifXEntryBridge *) pBridge;


  pEntry = new ifXEntry (this, pMyBridge);
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

/* ifXEntry */

ifXEntry::ifXEntry (ifXTable *pTable, ifXEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ifXEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new StringMibObject (kOID_ifName, kNAME_ifName, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ifInMulticastPkts, kNAME_ifInMulticastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ifInBroadcastPkts, kNAME_ifInBroadcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ifOutMulticastPkts, kNAME_ifOutMulticastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ifOutBroadcastPkts, kNAME_ifOutBroadcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ifHCInOctets, kNAME_ifHCInOctets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ifHCInUcastPkts, kNAME_ifHCInUcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ifHCInMulticastPkts, kNAME_ifHCInMulticastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ifHCInBroadcastPkts, kNAME_ifHCInBroadcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ifHCOutOctets, kNAME_ifHCOutOctets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ifHCOutUcastPkts, kNAME_ifHCOutUcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ifHCOutMulticastPkts, kNAME_ifHCOutMulticastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ifHCOutBroadcastPkts, kNAME_ifHCOutBroadcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ifLinkUpDownTrapEnable, kNAME_ifLinkUpDownTrapEnable, this, kReadWrite);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ifHighSpeed, kNAME_ifHighSpeed, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ifPromiscuousMode, kNAME_ifPromiscuousMode, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ifConnectorPresent, kNAME_ifConnectorPresent, this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_ifAlias, kNAME_ifAlias, this, kReadOnly);
  AddObject (pObject);

  pObject = new TimeticksMibObject (kOID_ifCounterDiscontinuityTime, kNAME_ifCounterDiscontinuityTime, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ifXEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmCounter64 C64;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ifName:
      fpBridge->Get_ifName (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_ifInMulticastPkts:
      Status = VB.SetUint32 (fpBridge->Get_ifInMulticastPkts (), VBT_COUNTER);
      break;

    case kLEAF_ifInBroadcastPkts:
      Status = VB.SetUint32 (fpBridge->Get_ifInBroadcastPkts (), VBT_COUNTER);
      break;

    case kLEAF_ifOutMulticastPkts:
      Status = VB.SetUint32 (fpBridge->Get_ifOutMulticastPkts (), VBT_COUNTER);
      break;

    case kLEAF_ifOutBroadcastPkts:
      Status = VB.SetUint32 (fpBridge->Get_ifOutBroadcastPkts (), VBT_COUNTER);
      break;

    case kLEAF_ifHCInOctets:
      fpBridge->Get_ifHCInOctets (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ifHCInUcastPkts:
      fpBridge->Get_ifHCInUcastPkts (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ifHCInMulticastPkts:
      fpBridge->Get_ifHCInMulticastPkts (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ifHCInBroadcastPkts:
      fpBridge->Get_ifHCInBroadcastPkts (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ifHCOutOctets:
      fpBridge->Get_ifHCOutOctets (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ifHCOutUcastPkts:
      fpBridge->Get_ifHCOutUcastPkts (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ifHCOutMulticastPkts:
      fpBridge->Get_ifHCOutMulticastPkts (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ifHCOutBroadcastPkts:
      fpBridge->Get_ifHCOutBroadcastPkts (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ifLinkUpDownTrapEnable:
      Status = VB.SetInt32 (fpBridge->Get_ifLinkUpDownTrapEnable ());
      break;

    case kLEAF_ifHighSpeed:
      Status = VB.SetUint32 (fpBridge->Get_ifHighSpeed (), VBT_GAUGE);
      break;

    case kLEAF_ifPromiscuousMode:
      Status = VB.SetBool (fpBridge->Get_ifPromiscuousMode ());
      break;

    case kLEAF_ifConnectorPresent:
      Status = VB.SetBool (fpBridge->Get_ifConnectorPresent ());
      break;

    case kLEAF_ifAlias:
      fpBridge->Get_ifAlias (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_ifCounterDiscontinuityTime:
      Status = VB.SetTimeticks (fpBridge->Get_ifCounterDiscontinuityTime ());
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

SNMP_STATUS ifXEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmCounter64 C64;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ifName:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifInMulticastPkts:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifInBroadcastPkts:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifOutMulticastPkts:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifOutBroadcastPkts:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifHCInOctets:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifHCInUcastPkts:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifHCInMulticastPkts:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifHCInBroadcastPkts:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifHCOutOctets:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifHCOutUcastPkts:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifHCOutMulticastPkts:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifHCOutBroadcastPkts:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifLinkUpDownTrapEnable:
    {
      int ifLinkUpDownTrapEnable;

      Status = VB.GetInt32 (&ifLinkUpDownTrapEnable);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ifLinkUpDownTrapEnable (ifLinkUpDownTrapEnable);
      break;
    }

    case kLEAF_ifHighSpeed:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifPromiscuousMode:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifConnectorPresent:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifAlias:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifCounterDiscontinuityTime:
      Status = SNMP_NOT_WRITABLE;
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}


/*\ 
 *  ifStackTable and related objects 
\*/

/* ifStackTable */

ifStackTable::ifStackTable ()
  : BcmMibTable (kOID_ifStackTable, kNAME_ifStackTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ifStackEntry::kLEAF_ifStackStatus);
}

SNMP_STATUS ifStackTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ifStackEntry *pEntry;
  ifStackEntryBridge *pMyBridge = (ifStackEntryBridge *) pBridge;


  pEntry = new ifStackEntry (this, pMyBridge);
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

/* ifStackEntry */

ifStackEntry::ifStackEntry (ifStackTable *pTable, ifStackEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ifStackEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_ifStackHigherLayer, kNAME_ifStackHigherLayer, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ifStackLowerLayer, kNAME_ifStackLowerLayer, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ifStackStatus, kNAME_ifStackStatus, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ifStackEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ifStackHigherLayer: // not-accessible
      Status = VB.SetInt32 (fpBridge->Get_ifStackHigherLayer ());
      break;

    case kLEAF_ifStackLowerLayer: // not-accessible
      Status = VB.SetInt32 (fpBridge->Get_ifStackLowerLayer ());
      break;

    case kLEAF_ifStackStatus:
      Status = VB.SetInt32 (fpBridge->Get_ifStackStatus ());
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

#if 0 // NOT REQUIRED OR SUPPORTED

/*\ 
 *  ifRcvAddressTable and related objects 
\*/

/* ifRcvAddressTable */

ifRcvAddressTable::ifRcvAddressTable ()
  : BcmMibTable (kOID_ifRcvAddressTable, kNAME_ifRcvAddressTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ifRcvAddressEntry::kLEAF_ifRcvAddressStatus);
  fColumnMask |= (1 << ifRcvAddressEntry::kLEAF_ifRcvAddressType);
}

SNMP_STATUS ifRcvAddressTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ifRcvAddressEntry *pEntry;
  ifRcvAddressEntryBridge *pMyBridge = (ifRcvAddressEntryBridge *) pBridge;


  pEntry = new ifRcvAddressEntry (this, pMyBridge);
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

BcmMibEntry *ifRcvAddressTable::NewDefaultEntry (BcmObjectId &Index)
{
  if (fpAgent == NULL)
    return NULL;

  /* Verify index length. */

  // ifIndex (numerical type): 1 element
  // ifRcvAddressAddress (MacAddress): 6 elements
  if (Index.Length() != 7)
    return NULL;

  /* Verify index value(s) */

  // UNFINISHED: ifIndex (unbounded numerical type)
  // Verify value of Index[0] if necessary.

  // ifRcvAddressAddress (MacAddress)
  if ((Index[1] < 0) || (Index[1] > 255)
  ||  (Index[2] < 0) || (Index[2] > 255)
  ||  (Index[3] < 0) || (Index[3] > 255)
  ||  (Index[4] < 0) || (Index[4] > 255)
  ||  (Index[5] < 0) || (Index[5] > 255)
  ||  (Index[6] < 0) || (Index[6] > 255))
    return NULL;

  SnmpLogRaw << "UNFINISHED: double-check index values in ifRcvAddressTable::NewDefaultEntry" << endl;

  ifRcvAddressEntryBridge *pBridge = new ifRcvAddressEntryBridge (Index, fpAgent);

  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
  {
    SnmpLogInit << "ifRcvAddressTable::NewDefaultEntry Bridge registration failed!  Deleting bridge object." << endl;
    delete (pBridge);
    return NULL;
  }
  
  return (BcmMibEntry *)pBridge->fpObject;
}

/* ifRcvAddressEntry */

ifRcvAddressEntry::ifRcvAddressEntry (ifRcvAddressTable *pTable, ifRcvAddressEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ifRcvAddressEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new MacAddrMibObject (kOID_ifRcvAddressAddress, kNAME_ifRcvAddressAddress, this, kNotAccessible);
  AddObject (pObject);

  pObject = new RowStatusMibObject (pTable, this, kOID_ifRcvAddressStatus, kNAME_ifRcvAddressStatus, kReadCreate);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ifRcvAddressType, kNAME_ifRcvAddressType, this, kReadCreate);
  AddObject (pObject);
}

SNMP_STATUS ifRcvAddressEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmMacAddress Mac;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ifRcvAddressAddress: // not-accessible
      fpBridge->Get_ifRcvAddressAddress (Mac);
      Status = VB.SetMacAddr (Mac);
      break;

    case kLEAF_ifRcvAddressStatus:
      Status = VB.SetInt32 (fpBridge->Get_ifRcvAddressStatus ());
      break;

    case kLEAF_ifRcvAddressType:
      Status = VB.SetInt32 (fpBridge->Get_ifRcvAddressType ());
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

SNMP_STATUS ifRcvAddressEntry::TestLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmMacAddress Mac;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ifRcvAddressAddress:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_ifRcvAddressStatus:
    {
      int ifRcvAddressStatus;

      Status = VB.GetInt32 (&ifRcvAddressStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_ifRcvAddressStatus (ifRcvAddressStatus);
      break;
    }

  }

  return Status;
}

SNMP_STATUS ifRcvAddressEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmMacAddress Mac;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  // If this row has not yet been created, either reject the set or create the
  // row; either is allowed per RFC-2579.  See note in MibObjs.h for details.
  if ((fpBridge->Get_ifRcvAddressStatus() == SNMP_STATUS_INVISIBLE)
  &&  ((fpBridge->fObjectMask & ifRcvAddressEntryBridge::kBIT_ifRcvAddressStatus) == 0))
  {
#ifdef SUPPORT_INDIRECT_ROW_CREATION
    int CreateStatus = SNMP_STATUS_CREATE_AND_WAIT;

    SnmpLogReq << "Indirect row creation supported, creating ifRcvAddressEntry" << endl;
    Status = fpBridge->Set_ifRcvAddressStatus (CreateStatus);
    if (Status != SNMP_NO_ERROR)
      return Status;
#else
    SnmpLogReq << "ifRcvAddressStatus not set (" << (void *)fpBridge->fObjectMask << "), INCONSISTENT_VALUE" << endl;
    return SNMP_INCONSISTENT_VALUE | SNMP_ROW_NOT_CREATED;
#endif
  }

  switch (Leaf.Leaf())
  {
    case kLEAF_ifRcvAddressAddress:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_ifRcvAddressStatus:
    {
      int ifRcvAddressStatus;

      Status = VB.GetInt32 (&ifRcvAddressStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ifRcvAddressStatus (ifRcvAddressStatus);
      break;
    }

    case kLEAF_ifRcvAddressType:
    {
      int ifRcvAddressType;

      Status = VB.GetInt32 (&ifRcvAddressType);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ifRcvAddressType (ifRcvAddressType);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}


/*\ 
 *  ifTestTable and related objects 
\*/

/* ifTestTable */

ifTestTable::ifTestTable ()
  : BcmMibTable (kOID_ifTestTable, kNAME_ifTestTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ifTestEntry::kLEAF_ifTestId);
  fColumnMask |= (1 << ifTestEntry::kLEAF_ifTestStatus);
  fColumnMask |= (1 << ifTestEntry::kLEAF_ifTestType);
  fColumnMask |= (1 << ifTestEntry::kLEAF_ifTestResult);
  fColumnMask |= (1 << ifTestEntry::kLEAF_ifTestCode);
  fColumnMask |= (1 << ifTestEntry::kLEAF_ifTestOwner);
}

SNMP_STATUS ifTestTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ifTestEntry *pEntry;
  ifTestEntryBridge *pMyBridge = (ifTestEntryBridge *) pBridge;


  pEntry = new ifTestEntry (this, pMyBridge);
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

/* ifTestEntry */

ifTestEntry::ifTestEntry (ifTestTable *pTable, ifTestEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ifTestEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new TestAndIncrMibObject (kOID_ifTestId, kNAME_ifTestId, this, kReadWrite);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ifTestStatus, kNAME_ifTestStatus, this, kReadWrite);
  AddObject (pObject);

  pObject = new OidMibObject (kOID_ifTestType, kNAME_ifTestType, this, kReadWrite);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ifTestResult, kNAME_ifTestResult, this, kReadOnly);
  AddObject (pObject);

  pObject = new OidMibObject (kOID_ifTestCode, kNAME_ifTestCode, this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_ifTestOwner, kNAME_ifTestOwner, this, kReadWrite);
  AddObject (pObject);
}

SNMP_STATUS ifTestEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmObjectId Oid;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ifTestId:
/*
      // NOTE: normally, TestAndIncr objects don't need bridge
      //       object methods for get, test, or set; the MIB
      //       object will just return SNMP_NO_OPINION and let the
      //       TestAndIncrMibObject do its thing.  However, a
      //       method is provided here in case you need to provide
      //       non-standard custom behaviour, with the following form:
      Status = VB.SetInt32 (fpBridge->Get_ifTestId ());
*/
      // ifTestId is a TestAndIncr object, let it handle itself.
      Status = SNMP_NO_OPINION;
      break;

    case kLEAF_ifTestStatus:
      Status = VB.SetInt32 (fpBridge->Get_ifTestStatus ());
      break;

    case kLEAF_ifTestType:
      fpBridge->Get_ifTestType (Oid);
      Status = VB.SetOid (Oid);
      break;

    case kLEAF_ifTestResult:
      Status = VB.SetInt32 (fpBridge->Get_ifTestResult ());
      break;

    case kLEAF_ifTestCode:
      fpBridge->Get_ifTestCode (Oid);
      Status = VB.SetOid (Oid);
      break;

    case kLEAF_ifTestOwner:
    {
      BcmString String;

      fpBridge->Get_ifTestOwner (String);
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

SNMP_STATUS ifTestEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmObjectId Oid;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ifTestId:
/*
      // NOTE: normally, TestAndIncr objects don't need bridge
      //       object methods for get, test, or set; the MIB
      //       object will just return SNMP_NO_OPINION and let the
      //       TestAndIncrMibObject do its thing.  However, a
      //       method is provided here in case you need to provide
      //       non-standard custom behaviour, with the following form:
      int ifTestId

      Status = VB.GetInt32 (&ifTestId);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ifTestId (ifTestId);
*/
      // ifTestId is a TestAndIncr object, let it handle itself.
      Status = SNMP_NO_OPINION;
      break;

    case kLEAF_ifTestStatus:
    {
      int ifTestStatus;

      Status = VB.GetInt32 (&ifTestStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ifTestStatus (ifTestStatus);
      break;
    }

    case kLEAF_ifTestType:
      Status = VB.GetOid (Oid);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ifTestType (Oid);
      break;

    case kLEAF_ifTestResult:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifTestCode:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ifTestOwner:
    {
      BcmString String;

      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ifTestOwner (String);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}

#endif
