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
//    Filename: docsDevEvBridge.cpp
//    Creation Date: Monday, May 20, 2002 at 14:46:53
//    Created by Broadcom V2 Mib Compiler BCMIBC.EXE version 1.2.0b 
//    from input file rfc2669.mib
//
//**************************************************************************

#include "MibObjs.h"
#include "docsDevEvBridge.h"

#include "docsDevEvMib.h"
#include "SnmpLog.h"
#include "BcmSnmpAgent.h"


/******************************************************************************\

-- NOTE: Various non-SMI text removed below (as indicated) for benefit of MIB
-- compiler.  Complete MIB text may be obtained at:
-- http://www.ietf.org/rfc/rfc2669.txt
-- Kevin O'Neal, 9/7/99

-- 1.  The SNMP Management Framework: REMOVED
-- 2.  Glossary: REMOVED
-- 3.  Overview: REMOVED

-- and we really start with...
-- 4.  Definitions

DOCS-CABLE-DEVICE-MIB DEFINITIONS ::= BEGIN

IMPORTS
        MODULE-IDENTITY,
        OBJECT-TYPE,
-- do not import        BITS,
        IpAddress,
        Unsigned32,
        Counter32,
        Integer32,
        zeroDotZero,
        mib-2
                FROM SNMPv2-SMI
        RowStatus,
        RowPointer,
        DateAndTime,
        TruthValue
                FROM SNMPv2-TC
        OBJECT-GROUP,
        MODULE-COMPLIANCE
                FROM SNMPv2-CONF
        SnmpAdminString
                FROM SNMP-FRAMEWORK-MIB -- RFC2571
        InterfaceIndexOrZero
                FROM IF-MIB;  -- RFC2233

docsDev MODULE-IDENTITY
        LAST-UPDATED    "9908190000Z" -- August 19, 1999
        ORGANIZATION    "IETF IPCDN Working Group"
        CONTACT-INFO
            "        Michael StJohns
             Postal: @Home Network
                     425 Broadway
                     Redwood City, CA 94063
                     U.S.A.
             Phone:  +1 650 569 5368
             E-mail: stjohns@corp.home.net"
        DESCRIPTION
            "This is the MIB Module for MCNS-compliant cable modems and
             cable-modem termination systems."
        REVISION "9908190000Z"
        DESCRIPTION
            "Initial Version, published as RFC 2669.
             Modified by Mike StJohns to add/revise filtering, TOS
             support, software version information objects."
        ::= { mib-2 69 }

docsDevMIBObjects  OBJECT IDENTIFIER ::= { docsDev 1 }
docsDevBase OBJECT IDENTIFIER ::= { docsDevMIBObjects 1 }

--
-- For the following object, there is no concept in the
-- RFI specification corresponding to a backup CMTS. The
-- enumeration is provided here in case someone is able
-- to define such a role or device.
--


\******************************************************************************/


/*\
 *
 *  docsDevEventGroupBridge for bridging scalars from the docsDevEventGroup group 
 *
\*/

/* 
docsDevEvent OBJECT IDENTIFIER ::= { docsDevMIBObjects 5 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

docsDevEventGroupBridge::docsDevEventGroupBridge (BcmSnmpAgent *pAgent, EventLog *pLog)
  : ScalarMibBridge ("1.3.6.1.2.1.69.1.5", pAgent)
{
  fpLog = pLog;
}

docsDevEventGroupBridge::~docsDevEventGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
docsDevEvControl OBJECT-TYPE
        SYNTAX INTEGER {
            resetLog(1),
            useDefaultReporting(2)
        }
        MAX-ACCESS  read-write
        STATUS      current
        DESCRIPTION
            "Setting this object to resetLog(1) empties the event log.
             All data is deleted. Setting it to useDefaultReporting(2)
             returns all event priorities to their factory-default
             reporting. Reading this object always returns
             useDefaultReporting(2)."
        ::= { docsDevEvent 1 }
*/ 
int docsDevEventGroupBridge::Get_docsDevEvControl ()
{
  return fpLog->GetLogControl();
}


/* 
docsDevEvSyslog OBJECT-TYPE
        SYNTAX      IpAddress
        MAX-ACCESS  read-write
        STATUS      current
        DESCRIPTION
            "The IP address of the Syslog server. If 0.0.0.0, syslog
             transmission is inhibited."
        ::= { docsDevEvent 2 }
*/ 
void docsDevEventGroupBridge::Get_docsDevEvSyslog (BcmIpAddress &docsDevEvSyslog)
{
  docsDevEvSyslog = fpLog->GetSyslogIp();
}


