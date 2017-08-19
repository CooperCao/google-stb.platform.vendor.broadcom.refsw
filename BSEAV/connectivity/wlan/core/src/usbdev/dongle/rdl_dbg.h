/*
 * RNDIS debug macros
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

#ifndef _rdl_dbg_h_
#define _rdl_dbg_h_

#include <typedefs.h>
#include <osl.h>

#define RDL_ERROR	0x00000001
#define RDL_TRACE	0x00000002
#define RDL_PRPKT	0x00000008
#define RDL_INFORM	0x00000010

#ifdef BCMDBG
#define rdl_dbg(bit, fmt, args...) do { \
	extern int rdl_msglevel; \
	if (rdl_msglevel & (bit)) \
		printf("%s: " fmt "\n", __FUNCTION__ , ## args); \
} while (0)
#define rdl_hex(msg, buf, len) do { \
	extern int rdl_msglevel; \
	if (rdl_msglevel & RNDIS_PRPKT) \
		prhex(msg, buf, len); \
} while (0)
#else
#define rdl_dbg(bit, fmt, args...)
#define rdl_hex(msg, buf, len)
#endif

/* Debug functions */
#define err(fmt, args...) rdl_dbg(RDL_ERROR, fmt , ## args)
#define trace(fmt, args...) rdl_dbg(RDL_TRACE, fmt , ## args)
#define dbg(fmt, args...) rdl_dbg(RDL_INFORM, fmt , ## args)
#define hex(msg, buf, len) rdl_hex(msg, buf, len)

#endif /* _rdl_dbg_h_ */
