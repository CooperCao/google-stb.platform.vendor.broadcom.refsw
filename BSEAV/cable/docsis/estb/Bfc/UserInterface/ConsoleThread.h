//****************************************************************************
//
//  Copyright (c) 2000-2004  Broadcom Corporation
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
//  $Id: ConsoleThread.h 1.35 2005/09/08 20:53:27Z sdunlap Release $
//
//  Filename:       ConsoleThread.h
//  Author:         David Pullen
//  Creation Date:  Feb 11, 2000
//
//****************************************************************************

#ifndef ConsoleThread_H
#define ConsoleThread_H

//********************** Include Files ***************************************

// My base class.
#include "Thread.h"

// The set of commands that I can handle.
#include "BuiltInCommandTable.h"

#include "IpAddress.h"
#include "MacAddress.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************


/** \ingroup BfcConsole
*
*      This is the abstract base class for all types of methods for gathering
*      input from users, and presenting either "command-line" or "menu" based
*      interfaces.  It allows users to configure various objects in the system.
*
*      All of the command parsing/processing occurs in the base class.  The
*      actual input and output of the command strings or responses are done
*      in the derived classes, who define what methods will be used.
*
*      The console is a stand-alone thread so that it's operation doesn't
*      block anything else in the system.  You will generally want to make this
*      the lowest priority thread.
*
*      In addition, it is a singleton, meaning that there is only one of these
*      in the system, and it is "globally" accessible.
*/
class BcmConsoleThread : public BcmThread
{
public:

    /// Default Constructor.  Initializes the state of the object, starting the
    /// thread.
    ///
    /// \param
    ///      startSuspended - tells whether or not the thread should suspend
    ///                       itself after initialization.
    /// \param
    ///      initialPriority - the priority at which you want the thread to run
    ///                        after initialization.
    ///
    BcmConsoleThread(bool startSuspended = false,
                     BcmOperatingSystem::ThreadPriority initialPriority = BcmOperatingSystem::kIdlePriority);

    /// Destructor.  Frees up any memory/objects allocated, cleans up internal
    /// state.
    ///
    virtual ~BcmConsoleThread();

    /// Tells the console thread to stop running (exit the thread).  This call
    /// blocks until the thread has exited.
    ///
    void StopRunning(void);

    /// This method allows the caller to wait for the user to press a key,
    /// timing out after the specified number of milliseconds if one is not
    /// detected.
    ///
    /// The derived class MUST implement this method.
    ///
    /// \param
    ///      timeoutMS - the number of milliseconds to wait for a keypress.
    /// \param
    ///      charToWaitFor - if you want to wait for a specific character,
    ///                      specify it here.  Otherwise, the default (0) means
    ///                      any character will be accepted.  PR6859
    ///
    /// \retval
    ///      true is a keypress was detected.
    /// \retval
    ///      false if it timed out waiting.
    ///
    virtual bool WaitForKeypress(unsigned int timeoutMS, char charToWaitFor = 0) = 0;

    /// Sets the specified command table as the primary "main" table.  It is the
    /// default active table, in addition to the set of commands provided by the
    /// console thread itself.
    ///
    /// The main application code will generally set this one up.
    ///
    /// \param
    ///      pCommandTable - pointer to the command table to be used.
    ///
    /// \return
    ///      The previous command table that was being used as the main table.
    ///
    BcmCommandTable *SetMainCommandTable(BcmCommandTable *pCommandTable);

    // Sets the command table to be used as the root when a telnet user logs in.
    // By default, this is NULL, and 'normal' command table is available.  If
    // you want a telnet user to have a more restricted set of commands
    // available, you can set up a separate command table that is installed
    // when the client logs in.
    //
    // PR5181
    //
    // Parameters:
    //      pCommandTable - the command table to be installed when a telnet user
    //                      logs in.
    //
    // Returns:  Nothing.
    //
    inline void SetRemoteAccessCommandTable(BcmCommandTable *pCommandTable);

    // When logged in through one of the remote access protocols (telnet/ssh)
    // if the user knows the su command and password, this forces the active
    // command table to the main developer command tables.  This should be 
    // used with care. 
    //
    // Parameters:
    //      enable - enable or disable developer command tables
    //
    // Returns:  Nothing.
    //
    inline void EnableRemoteAccessSuperUser(bool enable);

    // When logged in through one of the remote access protocols (telnet/ssh)
    // returns the state of the super-user login
    //
    // Parameters:
    //
    // Returns:  Nothing.
    //
    inline bool RemoteAccessSuperUserEnabled(void);

