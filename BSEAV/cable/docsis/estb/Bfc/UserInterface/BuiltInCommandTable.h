//****************************************************************************
//
//  Copyright (c) 2000  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16251 Laguna Canyon Road
//          Irvine, California 92618
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id: BuiltInCommandTable.h 1.11 2003/06/02 12:32:28Z dpullen Release $
//
//  Filename:       BuiltInCommandTable.h
//  Author:         David Pullen
//  Creation Date:  Feb 17, 2000
//
//****************************************************************************
//  Description:
//      This derived class handles set-up of the built-in commands in a command
//      table.  This is used by the BcmConsoleThread class.  This object is
//      different from most other command tables in that it doesn't implement
//      the command handlers; these are implemented in the console thread base
//      class.
//
//****************************************************************************

#ifndef BUILTINCOMMANDTABLE_H
#define BUILTINCOMMANDTABLE_H

//********************** Include Files ***************************************

// My base class.
#include "CommandTable.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************


class BcmBuiltInCommandTable : public BcmCommandTable
{
public:

    // Initializing Constructor.  Creates the command table, adds all of the
    // commands to it and sets the command handler function.
    //
    // Parameters:
    //      commandFunction - the function that should be called when a command is
    //                        is parsed.  This function must check the command
    //                        id to tell which command was parsed.
    //
    // Returns:  N/A
    //
    BcmBuiltInCommandTable(CommandFunctionWithParms commandFunction);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmBuiltInCommandTable();

    // Constants for the built-in commands.
    enum
    {
        kHelpCommand = 0,           // This is for "help"
        kQuestionCommand,           // This is for "?"
        kLsCommand,                 // This is for "ls"
        kDirCommand,                // This is for "dir"
        kManCommand,                // This is for "man"
        kBangCommand,               // This is for "!"
        kChangeActiveTable,         // This is for "cd"
        kPwdCommand,                // This is for "pwd"
        kShowCommandHistory,        // This is for "history"
        kShowInstances,             // This is for "instances"
        kShowCurrentTime,           // This is for "time"
        kShowUsage,                 // This is for "usage"
        kFindCommand,               // This is for "find_command"
        kSleepCommand,              // This is for "sleep"
        kRemarkCommand,             // This is for "REM"
        kSyntaxCommand              // This is for "syntax"
    };

    // Constants for command parameter identifiers
    enum
    {
        kPrimaryParameter = 1,
        kSecondaryParameter = 2
    };

    // Constants for the options/parameters for help.
    // They must correspond to the StringSetCommandParameter
    // list: -t|-l|-s|-i|-a|-lr
    enum
    {
        kDashT = 0,
        kDashL,
        kDashS,
        kDashI,
        kDashA,
        kDashLR
    };
    
protected:

private:

    // Default Constructor.  Not supported.
    BcmBuiltInCommandTable(void);

    // Copy Constructor.  Not supported.
    BcmBuiltInCommandTable(const BcmBuiltInCommandTable &otherInstance);

    // Assignment operator.  Not supported.
    BcmBuiltInCommandTable & operator = (const BcmBuiltInCommandTable &otherInstance);

};


//********************** Inline Method Implementations ***********************


#endif


