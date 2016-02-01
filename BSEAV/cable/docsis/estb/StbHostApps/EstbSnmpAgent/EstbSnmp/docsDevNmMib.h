//**************************************************************************
//                                                                          
//    Copyright 2004 Broadcom Corporation                                   
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
//    Description:                                                          
//                                                                          
//**************************************************************************
//    Revision History:                                                     
//                                                                          
//**************************************************************************
//    Filename: docsDevNmMib.h
//    Creation Date: Wednesday, February 04, 2004 at 15:43:07
//    Created by Broadcom BFC / V2 Mib Compiler BCMIBC.EXE version 3.0.0b
//    from input file rfc2669.mib
//
//**************************************************************************

#ifndef DOCSDEVNMMIB_H
#define DOCSDEVNMMIB_H

#include "MibObjs.h"
#include "MibBridge.h"
#include "BcmTrap.h"
#include "docsDevTranslator.h"


/* docsDevNmAccessTable and related objects */

// Forward reference:
class NmAccessController;
class docsDevNmAccessEntryBridge;


class docsDevNmAccessTable : public docsDevMibTable
{
  public:
    docsDevNmAccessTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
    docsDevMibEntry *NewDefaultEntry (BcmObjectId &Index);
    SNMP_STATUS DestroyEntry (BcmObjectId &OID);
    
    SNMP_STATUS ValidateRequest (int RequestType, int SrcIfNum, const BcmIpAddress &SrcIp, const BcmString &Community);
    int SendTrap (BcmTrap *pTrap, const BcmIpAddress *pSrcIp = NULL);
    
    NmAccessController *FirstNmController ();
    NmAccessController *NextNmController (NmAccessController *PrevNm);
};

class docsDevNmAccessEntry : public docsDevMibEntry
{
  friend class docsDevNmAccessEntryBridge;
  friend class docsDevNmAccessTable;

  public:
    docsDevNmAccessEntry (docsDevNmAccessTable *pTable, docsDevNmAccessEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS TestLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    
    bool IsAccessible (BcmMibRequest *pReq = NULL) const;
    NmAccessController *GetController () const;
    virtual ostream &Print(ostream &outStream);

  protected:
    docsDevNmAccessEntryBridge *fpBridge;
    docsDevNmAccessTable *fpNmTable;

    enum
    {
      kLEAF_docsDevNmAccessIndex = 1,
      kLEAF_docsDevNmAccessIp = 2,
      kLEAF_docsDevNmAccessIpMask = 3,
      kLEAF_docsDevNmAccessCommunity = 4,
      kLEAF_docsDevNmAccessControl = 5,
      kLEAF_docsDevNmAccessInterfaces = 6,
      kLEAF_docsDevNmAccessStatus = 7,
      kLEAF_docsDevNmAccessTrapVersion = 8
    };
};


inline ostream & operator << (ostream &outStream, docsDevNmAccessEntry &NmEntry)
{
    return NmEntry.Print(outStream);
}

#endif
