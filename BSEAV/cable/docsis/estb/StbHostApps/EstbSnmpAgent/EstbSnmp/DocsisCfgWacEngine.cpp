//****************************************************************************
//
//  Copyright (c) 2003  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id$
//
//  Filename:       DocsisCfgWacEngine.cpp
//  Author:         Kevin O'Neal
//  Creation Date:  August 29, 2003
//
//****************************************************************************
//  Description:
//      This class implements an engine to process SNMP write access control
//      elements form a DOCSIS config file (TLV-10 for CM and CableHome)
//
//****************************************************************************

#include "DocsisCfgWacEngine.h"
#include "BerObjs.h"
#include "SnmpLog.h"


BcmDocsisCfgWacEngine::BcmDocsisCfgWacEngine ()
{
}

BcmDocsisCfgWacEngine::~BcmDocsisCfgWacEngine ()
{
}

ostream &BcmDocsisCfgWacEngine::Print(ostream &outStream) const
{
  list<WacRec *>::const_iterator Pos = fWacs.begin();
  WacRec *pWac = NULL;
  
  outStream << "WAC List:" << endl;
  
  while (Pos != fWacs.end())
  {
    pWac = *Pos;
    
    outStream << "  " << pWac->OID << ": " << pWac->Access << endl;
    
    Pos++;
  }
  
  return outStream;
}

SNMP_STATUS BcmDocsisCfgWacEngine::AddWac (const unsigned char *pWacBuf, int Length)
{
  bool Access;
  BcmOctetBuffer myBuff;
  BcmObjectId Oid;

  /*\
   *  The config file TLV-10 buffer that we're getting here is really not a
   *  full OID tuple, it's just the 'V' part of that TLV. So we need to create
   *  a full tuple by prepending the OID type code as well as the full length
   *  so that our BER code can parse it.
  \*/
  
  // 12/12/2002: Per customer request (PR3996), let's first verify that this is
  // not a full tuple.  That's easy enough to do.  Check to see if the value at
  // pWacBuf[0] == BER_TLV_OBJECT_IDENTIFIER.  If it is, we'll assume that
  // this is a full tuple and not prepend.  We have every reason to believe that
  // CL uses non-tuples, but the only way this would bite us is if they happen
  // to try and set an OID that starts with 0.6, which doesn't even make sense.
  if (pWacBuf[0] != BER_TLV_OBJECT_IDENTIFIER)
  {
    // Prepend OID type code.
    myBuff.AddToEnd((char) BER_TLV_OBJECT_IDENTIFIER);
    
    // Prepend length.  Check to see if the length is greater that 127 and if so 
    // do the magic with the length to create the new byte with the MSB set plus 
    // extra.  Since the config file TLV scheme has only a single octet for the
    // 'L' part, we know that the largest possible length here is 255 bytes, so
    // we can get away with only handling a max of 2 octets for the length here.
    if((Length-1) > 127) 
    {
      uint8 newLenByte;
      newLenByte = BER_EXTEND_LENGTH + ((Length-1) / 128);
      myBuff.AddToEnd((char)(newLenByte));
      myBuff.AddToEnd((char)(((Length-1) % 128)));
    }
    else 
    {
  	  myBuff.AddToEnd((char)(Length-1));
    }
  }

  // Now tack on the raw OID buffer from the config file and we have a full
  // OID tuple.
  myBuff.AddToEnd(pWacBuf,Length-1);
  AsnBerOid BerOid((unsigned char *)myBuff.AccessBuffer());

  SnmpLogCfg << BerOid << endl;

  if (BerOid.GetValue (Oid) != SNMP_NO_ERROR)
  {
    SnmpLogRaw << "BER buffer can't be resolved as an OID:" << myBuff << endl;
    return SNMP_NO_SUCH_NAME;
  }

  // Special case: "0.0" means do everything.
  if ((Oid.Length() == 2)
  &&  (Oid[0] == 0) && (Oid[1] == 0))
  {
    SnmpLogCfg << "0.0 OID being changed to .1\n" << endl;
    Oid = "1";
  }

  SnmpLogCfg << "OID is " << Oid << endl;
  
  // Oddly enough, a '1' in the config file means deny access, a '0'
  // means allow...
  if (*(pWacBuf + Length - 1) == 0)
    Access = true;
  else
    Access = false;

  SnmpLogCfg << "Access set to " << Access << endl;
  
  // OK, now we have an OID and an access.  Make a WacRec and add it into
  // the list.
  
  WacRec *pWac = new WacRec;
  pWac->OID = Oid;
  pWac->Access = Access;
  
  // Insertion sort here.  Order the list of WAC's by length of OID,
  // longest OIDs first.
  list<WacRec *>::iterator Pos = fWacs.begin();
  WacRec *pListWac = NULL;
  
  while (Pos != fWacs.end())
  {
    pListWac = *Pos;
    
    // Longer OIDs go higher in the list.
    // If the OIDs are of equal length, insertion order equals order of
    // addition to the list.  Which is to say, OIDs added last go higher
    // in the list.
    if ((pWac->OID.Length() >  pListWac->OID.Length())
    ||  (pWac->OID.Length() == pListWac->OID.Length()))
    {
      fWacs.insert (Pos, pWac);
      break;
    }
    
    pListWac = NULL;
    Pos++;
  }
  
  // If this WAC is the shortest in the list, or if the list is empty,
  // add it to the end.
  if (pListWac == NULL)
    fWacs.push_back (pWac);
    
  gLogMessageRaw << "CfgWAC(" << fWacs.size() << "): " << pWac->OID << " -> " << pWac->Access << endl;

  return SNMP_NO_ERROR;
}

int BcmDocsisCfgWacEngine::NumWacs () const
{
  return fWacs.size();
}

void BcmDocsisCfgWacEngine::Flush ()
{
  WacRec *pWac = NULL;
  list<WacRec *>::iterator Pos = fWacs.begin();
  
  while (Pos != fWacs.end())
  {
    pWac = *Pos;
    Pos++;
    
    fWacs.remove (pWac);
    delete pWac;
  }
}

SNMP_STATUS BcmDocsisCfgWacEngine::ValidateOid (const BcmObjectId &OID) const
{
  WacRec *pWac;
  list<WacRec *>::const_iterator Pos = fWacs.begin();
  SNMP_STATUS Status = SNMP_NO_ERROR;
  
  // Iterate through the list and look for the first object in the list
  // which matches the supplied OID.
  while (Pos != fWacs.end())
  {
    pWac = *Pos;
    
    if ((pWac->OID == OID)
    ||  (pWac->OID.IsAscendant(OID)))
    {
      if (pWac->Access == false)
      {
        SnmpLogReq << "Matched OID " << pWac->OID << ", access=false, NOT_WRITABLE." << endl;
        Status = SNMP_NOT_WRITABLE;
      }
      else
        SnmpLogReq << "Matched OID " << pWac->OID << ", access=true, NO_ERROR." << endl;
      
      break;
    }
    
    Pos++;
  }

  return Status;
}
