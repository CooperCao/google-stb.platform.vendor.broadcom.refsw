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
//
//  Filename:       DsgccNonVolSettings.cpp
//  Author:         Kenny Lee
//  Creation Date:  Mar 15, 2007
//
//****************************************************************************
//  Description:
//      These are the Non-Volatile settings for Brcm Dsg Client Controller. 
//      This is derived from the abstract base NonVolSettings class, which 
//      provides hooks for loading and storing the settings.
//
//      Note that the settings are independant of the device to which they are
//      being stored, allowing the same application settings to be used on
//      multiple target platforms and devices!
//
//****************************************************************************
 
//********************** Include Files ***************************************

// My api and definitions...
#include "DsgccNonVolSettings.h"
#include "MergedNonVolDefaults.h"
#include "NonVolDevice.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

// Dynamic rev = 0.4
// Permanent rev = 0.2
const uint8 kCurrentVersionMajor[2] =
{
    0, 0
};

#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE)
const uint8 kCurrentVersionMinor[2] =
{
    2, 6
};
#else
const uint8 kCurrentVersionMinor[2] =
{
    2, 5
};
#endif

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************
// Set this to NULL initially.  This will be set up in my constructor.
BcmDsgccNonVolSettings *BcmDsgccNonVolSettings::pfSingletonInstance = NULL;


