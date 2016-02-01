//****************************************************************************
//
// Copyright (c) 2007-2011 Broadcom Corporation
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
//    Filename: EstbSnmpAgent.cpp
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

#include "SnmpType.h"
#include "SnmpApi.h"
#include "SnmpLog.h"

#include "EstbSnmpAgent.h"

#include "BerObjs.h"

// Backwards compatibility for V2: If we have BFC_CM_VENDOR_SUPPORT
// (which is the old V2 flag), also define the new BFC flag.
#if (BCM_VENDOR_SUPPORT)
#define BFC_CM_VENDOR_SUPPORT 1
#endif

#if (BFC_CM_VENDOR_SUPPORT)
#include "VendorCmAppFactory.h"
#endif

#include "snmpV2Mib.h"
#include "ifBridge.h"
#include "lnxIpBridge.h"
#include "estbDocsDevBridge.h"
#include "docsDevEvBridge.h"
#include "docsDevNmBridge.h"
#include "docsDevTrapBridge.h"
#include "usmDhBridge.h"
#include "communityBridge.h"
#include "lnxHrBridge.h"

#include "NmAccess.h"
#include "EstbEventLog.h"
#include "EstbEvents.h"

#include "NonVolDevice.h"
#include "EstbSnmpNonVolSettings.h"
#include "MessageLogNonVolSettings.h"
#include "OperatingSystemFactory.h"

#include "EstbSnmpAgentStatusACT.h"
#include "EstbStatusEventCodes.h"
#include "standardTrap.h"
#include "docsDevTrap.h"

#if (MOCA_MIB_SUPPORT)
#include "mocaBridge.h"
#endif

// #include "PktFilterController.h"

#if (BCM_HEAPMANAGER_SUPPORT)
#define  TLV11_HEAP_THRESHOLD 8192 // 8k
#include "BcmHeapManager.h"
#endif

// #include "IpHelperFactory.h"
// #include "Tftp.h"
#include "EstbConfig.h"
#include "EstbTLVcodes.h"
extern "C" {
#include "LnxInterface.h"
}

#include <sys/dir.h>

#if (BCM_EVD_INTERFACE_INCLUDED)
#if !(BCM_EVD_RPCCL_INTERFACE_INCLUDED)
#include "EcmEventDispatcherThread.h"
#endif 

// Event list to be filled in by the client.
#include "EstbSnmpAgentDefaultEventList.h"
#endif

#include "SnmpTrapExploderThread.h"
#include "BcmSnmpTrapHandler.h"

//********************** Local Functions *****************************************

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Extern Functions ************************************


// Static member initialization:
EstbSnmpAgent *EstbSnmpAgent::fpSingletonInstance = NULL;

//********************** Class Method Implementations ************************

// --------------------------------------------------------------------
// Constructor.  Initializes and starts the thread.
//
// Parameters:
//      initialPriority - the thread priority that should be used.
//
// Returns:  N/A
// --------------------------------------------------------------------
EstbSnmpAgent::EstbSnmpAgent () :
  DefaultSnmpAgentClass ((char *) ESTB_AGENT_NAME),
  fTlv11Engine (this, "TLV-11")
{
  LockMib();
  
  SnmpLogStart << "Creating eSTB SNMP agent." << endl;
  
  // Tables we'll need to flush at restart:
  // fpLlcFilterTable = NULL; 
  // fpIpFilterTable = NULL;
  // fpFilterPolicyTable = NULL;
  // fpTosTable = NULL;
  fpCpeFilterTable = NULL;
  fpNmAccessTable = NULL;
  fpTargetExtTable = NULL;
  fpDHKickstartTable = NULL;
  fpDHUserKeyTable = NULL;

  fpDocsisCtlThread = NULL;
  fpCmDocsisStatusACT = NULL;
  
  fpOsMonThread = NULL;		// These guys gets started whe we are told to start
#ifdef MOCA_MIB_SUPPORT
  fpMocaMonThread = NULL;	// This guys gets started whe we are told to start
#endif
  
#if (SNMP_SUPPORT == 3)
  SetTLV38Base (ESTB_AGENT_TLV38_BASE);
  SetTLV53Base (ESTB_AGENT_TLV53_BASE);
  fpDhThread = new BcmDhKickstartThread (this, "DHCM");
#endif
  
  // Initialize our SNMP version mode to NMACCESS until/unless we see a
  // DH kickstart pair in the config file.
  fSnmpMode = SNMP_MODE_NMACCESS;
  
  // Initialize system objects from non-vol
  BcmSnmpNonVolSettings *pSettings = BcmSnmpNonVolSettings::GetSingletonInstance();
  if (pSettings)
  {
    fSysDescr = pSettings->SysDescr();
    fSysObjectID = pSettings->SysObjectId();
    fSysContact = pSettings->SysContact();
    fSysName = pSettings->SysName();
    fSysLocation = pSettings->SysLocation();
    fSysServices = pSettings->SysServices();
    fSysORID = pSettings->SysORID();
    fSysORDescr = pSettings->SysORDescr();
    fInterfaceName = pSettings->InterfaceName();
    if (fInterfaceName.length() <= 0)  {
      fInterfaceName = ESTB_SNMP_DEFAULT_INTERFACE;
      SnmpLogRaw << "Using default interface " << fInterfaceName << endl;
    } else {
      SnmpLogRaw << "Using interface " << fInterfaceName << " from NON-VOL" << endl;
    }
  }
  
  UnlockMib();

	#if (BCM_EVD_INTERFACE_INCLUDED)
		#if (BCM_EVD_RPCCL_INTERFACE_INCLUDED)
		initializeEstbProvMgrEvdClientRpcInterfaceAndSubscribeToEvents();
		#else
		SubscribeToEvents();
		#endif
	#endif
}

EstbSnmpAgent::~EstbSnmpAgent ()
{
#if (SNMP_SUPPORT == 3)
  if (fpDhThread)
    delete fpDhThread;
    
  // if (fpFilterSnoop)
  //   delete fpFilterSnoop;
  
  // if (fpStpSnoop)
  //   delete fpStpSnoop;
#endif

 if (fpOsMonThread)
   delete fpOsMonThread;

#ifdef MOCA_MIB_SUPPORT
 if (fpMocaMonThread)
   delete fpMocaMonThread;
#endif
}

SNMP_STATUS EstbSnmpAgent::WriteNonVolSettings (BcmNonVolSettings::NonVolSection Section, bool Async)
{
  BcmNonVolDevice *pNonVolDevice = BcmNonVolDevice::GetSingletonInstance();;
  BcmSnmpNonVolSettings *pSettings = BcmSnmpNonVolSettings::GetSingletonInstance();
  SNMP_STATUS Status;
  bool result;


  if (pSettings == NULL)
  {
    gLogMessageRaw << "No BcmSnmpNonVolSettings object is present; can't write settings to the device!" << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }

  if (pNonVolDevice == NULL)
  {
    gLogMessageRaw << "No Non-Vol Device is present; can't write settings to the device!" << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }

  if (Async)
    result = pNonVolDevice->Write(*pSettings, Section, BcmNonVolDevice::kWriteModeAsync);
  else
    result = pNonVolDevice->Write(*pSettings, Section, BcmNonVolDevice::kWriteModeSync);

  if (result == false)
  {
    gLogMessageRaw << "ERROR - failed to write Non-Vol Settings to the device!" << endl;
    Status = SNMP_GEN_ERR;
  }
  else
  {
    gLogMessageRaw << "Non-Vol Settings successfully written to the device." << endl;
    Status = SNMP_NO_ERROR;
  }

  return Status;
}

