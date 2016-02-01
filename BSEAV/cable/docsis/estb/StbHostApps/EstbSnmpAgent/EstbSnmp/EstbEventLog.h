//**************************************************************************
//
//    Copyright 2006 Broadcom Corporation
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
//    Filename: EstbEventLog.h
//    Author:   Maurice Turcotte
//    Creation Date: 12 MAY 06
//
//**************************************************************************
//    Description:
//
//    Estb event log objects
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef ESTEVENTLOG_H
#define ESTEVENTLOG_H

#include "EventLog.h"


/*\
 *  EventLog class - singleton that controls overall event log behaviour.
\*/

class EstbEventLog : public EventLog
{
  public:
    static EstbEventLog &Singleton();
    
    ostream &PrintControl (ostream &outStream) const;

    // This function provides the latest CM status code as per 
    // the RFI spec appendix I.
    void GetCmStatusCode (char *pCode, int MaxLen) const;
    bool IsThrottled ();
    
  protected:
    EstbEventLog ();
    bool UpdateCmStatusCode (const EventLogEntry *pEvent, char *pCodeStr = NULL, time_t *pTime = NULL);
    virtual DocsisEvent *GetEvent (EventLogIdCode Id);
    virtual BcmTrap *GetNewTrap (const EventLogEntry *pEvent, DocsDevTrapTypeHint EvTrap);
    virtual docsDevSyslog *GetNewSyslog (EventLogEntry *pEvent, int SrcIf = 0);
    
    EventLogEntry *LogEventImpl (EventLogIdCode Id, EventLogSeverityLevel Level, const char *pText, EventLogPolicy Policy, bool PrintToConsole = true);
    EventLogEntry *LogEventImpl (EventLogIdCode Id, const char *pParam1, const char *pParam2, const char *pParam3, const char *pParam4, const char *pParam5, bool PrintToConsole = true);
    
    // Non-vol overrides
    virtual SNMP_STATUS InitFromNonVolImpl ();
    virtual bool ShouldWriteNonVolNow (const EventLogEntry *pEvent, bool IsNewEvent, bool IsNewText) const;
  
  protected:
    char fCmStatusCode[16];
    time_t fCmStatusCodeTime;
    static DocsisEvent fEvents[];
    
  public:
   ~EstbEventLog ();
};


#endif
