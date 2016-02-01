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
//    Filename: communityBridge.h
//    Creation Date: Thursday, June 20, 2002 at 12:57:29
//    Created by Broadcom V2 Mib Compiler BCMIBC.EXE version 1.2.0b
//    from input file rfc2576.mib
//
//**************************************************************************

#ifndef COMMUNITYBRIDGE_H
#define COMMUNITYBRIDGE_H

#define RFC2576_COEXISTENCE_VIEW "coexistenceView"

#if defined (__cplusplus)

#include "CoreObjs.h"
#include "MibObjs.h"
#include "MibBridge.h"

#include "communityMib.h"

class BcmSnmpV3Agent;

/*\
 *  snmpCommunityEntry 
\*/

#define MINLEN_SNMPCOMMUNITYINDEX  1
#define MAXLEN_SNMPCOMMUNITYINDEX  32
#define MINLEN_SNMPCOMMUNITYSECURITYNAME  1
#define MAXLEN_SNMPCOMMUNITYSECURITYNAME  32
#define MINLEN_SNMPCOMMUNITYCONTEXTENGINEID  5
#define MAXLEN_SNMPCOMMUNITYCONTEXTENGINEID  32
#define MINLEN_SNMPCOMMUNITYCONTEXTNAME  0
#define MAXLEN_SNMPCOMMUNITYCONTEXTNAME  32
#define MINLEN_SNMPCOMMUNITYTRANSPORTTAG  0
#define MAXLEN_SNMPCOMMUNITYTRANSPORTTAG  255
#define CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_OTHER  1
#define CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_VOLATILE  2
#define CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_NONVOLATILE  3
#define CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_PERMANENT  4
#define CONSTVAL_SNMPCOMMUNITYSTORAGETYPE_READONLY  5
#define CONSTVAL_SNMPCOMMUNITYSTATUS_ACTIVE  1
#define CONSTVAL_SNMPCOMMUNITYSTATUS_NOTINSERVICE  2
#define CONSTVAL_SNMPCOMMUNITYSTATUS_NOTREADY  3
#define CONSTVAL_SNMPCOMMUNITYSTATUS_CREATEANDWAIT  5
#define CONSTVAL_SNMPCOMMUNITYSTATUS_CREATEANDGO  4
#define CONSTVAL_SNMPCOMMUNITYSTATUS_DESTROY  6

class snmpCommunityEntryBridge : public TabularMibBridge
{
  friend class snmpCommunityEntry;

  public:
    snmpCommunityEntryBridge (const BcmString &snmpCommunityIndex, BcmSnmpV3Agent *pAgent);
    snmpCommunityEntryBridge (const BcmObjectId &Index, BcmSnmpV3Agent *pAgent);
   ~snmpCommunityEntryBridge ();

    /* GET methods */

    int Get_snmpCommunityIndex (BcmString &snmpCommunityIndex);
    int Get_snmpCommunityName (BcmString &snmpCommunityName);
    int Get_snmpCommunitySecurityName (BcmString &snmpCommunitySecurityName);
    int Get_snmpCommunityContextEngineID (BcmString &snmpCommunityContextEngineID);
    int Get_snmpCommunityContextName (BcmString &snmpCommunityContextName);
    int Get_snmpCommunityTransportTag (BcmString &snmpCommunityTransportTag);
    int Get_snmpCommunityStorageType ();
    int Get_snmpCommunityStatus ();

    /* TEST methods */

    SNMP_STATUS Test_snmpCommunityName (const BcmString &snmpCommunityName);
    SNMP_STATUS Test_snmpCommunitySecurityName (const BcmString &snmpCommunitySecurityName);
    SNMP_STATUS Test_snmpCommunityContextEngineID (const BcmString &snmpCommunityContextEngineID);
    SNMP_STATUS Test_snmpCommunityStorageType (int snmpCommunityStorageType);
    SNMP_STATUS Test_snmpCommunityStatus (int snmpCommunityStatus);

    /* SET methods */

