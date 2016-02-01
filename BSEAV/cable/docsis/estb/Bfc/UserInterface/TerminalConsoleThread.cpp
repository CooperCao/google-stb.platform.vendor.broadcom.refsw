//****************************************************************************
//
// Copyright (c) 2010 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.  
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//****************************************************************************
//  $Id: TerminalConsoleThread.cpp 1.32 2005/11/17 21:29:03Z sdunlap Release $
//
//  Filename:       TerminalConsoleThread.cpp
//  Author:         David Pullen
//  Creation Date:  Feb 13, 2000
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "TerminalConsoleThread.h"

#include "MutexSemaphore.h"
#include "OperatingSystem.h"
#include "SystemTime.h"
#include "OctetBuffer.h"

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <iomanip>
#else
#include <iomanip.h>
#endif

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


BcmTerminalConsoleThread::BcmTerminalConsoleThread(bool startSuspended,
                                                   BcmOperatingSystem::ThreadPriority initialPriority) :
    BcmConsoleThread(startSuspended, initialPriority)
{
    // Override the class name given by my parent class.
    fMessageLogSettings.SetModuleName("BcmTerminalConsoleThread");

    // Force the app-specific messages to be off, but use the other defaults.
    fMessageLogSettings.SetEnabledSeverities(fMessageLogSettings.GetEnabledSeverities() & (~BcmMessageLogSettings::kAppSpecificMessages));

    pfMutex = NULL;
    pfScript = NULL;

    // The derived class must spawn the thread using the following:
    //pfOperatingSystem->BeginThread(this);
}


BcmTerminalConsoleThread::~BcmTerminalConsoleThread()
{
    // The derived class must wait for the thread to exit using the following:
    //WaitForThread();
}


bool BcmTerminalConsoleThread::WaitForKeypress(unsigned int timeoutMS,
                                               char charToWaitFor)
{
    bool result, gotChar;
    unsigned int beginTimeMS, currentTimeMS;
    char character;

    // Protect against contention with the GetCommandString() method, who can
    // also be looking for characters.  Don't want him to snarf any input that
    // should be for me.
    result = pfMutex->Lock(BcmMutexSemaphore::kTimeout, timeoutMS);

    if (!result)
    {
        gErrorMsg(fMessageLogSettings, "WaitForKeypress")
            << "Failed to lock mutex!" << endl;

        return false;
    }

    // Get the time that we entered this method.  Don't let the mutex lock
    // count against them.
    beginTimeMS = SystemTimeMS();

    gotChar = false;

    // Keep going until I get a keypress, I time out waiting, or I am told to
    // exit.
    while (!fTimeToExit)
    {
        // Vector to the derived class to get a character.
        gotChar = ReadCharacter(character);
        if (gotChar)
        {
            // PR6859 - if the client wants to wait for a particular character,
            // make sure this matches what they want.  If not, ignore it and
            // keep waiting.
            if ((charToWaitFor == 0) || (charToWaitFor == character))
            {
                // If a special character was typed (left arrow, home, end, etc.),
                // then make sure we flush out the second part of the escape
                // sequence.
                if (character == 0x1b)
                {
                    ReadEscapeSequence();
                }

                gAppMsg(fMessageLogSettings, "WaitForKeypress", 0x80000000)
                    << "Got a keypress!" << endl;

                break;
            }
            else
            {
                gAppMsg(fMessageLogSettings, "WaitForKeypress", 0x80000000)
                    << "Got a keypress, but not the one we are waiting for; still waiting..." << endl;

                gotChar = false;
            }
        }

        // Calculate the elapsed time.  If we've waited long enough, then stop.
        currentTimeMS = SystemTimeMS();
        if (currentTimeMS - beginTimeMS > timeoutMS)
        {
            gAppMsg(fMessageLogSettings, "WaitForKeypress", 0x80000000)
                << "Timed out waiting for keypress." << endl;
            break;
        }

        // Should I be allowed to consume all resources?  What about other
        // threads who need to run?
        pfOperatingSystem->Sleep(50);
    }

    // Let the GetCommandString() method run again.
    pfMutex->Unlock();

    return gotChar;
}


