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
//****************************************************************************
//    Filename: EstbSnmpAgent.h
//    Author:   Maurice Turcotte
//    Creation Date: 22 MAY 06
//
//**************************************************************************
//    Description:
//
//    Portable starting point for the eSTB SNMP agent
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef ESTBSNMPAGNT_H
#define ESTBSNMPAGNT_H

#include "IpAddress.h"
// #include "HalIf.h"
#include "SnmpV3Agent.h"
#include "estbDocsDevMib.h"
// #include "PktFiltr.h"
#include "DhKickstartThread.h"
#include "EstbOsMonitorThread.h"
// #include "StpFilterSnoop.h"
// #include "DocsisMibFilterSnoop.h"
#include "DocsisCfgWacEngine.h"
#include "DocsisCfgVarBindEngine.h"

#if (MOCA_MIB_SUPPORT)
#include "MocaMonitorThread.h"
#endif

#define ESTB_AGENT_NAME "estb snmp agent"
// If the interface name is "any" then listen on all interfaces
#define ESTB_SNMP_ANY_INTERFACE "any"
// By default let's listen on all interfaces
#define ESTB_SNMP_DEFAULT_INTERFACE ESTB_SNMP_ANY_INTERFACE
#define ESTB_SNMP_INTERFACE_NAME_LEN 16
#define ESTB_AGENT_TLV38_BASE "@config"
#define ESTB_AGENT_TLV53_BASE "@STBconfig"

#define CPE_FILTERS 0x01
#define IP_FILTERS  0x02
#define LLC_FILTERS 0x04
#define NM_FILTERS  0x08


class EstbSnmpAgentCmDocsisStatusACT;
class BcmEstbApplication;
class BcmPacketSwitch;
class BcmCmDocsisCtlThread;


// eSTB agent will be derived from the default agent class, which means
// BcmSnmpV3Agent if we have SNMPv3 support or BcmSnmpAgent if not.
class EstbSnmpAgent : public DefaultSnmpAgentClass
{
  public:
    static EstbSnmpAgent &Singleton();
   ~EstbSnmpAgent ();
    
    static inline bool Exists ();
    
    /* Overridden DefaultSnmpAgentClass virtual methods */
    
  public:
    virtual ostream &Print(ostream &outStream) const;
    SNMP_STATUS Setup (const BcmMacAddress &Mac, unsigned int IfNum, BcmPacketSwitch *pSwitch, BcmCmDocsisCtlThread *pDocsisCtlThread);//, BcmEstbApplication *pEstbApplication);
    SNMP_STATUS Start ();
    SNMP_STATUS SetUdpPort (int UdpPort);
    SNMP_STATUS SetUdpTrapPort (int UdpTrapPort);
    void EnableManagement (bool Enable);
    SNMP_STATUS AuthenticateRequest (int RequestType, int SnmpVersion, int &SrcIfNum, const BcmIpAddress &SrcIp, const BcmString &Community);
    SNMP_STATUS ValidateSet (const BcmObjectId &OID, const BcmVarBind &VB, const BcmIpAddress &SrcIp, int SnmpVersion, const BcmString &Community);
    int SendTrap (BcmTrap *pTrap);
    int SendStandardTrap (int TrapType, int IfIndex=0, bool Async=false);
    int SendDeferredTraps ();
    virtual SNMP_STATUS WriteNonVolSettings (BcmNonVolSettings::NonVolSection = BcmNonVolSettings::kDynamicSection, bool Async = true);
    virtual SNMP_STATUS EnableFactoryMode (bool Enable);
    SNMP_STATUS sysContact (const BcmString &SysContact);
    SNMP_STATUS sysName (const BcmString &SysName);
    SNMP_STATUS sysLocation (const BcmString &SysLocation);
    int AssignIfIndex (ifEntryBridge *pIfEntry) const;
    unsigned long GetEngineBoots () const;
    SNMP_STATUS ProvisionDHKickstartEntry (BcmOctetBuffer &SecurityName, BcmOctetBuffer &PublicNumber);
    void ResetMibDefaults ();
    void FlushN2Mbridges ();
    int ScanLinuxDevices();
    int ScanLinuxProcessTable();
    void ScanLinuxArpTable ();
    void ScanLinuxTables ();
    
