//****************************************************************************
//
// Copyright (c) 2005-2012 Broadcom Corporation
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
//
//  Filename:       BcmEstbTestNonVolSettings.cpp
//  Author:         Pinar Taskiran Cyr
//  Creation Date:  Dec 8, 2000
//
//****************************************************************************
//  Description:
//      These are the Non-Volatile settings for Residential Gateway. This is derived from the 
//      abstract base NonVolSettings class, which provides hooks for loading and 
//      storing the settings.
//
//      Note that the settings are independant of the device to which they are
//      being stored, allowing the same application settings to be used on
//      multiple target platforms and devices!
//
//****************************************************************************
 
//********************** Include Files ***************************************

// My api and definitions...
#include "EstbTestNonVolSettings.h"

#include "MergedNonVolDefaults.h"

#include "NonVolDevice.h"
#include "EstbNonVolDefaults.h"
#include "DsgClientId.h"
#include "DsgClientManager.h"
//********************** Local Types *****************************************

//********************** Local Constants *************************************

// Dynamic rev = 0.1
// Permanent rev = 0.1
const uint8 kCurrentVersionMajor[2] =
{
    0, 0
};

const uint8 kCurrentVersionMinor[2] =
{
    1, 1
};

enum
{
	kDummyUint16ClientPortNumber  = 0x40000
};
//********************** Local Variables *************************************

//********************** Class Method Implementations ************************
// Set this to NULL initially.  This will be set up in my constructor.
BcmEstbTestNonVolSettings *BcmEstbTestNonVolSettings::pfSingletonInstance = NULL;


