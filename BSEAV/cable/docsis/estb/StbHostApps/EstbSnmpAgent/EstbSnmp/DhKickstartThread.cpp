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
//  Filename:       DhKickstartThread.cpp
//  Author:         Kevin O'Neal
//  Creation Date:  Jan 19, 2001
//
//****************************************************************************
//  Description:
//      Thread to allow event logging to take place outside the context of
//      the caller's thread.  Necessary because logging an event can involve
//      a certain amount of latency WRT sending traps & syslog messages, plus
//      conceivably you could have thread-safety issues otherwise.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "DhKickstartThread.h"

// Other objects of interest.
#include "Event.h"
#include "Timer.h"
#include "CountingSemaphore.h"
#include "OperatingSystem.h"
#include "OperatingSystemFactory.h"
#include "SnmpApi.h"
#include "SnmpLog.h"
#include "SnmpV3Agent.h"


//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


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
BcmDhKickstartThread::BcmDhKickstartThread(BcmSnmpV3Agent *pAgent, const char *pName, BcmOperatingSystem::ThreadPriority initialPriority) :
    BcmThread(pName, false, initialPriority)
{
    fMessageLogSettings.SetModuleName("BcmDhKickstartThread");

    fExitThread = false;
    fAbortCalculations = false;
    
    pfThreadStoppedSemaphore = NULL;
    
    fpAgent = pAgent;
    
    if (pfOperatingSystem->BeginThread(this, 16384) == false)
    {
        gFatalErrorMsg( fMessageLogSettings, "BcmDhKickstartThread" ) 
            << "Failed to spawn my thread!" << endl;
        assert(0);
    }
    
    //create a container for our Diffie-Hellman objects
    fpDhUserContainer = new DhUserContainer; 
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
BcmDhKickstartThread::~BcmDhKickstartThread()
{
  fExitThread = true;
  
  // Send a message to the thread to wake it up.
  if (pfDhKickstartQueue == NULL)
  {
    SnmpLogRaw << "ERROR - BcmDhKickstartThread::~BcmDhKickstartThread - pfDhKickstartQueue is NULL" << endl;
    return;
  }
  
  pfDhKickstartQueue->Send(kStopThread);
  
  // Wait for the thread to exit
  WaitForThread();
}


void BcmDhKickstartThread::KickstartFinish ()
{
  fAbortCalculations = false;

  // Send ourselves an event to kick off the DH calculations.
  if (pfDhKickstartQueue)
    pfDhKickstartQueue->Send(kKickstartFinish);
}


bool BcmDhKickstartThread::Stop ()
{
  SnmpLogStart << "Stopping DH kickstart thread..." << endl;

  fAbortCalculations = true;
  
  // Wait for the thread to reset.  This means that this call will block
  // until the thread has finished.
  pfThreadStoppedSemaphore->Get();
  pfThreadStoppedSemaphore->Release();
  
  SnmpLogStart << "DH kickstart thread stopped OK." << endl;
  
  return true;
}

bool BcmDhKickstartThread::ShouldAbortCalculations () const
{
  return fAbortCalculations;
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
bool BcmDhKickstartThread::Initialize(void)
{
    gFuncEntryExitMsg(fMessageLogSettings, "Initialize") << "Entering..." << endl;
    
    pfDhKickstartQueue = pfOperatingSystem->NewMessageQueue ("BcmDhKickstartThread message queue");
    
    if (pfDhKickstartQueue == NULL)
    {
      SnmpLogRaw << "ERROR - BcmDhKickstartThread failed to create its pfDhKickstartQueue!" << endl;
      return false;
    }
    
    pfThreadStoppedSemaphore = BcmOperatingSystemFactory::NewCountingSemaphore(true, 1, 1, "Thread Stopped Semaphore");

    if (pfThreadStoppedSemaphore == NULL)
    {
        gErrorMsg(fMessageLogSettings, "Initialize") << "Failed to create my semaphore!" << endl;

        return false;
    }
    
    return true;
}


// Thread destructor - this is the last method called when the thread is
// exiting, and is where the thread should delete all of the OS objects that
// it created.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmDhKickstartThread::Deinitialize(void)
{
    gFuncEntryExitMsg(fMessageLogSettings, "Deinitialize") << "Entering..." << endl;

    // Get rid of the OS objects that I created.
    if (pfDhKickstartQueue)
      delete pfDhKickstartQueue;
      
    pfDhKickstartQueue = NULL;
    
    if (pfThreadStoppedSemaphore)
      delete pfThreadStoppedSemaphore;
      
    pfThreadStoppedSemaphore = NULL;
}


// This is the main body of the thread's code.  This is the only method
// that absolutely must be provided in the derived class (since there is no
// reasonable default behavior).
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmDhKickstartThread::ThreadMain(void)
{
  unsigned int MsgCode;
  void *pMsgBuf = NULL;


    gFuncEntryExitMsg(fMessageLogSettings, "ThreadMain") << "Entering..." << endl;
    
    if (pfDhKickstartQueue == NULL)
    {
      SnmpLogRaw << "ERROR - BcmDhKickstartThread::ThreadMain - pfDhKickstartQueue is NULL" << endl;
      return;
    }

    while (1)
    {
      gInfoMsg(fMessageLogSettings, "ThreadMain") << "Waiting for someone to post my event." << endl;

      // Wait for a message to arrive on the queue
      if (pfDhKickstartQueue->Receive (MsgCode, pMsgBuf) == false)
      {
        gWarningMsg(fMessageLogSettings, "ThreadMain") << "pfDhKickstartQueue->Receive returned false." << endl;
        continue;
      }

      // If I'm being told to exit the thread, then do so.
      if (fExitThread)
        break;

      // Process my events here
      switch (MsgCode)
      {
        case kKickstartFinish:
          // Lock the semaphore so that anyone calling Stop() will be blocked
          // until we return from the following functions.
          pfThreadStoppedSemaphore->Get(BcmCountingSemaphore::kTimeout, 0);
          
          // A return value of other than NO_ERROR here means our kickstart
          // calculations were aborted, so don't turn on RF and don't send
          // the warm start trap.
          if (AgentKickstartFinish(fpAgent->GetIfNum()) == SNMP_NO_ERROR)
          {
              fpAgent->EnableUser(BcmString("dhKickstart"));
              SnmpLogRaw<<"Finished DH calculations -dhKickstart user enabled."<<endl;
              
              fpAgent->EnableManagement(true);
          }
          // Now unlock the semaphore so that the caller knows that we have stopped.
          pfThreadStoppedSemaphore->Release();
          break;
        
        default:
          break;
      }

      // If I'm being told to exit the thread, then do so.
      if (fExitThread)
        break;
    }

    gFuncEntryExitMsg(fMessageLogSettings, "ThreadMain") << "Exiting..." << endl;
}


DhUserContainer* BcmDhKickstartThread::GetDhUserContainer () const
{
  return fpDhUserContainer;
}

