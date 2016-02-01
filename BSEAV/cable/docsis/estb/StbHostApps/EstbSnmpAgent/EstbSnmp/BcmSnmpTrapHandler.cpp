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
//**************************************************************************
//    Filename: BcmSnmpTrapHandler.cpp
//    Author:   Maurice Turcotte
//    Creation Date: 14-Jun-2010
//
//**************************************************************************
//    Description:
//
//    BcmSnmpTrapHandler base class
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include "SnmpLog.h"
#include "SnmpApi.h"
#include "BerObjs.h"
#include "OperatingSystemFactory.h"
#include "NonVolDevice.h"
#include "MessageLogNonVolSettings.h"
#include "BcmSnmpTrapHandler.h"
#include "SnmpTrapExploderThread.h"
#include "MibBridge.h"
#include "MibFactory.h"
#include "HalIf.h"
#include "SocketFactory.h"
#include "standardTrap.h"


#include "PacketSwitch.h"
#include "EthernetPacket.h"

BcmMessageLogSettings BcmSnmpTrapHandler::gMessageLogSettings("BcmSnmpTrapHandler", "Snmp Agent message log");

BcmSnmpTrapHandler::BcmSnmpTrapHandler (char *pName, BcmSnmpAgent *pMasterAgent)
{
  static bool MsgLogRegistered = false;

  // Create a mutex for use in LockMib and UnlockMib methods.
  pfMutex = BcmOperatingSystemFactory::NewMutexSemaphore("BcmSnmpTrapHandler's mutex");
  if (pfMutex == NULL)
    SnmpLogRaw << "WARNING: failed to create mutex for " << pName << endl;

  gLogMessageRaw << "Creating SNMP Trap Handler " << pName << endl;
  
  fName = pName;
  
  fTrapsDeferred = true;
  
  fMutexLockCount = 0;
    
  LockMib();

  // Only register message log settings one time...
  if (MsgLogRegistered == false)
  {
    MsgLogRegistered = true;

    gMessageLogSettings.Register();

    // Enable info messages by default.
    gMessageLogSettings.SetEnabledSeverities(gMessageLogSettings.GetEnabledSeverities() | BcmMessageLogSettings::kInformationalMessages);

	#if KB_TODO
    // Load settings from NV if it exists
    BcmMessageLogNonVolSettings *pSettings = BcmMessageLogNonVolSettings::GetSingletonInstance();    
    if (pSettings)
      gMessageLogSettings.SetEnabledSeverities(gMessageLogSettings.GetEnabledSeverities() | pSettings->Severities());
      
    // If we have 'agent' debugging enabled in msglog, turn on agent debug
    // at the '1' level.
    if (gMessageLogSettings.GetEnabledSeverities() & kAgentMessages)
    {
      if (AgentGetDebugLevel() == 0)
      {
        gLogMessageRaw << "Agent debugging enabled in NV, turning on agent debug." << endl;
        AgentSetDebugLevel(1);
      }
    }
	#endif
  }

  fpDeferredTraps = NULL;
  
  fUdpTrapPort = 162; // standard SNMP trap port unless told otherwise.
  fRegistered = false;
  fStarted = false;
  
  fpMasterAgent = pMasterAgent;
  
  UnlockMib();
}

BcmSnmpTrapHandler::~BcmSnmpTrapHandler ()
{
  SnmpLogMem << "BcmSnmpTrapHandler::~BcmSnmpTrapHandler" << endl;

  if (pfMutex)
  {
    delete pfMutex;
    pfMutex = NULL;
  }

#if (BCM_CONSOLE_SUPPORT)
  // Deregister myself from the command table.
  GetCommandTableSingleton().RemoveInstance(this);
#endif

}

ostream &BcmSnmpTrapHandler::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  
  
  outStream << "BcmSnmpTrapHandler object '" << GetName() << "'" << endl;

  outStream << "  IP addr: " << GetIpAddr() << endl;
  outStream << "  UDP trap port: " << GetUdpTrapPort() << endl;
  outStream << "  Management enabled: " << ManagementEnabled() << endl;
  outStream << "  Traps deferred: " << TrapsDeferred() << endl;
  
  if (fpMasterAgent)
    outStream << "  Master agent: " << GetMasterAgentName() << endl;
  else
    outStream << "  NO MASTER AGENT!!!! " << endl;

    
  outStream << "  Mutex locks: " << MibLockCount() << endl;