    /// Adds the specified command table as a subtable of the specified parent
    /// table.  If the parent table is NULL, then the subtable is added to the
    /// main command table.
    ///
    /// Objects running in the system will generally set these up.
    ///
    /// \param
    ///      pParentTable - pointer to the table to which the subtable should
    ///                     be added.
    /// \param
    ///      pSubtable - the command table to be added.
    ///
    /// \retval
    ///      true if the subtable was added correctly.
    /// \retval
    ///      false if there was a problem.
    ///
    bool AddCommandTable(BcmCommandTable *pParentTable,
                         BcmCommandTable *pSubtable);

    /// Returns the pointer to the singleton instance for this class.  Most
    /// objects in the system will use this method rather than being passed a
    /// pointer to it.  The singleton pointer will be set up in the base-class
    /// constructor.
    ///
    /// \note   This can return NULL if a singleton has not been set up for the
    ///         system, so you must check for this condition.
    ///
    /// \note   You must not delete this object!
    ///
    /// \note   You should not store a pointer to the object that is returned,
    ///         since it may be deleted and replaced with a new one.
    ///
    /// \return
    ///      A pointer to the instance that should be used by the system.
    ///
    static BcmConsoleThread *GetSingletonInstance(void);

    /// Turn the command prompt display on or off.  This is most useful when
    /// a lot of output is being displayed and you don't want it interrupted by
    /// the console.
    ///
    /// Added for PR3402
    ///
    /// \param
    ///      inhibit - true to turn the display off, false to turn it on.
    ///
    void InhibitPromptDisplay(bool inhibit);

public:

    // DPullen, March4, 2001; made this method public so that it can be called
    // by BcmCommandParameter.  I am in the process of removing the GetParameter
    // and ParseParameter methods from this class, and moving this to the
    // BcmCommandParameter classes, primarily for code shrink.

    /// This is what the thread main calls in order to get a line of input.  The
    /// derived class uses its particular IO method to do the work.
    ///
    /// This method MUST be provided by the derived class.
    ///
    /// \param
    ///      pCommandPrompt - The prompt to be displayed.
    /// \param
    ///      pCommandString - pointer to the string into which the command is to
    ///                       be stored.
    /// \param
    ///      stringSize - the maximum number of bytes that the string can hold.
    /// \param
    ///      allowCommandCompletion - set this to true if you want command
    ///                               completion to operate; it is disabled by
    ///                               default.
    ///
    virtual void GetCommandString(const char *pCommandPrompt, 
                                  char *pCommandString, 
                                  unsigned int stringSize, 
                                  bool allowCommandCompletion = false) = 0;

    /// Run a script of console commands.  This allows a script file to be
    /// downloaded and executed.
    ///
    /// \param
    ///      pScript - buffer containing commands to be executed. The buffer
    ///                becomes owned by the ConsoleThread and will be deleted
    ///                when no longer required, so it must be allocated with new.
    /// \param
    ///      synchronous - if true, this method does not return until script
    ///                execution is complete
    ///
    virtual void RunScript(BcmOctetBuffer *pScript, bool synchronous = true) = 0;

protected:

    /// This flag is used to signal when it is time to exit the thread.
    bool fTimeToExit;

    /// The set of commands that are always available with every command table.
    BcmBuiltInCommandTable fBuiltInCommands;

    /// The main command table that serves as the root for all subtables.
    BcmCommandTable *pfMainCommandTable;

    /// The command table that is currently active.  This might be the main
    /// command table, or it might be a subtable.
    BcmCommandTable *pfActiveCommandTable;

    /// The active command table may change during processing of a subtable
    /// command. The command table to return to after the command is complete
    /// is stored here.
    BcmCommandTable *pfSavedCommandTable;

    // This is the command table that will be used when a telnet/ssh user
    // logs in.
    BcmCommandTable *pfRemoteAccessCommandTable;

    // determines whether the remote access user is a super-user
    bool pfEnableRemoteAccessSuperUser;

    enum
    {
        /// The number of commands that the history will store.
        kMaxCommands = 16,

        /// The maximum length of a command string that can be stored.
        kMaxCommandLength = 256
    };

    /// The last N commands that were executed.
    char pfCommandHistory[kMaxCommands][kMaxCommandLength];
    unsigned int fNextCommandIndex;

