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
//    Filename: standardTrap.cpp
//    Author:   Kevin O'Neal
//    Creation Date: 24-January-2001
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Traps associated with the docsDevNotifications group
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************


#include "standardTrap.h"
#include "MibObjs.h"
#include "BcmSnmpAgent.h"


coldStartTrap::coldStartTrap (BcmSnmpAgent *pAgent)
  : BcmTrap (BcmObjectId("1.3.6.1.6.3.1.1.5.1"), BcmString ("coldStartTrap"), pAgent, TRAP_ID_COLD_START)
{
  // For standard traps, we will make an exception to the rule from
  // RFC-2576 for the enterprise field.  By convention established in 
  // RFC-1157, the SNMPv1 community and much installed software base
  // expects enterprise=sysObjectID for these traps.
  BcmMibObject *pSysObjectID = pAgent->FindObject(BcmString("sysObjectID"));
  if (pSysObjectID)
  {
    BcmVarBind VB;
    pSysObjectID->Get (VB);
    VB.GetOid(fEnterprise);
  }
}

coldStartTrap::~coldStartTrap()
{
}

warmStartTrap::warmStartTrap (BcmSnmpAgent *pAgent)
  : BcmTrap (BcmObjectId("1.3.6.1.6.3.1.1.5.2"), BcmString ("warmStartTrap"), pAgent, TRAP_ID_WARM_START)
{
  // For standard traps, we will make an exception to the rule from
  // RFC-2576 for the enterprise field.  By convention established in 
  // RFC-1157, the SNMPv1 community and much installed software base
  // expects enterprise=sysObjectID for these traps.
  BcmMibObject *pSysObjectID = pAgent->FindObject(BcmString("sysObjectID"));
  if (pSysObjectID)
  {
    BcmVarBind VB;
    pSysObjectID->Get (VB);
    VB.GetOid(fEnterprise);
  }
}

warmStartTrap::~warmStartTrap()
{
}

linkDownTrap::linkDownTrap (BcmSnmpAgent *pAgent, int IfIndex)
  : BcmTrap (BcmObjectId("1.3.6.1.6.3.1.1.5.3"), BcmString ("linkDownTrap"), pAgent, TRAP_ID_LINK_DOWN)
{
  // For standard traps, we will make an exception to the rule from
  // RFC-2576 for the enterprise field.  By convention established in 
  // RFC-1157, the SNMPv1 community and much installed software base
  // expects enterprise=sysObjectID for these traps.
  BcmMibObject *pSysObjectID = pAgent->FindObject(BcmString("sysObjectID"));
  if (pSysObjectID)
  {
    BcmVarBind VB;
    pSysObjectID->Get (VB);
    VB.GetOid(fEnterprise);
  }

  // Append interface number to trap name for convenience when printing
  // to the console
  char IfId[16];
  sprintf (IfId, "(if%d)", IfIndex);
  fName += IfId;

  AddObject (BcmString("ifIndex"), IfIndex);
  AddObject (BcmString("ifAdminStatus"), IfIndex);
  AddObject (BcmString("ifOperStatus"), IfIndex);
}

linkDownTrap::~linkDownTrap()
{
}

linkUpTrap::linkUpTrap (BcmSnmpAgent *pAgent, int IfIndex)
  : BcmTrap (BcmObjectId("1.3.6.1.6.3.1.1.5.4"), BcmString ("linkUpTrap"), pAgent, TRAP_ID_LINK_UP)
{
  // For standard traps, we will make an exception to the rule from
  // RFC-2576 for the enterprise field.  By convention established in 
  // RFC-1157, the SNMPv1 community and much installed software base
  // expects enterprise=sysObjectID for these traps.
  BcmMibObject *pSysObjectID = pAgent->FindObject(BcmString("sysObjectID"));
  if (pSysObjectID)
  {
    BcmVarBind VB;
    pSysObjectID->Get (VB);
    VB.GetOid(fEnterprise);
  }

  // Append interface number to trap name for convenience when printing
  // to the console
  char IfId[16];
  sprintf (IfId, "(if%d)", IfIndex);
  fName += IfId;

  AddObject (BcmString("ifIndex"), IfIndex);
  AddObject (BcmString("ifAdminStatus"), IfIndex);
  AddObject (BcmString("ifOperStatus"), IfIndex);
}

linkUpTrap::~linkUpTrap()
{
}

authFailureTrap::authFailureTrap (BcmSnmpAgent *pAgent)
  : BcmTrap (BcmObjectId("1.3.6.1.6.3.1.1.5.5"), BcmString ("authFailureTrap"), pAgent, TRAP_ID_AUTH_FAILURE)
{
  // For standard traps, we will make an exception to the rule from
  // RFC-2576 for the enterprise field.  By convention established in 
  // RFC-1157, the SNMPv1 community and much installed software base
  // expects enterprise=sysObjectID for these traps.
  BcmMibObject *pSysObjectID = pAgent->FindObject(BcmString("sysObjectID"));
  if (pSysObjectID)
  {
    BcmVarBind VB;
    pSysObjectID->Get (VB);
    VB.GetOid(fEnterprise);
  }
}

authFailureTrap::~authFailureTrap()
{
}


