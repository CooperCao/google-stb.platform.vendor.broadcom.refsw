//****************************************************************************
//
// Copyright (c) 2007-2009 Broadcom Corporation
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
//    Filename: EventLog.h
//    Author:   Kevin O'Neal
//    Creation Date: 27-april-2000
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Header file for event log objects
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef EVENTLOG_H
#define EVENTLOG_H

#ifdef BFC
//#include "BfcSystemConfig.h"
#endif

#include "IpAddress.h"
#include "IpAddressList.h"
#ifndef BCM_STANDALONE_SNMP_AGENT
#include <NiHalIf.h>
#endif
#include "CoreObjs.h"
#include "BcmTrap.h"


#if (BCM_CONSOLE_SUPPORT)
#include "EventLogCommandTable.h"
#endif

#include "EventLogApi.h"
#include "docsDevSyslog.h"


/*\
 *  Forward references
\*/

class EventLogEntry;
class docsDevEventTable;
class docsDevNmAccessTable;
class BcmEventLogThread;
class BcmEventLogNonVolSettings;


#if (!SNMP_SUPPORT)
// If we are building w/o SNMP support, we must make our own list to store
// events.
#include <list>
#include "BcmString.h"
typedef list  <EventLogEntry *> EventLogTable;
typedef EventLogTable::iterator EventLogIterator;
typedef EventLogTable::const_iterator EventLogConstIterator;
typedef void DocsDevEvTrap;
#endif

#define EV_SEVERITY_LEVELS          8


typedef enum
{
  kUnconstrained = 1,
  kMaintainBelowThreshold,
  kStopAtThreshold,
  kInhibited
} ThrottleAdminStatus;

/*\
 *  EventBucket class - controls throttling of async messages
\*/

class EventBucket
{
  public:
    typedef enum 
    {
      kEmpty = -1
    } BucketState;
#if (SNMP_SUPPORT)    
    EventBucket (BcmSnmpAgent *pAgent, unsigned int Threshold = 0, unsigned int Interval = 1, ThrottleAdminStatus Policy = kUnconstrained);
#else
    EventBucket (unsigned int Threshold = 0, unsigned int Interval = 1, ThrottleAdminStatus Policy = kUnconstrained);
#endif
   ~EventBucket ();
   
    int Drip ();
    int Fill ();
    
    unsigned int GetInterval () const;
    unsigned int GetIntervalRemaining () const;
    unsigned int GetThreshold () const;
    int GetContents (); // can't be const because we might fill when called
    int PeekContents () const; // just returns contents, doesn't fill.
    ThrottleAdminStatus GetAdminStatus () const;
    
    SNMP_STATUS SetInterval (unsigned int Interval);
    SNMP_STATUS SetThreshold (unsigned int Threshold);
    SNMP_STATUS SetAdminStatus (ThrottleAdminStatus Admin);
    
  protected:
    time_t GetCurrentIntervalTime () const;
    
  protected:
#if (SNMP_SUPPORT)  
    BcmSnmpAgent *fpAgent;
#endif
    unsigned int fThreshold, fInterval, fContents;
    ThrottleAdminStatus fThrottleAdminStatus;
    time_t fLastFill;
};


/*\
 *  EventLogEntry class - one object per entry in the event log
\*/

class EventLog;

class EventLogEntry
{
  public:
    EventLogEntry (int Index, int Id, int Level, const BcmString &Text, const BcmString &SyslogAppend, DocsDevTrapTypeHint TrapType, EventLog *pLog);
    virtual ~EventLogEntry ();
   
    SNMP_STATUS Log (bool PrintToConsole = true);
    SNMP_STATUS Log (const char *pParam1, const char *pParam2, const char *pParam3, const char *pParam4, const char *pParam5, bool PrintToConsole = true);
    
    // Access functions.  Everything about an event is read-only, so no SET's.
    int GetIndex() const;
    time_t GetFirstTime () const;
    time_t GetLastTime () const;
    unsigned long GetCount () const;
    int GetLevel () const;
    int GetId () const;
    const BcmString &GetText () const;
    const BcmString &GetSyslogText ();
    NVPolicy NonVolPolicy() const;
    
