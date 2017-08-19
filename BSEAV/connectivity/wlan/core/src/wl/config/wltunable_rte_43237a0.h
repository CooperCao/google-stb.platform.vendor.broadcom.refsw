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
 * wl driver tunables for 43237a0 full dongle rte dev
 */

#define D11CONF		0x80000000	/* D11 Core Rev 31 */
#define D11CONF2	0	        /* D11 Core Rev > 31 */
#define NCONF		0x400		/* N-Phy Rev 0xa */

#define NTXD		32
#define NRXD		16
#define NRXBUFPOST	10
#define MAXSCB		8
#define WLC_DATAHIWAT	10
#define WLC_AMPDUDATAHIWAT      16
#define RXBND		4
#define WLC_MAXBSSCFG   4
#define AMPDU_NUM_MPDU  8