/* 
docsDevEvThrottleAdminStatus OBJECT-TYPE
        SYNTAX INTEGER {
            unconstrained(1),
            maintainBelowThreshold(2),
            stopAtThreshold(3),
            inhibited(4)
        }
        MAX-ACCESS  read-write
        STATUS      current
        DESCRIPTION
            "Controls the transmission of traps and syslog messages
             with respect to the trap pacing threshold.
             unconstrained(1) causes traps and syslog messages to be
             transmitted without regard to the threshold settings.
             maintainBelowThreshold(2) causes trap transmission and
             syslog messages to be suppressed if the number of traps
             would otherwise exceed the threshold.
             stopAtThreshold(3) causes trap transmission to cease
             at the threshold, and not resume until directed to do so.
             inhibited(4) causes all trap transmission and syslog
             messages to be suppressed.

             A single event is always treated as a single event for
             threshold counting. That is, an event causing both a trap
             and a syslog message is still treated as a single event.

             Writing to this object resets the thresholding state.

             At initial startup, this object has a default value of
             unconstrained(1)."
        ::= { docsDevEvent 3 }
*/ 
int docsDevEventGroupBridge::Get_docsDevEvThrottleAdminStatus ()
{
  return fpLog->GetThrottleAdminStatus();
}


/* 
docsDevEvThrottleInhibited OBJECT-TYPE
        SYNTAX      TruthValue
        MAX-ACCESS  read-only
        STATUS      current
        DESCRIPTION
            "If true(1), trap and syslog transmission is currently
             inhibited due to thresholds and/or the current setting of
             docsDevEvThrottleAdminStatus. In addition, this is set to
             true(1) if transmission is inhibited due to no
             syslog (docsDevEvSyslog) or trap (docsDevNmAccessEntry)
             destinations having been set."
        ::= { docsDevEvent 4 }
*/ 
bool docsDevEventGroupBridge::Get_docsDevEvThrottleInhibited ()
{
  return fpLog->GetThrottleInhibited();
}


/* 
docsDevEvThrottleThreshold OBJECT-TYPE
        SYNTAX      Unsigned32
        MAX-ACCESS  read-write
        STATUS      current
        DESCRIPTION
            "Number of trap/syslog events per docsDevEvThrottleInterval
             to be transmitted before throttling.

             A single event is always treated as a single event for
             threshold counting. That is, an event causing both a trap
             and a syslog message is still treated as a single event.

             At initial startup, this object returns 0."
        ::= { docsDevEvent 5 }
*/ 
unsigned int docsDevEventGroupBridge::Get_docsDevEvThrottleThreshold ()
{
  return fpLog->GetThrottleThreshold();
}


/* 
docsDevEvThrottleInterval OBJECT-TYPE
        SYNTAX      Integer32 (1..2147483647)
        UNITS       "seconds"
        MAX-ACCESS  read-write
        STATUS      current
        DESCRIPTION
            "The interval over which the trap threshold applies.
             At initial startup, this object has a value of 1."

        ::= { docsDevEvent 6 }
*/ 
int docsDevEventGroupBridge::Get_docsDevEvThrottleInterval ()
{
  return fpLog->GetThrottleInterval();
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/

/* 
docsDevEvControl OBJECT-TYPE
        SYNTAX INTEGER {
            resetLog(1),
            useDefaultReporting(2)
        }
        MAX-ACCESS  read-write
        STATUS      current
        DESCRIPTION
            "Setting this object to resetLog(1) empties the event log.
             All data is deleted. Setting it to useDefaultReporting(2)
             returns all event priorities to their factory-default
             reporting. Reading this object always returns
             useDefaultReporting(2)."
        ::= { docsDevEvent 1 }
*/ 
SNMP_STATUS docsDevEventGroupBridge::Set_docsDevEvControl (int docsDevEvControl)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (docsDevEvControl);

  if (Status == SNMP_NO_ERROR)
  {
    if ((docsDevEvControl != CONSTVAL_DOCSDEVEVCONTROL_RESETLOG)
    &&  (docsDevEvControl != CONSTVAL_DOCSDEVEVCONTROL_USEDEFAULTREPORTING))
      return SNMP_WRONG_VALUE;

    return fpLog->SetLogControl (docsDevEvControl);
  }

  return Status;
}


