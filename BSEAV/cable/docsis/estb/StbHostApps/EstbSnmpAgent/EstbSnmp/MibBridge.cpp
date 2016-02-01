//**************************************************************************
//
//    Copyright 2000 Broadcom Corporation
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
//    Filename: BcmMibBridge.cpp
//    Author:   Kevin O'Neal
//    Creation Date: 13-Mar-2000
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Base classes for MIB bridge objects.  Bridge objects are used to 
//    connect the SNMP subsystem with the rest of the application.
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include "MibObjs.h"
#include "MibBridge.h"
#include "BcmSnmpAgent.h"
#include "SnmpLog.h"

/**************************************************************************\
**                                                                        **
*     EXPLANATION OF BcmMibBridge::Register(CleanupMethod Cleanup)         *
*                     Mainly, what does Cleanup do?                        *
*                                                                          *
*  The Cleanup argument tells the bridge object how it is                  *
*  to be deleted.  This argument is mainly of interest to tabular bridges, *
*  which might have entries created and destroyed dynamically.  Scalar     *
*  objects generally don't come and go that way, but are usually created   *
*  at startup and stay there forever, so for scalars it probably doesn't   *
*  really matter which argument you use.                                   *
*                                                                          *
*  If you want to carry around a pointer to the new bridge object and      *
*  have the associated MIB object destroyed when the bridge object is      *
*  deleted, use Cleanup = kDeleteBridge.  In that case, when you           *
*  delete the bridge object, the corresponding MIB object or table entry   *
*  will be destroyed as well.  Note that if you use this approach, a call  *
*  to BcmMibTable::DestroyEntry will NOT delete the bridge object          *
*  associated with the entry, so destruction of the entry via that call    *
*  will generally result in a memory leak if Register(kDestroyMibObject)   *
*  is used.  However, deleting the bridge object will result in the        *
*  corresponding MIB entry being properly destroyed and will not cause a   *
*  memory leak.                                                            *
*                                                                          *
*  Alternately, you can set Cleanup = kDestroyMibObject, and in that       *
*  case the bridge object will be deleted when the corresponding MIB       *
*  object is destroyed.  That way, you can just pretty much forget about   *
*  the bridge object once it's created.  This is the setting to use any    *
*  time that a MIB object might go away without your knowing about it,     *
*  for example a read-create kind of table.  If a user comes along and     *
*  deletes a MIB entry by setting its status to DESTROY, then you want     *
*  to make sure the bridge object goes away at that point as well.  If     *
*  you choose this approach, remember that to programatically destroy the  *
*  bridge object you must use the BcmMibTable::DestroyEntry method, you    *
*  can't just delete the bridge object directly or else you'll invalidate  *
*  the MIB entry's fpBridge pointer and likely cause a crash down the      *
*  road.                                                                   *
*                                                                          *
*                                 EXAMPLES                                 *
*                                 ========                                 *
*                                                                          *
*  Proper use of BcmMibBridge::Register (kDeleteBridge)                    *
*  -------------------------------------------                             *
*  SNMP_STATUS CreateAndDestroyWithRegisterFalse ()                        *
*  {                                                                       *
*    exampleTabularBridge *pBridge;                                        *
*                                                                          *
*    // Create the bridge object, use index 1.  Note that this is          *
*    // assuming that exampleTabularBridge has been defined as a           *
*    // derived object of TabularMibBridge, and that the derived           *
*    // object knows the OID it's associated with.                         *
*    pBridge = new exampleTabularBridge (1, &CmSnmpAgent::Singleton());    *
*                                                                          *
*    // If Register fails (only if it can't match the bridge's OID to a    *
*    // known OID) then go ahead and delete the bridge object.             *
*    if (pBridge->Register(kDeleteBridge) != SNMP_NO_ERROR)                *
*    {                                                                     *
*      delete pBridge;                                                     *
*      return SNMP_NO_SUCH_NAME;                                           *
*    }                                                                     *
*                                                                          *
*    // Now delete it, the table entry will go away too.  Easy greasy!     *
*    // If for some reason the table entry goes away, you can still        *
*    // safely delete the pBridge pointer because the entry's destruction  *
*    // will not delete the bridge.  However, you'll be on the hook for    *
*    // deleting the bridge for that same reason, and meantime the bridge  *
*    // object will be sort of floating around not hooked to anything, so  *
*    // it'll be kind of useless.                                          *
*    delete pBridge;                                                       *
*    return SNMP_NO_ERROR;                                                 *
*  }                                                                       *
*                                                                          *
*  Proper use of BcmMibBridge::Register(kDestroyMibObject)                 *
*  -------------------------------------------                             *
*  SNMP_STATUS CreateAndDestroyWithRegisterTrue ()                         *
*  {                                                                       *
*    exampleTabularBridge *pBridge;                                        *
*                                                                          *
*    // Create the bridge object, use index 1.  Note that this is          *
*    // assuming that exampleTabularBridge has been defined as a           *
*    // derived object of TabularMibBridge, and that the derived           *
*    // object knows the OID it's associated with.                         *
*    pBridge = new exampleTabularBridge (1, &CmSnmpAgent::Singleton());    *
*                                                                          *
*    // If Register fails (only if it can't match the bridge's OID to a    *
*    // known OID) then go ahead and delete the bridge object.             *
*    if (pBridge->Register(kDestroyMibObject) != SNMP_NO_ERROR)            *
*    {                                                                     *
*      delete pBridge;                                                     *
*      return SNMP_NO_SUCH_NAME;                                           *
*    }                                                                     *
*                                                                          *
*    // This time, to delete the object we must call                       *
*    // BcmMibTable::DestroyEntry to destroy the entry and bridge.         *
*    // Assume the name of the table is "sampleTable"                      *
*    BcmMibTable *pTable;                                                  *
*    pTable = CmSnmpAgent::Singleton().FindTable ("sampleTable");          *
*    if (pTable == NULL)                                                   *
*    {                                                                     *
*      gLogMessageRaw << "Couldn't find table sampleTable!\n"              *
*                        "MEMORY LEAK! BAD!" << endl;                      *
*      return SNMP_NO_SUCH_NAME;                                           *
*    }                                                                     *
*                                                                          *
*    // Destroy entry with index 1.  This'll take the bridge object        *
*    // along with it.                                                     *
*    pTable->DestroyEntry(BcmObjectId (1));                                *
*                                                                          *
*    return SNMP_NO_ERROR;                                                 *
*  }                                                                       *
*                                                                          *
**                                                                        **
\**************************************************************************/


