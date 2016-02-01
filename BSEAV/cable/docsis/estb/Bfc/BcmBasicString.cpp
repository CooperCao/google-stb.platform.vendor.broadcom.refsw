//****************************************************************************
//
// Copyright (c) 2011 Broadcom Corporation
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
//    Filename: BcmBasicString.cpp
//    Author:   Kevin O'Neal
//    Creation Date: 18-April-2000
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Wrapper for an STL basic_string to eliminate inline expansions.
//    The sole reason for doing this is to reduce code size.  Turns out that
//    savings can be considerable: ~85k for the pSOS build as of this
//    release.  Note that this object does not wrap all basic_string
//    methods, just the ones we need to build.  Since the whole point of
//    this is code savings, I intentionally left out all unused methods.
//    Any method that needs to be added later can be, you get the idea.
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include "BcmBasicString.h"
#include "OctetBuffer.h"
#include "IpAddress.h"
#include "MacAddress.h"


BcmBasicString::BcmBasicString ()
{
}

BcmBasicString::BcmBasicString (const E *s)
  : fString (s)
{
}

BcmBasicString::BcmBasicString (const E *s, size_type n)
  : fString (s, n)
{
}

// This constructor doesn't match one of the standard basic_string
// constructors, but has been added because we have had many problems
// constructing this object with unsigned characters instead of signed
// ones.
BcmBasicString::BcmBasicString (const unsigned char *s, size_type n)
  : fString ((const E *)s, n)
{
}

BcmBasicString::BcmBasicString (const string &s)
  // this gives the dreaded "variable $$166.pad is used before set" warning, so
  // just set fString in the body of the constructor, below...
  //: fString (s) 
{
  fString = s;
}

BcmBasicString::BcmBasicString (const BcmBasicString &s)
  // this gives the dreaded "variable $$166.pad is used before set" warning, so
  // just set fString in the body of the constructor, below...
  //: fString (s.fString) 
{
  fString = s.fString;
}

BcmBasicString::~BcmBasicString ()
{
}

bool BcmBasicString::operator == (const E *s) const
{
  return (fString == s);
}

bool BcmBasicString::operator == (const BcmBasicString &s) const
{
  return (fString == s.fString);
}

bool BcmBasicString::operator != (const E *s) const
{
  return !(fString == s);
}

bool BcmBasicString::operator != (const BcmBasicString &s) const
{
  return (fString != s.fString);
}

BcmBasicString &BcmBasicString::operator = (E c)
{
  fString = c;
  return *this;
}

BcmBasicString &BcmBasicString::operator = (const E *s)
{
  fString = s;
  return *this;
}

BcmBasicString &BcmBasicString::operator = (const BcmBasicString &s)
{
  fString = s.fString;
  return *this;
}

BcmBasicString &BcmBasicString::operator += (E c)
{
  fString += c;
  return *this;
}

BcmBasicString &BcmBasicString::operator += (const E *s)
{
  fString += s;
  return *this;
}

BcmBasicString &BcmBasicString::operator += (const BcmBasicString &rhs)
{
  fString += rhs.fString;
  return *this;
}

//BcmBasicString::const_reference BcmBasicString::operator [] (size_type pos) const
const BcmBasicString::E BcmBasicString::operator [] (size_type pos) const
{
  return (E)fString[pos];
}

BcmBasicString::reference BcmBasicString::operator [] (size_type pos)
{
  return fString[pos];
}

BcmBasicString::size_type BcmBasicString::length () const
{
  return fString.length();
}

BcmBasicString::size_type BcmBasicString::size () const
{
  return fString.size();
}

bool BcmBasicString::empty () const
{
  return fString.empty();
}

BcmBasicString::size_type BcmBasicString::capacity () const
{
  return fString.capacity();
}

void BcmBasicString::reserve ()
{
  fString.reserve();
}

void BcmBasicString::reserve (size_type num)
{
  fString.reserve( num );
}

const BcmBasicString::E *BcmBasicString::c_str () const
{
  return fString.c_str();
}

const BcmBasicString::E *BcmBasicString::data () const
{
  return fString.data();
}

BcmBasicString &BcmBasicString::assign (const E *s, size_type n)
{
  fString.assign (s, n);
  return *this;
}

BcmBasicString &BcmBasicString::assign (const unsigned char *us, size_type n)
{
  fString.assign ((const E *)us, n);
  return *this;
}

BcmBasicString &BcmBasicString::assign (const BcmBasicString &str, size_type pos, size_type n)
{
  fString.assign (str.fString, pos, n);
  return *this;
}

BcmBasicString &BcmBasicString::insert(size_type p0, const E *s)
{
  fString.insert (p0, s);
  return *this;
}

BcmBasicString &BcmBasicString::append(const E *s, size_type n)
{
  fString.append (s, n);
  return *this;
}

BcmBasicString &BcmBasicString::append(const unsigned char *us, size_type n)
{
  fString.append ((const E *)us, n);
  return *this;
}

BcmBasicString::size_type BcmBasicString::find (const BcmBasicString& str, size_type pos) const
{
  return fString.find (str.fString, pos);
}

