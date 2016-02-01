//****************************************************************************
//
//  Copyright (c) 2000  Broadcom Corporation
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
//  $Id$
//
//  Filename:       NonVolCommandTable.cpp
//  Author:         David Pullen
//  Creation Date:  Feb 18, 2000
//
//****************************************************************************
//  Description:
//      This derived class handles set-up of the non-volatile settings
//      commands in a command table.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "CmAppNonVolCommandTable.h"

#include "CmAppNonVolSettings.h"

#include "BoolCommandParameter.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

static const char *pgTableDescr = "Commands to change the non-volatile settings for the Cable Modem Application.";

// Constants for the commands.
enum
{
    kChangeAutoConsole = BcmNonVolSettingsCommandTable::kDerivedClassCommands,
    kChangeDriverInitPrompt,
    kChangeStopAtConsolePrompt,
};
    
enum
{
	kPrimaryParameter = 1,
};

//********************** Local Variables *************************************

//********************** External Variables **********************************

//********************** Class Method Implementations ************************


// Default Constructor.  Creates the command table, adds all of the
// commands to it and sets the command handler function.
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmCmAppNonVolCommandTable::BcmCmAppNonVolCommandTable(void) :
    BcmNonVolSettingsCommandTable("CM Application Settings Commands", "cmapp", "CmApp", pgTableDescr)
{
    BcmCommand command;

    // Set up fields that don't change.
    command.fCommandFunctionWithParms = CommandHandler;
    command.fIsEnabled = true;
    command.fIsHidden = false;

    command.fCommandId = kChangeAutoConsole;
    command.pfCommandName = "auto_console";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(
		new BcmBoolCommandParameter( kPrimaryParameter, true, NULL, false ));
    command.pfCommandHelp = "Sets whether or not the application should automatically stop at the console "
                            "instead of running the application after waiting for the user to hit a key.";
    command.pfExamples = "auto_console\tShows the current auto_console setting.\n"
                         "auto_console false\tWaits for the user to hit a key, then runs the app.\n"
                         "auto_console true\tAutomatically breaks out into the console.";
    AddCommand(command);

    command.fCommandId = kChangeDriverInitPrompt;
    command.pfCommandName = "driver_init_prompt";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(
		new BcmBoolCommandParameter( kPrimaryParameter, true, NULL, false ));
    command.pfCommandHelp = "Sets whether or not the application should wait for a keypress in order to "
                            "bypass driver initialization.  Disabling this will speed up boot time.";
    command.pfExamples = "driver_init_prompt 0\tDisables the driver init prompt.";
    AddCommand(command);

    command.fCommandId = kChangeStopAtConsolePrompt;
    command.pfCommandName = "console_stop_prompt";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(
		new BcmBoolCommandParameter( kPrimaryParameter, true, NULL, false ));
    command.pfCommandHelp = "Sets whether or not the application should wait for a keypress in order to "
                            "stop at the console.  Disabling this will speed up boot time.";
    command.pfExamples = "console_stop_prompt 0\tDisables the console init prompt.";
    AddCommand(command);

    // Try to get the singleton instance of the settings object, and add it as
    // the one-and-only instance to me.
    BcmNonVolSettings *pSingleton = BcmCmAppNonVolSettings::GetSingletonInstance();

    if (pSingleton != NULL)
    {
        AddInstance("CM Application NonVol Settings", pSingleton);
    }
    else
    {
        gLogMessageRaw 
            << "BcmCmAppNonVolCommandTable::BcmCmAppNonVolCommandTable: ERROR - Singleton instance of BcmCmAppNonVolSettings is NULL!" << endl;
    }
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmCmAppNonVolCommandTable::~BcmCmAppNonVolCommandTable()
{
    // Nothing much to do.
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
void BcmCmAppNonVolCommandTable::CommandHandler(void *pInstanceValue, const BcmCommand &command)
{
    const BcmCommandParameter *pParameter = NULL;

    // By convention, the settings object registers its instance so that the
    // user value is the 'this' pointer.  This cast should be safe.
    BcmCmAppNonVolSettings *pNonVolSettings = (BcmCmAppNonVolSettings *) pInstanceValue;

    // Validate this just for sanity.
    if (pNonVolSettings == NULL)
    {
        gLogMessageRaw << "BcmCmAppNonVolCommandTable::CommandHandler:  ERROR - "
                       << "User value (pointer to NonVol\n"
                          "Settings instance) is NULL!  Can't process the command!" << endl;

        return;
    }

    // Retrieve the primary parameter for any command has one.
    if (command.pfCommandParameterList != NULL)
    {
        pParameter = command.pfCommandParameterList->Find(kPrimaryParameter);
    }

    switch (command.fCommandId)
    {
        case kChangeAutoConsole:
            if (pParameter->WasParsed())
            {
                pNonVolSettings->AutoStopAtConsole(pParameter->AsBool());
            }

            gLogMessageRaw << "Automatically stop at console = " << pNonVolSettings->AutoStopAtConsole() << "\n";
            break;

        case kChangeDriverInitPrompt:
            if (pParameter->WasParsed())
            {
                pNonVolSettings->DriverInitPrompt(pParameter->AsBool());
            }

            // PR2755 - reversed the last change to this string because it broke
            // some scripts.
            gLogMessageRaw << "'Skip Driver Init' prompt enabled = " << pNonVolSettings->DriverInitPrompt() << "\n";
            break;

        case kChangeStopAtConsolePrompt:
            if (pParameter->WasParsed())
            {
                pNonVolSettings->StopAtConsolePrompt(pParameter->AsBool());
            }

            // PR2755 - reversed the last change to this string because it broke
            // some scripts.
            gLogMessageRaw << "'Stop at console' prompt enabled = " << pNonVolSettings->StopAtConsolePrompt() << "\n";
            break;

        default:
            gLogMessageRaw << "WARNING - unknown command id (" << command.fCommandId << ")!  Ignoring..." << endl;
            break;
    }
}

