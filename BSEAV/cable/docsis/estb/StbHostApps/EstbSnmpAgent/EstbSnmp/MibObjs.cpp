
//**************************************************************************
//
//    Copyright 1999  Broadcom Corporation
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
//    Filename: MibObjs.cpp
//    Author:   Kevin O'Neal
//    Creation Date: 21-Jan-2000
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Basic classes for dealing with SNMP objects and concepts
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include <time.h>
#include <stdarg.h>
#include <stdio.h>

#include "IpAddress.h"
#include "MacAddress.h"
#include "OctetBuffer.h"

#include "MibObjs.h"
#include "MibBridge.h"
#include "BerObjs.h"
#include "SnmpLog.h"
#include "BcmSnmpAgent.h"
#include "SnmpApi.h" // for AppSnmpStatusAsString


BcmMibRequest::BcmMibRequest ()
{
  // Initialize members to indicate no packet:
  fVersion = SNMP_QUERY;
  fType = kUnknownPdu;
  fSrcIfIndex = 0;
  fpAgent = NULL;
}

BcmMibRequest::BcmMibRequest (int Type, int Version, int SrcIfIndex, const BcmObjectId &OID, const BcmObjectId &Index, const BcmIpAddress &SrcIp, const BcmIpAddress &DstIp, const BcmString &Cmty, BcmSnmpAgent *pAgent)
{
  fVersion = Version;
  fType = Type;
  fSrcIfIndex = SrcIfIndex;
  
  fCmty = Cmty;
  fOID = OID;
  fIndex = Index;
  fSrcIp = SrcIp;
  fDstIp = DstIp;
  fpAgent = pAgent;
  
  if (fpAgent)
  {
    if (fSrcIfIndex == 0)
    {
#ifdef BCM_STANDALONE_SNMP_AGENT 
      // If we don't know the src ifIndex, use 1, which it always is anyway.
        fSrcIfIndex = 1;
#else
      // If we don't know the src ifIndex, map it from the N2M table of the
      // supplied agent.
      if (fSrcIfIndex == 0)
      {
        const BcmHalIf *pHal = (fpAgent)->N2M().NetToMediaIf (fSrcIp);
        if (pHal)
          fSrcIfIndex = pHal->IfTableIndex(fpAgent);
      }
#endif
    }
  }
}

// This version of the constructor accepts a pointer to the SNMP packet PDU
// (after IP, UDP headers) and parses out the SNMP protocol version, PDU type,
// and community string.
BcmMibRequest::BcmMibRequest (unsigned char *pPacketPdu)
{
  // Initialize members to indicate no packet:
  fVersion = SNMP_QUERY;
  fType = kUnknownPdu;
  fSrcIfIndex = 0;

  ParseFrom (pPacketPdu);
}

BcmMibRequest::~BcmMibRequest ()
{
}

SNMP_STATUS BcmMibRequest::ParseFrom (unsigned char *pPacketPdu)
{
  AsnBerTuple Tuple(pPacketPdu);
  unsigned char *pCurPtr = pPacketPdu;
  
  // Initialize members to indicate no packet:
  fVersion = SNMP_QUERY;
  fType = kUnknownPdu;
  fSrcIfIndex = 0;
  
  SnmpLogCfg << "BcmMibRequest::BcmMibRequest via PDU buffer" << endl;
  
  // First tuple in the packet should be a constructed sequence
  if (Tuple.Tag() != (BER_F_CONSTRUCTED | BER_TLV_SEQUENCE))
  {
    SnmpLogCfg << "  Bad packet, 1st octet != CONSTRUCTED | CONSTRUCTED" << endl;
    return SNMP_GEN_ERR;
  }
  
  // Move pointer past the tag & length fields.
  pCurPtr += (Tuple.TotalLength() - Tuple.Length());
  
  // Next should be an integer which indicates the SNMP protocol type.
  // Feed the buffer into an int32 object to extract the value.
  AsnBerInt32 Version (pCurPtr);
  if (Version.Parse() != SNMP_NO_ERROR)
  {
    SnmpLogCfg << "  Bad packet, error parsing VERSION" << endl;
    return SNMP_GEN_ERR;
  }
  
  if (Version.GetValue (fVersion) != SNMP_NO_ERROR)
  {
    SnmpLogCfg << "  Bad packet, error getting VERSION value." << endl;
    return SNMP_GEN_ERR;
  }
  
  // Convert native SNMP version code to our type.
  switch (fVersion)
  {
    case 0:
      SnmpLogCfg << "  Version = SNMPv1" << endl;
      fVersion = SNMP_V1_QUERY;
      break;
      
    case 1:
      SnmpLogCfg << "  Version = SNMPv2c" << endl;
      fVersion = SNMP_V2_QUERY;
      break;
      
    case 2:
      SnmpLogCfg << "  Version = SNMPv2u" << endl;
      fVersion = SNMP_V2_QUERY;
      break;
      
    case 3:
      SnmpLogCfg << "  Version = SNMPv3" << endl;
      fVersion = SNMP_V3_QUERY;
      break;
      
    default:
      SnmpLogCfg << "  Unknown SNMP version " << fVersion << endl;
      fVersion = SNMP_QUERY;
      break;
  }
  
  // Move pointer past the whole tuple
  pCurPtr += Version.TotalLength(); //(Version.TotalLength() - Version.Length() + 1);
  
  // Next is the community string, provided this is a V1 or V2 query.  Note that
  // the field has to be there, even if the community string is a 0 length string.
  if ((fVersion == SNMP_V1_QUERY) || (fVersion == SNMP_V2_QUERY))
  {
    // Feed the buffer into a string object
    AsnBerOctetString Community(pCurPtr);
    
    if (Community.Parse() != SNMP_NO_ERROR)
    {
      SnmpLogCfg << "  Bad packet, error parsing COMMUNITY." << endl;
      return SNMP_GEN_ERR;
    }
    
    if (Community.GetValue (fCmty) != SNMP_NO_ERROR)
    {
      SnmpLogCfg << "  Bad packet: error getting COMMUNITY value." << endl;
      return SNMP_GEN_ERR;
    }
    else
      SnmpLogCfg << "  Community = " << fCmty << endl;
    
    // Move pointer past the whole tuple
    pCurPtr += Community.TotalLength(); //(Community.TotalLength() - Community.Length() + 1);
  }
  
  // Next field is the PDU type.  This is a single octet which should be
  // (PDU type | (BER_TLV_ENUMERATED << 4)) 
  if (((*pCurPtr) >> 4) != BER_TLV_ENUMERATED)
  {
    SnmpLogCfg << "  Bad packet: PDU type field not ENUMERATED." << endl;
    return SNMP_GEN_ERR;
  }
  
  fType = ((*pCurPtr) & 0x0F);
  
  SnmpLogCfg << "  PDU type = " << fType << endl;
  
  return SNMP_NO_ERROR;
}

void BcmMibRequest::Get (SnmpPacket &Packet) const
{
  Packet.SnmpVersion = fVersion;
  Packet.PduType = fType;
  
  if (Packet.SnmpVersion == SNMP_V3_QUERY)
    Packet.pCommunity = NULL;
  else
    Packet.pCommunity = fCmty.c_str();
    
  fSrcIp.Get(Packet.pSrcIp[0], Packet.pSrcIp[1], Packet.pSrcIp[2], Packet.pSrcIp[3]);
  fDstIp.Get(Packet.pDstIp[0], Packet.pDstIp[1], Packet.pDstIp[2], Packet.pDstIp[3]);
}

ostream &BcmMibRequest::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  const char *Pdu = "PDU??";
  const char *Version;


  switch (fType)
  {
  case kGetRequest:
    Pdu = " GET";
    break;

  case kNextRequest:
    Pdu = " NEXT";
    break;

  case kSetRequest:
    Pdu = " SET";
    break;

  case kTestRequest:
    Pdu = " TEST";
    break;
    
  case kValidateSetRequest:
    Pdu = " VALIDATE-SET";
    break;
    
  case kGetResponse:
    Pdu = " GET RESPONSE";
    break;
    
  case kTrap:
    Pdu = " TRAP";
    break;
    
  case kGetBulkRequest:
    Pdu = " GET BULK";
    break;
    
  case kInformRequest:
    Pdu = " INFORM";
    break;
    
  case kTrap2:
    Pdu = " TRAP2";
    break;
    
  case kRport:
    Pdu = " REPORT";
    break;
    
  case kUnknownPdu:
    Pdu = " UNKNOWN";
    break;
  }
  
  switch (fVersion)
  {
    case SNMP_V1_QUERY:
      Version = "SNMPv1";
      break;
      
    case SNMP_V2_QUERY:
      Version = "SNMPv2c";
      break;
      
    case SNMP_V3_QUERY:
      Version = "SNMPv3";
      break;
      
    default:
      Version = "SNMPv??";
      break;
  }

  outStream << Version << " "  << Pdu << " " <<  fSrcIp << "->" << fDstIp << " cmty='" << fCmty << "' IfIndex=" << fSrcIfIndex << endl;
  
  if (fOID.Length()) 
    outStream << "  Base OID=" << fOID << ", Index=" << fIndex << endl;
#endif

  return outStream;
}


/*\
 *  BcmMibObject methods
\*/ 

BcmMibObject::BcmMibObject (const char *pOID, const char *pName, BcmMibObject *pParent, MibObjectAccess Access)
{
  fpParent = NULL;
  fpMibBridge = NULL;
  fpAgent = NULL;
  fDestroyBridge = false;
  fObjType = kScalarMibObject;
  fpName = pName;
  fpOidString = pOID;
  fAccess = Access;
  fpRequest = NULL;
#ifdef USE_NODE_ID
  fNodeIdLen = 0;
  fpNodeId = NULL;
#endif

  SnmpLogInit << "Creating BcmMibObject: " << Name() << " " << fpOidString << endl;

  if (fpParent)
    SetParent (pParent);
  
  SnmpLogMem << "  " << *this << endl;
}

/*
This version of the constructor is no longer safe to use, since we rely on 
fpOidString being a const char * that's stored in code space (ie 
new BcmMibObject (".1.3.6.1", "internet", NULL)).  If we want to use this version
of the constructor we need to allow for newing up fpOidString and copying.

BcmMibObject::BcmMibObject (BcmObjectId &OID, const char *pName, BcmMibObject *pParent)
{
  fpMibBridge = NULL;
  fpAgent = NULL;
  fDestroyBridge = false;
  fObjType = kScalarMibObject;
  fpName = pName;
  fpOidString = NULL;
#ifdef USE_NODE_ID
  fNodeIdLen = 0;
  fpNodeId = NULL;
#endif

  SnmpLogInit << "Creating BcmMibObject: " << Name() << " " << OID << endl;

  if (fpParent)
    SetParent (pParent);
  
  SnmpLogMem << "  " << *this << endl;
}
*/

BcmMibObject::~BcmMibObject ()
{
  SnmpLogMem << "BcmMibObject::~BcmMibObject for " << Name() << " enter" << endl;

  // De-register and delete our bridge object if we have one
  if ((fpMibBridge != NULL) && (fDestroyBridge == true))
  {
    delete fpMibBridge;
    fpMibBridge = NULL;
    
    DeRegisterBridge (fpMibBridge);
  }

  // Remove ourselves from our parent.
  if (fpParent)
  {
    switch (fObjType)
    {
      // Scalar mib objects can only be owned by MibGroups
      case kScalarMibObject:
        {
          BcmMibGroup *pGroup = (BcmMibGroup *) fpParent;

          // Sanity check:
          if (fpParent->ObjectType() == kMibGroup)
            pGroup->RemoveScalar (this);
          break;
        }

        // Tabular mib objects can only be owned by MibEntries
      case kTabularMibObject:
        {
          BcmMibEntry *pEntry = (BcmMibEntry *) fpParent;

          // Sanity check:
          if (fpParent->ObjectType() == kMibEntry)
            pEntry->RemoveObject (this);
          break;
        }

      default:
        break;
    }
  }

#ifdef USE_NODE_ID    
  if (fpNodeId)
    delete fpNodeId;
#endif

  // Make sure our bridge (if it still exists) does not have a bum
  // pointer to us, which is about to be deleted.
  if ((fpMibBridge) && (fpMibBridge->fpObject == this))
    fpMibBridge->fpObject = NULL;

  SnmpLogMem << "BcmMibObject::~BcmMibObject for " << Name() << " exit" << endl;
}

SNMP_STATUS BcmMibObject::SetParent (BcmMibObject *pParent)
{
  fpParent = pParent;
  
#ifdef USE_NODE_ID    
  int OidLen;
  BcmObjectId MyOID, OtherOID;
  BcmObjectId Index;
  static unsigned long Objects = 0;

  /*\
   *  Build our node ID.
   *
   *  Note: this approach only works as long as we still carry around
   *  the fully-qualified OID string with our MIB objects.  We can further
   *  reduce RAM consumption (and code size) by changing this approach, but
   *  we will then have to revisit how we form our node ID.  Actually, what
   *  we'd likely do is simply have the string associated with each MibObject
   *  be limited to JUST it's node ID, which actually simplifies the process
   *  quite a bit in addition to having RAM and code size advantages.  If we
   *  only have our node ID as a string, then we don't need to wait until we
   *  have a parent to determine our NodeID, we just assign it at construction.
   *  For now, though, we can't determine which part of our OID is our node OID
   *  until we know what our parent is, then we say our node ID is that part of
   *  our whole OID that is different from our parent's.
  \*/

  if (fpNodeId)
    return SNMP_NO_ERROR;

  if (fpOidString)
    MyOID = fpOidString;

  // If we have no parent, then our node ID becomes our entire OID.
  if (fpParent == NULL)
    fNodeIdLen = MyOID.Length();
  else
  {
    fpParent->GetOid (OtherOID);

/*
    // DEBUG
    BcmString String;
    MyOID.AsString (String);
*/
    if (fpParent->ObjectType() == kMibEntry)
    {
      fObjType = kTabularMibObject;
      fpParent->GetIndex(Index);
      //Index.AsString (String);
      MyOID += Index;
      //MyOID.AsString (String);
    }
    else if (fpParent->ObjectType() == kMibTable)
    {
      fpParent->GetOid (MyOID);
      MyOID += 1;
      //MyOID.AsString (String);
    }

    // Build our node ID.  This is the part of our OID which is
    // different from our parent's
    fNodeIdLen = MyOID.Length() - OtherOID.Length();
  }

  OidLen = MyOID.Length();

  Objects++;
  //SnmpLogRaw << Objects << ": " << fpName << " " << fNodeIdLen << endl;

  fpNodeId = new unsigned long [fNodeIdLen];
  for (int Loop = 0; Loop < fNodeIdLen; Loop++)
    fpNodeId[Loop] = MyOID[OidLen - fNodeIdLen + Loop];

#else

  if (fpParent->ObjectType() == kMibEntry)
    fObjType = kTabularMibObject;

#endif
  
  return SNMP_NO_ERROR;
}

#ifdef USE_NODE_ID    
SNMP_STATUS BcmMibObject::BuildOid (BcmObjectId &OID)
{
  if (fpParent)
    fpParent->BuildOid (OID);
  else if (fNodeIdLen == 0)
    SetParent (NULL);

  for (int Loop = 0; Loop < fNodeIdLen; Loop++)
    OID += fpNodeId[Loop];

  return SNMP_NO_ERROR;
}
#endif

bool BcmMibObject::IsBridged () const
{
  if (fpMibBridge != NULL)
    return true;

  if (fpParent)
  {
    if (fpParent->IsBridged() == true)
      return true;
    else
      SnmpLogReq << fpParent->Name() << " isn't bridged, so neither is " << Name() << endl;
  }
  else
    SnmpLogReq << Name() << " has no parent and is not bridged." << endl;

  return false;
}

bool BcmMibObject::IsVisible () const
{
  if (fpParent)
  {
    if (fpParent->IsVisible () == false)
      return false;
  }

  return true;
}

bool BcmMibObject::IsAccessible (BcmMibRequest *pReq) const
{
  // Remember our request info.  I have to do a really evil casting away
  // of the constitude of this object in order to get away with this.
  // Changing this method to be non-const is too hairy, potentially impacting
  // many derived objects.  So be a naughty C++ programmer just this once...
  ((BcmMibObject *)this)->fpRequest = pReq;

  if (fpParent)
  {
    // If my parent is not accessible, neither am I.
    if (fpParent->IsAccessible (pReq) == false)
    {
      SnmpLogReq << Name() << ": fpParent->IsAccessible (" << fpParent->Name() << ") returned false, NOT ACCESSIBLE" << endl;
      return false;
    }

    // If my parent has a bridge, see what he has to say.
    if (fpParent->fpMibBridge)
    {
      if (fpParent->fpMibBridge->IsLeafAccessible (Leaf(), pReq) == false)
      {
        SnmpLogReq << Name() << ": fpParent->fpMibBridge->IsLeafAccessible (" << fpParent->Name() << ")returned false, NOT ACCESSIBLE" << endl;
        return false;
      }
    }
  }
    
  if (IsBridged() == false)
  {
    SnmpLogReq << Name() << ": IsBridged returned false, NOT ACCESSIBLE" << endl;
    return false;
  }
  
  return true;
}

int BcmMibObject::BoolToTruth (bool BoolVal) const
{
  if (BoolVal == true)
    return SNMP_TRUTH_TRUE;
  else
    return SNMP_TRUTH_FALSE;
}

bool BcmMibObject::TruthToBool (int TruthVal) const
{
  if (TruthVal == SNMP_TRUTH_TRUE)
    return true;
  else
    return false;
}

#ifdef USE_NODE_ID
void BcmMibObject::GetOid (BcmObjectId &Oid)
#else
void BcmMibObject::GetOid (BcmObjectId &Oid) const
#endif
{
#ifdef USE_NODE_ID
  if (fpParent)
  { 
    // Initialize OID to empty
    Oid = "";
    BuildOid (Oid);
  }
  else if (fpOidString)
    Oid = fpOidString;
  else
    Oid = "0.0";
#else
  if (fpOidString)
    Oid = fpOidString;
  else
    Oid = "";
    
  // If we are a tabular mib object, append the index here.
  if (fObjType == kTabularMibObject)
  {
    BcmObjectId Index;
    GetIndex (Index);
    Oid += Index;
  }
#endif
}

void BcmMibObject::GetIndex (BcmObjectId &Ix) const
{
#ifdef USE_NODE_ID
  // Leaf types: index is all componenets of our node ID except
  // the first one;  
  if ((fObjType == kScalarMibObject)
  ||  (fObjType == kTabularMibObject))
    Ix.Set (fpNodeId+1, fNodeIdLen-1);
  else
    Ix = "";
#else
  // Scalar objects always have an index of .0, but just in case somebody
  // tries something wacky, use the last element in the OID string
  if (fObjType == kScalarMibObject)
  {
    BcmObjectId OID = fpOidString;
    if (OID.Length() <= 0)
      Ix = "";
    else
      Ix = OID[OID.Length()-1];
  }
  // Tabular objects have the same index as the entry they are part of.
  else if (fObjType == kTabularMibObject)
  {
    if (fpParent)
      fpParent->GetIndex (Ix);
    else
      Ix = "";
  }
  // Any other kind of object (table or group) has no index.  Entry indices
  // are handled seperately in overridden method BcmMibEntry::GetIndex
  else
    Ix = "";
#endif
}

