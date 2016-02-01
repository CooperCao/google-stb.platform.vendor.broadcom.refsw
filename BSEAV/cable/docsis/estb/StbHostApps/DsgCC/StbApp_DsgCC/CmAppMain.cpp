//****************************************************************************
//
// Copyright (c) 2004-2013 Broadcom Corporation
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
//
//  Filename:       CmAppMain.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  Dec 06, 2004
//
//****************************************************************************
//  Description:
//      This file contains CM application startup code.
//
//      A bunch of this code was previously in the vxWorks/pSOS/Linux startup
//      code, and has been moved here since it isn't OS-specific, and we don't
//      like lots of duplicate code lying around.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "CmAppMain.h"

// Core V2 startup code.
#include "V2Main.h"

// The portable application and other DOCSIS components.
#include "CmApplication.h"

#if (SNMP_SUPPORT)
// These need to be included before DsgClientCtlThread.h or we run afoul of EnetPkt.h
#include "Forwarder.h"
#include "HalIfFactory.h"
#include "HalIf.h"
#include "BaseIpHalIf.h"
#include "StubHalIf.h"
#endif

// For Remote Interface Notification and Data  
#include "EstbRemoteIfHelperThread.h"

#if (BFC_INCLUDE_SEB_API_SUPPORT)
#include "SebRemoteIfHelperThread.h"
#endif

#include "DsgClientCtlThread.h"

#if (defined (BCM_DUAL_DOWNLOAD_SUPPORT) || defined (ESTB_RSWDLOAD_SUPPORT) )
#include "DsgDownloadCtlThread.h"

#if defined(BCM_CDL_LIB_SUPPORT)
#include "CommonDownloadHelperThread.h"
#endif
#endif

#if (defined (BCM_DUAL_DOWNLOAD_SUPPORT) || defined (COMMON_DOWNLOAD_SUPPORT) )
#include "DsgTftpServer.h"
#endif

#include "MainCommandTable.h"

#if (BCM_NONVOL_SUPPORT == 1)
	#include "CmAppCompositeNonVolCommandTable.h"
	#include "CmAppCompositeNonVolSettings.h"
	#include "CmAppNonVolSettings.h"
#endif

#if (SNMP_SUPPORT)
    #include "SnmpAgnt.h"
    #include "SnmpApi.h"
    // #include "OperatingSystemFactory.h"
    // Include the appropriate OS-specific derived class of the IP stack HalIf.
  #if defined(TARGETOS_vxWorks)
      #include "vxIpHalIf.h"
  #elif defined(TARGETOS_Linux)
      #include "LnxIpHalIf.h"
  #endif
#endif

#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE)
#if (BCM_NONVOL_SUPPORT ==1)
#include "DsgccNonVolSettings.h"
#endif
#endif

#include "ConsoleThread.h"

/*
#if (BCM_PING_SUPPORT)
    #include "PingThread.h"
#endif

*/
#if (BCM_CANNED_SNMP_SUPPORT)
#include "EstbCannedSnmp.h"
#endif

#if (BCM_CONSOLE_SUPPORT)
    #include "MsgLogSettingsCommandTable.h"
	#include "DsgCommandTable.h"

	#if defined(COMMON_DOWNLOAD_SUPPORT)
	#include "CommonDownloadCommandTable.h" 
	#endif

	#if (BCM_CANNED_SNMP_SUPPORT)
	#include "EstbCannedSnmpCommandTable.h"
	#endif

	#if (BFC_INCLUDE_DAVIC_API_SUPPORT)
	#include "DavicApiCommandTable.h"

		#if (BFC_INCLUDE_G8DAVIC_API_SUPPORT)
		#include "CiscoDavicApiCommandTable.h"
		#endif	

	#endif

	#if (BFC_INCLUDE_SEB_API_SUPPORT)
	#include "SebCommandTable.h"

		#if (BFC_INCLUDE_G8SEB_API_SUPPORT)
		#include "CiscoSebApiCommandTable.h"
		#endif	

	#endif
#endif

#include "EstbRemoteInterfaceStub.h"
//********************** Local Types *****************************************
    
//********************** Local Constants *************************************

