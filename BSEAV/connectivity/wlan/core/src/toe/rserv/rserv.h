/*
 * RSERV: Remote Sockets Server
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

#ifndef __RSERV_H__
#define __RSERV_H__

/* Bus protocol byte order conversion */

#if (RSERV_LE == RSOCK_BUS_LE)
#define htobl(x)		(x)
#define btohl(x)		(x)
#define htobs(x)		(x)
#define btohs(x)		(x)
#else
#define htobl(x)		rsock_swap32(x)
#define btohl(x)		rsock_swap32(x)
#define htobs(x)		rsock_swap16(x)
#define btohs(x)		rsock_swap16(x)
#endif

void rserv_input(RSERV_PKT *req_pkt);
void rserv_init(void);

#endif /* __RSERV_H__ */
