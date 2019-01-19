
/*
 * OBSS and bandwidth switch utilities
 * Broadcom 802.11 Networking Device Driver
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
 */

#ifndef _WLC_OBSS_UTIL_H_
#define _WLC_OBSS_UTIL_H_

enum {
	IOV_OBSS_PROT = 1,
	IOV_OBSS_DYN_BWSW_ENAB = 2,
	IOV_OBSS_DYN_BWSW_PARAMS = 3,
	IOV_OBSS_INACTIVITY_PERIOD = 4,
	IOV_OBSS_DUR = 5,
	IOV_CCASTATS = 6,
	IOV_OBSS_DYN_BWSW_DUMP = 7,
	IOV_LAST
};

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WL_SUPPORT_ACS)
void wlc_obss_util_stats(wlc_info_t *wlc, wlc_bmac_obss_counts_t *msrmnt_stored,
	wlc_bmac_obss_counts_t *prev_stats, wlc_bmac_obss_counts_t *curr_stats,
	uint8 report_opt, struct bcmstrbuf *b);
#endif /* BCMDBG || BCMDBG_DUMP || WL_SUPPORT_ACS */

void wlc_obss_util_update(wlc_info_t *wlc, wlc_bmac_obss_counts_t *curr,
	wlc_bmac_obss_counts_t *prev, wlc_bmac_obss_counts_t *o_total, chanspec_t chanspec);

#endif /* _WLC_OBSS_UTIL_H_ */