const char *BcmMibObject::Name () const
{
  static char NoName[] = "no-name";

  if (fpName)
    return fpName;

  return NoName;
}

int BcmMibObject::ObjectType () const
{
  return fObjType;
}

BcmMibObject::MibObjectAccess BcmMibObject::Access () const
{
  return fAccess;
}

unsigned long BcmMibObject::Leaf () const
{
#ifdef USE_NODE_ID
  if (fNodeIdLen == 0)
    return 0;

  // the 'leaf' is the first component of our node ID, excluding the index.
  return fpNodeId[0];
#else
  BcmObjectId OID, Index;
  GetOid (OID);
  GetIndex (Index);
  
  if (Index.Length() == 0)
    return 0;
  return OID[OID.Length()-Index.Length()-1];
#endif
}

BcmMibRequest *BcmMibObject::LastRequestInfo () const
{
  return fpRequest;
}

SNMP_STATUS BcmMibObject::SetIndex (const BcmObjectId &Index)
{
#ifdef USE_NODE_ID
  unsigned long NodeLen = Index.Length() + 1, Node = 0;

  if (fpNodeId)
  {
    Node = fpNodeId[0];
    delete fpNodeId;
  }

  fpNodeId = new unsigned long [NodeLen];

  fpNodeId[0] = Node;
  for (int Loop = 0; Loop < NodeLen; Loop++)
    fpNodeId[Loop+1] = Index[Loop];

  SnmpLogInit << "Setting " << Name() << " index to " << Index << endl;
#else
  // if we are anything other than a BcmMibEntry, we don't need to do
  // anything here.  And BcmMibEntry has an overridden method to handle
  // this.
#endif

  return SNMP_NO_ERROR;
}


SNMP_STATUS BcmMibObject::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{ 
  if (fpMibBridge != NULL)
  {
    //gLogMessageRaw << "===> WARNING: rejecting attempt to re-bridge " << Name() << " to " << fpAgent->GetName() << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }

  fpMibBridge = pBridge;
  pBridge->fpObject = this;
  
  if (DeleteUponDestruction)
    fDestroyBridge = true;
  else
    fDestroyBridge = false;

  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmMibObject::DeRegisterBridge (BcmMibBridge *pBridge)
{ 
  SnmpLogMem << "BcmMibObject::DeRegisterBridge - enter" << endl;
  
  fpMibBridge = NULL;

/*  
  UNFINISHED but probably not needed.  For scalar objects, at this point
  we should remove the scalar from its group.  But since we wouldn't
  normally expect scalars to be dynamic the way table entries might be,
  for the time being this is left unfinished. Something like
  
  fpParent->RemoveScalar (this);
*/  

  SnmpLogMem << "BcmMibObject::DeRegisterBridge - exit" << endl;

  return SNMP_NO_ERROR;
}


SNMP_STATUS BcmMibObject::SetAgent (BcmSnmpAgent *pAgent)
{
  if (fpAgent != NULL)
    return SNMP_RESOURCE_UNAVAILABLE;
    
  fpAgent = pAgent;
  return SNMP_NO_ERROR;
}


BcmSnmpAgent *BcmMibObject::GetAgent ()
{
  return fpAgent;
}


ostream &BcmMibObject::Print(ostream &outStream)
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  BcmVarBind Vb;
  BcmObjectId Oid, Index;
  SNMP_STATUS Status;
  const char *pAccess = "??";


  GetOid (Oid);
  GetIndex (Index);
/*
  // DEBUG:
  BcmObjectId OID;
  BcmString StrOID;
  BuildOid (OID);
  OID.AsString (StrOID);
*/
  Status = Get(Vb);
  
  switch (Access())
  {
    case kUnspecified:
      pAccess = "us";
      break;
      
    case kNotAccessible:
      pAccess = "NA";
      break;
      
    case kAccessibleForNotify:
      pAccess = "AfN";
      break;
      
    case kReadOnly:
      pAccess = "RO";
      break;
      
    case kReadWrite:
      pAccess = "RW";
      break;
      
    case kReadCreate:
      pAccess = "RC";
      break;
  }
  
  if ((Status == SNMP_NO_ERROR) || (Status == SNMP_NO_ACCESS))
    outStream << Name() << "." << Index << " (" << Oid << ":" << pAccess << ") value = " << Vb;
  else
  {
    char StatusString[32];
    
    AppSnmpStatusAsString (Status, StatusString, 32);
    outStream << Name() << "." << Index << " (" << Oid << ":" << pAccess << ") get error:" << StatusString << endl;
  }
#endif

  return outStream;
}


/*\
 *  BcmMibEntry methods
\*/

BcmMibEntry::BcmMibEntry (BcmMibTable *pTable, const char *pName, const char *pIndex)
: BcmMibObject ("", pName, NULL, kNotAccessible)
{
  fpOidString = pTable->OidString();
  fpTable = pTable;
  fpStatusObject = NULL;
  fpIndex = NULL;
  fIndexLen = 0;
  fLastColumn = 0;

#ifdef USE_NODE_ID
  // All BcmMibEntries have node id's of just 1.  Actually, this is
  // just GENERALLY true, not a requirement.  For example, docsQosParamSetEntry
  // as a node ID of 2 in the QOS-05 MIB because it got changed from the previous
  // version.  So, in the case of other entry objects where this general
  // rule doesn't apply, the derived object's constructor should change
  // the below.
  fNodeIdLen = 1;
  fpNodeId = new unsigned long;
  *fpNodeId = 1;
#endif

  BcmObjectId Index;

  if (pIndex)
    Index = pIndex;

  SetIndex (Index);
  SetParent (pTable);

  fObjType = kMibEntry;
}

BcmMibEntry::~BcmMibEntry ()
{
  SnmpLogMem << "BcmMibEntry::~BcmMibEntry for " << Name() << " enter" << endl;

  // De-register and delete our bridge object if we have one
  if ((fpMibBridge != NULL) && (fDestroyBridge == true))
  {
    delete fpMibBridge;
    fpMibBridge = NULL;
    
    DeRegisterBridge (fpMibBridge);
  }

  // Remove ourself from the table.
  if (fpTable)
    fpTable->RemoveEntry (this);

  // Delete all our objects.
  BcmMibObject  *pObject;
  MibObjectIterator Pos;


  // Spin through all Objects and see if we find the one passed in.
  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pObject = (BcmMibObject *) (*Pos).second;
#else
    pObject = (BcmMibObject *) *Pos;
#endif    

    fObjects.erase(Pos++);
    delete pObject;
  }
  
  if (fpIndex)
  {
    delete fpIndex;
    fpIndex = NULL;
  }

  SnmpLogMem << "BcmMibEntry::~BcmMibEntry for " << Name() << " exit" << endl;
}

void BcmMibEntry::GetIndex(BcmObjectId &Ix) const
{
  Ix.Set (fpIndex, fIndexLen);
}

#ifdef USE_NODE_ID
void BcmMibEntry::GetOid (BcmObjectId &Oid)
#else
void BcmMibEntry::GetOid (BcmObjectId &Oid) const
#endif
{
  // This call will essentially get the OID of our parent table,
  // mainly because we want to conserve RAM by not storing the OID
  // for each entry.
  BcmMibObject::GetOid (Oid);
  
  // Now, generally an entry OID is tableOid.1.  But every once in a while
  // some genius will make it tableOid.2, as is the case in the dreaded
  // docsQosParamSetTable for DOCSIS.  So any weirdball table like that needs
  // to override this method.
  Oid += 1;
}

unsigned long BcmMibEntry::LastColumn () const
{
  return fLastColumn;
}

SNMP_STATUS BcmMibEntry::SetIndex (const BcmObjectId &Index)
{
  fIndexLen = Index.Length();

  if (fpIndex)
  {
    delete fpIndex;
    fpIndex = NULL;
  }

  if (fIndexLen)
  {
    fpIndex = new unsigned long [fIndexLen];
    for (int Loop = 0; Loop < fIndexLen; Loop++)
      fpIndex[Loop] = Index[Loop];
  }

  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmMibEntry::AddObject (BcmMibObject *pObject)
{
  if (fpAgent)
    fpAgent->LockMib();

  pObject->SetParent (this);
  pObject->SetAgent (fpAgent);

  BcmObjectId Oid, Index;
  pObject->GetOid (Oid);
  pObject->GetIndex (Index);

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
  fObjects[Oid] = pObject;
#else
  fObjects.push_back(pObject);
#endif

  if (Oid[Oid.Length()-Index.Length()-1] > fLastColumn)
    fLastColumn = Oid[Oid.Length()-Index.Length()-1];

  if (fpAgent)
    fpAgent->UnlockMib();

  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmMibEntry::RemoveObject (BcmMibObject *pObject)
{
  BcmMibObject  *pCheckObject;
  MibObjectIterator Pos;
  SNMP_STATUS Status = SNMP_NO_SUCH_NAME;

  
  if (fpAgent)
    fpAgent->LockMib();

  // Spin through all Objects and see if we find the one passed in.
  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pCheckObject = (BcmMibObject *) (*Pos).second;
#else
    pCheckObject = (BcmMibObject *) *Pos;
#endif    
    if (pCheckObject == pObject)
    {
      fObjects.erase(Pos++);
      delete pCheckObject;

      Status = SNMP_NO_ERROR;
    }
    else
      Pos++;
  }
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return Status;
}

BcmMibObject *BcmMibEntry::FindObject (const BcmObjectId &OID)
{
  MibObjectIterator Pos;
  BcmMibObject *pObject;
  BcmObjectId MatchOid;

  
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
  Pos = fObjects.find (OID);
  if (Pos == fObjects.end())
    return NULL;

  return (*Pos).second;
#else

  if (fpAgent)
    fpAgent->LockMib(); 

  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
    pObject = *Pos;

    pObject->GetOid (MatchOid);
    if (MatchOid == OID)
    {
    
      if (fpAgent)
        fpAgent->UnlockMib();
        
      return pObject;
    }

    Pos++;
  }
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return NULL;
#endif
}

BcmMibObject *BcmMibEntry::FindObject (const char *pOidStr)
{
  MibObjectIterator Pos;
  BcmMibObject *pObject;
  char Element[16];
  char MatchStr[128];
  int Loop;

  
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
  Pos = fObjects.find (BcmObjectId (pOidStr));
  
  if (Pos == fObjects.end())
    return NULL;

  return (*Pos).second;
#else
  if (fpAgent)
    fpAgent->LockMib();
    
  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
    pObject = *Pos;

    // Build the object's OID.  Get the base...
    strcpy (MatchStr, pObject->OidString());
    
    // ...add on the index.
    for (Loop = 0; Loop < fIndexLen; Loop++)
    {
      sprintf (Element, ".%lu", fpIndex[Loop]);
      strcat (MatchStr, Element);
    }
    
    if (strcmp (MatchStr, pOidStr) == 0)
    {
      if (fpAgent)
        fpAgent->UnlockMib();
      
      return pObject;
    }

    Pos++;
  }
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return NULL;
#endif
}

BcmMibObject *BcmMibEntry::FindObject (const BcmString &Name, const BcmObjectId &Index)
{
  MibObjectIterator Pos;
  BcmMibObject *pObject;
  BcmObjectId ObjIx;

  SnmpLogSearch << fpName << "::FindObject " << Name << "." << Index << endl;
  
  if (fpAgent)
    fpAgent->LockMib();

  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
    pObject = (*Pos).second;
#else
    pObject = *Pos;
#endif
    
    SnmpLogSearch << "Check object " << (void *) pObject << endl;
    
    pObject->GetIndex (ObjIx);

    if ((Name == pObject->Name())
    &&  (Index == ObjIx))
    {
      if (fpAgent)
        fpAgent->UnlockMib();
        
      return pObject;
    }
    else
      SnmpLogSearch << pObject->Name() << "." << ObjIx << " - no match." << endl;

    Pos++;
  }
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return NULL;
}


BcmMibObject *BcmMibEntry::FindObject (const BcmString &Name)
{
  MibObjectIterator Pos;
  BcmMibObject *pObject;


  if (fpAgent)
    fpAgent->LockMib();

  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
    pObject = (*Pos).second;
#else
    pObject = *Pos;
#endif

    if (Name == pObject->Name())
    {
      if (fpAgent)
        fpAgent->UnlockMib();
       
      return pObject;
    }

    Pos++;
  }
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return NULL;
}


SNMP_STATUS BcmMibEntry::SetAgent (BcmSnmpAgent *pAgent)
{
  MibObjectIterator Pos;
  BcmMibObject *pObject;
  BcmSnmpAgent *pOldAgent = fpAgent, *pNewAgent = pAgent;

  
  if (pOldAgent)
    pOldAgent->LockMib();
  if (pNewAgent)
    pNewAgent->LockMib();

  Pos = fObjects.begin ();
  while (Pos != fObjects.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pObject = (BcmMibObject *) (*Pos).second;
#else
    pObject = (BcmMibObject *) *Pos;
#endif

    pObject->SetAgent (pAgent);

    Pos++;
  } 
  
  if (pOldAgent)
    pOldAgent->UnlockMib();
  if (pNewAgent)
    pNewAgent->UnlockMib();

  return BcmMibObject::SetAgent (pAgent);
}

RowStatusMibObject *BcmMibEntry::GetStatusObject ()
{
  return fpStatusObject;
}

// Handle the case of a row-status driven table which has entries which have
// not yet been "created".
bool BcmMibEntry::IsVisible () const
{
  if (fpStatusObject == NULL)
    return true;

  BcmVarBind VB;
  
  fpStatusObject->Get(VB);
  
  if (VB == SNMP_STATUS_INVISIBLE)
    return false;

  return true;
}

bool BcmMibEntry::IsAccessible (BcmMibRequest *pReq) const
{
  if (BcmMibObject::IsAccessible(pReq) == false)
    return false;
    
  return IsBridged ();
}

bool BcmMibEntry::IsBridged () const
{
  if (fpMibBridge != NULL)
    return true;
  else
    return false;
}

SNMP_STATUS BcmMibEntry::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  // If our table is NULL, that means that BcmMibTable::AddEntry
  // rejected this entry for being a duplicate.  Return an error.
  if (fpTable == NULL)
  {
    // Even though we are in error at this point, go ahead and
    // have the bridge know about the entry so everything will get 
    // deleted without memory leaks if the caller properly deletes the 
    // bridge on failure
    //pBridge->fpObject = this;
    //fpMibBridge = pBridge;
    
    return SNMP_DUPLICATE_VALUE;
  }
  
  return BcmMibObject::RegisterBridge (pBridge, DeleteUponDestruction);
}

SNMP_STATUS BcmMibEntry::DeRegisterBridge (BcmMibBridge *pBridge)
{
  SnmpLogMem << "BcmMibEntry::DeRegisterBridge for " << Name() << endl;

  if (fpTable)
    return fpTable->DeRegisterBridge (pBridge);
  else
    return BcmMibObject::DeRegisterBridge (pBridge);
}

ostream &BcmMibEntry::Print(ostream &outStream)
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  MibObjectIterator Pos;
  BcmMibObject *pObject;
  BcmObjectId Index;

/*
  // DEBUG:
  BcmObjectId OID;
  BcmString StrOID;
  BuildOid (OID);
  OID.AsString (StrOID);
*/
  GetIndex (Index);

  outStream << "----------------------------------" << endl;
  outStream << Name() << "." << Index << endl;
  outStream << "  last column = " << LastColumn() << endl;

  if (fpStatusObject)
    outStream << "  ROW STATUS object is " << fpStatusObject->Name() << ", creation method = " << fpStatusObject->CreationMethod() << endl;

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

    Pos++;
  }
#endif

  return outStream;
}


/*\
 *  BcmMibTable methods
\*/

BcmMibTable::BcmMibTable (const char *pOID, const char *pName)
: BcmMibObject (pOID, pName, NULL, kNotAccessible)
{
  SnmpLogMem << "BcmMibTable::BcmMibTable" << endl;
  fObjType = kMibTable;
  
  fColumnMask = 0xFFFFFFFF;
}

BcmMibTable::~BcmMibTable ()
{
  SnmpLogMem << "BcmMibTable:~:BcmMibTable for " << Name() << endl;

#if !DYNAMIC_POPULATION  
  Depopulate ();
#endif

  // De-register our bridge object.  Note that you wouldn't expect a single
  // bridge object for a whole table, generally they're per-entry.  But just
  // for completenes...
  DeRegisterBridge (fpMibBridge);

  // Remove ourselves from the group that owns us.
  if (fpParent)
  {
    // Sanity check that our parent is in fact a group:
    if (fpParent->ObjectType() == kMibGroup)
      ((BcmMibGroup *)fpParent)->RemoveTable (this);
    else
      SnmpLogMem << "WARNING: " << Name() << "'s parent not a group, it's " << fpParent->Name() << endl;
  }
}

bool BcmMibTable::Populate ()
{
  if (!fEntries.empty())
    return false;

  return true;
}

bool BcmMibTable::Depopulate ()
{ 
  MibEntryIterator Pos;                 
  BcmMibEntry *pEntry;


  SnmpLogInit << "Depopulating " << Name() << endl;

  // Iterate through all entries and delete 'em.
  Pos = fEntries.begin();
  while (Pos != fEntries.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pEntry = (BcmMibEntry *)(*Pos).second;
#else
    pEntry = (BcmMibEntry *) *Pos;
#endif

    fEntries.erase (Pos++);
    delete (pEntry);
  }

  // Just in case we missed one, why not...
  fEntries.clear ();

  return true;
}

/*
  10/30/01: got rid of this general approach.  Necessary cleanup is now
  handled by the particular objects that are related to various MIB components
  instead of having the MIB components handle themselves.
void BcmMibTable::InformRestartPending ()
{ 
  MibEntryIterator Pos;                 
  BcmMibEntry *pEntry;


  SnmpLogRaw << Name() << " informed of pending restart." << endl;
  
  // Iterate through all entries and have each one reset its default state.
  // Read-create tables will generally be emptied by this action, others
  // may have default values restored.
  Pos = fEntries.begin();
  while (Pos != fEntries.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pEntry = (BcmMibEntry *)(*Pos).second;
#else
    pEntry = (BcmMibEntry *) *Pos;
#endif
    
    // Increment iterator now because the entry might wind up
    // getting destroyed below, thus invalidating the iterator.
    Pos++;  
    
    if (pEntry->fpMibBridge)
      pEntry->fpMibBridge->SetDefaults();
    else
      SnmpLogInit << pEntry->Name() << "::InformRestartPending - no bridge!" << endl;
  }
}
*/

bool BcmMibTable::IsBridged () const
{
  if (fpMibBridge != NULL)
    return true;
  else
    return false;
}

bool BcmMibTable::IsAccessible (BcmMibRequest *pReq) const
{
  // Remember our request info.  I have to do a really evil casting away
  // of the constitude of this object in order to get away with this.
  // Changing this method to be non-const is too hairy, potentially impacting
  // many derived objects.  So be a naughty C++ programmer just this once...
  ((BcmMibTable *)this)->fpRequest = pReq;

  if (fpParent)
  {
    if (fpParent->IsAccessible(pReq) == false)
    {
      SnmpLogReq << Name() << ": fpParent->IsAccessible (" << fpParent->Name() << ") returned false, NOT ACCESSIBLE" << endl;
      return false;
    }
  }

  return true;
}

