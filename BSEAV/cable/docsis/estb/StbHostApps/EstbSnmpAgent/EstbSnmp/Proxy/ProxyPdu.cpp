//****************************************************************************
//
// Copyright (c) 2007-2010 Broadcom Corporation
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
//**************************************************************************
//    Filename: ProxyPdu.cpp
//    Author:   Maurice Turcotte / Kevin O'Neal
//    Creation Date: 4 AUG 06
//
//**************************************************************************
//    Description:
//
//      An object representing a Proxy PDU.
//
//      
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include "ProxyPdu.h"
#include "Proxy.h"


#include "BcmSocket.h"
#include "SocketSet.h"
#include "SocketFactory.h"
#include "SnmpLog.h"

#include "BerObjs.h"
#include "MessageLog.h"


BcmVariableBinding::BcmVariableBinding ()
{
}

BcmVariableBinding::BcmVariableBinding (const BcmVariableBinding &VB)
{
  fOID = VB.OID();
  fValue = VB.Value();
}

BcmVariableBinding::BcmVariableBinding (const BcmObjectId &OID, const BcmVarBind &Value)
{
  fOID = OID;
  fValue = Value;
}

BcmVariableBinding::BcmVariableBinding (const ProxyPduVariableBinding *pVarbind)
{
  if (pVarbind)
  {
    // Set OID
    fOID.Set (pVarbind->OID.pObjects, pVarbind->OID.Length);
    
    // Set value
    switch (pVarbind->Type)
    {
      case VBT_NUMBER:
        fValue.SetInt32(pVarbind->Value.Number32);
        break;
        
      case VBT_STRING:
        fValue.SetString(pVarbind->Value.String.pOctets, pVarbind->Value.String.Length);
        break;
        
      case VBT_OBJECT:
        fValue.SetOid(BcmObjectId (pVarbind->Value.OID.Length, pVarbind->Value.OID.pObjects));
        break;
        
      case VBT_EMPTY:
        fValue.SetEmpty();
        break;
        
      case VBT_IPADDRESS:
        fValue.SetIp(pVarbind->Value.pIpAddr);
        break;
        
      case VBT_COUNTER:
        fValue.SetUint32(pVarbind->Value.Counter32, VBT_COUNTER);
        break;
        
      case VBT_GAUGE:
        fValue.SetUint32(pVarbind->Value.Counter32, VBT_GAUGE);
        break;
        
      case VBT_TIMETICKS:
        fValue.SetTimeticks(pVarbind->Value.Counter32);
        break;
        
      case VBT_COUNTER64:
        fValue.SetUint64(BcmCounter64(pVarbind->Value.C64.Hi, pVarbind->Value.C64.Lo));
        break;
        
      case VBT_UINTEGER32:
        fValue.SetUint32(pVarbind->Value.Counter32, VBT_UINTEGER32);
        break;
    }
  }
}

BcmVariableBinding::BcmVariableBinding (const BcmString &Asn1Varbind)
{
  Asn1Decode(Asn1Varbind);
}

BcmVariableBinding::~BcmVariableBinding ()
{
}

const BcmObjectId &BcmVariableBinding::OID () const
{
  return fOID;
}

const BcmVarBind &BcmVariableBinding::Value () const
{
  return fValue;
}

ostream &BcmVariableBinding::Print(ostream &outStream) const
{
  outStream << OID() << " = " << Value();
  return outStream;
}

bool BcmVariableBinding::Asn1Decode (const BcmString &Asn1Varbind)
{
  AsnBerVarBind BerVB((const unsigned char *)Asn1Varbind.data(), (int)Asn1Varbind.size());
  
  if (BerVB.Decode() != SNMP_NO_ERROR)
    return false;
    
  BerVB.GetOid(fOID);
  BerVB.GetValue(fValue);
  
  return true;
}

bool BcmVariableBinding::Asn1Decode (const BcmString &Asn1Varbind, int *pLength)
{
  AsnBerVarBind BerVB((const unsigned char *)Asn1Varbind.data(), (int)Asn1Varbind.size());

  if (BerVB.Decode() != SNMP_NO_ERROR)
    return false;

  BerVB.GetOid(fOID);
  BerVB.GetValue(fValue);

  if (pLength)
    *pLength = BerVB.Length();

  return true;
}

