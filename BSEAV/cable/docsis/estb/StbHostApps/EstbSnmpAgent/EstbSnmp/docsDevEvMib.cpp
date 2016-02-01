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
//    Filename: docsDevEvMib.cpp
//    Creation Date: Wednesday, February 04, 2004 at 15:43:07
//    Created by Broadcom BFC / V2 Mib Compiler BCMIBC.EXE version 3.0.0b
//    from input file rfc2669.mib
//
//**************************************************************************

#include "docsDevEvMib.h"
#include "docsDevEvBridge.h"
#include "SnmpLog.h" // for SnmpLog... macros
#include "BcmSnmpAgent.h"



/*\
 *  docsDevEventGroup group
\*/

docsDevEventGroup::docsDevEventGroup ()
  : docsDevMibGroup ("1.3.6.1.2.1.69.1.5", "docsDevEventGroup")
{
  // Initialize bridge member to NULL
  fpBridge = NULL;

#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool docsDevEventGroup::Populate ()
{
  BcmMibObject *pScalar;
  docsDevMibTable *pTable;


  if (docsDevMibGroup::Populate() == false)
    return false;

  pScalar = new Int32MibObject ("1.3.6.1.2.1.69.1.5.1.0", "docsDevEvControl", this, kReadWrite);
  AddScalar (pScalar);

  pScalar = new IpAddrMibObject ("1.3.6.1.2.1.69.1.5.2.0", "docsDevEvSyslog", this, kReadWrite);
  AddScalar (pScalar);

  pScalar = new Int32MibObject ("1.3.6.1.2.1.69.1.5.3.0", "docsDevEvThrottleAdminStatus", this, kReadWrite);
  AddScalar (pScalar);

  pScalar = new Int32MibObject ("1.3.6.1.2.1.69.1.5.4.0", "docsDevEvThrottleInhibited", this, kReadOnly);
  AddScalar (pScalar);

  pScalar = new Uint32MibObject ("1.3.6.1.2.1.69.1.5.5.0", "docsDevEvThrottleThreshold", this, kReadWrite);
  AddScalar (pScalar);

  pScalar = new Int32MibObject ("1.3.6.1.2.1.69.1.5.6.0", "docsDevEvThrottleInterval", this, kReadWrite);
  AddScalar (pScalar);

  pTable = new docsDevEvControlTable();
  AddTable (pTable);

  pTable = new docsDevEventTable();
  AddTable (pTable);

  return true;
}

SNMP_STATUS docsDevEventGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  fpBridge = (docsDevEventGroupBridge *) pBridge;
  return docsDevMibGroup::RegisterBridge (pBridge, DeleteUponDestruction);
}

SNMP_STATUS docsDevEventGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;
  BcmIpAddress Ip;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_docsDevEvControl:
      Status = VB.SetInt32 (fpBridge->Get_docsDevEvControl ());
      break;

    case kLEAF_docsDevEvSyslog:
      fpBridge->Get_docsDevEvSyslog (Ip);
      Status = VB.SetIp (Ip);
      break;

    case kLEAF_docsDevEvThrottleAdminStatus:
      Status = VB.SetInt32 (fpBridge->Get_docsDevEvThrottleAdminStatus ());
      break;

    case kLEAF_docsDevEvThrottleInhibited:
      Status = VB.SetBool (fpBridge->Get_docsDevEvThrottleInhibited ());
      break;

    case kLEAF_docsDevEvThrottleThreshold:
      Status = VB.SetUint32 (fpBridge->Get_docsDevEvThrottleThreshold (), VBT_UNSIGNED32);
      break;  

    case kLEAF_docsDevEvThrottleInterval:
      Status = VB.SetInt32 (fpBridge->Get_docsDevEvThrottleInterval ());
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

