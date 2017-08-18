/*
 * Beacon Trim  module interface, 802.1x related.
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
 */

#ifndef __wlc_bcntrim_h__
#define __wlc_bcntrim_h__

extern wlc_bcntrim_info_t *wlc_bcntrim_attach(wlc_info_t *wlc);
extern void wlc_bcntrim_detach(wlc_bcntrim_info_t *bcntrim_info);
extern void wlc_bcntrim_update_bcntrim_enab(wlc_bcntrim_info_t *bcntrim_info, wlc_bsscfg_t * bsscfg,
	uint32 new_mc);
extern void wlc_bcntrim_recv_process_partial_beacon(wlc_bcntrim_info_t *bcntrim_info);
#endif /* _wlc_bcntrim_h_ */