#endif

  return outStream;
}

SNMP_STATUS BcmSnmpTrapHandler::Setup ()
{
  fIpAddr = kBroadcastIp;
  
#if (BCM_CONSOLE_SUPPORT)  
  // Add commands to the command table.
  InstallCommands();
#endif
  
  return SNMP_NO_ERROR;
}
  
SNMP_STATUS BcmSnmpTrapHandler::Register ()
{
  if (fRegistered == true)
    return SNMP_NO_ERROR;

  char Instance[128];
  // unsigned char IP[4];


  LockMib();
  
  SnmpLogStart << "BcmSnmpTrapHandler::Register" << endl;

  BcmSnmpTrapExploderThread::Singleton().RegisterTrapHandler(this);
    
  fRegistered = true;

#if (BCM_CONSOLE_SUPPORT)
  // Register myself with the command table.
  sprintf (Instance, "BcmSnmpTrapHandler name = '%s'", fName.c_str());
  GetCommandTableSingleton().AddInstance(Instance, this);
#endif

  UnlockMib();
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmSnmpTrapHandler::Start ()
{
  if (fStarted == true)
    return SNMP_NO_ERROR;
    
  SnmpLogStart << "BcmSnmpTrapHandler::Start" << endl;
  
  LockMib();
  
  Register();
  
  // Call non-portable SNMP agent entry point.  This function must
  // be supplied by the SNMP agent of choice.
  AgentStartSnmp (); 
  
  UnlockMib();
  
  fStarted = true;
  
  return SNMP_NO_ERROR;
}

bool BcmSnmpTrapHandler::ManagementEnabled () const
{
  if (fpMasterAgent)
    return fpMasterAgent->ManagementEnabled();

  return false;
}

void BcmSnmpTrapHandler::InformRestartPending ()
{
  SnmpLogInit << "BcmSnmpTrapHandler::InformRestartPending" << endl;

  // Give the non-portable agent a chance to do any necessary cleanup.
  // AgentInformRestartPending (GetIfNum());
}

void BcmSnmpTrapHandler::InformShutdownPending ()
{
  // Give the non-portable agent a chance to do any necessary cleanup.
  // AgentInformShutdownPending (GetIfNum());
}

SNMP_STATUS BcmSnmpTrapHandler::ConfigIp (const BcmIpAddress &IpAddr)
{
  LockMib();

  /* IP address */
  
  SnmpLogRaw << fName << " ConfigIp: Ip=" << IpAddr << endl;

  if (IpAddr != fIpAddr)
  {
    unsigned long SnmpIp;
   
    fIpAddr = IpAddr;
    
    fIpAddr.Get(SnmpIp);
    
  }
  else
    SnmpLogInit << "  Ip addr is the same, not rebinding." << endl;

  // BcmSnmpTrapExploderThread::Singleton().TrapHandlerStateChanged(this);
    
  UnlockMib();

  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmSnmpTrapHandler::SetUdpTrapPort (int UdpTrapPort)
{
  SnmpLogRaw << fName << " setting UDP port to " << UdpTrapPort << endl;

  if (UdpTrapPort != fUdpTrapPort)
  {
    // First, check the udpTable and make sure there is not already
    // a listener on this port for either our IP address or 0.0.0.0
    BcmMibTable *pUdpTable = NULL;
    if (fpMasterAgent)
      pUdpTable = fpMasterAgent->FindTable ("udpTable");
    if (pUdpTable)
    {
      // First check OurIp:UdpPort
      BcmObjectId Index = GetIpAddr();
      Index += UdpTrapPort;
      
      if (pUdpTable->FindEntry (Index) != NULL)
      {
        SnmpLogRaw << "  " << GetIpAddr() << ":" << UdpTrapPort << " already in use.  INCONSISTENT_VALUE." << endl;
        return SNMP_INCONSISTENT_VALUE;
      }

      // Next, try 0.0.0.0:UdpPort      
      Index = kAllZerosIp;
      Index += UdpTrapPort;
      
      if (pUdpTable->FindEntry (Index) != NULL)
      {
        SnmpLogRaw << "  0.0.0.0:" << UdpTrapPort << " already in use.  INCONSISTENT_VALUE." << endl;
        return SNMP_INCONSISTENT_VALUE;
      }
    }
  
    fUdpTrapPort = UdpTrapPort;

    // BcmSnmpTrapExploderThread::Singleton().TrapHandlerStateChanged(this);
  }
  else
    SnmpLogRaw << "  UDP Trap port is the same, not rebinding." << endl;

  return SNMP_NO_ERROR;
}

const BcmString &BcmSnmpTrapHandler::GetMasterAgentName () const
{
  return fpMasterAgent->GetName();
}

const BcmString &BcmSnmpTrapHandler::GetName () const
{
  return fName;
}

SNMP_STATUS BcmSnmpTrapHandler::WriteNonVolSettings (BcmNonVolSettings::NonVolSection Section, bool Async)
{
  SnmpLogRaw << GetName() << " doesn't know how to write non-vol settings!" << endl;  
  return SNMP_RESOURCE_UNAVAILABLE;
}

// Note that the reason we take an int & for SrcIfNum is because we might do a
// N2M lookup and map it if we get a 0, that way derived objects etc. can
// benefit from the lookup.
SNMP_STATUS BcmSnmpTrapHandler::AuthenticateRequest (int RequestType, int SnmpVersion, int &SrcIfNum, const BcmIpAddress &SrcIp, const BcmString &Community)
{
  SnmpLogNm << GetName() << " AuthenticateRequest: IP=" << SrcIp << ", If=" << SrcIfNum << ", community=" << Community  << endl;
 
  if (SrcIp == kSoftwareLoopbackIp)
  {
    SnmpLogNm << "  Internal request, allowing access." << endl;
    return SNMP_NO_ERROR;
  }
 
  return SNMP_NO_ACCESS;
}

#if (BCM_CONSOLE_SUPPORT)
// This static method allows an object (including myself) to get access to
// the singleton command table for controlling objects of my type.  This
// helps avoid order of initialization problems by ensuring that the
// object is created and initialized on-demand.
//
// Parameters:  None.
//
// Returns:
//      A reference to the command table singleton for my class.
//
BcmTrapHandlerCommandTable &BcmSnmpTrapHandler::GetCommandTableSingleton(void)
{
  static BcmTrapHandlerCommandTable gCommandTable;
  return gCommandTable;
}
#endif


//#define AGENT_MUTEX_DEBUG
#ifdef AGENT_MUTEX_DEBUG
bool RealLock = true;
extern ostream *pgMessageLogStream;
#endif
SNMP_STATUS BcmSnmpTrapHandler::LockMib (const char *pDebug)
{
#ifdef AGENT_MUTEX_DEBUG
  if (RealLock == false)
    return SNMP_NO_ERROR;
    
  if (BcmSnmpTrapHandler::gMessageLogSettings.GetEnabledSeverities () & BcmSnmpTrapHandler::kRequestMessages)
    MessageLogPrintThreadInfo(*pgMessageLogStream);
  SnmpLogReq << GetName() << " locking: " << pDebug << endl;
#endif

  if (pfMutex)
  {
    if (pfMutex->Lock() == true)
    {
      fMutexLockCount++;
    
#ifdef AGENT_MUTEX_DEBUG
      SnmpLogReq << "  LOCKED: " << fMutexLockCount << endl;
#endif
      return SNMP_NO_ERROR;
    }
    else
    {
#ifdef AGENT_MUTEX_DEBUG
      SnmpLogReq << "  LOCK FAILED! " << endl;
#endif
      return SNMP_GEN_ERR;
    }
  }
  
  return SNMP_RESOURCE_UNAVAILABLE;
}

SNMP_STATUS BcmSnmpTrapHandler::UnlockMib (const char *pDebug)
{
#ifdef AGENT_MUTEX_DEBUG
  if (RealLock == false)
    return SNMP_NO_ERROR;
    
  if (BcmSnmpTrapHandler::gMessageLogSettings.GetEnabledSeverities () & BcmSnmpTrapHandler::kRequestMessages)
    MessageLogPrintThreadInfo(*pgMessageLogStream);
  SnmpLogReq << GetName() << " unlocking: " << pDebug << endl;
#endif

  if (pfMutex)
  {
    if (pfMutex->Unlock() == true)
    {
      fMutexLockCount--;
    
#ifdef AGENT_MUTEX_DEBUG
      SnmpLogReq << "   UNLOCKED: " << fMutexLockCount << endl;
#endif
      return SNMP_NO_ERROR;
    }
    else
    {
#ifdef AGENT_MUTEX_DEBUG
      SnmpLogReq << "  UNLOCK FAILED! " << endl;
#endif
      return SNMP_GEN_ERR;
    }
  }
  
  return SNMP_RESOURCE_UNAVAILABLE;
}

int BcmSnmpTrapHandler::MibLockCount () const
{
  return fMutexLockCount;
}

int BcmSnmpTrapHandler::SendTrap (BcmTrap *pTrap)
{
  if (TrapsDeferred() == true)
  {
    SnmpLogEv << GetName() << " tanking up " << pTrap->Name() << " until after we're enabled..." << endl;
    return -1;
  }

  // If traps are not deferred, but we have a 0.0.0.0 IP address, don't bother.
  if (GetIpAddr() == kAllZerosIp)
  {
    SnmpLogEv << GetName() << " not sending " << pTrap->Name() << ", my IP=0.0.0.0" << endl;
    return 0;
  }

  // if (pTrap->Type() == AUTH_FAILURE_TRAP)
  // {
  // }

  // Even though we haven't really sent a trap, return 1 here so derived
  // classes can know that traps are a go.
  return 1;
}


bool BcmSnmpTrapHandler::TrapsDeferred () const
{
  return fTrapsDeferred;
}

void BcmSnmpTrapHandler::DeferTraps (bool Defer)
{
  fTrapsDeferred = Defer;
  
  if (Defer == true)
    SnmpLogRaw << GetName() << " defering traps." << endl;
  
  // If we are being un-deferred, go ahead and send any tanked traps.
  if (Defer == false)
    SendDeferredTraps();
}

int BcmSnmpTrapHandler::SendDeferredTraps ()
{
  SnmpLogRaw << GetName() << " sending deferred traps..." << endl;

  BcmTrap *pTrap, *pNextTrap;
  int SentTraps = 0;
  

  LockMib();

  // First, send any standard traps that we've tanked up.
  pTrap = fpDeferredTraps;
  while (pTrap)
  {
    if (SendTrap (pTrap) > 0)
      SentTraps++;
    
    pNextTrap = pTrap->fpNext;
    delete pTrap;
    pTrap = pNextTrap;
  }
  
  fpDeferredTraps = NULL;
  
  UnlockMib();
  
  SnmpLogRaw << "Done w/ deferred traps." << endl;
  
  return SentTraps;
}


// The command ids for the commands I can handle.
enum
{
    kTest           /* generic test command for whatever I need at the moment */ = BcmNonVolSettingsCommandTable::kDerivedClassCommands, 
    kDebug,         // Turn SNMP agent core debug on or off
    kShow,          // Print SNMP agent settings to console
    kLog,           // turn on/off various debugging
    kSendTrap,      // send a trap
    kThreadInfo     // print thread state stuff
};

#include "BoolCommandParameter.h"
#include "IpAddressCommandParameter.h"
#include "SignedIntCommandParameter.h"
#include "StringCommandParameter.h"
#include "StringSetCommandParameter.h"
#include "UnsignedIntCommandParameter.h"


// Constants for the command parameters.
enum
{
    kPrimaryParameter = 1,
    kSecondParameter,
    kThirdParameter,
    kFourthParameter
};

/* String set values: */

// kLog
enum
{
    kLogReq = 0,
    kLogThread
};

// kSendTrap
enum
{
  kColdStart = 0,
  kWarmStart,
  kLinkDown,
  kLinkUp,
  kAuthFail,
  kEgpNeighLoss,
  kEnterprise
};

void BcmSnmpTrapHandler::InstallCommands ()
{
  static bool CommandsInstalled = false;
  
  // Only install commands once.
  if (CommandsInstalled == true)
    return;
    
  CommandsInstalled = true;

  BcmCommand command;
    
  // Set up fields that don't change.
  command.fIsEnabled = true;
  command.fIsHidden = false;
  command.fCommandFunctionWithParms = GetCommandTableSingleton().CommandHandler;
 
#ifdef NOTYET 
  command.fCommandId = kTest;
  command.pfCommandName = "test";
  command.pfCommandParameterList = new BcmCommandParameterList;
  command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, true, "optional IP", 255, true));
  command.pfCommandHelp = "Test PR 6487.  To reproduce the crash from this PR, follow these steps:\n" \
                          "  1: Run as usual, become operational.\n" \
                          "  2: After going operational, cd to docsis and do an up_dis.\n" \
                          "  3: execute the test command in the SNMP directory.\n" \
                          "This should cause a crash in the PR 6487 style.  Note that if you specify\n" \
                          "an IP address as an argument, that IP will be used as the target.  If none\n" \
                          "is specified, 0.0.0.0 will be used.\n";
  command.pfExamples = "test\t";
  GetCommandTableSingleton().AddCommand(command);
#endif

  command.fCommandId = kDebug;
  command.pfCommandName = "agent_debug";
  command.pfCommandParameterList = new BcmCommandParameterList;
  command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, true, NULL, 0, 9));
  command.pfCommandHelp = "Set the debug level of the core agent.";
  command.pfExamples = "debug 0\tSet the core agent to be silent.\n"
                       "debug 9\tSet the core agent to max verbosity.";
  GetCommandTableSingleton().AddCommand(command);
  
  command.fCommandId = kShow;
  command.pfCommandName = "show";
  command.pfCommandHelp = "Print the trap handler's settings to the console.";
  command.pfExamples = "show\t";
  GetCommandTableSingleton().AddCommand(command);

  command.fCommandId = kLog;
  command.pfCommandName = "log";
  command.pfCommandParameterList = new BcmCommandParameterList;
  command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("req|thread", kPrimaryParameter, false));
  command.pfCommandParameterList->AddOrderDependent(new BcmBoolCommandParameter(kSecondParameter, false, "enable"));
  command.pfCommandHelp = "Enable or disable various kinds of logging.";
  command.pfExamples = "log req true\tEnable request debug.\n"
                       "log thread true\tEnable trap handler thread debug.\n";
  GetCommandTableSingleton().AddCommand(command);