SNMP_STATUS BcmMibTable::AddEntry (BcmMibEntry *pEntry)
{
  MibEntryIterator Pos;
  BcmObjectId Index;


  if (pEntry == NULL)
    return SNMP_WRONG_VALUE;
    
  SnmpLogInit << "BcmMibTable::AddEntry" << endl;

  // Verify that the entry belongs in this table.
  if (pEntry->fpTable != this)
  {
    SnmpLogInit << "BcmMibEntry " << pEntry->Name() << " is not a child of " << Name() << endl;
    return SNMP_INCONSISTENT_NAME;
  }
    
  pEntry->GetIndex (Index);
  
  // Verify we aren't trying to add a duplicate entry
  if (FindEntry (Index) != NULL)
  {
    //gLogMessageRaw << "===> WARNING: rejecting attempt to add duplicate entry " << Index << " to " << Name()  << " on " << fpAgent->GetName() << endl;
    
    // Setting the table to NULL here is used by BcmMibEntry::RegisterBridge
    // to realize that the addition of the new entry has been rejected.
    pEntry->fpTable = NULL;
      
    return SNMP_DUPLICATE_VALUE;
  }
  
  if (fpAgent)
    fpAgent->LockMib();
  
  pEntry->SetAgent (fpAgent);

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)    
  fEntries[Index] = pEntry;
#else

  // Order the insertion here.  Find the next entry that's larger than
  // this one, insert the new entry before it.  Note that NextEntry below
  // might return NULL if there's not a larger index in the table, but
  // that's OK because the Pos in that case will be fEntries.end, and we'll
  // wind up with the entry at the end of the list as we would like.
  NextEntry (Index, Pos);
  fEntries.insert (Pos, pEntry);
#endif

  SnmpLogInit << "Table " << Name() << " added entry " << pEntry->Name() << "." << Index << endl;
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmMibTable::RemoveEntry (BcmMibEntry *pEntry)
{
  BcmMibEntry  *pCheckEntry;
  MibEntryIterator Pos;
  SNMP_STATUS Status = SNMP_NO_SUCH_NAME;

  if (fpAgent)
    fpAgent->LockMib();

  // Spin through all Entries and see if we find the one passed in.
  Pos = fEntries.begin();
  while (Pos != fEntries.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pCheckEntry = (BcmMibEntry *) (*Pos).second;
#else
    pCheckEntry = (BcmMibEntry *) *Pos;
#endif    
    if (pCheckEntry == pEntry)
    {
      fEntries.erase(Pos++);
      Status = SNMP_NO_ERROR;
    }
    else
      Pos++;
  }
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return Status;
}

int BcmMibTable::NumEntries () const
{
  return fEntries.size();
}

bool BcmMibTable::DoesColumnExist (unsigned long Column) const
{
  if (Column > 32)
    return false;
    
  if ((1 << Column) & fColumnMask)
    return true;
    
  return false;
}

BcmMibObject *BcmMibTable::FindObject (const BcmObjectId &Oid)
{
  MibEntryIterator Pos;
  BcmMibObject *pObject;
  BcmMibEntry  *pEntry;
  BcmObjectId MyOID;

  GetOid (MyOID);


  // If the OID we got passed is not a descendant of
  // our own OID, then we ain't gonna find it!
  if (Oid.IsDescendant (MyOID) == false)
    return NULL;
    
  if (fpAgent)
    fpAgent->LockMib();

  // The OID we got passed is a descendant of this table's
  // OID, so there's a chance it's in here.  Let's have a look...
  Pos = fEntries.begin();
  while (Pos != fEntries.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pEntry = (BcmMibEntry *)(*Pos).second;
#else
    pEntry = (BcmMibEntry *)*Pos;
#endif

    pObject = pEntry->FindObject (Oid);
    if (pObject)
    {
      if (fpAgent)
        fpAgent->UnlockMib();
      return pObject;
    }

    Pos++;
  }
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return NULL;
}

BcmMibObject *BcmMibTable::FindObject (const char *pOidStr)
{
  MibEntryIterator Pos;
  BcmMibObject *pObject;
  BcmMibEntry  *pEntry;

  
  if (fpAgent)
    fpAgent->LockMib();

/*
  Don't check this here, do it in BcmMibGroup::FindObject before calling 
  this function.  Saves having to parse pOidStr multiple times...
    
  // If the OID we got passed is not a descendant of
  // our own OID, then we ain't gonna find it!
  BcmObjectId MyOid, Oid(pOidStr);
  GetOid (MyOid);
  if (Oid.IsDescendant (MyOid) == false)
    return NULL;
*/

  // The OID we got passed is a descendant of this table's
  // OID, so there's a chance it's in here.  Let's have a look...
  Pos = fEntries.begin();
  while (Pos != fEntries.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pEntry = (BcmMibEntry *)(*Pos).second;
#else
    pEntry = (BcmMibEntry *)*Pos;
#endif

    pObject = pEntry->FindObject (pOidStr);
    if (pObject)
    {
      if (fpAgent)
        fpAgent->UnlockMib();
        
      return pObject;
    }

    Pos++;
  }
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return NULL;
}

BcmMibObject *BcmMibTable::FindObject (const BcmString &Name, const BcmObjectId &Index)
{
  MibEntryIterator Pos;
  BcmMibObject *pObject;
  BcmMibEntry  *pEntry;

  
  SnmpLogSearch << fpName << "::FindObject " << Name << "." << Index << endl;
  
  if (fpAgent)
    fpAgent->LockMib();
  
  Pos = fEntries.begin();
  while (Pos != fEntries.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pEntry = (BcmMibEntry *)(*Pos).second;
#else
    pEntry = (BcmMibEntry *)*Pos;
#endif

    SnmpLogSearch << "Check entry " << (void *) pEntry << ", fpName = " << (void *) pEntry->fpName << endl;

    pObject = pEntry->FindObject (Name, Index);
    if (pObject)
    {
      if (fpAgent)
        fpAgent->UnlockMib();
      return pObject;
    }

    Pos++;
  }
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return NULL;
}

BcmMibObject *BcmMibTable::FindObject (const BcmString &Name)
{
  MibEntryIterator Pos;
  BcmMibObject *pObject;
  BcmMibEntry  *pEntry;


  if (fpAgent)
    fpAgent->LockMib();
                         
  Pos = fEntries.begin();
  while (Pos != fEntries.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pEntry = (BcmMibEntry *)(*Pos).second;
#else
    pEntry = (BcmMibEntry *)*Pos;
#endif

    pObject = pEntry->FindObject (Name);
    if (pObject)
    {
      if (fpAgent)
        fpAgent->UnlockMib();
        
      return pObject;
    }

    Pos++;
  }
  
  if (fpAgent)
    fpAgent->UnlockMib(); 

  return NULL;
}


SNMP_STATUS BcmMibTable::SetAgent (BcmSnmpAgent *pAgent)
{
  MibEntryIterator Pos;
  BcmMibEntry  *pEntry;
  BcmSnmpAgent *pOldAgent = fpAgent, *pNewAgent = pAgent;

  
  if (pOldAgent)
    pOldAgent->LockMib();
  if (pNewAgent)
    pNewAgent->LockMib();

  Pos = fEntries.begin();
  while (Pos != fEntries.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pEntry = (BcmMibEntry *)(*Pos).second;
#else
    pEntry = (BcmMibEntry *)*Pos;
#endif

    pEntry->SetAgent (pAgent);

    Pos++;
  }
  
  if (pOldAgent)
    pOldAgent->UnlockMib();
  if (pNewAgent)
    pNewAgent->UnlockMib();

  return BcmMibObject::SetAgent (pAgent);
}

BcmMibEntry *BcmMibTable::FirstEntry ()
{
  MibEntryIterator Pos;
  return FirstEntry (Pos);
}


BcmMibEntry *BcmMibTable::NextEntry (const BcmObjectId &Index)
{
  MibEntryIterator Pos;
  BcmMibEntry  *pEntry, *pNext = NULL;
  BcmObjectId MatchIx;

  if (fpAgent)
    fpAgent->LockMib();
    
  Pos = fEntries.begin();
  while (Pos != fEntries.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
    pEntry = (BcmMibEntry *)(*Pos).second;
#else
    pEntry = (BcmMibEntry *) *Pos;
#endif
    pEntry->GetIndex (MatchIx);

    if (MatchIx > Index)
    // KO MOD: still return an item, even if it is not visible.
    // It should be up to the caller to test this if need be, and
    // one should be able to recurse entries even if they are not
    // visible!
    //&& (pEntry->IsVisible()))
    {
      // Since the list is ordered now, we can break once we
      // find a larger one.
      pNext = pEntry;
      break;
    }

    Pos++;
  }
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return pNext;
}


BcmMibEntry *BcmMibTable::NextEntry (const BcmObjectId &Index, MibEntryIterator &Pos)
{
  BcmMibEntry  *pEntry, *pNext = NULL;
  BcmObjectId MatchIx;


  if (fpAgent)
    fpAgent->LockMib();

  // Iterate through the entries and get the next one after the Index passed in
  Pos = fEntries.begin();
  while (Pos != fEntries.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
    pEntry = (BcmMibEntry *)(*Pos).second;
#else
    pEntry = (BcmMibEntry *) *Pos;
#endif
    pEntry->GetIndex (MatchIx);

    if (MatchIx > Index)
    // KO MOD: still return an item, even if it is not visible.
    // It should be up to the caller to test this if need be, and
    // one should be able to recurse entries even if they are not
    // visible!
    //&& (pEntry->IsVisible()))
    {
      // Since the list is ordered, we can stop once we find the first
      // one that matches the above criteria.
      pNext = pEntry;
      break;
    }

    Pos++;
  }
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return pNext;
}


BcmMibEntry *BcmMibTable::LastEntry ()
{
  MibEntryIterator Pos;
  return LastEntry (Pos);
}


BcmMibEntry *BcmMibTable::PrevEntry (const BcmObjectId &Index)
{
  MibEntryIterator Pos;
  BcmMibEntry  *pEntry, *pPrev = NULL;
  BcmObjectId MatchIx;


  if (fEntries.empty())
    return NULL;
    
  if (fpAgent)
    fpAgent->LockMib();

  Pos = fEntries.end();

  do
  {
    Pos--;

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
    pEntry = (BcmMibEntry *)(*Pos).second;
#else
    pEntry = (BcmMibEntry *) *Pos;
#endif
    pEntry->GetIndex (MatchIx);

    if (MatchIx < Index)
    // KO MOD: still return an item, even if it is not visible.
    // It should be up to the caller to test this if need be, and
    // one should be able to recurse entries even if they are not
    // visible!
    //&& (pEntry->IsVisible()))
    {
      // Since the list is ordered now, we can break once we
      // find a larger one.
      pPrev = pEntry;
      break;
    }
  }
  while (Pos != fEntries.begin());
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return pPrev;
}


BcmMibEntry *BcmMibTable::PrevEntry (const BcmObjectId &Index, MibEntryIterator &Pos)
{
  BcmMibEntry  *pEntry, *pPrev = NULL;
  BcmObjectId MatchIx;


  if (Pos == fEntries.begin())
    return NULL;
    
  if (fpAgent)
    fpAgent->LockMib();

  // Iterate through the entries and get the next one after the Index passed in
  do
  {
    Pos--;

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
    pEntry = (BcmMibEntry *)(*Pos).second;
#else
    pEntry = (BcmMibEntry *) *Pos;
#endif
    pEntry->GetIndex (MatchIx);

    if (MatchIx < Index)
    // KO MOD: still return an item, even if it is not visible.
    // It should be up to the caller to test this if need be, and
    // one should be able to recurse entries even if they are not
    // visible!
    //&& (pEntry->IsVisible()))
    {
      // Since the list is ordered, we can stop once we find the first
      // one that matches the above criteria.
      pPrev = pEntry;
      break;
    }
  }
  while (Pos != fEntries.begin());
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return pPrev;
}

BcmMibEntry *BcmMibTable::LastEntry (MibEntryIterator &Pos)
{
  BcmMibEntry *pEntry = NULL;


  Pos = fEntries.end();

  if (fEntries.empty() == true)
    return NULL;

  Pos--;

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
  pEntry = (BcmMibEntry *)(*Pos).second;
#else
  pEntry = (BcmMibEntry *) *Pos;
#endif

  // KO MOD: still return an item, even if it is not visible.
  // It should be up to the caller to test this if need be, and
  // one should be able to recurse entries even if they are not
  // visible!
  //if (pEntry->IsVisible())
  return pEntry;
  //else
  //  return PrevEntry (Pos);
}

BcmMibEntry *BcmMibTable::PrevEntry (MibEntryIterator &Pos)
{
  BcmMibEntry  *pEntry, *pPrev = NULL;


  if (fEntries.empty() == true)
    return NULL;

  // If we are already at the beginning of the list, there
  // is nothing before that.
  if (Pos == fEntries.begin())
    return NULL;
    
  if (fpAgent)
    fpAgent->LockMib();

  // Iterate through the entries and get the next one after the Index passed in
  do
  {
    Pos--;

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
    pEntry = (BcmMibEntry *)(*Pos).second;
#else
    pEntry = (BcmMibEntry *) *Pos;
#endif

    // KO MOD: still return an item, even if it is not visible.
    // It should be up to the caller to test this if need be, and
    // one should be able to recurse entries even if they are not
    // visible!
    //if (pEntry->IsVisible())
    {
      // Since the list is ordered, we can stop once we find the first
      // one that matches the above criteria.
      pPrev = pEntry;
      break;
    }
  }
  while (Pos != fEntries.begin());
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return pPrev;
}

BcmMibEntry *BcmMibTable::FindEntry (const BcmObjectId &Index, MibEntryIterator &Pos)
{
  BcmMibEntry *pEntry;
  BcmObjectId MatchIx;
  
  
  if (fpAgent)
    fpAgent->LockMib();

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)    
  Pos = fEntries.find (Index);
  
  if (fpAgent)
    fpAgent->UnlockMib();
  
  if (Pos == fEntries.end())
    return NULL;

  return(BcmMibEntry *)(*Pos).second;
#else
  Pos = fEntries.begin();
  while (Pos != fEntries.end())
  {
    pEntry = (BcmMibEntry *)*Pos;
    pEntry->GetIndex(MatchIx);

    if (MatchIx == Index)
    {
      if (fpAgent)
        fpAgent->UnlockMib();
       
      return pEntry;
    }

    Pos++;
  }

  // If we didn't find one, reset pos to the beginning of the list
  Pos = fEntries.begin();
  
  if (fpAgent)
    fpAgent->UnlockMib();
  
  return NULL;
#endif
}

BcmMibEntry *BcmMibTable::FindEntry (const BcmObjectId &Index)
{
  MibEntryIterator Pos;
  BcmMibEntry *pEntry;
  BcmObjectId MatchIx;


#if (MIB_OBJECT_CONTAINERS == USE_MAPS)    
  Pos = fEntries.find (Index);
  if (Pos == fEntries.end())
    return NULL;

  return(BcmMibEntry *)(*Pos).second;
#else
  if (fpAgent)
    fpAgent->LockMib();
    
  Pos = fEntries.begin();
  while (Pos != fEntries.end())
  {
    pEntry = (BcmMibEntry *)*Pos;
    pEntry->GetIndex (MatchIx);

    if (MatchIx == Index)
    {
      if (fpAgent)
        fpAgent->UnlockMib();
      return pEntry;
    }

    Pos++;
  }

  if (fpAgent)
    fpAgent->UnlockMib();
    
  return NULL;
#endif
}


SNMP_STATUS BcmMibTable::ExtractEntryIndex (const BcmObjectId &Oid, BcmObjectId &Index)
{
  unsigned long Length;
  BcmObjectId MyOID;


  GetOid (MyOID);

  if (Oid.IsDescendant (MyOID) == false)
    return SNMP_WRONG_VALUE;

  // The index being requested is the part of OID that is longer
  // than our table's OID + 1, and it's as big as the difference in the
  // length of the OID's plus one.
  Length = Oid.Length() - MyOID.Length() - 2;
  Index.Set(&Oid[MyOID.Length()+2], Length);

  return SNMP_NO_ERROR;
}

BcmMibEntry *BcmMibTable::CreateEntry (const BcmObjectId &OID, BcmMibRequest *pReq)
{
  BcmObjectId Index;
  BcmMibEntry *pEntry;
  RowStatusMibObject *pStatusObject;
  //BcmVarBind StVb;


  if (ExtractEntryIndex(OID, Index) != SNMP_NO_ERROR)
  {
    SnmpLogInit << "BcmMibTable::CreateEntry: ExtractEntryIndex failed." << endl;
    return NULL;
  }

  SnmpLogInit << "BcmMibTable::CreateEntry: OID requested's index will be " << Index << endl;

  // Now go ahead and create the new entry.  Note that in general,
  // we don't know what kind of entry is contained in this table.
  // So our derived class will have to override the following function to
  // create a new WhateverEntry and add it to the list.
  pEntry = NewDefaultEntry (Index);

  // Check for this.  Base class NewDefaultEntry returns NULL.  Also, if the
  // index is not right we'll get a NULL back.
  if (pEntry == NULL)
  {
    SnmpLogInit << "BcmMibTable::CreateEntry: NewDefaultEntry returned NULL" << endl;
    return NULL;
  }

  pStatusObject = pEntry->GetStatusObject ();

  // Initially, this object will be INVISIBLE.  Set it's status object
  // accordingly.
  if (pStatusObject)
  {
//  StVb.SetInt32(SNMP_STATUS_INVISIBLE);
//  pStatusObject->Set (StVb, true);

    // Indicate which interface created it.
    if (pReq != NULL)
      pStatusObject->fHowCreated = pReq->fSrcIfIndex;
    else
      pStatusObject->fHowCreated = RowStatusMibObject::kFromProvisioning;
  }

  return pEntry;
}