    ostream &Print(ostream &outStream) const;
    
  protected:
    int fIndex;
    time_t fFirstTime;
    time_t fLastTime;
    unsigned long fCount;
    int fLevel;
    int fId;
    BcmString fEvText;
    
    int fNvIndex;
    EventLog *fpEventLog;
    NVPolicy fNonVolPolicy;
    BcmString fRawEvText;
    BcmString fRawSyslogText, fSyslogText;
    DocsDevTrapTypeHint fTrapType;
    docsDevSyslog *fpSyslog;
    BcmTrap *fpTrap;
    
    int fTrapsPending;
    int fSyslogsPending;
    
    virtual int SendTraps (bool ExistingOnly = false, bool PrintToConsole = true);
    virtual int SendSyslogMsg (bool ExistingOnly = false, bool PrintToConsole = true);
    
    friend class EventLog;
};


/*\
 *  EventLog class - controls overall event log behaviour.
\*/

class EventLog
{
  public:
#if (BCM_CONSOLE_SUPPORT)
    static BcmEventLogCommandTable &GetCommandTableSingleton(void);
#endif
  
    // Bits to apply in a mask for level control:
    typedef enum
    {
      kLocalNonVol   = 0x01,
      kTraps         = 0x02,
      kSyslog        = 0x04,
      kLocalVolatile = 0x08
    } LevelControlBits;
    
#if (SNMP_SUPPORT)
    // Accessor to the event table.
    docsDevEventTable *GetEventTable () const;
#endif

    // Public methods which occur asynchronously.  All these really do is post
    // a message to the event log thread, who then calls the protected impl 
    // methods.
    SNMP_STATUS LogEvent (EventLogIdCode Id, EventLogSeverityLevel Level, const char *pText, EventLogPolicy Policy);
    SNMP_STATUS LogEvent (EventLogIdCode Id, const char *pParam1, const char *pParam2, const char *pParam3, const char *pParam4, const char *pParam5);
    SNMP_STATUS SendTrap (int TrapType, int IfIndex = 0);
    SNMP_STATUS WriteNonVolSettings ();
    SNMP_STATUS ResetLog ();
    SNMP_STATUS SendDeferredAsyncs (bool Async = true);
    SNMP_STATUS InitFromNonVol ();
    
    // Event gettin' methods...
    int Capacity() const;
    EventLogEntry *FirstEvent () const;
    EventLogEntry *NextEvent (EventLogEntry *PrevEvent) const;
    EventLogEntry *LastEvent () const;
    EventLogEntry *PrevEvent (EventLogEntry *PrevEvent) const;
    virtual EventLogEntry *FindEvent (unsigned long Id, const char *pText = NULL) const;
    EventLogEntry *FindEventFromNvIndex (int NvIndex) const;
    
    // Access functions to control the way the event log works.
    int GetLogControl () const;
    unsigned short GetTrapControl () const;
    const BcmIpAddress &GetSyslogIp() const;
    void GetSyslogIp(unsigned char *pIp) const;
    int GetThrottleAdminStatus () const;
    bool GetThrottleInhibited ();
    int GetThrottleThreshold () const;
    int GetThrottleInterval () const;
    unsigned short GetLevelControl (int Level) const;
    
    SNMP_STATUS SetLogControl (int Control);
    SNMP_STATUS SetTrapControl (unsigned short Control);
    SNMP_STATUS SetSyslogIp (const BcmIpAddress &Ip);
    SNMP_STATUS SetSyslogIp (const unsigned char *pIp);
    SNMP_STATUS SetLogServers (const BcmIpAddressList &logServers);
    SNMP_STATUS SetThrottleAdminStatus (int Admin);
    SNMP_STATUS SetThrottleThreshold (int Threshold);
    SNMP_STATUS SetThrottleInterval (int Interval);
    SNMP_STATUS SetLevelControl (int Level, unsigned short Control);

    // Throttling things    
    virtual bool IsThrottled ();
    int SentAsyncMsg (EventLogEntry *pEvent, int NumTraps); 
    virtual SNMP_STATUS UseDefaultReporting ();
    virtual SNMP_STATUS DisableLogging ();
    
