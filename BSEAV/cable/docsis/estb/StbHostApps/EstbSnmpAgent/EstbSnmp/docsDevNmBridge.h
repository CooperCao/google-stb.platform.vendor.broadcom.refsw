//**************************************************************************
//                                                                          
//    Copyright 2001 Broadcom Corporation                                   
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
//    Filename: docsDevNmBridge.h
//    Creation Date: Monday, May 20, 2002 at 14:46:50
//    Created by Broadcom V2 Mib Compiler BCMIBC.EXE version 1.2.0b from input file rfc2669.mib
//
//**************************************************************************

#ifndef DOCSDEVNMBRIDGE_H
#define DOCSDEVNMBRIDGE_H


#include "CoreObjs.h"
#include "MibObjs.h"
#include "MibBridge.h"

#include "docsDevNmMib.h"


/*\
 *  docsDevNmAccessEntry 
\*/

#define MINVAL_DOCSDEVNMACCESSINDEX  1
#define MAXVAL_DOCSDEVNMACCESSINDEX  2147483647
#define CONSTVAL_DOCSDEVNMACCESSCONTROL_NONE  1
#define CONSTVAL_DOCSDEVNMACCESSCONTROL_READ  2
#define CONSTVAL_DOCSDEVNMACCESSCONTROL_READWRITE  3
#define CONSTVAL_DOCSDEVNMACCESSCONTROL_ROWITHTRAPS  4
#define CONSTVAL_DOCSDEVNMACCESSCONTROL_RWWITHTRAPS  5
#define CONSTVAL_DOCSDEVNMACCESSCONTROL_TRAPSONLY  6
#define CONSTVAL_DOCSDEVNMACCESSSTATUS_ACTIVE  1
#define CONSTVAL_DOCSDEVNMACCESSSTATUS_NOTINSERVICE  2
#define CONSTVAL_DOCSDEVNMACCESSSTATUS_NOTREADY  3
#define CONSTVAL_DOCSDEVNMACCESSSTATUS_CREATEANDWAIT  5
#define CONSTVAL_DOCSDEVNMACCESSSTATUS_CREATEANDGO  4
#define CONSTVAL_DOCSDEVNMACCESSSTATUS_DESTROY  6
#define CONSTVAL_DOCSDEVNMACCESSTRAPVERSION_DISABLESNMPV2TRAP  1
#define CONSTVAL_DOCSDEVNMACCESSTRAPVERSION_ENABLESNMPV2TRAP  2

class NmAccessController;
class docsDevNmAccessEntryBridge : public TabularMibBridge
{
  friend class docsDevNmAccessEntry;

  public:
    docsDevNmAccessEntryBridge (int docsDevNmAccessIndex, BcmSnmpAgent *pAgent);
    docsDevNmAccessEntryBridge (const BcmObjectId &Index, BcmSnmpAgent *pAgent);
   ~docsDevNmAccessEntryBridge ();

    /* GET methods */

    int Get_docsDevNmAccessIndex ();
    void Get_docsDevNmAccessIp (BcmIpAddress &docsDevNmAccessIp);
    void Get_docsDevNmAccessIpMask (BcmIpAddress &docsDevNmAccessIpMask);
    int Get_docsDevNmAccessCommunity (BcmString &docsDevNmAccessCommunity);
    int Get_docsDevNmAccessControl ();
    unsigned long Get_docsDevNmAccessInterfaces ();
    int Get_docsDevNmAccessStatus ();
    int Get_docsDevNmAccessTrapVersion ();

    /* TEST methods */

    SNMP_STATUS Test_docsDevNmAccessIp (const BcmIpAddress &docsDevNmAccessIp);
    SNMP_STATUS Test_docsDevNmAccessIpMask (const BcmIpAddress &docsDevNmAccessIpMask);
    SNMP_STATUS Test_docsDevNmAccessCommunity (const BcmString &docsDevNmAccessCommunity);
    SNMP_STATUS Test_docsDevNmAccessControl (int docsDevNmAccessControl);
    SNMP_STATUS Test_docsDevNmAccessInterfaces (unsigned long docsDevNmAccessInterfaces);
    SNMP_STATUS Test_docsDevNmAccessStatus (int docsDevNmAccessStatus);
    SNMP_STATUS Test_docsDevNmAccessTrapVersion (int docsDevNmAccessTrapVersion);

    /* SET methods */

    SNMP_STATUS Set_docsDevNmAccessIp (const BcmIpAddress &docsDevNmAccessIp);
    SNMP_STATUS Set_docsDevNmAccessIpMask (const BcmIpAddress &docsDevNmAccessIpMask);
    SNMP_STATUS Set_docsDevNmAccessCommunity (const BcmString &docsDevNmAccessCommunity);
    SNMP_STATUS Set_docsDevNmAccessControl (int docsDevNmAccessControl);
    SNMP_STATUS Set_docsDevNmAccessInterfaces (unsigned long docsDevNmAccessInterfaces);
    SNMP_STATUS Set_docsDevNmAccessStatus (int &docsDevNmAccessStatus);
    SNMP_STATUS Set_docsDevNmAccessTrapVersion (int docsDevNmAccessTrapVersion);


  protected:
    int fRowStatus;
    int fTestControl;
    BcmIpAddress fTestIpMask;
    NmAccessController *fpController;

    enum
    {
      kBIT_docsDevNmAccessIndex = 1 << (docsDevNmAccessEntry::kLEAF_docsDevNmAccessIndex - 1),
      kBIT_docsDevNmAccessIp = 1 << (docsDevNmAccessEntry::kLEAF_docsDevNmAccessIp - 1),
      kBIT_docsDevNmAccessIpMask = 1 << (docsDevNmAccessEntry::kLEAF_docsDevNmAccessIpMask - 1),
      kBIT_docsDevNmAccessCommunity = 1 << (docsDevNmAccessEntry::kLEAF_docsDevNmAccessCommunity - 1),
      kBIT_docsDevNmAccessControl = 1 << (docsDevNmAccessEntry::kLEAF_docsDevNmAccessControl - 1),
      kBIT_docsDevNmAccessInterfaces = 1 << (docsDevNmAccessEntry::kLEAF_docsDevNmAccessInterfaces - 1),
      kBIT_docsDevNmAccessStatus = 1 << (docsDevNmAccessEntry::kLEAF_docsDevNmAccessStatus - 1),
      kBIT_docsDevNmAccessTrapVersion = 1 << (docsDevNmAccessEntry::kLEAF_docsDevNmAccessTrapVersion - 1),
    };
};

#endif
