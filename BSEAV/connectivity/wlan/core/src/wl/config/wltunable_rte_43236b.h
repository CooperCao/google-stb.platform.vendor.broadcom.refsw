/*
 * Broadcom 802.11abg Networking Device Driver Configuration file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wltunable_rte_43236b1.h 241182 2011-02-17 21:50:03Z $
 *
 * wl driver tunables for 43236b1 full dongle rte dev
 */

#define D11CONF		0x1000000	/* D11 Core Rev 24 */
#define D11CONF2	0		/* No D11 Core Rev > 32 */
#define GCONF		0		/* No G-Phy */
#define ACONF		0		/* No A-Phy */
#define HTCONF		0		/* No HT-Phy */
#define NCONF		0x200		/* N-Phy Rev 9 */
#define LPCONF		0		/* No LP-Phy */
#define SSLPNCONF	0		/* No SSLPN-Phy */
#define NTXD		128
#define NRXD		32
#define NRXBUFPOST	16
#define WLC_DATAHIWAT	10
#define RXBND		16
#define WLC_MAXBSSCFG	8
#define	MAXSCB		(WLC_MAXBSSCFG )
#define AIDMAPSZ	32
