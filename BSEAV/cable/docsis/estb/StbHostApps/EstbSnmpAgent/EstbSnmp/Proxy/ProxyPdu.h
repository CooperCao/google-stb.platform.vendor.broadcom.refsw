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
//    Filename: ProxyPdu.h
//    Author:   Maurice Turcotte / Kevin O'Neal
//    Creation Date: 4 AUG 06
//
//**************************************************************************
//    Description:
//
//      An object to represent a Proxy PDU.
//
//      
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef PROXYPDU_H
#define PROXYPDU_H

#include "SnmpType.h"

// PR 9824: a more memory-efficient object identifier representation than
// the ObjectId structure defined in SnmpType.h.  Uses an allocated array
// of ulongs rather than a fixed-length array.  Also eliminates certain
// unused fields
typedef struct _ProxyPduObjectIdentifier
{
  unsigned char Length;    // Max OID size is 128 elements so uchar is sufficient here
  unsigned long *pObjects;
} ProxyPduObjectIdentifier;

typedef struct _ProxyPduOctetString
{
  unsigned short Length;   // How many bytes are used in the buffer.  Max string length for SNMP = 65535 so ushort is sufficient here
  unsigned char  *pOctets; // Buffer
} ProxyPduOctetString;

// PR 9824: a more memory-efficient variable binding representation than
// the VarBind structure defined in SnmpType.h.  Uses ObjectIdentifier
// structures to represent OIDs rather than ObjectId structures.
typedef struct _ProxyPduVariableBinding
{
  ProxyPduObjectIdentifier OID;
  
  unsigned char Type;
  union 
  {
    long                    Number32;  
    unsigned long           Counter32; 
    unsigned char           pIpAddr[4];
    ProxyPduOctetString      String;    
    ProxyPduObjectIdentifier OID;
    Counter64               C64;
  } Value;
  
  // For chaining:
  struct _ProxyPduVariableBinding *pNext;
} ProxyPduVariableBinding;

// PR 9824: a more memory-efficient PDU representation than the SnmpPacket
// structure defined in SnmpType.h.  Uses a list of PduVariableBinding
// structures rather than MibObject structures.
typedef struct _ProxyPdu
{
  int SnmpVersion;         // SNMP_V1_QUERY, etc (from SnmpStatus.h)
  int PduType;             // PDU_GET_REQUEST, etc (from SnmpStatus.h)
  const char *pCommunity;  // for V1 / V2c queries only, NULL for V3
  unsigned char pSrcIp[4]; // source IP (of manager)
  unsigned char pDstIp[4]; // destination IP (of agent)
  int ErrorStatus;         // Standard SNMP error codes
  int ErrorIndex;          // Index of errored varbind
  
  ProxyPduVariableBinding *pVarbinds; // Chain of varbinds in this PDU.
} ProxyPdu;

#if defined (__cplusplus)

#include "BcmString.h"
#include "IpAddress.h"
#include "ObjectIdentifier.h"
#include "VariableBinding.h"

#include <list>


class BcmVariableBinding
{
  public:
    BcmVariableBinding ();
    BcmVariableBinding (const BcmVariableBinding &VB);
    BcmVariableBinding (const BcmObjectId &OID, const BcmVarBind &Value);
    BcmVariableBinding (const ProxyPduVariableBinding *pVarbind);
    BcmVariableBinding (const BcmString &Asn1Varbind);
    
   ~BcmVariableBinding ();
   
    const BcmObjectId &OID () const;
    const BcmVarBind &Value () const;
    
    ostream &Print(ostream &outStream) const;
   
    bool Asn1Decode (const BcmString &Asn1Varbind);
    bool Asn1Decode (const BcmString &Asn1Varbind, int *pLength);
    bool Asn1Encode (BcmString &Asn1Varbind) const;
   
  protected:
    BcmObjectId fOID;
    BcmVarBind  fValue;
};

inline ostream & operator << (ostream &outStream, const BcmVariableBinding &Varbind)
{
    return Varbind.Print(outStream);
}


typedef list <BcmVariableBinding *> VarbindList;

class BcmProxyPdu
{
  public:
    enum ProxyPduType
    {
      kNoType = -1,
      kGet = PDU_GET_REQUEST,
      kNext = PDU_GET_NEXT_REQUEST,
      kResponse = PDU_GET_RESPONSE,
      kSet = PDU_SET_REQUEST,
      kTrap = PDU_TRAP,
      kBulk = PDU_GET_BULK_REQUEST,
      kInform = PDU_INFORM_REQUEST,
      kTrap2 = PDU_TRAP2,
      kReport = PDU_REPORT
    };
    
    enum SnmpProtocolVersion
    {
      kNoVersion = -1,
      kSNMPv1 = SNMP_V1_QUERY,
      kSNMPv2c = SNMP_V2_QUERY,
      kSNMPv3 = SNMP_V3_QUERY
    };
    
