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
//****************************************************************************
//  $Id$
//
//  Filename:       BcmDocsisCfgVarBindEngine.cpp
//  Author:         Kevin O'Neal
//  Creation Date:  June 26, 2003
//
//****************************************************************************
//  Description:
//      This class implements an engine to process SNMP varbinds from a 
//      DOCSIS config file (TLV-11 for CM, TLV-28 for CableHome)
//
//****************************************************************************

#include "DocsisCfgVarBindEngine.h"
#include "SocketFactory.h"
#include "BcmSocket.h"
#include "SocketSet.h"
#include "SnmpLog.h"
#include "BerObjs.h"
#include "OperatingSystemFactory.h"
#include "SnmpThread.h"

/*
Here is a captured SNMP-SET packet for sysContact='contact' and
sysName='name'

cablemodem factory agent socket fired, 76 bytes from 10.24.65.57:1316
  30 82 00 48   02 01 01 04   06 70 75 62   6c 69 63 a3
  82 00 39 02   02 37 cd 02   01 00 02 01   00 30 82 00
  2b 30 82 00   13 06 08 2b   06 01 02 01   01 04 00 04
  07 63 6f 6e   74 61 63 74   30 82 00 10   06 08 2b 06
  01 02 01 01   05 00 04 04   6e 61 6d 65

SNMPv2c SET 10.24.65.57->10.24.5.218 cmty='public' IfIndex=1
  Base OID=1.3.6.1.2.1.1.4.0, Index=0
 value to set is STRING, 7 bytes:
  63 6f 6e 74   61 63 74
  Ascii: 'contact'
SNMPv2c SET 10.24.65.57->10.24.5.218 cmty='public' IfIndex=1
  Base OID=1.3.6.1.2.1.1.5.0, Index=0
 value to set is STRING, 4 bytes:
  6e 61 6d 65
  Ascii: 'name'
  Set OK

Here's how this buffer is constructed:

===================
SNMP Message Header
===================
HEADER
30                        BER_F_CONSTRUCTED (0x20) | BER_TLV_SEQUENCE (0x10)
82                        Extended length (0x80), using 2 octets:
00 48                     LENGTH: 0x48 = 72 = PDU length - (sequence, int, length)

TUPLE: SNMP version
02                        TYPE: Integer (2)
01                        LENGTH: 1
01                        VALUE: SNMPv2c (0=v1, 1=v2c, 3=3)

TUPLE: Community
04                        TYPE: string (4)
06                        LENGTH: 6
70 75 62 6c 69 63         COMMUNITY: 'public'

========
SNMP PDU
========
HEADER
a3                        BER_CLASS_CONTEXT_SPECIFIC (0x80) | BER_F_CONSTRUCTED (0x20) | SNMP_SET (3)
82                        Extended length (0x80), using 2 octets:
00 39                     Length (57) of PDU past this point

TUPLE: Request ID
02                        TYPE: Integer (2)
02                        LENGTH: 2
37 cd                     VALUE: Request ID

TUPLE: Error status (0 since this is a request)
02                        TYPE: Integer (2) 
01                        LENGTH: 1
00                        VALUE: 0 (what is this?)

TUPLE: Error index (0 since this is a request)
02                        TYPE: Integer (2) 
01                        LENGTH: 1
00                        VALUE: 0 (what is this?)

----------------------
VARIABLE BINDINGS LIST
----------------------
HEADER
30                        BER_F_CONSTRUCTED (0x20) | BER_TLV_SEQUENCE (0x10)
82                        Extended length (0x80), using 2 octets:
00 2b                     Length (43) of all VB's added together, includes 30's, 82's, lengths of the VB's


VB 1: set 1.3.6.1.2.1.1.4.0 to 'contact'
----------------------------------------
HEADER
30                        BER_F_CONSTRUCTED (0x20) | BER_TLV_SEQUENCE (0x10)
82                        Extended length (0x80), using 2 octets:
00 13                     VB Length: 19 (exclude 30, 82, this length field)

TUPLE: OID
06                        TYPE: OID
08                        LENGTH: 8
2b 06 01 02 01 01 04 00   OID: 1.3.6.1.2.1.1.4.0

TUPLE: Value
04                        TYPE: string (4)
07                        LENGTH: 7
63 6f 6e 74 61 63 74      VALUE: contact


VB 1: set 1.3.6.1.2.1.1.5.0 to 'name'
-------------------------------------
HEADER
30                        BER_SEQUENCE | BER_F_PRIMITIVE | BER_TLV_SEQUENCE
82                        Extended length (0x80), using 2 octets:
00 10                     VB Length: 16 (exclude 30, 82, this length field)

TUPLE: OID
06                        TYPE: OID  
08                        LENGTH: 8  
2b 06 01 02 01 01 05 00   OID: 1.3.6.1.2.1.1.5.0

TUPLE: Value
04                        TYPE: string(4)
04                        LENGTH: 4
6e 61 6d 65               VALUE: contact

Here are captured varbinds from a config file which implements the same sets:
Note that the particular config file editor does not use the extended length
for these sets, whereas the SNMP manager that produced the captured packet
above chose to use extended length even though it was not necessary.  Either
approach is allowed.  So that's why these varbinds start with 0x30 0x13 and
0x30 0x10 instead of 0x82 0x00 0x13 and 0x82 0x00 0x10.

  30 13 06 08   2b 06 01 02   01 01 04 00   04 07 63 6f
  6e 74 61 63   74

Adding VB, 16bytes:
  30 10 06 08   2b 06 01 02   01 01 05 00   04 04 6e 61
  6d 65
  
Here is a captured response (NO_ERROR) to this set:

  30 40 02 01   01 04 06 70   75 62 6c 69   63 a2 33 02
  02 4b 4f 02   01 00 02 01   00 30 27 30   13 06 08 2b
  06 01 02 01   01 04 00 04   07 63 6f 6e   74 61 63 74
  30 10 06 08   2b 06 01 02   01 01 05 00   04 04 6e 61
  6d 65
*/

