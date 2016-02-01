//****************************************************************************
//
// Copyright (c) 2002-2009 Broadcom Corporation
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
//**************************************************************************
//    Revision History:                                                     
//                                                                          
//**************************************************************************
//    Filename: ipMib.cpp
//    Creation Date: Thursday, April 19, 2007 at 16:18:15
//    Created by Broadcom BFC Mib Compiler BCMIBC.EXE version 3.6.0b
//    from input file ip-mib_rfc4293.mib
//
//**************************************************************************

#include "ipMib.h"
#include "ipBridge.h"
#include "SnmpLog.h" // for SnmpLog... macros
#include "BcmSnmpAgent.h"

#include "IpStackManager.h"
#include "DhcpClientIf.h"
#include "DhcpClientLease.h"

#if (BCM_IPV6_SUPPORT)
#include "DhcpV6ClientIf.h"
#endif



/*\
 *  ipGroup group
\*/

ipGroup::ipGroup ()
  : BcmMibGroup (kOID_ipGroup, kNAME_ipGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ipGroup::Populate ()
{
  BcmMibObject *pScalar;
  BcmMibTable *pTable;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new Int32MibObject (kOID_ipForwarding, kNAME_ipForwarding, this, kReadWrite);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ipDefaultTTL, kNAME_ipDefaultTTL, this, kReadWrite);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ipReasmTimeout, kNAME_ipReasmTimeout, this, kReadOnly);
  AddScalar (pScalar);
  
  #if (BCM_IPV6_SUPPORT)
  
  pScalar = new Int32MibObject (kOID_ipv6IpForwarding, kNAME_ipv6IpForwarding, this, kReadWrite);
  AddScalar (pScalar);

  pScalar = new Int32MibObject (kOID_ipv6IpDefaultHopLimit, kNAME_ipv6IpDefaultHopLimit, this, kReadWrite);
  AddScalar (pScalar);
  
  #endif

  pScalar = new TimeticksMibObject (kOID_ipv4InterfaceTableLastChange, kNAME_ipv4InterfaceTableLastChange, this, kReadOnly);
  AddScalar (pScalar);
  
  #if (BCM_IPV6_SUPPORT)

  pScalar = new TimeticksMibObject (kOID_ipv6InterfaceTableLastChange, kNAME_ipv6InterfaceTableLastChange, this, kReadOnly);
  AddScalar (pScalar);
  
  #endif

  pScalar = new TestAndIncrMibObject (kOID_ipAddressSpinLock, kNAME_ipAddressSpinLock, this, kReadWrite);
  AddScalar (pScalar);
  
  #if (BCM_IPV6_SUPPORT)
  #if 0  //DOCSIS 3.0 says don't support any ipv6RouterAdvert objects
  pScalar = new TestAndIncrMibObject (kOID_ipv6RouterAdvertSpinLock, kNAME_ipv6RouterAdvertSpinLock, this, kReadWrite);
  AddScalar (pScalar);
  #endif
  #endif

  pScalar = new Uint32MibObject (kOID_ipInReceives, kNAME_ipInReceives, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ipInHdrErrors, kNAME_ipInHdrErrors, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ipInAddrErrors, kNAME_ipInAddrErrors, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ipForwDatagrams, kNAME_ipForwDatagrams, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ipInUnknownProtos, kNAME_ipInUnknownProtos, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ipInDiscards, kNAME_ipInDiscards, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ipInDelivers, kNAME_ipInDelivers, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ipOutRequests, kNAME_ipOutRequests, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ipOutDiscards, kNAME_ipOutDiscards, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ipOutNoRoutes, kNAME_ipOutNoRoutes, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ipReasmReqds, kNAME_ipReasmReqds, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ipReasmOKs, kNAME_ipReasmOKs, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ipReasmFails, kNAME_ipReasmFails, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ipFragOKs, kNAME_ipFragOKs, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ipFragFails, kNAME_ipFragFails, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ipFragCreates, kNAME_ipFragCreates, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_ipRoutingDiscards, kNAME_ipRoutingDiscards, this, kReadOnly);
  AddScalar (pScalar);

  ipv4InterfaceTable *pIpv4InterfaceTable = new ipv4InterfaceTable();
  AddTable (pIpv4InterfaceTable);
  
  #if (BCM_IPV6_SUPPORT)

  ipv6InterfaceTable *pIpv6InterfaceTable = new ipv6InterfaceTable();
  AddTable (pIpv6InterfaceTable);
  
  #endif
  
  #if 0

  pTable = new ipAddressPrefixTable();
  AddTable (pTable);
  
  #endif

  #if (BCM_IPV6_SUPPORT)
  pTable = new ipAddressTable(pIpv4InterfaceTable, pIpv6InterfaceTable);
  #else
  pTable = new ipAddressTable(pIpv4InterfaceTable);
  #endif
  AddTable (pTable);

  pTable = new ipNetToPhysicalTable();
  AddTable (pTable);
  
  #if 0

  pTable = new ipv6ScopeZoneIndexTable();
  AddTable (pTable);
  
  #endif

  pTable = new ipDefaultRouterTable();
  AddTable (pTable);
  
  #if 0

  pTable = new ipv6RouterAdvertTable();
  AddTable (pTable);
  
  #endif

  pTable = new ipAddrTable(pIpv4InterfaceTable);
  AddTable (pTable);

  pTable = new ipNetToMediaTable();
  AddTable (pTable);
  
  #if 0

  pGroup = new ipTrafficStatsGroup();
  AddGroup (pGroup);
  
  #endif

  return true;
}

SNMP_STATUS ipGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ipGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ipGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ipForwarding:
      Status = VB.SetInt32 (fpBridge->Get_ipForwarding ());
      break;

    case kLEAF_ipDefaultTTL:
      Status = VB.SetInt32 (fpBridge->Get_ipDefaultTTL ());
      break;

    case kLEAF_ipReasmTimeout:
      Status = VB.SetInt32 (fpBridge->Get_ipReasmTimeout ());
      break;
      
    #if (BCM_IPV6_SUPPORT)

    case kLEAF_ipv6IpForwarding:
      Status = VB.SetInt32 (fpBridge->Get_ipv6IpForwarding ());
      break;

    case kLEAF_ipv6IpDefaultHopLimit:
      Status = VB.SetInt32 (fpBridge->Get_ipv6IpDefaultHopLimit ());
      break;
      
    #endif

    case kLEAF_ipv4InterfaceTableLastChange:
      Status = VB.SetTimeticks (fpBridge->Get_ipv4InterfaceTableLastChange ());
      break;
      
    #if (BCM_IPV6_SUPPORT)

    case kLEAF_ipv6InterfaceTableLastChange:
      Status = VB.SetTimeticks (fpBridge->Get_ipv6InterfaceTableLastChange ());
      break;
      
    #endif

    case kLEAF_ipAddressSpinLock:
/*
      // NOTE: normally, TestAndIncr objects don't need bridge
      //       object methods for get, test, or set; the MIB
      //       object will just return SNMP_NO_OPINION and let the
      //       TestAndIncrMibObject do its thing.  However, a
      //       method is provided here in case you need to provide
      //       non-standard custom behaviour, with the following form:
      Status = VB.SetInt32 (fpBridge->Get_ipAddressSpinLock ());
*/
      // ipAddressSpinLock is a TestAndIncr object, let it handle itself.
      Status = SNMP_NO_OPINION;
      break;

#if 0  //DOCSIS 3.0 says don't support any ipv6RouterAdvert objects    
    case kLEAF_ipv6RouterAdvertSpinLock:
/*
      // NOTE: normally, TestAndIncr objects don't need bridge
      //       object methods for get, test, or set; the MIB
      //       object will just return SNMP_NO_OPINION and let the
      //       TestAndIncrMibObject do its thing.  However, a
      //       method is provided here in case you need to provide
      //       non-standard custom behaviour, with the following form:
      Status = VB.SetInt32 (fpBridge->Get_ipv6RouterAdvertSpinLock ());
*/
      // ipv6RouterAdvertSpinLock is a TestAndIncr object, let it handle itself.
      Status = SNMP_NO_OPINION;
      break;
#endif

    case kLEAF_ipInReceives:
      Status = VB.SetUint32 (fpBridge->Get_ipInReceives (), VBT_COUNTER);
      break;

    case kLEAF_ipInHdrErrors:
      Status = VB.SetUint32 (fpBridge->Get_ipInHdrErrors (), VBT_COUNTER);
      break;

    case kLEAF_ipInAddrErrors:
      Status = VB.SetUint32 (fpBridge->Get_ipInAddrErrors (), VBT_COUNTER);
      break;

    case kLEAF_ipForwDatagrams:
      Status = VB.SetUint32 (fpBridge->Get_ipForwDatagrams (), VBT_COUNTER);
      break;

    case kLEAF_ipInUnknownProtos:
      Status = VB.SetUint32 (fpBridge->Get_ipInUnknownProtos (), VBT_COUNTER);
      break;

    case kLEAF_ipInDiscards:
      Status = VB.SetUint32 (fpBridge->Get_ipInDiscards (), VBT_COUNTER);
      break;

    case kLEAF_ipInDelivers:
      Status = VB.SetUint32 (fpBridge->Get_ipInDelivers (), VBT_COUNTER);
      break;

    case kLEAF_ipOutRequests:
      Status = VB.SetUint32 (fpBridge->Get_ipOutRequests (), VBT_COUNTER);
      break;

    case kLEAF_ipOutDiscards:
      Status = VB.SetUint32 (fpBridge->Get_ipOutDiscards (), VBT_COUNTER);
      break;

    case kLEAF_ipOutNoRoutes:
      Status = VB.SetUint32 (fpBridge->Get_ipOutNoRoutes (), VBT_COUNTER);
      break;

    case kLEAF_ipReasmReqds:
      Status = VB.SetUint32 (fpBridge->Get_ipReasmReqds (), VBT_COUNTER);
      break;

    case kLEAF_ipReasmOKs:
      Status = VB.SetUint32 (fpBridge->Get_ipReasmOKs (), VBT_COUNTER);
      break;

    case kLEAF_ipReasmFails:
      Status = VB.SetUint32 (fpBridge->Get_ipReasmFails (), VBT_COUNTER);
      break;

    case kLEAF_ipFragOKs:
      Status = VB.SetUint32 (fpBridge->Get_ipFragOKs (), VBT_COUNTER);
      break;

    case kLEAF_ipFragFails:
      Status = VB.SetUint32 (fpBridge->Get_ipFragFails (), VBT_COUNTER);
      break;

    case kLEAF_ipFragCreates:
      Status = VB.SetUint32 (fpBridge->Get_ipFragCreates (), VBT_COUNTER);
      break;

    case kLEAF_ipRoutingDiscards:
      Status = VB.SetUint32 (fpBridge->Get_ipRoutingDiscards (), VBT_COUNTER);
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

SNMP_STATUS ipGroup::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ipForwarding:
    {
      int ipForwarding;

      Status = VB.GetInt32 (&ipForwarding);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipForwarding (ipForwarding);
      break;
    }

    case kLEAF_ipDefaultTTL:
    {
      int ipDefaultTTL;

      Status = VB.GetInt32 (&ipDefaultTTL);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipDefaultTTL (ipDefaultTTL);
      break;
    }

    case kLEAF_ipReasmTimeout:
      Status = SNMP_NOT_WRITABLE;
      break;
      
    #if (BCM_IPV6_SUPPORT)

    case kLEAF_ipv6IpForwarding:
    {
      int ipv6IpForwarding;

      Status = VB.GetInt32 (&ipv6IpForwarding);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipv6IpForwarding (ipv6IpForwarding);
      break;
    }

    case kLEAF_ipv6IpDefaultHopLimit:
    {
      int ipv6IpDefaultHopLimit;

      Status = VB.GetInt32 (&ipv6IpDefaultHopLimit);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipv6IpDefaultHopLimit (ipv6IpDefaultHopLimit);
      break;
    }
    
    #endif

    case kLEAF_ipv4InterfaceTableLastChange:
      Status = SNMP_NOT_WRITABLE;
      break;
      
    #if (BCM_IPV6_SUPPORT)

    case kLEAF_ipv6InterfaceTableLastChange:
      Status = SNMP_NOT_WRITABLE;
      break;
      
    #endif

    case kLEAF_ipAddressSpinLock:
/*
      // NOTE: normally, TestAndIncr objects don't need bridge
      //       object methods for get, test, or set; the MIB
      //       object will just return SNMP_NO_OPINION and let the
      //       TestAndIncrMibObject do its thing.  However, a
      //       method is provided here in case you need to provide
      //       non-standard custom behaviour, with the following form:
      int ipAddressSpinLock

      Status = VB.GetInt32 (&ipAddressSpinLock);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipAddressSpinLock (ipAddressSpinLock);
*/
      // ipAddressSpinLock is a TestAndIncr object, let it handle itself.
      Status = SNMP_NO_OPINION;
      break;

#if 0  //DOCSIS 3.0 says don't support any ipv6RouterAdvert objects
    case kLEAF_ipv6RouterAdvertSpinLock:
/*
      // NOTE: normally, TestAndIncr objects don't need bridge
      //       object methods for get, test, or set; the MIB
      //       object will just return SNMP_NO_OPINION and let the
      //       TestAndIncrMibObject do its thing.  However, a
      //       method is provided here in case you need to provide
      //       non-standard custom behaviour, with the following form:
      int ipv6RouterAdvertSpinLock

      Status = VB.GetInt32 (&ipv6RouterAdvertSpinLock);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipv6RouterAdvertSpinLock (ipv6RouterAdvertSpinLock);
*/
      // ipv6RouterAdvertSpinLock is a TestAndIncr object, let it handle itself.
      Status = SNMP_NO_OPINION;
      break;
#endif

    case kLEAF_ipInReceives:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipInHdrErrors:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipInAddrErrors:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipForwDatagrams:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipInUnknownProtos:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipInDiscards:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipInDelivers:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipOutRequests:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipOutDiscards:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipOutNoRoutes:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipReasmReqds:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipReasmOKs:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipReasmFails:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipFragOKs:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipFragFails:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipFragCreates:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipRoutingDiscards:
      Status = SNMP_NOT_WRITABLE;
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}


/*\ 
 *  ipv4InterfaceTable and related objects 
\*/

// Helper function used here and by the ipv6InterfaceTable
ifEntryBridge *FindIfEntryByPhysAddress (BcmSnmpAgent *pAgent, const BcmMacAddress &ifPhysAddress)
{
  BcmMacAddress Mac;
  ifEntryBridge *pIfEntry = pAgent->IfBridge().FirstIfEntry();
  
  while (pIfEntry)
  {
    pIfEntry->Get_ifPhysAddress(Mac);
    
    if (Mac == ifPhysAddress)
      break;
      
    pIfEntry = pAgent->IfBridge().NextIfEntry(pIfEntry->Get_ifIndex());
  }
  
  return pIfEntry;
}

/* ipv4InterfaceTable */

ipv4InterfaceTable::ipv4InterfaceTable ()
  : BcmMibTable (kOID_ipv4InterfaceTable, kNAME_ipv4InterfaceTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ipv4InterfaceEntry::kLEAF_ipv4InterfaceReasmMaxSize);
  fColumnMask |= (1 << ipv4InterfaceEntry::kLEAF_ipv4InterfaceEnableStatus);
  fColumnMask |= (1 << ipv4InterfaceEntry::kLEAF_ipv4InterfaceRetransmitTime);
  
  // HAL MAP is empty to begin with - all 0's
  memset (&fIpHalMap, 0, BCM_NUM_IP_STACKS * sizeof(int));
  fInitialized = false;
}

