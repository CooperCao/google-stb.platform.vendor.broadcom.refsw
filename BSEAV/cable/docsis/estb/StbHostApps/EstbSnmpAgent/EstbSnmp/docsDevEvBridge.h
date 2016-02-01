//**************************************************************************
//                                                                          
//    Copyright 2001 Broadcom Corporation                                   
//    All Rights Reserved                                                   
//    No portions of this material may be reproduced in any form without the
//    written permission of:                                                
//             Broadcom Corporation                                         
//             16251 Laguna Canyon Road                                     
//             Irvine, California  92618                                    
//    All information contained in this document is Broadcom Corporation    
//    company private, proprietary, and trade secret.                       
//                                                                          
//                                                                          
//                                                                          
//**************************************************************************
//    Description:                                                          
//                                                                          
//**************************************************************************
//    Revision History:                                                     
//                                                                          
//**************************************************************************
//    Filename: docsDevEvBridge.h
//    Creation Date: Monday, May 20, 2002 at 14:46:53
//    Created by Broadcom V2 Mib Compiler BCMIBC.EXE version 1.2.0b 
//    from input file rfc2669.mib
//
//**************************************************************************

#ifndef DOCSDEVEVBRIDGE_H
#define DOCSDEVEVBRIDGE_H


#include "CoreObjs.h"
#include "MibObjs.h"
#include "MibBridge.h"

//#include "docsDevEvMib.h"

#include "docsDevTrap.h"
#include "EventLog.h"

/*\
 *  docsDevEventGroup 
\*/

#define CONSTVAL_DOCSDEVEVCONTROL_RESETLOG  1
#define CONSTVAL_DOCSDEVEVCONTROL_USEDEFAULTREPORTING  2
#define CONSTVAL_DOCSDEVEVTHROTTLEADMINSTATUS_UNCONSTRAINED  1
#define CONSTVAL_DOCSDEVEVTHROTTLEADMINSTATUS_MAINTAINBELOWTHRESHOLD  2
#define CONSTVAL_DOCSDEVEVTHROTTLEADMINSTATUS_STOPATTHRESHOLD  3
#define CONSTVAL_DOCSDEVEVTHROTTLEADMINSTATUS_INHIBITED  4
#define CONSTVAL_DOCSDEVEVTHROTTLEINHIBITED_TRUE  1
#define CONSTVAL_DOCSDEVEVTHROTTLEINHIBITED_FALSE  2
#define MINVAL_DOCSDEVEVTHROTTLEINTERVAL  1
#define MAXVAL_DOCSDEVEVTHROTTLEINTERVAL  2147483647

class docsDevEventGroupBridge : public ScalarMibBridge
{
  friend class docsDevEventGroup;

  public:
    docsDevEventGroupBridge (BcmSnmpAgent *pAgent, EventLog *pLog);
   ~docsDevEventGroupBridge ();

    /* GET methods */

    int Get_docsDevEvControl ();
    void Get_docsDevEvSyslog (BcmIpAddress &docsDevEvSyslog);
    int Get_docsDevEvThrottleAdminStatus ();
    bool Get_docsDevEvThrottleInhibited ();
    unsigned int Get_docsDevEvThrottleThreshold ();
    int Get_docsDevEvThrottleInterval ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

    SNMP_STATUS Set_docsDevEvControl (int docsDevEvControl);
    SNMP_STATUS Set_docsDevEvSyslog (const BcmIpAddress &docsDevEvSyslog);
    SNMP_STATUS Set_docsDevEvThrottleAdminStatus (int docsDevEvThrottleAdminStatus);
    SNMP_STATUS Set_docsDevEvThrottleThreshold (unsigned int docsDevEvThrottleThreshold);
    SNMP_STATUS Set_docsDevEvThrottleInterval (int docsDevEvThrottleInterval);
    
  protected:
    EventLog *fpLog;
};



/*\
 *  docsDevEvControlEntry 
\*/

#define CONSTVAL_DOCSDEVEVPRIORITY_EMERGENCY  1
#define CONSTVAL_DOCSDEVEVPRIORITY_ALERT  2
#define CONSTVAL_DOCSDEVEVPRIORITY_CRITICAL  3
#define CONSTVAL_DOCSDEVEVPRIORITY_ERROR  4
#define CONSTVAL_DOCSDEVEVPRIORITY_WARNING  5
#define CONSTVAL_DOCSDEVEVPRIORITY_NOTICE  6
#define CONSTVAL_DOCSDEVEVPRIORITY_INFORMATION  7
#define CONSTVAL_DOCSDEVEVPRIORITY_DEBUG  8
#define CONSTVAL_DOCSDEVEVREPORTING_LOCAL  0
#define CONSTVAL_DOCSDEVEVREPORTING_TRAPS  1
#define CONSTVAL_DOCSDEVEVREPORTING_SYSLOG  2
#define CONSTVAL_DOCSDEVEVREPORTING_LOCALVOLATILE 3 // Added by OSS-N-01066

class docsDevEvControlEntryBridge : public TabularMibBridge
{
  friend class docsDevEvControlEntry;

  public:
    docsDevEvControlEntryBridge (int docsDevEvPriority, BcmSnmpAgent *pAgent, EventLog *pLog);
   ~docsDevEvControlEntryBridge ();

    /* GET methods */

    int Get_docsDevEvPriority ();
    unsigned long Get_docsDevEvReporting ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

    SNMP_STATUS Set_docsDevEvReporting (unsigned long docsDevEvReporting);
    
  protected:
    int fLevel;
    EventLog *fpLog;
};



/*\
 *  docsDevEventEntry 
\*/

#define MINVAL_DOCSDEVEVINDEX  1
#define MAXVAL_DOCSDEVEVINDEX  2147483647
#define CONSTVAL_DOCSDEVEVLEVEL_EMERGENCY  1
#define CONSTVAL_DOCSDEVEVLEVEL_ALERT  2
#define CONSTVAL_DOCSDEVEVLEVEL_CRITICAL  3
#define CONSTVAL_DOCSDEVEVLEVEL_ERROR  4
#define CONSTVAL_DOCSDEVEVLEVEL_WARNING  5
#define CONSTVAL_DOCSDEVEVLEVEL_NOTICE  6
#define CONSTVAL_DOCSDEVEVLEVEL_INFORMATION  7
#define CONSTVAL_DOCSDEVEVLEVEL_DEBUG  8
#define MINLEN_DOCSDEVEVTEXT  0
#define MAXLEN_DOCSDEVEVTEXT  255

// Forward reference:
class EventLogEntry;
class docsDevEventTable;
class docsDevEventEntryBridge : public TabularMibBridge
{
  friend class docsDevEventEntry;

  public:
    docsDevEventEntryBridge (int Id, int Level, const char *pText, const char *pSyslogAppend, DocsDevTrapTypeHint TrapType, BcmSnmpAgent *pAgent, EventLog *pLog);
   ~docsDevEventEntryBridge ();
   
   virtual SNMP_STATUS Register(CleanupMethod Cleanup = kDeleteBridge, bool SilentFailure=false);

    /* GET methods */

    int Get_docsDevEvIndex ();
    time_t Get_docsDevEvFirstTime ();
    time_t Get_docsDevEvLastTime ();
    unsigned int Get_docsDevEvCounts ();
    int Get_docsDevEvLevel ();
    unsigned int Get_docsDevEvId ();
    int Get_docsDevEvText (BcmString &docsDevEvText);

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */


    EventLogEntry *GetEvent ();
    
  protected:
    docsDevEventTable *fpEventTable;
    EventLogEntry *fpEvent;
    EventLog *fpLog; 
};

#endif
