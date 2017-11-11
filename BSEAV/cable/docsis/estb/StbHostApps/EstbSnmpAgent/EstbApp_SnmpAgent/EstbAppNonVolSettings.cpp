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
//  Filename:       EstbAppNonVolSettings.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  12 MAY 06
//
//****************************************************************************
//  Description:
//      These are the Non-Volatile settings for the eSTB SNMP Agent
//      application.  This is derived from the abstract base NonVolSettings
//      class, which provides hooks for loading and storing the settings.
//
//      Note that the settings are independant of the device to which they are
//      being stored, allowing the same application settings to be used on
//      multiple target platforms and devices!
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "EstbAppNonVolSettings.h"

#include <string.h>

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <iostream>
#include <iomanip>
#else
#include <iostream.h>
#include <iomanip.h>
#endif

//********************** Local Types *****************************************

//********************** Local Constants *************************************

// Dynamic rev = 0.0
// Permanent rev = 0.0
const uint8 kCurrentVersionMajor[2] =
{
    0, 0
};

const uint8 kCurrentVersionMinor[2] =
{
    0, 0
};

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Set this to NULL initially.  This will be set up in my constructor.
BcmEstbAppNonVolSettings *BcmEstbAppNonVolSettings::pfSingletonInstance = NULL;


// Default Constructor.  Initializes the state of the object, setting all
// fields to good default values.
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmEstbAppNonVolSettings::BcmEstbAppNonVolSettings(void) :
    BcmNonVolSettings("CM Application NonVol Settings")
{
    // Override the class name in the message log settings that was set by my
    // base class.
    fMessageLogSettings.SetModuleName("BcmEstbAppNonVolSettings");

    // Just let this do the work.
    ResetDefaults(kDynamicSection);
    ResetDefaults(kPermanentSection);

    // Set myself as the singleton.
    if (pfSingletonInstance == NULL)
    {
        gInfoMsg(fMessageLogSettings, "BcmEstbAppNonVolSettings")
            << "Setting up the singleton pointer." << endl;
        pfSingletonInstance = this;
    }
    else
    {
        gWarningMsg(fMessageLogSettings, "BcmEstbAppNonVolSettings")
            << "Singleton pointer is not NULL!  There are multiple instances!  Leaving the singleton pointer alone..." << endl;
    }
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmEstbAppNonVolSettings::~BcmEstbAppNonVolSettings()
{
    // Clear the singleton pointer.
    if (pfSingletonInstance == this)
    {
        gInfoMsg(fMessageLogSettings, "~BcmEstbAppNonVolSettings")
            << "Clearing the singleton pointer." << endl;

        pfSingletonInstance = NULL;
    }
    else
    {
        gWarningMsg(fMessageLogSettings, "~BcmEstbAppNonVolSettings")
            << "I'm not the singleton instance!  Leaving the singleton pointer alone..." << endl;
    }
}


// Set all values to a known good state.
//
// Note that the derived class should set the version numbers to the desired
// values in case they were changed (by ReadFrom(), etc).
//
// This method may be called if ReadFrom() returns false in order to ensure
// that there are valid settings in the object.
//
// Parameters:
//      section - the section that should be reset.
//
// Returns:  Nothing.
//
void BcmEstbAppNonVolSettings::ResetDefaults(NonVolSection section)
{
    // Set the magic numbers.  This is ASCII-hex for 'CMAp'.
    fMagicNumber = 0x434d4170;

    // Set the current version number.
    fVersionMajor[section] = kCurrentVersionMajor[section];
    fVersionMinor[section] = kCurrentVersionMinor[section];

    // There are no dynamic settings to reset.
    if (section == kPermanentSection)
    {
        // By default, go for fast bootup.  The user doesn't have the option
        // of skiping drivers or stopping at the console.
        fEnableDisableBitmask = kSkipDriverInitPromptDisable | kStopAtAppPromptDisable;
    }
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
BcmEstbAppNonVolSettings *BcmEstbAppNonVolSettings::GetSingletonInstance(void)
{
    if (pfSingletonInstance == NULL)
    {
        gLogMessageRaw
            << "BcmEstbAppNonVolSettings::GetSingletonInstance:  WARNING - the singleton instance is NULL, and someone is accessing it!" << endl;
    }
    
    return pfSingletonInstance;
}


// Here's where I write my members to the buffer in a flat format, in the
// order described at the top of this file.  All multibyte values are
// written in network byte order.
//
// Parameters:
//      outputBuffer - the buffer into which the settings should be written.
//      section - the section that should be written.
//
// Returns:
//      true if successful.
//      false if there was a problem (usually the buffer is too small).
//
bool BcmEstbAppNonVolSettings::WriteToImpl(BcmOctetBuffer &outputBuffer,
                                         NonVolSection section) const
{
    // Append the settings one at a time; if any of these fails, then the buffer
    // is probably too small.
    if (section == kPermanentSection)
    {
        if (!outputBuffer.NetworkAddToEnd(fEnableDisableBitmask))
        {
            // Log the error.
            gErrorMsg(fMessageLogSettings, "WriteToImpl") 
                << "Buffer too small to hold all of the settings!" << endl;

            return false;
        }
    }

    // If we got this far, then it worked!
    return true;
}


// Here's where I read my members from the buffer in a flat format, in the
// order described at the top of this file.  All multibyte values are read
// in network byte order.
//
// The currentOffset parameter tells the derived class where the app data
// starts in the buffer.  It can be used directly in the calls to Read() or
// NetworkRead() in BcmOctetBuffer (which expects an offset value).
//
// Parameters:
//      inputBuffer - the buffer from which the settings should be read.
//      currentOffset - the offset into the buffer from which the settings
//                      should be read.  This is necessary to allow the
//                      derived class to skip the header.
//      section - the section that should be read.
//
// Returns:
//      true if successful.
//      false if there was a problem (version number not supported, not
//          enough bytes in the buffer, etc.).
//
bool BcmEstbAppNonVolSettings::ReadFromImpl(const BcmOctetBuffer &inputBuffer, 
                                          unsigned int &currentOffset,
                                          NonVolSection section)
{
    // Read the settings one at a time; if any of these fails, then the buffer
    // is probably too small.
    if (section == kPermanentSection)
    {
        // For the Permanent Settings, we skipped 0.0, so we should never read
        // this.  If it is 0.0, then this is a problem, so we bail with comment.
        if ((fVersionMajor == 0) && (fVersionMinor == 0))
        {
            gErrorMsg(fMessageLogSettings, "ReadFromImpl")
                << "Read unsupported version (0.0)!  These settings are not valid!" << endl;

            return false;
        }

        if (!inputBuffer.NetworkRead(currentOffset, fEnableDisableBitmask))
        {
            // Log the error.
            gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
                << "Buffer too small; doesn't have all of the settings!" << endl;

            return false;
        }
        
        // Do whatever validation is necessary for the values.
    }
    
    // Call the helper code in the base class to check for version mismatch.
    CheckForVersionUpgrade(kCurrentVersionMajor[section],
                           kCurrentVersionMinor[section], section);

    // If we got this far, then if worked!
    return true;
}


// Causes me to do formatted output of my settings to the specified ostream,
// after the base class does its stuff.
//
// Parameters:
//      outputStream - the ostream to be used.
//
// Returns:
//      The reference to the ostream that was passed in.
//
ostream & BcmEstbAppNonVolSettings::PrintImpl(ostream & outputStream) const
{
    #if !(BCM_REDUCED_IMAGE_SIZE)

    // Dump the settings out one at a time.
    outputStream 
        << "\nCM Application Permanent NonVol Settings:\n\n"
#if (BCM_DSG_SUPPORT)
        << setw(38) << "Disable DSG and Automatically start Docsis = " << AutoStartDocsisOnly() << '\n'
#endif
        << setw(38) << "Automatically stop at console = " << AutoStopAtConsole() << '\n'
        << setw(38) << "'Skip driver init' Prompt Enabled = " << DriverInitPrompt() << '\n'    
        << setw(38) << "'Stop at console' Prompt Enabled = " << StopAtConsolePrompt() << '\n';

    #endif
    
    return outputStream;
}



