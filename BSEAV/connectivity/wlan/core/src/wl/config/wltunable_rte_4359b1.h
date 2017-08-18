/*
 * Broadcom 802.11ac Networking Device Driver Configuration file for 4359
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wltunable_rte_4359b1.h 523692 2015-01-01 09:42:50Z $
 *
 * wl driver tunables for RTE dev
 */

#define	D11CONF		0
#define D11CONF2	0x08000000	/* D11 Core Rev 59 */
#define ACCONF		0x1000000   /* AC-Phy rev 24 (0x18) */


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
#define NRXBUFPOST_SMALL       16
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
#define RXBND_SMALL    WL_RXBND_SMALL
#else
#define RXBND_SMALL            12
#endif /* RXBND_SMALL */
#define PKTCBND			RXBND

#define WLC_MAXBSSCFG		8
#define WLC_MAXDPT		1
#define	MAXSCB			(WLC_MAXBSSCFG + WLC_MAXDPT + 2)
#define AIDMAPSZ		32
#ifdef WL_AMPDU_NUM_MPDU
#define AMPDU_NUM_MPDU		WL_AMPDU_NUM_MPDU
#else
#define AMPDU_NUM_MPDU		48	/* max allowed number of mpdus in an ampdu */
#endif

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

#define NRXD_CLASSIFIED_FIFO	8

#ifndef NUMD11CORES
#define NUMD11CORES		2
#endif

#ifndef WLC_RSDB_PER_CORE_TXCHAIN
#define WLC_RSDB_PER_CORE_TXCHAIN	1
#endif

#ifndef WLC_RSDB_PER_CORE_RXCHAIN
#define WLC_RSDB_PER_CORE_RXCHAIN	1
#endif


#ifndef MAXVSIEBUILDCBS
#define MAXVSIEBUILDCBS 80
#endif

#ifndef MAXVSIEPARSECBS
#define MAXVSIEPARSECBS 64
#endif

#ifndef WLC_MAXMODULES
#define WLC_MAXMODULES	72	/* max #  wlc_module_register() calls */
#endif
