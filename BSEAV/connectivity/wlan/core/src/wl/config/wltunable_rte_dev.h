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
 * wl driver tunables for RTE dev
 */
#ifdef BCMDBG
#define	D11CONF		0x380		/* D11 Core Rev 7 - 9 (4320, 4712, 5352) */
#define D11CONF2	0		/* D11 Core Rev > 31 */
#else
#define	D11CONF		0x100		/* D11 Core Rev 8 (4320) */
#define D11CONF2	0		/* D11 Core Rev > 31 */
#endif
#ifdef BCMDBG
#define	GCONF		0xc4		/* G-Phy Rev 2 & 6 & 7 (4712, 5352, 4320) */
#else
#define	GCONF		0x40		/* G-Phy Rev 6 (4320A2) */
#endif

#define NTXD		32
#define NRXD		16
#define	NRXBUFPOST	4
#define	MAXSCB		8
#define WLC_DATAHIWAT	10
#define RXBND		4
