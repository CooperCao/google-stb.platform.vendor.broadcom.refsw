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
//    Filename: BerObjs.cpp
//    Author:   Kevin O'Neal
//    Creation Date: 18-April-2000
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    ASN.1 Basic Encoding Rules based MIB objects
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include "BerObjs.h"
#include "SnmpLog.h"

#include "SnmpType.h"



AsnBerTuple::AsnBerTuple (const unsigned char *pBerBuf, int Length)
{
  SNMP_STATUS Status;
  
  fSpecifiedLength = Length;
  
  Status = SetBuffer (pBerBuf, Length);
  
  if (Status != SNMP_NO_ERROR)
    SnmpLogCfg << "WARNING: AsnBerTuple::Decode returned error " << Status << endl;
    
  fMyBuffer = false;
  
//  if (fpBerBuf)
//    SnmpLogCfg << *this << endl;
}

AsnBerTuple::AsnBerTuple (int Tag, int Length)
{
  fSpecifiedLength = -1;
  
  SetBuffer (NULL, 0);
  
  fMyBuffer = false;
  
  fLength = Length;
  fTag = Tag;
}

AsnBerTuple::~AsnBerTuple ()
{
  if (fMyBuffer)
    delete [] fpBerBuf;
}

SNMP_STATUS AsnBerTuple::SetBuffer (const unsigned char *pBerBuf, int Length)
{
  fSpecifiedLength = Length;

  fpBerBuf = pBerBuf;
  fpValueBuf = NULL;
  
  fLength = 0;
  fTag = 0;
  
  return Decode();
}

ostream &AsnBerTuple::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  if (fpBerBuf == NULL)
    outStream << "ASN.1 BER Tuple: Tag=" << Tag() << ", Length=" << Length() << endl;
  else
    outStream << "ASN.1 BER Tuple: Tag=" << Tag() << ", Length=" << Length() << ", Buffer=" << BcmOctetBuffer ((unsigned char *)fpBerBuf, TotalLength(), TotalLength(), false);
     
  outStream << "AsnBerTupler::Print returning." << endl;
#endif
  
  return outStream;
}

/*
AsnBerTuple &AsnBerTuple::operator = (const unsigned char *pBerBuf)
{
  SnmpLogRaw << "AsnBerTuple::operator = (uchar *)" << endl;
  SetBuffer (pBerBuf);

  Print ();
  
  return *this;
}

AsnBerTuple &AsnBerTuple::operator = (AsnBerTuple &Tuple)
{
  SnmpLogRaw << "Entering AsnBerTuple::operator = (AsnBerTuple &)" << endl;
  
  *this = Tuple.BerBuf();
  
  SnmpLogRaw << "Exiting AsnBerTuple::operator = (AsnBerTuple &)" << endl;
  
  return *this;
}
*/

unsigned char AsnBerTuple::Tag () const
{
  return fTag;
}

unsigned int AsnBerTuple::Length () const
{
  return fLength;
}

unsigned int AsnBerTuple::TotalLength () const
{
  if ((fpValueBuf == NULL) || (fpBerBuf == NULL))
    return 0;
    
  return (fLength + fpValueBuf - fpBerBuf);
}


SNMP_STATUS AsnBerTuple::Decode ()
{
  int Octets, Loop;

  SnmpLogCfg << "AsnBerTuple::Decode - enter" << endl;
  
  if (fpBerBuf == NULL)
  {
    SnmpLogCfg << "  fpBerBuf = NULL" << endl;
    
    fLength = 0;
    fTag = 0;
    fpValueBuf = NULL;
    
    return SNMP_NO_ERROR;
  }

  fTag = fpBerBuf[0];
  SnmpLogCfg << "  fTag = " << fTag << endl;
  
  // If the MSB is not set, then the length is simply the
  // contents of the one and only octet used in the length field.
  if ((fpBerBuf[1] & BER_EXTEND_LENGTH) == 0)
  {
    SnmpLogCfg << "  No extended length." << endl;
    
    fLength = fpBerBuf[1];
    fpValueBuf = fpBerBuf + 2;
    
    SnmpLogCfg << "  fLength = " << fLength << endl;
    SnmpLogCfg << "  AsnBerTuple::Decode - exit" << endl;
    
    return SNMP_NO_ERROR;
  }
  
  // Otherwise, the 1st octet tells us how many octets to concatenate
  // together to arrive at the length.
  SnmpLogCfg << "  Extended length." << endl;
  
  fLength = 0;
  Octets = fpBerBuf[1] & 0x7F;
  
  SnmpLogCfg << "  Length field is " << Octets << " bytes long." << endl;
    
  for (Loop = 0; Loop < Octets; Loop++)
  {
    fLength <<= 8;
    fLength |= fpBerBuf[Loop + 2];
  }
  
  fpValueBuf = fpBerBuf + Octets + 2;
  
  SnmpLogCfg << "  AsnBerTuple::Decode - exit" << endl;
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS AsnBerTuple::Encode (BcmString &Buffer) const
{
  // Base class will simply load the type and length fields.  Note that
  // we rely on these having already been set before this is called!  Check
  // for 0 which is the only likely invalid value.
  if (fTag == 0)
    return SNMP_WRONG_TYPE;
    
  // Octet 0: type
  Buffer = Tag();
  
  // Length: be lazy and always use extended length encoding, employing
  // 4 bytes for the length.  So first tack on an 0x84, which is 
  // (BER_EXTEND_LENGTH | 4)
  Buffer += (unsigned char) 0x84;
  
  // Now slap on the length
  Buffer += (unsigned char) ((Length() >> 24) & 0xFF);
  Buffer += (unsigned char) ((Length() >> 16) & 0xFF);
  Buffer += (unsigned char) ((Length() >> 8)  & 0xFF);
  Buffer += (unsigned char) ((Length() >> 0)  & 0xFF);
/*
  // If we have no buffer, go ahead and allocate one.  
  if (fpBerBuf == NULL)
  {
    // note that the +5 here is to allow space for a 32 bit length
    // +1 for the extended length thingy.
    fpBerBuf = new unsigned char (fLength + 5);
    fpValueBuf = fpBerBuf + 5;
    
    memset (fpBerBuf, 0, fLength + 5);
    fMyBuffer = true;
  }
*/  
  return SNMP_NO_ERROR;
}

const unsigned char *AsnBerTuple::ValueBuf () const
{
  return fpValueBuf;
}

const unsigned char *AsnBerTuple::BerBuf () const
{
  return fpBerBuf;
}

/*\
 *  AsnBerLength
\*/

AsnBerLength::AsnBerLength (const unsigned char *pBerBuf, int Length)
  : AsnBerTuple (pBerBuf, Length)
{
  Decode();
}

AsnBerLength::AsnBerLength (int Length)
{
  fTag = 0;
  fLength = Length;
}

AsnBerLength::~AsnBerLength ()
{
}

SNMP_STATUS AsnBerLength::Decode ()
{
  int Octets, Loop;

  SnmpLogCfg << "AsnBerLength::Decode - enter" << endl;

  if (fpBerBuf == NULL)
  {
    SnmpLogCfg << "  fpBerBuf = NULL" << endl;

    fLength = 0;
    fTag = 0;
    fpValueBuf = NULL;

    return SNMP_NO_ERROR;
  }

  // For a length 'tuple' we'll always use 0 for the tag.
  fTag = 0;
  SnmpLogCfg << "  fTag = " << fTag << endl;

  // If the MSB is not set, then the length is simply the
  // contents of the one and only octet used in the length field.
  if ((fpBerBuf[0] & BER_EXTEND_LENGTH) == 0)
  {
    SnmpLogCfg << "  No extended length." << endl;

    fLength = fpBerBuf[0];
    fpValueBuf = fpBerBuf + 1;

    SnmpLogCfg << "  fLength = " << fLength << endl;
    SnmpLogCfg << "  AsnBerLength::Decode - exit" << endl;

    return SNMP_NO_ERROR;
  }

  // Otherwise, the 1st octet tells us how many octets to concatenate
  // together to arrive at the length.
  SnmpLogCfg << "  Extended length." << endl;

  fLength = 0;
  Octets = fpBerBuf[0] & 0x7F;

  SnmpLogCfg << "  Length field is " << Octets << " bytes long." << endl;

  for (Loop = 0; Loop < Octets; Loop++)
  {
    fLength <<= 8;
    fLength |= fpBerBuf[Loop + 1];
  }

  fpValueBuf = fpBerBuf + Octets + 1;

  SnmpLogCfg << "  AsnBerLength::Decode - exit" << endl;

  return SNMP_NO_ERROR;
}

SNMP_STATUS AsnBerLength::Encode (BcmString &Buffer) const
{
  // Length: be lazy and always use extended length encoding, employing
  // 4 bytes for the length.  So first tack on an 0x84, which is
  // (BER_EXTEND_LENGTH | 4)
  Buffer = (unsigned char) 0x84;

  // Now slap on the length
  Buffer += (unsigned char) ((Length() >> 24) & 0xFF);
  Buffer += (unsigned char) ((Length() >> 16) & 0xFF);
  Buffer += (unsigned char) ((Length() >> 8)  & 0xFF);
  Buffer += (unsigned char) ((Length() >> 0)  & 0xFF);

  return SNMP_NO_ERROR;
}

unsigned int AsnBerLength::TotalLength () const
{
  return fpValueBuf - fpBerBuf;
}

ostream &AsnBerLength::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  AsnBerTuple::Print(outStream);
#endif
  return outStream;
}


