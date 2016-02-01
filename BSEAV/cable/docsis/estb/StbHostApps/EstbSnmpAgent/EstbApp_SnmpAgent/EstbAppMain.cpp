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
//  Filename:       EstbAppMain.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  12 MAY 06
//
//****************************************************************************
//  Description:
//      This file contains eSTB SNMP Agent standalone application startup code.
//
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "EstbAppMain.h"

// Target-specific initialization code for V2.
#include "EstbTargetMain.h"

// Core V2 startup code.
#include "V2Main.h"


// The portable application and other components.
#include "EstbApplication.h"
// #include "IpRoutingFilterSnoop.h"
#include "EstbSnmpAgent.h"
#include "EstbEventLog.h"
#include "BcmSnmpTrapHandler.h"

#if (SNMP_SUPPORT)
//    #include "SnmpAgnt.h"
    #include "SnmpApi.h"
    #include "OperatingSystemFactory.h"
#endif
    
#include "EstbAppCompositeNonVolSettings.h"
// #include "HalIfNonVolSettings.h"
#include "EstbAppNonVolSettings.h"
#include "MessageLogNonVolSettings.h"

#include "MainCommandTable.h"
#include "EstbAppCompositeNonVolCommandTable.h"
// #include "SnoopCommandTable.h"

#include "ConsoleThread.h"

// #include "Forwarder.h"
// #include "HalIfFactory.h"
// #include "HalIf.h"
// #include "BaseIpHalIf.h"
// #include "PacketSwitchBridgingHalIf.h"
// #include "IpStackManager.h"
// #include "MiniUsfsSnoop.h"
// #include "UsfsTable.h"

#if (BCM_VENDOR_SUPPORT) || (BCM_VENDOR_CMAPP_SUPPORT)
    #include "VendorCmAppFactory.h"
    #include "VendorCmApplicationBase.h"
#endif

#if (BCM_PING_SUPPORT)
    #include "PingThread.h"
#endif

#if (SETTOP_API_INCLUDED)
    #if (BCM_CONSOLE_SUPPORT)
	    #include "CmCtrlBridgeCommandTable.h"
	#endif
	#include "CmControlBridge.h"
#endif

#if (BCM_SETTOP_SUPPORT)
    #include "OperatingSystemFactory.h"
#endif

#ifdef NOTDEF
extern "C" 
{
    STATUS bcmHalMacAddrGet( char *pMacAddr, unsigned char stackIfNum );
}
#endif

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

#if (BCM_VENDOR_SUPPORT) || (BCM_VENDOR_CMAPP_SUPPORT)

static BcmVendorCmApplicationBase *pgVendorCmApplication = NULL;

#endif

static BcmEstbApplication *pgEstbApplication = NULL;

static bool gSkipDriverInit = false;  // PR1968

// PR4984 - Add new variable that separates the "brick" behavior from skipping
// driver init.
static bool gPermanentNonvolCorrupt = false;

static BcmMainCommandTable *pgMainCommandTable = NULL;

#if (BCM_PING_SUPPORT)

static BcmPingThread *pgPingHelper = NULL;

#endif

//********************** Local Functions *************************************

//********************** Function Implementations ****************************


void AppPreNonvolInit(void)
{
    unsigned int readResult;


    gLogMessageRaw << "In AppPreNonvolInit() \n" << endl;
    // Allow a vendor to create an application object, and initialize it.
    #if (BCM_VENDOR_SUPPORT) || (BCM_VENDOR_CMAPP_SUPPORT)

    pgVendorCmApplication = BcmVendorCmAppFactory::NewVendorCmApplication();

    gLogMessageRaw << "\tNewVendorCmApplication() done\n" << endl;
    if (pgVendorCmApplication != NULL)
    {
        pgVendorCmApplication->Initialize();
        gLogMessageRaw << "\tpgVendorCmApplication->Initialize() done\n" << endl;
    }

    #endif

    // Create the non-vol settings singleton instances and read the data in
    // from the device.  Afterwards, create the command tables associated with
    // the settings (if compiled in).  The settings must exist before creating
    // the command tables, because the tables access the singleton settings
    // instances to automatically register them.
    BcmEstbAppCompositeNonVolSettings *pNonVolSettings = NULL;

    // Create the root composite object, which creates the singleton objects
    // for all of the subsystems.
    pNonVolSettings = new BcmEstbAppCompositeNonVolSettings;

    // Let the vendor create and register nonvol settings objects.
    #if (BCM_VENDOR_SUPPORT)

    BcmVendorCmAppFactory::CreateAndRegisterNonVolSettings(*pNonVolSettings);
    gLogMessageRaw << "\tCreateAndRegisterNonVolSettings() done\n" << endl;

    #endif

    gLogMessageRaw << "\tV2MainReadNonVol()\n" << endl;
    // Call the V2 helper function to read and validate the settings.
    readResult = V2MainReadNonVol(pNonVolSettings);

    gLogMessageRaw << "\tV2MainReadNonVol() done \n" << endl;

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
}


