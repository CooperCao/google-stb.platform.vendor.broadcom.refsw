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
 * $Id$:
 *
 * wl driver tunables for RTE dev
 */

#define	D11CONF		0
#define D11CONF2	0x00010000	/* D11 Core Rev 48 */
#define	D11CONF3	0
#define ACCONF		0x100		/* AC-Phy rev 8 */
#define ACCONF2		0

#define NTXD		64
#define NRXD		64
#ifndef NRXBUFPOST
#define NRXBUFPOST	12
#endif
#define WLC_DATAHIWAT	32
#define RXBND		24
#define WLC_MAXBSSCFG	8
#define WLC_MAXTDLS	5
#ifdef MINPKTPOOL
#define MAXSCB		64
#else
#define MAXSCB		31 /*(WLC_MAXBSSCFG + WLC_MAXTDLS)*/
#endif
#define AIDMAPSZ	32

#ifndef AMPDU_RX_BA_DEF_WSIZE
#define AMPDU_RX_BA_DEF_WSIZE	32 /* Default value to be overridden for dongle */
#endif
#define PKTCBND		RXBND

#define WLC_AMPDUDATAHIWAT	WLC_DATAHIWAT

#define NTXD_LARGE_AC3X3	NTXD
#define NRXD_LARGE_AC3X3	NRXD
#define RXBND_LARGE_AC3X3	RXBND
#define NRXBUFPOST_LARGE_AC3X3	NRXBUFPOST

#define WLFCFIFOCREDITAC0	9
#define WLFCFIFOCREDITAC1	39
#define WLFCFIFOCREDITAC2	18
#define WLFCFIFOCREDITAC3	9
#define WLFCFIFOCREDITBCMC	9

#define WLFC_FIFO_CR_PENDING_THRESH_AC_BK	2
#if defined(DMATXRC) && !defined(DMATXRC_DISABLED)
#define WLFC_FIFO_CR_PENDING_THRESH_AC_BE	10
#else
#define WLFC_FIFO_CR_PENDING_THRESH_AC_BE	20
#endif
#define WLFC_FIFO_CR_PENDING_THRESH_AC_VI	3
#define WLFC_FIFO_CR_PENDING_THRESH_AC_VO	2
#define NRXD_FIFO1      32
