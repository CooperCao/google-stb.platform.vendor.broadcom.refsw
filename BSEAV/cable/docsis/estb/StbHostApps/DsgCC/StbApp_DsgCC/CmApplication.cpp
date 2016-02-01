//****************************************************************************
//
//  Copyright (c) 2000, 2001  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//
//  Filename:       CmApplication.cpp
//  Author:         David Pullen
//  Creation Date:  Feb 21, 2000
//
//****************************************************************************
//  Description:
//      This is the portable Cable Modem application main.  This class is
//      created by the target-specific main, configured with various platform-
//      specific devices and objects, and is then called to create and start
//      the portable, application-specific objects.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "CmApplication.h"

// Other objects of interest.
#include "ConsoleThread.h"
#if (SNMP_SUPPORT)
#include "Forwarder.h"

#include "IpStackManager.h"
#include "BaseIpHalIf.h"
#endif

#include "CountingSemaphore.h"
#include "OperatingSystemFactory.h"
#include "MsgLogSettingsCommandTable.h"

#include "CompanyLogo.h"

#if (SNMP_SUPPORT)
    #include "SnmpApi.h"
    #include "BcmSnmpAgent.h"
    // #include "ifBridge.h"

    #include "EstbSnmpAgent.h"
    //    #include "CmEventLog.h"

#endif

#include "CmAppVersionInfo.h"

#include "DsgClientCtlThread.h"