bool BcmVariableBinding::Asn1Encode (BcmString &Asn1Varbind) const
{
  AsnBerVarBind BerVB(fOID, fValue);
  
  if (BerVB.Encode(Asn1Varbind) != SNMP_NO_ERROR)
    return false;
    
  return true;
}


BcmProxyPdu::BcmProxyPdu ()
{
  ClearPdu();
}

BcmProxyPdu::BcmProxyPdu (const ProxyPdu *pPdu)
{
  SetPdu (pPdu);
}

BcmProxyPdu::BcmProxyPdu (const BcmProxyPdu &Pdu)
{
  SetPdu (Pdu);
}

BcmProxyPdu::BcmProxyPdu (ProxyPduType Type, SnmpProtocolVersion Version, SnmpErrorStatus ErrorStatus, int ErrorIndex, const BcmString &CommunityOrSecName)
{
  ClearPdu();
  
  fPduType = Type;
  fProtocolVersion = Version;
  fErrorStatus = ErrorStatus;
  fErrorIndex = ErrorIndex;
  
  if (ProtocolVersion() == kSNMPv3)
    fSecurityName = CommunityOrSecName;
  else
    fCommunity = CommunityOrSecName;
}

void BcmProxyPdu::ClearPdu ()
{
  fPduType = kNoType;
  fProtocolVersion = kNoVersion;
  fErrorStatus = kNoStatus;
  fErrorIndex = -1;
  fSrcIp = kAllZerosIp;
  fDstIp = kAllZerosIp;
  fCommunity = "";
  fSecurityName = "";

  // Iterate over list and delete
  VarbindList::iterator Pos = fVarbinds.begin();
  while (Pos != fVarbinds.end())
  {
    delete (*Pos);
    fVarbinds.erase(Pos);
    Pos = fVarbinds.begin();
  }
}

BcmProxyPdu::~BcmProxyPdu ()
{
  ClearPdu();
}

void BcmProxyPdu::SetPdu (const ProxyPdu *pPdu)
{
  ClearPdu();

  if (pPdu == NULL)
    return;

  fPduType = (ProxyPduType) pPdu->PduType;
  fProtocolVersion = (SnmpProtocolVersion) pPdu->SnmpVersion;
  fErrorStatus = (SnmpErrorStatus) pPdu->ErrorStatus;
  fErrorIndex = pPdu->ErrorIndex;
  fSrcIp.Set(pPdu->pSrcIp[0],pPdu->pSrcIp[1],pPdu->pSrcIp[2],pPdu->pSrcIp[3]);
  fDstIp.Set(pPdu->pDstIp[0],pPdu->pDstIp[1],pPdu->pDstIp[2],pPdu->pDstIp[3]);
  
  if (pPdu->pCommunity)
  {
    if (fProtocolVersion != kSNMPv3)
      fCommunity = pPdu->pCommunity;
    else
      fSecurityName = pPdu->pCommunity;
  }
  
  ProxyPduVariableBinding *pVB = pPdu->pVarbinds;
  while (pVB)
  {
    AddVarbind(pVB);
    pVB = pVB->pNext;
  }
}

void BcmProxyPdu::SetPdu (const BcmProxyPdu &Pdu)
{
  ClearPdu();

  fPduType = Pdu.PduType();
  fProtocolVersion = Pdu.ProtocolVersion();
  fErrorStatus = Pdu.ErrorStatus();
  fErrorIndex = Pdu.ErrorIndex();
  fSrcIp = Pdu.SrcIp();
  fDstIp = Pdu.DstIp();

  if (Pdu.Community())  
    fCommunity = Pdu.Community();
    
  if (Pdu.SecurityName())
    fSecurityName = Pdu.SecurityName();
  
  for (int Loop = 0; Loop < Pdu.NumVarbinds(); Loop++)
    AddVarbind(BcmVariableBinding(*Pdu.Varbind(Loop)));
}

BcmProxyPdu &BcmProxyPdu::operator = (const ProxyPdu *pPdu)
{
  SetPdu (pPdu);
  return *this;
}

BcmProxyPdu &BcmProxyPdu::operator = (const BcmProxyPdu &Pdu)
{
  SetPdu (Pdu);
  return *this;
}

BcmProxyPdu::ProxyPduType BcmProxyPdu::PduType () const
{
  return fPduType;
}