/* 
docsDevEvSyslog OBJECT-TYPE
        SYNTAX      IpAddress
        MAX-ACCESS  read-write
        STATUS      current
        DESCRIPTION
            "The IP address of the Syslog server. If 0.0.0.0, syslog
             transmission is inhibited."
        ::= { docsDevEvent 2 }
*/ 
SNMP_STATUS docsDevEventGroupBridge::Set_docsDevEvSyslog (const BcmIpAddress &docsDevEvSyslog)
{
  SNMP_STATUS Status = IpAddrMibObject::TestDefaults (docsDevEvSyslog);

  if (Status == SNMP_NO_ERROR)
  {
    return fpLog->SetSyslogIp (docsDevEvSyslog);
  }

  return Status;
}


/* 
docsDevEvThrottleAdminStatus OBJECT-TYPE
        SYNTAX INTEGER {
            unconstrained(1),
            maintainBelowThreshold(2),
            stopAtThreshold(3),
            inhibited(4)
        }
        MAX-ACCESS  read-write
        STATUS      current
        DESCRIPTION
            "Controls the transmission of traps and syslog messages
             with respect to the trap pacing threshold.
             unconstrained(1) causes traps and syslog messages to be
             transmitted without regard to the threshold settings.
             maintainBelowThreshold(2) causes trap transmission and
             syslog messages to be suppressed if the number of traps
             would otherwise exceed the threshold.
             stopAtThreshold(3) causes trap transmission to cease
             at the threshold, and not resume until directed to do so.
             inhibited(4) causes all trap transmission and syslog
             messages to be suppressed.

             A single event is always treated as a single event for
             threshold counting. That is, an event causing both a trap
             and a syslog message is still treated as a single event.

             Writing to this object resets the thresholding state.

             At initial startup, this object has a default value of
             unconstrained(1)."
        ::= { docsDevEvent 3 }
*/ 
SNMP_STATUS docsDevEventGroupBridge::Set_docsDevEvThrottleAdminStatus (int docsDevEvThrottleAdminStatus)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (docsDevEvThrottleAdminStatus);

  if (Status == SNMP_NO_ERROR)
  {
    if ((docsDevEvThrottleAdminStatus != CONSTVAL_DOCSDEVEVTHROTTLEADMINSTATUS_UNCONSTRAINED)
    &&  (docsDevEvThrottleAdminStatus != CONSTVAL_DOCSDEVEVTHROTTLEADMINSTATUS_MAINTAINBELOWTHRESHOLD)
    &&  (docsDevEvThrottleAdminStatus != CONSTVAL_DOCSDEVEVTHROTTLEADMINSTATUS_STOPATTHRESHOLD)
    &&  (docsDevEvThrottleAdminStatus != CONSTVAL_DOCSDEVEVTHROTTLEADMINSTATUS_INHIBITED))
      return SNMP_WRONG_VALUE;

    return fpLog->SetThrottleAdminStatus (docsDevEvThrottleAdminStatus);
  }

  return Status;
}


/* 
docsDevEvThrottleThreshold OBJECT-TYPE
        SYNTAX      Unsigned32
        MAX-ACCESS  read-write
        STATUS      current
        DESCRIPTION
            "Number of trap/syslog events per docsDevEvThrottleInterval
             to be transmitted before throttling.

             A single event is always treated as a single event for
             threshold counting. That is, an event causing both a trap
             and a syslog message is still treated as a single event.

             At initial startup, this object returns 0."
        ::= { docsDevEvent 5 }
*/ 
SNMP_STATUS docsDevEventGroupBridge::Set_docsDevEvThrottleThreshold (unsigned int docsDevEvThrottleThreshold)
{
  SNMP_STATUS Status = Uint32MibObject::TestDefaults (docsDevEvThrottleThreshold);

  if (Status == SNMP_NO_ERROR)
  {
    return fpLog->SetThrottleThreshold (docsDevEvThrottleThreshold);
  }

  return Status;
}


/* 
docsDevEvThrottleInterval OBJECT-TYPE
        SYNTAX      Integer32 (1..2147483647)
        UNITS       "seconds"
        MAX-ACCESS  read-write
        STATUS      current
        DESCRIPTION
            "The interval over which the trap threshold applies.
             At initial startup, this object has a value of 1."

        ::= { docsDevEvent 6 }
*/ 
SNMP_STATUS docsDevEventGroupBridge::Set_docsDevEvThrottleInterval (int docsDevEvThrottleInterval)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (docsDevEvThrottleInterval);

  if (Status == SNMP_NO_ERROR)
  {
    if ((docsDevEvThrottleInterval < MINVAL_DOCSDEVEVTHROTTLEINTERVAL)
    ||  (docsDevEvThrottleInterval > MAXVAL_DOCSDEVEVTHROTTLEINTERVAL))
      return SNMP_WRONG_VALUE;

    return fpLog->SetThrottleInterval (docsDevEvThrottleInterval);
  }

  return Status;
}






