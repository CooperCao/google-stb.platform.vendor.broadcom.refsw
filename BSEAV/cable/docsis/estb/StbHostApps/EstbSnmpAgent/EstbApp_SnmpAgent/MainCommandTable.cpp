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
//
//**************************************************************************
//  $Id$
//
//  Filename:       MainCommandTable.cpp
//  Author:         David Pullen
//  Creation Date:  Feb 18, 2000
//
//****************************************************************************
//  Description:
//      This derived class handles set-up of the main eSTB SNMP Agent application
//      commands in a command table.  This is used by the EstbApplication class.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "MainCommandTable.h"

#include "EstbApplication.h"
#include "CountingSemaphore.h"
// #include "PingThread.h"
#include "ConsoleThread.h"

#include "UnsignedIntCommandParameter.h"
#include "IpAddressCommandParameter.h"
#include "FlagCommandParameter.h"

#if (SNMP_SUPPORT)
#include "EstbSnmpAgent.h"
#endif

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <iostream>
#include <iomanip>
#else
#include <iostream.h>
#include <iomanip.h>
#endif
#include <ctype.h>

#if defined(TARGETOS_stblinux) && defined(BCM97110)
#include "bcmtypes.h"
extern "C" {
  ULONG user_7110reg_base;
}
#endif

//********************** Local Types *****************************************

#if defined(TARGETOS_stblinux) && defined(BCM97110)
#define REG8(addr)                  (*(volatile UCHAR  * const)((addr)^3))
#define REG16(addr)                 (*(volatile USHORT * const)((addr)^2))
#endif

//********************** Local Constants *************************************

static const char *pgTableDescr = "Basic eSTB SNMP Agent Application commands.";

// PR705 - parameter constants for the read/write memory commands.
enum
{
    kNoParameter = 0,
    kIpAddressParameter,
    kStartAddress,
    kNumberOfBytes,
    kContinueFlag,
    kElementSize,
    kValueToWrite,
    kShowSettingsFlag
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
BcmMainCommandTable::BcmMainCommandTable(void) :
    BcmCommandTable("eSTB SNMP Agent App Commands", "ESTB-SNMP-App", "ESTBSNMP", pgTableDescr)
{
    BcmCommand command;

    // Set up fields that don't change.
    command.fCommandFunctionWithParms = CommandHandler;
    command.fIsHidden = false;

    // These commands are available for all targets.
    command.fIsEnabled = true;
    command.fCommandId = kRunApplication;
    command.pfCommandName = "run_app";
    command.pfCommandHelp = "If the application was stopped at the console (either via keypress or via "
                            "non-vol setting that automatically stopped it), then this command will "
                            "allow it to start running.  This command is not available if the application "
                            "is already running.";
    command.pfExamples = "run_app\t";
    AddCommand(command);

    command.fIsEnabled = true;
    command.fCommandId = kResetCommand;
    command.pfCommandName = "reset";
    command.pfCommandHelp = "Resets the system (warm boot).  The hardware reset line is triggered, "
                            "causing the application to be reloaded from scratch.  On host-based app "
                            "simulators, this will cause the application to exit.";
    command.pfExamples = "reset\t";
    AddCommand(command);

    command.fCommandId = kShowVersion;
    command.pfCommandName = "version";
    command.pfCommandHelp = "Displays the current software version and feature codes by printing "
                            "the startup banner.  This allows the user to view the current version "
                            "without having to reboot the modem.";
    command.pfExamples = "version\t";
    AddCommand(command);

    // PR705
    // read_memory [-p] [-c] [-s 1..4] [-n NumBytes] [StartAddr]
    command.fCommandId = kReadMemory;
    command.pfCommandName = "read_memory";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderIndependent(new BcmFlagCommandParameter('p', kShowSettingsFlag));
    command.pfCommandParameterList->AddOrderIndependent(new BcmFlagCommandParameter('c', kContinueFlag));
    command.pfCommandParameterList->AddOrderIndependent(new BcmFlagCommandParameter('s', kElementSize, true,
                                                                                    new BcmUnsignedIntCommandParameter(kElementSize, false, "ElementSize", 1, 4)));
    command.pfCommandParameterList->AddOrderIndependent(new BcmFlagCommandParameter('n', kNumberOfBytes, true,
                                                                                    new BcmUnsignedIntCommandParameter(kNumberOfBytes, false, "NumberOfBytes", 1, 4096)));
    command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kStartAddress, true, "StartAddress"));
    command.pfCommandHelp = 
        "Displays the contents of memory (in hex and ASCII) to the console.\n\n"
        "  StartAddress : the address to start displaying (can be memory, registers,\n"
        "                 etc).\n"
        "  -s : sets the element size to be read (1, 2, or 4 bytes).  Most useful for\n"
        "       registers; defaults to 1.\n"
        "  -n : sets the number of bytes to be read.  Defaults to 16.  Note that this\n"
        "       will always be padded out to a multiple of the element size.\n"
        "  -c : increments the start address by the number of bytes before reading.\n"
        "       This is most useful for continuing the previous read (with the same\n"
        "       parameters).\n"
        "  -p : prints the current options (which would be used if not otherwise\n"
        "       supplied).\n\n"
        "If no parameters are specified, it will perform the last read again.\n\n"
        "NOTE - the parameters are remembered from one read to the next; e.g. if "
        "you set the element size to 4 bytes, then all subsequent reads will use "
        "this, unless explicitly overridden.\n\n"
        "WARNING - it may be possible to make the system hang or crash if you "
        "read from an illegal address!";
    command.pfExamples = "read_memory -s 4 -n 64 0x80001234\tReads 64 bytes as 32-bit values.\n"
                         "read_memory -n 32 0x80001234\tReads 32 bytes starting with the specified address.\n"
                         "read_memory -c\tReads the next 32 bytes, continuing from the previous read.";
    AddCommand(command);

    // PR705
    // write_memory [-s 1..4] Address Value
    command.fCommandId = kWriteMemory;
    command.pfCommandName = "write_memory";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderIndependent(new BcmFlagCommandParameter('s', kElementSize, true,
                                                                                    new BcmUnsignedIntCommandParameter(kElementSize, false, "ElementSize", 1, 4)));
    command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kStartAddress, false, "Address"));
    command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kValueToWrite, false, "Value"));
    command.pfCommandHelp = 
        "Writes the specified value to the specified address.\n\n"
        "  Address : the address to write to (can be memory, registers, etc).\n"
        "  Value   : the value to write.\n"
        "  -s      : sets the element size to be written (1, 2 or 4 bytes).  If not\n"
        "            specified, the default is 1 byte.\n\n"
        "NOTE - unlike read_memory, the parameters are not remembered from one write "
        "to the next.\n\n"
        "WARNING - it is possible to make the system hang or crash if you write "
        "to an illegal address (or write over the application code)!";
    command.pfExamples = "write_memory 0x80001234 0x56\tWrite a byte to the address.\n"
                         "write_memory -s 4 0x80001234 0x12345678\tWrite 32 bits.";
    AddCommand(command);

