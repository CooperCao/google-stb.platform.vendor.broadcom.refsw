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
//    Filename: docsDevTranslator.cpp
//    Creation Date: Friday, February 13, 2004 at 13:13:13
//
//    This module provides the mechanism to translate from the old
//    draft location of the docsDev node (experimental.83) to the new
//    location (transmission.69)
//
//**************************************************************************


#include "docsDevTranslator.h"
#include "SnmpApi.h"


// Helper functions to translate old draft location to new RFC location and
// vice-versa.

SNMP_STATUS TranslateRfcToDraft (BcmObjectId &RfcOid)
{
  int Loop;
  BcmObjectId RfcBase ("1.3.6.1.2.1.69");
  BcmObjectId DraftOid ("1.3.6.1.3.83");
  
  
  if (RfcBase.IsAscendant (RfcOid) == false)
    return SNMP_INCONSISTENT_NAME;
    
  for (Loop = RfcBase.Length(); Loop < RfcOid.Length(); Loop++)
    DraftOid += RfcOid[Loop];
    
  RfcOid = DraftOid;

  return SNMP_NO_ERROR;
}


SNMP_STATUS TranslateDraftToRfc (BcmObjectId &DraftOid)
{
  int Loop;
  BcmObjectId DraftBase ("1.3.6.1.3.83");
  BcmObjectId RfcOid ("1.3.6.1.2.1.69");
  
  
  if (DraftBase.IsAscendant (DraftOid) == false)
    return SNMP_INCONSISTENT_NAME;
    
  for (Loop = DraftBase.Length(); Loop < DraftOid.Length(); Loop++)
    RfcOid += DraftOid[Loop];
    
  DraftOid = RfcOid;

  return SNMP_NO_ERROR;
}


docsDevMibGroup::docsDevMibGroup (const char *pOID, const char *pName)
  : BcmMibGroup (pOID, pName)
{
}

SNMP_STATUS docsDevMibGroup::AddScalar (BcmMibObject *pObject)
{
  SNMP_STATUS Status = BcmMibGroup::AddScalar (pObject);

  if (Status != SNMP_NO_ERROR)
    return Status;
    
  // Translate the new OID to the old one and add a handler to the agent.
  BcmObjectId DraftOID;
  ObjectId AgentOid;
   
  pObject->GetOid (DraftOID);
  if (TranslateRfcToDraft (DraftOID) == SNMP_NO_ERROR)
  {
    DraftOID.GetOID (&AgentOid);
    AgentRegisterScalar (&AgentOid, pObject->Name());
  }
  
  return Status;
}

SNMP_STATUS docsDevMibGroup::AddTable (BcmMibTable *pTable)
{
  SNMP_STATUS Status = BcmMibGroup::AddTable (pTable);

  if (Status != SNMP_NO_ERROR)
    return Status;
    
  // Translate the new OID to the old one and add a handler to the agent.
  BcmObjectId DraftOID;
  ObjectId AgentOid;
   
  pTable->GetOid (DraftOID);
  if (TranslateRfcToDraft (DraftOID) == SNMP_NO_ERROR)
  {
    DraftOID.GetOID (&AgentOid);
    AgentRegisterTabular (&AgentOid, pTable->Name());
  }
  
  return Status;
}

BcmMibTable *docsDevMibGroup::FindTable (const BcmObjectId &OID)
{
  BcmObjectId RfcOid = OID;

  // Handle the fact that this group may be used for objects at the old
  // draft (experimental.83) or new RFC (mib-II.69) locations.  If we get
  // a request for a draft object, just translate.  
  TranslateDraftToRfc (RfcOid);
  
  return BcmMibGroup::FindTable (RfcOid);
}

BcmMibObject *docsDevMibGroup::FindObject (const BcmObjectId &OID)
{
  BcmObjectId RfcOid = OID;
  
  // Handle the fact that this group may be used for objects at the old
  // draft (experimental.83) or new RFC (mib-II.69) locations.  If we get
  // a request for a draft object, just translate.  
  TranslateDraftToRfc (RfcOid);
  
  return BcmMibGroup::FindObject (RfcOid);
}

BcmMibObject *docsDevMibGroup::FindObject (const char *pOidStr)
{
  BcmObjectId RfcOid = pOidStr;

  // Handle the fact that this group may be used for objects at the old
  // draft (experimental.83) or new RFC (mib-II.69) locations.  If we get
  // a request for a draft object, just translate.    
  TranslateDraftToRfc (RfcOid);
  
  return BcmMibGroup::FindObject (RfcOid);
}

BcmMibObject *docsDevMibGroup::FindOtherInstance (const BcmObjectId &OID, const BcmMibRequest *pReq)
{
  BcmObjectId RfcOid = OID;
  
  // Handle the fact that this group may be used for objects at the old
  // draft (experimental.83) or new RFC (mib-II.69) locations.  If we get
  // a request for a draft object, just translate.  
  TranslateDraftToRfc (RfcOid);
  
  return BcmMibGroup::FindOtherInstance (RfcOid, pReq);
}


docsDevMibTable::docsDevMibTable (const char *pOID, const char *pName)
  : BcmMibTable (pOID, pName)
{
}

BcmMibObject *docsDevMibTable::FindObject (const BcmObjectId &OID)
{
  BcmObjectId RfcOid = OID;
  
  // Handle the fact that this group may be used for objects at the old
  // draft (experimental.83) or new RFC (mib-II.69) locations.  If we get
  // a request for a draft object, just translate.  
  TranslateDraftToRfc (RfcOid);
  
  return BcmMibTable::FindObject (RfcOid);
}


SNMP_STATUS docsDevMibTable::ExtractEntryIndex (const BcmObjectId &Oid, BcmObjectId &Index)
{
  BcmObjectId RfcOid = Oid;
  
  // Handle the fact that this group may be used for objects at the old
  // draft (experimental.83) or new RFC (mib-II.69) locations.  If we get
  // a request for a draft object, just translate.  
  TranslateDraftToRfc (RfcOid);
  
  return BcmMibTable::ExtractEntryIndex (RfcOid, Index);
}


docsDevMibEntry::docsDevMibEntry (BcmMibTable *pTable, const char *pName, const char *pIndex)
  : BcmMibEntry (pTable, pName, pIndex)
{
}

BcmMibObject *docsDevMibEntry::FindObject (const BcmObjectId &OID)
{
  BcmObjectId RfcOid = OID;
  
  // Handle the fact that this group may be used for objects at the old
  // draft (experimental.83) or new RFC (mib-II.69) locations.  If we get
  // a request for a draft object, just translate.  
  TranslateDraftToRfc (RfcOid);
  
  return BcmMibEntry::FindObject (RfcOid);
}
