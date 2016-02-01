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
//    Filename: communityNewMib.h
//    Creation Date: Wednesday, February 04, 2004 at 15:30:51
//    Created by Broadcom BFC / V2 Mib Compiler BCMIBC.EXE version 3.0.0b
//    from input file rfc2576.mib
//
//**************************************************************************

#ifndef COMMUNITYMIB_H
#define COMMUNITYMIB_H

#include "MibObjs.h"
#include "MibBridge.h"

class snmpCommunityEntryBridge;
class snmpTargetAddrExtEntryBridge;

/*\
 *  snmpCommunityMIBGroup group
\*/

class snmpCommunityMIBGroup : public BcmMibGroup
{
  public:
    snmpCommunityMIBGroup ();

    bool Populate ();
};



/*\
 *  snmpCommunityMIBObjectsGroup group
\*/

class snmpCommunityMIBObjectsGroup : public BcmMibGroup
{
  friend class snmpCommunityMIBObjectsGroupBridge;

  public:
    snmpCommunityMIBObjectsGroup ();

    bool Populate ();
};


/* snmpCommunityTable and related objects */

class snmpCommunityTable : public BcmMibTable
{
  public:
    snmpCommunityTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
    BcmMibEntry *NewDefaultEntry (BcmObjectId &Index);
    
    SNMP_STATUS ValidateRequest (const BcmString &Community, const BcmIpAddress &SrcIp, int SrcPort, BcmString &SecurityName);
    SNMP_STATUS ResolveSecNameToCommName (BcmString &Community, const BcmIpAddress &SrcIp, BcmString &SecurityName, int TrapPort);

};

class snmpCommunityEntry : public BcmMibEntry
{
  friend class snmpCommunityEntryBridge;
  friend class snmpCommunityTable;

  public:
    snmpCommunityEntry (snmpCommunityTable *pTable, snmpCommunityEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS TestLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    snmpCommunityEntryBridge *fpBridge;

    enum
    {
      kLEAF_snmpCommunityIndex = 1,
      kLEAF_snmpCommunityName = 2,
      kLEAF_snmpCommunitySecurityName = 3,
      kLEAF_snmpCommunityContextEngineID = 4,
      kLEAF_snmpCommunityContextName = 5,
      kLEAF_snmpCommunityTransportTag = 6,
      kLEAF_snmpCommunityStorageType = 7,
      kLEAF_snmpCommunityStatus = 8
    };
};


/* snmpTargetAddrExtTable and related objects */

class snmpTargetAddrExtTable : public BcmMibTable
{
  public:
    snmpTargetAddrExtTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
    BcmMibEntry *NewDefaultEntry (BcmObjectId &Index);
};

class snmpTargetAddrExtEntry : public BcmMibEntry
{
  friend class snmpTargetAddrExtEntryBridge;
  friend class snmpTargetAddrExtTable;
  friend class snmpCommunityEntryBridge;

  public:
    snmpTargetAddrExtEntry (snmpTargetAddrExtTable *pTable, snmpTargetAddrExtEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS TestLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    
    bool IsVisible () const;

  protected:
    snmpTargetAddrExtEntryBridge *fpBridge;

    enum
    {
      kLEAF_snmpTargetAddrTMask = 1,
      kLEAF_snmpTargetAddrMMS = 2
    };
};



#endif