//********************** Local Variables *************************************

static BcmCmApplication *pgCmApplication = NULL;

static bool gSkipDriverInit = false;  // PR1968

// PR4984 - Add new variable that separates the "brick" behavior from skipping
// driver init.
static bool gPermanentNonvolCorrupt = false;

static BcmMainCommandTable *pgMainCommandTable = NULL;

#if (BCM_PING_SUPPORT)

static BcmPingThread *pgPingHelper = NULL;

#endif
 
//#if (BCM_DSG_DUAL_PROCESSOR_INTERFACE)
//static char *pStr= NULL; 
//char ipcInterfaceName[IPC_INTERFACE_NAME_MAX_LEN];
//#endif

//********************** Local Functions *************************************

//********************** Function Implementations ****************************

extern "C"
{
	//extern SetupRemoteInterfaceReturnCodes SetupRemoteInterface(char *pIpcInterfaceName);
	extern char* GetEnvVarEcmInterface(void);
}

void AppPreNonvolInit(void)
{
	#if (BCM_NONVOL_SUPPORT == 1)
    unsigned int readResult;

    // Allow a vendor to create an application object, and initialize it.
    #if (BCM_VENDOR_SUPPORT) || (BCM_VENDOR_CMAPP_SUPPORT)

    pgVendorCmApplication = BcmVendorCmAppFactory::NewVendorCmApplication();

    if (pgVendorCmApplication != NULL)
    {
        pgVendorCmApplication->Initialize();
    }

    #endif

    // Create the non-vol settings singleton instances and read the data in
    // from the device.  Afterwards, create the command tables associated with
    // the settings (if compiled in).  The settings must exist before creating
    // the command tables, because the tables access the singleton settings
    // instances to automatically register them.
    BcmCmAppCompositeNonVolSettings *pNonVolSettings = NULL;

    // Create the root composite object, which creates the singleton objects
    // for all of the subsystems.
    pNonVolSettings = new BcmCmAppCompositeNonVolSettings;

    // Let the vendor create and register nonvol settings objects.
    #if (BCM_VENDOR_SUPPORT)

    BcmVendorCmAppFactory::CreateAndRegisterNonVolSettings(*pNonVolSettings);

    #endif

    // Call the V2 helper function to read and validate the settings.
    readResult = V2MainReadNonVol(pNonVolSettings);

    // If permanent nonvol is corrupted, then we need to "brick" ourselves so
    // that we don't attempt to operate with bogus settings (like MAC addresses
    // and security keys).
    if (readResult & kNonVolRead_ErrorReadingPermanent)
    {
        // Call out to the vendor to let them know about this.
        #if (BCM_VENDOR_SUPPORT)

        if (pgVendorCmApplication != NULL)
        {
            pgVendorCmApplication->PermanentNonVolIsCorrupt();
        }

        #endif

        // PR4984 - Need to tag this new variable; the "brick" thing doesn't
        // skip driver init any more.
        gPermanentNonvolCorrupt = true;
    }
	#endif
}

