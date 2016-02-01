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
//    Filename: docsDevNmMib.cpp
//    Creation Date: Wednesday, February 04, 2004 at 15:43:07
//    Created by Broadcom BFC / V2 Mib Compiler BCMIBC.EXE version 3.0.0b
//    Created by Broadcom BFC / V2 Mib Compiler BCMIBC.EXE version 3.0.0a
//
//**************************************************************************

#include "docsDevNmMib.h"
#include "docsDevNmBridge.h"
#include "SnmpLog.h" // for SnmpLog... macros
#include "BcmSnmpAgent.h"
#include "NmAccess.h"

#include "ifBridge.h"
#include "OperatingSystemFactory.h"

/*\ 
 *  docsDevNmAccessTable and related objects 
\*/

/* docsDevNmAccessTable */

docsDevNmAccessTable::docsDevNmAccessTable ()
  : docsDevMibTable ("1.3.6.1.2.1.69.1.2", "docsDevNmAccessTable")
{
  // Initialize column mask
  fColumnMask = 0;
  fColumnMask |= (1 << docsDevNmAccessEntry::kLEAF_docsDevNmAccessIp);
  fColumnMask |= (1 << docsDevNmAccessEntry::kLEAF_docsDevNmAccessIpMask);
  fColumnMask |= (1 << docsDevNmAccessEntry::kLEAF_docsDevNmAccessCommunity);
  fColumnMask |= (1 << docsDevNmAccessEntry::kLEAF_docsDevNmAccessControl);
  fColumnMask |= (1 << docsDevNmAccessEntry::kLEAF_docsDevNmAccessInterfaces);
  fColumnMask |= (1 << docsDevNmAccessEntry::kLEAF_docsDevNmAccessStatus);
  fColumnMask |= (1 << docsDevNmAccessEntry::kLEAF_docsDevNmAccessTrapVersion);
}

SNMP_STATUS docsDevNmAccessTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  docsDevNmAccessEntry *pEntry;
  docsDevNmAccessEntryBridge *pMyBridge = (docsDevNmAccessEntryBridge *) pBridge;


  pEntry = new docsDevNmAccessEntry (this, pMyBridge);
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

docsDevMibEntry *docsDevNmAccessTable::NewDefaultEntry (BcmObjectId &Index)
{
  if (fpAgent == NULL)
    return NULL;

  // Verify index length
  if (Index.Length() != 1)
    return NULL;
    
  if ((Index[0] < MINVAL_DOCSDEVNMACCESSINDEX) 
  ||  (Index[0] > MAXVAL_DOCSDEVNMACCESSINDEX))
    return NULL;

  docsDevNmAccessEntryBridge *pBridge = new docsDevNmAccessEntryBridge (Index, fpAgent);

  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
  {
    SnmpLogInit << "docsDevNmAccessTable::NewDefaultEntry Bridge registration failed!  Deleting bridge object." << endl;
    delete (pBridge);
    return NULL;
  }
  
  return (docsDevMibEntry *)pBridge->fpObject;
}