SNMP_STATUS EstbSnmpAgent::sysContact (const BcmString &SysContact)
{
  SNMP_STATUS Status = DefaultSnmpAgentClass::sysContact (SysContact);
  
  if (Status != SNMP_NO_ERROR)
    return Status;
    
  BcmSnmpNonVolSettings *pSettings = BcmSnmpNonVolSettings::GetSingletonInstance();
  if (pSettings == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;
    
  {
#ifdef NOTDEF
      // PR6478 - acquire the nonvol access lock before updating any values.
      // It will be released automatically when we exit this scope.
      BcmNonVolSettingsAccessLock nonvolLock;
#endif

      pSettings->SysContact(SysContact.c_str());
  }
  return WriteNonVolSettings();
}

SNMP_STATUS EstbSnmpAgent::sysName (const BcmString &SysName)
{
  SNMP_STATUS Status = DefaultSnmpAgentClass::sysName (SysName);
  
  if (Status != SNMP_NO_ERROR)
    return Status;
    
  BcmSnmpNonVolSettings *pSettings = BcmSnmpNonVolSettings::GetSingletonInstance();
  if (pSettings == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;
    
  {
#ifdef NOTDEF
      // PR6478 - acquire the nonvol access lock before updating any values.
      // It will be released automatically when we exit this scope.
      BcmNonVolSettingsAccessLock nonvolLock;
#endif

      pSettings->SysName(SysName.c_str());
  }
  return WriteNonVolSettings();
}

SNMP_STATUS EstbSnmpAgent::sysLocation (const BcmString &SysLocation)
{
  SNMP_STATUS Status = DefaultSnmpAgentClass::sysLocation (SysLocation);
  
  if (Status != SNMP_NO_ERROR)
    return Status;
    
  BcmSnmpNonVolSettings *pSettings = BcmSnmpNonVolSettings::GetSingletonInstance();
  if (pSettings == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;
    
  {
#ifdef NOTDEF
      // PR6478 - acquire the nonvol access lock before updating any values.
      // It will be released automatically when we exit this scope.
      BcmNonVolSettingsAccessLock nonvolLock;
#endif

      pSettings->SysLocation(SysLocation.c_str());
  }
  return WriteNonVolSettings();
}

int EstbSnmpAgent::AssignIfIndex (ifEntryBridge *pIfEntry) const
{
  int ifIndex = 1;

  // There is only one interface, right?

  return ifIndex;
}

unsigned long EstbSnmpAgent::GetEngineBoots () const
{
  BcmSnmpNonVolSettings *pSnmpSettings = BcmSnmpNonVolSettings::GetSingletonInstance();
  
  
  if (pSnmpSettings)
    return pSnmpSettings->SnmpEngineBoots();
    
  return 1;
}

EstbSnmpAgent &EstbSnmpAgent::Singleton()
{
  if (fpSingletonInstance)
    return *fpSingletonInstance;
    
  fpSingletonInstance = new EstbSnmpAgent();
  
  return *fpSingletonInstance;
}

ostream &EstbSnmpAgent::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)
    DefaultSnmpAgentClass::Print (outStream);
    
    switch (fSnmpMode)
    {
      case SNMP_MODE_NMACCESS:
        outStream << "  SNMP mode: NMACCESS" << endl;
        break;
        
      case SNMP_MODE_COEXISTENCE:
        outStream << "  SNMP mode: COEXISTENCE" << endl;
        break;
        
      case SNMP_MODE_V3:
        outStream << "  SNMP mode: V3" << endl;
        break;
        
      default:
        outStream << "  SNMP mode: " << fSnmpMode << endl;
        break;
    }
    
    if (fTlv10Engine.NumWacs() > 0)
      outStream << "  " << fTlv10Engine << endl;

    outStream << "  Interface: " << fInterfaceName << endl;
    
#endif
    
	return outStream;
}

SNMP_MODE EstbSnmpAgent::GetDocsisSnmpMode () const
{
  return fSnmpMode;
}

bool EstbSnmpAgent::PopulateMibs ()
{
  BcmMibGroup *pGroup;
 
  // Let the base class populate with any globally supported objects...
  if (DefaultSnmpAgentClass::PopulateMibs() == false)
    return false;
  
  LockMib();
  
  // Install all eSTB required MIB groups over and beyond what the base
  // class installs.
  
  pGroup = new snmpSetGroup ();
  AddGroup (pGroup);

#if (SNMP_SUPPORT == 3)    
  pGroup = new snmpCommunityMIBObjectsGroup ();
  AddGroup (pGroup);
#endif

  pGroup = new docsDevMIBObjectsGroup ();
  AddGroup (pGroup);
  
#if (SNMP_SUPPORT == 3) 
  // Diffie-Helman kickstart
  pGroup = new usmDHKickstartGroup ();
  AddGroup (pGroup);

  pGroup = new usmDHPublicObjectsGroup ();
  AddGroup (pGroup);

#endif

  pGroup = new hostGroup ();
  AddGroup (pGroup);

#if (MOCA_MIB_SUPPORT)
  pGroup = new moca11Group ();
  AddGroup (pGroup);
#endif

  // Find some tables that we need local copies of:
  fpNmAccessTable = (docsDevNmAccessTable *)FindTable (BcmString ("docsDevNmAccessTable"));
  fpTargetExtTable  = FindTable (BcmString ("snmpTargetAddrExtTable"));
  fpDHKickstartTable  = FindTable (BcmString ("usmDHKickstartTable"));
  fpDHUserKeyTable = FindTable (BcmString ("usmDHUserKeyTable"));
  fpIpNetToMediaTable = FindTable (BcmString ("ipNetToMediaTable"));
  fpIpNetToPhysicalTable = FindTable (BcmString ("ipNetToPhysicalTable"));
  fpHrSWRunTable = FindTable (BcmString ("hrSWRunTable"));
  fpHrSWRunPerfTable = FindTable (BcmString ("hrSWRunPerfTable"));

  if (fpNmAccessTable == NULL)
    SnmpLogRaw << "WARNING - Failed to find docsDevNmAccessTable" << endl;
  if (fpTargetExtTable == NULL)
    SnmpLogRaw << "WARNING - Failed to find snmpTargetAddrExtTable" << endl;
  if (fpDHKickstartTable == NULL)
    SnmpLogStart << "WARNING - Failed to find usmDHKickstartTable" << endl;
  if (fpDHUserKeyTable == NULL)
    SnmpLogStart << "WARNING - Failed to find usmDHUserKeyTable" << endl;
  if (fpIpNetToMediaTable == NULL)
    SnmpLogStart << "WARNING - Failed to find ipNetToMediaTable" << endl;
  if (fpIpNetToPhysicalTable == NULL)
    SnmpLogStart << "WARNING - Failed to find ipNetToPhysicalTable" << endl;
  if (fpHrSWRunTable == NULL)
    SnmpLogStart << "WARNING - Failed to find hrSWRunTable" << endl;
  if (fpHrSWRunPerfTable == NULL)
    SnmpLogStart << "WARNING - Failed to find hrSWRunPerfTable" << endl;
    
  UnlockMib();
  
  return true;
}

bool EstbSnmpAgent::PopulateBridges ()  
{
if (DefaultSnmpAgentClass::PopulateBridges() == false)
    return false;

  BcmMibBridge *pBridge;
  int Loop;

  // docsDevSoftware group:
  pBridge = new docsDevSoftwareGroupBridge(this);
  if (pBridge->Register(kDeleteBridge) != SNMP_NO_ERROR)
    delete pBridge;

  
  // docsDevEvent group:
  pBridge = new docsDevEventGroupBridge (this, &EstbEventLog::Singleton());
  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
    delete (pBridge);
  
  for (Loop = 1; Loop < 9; Loop++)
  {
    pBridge = new docsDevEvControlEntryBridge (Loop, this, &EstbEventLog::Singleton());
    if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
      delete (pBridge);
  }

  pBridge = new docsDevFilterGroupBridge (this);
  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
    delete (pBridge);
   
  // Also create a bogus bridge for the snmpSetGroup to get snmpSetSerialNo
  pBridge = new ScalarMibBridge ("1.3.6.1.6.3.1.1.6", this);
  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
    delete (pBridge);


#if (SNMP_SUPPORT == 3)    
  // add support for the dh params object
  pBridge = new usmDHPublicObjectsGroupBridge (this);
  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
    delete pBridge;
#endif

  // hrStorage group
  pBridge = new lnxHrStorageGroupBridge (this);
  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
    delete (pBridge);

  // hrStorageTableEntry for physical memory 
  pBridge = new lnxHrStorageEntryBridge (HR_STORAGE_INDEX_RAM, this);
  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
    delete (pBridge);

  // hrStorageTableEntry for virtual memory 
  pBridge = new lnxHrStorageEntryBridge (HR_STORAGE_INDEX_VIRTUAL, this);
  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
    delete (pBridge);

  // hrProcessorTableEntry  and its entry in the hrDeviceTable
  pBridge = new lnxHrProcessorEntryBridge (1, this);
  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
    delete (pBridge);
  pBridge = new lnxHrDeviceEntryBridge (1, BcmObjectId("3"), 
    BcmString("Main Processor"), BcmObjectId(kOID_bcm97401), this);
  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
    delete (pBridge);

#if (MOCA_MIB_SUPPORT)
#ifdef SAMPLE_CODE_WITH_DUMMY_VALUES
  pBridge = new mocaIfConfigEntryBridge (1, this);
  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
    delete (pBridge);
  
  pBridge = new mocaIfAccessEntryBridge (1, this);
  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
    delete (pBridge);
  
  pBridge = new mocaIfStatusEntryBridge (1, this);
  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
    delete (pBridge);
  
  pBridge = new mocaIfStatsEntryBridge (1, this);
  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
    delete (pBridge);
  
  pBridge = new mocaIfFlowStatsEntryBridge (1, 1, this);
  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
    delete (pBridge);
  
  pBridge = new mocaNodeEntryBridge (1, 1, this);
  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
    delete (pBridge);
  
  pBridge = new mocaMeshEntryBridge (1, 1, 1, this);
  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
    delete (pBridge);
  
  pBridge = new mocaBridgeEntryBridge (1, 1, 1, this);
  if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
    delete (pBridge);

#endif
#endif
  
  return true;
}

void EstbSnmpAgent::SetMasterAgent (DefaultSnmpAgentClass *pAgent)
{
  if (pAgent)
    SnmpLogRaw << GetName() << " setting master agent to " << pAgent->GetName() << endl;
    
  fpMasterAgent = pAgent;
}

// Note that this is not the same as the virtual Setup method from the
// base class because it takes an additional argument (DocsisCtlThread), but it 
// does call the base class method and should be called in lieu of it.
SNMP_STATUS EstbSnmpAgent::Setup (const BcmMacAddress &Mac, unsigned int IfNum, BcmPacketSwitch *pSwitch, BcmCmDocsisCtlThread *pDocsisCtlThread)//, BcmEstbApplication *pEstbApplication)
{
    
  fpDocsisCtlThread = pDocsisCtlThread;
  //fpEstbApplication = pEstbApplication;
  SnmpLogRaw << "Instantiating the main trap handler " << endl;
  char *pTHName = (char *) "Main Trap Handler";
  BcmSnmpTrapHandler *pTH = new BcmSnmpTrapHandler(pTHName, this);
  // SnmpLogRaw << "Setup trap handler " << endl;
  pTH->Setup();
  // SnmpLogRaw << "Setting loopback IP in trap handler " << endl;
  pTH->ConfigIp(kSoftwareLoopbackIp);
  // SnmpLogRaw << "Setting port in trap handler " << endl;
  pTH->SetUdpTrapPort(16977);
  SnmpLogRaw << "Registering trap handler " << endl;
  pTH->Register();
  
  return DefaultSnmpAgentClass::Setup (Mac, IfNum, pSwitch);
}

SNMP_STATUS EstbSnmpAgent::SetUdpPort (int UdpPort)
{
#ifdef NOTDEF  
  SNMP_STATUS Status = DefaultSnmpAgentClass::SetUdpPort (UdpPort);
  // Set our CPE agent's port, too (assuming the CPE agent exists)
  if ((Status == SNMP_NO_ERROR) && (CpeSnmpAgent::Exists()))
    CpeSnmpAgent::Singleton().SetUdpPort(UdpPort);
    
  return Status;
#endif
  return SNMP_NO_ERROR;
}

SNMP_STATUS EstbSnmpAgent::SetUdpTrapPort (int UdpTrapPort)
{ 
#ifdef NOTDEF  
  SNMP_STATUS Status = DefaultSnmpAgentClass::SetUdpTrapPort (UdpTrapPort);

  // Set our CPE agent's port, too (assuming the CPE agent exists)
  if ((Status == SNMP_NO_ERROR) && (CpeSnmpAgent::Exists()))
    CpeSnmpAgent::Singleton().SetUdpTrapPort (UdpTrapPort);
    
  return Status;
#endif
  return SNMP_NO_ERROR;
}

void EstbSnmpAgent::SetupSnoops ()
{
}

SNMP_STATUS EstbSnmpAgent::Start ()
{
  SNMP_STATUS Status;

  
  SnmpLogStart << "Starting eSTB SNMP Agent." << endl;
  
  // Increment the snmpEngineBoots non-vol setting
  BcmSnmpNonVolSettings *pSnmpSettings = BcmSnmpNonVolSettings::GetSingletonInstance();
  if (pSnmpSettings)
  {
      {
#ifdef NOTDEF
          // PR6478 - acquire the nonvol access lock before updating any values.
          // It will be released automatically when we exit this scope.
          BcmNonVolSettingsAccessLock nonvolLock;
#endif

          pSnmpSettings->SnmpEngineBoots(pSnmpSettings->SnmpEngineBoots() + 1);
      }

      BcmNonVolDevice *pNonVolDevice = BcmNonVolDevice::GetSingletonInstance();
      if (pNonVolDevice)
          pNonVolDevice->Write(*pSnmpSettings);
  }
  
  // EstbLogEvent (kSnmpAgentStartup, "Starting eSTB SNMP agent.");
  
  Status = DefaultSnmpAgentClass::Start ();
  
  // If SNMP non-vol 'allow config' is enabled, enable factory mode.
  if (pSnmpSettings)
    if (pSnmpSettings->AllowConfiguration() == true) {
      EnableFactoryMode(true);
  
  }

#if (!BCM_EVD_INTERFACE_INCLUDED)
  // The OS Mon thread has to be started because he is the one who 
  // actually tells the SNMP Agent when the config file is here and
  // when the interface is is up.
  fpOsMonThread = new BcmEstbOsMonitorThread(this, "OSMN");
#endif

#ifdef MOCA_MIB_SUPPORT
  // The MoCA Monitor thread  monitors the MoCA driver and updates
  // our MoCA MIB tables
  fpMocaMonThread = new BcmMocaMonitorThread(this, "MOCA");
#endif

  return Status;
}  

SNMP_STATUS EstbSnmpAgent::EnableFactoryMode (bool Enable)
{
  SNMP_STATUS Status = DefaultSnmpAgentClass::EnableFactoryMode (Enable);
  if (Status != SNMP_NO_ERROR)
    return Status;
    
  BcmSnmpNonVolSettings *pSnmpSettings = BcmSnmpNonVolSettings::GetSingletonInstance();
  if (pSnmpSettings)
    pSnmpSettings->AllowConfiguration (Enable);
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS EstbSnmpAgent::BerSetWriteAccess (unsigned char *pBerBuf, int bufferLen)
{
  // Add this object to the TLV10 engine.
  fTlv10Engine.AddWac (pBerBuf, bufferLen);
  
  return SNMP_NO_ERROR;
}  


SNMP_STATUS EstbSnmpAgent::BerSetObject (unsigned char *pBerBuf, int BufferLen)
{ 
  // Add this object to the TLV11 engine.
  return fTlv11Engine.AddVarBind (pBerBuf, BufferLen);
}


SNMP_STATUS EstbSnmpAgent::ProcessSetList ()
{
  // Remember all objects stored in non-vol in case we wind up 
  // rejecting the config file.  These include sysName, sysContact, sysLocation,
  // docsDevSwAdminStatus, docsDevSwServer, and docsDevSwFilename.  This allows
  // us to back out changes to these objects in case they get set prior to a
  // bad value.  Everything else gets reset in ::ResetMibDefaults.
  BcmString sysName, sysContact, sysLocation, docsDevSwFilename;
  BcmIpAddress docsDevSwServer;
  int docsDevSwAdminStatus = 0;
  
  BcmSnmpNonVolSettings *pSettings = BcmSnmpNonVolSettings::GetSingletonInstance();
  if (pSettings)
  {
    sysName = pSettings->SysName();
    sysContact = pSettings->SysContact();
    sysLocation = pSettings->SysLocation();
    // docsDevSwFilename = pSettings->DocsDevSwFilename();
    // docsDevSwServer = pSettings->DocsDevSwServer();
    // docsDevSwAdminStatus = pSettings->DocsDevSwAdminStatus();
  }

  // Call the TLV-11 engine and let him process all the TLV's we have given him.
  SNMP_STATUS Status = fTlv11Engine.Process();

  // Host CFR wants us to log duplicates and unknowns, but keep going

  // If we've encountered any unknown OID sets, we need to log
  // I411.1 events, one for each unknow OID.  Lucky for us, we don't append
  // the OID or value to this event, so we can just loop'n'log.
  for (int Loop = 0; Loop < TLV11UnknownOIDs(); Loop++)
    EstbLogEvent (kEE_T41101);

  // same with duplicates, since we can recover from these.
  for (Loop = 0; Loop < TLV11Duplicates(); Loop++)
      EstbLogEvent (kEE_T41102);
        
  // OSSI spec section 2.2.b sez:
  // "CM is in SNMP coexistence mode if the CM configuration file 
  // contains snmpCommunityTable setting"
  // So if we now have a row in the snmpCommunityTable, then we're in
  // coexistence mode.
  if ((fpCommunityTable != NULL) && (fpCommunityTable->NumEntries() != 0))
    fSnmpMode = SNMP_MODE_COEXISTENCE;

#if 1
  if (Status != SNMP_NO_ERROR)
  {
    // Log the appropriate events to indicate wrong values...
    if (TLV11WrongValues())
      EstbLogEvent (kEE_T41103);
    
    SnmpLogRaw << "Invalid TLV-11's: " << TLV11WrongValues() << " wrong values.\n"
                    "                    But since we're in an eSTB we won't reject the cfg file." << endl;
      
      Status = SNMP_NO_ERROR;
  }
    
#else

  if (Status == SNMP_NO_ERROR)
  {
    // OSSI spec section 2.2.b sez:
    // "CM is in SNMP coexistence mode if the CM configuration file 
    // contains snmpCommunityTable setting"
    // So if we now have a row in the snmpCommunityTable, then we're in
    // coexistence mode.
    if ((fpCommunityTable != NULL) && (fpCommunityTable->NumEntries() != 0))
      fSnmpMode = SNMP_MODE_COEXISTENCE;
  }
  else
  {
    // Log the appropriate events to indicate wrong values...
    // if (TLV11WrongValues())
    //   EstbLogEvent (kEE_0100);
    
    // ... or duplicates, as appropriate.
    if (TLV11Duplicates())
      EstbLogEvent (kEE_T41102);
        
    // If we are in factory mode, we will not reject the config file.
    if (FactoryMode() == true)
    {
      SnmpLogRaw << "Invalid TLV-11's: " << TLV11WrongValues() << " wrong values, " << TLV11Duplicates() << " duplicate sets.\n"
                    "                    But since we're in FACTORY mode we won't reject the cfg file." << endl;
      
      Status = SNMP_NO_ERROR;
    }
    
    // Otherwise, reject the config file and reset our MIB state.
    else
    {
      SnmpLogRaw << "Invalid TLV-11's: " << TLV11WrongValues() << " wrong values, " << TLV11Duplicates() << " duplicate sets.\n"
                    "                    Rejecting cfg file & restoring default MIB state." << endl;
      
      // Reset all objects to their default states.  Need to do this here even
      // though we will do it again once we fail reg & rescan because we might
      // theoretically get the 'rescue config file' prior to rescanning.
      ResetMibDefaults();
    
      // If any of our NV parameters were changed, reset them to what they were
      // before.  
      if (pSettings)
      {
        if ((sysName != pSettings->SysName())
        ||  (sysContact != pSettings->SysContact())
        ||  (sysLocation != pSettings->SysLocation()))
        {
          BcmNonVolDevice *pNonVolDevice = BcmNonVolDevice::GetSingletonInstance();
          
          if (pNonVolDevice)
          {
            {
#ifdef NOTDEF
                // PR6478 - acquire the nonvol access lock before updating any values.
                // It will be released automatically when we exit this scope.
                BcmNonVolSettingsAccessLock nonvolLock;
#endif

                pSettings->SysName(sysName.c_str());
                pSettings->SysContact(sysContact.c_str());
                pSettings->SysLocation(sysLocation.c_str());
            }
            
            if (pNonVolDevice->Write(*pSettings) == false)
              gLogMessageRaw << "ERROR - failed to write Non-Vol Settings to the device!" << endl;
            else
              gLogMessageRaw << "Non-Vol Settings successfully written to the device." << endl;
          }
        }
      }
    }
  }
#endif
  
  return Status;
}

SNMP_STATUS EstbSnmpAgent::ProvisionDHKickstartEntry (BcmOctetBuffer &SecurityName, BcmOctetBuffer &PublicNumber)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;  
  BcmString SecName = SecurityName;

#if (SNMP_SUPPORT == 3)
  
  LockMib();

  // Check for valid users first:
  // NOTE: not clear what we should do if the security name is NULL terminated.
  // Take it or not?  We will err on the side of tolerance and accept it.  
  // 86 any NULL terminators on the security name:
  //
  // According to CZ, CL sez MUST NOT accept if NULL terminated.
  // SecName = SecName.c_str(); 
  
  if (SecName == "docsisManager")
  {
    SnmpLogRaw<<"TLV-34 docsisManager - Installing User."<<endl;
    AgentInstallUser  ((char *) "docsisManager", SNMP_SEC_PRIV);
  }
  else if(SecName == "docsisOperator")
  {
    SnmpLogRaw<<"TLV-34 docsisOperator - Installing User."<<endl;
    AgentInstallUser  ((char *) "docsisOperator", SNMP_SEC_PRIV);
  }
  else if(SecName == "docsisMonitor")
  {
    SnmpLogRaw<<"TLV-34 docsisMonitor - Installing User."<<endl;
    AgentInstallUser  ((char *) "docsisMonitor", SNMP_SEC_AUTH);
  }
  else if(SecName == "docsisUser")
  {
    SnmpLogRaw<<"TLV-34 docsisUser - Installing User."<<endl;
    AgentInstallUser  ((char *) "docsisUser", SNMP_SEC_AUTH);
  }
  else
  {
    //PR8478: Don't return here - allow other TLV34 users.

    // SnmpLogRaw<<"TLV-34 NOT Installing unknown user " << SecName <<endl;
    // UnlockMib();
    // return SNMP_BAD_VALUE;

    SnmpLogRaw<<"TLV-34 Installing Non-ESTB Specified User - "<<SecName<<endl;
    AgentInstallUser  (&SecName[0], SNMP_SEC_PRIV);

  }
  
  // If this is the first DH kickstart value we've gotten, we
  // need to go create our V3 users now.
  if (fSnmpMode == SNMP_MODE_NMACCESS)
    InstallUsers ();
    
  Status = DefaultSnmpAgentClass::ProvisionDHKickstartEntry (SecurityName, PublicNumber);
  
  // We must go into coexistence mode even if the user lookup fails.  From the
  // 2.0 OSS spec, section 5.2, bullet (d):
  //
  // "d) In case of failure to complete SNMPv3 initialization (i.e., NMS cannot 
  // access CM via SNMPv3 PDU), the CM is in co-existence mode and will allow 
  // SNMPv1/v2c access if and only if the community MIB entries (and related 
  // entries) are configured." 
  //
  // This is kind of a weird requirement, because it means that we will be
  // totally unmanageable!  But that's what the rules sez...
  // if (Status == SNMP_NO_ERROR)
  fSnmpMode = SNMP_MODE_COEXISTENCE;
  
  UnlockMib();
  
#endif
  
  return Status;
}
   
SNMP_STATUS EstbSnmpAgent::InformProvisioningComplete ()
{
  // If any TLV-11's fail, we gotta reject the config file.
  if (ProcessSetList() != SNMP_NO_ERROR)
    return SNMP_GEN_ERR;
    
  AgentUpdateEngineTimes();

  return SNMP_NO_ERROR;
}
  
void EstbSnmpAgent::InformRegistrationComplete ()
{

  // If no provisioned views have been installed, then install a V1 view
  // to allow full access subject to normal community and
  // docsDevNmAccess rules.
  if (fSnmpMode == SNMP_MODE_NMACCESS)
  {
  	SnmpLogRaw << "+++ No DH kickstart profiles or snmpCommunityTable entries installed.\n"
                  "    We will operate in NMACCESS mode." << endl;
    SetV1V2View ("docsisNmAccessView");
    EnableManagement(true);
  }
  else if (fSnmpMode == SNMP_MODE_COEXISTENCE)
  {
	SnmpLogRaw << "+++ DH kickstart profiles and/or snmpCommunityTable entries installed.\n"
                  "    We will operate in COEXISTENCE mode." << endl;
     
    // Use unified coexistence view for any agent instead of unique ones
    // for each agent type.  Avoid the need to check multiple hard-coded
    // views in agent core for backdoor.
    SetV1V2View (RFC2576_COEXISTENCE_VIEW);

    //EnableUser (BcmString("dhKickstart"));

#if (SNMP_SUPPORT == 3)
    // Doing our DH calculations can take a few seconds, so we need to do them
    // asynchronously in a separate thread so we don't clobber ranging, etc.        
    fpDhThread->KickstartFinish();
#else
    EnableManagement(true);
#endif
  }
  else
  {
    // This should never happen...
    SnmpLogRaw << "+++ How did we wind up in SNMPv3 only mode???" << endl;
    
    // Use unified coexistence view for any agent instead of unique ones
    // for each agent type.  Avoid the need to check multiple hard-coded
    // views in agent core for backdoor.
    SetV1V2View (RFC2576_COEXISTENCE_VIEW);

#if (SNMP_SUPPORT == 3)    
    fpDhThread->KickstartFinish();
#else
    EnableManagement(true);
#endif
  }

}

void EstbSnmpAgent::InformRestartPending ()
{
  SnmpLogInit << "EstbSnmpAgent::InformRestartPending" << endl;

#if (SNMP_SUPPORT == 3)  
  // Abort our DH kickstart calculations if they are in progress.
  // Note that this call will block until the thread is finished.
  fpDhThread->Stop();
#endif

  // return to default snmp mode, NmAccess.
  fSnmpMode = SNMP_MODE_NMACCESS;
  
  /*\ 
   *  Perform CM agent cleanup
  \*/
  
  // Reset various tables and scalars to default states
  ResetMibDefaults ();
  
  // If our management is off right now, that means that we are now scanning
  // to a new channel without ever having registered.  For example, say we
  // got a bad config file or for some other reason failed registration.
  // In that case, let's go ahead and send any syslog or trap messages that we
  // may have tanked up in the meantime.
  if (ManagementEnabled() == false)
  {
    // Flush any deferred traps or syslog messages in order to prevent a nasty
    // case of async-message-buildup in the case where we scan and re-register
    // lots of times, eg DSX testing.  Note that calling DeferTraps(false) will
    // send the traps, but we still need to send the asyncs.  The false arg
    // to SendDeferredAsyncs means do it synchronously, so by the time the
    // call returns everything should be flushed.
    DeferTraps (false);
    EstbEventLog::Singleton().SendDeferredAsyncs(false);
    DeferTraps (true);
  }

  // Turn off RF SNMP access.  Do this by way of setting our IP address to
  // 0.0.0.0, which will disable management plus remove our current IP address
  // from the ipAddrTable
  
  // Unrestricted view (so config file can work OK next time)
  SetV1V2View ("unrestricted");

  // Write NV
  WriteNonVolSettings();
  
  // Call the base class
  DefaultSnmpAgentClass::InformRestartPending();
}

// This method is called when we are about to reboot, ie via docsDevResetNow.
void EstbSnmpAgent::InformShutdownPending ()
{
  // Disable management now, don't wait the 500 msecs.  Otherwise we might
  // still reply to queries that come in after a reset, which can trick
  // test stations who are polling us into thinking we've already rebooted
  // when we have not.
  EnableManagement (false);
  
  // Note that we do NOT want traps deferred, that's kind of the point of
  // the ThreadSleep below.  Disabling management will defer traps, so turn
  // 'em back on now.
  DeferTraps (false);
  
  // Short delay to allow any pending replies, traps, etc. to 
  // make it out.  Really should have some better synchronization mechanism
  // here...
  BcmOperatingSystemFactory::ThreadSleep (500);
  
  // Turn off all subsequent event logging.  
  // PR 6763 (ECW14): Do this after the 500ms wait to allow any events logged 
  // right before shutdown to get processed and possibly send traps etc.
  EstbEventLog::Singleton().DisableLogging();
  
  // Write NV.  Do this synchronously so we make sure that the
  // NV is written OK.
  WriteNonVolSettings(BcmNonVolSettings::kDynamicSection, false);
  
  // Call base class method
  DefaultSnmpAgentClass::InformShutdownPending ();
}

void EstbSnmpAgent::ResetMibDefaults ()
{
  if (FactoryMode() == true)
  {
    // If we're in factory mode, don't reset things here.
    SnmpLogRaw << "EstbSnmpAgent::ResetMibDefaults not restting due to factory mode" << endl;
    return;
  }
  
  SnmpLogRaw << GetName() << "Resetting to default state." << endl;
  
  // Destroy any users we've set up
  DestroyUsers ();
  
  // Likewise with any notify things
  DestroyNotifies ();
  
  // Flush WAC list
  fTlv10Engine.Flush();

  /* Reset event log things */
  
  EstbEventLog::Singleton().UseDefaultReporting ();
  EstbEventLog::Singleton().SetSyslogIp (BcmIpAddress(0,0,0,0));
  EstbEventLog::Singleton().SetThrottleAdminStatus (kUnconstrained);
  EstbEventLog::Singleton().SetThrottleThreshold (0);
  EstbEventLog::Singleton().SetThrottleInterval (1);
  EstbEventLog::Singleton().SetTrapControl (0);
  
  /* Reset NM access things */
  
  if (fpNmAccessTable)
    fpNmAccessTable->Flush();
  
  if (fpCommunityTable)
    fpCommunityTable->Flush();
  if (fpTargetExtTable)
    fpTargetExtTable->Flush();
  if (fpDHKickstartTable)
    fpDHKickstartTable->Flush();
  if (fpDHUserKeyTable)
    fpDHUserKeyTable->Flush();
  if (fpIpNetToMediaTable)
    fpIpNetToMediaTable->Flush();
  if (fpIpNetToPhysicalTable)
    fpIpNetToPhysicalTable->Flush();
  if (fpHrSWRunTable)
    fpHrSWRunTable->Flush();
  if (fpHrSWRunPerfTable)
    fpHrSWRunPerfTable->Flush();
    
  // return to default snmp mode, NmAccess.
  fSnmpMode = SNMP_MODE_NMACCESS;
}

typedef struct lnxHrSWRunEntryBridgeNode
{
	lnxHrSWRunEntryBridge *pBridge;
	lnxHrSWRunPerfEntryBridge *pPerfBridge;
	struct lnxHrSWRunEntryBridgeNode *next;
} lnxHrSWRunEntryBridgeNode;

// This function scans the Linux Process table
int EstbSnmpAgent::ScanLinuxProcessTable () {

  DIR            *procdir = NULL;
  struct dirent  *procentry_p;
  int             pid = 0;
  int             count = 0;
  lnxHrSWRunEntryBridge *pBridge = NULL;
  lnxHrSWRunPerfEntryBridge *pPerfBridge = NULL;
  static lnxHrSWRunEntryBridgeNode * head = NULL;
  lnxHrSWRunEntryBridgeNode *tmpNode;

  if (fpHrSWRunTable)
    fpHrSWRunTable->Flush();
  if (fpHrSWRunPerfTable)
    fpHrSWRunPerfTable->Flush();

  procdir = opendir("/proc");

  LockMib();

  // Get rid of old bridge entries
  while (head)
  {
	tmpNode = head;
	head = head->next;
	delete tmpNode->pBridge;
	delete tmpNode->pPerfBridge;
	delete tmpNode;
  }
  
  while ((procentry_p = readdir(procdir)) != NULL) {

    pid = atoi(procentry_p->d_name);
    if (pid > 0) {
      pBridge = new lnxHrSWRunEntryBridge (pid, this);
      pPerfBridge = new lnxHrSWRunPerfEntryBridge (pid, this);
      if ((pBridge) && (pPerfBridge)) {
		  lnxHrSWRunEntryBridgeNode * curr = new lnxHrSWRunEntryBridgeNode;
		  if (curr)
		  {
			  curr->pBridge = pBridge;
			  curr->pPerfBridge = pPerfBridge;
			  curr->next = head;
			  head = curr;
		  }
        pBridge->Register(kDeleteBridge);
        pPerfBridge->Register(kDeleteBridge);
        count++;
      }
    }

  }

  UnlockMib();

  closedir(procdir);

  return (count);

}

typedef struct lnxIpNetToMediaEntryBridgeNode
{
	lnxIpNetToMediaEntryBridge *pBridge;
	lnxIpNetToPhysicalEntryBridge *pPBridge;
	struct lnxIpNetToMediaEntryBridgeNode *next;
} lnxIpNetToMediaEntryBridgeNode;

// This function scans the Linux ARP table
void EstbSnmpAgent::ScanLinuxArpTable () {

  FILE           *in;
  char            line[128];
  int             za, zb, zc, zd, ze, zf, zg, zh, zi, zj, type;
  char            ifname[21];
  BcmString  ipAddrString;
  BcmIpAddress  ipAddr;
  BcmMacAddress macAddr;
  lnxIpNetToMediaEntryBridge *pBridge = NULL;
  lnxIpNetToPhysicalEntryBridge *pPBridge = NULL;
  static lnxIpNetToMediaEntryBridgeNode * head = NULL;
  lnxIpNetToMediaEntryBridgeNode *tmpNode;
    
  in = fopen("/proc/net/arp", "r");
  if (!in) {
    SnmpLogRaw << "EstbSnmpAgent can't open /proc/net/arp: " << endl;
    return;
  }
  
  LockMib();

  // Get rid of old bridges
  if (fpIpNetToMediaTable)
    fpIpNetToMediaTable->Flush();

  if (fpIpNetToPhysicalTable)
    fpIpNetToPhysicalTable->Flush();

  // Get rid of old bridge entries
  while (head)
  {
	tmpNode = head;
	head = head->next;
	delete tmpNode->pBridge;
	delete tmpNode->pPBridge;
	delete tmpNode;
  }

  // Get rid of the header line
  fgets(line, sizeof(line), in);

  while (fgets(line, sizeof(line), in)) {
    int             flags;

    if (12 != sscanf(line,
           "%d.%d.%d.%d 0x%*x 0x%x %x:%x:%x:%x:%x:%x %*[^ ] %20s\n",
           &za, &zb, &zc, &zd, &flags, &ze, &zf, &zg, &zh, &zi,
           &zj, ifname)) {
      SnmpLogRaw << "EstbSnmpAgent got Bad line in /proc/net/arp: " << line << endl;
      continue;
    }

    // Invalidated entries have their flag set to 0.
    // Ignore them
    if (flags == 0) {
      continue;
    }

    ipAddrString = (uint8) za;
    ipAddrString += (uint8) zb;
    ipAddrString += (uint8) zc;
    ipAddrString += (uint8) zd;
    ipAddr.Set((uint8) za, (uint8) zb, (uint8) zc, (uint8) zd);
    macAddr.Set((uint8) ze, (uint8) zf, (uint8) zg, (uint8) zh,
                (uint8) zi, (uint8) zj);
    
   type = (flags & 0x04) ? 
          CONSTVAL_IPNETTOMEDIATYPE_STATIC : CONSTVAL_IPNETTOMEDIATYPE_DYNAMIC;

    pBridge = new lnxIpNetToMediaEntryBridge (macAddr, ipAddr, type, this);
    // We only have ifIndex 1 and support pv4(1).
    pPBridge = new lnxIpNetToPhysicalEntryBridge (pBridge);
    if ((pBridge) && (pPBridge))
    {
        pPBridge->Set_ipNetToPhysicalPhysAddress (macAddr);
        pPBridge->Set_ipNetToPhysicalType (type);
	lnxIpNetToMediaEntryBridgeNode * curr = new lnxIpNetToMediaEntryBridgeNode;
		if (curr)
		{
			curr->pBridge = pBridge;
			curr->pPBridge = pPBridge;
			curr->next = head;
			head = curr;
		}
        pBridge->Register(kDeleteBridge);
        pPBridge->Register(kDeleteBridge);
    }
  }

  UnlockMib();

  fclose(in);
}

// This function scans all the Linux tables that may have
// entries that come and go periodically, like the ipNetToMedia table
void EstbSnmpAgent::ScanLinuxTables () {

  ScanLinuxArpTable();
  ScanLinuxProcessTable();
}


void EstbSnmpAgent::EnableManagement (bool Enable)
{
  LockMib();
    
  if (Enable == false)
  {
    // Do these need to be asynchronous?  Synchronous is better so we can
    // improve the chances that the traps get out in time...
    SendStandardTrap (LINK_DOWN_TRAP, 1); //, true);
  }
    
  // This will send a cold or warm start trap, plus send any
  // deferred standard traps.
  DefaultSnmpAgentClass::EnableManagement (Enable);
  
  if (Enable == true)
  {
    // Do these need to be asynchronous?  Messes up the order of sending
    // traps if they are.  We want coldStart, linkUp, eventLog traps.
    SendStandardTrap (LINK_UP_TRAP, 1); // , true);
    
    // Send any event log traps / syslog messages.
    EstbEventLog::Singleton().SendDeferredAsyncs();
  }
  
  UnlockMib();
}

SNMP_STATUS EstbSnmpAgent::ConfigDocsisViews (unsigned long *pSubtree, int SubtreeSize, SNMP_VIEW_STATUS Status)
{
  // In general, the only views with full access to all MIB objects are the
  // docsisNmAccessView and docsisManagerView.  Other views have only very
  // restricted access and should not be tweaked after initial setup.
  AgentConfigView ((char *) "docsisNmAccessView", pSubtree, SubtreeSize, Status);
  AgentConfigView ((char *) "docsisManagerView", pSubtree, SubtreeSize, Status);
  
  return SNMP_NO_ERROR;
}


SNMP_STATUS EstbSnmpAgent::InstallViews ()
{
  SnmpLogRaw << "EstbSnmpAgent installing views..." << endl;
  
  {
    // 'unrestricted' view is used during config-file processing.
    unsigned long everything[] = {1};
    AgentConfigView ((char *) "unrestricted", everything, sizeof(everything)/sizeof(unsigned long), VIEW_STATUS_INCLUDE);

  
    /*\
     *  Configure standard views: 
     *
     *    docsisNmAccessView
     *    docsisManagerView
     *    docsisOperatorWriteView
     *    docsisMonitorView
     *    docsisUserView
     *
     *  View details are in section 5.2.4 of the 2.0 spec
    \*/
    
    // All the OID subtrees involved in required views: 
    // NOTE: is this too much stack data?  We might need to break this
    // up into multiple functions if so.
    unsigned long entireMib[]                     = {1,3,6,1}; 
    unsigned long system[]                        = {1,3,6,1,2,1,1};

    unsigned long usmDHKickstartTable[]           = {1,3,6,1,3,101,1,2,1};
    unsigned long usmDH[]                         = {1,3,6,1,3,101};
    unsigned long v3Mibs[]                        = {1,3,6,1,6,3};
    //unsigned long bpi2Mib[]                       = {1,3,6,1,2,1,10,127,6};
    unsigned long snmpSetSerialNo[]               = {1,3,6,1,6,3,1,1,6,1};
    //unsigned long igmpStdMIB[]                    = {1,3,6,1,2,1,85};
    
    
    /* NmAccess mode view */

    // docsisNmAccessView
    AgentConfigView ((char *) "docsisNmAccessView", entireMib, sizeof(entireMib)/sizeof(unsigned long), VIEW_STATUS_INCLUDE);
    AgentConfigView ((char *) "docsisNmAccessView", v3Mibs, sizeof(v3Mibs)/sizeof(unsigned long), VIEW_STATUS_EXCLUDE);
    AgentConfigView ((char *) "docsisNmAccessView", usmDH, sizeof(usmDH)/sizeof(unsigned long), VIEW_STATUS_EXCLUDE);
    AgentConfigView ((char *) "docsisNmAccessView", snmpSetSerialNo, sizeof(snmpSetSerialNo)/sizeof(unsigned long), VIEW_STATUS_INCLUDE);
    
    // dhKickstartView
    SnmpLogStart << "Configuring view 'dhKickstartView'" << endl;
    AgentConfigView ((char *) "dhKickstartView", system, sizeof(system)/sizeof(unsigned long), VIEW_STATUS_INCLUDE);
    AgentConfigView ((char *) "dhKickstartView", usmDHKickstartTable, sizeof(usmDHKickstartTable)/sizeof(unsigned long), VIEW_STATUS_INCLUDE);
  }
  
  SetV1V2View ("unrestricted");
  
  // Give base class a chance to install its views.
  return DefaultSnmpAgentClass::InstallViews ();
}

SNMP_STATUS EstbSnmpAgent::InstallUsers ()
{
  SnmpLogRaw << "EstbSnmpAgent installing users..." << endl;

  // Install groups first
  AgentInstallGroup ((char *) "docsisManager", (char *) "docsisManager", SNMP_SEC_MDL_USM);
  AgentInstallGroup ((char *) "docsisOperator", (char *) "docsisOperator", SNMP_SEC_MDL_USM);
  AgentInstallGroup ((char *) "docsisMonitor", (char *) "docsisMonitor", SNMP_SEC_MDL_USM);
  AgentInstallGroup ((char *) "docsisUser", (char *) "docsisUser", SNMP_SEC_MDL_USM);
  AgentInstallGroup ((char *) "dhKickstart", (char *) "dhKickstart", SNMP_SEC_MDL_USM);
  
  
  // NO - we're going to do this selectively if we get the TLV34
  // Then users
  //AgentInstallUser ("docsisManager", SNMP_SEC_PRIV);
  //AgentInstallUser ("docsisOperator", SNMP_SEC_PRIV);
  //AgentInstallUser ("docsisMonitor", SNMP_SEC_AUTH);
  //AgentInstallUser ("docsisUser", SNMP_SEC_AUTH);
  AgentInstallUser ((char *) "dhKickstart", SNMP_SEC_NONE);
  
  // Then access
  AgentInstallAccess ((char *) "docsisManager", (char *) "docsisManagerView", (char *) "docsisManagerView", (char *) "docsisManagerView", SNMP_SEC_PRIV, SNMP_SEC_MDL_USM);
  AgentInstallAccess ((char *) "docsisOperator", (char *) "docsisManagerView", (char *) "docsisOperatorWriteView", (char *) "docsisManagerView", SNMP_SEC_PRIV, SNMP_SEC_MDL_USM);
  AgentInstallAccess ((char *) "docsisOperator", (char *) "docsisManagerView", (char *) "docsisOperatorWriteView", (char *) "docsisManagerView", SNMP_SEC_AUTH, SNMP_SEC_MDL_USM);
  AgentInstallAccess ((char *) "docsisMonitor", (char *) "docsisMonitorView", (char *) "", (char *) "docsisMonitorView", SNMP_SEC_AUTH, SNMP_SEC_MDL_USM);
  AgentInstallAccess ((char *) "docsisUser", (char *) "docsisUserView", (char *) "", (char *) "", SNMP_SEC_AUTH, SNMP_SEC_MDL_USM);
  AgentInstallAccess ((char *) "dhKickstart", (char *) "dhKickstartView", (char *) "", (char *) "", SNMP_SEC_NONE, SNMP_SEC_MDL_USM);

  //trc- moved to dhKickstart thread
  //EnableUser(BcmString("dhKickstart"));
     
  return SNMP_NO_ERROR;
}

SNMP_STATUS EstbSnmpAgent::AuthenticateRequest (int RequestType, int SnmpVersion, int &SrcIfNum, const BcmIpAddress &SrcIp, const BcmString &Community)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  
  
  // Let the base class have a whirl.
  Status = DefaultSnmpAgentClass::AuthenticateRequest (RequestType, SnmpVersion, SrcIfNum, SrcIp, Community);
  if (Status != SNMP_NO_ERROR)
    return Status;
  
  // If this is an internal request coming from 127.0.0.1, then let it through
  // no matter what.  This means it's coming from the config file.  Although
  // we check this in the base class, we need to do so here as well because we
  // might have had a TLV-34 in the config file which put us into coexistence
  // mode before we get around to processing TLV-11's, and in that case the
  // check for NMACCESS mode below would kick us out.
  if (SrcIp == BcmIpAddress(127,0,0,1))
  {
    SnmpLogNm << "  Internal request (CM), allowing access." << endl;
    return SNMP_NO_ERROR;
  }
  
  // If we are not in NMACCESS mode, then return NO_OPINION and let the portable
  // agent handle it.
  if (GetDocsisSnmpMode() != SNMP_MODE_NMACCESS)
  {
    SnmpLogNm << "  Not in NMACCESS mode, NO_OPINION" << endl;
    return SNMP_NO_OPINION;
  }
    
  SnmpLogNm << "  No Coexistence configured, NO_ACCESS" << endl;
    return SNMP_NO_ACCESS;
  
}


