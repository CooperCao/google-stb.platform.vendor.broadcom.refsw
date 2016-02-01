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
//    Filename: communityNewMib.cpp
//    Creation Date: Wednesday, February 04, 2004 at 15:30:51
//    Created by Broadcom BFC / V2 Mib Compiler BCMIBC.EXE version 3.0.0b
//    from input file rfc2576.mib
//
//**************************************************************************

#include "communityMib.h"
#include "communityBridge.h"
#include "SnmpLog.h" // for SnmpLog... macros
#include "BcmSnmpAgent.h"
#include "SnmpApi.h"
#include "SnmpLog.h"


/*\
 *  snmpCommunityMIBGroup group
\*/

snmpCommunityMIBGroup::snmpCommunityMIBGroup ()
  : BcmMibGroup ("1.3.6.1.6.3.18", "snmpCommunityMIBGroup")
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool snmpCommunityMIBGroup::Populate ()
{
  BcmMibGroup *pGroup;


  if (BcmMibGroup::Populate() == false)
    return false;

  pGroup = new snmpCommunityMIBObjectsGroup();
  AddGroup (pGroup);

  return true;
}

/*\
 *  snmpCommunityMIBObjectsGroup group
\*/

snmpCommunityMIBObjectsGroup::snmpCommunityMIBObjectsGroup ()
  : BcmMibGroup ("1.3.6.1.6.3.18.1", "snmpCommunityMIBObjectsGroup")
{
#if !DYNAMIC_POPULATION
  Populate ();
#endif
}

bool snmpCommunityMIBObjectsGroup::Populate ()
{
  BcmMibTable *pTable;


  if (BcmMibGroup::Populate() == false)
    return false;

  pTable = new snmpCommunityTable();
  AddTable (pTable);

  pTable = new snmpTargetAddrExtTable();
  AddTable (pTable);

  return true;
}


/*\ 
 *  snmpCommunityTable and related objects 
\*/

/* snmpCommunityTable */

snmpCommunityTable::snmpCommunityTable ()
  : BcmMibTable ("1.3.6.1.6.3.18.1.1", "snmpCommunityTable")
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << snmpCommunityEntry::kLEAF_snmpCommunityName);
  fColumnMask |= (1 << snmpCommunityEntry::kLEAF_snmpCommunitySecurityName);
  fColumnMask |= (1 << snmpCommunityEntry::kLEAF_snmpCommunityContextEngineID);
  fColumnMask |= (1 << snmpCommunityEntry::kLEAF_snmpCommunityContextName);
  fColumnMask |= (1 << snmpCommunityEntry::kLEAF_snmpCommunityTransportTag);
  fColumnMask |= (1 << snmpCommunityEntry::kLEAF_snmpCommunityStorageType);
  fColumnMask |= (1 << snmpCommunityEntry::kLEAF_snmpCommunityStatus);
}

SNMP_STATUS snmpCommunityTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  snmpCommunityEntry *pEntry;
  snmpCommunityEntryBridge *pMyBridge = (snmpCommunityEntryBridge *) pBridge;


  pEntry = new snmpCommunityEntry (this, pMyBridge);
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

BcmMibEntry *snmpCommunityTable::NewDefaultEntry (BcmObjectId &Index)
{
  if (fpAgent == NULL)
    return NULL;

  /* Verify index length. */

  // snmpCommunityIndex (String / OID): 1 .. 32 elements
  if ((Index.Length() < 1)
  ||  (Index.Length() > 32))
    return NULL;

  /* Verify index value(s) */

  // UNFINISHED: snmpCommunityIndex (string / OID of variable size)
  // Verify value of this and subsequent elements if
  // necessary.

  // Only register one of these to an SNMPv3 agent.  Check with the thread
  // to see if my agent is a V3 one.
  BcmSnmpV3Agent *pV3Agent = BcmSnmpThread::Singleton().FindV3Agent(fpAgent);
  if (pV3Agent == NULL)
    return NULL;

  snmpCommunityEntryBridge *pBridge = new snmpCommunityEntryBridge (Index, (BcmSnmpV3Agent *)fpAgent);

  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
  {
    SnmpLogInit << "snmpCommunityTable::NewDefaultEntry Bridge registration failed!  Deleting bridge object." << endl;
    delete (pBridge);
    return NULL;
  }
  
  return (BcmMibEntry *)pBridge->fpObject;
}