SNMP_STATUS docsDevEventGroup::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status;
  BcmIpAddress Ip;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_docsDevEvControl:
    {
      int docsDevEvControl;

      Status = VB.GetInt32 (&docsDevEvControl);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevEvControl (docsDevEvControl);
      break;
    }

    case kLEAF_docsDevEvSyslog:
      Status = VB.GetIp (Ip);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevEvSyslog (Ip);
      break;

    case kLEAF_docsDevEvThrottleAdminStatus:
    {
      int docsDevEvThrottleAdminStatus;

      Status = VB.GetInt32 (&docsDevEvThrottleAdminStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevEvThrottleAdminStatus (docsDevEvThrottleAdminStatus);
      break;
    }

    case kLEAF_docsDevEvThrottleInhibited:
      Status = SNMP_NOT_WRITABLE;
      break;

    case kLEAF_docsDevEvThrottleThreshold:
    {
      unsigned int docsDevEvThrottleThreshold;

      Status = VB.GetUint32 (&docsDevEvThrottleThreshold);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevEvThrottleThreshold (docsDevEvThrottleThreshold);
      break;
    }

    case kLEAF_docsDevEvThrottleInterval:
    {
      int docsDevEvThrottleInterval;

      Status = VB.GetInt32 (&docsDevEvThrottleInterval);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevEvThrottleInterval (docsDevEvThrottleInterval);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}


/*\ 
 *  docsDevEvControlTable and related objects 
\*/

/* docsDevEvControlTable */

docsDevEvControlTable::docsDevEvControlTable ()
  : docsDevMibTable ("1.3.6.1.2.1.69.1.5.7", "docsDevEvControlTable")
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << docsDevEvControlEntry::kLEAF_docsDevEvReporting);
}

SNMP_STATUS docsDevEvControlTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  docsDevEvControlEntry *pEntry;
  docsDevEvControlEntryBridge *pMyBridge = (docsDevEvControlEntryBridge *) pBridge;


  pEntry = new docsDevEvControlEntry (this, pMyBridge);
  SNMP_STATUS Status = AddEntry (pEntry);

  if (Status == SNMP_NO_ERROR)
  {
    // Remember bridge for destruction by base class later if specified.
    Status = pEntry->RegisterBridge (pBridge, DeleteUponDestruction);
  }
  else
    delete pEntry;
    
  return Status;
}

/* docsDevEvControlEntry */

docsDevEvControlEntry::docsDevEvControlEntry (docsDevEvControlTable *pTable, docsDevEvControlEntryBridge *pBridge)
  : docsDevMibEntry (pTable, "docsDevEvControlEntry")
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject ("1.3.6.1.2.1.69.1.5.7.1.1", "docsDevEvPriority", this, kNotAccessible);
  AddObject (pObject);

  pObject = new BitsMibObject ("1.3.6.1.2.1.69.1.5.7.1.2", "docsDevEvReporting", this, kReadWrite);
  AddObject (pObject);
}

bool docsDevEventEntry::IsAccessible (BcmMibRequest *pReq) const
{
  int MyLevel;
  int EvControl;
  

  // First, check for plain old docsDevMibEntry accessibility
  if (docsDevMibEntry::IsAccessible (pReq) == false)
    return false;

  MyLevel = fpBridge->Get_docsDevEvLevel();
  EvControl = fpBridge->fpLog->GetLevelControl(MyLevel);
  
  // If my severity level doesn't have kLocal turned on, hide
  // this event.  Note that this should never happen because we must
  // never have an event level which specifies traps or syslog without
  // having either local nonvol or local vol, but one never knows
  // what our boys at CL will cook up.
  if (((EvControl & EventLog::kLocalVolatile) == 0)
  &&  ((EvControl & EventLog::kLocalNonVol)   == 0))
    return false;
    
  return true;
}

SNMP_STATUS docsDevEvControlEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_docsDevEvPriority: // not-accessible
      Status = VB.SetInt32 (fpBridge->Get_docsDevEvPriority ());
      break;

    case kLEAF_docsDevEvReporting:
      // UNFINISHED: Make sure the number of octets argument is correct
      Status = VB.SetBits (fpBridge->Get_docsDevEvReporting (), 1);
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

SNMP_STATUS docsDevEvControlEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_docsDevEvPriority:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_docsDevEvReporting:
    {
      unsigned long docsDevEvReporting;

      Status = VB.GetBits (&docsDevEvReporting);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevEvReporting (docsDevEvReporting);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}