void AppPostNonvolInit(void)
{
    // Get the main command table.
    pgMainCommandTable = BcmMainCommandTable::GetCommandTableSingleton();

    // Create the portable application object and register it with the main
    // command table.
    pgEstbApplication = new BcmEstbApplication;

    pgMainCommandTable->AddInstance("BcmApplication", pgEstbApplication);

    // Get the V2 core to add its command tables and commands.
    V2CmdTableInit(pgMainCommandTable, 4000);


#if (BCM_CONSOLE_SUPPORT)
    
    // Create the non-vol command table object.
    BcmEstbAppCompositeNonVolCommandTable *pEstbAppSettingsCommandTable = NULL;

    // Create the default command table if NULL
    pEstbAppSettingsCommandTable = new BcmEstbAppCompositeNonVolCommandTable;

    // Add the non-vol settings command table as a subtable under the main
    // command table.
    pgMainCommandTable->AddSubtable(pEstbAppSettingsCommandTable);

    // Add the event log command table as a subtable under my command table.
    pgMainCommandTable->AddSubtable(&EventLog::GetCommandTableSingleton());

    // Add the SNMP command table as a subtable under my command table.
    pgMainCommandTable->AddSubtable(&BcmSnmpAgent::GetCommandTableSingleton());

    // Add the Trap Handler command table as a subtable under my command table.
    pgMainCommandTable->AddSubtable(&BcmSnmpTrapHandler::GetCommandTableSingleton());


    // Add vendor specific nonvol command table(s), if we aren't slim.
    #if (BCM_VENDOR_SUPPORT)

    BcmVendorCmAppFactory::CreateAndRegisterNonVolCommandTables(*pEstbAppSettingsCommandTable);

    #endif
    
#endif // BCM_CONSOLE_SUPPORT


    // Add vendor specific command table(s).  We'll allow the user to decide
    // whether or not to include these command tables for slim builds.
    #if (BCM_VENDOR_SUPPORT)

    BcmVendorCmAppFactory::CreateAndRegisterCommandTables(*pgMainCommandTable);

    #endif


    #if (BCM_PING_SUPPORT)

    // Create the Ping Helper object, and give a pointer to the CM App object.
    pgPingHelper = new BcmPingThread("Ping Thread", BcmOperatingSystem::kIdlePriority);

    pgEstbApplication->PingHelper(pgPingHelper);
    

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

    BcmEstbAppNonVolSettings *pEstbAppNonVol = BcmEstbAppNonVolSettings::GetSingletonInstance();

    // PR4984 - we no longer short-circuit driver init if permanent nonvol is
    // corrupted.
    if ((pConsoleThread != NULL) &&
        ((pEstbAppNonVol == NULL) || (pEstbAppNonVol->DriverInitPrompt() == true)))
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

    #if (BCM_VENDOR_SUPPORT) || (BCM_VENDOR_CMAPP_SUPPORT)

    if (pgVendorCmApplication != NULL)
    {
        pgVendorCmApplication->PreDriverInitialization();
    }

    #endif
    
}


void AppRun(void)
{
    gLogMessageRaw << "AppRun()\n" << endl;
    // PR1968 - reorganized this a bit to allow stopping early due to nonvol
    // failure or console override.  This is what "bricks" the app.
    if (gSkipDriverInit)
    {
        gLogMessageRaw << "\n\nNOTE:  Driver initialization was skipped!  The application will not run or foward data.  You can change NonVol Setting, but that's about it...\n" << endl;

        pgEstbApplication->Run();
        
        return;
    }


    // Set the Snmp Agent thread priority to "kNormal"
    gLogMessageRaw << "Installing Snmp Agent thread." << endl;
    AgentInitSnmp(BcmOperatingSystemFactory::ConvertThreadPriority(BcmOperatingSystem::kNormalPriority));

    gLogMessageRaw << "Instantiating Estb Snmp Agent." << endl;

    EstbSnmpAgent::Singleton();

    pgEstbApplication->Run();
}
 