BcmProxyPdu::SnmpProtocolVersion BcmProxyPdu::ProtocolVersion () const
{
  return fProtocolVersion;
}

BcmProxyPdu::SnmpErrorStatus BcmProxyPdu::ErrorStatus () const
{
  return fErrorStatus;
}

int BcmProxyPdu::ErrorIndex () const
{
  return fErrorIndex;
}

const BcmIpAddress &BcmProxyPdu::SrcIp () const
{
  return fSrcIp;
}

const BcmIpAddress &BcmProxyPdu::DstIp () const
{
  return fDstIp;
}

const char *BcmProxyPdu::Community () const
{
  if (ProtocolVersion() == kSNMPv3)
    return NULL;
    
  return fCommunity.c_str();
}

const char *BcmProxyPdu::SecurityName () const
{
  if (ProtocolVersion() != kSNMPv3)
    return NULL;
    
  return fSecurityName.c_str();
}

int BcmProxyPdu::NumVarbinds () const
{
  return fVarbinds.size();
}

const BcmVariableBinding *BcmProxyPdu::Varbind (int Index) const
{
  if (Index >= NumVarbinds())
    return NULL;
    
  int Loop;
    
  VarbindList::const_iterator Pos = fVarbinds.begin();
  
  for (Loop = 0; Loop < Index; Loop++)
    Pos++;
    
  return *Pos;
}

bool BcmProxyPdu::AddVarbind (const BcmVariableBinding &Varbind)
{
  // COPY the varbind and add it to our list
  BcmVariableBinding *pVB = new BcmVariableBinding(Varbind);

  fVarbinds.push_back(pVB);
  return true;
}

bool BcmProxyPdu::AddVarbind (const BcmObjectId &OID, const BcmVarBind &Value)
{
  return AddVarbind (BcmVariableBinding(OID, Value));
}

bool BcmProxyPdu::AddVarbind (const ProxyPduVariableBinding *pVarbind)
{
  return AddVarbind (BcmVariableBinding(pVarbind));
}

