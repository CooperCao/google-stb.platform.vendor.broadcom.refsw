//**************************************************************************
//
//    Copyright 2003 Broadcom Corporation
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
//    Filename: ObjectIdentifier.cpp
//    Author:   Kevin O'Neal
//    Creation Date: 18-April-2000
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    BcmObjectId moved to its own module.
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include "ObjectIdentifier.h"
#include "IpAddress.h"
#include "SnmpLog.h"

/*\
 *  BcmObjectId methods
\*/

BcmObjectId::BcmObjectId ()
{
  fpOID = NULL;
  fIndexOffset = UNSPECIFIED_INDEX_OFFSET;
}

// Construct with up to 8 elements
BcmObjectId::BcmObjectId (unsigned long Obj0, 
                          unsigned long Obj1, 
                          unsigned long Obj2, 
                          unsigned long Obj3, 
                          unsigned long Obj4, 
                          unsigned long Obj5, 
                          unsigned long Obj6, 
                          unsigned long Obj7)
{
  fpOID = NULL;
  fIndexOffset = UNSPECIFIED_INDEX_OFFSET;
  
  Set (&Obj0, 1);
  
  if (Obj1 != UNUSED_OID_ARG)
    *this += Obj1;
  if (Obj2 != UNUSED_OID_ARG)
    *this += Obj2;
  if (Obj3 != UNUSED_OID_ARG)
    *this += Obj3;
  if (Obj4 != UNUSED_OID_ARG)
    *this += Obj4;
  if (Obj5 != UNUSED_OID_ARG)
    *this += Obj5;
  if (Obj6 != UNUSED_OID_ARG)
    *this += Obj6;
  if (Obj7 != UNUSED_OID_ARG)
    *this += Obj7;  
}

BcmObjectId::BcmObjectId (const BcmObjectId &OID)
{
  fpOID = NULL;
  fIndexOffset = OID.fIndexOffset;
  fObjects = OID.fObjects;
}

BcmObjectId::BcmObjectId (ObjectId *pOID)
{
  fpOID = NULL;
  
  if (pOID == NULL)
    fIndexOffset = UNSPECIFIED_INDEX_OFFSET;
  else
  {
    Set (pOID->pObjects, pOID->Length);
    fIndexOffset = pOID->IndexOffset;
    
    fpOID = pOID;
    
    // Calculate leaf in case the OID doesn't have it assigned.
    fpOID->Leaf = Leaf();
  }
}

BcmObjectId::BcmObjectId (int Length, const unsigned long *pObjects)
{
  fpOID = NULL;
  Set (pObjects, Length);
}

// accept a dotted-decimal NULL terminated text string and
// parse the OID from it.  EX: ".1.3.6.1" or "1.3.6.1"
BcmObjectId::BcmObjectId (const char *pString, ParseMode Parse)
{
  fpOID = NULL;
  
  BcmString String;

  if (pString)
    String = pString;
  
  if (Parse == kDottedDecimal)
    Set (String);
  else
    SetFromAscii (String);
}

BcmObjectId::BcmObjectId (const BcmString &String, ParseMode Parse)
{
  fpOID = NULL;
  
  if (Parse == kDottedDecimal)
    Set (String);
  else
    SetFromAscii (String);
}

BcmObjectId::BcmObjectId (const BcmMacAddress &Mac)
{
  fpOID = NULL;
  Set (Mac);
}

BcmObjectId::BcmObjectId (const BcmIpAddress &IP, bool PrependLength, bool PrependType)
{
  fpOID = NULL;
  Set (IP, PrependLength, PrependType);
}

BcmObjectId::BcmObjectId (const BcmOctetBuffer &Buffer)
{
  fpOID = NULL;
  Set (Buffer);
}

BcmObjectId::~BcmObjectId ()
{
}

int BcmObjectId::IndexOffset (int Offset)
{
  if ((Offset < 0) || (Offset > Length()))
    fIndexOffset = UNSPECIFIED_INDEX_OFFSET;
  else
    fIndexOffset = Offset;
    
  UpdateOID();
    
  return IndexOffset();
}

