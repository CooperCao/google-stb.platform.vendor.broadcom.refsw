//****************************************************************************
//
// Copyright (c) 2003-2012 Broadcom Corporation
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
//  $Id$
//
//  Filename:       MainCommandTable.cpp
//  Author:         David Pullen
//  Creation Date:  Feb 18, 2000
//
//****************************************************************************
//  Description:
//      This derived class handles set-up of the main Cable Modem application
//      commands in a command table.  This is used by the CmApplication class.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "MainCommandTable.h"

#include "CmApplication.h"
#include "CountingSemaphore.h"
#include "PingThread.h"
#include "ConsoleThread.h"

#include "UnsignedIntCommandParameter.h"
#include "IpAddressCommandParameter.h"
#include "FlagCommandParameter.h"

#if (SNMP_SUPPORT)
#include "CmSnmpAgent.h"
#endif

#include <iostream>
#include <iomanip>

#include <ctype.h>
#ifdef TARGETOS_stblinux
#include "OperatingSystemFactory.h"
#include <sys/reboot.h>
#endif

#include "EstbRemoteInterfaceStub.h"
// #if defined(TARGETOS_stblinux) && defined(BCM97110)
// #include "bcmtypes.h"
// #include "CmHalMacros7110LE.h"
// extern "C" {
//   ULONG user_7110reg_base;
// }
// #endif

//********************** Local Types *****************************************

//********************** Local Constants *************************************

static const char *pgTableDescr = "Basic DSG Application commands.";

// PR705 - parameter constants for the read/write memory commands.
enum
{
    kNoParameter = 0,
    kIpAddressParameter,
    kStartAddress,
    kNumberOfBytes,
    kContinueFlag,
    kElementSize,
    kValueToWrite,
    kShowSettingsFlag,
    kPrimaryParameter
};

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Default Constructor.  Creates the command table, adds all of the
// commands to it and sets the command handler function.
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmMainCommandTable::BcmMainCommandTable(void) :
    BcmCommandTable("DSG App Commands", "CM-App", "DSG", pgTableDescr)
{
    BcmCommand command;

    // Set up fields that don't change.
    command.fCommandFunctionWithParms = CommandHandler;
    command.fIsHidden = false;

    // These commands are available for all targets.
    command.fIsEnabled = true;
    command.fCommandId = kRunApplication;
    command.pfCommandName = "run_app";
    command.pfCommandHelp = "If the application was stopped at the console (either via keypress or via "
                            "non-vol setting that automatically stopped it), then this command will "
                            "allow it to start running.  This command is not available if the application "
                            "is already running.";
    command.pfExamples = "run_app\t";
    AddCommand(command);

    command.fIsEnabled = true;
    command.fCommandId = kResetCommand;
    command.pfCommandName = "reset";
    command.pfCommandHelp = "Resets the system (warm boot).  The hardware reset line is triggered, "
                            "causing the application to be reloaded from scratch.  On host-based app "
                            "simulators, this will cause the application to exit.";
    command.pfExamples = "reset\t";
    AddCommand(command);

    command.fCommandId = kShowVersion;
    command.pfCommandName = "version";
    command.pfCommandHelp = "Displays the current software version and feature codes by printing "
                            "the startup banner.  This allows the user to view the current version "
                            "without having to reboot the modem.";
    command.pfExamples = "version\t";
    AddCommand(command);


#if ( BCM_PING_CONSOLE_SUPPORT )
    // Assume that ping is not provided.  It will be enabled if a ping instance
    // is created.
    command.fIsEnabled = false;
    command.fCommandId = kPingCommand;
    command.pfCommandName = "ping";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmIpAddressCommandParameter(kIpAddressParameter));
    command.pfCommandHelp = "Pings the specified target IP address, sending 3 64-byte packets, and waiting "
                            "up to 5 seconds for a response.  This is a basic 'standard' ping.  For more "
                            "options or control over ping parameters and behavior, you will need to go to "
                            "the Ping Command table ('cd pingHelper').\n\n"
                            "In order for this to work, the CM must either have successfully completed "
                            "DHCP, or must otherwise have been configured with a valid IP address.\n\n"
                            "Note that this command causes the ping options to be reset to their default "
                            "state.\n\n"
                            "This may be disabled if the platform doesn't provide an implementation of ping.";
    command.pfExamples =	"ping 11.24.4.3\tPing IP address 11.24.4.3.";
    AddCommand(command);
#endif

	command.fCommandId = kSetApiDebugLevel;
	command.pfCommandName = "api_debug_level";
	command.fIsHidden = true;
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, true, "API debug level", 0x0, 0xFFFF));
	command.pfCommandHelp = "Sets eSTB socket API debug level.";
	command.pfExamples = "api_debug_level 0x0 (disabled)\n"
						 "api_debug_level 0x1 (level 1)\n"
						 "api_debug_level 0x2 (level 2)\n"
						 "api_debug_level 0x4 (level 3)\n"
						 "api_debug_level 0x8 (level 4)\n"
						 "kLevel0      = 0x00000000\n"             
						 "kLevel1      = 0x00000001\n"         
						 "kLevel2      = 0x00000002\n"              
						 "kLevel3      = 0x00000004\n"              
						 "kLevel4      = 0x00000008\n"
						 "kDsg	       = 0x00000010\n"
						 "kCdl	       = 0x00000020\n"
						 "kDavic       = 0x00000040\n"
						 "kDavicLevel1 = 0x00000080\n"
						 "kDavicLevel2 = 0x00000100\n"
						 "kDavicVendor = 0x00000200\n"		
						 "kSebLevel1   = 0x00000400\n"
						 "kSebLevel2   = 0x00000800\n"							 
						 ;
	AddCommand(command);

}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmMainCommandTable::~BcmMainCommandTable()
{
    // Nothing to do...
}


