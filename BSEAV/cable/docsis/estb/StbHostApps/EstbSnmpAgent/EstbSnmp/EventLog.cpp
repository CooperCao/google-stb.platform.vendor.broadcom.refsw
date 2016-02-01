//**************************************************************************
//
// Copyright (c) 2007-2010 Broadcom Corporation
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
//
//
//**************************************************************************
//    Filename: EventLog.cpp
//    Author:   Kevin O'Neal
//    Creation Date: 27-april-2000
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Source code for event log objects.
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include "EventLog.h"
#include "EventLogThread.h"
#include "MibObjs.h"
#include "EventLogNonVolSettings.h"
#include "NonVolDevice.h"


#if (BCM_CONSOLE_SUPPORT)
#include "SnmpCommandTable.h"
#endif

#include "BcmSnmpAgent.h"
#include "SnmpLog.h"

#if (SNMP_SUPPORT)
#include "docsDevEvMib.h"
#include "docsDevEvBridge.h"
#include "ifBridge.h"
#include "standardTrap.h"
#include "SnmpApi.h"
#else
// If we have no SNMP support, we need to define some constants here that are
// defined in docsDevEvBridge.h
#define CONSTVAL_DOCSDEVEVCONTROL_RESETLOG  1
#define CONSTVAL_DOCSDEVEVCONTROL_USEDEFAULTREPORTING  2
#endif


// Only need one event log thread, even with several event log instances.
BcmEventLogThread *EventLog::fpThread = NULL;

// By default, not silent.
bool EventLog::fSilent = false;

// Base class native events.  For now just a generic one to log an informational
// event with specified text.
DocsisEvent EventLog::fEvents [] = 
{
  {kGeneric, kInformation, (char *) "<P1>", kDefaultNewRowBehavior, kDefaultNvBehavior, kUnspecifiedTrap, NULL}
};

/******************************************************************************\
**                                                                            **
*  EventBucket class.  Does a kind of token-bucket algorithm to throttle trap  *
*  and syslog message generation per the docsDevEvThrottle... constraints      *
**                                                                            **
\******************************************************************************/

#if (SNMP_SUPPORT)    
EventBucket::EventBucket (BcmSnmpAgent *pAgent, unsigned int Threshold, unsigned int Interval, ThrottleAdminStatus Admin)
#else
EventBucket::EventBucket (unsigned int Threshold, unsigned int Interval, ThrottleAdminStatus Admin)
#endif
{
#if (SNMP_SUPPORT)    
  fpAgent = pAgent;
#endif
  fThreshold = Threshold;
  fInterval = Interval;
  fLastFill = 0;
  fThrottleAdminStatus = Admin;
  
  fContents = fThreshold;
}

EventBucket::~EventBucket ()
{
}

int EventBucket::Drip ()
{
  if (GetContents() == 0)
    return kEmpty;
    
  if (fContents > 0)
    fContents--;
   
  return fContents;
}

// PR 6827: Better to base throttling interval on sysUpTime instead of
// current time of day if possible, since TOD exchange can and will alter
// the current time of day, but not sysUpTime.
time_t EventBucket::GetCurrentIntervalTime () const
{
  time_t Now;
  
#if (SNMP_SUPPORT)
  if (fpAgent)
    Now = fpAgent->sysUpTime();
  else
#endif  
    time (&Now);
    
  return Now;
}

int EventBucket::Fill ()
{ 
  time_t Now = GetCurrentIntervalTime();
  
  fContents = fThreshold;
  fLastFill = Now;
  
  SnmpLogEv << "EventBucket::Fill - resetting throttling state." << endl;
  
  return fContents;
}

unsigned int EventBucket::GetInterval () const
{
  return fInterval;
}

unsigned int EventBucket::GetIntervalRemaining () const
{
  time_t IntervalEnd = fLastFill + fInterval;
  time_t Now = GetCurrentIntervalTime();
  
  if (Now > IntervalEnd)
    return 0;
  
  return IntervalEnd - Now;
}

unsigned int EventBucket::GetThreshold () const
{
  return fThreshold;
}

int EventBucket::GetContents ()
{
  time_t Now = GetCurrentIntervalTime();


  if (fThrottleAdminStatus == kUnconstrained)
    return fThreshold;
    
  if (fThrottleAdminStatus == kInhibited)
    return 0;

  // Refill the bucket if it's time.  This algorithm uses big old
  // fThreshold sized splooshes every fInterval seconds rather than 
  // a continuous stream of (fThreshold / fInterval) every second.
  if ((Now - fLastFill >= (int)fInterval) && (fThrottleAdminStatus != kStopAtThreshold))
    Fill ();
  
  return fContents;
}

int EventBucket::PeekContents () const
{
  return fContents;
}

ThrottleAdminStatus EventBucket::GetAdminStatus () const
{
  return fThrottleAdminStatus;
}