/*\
 *  AsnBerInt32
\*/

AsnBerInt32::AsnBerInt32 (const unsigned char *pBerBuf, int Length)
  : AsnBerTuple (pBerBuf, Length)
{
	Decode();
}

AsnBerInt32::AsnBerInt32 (int Value)
  : AsnBerTuple ()
{
  fTag = VBT_NUMBER;
  fValue = Value;
  fLength = 4;
}

AsnBerInt32::~AsnBerInt32 ()
{
}

ostream &AsnBerInt32::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  outStream << "AsnBerInt32 Value = " << fValue << endl;
  AsnBerTuple::Print(outStream);
#endif
  return outStream;
}

SNMP_STATUS AsnBerInt32::GetValue (int &Val) const
{ 
  //if (Tag() != (BER_CLASS_UNIVERSAL | BER_F_PRIMITIVE | BER_TLV_INTEGER))
  if (fTag != VBT_NUMBER)
  {
    SnmpLogCfg << "AsnBerInt32::GetValue failed, Tag != INTEGER " << Tag() << endl;
    return SNMP_WRONG_TYPE;
  }
  else
    SnmpLogCfg << "AsnBerInt32::GetValue fValue=" << fValue << endl;
    
  Val = fValue;
  SnmpLogCfg << "AsnBerInt32::GetValue setting Val=fValue, Val=" << Val << endl;
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS AsnBerInt32::Decode ()
{
  SNMP_STATUS Status;
  const unsigned char *pOctet;


  SnmpLogCfg << "AsnBerInt32::Decode" << endl;

  Status = AsnBerTuple::Decode();
  if (Status != SNMP_NO_ERROR)
  {
    SnmpLogCfg << "AsnBerTuple::Decode failed " << Status << endl;
    return Status;
  }
  
  // Verify this is an integer-like tuple:
  if (fTag != VBT_NUMBER)
  {
    SnmpLogCfg << "  fTag is not integer: " << fTag << " WRONG_TYPE" << endl;
    return SNMP_WRONG_TYPE;
  }
  else
    SnmpLogCfg << "  fTag is OK" << endl;
  
  // Length shouldn't be greater than 4 (32 bits)  
  if (fLength > 4)
  {
    SnmpLogCfg << "  fLength > 4 (" << fLength << ") WRONG_LENGTH" << endl;
    return SNMP_WRONG_LENGTH;
  }
  else
    SnmpLogCfg << "  fLength is OK" << endl;
  
  pOctet = fpValueBuf;
  
  // Initialize value
  fValue = 0;
  
  // Check for a negative number.  No matter how many octets are used to
  // encode the value, if the MSB of the first octet is set then this object
  // is negative.  In that case we should init fValue to -1 (0xFFFFFFFF).  That
  // was as we shift and OR in the octets below, we keep the leftmost bits of
  // fValue as all 1's, thus preserving the negativity.  If we use an init val 
  // of 0 here then the value will look positive unless a full 32 bits is used 
  // to encode the value.
  if ((*pOctet) & 0x80)
  {
    SnmpLogCfg << "Negative value encoded w/ " << fLength << " octets; init to -1." << endl;
    fValue = -1;
  }
  
  SnmpLogCfg << "  pOctet=" << (void *)pOctet << ", fpValueBuf=" << (void *)fpValueBuf << "fLength=" << fLength << endl;
  
  while (pOctet < (fpValueBuf + fLength))
  {
    fValue <<= 8;
    fValue |= *pOctet;
    pOctet++;
  }
  
  SnmpLogCfg << "  Value is " << fValue << endl;
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS AsnBerInt32::Encode (BcmString &Buffer) const
{
  // Call base class to get type and length encoded.
  SNMP_STATUS Status = AsnBerTuple::Encode(Buffer);
  
  if (Status != SNMP_NO_ERROR)
    return Status;
    
  // Now tack on the actual value.
  Buffer += ((fValue >> 24) & 0xFF);
  Buffer += ((fValue >> 16) & 0xFF);
  Buffer += ((fValue >> 8)  & 0xFF);
  Buffer += ((fValue >> 0)  & 0xFF);
  
  // And we are done!
  return SNMP_NO_ERROR;
}

/*\
 *  AsnBerUint32
\*/

AsnBerUint32::AsnBerUint32 (const unsigned char *pBerBuf, int Length)
  : AsnBerTuple (pBerBuf, Length)
{
	Decode();
}

AsnBerUint32::AsnBerUint32 (unsigned int Value, int Type)
  : AsnBerTuple ()
{
  fTag = Type;
  fValue = Value;
  fLength = 4;
}

AsnBerUint32::~AsnBerUint32 ()
{
}

ostream &AsnBerUint32::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  outStream << "AsnBerUint32 Value = " << fValue << endl;
  AsnBerTuple::Print(outStream);
#endif
  return outStream;
}

SNMP_STATUS AsnBerUint32::GetValue (unsigned int &Val) const
{ 
  //if (Tag() != (BER_CLASS_APPLICATION_WIDE | BER_F_PRIMITIVE | BER_TLV_INTEGER))
  if ((fTag != VBT_COUNTER)
  &&  (fTag != VBT_UINTEGER32)
  &&  (fTag != VBT_GAUGE)
  &&  (fTag != VBT_TIMETICKS))
  {
    SnmpLogCfg << "AsnBerUint32::GetValue failed, Tag != UINTEGER " << fTag << endl;
    return SNMP_WRONG_TYPE;
  }
  else
    SnmpLogCfg << "AsnBerUint32::GetValue fValue=" << fValue << endl;
    
  Val = fValue;
  SnmpLogCfg << "AsnBerUint32::GetValue setting Val=fValue, Val=" << Val << endl;
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS AsnBerUint32::Decode ()
{
  SNMP_STATUS Status;
  const unsigned char *pOctet;


  SnmpLogCfg << "AsnBerUint32::Decode" << endl;

  Status = AsnBerTuple::Decode();
  if (Status != SNMP_NO_ERROR)
  {
    SnmpLogCfg << "AsnBerTuple::Decode failed " << Status << endl;
    return Status;
  }
  
  // Verify this is an unsigned-integer-like tuple:
  if ((fTag != VBT_COUNTER)
  &&  (fTag != VBT_UINTEGER32)
  &&  (fTag != VBT_GAUGE)
  &&  (fTag != VBT_TIMETICKS))
  {
    SnmpLogCfg << "  fTag is not Uint32-like: " << fTag << " WRONG_TYPE" << endl;
    return SNMP_WRONG_TYPE;
  }
  else
    SnmpLogCfg << "  fTag is OK" << endl;
    
  // Length shouldn't be greater than 4 (32 bits)  
  if (fLength > 4)
  {
    SnmpLogCfg << "  fLength > 4 (" << fLength << ") WRONG_LENGTH" << endl;
    return SNMP_WRONG_LENGTH;
  }
  else
    SnmpLogCfg << "  fLength is OK" << endl;
  
  pOctet = fpValueBuf;
  
  // Initialize value
  fValue = 0;
  
  SnmpLogCfg << "  pOctet=" << (void *)pOctet << ", fpValueBuf=" << (void *)fpValueBuf << "fLength=" << fLength << endl;
  
  while (pOctet < (fpValueBuf + fLength))
  {
    fValue <<= 8;
    fValue |= *pOctet;
    pOctet++;
  }
  
  SnmpLogCfg << "  Value is " << fValue << endl;
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS AsnBerUint32::Encode (BcmString &Buffer) const
{
  // Call base class to get type and length encoded.
  SNMP_STATUS Status = AsnBerTuple::Encode(Buffer);
  
  if (Status != SNMP_NO_ERROR)
    return Status;
    
  // Now tack on the actual value.
  Buffer += ((fValue >> 24) & 0xFF);
  Buffer += ((fValue >> 16) & 0xFF);
  Buffer += ((fValue >> 8)  & 0xFF);
  Buffer += ((fValue >> 0)  & 0xFF);
  
  // And we are done!
  return SNMP_NO_ERROR;
}


/*\
 *  AsnBerUint64
\*/

AsnBerUint64::AsnBerUint64 (const unsigned char *pBerBuf, int Length)
  : AsnBerTuple (pBerBuf, Length)
{
	Decode();
}

AsnBerUint64::AsnBerUint64 (const BcmCounter64 &Value)
  : AsnBerTuple ()
{
  fTag = VBT_COUNTER64;
  fValue = Value;
  fLength = 8;
}

AsnBerUint64::~AsnBerUint64 ()
{
}

ostream &AsnBerUint64::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  outStream << "AsnBerUint64 Value = " << fValue << endl;
  AsnBerTuple::Print(outStream);
#endif
  return outStream;
}

SNMP_STATUS AsnBerUint64::GetValue (BcmCounter64 &Val) const
{ 
  //if (Tag() != (BER_CLASS_APPLICATION_WIDE | BER_F_PRIMITIVE | BER_TLV_INTEGER))
  if (fTag != VBT_COUNTER64)
  {
    SnmpLogCfg << "AsnBerUint64::GetValue failed, Tag != VBT_UINTEGER64 " << fTag << endl;
    return SNMP_WRONG_TYPE;
  }
  else
    SnmpLogCfg << "AsnBerUint64::GetValue fValue=" << fValue << endl;
    
  Val = fValue;
  SnmpLogCfg << "AsnBerUint64::GetValue setting Val=fValue, Val=" << Val << endl;
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS AsnBerUint64::Decode ()
{
  SNMP_STATUS Status;
  const unsigned char *pOctet;


  SnmpLogCfg << "AsnBerUint64::Decode" << endl;

  Status = AsnBerTuple::Decode();
  if (Status != SNMP_NO_ERROR)
  {
    SnmpLogCfg << "AsnBerTuple::Decode failed " << Status << endl;
    return Status;
  }
  
  // Verify this is an unsigned-integer-like tuple:
  if (fTag != VBT_COUNTER64)
  {
    SnmpLogCfg << "  fTag is not Uint64: " << fTag << " WRONG_TYPE" << endl;
    return SNMP_WRONG_TYPE;
  }
  else
    SnmpLogCfg << "  fTag is OK" << endl;
    
  // Length shouldn't be greater than 8 (64 bits)  
  if (fLength > 8)
  {
    SnmpLogCfg << "  fLength > 8 (" << fLength << ") WRONG_LENGTH" << endl;
    return SNMP_WRONG_LENGTH;
  }
  else
    SnmpLogCfg << "  fLength is OK" << endl;
  
  pOctet = fpValueBuf + fLength - 1;
  
  // Initialize value
  unsigned long Value[2] = {0, 0};
  int Ix = 0;
  
  SnmpLogCfg << "  pOctet=" << (void *)pOctet << ", fpValueBuf=" << (void *)fpValueBuf << "fLength=" << fLength << endl;
  
  while (pOctet >= fpValueBuf)
  {
    Value[Ix / 4] |= (*pOctet << (Ix * 8));
    Ix++;
    pOctet--;
  }
  
  fValue.Set(Value[0], Value[1]);
  
  SnmpLogCfg << "  Value is " << fValue << endl;
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS AsnBerUint64::Encode (BcmString &Buffer) const
{
  // Call base class to get type and length encoded.
  SNMP_STATUS Status = AsnBerTuple::Encode(Buffer);
  
  if (Status != SNMP_NO_ERROR)
    return Status;
    
  // Now tack on the actual value.
  Buffer += ((fValue.High32() >> 24) & 0xFF);
  Buffer += ((fValue.High32() >> 16) & 0xFF);
  Buffer += ((fValue.High32() >> 8)  & 0xFF);
  Buffer += ((fValue.High32() >> 0)  & 0xFF);
  
  Buffer += ((fValue.Low32() >> 24) & 0xFF);
  Buffer += ((fValue.Low32() >> 16) & 0xFF);
  Buffer += ((fValue.Low32() >> 8)  & 0xFF);
  Buffer += ((fValue.Low32() >> 0)  & 0xFF);
  
  // And we are done!
  return SNMP_NO_ERROR;
}


/*\
 *  AsnBerBitString
\*/

AsnBerBitString::AsnBerBitString (const unsigned char *pBerBuf, int Length)
  : AsnBerTuple (pBerBuf, Length)
{
	Decode();
}

AsnBerBitString::AsnBerBitString (const BcmString &Value, int UnusedBits)
  : AsnBerTuple ()
{
  // Note that a BITSTRING is _NOT_ the same as a BITS, which is a 
  // textual convention on OCTET STRING
  fTag = ASN_1_BITSTRING;
  fLength = Value.length();
  fValue = Value;
  fUnusedBits = UnusedBits;
}

AsnBerBitString::~AsnBerBitString ()
{
}

ostream &AsnBerBitString::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  BcmOctetBuffer Buf((unsigned char *)fValue.data(), fValue.length(), fValue.length(), false);

  outStream << "AsnBerBitString value " << Buf <<  endl;
  AsnBerTuple::Print (outStream);
#endif  
  return outStream;
}

SNMP_STATUS AsnBerBitString::GetValue (BcmString &Val, int &UnusedBits) const
{ 
  //if (Tag() != (BER_CLASS_UNIVERSAL | BER_F_PRIMITIVE | BER_TLV_BIT_STRING))
  if (fTag != ASN_1_BITSTRING)
  {
    SnmpLogCfg << "AsnBerBitString::GetValue failed, Tag != BIT_STRING " << Tag() << endl;
    return SNMP_WRONG_TYPE;
  }
    
  Val = fValue;
  UnusedBits = fUnusedBits;
  
  return SNMP_NO_ERROR;
}

// Bitstrings are kind of weird, and I think not used very often.  I
// couldn't find any in any of the DOCSIS required MIBs.  I will return
// a bitstring value as a sequence of octets (String) with an indication
// of how many unused bits there are in the string.  The unused bits will
// be in the least significant positions of the last octet in the string.
// Also, the epilogue agent doesn't seem to really support them either, 
// since a varbind structure doesn't have anyplace to really indicate this
// unused bits value.  But for completeness, if nothing else, here's how to
// handle them...
SNMP_STATUS AsnBerBitString::Decode ()
{
  SNMP_STATUS Status;
  const unsigned char *pOctet;


  SnmpLogCfg << "AsnBerBitString::Decode" << endl;

  Status = AsnBerTuple::Decode();
  if (Status != SNMP_NO_ERROR)
  {
    SnmpLogCfg << "AsnBerTuple::Decode failed " << Status << endl;
    return Status;
  }
  
  // Verify this is a bitstring tuple:
  // Note that a BITSTRING is _NOT_ the same as a BITS
  if (fTag != ASN_1_BITSTRING)
    return SNMP_WRONG_TYPE;
  
  pOctet = fpValueBuf;
    
  // Initialize to empty string
  fValue = "";
  
  // First thing in a bitstring's value section is the number of unused bits.
  fUnusedBits = *pOctet;
  pOctet++;
    
  // Then come the bits themselves, just like a plain old octet string.
  while (pOctet < (fpValueBuf + fLength))
  {
    fValue += *pOctet;
    pOctet++;
  }
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS AsnBerBitString::Encode (BcmString &Buffer) const
{
  // Call base class to get type and length encoded.
  SNMP_STATUS Status = AsnBerTuple::Encode(Buffer);
  
  if (Status != SNMP_NO_ERROR)
    return Status;
    
  // Next is a single byte indicating the unused bits
  Buffer += (unsigned char) fUnusedBits;
  
  // Then the value.
  Buffer += fValue;
  
  // And we are done!
  return SNMP_NO_ERROR;
}

/*\
 *  AsnBerOctetString
\*/

AsnBerOctetString::AsnBerOctetString (const unsigned char *pBerBuf, int Length)
  : AsnBerTuple (pBerBuf, Length)
{
	Decode();
}

AsnBerOctetString::AsnBerOctetString (const BcmString &Value)
  : AsnBerTuple ()
{
    fTag = VBT_STRING;
    fLength = Value.length();
    fValue = Value;
}

AsnBerOctetString::~AsnBerOctetString ()
{
}

ostream &AsnBerOctetString::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  BcmOctetBuffer Buf((unsigned char *)fValue.data(), fValue.length(), fValue.length(), false);

  outStream << "AsnBerOctetString Value = " << Buf << endl;
  AsnBerTuple::Print (outStream);
#endif  
  return outStream;
}

SNMP_STATUS AsnBerOctetString::GetValue (BcmString &Val) const
{ 
  //if (Tag() != (BER_CLASS_UNIVERSAL | BER_F_PRIMITIVE | BER_TLV_OCTET_STRING))
  if ((fTag != VBT_STRING)
  &&  (fTag != VBT_OPAQUE)
  &&  (fTag != VBT_IPADDRESS))
  {
    SnmpLogCfg << "AsnBerOctetString::GetValue failed, Tag != OCTET_STRING " << Tag() << endl;
    return SNMP_WRONG_TYPE;
  }
    
  Val = fValue;
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS AsnBerOctetString::Decode ()
{
  SNMP_STATUS Status;
  const unsigned char *pOctet;


  SnmpLogCfg << "AsnBerOctetString::Decode" << endl;

  Status = AsnBerTuple::Decode();
  if (Status != SNMP_NO_ERROR)
  {
    SnmpLogCfg << "AsnBerTuple::Decode failed " << Status;
    return Status;
  }
  
  // Verify this is a string-like tuple:
  if ((fTag != VBT_STRING)
  &&  (fTag != VBT_OPAQUE)
  &&  (fTag != VBT_IPADDRESS))
    return SNMP_WRONG_TYPE;
  
  pOctet = fpValueBuf;
    
  // Initialize to empty string
  fValue = "";
    
  while (pOctet < (fpValueBuf + fLength))
  {
    fValue += *pOctet;
    pOctet++;
  }
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS AsnBerOctetString::Encode (BcmString &Buffer) const
{
  // Call base class to get type and length encoded.
  SNMP_STATUS Status = AsnBerTuple::Encode(Buffer);
  
  if (Status != SNMP_NO_ERROR)
    return Status;
    
  // Just append the value.
  Buffer += fValue;
  
  // And we are done!
  return SNMP_NO_ERROR;
}

/*\
 *  AsnBerIpAddress
\*/

AsnBerIpAddress::AsnBerIpAddress (const unsigned char *pBerBuf, int Length)
  : AsnBerOctetString (pBerBuf, Length)
{
  fTag = VBT_IPADDRESS;
}

AsnBerIpAddress::AsnBerIpAddress (const BcmIpAddress &Value)
  : AsnBerOctetString ()
{
  fTag = VBT_IPADDRESS;
  fLength = 4;
  fIpValue = Value;
  fValue = Value;
}

AsnBerIpAddress::~AsnBerIpAddress ()
{
}

ostream &AsnBerIpAddress::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  BcmOctetBuffer Buf((unsigned char *)fValue.data(), fValue.length(), fValue.length(), false);

  outStream << "AsnBerIpAddress Value = " << Buf << endl;
  AsnBerTuple::Print (outStream);
#endif  
  return outStream;
}

SNMP_STATUS AsnBerIpAddress::Decode ()
{
  SNMP_STATUS Status = AsnBerOctetString::Decode();
  
  if (Status != SNMP_NO_ERROR)
    return Status;
  
  // Base class will load fValue with a 4 byte octet string.  Convert
  // this to IP address type.
  fIpValue.Set((unsigned char *)fValue.data());  
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS AsnBerIpAddress::GetValue (BcmIpAddress &Val) const
{
  if (fTag != VBT_IPADDRESS)
  {
    SnmpLogCfg << "AsnBerIpAddress::GetValue failed, Tag != IPADDRESS " << Tag() << endl;
    return SNMP_WRONG_TYPE;
  }
  
  Val = fIpValue;
  
  return SNMP_NO_ERROR;
}

/*\
 *  AsnBerOid
\*/

AsnBerOid::AsnBerOid (const unsigned char *pBerBuf, int Length)
  : AsnBerTuple (pBerBuf, Length)
{
	Decode();
}

AsnBerOid::AsnBerOid (const BcmObjectId &Value)
  : AsnBerTuple ()
{
  fTag = VBT_OBJECT;
  fValue = Value;
  
  // Pre-calculate the length that the encoded buffer will be.  To do this
  // we must do a lot of the same stuff that happens in Encode, below.  See
  // the Encode method for more details on what this weirdness is doing.
  
  BcmObjectId TempOID;
  
  if (NeedProprietaryEncoding(fValue) == true)
  {
    // accomodate proprietary encoding flag (0xFF)
    fLength++; 
    
    // Don't compress elements 0 & 1
    if (fValue.Length() >= 1)
      TempOID += fValue[0];
    
    if (fValue.Length() >= 2)
      TempOID += fValue[1];
  }
  
  // Special case check for length 0.  Doesn't need proprietary
  // encoding but TempOID will need to be empty.
  else if (fValue.Length() > 0)
  {
    // Use standard ASN.1 encoding
    TempOID += (fValue[0] * 40) + fValue[1];
  }
  
  // Finish building OID to calculate with
  int Loop;
  for (Loop = 2; Loop < fValue.Length(); Loop++)
    TempOID += fValue[Loop];
  
  // Now figure out how many bytes are required to encode this OID.
  for (Loop = 0; Loop < TempOID.Length(); Loop++)
  {
    fLength++;
    for (int Pool = 4; Pool > 0; Pool--)
    {
      if (TempOID[Loop] >> (7 * Pool))
        fLength++;
    }
  }
}

AsnBerOid::~AsnBerOid ()
{
}

ostream &AsnBerOid::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)
///  outStream << "AsnBerOid, value: " << fValue << endl;
    AsnBerTuple::Print(outStream);
#endif
  return outStream;
}

SNMP_STATUS AsnBerOid::GetValue (BcmObjectId &Oid) const
{ 
  //if (Tag() != (BER_CLASS_UNIVERSAL | BER_F_PRIMITIVE | BER_TLV_OBJECT_IDENTIFIER))
  if (fTag != VBT_OBJECT)
  {
    SnmpLogCfg << "AsnBerOid::GetValue failed, Tag != OID " << Tag() << endl;
    return SNMP_WRONG_TYPE;                                       
  }
  
  Oid = fValue;
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS AsnBerOid::Decode ()
{
  SNMP_STATUS Status;
  const unsigned char *pOctet;
  unsigned long Element;

  
  Status = AsnBerTuple::Decode();
  if (Status != SNMP_NO_ERROR)
  {
///    SnmpLogCfg << "AsnBerTuple::Decode failed " << Status;
    return Status;
  }
  
  // Verify this is an OID tuple:
  if (fTag != VBT_OBJECT)
    return SNMP_WRONG_TYPE;
  
  pOctet = fpValueBuf;
    
  // Initialize to empty OID
  fValue = "";
  
  // Special case: zero length means empty OID.  Return here if we see that.
  if (fLength == 0)
    return SNMP_NO_ERROR;
    
  // The first two elements of the OID are always ecoded into a single octet.
  // This is a small optimization which takes advantage of the fact that the
  // first element may only have values of 0, 1, or 2, and that the second
  // element may be no larger than 39.  The equation is:
  //
  // Octet0 = (40 * Element1) + Element2
  
  // Now, we can also accept special encoding to handle non-standard OIDs.
  // If we have encoded this ourselves, and the OID can't work with the
  // standard formula (see the Encode and NeedProprietaryEncoding methods for 
  // more details) then the first octet will be 0xFF.  Check for that.
  if (*pOctet == 0xFF)
  {
    // In this case, we don't combine the first 2 elements.  So just skip
    // the flag octet and proceed.  Note that we don't need to increment
    // pOctet here in order to skip the flag since that's done inside the 'do'
    // loop below.
  }
  else
  {
    Element = *pOctet / 40;
    fValue += Element;
    
    Element = *pOctet % 40;
    fValue += Element;
  }
  
  // Now we have to decode the rest of the OID one element at a time.  Since 
  // a single element may require multiple octets to encode, we have to check
  // the BER rules about multi-byte OID elements as we go.
  //
  // Note the fLength - 1 here is because pOctet gets incremented before 
  // processing, not after.  So at the top of this while loop, pOctet is
  // pointing to the last octet processed, not the next one to be processed.
  while (pOctet < (fpValueBuf + (fLength - 1)))
  {
    // If we have a multi-byte element, the MSB of the octet will be set.
    // If more than 2 octets are required, the MSB of each octet will be set
    // until we reach the last one, which will have the MSB clear.  The total
    // value will be the concatenation of all the SEVEN bit values.
    Element = 0;
    do
    {
      pOctet++;
      
      Element |= *pOctet & 0x7F;
      
      if (*pOctet & 0x80)
        Element <<= 7;
    }
    while (*pOctet & 0x80);
    
    fValue += Element;
  }
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS AsnBerOid::Encode (BcmString &Buffer) const
{
  // Call base class to get type and length encoded.
  SNMP_STATUS Status = AsnBerTuple::Encode(Buffer);
  
  if (Status != SNMP_NO_ERROR)
    return Status;

  // OIDs are encoded strangely.  For a small optimization, an assumption is
  // made that the first 2 elements will always be in a restricted range. 
  // Namely, element 0 will always be 0, 1, or 2, and element 1 will always
  // be <= 39.  Thus we can combine the 2 first elements into 1 and compress
  // them into a single byte with the formula ((40 * Element0) + Element 1).  
  //
  // Problem with this approach is it limits the ability to encode non-compliant 
  // OIDs.  But we would like to be able to encode non-compliant OIDs if we 
  // need to!  So what we'll do is, use the standard formula so long as we can
  // produce a result which doesn't overrun 0xFFFFFFFF and is not ambiguous.
  // Otherwise we will use a proprietary encoding scheme so at least we can
  // do this internally for our own uses.
  BcmObjectId TempOID;
  
  if (NeedProprietaryEncoding (fValue) == true)
  {
    // Use proprietarty encoding.
    Buffer += (unsigned char)0xFF; // FLAG!
    
    // Then encode 2 elements individually
    // instead of compressing.
    if (fValue.Length() >= 1)
      TempOID += fValue[0];          
      
    if (fValue.Length() >= 2)
      TempOID += fValue[1];          
  }
  
  // Special case check for length 0.  Doesn't need proprietary
  // encoding but TempOID will need to be empty.
  else if (fValue.Length() > 0)
  {
    // Use standard encoding.
    TempOID += (fValue[0] * 40) + fValue[1];
  }
  
  // Finish constructing our temp OID.  By now we have encoded the first 2
  // elements so start position [2].
  int Loop;
  for (Loop = 2; Loop < fValue.Length(); Loop++)
    TempOID += fValue[Loop];
    
  // Now, iterate over the temp OID and encode.
  for (Loop = 0; Loop < TempOID.Length(); Loop++)
  {
    // If the element has a value > 0x7F, we must use multi-byte encoding.  
    // The way this works is, we use as many octets as necessary to encode 
    // the value, where each octet represents a 7 bit value, with the MSB 
    // reserved to indicate that this is a byte in an extended value.  The last 
    // (or only if the value <= 0x7F) byte will have the MSB clear.  Note that 
    // at most, we have a value of 0xFFFFFFFF, which would require 5 bytes to 
    // encode.
    unsigned char Octets[5];
    bool Started = false;
    
    // Break the 32 bit value up into 5 7 bit octets.
    Octets[0] = (TempOID[Loop] >> 28) & 0x7F;
    Octets[1] = (TempOID[Loop] >> 21) & 0x7F;
    Octets[2] = (TempOID[Loop] >> 14) & 0x7F;
    Octets[3] = (TempOID[Loop] >> 7)  & 0x7F;
    Octets[4] = (TempOID[Loop] >> 0)  & 0x7F;
    
    // Iterate over them (starting with the most significant octet) and start 
    // appending them to the buffer as soon as we see the first non-zero value.
    for (int Nest = 0; Nest < 4; Nest++)
    {
      if ((Started == true)
      ||  (Octets[Nest] != 0))
      {
        Octets[Nest] |= 0x80;
        Buffer += Octets[Nest];
        
        // Set the flag so we continue next time
        Started = true;
      }
    }
    
    // We will always use at least the least significant 7 bits,
    // and the MSB of this octet will always be clear.
    Buffer += Octets[4];
  }
  
  // And we are done!
  return SNMP_NO_ERROR;
}

bool AsnBerOid::NeedProprietaryEncoding (const BcmObjectId &OID) const
{
  // We can't properly encode an OID with exactly 1 element using the standard
  // scheme, lest it be ambiguous.  For example, say we have .1.  If we encode
  // it as (40 * 1) that gives us 40, which is ambiguous with .1.0.  If we
  // encode it as 1, that's ambiguous with 0.1.  So for our purposes any OID 
  // with 1 element gets proprietary encoding.  
  if (OID.Length() == 1)
    return true;
    
  // Another special case is 0 elements.  This does not require proprietary
  // encoding because the length of the OID tuple will be 0.  However
  // do check for this special case because below we check OID[0] and OID[1]
  // which will be undefined for a 0 length OID.
  if (OID.Length() == 0)
    return false;

  // Check to make sure we can do this without overrun.
  // We would overrun if (element[0] * 40) > (0xFFFFFFFF - 40).
  if (OID[0] > (0xFFFFFFFF / 41))
    return true;
  
  // Check for ambiguity.  Standard encoding will yield an ambiguous result if
  // element[1] >= 40 (since 0.41 would encode the same as 1.1, for example).
  if (OID[1] >= 40)
    return true;
  
  // Another thing we have to check for is to be sure that the value we are
  // encoding the traditional way won't ecode the first octet to 0xFF.  In 
  // order to get an 0xFF in the first position, we'd have to have 0x7F in the 
  // most significant 7 bits.  Meaning 0x7F in bits 7-13, 14-20, or 21-28, 
  // depending on the value being encoded.  So if ((element0 * 40) + element0) 
  // has 0x7F in these positions, we must use proprietary encoding.
  unsigned long Encoded = (OID[0] * 40) + OID[1];
  
  // We can accomplish this by shifting the encoded value right 7 bits at
  // a time, and then checking to see if the resulting value is exactly
  // 0x7F.  We don't want to apply a mask because 0x7F is OK as long as it's
  // not the leftmost nonzero bits.  We want to shift right before doing the
  // first check because exactly 0x7F is OK, too.
  for (int Loop = 0; Loop < 3; Loop++)
  {
    Encoded >>= 7;
    if (Encoded == 0x7F)
      return true;
  }
  
  return false;
}

/*\
 *  AsnBerNull
\*/

AsnBerNull::AsnBerNull (const unsigned char *pBerBuf, int Length)
  : AsnBerTuple (pBerBuf, Length)
{
	Decode();
}

AsnBerNull::~AsnBerNull ()
{
}

ostream &AsnBerNull::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)
///  outStream << "AsnBerNull, value:" << endl;
    AsnBerTuple::Print(outStream);
#endif
  return outStream;
}

SNMP_STATUS AsnBerNull::GetValue () const
{ 
  //if (Tag() != (BER_CLASS_UNIVERSAL | BER_F_PRIMITIVE | BER_TLV_NULL))
  if (fTag != VBT_EMPTY)
  {
    SnmpLogCfg << "AsnBerNull::GetValue failed, Tag != NULL " << Tag() << endl;
    return SNMP_WRONG_TYPE;
  }
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS AsnBerNull::Decode ()
{
  SNMP_STATUS Status;
  const unsigned char *pOctet;


///  SnmpLogCfg << "AsnBerNull::Decode" << endl;

  Status = AsnBerTuple::Decode();
  if (Status != SNMP_NO_ERROR)
  {
///    SnmpLogCfg << "AsnBerTuple::Decode failed " << Status << endl;
    return Status;
  }
  
  // Verify this is a NULL tuple:
  if (fTag != VBT_EMPTY)
    return SNMP_WRONG_TYPE;
  
  pOctet = fpValueBuf;
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS AsnBerNull::Encode (BcmString &Buffer) const
{
  // Don't want to call base class to get type and length encoded.
  // It will put long format on length. - Just do it here.
  
  unsigned char pNullVal[] = {0x05, 0x00};
  Buffer.append ((char *)pNullVal, sizeof(pNullVal));

  return SNMP_NO_ERROR;
}


/*\
 *  AsnBerVarBind
\*/

AsnBerVarBind::AsnBerVarBind (const unsigned char *pBerBuf, int Length)
{
///  SnmpLogCfg << "Creating AsnBerVarBind from buffer at " << (unsigned long) pBerBuf << endl;

  fSpecifiedLength = Length;

  fpBerBuf = pBerBuf;
  fLength = 0;
  
  Decode();
}

AsnBerVarBind::AsnBerVarBind (const BcmObjectId &Oid, const BcmVarBind &Vb)
{
  fSpecifiedLength = -1;
  
  fpBerBuf = NULL;
  fLength = 0;
  
  fOid = Oid;
  fVb = Vb;
  
  SnmpLogCfg << "AsnBerVarBind constructor." << endl;
  SnmpLogCfg << "  OID = " << Oid << endl;
  SnmpLogCfg << "  VB = " << Vb << endl;

  // Format our buffer and adopt the results as fpBerBuf
  Encode (fBuffer);
  fpBerBuf = (unsigned char *)fBuffer.data();
  
  SnmpLogCfg << "  Encoded, buffer = " << fpBerBuf << endl;
  
  // Now decode which will create our fVbTuple, fValueTuple, and fOidTuple
  Decode();
  
  SnmpLogCfg << "  Decoded." << endl;
}

AsnBerVarBind::~AsnBerVarBind ()
{
}

SNMP_STATUS AsnBerVarBind::SetBuffer (const unsigned char *pBerBuf, int Length)
{
  fSpecifiedLength = Length;

  fpBerBuf = pBerBuf;
  fpValueBuf = NULL;
  
  fLength = 0;
  
  return Decode();
}


SNMP_STATUS AsnBerVarBind::Decode ()
{
  fVbTuple.SetBuffer (fpBerBuf);
  
  // First tuple better be a SEQUENCE
  if (fVbTuple.Tag() != (BER_CLASS_UNIVERSAL | BER_F_CONSTRUCTED | BER_TLV_SEQUENCE))
  {
    SnmpLogCfg << "AsnBerVarBind VB Tag " << fVbTuple.Tag() << " is not SEQUENCE" << endl;
    fVbTuple = NULL;
    return SNMP_WRONG_TYPE;
  }
  
  // Next is the length of the whole var bind.
  fLength = fVbTuple.TotalLength();
  SnmpLogCfg << "AsnBerVarBind OK, length of " << fLength << endl;
  //SnmpLogCfg << fVbTuple << endl;
  
  SnmpLogCfg << "Calling fOidTuple.SetBuffer" << endl;
  fOidTuple.SetBuffer (fVbTuple.ValueBuf(), fSpecifiedLength);
  SnmpLogCfg << "fOidTuple.SetBuffer returned.  Calling fOidTuple.Tag()" << endl;
  
  // This better be an OID
  //if (fOidTuple.Tag() != (BER_CLASS_UNIVERSAL | BER_F_PRIMITIVE | BER_TLV_OBJECT_IDENTIFIER))
  if (fOidTuple.Tag() != VBT_OBJECT)
  {
    SnmpLogCfg << "AsnBerVarBind OID Tag " << fOidTuple.Tag() << " is not OID\n" << endl;
    fOidTuple.SetBuffer (NULL);
    return SNMP_WRONG_TYPE;
  }
  
  SnmpLogCfg << "AsnBerVarBind OID tuple OK, length of " << fOidTuple.Length()  << endl; 
  SnmpLogCfg << fOidTuple << endl;
  
  // Finally, the actual value
  fValueTuple.SetBuffer(fOidTuple.ValueBuf() + fOidTuple.Length());
  fpValueBuf = fValueTuple.ValueBuf();
  SnmpLogCfg << "AsnBerVarBind VAL tuple OK, length of " << fValueTuple.Length() << endl;
  SnmpLogCfg << fValueTuple << endl;
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS AsnBerVarBind::Encode (BcmString &Buffer) const
{
  // First, construct the OID tuple.
  AsnBerOid OidTuple(fOid);
  
  // Next, construct the value tuple
  AsnBerTuple *pValueTuple = NULL;
  
  switch (fVb.GetType())
  {
    // All these use the ASN.1 simple type of INTEGER, then indicate
    // the type via the tuple's TAG.
    case VBT_NUMBER:
    {
      int Value;
      fVb.GetInt32 (&Value);
    
      pValueTuple = new AsnBerInt32 (Value);
      break;
    }
    
    case VBT_COUNTER:
    case VBT_UINTEGER32:
    case VBT_GAUGE:
    {
      unsigned int Value;
      fVb.GetUint32 (&Value);
    
      pValueTuple = new AsnBerUint32 (Value, fVb.GetType());
      break;
    }
    
    case VBT_TIMETICKS:
    {
      unsigned long Value;
      fVb.GetTimeticks (&Value);
    
      pValueTuple = new AsnBerUint32 (Value, fVb.GetType());
      break;
    }

    // IP address is encoded like a string, so we'll use the string
    // decoders.  But we need a seperate case here to properly set the
    // varbind to IP.
    case VBT_IPADDRESS:
    {
      BcmIpAddress Value;
      fVb.GetIp (Value);
    
      pValueTuple = new AsnBerIpAddress (Value);
      break;
    }
      
    // String-like things are all encoded as ASN.1 simple type of STRING, 
    // specific type indicated by tag
    case VBT_STRING:
    case VBT_OPAQUE:
    {
      BcmString Value;
      fVb.GetString (Value);
    
      pValueTuple = new AsnBerOctetString (Value);
      break;
    }
      
    // OID's are uniquely encoded
    case VBT_OBJECT:
    {
      BcmObjectId Value;
      fVb.GetOid (Value);
    
      pValueTuple = new AsnBerOid (Value);
      break;
    }

/*
    // So are BITSTRING's.  These are not the same as VBT_BITS, which are 
    // really just octet strings.
    case ASN_1_BITSTRING:
    {
      BcmString Value;
      fVb.GetString (&Value);
    
      pValueTuple = new AsnBerBitstring (Value, 0);
      break;
    }
*/
      
    // So are NULL's (why would you set one of these?)
    case ASN_1_NO_ID:
    case VBT_EMPTY:
    {
      pValueTuple = new AsnBerNull ();
      break;
    }
  }
  
  if (pValueTuple == NULL)
    return SNMP_WRONG_TYPE;

  BcmString SequenceBuf, OidBuf, ValueBuf;

  // Encode OID and value now  
  OidTuple.Encode(OidBuf);
  pValueTuple->Encode(ValueBuf);

  // Now that we know the length of the OID and the length of the value
  // we can create the sequence tuple and encode it.
  AsnBerTuple SequenceTuple(BER_CLASS_UNIVERSAL | BER_F_CONSTRUCTED | BER_TLV_SEQUENCE, OidBuf.length() + ValueBuf.length());
  SequenceTuple.Encode(SequenceBuf);
  
  // The final result is...
  Buffer  = SequenceBuf;
  Buffer += OidBuf;
  Buffer += ValueBuf;

  delete pValueTuple;

  return SNMP_NO_ERROR;
}

unsigned int AsnBerVarBind::Length() const
{
  return fLength;
}

SNMP_STATUS AsnBerVarBind::GetOid (BcmObjectId &Oid) const
{
  return fOidTuple.GetValue(Oid);
}

SNMP_STATUS AsnBerVarBind::GetValue (BcmVarBind &Vb) const
{
  SnmpLogCfg << "AsnBerVarBind::GetValue" << endl;

  switch (fValueTuple.Tag())
  {
    // All these use the ASN.1 simple type of INTEGER, then indicate
    // the type via the tuple's TAG.
    case VBT_NUMBER:
    {
      AsnBerInt32 BerInt;
      int Value = 0;
      
      BerInt.SetBuffer (fValueTuple.BerBuf());
      BerInt.GetValue (Value);
      
      SnmpLogCfg << "Integer value=" << Value << endl;
      
      SnmpLogCfg << "Setting VB to Int32 value=" << Value << endl;
      Vb.SetInt32 (Value);
      
      break;
    }
    
    case VBT_COUNTER:
    case VBT_UINTEGER32:
    case VBT_GAUGE:
    {
      AsnBerUint32 BerUint;
      unsigned int Value = 0;
      
      BerUint.SetBuffer (fValueTuple.BerBuf());
      BerUint.GetValue (Value);
      
      SnmpLogCfg << "Uint32-type value " << fValueTuple.Tag() << ", Value=" << Value << endl;
      
      SnmpLogCfg << "Setting VB to Uint32 value=" << Value << " type=" << fValueTuple.Tag() << endl;
      Vb.SetUint32 (Value, fValueTuple.Tag());
      
      break;
    }
    
    case VBT_COUNTER64:
    {
      AsnBerUint64 BerUint;
      BcmCounter64 Value = 0;
      
      BerUint.SetBuffer (fValueTuple.BerBuf());
      BerUint.GetValue (Value);
      
      SnmpLogCfg << "Uint64 value =" << Value << endl;
      
      SnmpLogCfg << "Setting VB to Uint64 value=" << Value << " type=" << fValueTuple.Tag() << endl;
      Vb.SetUint64 (Value);
      
      break;
    }
    
    case VBT_TIMETICKS:
    {
      AsnBerUint32 BerUint;
      unsigned int Value = 0;
      
      BerUint.SetBuffer (fValueTuple.BerBuf());
      BerUint.GetValue (Value);
      
      SnmpLogCfg << "Timeticks value=" << Value << endl;
      
      SnmpLogCfg << "Setting VB to timeticks value=" << Value << endl;
      Vb.SetTimeticks (Value);
      
      break;
    }

    // IP address is encoded like a string, so we'll use the string
    // decoders.  But we need a seperate case here to properly set the
    // varbind to IP.
    case VBT_IPADDRESS:
    {
      AsnBerIpAddress BerIp;
      BcmIpAddress Ip;
    
      SnmpLogCfg << "Value tuple's tag = VBT_IPADDRESS" << endl;
      BerIp.SetBuffer (fValueTuple.BerBuf());
      BerIp.GetValue (Ip);
      
      Vb.SetIp (Ip);
      SnmpLogCfg << Vb << endl;
      
      /*
      // 3/16/05 - use new AsnBerIpAddress object for this now!
      AsnBerOctetString BerStr;
      BcmString String;
      
      SnmpLogCfg << "Value tuple's tag = VBT_IPADDRESS" << endl;
      BerStr.SetBuffer (fValueTuple.BerBuf());
      BerStr.GetValue (String);
      
      // Theoretically I suppose we could encounter an IP address tuple
      // which is not exactly 4 bytes.  Nothing to do in that case, 
      // other than print a warning and treat it like a 4 byte IP
      if (String.length() != 4)
        gLogMessageRaw << "WARNING: AsnBerVarBind VBT_IPADDRESS length " << String.length() << ", using 4 bytes." << endl;
      
      Vb.SetIp ((unsigned char *)String.data());
      SnmpLogCfg << Vb << endl;
      */
      break;
    }
      
    // String-like things are all encoded as ASN.1 simple type of STRING, 
    // specific type indicated by tag
    case VBT_STRING:
    case VBT_OPAQUE:
    {
      AsnBerOctetString BerStr;
      BcmString String;
      
      SnmpLogCfg << "Value tuple's tag = VBT_STRING" << endl;
      BerStr.SetBuffer (fValueTuple.BerBuf());
      BerStr.GetValue (String);
      
      // We don't really handle opaque, which is probably OK since I have never
      // actually seen one.  But if we get one then yell to the console.
      if (fValueTuple.Tag() == VBT_OPAQUE)
        gLogMessageRaw << "WARNING: AsnBerVarBind opaque varbind!  Using VBT_STRING." << endl;
      
      Vb.SetString ((unsigned char *)String.data(), String.length());
      SnmpLogCfg << Vb << endl;
      break;
    }
      
    // OID's are uniquely encoded
    case VBT_OBJECT:
    {
      AsnBerOid BerOid;
      BcmObjectId Oid;
      
      SnmpLogCfg << "Value tuple's tag = VBT_OBJECT" << endl;
      BerOid.SetBuffer(fValueTuple.BerBuf());
      
      BerOid.GetValue (Oid);
      Vb = Oid;
      SnmpLogCfg << Oid << endl;
      break;
    }
    
    // PR 11894:  Support the noSuchObject and noSuchInstance exception types
    case VBT_NOSUCHOBJECT:
    {
      Vb.SetNoSuchObject();
      break;
    }
    
    case VBT_NOSUCHINSTANCE:
    {
      Vb.SetNoSuchInstance();
      break;
    }

/*    
    // So are BITSTRING's.  These are not the same as VBT_BITS, which are 
    // really just octet strings.
    case ASN_1_BITSTRING:
    {
      AsnBerBitString BerBits;
      BcmString Bitstring;
      int UnusedBits;
      
      SnmpLogCfg << "Value tuple's tag = ASN_1_BITSTRING" << endl;
      BerBits.SetBuffer(fValueTuple.BerBuf());
      
      BerBits.GetValue (Bitstring, UnusedBits);
      
      //VARBIND class doesn't handle bitstrings!  Gotta code that up...
      //Vb.SetBitstring(Bitstring, UnusedBits);
      break;
    }
*/
      
    // Some special snmpv2 tags)
    case VBT_ENDOFMIBVIEW:
    {
      AsnBerNull BerNull;
      
      SnmpLogCfg << "Value tuple's tag = VBT_EMPTY" << endl;
      BerNull.SetBuffer (fValueTuple.BerBuf());
      
      Vb.SetEmpty(fValueTuple.Tag());
      break;
    }
    
    // So are NULL's (why would you set one of these?)
    case ASN_1_NO_ID:
    case VBT_EMPTY:
    {
      AsnBerNull BerNull;
      
      SnmpLogCfg << "Value tuple's tag = VBT_EMPTY" << endl;
      BerNull.SetBuffer (fValueTuple.BerBuf());
      
      Vb.SetEmpty();
      break;
    }
    
    // Unknown type.
    default:
      gLogMessageRaw << "WARNING: AsnBerVarBind unknown VarBind type: " << (int)fValueTuple.Tag() << endl;
      break;
  }
  
  SnmpLogCfg << "AsnBerVarBind::GetValue value of VB is " << Vb << endl;
  
  return SNMP_NO_ERROR;
}

ostream &AsnBerVarBind::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  outStream << "AsnBerVarBind, length = " << fLength << endl;
  outStream << "VB Tuple = " << fVbTuple << endl;
  outStream << "Value Tuple = " << fValueTuple << endl;
  outStream << "OID Tuple = " << fOidTuple << endl;
#endif  
  return outStream;
}
