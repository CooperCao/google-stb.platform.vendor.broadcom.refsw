//****************************************************************************
//
// Copyright (c) 2004-2012 Broadcom Corporation
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
//  Filename:       DsgDownloadCtlThread.cpp
//  Author:         Jean Cyr, Pinar Taskiran
//  Creation Date:  November 22, 2004
//
//****************************************************************************

//********************** Include Files ***************************************

#include	<sys/types.h>	/* basic system data types */
#if defined(WIN32)
#include	<WinSock2.h>	/* basic socket definitions */
#else
#include	<sys/socket.h>	/* basic socket definitions */
#endif
#ifdef TARGETOS_vxWorks
#include	<socklib.h>
#endif
#ifdef __linux__
#include	<sys/time.h>	/* timeval{} for select() */
#endif
#if !defined(WIN32)
#include	<sys/ioctl.h>
#include	<unistd.h>
#include	<netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include	<arpa/inet.h>	/* inet(3) functions */
#include	<net/if.h>
#endif
#include	<time.h>		/* timespec{} for pselect() */
#ifdef TARGETOS_vxWorks
#include	<inetLib.h>
#endif
#include	<iostream>
#include	<assert.h>
#include	<string.h>

// My api and definitions...
#include "DsgDownloadCtlThread.h"
#include "EstbRemoteInterfaceStub.h" 
#include "DsgClientCtlThread.h"

// OS stuff.
#include "OperatingSystemFactory.h"

// IP Helper stuff.

// Other stuff.
//#include "PacketSwitch.h"
#include "OctetBuffer.h"

#include "EstbCannedSnmp.h"

#ifdef MAC_STUB
    #include "HepDebug.h"
    #include "GlEventSim.h"
#endif

#if defined(WIN32)
#include <algorithm>
#else
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <algorithm>
#else
#include <algo.h>
#endif
#endif
#include "ProgramStoreDeviceDriver.h"

#if defined(WIN32)
#define bzero(a, l) memset(a, 0, l)
#endif

//********************** Local Types *****************************************
#if defined(TARGETOS_vxWorks) || defined(WIN32)
typedef int socklen_t;
#endif

//********************** Local Constants *************************************

// Some application dependent defines. These would be replaced by the customer
#define SIGNED_IMAGE_FILENAME "signed_ecram_sto.bin"

//********************** Local Variables *************************************

//********************** Global Variables *************************************
// Set to NULL. Will be set up by constructor.
BcmDsgDownloadCtlThread *BcmDsgDownloadCtlThread::pfSingletonInstance = NULL;
unsigned long gLocalIpAddress = 0;

//********************** Local Variables *************************************
static bool downloadInProgress = false;
//********************** Local Functions *************************************

//****************** Extern/Local Functions prototypes ************************
#if defined (ESTB_RSWDLOAD_SUPPORT)
extern "C"
{
	int InitializeRemoteProgramStoreHost(char* pInterfaceName, uint32 localRxIpAddress, uint32 remoteEcmIpAddress);

	void CommonDownloadSetEcmNotificationCallback( void (*callback)(CommonDownloadExtNotificationMsg)) ;
}
#endif

//****************** Extern Variables**************** ************************
#if (BCM_DSG_DUAL_PROCESSOR_INTERFACE)
extern char EstbPrimaryIpStackName[IPC_INTERFACE_NAME_MAX_LEN];
#endif
//********************** Class Method Implementations ************************

// BcmDsgDownloadCtlThread() - Object constructor
//		Initiates thread and initializes singleton.
//
// Parameters:
//      initialPriority - Initial thread priority.
//
// Returns:
//      None.
//
BcmDsgDownloadCtlThread::BcmDsgDownloadCtlThread(BcmOperatingSystem::ThreadPriority initialPriority) :
    BcmThread("BcmThread", false, initialPriority),
	pfEstbCannedSnmp(NULL)
{
    // Override the class name given by my parent.
    fMessageLogSettings.SetModuleName("BcmDsgDownloadCtlThread");

   // Set up the singleton.
    if (pfSingletonInstance != NULL)
    {
        gWarningMsg(fMessageLogSettings, "BcmDsgDownloadCtlThread")
            << "Singleton pointer is not NULL!  There are multiple BcmDsgDownloadCtlThread!  Leaving the singleton pointer alone..." << endl;
    }
    else
    {
        gInfoMsg(fMessageLogSettings, "BcmDsgDownloadCtlThread")
            << "Setting up singleton pointer." << endl;

        pfSingletonInstance = this;
    }

	pfEstbCannedSnmp = BcmEstbCannedSnmp::GetSingletonInstance();	
	if(!pfEstbCannedSnmp)
	{
        gFatalErrorMsg( fMessageLogSettings, "BcmDsgDownloadCtlThread" ) 
            << "Failed to get a pointer to Canned SNMP module!" << endl;
        assert(0);
    }

    // Spawn my thread.
#if defined(TARGETOS_vxWorks) || defined(WIN32)
    if (pfOperatingSystem->BeginThread(this, 16 * 1024) == false)
#else
    if (pfOperatingSystem->BeginThread(this, 8 * 1024) == false)
#endif
    {
        gFatalErrorMsg( fMessageLogSettings, "BcmDsgDownloadCtlThread" ) 
            << "Failed to spawn my thread!" << endl;
        assert(0);
    }
}

