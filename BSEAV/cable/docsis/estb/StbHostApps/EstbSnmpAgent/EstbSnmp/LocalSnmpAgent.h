//****************************************************************************
//
// Copyright (c) 2010 Broadcom Corporation
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
//****************************************************************************
//    Filename: LocalSnmpAgent.cpp
//    Author:   Maurice Turcotte
//    Creation Date: 9 NOV 10
//
//**************************************************************************
//    Description:
//
//    The local SNMP agent - listens on loopback address and has full access
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef LOCALSNMPAGNT_H
#define LOCALSNMPAGNT_H

#include "IpAddress.h"
#include "HalIf.h"
#include "SnmpV3Agent.h"
#include "EstbSnmpAgent.h"


#define LOCAL_AGENT_NAME "Local SNMP Agent"

// forward reference
class docsDevNmAccessTable;

// CM agent will be derived from the default agent class, which means
// BcmSnmpV3Agent if we have SNMPv3 support or BcmSnmpAgent if not.
class LocalSnmpAgent : public DefaultSnmpAgentClass
{
  public:
    static LocalSnmpAgent &Singleton();
   ~LocalSnmpAgent ();
   
    static inline bool Exists ();
    
    virtual bool PopulateMibs();
    virtual void SetupSnoops();
    SNMP_STATUS Start ();
    SNMP_STATUS EnableFactoryMode (bool Enable);
    
    void InformRegistrationComplete ();
    void InformRestartPending ();
    
    SNMP_STATUS AuthenticateRequest (int RequestType, int SnmpVersion, int &SrcIfNum, const BcmIpAddress &SrcIp, const BcmString &Community);
    
    virtual SNMP_STATUS TestObject (BcmObjectId &OID, BcmVarBind &VB, bool FromConfig = false, BcmMibRequest *pReq = NULL);
    virtual SNMP_STATUS SetObject (BcmObjectId &OID, BcmVarBind &VB, bool FromConfig = false, BcmMibRequest *pReq = NULL);
    
  private:
    static LocalSnmpAgent *fpSingletonInstance;
   
  protected:
    docsDevNmAccessTable *fpNmAccessTable;
    
    LocalSnmpAgent ();
    
    virtual SNMP_STATUS InstallViews ();
    SNMP_STATUS InstallUsers ();
    
  friend class EstbSnmpAgent;
  friend class BcmCmDocsisFactory; // if LocalSnmpAgent base class is instantiated
};

// Static method to allow the caller to tell if the Local agent exists or not
// without having to instantiate him via a call to Singleton().
inline bool LocalSnmpAgent::Exists ()
{
  if (fpSingletonInstance)
    return true;
    
  return false;
}

#endif
