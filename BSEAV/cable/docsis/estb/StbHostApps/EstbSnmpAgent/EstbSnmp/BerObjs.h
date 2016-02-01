//**************************************************************************
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
//    Filename: BerObjs.h
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

#ifndef BEROBJS_H
#define BEROBJS_H

#include "CoreObjs.h"
#include "Counter64.h"

//#include <iostream.h>

/*\
 *  Note that ASN.1 BER encoding is just TLV encoding with some special rules.
 *  I suggest chapter 6 of "The Simple Book" by Marshall T. Rose 
 *  (Prentice Hall, 1994) for the details.  Or, if you want ALL the details, 
 *  check out "Abstract Syntax Notation One (ASN.1): The Tutorial and Reference"
 *  by Douglas Steedman (Technology Appraisals Ltd, 1990).
 *
 *  Function returns 1 if it succeeds, 0 if it fails.
\*/

// Type ("Tag" actually) values for BER encoding are a single byte in size
// and look like this:
//
// +--bit number (7=MSB, 0=LSB)-+
// |  7  6 | 5 | 4  3  2  1  0  |
// +-------+---+----------------+
// | class | f |     number     |
// +-------+---+----------------+

#define BER_CLASS_UNIVERSAL         0x00
#define BER_CLASS_APPLICATION_WIDE  0x40
#define BER_CLASS_CONTEXT_SPECIFIC  0x80
#define BER_CLASS_PRIVATE           0xC0

#define BER_F_PRIMITIVE             0x00
#define BER_F_CONSTRUCTED           0x20

#define BER_TLV_BOOLEAN             1
#define BER_TLV_INTEGER             2
#define BER_TLV_BIT_STRING          3
#define BER_TLV_OCTET_STRING        4
#define BER_TLV_NULL                5
#define BER_TLV_OBJECT_IDENTIFIER   6
#define BER_TLV_OBJECT_DESCRIPTOR   7  // Not
#define BER_TLV_EXTERNAL            8  // Gonna
#define BER_TLV_REAL                9  // Use
#define BER_TLV_ENUMERATED          10 // These!
#define BER_TLV_SEQUENCE            16

// PR 11894:  Support the noSuchObject and noSuchInstance exception types
// For noSuchObject and noSuchInstance exceptions under SNMPv2, error status
// is NO_SUCH_ERROR but the value of each excepted varbind is reported with
// tag = 0x80 or 0x81, length = 0
#define BER_TLV_NOSUCHOBJECT        ( BER_CLASS_CONTEXT_SPECIFIC | BER_F_PRIMITIVE | 0x00 ) // 0x80 = 128
#define BER_TLV_NOSUCHINSTANCE      ( BER_CLASS_CONTEXT_SPECIFIC | BER_F_PRIMITIVE | 0x01 ) // 0x80 = 129

#define BER_EXTEND_LENGTH           0x80


class AsnBerTuple 
{
  public:
    AsnBerTuple (const unsigned char *pBerBuf = NULL, int Length = -1);
    AsnBerTuple (int Tag, int Length);
    virtual ~AsnBerTuple ();
   
    SNMP_STATUS SetBuffer (const unsigned char *pBerBuf, int Length = -1);
   
    virtual SNMP_STATUS Decode ();
    virtual SNMP_STATUS Encode (BcmString &Buffer) const;
    
    // For backwards compatibility...
    inline SNMP_STATUS Parse () { return Decode (); }
    
    virtual ostream &Print(ostream &outStream) const;
    
    unsigned char Tag () const;
    unsigned int Length () const; // length of the VALUE part only
    virtual unsigned int TotalLength () const; // length of the whole shootin' match
    
    //AsnBerTuple &operator = (AsnBerTuple &Tuple);
    //AsnBerTuple &operator = (unsigned char *pBerBuf);
    
    const unsigned char *ValueBuf () const;
    const unsigned char *BerBuf () const;

  protected: 
    int fSpecifiedLength;
    unsigned char fTag;
    unsigned int fLength;
    const unsigned char *fpBerBuf, *fpValueBuf;
    bool fMyBuffer;
};

class AsnBerLength : public AsnBerTuple
{
  public:
    AsnBerLength (const unsigned char *pBerBuf = NULL, int Length = -1);
    AsnBerLength (int Length);
    virtual ~AsnBerLength ();
    
    SNMP_STATUS Decode ();   
    SNMP_STATUS Encode (BcmString &Buffer) const;
    
    unsigned int TotalLength () const;

    ostream &Print(ostream &outStream) const;
};

class AsnBerInt32 : public AsnBerTuple
{
  public:
    AsnBerInt32 (const unsigned char *pBerBuf = NULL, int Length = -1);
    AsnBerInt32 (int Value);
    
    virtual ~AsnBerInt32 ();
    
    SNMP_STATUS Decode ();   
    SNMP_STATUS Encode (BcmString &Buffer) const;

    ostream &Print(ostream &outStream) const;
    SNMP_STATUS GetValue (int &Val) const; 
   
  protected:
    int fValue;
};

class AsnBerUint32 : public AsnBerTuple
{
  public:
    AsnBerUint32 (const unsigned char *pBerBuf = NULL, int Length = -1);
    AsnBerUint32 (unsigned int Value, int Type = VBT_UNSIGNED32);
    
    virtual ~AsnBerUint32 ();

    SNMP_STATUS Decode ();
    SNMP_STATUS Encode (BcmString &Buffer) const;

    ostream &Print(ostream &outStream) const;    
    SNMP_STATUS GetValue (unsigned int &Val) const; 
   
  protected:
    unsigned int fValue;
};

