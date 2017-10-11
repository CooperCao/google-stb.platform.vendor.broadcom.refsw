/*
 * Broadcom 802.11abgn Networking Device Driver Configuration file for 43242
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wltunable_rte_43242a0.h 256691 2011-05-02 07:04:15Z $:
 *
 * wl driver tunables for RTE dev
 */

#define	D11CONF			0
#define	D11CONF2		0x00000020	/* D11 Core Rev 37 */
#define	D11CONF3		0
#define	GCONF			0		/* No G-Phy */
#define ACONF			0		/* No A-Phy */
#define HTCONF			0		/* No HT-Phy */
#define NCONF			0x100000	/* N-Phy Rev20(LCN 2x2 Rev4) */
#define LPCONF			0		/* No LP-Phy */
#define SSLPNCONF		0		/* No SSLPN-Phy */

#define NTXD			32
#define NRXD			64
#ifndef NRXBUFPOST
#define	NRXBUFPOST		32
#endif
#define WLC_DATAHIWAT		10
#define RXBND			16
#define WLC_MAXBSSCFG		8
#define WLC_MAXTDLS		1
#define	MAXSCB			10
/* Should be at least WLC_MAXBSSCFG */
#if (MAXSCB < WLC_MAXBSSCFG )
#error MAXSCB definition too small
#endif
#if (MAXSCB < WLC_MAXBSSCFG + WLC_MAXTDLS)
#error MAXSCB definition too small
#endif
#define AIDMAPSZ		32

#define AMPDU_BA_MAX_WSIZE	64	/* Used to max size arrays in structures */

#define AMPDU_TX_BA_MAX_WSIZE	64	/* Max TX BA window size; power of 2; attach time tunable */
#define AMPDU_TX_BA_DEF_WSIZE	64	/* Default value for TX BA window size */

#define AMPDU_RX_BA_MAX_WSIZE	32	/* Max RX BA window size; power of 2; attach time tunable */
#define AMPDU_RX_BA_DEF_WSIZE	32	/* Default value for RX BA window size */

#define WLFC_FIFO_CR_PENDING_THRESH_AC_BE 8

#define WLFCFIFOCREDITAC0	8
#define WLFCFIFOCREDITAC1	42
#define WLFCFIFOCREDITAC2	8
#define WLFCFIFOCREDITAC3	4
#define WLFCFIFOCREDITBCMC	2