// ~BcmDsgDownloadCtlThread() - Object constructor
//		Signals thread and waits for termination.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
BcmDsgDownloadCtlThread::~BcmDsgDownloadCtlThread()
{
    gFuncEntryExitMsg( fMessageLogSettings, "~BcmDsgDownloadCtlThread" ) << "Entering..." << endl;    

    gInfoMsg( fMessageLogSettings, "~BcmDsgDownloadCtlThread" ) 
        << "Signalling the thread to exit..." << endl;

    // Wait for the thread to exit.
    WaitForThread();

    gFuncEntryExitMsg(fMessageLogSettings, "~BcmDsgDownloadCtlThread") << "Exiting..." << endl;
}

// Initialize() - Initialize thread
//		This is the first method called after the thread has
//		been spawned, and is where the thread should create all OS objects.
//		This has to be done here, rather than in the object's constructor,
//		because some OS objects must be created in the context of the thread
//		that will use them.  The object's constructor is still running in the
//		context of the thread that created this object.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
bool BcmDsgDownloadCtlThread::Initialize()
{
    gFuncEntryExitMsg( fMessageLogSettings, "Initialize" ) << "Entering..." << endl;

    gFuncEntryExitMsg( fMessageLogSettings, "Initialize" ) << "Exiting..." << endl;

    return true;
}

// GetECMVersion(uint16 &major, uint16 &minor, uint16 &subMinor)
//		Using SNMP, retrieve the version of the image currently running in the eCM
//
// Parameters:
//      major - Major version number.
//      minor - Minor version number.
//      subMinor - Minor sub-version number.
//
// Returns:
//      None. If fails then major, minor and subMinor are returned as 0
//
bool BcmDsgDownloadCtlThread::GetECMVersion(uint16 &major, uint16 &minor, uint16 &subMinor)
{
	return pfEstbCannedSnmp->Get_docsDevSwCurrentVers(major, minor, subMinor);
}

// SetImageFileName(const char *fileName)
//		Create, send, and verify and SNMP SET for the docsDevSwFilename MIB.
//
// Parameters:
//      fileName - Pinter to null terminated file name ascii string.
//
// Returns:
//      true - All is well
//		False - Something didn't work
//
bool BcmDsgDownloadCtlThread::SetImageFileName(const char *fileName)
{
	return pfEstbCannedSnmp->Set_docsDevSwFilename(fileName);
}

static void GetLocalIpAddress()
{

#if defined(USE_PRIVATE_NETWORK)
	gLocalIpAddress = kLocalRxIpAddress;
#else
	GetEstbIpAddress(&gLocalIpAddress);
#endif

}

// SetServerIpAddress()
//		Create, send, and verify and SNMP SET for the docsDevSwServer MIB. The
//		IP address used will always be 192.168.100.1 since this is the standard
//		address used by the eCM for early SNMP access.
//
// Parameters:
//      none.
//
// Returns:
//      true - All is well
//		False - Something didn't work
//
bool BcmDsgDownloadCtlThread::SetServerIpAddress()
{
	return pfEstbCannedSnmp->Set_docsDevSwServer(gLocalIpAddress);
}

// InitiateSecureDownload()
//		Create, send, and verify and SNMP SET for the docsDevSwAdminStatus MIB. The
//		MIB will always be set to one since this is the value we need to initiate a
//		secure software download.
//
// Parameters:
//      none.
//
// Returns:
//      true - All is well
//		False - Something didn't work
//
bool BcmDsgDownloadCtlThread::InitiateSecureDownload()
{
	return pfEstbCannedSnmp->Set_docsDevSwAdminStatus(1);
}

// GetOperStatus(uint16 &status)
//		Create, send, and verify and SNMP GET to retrieve the docsDevSwOperStatus MIB.
//
// Parameters:
//      status - The mib value returned.
//
// Returns:
//      true - All is well
//		False - Something didn't work
//
bool BcmDsgDownloadCtlThread::GetOperStatus(uint16 &status)
{
	return pfEstbCannedSnmp->Get_docsDevSwOperStatus(status);
}

