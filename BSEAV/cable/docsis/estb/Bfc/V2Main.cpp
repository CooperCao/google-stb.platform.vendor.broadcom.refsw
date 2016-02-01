//****************************************************************************
//
// Copyright (c) 2002-2009 Broadcom Corporation
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
//  Filename:       V2Main.cpp
//  Author:         David Pullen
//  Creation Date:  Nov 27, 2002
//
//****************************************************************************
//  Description:
//      This file contains V2 startup code common across all V2 applications.
//
//      A bunch of this code was previously in the vxWorks/pSOS/Linux startup
//      code, and has been moved here since it isn't OS-specific, and we don't
//      like lots of duplicate code lying around.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "V2Main.h"

#include "NonVolDevice.h"
#include "NonVolSettings.h"

#include "CommandTable.h"

#include "MessageLog.h"
#include "MessageLogNonVolSettings.h"

#if (BCM_TELNET_SUPPORT)
    #include "TelnetThread.h"
#endif

#if (BCM_CONSOLE_SUPPORT)
    #include "MsgLogSettingsCommandTable.h"
#endif

#if (BCM_PING_CONSOLE_SUPPORT)
    #include "PingThread.h"
#endif

#if (BCM_HEAPMANAGER_SUPPORT)
    #include "HeapManagerCommandTable.h"
#endif

//********************** Local Types *****************************************

//********************** Local Constants *************************************

enum
{
    kLogout = 1,
};

//********************** Local Variables *************************************

static unsigned int gCommandIdBase = 0;

//********************** Local Functions *************************************

static bool ReadNonVolSection(BcmNonVolSettings::NonVolSection section,
                              const char *pSectionName,
                              BcmNonVolSettings *pSettings,
                              BcmNonVolDevice *pNonVolDevice);

static void CommandHandler(void *pInstanceValue, const BcmCommand &command);

//********************** Function Implementations ****************************


// This function handles reading and verifying nonvol settings.
//
// Parameters:
//      pSettings - pointer to the settings object (which is likely the root
//                  object of a composite) that is to be read.
//
// Returns:
//      One of the enum values above.
//
unsigned int V2MainReadNonVol(BcmNonVolSettings *pSettings)
{
    unsigned int result = kNonVolRead_NoProblem;

    // Get access to the nonvol device singleton.
    BcmNonVolDevice *pNonVolDevice = BcmNonVolDevice::GetSingletonInstance();

    if ((pSettings == NULL) || (pNonVolDevice == NULL))
    {
        return kNonVolRead_ErrorReadingBoth;
    }

    // Try to read permanent nonvol.
    //
    // PR5677 - do permanent before dynamic so that ordering dependencies don't
    // become an issue.
    if (ReadNonVolSection(BcmNonVolSettings::kPermanentSection,
                          "Permanent", pSettings, pNonVolDevice) == false)
    {
        result |= kNonVolRead_ErrorReadingPermanent;
    }

    // Try to read dynamic nonvol.
    if (ReadNonVolSection(BcmNonVolSettings::kDynamicSection,
                          "Dynamic", pSettings, pNonVolDevice) == false)
    {
        result |= kNonVolRead_ErrorReadingDynamic;
    }

    return result;
}


// This function installs the common V2 command table support with the
// specified parent command table.
//
// Parameters:
//      pParentCmdTable - the command table to be used as the parent.
//      commandIdBase - the base id that I should use for the commands that I
//                      add to the command table.
//
// Returns:  Nothing.
//
void V2CmdTableInit(BcmCommandTable *pParentCmdTable,
                    unsigned int commandIdBase)
{
    if (pParentCmdTable != NULL)
    {
        #if (BCM_CONSOLE_SUPPORT)
        
        // Add the message log settings command table as a subtable under the main
        // command table.
        pParentCmdTable->AddSubtable(&(BcmMessageLogSettings::GetCommandTableSingleton()));

        #if (BCM_PING_CONSOLE_SUPPORT)

        // Add the Ping helper command table as a subtable under my command table.
        pParentCmdTable->AddSubtable(&(BcmPingThread::GetCommandTableSingleton()));

        #endif // BCM_PING_CONSOLE_SUPPORT


        #if (BCM_HEAPMANAGER_SUPPORT)
        
        static BcmHeapManagerCommandTable gHeapManagerCmdTable;

        // Add the HeapManager command table as a subtable under my command table.
        pParentCmdTable->AddSubtable(&gHeapManagerCmdTable);

        #endif

        // Store this for future use; it is the offset for all of my commands,
        // which allows me to avoid conflicts with other modules that are adding
        // commands to this command table.
        gCommandIdBase = commandIdBase;

        BcmCommand command;

        // Set up fields that don't change. 
        command.fCommandFunctionWithParms = CommandHandler;
        command.fIsEnabled = true;
        command.fIsHidden = false;

        #if (BCM_TELNET_SUPPORT)

        command.fCommandId = kLogout + gCommandIdBase;
        command.pfCommandName = "logout";
        command.pfCommandHelp = "For Telnet clients, this lets the user log out cleanly.";
        command.pfExamples = "logout\t";
        pParentCmdTable->AddCommand(command);

        #endif
        #endif // BCM_CONSOLE_SUPPORT
    }
}


// Helper function that reads the specified section of nonvol settings.
//
// Parameters:
//      section - the section to read.
//      pSectionName - the name of the section (for display).
//      pSettings - the settings object to read into.
//      pNonVolDevice - the nonvol device to read from.
//
// Returns:
//      true if the settings were read (may be upgraded or defaulted).
//      false if there was an error in reading.
//
static bool ReadNonVolSection(BcmNonVolSettings::NonVolSection section,
                              const char *pSectionName,
                              BcmNonVolSettings *pSettings,
                              BcmNonVolDevice *pNonVolDevice)
{
    gLogMessageRaw << "\nReading " << pSectionName << " settings from non-vol..." << endl;

    if (pNonVolDevice->Read(*pSettings, section) == false)
    {
        gLogMessageRaw << "\n*\n*\n* Failed to read non-vol settings from the device!\n*\n*" << endl;

        return false;
    }

    if (pSettings->IsDefault(section))
    {
        gLogMessageRaw << "\n*\n*\n* One or more of the settings groups was missing, possibly as a result of a code upgrade.\n*\n*" << endl;

        // UNFINISHED - Do we need to do anything special?  Disable the
        // features that were going to use the nonvol?
    }
    if (pSettings->IsUpgraded(section))
    {
        gLogMessageRaw << "\n*\n*\n* One or more of the settings groups was upgraded.\n*\n*" << endl;

        // UNFINISHED - Do we need to do anything special?  Disable the
        // features that were going to use the nonvol?
    }

    gLogMessageRaw << "Settings were read and verified.\n" << endl;

    return true;
}


static void CommandHandler(void *pInstanceValue, const BcmCommand &command)
{
    switch (command.fCommandId - gCommandIdBase)
    {
        #if (BCM_TELNET_SUPPORT)
        
        case kLogout:
            {
                BcmTelnetThread *pTelnetThread = BcmTelnetThread::GetSingleton();

                if (pTelnetThread != NULL)
                {
                    if (!pTelnetThread->LogUserOut())
                    {
                        gLogMessageRaw << "Failed to log user out!\n";
                    }
                }
                else
                {
                    gLogMessageRaw << "There is no Telnet server!\n";
                }
            }
            break;

        #endif
        
        default:
            gLogMessageRaw 
                << "WARNING - unknown command id (" << command.fCommandId << ")!  Ignoring...\n";
            break;
    }
}


