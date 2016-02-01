//**************************************************************************
//                                                                          
//    Copyright 2007 Broadcom Corporation                                   
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
//    Filename: udpMib.cpp
//    Creation Date: Monday, April 16, 2007 at 15:13:40
//    Created by Broadcom BFC Mib Compiler BCMIBC.EXE version 3.6.0b
//    from input file udp-mib_rfc4113.mib
//
//**************************************************************************

#include "udpMib.h"
#include "udpBridge.h"
#include "SnmpLog.h" // for SnmpLog... macros
#include "BcmSnmpAgent.h"

#include "MibFactory.h"

/*\
 *  udpGroup group
\*/

udpGroup::udpGroup ()
  : BcmMibGroup (kOID_udpGroup, kNAME_udpGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool udpGroup::Populate ()
{
  BcmMibObject *pScalar;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new Uint32MibObject (kOID_udpInDatagrams, kNAME_udpInDatagrams, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_udpNoPorts, kNAME_udpNoPorts, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_udpInErrors, kNAME_udpInErrors, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_udpOutDatagrams, kNAME_udpOutDatagrams, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint64MibObject (kOID_udpHCInDatagrams, kNAME_udpHCInDatagrams, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint64MibObject (kOID_udpHCOutDatagrams, kNAME_udpHCOutDatagrams, this, kReadOnly);
  AddScalar (pScalar);

  fpUdpEndpointTable = new udpEndpointTable();
  AddTable (fpUdpEndpointTable);

  fpUdpTable = new udpTable();
  AddTable (fpUdpTable);

  return true;
}

SNMP_STATUS udpGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (udpGroupBridge *) pBridge;

  fpUdpTable->fpUdpBridge = fpBridge;
  fpUdpEndpointTable->fpUdpBridge = fpBridge;

  return Status;
}

SNMP_STATUS udpGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;
  BcmCounter64 C64;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_udpInDatagrams:
      Status = VB.SetUint32 (fpBridge->Get_udpInDatagrams (), VBT_COUNTER);
      break;

    case kLEAF_udpNoPorts:
      Status = VB.SetUint32 (fpBridge->Get_udpNoPorts (), VBT_COUNTER);
      break;

    case kLEAF_udpInErrors:
      Status = VB.SetUint32 (fpBridge->Get_udpInErrors (), VBT_COUNTER);
      break;

    case kLEAF_udpOutDatagrams:
      Status = VB.SetUint32 (fpBridge->Get_udpOutDatagrams (), VBT_COUNTER);
      break;

    case kLEAF_udpHCInDatagrams:
      fpBridge->Get_udpHCInDatagrams (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_udpHCOutDatagrams:
      fpBridge->Get_udpHCOutDatagrams (C64);
      Status = VB.SetUint64 (C64);
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
 *  udpEndpointTable and related objects 
\*/

/* udpEndpointTable */

udpEndpointTable::udpEndpointTable ()
  : BcmMibTable (kOID_udpEndpointTable, kNAME_udpEndpointTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << udpEndpointEntry::kLEAF_udpEndpointProcess);
  
  fpUdpBridge = NULL;
}

SNMP_STATUS udpEndpointTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  udpEndpointEntry *pEntry;
  udpEndpointEntryBridge *pMyBridge = (udpEndpointEntryBridge *) pBridge;


  pEntry = new udpEndpointEntry (this, pMyBridge);
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

void udpEndpointTable::Update ()
{
  if (fpUdpBridge)
    fpUdpBridge->UpdateUdpEndpointTable();
}

/* udpEndpointEntry */

udpEndpointEntry::udpEndpointEntry (udpEndpointTable *pTable, udpEndpointEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_udpEndpointEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_udpEndpointLocalAddressType, kNAME_udpEndpointLocalAddressType, this, kNotAccessible);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_udpEndpointLocalAddress, kNAME_udpEndpointLocalAddress, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_udpEndpointLocalPort, kNAME_udpEndpointLocalPort, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_udpEndpointRemoteAddressType, kNAME_udpEndpointRemoteAddressType, this, kNotAccessible);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_udpEndpointRemoteAddress, kNAME_udpEndpointRemoteAddress, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_udpEndpointRemotePort, kNAME_udpEndpointRemotePort, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_udpEndpointInstance, kNAME_udpEndpointInstance, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_udpEndpointProcess, kNAME_udpEndpointProcess, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS udpEndpointEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_udpEndpointLocalAddressType:
      Status = VB.SetInt32 (fpBridge->Get_udpEndpointLocalAddressType ());
      break;

    case kLEAF_udpEndpointLocalAddress:
      fpBridge->Get_udpEndpointLocalAddress (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_udpEndpointLocalPort:
      Status = VB.SetUint32 (fpBridge->Get_udpEndpointLocalPort (), VBT_UNSIGNED32);
      break;

    case kLEAF_udpEndpointRemoteAddressType:
      Status = VB.SetInt32 (fpBridge->Get_udpEndpointRemoteAddressType ());
      break;

    case kLEAF_udpEndpointRemoteAddress:
      fpBridge->Get_udpEndpointRemoteAddress (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_udpEndpointRemotePort:
      Status = VB.SetUint32 (fpBridge->Get_udpEndpointRemotePort (), VBT_UNSIGNED32);
      break;

    case kLEAF_udpEndpointInstance:
      Status = VB.SetUint32 (fpBridge->Get_udpEndpointInstance (), VBT_UNSIGNED32);
      break;

    case kLEAF_udpEndpointProcess:
      Status = VB.SetUint32 (fpBridge->Get_udpEndpointProcess (), VBT_UNSIGNED32);
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
 *  udpTable and related objects 
\*/

/* udpTable */

udpTable::udpTable ()
  : BcmMibTable (kOID_udpTable, kNAME_udpTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << udpEntry::kLEAF_udpLocalAddress);
  fColumnMask |= (1 << udpEntry::kLEAF_udpLocalPort);
  
  fpUdpBridge = NULL;
}

SNMP_STATUS udpTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  udpEntry *pEntry;
  udpEntryBridge *pMyBridge = (udpEntryBridge *) pBridge;


  pEntry = new udpEntry (this, pMyBridge);
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

void udpTable::Update ()
{
  if (fpUdpBridge)
    fpUdpBridge->UpdateUdpTable();
}

/* udpEntry */

udpEntry::udpEntry (udpTable *pTable, udpEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_udpEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new IpAddrMibObject (kOID_udpLocalAddress, kNAME_udpLocalAddress, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_udpLocalPort, kNAME_udpLocalPort, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS udpEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmIpAddress Ip;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_udpLocalAddress:
      fpBridge->Get_udpLocalAddress (Ip);
      Status = VB.SetIp (Ip);
      break;

    case kLEAF_udpLocalPort:
      Status = VB.SetInt32 (fpBridge->Get_udpLocalPort ());
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