SNMP_STATUS docsDevNmAccessTable::DestroyEntry (BcmObjectId &Index)
{
  docsDevNmAccessEntry *pEntry = NULL;
  MibEntryIterator Pos;


  SnmpLogInit << "docsDevNmAccessTable::DestroyEntry: destroying entry of index " << Index << endl;

  /* Actually destroy this entry */
  
  if (fpAgent)
    fpAgent->LockMib();

  // Find and remove entry from entry container
  pEntry = (docsDevNmAccessEntry *) FindEntry (Index, Pos);

  if (pEntry == NULL)
  {
    SnmpLogInit << "docsDevNmAccessTable::DestroyEntry failed to find Index " << Index << endl;
    
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

NmAccessController *docsDevNmAccessTable::FirstNmController ()
{
  docsDevNmAccessEntry *pNmEntry;
  
  
  pNmEntry = (docsDevNmAccessEntry *)FirstEntry ();
  
  if (pNmEntry)
    return pNmEntry->GetController();
    
  return NULL;
}

NmAccessController *docsDevNmAccessTable::NextNmController (NmAccessController *pPrevNm)
{ 
  docsDevNmAccessEntry *pNmEntry;
  BcmObjectId Index;
  
  
  if (pPrevNm == NULL)
    return FirstNmController();
    
  Index = (unsigned long)pPrevNm->GetIndex();
  
  pNmEntry = (docsDevNmAccessEntry *) NextEntry (Index);
  
  if (pNmEntry)
    return pNmEntry->GetController();
    
  return NULL;
}

SNMP_STATUS docsDevNmAccessTable::ValidateRequest (int RequestType, int SrcIfNum, const BcmIpAddress &SrcIp, const BcmString &Community)
{
  NmAccessController *pController;
  SNMP_STATUS Status = SNMP_NO_ERROR, TempStatus;

  fpAgent->LockMib();

  pController = FirstNmController();
  
  // If the table is empty, FULL ACCESS TO EVERYONE!
  if (pController == NULL)
  {
/*  
    // Note: if we are in pure DOCSIS 1.0 mode, expect 'public' or 'private'.
    // In DOCSIS 1.1 or 1.1 modem in 1.0 mode, accept any community string,
    // even an empty one.
    if (GetDocsisMode() == DOCSIS_MODE_PURE_1_0)
    {
      SnmpLogNm << "  NmAccess table is empty, but we're in pure 1.0 mode: community = " << Community << " PDU type = " << RequestType << endl;
      
      if ((Community == "public") || (Community == "private"))
        Status = SNMP_BAD_COMMUNITY_USE;
      else
        Status = SNMP_BAD_COMMUNITY_NAME;
      
      switch (RequestType)
      {
        case PDU_GET_REQUEST:
        case PDU_GET_NEXT_REQUEST:
        case PDU_GET_BULK_REQUEST:
          if ((Community == "public") || (Community == "private"))
          {
            SnmpLogNm << "  GET/NEXT/BULK: matched 'public' or 'private', NO_ERROR" << endl;
            Status = SNMP_NO_ERROR;
          }
          break;
        
        case PDU_SET_REQUEST:
          if (Community == "private")
          {
            SnmpLogNm << "  SET: matched 'private', NO_ERROR" << endl;
            Status = SNMP_NO_ERROR;
          }
          break;
    
        default:
          SnmpLogNm << "  unknown PDU type " << RequestType << endl;
          break;
      }
    }
    else
    {
      SnmpLogNm << "  NM Access unrestricted, table is empty" << endl;
      Status = SNMP_NO_ERROR;
    }
*/
  }
  else
    SnmpLogNm << "  NM access mode, entries in the table." << endl;
  
  while (pController)
  {
    TempStatus = pController->ValidateRequest (RequestType, SrcIfNum, SrcIp, Community);
    
    // ValidateRequest can return one of the following:
    // NO_ERROR: matched the entry on all counts
    // NO_OPINION: inactive entry
    // BAD_COMMUNITY_NAME: failed to match community name
    // BAD_COMMUNITY_USE: matched community name, but failed on access or IP
    
    if (TempStatus == SNMP_NO_ERROR)
    {
      SnmpLogNm << "  NM Access granted!" << endl;
      Status = SNMP_NO_ERROR;
      break;
    }
    
    // If any entry matches the community name but not something else,
    // then eventually we will either return NO_ERROR or BAD_COMMUNITY_USE.
    else if (TempStatus == SNMP_BAD_COMMUNITY_USE)
      Status = SNMP_BAD_COMMUNITY_USE;
      
    // If we get BAD_COMMUNITY_NAME, don't replace a current status of 
    // BAD_COMMUNITY_USE with it.  But if our current status is NO_ERROR,
    // then do replace it.  Note that I check for != NO_OPINION here instead
    // of == BAD_COMMUNITY_NAME just in case we add other return codes
    // at some time in the future.
    else if ((TempStatus != SNMP_NO_OPINION)
    &&       (Status != SNMP_BAD_COMMUNITY_USE))
      Status = TempStatus;
    
    SnmpLogNm << "  No match on this entry." << endl;
      
    pController = NextNmController (pController);
  }

  fpAgent->UnlockMib();
  
  return Status;
}

int docsDevNmAccessTable::SendTrap (BcmTrap *pTrap, const BcmIpAddress *pSrcIp)
{
  NmAccessController *pNm;
  SNMP_STATUS NmStatus, Status = SNMP_GEN_ERR;
  int NumTraps = 0;
  BcmIpAddress SrcIp;
  
  
  if (pSrcIp)
    SrcIp = *pSrcIp;
  else
    SrcIp = fpAgent->GetIpAddr();
  
  pNm = FirstNmController ();
  while (pNm)
  {
    if ((pNm->GetControl() == CONSTVAL_DOCSDEVNMACCESSCONTROL_ROWITHTRAPS)
    ||  (pNm->GetControl() == CONSTVAL_DOCSDEVNMACCESSCONTROL_RWWITHTRAPS)
    ||  (pNm->GetControl() == CONSTVAL_DOCSDEVNMACCESSCONTROL_TRAPSONLY))
    {
      SnmpLogEv << "  NM # " << pNm->GetIndex() << " has trap access." << endl;
      
      if (pNm->GetStatus() == CONSTVAL_DOCSDEVNMACCESSSTATUS_ACTIVE)
      {
        SnmpLogEv << "  NM is active" << endl;
        
        // Only send traps if the DestMask is 255.255.255.255, per
        // RFC 2669.
        if (pNm->GetIpMask() == kBroadcastIp)
        {
          SnmpLogEv << "  Mask is 255.255.255.255" << endl;
          
          // NOTE: OSS-IO3 section 3.6 requires all traps have community string
          // public (v1 & v2 traps, that is):
          // (2) Trap Type: The community String for traps in SNMP V1 and V2 packets 
          // SHALL be "public". 
          if (pNm->GetTrapType() == CONSTVAL_DOCSDEVNMACCESSTRAPVERSION_DISABLESNMPV2TRAP)
          {
            NmStatus = pTrap->SendV1Trap (SrcIp, pNm->GetIpAddr(), pNm->GetCommunity(), fpAgent->GetUdpTrapPort());
            if (NmStatus != SNMP_NO_ERROR)
              SnmpLogEv << "   SendV1Trap failed w/ error code " << NmStatus << endl;
            else
              NumTraps++;
          }
            
          else
          {
            NmStatus = pTrap->SendV2Trap (SrcIp, pNm->GetIpAddr(), pNm->GetCommunity(), fpAgent->GetUdpTrapPort());
            if (NmStatus != SNMP_NO_ERROR)
              SnmpLogEv << "   SendV2Trap failed w/ error code " << NmStatus << endl;
            else
              NumTraps++;
          }
          
          if (NmStatus == SNMP_NO_ERROR)
          {
            SnmpLogEv << "  Sent OK" << endl;
            Status = NmStatus;
          }
        }
      }
    }
    
    pNm = NextNmController (pNm);
  }
  
  return NumTraps;
}

/* docsDevNmAccessEntry */

docsDevNmAccessEntry::docsDevNmAccessEntry (docsDevNmAccessTable *pTable, docsDevNmAccessEntryBridge *pBridge)
  : docsDevMibEntry (pTable, "docsDevNmAccessEntry")
{
  BcmMibObject *pObject;
  BcmObjectId   Index;


  fpNmTable = pTable;

  fpBridge = pBridge;
  fpBridge->GetIndex (Index);
  SetIndex (Index);

  pObject = new Int32MibObject ("1.3.6.1.2.1.69.1.2.1.1", "docsDevNmAccessIndex", this, kNotAccessible);
  AddObject (pObject);

  pObject = new IpAddrMibObject ("1.3.6.1.2.1.69.1.2.1.2", "docsDevNmAccessIp", this, kReadCreate);
  AddObject (pObject);

  pObject = new IpAddrMibObject ("1.3.6.1.2.1.69.1.2.1.3", "docsDevNmAccessIpMask", this, kReadCreate);
  AddObject (pObject);

  pObject = new StringMibObject ("1.3.6.1.2.1.69.1.2.1.4", "docsDevNmAccessCommunity", this, kReadCreate);
  AddObject (pObject);

  pObject = new Int32MibObject ("1.3.6.1.2.1.69.1.2.1.5", "docsDevNmAccessControl", this, kReadCreate);
  AddObject (pObject);

  pObject = new BitsMibObject ("1.3.6.1.2.1.69.1.2.1.6", "docsDevNmAccessInterfaces", this, kReadCreate);
  AddObject (pObject);

  pObject = new RowStatusMibObject (pTable, this, "1.3.6.1.2.1.69.1.2.1.7", "docsDevNmAccessStatus", kReadCreate);
  AddObject (pObject);

  pObject = new Int32MibObject ("1.3.6.1.2.1.69.1.2.1.8", "docsDevNmAccessTrapVersion", this, kReadCreate);
  AddObject (pObject);
}

// Special print for docsDevNmAccessEntry so we can see the damn
// community string at least to the console!
ostream &docsDevNmAccessEntry::Print(ostream &outStream)
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  MibObjectIterator Pos;
  BcmMibObject *pObject;
  BcmObjectId Index;


  GetIndex (Index);

  outStream << "----------------------------------" << endl;

  if (fpAgent)
    outStream << Name() << "." << Index << /*" agent='" << fpAgent->GetName() << "'" << */ endl;
  else
    outStream << Name() << "." << Index << " agent=NULL" << endl;

  if (fpStatusObject)
    outStream << "ROW STATUS object is " << fpStatusObject->Name() << ", creation method = " << fpStatusObject->CreationMethod() << endl;

  outStream << "----------------------------------" << endl;

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

    // Here's the special part...    
    if (pObject->Name() == "docsDevNmAccessCommunity")
      outStream << "  COMMUNITY STRING EXPOSED: '" << fpBridge->fpController->GetCommunity() << "'" << endl;

    Pos++;
  }
#endif

  return outStream;
}

