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
//    Filename: BcmSnmpAgent.h
//    Author:   Kevin O'Neal
//    Creation Date: 14-Jun-2001
//
//**************************************************************************
//    Description:
//
//      Cablemodem V2 code
//      
//    BcmSnmpAgent base class module
//
//      
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef BCMSNMPAGENT_H
#define BCMSNMPAGENT_H

#ifdef BFC
//#include "BfcSystemConfig.h"
#endif
  
#if (BCM_CONSOLE_SUPPORT)
#include "SnmpCommandTable.h"
#endif

#include "MibObjs.h"
#include "IpAddress.h"
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <map>
#else
#include <map.h>
#endif
#include "BcmTrap.h"
#include "NonVolSettings.h"
#include "ipBridge.h"
#include "ifBridge.h"
#ifndef BCM_STANDALONE_SNMP_AGENT
#include "NetToMediaSnoop.h"
#include "IfUnknownProtosSnoop.h"
#endif
#include "SnmpThread.h"
#include "BaseIpHalIf.h"


// forward references:
class BcmPacketSwitch;
class interfacesGroupBridge;
class BcmHalIf;
class BcmEthernetPacket;
class ifEntryBridge;


class BcmSnmpAgent
{
  public:
    // Construction / destruction
    BcmSnmpAgent(char *pName, BcmSnmpAgent *pMasterAgent = NULL);
    virtual ~BcmSnmpAgent();
    
    // Printing and console
#if (BCM_CONSOLE_SUPPORT)
    static BcmSnmpCommandTable &GetCommandTableSingleton(void);
#endif
    virtual ostream &Print(ostream &outStream) const;
    
    // Initial setup and related accessors
    virtual SNMP_STATUS Setup (const BcmMacAddress &Mac, int IfNum, BcmPacketSwitch *pSwitch);
    virtual SNMP_STATUS Register ();
    virtual SNMP_STATUS Start ();
    inline int GetIfNum () const;
    virtual const BcmString &GetName () const;
    const BcmMacAddress &GetMacAddress () const;
    BcmPacketSwitch *GetPacketSwitch () const;
    
    // Thread synchronization: lock and unlock mutex.
    virtual SNMP_STATUS LockMib (const char *pDebug = "");
    virtual SNMP_STATUS UnlockMib (const char *pDebug = "");
    virtual int MibLockCount () const;
    
    // IP configuration and related accessors
    virtual SNMP_STATUS ConfigIp (const BcmIpAddress &IpAddr, const BcmIpAddress &SubnetMask, const BcmIpAddress &DefaultGateway);
    inline const BcmIpAddress &GetIpAddr () const;
    inline const BcmIpAddress &GetSubnetMask () const;
    inline const BcmIpAddress &GetRouter () const;
    
    // UDP port configuration and related accessors
    virtual SNMP_STATUS SetUdpPort (int UdpPort);
    virtual SNMP_STATUS SetUdpTrapPort (int UdpTrapPort);
    inline int GetUdpPort () const;
    inline int GetUdpTrapPort () const;
    
    // Operational modes
    virtual bool ManagementEnabled () const;
    virtual void EnableManagement (bool Enable);
    
    typedef enum
    {
      kNoPacketSwitching = 0,
      kTransparentBridge,
      kSpanningTreeBridge,
      kRouter
    } PacketSwitchMode;
    
    virtual PacketSwitchMode PacketSwitchOperMode () const;
    
    // View configuration
    virtual SNMP_STATUS SetV1V2View (const char *pView);
    virtual void GetV1V2View (char *pView, int MaxLen) const;
    virtual SNMP_STATUS SetV1V2View (const BcmString &View);
    virtual const BcmString &GetV1V2View () const;
    
    // PDU authentication and related
    virtual SNMP_STATUS AuthenticateRequest (int RequestType, int SnmpVersion, int &SrcIfNum, const BcmIpAddress &SrcIp, const BcmString &Community);
    virtual SNMP_STATUS ValidateSet (const BcmObjectId &OID, const BcmVarBind &VB, const BcmIpAddress &SrcIp, int SnmpVersion, const BcmString &Community);
    virtual bool IsObjectSupported (const BcmObjectId &OID) const;
    
    // Traps
    virtual int SendStandardTrap (int TrapType, int IfIndex=0, bool Async=false);
    virtual int SendTrap (BcmTrap *pTrap);
    virtual bool TrapsDeferred () const;
    virtual void DeferTraps (bool Defer);
    virtual int SendDeferredTraps ();
    
    // Core get/set methods called from agent APIs (see SnmpApi.h)
    virtual SNMP_STATUS GetObject (BcmObjectId &OID, BcmVarBind &VB, BcmMibRequest *pReq = NULL);
    virtual SNMP_STATUS NextObject (BcmObjectId &OID, BcmObjectId &Index, BcmVarBind &VB, BcmMibRequest *pReq = NULL);
    virtual SNMP_STATUS TestObject (BcmObjectId &OID, BcmVarBind &VB, bool FromConfig = false, BcmMibRequest *pReq = NULL);
    virtual SNMP_STATUS SetObject (BcmObjectId &OID, BcmVarBind &VB, bool FromConfig = false, BcmMibRequest *pReq = NULL);
    