    // print
    virtual ostream &PrintHeader(ostream &outStream) const;
    virtual ostream &Print(ostream &outStream) const;
    virtual ostream &PrintControl(ostream &outStream) const;
    
    // test & debug
    virtual SNMP_STATUS StartStressTest (int NumEvents, bool Silent = false);
    virtual SNMP_STATUS StopStressTest ();
    
  protected:
    virtual SNMP_STATUS SendDeferredAsyncsImpl ();
    
    // Actually create and log the event.
    virtual EventLogEntry *LogEventImpl (EventLogIdCode Id, EventLogSeverityLevel Level, const char *pText, EventLogPolicy Policy, bool PrintToConsole = true);
    virtual EventLogEntry *LogEventImpl (EventLogIdCode Id, const char *pParam1, const char *pParam2, const char *pParam3, const char *pParam4, const char *pParam5, bool PrintToConsole = true);
    SNMP_STATUS ResetLogImpl ();
    virtual int SendSyslog (docsDevSyslog *pSyslog, bool PrintToConsole = true);
    
    // Non-vol methods
    virtual SNMP_STATUS InitFromNonVolImpl ();
    virtual bool ShouldStoreEventToNonVol (const EventLogEntry *pEvent) const;
    virtual bool ShouldWriteNonVolNow (const EventLogEntry *pEvent, bool IsNewEvent, bool IsNewText) const;
    virtual SNMP_STATUS StoreEventToNonVol (EventLogEntry *pEvent);
    virtual SNMP_STATUS WriteNonVolSettingsImpl ();
    virtual SNMP_STATUS ClearNonVol ();
    
    int ReplaceEmbeddedObjects (const BcmString &RawText, BcmString &EvText, const char *pParam1, const char *pParam2, const char *pParam3, const char *pParam4, const char *pParam5);
    int DecodeEmbeddedObjects (const BcmString &RawText, BcmString &DecodedText);
    
    // Base class implementation knows of no events.  Overload in 
    // derived classes to return a real event.
    virtual DocsisEvent *GetEvent (EventLogIdCode Id);
    
    // Create and return a new trap object based on the event's
    // trap hint.  Might return NULL if the trap type is unknown.
    // Note that it is the caller's responsibility to delete this 
    // trap when finished with it.
    virtual BcmTrap *GetNewTrap (const EventLogEntry *pEvent, DocsDevTrapTypeHint EvTrap);
    
    // Create and return a new syslog message object.  This might vary for
    // different event log objects which may require different syslog
    // formatting.  Could return NULL if the event log object doesn't support
    // syslog.  Note that it is the caller's responsibility to delete this
    // syslog object when finished with it.
    virtual docsDevSyslog *GetNewSyslog (EventLogEntry *pEvent, int SrcIf = 0) { return NULL;};
    
    virtual void StressTestImpl ();

  public:
#if (SNMP_SUPPORT)
    EventLog (const char *pName, BcmSnmpAgent *pAgent);
#else
    EventLog (const char *pName);
#endif
    
    virtual ~EventLog ();
   
  protected:
    static BcmEventLogThread *fpThread;
    static DocsisEvent fEvents[];
    const char *fpName;
#if (SNMP_SUPPORT)
    docsDevEventTable    *fpEventTable;
    BcmSnmpAgent *fpAgent;
#else
    EventLogTable fEventTable;
#endif
    BcmEventLogNonVolSettings *fpSettings;
    bool fAsyncsInhibited;
    int fControl;
    EventBucket fBucket;
    BcmIpAddress fSyslogIp; // docsDevEvSyslog
    BcmIpAddressList fLogServers; // from DHCP
    unsigned short fLevelControl[EV_SEVERITY_LEVELS];
    unsigned short fTrapControl;
    int fStressEvents, fStressCount;
    bool fSilentStress;
    
  public:
    static bool fSilent; // Globally silence console printing
   
  friend class EventLogEntry;
  friend class BcmEventLogThread;
};


inline ostream & operator << (ostream &outStream, const EventLogEntry &Event)
{
    return Event.Print(outStream);
}

inline ostream & operator << (ostream &outStream, EventLog &Log)
{
    return Log.Print(outStream);
}

#endif