void AppPostNonvolInit(void)
{
    // Create the portable application object and register it with the main
    // command table.
    pgCmApplication = new BcmCmApplication;

#if (BCM_CONSOLE_SUPPORT)
    // Get the main command table.
    pgMainCommandTable = BcmMainCommandTable::GetCommandTableSingleton();

    pgMainCommandTable->AddInstance("BcmApplication", pgCmApplication);

	// Add the DSG command table as a subtable under my command table.
    pgMainCommandTable->AddSubtable(&BcmDsgClientCtlThread::GetCommandTableSingleton());

	#if (BCM_NONVOL_SUPPORT == 1)
    // Get the V2 core to add its command tables and commands.
    V2CmdTableInit(pgMainCommandTable, 4000);
	#endif

    #if (BCM_NONVOL_SUPPORT == 1)
    // Create the non-vol command table object.
    BcmCmAppCompositeNonVolCommandTable *pCmAppSettingsCommandTable = NULL;

    // Create the default command table if NULL
    pCmAppSettingsCommandTable = new BcmCmAppCompositeNonVolCommandTable;

    // Add the non-vol settings command table as a subtable under the main
    // command table.
    pgMainCommandTable->AddSubtable(pCmAppSettingsCommandTable);
	#endif

#if (SNMP_SUPPORT)
    // Add the SNMP command table as a subtable under the main
    // command table.
    pgMainCommandTable->AddSubtable(&BcmSnmpAgent::GetCommandTableSingleton());
#endif

#if defined(COMMON_DOWNLOAD_SUPPORT)
    // Add the Common Download command table as a subtable under the main
    // command table.

    static BcmCommonDownloadCommandTable gCommonDownloadCommandTable;
	gCommonDownloadCommandTable.AddInstance("Common Download", pgCmApplication);
    pgMainCommandTable->AddSubtable(&gCommonDownloadCommandTable);

#endif

#if (BCM_CANNED_SNMP_SUPPORT)
    // Add the eSTB Canned SNMP command table as a subtable under the main
    // command table.

	// Add the DSG command table as a subtable under my command table.
    pgMainCommandTable->AddSubtable(&BcmEstbCannedSnmp::GetCommandTableSingleton());
#endif

#if (BFC_INCLUDE_DAVIC_API_SUPPORT)
    // Add the DAVIC API command table as a subtable under the main
    // command table.

    static BcmDavicApiCommandTable gDavicApiCommandTable;
	gDavicApiCommandTable.AddInstance("DAVIC API", pgCmApplication);

    pgMainCommandTable->AddSubtable(&gDavicApiCommandTable);

	#if (BFC_INCLUDE_G8DAVIC_API_SUPPORT)
	static CiscoCmDavicApiCommandTable ciscoCmDavicApiCommandTable;
	ciscoCmDavicApiCommandTable.AddInstance("CISCO Davic API", &ciscoCmDavicApiCommandTable);
	gDavicApiCommandTable.AddSubtable (&ciscoCmDavicApiCommandTable);
	#endif
#endif

#if (BFC_INCLUDE_SEB_API_SUPPORT)
    // Add the SEB API command table as a subtable under the main
    // command table.

    static SebBfcCommandTable gSebApiCommandTable;
	gSebApiCommandTable.AddInstance("SEB API", pgCmApplication);

    pgMainCommandTable->AddSubtable(&gSebApiCommandTable);

	#if (BFC_INCLUDE_G8SEB_API_SUPPORT)
	static CiscoCmSebApiCommandTable ciscoCmSebApiCommandTable;
	ciscoCmSebApiCommandTable.AddInstance("CISCO SEB API", &ciscoCmSebApiCommandTable);
	gSebApiCommandTable.AddSubtable (&ciscoCmSebApiCommandTable);
	#endif
#endif

    // Add the message log settings command table as a subtable under the main
    // command table.
    //pgMainCommandTable->AddSubtable(&(BcmMessageLogSettings::GetCommandTableSingleton()));




    #if (BCM_PING_SUPPORT)

    // Create the Ping Helper object, and give a pointer to the CM App object.
    pgPingHelper = new BcmPingThread("Ping Thread", BcmOperatingSystem::kIdlePriority);

    pgCmApplication->PingHelper(pgPingHelper);
   
        
    #if (BCM_PING_CONSOLE_SUPPORT)
    
    // Enable/disable the ping command depending on whether a valid object
    // was created.
    if (pgPingHelper != NULL)
    {
        pgMainCommandTable->EnableCommand(BcmMainCommandTable::kPingCommand, true);
    }
    else
    {
        gLogMessageRaw << "WARNING:  No Ping Helper for the target platform!  Ping not supported..." << endl;

        pgMainCommandTable->EnableCommand(BcmMainCommandTable::kPingCommand, false);
    }
    #endif
    
    #endif // BCM_PING_SUPPORT



    // Get a pointer to the console thread.  If the "skip driver init" prompt
    // is enabled in nonvol, then we will give the user a chance to do so.
    BcmConsoleThread *pConsoleThread = BcmConsoleThread::GetSingletonInstance();

    // Set my command table with the console thread (as the main table).
    pConsoleThread->SetMainCommandTable(pgMainCommandTable);
#endif
//    BcmCmAppNonVolSettings *pCmAppNonVol = BcmCmAppNonVolSettings::GetSingletonInstance();
  
#if 0
    if ((pConsoleThread != NULL) )
		//&&        ((pCmAppNonVol == NULL) || (pCmAppNonVol->DriverInitPrompt() == true)))
    {
        // Before creating and starting up the drivers, give the user a chance
        // to bypass this.  This is most useful for systems where the nonvol
        // settings are not set correctly, and starting a driver will cause it
        // to crash.
        gLogMessageRaw << "\nPress any key in 1.5 seconds to skip driver initialization..." << flush;

        if (pConsoleThread->WaitForKeypress(1500))
        {
            // PR1968
            gSkipDriverInit = true;
        }

        gLogMessageRaw << endl;
    }

#ifdef BCM_SETTOP_SUPPORT
    else
    {
        gLogMessageRaw
            << "\nIf you pressed a key before this point, we will skip driver initialization..." << endl;

        if (pConsoleThread->WaitForKeypress(0))
        {
            gSkipDriverInit = true;
        }
    }
#endif
#endif
}