SNMP_STATUS BcmMibTable::RemoveEntry (const BcmObjectId &Index)
{
  BcmMibEntry *pEntry = NULL;
  MibEntryIterator Pos;


  SnmpLogInit << "BcmMibTable::RemoveEntry: removing entry of index " << Index << endl;
  
  if (fpAgent)
    fpAgent->LockMib();

  // Find and remove entry from entry container
  pEntry = FindEntry (Index, Pos);

  if (pEntry == NULL)
  {
    SnmpLogInit << "BcmMibTable::RemoveEntry failed to find Index " << Index << endl;
    
    if (fpAgent)
      fpAgent->UnlockMib();
      
    return SNMP_NO_SUCH_NAME;
  }

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
  fEntries.erase (Index);
#else
  fEntries.erase (Pos);
#endif

  // Set entry's fpTable pointer to NULL to avoid calling this again
  // in the entry's destructor.  Actually, re-calling wouldn't hurt us, 
  // but it does take some time so this will speed things up a bit.
  pEntry->fpTable = NULL;
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmMibTable::DestroyEntry (const BcmObjectId &Index)
{
  BcmMibEntry *pEntry = NULL;
  MibEntryIterator Pos;


  SnmpLogInit << "BcmMibTable::DestroyEntry: destroying entry of index " << Index << endl;

  /* Actually destroy this entry */
  
  if (fpAgent)
    fpAgent->LockMib();

  // Find and remove entry from entry container
  pEntry = FindEntry (Index, Pos);

  if (pEntry == NULL)
  {
    SnmpLogInit << "BcmMibTable::DestroyEntry failed to find Index " << Index << endl;
    
    if (fpAgent)
      fpAgent->UnlockMib();
    
    return SNMP_NO_SUCH_NAME;
  }

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
  fEntries.erase (Index);
#else
  fEntries.erase (Pos);
#endif

  // Set entry's fpTable pointer to NULL to avoid calling this again
  // in the entry's destructor.  Actually, re-calling wouldn't hurt us, 
  // but it does take some time so this will speed things up a bit.
  pEntry->fpTable = NULL;

  delete (pEntry);
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return SNMP_NO_ERROR;
}

// Remove & destroy all entries from the table
void BcmMibTable::Flush ()
{
  MibEntryIterator Pos;                 
  BcmMibEntry *pEntry;


  if (fpAgent)
    fpAgent->LockMib();

  // Iterate through all entries and delete each one.
  Pos = fEntries.begin();
  while (Pos != fEntries.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pEntry = (BcmMibEntry *)(*Pos).second;
#else
    pEntry = (BcmMibEntry *) *Pos;
#endif
    
    // Increment iterator now because the entry might wind up
    // getting destroyed below, thus invalidating the iterator.
    Pos++;  
    
    delete pEntry;
  }
  
  if (fpAgent)
    fpAgent->UnlockMib();
}

ostream &BcmMibTable::Print(ostream &outStream)
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  MibEntryIterator Pos;
  BcmMibEntry *pEntry;
  BcmObjectId Oid;

  GetOid (Oid);
  
  if (fpAgent)
    fpAgent->LockMib();

/*
  // DEBUG:
  BcmObjectId Oid;
  BcmString StrOID;
  BuildOid (Oid);
  Oid.AsString (StrOID);
*/
  if (fpAgent)
    outStream << Name() << " OID = " << Oid << /*" agent='" << fpAgent->GetName() << "'" <<*/ endl;
  else
    outStream << Name() << " OID = " << Oid << " agent=NULL" << endl;

  outStream << "**********************************" << endl;

  // Iterate through all objects and print 'em
  Pos = fEntries.begin();
  while (Pos != fEntries.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
    pEntry = (BcmMibEntry *) (*Pos).second;
#else
    pEntry = (BcmMibEntry *) *Pos;
#endif

    outStream << *pEntry;

    Pos++;
  }
  
  if (fpAgent)
    fpAgent->UnlockMib();
#endif

  return outStream;
}

SNMP_STATUS BcmMibTable::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{ 
  BcmObjectId NewIndex;
  
  // Don't allow a duplicate entry to be bridged.
  pBridge->GetIndex (NewIndex);
  if (FindEntry (NewIndex) != NULL)
  {
    //gLogMessageRaw << "===> WARNING: rejecting attempt to re-bridge " << Name() << "." << NewIndex << " to " << fpAgent->GetName() << endl;
    return SNMP_DUPLICATE_VALUE;
  }
  
  return BcmMibObject::RegisterBridge (pBridge, DeleteUponDestruction);
}

SNMP_STATUS BcmMibTable::DeRegisterBridge (BcmMibBridge *pBridge)
{ 
  BcmObjectId NixIx;

  SnmpLogMem << "BcmMibTable::DeRegisterBridge" << endl;

  if (pBridge)
  {
    // Remove the entry associated with this bridge.  
    if (pBridge->fpObject) 
    {
      pBridge->fpObject->GetIndex(NixIx);
      RemoveEntry (NixIx);
    }
  }

  return BcmMibObject::DeRegisterBridge (pBridge);
}


/*\
 *  BcmMibGroup methods
\*/

BcmMibGroup::BcmMibGroup(const char *pOID, const char *pName)
: BcmMibObject (pOID, pName, NULL, kNotAccessible)
{
  fpName = pName;

  fObjType = kMibGroup;
/*  
  BcmObjectId OID;
  ObjectId AgentOid;

  GetOid (OID);
  OID.GetOID (&AgentOid);
  
  AgentRegisterGroup (&AgentOid, Name());
*/  
}

BcmMibGroup::~BcmMibGroup ()
{
  SnmpLogMem << "BcmMibGroup::~BcmMibGroup for " << Name() << endl;

#if !DYNAMIC_POPULATION
  Depopulate ();
#endif

  // De-register our bridge object.
  DeRegisterBridge (fpMibBridge);

  // Remove ourselves from the group that owns us, if there is one.
  if (fpParent)
  {
    // Sanity check that our parent is in fact a group:
    if (fpParent->ObjectType() == kMibGroup)
      ((BcmMibGroup *)fpParent)->RemoveGroup (this);
    else
    {
      SnmpLogMem << "WARNING: " << Name() << "'s parent not a group, it's " << fpParent->Name() << endl;
    }
  }

  // Try to remove ourselves from the agent.
  if (fpAgent)
    fpAgent->RemoveGroup (this);
}

bool BcmMibGroup::Populate ()
{
  SnmpLogInit << "BcmMibGroup::Populate" << endl;
/*
  if (!fScalars.empty())
    return false;

  if (!fTables.empty())
    return false;

  if (!fGroups.empty())
    return false;
*/
  if (!fObjects.empty())
    return false;

  return true;
}

bool BcmMibGroup::Depopulate ()
{ 
/*
  MibObjectIterator SPos;                 
  MibTableIterator TPos;
  MibGroupIterator GPos;
  BcmMibObject *pScalar;
  BcmMibTable  *pTable;
  BcmMibGroup *pGroup;


  SnmpLogInit << "Depopulating " << Name() << endl;

  // Iterate through all objects and delete 'em.
  SPos = fScalars.begin();
  while (SPos != fScalars.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pScalar = S(*Pos).second;
#else
    pScalar = *SPos;
#endif
    fScalars.erase (SPos++);
    delete (pScalar);
  }

  // Just in case we missed one, why not...
  fScalars.clear ();

  TPos = fTables.begin();
  while (TPos != fTables.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pTable = (BcmMibTable *)T(*Pos).second;
#else
    pTable = (BcmMibTable *) *TPos;
#endif
    fTables.erase (TPos++);
    delete (pTable);
  }

  // Just in case we missed one, why not...
  fTables.clear ();

  // Now do any sub-groups
  GPos = fGroups.begin();
  while (GPos != fGroups.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pGroup = (BcmMibGroup *)G(*Pos).second;
#else
    pGroup = (BcmMibGroup *) *GPos;
#endif
    fGroups.erase (GPos++);
    delete (pGroup);
  }

  // Just in case we missed one, why not...
  fGroups.clear ();

  return true;
*/
  MibObjectIterator Pos;                 
  BcmMibObject *pObject;


  SnmpLogInit << "Depopulating " << Name() << endl;

  // Iterate through all objects and delete 'em.
  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pObject = (*Pos).second;
#else
    pObject = *Pos;
#endif
    fObjects.erase (Pos++);
    delete (pObject);
  }

  // Just in case we missed one, why not...
  fObjects.clear ();


  return true;
}

/*
  10/30/01: got rid of this general approach.  Necessary cleanup is now
  handled by the particular objects that are related to various MIB components
  instead of having the MIB components handle themselves.
void BcmMibGroup::InformRestartPending ()
{ 
  MibObjectIterator Pos;
  BcmMibObject *pObject;
  BcmMibTable  *pTable;
  BcmMibGroup  *pGroup;


  SnmpLogRaw << Name() << " informed of pending restart..." << endl;
  
  // Tell our bridge to put himself back in his default state.
  if (fpMibBridge)
    fpMibBridge->SetDefaults();
  else
    SnmpLogInit << Name() << "::InformRestartPending - no bridge!" << endl;

  // Inform any tables & groups of our demise
  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pObject = (*Pos).second;
#else
    pObject = *Pos;
#endif
    if (pObject->ObjectType() == kMibTable)
    {
      pTable = (BcmMibTable *)pObject;
      pTable->InformRestartPending ();
    }
    else if (pObject->ObjectType() == kMibGroup)
    {
      pGroup = (BcmMibGroup *)pObject;
      pGroup->InformRestartPending ();
    }
    
    Pos++;
  }
}
*/

bool BcmMibGroup::IsBridged () const
{
  if (fpMibBridge != NULL)
    return true;
  else
    return false;
}

bool BcmMibGroup::IsAccessible (BcmMibRequest *pReq) const
{
  // Remember our request info.  I have to do a really evil casting away
  // of the constitude of this object in order to get away with this.
  // Changing this method to be non-const is too hairy, potentially impacting
  // many derived objects.  So be a naughty C++ programmer just this once...
  ((BcmMibGroup *)this)->fpRequest = pReq;
  
  if (fpParent)
  {
    if (fpParent->IsAccessible (pReq) == false)
    {
      SnmpLogReq << Name() << ": fpParent->IsAccessible (" << fpParent->Name() << ") returned false, NOT ACCESSIBLE" << endl;
      return false;
    }
  }

  return true;
}

// This function is used to register a bridge object for all the 
// SCALAR objects in the group.  Tabular bridges are registered
// individually.
SNMP_STATUS BcmMibGroup::RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction)
{
  BcmObjectId Oid, BridgeOid;

  pBridge->GetOid(BridgeOid);
  GetOid (Oid);
  
  // If the bridge's OID matches my own, go ahead and register.
  if (Oid == BridgeOid)
    return BcmMibObject::RegisterBridge (pBridge, DeleteUponDestruction);
    
  return SNMP_NO_SUCH_NAME;
}

SNMP_STATUS BcmMibGroup::DeRegisterBridge (BcmMibBridge *pBridge)
{
  SnmpLogMem << "BcmMibGroup::DeRegisterBridge" << endl;
  return BcmMibObject::DeRegisterBridge(pBridge);
}

SNMP_STATUS BcmMibGroup::AddScalar (BcmMibObject *pScalar)
{
/*
  pScalar->SetAgent (fpAgent);
  pScalar->SetParent (this);

  SnmpLogInit << "BcmMibGroup: Adding scalar " << pScalar->Name() << endl;

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
  BcmObjectId Oid;
  pScalar->GetOid(Oid);
  fScalars[Oid] = pScalar;
#else
  fScalars.push_back(pScalar);
#endif

  return SNMP_NO_ERROR;
*/

  BcmObjectId OID;
  ObjectId AgentOid;

  if (fpAgent)
    fpAgent->LockMib();
    
  pScalar->GetOid (OID);
  OID.GetOID (&AgentOid);
  
  AgentRegisterScalar (&AgentOid, pScalar->Name());

  pScalar->SetAgent (fpAgent);
  pScalar->SetParent (this);

  SnmpLogInit << "BcmMibGroup: Adding scalar " << pScalar->Name() << endl;

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
  BcmObjectId Oid;
  pScalar->GetOid(Oid);
  fObjects[Oid] = pScalar;
#else
  fObjects.push_back(pScalar);
#endif

  if (fpAgent)
    fpAgent->UnlockMib();

  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmMibGroup::RemoveScalar (BcmMibObject *pScalar)
{
/*
  BcmMibObject  *pCheckObject;
  MibObjectIterator Pos;
  SNMP_STATUS Status = SNMP_NO_SUCH_NAME;


  // Spin through all Objects and see if we find the one passed in.
  Pos = fScalars.begin();
  while (Pos != fScalars.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pCheckObject = (BcmMibObject *) (*Pos).second;
#else
    pCheckObject = (BcmMibObject *) *Pos;
#endif    
    if (pCheckObject == pScalar)
    {
      fScalars.erase(Pos++);
      Status = SNMP_NO_ERROR;
    }
    else
      Pos++;
  }

  return Status;
*/
  BcmMibObject  *pCheckObject;
  MibObjectIterator Pos;
  SNMP_STATUS Status = SNMP_NO_SUCH_NAME;

  if (fpAgent)
    fpAgent->LockMib();

  // Spin through all Objects and see if we find the one passed in.
  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pCheckObject = (BcmMibObject *) (*Pos).second;
#else
    pCheckObject = (BcmMibObject *) *Pos;
#endif    
    if (pCheckObject == pScalar)
    {
      fObjects.erase(Pos++);
      Status = SNMP_NO_ERROR;
    }
    else
      Pos++;
  }
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return Status;
}

SNMP_STATUS BcmMibGroup::AddTable (BcmMibTable *pTable)
{
/*
  SnmpLogInit << "BcmMibGroup: Adding table " << pTable->Name() << endl;

  pTable->SetAgent (fpAgent);
  pTable->SetParent (this);

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
  BcmObjectId Oid;
  pTable->GetOid(Oid);
  fTables[Oid] = pTable;
#else
  fTables.push_back (pTable);
#endif

  return SNMP_NO_ERROR;
*/
  BcmObjectId OID;
  ObjectId AgentOid;
  

  SnmpLogInit << "BcmMibGroup: Adding table " << pTable->Name() << endl;
  
  if (fpAgent)
    fpAgent->LockMib();
    
  pTable->GetOid (OID);
  OID.GetOID (&AgentOid);
  AgentRegisterTabular (&AgentOid, pTable->Name());

  pTable->SetAgent (fpAgent);
  pTable->SetParent (this);

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
  BcmObjectId Oid;
  pTable->GetOid(Oid);
  fObjects[Oid] = pTable;
#else
  fObjects.push_back (pTable);
#endif

  if (fpAgent)
    fpAgent->UnlockMib();

  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmMibGroup::RemoveTable (BcmMibTable *pTable)
{
/*
  BcmMibTable  *pCheckTable;
  MibTableIterator Pos;
  SNMP_STATUS Status = SNMP_NO_SUCH_NAME;


  // Spin through all Tables and see if we find the one passed in.
  Pos = fTables.begin();
  while (Pos != fTables.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pCheckTable = (BcmMibTable *) (*Pos).second;
#else
    pCheckTable = (BcmMibTable *) *Pos;
#endif    
    if (pCheckTable == pTable)
    {
      fTables.erase(Pos++);
      Status = SNMP_NO_ERROR;
    }
    else
      Pos++;
  }

  return Status;
*/
  BcmMibTable  *pCheckTable;
  MibTableIterator Pos;
  SNMP_STATUS Status = SNMP_NO_SUCH_NAME;

  
  if (fpAgent)
    fpAgent->LockMib();

  // Spin through all Tables and see if we find the one passed in.
  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pCheckTable = (BcmMibTable *) (*Pos).second;
#else
    pCheckTable = (BcmMibTable *) *Pos;
#endif    
    if (pCheckTable == pTable)
    {
      fObjects.erase(Pos++);
      Status = SNMP_NO_ERROR;
    }
    else
      Pos++;
  }
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return Status;
}

SNMP_STATUS BcmMibGroup::AddGroup (BcmMibGroup *pGroup)
{
/*
  SnmpLogInit << "BcmMibGroup: Adding Group " << pGroup->Name() << endl;

  pGroup->SetAgent (fpAgent);
  pGroup->SetParent (this);

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
  BcmObjectId Oid;
  pGroup->GetOid(Oid);
  fGroups[Oid] = pGroup;
#else
  fGroups.push_back(pGroup);
#endif

  return SNMP_NO_ERROR;
*/
  SnmpLogInit << "BcmMibGroup: Adding Group " << pGroup->Name() << endl;
  
  if (fpAgent)
    fpAgent->LockMib();

  pGroup->SetAgent (fpAgent);
  pGroup->SetParent (this);

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
  BcmObjectId Oid;
  pGroup->GetOid(Oid);
  fObjects[Oid] = pGroup;
#else
  fObjects.push_back(pGroup);
#endif

  if (fpAgent)
    fpAgent->UnlockMib();

  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmMibGroup::RemoveGroup (BcmMibGroup *pGroup)
{
/*
  BcmMibGroup  *pCheckGroup;
  MibGroupIterator Pos;
  SNMP_STATUS Status = SNMP_NO_SUCH_NAME;


  // Spin through all groups and see if we find the one passed in.
  Pos = fGroups.begin();
  while (Pos != fGroups.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pCheckGroup = (BcmMibGroup *) (*Pos).second;
#else
    pCheckGroup = (BcmMibGroup *) *Pos;
#endif    
    if (pCheckGroup == pGroup)
    {
      fGroups.erase(Pos++);
      Status = SNMP_NO_ERROR;
    }
    else
      Pos++;
  }

  return Status;
*/
  BcmMibGroup  *pCheckGroup;
  MibObjectIterator Pos;
  SNMP_STATUS Status = SNMP_NO_SUCH_NAME;

  
  if (fpAgent)
    fpAgent->LockMib();

  // Spin through all groups and see if we find the one passed in.
  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pCheckGroup = (BcmMibGroup *) (*Pos).second;
#else
    pCheckGroup = (BcmMibGroup *) *Pos;
#endif    
    if (pCheckGroup == pGroup)
    {
      fObjects.erase(Pos++);
      Status = SNMP_NO_ERROR;
    }
    else
      Pos++;
  }
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return Status;
}

BcmMibObject *BcmMibGroup::FindObject (const BcmObjectId &Oid)
{
/*
  MibObjectIterator ScalarPos;
  MibTableIterator TablePos;
  MibGroupIterator GroupPos;
  BcmMibObject *pObject = NULL;
  BcmMibTable *pTable;  
  BcmMibGroup *pGroup;
  BcmObjectId MatchOid;


  GetOid (MatchOid);

  if (MatchOid.IsAscendant (Oid) == false)
    return NULL;

  // Check scalar members first:
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
  ScalarPos = fScalars.find (Oid);
  if (ScalarPos != fScalars.end())
    pObject = Scalar(*Pos).second;
#else
  ScalarPos = fScalars.begin();
  while (ScalarPos != fScalars.end())
  {
    pObject = *ScalarPos;
    pObject->GetOid(MatchOid);
    if (MatchOid == Oid)
      break;

    ScalarPos++;
    pObject = NULL;
  }
#endif

  if (pObject)
  {
    SnmpLogInit << "Found matching object " << pObject->Name() << endl;
    return pObject;
  }

  // If no scalar object matches, iterate through tables
  TablePos = fTables.begin();
  while (TablePos != fTables.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pTable = (BcmMibTable *)Table(*Pos).second;
#else
    pTable = (BcmMibTable *) *TablePos;
#endif

    pObject = pTable->FindObject (Oid);
    if (pObject)
      return pObject;

    SnmpLogInit << "No match " << Name() << "." << pTable->Name() << endl;

    TablePos++;
  }

  // No match in any tables either?  How about sub-groups...
  GroupPos = fGroups.begin();
  while (GroupPos != fGroups.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pGroup = (BcmMibGroup *)Group(*Pos).second;
#else
    pGroup = (BcmMibGroup *) *GroupPos;
#endif

    pObject = pGroup->FindObject (Oid);
    if (pObject)
      return pObject;

    SnmpLogInit << "No match " << Name() << "." << pGroup->Name() << endl;

    GroupPos++;
  }

  SnmpLogInit << "No match" << endl;
  return NULL;  
*/
  MibObjectIterator Pos;
  BcmMibObject *pObject = NULL;
  BcmMibTable *pTable;  
  BcmMibGroup *pGroup;
  BcmObjectId MatchOid;


  GetOid (MatchOid);

  if (MatchOid.IsAscendant (Oid) == false)
    return NULL;
    
  if (fpAgent)
    fpAgent->LockMib();

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
  // If the OID matches a scalar object, this'll find it right off.
  Pos = fObjects.find (Oid);
  if (Pos != fObjects.end())
    pObject = (*Pos).second;
  
  // No scalar match? then we gotta iterate and check subgroups and tables.
  if (pObject == NULL)
  {
    Pos = fObjects.begin();
    while (Pos != fObjects.end())
    {
      pObject = (*Pos).second;
      if (pObject->ObjectType() == kMibTable)
      {
        pTable = (BcmMibTable *) pObject;
        pObject = pTable->FindObject(Oid);
        if (pObject)
          break;
      }
      else if (pObject->ObjectType() == kMibGroup)
      {
        pGroup = (BcmMibGroup *) pObject;
        pObject = pGroup->FindObject(Oid);
        if (pObject)
          break;
      }

      Pos++;
      pObject = NULL;
    }
  }
#else
  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
    pObject = *Pos;
    pObject->GetOid(MatchOid);
    if ((pObject->ObjectType() == kScalarMibObject)
    ||  (pObject->ObjectType() == kTabularMibObject))
    {
      if (MatchOid == Oid)
        break;
    }
    else if (pObject->ObjectType() == kMibTable)
    {
      pTable = (BcmMibTable *) pObject;
      pObject = pTable->FindObject(Oid);
      if (pObject)
        break;
    }
    else if (pObject->ObjectType() == kMibGroup)
    {
      pGroup = (BcmMibGroup *) pObject;
      pObject = pGroup->FindObject(Oid);
      if (pObject)
        break;
    }

    Pos++;
    pObject = NULL;
  }