#if !defined (TARGETOS_Linux)
void ipv4InterfaceTable::Update()
{
  // Auto update on first query.  After that the contents of this table should
  // not change.
  if (fInitialized)
    return;
    
  if (fpAgent)
  {
    BcmBaseIpHalIf *pIpHal;
  
    // Initialize our HAL map.  Default behavior here is to iterate over all
    // IP stacks and map each one to the ifTable entry of which ifPhysAddress
    // matches the MAC address of the stack.  If no such match exists, then
    // use the ifIndex of the default HAL for our agent.
    for (int Stack = 1; Stack <= BCM_NUM_IP_STACKS; Stack++)
    {
      pIpHal = BcmIpStackManager::GetSingleton().Find(Stack);
      
      // Don't re-map if this has been explicitly mapped already.
      if ((pIpHal) && (IpIfIndex(pIpHal) == 0))
      {
        int ifIndex = 0;
      
        ifEntryBridge *pIfEntry = FindIfEntryByPhysAddress (fpAgent, pIpHal->MacAddress());
        if (pIfEntry)
          ifIndex = pIfEntry->Get_ifIndex();
        else
        {
          BcmHalIf *pHal = fpAgent->GetPacketSwitch()->DefaultHalIf();
          
          if (pHal)
            ifIndex = pHal->IfTableIndex(fpAgent);
        }
      
        IpIfIndex (pIpHal, ifIndex);
      }
    }
    
    fInitialized = true;
  }
}
#endif

SNMP_STATUS ipv4InterfaceTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ipv4InterfaceEntry *pEntry;
  ipv4InterfaceEntryBridge *pMyBridge = (ipv4InterfaceEntryBridge *) pBridge;


  pEntry = new ipv4InterfaceEntry (this, pMyBridge);
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

// Methods to map an IP stack HAL to an ifIndex
bool ipv4InterfaceTable::IpIfIndex (const BcmBaseIpHalIf *pIpHal, int ifIndex)
{
  // Make sure the stack number is not out of range
  if ((pIpHal->StackNumber() < 1)
  ||  (pIpHal->StackNumber() > BCM_NUM_IP_STACKS))
    return false;
    
  // Map it.
  fIpHalMap[pIpHal->StackNumber() - 1] = ifIndex;
  
  // Flush and rebuild the table.
  Flush();
  
  ipv4InterfaceEntryBridge *pBridge;
  
  for (int Stack = 0; Stack < BCM_NUM_IP_STACKS; Stack++)
  {
    if (fIpHalMap[Stack] != 0)
    {
      pBridge = new ipv4InterfaceEntryBridge(fIpHalMap[Stack], fpAgent);
      
      // Calling BcmMibTable::RegisterBridge here is much faster then 
      // calling BcmMibBridge::Register because it means we don't have
      // to search for the table.
      if (RegisterBridge(pBridge, true) != SNMP_NO_ERROR)
        delete pBridge;
    }
  }
  
  // If our ifTable includes a software loopack entry, then install
  // a corresponding entry
  ifEntryBridge *pLoopback = fpAgent->IfBridge().FirstIfEntryOfType(CONSTVAL_IANAIFTYPE_SOFTWARELOOPBACK);
  if (pLoopback)
  {
    pBridge = new ipv4InterfaceEntryBridge (pLoopback->Get_ifIndex(), fpAgent);
    if (RegisterBridge(pBridge, true) != SNMP_NO_ERROR)
      delete pBridge;
  }
  
  return true;
}

int ipv4InterfaceTable::IpIfIndex (const BcmBaseIpHalIf *pIpHal) const
{
  // Make sure the stack number is not out of range
  if (pIpHal->StackNumber() > BCM_NUM_IP_STACKS)
    return 0;

  // Map it.
  return fIpHalMap[pIpHal->StackNumber() - 1];
}

/* ipv4InterfaceEntry */

ipv4InterfaceEntry::ipv4InterfaceEntry (ipv4InterfaceTable *pTable, ipv4InterfaceEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ipv4InterfaceEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_ipv4InterfaceIfIndex, kNAME_ipv4InterfaceIfIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipv4InterfaceReasmMaxSize, kNAME_ipv4InterfaceReasmMaxSize, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipv4InterfaceEnableStatus, kNAME_ipv4InterfaceEnableStatus, this, kReadWrite);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv4InterfaceRetransmitTime, kNAME_ipv4InterfaceRetransmitTime, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ipv4InterfaceEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ipv4InterfaceIfIndex:
      Status = VB.SetInt32 (fpBridge->Get_ipv4InterfaceIfIndex ());
      break;

    case kLEAF_ipv4InterfaceReasmMaxSize:
      Status = VB.SetInt32 (fpBridge->Get_ipv4InterfaceReasmMaxSize ());
      break;

    case kLEAF_ipv4InterfaceEnableStatus:
      Status = VB.SetInt32 (fpBridge->Get_ipv4InterfaceEnableStatus ());
      break;

    case kLEAF_ipv4InterfaceRetransmitTime:
      Status = VB.SetUint32 (fpBridge->Get_ipv4InterfaceRetransmitTime (), VBT_UNSIGNED32);
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

SNMP_STATUS ipv4InterfaceEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ipv4InterfaceIfIndex:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_ipv4InterfaceReasmMaxSize:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipv4InterfaceEnableStatus:
    {
      int ipv4InterfaceEnableStatus;

      Status = VB.GetInt32 (&ipv4InterfaceEnableStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipv4InterfaceEnableStatus (ipv4InterfaceEnableStatus);
      break;
    }

    case kLEAF_ipv4InterfaceRetransmitTime:
      Status = SNMP_NOT_WRITABLE;
      break;

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}

#if (BCM_IPV6_SUPPORT)

/*\ 
 *  ipv6InterfaceTable and related objects 
\*/

/* ipv6InterfaceTable */

ipv6InterfaceTable::ipv6InterfaceTable ()
  : BcmMibTable (kOID_ipv6InterfaceTable, kNAME_ipv6InterfaceTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ipv6InterfaceEntry::kLEAF_ipv6InterfaceReasmMaxSize);
  fColumnMask |= (1 << ipv6InterfaceEntry::kLEAF_ipv6InterfaceIdentifier);
  fColumnMask |= (1 << ipv6InterfaceEntry::kLEAF_ipv6InterfaceEnableStatus);
  fColumnMask |= (1 << ipv6InterfaceEntry::kLEAF_ipv6InterfaceReachableTime);
  fColumnMask |= (1 << ipv6InterfaceEntry::kLEAF_ipv6InterfaceRetransmitTime);
  fColumnMask |= (1 << ipv6InterfaceEntry::kLEAF_ipv6InterfaceForwarding);
  
  // HAL MAP is empty to begin with - all 0's
  memset (&fIpHalMap, 0, BCM_NUM_IP_STACKS * sizeof(int));
  fInitialized = false;
}

SNMP_STATUS ipv6InterfaceTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ipv6InterfaceEntry *pEntry;
  ipv6InterfaceEntryBridge *pMyBridge = (ipv6InterfaceEntryBridge *) pBridge;


  pEntry = new ipv6InterfaceEntry (this, pMyBridge);
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

#if !defined (TARGETOS_Linux)
void ipv6InterfaceTable::Update()
{
  // Auto update on first query.  After that the contents of this table should
  // not change.
  if (fInitialized)
    return;

  if (fpAgent)
  {
    BcmBaseIpHalIf *pIpHal;
  
    // Initialize our HAL map.  Default behavior here is to iterate over all
    // IP stacks and map each one to the ifTable entry of which ifPhysAddress
    // matches the MAC address of the stack.  If no such match exists, then
    // use the ifIndex of the default HAL for our agent.
    for (int Stack = 1; Stack <= BCM_NUM_IP_STACKS; Stack++)
    {
      pIpHal = BcmIpStackManager::GetSingleton().Find(Stack);
      
      // Don't re-map if this has been explicitly mapped already.
      if ((pIpHal) && (IpIfIndex(pIpHal) == 0))
      {
        int ifIndex = 0;
      
        ifEntryBridge *pIfEntry = FindIfEntryByPhysAddress (fpAgent, pIpHal->MacAddress());
        if (pIfEntry)
          ifIndex = pIfEntry->Get_ifIndex();
        else
        {
          BcmHalIf *pHal = fpAgent->GetPacketSwitch()->DefaultHalIf();
          
          if (pHal)
            ifIndex = pHal->IfTableIndex(fpAgent);
        }
      
        IpIfIndex (pIpHal, ifIndex);
      }
    }
    
    fInitialized = true;
  }
}
#endif

// Methods to map an IP stack HAL to an ifIndex
bool ipv6InterfaceTable::IpIfIndex (const BcmBaseIpHalIf *pIpHal, int ifIndex)
{
  // Make sure the stack number is not out of range
  if ((pIpHal->StackNumber() < 1)
  ||  (pIpHal->StackNumber() > BCM_NUM_IP_STACKS))
    return false;

  // Map it.
  fIpHalMap[pIpHal->StackNumber() - 1] = ifIndex;
  
  // Flush and rebuild the table.
  Flush();
  
  ipv6InterfaceEntryBridge *pBridge;
  
  for (int Stack = 0; Stack < BCM_NUM_IP_STACKS; Stack++)
  {
    if (fIpHalMap[Stack] != 0)
    {
      BcmString EUI64 = "";
      ifEntryBridge *pIfEntry = fpAgent->IfBridge().FindIfEntry(fIpHalMap[Stack]);
      
      if (pIfEntry)
      {
        BcmMacAddress ifPhysAddress;
        pIfEntry->Get_ifPhysAddress(ifPhysAddress);
        
        // Calculate the EUI-64 value for this interface's ifPhysAddress.
        // This is what must be returned for ipv6InterfaceIdentifier.
        if (ifPhysAddress != kAllZerosMac)
        {
          uint8 pAddr[6];
          ifPhysAddress.getAddr(pAddr);

          // An EUI-64 is a 64 bit value which is pretty much just the MAC
          // address split into 2 3-octet chunks with an 0xFF and an 0xFE
          // stuffed in between.  It's used in creating link-local addresses.
          EUI64 += pAddr[0];
          EUI64 += pAddr[1];
          EUI64 += pAddr[2];
          EUI64 += (char) 0xFF;
          EUI64 += (char) 0xFE;
          EUI64 += pAddr[3];
          EUI64 += pAddr[4];
          EUI64 += pAddr[5];
        }
        else
        {
          // What should I do for ipv6InterfaceIdentifier if there's no
          // ifPhysAddress?  Leave it empty I guess.  I suppose if the
          // ifEntry wraps a packet switch bridging HAL we could drill down
          // and try to find a MAC address on the other packet switch to use.
        }
      }
    
      pBridge = new ipv6InterfaceEntryBridge(fIpHalMap[Stack], fpAgent, pIpHal->NeighborDiscoveryThread(), EUI64);
      
      // Calling BcmMibTable::RegisterBridge here is much faster then 
      // calling BcmMibBridge::Register because it means we don't have
      // to search for the table.
      if (RegisterBridge(pBridge, true) != SNMP_NO_ERROR)
        delete pBridge;
    }
  }
  
  // If our ifTable includes a software loopack entry, then install a 
  // corresponding entry. No EUI-64 for SW loopback since it has no MAC address.
  ifEntryBridge *pLoopback = fpAgent->IfBridge().FirstIfEntryOfType(CONSTVAL_IANAIFTYPE_SOFTWARELOOPBACK);
  if (pLoopback)
  {
    pBridge = new ipv6InterfaceEntryBridge (pLoopback->Get_ifIndex(), fpAgent, pIpHal->NeighborDiscoveryThread(), BcmString(""));
    if (pBridge->Register (kDeleteBridge) != SNMP_NO_ERROR)
      delete pBridge;
  }
  
  return true;
}

int ipv6InterfaceTable::IpIfIndex (const BcmBaseIpHalIf *pIpHal) const
{
  // Make sure the stack number is not out of range
  if (pIpHal->StackNumber() > BCM_NUM_IP_STACKS)
    return 0;

  // Map it.
  return fIpHalMap[pIpHal->StackNumber() - 1];
}

/* ipv6InterfaceEntry */

ipv6InterfaceEntry::ipv6InterfaceEntry (ipv6InterfaceTable *pTable, ipv6InterfaceEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ipv6InterfaceEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_ipv6InterfaceIfIndex, kNAME_ipv6InterfaceIfIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv6InterfaceReasmMaxSize, kNAME_ipv6InterfaceReasmMaxSize, this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_ipv6InterfaceIdentifier, kNAME_ipv6InterfaceIdentifier, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipv6InterfaceEnableStatus, kNAME_ipv6InterfaceEnableStatus, this, kReadWrite);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv6InterfaceReachableTime, kNAME_ipv6InterfaceReachableTime, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv6InterfaceRetransmitTime, kNAME_ipv6InterfaceRetransmitTime, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipv6InterfaceForwarding, kNAME_ipv6InterfaceForwarding, this, kReadWrite);
  AddObject (pObject);
}

