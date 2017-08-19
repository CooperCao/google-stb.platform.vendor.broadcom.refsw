/*
 * Copyright 1998 Epigram, Inc.
 *
 * $Id$
 *
 */

#ifndef _NTSYSLOG_H_
#define _NTSYSLOG_H_

HANDLE
open_event_log(void);

void
close_event_log(void);

HANDLE
get_event_log(void);

void
set_event_source(char* source_name);

#endif /* _NTSYSLOG_H_ */
