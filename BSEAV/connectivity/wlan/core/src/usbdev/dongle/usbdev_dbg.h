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

#ifndef	_usbdev_dbg_h_
#define	_usbdev_dbg_h_

#include <typedefs.h>
#include <osl.h>

#define USBDEV_ERROR	0x00000001
#define USBDEV_TRACE	0x00000002
#define USBDEV_PRPKT	0x00000008
#define USBDEV_INFORM	0x00000010

#if defined(BCMDBG) || defined(BCMDBG_ERR)
#define usbdev_dbg(bit, fmt, args...) do { \
	extern int usbdev_msglevel; \
	if (usbdev_msglevel & (bit)) \
		printf("%s: " fmt "\n", __FUNCTION__ , ## args); \
} while (0)
#define usbdev_hex(msg, buf, len) do { \
	extern int usbdev_msglevel; \
	if (usbdev_msglevel & USBDEV_PRPKT) \
		prhex(msg, buf, len); \
} while (0)
#else
#define usbdev_dbg(bit, fmt, args...)
#define usbdev_hex(msg, buf, len)
#endif

/* Debug functions */
#if defined(BCMDBG) || defined(BCMDBG_ERR)
#define err(fmt, args...) usbdev_dbg(USBDEV_ERROR, fmt , ## args)
#else
#define err(fmt, args...)
#endif

#if defined(BCMDBG)
#define trace(fmt, args...) usbdev_dbg(USBDEV_TRACE, fmt , ## args)
#define dbg(fmt, args...) usbdev_dbg(USBDEV_INFORM, fmt , ## args)
#define hex(msg, buf, len) usbdev_hex(msg, buf, len)
#else
#define trace(fmt, args...)
#define dbg(fmt, args...)
#define hex(msg, buf, len)
#endif

#endif /* _usbdev_dbg_h_ */
