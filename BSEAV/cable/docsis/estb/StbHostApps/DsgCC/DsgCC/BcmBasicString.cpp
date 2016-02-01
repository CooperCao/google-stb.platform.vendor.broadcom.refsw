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


BcmBasicString::BcmBasicString (const BcmIpAddress &IP, bool DottedDecimal)
{
  Set (IP, false);
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
    char DottedString[16];
    
    IP.Get (DottedString);
    fString = DottedString;
  }
  
  else
  {
    fString = "";
    
    fString += IP[0];
    fString += IP[1];
    fString += IP[2];
    fString += IP[3];
  }
  
  return *this;
}

bool BcmBasicString::operator < (const BcmBasicString &s) const
{
    return (fString < s.fString);
}
