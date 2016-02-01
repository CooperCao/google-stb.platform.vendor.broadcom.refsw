//****************************************************************************
//
//  Copyright (c) 2004  Broadcom Corporation
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
//  Filename:       EstbRemoteIfHelperThread.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  Nov 23, 2004
//
//****************************************************************************
//  Description:

//
//****************************************************************************

#ifndef ESTBREMOTEIFDATAHANDLERTHREAD_H
#define ESTBREMOTEIFDATAHANDLERTHREAD_H

//********************** Include Files ***************************************

#include "Thread.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************
class BcmDsgClientCtlThread;

//********************** Class Declaration ***********************************


class BcmEstbRemoteIfDataHandlerThread : public BcmThread
{
public:

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
	BcmEstbRemoteIfDataHandlerThread( BcmOperatingSystem::ThreadPriority initialPriority, BcmDsgClientCtlThread* pDsgClientCtlThread);

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
    virtual ~BcmEstbRemoteIfDataHandlerThread();

	//void SetEstbIpAddress(BcmIpAddress &eStbIpAddress);

    static BcmEstbRemoteIfDataHandlerThread *GetSingletonInstance(void);

protected:

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
    virtual bool Initialize(void);

    // This is the main body of the thread's code.  This is the only method
    // that absolutely must be provided in the derived class (since there is no
    // reasonable default behavior).
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    virtual void ThreadMain(void);

    // Thread destructor - this is the last method called when the thread is
    // exiting, and is where the thread should delete all of the OS objects that
    // it created.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    virtual void Deinitialize(void);

protected:
    // Tells the thread when to exit (set in the destructor).
    bool fExitThread;

    BcmDsgClientCtlThread *pfDsgClientCtlThread;

private:

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmEstbRemoteIfDataHandlerThread(const BcmEstbRemoteIfDataHandlerThread &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmEstbRemoteIfDataHandlerThread & operator = (const BcmEstbRemoteIfDataHandlerThread &otherInstance);

};


//********************** Inline Method Implementations ***********************


#endif



