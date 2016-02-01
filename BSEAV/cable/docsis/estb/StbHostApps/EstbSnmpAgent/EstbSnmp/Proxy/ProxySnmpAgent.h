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
//    Filename: ProxySnmpAgent.h
//    Author:   Maurice Turcotte / Thomas Clack
//    Creation Date: 4 AUG 06
//
//**************************************************************************
//    Description:
//
//      Implements a proxy mechanism which communicates with a
//      remote subAgent via SNMPv2c.
//      
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef PROXYSNMPAGENT_H
#define PROXYSNMPAGENT_H

#include "BcmSnmpAgent.h"
#include "BerObjs.h"
        


class BcmProxySnmpPdu
{
  public:
    BcmProxySnmpPdu (const BcmObjectId &OID, const BcmVarBind &VB, const BcmObjectId &Index, SNMP_STATUS Status);
    BcmProxySnmpPdu (const BcmString &PduBuffer);
   ~BcmProxySnmpPdu ();

    //bool GetOID (BcmObjectId &OID) const;
    //bool GetVB (BcmVarBind &VB) const;
    //bool GetIndex (BcmObjectId &Index) const;
    //bool GetStatus (SNMP_STATUS &Status) const;
    
    //bool GetPdu (BcmString &Pdu) const;
    
    ostream &Print(ostream &outStream) const;

  protected:
    AsnBerVarBind fVB;
    AsnBerOid     fIndex;
    AsnBerUint32  fStatus;
    BcmString     fEncodedPdu;
};

inline ostream & operator << (ostream &outStream, const BcmProxySnmpPdu &Pdu)
{
  return Pdu.Print(outStream);
}

class BcmProxySnmpAgent : public BcmSnmpAgent
{
  public:
    BcmProxySnmpAgent (char *pName, BcmSnmpAgent *pMasterAgent = NULL);
    virtual ~BcmProxySnmpAgent();
    
    virtual SNMP_STATUS GetObject (BcmObjectId &OID, BcmVarBind &VB, BcmMibRequest *pReq = NULL);
    virtual SNMP_STATUS NextObject (BcmObjectId &OID, BcmObjectId &Index, BcmVarBind &VB, BcmMibRequest *pReq = NULL);
    virtual SNMP_STATUS TestObject (BcmObjectId &OID, BcmVarBind &VB, bool FromConfig = false, BcmMibRequest *pReq = NULL);
    virtual SNMP_STATUS SetObject (BcmObjectId &OID, BcmVarBind &VB, bool FromConfig = false, BcmMibRequest *pReq = NULL);
    
  protected:
    virtual SNMP_STATUS DoProxy (int PduType, BcmObjectId &OID, BcmVarBind &VB, BcmObjectId &Index);
};

#endif