SNMP_STATUS snmpCommunityTable::ValidateRequest (const BcmString &Community, const BcmIpAddress &SrcIp, int SrcPort, BcmString &SecurityName)
{
    // Initialize to indicate no cmty match.
    SNMP_STATUS Status = SNMP_BAD_COMMUNITY_NAME; 

    // Iterate through the table, check each entry for a match.
    BcmObjectId Index;
    snmpCommunityEntry *pEntry = (snmpCommunityEntry *) FirstEntry ();
    snmpCommunityEntryBridge *pCmty;

    SnmpLogNm << "snmpCommunityTable validating request." << endl;
    
    while ( pEntry )
    {
        pEntry->GetIndex(Index);
        SnmpLogNm << "  Checking snmpCommunityEntry." << Index << " for match..." << endl;

        pCmty = pEntry->fpBridge;
        if ( pCmty->MatchesCommunity (Community) )
        {
            SnmpLogNm << "  Matched community, checking IP address." << endl;
            if ( pCmty->MatchesIp (SrcIp, SrcPort) )
            {
                // Map our security name if we have a full match.
                pCmty->Get_snmpCommunitySecurityName (SecurityName);
                SnmpLogNm << "  Matched IP address.  Access granted.  SecurityName=" << SecurityName << endl;
                return SNMP_NO_ERROR;
            }

            // If we match a community but not the IP address, then it's
            // BAD_COMMUNITY_USE for us.  But keep looking in case we match
            // both elsewhere.
            else
            {
                SnmpLogNm << "  No match on IP address." << endl;
                Status = SNMP_BAD_COMMUNITY_USE;  
            }
        }

        pEntry = (snmpCommunityEntry *) NextEntry (Index);
    }

    if ( Status == SNMP_BAD_COMMUNITY_NAME )
        SnmpLogNm << "Matched no community.  BAD_COMMUNITY_NAME." << endl;
    else if ( Status == SNMP_BAD_COMMUNITY_USE )
        SnmpLogNm << "Matched community but not IP.  BAD_COMMUNITY_USE." << endl;
    else
        SnmpLogNm << "Validation failed w/ code " << Status << endl;

    return Status;
}


SNMP_STATUS snmpCommunityTable::ResolveSecNameToCommName ( BcmString &Community, const BcmIpAddress &SrcIp, BcmString &SecurityName, int TrapPort)
{
    // Initialize to indicate no cmty match.
    SNMP_STATUS Status = SNMP_BAD_COMMUNITY_NAME; 

    // Iterate through the table, check each entry for a match.
    BcmObjectId Index;
    snmpCommunityEntry *pEntry = (snmpCommunityEntry *) FirstEntry ();
    snmpCommunityEntryBridge *pCmty;

    SnmpLogNm << "Finding coex trap community for sec name " << SecurityName << endl;

    while ( pEntry )
    {
        pEntry->GetIndex(Index);
        SnmpLogNm << "  Checking snmpCommunityEntry." << Index << " for match..." << endl;

        pCmty = pEntry->fpBridge;
        if ( pCmty->MatchesSecurityName (SecurityName) )
        {
            SnmpLogNm << "  Matched SecurityName, checking IP address." << endl;
            if ( pCmty->MatchesIp (SrcIp, TrapPort, false) ) // Notify port = 162...
            {
                // Map our security name if we have a full match.
                pCmty->Get_snmpCommunityName (Community);
                SnmpLogNm << "  Matched IP address.  Community=" << Community << endl;
                return SNMP_NO_ERROR;
            }

            // If we match a community but not the IP address, then it's
            // BAD_COMMUNITY_USE for us.  But keep looking in case we match
            // both elsewhere.
            else
            {
                SnmpLogNm << "  No match on IP address." << endl;
                Status = SNMP_BAD_COMMUNITY_USE;  
            }
        }

        pEntry = (snmpCommunityEntry *) NextEntry (Index);
    }

    if ( Status == SNMP_BAD_COMMUNITY_NAME )
        SnmpLogNm << "Matched no community.  BAD_COMMUNITY_NAME." << endl;
    else if ( Status == SNMP_BAD_COMMUNITY_USE )
        SnmpLogNm << "Matched community but not IP.  BAD_COMMUNITY_USE." << endl;
    else
        SnmpLogNm << "Validation failed w/ code " << Status << endl;

    return Status;
}

/* snmpCommunityEntry */

snmpCommunityEntry::snmpCommunityEntry (snmpCommunityTable *pTable, snmpCommunityEntryBridge *pBridge)
  : BcmMibEntry (pTable, "snmpCommunityEntry")
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new StringMibObject ("1.3.6.1.6.3.18.1.1.1.1", "snmpCommunityIndex", this, kNotAccessible);
  AddObject (pObject);

  pObject = new StringMibObject ("1.3.6.1.6.3.18.1.1.1.2", "snmpCommunityName", this, kReadCreate);
  AddObject (pObject);

  pObject = new StringMibObject ("1.3.6.1.6.3.18.1.1.1.3", "snmpCommunitySecurityName", this, kReadCreate);
  AddObject (pObject);

  pObject = new StringMibObject ("1.3.6.1.6.3.18.1.1.1.4", "snmpCommunityContextEngineID", this, kReadCreate);
  AddObject (pObject);

  pObject = new StringMibObject ("1.3.6.1.6.3.18.1.1.1.5", "snmpCommunityContextName", this, kReadCreate);
  AddObject (pObject);

  pObject = new StringMibObject ("1.3.6.1.6.3.18.1.1.1.6", "snmpCommunityTransportTag", this, kReadCreate);
  AddObject (pObject);

  pObject = new Int32MibObject ("1.3.6.1.6.3.18.1.1.1.7", "snmpCommunityStorageType", this, kReadCreate);
  AddObject (pObject);

  pObject = new RowStatusMibObject (pTable, this, "1.3.6.1.6.3.18.1.1.1.8", "snmpCommunityStatus", kReadCreate);
  AddObject (pObject);
}