#ifdef NOTYET
  command.fCommandId = kSendTrap;
  command.pfCommandName = "trap";
  command.pfCommandParameterList = new BcmCommandParameterList;
  command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("coldStart|warmStart|linkDown|linkUp|authFail", kPrimaryParameter, false));
  command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kSecondParameter, true, "snmpVersion", 1, 3));
  command.pfCommandParameterList->AddOrderDependent(new BcmIpAddressCommandParameter(kThirdParameter, true, "destIp"));
  command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kFourthParameter, true, "community", 255, true));
  command.pfCommandHelp = "Send the specified trap type using specified snmpVersion to destIp with specified community string\n"
      "\tsnmpVersion\t2=SNMPv1 trap\n"
      "\tsnmpVersion\t2=SNMPv2c trap\n"
      "\tsnmpVersion\t3=Notify";
  command.pfExamples = "trap coldStart 1 10.24.65.171 public\tSend a COLD_START SNMPv1 trap to 10.24.65.171 with community 'public'.";
  GetCommandTableSingleton().AddCommand(command);
#endif

  command.fCommandId = kThreadInfo;
  command.pfCommandName = "thread_info";
  command.pfCommandHelp = "Print the state of the SNMP threads and related info.";
  command.pfExamples = "thread_info\t";
  GetCommandTableSingleton().AddCommand(command);
  
}