// Returns the pointer to the singleton instance for this class.  Most
// objects in the system will use this method rather than being passed a
// pointer to it.  The singleton pointer will be set up in the base-class
// constructor.
//
// NOTES:  This can return NULL if a singleton has not been set up for the
//         system, so you must check for this condition.
//
//         You must not delete this object!
//
//         You should not store a pointer to the object that is returned,
//         since it may be deleted and replaced with a new one.
//
// Parameters:  None.
//
// Returns:
//      A pointer to the instance that should be used by the system.
//
BcmMainCommandTable *BcmMainCommandTable::GetCommandTableSingleton(void)
{
    static BcmMainCommandTable gSingletonInstance;
    
    return &gSingletonInstance;
}


// This is the entrypoint that is called whenever one of my commands is
// entered.
//
// Parameters:
//      pInstanceValue - the instance value to which the command should be
//                       applied.  Instances must be registered with the
//                       command table in order for them to receive the
//                       command.
//      command - the command (with parameters) that was entered by the user.
//
// Returns:  Nothing.
//
void BcmMainCommandTable::CommandHandler(void *pInstanceValue,
                                         const BcmCommand &command)
{
    // By convention, the application object registers its instance so that the
    // user value is the 'this' pointer.  This cast should be safe.
    BcmCmApplication *pInstance = (BcmCmApplication *) pInstanceValue;

    // Validate this just for sanity.
    if (pInstance == NULL)
    {
        gLogMessageRaw << "BcmMainCommandTable::CommandHandler:  ERROR - User value (pointer to CM App\n"
                          "instance) is NULL!  Can't process the command!\n";

        return;
    }

    const BcmCommandParameter *pParameter = NULL;

    // Many commands below have a single primary parameter.  Retrieve it for
    // them.
    if (command.pfCommandParameterList != NULL)
    {
        pParameter = command.pfCommandParameterList->Find(kPrimaryParameter);
    }

    switch (command.fCommandId)
    {
        case kResetCommand:
        #if ((SNMP_SUPPORT) && (CM_HAL_INCLUDED))
        // Tell the SNMP agent we're about to go away.
        CmSnmpAgent::Singleton().InformShutdownPending();
        #endif
        #ifdef TARGETOS_stblinux
			// We only want to allow the reboot to occur with the
			// console command.  STB does not support reboot at
			// any other time.
		
			// Allow time for the inform ShutdownPending to occur.
			BcmOperatingSystemFactory::ThreadSleep(1000);
		    reboot(RB_AUTOBOOT);
		#if KB_TODO // #else
            pInstance->ExitNow();
		#endif
		#endif
            break;

        case kRunApplication:
            // Tell the app to start running.
            //#if (CM_HAL_INCLUDED)
            pInstance->StartDsgCC();
            //#endif
            break;

        #if ( BCM_PING_CONSOLE_SUPPORT )
        case kPingCommand:

            if (pInstance->PingHelper() == NULL)
            {
                gLogMessageRaw << "ERROR - unable to ping; no ping implementation provided for the target!\n";
            }
            else
            {
                bool result;
                BcmConsoleThread *pConsoleThread;
                const BcmCommandParameter *pIpAddressParm = command.pfCommandParameterList->Find(kIpAddressParameter);

                // Get the console thread singleton.  This may be used by handlers below to
                // parse parameters and/or get user input.
                pConsoleThread = BcmConsoleThread::GetSingletonInstance();

                if (pConsoleThread == NULL)
                {
                    gLogMessageRaw << "BcmMainCommandTable::CommandHandler:  ERROR - Console thread singleton is NULL!\n";

                    break;
                }

                // Reset the default options.
                pInstance->PingHelper()->ResetDefaultOptions();

                // Set the IP address to ping.
                pInstance->PingHelper()->SetIpAddress(pIpAddressParm->AsIpAddress());

                result = pInstance->PingHelper()->Start();

                if (!result)
                {
                    gLogMessageRaw << "ERROR - unable to start pinging the target!\n";
                }
                else
                {
                    // Poll the console thread for a keypress, and also
                    // poll the ping thread for it stopping on its own.
                    while (pInstance->PingHelper()->PingingInProgress() == true)
                    {
                        if (pConsoleThread->WaitForKeypress(250) == true)
                        {
                            gLogMessageRaw << "\nStopping pings due to keypress..." << endl;

                            pInstance->PingHelper()->Stop();

                            break;
                        }
                    }
                }
            }
            break;
        #endif

    case kShowVersion:
        //#if (CM_HAL_INCLUDED)
            pInstance->PrintVersion();
        //#endif
            break;

    case kSetApiDebugLevel:
		{
			if (pParameter->WasParsed())
			{
				uint32 mode = pParameter->AsUnsignedInt();
				SetDisplayMode((DisplayMode)mode);
			}
		}
        break;

        default:
            gLogMessageRaw << "WARNING - unknown command id (" << command.fCommandId << ")!  Ignoring...\n";
            break;
    }
}


