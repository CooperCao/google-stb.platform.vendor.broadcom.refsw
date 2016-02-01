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
//    Filename: BcmSnmpAgent.cpp
//    Author:   Kevin O'Neal
//    Creation Date: 14-Jun-2001
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    BcmSnmpAgent base class
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
#include "BcmSnmpAgent.h"
#include "SnmpThread.h"
#include "MibBridge.h"
#include "MibFactory.h"
#include "systemBridge.h"
#include "HalIf.h"
#include "SocketFactory.h"
#include "standardTrap.h"

#if (WIFI80211_HAL_INCLUDED)
#include "dot11Mib.h"
#endif

// BRCM private runtime MIBs
#if (BCM_RUNTIMEMIB_SUPPORT)
#include "brcmSnmpMgmtBridge.h"
#endif

#include "ifMib.h"
#include "ipBridge.h"
#include "ifBridge.h"
#include "PacketSwitch.h"
#include "EthernetPacket.h"
#include "DhKickstartThread.h"
#if (CM_HAL_INCLUDED)
#include "dot1dBridge.h"
#endif
#if (ENET_HAL_INCLUDED)
#include "etherLikeMib.h"
#endif
#if (USB_HAL_INCLUDED)
#include "usbMib.h"
#endif


// Experimentally, I have found that 0 seems to be enough time for this.
// So to be extra-cautious, allow 50 msecs.
#define N2M_LEARN_MS 50
#ifndef BCM_STANDALONE_SNMP_AGENT
BcmIfUnknownProtosSnoop *BcmSnmpAgent::fpUnkProtosSnoop = NULL;
#endif

BcmMessageLogSettings BcmSnmpAgent::gMessageLogSettings("BcmSnmpAgent", "Snmp Agent message log");

BcmSnmpAgent::BcmSnmpAgent (char *pName, BcmSnmpAgent *pMasterAgent)
{
  static bool MsgLogRegistered = false;

  // Create a mutex for use in LockMib and UnlockMib methods.
  pfMutex = BcmOperatingSystemFactory::NewMutexSemaphore("BcmSnmpAgent's mutex");
  if (pfMutex == NULL)
    SnmpLogRaw << "WARNING: failed to create mutex for " << pName << endl;

  gLogMessageRaw << "Creating SNMP agent " << pName << endl;
  
  fName = pName;
  
  // Initialize to management disabled, until we start.
  EnableManagement (false);
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

  fpPktSwitch = NULL;
  fpDeferredTraps = NULL;
  fpIfBridge = NULL;
  fpIpAddrBridge = NULL;
  //fpIpFDSnoop = NULL;
#ifndef BCM_STANDALONE_SNMP_AGENT
  fpN2MSnoop = NULL;
#endif
  
  fIfNum = 0;
  fUdpPort = 161; // standard SNMP port unless told otherwise.
  fUdpTrapPort = 162; // standard SNMP trap port unless told otherwise.
  fStartTrap = COLD_START_TRAP;
  fRegistered = false;
  fStarted = false;
  fFactoryMode = false;
  
  fpMasterAgent = pMasterAgent;
  fpSubAgent = NULL;
  
  // Intialize system objects.  Most of these don't have good defaults,
  // so derived objects will need to change these to something meaningful.
  fSysDescr = fName;
  fSysObjectID = "0.0";
  fSysServices = 0x7F;
  fSysORID = "0.0";
  
  UnlockMib();
}

BcmSnmpAgent::~BcmSnmpAgent ()
{
  SnmpLogMem << "BcmSnmpAgent::~BcmSnmpAgent" << endl;

  Depopulate ();
  
  if (pfMutex)
  {
    delete pfMutex;
    pfMutex = NULL;
  }
/*  
  if (fpIpFDSnoop)
  {
    delete fpIpFDSnoop;
    fpIpFDSnoop = NULL;
  }
*/  
#ifndef BCM_STANDALONE_SNMP_AGENT
  if (fpN2MSnoop)
  {
    delete fpN2MSnoop;
    fpN2MSnoop = NULL;
  }
#endif

#if (BCM_CONSOLE_SUPPORT)
  // Deregister myself from the command table.
  GetCommandTableSingleton().RemoveInstance(this);
#endif

}

ostream &BcmSnmpAgent::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  char pView[64];
  
  
  memset (pView, 0, sizeof (pView));
  
  GetV1V2View (pView, sizeof(pView));
  
  outStream << "BcmSnmpAgent object '" << GetName() << "'" << endl;

  if (fpPktSwitch)
    outStream << "  Forwarder: " << fpPktSwitch->Name() << "(" << (void *)fpPktSwitch << ")" << endl;
  else
    outStream << "  Forwarder: NULL (don't worry, it's OK)" << endl;

  outStream << "  IP stack: " << GetIfNum() << endl;
  outStream << "  MAC addr: " << GetMacAddress() << endl;
  outStream << "  IP addr: " << GetIpAddr() << endl;
  outStream << "  Subnet: " << GetSubnetMask() << endl;
  outStream << "  Default router: " << GetRouter() << endl;
  outStream << "  UDP port: " << GetUdpPort() << endl;
  outStream << "  UDP trap port: " << GetUdpTrapPort() << endl;
  outStream << "  Current v1/v2c view: '" << pView << "'" << endl;
  outStream << "  Management enabled: " << ManagementEnabled() << endl;
  outStream << "  Traps deferred: " << TrapsDeferred() << endl;
  
  if (fpMasterAgent)
    outStream << "  Master agent: " << fpMasterAgent->GetName() << endl;

#ifdef SNMP_PROXY_SUPPORT
  if (fpSubAgent)
    outStream << "  Sub agent: " << fpSubAgent->GetName() << endl;
#endif
    
  switch (PacketSwitchOperMode())
  {
    case kNoPacketSwitching:
      outStream << "  Packet Switching: none" << endl;
      break;
      
    case kTransparentBridge:
      outStream << "  Packet switching: transparent bridge" << endl;
      break;
      
    case kSpanningTreeBridge:
      outStream << "  Packet switching: spanning tree bridge" << endl;
      break;
      
    case kRouter:
      outStream << "  Packet switching: router" << endl;
      break;
  }
    
  outStream << "  Factory mode: " << FactoryMode() << endl;
  outStream << "  Mutex locks: " << MibLockCount() << endl;
#endif

  return outStream;
}

SNMP_STATUS BcmSnmpAgent::Setup (const BcmMacAddress &Mac, int IfNum, BcmPacketSwitch *pSwitch)
{
  if (GetIfNum())
    return SNMP_RESOURCE_UNAVAILABLE;
    
  fIfNum = IfNum;
  fMacAddress = Mac;
  fpPktSwitch = pSwitch;
  fIpAddr = kBroadcastIp;
  
  // Setup any snoops
  SetupSnoops();

#if (BCM_CONSOLE_SUPPORT)  
  // Add commands to the command table.
  InstallCommands();
#endif
  
  return SNMP_NO_ERROR;
}
  
void BcmSnmpAgent::SetupSnoops ()
{
#ifndef BCM_STANDALONE_SNMP_AGENT

  if (fpPktSwitch)
  {
    BcmHalIf *pHal;
    
    // ipNetToMedia: IN/OUT on our IP stack
    pHal = fpPktSwitch->FindByIfNum (GetIfNum());
    if (pHal)
    {
      // Create the N2M snoop only if it's NULL, so derived classes can
      // create their own and still call the base class.
      if (fpN2MSnoop == NULL)
      {
        fpN2MSnoop = new BcmNetToMediaSnoop (false);
        pHal->AddSnoop(fpN2MSnoop, BcmHalIf::kInAndOutbound);
        fpN2MSnoop->SnmpAgent(this);
        fpN2MSnoop->PacketSwitch(fpPktSwitch);
      }
    }

    // ifInUnknownProtos: we only need one of these snoops to exist,
    // so this is a static class member.  Create it only if it's NULL.
    if (fpUnkProtosSnoop == NULL)
      fpUnkProtosSnoop = new BcmIfUnknownProtosSnoop (false);

    // Iterate through IF's and for any IP stack we find, install this snoop
    // if it's not already.
    for (int Loop = 0; Loop < fpPktSwitch->NumberOfRegisteredHalIf(); Loop++)
    {
      pHal = fpPktSwitch->Find (Loop);
      if (pHal == NULL)
        continue;

      if (pHal->IanaType() == BcmHalIf::kIanaTypeIpStack)
      {
        if (pHal->SnoopIsRegistered (fpUnkProtosSnoop, BcmHalIf::kOutbound) == false)
          pHal->AddSnoop(fpUnkProtosSnoop, BcmHalIf::kOutbound);
      }
    }

  }
#endif // BCM_STANDALONE_SNMP_AGENT
}


