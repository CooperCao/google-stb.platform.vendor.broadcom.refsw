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
//  $Id: ConsoleThread.cpp 1.74 2005/11/09 15:56:46Z sdunlap Release $
//
//  Filename:       ConsoleThread.cpp
//  Author:         David Pullen
//  Creation Date:  Feb 11, 2000
//
//****************************************************************************
//  Description:
//      This is the abstract base class for all types of methods for gathering
//      input from users, and presenting either "command-line" or "menu" based
//      interfaces.  It allows users to configure various objects in the system.
//
//      All of the command parsing/processing occurs in the base class.  The
//      actual input and output of the command strings or responses are done
//      in the derived classes, who define what methods will be used.
//
//      The console is a stand-alone thread so that it's operation doesn't
//      block anything else in the system.  You will generally want to make this
//      the lowest priority thread.
//
//      In addition, it is a singleton, meaning that there is only one of these
//      in the system, and it is "globally" accessible.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "ConsoleThread.h"

#include "SystemTime.h"
#include "OperatingSystemFactory.h"

#include <stdio.h>

//********************** Local Types *****************************************

//********************** Local Constants *************************************

// Added for PR3402
enum
{
    kInhibitSleepTimeMs = 100
};

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************

// Initially, this is NULL.  The constructor will store the "this" pointer here.
BcmConsoleThread *BcmConsoleThread::pfSingletonInstance = NULL;


