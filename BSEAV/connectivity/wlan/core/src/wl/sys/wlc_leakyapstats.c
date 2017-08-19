/**
 * @file
 * @brief
 * Power statistics: Collect power related stats and expose to other modules
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

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scan.h>
#include <event_log.h>
#include <wlc_event_utils.h>
#include <wlc_txs.h>
#include <wlc_leakyapstats.h>

#define WL_LEAKY_AP_STATS_DEFAULT_CFG_IDX	-1
/* check if it is in guard time */
#define LAPS_IN_GT(laps)		((laps)->guard_marker->start_time)
#define TSF_WRAP_AROUND			(0xffffffff)
#define RESET_FROM(t, p, m)		(bzero(&(p->m), sizeof(*p) - OFFSETOF(t, m)))

struct wlc_leakyapstats_info_v1 {
	wlc_info_t			*wlc;
	wlc_leaked_infra_guard_marker_t *guard_marker;
	wlc_leaked_infra_packet_stat_t	*packet_stats;
	int8				cfg_idx;
	bool				laps_suppr;		/* to suppress per pkt log */
};

/* iovar table */
enum {
	IOV_LEAKY_AP_STATS	= 0,
	IOV_LEAKY_AP_SEP	= 1,
	IOV_LAST		= 2
};

static const bcm_iovar_t leakyapstats_iovars[] = {
	{"leaky_ap_stats", IOV_LEAKY_AP_STATS, (0), 0, IOVT_BOOL, 0},
	{"leaky_ap_sep", IOV_LEAKY_AP_SEP, (0), 0, IOVT_BOOL, 0},
	{NULL, 0, 0, 0, 0, 0}
};

static bool
leakyapstats_is_tracked_infra_sta(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	/* Not tracking any cfg */
	if (wlc->leakyapstats_info->cfg_idx == WL_LEAKY_AP_STATS_DEFAULT_CFG_IDX) {
		return FALSE;
	}
	if (cfg->_idx != wlc->leakyapstats_info->cfg_idx) {
		return FALSE;
	}
	if (!(cfg->BSS && BSSCFG_STA(cfg) && cfg->associated && cfg == wlc->cfg)) {
		return FALSE;
	}
	return TRUE;
}

static void
wlc_leakyapstats_pkt_event(wlc_info_t *wlc)
{
	wlc_leaked_infra_packet_stat_t *ps = wlc->leakyapstats_info->packet_stats;

	/* event log dump */
#ifdef EVENT_LOG_COMPILE
	EVENT_LOG_BUFFER(EVENT_LOG_TAG_LEAKY_AP_STATS, (void *)ps, sizeof(*ps));
#else
	wlc_mac_event(wlc, WLC_E_LEAKY_AP_STATS, NULL, WL_LEAKY_AP_STATS_PKT_TYPE,
			0, 0, (void *)ps, sizeof(*ps));
#endif /* EVENT_LOG_COMPILE */

	/* reset pkt stats related info */
	RESET_FROM(wlc_leaked_infra_packet_stat_t, ps, ppdu_len_bytes);
}

void
wlc_leakyapstats_pkt_stats_upd(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	uint16 seq, uint32 rxtime, uint32 rspec, int8 rssi, uint8 tid, uint16 len)
{
	wlc_leakyapstats_info_t *laps = wlc->leakyapstats_info;
	wlc_leaked_infra_packet_stat_t *ps = laps->packet_stats;

	if (!LAPS_IN_GT(laps) || !leakyapstats_is_tracked_infra_sta(wlc, cfg)) {
		return;
	}
	/* Supress per-PPDU report */
	if (laps->laps_suppr) {
		laps->guard_marker->num_pkts++;
		return;
	}

	if (rxtime == ps->ppdu_time) {
		/* Update existing PPDU information */
		ps->ppdu_len_bytes += len;
		ps->num_mpdus++;
	} else {
		/* deal with previous PPDU */
		if (laps->guard_marker->num_pkts > 0)
			wlc_leakyapstats_pkt_event(wlc);
		/* update for a new PPDU */
		ps->ppdu_len_bytes = len;
		ps->num_mpdus++;
		ps->ppdu_time = rxtime;
		ps->rate = rspec;
		ps->seq_number = seq >> SEQNUM_SHIFT;
		ps->rssi = rssi;
		ps->tid = tid;

		laps->guard_marker->num_pkts++;
	}
}