// BCDToBin(uint16 bcd)
//		Convert 16 bit binary code decival value to 16 bit binary value
//
// Parameters:
//      bcd - 16 bit binary code decimal value.
//
// Returns:
//      16 bit binary value.
//
static uint16 BCDToBin(uint16 bcd)
{
	uint16 bin = 0, digit;

	for (digit = 0; digit < 4; digit++)
	{
		bin = (bin * 10) + ((bcd & 0xf000) >> 12);
		bcd <<= 4;
	}
	return bin;
}

// GetLocalFileVersion(const char *fileName, uint16 &major, uint16 &minor, uint16 &subMinor)
//		Parse image stored in local file system and extract the version of the image.
//
// Parameters:
//		fileName - Pointer to null terminated file name string.
//      major - Major version number.
//      minor - Minor version number.
//      subMinor - Minor sub-version number.
//
// Returns:
//      None. If fails then major, minor and subMinor are returned as 0
//
void BcmDsgDownloadCtlThread::GetLocalFileVersion(const char *fileName, uint16 &major, uint16 &minor, uint16 &subMinor)
{
	major = 0;
	minor = 0;
	subMinor = 0;

	// NOTE: This is very implementation dependent. This reference design assumes
	// a local file system where the signed image exists as a file called
	// signed_ecram_sto.bin.
	FILE *fh;
	fh = fopen(fileName, "r");
	if (fh == NULL)
	{
		gErrorMsg( fMessageLogSettings, "GetLocalFileVersion" )
			<< "Unable to open eCM signed image!" << endl;
		return;
	}

	do { // Do once. This construct provides a way to bail simply by doing a 'break'.

		// Read the TLV tag which should be 0x30 (PKCS data)
		uint8 len8;
		size_t bytesRead = fread(&len8, 1, sizeof(len8), fh);
		if (bytesRead < sizeof(len8))
		{
			gErrorMsg( fMessageLogSettings, "GetLocalFileVersion" )
				<< "Attempt to read pas end of eCM signed image!" << endl;
			break;
		}
		if (len8 != 0x30)
		{
			gErrorMsg( fMessageLogSettings, "GetLocalFileVersion" )
				<< "eCM signed image is missing PKCS tag!" << endl;
			break;
		}
		// Read the TLV length
		uint16 len16;
		bytesRead = fread(&len8, 1, sizeof(len8), fh);
		if (bytesRead < sizeof(len8))
		{
			gErrorMsg( fMessageLogSettings, "GetLocalFileVersion" )
				<< "Attempt to read pas end of eCM signed image!" << endl;
			break;
		}
		// Check for extended lenth field
		if (len8 & 0x80)
		{
			// Extended length field, compute the actual length
			len16 = 0;
			uint8 count = len8 & 0x7f;
			bool error = false;
			while (count--)
			{
				len16 <<= 8;
				bytesRead = fread(&len8, 1, sizeof(len8), fh);
				if (bytesRead < sizeof(len8))
				{
					gErrorMsg( fMessageLogSettings, "GetLocalFileVersion" )
						<< "Attempt to read pas end of eCM signed image!" << endl;
					error = true;
					break;
				}
				len16 |= len8;
			}
			if (error)
				break;
		}
		else
		{
			// Simple length, use it
			len16 = len8;
		}

		// Seek past the PKCS
		if (fseek(fh, len16, SEEK_CUR) != 0)
		{
			gErrorMsg( fMessageLogSettings, "GetLocalFileVersion" )
				<< "Attempt to read pas end of eCM signed image!" << endl;
			break;
		}

		// Read the 1st byte of the download header
		bytesRead = fread(&len8, 1, sizeof(len8), fh);
		if (bytesRead < sizeof(len8))
		{
			gErrorMsg( fMessageLogSettings, "GetLocalFileVersion" )
				<< "Attempt to read pas end of eCM signed image!" << endl;
			break;
		}
		if (len8 != 0x1c)
		{
			gErrorMsg( fMessageLogSettings, "GetLocalFileVersion" )
				<< "eCM signed image is missing download header!" << endl;
			break;
		}

		// Read the length of the download header
		bytesRead = fread(&len16, 1, sizeof(len16), fh);
		if (bytesRead < sizeof(len16))
		{
			gErrorMsg( fMessageLogSettings, "GetLocalFileVersion" )
				<< "Attempt to read pas end of eCM signed image!" << endl;
			break;
		}

		// Skip past download header
		if (len16)
		{
			if (fseek(fh, len16, SEEK_CUR) != 0)
			{
				gErrorMsg( fMessageLogSettings, "GetLocalFileVersion" )
					<< "Attempt to read pas end of eCM signed image!" << endl;
				break;
			}
		}

		// Finally! We are at the image. Read the header
		BcmProgramHeader header;
		bytesRead = fread(&header, 1, sizeof(header), fh);
		if (bytesRead < sizeof(header))
		{
			gErrorMsg( fMessageLogSettings, "GetLocalFileVersion" )
				<< "Attempt to read pas end of eCM signed image!" << endl;
			break;
		}

		major = BCDToBin(HostToNetwork(header.usMajorRevision));
		minor = BCDToBin(HostToNetwork(header.usMinorRevision) >> 8);
		subMinor = BCDToBin(HostToNetwork(header.usMinorRevision) & 0x7f);

	} while (0);

	fclose(fh);

}

