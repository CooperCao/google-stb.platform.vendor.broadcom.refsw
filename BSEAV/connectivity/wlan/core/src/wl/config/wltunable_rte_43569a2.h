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
 * $Id: wltunable_rte_43569a2.h 417811 2013-08-12 18:27:27Z $:
 *
 * wl driver tunables for RTE dev
 */

#define	D11CONF		0
#define D11CONF2	0x00010000	/* D11 Core Rev 48 */
#define	D11CONF3	0
#define ACCONF		0x20000		/* AC-Phy rev 17 */
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
#define MAXSCB		32 /*(WLC_MAXBSSCFG + WLC_MAXDPT + WLC_MAXTDLS)*/
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
#define NTXD_LFRAG       1024

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

/* IE MGMT tunables */
#define MAXIEREGS			8

#define TX_AC_BK_FIFO_REDUCTION_FACTOR 16
#define TX_AC_VO_FIFO_REDUCTION_FACTOR 4
#define TX_BCMC_FIFO_REDUCTION_FACTOR  16
#define TX_ATIM_FIFO_REDUCTION_FACTOR  16
#define AMPDU_NUM_MPDU	32 /* AMPDU Aggregation default number */