bool BcmProxyPdu::Asn1Decode (const BcmString &Asn1Pdu, const BcmIpAddress &SrcIp, const BcmIpAddress &DstIp)
{
  int Index = 0;

 
  // First clear the PDU.  Even if the parse fails calling this method flushes
  // us.
  ClearPdu();
 
  // Go ahead and store the src & dst IP addrs
  fSrcIp = SrcIp;
  fDstIp = DstIp;

  // First we have to decode the stuff out of the header.  This includes
  // SNMP version, PDU type, community, error status/index, etc.

  // The first byte better be 0x30.  If it's not, return false.
  if (Asn1Pdu[0] != (BER_F_CONSTRUCTED | BER_TLV_SEQUENCE))
  {
    gLogMessageRaw << "Asn1Decode failure: byte 0 != 0x30" << endl;
    return false;
  }
  Index++;

  // Next is the length of the whole packet.  We can decode this using the
  // AsnBerLength base class.
  AsnBerLength PacketLength((const unsigned char *)Asn1Pdu.data() + Index);
  if (Asn1Pdu.size() != PacketLength.Length() + PacketLength.TotalLength() + Index)
  {
    gLogMessageRaw << "Asn1Decode failure: bad packet length" << endl;
    gLogMessageRaw << "PacketLength.Length=" << PacketLength.Length() << " TotalLength=" << PacketLength.TotalLength() << " PDU.size=" << Asn1Pdu.size() << endl;
    return false;
  }
  Index += PacketLength.TotalLength();

  // Next up is an integer tuple with the SNMP protocol version.
  AsnBerInt32 Version((const unsigned char *)Asn1Pdu.data() + Index);
  if (Version.GetValue((int &) fProtocolVersion) != SNMP_NO_ERROR)
  {
    gLogMessageRaw << "Asn1Decode failure: failed parsing version" << endl;
    ClearPdu();
    return false;
  }

  // SNMPv3 not supported for now.
  if (fProtocolVersion == kSNMPv3)
  {
    gLogMessageRaw << "Asn1Decode failure: SNMPv3 not supported." << endl;
    ClearPdu();
    return false;
  }

  // Must be SNMPv1 or SNMPv2.
  if ((fProtocolVersion != kSNMPv1) && (fProtocolVersion != kSNMPv2c))
  {
    gLogMessageRaw << "Asn1Decode failure: unknown protocol version." << endl;
    ClearPdu();
    return false;
  }
  Index += Version.TotalLength();

  // Now we are at the community
  AsnBerOctetString Community((const unsigned char *)Asn1Pdu.data() + Index);
  if (Community.GetValue(fCommunity) != SNMP_NO_ERROR)
  {
    gLogMessageRaw << "Asn1Decode failure: failed parsing community" << endl;
    ClearPdu();
    return false;
  }
  Index += Community.TotalLength();

  // Now we are at the PDU type.  The first nibble better be 0xA
  if ((Asn1Pdu[Index] & 0xF0) != (BER_CLASS_CONTEXT_SPECIFIC | BER_F_CONSTRUCTED))
  {
    gLogMessageRaw << "Asn1Decode failure: failed parsing PDU type" << endl;
    ClearPdu();
    return false;
  }

  fPduType = (ProxyPduType)(Asn1Pdu[Index] & 0x0F);
  if ((fPduType != kGet)
  &&  (fPduType != kNext)
  &&  (fPduType != kResponse)
  &&  (fPduType != kSet)
  &&  (fPduType != kTrap)
  &&  (fPduType != kBulk)
  &&  (fPduType != kInform)
  &&  (fPduType != kTrap2)
  &&  (fPduType != kReport))
  {
    gLogMessageRaw << "Asn1Decode failure: unknown PDU type" << endl;
    ClearPdu();
    return false;
  }
  Index++;

  // Next is the length of the PDU proper.
  AsnBerLength PduLength((const unsigned char *)Asn1Pdu.data() + Index);
  if (Asn1Pdu.size() != PduLength.Length() + PduLength.TotalLength() + Index)
  {
    gLogMessageRaw << "Asn1Decode failure: bad PDU length" << endl;
    return false;
  }
  Index += PduLength.TotalLength();

  // Next up is an integer tuple with the SNMP request ID.
  int RequestId;
  AsnBerInt32 ReqId((const unsigned char *)Asn1Pdu.data() + Index);
  if (ReqId.GetValue(RequestId) != SNMP_NO_ERROR)
  {
    gLogMessageRaw << "Asn1Decode failure: failed parsing request ID" << endl;
    ClearPdu();
    return false;
  }
  Index += ReqId.TotalLength();

  // Next up is an integer tuple with the error status.
  AsnBerInt32 ErrorStatus((const unsigned char *)Asn1Pdu.data() + Index);
  if (ErrorStatus.GetValue((int &)fErrorStatus) != SNMP_NO_ERROR)
  {
    gLogMessageRaw << "Asn1Decode failure: failed parsing error status" << endl;
    ClearPdu();
    return false;
  }
  Index += ErrorStatus.TotalLength();

  // Next up is an integer tuple with the error index.
  AsnBerInt32 ErrorIndex((const unsigned char *)Asn1Pdu.data() + Index);
  if (ErrorIndex.GetValue((int &)fErrorIndex) != SNMP_NO_ERROR)
  {
    gLogMessageRaw << "Asn1Decode failure: failed parsing error index" << endl;
    ClearPdu();
    return false;
  }
  Index += ErrorIndex.TotalLength();

  // Now we are to the start of the variable bindings list.  First is a
  // header.  It must start with 0x30
  if (Asn1Pdu[Index] != (BER_F_CONSTRUCTED | BER_TLV_SEQUENCE))
  {
    gLogMessageRaw << "Asn1Decode failure: failed parsing varbind list header" << endl;
    ClearPdu();
    return false;
  }
  Index++;

  // Next is a tuple with the varbind list length.
  AsnBerLength VarbindListLength((const unsigned char *)Asn1Pdu.data() + Index);
  if (Asn1Pdu.size() != VarbindListLength.Length() + VarbindListLength.TotalLength() + Index)
  {
    gLogMessageRaw << "Asn1Decode failure: bad varbind list length" << endl;
    return false;
  }
  Index += VarbindListLength.TotalLength();

  // OK, now we are at a point where we can finally loop through all the
  // varbinds.
  int VarbindIndex = 0;
  BcmVariableBinding Varbind;
  int LengthLeft = Asn1Pdu.size() - Index;
  while (LengthLeft > 0)
  {
    BcmString VarbindString(BcmOctetBuffer ((unsigned char *)Asn1Pdu.data() + Index, LengthLeft, LengthLeft, false));
    int VarbindLength;

    if (Varbind.Asn1Decode(VarbindString, &VarbindLength) == false)
    {
      gLogMessageRaw << "Asn1Decode failure: bad varbind " << VarbindIndex << endl;
      ClearPdu();
      return false;
    }
    AddVarbind(Varbind);
    Index += VarbindLength;
    LengthLeft -= VarbindLength;
    VarbindIndex++;
  }
 
  // OK if we made it here all is well.  Return true.
  return true;
}


