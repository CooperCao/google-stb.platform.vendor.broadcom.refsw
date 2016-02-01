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
//    Filename: docsDevSyslog.h
//    Author:   Kevin O'Neal
//    Creation Date: 22-may-2000
//
//**************************************************************************
//    Description:
//
//      Header file for syslog object, used by event log
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef DOCSDEVSYSLOG_H
#define DOCSDEVSYSLOG_H

#include "BcmSyslog.h"

// forward reference:
class EventLogEntry;

class docsDevSyslog : public BcmSyslog
{ 
  public:
    docsDevSyslog (EventLogEntry *pEvent, int SrcIf = 0);
    virtual ~docsDevSyslog ();
    
  protected:
    virtual void FormatMsg (bool showYear);
    
  protected:
    EventLogEntry *fpEvent;
};

#endif
