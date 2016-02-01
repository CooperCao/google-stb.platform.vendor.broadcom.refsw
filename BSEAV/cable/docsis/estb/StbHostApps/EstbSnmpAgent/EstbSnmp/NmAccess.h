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
//    Filename: NmAccess.h
//    Author:   Kevin O'Neal
//    Creation Date: 27-Mar-2000
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Objects for network manaagement access control
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************


#ifndef NMACCESS_H
#define NMACCESS_H

#include <IpAddress.h>

#include "SnmpType.h"
#include "CoreObjs.h"
#include "EthernetPacket.h"


// forward references:
class docsDevNmAccessTable;
class docsDevNmAccessEntryBridge;
class BcmSnmpAgent;

class NmAccessController
{
  public:
    NmAccessController (int Index, BcmSnmpAgent *pAgent);
   ~NmAccessController ();
   
    // Get/Set methods
    int GetIndex () const;
    int GetControl () const;
    unsigned long GetInterfaces () const;
    int GetStatus () const;
    int GetTrapType () const;
    const BcmIpAddress &GetIpAddr () const;
    const BcmIpAddress &GetIpMask () const;
    const BcmString &GetCommunity () const;
    
    SNMP_STATUS SetControl (int Control);
    SNMP_STATUS SetInterfaces (unsigned long Interfaces);
    SNMP_STATUS SetStatus (int Status);
    SNMP_STATUS SetTrapType (int TrapType);
    SNMP_STATUS SetIpAddr (const BcmIpAddress &Ip);
    SNMP_STATUS SetIpMask (const BcmIpAddress &Mask);
    SNMP_STATUS SetCommunity (const BcmString &Community);

    // Validation methods
    SNMP_STATUS ValidateRequest (int PduType, int IfNum,  const BcmIpAddress &SA, const BcmString &Community) const;
    
    SNMP_STATUS ValidateReqPduType (int PduType) const;
    SNMP_STATUS ValidateReqAddr (const BcmIpAddress &PktIp) const;
    SNMP_STATUS ValidateReqInterfaces (int IfNum) const;
    SNMP_STATUS ValidateReqCommunity (const BcmString &Community) const;
    
    docsDevNmAccessEntryBridge *fpBridge;
    
  protected:
    int fIndex, fControl;
    BcmIpAddress fIpAddr, fIpMask;
    BcmString fCommunity;
    unsigned long fInterfaces;
    int fStatus;
    int fTrapType;
    BcmSnmpAgent *fpAgent;    
};

#endif