SNMP_STATUS snmpCommunityEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_snmpCommunityIndex: // not-accessible
      fpBridge->Get_snmpCommunityIndex (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_snmpCommunityName:
      fpBridge->Get_snmpCommunityName (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_snmpCommunitySecurityName:
      fpBridge->Get_snmpCommunitySecurityName (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_snmpCommunityContextEngineID:
      fpBridge->Get_snmpCommunityContextEngineID (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_snmpCommunityContextName:
      fpBridge->Get_snmpCommunityContextName (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_snmpCommunityTransportTag:
      fpBridge->Get_snmpCommunityTransportTag (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_snmpCommunityStorageType:
      Status = VB.SetInt32 (fpBridge->Get_snmpCommunityStorageType ());
      break;

    case kLEAF_snmpCommunityStatus:
      Status = VB.SetInt32 (fpBridge->Get_snmpCommunityStatus ());
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

SNMP_STATUS snmpCommunityEntry::TestLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_snmpCommunityIndex:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_snmpCommunityName:
      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_snmpCommunityName (String);
      break;

    case kLEAF_snmpCommunitySecurityName:
      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_snmpCommunitySecurityName (String);
      break;
      
    case kLEAF_snmpCommunityContextEngineID:
      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_snmpCommunityContextEngineID (String);
      break;
      
    case kLEAF_snmpCommunityStorageType:
    {
      int snmpCommunityStorageType;

      Status = VB.GetInt32 (&snmpCommunityStorageType);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_snmpCommunityStorageType (snmpCommunityStorageType);
      break;
    }

    case kLEAF_snmpCommunityStatus:
    {
      int snmpCommunityStatus;

      Status = VB.GetInt32 (&snmpCommunityStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_snmpCommunityStatus (snmpCommunityStatus);
      break;
    }

  }

  return Status;
}

SNMP_STATUS snmpCommunityEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;
    
  // From the StorageType TEXTUAL-CONVENTION defined in SNMPv2-TC (RFC-2579):
  // "A row which is readOnly(5) cannot be changed nor deleted.
  if (fpBridge->Get_snmpCommunityStorageType() == CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_READONLY)
    return SNMP_NOT_WRITABLE;

  // If this row has not yet been created, either reject the set or create the
  // row; either is allowed per RFC-2579.  See note in MibObjs.h for details.
  if ((fpBridge->Get_snmpCommunityStatus() == SNMP_STATUS_INVISIBLE)
  &&  ((fpBridge->fObjectMask & snmpCommunityEntryBridge::kBIT_snmpCommunityStatus) == 0))
  {
#ifdef SUPPORT_INDIRECT_ROW_CREATION
    int CreateStatus = SNMP_STATUS_CREATE_AND_WAIT;

    SnmpLogReq << "Indirect row creation supported, creating snmpCommunityEntry" << endl;
    Status = fpBridge->Set_snmpCommunityStatus (CreateStatus);
    if (Status != SNMP_NO_ERROR)
      return Status;
#else
    SnmpLogReq << "snmpCommunityStatus not set (" << (void *)fpBridge->fObjectMask << "), INCONSISTENT_VALUE" << endl;
    return SNMP_INCONSISTENT_VALUE | SNMP_ROW_NOT_CREATED;
#endif
  }

  switch (Leaf.Leaf())
  {
    case kLEAF_snmpCommunityIndex:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_snmpCommunityName:
      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_snmpCommunityName (String);
      break;

    case kLEAF_snmpCommunitySecurityName:
      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_snmpCommunitySecurityName (String);
      break;

    case kLEAF_snmpCommunityContextEngineID:
      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_snmpCommunityContextEngineID (String);
      break;

    case kLEAF_snmpCommunityContextName:
      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_snmpCommunityContextName (String);
      break;

    case kLEAF_snmpCommunityTransportTag:
      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_snmpCommunityTransportTag (String);
      break;

    case kLEAF_snmpCommunityStorageType:
    {
      int snmpCommunityStorageType;

      Status = VB.GetInt32 (&snmpCommunityStorageType);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_snmpCommunityStorageType (snmpCommunityStorageType);
      break;
    }

    case kLEAF_snmpCommunityStatus:
    {
      int snmpCommunityStatus;

      Status = VB.GetInt32 (&snmpCommunityStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_snmpCommunityStatus (snmpCommunityStatus);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}


/*\ 
 *  snmpTargetAddrExtTable and related objects 
\*/

/* snmpTargetAddrExtTable */

snmpTargetAddrExtTable::snmpTargetAddrExtTable ()
  : BcmMibTable ("1.3.6.1.6.3.18.1.2", "snmpTargetAddrExtTable")
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << snmpTargetAddrExtEntry::kLEAF_snmpTargetAddrTMask);
  fColumnMask |= (1 << snmpTargetAddrExtEntry::kLEAF_snmpTargetAddrMMS);
}

SNMP_STATUS snmpTargetAddrExtTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  snmpTargetAddrExtEntry *pEntry;
  snmpTargetAddrExtEntryBridge *pMyBridge = (snmpTargetAddrExtEntryBridge *) pBridge;


  pEntry = new snmpTargetAddrExtEntry (this, pMyBridge);
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

// This is a weird kind of table, because it is a R/C table with no
// RowStatus object.  The reason is because it extends the more normal
// R/C snmpTargetAddrTable, which controls creation and destruction of
// rows in this table.  However, we might have a case (multi-set PDU or
// in DOCSIS, a cfg file set) where we get a set on a row in this table
// before the corresponding row in the snmpTargetAddrTable is created.
// So we need to provide a NewDefaultEntry method here as well.
BcmMibEntry *snmpTargetAddrExtTable::NewDefaultEntry (BcmObjectId &Index)
{
    if ( fpAgent == NULL )
        return NULL;

    // Only register one of these to an SNMPv3 agent.  Check with the thread
    // to see if my agent is a V3 one.
    BcmSnmpV3Agent *pV3Agent = BcmSnmpThread::Singleton().FindV3Agent(fpAgent);
    if (pV3Agent == NULL)
      return NULL;
      
    snmpTargetAddrExtEntryBridge *pBridge = new snmpTargetAddrExtEntryBridge (Index, pV3Agent);

    if ( pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR )
    {
        SnmpLogInit << "Bridge registration failed!  Deleting bridge object." << endl;
        delete (pBridge);
        return NULL;
    }

    return(BcmMibEntry *)pBridge->fpObject;
}

/* snmpTargetAddrExtEntry */

snmpTargetAddrExtEntry::snmpTargetAddrExtEntry (snmpTargetAddrExtTable *pTable, snmpTargetAddrExtEntryBridge *pBridge)
  : BcmMibEntry (pTable, "snmpTargetAddrExtEntry")
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new StringMibObject ("1.3.6.1.6.3.18.1.2.1.1", "snmpTargetAddrTMask", this, kReadCreate);
  AddObject (pObject);

  pObject = new Int32MibObject ("1.3.6.1.6.3.18.1.2.1.2", "snmpTargetAddrMMS", this, kReadCreate);
  AddObject (pObject);
}

bool snmpTargetAddrExtEntry::IsVisible() const
{
    TargetAddr TAddr;
    BcmObjectId Index;
    BcmString Name;

    GetIndex (Index);
    for ( int Loop = 0; Loop < Index.Length(); Loop++ )
        Name += (char)Index[Loop];

    // hide ourselves if there is not snmpTargetAddrTable with the same name.
    if ( AgentFindTargetAddr (Name.data(), Name.length() ,&TAddr) != SNMP_NO_ERROR )
    {
        SnmpLogReq << "couldn't find snmpTargetAddrTable." << Name << endl;
        return false;
    }

    return true;
}

SNMP_STATUS snmpTargetAddrExtEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_snmpTargetAddrTMask:
    {
      BcmString String;

      fpBridge->Get_snmpTargetAddrTMask (String);
      Status = VB.SetString (String);
      break;
    }

    case kLEAF_snmpTargetAddrMMS:
      Status = VB.SetInt32 (fpBridge->Get_snmpTargetAddrMMS ());
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

SNMP_STATUS snmpTargetAddrExtEntry::TestLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_snmpTargetAddrTMask:
    {
      BcmString String;

      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_snmpTargetAddrTMask (String);
      break;
    }

  }

  return Status;
}

SNMP_STATUS snmpTargetAddrExtEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_snmpTargetAddrTMask:
    {
      BcmString String;

      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_snmpTargetAddrTMask (String);
      break;
    }

    case kLEAF_snmpTargetAddrMMS:
    {
      int snmpTargetAddrMMS;

      Status = VB.GetInt32 (&snmpTargetAddrMMS);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_snmpTargetAddrMMS (snmpTargetAddrMMS);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}