BcmDocsisCfgVarBindEngine::BcmDocsisCfgVarBindEngine (const BcmSnmpAgent *pAgent, const char *pVbName, bool AllowDuplicates)
{
  fpAgent = pAgent;
  fVbName = pVbName;
  fAllowDuplicates = AllowDuplicates;
  
  // Arbitraty starting point for source port.  This is incremented each time
  // we processs the TLV 11's.  By convention, ports > 5000 are reserved for
  // users, so let's pick something bigger than 5000.  Why not 5555?
  fSourcePort = 5555;
  
  // Arbitrary 16 bit request ID.  Also incremented each time.  Let's start 
  // with 'KO'.  Why not?
  fRequestId = 0x4B4F;
  
  // We don't really care what the community string is, so use 'public' 
  // since it's a kind of de-facto standard.
  fCommunity = "public";
  
  fLastError = SNMP_NO_ERROR;
  fLastErrorIndex = 0;
  fNumVarBinds = 0;
  fNumDuplicates = 0;
  fNumUnknowns = 0;
  fNumWrongValues = 0;
}

BcmDocsisCfgVarBindEngine::~BcmDocsisCfgVarBindEngine ()
{
}

SNMP_STATUS BcmDocsisCfgVarBindEngine::AddVarBind (const unsigned char *pVbBuf, int Length)
{
  // First off, check our lists and make sure that this is not a duplicate
  // value.
  bool Duplicate = false;
  AsnBerVarBind BerVB((unsigned char *)pVbBuf);
  BcmObjectId OID;
  BcmVarBind VB;
  BcmString Val;
  VbRec *pVB;
  list<VbRec *>::iterator Pos = fVarBinds.begin();
  
  
  // If this is our first VB, then clear our statistics from last time.
  if ((fVarBinds.size() == 0)
  &&  (fUnknowns.size() == 0))
  {
    fLastError = SNMP_NO_ERROR;
    fLastErrorIndex = 0;
    fLastErrorVarBind = "";
    fNumVarBinds = 0;
    fNumDuplicates = 0;
    fNumUnknowns = 0;
    fNumWrongValues = 0;
  }
  
  fNumVarBinds++;
  
  if ((BerVB.GetOid(OID) != SNMP_NO_ERROR)
  ||  (BerVB.GetValue(VB) != SNMP_NO_ERROR))
  {
    SnmpLogRaw << "ERROR: Couldn't parse VarBind!." << endl;
    
    if (Length == -1)
    {
      SnmpLogRaw << "(No length provided, printing 256 octets)" << endl;
      Length = 256;
    }
      
    SnmpLogRaw << BcmOctetBuffer((unsigned char *)pVbBuf, Length, Length, false) << endl;
    
    fLastErrorIndex = fNumVarBinds;
    fLastErrorVarBind = BcmString ((const char *)pVbBuf, BerVB.Length());
    
    return SNMP_WRONG_ENCODING;
  }
  
  // Sanity check on length if one is provided.  We have seen frequent config
  // file encoding errors where the length is wrong.
  if (Length != -1)
  {
    if (Length != (int) BerVB.Length())
    {
      gLogMessageRaw << "WARNING: Length mismatch on the following object!  Config file encoding error!\n" <<
                        "  Config file length = " << Length << ", VarBind length = " << BerVB.Length() << endl;
    } 
  }
  
  VB.AsString(Val);
  
  // Check the varbinds for duplicate.  Unless that is, we are the kinda
  // engine that allows duplicates (ie PacketCable).
  if (fAllowDuplicates == false)
  {
    while ((Pos != fVarBinds.end()) && (Duplicate == false))
    {
      pVB = *Pos;
    
      AsnBerVarBind CheckVB((unsigned char *)(pVB->VB.data()));
      BcmObjectId CheckOID;
      
      CheckVB.GetOid(CheckOID);
      
      if (CheckOID == OID)
        Duplicate = true;
      else
      {
        BcmObjectId DraftOID = OID;
        
        // Also check to see if we have a duplicate between the old and new
        // docsDev locations.
        TranslateDocsisDraftToRFC (DraftOID);
        
        if (CheckOID == DraftOID)
          Duplicate = true;
      }
    
      Pos++;
    }
    
    // Also check the unknowns for duplicate.  This means that even unknown
    // VB's can't be duplicated.
    Pos = fUnknowns.begin();
    while ((Pos != fUnknowns.end()) && (Duplicate == false))
    {
      pVB = *Pos;
      
      AsnBerVarBind CheckVB((unsigned char *)(pVB->VB.data()));
      BcmObjectId CheckOID;
      
      CheckVB.GetOid(CheckOID);
      
      if (CheckOID == OID)
        Duplicate = true;
      else
      {
        BcmObjectId DraftOID = OID;
        
        // Also check to see if we have a duplicate between the old and new
        // docsDev locations.
        TranslateDocsisDraftToRFC (DraftOID);
        
        if (CheckOID == DraftOID)
          Duplicate = true;
      }
    
      Pos++;
    }
  }
  
  if (Duplicate)
  {
    // Special case: allow duplicate sets to BRCM 'private' MIB objects.
    // In particular, we want to allow multiple sets to the direct memory
    // access objects from the BRCM engineering group.
    if (OID.IsDescendant(BcmObjectId("1.3.6.1.4.1.4413.2.99")) == true)
      gLogMessageRaw << "Shhh! BRCM private MIB duplicate, I won't tell if you won't..." << endl;
    else
    {
      fNumDuplicates++;
      gLogMessageRaw << fVbName << "[" << fNumVarBinds << "]: " << OID << " -> " << Val << " DUPLICATE!" << endl;
      
      // Remember the location of the first duplicate and report it as our
      // error index.
      if (fLastErrorIndex == 0)
      {
        fLastErrorIndex = fNumVarBinds;
        fLastErrorVarBind = BcmString ((const char *)pVbBuf, BerVB.Length());
      }
        
      return SNMP_DUPLICATE_VALUE;
    }
  }
  
  // Cool, this is no duplicate.
  VbRec *pTlv11 = new VbRec;
  pTlv11->Index = fNumVarBinds;
  pTlv11->VB.assign ((const char *)pVbBuf, BerVB.Length());
  
  SnmpLogCfg << fVbName << " engine adding VB, " << BerVB.Length() << " bytes:" << BcmOctetBuffer((unsigned char *)pVbBuf, BerVB.Length(), BerVB.Length(), false) << endl;
  SnmpLogCfg << "OID=" << OID << endl;
  
  // Is this object supported by our agent?
  if (fpAgent->IsObjectSupported(OID) == false)
  {
    // If not, add it to the unknown list.
    fUnknowns.push_back(pTlv11);
    fNumUnknowns++;
    gLogMessageRaw << fVbName << "[" << fNumVarBinds << "]: " << OID << " -> " << Val << " UNKNOWN, ignoring." << endl;
  }
  
  else
  {
    // Otherwise, add it to the list to be processed.
    fVarBinds.push_back(pTlv11);
    SnmpLogCfg << fVbName << "[" << fNumVarBinds << "]: " << OID << " -> " << Val << endl;
  }
  
  return SNMP_NO_ERROR;
}