bool docsDevNmAccessEntry::IsAccessible (BcmMibRequest *pReq) const
{
  NmAccessController *pNm = fpNmTable->FirstNmController ();
  int ActiveEntries = 0;
  
  
  // First, check for plain old docsDevMibEntry accessibility
  if (docsDevMibEntry::IsAccessible (pReq) == false)
    return false;
  
  // If no pReq is specified, then we allow access to EVERYBODY!
  if (pReq == NULL)
    return true;
    
  // Special case: allow any SET request access.  This might seem odd, but
  // it's OK because the TEST request precedes it and we can reject the
  // TEST.  Problem is, if we don't assume that the access restrictions
  // were properly applied by the TESTs, then re-apply them on SET, we
  // can start having problems with a multi-set PDU creating entries in this
  // table and running into access problems as rows are created.
  if (pReq->fType == BcmMibRequest::kSetRequest)
    return true;
  
  // We must not allow access to this table AT ALL (read or write) if the
  // station making the request has read-only access.
  while (pNm)
  {
    if (pNm->GetStatus() == SNMP_STATUS_ACTIVE)
    {
      ActiveEntries++;
      
      // Does the manager requesting this info match this NM controller?
      if ((pNm->ValidateReqAddr (pReq->fSrcIp) == SNMP_NO_ERROR)
      // Don't check community here.  If any entry with read-write permission
      // matches the IP address, that's good enough.  Per CW17 failure from
      // CL & subsequent PR #580
      // 9/28/01: Now CL will fail you the opposite of the CW17 failure.
      // CW19 sez look at the community string after all.  PR #1032
      &&  (pNm->ValidateReqCommunity (pReq->fCmty) == SNMP_NO_ERROR))
      {
        if ((pNm->GetControl() == CONSTVAL_DOCSDEVNMACCESSCONTROL_RWWITHTRAPS)
        ||  (pNm->GetControl() == CONSTVAL_DOCSDEVNMACCESSCONTROL_READWRITE))
          return true;
      }
    }
    
    pNm = fpNmTable->NextNmController (pNm);
  }
  
  if (ActiveEntries)
    return false;
    
  return true;
}