SNMP_STATUS EstbSnmpAgent::ValidateSet (const BcmObjectId &OID, const BcmVarBind &VB, const BcmIpAddress &SrcIp, int SnmpVersion, const BcmString &Community)
{
  SNMP_STATUS Status = DefaultSnmpAgentClass::ValidateSet (OID, VB, SrcIp, SnmpVersion, Community);
  
  if (Status != SNMP_NO_ERROR)
    return Status;
  
  if (SrcIp == BcmIpAddress(127,0,0,1))
  {
    SnmpLogReq << "  SET is from 127.0.0.1, NO ERROR." << endl;
    return SNMP_NO_ERROR;
  }
  
  return fTlv10Engine.ValidateOid (OID);
}


/* Trap things */

int EstbSnmpAgent::SendTrap (BcmTrap *pTrap)
{
  int NumTraps = 0;
  
  
  // Give the base class a chance to defer or drop the trap.
  NumTraps = DefaultSnmpAgentClass::SendTrap (pTrap);
  if ((NumTraps == 0) || (NumTraps == -1))
    return NumTraps;
    
  // Check to see if we are throttled.
  if (EstbEventLog::Singleton().IsThrottled() == true)
    return 0;
  
  // If we're in coexistence mode, use the various trap control mibs to send
  // a notify.
  if (GetDocsisSnmpMode() == SNMP_MODE_COEXISTENCE)
  {
    if (pTrap->SendNotify (GetIpAddr()) == SNMP_NO_ERROR)
      NumTraps = 1;
  }
  else {
    // otherwise (NmAccess mode) use the NmAccess table as usual.
    NumTraps = fpNmAccessTable->SendTrap (pTrap);
  }
    
  return NumTraps;
}