VbRec *BcmDocsisCfgVarBindEngine::FindVarBind (int Index)
{
  VbRec *pVB = NULL;
  int Ix = 1;
  list<VbRec *>::iterator Pos = fVarBinds.begin();
  
  // Find the requested packet.
  while (Pos != fVarBinds.end())
  {
    if (Ix == Index)
    {
      pVB = *Pos;
      break;
    }
    
    Ix++;
    Pos++;
  }
    
  return pVB;
}
   
SNMP_STATUS BcmDocsisCfgVarBindEngine::Process ()
{
  list<VbRec *>::iterator Pos;
  SNMP_STATUS Status;
  VbRec *pErrVB = NULL;
  
  
  // No CfgVB's: nothing to do.
  if (fVarBinds.size() == 0)
    return SNMP_NO_ERROR;
    
  // Yield the processor for just a smidge.
  BcmOperatingSystemFactory::ThreadSleep (100);
    
  gLogMessageRaw << fpAgent->GetName() << " processing " << fVbName << "'s" << endl;
    
  // Duplicates encountered.
  // if (fNumDuplicates)
  // {
  //   gLogMessageRaw << "  " << fNumDuplicates << " duplicate " << fVbName << "'s (first one at index " << fLastErrorIndex << ")" << endl;
 //    fLastError = SNMP_DUPLICATE_VALUE;
 //    Status = SNMP_DUPLICATE_VALUE;
  // }
  // else
  {
    Status = SendPacket (fpAgent->GetIpAddr(), fpAgent->GetUdpPort());
    
    // We are looping here in case we have an unsupported object.  In that case,
    // we'll remove the unknown object and try again.  Note that we really 
    // should never wind up trying more than once, because we screen the objects
    // in AddVarBind, and that screening should be pretty thorough - it handles
    // objects at the V2 level and the core agent level, and even considers
    // the current view.  The only possible exception that I can think of is
    // if we have an indeterminate view situation, which means that an object
    // of the same base OID but a different index might be excluded.  Example,
    // if ifAdminStatus.2 is excluded by the view and we just check for
    // ifAdminStatus.  But in practice I don't think this will happen.
    while (Status == SNMP_NO_ACCESS)
    {
      pErrVB = FindVarBind (fLastErrorIndex);
      
      if (pErrVB)
      {
        fNumUnknowns++;
      
        // Remove the unknown VB from our var binds list, and add it to the
        // unknowns list.
        fVarBinds.remove (pErrVB);
        fUnknowns.push_back (pErrVB);
    
        // Try again.
        Status = SendPacket (fpAgent->GetIpAddr(), fpAgent->GetUdpPort());
      }
    }
    
    // Print any error codes here.
    if (Status != SNMP_NO_ERROR)
    {
      pErrVB = FindVarBind (fLastErrorIndex);
      if (pErrVB)
      {
        AsnBerVarBind BerVB((unsigned char *)(pErrVB->VB.data()));
        BcmObjectId OID;
        BcmVarBind VB;
        BcmString Val;
        char ErrStatus[32];
        
        BerVB.GetOid(OID);
        BerVB.GetValue(VB);
        VB.AsString(Val);
        AppSnmpStatusAsString (Status, ErrStatus, sizeof(ErrStatus));
        
        gLogMessageRaw << "  " << fVbName << " error: " << OID << " -> " << Val << ": " << ErrStatus << endl;
        
        // If we have an error index, adjust it by the number of objects in the
        // unknown list.  This is because the error index we just got back is
        // the index into the current list of objects, whereas what we want is
        // the index into the ORIGINAL list.
        if (fLastErrorIndex)
          fLastErrorIndex = pErrVB->Index;
      }
    }
  }
  
  // Flush both of our lists.
  Pos = fVarBinds.begin();
  while (Pos != fVarBinds.end())
  {
    delete *Pos;
    Pos++;
  }
  fVarBinds.clear();
  
  Pos = fUnknowns.begin();
  while (Pos != fUnknowns.end())
  {
    delete *Pos;
    Pos++;
  }
  fUnknowns.clear();
  
  if (Status == SNMP_NO_ERROR)
    gLogMessageRaw << "  " << fNumVarBinds << " " << fVbName << "'s OK." << endl;
  else
    gLogMessageRaw << "  " << fVbName << " failure: error status=" << (int) Status << ", error index=" << fLastErrorIndex << endl;
  
  return Status;
}