// ThreadMain()
//		Implements the execution thread. Retrieves the local image version and compare it
//		to the eCM image version. Upgrade the eCM if it is running an older version.
//
// Parameters:
//		None.
//
// Returns:
//      None.
//
void BcmDsgDownloadCtlThread::ThreadMain()
{
	gFuncEntryExitMsg( fMessageLogSettings, "ThreadMain" ) << "Entering..." << endl;

	gAlwaysMsg( fMessageLogSettings, "ThreadMain" ) << "Starting DSG download control thread." << endl;

#if defined (UPGRADE_ECM_AFTER_REBOOT)
	// Sleep for not other purpose but to make the console and event log pretty
	// Also allows of any IP reconfiguration to occur
	BcmOperatingSystemFactory::ThreadSleep(5000);

	CheckForEcmUpgrade();

	// Now tell DSG-CC to start. 
	BcmDsgClientCtlThread::GetSingletonInstance()->Start();
#endif

#if !(DSGCC_BUILT_AS_LIB)
	gAlwaysMsg( fMessageLogSettings, "ThreadMain" ) << "SETTING NOTIFICATION CALLBACK FUNCTION!" << endl;
	CommonDownloadSetEcmNotificationCallback(CdlNotificationCallback);
#endif

#if defined (ESTB_RSWDLOAD_SUPPORT)

	gAlwaysMsg( fMessageLogSettings, "ThreadMain" ) << "Initializing Remote Program Store Host.." << endl;

	// Start Remote Program Storage Host Application.
	InitializeRemoteProgramStoreHost(EstbPrimaryIpStackName, kLocalRxIpAddress, kRemoteEcmIpAddress);
#endif

	gFuncEntryExitMsg( fMessageLogSettings, "ThreadMain" ) << "Exiting..." << endl;
}