#if ( BCM_PING_CONSOLE_SUPPORT )
    // Assume that ping is not provided.  It will be enabled if a ping instance
    // is created.
    command.fIsEnabled = false;
    command.fCommandId = kPingCommand;
    command.pfCommandName = "ping";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmIpAddressCommandParameter(kIpAddressParameter));
    command.pfCommandHelp = "Pings the specified target IP address, sending 3 64-byte packets, and waiting "
                            "up to 5 seconds for a response.  This is a basic 'standard' ping.  For more "
                            "options or control over ping parameters and behavior, you will need to go to "
                            "the Ping Command table ('cd pingHelper').\n\n"
                            "In order for this to work, the CM must either have successfully completed "
                            "DHCP, or must otherwise have been configured with a valid IP address.\n\n"
                            "Note that this command causes the ping options to be reset to their default "
                            "state.\n\n"
                            "This may be disabled if the platform doesn't provide an implementation of ping.";
    command.pfExamples =	"ping 11.24.4.3\tPing IP address 11.24.4.3.";
    AddCommand(command);
#endif

}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmMainCommandTable::~BcmMainCommandTable()
{
    // Nothing to do...
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
BcmMainCommandTable *BcmMainCommandTable::GetCommandTableSingleton(void)
{
    static BcmMainCommandTable gSingletonInstance;
    
    return &gSingletonInstance;
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
void BcmMainCommandTable::CommandHandler(void *pInstanceValue,
                                         const BcmCommand &command)
{
    // By convention, the application object registers its instance so that the
    // user value is the 'this' pointer.  This cast should be safe.
    BcmEstbApplication *pInstance = (BcmEstbApplication *) pInstanceValue;

    // Validate this just for sanity.
    if (pInstance == NULL)
    {
        gLogMessageRaw << "BcmMainCommandTable::CommandHandler:  ERROR - User value (pointer to CM App\n"
                          "instance) is NULL!  Can't process the command!\n";

        return;
    }

    switch (command.fCommandId)
    {
        case kResetCommand:
			#if ((SNMP_SUPPORT) && (CM_HAL_INCLUDED))
			// Tell the SNMP agent we're about to go away.
			EstbSnmpAgent::Singleton().InformShutdownPending();
			#endif

			#if KB_TODO
			pInstance->ExitNow();
			#endif
			break;

        case kRunApplication:
            // Tell the app to start running.
            #if (CM_HAL_INCLUDED)
            pInstance->StartDocsis();
            #endif
            break;

        #if ( BCM_PING_CONSOLE_SUPPORT )
        case kPingCommand:

            if (pInstance->PingHelper() == NULL)
            {
                gLogMessageRaw << "ERROR - unable to ping; no ping implementation provided for the target!\n";
            }
            else
            {
                bool result;
                BcmConsoleThread *pConsoleThread;
                const BcmCommandParameter *pIpAddressParm = command.pfCommandParameterList->Find(kIpAddressParameter);

                // Get the console thread singleton.  This may be used by handlers below to
                // parse parameters and/or get user input.
                pConsoleThread = BcmConsoleThread::GetSingletonInstance();

                if (pConsoleThread == NULL)
                {
                    gLogMessageRaw << "BcmMainCommandTable::CommandHandler:  ERROR - Console thread singleton is NULL!\n";

                    break;
                }

                // Reset the default options.
                pInstance->PingHelper()->ResetDefaultOptions();

                // Set the IP address to ping.
                pInstance->PingHelper()->SetIpAddress(pIpAddressParm->AsIpAddress());

                result = pInstance->PingHelper()->Start();

                if (!result)
                {
                    gLogMessageRaw << "ERROR - unable to start pinging the target!\n";
                }
                else
                {
                    // Poll the console thread for a keypress, and also
                    // poll the ping thread for it stopping on its own.
                    while (pInstance->PingHelper()->PingingInProgress() == true)
                    {
                        if (pConsoleThread->WaitForKeypress(250) == true)
                        {
                            gLogMessageRaw << "\nStopping pings due to keypress..." << endl;

                            pInstance->PingHelper()->Stop();

                            break;
                        }
                    }
                }
            }
            break;
        #endif

    case kShowVersion:
        #if (CM_HAL_INCLUDED)
            pInstance->PrintVersion();
        #endif
            break;

        // PR705
        case kReadMemory:
            {
                static uint8 *pgStartAddress = NULL;
                static unsigned int gNumberOfBytes = 16;
                static unsigned int gElementSize = 1;

                const BcmCommandParameter *pStartAddressParm = command.pfCommandParameterList->Find(kStartAddress);
                const BcmCommandParameter *pNumberOfBytesFlag = command.pfCommandParameterList->Find(kNumberOfBytes);
                const BcmCommandParameter *pContinueFlag = command.pfCommandParameterList->Find(kContinueFlag);
                const BcmCommandParameter *pElementSizeFlag = command.pfCommandParameterList->Find(kElementSize);
                const BcmCommandParameter *pShowSettingsFlag = command.pfCommandParameterList->Find(kShowSettingsFlag);

                if (pShowSettingsFlag->WasParsed())
                {
                    // Print our current settings, then bail...
                    gLogMessageRaw 
                        << "     Read Address:  0x" << hex << setfill('0') << setw(8) << pgStartAddress << setfill(' ') << dec << '\n'
                        << "  Number of Bytes:  " << gNumberOfBytes << " bytes\n"
                        << "     Element size:  " << gElementSize << " bytes" << endl;

                    break;
                }

                // If a parameter was specified, use it.
                if (pStartAddressParm->WasParsed())
                {
                    pgStartAddress = (uint8 *) (pStartAddressParm->AsUnsignedInt());
                }
                if (pNumberOfBytesFlag->WasParsed())
                {
                    gNumberOfBytes = pNumberOfBytesFlag->AsFlagParameter()->AsUnsignedInt();
                }
                if (pElementSizeFlag->WasParsed())
                {
                    // Check for illegal value.
                    if (pElementSizeFlag->AsFlagParameter()->AsUnsignedInt() == 3)
                    {
                        gLogMessageRaw << "ERROR - Element Size can't be 3; specify 1, 2, or 4!\n";

                        break;
                    }

                    gElementSize = pElementSizeFlag->AsFlagParameter()->AsUnsignedInt();
                }
                if (pContinueFlag->WasParsed())
                {
                    // Increment the address by the number of bytes.
                    pgStartAddress += gNumberOfBytes;
                }

                // If the address is NULL, make sure they really want to do this.
                if (pgStartAddress == NULL)
                {
                    // UNFINISHED - for now, just error and bail.
                    gLogMessageRaw << "ERROR - Address is NULL!\n";

                    break;
                }

                // If the address is not aligned w.r.t. the size, then make sure
                // they really want to do this.
                if ((((unsigned int) pgStartAddress) & (gElementSize - 1)) != 0)
                {
                    // UNFINISHED - for now, just error and bail.
                    gLogMessageRaw 
                        << "ERROR - Address " << pgStartAddress 
                        << " not aligned for the element size (" 
                        << gElementSize << " bytes)!\n";

                    break;
                }

                // At this point, we are committed to doing the read.
                unsigned int numberOfBytes = 0;
                unsigned int lineOffset = 0;
                char asciiRep[17];

#if defined(TARGETOS_stblinux) && defined(BCM97110)
				// This will currently only read register space for 0xfffexxxx
                static int pgMapStartAddress = ((int)pgStartAddress & 0x0000ffff) | user_7110reg_base;
                uint8 *pValue8 = (uint8 *) pgMapStartAddress;
                uint16 *pValue16 = (uint16 *) pgMapStartAddress;
                uint32 *pValue32 = (uint32 *) pgMapStartAddress;
#else
                uint8 *pValue8 = (uint8 *) pgStartAddress;
                uint16 *pValue16 = (uint16 *) pgStartAddress;
                uint32 *pValue32 = (uint32 *) pgStartAddress;
#endif

                // Fill in the end of the ASCII representation for printing.
                asciiRep[16] = '\0';

                // Set the ostream to hex mode, with 0 fill.
                gLogMessageRaw << hex << setfill('0');

                while (numberOfBytes < gNumberOfBytes)
                {
                    // If we are at the front of a line of output, then display
                    // the address.
                    if (lineOffset == 0)
                    {
                        gLogMessageRaw 
                            << setw(8) 
                            << ((uint32) pgStartAddress) + numberOfBytes 
                            << ": ";
                    }

                    // Read the value as an element of the appropriate size,
                    // and fill in the ASCII rep for the value.
                    if (gElementSize == 1)
                    {
#if defined(TARGETOS_stblinux) && defined(BCM97110)
                        uint8 value = REG8((int)pValue8);
#else
                        uint8 value = *pValue8;
#endif

                        gLogMessageRaw << setw(2) << (unsigned int) value << ' ';

                        asciiRep[lineOffset] = (char) value;

                        if (!isprint(asciiRep[lineOffset]))
                        {
                            asciiRep[lineOffset] = '.';
                        }

                        pValue8++;
                    }
                    else if (gElementSize == 2)
                    {
                        // Do I need to endian swap?
#if defined(TARGETOS_stblinux) && defined(BCM97110)
                        uint16 value = REG16((int)pValue16);
#else
                        uint16 value = *pValue16;
#endif

                        gLogMessageRaw << setw(4) << (unsigned int) value << ' ';

                        asciiRep[lineOffset] = (char) ((value >> 8) & 0xff);
                        asciiRep[lineOffset + 1] = (char) (value & 0xff);

                        if (!isprint(asciiRep[lineOffset]))
                        {
                            asciiRep[lineOffset] = '.';
                        }
                        if (!isprint(asciiRep[lineOffset + 1]))
                        {
                            asciiRep[lineOffset + 1] = '.';
                        }

                        pValue16++;
                    }
                    else
                    {
                        // Do I need to endian swap?
                        uint32 value = *pValue32;

                        gLogMessageRaw << setw(8) << (unsigned int) value << ' ';

                        asciiRep[lineOffset] = (char) ((value >> 24) & 0xff);
                        asciiRep[lineOffset + 1] = (char) ((value >> 16) & 0xff);
                        asciiRep[lineOffset + 2] = (char) ((value >> 8) & 0xff);
                        asciiRep[lineOffset + 3] = (char) (value & 0xff);

                        if (!isprint(asciiRep[lineOffset]))
                        {
                            asciiRep[lineOffset] = '.';
                        }
                        if (!isprint(asciiRep[lineOffset + 1]))
                        {
                            asciiRep[lineOffset + 1] = '.';
                        }
                        if (!isprint(asciiRep[lineOffset + 2]))
                        {
                            asciiRep[lineOffset + 2] = '.';
                        }
                        if (!isprint(asciiRep[lineOffset + 3]))
                        {
                            asciiRep[lineOffset + 3] = '.';
                        }

                        pValue32++;
                    }

                    numberOfBytes += gElementSize;
                    lineOffset += gElementSize;

                    // Only display up to 16 bytes per line.  After this,
                    // display the ASCII representation and start a new line.
                    if (lineOffset == 16)
                    {
                        gLogMessageRaw << "| " << asciiRep << endl;

                        lineOffset = 0;
                    }

                    // Every 4 bytes, output extra spaces to separate groups of
                    // values (except for after the last 4 bytes on the line,
                    // because we write out the ASCII representation).
                    else if ((lineOffset & 0x03) == 0)
                    {
                        gLogMessageRaw << ' ';
                    }
                }

                // Set the ostream back to decimal and space fill.
                gLogMessageRaw << dec << setfill(' ');

                // If we didn't finish out a whole line, then pad with spaces
                // and then print the ASCII rep for the partial line.
                if (lineOffset > 0)
                {
                    asciiRep[lineOffset] = '\0';

                    while (lineOffset < 16)
                    {
                        gLogMessageRaw << setw((gElementSize * 2) + 1) << " ";

                        lineOffset += gElementSize;
                    
                        // Every 4 bytes, output extra spaces to separate groups
                        // of values (except for the end of the line).
                        if (((lineOffset & 0x03) == 0) &&
                            (lineOffset != 16))
                        {
                            gLogMessageRaw << ' ';
                        }
                    }
                
                    gLogMessageRaw << "| " << asciiRep << endl;
                }
            }
            break;

        // PR705
        case kWriteMemory:
            {
                const BcmCommandParameter *pAddressParm = command.pfCommandParameterList->Find(kStartAddress);
                const BcmCommandParameter *pValueParm = command.pfCommandParameterList->Find(kValueToWrite);
                const BcmCommandParameter *pElementSizeFlag = command.pfCommandParameterList->Find(kElementSize);

                unsigned int valueSize = 1;

                // If the user specified a size, then use it.
                if (pElementSizeFlag->WasParsed())
                {
                    // Check for illegal value.
                    if (pElementSizeFlag->AsFlagParameter()->AsUnsignedInt() == 3)
                    {
                        gLogMessageRaw << "ERROR - Element Size can't be 3; specify 1, 2, or 4!\n";

                        break;
                    }

                    valueSize = pElementSizeFlag->AsFlagParameter()->AsUnsignedInt();
                }

                // If the address is NULL, make sure they really want to do this.
                if (pAddressParm->AsUnsignedInt() == 0)
                {
                    // UNFINISHED - for now, just error and bail.
                    gLogMessageRaw << "ERROR - Address is NULL!\n";

                    break;
                }

                // If the address is not aligned w.r.t. the size, then make sure
                // they really want to do this.
                if ((pAddressParm->AsUnsignedInt() & (valueSize - 1)) != 0)
                {
                    // UNFINISHED - for now, just error and bail.
                    gLogMessageRaw 
                        << "ERROR - Address " << pAddressParm->AsUnsignedInt() 
                        << " not aligned for the element size (" 
                        << valueSize << " bytes)!\n";

                    break;
                }

                // At this point, we are committed to doing the write.
                gLogMessageRaw 
                    << "Writing " << (void *) (pValueParm->AsUnsignedInt()) 
                    << " (" << pValueParm->AsUnsignedInt() << ") to 0x" << hex
                    << setfill('0') << setw(8) << pAddressParm->AsUnsignedInt()
                    << setfill(' ') << dec << endl;

                // Write the value corresponding to the size selected.
                if (valueSize == 1)
                {
                    uint8 *pValue = (uint8 *) (pAddressParm->AsUnsignedInt());

                    // Check for value out of range; print a warning if too big.
                    if (pValueParm->AsUnsignedInt() > 0xff)
                    {
                        gLogMessageRaw << "WARNING:  Value too big!  Truncating to 8 bits..." << endl;
                    }

                    *pValue = (uint8) (pValueParm->AsUnsignedInt() & 0xff);
                }
                else if (valueSize == 2)
                {
                    uint16 *pValue = (uint16 *) (pAddressParm->AsUnsignedInt());

                    // Check for value out of range; print a warning if too big.
                    if (pValueParm->AsUnsignedInt() > 0xffff)
                    {
                        gLogMessageRaw << "WARNING:  Value too big!  Truncating to 16 bits..." << endl;
                    }

                    *pValue = (uint16) (pValueParm->AsUnsignedInt() & 0xffff);
                }
                else
                {
                    uint32 *pValue = (uint32 *) (pAddressParm->AsUnsignedInt());

                    *pValue = (uint32) (pValueParm->AsUnsignedInt());
                }
            }
            break;

        default:
            gLogMessageRaw << "WARNING - unknown command id (" << command.fCommandId << ")!  Ignoring...\n";
            break;
    }
}