SNMP_STATUS docsDevNmAccessEntry::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmIpAddress Ip;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_docsDevNmAccessIndex: // not-accessible
      Status = VB.SetInt32 (fpBridge->Get_docsDevNmAccessIndex ());
      break;

    case kLEAF_docsDevNmAccessIp:
      fpBridge->Get_docsDevNmAccessIp (Ip);
      Status = VB.SetIp (Ip);
      break;

    case kLEAF_docsDevNmAccessIpMask:
      fpBridge->Get_docsDevNmAccessIpMask (Ip);
      Status = VB.SetIp (Ip);
      break;

    case kLEAF_docsDevNmAccessCommunity:
      fpBridge->Get_docsDevNmAccessCommunity (String);
      Status = VB.SetString (String);
      break;

    case kLEAF_docsDevNmAccessControl:
      Status = VB.SetInt32 (fpBridge->Get_docsDevNmAccessControl ());
      break;

    case kLEAF_docsDevNmAccessInterfaces:
    {
      int NumOctets = ((fpAgent->IfBridge().Get_ifNumber()-1)/8)+1;
      
      Status = VB.SetBits (fpBridge->Get_docsDevNmAccessInterfaces(), NumOctets);
      break;
    }

    case kLEAF_docsDevNmAccessStatus:
      Status = VB.SetInt32 (fpBridge->Get_docsDevNmAccessStatus ());
      break;

    case kLEAF_docsDevNmAccessTrapVersion:
      Status = VB.SetInt32 (fpBridge->Get_docsDevNmAccessTrapVersion ());
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