SNMP_STATUS EventBucket::SetInterval (unsigned int Interval)
{
  fInterval = Interval;
  
  // Refill now if necessary
  GetContents();
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS EventBucket::SetThreshold (unsigned int Threshold)
{
  int Difference = fThreshold - Threshold;
  int NewContents;

  // Make necessary adjustments to contents
  NewContents = fThreshold - Difference;
  
  if (NewContents < 0)
    fContents = 0;
  else
    fContents = NewContents;

  fThreshold = Threshold;
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS EventBucket::SetAdminStatus (ThrottleAdminStatus Admin)
{
  fThrottleAdminStatus = Admin;
  
  SnmpLogEv << "EventBucket::SetAdminStatus to " << (int) Admin << endl;
  
  // Per mib, refill the bucket at this time.
  // "Writing to this object resets the thresholding state."
  Fill ();
  
  return SNMP_NO_ERROR;
}



/******************************************************************************\
**                                                                            **
*  EventLog class                                                              *
*  Controls and provides an API for getting / setting elements which define    *
*  event log behaviour.  Access provided via Singleton() method.  This class   *
*  also contains the thread that provides a unique context for event logging,  *
*  so that time-critical applications won't have to deal with the possible     *
*  latency associated with sending trap & syslog messages.                     *
**                                                                            **
\******************************************************************************/

#if (SNMP_SUPPORT)
EventLog::EventLog (const char *pName, BcmSnmpAgent *pAgent)
  : fBucket (pAgent)
{
  fpAgent = pAgent;
#else
EventLog::EventLog (const char *pName)
{
#endif
  fpName = pName;
  fpSettings = NULL;
  
  fSyslogIp.Set (0,0,0,0);  
  fControl = CONSTVAL_DOCSDEVEVCONTROL_USEDEFAULTREPORTING;
  fTrapControl = 0;

  if (fpThread == NULL)
    fpThread = new BcmEventLogThread();

  fAsyncsInhibited = false;

#if (BCM_CONSOLE_SUPPORT)
  GetCommandTableSingleton().AddInstance(fpName, this);
#endif

#if (SNMP_SUPPORT)
  
  fpEventTable = NULL;
  
  // Use default control values defined by 1.1 OSS spec, section 4.4.2.3
  UseDefaultReporting ();
  
  // Find the event log table...
  fpEventTable = (docsDevEventTable *)fpAgent->FindTable (BcmString("docsDevEventTable"));

#else

  // For non-SNMP implementation, use syslog by default.
  for (int Loop = 0; Loop < EV_SEVERITY_LEVELS; Loop++)
    fLevelControl[Loop] = kLocalNonVol | kSyslog;

  // Also set a default syslog IP address
  fSyslogIp.Set (10,24,2,103);  
    
#endif
}

EventLog::~EventLog ()
{
#if (BCM_CONSOLE_SUPPORT)
  // Deregister myself from the command table.
  GetCommandTableSingleton().RemoveInstance(this);
#endif
}


#if (BCM_CONSOLE_SUPPORT)
// This static method allows an object (including myself) to get access to
// the singleton command table for controlling objects of my type.  This
// helps avoid order of initialization problems by ensuring that the
// object is created and initialized on-demand.
//
// Parameters:  None.
//
// Returns:
//      A reference to the command table singleton for my class.
//
BcmEventLogCommandTable &EventLog::GetCommandTableSingleton(void)
{
    static BcmEventLogCommandTable gCommandTable;
    return gCommandTable;
}
#endif


#if (SNMP_SUPPORT)
    // Accessor to the event table.
docsDevEventTable *EventLog::GetEventTable () const
{
  return fpEventTable;
}
#endif


DocsisEvent *EventLog::GetEvent (EventLogIdCode Id)
{
  int NumEvents = sizeof(fEvents) / sizeof(DocsisEvent);
  int Loop;
  DocsisEvent *pEvent = NULL;
  
  
  for (Loop = 0; Loop < NumEvents; Loop++)
  {
    if (fEvents[Loop].Id == Id)
    {
      pEvent = &fEvents[Loop];
      break;
    }
  }
  
  return pEvent;
}

SNMP_STATUS EventLog::StartStressTest (int NumEvents, bool Silent)
{
  // Tell the thread to start stressing us.
  SNMP_STATUS Status;

  fStressCount = 0;
  fStressEvents = NumEvents;
  
  fSilentStress = Silent;
  
  if (Silent)
    SnmpLogRaw << "SILENT" << endl;
    
  // Tell the thread to start stressing us.
  Status = fpThread->StartStressTest (this);
  
  // Log an event to kick the thread
  LogEvent (kGeneric, "Starting event log stress test", NULL, NULL, NULL, NULL);
  
  return Status;
}

SNMP_STATUS EventLog::StopStressTest ()
{
  char pText[64];
  SNMP_STATUS Status = fpThread->StopStressTest (this);
  
  sprintf (pText, "Stopping event log stress test after %d events.", fStressCount);
  
  LogEvent (kGeneric, pText, NULL, NULL, NULL, NULL);
  
  return Status;
}

void EventLog::StressTestImpl ()
{
  char pStressMsg[128];
  EventLogEntry *pEvent;
  
  if (fStressEvents)
    sprintf (pStressMsg, "%s: StressTest %d / %d", fpName, ++fStressCount, fStressEvents);
  else
    sprintf (pStressMsg, "%s: StressTest %d", fpName, ++fStressCount);
  
  pEvent = LogEventImpl (kGeneric, pStressMsg, NULL, NULL, NULL, NULL, !fSilentStress);
  if (pEvent)
    StoreEventToNonVol(pEvent);
  
  if (fStressCount == fStressEvents)
    StopStressTest();
}

SNMP_STATUS EventLog::LogEvent (EventLogIdCode Id, EventLogSeverityLevel Level, const char *pText, EventLogPolicy Policy)
{
#if (SNMP_SUPPORT)
  if (fpEventTable == NULL)
  {
    gLogMessageRaw << "WARNING: EventLog::LogEvent can't log events yet, not initialized. ID=" << (int)Id << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }
#endif
  
  if (fpThread == NULL)
  {
    gLogMessageRaw << "WARNING: EventLog::LogEvent can't log event, no thread. ID=" << (int)Id << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }

  return fpThread->LogEvent (Id, Level, pText, Policy, this);
}

SNMP_STATUS EventLog::LogEvent (EventLogIdCode Id, const char *pParam1, const char *pParam2, const char *pParam3, const char *pParam4, const char *pParam5)
{
#if (SNMP_SUPPORT)
  if (fpEventTable == NULL)
  {
    gLogMessageRaw << "WARNING: EventLog::LogEvent can't log events yet, not initialized. ID=" << (int)Id << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }
#endif
  
  if (fpThread == NULL)
  {
    gLogMessageRaw << "WARNING: EventLog::LogEvent can't log event, no thread. ID=" << (int)Id <<endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }

  return fpThread->LogEvent (Id, pParam1, pParam2, pParam3, pParam4, pParam5, this);
//  return fpThread->LogEvent (Id, pParam1, pParam2, pParam3, pParam4, pParam5, this);
}

SNMP_STATUS EventLog::SendTrap (int TrapType, int IfIndex)
{
  SnmpLogEv << "EventLog::SendStandardTrap, trap type " << TrapType << endl;

#if (SNMP_SUPPORT)
/*
  Standard traps don't require knowledge of the event table, so remove this
  restriction.
  if (fpEventTable == NULL)
  {
    gLogMessageRaw << "WARNING: EventLog::SendTrap can't send traps yet, not initialized. ID=" << TrapType << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }
*/
  
  if (fpThread == NULL)
  {
    gLogMessageRaw << "WARNING: EventLog::SendTrap can't send trap, no thread. ID="  << TrapType << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }

  return fpThread->SendTrap (TrapType, IfIndex, fpAgent);
#else
  return 0;
#endif
}

SNMP_STATUS EventLog::WriteNonVolSettings ()
{
  SnmpLogEv << "EventLog::WriteNonVol" << endl;

  if (fpThread == NULL)
  {
    gLogMessageRaw << "WARNING: EventLog::WriteNonVol can't write NV, no thread."  << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }

  return fpThread->WriteNonVolSettings (this);
}

SNMP_STATUS EventLog::ResetLog ()
{
  SnmpLogEv << "EventLog::ResetLog" << endl;

  if (fpThread == NULL)
  {
    gLogMessageRaw << "WARNING: EventLog::ResetLog can't reset, no thread."  << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }

  return fpThread->ResetLog (this);
}

bool EventLog::ShouldStoreEventToNonVol (const EventLogEntry *pEvent) const
{
#if (kDefaultValue_EventLogLocalVolatileEqualsLocalNonVol)
  // Section 4.4.2.1 of the OSS spec explicitly allows us to treat localVolatile
  // the same as localNonVol. "CMTS/CM events designated for local-volatile 
  // storage MAY also be retained in local-nonvolatile storage."
  if ((GetLevelControl(pEvent->GetLevel()) & kLocalNonVol)
  ||  (GetLevelControl(pEvent->GetLevel()) & kLocalVolatile))
    return true;
#else
  // OSS-N-01025 / 01066 support: don't log to NV if we have bit 0 clear
  if (GetLevelControl(pEvent->GetLevel()) & kLocalNonVol)
    return true;
#endif
  else
  {
    SnmpLogEv << "EventLog::ShouldStoreEventToNonVol - level " << pEvent->GetLevel() << " is local-volatile only, don't store to NV" << endl;
    return false;
  }
}

bool EventLog::ShouldWriteNonVolNow (const EventLogEntry *pEvent, bool IsNewEvent, bool IsNewText) const
{
  // If the policy is kImmediate, write.
  if (pEvent->NonVolPolicy() == kImmediate)
    return true;
    
  // If the policy is kDefer, don't write.
  if (pEvent->NonVolPolicy() == kDefer)
    return false;
    
  // We must now take steps to evade the CableLabs test scenario where a TEP
  // invokes an event, then power cycles the board rather than using a soft
  // reset mechanism like docsDevResetNow, then looks for the expected event
  // after reboot.  We can weasel around this by writing NV only when the event
  // is a new one in non-vol, in hopes that the TEP just looks for an event
  // ID or a text string and ignores the count and lastTime.  That way, we 
  // strike a balance between conserving flash writes and passing the CL tests.
  // Note, however, that the latest rev of certain tests that do this nonsense
  // do appear to check the counter, so if that's the case then all we can do
  // is identify the specific events that are tested and assign them a policy
  // of kImmediate.
    
#if (kDefaultValue_EventLogOptimizeForSmallerSize)
  
  // If we are optimized for smaller size, then we will only write NV if
  // this is a new event or if the text changed.  Simple check.
  if ((pEvent->NonVolPolicy() == kDefaultNvBehavior) && (IsNewText || IsNewEvent))
    return true;
    
#else
    
  // If we are optimizing the log for readability as opposed to smaller size,
  // then IsNewEvent will almost ALWAYS be true!  So in that case, we have to
  // do a little research.  Spin through the event log and look for an event
  // which matches this one (ie ID and text the same).  If we find one, don't
  // write NV.
  
  int MatchingEvents = 0;
  
  // This should never happen...
  if (fpSettings == NULL)
    return false; 

#if (SNMP_SUPPORT)  
  fpAgent->LockMib();
#endif
  
  // Iterate through events and look for one like this...  
  for (int Loop = 0; Loop < fpSettings->Capacity(); Loop++)
  {
    if ((fpSettings->Event(Loop).Id == (uint32) pEvent->GetId())
    &&  (pEvent->GetText() == fpSettings->Event(Loop).Text))
      MatchingEvents++;
  }

#if (SNMP_SUPPORT)    
  fpAgent->UnlockMib(); 
#endif
  
  // We will write immediately only if there is EXACTLY one event like this
  // in NV and his count is 1.  If his count is > 1 that means he's already
  // been written.
  if (MatchingEvents == 1)
  {
    if (pEvent->GetCount() <= 1)
      return true;
  }
  
#endif
    
  return false;
}

EventLogEntry *EventLog::FindEvent (unsigned long Id, const char *pText) const
{
  EventLogEntry *pEvent;
  

#if (SNMP_SUPPORT)
  if (fpEventTable == NULL)
    return NULL;
  
  fpAgent->LockMib();
#endif

  pEvent = LastEvent();

#if (kDefaultValue_EventLogOptimizeForSmallerSize)  
  
  // This implementatation is for when we want to generally update
  // docsDevEvCounts and docsDevEvLastTime rather than create a new entry
  // in the docsDevEventTable.  So if we have an event anywhere in the log
  // with the same ID and text, just update these members.
  
  while (pEvent)
  {
    if (pEvent->GetId() == Id)
    {
      if (pText)
      {
        // Found a match!  Event ID and text match the supplied event.
        if (pEvent->GetText() == pText)
          break;
      }
      // No text specified, all we match on is the ID.
      else
        break;
    }
      
    pEvent = PrevEvent (pEvent);
  }

#else
  
  // In this implementation, we will generally create a new entry unless the
  // event in question is the same as the last one in the log.  This produces
  // a more readable log, but it means that entries will rotate out faster,
  // resulting in a shorter history.  There is an exception to this, below...
  if (pEvent != NULL)
  {
    if (pText)
    {
      if(((uint32) pEvent->GetId() != Id)
      || (pEvent->GetText() != pText))
        pEvent = NULL;
    }
    
    // Now a special case: if we have pText == NULL, that means that this is
    // an UpdateExisting event.  In that case, we must actually recurse through
    // the list and look for ANY matching event (same ID).  In this case, we 
    // will not just update the last one, but any one.
    else
    {
      while (pEvent)
      {
        if ((uint32) pEvent->GetId() == Id)
          break;
          
        pEvent = PrevEvent (pEvent);
      }
    }
  }
#endif
  
#if (SNMP_SUPPORT)
  fpAgent->UnlockMib();
#endif
    
  return pEvent;
}

EventLogEntry *EventLog::FindEventFromNvIndex (int NvIndex) const
{
  EventLogEntry *pEvent = NULL;
  
#if (SNMP_SUPPORT)
  if (fpEventTable == NULL)
    return NULL;
  
  fpAgent->LockMib();
#endif

  pEvent = FirstEvent ();
  while (pEvent)
  {
    if (pEvent->fNvIndex == NvIndex)
      break;
    
    pEvent = NextEvent(pEvent);
  }

#if (SNMP_SUPPORT)  
  fpAgent->UnlockMib();
#endif
    
  return pEvent;
}

int EventLog::Capacity () const
{
  if (fpSettings)
    return fpSettings->Capacity();
  
  return 0;
}

#if (SNMP_SUPPORT)
// Actually create and log the event
EventLogEntry *EventLog::LogEventImpl (EventLogIdCode Id, EventLogSeverityLevel Level, const char *pText, EventLogPolicy Policy, bool PrintToConsole)
{
  docsDevEventEntryBridge *pBridge = NULL;
  EventLogEntry *pEvent = NULL;
  DocsisEvent *pArgs = GetEvent (Id);
  int EvLevel, EvPolicy;
  const char *pEvText = NULL, *pSyslogAppend = NULL;
  DocsDevTrapTypeHint TrapType = kUnspecifiedTrap;


  if (fpEventTable == NULL)
  {
    gLogMessageRaw << "WARNING: EventLog::LogEventImpl can't log events yet, not initialized. ID=" << (int) Id  << endl;
    return NULL;
  }
  
  SnmpLogEv << "EventLog::LogEventImpl for ID " << (unsigned int) Id << " with DECODED objects." << endl;
  
  // Change 5/18/2001: if the requested event isn't in our default table, then
  // don't log it.  This will protect against problems if somehow we wind up
  // with a bogus event.
  if (pArgs == NULL)
  {
#if (!BCM_REDUCED_IMAGE_SIZE)
    if (PrintToConsole == true)
      gLogMessageRaw << "Not logging event ID " << (unsigned int) Id << ", event info not found." << endl;
#endif
    return NULL;
  }
  
  fpAgent->LockMib();

  /* Use any defaults that are specified */
  
  if (Level == kUseDefaultLevel)
  {
    if (pArgs)
      EvLevel = pArgs->Level;
    else // default requested but none specified
      EvLevel = kInformation;
  }
  else
    EvLevel = Level;
    
  if (Policy == kUseDefaultPolicy)
  {
    if (pArgs)
      EvPolicy = pArgs->Policy;
    else // default requested but none specified
      EvPolicy = kCreateNew;
  }
  else
    EvPolicy = Policy;
    
  if ((pText == NULL) || (pText[0] == 0))
  {
    if (pArgs)
    {
      pEvText = pArgs->pText;
      pSyslogAppend = pArgs->pSyslogAppend;
    }
    else // default requested but none specified
    {
      pEvText = "No default text specified";
      pSyslogAppend = NULL;
    }
  }
  else
    pEvText = pText;
    
  if (pArgs)
    TrapType = pArgs->TrapType;
    
  // If we have no event text, then don't log the event.
  if (pEvText == NULL)
  {
    if (PrintToConsole == true)
      gLogMessageRaw << "Not logging event ID " << (unsigned int) Id << ", no text specified." << endl;
    
    fpAgent->UnlockMib();
    return NULL;
  }
    
  // If neither kLocalVolatile or kLocalNonVol is specified for this
  // level, don't log the event.
  if (GetLevelControl(EvLevel) == 0)
  { 
    if (PrintToConsole == true) 
      gLogMessageRaw << "Not logging event ID " << (unsigned int) Id << ", control  for level " << EvLevel << " is 0." << endl;
    
    fpAgent->UnlockMib();
    return NULL;
  }
  
  // If kUpdateExisting or kDefaultNewRowBehavior is specified, find that event 
  // and re-log it.
  if (EvPolicy != kCreateNew)
  {
    BcmString MatchText;
    const char *pMatchText = NULL;
    
    // If kDefaultNewRowBehavior is specified, then we need to go ahead and
    // decode any embedded objects now so we can compare against the text
    // of the event.
    if (EvPolicy == kDefaultNewRowBehavior)
    {
      DecodeEmbeddedObjects (BcmString(pEvText), MatchText);
      pMatchText = MatchText.c_str();
    }
    
    pEvent = FindEvent (Id, pMatchText);
    
    if (pEvent)
    {
      // Gotta unlock the mutex PRIOR to calling Log, because Log may send out
      // a trap and/or syslog message, in which case we could be exposed to
      // a deadlock situation if any of the HAL threads are waiting on the
      // mutex.
      fpAgent->UnlockMib();
      pEvent->Log(PrintToConsole);
      
      return pEvent;
    }
  }
  
  pBridge= new docsDevEventEntryBridge (Id, EvLevel, pEvText, pSyslogAppend, TrapType, fpAgent, this);
  
  //fpAgent->LockMib();
  
  // Could speed things up a bit here by making a special Register function for the
  // DocsDevEventTableBridge that already knows where to register rather than
  // using the base class Register that has to search all the mib objects.
  if (pBridge->Register(kDestroyMibObject) != SNMP_NO_ERROR)
  {
    fpAgent->UnlockMib();
    
    delete (pBridge);
    pBridge = NULL;
    
    return NULL;
  }
   
  if (pBridge)
  {
    SnmpLogEv << "Bridge OK, getting event." << endl;
    pEvent = pBridge->GetEvent();
  }
  
  fpAgent->UnlockMib();
    
  if (pEvent)
  {
    SnmpLogEv << "Event: " << (void *)pEvent << endl << *pEvent << endl;
    if (pArgs)
      pEvent->fNonVolPolicy = pArgs->WriteToNV;
      
    pEvent->Log(PrintToConsole);
  }
  else
    SnmpLogEv << "WARNING: pEvent is NULL!" << endl;
                                                        
  return pEvent;
}

EventLogEntry *EventLog::LogEventImpl (EventLogIdCode Id, const char *pParam1, const char *pParam2, const char *pParam3, const char *pParam4, const char *pParam5, bool PrintToConsole)
{
  docsDevEventEntryBridge *pBridge = NULL;
  EventLogEntry *pEvent = NULL;
  DocsisEvent *pArgs = GetEvent (Id);
  int EvLevel, EvPolicy;
  DocsDevTrapTypeHint TrapType = kUnspecifiedTrap;
  BcmString EvText, SyslogAppend;
  
  /* Use any defaults that are specified */
  
  if (fpEventTable == NULL)
  {
    gLogMessageRaw << "WARNING: EventLog::LogEventImpl can't log events yet, not initialized. ID=" << (int) Id << endl;
    return NULL;
  }
  
  // Change 5/18/2001: if the requested event isn't in our default table, then
  // don't log it.  This will protect against problems if somehow we wind up
  // with a bogus event.
  if (pArgs == NULL)
  {
    if (PrintToConsole == true)
      gLogMessageRaw << "Not logging event ID " << (unsigned int) Id << ", event info not found." << endl;
    return NULL;
  }
  
  SnmpLogEv << "EventLog::LogEventImpl for ID " << (int) Id << " with REPLACED objects." << endl;
  
  fpAgent->LockMib();
  
  if (pArgs)
    EvLevel = pArgs->Level;
  else // default requested but none specified
    EvLevel = kInformation;
  
  if (pArgs)
    EvPolicy = pArgs->Policy;
  else // default requested but none specified
    EvPolicy = kCreateNew;
  
  if (pArgs)
    TrapType = pArgs->TrapType;
    
  // If neither kLocalVolatile or kLocalNonVol is specified for this
  // level, don't log the event.
  if (GetLevelControl(EvLevel) == 0)
  {
    if (PrintToConsole == true)
      gLogMessageRaw << "Not logging event ID " << (unsigned int) Id << ", control  for level " << EvLevel << " is 0." << endl;
    
    fpAgent->UnlockMib();
    return NULL;
  }

  if (pArgs)
  {
    EvText = pArgs->pText;
    if (pArgs->pSyslogAppend)
      SyslogAppend = pArgs->pSyslogAppend;
  }
  else // default requested but none specified: just concatenate any pParams together.
  {
    if ((pParam1 == NULL) && (pParam2 == NULL) && (pParam3 == NULL) && (pParam4 == NULL))
      EvText = "No default text specified";

    if (pParam1)
      EvText = pParam1;
    if (pParam2)
    {
      EvText += " ";
      EvText += pParam2;
    }
    if (pParam3)
    {
      EvText += " ";
      EvText += pParam3;
    }
    if (pParam4)
    {
      EvText += " ";
      EvText += pParam4;
    }
    if (pParam5)
    {
      EvText += " ";
      EvText += pParam5;
    }
  }
  
  // If kUpdateExisting is specified, find that event and re-log it.
  if (EvPolicy != kCreateNew)
  {
    BcmString MatchText;
    const char *pMatchText = NULL;
    
    // If kDefaultNewRowBehavior is specified, then we need to go ahead and
    // replace any embedded objects now so we can compare against the text
    // of the event.
    if (EvPolicy == kDefaultNewRowBehavior)
    {
      ReplaceEmbeddedObjects (EvText, MatchText, pParam1, pParam2, pParam3, pParam4, pParam5);
      pMatchText = MatchText.c_str();
    }
    
    pEvent = FindEvent (Id, pMatchText);

    if (pEvent)
    {
      fpAgent->UnlockMib();
      pEvent->Log(pParam1, pParam2, pParam3, pParam4, pParam5, PrintToConsole);
      return pEvent;
    }
  }
  
  pBridge= new docsDevEventEntryBridge (Id, EvLevel, EvText.c_str(), SyslogAppend.c_str(), TrapType, fpAgent, this);

  // Could speed things up a bit here by making a special Register function for the
  // docsDevEventEntryBridge that already knows where to register rather than
  // using the base class Register that has to search all the mib objects.
  if (pBridge->Register(kDestroyMibObject) != SNMP_NO_ERROR)
  {
    delete (pBridge);
    pBridge = NULL;
  }
   
  if (pBridge)
  {
    pEvent = pBridge->GetEvent();
  }
  
  fpAgent->UnlockMib();
    
  if (pEvent)
  {
    if (pArgs)
      pEvent->fNonVolPolicy = pArgs->WriteToNV;
      
    pEvent->Log(pParam1, pParam2, pParam3, pParam4, pParam5, PrintToConsole);
  }
                                                        
  return pEvent;
}

EventLogEntry *EventLog::FirstEvent () const
{
  docsDevEventEntry *pEvEntry;
  EventLogEntry *pEvent = NULL;
  
  
  if (fpEventTable == NULL)
    return NULL;

  fpAgent->LockMib();
  pEvEntry = (docsDevEventEntry *)fpEventTable->FirstEntry ();
  
  if (pEvEntry)
    pEvent = pEvEntry->GetEvent();
    
  fpAgent->UnlockMib();
    
  return pEvent;
}

EventLogEntry *EventLog::NextEvent (EventLogEntry *pPrevEvent) const
{ 
  docsDevEventEntry *pEvEntry;
  EventLogEntry *pEvent = NULL;
  BcmObjectId Index;
  
  
  if (pPrevEvent == NULL)
    return FirstEvent();
    
  Index = (unsigned long)pPrevEvent->GetIndex();
  
  if (fpEventTable == NULL)
    return NULL;

  fpAgent->LockMib();
  
  pEvEntry = (docsDevEventEntry *) fpEventTable->NextEntry (Index);
  
  if (pEvEntry)
    pEvent = pEvEntry->GetEvent();
  
  fpAgent->UnlockMib();
    
  return pEvent;
}

EventLogEntry *EventLog::LastEvent () const
{
  docsDevEventEntry *pEvEntry;
  EventLogEntry *pEvent = NULL;
  
  
  if (fpEventTable == NULL)
    return NULL;

  fpAgent->LockMib();
  pEvEntry = (docsDevEventEntry *)fpEventTable->LastEntry ();
  
  if (pEvEntry)
    pEvent = pEvEntry->GetEvent();
    
  fpAgent->UnlockMib();
    
  return pEvent;
}

EventLogEntry *EventLog::PrevEvent (EventLogEntry *pPrevEvent) const
{ 
  docsDevEventEntry *pEvEntry;
  EventLogEntry *pEvent = NULL;
  BcmObjectId Index;
  
  
  if (pPrevEvent == NULL)
    return LastEvent();
    
  Index = (unsigned long)pPrevEvent->GetIndex();
  
  if (fpEventTable == NULL)
    return NULL;

  fpAgent->LockMib();
  
  pEvEntry = (docsDevEventEntry *) fpEventTable->PrevEntry (Index);
  
  if (pEvEntry)
    pEvent = pEvEntry->GetEvent();
  
  fpAgent->UnlockMib();
    
  return pEvent;
}

SNMP_STATUS EventLog::SendDeferredAsyncs (bool Async)
{
  SnmpLogEv << "EventLog::SendDeferredAsyncs" << endl;

  if (fpThread == NULL)
  {
    gLogMessageRaw << "WARNING: EventLog::SendDeferredAsyncs can't run, no thread."  << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }
  
  // Allow the caller to specify immediate sending instead of async.  This
  // would not be a typical usage, normally this call occurs asynchronously.
  if (Async == false)
    return SendDeferredAsyncsImpl();

  return fpThread->SendDeferredAsyncs (this);
}

SNMP_STATUS EventLog::SendDeferredAsyncsImpl ()
{
  int Traps, Syslogs;


  SnmpLogRaw << fpName << " sending deferred async messages..." << endl;
  
#if (SNMP_SUPPORT)  
  // PR 5883: Lock the MIB here.  Avoid a situation where our agent is not
  // enabled when we send the syslog message (thus deferring it), but becomes
  // enabled prior to sending traps (thus they are sent immediately).  This
  // situation will result in a miscount for throttling purposes because the
  // traps will be counted immediately, then the syslog message will be counted
  // again later.
  if (fpAgent)
    fpAgent->LockMib();
#endif
  
  // Then, iterate through all events and send any traps or syslog
  // messages that might be there.
  EventLogEntry *pEvent = FirstEvent();
  while (pEvent)
  {
    Syslogs = pEvent->SendSyslogMsg (true);
    Traps = pEvent->SendTraps (true);
    
    // Apply throttling rules to deferred msgs
    if ((Traps > 0) || (Syslogs > 0))
    {
      if (Syslogs > Traps)
        SentAsyncMsg(pEvent, Syslogs);
      else
        SentAsyncMsg(pEvent, Traps);
    }
      
    pEvent = NextEvent(pEvent);
  }
  
#if (SNMP_SUPPORT)  
  if (fpAgent)
    fpAgent->UnlockMib();
#endif
  
  SnmpLogRaw << "Done w/ deferred msgs" << endl;

  return SNMP_NO_ERROR;
}


SNMP_STATUS EventLog::ResetLogImpl ()
{
  docsDevEventEntry *pEvEntry; 
  BcmObjectId Index;
  

  if (fpEventTable == NULL)
  {
    fpEventTable = (docsDevEventTable *)fpAgent->FindTable (BcmString("docsDevEventTable"));
    if (fpEventTable == NULL)
      return SNMP_RESOURCE_UNAVAILABLE;
  }
  
  fpAgent->LockMib();
  pEvEntry = (docsDevEventEntry *)fpEventTable->FirstEntry ();
  
  // Spin through the event log and destroy all entries.
  while (pEvEntry)
  {
    pEvEntry->GetIndex (Index);
    fpEventTable->DestroyEntry (Index);
    pEvEntry = (docsDevEventEntry *)fpEventTable->FirstEntry ();
  }
  
  fpAgent->UnlockMib();
  
  // Flush the log from NV, too.  Note that this will NOT actually
  // write NV, just empty the NV event log settings from RAM
  ClearNonVol();
    
  return SNMP_NO_ERROR;
}

bool EventLog::IsThrottled ()
{
  if (fAsyncsInhibited == true)
  {
    SnmpLogEv << "EventLog::IsThrottled - throttled because fAsyncsInhibited is TRUE" << endl;
    return true;
  }

  if (fBucket.GetAdminStatus() == kUnconstrained)
  {
    SnmpLogEv << "EventLog::IsThrottled - not throttled because bucket adminstatus is UNCONSTRAINED" << endl;
    return false;
  }
    
  if (fBucket.GetAdminStatus() == kInhibited)
  {
    SnmpLogEv << "EventLog::IsThrottled - throttled because bucket adminstatus is INHIBITED" << endl;
    return true;
  }
    
  if (fBucket.GetContents() == 0)
  {
    SnmpLogEv << "EventLog::IsThrottled - throttled because bucket is EMPTY" << endl;
    return true;
  }
   
  SnmpLogEv << "EventLog::IsThrottled - not throttled: " << fBucket.GetContents() << " of " << fBucket.GetThreshold() << " drips remain this " << fBucket.GetInterval() << " seconds." << endl; 
  return false;
}

#else
/* NON-SNMP implementation follows... */

// Actually create and log the event
EventLogEntry *EventLog::LogEventImpl (EventLogIdCode Id, EventLogSeverityLevel Level, const char *pText, EventLogPolicy Policy, bool PrintToConsole)
{
  EventLogEntry *pEvent = NULL;
  DocsisEvent *pArgs = GetEvent (Id);
  int EvLevel, EvPolicy, Index = 0;
  const char *pEvText = NULL, *pSyslogAppend = NULL;
  DocsDevTrapTypeHint TrapType = kUnspecifiedTrap;
  EventLogTable::reverse_iterator Pos;


  SnmpLogEv << "NO-SNMP LogEventImpl" << endl;

  // Change 5/18/2001: if the requested event isn't in our default table, then
  // don't log it.  This will protect against problems if somehow we wind up
  // with a bogus event.
  if (pArgs == NULL)
  {
    if (PrintToConsole == true)
      gLogMessageRaw << "Not logging event ID " << (unsigned int) Id << ", event info not found." << endl;
    return NULL;
  }

  /* Use any defaults that are specified */
  
  if (Level == kUseDefaultLevel)
  {
    if (pArgs)
      EvLevel = pArgs->Level;
    else // default requested but none specified
      EvLevel = kInformation;
  }
  else
    EvLevel = Level;
    
  if (Policy == kUseDefaultPolicy)
  {
    if (pArgs)
      EvPolicy = pArgs->Policy;
    else // default requested but none specified
      EvPolicy = kCreateNew;
  }
  else
    EvPolicy = Policy;
    
  if ((pText == NULL) || (pText[0] == 0))
  {
    if (pArgs)
      pEvText = pArgs->pText;
    else // default requested but none specified
      pEvText = "No default text specified";
  }
  else
    pEvText = pText;
    
  if ((pArgs) && (pArgs->pSyslogAppend))
    pSyslogAppend = pArgs->pSyslogAppend;
  else
    pSyslogAppend = "";
    
  // If kUpdateExisting is specified, find that event and re-log it.
  if (EvPolicy != kCreateNew)
  {
    BcmString MatchText;
    const char *pMatchText = NULL;
    
    // If kDefaultNewRowBehavior is specified, then we need to go ahead and
    // decode any embedded objects now so we can compare against the text
    // of the event.
    if (EvPolicy == kDefaultNewRowBehavior)
    {
      DecodeEmbeddedObjects (BcmString(pEvText), MatchText);
      pMatchText = MatchText.c_str();
    }
    
    pEvent = FindEvent (Id, pMatchText);

    if (pEvent)
    {
      SnmpLogEv << "Matched event, logging." << endl;
      pEvent->Log(PrintToConsole);
      return pEvent;
    }
  }
  
  // Determine the new index by getting the last item from the list
  // and incrementing its index.
  Pos = fEventTable.rbegin();
  if (Pos != fEventTable.rend())
  {
    pEvent = *Pos;
    Index = pEvent->GetIndex() + 1;
  }
  
  SnmpLogEv << "Creating new event at index " << Index << ", text = '" << pEvText << "'" << endl;
  
  //pEvent = new EventLogEntry (Index, Id, EvLevel, BcmString (pEvText), kUnspecifiedTrap);
  pEvent = new EventLogEntry (Index, Id, EvLevel, BcmString (pEvText), BcmString (pSyslogAppend), TrapType, this);
      
  if (pEvent)
  {
    pEvent->Log();
    
    if (pArgs)
      pEvent->fNonVolPolicy = pArgs->WriteToNV;
      
    fEventTable.push_back (pEvent);
    
    // Check for how many entries here and pop off the front one if
    // necessary.
    if (fEventTable.size() > Capacity())
    {
      pEvent = fEventTable.front();
      delete pEvent;
      
      fEventTable.pop_front();
    }
  }
                                                        
  return pEvent;
}

EventLogEntry *EventLog::LogEventImpl (EventLogIdCode Id, const char *pParam1, const char *pParam2, const char *pParam3, const char *pParam4, const char *pParam5, bool PrintToConsole)
{
  EventLogEntry *pEvent = NULL;
  DocsisEvent *pArgs = GetEvent (Id);
  int EvLevel, EvPolicy, NumUsed, Index;
  DocsDevTrapTypeHint TrapType = kUnspecifiedTrap;
  BcmString EvText, SyslogAppend;
  EventLogTable::reverse_iterator Pos;


  // Change 5/18/2001: if the requested event isn't in our default table, then
  // don't log it.  This will protect against problems if somehow we wind up
  // with a bogus event.
  if (pArgs == NULL)
  {
    if (PrintToConsole == true)
      gLogMessageRaw << "Not logging event ID " << (unsigned int) Id << ", event info not found." << endl;
    return NULL;
  }

  /* Use any defaults that are specified */
  
  if (pArgs)
    EvLevel = pArgs->Level;
  else // default requested but none specified
    EvLevel = kInformation;
  
  if (pArgs)
    EvPolicy = pArgs->Policy;
  else // default requested but none specified
    EvPolicy = kCreateNew;
  
  if (pArgs)
    TrapType = pArgs->TrapType;
    
  if (pArgs)
  {
    EvText = pArgs->pText;
    if (pArgs->pSyslogAppend)
      SyslogAppend = pArgs->pSyslogAppend;
  }
  else // default requested but none specified: just concatenate any pParams together.
  {
    if ((pParam1 == NULL) && (pParam2 == NULL) && (pParam3 == NULL) && (pParam4 == NULL))
      EvText = "No default text specified";

    if (pParam1)
      EvText = pParam1;
    if (pParam2)
    {
      EvText += " ";
      EvText += pParam2;
    }
    if (pParam3)
    {
      EvText += " ";
      EvText += pParam3;
    }
    if (pParam4)
    {
      EvText += " ";
      EvText += pParam4;
    }
    if (pParam5)
    {
      EvText += " ";
      EvText += pParam5;
    }
  }
  
  // If kUpdateExisting is specified, find that event and re-log it.
  if (EvPolicy != kCreateNew)
  {
    BcmString MatchText;
    const char *pMatchText = NULL;
    
    // If kDefaultNewRowBehavior is specified, then we need to go ahead and
    // replace any embedded objects now so we can compare against the text
    // of the event.
    if (EvPolicy == kDefaultNewRowBehavior)
    {
      ReplaceEmbeddedObjects (EvText, MatchText, pParam1, pParam2, pParam3, pParam4, pParam5);
      pMatchText = MatchText.c_str();
    }
    
    pEvent = FindEvent (Id, pMatchText);

    if (pEvent)
    {
      pEvent->Log(PrintToConsole);
      return pEvent;
    }
  }
  
  // Determine the new index by getting the last item from the list
  // and incrementing its index.
  Pos = fEventTable.rbegin();
  if (Pos != fEventTable.rend())
  {
    pEvent = *Pos;
    Index = pEvent->GetIndex() + 1;
  }
  
  //pEvent= new EventLogEntry (Index, Id, EvLevel, EvText, kUnspecifiedTrap);
  pEvent = new EventLogEntry (Index, Id, EvLevel, EvText, SyslogAppend, TrapType, this);
      
  if (pEvent)
  {
    pEvent->Log();
    
    if (pArgs)
      pEvent->fNonVolPolicy = pArgs->WriteToNV;
      
    fEventTable.push_back (pEvent);
    
    // Check for how many entries here and pop off the front one if
    // necessary.
    if (fEventTable.size() > Capacity())
    {
      pEvent = fEventTable.front();
      delete pEvent;
      
      fEventTable.pop_front();
    }
    
    pEvent->Log(pParam1, pParam2, pParam3, pParam4, pParam5, PrintToConsole);
  }
                                                        
  return pEvent;
}

EventLogEntry *EventLog::FirstEvent () const
{
  EventLogConstIterator Pos;
  
  
  Pos = fEventTable.begin();
  if (Pos != fEventTable.end())
    return *Pos;
    
  return NULL;
}

EventLogEntry *EventLog::NextEvent (EventLogEntry *pPrevEvent) const
{ 
  EventLogEntry *pEvent;
  EventLogConstIterator Pos;
  bool NextOne = false;
  
  
  Pos = fEventTable.begin();
  while (Pos != fEventTable.end())
  {
    pEvent = *Pos;
    
    if (NextOne == true)
      return pEvent;
      
    if (pEvent == pPrevEvent)
      NextOne = true;
      
    Pos++;
  }
  
  return NULL;
}


EventLogEntry *EventLog::LastEvent () const
{
  EventLogConstIterator Pos;
  
  
  if (fEventTable.empty() == true)
    return NULL;
  
  Pos = fEventTable.end();
  Pos--;
  
  return *Pos;
}

EventLogEntry *EventLog::PrevEvent (EventLogEntry *pPrevEvent) const
{ 
  EventLogEntry *pEvent;
  EventLogConstIterator Pos;
  bool NextOne = false;
  
  
  Pos = fEventTable.end();
  while (Pos != fEventTable.begin())
  {
    pEvent = *Pos;
    
    if (NextOne == true)
      return pEvent;
      
    if (pEvent == pPrevEvent)
      NextOne = true;
      
    Pos--;
  }
  
  return NULL;
}


SNMP_STATUS EventLog::ResetLogImpl ()
{
  EventLogEntry *pEvent;
  EventLogIterator Pos;
  
  
  // Spin through the event log and destroy all entries.
  Pos = fEventTable.begin();
  while (Pos != fEventTable.end())
  {
    pEvent = *Pos;
    delete pEvent;
    
    Pos++;
  }
  // Then flush the table.
  fEventTable.clear();
  
  // Flush the log from NV, too.
  ClearNonVol();
    
  return SNMP_NO_ERROR;
}


// Don't bother if we have no SNMP.
SNMP_STATUS EventLog::SendDeferredAsyncsImpl ()
{
  return SNMP_NO_ERROR;
}


bool EventLog::IsThrottled ()
{
  static BcmIpAddress NullIp(0,0,0,0);
  bool GotTraps = false;
  
  
  if (fAsyncsInhibited == true)
    return true;
  
  // Do we have a syslog IP addr?
  if ((GotTraps == false)
  &&  (fSyslogIp == NullIp))
    return true;
  
  if (fBucket.GetAdminStatus() == kUnconstrained)
    return false;
    
  if (fBucket.GetAdminStatus() == kInhibited)
    return true;
    
  if (fBucket.GetContents() == 0)
    return true;
    
  return false;
}

#endif

int EventLog::SendSyslog (docsDevSyslog *pSyslog, bool PrintToConsole)
{
  int NumLogs = 0;

#if (SNMP_SUPPORT)   
  // If we have traps deferred, don't send the message, tank it
  // up to later.
  if (fpAgent->TrapsDeferred() == true)
    return -1;
#endif

  // Check to see if we're throttled
  if (IsThrottled() == true)
    return 0;
    
  // Send to docsDevEvSyslog first
  if (pSyslog->Send (GetSyslogIp(), false, !PrintToConsole) == SNMP_NO_ERROR)
    NumLogs++;

/* 
  Don't send to DHCP log servers.  If we get a log server from DHCP and not
  in the config file, we'll set docsDevEvSyslog to what the DHCP offer tells
  us.  However, we must make sure that if the cfg file says docsDevSyslogServer
  is 0.0.0.0 that we don't send any syslogs.
    
  SNMP_STATUS Status = SNMP_NO_ERROR;
  
  // Then send to any log servers from DHCP
  SnmpLogEv << "Sending syslog message to DHCP log servers" << endl;
  list <BcmIpAddress>::iterator Pos = fLogServers.begin();
  while (Pos != fLogServers.end())
  {
    // Skip any 0.0.0.0's
    if (*Pos == kAllZerosIp)
    {
      Pos++;
      continue;
    }
    
    // Also skip any entries that match docsDevEvSyslog, since we already
    // sent it, above.
    if (*Pos != GetSyslogIp())
      Status = pSyslog->Send(*Pos);
    
    if (Status != SNMP_NO_ERROR)
      SnmpLogEv << "   docsDevSyslog::Send failed w/ error code " << Status << endl;
    else
    {
      SnmpLogEv << "  Sent OK" << endl;
      NumLogs++;
    }
      
    Pos++;
  }
*/
  
  return NumLogs;
}

SNMP_STATUS EventLog::UseDefaultReporting ()
{
#if (kDefaultValue_EventLogDynamicLevelReporting)
  unsigned short OldLevel[EV_SEVERITY_LEVELS];
  unsigned short Control, OldControl;
  
  // Remember previous settings
  memcpy (OldLevel, fLevelControl, sizeof(OldLevel));
#endif

#if (SNMP_SUPPORT)
  // Error, warning, notice reporting changed per OSS-I06
  fLevelControl[kEmergency-1]   = kLocalNonVol; // | kLocalVolatile?
  fLevelControl[kAlert-1]       = kLocalNonVol; // | kLocalVolatile?
  fLevelControl[kCritical-1]    = kLocalNonVol; // | kLocalVolatile?
  fLevelControl[kError-1]       = kSyslog | kTraps | kLocalNonVol;
  fLevelControl[kWarning-1]     = kLocalNonVol;
  fLevelControl[kNotice-1]      = kLocalVolatile;
  fLevelControl[kInformation-1] = 0;
  fLevelControl[kDebug-1]       = 0;
#else
  // With no SNMP support, by default turn off event logging.  If it is to
  // be enabled you'll have to do that from the event log directory.
  // No-SNMP builds are typically only used during things like new chip or
  // OS bringup, and in that case event logging is generally not necessary
  // and in many cases causes trouble.
  fLevelControl[kEmergency-1]   = 0;
  fLevelControl[kAlert-1]       = 0;
  fLevelControl[kCritical-1]    = 0;
  fLevelControl[kError-1]       = 0;
  fLevelControl[kWarning-1]     = 0;
  fLevelControl[kNotice-1]      = 0;
  fLevelControl[kInformation-1] = 0;
  fLevelControl[kDebug-1]       = 0;
#endif

#if (kDefaultValue_EventLogDynamicLevelReporting)
#if (SNMP_SUPPORT)
  if (fpEventTable)
#endif
  {
    // If local display changed from yes to no or vice-versa for any level,
    // rebuild the table from NV.
    for (int Loop = 0; Loop < EV_SEVERITY_LEVELS; Loop++)
    {
      OldControl = OldLevel[Loop];
      Control = fLevelControl[Loop];
      
      if (((OldControl & (kLocalNonVol | kLocalVolatile)) == 0)
      &&  ((Control & (kLocalNonVol | kLocalVolatile)) != 0))
      {
        InitFromNonVol();
        break;
      }
      else if (((OldControl & (kLocalNonVol | kLocalVolatile)) != 0)
      &&  ((Control & (kLocalNonVol | kLocalVolatile)) == 0))
      {
        InitFromNonVol();
        break;
      }
    }
  }
#endif

  return SNMP_NO_ERROR;
}

SNMP_STATUS EventLog::DisableLogging ()
{
  // Turn off all levels of logging but don't init from nonvol.
  fLevelControl[kEmergency-1]   = 0;
  fLevelControl[kAlert-1]       = 0;
  fLevelControl[kCritical-1]    = 0;
  fLevelControl[kError-1]       = 0;
  fLevelControl[kWarning-1]     = 0;
  fLevelControl[kNotice-1]      = 0;
  fLevelControl[kInformation-1] = 0;
  fLevelControl[kDebug-1]       = 0;
  
  return SNMP_NO_ERROR;
}

const BcmIpAddress &EventLog::GetSyslogIp() const
{
  return fSyslogIp;
}

void EventLog::GetSyslogIp(unsigned char *pIp) const
{
  fSyslogIp.Get (pIp[0],pIp[1],pIp[2],pIp[3]);
}

int EventLog::GetThrottleAdminStatus () const
{
  return fBucket.GetAdminStatus();
}

bool EventLog::GetThrottleInhibited ()
{
  return IsThrottled();
}

int EventLog::GetThrottleThreshold () const
{
  return fBucket.GetThreshold();
}

int EventLog::GetThrottleInterval () const
{
  return fBucket.GetInterval();
}

unsigned short EventLog::GetLevelControl (int Level) const
{
  // Special case: level -1 means just indicate if
  // logging is on at all.
  if (Level == -1)
  {
    if ((fLevelControl[0] == 0)
    &&  (fLevelControl[1] == 0)
    &&  (fLevelControl[2] == 0)
    &&  (fLevelControl[3] == 0)
    &&  (fLevelControl[4] == 0)
    &&  (fLevelControl[5] == 0)
    &&  (fLevelControl[6] == 0)
    &&  (fLevelControl[7] == 0))
      return 0;
    else
      return 1;
  }

  if ((Level < kEmergency) || (Level > kDebug))
    return 0;
    
  return fLevelControl[Level-1];
}

// This method is called by EventLogEntry to inform the event log that an
// async message has been sent.  This is necessary for proper rate throttling.
// Note that in the case of a standard trap being sent, this might be called
// with pEvent = NULL.
int EventLog::SentAsyncMsg (EventLogEntry *pEvent, int NumMsgs)
{
  int Loop, Drips = 0;

  for (Loop = 0; Loop < NumMsgs; Loop++)
    Drips = fBucket.Drip();
    
  return Drips;
}

SNMP_STATUS EventLog::SetLogControl (int Control)
{
  if (Control == CONSTVAL_DOCSDEVEVCONTROL_RESETLOG)
  {
    ResetLog ();
    WriteNonVolSettings ();
    return SNMP_NO_ERROR;
  }
  else if (Control == CONSTVAL_DOCSDEVEVCONTROL_USEDEFAULTREPORTING)
    return UseDefaultReporting ();
  else
    return SNMP_WRONG_VALUE;
}

SNMP_STATUS EventLog::SetTrapControl (unsigned short Control)
{
  fTrapControl = Control;
  return SNMP_NO_ERROR;
}

SNMP_STATUS EventLog::SetSyslogIp (const BcmIpAddress &Ip)
{
  fSyslogIp = Ip;
  
  // If IP is 0.0.0.0, flush our list of additional log servers.
  if (fSyslogIp == kAllZerosIp)
    fLogServers.Reset();
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS EventLog::SetSyslogIp (const unsigned char *pIp)
{
  fSyslogIp.Set(pIp[0],pIp[1],pIp[2],pIp[3]);
  return SNMP_NO_ERROR;
}

SNMP_STATUS EventLog::SetLogServers (const BcmIpAddressList &logServers)
{
  // Copy the list.
  fLogServers = logServers;
  
  // Replace docsDevEvSyslog with logServerIpAddress if docsDevEvSyslog
  // was previously 0.0.0.0.  Otherwise, leave it alone.
  if (fSyslogIp == kAllZerosIp)
    fSyslogIp = fLogServers[0];

  return SNMP_NO_ERROR;
}

SNMP_STATUS EventLog::SetThrottleAdminStatus (int Admin)
{
  return fBucket.SetAdminStatus ((ThrottleAdminStatus) Admin);
}

SNMP_STATUS EventLog::SetThrottleThreshold (int Threshold)
{
  return fBucket.SetThreshold (Threshold);
}

SNMP_STATUS EventLog::SetThrottleInterval (int Interval)
{
  return fBucket.SetInterval (Interval);
}

SNMP_STATUS EventLog::SetLevelControl (int Level, unsigned short Control)
{
  // Make sure some wisecrack isn't trying to set control for a bogus level
  if (Level > EV_SEVERITY_LEVELS)
    return SNMP_WRONG_VALUE;
    
#if (kDefaultValue_EventLogDynamicLevelReporting)  
  unsigned short OldControl = fLevelControl[Level-1];
#endif

  fLevelControl[Level-1] = Control;

#if (kDefaultValue_EventLogDynamicLevelReporting)  
  if (((OldControl & (kLocalNonVol | kLocalVolatile)) == 0)
  &&  ((Control & (kLocalNonVol | kLocalVolatile)) != 0))
    InitFromNonVol();
  else if (((OldControl & (kLocalNonVol | kLocalVolatile)) != 0)
  &&  ((Control & (kLocalNonVol | kLocalVolatile)) == 0))
    InitFromNonVol();
#endif
  
  return SNMP_NO_ERROR;
}

int EventLog::GetLogControl () const
{
  return fControl;
}

unsigned short EventLog::GetTrapControl () const
{
  return fTrapControl;
}

ostream &EventLog::PrintHeader(ostream &outStream) const
{
  BcmString Header = 
"-------------------------------------------------------------------------------\n";
  Header +=
"    ";
  Header += fpName;
  Header += "\n"
"----+-----------------+----------+-----+---------------------------------------\n"
"Inx |    Date/Time    | ID (hex) | Lvl | Text\n"
"----+-----------------+----------+-----+---------------------------------------\n";

  outStream << Header;
  
  return outStream;
}

ostream &EventLog::PrintControl (ostream &outStream) const
{
  int Loop, Reporting;
  unsigned char ReportingBits;
  BcmVarBind VB;
  char ReportingString[8];
  const char Levels[8][14]={ "(Emergency)  ",
                             "(Alert)      ",
                             "(Critical)   ",
                             "(Error)      ",
                             "(Warning)    ",
                             "(Notice)     ",
                             "(Information)",
                             "(Debug)      "};
                          
  outStream << "-------------------------------------------------------------------------------\n"
                "                              Event Control Table\n"
                "----------------+--------------------------------------------------------------\n"
                "    Priority    |                        Reporting\n"
#if (SNMP_SUPPORT)              
                "     Level      |  0x80=LocalNonVol 0x40=Traps 0x20=Syslog 0x10=LocalVolatile\n"
#else
                "     Level      |  0x01=LocalNonVol 0x02=Traps 0x04=Syslog 0x08=LocalVolatile\n"
#endif
                "----------------+--------------------------------------------------------------" << endl;
  
  for (Loop = 0; Loop < 8; Loop++)
  {
    Reporting = GetLevelControl(Loop+1);
#if (SNMP_SUPPORT)              
    VB.SetBits (Reporting, 1);
    VB.GetString (&ReportingBits, 1);
#else
    ReportingBits = Reporting;
#endif
    sprintf (ReportingString, "0x%02X", ReportingBits);
    
    outStream << Loop+1 << " " << Levels[Loop] << " | " << ReportingString << ": ";
    
    if (Reporting == 0)
      outStream << "No-Logging ";
    if (Reporting & EventLog::kLocalNonVol)
      outStream << "LocalNonVol ";
    if (Reporting & EventLog::kTraps)
      outStream << "Traps ";
    if (Reporting & EventLog::kSyslog)
      outStream << "Syslog ";
    if (Reporting & EventLog::kLocalVolatile)
      outStream << "LocalVolatile ";
    
    outStream  << endl;
  }  
  
  return outStream;
}

ostream &EventLog::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)
/*
#if (SNMP_SUPPORT)  
  outStream << "SNMP agent: " << fpAgent->GetName() << endl;
  outStream << "docsDevEventTable = " << fpEventTable << endl;
#endif
*/

  // Debug info
  SnmpLogEv << "Debug:\n"
            << "------" << endl;
  SnmpLogEv << "  Queued messages: " << fpThread->pfEventLogQueue->NumberOfMessages() << endl;
  SnmpLogEv << "  Log Servers from DHCP: " << fLogServers << endl;
  SnmpLogEv << endl;
  
  // Complie time options
  outStream << "Build Options:\n"
            << "--------------" << endl;
            
#if (kDefaultValue_EventLogDynamicLevelReporting)
  outStream << "  Level reporting: dynamic" << endl;
#else
  outStream << "  Level reporting: static" << endl;
#endif

#if (kDefaultValue_EventLogOptimizeForSmallerSize)
  outStream << "  Optimization: smaller size" << endl;
#else
  outStream << "  Optimization: readability" << endl;
#endif

#if (kDefaultValue_EventLogLocalVolatileEqualsLocalNonVol)
  outStream << "  NonVol: localVolatile and localNonVol" << endl;
#else
  outStream << "  NonVol: localNonVol only" << endl;
#endif

  outStream << endl;

  // Throttling info
  outStream << "Throttling:\n"
            << "-----------" << endl;
  if (GetThrottleAdminStatus() == kUnconstrained)
    outStream << "  ThrottleAdminStatus: unconstrained(1)"  << endl;
  else if (GetThrottleAdminStatus() == kMaintainBelowThreshold)
    outStream << "  ThrottleAdminStatus: maintainBelowThreshold(2)"  << endl;
  else if (GetThrottleAdminStatus() == kStopAtThreshold)
    outStream << "  ThrottleAdminStatus: stopAtThreshold(3)"  << endl;
  else if (GetThrottleAdminStatus() == kInhibited)
    outStream << "  ThrottleAdminStatus: inhibited(4)"  << endl;
    
  // cast away const warning on IsThrottled
  outStream << "  Currently throttled: " << ((EventLog *)this)->IsThrottled() << endl;
  
  outStream << endl;
  
  if ((GetThrottleAdminStatus() == kMaintainBelowThreshold)
  ||  (GetThrottleAdminStatus() == kStopAtThreshold))
    outStream << "Current throttle state: " << fBucket.PeekContents() << " / " << fBucket.GetThreshold() << " remaining this " << fBucket.GetIntervalRemaining() << " / " << fBucket.GetInterval() << " second(s)." << endl;
  
  // Print the header
  PrintHeader(outStream);

#if (SNMP_SUPPORT)  
  fpAgent->LockMib();
#endif
  
  // Print all events
  EventLogEntry *pEvent = FirstEvent();
  while (pEvent != NULL)
  {
    outStream << *pEvent << endl;
    pEvent = NextEvent(pEvent);
  }

#if (SNMP_SUPPORT)    
  fpAgent->UnlockMib();
#endif
#endif
  
  return outStream;
}                                    


SNMP_STATUS EventLog::InitFromNonVol ()
{
#if (SNMP_SUPPORT)
  if (fpEventTable == NULL)
  {
    fpEventTable = (docsDevEventTable *)fpAgent->FindTable (BcmString("docsDevEventTable"));
    if (fpEventTable == NULL)
    {
      gLogMessageRaw << "  WARNING: can't initialize event log from NV, docsDevEvent table not found!" << endl;
      return SNMP_RESOURCE_UNAVAILABLE;
    }
  }
#endif
  
  return fpThread->InitFromNonVol (this);
}


// Resolve any embedded objects in the raw text string.
int EventLog::DecodeEmbeddedObjects (const BcmString &RawText, BcmString &DecodedText)
{
  int NumDecoded = 0;
  
  SnmpLogEv << "EventLogEntry::DecodeEmbeddedObjects in " << RawText << endl;

#if (SNMP_SUPPORT)
  BcmMibObject *pObject;
  int ObjStartIx, ObjEndIx, PrevIx = 0, CurIx = 0;
  BcmString ObjName, TempName, TextSegment;
  BcmVarBind VB;

  // Scan for objects embedded in the text string, indicated by <objectName>
  ObjStartIx = RawText.find_first_of ('<', CurIx);
  ObjEndIx = RawText.find_first_of ('>', ObjStartIx);

  // Initialize formatted text:
  DecodedText = "";
  
  fpAgent->LockMib();
  
  // If we have any embedded objects, deal with them.
  while ((ObjStartIx != -1) && (ObjEndIx != -1))
  {
    TextSegment = RawText.substr (PrevIx, ObjStartIx-PrevIx);

    ObjName = RawText.substr (ObjStartIx+1, (ObjEndIx-ObjStartIx-1));
    SnmpLogEv << "Found embedded object '" << ObjName << "'" << endl;

    // Attempt to find the named object and splat it out into our text string.
    pObject = fpAgent->FindObject (ObjName);
    if (pObject == NULL)
    {
      /*
      // Note: doing the following produces weird warnings under psos:
      // ObjName = "<" + ObjName + ">"
      // so take an extra step:
      
      TempName = "<"; 
      TempName += ObjName;
      TempName += ">";
      
      ObjName = TempName;
      */
      
      // If the decode fails, don't insert <objectName> as above, just leave it out.
      ObjName = "";
    }
    else
    {
      SNMP_STATUS Got = pObject->Get(VB);
      
      // We'll accept noAccess return values.  Certain 'not-accessible'
      // objects can still be retrieved programatically (ie docsIfCmServiceId)
      if ((Got != SNMP_NO_ERROR)
      &&  (Got != SNMP_NO_ACCESS))
      {
        /*
        // Note: doing the following produces weird warnings under psos:
        // ObjName = "<" + ObjName + ">"
        // so take an extra step:
      
        TempName = "<";
        TempName += ObjName;
        TempName += ">";
        
        ObjName = TempName;
        */
        
        // If the decode fails, don't insert <objectName> as above, just leave it out.
        ObjName = "";
      }
      else
      { 
        NumDecoded++;
        VB.AsString(ObjName);
      }
    }

    DecodedText += TextSegment;
    DecodedText += ObjName;

    CurIx = ObjEndIx;
    PrevIx = CurIx + 1;

    ObjStartIx = RawText.find_first_of ('<', CurIx);
    ObjEndIx = RawText.find_first_of ('>', ObjStartIx);
  }
  
  fpAgent->UnlockMib();

  // Handle any text after the last embedded object. If we have no
  // embedded objects, this will handle the whole string.
  TextSegment = RawText.substr (PrevIx, RawText.length()-PrevIx);
  DecodedText += TextSegment;
#else
  DecodedText = RawText;
#endif

  SnmpLogEv << "  RawText = " << RawText << "\n"
            << "  DecodedText = " << DecodedText << endl;

  return NumDecoded;
}

SNMP_STATUS EventLog::StoreEventToNonVol (EventLogEntry *pEvent)
{
  if (fpSettings == NULL)
  {
    SnmpLogErr << "EventLog::StoreToNonVol failed, couldn't get settings instance." << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }
  
  if (ShouldStoreEventToNonVol (pEvent) == false)
    return SNMP_NO_ERROR;
    
  EventLogNvRec EvRec;
  bool NewText = false, NewEvent = true;
  int EvIx;
  
  EvRec.Id = 0;
  EvRec.Count = 0;
  EvRec.FirstTime = 0;
  EvRec.LastTime = 0;
  
  SnmpLogEv << "  Event " << pEvent->GetIndex() << " has NV index " << pEvent->fNvIndex << endl;
  
  // If this event is already in non-vol then the entry will have a non-negative
  // fNvIndex member that indicates which NV entry it's associated with.  Update
  // that entry.
  if (pEvent->fNvIndex != -1)
  {
    NewEvent = false;

    // Do a little sanity checking here.  ID & first time of the event
    // should be the same as what's already there.
    if ((fpSettings->Event(pEvent->fNvIndex).Id != (uint32) pEvent->GetId())
    ||  (fpSettings->Event(pEvent->fNvIndex).FirstTime != pEvent->GetFirstTime()))
      SnmpLogRaw << "WARNING: CM event log ID mismatch on NV index " << pEvent->fNvIndex << endl;

    EvIx = pEvent->fNvIndex;
  }
  
  // If the fNvIndex is -1, that means this is a brand new entry and we need
  // to add it to NV.  Is there a slot available?
  else if (fpSettings->NextAvailableEvent() != -1)
  {
    EvIx = fpSettings->NextAvailableEvent();
    SnmpLogEv << "  Using next available NV index " << EvIx << endl;
  }
    
  // This means NV is full.  We must scoot all events back a notch and put
  // this new one at the end.  So that means we must also update the
  // fNvIndex of all our existing events.
  else
  {
    int Loop;
    
    SnmpLogEv << "  NV is full, backing up events and putting at the end." << endl;
    
    for (Loop = 0; Loop < fpSettings->Capacity()-1; Loop++)
    {
      fpSettings->Event(Loop, fpSettings->Event(Loop+1));
      fpSettings->EventText(Loop, fpSettings->EventText(Loop+1));
    }
    
    EvIx = Loop;
    
    EventLogEntry *pLoopEv = FirstEvent();
    while (pLoopEv)
    {
      if (pLoopEv->fNvIndex >= 0)
        pLoopEv->fNvIndex--;
        
      pLoopEv = NextEvent(pLoopEv);
    }
  }
  
  EvRec.Id = pEvent->GetId();
  EvRec.Count = pEvent->GetCount();
  EvRec.FirstTime = pEvent->GetFirstTime();
  EvRec.LastTime = pEvent->GetLastTime();
  pEvent->GetText().Get(EvRec.Text);
  
  // Has the text for this event changed?  We might need to know this if
  // the event's NV policy is to store immediately if the text has changed.
  if (fpSettings->EventText (EvIx) != EvRec.Text)
    NewText = true;
  
  // Store to NV object.  Note that this does not actually write NV to
  // flash, that will come later if it's time.
  fpSettings->Event (EvIx, EvRec);
  
  // Associate this event with this NV entry.
  pEvent->fNvIndex = EvIx;
  
  // Now, should we write NV?
  if (ShouldWriteNonVolNow (pEvent, NewEvent, NewText))
    return WriteNonVolSettings();
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS EventLog::ClearNonVol ()
{
  if (fpSettings)
  {
    fpSettings->ClearAllEvents();
  }
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS EventLog::InitFromNonVolImpl ()
{
  EventLogEntry *pEvent;
  int Loop;
  //time_t AlertStatusTime = 0;
  bool OldAsyncs = fAsyncsInhibited;
 
 
  gLogMessageRaw << fpName << " initializing with stored events." << endl;
 
  if (fpSettings == NULL)
  {
    SnmpLogErr << "  InitFromNonvol failed, couldn't get settings instance." << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }

#if (SNMP_SUPPORT)  
  if (fpAgent)
    fpAgent->LockMib();
#endif
  
  // Turn off traps/syslogs so we don't send them out when just loading
  // events from storage;
  fAsyncsInhibited = true;

#if (!kDefaultValue_EventLogDynamicLevelReporting)  
  // If we are using static level reporting we must  first set all reporting 
  // levels to kLocalVolatile.  This means the log at startup will have whatever
  // is in NV, regardless of reporting levels.
  for (Loop = 1; Loop <= EV_SEVERITY_LEVELS; Loop++)
    SetLevelControl (Loop, kLocalVolatile);
#endif
  
  // OK, here we go again.  We can't just flush the table and re-create it
  // from NV, because this method gets called not only at start time but
  // also when docsDevEvReporting changes.  Problem with that is, what if
  // we already have entries in the table but they are NOT in NV, that is
  // docsDevEvReporting for that level is localVolatile, not localNonVol.
  // In that case, we would get rid of entries that ought to still be there.
  // Furthermore, all the indices will get screwed around and start over at
  // 1 even if the log should have much larger indices than that.  OK, so
  // what we'll do here is go through NV and for any entry in NV that is NOT
  // in the event log, we'll go ahead and log it.  This means that any
  // events stored to NV that are being added will wind up at the end of
  // the log, but so be it.  It also means that if we turn off local reporting
  // for a given event, it will not be removed from the table immediately,
  // but so be that, too.
    
  for (Loop = 0; Loop < fpSettings->Capacity(); Loop++)
  {
    SnmpLogEv << "NV event " << Loop << endl;
    if (fpSettings->Event(Loop).Id != 0)
    {
      // Is there already an event with this as its NV index?  If so,
      // skip it.
      if (FindEventFromNvIndex(Loop) != NULL)
        continue;
      
      SnmpLogEv << "  Logging for ID " << fpSettings->Event(Loop).Id << endl;
      pEvent = LogEventImpl ((EventLogIdCode)fpSettings->Event(Loop).Id, kUseDefaultLevel, NULL, kCreateNew, false);
      if (pEvent)
      {
        pEvent->fCount = fpSettings->Event(Loop).Count;
        pEvent->fFirstTime = fpSettings->Event(Loop).FirstTime;
        pEvent->fLastTime = fpSettings->Event(Loop).LastTime;
        pEvent->fNvIndex = Loop;

        // If the event has text associated with it in NV, then use that
        // text in favor of what the default text is.        
        if (fpSettings->EventText(Loop).BytesUsed() != 0)
          pEvent->fEvText = fpSettings->EventText(Loop);
        
        // If not, then write the event text to NV.  Actually, this won't
        // write to NV, just store in RAM so next time NV gets written this
        // event will be filled in.
        else
        {
          BcmOctetBuffer EvText;
          pEvent->GetText().Get(EvText);
          
          fpSettings->EventText(Loop, EvText);
        }
        
        //if (pEvent->GetLevel() == kAlert)
          //UpdateCmStatusCode (pEvent, AlertStatusCode, &AlertStatusTime);
      }
      else
        SnmpLogEv << "Didn't recognize ID, no event logged." << endl;
    }
    else
      SnmpLogEv << "ID = 0, not logging." << endl;
  }
  
  SnmpLogRaw << "Event log initialization complete." << endl;

#if (!kDefaultValue_EventLogDynamicLevelReporting)  
  // Back to default reporting.
  UseDefaultReporting();
#endif
  
  fAsyncsInhibited = OldAsyncs;

#if (SNMP_SUPPORT)    
  if (fpAgent)
    fpAgent->UnlockMib();
#endif
  
  /*\
   *  How should we initialize docsIfCmStatusCode?  Well, here's what section
   *  4.4.2.3 of OSS-I-05 says:
   *  
   *  "Alert event (priority 2)
   *   A serious failure, which causes reporting system to reboot but it is not 
   *   caused by h/w or s/w malfunctioning. After recovering from the critical 
   *   event system MUST send the cold/warm start notification. Alert event 
   *   could not be reported as a Trap or SYSLOG message and MUST be stored in 
   *   the internal log file. The code of this event MUST be saved in 
   *   non-volatile memory and reported later through docsIfCmStatusCode SNMP 
   *   variable [RFC-2670].
   *   
   *   Critical event (priority 3)
   *   A serious failure that requires attention and prevents the device from 
   *   transmitting data but could be recovered without rebooting the system. 
   *   After recovering from the error event Cable Modem Device MUST send the 
   *   Link Up notification. Critical events could not be reported as a Trap or 
   *   SYSLOG message and MUST be stored in the internal log file. The code of 
   *   this event MUST be reported later through docsIfCmStatusCode SNMP 
   *   variable [RFC-2670]. The examples of such events could be configuration
   *   file problems detected by the modem or inability to get IP address from 
   *   DHCP."
   *  
   *  I suppose this means that after reboot docsIfCmStatusCode should be the
   *  value of the last Alert priority event logged, otherwise empty.  And
   *  then it can be overwritten at runtime by a new Critical event or a new
   *  Alert event.  But if it's overwritten by a Critical event, then after
   *  reset we should still show the old Alert event code.  This is a bit
   *  unclear, but this is my best guess.
  \*/
  
  //memset (fCmStatusCode, 0, sizeof(fCmStatusCode));
  //strncpy (fCmStatusCode, AlertStatusCode, sizeof (fCmStatusCode)-1);
  //fCmStatusCodeTime = AlertStatusTime; 
  
  return SNMP_NO_ERROR;
}


SNMP_STATUS EventLog::WriteNonVolSettingsImpl ()
{
  BcmNonVolDevice *pNonVolDevice = BcmNonVolDevice::GetSingletonInstance();;
  SNMP_STATUS Status;
  bool result;
  
          
  if (fpSettings == NULL)
  {
    gLogMessageRaw << "No SnmpNonVolSettings object is present; can't write settings to the device!" << endl;
    return SNMP_RESOURCE_UNAVAILABLE;
  }

  if (pNonVolDevice == NULL)
  {
      gLogMessageRaw << "No Non-Vol Device is present; can't write settings to the device!" << endl;
      return SNMP_RESOURCE_UNAVAILABLE;
  }
  
  result = pNonVolDevice->Write(*fpSettings);

  if (result == false)
  {
      gLogMessageRaw << "ERROR - failed to write Non-Vol Settings to the device!" << endl;
      Status = SNMP_GEN_ERR;
  }
  else
  {
      gLogMessageRaw << "Non-Vol Settings successfully written to the device." << endl;
      Status = SNMP_NO_ERROR;
  }
  
  return Status;
}

int EventLog::ReplaceEmbeddedObjects (const BcmString &RawText, BcmString &EvText, const char *pParam1, const char *pParam2, const char *pParam3, const char *pParam4, const char *pParam5)
{
  int NumReplaced = 0;
  int ObjStartIx, ObjEndIx, ParamNum = 1, PrevIx = 0, CurIx = 0;
  BcmString Param, TempName, TextSegment;
  
  SnmpLogEv << "ReplaceEmbeddedObjects: RawText='" << RawText << "'" << endl;

  // Scan for objects embedded in the text string, indicated by <objectName>
  ObjStartIx = RawText.find_first_of ('<', CurIx);
  ObjEndIx = RawText.find_first_of ('>', ObjStartIx);

  // Initialize formatted text:
  EvText = "";

  // If we have any embedded objects, deal with them.
  while ((ObjStartIx != -1) && (ObjEndIx != -1))
  {
    NumReplaced++;

    TextSegment = RawText.substr (PrevIx, ObjStartIx-PrevIx);
    EvText += TextSegment;

    Param = RawText.substr (ObjStartIx+1, (ObjEndIx-ObjStartIx-1));
    SnmpLogEv << "Found parameter '" << Param << "'" << endl;

    if (ParamNum == 1)
    {
      if (pParam1)
        EvText += pParam1;
      /*
      else // leave the encoded parameter the way it was
      {
        EvText += "<";
        EvText += Param;
        EvText += ">";
      }
      */
    }
    else if (ParamNum == 2)
    {
      if (pParam2)
        EvText += pParam2;
      /*
      else // leave the encoded parameter the way it was
      {
        EvText += "<";
        EvText += Param;
        EvText += ">";
      }
      */
    }
    else if (ParamNum == 3)
    {
      if (pParam3)
        EvText += pParam3;
      /*
      else // leave the encoded parameter the way it was
      {
        EvText += "<";
        EvText += Param;
        EvText += ">";
      }
      */
    }
    else if (ParamNum == 4)
    {
      if (pParam4)
        EvText += pParam4;
      /*
      else // leave the encoded parameter the way it was
      {
        EvText += "<";
        EvText += Param;
        EvText += ">";
      }
      */
    }
    else if (ParamNum == 5)
    {
      if (pParam5)
        EvText += pParam5;
      /*
      else // leave the encoded parameter the way it was
      {
        EvText += "<";
        EvText += Param;
        EvText += ">";
      }
      */
    }
    else
      break;

    ParamNum++;

    CurIx = ObjEndIx;
    PrevIx = CurIx + 1;

    ObjStartIx = RawText.find_first_of ('<', CurIx);
    ObjEndIx = RawText.find_first_of ('>', ObjStartIx);
  }

  // Handle any text after the last embedded object. If we have no
  // embedded objects, this will handle the whole string.
  TextSegment = RawText.substr (PrevIx, RawText.length()-PrevIx);
  EvText += TextSegment;
  
  SnmpLogEv << "ReplacedText='" << EvText << "'" << endl;

  return NumReplaced;
}


BcmTrap *EventLog::GetNewTrap (const EventLogEntry *pEvent, DocsDevTrapTypeHint EvTrap)
{
#if (SNMP_SUPPORT)
  BcmTrap *pTrap = NULL;
  
  SnmpLogEv << "EventLog::GetNewTrap - creating trap" << endl;
    
  switch (EvTrap)
  {
    // Standard trap types:
    case kColdStartTrap:
      pTrap = new coldStartTrap (fpAgent);
      break;
      
    case kWarmStartTrap:
      pTrap = new warmStartTrap (fpAgent);
      break;
      
    case kLinkDownTrap:
      // What interface should I specify?  It's academic, really,
      // since in fact no DOCSIS events specify linkDown as their trap
      // type.  So just use interface 0.
      pTrap = new linkDownTrap (fpAgent, 0);
      break;
      
    case kLinkUpTrap:
      // What interface should I specify?  It's academic, really,
      // since in fact no DOCSIS events specify linkDown as their trap
      // type.  So just use interface 0.
      pTrap = new linkUpTrap (fpAgent, 0);
      break;
      
    case kSnmpAuthFailureTrap:
      pTrap = new authFailureTrap (fpAgent);
      break;
    
    // These aren't supported for now.
    case kEgpNeighborLossTrap:
    case kEnterpriseTrap:
    default:
      SnmpLogEv << "Unsupported trap type " << (int)EvTrap << endl;
      pTrap = NULL;
      break;
  }
  
  return pTrap;
#else
  return NULL;
#endif
}


/******************************************************************************\
**                                                                            **
*  EventLogEntry class                                                         *
*  One instance of this class exists for each entry in the event log.  Handles *
*  message formatting, async messages, etc.                                    *
**                                                                            **
\******************************************************************************/

EventLogEntry::EventLogEntry (int Index, int Id, int Level, const BcmString &Text, const BcmString &SyslogAppend, DocsDevTrapTypeHint TrapType, EventLog *pLog)
{
  fpEventLog = pLog;
  fIndex = Index;
  fId = Id;
  fLevel = Level;
  fRawEvText = Text;
  fRawSyslogText = SyslogAppend;
  fTrapType = TrapType;
  fCount = 0;
  fNvIndex = -1;
  
  time (&fFirstTime);
  fLastTime = 0;
  
  fpSyslog = NULL;
  fpTrap = NULL;
  fNonVolPolicy = kDefaultNvBehavior;
  
  fTrapsPending = 0;
  fSyslogsPending = 0;

  // Gotta call this explicitly now due to the way 1.1 event logging works.
  // The entry must be in the table prior to sending traps, and we aren't in
  // the table yet.
  // Log();
}

EventLogEntry::~EventLogEntry ()
{
  if (fpSyslog)
    delete fpSyslog;
    
  if (fpTrap)
    delete fpTrap;
}

int EventLogEntry::GetIndex() const
{
  return fIndex;
}

time_t EventLogEntry::GetFirstTime () const
{
  return fFirstTime;
}

time_t EventLogEntry::GetLastTime () const
{
  return fLastTime;
}

unsigned long EventLogEntry::GetCount () const
{
  return fCount;
}

int EventLogEntry::GetLevel () const
{
  return fLevel;
}

int EventLogEntry::GetId () const
{
  return fId;
}

const BcmString &EventLogEntry::GetText () const
{
  return fEvText;
}

const BcmString &EventLogEntry::GetSyslogText ()
{
  BcmString SyslogAppend;

  // DO decode the event log text.  This is because the only time
  // this method gets called is right when we're about to shoot off a syslog,
  // and we do want the latest decoded values at that time.
  if ((fpEventLog->DecodeEmbeddedObjects (fRawSyslogText, SyslogAppend) > 0)
  ||  (fSyslogText.empty() == true))
  {
    fSyslogText = fEvText;
    fSyslogText += " ";
    fSyslogText += SyslogAppend;
  }
  
  return fSyslogText;
}

NVPolicy EventLogEntry::NonVolPolicy() const
{
  return fNonVolPolicy;
}

SNMP_STATUS EventLogEntry::Log (bool PrintToConsole)
{
  int Syslogs, Traps;
  
  
  fCount++;
  time (&fLastTime);

  // Resolve any embedded objects in the text string
  fpEventLog->DecodeEmbeddedObjects (fRawEvText, fEvText);
  
  // Print the event to the console.
  if ((PrintToConsole == true) && (EventLog::fSilent == false))
    gLogMessageRaw << "Logging event: " << GetText() << endl;

  if (fpEventLog->IsThrottled() == true)
  {
    SnmpLogEv << "EventLogEntry::Log - async msgs throttled, no traps or syslogs." << endl;
    return SNMP_EVENT_LOG_THROTTLED;
  }

#if (SNMP_SUPPORT)  
  // PR 5883: Lock the MIB here.  Avoid a situation where our agent is not
  // enabled when we send the syslog message (thus deferring it), but becomes
  // enabled prior to sending traps (thus they are sent immediately).  This
  // situation will result in a miscount for throttling purposes because the
  // traps will be counted immediately, then the syslog message will be counted
  // again later.
  if (fpEventLog->fpAgent)
    fpEventLog->fpAgent->LockMib();
#endif
  
  // Send any async messages as required
  Syslogs = SendSyslogMsg (false, PrintToConsole);
  Traps = SendTraps (false, PrintToConsole);
  
  if ((Traps > 0) || (Syslogs > 0))
  {
    if (Syslogs > Traps)
      fpEventLog->SentAsyncMsg(this, Syslogs);
    else
      fpEventLog->SentAsyncMsg(this, Traps);
  }
  
#if (SNMP_SUPPORT)  
  if (fpEventLog->fpAgent)
    fpEventLog->fpAgent->UnlockMib();
#endif
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS EventLogEntry::Log (const char *pParam1, const char *pParam2, const char *pParam3, const char *pParam4, const char *pParam5, bool PrintToConsole)
{
  int Syslogs, Traps, NumUsed, NumReplaced=0;
  
  
  fCount++;
  time (&fLastTime);

  // Resolve any embedded objects in the text string
  NumUsed = fpEventLog->ReplaceEmbeddedObjects (fRawEvText, fEvText, pParam1, pParam2, pParam3, pParam4, pParam5);
  
  // Print the event to the console.
  if ((PrintToConsole == true) && (EventLog::fSilent == false))
    gLogMessageRaw << "Logging event: " << GetText() << endl;

  if (fpEventLog->IsThrottled() == true)
  {
    SnmpLogEv << "EventLogEntry::Log - async msgs throttled, no traps or syslogs." << endl;
    return SNMP_EVENT_LOG_THROTTLED;
  }
  
  // Somewhat awkward here, but it works.  The point of this is to put any args
  // leftover after replacing params in the event log text into the syslog text.
  // 
  // 8/30/2002: note that the code below will result in fSyslogText being
  // properly formatted with passed in parmeters, but it winds up still not
  // working because docsDevSyslog::FormatMsg calls EventLogEntry::GetSyslogText
  // which calls DecodeEmbeddedObjects and overwrites this stuff.  For now,
  // I am not going to fix it because we don't have any syslog messages that
  // need this functionality anyway.  But one of these days, need a way in
  // GetSyslogText to know that we have already done a replace and we should
  // not decode...
  if (fRawSyslogText.empty() == false)
  {
    BcmString SyslogAppend;
    switch (NumUsed)
    {
      case 0:
        NumReplaced = fpEventLog->ReplaceEmbeddedObjects (fRawSyslogText, SyslogAppend, pParam1, pParam2, pParam3, pParam4, pParam5);
        break;

      case 1:
        NumReplaced = fpEventLog->ReplaceEmbeddedObjects (fRawSyslogText, SyslogAppend, pParam2, pParam3, pParam4, pParam5, NULL);
        break;

      case 2:
        NumReplaced = fpEventLog->ReplaceEmbeddedObjects (fRawSyslogText, SyslogAppend, pParam3, pParam4, pParam5, NULL, NULL);
        break;

      case 3:
        NumReplaced = fpEventLog->ReplaceEmbeddedObjects (fRawSyslogText, SyslogAppend, pParam4, pParam5, NULL, NULL, NULL);
        break;
      
     case 4:
        NumReplaced = fpEventLog->ReplaceEmbeddedObjects (fRawSyslogText, SyslogAppend, pParam5, NULL, NULL, NULL, NULL);
        break;
    }
    
    fSyslogText = fEvText;
    fSyslogText += " ";
    fSyslogText += SyslogAppend;
  }

  /* Send any async messages as required. */
  
  // Send syslog. If we have a syslog object already, delete it.  That way we'll be sure
  // to update the text with the appended info from above.
  if (fpSyslog)
  {
    delete fpSyslog;
    fpSyslog = NULL;
  }
  
#if (SNMP_SUPPORT)  
  // PR 5883: Lock the MIB here.  Avoid a situation where our agent is not
  // enabled when we send the syslog message (thus deferring it), but becomes
  // enabled prior to sending traps (thus they are sent immediately).  This
  // situation will result in a miscount for throttling purposes because the
  // traps will be counted immediately, then the syslog message will be counted
  // again later.
  if (fpEventLog->fpAgent)
    fpEventLog->fpAgent->LockMib();
#endif
  
  Syslogs = SendSyslogMsg (false, PrintToConsole);
  Traps = SendTraps (false, PrintToConsole);
  
  if ((Traps > 0) || (Syslogs > 0))
  {
    if (Syslogs > Traps)
      fpEventLog->SentAsyncMsg(this, Syslogs);
    else
      fpEventLog->SentAsyncMsg(this, Traps);
  }   
  
#if (SNMP_SUPPORT)  
  if (fpEventLog->fpAgent)
    fpEventLog->fpAgent->UnlockMib();
#endif
  
  return SNMP_NO_ERROR;
}


int EventLogEntry::SendTraps (bool ExistingOnly, bool PrintToConsole)
{ 
  int NumTraps = 0;
  
#if (SNMP_SUPPORT)
  if (!(fpEventLog->GetLevelControl (fLevel) & EventLog::kTraps))
  {
    SnmpLogEv << "EventLogEntry::SendTraps - traps not enabled for level " << fLevel << endl;
    return 0;
  }
  
  if (fpEventLog->IsThrottled() == true)
    return 0;
    
  if (fpEventLog->fpAgent)
    fpEventLog->fpAgent->LockMib();
  
  // Create the kind of trap associated with this event.
  if (ExistingOnly == false)
  {
    if (fpTrap == NULL)
      fpTrap = fpEventLog->GetNewTrap (this, fTrapType);
      
    fTrapsPending++;
  }
  
  if (fpTrap == NULL)
  {
    if (fpEventLog->fpAgent)
      fpEventLog->fpAgent->UnlockMib();
      
    return 0;
  }
    
  // Send the trap.  Note that for traps which were deferred until after
  // registration, we might have to send it multiple times if the same
  // event was logged multiple times, thus incrementing the count and
  // timestamps only.  So we must loop here.
  NumTraps = fTrapsPending;
  while (fTrapsPending)
  {
    // If SendTrap() returns -1, that means that the CM has not
    // completed its initialization (ie registration, DH calculations, etc).
    // Don't delete the trap in that case, it'll get sent later in 
    // SendDeferredAsyncs().
    if (fpEventLog->fpAgent->SendTrap (fpTrap) == -1)
    {
      NumTraps = -1;
      break;
    }
    
    fTrapsPending--;
  }
  
  if (NumTraps != -1)
  {
    delete fpTrap;
    fpTrap = NULL;
  }
  else
    NumTraps = 0;
    
  if (fpEventLog->fpAgent)
    fpEventLog->fpAgent->UnlockMib();
    
#endif

  return NumTraps;
}

int EventLogEntry::SendSyslogMsg (bool ExistingOnly, bool PrintToConsole)
{
  if (!(fpEventLog->GetLevelControl (fLevel) & EventLog::kSyslog))
  {
    SnmpLogEv << "EventLogEntry::SendSyslogMsg - syslog not enabled for level " << fLevel << endl;
    return 0;
  }
  
  int NumLogs = 0;

#if (SNMP_SUPPORT)
  if (fpEventLog->fpAgent)
    fpEventLog->fpAgent->LockMib();
  
  if (ExistingOnly == false)
  {
    if (fpSyslog == NULL)
    {
      SnmpLogEv << "EventLogEntry::SendSyslogMsg - creating syslog msg " << fLevel << endl;
      //fpSyslog = new docsDevSyslog (this, fpEventLog->fpAgent->GetIfNum());
      fpSyslog = fpEventLog->GetNewSyslog (this, fpEventLog->fpAgent->GetIfNum());
    }
    
    fSyslogsPending++;
  }
    
  if (fpSyslog == NULL)
  {
    SnmpLogEv << "EventLog returned NULL for syslog, not sending." << endl;
    
    if (fpEventLog->fpAgent)
      fpEventLog->fpAgent->UnlockMib();
      
    return 0;
  }
    
  // Send the syslog.  Note that for syslogs which were deferred until after
  // registration, we might have to send it multiple times if the same
  // event was logged multiple times, thus incrementing the count and
  // timestamps only.  So we must loop here.
  NumLogs = fSyslogsPending;
  while (fSyslogsPending)
  {
    // If SendSyslog() returns -1, that means that the CM has not
    // completed its initialization (ie registration, DH calculations, etc).
    // Don't delete the syslog message in that case, it'll get sent later in 
    // SendDeferredAsyncs().
    if (fpEventLog->SendSyslog (fpSyslog, PrintToConsole) == -1)
    {
      NumLogs = -1;
      break;
    }
  
    fSyslogsPending--;
  }
  
  // If SendSyslog() returns -1, that means that the CM has not
  // completed its initialization (ie registration, DH calculations, etc).
  // Don't delete the syslog message in that case, it'll get sent later in 
  // SendDeferredAsyncs().
  if (NumLogs != -1)
  {
    delete fpSyslog;
    fpSyslog = NULL;
  }
  
  if (fpEventLog->fpAgent)
    fpEventLog->fpAgent->UnlockMib();
#endif  

  return NumLogs;
}

ostream &EventLogEntry::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)
/*
"-------------------------------------------------------------------------------\n"
"                                   Event Log\n"
"----+-----------------+----------+-----+---------------------------------------\n"
"Inx |    Date/Time    | ID (hex) | Lvl | Text\n"
"----+-----------------+----------+-----+---------------------------------------\n";
*/
  char *TimeStr = ctime (&fLastTime);
  char  PrintStr[96];
  char  TextStr[45];
  
  TimeStr[19] = 0;
  TimeStr += 4;
  
  memset (TextStr, 0, sizeof(TextStr));
  
  if (fEvText.length() <= 38)
    strncpy (TextStr, fEvText.c_str(), 38);
  else
  {
    strncpy (TextStr, fEvText.c_str(), 35);
    strcat (TextStr, "...");
  }

  // Need some formatting not supported by streams here, so sprintf it into
  // a string and then stream that sucker out...
  sprintf (PrintStr, "%03d | %s | %08X |  %d  | %s", 
    fIndex, TimeStr, fId, fLevel, TextStr);

  outStream << PrintStr;
#endif
  
  return outStream;
}


