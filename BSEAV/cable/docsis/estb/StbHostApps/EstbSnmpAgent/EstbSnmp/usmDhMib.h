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
//    Filename: usmDhNewMib.h
//    Creation Date: Wednesday, February 04, 2004 at 15:34:16
//    Created by Broadcom BFC / V2 Mib Compiler BCMIBC.EXE version 3.0.0b
//    from input file rfc2786.mib
//
//**************************************************************************

#ifndef USMDHMIB_H
#define USMDHMIB_H

#include "MibObjs.h"
#include "MibBridge.h"

class usmDHPublicObjectsGroupBridge;
class usmDHUserKeyEntryBridge;
class usmDHKickstartEntryBridge;

/*\
 *  snmpUsmDHObjectsMIBGroup group
\*/

class snmpUsmDHObjectsMIBGroup : public BcmMibGroup
{
  public:
    snmpUsmDHObjectsMIBGroup ();

    bool Populate ();
};



/*\
 *  usmDHKeyObjectsGroup group
\*/

class usmDHKeyObjectsGroup : public BcmMibGroup
{
  public:
    usmDHKeyObjectsGroup ();

    bool Populate ();
};



/*\
 *  usmDHPublicObjectsGroup group
\*/

class usmDHPublicObjectsGroup : public BcmMibGroup
{
  friend class usmDHPublicObjectsGroupBridge;

  public:
    usmDHPublicObjectsGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    usmDHPublicObjectsGroupBridge *fpBridge;

    enum
    {
      kLEAF_usmDHParameters = 1
    };
};


/* usmDHUserKeyTable and related objects */

class usmDHUserKeyTable : public BcmMibTable
{
  public:
    usmDHUserKeyTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class usmDHUserKeyEntry : public BcmMibEntry
{
  friend class usmDHUserKeyEntryBridge;
  friend class usmDHUserKeyTable;

  public:
    usmDHUserKeyEntry (usmDHUserKeyTable *pTable, usmDHUserKeyEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    usmDHUserKeyEntryBridge *fpBridge;

    enum
    {
      kLEAF_usmDHUserAuthKeyChange = 1,
      kLEAF_usmDHUserOwnAuthKeyChange = 2,
      kLEAF_usmDHUserPrivKeyChange = 3,
      kLEAF_usmDHUserOwnPrivKeyChange = 4
    };
};



/*\
 *  usmDHKickstartGroup group
\*/

class usmDHKickstartGroup : public BcmMibGroup
{
  public:
    usmDHKickstartGroup ();

    bool Populate ();
};


/* usmDHKickstartTable and related objects */

class usmDHKickstartTable : public BcmMibTable
{
  public:
    usmDHKickstartTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class usmDHKickstartEntry : public BcmMibEntry
{
  friend class usmDHKickstartEntryBridge;
  friend class usmDHKickstartTable;

  public:
    usmDHKickstartEntry (usmDHKickstartTable *pTable, usmDHKickstartEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    usmDHKickstartEntryBridge *fpBridge;

    enum
    {
      kLEAF_usmDHKickstartIndex = 1,
      kLEAF_usmDHKickstartMyPublic = 2,
      kLEAF_usmDHKickstartMgrPublic = 3,
      kLEAF_usmDHKickstartSecurityName = 4
    };
};



#endif