    SNMP_STATUS Set_snmpCommunityName (const BcmString &snmpCommunityName);
    SNMP_STATUS Set_snmpCommunitySecurityName (const BcmString &snmpCommunitySecurityName);
    SNMP_STATUS Set_snmpCommunityContextEngineID (const BcmString &snmpCommunityContextEngineID);
    SNMP_STATUS Set_snmpCommunityContextName (const BcmString &snmpCommunityContextName);
    SNMP_STATUS Set_snmpCommunityTransportTag (const BcmString &snmpCommunityTransportTag);
    SNMP_STATUS Set_snmpCommunityStorageType (int snmpCommunityStorageType);
    SNMP_STATUS Set_snmpCommunityStatus (int &snmpCommunityStatus);
    
    /* Do some work */
    
    bool MatchesCommunity (const BcmString &Community);
    bool MatchesIp (const BcmIpAddress &Ip, int SrcPort, bool ApplyTMask = true);
    bool MatchesSecurityName (const BcmString &SecurityName);

  protected:
    int fRowStatus;

    enum
    {
      kBIT_snmpCommunityIndex = 1 << (snmpCommunityEntry::kLEAF_snmpCommunityIndex - 1),
      kBIT_snmpCommunityName = 1 << (snmpCommunityEntry::kLEAF_snmpCommunityName - 1),
      kBIT_snmpCommunitySecurityName = 1 << (snmpCommunityEntry::kLEAF_snmpCommunitySecurityName - 1),
      kBIT_snmpCommunityContextEngineID = 1 << (snmpCommunityEntry::kLEAF_snmpCommunityContextEngineID - 1),
      kBIT_snmpCommunityContextName = 1 << (snmpCommunityEntry::kLEAF_snmpCommunityContextName - 1),
      kBIT_snmpCommunityTransportTag = 1 << (snmpCommunityEntry::kLEAF_snmpCommunityTransportTag - 1),
      kBIT_snmpCommunityStorageType = 1 << (snmpCommunityEntry::kLEAF_snmpCommunityStorageType - 1),
      kBIT_snmpCommunityStatus = 1 << (snmpCommunityEntry::kLEAF_snmpCommunityStatus - 1),
    };
    
  private:
    BcmString    fCommunityName;
    BcmString    fSecurityName;
    BcmString    fEngineId;
    BcmString    fContext;
    BcmString    fTransportTag;
    int          fStorageType;
    BcmMibTable *fpTargetAddrExtTable;
};



/*\
 *  snmpTargetAddrExtEntry 
\*/

#define MINLEN_SNMPTARGETADDRTMASK  0
#define MAXLEN_SNMPTARGETADDRTMASK  255
#define MINVAL_SNMPTARGETADDRMMS  484
#define MAXVAL_SNMPTARGETADDRMMS  2147483647

class snmpTargetAddrExtEntryBridge : public TabularMibBridge
{
  friend class snmpTargetAddrExtEntry;

  public:
    snmpTargetAddrExtEntryBridge (const BcmString &snmpTargetAddrName, // index of snmpTargetAddrTable that this extends
                                  BcmSnmpV3Agent *pAgent);
    snmpTargetAddrExtEntryBridge (const BcmObjectId &Index, BcmSnmpV3Agent *pAgent);
   ~snmpTargetAddrExtEntryBridge ();

    /* GET methods */

    int Get_snmpTargetAddrTMask (BcmString &snmpTargetAddrTMask);
    int Get_snmpTargetAddrMMS ();

    /* TEST methods - not required because this is not a READ-CREATE table */
    
    SNMP_STATUS Test_snmpTargetAddrTMask (const BcmString &snmpTargetAddrTMask);


    /* SET methods */

    SNMP_STATUS Set_snmpTargetAddrTMask (const BcmString &snmpTargetAddrTMask);
    SNMP_STATUS Set_snmpTargetAddrMMS (int snmpTargetAddrMMS);
    
    /* Do some work */
    
    bool MatchesIp (const BcmString &TransportTag, const BcmIpAddress &Ip, int SrcPort, bool ApplyTMask = true);
    
  private:
    // Table columns:
    BcmIpAddress fTMaskIp;
    unsigned short fTMaskPort;
    int fMMS;
    
    // Our index re-formatted as a string for ease of use w/ snmpTargetAddrName
    BcmString fTAddrName;
    
};

#endif

#endif
