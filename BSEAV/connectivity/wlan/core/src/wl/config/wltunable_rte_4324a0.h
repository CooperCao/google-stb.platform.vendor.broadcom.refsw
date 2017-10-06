/*
 * Broadcom 802.11abg Networking Device Driver Configuration file for 4324
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$:
 *
 * wl driver tunables for RTE dev
 */

#define	D11CONF		0
#define	D11CONF2	0x00000004	/* D11 Core Rev 34 */
#define NCONF		0x80000		/* N-Phy Rev19(LCN 2x2 Rev3) */

#define NTXD			32
#define NRXD			8
#ifndef NRXBUFPOST
#define	NRXBUFPOST		4
#endif
#define WLC_DATAHIWAT	10
#define RXBND			4
#define WLC_MAXBSSCFG	8
#define	MAXSCB			(WLC_MAXBSSCFG )
#define AIDMAPSZ		32

#define AMPDU_RX_BA_DEF_WSIZE	16	/* Default value to be overridden for dongle */
