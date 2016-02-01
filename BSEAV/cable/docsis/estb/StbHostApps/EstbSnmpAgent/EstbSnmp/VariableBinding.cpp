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
//    Filename: VariableBinding.cpp
//    Author:   Kevin O'Neal
//    Creation Date: 18-September-2003
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    BcmVarBind moved to its own module.
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include <time.h>
#include "VariableBinding.h"
#include "SnmpLog.h"


/*\
 *  BcmVarBind methods
\*/

BcmVarBind::BcmVarBind ()
{
  fpVB = NULL;
  fMyString = false;
  
  Reset();
}

BcmVarBind::BcmVarBind (VarBind *pVB)
{
  // Adopt the VarBind that was passed in.  This is consistent
  // with the old RAM-hungry approach.
  fpVB = pVB;
  fMyString = false;
  
  Reset();
  SetVarBind (pVB);
}

BcmVarBind::BcmVarBind (const BcmVarBind &VB)
{
  fpVB = NULL;
  fMyString = false;
  
  Reset();
  SetVarBind (VB);
}

BcmVarBind::~BcmVarBind ()
{
  // If we have allocated storage for a string, delete it.
  if (fpVB)
  {
    if ((fpVB->Value.String.pOctets)
    &&  (fMyString == true))
    {
      // DEBUG: after deleting the string storage, seed with a known string
      // so we can make sure this is not going out of scope.  That way, if
      // we see the special string below in a MIB object we will know something
      // is amiss.  Otherwise we might just get lucky and have this area of
      // memory not overwritten, thus the error will not be obvious.
      delete [] fpVB->Value.String.pOctets;
      
      fpVB->Value.String.Length = strlen ("WARNING! DELETED VARBIND STRING");
      fpVB->Value.String.pOctets = (unsigned char *)"WARNING! DELETED VARBIND STRING";
    }
    
    // If we have a pPrivateFree function pointer, apply it to all
    // private objects.
    if (fpVB->pPrivateFree)
    {
      for (int Loop=0; Loop < VB_PRIVATES; Loop++)
        fpVB->pPrivateFree(fpVB->pPrivate[Loop]);
    }
  }
}

void BcmVarBind::Reset ()
{
  fType = 0;
  fNumber32 = 0;
  fUnsigned32 = 0;
  fString = "";
  fOID = "";
  fC64 = 0;
  fTextualConvention = kUnspecified;
  
  // Don't clear fpVB here because it gets updated in all the
  // various Set methods, which in turn call this method.
}

ostream &BcmVarBind::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  //outStream << "Variable Binding at " << fpVB << ", this=" << this << endl;

  switch (fType)
  {
    case VBT_NUMBER:
    {
      outStream << "NUMBER: " << fNumber32 << endl;
      
      if (fTextualConvention == kTruthValue)
      {
        switch (fNumber32)
        {
          case SNMP_TRUTH_TRUE:
            outStream << "  TruthValue: TRUE" << endl;
            break;
            
          case SNMP_TRUTH_FALSE:
            outStream << "  TruthValue: FALSE" << endl;
            break;
            
          default:
            outStream << "  TruthValue: undefined" << endl;
        }
      }
      else if (fTextualConvention == kRowStatus)
      {
        switch (fNumber32)
        {
          case SNMP_STATUS_INVISIBLE:
            outStream << "  RowStatus: INVISIBLE" << endl;
            break;
            
          case SNMP_STATUS_ACTIVE:
            outStream << "  RowStatus: ACTIVE" << endl;
            break;
            
          case SNMP_STATUS_NOT_IN_SERVICE:
            outStream << "  RowStatus: NOT IN SERVICE" << endl;
            break;
            
          case SNMP_STATUS_NOT_READY:
            outStream << "  RowStatus: NOT READY" << endl;
            break;
            
          case SNMP_STATUS_CREATE_AND_WAIT:
            outStream << "  RowStatus: CREATE AND WAIT" << endl;
            break;
            
          case SNMP_STATUS_CREATE_AND_GO:
            outStream << "  RowStatus: CREATE AND GO" << endl;
            break;
            
          case SNMP_STATUS_DESTROY:
            outStream << "  RowStatus: DESTROY" << endl;
            break;
            
          default:
            outStream << "  RowStatus: undefined" << endl;
        }
      }
      break;
    }
      
    case VBT_STRING:
    {
      bool AsciiPrintable = true;
      BcmOctetBuffer RawBuf;

      
      fString.Get(RawBuf);
      
      if (fString.length() == 0)
      {
        outStream << "STRING, 0 bytes (empty)" << endl;
        AsciiPrintable = false;
      }
      else
        outStream << "STRING, " << fString.length() << " bytes:  " << RawBuf;
        
      if (fTextualConvention == kDateAndTime)
      {
        char DateTimeStr[32];
        unsigned int Year = (((unsigned char)fString[0]) << 8) | (((unsigned char)fString[1]));
        
        sprintf (DateTimeStr, "%d-%d-%d, %02d:%02d:%02d.%d",
          Year, fString[2],fString[3],fString[4],
          fString[5],fString[6],fString[7]);
          
        if (fString.length() == 11)
        {
           sprintf (DateTimeStr, "%d-%d-%d, %02d:%02d:%02d.%d,%c%d:%d",
             Year, fString[2],fString[3],fString[4],
             fString[5],fString[6],fString[7],fString[8],fString[9],fString[10]);
        }

        outStream << "  DateAndTime: " << DateTimeStr << endl;
      }
      
      break;
    }
      
    case VBT_OBJECT:
    {
      outStream << "OBJECT: " << fOID << endl;
      break;
    }
      
    case VBT_EMPTY:
      outStream << "EMPTY" << endl;
      break;
      
    case VBT_IPADDRESS:
      outStream << "IPADDRESS: " << fIpAddr << endl;
      break;
      
    case VBT_COUNTER:
      outStream << "COUNTER: " << fUnsigned32 << endl; 
      break;
      
    case VBT_GAUGE:
      outStream << "GAUGE: " << fUnsigned32 << endl;
      break;
      
    case VBT_TIMETICKS:
    {
      unsigned long Tix = fUnsigned32;
      unsigned long TotalSecs = Tix / 100;
      int Hours, Minutes, Seconds, Days;
      char TimeStr[32];
      
      Days    =  TotalSecs / (60*60*24);
      Hours   = (TotalSecs % (60*60*24)) / (60*60);
      Minutes = (TotalSecs % (60*60)) / 60;
      Seconds =  TotalSecs % 60;
      
      sprintf (TimeStr, " (%d days, %02d:%02d:%02d)", Days, Hours, Minutes, Seconds);
      outStream << "TIMETICKS: " << Tix << TimeStr << endl;
      break;
    }
      
    case VBT_OPAQUE:
      outStream << "OPAQUE" << endl;
      break;
      
    case VBT_COUNTER64:
    {
      outStream << "COUNTER64: " << fC64 << endl;
      break;
    }
      
    case VBT_UINTEGER32:
      outStream << "UINTEGER32: " << fUnsigned32 << endl;
      break;
      
    // PR 11894:  Support the noSuchObject and noSuchInstance exception types
    case VBT_NOSUCHOBJECT:
      outStream << "NO_SUCH_OBJECT" << endl;
      break;
      
    case VBT_NOSUCHINSTANCE:
      outStream << "NO_SUCH_INSTANCE" << endl;
      break;
    
    default:
      outStream << "UNKNOWN TYPE: " << fType << endl;
      break;
  }