// Default Constructor.  Initializes the state of the object, setting all
// fields to good default values.
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmEstbTestNonVolSettings::BcmEstbTestNonVolSettings(void) :
BcmNonVolSettings("eSTB Test NonVol Settings")
{
    // Override the class name in the message log settings that was set by my
    // base class.
    fMessageLogSettings.SetModuleName("BcmEstbTestNonVolSettings");

    fConstructorIsRunning = true;

     // Just let this do the work.
    ResetDefaults(kDynamicSection);
    ResetDefaults(kPermanentSection);

    fConstructorIsRunning = false;

    // Set myself as the singleton.
    if (pfSingletonInstance == NULL)
    {
        gInfoMsg(fMessageLogSettings, "BcmEstbTestNonVolSettings")
            << "Setting up the singleton pointer." << endl;
        pfSingletonInstance = this;
    }
    else
    {
        gWarningMsg(fMessageLogSettings, "BcmEstbTestNonVolSettings")
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
BcmEstbTestNonVolSettings::~BcmEstbTestNonVolSettings()
{
 
    // Clear the singleton pointer.
    if (pfSingletonInstance == this)
    {
        gInfoMsg(fMessageLogSettings, "~BcmEstbTestNonVolSettings")
            << "Clearing the singleton pointer." << endl;

        pfSingletonInstance = NULL;
    }
    else
    {
        gWarningMsg(fMessageLogSettings, "~BcmEstbTestNonVolSettings")
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
void BcmEstbTestNonVolSettings::ResetDefaults(NonVolSection section)
{
		// Set the magic numbers.  This is ASCII-hex for "ESTB".
		fMagicNumber = 0x54455354;

    // Set the current version number.
    fVersionMajor[section] = kCurrentVersionMajor[section];
    fVersionMinor[section] = kCurrentVersionMinor[section];

    // There are dynamic settings to reset.
    if (section == kDynamicSection)
    {
		#if (defined(CLIENT_CONTROLLER_SUPPORT) || defined(BCM_STANDALONE_DSGCC))
		fDsgClientManager.Reset();
		#endif

        return;
    }

    //default DSG enable mode 
    fDsgMode = kDefaultValue_dsgMode;
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
BcmEstbTestNonVolSettings *BcmEstbTestNonVolSettings::GetSingletonInstance(void)
{
    if (pfSingletonInstance == NULL)
    {
        gLogMessageRaw
            << "BcmEstbTestNonVolSettings::GetSingletonInstance:  WARNING - the singleton instance is NULL, and someone is accessing it!" << endl;
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
uint8 BcmEstbTestNonVolSettings::DsgMode(void) const
{
    return fDsgMode;
}

void BcmEstbTestNonVolSettings::DsgMode(uint8 dsgMode)
{
    fDsgMode = dsgMode;

	/* DO NOT WRITE TO NONVOL YET *******************
    // write to nonvol
    BcmNonVolDevice *pNonVolDevice;

    // If a non-vol device was given to me, then use it.
    pNonVolDevice = BcmNonVolDevice::GetSingletonInstance();
    if(pNonVolDevice->Write(*this, BcmNonVolSettings::kPermanentSection) == false)
    {
       gErrorMsg(fMessageLogSettings, "DsgMode") << "Failed to write the settings!" << endl;
    }
	*/
}

bool BcmEstbTestNonVolSettings::DsgEnabled(void)
{
    if( fDsgMode == kDsgDisabledMode )
	{
		return false;
	}

	return true;
}

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
bool BcmEstbTestNonVolSettings::WriteToImpl(BcmOctetBuffer &outputBuffer, 
                                            NonVolSection section) const
{
    // Output the dynamic section.
    if (section == kDynamicSection)
    {
		#if (defined(CLIENT_CONTROLLER_SUPPORT) || defined(BCM_STANDALONE_DSGCC))
		// Store client list.
		if (!outputBuffer.NetworkAddToEnd((uint32)fDsgClientManager.size()) )
		{
			// Log the error.
			gErrorMsg(fMessageLogSettings, "WriteToImpl") << "Buffer too small to hold all of the settings!" << endl;

			return false;
		}
	
		SubscriberMmapConstIt client_begin = fDsgClientManager.fSubscriberMmap.begin();
		SubscriberMmapConstIt client_end = fDsgClientManager.fSubscriberMmap.end();

		while( client_begin != client_end )
		{
			// Only store dummy clients added from console.
			if( client_begin->second->ClientPort() >= 65536 )
			{
				uint8 idType = client_begin->second->IdType();

				// Write client ID type.
				if (!outputBuffer.NetworkAddToEnd(idType))
				{
					// Log the error.
					gErrorMsg(fMessageLogSettings, "WriteToImpl") << "Buffer too small to hold all of the settings!" << "idType = " << idType<< endl;

					return false;
				}

				// Write client ID.
				if( client_begin->second->IdType() == kMacAddressType )
				{
					if( !client_begin->second->ClientAddress().WriteTo(outputBuffer) )
					{
						// Log the error.
						gErrorMsg(fMessageLogSettings, "WriteToImpl") << "Buffer too small to hold all of the settings!" << endl;

						return false;
					}
				}
				else if(client_begin->second->IdType() == kApplicationNameType)
				{
					// Write length and name. Do not include null terminator.
					if( !outputBuffer.NetworkAddToEnd( (uint8) strlen((client_begin->second)->ClientName().c_str()) ) ||
						!outputBuffer.AddToEnd( (char*) (client_begin->second)->ClientName().c_str(), false)            
					  )
					{
						gErrorMsg(fMessageLogSettings, "WriteToImpl")
							<< "Buffer too small to hold all of the settings!" << endl;
						return false;
					}
				}
				else
				{
					if (!outputBuffer.NetworkAddToEnd( (uint16) client_begin->second->Id()))
					{
						// Log the error.
						gErrorMsg(fMessageLogSettings, "WriteToImpl") << "Buffer too small to hold all of the settings!" << endl;

						return false;
					}			
				}
			}

			client_begin++;
			
		}

		#endif

        return true;
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
bool BcmEstbTestNonVolSettings::ReadFromImpl(const BcmOctetBuffer &inputBuffer, 
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

    // Do the dynamic section first
    if (section == kDynamicSection)
    {

		if (MyVersionIsGreaterThanOrEqualTo(0, 1, section))
		{
			uint32 fNumDsgClients = 0;

			#if (defined(CLIENT_CONTROLLER_SUPPORT) || defined(BCM_STANDALONE_DSGCC))
			// Read number of clients.
			if (!inputBuffer.NetworkRead(currentOffset, fNumDsgClients))
			{
				// Log the error.
				gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
					<< "Buffer too small; doesn't have all of the Dynamic settings!" << endl;
				return false;
			}

			// Make sure the list is empty.
			fDsgClientManager.Reset();

			// Start reading clients.
			while(fNumDsgClients)
			{	
				// kDummyUint16ClientPortNumber is assigned to clients created using the console. Add 1000 to prevent duplication.
				static uint32 portNumber = kDummyUint16ClientPortNumber + 1000;				

				BcmDsgClient* pClient = new BcmDsgClient;
				uint8 idType;

				// Read client type.				
				if (!inputBuffer.NetworkRead(currentOffset, idType))
				{
					// Log the error.
					gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
						<< "Buffer too small; doesn't have all of the Dynamic settings!" << endl;
					return false;
				}

				pClient->IdType((uint32) idType);

				if( pClient->IdType() == kMacAddressType )
				{
					BcmMacAddress id(kAllZerosMac);

					// Read client ID in MAC address format.
					if( !id.ReadFrom(inputBuffer, currentOffset))
					{
						// Log the error.
						gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
							<< "Buffer too small; doesn't have all of the Dynamic settings!" << endl;
						return false;
					}

					pClient->ClientAddress(id);
				}
				else if( pClient->IdType() == kApplicationNameType)
				{
					BcmOctetBuffer appName(0);
					uint8 appNameLength = 0;
					// Read the category string length.
					if ( inputBuffer.NetworkRead(currentOffset, appNameLength) == false )
					{
						gAlwaysMsgNoFields(fMessageLogSettings) << " Error reading appNameLength "<< endl;
						// Log the error.
						gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
							<< "Buffer too small; doesn't have all of the settings!" << endl;

						return false;
					}

					// Adjust the buffer to hold the new category size.
					appName.ResizeBuffer(appNameLength + 1);

					// Read the category.
					if ( inputBuffer.Read(currentOffset, appName, (unsigned int) appNameLength) == false )
					{
						gAlwaysMsgNoFields(fMessageLogSettings) << " Error reading appName "<< endl;
						// Log the error.
						gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
							<< "Buffer too small; doesn't have all of the settings!" << endl;

						return false;
					}

					strncpy( (char*)(appName.AccessBuffer()+ appName.BytesUsed()), "\0", 1);
					pClient->ClientName(appName);

				}
				else 
				{
					uint16 clientId;

					// Read client ID,
					if (!inputBuffer.NetworkRead(currentOffset, clientId))
					{
						// Log the error.
						gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
							<< "Buffer too small; doesn't have all of the Dynamic settings!" << endl;
						return false;
					}

					pClient->Id((uint32) clientId);
				}

				// Set unique port number.
				pClient->ClientPort(portNumber++);

				// Insert client into the list.	
				// Clients tunnel ID is zero. Not subscribed to any tunnel yet.
				uint32 tunnelId = 0;
				fDsgClientManager.Subscribe( tunnelId, pClient);
								
				fNumDsgClients--;
			}
			#endif
		}

		// Call the helper code in the base class to check for version mismatch.
		CheckForVersionUpgrade(kCurrentVersionMajor[section],
			kCurrentVersionMinor[section], section);

        return true;
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
ostream & BcmEstbTestNonVolSettings::PrintImpl(ostream & outputStream) const
{
    // Dump the settings out one at a time.
    outputStream
        << "\neSTB Test Dynamic NonVol Settings:\n\n"	
		<< "\n"

	#if (defined(CLIENT_CONTROLLER_SUPPORT) || defined(BCM_STANDALONE_DSGCC))
		<< fDsgClientManager 
	#endif

		<< endl;

	return outputStream;
}
 



