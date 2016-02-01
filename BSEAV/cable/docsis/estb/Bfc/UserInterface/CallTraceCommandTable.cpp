//****************************************************************************
//
//  Copyright (c) 2001  Broadcom Corporation
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
//  $Id: CallTraceCommandTable.cpp 1.1 2001/08/02 20:09:02Z dpullen Release $
//
//  Filename:       CallTraceCommandTable.cpp
//  Author:         David Pullen
//  Creation Date:  Feb 27, 2000
//
//****************************************************************************
//  Description:
//      This derived class handles set-up of the Call Trace commands in a
//      command table.  This is used by the message logging facilities to
//      enable/disable call tracing.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "CallTraceCommandTable.h"

#include "BoolCommandParameter.h"
#include "SignedIntCommandParameter.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

static const char *pgTableDescr = 
"Commands for changing Call Trace Settings.  These commands allow you enable\n"
"and disable call tracing for various threads in the system.";

// Constants for the commands.
enum
{
    kShowSettings = 0,
    kGlobalCallTrace,
    kThreadCallTrace,
};

// Constants for the command parameters.
enum
{
    kPrimaryParameter = 1,
    kSecondParameter
};

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Default Constructor.  Creates the command table, adds all of the commands
// to it and sets the command handler function.
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmCallTraceCommandTable::BcmCallTraceCommandTable(void) :
    BcmCommandTable("Call Trace Commands", "callTrace", "CallTrace", pgTableDescr)
{
    BcmCommand command;

    // Set up fields that don't change.
    command.fCommandFunctionWithParms = CommandHandler;
    command.fIsEnabled = true;
    command.fIsHidden = false;

    command.fCommandId = kShowSettings;
    command.pfCommandName = "show";
    command.pfCommandHelp = 
        //                                                                               | <- 80
        "Displays the current Call Trace settings.";
    command.pfExamples = "show";
    AddCommand(command);

    command.fCommandId = kGlobalCallTrace;
    command.pfCommandName = "global_enable";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmBoolCommandParameter(kPrimaryParameter, true));
    command.pfCommandHelp = 
        //                                                                               | <- 80
        "Sets or shows the global Call Trace enable state.  If disabled, then no threads\n"
        "will do Call Tracing.  If enabled, then each thread will do Call Tracing based\n"
        "on the per-thread state.";
    command.pfExamples = "global_enable 1       Enables global Call Tracing.";
    AddCommand(command);

    command.fCommandId = kThreadCallTrace;
    command.pfCommandName = "thread_enable";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmSignedIntCommandParameter(kPrimaryParameter, false, "ThreadIndex"));
    command.pfCommandParameterList->AddOrderDependent(new BcmBoolCommandParameter(kSecondParameter, true));
    command.pfCommandHelp = 
        //                                                                               | <- 80
        "Sets or shows the per-thread Call Trace enable state.  If Call Tracing is\n"
        "enabled globally, and enabled for the thread, then that thread will do Call\n"
        "Tracing.  Type 'show' to see the list of threads that can be specified.  If you\n"
        "specify -1 as the ThreadIndex parameter, then all threads will be set.";
    command.pfExamples = "thread_enable -1 0    Disables Call Tracing for all threads.\n"
                         "thread_enable -1      Shows the enable state for all threads.\n"
                         "thread_enable 0 1     Enables Call Tracing for thread index 0.";
    AddCommand(command);

    // I don't need/use the instance pointer, since I can get to everything via
    // singletons.  Just set up a dummy NULL instance pointer.
    AddInstance("Singleton Instance", NULL);
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmCallTraceCommandTable::~BcmCallTraceCommandTable()
{
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
void BcmCallTraceCommandTable::CommandHandler(void * /*pInstanceValue*/, 
                                              const BcmCommand &command)
{
    const BcmCommandParameter *pParameter = NULL;

    // Many commands below have a single primary parameter.  Retrieve it for
    // them.
    if (command.pfCommandParameterList != NULL)
    {
        pParameter = command.pfCommandParameterList->Find(kPrimaryParameter);
    }

    switch (command.fCommandId)
    {
        case kShowSettings:
            gLogMessageRaw 
                << "Call Trace Settings:\n"
                << "    Global Enable: " << gGlobalCallTraceEnabled << "\n";

            // Loop over the registered threads, and show their enable state.
            break;

        case kGlobalCallTrace:
            if (pParameter->WasParsed())
            {
                gGlobalCallTraceEnabled = pParameter->AsBool();
            }
            
            gLogMessageRaw << "Global Call Trace enabled = " << gGlobalCallTraceEnabled << "\n";
            break;

        case kThreadCallTrace:
            gLogMessageRaw << "NOT YET IMPLEMENTED..." << endl;
            break;

        default:
            gLogMessageRaw << "WARNING - unknown command id (" << command.fCommandId << ")!  Ignoring...\n";
            break;
    }
}