#endif
  
  return outStream;
}

void BcmVarBind::AsString (BcmString &String, bool Verbose) const
{
  char chString[64];


  //outStream << "Variable Binding at " << fpVB << ", this=" << this << endl;

  String = "";

  switch (fType)
  {
    case VBT_NUMBER:
    {
      if (Verbose)
        sprintf (chString, "%ld (i32)", fNumber32);
      else
        sprintf (chString, "%ld", fNumber32);
      String = chString;
      break;
    }
    
    case VBT_STRING:
    {
      BcmOctetBuffer RawBuf;
      bool AsciiPrintable = true;
      unsigned int Loop;
      
      fString.Get(RawBuf);
      
      if (fString.length() == 0)
      {
        if (Verbose)
          String = "{empty string}";
        AsciiPrintable = false;
      }
        
      for (Loop = 0; Loop < fString.length(); Loop++)
      {
        // Don't just check for (byte & 0x80) here, otherwise we will still
        // print some non-printable characters in the < 0x20 range to the 
        // console.
        if (isprint (fString[Loop]) == 0) //(pString->pOctets[Loop] & 0x80)
        {
          AsciiPrintable = false;
          break;
        }
      }
      
      if (fTextualConvention == kDateAndTime)
      {
        char DateTimeStr[32];
        int Year = (fString[0] << 8) | (fString[1]);
        
        sprintf (DateTimeStr, "%d-%d-%d, %02d:%02d:%02d.%d",
          Year, fString[2],fString[3],fString[4],
          fString[5],fString[6],fString[7]);
          
        if (fString.length() == 11)
        {
           sprintf (DateTimeStr, "%d-%d-%d, %02d:%02d:%02d.%d,%c%d:%d",
             Year, fString[2],fString[3],fString[4],
             fString[5],fString[6],fString[7],fString[8],fString[9],fString[10]);
        }  
        String = DateTimeStr;
      }
      
      else if (AsciiPrintable)
        String = fString;
      else
      {
        char Octet[8];
        
        if (Verbose)
          String = "HEX:";
        
        for (Loop = 0; Loop < fString.length(); Loop++)
        {
          sprintf (Octet, "%02X ", (unsigned char)fString[Loop]);
          String += Octet;
        }
      }
      
      break;
    }
/*      
    case VBT_STRING:
    {
      pString = VARBIND_GET_STRING(fpVB);
      String = (char *)pString;
      break;
    }
*/      
    case VBT_OBJECT:
    {
      // Indicate index with a colon.  Usually this is not known
      // in an OID used as a value, but sometimes it might be...
      if (Verbose)
        fOID.AsString(String, ':');
      else
        fOID.AsString(String);
      break;
    }
      
    case VBT_EMPTY:
      String = "EMPTY";
      break;
      
    case VBT_IPADDRESS:
    {
      BcmIpAddress Ip;
      GetIp (Ip);
      Ip.Get(chString, sizeof( chString ));
      String = chString;
      break;
    }
      
    case VBT_COUNTER:
    case VBT_GAUGE:
    case VBT_UINTEGER32:
      if (Verbose)
        sprintf (chString, "%lu (u32)", fUnsigned32); 
      else
        sprintf (chString, "%lu (u32)", fUnsigned32); 
      String = chString;
      break;
      
    case VBT_TIMETICKS:
    {
      unsigned long Tix = fUnsigned32;
      unsigned long TotalSecs = Tix / 100;
      int Hours, Minutes, Seconds, Days;
      char TimeStr[32];
      
      Days    =  TotalSecs / (60*60*24);
      Hours   = (TotalSecs % (60*60*24)) / (60*60);
      Minutes = (TotalSecs % (60*60)) / 60;
      Seconds =  TotalSecs % 60;
      
      sprintf (TimeStr, "%d days, %02d:%02d:%02d", Days, Hours, Minutes, Seconds);
      String = TimeStr;
      break;
    }
      
    case VBT_OPAQUE:
      String = "OPAQUE";
      break;
      
    case VBT_COUNTER64:
    {
      sprintf (chString, "%08lX%08lX", fC64.High32(), fC64.Low32());
      String = chString;
      break;
    }
      
    default:
      sprintf (chString, "UNKNOWN TYPE: %d", fType);
      String = chString;
      break;
  }
}

int BcmVarBind::GetType () const
{
  return fType;
}

int BcmVarBind::GetTextualConvention () const
{
  return (int) fTextualConvention;
}

SNMP_STATUS BcmVarBind::SetTextualConvention (VbTextualConvention TC)
{
  fTextualConvention = TC;
  return SNMP_NO_ERROR;
}

BcmVarBind &BcmVarBind::operator = (const BcmVarBind &VB)
{
  SetVarBind (VB);
  return *this;
}

BcmVarBind &BcmVarBind::operator = (VarBind &VB)
{
  SetVarBind (&VB);
  return *this;
}

BcmVarBind &BcmVarBind::operator = (const BcmString &String)
{
  SetString (String);
  return *this;
}

BcmVarBind &BcmVarBind::operator = (const char *pString)
{
  BcmString String = pString;
  SetString (String);
  return *this;
}

BcmVarBind &BcmVarBind::operator = (int Value)
{
  SetInt32(Value);
  return *this;
}

BcmVarBind &BcmVarBind::operator = (unsigned int Value)
{
  SetUint32(Value);
  return *this;
}

BcmVarBind &BcmVarBind::operator = (const BcmObjectId &OID)
{
  SetOid (OID);
  return *this;
}

BcmVarBind &BcmVarBind::operator = (const BcmIpAddress &Value)
{
  SetIp (Value);
  return *this;
}

BcmVarBind &BcmVarBind::operator = (const BcmMacAddress &Value)
{
  SetMacAddr (Value);
  return *this;
}