// Default Constructor.  Initializes the state of the object, starting the
// thread.
//
// Parameters:
//      startSuspended - tells whether or not the thread should suspend itself
//                       after initialization.
//      initialPriority - the priority at which you want the thread to run
//                        after initialization.
//
// Returns:  N/A
//
BcmConsoleThread::BcmConsoleThread(bool startSuspended,
                                   BcmOperatingSystem::ThreadPriority initialPriority) :
    BcmThread("ConsoleThread", startSuspended, initialPriority),
    fBuiltInCommands(UserCommandEntrypoint),
    pfRemoteAccessCommandTable( NULL ),
    pfEnableRemoteAccessSuperUser( false )
{
    // Override the class name given by my parent class.
    fMessageLogSettings.SetModuleName("BcmConsoleThread");

    fTimeToExit = false;

    pfMainCommandTable = NULL;
    pfActiveCommandTable = NULL;
    pfSavedCommandTable = NULL;

    memset(pfCommandHistory, 0, sizeof(pfCommandHistory));
    fNextCommandIndex = 0;

    // Added for PR3402
    fInhibitPrompt = 0;

    memset(pfCurrentCommand, 0, sizeof(pfCurrentCommand));

    // Set up the singleton.
    if (pfSingletonInstance != NULL)
    {
        gWarningMsg(fMessageLogSettings, "BcmConsoleThread")
            << "Singleton pointer is not NULL!  There are multiple console threads!  Leaving the singleton pointer alone..." << endl;
    }
    else
    {
        gInfoMsg(fMessageLogSettings, "BcmConsoleThread")
            << "Setting up singleton pointer." << endl;

        pfSingletonInstance = this;
    }
    
    // Register myself with the built-in command table.
    fBuiltInCommands.AddInstance("Console Thread", this);
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmConsoleThread::~BcmConsoleThread()
{
    // Do I delete the tables that I have?  I'm going to say no...they belong to
    // the client who gave them to me.
    pfMainCommandTable = NULL;
    pfActiveCommandTable = NULL;
    pfSavedCommandTable = NULL;

    // Clear the singleton pointer.
    if (pfSingletonInstance == this)
    {
        gInfoMsg(fMessageLogSettings, "~BcmConsoleThread")
            << "Clearing the singleton pointer." << endl;

        pfSingletonInstance = NULL;
    }
    else
    {
        gWarningMsg(fMessageLogSettings, "~BcmConsoleThread")
            << "I'm not the singleton instance!  Leaving the singleton pointer alone..." << endl;
    }
}


// Tells the console thread to stop running (exit the thread).  This call
// blocks until the thread has exited.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmConsoleThread::StopRunning(void)
{
    fTimeToExit = true;

    gInfoMsg(fMessageLogSettings, "StopRunning")
         << "Waiting for the thread to exit." << endl;

    WaitForThread();

    gInfoMsg(fMessageLogSettings, "StopRunning")
         << "Thread exited!" << endl;
}


// Sets the specified command table as the primary "main" table.  It is the
// default active table, in addition to the set of commands provided by the
// console thread itself.
//
// The main application code will generally set this one up.
//
// Parameters:
//      pCommandTable - pointer to the command table to be used.
//
// Returns:
//      The previous command table that was being used as the main table.
//
BcmCommandTable *BcmConsoleThread::SetMainCommandTable(BcmCommandTable *pCommandTable)
{
    BcmCommandTable *pPreviousTable = pfMainCommandTable;

    pfMainCommandTable = pCommandTable;
    pfActiveCommandTable = pCommandTable;

    return pPreviousTable;
}


// Adds the specified command table as a subtable of the specified parent
// table.  If the parent table is NULL, then the subtable is added to the
// main command table.
//
// Objects running in the system will generally set these up.
//
// Parameters:
//      pParentTable - pointer to the table to which the subtable should
//                     be added.
//      pSubtable - the command table to be added.
//
// Returns:
//      true if the subtable was added correctly.
//      false if there was a problem.
//
bool BcmConsoleThread::AddCommandTable(BcmCommandTable *pParentTable,
                                       BcmCommandTable *pSubtable)
{
    if (pSubtable == NULL)
    {
        gErrorMsg(fMessageLogSettings, "AddCommandTable")
             << "Subtable is NULL!  Can't add it!" << endl;

        return false;
    }

    // If the parent pointer is NULL, then default to the main table.
    if (pParentTable == NULL)
    {
        if (pfMainCommandTable == NULL)
        {
            gErrorMsg(fMessageLogSettings, "AddCommandTable")
                 << "Can't add the subtable to the main table (main table is NULL)!" << endl;

            return false;
        }

        pParentTable = pfMainCommandTable;
    }

    gInfoMsg(fMessageLogSettings, "AddCommandTable")
         << "Adding subtable '"
         << pSubtable->FullName() << "' to parent table '"
         << pParentTable->FullName() << "'." << endl;

    // Add this table to the parent table.
    pParentTable->AddSubtable(pSubtable);

    return true;
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
BcmConsoleThread *BcmConsoleThread::GetSingletonInstance(void)
{
    if (pfSingletonInstance == NULL)
    {
        gLogMessageRaw
            << "BcmConsoleThread::GetSingletonInstance:  WARNING - the singleton instance is NULL, and someone is accessing it!" << endl;
    }
    
    return pfSingletonInstance;
}


// Turn the command prompt display on or off.  This is most useful when
// a lot of output is being displayed and you don't want it interrupted by
// the console.
//
// Added for PR3402
//
// Parameters:
//      inhibit - true to turn the display off, false to turn it on.
//
// Returns:  Nothing.
//
void BcmConsoleThread::InhibitPromptDisplay(bool inhibit)
{
    // If inhibit is enabled, then set the timeout; otherwise, clear the
    // counter so we don't inhibit.
    if (inhibit)
    {
        fInhibitPrompt = 30 * kOneSecond / kInhibitSleepTimeMs;
    }
    else
    {
        fInhibitPrompt = 0;
    }
}


// This is what handles the execution of a command once entered.  It has the
// logic to try both the built-in table and the active table, and will
// display various error messages as needed.
//
// It assumes that the command string is already in pfCurrentCommand.
//
// Parameters:  None.
//
// Returns:
//      true if the command was executed.
//      false if the command was not executed.
//
bool BcmConsoleThread::TryCommand(void)
{
    BcmCommandTable::ExecuteResult result = BcmCommandTable::kNotFound;

    char * thisCommand=pfCurrentCommand;
    char * nextCommand;
    unsigned int    charCount;
    
    // Store the existing command table in case we need to temporarily
    // change the active command table
    pfSavedCommandTable = pfActiveCommandTable;

    // Iterate over backquote-delimited commands.
    while (thisCommand != NULL)
    {
        result = BcmCommandTable::kNotFound;
        
        // Find the end of this backquote-delimited command.  Terminate it
        // with a null character and save a pointer to the next command, if
        // one exists.
        nextCommand = strchr( thisCommand, '`' );
        if (nextCommand != NULL)
        {
            *nextCommand = '\0';
            nextCommand++;
        }

        // Skip over leading white-space characters.  This makes it easy to 
        // tell if the user entered a null command between backquotes.
        charCount = strspn( thisCommand, " \t" );
        thisCommand += charCount;

        // Check for root table reference
        while (*thisCommand == '/' || *thisCommand == '\\')
        {
            pfActiveCommandTable = pfMainCommandTable;
            thisCommand++;
        }

        // Look for a / or \ character in the command name, indicating a subtable
        while ((charCount = strcspn(thisCommand, "/\\")) < strcspn(thisCommand, " \t"))
        {
            // Null the separator
            thisCommand[charCount] = '\0';

            if (charCount == 0)
            {
                // Ignore multiple /'s
            }
            // This takes us up one level.
            else if (strcmp(thisCommand, "..") == 0)
            {
                // Don't go above the main table...
                if (pfActiveCommandTable != pfMainCommandTable)
                {
                    pfActiveCommandTable = pfActiveCommandTable->ParentTable();
                }
            }
            // Update our command table or indicate an error
            else if (!pfActiveCommandTable->IsValidSubtable(thisCommand, pfActiveCommandTable))
            {
                gLogMessageRaw
                     << "'" << thisCommand 
                     << "' is not a valid command table.\n\nType 'help' for information about valid commands and tables."
                     << endl;
            
                result = BcmCommandTable::kError;
            }

            // Proceed with remaining command
            thisCommand += (charCount+1);
        }

        
        // See if it's in the built-in table.
        if (result == BcmCommandTable::kNotFound)
        {
            result = fBuiltInCommands.ExecuteCommand(thisCommand);
        }

        // If not, and there is an active table, then see if it's in there.
        if ((result == BcmCommandTable::kNotFound) && 
            (pfActiveCommandTable != NULL))
        {
            BcmCommandTable *pSubtable;

            result = pfActiveCommandTable->ExecuteCommand(thisCommand);

            // If it's not in here, then see if they did something like
            // just typing the name of a subtable.
            if ((result == BcmCommandTable::kNotFound) &&
                (pfActiveCommandTable->IsValidSubtable(thisCommand,
                                                       pSubtable) == true))
            {
                gLogMessageRaw
                     << "'" << pSubtable->Name() 
                     << "' is a command table.\n\nType 'help " 
                     << pSubtable->Name() 
                     << "' for information about the table, or\n'cd "
                     << pSubtable->Name() << "' to change to that table.\n";
            }
        }

        // Return to saved command table
        pfActiveCommandTable = pfSavedCommandTable;

        // Break if the command wasn't successful.  In case the user typed two 
        // backquotes (cd non-vol``ls), don't break on an empty string.
        if ((*thisCommand != '\0') && 
            (result != BcmCommandTable::kFound))
            break;

        // Loop again to see if there is another command.
        thisCommand = nextCommand;
    }

    // If the command was not processed, then complain about it.  If it was
    // found, or if there was a different kind of error, then it has already
    // been complained about.
    if (result == BcmCommandTable::kNotFound)
    {
        gLogMessageRaw << "Error - Unknown command:  '"
             << thisCommand << "'" << endl;
    }

    // Return the correct result code.
    if (result == BcmCommandTable::kFound)
    {
        return true;
    }
    return false;
}


// Utility method to help with printing the path from the main table to
// the specified table.  It uses recursion to start printing at the main
// table, then print each successive subtable from left to right.
//
// Parameters:
//      outputStream - the ostream to which the table names should be printed.
//      pTable - the starting point for this level of recursion.
//
// Returns:  Nothing.
//
void BcmConsoleThread::PrintTablePath(ostream &outputStream, BcmCommandTable *pTable)
{
    // If this isn't the main table, then call myself giving the parent table.
    // I also need to print an arrow to show that the parent table goes to my
    // table.
    if (pTable != pfMainCommandTable)
    {
        if( pTable->ParentTable() )
        {
            PrintTablePath(outputStream, pTable->ParentTable());
        }

        outputStream << " -> ";
    }

    // Now I show my table name.
    outputStream << pTable->Name();
}


// This is what is called when the user enters a command from the built-in
// table.  It vectors to the appropriate BcmConsoleThread instance method
// from below.
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
void BcmConsoleThread::UserCommandEntrypoint(void *pInstanceValue, const BcmCommand &command)
{
    // Cast the void parameter to a BcmConsoleThread instance.
    BcmConsoleThread *pConsoleThread = (BcmConsoleThread *) pInstanceValue;
    static unsigned long gLastTimeMs = 0;

    // Check the command id and vector to the correct instance method.
    switch (command.fCommandId)
    {
        case BcmBuiltInCommandTable::kHelpCommand:
        case BcmBuiltInCommandTable::kQuestionCommand:
        case BcmBuiltInCommandTable::kLsCommand:
        case BcmBuiltInCommandTable::kDirCommand:
        case BcmBuiltInCommandTable::kManCommand:
            // These are identical.
            pConsoleThread->ShowHelp(command);
            break;

        case BcmBuiltInCommandTable::kBangCommand:
            pConsoleThread->BangCommand(command);
            break;

        case BcmBuiltInCommandTable::kChangeActiveTable:
            pConsoleThread->ChangeActiveTable(command);
            break;

        case BcmBuiltInCommandTable::kPwdCommand:
            pConsoleThread->PwdCommand();
            break;

        case BcmBuiltInCommandTable::kShowCommandHistory:
            pConsoleThread->ShowCommandHistory(command);
            break;

        case BcmBuiltInCommandTable::kShowInstances:
            pConsoleThread->ShowInstances(command);
            break;

        case BcmBuiltInCommandTable::kShowCurrentTime:
            {
                unsigned long currentTimeMs = SystemTimeMS();
                unsigned long elapsedTimeMs;

                // Calculate the elapsed time since the last query, and store
                // the current time for the next query.
                elapsedTimeMs = currentTimeMs - gLastTimeMs;
                gLastTimeMs = currentTimeMs;

                gLogMessageRaw << "The current system millisecond tick counter is " 
                    << (void *) currentTimeMs << " (" << currentTimeMs << ")\n"
                    << "Time since last query is " << elapsedTimeMs << " ms." << endl;
            }
            break;

        case BcmBuiltInCommandTable::kShowUsage:
            gLogMessageRaw <<
               //1                                                                              | <-80
                "The V2 console works very much like a DOS or Unix command line interface.  It\n"
                "allows you to enter commands to the system, controlling various objects.  The\n"
                "console supports the following features:\n\n"
                
                "Subdirectories - The commands are grouped into DOS-like subdirectories, where\n"
                "    each subdir is associated with a subsystem in the application.  Use the\n"
                "    'cd' command to change the current dir.  There are a group of commands that\n"
                "    are always available regardless of what subdir you are in.  Use the 'help'\n"
                "    command to see the commands that are available for the subdir.\n\n"
                
                "Multiple instance support - Each command is executed for an instance of an\n"
                "    object in the system.  Often, there is only one instance, but in some cases\n"
                "    there can be multiple instances.  When executing a command, you can select\n"
                "    all, one, or a set of instances using special command format.  Type\n"
                "    'help -u' for the command format to select instances.\n\n"
                
                "Command name expansion - When you type part of a command name and hit space,\n"
                "    tab, or enter, the parser will automatically expand the partial name out to\n"
                "    its full name.  This helps clarify exactly which command will be executed,\n"
                "    especially when there are multiple commands that match the partial string.\n"
                "    If there are no commands that match the partial string, then the console\n"
                "    will beep at you, giving feedback that the command is not valid.\n\n"
                
                "Special key handling - The V2 console supports the following special keys:\n"
                "    Left/Right Arrow, Home/End, Ctl-Left, Ctl-Right Arrow:  These move the\n"
                "        cursor, changing the point at which new characters will be inserted.\n"
                "        The console does not support overwriting, only insertion.\n"
                "    Up/Down Arrow:  These scroll back/forward through the command history,\n"
                "        showing commands that had previously been executed.\n"
                "    Del:  Deletes the character at the cursor.\n"
                "    Backspace:  Deletes the character before the cursor, moving the cursor back\n"
                "        one space.\n"
                "    Esc:  Aborts the command that is currently being entered.\n"
                "    ?:  When entering a command, pressing '?' will show you the help for that\n"
                "        command, including parameters.\n\n"

                "For ANSI or VT100 type terminal emulators (CRT, HyperTerm, etc.), you may need\n"
                "to configure the keyboard mapping in order for some of the special keys to be\n"
                "handled properly:\n"
                "  Esc:  0x1b                      Del:  0x7f                Backspace:  0x08\n"
                "  Up Arrow:        0x1b 5b 41 'Esc[A'    Down Arrow:      0x1b 5b 42 'Esc[B'\n"
                "  Right Arrow:     0x1b 5b 43 'Esc[C'    Left Arrow:      0x1b 5b 44 'Esc[D'\n"
                "  Home:            0x1b 5b 45 'Esc[E'    End:             0x1b 5b 46 'Esc[F'\n"
                "  Ctl-Left Arrow:  0x1b 5b 47 'Esc[G'    Ctl-Right Arrow: 0x1b 5b 48 'Esc[H'"
                << endl;
            break;

        case BcmBuiltInCommandTable::kFindCommand:
            pConsoleThread->FindCommand(command);
            break;

        case BcmBuiltInCommandTable::kSleepCommand:
            {
                // Retrieve the parameter.
                const BcmCommandParameter *pParameter = command.pfCommandParameterList->Find(BcmBuiltInCommandTable::kPrimaryParameter);

                // Just sleep for the specified number of milliseconds.
                BcmOperatingSystemFactory::ThreadSleep(pParameter->AsUnsignedInt());
            }
            break;

        case BcmBuiltInCommandTable::kRemarkCommand:
            // Do nothing.
            break;

        case BcmBuiltInCommandTable::kSyntaxCommand:
            pConsoleThread->fBuiltInCommands.PrintUsage(*pgMessageLogStream);
            break;

        default:
            pConsoleThread->UnknownCommand(command.fCommandId);
            break;
    }
}


// These are the command functions that handle all of the built-in commands.
//
// This one displays the available commands in various formats, along with
// other optional information.
//
// Parameters:
//      command - the command object.
//
// Returns:  Nothing.
//
void BcmConsoleThread::ShowHelp(const BcmCommand &command)
{
    const BcmCommandParameter *pParameter = command.pfCommandParameterList->Find(BcmBuiltInCommandTable::kPrimaryParameter);
    const BcmCommandParameter *sParameter = command.pfCommandParameterList->Find(BcmBuiltInCommandTable::kSecondaryParameter);

    // If any of this has any hope of doing anything at all, then there must be
    // a valid instance of the message logging ostream around.  If not, then the
    // log messages must be discarded, so this code won't do anything.  May as
    // well bail out now.
    if (pgMessageLogStream == NULL)
    {
        return;
    }

    if (pParameter->WasParsed())
    {
        switch (pParameter->AsUnsignedInt())
        {
        case (BcmBuiltInCommandTable::kDashT): // help -t
            // Do table info and commands (recursively) for the built-in table.
            fBuiltInCommands.Print(*pgMessageLogStream, BcmCommandTable::kTableInfoAndCommandsRecursive);
            
            if (pfMainCommandTable != NULL)
            {
                // Do the same for the main table (recursively).
                pfMainCommandTable->Print(*pgMessageLogStream, BcmCommandTable::kTableInfoAndCommandsRecursive);
            }
            break;
        case (BcmBuiltInCommandTable::kDashL): // help -l
            // Only print the commands from the built-in table.
            fBuiltInCommands.Print(*pgMessageLogStream, BcmCommandTable::kCommandsAndHelp);
            
            if (pfActiveCommandTable != NULL)
            {
                // Print the commands.
                pfActiveCommandTable->Print(*pgMessageLogStream, BcmCommandTable::kCommandsAndHelp);
                
                // Print the subtables.
                pfActiveCommandTable->PrintSubtables(*pgMessageLogStream, BcmCommandTable::kFull);
            }
            break;
        case (BcmBuiltInCommandTable::kDashLR): // help -lr
            // Only print the commands from the built-in table.
            fBuiltInCommands.Print(*pgMessageLogStream, BcmCommandTable::kCommandsAndHelpRecursive);
            
            if (pfActiveCommandTable != NULL)
            {
                // Print the commands.
                pfActiveCommandTable->Print(*pgMessageLogStream, BcmCommandTable::kCommandsAndHelpRecursive);
            }
            break;
        case (BcmBuiltInCommandTable::kDashS): // help -s
            if (pfActiveCommandTable != NULL)
            {
                // Print the subtables.
                pfActiveCommandTable->PrintSubtables(*pgMessageLogStream, BcmCommandTable::kFull);
            }
            break;
        case (BcmBuiltInCommandTable::kDashI): // help -i
            // This is the same as the show instances command.  Just vector there
            // to let it do the work.
            ShowInstances(command);
            break;
        case (BcmBuiltInCommandTable::kDashA): // help -a
            // Only print the commands from the built-in table.
            fBuiltInCommands.Print(*pgMessageLogStream, BcmCommandTable::kCommandsAndHelp);

            if (pfActiveCommandTable != NULL)
            {
                // Print the commands.
                pfActiveCommandTable->Print(*pgMessageLogStream, BcmCommandTable::kCommandsAndHelp);
                
                // Print the subtables.
                pfActiveCommandTable->PrintSubtables(*pgMessageLogStream, BcmCommandTable::kFull);
            }

            // Now do the "-i" behavior.  This is somewhat sneaky.
            // The help command is defined with a string parameter as the secondary parameter.
            // When we pass this command to ShowInstances, it works because ShowInstances 
            // expects to find a string as the "secondary" parameter.
            ShowInstances(command);
            break;
        }
    }
    else if (sParameter->WasParsed())
    {
        const char * parameterString = sParameter->AsString();
        ShowHelp( parameterString );
    }
    // If there are no parameters, then do a basic command listing for the
    // built-in and active tables.  Otherwise, show detailed information about
    // the command that was entered.
    else
    {
        // Only print the commands from the built-in table.
        fBuiltInCommands.Print(*pgMessageLogStream, BcmCommandTable::kCommandsOnly);

        if (pfActiveCommandTable != NULL)
        {
            // Print the commands.
            pfActiveCommandTable->Print(*pgMessageLogStream, BcmCommandTable::kCommandsOnly);

            // Print the subtables.
            pfActiveCommandTable->PrintSubtables(*pgMessageLogStream, BcmCommandTable::kAbbreviated);
        }
    }
}


// This displays help on a specific command or series of commands.
//
// Parameters:
//      parameterString - the list of commands.
//
// Returns:  Nothing.
//
void BcmConsoleThread::ShowHelp( const char * parameterString)
{
    char parameter[128];
    int nextParameterOffset, numScanned;
    unsigned int commandId;
    BcmCommandTable *pCommandTable;

    numScanned = sscanf(parameterString, "%s%n", parameter, &nextParameterOffset);

    while (numScanned > 0)
    {
        // If this is a built-in command, then print the help from there.
        // Otherwise, try from the active command table.
        if (fBuiltInCommands.IsValidCommand(parameter, commandId))
        {
            fBuiltInCommands.PrintCommand(*pgMessageLogStream, commandId);
        }
        else if ((pfActiveCommandTable != NULL) &&
            (pfActiveCommandTable->IsValidCommand(parameter, commandId)))
        {
            pfActiveCommandTable->PrintCommand(*pgMessageLogStream, commandId);
        }

        // Check for this as a subtable...
        else if ((pfActiveCommandTable != NULL) &&
            (pfActiveCommandTable->IsValidSubtable(parameter, pCommandTable)))
        {
            pCommandTable->Print(*pgMessageLogStream, BcmCommandTable::kCommandTableInfo);
        }

        else
        {
            gLogMessageRaw
                << "Help:  Specified command ('" << parameter << "') was not found!  No help available..." << endl;
        }

        gLogMessageRaw
            << "\n" << setfill('-') << setw(77) << "-\n\n" << setfill(' ');

        // Get the next parameter.
        parameterString += nextParameterOffset;
        numScanned = sscanf(parameterString, "%s%n", 
            parameter, &nextParameterOffset);
    }
}

// This handles execution of one of the commands stored in the history, based
// on the parameters that are specified.
//
// Parameters:
//      command - the command object.
//
// Returns:  Nothing.
//
void BcmConsoleThread::BangCommand(const BcmCommand &command)
{
    const BcmCommandParameter *pParameter = command.pfCommandParameterList->Find(BcmBuiltInCommandTable::kPrimaryParameter);
    const BcmCommandParameter *sParameter = command.pfCommandParameterList->Find(BcmBuiltInCommandTable::kSecondaryParameter);

    unsigned int commandToExecute, commandNumber = 0;
    bool matchFound = false, usedCommandNumber = false;
    const char * pCommandString;

    if (sParameter->WasParsed())
    {
        pCommandString = sParameter->AsString();
    }
    else
    {
        pCommandString = "";
    }

    // See if the user specified a number.  If so, then try to execute that
    // command from the history.
    if (pParameter->WasParsed())
    {
        unsigned int currentCommandNumber;

        commandNumber = pParameter->AsUnsignedInt();
        usedCommandNumber = true;

        // Find the command index corresponding to that number.
        commandToExecute = fNextCommandIndex;

        currentCommandNumber = 0;

        do
        {
            // If this command field is empty, then skip it.
            if (strlen(pfCommandHistory[commandToExecute]) > 0)
            {
                // If this is the command number we are looking for, then break
                // out.
                if (currentCommandNumber == commandNumber)
                {
                    matchFound = true;

                    break;
                }

                // Otherwise, keep going.
                currentCommandNumber++;
            }

            // Go to the next command.
            commandToExecute++;
            if (commandToExecute >= kMaxCommands)
            {
                commandToExecute = 0;
            }

        } while (commandToExecute != fNextCommandIndex);
    }
    else
    {
        unsigned int startCommandNumber;
        // Calculate the index of the last command that was stored.
        if (fNextCommandIndex == 0)
        {
            startCommandNumber = kMaxCommands - 1;
        }
        else
        {
            startCommandNumber = fNextCommandIndex - 1;
        }
        commandToExecute = startCommandNumber;

        // Find the first command in the history that matches.  This works even if
        // no parameter was specified (it will select the last command that was
        // stored).
        do
        {
            // If the current command is empty, then skip it.
            if (strlen(pfCommandHistory[commandToExecute]) != 0)
            {
                // See if the partial command matches what's in the history.
                if (strncmp(pfCommandHistory[commandToExecute], pCommandString, strlen(pCommandString)) == 0)
                {
                    matchFound = true;
                    break;
                }
            }

            // Calculate the next command index.
            if (commandToExecute == 0)
            {
                commandToExecute = kMaxCommands - 1;
            }
            else
            {
                commandToExecute -= 1;
            }
        } while (commandToExecute != startCommandNumber);
    }
    
    if (matchFound == false)
    {
        if (usedCommandNumber == true)
        {
            gLogMessageRaw
                 << "There is no command at index " << commandNumber << " in the history!" << endl;
        }
        else
        {
            gLogMessageRaw
                 << "There is no command matching '" << pCommandString << "' in the history!" << endl;
        }

        return;
    }

    // Display the command before executing it.
    gLogMessageRaw
         << pfCommandHistory[commandToExecute] << "\n" << endl;

    // Copy it to the current command buffer so that it can be modified without
    // corrupting the history.
    strcpy(pfCurrentCommand, pfCommandHistory[commandToExecute]);

    // Call the routine that handles execution of the command.
    TryCommand();
}


// This handles changing and/or showing the active command table.
//
// Parameters:
//      command - the command object.
//
// Returns:  Nothing.
//
void BcmConsoleThread::ChangeActiveTable(const BcmCommand &command)
{
    const BcmCommandParameter *pParameter = command.pfCommandParameterList->Find(BcmBuiltInCommandTable::kPrimaryParameter);
    char parameter[128];

    // Some quick error checking.
    if (pfActiveCommandTable == NULL)
    {
        gErrorMsg(fMessageLogSettings, "ChangeActiveTable")
             << "There is no active table!  Can't change tables..." << endl;

        return;
    }

    // If no parameters are specified, then this is the same as the Pwd command.
    if (pParameter->WasParsed())
    {
        int offset = 0;
        char *pToken;

        strncpy( parameter, pParameter->AsString(), 127);
        parameter[127] = '\0';

        // These take us to the main table.
        if ((parameter[0] == '/') || (parameter[0] == '\\'))
        {
            pfActiveCommandTable = pfMainCommandTable;
            offset = 1;
        }

        // From here on, / or \ is a delimiter between command tables.
        pToken = strtok(parameter + offset, "/\\ \r\n\t");

        while (pToken != NULL)
        {
            // This takes us up one level.
            if (strcmp(pToken, "..") == 0)
            {
                // Don't go above the main table...
                if (pfActiveCommandTable != pfMainCommandTable)
                {
                    pfActiveCommandTable = pfActiveCommandTable->ParentTable();
                }
            }

            // Anything else is assumed to be a subtable name, so try to make it the
            // active one.
            else
            {
                bool result;
                BcmCommandTable *pSubtable;

                result = pfActiveCommandTable->IsValidSubtable(pToken, pSubtable);

                if (result == true)
                {
                    // Set the parent command table before entering the subtable.  In this way
                    // a commad table can have multiple parents. 
                    pSubtable->SetParentTable( pfActiveCommandTable );
                    pfActiveCommandTable = pSubtable;
                }
                else
                {
                    gLogMessageRaw
                         << "Specified subtable ('" << pToken << "') doesn't exist!  Can't change to it...\n" << endl;

                    break;
                }
            }
            
            pToken = strtok(NULL, "/\\ \r\n\t");
        }
    }

    // Update the saved command table as well
    pfSavedCommandTable = pfActiveCommandTable;

    // Show the new active command table.
    PwdCommand();
}


// This shows the active command table, along with the path from the current
// table to the main (root) table.
//
// Parameters: None
//
// Returns:  Nothing.
//
void BcmConsoleThread::PwdCommand(void)
{
    gLogMessageRaw << "Active Command Table:  ";

    if (pfActiveCommandTable != NULL)
    {
        // Print the full name of the table.
        gLogMessageRaw << pfActiveCommandTable->FullName() << " (" << pfActiveCommandTable->Name() << ")\n\n";

        // Show the path from the main table that got us here.
        if (pgMessageLogStream != NULL)
        {
            PrintTablePath(*pgMessageLogStream, pfActiveCommandTable);
        }
    }
    else
    {
        gLogMessageRaw << "{none}";
    }

    gLogMessageRaw << endl;
}


// Lists all of the commands that are currently in the command history.
//
// Parameters:
//      command - the command object.  Not used because there are no parameters.
//
// Returns:  Nothing.
//
void BcmConsoleThread::ShowCommandHistory(const BcmCommand & /*command*/)
{
    unsigned int index, historyNumber;

    gLogMessageRaw << "Command History:\n\n";

    // Start just after the last command that was entered.
    index = fNextCommandIndex;
    historyNumber = 0;

    do 
    {
        // Don't print the entry if it doesn't have a command in it.
        if (strlen(pfCommandHistory[index]) > 0)
        {
            gLogMessageRaw
                 << historyNumber << ")  " << pfCommandHistory[index] << "\n";
        
            historyNumber++;
        }

        // Go to the next command, wrapping around if necessary.
        index++;
        if (index >= kMaxCommands)
        {
            index = 0;
        }
    } while (index != fNextCommandIndex);
}


// Shows the object instances that are registered with the current command
// table.  This is similar to "help -i".
//
// Parameters:
//      command - the command object.
//
// Returns:  Nothing.
//
void BcmConsoleThread::ShowInstances(const BcmCommand &command)
{
    const BcmCommandParameter *pParameter = command.pfCommandParameterList->Find(BcmBuiltInCommandTable::kSecondaryParameter);
    const char * parameters = "";

    // See if there was a parameter.
    if (pParameter->WasParsed())
    {
        parameters = pParameter->AsString();
    }

    if (pfActiveCommandTable != NULL)
    {
        if (pgMessageLogStream != NULL)
        {
            pfActiveCommandTable->PrintInstances(*pgMessageLogStream, parameters);
        }
    }
    else
    {
        gLogMessageRaw << "There is no active table!" << endl;
    }
}


void BcmConsoleThread::FindCommand (const BcmCommand &command)
{
    const BcmCommandParameter *pParameter = command.pfCommandParameterList->Find(BcmBuiltInCommandTable::kPrimaryParameter);

    if (pfActiveCommandTable == NULL)
    {
        gErrorMsg(fMessageLogSettings, "FindCommand")
            << "There is no active table!" << endl;
        
        return;
    }

    if (pParameter->WasParsed())
    {
        bool found;
        found  = fBuiltInCommands.ListMatchingCommands     (pParameter->AsString(), false);
        found |= pfActiveCommandTable->ListMatchingCommands (pParameter->AsString(), true);
        if ( !found )
        {
            gLogMessageRaw << pParameter->AsString() << " not found" << endl;
        }
    }
}


// Just prints a message that the specified command is unknown.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmConsoleThread::UnknownCommand(unsigned int commandId)
{
    gErrorMsg(fMessageLogSettings, "UnknownCommand") << "Unknown command id:  " << commandId << endl;
}


// Thread constructor - this is the first method called after the thread has
// been spawned, and is where the thread should create all OS objects.  This
// has to be done here, rather than in the object's constructor, because
// some OS objects must be created in the context of the thread that will
// use them.  The object's constructor is still running in the context of
// the thread that created this object.
//
// Parameters:  None.
//
// Returns:
//      true if successful and ThreadMain() should be called.
//      false if there was a problem (couldn't create an OS object, etc.)
//          and ThreadMain() should not be called.
//
bool BcmConsoleThread::Initialize(void)
{
    return true;
}


// This is the main body of the thread's code.  This is the only method
// that absolutely must be provided in the derived class (since there is no
// reasonable default behavior).
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmConsoleThread::ThreadMain(void)
{
    unsigned int commandId;
    char         promptString[256];
    char *       pThisCommand;

    gLogMessageRaw << "\nType 'help' or '?' for a list of commands...\n\n";

    // Keep going until I'm told to exit.
    while (!fTimeToExit)
    {
        // If we are being told to stop displaying the prompt, then yield the
        // processor for a bit then loop around again.  We also decrement the
        // counter so that we don't stay in this state forever.
        // Added for PR3402
        if (fInhibitPrompt > 0)
        {
            fInhibitPrompt--;
            pfOperatingSystem->Sleep(kInhibitSleepTimeMs);
            continue;
        }

        // Figure out which prompt I want to present to the user.
        if (pfActiveCommandTable != NULL)
        {
            pfActiveCommandTable->CommandPromptWithParents(promptString, sizeof(promptString), "/");
            //pPromptString = pfActiveCommandTable->CommandPrompt();
        }
        else
        {
            fBuiltInCommands.CommandPromptWithParents(promptString, sizeof(promptString), "/");
            //pPromptString = fBuiltInCommands.CommandPrompt();
        }

        strncat(promptString, "> ", sizeof(promptString) - 1 - strlen(promptString));

        // Call the derived class to get a line of input.  Allow command
        // completion to work.
        GetCommandString(promptString, pfCurrentCommand, kMaxCommandLength, true);

        // If I returned because I'm being told to exit, then don't try to parse
        // the command.
        if (fTimeToExit)
        {
            break;
        }

        gInfoMsg(fMessageLogSettings, "ThreadMain")
             << "User entered command:\n\t" << pfCurrentCommand << endl << endl;

        pThisCommand = pfCurrentCommand;

        // Skip leading white space.
        pThisCommand += strspn( pThisCommand, " \t\n" );

        // If there was no command, don't do anything.
        if (strlen(pThisCommand) == 0)
        {
            continue;
        }

        // Push this command onto the history list.  Note that I don't want to
        // add the 'history' or '!' commands to the history list.
        if (!fBuiltInCommands.IsValidCommand(pThisCommand, commandId))
        {
            // If this isn't in the built-in table, then set the command id to
            // something innocuous.
            commandId = BcmBuiltInCommandTable::kHelpCommand;
        }

        if ((commandId != BcmBuiltInCommandTable::kBangCommand) && 
            (commandId != BcmBuiltInCommandTable::kShowCommandHistory))
        {
            // See if this is the same command that we ran last time; if so,
            // don't add it to the history again (there's no point in filling
            // the history with the same command).
            unsigned int prevCommandIndex;

            if (fNextCommandIndex == 0)
            {
                prevCommandIndex = kMaxCommands - 1;
            }
            else
            {
                prevCommandIndex = fNextCommandIndex - 1;
            }

            if (strcmp(pfCommandHistory[prevCommandIndex], pThisCommand) != 0)
            {
                // Copy the command into the next available slot.
                strcpy(pfCommandHistory[fNextCommandIndex], pThisCommand);

                // Go to the next history entry, wrapping to 0 if necessary.
                fNextCommandIndex++;
                if (fNextCommandIndex >= kMaxCommands)
                {
                    fNextCommandIndex = 0;
                }
            }
        }

        gLogMessageRaw << "\n";

        // Call the routine that tries to execute the command.
        TryCommand();

        // If we are displaying the prompt, then add a blank line.
        // Added for PR3402
        if (fInhibitPrompt == 0)
        {
            gLogMessageRaw << "\n";
        }
    }

    gFuncEntryExitMsg(fMessageLogSettings, "ThreadMain") << "Exiting..." << endl;
}


// Thread destructor - this is the last method called when the thread is
// exiting, and is where the thread should delete all of the OS objects that
// it created.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmConsoleThread::Deinitialize(void)
{
}



