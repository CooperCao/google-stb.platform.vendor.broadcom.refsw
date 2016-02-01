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
//
//  Filename:       DsgccNonVolSettingsCommandTable.cpp
//  Author:         Kenny Lee
//  Creation Date:  Mar 15, 2007
//
//****************************************************************************
//  Description:
//      This derived class handles set-up of the Dsg Client Controller non-volatile
//      settings commands in a command table.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
// My api and definitions...
#include "DsgccNonVolSettingsCommandTable.h"

#include "MergedNonVolDefaults.h"
#include "DsgccNonVolSettings.h"

#include "ConsoleThread.h"
 
#include "BoolCommandParameter.h"
#include "MacAddressCommandParameter.h"
#include "UnsignedIntCommandParameter.h"
#include "SignedIntCommandParameter.h"

#include "IpAddressCommandParameter.h"
#include "StringCommandParameter.h"
#include "StringSetCommandParameter.h"

#include "IpHelperFactory.h"

//********************** Local Variables *************************************

//********************** Local Types *****************************************

//********************** Local Constants *************************************

static const char *pgTableDescr = "Commands to show/change the ESTB NonVol settings.";

// The command ids for the commands I can handle.
enum
{
    // Command Supported
    kDsgMode = BcmNonVolSettingsCommandTable::kDerivedClassCommands
#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE)
	,kIpcInterfaceName
#endif
};

enum
{
    kPrimaryParameter = 1,
    kSecondaryParameter,
    kThirdParameter
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
BcmDsgccNonVolSettingsCommandTable::BcmDsgccNonVolSettingsCommandTable(void) :
    BcmNonVolSettingsCommandTable("dsgcc NonVol Commands", "dsgcc", "dsgcc NonVol", pgTableDescr)
{
    BcmCommand command;

    // Set up fields that don't change.
    command.fIsEnabled = true;
    command.fIsHidden = false;
    command.fCommandFunctionWithParms = CommandHandler;
    
    command.fCommandId = kDsgMode;
    command.pfCommandName = "dsg_mode";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, true, "DSG Mode"));
    command.pfCommandHelp = 
        "Set global DSG mode:\n"
        "  0 - Disabled\n"
		"  1 - Basic\n"
        "  2 - Advance\n";
    command.pfExamples = "dsg_mode 0 (disabled)\n"
						 "dsg_mode 1 (basic)\n"
						 "dsg_mode 2 (advance)\n";
    AddCommand(command);

#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE)
    command.fCommandId = kIpcInterfaceName;
    command.pfCommandName = "ipc_interface";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, true, "IPC Interface", IPC_INTERFACE_NAME_MAX_LEN, false));
    command.pfCommandHelp = 
        "  Set IPC interface name.\n";
	command.pfExamples = "  ipc_interface eth0:0";
    AddCommand(command);
#endif

	// Try to get the singleton instance of the settings object, and add it as
    // the one-and-only instance to me.
    BcmDsgccNonVolSettings *pSingleton = BcmDsgccNonVolSettings::GetSingletonInstance();

    if (pSingleton != NULL)
    {
        AddInstance("Dsgcc NonVol Settings", pSingleton);
    }
    else
    {
        gLogMessageRaw 
            << "BcmDsgccNonVolSettingsCommandTable::BcmDsgccNonVolSettingsCommandTable: ERROR - Singleton instance of BcmDsgccNonVolSettingsCommandTable is NULL!" << endl;
    }

}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmDsgccNonVolSettingsCommandTable::~BcmDsgccNonVolSettingsCommandTable()
{
    // Nothing to do...
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
void BcmDsgccNonVolSettingsCommandTable::CommandHandler(void *pInstanceValue, 
                                                           const BcmCommand &command)
{
    const char *pRebootString = "\nNOTE:  You must write the settings and reboot for the change to take effect!\n";
    bool somethingChanged = false;
    const BcmCommandParameter *pParameter = NULL;
    
    // By convention, the settings object registers its instance so that the
    // user value is the 'this' pointer.  This cast should be safe.
    BcmDsgccNonVolSettings *pSettings = (BcmDsgccNonVolSettings *) pInstanceValue;

    if (pSettings == NULL)
    {
        gLogMessageRaw << "BcmDsgccNonVolSettingsCommandTable::CommandHandler: ERROR - settings pointer is NULL!" << endl;

        return;
    }


    if (command.pfCommandParameterList != NULL)
    {
        pParameter = command.pfCommandParameterList->Find(kPrimaryParameter);
    }

    switch (command.fCommandId)
    {
        case kDsgMode:
        {
            if (pParameter->WasParsed())
            {
                pSettings->DsgMode(pParameter->AsUnsignedInt());

                somethingChanged = true;
            }
            int mode = pSettings->DsgMode();
			gLogMessageRaw << "DSG Mode:  " << mode << " = " << ((mode == kDsgDisabledMode) ? "DISABLED" : 
																 ((mode == kDsgBasicMode) ? "BASIC" :
																 ((mode == kDsgAdvanceMode) ? "ADVANCE" :"NOT RECOGNIZED") )) <<endl;
        }
        break;

#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE)
		case kIpcInterfaceName:
			{
				if (pParameter->WasParsed())
				{
					pSettings->IpcInterfaceName(pParameter->AsString());
					somethingChanged = true;
				}
				gLogMessageRaw << pSettings->IpcInterfaceName() << " is used for eCM-eSTB IPC." << endl;
			}
			break;
#endif
        default:
            gLogMessageRaw << "WARNING - unknown command id (" << command.fCommandId << ")!  Ignoring...\n";
            break;
    }  

    if (somethingChanged)
    {
        gLogMessageRaw << pRebootString;
    }


}



