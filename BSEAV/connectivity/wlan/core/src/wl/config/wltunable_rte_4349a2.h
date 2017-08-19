/*
 * Broadcom 802.11ac Networking Device Driver Configuration file for 4349
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wltunable_rte_4349a0.h 475239 2014-05-04 14:07:13Z $:
 *
 * wl driver tunables for RTE dev
 */

#define	D11CONF		0
#define D11CONF2	0x00040000	/* D11 Core Rev 50 */
#define	D11CONF3	0
#define ACCONF		0x1000   /* AC-Phy rev 12 */
#define ACCONF2		0

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
#define	NRXBUFPOST		4
#endif
#define WLC_DATAHIWAT		10
#define WLC_AMPDUDATAHIWAT	15
#define RXBND			4
#define WLC_MAXBSSCFG		8
#define WLC_MAXDPT		1
#define	MAXSCB			(WLC_MAXBSSCFG + WLC_MAXDPT)
#define AIDMAPSZ		32
#define AMPDU_NUM_MPDU		16	/* max allowed number of mpdus in an ampdu */
#define NTXD_LFRAG		128
#ifndef AMPDU_RX_BA_DEF_WSIZE
#define AMPDU_RX_BA_DEF_WSIZE	8	/* Default value to be overridden for dongle */
#endif

#define NTXD_LARGE_AC3X3	NTXD
#define NRXD_LARGE_AC3X3	NRXD
#define RXBND_LARGE_AC3X3	RXBND
#define NRXBUFPOST_LARGE_AC3X3	NRXBUFPOST

#ifndef NUMD11CORES
#define NUMD11CORES		2
#endif

#ifndef WLC_RSDB_PER_CORE_TXCHAIN
#define WLC_RSDB_PER_CORE_TXCHAIN	1
#endif

#ifndef WLC_RSDB_PER_CORE_RXCHAIN
#define WLC_RSDB_PER_CORE_RXCHAIN	1
#endif

#define MAXBSSCFGCUBBIES	40	/* max number of cubbies in bsscfg container */

#define NAN_MAX_PEERS 16 /* default max NAN peers supported */