BcmBasicString::size_type BcmBasicString::find_first_of (E c, size_type pos) const
{
  return fString.find_first_of (c, pos);
}

BcmBasicString BcmBasicString::substr (size_type pos, size_type n) const
{
  return BcmBasicString (fString.substr(pos, n));
}

void BcmBasicString::swap (BcmBasicString &str)
{
  fString.swap( str.fString );
}

void BcmBasicString::clear ()
{
  fString.clear();
}

ostream &BcmBasicString::Print(ostream &outStream) const
{
  outStream << fString;
  return outStream;
}

/*\
 *  Methods below here are not STL drop-ins, they are V2 specific
\*/

BcmBasicString::BcmBasicString (const BcmOctetBuffer &Buffer)
{
  Set (Buffer);
}

BcmBasicString &BcmBasicString::operator = (const BcmOctetBuffer &Buffer)
{
  Set (Buffer);
  return *this;
}

BcmBasicString &BcmBasicString::Set (const BcmOctetBuffer &Buffer)
{
  fString.assign ((const char *)Buffer.AccessBuffer(), Buffer.BytesUsed());
  return *this;
}

// This is intended to be used as if we're saying MyBuffer = MyString, but
// without making an = operator for BcmOctetBuffer that takes a BcmString.
// So MyString.Get(MyBuffer) is equivalent to MyBuffer = MyString.  It just
// copies the string's data into the buffer.
void BcmBasicString::Get (BcmOctetBuffer &Buffer) const
{
  // Make sure we have enough space to hold the data.  If we don't,
  // make room for it.
  if (Buffer.BufferSize() < length())
    Buffer.ResizeBuffer (length());
  
  // If we already have room, just empty out the buffer.
  else
    Buffer.Empty();
  
  // Copy our data into the buffer  
  Buffer.AddToEnd ((uint8*)data(), length());
}


bool BcmBasicString::WriteTo(BcmOctetBuffer &octetBuffer) const
{
  return octetBuffer.AddToEnd((uint8 *)data(), length());
}

bool BcmBasicString::ReadFrom(const BcmOctetBuffer &octetBuffer, unsigned int &offset, unsigned int numBytes)
{
  bool ReadIt;
  unsigned char *pBuf = new unsigned char [numBytes];
  
  ReadIt = octetBuffer.Read(offset, pBuf, numBytes);
  
  if (ReadIt)
    assign(pBuf, numBytes);
    
  delete[] pBuf;
  
  return ReadIt;
}


BcmBasicString::BcmBasicString (const BcmIpAddress &IP, bool DottedDecimal)
{
  Set (IP, DottedDecimal);
}

BcmBasicString &BcmBasicString::operator = (const BcmIpAddress &IP)
{
  Set (IP);
  return *this;
}

BcmBasicString &BcmBasicString::Set (const BcmIpAddress &IP, bool DottedDecimal)
{
  if (DottedDecimal)
  {
    char DottedString[64];
    
    IP.Get (DottedString, sizeof( DottedString ));
    fString = DottedString;
  }
  
  else 
  {
    fString = "";
    
    fString += IP[0];
    fString += IP[1];
    fString += IP[2];
    fString += IP[3];
    
    // PR 12595
    if (IP.IsIpV6Address() == true)
    {
      fString += IP[4];
      fString += IP[5];
      fString += IP[6];
      fString += IP[7];
      fString += IP[8];
      fString += IP[9];
      fString += IP[10];
      fString += IP[11];
      fString += IP[12];
      fString += IP[13];
      fString += IP[14];
      fString += IP[15];
    }
  }
  
  return *this;
}

bool BcmBasicString::Get (BcmIpAddress &IP, bool DottedDecimal) const
{
  if (DottedDecimal)
    return IP.Set(fString.c_str());
  
  if (fString.length() != 4)
    return false;
    
  IP.Set((uint8)fString[0],(uint8)fString[1],
         (uint8)fString[2],(uint8)fString[3]);
  
  return true;
}

BcmBasicString::BcmBasicString (const BcmMacAddress &MAC, bool Ascii)
{
  Set (MAC, false);
}

BcmBasicString &BcmBasicString::operator = (const BcmMacAddress &MAC)
{
  Set (MAC);
  return *this;
}

BcmBasicString &BcmBasicString::Set (const BcmMacAddress &MAC, bool Ascii)
{
  if (Ascii)
  {
    char AsciiString[32];
    
    MAC.Get (AsciiString);
    fString = AsciiString;
  }
  
  else
  {
    BcmOctetBuffer Buf(16);
    
    MAC.WriteTo (Buf);
    Set (Buf);
  }
  
  return *this;
}

bool BcmBasicString::Get (BcmMacAddress &MAC, bool Ascii) const
{
  if (Ascii)
    return MAC.Set(fString.c_str());
  
  if (fString.length() != 6)
    return false;
    
  MAC.Set((uint8)fString[0],(uint8)fString[1],
          (uint8)fString[2],(uint8)fString[3],
          (uint8)fString[4],(uint8)fString[5]);
  
  return true;
}

bool BcmBasicString::operator < (const BcmBasicString &s) const
{
    return (fString < s.fString);
}

