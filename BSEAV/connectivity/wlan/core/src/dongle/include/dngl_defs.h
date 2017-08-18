/*
 * RTE DONGLE Defines
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * <<Broadcom-WL-IPTag/Open:>>  
 *
 * $Id$
 */

#ifndef _dngl_defs_h_
#define _dngl_defs_h_

/* Enumerating top level SW entities for use by health check */
typedef enum {
	HCHK_SW_ENTITY_UNDEFINED = 0,
	HCHK_SW_ENTITY_PCIE = 1,
	HCHK_SW_ENTITY_SDIO = 2,
	HCHK_SW_ENTITY_USB = 3,
	HCHK_SW_ENTITY_RTE = 4,
	HCHK_SW_ENTITY_WL_PRIMARY = 5, /* WL instance 0 */
	HCHK_SW_ENTITY_WL_SECONDARY = 6, /* WL instance 1 */
	HCHK_SW_ENTITY_MAX
} hchk_sw_entity_t;
#endif /* _dngl_defs_h_ */
