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
//  Filename:       EventLogThread.cpp
//  Author:         David Pullen
//  Creation Date:  Jan 17, 2000
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
#include "EventLogThread.h"

// Other objects of interest.
#include "Event.h"
#include "Timer.h"
#include "CountingSemaphore.h"
#include "OperatingSystem.h"
#include "OperatingSystemFactory.h"

// Event log related headers
#include "MibObjs.h"
#include "EventLog.h"
#include "BcmSnmpAgent.h"
#include "SnmpLog.h"


#define MAX_QUEUED_MSGS 64

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
BcmEventLogThread::BcmEventLogThread(BcmOperatingSystem::ThreadPriority initialPriority) :
    BcmThread("Event Log Thread", false, initialPriority)
{
    fMessageLogSettings.SetModuleName("BcmEventLogThread");

    fExitThread = false;
    
    for (int Loop = 0; Loop < NUM_STRESSORS; Loop++)
      fpStressors[Loop] = NULL;
    
    // 4k was NOT sufficient, can crash us.  Back to 8k.
    if (pfOperatingSystem->BeginThread(this, 8192) == false)
    {
        gFatalErrorMsg( fMessageLogSettings, "BcmEventLogThread" ) 
            << "Failed to spawn my thread!" << endl;
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
BcmEventLogThread::~BcmEventLogThread()
{
  fExitThread = true;
  
  // Send a message to the thread to wake it up.
  if (pfEventLogQueue == NULL)
  {
    SnmpLogRaw << "ERROR - BcmEventLogThread::~BcmEventLogThread - pfEventLogQueue is NULL" << endl;
    return;
  }
  
  pfEventLogQueue->Send(kStopThread);
  
  // Wait for the thread to exit
  WaitForThread();
}


// Public API to log an event
SNMP_STATUS BcmEventLogThread::LogEvent (EventLogIdCode Id, EventLogSeverityLevel Level, const char *pText, EventLogPolicy Policy, EventLog *pLog)
{
  bool Sent;
  SNMP_STATUS Status = SNMP_NO_ERROR;
  EventLogMsgPayload *pPayload;


  if (pfEventLogQueue == NULL)
  {
    SnmpLogRaw << "ERROR - BcmEventLogThread::LogEvent - pfEventLogQueue is NULL" << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  } 
  
  if (pfEventLogQueue->NumberOfMessages() >= MAX_QUEUED_MSGS)
  {
    SnmpLogEv << "WARNING: event log msg queue is dangerously full, discarding event." << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }
  
  pPayload = new EventLogMsgPayload;
  memset (pPayload, 0, sizeof (EventLogMsgPayload));
  
  pPayload->pLog = pLog;
  pPayload->Id = Id;
  pPayload->Level = Level;
  
  if (pText)
    strncpy (pPayload->pText, pText, 255);

  pPayload->Policy = Policy;
  
  Sent = pfEventLogQueue->Send(kLogEvent, pPayload);
  
  if (Sent == false)
  {
    SnmpLogEv << "  pfEventLogQueue->Send failed." << endl;
    
    delete pPayload;
    Status = SNMP_GEN_ERR;
  }
  
  return Status;
}

SNMP_STATUS BcmEventLogThread::LogEvent (EventLogIdCode Id, const char *pParam1, const char *pParam2, const char *pParam3, const char *pParam4, const char *pParam5, EventLog *pLog)
{
  bool Sent;
  SNMP_STATUS Status = SNMP_NO_ERROR;
  EventLogMsgPayload *pPayload;


  if (pfEventLogQueue == NULL)
  {
    SnmpLogRaw << "ERROR - BcmEventLogThread::LogEvent - pfEventLogQueue is NULL" << endl;
    return false;
  }
  
  if (pfEventLogQueue->NumberOfMessages() >= MAX_QUEUED_MSGS)
  {
    SnmpLogEv << "WARNING: event log msg queue is dangerously full, discarding event." << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }  
  
  pPayload = new EventLogMsgPayload;
  memset (pPayload, 0, sizeof (EventLogMsgPayload));
  
  pPayload->pLog = pLog;
  pPayload->Id = Id;
  pPayload->Level = kUseDefaultLevel;
  
  pPayload->Policy = kUseDefaultPolicy;

  if (pParam1)
    strncpy (pPayload->pParams[0], pParam1, 255);

  if (pParam2)
    strncpy (pPayload->pParams[1], pParam2, 255);

  if (pParam3)
    strncpy (pPayload->pParams[2], pParam3, 255);

  if (pParam4)
    strncpy (pPayload->pParams[3], pParam4, 255);
  
  if (pParam5)
    strncpy (pPayload->pParams[4], pParam5, 255);

  Sent = pfEventLogQueue->Send(kLogEventWithParams, pPayload);
  
  if (Sent == false)
  {
    SnmpLogEv << "  pfEventLogQueue->Send failed." << endl;
    
    delete pPayload;
    Status = SNMP_GEN_ERR;
  }
  
  return Status;
}

// Public API to send a trap
SNMP_STATUS BcmEventLogThread::SendTrap (int TrapType, int IfIndex, BcmSnmpAgent *pAgent)
{
  bool Sent;
  SNMP_STATUS Status = SNMP_NO_ERROR;
  TrapMsgPayload *pPayload;
  
  
  SnmpLogEv << "BcmEventLogThread::SendTrap for type " << TrapType << endl;
  
  if (pfEventLogQueue == NULL)
  {
    SnmpLogRaw << "ERROR - BcmEventLogThread::SendTrap - pfEventLogQueue is NULL" << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }
  
  if (pfEventLogQueue->NumberOfMessages() >= MAX_QUEUED_MSGS)
  {
    SnmpLogEv << "WARNING: event log msg queue is dangerously full, discarding event." << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }  
  
  pPayload = new TrapMsgPayload;
  memset (pPayload, 0, sizeof (TrapMsgPayload));

  pPayload->pAgent = pAgent;  
  pPayload->TrapType = TrapType;
  pPayload->IfIndex = IfIndex;
  
  Sent = pfEventLogQueue->Send(kStdTrap, (void *)pPayload);
  
  if (Sent != true)
  {
    SnmpLogEv << "  pfEventLogQueue->Send failed." << endl;
    Status = SNMP_GEN_ERR;
    
    delete pPayload;
  }
  else
    SnmpLogEv << "  pfEventLogQueue->Send OK." << endl;
  
  return Status;
}

SNMP_STATUS BcmEventLogThread::WriteNonVolSettings (EventLog *pLog)
{
  SnmpLogEv << "BcmEventLogThread::WriteNonVolSettings" << endl;
  
  if (pfEventLogQueue == NULL)
  {
    SnmpLogRaw << "ERROR - BcmEventLogThread::WriteNonVolSettings - pfEventLogQueue is NULL" << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }
  
  if (pfEventLogQueue->NumberOfMessages() >= MAX_QUEUED_MSGS)
  {
    SnmpLogEv << "WARNING: event log msg queue is dangerously full, discarding event." << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }  
  
  if (pfEventLogQueue->Send(kWriteNonVol, (void *)pLog) != true)
  {
    SnmpLogEv << "  pfEventLogQueue->Send failed." << endl;
    return SNMP_GEN_ERR;
  }
  else
    SnmpLogEv << "  pfEventLogQueue->Send OK." << endl;
  
  return SNMP_NO_ERROR;
}


SNMP_STATUS BcmEventLogThread::ResetLog (EventLog *pLog)
{
  SnmpLogEv << "BcmEventLogThread::ResetLog" << endl;
  
  if (pfEventLogQueue == NULL)
  {
    SnmpLogRaw << "ERROR - BcmEventLogThread::ResetLog - pfEventLogQueue is NULL" << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }
  
  if (pfEventLogQueue->NumberOfMessages() >= MAX_QUEUED_MSGS)
  {
    SnmpLogEv << "WARNING: event log msg queue is dangerously full, discarding event." << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }  
  
  if (pfEventLogQueue->Send(kResetLog, (void *)pLog) != true)
  {
    SnmpLogEv << "  pfEventLogQueue->Send failed." << endl;
    return SNMP_GEN_ERR;
  }
  else
    SnmpLogEv << "  pfEventLogQueue->Send OK." << endl;
  
  return SNMP_NO_ERROR;
}


SNMP_STATUS BcmEventLogThread::InitFromNonVol (EventLog *pLog)
{
  SnmpLogEv << "BcmEventLogThread::InitFromNonVol" << endl;
  
  if (pfEventLogQueue == NULL)
  {
    SnmpLogRaw << "ERROR - BcmEventLogThread::InitFromNonVol - pfEventLogQueue is NULL" << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }
  
  if (pfEventLogQueue->NumberOfMessages() >= MAX_QUEUED_MSGS)
  {
    SnmpLogEv << "WARNING: event log msg queue is dangerously full, discarding event." << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }  
  
  if (pfEventLogQueue->Send(kInitFromNonVol, (void *)pLog) != true)
  {
    SnmpLogEv << "  pfEventLogQueue->Send failed." << endl;
    return SNMP_GEN_ERR;
  }
  else
    SnmpLogEv << "  pfEventLogQueue->Send OK." << endl;
  
  return SNMP_NO_ERROR;
}


SNMP_STATUS BcmEventLogThread::SendDeferredAsyncs (EventLog *pLog)
{
  SnmpLogEv << "BcmEventLogThread::SendDeferredAsyncs" << endl;
  
  if (pfEventLogQueue == NULL)
  {
    SnmpLogRaw << "ERROR - BcmEventLogThread::SendDeferredAsyncs - pfEventLogQueue is NULL" << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }
  
  if (pfEventLogQueue->NumberOfMessages() >= MAX_QUEUED_MSGS)
  {
    SnmpLogEv << "WARNING: event log msg queue is dangerously full, discarding event." << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }  
  
  if (pfEventLogQueue->Send(kSendDeferredAsyncs, (void *)pLog) != true)
  {
    SnmpLogEv << "  pfEventLogQueue->Send failed." << endl;
    return SNMP_GEN_ERR;
  }
  else
    SnmpLogEv << "  pfEventLogQueue->Send OK." << endl;
  
  return SNMP_NO_ERROR;
}


SNMP_STATUS BcmEventLogThread::StartStressTest (EventLog *pLog)
{
  int Loop;
  
  for (Loop = 0; Loop < NUM_STRESSORS; Loop++)
  {
    if (fpStressors[Loop] == NULL)
    {
      fpStressors[Loop] = pLog;
      SnmpLogRaw << "Starting stress test for " << pLog->fpName << endl;
      return SNMP_NO_ERROR;
    }
  }
  
  return SNMP_RESOURCE_UNAVAILABLE;
}

SNMP_STATUS BcmEventLogThread::StopStressTest (EventLog *pLog)
{
  int Loop;
  
  for (Loop = 0; Loop < NUM_STRESSORS; Loop++)
  {
    if (fpStressors[Loop] == pLog)
    {
      fpStressors[Loop] = NULL;
      SnmpLogRaw << "Stopping stress test for " << pLog->fpName << endl;
      return SNMP_NO_ERROR;
    }
  }
  
  return SNMP_RESOURCE_UNAVAILABLE;
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
bool BcmEventLogThread::Initialize(void)
{
    gFuncEntryExitMsg(fMessageLogSettings, "Initialize") << "Entering..." << endl;
    
    pfEventLogQueue = pfOperatingSystem->NewMessageQueue ("BcmEventLogThread message queue");
    
    if (pfEventLogQueue == NULL)
    {
      SnmpLogRaw << "ERROR - BcmEventLogThread failed to create its pfEventLogQueue!" << endl;
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
void BcmEventLogThread::Deinitialize(void)
{
    gFuncEntryExitMsg(fMessageLogSettings, "Deinitialize") << "Entering..." << endl;

    // Get rid of the OS objects that I created.
    if (pfEventLogQueue)
      delete pfEventLogQueue;
      
    pfEventLogQueue = NULL;
}


// This is the main body of the thread's code.  This is the only method
// that absolutely must be provided in the derived class (since there is no
// reasonable default behavior).
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmEventLogThread::ThreadMain(void)
{
  unsigned int MsgCode;
  void *pMsgBuf = NULL;
  BcmMessageQueue::WaitMode WaitMode = BcmMessageQueue::kForever;


    gFuncEntryExitMsg(fMessageLogSettings, "ThreadMain") << "Entering..." << endl;
    
    if (pfEventLogQueue == NULL)
    {
      SnmpLogRaw << "ERROR - BcmEventLogThread::ThreadMain - pfEventLogQueue is NULL" << endl;
      return;
    }

    while (1)
    {
      gInfoMsg(fMessageLogSettings, "ThreadMain") << "Waiting for someone to post my event." << endl;
      
      // Initialize wait mode to 'forever'...
      WaitMode = BcmMessageQueue::kForever;

      // If we are stressing any logs, call his stress method and then put our
      // receive call below on a timeout so we come right back up here again if
      // we have no other events to process.      
      for (int Loop = 0; Loop < NUM_STRESSORS; Loop++)
      {
        if (fpStressors[Loop])
        {
          fpStressors[Loop]->StressTestImpl();
          WaitMode = BcmMessageQueue::kTimeout;
        }
      }

      // Wait for a message to arrive on the queue
      if (pfEventLogQueue->Receive (MsgCode, pMsgBuf, WaitMode, 50) == false)
        continue;

      // If I'm being told to exit the thread, then do so.
      if (fExitThread)
        break;
        
      switch (MsgCode)
      {
        case kLogEvent:
        {
          EventLogMsgPayload *pPayload = (EventLogMsgPayload *) pMsgBuf;
          EventLogEntry *pEvent;
          
          pEvent = pPayload->pLog->LogEventImpl (pPayload->Id, pPayload->Level, pPayload->pText, pPayload->Policy);
          if (pEvent)
            pPayload->pLog->StoreEventToNonVol(pEvent);
            
          delete pPayload;
          break;
        }

        case kLogEventWithParams:
        {
          EventLogMsgPayload *pPayload = (EventLogMsgPayload *) pMsgBuf;
          EventLogEntry *pEvent;
          char *pParams[4];

          if (pPayload->pParams[0][0] == 0)
            pParams[0] = NULL;
          else
            pParams[0] = pPayload->pParams[0];

          if (pPayload->pParams[1][0] == 0)
            pParams[1] = NULL;
          else
            pParams[1] = pPayload->pParams[1];

          if (pPayload->pParams[2][0] == 0)
            pParams[2] = NULL;
          else
            pParams[2] = pPayload->pParams[2];

          if (pPayload->pParams[3][0] == 0)
            pParams[3] = NULL;
          else
            pParams[3] = pPayload->pParams[3];
          
          if (pPayload->pParams[4][0] == 0)
            pParams[4] = NULL;
          else
            pParams[4] = pPayload->pParams[4];

          pEvent = pPayload->pLog->LogEventImpl (pPayload->Id, pParams[0], pParams[1], pParams[2], pParams[3], pParams[4]);
          if (pEvent)
            pPayload->pLog->StoreEventToNonVol(pEvent);
            
          delete pPayload;
          break;
        }
        
        case kStdTrap:
        {
          TrapMsgPayload *pTrap = (TrapMsgPayload *) pMsgBuf;
          
          SnmpLogEv << "  BcmEventLogThread Received a trap msg type " << pTrap->TrapType << endl;
          pTrap->pAgent->SendStandardTrap(pTrap->TrapType, pTrap->IfIndex);
            
          delete pTrap;
          break;
        }
        
        case kWriteNonVol:
        {
          EventLog *pLog = (EventLog *) pMsgBuf;
          
          pLog->WriteNonVolSettingsImpl();
          break;
        }
        
        case kResetLog:
        {
          EventLog *pLog = (EventLog *) pMsgBuf;
          
          pLog->ResetLogImpl();
          break;
        }
        
        case kSendDeferredAsyncs:
        {
          EventLog *pLog = (EventLog *) pMsgBuf;
          
          pLog->SendDeferredAsyncsImpl();
          break;
        }
        
        case kInitFromNonVol:
        {
          EventLog *pLog = (EventLog *) pMsgBuf;
          
          pLog->InitFromNonVolImpl();
          break;
        }
        
        default:
          break;
      }

      // If I'm being told to exit the thread, then do so.
      if (fExitThread)
        break;
    }

    gFuncEntryExitMsg(fMessageLogSettings, "ThreadMain") << "Exiting..." << endl;
}