/*\
 *  BcmMibBridge
\*/

BcmMibBridge::BcmMibBridge (const char *pOidStr, BcmSnmpAgent *pAgent)
{
  fpAgent = pAgent;
  fpObject = NULL;
  
  fpOidString = pOidStr;
  
  SnmpLogMem << "BcmMibBridge::BcmMibBridge " << fpOidString << endl;
}

BcmMibBridge::~BcmMibBridge()
{
  SnmpLogMem << "BcmMibBridge::~BcmMibBridge " << fpOidString << endl;
  
  DeRegister ();
  
  // If the object isn't supposed to destroy us, then we're supposed
  // to destroy him.
  if (fpObject)
  {
    if (fpObject->fDestroyBridge == false)
      delete fpObject;
  }
}

SNMP_STATUS BcmMibBridge::Register(CleanupMethod Cleanup, bool SilentFailure)
{
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmMibBridge::DeRegister()
{
  return SNMP_NO_ERROR;
}

void BcmMibBridge::GetOid(BcmObjectId &Oid) const
{
  if (fpOidString)
    Oid = fpOidString;
  else
    Oid = "";
}

void BcmMibBridge::GetIndex (BcmObjectId &Index) const
{
  // By default, use .0 as index.  This is overridden in
  // TabularMibBridge.
  Index = "0";
}

int BcmMibBridge::BoolToTruth (bool BoolVal) const
{
  if (BoolVal == true)
    return SNMP_TRUTH_TRUE;
  else
    return SNMP_TRUTH_FALSE;
}

bool BcmMibBridge::TruthToBool (int TruthVal) const
{
  if (TruthVal == SNMP_TRUTH_TRUE)
    return true;
  else
    return false;
}

BcmMibRequest *BcmMibBridge::LastRequestInfo () const
{
  if (fpObject)
    return fpObject->LastRequestInfo ();

  return NULL;
}



/*\
 *  ScalarMibBridge
\*/

ScalarMibBridge::ScalarMibBridge (const char *pOidStr, BcmSnmpAgent *pAgent)
  : BcmMibBridge(pOidStr, pAgent)
{
  SnmpLogMem << "ScalarMibBridge::ScalarMibBridge" << endl;
}


ScalarMibBridge::~ScalarMibBridge()
{
  SnmpLogMem << "ScalarMibBridge::~ScalarMibBridge" << endl;
  DeRegister ();
}

SNMP_STATUS ScalarMibBridge::Register (CleanupMethod Cleanup, bool SilentFailure)
{
  SNMP_STATUS Status;
  
  
  SnmpLogInit << "===> REGISTERING ScalarMibBridge " << fpOidString << " to agent " << fpAgent->GetName() << endl;

  if (Cleanup == kDeleteBridge)
    Status = fpAgent->RegisterScalar(this, false);
  else if (Cleanup == kDestroyMibObject)
    Status = fpAgent->RegisterScalar(this, true);
  else
    return SNMP_WRONG_VALUE;
    
  if (SilentFailure == false)
  {
    if (Status != SNMP_NO_ERROR)
      gLogMessageRaw << "===> WARNING: Failed to register ScalarMibBridge " << fpOidString << " to agent " << fpAgent->GetName() << endl;
    if (Status == SNMP_RESOURCE_UNAVAILABLE)
      gLogMessageRaw << "===> Object already bridged." << endl;
    else if (Status == SNMP_NO_SUCH_NAME)
        gLogMessageRaw << "===> Object not found on agent." << endl;
  }
    
  return Status;
}

SNMP_STATUS ScalarMibBridge::DeRegister ()
{
  SnmpLogInit << "===> DEREGISTERING ScalarMibBridge " << fpOidString << " to agent " << fpAgent->GetName() << endl;

  // Only de-register from our agent if we have successfully registered
  // previously, indicated by fpObject != NULL.  Otherwise, we can get
  // into trouble when we have a rejected attempt to re-register due to
  // programmer error.
  if ((fpAgent) && (fpObject))
    return fpAgent->DeRegisterScalar(this);
  else
    return SNMP_NO_ERROR;
}


/*\
 *  TabularMibBridge
\*/

TabularMibBridge::TabularMibBridge (const char *pOidStr, const char *pIndexStr, BcmSnmpAgent *pAgent)
  : BcmMibBridge(pOidStr, pAgent)
{
  fpIndex = NULL;
  SetIndex (BcmObjectId(pIndexStr));
  fpObject = NULL;
  fObjectMask = 0;
  fRequiredObjects = 0;
  
  SnmpLogMem << "TabularMibBridge::TabularMibBridge(1), index = " << pIndexStr << endl;
}

TabularMibBridge::TabularMibBridge (const char *pOidStr, int Index, BcmSnmpAgent *pAgent)
  : BcmMibBridge(pOidStr, pAgent)
{
  fpIndex = NULL;
  SetIndex (BcmObjectId(Index));
  fpObject = NULL;
  fObjectMask = 0;
  fRequiredObjects = 0;
  
  SnmpLogMem << "TabularMibBridge::TabularMibBridge(2), index = " << Index << endl;
}

TabularMibBridge::TabularMibBridge (const char *pOidStr, const BcmObjectId &Index, BcmSnmpAgent *pAgent)
  : BcmMibBridge(pOidStr, pAgent)
{
  fpIndex = NULL;
  SetIndex (Index);
  fpObject = NULL;
  fObjectMask = 0;
  fRequiredObjects = 0;
  
  SnmpLogMem << "TabularMibBridge::TabularMibBridge(2), index = " << Index << endl;
}

TabularMibBridge::TabularMibBridge (const char *pOidStr, BcmSnmpAgent *pAgent)
  : BcmMibBridge(pOidStr, pAgent)
{
  fpIndex = NULL;
  SetIndex (BcmObjectId((unsigned long)0));
  fpObject = NULL;
  fObjectMask = 0;
  fRequiredObjects = 0;
  
  SnmpLogMem << "TabularMibBridge::TabularMibBridge(3), index = 0" << endl;
}

TabularMibBridge::~TabularMibBridge()
{
  DeRegister ();
  
  if (fpIndex)
  {
    delete fpIndex;
    fpIndex = NULL;
  }
}

SNMP_STATUS TabularMibBridge::Register (CleanupMethod Cleanup, bool SilentFailure)
{
  SNMP_STATUS Status;

  SnmpLogInit << "===> REGISTERING TabularMibBridge " << fpOidString << " to agent " << fpAgent->GetName() << endl;
 
  if (Cleanup == kDeleteBridge)
    Status = fpAgent->RegisterTabular(this, false);
  else if (Cleanup == kDestroyMibObject)
    Status = fpAgent->RegisterTabular(this, true);
  else
    return SNMP_WRONG_VALUE;
    
  if (Status != SNMP_NO_ERROR)
  {
    if (SilentFailure == false)
    {
      gLogMessageRaw << "===> WARNING: Failed to register TabularMibBridge " << fpOidString << " to agent " << fpAgent->GetName() << endl;
      if (Status == SNMP_RESOURCE_UNAVAILABLE)
        gLogMessageRaw << "===> Object already bridged." << endl;
      else if (Status == SNMP_DUPLICATE_VALUE)
        gLogMessageRaw << "===> Entry already exists." << endl;
      else if (Status == SNMP_NO_SUCH_NAME)
        gLogMessageRaw << "===> Object not found on agent." << endl;
    }
  }
  else if (fpObject)
  {
    // We can save a bit of RAM here by 86ing our fpIndex.  We can get the
    // index from our object if needed.
    if (fpIndex)
    {
      delete fpIndex;
      fpIndex = NULL;
    }
  }
    
  return Status;
}

SNMP_STATUS TabularMibBridge::DeRegister ()
{
  SnmpLogInit << "===> DEREGISTERING TabularMibBridge " << fpOidString << " from agent " << fpAgent->GetName() << endl;
  
  if (fpObject)
  {
    // Restore our index now.  Might need it some day.
    BcmObjectId Index;
    fpObject->GetIndex (Index);
    SetIndex (Index);
  }
  
  // Only de-register from our agent if we have successfully registered
  // previously, indicated by fpObject != NULL.  Otherwise, we can get
  // into trouble when we have a rejected attempt to re-register due to
  // programmer error.
  if ((fpAgent) && (fpObject))
    return fpAgent->DeRegisterTabular(this);
  else
    return SNMP_NO_ERROR;
}

void TabularMibBridge::GetIndex (BcmObjectId &Index) const
{
  if (fpIndex)
    Index = *fpIndex;
  else if (fpObject)
    fpObject->GetIndex (Index);
  else
    Index = "";
}

SNMP_STATUS TabularMibBridge::SetIndex (const BcmObjectId &Index)
{
  if (fpIndex == NULL)
    fpIndex = new BcmObjectId (Index);
  else
    *fpIndex = Index;
  
  return SNMP_NO_ERROR;
}

int TabularMibBridge::SetCreationMethod (int CreationMethod)
{
  BcmMibEntry *pEntry = (BcmMibEntry *) fpObject;
  RowStatusMibObject *pRS = NULL;
  
  
  if (pEntry)
    pRS = pEntry->GetStatusObject ();
  
  if (pRS)
  {
    pRS->fHowCreated = CreationMethod;
    return pRS->fHowCreated;
  }
  
  return -1;
}

SNMP_STATUS TabularMibBridge::SetRowStatus (int &RowStatus)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;

  // If all required objects have not been set, then createAndGo and
  // Active are not allowed; createAndWait will result in notReady.
  if ((fObjectMask & fRequiredObjects) != fRequiredObjects)
  {
    if (RowStatus == SNMP_STATUS_CREATE_AND_GO)
    {
      SnmpLogReq << "All req'd objects not set on createAndGo. (" << (void *)fObjectMask <<")  INCONSISTENT_VALUE" << endl;
      Status = SNMP_INCONSISTENT_VALUE | SNMP_ROW_NOT_CREATED;
    }
    else if (RowStatus == SNMP_STATUS_ACTIVE)
    {
      SnmpLogReq << "All req'd objects not set on active. (" << (void *)fObjectMask <<")  INCONSISTENT_VALUE" << endl;
      Status = SNMP_INCONSISTENT_VALUE;
    }
    else if (RowStatus == SNMP_STATUS_CREATE_AND_WAIT)
    {
      SnmpLogReq << "CREATE_AND_WAIT, not all objects: NOT_READY" << endl;
      RowStatus = SNMP_STATUS_NOT_READY;
    }
  }
  // If all required objects have been set, the createAndWait results
  // in notInService...
  else if (RowStatus == SNMP_STATUS_CREATE_AND_WAIT)
  {
    SnmpLogReq << "CREATE_AND_WAIT, all objects: NOT_IN_SERVICE" << endl;
    RowStatus = SNMP_STATUS_NOT_IN_SERVICE;
  }
  // ...createAndGo results in Active
  else if (RowStatus == SNMP_STATUS_CREATE_AND_GO)
  {
    SnmpLogReq << "CREATE_AND_GO, all objects: ACTIVE" << endl;
    RowStatus = SNMP_STATUS_ACTIVE;
  }
  
  return Status;
}

