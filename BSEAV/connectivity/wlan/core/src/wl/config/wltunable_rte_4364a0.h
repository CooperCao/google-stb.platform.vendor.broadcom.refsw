/*
 * Broadcom 802.11ac Networking Device Driver Configuration file for 4364
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wltunable_rte_4364a0.h 498584 2014-08-25 07:59:03Z $:
 *
 * wl driver tunables for RTE dev
 */

#define	D11CONF		0
#define D11CONF2	0x04000000	/* D11 Core Rev 58 */
#define D11CONF3	0
#ifdef WL_DUALMAC_RSDB
#define ACCONF		0xC000000   /* AC-Phy rev 27 */
#else
#define ACCONF		0x4000000   /* AC-Phy rev 27 */
#endif

#ifdef WL_NTXD
#define NTXD                    WL_NTXD
#else
#define NTXD                    64
#endif

#ifdef WL_NRXD
#define NRXD                    WL_NRXD
#else
#define NRXD                    64
#endif

#ifndef NRXBUFPOST
#define	NRXBUFPOST		64
#endif
#ifndef NRXBUFPOST_SMALL
#define NRXBUFPOST_SMALL	16
#endif

#ifndef WLC_DATAHIWAT
#define WLC_DATAHIWAT		32
#endif
#define WLC_AMPDUDATAHIWAT	WLC_DATAHIWAT
#ifdef WL_RXBND
#define RXBND			WL_RXBND
#else
#define RXBND			24
#endif

#ifdef WL_RXBND_SMALL
#define RXBND_SMALL		WL_RXBND_SMALL
#else
#define RXBND_SMALL		12
#endif /* RXBND_SMALL */

#define PKTCBND			RXBND
#define WLC_MAXBSSCFG		8
#define WLC_MAXDPT		1
#define	MAXSCB			(WLC_MAXBSSCFG + WLC_MAXDPT)
#define AIDMAPSZ		32
#define AMPDU_NUM_MPDU		48	/* max allowed number of mpdus in an ampdu */
#ifndef AMPDU_RX_BA_DEF_WSIZE
#define AMPDU_RX_BA_DEF_WSIZE	32	/* Default value to be overridden for dongle */
#endif

#define NTXD_LARGE_AC3X3	NTXD
#define NRXD_LARGE_AC3X3	NRXD
#define RXBND_LARGE_AC3X3	RXBND
#define NRXBUFPOST_LARGE_AC3X3	NRXBUFPOST

#ifdef WL_NTXD_LFRAG
#define NTXD_LFRAG	WL_NTXD_LFRAG
#else
#define NTXD_LFRAG		256
#endif
#define NRXD_CLASSIFIED_FIFO	16
#ifndef NUMD11CORES
#define NUMD11CORES		2
#endif
#ifndef MAXVSIEPARSECBS
#define MAXVSIEPARSECBS 80
#endif
#ifndef WLC_MAXMODULES
#define WLC_MAXMODULES 90
#endif
#ifndef MAXVSIEBUILDCBS
#define MAXVSIEBUILDCBS 80
#endif
#ifdef USE_RSDBAUXCORE_TUNE
#ifndef WL_NRXD_AUX
#define WL_NRXD_AUX WL_NRXD
#endif /* WL_NRXD_AUX */
#ifndef NRXBUFPOST_AUX
#define NRXBUFPOST_AUX	NRXBUFPOST
#endif /* NRXBUFPOST_AUX */
#ifndef WL_RXBND_AUX
#define WL_RXBND_AUX WL_RXBND
#endif /* WL_RXBND_AUX */
#ifndef NTXD_LFRAG_AUX
#define NTXD_LFRAG_AUX NTXD_LFRAG
#endif /* NTXD_LFRAG_AUX */
#endif /* USE_RSDBAUXCORE_TUNE */
/* List objreg strcutures that cannot be shared for 4364 */
#define OBJR_WLC_BANDSTATE_KEY_ENABLE 0 /* Band state cannot be shared */
