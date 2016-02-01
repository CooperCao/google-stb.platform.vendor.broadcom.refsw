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
//  Filename:       EstbRemoteIfDataHandlerThread.cpp
//  Author:         Karthik Balasubramanian
//  Creation Date:  Sep 12, 2005
//
//****************************************************************************
//  Description:

//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "EstbRemoteIfDataHandlerThread.h"
#include "OperatingSystem.h"
#include "OperatingSystemFactory.h"
#include "StubInterface.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************
extern "C"
{
	extern void EstbDsgRemoteInterfaceDataThread ( void );
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
BcmEstbRemoteIfDataHandlerThread::BcmEstbRemoteIfDataHandlerThread( BcmOperatingSystem::ThreadPriority initialPriority, BcmDsgClientCtlThread* pDsgClientCtlThread) :
BcmThread("eSTB Remote Interface Data Handler Thread", false, initialPriority)
//fEstbIpAddress(kAllZerosIp)
{
    CallTrace("BcmEstbRemoteIfDataHandlerThread", "BcmEstbRemoteIfDataHandlerThread");

    fMessageLogSettings.SetModuleName("BcmEstbRemoteIfDataHandlerThread");

    // Set some clean default values.
    fExitThread = false;

    if( pDsgClientCtlThread == NULL )
    {
	gFatalErrorMsg(fMessageLogSettings, "BcmEstbRemoteIfDataHandlerThread") << "pDsgClientCtlThread is NULL!" << endl;
    }
    else
    {
  	pfDsgClientCtlThread = pDsgClientCtlThread;
    }


    // Just kick off the thread.  PR1613 - VxWorks uses almost 4k of stack for
    // some reason; not sure why.  Give it 6k so we have lots of margin.
    if (pfOperatingSystem->BeginThread(this, 8 * 1024) == false)
    {
        gFatalErrorMsg(fMessageLogSettings, "BcmEstbRemoteIfDataHandlerThread") << "Failed to spawn my thread!" << endl;
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
BcmEstbRemoteIfDataHandlerThread::~BcmEstbRemoteIfDataHandlerThread()
{
    CallTrace("BcmEstbRemoteIfDataHandlerThread", "~BcmEstbRemoteIfDataHandlerThread");
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
bool BcmEstbRemoteIfDataHandlerThread::Initialize(void)
{
    CallTrace("BcmEstbRemoteIfDataHandlerThread", "Initialize");

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
void BcmEstbRemoteIfDataHandlerThread::ThreadMain(void)
{
    CallTrace("BcmEstbRemoteIfDataHandlerThread", "ThreadMain");
    EstbDsgRemoteInterfaceDataThread();
}


// Thread destructor - this is the last method called when the thread is
// exiting, and is where the thread should delete all of the OS objects that
// it created.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmEstbRemoteIfDataHandlerThread::Deinitialize(void)
{
    CallTrace("BcmEstbRemoteIfDataHandlerThread", "Deinitialize");

    // Get rid of the OS objects that I created.
}




