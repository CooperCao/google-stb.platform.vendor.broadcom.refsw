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
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 *
 * wl driver tunables for Mac OS
 */
#define D11CONF		0x24800000 	/* D11 Core Rev 23 (432224B0), 26(4331A0), 29(4331B0) */

#define D11CONF2	0x00010f00	/* D11 Core Rev 40 (4360A0), 41 (4335A0), 42(4360B0)
					 * 43 (4350A0), 48 (4350C0)
					 */
#define D11CONF3	0x00000000	/* Core Rev between 64 to 95 */
#define D11CONF4	0x00000000	/* Core Rev between 96 to 127 */
#define D11CONF5	0x00000000	/* Core Rev between 128 to 159 */

#define PHYCONF_DEFAULTS	1	/* Use default settings for all PHYs, except for the
					 * ones listed below which are disabled.
					 */

#define LPCONF		0		/* No LP-Phy */
#define SSLPNCONF	0		/* No SSLPN-Phy */

#define MAXBSS		128		/* max # available networks */

#define WLC_DATAHIWAT   500
#define NRXBUFPOST      240

#define NTXD_LARGE	2048		/* TX descriptor ring */

#define WLC_DEFAULT_SETTLE_TIME 10

#define WLC_UC_TXS_HIST_LEN 100         /* TxStatus history length */
#define WLC_UC_TXS_TIMESTAMP            /* configure timestamps */
