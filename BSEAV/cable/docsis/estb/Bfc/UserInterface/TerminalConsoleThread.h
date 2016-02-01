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
//  $Id: TerminalConsoleThread.h 1.10 2005/07/27 19:35:18Z sdunlap Release $
//
//  Filename:       TerminalConsoleThread.h
//  Author:         David Pullen
//  Creation Date:  Feb 13, 2000
//
//****************************************************************************

#ifndef TerminalConsoleThread_H
#define TerminalConsoleThread_H

//********************** Include Files ***************************************

// By base class.
#include "ConsoleThread.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************


/** \ingroup BfcConsole
*
*      This is the implementation of a Console Thread that uses Terminal-based
*      character I/O methods.  This implementation isn't OS-specific, but it
*      relies on facilities provided by derived classes that are.  The derived
*      class actually does the input/output of characters.
*
*      Note that the derived class must configure the terminal console such
*      that reading a character doesn't block, and any characters that are
*      typed are not automatically echoed.
*
*      This class handles all of the logic to poll for characters and to echo
*      the characters back to the console.
*/
class BcmTerminalConsoleThread : public BcmConsoleThread
{
public:

    /// Default Constructor.  Initializes the state of the object, starting the
    /// thread.
    ///
    /// \param
    ///      startSuspended - tells whether or not the thread should suspend itself
    ///                       after initialization.
    /// \param
    ///      initialPriority - the priority at which you want the thread to run
    ///                        after initialization.
    ///
    BcmTerminalConsoleThread(bool startSuspended = false,
                             BcmOperatingSystem::ThreadPriority initialPriority = BcmOperatingSystem::kIdlePriority);

    /// Destructor.  Frees up any memory/objects allocated, cleans up internal
    /// state.
    ///
    virtual ~BcmTerminalConsoleThread();

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
    virtual bool WaitForKeypress(unsigned int timeoutMS, char charToWaitFor = 0);

protected:

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
    virtual void GetCommandString(const char *pCommandPrompt, char *pCommandString, 
                                  unsigned int stringSize, bool allowCommandCompletion = false);

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
    virtual void RunScript(BcmOctetBuffer *pScript, bool synchronous = true);

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

    /// Thread destructor - this is the last method called when the thread is
    /// exiting, and is where the thread should delete all of the OS objects that
    /// it created.
    ///
    virtual void Deinitialize(void);

protected:

    /// This is what this class calls when it wants to read a character from the
    /// terminal.  It should check for the existance of a character, and if one
    /// is not present, then it should return immediately.  Otherwise, it should
    /// read and return the character.
    ///
    /// The character should not be displayed, and the derived class should not
    /// wait for a character.  All of this logic is handled by this class.
    ///
    /// This method MUST be implemented by derived classes.
    ///
    /// \param
    ///      character - reference to a char into which the byte should be stored.
    ///
    /// \retval
    ///      true if a character was read.
    /// \retval
    ///      false if a character was not read.
    ///
    virtual bool ReadCharacter(char &character) = 0;

    typedef enum
    {
        kUnknownEscape,     ///< This applies to escape sequences not shown below.
        kEscapeOnly,        ///< Only the escape key was pressed.
        kHomeKey,           ///< The home key was pressed.
        kEndKey,            ///< The end key was pressed.
        kRightArrow,        ///< The right arrow key was pressed.
        kLeftArrow,         ///< The left arrow key was pressed.
        kUpArrow,           ///< The up arrow key was pressed.
        kDownArrow,         ///< The down arrow key was pressed.
        kCtlLeftArrow,      ///< Left arrow with Control key.
        kCtlRightArrow,     ///< Right arrow with Control key.
        kDelKey             ///< The delete key.
    } EscapeKeySequence;

    /// This is what this class calls when it wants to read an escape sequence
    /// from the terminal.  The return value must be one of the enum values
    /// above.  The actual escape sequence depends on the terminal emulator type,
    /// etc, which is why the translations is necessary.
    ///
    /// \return
    ///      The escape key sequence entered.
    ///
    virtual EscapeKeySequence ReadEscapeSequence(void) = 0;

    /// This is what this class calls when it needs to write a character to the
    /// terminal.  This is what does the echoing of bytes back to the user.
    ///
    /// This method MUST be implemented by derived classes.
    ///
    /// \param
    ///      character - the byte to write to the console.
    ///
    /// \retval
    ///      true if the character was written.
    /// \retval
    ///      false if a problem occurred.
    ///
    virtual bool WriteCharacter(char character) = 0;

    /// This is what this class calls when it needs to do backspace processing.
    /// The derived class will generally write a backspace, followed by a space
    /// (to delete the original character), followed by a backspace.
    ///
    /// This method MUST be implemented by derived classes.
    ///
    /// \retval
    ///      true if the character was written.
    /// \retval
    ///      false if a problem occurred.
    ///
    virtual bool WriteBackspace(void) = 0;

    /// This helper method handles backspace or delete keypress.  It deletes a
    /// character from the input string, and pulls the remaining chars back one
    /// space.
    ///
    /// \param
    ///      pCommandString - the current command string that is being built.
    /// \param
    ///      offset - where the cursor currently is.
    /// \param
    ///      lastChar - the position of the last char in the string.
    /// \param
    ///      character - the character that was read from the input.
    ///
    void HandleDelBackspace(char *pCommandString, unsigned int &offset,
                            unsigned int &lastChar, char character);

private:

    /// Mutex to protect against different threads calling me at the same time.
    /// This is mainly to protect access to the serial driver when another
    /// thread calls WaitForKeypress().
    BcmMutexSemaphore *pfMutex;

    /// The currently executing script file, if any
    BcmOctetBuffer *pfScript;

    /// The current offset into the script file
    unsigned int fScriptOffset;

private:

    /// Copy Constructor.  Not supported.
    ///
    BcmTerminalConsoleThread(const BcmTerminalConsoleThread &otherInstance);

    /// Assignment operator.  Not supported.
    ///
    BcmTerminalConsoleThread & operator = (const BcmTerminalConsoleThread &otherInstance);

};


//********************** Inline Method Implementations ***********************

#endif


