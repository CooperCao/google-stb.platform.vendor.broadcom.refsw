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
//    Filename: MibBridge.h
//    Author:   Kevin O'Neal
//    Creation Date: 13-Mar-2000
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Base classes for MIB bridge objects.  Bridge objects are used to 
//    connect the SNMP subsystem with the rest of the application.
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef MIBBRIDGE_H
#define MIBBRIDGE_H

#include "SnmpType.h"
#include "MibObjs.h"


// forward references
class BcmSnmpAgent;
class BcmMibObject;

// Some error return codes:
#define MIB_BRIDGE_NULL_FRIEND        111
#define MIB_BRIDGE_NULL_HANDLER       222
#define MIB_BRIDGE_HANDLER_FAILED     333

// See MibBridge.cpp for detailed explanation of what this means...
typedef enum
{
  kDeleteBridge = 0,
  kDestroyMibObject
} CleanupMethod;

class BcmMibBridge
{
  public:
    BcmMibBridge (const char *OidStr, BcmSnmpAgent *pAgent);
    virtual ~BcmMibBridge();

    virtual SNMP_STATUS Register(CleanupMethod Cleanup, bool SilentFailure=false);
    virtual SNMP_STATUS DeRegister();
    
    void GetOid (BcmObjectId &Oid) const;
    virtual void GetIndex (BcmObjectId &Index) const;
    BcmSnmpAgent *GetAgent () const { return fpAgent; }

    
    int BoolToTruth (bool BoolVal) const;
    bool TruthToBool (int TruthVal) const;
    
    virtual bool IsLeafAccessible (int Leaf, const BcmMibRequest *pReq) const {return true;}
   
    BcmMibRequest *LastRequestInfo () const;
 
  protected:
    const char *fpOidString;
    BcmSnmpAgent *fpAgent;
    
  public:
    BcmMibObject *fpObject;
};


class ScalarMibBridge : public BcmMibBridge
{
  public:
    ScalarMibBridge (const char *OidStr, BcmSnmpAgent *pAgent);
    virtual ~ScalarMibBridge();

    virtual SNMP_STATUS Register(CleanupMethod Cleanup, bool SilentFailure=false);
    virtual SNMP_STATUS DeRegister ();
};


class TabularMibBridge : public BcmMibBridge
{
  public:
    TabularMibBridge (const char *pOidStr, const char *pIndexStr, BcmSnmpAgent *pAgent);
    TabularMibBridge (const char *pOidStr, int Index, BcmSnmpAgent *pAgent);
    TabularMibBridge (const char *pOidStr, const BcmObjectId &Index, BcmSnmpAgent *pAgent);
    TabularMibBridge (const char *pOidStr, BcmSnmpAgent *pAgent);
    virtual ~TabularMibBridge();
    
    virtual SNMP_STATUS Register(CleanupMethod Cleanup, bool SilentFailure=false);
    virtual SNMP_STATUS DeRegister ();
    
    virtual int SetCreationMethod (int CreationMethod);
    
    virtual void GetIndex (BcmObjectId &Index) const;
    virtual SNMP_STATUS SetIndex (const BcmObjectId &Index);
    
    virtual bool IsLeafAccessible (int Leaf, const BcmMibRequest *pReq) const;
    
  protected:
    BcmObjectId *fpIndex;
    const char  *fpIndexString;
    unsigned long fObjectMask;
    unsigned long fRequiredObjects;
    
    virtual SNMP_STATUS SetRowStatus (int &RowStatus);
    
    friend class BcmMibEntry;
};

#endif
