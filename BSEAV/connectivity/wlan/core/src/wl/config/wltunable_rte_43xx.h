/*
 * Broadcom 802.11abg Networking Device Driver Configuration file for 4330
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wltunable_rte_43xx.h,v 1.7.2.2 2011-01-25 02:15:39 $:
 *
 * wl driver tunables for RTE dev
 */

#define	D11CONF		0
#define	D11CONF2	0x00000002	/* D11 Core Rev 33 */
#define NCONF		0x80000		/* N-Phy Rev19(LCN 2x2 Rev3) */

#define NTXD			32
#define NRXD			32
#define	NRXBUFPOST		4
#define WLC_DATAHIWAT		4
#define RXBND			4
#define WLC_MAXBSSCFG		8
#define	MAXSCB			(WLC_MAXBSSCFG)
#define AIDMAPSZ	32
#define WLC_AMPDUDATAHIWAT	15
#define AMPDU_NUM_MPDU		8

#define AMPDU_RX_BA_DEF_WSIZE	16	/* Default value to be overridden for dongle */
#define NAN_MAX_PEERS	16 /* default NAN peers supported */
#ifdef WLRSDB
#define NAN_MAX_AVAIL 2
#else
#define NAN_MAX_AVAIL 1
#endif /* WLRSDB */
#define NAN_MAX_NDC	4