#endif

  if (pObject)
    SnmpLogInit << "Found matching object " << pObject->Name() << endl;
  else
    SnmpLogInit << "No match" << endl;
    
  if (fpAgent)
    fpAgent->UnlockMib();
  
  return pObject;  
}

BcmMibObject *BcmMibGroup::NextObject (const BcmObjectId &Oid)
{
  MibObjectIterator Pos;
  BcmMibObject *pObject = NULL;
  BcmMibGroup *pGroup;
  BcmObjectId MatchOid;


  SnmpLogReq << "  NextObject for " << Name() << endl;

  GetOid (MatchOid);

  if ((MatchOid.IsAscendant (Oid) == false)
  &&  (MatchOid != Oid))
  {
    SnmpLogReq << "  " << MatchOid << " is not ascendant of or equal to " << Oid << endl;
    return NULL;
  }
    
  if (fpAgent)
    fpAgent->LockMib();

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
  UNFINISHED - revisit this if we ever use maps! (intentional compiler error here)

  // If the OID matches a scalar object, this'll find it right off.
  Pos = fObjects.find (Oid);
  if (Pos != fObjects.end())
    pObject = (*Pos).second;
  
  // No scalar match? then we gotta iterate and check subgroups and tables.
  if (pObject == NULL)
  {
    Pos = fObjects.begin();
    while (Pos != fObjects.end())
    {
      pObject = (*Pos).second;
      if (pObject->ObjectType() == kMibTable)
      {
        pTable = (BcmMibTable *) pObject;
        pObject = pTable->NextObject(Oid);
        if (pObject)
          break;
      }
      else if (pObject->ObjectType() == kMibGroup)
      {
        pGroup = (BcmMibGroup *) pObject;
        pObject = pGroup->NextObject(Oid);
        if (pObject)
          break;
      }

      Pos++;
      pObject = NULL;
    }
  }
#else
  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
    pObject = *Pos;
    pObject->GetOid(MatchOid);
    
    if (pObject->ObjectType() == kScalarMibObject)
    {
      SnmpLogReq << MatchOid << " > " << Oid << "?" << endl;
      if (MatchOid > Oid)
      {
        SnmpLogReq << "  YES!" << endl;
        break;
      }
    }
    else if (pObject->ObjectType() == kMibGroup)
    {
      SnmpLogReq << "  Recurse into " << pObject->Name() << endl;
      pGroup = (BcmMibGroup *) pObject;
      pObject = pGroup->NextObject(Oid);
      if (pObject)
        break;
    }

    Pos++;
    pObject = NULL;
  }
#endif

  if (pObject)
    SnmpLogInit << "Found matching object " << pObject->Name() << endl;
  else
    SnmpLogInit << "No match" << endl;
    
  if (fpAgent)
    fpAgent->UnlockMib();
  
  return pObject;  
}

BcmMibObject *BcmMibGroup::FindObject (const char *pOidStr)
{
  MibObjectIterator Pos;
  BcmMibObject *pObject = NULL;
  BcmMibTable *pTable;  
  BcmMibGroup *pGroup;
  BcmObjectId MatchOid, Oid(pOidStr);

/*  
  GetOid (MatchOid);
  if (MatchOid.IsAscendant (Oid) == false)
    return NULL;
*/

  if (fpAgent)
    fpAgent->LockMib();

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
  // If the OID matches a scalar object, this'll find it right off.
  Pos = fObjects.find (Oid);
  if (Pos != fObjects.end())
    pObject = (*Pos).second;
  
  // No scalar match? then we gotta iterate and check subgroups and tables.
  if (pObject == NULL)
  {
    Pos = fObjects.begin();
    while (Pos != fObjects.end())
    {
      pObject = (*Pos).second;
      if (pObject->ObjectType() == kMibTable)
      {
        pTable = (BcmMibTable *) pObject;
        pTable->GetOid (MatchOid);
        if (MatchOid.IsAscendant (Oid) == true)
        {
          pObject = pTable->FindObject(pOidStr);
          if (pObject)
            break;
        }
      }
      else if (pObject->ObjectType() == kMibGroup)
      {
        pGroup = (BcmMibGroup *) pObject;
        pGroup->GetOid (MatchOid);
        if (MatchOid.IsAscendant (Oid) == true)
        {
          pObject = pGroup->FindObject(pOidStr);
          if (pObject)
            break;
        }
      }

      Pos++;
      pObject = NULL;
    }
  }
#else
  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
    pObject = *Pos;
    if ((pObject->ObjectType() == kScalarMibObject)
    ||  (pObject->ObjectType() == kTabularMibObject))
    {
      if (strcmp (pObject->OidString(), pOidStr) == 0)
        break;
    }
    else if (pObject->ObjectType() == kMibTable)
    {
      pTable = (BcmMibTable *) pObject;
      pTable->GetOid (MatchOid);
      if (MatchOid.IsAscendant (Oid) == true)
      {
        pObject = pTable->FindObject(pOidStr);
        if (pObject)
          break;
      }
    }
    else if (pObject->ObjectType() == kMibGroup)
    {
      pGroup = (BcmMibGroup *) pObject;
      pGroup->GetOid (MatchOid);
      if (MatchOid.IsAscendant (Oid) == true)
      {
        pObject = pGroup->FindObject(pOidStr);
        if (pObject)
          break;
      }
    }

    Pos++;
    pObject = NULL;
  }
#endif

  if (pObject)
    SnmpLogInit << "Found matching object " << pObject->Name() << endl;
  else
    SnmpLogInit << "No match" << endl;
    
  if (fpAgent)
    fpAgent->UnlockMib();
  
  return pObject;  
}

BcmMibObject *BcmMibGroup::FindObject (const BcmString &Name, const BcmObjectId &Index)
{
/*
  MibObjectIterator ScalarPos;
  MibTableIterator TablePos;
  MibGroupIterator GroupPos;
  BcmMibObject *pObject = NULL;
  BcmMibTable *pTable;  
  BcmMibGroup *pGroup;
  BcmObjectId MatchIx;


  // Check scalar members first:
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
  ScalarPos = fScalars.find (OID);
  if (ScalarPos != fScalars.end())
    pObject = Scalar(*Pos).second;
#else
  ScalarPos = fScalars.begin();
  while (ScalarPos != fScalars.end())
  {
    pObject = *ScalarPos;
    pObject->GetIndex(MatchIx);
    if ((pObject->Name() == Name)
    &&  (MatchIx == Index))
      break;

    ScalarPos++;
    pObject = NULL;
  }
#endif

  if (pObject)
  {
    SnmpLogInit << "Found matching object " << pObject->Name() << endl;
    return pObject;
  }

  // If no scalar object matches, iterate through tables
  TablePos = fTables.begin();
  while (TablePos != fTables.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pTable = (BcmMibTable *)Table(*Pos).second;
#else
    pTable = (BcmMibTable *) *TablePos;
#endif

    pObject = pTable->FindObject (Name, Index);
    if (pObject)
      return pObject;

    SnmpLogInit << "No match " << this->Name() << " index " << Index << endl;

    TablePos++;
  }

  // No match in any tables either?  How about sub-groups...
  GroupPos = fGroups.begin();
  while (GroupPos != fGroups.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pGroup = (BcmMibGroup *)Group(*Pos).second;
#else
    pGroup = (BcmMibGroup *) *GroupPos;
#endif

    pObject = pGroup->FindObject (Name, Index);
    if (pObject)
      return pObject;

    SnmpLogInit << "No match " << Name << " index " << Index << " in group " << this->Name() << endl;

    GroupPos++;
  }

  SnmpLogInit << "No match" << endl;
  return NULL;  
*/ 
  MibObjectIterator Pos;
  BcmMibObject *pObject = NULL;
  BcmMibTable *pTable;  
  BcmMibGroup *pGroup;
  BcmObjectId MatchIx;
  
  SnmpLogSearch << fpName << "::FindObject " << Name << "." << Index << endl;
  
  if (fpAgent)
    fpAgent->LockMib();

  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
    pObject = (*Pos).second;
#else
    pObject = *Pos;
#endif

    SnmpLogSearch << "Check object " << (void *) pObject << endl;

    if ((pObject->ObjectType() == kTabularMibObject)
    ||  (pObject->ObjectType() == kScalarMibObject))
    {
      pObject->GetIndex(MatchIx);
      if ((Name == pObject->Name())
      &&  (MatchIx == Index))
        break;
      else
        SnmpLogSearch << pObject->Name() << "." << MatchIx << " - no match." << endl;
    }
    else if (pObject->ObjectType() == kMibTable)
    {
      pTable = (BcmMibTable *) pObject;
      pObject = pTable->FindObject(Name, Index);
      if (pObject)
        break;
    }
    else if (pObject->ObjectType() == kMibGroup)
    {
      pGroup = (BcmMibGroup *) pObject;
      pObject = pGroup->FindObject(Name, Index);
      if (pObject)
        break;
    }

    Pos++;
    pObject = NULL;
  }

  if (pObject)
    SnmpLogInit << "Found matching object " << pObject->Name() << endl;
  else
    SnmpLogInit << "No match " << this->Name() << " index " << Index << endl;
    
  if (fpAgent)
    fpAgent->UnlockMib();

  return pObject;
}

BcmMibObject *BcmMibGroup::FindObject (const BcmString &Name)
{
/*
  MibObjectIterator ScalarPos;
  MibTableIterator TablePos;
  MibGroupIterator GroupPos;
  BcmMibObject *pObject = NULL;
  BcmMibTable *pTable;  
  BcmMibGroup *pGroup;


  // Check scalar members first:
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
  ScalarPos = fScalars.find (OID);
  if (ScalarPos != fScalars.end())
    pObject = Scalar(*Pos).second;
#else
  ScalarPos = fScalars.begin();
  while (ScalarPos != fScalars.end())
  {
    pObject = *ScalarPos;
    if (pObject->Name() == Name)
      break;

    ScalarPos++;
    pObject = NULL;
  }
#endif

  if (pObject)
  {
    SnmpLogInit << "Found matching object " << pObject->Name() << endl;
    return pObject;
  }

  // If no scalar object matches, iterate through tables
  TablePos = fTables.begin();
  while (TablePos != fTables.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pTable = (BcmMibTable *)Table(*Pos).second;
#else
    pTable = (BcmMibTable *) *TablePos;
#endif

    pObject = pTable->FindObject (Name);
    if (pObject)
      return pObject;

    SnmpLogInit << "No match " << this->Name() << endl;

    TablePos++;
  }

  // No match in any tables either?  How about sub-groups...
  GroupPos = fGroups.begin();
  while (GroupPos != fGroups.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pGroup = (BcmMibGroup *)Group(*Pos).second;
#else
    pGroup = (BcmMibGroup *) *GroupPos;
#endif

    pObject = pGroup->FindObject (Name);
    if (pObject)
      return pObject;

    SnmpLogInit << "No match " << Name << " in group " << this->Name() << endl;

    GroupPos++;
  }

  SnmpLogInit << "No match" << endl;
  return NULL;
*/

  MibObjectIterator Pos;
  BcmMibObject *pObject = NULL;
  BcmMibTable *pTable;  
  BcmMibGroup *pGroup;
  
  
  if (fpAgent)
    fpAgent->LockMib();

  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
    pObject = (*Pos).second;
#else
    pObject = *Pos;
#endif
    if ((pObject->ObjectType() == kScalarMibObject)
    ||  (pObject->ObjectType() == kTabularMibObject))
    {
      if (Name == pObject->Name())
        break;
    }
    else if (pObject->ObjectType() == kMibTable)
    {
      pTable = (BcmMibTable *) pObject;
      pObject = pTable->FindObject(Name);
      if (pObject)
        break;
    }
    else if (pObject->ObjectType() == kMibGroup)
    {
      pGroup = (BcmMibGroup *) pObject;
      pObject = pGroup->FindObject(Name);
      if (pObject)
        break;
    }

    Pos++;
    pObject = NULL;
  }

  if (pObject)
    SnmpLogInit << "Found matching object " << pObject->Name() << endl;
  else
    SnmpLogInit << "No match" << endl;
    
  if (fpAgent)
    fpAgent->UnlockMib();

  return pObject;
}


SNMP_STATUS BcmMibGroup::SetAgent (BcmSnmpAgent *pAgent)
{
/*
  SNMP_STATUS Status = SNMP_NO_SUCH_NAME;
  MibObjectIterator ScalarPos;
  MibTableIterator TablePos;
  MibGroupIterator GroupPos;
  BcmMibObject *pObject = NULL;
  BcmMibTable *pTable;  
  BcmMibGroup *pGroup;


  // Check scalar members first:
  ScalarPos = fScalars.begin();
  while (ScalarPos != fScalars.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pObject = (BcmMibObject *) Scalar(*Pos).second;
#else
    pObject = (BcmMibObject *) *ScalarPos;
#endif

    pObject->SetAgent (pAgent);

    ScalarPos++;
  }

  // Next tables...
  TablePos = fTables.begin();
  while (TablePos != fTables.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pTable = (BcmMibTable *) Table(*Pos).second;
#else
    pTable = (BcmMibTable *) *TablePos;
#endif

    pTable->SetAgent (pAgent);

    TablePos++;
  }

  // Finally, sub-groups
  GroupPos = fGroups.begin();
  while (GroupPos != fGroups.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pGroup = (BcmMibGroup *) Group(*Pos).second;
#else
    pGroup = (BcmMibGroup *) *GroupPos;
#endif

    pGroup->SetAgent (pAgent);

    GroupPos++;
  }

  return BcmMibObject::SetAgent (pAgent);
*/
  MibObjectIterator Pos;
  BcmMibObject *pObject;
  BcmSnmpAgent *pOldAgent = fpAgent, *pNewAgent = pAgent;

  
  if (pOldAgent)
    pOldAgent->LockMib();
  if (pNewAgent)
    pNewAgent->LockMib();
  
  // Iterate through everything and set the agent.
  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pObject = (*Pos).second;
#else
    pObject = *Pos;
#endif

    pObject->SetAgent (pAgent);

    Pos++;
  }
  
  if (pOldAgent)
    pOldAgent->UnlockMib();
  if (pNewAgent)
    pNewAgent->UnlockMib();

  return BcmMibObject::SetAgent (pAgent);
}

BcmMibTable *BcmMibGroup::FindTable (const BcmObjectId &Oid)
{
/*
  MibTableIterator Pos;
  MibGroupIterator GroupPos;
  BcmMibTable *pTable;
  BcmMibGroup *pGroup;
  BcmObjectId MatchOid;


  SnmpLogInit << "BcmMibGroup " << Name() << " Find table " << Oid << endl;

  //if (fTables.empty())
  //return NULL;

  // Iterate through tables
  Pos = fTables.begin();
  while (Pos != fTables.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pTable = (BcmMibTable *)(*Pos).second;
#else
    pTable = (BcmMibTable *) *Pos;
#endif
    pTable->GetOid(MatchOid);

    if (MatchOid == Oid)
      return pTable;

    if (MatchOid.IsAscendant (Oid))
      return pTable;

    Pos++;
  }

  // No match in the tables?  Wha' bout sub groups...
  GroupPos = fGroups.begin();
  while (GroupPos != fGroups.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pGroup = (BcmMibGroup *)Group(*Pos).second;
#else
    pGroup = (BcmMibGroup *) *GroupPos;
#endif

    pTable = pGroup->FindTable (Oid);
    if (pTable)
      return pTable;

    SnmpLogInit << "No match " << Name() << "." << pGroup->Name() << endl;

    GroupPos++;
  }

  return NULL;  
*/
  MibObjectIterator Pos;
  BcmMibObject *pObject;
  BcmMibTable *pTable;
  BcmMibGroup *pGroup;
  BcmObjectId MatchOid;


  SnmpLogInit << "BcmMibGroup " << Name() << " Find table " << Oid << endl;
  
  GetOid (MatchOid);
  if (MatchOid.IsAscendant(Oid) == false)
    return NULL;
  
  if (fpAgent)
    fpAgent->LockMib();

  // Iterate
  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pObject = (*Pos).second;
#else
    pObject = *Pos;
#endif
    if (pObject->ObjectType() == kMibTable)
    {
      pTable = (BcmMibTable *) pObject;
      pTable->GetOid(MatchOid);

      if (MatchOid == Oid)
      {
        if (fpAgent)
          fpAgent->UnlockMib();
        return pTable;
      }

      if (MatchOid.IsAscendant (Oid))
      {
        if (fpAgent)
          fpAgent->UnlockMib();
        return pTable;
      }
    }
    if (pObject->ObjectType() == kMibGroup)
    {
      pGroup = (BcmMibGroup *) pObject;
      pTable = pGroup->FindTable (Oid);
      if (pTable)
      {
        if (fpAgent)
          fpAgent->UnlockMib();
        return pTable;
      }
    }

    Pos++;
  }
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return NULL;  
}

