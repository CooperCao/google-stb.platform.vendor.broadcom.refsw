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
 * $Id: wltunable_efi.h 357572 2012-09-19 02:00:49Z $
 *
 * wl driver tunables sample
 */

#define D11CONF 0x00000000	/* No Core Rev less than 40 */
#define D11CONF2 0x00030500	/* D11 Core Rev 40 (4360A0), 42(4360B0),
				 * 48 (4350C0), 49 (43602A0)
				 */
#define D11CONF3 0x00000000	/* Core Rev between 64 to 95 */
#define D11CONF4 0x00000000	/* Core Rev between 96 to 127 */
#define D11CONF5 0x00000000	/* Core Rev between 128 to 159 */

#define PHYCONF_DEFAULTS	1	/* Use default settings for all PHYs, except for the
					 * ones listed below which are disabled.
					 */

#define GCONF 0
#define ACONF 0
#define NCONF 0
#define LPCONF 0
#define SSLPNCONF 0
#define LCN20CONF 0		/* No LCN-Phy */
#define HTCONF 0		/* No HT-Phy */
#define ACCONF (0x44303 & ACPHY_DEFAULT)	/* AC-Phy rev 0, 1, 8, 9, 14, 18 */
#define ACCONF2 0

#define MAXSCB	8
#define NRXBUFPOST 128 /* Post more number of buffers to receive as it's a polled environment */
