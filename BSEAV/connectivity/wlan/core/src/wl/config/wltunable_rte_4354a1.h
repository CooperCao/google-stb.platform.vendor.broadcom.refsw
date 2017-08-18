/*
 * Broadcom 802.11 Networking Device Driver Configuration file for 4350
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wltunable_rte_4354a1.h 417811 2013-08-12 18:27:27Z $:
 *
 * wl driver tunables for RTE dev
 */

#define	D11CONF		0
#define D11CONF2	0x00010000	/* D11 Core Rev 48 */
#define D11CONF3	0
#define ACCONF		0x8000		/* AC-Phy rev 15 */
#define ACCONF2		0

#ifdef WL_NTXD
#define NTXD		WL_NTXD
#else
#define NTXD		64
#endif


#ifdef WL_NRXD
#define NRXD		WL_NRXD
#elif BCMSPLITRX
#define NRXD            128		/* split-rx use 2x descriptor memory */
#else
#define NRXD		64
#endif

#ifndef NRXBUFPOST
#define NRXBUFPOST	12
#endif

#ifndef WLC_DATAHIWAT
#define WLC_DATAHIWAT	32
#endif

#ifdef WL_RXBND
#define RXBND		WL_RXBND
#else
#define RXBND		24
#endif

#define WLC_MAXBSSCFG	8
#define WLC_MAXDPT	1
#define WLC_MAXTDLS	5
#ifdef MINPKTPOOL
#define MAXSCB		64
#else
#define MAXSCB		32 /* (WLC_MAXBSSCFG + WLC_MAXDPT + WLC_MAXTDLS) */
#endif
#define AIDMAPSZ	32 /* Fixed number for now, to avoid the dep on MAXSCB */

#ifndef AMPDU_RX_BA_DEF_WSIZE
#define AMPDU_RX_BA_DEF_WSIZE	32 /* Default value to be overridden for dongle */
#endif
#define PKTCBND		RXBND

#define WLC_AMPDUDATAHIWAT	WLC_DATAHIWAT

#define NTXD_LARGE_AC3X3	NTXD
#define NRXD_LARGE_AC3X3	NRXD
#define RXBND_LARGE_AC3X3	RXBND
#define NRXBUFPOST_LARGE_AC3X3	NRXBUFPOST
#define NTXD_LFRAG       256

#define WLFCFIFOCREDITAC0	9
#define WLFCFIFOCREDITAC1	52
#define WLFCFIFOCREDITAC2	24
#define WLFCFIFOCREDITAC3	10
#define WLFCFIFOCREDITBCMC	5

#define WLFC_FIFO_CR_PENDING_THRESH_AC_BK	2
#if defined(DMATXRC) && !defined(DMATXRC_DISABLED)
#define WLFC_FIFO_CR_PENDING_THRESH_AC_BE	10
#else
#define WLFC_FIFO_CR_PENDING_THRESH_AC_BE	20
#endif
#define WLFC_FIFO_CR_PENDING_THRESH_AC_VI	3
#define WLFC_FIFO_CR_PENDING_THRESH_AC_VO	2

/* Once the credit borrowing happened, credit threshold is adjusted to higher value
	This ratio is used to calcualte new threshold from total credit
*/
#define WLFC_FIFO_BO_CR_RATIO	5

/* IE MGMT tunables */
#define MAXIEREGS	10

#ifdef WL_NAN
/* Customize/increase pre-allocated notification memory pool size */
/* Maximum number of notification servers */
#define MAX_NOTIF_SERVERS 18
/* Maximum number of notification clients */
#define MAX_NOTIF_CLIENTS 30
#endif