static int
wlc_leakyapstats_doiovar(void *hdl, uint32 actionid, void *params, uint p_len,
	void *arg, uint len, uint val_size, struct wlc_if *wlcif)
{
	wlc_leakyapstats_info_t *laps = (wlc_leakyapstats_info_t *)hdl;
	wlc_info_t *wlc = laps->wlc;
	int err = BCME_OK;
	int32 int_val = 0;
	int32 *ret_int_ptr;
	bool bool_val;

	BCM_REFERENCE(wlcif);

	ret_int_ptr = (int32 *)arg;

	if (p_len >= (int)sizeof(int_val))
		memcpy(&int_val, params, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;

	switch (actionid) {
	case IOV_GVAL(IOV_LEAKY_AP_STATS):
		*ret_int_ptr = wlc->pub->_leakyapstats;
		break;
	case IOV_SVAL(IOV_LEAKY_AP_STATS):
		wlc->pub->_leakyapstats = bool_val;
		if (!bool_val) {
			laps->laps_suppr = 0;
			RESET_FROM(wlc_leaked_infra_guard_marker_t,
					laps->guard_marker, seq_number);
			RESET_FROM(wlc_leaked_infra_packet_stat_t,
					laps->packet_stats, ppdu_len_bytes);
		}
		break;
	case IOV_GVAL(IOV_LEAKY_AP_SEP):
		if (!wlc->pub->_leakyapstats)
			err = BCME_NOTREADY;
		else
			*ret_int_ptr = laps->laps_suppr;
		break;
	case IOV_SVAL(IOV_LEAKY_AP_SEP):
		if (!wlc->pub->_leakyapstats)
			err = BCME_NOTREADY;
		else
			laps->laps_suppr = bool_val;
			if (bool_val) {
				/* 1: disable */
				RESET_FROM(wlc_leaked_infra_packet_stat_t,
						laps->packet_stats, ppdu_len_bytes);
			}
		break;
	}
	return err;
}

static void
leakyapstats_bsscfg_up_down(void *ctx, bsscfg_up_down_event_data_t *evt_data)
{
	wlc_leakyapstats_info_t *laps =	(wlc_leakyapstats_info_t *)ctx;
	wlc_bsscfg_t *cfg = evt_data->bsscfg;

	if (evt_data == NULL || evt_data->bsscfg == NULL) {
		WL_ERROR(("%s: evt_data or bsccfg NULL\n", __FUNCTION__));
		ASSERT(0);
		return;
	}
	if (!evt_data->up) {
		if (cfg->_idx == laps->cfg_idx) {
			/* The primary infra STA we track is down. Clear the stats */
			laps->cfg_idx = WL_LEAKY_AP_STATS_DEFAULT_CFG_IDX;
			bzero(laps->guard_marker, sizeof(*laps->guard_marker));
			bzero(laps->packet_stats, sizeof(*laps->packet_stats));
		}
	} else {
		/* the cfg is primary infra STA and is up, we will track it */
		if (cfg->BSS && BSSCFG_STA(cfg) && cfg == cfg->wlc->cfg) {
			laps->cfg_idx = cfg->_idx;
			laps->guard_marker->type = WL_LEAKY_AP_STATS_GT_TYPE;
			laps->guard_marker->len = sizeof(*laps->guard_marker) -
					OFFSETOF(wlc_leaked_infra_guard_marker_t, seq_number);
			laps->packet_stats->type = WL_LEAKY_AP_STATS_PKT_TYPE;
			laps->packet_stats->len = sizeof(*laps->packet_stats) -
					OFFSETOF(wlc_leaked_infra_packet_stat_t, ppdu_len_bytes);
		}
	}
}

static int
BCMUNINITFN(leakyapstats_up_down)(void *hdl)
{
	wlc_leakyapstats_info_t *laps = (wlc_leakyapstats_info_t *)hdl;
	laps->cfg_idx = WL_LEAKY_AP_STATS_DEFAULT_CFG_IDX;
	laps->laps_suppr = 0;
	bzero(laps->guard_marker, sizeof(*laps->guard_marker));
	bzero(laps->packet_stats, sizeof(*laps->packet_stats));
	return BCME_OK;
}

void *
BCMATTACHFN(wlc_leakyapstats_attach)(wlc_info_t *wlc)
{
	int err;
	wlc_leakyapstats_info_t *laps = NULL;
	wlc_iov_disp_fn_t iovar_fn = wlc_leakyapstats_doiovar;
	const bcm_iovar_t *iovars = leakyapstats_iovars;

	/* allocate leaky ap stats info handle in wlc */
	if (!(laps = MALLOCZ(wlc->osh, sizeof(*laps))))
		goto fail;
	if (!(laps->guard_marker = MALLOCZ(wlc->osh, sizeof(*laps->guard_marker))))
		goto fail;
	if (!(laps->packet_stats = MALLOCZ(wlc->osh, sizeof(*laps->packet_stats))))
		goto fail;

	/* register module */
	if (wlc_module_register(wlc->pub, iovars, "leaky_ap_stats", laps,
		iovar_fn, NULL, leakyapstats_up_down, leakyapstats_up_down)) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n", wlc->pub->unit, __FUNCTION__));
		ASSERT(0);
		goto fail;
	}

	wlc->pub->_leakyapstats = FALSE;
	laps->wlc = wlc;
	laps->cfg_idx = WL_LEAKY_AP_STATS_DEFAULT_CFG_IDX;
	wlc->leakyapstats_info = laps;

	/* bss up/down */
	err = wlc_bsscfg_updown_register(wlc, leakyapstats_bsscfg_up_down, (void *)laps);
	if (err != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_updown_register failed, error %d\n",
		        WLCWLUNIT(wlc), __FUNCTION__, err));
		goto fail;
	}
	return laps;

