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
//    Filename: BcmSnmpTrapHandler.h
//    Author:   Maurice Turcotte
//    Creation Date: 14-Jun-2010
//
//**************************************************************************
//    Description:
//
//    BcmSnmpTrapHandler base class module
//
//      
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef BCMSNMPTRAPHANDLER_H
#define BCMSNMPTRAPHANDLER_H

#ifdef BFC
//#include "BfcSystemConfig.h"
#endif
  
#if (BCM_CONSOLE_SUPPORT)
#include "TrapHandlerCommandTable.h"
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
class BcmSnmpAgent;


class BcmSnmpTrapHandler
{
  public:
    // Construction / destruction
    BcmSnmpTrapHandler(char *pName, BcmSnmpAgent *pMasterAgent = NULL);
    virtual ~BcmSnmpTrapHandler();
    
    // Printing and console
#if (BCM_CONSOLE_SUPPORT)
    static BcmTrapHandlerCommandTable &GetCommandTableSingleton(void);
#endif
    virtual ostream &Print(ostream &outStream) const;
    
    // Initial setup and related accessors
    virtual SNMP_STATUS Setup ();
    virtual SNMP_STATUS Register ();
    virtual SNMP_STATUS Start ();
    virtual const BcmString &GetName () const;
    virtual const BcmString &GetMasterAgentName () const;
    
    // Thread synchronization: lock and unlock mutex.
    virtual SNMP_STATUS LockMib (const char *pDebug = "");
    virtual SNMP_STATUS UnlockMib (const char *pDebug = "");
    virtual int MibLockCount () const;
    
    // IP configuration and related accessors
    virtual SNMP_STATUS ConfigIp (const BcmIpAddress &IpAddr);
    inline BcmSnmpAgent* MasterAgent () const;
    inline const BcmIpAddress &GetIpAddr () const;
    
    // UDP port configuration and related accessors
    virtual SNMP_STATUS SetUdpTrapPort (int UdpTrapPort);
    inline int GetUdpTrapPort () const;
    
    // Operational modes
    virtual bool ManagementEnabled () const;
    
    // PDU authentication and related
    virtual SNMP_STATUS AuthenticateRequest (int RequestType, int SnmpVersion, int &SrcIfNum, const BcmIpAddress &SrcIp, const BcmString &Community);
    
    // Traps
    virtual int SendTrap (BcmTrap *pTrap);
    virtual bool TrapsDeferred () const;
    virtual void DeferTraps (bool Defer);
    virtual int SendDeferredTraps ();
    
    // Write NV associated with this agent.
    virtual SNMP_STATUS WriteNonVolSettings (BcmNonVolSettings::NonVolSection = BcmNonVolSettings::kDynamicSection, bool Async = true);
    
  protected:
#if (BCM_CONSOLE_SUPPORT)
    virtual void InstallCommands ();
    virtual void CommandHandler (const BcmCommand &command);
#endif    
    
    // Inform the agent that something interesting has happened or is about to
    virtual void InformRestartPending ();
    virtual void InformShutdownPending ();
    
  private:
    // Default constructor: not supported
    BcmSnmpTrapHandler (void);
    // Copy constructor: not supported
    BcmSnmpTrapHandler (const BcmSnmpTrapHandler &otherInstance);
    // Assignment operator: not supported      
    BcmSnmpTrapHandler & operator = (const BcmSnmpTrapHandler &otherInstance);
  
  
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
    bool fRegistered, fStarted;
    BcmMutexSemaphore *pfMutex;
    int fMutexLockCount;
    BcmIpAddress fIpAddr;
    int fUdpTrapPort;
    bool fTrapsDeferred;
    BcmTrap *fpDeferredTraps;

    
  friend class BcmMibGroup; // RemoveGroup
  friend class ScalarMibBridge; // RegisterScalar
  friend class TabularMibBridge; // RegisterTabular
  friend class interfacesGroupBridge; // fpIfBridge
  friend class BcmTrapHandlerCommandTable; // CommandHandler
};

inline BcmSnmpAgent* BcmSnmpTrapHandler::MasterAgent () const
{
  return fpMasterAgent;
}

inline const BcmIpAddress &BcmSnmpTrapHandler::GetIpAddr () const
{
  return fIpAddr;
}

inline int BcmSnmpTrapHandler::GetUdpTrapPort () const
{
  return fUdpTrapPort;
}

inline ostream & operator << (ostream &outStream, const BcmSnmpTrapHandler &Agent)
{
  return Agent.Print(outStream);
}

typedef BcmSnmpTrapHandler DefaultSnmpTrapHandlerClass;

#endif

