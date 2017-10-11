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
 * $Id$
 *
 * wl driver tunables for RTE dev
 */

#define D11CONF		0xc000		/* D11 Core Rev 14 (4325A0/A1), 15 (4325B0) */
#define D11CONF2	0		/* D11 Core Rev > 31 */
#define LPCONF		0xc		/* LP-Phy Rev 2, 3 */

#define NTXD		32
#define NRXD		16
#define	NRXBUFPOST	4
#define	MAXSCB		8
#define WLC_DATAHIWAT	10
#define RXBND		4

#define WME_PER_AC_TX_PARAMS 1
#define WME_PER_AC_TUNING 1
