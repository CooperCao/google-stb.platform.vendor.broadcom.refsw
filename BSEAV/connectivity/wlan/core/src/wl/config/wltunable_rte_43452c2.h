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
#define	D11CONF2	0x00400000	/* D11 Core Rev 54 */
#define D11CONF3	0
#define ACCONF		0x100000	/* AC-Phy rev 20 */
#define ACCONF2		0
#ifdef WL_NTXD
#define NTXD		WL_NTXD
#else
#define NTXD		64
#endif

#ifdef WL_NRXD
#define NRXD		WL_NRXD
#else
#define NRXD		32
#endif

#ifndef NRXBUFPOST
#define	NRXBUFPOST		12
#endif
#define WLC_DATAHIWAT	10

#ifdef WL_RXBND
#define RXBND		WL_RXBND
#else
#define RXBND			12
#endif

#define WLC_MAX_UCODE_BSS       4       /* Max BSS supported */
#define WLC_MAXBSSCFG	8
#define WLC_MAXDPT		1
#define WLC_MAXTDLS		1
#define	MAXSCB			32 /* (WLC_MAXBSSCFG + WLC_MAXDPT + WLC_MAXTDLS) */
#define AIDMAPSZ		32
#ifndef AMPDU_RX_BA_DEF_WSIZE
#define AMPDU_RX_BA_DEF_WSIZE	16	/* Default value to be overridden for dongle */
#endif
#define PKTCBND	RXBND
#define NTXD_LFRAG	256

/* Customize/increase pre-allocated notification memory pool size */
/* Maximum number of notification servers */
#define MAX_NOTIF_SERVERS 64
/* Maximum number of notification clients */
#define MAX_NOTIF_CLIENTS 100