#if defined(BCM_NONVOL_SUPPORT)
#include "CmAppNonVolSettings.h"
#endif
//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Default Constructor.  Initializes the state of the object...
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmCmApplication::BcmCmApplication(void) :
	pfDsgClientCtlThread(NULL),
    // By default, run the console thread when we start.
    fStartConsole(true),
    pfAppSemaphore(0)
{
    // Override the class name given by my parent.
    fMessageLogSettings.SetModuleName("BcmCmApplication");

    pfAppSemaphore = BcmOperatingSystemFactory::NewCountingSemaphore(true, 1, 0, "App Running Semaphore");
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmCmApplication::~BcmCmApplication()
{
    delete pfAppSemaphore;
    pfAppSemaphore = NULL;
}


// The target main will call this to start the portable application running.
//
// Note that in a typical embedded system, this will generally never return,
// since the application is supposed to run forever.  In host simulators,
// this will exit typically when the user types 'quit'.
//
// The target main can choose to handle returns from this method however it
// wants (it can exit the app, call Run again, tear objects down and restart,
// reset the hardware, etc.).
//
// Parameters:  None.
//
// Returns:
//      true if the application ran and exited successfully.
//      false if the application exited abnormally.
//
bool BcmCmApplication::Run(void)
{
    BcmConsoleThread *pConsoleThread;
    bool stopAtConsole = false;

	#if KB_TODO
    // Register my semaphore so that I get notified when the thread exits.
    if (pfDsgClientCtlThread != NULL)
    {
        pfDsgClientCtlThread->RegisterExitSemaphore(pfAppSemaphore);
    }
	#endif

    // Print the standard company logo, copyright, build date/time, and
    // revision string.
    PrintVersion();

	PrintStbBanner();
    gAlwaysMsgNoFields(fMessageLogSettings) 
        << "Broadcom DSG 1.0 Client Controller Application starting...\n" 
		<< endl;

#ifndef MAC_STUB
#if (BCM_CONSOLE_SUPPORT)
    // If the console thread is not NULL, then see if the user wants to modify
    // run-time settings before kicking the rest of the app off.
    pConsoleThread = BcmConsoleThread::GetSingletonInstance();
    if (pConsoleThread != NULL)
    {
		#if defined(BCM_NONVOL_SUPPORT)
        BcmCmAppNonVolSettings *pNonVolSettings;

        // If the non-vol settings say to break out to the console automatically,
        // then do so.  Otherwise, wait for a keypress, and if one is received,
        // then break out to the console.
        pNonVolSettings = BcmCmAppNonVolSettings::GetSingletonInstance();
        if ((pNonVolSettings != NULL) && 
            (pNonVolSettings->AutoStopAtConsole() == true))
		#endif
        {
            gAlwaysMsgNoFields(fMessageLogSettings) 
                << "Automatically stopping at console because non-vol settings told me to...\n" << endl;

            stopAtConsole = true;
        }
		#if  defined(BCM_NONVOL_SUPPORT)
        else if ((pNonVolSettings != NULL) &&
                 (pNonVolSettings->StopAtConsolePrompt() == true))
		#endif
        {
            gAlwaysMsgNoFields(fMessageLogSettings) 
                << "Press any key in 2 seconds to stop at console..." << flush;

            stopAtConsole = pConsoleThread->WaitForKeypress(2000);

            gAlwaysMsgNoFields(fMessageLogSettings) << "\n" << endl;
        }

        if (fStartConsole)
        {
			cout << " Start the console thread running. " << endl;
            // Start the console thread running.
            pConsoleThread->Resume();
        }
    }
#endif
#endif // endif-ndef MAC_STUB

    // The user may have typed quit or reset at the console, so I need to abort
    // starting the app in this case.
	#if KB_TODO
    if (fTimeToQuit == false)
	#endif
    {
        if ((fStartConsole == false) || (stopAtConsole == false))
        {
			#if !defined(UPGRADE_ECM_AFTER_REBOOT)
			// Do not do anything. Just wait for clients to register.
			// Actually, at this point DSG-CC should start if it has any clients.
            StartDsgCC();
	
			#endif
        }
        else
        {
            gAlwaysMsgNoFields(fMessageLogSettings) 
                << "NOTE: DSG-CC will not configure eCM to scan downstream unless at least one client registers." << endl
                << "      Use DSG Test Client application to start client applications." << endl
				<< "      OR use console command < add_client > to register fake clients with DSG-CC." << endl;
        }

       // Now wait to be told to exit.
		pfAppSemaphore->Get();
    }

    return true;
}

// Starts up DOCSIS operation.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmCmApplication::StartDsgCC(void)
{
    gAlwaysMsgNoFields(fMessageLogSettings) 
        << "\nBeginning DSG-CC operation...\n" << endl;

    // Set up other objects, and start things running.
    if (pfDsgClientCtlThread == NULL)
    {
        gWarningMsgNoFields(fMessageLogSettings) 
            << "WARNING - No DSG Control thread provided.  DSG state machine can't run!\n" << endl;

        return;
    }

	pfDsgClientCtlThread->Start();

}

// Prints the company logo and version information.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmCmApplication::PrintVersion(void)
{
    BcmCompanyLogo logo;

    // Set the build date/time and revision and feature strings.
    gGetCmAppVersionInfo(logo);

    // Print the strings out.
#if (BCM_CONSOLE_SUPPORT)
    gAlwaysMsgNoFields(fMessageLogSettings) << logo ;
#else    
    cout << logo ;
#endif

	
}

#if (SNMP_SUPPORT)

// Instantiate, register, populate and start the SNMP agent(s).
//
// Parameters:  None
//
// Returns:  Nothing.
//
// One of these days I should move this from here and put it more in the
// SNMP domain.  Perhaps have an SNMP control object which maintains a list of
// SNMP agents and does this more generically.  One of these days...
//
void BcmCmApplication::StartSnmp(void)
{
    BcmHalIf *pHal;

    int DsgccIf=1;

  printf("MOE... BcmCmApplication::StartSnmp ()\n");


    gLogMessageRaw << "Starting SNMP subsystem" << endl;

    // If we got no forwarder, we can't really set up SNMP...
    if (pfPacketSwitch)
    {
  printf("MOE... BcmCmApplication::StartSnmp has pfPacketSwitch\n");
        pHal = pfPacketSwitch->FindByIfNum (DsgccIf);
        if (pHal)
        {
  printf("MOE... BcmCmApplication::StartSnmp has pHal\n");
#ifdef TARGETOS_stblinux
        gLogMessageRaw << "Instantiating CmSnmpAgent object." << endl;

        // Set the priority to "kNormal"
        AgentInitSnmp(BcmOperatingSystemFactory::ConvertThreadPriority(BcmOperatingSystem::kNormalPriority));

        EstbSnmpAgent::Singleton();

        gLogMessageRaw << "Starting STB NETSNMP subsystem" << endl;

  printf("MOE... BcmCmApplication::StartSnmp calling EstbSnmpAgent::Singleton().Setup ()\n");
        EstbSnmpAgent::Singleton().Setup(pHal->IfPhysAddress(0), pHal->InterfaceNumber(),
                                       pfPacketSwitch, pfDsgClientCtlThread, this);

        // Start the agent.
        EstbSnmpAgent::Singleton().Start();

#ifdef NOTYET
        // Let the docsis ctl thread register its SNMP objects.  Note that IGMP,
        // BPI, and perhaps some other objects will get registered in this call
        // as well.
        pfCmDocsisCtlThread->RegisterSnmpObjects();

        // Instantiate the event log and init from non-vol
        CmEventLog::Singleton().InitFromNonVol();
#endif

        gLogMessageRaw << "SNMP startup complete." << endl;
    }
#else
          EstbSnmpAgent::Singleton().Setup(pHal->IfPhysAddress(0), pHal->InterfaceNumber(), pfPacketSwitch, pfDsgClientCtlThread, this);

          // Let the docsis ctl thread register its objects (if we have a docsis ctl
          // thread).  Note that IGMP, BPI, and perhaps some other objects will get
          // registered in this call as well.
          if (pfCmDocsisCtlThread)
            pfCmDocsisCtlThread->RegisterSnmpObjects();

          // Instantiate the event log and init from non-vol
          CmEventLog::Singleton().InitFromNonVol();

          // Start the agent.
          CmSnmpAgent::Singleton().Start();
        }

#endif

// If we have no CM create a stub SNMP agent to
// operate on the CPE IP address.
#if (0)
        pHal = pfPacketSwitch->FindByIfNum (CpeIf);
        if (pHal)
        {
          BcmSnmpAgent *pStubAgent = new BcmSnmpAgent ("Stub Agent");
          pStubAgent->Setup(pHal->IfPhysAddress(0), pHal->InterfaceNumber(), pfPacketSwitch);
          pStubAgent->Start();

          // Install a view for the stub agent which gives access to everything.
          unsigned long subtree_entireMib[] = {1,3,6,1};
          AgentConfigView ("stubView", subtree_entireMib, sizeof(subtree_entireMib)/sizeof(unsigned long), VIEW_STATUS_INCLUDE);
          pStubAgent->SetV1V2View ("stubView");
        }
#endif
    }

    gLogMessageRaw << "SNMP startup complete." << endl;
}
#endif