bool BcmProxyPdu::Asn1Encode (BcmString &Asn1Pdu) const
{
  // Arbitrary 16 bit request ID, incremented each time.  Let's start 
  // with 'KO'.  Why not?
  static unsigned short RequestId = 0x4B4F;
  
  
  /* Check for some things that this implementation currently doesn't encode */

  // For now we don't support encoding SNMPv3 packets.
  if ((ProtocolVersion() != kSNMPv1) && (ProtocolVersion() != kSNMPv2c))
  {
    gLogMessageRaw << "BcmProxyPdu::Asn1Encode only supports SNMPv1 & SNMPv2c" << endl;
    return false;
  }
  
  // We use standard length encoding for error index so nothing bigger than 127
  // Supporting this would be an easy change but for now not necessary.
  if (ErrorIndex() > 127)
  {
    gLogMessageRaw << "BcmProxyPdu::Asn1Encode only supports error index <= 127" << endl;
    return false;
  }
  
  // Likewise with community length.
  if (fCommunity.length() > 127)
  {
    gLogMessageRaw << "BcmProxyPdu::Asn1Encode only supports community length <= 127" << endl;
    return false;
  }
    
  /* Construct an SNMP packet with all our parameters in it. */
  
  BcmString Msg, VarBinds;
  
  // TEST: intentionally mis-encode packet to test timeout code.
  // const unsigned char pMsgHdr1[] = {0x31, 0x82}; // First octet oughta be 0x30, not 0x31
  const unsigned char pMsgHdr1[] = {0x30, 0x82}; // Up to length
  unsigned char pMsgHdr2[] = {0x02, 0x01, 0x01, 0x04}; // Up to community
  unsigned char pPduHdr1[] = {0xa3, 0x82}; // Up to length
  const unsigned char pPduHdr2[] = {0x02, 0x02}; // Up to request ID
  unsigned char pPduHdr3[] = {0x02, 0x01, 0x00, 0x02, 0x01, 0x00}; // Error status, index
  const unsigned char pVbHdr[] = {0x30, 0x82};  
  
  // Set the correct PDU type
  pPduHdr1[0] = PduType()+0xA0;
  
  // Set our protocol version.  It's initialized to SNMPv2c so we only need to
  // change it for SNMPv1
  if (ProtocolVersion() == kSNMPv1)
    pMsgHdr2[2] = 0;
    
  // Set the error status...
  pPduHdr3[2] = ErrorStatus();
  
  // ...and index.
  pPduHdr3[5] = ErrorIndex();
  
  // Start by concatenating all our varbinds into a single string.  We need
  // to do this now because we need to know the total length of the varbinds
  // in order to do some length calculations below.
  BcmVariableBinding *pVB;
  BcmString VarbindBuffer;
  VarbindList::const_iterator Pos = fVarbinds.begin();
  while (Pos != fVarbinds.end())
  {
    pVB = *Pos;
    if (pVB->Asn1Encode(VarbindBuffer))
      VarBinds.append ((const char *)VarbindBuffer.data(), VarbindBuffer.length());
    else
    {
      gLogMessageRaw << "BcmProxyPdu::Asn1Encode failed to encode " << *pVB << endl;
      return false;
    } 
    
    Pos++;
  }  
  
  // Message header, up to length:
  Msg.append ((char *)pMsgHdr1, sizeof(pMsgHdr1));
  
  // Add length.  Note that we choose to use the extended length encoding
  // here even if it is not necessary.  Just easier that way, and it is
  // allowed.
  Msg += (char)(((VarBinds.length() + fCommunity.length() + 23) >> 8) & 0xFF);
  Msg += (char)((VarBinds.length() + fCommunity.length() + 23) & 0xFF);
  
  // Message header, up to community string:
  Msg.append ((char *)pMsgHdr2, sizeof(pMsgHdr2));
  
  // Community string length (8 bits):
  Msg += (char) fCommunity.length();
  
  // Community string:
  Msg.append (fCommunity.data(), fCommunity.length());
  
  // PDU header, up to length:
  Msg.append ((char *)pPduHdr1, sizeof(pPduHdr1));
  
  // Add length.  Note that we choose to use the extended length encoding
  // here even if it is not necessary.  Just easier that way, and it is
  // allowed.
  Msg += (char)(((VarBinds.length() + 14) >> 8) & 0xFF);
  Msg += (char)((VarBinds.length() + 14) & 0xFF);
  
  // PDU header, up to request ID:
  Msg.append ((char *)pPduHdr2, sizeof(pPduHdr2));
  
  // Request ID (16 bits)
  RequestId++; // Increment it.
  Msg += (char)((RequestId >> 8) & 0xFF);
  Msg += (char)(RequestId & 0xFF);
  
  // Error status, error index
  Msg.append ((char *)pPduHdr3, sizeof(pPduHdr3));
  
  // Now the VB list header
  Msg.append ((char *)pVbHdr, sizeof(pVbHdr));
  
  // Add the length of the VB list (16 bits).  Note that we choose to use the 
  // extended length encoding here even if it is not necessary.  Just easier 
  // that way, and it is allowed.
  Msg += (char)((VarBinds.length() >> 8) & 0xFF);
  Msg += (char)(VarBinds.length() & 0xFF);
  
  // Finally, the VB list itself.
  Msg += VarBinds;
  
  // All that's left to do is assign to our argument.
  Asn1Pdu = Msg;
  return true;
}

