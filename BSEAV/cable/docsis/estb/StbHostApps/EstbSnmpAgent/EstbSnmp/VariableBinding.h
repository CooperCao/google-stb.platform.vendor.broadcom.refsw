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
//    Filename: VariableBinding.h
//    Author:   Kevin O'Neal
//    Creation Date: 18-September-2003
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    BcmVarBind object moved to its own module.
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************


#ifndef VARIABLEBINDING_H
#define VARIABLEBINDING_H

#include <time.h>

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <iostream>
#else
#include <iostream.h>
#endif

#include "BcmString.h"
#include "IpAddress.h"
#include "MacAddress.h"
#include "OctetBuffer.h"


// BcmCounter64 has been moved to a common file.
#include "Counter64.h"

// So has BcmObjectId
#include "ObjectIdentifier.h"


// time_t is a signed 32 bit value.  The DateAndTime textual convention
// (see RFC-2579) does allow a value of all zeros, so we'll pick a valid
// but unlikely-to-be-used time_t of the largest (smallest?) possible negative
// value to specify this case.                       
typedef enum
{
  kAllZerosDateAndTime = 0x80000001
} DateAndTimeTypes;

class BcmVarBind
{
  public:
    BcmVarBind ();
    BcmVarBind (VarBind *pVB);
    BcmVarBind (const BcmVarBind &VB);
   ~BcmVarBind ();
   
    typedef enum
    {
      kUnspecified = 0,
      kDateAndTime,
      kMacAddress,
      kTruthValue,
      kRowStatus
    } VbTextualConvention;
    
    void Reset ();
   
    ostream &Print(ostream &outStream) const;
    void AsString (BcmString &String, bool Verbose = true) const;
    
    int GetType () const;
    int GetTextualConvention () const;
    SNMP_STATUS SetTextualConvention (VbTextualConvention TC);
   
    BcmVarBind &operator = (const BcmVarBind &VB);
    BcmVarBind &operator = (VarBind &VB);
    BcmVarBind &operator = (const char *pValue);
    BcmVarBind &operator = (const BcmString &Value);
    BcmVarBind &operator = (int Value);
    BcmVarBind &operator = (unsigned int Value);
    BcmVarBind &operator = (const BcmObjectId &Value);
    BcmVarBind &operator = (const BcmIpAddress &Value);
    BcmVarBind &operator = (const BcmMacAddress &Value);
    
    bool operator == (const BcmVarBind &VB) const;
    bool operator == (int Value) const;
    bool operator == (unsigned int Value) const;
    bool operator == (const BcmString &Value) const;
    bool operator == (const char *pValue) const;
    bool operator == (const BcmObjectId &Value) const;
    bool operator == (const BcmIpAddress &Value) const;
    bool operator == (const BcmMacAddress &Value) const;
    
    bool operator != (const BcmVarBind &VB) const;
    bool operator != (int Value) const;
    bool operator != (unsigned int Value) const;
    bool operator != (const BcmString &Value) const;
    bool operator != (const char *pValue) const;
    bool operator != (const BcmObjectId &Value) const;
    bool operator != (const BcmIpAddress &Value) const;
    bool operator != (const BcmMacAddress &Value) const;
    
    SNMP_STATUS SetEmpty ();
    SNMP_STATUS SetEmpty (int type);

    SNMP_STATUS SetVarBind (const BcmVarBind &VB);
    SNMP_STATUS SetVarBind (VarBind *pVB);    
    SNMP_STATUS SetString (const BcmString &String);
    SNMP_STATUS SetString (const unsigned char *pString, int Len = -1);
    SNMP_STATUS SetString (const char *pString, int Len = -1);
    SNMP_STATUS SetInt32 (int Value);
    SNMP_STATUS SetBool (bool BoolVal);
    SNMP_STATUS SetUint32 (unsigned int Value, int Type = VBT_UNSIGNED32);
    SNMP_STATUS SetUint64 (const BcmCounter64 &C64);
    SNMP_STATUS SetOid (const ObjectId *pOID);
    SNMP_STATUS SetOid (const BcmObjectId &OID);
    SNMP_STATUS SetIp (const unsigned char *pIp);
    SNMP_STATUS SetIp (const BcmIpAddress &Ip);
    SNMP_STATUS SetMacAddr (const BcmMacAddress &Mac);
    SNMP_STATUS SetTimeticks (unsigned long Ticks);
    SNMP_STATUS SetDateAndTime (time_t EpochTime, bool IncludeTimezone = false, int UtcOffsetSeconds = 0);
    SNMP_STATUS SetDateAndTime (unsigned int Month, unsigned int Day, unsigned int Year, unsigned int Hour, unsigned int Minute, unsigned int Second);
    SNMP_STATUS SetDateAndTime (unsigned int Month, unsigned int Day, unsigned int Year, unsigned int Hour, unsigned int Minute, unsigned int Second, char DirFromUTC, unsigned int HourFromUTC, unsigned int MinFromUTC);
    SNMP_STATUS SetBits (unsigned long NormalBitmask, int NumOctets);
    
    // PR 11894:  Support the noSuchObject and noSuchInstance exception types
    SNMP_STATUS SetNoSuchObject ();
    SNMP_STATUS SetNoSuchInstance ();
    
    SNMP_STATUS SetPrivate (int Index, void *pPrivate);
    SNMP_STATUS SetMisc (void *pMisc);
    
    SNMP_STATUS GetVarBind (VarBind *pVB) const;
    SNMP_STATUS GetString (BcmString &String) const;
    SNMP_STATUS GetString (unsigned char *pString, int MaxLen) const;
    SNMP_STATUS GetString (char *pString, int MaxLen) const;
    SNMP_STATUS GetInt32 (int *pInt32) const;
    SNMP_STATUS GetBool (bool *pBoolVal) const;
    SNMP_STATUS GetUint32 (unsigned int *pUint32, int *pType = NULL) const;
    SNMP_STATUS GetUint64 (BcmCounter64 &C64) const;
    SNMP_STATUS GetOid (BcmObjectId &OID) const;
    SNMP_STATUS GetIp (unsigned char *pIp) const;
    SNMP_STATUS GetIp (BcmIpAddress &Ip) const;
    SNMP_STATUS GetMacAddr (BcmMacAddress &Mac) const;
    SNMP_STATUS GetTimeticks (unsigned long *Ticks) const;
    SNMP_STATUS GetDateAndTime (time_t *EpochTime) const;
    SNMP_STATUS GetBits (unsigned long *pNormalBitmask, int *pNumBytes = NULL) const;

    void *GetPrivate (int Index) const;
    void *GetMisc () const;
    
  protected:
    // What am I, anyway?
    int fType;
    
    // Native SNMP types:
    long fNumber32;
    unsigned long fUnsigned32;
    BcmIpAddress fIpAddr;
    BcmString fString;
    BcmObjectId fOID;
    BcmCounter64 fC64;

    // Special properties:    
    VbTextualConvention fTextualConvention;
    
    // If someone passes us in one of these structures
    VarBind *fpVB;
    bool fMyString;
};

inline ostream & operator << (ostream &outStream, const BcmVarBind &VB)
{
    return VB.Print(outStream);
}

#endif

