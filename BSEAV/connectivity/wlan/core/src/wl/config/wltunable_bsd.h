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
 * wl driver BSD tunables
 */

#define D11CONF		0x77800000	/* D11 Core Rev 
					 * 23 (43224b0), 24 (4313), 25 (5357a0), 26 (4331a0),
					 * 28 (5357b0), 29 (4331B0), 30(43228).
					 */
#define D11CONF2	0x500		/* D11 Core Rev > 31, Rev 40(4360a0), 42(4360B0) */

#define NRXBUFPOST	56	/* # rx buffers posted */
#define RXBND		24	/* max # rx frames to process */
#define PKTCBND		36	/* max # rx frames to chain */
#define CTFPOOLSZ       192	/* max buffers in ctfpool */

#define WME_PER_AC_TX_PARAMS 1
#define WME_PER_AC_TUNING 1

#define NTXD_AC3X3		512	/* TX descriptor ring */
#define NRXD_AC3X3		512	/* RX descriptor ring */
#define NTXD_LARGE_AC3X3	2048	/* TX descriptor ring */
#define NRXD_LARGE_AC3X3	2048	/* RX descriptor ring */
#define NRXBUFPOST_AC3X3	320	/* # rx buffers posted */
#define RXBND_AC3X3		36	/* max # rx frames to process */
#define CTFPOOLSZ_AC3X3		512	/* max buffers in ctfpool */
#define PKTCBND_AC3X3		48	/* max # rx frames to chain */

#define TXMR			2	/* number of outstanding reads */
#define TXPREFTHRESH		8	/* prefetch threshold */
#define TXPREFCTL		16	/* max descr allowed in prefetch request */
#define TXBURSTLEN		256	/* burst length for dma reads */

#define RXPREFTHRESH		1	/* prefetch threshold */
#define RXPREFCTL		8	/* max descr allowed in prefetch request */
#define RXBURSTLEN		256	/* burst length for dma writes */

#define MRRS			512	/* Max read request size */

#define AMPDU_PKTQ_LEN          1536
#define AMPDU_PKTQ_FAVORED_LEN  4096

#define ACCONF	0x000007f	/* supported acphy revs:
				 * 0	4360a0
				 * 1	4360b0
				 * 2	4335a0
				 * 3	4350a0/b0/b1
				 * 4	4345tc
				 * 5	4335b0
				 * 6	4335c0
				 */
#define HTCONF	0x00000003	/* Supported htphy revs:
				 *	0	4331a0
				 *	1	4331a1
				 */
