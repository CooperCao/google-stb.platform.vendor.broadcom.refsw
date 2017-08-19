/*
 * Leaky AP statistics
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
 * $Id: $
 */

#ifndef _wlc_leakyapstats_h_
#define _wlc_leakyapstats_h_

extern void *wlc_leakyapstats_attach(wlc_info_t *wlc);
extern void wlc_leakyapstats_detach(wlc_leakyapstats_info_t *laps);
extern void wlc_leakyapstats_gt_event(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern void wlc_leakyapstats_gt_start_time_upd(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	tx_status_t *txs);
extern void wlc_leakyapstats_gt_reason_upd(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint8 reason);
extern void wlc_leakyapstats_pkt_stats_upd(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	uint16 seq, uint32 rxtime, uint32 rspec, int8 rssi, uint8 tid, uint16 len);
#endif /* _wlc_leakyapstats_h_ */