void AppRun(void)
{
    // PR1968 - reorganized this a bit to allow stopping early due to nonvol
    // failure or console override.  This is what "bricks" the modem.

	gSkipDriverInit = false;
    if (gSkipDriverInit)
    {
        gLogMessageRaw << "\n\nNOTE:  Driver initialization was skipped!  The application will not run or foward data.  You can change NonVol Setting, but that's about it...\n" << endl;

        //pgCmApplication->SetDocsisCtlThread(NULL);
        pgCmApplication->Run();
        
        return;
    }

	// Setup remote interface before anything else.
	#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE) || defined(BCM_DSG_DUAL_PROCESS_INTERFACE)
		#if !(defined ECM_STUB_API || DSGCC_BUILT_AS_LIB_FOR_ESTBOPENCABLESYSTEM)

			// Get eCM interface from environment variable.
			/*pStr = GetEnvVarEcmInterface();

			memset(ipcInterfaceName, 0, sizeof(ipcInterfaceName));
			if( pStr != NULL )
			{
				// If env var is read then set 
				if( strlen(pStr) > sizeof(ipcInterfaceName) )
				{
					gLogMessageRaw << "\n\nDSG-CC: strlen(pStr) > sizeof(ipcInterfaceName) \n!" << endl;
				}
				else
					strncpy(ipcInterfaceName, pStr, sizeof(ipcInterfaceName));
			}
			else
			{
				// env var does not exist. read from nonvol.
				#if (BCM_NONVOL_SUPPORT ==1)
				// set the IPC interface name
				strcpy(ipcInterfaceName, BcmDsgccNonVolSettings::GetSingletonInstance()->IpcInterfaceName());
				pStr = ipcInterfaceName;
				#endif
			}*/

			// First initialize the link. This is a blocking call.
			if(SetupRemoteInterface(NULL) != kRIsetupNoError)
			{
				gLogMessageRaw
					<< "\n\nFailed to configure remote interface \n!" << endl;
			}

			// Set up eSTB Remote Interface Helper Thread
			// Create my helper thread.	 
			BcmEstbRemoteIfHelperThread *pEstbRemoteIfHelperThread = new BcmEstbRemoteIfHelperThread (BcmOperatingSystem::kNormalPriority);

			#if (BFC_INCLUDE_SEB_API_SUPPORT) 
			gLogMessageRaw << "\n***** ATTENTION ***** SEB API is not initialized as default. Use console command Dsg/SebCtl>init_api!!!\n" << endl;
			#endif
		#endif
	#endif 

	#if (BCM_CANNED_SNMP_SUPPORT)
	gLogMessageRaw << "Creating ESTB CANNED SNMP module.." << endl;
	BcmEstbCannedSnmp *pEstbCannedSnmp = new BcmEstbCannedSnmp();
	#endif

	#if (defined (BCM_DUAL_DOWNLOAD_SUPPORT))// || defined (COMMON_DOWNLOAD_SUPPORT) )
	gLogMessageRaw << "Creating DSG Common Download TFTP Server Thread..." << endl;
	BcmDsgTftpThread *pfDsgTftpServerThread = new BcmDsgTftpThread(BcmOperatingSystem::kNormalPriority);
	#endif
 
	#if (defined (BCM_DUAL_DOWNLOAD_SUPPORT) || defined (ESTB_RSWDLOAD_SUPPORT) )
	gLogMessageRaw << "Creating DSG Download Control Thread..." << endl;
	BcmDsgDownloadCtlThread *pDsgDownloadCtlThread = new BcmDsgDownloadCtlThread(BcmOperatingSystem::kNormalPriority);
	// This should be enabled only for CDL LIB testing purposes.
	#if defined(BCM_CDL_LIB_SUPPORT)
		BcmCommonDownloadHelperThread *pCdlHelperThread = new BcmCommonDownloadHelperThread (BcmOperatingSystem::kNormalPriority);
	#endif
	#endif

	gLogMessageRaw << "Creating DSG Client Controller Thread..." << endl;
	BcmDsgClientCtlThread *pDsgClientCtlThread = new BcmDsgClientCtlThread(BcmOperatingSystem::kNormalPriority);

    // Store the DSG Client Ctl thread object with the application object.
    pgCmApplication->SetDsgClientCtlThread(pDsgClientCtlThread);

/*
    BcmCmDocsisCtlThread *pCmDocsisCtlThread = NULL;
    BcmCmDocsisNonVolSettings *pCmDocsisSettings = BcmCmDocsisNonVolSettings::GetSingletonInstance();

   
    // If both the CM interface and the CM's IP stack were created, then start
    // and configure the other DOCSIS-related components.
    if ((pCmPublicStack != NULL) && (pCmHalIf != NULL))
    {
        gLogMessageRaw << "Creating DOCSIS Control Thread..." << endl;

        // Create the DOCSIS Protocol Control object.  He needs to run above
        // normal priority, since handling DOCSIS state/timers/messages is
        // critical to the operation of the modem.
        pCmDocsisCtlThread = new BcmCmDocsisCtlThread(BcmOperatingSystem::kHighNormalPriority);

        // Store the DOCSIS Ctl thread object with the application object.
        pgCmApplication->SetDocsisCtlThread(pCmDocsisCtlThread);
    }

    // If console support is compiled in, then we need to get each interface
    // to create its command table, register with it, and add the command
    // table to the main command table.
    #if (BCM_CONSOLE_SUPPORT)
    {
        BcmHalIf *pHalIf;

        pHalIf = pHalIfFactory->First();
        while (pHalIf != NULL)
        {
            // If this interface has a command table, add it as a subtable
            // of the main command table.
            if (pHalIf->CommandTable() != NULL)
            {
                BcmCommandTable *pDummy = NULL;

                // Because multiple interfaces might use the same command table
                // (for example, all IP stacks use the IpHalCommandTable), we
                // need to avoid adding the same table more than once.
                if (pgMainCommandTable->IsValidSubtable(pHalIf->CommandTable()->Name(),
                                                        pDummy) == false)
                {
                    pgMainCommandTable->AddSubtable(pHalIf->CommandTable());
                }
            }

            pHalIf = pHalIfFactory->Next(pHalIf);
        }
    }
    #endif
*/        

#if (SNMP_SUPPORT)
    // For SNMP operation in the V2 environment, we need a single
    // forwarder object that has an IpHalIf
    // registered with it as the default interface.
    //
    gLogMessageRaw << "Creating DSGCC forwarder..." << endl;

    BcmForwarder *pDsgccForwarder = new BcmForwarder(0, "DSGCC Forwarder");

    gLogMessageRaw << "Creating IP HAL for DSGCC..." << endl;

    // There is just one IP stack here, so we'll call it stack 1
    // Create the appropriate (for this OS) object

    BcmBaseIpHalIf *pIpHalIf;
    unsigned int ipStackNumber = 1;

#if defined(TARGETOS_vxWorks)

    pIpHalIf = new BcmVxIpHalIf(pSettings->MacAddressIp(ipStackNumber), "DSGCC IP Stack 1");

#elif defined(TARGETOS_stblinux)

    gLogMessageRaw << "MOE... Creating Linux IP HAL for DSGCC..." << endl;
    pIpHalIf = (BcmBaseIpHalIf *) new BcmStubHalIf (420, BcmHalIf::kIanaTypeSoftwareLoopback, "DSGCC IP Stack 1", 0, 1500);
    // pIpHalIf = new BcmLnxIpHalIf(pSettings->MacAddressIp(ipStackNumber), "DSGCC IP Stack 1");

#endif
    // ConfigureAndRegisterNiHalIf(pIpHalIf, gIpStackNames[ipStackNumber - 1], BcmHalIf::kIp0 + ipStackNumber - 1) 
    // Set the short/common name for this interface.
    pIpHalIf->ShortName(BcmHalIf::kIp0 + ipStackNumber - 1);

    // Start the driver for the interface.
    gLogMessageRaw << "\tRegistering " << "DSGCC IP Stack 1" << " driver" << endl;

// For now, don't start a driver
#ifdef NOTYET
    if (pIpHalIf->StartDriver() == false) {
        gLogMessageRaw << "\tERROR - failed to start " << "DSGCC IP Stack 1" << " driver" << endl;

        delete pIpHalIf;
        pIpHalIf = NULL;
    } else {
#endif
      // we're done with IP HAL, so we can register it with the forwarder.
      gLogMessageRaw << "Registering " << pIpHalIf->Description() << " with " << pDsgccForwarder->Name() << endl;

      pDsgccForwarder->Register(pIpHalIf);

      // Prime the Forwarder with an association between the IpHalIf
      // and it's MAC address.  This allows us to be addressable
      // immediately.
      if (pIpHalIf->IanaType() == BcmHalIf::kIanaTypeIpStack)
      {
    gLogMessageRaw << "MOE... Priming DSGCC Forwarder ..." << endl;
          pDsgccForwarder->Add(pIpHalIf->IfPhysAddress(0), pIpHalIf, true);
      }

      // No interface is allowed to send packets back to itself; this would
      // cause packet replication on the network. Disable loopback.
      pIpHalIf->NeverEnableTransmitTo(*pIpHalIf, true);

      // Store the forwarder object with the application object.
    gLogMessageRaw << "MOE... calling SetPacketSwitch..." << endl;
      pgCmApplication->SetPacketSwitch(pDsgccForwarder);


#ifdef NOTYET
    }
#endif

        // Instantiate, populate, and start the SNMP
        // agent.
        pgCmApplication->StartSnmp();

    #else

    // Instantiate the event log and init from non-vol
    // CmEventLog::Singleton().InitFromNonVol();

    #endif

#if (BCM_DSG_SUPPORT)
	// Add some delay(ms) to allow pretty console log print out of the event
	// log from non-vol.
	BcmOperatingSystemFactory::ThreadSleep(200);
#endif


/*    #if (BCM_SETTOP_SUPPORT) */
    
    /**********************************************************
               Settop/Cm Control Bridge Initialization 
    **********************************************************/
/*    // Configure the system setting so that the Cable Modem within the Settop box does not
    // do a hard-reset but only stops and restarts the Docsis Control thread.
    pCmDocsisCtlThread->ShutdownExitsApp(false);

    #if (SETTOP_API_INCLUDED)          
    BcmDocsisControlBridge *pDocsisControlBridge = NULL;

    // Create the DocsisControlBridge object and establish the required links with the other objects.
    pDocsisControlBridge = new BcmDocsisControlBridge((ClientNotifier)BcmDocsisCallBackFunc); //, pCmDocsisCtlThread);    

    #if (BCM_CONSOLE_SUPPORT)
    //register above pointer as the instance to the command table
    BcmDocsisCtrlBridgeCommandTable *pDocCtlBrCmdTab = BcmDocsisCtrlBridgeCommandTable::GetCommandTableSingleton();

    pDocCtlBrCmdTab->AddInstance("BcmCtrlBridgeCmdTable", pDocsisControlBridge);
	#endif // BCM_CONSOLE_SUPPORT
*/
    pgCmApplication->Run();
}
 