#include "BcmSocket.h"
#include "SocketFactory.h"
void BcmSnmpTrapHandler::CommandHandler (const BcmCommand &command)
{
    const BcmCommandParameter *pParameter = NULL;
    
    // Many commands below have a single primary parameter.  Retrieve it for
    // them.
    if (command.pfCommandParameterList != NULL)
    {
        pParameter = command.pfCommandParameterList->Find(kPrimaryParameter);
    }

    switch (command.fCommandId)
    {
#ifdef NOTYET
        case kTest:
        {
            BcmIpAddress Target = kAllZerosIp;
            
            if (pParameter->WasParsed())
            {
              const char *pIpStr = pParameter->AsString();
              Target.Set(pIpStr);
            }
        
            SnmpLogRaw << "Testing PR 6487.  Sending packet to " << Target << endl;
            
            char *pTestPkt = (char *) "Testing 1,2,3";
            BcmSocket *pMySock = BcmSocketFactory::NewSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, 2);
            
            pMySock->Sendto (pTestPkt, sizeof(pTestPkt), 0, Target, 4413);
            
            delete pMySock;
/*        
            // Hah, this oughta keep 'em guessing...
            if (pParameter->WasParsed())
              SnmpLogRaw << "Test " << pParameter->AsString() << " passed." << endl;
            else
              SnmpLogRaw << "Test  passed." << endl;
*/
            break;
        }
#endif

        case kDebug:
        {
            unsigned int Level;
             
            if (pParameter->WasParsed())
            {
              Level = pParameter->AsUnsignedInt();
              Level = AgentSetDebugLevel (Level);
            }
            else
              Level = AgentGetDebugLevel ();
        
            SnmpLogRaw << "Core agent's (" << AgentGetName() << ") debug level = " << Level << endl;
            break;
        }

        case kShow:
        {
            SnmpLogRaw << *this << endl;
            break;
        }
        
        case kLog:
        {
            const BcmCommandParameter *pEnableParm = command.pfCommandParameterList->Find(kSecondParameter);
            int LogType = pParameter->AsUnsignedInt();
            unsigned long Severities;
        
        
            Severities = BcmSnmpTrapHandler::gMessageLogSettings.GetEnabledSeverities();
            
            if (pEnableParm->AsBool() == true)
            {
                if (LogType == kLogReq)
                {
                  SnmpLogRaw << "Enabling SNMP request debug." << endl;
                  Severities |= BcmMessageLogSettings::kInformationalMessages | BcmSnmpAgent::kRequestMessages;
                }
                else if (LogType == kLogThread)
                {
                  SnmpLogRaw << "Enabling thread debug." << endl;
                  Severities |= BcmMessageLogSettings::kInformationalMessages | BcmSnmpAgent::kThreadMessages;
                }
            }
            else
            {
                if (LogType == kLogReq)
                {
                    SnmpLogRaw << "Disabling request debug." << endl;
                    Severities &= ~BcmSnmpAgent::kRequestMessages;
                }
                else if (LogType == kLogThread)
                {
                    SnmpLogRaw << "Disabling thread debug." << endl;
                    Severities &= ~BcmSnmpAgent::kThreadMessages;
                }
            }
            
            BcmSnmpAgent::gMessageLogSettings.SetEnabledSeverities(Severities);
            
            break;
        }

#ifdef NOTYET
        case kSendTrap:
        {
            const BcmCommandParameter *pVersParam = command.pfCommandParameterList->Find(kSecondParameter);
            const BcmCommandParameter *pIpParam = command.pfCommandParameterList->Find(kThirdParameter);
            const BcmCommandParameter *pCmtyParam = command.pfCommandParameterList->Find(kFourthParameter);
            BcmTrap *pTrap;
            
            // If version, IP and community are not specified, just use the 
            // agent's internal mechanisms to send the trap wherever it is 
            // provisioned to be sent.  Eg use snmpTargetAddrTable if the agent 
            // uses that mechanism to send a trap.
            if ((pVersParam->WasParsed() == false)
            ||  (pIpParam->WasParsed() == false)
            ||  (pCmtyParam->WasParsed() == false))
            {
               //printf("SENDING ASYNC\n");
               //SendStandardTrap (pParameter->AsUnsignedInt());
               SendStandardTrap (pParameter->AsUnsignedInt(),true); //send async

               break;
            }
        
            switch ( pParameter->AsUnsignedInt() )
            {
                case kColdStart:
                    pTrap = new coldStartTrap(this);
                    break;
            
                case kWarmStart:
                    pTrap = new warmStartTrap(this);
                    break;
            
                case kLinkDown:
                    pTrap = new linkDownTrap(this, 0);
                    break;
            
                case kLinkUp:
                    pTrap = new linkUpTrap(this, 0);
                    break;
            
                case kAuthFail:
                    pTrap = new authFailureTrap(this);
                    break;
            
                default:
                    SnmpLogRaw << "No standard trap known of type " << pParameter->AsUnsignedInt() << endl;
                    return;
            }
        
            if ( pVersParam->AsUnsignedInt() == 1 )
                pTrap->SendV1Trap (GetIpAddr(), pIpParam->AsIpAddress(), pCmtyParam->AsString());
            else if ( pVersParam->AsUnsignedInt() == 2 )
                pTrap->SendV2Trap (GetIpAddr(), pIpParam->AsIpAddress(), pCmtyParam->AsString());
            else if ( pVersParam->AsUnsignedInt() )
                pTrap->SendNotify (GetIpAddr());
            else
                SnmpLogRaw << pVersParam->AsUnsignedInt() << " is not a valid trap version: 1, 2, or 3" << endl;
        
            delete pTrap;

            break;
        }
#endif

        case kThreadInfo:
        {
            SnmpLogRaw << BcmSnmpTrapExploderThread::Singleton() << endl;
            break;
        }
        
        default:
            gLogMessageRaw << "WARNING - unknown command id (" << command.fCommandId << ")!  Ignoring...\n";
            break;
    }
}
