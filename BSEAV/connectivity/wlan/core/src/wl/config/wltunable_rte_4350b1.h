/*
 * Broadcom 802.11 Networking Device Driver Configuration file for 4350
 *
 * Copyright (C) 2012, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wltunable_rte_4350b1.h$
 *
 * wl driver tunables for RTE dev
 */

#define	D11CONF		0
#define	D11CONF2	0x00000800	/* D11 Core Rev 43 */
#define	D11CONF3	0
#define	GCONF		0		/* No G-Phy */
#define ACONF		0		/* No A-Phy */
#define HTCONF		0		/* No HT-Phy */
#define NCONF		0		/* No N-Phy */
#define LPCONF		0		/* No LP-Phy */
#define SSLPNCONF	0		/* No SSLPN-Phy */
#define LCN20CONF	0		/* No LCN-Phy */
#define ACCONF		0x8		/* AC-Phy rev 3 */
#define ACCONF2		0
#define NTXD			32
#define NRXD			8
#ifndef NRXBUFPOST
#define	NRXBUFPOST		4
#endif
#define WLC_DATAHIWAT	10
#define RXBND			4
#define WLC_MAXBSSCFG	8
#define WLC_MAXTDLS		1
#define	MAXSCB			(WLC_MAXBSSCFG + WLC_MAXTDLS)
#define AIDMAPSZ		32

#define AMPDU_RX_BA_DEF_WSIZE	16	/* Default value to be overridden for dongle */