/*\ 
 *  docsDevEventTable and related objects 
\*/

/* docsDevEventTable */

docsDevEventTable::docsDevEventTable ()
  : docsDevMibTable ("1.3.6.1.2.1.69.1.5.8", "docsDevEventTable")
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << docsDevEventEntry::kLEAF_docsDevEvFirstTime);
  fColumnMask |= (1 << docsDevEventEntry::kLEAF_docsDevEvLastTime);
  fColumnMask |= (1 << docsDevEventEntry::kLEAF_docsDevEvCounts);
  fColumnMask |= (1 << docsDevEventEntry::kLEAF_docsDevEvLevel);
  fColumnMask |= (1 << docsDevEventEntry::kLEAF_docsDevEvId);
  fColumnMask |= (1 << docsDevEventEntry::kLEAF_docsDevEvText);
}

SNMP_STATUS docsDevEventTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  docsDevEventEntry *pEntry;
  docsDevEventEntryBridge *pMyBridge = (docsDevEventEntryBridge *) pBridge;


  pEntry = new docsDevEventEntry (this, pMyBridge);
  SNMP_STATUS Status = AddEntry (pEntry);

  if (Status == SNMP_NO_ERROR)
  {
    // Remember bridge for destruction by base class later if specified.
    Status = pEntry->RegisterBridge (pBridge, DeleteUponDestruction);
  }
  else
    delete pEntry;
    
  return Status;
}

/* docsDevEventEntry */

docsDevEventEntry::docsDevEventEntry (docsDevEventTable *pTable, docsDevEventEntryBridge *pBridge)
  : docsDevMibEntry (pTable, "docsDevEventEntry")
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject ("1.3.6.1.2.1.69.1.5.8.1.1", "docsDevEvIndex", this, kNotAccessible);
  AddObject (pObject);

  pObject = new DateTimeMibObject ("1.3.6.1.2.1.69.1.5.8.1.2", "docsDevEvFirstTime", this, kReadOnly);
  AddObject (pObject);

  pObject = new DateTimeMibObject ("1.3.6.1.2.1.69.1.5.8.1.3", "docsDevEvLastTime", this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject ("1.3.6.1.2.1.69.1.5.8.1.4", "docsDevEvCounts", this, kReadOnly);
  AddObject (pObject);

  pObject = new Int32MibObject ("1.3.6.1.2.1.69.1.5.8.1.5", "docsDevEvLevel", this, kReadOnly);
  AddObject (pObject);

  pObject = new Uint32MibObject ("1.3.6.1.2.1.69.1.5.8.1.6", "docsDevEvId", this, kReadOnly);
  AddObject (pObject);

  pObject = new StringMibObject ("1.3.6.1.2.1.69.1.5.8.1.7", "docsDevEvText", this, kReadOnly);
  AddObject (pObject);
}

SNMP_STATUS docsDevEventEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_docsDevEvIndex: // not-accessible
      Status = VB.SetInt32 (fpBridge->Get_docsDevEvIndex ());
      break;

    case kLEAF_docsDevEvFirstTime:
      Status = VB.SetDateAndTime (fpBridge->Get_docsDevEvFirstTime ());
      break;

    case kLEAF_docsDevEvLastTime:
      Status = VB.SetDateAndTime (fpBridge->Get_docsDevEvLastTime ());
      break;

    case kLEAF_docsDevEvCounts:
      Status = VB.SetUint32 (fpBridge->Get_docsDevEvCounts (), VBT_COUNTER);
      break;

    case kLEAF_docsDevEvLevel:
      Status = VB.SetInt32 (fpBridge->Get_docsDevEvLevel ());
      break;

    case kLEAF_docsDevEvId:
      Status = VB.SetUint32 (fpBridge->Get_docsDevEvId (), VBT_UNSIGNED32);
      break;

    case kLEAF_docsDevEvText:
    {
      BcmString String;

      fpBridge->Get_docsDevEvText (String);
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