SNMP_STATUS ipv6InterfaceEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ipv6InterfaceIfIndex:
      Status = VB.SetInt32 (fpBridge->Get_ipv6InterfaceIfIndex ());
      break;

    case kLEAF_ipv6InterfaceReasmMaxSize:
      Status = VB.SetUint32 (fpBridge->Get_ipv6InterfaceReasmMaxSize (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipv6InterfaceIdentifier:
    {
      BcmString String;

      fpBridge->Get_ipv6InterfaceIdentifier (String);
      Status = VB.SetString (String);
      break;
    }

    case kLEAF_ipv6InterfaceEnableStatus:
      Status = VB.SetInt32 (fpBridge->Get_ipv6InterfaceEnableStatus ());
      break;

    case kLEAF_ipv6InterfaceReachableTime:
      Status = VB.SetUint32 (fpBridge->Get_ipv6InterfaceReachableTime (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipv6InterfaceRetransmitTime:
      Status = VB.SetUint32 (fpBridge->Get_ipv6InterfaceRetransmitTime (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipv6InterfaceForwarding:
      Status = VB.SetInt32 (fpBridge->Get_ipv6InterfaceForwarding ());
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

SNMP_STATUS ipv6InterfaceEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ipv6InterfaceIfIndex:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_ipv6InterfaceReasmMaxSize:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipv6InterfaceIdentifier:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipv6InterfaceEnableStatus:
    {
      int ipv6InterfaceEnableStatus;

      Status = VB.GetInt32 (&ipv6InterfaceEnableStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipv6InterfaceEnableStatus (ipv6InterfaceEnableStatus);
      break;
    }

    case kLEAF_ipv6InterfaceReachableTime:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipv6InterfaceRetransmitTime:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipv6InterfaceForwarding:
    {
      int ipv6InterfaceForwarding;

      Status = VB.GetInt32 (&ipv6InterfaceForwarding);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipv6InterfaceForwarding (ipv6InterfaceForwarding);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}

#endif

#if 0

/*\ 
 *  ipAddressPrefixTable and related objects 
\*/

/* ipAddressPrefixTable */

ipAddressPrefixTable::ipAddressPrefixTable ()
  : BcmMibTable (kOID_ipAddressPrefixTable, kNAME_ipAddressPrefixTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ipAddressPrefixEntry::kLEAF_ipAddressPrefixOrigin);
  fColumnMask |= (1 << ipAddressPrefixEntry::kLEAF_ipAddressPrefixOnLinkFlag);
  fColumnMask |= (1 << ipAddressPrefixEntry::kLEAF_ipAddressPrefixAutonomousFlag);
  fColumnMask |= (1 << ipAddressPrefixEntry::kLEAF_ipAddressPrefixAdvPreferredLifetime);
  fColumnMask |= (1 << ipAddressPrefixEntry::kLEAF_ipAddressPrefixAdvValidLifetime);
}

SNMP_STATUS ipAddressPrefixTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ipAddressPrefixEntry *pEntry;
  ipAddressPrefixEntryBridge *pMyBridge = (ipAddressPrefixEntryBridge *) pBridge;


  pEntry = new ipAddressPrefixEntry (this, pMyBridge);
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

/* ipAddressPrefixEntry */

ipAddressPrefixEntry::ipAddressPrefixEntry (ipAddressPrefixTable *pTable, ipAddressPrefixEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ipAddressPrefixEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_ipAddressPrefixIfIndex, kNAME_ipAddressPrefixIfIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipAddressPrefixType, kNAME_ipAddressPrefixType, this, kNotAccessible);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_ipAddressPrefixPrefix, kNAME_ipAddressPrefixPrefix, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipAddressPrefixLength, kNAME_ipAddressPrefixLength, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipAddressPrefixOrigin, kNAME_ipAddressPrefixOrigin, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipAddressPrefixOnLinkFlag, kNAME_ipAddressPrefixOnLinkFlag, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipAddressPrefixAutonomousFlag, kNAME_ipAddressPrefixAutonomousFlag, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipAddressPrefixAdvPreferredLifetime, kNAME_ipAddressPrefixAdvPreferredLifetime, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipAddressPrefixAdvValidLifetime, kNAME_ipAddressPrefixAdvValidLifetime, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ipAddressPrefixEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ipAddressPrefixIfIndex:
      Status = VB.SetInt32 (fpBridge->Get_ipAddressPrefixIfIndex ());
      break;

    case kLEAF_ipAddressPrefixType:
      Status = VB.SetInt32 (fpBridge->Get_ipAddressPrefixType ());
      break;

    case kLEAF_ipAddressPrefixPrefix:
    {
      BcmString String;

      fpBridge->Get_ipAddressPrefixPrefix (String);
      Status = VB.SetString (String);
      break;
    }

    case kLEAF_ipAddressPrefixLength:
      Status = VB.SetInt32 (fpBridge->Get_ipAddressPrefixLength ());
      break;

    case kLEAF_ipAddressPrefixOrigin:
      Status = VB.SetInt32 (fpBridge->Get_ipAddressPrefixOrigin ());
      break;

    case kLEAF_ipAddressPrefixOnLinkFlag:
      Status = VB.SetBool (fpBridge->Get_ipAddressPrefixOnLinkFlag ());
      break;

    case kLEAF_ipAddressPrefixAutonomousFlag:
      Status = VB.SetBool (fpBridge->Get_ipAddressPrefixAutonomousFlag ());
      break;

    case kLEAF_ipAddressPrefixAdvPreferredLifetime:
      Status = VB.SetUint32 (fpBridge->Get_ipAddressPrefixAdvPreferredLifetime (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipAddressPrefixAdvValidLifetime:
      Status = VB.SetUint32 (fpBridge->Get_ipAddressPrefixAdvValidLifetime (), VBT_UNSIGNED32);
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

#endif


/*\ 
 *  ipAddressTable and related objects 
\*/

/* ipAddressTable */

#if (BCM_IPV6_SUPPORT)
ipAddressTable::ipAddressTable (ipv4InterfaceTable *pIpv4InterfaceTable, ipv6InterfaceTable *pIpv6InterfaceTable)
#else
ipAddressTable::ipAddressTable (ipv4InterfaceTable *pIpv4InterfaceTable)
#endif
  : BcmMibTable (kOID_ipAddressTable, kNAME_ipAddressTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ipAddressEntry::kLEAF_ipAddressIfIndex);
  fColumnMask |= (1 << ipAddressEntry::kLEAF_ipAddressType);
  fColumnMask |= (1 << ipAddressEntry::kLEAF_ipAddressPrefix);
  fColumnMask |= (1 << ipAddressEntry::kLEAF_ipAddressOrigin);
  fColumnMask |= (1 << ipAddressEntry::kLEAF_ipAddressStatus);
  fColumnMask |= (1 << ipAddressEntry::kLEAF_ipAddressCreated);
  fColumnMask |= (1 << ipAddressEntry::kLEAF_ipAddressLastChanged);
  fColumnMask |= (1 << ipAddressEntry::kLEAF_ipAddressRowStatus);
  fColumnMask |= (1 << ipAddressEntry::kLEAF_ipAddressStorageType);
  
  fpIpv4InterfaceTable = pIpv4InterfaceTable;
  #if (BCM_IPV6_SUPPORT)
  fpIpv6InterfaceTable = pIpv6InterfaceTable;
  #endif
}

SNMP_STATUS ipAddressTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ipAddressEntry *pEntry;
  ipAddressEntryBridge *pMyBridge = (ipAddressEntryBridge *) pBridge;


  pEntry = new ipAddressEntry (this, pMyBridge);
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

#if !defined (TARGETOS_Linux)
void ipAddressTable::Update ()
{
  /* Update V4 and V6 interface tables */
  
  fpIpv4InterfaceTable->UpdateTable();
  #if (BCM_IPV6_SUPPORT)
  fpIpv6InterfaceTable->UpdateTable();
  #endif

  /*\
   *  Flush the table and rebuild.  This is not the most efficient way in the
   *  world to handle this update, since it flushes and rebuilds every time.
   *  However it's not as bad as it could be because of the base class 
   *  strategies which prevent an update from happening unless an update has 
   *  not previously taken place within the last 5 seconds.
   *
   *  The efficiency of this could be improved by using the DHCP lease 
   *  timestamp (see ipAddressCreated) to track the "newest" lease and only
   *  update if the number of leases changes or the newest lease is newer than
   *  the most recent update.
  \*/
  
  Flush();
  
  ipAddressEntryBridge *pBridge;   
  BcmIpAddress ipAddressAddr;
  int ipAddressIfIndex = 0;
  int ipAddressType = CONSTVAL_IPADDRESSTYPE_UNICAST;
  BcmObjectId ipAddressPrefix("0.0");
  int ipAddressOrigin = CONSTVAL_IPADDRESSORIGIN_DHCP;
  int ipAddressStatus = CONSTVAL_IPADDRESSSTATUS_PREFERRED;
  unsigned long ipAddressCreated = 0;
  unsigned long ipAddressLastChanged = 0;
  unsigned long ipAddressStorageType = CONSTVAL_IPADDRESSSTORAGETYPE_VOLATILE;
  
  // First of all, we will put in an entry for 127.0.0.1.
  // It is of 'manual' origin and was created at sysUpTime = 0.
  // Only if we have a SW loopback entry in our ifTable.
  ifEntryBridge *pIf = fpAgent->IfBridge().FirstIfEntryOfType (BcmHalIf::kIanaTypeSoftwareLoopback);
  if (pIf)
  {
  ipAddressAddr = kSoftwareLoopbackIp;
  
  ipAddressOrigin = CONSTVAL_IPADDRESSORIGIN_MANUAL;
  ipAddressStorageType = CONSTVAL_IPADDRESSSTORAGETYPE_NONVOLATILE;
  
    ipAddressIfIndex = pIf->Get_ifIndex();
    
  pBridge = new ipAddressEntryBridge (ipAddressAddr, ipAddressIfIndex, ipAddressType,
                                      ipAddressPrefix, ipAddressOrigin, ipAddressStatus,
                                      ipAddressCreated, ipAddressLastChanged, ipAddressStorageType,
                                      fpAgent);
  
  // Calling BcmMibTable::RegisterBridge here is much faster then 
  // calling BcmMibBridge::Register because it means we don't have
  // to search for the table.
  if (RegisterBridge(pBridge, true) != SNMP_NO_ERROR)
    delete pBridge;
  }
     
  /* Now, iterate over all IP stacks and put an entry in the table for all
     bound IP addresses on every stack */
     
  BcmBaseIpHalIf *pIpHal;
  
  for (int Stack = 1; Stack <= BCM_NUM_IP_STACKS; Stack++)
  {
    pIpHal = BcmIpStackManager::GetSingleton().Find(Stack);
    if (pIpHal == NULL)
      continue;

    /* Iterate over all IPv4 leases for this stack and create an entry for each. */
    
    ipAddressIfIndex = fpIpv4InterfaceTable->IpIfIndex(pIpHal);
    
    if (ipAddressIfIndex)
    {
      BcmDhcpClientIf *pDhcp = pIpHal->DhcpClientIf();
      if (pDhcp == NULL)
        continue;
      
      BcmDhcpClientLease *pLease = pDhcp->FirstLease();
      while (pLease)
      {
        // Get relevant information for this IP.  Some columns (ipAddressType,
        // ipAddressPrefix, ipAddressStatus, ipAddressStorageType) are always
        // the same for every IP.
        ipAddressAddr = pLease->ServerLeaseSettings().YourIpAddress();
        
        // If we get 0.0.0.0 as our IP address that means this lease is not
        // yet configured, so skip it.
        if (ipAddressAddr != kAllZerosIp)
        {
          if (pLease->StaticallyProvisioned() == true)
          {
            ipAddressOrigin = CONSTVAL_IPADDRESSORIGIN_MANUAL;
            ipAddressStorageType = CONSTVAL_IPADDRESSSTORAGETYPE_NONVOLATILE;
          }
          else
          {
            ipAddressOrigin = CONSTVAL_IPADDRESSORIGIN_DHCP;
            ipAddressStorageType = CONSTVAL_IPADDRESSSTORAGETYPE_VOLATILE;
          }
            
          ipAddressCreated = pLease->ClientLeaseSettings().CreationTime();
          ipAddressLastChanged = ipAddressCreated;
          
          pBridge = new ipAddressEntryBridge (ipAddressAddr, ipAddressIfIndex, ipAddressType,
                                              ipAddressPrefix, ipAddressOrigin, ipAddressStatus,
                                              ipAddressCreated, ipAddressLastChanged, ipAddressStorageType,
                                              fpAgent);
          
          // Calling BcmMibTable::RegisterBridge here is much faster then 
          // calling BcmMibBridge::Register because it means we don't have
          // to search for the table.
          if (RegisterBridge(pBridge, true) != SNMP_NO_ERROR)
            delete pBridge;
        }
      
        pLease = pDhcp->NextLease(pLease);
      }
    }
    
    #if (BCM_IPV6_SUPPORT)
    
    /* Next do the same with IPv6 addresses. */
    
    ipAddressIfIndex = fpIpv6InterfaceTable->IpIfIndex(pIpHal);
    
    if (ipAddressIfIndex)
    {
      // Link-local address (1 per stack at most) are 'linklayer'
      BcmIpV6Address LinkLocal;
      
      // This call will calculate the value for a link local address on this 
      // stack, but that doesn't necessarily mean it is actually bound...
      pIpHal->GetIpv6LinkLocalAddress(LinkLocal);
      
      // ...so only add to the table if it is bound.
      if (pIpHal->IsIpAddressBound(LinkLocal) == true)
      {
        ipAddressAddr = LinkLocal;
        
        // Use 'manual' for linkLocal.  DOCSIS explicitly forbids IPv6 stateless
        // autoconfig so even though we calculate our link-local address per
        // RFC-4862 we should report 'manual' here.
        ipAddressOrigin = CONSTVAL_IPADDRESSORIGIN_MANUAL; // CONSTVAL_IPADDRESSORIGIN_LINKLAYER;
        ipAddressStorageType = CONSTVAL_IPADDRESSSTORAGETYPE_NONVOLATILE;
        
        // I guess we can consider these created at time 0...
        ipAddressCreated = 0;
        ipAddressLastChanged = 0;
        
        pBridge = new ipAddressEntryBridge (ipAddressAddr, ipAddressIfIndex, ipAddressType,
                                            ipAddressPrefix, ipAddressOrigin, ipAddressStatus,
                                            ipAddressCreated, ipAddressLastChanged, ipAddressStorageType,
                                            fpAgent);
        // Calling BcmMibTable::RegisterBridge here is much faster then 
        // calling BcmMibBridge::Register because it means we don't have
        // to search for the table.
        if (RegisterBridge(pBridge, true) != SNMP_NO_ERROR)
          delete pBridge;
      }
      
      // Now use the DHCPv6 client to iterate over all IPv6 addresses and add those.
      BcmDhcpV6ClientIf *pDhcp6 = pIpHal->DhcpV6ClientIf();
      if (pDhcp6 == NULL)
        continue;
      
      // The DHCPv6 equivalent of a lease is called an "Identity Association"
      // address
      for (unsigned int Loop = 0; Loop < pDhcp6->IA().NumberOfIaAddresses(); Loop++)
      {
        BcmDhcpV6IaAddress IaAddress;
        pDhcp6->IA().Get(Loop, IaAddress);
        
        ipAddressAddr = IaAddress.IpV6Address();
        
        ipAddressOrigin = CONSTVAL_IPADDRESSORIGIN_DHCP;
        ipAddressStorageType = CONSTVAL_IPADDRESSSTORAGETYPE_VOLATILE;
          
        ipAddressCreated = IaAddress.CreationTime();
        ipAddressLastChanged = ipAddressCreated;
        
        pBridge = new ipAddressEntryBridge (ipAddressAddr, ipAddressIfIndex, ipAddressType,
                                            ipAddressPrefix, ipAddressOrigin, ipAddressStatus,
                                            ipAddressCreated, ipAddressLastChanged, ipAddressStorageType,
                                            fpAgent);
        // Calling BcmMibTable::RegisterBridge here is much faster then 
        // calling BcmMibBridge::Register because it means we don't have
        // to search for the table.
        if (RegisterBridge(pBridge, true) != SNMP_NO_ERROR)
          delete pBridge;
      }
    }
    
    #endif
  }
}
#endif

/* ipAddressEntry */

ipAddressEntry::ipAddressEntry (ipAddressTable *pTable, ipAddressEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ipAddressEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_ipAddressAddrType, kNAME_ipAddressAddrType, this, kNotAccessible);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_ipAddressAddr, kNAME_ipAddressAddr, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipAddressIfIndex, kNAME_ipAddressIfIndex, this, kReadCreate);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipAddressType, kNAME_ipAddressType, this, kReadCreate);
  AddObject (pObject);

  pObject = new OidMibObject (kOID_ipAddressPrefix, kNAME_ipAddressPrefix, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipAddressOrigin, kNAME_ipAddressOrigin, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipAddressStatus, kNAME_ipAddressStatus, this, kReadCreate);
  AddObject (pObject);

  pObject = new TimeticksMibObject (kOID_ipAddressCreated, kNAME_ipAddressCreated, this, kReadOnly);
  AddObject (pObject);

  pObject = new TimeticksMibObject (kOID_ipAddressLastChanged, kNAME_ipAddressLastChanged, this, kReadOnly);
  AddObject (pObject);

  pObject = new RowStatusMibObject (pTable, this, kOID_ipAddressRowStatus, kNAME_ipAddressRowStatus, kReadCreate);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipAddressStorageType, kNAME_ipAddressStorageType, this, kReadCreate);
  AddObject (pObject);
}

SNMP_STATUS ipAddressEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmObjectId Oid;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ipAddressAddrType:
      Status = VB.SetInt32 (fpBridge->Get_ipAddressAddrType ());
      break;

    case kLEAF_ipAddressAddr:
    {
      BcmString String;

      fpBridge->Get_ipAddressAddr (String);
      Status = VB.SetString (String);
      break;
    }

    case kLEAF_ipAddressIfIndex:
      Status = VB.SetInt32 (fpBridge->Get_ipAddressIfIndex ());
      break;

    case kLEAF_ipAddressType:
      Status = VB.SetInt32 (fpBridge->Get_ipAddressType ());
      break;

    case kLEAF_ipAddressPrefix:
      fpBridge->Get_ipAddressPrefix (Oid);
      Status = VB.SetOid (Oid);
      break;

    case kLEAF_ipAddressOrigin:
      Status = VB.SetInt32 (fpBridge->Get_ipAddressOrigin ());
      break;

    case kLEAF_ipAddressStatus:
      Status = VB.SetInt32 (fpBridge->Get_ipAddressStatus ());
      break;

    case kLEAF_ipAddressCreated:
      Status = VB.SetTimeticks (fpBridge->Get_ipAddressCreated ());
      break;

    case kLEAF_ipAddressLastChanged:
      Status = VB.SetTimeticks (fpBridge->Get_ipAddressLastChanged ());
      break;

    case kLEAF_ipAddressRowStatus:
      Status = VB.SetInt32 (fpBridge->Get_ipAddressRowStatus ());
      break;

    case kLEAF_ipAddressStorageType:
      Status = VB.SetInt32 (fpBridge->Get_ipAddressStorageType ());
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

SNMP_STATUS ipAddressEntry::TestLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmObjectId Oid;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ipAddressAddrType:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_ipAddressAddr:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_ipAddressIfIndex:
    {
      int ipAddressIfIndex;

      Status = VB.GetInt32 (&ipAddressIfIndex);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_ipAddressIfIndex (ipAddressIfIndex);
      break;
    }

    case kLEAF_ipAddressOrigin:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipAddressCreated:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipAddressLastChanged:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipAddressRowStatus:
    {
      int ipAddressRowStatus;

      Status = VB.GetInt32 (&ipAddressRowStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_ipAddressRowStatus (ipAddressRowStatus);
      break;
    }

  }

  return Status;
}

SNMP_STATUS ipAddressEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmObjectId Oid;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  // If this row has not yet been created, either reject the set or create the
  // row; either is allowed per RFC-2579.  See note in MibObjs.h for details.
  if ((fpBridge->Get_ipAddressRowStatus() == SNMP_STATUS_INVISIBLE)
  &&  ((fpBridge->fObjectMask & ipAddressEntryBridge::kBIT_ipAddressRowStatus) == 0))
  {
#ifdef SUPPORT_INDIRECT_ROW_CREATION
    int CreateStatus = SNMP_STATUS_CREATE_AND_WAIT;

    SnmpLogReq << "Indirect row creation supported, creating ipAddressEntry" << endl;
    Status = fpBridge->Set_ipAddressRowStatus (CreateStatus);
    if (Status != SNMP_NO_ERROR)
      return Status;
#else
    SnmpLogReq << "ipAddressRowStatus not set (" << (void *)fpBridge->fObjectMask << "), INCONSISTENT_VALUE" << endl;
    return SNMP_INCONSISTENT_VALUE | SNMP_ROW_NOT_CREATED;
#endif
  }

  switch (Leaf.Leaf())
  {
    case kLEAF_ipAddressAddrType:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_ipAddressAddr:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_ipAddressIfIndex:
    {
      int ipAddressIfIndex;

      Status = VB.GetInt32 (&ipAddressIfIndex);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipAddressIfIndex (ipAddressIfIndex);
      break;
    }

    case kLEAF_ipAddressType:
    {
      int ipAddressType;

      Status = VB.GetInt32 (&ipAddressType);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipAddressType (ipAddressType);
      break;
    }

    case kLEAF_ipAddressPrefix:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipAddressOrigin:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipAddressStatus:
    {
      int ipAddressStatus;

      Status = VB.GetInt32 (&ipAddressStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipAddressStatus (ipAddressStatus);
      break;
    }

    case kLEAF_ipAddressCreated:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipAddressLastChanged:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipAddressRowStatus:
    {
      int ipAddressRowStatus;

      Status = VB.GetInt32 (&ipAddressRowStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipAddressRowStatus (ipAddressRowStatus);
      break;
    }

    case kLEAF_ipAddressStorageType:
    {
      int ipAddressStorageType;

      Status = VB.GetInt32 (&ipAddressStorageType);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipAddressStorageType (ipAddressStorageType);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}


/*\ 
 *  ipNetToPhysicalTable and related objects 
\*/

/* ipNetToPhysicalTable */

ipNetToPhysicalTable::ipNetToPhysicalTable ()
  : BcmMibTable (kOID_ipNetToPhysicalTable, kNAME_ipNetToPhysicalTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ipNetToPhysicalEntry::kLEAF_ipNetToPhysicalPhysAddress);
  fColumnMask |= (1 << ipNetToPhysicalEntry::kLEAF_ipNetToPhysicalLastUpdated);
  fColumnMask |= (1 << ipNetToPhysicalEntry::kLEAF_ipNetToPhysicalType);
  fColumnMask |= (1 << ipNetToPhysicalEntry::kLEAF_ipNetToPhysicalState);
  fColumnMask |= (1 << ipNetToPhysicalEntry::kLEAF_ipNetToPhysicalRowStatus);
}

SNMP_STATUS ipNetToPhysicalTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ipNetToPhysicalEntry *pEntry;
  ipNetToPhysicalEntryBridge *pMyBridge = (ipNetToPhysicalEntryBridge *) pBridge;


  pEntry = new ipNetToPhysicalEntry (this, pMyBridge);
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

/* ipNetToPhysicalEntry */

ipNetToPhysicalEntry::ipNetToPhysicalEntry (ipNetToPhysicalTable *pTable, ipNetToPhysicalEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ipNetToPhysicalEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_ipNetToPhysicalIfIndex, kNAME_ipNetToPhysicalIfIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipNetToPhysicalNetAddressType, kNAME_ipNetToPhysicalNetAddressType, this, kNotAccessible);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_ipNetToPhysicalNetAddress, kNAME_ipNetToPhysicalNetAddress, this, kNotAccessible);
  AddObject (pObject);

  pObject = new MacAddrMibObject (kOID_ipNetToPhysicalPhysAddress, kNAME_ipNetToPhysicalPhysAddress, this, kReadCreate);
  AddObject (pObject);

  pObject = new TimeticksMibObject (kOID_ipNetToPhysicalLastUpdated, kNAME_ipNetToPhysicalLastUpdated, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipNetToPhysicalType, kNAME_ipNetToPhysicalType, this, kReadCreate);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipNetToPhysicalState, kNAME_ipNetToPhysicalState, this, kReadOnly);
  AddObject (pObject);

  pObject = new RowStatusMibObject (pTable, this, kOID_ipNetToPhysicalRowStatus, kNAME_ipNetToPhysicalRowStatus, kReadCreate);
  AddObject (pObject);
}

SNMP_STATUS ipNetToPhysicalEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmMacAddress Mac;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ipNetToPhysicalIfIndex:
      Status = VB.SetInt32 (fpBridge->Get_ipNetToPhysicalIfIndex ());
      break;

    case kLEAF_ipNetToPhysicalNetAddressType:
      Status = VB.SetInt32 (fpBridge->Get_ipNetToPhysicalNetAddressType ());
      break;

    case kLEAF_ipNetToPhysicalNetAddress:
    {
      BcmString String;

      fpBridge->Get_ipNetToPhysicalNetAddress (String);
      Status = VB.SetString (String);
      break;
    }

    case kLEAF_ipNetToPhysicalPhysAddress:
      fpBridge->Get_ipNetToPhysicalPhysAddress (Mac);
      Status = VB.SetMacAddr (Mac);
      break;

    case kLEAF_ipNetToPhysicalLastUpdated:
      Status = VB.SetTimeticks (fpBridge->Get_ipNetToPhysicalLastUpdated ());
      break;

    case kLEAF_ipNetToPhysicalType:
      Status = VB.SetInt32 (fpBridge->Get_ipNetToPhysicalType ());
      break;

    case kLEAF_ipNetToPhysicalState:
      Status = VB.SetInt32 (fpBridge->Get_ipNetToPhysicalState ());
      break;

    case kLEAF_ipNetToPhysicalRowStatus:
      Status = VB.SetInt32 (fpBridge->Get_ipNetToPhysicalRowStatus ());
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

SNMP_STATUS ipNetToPhysicalEntry::TestLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmMacAddress Mac;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ipNetToPhysicalIfIndex:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_ipNetToPhysicalNetAddressType:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_ipNetToPhysicalNetAddress:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_ipNetToPhysicalPhysAddress:
      Status = VB.GetMacAddr (Mac);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_ipNetToPhysicalPhysAddress (Mac);
      break;

    case kLEAF_ipNetToPhysicalLastUpdated:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipNetToPhysicalState:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipNetToPhysicalRowStatus:
    {
      int ipNetToPhysicalRowStatus;

      Status = VB.GetInt32 (&ipNetToPhysicalRowStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_ipNetToPhysicalRowStatus (ipNetToPhysicalRowStatus);
      break;
    }

  }

  return Status;
}

