//****************************************************************************
//
// Copyright (c) 2009 Broadcom Corporation
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
//  Filename:       EstbRemoteIfHelperThread.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  Nov 19, 2004
//
//****************************************************************************
//  Description:

//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "EstbRemoteIfHelperThread.h"

#include "OperatingSystem.h"
#include "OperatingSystemFactory.h"

//#include "dsg_api.h"
#include "StubInterface.h"
      
//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************
extern "C"
{
	extern void EstbDsgRemoteInterfaceHelperThread ( void );
	extern int SetupRemoteInterface(void);
}

// Default Constructor.  Starts the thread and waits for it to initialze.
// When the constructor returns, you can successfully manipulate the thread.
// Note that it is the most-derived class who actually spawns the thread and
// waits for it.
//
// For now, the thread name is hard-wired to be "Time Of Day Thread", and
// it will not start suspended.  These may be changed if we find that it is
// desireable.
//
// You can set the thread priority as desired (defaults to normal) so that
// the thread consumes the appropriate amount of processor time.
//
// Parameters:
//      initialPriority - the priority that you want the thread to have.
//
// Returns:  N/A
//
BcmEstbRemoteIfHelperThread::BcmEstbRemoteIfHelperThread( BcmOperatingSystem::ThreadPriority initialPriority) : //, BcmDsgClientCtlThread* pDsgClientCtlThread) :
BcmThread("eSTB Remote Interface Thread", false, initialPriority)
//fEstbIpAddress(kAllZerosIp)
{
    CallTrace("BcmEstbRemoteIfHelperThread", "BcmEstbRemoteIfHelperThread");

    fMessageLogSettings.SetModuleName("BcmEstbRemoteIfHelperThread");

    // Set some clean default values.
    fExitThread = false;

	//fEcmIpAddress = eCmIpAddress;
	//fEstbIpAddress = eStbIpAddress;
 
	// Setup sockets for Broadcom Remote Interface.
	/*if(!SetupRemoteInterface())
	{
		gFatalErrorMsg(fMessageLogSettings, "BcmEstbRemoteIfHelperThread") << "Failed to initialize the socket interface!" << endl;
	}*/

    // Just kick off the thread.  PR1613 - VxWorks uses almost 4k of stack for
    // some reason; not sure why.  Give it 6k so we have lots of margin.
#if defined(TARGETOS_vxWorks) || defined(WIN32)
	if (pfOperatingSystem->BeginThread(this, 16 * 1024) == false)
#else
	if (pfOperatingSystem->BeginThread(this, 8 * 1024) == false)
#endif
    {
        gFatalErrorMsg(fMessageLogSettings, "BcmEstbRemoteIfHelperThread") << "Failed to spawn my thread!" << endl;

        assert(0);
    }
}
         

// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Note that the event that was passed to us via SetNotificationEvent() will
// not be deleted!  It is the owner's responsibility to do so!
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmEstbRemoteIfHelperThread::~BcmEstbRemoteIfHelperThread()
{
    CallTrace("BcmEstbRemoteIfHelperThread", "~BcmEstbRemoteIfHelperThread");

    fExitThread = true;

    // Make sure the thread has exited.
    WaitForThread();    
}

// Thread constructor - this is the first method called after the thread has
// been spawned, and is where the thread should create all OS objects.  This
// has to be done here, rather than in the object's constructor, because
// some OS objects must be created in the context of the thread that will
// use them.  The object's constructor is still running in the context of
// the thread that created this object.
//
// Parameters:  None.
//
// Returns:
//      true if successful and ThreadMain() should be called.
//      false if there was a problem (couldn't create an OS object, etc.)
//          and ThreadMain() should not be called.
//
bool BcmEstbRemoteIfHelperThread::Initialize(void)
{
    CallTrace("BcmEstbRemoteIfHelperThread", "Initialize");

    return true;
}


// This is the main body of the thread's code.  This is the only method
// that absolutely must be provided in the derived class (since there is no
// reasonable default behavior).
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmEstbRemoteIfHelperThread::ThreadMain(void)
{
    CallTrace("BcmEstbRemoteIfHelperThread", "ThreadMain");



	// Broadcom Remote Interface.
	EstbDsgRemoteInterfaceHelperThread();
}


// Thread destructor - this is the last method called when the thread is
// exiting, and is where the thread should delete all of the OS objects that
// it created.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmEstbRemoteIfHelperThread::Deinitialize(void)
{
    CallTrace("BcmEstbRemoteIfHelperThread", "Deinitialize");

    // Get rid of the OS objects that I created.
}

/*void BcmEstbRemoteIfHelperThread::SetEstbIpAddress(BcmIpAddress &eStbIpAddress)
{
	fEstbIpAddress = eStbIpAddress;
}*/





