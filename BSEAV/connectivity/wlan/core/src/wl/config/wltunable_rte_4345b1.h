/*
 * Broadcom 802.11abg Networking Device Driver Configuration file for 4345
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id:
 *
 * wl driver tunables for RTE dev
 */

#define	D11CONF		0
#define	D11CONF2	0x00080000	/* D11 Core Rev 51 */
#define	D11CONF3	0
#define ACCONF		0x2000		/* AC-Phy rev 13 */
#define ACCONF2		0
#define NTXD			64
#define NRXD			32
#ifndef NRXBUFPOST
#define	NRXBUFPOST		12
#endif
#define WLC_DATAHIWAT	10
#define RXBND			12
#define WLC_MAXBSSCFG	8
#define WLC_MAXTDLS		1
#define	MAXSCB			(WLC_MAXBSSCFG + WLC_MAXTDLS)
#define AIDMAPSZ		32

#define AMPDU_RX_BA_DEF_WSIZE	16	/* Default value to be overridden for dongle */
#define NTXD_LFRAG	256