// Note that this is called when sending standard traps, such as SNMP auth
// failures.  Traps sent in conjunction with events are done through
// EventLogEntry::SendTraps
int EstbSnmpAgent::SendStandardTrap (int TrapType, int IfIndex, bool Async)
{
  if (Async)
  {
    EstbSendTrap (TrapType, IfIndex);
    return 0;
  }

  int SentTraps = 0;
  
  if (EstbEventLog::Singleton().IsThrottled() == true)
  {
    SnmpLogEv << "EstbEventLog::SendStandardTrap - async msgs throttled, no traps." << endl;
    return 0;
  }
  
  SentTraps = DefaultSnmpAgentClass::SendStandardTrap (TrapType, IfIndex, false);
  
  // Per CL at CW19 post mortem 10/4, throttle traps per trap event, not
  // per trap packet.  That is, if we send the same trap to 2 targets, it
  // counts as 1 trap for throttling purposes, not 2.
  if (SentTraps > 0)
    EstbEventLog::Singleton().SentAsyncMsg(NULL, 1); //SentTraps);
    
  return SentTraps;
}


// This is exactly the same as DefaultSnmpAgentClass::SendDeferredTraps except that we
// increment the async msg counter in our event log.
int EstbSnmpAgent::SendDeferredTraps ()
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
    {
      // Let the event log know we just sent a trap.
      EstbEventLog::Singleton().SentAsyncMsg(NULL, 1);
      SentTraps++;
    }
    
    pNextTrap = pTrap->fpNext;
    delete pTrap;
    pTrap = pNextTrap;
  }
  
  fpDeferredTraps = NULL;
  
  UnlockMib();
  
  SnmpLogRaw << "Done w/ deferred traps." << endl;
  
  return SentTraps;
}