  protected:
    EstbSnmpAgent ();
    virtual SNMP_STATUS InstallViews ();
    virtual SNMP_STATUS InstallUsers ();
    virtual bool PopulateMibs();
    virtual bool PopulateBridges();
    void SetupSnoops ();
#if (BCM_CONSOLE_SUPPORT)
    virtual void InstallCommands ();
    virtual void CommandHandler (const BcmCommand &command);
#endif
    void InformRestartPending ();
    void InformShutdownPending ();   

    
    /* CM specific things */
    
  public:
    virtual SNMP_STATUS ConfigDocsisViews (unsigned long *pSubtree, int SubtreeSize, SNMP_VIEW_STATUS Status);
	void SetMasterAgent(DefaultSnmpAgentClass *pAgent);
    SNMP_MODE GetDocsisSnmpMode () const;
    // DOCSIS_MODE GetDocsisMode () const;
    SNMP_STATUS InformProvisioningComplete ();
    virtual void InformRegistrationComplete ();
    SNMP_STATUS BerSetWriteAccess (unsigned char *pBerBuf, int bufferLen);
    SNMP_STATUS BerSetObject (unsigned char *pBerBuf, int BufferLen = -1);
    inline int TLV11Sets () const { return fTlv11Engine.NumVarBinds(); };
    inline int TLV11Duplicates () const { return fTlv11Engine.NumDuplicates(); };
    inline int TLV11UnknownOIDs () const { return fTlv11Engine.NumUnknowns(); };
    inline int TLV11WrongValues () const { return fTlv11Engine.NumWrongValues(); };  
    inline int TLV11LastErr () const { return fTlv11Engine.LastError(); };  
    inline int TLV11LastErrIndex () const { return fTlv11Engine.LastErrorIndex(); };  
    inline BcmString GetInterfaceName() const { return fInterfaceName; };
    void SystemTimeChangeEvent();
    void EstbConfigFileNewEvent();
    void InterfaceOperationalEvent ();
    void ShutdownPendingEvent();


  protected:
    SNMP_STATUS ProcessSetList ();
    SNMP_STATUS LoadConfigFile (const char *Path);

  private:

	#if (BCM_EVD_INTERFACE_INCLUDED)
	#if !(BCM_EVD_RPCCL_INTERFACE_INCLUDED)
	void SubscribeToEvents(void);
	#endif
	#endif
    
    /* data members */
  
  private:
    static EstbSnmpAgent *fpSingletonInstance;
    
  public:
    BcmCmDocsisCtlThread *fpDocsisCtlThread;
    //BcmEstbApplication *fpEstbApplication;
    
  protected:
    SNMP_MODE fSnmpMode;
#ifdef NOTDEF
    DOCSIS_MODE fDocsisMode;
    BcmMibTable *fpLlcFilterTable, *fpIpFilterTable, *fpFilterPolicyTable,
                *fpTosTable, *fpCpeFilterTable, 
                *fpTargetExtTable, *fpDHKickstartTable,
                *fpDHUserKeyTable, *fpIpNetToMediaTable;
#endif
    BcmMibTable *fpCpeFilterTable, *fpTargetExtTable, *fpDHKickstartTable,
                *fpDHUserKeyTable, *fpIpNetToMediaTable,
                *fpIpNetToPhysicalTable, *fpHrSWRunTable,
                *fpHrSWRunPerfTable;
    docsDevNmAccessTable *fpNmAccessTable;
    EstbSnmpAgentCmDocsisStatusACT *fpCmDocsisStatusACT;
    // BcmDocsisMibFilterSnoop *fpFilterSnoop;
    // BcmStpFilterSnoop *fpStpSnoop;
    BcmDocsisCfgVarBindEngine fTlv11Engine;
    BcmDocsisCfgWacEngine fTlv10Engine;
    BcmString fInterfaceName;
    BcmEstbOsMonitorThread *fpOsMonThread;
#if (MOCA_MIB_SUPPORT)
    BcmMocaMonitorThread *fpMocaMonThread;
#endif

    
  friend class EstbSnmpAgentCmDocsisStatusACT; // ACT events
  friend class BcmMainCommandTable; // InformShutdownPending for reset cmd
  friend class BcmEstbOsMonitorThread; // Status events from OS monitor

#if (MOCA_MIB_SUPPORT)
  friend class BcmMocaMonitorThread; // Status events from OS monitor
#endif
};


// Static method to allow the caller to tell if the CPE agent exists or not
// without having to instantiate him via a call to Singleton().
inline bool EstbSnmpAgent::Exists ()
{
  if (fpSingletonInstance != NULL)
    return true;
    
  return false;
}

#endif
