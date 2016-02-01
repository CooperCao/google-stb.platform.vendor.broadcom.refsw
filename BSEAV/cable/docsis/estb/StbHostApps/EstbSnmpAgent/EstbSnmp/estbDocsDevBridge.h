//**************************************************************************
//                                                                          
//    Copyright 2006 Broadcom Corporation                                   
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
//    Filename: estbDocsDevBridge.h
//    Creation Date: 15 DEC 2006
//    Created by BCMIBC.EXE version 1.1.0b from input file rfc2669.mib
//
//**************************************************************************

#ifndef ESTBDOCSDEVBRIDGE_H
#define ESTBDOCSDEVBRIDGE_H


#include "CoreObjs.h"
#include "MibObjs.h"
#include "MibBridge.h"
#include "NmAccess.h"
#include "docsDevTrap.h"
#include "EventLog.h"

#include "estbDocsDevMib.h"

// forward reference:
class BcmSnmpAgent;
// class BcmCmDocsisCtlThread;


/*\
 *  docsDevSoftwareGroup 
\*/

#define MINLEN_DOCSDEVSWFILENAME  0
#define MAXLEN_DOCSDEVSWFILENAME  64
#define CONSTVAL_DOCSDEVSWADMINSTATUS_UPGRADEFROMMGT  1
#define CONSTVAL_DOCSDEVSWADMINSTATUS_ALLOWPROVISIONINGUPGRADE  2
#define CONSTVAL_DOCSDEVSWADMINSTATUS_IGNOREPROVISIONINGUPGRADE  3
#define CONSTVAL_DOCSDEVSWOPERSTATUS_INPROGRESS  1
#define CONSTVAL_DOCSDEVSWOPERSTATUS_COMPLETEFROMPROVISIONING  2
#define CONSTVAL_DOCSDEVSWOPERSTATUS_COMPLETEFROMMGT  3
#define CONSTVAL_DOCSDEVSWOPERSTATUS_FAILED  4
#define CONSTVAL_DOCSDEVSWOPERSTATUS_OTHER  5
#define MINLEN_DOCSDEVSWCURRENTVERS  0
#define MAXLEN_DOCSDEVSWCURRENTVERS  255

class docsDevSoftwareGroupBridge : public ScalarMibBridge
{
  friend class docsDevSoftwareGroup;

  public:
    // docsDevSoftwareGroupBridge (BcmCmDocsisCtlThread *pFriend, BcmSnmpAgent *pAgent);
    docsDevSoftwareGroupBridge (BcmSnmpAgent *pAgent);
   ~docsDevSoftwareGroupBridge ();

    /* GET methods */

    virtual void Get_docsDevSwServer (BcmIpAddress &docsDevSwServer);
    virtual int Get_docsDevSwFilename (BcmString &docsDevSwFilename);
    virtual int Get_docsDevSwAdminStatus ();
    virtual int Get_docsDevSwOperStatus ();
    virtual int Get_docsDevSwCurrentVers (BcmString &docsDevSwCurrentVers);

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

    virtual SNMP_STATUS Set_docsDevSwServer (const BcmIpAddress &docsDevSwServer);
    virtual SNMP_STATUS Set_docsDevSwFilename (const BcmString &docsDevSwFilename);
    virtual SNMP_STATUS Set_docsDevSwAdminStatus (int docsDevSwAdminStatus);
    virtual SNMP_STATUS Set_docsDevSwCurrentVers (const BcmString &docsDevSwCurrentVers);
    
  protected:
    // BcmCmDocsisCtlThread *pfFriend;
    // BcmCmDocsisCtlThread *pfAgent;
};

/*\
 *  docsDevFilterGroup
\*/

#define CONSTVAL_DOCSDEVFILTERIPDEFAULT_DISCARD  1
#define CONSTVAL_DOCSDEVFILTERIPDEFAULT_ACCEPT  2

class docsDevFilterGroupBridge : public ScalarMibBridge
{
  friend class docsDevFilterGroup;

  public:
    docsDevFilterGroupBridge (BcmSnmpAgent *pAgent);
   ~docsDevFilterGroupBridge ();

    /* GET methods */

    int Get_docsDevFilterIpDefault ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

    SNMP_STATUS Set_docsDevFilterIpDefault (int docsDevFilterIpDefault);
};


/*\
 *  docsDevFilterIpEntry
\*/

