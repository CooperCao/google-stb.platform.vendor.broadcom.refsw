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
//    Filename: estbDocsDevNewMib.h
//    Creation Date: 15 DEC 2006
//    Created by Broadcom BFC / V2 Mib Compiler BCMIBC.EXE version 3.0.0b
//    from input file ../docsis/rfc2669.mib
//
//**************************************************************************

#ifndef ESTBDOCSDEVMIB_H
#define ESTBDOCSDEVMIB_H

#include "MibObjs.h"
#include "MibBridge.h"

#include "docsDevTranslator.h"

// forward references:
class docsDevSoftwareGroupBridge;
class docsDevFilterGroupBridge;
class docsDevFilterIpEntryBridge;
class docsDevNmAccessTable;
class docsDevEventEntry;
class ifEntryBridge;



/*\
 *  docsDevGroup group
\*/

class docsDevGroup : public docsDevMibGroup
{
  public:
    docsDevGroup ();

    bool Populate ();
};



/*\
 *  docsDevMIBObjectsGroup group
\*/

class docsDevMIBObjectsGroup : public docsDevMibGroup
{
  public:
    docsDevMIBObjectsGroup ();

    bool Populate ();
};


/*\
 *  docsDevSoftwareGroup group
\*/

class docsDevSoftwareGroup : public docsDevMibGroup
{
  friend class docsDevSoftwareGroupBridge;

  public:
    docsDevSoftwareGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    docsDevSoftwareGroupBridge *fpBridge;

    enum
    {
      kLEAF_docsDevSwServer = 1,
      kLEAF_docsDevSwFilename = 2,
      kLEAF_docsDevSwAdminStatus = 3,
      kLEAF_docsDevSwOperStatus = 4,
      kLEAF_docsDevSwCurrentVers = 5
    };
};

/*\
 *  docsDevFilterGroup group
\*/

class docsDevFilterGroup : public docsDevMibGroup
{
  friend class docsDevFilterGroupBridge;

  public:
    docsDevFilterGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    docsDevFilterGroupBridge *fpBridge;

    enum
    {
      kLEAF_docsDevFilterIpDefault = 3
    };
};

/* docsDevFilterIpTable and related objects */

class docsDevFilterIpTable : public docsDevMibTable
{
  public:
    docsDevFilterIpTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
    BcmMibEntry *NewDefaultEntry (BcmObjectId &Index);
    SNMP_STATUS DestroyEntry (BcmObjectId &OID);
};

// Forward reference:
class IpFilter;
class docsDevFilterIpEntry : public docsDevMibEntry
{
  friend class docsDevFilterIpEntryBridge;
  friend class docsDevFilterIpTable;

  public:
    docsDevFilterIpEntry (docsDevFilterIpTable *pTable, docsDevFilterIpEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS TestLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);


    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
    bool IsAccessible (BcmMibRequest *pReq = NULL) const;
    inline IpFilter *GetFilter () const;

  protected:
    docsDevFilterIpEntryBridge *fpBridge;
    IpFilter *fpFilter;
    ifEntryBridge *fpUsbIf;

    enum
    {
      kLEAF_docsDevFilterIpIndex = 1,
      kLEAF_docsDevFilterIpStatus = 2,
      kLEAF_docsDevFilterIpControl = 3,
      kLEAF_docsDevFilterIpIfIndex = 4,
      kLEAF_docsDevFilterIpDirection = 5,
      kLEAF_docsDevFilterIpBroadcast = 6,
      kLEAF_docsDevFilterIpSaddr = 7,
      kLEAF_docsDevFilterIpSmask = 8,
      kLEAF_docsDevFilterIpDaddr = 9,
      kLEAF_docsDevFilterIpDmask = 10,
      kLEAF_docsDevFilterIpProtocol = 11,
      kLEAF_docsDevFilterIpSourcePortLow = 12,
      kLEAF_docsDevFilterIpSourcePortHigh = 13,
      kLEAF_docsDevFilterIpDestPortLow = 14,
      kLEAF_docsDevFilterIpDestPortHigh = 15,
      kLEAF_docsDevFilterIpMatches = 16,
      kLEAF_docsDevFilterIpTos = 17,
      kLEAF_docsDevFilterIpTosMask = 18,
      kLEAF_docsDevFilterIpContinue = 19,
      kLEAF_docsDevFilterIpPolicyId = 20
    };
};

inline IpFilter *docsDevFilterIpEntry::GetFilter () const
{
  return fpFilter;
}



#endif
