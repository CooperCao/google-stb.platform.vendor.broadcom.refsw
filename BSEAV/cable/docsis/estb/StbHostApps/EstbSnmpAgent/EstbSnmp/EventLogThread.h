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
//  Filename:       EventLogThread.h
//  Author:         Kevin O'Neal
//  Creation Date:  June 20, 2000
//
//****************************************************************************
//  Description:
//      Thread to allow event logging to take place outside the context of
//      the caller's thread.  Necessary because logging an event can involve
//      a certain amount of latency WRT sending traps & syslog messages, plus
//      conceivably you could have thread-safety issues otherwise.
//
//****************************************************************************

#ifndef EVENTLOGTHREAD_H
#define EVENTLOGTHREAD_H

//********************** Include Files ***************************************

#include "Thread.h"
#include "MessageQueue.h"
#include "IpAddress.h"
#include "EventLog.h"

#define NUM_STRESSORS 8

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************
class EventLogEntry;

//********************** Class Declaration ***********************************


class BcmEventLogThread : public BcmThread
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
    BcmEventLogThread(BcmOperatingSystem::ThreadPriority initialPriority = BcmOperatingSystem::kLowNormalPriority);

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
    virtual ~BcmEventLogThread();
    
    // public API's to log an event.
    SNMP_STATUS LogEvent (EventLogIdCode Id, EventLogSeverityLevel Level, const char *pText, EventLogPolicy Policy, EventLog *pLog);
    SNMP_STATUS LogEvent (EventLogIdCode Id, const char *pParam1, const char *pParam2, const char *pParam3, const char *pParam4, const char *pParam5, EventLog *pLog);
    
    SNMP_STATUS SendTrap (int TrapType, int IfIndex, BcmSnmpAgent *pAgent);
    
    SNMP_STATUS WriteNonVolSettings (EventLog *pLog);
    
    SNMP_STATUS ResetLog (EventLog *pLog);
    
    SNMP_STATUS InitFromNonVol (EventLog *pLog);
    
    SNMP_STATUS SendDeferredAsyncs (EventLog *pLog);
    
    SNMP_STATUS StartStressTest (EventLog *pLog);
    SNMP_STATUS StopStressTest (EventLog *pLog);
    
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
      kLogEvent,
      kLogEventWithParams,
      kStdTrap,
      kWriteNonVol,
      kResetLog,
      kInitFromNonVol,
      kSendDeferredAsyncs
    };
    
    struct EventLogMsgPayload
    {
      EventLog *pLog;
      EventLogIdCode Id;
      EventLogSeverityLevel Level;
      char pText[256];
      EventLogPolicy Policy;
      char pParams[5][256]; // for specifying additional parameters as needed
    };
    
    struct TrapMsgPayload
    {
      BcmSnmpAgent *pAgent;
      int TrapType;
      int IfIndex;
    };

    // Tells the thread when to exit (set in the destructor).
    bool fExitThread;
    BcmMessageQueue *pfEventLogQueue;
    
    // An array of event logs which we may be stressing.
    EventLog *fpStressors[NUM_STRESSORS];

private:
    

private:

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmEventLogThread(const BcmEventLogThread &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmEventLogThread & operator = (const BcmEventLogThread &otherInstance);

friend class EventLog;
};


//********************** Inline Method Implementations ***********************

#endif