/* Handlers for ACT events */

void EstbSnmpAgent::InterfaceOperationalEvent ()
{
  BcmIpAddress MyAddress;
  BcmIpAddress MyNetmask;
  BcmIpAddress Router;
  BcmIpAddressList logServers;
  char myInterfaceName[ESTB_SNMP_INTERFACE_NAME_LEN] = ESTB_SNMP_DEFAULT_INTERFACE;
  uint32 tmpAddr = 0;

  SnmpLogRaw << "EstbSnmpAgent::InterfaceOperationalEvent for " << GetName() << endl;
  SnmpLogRaw << "EstbSnmpAgent::InterfaceOperationalEvent fInterfaceName " << fInterfaceName << endl;
  MyAddress.Set(kBroadcastIp);  // initialize to a NULL address
  if (fInterfaceName.length() <= 0)  {
    fInterfaceName = myInterfaceName;
    SnmpLogRaw << "    using default interface " << myInterfaceName << endl;
  } else {
    bzero(myInterfaceName, sizeof(myInterfaceName));
    strcpy(myInterfaceName, fInterfaceName.c_str());
    SnmpLogRaw << "    using configured (in NonVol) interface " << fInterfaceName << endl;
  }
  if (strcmp(myInterfaceName, ESTB_SNMP_ANY_INTERFACE) == 0) {
    MyAddress.Set(kAllZerosIp);
    MyNetmask.Set(kBroadcastIp);
    Router.Set(kAllZerosIp);
    SnmpLogRaw << "    listening on all interfaces " << endl;
    ConfigIp (MyAddress, MyNetmask, Router);
  } else {
    if (GetIpStackAddress(&tmpAddr, myInterfaceName) == 0) {
      MyAddress.Set((uint32)htonl(tmpAddr));
      SnmpLogRaw << "    IP addr = " << MyAddress << endl;
    } else  {
      SnmpLogRaw << "    Get new IP addr failed " << endl;
    }
    if (GetIpStackNetmask(&tmpAddr, myInterfaceName) == 0) {
      MyNetmask.Set((uint32)htonl(tmpAddr));
      SnmpLogRaw << "    Netmask = " << MyNetmask << endl;
    } else  {
      SnmpLogRaw << "    Get new Netmask failed " << endl;
    }
    Router = kAllZerosIp;

    // Why do we need a router addr anyway?
    // pLease->ServerLeaseSettings().GetOption3OnNetRouter(Router);

    SnmpLogRaw << "    Router IP addr = " << Router << endl;
    if (InterfaceIsUp(myInterfaceName)) {
      SnmpLogRaw << "  Interface " << myInterfaceName << " is UP!!!" <<endl;
      ConfigIp (MyAddress, MyNetmask, Router);
    } else {
      SnmpLogRaw << "  Interface " << myInterfaceName << " is DOWN!!! " <<endl;
    }
  }

  // pLease->ServerLeaseSettings().GetOption(kDhcpLogServer, logServers);
  // EstbEventLog::Singleton().SetLogServers (logServers);

  InformRegistrationComplete ();
}