ostream &BcmProxyPdu::Print(ostream &outStream) const
{
  const char *pPduType[] = 
             { "N/A", "GET", "NEXT", "RESPONSE", "SET", "TRAP", "BULK", 
               "INFORM", "TRAP2", "REPORT" };
  const char *pProtocolVersion[] = 
             { "N/A", "SNMPv1", "SNMPv2c", "SNMPv2u", "SNMPv3" };
  const char *pStandardErrorStatus[] = 
             { "N/A", "NO ERROR", "TOO BIG", "NO SUCH NAME", "BAD VALUE", "READ ONLY", 
               "GEN ERR", "NO ACCESS", "WRONG TYPE", "WRONG LENGTH", 
               "WRONG ENCODING", "WRONG VALUE", "NO CREATION", 
               "INCONSISTENT VALUE", "RESOURCE UNAVAILABLE", "COMMIT FAILED", 
               "UNDO FAILED", "AUTH ERROR", "NOT WRITABLE", "INCONSISTENT NAME", 
               "LAST ERROR" };
  const char *pAppErrorStatus[] = 
             { "SCALAR OBJECT", "NO OPINION", "DUPLICATE VALUE", 
               "BAD COMMUNITY NAME", "BAD COMMUNITY USE", "NO SUCH INSTANCE" };                                    
  
  
  // All the [??? + 1] indexing below is to handle the fact that for a 'cleared'
  // PDU we use -1 as a value for N/A on type, version, and error status.
  
  outStream << pProtocolVersion[ProtocolVersion() + 1] << " " 
            << pPduType[PduType() + 1] << " PDU, " << fSrcIp << "->" << fDstIp << ":" << endl;
            
  if (ProtocolVersion() == kSNMPv3)
    outStream << "Security Name = '" << fSecurityName << "'" << endl;
  else
    outStream << "Community = '" << fCommunity << "'" << endl;
            
  outStream << "Varbinds:" << endl;
  
  for (int Loop = 0; Loop < NumVarbinds(); Loop++)
    outStream << "  " << *Varbind(Loop);
    
  if (ErrorStatus() <= kLastError)
    outStream << "  Error = " << pStandardErrorStatus[ErrorStatus() + 1] << endl;
  else
    outStream << "  Error = " << pAppErrorStatus[ErrorStatus() - kScalarObject] << endl;
    
  outStream << "  Error Index = " << ErrorIndex() << endl;
  
  // Encode and print
  BcmString Asn1String;
  if (Asn1Encode(Asn1String))
  {
    BcmOctetBuffer Asn1Buf;
    Asn1String.Get(Asn1Buf);
    
    outStream << "ASN.1 encoded buffer:" << Asn1Buf << endl;
  }

  return outStream;
}

