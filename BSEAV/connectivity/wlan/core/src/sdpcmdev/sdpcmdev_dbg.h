/*
 * USB device debug macros
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#ifndef	_sdpcmdev_dbg_h_
#define	_sdpcmdev_dbg_h_

#include <typedefs.h>
#include <osl.h>

#define SD_ERROR_VAL	0x00000001	/* Error level tracing */
#define SD_TRACE_VAL	0x00000002	/* Function level tracing */
#define SD_PRHDRS_VAL	0x00000004	/* print packet headers */
#define SD_PRPKT_VAL	0x00000008	/* print packet contents */
#define SD_INFORM_VAL	0x00000010	/* debug level tracing */

/* sd_msg_level is a bitvector with defs in wlioctl.h */
#ifdef	BCMDBG
extern int sd_msg_level;

#ifdef MSGTRACE
#define HBUS_TRACE msgtrace_hbus_trace = TRUE
#else
#define HBUS_TRACE
#endif

#define	SD_ERROR(args)		do {if (sd_msg_level & SD_ERROR_VAL) printf args;} while (0)
#define	SD_TRACE(args)		do {if (sd_msg_level & SD_TRACE_VAL) \
					{HBUS_TRACE; printf args;}} while (0)
#define SD_PRHDRS(i, p, f, t, r, l) do {if (sd_msg_level & SD_PRHDRS_VAL)  \
					{HBUS_TRACE; wlc_dumphdrs(i, p, f, t, r, l);}} while (0)
#define	SD_PRPKT(m, b, n)	do {if (sd_msg_level & SD_PRPKT_VAL) \
					{HBUS_TRACE; prhex(m, b, n);}} while (0)
#define	SD_INFORM(args)		do {if (sd_msg_level & SD_INFORM_VAL) \
					{HBUS_TRACE; printf args;}} while (0)
#else	/* BCMDBG */

#ifdef BCMDBG_ERR
#define	SD_ERROR(args)		printf args
#else
#define	SD_ERROR(args)
#endif /* BCMDBG_ERR */

#define	SD_TRACE(args)
#define	SD_PRHDRS(i, s, h, p, n, l)
#define	SD_PRPKT(m, b, n)
#define	SD_INFORM(args)
#endif /* !BCMDBG */


#endif /* _sdpcmdev_dbg_h_ */
