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
//  Filename:       DocsisCfgWacEngine.h
//  Author:         Kevin O'Neal
//  Creation Date:  August 29, 2003
//
//****************************************************************************
//  Description:
//      This class implements an engine to process SNMP write access control
//      elements form a DOCSIS config file (TLV-10 for CM and CableHome)
//
//****************************************************************************

#ifndef DOCSISCFGWACENGINE_H
#define DOCSISCFGWACENGINE_H

#include <list>

#include "CoreObjs.h"


class BcmDocsisCfgWacEngine
{
  public:
    BcmDocsisCfgWacEngine ();
    virtual ~BcmDocsisCfgWacEngine ();
   
    SNMP_STATUS AddWac (const unsigned char *pWacBuf, int Length);
    
    int NumWacs () const;
    void Flush ();
    
    SNMP_STATUS ValidateOid (const BcmObjectId &OID) const;
    
    virtual ostream &Print(ostream &outStream) const;
   
  protected:
    typedef struct _WacRec
    {
      BcmObjectId OID;
      bool        Access;
    } WacRec;
  
    list <WacRec *> fWacs;
};

inline ostream & operator << (ostream &outStream, const BcmDocsisCfgWacEngine &Engine)
{
  return Engine.Print(outStream);
}

#endif
