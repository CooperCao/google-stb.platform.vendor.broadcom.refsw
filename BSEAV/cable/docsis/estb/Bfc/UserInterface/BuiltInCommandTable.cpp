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
//  $Id: BuiltInCommandTable.cpp 1.21 2003/06/02 12:32:21Z dpullen Release $
//
//  Filename:       BuiltInCommandTable.cpp
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

//********************** Include Files ***************************************

// My api and definitions...
#include "BuiltInCommandTable.h"

#include "StringSetCommandParameter.h"
#include "StringCommandParameter.h"
#include "UnsignedIntCommandParameter.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

static const char *pgTableDescr =
"System-wide, built-in commands that are handled by the Console Thread.  These\n"
"commands are always available, regardless of which command table is active.";

static const char *pgHelpOptions = "-t|-l|-s|-i|-a|-lr";

static const char *pgHelpCommandString = "command [...]";

static const char *pgHelpHelp =
    "Shows usage information about the specified command(s), or lists the set of "
    "commands available in the active table.  If no parameters are specified, then "
    "an abbreviated list of all commands and subtables is displayed.\n\n"
    "\t-t\tShows the entire tree of command tables and commands (in abbreviated form).\n"
    "\t-l\tShows detailed information about all commands and subtables (this can print a LOT of information)!\n"
    "\t-s\tShows detailed information on just the subtables.\n"
    "\t-i\tShows detailed information on just the registered instances for the active table.\n"
    "\t-a\tDoes everything that the -l, -s, and -i options do.\n\n"
    "Command is the name (or partial name) of one or more commands and subtables for "
    "which you want detailed help to be displayed.";

static const char *pgHelpExamples =
    "help\tThis shows an abbreviated list of all commands and subtables.\n"
    "help cd\tThis shows detailed help on the 'cd' command.\n"
    "help cd ! diag\tThis shows detailed help on the 2 commands and subtable listed.\n"
    "help -l\tShows detailed help on all available commands and subtables.";