void BcmTerminalConsoleThread::GetCommandString(const char *pCommandPrompt,
                                                char *pCommandString, 
                                                unsigned int stringSize,
                                                bool allowCommandCompletion)
{
    unsigned int offset = 0, lastChar = 0, historyOffset;
    bool gotChar, result;
    char character;
    unsigned long beginTime = 0, currentTime;
    bool localMessageWritten = false, rewriteCommandLine = false;
    bool clearCommandLine = false;
    unsigned int commandLineClearLength = 0;
    bool localTimeToExit = false;

    if (pCommandString == NULL)
    {
        gErrorMsg(fMessageLogSettings, "GetCommandString")
            << "Pointer to command string is NULL!" << endl;

        return;
    }

    // Show the command prompt.
    gLogMessageRaw << pCommandPrompt << flush;
    
    gMessageWritten = false;
    
    pCommandString[0] = '\0';

    historyOffset = fNextCommandIndex;

    // Keep going until I get a full command (terminated by Enter) or I am told
    // to exit.
    while (!fTimeToExit && !localTimeToExit)
    {
        // Avoid collisions between this and waiting for a keypress...
        result = pfMutex->Lock();

        if (result == false)
        {
            gErrorMsg(fMessageLogSettings, "GetCommandString")
                << "Failed to lock mutex!" << endl;

            return;
        }

        character = '\0';
        gotChar = false;

        if (pfScript)
        {
            // Read a character from the script
            gotChar = pfScript->Read(fScriptOffset, (uint8 *)&character, 1);

            if (!gotChar)
            {
                // Script complete, delete it
                delete pfScript;
                pfScript = NULL;
            }
        }
        
        if (!gotChar)
        {    
            // Check for a keypress.
            gotChar = ReadCharacter(character);
        }

        // PR6177 - don't allow a \0 character to be added to the buffer.  This
        // will prematurely truncate the command, causing characters that follow
        // to be ignored.  This was found as a result of testing the telnet
        // server with a Linux telnet client; when you hit enter on Linux, it
        // sends a \n\0, and the \0 is left hanging around for the start of the
        // next command, which is what causes problems.
        if (character == '\0')
        {
            gotChar = false;
        }

        // If the user has entered at least part of a command string, and they
        // type a ?, then show the online help for that command.  Treat this
        // as though a char wasn't entered.
        if (gotChar && (lastChar > 0) && (character == '?'))
        {
            char commandOnly[128];
            
            // Pull out the command; if we pass the full command line in to
            // ShowHelp, then it will try to show help for the parameters in
            // the command.  We only want help on the command itself.
            if (sscanf(pCommandString, "%127s", commandOnly) >= 0)
            {
                // Forget that they entered this char.
                gotChar = false;

                // Make sure the command line gets re-displayed.
                rewriteCommandLine = true;

                gLogMessageRaw << "\n\n";

                ShowHelp(commandOnly);
            }
        }

        // If a key was pressed, then process it, otherwise try again.
        if (gotChar)
        {
            // PR9692 - insert 'enter' character if the command line grows rediculously long
            // to avoid a crash
            if( strlen( pCommandString ) >= 127 )
            {
                character = '\n';
            }
        
            // Translate a tab to a space.
            if (character == '\t')
            {
                character = ' ';
            }

            // If this is whitespace, and this is the first such occurrance,
            // then expand out the partial command to the full command name.
            if (allowCommandCompletion &&
                ((character == ' ') || (character == '\n') || (character == '\r')))
            {
                unsigned int commandId;
                const char *pFullCommandString = NULL;
                bool firstWhitespace = true;

                // Check to see if there already is a space in the string
                // (before the current offset).  If not, then expand out the
                // partial command to the full command name.
                if (offset == 0)
                {
                    firstWhitespace = false;
                }
                else
                {
                    for (unsigned int i = 0; i < offset; i++)
                    {
                        if (pCommandString[i] == ' ')
                        {
                            firstWhitespace = false;
                            break;
                        }
                    }
                }

                // If this is the first whitespace, and the character is a
                // newline, and the cursor isn't at the end of the line, then
                // don't do command completion.  If we don't do this, then you
                // will get unexpected behavior if you use home or left arrow
                // to change some chars and hit enter from the middle of the
                // command (it will add chars into the middle of the command).
                if (firstWhitespace && ((character == '\n') || (character == '\r')) &&
                    (offset < lastChar))
                {
                    firstWhitespace = false;
                }

                if (firstWhitespace)
                {
                    char *pTempCommand, *pTempParameter;

                    // Create temporary strings to keep the command string and
                    // parameters.
                    pTempCommand = new char[offset + 1];
                    pTempParameter = new char[lastChar - offset + 1];

                    // Pull out the command; IsValidCommand won't work if there
                    // are any spaces or parameters after the command.
                    strncpy(pTempCommand, pCommandString, offset);
                    pTempCommand[offset] = '\0';

                    strcpy(pTempParameter, pCommandString + offset);

                    // See if it is in the built-in command table.  If not, see
                    // if it is in the active command table.
                    if (fBuiltInCommands.IsValidCommand(pTempCommand, commandId))
                    {
                        // Get a pointer to the full string.
                        pFullCommandString = fBuiltInCommands.GetCommandString(commandId);
                    }
                    else if ((pfActiveCommandTable != NULL) &&
                             pfActiveCommandTable->IsValidCommand(pTempCommand, commandId))
                    {
                        // Get a pointer to the full string.
                        pFullCommandString = pfActiveCommandTable->GetCommandString(commandId);
                    }

                    // If we got the full command string, then copy it to the
                    // command string being built, and update the settings so
                    // that we don't do this again on this command, and we do
                    // re-display the command to show the full string.  Note
                    // that we have to be careful when doing this, because we
                    // have to 'delete' the old partial command name, and
                    // 'insert' the full one in front of any parameters.
                    if (pFullCommandString != NULL)
                    {
                        rewriteCommandLine = true;
                        clearCommandLine = true;

                        // Just copy the full command name, followed by the
                        // rest of the parameters.
                        strcpy(pCommandString, pFullCommandString);
                        strncat(pCommandString, pTempParameter, stringSize - strlen(pFullCommandString) - 1);
                        pCommandString[stringSize - 1] = '\0';

                        offset = strlen(pFullCommandString);
                        lastChar = strlen(pCommandString);
                    }
                    else if ((character != '\n') && (character != '\r'))
                    {
                        // This sends a "bell" indicator; only if not enter key.
                        WriteCharacter('\07');
                    }

                    // Get rid of the memory I created for the temporary strings.
                    delete pTempCommand;
                    delete pTempParameter;
                }
            }
            
            // If it is the "enter" key, then close out the current command
            // with a string terminator and break out.
            if ((character == '\n') || (character == '\r'))
            {
                // Fall through the end of the loop, allowing the command line
                // to be redisplayed, then exit normally.
                localTimeToExit = true;

                if (allowCommandCompletion)
                {
                    rewriteCommandLine = true;
                }
                else
                {
                    WriteCharacter('\n');
                }
            }

            // If it is the "escape" key, then do escape sequence processing.
            else if (character == 0x1b)
            {
                EscapeKeySequence escapeSequence;

                // Get the translated escape sequence.
                escapeSequence = ReadEscapeSequence();

                // Check for known escape sequences that we can handle.
                switch (escapeSequence)
                {
                    case kEscapeOnly:
                        // If it was only the escape key, then handle this by
                        // printing an escape string, then clearing out the
                        // current command and returning.  This aborts the
                        // current command line that is being built.
                        WriteCharacter(' ');
                        WriteCharacter('[');
                        WriteCharacter('E');
                        WriteCharacter('s');
                        WriteCharacter('c');
                        WriteCharacter(']');
                        WriteCharacter(' ');
                        WriteCharacter('\n');

                        pCommandString[0] = '\0';

                        localTimeToExit = true;
                        break;

                    case kRightArrow:
                        if (offset < lastChar)
                        {
                            // Move the cursor.  This writes the current char
                            // before advancing the offset.
                            WriteCharacter(pCommandString[offset]);
                            
                            offset++;
                        }
                        else
                        {
                            // This sends a "bell" indicator.
                            WriteCharacter('\07');
                        }
                        break;

                    case kCtlRightArrow:
                        if (offset < lastChar)
                        {
                            // Seek forward to a non alpha-numeric char.
                            while ((offset < lastChar) &&
                                   (isalnum(pCommandString[offset]) || (pCommandString[offset] == '_')))
                            {
                                // Move the cursor.  This writes the current char
                                // before advancing the offset.
                                WriteCharacter(pCommandString[offset]);

                                offset++;
                            }

                            // Then seek forward to the first alpha-numeric char.
                            while ((offset < lastChar) &&
                                   (!isalnum(pCommandString[offset]) && (pCommandString[offset] != '_')))
                            {
                                // Move the cursor.  This writes the current char
                                // before advancing the offset.
                                WriteCharacter(pCommandString[offset]);

                                offset++;
                            }
                        }
                        else
                        {
                            // This sends a "bell" indicator.
                            WriteCharacter('\07');
                        }
                        break;

                    case kLeftArrow:
                        // Left arrow.
                        if (offset > 0)
                        {
                            // Move the cursor.  This backs it up one space.
                            WriteCharacter('\b');
                            
                            offset--;
                        }
                        else
                        {
                            // This sends a "bell" indicator.
                            WriteCharacter('\07');
                        }
                        break;

                    case kCtlLeftArrow:
                        // Left arrow.
                        if (offset > 0)
                        {
                            // Seek backward until the char just before this one
                            // is alpha-numeric.
                            while ((offset > 0) &&
                                   (!isalnum(pCommandString[offset - 1]) && (pCommandString[offset - 1] != '_')))
                            {
                                // Move the cursor.  This backs it up one space.
                                WriteCharacter('\b');

                                offset--;
                            }

                            // Seek backward until the char just before this
                            // one is not alpha-numeric.
                            while ((offset > 0) &&
                                   (isalnum(pCommandString[offset - 1]) || (pCommandString[offset - 1] == '_')))
                            {
                                // Move the cursor.  This backs it up one space.
                                WriteCharacter('\b');

                                offset--;
                            }
                        }
                        else
                        {
                            // This sends a "bell" indicator.
                            WriteCharacter('\07');
                        }
                        break;

                    case kHomeKey:
                        // Home key.  Move the cursor all the way back to the
                        // left.
                        while (offset > 0)
                        {
                            WriteCharacter('\b');

                            offset--;
                        }
                        break;

                    case kEndKey:
                        // End key.  Move the cursor all the way to the right
                        // by printing the rest of the current command.
                        gLogMessageRaw << pCommandString + offset << flush;
                        offset = lastChar;
                        break;

                    case kUpArrow:
                        // Up arrow.  Replace the current command with the
                        // previous one from the history.  Wrap around in the
                        // list if necessary, and skip over empty history slots.
                        do
                        {
                            if (historyOffset == 0)
                            {
                                historyOffset = kMaxCommands - 1;
                            }
                            else
                            {
                                historyOffset--;
                            }

                        } while ((strlen(pfCommandHistory[historyOffset]) == 0) &&
                                 (historyOffset != fNextCommandIndex));

                        // Since the new command may be shorter than the
                        // previous one, we need to overwrite the previous one
                        // with spaces before displaying the new one.
                        commandLineClearLength = strlen(pCommandString);
                        if (commandLineClearLength > strlen(pfCommandHistory[historyOffset]))
                        {
                            clearCommandLine = true;
                        }
                        rewriteCommandLine = true;

                        // Store this command as the one being built, and update
                        // my state.
                        strcpy(pCommandString, pfCommandHistory[historyOffset]);
                        offset = strlen(pCommandString);
                        lastChar = offset;
                        break;

                    case kDownArrow:
                        // Down arrow.  Replace the current command with the
                        // next one in the history.  Wrap around in the
                        // list if necessary, and skip over empty history slots.
                        do
                        {
                            historyOffset++;
                            if (historyOffset >= kMaxCommands)
                            {
                                historyOffset = 0;
                            }

                        } while ((strlen(pfCommandHistory[historyOffset]) == 0) &&
                                 (historyOffset != fNextCommandIndex));
                        
                        // Since the new command may be shorter than the
                        // previous one, we need to overwrite the previous one
                        // with spaces before displaying the new one.
                        commandLineClearLength = strlen(pCommandString);
                        if (commandLineClearLength > strlen(pfCommandHistory[historyOffset]))
                        {
                            clearCommandLine = true;
                        }
                        rewriteCommandLine = true;

                        // Store this command as the one being built, and update
                        // my state.
                        strcpy(pCommandString, pfCommandHistory[historyOffset]);
                        offset = strlen(pCommandString);
                        lastChar = offset;
                        break;

                    case kDelKey:
                        HandleDelBackspace(pCommandString, offset, lastChar, 0x7f);
                        break;

                    default:
                        // This sends a "bell" indicator.
                        WriteCharacter('\07');
                        break;
                }
            }

            // If it is the delete or backspace key, then I need to copy all of
            // the existing characters back one space.
            else if ((character == 0x7f) || (character == '\b'))
            {
                HandleDelBackspace(pCommandString, offset, lastChar, character);
            }

            // Otherwise, I need to store the character.  If the user has typed
            // too many bytes for me to store, then I discard the remaining bytes.
            else if (lastChar < stringSize - 1)
            {
                unsigned int insertOffset;

                // Move all chars from the current offset to the \0 terminator
                // right one.
                for (insertOffset = lastChar + 1; insertOffset > offset; insertOffset--)
                {
                    pCommandString[insertOffset] = pCommandString[insertOffset - 1];
                }

                // Insert the new character at the current offset.
                pCommandString[offset] = character;

                // Display the newly inserted string to the end.
                gLogMessageRaw << pCommandString + offset << flush;

                offset++;
                lastChar++;
                
                // Move the cursor back to its original position.
                insertOffset = lastChar;
                while (insertOffset > offset)
                {
                    WriteCharacter('\b');

                    insertOffset--;
                }
            }
            else
            {
                // This sends a "bell" indicator.
                WriteCharacter('\07');
            }
        }
    
        // Allow someone to wait for a keypress now.
        pfMutex->Unlock();

        // Don't consume all available CPU for this...?  I should be the lowest
        // priority task, so why not?  Because my priority doesn't have to be
        // low, so this is safer.
        //
        // Here's a new twist:  Allow the user to send a burst of characters
        // without a delay between them.  I only sleep if I tried to read a
        // character but there wasn't one.  This vastly improves performance
        // when pasting a string, etc.  I also increased the amount of time
        // that I will sleep, so overall, this should improve things.
        if (!gotChar || rewriteCommandLine)
        {
            if (!rewriteCommandLine)
            {
                #if (BFC_INCLUDE_IKOS_SUPPORT)
                {
                    // For running on IKOS, make this much faster so we don't
                    // have to wait forever to gather user input.
                    pfOperatingSystem->Sleep(10 * kOneMillisecond);
                }
                #else
                {
                    pfOperatingSystem->Sleep(100 * kOneMillisecond);
                }
                #endif

                // After waking up, let's check to see whether any other process
                // produced some output.  If so, then we need to take note of when this
                // happened so that we can reproduce the command line when the output
                // has been idle long enough.
                if (gMessageWritten == true)
                {
                    gMessageWritten = false;
                    localMessageWritten = true;

                    beginTime = SystemTimeMS();
                }

                // If the message logging has been idle for a while, then we need to
                // output the current command string that's being built.
                else if (localMessageWritten == true)
                {
                    currentTime = SystemTimeMS();

                    if (currentTime - beginTime > kOneTenthSecond)
                    {
                        localMessageWritten = false;

                        rewriteCommandLine = true;
                        clearCommandLine = false;
                    }
                }
            }
    
            if (rewriteCommandLine == true)
            {
                rewriteCommandLine = false;
            
                // If we're being told to clear the command line before rewriting
                // the new one, then do so.
                if (clearCommandLine == true)
                {
                    clearCommandLine = false;
    
                    gLogMessageRaw << "\r" << pCommandPrompt << setfill(' ') 
                                   << setw(commandLineClearLength) << " ";
                }
    
                gLogMessageRaw << "\r" << pCommandPrompt << pCommandString << flush;

                // If the character is a newline, then we're about to exit the
                // loop, and I need to move the cursor to the next line.
                if ((character == '\n') || (character == '\r'))
                {
                    gLogMessageRaw << endl;
                }
                else
                {
                    // Now I have to move the cursor back to its original offset.
                    for (unsigned int newOffset = lastChar; newOffset > offset; newOffset--)
                    {
                        // Move the cursor.  This backs it up one space.
                        WriteCharacter('\b');
                    }
                }
            }
        }
    }
}


