/*
 * Assoc Mgr interfaces
 *
 * This file contains the code specific to the NDIS WDI Driver model.
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
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

#ifndef _WLC_ASSOC_MGR_H_
#define _WLC_ASSOC_MGR_H_
#ifdef WL_ASSOC_MGR
extern wlc_assoc_mgr_info_t *
wlc_assoc_mgr_attach(wlc_info_t *wlc);

extern void
wlc_assoc_mgr_detach(wlc_assoc_mgr_info_t *am_info);

extern bool
wlc_assoc_mgr_pause_on_auth_resp(wlc_assoc_mgr_info_t *am_info, wlc_bsscfg_t *bsscfg,
	struct ether_addr* addr, uint auth_status, uint auth_type);

extern void
wlc_assoc_mgr_save_auth_resp(wlc_assoc_mgr_info_t *am_info, wlc_bsscfg_t *bsscfg,
	uint8 *auth_resp, uint auth_resp_len);

#endif /* WL_ASSOC_MGR */
#endif /* _WLC_ASSOC_MGR_H_ */