SNMP_STATUS docsDevNmAccessEntry::TestLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmIpAddress Ip;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  switch (Leaf.Leaf())
  {
    case kLEAF_docsDevNmAccessIndex:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_docsDevNmAccessIp:
      Status = VB.GetIp (Ip);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_docsDevNmAccessIp (Ip);
      break;

    case kLEAF_docsDevNmAccessIpMask:
      Status = VB.GetIp (Ip);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_docsDevNmAccessIpMask (Ip);
      break;

    case kLEAF_docsDevNmAccessCommunity:
      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_docsDevNmAccessCommunity (String);
      break;

    case kLEAF_docsDevNmAccessControl:
    {
      int docsDevNmAccessControl;

      Status = VB.GetInt32 (&docsDevNmAccessControl);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_docsDevNmAccessControl (docsDevNmAccessControl);
      break;
    }

    case kLEAF_docsDevNmAccessInterfaces:
    {
      unsigned long docsDevNmAccessInterfaces;
      int SetOctets = 0;

      Status = VB.GetBits (&docsDevNmAccessInterfaces, &SetOctets);
/*
      // KO 9/26/02: back this weird behaviour out, we got noted for it in
      // CW23.  Guess someone at CL snapped to the fact that this ECR never
      // made it.
        
      // Below is a new wrinkle that was originally required in OSS-R-01108.
      // This ECR was allowed to expire without becoming ECN, but nevertheless
      // its requirements were rolled into OSS-07.1.  So we will code to the
      // test here.  The requirement kicks in if the value written to 
      // docsDevNmAccessInterfaces is shorter than necessary: ie if we have
      // 9-16 interfaces, and the value specified is only 8 bits long.  In that
      // case, we must 1-fill rather than 0-fill the extra bits.  So, for
      // example, if we get a set to 0xC0, we should mangle this to 0xC0FF.
      // Added 4/4/2002
      //
      // Even more bizarre: we must pad with whatever the least significant
      // bit of the last octet is.  So we 0-pad 0xC0 to 0xC000, but 1-pad
      // 0xC1 to 0xC1FF.  Whatever.  Added 4/9/2002.
      //
      // Note that the logic below is somewhat obscured by the fact that
      // we must operate on the traditional bitmask value rather than the
      // SNMP bitmask value, so don't get confused...
      int ReqdOctets = ((fpAgent->IfBridge().Get_ifNumber()-1)/8)+1;
      
      if (docsDevNmAccessInterfaces & (0x80 << ((SetOctets-1)*8) ))
      {
        for (int Loop = SetOctets; Loop < ReqdOctets; Loop++)
          docsDevNmAccessInterfaces |= (0xFF << (8*Loop));
      }
*/      
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_docsDevNmAccessInterfaces (docsDevNmAccessInterfaces);
      break;
    }

    case kLEAF_docsDevNmAccessStatus:
    {
      int docsDevNmAccessStatus;

      Status = VB.GetInt32 (&docsDevNmAccessStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_docsDevNmAccessStatus (docsDevNmAccessStatus);
      break;
    }

    case kLEAF_docsDevNmAccessTrapVersion:
    {
      int docsDevNmAccessTrapVersion;

      Status = VB.GetInt32 (&docsDevNmAccessTrapVersion);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Test_docsDevNmAccessTrapVersion (docsDevNmAccessTrapVersion);
      break;
    }
  }

  return Status;
}