SNMP_STATUS BcmSnmpAgent::Register ()
{
  if (fRegistered == true)
    return SNMP_NO_ERROR;

  char Instance[128];
  // unsigned char IP[4];


  LockMib();
  
  SnmpLogStart << "BcmSnmpAgent::Register" << endl;

  BcmSnmpThread::Singleton().RegisterAgent(this);
    
  fRegistered = true;

  // fIpAddr.Get(IP[0],IP[1],IP[2],IP[3]);

#if (BCM_CONSOLE_SUPPORT)
  // Register myself with the command table.
  sprintf (Instance, "BcmSnmpAgent for IF %d, name = '%s'", GetIfNum(), fName.c_str());
  GetCommandTableSingleton().AddInstance(Instance, this);
#endif

  UnlockMib();
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmSnmpAgent::Start ()
{
  if (fStarted == true)
    return SNMP_NO_ERROR;
    
  SnmpLogStart << "BcmSnmpAgent::Start" << endl;
  
  LockMib();
  
  Register();
  
  // Call non-portable SNMP agent entry point.  This function must
  // be supplied by the SNMP agent of choice.
  AgentStartSnmp (); 
  
  // Populate our mibs...
  PopulateMibs();
  
  // Then bridges.
  PopulateBridges();
  
  // If we have a master agent, initialize our view to whatever his is.
  if (fpMasterAgent)
  {
    char pView[32];
    fpMasterAgent->GetV1V2View (pView, sizeof(pView));
    SetV1V2View (pView);
  }
  
  InstallViews ();
  
  UnlockMib();
  
  fStarted = true;
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmSnmpAgent::EnableFactoryMode (bool Enable)
{
  // If factory mode is being set to what it already is, do nothing and
  // return an error code.
  if (Enable == fFactoryMode)
    return SNMP_INCONSISTENT_VALUE;

  if ((Enable) && (!fFactoryMode))
    SnmpLogRaw << GetName() << " entering factory mode." << endl;
  else if ((!Enable) && (fFactoryMode))
    SnmpLogRaw << GetName() << " leaving factory mode." << endl;
  
  fFactoryMode = Enable;
  
  return SNMP_NO_ERROR;
}

bool BcmSnmpAgent::FactoryMode () const
{
  return fFactoryMode;
}

BcmPacketSwitch *BcmSnmpAgent::GetPacketSwitch () const
{
  return fpPktSwitch;
}

bool BcmSnmpAgent::ManagementEnabled () const
{
  return fMgmtEnabled;
}

void BcmSnmpAgent::EnableManagement (bool Enable)
{
  // If our state is not changing, take no action.
  if (Enable == ManagementEnabled())
    return;
    
  LockMib();

  fMgmtEnabled = Enable;
  
  if (Enable == true)
  {
    SnmpLogRaw << GetName() << " enabling management." << endl;
   
#ifndef BCM_STANDALONE_SNMP_AGENT 
    // Verify that our router is in the ipNetToMediaTable.  We have seen
    // cases (especially w/ the 'slim' load) where SetRouter is called
    // too quickly after we are configuring the IP stack and the discover
    // packet doesn't ever really go out.  So if we have not mapped the router,
    // do so now.
    // 4/24/03: I think all the timing issues have been resolved now so that
    // this should never be necessary, but still, doesn't hurt to make sure.
    if ((fRouter != kAllZerosIp)
    &&  (N2M().NetToMediaIf(fRouter) == NULL))
      N2M().LearnNetToMedia(fRouter);
#endif
    
    // fStartTrap is initialized in the constructor to COLD_START.
    // After the first one, send WARM_START's.  Note: directly set
    // fTrapsDeferred to false prior to sending the start trap rather
    // than calling DeferTraps(false).  The reason is because DeferTraps(false)
    // will cause any tanked up traps to go out, and we want the start trap
    // to go out before any deferred ones.
    fTrapsDeferred = false;
    SendStandardTrap (fStartTrap); //, 0, true); why was this async?
    fStartTrap = WARM_START_TRAP;
    
    // NOW call DeferTraps(false) which will send out any traps which were
    // previously deferred.
    DeferTraps (false);
  }
  else
  {
    SnmpLogRaw << GetName() << " disabling management." << endl;
    DeferTraps (true);
  }
  
  UnlockMib();
}

bool BcmSnmpAgent::PopulateMibs()
{
  BcmMibGroup *pGroup;
  
  if (fGroups.empty() != true)
  {
    SnmpLogStart << "Warning: someone is trying to re-populate " << GetName() << endl;
    return false;
  }
  
  // If we have a master agent, don't install the usual MIBs.  If a 'slave'
  // agent wants these MIBs for himself, he will need to override this method
  // and install them.
  if (fpMasterAgent)
    return true;
  
  SnmpLogStart << "Populating " << GetName() << endl;
   
  // Lock the mutex to protect against re-entrancy
  LockMib();

  // All agents support certain basic objects in general.  For agents which
  // don't want to support these or will be sharing them with a master agent,
  // this method will need to be overridden.
    
  pGroup = new systemGroup ();
  AddGroup (pGroup);

  pGroup = new interfacesGroup ();
  AddGroup (pGroup);
  
  pGroup = new ifMIBObjectsGroup ();
  AddGroup (pGroup);
 
  pGroup = new icmpGroup ();
  AddGroup (pGroup);
  
  pGroup = new ipGroup ();
  AddGroup (pGroup);
  
  pGroup = new udpGroup ();
  AddGroup (pGroup);

#ifndef BCM_STANDALONE_SNMP_AGENT
  // Iterate through the packet switch and install interface-specific
  // MIBs as necessary.  These are installed by the HAL objects, so by
  // default we'll assume that any agent with these kinds of interfaces
  // will support these MIBs.  As we add new kinds of supported interfaces,
  // this list may grow.
  if (fpPktSwitch)
  {
    unsigned int Loop;
    BcmHalIf *pHal;
    
    for (Loop = 0; Loop < fpPktSwitch->NumberOfRegisteredHalIf(); Loop++)
    {
      // Retrieve the interface.
      pHal = fpPktSwitch->Find(Loop);

      switch (pHal->IanaType())
      {
#if (ENET_HAL_INCLUDED)
        case BcmHalIf::kIanaTypeEthernet:
          pGroup = new dot3Group (); // ether-like mib
          AddGroup (pGroup);
          break;
#endif

#if (USB_HAL_INCLUDED)
        case BcmHalIf::kIanaTypeUsb:
          pGroup = new usbMibObjectsGroup ();
          AddGroup (pGroup);
          break;
#endif

#if (WIFI80211_HAL_INCLUDED)        
        case BcmHalIf::kIanaTypeHal80211:
          pGroup = new ieee802dot11Group ();
          AddGroup (pGroup);
          break;
#endif
      }
    }
  }

#if (CM_HAL_INCLUDED)
  // If we are a bridge, install dot1d group
  if ((PacketSwitchOperMode() == kTransparentBridge)
  ||  (PacketSwitchOperMode() == kSpanningTreeBridge))
  {
    pGroup = new dot1dBridgeGroup (); // bridge mib
    AddGroup (pGroup);
  }
#endif
  
// BRCM private runtime MIBs
#if (BCM_RUNTIMEMIB_SUPPORT)
    pGroup = new snmpMgmtGroup ();
    AddGroup (pGroup);
#endif

#endif // BCM_STANDALONE_SNMP_AGENT
  
  UnlockMib ();
  
  return true;
}

bool BcmSnmpAgent::PopulateBridges ()
{
  BcmMibBridge *pBridge;

  // If we have a master agent, don't install the usual MIBs.  If a 'slave'
  // agent wants these MIBs for himself, he will need to override this method
  // and install them.
  if (fpMasterAgent)
    return true;
  
  LockMib();
  
  // Set up the system group.
  pBridge = new systemGroupBridge(this);
  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
    delete (pBridge);
 
  pBridge = new sysOREntryBridge (1, this);
  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
    delete (pBridge);
    
  pBridge = BcmMibFactory::NewIpGroupBridge (this);
  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
    delete (pBridge);
    
  pBridge = BcmMibFactory::NewUdpGroupBridge (this);
  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
    delete (pBridge);

  // Set up the interfaces group.  Check to be sure this has not
  // already been done (like in a derived class), only do this if 
  // fpIfBridge == NULL.
  if (fpIfBridge == NULL)
  {
#ifdef BCM_STANDALONE_SNMP_AGENT
    fpIfBridge = new interfacesGroupBridge (this);
#else
    fpIfBridge = new interfacesGroupBridge (fpPktSwitch, this);
#endif
    if (fpIfBridge->Register(kDestroyMibObject) != SNMP_NO_ERROR)
    {
      delete fpIfBridge;
      fpIfBridge = NULL;
    }
    
#ifndef BCM_STANDALONE_SNMP_AGENT
    // Don't need to store this, just register it.
    pBridge = new ifMIBObjectsGroupBridge (this);
    if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
      delete (pBridge);
#endif
  }
  
  pBridge = BcmMibFactory::NewIcmpGroupBridge (this);
  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
    delete (pBridge);

  // If we have a SW loopback interface, create an entry for it in the
  // ipAddrTable.
  ifEntryBridge *pIf = IfBridge().FirstIfEntryOfType (BcmHalIf::kIanaTypeSoftwareLoopback);
  if (pIf)
  {
    pBridge = new ipAddrEntryBridge (kSoftwareLoopbackIp, 
                                     pIf->Get_ifIndex(),
                                     BcmIpAddress (255,0,0,0),
                                     0,
                                     65535,
                                     this);
    if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
      delete (pBridge);
  }

#ifndef BCM_STANDALONE_SNMP_AGENT
#if (CM_HAL_INCLUDED) 
  // If we are a bridge, create and register dot1d objects
  if ((PacketSwitchOperMode() == kTransparentBridge)
  ||  (PacketSwitchOperMode() == kSpanningTreeBridge))
  {
    pBridge = new dot1dBaseGroupBridge (this, GetMacAddress());
    if (pBridge->Register(kDestroyMibObject) != SNMP_NO_ERROR)
        delete pBridge;
  
/*        
  // Current implementation links dot1dTp group to CM HAL, so we can't
  // do this here.  Someday we should unhook this group from the
  // CM HAL.
  if (PacketSwitchOperMode() == kTransparentBridge)
  {
    pBridge  = new dot1dTpGroupBridge (fpDocsisCtlThread->DocsisCmHalIf(), this);
    if (pBridge->Register(kDestroyMibObject) != SNMP_NO_ERROR)
      delete pBridge;
  }
*/
  }
#endif
  
// BRCM private runtime MIBs
#if (BCM_RUNTIMEMIB_SUPPORT)
  pBridge = new snmpMgmtGroupBridge (this);
  if (pBridge->Register(kDestroyMibObject) != SNMP_NO_ERROR)
      delete pBridge;
#endif

#endif // BCM_STANDALONE_SNMP_AGENT

  UnlockMib();
  
  return true;
}

SNMP_STATUS BcmSnmpAgent::SetV1V2View (const char *pView)
{
  SnmpLogRaw << GetName() << " setting V1/V2 view to " << pView << endl;
  
  fV1V2View = pView;

  return SNMP_NO_ERROR;
}

void BcmSnmpAgent::GetV1V2View (char *pView, int MaxLen) const
{
  strncpy (pView, fV1V2View.c_str(), MaxLen-1);
}

SNMP_STATUS BcmSnmpAgent::SetV1V2View (const BcmString &View)
{
  SnmpLogRaw << GetName() << " setting V1/V2 view to " << View << endl;
  
  fV1V2View = View;
    
  return SNMP_NO_ERROR;
}

const BcmString &BcmSnmpAgent::GetV1V2View () const
{
  return fV1V2View;
}

SNMP_STATUS BcmSnmpAgent::InstallViews ()
{
  // No default views
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmSnmpAgent::DestroyViews ()
{
  SnmpLogRaw << GetName() << " destroying views..." << endl;
  return AgentCleanupViews ();
}

bool BcmSnmpAgent::Depopulate()
{
  BcmMibGroup *pGroup;
  MibGroupIterator Pos;


  SnmpLogInit << "BcmSnmpAgent::Depopulate" << endl;

  Pos = fGroups.begin();
  while (Pos != fGroups.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pGroup = (BcmMibGroup *) (*Pos).second;
#else
    pGroup = (BcmMibGroup *) *Pos;
#endif
    fGroups.erase (Pos++);
    delete (pGroup);
  }

  // Just in case we missed one, why not...
  fGroups.clear();

  return true;
}

void BcmSnmpAgent::InformRestartPending ()
{
  SnmpLogInit << "BcmSnmpAgent::InformRestartPending" << endl;

/*
  10/30/01: got rid of this general approach.  Necessary cleanup is now
  handled by the particular objects that are related to various MIB components
  instead of having the MIB components handle themselves.
  
  BcmMibGroup *pGroup;
  MibGroupIterator Pos;
  
  Pos = fGroups.begin();

  while (Pos != fGroups.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pGroup = (BcmMibGroup *) (*Pos).second;
#else
    pGroup = (BcmMibGroup *) *Pos;
#endif

    pGroup->InformRestartPending ();
    Pos++;
  }
*/

  // Give the non-portable agent a chance to do any necessary cleanup.
  AgentInformRestartPending (GetIfNum());
}

void BcmSnmpAgent::InformShutdownPending ()
{
  // Give the non-portable agent a chance to do any necessary cleanup.
  AgentInformShutdownPending (GetIfNum());
}

interfacesGroupBridge &BcmSnmpAgent::IfBridge() const
{
#ifdef BCM_STANDALONE_SNMP_AGENT
  static interfacesGroupBridge TempBridge (NULL);
#else
  static interfacesGroupBridge TempBridge (NULL, NULL);
#endif
  
  if (fpIfBridge == NULL)
  {
    if (fpMasterAgent)
      return fpMasterAgent->IfBridge();
      
    return TempBridge;
  }
  else
    return *fpIfBridge;
}

ifEntryBridge *BcmSnmpAgent::NewIfEntryBridge (BcmInterfaceCounters *pInterfaceCounters, int ifIndex, int SubIface, int ifType)
{
  return new ifEntryBridge (pInterfaceCounters, this, ifIndex, SubIface, ifType);
}

#ifndef BCM_STANDALONE_SNMP_AGENT

BcmNetToMediaSnoop &BcmSnmpAgent::N2M() const
{
  static BcmNetToMediaSnoop *pTempSnoop = NULL;
  
  if (fpN2MSnoop == NULL)
  {
    if (fpMasterAgent)
      return fpMasterAgent->N2M();
      
    if (pTempSnoop == NULL)
      pTempSnoop = new BcmNetToMediaSnoop (true);

    SnmpLogRaw << "*** WARNING: " << GetName() << " using temporary NetToMediaSnoop!" << endl;      
    return *pTempSnoop;
  }
  else
    return *fpN2MSnoop;
}

#endif

SNMP_STATUS BcmSnmpAgent::ConfigIp (const BcmIpAddress &IpAddr, const BcmIpAddress &SubnetMask, const BcmIpAddress &DefaultGateway)
{
  LockMib();

  /* IP address */
  
  SnmpLogRaw << fName << " ConfigIp: Ip=" << IpAddr << ", Subnet=" << SubnetMask << ", Gateway=" << DefaultGateway << endl;

  if (IpAddr != fIpAddr)
  {
    unsigned long SnmpIp;
   
    // If we already have an entry in the ipAddrTable, get ridda it.
    if (fpIpAddrBridge)
    {
      delete fpIpAddrBridge;
      fpIpAddrBridge = NULL;
    }
    
#ifndef BCM_STANDALONE_SNMP_AGENT
    // Flush the ipNetToMediaTable.
    N2M().FlushNetToMedia(GetIfNum());
#endif
    
    fIpAddr = IpAddr;
    
    fIpAddr.Get(SnmpIp);
    
    // Put our new IP in the ipAddrTable
    if ((fIpAddr != kAllZerosIp) && (fIpAddr != kBroadcastIp))
    {

	  // KB_TODO: CHECK TO SEE IF THIS IS VALID
	  fpIpAddrBridge = new ipAddrEntryBridge (fIpAddr, 
                                     fIfNum,
                                     SubnetMask,
                                     0,
                                     65535,
                                     this);

      // fpIpAddrBridge = new ipAddrEntryBridge (this);
      if (fpIpAddrBridge->Register (kDeleteBridge) != SNMP_NO_ERROR)
      {
        delete fpIpAddrBridge;
        fpIpAddrBridge = NULL;
      }
    }
  }
  else
    SnmpLogInit << "  Ip addr is the same, not rebinding." << endl;

  /* Subnet Mask */
  
  fSubnetMask = SubnetMask;
  
#ifndef BCM_STANDALONE_SNMP_AGENT
  if (fpIpAddrBridge)
    fpIpAddrBridge->fIpAdEntNetMask = fSubnetMask;
#endif
    
  /* Default Gateway */
  
  fRouter = DefaultGateway;
  
#ifndef BCM_STANDALONE_SNMP_AGENT
  // Learn the NetToMedia association for the router, provided it's
  // not 0.0.0.0 and has not already been learned.
  if ((DefaultGateway != kAllZerosIp)
  &&  (N2M().NetToMediaIf(DefaultGateway) == NULL))
  {
    // Give those IP settings a chance to kick in before proceeding...
    BcmOperatingSystemFactory::ThreadSleep(500);
    
    N2M().LearnNetToMedia (DefaultGateway);
    
    // Now a chance for our discovery packet to make it out
    BcmOperatingSystemFactory::ThreadSleep(500);
  }
  
  // Update the ipNetToMediaTable to make sure any stranded entries get added.
  // That is, any hosts we heard from prior to our subnet info getting set.
  N2M().UpdateNetToMedia ();
#endif
  
  // If we got 255.255.255.255 as an IP, disable management
  if (fIpAddr == kBroadcastIp)
    EnableManagement (false);
  // Otherwise, enable.
  else
    EnableManagement (true);

  BcmSnmpThread::Singleton().AgentStateChanged(this);
    
  UnlockMib();

  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmSnmpAgent::SetUdpPort (int UdpPort)
{
  SnmpLogRaw << fName << " setting UDP port to " << UdpPort << endl;

  if (UdpPort != fUdpPort)
  {
    // First, check the udpTable and make sure there is not already
    // a listener on this port for either our IP address or 0.0.0.0
    BcmMibTable *pUdpTable = FindTable ("udpTable");
    if (pUdpTable)
    {
      // First check OurIp:UdpPort
      BcmObjectId Index = GetIpAddr();
      Index += UdpPort;
      
      if (pUdpTable->FindEntry (Index) != NULL)
      {
        SnmpLogRaw << "  " << GetIpAddr() << ":" << UdpPort << " already in use.  INCONSISTENT_VALUE." << endl;
        return SNMP_INCONSISTENT_VALUE;
      }

      // Next, try 0.0.0.0:UdpPort      
      Index = kAllZerosIp;
      Index += UdpPort;
      
      if (pUdpTable->FindEntry (Index) != NULL)
      {
        SnmpLogRaw << "  0.0.0.0:" << UdpPort << " already in use.  INCONSISTENT_VALUE." << endl;
        return SNMP_INCONSISTENT_VALUE;
      }
    }
  
    fUdpPort = UdpPort;

    BcmSnmpThread::Singleton().AgentStateChanged(this);
  }
  else
    SnmpLogRaw << "  UDP port is the same, not rebinding." << endl;

  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmSnmpAgent::SetUdpTrapPort (int UdpTrapPort)
{
  SnmpLogRaw << fName << " setting UDP trap port to " << UdpTrapPort << endl;

  fUdpTrapPort = UdpTrapPort;

  return SNMP_NO_ERROR;
}


/* MOVED INLINE
int BcmSnmpAgent::GetIfNum ()
{
  return fIfNum;
}
*/

const BcmMacAddress &BcmSnmpAgent::GetMacAddress () const
{
  return fMacAddress;
}

const BcmString &BcmSnmpAgent::GetName () const
{
  return fName;
}

// By default, we'll return the default HAL for our packet switch.  Usually this
// is the right answer, but not always.  Derived objects might need to override
// this method.
int BcmSnmpAgent::GetIpAdEntIfIndex () const
{

#ifdef BCM_STANDALONE_SNMP_AGENT

  // For a standalone Snmp Agent, we only have one ifIndex, so ...

  return 1;

#else

  BcmHalIf *pHal;
  
  if (fpPktSwitch == NULL)
    return 0;

  pHal = fpPktSwitch->DefaultHalIf();
  if (pHal)
    return pHal->IfTableIndex(this);
    
  return 0;
#endif
}

SNMP_STATUS BcmSnmpAgent::WriteNonVolSettings (BcmNonVolSettings::NonVolSection Section, bool Async)
{
  SnmpLogRaw << GetName() << " doesn't know how to write non-vol settings!" << endl;  
  return SNMP_RESOURCE_UNAVAILABLE;
}

// System group accessors
const BcmString &BcmSnmpAgent::sysDescr () const
{
  static BcmString FactoryDescr;
  
  if (FactoryMode() == false)
    return fSysDescr;
  else
  {
    FactoryDescr = fSysDescr;
    FactoryDescr += " FACTORY MODE ENABLED!";
    return FactoryDescr;
  }
}

const BcmObjectId &BcmSnmpAgent::sysObjectID () const
{
  return fSysObjectID;
}

time_t BcmSnmpAgent::sysUpTime () const
{
  time_t Now;

  if (time (&Now) == (time_t) -1)
    return 0;

  return (Now - GetSystemStartTime());
}

const BcmString &BcmSnmpAgent::sysContact () const
{
  return fSysContact;
}

const BcmString &BcmSnmpAgent::sysName () const
{
  return fSysName;
}

const BcmString &BcmSnmpAgent::sysLocation () const
{
  return fSysLocation;
}

int BcmSnmpAgent::sysServices () const
{
  return fSysServices;
}

time_t BcmSnmpAgent::sysORLastChange () const
{
  return 0;
}

const BcmObjectId &BcmSnmpAgent::sysORID (int sysORIndex) const
{
  return fSysORID;
}

const BcmString &BcmSnmpAgent::sysORDescr (int sysORIndex) const
{
  return fSysORDescr;
}

time_t BcmSnmpAgent::sysORUpTime (int sysORIndex) const
{
  return 0;
}

// Only 3 of these are writable.  Note that base class will change the value,
// but not store to NV.  Derived objects should implement NV storage of these
// objects.

SNMP_STATUS BcmSnmpAgent::sysContact (const BcmString &SysContact)
{
  fSysContact = SysContact;
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmSnmpAgent::sysName (const BcmString &SysName)
{
  fSysName = SysName;
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmSnmpAgent::sysLocation (const BcmString &SysLocation)
{
  fSysLocation = SysLocation;
  return SNMP_NO_ERROR;
}

BcmSnmpAgent::PacketSwitchMode BcmSnmpAgent::PacketSwitchOperMode () const 
{ 
  // If we have a master agent, be the same as him.
  if (fpMasterAgent)
    return fpMasterAgent->PacketSwitchOperMode ();
  
  // By default we are a transparent bridge.  
  return kTransparentBridge; 
}

SNMP_STATUS BcmSnmpAgent::GetObject (const char *pName, BcmVarBind &Value, const BcmObjectId &Index)
{
  BcmMibObject *pObject;
 
  if (Index.Length() != 0)
    pObject = FindObject (BcmString(pName), Index);
  // If no index is specified, find the first occurence of the object
  else
    pObject = FindObject (BcmString(pName));

  if (pObject == NULL)
    return SNMP_NO_SUCH_NAME;
  
  return pObject->Get(Value);
}

SNMP_STATUS BcmSnmpAgent::GetIntObject (const char *pName, int &Value, const BcmObjectId &Index)
{
  BcmVarBind VB;
  SNMP_STATUS Status = GetObject (pName, VB, Index);
  
  // Go ahead and fill in the value, even if the Get above fails.
  // Mainly, this is for objects which are not-accessible, but we still
  // want to provide programatic access to.  However, we will still return
  // the error code.  Chance of bogus data being shoved into Value this way,
  // but the caller must be cautious when calling.
  VB.GetInt32 (&Value);
  
  return Status;  
}

SNMP_STATUS BcmSnmpAgent::GetUintObject (const char *pName, unsigned int &Value, const BcmObjectId &Index)
{
  BcmVarBind VB;
  SNMP_STATUS Status = GetObject (pName, VB, Index);
  
  // Go ahead and fill in the value, even if the Get above fails.
  // Mainly, this is for objects which are not-accessible, but we still
  // want to provide programatic access to.  However, we will still return
  // the error code.  Chance of bogus data being shoved into Value this way,
  // but the caller must be cautious when calling.
  VB.GetUint32 (&Value);
  
  return Status;  
}

SNMP_STATUS BcmSnmpAgent::GetOctetStringObject (const char *pName, unsigned char *pValue, int MaxLen, const BcmObjectId &Index)
{
  BcmVarBind VB;
  SNMP_STATUS Status = GetObject (pName, VB, Index);
  
  // Go ahead and fill in the value, even if the Get above fails.
  // Mainly, this is for objects which are not-accessible, but we still
  // want to provide programatic access to.  However, we will still return
  // the error code.  Chance of bogus data being shoved into Value this way,
  // but the caller must be cautious when calling.
  VB.GetString (pValue, MaxLen);
  
  return Status;
}

SNMP_STATUS BcmSnmpAgent::GetIpAddrObject (const char *pName, BcmIpAddress &Value, const BcmObjectId &Index)
{
  BcmVarBind VB;
  SNMP_STATUS Status = GetObject (pName, VB, Index);
  
  // Go ahead and fill in the value, even if the Get above fails.
  // Mainly, this is for objects which are not-accessible, but we still
  // want to provide programatic access to.  However, we will still return
  // the error code.  Chance of bogus data being shoved into Value this way,
  // but the caller must be cautious when calling.
  VB.GetIp (Value);
  
  return Status;
}

SNMP_STATUS BcmSnmpAgent::GetMacAddrObject (const char *pName, BcmMacAddress &Value, const BcmObjectId &Index)
{
  BcmVarBind VB;
  SNMP_STATUS Status = GetObject (pName, VB, Index);
  
  // Go ahead and fill in the value, even if the Get above fails.
  // Mainly, this is for objects which are not-accessible, but we still
  // want to provide programatic access to.  However, we will still return
  // the error code.  Chance of bogus data being shoved into Value this way,
  // but the caller must be cautious when calling.
  VB.GetMacAddr (Value);
  
  return Status;
}

SNMP_STATUS BcmSnmpAgent::GetOidObject (const char *pName, BcmObjectId &Value, const BcmObjectId &Index)
{
  BcmVarBind VB;
  SNMP_STATUS Status = GetObject (pName, VB, Index);
  
  // Go ahead and fill in the value, even if the Get above fails.
  // Mainly, this is for objects which are not-accessible, but we still
  // want to provide programatic access to.  However, we will still return
  // the error code.  Chance of bogus data being shoved into Value this way,
  // but the caller must be cautious when calling.
  VB.GetOid (Value);
  
  return Status;
}

SNMP_STATUS BcmSnmpAgent::SetObject (const char *pName, const BcmVarBind &Value, const BcmObjectId &Index)
{
  SNMP_STATUS Status;
  BcmMibObject *pObject;
  BcmVarBind Val = Value; // because Test and Set don't take consts
   
  if (Index.Length() != 0)
    pObject = FindObject (BcmString(pName), Index);
  // If no index is specified, find the first occurence of the object
  else
    pObject = FindObject (BcmString(pName));

  if (pObject == NULL)
    return SNMP_NO_SUCH_NAME;
    
  Status = pObject->Test (Val);
  if (Status != SNMP_NO_ERROR)
    return Status;
  
  return pObject->Set(Val);
}

SNMP_STATUS BcmSnmpAgent::SetIntObject (const char *pName, int Value, const BcmObjectId &Index)
{
  BcmVarBind VB;
  VB = Value;
  return SetObject (pName, VB, Index);
}

SNMP_STATUS BcmSnmpAgent::SetUintObject (const char *pName, unsigned int Value, const BcmObjectId &Index)
{
  BcmVarBind VB;
  VB = Value;
  return SetObject (pName, VB, Index);
}

SNMP_STATUS BcmSnmpAgent::SetOctetStringObject (const char *pName, const unsigned char *pValue, int MaxLen, const BcmObjectId &Index)
{
  BcmVarBind VB;
  VB.SetString (pValue, MaxLen);
  return SetObject (pName, VB, Index);
}

SNMP_STATUS BcmSnmpAgent::SetIpAddrObject (const char *pName, const BcmIpAddress &Value, const BcmObjectId &Index)
{
  BcmVarBind VB;
  VB = Value;
  return SetObject (pName, VB, Index);
}

SNMP_STATUS BcmSnmpAgent::SetMacAddrObject (const char *pName, const BcmMacAddress &Value, const BcmObjectId &Index)
{
  BcmVarBind VB;
  VB = Value;
  return SetObject (pName, VB, Index);
}

SNMP_STATUS BcmSnmpAgent::SetOidObject (const char *pName, const BcmObjectId &Value, const BcmObjectId &Index)
{
  BcmVarBind VB;
  VB = Value;
  return SetObject (pName, VB, Index);
}

SNMP_STATUS BcmSnmpAgent::AddGroup (BcmMibGroup *pGroup)
{
  BcmObjectId Oid;

  pGroup->SetAgent (this);
  pGroup->GetOid (Oid);
  
  LockMib(); 

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
  fGroups[Oid] = pGroup;
#else 
  fGroups.push_back (pGroup);
#endif

  SnmpLogInit << "Group " << pGroup->Name() << " added" << endl;
  
  UnlockMib(); 

  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmSnmpAgent::RemoveGroup (BcmMibGroup *pGroup)
{
  BcmMibGroup  *pCheckGroup;
  MibGroupIterator Pos;
  SNMP_STATUS Status = SNMP_NO_SUCH_NAME;

  LockMib(); 

  // Spin through all groups and see if we find the one passed in.
  Pos = fGroups.begin();
  while (Pos != fGroups.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pCheckGroup = (BcmMibGroup *) (*Pos).second;
#else
    pCheckGroup = (BcmMibGroup *) *Pos;
#endif    
    if (pCheckGroup == pGroup)
    {
      fGroups.erase(Pos++);
      Status = SNMP_NO_ERROR;
    }
    else
      Pos++;
  }
  
  UnlockMib(); 

  return Status;
}

bool BcmSnmpAgent::IsObjectSupported (const BcmObjectId &OID) const
{
  ObjectId AgentOID;
  
  OID.GetOID (&AgentOID);
  
  // The objects supported by this object will always be a subset of what's
  // supported by the agent core.  So if he doesn't support it, neither do
  // we.
  if (AgentIsObjectSupported(&AgentOID, GetV1V2View().c_str()) == 0)
    return false;
    
  // If the agent core does support this object, then we still might not - it
  // might be another instance of this class which does.  So now see if we
  // personally support this.  If we do, then return true.  Note that we are
  // looking here for support of the object's base OID, sans index.  So we can
  // just call FindOtherInstance.
  if (FindOtherInstance (OID))
    return true;

  // NOW, at this point, there are 2 possibilties.  Either the object in 
  // question is supported at the V2 level on another agent object, or else 
  // it's supported natively by the core agent and we support it too.  So 
  // figure out which is the case.  Check all other agents AT THE V2 LEVEL
  // ONLY via FindOtherInstace, and if any of them have V2 support of the
  // object, then we return false.  Otherwise, we return true.
  for (int Loop = 0; Loop < BcmSnmpThread::Singleton().NumAgents(); Loop++)
  {
    BcmSnmpAgent *pAgent = BcmSnmpThread::Singleton().GetAgent(Loop);
    
    if ((pAgent != NULL) &&  (pAgent != this))
    {
      if (pAgent->FindOtherInstance(OID))
        return false;
    }
  }
  
  return true;
}

// Note that the reason we take an int & for SrcIfNum is because we might do a
// N2M lookup and map it if we get a 0, that way derived objects etc. can
// benefit from the lookup.
SNMP_STATUS BcmSnmpAgent::AuthenticateRequest (int RequestType, int SnmpVersion, int &SrcIfNum, const BcmIpAddress &SrcIp, const BcmString &Community)
{
  SnmpLogNm << GetName() << " AuthenticateRequest: IP=" << SrcIp << ", If=" << SrcIfNum << ", community=" << Community  << endl;
 
#ifdef BCM_STANDALONE_SNMP_AGENT 
  // If SrcIfNum is 0, that means we don't know what interface the packet
  // came in on.  Assume 1.
  if (SrcIfNum == 0)
    SrcIfNum = 1;
#else
  // If SrcIfNum is 0, that means we don't know what interface the packet
  // cam in on.  Get the info from our net to media map.
  if (SrcIfNum == 0)
  {
    const BcmHalIf *pHal = N2M().NetToMediaIf (SrcIp);
    if (pHal)
    {
      SrcIfNum = pHal->IfTableIndex(this);
      SnmpLogNm << "  Mapped IP " << SrcIp << " to IF " << SrcIfNum << endl;
    }
    else
      SnmpLogNm << "  Couldn't map interface for " << SrcIp << endl;
  }
#endif
  
  if (SrcIp == kSoftwareLoopbackIp)
  {
    SnmpLogNm << "  Internal request, allowing access." << endl;
    return SNMP_NO_ERROR;
  }
 
  // By default all we will do here is check to see if we're disabled, and
  // return NO_ACCESS if we are.  Derived classes should override this method
  // to apply any authentication necessary.
  if (ManagementEnabled() == false)
  {
    SnmpLogNm << "  Management disabled: DISCARD" << endl;
    return SNMP_NO_ACCESS;
  }
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmSnmpAgent::ValidateSet (const BcmObjectId &OID, const BcmVarBind &VB, const BcmIpAddress &SrcIp, int SnmpVersion, const BcmString &Community)
{
  // If we have a master agent, do his bidding.  Otherwise, base class here
  // will check writability and type (if known).
  if (fpMasterAgent)
    return fpMasterAgent->ValidateSet (OID, VB, SrcIp, SnmpVersion, Community);
    
  BcmMibObject *pObject;
  BcmObjectId Index, FindOID = OID; // Dummy index, don't care for this purpose anyway.
  BcmMibRequest Req(BcmMibRequest::kValidateSetRequest, SnmpVersion, 0, OID, Index, SrcIp, GetIpAddr(), Community, this);

  LockMib();

  SnmpLogReq << Req << " value to validate is " << VB << "  Agent=" << fName << endl;

  // Try to find the object of interest.  Note that we must create a new OID
  // because FindObject can't take a const OID.
  pObject = FindObject (FindOID);
  if (pObject)
  {
    SNMP_STATUS Status = SNMP_NO_ERROR;

/*  Don't check this here.  Clobbers some sneaky factory stuff.  
    if (pObject->IsAccessible(&Req) == false)
    {
      SnmpLogReq << "Found object, but it's not accessible. SNMP_NOT_WRITABLE." << endl;
      UnlockMib();

      return SNMP_NOT_WRITABLE;
    }
*/

    SnmpLogReq << "Found object " << *pObject << ", checking type and writability." << endl;

    if (pObject->Access() != BcmMibObject::kUnspecified)
    {
      // Check access.
      if ((pObject->Access() != BcmMibObject::kReadWrite) 
      &&  (pObject->Access() != BcmMibObject::kReadCreate))
        Status = SNMP_NOT_WRITABLE;
        
      // Check type.  Don't do this unless we know the access!  Don't wanna
      // return wrongType when we oughta return notWritable!  Easiest way
      // to do this is with a 'GET'
      else
      {
        BcmVarBind TypeVB;
        
        pObject->Get(TypeVB);
        if (TypeVB.GetType() != VB.GetType())
          Status = SNMP_WRONG_TYPE;
      }
    }
    else
      SnmpLogReq << "  Object has unspecified access.  Can't validate access/type." << endl;

    if (Status == SNMP_NO_ERROR)
      SnmpLogReq << "  Object " << pObject->Name() << " validated OK." << endl;
    else
      SnmpLogReq << "  Object " << pObject->Name() << " failed validation w/ status " << Status <<endl;

    UnlockMib();

    return Status;
  }
  else
    SnmpLogReq << "  Couldn't find object, assume NO_ERROR." << endl;
    
  UnlockMib();
    
  return SNMP_NO_ERROR;
}

BcmMibObject *BcmSnmpAgent::FindObject (BcmObjectId &OID)
{
  BcmMibGroup  *pGroup;
  BcmMibObject *pObject;
  MibGroupIterator Pos;
  BcmString OidStr;

  
  OID.AsString (OidStr);
  
  // Lock the mutex for thread safety
  LockMib (); 

  // Spin through all groups and check each one that is an
  // ascendant of the OID we got passed in.  Note that it IS possible that
  // multiple groups will be ascendants of this OID...
  Pos = fGroups.begin();
  while (Pos != fGroups.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pGroup = (BcmMibGroup *) (*Pos).second;
#else
    pGroup = (BcmMibGroup *) *Pos;
#endif    
    SnmpLogInit << "Searching for OID in group " << pGroup->Name() << endl;
    
    // Use this line to search via string comparisons, which is actually
    // faster now that each mib object doesn't store its own OID in order
    // to conserve memory.
    pObject = pGroup->FindObject (OidStr.c_str());
    
    // Use this line to search via OID object comparisons, which unfortunately
    // now is much slower since we must build each object's OID when it's
    // requested...
    // pObject = pGroup->FindObject (OID);

    if (pObject)
    {
      UnlockMib();  
      return pObject;
    }
    else
      SnmpLogInit << "No match in group " << pGroup->Name() << endl;

    Pos++;
  }
  
  UnlockMib();
  
  // If we didn't find the object in our own info base, try our
  // master agent.
  if (fpMasterAgent) 
    return fpMasterAgent->FindObject (OID);

  return NULL;
}

BcmMibObject *BcmSnmpAgent::FindObject (const BcmString &Name, const BcmObjectId &Index)
{
  BcmMibGroup  *pGroup;
  BcmMibObject *pObject;
  MibGroupIterator Pos;

  
  LockMib(); 

  // Spin through all groups and check each one that is an
  // ascendant of the OID we got passed in.  Note that it IS possible that
  // multiple groups will be ascendants of this OID...
  Pos = fGroups.begin();
  while (Pos != fGroups.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pGroup = (BcmMibGroup *) (*Pos).second;
#else
    pGroup = (BcmMibGroup *) *Pos;
#endif    
    SnmpLogInit << "Searching for object " << Name << " index " << Index << " in group " << fName << endl;
    pObject = pGroup->FindObject (Name, Index);

    if (pObject)
    {
      UnlockMib(); 
      return pObject;
    }
    else
      SnmpLogInit << "No match in group " << fName << endl;

    Pos++;
  }
  
  UnlockMib();
  
  // If we didn't find the object in our own info base, try our
  // master agent.
  if (fpMasterAgent) 
    return fpMasterAgent->FindObject (Name, Index);

  return NULL;
}

BcmMibObject *BcmSnmpAgent::FindObject (const BcmString &Name)
{
  BcmMibGroup  *pGroup;
  BcmMibObject *pObject;
  MibGroupIterator Pos;


  // Lock the mutex for thread safety
  LockMib();
    
  // Spin through all groups and check each one that is an
  // ascendant of the OID we got passed in.  Note that it IS possible that
  // multiple groups will be ascendants of this OID...
  Pos = fGroups.begin();
  while (Pos != fGroups.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pGroup = (BcmMibGroup *) (*Pos).second;
#else
    pGroup = (BcmMibGroup *) *Pos;
#endif    
    SnmpLogInit << "Searching for first occurence of object " << Name << " in group " << fName << endl;
    pObject = pGroup->FindObject (Name);

    if (pObject)
    {
      UnlockMib();
      return pObject;
    }
    else
      SnmpLogInit << "No match in group " << fName << endl;

    Pos++;
  }
  
  UnlockMib();
  
  // If we didn't find the object in our own info base, try our
  // master agent.
  if (fpMasterAgent) 
    return fpMasterAgent->FindObject (Name);

  return NULL;
}

BcmMibTable *BcmSnmpAgent::FindTable (BcmObjectId &OID)
{
  BcmMibGroup *pGroup;
  BcmMibTable *pTable;
  MibGroupIterator Pos;


  //if (fGroups.empty())
    //return NULL;
    
  // Lock the mutex for thread safety
  LockMib();

  // Spin through all groups and find the one that is an
  // ascendant of the OID we got passed in.
  Pos = fGroups.begin();
  while (Pos != fGroups.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pGroup = (BcmMibGroup *) (*Pos).second;
#else
    pGroup = (BcmMibGroup *) *Pos;
#endif
    
    // Don't call this here, we need to check all groups since some
    // (eg docsDev) might be registered at multiple locations...
    // BcmObjectId MatchOid;
    // pGroup->GetOid (MatchOid);
    // if (MatchOid.IsAscendant (OID))
    {
      pTable = pGroup->FindTable (OID);

      if (pTable)
      {
        UnlockMib();
        return pTable;
      }
    }

    Pos++;
  }
  
  UnlockMib();
  
  // If we didn't find the table in our own info base, try our
  // master agent.
  if (fpMasterAgent) 
    return fpMasterAgent->FindTable (OID);

  return NULL;
}

BcmMibTable *BcmSnmpAgent::FindTable (const BcmString &Name)
{
  BcmMibGroup *pGroup;
  BcmMibTable *pTable;
  MibGroupIterator Pos;


  //if (fGroups.empty())
    //return NULL;
    
  // Lock the mutex for thread safety
  LockMib(); 

  // Spin through all groups and find the one that is an
  // ascendant of the OID we got passed in.
  Pos = fGroups.begin();
  while (Pos != fGroups.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pGroup = (BcmMibGroup *) (*Pos).second;
#else
    pGroup = (BcmMibGroup *) *Pos;
#endif
    pTable = pGroup->FindTable (Name);

    if (pTable)
    {
      UnlockMib(); 
      return pTable;
    }

    Pos++;
  }
  
  UnlockMib();
  
  // If we didn't find the table in our own info base, try our
  // master agent.
  if (fpMasterAgent) 
    return fpMasterAgent->FindTable (Name);

  return NULL;
}


BcmMibGroup *BcmSnmpAgent::FindGroup (const BcmObjectId &OID)
{
  BcmMibGroup *pGroup;
  MibGroupIterator Pos;
  BcmObjectId MatchOid;


  // Lock the mutex for thread safety
  LockMib();

  // Spin through all groups and find the one that is an
  // ascendant of the OID we got passed in.
  Pos = fGroups.begin();
  while (Pos != fGroups.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pGroup = (BcmMibGroup *) (*Pos).second;
#else
    pGroup = (BcmMibGroup *) *Pos;
#endif
    pGroup->GetOid (MatchOid);
    
    if (MatchOid == OID)
    {
      UnlockMib();
      return pGroup;
    }
    
    // Don't call this here, we need to check all groups since some
    // (eg docsDev) might be registered at multiple locations...
    // if (MatchOid.IsAscendant (OID))
    {
      pGroup = pGroup->FindGroup (OID);

      if (pGroup)
      {
        UnlockMib();
        return pGroup;
      }
    }

    Pos++;
  }
  
  UnlockMib();
  
  // If we didn't find the table in our own info base, try our
  // master agent.
  if (fpMasterAgent) 
    return fpMasterAgent->FindGroup (OID);

  // If all else fails...
  return NULL;
}


BcmMibObject *BcmSnmpAgent::FindOtherInstance (const BcmObjectId &OID, const BcmMibRequest *pReq) const
{
  BcmMibGroup *pGroup;
  BcmMibObject *pObj = NULL;
  MibGroupConstIterator GPos;
  
  GPos = fGroups.begin();
  while (GPos != fGroups.end())
  {
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pGroup = (BcmMibGroup *) (*GPos).second;
#else
    pGroup = (BcmMibGroup *) *GPos;
#endif
    
    pObj = pGroup->FindOtherInstance (OID, pReq);
    if (pObj)
      return pObj;

    GPos++;
  }
  
  // If we didn't find the object in our own info base, try our
  // master agent.
  if (fpMasterAgent) 
    return fpMasterAgent->FindOtherInstance (OID, pReq);
  
  return NULL;
}


SNMP_STATUS BcmSnmpAgent::RegisterTabular (TabularMibBridge *pBridge, bool DeleteUponDestruction)
{
  SNMP_STATUS Status;
  BcmMibTable *pTable;
  BcmObjectId OID;


  pBridge->GetOid (OID);
  SnmpLogInit << "BcmSnmpAgent::RegisterTabular for OID " << OID << endl;
  
  // Lock the mutex for thread safety
  LockMib();
  
  // Find the table with the OID of the bridge passed in, and register
  // a new entry for it.
  pTable = FindTable (OID);

  if (pTable == NULL)
  {
    SnmpLogInit << "BcmSnmpAgent::RegisterTabular: No table match for" << OID << endl;
    UnlockMib();
    
    return SNMP_NO_SUCH_NAME;
  }

  SnmpLogInit << "BcmSnmpAgent::RegisterTabular: matched table " << pTable->Name() << endl;
  Status = pTable->RegisterBridge (pBridge, DeleteUponDestruction);
  SnmpLogInit << "RegisterTabular returned " << Status << endl;
  
  UnlockMib();

  return Status;
}

SNMP_STATUS BcmSnmpAgent::RegisterScalar (ScalarMibBridge *pBridge, bool DeleteUponDestruction)
{
/*
  BcmObjectId OID;
  BcmMibGroup *pGroup;
  MibGroupIterator Pos;
  SNMP_STATUS Status = SNMP_NO_SUCH_NAME;
  BcmObjectId MatchOid;


  pBridge->GetOid (OID);

  LockMib();

  // Use this bridge for every scalar object whose OID is 
  // a descendant of the bridge's OID.  Iterate through all groups
  // and register this scalar bridge with any group whose OID is a
  // descendant of the bridge's, or whose OID equals the bridge's
  Pos = fGroups.begin();

  while (Pos != fGroups.end())
  {
    // We might need to apply this bridge if the OID of a group
    // equal to, ascendant of, or descendant of the bridge OID.
    // Actually, we will still only register with a single group,
    // but because groups may contain sub-groups, we still need to 
    // call RegisterBridge on any group that is ascendant or descendant
    // of the bridge OID.  If they contain no subgroup which matches the
    // OID, the call will fail and no harm done.
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pGroup = (BcmMibGroup *) (*Pos).second;
#else
    pGroup = (BcmMibGroup *) *Pos;
#endif
    pGroup->GetOid(MatchOid);

    if ((MatchOid == OID)
    ||  (MatchOid.IsAscendant (OID))
    ||  (MatchOid.IsDescendant (OID)))
    {
      if (pGroup->RegisterBridge (pBridge, DeleteUponDestruction) == SNMP_NO_ERROR)
      {
        Status = SNMP_NO_ERROR;
        break;
      }
    }

    Pos++;
  }
  
  UnlockMib();

  return Status;
*/

  SNMP_STATUS Status;
  BcmMibGroup *pGroup;
  BcmObjectId OID;


  pBridge->GetOid (OID);
  SnmpLogInit << "BcmSnmpAgent::RegisterScalar for OID " << OID << endl;
  
  // Lock the mutex for thread safety
  LockMib();
  
  // Find the group with the OID of the bridge passed in, and register with it.
  pGroup = FindGroup (OID);

  if (pGroup == NULL)
  {
    SnmpLogInit << "BcmSnmpAgent::RegisterScalar: No group match for" << OID << endl;
    UnlockMib();
    
    return SNMP_NO_SUCH_NAME;
  }

  SnmpLogInit << "BcmSnmpAgent::RegisterScalar: matched group " << pGroup->Name() << endl;
  Status = pGroup->RegisterBridge (pBridge, DeleteUponDestruction);
  SnmpLogInit << "RegisterScalar returned " << Status << endl;
  
  UnlockMib();

  return Status;
}

SNMP_STATUS BcmSnmpAgent::DeRegisterTabular (TabularMibBridge *pBridge)
{
  SNMP_STATUS Status;
  BcmMibTable *pTable;
  BcmObjectId OID;


  SnmpLogInit << "BcmSnmpAgent::DeRegisterTabular for OID " << OID << endl;

  pBridge->GetOid (OID);
  
  LockMib();

  // Find the table with the OID of the bridge passed in, and register
  // a new entry for it.
  pTable = FindTable (OID);

  if (pTable == NULL)
  {
    SnmpLogInit << "BcmSnmpAgent::DeRegisterTabular: No table match for " << OID << endl;
    UnlockMib();
      
    return SNMP_NO_SUCH_NAME;
  }

  SnmpLogInit << "BcmSnmpAgent::DeRegisterTabular: matched table " << pTable->Name() << endl;
  Status = pTable->DeRegisterBridge (pBridge);
  SnmpLogInit << "DeRegisterTabular returned " << Status << endl;
  
  UnlockMib();

  return Status;
}

SNMP_STATUS BcmSnmpAgent::DeRegisterScalar (ScalarMibBridge *pBridge)
{
  BcmObjectId MatchOid, OID;
  BcmMibGroup *pGroup;
  MibGroupIterator Pos;


  pBridge->GetOid (OID);

  LockMib();
    
  // Use this bridge for every scalar object whose OID is 
  // a descendant of the bridge's OID.  Iterate through all groups
  // and register this scalar bridge with any group whose OID is a
  // descendant of the bridge's, or whose OID equals the bridge's
  Pos = fGroups.begin();

  while (Pos != fGroups.end())
  {
    // We might need to apply this bridge if the OID of a group
    // equal to, ascendant of, or descendant of the bridge OID.
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
    pGroup = (BcmMibGroup *) (*Pos).second;
#else
    pGroup = (BcmMibGroup *) *Pos;
#endif
    pGroup->GetOid (MatchOid);

    if ((MatchOid == OID)
    ||  (MatchOid.IsAscendant (OID))
    ||  (MatchOid.IsDescendant (OID)))
      pGroup->DeRegisterBridge (pBridge);

    Pos++;
  }
  
  UnlockMib();

  return SNMP_NO_ERROR;
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
BcmSnmpCommandTable &BcmSnmpAgent::GetCommandTableSingleton(void)
{
  static BcmSnmpCommandTable gCommandTable;
  return gCommandTable;
}
#endif


//#define AGENT_MUTEX_DEBUG
#ifdef AGENT_MUTEX_DEBUG
bool RealLock = true;
extern ostream *pgMessageLogStream;
#endif
SNMP_STATUS BcmSnmpAgent::LockMib (const char *pDebug)
{
#ifdef AGENT_MUTEX_DEBUG
  if (RealLock == false)
    return SNMP_NO_ERROR;
    
  if (BcmSnmpAgent::gMessageLogSettings.GetEnabledSeverities () & BcmSnmpAgent::kRequestMessages)
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

SNMP_STATUS BcmSnmpAgent::UnlockMib (const char *pDebug)
{
#ifdef AGENT_MUTEX_DEBUG
  if (RealLock == false)
    return SNMP_NO_ERROR;
    
  if (BcmSnmpAgent::gMessageLogSettings.GetEnabledSeverities () & BcmSnmpAgent::kRequestMessages)
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

int BcmSnmpAgent::MibLockCount () const
{
  return fMutexLockCount;
}

SNMP_STATUS BcmSnmpAgent::GetObject (BcmObjectId &OID, BcmVarBind &VB, BcmMibRequest *pReq)
{
  BcmMibObject *pObject;
  SNMP_STATUS Status;

/*  
  // Check for the unlikely but possible case of the router N2M mapping failing 
  // at both ConfigIp AND EnableManagement.  If it did, try again.
  if ((fRouter != kAllZerosIp)
  &&  (N2M().NetToMediaIf(fRouter) == NULL))
  {
    N2M().LearnNetToMedia(fRouter);
    
    // Wait awhile so the learning process has time to complete...
    BcmOperatingSystemFactory::ThreadSleep(N2M_LEARN_MS);
  }
*/

  LockMib();

  if (pReq)
    SnmpLogReq << *pReq << "  Agent=" << fName << endl;
  else
    SnmpLogReq << "GET request for OID " << OID << endl;

  pObject = FindObject (OID);

  if (pObject == NULL)
  {
#ifdef SNMP_PROXY_SUPPORT
    // If we have a sub-agent, try looking for the object on him.
    if (fpSubAgent) {
      SNMP_STATUS SubAgentReturnStatus = fpSubAgent->GetObject(OID, VB, pReq);
      UnlockMib();
      return SubAgentReturnStatus;
    }
#endif
    
    // No match, we need to either return NO_SUCH_OBJECT (aka NO_SUCH_NAME) or 
    // NO_SUCH_INSTANCE.  Which we return depends on whether we have an object 
    // with the base OID but not of that index, or whether we have no object 
    // even of that base OID.
    pObject = FindOtherInstance (OID, pReq);
    UnlockMib();
    
    if (pObject)
      return SNMP_NO_SUCH_INSTANCE;

    return SNMP_NO_SUCH_NAME;
  }
  else
    SnmpLogReq << "  Matched object " << pObject->Name() << endl;;

  if (pObject->IsAccessible(pReq) == false)
  {
    SnmpLogReq << "  Object is not accessible: NO_SUCH_NAME" << endl;
    UnlockMib();

    return SNMP_NO_SUCH_NAME;
  }

  if (pObject->IsVisible() == false)
  {
    SnmpLogReq << "  Object is not visible: NO_SUCH_NAME" << endl;
    UnlockMib();

    return SNMP_NO_SUCH_NAME;
  }

  Status = pObject->Get(VB);

  if (Status == SNMP_NO_ERROR)
    SnmpLogReq << " Value = " << VB << endl;
  else
    SnmpLogReq << " Get failed, status = " << Status << endl;

  UnlockMib();

  return Status;
}

SNMP_STATUS BcmSnmpAgent::NextObject (BcmObjectId &OID, BcmObjectId &Index, BcmVarBind &VB, BcmMibRequest *pReq)
{
  BcmMibTable  *pTable;
  BcmMibEntry  *pEntry;
  BcmMibObject *pObject;
  BcmObjectId WholeOID;

/*
  // Check for the unlikely but possible case of the router N2M mapping failing 
  // at both ConfigIp AND EnableManagement.  If it did, try again.
  if ((fRouter != kAllZerosIp)
  &&  (N2M().NetToMediaIf(fRouter) == NULL))
  {
    N2M().LearnNetToMedia(fRouter);
    
    // Wait awhile so the learning process has time to complete...
    BcmOperatingSystemFactory::ThreadSleep(N2M_LEARN_MS);
  }
*/  
  LockMib();
  
  if (pReq)
    SnmpLogReq << *pReq << "  Agent=" << fName << endl;
  else
    SnmpLogReq << "NEXT request for OID " << OID << " Index " << Index << endl;

  pTable = FindTable (OID);

  // If we don't find the table, then assume this is a scalar
  // object and try that.
  if (pTable == NULL)
  {
#if SNMP_PROXY_SUPPORT
    // If we have a sub-agent, try looking for the object on him.
    if (fpSubAgent) {
      SNMP_STATUS SubAgentReturnStatus = fpSubAgent->NextObject(OID, Index, VB, pReq);
      if (SubAgentReturnStatus != SNMP_NO_SUCH_NAME) {
        UnlockMib ();
        return SubAgentReturnStatus;
      }
    }
#endif

    BcmMibObject *pScalar;
    BcmObjectId ScalarOid = OID;

    if (Index.Length() != 0)
    {
      // If we're receiving a NEXT request and we didn't find a table, that
      // means that either the thing being requested is either flat out not
      // supported, or else it's a scalar.  If the former is the case, we can
      // return NO_SUCH_NAME.  If the latter, then we know since it's a scalar
      // there can be only one instance, so we can still return NO_SUCH_NAME.
      // The non-portable agent will go to the next lexigraphical OID and
      // try again.
      
      SnmpLogReq << "  Table not found, index=" << Index <<". Can't be add'l scalar instance. NO_SUCH_NAME." << endl;
      UnlockMib ();
      
      return SNMP_NO_SUCH_NAME;
    }
    
    // Since the object being requested has no index, and we found no
    // matching table, it could be a scalar with instance .0.  Try that 
    // by making the Index = .0.  Note that we must set Index this way
    // rather than just doing ScalarOid += 0 because this way the
    // new index will filter back down to the non-portable agent in case
    // we find a match.
    
    Index += 0;
    ScalarOid += Index;

    SnmpLogReq << "  Table not found, no index. Try scalar w/ OID " << ScalarOid << endl;
    pScalar = FindObject (ScalarOid);

    if (pScalar == NULL)
    {
      // This is the case where we don't match the object at all.
      SnmpLogReq << "  No scalar match in the portable agent.  NO_SUCH_NAME." << endl;
      UnlockMib();

      return SNMP_NO_SUCH_NAME;
    }
    else if (pScalar->IsAccessible(pReq) == false)
    {
      // This is the case where a scalar object is known by the portable
      // agent but is being programatically excluded at runtime.
      // Example: NmAccess table to a mgmt station w/o write access.
      SnmpLogReq << "  Matched an inaccessible scalar, NO_SUCH_NAME" << endl;
      UnlockMib();

      return SNMP_NO_SUCH_NAME;
    }
    else
    {
      SNMP_STATUS Status = pScalar->Get(VB);

      // This is the case where we have found a scalar and handle it.
      SnmpLogReq << "  Matched scalar object: " << *pScalar << endl;      
      UnlockMib();
          
      if (Status != SNMP_NO_ERROR)
        SnmpLogReq << "Get failed - status = " << Status << endl;
        
      return Status;
    }
  }
  else
    SnmpLogReq << "  Matched table " << pTable->Name() << endl;

#if DYNAMIC_POPULATION  
  pTable->Populate ();  
#endif

  pEntry = pTable->NextEntry (Index);

  if (pEntry == NULL)
  {
    SnmpLogReq << "  Table " << pTable->Name() << " Entry not found: NO_SUCH_NAME" << endl;

#if DYNAMIC_POPULATION  
    pTable->Depopulate ();
#endif

    UnlockMib();

    return SNMP_NO_SUCH_NAME;
  }

  while (pEntry)
  {
    // Remember our request index, need it below.
    BcmObjectId ReqIndex = Index;

    pEntry->GetIndex(Index);
    SnmpLogReq << "  Table " << pTable->Name() << " Next index: " << Index << endl;
  
    if ((pEntry->IsAccessible(pReq) == true)
    &&  (pEntry->IsVisible() == true))
    {
      // Figure out what element of the requested OID is the 'entry'
      // like ifTable.1 = ifEntry
      int ReqOidEntryElement;

      // Figure out where the 'entry' part of the OID is.  Now, how this
      // works is, the OID object here (the requested OID) is always just
      // going to be the 'base' OID, up to and including the entry and column
      // part but without the instance.  This is true even if we do a getnext
      // tableName or a getnext entryName, as the agent will build up the first
      // expected column (we might need to skip some columns if the table
      // skips some due to not-accessible index columns or some just missing).
      // The instance is specified in the Index object, reassigned here to
      // ReqIndex.  So the 'entry' element is always the 2 from the length of the
      // OID object, regardless of the index length.
      ReqOidEntryElement = OID.Length() - 2;

      // Print some debug stuff...
      SnmpLogReq << "  REQ OID: " << OID << endl;
      SnmpLogReq << "  REQ INDEX: " << ReqIndex << endl;
      SnmpLogReq << "  ENTRY: " << WholeOID << endl;
      SnmpLogReq << "  ENTRY INDEX: " << Index << endl;
      SnmpLogReq << "  ENTRY ELEMENT = " << ReqOidEntryElement << endl;
    
      // Now, most tables look kind of like this:
      //
      // someTable
      //   someEntry = someTable.1
      //     someColumn = someEntry.n
      //
      // HOWEVER, once in a while some clever fellow will make a table where the entry
      // is NOT table.1, but rather table.2 or whatever.  An example of this
      // is the docsQosParamSetTable from DOCSIS.  So, when we get back the
      // entry, set the OID we were passed in to what the OID really is just
      // in case we have one of these situations.  That way the agent core
      // can rely on us to tip him off if some MIB author wants to be a 
      // schmuck.
      pEntry->GetOid (WholeOID);
      if (WholeOID[WholeOID.Length()-1] != OID[ReqOidEntryElement])
      {
        // Now actually, one of two things is happening here.  Either we have
        // a weird table, or somebody is trying to trick us by jacking the
        // entry number up to a larger than it should be number.  Usually the
        // body in question is an automated test suite like Silver Creek.  So
        // that would be the case where the requested OID has a BIGGER entry
        // OID element than the expected one.  Example: getnext ifTable.2
        // instead of ifTable.1
        if (WholeOID[WholeOID.Length()-1] < OID[ReqOidEntryElement])
        {
          SnmpLogReq << "  Hmmm, some sneaky test program is trying to trick us by mucking with the entry OID!  Thwart him.  NO SUCH NAME!" << endl;

          UnlockMib ();
          return SNMP_NO_SUCH_NAME;
        }

        // Now process the unconventional table case.

        SnmpLogReq << "  Hmmm, peculiar table.  Entry is at " << pTable->Name() << "." << WholeOID[WholeOID.Length()-1] << "..." << endl;
        
        // Change the entry part of the OID to match that of this entry.
        // Note that the OID being passed to us as the base OID is that of
        // a column, so we want to change the NEXT to the last element of the
        // OID to be equal to the LAST element of the entry.
        OID[OID.Length()-2] = WholeOID[WholeOID.Length()-1];
          
        // This might look like a no-op, but we need to do this so that 
        // BcmObjectId::UpdateOid will be called to update the internal OID
        // structure.
        OID = OID;
        
        SnmpLogReq << "  Changed base OID to " << OID << endl;
      }
      
      // First, check for the possibility that the particular object in this
      // entry is not-accessible.  That would be a case where the agent knows
      // this object but it's being not-supported at runtime.  An example of this
      // is docsIfDownChannelAnnex, which is required by Euro-docsis but would
      // cause a failure in US-docsis if it's supported.
      WholeOID = OID;
      WholeOID += Index;

      pObject = pEntry->FindObject (WholeOID);
      if (pObject == NULL)
      {
        SnmpLogReq << "Couldn't find " << OID << " in entry " << pTable->Name() << "." << Index << endl;
        
        UnlockMib ();
        
        // Check for the possibility that we have some goofy table with
        // non-sequential columns.  Sometimes this happens when MIBs get
        // revised and certain objects get deprecated.  A good example of this
        // is the docsQosServiceFlowTable for DOCSIS, which has columns
        // 1, 6, 7, and 8.  So if we get a 'next' for column 2 in that case,
        // we don't want to return NO_SUCH_NAME, but rather NO_ACCESS so the
        // agent will keep trying.
        if (pEntry->LastColumn() > OID[OID.Length()-1])
        {
          SnmpLogReq << "More columns in this table (" << pEntry->LastColumn() << "), NO_ACCESS." << endl;
          return SNMP_NO_ACCESS;
        }
        
        return SNMP_NO_SUCH_NAME;
      }
      else
      {
        SnmpLogReq << "Found object " << *pObject << endl;
        if (pObject->IsAccessible (pReq) == false)
        {
          // PR 5533
          // If this object is not accessible, then we can't just bail with
          // NO_SUCH_NAME, we need to try the next row.  This is because
          // it's possible that someObject.x might be inaccessible, but
          // someObject.x+1 might be accessible.  An example of this is in a
          // R/C table where certain objects are required for activation.
          // In that case, returning NO_SUCH_NAME for an uninitialized 
          // instance will terminate the walk on that column, and any 
          // instances below this one will be skipped.
          SnmpLogReq << "Object not accessible, try the next row." << endl;
          
          pEntry = pTable->NextEntry (Index);

          if (pEntry == NULL)
          {
            SnmpLogReq << "No next row, NO_ACCESS" << endl;

            UnlockMib();
            return SNMP_NO_ACCESS;
          }

          continue;
        }
        else 
        {
          SNMP_STATUS Status = pObject->Get(VB);
          
          if (Status != SNMP_NO_ERROR)
          {
            SnmpLogReq << "Get failed - status = " << Status << endl;
            
            UnlockMib();
            return Status;
          }
        }
      }

      UnlockMib();

      return SNMP_NO_ERROR;
    }
    if (pEntry->IsAccessible(pReq) == false)
      SnmpLogReq << "  Entry is not accessible!" << endl;
    if (pEntry->IsVisible() == false)
      SnmpLogReq << "  Entry is not visible!" << endl;

    SnmpLogReq << "  Entry " << Index << " is not accessible, or not visible.  Try the next one..." << endl;
    pEntry = pTable->NextEntry (Index);
  }

  UnlockMib();

  return SNMP_NO_SUCH_NAME;
}

SNMP_STATUS BcmSnmpAgent::TestObject (BcmObjectId &OID, BcmVarBind &VB, bool FromConfig, BcmMibRequest *pReq)
{
  BcmMibTable *pTable;
  BcmMibEntry *pEntry;
  BcmMibObject *pObject;
  BcmObjectId Index;
  SNMP_STATUS Status;

/*
  // Check for the unlikely but possible case of the router N2M mapping failing 
  // at both ConfigIp AND EnableManagement.  If it did, try again.
  if ((fRouter != kAllZerosIp)
  &&  (N2M().NetToMediaIf(fRouter) == NULL))
  {
    N2M().LearnNetToMedia(fRouter);
    
    // Wait awhile so the learning process has time to complete...
    BcmOperatingSystemFactory::ThreadSleep(N2M_LEARN_MS);
  }
*/  
  LockMib();

  if (pReq)
    SnmpLogReq << *pReq << " value to test is " << VB << "  Agent=" << fName << endl;
  else
    SnmpLogReq << "TEST request for OID " << OID << " value " << VB << endl;

  // Try to find the object of interest:
  pObject = FindObject (OID);
  if (pObject)
  {
    if (pObject->IsAccessible(pReq) == false)
    {
      SnmpLogReq << "Found object, but it's not accessible. NO_SUCH_NAME" << endl;
      UnlockMib();

      return SNMP_NO_SUCH_NAME;
    }

    SnmpLogSearch << "Found object, testing value." << endl;

    Status = pObject->Test (VB);

    if (Status == SNMP_NO_ERROR)
      SnmpLogReq << "  Object " << pObject->Name() << " tested OK." << endl;
    else
      SnmpLogReq << "  Object " << pObject->Name() << " failed test w/ status " << Status <<endl;

    UnlockMib();

    return Status;
  }

  // No object found?  If not, then try to find a table that has this OID
  // in it.
  SnmpLogSearch << "  Couldn't find object, checking for table." << endl;
  pTable = FindTable (OID);

  // If we don't find the table, then it's NO SUCH NAME
  if (pTable == NULL)
  {
    // If this is not coming from the config file, we're done.
    // Actually, we're done anyway.  Don't attempt to set objects via the
    // agent individually, they must be set in bulk via AgentSetObjects.
    SnmpLogSearch << "  Test of unknown object, not from the config file.  NO_SUCH_NAME" << endl;
    UnlockMib();

#if SNMP_PROXY_SUPPORT
    // If we have a sub-agent, try looking for the object on him.
    if (fpSubAgent)
      return fpSubAgent->TestObject(OID, VB, pReq);
#endif

    return SNMP_NO_SUCH_NAME;
  }

  // So a table exists, can we create and entry in it?
  SnmpLogSearch << "  Table " << pTable->Name() << " found, can I create an entry?" << endl;

  // Go ahead and create a default entry, then test the VB we got.
  // If the test fails, destroy the brand-new entry and return WRONG_VALUE.
  // If it succeeds, then return NO_ERROR and the set will occur via the
  // normal process.

  // Try to make a new entry.
  if (pTable->IsAccessible(pReq) == false)
  {
    SnmpLogReq << "  Table is not accessible.  No new entries allowed." << endl;
    pEntry = NULL;
  }
  else
    pEntry = pTable->CreateEntry (OID, pReq);

  // If we get a NULL back then that means this table is not a creatable one, or
  // for some reason the creation failed.
  if (pEntry == NULL)
  {
    SnmpLogReq << "  Table " << pTable->Name() << " found, but CreateEntry failed. NO_SUCH_NAME." << endl;
    UnlockMib();
    return SNMP_NO_SUCH_NAME;
  }
  else if (pEntry->IsAccessible (pReq) == false)
  {
    SnmpLogReq << "  Table " << pTable->Name() << " found, but new entry is not accessible.  NO_SUCH_NAME." << endl;
    UnlockMib ();
    
    delete pEntry;
    
    return SNMP_NO_SUCH_NAME;
  }

  SnmpLogSearch << "  Created entry " << *pEntry << endl;

  // If we get back an entry, then we darn sure oughta be able to find the
  // object now.
  pObject = pEntry->FindObject (OID);

  // If the object isn't in there, then that's really weird and something is
  // whack.  Destroy the entry and return NO_SUCH_NAME
  if (pObject == NULL)
  {
    SnmpLogErr << "  New table entry doesn't contain desired OID! Destroying." << endl;

    pTable->ExtractEntryIndex (OID, Index);
    pTable->DestroyEntry (Index);

    UnlockMib();

    return SNMP_NO_SUCH_NAME;
  }

  SnmpLogReq << "  Found object " << *pObject << endl;

  // Now, test the VB we got to see if the set is good.
  Status = pObject->Test (VB);
  if (Status != SNMP_NO_ERROR)
  {
    SnmpLogReq << "Test failed, destroying entry." << endl;

    // If not, then destroy that new entry and return WRONG_VALUE
    pTable->ExtractEntryIndex (OID, Index);
    pTable->DestroyEntry (Index);

    UnlockMib();

    return Status;
  }

  SnmpLogReq << "Test OK, NO ERROR." << endl;

  UnlockMib();

  // Test OK, NO ERROR
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmSnmpAgent::SetObject (BcmObjectId &OID, BcmVarBind &VB, bool FromConfig, BcmMibRequest *pReq)
{
  BcmMibObject *pObject;
  SNMP_STATUS Status = SNMP_NO_SUCH_NAME;

/*
  // Check for the unlikely but possible case of the router N2M mapping failing 
  // at both ConfigIp AND EnableManagement.  If it did, try again.
  if ((fRouter != kAllZerosIp)
  &&  (N2M().NetToMediaIf(fRouter) == NULL))
  {
    N2M().LearnNetToMedia(fRouter);
    
    // Wait awhile so the learning process has time to complete...
    BcmOperatingSystemFactory::ThreadSleep(N2M_LEARN_MS);
  }
*/  
  LockMib();

  if (pReq)
    SnmpLogReq << *pReq << " value to set is " << VB << "  Agent=" << fName << endl;
  else
    SnmpLogReq << "SET request for OID " << OID << " to " << VB << endl;

  pObject = FindObject (OID);

  if (pObject != NULL)
  {
    if (pObject->IsAccessible(pReq) == false)
    {
      SnmpLogReq << "  Found object, but it's not visible.  NO_SUCH_NAME" << endl;
      UnlockMib();

      return SNMP_NO_SUCH_NAME;  
    }

    Status = pObject->Set(VB);
    if (Status == SNMP_NO_ERROR)
      SnmpLogReq << "  Set OK" << endl;
    else if (Status & SNMP_ROW_NOT_CREATED)
    {
      Status &= ~SNMP_ROW_NOT_CREATED;
      
      SnmpLogReq << "  Set failed w/ status " << Status << ".  Row not created." << endl;
      
      BcmMibTable *pTable = FindTable (OID);
      if (pTable)
      {
        BcmObjectId Index;
        pObject->GetIndex (Index);
        pTable->DestroyEntry (Index);
      }
    }
    else
      SnmpLogReq << "  Set failed w/ status " << Status << endl;
  }
  else
  {
    SnmpLogReq << "  Set of unknown object.  SNMP_NO_SUCH_NAME" << endl;
    UnlockMib();

#if SNMP_PROXY_SUPPORT
    // If we have a sub-agent, try looking for the object on him.
    if (fpSubAgent)
      return fpSubAgent->SetObject(OID, VB, pReq);
#endif

    return SNMP_NO_SUCH_NAME;
  }

  UnlockMib();

  return Status;
}

#define MAX_TANKED_TRAPS      16

int BcmSnmpAgent::SendTrap (BcmTrap *pTrap)
{
  if (TrapsDeferred() == true)
  {
    SnmpLogEv << GetName() << " tanking up " << pTrap->Name() << " until after we're enabled..." << endl;
    return -1;
  }

  // Special case: check AuthFailTrapsEnabled for auth fail trap
  if (pTrap->Type() == AUTH_FAILURE_TRAP)
  {
    if (AgentGetAuthFailTrapsEnabled() == false)
    {
      SnmpLogEv << GetName() << " not sending " << pTrap->Name() << ", authFailTrapEnable=false" << endl;
      return 0;
    }
  }

  // Special case: check for ifLinkUpDownTrapEnable for linkUp and linkDown
  // traps.  Note that this is checked in EventLog::SendStandardTrap, but
  // in the case of a trap delivered from the HAL prior to cfg file processing,
  // where the cfg file turns traps off for that interface, we need to check
  // again here.
  if ((pTrap->Type() == LINK_UP_TRAP) || (pTrap->Type() == LINK_DOWN_TRAP))
  {
    // Get VB[0] from this trap, that's the ifIndex of the trap.
    const TrapObject *pObj = pTrap->GetObject (0);
    
    if (pObj)
    {
      int IfIndex;
      if (pObj->Value.GetInt32(&IfIndex) == SNMP_NO_ERROR)
      {
        // Find the ifXEntryBridge for the specified ifIndex.
        ifEntryBridge *pIfBridge = IfBridge().FindIfEntry (IfIndex);
        
        if (pIfBridge)
        {
          ifXEntryBridge *pXBridge = pIfBridge->IfXBridge();
          if (pXBridge)
          {
            if (pXBridge->Get_ifLinkUpDownTrapEnable() == CONSTVAL_IFLINKUPDOWNTRAPENABLE_DISABLED)
            {
              SnmpLogEv << GetName() << " not sending " << pTrap->Name() << ", ifLinkUpDownTrapEnable=false" << endl;
              return 0;
            }
          }
        }
      }
    }
  }
  
  // Even though we haven't really sent a trap, return 1 here so derived
  // classes can know that traps are a go.
  return 1;
}

int BcmSnmpAgent::SendStandardTrap (int TrapType, int IfIndex, bool Async)
{
  BcmTrap *pTrap = NULL;
  int NumTraps = 0, SentTraps = 0;
  ifXEntryBridge *pXBridge=NULL;


  if (IfIndex)
  {
    // Find the ifXEntryBridge for the specified ifIndex.
    ifEntryBridge *pIfBridge = IfBridge().FindIfEntry (IfIndex);
    
    if (pIfBridge)
      pXBridge = pIfBridge->IfXBridge();
  }
  
  switch (TrapType)
  {
    case TRAP_ID_COLD_START:
      pTrap = new coldStartTrap(this);
      break;
      
    case TRAP_ID_WARM_START:
      pTrap = new warmStartTrap(this);
      break;
      
    case TRAP_ID_LINK_DOWN:
      if ((pXBridge)
      &&  (pXBridge->Get_ifLinkUpDownTrapEnable() == CONSTVAL_IFLINKUPDOWNTRAPENABLE_ENABLED))
        pTrap = new linkDownTrap(this, IfIndex);
      break;
      
    case TRAP_ID_LINK_UP:
      if ((pXBridge)
      &&  (pXBridge->Get_ifLinkUpDownTrapEnable() == CONSTVAL_IFLINKUPDOWNTRAPENABLE_ENABLED))
        pTrap = new linkUpTrap(this, IfIndex);
      break;
      
    case TRAP_ID_AUTH_FAILURE:
      if (AgentGetAuthFailTrapsEnabled() == 1) // straight "C" API don't know from bool...
        pTrap = new authFailureTrap(this);
      break;
      
    default:
      SnmpLogEv << "No standard trap known of type " << TrapType << endl;
      return 0;
  }
  
  // This would mean that we got a trap request that we recognize, but
  // that kind of trap is administratively disabled: eg link up/down, or
  // auth fail enabled.
  if (pTrap == NULL)
    return 0;

  SentTraps = SendTrap (pTrap);
  
  if (SentTraps != -1)
    delete pTrap;
  else
  {
    // If SendTrap() returns -1, that means that the CM has not
    // completed its initialization (ie registration, DH calculations, etc).
    // Don't delete the trap in that case, it'll get sent later in 
    // SendDeferredAsyncs().  Add it to the list.
    
    if (fpDeferredTraps == NULL)
      fpDeferredTraps = pTrap;
    else
    {
      BcmTrap *pNextTrap = fpDeferredTraps;

      // Spin down to the end of the list...
      while (pNextTrap->fpNext)
      {
        NumTraps++;
        pNextTrap = pNextTrap->fpNext;
      }

      // ... and add this one to it.
      pNextTrap->fpNext = pTrap;
      
      // If our list is longer than MAX_TANKED_TRAPS, nix the first
      // one off the list.
      if (NumTraps > MAX_TANKED_TRAPS)
      {
        SnmpLogEv << "Tanked up traps has exceeded limit.  Say buh-bye to: " << *fpDeferredTraps << endl;
        pNextTrap = fpDeferredTraps;
        fpDeferredTraps = fpDeferredTraps->fpNext;
        delete pNextTrap;
      }
    }
  }
  
  return SentTraps;
}

bool BcmSnmpAgent::TrapsDeferred () const
{
  return fTrapsDeferred;
}

void BcmSnmpAgent::DeferTraps (bool Defer)
{
  fTrapsDeferred = Defer;
  
  if (Defer == true)
    SnmpLogRaw << GetName() << " defering traps." << endl;
  
  // If we are being un-deferred, go ahead and send any tanked traps.
  if (Defer == false)
    SendDeferredTraps();
}

int BcmSnmpAgent::SendDeferredTraps ()
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

#ifdef SNMP_PROXY_SUPPORT
void BcmSnmpAgent::SetSubAgent(BcmSnmpAgent *subAgent)
{
    fpSubAgent = subAgent;
}

BcmSnmpAgent* BcmSnmpAgent::GetSubAgent(void)
{
    return fpSubAgent;
}
#endif





