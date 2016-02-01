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
//  $Id: MsgLogSettingsCommandTable.cpp 1.19 2007/01/03 22:14:36Z ceyles Release $
//
//  Filename:       MsgLogSettingsCommandTable.cpp
//  Author:         David Pullen
//  Creation Date:  Feb 27, 2000
//
//****************************************************************************
//  Description:
//      This derived class handles set-up of the Message Log Settings commands
//      in a command table.  This is used by the BcmMessageLogSettings class.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "MsgLogSettingsCommandTable.h"

#if (BCM_TELNET_SUPPORT  || BCM_SSH_SUPPORT || BCM_HTTP_SUPPORT)
#include "RemoteAccessCommandTable.h"
#endif

#if ENABLE_CALL_TRACING
#include "CallTraceCommandTable.h"
#endif

#include "MessageLogSettings.h"

#include "BitmaskCommandParameter.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

static const char *pgTableDescr = 
    "Commands for changing Message Log Settings.  These commands allow you enable\n"
    "and disable display fields and severity levels for individual objects or for\n"
    "groups of objects.";

// Constants for the commands.
enum
{
    kShowSettings = 0,
    kChangeSeverities,
    kChangeFields
};

// Constants for the command parameters.
enum
{
    kPrimaryParameter = 1,
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
BcmMsgLogSettingsCommandTable::BcmMsgLogSettingsCommandTable(void) :
    BcmCommandTable("Message Log Settings Commands", "msgLog", "MsgLog", pgTableDescr)
{
    BcmCommand command;

    // Set up fields that don't change.
    command.fCommandFunctionWithParms = CommandHandler;
    command.fIsEnabled = true;
    command.fIsHidden = false;

    command.fCommandId = kShowSettings;
    command.pfCommandName = "show_settings";
    command.pfCommandHelp = "Displays the current Message Log Settings.";
    command.pfExamples = "show_settings\t";
    AddCommand(command);

    command.fCommandId = kChangeSeverities;
    command.pfCommandName = "severities";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmBitmaskCommandParameter(kPrimaryParameter, true));
    command.pfCommandHelp = "Displays or sets the different message severity levels that are enabled for "
                            "display.  Message logging bit definitions:\n\n"
                            "\t0x00000001\tFatal Errors\n"
                            "\t0x00000002\tErrors\n"
                            "\t0x00000004\tWarnings\n"
                            "\t0x00000008\tInitialization\n"
                            "\t0x00000010\tFunction entry/exit\n"
                            "\t0x00000020\tInformational\n"
                            "\t0xffffffff\tAll messages";
    command.pfExamples = "severities 0\tTurns off all messages.\n"
                         "severities 0xffffffff\tTurns on all messages.";
    AddCommand(command);

    command.fCommandId = kChangeFields;
    command.pfCommandName = "fields";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmBitmaskCommandParameter(kPrimaryParameter, true, NULL, 0x7f));
    command.pfCommandHelp = 
        "Displays or sets the different message fields that are enabled for display.  "
        "Message field bit definitions:\n\n"
        "\t0x01\tThe severity of the message (INFO, WARNING, ERROR, etc.)\n"
        "\t0x02\tThe instance name of the object that generated the message.\n"
        "\t0x04\tThe function/method in which the message was generated.\n"
        "\t0x08\tThe name of the module/class in which the message was generated.\n"
        "\t0x10\tThe system timestamp (Time of Day).\n"
        "\t0x20\tThe thread Id.\n"
        "\t0x40\tThe system timestamp (millisecond, in hex).";
    command.pfExamples = "fields 0\tTurns off all optional fields.\n"
                         "fields 0x7f\tTurns on all optional fields.";
    AddCommand(command);

    #if (BCM_TELNET_SUPPORT || BCM_SSH_SUPPORT || BCM_HTTP_SUPPORT)

    // If compiled in, add the subtable to control the remote access servers.
    pfRemoteAccessCommandTable = new BcmRemoteAccessCommandTable;

    AddSubtable(pfRemoteAccessCommandTable);

    #endif
    
    #if ENABLE_CALL_TRACING
    
    // If compiled in, add the subtable to control call tracing.
    pfCallTraceCommandTable = new BcmCallTraceCommandTable;

    AddSubtable(pfCallTraceCommandTable);

    #endif
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmMsgLogSettingsCommandTable::~BcmMsgLogSettingsCommandTable()
{
    #if (BCM_TELNET_SUPPORT || BCM_SSH_SUPPORT || BCM_HTTP_SUPPORT)

    // If compiled in, remove the subtable to control the remote access servers.
    RemoveSubtable(pfRemoteAccessCommandTable);

    delete pfRemoteAccessCommandTable;
    pfRemoteAccessCommandTable = NULL;

    #endif
    
    #if ENABLE_CALL_TRACING
    
    // If compiled in, remove the subtable to control call tracing.
    RemoveSubtable(pfCallTraceCommandTable);

    delete pfCallTraceCommandTable;
    pfCallTraceCommandTable = NULL;

    #endif
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
void BcmMsgLogSettingsCommandTable::CommandHandler(void *pInstanceValue, 
                                                   const BcmCommand &command)
{
    // By convention, the message log settings registers its instance so that
    // the user value is the 'this' pointer.  This cast should be safe.
    BcmMessageLogSettings *pMsgLogSettings = (BcmMessageLogSettings *) pInstanceValue;
    const BcmCommandParameter *pParameter = NULL;
    char bitNames[256];

    // Validate this just for sanity.
    if (pMsgLogSettings == NULL)
    {
        gLogMessageRaw << "BcmMsgLogSettingsCommandTable::CommandHandler:  ERROR - "
             << "User value (pointer to Message\n"
                "Log Settings instance) is NULL!  Can't process the command!\n";

        return;
    }

    // Many commands below have a single primary parameter.  Retrieve it for
    // them.
    if (command.pfCommandParameterList != NULL)
    {
        pParameter = command.pfCommandParameterList->Find(kPrimaryParameter);
    }

    switch (command.fCommandId)
    {
        case kShowSettings:
            gLogMessageRaw << *pMsgLogSettings;
            break;

        case kChangeFields:
            if (pParameter->WasParsed())
            {
                pMsgLogSettings->SetEnabledFields( pParameter->AsBitmask(pMsgLogSettings->GetEnabledFields()) );
            }
            
            BcmMessageLogSettings::GetEnabledFieldsNames(pMsgLogSettings->GetEnabledFields(), 
                                                         bitNames, 
                                                         sizeof(bitNames));

            gLogMessageRaw 
                << "Enabled Fields = " 
                << inHex(pMsgLogSettings->GetEnabledFields())
                << " (" << bitNames << ")\n";
            break;

        case kChangeSeverities:
            if (pParameter->WasParsed())
            {
                pMsgLogSettings->SetEnabledSeverities( pParameter->AsBitmask(pMsgLogSettings->GetEnabledSeverities()) );
            }

            BcmMessageLogSettings::GetEnabledSeveritiesNames(pMsgLogSettings->GetEnabledSeverities(), 
                                                             bitNames, sizeof(bitNames));

            gLogMessageRaw 
                << "Enabled Severities = " 
                << inHex(pMsgLogSettings->GetEnabledSeverities())
                << " (" << bitNames << ")\n";
            break;

        default:
            gLogMessageRaw << "WARNING - unknown command id (" << command.fCommandId << ")!  Ignoring...\n";
            break;
    }
}


