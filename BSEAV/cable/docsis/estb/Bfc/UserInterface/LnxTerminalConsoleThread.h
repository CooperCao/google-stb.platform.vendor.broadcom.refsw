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
//  $Id: LnxTerminalConsoleThread.h 1.1 2001/11/01 17:57:52Z tsaladino Release $
//
//  Filename:       LnxTerminalConsoleThread.h
//  Author:         David Pullen
//  Creation Date:  Feb 17, 2000
//
//****************************************************************************
//  Description:
//      This is the implementation of a Terminal Console Thread that handles
//      pSOS console driver reads and writes.
//
//****************************************************************************

#ifndef PSOSTERMINALCONSOLETHREAD_H
#define PSOSTERMINALCONSOLETHREAD_H

//********************** Include Files ***************************************

#include <termios.h>

// By base class.
#include "TerminalConsoleThread.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************


class BcmLnxTerminalConsoleThread : public BcmTerminalConsoleThread
{
public:

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
    BcmLnxTerminalConsoleThread(bool startSuspended = false,
                                 BcmOperatingSystem::ThreadPriority initialPriority = BcmOperatingSystem::kIdlePriority);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmLnxTerminalConsoleThread();

protected:

    // This is what this class calls when it wants to read a character from the
    // terminal.  It should check for the existance of a character, and if one
    // is not present, then it should return immediately.  Otherwise, it should
    // read and return the character.
    //
    // The character should not be displayed, and the derived class should not
    // wait for a character.  All of this logic is handled by this class.
    //
    // Parameters:
    //      character - reference to a char into which the byte should be stored.
    //
    // Returns:
    //      true if a character was read.
    //      false if a character was not read.
    //
    virtual bool ReadCharacter(char &character);

    // This is what this class calls when it wants to read an escape sequence
    // from the terminal.  The return value must be one of the enum values
    // above.  The actual escape sequence depends on the terminal emulator type,
    // etc, which is why the translations is necessary.
    //
    // Parameters:  None.
    //
    // Returns:
    //      The escape key sequence entered.
    //
    virtual EscapeKeySequence ReadEscapeSequence(void);

    // This is what this class calls when it needs to write a character to the
    // terminal.  This is what does the echoing of bytes back to the user.
    //
    // Parameters:
    //      character - the byte to write to the console.
    //
    // Returns:
    //      true if the character was written.
    //      false if a problem occurred.
    //
    virtual bool WriteCharacter(char character);

    // This is what this class calls when it needs to do backspace processing.
    // The derived class will generally write a backspace, followed by a space
    // (to delete the original character), followed by a backspace.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if the character was written.
    //      false if a problem occurred.
    //
    virtual bool WriteBackspace(void);

private:

    // The original terminal settings, so that they can be reset.
    struct termios fOriginalTerminalIO;

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmLnxTerminalConsoleThread(const BcmLnxTerminalConsoleThread &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmLnxTerminalConsoleThread & operator = (const BcmLnxTerminalConsoleThread &otherInstance);

};


//********************** Inline Method Implementations ***********************

#endif


