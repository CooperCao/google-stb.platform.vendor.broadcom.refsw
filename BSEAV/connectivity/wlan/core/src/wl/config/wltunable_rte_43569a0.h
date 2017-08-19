/*
 * Broadcom 802.11 Networking Device Driver Configuration file for 43569
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wltunable_rte_43569a0.h 417811 2013-08-12 18:27:27Z $:
 *
 * wl driver tunables for RTE dev
 */

#define	D11CONF		0
#define D11CONF2	0x00010000	/* D11 Core Rev 48 */
#define	D11CONF3	0
#define ACCONF		0x8000		/* AC-Phy rev 15 */
#define ACCONF2		0

#define NTXD		64
#define NRXD		64
#ifndef NRXBUFPOST
#define NRXBUFPOST	6
#endif
#define WLC_DATAHIWAT	16
#define RXBND		16
#define WLC_MAXBSSCFG	8
#define WLC_MAXTDLS	5
#ifdef MINPKTPOOL
#define MAXSCB		64
#else
#define MAXSCB		31 /*(WLC_MAXBSSCFG + WLC_MAXTDLS)*/
#endif
#define AIDMAPSZ	16

#ifndef AMPDU_RX_BA_DEF_WSIZE
#define AMPDU_RX_BA_DEF_WSIZE	16 /* Default value to be overridden for dongle */
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