    // Find objects, tables, or groups.
    virtual BcmMibObject *FindObject (BcmObjectId &OID);
    virtual BcmMibObject *FindObject (const BcmString &Name, const BcmObjectId &Index);
    virtual BcmMibObject *FindObject (const BcmString &Name);
    virtual BcmMibTable  *FindTable  (BcmObjectId &OID);
    virtual BcmMibTable  *FindTable  (const BcmString &Name);
    virtual BcmMibGroup  *FindGroup (const BcmObjectId &OID);
    
    // Allow groups to be added publicly.
    virtual SNMP_STATUS AddGroup  (BcmMibGroup *pGroup);
    
    // Convenience methods to get / set MIB objects directly by name.
    SNMP_STATUS GetObject (const char *pName, BcmVarBind &Value, const BcmObjectId &Index = BcmObjectId());
    SNMP_STATUS GetIntObject (const char *pName, int &Value, const BcmObjectId &Index = BcmObjectId());
    SNMP_STATUS GetUintObject (const char *pName, unsigned int &Value, const BcmObjectId &Index = BcmObjectId());
    SNMP_STATUS GetOctetStringObject (const char *pName, unsigned char *pValue, int MaxLen, const BcmObjectId &Index = BcmObjectId());
    SNMP_STATUS GetIpAddrObject (const char *pName, BcmIpAddress &Value, const BcmObjectId &Index = BcmObjectId());
    SNMP_STATUS GetMacAddrObject (const char *pName, BcmMacAddress &Value, const BcmObjectId &Index = BcmObjectId());
    SNMP_STATUS GetOidObject (const char *pName, BcmObjectId &Value, const BcmObjectId &Index = BcmObjectId());
    SNMP_STATUS SetObject (const char *pName, const BcmVarBind &Value, const BcmObjectId &Index = BcmObjectId());
    SNMP_STATUS SetIntObject (const char *pName, int Value, const BcmObjectId &Index = BcmObjectId());
    SNMP_STATUS SetUintObject (const char *pName, unsigned int Value, const BcmObjectId &Index = BcmObjectId());
    SNMP_STATUS SetOctetStringObject (const char *pName, const unsigned char *pValue, int Len = -1, const BcmObjectId &Index = BcmObjectId());
    SNMP_STATUS SetIpAddrObject (const char *pName, const BcmIpAddress &Value, const BcmObjectId &Index = BcmObjectId());
    SNMP_STATUS SetMacAddrObject (const char *pName, const BcmMacAddress &Value, const BcmObjectId &Index = BcmObjectId());
    SNMP_STATUS SetOidObject (const char *pName, const BcmObjectId &Value, const BcmObjectId &Index = BcmObjectId());
    
    // Write NV associated with this agent.
    virtual SNMP_STATUS WriteNonVolSettings (BcmNonVolSettings::NonVolSection = BcmNonVolSettings::kDynamicSection, bool Async = true);
    
    // Factory mode
    virtual SNMP_STATUS EnableFactoryMode (bool Enable);
    virtual bool FactoryMode () const;
    
    /* Native support for certain common MIB objects */
    
    // System group
    virtual const BcmString &sysDescr () const;
    virtual const BcmObjectId &sysObjectID () const;
    virtual time_t sysUpTime () const;
    virtual const BcmString &sysContact () const;
    virtual const BcmString &sysName () const;
    virtual const BcmString &sysLocation () const;
    virtual int sysServices () const;
    virtual time_t sysORLastChange () const;
    virtual const BcmObjectId &sysORID (int sysORIndex) const;
    virtual const BcmString &sysORDescr (int sysORIndex) const;
    virtual time_t sysORUpTime (int sysORIndex) const;
    virtual SNMP_STATUS sysContact (const BcmString &SysContact);
    virtual SNMP_STATUS sysName (const BcmString &SysName);
    virtual SNMP_STATUS sysLocation (const BcmString &SysLocation);
    
    // Interfaces group
    virtual interfacesGroupBridge &IfBridge() const;
    virtual int AssignIfIndex (ifEntryBridge *pIfEntry) const { return 0; }
    virtual ifEntryBridge *NewIfEntryBridge (BcmInterfaceCounters *pInterfaceCounters, 
                                             int ifIndex = 0, int SubIface = 0, 
                                             int ifType = 0);
    
    // IP group
#ifndef BCM_STANDALONE_SNMP_AGENT
    virtual BcmNetToMediaSnoop &N2M() const;
#endif

    virtual int GetIpAdEntIfIndex () const;

#ifdef SNMP_PROXY_SUPPORT
    virtual void SetSubAgent(BcmSnmpAgent *subAgent);
    virtual BcmSnmpAgent *GetSubAgent(void);
#endif
    
  protected:
    virtual bool PopulateMibs();
    virtual bool PopulateBridges();
    virtual SNMP_STATUS InstallViews ();
    virtual void SetupSnoops ();
#if (BCM_CONSOLE_SUPPORT)
    virtual void InstallCommands ();
    virtual void CommandHandler (const BcmCommand &command);
#endif    
    