SNMP_STATUS docsDevNmAccessEntry::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmIpAddress Ip;
  BcmString String;


  if (fpBridge == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  // If this row has not yet been created, either reject the set or create the
  // row; either is allowed per RFC-1903.  See note in MibObjs.h for details.
  if ((fpBridge->Get_docsDevNmAccessStatus() == SNMP_STATUS_INVISIBLE)
  &&  ((fpBridge->fObjectMask & docsDevNmAccessEntryBridge::kBIT_docsDevNmAccessStatus) == 0))
  {
#ifdef SUPPORT_INDIRECT_ROW_CREATION
    int CreateStatus = SNMP_STATUS_CREATE_AND_WAIT;

    SnmpLogReq << "Indirect row creation supported, creating docsDevNmAccessEntry" << endl;
    Status = fpBridge->Set_docsDevNmAccessStatus (CreateStatus);
    if (Status != SNMP_NO_ERROR)
      return Status;
#else
    SnmpLogReq << "docsDevNmAccessStatus not set (" << (void *)fpBridge->fObjectMask << "), INCONSISTENT_VALUE" << endl;
    return SNMP_INCONSISTENT_VALUE | SNMP_ROW_NOT_CREATED;
#endif
  }

  switch (Leaf.Leaf())
  {
    case kLEAF_docsDevNmAccessIndex:
      Status = SNMP_NO_ACCESS; // not-accessible
      break;

    case kLEAF_docsDevNmAccessIp:
      Status = VB.GetIp (Ip);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevNmAccessIp (Ip);
      break;

    case kLEAF_docsDevNmAccessIpMask:
      Status = VB.GetIp (Ip);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevNmAccessIpMask (Ip);
      break;

    case kLEAF_docsDevNmAccessCommunity:
      Status = VB.GetString (String);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevNmAccessCommunity (String);
      break;

    case kLEAF_docsDevNmAccessControl:
    {
      int docsDevNmAccessControl;

      Status = VB.GetInt32 (&docsDevNmAccessControl);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevNmAccessControl (docsDevNmAccessControl);
      break;
    }

    case kLEAF_docsDevNmAccessInterfaces:
    {
      unsigned long docsDevNmAccessInterfaces;
      int SetOctets = 0, ReqdOctets = ((fpAgent->IfBridge().Get_ifNumber()-1)/8)+1;

      Status = VB.GetBits (&docsDevNmAccessInterfaces, &SetOctets);
      
      // Below is a new wrinkle that was originally required in OSS-R-01108.
      // This ECR was allowed to expire without becoming ECN, but nevertheless
      // its requirements were rolled into OSS-07.1.  So we will code to the
      // test here.  The requirement kicks in if the value written to 
      // docsDevNmAccessInterfaces is shorter than necessary: ie if we have
      // 9-16 interfaces, and the value specified is only 8 bits long.  In that
      // case, we must 1-fill rather than 0-fill the extra bits.  So, for
      // example, if we get a set to 0xC0, we should mangle this to 0xC0FF.
      // Added 4/4/2002
      //
      // Even more bizarre: we must pad with whatever the least significant
      // bit of the last octet is.  So we 0-pad 0xC0 to 0xC000, but 1-pad
      // 0xC1 to 0xC1FF.  Whatever.  Added 4/9/2002.
      //
      // Note that the logic below is somewhat obscured by the fact that
      // we must operate on the traditional bitmask value rather than the
      // SNMP bitmask value, so don't get confused...
      if (docsDevNmAccessInterfaces & (0x80 << ((SetOctets-1)*8) ))
      {
        for (int Loop = SetOctets; Loop < ReqdOctets; Loop++)
          docsDevNmAccessInterfaces |= (0xFF << (8*Loop));
      }
      
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevNmAccessInterfaces (docsDevNmAccessInterfaces);
      break;
    }

    case kLEAF_docsDevNmAccessStatus:
    {
      int docsDevNmAccessStatus;

      Status = VB.GetInt32 (&docsDevNmAccessStatus);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevNmAccessStatus (docsDevNmAccessStatus);
      break;
    }

    case kLEAF_docsDevNmAccessTrapVersion:
    {
      int docsDevNmAccessTrapVersion;

      Status = VB.GetInt32 (&docsDevNmAccessTrapVersion);
      if (Status == SNMP_NO_ERROR)
        Status = fpBridge->Set_docsDevNmAccessTrapVersion (docsDevNmAccessTrapVersion);
      break;
    }

    default:
      Status = SNMP_NO_SUCH_NAME;
      break;
  }

  return Status;
}

NmAccessController *docsDevNmAccessEntry::GetController () const
{
  return fpBridge->fpController;
}