#define MINVAL_DOCSDEVFILTERIPINDEX  1
#define MAXVAL_DOCSDEVFILTERIPINDEX  2147483647
#define CONSTVAL_DOCSDEVFILTERIPSTATUS_ACTIVE  1
#define CONSTVAL_DOCSDEVFILTERIPSTATUS_NOTINSERVICE  2
#define CONSTVAL_DOCSDEVFILTERIPSTATUS_NOTREADY  3
#define CONSTVAL_DOCSDEVFILTERIPSTATUS_CREATEANDWAIT  5
#define CONSTVAL_DOCSDEVFILTERIPSTATUS_CREATEANDGO  4
#define CONSTVAL_DOCSDEVFILTERIPSTATUS_DESTROY  6
#define CONSTVAL_DOCSDEVFILTERIPCONTROL_DISCARD  1
#define CONSTVAL_DOCSDEVFILTERIPCONTROL_ACCEPT  2
#define CONSTVAL_DOCSDEVFILTERIPCONTROL_POLICY  3
#define MINVAL_DOCSDEVFILTERIPIFINDEX  0
#define MAXVAL_DOCSDEVFILTERIPIFINDEX  2147483647
#define CONSTVAL_DOCSDEVFILTERIPDIRECTION_INBOUND  1
#define CONSTVAL_DOCSDEVFILTERIPDIRECTION_OUTBOUND  2
#define CONSTVAL_DOCSDEVFILTERIPDIRECTION_BOTH  3
#define CONSTVAL_DOCSDEVFILTERIPBROADCAST_TRUE  1
#define CONSTVAL_DOCSDEVFILTERIPBROADCAST_FALSE  2
#define MINVAL_DOCSDEVFILTERIPPROTOCOL  0
#define MAXVAL_DOCSDEVFILTERIPPROTOCOL  256
#define MINVAL_DOCSDEVFILTERIPSOURCEPORTLOW  0
#define MAXVAL_DOCSDEVFILTERIPSOURCEPORTLOW  65535
#define MINVAL_DOCSDEVFILTERIPSOURCEPORTHIGH  0
#define MAXVAL_DOCSDEVFILTERIPSOURCEPORTHIGH  65535
#define MINVAL_DOCSDEVFILTERIPDESTPORTLOW  0
#define MAXVAL_DOCSDEVFILTERIPDESTPORTLOW  65535
#define MINVAL_DOCSDEVFILTERIPDESTPORTHIGH  0
#define MAXVAL_DOCSDEVFILTERIPDESTPORTHIGH  65535
#define CONSTLEN_DOCSDEVFILTERIPTOS  1
#define CONSTLEN_DOCSDEVFILTERIPTOSMASK  1
#define CONSTVAL_DOCSDEVFILTERIPCONTINUE_TRUE  1
#define CONSTVAL_DOCSDEVFILTERIPCONTINUE_FALSE  2
#define MINVAL_DOCSDEVFILTERIPPOLICYID  0
#define MAXVAL_DOCSDEVFILTERIPPOLICYID  2147483647

class IpFilter;
class docsDevFilterIpEntryBridge : public TabularMibBridge
{
  public:
    docsDevFilterIpEntryBridge (int Index, BcmSnmpAgent *pAgent);
   ~docsDevFilterIpEntryBridge ();

    /* GET methods */

    int Get_docsDevFilterIpIndex ();
    int Get_docsDevFilterIpStatus ();
    int Get_docsDevFilterIpControl ();
    int Get_docsDevFilterIpIfIndex ();
    int Get_docsDevFilterIpDirection ();
    bool Get_docsDevFilterIpBroadcast ();
    void Get_docsDevFilterIpSaddr (BcmIpAddress &docsDevFilterIpSaddr);
    void Get_docsDevFilterIpSmask (BcmIpAddress &docsDevFilterIpSmask);
    void Get_docsDevFilterIpDaddr (BcmIpAddress &docsDevFilterIpDaddr);
    void Get_docsDevFilterIpDmask (BcmIpAddress &docsDevFilterIpDmask);
    int Get_docsDevFilterIpProtocol ();
    int Get_docsDevFilterIpSourcePortLow ();
    int Get_docsDevFilterIpSourcePortHigh ();
    int Get_docsDevFilterIpDestPortLow ();
    int Get_docsDevFilterIpDestPortHigh ();
    unsigned int Get_docsDevFilterIpMatches ();
    int Get_docsDevFilterIpTos (BcmString &docsDevFilterIpTos);
    int Get_docsDevFilterIpTosMask (BcmString &docsDevFilterIpTosMask);
    bool Get_docsDevFilterIpContinue ();
    int Get_docsDevFilterIpPolicyId ();

    /* TEST methods */

    SNMP_STATUS Test_docsDevFilterIpStatus (int docsDevFilterIpStatus);

    /* SET methods */

