/*
 * Broadcom driver for Linux User Mode SDIO access
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
 */

#ifndef _UMSD_H
#define _UMSD_H

#define UMSD_FUNC_MAX		8
#define UMSD_CLKDIV_MAX		256

/* UMSD ioctls */
#define UMSD_SDEVSIG		_IOW(0xc1, 0x00, unsigned int)
#define UMSD_GDEVSIG		_IOR(0xc1, 0x01, unsigned int)
#define UMSD_SPOWER		_IOW(0xc1, 0x10, unsigned int)
#define UMSD_GPOWER		_IOR(0xc1, 0x11, unsigned int)
#define UMSD_SCLKBASE		_IOW(0xc1, 0x12, unsigned int)
#define UMSD_GCLKBASE		_IOW(0xc1, 0x13, unsigned int)
#define UMSD_SCLKDIV		_IOW(0xc1, 0x14, unsigned int)
#define UMSD_GCLKDIV		_IOR(0xc1, 0x15, unsigned int)
#define UMSD_SWIDTH		_IOW(0xc1, 0x16, unsigned int)
#define UMSD_GWIDTH		_IOR(0xc1, 0x17, unsigned int)
#define UMSD_SHISPEED		_IOW(0xc1, 0x18, unsigned int)
#define UMSD_GHISPEED		_IOR(0xc1, 0x19, unsigned int)
#define UMSD_SBLKSIZE		_IOW(0xc1, 0x1a, unsigned short [UMSD_FUNC_MAX])
#define UMSD_GBLKSIZE		_IOR(0xc1, 0x1b, unsigned short [UMSD_FUNC_MAX])
#define UMSD_REQCANCEL		_IOW(0xc1, 0x20, umsd_req_id_t)
#define UMSD_REQREAD		_IOWR(0xc1, 0x21, umsd_req_t)
#define UMSD_REQWRITE		_IOWR(0xc1, 0x22, umsd_req_t)

#define UMSD_REQ_RAW		(1U << 0)	/* CMD52 read-after-write mode */
#define UMSD_REQ_BLKMODE	(1U << 1)	/* Block mode (vs byte mode) */
#define UMSD_REQ_FIFO		(1U << 2)	/* FIFO mode (non-incrementing address) */

typedef unsigned int umsd_req_id_t;	/* Non-zero request ID */

typedef struct {
    unsigned int cmd;		/* SDIO command number */
    unsigned int func;		/* SDIO function number */
    unsigned int addr;		/* SDIO register address */
    unsigned int arg;		/* Command argument, or write data for CMD52 */
    unsigned int flags;		/* Zero or more bits from UMSD_REQ_xxx */
    void *buf;			/* Transmit/receive buffer (CMD53 only) */
    unsigned int count;		/* Number of bytes or blocks */
    umsd_req_id_t id;		/* Unique request ID (used for canceling request) */
    unsigned int resp;		/* Location into which SDIO response word will be
				 *    written upon completion
				 */
} umsd_req_t;

#endif	/* _UMSD_H */
