/*
 * Broadcom 802.11ac Networking Device Driver Configuration file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wltunable_rte_43556b1_usbap.h $
 *
 * wl driver tunables for 43556b1 BMAC rte dev
 */

#define D11CONF		0x00000000
#define D11CONF2        0x00000800      /* D11 Core Rev 43 */
#define D11CONF3	0x00000000
#define ACCONF          0x8             /* AC-Phy rev 3 */
#define ACCONF2		0
#define NTXD		128
#define NRXD		128
#define NRXBUFPOST	64
#define WLC_DATAHIWAT	10
#define RXBND		16
#define DNGL_MEM_RESTRICT_RXDMA    (6*2048)