    /// Keeps the console prompt from being displayed.  This is most helpful when
    /// a lot of output is going to be printed, and we don't want it interrupted
    /// by the console.
    /// Added for PR3402
    unsigned int fInhibitPrompt;

private:
    
    /// The command that the user just entered.
    char pfCurrentCommand[kMaxCommandLength];

    /// This is the pointer to the singleton instance.  It is set up by the
    /// constructor.
    static BcmConsoleThread *pfSingletonInstance;

private:

    /// This is what handles the execution of a command once entered.  It has the
    /// logic to try both the built-in table and the active table, and will
    /// display various error messages as needed.
    ///
    /// It assumes that the command string is already in pfCurrentCommand.
    ///
    /// \retval
    ///      true if the command was executed.
    /// \retval
    ///      false if the command was not executed.
    ///
    bool TryCommand(void);

    /// Utility method to help with printing the path from the main table to
    /// the specified table.  It uses recursion to start printing at the main
    /// table, then print each successive subtable from left to right.
    ///
    /// \param
    ///      outputStream - the ostream to which the table names should be printed.
    /// \param
    ///      pTable - the starting point for this level of recursion.
    ///
    void PrintTablePath(ostream &outputStream, BcmCommandTable *pTable);

    /// This is what is called when the user enters a command from the built-in
    /// table.  It vectors to the appropriate BcmConsoleThread instance method
    /// from below.
    ///
    /// \param
    ///      pInstanceValue - the instance value to which the command should be
    ///                       applied.  Instances must be registered with the
    ///                       command table in order for them to receive the
    ///                       command.
    /// \param
    ///      command - the command (with parameters) that was entered by the user.
    ///
    static void UserCommandEntrypoint(void *pInstanceValue, const BcmCommand &command);

protected:

    /// This displays help on a specific command or series of commands.
    ///
    /// \param
    ///      parameterString - the list of commands.
    ///
    void ShowHelp(const char *parameterString);

private:

    // These are the command functions that handle all of the built-in commands.

    /// This one displays the available commands in various formats, along with
    /// other optional information.
    ///
    /// \param
    ///      command - the command object.
    ///
    void ShowHelp(const BcmCommand &command);

    /// This handles execution of one of the commands stored in the history, based
    /// on the parameters that are specified.
    ///
    /// \param
    ///      command - the command object.
    ///
    void BangCommand(const BcmCommand &command);
    
    /// This handles changing and/or showing the active command table.
    ///
    /// \param
    ///      command - the command object.
    ///
    void ChangeActiveTable(const BcmCommand &command);

    /// This shows the active command table, along with the path from the current
    /// table to the main (root) table.
    ///
    void PwdCommand(void);

    /// Lists all of the commands that are currently in the command history.
    ///
    /// \param
    ///      command - the command object.
    ///
    void ShowCommandHistory(const BcmCommand &command);

    /// Shows the object instances that are registered with the current command
    /// table.  This is similar to "help -i".
    ///
    /// \param
    ///      command - the command object.
    ///
    void ShowInstances(const BcmCommand &command);

    /// Iterates over all of the command tables, starting with the currently
    /// active command table (including the built-in command table), and gets
    /// each command table to list the available commands that match the one
    /// specified as the parameter of the command.
    ///
    /// \param
    ///     command - the command object.
    ///
    void FindCommand(const BcmCommand &command);

    /// Just prints a message that the specified command is unknown.
    ///
    /// \param
    ///      commandId - the id of the unknown command.
    ///
    void UnknownCommand(unsigned int commandId);

protected:

    /// Thread constructor - this is the first method called after the thread has
    /// been spawned, and is where the thread should create all OS objects.  This
    /// has to be done here, rather than in the object's constructor, because
    /// some OS objects must be created in the context of the thread that will
    /// use them.  The object's constructor is still running in the context of
    /// the thread that created this object.
    ///
    /// \retval
    ///      true if successful and ThreadMain() should be called.
    /// \retval
    ///      false if there was a problem (couldn't create an OS object, etc.)
    ///          and ThreadMain() should not be called.
    ///
    virtual bool Initialize(void);

    /// This is the main body of the thread's code.  This is the only method
    /// that absolutely must be provided in the derived class (since there is no
    /// reasonable default behavior).
    ///
    virtual void ThreadMain(void);

    /// Thread destructor - this is the last method called when the thread is
    /// exiting, and is where the thread should delete all of the OS objects that
    /// it created.
    ///
    virtual void Deinitialize(void);

private:

