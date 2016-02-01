//****************************************************************************
//
// Copyright (c) 2007-2010 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//**************************************************************************
//    Filename: ProxySnmpAgent.cpp
//    Author:   Thomas Clack / Maurice Turcotte
//    Creation Date: 4 AUG 2006
//
//**************************************************************************
//    Description:
//
//      Implements a proxy mechanism which communicates with a
//      remote subAgent via SNMPv2.
//      
//**************************************************************************
//    Revision History:
//
//**************************************************************************


#include "ProxySnmpAgent.h"
#include "BerObjs.h"
#include "SnmpLog.h"

#include "Proxy.h"
#include "ProxyPdu.h"

#include <net-snmp/library/asn1.h>
#include <net-snmp/library/snmp.h>


BcmProxySnmpAgent::BcmProxySnmpAgent (char *pName, BcmSnmpAgent *pMasterAgent)
: BcmSnmpAgent (pName, pMasterAgent)
{
    if ( pMasterAgent )
    {
        pMasterAgent->SetSubAgent(this);
    }

}

BcmProxySnmpAgent::~BcmProxySnmpAgent()
{
}

SNMP_STATUS BcmProxySnmpAgent::GetObject (BcmObjectId &OID, BcmVarBind &VB, BcmMibRequest *pReq)
{
    BcmObjectId Index; // Unused except for 'next'
    // SnmpLogReq << "Proxy agent " << GetName () << " GET request for " << OID << endl;

    return DoProxy (PDU_GET_REQUEST, OID, VB, Index);
}

SNMP_STATUS BcmProxySnmpAgent::NextObject (BcmObjectId &OID, BcmObjectId &Index, BcmVarBind &VB, BcmMibRequest *pReq)
{
    // SnmpLogReq << "Proxy agent " << GetName () << " NEXT request for " << OID << endl;

    return DoProxy (PDU_GET_NEXT_REQUEST, OID, VB, Index);
}

SNMP_STATUS BcmProxySnmpAgent::TestObject (BcmObjectId &OID, BcmVarBind &VB, bool FromConfig, BcmMibRequest *pReq)
{
    BcmObjectId Index; // Unused except for 'next'
    SnmpLogReq << "Proxy agent " << GetName () << " TEST request for " << OID << endl;

    return DoProxy (PDU_TEST_REQUEST, OID, VB, Index);
}

SNMP_STATUS BcmProxySnmpAgent::SetObject (BcmObjectId &OID, BcmVarBind &VB, bool FromConfig, BcmMibRequest *pReq)
{
    BcmObjectId Index; // Unused except for 'next'
    SnmpLogReq << "Proxy agent " << GetName () << " SET request for " << OID << endl;

    return DoProxy (PDU_SET_REQUEST, OID, VB, Index);
}

SNMP_STATUS BcmProxySnmpAgent::DoProxy (int PduType, BcmObjectId &OID, BcmVarBind &VB, BcmObjectId &Index)
{

    SNMP_STATUS Status = SNMP_NO_ERROR;
    BcmObjectId tmpOid = OID + Index;
    

    // cout<<"DoProxy() is called!!!!!"<<endl;


    switch (PduType) {
    case PDU_GET_REQUEST:
      Status = LocalProxyGet (OID, VB);
      break;
    case PDU_GET_NEXT_REQUEST:
      Status = LocalProxyGetNext (tmpOid, VB);
      if (Status == SNMP_NO_ERROR) {
        OID = tmpOid;
        tmpOid.GetIndex(Index);
      }
      break;
    case PDU_SET_REQUEST:
      Status = LocalProxySet (OID, VB);
      break;
    case PDU_TEST_REQUEST:
      break;
    default:
      cout<<"DoProxy() got an unknown PDU type " << PduType <<", doing nothing!!!!"<<endl;
    }

    if (Status != SNMP_NO_ERROR)  /* if there was an error, just return it */
      return Status;
    else                          /* if not, we need to check the value cuz this is V2 */
      switch (VB.GetType()) {
      case SNMP_NOSUCHOBJECT:
      case SNMP_NOSUCHINSTANCE:
      case SNMP_ENDOFMIBVIEW:
        return SNMP_ERR_NOSUCHNAME;
      default:
        return Status;
      }
#ifdef NOTDEF


    //TODO:  not sure why this may be 0
    if ( VB.GetType() == 0 )
    {
        cout  <<  "setting the VB type to EMPTY (NULL)" << endl;
        VB.SetEmpty();
        cout  <<  "Do Proxy varbind NOW is"  <<  VB  <<  endl; 
    }

    //This is where we set the community string to use for the Proxy Snmp Pdu
    BcmString community("pub");

    int errorIndex = 0;

    BcmProxyPdu TxPDU( BcmProxyPdu::ProxyPduType(PduType), 
                      BcmProxyPdu::SnmpProtocolVersion(SNMP_V2_QUERY),
                      BcmProxyPdu::SnmpErrorStatus(SNMP_NO_ERROR),
                      errorIndex,
                      community);

    TxPDU.AddVarbind (OID, VB);

    BcmString myProxyPdu ("a place for the pdu");
    TxPDU.Asn1Encode (myProxyPdu);

    // cout<<"This is the PROXY SNMP pdu to be transmitted ...."<<endl;
    // cout<<TxPDU<<endl;
    // cout<<"type is "<<TxPDU.PduType()<<endl;


    BcmProxy myProxy;
    BcmOctetBuffer TxBuf, RxBuf;

    // Convert encoded request to a BcmOctetBuffer
    myProxyPdu.Get(TxBuf);

    // If this call fails it probably means the Proxy transaction timed out.
    // In that case we'll return NO_SUCH_NAME so at least we won't break
    // mibwalks, etc.

    if ( myProxy.SendPacket( myProxyPdu, &RxBuf) != true )
    {
        cout << "  Proxy SendPacket failed!" << endl;
        return SNMP_NO_SUCH_NAME;
    }


    //TODO: Need to decode the RxBuf and copy to vars

#endif

}
