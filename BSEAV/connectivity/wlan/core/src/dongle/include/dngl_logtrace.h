/*
 * Trace log blocks sent over HBUS
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: logtrace.h 333856 2012-05-17 23:43:07Z $
 */

#ifndef	_dngl_logtrace_h_
#define	_dngl_logtrace_h_

#include <typedefs.h>
#include <osl_decl.h>

extern void logtrace_start(void);
extern void logtrace_stop(void);
extern int logtrace_sent(void);
extern int logtrace_init(osl_t* osh);
extern void logtrace_deinit(osl_t* osh);
extern bool logtrace_event_enabled(void);

/* setup trace sendup func */
typedef void (*logtrace_sendup_trace_fn_t)(void *ctx, uint8 *hdr, uint16 hdrlen,
	uint8 *buf, uint16 buflen);
void logtrace_set_sendup_trace_fn(logtrace_sendup_trace_fn_t fn, void *ctx);

#endif	/* _dngl_logtrace_h_ */