void BcmTerminalConsoleThread::RunScript(BcmOctetBuffer *pScript, bool synchronous)
{
    // Avoid collisions while we move scripts around...
    bool result = pfMutex->Lock();

    if (result == false)
    {
        gErrorMsg(fMessageLogSettings, "RunScript")
            << "Failed to lock mutex!" << endl;

        return;
    }

    // If there was already a script in progress, delete it
    delete pfScript;

    // Store the script for character input
    pfScript = pScript;
    fScriptOffset = 0;

    // Let the input methods run again.
    pfMutex->Unlock();

    if (synchronous == false)
    {
        return;
    }

    // Optionally wait for it to complete
    while (pfScript != NULL)
    {
        pfOperatingSystem->Sleep(100 * kOneMillisecond);
    }    
}


bool BcmTerminalConsoleThread::Initialize(void)
{
    pfMutex = pfOperatingSystem->NewMutexSemaphore("Terminal Console Mutex");

    if (pfMutex == NULL)
    {
        gErrorMsg(fMessageLogSettings, "Initialize")
             << "Failed to create mutex!" << endl;
        
        return false;
    }

    // Let my parent to it's thing.
    return BcmConsoleThread::Initialize();
}


void BcmTerminalConsoleThread::Deinitialize(void)
{
    delete pfMutex;
    pfMutex = NULL;

    // Let my parent to it's thing.
    BcmConsoleThread::Deinitialize();
}


void BcmTerminalConsoleThread::HandleDelBackspace(char *pCommandString, 
                                                  unsigned int &offset,
                                                  unsigned int &lastChar, 
                                                  char character)
{
    // Don't allow backspace at offset 0, or delete at the last
    // character.
    if (((character == 0x7f) && (offset == lastChar)) ||
        ((character == '\b') && (offset == 0)))
    {
        // This sends a "bell" indicator.
        WriteCharacter('\07');
    }
    else
    {
        unsigned int delOffset;

        // For a backspace, we need to move the cursor back one
        // char.
        if (character == '\b')
        {
            WriteCharacter('\b');
            offset--;
        }

        // Pull the characters back one space, starting at the
        // current offset, going through the \0 terminator.
        for (delOffset = offset + 1; delOffset <= lastChar; delOffset++)
        {
            pCommandString[delOffset - 1] = pCommandString[delOffset];
        }

        // Display the newly shortened string, with an additional
        // space to delete the last shown character.
        gLogMessageRaw << pCommandString + offset << " " << flush;

        // Move the cursor back to its original position.
        delOffset--;
        while (delOffset > offset)
        {
            WriteCharacter('\b');

            delOffset--;
        }

        // The last char is now reduced by 1.
        lastChar--;
    }
}