BcmMibTable *BcmMibGroup::FindTable (const BcmString &TableName)
{
/*
  MibTableIterator Pos;
  MibGroupIterator GroupPos;
  BcmMibTable *pTable;
  BcmMibGroup *pGroup;


  //if (fTables.empty())
  //return NULL;

  // Iterate through tables
  Pos = fTables.begin();
  while (Pos != fTables.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pTable = (BcmMibTable *)(*Pos).second;
#else
    pTable = (BcmMibTable *) *Pos;
#endif

    if (pTable->Name() == TableName)
      return pTable;

    Pos++;
  }

  // No match in the tables?  Wha' bout sub groups...
  GroupPos = fGroups.begin();
  while (GroupPos != fGroups.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pGroup = (BcmMibGroup *)Group(*Pos).second;
#else
    pGroup = (BcmMibGroup *) *GroupPos;
#endif

    pTable = pGroup->FindTable (TableName);
    if (pTable)
      return pTable;

    SnmpLogInit << "No match in group " << Name() << " for table " << TableName << endl;

    GroupPos++;
  }

  return NULL;  
*/
  MibObjectIterator Pos;
  BcmMibObject *pObject;
  BcmMibTable *pTable;
  BcmMibGroup *pGroup;


  if (fpAgent)
    fpAgent->LockMib();

  // Iterate
  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pObject = (*Pos).second;
#else
    pObject = *Pos;
#endif

    if (pObject->ObjectType() == kMibTable)
    {
      pTable = (BcmMibTable *)pObject;
      if (TableName == pTable->Name())
      {
        if (fpAgent)
          fpAgent->UnlockMib();
        return pTable;
      }
    }
    else if (pObject->ObjectType() == kMibGroup)
    {
      pGroup = (BcmMibGroup *)pObject;
      pTable = pGroup->FindTable (TableName);
      if (pTable)
      {
        if (fpAgent)
          fpAgent->UnlockMib();
        return pTable;
      }
    }

    Pos++;
  }
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return NULL;  
}

BcmMibGroup *BcmMibGroup::FindGroup (const BcmObjectId &Oid)
{
/*
  MibGroupIterator Pos;
  BcmMibGroup *pGroup;
  BcmObjectId MatchOid;


  SnmpLogInit << "BcmMibGroup " << Name() << " FindGroup " << Oid << endl;

  if (fGroups.empty())
    return NULL;

  // Iterate through Groups
  Pos = fGroups.begin();
  while (Pos != fGroups.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pGroup = (BcmMibGroup *)(*Pos).second;
#else
    pGroup = (BcmMibGroup *) *Pos;
#endif
    pGroup->GetOid(MatchOid);

    if (MatchOid == Oid)
      return pGroup;

    if (MatchOid.IsAscendant (Oid))
      return pGroup;

    Pos++;
  }

  return NULL;  
*/
  MibObjectIterator Pos;
  BcmMibObject *pObject;
  BcmObjectId MatchOid;


  SnmpLogInit << "BcmMibGroup " << Name() << " FindGroup " << Oid << endl;
  
  if (fObjects.empty())
    return NULL;
    
  if (fpAgent)
    fpAgent->LockMib();

  // Iterate through Groups and look for the indicated one.
  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pObject = (*Pos).second;
#else
    pObject = *Pos;
#endif
    
    if (pObject->ObjectType() == kMibGroup)
    {
      BcmMibGroup *pGroup = (BcmMibGroup *)pObject;
      
      pGroup->GetOid(MatchOid);
      
      // If this group matches, return him and we're done.
      if (MatchOid == Oid)
      {
        if (fpAgent)
          fpAgent->UnlockMib();
        return pGroup;
      }

      // If no match, see if he has a matching subgroup by calling
      // FindGroup on him.  Don't bother unless his OID ascends the one
      // of interest.
      if (MatchOid.IsAscendant (Oid))
      {
        pGroup = pGroup->FindGroup (Oid);
        if (pGroup)
        {
          if (fpAgent)
            fpAgent->UnlockMib();
          return pGroup;
        }
      }
    }

    Pos++;
  }
  
  if (fpAgent)
    fpAgent->UnlockMib();

  return NULL;  
}

BcmMibObject *BcmMibGroup::FindOtherInstance (const BcmObjectId &OID, const BcmMibRequest *pReq)
{
  MibObjectIterator Pos;
  BcmMibObject *pObject = NULL;
  BcmMibTable *pTable = NULL;  
  BcmMibGroup *pGroup = NULL;
  BcmObjectId MatchOid;


  GetOid (MatchOid);

  if (MatchOid.IsAscendant (OID) == false)
    return NULL;
    
  SnmpLogReq << "Checking group " << Name() << " for other instance of " << OID << endl;
    
  if (fpAgent)
    fpAgent->LockMib();

  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
    pObject = *Pos;
    pObject->GetOid(MatchOid);
    if (pObject->ObjectType() == kScalarMibObject)
    {
      BcmObjectId BaseOid;
      
      // Get the base OID of this scalar by nixing the last element of the
      // OID.
      for (int Loop = 0; Loop < MatchOid.Length()-1; Loop++)
        BaseOid += MatchOid[Loop];
        
      // Check to see if this is another instance of the supplied OID.
      if ((BaseOid == OID)
      ||  (BaseOid.IsAscendant(OID)))
      {
        SnmpLogReq << "Found a matching scalar: " << pObject->Name() << endl;
        break;
      }
    }
    else if (pObject->ObjectType() == kMibTable)
    {
      pTable = (BcmMibTable *) pObject;
      pObject = NULL;

      
      // Does this table's OID ascend from the one of interest?
      // Note that in order to get noSuchInstance, we must also require that
      // the OID is of a valid column in the table.  As such, the OID must be
      // at least 2 elements longer than the table OID.
      if ((MatchOid.IsAscendant(OID))
      &&  (OID.Length() >= MatchOid.Length() + 2))
      {
        MibEntryIterator EPos;
        BcmMibEntry *pEntry;
        
        SnmpLogReq << "Found a matching table: " << pTable->Name() << endl;
        
        // Iterate through entries, find the first accessible one.
        pEntry = pTable->FirstEntry (EPos);
        while (pEntry)
        {
          pEntry->GetOid(MatchOid);
          if (MatchOid.IsAscendant(OID))
          {
            if (pEntry->IsAccessible((BcmMibRequest *)pReq) == true)
            {
              SnmpLogReq << "Found an accessible entry!" << endl;
              
              // Extract the index of this entry, then check to make sure
              // that in fact this entry contains the object of interest
              // by constructing the new OID.
              BcmObjectId Index, OtherOID;
              
              pEntry->GetIndex(Index);
              
/*              
              1/8/2004: This approach fails if the index of the object passed
                        in is shorter than the length of the index of this
                        entry, but still not empty.
                        Example: ipAdEntAddr.192.168.100    when the entry is
                                 ipAdEntAddr.192.168.100.1.
                        See new approach, below.
                                 
              // Now, one possibility is that this OID has no index at all.
              // That is, say somebody has just requested ifDescr.  In that
              // case, calculate the base OID length as being the full length
              // of the supplied OID.
              int BaseOidLen;
              
              if (OID.Length() == MatchOid.Length() + 1)
                BaseOidLen = OID.Length();
                
              // Otherwise, we'll subtract the length of the index from the
              // length of the supplied OID to arrive at the base OID length.
              else
                BaseOidLen = OID.Length() - Index.Length();

              // The OID we're going to try to find has the base of the 
              // supplied OID but the index of this entry we just found.
              // So start building it with the base OID...
              for (int Loop = 0; Loop < BaseOidLen; Loop++)
                OtherOID += OID[Loop];
                
              // Then append the index.
              OtherOID += Index;
*/
              /*\
               *  We want to make sure that the provided column is present in
               *  this entry.  So here's how we'll construct the OID of interest.  
               *  Start with the OID of the entry.  Next, add the column number 
               *  from the OID we were handed.  Finally, tack on the index.
              \*/
              
              // Step 1: get the entry's OID.
              OtherOID = MatchOid;
              
              // Step 2: get the column from the OID that came in.  This will
              // be the next OID element after the length of the base OID that
              // we just got.
              OtherOID += OID[OtherOID.Length()];
              
              // Step 3: add the index.
              OtherOID += Index;
              
              SnmpLogReq << "Checking for OID " << OtherOID << endl;
              
              // See if that object really exists in this entry.
              pObject = pEntry->FindObject (OtherOID);
              if (pObject)
              {
                SnmpLogReq << "Found matching object" << endl;
                
                // Finally, verify its accessibility.
                if (pObject->IsAccessible ((BcmMibRequest *)pReq) == false)
                  SnmpLogReq << "Not accessible." << endl;
                else
                  break;
              }
              else
              {
                SnmpLogReq << "No matching object in this entry!" << endl;
                fpAgent->UnlockMib();
                return NULL;
              }
            }
            else
              SnmpLogReq << "Entry is not accessible." << endl;
          }
          pEntry = pTable->NextEntry (EPos);
        }
        
        // If there is no accessible entry, then see if the requested
        // column exists.
        if (pObject == NULL)
        {
          if (pTable->IsAccessible() == true)
          {
            unsigned long Column;
            BcmObjectId TableOID;
            
            // Determine the column number that's being asked for.  Start with
            // the table's OID, then the column is the element of the requested
            // OID which is 2 locations past the end of the table's OID.
            pTable->GetOid (TableOID);
            
            Column = OID[TableOID.Length() + 1];
          
            // If the table's empty but the requested column does exist,
            // the just return the table.
            if (pTable->DoesColumnExist(Column) == true)
              pObject = pTable;
          }
        }
          
        break;
      }
    }
    else if (pObject->ObjectType() == kMibGroup)
    {
      pGroup = (BcmMibGroup *) pObject;
      pObject = pGroup->FindOtherInstance (OID, pReq);
      if (pObject)
        break;
    }

    Pos++;
    pObject = NULL;
  }
  
  if (pObject)
  {
    SnmpLogReq << "Found another instance: " << pObject->Name() << endl;
   
    // If this is an actual object (not a table, entry, or group),
    // then check accessibility.
    if ((pObject->ObjectType() == kScalarMibObject)
    ||  (pObject->ObjectType() == kTabularMibObject))
    {
      if ((pObject->Access() == kNotAccessible) 
      ||  (pObject->Access() == kAccessibleForNotify)) 
      {
        SnmpLogReq << "Access = not-accessible, no match." << endl;
        pObject = NULL;
      }
    }
  }
  else
    SnmpLogReq << "Found no other instance." << endl;

  if ((pObject) && (pObject->IsAccessible((BcmMibRequest *)pReq) == false))
  {
    SnmpLogReq << "Not accessible, no match." << endl;
    pObject = NULL;
  }
    
  if (fpAgent)
    fpAgent->UnlockMib();
    
  return pObject;  
}

/*\
 *  StringMibObject
\*/

StringMibObject::StringMibObject (const char *pOID, const char *pName, BcmMibObject *pParent, MibObjectAccess Access)
: BcmMibObject (pOID, pName, pParent, Access)
{
}

StringMibObject::~StringMibObject ()
{
}

SNMP_STATUS StringMibObject::Get (BcmVarBind &VB)
{
  if (fpParent == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;
    
  return fpParent->GetLeafValue (*this, VB);
}

SNMP_STATUS StringMibObject::Test (BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_OPINION;


  SnmpLogSearch << "Testing string object " << Name() << " set to " << VB << endl;
  
  // PR 6320: First, if our access is specified, check access and type
  // restrictions.  If access is not specified, we must defer type checking to
  // the Set method, lest we check type before accessibility and return 
  // wrongType instead of notWritable for R/O objects set to the wrong type.
  if (Access() != kUnspecified)
  {
    // Check access
    if ((Access() != kReadWrite) && (Access() != kReadCreate))
      return SNMP_NOT_WRITABLE;
      
    // Check type.
    BcmString TestVal;
    Status = VB.GetString(TestVal);
    
    if (Status != SNMP_NO_ERROR)
      return Status;
  }

  // Check parental constraints if we have a parent...
  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling TestLeafValue." << endl;
    Status = fpParent->TestLeafValue (*this, VB);
  }

  SnmpLogSearch << "  Object test complete, return value = " << Status << endl;

  return Status;
}

SNMP_STATUS StringMibObject::TestDefaults (const BcmString &TestVal)
{
  unsigned int Loop;
  SNMP_STATUS Status = SNMP_NO_ERROR;
  

  if (TestVal.length() > 255)
  {
    SnmpLogReq << "  String length is too long: " << TestVal.length() << " WRONG_VALUE." << endl;
    Status = SNMP_WRONG_LENGTH;
  }
  else
  {
    for (Loop = 0; Loop < TestVal.length(); Loop++)
    {
      // Must be ASCII characters only! Note: use "C" function isprint here
      // instead of just checking MSB of octet.
      if (isprint (TestVal[Loop]) == 0) //  & 0x80)
      {
        SnmpLogReq << "  String contains non-ASCII character(s), WRONG_VALUE." << endl;

        Status = SNMP_WRONG_VALUE;
        break;
      }

      // If we have a \r, it must be followed by \n or NULL.
      if ((TestVal[Loop] == '\r')
      &&  (TestVal[Loop+1] != '\n')
      &&  (TestVal[Loop+1] != 0))
      {
        SnmpLogReq << "  String contains \r not followed by \n or \0, BAD_VALUE." << endl;

        Status = SNMP_WRONG_VALUE;
        break;
      }
    }
  }
  
  return Status;
}

SNMP_STATUS StringMibObject::Set (BcmVarBind &VB)
{
  BcmString TestVal;
  SNMP_STATUS Status = SNMP_NO_ERROR;


  SnmpLogSearch << "Setting string object " << Name() << " to " << VB << endl;

  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling SetLeafValue." << endl;
    Status = fpParent->SetLeafValue (*this, VB);
  }
  
  // The parent SHOULD verify that we got passed the proper type, but just
  // in case he didn't, check it here.  Also, in case we got no parent.
  if ((Status == SNMP_NO_ERROR) || (Status == SNMP_NO_OPINION))
  {
    SNMP_STATUS GetStatus = VB.GetString (TestVal);
    if (GetStatus != SNMP_NO_ERROR)
    {
      SnmpLogSearch << "  Set to type other than STRING, WRONG_TYPE" << endl;
      return GetStatus;
    }
  }

  if ((Status != SNMP_NO_ERROR)
  &&  (Status != SNMP_NO_OPINION))
  {
    SnmpLogSearch << "  Set failed, return value " << Status << endl;
    return Status;
  }

  return SNMP_NO_ERROR;
}


/*\
 *  IpAddrMibObject
\*/

IpAddrMibObject::IpAddrMibObject (const char *pOID, const char *pName, BcmMibObject *pParent, MibObjectAccess Access)
: BcmMibObject (pOID, pName, pParent, Access)
{
}

IpAddrMibObject::~IpAddrMibObject ()
{
}

SNMP_STATUS IpAddrMibObject::Get (BcmVarBind &VB)
{
  if (fpParent == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;
    
  return fpParent->GetLeafValue (*this, VB);
}

SNMP_STATUS IpAddrMibObject::Test (BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_OPINION;


  SnmpLogSearch << "Testing IP addr object " << Name() << " set to " << VB << endl;
  
  // PR 6320: First, if our access is specified, check access and type
  // restrictions.  If access is not specified, we must defer type checking to
  // the Set method, lest we check type before accessibility and return 
  // wrongType instead of notWritable for R/O objects set to the wrong type.
  if (Access() != kUnspecified)
  {
    // Check access
    if ((Access() != kReadWrite) && (Access() != kReadCreate))
      return SNMP_NOT_WRITABLE;
      
    // Check type.
    BcmIpAddress TestVal;
    Status = VB.GetIp (TestVal);
    
    if (Status != SNMP_NO_ERROR)
      return Status;
  }

  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling TestLeafValue." << endl;
    Status = fpParent->TestLeafValue (*this, VB);
  }

  SnmpLogSearch << "  Object test complete, return value = " << Status << endl;

  return Status;
}

SNMP_STATUS IpAddrMibObject::TestDefaults (const BcmIpAddress &TestVal)
{
  // No constraints.
  return SNMP_NO_ERROR;
}

SNMP_STATUS IpAddrMibObject::Set (BcmVarBind &VB)
{
  unsigned char TestIp[4];
  SNMP_STATUS Status = SNMP_NO_ERROR;


  SnmpLogSearch << "Setting IP addr object " << Name() << " to " << VB << endl;

  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling SetLeafValue." << endl;
    Status = fpParent->SetLeafValue (*this, VB);
  }
  
  // The parent SHOULD verify that we got passed the proper type, but just
  // in case he didn't, check it here.  Also, in case we got no parent.
  if ((Status == SNMP_NO_ERROR) || (Status == SNMP_NO_OPINION))
  {
    SNMP_STATUS GetStatus = VB.GetIp (TestIp);
    if (GetStatus != SNMP_NO_ERROR)
    {
      SnmpLogSearch << "  Set to type other than IP addr, WRONG_TYPE" << endl;
      return GetStatus;
    }
  }

  if ((Status != SNMP_NO_ERROR)
  &&  (Status != SNMP_NO_OPINION))
  {
    SnmpLogSearch << "  Set failed, return value " << Status << endl;
    return Status;
  }

  return SNMP_NO_ERROR;
}


/*\
 *  MacAddrMibObject
\*/

MacAddrMibObject::MacAddrMibObject (const char *pOID, const char *pName, BcmMibObject *pParent, MibObjectAccess Access)
: BcmMibObject (pOID, pName, pParent, Access)
{
  fSyntax = kMacAddrSyntax;
}

MacAddrMibObject::~MacAddrMibObject ()
{
}

SNMP_STATUS MacAddrMibObject::SetSyntax (MacAddrSyntax Syntax)
{
  if ((Syntax != kMacAddrSyntax) && (Syntax != kPhysAddrSyntax))
    return SNMP_WRONG_VALUE;

  fSyntax = Syntax;
  return SNMP_NO_ERROR;
}

int MacAddrMibObject::GetSyntax ()
{
  return fSyntax;
}


SNMP_STATUS MacAddrMibObject::Get (BcmVarBind &VB)
{
  SNMP_STATUS Status;
  

  if (fpParent == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;
    
  Status = fpParent->GetLeafValue (*this, VB);

  if ((Status != SNMP_NO_ERROR)
  &&  (Status != SNMP_NO_OPINION))
    return Status;

  // Special case: if all 6 bytes are zero's, return empty string.
  // This is actually for syntax PhysAddress, rather than MacAddress.
  if (fSyntax == kPhysAddrSyntax)
  {
    BcmMacAddress AllZerosMac(0,0,0,0,0,0), VbMac;
    
    VB.GetMacAddr (VbMac);
    
    if (VbMac == AllZerosMac)
      VB.SetString ("", 0);
  }
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS MacAddrMibObject::Test (BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_OPINION;


  SnmpLogSearch << "Testing MAC addr object " << Name() << " set to " << VB << endl;
  
  // PR 6320: First, if our access is specified, check access and type
  // restrictions.  If access is not specified, we must defer type checking to
  // the Set method, lest we check type before accessibility and return 
  // wrongType instead of notWritable for R/O objects set to the wrong type.
  if (Access() != kUnspecified)
  {
    // Check access
    if ((Access() != kReadWrite) && (Access() != kReadCreate))
      return SNMP_NOT_WRITABLE;
      
    // Check type.
    BcmMacAddress TestVal;
    Status = VB.GetMacAddr (TestVal);
    
    if (Status != SNMP_NO_ERROR)
      return Status;
  }

  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling TestLeafValue." << endl;
    Status = fpParent->TestLeafValue (*this, VB);
  }

  return Status;
}

SNMP_STATUS MacAddrMibObject::TestDefaults (const BcmMacAddress &TestVal)
{
  // No constraints.
  return SNMP_NO_ERROR;
}

SNMP_STATUS MacAddrMibObject::Set (BcmVarBind &VB)
{
  unsigned char TestVal[6];
  SNMP_STATUS Status = SNMP_NO_ERROR;


  SnmpLogSearch << "Setting MAC addr object " << Name() << " to " << VB << endl;

  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling SetLeafValue." << endl;
    Status = fpParent->SetLeafValue (*this, VB);
  }
  
  // The parent SHOULD verify that we got passed the proper type, but just
  // in case he didn't, check it here.  Also, in case we got no parent.
  if ((Status == SNMP_NO_ERROR) || (Status == SNMP_NO_OPINION))
  {
    SNMP_STATUS GetStatus = VB.GetString (TestVal,6);
    if (GetStatus != SNMP_NO_ERROR)
    {
      SnmpLogSearch << "  Set to type other than STRING, WRONG_TYPE" << endl;
      return GetStatus;
    }
  }

  if ((Status != SNMP_NO_ERROR)
  &&  (Status != SNMP_NO_OPINION))
  {
    SnmpLogSearch << "  Set failed, return value " << Status << endl;
    return Status;
  }

  return SNMP_NO_ERROR;
}


