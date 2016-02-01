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
//  Filename:       DocsisCfgVarBindEngine.h
//  Author:         Kevin O'Neal
//  Creation Date:  June 26, 2003
//
//****************************************************************************
//  Description:
//      This class implements an engine to process SNMP varbinds from a 
//      DOCSIS config file (TLV-11 for CM, TLV-28 for CableHome)
//
//****************************************************************************

#ifndef DOCSISCFGVARBINDENGINE_H
#define DOCSISCFGVARBINDENGINE_H

#include "IpAddress.h"
#include "BcmString.h"
#include "BcmSnmpAgent.h"

#include <list>

class BcmSnmpAgent;

typedef struct _VbRec
{
  BcmString VB;
  int Index;
} VbRec;


class BcmDocsisCfgVarBindEngine
{
  public:
    BcmDocsisCfgVarBindEngine (const BcmSnmpAgent *pAgent, const char *pVbName = "CfgVB", bool AllowDuplicates = false);
   ~BcmDocsisCfgVarBindEngine ();
   
    // Will return an error if this is a duplicate.  Note that if the Length
    // argument is the default -1, then we'll figure it out by parsing.
    // Preferred is to provide a length as a safety check.
    SNMP_STATUS AddVarBind (const unsigned char *pVbBuf, int Length = -1);
   
    // Go to it!
    SNMP_STATUS Process ();

    // Retrieve error info and statistics    
    SNMP_STATUS LastError () const { return fLastError; };
    inline int LastErrorIndex () const { return fLastErrorIndex; };
    inline const BcmString &LastErrorVarBind () const { return fLastErrorVarBind; };
    inline int NumVarBinds () const { return fNumVarBinds; };
    inline int NumDuplicates () const { return fNumDuplicates; };
    inline int NumUnknowns () const { return fNumUnknowns; };
    inline int NumWrongValues () const { return fNumWrongValues; };
    
  protected:
    SNMP_STATUS SendPacket (const BcmIpAddress &DestIp, unsigned short DestPort);
    VbRec *FindVarBind (int Index);
    SNMP_STATUS TranslateDocsisDraftToRFC (BcmObjectId &OID) const;
    
  protected:
    const BcmSnmpAgent *fpAgent;
    BcmString fVbName;
    bool fAllowDuplicates;
    list <VbRec *> fVarBinds;
    list <VbRec *> fUnknowns;
    unsigned short fSourcePort;
    unsigned short fRequestId;
    BcmString fCommunity;
    
    // Retrievable statistics:
    SNMP_STATUS fLastError;
    int fLastErrorIndex;
    BcmString fLastErrorVarBind;
    int fNumVarBinds;
    int fNumDuplicates;
    int fNumUnknowns;
    int fNumWrongValues;
};

#endif
