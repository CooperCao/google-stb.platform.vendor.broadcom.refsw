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
//    Filename: snmpV2Mib.cpp
//    Author:   Kevin O'Neal
//    Creation Date: 25-Feb-2000
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

#include "snmpV2Mib.h"


/*\
 *  snmpSetGroup 
\*/

snmpSetGroup::snmpSetGroup () 
  : BcmMibGroup ("1.3.6.1.6.3.1.1.6", "snmpV2")
{
#if !DYNAMIC_POPULATION  
  Populate ();
#endif
}

bool snmpSetGroup::Populate ()
{
  BcmMibObject *pScalar;
  
  
  if (BcmMibGroup::Populate() == false)
    return false;

  pScalar = new TestAndIncrMibObject ("1.3.6.1.6.3.1.1.6.1.0","snmpSetSerialNo",this,kReadWrite);
  AddScalar (pScalar);

  return true;
}


SNMP_STATUS snmpSetGroup::GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_SUCH_NAME;
  
  
  switch (Leaf.Leaf())
  {
    // snmpSetSerialNo is a TestAndIncr object, let it handle itself.
    case kLEAF_snmpSetSerialNo:
      Status = SNMP_NO_OPINION;
      break;
  }
  
  return Status;
}

SNMP_STATUS snmpSetGroup::TestLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_SUCH_NAME;
  
  
  switch (Leaf.Leaf())
  {
    // snmpSetSerialNo is a TestAndIncr object, let it handle itself.
    case kLEAF_snmpSetSerialNo:
      Status = SNMP_NO_OPINION;
      break;
  }
  
  return Status;
}

SNMP_STATUS snmpSetGroup::SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB)
{
  SNMP_STATUS Status = SNMP_NO_SUCH_NAME;
  
  
  switch (Leaf.Leaf())
  {
    // snmpSetSerialNo is a TestAndIncr object, let it handle itself.
    case kLEAF_snmpSetSerialNo:
      Status = SNMP_NO_OPINION;
      break;
  }
  
  return Status;
}