// CheckForEcmUpgrade()
//		This functions upgrades eCM software via SNMP if an upgrade is available.
//		Note that this function is not part of Common Download API. 
//		eCM will respond to these SNMP sets only in initial state. Once eCM is told to scan
//		it cannot be upgraded via this function.
//
// Parameters:
//		None.
//
// Returns:
//      None.
//
void BcmDsgDownloadCtlThread::CheckForEcmUpgrade(void)
{
	uint16 majorVersion;
	uint16 minorVersion;
	uint16 subMinorVersion;

#if defined(USE_PRIVATE_NETWORK)
	// Set temporary route such that we can reach ip stack 2 on the eCM
	BcmAddRoute(0xC0A86401, EstbPrimaryIpStackName, false);
#endif

	// Initialize the SNMP request id sequence
	fRequestId = 1234;

	do { // Do once. This construct provides a way to bail simply by doing a 'break'.

		// Retrieve eCM image version from local filesystem
		const char *fnPtr = SIGNED_IMAGE_FILENAME;
		GetLocalFileVersion(fnPtr, majorVersion, minorVersion, subMinorVersion);
		if ((majorVersion == 0) && (minorVersion == 0) && (subMinorVersion == 0)) {
			gErrorMsg( fMessageLogSettings, "ThreadMain" )
				<< "Could not determine version of signed image " << fnPtr << endl;
			break;
		}
		gAlwaysMsg( fMessageLogSettings, "ThreadMain" )
			<< "'" << fnPtr << "' version = "
			<< majorVersion << "." << minorVersion << "." << subMinorVersion << endl;

		GetLocalIpAddress();

		// Retrieve eCM image version from eCM
		if (!GetECMVersion(fMajorVersion, fMinorVersion, fSubMinorVersion)) {
			gErrorMsg( fMessageLogSettings, "ThreadMain" )
				<< "Could not determine version of eCM image " << endl;
			break;
		}
		gAlwaysMsg( fMessageLogSettings, "ThreadMain" ) 
			<< "eCM version = "
			<< fMajorVersion << "." << fMinorVersion << "." << fSubMinorVersion << endl;

		bool upgrade = false;
		if (majorVersion > fMajorVersion)
		{
			upgrade = true;
		}
		else if (majorVersion == fMajorVersion) {
			if (minorVersion > fMinorVersion)
			{
				upgrade = true;
			}
			else if (minorVersion == fMinorVersion) {
				if (subMinorVersion > fSubMinorVersion)
				{
					upgrade = true;
				}
			}
		}

		if (!upgrade)
		{
			gAlwaysMsg( fMessageLogSettings, "ThreadMain" )
				<< "eCM does not require upgrade" << endl;
			break;
		}

		gAlwaysMsg( fMessageLogSettings, "ThreadMain" )
			<< "Initiating eCM upgrade" << endl;

		if (!SetImageFileName(fnPtr)) {
			gAlwaysMsg( fMessageLogSettings, "ThreadMain" )
				<< "Unable to set eCM's image file name" << endl;
			break;
		}
		if (!SetServerIpAddress()) {
			gAlwaysMsg( fMessageLogSettings, "ThreadMain" )
				<< "Unable to set eCM's server address" << endl;
			break;
		}
		if (!InitiateSecureDownload()) {
			gAlwaysMsg( fMessageLogSettings, "ThreadMain" )
				<< "Unable to initiate download" << endl;
			break;
		}
		uint16 status;
		bool displayInProgress = true;
		bool result;
#if defined(WIN32)
		Sleep(2000);
#else
		sleep(2);
#endif
		while ((result = GetOperStatus(status))) {
			if (status == 1) {  // in-progress
				if (displayInProgress) {
					displayInProgress = false;
					gAlwaysMsg( fMessageLogSettings, "ThreadMain" )
						<< "Download in progress" << endl;

					downloadInProgress = true;
				}
			}
			else if (status == 4) { // failed
				gAlwaysMsg( fMessageLogSettings, "ThreadMain" )
					<< "Download in failed" << endl;

				downloadInProgress = false;
				break;
			}
#if defined(WIN32)
			Sleep(2000);
#else
			sleep(2);
#endif
		}
		if (!result) {
			gAlwaysMsg( fMessageLogSettings, "ThreadMain" )
				<< "Download completed, waiting for eCM to reset" << endl;
		}
		while (!GetECMVersion(fMajorVersion, fMinorVersion, fSubMinorVersion)) {
#if defined(WIN32)
			Sleep(5000);
#else
			sleep(5);
#endif
		}

		downloadInProgress = false;
		gAlwaysMsg( fMessageLogSettings, "ThreadMain" )
			<< "Download completed, eCM version = " 
			<< fMajorVersion << "." << fMinorVersion << "." << fSubMinorVersion << endl;

	} while (0);

#if defined(USE_PRIVATE_NETWORK)
	// Set temporary route such that we can reach ip stack 2 on the eCM
	BcmDeleteRoute(0xC0A86401, EstbPrimaryIpStackName);
#endif
}

// Print() - Print detailed formatted object contents 
//      to specified ostream.
//
// Parameters:
//      ostream& ostrm - reference to the destination output stream.
//
// Returns:
//      reference to the destination output stream.
//
ostream& BcmDsgDownloadCtlThread::Print( ostream& ostrm ) const
{
	return ostrm;
}

// Returns the pointer to the singleton program store device.  Most objects
// in the system will use this method rather than being passed a pointer to
// the object.  The singleton pointer will be set up in the base-class
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
//      A pointer to the program store device that should be used by the
//      system.
//
BcmDsgDownloadCtlThread *BcmDsgDownloadCtlThread::GetSingletonInstance(void)
{
    if (pfSingletonInstance == NULL)
    {
        gLogMessageRaw
            << "BcmDsgDownloadCtlThread::GetSingletonInstance:  WARNING - the singleton instance is NULL, and someone is accessing it!" << endl;
    }
    return pfSingletonInstance;
}

void BcmDsgDownloadCtlThread::CdlNotificationCallback(CommonDownloadExtNotificationMsg extNotification)
{
	if( extNotification.eventCode == kEcmImageDownloadSuccessful )
	{
        gLogMessageRaw
            << "BcmDsgDownloadCtlThread is resetting eCM after successful sw upgrade." << endl;

		BcmEstbCannedSnmp *pEstbCannedSnmp = BcmEstbCannedSnmp::GetSingletonInstance();
		if(pEstbCannedSnmp)
		{
			pEstbCannedSnmp->Set_docsDevResetNow(1);
		}
	}
}