SNMP_STATUS SendProxyPdu (BcmProxyPdu &Pdu, BcmIpAddress &SrcIp, const BcmIpAddress &DstIp, int TimeoutMsecs)
{
  uint16 SourcePort = 12345;
  BcmString Buffer;
 
  // Encode our PDU.
  Pdu.Asn1Encode (Buffer);

  // Create a socket:
  BcmSocket *pSocket = BcmSocketFactory::NewSocket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (pSocket == NULL)
  {
    SnmpLogWarn << "SendProxyPdu couldn't create a socket." << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }

  // Bind to the source IP.  Doesn't really matter what port, just pick one.
  if (pSocket->Bind (SrcIp, SourcePort) != 0)
  {
    SnmpLogWarn << "SendProxyPdu bind failure on socket " << pSocket->SocketDescriptor() << " to " << SrcIp << ":" << SourcePort << ".  Trying another port." << endl;
    SourcePort++;

    if (pSocket->Bind (SrcIp, SourcePort) != 0)
    {
      SnmpLogWarn << "Bind failure on socket " << pSocket->SocketDescriptor() << " to " << SrcIp << ":" << SourcePort << endl;
      delete pSocket;
      return SNMP_RESOURCE_UNAVAILABLE;
    }
  }

  // Fire away!
  SnmpLogThd << "Sending SNMP PDU to " << DstIp << endl;
  SnmpLogThd << BcmOctetBuffer ((unsigned char *)Buffer.data(), Buffer.length(), Buffer.length(), false) << endl;
  if (pSocket->Sendto ((char *)Buffer.data(), Buffer.length(), 0, DstIp, PROXY_DEST_PORT) != (int)Buffer.length())
  {
    // If the sendto call fails, good chance it's because we are dealing with
    // a packet which is bigger than the OS / IP stack can handle.  For
    // eCos, this happens to be 8k.  So if that's the case, we are just hosed
    // so bail out.
    SnmpLogWarn << "SendProxyPdu sendto error!  Packet is probably too big." << endl;
    delete pSocket;
    return SNMP_TOO_BIG;
  }

  // Wait for a reply.
  SnmpLogThd << "Waiting for reply..." << endl;

  // We must use a socket set and a select() call here instead of just
  // calling recvfrom, because if the packet is badly formatted the agent
  // will not reply at all.  So we need the timeout provided by select.
  BcmSocketSet SocketSet("SendProxyPdu Socket Set");

  SocketSet.SetRead (pSocket);
  if (SocketSet.Select (TimeoutMsecs) == 0)
  {
    SnmpLogWarn << "SendProxyPdu timeout on Select." << endl;
    delete pSocket;
    return SNMP_NO_SUCH_NAME;
  }
 
  if (SocketSet.IsSetRead (pSocket) == false)
  {
    SnmpLogWarn << "SendProxyPdu socket not set read??" << endl;
    delete pSocket;
    return SNMP_NO_OPINION;
  }

  BcmIpAddress FromAddr;
  uint16 FromPort;
  int RcvdBytes;
  // 8k is max size packet the ecos IP stack can handle
  unsigned char *pRcvPkt = new unsigned char [8192];

  memset (pRcvPkt, 0, sizeof(pRcvPkt));
  RcvdBytes = pSocket->Recvfrom ((char *)pRcvPkt, 8192, 0, FromAddr, FromPort);

  SnmpLogThd<< "Got response, " << RcvdBytes << " bytes from " << FromAddr << ":" << FromPort << endl;
  SnmpLogThd<< BcmOctetBuffer ((unsigned char *)pRcvPkt, RcvdBytes, RcvdBytes, false) << endl;

  // Make a BcmString so we can PDU-ify the response.
  BcmString PduString(BcmOctetBuffer ((unsigned char *)pRcvPkt, RcvdBytes, RcvdBytes, false));

  // Decode the response
  Pdu.Asn1Decode(PduString, FromAddr, SrcIp);

  delete pSocket;
  delete [] pRcvPkt;

  // Return the error status from the response.
  return Pdu.ErrorStatus();
}


