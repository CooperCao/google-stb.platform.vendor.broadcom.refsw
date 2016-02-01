//****************************************************************************
//
//  Copyright (c) 2002  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92618
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id$
//
//  Filename:       LinuxConsoleCommands.cpp
//  Author:         David Pullen
//  Creation Date:  December 5, 2002
//
//****************************************************************************
//  Description:
//      This file contains V2 console commands specific to Linux.  All V2
//      applications built for Linux will likely want to include these
//      commands somewhere (usually the main command table).
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include <stdlib.h>
#include "LinuxConsoleCommands.h"

#include "CommandTable.h"

// Handles message logging stuff.
#include "MessageLog.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

enum
{
    // Enum values for the commands will go here, starting with 1.  
    kShellCommand = 1,
    kTaskShowCommand
};

//********************** Local Variables *************************************

static unsigned int gCommandIdBase = 0;

//********************** Local Functions *************************************

static void CommandHandler(void *pInstanceValue, const BcmCommand &command);

//********************** Global Functions ************************************
extern "C"
{
	void DisplayThreadInfo(void);
}

//********************** Function Implementations ****************************


// This function installs the Linux-specific command table support with the
// specified parent command table.
//
// Parameters:
//      pParentCmdTable - the command table to be used as the parent.
//      commandIdBase - the base id that I should use for the commands that I
//                      add to the command table.
//
// Returns:  Nothing.
//
void LinuxCmdTableInit(BcmCommandTable *pParentCmdTable,
                       unsigned int commandIdBase)
{
    if (pParentCmdTable != NULL)
    {
        // Store this for future use; it is the offset for all of my commands,
        // which allows me to avoid conflicts with other modules that are adding
        // commands to this command table.
        gCommandIdBase = commandIdBase;

        BcmCommand command;

        // Set up fields that don't change. 
        command.fCommandFunctionWithParms = CommandHandler;
        command.fIsEnabled = true;
        command.fIsHidden = false;

        command.fCommandId = kShellCommand + gCommandIdBase;
        command.pfCommandName = "shell";
        command.pfCommandHelp = "Causes the application to jump to Linux shell.";
        command.pfExamples = "shell\t";
        pParentCmdTable->AddCommand(command);

        #if (BCM_CONSOLE_SUPPORT)
        
        command.fCommandId = kTaskShowCommand + gCommandIdBase;
        command.pfCommandName = "taskShow";
        command.pfCommandHelp = "Displays current USER MODE tasks information.";
        command.pfExamples = "taskShow\t";
        pParentCmdTable->AddCommand(command);

		#endif
        
    }
}


static void CommandHandler(void * /*pInstanceValue*/, const BcmCommand &command)
{
    switch (command.fCommandId - gCommandIdBase)
    {
        case kShellCommand:
			system("bash");
            break;
        case kTaskShowCommand:
			{
				DisplayThreadInfo();
			}
            break;
        default:
            gLogMessageRaw 
                << "WARNING - unknown command id (" << command.fCommandId << ")!  Ignoring...\n";
            break;
    }
}


