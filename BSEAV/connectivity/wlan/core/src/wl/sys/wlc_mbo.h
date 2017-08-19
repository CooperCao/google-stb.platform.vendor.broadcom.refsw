/*
 * MBO declarations/definitions for
 * Broadcom 802.11abgn Networking Device Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 *
 */

/**
 * WFA Multiband Operation (MBO) certification program certify features that facilitate
 * efficient use of multiple frequency bands/channels available to Access Points(APs)
 * and wireless devices(STAs) that may associates with them. The prerequisites of the
 * program is that AP and STAs have information which can help in making most effective
 * selection of the spectrum band in which the STA and AP should be communicating.
 * AP and STAs enable each other to make intelligent decisions collectively for more
 * efficient use of the available spectrum by exchanging this information.
 */

#ifndef _wlc_mbo_h_
#define _wlc_mbo_h_

#define WL_MBO_CNT_INR(_m, _ctr) (++((_m)->cntrs->_ctr))
#define MBO_MAX_CHAN_PREF_ENTRIES 16

/* flags to mark MBO ap capability */
#define MBO_FLAG_AP_CELLULAR_AWARE  0x1
/* flag to association attempt even AP is not accepting connection */
#define MBO_FLAG_FORCE_ASSOC_TO_AP  0x2
/* flag to forcefully reject bss transition request from AP */
#define MBO_FLAG_FORCE_BSSTRANS_REJECT  0x4

typedef struct wlc_mbo_chan_pref {
	uint8 opclass;
	uint8 chan;
	uint8 pref;
	uint8 reason;
} wlc_mbo_chan_pref_t;

wlc_mbo_info_t * wlc_mbo_attach(wlc_info_t *wlc);
void wlc_mbo_detach(wlc_mbo_info_t *mbo);
int
wlc_mbo_add_chan_pref(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	wlc_mbo_chan_pref_t *ch_pref);
int
wlc_mbo_del_chan_pref(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	wlc_mbo_chan_pref_t *ch_pref);
int
wlc_mbo_set_cellular_data_cap(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	uint8 cell_data_cap);
int
wlc_mbo_get_cellular_data_cap(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	uint8 *cell_data_cap);
#ifdef WL_MBO_WFA_CERT
int
wlc_mbo_set_bsstrans_reject(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	uint8 enable, uint8 reason);
int
wlc_mbo_get_bsstrans_reject(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	uint8 *enable, uint8 *reason);
#endif /* WL_MBO_WFA_CERT */
#endif	/* _wlc_mbo_h_ */
