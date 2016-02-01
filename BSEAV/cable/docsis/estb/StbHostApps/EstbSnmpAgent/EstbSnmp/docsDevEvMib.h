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
//    Filename: docsDevEvMib.h
//    Creation Date: Wednesday, February 04, 2004 at 15:43:07
//    Created by Broadcom BFC / V2 Mib Compiler BCMIBC.EXE version 3.0.0b
//    from input file rfc2669.mib
//
//**************************************************************************

#ifndef DOCSDEVEVMIB_H
#define DOCSDEVEVMIB_H

#include "MibObjs.h"
#include "MibBridge.h"

#include "docsDevTranslator.h"
#include "docsDevEvBridge.h"

/*\
 *  docsDevEventGroup group
\*/

class docsDevEventGroup : public docsDevMibGroup
{
  friend class docsDevEventGroupBridge;

  public:
    docsDevEventGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    docsDevEventGroupBridge *fpBridge;

    enum
    {
      kLEAF_docsDevEvControl = 1,
      kLEAF_docsDevEvSyslog = 2,
      kLEAF_docsDevEvThrottleAdminStatus = 3,
      kLEAF_docsDevEvThrottleInhibited = 4,
      kLEAF_docsDevEvThrottleThreshold = 5,
      kLEAF_docsDevEvThrottleInterval = 6
    };
};


/* docsDevEvControlTable and related objects */

class docsDevEvControlTable : public docsDevMibTable
{
  public:
    docsDevEvControlTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class docsDevEvControlEntry : public docsDevMibEntry
{
  friend class docsDevEvControlEntryBridge;
  friend class docsDevEvControlTable;

  public:
    docsDevEvControlEntry (docsDevEvControlTable *pTable, docsDevEvControlEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    docsDevEvControlEntryBridge *fpBridge;

    enum
    {
      kLEAF_docsDevEvPriority = 1,
      kLEAF_docsDevEvReporting = 2
    };
};


/* docsDevEventTable and related objects */

class docsDevEventTable : public docsDevMibTable
{
  public:
    docsDevEventTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class docsDevEventEntry : public docsDevMibEntry
{
  friend class docsDevEventEntryBridge;
  friend class docsDevEventTable;

  public:
    docsDevEventEntry (docsDevEventTable *pTable, docsDevEventEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    bool IsAccessible (BcmMibRequest *pReq = NULL) const;
    
    inline EventLogEntry *GetEvent ();

  protected:
    docsDevEventEntryBridge *fpBridge;

    enum
    {
      kLEAF_docsDevEvIndex = 1,
      kLEAF_docsDevEvFirstTime = 2,
      kLEAF_docsDevEvLastTime = 3,
      kLEAF_docsDevEvCounts = 4,
      kLEAF_docsDevEvLevel = 5,
      kLEAF_docsDevEvId = 6,
      kLEAF_docsDevEvText = 7
    };
};

inline EventLogEntry *docsDevEventEntry::GetEvent ()
{
  return fpBridge->GetEvent();
}

#endif
