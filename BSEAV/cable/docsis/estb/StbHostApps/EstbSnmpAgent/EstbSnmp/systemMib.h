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
//    Filename: systemNewMib.h
//    Creation Date: Wednesday, February 04, 2004 at 15:32:40
//    Created by Broadcom BFC / V2 Mib Compiler BCMIBC.EXE version 3.0.0b
//    from input file rfc3418.mib
//
//**************************************************************************

#ifndef SYSTEMMIB_H
#define SYSTEMMIB_H

#include "MibObjs.h"
#include "MibBridge.h"

class systemGroupBridge;
class sysOREntryBridge;

/*\
 *  systemGroup group
\*/

class systemGroup : public BcmMibGroup
{
  friend class systemGroupBridge;

  public:
    systemGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    systemGroupBridge *fpBridge;

    enum
    {
      kLEAF_sysDescr = 1,
      kLEAF_sysObjectID = 2,
      kLEAF_sysUpTime = 3,
      kLEAF_sysContact = 4,
      kLEAF_sysName = 5,
      kLEAF_sysLocation = 6,
      kLEAF_sysServices = 7,
      kLEAF_sysORLastChange = 8
    };
};


/* sysORTable and related objects */

class sysORTable : public BcmMibTable
{
  public:
    sysORTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class sysOREntry : public BcmMibEntry
{
  friend class sysOREntryBridge;
  friend class sysORTable;

  public:
    sysOREntry (sysORTable *pTable, sysOREntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    sysOREntryBridge *fpBridge;

    enum
    {
      kLEAF_sysORIndex = 1,
      kLEAF_sysORID = 2,
      kLEAF_sysORDescr = 3,
      kLEAF_sysORUpTime = 4
    };
};

#endif
