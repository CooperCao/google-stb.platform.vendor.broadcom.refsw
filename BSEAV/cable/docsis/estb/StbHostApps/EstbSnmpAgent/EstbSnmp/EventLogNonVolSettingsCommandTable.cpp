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
//  Filename:       CmVendorNonVolSettingsCommandTable.cpp
//  Author:         David Pullen
//  Creation Date:  August 28, 2002
//
//****************************************************************************
//  Description:
//      This derived class handles set-up and configuration of the Thomson
//      Multimedia Cable Modem nonvol settings.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "EventLogNonVolSettingsCommandTable.h"

#include "CommandParameterList.h"
#include "CommandParameter.h"

#include "BoolCommandParameter.h"
#include "IpAddressCommandParameter.h"
#include "SignedIntCommandParameter.h"
#include "StringCommandParameter.h"
#include "UnsignedIntCommandParameter.h"

#include "EventLogNonVolSettings.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

static const char *pgTableDescr = "Commands to show/change the DOCSIS EventLog NonVol settings.";

// The command ids for the commands I can handle.
enum
{
    // Commands Supported
    kClearEvent = BcmNonVolSettingsCommandTable::kDerivedClassCommands,
    kSetEvent,
};

enum
{
    kPrimaryParameter = 1,
    kSecondParameter,
    kThirdParameter,
    kFourthParameter
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
BcmEventLogNonVolSettingsCommandTable::BcmEventLogNonVolSettingsCommandTable(void) :
    BcmNonVolSettingsCommandTable ("DOCSIS EventLog NonVol Commands", "event_log", "DOCSIS EventLog NonVol", pgTableDescr)
{
    BcmCommand command;

    // Set up fields that don't change.
    command.fIsEnabled = true;
    command.fIsHidden = false;
    command.fCommandFunctionWithParms = CommandHandler;

    command.fCommandId = kClearEvent;
    command.pfCommandName = "clear_event";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmSignedIntCommandParameter(kPrimaryParameter, false, "index", -1));
    command.pfCommandHelp = 
        "Causes an event at the specified index to be cleared.  If you specify -1, then "
        "all will be cleared.";
    command.pfExamples = 
        "clear_event 0\tClears the event at index 0.";
    AddCommand(command);

    command.fCommandId = kSetEvent;
    command.pfCommandName = "event";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, true, "index"));
    command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kSecondParameter, true, "id"));
    command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kThirdParameter, true, "count", 1, 0xffff));
    command.pfCommandHelp = 
        "Causes an event at the specified index to be set to the id and count specified.  "
        "The id is a 32-bit value, and the count is a 16-bit value.  The first/last time "
        "values are taken as the current time.";
    command.pfExamples = 
        "event 0 0x80015432 25\tSets event at index 0 to id 0x80015432, count 25.";
    AddCommand(command);
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmEventLogNonVolSettingsCommandTable::~BcmEventLogNonVolSettingsCommandTable()
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
void BcmEventLogNonVolSettingsCommandTable::CommandHandler(void *pInstanceValue,
                                                           const BcmCommand &command)
{
    // By convention, the settings object registers its instance so that the
    // user value is the 'this' pointer.  This cast should be safe.
    BcmEventLogNonVolSettings *pSettings = (BcmEventLogNonVolSettings *) pInstanceValue;

    if (pSettings == NULL)
    {
        gLogMessageRaw << "BcmCmVendorNonVolSettingsCommandTable::CommandHandler: ERROR - settings pointer is NULL!" << endl;

        return;
    }

    // Many commands have at least one parameter; retrieve it for them.
    const BcmCommandParameter *pParameter = NULL;

    if (command.pfCommandParameterList != NULL)
    {
        pParameter = command.pfCommandParameterList->Find(kPrimaryParameter);
    }

    switch (command.fCommandId)
    {
        case kClearEvent:
            if (pParameter->AsSignedInt() == -1)
            {
                gLogMessageRaw << "Clearing all events.\n";

                pSettings->ClearAllEvents();
            }
            else
            {
                EventLogNvRec event;
                BcmOctetBuffer Empty;

                gLogMessageRaw << "Clearing event at index " << pParameter->AsSignedInt() << "\n";

                memset(&event, 0, sizeof(event));

                pSettings->Event(pParameter->AsSignedInt(), event);
                pSettings->EventText (pParameter->AsSignedInt(), Empty);
            }
            break;

        case kSetEvent:
            {
                EventLogNvRec event;
                time_t now;
                const BcmCommandParameter *pIdParameter;
                const BcmCommandParameter *pCountParameter;

                pIdParameter = command.pfCommandParameterList->Find(kSecondParameter);
                pCountParameter = command.pfCommandParameterList->Find(kThirdParameter);

                time(&now);

                gLogMessageRaw
                    << "Setting event at index " << pParameter->AsUnsignedInt()
                    << " to id=" << (void *) pIdParameter->AsUnsignedInt()
                    << ", count=" << pCountParameter->AsUnsignedInt()
                    << ", and first/last time=" << now << endl;

                event.Id = pIdParameter->AsUnsignedInt();
                event.Count = pCountParameter->AsUnsignedInt();
                event.FirstTime = now;
                event.LastTime = now;

                pSettings->Event(pParameter->AsUnsignedInt(), event);
            }
            break;

        default:
            gLogMessageRaw << "WARNING - unknown command id (" << command.fCommandId << ")!  Ignoring...\n";
            break;
    }  
}



