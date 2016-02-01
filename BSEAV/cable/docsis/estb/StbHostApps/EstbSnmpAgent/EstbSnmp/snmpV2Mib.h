//**************************************************************************
//
//    Copyright 2000 Broadcom Corporation
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
//    Filename: snmpV2.h
//    Author:   Kevin O'Neal
//    Creation Date: 24-Feb-2000
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Classes dealing with the SNMPv2-MIB, RFC-1450
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef SNMPV2MIB_H
#define SNMPV2MIB_H

#include "MibObjs.h"

/*\
 *  snmpSetGroup
\*/

class snmpSetGroup : public BcmMibGroup
{
  public:
    snmpSetGroup ();
   
    bool Populate ();
    
    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS TestLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    
  protected:
    
    enum
    {
      kLEAF_snmpSetSerialNo = 1
    };
};

#endif