/*\
 *  Int32MibObject
\*/

Int32MibObject::Int32MibObject (const char *pOID, const char *pName, BcmMibObject *pParent, MibObjectAccess Access)
: BcmMibObject (pOID, pName, pParent, Access)
{
}

Int32MibObject::~Int32MibObject ()
{
}

SNMP_STATUS Int32MibObject::Get (BcmVarBind &VB)
{
  if (fpParent == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;
    
  return fpParent->GetLeafValue (*this, VB);
}

SNMP_STATUS Int32MibObject::Test (BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_OPINION;


  SnmpLogSearch << "Testing Int32 object " << Name() << " set to " << VB << endl;
  
  // PR 6320: First, if our access is specified, check access and type
  // restrictions.  If access is not specified, we must defer type checking to
  // the Set method, lest we check type before accessibility and return 
  // wrongType instead of notWritable for R/O objects set to the wrong type.
  if (Access() != kUnspecified)
  {
    // Check access
    if ((Access() != kReadWrite) && (Access() != kReadCreate))
      return SNMP_NOT_WRITABLE;
      
    // Check type.
    int TestVal;
    Status = VB.GetInt32 (&TestVal);
    
    if (Status != SNMP_NO_ERROR)
      return Status;
  }

  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling TestLeafValue." << endl;
    Status = fpParent->TestLeafValue (*this, VB);
  }
  
  return Status;
}

SNMP_STATUS Int32MibObject::TestDefaults (int TestVal)
{
  // No constraints.
  return SNMP_NO_ERROR;
}

SNMP_STATUS Int32MibObject::Set (BcmVarBind &VB)
{
  int TestVal;
  SNMP_STATUS Status = SNMP_NO_ERROR;


  SnmpLogSearch << "Setting Int32 object " << Name() << " to " << VB << endl;

  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling SetLeafValue." << endl;
    Status = fpParent->SetLeafValue (*this, VB);
  }
  
  // The parent SHOULD verify that we got passed the proper type, but just
  // in case he didn't, check it here.  Also, in case we got no parent.
  if ((Status == SNMP_NO_ERROR) || (Status == SNMP_NO_OPINION))
  {
    SNMP_STATUS GetStatus = VB.GetInt32(&TestVal);
    if (GetStatus != SNMP_NO_ERROR)
    {
      SnmpLogSearch << "  Set to type other than INT32, WRONG_TYPE" << endl;
      return GetStatus;
    }
  }

  if ((Status != SNMP_NO_ERROR)
  &&  (Status != SNMP_NO_OPINION))
  {
    SnmpLogSearch << "  Set failed, return value " << Status << endl;
    return Status;
  }

  return SNMP_NO_ERROR;
}


/*\
 *  Uint32MibObject
\*/

Uint32MibObject::Uint32MibObject (const char *pOID, const char *pName, BcmMibObject *pParent, MibObjectAccess Access)
: BcmMibObject (pOID, pName, pParent, Access)
{
}

Uint32MibObject::~Uint32MibObject ()
{
}

SNMP_STATUS Uint32MibObject::Get (BcmVarBind &VB)
{
  if (fpParent == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;
    
  return fpParent->GetLeafValue (*this, VB);
}

SNMP_STATUS Uint32MibObject::Test (BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_OPINION;
  // unsigned int TestVal;
  // int TestType;


  SnmpLogSearch << "Testing UINT32 object " << Name() << " set to " << VB << endl;
  
  // PR 6320: First, if our access is specified, check access and type
  // restrictions.  If access is not specified, we must defer type checking to
  // the Set method, lest we check type before accessibility and return 
  // wrongType instead of notWritable for R/O objects set to the wrong type.
  if (Access() != kUnspecified)
  {
    // Check access
    if ((Access() != kReadWrite) && (Access() != kReadCreate))
      return SNMP_NOT_WRITABLE;
      
    // Check type.
    unsigned int TestVal;
    int TestType;
    Status = VB.GetUint32 (&TestVal, &TestType);
    
    if (Status != SNMP_NO_ERROR)
      return Status;
  }

  // Check parental constraints if we have a parent...
  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling TestLeafValue." << endl;
    Status = fpParent->TestLeafValue (*this, VB);
  }

  return Status;
}

SNMP_STATUS Uint32MibObject::TestDefaults (unsigned long TestVal)
{
  // No constraints.
  return SNMP_NO_ERROR;
}

SNMP_STATUS Uint32MibObject::Set (BcmVarBind &VB)
{
  unsigned int TestVal;
  int TestType;
  SNMP_STATUS Status = SNMP_NO_ERROR;


  SnmpLogSearch << "Setting UINT32 object " << Name() << " to " << VB << endl;

  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling SetLeafValue." << endl;
    Status = fpParent->SetLeafValue (*this, VB);
  }
  
  // The parent SHOULD verify that we got passed the proper type, but just
  // in case he didn't, check it here.  Also, in case we got no parent.
  if ((Status == SNMP_NO_ERROR) || (Status == SNMP_NO_OPINION))
  {
    SNMP_STATUS GetStatus = VB.GetUint32 (&TestVal, &TestType);
    if (GetStatus != SNMP_NO_ERROR)
    {
      SnmpLogSearch << "  Set to type other than UINT32, WRONG_TYPE" << endl;
      return GetStatus;
    }
  }

  if ((Status != SNMP_NO_ERROR)
  &&  (Status != SNMP_NO_OPINION))
  {
    SnmpLogSearch << "  Set failed, return value " << Status << endl;
    return Status;
  }

  return SNMP_NO_ERROR;
}


/*\
 *  Uint64MibObject
\*/

Uint64MibObject::Uint64MibObject (const char *pOID, const char *pName, BcmMibObject *pParent, MibObjectAccess Access)
: BcmMibObject (pOID, pName, pParent, Access)
{
}

Uint64MibObject::~Uint64MibObject ()
{
}

SNMP_STATUS Uint64MibObject::Get (BcmVarBind &VB)
{
  if (fpParent == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;
    
  return fpParent->GetLeafValue (*this, VB);
}

SNMP_STATUS Uint64MibObject::Test (BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_OPINION;
  // BcmCounter64 TestVal;


  SnmpLogSearch << "Testing UINT64 object " << Name() << " set to " << VB << endl;
  
  // PR 6320: First, if our access is specified, check access and type
  // restrictions.  If access is not specified, we must defer type checking to
  // the Set method, lest we check type before accessibility and return 
  // wrongType instead of notWritable for R/O objects set to the wrong type.
  if (Access() != kUnspecified)
  {
    // Check access
    if ((Access() != kReadWrite) && (Access() != kReadCreate))
      return SNMP_NOT_WRITABLE;
      
    // Check type.
    BcmCounter64 TestVal;
    Status = VB.GetUint64 (TestVal);
    
    if (Status != SNMP_NO_ERROR)
      return Status;
  }

  // Check parental constraints if we have a parent...
  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling TestLeafValue." << endl;
    Status = fpParent->TestLeafValue (*this, VB);
  }

  return Status;
}

SNMP_STATUS Uint64MibObject::TestDefaults (const BcmCounter64 &TestVal)
{
  // No constraints.
  return SNMP_NO_ERROR;
}

SNMP_STATUS Uint64MibObject::Set (BcmVarBind &VB)
{
  BcmCounter64 TestVal;
  SNMP_STATUS Status = SNMP_NO_ERROR;


  SnmpLogSearch << "Setting UINT64 object " << Name() << " to " << VB << endl;

  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling SetLeafValue." << endl;
    Status = fpParent->SetLeafValue (*this, VB);
  }
/*
  PR 6320: Don't check type here, because if we do we will wind up returning 
  wrongType rather than notWritable if somebody sets a R/O object to the wrong 
  type.
    
  // The parent SHOULD verify that we got passed the proper type, but just
  // in case he didn't, check it here.  Also, in case we got no parent.
  if ((Status == SNMP_NO_ERROR) || (Status == SNMP_NO_OPINION))
  {
    SNMP_STATUS GetStatus = VB.GetUint64 (TestVal);
    if (GetStatus != SNMP_NO_ERROR)
    {
      SnmpLogSearch << "  Set to type other than UINT64, WRONG_TYPE" << endl;
      return GetStatus;
    }
  }
*/
  if ((Status != SNMP_NO_ERROR)
  &&  (Status != SNMP_NO_OPINION))
  {
    SnmpLogSearch << "  Set failed, return value " << Status << endl;
    return Status;
  }

  return SNMP_NO_ERROR;
}



/*\
 *  TimeticksMibObject
\*/

TimeticksMibObject::TimeticksMibObject (const char *pOID, const char *pName, BcmMibObject *pParent, MibObjectAccess Access)
: BcmMibObject (pOID, pName, pParent, Access)
{
}

TimeticksMibObject::~TimeticksMibObject ()
{
}

SNMP_STATUS TimeticksMibObject::Get (BcmVarBind &VB)
{
  SNMP_STATUS Status;

  if (fpParent == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;

  Status = fpParent->GetLeafValue (*this, VB);
  
  if (Status != SNMP_NO_ERROR)
    return Status;
    
  // We now need to do some unit conversions.  Bridges that deal
  // with TimeticksMibObjects use time_t units, which have a precision
  // of one second, whereas the SNMP timeticks TC has a precision of .01
  // seconds.  So we need to multiply here by 100.  Note that MsTimeticksMibObject
  // is able to handle the full precision if required.
  unsigned long SecTicks;
  
  VB.GetTimeticks (&SecTicks);
  VB.SetTimeticks (SecTicks * 100);

  return SNMP_NO_ERROR;
}

SNMP_STATUS TimeticksMibObject::Test (BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_OPINION;


  SnmpLogSearch << "Testing string object " << Name() << " set to " << VB << endl;
  
  // PR 6320: First, if our access is specified, check access and type
  // restrictions.  If access is not specified, we must defer type checking to
  // the Set method, lest we check type before accessibility and return 
  // wrongType instead of notWritable for R/O objects set to the wrong type.
  if (Access() != kUnspecified)
  {
    // Check access
    if ((Access() != kReadWrite) && (Access() != kReadCreate))
      return SNMP_NOT_WRITABLE;
      
    // Check type.
    unsigned long TestVal;
    Status = VB.GetTimeticks (&TestVal);
    
    if (Status != SNMP_NO_ERROR)
      return Status;
  }

  // Check parental constraints if we have a parent...
  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling TestLeafValue." << endl;
    Status = fpParent->TestLeafValue (*this, VB);
  }

  return Status;
}

SNMP_STATUS TimeticksMibObject::TestDefaults (time_t TestVal)
{
  // No constraints.
  return SNMP_NO_ERROR;
}

SNMP_STATUS TimeticksMibObject::Set (BcmVarBind &VB)
{
  unsigned long Ticks;
  SNMP_STATUS Status = SNMP_NO_ERROR;


  SnmpLogSearch << "Setting timeticks object " << Name() << " to " << VB << endl;

  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling SetLeafValue." << endl;
    Status = fpParent->SetLeafValue (*this, VB);
  }
  
  // The parent SHOULD verify that we got passed the proper type, but just
  // in case he didn't, check it here.  Also, in case we got no parent.
  if ((Status == SNMP_NO_ERROR) || (Status == SNMP_NO_OPINION))
  {
    SNMP_STATUS GetStatus = VB.GetTimeticks (&Ticks);
    if (GetStatus != SNMP_NO_ERROR)
    {
      SnmpLogSearch << "  Set to type other than TIMETICKS, WRONG_TYPE" << endl;
      return GetStatus;
    }
  }

  if ((Status != SNMP_NO_ERROR)
  &&  (Status != SNMP_NO_OPINION))
  {
    SnmpLogSearch << "  Set failed, return value " << Status << endl;
    return Status;
  }

  return SNMP_NO_ERROR;
}


/*\
 *  MsTimeticksMibObject
 *  similar to TimeticksMibObject but deals in milliseconds instead of seconds.
\*/

MsTimeticksMibObject::MsTimeticksMibObject (const char *pOID, const char *pName, BcmMibObject *pParent, MibObjectAccess Access)
: BcmMibObject (pOID, pName, pParent, Access)
{
}

MsTimeticksMibObject::~MsTimeticksMibObject ()
{
}

SNMP_STATUS MsTimeticksMibObject::Get (BcmVarBind &VB)
{
  SNMP_STATUS Status;
  

  if (fpParent == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;
  
  Status = fpParent->GetLeafValue (*this, VB);
  
  if (Status != SNMP_NO_ERROR)
    return Status;
  
  // We now need to do some unit conversions.  Bridges that deal
  // with MsTimeticksMibObjects have a precision of 1 millisecond,
  // of one second, whereas timeticks SNMP TC has a precision of .01
  // seconds.  So we need to divide here by 10.  Note that TimeticksMibObject
  // uses 1 second precision.
  unsigned long MsecTicks;
  
  VB.GetTimeticks (&MsecTicks);
  VB.SetTimeticks (MsecTicks / 10);

  return SNMP_NO_ERROR;
}

SNMP_STATUS MsTimeticksMibObject::Test (BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_OPINION;


  SnmpLogSearch << "Testing MsTimeticks object " << Name() << " set to " << VB << endl;
  
  // PR 6320: First, if our access is specified, check access and type
  // restrictions.  If access is not specified, we must defer type checking to
  // the Set method, lest we check type before accessibility and return 
  // wrongType instead of notWritable for R/O objects set to the wrong type.
  if (Access() != kUnspecified)
  {
    // Check access
    if ((Access() != kReadWrite) && (Access() != kReadCreate))
      return SNMP_NOT_WRITABLE;
      
    // Check type.
    unsigned long TestVal;
    Status = VB.GetTimeticks (&TestVal);
    
    if (Status != SNMP_NO_ERROR)
      return Status;
  }

  // Check parental constraints if we have a parent...
  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling TestLeafValue." << endl;
    Status = fpParent->TestLeafValue (*this, VB);
  }

  return Status;
}

SNMP_STATUS MsTimeticksMibObject::TestDefaults (unsigned long TestVal)
{
  // No constraints.
  return SNMP_NO_ERROR;
}

SNMP_STATUS MsTimeticksMibObject::Set (BcmVarBind &VB)
{
  unsigned long Ticks;
  SNMP_STATUS Status = SNMP_NO_ERROR;


  SnmpLogSearch << "Setting MS timeticks object " << Name() << " to " << VB << endl;

  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling SetLeafValue." << endl;
    Status = fpParent->SetLeafValue (*this, VB);
  }
  
  // The parent SHOULD verify that we got passed the proper type, but just
  // in case he didn't, check it here.  Also, in case we got no parent.
  if ((Status == SNMP_NO_ERROR) || (Status == SNMP_NO_OPINION))
  {
    SNMP_STATUS GetStatus = VB.GetTimeticks (&Ticks);
    if (GetStatus != SNMP_NO_ERROR)
    {
      SnmpLogSearch << "  Set to type other than TIMETICKS, WRONG_TYPE" << endl;
      return GetStatus;
    }
  }

  if ((Status != SNMP_NO_ERROR)
  &&  (Status != SNMP_NO_OPINION))
  {
    SnmpLogSearch << "  Set failed, return value " << Status << endl;
    return Status;
  }

  return SNMP_NO_ERROR;
}


/*\
 *  RowStatusMibObject
\*/

RowStatusMibObject::RowStatusMibObject (BcmMibTable *pTable, BcmMibEntry *pEntry, const char *pOID, const char *pName, MibObjectAccess Access)
: Int32MibObject (pOID, pName, pEntry, Access)
{
  fpTable = pTable;
  fHowCreated = kUnspecified;
}

RowStatusMibObject::~RowStatusMibObject ()
{
}

SNMP_STATUS RowStatusMibObject::Get (BcmVarBind &VB)
{
  if (fpParent == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;
    
  return fpParent->GetLeafValue (*this, VB);
}

SNMP_STATUS RowStatusMibObject::Test (BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_OPINION;


  SnmpLogSearch << "Testing row-status object " << Name() << " set to " << VB << endl;
  
  // PR 6320: First, if our access is specified, check access and type
  // restrictions.  If access is not specified, we must defer type checking to
  // the Set method, lest we check type before accessibility and return 
  // wrongType instead of notWritable for R/O objects set to the wrong type.
  // Note: cast here is because we have a kUnspecified enum value for 
  // the RowStatusCreationMethod enum as well.
  if (Access() != (MibObjectAccess)kUnspecified)
  {
    // Check access.  This is a little over-cautious perhaps because who ever
    // heard of a RowStatus object that is not R/C, but still...
    if ((Access() != kReadWrite) && (Access() != kReadCreate))
      return SNMP_NOT_WRITABLE;
      
    // Check type.
    int TestVal;
    Status = VB.GetInt32 (&TestVal);
    
    if (Status != SNMP_NO_ERROR)
      return Status;
  }

  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling TestLeafValue." << endl;
    Status = fpParent->TestLeafValue (*this, VB);
  }

  return Status;
}

SNMP_STATUS RowStatusMibObject::TestDefaults (int TestVal, int CurVal)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  

  switch (TestVal)
  {
    case SNMP_STATUS_INVISIBLE:
      Status = SNMP_WRONG_VALUE;
      break;

    case SNMP_STATUS_ACTIVE:
    case SNMP_STATUS_NOT_IN_SERVICE:
      if (CurVal == SNMP_STATUS_INVISIBLE)
      {
        SnmpLogReq << "  RowStatus set to ACTIVE or NOT-IN-SERVICE for not-yet-created object.\n"
                      "  INCONSISTENT VALUE." << endl;
        Status = SNMP_INCONSISTENT_VALUE;
      }
      else
        Status = SNMP_NO_ERROR;
      break;
      
    case SNMP_STATUS_NOT_READY:
      SnmpLogReq << "  RowStatus set to NOT-READY.  INCONSISTENT VALUE." << endl;
      Status = SNMP_WRONG_VALUE;
      break;

    // If we are getting sets from the config file, and there is already
    // an existing entry in the table, don't do the usual thing of rejecting
    // CREATE_AND_WAIT or CREATE_AND_GO.  
    // 10/30/01: Why not?  Change this.  Pretty sure
    // this is leftover from a previous approach.
    case SNMP_STATUS_CREATE_AND_WAIT:
    case SNMP_STATUS_CREATE_AND_GO:
      if (CurVal == SNMP_STATUS_INVISIBLE)
        Status = SNMP_NO_ERROR;
      else
      {
        SnmpLogReq << "  RowStatus set to CREATE AND WAIT/GO for existing object.  INCONSISTENT VALUE." << endl;
        Status = SNMP_INCONSISTENT_VALUE;
      }
      break;

    case SNMP_STATUS_DESTROY:
      Status = SNMP_NO_ERROR;
      break;

    default:
      SnmpLogReq << "  RowStatus set to " << TestVal << ".  WRONG VALUE." << endl;
      Status = SNMP_WRONG_VALUE;
      break;
  }
    
  return Status;
}

