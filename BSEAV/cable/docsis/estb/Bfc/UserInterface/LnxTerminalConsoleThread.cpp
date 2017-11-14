/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
//
//****************************************************************************
//  $Id$
//
//  Filename:       TerminalConsoleThread.cpp
//  Author:         David Pullen
//  Creation Date:  Feb 17, 2000
//
//****************************************************************************
//  Description:
//      This is the implementation of a Terminal Console Thread that handles
//      pSOS console driver reads and writes.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "LnxTerminalConsoleThread.h"

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <iostream>
#else
#include <iostream.h>
#endif
#include <termios.h>
#include <unistd.h>

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Global Functions ************************************

//********************** Class Method Implementations ************************


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
BcmLnxTerminalConsoleThread::BcmLnxTerminalConsoleThread(bool startSuspended,
                                                           BcmOperatingSystem::ThreadPriority initialPriority) :
    BcmTerminalConsoleThread(startSuspended, initialPriority)
{
    struct termios terminalIO;

    // Override the class name set by my parent class.
    fMessageLogSettings.SetModuleName("BcmLnxTerminalConsoleThread");

    // Change the pSOS terminal console driver so that it uses non-canonical
    // mode, with echo turned off.  Start by getting the current configuration.

    tcgetattr(STDIN_FILENO, &terminalIO);
    fOriginalTerminalIO = terminalIO;

    // This should mean "Leave the baud rate alone".  For some reason, reading
    // the baud rate in the de_cntrl doesn't return the correct current value;
    // however, storing it back causes it to be set, which changes it to the
    // wrong value.
    // 062206 SJ PR21331: Remove following line to fix PR
    // terminalIO.c_cflag &= ~CBAUD;

    // Turn off canonical mode and echo.
    terminalIO.c_lflag &= ~(ICANON | ECHO | ECHONL);

    // Allow the read to return after 0 characters are read, and set the timeout
    // to 0 ms.  This lets me poll for input.
    terminalIO.c_cc[VMIN] = 0;
    terminalIO.c_cc[VTIME] = 0;

    // Now lock in the new configuration.  All pending input will be discarded,
    // and output will be flushed.
    tcsetattr(STDIN_FILENO, TCSANOW, &terminalIO);

    // Spawn the thread.
    if (!pfOperatingSystem->BeginThread(this))
    {
        gErrorMsg(fMessageLogSettings, "BcmLnxTerminalConsoleThread")
             << "Failed to spawn thread!" << endl;
    }
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmLnxTerminalConsoleThread::~BcmLnxTerminalConsoleThread()
{
    // Reset the original terminal settings.
    tcsetattr(STDIN_FILENO, TCSANOW, &fOriginalTerminalIO);

    gInfoMsg(fMessageLogSettings, "~BcmLnxTerminalConsoleThread")
        << "Waiting for the thread to exit..." << endl;

    // Wait for the thread to exit.
    WaitForThread();

    gInfoMsg(fMessageLogSettings, "~BcmLnxTerminalConsoleThread")
        << "Thread exited!" << endl;
}


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
bool BcmLnxTerminalConsoleThread::ReadCharacter(char &character)
{
    long result;
    
    // See if there is a character waiting.  If so, then it will be consumed.
    // Otherwise, it will return immediately with bytesRead == 0.
    result = read(STDIN_FILENO, &character, 1);

    if (result == -1)
    {
        gErrorMsg(fMessageLogSettings, "ReadCharacter")
             << "read failed!  Result is " << (void *) result << endl;

        return false;
    }

    if (result > 0)
    {
        gAppMsg(fMessageLogSettings, "ReadCharacter", 0x80000000)
            << "User typed character code " << (void *)(int)character << endl;

        return true;
    }

    return false;
}


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
BcmLnxTerminalConsoleThread::EscapeKeySequence BcmLnxTerminalConsoleThread::ReadEscapeSequence(void)
{
    bool keyRead;
    char character;

    keyRead = ReadCharacter(character);

    // If there is no character waiting, then only the escape key was pressed.
    if (keyRead == false)
    {
        return kEscapeOnly;
    }

    // This forms the first character in the ANSI escape sequence.
    if (character == 0x5b)
    {
        // See what the second character is.
        keyRead = ReadCharacter(character);

        if (keyRead == true)
        {
            switch (character)
            {
                case 0x41:
                    // Up arrow.
                    return kUpArrow;

                case 0x42:
                    // Down arrow.
                    return kDownArrow;

                case 0x43:
                    // Right arrow.
                    return kRightArrow;

                case 0x44:
                    // Left arrow.
                    return kLeftArrow;

                case 0x45:
                    // Home key.  I'm not sure if this is legitimate...CRT
                    // (and other terminal emulators) won't send this character
                    // natively.
                    return kHomeKey;

                case 0x46:
                    // End key.  I'm not sure if this is legitimate...CRT
                    // (and other terminal emulators) won't send this character
                    // natively.
                    return kEndKey;

                case 0x47:
                    // Control-Left-Arrow.  I'm not sure if this is legitimate...
                    // CRT (and other terminal emulators) won't send this
                    // character natively.
                    return kCtlLeftArrow;

                case 0x48:
                    // Control-Right-Arrow.  I'm not sure if this is legitimate...
                    // CRT (and other terminal emulators) won't send this
                    // character natively.
                    return kCtlRightArrow;

                default:
                    // Unknown.
                    break;
            }
        }
    }

    return kUnknownEscape;
}


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
bool BcmLnxTerminalConsoleThread::WriteCharacter(char character)
{
    long result;

    // Write it out.
    result = write(STDOUT_FILENO, &character, 1);

    if (result == -1)
    {
        gErrorMsg(fMessageLogSettings, "WriteCharacter")
             << "write failed!  Result is " << (void *) result << endl;

        return false;
    }

    if (result != 1)
    {
        gErrorMsg(fMessageLogSettings, "WriteCharacter")
             << "write didn't write the character!" << endl;

        return false;
    }

    return true;
}


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
bool BcmLnxTerminalConsoleThread::WriteBackspace(void)
{
    long result;
    unsigned char backspace[3];

    backspace[0] = '\b';
    backspace[1] = ' ';
    backspace[2] = '\b';
    
    // Write it out.
    result = write(STDOUT_FILENO , backspace, 3);

    if (result == -1)
    {
        gErrorMsg(fMessageLogSettings, "WriteCharacter")
             << "write failed!  Result is " << (void *) result << endl;

        return false;
    }

    if (result != 3)
    {
        gErrorMsg(fMessageLogSettings, "WriteCharacter")
             << "write didn't write the characters!" << endl;

        return false;
    }

    return true;
}