static const char *pgHelpAlternateHelp = "Alias for 'help'.  Type 'help help' for more information."; 

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Initializing Constructor.  Creates the command table, adds all of the
// built-in commands to it, sets the command handler function and registers
// the instance with the table.
//
// Parameters:
//      commandFunction - the function that should be called when a command is
//                        is parsed.  This function must check the command
//                        id to tell which command was parsed.
//
// Returns:  N/A
//
BcmBuiltInCommandTable::BcmBuiltInCommandTable(CommandFunctionWithParms commandFunction) :
    BcmCommandTable("Built-in Commands", "built-in", "Command", pgTableDescr)
{
    BcmCommand command;

    // Add the built-in commands to the table.
    // Set up fields that don't change.
    command.fIsEnabled = true;
    command.fCommandFunctionWithParms = commandFunction;

    // For the "help" command.
    command.fCommandId = kHelpCommand;
    command.pfCommandName = "help";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(
        new BcmStringSetCommandParameter( pgHelpOptions, kPrimaryParameter, true));
    command.pfCommandParameterList->AddOrderDependent(
        new BcmStringCommandParameter( kSecondaryParameter, true, pgHelpCommandString, 127, true ));
    command.pfCommandHelp = pgHelpHelp;
    command.pfExamples = pgHelpExamples;
    AddCommand(command);

    // The "?" version of the help command.
    command.fCommandId = kQuestionCommand;
    command.pfCommandName = "?";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(
        new BcmStringSetCommandParameter( pgHelpOptions, kPrimaryParameter, true));
    command.pfCommandParameterList->AddOrderDependent(
        new BcmStringCommandParameter( kSecondaryParameter, true, pgHelpCommandString, 127, true ));
    command.pfCommandHelp = pgHelpAlternateHelp;
    command.pfExamples = NULL;
    AddCommand(command);

    // The "ls" version of the help command.
    command.fCommandId = kLsCommand;
    command.pfCommandName = "ls";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(
        new BcmStringSetCommandParameter( pgHelpOptions, kPrimaryParameter, true));
    command.pfCommandParameterList->AddOrderDependent(
        new BcmStringCommandParameter( kSecondaryParameter, true, pgHelpCommandString, 127, true ));
    command.pfCommandHelp = pgHelpAlternateHelp;
    AddCommand(command);

    // The "dir" version of the help command.
    command.fCommandId = kDirCommand;
    command.pfCommandName = "dir";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(
        new BcmStringSetCommandParameter( pgHelpOptions, kPrimaryParameter, true));
    command.pfCommandParameterList->AddOrderDependent(
        new BcmStringCommandParameter( kSecondaryParameter, true, pgHelpCommandString, 127, true ));
    command.pfCommandHelp = pgHelpAlternateHelp;
    AddCommand(command);

    // The "man" version of the help command.
    command.fCommandId = kManCommand;
    command.pfCommandName = "man";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(
        new BcmStringSetCommandParameter( pgHelpOptions, kPrimaryParameter, true));
    command.pfCommandParameterList->AddOrderDependent(
        new BcmStringCommandParameter( kSecondaryParameter, true, pgHelpCommandString, 127, true ));
    command.pfCommandHelp = pgHelpAlternateHelp;
    AddCommand(command);

    command.fCommandId = kBangCommand;
    command.pfCommandName = "!";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(
        new BcmUnsignedIntCommandParameter( kPrimaryParameter, true, NULL, 0, 15 ));
    command.pfCommandParameterList->AddOrderDependent(
        new BcmStringCommandParameter( kSecondaryParameter, true, "command" ));
    command.pfCommandHelp = "Executes the last command that was entered.  If a command (or history number) "
                            "is specified, then it executes that command from the history buffer.  This "
                            "works like the Unix '!' command";
    command.pfExamples = "!\tThis repeats the last command that was entered.\n"
                         "! cd\tThis repeats the last 'cd' command that was entered.";
    AddCommand(command);

    command.fCommandId = kChangeActiveTable;
    command.pfCommandName = "cd";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(
        new BcmStringCommandParameter( kPrimaryParameter, true, "subtable | .. | \\ | /" ));
    command.pfCommandHelp = "Sets the specified command table as the active table.  This works like the DOS "
                            "or Unix 'cd' command where '..' takes you to the previous table, and '\\' or "
                            "'/' takes you to the root table.  If no parameters are specified, then it "
                            "shows the name of the currently active command table.";
    command.pfExamples = "cd classifiers\tMakes the specified subtable the active command table.\n"
                         "cd \\\tMakes the main command table active.\n"
                         "cd ..\tMakes the previous (parent) command table active.\n"
                         "cd\tShows the name of the active command table.\n"
                         "cd \\non\\doc\tYou can specify partial names, and mutliple subdirs.";
    AddCommand(command);

    command.fCommandId = kPwdCommand;
    command.pfCommandName = "pwd";
    command.pfCommandHelp = "Shows the name of the currently active command table.  This is like the Unix "
                            "'pwd' command.";
    command.pfExamples = "pwd\t";
    AddCommand(command);

    command.fCommandId = kShowCommandHistory;
    command.pfCommandName = "history";
    command.pfCommandHelp = "Shows a list of commands that were previously typed.";
    command.pfExamples = "history\t";
    AddCommand(command);

    // The instances command must take its only parameter as a "secondary" parameter
    // because there will be times when the "help -i" command is processed as an
    // instances command.  This is sneaky, but not too big of a kludge.
    command.fCommandId = kShowInstances;
    command.pfCommandName = "instances";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(
        new BcmStringCommandParameter( kSecondaryParameter, true, "name" ));
    command.pfCommandHelp = "Shows the set of object instances that have registered with the active table.  "
                            "This is the same as 'help -i', except that it lets you specify a partial "
                            "instance name in order to limit the list that is displayed (only instances "
                            "whose names match the partial string are shown).  The name is not case "
                            "sensitive.";
    command.pfExamples = "instances\tShows all instances registered with the command table.\n"
                         "instances p\tShows all instances whose name begins with 'p' or 'P'.";
    AddCommand(command);

    command.fCommandId = kShowCurrentTime;
    command.pfCommandName = "system_time";
    command.pfCommandHelp = "Displays the current system millisecond tick counter.";
    command.pfExamples = "system_time\t";
    AddCommand(command);

    command.fCommandId = kShowUsage;
    command.pfCommandName = "usage";
    command.pfCommandHelp = "Displays information about how the console works, and how to use it.";
    command.pfExamples = "usage\t";
    AddCommand(command);

    command.fCommandId = kFindCommand;
    command.pfCommandName = "find_command";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(
        new BcmStringCommandParameter( kPrimaryParameter, false, "command" ));
    command.pfCommandHelp = "Displays the name of all subdirectories which contain the specified command.";
    command.pfExamples = "find_command show\t";
    AddCommand(command);

    command.fCommandId = kSleepCommand;
    command.pfCommandName = "sleep";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(
        new BcmUnsignedIntCommandParameter(kPrimaryParameter, false, "Milliseconds"));
    command.pfCommandHelp = "Causes the console to sleep for the specified number of milliseconds.  This "
                            "is useful for scripting, where you want to delay between commands.";
    command.pfExamples = "sleep 1000\tMakes the console sleep for 1 second";
    AddCommand(command);

    command.fCommandId = kRemarkCommand;
    command.pfCommandName = "REM";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(
        new BcmStringCommandParameter(kPrimaryParameter, true, "Remark text", 127, true ));
    command.pfCommandHelp = "Ignores the text that follows; used for remarks, scripting, etc.";
    command.pfExamples = "REM Started test here.\t";
    AddCommand(command);

    command.fCommandId = kSyntaxCommand;
    command.pfCommandName = "syntax";
    command.pfCommandHelp = "Displays detailed information on command line syntax and how the parser works.";
    command.pfExamples = "syntax\t";
    AddCommand(command);
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmBuiltInCommandTable::~BcmBuiltInCommandTable()
{
    // Nothing to do...
}


