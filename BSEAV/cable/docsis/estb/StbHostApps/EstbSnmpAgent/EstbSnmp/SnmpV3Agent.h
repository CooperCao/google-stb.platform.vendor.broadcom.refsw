//****************************************************************************
//
// Copyright (c) 2007-2008 Broadcom Corporation
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
//    Filename: BcmSnmpV3Agent.h
//    Author:   Kevin O'Neal
//    Creation Date: 18-September-2003
//
//**************************************************************************
//    Description:
//
//      Broadcom BFC code
//      
//    Base class module for an SNMPv3 agent, derived from BcmSnmpAgent
//    base class
//
//      
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef BCMSNMPV3AGENT_H
#define BCMSNMPV3AGENT_H

#include "BcmSnmpAgent.h"

#if (SNMP_SUPPORT == 3)

#include "snmpDH.h"

class BcmDhKickstartThread;  

class BcmSnmpV3Agent : public BcmSnmpAgent
{
  public:
    BcmSnmpV3Agent(char *pName, BcmSnmpAgent *pMasterAgent = NULL);
    virtual ~BcmSnmpV3Agent();
    
    /* Overridden BcmSnmpAgent virtual methods */
    
    virtual ostream &Print(ostream &outStream) const;
    virtual SNMP_STATUS Register ();
    virtual SNMP_STATUS Start ();
    virtual SNMP_STATUS AuthenticateRequest (int RequestType, int SnmpVersion, int &SrcIfNum, const BcmIpAddress &SrcIp, const BcmString &Community);
   
  protected:
    virtual bool PopulateMibs();
    virtual bool PopulateBridges();
#if (BCM_CONSOLE_SUPPORT)
    virtual void InstallCommands ();
    virtual void CommandHandler (const BcmCommand &command);
#endif    

    
    /* SNMPv3 specific things */
  
  public:
    virtual const BcmString &GetEngineId () const;
    virtual unsigned long GetEngineBoots () const;
    virtual const BcmString &GetContext () const;
    inline const char *GetTLV38Base () const;
    inline const char *GetTLV53Base () const;
    virtual SNMP_STATUS ProvisionDHKickstartEntry (BcmOctetBuffer &SecurityName, BcmOctetBuffer &PublicNumber);
    virtual SNMP_STATUS ProvisionNotifyEntry (int TrapType, const BcmString &ElementName, const BcmString &SecurityName, const BcmIpAddress &TargetIp, int Port, int TimeOut, int Retries, BcmOctetBuffer &Filter);
    virtual BcmDhKickstartThread *DhKickstartThread() const;
    virtual SNMP_STATUS ProvisionCommunityEntry (const BcmString &tableIndex, const BcmString &commName, const BcmString &secName, const BcmString &transportTag);
    virtual SNMP_STATUS ProvisionTargetAddrEntry (const BcmString &elementName, const BcmString &tAddr, const BcmString &tagList);
    virtual SNMP_STATUS ProvisionTargetAddrExtEntry (const BcmString &elementName, const BcmString &tAddrMask);
    virtual SNMP_STATUS ProvisionSecurityToGroupEntry (int secModel, const BcmString &secName, const BcmString &groupName);
    virtual SNMP_STATUS ProvisionVacmAccessEntry (const BcmString &groupName, int viewType, const BcmString &viewName, int secModel);
    virtual SNMP_STATUS ProvisionVacmViewTreeFamilyEntry (const BcmString &viewName,const BcmObjectId &subtree, BcmOctetBuffer &mask, int type);
    
  protected:
    virtual SNMP_STATUS InstallEngineId ();
    virtual SNMP_STATUS InstallContext ();
    virtual SNMP_STATUS InstallUsers ();
    virtual SNMP_STATUS InstallNotifies ();
    virtual SNMP_STATUS EnableUser (const BcmString &UserName);
    virtual SNMP_STATUS DisableUser (const BcmString &UserName);
    virtual SNMP_STATUS DestroyUsers ();
    virtual SNMP_STATUS DestroyNotifies (); 
    void SetTLV38Base (const char *TLV38_BaseName);
    void SetTLV53Base (const char *TLV53_BaseName);
    
  private:
    // Default constructor: not supported
    BcmSnmpV3Agent (void);
    // Copy constructor: not supported
    BcmSnmpV3Agent (const BcmSnmpV3Agent &otherInstance);
    // Assignment operator: not supported      
    BcmSnmpV3Agent & operator = (const BcmSnmpV3Agent &otherInstance);

 /* Data members */

  public:
    snmpCommunityTable *fpCommunityTable;
    
  protected:    
	BcmString fTLV38_Base;
	BcmString fTLV38_Base_0;
	BcmString fTLV38_Base_inform;
	BcmString fTLV38_Base_trap;
	BcmString fTLV38_BaseUSM;
	BcmString fTLV38_BaseV1;
	BcmString fTLV38_BaseV2;
	BcmString fTLV53_Base;
    BcmDhKickstartThread  *fpDhThread;
    
    friend class BcmDhKickstartThread;
};

inline const char *BcmSnmpV3Agent::GetTLV38Base () const
{
  return fTLV38_Base.c_str();
}

inline const char *BcmSnmpV3Agent::GetTLV53Base () const
{
  return fTLV53_Base.c_str();
}

// If SNMPv3 support is included, then for convenience declare the
// SNMPv3 agent as the default agent class.
typedef BcmSnmpV3Agent DefaultSnmpAgentClass;

#endif

#endif