SNMP_STATUS ipNetToPhysicalEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmMacAddress Mac;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  // If this row has not yet been created, either reject the set or create the
  // row; either is allowed per RFC-2579.  See note in MibObjs.h for details.
  if ((fpBridge->Get_ipNetToPhysicalRowStatus() == SNMP_STATUS_INVISIBLE)
  &&  ((fpBridge->fObjectMask & ipNetToPhysicalEntryBridge::kBIT_ipNetToPhysicalRowStatus) == 0))
  {
#ifdef SUPPORT_INDIRECT_ROW_CREATION
    int CreateStatus = SNMP_STATUS_CREATE_AND_WAIT;

    SnmpLogReq << "Indirect row creation supported, creating ipNetToPhysicalEntry" << endl;
    Status = fpBridge->Set_ipNetToPhysicalRowStatus (CreateStatus);
    if (Status != SNMP_NO_ERROR)
      return Status;
#else
    SnmpLogReq << "ipNetToPhysicalRowStatus not set (" << (void *)fpBridge->fObjectMask << "), INCONSISTENT_VALUE" << endl;
    return SNMP_INCONSISTENT_VALUE | SNMP_ROW_NOT_CREATED;
#endif
  }

  switch (Leaf.Leaf())
  {
    case kLEAF_ipNetToPhysicalIfIndex:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_ipNetToPhysicalNetAddressType:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_ipNetToPhysicalNetAddress:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_ipNetToPhysicalPhysAddress:
      Status = VB.GetMacAddr (Mac);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipNetToPhysicalPhysAddress (Mac);
      break;

    case kLEAF_ipNetToPhysicalLastUpdated:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipNetToPhysicalType:
    {
      int ipNetToPhysicalType;

      Status = VB.GetInt32 (&ipNetToPhysicalType);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipNetToPhysicalType (ipNetToPhysicalType);
      break;
    }

    case kLEAF_ipNetToPhysicalState:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_ipNetToPhysicalRowStatus:
    {
      int ipNetToPhysicalRowStatus;

      Status = VB.GetInt32 (&ipNetToPhysicalRowStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipNetToPhysicalRowStatus (ipNetToPhysicalRowStatus);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}

#if 0

/*\ 
 *  ipv6ScopeZoneIndexTable and related objects 
\*/

/* ipv6ScopeZoneIndexTable */

ipv6ScopeZoneIndexTable::ipv6ScopeZoneIndexTable ()
  : BcmMibTable (kOID_ipv6ScopeZoneIndexTable, kNAME_ipv6ScopeZoneIndexTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ipv6ScopeZoneIndexEntry::kLEAF_ipv6ScopeZoneIndexLinkLocal);
  fColumnMask |= (1 << ipv6ScopeZoneIndexEntry::kLEAF_ipv6ScopeZoneIndex3);
  fColumnMask |= (1 << ipv6ScopeZoneIndexEntry::kLEAF_ipv6ScopeZoneIndexAdminLocal);
  fColumnMask |= (1 << ipv6ScopeZoneIndexEntry::kLEAF_ipv6ScopeZoneIndexSiteLocal);
  fColumnMask |= (1 << ipv6ScopeZoneIndexEntry::kLEAF_ipv6ScopeZoneIndex6);
  fColumnMask |= (1 << ipv6ScopeZoneIndexEntry::kLEAF_ipv6ScopeZoneIndex7);
  fColumnMask |= (1 << ipv6ScopeZoneIndexEntry::kLEAF_ipv6ScopeZoneIndexOrganizationLocal);
  fColumnMask |= (1 << ipv6ScopeZoneIndexEntry::kLEAF_ipv6ScopeZoneIndex9);
  fColumnMask |= (1 << ipv6ScopeZoneIndexEntry::kLEAF_ipv6ScopeZoneIndexA);
  fColumnMask |= (1 << ipv6ScopeZoneIndexEntry::kLEAF_ipv6ScopeZoneIndexB);
  fColumnMask |= (1 << ipv6ScopeZoneIndexEntry::kLEAF_ipv6ScopeZoneIndexC);
  fColumnMask |= (1 << ipv6ScopeZoneIndexEntry::kLEAF_ipv6ScopeZoneIndexD);
}

SNMP_STATUS ipv6ScopeZoneIndexTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ipv6ScopeZoneIndexEntry *pEntry;
  ipv6ScopeZoneIndexEntryBridge *pMyBridge = (ipv6ScopeZoneIndexEntryBridge *) pBridge;


  pEntry = new ipv6ScopeZoneIndexEntry (this, pMyBridge);
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

/* ipv6ScopeZoneIndexEntry */

ipv6ScopeZoneIndexEntry::ipv6ScopeZoneIndexEntry (ipv6ScopeZoneIndexTable *pTable, ipv6ScopeZoneIndexEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ipv6ScopeZoneIndexEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_ipv6ScopeZoneIndexIfIndex, kNAME_ipv6ScopeZoneIndexIfIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv6ScopeZoneIndexLinkLocal, kNAME_ipv6ScopeZoneIndexLinkLocal, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv6ScopeZoneIndex3, kNAME_ipv6ScopeZoneIndex3, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv6ScopeZoneIndexAdminLocal, kNAME_ipv6ScopeZoneIndexAdminLocal, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv6ScopeZoneIndexSiteLocal, kNAME_ipv6ScopeZoneIndexSiteLocal, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv6ScopeZoneIndex6, kNAME_ipv6ScopeZoneIndex6, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv6ScopeZoneIndex7, kNAME_ipv6ScopeZoneIndex7, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv6ScopeZoneIndexOrganizationLocal, kNAME_ipv6ScopeZoneIndexOrganizationLocal, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv6ScopeZoneIndex9, kNAME_ipv6ScopeZoneIndex9, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv6ScopeZoneIndexA, kNAME_ipv6ScopeZoneIndexA, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv6ScopeZoneIndexB, kNAME_ipv6ScopeZoneIndexB, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv6ScopeZoneIndexC, kNAME_ipv6ScopeZoneIndexC, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv6ScopeZoneIndexD, kNAME_ipv6ScopeZoneIndexD, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ipv6ScopeZoneIndexEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ipv6ScopeZoneIndexIfIndex:
      Status = VB.SetInt32 (fpBridge->Get_ipv6ScopeZoneIndexIfIndex ());
      break;

    case kLEAF_ipv6ScopeZoneIndexLinkLocal:
      Status = VB.SetUint32 (fpBridge->Get_ipv6ScopeZoneIndexLinkLocal (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipv6ScopeZoneIndex3:
      Status = VB.SetUint32 (fpBridge->Get_ipv6ScopeZoneIndex3 (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipv6ScopeZoneIndexAdminLocal:
      Status = VB.SetUint32 (fpBridge->Get_ipv6ScopeZoneIndexAdminLocal (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipv6ScopeZoneIndexSiteLocal:
      Status = VB.SetUint32 (fpBridge->Get_ipv6ScopeZoneIndexSiteLocal (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipv6ScopeZoneIndex6:
      Status = VB.SetUint32 (fpBridge->Get_ipv6ScopeZoneIndex6 (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipv6ScopeZoneIndex7:
      Status = VB.SetUint32 (fpBridge->Get_ipv6ScopeZoneIndex7 (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipv6ScopeZoneIndexOrganizationLocal:
      Status = VB.SetUint32 (fpBridge->Get_ipv6ScopeZoneIndexOrganizationLocal (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipv6ScopeZoneIndex9:
      Status = VB.SetUint32 (fpBridge->Get_ipv6ScopeZoneIndex9 (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipv6ScopeZoneIndexA:
      Status = VB.SetUint32 (fpBridge->Get_ipv6ScopeZoneIndexA (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipv6ScopeZoneIndexB:
      Status = VB.SetUint32 (fpBridge->Get_ipv6ScopeZoneIndexB (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipv6ScopeZoneIndexC:
      Status = VB.SetUint32 (fpBridge->Get_ipv6ScopeZoneIndexC (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipv6ScopeZoneIndexD:
      Status = VB.SetUint32 (fpBridge->Get_ipv6ScopeZoneIndexD (), VBT_UNSIGNED32);
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

#endif

/*\ 
 *  ipDefaultRouterTable and related objects 
\*/

/* ipDefaultRouterTable */

ipDefaultRouterTable::ipDefaultRouterTable ()
  : BcmMibTable (kOID_ipDefaultRouterTable, kNAME_ipDefaultRouterTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ipDefaultRouterEntry::kLEAF_ipDefaultRouterLifetime);
  fColumnMask |= (1 << ipDefaultRouterEntry::kLEAF_ipDefaultRouterPreference);
}

SNMP_STATUS ipDefaultRouterTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ipDefaultRouterEntry *pEntry;
  ipDefaultRouterEntryBridge *pMyBridge = (ipDefaultRouterEntryBridge *) pBridge;


  pEntry = new ipDefaultRouterEntry (this, pMyBridge);
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

/* ipDefaultRouterEntry */

ipDefaultRouterEntry::ipDefaultRouterEntry (ipDefaultRouterTable *pTable, ipDefaultRouterEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ipDefaultRouterEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_ipDefaultRouterAddressType, kNAME_ipDefaultRouterAddressType, this, kNotAccessible);
  AddObject (pObject);

  pObject = new StringMibObject (kOID_ipDefaultRouterAddress, kNAME_ipDefaultRouterAddress, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipDefaultRouterIfIndex, kNAME_ipDefaultRouterIfIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipDefaultRouterLifetime, kNAME_ipDefaultRouterLifetime, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipDefaultRouterPreference, kNAME_ipDefaultRouterPreference, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ipDefaultRouterEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ipDefaultRouterAddressType:
      Status = VB.SetInt32 (fpBridge->Get_ipDefaultRouterAddressType ());
      break;

    case kLEAF_ipDefaultRouterAddress:
    {
      BcmString String;

      fpBridge->Get_ipDefaultRouterAddress (String);
      Status = VB.SetString (String);
      break;
    }

    case kLEAF_ipDefaultRouterIfIndex:
      Status = VB.SetInt32 (fpBridge->Get_ipDefaultRouterIfIndex ());
      break;

    case kLEAF_ipDefaultRouterLifetime:
      Status = VB.SetUint32 (fpBridge->Get_ipDefaultRouterLifetime (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipDefaultRouterPreference:
      Status = VB.SetInt32 (fpBridge->Get_ipDefaultRouterPreference ());
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

#if 0

/*\ 
 *  ipv6RouterAdvertTable and related objects 
\*/

/* ipv6RouterAdvertTable */

ipv6RouterAdvertTable::ipv6RouterAdvertTable ()
  : BcmMibTable (kOID_ipv6RouterAdvertTable, kNAME_ipv6RouterAdvertTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertSendAdverts);
  fColumnMask |= (1 << ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertMaxInterval);
  fColumnMask |= (1 << ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertMinInterval);
  fColumnMask |= (1 << ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertManagedFlag);
  fColumnMask |= (1 << ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertOtherConfigFlag);
  fColumnMask |= (1 << ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertLinkMTU);
  fColumnMask |= (1 << ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertReachableTime);
  fColumnMask |= (1 << ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertRetransmitTime);
  fColumnMask |= (1 << ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertCurHopLimit);
  fColumnMask |= (1 << ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertDefaultLifetime);
  fColumnMask |= (1 << ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertRowStatus);
}

SNMP_STATUS ipv6RouterAdvertTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ipv6RouterAdvertEntry *pEntry;
  ipv6RouterAdvertEntryBridge *pMyBridge = (ipv6RouterAdvertEntryBridge *) pBridge;


  pEntry = new ipv6RouterAdvertEntry (this, pMyBridge);
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

BcmMibEntry *ipv6RouterAdvertTable::NewDefaultEntry (BcmObjectId &Index)
{
  if (fpAgent == NULL)
    return NULL;

  /* Verify index length. */

  // ipv6RouterAdvertIfIndex (numerical type): 1 element
  if (Index.Length() != 1)
    return NULL;

  /* Verify index value(s) */

  // ipv6RouterAdvertIfIndex (numerical type)
  if ((Index[0] < MINVAL_IPV6ROUTERADVERTIFINDEX)
  ||  (Index[0] > MAXVAL_IPV6ROUTERADVERTIFINDEX))
    return NULL;

  SnmpLogRaw << "UNFINISHED: double-check index values in ipv6RouterAdvertTable::NewDefaultEntry" << endl;

  ipv6RouterAdvertEntryBridge *pBridge = new ipv6RouterAdvertEntryBridge (Index, fpAgent);

  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
  {
    SnmpLogInit << "ipv6RouterAdvertTable::NewDefaultEntry Bridge registration failed!  Deleting bridge object." << endl;
    delete (pBridge);
    return NULL;
  }
  
  return (BcmMibEntry *)pBridge->fpObject;
}

/* ipv6RouterAdvertEntry */

ipv6RouterAdvertEntry::ipv6RouterAdvertEntry (ipv6RouterAdvertTable *pTable, ipv6RouterAdvertEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ipv6RouterAdvertEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_ipv6RouterAdvertIfIndex, kNAME_ipv6RouterAdvertIfIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipv6RouterAdvertSendAdverts, kNAME_ipv6RouterAdvertSendAdverts, this, kReadCreate);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv6RouterAdvertMaxInterval, kNAME_ipv6RouterAdvertMaxInterval, this, kReadCreate);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv6RouterAdvertMinInterval, kNAME_ipv6RouterAdvertMinInterval, this, kReadCreate);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipv6RouterAdvertManagedFlag, kNAME_ipv6RouterAdvertManagedFlag, this, kReadCreate);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipv6RouterAdvertOtherConfigFlag, kNAME_ipv6RouterAdvertOtherConfigFlag, this, kReadCreate);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv6RouterAdvertLinkMTU, kNAME_ipv6RouterAdvertLinkMTU, this, kReadCreate);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv6RouterAdvertReachableTime, kNAME_ipv6RouterAdvertReachableTime, this, kReadCreate);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv6RouterAdvertRetransmitTime, kNAME_ipv6RouterAdvertRetransmitTime, this, kReadCreate);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv6RouterAdvertCurHopLimit, kNAME_ipv6RouterAdvertCurHopLimit, this, kReadCreate);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipv6RouterAdvertDefaultLifetime, kNAME_ipv6RouterAdvertDefaultLifetime, this, kReadCreate);
  AddObject (pObject);

  pObject = new RowStatusMibObject (pTable, this, kOID_ipv6RouterAdvertRowStatus, kNAME_ipv6RouterAdvertRowStatus, kReadCreate);
  AddObject (pObject);
}

SNMP_STATUS ipv6RouterAdvertEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ipv6RouterAdvertIfIndex:
      Status = VB.SetInt32 (fpBridge->Get_ipv6RouterAdvertIfIndex ());
      break;

    case kLEAF_ipv6RouterAdvertSendAdverts:
      Status = VB.SetBool (fpBridge->Get_ipv6RouterAdvertSendAdverts ());
      break;

    case kLEAF_ipv6RouterAdvertMaxInterval:
      Status = VB.SetUint32 (fpBridge->Get_ipv6RouterAdvertMaxInterval (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipv6RouterAdvertMinInterval:
      Status = VB.SetUint32 (fpBridge->Get_ipv6RouterAdvertMinInterval (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipv6RouterAdvertManagedFlag:
      Status = VB.SetBool (fpBridge->Get_ipv6RouterAdvertManagedFlag ());
      break;

    case kLEAF_ipv6RouterAdvertOtherConfigFlag:
      Status = VB.SetBool (fpBridge->Get_ipv6RouterAdvertOtherConfigFlag ());
      break;

    case kLEAF_ipv6RouterAdvertLinkMTU:
      Status = VB.SetUint32 (fpBridge->Get_ipv6RouterAdvertLinkMTU (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipv6RouterAdvertReachableTime:
      Status = VB.SetUint32 (fpBridge->Get_ipv6RouterAdvertReachableTime (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipv6RouterAdvertRetransmitTime:
      Status = VB.SetUint32 (fpBridge->Get_ipv6RouterAdvertRetransmitTime (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipv6RouterAdvertCurHopLimit:
      Status = VB.SetUint32 (fpBridge->Get_ipv6RouterAdvertCurHopLimit (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipv6RouterAdvertDefaultLifetime:
      Status = VB.SetUint32 (fpBridge->Get_ipv6RouterAdvertDefaultLifetime (), VBT_UNSIGNED32);
      break;

    case kLEAF_ipv6RouterAdvertRowStatus:
      Status = VB.SetInt32 (fpBridge->Get_ipv6RouterAdvertRowStatus ());
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

SNMP_STATUS ipv6RouterAdvertEntry::TestLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ipv6RouterAdvertIfIndex:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_ipv6RouterAdvertMinInterval:
    {
      unsigned int ipv6RouterAdvertMinInterval;

      Status = VB.GetUint32 (&ipv6RouterAdvertMinInterval);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_ipv6RouterAdvertMinInterval (ipv6RouterAdvertMinInterval);
      break;
    }

    case kLEAF_ipv6RouterAdvertCurHopLimit:
    {
      unsigned int ipv6RouterAdvertCurHopLimit;

      Status = VB.GetUint32 (&ipv6RouterAdvertCurHopLimit);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_ipv6RouterAdvertCurHopLimit (ipv6RouterAdvertCurHopLimit);
      break;
    }

    case kLEAF_ipv6RouterAdvertDefaultLifetime:
    {
      unsigned int ipv6RouterAdvertDefaultLifetime;

      Status = VB.GetUint32 (&ipv6RouterAdvertDefaultLifetime);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_ipv6RouterAdvertDefaultLifetime (ipv6RouterAdvertDefaultLifetime);
      break;
    }

    case kLEAF_ipv6RouterAdvertRowStatus:
    {
      int ipv6RouterAdvertRowStatus;

      Status = VB.GetInt32 (&ipv6RouterAdvertRowStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_ipv6RouterAdvertRowStatus (ipv6RouterAdvertRowStatus);
      break;
    }

  }

  return Status;
}

SNMP_STATUS ipv6RouterAdvertEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  // If this row has not yet been created, either reject the set or create the
  // row; either is allowed per RFC-2579.  See note in MibObjs.h for details.
  if ((fpBridge->Get_ipv6RouterAdvertRowStatus() == SNMP_STATUS_INVISIBLE)
  &&  ((fpBridge->fObjectMask & ipv6RouterAdvertEntryBridge::kBIT_ipv6RouterAdvertRowStatus) == 0))
  {
#ifdef SUPPORT_INDIRECT_ROW_CREATION
    int CreateStatus = SNMP_STATUS_CREATE_AND_WAIT;

    SnmpLogReq << "Indirect row creation supported, creating ipv6RouterAdvertEntry" << endl;
    Status = fpBridge->Set_ipv6RouterAdvertRowStatus (CreateStatus);
    if (Status != SNMP_NO_ERROR)
      return Status;
#else
    SnmpLogReq << "ipv6RouterAdvertRowStatus not set (" << (void *)fpBridge->fObjectMask << "), INCONSISTENT_VALUE" << endl;
    return SNMP_INCONSISTENT_VALUE | SNMP_ROW_NOT_CREATED;
#endif
  }

  switch (Leaf.Leaf())
  {
    case kLEAF_ipv6RouterAdvertIfIndex:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_ipv6RouterAdvertSendAdverts:
    {
      bool ipv6RouterAdvertSendAdverts;

      Status = VB.GetBool (&ipv6RouterAdvertSendAdverts);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipv6RouterAdvertSendAdverts (ipv6RouterAdvertSendAdverts);
      break;
    }

    case kLEAF_ipv6RouterAdvertMaxInterval:
    {
      unsigned int ipv6RouterAdvertMaxInterval;

      Status = VB.GetUint32 (&ipv6RouterAdvertMaxInterval);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipv6RouterAdvertMaxInterval (ipv6RouterAdvertMaxInterval);
      break;
    }

    case kLEAF_ipv6RouterAdvertMinInterval:
    {
      unsigned int ipv6RouterAdvertMinInterval;

      Status = VB.GetUint32 (&ipv6RouterAdvertMinInterval);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipv6RouterAdvertMinInterval (ipv6RouterAdvertMinInterval);
      break;
    }

    case kLEAF_ipv6RouterAdvertManagedFlag:
    {
      bool ipv6RouterAdvertManagedFlag;

      Status = VB.GetBool (&ipv6RouterAdvertManagedFlag);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipv6RouterAdvertManagedFlag (ipv6RouterAdvertManagedFlag);
      break;
    }

    case kLEAF_ipv6RouterAdvertOtherConfigFlag:
    {
      bool ipv6RouterAdvertOtherConfigFlag;

      Status = VB.GetBool (&ipv6RouterAdvertOtherConfigFlag);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipv6RouterAdvertOtherConfigFlag (ipv6RouterAdvertOtherConfigFlag);
      break;
    }

    case kLEAF_ipv6RouterAdvertLinkMTU:
    {
      unsigned int ipv6RouterAdvertLinkMTU;

      Status = VB.GetUint32 (&ipv6RouterAdvertLinkMTU);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipv6RouterAdvertLinkMTU (ipv6RouterAdvertLinkMTU);
      break;
    }

    case kLEAF_ipv6RouterAdvertReachableTime:
    {
      unsigned int ipv6RouterAdvertReachableTime;

      Status = VB.GetUint32 (&ipv6RouterAdvertReachableTime);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipv6RouterAdvertReachableTime (ipv6RouterAdvertReachableTime);
      break;
    }

    case kLEAF_ipv6RouterAdvertRetransmitTime:
    {
      unsigned int ipv6RouterAdvertRetransmitTime;

      Status = VB.GetUint32 (&ipv6RouterAdvertRetransmitTime);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipv6RouterAdvertRetransmitTime (ipv6RouterAdvertRetransmitTime);
      break;
    }

    case kLEAF_ipv6RouterAdvertCurHopLimit:
    {
      unsigned int ipv6RouterAdvertCurHopLimit;

      Status = VB.GetUint32 (&ipv6RouterAdvertCurHopLimit);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipv6RouterAdvertCurHopLimit (ipv6RouterAdvertCurHopLimit);
      break;
    }

    case kLEAF_ipv6RouterAdvertDefaultLifetime:
    {
      unsigned int ipv6RouterAdvertDefaultLifetime;

      Status = VB.GetUint32 (&ipv6RouterAdvertDefaultLifetime);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipv6RouterAdvertDefaultLifetime (ipv6RouterAdvertDefaultLifetime);
      break;
    }

    case kLEAF_ipv6RouterAdvertRowStatus:
    {
      int ipv6RouterAdvertRowStatus;

      Status = VB.GetInt32 (&ipv6RouterAdvertRowStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipv6RouterAdvertRowStatus (ipv6RouterAdvertRowStatus);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}

#endif

/*\ 
 *  ipAddrTable and related objects 
\*/

/* ipAddrTable */

ipAddrTable::ipAddrTable (ipv4InterfaceTable *pIpv4InterfaceTable)
  : BcmMibTable (kOID_ipAddrTable, kNAME_ipAddrTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ipAddrEntry::kLEAF_ipAdEntAddr);
  fColumnMask |= (1 << ipAddrEntry::kLEAF_ipAdEntIfIndex);
  fColumnMask |= (1 << ipAddrEntry::kLEAF_ipAdEntNetMask);
  fColumnMask |= (1 << ipAddrEntry::kLEAF_ipAdEntBcastAddr);
  fColumnMask |= (1 << ipAddrEntry::kLEAF_ipAdEntReasmMaxSize);
  
  fpIpv4InterfaceTable = pIpv4InterfaceTable;
}

SNMP_STATUS ipAddrTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ipAddrEntry *pEntry;
  ipAddrEntryBridge *pMyBridge = (ipAddrEntryBridge *) pBridge;


  pEntry = new ipAddrEntry (this, pMyBridge);
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

void ipAddrTable::Update ()
{
#if !defined (TARGETOS_Linux)
  /* Update V4 interface table */

  fpIpv4InterfaceTable->UpdateTable();

  /*\
   *  Flush the table and rebuild.  This is not the most efficient way in the
   *  world to handle this update, since it flushes and rebuilds every time.
   *  However it's not as bad as it could be because of the base class 
   *  strategies which prevent an update from happening unless an update has 
   *  not previously taken place within the last 5 seconds.
   *
   *  The efficiency of this could be improved by using the DHCP lease 
   *  timestamp (see ipAddressCreated) to track the "newest" lease and only
   *  update if the number of leases changes or the newest lease is newer than
   *  the most recent update.
  \*/
  
  
  Flush();
  
  ipAddrEntryBridge *pBridge;   
  BcmIpAddress ipAdEntAddr;
  int ipAdEntIfIndex;
  BcmIpAddress ipAdEntNetMask;
  int ipAdEntBcastAddr = 1; // same for everyone
  int ipAdEntReasmMaxSize = 65535; // same for everyone
     
  // First of all, we will put in an entry for 127.0.0.1.  It will have the
  // following settings:
  ipAdEntAddr.Set (127,0,0,1);
  ipAdEntNetMask.Set(255,0,0,0);
  
  // ipAdEntIfIndex: ifIndex of softwareLoopback on my agent
  ipAdEntIfIndex = 0;
  ifEntryBridge *pIf = fpAgent->IfBridge().FirstIfEntryOfType (BcmHalIf::kIanaTypeSoftwareLoopback);
  if (pIf)
    ipAdEntIfIndex = pIf->Get_ifIndex();
    
  pBridge = new ipAddrEntryBridge (ipAdEntAddr, ipAdEntIfIndex, ipAdEntNetMask,
                                   ipAdEntBcastAddr, ipAdEntReasmMaxSize, fpAgent);
  
  // Calling BcmMibTable::RegisterBridge here is much faster then 
  // calling BcmMibBridge::Register because it means we don't have
  // to search for the table.
  if (RegisterBridge(pBridge, true) != SNMP_NO_ERROR)
    delete pBridge;
     
  /* Now, iterate over all IP stacks and put an entry in the table for all
     bound IP addresses on every stack */
     
  BcmBaseIpHalIf *pIpHal;
  vector <BcmIpAddress> BoundIps;
  
  for (int Stack = 1; Stack <= BCM_NUM_IP_STACKS; Stack++)
  {
    pIpHal = BcmIpStackManager::GetSingleton().Find(Stack);
    if (pIpHal == NULL)
      continue;
      
    ipAdEntIfIndex = fpIpv4InterfaceTable->IpIfIndex(pIpHal);
    
    if (ipAdEntIfIndex)
    {
      pIpHal->GetAllBoundIpAddresses(BoundIps);
      for (unsigned int Loop = 0; Loop < BoundIps.size(); Loop++)
      {
        // Only IPv4 addresses get entries in the ipAddrTable.
        if (BoundIps[Loop].IsIpV6Address() == false)
        {
          ipAdEntAddr = BoundIps[Loop];
          
          pIpHal->SubnetMask(ipAdEntAddr, ipAdEntNetMask);
          
          pBridge = new ipAddrEntryBridge (ipAdEntAddr, ipAdEntIfIndex, ipAdEntNetMask,
                                       ipAdEntBcastAddr, ipAdEntReasmMaxSize, fpAgent);
          
          // Calling BcmMibTable::RegisterBridge here is much faster then 
          // calling BcmMibBridge::Register because it means we don't have
          // to search for the table.
          if (RegisterBridge(pBridge, true) != SNMP_NO_ERROR)
            delete pBridge;
        }
      }
    }
  }
#endif
}

/* ipAddrEntry */

ipAddrEntry::ipAddrEntry (ipAddrTable *pTable, ipAddrEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ipAddrEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new IpAddrMibObject (kOID_ipAdEntAddr, kNAME_ipAdEntAddr, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipAdEntIfIndex, kNAME_ipAdEntIfIndex, this, kReadOnly);
  AddObject (pObject);

  pObject = new IpAddrMibObject (kOID_ipAdEntNetMask, kNAME_ipAdEntNetMask, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipAdEntBcastAddr, kNAME_ipAdEntBcastAddr, this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipAdEntReasmMaxSize, kNAME_ipAdEntReasmMaxSize, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ipAddrEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmIpAddress Ip;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ipAdEntAddr:
      fpBridge->Get_ipAdEntAddr (Ip);
      Status = VB.SetIp (Ip);
      break;

    case kLEAF_ipAdEntIfIndex:
      Status = VB.SetInt32 (fpBridge->Get_ipAdEntIfIndex ());
      break;

    case kLEAF_ipAdEntNetMask:
      fpBridge->Get_ipAdEntNetMask (Ip);
      Status = VB.SetIp (Ip);
      break;

    case kLEAF_ipAdEntBcastAddr:
      Status = VB.SetInt32 (fpBridge->Get_ipAdEntBcastAddr ());
      break;

    case kLEAF_ipAdEntReasmMaxSize:
      Status = VB.SetInt32 (fpBridge->Get_ipAdEntReasmMaxSize ());
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
 *  ipNetToMediaTable and related objects 
\*/

/* ipNetToMediaTable */

ipNetToMediaTable::ipNetToMediaTable ()
  : BcmMibTable (kOID_ipNetToMediaTable, kNAME_ipNetToMediaTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ipNetToMediaEntry::kLEAF_ipNetToMediaIfIndex);
  fColumnMask |= (1 << ipNetToMediaEntry::kLEAF_ipNetToMediaPhysAddress);
  fColumnMask |= (1 << ipNetToMediaEntry::kLEAF_ipNetToMediaNetAddress);
  fColumnMask |= (1 << ipNetToMediaEntry::kLEAF_ipNetToMediaType);
}

SNMP_STATUS ipNetToMediaTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ipNetToMediaEntry *pEntry;
  ipNetToMediaEntryBridge *pMyBridge = (ipNetToMediaEntryBridge *) pBridge;


  pEntry = new ipNetToMediaEntry (this, pMyBridge);
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

/* ipNetToMediaEntry */

ipNetToMediaEntry::ipNetToMediaEntry (ipNetToMediaTable *pTable, ipNetToMediaEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ipNetToMediaEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_ipNetToMediaIfIndex, kNAME_ipNetToMediaIfIndex, this, kReadCreate);
  AddObject (pObject);

  pObject = new MacAddrMibObject (kOID_ipNetToMediaPhysAddress, kNAME_ipNetToMediaPhysAddress, this, kReadCreate);
  AddObject (pObject);

  pObject = new IpAddrMibObject (kOID_ipNetToMediaNetAddress, kNAME_ipNetToMediaNetAddress, this, kReadCreate);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipNetToMediaType, kNAME_ipNetToMediaType, this, kReadCreate);
  AddObject (pObject);
}

#if (BCM_IPV6_SUPPORT)
bool ipNetToMediaEntry::IsAccessible (BcmMibRequest *pReq) const
{
  if (fpBridge != NULL)
  {
    BcmIpAddress Ip;
    fpBridge->Get_ipNetToMediaNetAddress(Ip);
    
    // IPv6 addresses shouldn't show up in this table.  However, such entries
    // will exist by virtue of how the ipNetToMediaTable relates to the
    // ipNetToPhysicalTable.
    if (Ip.IsIpV6Address() == true)
      return false;
  }
  
  return BcmMibEntry::IsAccessible (pReq);
}
#endif

SNMP_STATUS ipNetToMediaEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmIpAddress Ip;
  BcmMacAddress Mac;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ipNetToMediaIfIndex:
      Status = VB.SetInt32 (fpBridge->Get_ipNetToMediaIfIndex ());
      break;

    case kLEAF_ipNetToMediaPhysAddress:
      fpBridge->Get_ipNetToMediaPhysAddress (Mac);
      Status = VB.SetMacAddr (Mac);
      break;

    case kLEAF_ipNetToMediaNetAddress:
      fpBridge->Get_ipNetToMediaNetAddress (Ip);
      Status = VB.SetIp (Ip);
      break;

    case kLEAF_ipNetToMediaType:
      Status = VB.SetInt32 (fpBridge->Get_ipNetToMediaType ());
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

SNMP_STATUS ipNetToMediaEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmIpAddress Ip;
  BcmMacAddress Mac;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ipNetToMediaIfIndex:
    {
      int ipNetToMediaIfIndex;

      Status = VB.GetInt32 (&ipNetToMediaIfIndex);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipNetToMediaIfIndex (ipNetToMediaIfIndex);
      break;
    }

    case kLEAF_ipNetToMediaPhysAddress:
      Status = VB.GetMacAddr (Mac);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipNetToMediaPhysAddress (Mac);
      break;

    case kLEAF_ipNetToMediaNetAddress:
      Status = VB.GetIp (Ip);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipNetToMediaNetAddress (Ip);
      break;

    case kLEAF_ipNetToMediaType:
    {
      int ipNetToMediaType;

      Status = VB.GetInt32 (&ipNetToMediaType);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_ipNetToMediaType (ipNetToMediaType);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}

#if 0

/*\
 *  ipTrafficStatsGroup group
\*/

ipTrafficStatsGroup::ipTrafficStatsGroup ()
  : BcmMibGroup (kOID_ipTrafficStatsGroup, kNAME_ipTrafficStatsGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool ipTrafficStatsGroup::Populate ()
{
  BcmMibObject *pScalar;
  BcmMibTable *pTable;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new TimeticksMibObject (kOID_ipIfStatsTableLastChange, kNAME_ipIfStatsTableLastChange, this, kReadOnly);
  AddScalar (pScalar);

  pTable = new ipSystemStatsTable();
  AddTable (pTable);

  pTable = new ipIfStatsTable();
  AddTable (pTable);

  return true;
}

SNMP_STATUS ipTrafficStatsGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (ipTrafficStatsGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS ipTrafficStatsGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ipIfStatsTableLastChange:
      Status = VB.SetTimeticks (fpBridge->Get_ipIfStatsTableLastChange ());
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
 *  ipSystemStatsTable and related objects 
\*/

/* ipSystemStatsTable */

ipSystemStatsTable::ipSystemStatsTable ()
  : BcmMibTable (kOID_ipSystemStatsTable, kNAME_ipSystemStatsTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsInReceives);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsHCInReceives);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsInOctets);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsHCInOctets);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsInHdrErrors);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsInNoRoutes);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsInAddrErrors);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsInUnknownProtos);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsInTruncatedPkts);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsInForwDatagrams);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsHCInForwDatagrams);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsReasmReqds);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsReasmOKs);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsReasmFails);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsInDiscards);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsInDelivers);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsHCInDelivers);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsOutRequests);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsHCOutRequests);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsOutNoRoutes);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsOutForwDatagrams);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsHCOutForwDatagrams);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsOutDiscards);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsOutFragReqds);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsOutFragOKs);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsOutFragFails);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsOutFragCreates);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsOutTransmits);
  fColumnMask |= (1 << ipSystemStatsEntry::kLEAF_ipSystemStatsHCOutTransmits);
  
  // Since this table has > 32 columns, we must use a second ULONG to fully
  // characterize which columns exist.
  fColumnMask2 |= (1 << (ipSystemStatsEntry::kLEAF_ipSystemStatsOutOctets - 32));
  fColumnMask2 |= (1 << (ipSystemStatsEntry::kLEAF_ipSystemStatsHCOutOctets - 32));
  fColumnMask2 |= (1 << (ipSystemStatsEntry::kLEAF_ipSystemStatsInMcastPkts - 32));
  fColumnMask2 |= (1 << (ipSystemStatsEntry::kLEAF_ipSystemStatsHCInMcastPkts - 32));
  fColumnMask2 |= (1 << (ipSystemStatsEntry::kLEAF_ipSystemStatsInMcastOctets - 32));
  fColumnMask2 |= (1 << (ipSystemStatsEntry::kLEAF_ipSystemStatsHCInMcastOctets - 32));
  fColumnMask2 |= (1 << (ipSystemStatsEntry::kLEAF_ipSystemStatsOutMcastPkts - 32));
  fColumnMask2 |= (1 << (ipSystemStatsEntry::kLEAF_ipSystemStatsHCOutMcastPkts - 32));
  fColumnMask2 |= (1 << (ipSystemStatsEntry::kLEAF_ipSystemStatsOutMcastOctets - 32));
  fColumnMask2 |= (1 << (ipSystemStatsEntry::kLEAF_ipSystemStatsHCOutMcastOctets - 32));
  fColumnMask2 |= (1 << (ipSystemStatsEntry::kLEAF_ipSystemStatsInBcastPkts - 32));
  fColumnMask2 |= (1 << (ipSystemStatsEntry::kLEAF_ipSystemStatsHCInBcastPkts - 32));
  fColumnMask2 |= (1 << (ipSystemStatsEntry::kLEAF_ipSystemStatsOutBcastPkts - 32));
  fColumnMask2 |= (1 << (ipSystemStatsEntry::kLEAF_ipSystemStatsHCOutBcastPkts - 32));
  fColumnMask2 |= (1 << (ipSystemStatsEntry::kLEAF_ipSystemStatsDiscontinuityTime - 32));
  fColumnMask2 |= (1 << (ipSystemStatsEntry::kLEAF_ipSystemStatsRefreshRate - 32));
}

