/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
//
//**************************************************************************
//    Filename: ObjectIdentifier.h
//    Author:   Kevin O'Neal
//    Creation Date: 18-September-2003
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    OID object code put into its own module
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************


#ifndef OBJECTIDENTIFIER_H
#define OBJECTIDENTIFIER_H

#if ((__GNUC__ >= 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <iostream>
#else
#include <iostream.h>
#endif

#include "BcmString.h"
#include <vector>

typedef vector <unsigned long> UlongVector;

#include "IpAddress.h"
#include "MacAddress.h"
#include "OctetBuffer.h"
#include "SnmpType.h"


#define UNUSED_OID_ARG  0xFFFFFFFF

class BcmObjectId
{
  public:
    typedef enum
    {
      kDottedDecimal = 0,
      kAscii
    } ParseMode;
    
    BcmObjectId ();
    BcmObjectId (unsigned long Obj0, 
                 unsigned long Obj1=UNUSED_OID_ARG, 
                 unsigned long Obj2=UNUSED_OID_ARG, 
                 unsigned long Obj3=UNUSED_OID_ARG, 
                 unsigned long Obj4=UNUSED_OID_ARG, 
                 unsigned long Obj5=UNUSED_OID_ARG, 
                 unsigned long Obj6=UNUSED_OID_ARG, 
                 unsigned long Obj7=UNUSED_OID_ARG);
    BcmObjectId (const BcmObjectId &OID);
    BcmObjectId (ObjectId *pOID);
    BcmObjectId (int Length, const unsigned long *pObjects);
    BcmObjectId (const char *pString, ParseMode Parse = kDottedDecimal);
    BcmObjectId (const BcmString &String, ParseMode Parse = kDottedDecimal);
    BcmObjectId (const BcmMacAddress &Mac);
    BcmObjectId (const BcmIpAddress &IP, bool PrependLength = false, bool PrependType = false);
    BcmObjectId (const BcmOctetBuffer &Buffer);
   ~BcmObjectId ();
   
    inline int Length () const;
    
    // New methods to ease index extraction.  If we don't know where the
    // index begins or if the object has no 'index' per-se, then these return
    // -1.  The index is zero-based and points to the first element of the
    // index.  So for example, sysDescr = 1.3.6.1.2.1.1.1.0, IndexOffset is
    // 8 (the .0)
    inline int IndexOffset () const;
    int IndexOffset (int Offset);
    
    unsigned long Leaf () const;
    
    SNMP_STATUS GetIndex (BcmObjectId &Index) const;
   
    SNMP_STATUS Set (const unsigned long *pObjects, int Length);
    SNMP_STATUS Set (const char *pString);
    SNMP_STATUS Set (const BcmString &String);
    SNMP_STATUS Set (const BcmMacAddress &Mac);
    SNMP_STATUS Set (const BcmIpAddress &IP, bool PrependLength = false, bool PrependType = false);
    SNMP_STATUS Set (const BcmOctetBuffer &Buffer);
    SNMP_STATUS SetFromAscii (const BcmString &String);
    
    BcmObjectId &operator = (const BcmObjectId &OID);
    BcmObjectId &operator = (const ObjectId &OID);
    BcmObjectId &operator = (const BcmString &String);
    BcmObjectId &operator = (const BcmMacAddress &Mac);
    BcmObjectId &operator = (const BcmIpAddress &IP);
    BcmObjectId &operator = (const BcmOctetBuffer &Buffer);
    BcmObjectId &operator = (const char *pString);
    BcmObjectId &operator = (unsigned long SingleObject);
    
    BcmObjectId &operator += (const BcmObjectId &OID);
    BcmObjectId &operator += (unsigned long SingleObject);
    
    BcmObjectId operator + (const BcmObjectId &OID) const;
   
    // Returns 0 if this is equal to OID, negative number if this is less than
    // OID, or positive number if this is greater than OID.  Note that it can
    // return any positive or negative number!
    int Compare (const BcmObjectId &OID) const;
   
    inline bool operator == (const BcmObjectId &OID) const;
    inline bool operator != (const BcmObjectId &OID) const;
    inline bool operator <  (const BcmObjectId &OID) const;
    inline bool operator <= (const BcmObjectId &OID) const;
    inline bool operator >  (const BcmObjectId &OID) const;
    inline bool operator >= (const BcmObjectId &OID) const;
    
    // These can't be const methods because internally they construct
    // a new BcmObjectId object and pass the address of the OID reference
    // in.  This type of construction just stores the pointer and updates
    // it if the value changes, so that makes it non-constable.
    bool operator == (ObjectId &OID);
    bool operator != (ObjectId &OID);
    bool operator <  (ObjectId &OID);
    bool operator <= (ObjectId &OID);
    bool operator >  (ObjectId &OID);
    bool operator >= (ObjectId &OID);
    
    unsigned long &operator [] (int Index);
    const unsigned long &operator [] (int Index) const;
    unsigned long Element (int Index) const;
    
    bool IsAscendant (const BcmObjectId &OID) const;  // 1.3.6 is ascendant of 1.3.6.1
    bool IsDescendant (const BcmObjectId &OID) const; // 1.3.6.1 is descendant of 1.3.6
    bool IsSibling (const BcmObjectId &OID, int UpToLen) const;    // 1.3.6.2 is sibling of 1.3.6.1
    
    void AsString (BcmString &String, char IndexIndicator = '.', ParseMode Parse = kDottedDecimal) const;
    ObjectId *GetOID(ObjectId *pOid = NULL) const;
    
    ostream &Print(ostream &outStream) const;
    
  protected:
    UlongVector fObjects;
    int fIndexOffset;
    ObjectId *fpOID;
    
    void UpdateOID ();
};

inline int BcmObjectId::Length() const
{
  return fObjects.size();
}

inline int BcmObjectId::IndexOffset() const
{
  return fIndexOffset;
}

// RETURNS: 1 if OID == this, 0 otherwise
inline bool BcmObjectId::operator == (const BcmObjectId &OID) const
{
  if (Compare (OID) == 0)
    return true;
  return false;
}

inline bool BcmObjectId::operator != (const BcmObjectId &OID) const
{
  if (Compare (OID) == 0)
    return false;
  return true;
}

inline bool BcmObjectId::operator < (const BcmObjectId &OID) const
{
  if (Compare (OID) < 0)
    return true;
  return false;
}

inline bool BcmObjectId::operator <= (const BcmObjectId &OID) const
{
  if (Compare (OID) <= 0)
    return true;
  return false;
}

inline bool BcmObjectId::operator > (const BcmObjectId &OID) const
{
  if (Compare (OID) > 0)
    return true;
  return false;
}

inline bool BcmObjectId::operator >= (const BcmObjectId &OID) const
{
  if (Compare (OID) <= 0)
    return true;
  return false;
}

inline ostream & operator << (ostream &outStream, const BcmObjectId &OID)
{
    return OID.Print(outStream);
}

#endif