bool BcmVarBind::operator == (const BcmVarBind &VB) const
{
  bool Equals = true;

  // Not equal if two types are not equal
  if (VB.GetType() != GetType())
    Equals = false;
    
  // Not equal if textual conventions are not equal
  if (VB.GetTextualConvention() != GetTextualConvention())
    Equals = false;
    
  switch (GetType())
  {
    case VBT_NUMBER:
    {
      int My32, His32;

      VB.GetInt32 (&His32);
      GetInt32 (&My32);

      if (His32 != My32)
        Equals = false;
      break;
    }
      
    case VBT_STRING:
    case VBT_OPAQUE:
    {
      BcmString MyString, HisString;
      VB.GetString (HisString);
      GetString (MyString);
      if (HisString != MyString)
        Equals = false;
      break;
    }
      
    case VBT_OBJECT:
    {
      BcmObjectId MyOid, HisOid;
      VB.GetOid (HisOid);
      GetOid (MyOid);
      if (HisOid != MyOid)
        Equals = false;
      break;
    }
      
    case VBT_EMPTY:
      break;
      
    case VBT_IPADDRESS:
    {
      BcmIpAddress MyIp, HisIp;
      VB.GetIp (HisIp);
      GetIp (MyIp);
      if (HisIp != MyIp)
        Equals = false;
      break;
    }
    
    case VBT_GAUGE:  
    case VBT_COUNTER:
    case VBT_UINTEGER32:
    {
      unsigned int My32, His32;

      VB.GetUint32 (&His32);
      GetUint32 (&My32);

      if (His32 != My32)
        Equals = false;
      break;
    }
      
    case VBT_TIMETICKS:
    {
      unsigned long MyTicks, HisTicks;

      VB.GetTimeticks (&HisTicks);
      GetTimeticks (&MyTicks);

      if (HisTicks != MyTicks)
        Equals = false;
      break;
    }
      
    case VBT_COUNTER64:
    {
      BcmCounter64 My64, His64;
      VB.GetUint64 (His64);
      GetUint64 (My64);
      if (His64 != My64)
        Equals = false;
      break;
    }
    
    default:
      break;
  }

  return Equals;
}

bool BcmVarBind::operator == (int Value) const
{
  int IntVal;
  SNMP_STATUS Status;
  
  Status = GetInt32 (&IntVal);
  
  if (Status != SNMP_NO_ERROR)
    return false;
    
  return (Value == IntVal);
}

bool BcmVarBind::operator == (unsigned int Value) const
{
  unsigned int UintVal;
  SNMP_STATUS Status;
  
  Status = GetUint32 (&UintVal);
  
  if (Status != SNMP_NO_ERROR)
    return false;
    
  return (Value == UintVal);
}

bool BcmVarBind::operator == (const BcmString &Value) const
{
  BcmString StrVal;
  SNMP_STATUS Status;
  
  Status = GetString (StrVal);
  
  if (Status != SNMP_NO_ERROR)
    return false;
    
  return (Value == StrVal);
}

bool BcmVarBind::operator == (const char *pValue) const
{
  BcmString StrVal;
  SNMP_STATUS Status;
  
  Status = GetString (StrVal);
  
  if (Status != SNMP_NO_ERROR)
    return false;
    
  return (StrVal == pValue);
}

bool BcmVarBind::operator == (const BcmObjectId &Value) const
{
  BcmObjectId OidVal;
  SNMP_STATUS Status;
  
  Status = GetOid (OidVal);
  
  if (Status != SNMP_NO_ERROR)
    return false;
    
  return (Value == OidVal);
}

bool BcmVarBind::operator == (const BcmIpAddress &Value) const
{
  BcmIpAddress IpVal;
  SNMP_STATUS Status;
  
  Status = GetIp (IpVal);
  
  if (Status != SNMP_NO_ERROR)
    return false;
    
  return (Value == IpVal);
}

bool BcmVarBind::operator == (const BcmMacAddress &Value) const
{
  BcmMacAddress MacVal;
  SNMP_STATUS Status;
  
  Status = GetMacAddr (MacVal);
  
  if (Status != SNMP_NO_ERROR)
    return false;
    
  return (Value == MacVal);
}

bool BcmVarBind::operator != (const BcmVarBind &VB) const
{
  if (*this == VB)
    return false;
    
  return true;
}

bool BcmVarBind::operator != (int Value) const
{
  int IntVal;
  SNMP_STATUS Status;
  
  Status = GetInt32 (&IntVal);
  
  if (Status != SNMP_NO_ERROR)
    return false;
    
  return (Value != IntVal);
}

bool BcmVarBind::operator != (unsigned int Value) const
{
  unsigned int UintVal;
  SNMP_STATUS Status;
  
  Status = GetUint32 (&UintVal);
  
  if (Status != SNMP_NO_ERROR)
    return false;
    
  return (Value != UintVal);
}

bool BcmVarBind::operator != (const BcmString &Value) const
{
  BcmString StrVal;
  SNMP_STATUS Status;
  
  Status = GetString (StrVal);
  
  if (Status != SNMP_NO_ERROR)
    return false;
    
  return (StrVal != Value);
}

bool BcmVarBind::operator != (const char *pValue) const
{
  BcmString StrVal;
  SNMP_STATUS Status;
  
  Status = GetString (StrVal);
  
  if (Status != SNMP_NO_ERROR)
    return false;
    
  return (StrVal != pValue);
}

bool BcmVarBind::operator != (const BcmObjectId &Value) const
{
  BcmObjectId OidVal;
  SNMP_STATUS Status;
  
  Status = GetOid (OidVal);
  
  if (Status != SNMP_NO_ERROR)
    return false;
    
  return (Value != OidVal);
}

bool BcmVarBind::operator != (const BcmIpAddress &Value) const
{
  BcmIpAddress IpVal;
  SNMP_STATUS Status;
  
  Status = GetIp (IpVal);
  
  if (Status != SNMP_NO_ERROR)
    return false;
    
  return (Value != IpVal);
}

bool BcmVarBind::operator != (const BcmMacAddress &Value) const
{
  BcmMacAddress MacVal;
  SNMP_STATUS Status;
  
  Status = GetMacAddr (MacVal);
  
  if (Status != SNMP_NO_ERROR)
    return false;
    
  return (Value != MacVal);
}

