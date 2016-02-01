//****************************************************************************
//
//  Copyright (c) 2006  Broadcom Corporation
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
//  $Id$
//
//  Filename:       EstbOsMonitorThread.h
//  Author:         Maurice Turcotte
//  Creation Date:  30 NOV 06
//
//****************************************************************************
//  Description:
//      This thread monitors some things out in OS-land periodocally and 
//      notifies interested parties.
//
//****************************************************************************

#ifndef ESTBOSMONITORTHREAD_H
#define ESTBOSMONITORTHREAD_H

//********************** Include Files ***************************************

#include "Thread.h"
#include "MessageQueue.h"
#include "IpAddress.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class EstbSnmpAgent;

//********************** Class Declaration ***********************************


class BcmEstbOsMonitorThread : public BcmThread
{
public:

    // Default Constructor.  Starts the thread and waits for it to initialze.
    // When the constructor returns, you can successfully manipulate the thread.
    // Note that it is the most-derived class who actually spawns the thread and
    // waits for it.
    //
    // For now, the thread name is hard-wired to be "OS Monitor", and
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
    BcmEstbOsMonitorThread(EstbSnmpAgent *pAgent, const char *pName, BcmOperatingSystem::ThreadPriority initialPriority = BcmOperatingSystem::kLowNormalPriority);

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
    virtual ~BcmEstbOsMonitorThread();
    
    // public API's:
    void StartMonitoring();
    bool StopThread ();

    bool ShouldAbortOsMonitoring () const;
    
    
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
    
    // Message codes
    enum
    {
      kStartThread = 0,
      kStopThread,
      kStartMonitoring
    };
    
    // Tells the thread when to exit (set in the destructor).
    bool fExitThread;
    bool fAbortOsMonitoring;
    BcmMessageQueue *pfEstbOsMonitorQueue;

    // Stuff to monitor
    BcmIpAddress fLastIpAddr;
    
    
    // This is used by StopThread() to determine when the thread has responded to
    // the request to stop.
    BcmCountingSemaphore *pfThreadStoppedSemaphore;
    
    EstbSnmpAgent *fpAgent;
    

private:
    

private:

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmEstbOsMonitorThread(const BcmEstbOsMonitorThread &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmEstbOsMonitorThread & operator = (const BcmEstbOsMonitorThread &otherInstance);

};


//********************** Inline Method Implementations ***********************

#endif