void EstbSnmpAgent::EstbConfigFileNewEvent ()
{
  LoadConfigFile("./estbconfigsettings.bin");
}

void EstbSnmpAgent::SystemTimeChangeEvent()
{
  SnmpLogRaw << "  DefaultSnmpAgentClass::SystemTimeChangeEvent for " << GetName() << endl;
  
#if (SNMP_SUPPORT == 3)
  //cout<<"systemtimechangeevent_ reinit enginetimes"<<endl;
  //LockMib();
  //AgentUpdateEngineTimes();
  //UnlockMib();
#endif

}

void EstbSnmpAgent::ShutdownPendingEvent()
{
  SnmpLogRaw << "  DefaultSnmpAgentClass::ShutdownPendingEvent for " << GetName() << endl;
  InformShutdownPending ();
}

SNMP_STATUS EstbSnmpAgent::LoadConfigFile (const char *Path)
{
  FILE *fd;
  size_t bytesRead;
  char ElementName[32];
  char ElementTag[32];
  char V1GroupName[32];
  char V2GroupName[32];
  int ElementNum, ElementTagNum;
  BcmString SecurityName;
  BcmSnmpV1V2Transport *pTxport;
  BcmSnmpV1V2Coexistence *pCoex;
  BcmSnmpV3AccessView *pView;

  // Create a buffer to download with.  Make him a whopper, so we can test
  // enormo-files. 
  BcmOctetBuffer Buffer (64*1024);
  
  // When we call InformRestartPending we'll wipe the agent's IP settings,
  // so remember them now and re-apply later.
  BcmIpAddress MyIp = GetIpAddr();
  BcmIpAddress MySubnet = GetSubnetMask();
  BcmIpAddress MyRouter = GetRouter();
  
  // Call InformRestartPending so that we flush all tables, etc.
  InformRestartPending();
  
  SnmpLogRaw << "Loading Estb config file " << Path << endl;
  
  if (Path == NULL) {
    SnmpLogRaw << "eSTB config file path not specified!" << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }

  fd = fopen (Path, "rb");

  if (fd == NULL) {
    SnmpLogRaw << "eSTB config file not opened" << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }

  Buffer.Empty();

  // Read from file into buffer
  bytesRead = fread(Buffer.AccessBuffer(), 1, Buffer.BufferSize(), fd); 

  if (!feof(fd)) {	// File too big!!
    SnmpLogRaw << "eSTB Config File Too Big!" << endl;
    Buffer.Empty();
    fclose(fd);
    return SNMP_RESOURCE_UNAVAILABLE;
  }

  //  Since we did the manually.........
  Buffer.SetBytesUsed(bytesRead);

  // clean up, now
  fclose(fd);

  // SnmpLogRaw << "Config file binary contents:" << Buffer << endl;
  
  // Create a config file object and feed him the buffer we just read.
  BcmEstbConfig Cfg;
  unsigned int Offset = 0;
  
  Cfg.UsageContext (kEstbConfigFile);
  if (Cfg.ParseFrom (Buffer, Offset) != 0)
  {
    SnmpLogRaw << "Error parsing file!" << endl;
    EstbLogEvent(kEE_T40103);
    return SNMP_RESOURCE_UNAVAILABLE;
  }
  
  if (!Cfg.IsValid())
  {
    SnmpLogRaw << "Config file failed validation check!" << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }
  
  // SnmpLogRaw << Cfg << endl;
  
  // All we care about are TLV-11's (SNMP-sets), TLV-38's (Notifies), 
  // TLV-53's (V1V2 Coexistence), and TLV-54's (Views).  We don't
  // care about any of the other settings.  So just reset the agent, then
  // feed him these settings.
  
  // Apply settings in the same order that the DocsisCtlThread does.
  // The following code is cut-and-pasted from 
  // BcmCmDocsisCtlThread::TestAndApplyPreRegConfigFileItems()
  
  // 5) SNMP3 Kickstart value list.
  // while( !Cfg.fSnmp3KickList.empty() )
  // {
      // pass kickstart value to SNMP agent.
  //     ProvisionDHKickstartEntry( 
  //         (Cfg.fSnmp3KickList.front()).fKickSecurityName,
  //         (Cfg.fSnmp3KickList.front()).fKickMgrPublicNum );
      //
      // delete the object at the front() of the list.
  //     Cfg.fSnmp3KickList.pop_front();
  // }
  
  // Estb Snmpv1v2 Coexistence Configuration Table entries
  // SnmpLogRaw << "Applying TLV-53 from config file..." << endl;
              
  memset(ElementName, 0, sizeof(ElementName));
  memset(ElementTag, 0, sizeof(ElementTag));
  ElementNum = 0;
  while( !Cfg.fSnmpV1V2CoexistenceList.empty() ) {
    pCoex = &(Cfg.fSnmpV1V2CoexistenceList.front());
    sprintf (ElementName, "%s_%d", GetTLV53Base(),ElementNum);
    sprintf (V1GroupName, "%sV1_%d", GetTLV53Base(),ElementNum);
    sprintf (V2GroupName, "%sV2_%d", GetTLV53Base(),ElementNum);
    ElementNum++;

    // SnmpLogRaw << "Applying TLV-53 for "<< ElementName << endl;
    ProvisionCommunityEntry( BcmString(ElementName), pCoex->fCommunityName,
                             BcmString(ElementName), pCoex->fTag);

    ElementTagNum = 0;
    while( !pCoex->fSnmpV1V2TransportList.empty() ) {
      pTxport = &(pCoex->fSnmpV1V2TransportList.front());
      sprintf (ElementTag, "%sTag_%d", GetTLV53Base(),ElementTagNum);
      ElementTagNum++;

      // SnmpLogRaw << "Applying TLV-53 transport list for "<< ElementTag << endl;
      ProvisionTargetAddrEntry( BcmString(ElementTag),
        pTxport->fTransportAddress, pCoex->fTag);
 
      if (pTxport->fTransportAddressMaskSpecified) 
        ProvisionTargetAddrExtEntry( BcmString(ElementTag),
                                     pTxport->fTransportAddressMask);
  
      pCoex->fSnmpV1V2TransportList.pop_front();
    }

    ProvisionSecurityToGroupEntry( SNMP_SEC_MDL_V1, BcmString(ElementName),
      BcmString(V1GroupName) );
    ProvisionSecurityToGroupEntry( SNMP_SEC_MDL_V2, BcmString(ElementName),
      BcmString(V2GroupName) );
 
    if (!pCoex->fViewTypeSpecified)  
      pCoex->fViewType = 1;   // Read-only by default
    ProvisionVacmAccessEntry( BcmString(V1GroupName), pCoex->fViewType, 
                                pCoex->fViewName, SNMP_SEC_MDL_V1);
    ProvisionVacmAccessEntry( BcmString(V2GroupName), pCoex->fViewType, 
                                pCoex->fViewName, SNMP_SEC_MDL_V2);
  
    // delete the object at the front() of the list.
    Cfg.fSnmpV1V2CoexistenceList.pop_front();
  }
  
  // Estb Snmpv1v2 View Configuration Table entries
  // SnmpLogRaw << "Applying TLV-54 from config file..." << endl;
              
  while( !Cfg.fSnmpV3AccessViewList.empty() ) {
    pView = &(Cfg.fSnmpV3AccessViewList.front());

    // SnmpLogRaw << "Applying TLV-54 for "<< pView->fName << endl;
    ProvisionVacmViewTreeFamilyEntry( pView->fName,
          pView->fSubtreeOID, pView->fMask, pView->fType );
  
    // delete the object at the front() of the list.
    Cfg.fSnmpV3AccessViewList.pop_front();
  }
  
  // SnmpLogRaw << "Applying TLV-38 from config file..." << endl;
              
  // OSSI-IO3 section 3.6: docsisV3NotificationReceiver
  memset(ElementName, 0, sizeof(ElementName));
  ElementNum = 0;
  
  {
    
    while( !Cfg.fSnmpV3NotifyList.empty() )
    {
        sprintf (ElementName, "%s_%d", GetTLV38Base(),ElementNum);
        ElementNum++;
        
        SecurityName.assign ((char *)Cfg.fSnmpV3NotifyList.front().fSecurityName.AccessBuffer(),
                             Cfg.fSnmpV3NotifyList.front().fSecurityName.BytesUsed());

        //cout<<"-------------------TLV 38-----------------------------"<<endl;
        //cout<<Cfg.fSnmpV3NotifyList.front()<<endl;
        
        if((!Cfg.fSnmpV3NotifyList.front().fTypeSpecified) ||
           (!Cfg.fSnmpV3NotifyList.front().fIpAddressSpecified))
        {  
            //a required parameter was not specified.
            EstbLogEvent( kRescanCfgFileInvalid );

            SnmpLogRaw << "\n\tErrors with TLV-38 (SNMPv3 NotifyReceiver) items." << endl;
            
            return SNMP_WRONG_VALUE;
        }
            
        // SNMP3 notify receiver list.
        //
        // pass notify receiver value to SNMP agent.
        ProvisionNotifyEntry( 
            (Cfg.fSnmpV3NotifyList.front()).fType,
            BcmString (ElementName),
            // Get aload of this line:
            // KO MOD: this won't work, fSecurityName is not NULL
            // terminated.  Use BcmString::assign, above and specify
            // length
            //BcmString((char *)Cfg.fSnmp3NotifyList.front().fSecurityName.AccessBuffer()),
            SecurityName,
            (Cfg.fSnmpV3NotifyList.front()).fIpAddress,
            (Cfg.fSnmpV3NotifyList.front()).fPort,
            ((Cfg.fSnmpV3NotifyList.front()).fTimeout)/10, //TLV38 timout units are microseconds
            (Cfg.fSnmpV3NotifyList.front()).fRetries,      // and target mib is hundrendths of seconds.
            (Cfg.fSnmpV3NotifyList.front()).fFilterParams);
        //
        // delete the object at the front() of the list.
        Cfg.fSnmpV3NotifyList.pop_front();
    }
  }
  
  // SnmpLogRaw << "Applying TLV-11 from config file..." << endl;
              
  // 6) SNMP MIB object transaction list
  while( !Cfg.fSnmpMibObjectList.empty() )
  {
      // SNMP MIB object attributes present.
      //
      // convert BcmOctetBuffer object at the front()
      // of the SNMP mib object list to a byte* and 
      // pass to the SNMP agent.
      BerSetObject( 
          (Cfg.fSnmpMibObjectList.front()).AccessBuffer(),
          (Cfg.fSnmpMibObjectList.front()).BytesUsed());
      //
      // delete the BcmOctetBuffer object at the front()
      // of the list.
      Cfg.fSnmpMibObjectList.pop_front();
  } 
  
  // tell snmp agent to process SNMP config file items.
  //
  
  // SnmpLogRaw << "Informing agent provisioning complete..." << endl;  
  
  SNMP_STATUS Status = InformProvisioningComplete();
  if (Status != SNMP_NO_ERROR)
  {
      // error.  something was amiss with our TLV-11s and we gotta reject 
      // the config file and not register.
      // Log event(s)
      EstbLogEvent( kRescanCfgFileInvalid );
      
      return Status;
  }
  
  // Restore IP settings from before
  ConfigIp (MyIp, MySubnet, MyRouter);
  
  // SnmpLogRaw << "Informing agent registration complete..." << endl;  
  
  InformRegistrationComplete();
  
  return SNMP_NO_ERROR;
}

#if (BCM_EVD_INTERFACE_INCLUDED)
#if !(BCM_EVD_RPCCL_INTERFACE_INCLUDED)
void EstbSnmpAgent::SubscribeToEvents(void)
{
	// SUBSCRIBE TO EVENTS
	// Now subscribe to events.	
	unsigned int i=0;
	unsigned int eventList[kDefault_NumOfEvents] = kDefault_EventList;

    // create my ACT & hook it up to Event Dispatcher
    // the ACT will relay interesting state changes from the Event Dispatcher.
	fpCmDocsisStatusACT = new EstbSnmpAgentCmDocsisStatusACT (this);

	BcmEcmEventDispatcherThread *pEvDispatcher = BcmEcmEventDispatcherThread::GetSingletonInstance();

	for( i=0; i<kDefault_NumOfEvents; i++ )
	{
		if(pEvDispatcher)
			pEvDispatcher->SubscribeMsgNote (*(eventList+i), fpCmDocsisStatusACT);

		cout <<" ESTB SNMP AGENT: SubscribeMsgNote event " << (*(eventList+i)) << endl;
	}
}
#endif
#endif