    SNMP_STATUS Set_docsDevFilterIpStatus (int &docsDevFilterIpStatus);
    SNMP_STATUS Set_docsDevFilterIpControl (int docsDevFilterIpControl);
    SNMP_STATUS Set_docsDevFilterIpIfIndex (int docsDevFilterIpIfIndex);
    SNMP_STATUS Set_docsDevFilterIpDirection (int docsDevFilterIpDirection);
    SNMP_STATUS Set_docsDevFilterIpBroadcast (bool docsDevFilterIpBroadcast);
    SNMP_STATUS Set_docsDevFilterIpSaddr (const BcmIpAddress &docsDevFilterIpSaddr);
    SNMP_STATUS Set_docsDevFilterIpSmask (const BcmIpAddress &docsDevFilterIpSmask);
    SNMP_STATUS Set_docsDevFilterIpDaddr (const BcmIpAddress &docsDevFilterIpDaddr);
    SNMP_STATUS Set_docsDevFilterIpDmask (const BcmIpAddress &docsDevFilterIpDmask);
    SNMP_STATUS Set_docsDevFilterIpProtocol (int docsDevFilterIpProtocol);
    SNMP_STATUS Set_docsDevFilterIpSourcePortLow (int docsDevFilterIpSourcePortLow);
    SNMP_STATUS Set_docsDevFilterIpSourcePortHigh (int docsDevFilterIpSourcePortHigh);
    SNMP_STATUS Set_docsDevFilterIpDestPortLow (int docsDevFilterIpDestPortLow);
    SNMP_STATUS Set_docsDevFilterIpDestPortHigh (int docsDevFilterIpDestPortHigh);
    SNMP_STATUS Set_docsDevFilterIpTos (const BcmString &docsDevFilterIpTos);
    SNMP_STATUS Set_docsDevFilterIpTosMask (const BcmString &docsDevFilterIpTosMask);
    SNMP_STATUS Set_docsDevFilterIpContinue (bool docsDevFilterIpContinue);
    SNMP_STATUS Set_docsDevFilterIpPolicyId (int docsDevFilterIpPolicyId);

    IpFilter *fpFilter;

  protected:
    enum
    {
      kBIT_docsDevFilterIpIndex = 1 << (docsDevFilterIpEntry::kLEAF_docsDevFilterIpIndex - 1),
      kBIT_docsDevFilterIpStatus = 1 << (docsDevFilterIpEntry::kLEAF_docsDevFilterIpStatus - 1),
      kBIT_docsDevFilterIpControl = 1 << (docsDevFilterIpEntry::kLEAF_docsDevFilterIpControl - 1),
      kBIT_docsDevFilterIpIfIndex = 1 << (docsDevFilterIpEntry::kLEAF_docsDevFilterIpIfIndex - 1),
      kBIT_docsDevFilterIpDirection = 1 << (docsDevFilterIpEntry::kLEAF_docsDevFilterIpDirection - 1),
      kBIT_docsDevFilterIpBroadcast = 1 << (docsDevFilterIpEntry::kLEAF_docsDevFilterIpBroadcast - 1),
      kBIT_docsDevFilterIpSaddr = 1 << (docsDevFilterIpEntry::kLEAF_docsDevFilterIpSaddr - 1),
      kBIT_docsDevFilterIpSmask = 1 << (docsDevFilterIpEntry::kLEAF_docsDevFilterIpSmask - 1),
      kBIT_docsDevFilterIpDaddr = 1 << (docsDevFilterIpEntry::kLEAF_docsDevFilterIpDaddr - 1),
      kBIT_docsDevFilterIpDmask = 1 << (docsDevFilterIpEntry::kLEAF_docsDevFilterIpDmask - 1),
      kBIT_docsDevFilterIpProtocol = 1 << (docsDevFilterIpEntry::kLEAF_docsDevFilterIpProtocol - 1),
      kBIT_docsDevFilterIpSourcePortLow = 1 << (docsDevFilterIpEntry::kLEAF_docsDevFilterIpSourcePortLow - 1),
      kBIT_docsDevFilterIpSourcePortHigh = 1 << (docsDevFilterIpEntry::kLEAF_docsDevFilterIpSourcePortHigh - 1),
      kBIT_docsDevFilterIpDestPortLow = 1 << (docsDevFilterIpEntry::kLEAF_docsDevFilterIpDestPortLow - 1),
      kBIT_docsDevFilterIpDestPortHigh = 1 << (docsDevFilterIpEntry::kLEAF_docsDevFilterIpDestPortHigh - 1),
      kBIT_docsDevFilterIpMatches = 1 << (docsDevFilterIpEntry::kLEAF_docsDevFilterIpMatches - 1),
      kBIT_docsDevFilterIpTos = 1 << (docsDevFilterIpEntry::kLEAF_docsDevFilterIpTos - 1),
      kBIT_docsDevFilterIpTosMask = 1 << (docsDevFilterIpEntry::kLEAF_docsDevFilterIpTosMask - 1),
      kBIT_docsDevFilterIpContinue = 1 << (docsDevFilterIpEntry::kLEAF_docsDevFilterIpContinue - 1),
      kBIT_docsDevFilterIpPolicyId = 1 << (docsDevFilterIpEntry::kLEAF_docsDevFilterIpPolicyId - 1),
    };

    friend class docsDevFilterIpEntry;
};







#endif