/*\
 *
 *  docsDevEvControlEntryBridge for bridging elements in the docsDevEvControlTable
 *
\*/

/* 
docsDevEvControlTable OBJECT-TYPE
        SYNTAX      SEQUENCE OF DocsDevEvControlEntry
        MAX-ACCESS  not-accessible
        STATUS      current
        DESCRIPTION
            "This table allows control of the reporting of event classes.
        For each event priority, a combination of logging and
        reporting mechanisms may be chosen. The mapping of event types
        to priorities is vendor-dependent. Vendors may also choose to
        allow the user to control that mapping through proprietary
        means."
        ::= {  docsDevEvent 7 }

INDEX { docsDevEvPriority }
*/ 

/*\ 
 *  construction / destruction
\*/ 

docsDevEvControlEntryBridge::docsDevEvControlEntryBridge (int docsDevEvPriority, BcmSnmpAgent *pAgent, EventLog *pLog)
  : TabularMibBridge ("1.3.6.1.2.1.69.1.5.7",
                      BcmObjectId(docsDevEvPriority),
                      pAgent)
{
  fLevel = docsDevEvPriority;
  fpLog = pLog;
}

docsDevEvControlEntryBridge::~docsDevEvControlEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
docsDevEvPriority OBJECT-TYPE
        SYNTAX INTEGER {
            emergency(1),
            alert(2),
            critical(3),
            error(4),
            warning(5),
            notice(6),
            information(7),
            debug(8)
        }
        MAX-ACCESS  not-accessible
        STATUS      current
        DESCRIPTION
            "The priority level that is controlled by this
             entry. These are ordered from most (emergency) to least
             (debug) critical.  Each event with a CM or CMTS has a
             particular priority level associated with it (as defined
             by the vendor). During normal operation no event more
             critical than notice(6) should be generated. Events between
             warning and emergency should be generated at appropriate
             levels of problems (e.g. emergency when the box is about to
             crash)."
        ::= { docsDevEvControlEntry 1 }
*/ 
int docsDevEvControlEntryBridge::Get_docsDevEvPriority ()
{
  return fLevel;
}