// Default Constructor.  Initializes the state of the object, setting all
// fields to good default values.
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmDsgccNonVolSettings::BcmDsgccNonVolSettings(void) :
BcmNonVolSettings("Dsgcc NonVol Settings")
{
    // Override the class name in the message log settings that was set by my
    // base class.
    fMessageLogSettings.SetModuleName("BcmDsgccNonVolSettings");

    fConstructorIsRunning = true;

     // Just let this do the work.
    ResetDefaults(kDynamicSection);
    ResetDefaults(kPermanentSection);

    fConstructorIsRunning = false;

    // Set myself as the singleton.
    if (pfSingletonInstance == NULL)
    {
        gInfoMsg(fMessageLogSettings, "BcmDsgccNonVolSettings")
            << "Setting up the singleton pointer." << endl;
        pfSingletonInstance = this;
    }
    else
    {
        gWarningMsg(fMessageLogSettings, "BcmDsgccNonVolSettings")
            << "Singleton pointer is not NULL!  There are multiple instances!  Leaving the singleton pointer alone..." << endl;
    }

#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE)
	// Initialize the interface name
	memset((void *)fIpcInterfaceName, 0, IPC_INTERFACE_NAME_MAX_LEN);
#endif
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmDsgccNonVolSettings::~BcmDsgccNonVolSettings()
{
 
    // Clear the singleton pointer.
    if (pfSingletonInstance == this)
    {
        gInfoMsg(fMessageLogSettings, "~BcmDsgccNonVolSettings")
            << "Clearing the singleton pointer." << endl;

        pfSingletonInstance = NULL;
    }
    else
    {
        gWarningMsg(fMessageLogSettings, "~BcmDsgccNonVolSettings")
            << "I'm not the singleton instance!  Leaving the singleton pointer alone..." << endl;
    }
}


// Allows the derived class to set its values to a known good state.  This
// MUST be implemented by the derived class.
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
void BcmDsgccNonVolSettings::ResetDefaults(NonVolSection section)
{
	// Set the magic numbers.  This is ASCII-hex for "ESTB".
	fMagicNumber = 0x45535442;

    // Set the current version number.
    fVersionMajor[section] = kCurrentVersionMajor[section];
    fVersionMinor[section] = kCurrentVersionMinor[section];

    // There are no dynamic settings to reset.
    if (section == kDynamicSection)
    {
        return;
    }

    // Do not delete! We have to maintain the flash locations for backward compatibility
    // The locations that were not used by Dsgcc is marked unused now for later use
    fDsgccNonVolU32NotUsed1 = fDsgccNonVolU32NotUsed2 = 0;
	fDsgccNonVolU8NotUsed3 = fDsgccNonVolU8NotUsed4 = fDsgccNonVolU8NotUsed5 = 0;

	// PR 8572: Allow dsg mode non-vol object to be set in a 
	// header file rather than hardcoded.
    //default DSG enable mode to disabled
    fDsgMode = kDefaultValue_dsgMode;

#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE)
	strcpy(fIpcInterfaceName, kDefaultEcmInterfaceName);
#endif
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
BcmDsgccNonVolSettings *BcmDsgccNonVolSettings::GetSingletonInstance(void)
{
    if (pfSingletonInstance == NULL)
    {
        gLogMessageRaw
            << "BcmDsgccNonVolSettings::GetSingletonInstance:  WARNING - the singleton instance is NULL, and someone is accessing it!" << endl;
    }
    
    return pfSingletonInstance;
}

// Accessors to get/set the DSG global enable bit.
//      mode =
//      enum
//      {
//  	    kDsgDisabledMode = 0,
//  	    kDsgBasicMode,
//  	    kDsgAdvanceMode
//      };
//
// Parameters:
//      
//
// Returns:
//
uint8 BcmDsgccNonVolSettings::DsgMode(void) const
{
    return fDsgMode;
}

void BcmDsgccNonVolSettings::DsgMode(uint8 dsgMode)
{
    fDsgMode = dsgMode;
}

#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE)
// Accessors to get/set IPC interface name
//
//
const char * BcmDsgccNonVolSettings::IpcInterfaceName(void)
{
	return fIpcInterfaceName;
}

void BcmDsgccNonVolSettings::IpcInterfaceName(const char *name)
{
	if(strlen(name) < IPC_INTERFACE_NAME_MAX_LEN)
	{
		strcpy(fIpcInterfaceName, name);
	}
}
#endif

// This is where the derived class writes its members to the buffer in a
// flat format.  It is up to the derived class to determine what order,
// what data size, whether to use network or host byte order, etc.
//
// This MUST be implemented by the derived class.
//
// Parameters:
//      outputBuffer - the buffer into which the settings should be written.
//      section - the section that should be written.
//
// Returns:
//      true if successful.
//      false if there was a problem (usually the buffer is too small).
//
bool BcmDsgccNonVolSettings::WriteToImpl(BcmOctetBuffer &outputBuffer, 
                                            NonVolSection section) const
{
    // Output the dynamic section.
    if (section == kDynamicSection)
    {
		if (!outputBuffer.NetworkAddToEnd((uint16) fDsgDownloadCVC.BytesUsed()) ||
			!outputBuffer.Append(fDsgDownloadCVC))
		{
			// Log the error.
			gErrorMsg(fMessageLogSettings, "WriteToImpl") << "Buffer too small to hold all of the settings!" << endl;

			return false;
		}
        return true;
    }

	// Output the permanent section
    if (!outputBuffer.NetworkAddToEnd(fDsgccNonVolU32NotUsed1) ||
        !outputBuffer.NetworkAddToEnd(fDsgMode) ||
        !outputBuffer.NetworkAddToEnd(fDsgccNonVolU32NotUsed2) ||
		!outputBuffer.NetworkAddToEnd(fDsgccNonVolU8NotUsed3) ||
		!outputBuffer.NetworkAddToEnd(fDsgccNonVolU8NotUsed4) ||
		!outputBuffer.NetworkAddToEnd(fDsgccNonVolU8NotUsed5) ||
#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE)
		!outputBuffer.AddToEnd((const uint8 *)fIpcInterfaceName, IPC_INTERFACE_NAME_MAX_LEN) ||
#endif
		0)
    {
        // Log the error.
        gErrorMsg(fMessageLogSettings, "WriteToImpl") << "Buffer too small to hold all of the settings!" << endl;

        return false;
    }

    // If we got this far, then it worked!
    return true;
}


// This is where the derived class reads its members from the buffer in a
// flat format.  It is up to the derived class to determine what ordering,
// etc., should be used.
//
// The currentOffset parameter tells the derived class where the app data
// starts in the buffer.  It can be used directly in the calls to Read() or
// NetworkRead() in BcmOctetBuffer (which expects an offset value).
//
// This MUST be implemented by the derived class.
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
bool BcmDsgccNonVolSettings::ReadFromImpl(const BcmOctetBuffer &inputBuffer, 
                                             unsigned int &currentOffset, 
                                             NonVolSection section)
{

	uint16 length;

	// For this object, we skipped 0.0 for both dynamic and permanent, so this
	// should never be read.  If it is 0.0, then this is a problem, so we bail
	// with comment.
	if ((fVersionMajor[section] == 0) && (fVersionMinor[section] == 0))
	{
		gErrorMsg(fMessageLogSettings, "ReadFromImpl")
			<< "Read unsupported version (0.0)!  These settings are not valid!" << endl;

		return false;
	}

    // Nothing to do in the dynamic section
    if (section == kDynamicSection)
    {
        return true;
    }

    // Read the settings for version 0.1 one at a time; if any of these fails,
    // then the buffer is probably too small.
    if (!inputBuffer.NetworkRead(currentOffset, fDsgccNonVolU32NotUsed1) ||
        !inputBuffer.NetworkRead(currentOffset, fDsgMode) ||
        !inputBuffer.NetworkRead(currentOffset, fDsgccNonVolU32NotUsed2) )
    {
        // Log the error.
        gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
            << "Buffer too small; doesn't have all of the settings!" << endl;

        return false;
    }
    
    // Read the settings for version 0.2.
    if (MyVersionIsGreaterThanOrEqualTo(0, 2, section))
    {
		// Read the first length field.
		if (!inputBuffer.NetworkRead(currentOffset, fDsgccNonVolU8NotUsed3))
		{
			// Log the error.
			gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
				<< "Buffer too small; doesn't have all of the Permanent settings!" << endl;
			return false;
		}
	}

	if (MyVersionIsGreaterThanOrEqualTo(0, 3, section))
	{
		if (!inputBuffer.NetworkRead(currentOffset, fDsgccNonVolU8NotUsed4))
		{
			// Log the error.
			gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
				<< "Buffer too small; doesn't have all of the Permanent settings!" << endl;
			return false;
		}
	}

    // Read the settings for version 0.5.
    if (MyVersionIsGreaterThanOrEqualTo(0, 5, section))
    {
		// Read the first length field.
		if (!inputBuffer.NetworkRead(currentOffset, fDsgccNonVolU8NotUsed5))
		{
			// Log the error.
			gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
				<< "Buffer too small; doesn't have all of the Permanent settings!" << endl;
			return false;
		}
	}

#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE)
    // Read the settings for version 0.6.
    if (MyVersionIsGreaterThanOrEqualTo(0, 6, section))
    {
		// Read the interface name including the null character
		if (!inputBuffer.Read(currentOffset, (uint8 *)&fIpcInterfaceName[0], IPC_INTERFACE_NAME_MAX_LEN))
		{
			// Log the error.
			gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
				<< "Buffer too small; doesn't have all of the Permanent settings!" << endl;
			return false;
		}
	}
#endif	

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
ostream & BcmDsgccNonVolSettings::PrintImpl(ostream & outputStream) const
{
    // Dump the settings out one at a time.
    outputStream
        << "\nDsgcc Permanent NonVol Settings:\n\n"
        << "DSG Configuration Settings:\n" << setw(30)
		<< "                       Mode = " << ((fDsgMode == kDsgDisabledMode) ? "DISABLED" : 
    											((fDsgMode == kDsgBasicMode) ? "BASIC" :
	    										((fDsgMode == kDsgAdvanceMode) ? "ADVANCED" :"NOT RECOGNIZED") )) << "\n"
	#if defined ( BCM_DSG_DUAL_PROCESSOR_INTERFACE)
		<< "\neCM-eSTB IPC interface:  " << fIpcInterfaceName << "\n"
	#endif
		<< "\n";

	return outputStream;
}