fail:
	wlc_leakyapstats_detach(laps);
	return NULL;
}

void
BCMATTACHFN(wlc_leakyapstats_detach)(wlc_leakyapstats_info_t *laps)
{
	if (laps == NULL)
		ASSERT(0);
		return;

	/* unregister for bss up/down */
	wlc_bsscfg_updown_unregister(laps->wlc, leakyapstats_bsscfg_up_down, (void *)laps);

	wlc_module_unregister(laps->wlc->pub, "leaky_ap_stats", laps);
	MFREE(laps->wlc->osh, laps->packet_stats, sizeof(*laps->packet_stats));
	MFREE(laps->wlc->osh, laps->guard_marker, sizeof(*laps->guard_marker));
	MFREE(laps->wlc->osh, laps, sizeof(*laps));
}

void
wlc_leakyapstats_gt_event(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	wlc_leakyapstats_info_t *laps = wlc->leakyapstats_info;
	wlc_leaked_infra_guard_marker_t *gm = laps->guard_marker;
	uint32 tsf_l;

	if (!leakyapstats_is_tracked_infra_sta(wlc, cfg)) {
		return;
	}

	if (!LAPS_IN_GT(laps)) {
		/* Clear stale reason */
		if (gm->flag) {
			gm->flag = WL_LEAKED_GUARD_TIME_NONE;
		}
		return;
	}

	/* Push out existing PPDU */
	if (!laps->laps_suppr && gm->num_pkts > 0) {
		wlc_leakyapstats_pkt_event(wlc);
	}

	gm->seq_number++;
	wlc_read_tsf(wlc, &tsf_l, NULL);
	gm->gt_tsf_l = tsf_l;
	/* Round up to millseconds */
	gm->guard_duration = (tsf_l - gm->start_time + 500) / 1000;

	/* Generate event log */
#ifdef EVENT_LOG_COMPILE
	EVENT_LOG_BUFFER(EVENT_LOG_TAG_LEAKY_AP_STATS, (void *)gm, sizeof(*gm));
#else
	wlc_mac_event(wlc, WLC_E_LEAKY_AP_STATS, NULL, WL_LEAKY_AP_STATS_GT_TYPE,
			0, 0, (void *)gm, sizeof(*gm));
#endif /* EVENT_LOG_COMPILE */

	/* reset GT info */
	RESET_FROM(wlc_leaked_infra_guard_marker_t, gm, start_time);
}

void
wlc_leakyapstats_gt_start_time_upd(wlc_info_t *wlc, wlc_bsscfg_t *cfg, tx_status_t *txs)
{
	wlc_leaked_infra_guard_marker_t *gm = wlc->leakyapstats_info->guard_marker;

	if (!leakyapstats_is_tracked_infra_sta(wlc, cfg)) {
		return;
	}

	/* We start guard time only when PM frame is ACKed */
	if (((uint)(wlc_txs_alias_to_old_fmt(wlc, &(txs->status))) &
		(TX_STATUS_ACK_RCV | TX_STATUS_PMINDCTD)) ==
		(TX_STATUS_ACK_RCV | TX_STATUS_PMINDCTD)) {
		/* Overlapping case, no update for guard time start time */
		if (gm->start_time) {
			return;
		}
		gm->start_time = (txs->lasttxtime == 0) ? TSF_WRAP_AROUND : txs->lasttxtime;
	}
}

void
wlc_leakyapstats_gt_reason_upd(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint8 reason)
{
	wlc_leaked_infra_guard_marker_t *gm = wlc->leakyapstats_info->guard_marker;

	if (!leakyapstats_is_tracked_infra_sta(wlc, cfg)) {
		return;
	}
	gm->flag |= reason;
}