SNMP_STATUS RowStatusMibObject::Set (BcmVarBind &VB)
{
  int CurVal, NewVal = -1;
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmObjectId MyOid, MyIndex;
  BcmVarBind MyVal;


  SnmpLogSearch << "Setting row-status object " << Name() << " to " << VB << endl;

  Status = Get (MyVal);
  if (Status != SNMP_NO_ERROR)
    return Status;
    
  MyVal.GetInt32 (&CurVal);
  VB.GetInt32(&NewVal);
  
  switch (NewVal)
  {
    case SNMP_STATUS_INVISIBLE:
      SnmpLogSearch << "  INVISIBLE" << endl;
      Status = SNMP_WRONG_VALUE;
      break;

    case SNMP_STATUS_ACTIVE:
    case SNMP_STATUS_NOT_IN_SERVICE:
      if (CurVal == SNMP_STATUS_INVISIBLE)
        Status = SNMP_INCONSISTENT_VALUE;
      else
        Status = fpParent->SetLeafValue (*this, VB);
      break;
      
    case SNMP_STATUS_NOT_READY:
      Status = SNMP_WRONG_VALUE;
      break;

    case SNMP_STATUS_CREATE_AND_WAIT:
      if (CurVal == SNMP_STATUS_INVISIBLE)
      {
        Status = fpParent->SetLeafValue (*this, VB);
        if (Status == SNMP_NO_ERROR)
        {
        }
        else if (Status == SNMP_NO_OPINION)
        {
          VB = SNMP_STATUS_NOT_IN_SERVICE;
        }
      }
      else
        Status = SNMP_INCONSISTENT_VALUE;
      break;

    case SNMP_STATUS_CREATE_AND_GO:
      if (CurVal == SNMP_STATUS_INVISIBLE)
      {
        Status = fpParent->SetLeafValue (*this, VB);
        if (Status == SNMP_NO_ERROR)
        {
        }
      }
      else
        Status = SNMP_INCONSISTENT_VALUE;
      break;

    case SNMP_STATUS_DESTROY:
      Status = fpParent->SetLeafValue (*this, VB);
      if (Status == SNMP_NO_ERROR)
      {
        GetOid (MyOid);
        fpTable->ExtractEntryIndex (MyOid, MyIndex);
        Status = fpTable->DestroyEntry (MyIndex);
      }
      break;

    default:
      Status = SNMP_WRONG_VALUE;
      break;
  }
  
  // The parent SHOULD verify that we got passed the proper type, but just
  // in case he didn't, check it here.  Also, in case we got no parent.
  if ((Status == SNMP_NO_ERROR) || (Status == SNMP_NO_OPINION))
  {
    SNMP_STATUS GetStatus = VB.GetInt32 (&NewVal);
    if (GetStatus != SNMP_NO_ERROR)
    {
      SnmpLogSearch << "  Set to type other than INT32, WRONG_TYPE" << endl;
      return GetStatus;
    }
  }

  if ((Status != SNMP_NO_ERROR)
  &&  (Status != SNMP_NO_OPINION))
  {
    SnmpLogSearch << "  Set failed, return value " << Status << endl;
    return Status;
  }

  return Status;
}

int RowStatusMibObject::CreationMethod () const
{
  return fHowCreated;
}

SNMP_STATUS RowStatusMibObject::SetParent (BcmMibObject *pParent)
{
  SNMP_STATUS Status = Int32MibObject::SetParent(pParent);
  
  if (Status != SNMP_NO_ERROR)
    return Status;

  // Note that we'd only expect a BcmMibEntry to be the parent of a 
  // RowStatusMibObject.  Still, check just to make sure.  Assuming it is
  // an entry, set its fpStatusObject member.
  if (pParent->ObjectType() == kMibEntry)
  {
    BcmMibEntry *pEntry = (BcmMibEntry *) pParent;
    pEntry->fpStatusObject = this;
  }
    
  return SNMP_NO_ERROR;
}


/*\
 *  TestAndIncrMibObject
\*/

/*
This object provides support for the TestAndIncr textual convention,
as described below (from RFC-1903)

TestAndIncr ::= TEXTUAL-CONVENTION
    STATUS       current
    DESCRIPTION
            "Represents integer-valued information used for atomic
            operations.  When the management protocol is used to specify
            that an object instance having this syntax is to be
            modified, the new value supplied via the management protocol
            must precisely match the value presently held by the
            instance.  If not, the management protocol set operation
            fails with an error of `inconsistentValue'.  Otherwise, if
            the current value is the maximum value of 2^31-1 (2147483647
            decimal), then the value held by the instance is wrapped to
            zero; otherwise, the value held by the instance is
            incremented by one.  (Note that regardless of whether the
            management protocol set operation succeeds, the variable-
            binding in the request and response PDUs are identical.)

            The value of the ACCESS clause for objects having this
            syntax is either `read-write' or `read-create'.  When an
            instance of a columnar object having this syntax is created,
            any value may be supplied via the management protocol.

            When the network management portion of the system is re-
            initialized, the value of every object instance having this
            syntax must either be incremented from its value prior to
            the re-initialization, or (if the value prior to the re-
            initialization is unknown) be set to a pseudo-randomly
            generated value."
    SYNTAX       INTEGER (0..2147483647)
*/

TestAndIncrMibObject::TestAndIncrMibObject (const char *pOID, const char *pName, BcmMibObject *pParent, MibObjectAccess Access)
: Int32MibObject (pOID, pName, pParent, Access)
{
  // Initialize value to random number as required above
  time_t Now;
  time (&Now);

  srand (Now);
  fValue = rand ();
}

TestAndIncrMibObject::~TestAndIncrMibObject ()
{
}

SNMP_STATUS TestAndIncrMibObject::Get (BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;


  SnmpLogSearch << "Getting TestAndIncr object " << Name() << endl;

  if (fpParent)
  {
    Status = fpParent->GetLeafValue (*this, VB);
    if (Status == SNMP_NO_ERROR)
      VB.GetInt32 (&fValue);
  }

  if ((Status != SNMP_NO_ERROR)
  &&  (Status != SNMP_NO_OPINION))
    return Status;

  VB.SetInt32(fValue);
  return SNMP_NO_ERROR;
}

SNMP_STATUS TestAndIncrMibObject::Test (BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_INCONSISTENT_VALUE;
  int TestVal;
  

  SnmpLogSearch << "Testing TestAndIncr object " << Name() << " set to " << VB << endl;
  
  // PR 6320: First, if our access is specified, check access and type
  // restrictions.  If access is not specified, we must defer type checking to
  // the Set method, lest we check type before accessibility and return 
  // wrongType instead of notWritable for R/O objects set to the wrong type.
  if (Access() != kUnspecified)
  {
    // Check access
    if ((Access() != kReadWrite) && (Access() != kReadCreate))
      return SNMP_NOT_WRITABLE;
      
    // Check type.
    Status = VB.GetInt32 (&TestVal);
    
    if (Status != SNMP_NO_ERROR)
      return Status;
  }

  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling TestLeafValue." << endl;
    Status = fpParent->TestLeafValue (*this, VB);
  }
  
  // Check generic TestAndIncr constraints if our parent defers to us.
  if (Status == SNMP_NO_OPINION)
  {
    SnmpLogSearch << "  Parent has NO_OPINION, applying default TestAndIncr constraints." << endl;
    Status = TestDefaults (TestVal, fValue);
  }

  return Status;
}

SNMP_STATUS TestAndIncrMibObject::TestDefaults (int TestVal, int CurVal)
{
  SNMP_STATUS Status;
  
  if (TestVal < 0)
    Status = SNMP_WRONG_VALUE;
  else if (TestVal != CurVal)
    Status = SNMP_INCONSISTENT_VALUE;
  else
    Status = SNMP_NO_ERROR;
    
  return Status;
}

SNMP_STATUS TestAndIncrMibObject::Set (BcmVarBind &VB)
{
  int Value;
  SNMP_STATUS Status = SNMP_NO_ERROR;
  BcmObjectId MyIndex;


  SnmpLogSearch << "Setting TestAndIncr object " << Name() << " to " << VB << endl;

  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling TestLeafValue." << endl;
    Status = fpParent->TestLeafValue (*this, VB);
  }
  
  // The parent SHOULD verify that we got passed the proper type, but just
  // in case he didn't, check it here.  Also, in case we got no parent.
  if ((Status == SNMP_NO_ERROR) || (Status == SNMP_NO_OPINION))
  {
    SNMP_STATUS GetStatus = VB.GetInt32 (&Value);
    if (GetStatus != SNMP_NO_ERROR)
    {
      SnmpLogSearch << "  Set to type other than INT32, WRONG_TYPE" << endl;
      return GetStatus;
    }
  }

  // Check generic TestAndIncr constraints
  if (Status == SNMP_NO_OPINION)
  {
    SnmpLogSearch << "  Parent has NO_OPINION, applying default TestAndIncr constraints." << endl;

    if (Value != fValue)
      Status = SNMP_INCONSISTENT_VALUE;
    else
    {
      // Increment: if we hit the max signed int value, roll over
      if (fValue == 2147483647)
        fValue = 0;
      else
        fValue++;

      Status = SNMP_NO_ERROR;
    }
  }
  else
    fValue = Value;

  if ((Status != SNMP_NO_ERROR)
  &&  (Status != SNMP_NO_OPINION))
  {
    SnmpLogSearch << "  Set failed, return value " << Status << endl;
    return Status;
  }

  return Status;
}


/*\
 *  OidMibObject
\*/

OidMibObject::OidMibObject (const char *pOID, const char *pName, BcmMibObject *pParent, MibObjectAccess Access)
: BcmMibObject (pOID, pName, pParent, Access)
{
}

OidMibObject::~OidMibObject ()
{
}

SNMP_STATUS OidMibObject::Get (BcmVarBind &VB)
{
  if (fpParent == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;
    
  return fpParent->GetLeafValue (*this, VB);
}

SNMP_STATUS OidMibObject::Test (BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_OPINION;


  SnmpLogSearch << "Testing OID object " << Name() << " set to " << VB << endl;
  
  // PR 6320: First, if our access is specified, check access and type
  // restrictions.  If access is not specified, we must defer type checking to
  // the Set method, lest we check type before accessibility and return 
  // wrongType instead of notWritable for R/O objects set to the wrong type.
  if (Access() != kUnspecified)
  {
    // Check access
    if ((Access() != kReadWrite) && (Access() != kReadCreate))
      return SNMP_NOT_WRITABLE;
      
    // Check type.
    BcmObjectId TestVal;
    Status = VB.GetOid (TestVal);
    
    if (Status != SNMP_NO_ERROR)
      return Status;
  }

  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling TestLeafValue." << endl;
    Status = fpParent->TestLeafValue (*this, VB);
  }

  return Status;
}

SNMP_STATUS OidMibObject::TestDefaults (const BcmObjectId &TestVal)
{
  // No constraints.
  return SNMP_NO_ERROR;
}

SNMP_STATUS OidMibObject::Set (BcmVarBind &VB)
{
  BcmObjectId TestVal;
  SNMP_STATUS Status = SNMP_NO_ERROR;


  SnmpLogSearch << "Setting OID object " << Name() << " to " << VB << endl;

  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling SetLeafValue." << endl;
    Status = fpParent->SetLeafValue (*this, VB);
  }
  
  // The parent SHOULD verify that we got passed the proper type, but just
  // in case he didn't, check it here.  Also, in case we got no parent.
  if ((Status == SNMP_NO_ERROR) || (Status == SNMP_NO_OPINION))
  {
    SNMP_STATUS GetStatus = VB.GetOid (TestVal);
    if (GetStatus != SNMP_NO_ERROR)
    {
      SnmpLogSearch << "  Set to type other than OID, WRONG_TYPE" << endl;
      return GetStatus;
    }
  }

  if ((Status != SNMP_NO_ERROR)
  &&  (Status != SNMP_NO_OPINION))
  {
    SnmpLogSearch << "  Set failed, return value " << Status << endl;
    return Status;
  }

  return SNMP_NO_ERROR;
}


/*\
 *  BitsMibObject
\*/

BitsMibObject::BitsMibObject (const char *pOID, const char *pName, BcmMibObject *pParent, MibObjectAccess Access)
: BcmMibObject (pOID, pName, pParent, Access)
{
}

BitsMibObject::~BitsMibObject ()
{
}

SNMP_STATUS BitsMibObject::Get (BcmVarBind &VB)
{
  if (fpParent == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;
    
  return fpParent->GetLeafValue (*this, VB);
}

SNMP_STATUS BitsMibObject::Test (BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_OPINION;


  SnmpLogSearch << "Testing string object " << Name() << " set to " << VB << endl;
  
  // PR 6320: First, if our access is specified, check access and type
  // restrictions.  If access is not specified, we must defer type checking to
  // the Set method, lest we check type before accessibility and return 
  // wrongType instead of notWritable for R/O objects set to the wrong type.
  if (Access() != kUnspecified)
  {
    // Check access
    if ((Access() != kReadWrite) && (Access() != kReadCreate))
      return SNMP_NOT_WRITABLE;
      
    // Check type.
    unsigned long TestVal;
    Status = VB.GetBits (&TestVal);
    
    if (Status != SNMP_NO_ERROR)
      return Status;
  }

  // Check parental constraints if we have a parent...
  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling TestLeafValue." << endl;
    Status = fpParent->TestLeafValue (*this, VB);
  }

  return Status;
}

SNMP_STATUS BitsMibObject::TestDefaults (unsigned long TestVal)
{
  // No constraints.
  return SNMP_NO_ERROR;
}

SNMP_STATUS BitsMibObject::Set (BcmVarBind &VB)
{
  unsigned long TestVal;
  SNMP_STATUS Status = SNMP_NO_ERROR;


  SnmpLogSearch << "Setting string object " << Name() << " to " << VB << endl;

  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling SetLeafValue." << endl;
    Status = fpParent->SetLeafValue (*this, VB);
  }
  
  // The parent SHOULD verify that we got passed the proper type, but just
  // in case he didn't, check it here.  Also, in case we got no parent.
  if ((Status == SNMP_NO_ERROR) || (Status == SNMP_NO_OPINION))
  {
    SNMP_STATUS GetStatus = VB.GetBits (&TestVal);
    if (GetStatus != SNMP_NO_ERROR)
    {
      SnmpLogSearch << "  Set to type other than BITS, WRONG_TYPE" << endl;
      return GetStatus;
    }
  }

  if ((Status != SNMP_NO_ERROR)
  &&  (Status != SNMP_NO_OPINION))
  {
    SnmpLogSearch << "  Set failed, return value " << Status << endl;
    return Status;
  }

  return SNMP_NO_ERROR;
}


/*\
 *  DateTimeMibObject
\*/

DateTimeMibObject::DateTimeMibObject (const char *pOID, const char *pName, BcmMibObject *pParent, MibObjectAccess Access)
: BcmMibObject (pOID, pName, pParent, Access)
{
}

DateTimeMibObject::~DateTimeMibObject ()
{
}

SNMP_STATUS DateTimeMibObject::Get (BcmVarBind &VB)
{
  if (fpParent == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;
  
  return fpParent->GetLeafValue (*this, VB);
}

SNMP_STATUS DateTimeMibObject::Test (BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_OPINION;
  // time_t TestVal;


  SnmpLogSearch << "Testing date/time object " << Name() << " set to " << VB << endl;
  
  // PR 6320: First, if our access is specified, check access and type
  // restrictions.  If access is not specified, we must defer type checking to
  // the Set method, lest we check type before accessibility and return 
  // wrongType instead of notWritable for R/O objects set to the wrong type.
  if (Access() != kUnspecified)
  {
    // Check access
    if ((Access() != kReadWrite) && (Access() != kReadCreate))
      return SNMP_NOT_WRITABLE;
      
    // Check type.
    time_t TestVal;
    Status = VB.GetDateAndTime (&TestVal);
    
    if (Status != SNMP_NO_ERROR)
      return Status;
  }

  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling TestLeafValue." << endl;
    Status = fpParent->TestLeafValue (*this, VB);
  }

  return Status;
}

SNMP_STATUS DateTimeMibObject::TestDefaults (time_t TestVal)
{
  // No constraints.
  return SNMP_NO_ERROR;
}

SNMP_STATUS DateTimeMibObject::Set (BcmVarBind &VB)
{
  time_t TestVal;
  SNMP_STATUS Status = SNMP_NO_ERROR;


  SnmpLogSearch << "Setting DateTime object " << Name() << " to " << VB << endl;

  if (fpParent)
  {
    SnmpLogSearch << "  Object has parent " << fpParent->Name() << ", calling SetLeafValue." << endl;
    Status = fpParent->SetLeafValue (*this, VB);
  }
  
  // The parent SHOULD verify that we got passed the proper type, but just
  // in case he didn't, check it here.  Also, in case we got no parent.
  if ((Status == SNMP_NO_ERROR) || (Status == SNMP_NO_OPINION))
  {
    SNMP_STATUS GetStatus = VB.GetDateAndTime (&TestVal);
    if (GetStatus != SNMP_NO_ERROR)
    {
      SnmpLogSearch << "  Set to invalid DATE_TIME: " << GetStatus << endl;
      return GetStatus;
    }
  }

  if ((Status != SNMP_NO_ERROR)
  &&  (Status != SNMP_NO_OPINION))
  {
    SnmpLogSearch << "  Set failed, return value " << Status << endl;
    return Status;
  }

  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmMibGroup::AddProxyScalar (BcmMibObject *pScalar)
{
  BcmObjectId OID;
  ObjectId AgentOid;

  if (fpAgent)
    fpAgent->LockMib();
    
  pScalar->GetOid (OID);
  OID.GetOID (&AgentOid);
  
  SnmpLogInit << "BcmMibGroup: Adding Proxy scalar " << pScalar->Name() << endl;

  AgentRegisterScalar (&AgentOid, pScalar->Name());

  if (fpAgent)
    fpAgent->UnlockMib();

  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmMibGroup::AddProxyTable (BcmMibTable *pTable)
{
  BcmObjectId OID;
  ObjectId AgentOid;
  

  SnmpLogInit << "BcmMibGroup: Adding Proxy table " << pTable->Name() << endl;
  
  if (fpAgent)
    fpAgent->LockMib();
    
  pTable->GetOid (OID);
  OID.GetOID (&AgentOid);
  AgentRegisterTabular (&AgentOid, pTable->Name());

  if (fpAgent)
    fpAgent->UnlockMib();

  return SNMP_NO_ERROR;
}


