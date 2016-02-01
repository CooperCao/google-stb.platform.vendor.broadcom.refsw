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
//  Filename:       CommonDownloadHelperThread.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  Sep 22, 2008
//
//****************************************************************************
//  Description:  This thread initializes the Common Download Library (CDL) 
//				  that provides methods for storing image in the proper flash 
//				  partition, image expansion, etc.
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "CommonDownloadHelperThread.h"
#include "cdl_common.h"
#include "cdl_service.h"

#include "OperatingSystem.h"
#include "OperatingSystemFactory.h"

#include "StubInterface.h"
      
//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************
extern "C"
{
	extern void cdl_service_init(); 
	extern int cdl_service_start(void);
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
BcmCommonDownloadHelperThread::BcmCommonDownloadHelperThread( BcmOperatingSystem::ThreadPriority initialPriority) :
BcmThread("eSTB Remote Interface Thread", false, initialPriority)
{
    CallTrace("BcmCommonDownloadHelperThread", "BcmCommonDownloadHelperThread");

    fMessageLogSettings.SetModuleName("BcmCommonDownloadHelperThread");

    // Set some clean default values.
    fExitThread = false;

	if (pfOperatingSystem->BeginThread(this, 8 * 1024) == false)
    {
        gFatalErrorMsg(fMessageLogSettings, "BcmCommonDownloadHelperThread") << "Failed to spawn my thread!" << endl;

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
BcmCommonDownloadHelperThread::~BcmCommonDownloadHelperThread()
{
    CallTrace("BcmCommonDownloadHelperThread", "~BcmCommonDownloadHelperThread");

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
bool BcmCommonDownloadHelperThread::Initialize(void)
{
    CallTrace("BcmCommonDownloadHelperThread", "Initialize");

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
void BcmCommonDownloadHelperThread::ThreadMain(void)
{
    CallTrace("BcmCommonDownloadHelperThread", "ThreadMain");

	// start CDL service
	cdl_service_init();
	cdl_service_start();
}


// Thread destructor - this is the last method called when the thread is
// exiting, and is where the thread should delete all of the OS objects that
// it created.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmCommonDownloadHelperThread::Deinitialize(void)
{
    CallTrace("BcmCommonDownloadHelperThread", "Deinitialize");

    // Get rid of the OS objects that I created.
}






