//****************************************************************************
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
//  Filename:       DsgDownloadCtlThread.h
//  Author:         Jean Cyr, Pinar Taskiran
//  Creation Date:  November 22, 2004
//
//****************************************************************************

#ifndef DSGDOWNLOADCTLTHREAD_H
#define DSGDOWNLOADCTLTHREAD_H

//********************** Include Files ***************************************

#include "Thread.h"

#include "IpAddress.h"

#include <dsg_api.h>

#include "EnetPkt.h"
#include "zone.h"

#include "DcdMsg.h"
#include "DsgConfigSettings.h"
#include "EstbCommonDownloadInterface.h"
//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmEvent;
class BcmCountingSemaphore;
class BcmSocket;
class BcmMacAddress;
class BcmEstbRemoteIfHelperThread;
class BcmEstbCannedSnmp;
//********************** Class Declaration ***********************************

class BcmDsgDownloadCtlThread : public BcmThread
{
public:

	// BcmDsgDownloadCtlThread() - Object constructor
	//		Initiates thread and initializes singleton.
	//
	// Parameters:
	//      initialPriority - Initial thread priority.
	//
	// Returns:
	//      None.
	//
	BcmDsgDownloadCtlThread(BcmOperatingSystem::ThreadPriority initialPriority);

	// ~BcmDsgDownloadCtlThread() - Object constructor
	//		Signals thread and waits for termination.
	//
	// Parameters:
	//      None.
	//
	// Returns:
	//      None.
	//
    virtual ~BcmDsgDownloadCtlThread();

	// Print() - Print detailed formatted object contents 
	//      to specified ostream.
	//
	// Parameters:
	//      ostream& ostrm - reference to the destination output stream.
	//
	// Returns:
	//      reference to the destination output stream.
	//
	ostream& Print( ostream& ostrm ) const;

	static BcmDsgDownloadCtlThread *GetSingletonInstance(void);

protected:

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
	bool Initialize();

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
	void ThreadMain();

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
	void CheckForEcmUpgrade(void);

public:

	//********************** Private Types ****************************************

	//********************** Private Constants ************************************

	//********************** Private Variables ************************************

	// This is the singleton instance of this class.
	static BcmDsgDownloadCtlThread *pfSingletonInstance;

	// Program version of eCM image
	uint16 fMajorVersion;
	uint16 fMinorVersion;
	uint16 fSubMinorVersion;

	// Request id for SNMP GET/SET requests
	uint16 fRequestId;

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
	void GetLocalFileVersion(const char *fileName, uint16 &major, uint16 &minor, uint16 &subMinor);

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
	bool GetECMVersion(uint16 &major, uint16 &minor, uint16 &subMinor);

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
	bool SetImageFileName(const char *fileName);

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
	bool SetServerIpAddress();

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
	bool InitiateSecureDownload();

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
	bool GetOperStatus(uint16 &status);

	static void CdlNotificationCallback(CommonDownloadExtNotificationMsg extNotification);

private:

	BcmEstbCannedSnmp *pfEstbCannedSnmp;

};

//********************** Inline Method Implementations ***********************
inline ostream& operator<<( ostream& ostrm, const BcmDsgDownloadCtlThread& rhs )
{
    return rhs.Print( ostrm );
}

#endif