SNMP_STATUS BcmDocsisCfgVarBindEngine::SendPacket (const BcmIpAddress &DestIp, unsigned short DestPort)
{
  static unsigned char pRcvPkt[256];
  BcmIpAddress tmpIp;
  
  // If our varbind list is empty, no point in doing this.
  if (fVarBinds.size() == 0)
    return SNMP_NO_ERROR;
  
  int CurPktSize;
  BcmSocket *pSocket;
  BcmString Msg, VarBinds;
  list<VbRec *>::iterator Pos;
  
  /*\ 
   *  Construct an SNMPv2c packet which has all the VBs we have received in it. 
   *  We will use SNMPv2c so we get the expanded error code set if it fails.
   *  See the big comment at the top of this file for field explanations.
  \*/
  
  const unsigned char pMsgHdr1[] = {0x30, 0x82}; // Up to length
  const unsigned char pMsgHdr2[] = {0x02, 0x01, 0x01, 0x04}; // Up to community
  const unsigned char pPduHdr1[] = {0xa3, 0x82}; // Up to length
  const unsigned char pPduHdr2[] = {0x02, 0x02}; // Up to request ID
  const unsigned char pPduHdr3[] = {0x02, 0x01, 0x00, 0x02, 0x01, 0x00}; // Error status, index
  const unsigned char pVbHdr[] = {0x30, 0x82};  
  
  // Start by concatenating all our varbinds into a single string.  We need
  // to do this now because we need to know the total length of the varbinds
  // in order to do some length calculations below.
  Pos = fVarBinds.begin();
  while (Pos != fVarBinds.end())
  {
    VbRec *pVB = *Pos;
    VarBinds.append ((const char *)pVB->VB.data(), pVB->VB.length());
    
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
  Msg += (char)((fRequestId >> 8) & 0xFF);
  Msg += (char)(fRequestId & 0xFF);
  
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
  
  // Now we are ready to send the packet to the specified agent.  First, make
  // sure our thread can handle a packet this big.
  CurPktSize = BcmSnmpThread::Singleton().MaxPacketSize();
  if (CurPktSize <= (int) Msg.length())
    BcmSnmpThread::Singleton().MaxPacketSize (Msg.length() + 1);
  
  // Create a socket:
  pSocket = BcmSocketFactory::NewSocket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  
  if (pSocket)
  {
    // Bind to the SW loopback address on our source port.
    tmpIp = kSoftwareLoopbackIp;
    pSocket->Bind (tmpIp, fSourcePort);
  
    // Fire away!
    SnmpLogCfg << "Sending set packet to " << DestIp << endl;
    if (pSocket->Sendto ((char *)Msg.data(), Msg.length(), 0, DestIp, DestPort) != (int)Msg.length())
    {
      // If the sendto call fails, good chance it's because we are dealing with
      // a packet which is bigger than the OS / IP stack can handle.  For
      // eCos, this happens to be 8k.  So if that's the case, then try our
      // clever PacketPointer technique...
      SnmpLogRaw << "Sendto error!  Packet is probably too big.  Trying PacketPointer." << endl;
      
      PacketPointer PktPointer;
      
      PktPointer.Magic = PACKETPOINTER_MAGIC_NUMBER;
      PktPointer.Length = Msg.length();
      PktPointer.pData = new unsigned char[Msg.length()];
      memcpy (PktPointer.pData, Msg.data(), Msg.length());
      
      if (pSocket->Sendto ((char *)&PktPointer, sizeof (PacketPointer), 0, DestIp, DestPort) != sizeof (PacketPointer))
      {
        SnmpLogRaw << "PacketPointer doesn't work either, what the...  Giving up." << endl;
        
        delete [] PktPointer.pData;
        delete pSocket;
        
        return SNMP_RESOURCE_UNAVAILABLE;
      }
    }
    
    // Wait for a reply.  Note that we really don't need the entire response
    // packet, all we are interested in is the part up to the error status and
    // error index.  So 256 bytes is plenty.
    SnmpLogCfg << "Waiting for reply..." << endl;
    
    // We must use a socket set and a select() call here instead of just
    // calling recvfrom, because if the packet is badly formatted the agent
    // will not reply at all.  So we need the timeout provided by select.
    BcmSocketSet SocketSet("CfgVB Socket Set");
    
    SocketSet.SetRead (pSocket);
    
    // PR 6244: Increase timeout to 15 seconds in case this is some
    // rediculous gigantic config file with bazillions of VBs in it.
    // PR 6258: Damn if somebody didn't come along with an event BIGGER
    // config file.  Up to 30 seconds.  Geez!
    //
    // Check the return value from Select here, not IsSetRead.  Select returns
    // 0 on timeout, non-zero (the number of sockets that are ready) if
    // data comes in.  So, if we time-out, IsSetRead will still return true!
    // And if we don't time out, we don't really even need to check IsSetRead
    // because we only have one socket in the set.  But check anyway.
    if (SocketSet.Select (30000) != 0)
    {
      if (SocketSet.IsSetRead (pSocket))
      {
        BcmIpAddress FromAddr;
        uint16 FromPort;
        int RcvdBytes;
        
        memset (pRcvPkt, 0, sizeof(pRcvPkt));
        RcvdBytes = pSocket->Recvfrom ((char *)pRcvPkt, sizeof(pRcvPkt), 0, FromAddr, FromPort);
        
        // Use SnmpLogThread here instead of cfg.  That allows us to get the
        // set packet and the receive packet using the same debug command.
        SnmpLogThd << "Got response, " << RcvdBytes << " bytes from " << FromAddr << ":" << FromPort << endl;
        SnmpLogThd << BcmOctetBuffer ((unsigned char *)pRcvPkt, RcvdBytes, RcvdBytes, false) << endl;
        
        // Extract the error status and error index from the response.
        // The variable things here are the length of the community string, and
        // whether variable length encoding is used.  If our lengths are encoded
        // in just a single octet, then here is where the error status code is
        // located:
        int ErrorStatusIx = fCommunity.length() + 15;
        
        // If we have extended length encoding, then octet 1 of the length field
        // will have the MSB set.  The number of octets required to encode the 
        // length will then be in the 7 LSB's of octet 1.  Note that the agent is 
        // allowed to use extended length ecoding even if it is not necessary!  
        // So check for it regardless of overall packet length.  First check the
        // SNMP message header for extended length.
        if (pRcvPkt[1] & BER_EXTEND_LENGTH)
        {
          ErrorStatusIx += pRcvPkt[1] & 0x7f;
          
          // The SNMP PDU header also may or may not use extended length encoding,
          // so check it as well.
          if (pRcvPkt[fCommunity.length() + (pRcvPkt[1] & 0x7f) + 8] & BER_EXTEND_LENGTH)
            ErrorStatusIx += pRcvPkt[fCommunity.length() + (pRcvPkt[1] & 0x7f) + 8] & 0x7f;
        }
        
        fLastError = pRcvPkt[ErrorStatusIx];
        fLastErrorIndex = pRcvPkt[ErrorStatusIx + 3];
        
        // If there is an error, remember the varbind
        if (fLastErrorIndex)
        {
          VbRec *pVB = FindVarBind (fLastErrorIndex);
          if (pVB)
            fLastErrorVarBind = pVB->VB;
        }
        
        if ((fLastError != SNMP_NO_ERROR)
        &&  (fLastError != SNMP_NO_SUCH_NAME)
        &&  (fLastError != SNMP_NO_SUCH_INSTANCE)
        &&  (fLastError != SNMP_NO_ACCESS))
          fNumWrongValues++;
        
        SnmpLogCfg << "Error status = " << (int)fLastError << endl;
        SnmpLogCfg << "Error index = " << (int)fLastErrorIndex << endl;
      }
    }
    else
    {
      gLogMessageRaw << "SNMP agent timed out " << fVbName << " packet.  Bad encoding." << endl;
      fLastError = SNMP_WRONG_ENCODING;
      fLastErrorIndex = 0;
      fNumWrongValues++;
    }
    
    delete pSocket;
    
    // Back to previous packet size:
    if (CurPktSize <= (int) Msg.length())
      BcmSnmpThread::Singleton().MaxPacketSize (CurPktSize);
  }
  else
  {
    fLastError = SNMP_RESOURCE_UNAVAILABLE;
    fLastErrorIndex = 0;
  }
  
  // Increment source port and request ID.
  fSourcePort++;
  fRequestId++;
  
  return fLastError;
}

// This exact same method is also in the CmSnmpAgent object...
SNMP_STATUS BcmDocsisCfgVarBindEngine::TranslateDocsisDraftToRFC (BcmObjectId &OID) const
{
  static BcmObjectId DraftBase ("1.3.6.1.3.83");
  int Loop;
  
  
  if (DraftBase.IsAscendant (OID) == false)
    return SNMP_INCONSISTENT_NAME;
    
  // If we get a request for a DOCSIS object at the old experimental location,
  // go ahead and process it as if it's at the new location.
  BcmObjectId TempOid ("1.3.6.1.2.1.69");
  
  for (Loop = DraftBase.Length(); Loop < OID.Length(); Loop++)
    TempOid += OID[Loop];
    
  OID = TempOid;
  
  return SNMP_NO_ERROR;
}