class AsnBerUint64 : public AsnBerTuple
{
  public:
    AsnBerUint64 (const unsigned char *pBerBuf = NULL, int Length = -1);
    AsnBerUint64 (const BcmCounter64 &Value);
    
    virtual ~AsnBerUint64 ();

    SNMP_STATUS Decode ();
    SNMP_STATUS Encode (BcmString &Buffer) const;

    ostream &Print(ostream &outStream) const;    
    SNMP_STATUS GetValue (BcmCounter64 &Val) const; 
   
  protected:
    BcmCounter64 fValue;
};

class AsnBerBitString : public AsnBerTuple
{
  public:
    AsnBerBitString (const unsigned char *pBerBuf = NULL, int Length = -1);
    AsnBerBitString (const BcmString &Value, int UnusedBits);
    
    virtual ~AsnBerBitString ();
   
    SNMP_STATUS Decode ();
    SNMP_STATUS Encode (BcmString &Buffer) const;

    ostream &Print(ostream &outStream) const;    
    SNMP_STATUS GetValue (BcmString &Val, int &UnusedBits) const;
   
  protected:
    BcmString fValue;
    int fUnusedBits;
};

class AsnBerOctetString : public AsnBerTuple
{
  public:
    AsnBerOctetString (const unsigned char *pBerBuf = NULL, int Length = -1);
    AsnBerOctetString (const BcmString &Value);
    
    virtual ~AsnBerOctetString ();
    
    SNMP_STATUS Decode ();
    SNMP_STATUS Encode (BcmString &Buffer) const;
    
    ostream &Print(ostream &outStream) const;
    SNMP_STATUS GetValue (BcmString &Val) const;
   
  protected:
    BcmString fValue;
};

class AsnBerIpAddress : public AsnBerOctetString
{
  public:
    AsnBerIpAddress (const unsigned char *pBerBuf = NULL, int Length = -1);
    AsnBerIpAddress (const BcmIpAddress &Value);
    
    virtual ~AsnBerIpAddress ();
    
    SNMP_STATUS Decode ();
    
    ostream &Print(ostream &outStream) const;
    SNMP_STATUS GetValue (BcmIpAddress &Val) const;
    
  protected:
    BcmIpAddress fIpValue;
};

class AsnBerOid : public AsnBerTuple
{
  public:
    AsnBerOid (const unsigned char *pBerBuf = NULL, int Length = -1);
    AsnBerOid (const BcmObjectId &Value);
    
    virtual ~AsnBerOid ();
   
    SNMP_STATUS Decode ();
    SNMP_STATUS Encode (BcmString &Buffer) const;
    
    ostream &Print(ostream &outStream) const;
    SNMP_STATUS GetValue (BcmObjectId &Val) const;
    
    bool NeedProprietaryEncoding (const BcmObjectId &OID) const;
   
  protected:
    BcmObjectId fValue;
};

class AsnBerNull : public AsnBerTuple
{
  public:
    AsnBerNull (const unsigned char *pBerBuf = NULL, int Length = -1);
    
    virtual ~AsnBerNull ();
   
    SNMP_STATUS Decode ();
    SNMP_STATUS Encode (BcmString &Buffer) const;
    
    ostream &Print(ostream &outStream) const;
    SNMP_STATUS GetValue () const;
};

class AsnBerVarBind
{
  public:
    AsnBerVarBind (const unsigned char *pBerBuf = NULL, int Length = -1);
    AsnBerVarBind (const BcmObjectId &Oid, const BcmVarBind &Vb);
   ~AsnBerVarBind ();
   
    SNMP_STATUS GetOid (BcmObjectId &Oid) const;
    SNMP_STATUS GetValue (BcmVarBind &Vb) const;
    
    unsigned int Length() const;
    
    SNMP_STATUS SetBuffer (const unsigned char *pBerBuf, int Length = -1);
    
    SNMP_STATUS Decode ();
    SNMP_STATUS Encode (BcmString &Buffer) const;
    
    ostream &Print(ostream &outStream) const;
    
  protected:
    int fSpecifiedLength;
    unsigned int fLength;
    const unsigned char *fpBerBuf, *fpValueBuf;
    AsnBerTuple fVbTuple;
    AsnBerTuple fValueTuple;
    AsnBerOid fOidTuple;
    
    // For encoding
    BcmObjectId fOid;
    BcmVarBind fVb;
    BcmString  fBuffer;
};

inline ostream & operator << (ostream &outStream, const AsnBerTuple &Tuple)
{
    return Tuple.Print(outStream);
}

inline ostream & operator << (ostream &outStream, const AsnBerInt32 &Tuple)
{
    return Tuple.Print(outStream);
}

inline ostream & operator << (ostream &outStream, const AsnBerUint32 &Tuple)
{
    return Tuple.Print(outStream);
}

inline ostream & operator << (ostream &outStream, const AsnBerBitString &Tuple)
{
    return Tuple.Print(outStream);
}

inline ostream & operator << (ostream &outStream, const AsnBerOctetString &Tuple)
{
    return Tuple.Print(outStream);
}

inline ostream & operator << (ostream &outStream, const AsnBerIpAddress &Tuple)
{
    return Tuple.Print(outStream);
}

inline ostream & operator << (ostream &outStream, const AsnBerOid &Tuple)
{
    return Tuple.Print(outStream);
}

inline ostream & operator << (ostream &outStream, const AsnBerNull &Tuple)
{
    return Tuple.Print(outStream);
}

inline ostream & operator << (ostream &outStream, const AsnBerVarBind &Tuple)
{
    return Tuple.Print(outStream);
}

#endif