unsigned long BcmObjectId::Leaf () const
{
    unsigned long Leaf;

    if ((IndexOffset() > 0) && (IndexOffset() < Length()))
      Leaf = fObjects[IndexOffset()-1];
    else if (Length() > 0)
      Leaf = fObjects[Length() - 1];
    else
      Leaf = UNSPECIFIED_LEAF;
      
    return Leaf;
}

SNMP_STATUS BcmObjectId::GetIndex (BcmObjectId &Index) const
{
  // Initialize index to ""
  Index = "";
  
  // If we don't know our index offset, we can't build the index.
  if (IndexOffset() == UNSPECIFIED_INDEX_OFFSET)
    return SNMP_RESOURCE_UNAVAILABLE;
  
  for (int Loop = IndexOffset(); Loop < Length(); Loop++)
    Index += fObjects[Loop];
    
  // May as well point the index offset of Index to 0...
  Index.IndexOffset(0);
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmObjectId::Set (const unsigned long *pObjects, int Length)
{
  int Loop;
  
  

  fObjects.resize(Length);
  
  if (pObjects)
  {
    for (Loop = 0; Loop < Length; Loop++)
      fObjects[Loop] = pObjects[Loop];
  }
  
  // Clear index offset
  IndexOffset(UNSPECIFIED_INDEX_OFFSET);
  
  UpdateOID();
  
  return SNMP_NO_ERROR;
}

// accept a dotted-decimal NULL terminated text string and
// parse the OID from it.  EX: ".1.3.6.1" or "1.3.6.1"
// Returns SNMP_NO_ERROR if the string is valid, SNMP_BAD_VALUE
// if it is bogus.
SNMP_STATUS BcmObjectId::Set (const char *pString)
{
  char  IntStr[16];
  int Loop = 0, IntIdx = 0;
  int Length = strlen (pString);
  ObjectId OID;


  SnmpLogInit << "Creating OID from string " << pString << ", length " << Length << endl;

  if (Length == 0)
  {
    Set (NULL, 0);
    return SNMP_NO_ERROR;
  }
  
  memset (&OID, 0, sizeof (ObjectId));
  memset (IntStr, 0, sizeof (IntStr));
  
  // Actually, we'll take (almost) any old delimiters, and ignore whitespace
  // between objects.  Start spinning:
  for (Loop = 0; Loop < Length; Loop++)
  {
    // Objects...
    if ((pString[Loop] >= '0') && (pString[Loop] <= '9'))
    {
      IntStr[IntIdx] = pString[Loop];
      IntIdx++;
      
      // Longest object can be no more than 15 characters or we blow past
      // max size for a ULONG.  This is not a complete check, because some
      // 15 character numeric strings will still be too big, but it's better
      // than nothing.
      if (IntIdx > 15)
      {
        SnmpLogInit <<  "Invalid OID string, object " << IntStr << "too long." << endl;
        return SNMP_BAD_VALUE;
      }
    }
    
    // Valid delimeters are as follows:
    // [space] ! " # $ % & ' ( ) * + , - . / : ; < = > ? @
    // which are ASCII 0x20 - 0x40, excluding 0x30-0x39 which are
    // '0' through '9' as checked in the first if, above.  Anything
    // else is invalid and will result in an error code.
    else if ((pString[Loop] < 0x20) || (pString[Loop] > 0x40))
    {
      SnmpLogInit << "Invalid OID string, character " << pString[Loop] << " not valid." << endl;
      return SNMP_BAD_VALUE;
    }
    
    // Found a reasonable delimeter and we have at least one character between
    // delimeters
    else if (IntIdx)
    {
      IntStr[IntIdx] = 0;
      OID.pObjects[OID.Length] = atol (IntStr);
      OID.Length++;
      IntIdx = 0;
      memset (IntStr, 0, sizeof (IntStr));
    }
    
    // 2 delimeters in succession without a numeric value in between, bad!
    else if (Loop != 0)
    {
      SnmpLogInit << "Invalid OID string, multiple delimeters in a row." << endl;
      return SNMP_BAD_VALUE;
    }
  }
  
  if (IntIdx)
  {
    IntStr[IntIdx] = 0;
    OID.pObjects[OID.Length] = atol (IntStr);
    OID.Length++;
    IntIdx = 0;
    memset (IntStr, 0, sizeof (IntStr));
  }
  
  Set (OID.pObjects, OID.Length);
  UpdateOID();
  
  return SNMP_NO_ERROR;
}

// accept a dotted-decimal NULL terminated text string and
// parse the OID from it.  EX: ".1.3.6.1" or "1.3.6.1"
// Returns SNMP_NO_ERROR if the string is valid, SNMP_BAD_VALUE
// if it is bogus.
SNMP_STATUS BcmObjectId::Set (const BcmString &String)
{
  return Set (String.c_str());
}

SNMP_STATUS BcmObjectId::SetFromAscii (const BcmString &String)
{
  unsigned int Loop;
  unsigned char Byte;
  
  Set (NULL, 0);
  for (Loop = 0; Loop < String.length(); Loop++)
  {
    Byte = (unsigned char) String[Loop];
    *this += (unsigned long) Byte;
  }
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmObjectId::Set (const BcmMacAddress &Mac)
{
  unsigned char ucMac[6];
  
  Mac.Get (ucMac[0], ucMac[1], ucMac[2], ucMac[3], ucMac[4], ucMac[5]);
  Set (NULL, 0);
  
  for (int Loop = 0; Loop < 6; Loop++)
    *this += ucMac[Loop];
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmObjectId::Set (const BcmIpAddress &IP, bool PrependLength, bool PrependType)
{
  unsigned char ucIP[SNMP_INETADDRESSLEN_IPV6]; // Size for IPv6
  int IpLen = SNMP_INETADDRESSLEN_IPV6;
  
  if (IP.IsIpV6Address() == false)
    IpLen = SNMP_INETADDRESSLEN_IPV4;
    
  IP.Get (ucIP, IpLen);
  Set (NULL, 0);
  
  // When an InetAddress object is used as a table index, generally
  // it's encoded with the type and length.  The only way it would not
  // be is if the InetAddress part of the index is IMPLIED.
  if (PrependType)
  {
    if (IP == kIPv6UnspecifiedAddress)
      *this += SNMP_INETADDRESSTYPE_UNKNOWN;
    else if (IP.IsIpV6Address())
      *this += SNMP_INETADDRESSTYPE_IPV6;
    else
      *this += SNMP_INETADDRESSTYPE_IPV4;
  }
  
  if (PrependLength)
  {
    if (IP == kIPv6UnspecifiedAddress)
      *this += (unsigned long) SNMP_INETADDRESSLEN_UNKNOWN;
    else if (IP.IsIpV6Address())
      *this += SNMP_INETADDRESSLEN_IPV6;
    else
      *this += SNMP_INETADDRESSLEN_IPV4;
  }
  
  // Is this the right way to encode an IPv6 address?  Probably so, but
  // we would generally have the InetAddressType as well before this, as
  // well as maybe the size.
  for (int Loop = 0; Loop < IpLen; Loop++)
    *this += ucIP[Loop];
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmObjectId::Set (const BcmOctetBuffer &Buffer)
{
  BcmString Buf;
  
  Buf.assign ((const char *)Buffer.AccessBuffer(), Buffer.BytesUsed());
  SetFromAscii (Buf);
  
  return SNMP_NO_ERROR;
}

BcmObjectId &BcmObjectId::operator = (const BcmObjectId &OID)
{
  fObjects = OID.fObjects;
  fIndexOffset = OID.fIndexOffset;
  UpdateOID();
  
  return *this;
}

BcmObjectId &BcmObjectId::operator = (const ObjectId &OID)
{
  Set (OID.pObjects, OID.Length);
  IndexOffset (OID.IndexOffset);
  UpdateOID();
  
  return *this;
}

BcmObjectId &BcmObjectId::operator = (const char *pString)
{
  Set (pString);
  return *this;
}

BcmObjectId &BcmObjectId::operator = (const BcmString &String)
{
  Set (String);
  return *this;
}

BcmObjectId &BcmObjectId::operator = (const BcmMacAddress &Mac)
{
  Set (Mac);
  return *this;
}

BcmObjectId &BcmObjectId::operator = (const BcmIpAddress &IP)
{
  Set (IP);
  return *this;
}

BcmObjectId &BcmObjectId::operator = (const BcmOctetBuffer &Buffer)
{
  Set (Buffer);
  return *this;
}

BcmObjectId &BcmObjectId::operator = (unsigned long SingleObject)
{
  Set (&SingleObject, 1);
  return *this;
}

BcmObjectId &BcmObjectId::operator += (const BcmObjectId &OID)
{
  int Loop;
  unsigned long Elem;
  
  for (Loop = 0; Loop < OID.Length(); Loop++)
  {
    Elem = OID.fObjects[Loop];
    fObjects.push_back(Elem);
  }
  
  UpdateOID();
  return *this;
}

BcmObjectId &BcmObjectId::operator += (unsigned long SingleObject)
{
  fObjects.push_back(SingleObject);
  
  UpdateOID();
  return *this;
}

BcmObjectId BcmObjectId::operator + (const BcmObjectId &OID) const
{
  BcmObjectId Sum (*this);
  Sum += OID;
  
  return Sum;
}

// If we were constructed with an ObjectId pointer, update the contents
// of that structure now.  This allows the caller to use the BcmObjectId
// class to more easily modify the contents of an ObjectId structure.
void BcmObjectId::UpdateOID ()
{
  int Loop;


  if (fpOID == NULL)
    return;
    
  fpOID->Length = Length();
  fpOID->IndexOffset = IndexOffset();
  
  for (Loop = 0; Loop < fpOID->Length; Loop++)
    fpOID->pObjects[Loop] = fObjects[Loop];
    
  fpOID->Leaf = Leaf();
}

// RETURNS: positive number if this > OID, negative number if this < OID, 0 if
// equal.  This function is used by the <, <=, ==, >=, > operators, below.
int BcmObjectId::Compare (const BcmObjectId &OID) const
{
  int Loop, Shortest;
  
  // Otherwise, apply the usual rules for OID ordering.
  if (Length() >= OID.Length())
    Shortest = OID.Length();
  else
    Shortest = Length();
    
  // Compare as many elements as the OID's have in common.  The first
  // element that's different between the two will tell us which is larger.
  for (Loop = 0; Loop < Shortest; Loop++)
  {
      if (fObjects[Loop] != OID.fObjects[Loop])
      {
          if (fObjects[Loop] < OID.fObjects[Loop])
            return -1;
          
          if (fObjects[Loop] > OID.fObjects[Loop])
            return 1;
      }
  }
  
  // OK, we made it this far and the two OID's have all their common elements
  // the same.  That means that the longer OID is a descendant of the shorter,
  // so the longer OID is lexigraphically larger than the shorter one...
  return (Length() - OID.Length());
}

bool BcmObjectId::operator == (ObjectId &OID)
{
  BcmObjectId MibOid(&OID);
  return (MibOid == *this);
}

bool BcmObjectId::operator != (ObjectId &OID)
{
  BcmObjectId MibOid(&OID);
  return (MibOid != *this);
}

bool BcmObjectId::operator < (ObjectId &OID)
{
  BcmObjectId MibOid(&OID);
  return (MibOid < *this);
}

bool BcmObjectId::operator <= (ObjectId &OID) 
{
  BcmObjectId MibOid(&OID);
  return (MibOid <= *this);
}

bool BcmObjectId::operator > (ObjectId &OID) 
{
  BcmObjectId MibOid(&OID);
  return (MibOid > *this);
}

bool BcmObjectId::operator >= (ObjectId &OID) 
{
  BcmObjectId MibOid(&OID);
  return (MibOid >= *this);
}

unsigned long &BcmObjectId::operator [] (int Index)
{
  return fObjects[Index];
}

const unsigned long &BcmObjectId::operator [] (int Index) const
{
  return fObjects[Index];
}

unsigned long BcmObjectId::Element (int Index) const
{
  return fObjects[Index];
}

// 1.3.6 is ascendant of 1.3.6.1
bool BcmObjectId::IsAscendant (const BcmObjectId &OID) const
{
  int Loop;


  if (Length() >= OID.Length())
    return false;
    
  // this object's length is less than OID's length.
  // For this to be an ascendant of OID, all of this's
  // objects must match OID's up to as long as this is.
  for (Loop = 0; Loop < Length(); Loop++)
  {
    if (fObjects[Loop] != OID.fObjects[Loop])
      return false;
  }
      
  return true;
}

// 1.3.6.1 is descendant of 1.3.6
bool BcmObjectId::IsDescendant (const BcmObjectId &OID) const
{
  int Loop;


  if (Length() <= OID.Length())
    return false;
    
  // this object's length is greater than OID's length.
  // For this to be a descendant of OID, all of OID's
  // objects must match this's up to as long as OID is.
  for (Loop = 0; Loop < OID.Length(); Loop++)
  {
    if (fObjects[Loop] != OID.fObjects[Loop])
      return false;
  }
      
  return true;
}

// 1.3.6.2 is sibling of 1.3.6.1
bool BcmObjectId::IsSibling (const BcmObjectId &OID, int UpToLen) const
{
  int Loop;


  if (Length() == OID.Length())
    return false;
    
  if (UpToLen >= Length())
    return false;
    
  // this object's length is the same as OID's length.
  // For this to be a sibling of OID, all of OID's
  // objects must match this's except for the index.  Just
  // from the OID, we of course can't determine what the 
  // index is!  That's why the UpTo argument.
  for (Loop = 0; Loop < UpToLen; Loop++)
  {
    if (fObjects[Loop] != OID.fObjects[Loop])
      return false;
  }
      
  return true;
}

void BcmObjectId::AsString (BcmString &String, char IndexIndicator, ParseMode Parse) const
{
  int Loop, Len = Length()-1;
  char Seg[16];

  String = "";
  
  for (Loop = 0; Loop < Len; Loop++)
  {
    // If this is an ASCII mode conversion, then just keep slapping
    // our objects onto the end of the string.
    if (Parse == kAscii)
    {
      unsigned char cObject = fObjects[Loop] & 0xFF;
      String += cObject;
      continue;
    }
    
    // NEW: indicate location of index with the IndexIndicator instead of a '.'
    // where it begins
    if (Loop == (IndexOffset()-1))
      sprintf (Seg, "%lu%c", fObjects[Loop], IndexIndicator);
    else
      sprintf (Seg, "%lu.", fObjects[Loop]);
    String += Seg;
  }
  
  if (Len >= 0)
  {
    if (Parse == kAscii)
      String += (char)(fObjects[Loop] & 0xFF);
    else
    {
      sprintf (Seg, "%lu", fObjects[Loop]);
      String += Seg;
    }
  }
  
  // If the index offset happens to be equal to our length, print an 
  // IndexIndicator at the end, unless it's the default of '.'
  if ((IndexOffset() == Length())
  &&  (IndexIndicator != '.'))
    String += IndexIndicator;
}

ObjectId *BcmObjectId::GetOID(ObjectId *pOid) const
{
  int Loop;
  

  if (pOid)
  {
    for (Loop = 0; Loop < Length(); Loop++)
      pOid->pObjects[Loop] = fObjects[Loop];
      
    pOid->Length = Length();  
    pOid->IndexOffset = IndexOffset();
    pOid->Leaf = Leaf();
  }
  
  return fpOID;
}

ostream &BcmObjectId::Print(ostream &outStream) const
{
  BcmString String;
  
  // When printing, indicate index as a colon.
  AsString(String, ':');
  outStream << String;

  return outStream;
}



