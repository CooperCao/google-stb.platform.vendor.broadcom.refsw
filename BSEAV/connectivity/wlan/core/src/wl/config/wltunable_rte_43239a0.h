/*
 * Broadcom 802.11abg Networking Device Driver Configuration file
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
 *
 * wl driver tunables for 43239a0 full dongle rte dev
 */

#define D11CONF		0x0	        /* D11 Core Rev 32; D11CONF2 */
#define D11CONF2	0x00000001	/* D11 Core Rev 32 */
#define NCONF		0x40000		/* N-Phy Rev18(LCN 2x2 Rev2) */

#define NTXD		64
#define NRXD		64
#define NRXBUFPOST	24
#define WLC_MAXBSSCFG	8
#define	MAXSCB		(WLC_MAXBSSCFG )
#define WLC_DATAHIWAT	48
#define RXBND		16
#define AIDMAPSZ	32
