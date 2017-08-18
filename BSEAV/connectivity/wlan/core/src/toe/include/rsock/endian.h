/*
 * RSOCK (Remote Sockets) - Endian Configuration
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

#ifndef __RSOCK_ENDIAN_H__
#define __RSOCK_ENDIAN_H__

/* Endian of host running rsock client library */
#ifndef RSOCK_LE
#define RSOCK_LE		1	/* 0 for big endian, 1 for little */
#endif

/* Endian of dongle running rserv */
#ifndef RSERV_LE
#define RSERV_LE		1
#endif

/* Endian to be used on bus between client and server */
#ifndef RSOCK_BUS_LE
#define RSOCK_BUS_LE		0
#endif

#define rsock_swap32(l)	\
	((((l) >> 24) & 0x000000ff) | \
	 (((l) >>  8) & 0x0000ff00) | \
	 (((l) <<  8) & 0x00ff0000) | \
	 (((l) << 24) & 0xff000000))

#define rsock_swap16(s)	\
	((((s) >> 8) & 0x00ff) | (((s) << 8) & 0xff00))

#endif /* __RSOCK_ENDIAN_H__ */