/* 
docsDevEvReporting OBJECT-TYPE
        SYNTAX BITS {
            local(0),
            traps(1),
            syslog(2)
        }
        MAX-ACCESS  read-write
        STATUS      current
        DESCRIPTION
            "Defines the action to be taken on occurrence of this
             event class. Implementations may not necessarily support
             all options for all event classes, but at minimum must
             allow traps and syslogging to be disabled. If the
             local(0) bit is set, then log to the internal log, if the
             traps(1) bit is set, then generate a trap, if the
             syslog(2) bit is set, then send a syslog message
             (assuming the syslog address is set)."
        ::= { docsDevEvControlEntry 2 }
*/ 
unsigned long docsDevEvControlEntryBridge::Get_docsDevEvReporting ()
{
  return fpLog->GetLevelControl (fLevel);
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/

/* 
docsDevEvReporting OBJECT-TYPE
        SYNTAX BITS {
            local(0),
            traps(1),
            syslog(2)
        }
        MAX-ACCESS  read-write
        STATUS      current
        DESCRIPTION
            "Defines the action to be taken on occurrence of this
             event class. Implementations may not necessarily support
             all options for all event classes, but at minimum must
             allow traps and syslogging to be disabled. If the
             local(0) bit is set, then log to the internal log, if the
             traps(1) bit is set, then generate a trap, if the
             syslog(2) bit is set, then send a syslog message
             (assuming the syslog address is set)."
        ::= { docsDevEvControlEntry 2 }
*/ 
SNMP_STATUS docsDevEvControlEntryBridge::Set_docsDevEvReporting (unsigned long docsDevEvReporting)
{
  SNMP_STATUS Status = BitsMibObject::TestDefaults (docsDevEvReporting);

  if (Status == SNMP_NO_ERROR)
  {
    unsigned long Mask = ~((1 << CONSTVAL_DOCSDEVEVREPORTING_LOCAL) |
                           (1 << CONSTVAL_DOCSDEVEVREPORTING_TRAPS) |
                           (1 << CONSTVAL_DOCSDEVEVREPORTING_SYSLOG) |
                           (1 << CONSTVAL_DOCSDEVEVREPORTING_LOCALVOLATILE));

    if (docsDevEvReporting & Mask)
      return SNMP_WRONG_VALUE;

    /*\
     *  From OSS-N-01066:
     *  =================
     *  
     *  Permissible BITs values for RFC2669 object docsDevEvReporting
     *  include:
     *  1:local-nonvolatile(0)
     *  2:traps(1)
     *  3:syslog(2)
     *  4:local-volatile(3)
     *
     *  ** To maintain compatibility with the meaning of default values used by 
     *     Docsis 1.0, the term local-nonvolatile should be interpreted as 
     *     (local- nonvolatile and volatile). The term local-volatile should be 
     *     interpreted to mean local-volatile only. If a vendor implements the 
     *     local-nonvolatile log, the default setting of the local-nonvolatile 
     *     logging bit in the docsDevEvReporting object is at the discretion of 
     *     the vendor.
     *
     *  The following BITS type values for RFC2669 object docsDevEvReporting MUST 
     *  not be accepted: 
     *     0x20 = syslog only 
     *     0x40 = trap only 
     *     0x60 = (trap + syslog) only
     *  Note that the lower nibble MUST be zero in all cases, resulting in 
     *  thirteen acceptable values.  Snmp SET requests for unacceptable values 
     *  MUST result in a Wrong Value error for SNMPv2c/v3 PDUs or a 'Bad Value' 
     *  error for SNMPv1 PDUs.
    \*/
    
    if ((docsDevEvReporting & ~0x0F) != 0)
      return SNMP_WRONG_VALUE;
      
    if ((docsDevEvReporting == EventLog::kTraps)
    ||  (docsDevEvReporting == EventLog::kSyslog)
    ||  (docsDevEvReporting == (EventLog::kSyslog | EventLog::kTraps)))
      return SNMP_WRONG_VALUE;

    return fpLog->SetLevelControl (fLevel, docsDevEvReporting);
  }

  return Status;
}






/*\
 *
 *  docsDevEventEntryBridge for bridging elements in the docsDevEventTable
 *
\*/

/* 
docsDevEventTable OBJECT-TYPE
        SYNTAX      SEQUENCE OF DocsDevEventEntry
        MAX-ACCESS  not-accessible
        STATUS      current
        DESCRIPTION
            "Contains a log of network and device events that may be
             of interest in fault isolation and troubleshooting."
        ::= {  docsDevEvent 8 }

INDEX { docsDevEvIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

docsDevEventEntryBridge::docsDevEventEntryBridge (int Id, int Level, const char *pText, const char *pSyslogAppend, DocsDevTrapTypeHint TrapType, BcmSnmpAgent *pAgent, EventLog *pLog)
  : TabularMibBridge ("1.3.6.1.2.1.69.1.5.8",
                      0,
                      pAgent)
{
  int Index;
  BcmObjectId MyOid, IxOid;
  
  
  fpLog = pLog;
  
  fpEventTable = fpLog->GetEventTable();
    
  if (fpEventTable == NULL)
  {
    SnmpLogInit << "docsDevEventEntryBridge: Couldn't find docsDevEventTable" << endl;
    return;
  }
  
  // Figure out what the index of this event should be.  Have a look at the
  // last event in the table and increment its index by one.
  // Index = Something...
  BcmMibEntry *pEntry = fpEventTable->LastEntry();
  if (pEntry == NULL)
    Index = 1;
  else
  {
    pEntry->GetIndex(IxOid);
    Index = IxOid[0]+1;
  }
    
  // NULL syslog append, just use an empty string:
  if (pSyslogAppend == NULL)
    pSyslogAppend = "";

  fpEvent = new EventLogEntry (Index, Id, Level, BcmString (pText), BcmString (pSyslogAppend), TrapType, fpLog);
  
  SetIndex (BcmObjectId ((unsigned long)Index));
}

docsDevEventEntryBridge::~docsDevEventEntryBridge ()
{
  if (fpEvent)
  {
    SnmpLogMem << "Deleting " << *fpEvent << endl;
    delete fpEvent;                               
  }
}

SNMP_STATUS docsDevEventEntryBridge::Register (CleanupMethod Cleanup, bool SilentFailure)
{
  SNMP_STATUS Status;
  

  if (fpEvent == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;
    
  Status = TabularMibBridge::Register (Cleanup, SilentFailure);
  
  // At this point we should also pop off the head of the event log list
  // if it's gotten too long.
  if ((Status == SNMP_NO_ERROR)
  &&  (fpEventTable->NumEntries() > fpLog->Capacity()))
  {
    BcmMibEntry *pEntry = fpEventTable->FirstEntry();

    if (pEntry)
    {
      BcmObjectId Index;
      pEntry->GetIndex(Index);
      fpEventTable->DestroyEntry (Index);
    }
  }
  
  return Status;
}

/*\ 
 *  GET methods
\*/

/* 
docsDevEvIndex OBJECT-TYPE
        SYNTAX      Integer32 (1..2147483647)
        MAX-ACCESS  not-accessible
        STATUS      current
        DESCRIPTION
            "Provides relative ordering of the objects in the event
             log. This object will always increase except when
             (a) the log is reset via docsDevEvControl,
             (b) the device reboots and does not implement non-volatile
             storage for this log, or (c) it reaches the value 2^31.
             The next entry for all the above cases is 1."
        ::= { docsDevEventEntry 1 }
*/ 
int docsDevEventEntryBridge::Get_docsDevEvIndex ()
{
  return fpEvent->GetIndex();
}


/* 
docsDevEvFirstTime OBJECT-TYPE
        SYNTAX      DateAndTime
        MAX-ACCESS  read-only
        STATUS      current
        DESCRIPTION
            "The time that this entry was created."
        ::= { docsDevEventEntry 2 }
*/ 
time_t docsDevEventEntryBridge::Get_docsDevEvFirstTime ()
{
  return fpEvent->GetFirstTime();
}


/* 
docsDevEvLastTime OBJECT-TYPE
        SYNTAX      DateAndTime
        MAX-ACCESS  read-only
        STATUS      current
        DESCRIPTION
            "If multiple events are reported via the same entry, the
             time that the last event for this entry occurred,
             otherwise this should have the same value as
             docsDevEvFirstTime. "
        ::= { docsDevEventEntry 3 }
*/ 
time_t docsDevEventEntryBridge::Get_docsDevEvLastTime ()
{
  return fpEvent->GetLastTime();
}


/* 
docsDevEvCounts OBJECT-TYPE
        SYNTAX      Counter32
        MAX-ACCESS  read-only
        STATUS      current
        DESCRIPTION
            "The number of consecutive event instances reported by
             this entry.  This starts at 1 with the creation of this
             row and increments by 1 for each subsequent duplicate
             event."
        ::= { docsDevEventEntry 4 }
*/ 
unsigned int docsDevEventEntryBridge::Get_docsDevEvCounts ()
{
  return fpEvent->GetCount();
}


/* 
docsDevEvLevel OBJECT-TYPE
        SYNTAX INTEGER {
            emergency(1),
            alert(2),
            critical(3),
            error(4),
            warning(5),
            notice(6),
            information(7),
            debug(8)
        }
        MAX-ACCESS  read-only
        STATUS      current
        DESCRIPTION
            "The priority level of this event as defined by the
             vendor.  These are ordered from most serious (emergency)
             to least serious (debug)."
        ::= { docsDevEventEntry 5 }
*/ 
int docsDevEventEntryBridge::Get_docsDevEvLevel ()
{
  return fpEvent->GetLevel();
}


/* 
docsDevEvId OBJECT-TYPE
        SYNTAX      Unsigned32
        MAX-ACCESS  read-only
        STATUS      current
        DESCRIPTION
            "For this product, uniquely identifies the type of event
             that is reported by this entry."
        ::= { docsDevEventEntry 6 }
*/ 
unsigned int docsDevEventEntryBridge::Get_docsDevEvId ()
{
  return fpEvent->GetId();
}


/* 
docsDevEvText OBJECT-TYPE
        SYNTAX      SnmpAdminString
        MAX-ACCESS  read-only
        STATUS      current
        DESCRIPTION
            "Provides a human-readable description of the event,
             including all relevant context (interface numbers,
             etc.)."
        ::= { docsDevEventEntry 7 }
*/ 
int docsDevEventEntryBridge::Get_docsDevEvText (BcmString &docsDevEvText)
{
  docsDevEvText = fpEvent->GetText();
  return docsDevEvText.length();
}

EventLogEntry *docsDevEventEntryBridge::GetEvent ()
{
  return fpEvent;
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/