SNMP_STATUS ipSystemStatsTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ipSystemStatsEntry *pEntry;
  ipSystemStatsEntryBridge *pMyBridge = (ipSystemStatsEntryBridge *) pBridge;


  pEntry = new ipSystemStatsEntry (this, pMyBridge);
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

/* ipSystemStatsEntry */

ipSystemStatsEntry::ipSystemStatsEntry (ipSystemStatsTable *pTable, ipSystemStatsEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ipSystemStatsEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_ipSystemStatsIPVersion, kNAME_ipSystemStatsIPVersion, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsInReceives, kNAME_ipSystemStatsInReceives, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipSystemStatsHCInReceives, kNAME_ipSystemStatsHCInReceives, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsInOctets, kNAME_ipSystemStatsInOctets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipSystemStatsHCInOctets, kNAME_ipSystemStatsHCInOctets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsInHdrErrors, kNAME_ipSystemStatsInHdrErrors, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsInNoRoutes, kNAME_ipSystemStatsInNoRoutes, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsInAddrErrors, kNAME_ipSystemStatsInAddrErrors, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsInUnknownProtos, kNAME_ipSystemStatsInUnknownProtos, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsInTruncatedPkts, kNAME_ipSystemStatsInTruncatedPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsInForwDatagrams, kNAME_ipSystemStatsInForwDatagrams, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipSystemStatsHCInForwDatagrams, kNAME_ipSystemStatsHCInForwDatagrams, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsReasmReqds, kNAME_ipSystemStatsReasmReqds, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsReasmOKs, kNAME_ipSystemStatsReasmOKs, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsReasmFails, kNAME_ipSystemStatsReasmFails, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsInDiscards, kNAME_ipSystemStatsInDiscards, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsInDelivers, kNAME_ipSystemStatsInDelivers, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipSystemStatsHCInDelivers, kNAME_ipSystemStatsHCInDelivers, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsOutRequests, kNAME_ipSystemStatsOutRequests, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipSystemStatsHCOutRequests, kNAME_ipSystemStatsHCOutRequests, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsOutNoRoutes, kNAME_ipSystemStatsOutNoRoutes, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsOutForwDatagrams, kNAME_ipSystemStatsOutForwDatagrams, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipSystemStatsHCOutForwDatagrams, kNAME_ipSystemStatsHCOutForwDatagrams, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsOutDiscards, kNAME_ipSystemStatsOutDiscards, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsOutFragReqds, kNAME_ipSystemStatsOutFragReqds, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsOutFragOKs, kNAME_ipSystemStatsOutFragOKs, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsOutFragFails, kNAME_ipSystemStatsOutFragFails, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsOutFragCreates, kNAME_ipSystemStatsOutFragCreates, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsOutTransmits, kNAME_ipSystemStatsOutTransmits, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipSystemStatsHCOutTransmits, kNAME_ipSystemStatsHCOutTransmits, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsOutOctets, kNAME_ipSystemStatsOutOctets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipSystemStatsHCOutOctets, kNAME_ipSystemStatsHCOutOctets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsInMcastPkts, kNAME_ipSystemStatsInMcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipSystemStatsHCInMcastPkts, kNAME_ipSystemStatsHCInMcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsInMcastOctets, kNAME_ipSystemStatsInMcastOctets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipSystemStatsHCInMcastOctets, kNAME_ipSystemStatsHCInMcastOctets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsOutMcastPkts, kNAME_ipSystemStatsOutMcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipSystemStatsHCOutMcastPkts, kNAME_ipSystemStatsHCOutMcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsOutMcastOctets, kNAME_ipSystemStatsOutMcastOctets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipSystemStatsHCOutMcastOctets, kNAME_ipSystemStatsHCOutMcastOctets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsInBcastPkts, kNAME_ipSystemStatsInBcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipSystemStatsHCInBcastPkts, kNAME_ipSystemStatsHCInBcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsOutBcastPkts, kNAME_ipSystemStatsOutBcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipSystemStatsHCOutBcastPkts, kNAME_ipSystemStatsHCOutBcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new TimeticksMibObject (kOID_ipSystemStatsDiscontinuityTime, kNAME_ipSystemStatsDiscontinuityTime, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipSystemStatsRefreshRate, kNAME_ipSystemStatsRefreshRate, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ipSystemStatsEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmCounter64 C64;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ipSystemStatsIPVersion:
      Status = VB.SetInt32 (fpBridge->Get_ipSystemStatsIPVersion ());
      break;

    case kLEAF_ipSystemStatsInReceives:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsInReceives (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsHCInReceives:
      fpBridge->Get_ipSystemStatsHCInReceives (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipSystemStatsInOctets:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsInOctets (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsHCInOctets:
      fpBridge->Get_ipSystemStatsHCInOctets (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipSystemStatsInHdrErrors:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsInHdrErrors (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsInNoRoutes:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsInNoRoutes (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsInAddrErrors:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsInAddrErrors (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsInUnknownProtos:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsInUnknownProtos (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsInTruncatedPkts:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsInTruncatedPkts (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsInForwDatagrams:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsInForwDatagrams (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsHCInForwDatagrams:
      fpBridge->Get_ipSystemStatsHCInForwDatagrams (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipSystemStatsReasmReqds:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsReasmReqds (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsReasmOKs:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsReasmOKs (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsReasmFails:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsReasmFails (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsInDiscards:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsInDiscards (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsInDelivers:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsInDelivers (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsHCInDelivers:
      fpBridge->Get_ipSystemStatsHCInDelivers (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipSystemStatsOutRequests:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsOutRequests (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsHCOutRequests:
      fpBridge->Get_ipSystemStatsHCOutRequests (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipSystemStatsOutNoRoutes:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsOutNoRoutes (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsOutForwDatagrams:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsOutForwDatagrams (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsHCOutForwDatagrams:
      fpBridge->Get_ipSystemStatsHCOutForwDatagrams (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipSystemStatsOutDiscards:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsOutDiscards (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsOutFragReqds:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsOutFragReqds (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsOutFragOKs:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsOutFragOKs (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsOutFragFails:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsOutFragFails (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsOutFragCreates:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsOutFragCreates (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsOutTransmits:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsOutTransmits (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsHCOutTransmits:
      fpBridge->Get_ipSystemStatsHCOutTransmits (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipSystemStatsOutOctets:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsOutOctets (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsHCOutOctets:
      fpBridge->Get_ipSystemStatsHCOutOctets (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipSystemStatsInMcastPkts:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsInMcastPkts (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsHCInMcastPkts:
      fpBridge->Get_ipSystemStatsHCInMcastPkts (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipSystemStatsInMcastOctets:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsInMcastOctets (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsHCInMcastOctets:
      fpBridge->Get_ipSystemStatsHCInMcastOctets (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipSystemStatsOutMcastPkts:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsOutMcastPkts (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsHCOutMcastPkts:
      fpBridge->Get_ipSystemStatsHCOutMcastPkts (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipSystemStatsOutMcastOctets:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsOutMcastOctets (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsHCOutMcastOctets:
      fpBridge->Get_ipSystemStatsHCOutMcastOctets (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipSystemStatsInBcastPkts:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsInBcastPkts (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsHCInBcastPkts:
      fpBridge->Get_ipSystemStatsHCInBcastPkts (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipSystemStatsOutBcastPkts:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsOutBcastPkts (), VBT_COUNTER);
      break;

    case kLEAF_ipSystemStatsHCOutBcastPkts:
      fpBridge->Get_ipSystemStatsHCOutBcastPkts (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipSystemStatsDiscontinuityTime:
      Status = VB.SetTimeticks (fpBridge->Get_ipSystemStatsDiscontinuityTime ());
      break;

    case kLEAF_ipSystemStatsRefreshRate:
      Status = VB.SetUint32 (fpBridge->Get_ipSystemStatsRefreshRate (), VBT_UNSIGNED32);
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
 *  ipIfStatsTable and related objects 
\*/

/* ipIfStatsTable */

ipIfStatsTable::ipIfStatsTable ()
  : BcmMibTable (kOID_ipIfStatsTable, kNAME_ipIfStatsTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsInReceives);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsHCInReceives);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsInOctets);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsHCInOctets);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsInHdrErrors);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsInNoRoutes);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsInAddrErrors);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsInUnknownProtos);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsInTruncatedPkts);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsInForwDatagrams);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsHCInForwDatagrams);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsReasmReqds);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsReasmOKs);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsReasmFails);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsInDiscards);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsInDelivers);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsHCInDelivers);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsOutRequests);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsHCOutRequests);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsOutForwDatagrams);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsHCOutForwDatagrams);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsOutDiscards);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsOutFragReqds);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsOutFragOKs);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsOutFragFails);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsOutFragCreates);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsOutTransmits);
  fColumnMask |= (1 << ipIfStatsEntry::kLEAF_ipIfStatsHCOutTransmits);
  
  // Since this table has > 32 columns, we must use a second ULONG to fully
  // characterize which columns exist.
  fColumnMask2 |= (1 << (ipIfStatsEntry::kLEAF_ipIfStatsOutOctets - 32));
  fColumnMask2 |= (1 << (ipIfStatsEntry::kLEAF_ipIfStatsHCOutOctets - 32));
  fColumnMask2 |= (1 << (ipIfStatsEntry::kLEAF_ipIfStatsInMcastPkts - 32));
  fColumnMask2 |= (1 << (ipIfStatsEntry::kLEAF_ipIfStatsHCInMcastPkts - 32));
  fColumnMask2 |= (1 << (ipIfStatsEntry::kLEAF_ipIfStatsInMcastOctets - 32));
  fColumnMask2 |= (1 << (ipIfStatsEntry::kLEAF_ipIfStatsHCInMcastOctets - 32));
  fColumnMask2 |= (1 << (ipIfStatsEntry::kLEAF_ipIfStatsOutMcastPkts - 32));
  fColumnMask2 |= (1 << (ipIfStatsEntry::kLEAF_ipIfStatsHCOutMcastPkts - 32));
  fColumnMask2 |= (1 << (ipIfStatsEntry::kLEAF_ipIfStatsOutMcastOctets - 32));
  fColumnMask2 |= (1 << (ipIfStatsEntry::kLEAF_ipIfStatsHCOutMcastOctets - 32));
  fColumnMask2 |= (1 << (ipIfStatsEntry::kLEAF_ipIfStatsInBcastPkts - 32));
  fColumnMask2 |= (1 << (ipIfStatsEntry::kLEAF_ipIfStatsHCInBcastPkts - 32));
  fColumnMask2 |= (1 << (ipIfStatsEntry::kLEAF_ipIfStatsOutBcastPkts - 32));
  fColumnMask2 |= (1 << (ipIfStatsEntry::kLEAF_ipIfStatsHCOutBcastPkts - 32));
  fColumnMask2 |= (1 << (ipIfStatsEntry::kLEAF_ipIfStatsDiscontinuityTime - 32));
  fColumnMask2 |= (1 << (ipIfStatsEntry::kLEAF_ipIfStatsRefreshRate - 32));
}

SNMP_STATUS ipIfStatsTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  ipIfStatsEntry *pEntry;
  ipIfStatsEntryBridge *pMyBridge = (ipIfStatsEntryBridge *) pBridge;


  pEntry = new ipIfStatsEntry (this, pMyBridge);
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

/* ipIfStatsEntry */

ipIfStatsEntry::ipIfStatsEntry (ipIfStatsTable *pTable, ipIfStatsEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_ipIfStatsEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_ipIfStatsIPVersion, kNAME_ipIfStatsIPVersion, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_ipIfStatsIfIndex, kNAME_ipIfStatsIfIndex, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsInReceives, kNAME_ipIfStatsInReceives, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipIfStatsHCInReceives, kNAME_ipIfStatsHCInReceives, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsInOctets, kNAME_ipIfStatsInOctets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipIfStatsHCInOctets, kNAME_ipIfStatsHCInOctets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsInHdrErrors, kNAME_ipIfStatsInHdrErrors, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsInNoRoutes, kNAME_ipIfStatsInNoRoutes, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsInAddrErrors, kNAME_ipIfStatsInAddrErrors, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsInUnknownProtos, kNAME_ipIfStatsInUnknownProtos, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsInTruncatedPkts, kNAME_ipIfStatsInTruncatedPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsInForwDatagrams, kNAME_ipIfStatsInForwDatagrams, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipIfStatsHCInForwDatagrams, kNAME_ipIfStatsHCInForwDatagrams, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsReasmReqds, kNAME_ipIfStatsReasmReqds, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsReasmOKs, kNAME_ipIfStatsReasmOKs, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsReasmFails, kNAME_ipIfStatsReasmFails, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsInDiscards, kNAME_ipIfStatsInDiscards, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsInDelivers, kNAME_ipIfStatsInDelivers, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipIfStatsHCInDelivers, kNAME_ipIfStatsHCInDelivers, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsOutRequests, kNAME_ipIfStatsOutRequests, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipIfStatsHCOutRequests, kNAME_ipIfStatsHCOutRequests, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsOutForwDatagrams, kNAME_ipIfStatsOutForwDatagrams, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipIfStatsHCOutForwDatagrams, kNAME_ipIfStatsHCOutForwDatagrams, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsOutDiscards, kNAME_ipIfStatsOutDiscards, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsOutFragReqds, kNAME_ipIfStatsOutFragReqds, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsOutFragOKs, kNAME_ipIfStatsOutFragOKs, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsOutFragFails, kNAME_ipIfStatsOutFragFails, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsOutFragCreates, kNAME_ipIfStatsOutFragCreates, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsOutTransmits, kNAME_ipIfStatsOutTransmits, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipIfStatsHCOutTransmits, kNAME_ipIfStatsHCOutTransmits, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsOutOctets, kNAME_ipIfStatsOutOctets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipIfStatsHCOutOctets, kNAME_ipIfStatsHCOutOctets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsInMcastPkts, kNAME_ipIfStatsInMcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipIfStatsHCInMcastPkts, kNAME_ipIfStatsHCInMcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsInMcastOctets, kNAME_ipIfStatsInMcastOctets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipIfStatsHCInMcastOctets, kNAME_ipIfStatsHCInMcastOctets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsOutMcastPkts, kNAME_ipIfStatsOutMcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipIfStatsHCOutMcastPkts, kNAME_ipIfStatsHCOutMcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsOutMcastOctets, kNAME_ipIfStatsOutMcastOctets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipIfStatsHCOutMcastOctets, kNAME_ipIfStatsHCOutMcastOctets, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsInBcastPkts, kNAME_ipIfStatsInBcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipIfStatsHCInBcastPkts, kNAME_ipIfStatsHCInBcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsOutBcastPkts, kNAME_ipIfStatsOutBcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint64MibObject (kOID_ipIfStatsHCOutBcastPkts, kNAME_ipIfStatsHCOutBcastPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new TimeticksMibObject (kOID_ipIfStatsDiscontinuityTime, kNAME_ipIfStatsDiscontinuityTime, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_ipIfStatsRefreshRate, kNAME_ipIfStatsRefreshRate, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS ipIfStatsEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmCounter64 C64;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_ipIfStatsIPVersion:
      Status = VB.SetInt32 (fpBridge->Get_ipIfStatsIPVersion ());
      break;

    case kLEAF_ipIfStatsIfIndex:
      Status = VB.SetInt32 (fpBridge->Get_ipIfStatsIfIndex ());
      break;

    case kLEAF_ipIfStatsInReceives:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsInReceives (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsHCInReceives:
      fpBridge->Get_ipIfStatsHCInReceives (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipIfStatsInOctets:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsInOctets (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsHCInOctets:
      fpBridge->Get_ipIfStatsHCInOctets (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipIfStatsInHdrErrors:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsInHdrErrors (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsInNoRoutes:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsInNoRoutes (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsInAddrErrors:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsInAddrErrors (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsInUnknownProtos:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsInUnknownProtos (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsInTruncatedPkts:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsInTruncatedPkts (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsInForwDatagrams:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsInForwDatagrams (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsHCInForwDatagrams:
      fpBridge->Get_ipIfStatsHCInForwDatagrams (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipIfStatsReasmReqds:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsReasmReqds (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsReasmOKs:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsReasmOKs (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsReasmFails:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsReasmFails (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsInDiscards:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsInDiscards (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsInDelivers:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsInDelivers (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsHCInDelivers:
      fpBridge->Get_ipIfStatsHCInDelivers (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipIfStatsOutRequests:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsOutRequests (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsHCOutRequests:
      fpBridge->Get_ipIfStatsHCOutRequests (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipIfStatsOutForwDatagrams:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsOutForwDatagrams (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsHCOutForwDatagrams:
      fpBridge->Get_ipIfStatsHCOutForwDatagrams (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipIfStatsOutDiscards:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsOutDiscards (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsOutFragReqds:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsOutFragReqds (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsOutFragOKs:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsOutFragOKs (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsOutFragFails:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsOutFragFails (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsOutFragCreates:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsOutFragCreates (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsOutTransmits:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsOutTransmits (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsHCOutTransmits:
      fpBridge->Get_ipIfStatsHCOutTransmits (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipIfStatsOutOctets:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsOutOctets (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsHCOutOctets:
      fpBridge->Get_ipIfStatsHCOutOctets (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipIfStatsInMcastPkts:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsInMcastPkts (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsHCInMcastPkts:
      fpBridge->Get_ipIfStatsHCInMcastPkts (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipIfStatsInMcastOctets:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsInMcastOctets (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsHCInMcastOctets:
      fpBridge->Get_ipIfStatsHCInMcastOctets (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipIfStatsOutMcastPkts:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsOutMcastPkts (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsHCOutMcastPkts:
      fpBridge->Get_ipIfStatsHCOutMcastPkts (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipIfStatsOutMcastOctets:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsOutMcastOctets (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsHCOutMcastOctets:
      fpBridge->Get_ipIfStatsHCOutMcastOctets (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipIfStatsInBcastPkts:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsInBcastPkts (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsHCInBcastPkts:
      fpBridge->Get_ipIfStatsHCInBcastPkts (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipIfStatsOutBcastPkts:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsOutBcastPkts (), VBT_COUNTER);
      break;

    case kLEAF_ipIfStatsHCOutBcastPkts:
      fpBridge->Get_ipIfStatsHCOutBcastPkts (C64);
      Status = VB.SetUint64 (C64);
      break;

    case kLEAF_ipIfStatsDiscontinuityTime:
      Status = VB.SetTimeticks (fpBridge->Get_ipIfStatsDiscontinuityTime ());
      break;

    case kLEAF_ipIfStatsRefreshRate:
      Status = VB.SetUint32 (fpBridge->Get_ipIfStatsRefreshRate (), VBT_UNSIGNED32);
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

#endif

/*\
 *  icmpGroup group
\*/

icmpGroup::icmpGroup ()
  : BcmMibGroup (kOID_icmpGroup, kNAME_icmpGroup)
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool icmpGroup::Populate ()
{
  BcmMibObject *pScalar;
  BcmMibTable *pTable;


  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new Uint32MibObject (kOID_icmpInMsgs, kNAME_icmpInMsgs, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpInErrors, kNAME_icmpInErrors, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpInDestUnreachs, kNAME_icmpInDestUnreachs, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpInTimeExcds, kNAME_icmpInTimeExcds, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpInParmProbs, kNAME_icmpInParmProbs, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpInSrcQuenchs, kNAME_icmpInSrcQuenchs, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpInRedirects, kNAME_icmpInRedirects, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpInEchos, kNAME_icmpInEchos, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpInEchoReps, kNAME_icmpInEchoReps, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpInTimestamps, kNAME_icmpInTimestamps, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpInTimestampReps, kNAME_icmpInTimestampReps, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpInAddrMasks, kNAME_icmpInAddrMasks, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpInAddrMaskReps, kNAME_icmpInAddrMaskReps, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpOutMsgs, kNAME_icmpOutMsgs, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpOutErrors, kNAME_icmpOutErrors, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpOutDestUnreachs, kNAME_icmpOutDestUnreachs, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpOutTimeExcds, kNAME_icmpOutTimeExcds, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpOutParmProbs, kNAME_icmpOutParmProbs, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpOutSrcQuenchs, kNAME_icmpOutSrcQuenchs, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpOutRedirects, kNAME_icmpOutRedirects, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpOutEchos, kNAME_icmpOutEchos, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpOutEchoReps, kNAME_icmpOutEchoReps, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpOutTimestamps, kNAME_icmpOutTimestamps, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpOutTimestampReps, kNAME_icmpOutTimestampReps, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpOutAddrMasks, kNAME_icmpOutAddrMasks, this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject (kOID_icmpOutAddrMaskReps, kNAME_icmpOutAddrMaskReps, this, kReadOnly);
  AddScalar (pScalar);

  pTable = new icmpStatsTable();
  AddTable (pTable);

  pTable = new icmpMsgStatsTable(this);
  AddTable (pTable);

  return true;
}

SNMP_STATUS icmpGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status = BcmMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);

  if (Status == SNMP_NO_ERROR)
    fpBridge = (icmpGroupBridge *) pBridge;

  return Status;
}

SNMP_STATUS icmpGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_icmpInMsgs:
      Status = VB.SetUint32 (fpBridge->Get_icmpInMsgs (), VBT_COUNTER);
      break;

    case kLEAF_icmpInErrors:
      Status = VB.SetUint32 (fpBridge->Get_icmpInErrors (), VBT_COUNTER);
      break;

    case kLEAF_icmpInDestUnreachs:
      Status = VB.SetUint32 (fpBridge->Get_icmpInDestUnreachs (), VBT_COUNTER);
      break;

    case kLEAF_icmpInTimeExcds:
      Status = VB.SetUint32 (fpBridge->Get_icmpInTimeExcds (), VBT_COUNTER);
      break;

    case kLEAF_icmpInParmProbs:
      Status = VB.SetUint32 (fpBridge->Get_icmpInParmProbs (), VBT_COUNTER);
      break;

    case kLEAF_icmpInSrcQuenchs:
      Status = VB.SetUint32 (fpBridge->Get_icmpInSrcQuenchs (), VBT_COUNTER);
      break;

    case kLEAF_icmpInRedirects:
      Status = VB.SetUint32 (fpBridge->Get_icmpInRedirects (), VBT_COUNTER);
      break;

    case kLEAF_icmpInEchos:
      Status = VB.SetUint32 (fpBridge->Get_icmpInEchos (), VBT_COUNTER);
      break;

    case kLEAF_icmpInEchoReps:
      Status = VB.SetUint32 (fpBridge->Get_icmpInEchoReps (), VBT_COUNTER);
      break;

    case kLEAF_icmpInTimestamps:
      Status = VB.SetUint32 (fpBridge->Get_icmpInTimestamps (), VBT_COUNTER);
      break;

    case kLEAF_icmpInTimestampReps:
      Status = VB.SetUint32 (fpBridge->Get_icmpInTimestampReps (), VBT_COUNTER);
      break;

    case kLEAF_icmpInAddrMasks:
      Status = VB.SetUint32 (fpBridge->Get_icmpInAddrMasks (), VBT_COUNTER);
      break;

    case kLEAF_icmpInAddrMaskReps:
      Status = VB.SetUint32 (fpBridge->Get_icmpInAddrMaskReps (), VBT_COUNTER);
      break;

    case kLEAF_icmpOutMsgs:
      Status = VB.SetUint32 (fpBridge->Get_icmpOutMsgs (), VBT_COUNTER);
      break;

    case kLEAF_icmpOutErrors:
      Status = VB.SetUint32 (fpBridge->Get_icmpOutErrors (), VBT_COUNTER);
      break;

    case kLEAF_icmpOutDestUnreachs:
      Status = VB.SetUint32 (fpBridge->Get_icmpOutDestUnreachs (), VBT_COUNTER);
      break;

    case kLEAF_icmpOutTimeExcds:
      Status = VB.SetUint32 (fpBridge->Get_icmpOutTimeExcds (), VBT_COUNTER);
      break;

    case kLEAF_icmpOutParmProbs:
      Status = VB.SetUint32 (fpBridge->Get_icmpOutParmProbs (), VBT_COUNTER);
      break;

    case kLEAF_icmpOutSrcQuenchs:
      Status = VB.SetUint32 (fpBridge->Get_icmpOutSrcQuenchs (), VBT_COUNTER);
      break;

    case kLEAF_icmpOutRedirects:
      Status = VB.SetUint32 (fpBridge->Get_icmpOutRedirects (), VBT_COUNTER);
      break;

    case kLEAF_icmpOutEchos:
      Status = VB.SetUint32 (fpBridge->Get_icmpOutEchos (), VBT_COUNTER);
      break;

    case kLEAF_icmpOutEchoReps:
      Status = VB.SetUint32 (fpBridge->Get_icmpOutEchoReps (), VBT_COUNTER);
      break;

    case kLEAF_icmpOutTimestamps:
      Status = VB.SetUint32 (fpBridge->Get_icmpOutTimestamps (), VBT_COUNTER);
      break;

    case kLEAF_icmpOutTimestampReps:
      Status = VB.SetUint32 (fpBridge->Get_icmpOutTimestampReps (), VBT_COUNTER);
      break;

    case kLEAF_icmpOutAddrMasks:
      Status = VB.SetUint32 (fpBridge->Get_icmpOutAddrMasks (), VBT_COUNTER);
      break;

    case kLEAF_icmpOutAddrMaskReps:
      Status = VB.SetUint32 (fpBridge->Get_icmpOutAddrMaskReps (), VBT_COUNTER);
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
 *  icmpStatsTable and related objects 
\*/

/* icmpStatsTable */

icmpStatsTable::icmpStatsTable ()
  : BcmMibTable (kOID_icmpStatsTable, kNAME_icmpStatsTable)
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << icmpStatsEntry::kLEAF_icmpStatsInMsgs);
  fColumnMask |= (1 << icmpStatsEntry::kLEAF_icmpStatsInErrors);
  fColumnMask |= (1 << icmpStatsEntry::kLEAF_icmpStatsOutMsgs);
  fColumnMask |= (1 << icmpStatsEntry::kLEAF_icmpStatsOutErrors);
}

SNMP_STATUS icmpStatsTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  icmpStatsEntry *pEntry;
  icmpStatsEntryBridge *pMyBridge = (icmpStatsEntryBridge *) pBridge;


  pEntry = new icmpStatsEntry (this, pMyBridge);
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

/* icmpStatsEntry */

icmpStatsEntry::icmpStatsEntry (icmpStatsTable *pTable, icmpStatsEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_icmpStatsEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_icmpStatsIPVersion, kNAME_icmpStatsIPVersion, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_icmpStatsInMsgs, kNAME_icmpStatsInMsgs, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_icmpStatsInErrors, kNAME_icmpStatsInErrors, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_icmpStatsOutMsgs, kNAME_icmpStatsOutMsgs, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_icmpStatsOutErrors, kNAME_icmpStatsOutErrors, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS icmpStatsEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_icmpStatsIPVersion:
      Status = VB.SetInt32 (fpBridge->Get_icmpStatsIPVersion ());
      break;

    case kLEAF_icmpStatsInMsgs:
      Status = VB.SetUint32 (fpBridge->Get_icmpStatsInMsgs (), VBT_COUNTER);
      break;

    case kLEAF_icmpStatsInErrors:
      Status = VB.SetUint32 (fpBridge->Get_icmpStatsInErrors (), VBT_COUNTER);
      break;

    case kLEAF_icmpStatsOutMsgs:
      Status = VB.SetUint32 (fpBridge->Get_icmpStatsOutMsgs (), VBT_COUNTER);
      break;

    case kLEAF_icmpStatsOutErrors:
      Status = VB.SetUint32 (fpBridge->Get_icmpStatsOutErrors (), VBT_COUNTER);
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
 *  icmpMsgStatsTable and related objects 
\*/

/* icmpMsgStatsTable */

icmpMsgStatsTable::icmpMsgStatsTable (icmpGroup * icmpgroup)
  : BcmMibTable (kOID_icmpMsgStatsTable, kNAME_icmpMsgStatsTable)
{
   fpIcmpGroup = icmpgroup;

  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << icmpMsgStatsEntry::kLEAF_icmpMsgStatsInPkts);
  fColumnMask |= (1 << icmpMsgStatsEntry::kLEAF_icmpMsgStatsOutPkts);

  fpMsgTypesV4 = new int [MAXNUM_ICMPMTYPESV4];
  
  for (int m = 0 ;m< MAXNUM_ICMPMTYPESV4 ;m++)
    *(fpMsgTypesV4+m) = -1;

  int i=0;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_ECHOREP;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_DESTUNREACH;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_SRCQUENCH;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_REDIRECT;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_ALTERNATEHOSTADDRESS;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_ECHO;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_ROUTERADVERTISEMENT;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_ROUTERSELECTION;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_TIMEEXCD;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_PARMPROB;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_TIMESTAMP;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_TIMESTAMPREP;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_INFORMATIONREQUEST;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_INFORMATIONREPLY;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_ADDRMASK;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_ADDRMASKREP;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_TRACEROUTE;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_DATAGRAMCONVERSIONERROR;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_MOBILEHOSTREDIRECT;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_IPV6WHEREAREYOU;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_IPV6IAMHERE;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_MOBILEREGISTRATIONREQUEST;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_MOBILEREGISTRATIONREPLY;
  *(fpMsgTypesV4+i++) = CONSTVAL_ICMPMSGTYPE_DOMAINNAMEREQUEST;
   //assert(i > MAXNUM_ICMPMTYPESV4);

#if (BCM_IPV6_SUPPORT)
   fpMsgTypesV6 = new int [MAXNUM_ICMPMTYPESV6];
   
   for ( m = 0 ;m< MAXNUM_ICMPMTYPESV6; m++)
     *(fpMsgTypesV6+m) = -1;

   i=0;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_DESTUNREACH;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_PKTTOOBIG;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_TIMEEXCD;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_PARMPROBLEM;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_ECHO;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_ECHOREP;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_MCLISTENERQUERY;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_MCLISTENERREPORT;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_MCLISTENERDONE;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_ROUTERSOLICITATION;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_ROUTERADVERTISEMENT;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_NEIGHBORSOLICITATION;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_NEIGHBORADVERTISEMENT;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_REDIRECT;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_ROUTERRENUMBER;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_NODEINFOQUERY;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_NODEINFORESPONSE;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_INVNDSOLICITATION;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_INVNDADVERTISEMENT;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_V2MCLISTENERREPORT;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_HAADDRDISCREQUEST;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_HAADDRDISCREPLY;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_MOBILEPRESOLICITATION;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_MOBILEPREADVERTISEMENT;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_CERTPATHSOLICITATION;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_CERTPATHADVERTISEMENT;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_MCROUTERADVERTISEMENT;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_MCROUTERSOLICITATION;
   *(fpMsgTypesV6+i++) = CONSTVAL_ICMPv6MSGTYPE_MCROUTERTERMINATION;
    //assert(i > MAXNUM_ICMPMTYPESV6);
#endif
}

icmpMsgStatsTable::~icmpMsgStatsTable ()
{
  delete [] fpMsgTypesV4;
  delete [] fpMsgTypesV6;
}

SNMP_STATUS icmpMsgStatsTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  icmpMsgStatsEntry *pEntry;
  icmpMsgStatsEntryBridge *pMyBridge = (icmpMsgStatsEntryBridge *) pBridge;


  pEntry = new icmpMsgStatsEntry (this, pMyBridge);
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

/* icmpMsgStatsEntry */

icmpMsgStatsEntry::icmpMsgStatsEntry (icmpMsgStatsTable *pTable, icmpMsgStatsEntryBridge *pBridge)
  : BcmMibEntry (pTable, kNAME_icmpMsgStatsEntry)
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject (kOID_icmpMsgStatsIPVersion, kNAME_icmpMsgStatsIPVersion, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Int32MibObject (kOID_icmpMsgStatsType, kNAME_icmpMsgStatsType, this, kNotAccessible);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_icmpMsgStatsInPkts, kNAME_icmpMsgStatsInPkts, this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject (kOID_icmpMsgStatsOutPkts, kNAME_icmpMsgStatsOutPkts, this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS icmpMsgStatsEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_icmpMsgStatsIPVersion:
      Status = VB.SetInt32 (fpBridge->Get_icmpMsgStatsIPVersion ());
      break;

    case kLEAF_icmpMsgStatsType:
      Status = VB.SetInt32 (fpBridge->Get_icmpMsgStatsType ());
      break;

    case kLEAF_icmpMsgStatsInPkts:
      Status = VB.SetUint32 (fpBridge->Get_icmpMsgStatsInPkts (), VBT_COUNTER);
      break;

    case kLEAF_icmpMsgStatsOutPkts:
      Status = VB.SetUint32 (fpBridge->Get_icmpMsgStatsOutPkts (), VBT_COUNTER);
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


void icmpMsgStatsTable::Update ()
{
  // The table should have a row for all IPv4 ICMP message types
  // and a row for all IPv6 ICMP message types. However, a given row
  // need not be instantiated unless a message of that type has been
  // processed.  So, we don't delete any rows here; but, we need to 
  // check and see if we need to add rows.

  // icmpGroupBridge* icmpgroupbridge = (icmpGroupBridge*)(GetParent()->GetBridge());
  icmpMsgStatsEntryBridge *pBridge;

  icmpGroupBridge * icmpgroupbridge = fpIcmpGroup->fpBridge;

  // We are ready to new up some bridges if necessary. Go through our list of
  // ICMP message types and create an entry for each type that has counts. 
  // When we create an entry we won't try that one next time.
  for ( int Loop = 0; Loop < MAXNUM_ICMPMTYPESV4; Loop++ )
  {
    if ( *(fpMsgTypesV4+Loop) != -1 )
    {
      if ( ( icmpgroupbridge->IcmpInMsgs (*(fpMsgTypesV4+Loop)) > 0 ) ||
           ( icmpgroupbridge->IcmpOutMsgs(*(fpMsgTypesV4+Loop)) > 0 ) )
      {
        // We have some counts for this message type. Create a entry in the table
        pBridge = new icmpMsgStatsEntryBridge (CONSTVAL_ICMPMSGSTATSIPVERSION_IPV4, *(fpMsgTypesV4+Loop), icmpgroupbridge, fpAgent);
        
        // Calling BcmMibTable::RegisterBridge here is much faster then 
        // calling BcmMibBridge::Register because it means we don't have
        // to search for the table.
        if (RegisterBridge(pBridge, true) != SNMP_NO_ERROR)
          delete pBridge;

        *(fpMsgTypesV4+Loop) = -1; // don't try this one again
      }
    }
  }

#if (BCM_IPV6_SUPPORT)
  // Add ICMPv6 to the table
  for ( Loop = 0; Loop < MAXNUM_ICMPMTYPESV6; Loop++ )
  {
    if ( *(fpMsgTypesV6+Loop) != -1 )
    {
      if ( ( icmpgroupbridge->Icmp6InMsgs (*(fpMsgTypesV6+Loop)) > 0 ) ||
           ( icmpgroupbridge->Icmp6OutMsgs(*(fpMsgTypesV6+Loop)) > 0 ) )
      {
        // We have some counts for this message type. Create a entry in the table
        pBridge = new icmpMsgStatsEntryBridge (CONSTVAL_ICMPMSGSTATSIPVERSION_IPV6, *(fpMsgTypesV6+Loop), icmpgroupbridge, fpAgent);
        
        // Calling BcmMibTable::RegisterBridge here is much faster then 
        // calling BcmMibBridge::Register because it means we don't have
        // to search for the table.
        if (RegisterBridge(pBridge, true) != SNMP_NO_ERROR)
          delete pBridge;

        *(fpMsgTypesV6+Loop) = -1; //don't try this one again
      }
    }
  }

#endif
}