    enum SnmpErrorStatus
    {
      kNoStatus = -1,
      kNoError = SNMP_NO_ERROR,
      kTooBig = SNMP_TOO_BIG,
      kNoSuchName = SNMP_NO_SUCH_NAME,
      kBadValue = SNMP_BAD_VALUE,
      kReadOnly = SNMP_READ_ONLY,
      kGenErr = SNMP_GEN_ERR,
      kNoAccess = SNMP_NO_ACCESS,
      kWrongType = SNMP_WRONG_TYPE,
      kWrongLength = SNMP_WRONG_LENGTH,
      kWrongEncoding = SNMP_WRONG_ENCODING,
      kWrongValue = SNMP_WRONG_VALUE,
      kNoCreation = SNMP_NO_CREATION,
      kInconsistentValue = SNMP_INCONSISTENT_VALUE,
      kResourceUnavailable = SNMP_RESOURCE_UNAVAILABLE,
      kCommitFailed = SNMP_COMMIT_FAILED,
      kUndoFailed = SNMP_UNDO_FAILED,
      kAuthError = SNMP_AUTHORIZATION_ERROR,
      kNotWritable = SNMP_NOT_WRITABLE,
      kInconsistentName = SNMP_INCONSISTENT_NAME,
      kLastError = SNMP_LAST_ERROR,
      
      kScalarObject = SNMP_SCALAR_OBJECT,
      kNoOpinion = SNMP_NO_OPINION,
      kDuplicateValue = SNMP_DUPLICATE_VALUE,
      kBadCommunityName = SNMP_BAD_COMMUNITY_NAME,
      kBadCommunityUse = SNMP_BAD_COMMUNITY_USE,
      kNoSuchInstance = SNMP_NO_SUCH_INSTANCE
    };

  public:
    BcmProxyPdu ();
    BcmProxyPdu (const ProxyPdu *pPdu);
    BcmProxyPdu (const BcmProxyPdu &Pdu);
    BcmProxyPdu (ProxyPduType Type, SnmpProtocolVersion Version, SnmpErrorStatus ErrorStatus, int ErrorIndex, const BcmString &CommunityOrSecName);
   ~BcmProxyPdu ();
   
    void ClearPdu ();
    void SetPdu (const ProxyPdu *pPdu);
    void SetPdu (const BcmProxyPdu &Pdu);
    
    BcmProxyPdu &operator = (const ProxyPdu *pPdu);
    BcmProxyPdu &operator = (const BcmProxyPdu &Pdu);
    
    ProxyPduType PduType () const;
    SnmpProtocolVersion ProtocolVersion () const;
    SnmpErrorStatus ErrorStatus () const;
    int ErrorIndex () const;
    
    const char *Community () const;
    const char *SecurityName () const;
    
    const BcmIpAddress &SrcIp () const;
    const BcmIpAddress &DstIp () const;
    
    int NumVarbinds () const;
    const BcmVariableBinding *Varbind (int Index) const;
    
    ostream &Print(ostream &outStream) const;
    
    bool AddVarbind (const BcmVariableBinding &Varbind);
    bool AddVarbind (const BcmObjectId &OID, const BcmVarBind &Value);
    bool AddVarbind (const ProxyPduVariableBinding *pVarbind);
   
    bool Asn1Encode (BcmString &Asn1Pdu) const;
    bool Asn1Decode (const BcmString &Asn1Pdu, const BcmIpAddress &SrcIp = kAllZerosIp, const BcmIpAddress &DstIp = kAllZerosIp);

    
  protected:
    ProxyPduType fPduType;
    SnmpProtocolVersion fProtocolVersion;
    SnmpErrorStatus fErrorStatus;
    int fErrorIndex;
    BcmIpAddress fSrcIp;
    BcmIpAddress fDstIp;
    BcmString fCommunity;    // SNMPv1 / v2c only
    BcmString fSecurityName; // SNMPv3 only
    
    VarbindList fVarbinds;
};

inline ostream & operator << (ostream &outStream, const BcmProxyPdu &Pdu)
{
    return Pdu.Print(outStream);
}

SNMP_STATUS BcmProxyGet (BcmIpAddress &SrcIp, const BcmIpAddress &DstIp, const BcmString &Community, const BcmObjectId &OID, BcmVarBind &VB);
SNMP_STATUS BcmProxyGetNext (BcmIpAddress &SrcIp, const BcmIpAddress &DstIp, const BcmString &Community, BcmObjectId &OID, BcmVarBind &VB);
SNMP_STATUS BcmProxySet (BcmIpAddress &SrcIp, const BcmIpAddress &DstIp, const BcmString &Community, const BcmObjectId &OID, BcmVarBind &VB);

SNMP_STATUS LocalProxyGet (const BcmObjectId &OID, BcmVarBind &VB);
SNMP_STATUS LocalProxyGetNext (BcmObjectId &OID, BcmVarBind &VB);
SNMP_STATUS LocalProxySet (const BcmObjectId &OID, BcmVarBind &VB);


#endif

#endif

