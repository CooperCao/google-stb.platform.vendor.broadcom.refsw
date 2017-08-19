/*
 * Broadcom 802.11abg Networking Device Driver Configuration file for 43430
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wltunable_rte_43430a1.h eugenep $:
 *
 * wl driver tunables for RTE dev
 */

#define	D11CONF		0
#define	D11CONF2	0x00000080	/* D11 Core Rev 39 */
#define	D11CONF3	0
#define GCONF           0               /* No G-Phy */
#define HTCONF          0               /* No HT-Phy */
#define NCONF           0               /* No N-Phy */
#define LPCONF          0               /* No LP-Phy */
#define SSLPNCONF       0               /* No SSLPN-Phy */
#define LCN20CONF       1               /* LCN20-Phy rev 1 */
#define ACCONF          0               /* No AC-Phy */
#define NTXD			64
#define NRXD			32
#ifndef NRXBUFPOST
#define	NRXBUFPOST		12
#endif
#define WLC_DATAHIWAT	20
#define RXBND			24
#define WLC_MAXBSSCFG	8
#define WLC_MAXDPT		1
#define WLC_MAXTDLS		1
#ifdef MINPKTPOOL
#define	MAXSCB			64
#else
#define	MAXSCB			32 /* (WLC_MAXBSSCFG + WLC_MAXDPT + WLC_MAXTDLS) */
#endif
#define AIDMAPSZ		32

#ifndef AMPDU_RX_BA_DEF_WSIZE
#define AMPDU_RX_BA_DEF_WSIZE	32	/* Default value to be overridden for dongle */
#endif
#define PKTCBND	RXBND

#define AMPDU_NUM_MPDU            8

#define WLFCFIFOCREDITAC0 9
#define WLFCFIFOCREDITAC1 39
#define WLFCFIFOCREDITAC2 18
#define WLFCFIFOCREDITAC3 9
#define WLFCFIFOCREDITBCMC 9

#define WLFC_FIFO_CR_PENDING_THRESH_AC_BK 2
#define WLFC_FIFO_CR_PENDING_THRESH_AC_BE 20
#define WLFC_FIFO_CR_PENDING_THRESH_AC_VI 3
#define WLFC_FIFO_CR_PENDING_THRESH_AC_VO 2
