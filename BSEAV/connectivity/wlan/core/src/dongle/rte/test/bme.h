/*
 * Byte Move Engine definitions
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

#ifndef _BME_H
#define _BME_H

#include "arms_43602.h"

#define BME_CAP_REG			(REGBASE_SOCSRAM + 0x08)

#define BME_SRC_ADDR0		(REGBASE_SOCSRAM + 0x60)
#define BME_DST_ADDR0		(REGBASE_SOCSRAM + 0x64)
#define BME_SRC_ADDR1		(REGBASE_SOCSRAM + 0x70)
#define BME_DST_ADDR1		(REGBASE_SOCSRAM + 0x74)
#define BME_STATUS			(REGBASE_SOCSRAM + 0x7c)

#define BME_BUSY0			0x00000001
#define BME_RDERR0			0x00000002
#define BME_WRERR0			0x00000004
#define BME_4KPGX0			0x00000008
#define BME_BUSY1			0x00000100
#define BME_RDERR1			0x00000200
#define BME_WRERR1			0x00000400
#define BME_4KPGX1			0x00000800

#define BME_SRC_SET(src, len, reg) (*((uint32 *)(reg)) = \
	((src) & 0xffffff) | (((len) & 0xf80) << 17))
#define BME_DST_SET(z, dst, len, reg) (*((uint32 *)(reg)) = \
	((dst) & 0xffffff) | (((len) & 0x7f) << 24) | ((z) << 31))

#define BME_SRC_SET0(src, len) BME_SRC_SET(src, len, BME_SRC_ADDR0)
#define BME_SRC_SET1(src, len) BME_SRC_SET(src, len, BME_SRC_ADDR1)
#define BME_DST_SET0(z, dst, len) BME_DST_SET(z, dst, len, BME_DST_ADDR0)
#define BME_DST_SET1(z, dst, len) BME_DST_SET(z, dst, len, BME_DST_ADDR1)


#define BME_STAT_BUSY0		((ACCESS(BME_STATUS)) & BME_BUSY0)
#define BME_STAT_RDERR0		((ACCESS(BME_STATUS)) & BME_RDERR0)
#define BME_STAT_WRERR0		((ACCESS(BME_STATUS)) & BME_WRERR0)
#define BME_STAT_4KPGX0		((ACCESS(BME_STATUS)) & BME_4KPGX0)
#define BME_STAT_BUSY1		((ACCESS(BME_STATUS)) & BME_BUSY1)
#define BME_STAT_RDERR1		((ACCESS(BME_STATUS)) & BME_RDERR1)
#define BME_STAT_WRERR1		((ACCESS(BME_STATUS)) & BME_WRERR1)
#define BME_STAT_4KPGX1		((ACCESS(BME_STATUS)) & BME_4KPGX1)


#define BME_TRANSFER_REQ0(src, dst, len)	{ \
	BME_SRC_SET0(src, len); \
	BME_DST_SET0(0, dst, len); }
#define BME_TRANSFER_REQ1(src, dst, len)	{ \
	BME_SRC_SET1(src, len); \
	BME_DST_SET1(0, dst, len); }

#define BME_ZEROSET_REQ0(dst, len)	{ \
	BME_SRC_SET0(0, len); \
	BME_DST_SET0(1, dst, len); }
#define BME_ZEROSET_REQ1(dst, len)	{ \
	BME_SRC_SET1(0, len); \
	BME_DST_SET1(1, dst, len); }

void bme_transfer(bme_params_t *bme_params, volatile arm_stat_t *stat);

#endif /* _BME_H */