SNMP_STATUS BcmVarBind::SetEmpty ()
{
  Reset();

  fType = VBT_EMPTY;
  
  if (fpVB)
    VARBIND_SET_EMPTY (fpVB);
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::SetEmpty (int type)
{
  Reset();

  fType = type;
  
  if (fpVB)
    VARBIND_SET_EMPTY (fpVB);
    
  return SNMP_NO_ERROR;
}

// Yet another new approach here.  We want to be consistent with the old
// RAM-hungry behaviour, which was to allocate a new VarBind on construction
// unless we got passed one in the constructor, then always copy.  So to
// be consistent here, what we really should do is not adopt the VB, but copy
// IF we already have one.  Otherwise leave it NULL.
SNMP_STATUS BcmVarBind::SetVarBind (VarBind *pVB)
{
  Reset();
  
  fType = pVB->Type;
  
  switch (fType)
  {
    case VBT_NUMBER:
      fNumber32 = pVB->Value.Number32;
      break;
      
    case VBT_IPADDRESS:
      fIpAddr.Set (pVB->Value.pIpAddr[0], 
                   pVB->Value.pIpAddr[1], 
                   pVB->Value.pIpAddr[2], 
                   pVB->Value.pIpAddr[3]);
      break;
        
    case VBT_STRING:
    //case VBT_BITS:
    case VBT_OPAQUE:
      fString.assign ((const char *)pVB->Value.String.pOctets, pVB->Value.String.Length);
      break;
      
    case VBT_OBJECT:
      fOID = pVB->Value.OID;
      break;
      
    case VBT_EMPTY:
      // Nothing to do...
      break;
      
    case VBT_COUNTER:
    case VBT_GAUGE:
    //case VBT_UNSIGNED32:
    case VBT_TIMETICKS:
    case VBT_UINTEGER32:
      fUnsigned32 = pVB->Value.Counter32;
      break;
      
    case VBT_COUNTER64:
      fC64.Set(pVB->Value.C64.Lo, pVB->Value.C64.Hi);
      break;
    
    default:
      break;
  }

  // 5/28/2003: change this approach.  This gives us trouble if we use the
  // same BcmVarBind object over and over and call SetVarBind with different
  // VarBind structures, because each successive call will corrupt the first
  // pointer we got passed in.  So instead, each time this is called adopt
  // the new pointer and discard the old one.
  
  // Yet another new approach here.  We want to be consistent with the old
  // RAM-hungry behaviour, which was to allocate a new VarBind on construction
  // unless we got passed one in the constructor, then always copy.  So to
  // be consistent here, what we really should do is not adopt the VB, but copy
  // IF we already have one.  Otherwise leave it NULL.
    
  // If we already have a VB, just copy this into him.
  if (fpVB)
    memcpy (fpVB, pVB, sizeof (VarBind));
/*    
  // Otherwise, store the VB we got passed.
  else
    fpVB = pVB;
*/    
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::SetVarBind (const BcmVarBind &VB)
{
  Reset();
  
  fType = VB.fType;
  fNumber32 = VB.fNumber32;
  fUnsigned32 = VB.fUnsigned32;
  fString = VB.fString;
  fOID = VB.fOID;
  fC64 = VB.fC64;
  
  // PR 7418:  Gotta copy the (not so) new fIpAddr object, too!
  // Immediate result of having not done this is that IP address objects
  // in traps would always have a value of 0.0.0.0
  fIpAddr = VB.fIpAddr;
  
  fTextualConvention = VB.fTextualConvention;
  
  // If we have a VB pointer, copy the contents of the supplied reference
  // into it.
  if (fpVB)
    VB.GetVarBind (fpVB);

/*    
  // 5/28/03: Don't adopt the fpVB of the supplied VB pointer.  In the old
  // RAM-HOG days we didn't adopt it, so we should not here either to keep
  // behaviour the same.
  
  // Otherwise, if the supplied reference has a VB pointer, adopt his.
  else if (VB.fpVB)
    fpVB = VB.fpVB;
*/
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::SetString (const unsigned char *pString, int Len)
{
  return SetString ((const char *)pString, Len);
}

SNMP_STATUS BcmVarBind::SetString (const char *pString, int Len)
{
  Reset();

  // If the length is indicated as -1, take the length from strlen.
  if (Len == -1)
    Len = strlen (pString);
   
  fString.assign (pString, Len);
  fType = VBT_STRING;
  
  if (fpVB)
  {
    // If the VB already has a buffer allocated, but
    // it is not big enough for the string, free up the memory here
    // and we will reallocate below and assign Size and Length.
    if (fpVB->Value.String.pOctets != NULL)
      if (fString.length() > fpVB->Value.String.Size) {
        delete [] fpVB->Value.String.pOctets;
        fpVB->Value.String.pOctets = NULL;
      }

    // Allocate storage for our string if needed.  If there is already
    // a pointer there, just use it.
    if (fpVB->Value.String.pOctets == NULL)
    {
      fpVB->Value.String.pOctets = new unsigned char[Len];
      fpVB->Value.String.Size = Len;
      //fMyString = true;
    }
    
    VARBIND_SET_STRING (fpVB, pString, Len);
  }
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::SetString (const BcmString &String)
{
  int Loop, Len = String.length();
  
  
  Reset();
  
  fString = String;
  fType = VBT_STRING;
  
  if (fpVB)
  {
    // If the VB already has a buffer allocated, but
    // it is not big enough for the string, free up the memory here
    // and we will reallocate below and assign Size and Length.
    if (fpVB->Value.String.pOctets != NULL)
      if (fString.length() > fpVB->Value.String.Size) {
        delete [] fpVB->Value.String.pOctets;
        fpVB->Value.String.pOctets = NULL;
      }

    // Allocate storage for our string if needed.  If there is already
    // a pointer there, use it.
    if (fpVB->Value.String.pOctets == NULL)
    {
      fpVB->Value.String.pOctets = new unsigned char[Len];
      fpVB->Value.String.Size = Len;
      //fMyString = true;
    }
    
    for (Loop = 0; Loop < Len; Loop++)
      fpVB->Value.String.pOctets[Loop] = String[Loop];
  /*    
    // No NULL terminator for SNMP strings.
    // Check that: this could be a binary string, and we
    // don't want to eliminate 0's in that case.  Also, what
    // if it's an empty string?  Best to not do this!
    if (fpVB->Value.String.pOctets[Len-1] == 0)
      Len--;
  */    
    fpVB->Type = VBT_STRING;
    fpVB->Value.String.Length = Len;
  }
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::SetOid (const ObjectId *pOID)
{
  Reset();

  fType = VBT_OBJECT;
  
  if (pOID)
    fOID = *pOID;
  else
    fOID = "";
  
  if (fpVB)
    VARBIND_SET_OID (fpVB, pOID);
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::SetOid (const BcmObjectId &OID)
{
  Reset ();
  fType = VBT_OBJECT;
  fOID = OID;

  if (fpVB)
  {
    ObjectId Oid;
    
    OID.GetOID (&Oid);
    VARBIND_SET_OID (fpVB, &Oid);
  }
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::SetIp (const unsigned char *pIp)
{
  Reset();

  fType = VBT_IPADDRESS;
  
  fIpAddr.Set(pIp[0], pIp[1], pIp[2], pIp[3]);

  if (fpVB)
    VARBIND_SET_IPADDR (fpVB, pIp);
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::SetIp (const BcmIpAddress &Ip)
{
  Reset();

  fType = VBT_IPADDRESS;
  fIpAddr = Ip;
  
  if (fpVB)
  {
    unsigned char pIp[4] = {0,0,0,0}; 

    if (Ip.IsIpV6Address() == false)
      Ip.Get (pIp, 4);
    else
      SnmpLogRaw << "WARNING: Trying to set an IpAddress varbind to an IPv6 address, using 0.0.0.0." << endl;
    
    VARBIND_SET_IPADDR (fpVB, pIp);
  }
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::SetMacAddr (const BcmMacAddress &Mac)
{
  unsigned char pMac[6];

                   
  Reset();
  
  SetTextualConvention(kMacAddress);

  Mac.Get (pMac[0],pMac[1],pMac[2],pMac[3],pMac[4],pMac[5]);
  return SetString (pMac, 6);
}

SNMP_STATUS BcmVarBind::SetInt32 (int Value)
{
  Reset();

  fType = VBT_NUMBER;
  fNumber32 = Value;
  
  if (fpVB)
    VARBIND_SET_INT32 (fpVB, Value);
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::SetBool (bool BoolVal)
{
  if ((BoolVal != true) && (BoolVal != false))
    return SNMP_WRONG_VALUE;
    
  Reset();
    
  fType = VBT_NUMBER;
  
  // translate bool to truth
  if (BoolVal == true)
    fNumber32 = SNMP_TRUTH_TRUE;
  else
    fNumber32 = SNMP_TRUTH_FALSE;
    
  if (fpVB)
  {
    // translate bool to truth
    if (BoolVal == true)
      VARBIND_SET_INT32 (fpVB, SNMP_TRUTH_TRUE);
    else
      VARBIND_SET_INT32 (fpVB, SNMP_TRUTH_FALSE);
  }
    
  SetTextualConvention (kTruthValue);
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::SetUint32 (unsigned int Value, int Type)
{
  Reset();

  fType = Type;
  fUnsigned32 = Value;
  
  if (fpVB)
    VARBIND_SET_UINT32 (fpVB, Value, Type);
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::SetUint64 (const BcmCounter64 &C64)
{
  Reset();
  
  fType = VBT_COUNTER64;
  fC64 = C64;
  
  if (fpVB)
    VARBIND_SET_UINT64 (fpVB, C64.High32(), C64.Low32());
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::SetTimeticks (unsigned long Ticks)
{
  Reset();
  
  fType = VBT_TIMETICKS;
  fUnsigned32 = Ticks;
  
  if (fpVB)
    VARBIND_SET_UINT32 (fpVB, Ticks, VBT_TIMETICKS);
    
  return SNMP_NO_ERROR;
}

/*
   From RFC-2579 (SNMPv2-TC):
   
   DateAndTime ::= TEXTUAL-CONVENTION
       DISPLAY-HINT "2d-1d-1d,1d:1d:1d.1d,1a1d:1d"
       STATUS       current
       DESCRIPTION
               "A date-time specification.

               field  octets  contents                  range
               -----  ------  --------                  -----
                 1      1-2   year                      0..65536
                 2       3    month                     1..12
                 3       4    day                       1..31
                 4       5    hour                      0..23
                 5       6    minutes                   0..59
                 6       7    seconds                   0..60
                              (use 60 for leap-second)
                 7       8    deci-seconds              0..9
                 8       9    direction from UTC        '+' / '-'
                 9      10    hours from UTC            0..13
                10      11    minutes from UTC          0..59

               For example, Tuesday May 26, 1992 at 1:30:15 PM EDT would be
               displayed as:

                                1992-5-26,13:30:15.0,-4:0

               Note that if only local time is known, then timezone
               information (fields 8-10) is not present."
       SYNTAX       OCTET STRING (SIZE (8 | 11))
*/
SNMP_STATUS BcmVarBind::SetDateAndTime (time_t EpochTime, bool IncludeTimezone, int UtcOffsetSeconds)
{
  unsigned char SnmpTime[11];
  int SnmpSize = 11;
  struct tm *tmTime = localtime (&EpochTime);
  int Year = tmTime->tm_year + 1900;

  Reset();
  
  // Special case: if we get the special value kAllZerosDateAndTime,
  // set everything to 0's, and don't include timezone info.
  if (EpochTime == (time_t)kAllZerosDateAndTime)
  {
    SnmpSize = 8;
    memset (SnmpTime, 0, SnmpSize);
  }
  
  // Otherwise, format the string according to the RFC.
  else
  {
    SnmpTime[0] = (Year >> 8);
    SnmpTime[1] = (Year & 0xFF);
    SnmpTime[2] = tmTime->tm_mon + 1;
    SnmpTime[3] = tmTime->tm_mday;
    SnmpTime[4] = tmTime->tm_hour;
    SnmpTime[5] = tmTime->tm_min;
    SnmpTime[6] = tmTime->tm_sec;
    SnmpTime[7] = 0; // Tenths of seconds = 0. Close enough.
  
    // Include timezone offset info if requested
    if (IncludeTimezone)
    {
      int HoursFromUtc = 0;
      int MinutesFromUtc = 0;
      
      if (UtcOffsetSeconds < 0)
      {
        SnmpTime[8] = '-';
        
        // Hour and minutes fields must be positive even if the offset seconds
        // are negative.  So if seconds is negative, positivize it.
        UtcOffsetSeconds *= -1;
      }
      else
        SnmpTime[8] = '+';
        
      HoursFromUtc = UtcOffsetSeconds / 3600;
      MinutesFromUtc = (UtcOffsetSeconds % 3600) / 60;
      
      // Anything bigger than 13 hours is invalid.  RFC-1907 said 11 for
      // this but then they realized that during daylight savings time in
      // New Zeland it's 13 hours.  So RFC-2579 updated to 13.
      if (HoursFromUtc > 13)
        HoursFromUtc = 13;
        
      SnmpTime[9] = HoursFromUtc;
      SnmpTime[10] = MinutesFromUtc;
        
      SnmpSize = 11;
    }
    else
    {
      SnmpSize = 8;
    }
  }
  
  fType = VBT_STRING;
  fString.assign ((const char *)SnmpTime, SnmpSize);

  if (fpVB)
  {
    // Allocate storage for our string if needed.  If there is already
    // a pointer there, use it.
    if (fpVB->Value.String.pOctets == NULL)
    {
      fpVB->Value.String.pOctets = new unsigned char[SnmpSize];
      fpVB->Value.String.Size = SnmpSize;
      //fMyString = true;
    }
    
    VARBIND_SET_STRING (fpVB, SnmpTime, SnmpSize);
  }
  
  SetTextualConvention (kDateAndTime);
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::SetDateAndTime (unsigned int Month, unsigned int Day, unsigned int Year, unsigned int Hour, unsigned int Minute, unsigned int Second)
{
  unsigned char SnmpTime[8];
  
  // Check for valid values first.  Note that 0/0/0000 00:00:00 is a special
  // case OK time which will result in an all 0s string.
  if ((Month == 0) && (Day == 0)    && (Year == 0)
  &&  (Hour == 0)  && (Minute == 0) && (Second == 0))
    /* this is OK */;
  else if ((Month < 1) || (Month > 12) 
  ||  (Day < 1) || (Day > 31)
  ||  (Year > 65536)
  ||  (Hour > 23)
  ||  (Minute > 59)
  ||  (Second > 60))
    return SNMP_WRONG_VALUE;

  Reset();
  
  SnmpTime[0] = (Year >> 8);
  SnmpTime[1] = (Year & 0xFF);
  SnmpTime[2] = Month;
  SnmpTime[3] = Day;
  SnmpTime[4] = Hour;
  SnmpTime[5] = Minute;
  SnmpTime[6] = Second;
  SnmpTime[7] = 0; // Tenths of seconds = 0. Close enough.
  
  // We won't use the timezone fields, we have no concept
  // of our local timezone.
  
  fType = VBT_STRING;
  fString.assign ((const char *)SnmpTime, 8);

  if (fpVB)
  {
    // Allocate storage for our string if needed.  If there is already
    // a pointer there, use it.
    if (fpVB->Value.String.pOctets == NULL)
    {
      fpVB->Value.String.pOctets = new unsigned char[8];
      fpVB->Value.String.Size = 8;
      //fMyString = true;
    }
    
    VARBIND_SET_STRING (fpVB, SnmpTime, 8);
  }
  
  SetTextualConvention (kDateAndTime);
  return SNMP_NO_ERROR;
}

//
// An extended version of SetDateAndTime that supports the TimeZone information
//
SNMP_STATUS BcmVarBind::SetDateAndTime (unsigned int Month, unsigned int Day, unsigned int Year, unsigned int Hour, unsigned int Minute,
                                        unsigned int Second, char DirFromUTC, unsigned int HourFromUTC, unsigned int MinFromUTC)
{
  unsigned char SnmpTime[11];
  
  // Check for valid values first.  Note that 0/0/0000 00:00:00 is a special
  // case OK time which will result in an all 0s string.
  if ((Month == 0) && (Day == 0)    && (Year == 0)
  &&  (Hour == 0)  && (Minute == 0) && (Second == 0))
    /* this is OK */;
  else if ((Month < 1) || (Month > 12) 
  ||  (Day < 1) || (Day > 31)
  ||  (Year > 65536)
  ||  (Hour > 23)
  ||  (Minute > 59)
  ||  (Second > 60)
  ||  ((DirFromUTC != '+') && (DirFromUTC != '-'))
  ||  (HourFromUTC > 13)   // yes 13, daylight saving time in New Zealand is +13
  ||  (MinFromUTC > 59))
    return SNMP_WRONG_VALUE;

  Reset();
  
  SnmpTime[0] = (Year >> 8);
  SnmpTime[1] = (Year & 0xFF);
  SnmpTime[2] = Month;
  SnmpTime[3] = Day;
  SnmpTime[4] = Hour;
  SnmpTime[5] = Minute;
  SnmpTime[6] = Second;
  SnmpTime[7] = 0; // Tenths of seconds = 0. Close enough.
  SnmpTime[8] = DirFromUTC;
  SnmpTime[9] = HourFromUTC;
  SnmpTime[10] = MinFromUTC;
    
  fType = VBT_STRING;
  fString.assign ((const char *)SnmpTime, 11);

  if (fpVB)
  {
    // Allocate storage for our string if needed.  If there is already
    // a pointer there, use it.
    if (fpVB->Value.String.pOctets == NULL)
    {
      fpVB->Value.String.pOctets = new unsigned char[11];
      fpVB->Value.String.Size = 11;
      //fMyString = true;
    }
    
    VARBIND_SET_STRING (fpVB, SnmpTime, 11);
  }
  
  SetTextualConvention (kDateAndTime);
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::SetBits (unsigned long NormalBitmask, int NumOctets)
{
  unsigned char pBits[] = {0,0,0,0};
  unsigned long Bit;
  int Loop, NumBits;
  
  
  SnmpLogInit << "BcmVarBind::SetBits " << NormalBitmask << ", " << NumOctets << " octets\n" << endl;
  
  if (NumOctets > 4)
  {
    SnmpLogInit << "Larger than 32 bits!  WRONG_LENGTH" << endl;
    return SNMP_WRONG_LENGTH;
  }
  
  Reset();
  
  // if NumOctets is unspecified, use as many bytes as required to
  // express the value given.
  if (NumOctets == 0)
  {
    if (NormalBitmask <= 0x000000FF)
      NumOctets = 1;
    else if (NormalBitmask <= 0x0000FFFF)
      NumOctets = 2;
    else if (NormalBitmask <= 0x00FFFFFF)
      NumOctets = 2;
    else
      NumOctets = 4;
  }
    
  // Convert the normal 32 bit bitmask to an SNMP-BITS syntax.  The syntax of
  // BITS is kind of weird. The bits are arranged in reverse order of normal; 
  // that is, bit 0 for SNMP purposes is considered to be the MOST significant
  // bit.  Plus, the BITS is an arbitrarily long array of bytes, not a word or
  // a dword.  This is to allow a bitmask of arbitrary length.  This function
  // only supports up to 32 bits.  Need more than 32, you're on your own; luckily,
  // so far as I know none of the DOCSIS MIB's has more than 32 bits...
  
  NumBits = NumOctets * 8;
  for (Loop = 0; Loop < NumBits; Loop++)
  {
    Bit = (NormalBitmask >> Loop) & 0x00000001;
    pBits[Loop/8] |= (Bit << (7 - (Loop % 8)));
  }
  
  SnmpLogInit << NumOctets <<": " << " " << NormalBitmask << " -> " << pBits[0] << " " <<pBits[1] << " " << pBits[2] << pBits[3] << endl;
  
  fType = VBT_STRING;
  fString.assign((const char *)pBits, NumOctets);
  
  if (fpVB)
  {
    // If the VB already has a buffer allocated, but
    // it is not big enough for the string, free up the memory here
    // and we will reallocate below and assign Size and Length.
    if (fpVB->Value.String.pOctets != NULL)
      if (fString.length() > fpVB->Value.String.Size) {
        delete [] fpVB->Value.String.pOctets;
        fpVB->Value.String.pOctets = NULL;
      }

    // Allocate storage for our string if needed.  If there is already
    // a pointer there, use it.
    if (fpVB->Value.String.pOctets == NULL)
    {
      fpVB->Value.String.pOctets = new unsigned char[NumOctets];
      fpVB->Value.String.Size = NumOctets;
      //fMyString = true;
    }
    
    VARBIND_SET_STRING (fpVB, pBits, NumOctets);
  }
  
  return SNMP_NO_ERROR;
}

// PR 11894:  Support the noSuchObject and noSuchInstance exception types
SNMP_STATUS BcmVarBind::SetNoSuchObject ()
{
  Reset();

  fType = VBT_NOSUCHOBJECT;
  
  if (fpVB)
    VARBIND_SET_TYPE (fpVB, VBT_NOSUCHOBJECT);
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::SetNoSuchInstance ()
{
  Reset();

  fType = VBT_NOSUCHINSTANCE;
  
  if (fpVB)
    VARBIND_SET_TYPE (fpVB, VBT_NOSUCHINSTANCE);
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::SetPrivate (int Index, void *pPrivate)
{
  if ((Index < 0) || (Index >= VB_PRIVATES))
    return SNMP_BAD_VALUE;
  
  // For now, don't store private pointers in the object, only
  // transfer them to the fpVB structure if we have one.  
  if (fpVB == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;
    
  fpVB->pPrivate[Index] = pPrivate;
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::SetMisc (void *pMisc)
{
  // For now, don't store misc pointers in the object, only
  // transfer them to the fpVB structure if we have one.  
  if (fpVB == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;
    
  fpVB->pMisc = pMisc;
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::GetVarBind (VarBind *pVB) const
{
  if (fpVB)
    memcpy (pVB, fpVB, sizeof (VarBind));
    
  else
  {
    memset (pVB, 0, sizeof(VarBind));
    pVB->Type = fType;
    
    switch (fType)
    {
      case VBT_NUMBER:
        pVB->Value.Number32 = fNumber32;
        break;
        
      case VBT_IPADDRESS:
        fIpAddr.Get(pVB->Value.pIpAddr[0], 
                    pVB->Value.pIpAddr[1], 
                    pVB->Value.pIpAddr[2], 
                    pVB->Value.pIpAddr[3]);
        break;
        
      case VBT_STRING:
      //case VBT_BITS:
      case VBT_OPAQUE:
      {
        // If the VB we are handed already has a buffer allocated, but 
        // it is not big enough for the string, free up the memory here
        // and we will reallocate below and assign Size and Length.
        if (pVB->Value.String.pOctets != NULL)
          if (fString.length() > pVB->Value.String.Size) {
            delete [] pVB->Value.String.pOctets;
            pVB->Value.String.pOctets = NULL;
          }

       pVB->Value.String.Length = fString.length();
 
        // If the VB we are handed already has storage for the string,
        // copy our string in there.  If not, we need to relocate otherwise
        // the pointer will be deleted in both expunge() and C++ destructor
       if (pVB->Value.String.pOctets == NULL) {
         pVB->Value.String.pOctets = new unsigned char[pVB->Value.String.Length];
         pVB->Value.String.Size = pVB->Value.String.Length;
       }
        memcpy (pVB->Value.String.pOctets, fString.data(), fString.length());
 

        
        break;
      }
        
      case VBT_OBJECT:
        fOID.GetOID(&pVB->Value.OID);
        break;
        
      case VBT_EMPTY:
        // Nothing to do...
        break;
        
      case VBT_COUNTER:
      case VBT_GAUGE:
      //case VBT_UNSIGNED32:
      case VBT_TIMETICKS:
      case VBT_UINTEGER32:
        pVB->Value.Counter32 = fUnsigned32;
        break;
        
      case VBT_COUNTER64:
        pVB->Value.C64.Lo = fC64.Low32();
        pVB->Value.C64.Hi = fC64.High32();
        break;
      
      default:
        break;
    }
  }
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::GetString (BcmString &String) const
{
  if (fType != VBT_STRING)
    return SNMP_WRONG_TYPE;
    
  String = fString;
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::GetString (unsigned char *pString, int MaxLen) const
{
  if (fType != VBT_STRING)
    return SNMP_WRONG_TYPE;
    
  if ((int)fString.length() > MaxLen)
    return SNMP_WRONG_LENGTH;
    
  memcpy (pString, fString.data(), fString.length());
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::GetString (char *pString, int MaxLen) const
{
  return GetString ((unsigned char *)pString, MaxLen);
}

SNMP_STATUS BcmVarBind::GetInt32 (int *pInt32) const
{ 
  if (fType != VBT_NUMBER)                                   
    return SNMP_WRONG_TYPE;
    
  *pInt32 = fNumber32;
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::GetBool (bool *pBoolVal) const
{ 
  if (fType != VBT_NUMBER)                                   
    return SNMP_WRONG_TYPE;
    
  // Translate SNMP truth val to bool
  if (fNumber32 == SNMP_TRUTH_TRUE)
    *pBoolVal = true;
  else if (fNumber32 == SNMP_TRUTH_FALSE)
    *pBoolVal = false;
  else
    return SNMP_WRONG_VALUE;
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::GetUint32 (unsigned int *pUint32, int *pType) const
{
  if ((fType != VBT_UNSIGNED32)
  &&  (fType != VBT_GAUGE)
  &&  (fType != VBT_COUNTER)
  &&  (fType != VBT_UINTEGER32))
    return SNMP_WRONG_TYPE;
    
  *pUint32 = fUnsigned32;
  
  if (pType)
    *pType = fType;
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::GetUint64 (BcmCounter64 &C64) const
{
  if (fType != VBT_COUNTER64)
    return SNMP_WRONG_TYPE;
    
  C64 = fC64;
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::GetTimeticks (unsigned long *Ticks) const
{
  if (fType != VBT_TIMETICKS)
    return SNMP_WRONG_TYPE;
    
  *Ticks = fUnsigned32;
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::GetOid (BcmObjectId &OID) const
{
  if (fType != VBT_OBJECT)
    return SNMP_WRONG_TYPE;
    
  OID = fOID;
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::GetIp (unsigned char *pIp) const
{
  if (fType != VBT_IPADDRESS)
    return SNMP_WRONG_TYPE;
    
  if (fIpAddr.IsIpV6Address() == true)
  {
    SnmpLogRaw << "WARNING: Trying to get an IpAddress varbind from an IPv6 address, using 0.0.0.0." << endl;
    memset (pIp, 0, 4);
  }
  else  
    fIpAddr.Get(pIp[0], pIp[1], pIp[2], pIp[3]);
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::GetIp (BcmIpAddress &Ip) const
{
  if (fType != VBT_IPADDRESS)
    return SNMP_WRONG_TYPE;
    
  Ip = fIpAddr;
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::GetMacAddr (BcmMacAddress &Mac) const
{
  if (fType != VBT_STRING)
    return SNMP_WRONG_TYPE;
    
  if (fString.length() != 6)
    return SNMP_WRONG_LENGTH;
    
  Mac.Set (fString[0],fString[1],fString[2],fString[3],fString[4],fString[5]);
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmVarBind::GetBits (unsigned long *pNormalBitmask, int *pNumBytes) const
{
  unsigned long SnmpBitmask = 0, Bit;
  int Loop, NumBits;
  int NumOctets = 4;
  
  
  SnmpLogInit << "BcmVarBind::GetBits" << endl;

  if (fType != VBT_STRING)
  {
    SnmpLogInit << "Not VBT_STRING" << endl;
    return SNMP_WRONG_TYPE;
  }
    
  if (fString.length() > 4)
  {
    // Don't reject this.  CL is liable to throw some stupid
    // long string at us.  Just truncate.
    //SnmpLogInit << "Larger than 32 bits.  WRONG_LENGTH" << endl;
    //return SNMP_WRONG_LENGTH;
    
    SnmpLogRaw << "WARNING - BcmVarBind::GetBits truncating to 4 bytes from " << fpVB->Value.String.Length << endl;
  }
  else
    NumOctets = fString.length();
    
  // Convert the normal 32 bit bitmask to an SNMP-BITS syntax.  The syntax of
  // BITS is kind of weird. The bits are arranged in reverse order of normal; 
  // that is, bit 0 for SNMP purposes is considered to be the MOST significant
  // bit.  Plus, the BITS is an arbitrarily long array of bytes, not a word or
  // a dword.  This is to allow a bitmask of arbitrary length.  This function
  // only supports up to 32 bits.  Need more than 32, you're on your own; luckily,
  // so far as I know none of the DOCSIS MIB's has more than 32 bits...
  
  // First, build a backwards, 32 bit SNMP style bitmask.  Example: say we have
  // a 3 octet bitmask, 0x01 0x02 0x03.  The SnmpBitmask of that is 0x00010203.
  //SnmpLogInit << "VB: " << pBits->Length << " octets " << endl;
  //for (Loop = 0; Loop < pBits->Length; Loop++)
  SnmpLogInit << "VB: " << NumOctets << " octets " << endl;
  for (Loop = 0; Loop < NumOctets; Loop++)
  {
    SnmpLogInit << fString[Loop];
    //SnmpBitmask |= (fString[Loop] << ((fString.length() - Loop - 1) * 8));
    SnmpBitmask |= (fString[Loop] << ((NumOctets - Loop - 1) * 8));
  }
  SnmpLogInit << "\nSnmpBitmask = " << SnmpBitmask;
    
  // Now, reverse the order of the bits to arrive at the "normal" bitmask.
  // We can't just flat-out reverse the dword, though, because that ignores
  // how long the bitmask is.  In our example, we want to wind up with
  // 0x01 0x02 0x03 -> 0x00010203 -> 0x00C04080, not
  // 0x01 0x02 0x03 -> 0x00010203 -> 0xC0408000
  
  *pNormalBitmask = 0;
  
  //NumBits = fString.length() * 8;
  NumBits = NumOctets * 8;
  for (Loop = 0; Loop < NumBits; Loop++)
  {
    Bit = (SnmpBitmask >> Loop) & 0x00000001;
    *pNormalBitmask |= (Bit << (NumBits - Loop - 1));
  }
  
  if (pNumBytes)
    //*pNumBytes = pBits->Length;
    *pNumBytes = NumOctets;

  SnmpLogInit << "NormalBitmask = " << *pNormalBitmask << endl;
    
  return SNMP_NO_ERROR;
}

/*
   From RFC-1903 (SNMPv2-TC):
   
   DateAndTime ::= TEXTUAL-CONVENTION
       DISPLAY-HINT "2d-1d-1d,1d:1d:1d.1d,1a1d:1d"
       STATUS       current
       DESCRIPTION
               "A date-time specification.

               field  octets  contents                  range
               -----  ------  --------                  -----
                 1      1-2   year                      0..65536
                 2       3    month                     1..12
                 3       4    day                       1..31
                 4       5    hour                      0..23
                 5       6    minutes                   0..59
                 6       7    seconds                   0..60
                              (use 60 for leap-second)
                 7       8    deci-seconds              0..9
                 8       9    direction from UTC        '+' / '-'
                 9      10    hours from UTC            0..11
                10      11    minutes from UTC          0..59

               For example, Tuesday May 26, 1992 at 1:30:15 PM EDT would be
               displayed as:

                                1992-5-26,13:30:15.0,-4:0

               Note that if only local time is known, then timezone
               information (fields 8-10) is not present."
       SYNTAX       OCTET STRING (SIZE (8 | 11))
*/
SNMP_STATUS BcmVarBind::GetDateAndTime (time_t *EpochTime) const
{
  struct tm *pTmTime;
  struct tm tmTime;
  int Year; // tmTime->tm_year + 1900;
  unsigned char *pString;
  
  SnmpLogReq << "BcmVarBind::GetDateAndTime" << endl;
  
  if (fType != VBT_STRING)
  {
    SnmpLogReq << "  Not VBT_STRING - WRONG_TYPE" << endl;
    return SNMP_WRONG_TYPE;
  }
  
  // DateAndTime must be exactly 8 or 11 octets in length
  if ((fString.length() != 8) && (fString.length() != 11))
  {
    SnmpLogReq << "  Not 8 or 11 bytes: " << fString.length() << ", WRONG_LENGTH" << endl;
    return SNMP_WRONG_LENGTH;
  }
  
  // Convert to unsigned char's this way:
  pString = (unsigned char *)fString.data();
  
  // Special case: if everything is 0's, return the special kAllZerosDateAndTime
  // value.
  if ((pString[0] == 0) && (pString[1] == 0)
  &&  (pString[2] == 0) && (pString[3] == 0)
  &&  (pString[4] == 0) && (pString[5] == 0)
  &&  (pString[6] == 0) && (pString[7] == 0))
  {
    SnmpLogReq << "  All 0's, setting to 1/1/1970" << endl; 
    *EpochTime = kAllZerosDateAndTime;
    return SNMP_NO_ERROR;
  }
  
  // Verify range of fields per TC.
  if ((pString[2] < 1) || (pString[2] > 12)  // Month: 1-12
  ||  (pString[3] < 1) || (pString[3] > 31)  // Day: 1-31
  ||  (pString[4] > 23)  // Hour: 0-23
  ||  (pString[5] > 59)  // Minute 0-59
  ||  (pString[6] > 60)  // Second: 0-60 (60? That's what 1907 says...)
  ||  (pString[7] > 10)) // Deci-second: 0-10
  {
    SnmpLogReq << "  Some field is bad, WRONG_VALUE" << endl;
    return SNMP_WRONG_VALUE;
  }
    
  // If timezone is specified, check octets 8,9, and 10.  Note that we have
  // no interest in these values (we don't use them) but we should check
  // for appropriate ranges if they are specified.
  if (fString.length() == 11)
  {
    // Direction from UTC: '+' or '-'
    if ((pString[8] != '+') && (pString[8] != '-'))
    {
      SnmpLogReq << "  TZ field doesn't start w/ + or -, WRONG_VALUE" << endl;
      return SNMP_WRONG_VALUE;
    }
      
    if ((pString[9] > 11)   // Hours from UTC: 0-11
    ||  (pString[10] > 59)) // Minutes from UTC: 0-59
    {
      SnmpLogReq << "  TZ field not between 00:00 and 11:59, WRONG_VALUE" << endl;
      return SNMP_WRONG_VALUE;
    }
  }
  
  // initialize some stuff in the tm structure (DST, etc) by getting 
  // the current time:
  time (EpochTime);
  pTmTime = localtime (EpochTime);
  
  // Copy to our local structure.
  memcpy (&tmTime, pTmTime, sizeof (struct tm));
  
  SnmpLogReq << "  parsed date/time OK.  Current time: " << ctime(EpochTime) << endl;
  
  // Certain compilers get confused about the fact that we are doing some
  // unsigned char -> int math here.  So take it easy on the compiler and
  // do this one step at a time...
  Year = pString[0];
  Year <<= 8;
  Year += pString[1];
  
  tmTime.tm_year = Year - 1900;
  tmTime.tm_mon  = pString[2] - 1;
  tmTime.tm_mday = pString[3];
  tmTime.tm_hour = pString[4];
  tmTime.tm_min  = pString[5];
  tmTime.tm_sec  = pString[6];
  
  SnmpLogReq << "  tm fields:" << "\n"
             << "    tm_year=" << tmTime.tm_year << "\n"
             << "    tm_mon=" << tmTime.tm_mon << "\n"
             << "    tm_mday=" << tmTime.tm_mday << "\n"
             << "    tm_hour=" << tmTime.tm_hour << "\n"
             << "    tm_min=" << tmTime.tm_min << "\n"
             << "    tm_sec=" << tmTime.tm_sec << endl;
  
  *EpochTime = mktime (&tmTime);
  
  // Check for failure:
  if (*EpochTime == -1)
  {
    SnmpLogReq << "  mktime failure!  RESOURCE_UNAVAILABLE" << endl;  
    return SNMP_RESOURCE_UNAVAILABLE;
  }
  
  SnmpLogReq << "  New time:" << ctime(EpochTime) << "NO_ERROR" << endl;
  
  return SNMP_NO_ERROR;
}

void *BcmVarBind::GetPrivate (int Index) const
{
  if ((Index < 0) || (Index >= VB_PRIVATES) || (fpVB == NULL))
    return NULL;
    
  return fpVB->pPrivate[Index];
}

void *BcmVarBind::GetMisc () const
{
  if (fpVB == NULL)
    return NULL;
    
  return fpVB->pMisc;
}