    virtual bool Depopulate();
    virtual SNMP_STATUS DestroyViews ();
    
    virtual SNMP_STATUS RemoveGroup (BcmMibGroup *pGroup);
    
    virtual SNMP_STATUS RegisterTabular (TabularMibBridge *pBridge, bool DeleteUponDestruction = false);
    virtual SNMP_STATUS RegisterScalar (ScalarMibBridge *pBridge, bool DeleteUponDestruction = false);
    virtual SNMP_STATUS DeRegisterTabular (TabularMibBridge *pBridge);
    virtual SNMP_STATUS DeRegisterScalar (ScalarMibBridge *pBridge);
    
    virtual BcmMibObject *FindOtherInstance (const BcmObjectId &OID, const BcmMibRequest *pReq = NULL) const;
    
    // Inform the agent that something interesting has happened or is about to
    virtual void InformRestartPending ();
    virtual void InformShutdownPending ();
    
  private:
    // Default constructor: not supported
    BcmSnmpAgent (void);
    // Copy constructor: not supported
    BcmSnmpAgent (const BcmSnmpAgent &otherInstance);
    // Assignment operator: not supported      
    BcmSnmpAgent & operator = (const BcmSnmpAgent &otherInstance);
  
  
  /* Data members */
    
  public:
    // SNMP message log bits and settings used by SnmpLog.h
    typedef enum
    {
      kStartupMessages =              0x00000040, // startup messages
      kConfigFileMessages =           0x00000080, // config file messages
      kInitializationMessages =       0x00000100, // initialization stuff, including OID construction/searches, etc
      kRequestMessages =              0x00000200, // log any requests: SET, GET, NEXT, etc
      kNmAccessMessages =             0x00000400, // log all NM access
      kPktFilterMessages =            0x00000800, // log packet filter messages
      kMemoryMessages =               0x00001000, // log new/delete msgs
      kSearchMessages =               0x00002000, // log MIB search msgs
      kEventLogMessages =             0x00004000, // log event log msgs
      kThreadMessages =               0x00008000, // log thread msgs
      kAgentMessages =                0x00010000  // log agent core msgs
    } MsgBits;
    static BcmMessageLogSettings gMessageLogSettings;
    
  protected:
    BcmString fName;
    BcmSnmpAgent *fpMasterAgent;
    BcmSnmpAgent *fpSubAgent;
    int fIfNum;
    BcmMacAddress fMacAddress;
    BcmPacketSwitch *fpPktSwitch;
    bool fRegistered, fStarted, fMgmtEnabled;
    BcmMutexSemaphore *pfMutex;
    int fMutexLockCount;
    BcmIpAddress fIpAddr, fSubnetMask, fRouter;
    int fUdpPort, fUdpTrapPort;
    BcmString fV1V2View;
    MibGroupContainer fGroups;
    int fStartTrap;
    bool fTrapsDeferred;
    BcmTrap *fpDeferredTraps;
    bool fFactoryMode;
    BcmString fSysDescr;
    BcmObjectId fSysObjectID;
    BcmString fSysContact;
    BcmString fSysName;
    BcmString fSysLocation;
    int fSysServices;
    BcmObjectId fSysORID;
    BcmString fSysORDescr;
    interfacesGroupBridge *fpIfBridge;
    ipAddrEntryBridge *fpIpAddrBridge;
#ifndef BCM_STANDALONE_SNMP_AGENT
    BcmNetToMediaSnoop *fpN2MSnoop;
    static BcmIfUnknownProtosSnoop *fpUnkProtosSnoop;
#endif

    
  friend class BcmMibGroup; // RemoveGroup
  friend class ScalarMibBridge; // RegisterScalar
  friend class TabularMibBridge; // RegisterTabular
  friend class interfacesGroupBridge; // fpIfBridge
  friend class BcmSnmpCommandTable; // CommandHandler
};

inline const BcmIpAddress &BcmSnmpAgent::GetIpAddr () const
{
  return fIpAddr;
}

inline int BcmSnmpAgent::GetUdpPort () const
{
  return fUdpPort;
}

inline int BcmSnmpAgent::GetUdpTrapPort () const
{
  return fUdpTrapPort;
}

inline const BcmIpAddress &BcmSnmpAgent::GetSubnetMask () const
{
  return fSubnetMask;
}

inline const BcmIpAddress &BcmSnmpAgent::GetRouter () const
{
  return fRouter;
}

inline int BcmSnmpAgent::GetIfNum () const
{
  return fIfNum;
}

inline ostream & operator << (ostream &outStream, const BcmSnmpAgent &Agent)
{
  return Agent.Print(outStream);
}

#if (SNMP_SUPPORT == 3)
#include "SnmpV3Agent.h"
#else
// If SNMPv3 support is not included, then for convenience declare the
// SNMPv2 agent as the default agent class.
typedef BcmSnmpAgent DefaultSnmpAgentClass;
#endif

#endif