// This method implements the requirement from RFC-2579 (esp. pp 11-19) that 
// when a read-create entry is NOT_READY, any required objects which have not
// yet been set should return NO_SUCH_INSTANCE when queried and be skipped when
// walking.
// Note that this virtual method may need to be overriden if you
// have a table with more than 32 objects or if for some reason you
// choose to implement the column bits differently than the standard
// way of (1 << (Column-1)).  Or, if a particular table has special accessibility
// requirements outside those of RFC-2579.
bool TabularMibBridge::IsLeafAccessible (int Leaf, const BcmMibRequest *pReq) const
{
  unsigned long LeafBit = (1 << (Leaf - 1));
  
  // Don't apply this restriction to TEST or SET requests, lest we never be able
  // to set required objects!
  if ((pReq == NULL)
  ||  (pReq->fType == BcmMibRequest::kTestRequest)
  ||  (pReq->fType == BcmMibRequest::kSetRequest))
     return true;
/*  
  SnmpLogReq << "TabularMibBridge::IsLeafVisible: " << Leaf << "\n"
             << "  fReqObj = " << (void *) fRequiredObjects << "\n"
             << "  fObjMsk = " << (void *) fObjectMask << "\n"
             << "  LeafBit = " << (void *) LeafBit << endl;
*/  
  if ((fRequiredObjects & LeafBit) == 0)
  {
    //SnmpLogReq << "(fRequiredObjects & LeafBit) == 0: leaf not required.  VISIBLE." << endl;
    return true;
  }
    
  if ((fObjectMask & LeafBit) == 0)
  {
    //SnmpLogReq << "Leaf is requred, (fObjectMask & LeafBit) == 0: NOT VISIBLE." << endl;
    return false;
  }
  
  //SnmpLogReq << "Leaf is required and has been set: VISIBLE" << endl;
  
  return true;
}


