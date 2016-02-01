//**************************************************************************
//
//    Copyright 2000  Broadcom Corporation
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
//    Filename: docsDevSyslog.cpp
//    Author:   Kevin O'Neal
//    Creation Date: 22-may-2000
//
//**************************************************************************
//    Description:
//
//      Source file for syslog & trap objects, used by event log
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include "docsDevSyslog.h"
#include "EventLog.h"
#include "SnmpLog.h"
#include "MergedNonVolDefaults.h"


docsDevSyslog::docsDevSyslog (EventLogEntry *pEvent, int SrcIf)
  : BcmSyslog (pEvent->GetText().c_str(), LOG_NOTICE, SrcIf)
{
  fpEvent = pEvent;
}

docsDevSyslog::~docsDevSyslog ()
{
}

void docsDevSyslog::FormatMsg (bool /*showYear*/)
{
  char DocsisMessage[256];
  char Text[128];
  char Vendor[32];

  // NOTE: this derived class doesn't know from showYear.  Ignore it.

  memset (Vendor, 0, sizeof(Vendor));
  
  // Vendor field is the vendor name for vendor-specific events only,
  // DOCSIS for standard DOCSIS events.  Per OSS-I04 section 4.4.2.2.2
  if (fpEvent->GetId() & VENDOR_SPECIFIC_EVENT_ID_PREFIX)
    // NOTE: vendor name used to be stored in NV, but now is compiled in.
    // To avoid platform-specific NV dependency here, use the macro directly.
    strncpy (Vendor, kDefaultValue_VendorName, sizeof(Vendor)-1);  
  else
    strcpy (Vendor, "DOCSIS");
  
  memset (Text, 0, sizeof(Text));
  strncpy (Text, fpEvent->GetSyslogText().c_str(), sizeof(Text)-1);
  
  /*\
   *  Format the message according to ATP
   *  (TP-ATP-I01-981014, section 2.6.8 p. 330)
   *  [LEVEL].CABLEMODEM.[VENDOR]:[TEXT]
   *
   *  Here is an example from a certified GI/Moto CM, so we can
   *  assume it matches the desired format:
   *  <135>Cablemodem [Motorola Corporation]7 H501.6 HFC: TRC SYNC Recovery OK
   *  where the part "7 H501.6 HFC: TRC SYNC Recovery OK" is the text
  \*/ 
  /*
  sprintf (DocsisMessage, "%d.CABLEMODEM.Broadcom:%s",
    (fLevel-1) | LOG_LOCAL0, fText.c_str());
  */
  sprintf (DocsisMessage, "<%d> CABLEMODEM [%s]: <%lu> %s",
    (fpEvent->GetLevel()-1) | LOG_LOCAL0, Vendor, (unsigned long)fpEvent->GetId(), Text);
  
  /*\
   *  Format the message, syslog style:
   *  "<%d>%.15s %s[%d]: %s",
   *  <priority, facillity code>, <date time: ctime()+4>,
   *  <process name>, <PID>, <message text>
   * 
   *  Facility code must be 128 per ATP.
   *  AKA LOG_LOCAL0
   * 
   *  Process name, would LIKE to use task name. 
   *  Unfortunately, there is no pSOS system call to map tid->name
   *  (see http://www.isi.com/customer/customer/faq/psossys/Common/common4.html)
   *  So use SYSLOG_CMNAME instead.
   * 
   *  PID, use TID.
  
  NOTE: CableLabs does not want a standard syslog message.  They just want
        the format from DocsisMessage, above.  So don't do this:
        
  static char SyslogMessage[128];
        
  // Get the task ID of the caller.
  if (t_ident (NULL, 0, &tid))
    Print ("t_ident call failed.\n");
        
  sprintf (SyslogMessage, "<%d>%.15s %s[%d]: %s",
    Priority | LOG_LOCAL0, TimeStr+4, SYSLOG_CMNAME, tid, DocsisMessage);
  \*/
  
  fFormattedMsg = DocsisMessage;
}


