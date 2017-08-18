/*
 * Chip-specific hardware definitions for
 * Broadcom 802.11abg Networking Device Driver
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
 * $Id: debug_dump_util.h 628946 2016-04-01 18:02:49Z $
 */

#ifndef	_DBG_DUMP_H
#define	_DBG_DUMP_H

#define TAG_TYPE_CHIPC		0
#define TAG_TYPE_MAC		1
#define TAG_TYPE_UTRACE		2
#define TAG_TYPE_PCIE		3
#define TAG_TYPE_LHL		4
#define TAG_TYPE_PC		5
#define TAG_TYPE_PMU		6
#define TAG_TYPE_CHIPC_INDX	0xE0 /* 224 */
#define TAG_TYPE_PHY		0xf0 /* 240 */
#define TAG_TYPE_RAD		0xf1 /* 241 */

#define UCODE_CORE_ID(core)	(core+1)
#define PC_DUMP_DEPTH		64


#endif	/* _DBG_DUMP_H */
