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

#define	D11CONF		0xe000		/* D11 Core Rev 13 (4328), 14 (4325A0/A1), 15 (4325B0) */
#define D11CONF2	0		/* D11 Core Rev > 31 */
#define NCONF		0x7		/* N-Phy Rev 0 - 2 */
#define LPCONF		0xd		/* LP-Phy Rev 0 & 2 & 3 */

#define NTXD		32
#define NRXD		16
#define	NRXBUFPOST	4
#define	MAXSCB		8
#define WLC_DATAHIWAT	10
#define RXBND		4
