//**************************************************************************
//
//    Copyright 2000  Broadcom Corporation
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
//    Filename: BcmTrap.h
//    Author:   Kevin O'Neal
//    Creation Date: 22-may-2000
//
//**************************************************************************
//    Description:
//
//      Header file for base trap object
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef BCMTRAP_H
#define BCMTRAP_H

#include <list>
#include "IpAddress.h"
#include "CoreObjs.h"

// Generic trap ID's:
#define TRAP_ID_COLD_START          0
#define TRAP_ID_WARM_START          1
#define TRAP_ID_LINK_DOWN           2
#define TRAP_ID_LINK_UP             3
#define TRAP_ID_AUTH_FAILURE        4
#define TRAP_ID_EGP_NEIGHBOR_LOSS   5
#define TRAP_ID_ENTERPRISE          6

#define V1_TRAP         1
#define V2_TRAP         2
#define V1_AND_V2_TRAP  3

// forward references:
class BcmMibObject;
class BcmSnmpAgent;

struct TrapObject
{
  BcmObjectId OID;
  BcmVarBind  Value;
};

// Standard trap object
class BcmTrap
{ 
  public:
    BcmTrap (const BcmObjectId &OID, const BcmString &Name, BcmSnmpAgent *pAgent, int TrapType = TRAP_ID_ENTERPRISE);
    virtual ~BcmTrap ();
    
    inline const BcmObjectId &OID ();
    inline const BcmString &Name ();
    inline int Type ();
    
    virtual SNMP_STATUS AddObject (const BcmString &Name, const BcmObjectId &Index);
    virtual SNMP_STATUS AddObject (BcmObjectId &OID);
    virtual const TrapObject *GetObject (int Index) const;
    
    virtual SNMP_STATUS SendV1Trap (const BcmIpAddress &From, const BcmIpAddress &To, const BcmString &Community, int ToPort=162);
    virtual SNMP_STATUS SendV2Trap (const BcmIpAddress &From, const BcmIpAddress &To, const BcmString &Community, int ToPort=162);
    virtual SNMP_STATUS SendNotify (const BcmIpAddress &From);
    
    virtual ostream &Print(ostream &outStream);
    
    BcmTrap *fpNext; // for chaining.
    
  protected:
    BcmString fName;
    BcmObjectId fOID;
    BcmSnmpAgent *fpAgent;

    list <TrapObject *> fObjects;
    
    // V1-specific trap things:
    int fGeneric, fSpecific;
    BcmObjectId fEnterprise;
    
  public:
    static bool fSilent; // Globally silence console printing
};

typedef list <BcmTrap *> BcmTrapContainer;

typedef BcmTrapContainer::iterator BcmTrapIterator;

inline ostream & operator << (ostream &outStream, BcmTrap &Trap)
{
    return Trap.Print(outStream);
}

inline const BcmObjectId &BcmTrap::OID ()
{
  return fOID;
}

inline const BcmString &BcmTrap::Name ()
{
  return fName;
}

inline int BcmTrap::Type()
{
  return fGeneric;
}


#endif
