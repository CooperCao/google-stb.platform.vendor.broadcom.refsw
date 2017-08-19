/*
 * Broadcom 802.11abg Networking Device Driver Configuration file for 4335
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wltunable_rte_4335a0.h 256691 2011-05-02 07:04:15Z $:
 *
 * wl driver tunables for RTE dev
 */

#define	D11CONF		0
#define	D11CONF2	0x00000200	/* D11 Core Rev 41 */
#define D11CONF3	0
#define ACCONF		0x4		/* AC-Phy rev 2 */
#define ACCONF2		0
#define NTXD			64
#define NRXD			32
#ifndef NRXBUFPOST
#define	NRXBUFPOST		12
#endif
#define WLC_DATAHIWAT	20
#define RXBND			24
#define WLC_MAXBSSCFG	8
#define WLC_MAXTDLS		1
#ifdef MINPKTPOOL
#define	MAXSCB			64
#else
#define	MAXSCB			31 /* (WLC_MAXBSSCFG + WLC_MAXTDLS) */
#endif
#define AIDMAPSZ		32

#ifndef AMPDU_RX_BA_DEF_WSIZE
#define AMPDU_RX_BA_DEF_WSIZE	32	/* Default value to be overridden for dongle */
#endif
#define PKTCBND	RXBND

#define WLFCFIFOCREDITAC0 9
#define WLFCFIFOCREDITAC1 39
#define WLFCFIFOCREDITAC2 18
#define WLFCFIFOCREDITAC3 9
#define WLFCFIFOCREDITBCMC 9

#define WLFC_FIFO_CR_PENDING_THRESH_AC_BK 2
#define WLFC_FIFO_CR_PENDING_THRESH_AC_BE 20
#define WLFC_FIFO_CR_PENDING_THRESH_AC_VI 3
#define WLFC_FIFO_CR_PENDING_THRESH_AC_VO 2
