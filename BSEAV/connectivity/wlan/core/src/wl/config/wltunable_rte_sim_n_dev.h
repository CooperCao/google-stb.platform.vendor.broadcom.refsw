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
 * wl driver tunables for RTESim AG dev
 */

#define	D11CONF		0x80		/* D11 Core Rev 7 */
#define D11CONF2	0		/* D11 Core Rev > 31 */
#define	GCONF		0x80		/* G-Phy Rev 6 */
#define	ACONF		0x80		/* A-Phy Rev 6 */
#define NCONF		1		/* N-Phy Rev 0 */

#define	NRXBUFPOST	4
#define	MAXSCB		8
#define WLC_DATAHIWAT	20
#define RXBND		1
#define TXSBND		1