    /// Copy Constructor.  Not supported.
    BcmConsoleThread(const BcmConsoleThread &otherInstance);

    /// Assignment operator.  Not supported.
    BcmConsoleThread & operator = (const BcmConsoleThread &otherInstance);

};


//********************** Inline Method Implementations ***********************
// Sets the command table to be used as the root when a telnet/ssh user logs in.
// By default, this is NULL, and 'normal' command table is available.  If
// you want a telnet/ssh user to have a more restricted set of commands
// available, you can set up a separate command table that is installed
// when the client logs in.
//
// PR5181
//
// Parameters:
//      pCommandTable - the command table to be installed when a telnet user
//                      logs in.
//
// Returns:  Nothing.
//
inline void BcmConsoleThread::SetRemoteAccessCommandTable(BcmCommandTable *pCommandTable)
{
    pfRemoteAccessCommandTable = pCommandTable;
}

// When logged in through one of the remote access protocols (telnet/ssh)
// if the user knows the su command and password, this forces the active
// command table to the main developer command tables.  This should be 
// used with care. 
//
// Parameters:
//      enable - enable or disable developer command tables
//
// Returns:  Nothing.
//
inline void BcmConsoleThread::EnableRemoteAccessSuperUser(bool enable)
{
    pfEnableRemoteAccessSuperUser = enable;
}

// When logged in through one of the remote access protocols (telnet/ssh)
// returns the state of the super-user login
//
// Parameters:
//
// Returns:  Nothing.
//
inline bool BcmConsoleThread::RemoteAccessSuperUserEnabled(void)
{
    return pfEnableRemoteAccessSuperUser;
}

// Special doxygen documentation tag for the Console component.
/** \defgroup BfcConsole BFC Console Component

The BFC Console is an optional component in any BFC System.  The console 
generally provides a TTY-like input mechanism for gathering input from the 
user, usually in a character at a time mode, though it can operate in line 
at a time mode depending on the implementation that is selected.  

The console can gather input from any number of sources:  a serial port, a 
telnet session, a locally-connected keyboard, etc.  It is very useful for 
debugging the system, but it introduces a security risk (allowing hackers 
to gain access to the system), and it increases the code size 
significantly (largely because of the text strings associated with the 
console commands).  It is common to enable the console for debugging, but 
to remove it when deploying the product.  

The console provides a number of commands that the user can execute.  The 
commands are organized into 'directories', where each directory contains a 
related group of commands.  Each directory is represented by a command 
table (BcmCommandTable); just as in DOS or Unix, there can be 
subdirectories, which are command tables that are registered as sub-tables 
under some parent command table.  

Each command is represented by a BcmCommand object; each command can 
contain 0 or more command parameters, represented by BcmCommandParameter 
objects.  There are different types of command parameters to handle 
different data formats; for example, BcmUnsignedIntCommandParameter 
handles unsigned integers, and BcmIpAddressCommandParameter handles IP 
addresses.  

For basic console support, there is a group of built-in commands which are 
always available.  These commands handle things like changing the current 
directory (cd), showing the current directory (pwd), showing command help 
(help, man, ls, dir, etc), plus additional helper commands for scripting 
like displaying the current system time (system_time) and delaying the 
console thread before continuing with the next command (sleep).  

The main command table must be created and registered by some system-level 
startup code.  In BFC, this is done by BcmBfcSystem shortly after the 
console thread is created.  Then, various system components have a chance 
to create and register their own commands and command tables with this 
main command table.  

The BFC main command table (BcmBfcMainCmdTable) automatically adds 
commands to handle things like reading and writing memory (a common debug 
tool), running the system, quitting/resetting the system, showing the 
version banner, etc.  

The OS-specific BFC application component will usually add OS-specific 
debugging commands to the top level command table.  

Other components can either add their commands directly to the main 
command table or can create their own command tables and register them as 
sub-directories.  This latter approach is recommended for non-trivial 
subsystems, since it keeps the main command table from getting too 
cluttered.  The sub-table approach is necessary if a particular instance 
value is needed (such as a pointer to an object that is being debugged); 
the main command table has the BcmBfcSystem object registered as the 
instance, and you can't register an instance of a completely different 
type without causing the system to crash.  

<H2>Configuring the BFC Console</H2>

\par <tt>BFC_INCLUDE_CONSOLE_SUPPORT</tt>
Set this to 1 to enable console support, or 0 to disable console support.

*/

#endif


