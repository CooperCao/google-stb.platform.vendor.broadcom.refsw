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
//
//  Filename:       EstbApplication.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  12 MAY 06
//
//****************************************************************************
//  Description:
//      This is the portable eSTB SNMP Agent application main.  This class is
//      created by the target-specific main, configured with various platform-
//      specific devices and objects, and is then called to create and start
//      the portable, application-specific objects.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "EstbApplication.h"

// Other objects of interest.
#include "ConsoleThread.h"
#include "NonVolDevice.h"
// #include "IpHelperFactory.h"
// #include "Forwarder.h"

// #include "IpStackManager.h"
// #include "BaseIpHalIf.h"
// #include "DhcpClientLease.h"
// #include "DhcpClientIf.h"
// #include "DhcpClientId.h"

#include "EstbAppNonVolSettings.h"
#include "MessageLogNonVolSettings.h"

#include "CountingSemaphore.h"
#include "OperatingSystemFactory.h"

#include "MsgLogSettingsCommandTable.h"

#include "CompanyLogo.h"

#if BCM_TELNET_SUPPORT
    #include "TelnetThread.h"
#endif

#if (SNMP_SUPPORT)
    #include "SnmpApi.h"
    #include "BcmSnmpAgent.h"
    #include "ifBridge.h"
//    #include "dot1dBridge.h"
    #include "EstbSnmpAgent.h"
    #include "EstbEventLog.h"
#ifdef SNMP_PROXY_SUPPORT
    #include "Proxy.h"
#endif
#endif

#include "EstbAppVersionInfo.h"

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
BcmEstbApplication::BcmEstbApplication(void) :

    pfPacketSwitch(NULL),

    #if (BCM_PING_SUPPORT)

    pfPingHelper(NULL),

    #endif

    // By default, run the console thread when we start.
    fStartConsole(true)
{
    // Override the class name given by my parent.
    fMessageLogSettings.SetModuleName("BcmEstbApplication");

    pfAppSemaphore = BcmOperatingSystemFactory::NewCountingSemaphore(true, 1, 0, "App Running Semaphore");

}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmEstbApplication::~BcmEstbApplication()
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
bool BcmEstbApplication::Run(void)
{
    BcmConsoleThread *pConsoleThread;
    bool stopAtConsole = false;

    // Print the standard company logo, copyright, build date/time, and
    // revision string.
    PrintVersion();

    gAlwaysMsgNoFields(fMessageLogSettings) 
        << "Broadcom eSTB Application starting...\n" << endl;

#ifndef MAC_STUB

    // If the console thread is not NULL, then see if the user wants to modify
    // run-time settings before kicking the rest of the app off.
    pConsoleThread = BcmConsoleThread::GetSingletonInstance();
    if (pConsoleThread != NULL)
    {
        BcmEstbAppNonVolSettings *pNonVolSettings;

        // If the non-vol settings say to break out to the console automatically,
        // then do so.  Otherwise, wait for a keypress, and if one is received,
        // then break out to the console.
        pNonVolSettings = BcmEstbAppNonVolSettings::GetSingletonInstance();
        if ((pNonVolSettings != NULL) && 
            (pNonVolSettings->AutoStopAtConsole() == true))
        {
            gAlwaysMsgNoFields(fMessageLogSettings) 
                << "Automatically stopping at console because non-vol settings told me to...\n" << endl;

            stopAtConsole = true;
        }
        else if ((pNonVolSettings != NULL) &&
                 (pNonVolSettings->StopAtConsolePrompt() == true))
        {
            gAlwaysMsgNoFields(fMessageLogSettings) 
                << "Press any key in 2 seconds to stop at console..." << flush;

            stopAtConsole = pConsoleThread->WaitForKeypress(2000);

            gAlwaysMsgNoFields(fMessageLogSettings) << "\n" << endl;
        }
        if (fStartConsole)
        {
            // Start the console thread running.
            pConsoleThread->Resume();
        }
    }

#endif // endif-ndef MAC_STUB

    // The user may have typed quit or reset at the console, so I need to abort
    // starting the app in this case.
    while (1) {
    // if (fTimeToQuit == false) {
      if ((fStartConsole == false) || (stopAtConsole == false)) {
        // Here is where we actually start the SNMP agent.
        StartSnmp();
      } else {
        gAlwaysMsgNoFields(fMessageLogSettings) 
           << "NOTE:  The application is not yet running!  You will need to type 'run_app' to\n"
           << "       start it..." << endl;
      }

      // Now wait to be told to exit.
      pfAppSemaphore->Get();
    }

	cout << "EXITING..." << endl;
    return true;
}



// Prints the company logo and version information.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmEstbApplication::PrintVersion(void)
{
    BcmCompanyLogo logo;

    // Set the build date/time and revision and feature strings.
    gGetEstbAppVersionInfo(logo);

    // Print the strings out.
#if (BCM_CONSOLE_SUPPORT)
    gAlwaysMsgNoFields(fMessageLogSettings) << logo;
#else    
    cout << logo;
#endif

}


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
void BcmEstbApplication::StartSnmp(void)
{
  gLogMessageRaw << "Setting up EstbSnmpAgent." << endl;

  gLogMessageRaw << "EstbSnmpAgent::Singleton().Setup()\n" << endl;

  BcmMacAddress MyNullAddress;

  MyNullAddress = kAllZerosMac;

  EstbSnmpAgent::Singleton().Setup(MyNullAddress, (unsigned int) 1,
                                       NULL, NULL);

  gLogMessageRaw << "EstbSnmpAgent::Singleton().Start()\n" << endl;
  // Start the agent.
  EstbSnmpAgent::Singleton().Start();

  // Let the agent start completely before we throw other things at it
  BcmOperatingSystemFactory::ThreadSleep(50);

#ifdef SNMP_PROXY_SUPPORT
  gLogMessageRaw << "Initialize Proxy Object." << endl;
  BcmProxy myProxy;
  myProxy.Initialize(&EstbSnmpAgent::Singleton());
#endif

  gLogMessageRaw << "EstbEventLog::Singleton().InitFromNonVol()\n" << endl;
  // Instantiate the event log and init from non-vol
  EstbEventLog::Singleton().InitFromNonVol();

  gLogMessageRaw << "In StartSnmp() - ESTB SNMP startup complete." << endl;
}




