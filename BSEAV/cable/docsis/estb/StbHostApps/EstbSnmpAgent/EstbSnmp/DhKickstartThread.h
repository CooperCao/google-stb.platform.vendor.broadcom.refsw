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
//  $Id$
//
//  Filename:       DhKickstartThread.h
//  Author:         Kevin O'Neal
//  Creation Date:  January 19, 2001
//
//****************************************************************************
//  Description:
//      Thread to allow DH kickstart calculations to proceed asynchronously.
//      These calculations can take some time to complete, so we can't allow
//      them to run synchronously in the DOCSIS ctl thread lest we stop
//      processing ranging, etc.
//
//****************************************************************************

#ifndef DHKICKSTARTTHREAD_H
#define DHKICKSTARTTHREAD_H

//********************** Include Files ***************************************

#include "Thread.h"
#include "MessageQueue.h"
#include "IpAddress.h"
#include "snmpDH.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmSnmpV3Agent;

//********************** Class Declaration ***********************************


class BcmDhKickstartThread : public BcmThread
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
    BcmDhKickstartThread(BcmSnmpV3Agent *pAgent, const char *pName, BcmOperatingSystem::ThreadPriority initialPriority = BcmOperatingSystem::kLowNormalPriority);

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
    virtual ~BcmDhKickstartThread();
    
    // public API's:
    void KickstartFinish ();
    bool Stop ();
    
    bool ShouldAbortCalculations () const;
    
    DhUserContainer *GetDhUserContainer () const;
    
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
      kKickstartFinish
    };
    
    // Tells the thread when to exit (set in the destructor).
    bool fExitThread;
    bool fAbortCalculations;
    BcmMessageQueue *pfDhKickstartQueue;
    
    // This is used by Stop() to determine when the thread has responded to
    // the request to stop.
    BcmCountingSemaphore *pfThreadStoppedSemaphore;
    
    BcmSnmpV3Agent *fpAgent;
    
    DhUserContainer *fpDhUserContainer;

private:
    

private:

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmDhKickstartThread(const BcmDhKickstartThread &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmDhKickstartThread & operator = (const BcmDhKickstartThread &otherInstance);

};


//********************** Inline Method Implementations ***********************

#endif