SNMP_STATUS BcmProxyGet (BcmIpAddress &SrcIp, const BcmIpAddress &DstIp, const BcmString &Community, const BcmObjectId &OID, BcmVarBind &VB)
{
  SNMP_STATUS Status;

  SnmpLogThd<< "BcmProxyGet from " <<  DstIp << endl;
  BcmProxyPdu Pdu(BcmProxyPdu::kGet, BcmProxyPdu::kSNMPv2c, BcmProxyPdu::kNoError, 0, Community);
  Pdu.AddVarbind (OID, VB);

  Status =  SendProxyPdu (Pdu, SrcIp, DstIp, 5000);
  
  if (Status == SNMP_NO_ERROR)
  {
    const BcmVariableBinding *pVB = Pdu.Varbind(0);

    if (pVB)
      VB = pVB->Value();
  }

  return Status;
}

SNMP_STATUS BcmProxyGetNext (BcmIpAddress &SrcIp, const BcmIpAddress &DstIp, const BcmString &Community, BcmObjectId &OID, BcmVarBind &VB)
{
  SNMP_STATUS Status;

  BcmProxyPdu Pdu(BcmProxyPdu::kNext, BcmProxyPdu::kSNMPv2c, BcmProxyPdu::kNoError, 0, Community);
  Pdu.AddVarbind (OID, VB);

  Status =  SendProxyPdu (Pdu, SrcIp, DstIp, 5000);
  
  if (Status == SNMP_NO_ERROR)
  {
    const BcmVariableBinding *pVB = Pdu.Varbind(0);

    if (pVB) {
      VB = pVB->Value();
      OID = pVB->OID();
    }
  }

  return Status;
}

SNMP_STATUS BcmProxySet (BcmIpAddress &SrcIp, const BcmIpAddress &DstIp, const BcmString &Community, const BcmObjectId &OID, BcmVarBind &VB)
{
  SNMP_STATUS Status;

  BcmProxyPdu Pdu(BcmProxyPdu::kSet, BcmProxyPdu::kSNMPv2c, BcmProxyPdu::kNoError, 0, Community);
  Pdu.AddVarbind (OID, VB);

  return SendProxyPdu (Pdu, SrcIp, DstIp, 5000);
}

SNMP_STATUS LocalProxyGet (const BcmObjectId &OID, BcmVarBind &VB)
{
  SNMP_STATUS Status;
  BcmString Community = "pub";
  BcmIpAddress LocalIp, RemoteIp;
 
  LocalIp = RemoteIp = kSoftwareLoopbackIp;
 
  Status = BcmProxyGet(LocalIp, RemoteIp, Community, OID, VB);
  if (Status != SNMP_NO_ERROR)
    SnmpLogWarn << "Local Proxy Get " << LocalIp << " -> " << RemoteIp << " for " << OID << " failed w/ status " << Status << endl;

  return Status;
}

SNMP_STATUS LocalProxyGetNext (BcmObjectId &OID, BcmVarBind &VB)
{
  SNMP_STATUS Status;
  BcmString Community = "pub";
  BcmIpAddress LocalIp, RemoteIp;
 
  LocalIp = RemoteIp = kSoftwareLoopbackIp;
 
  Status = BcmProxyGetNext(LocalIp, RemoteIp, Community, OID, VB);
  if (Status != SNMP_NO_ERROR)
    SnmpLogWarn << "Local Proxy GetNext " << LocalIp << " -> " << RemoteIp << " for " << OID << " failed w/ status " << Status << endl;

  return Status;
}

SNMP_STATUS LocalProxySet (const BcmObjectId &OID, BcmVarBind &VB)
{
  SNMP_STATUS Status;
  BcmString Community = "pub";
  BcmIpAddress LocalIp, RemoteIp;

  LocalIp = RemoteIp = kSoftwareLoopbackIp;

  Status = BcmProxySet(LocalIp, RemoteIp, Community, OID, VB);
  if (Status != SNMP_NO_ERROR)
    SnmpLogWarn << "Local Proxy Set " << LocalIp << " -> " << RemoteIp << " for " << OID << " failed w/ status " << Status << endl;

  return Status;
}


