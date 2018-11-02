/*
 * Common CCX function header
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


#ifndef	_wlc_ccx_h_
#define	_wlc_ccx_h_


#define WLC_NRMFRAMEHASH	16	/* # rm frame measurement hash buckets */
/* radio measurement frame report book-keeping data structure */
typedef struct wlc_ccx_rm_frm_elt {
	struct wlc_ccx_rm_frm_elt *next;
	struct ether_addr ta;		/* transmitter address */
	struct ether_addr bssid;	/* bssid transmitter belongs to */
	uint8	frames;			/* total number of frames */
	int	rssi_sum;		/* sum of rssi */
} wlc_ccx_rm_frm_elt_t;
#define CCX_RMPATHLOSS_CHANNELMAX	32
/* S60 related  pathloss measurement */
#ifdef BCMCCX
typedef struct ccx_rm_pl_data {
	/* Data from AP's Pathloss Measurment request */
	uint16	nbursts;	/* nbursts */
	uint16	burst_len;	/* burst len */
	uint16	burst_interval;	/* Burst Interval */
	uint16	duration;	/* Burst Duration */
	int8	req_txpower;	/* txpower */
	struct	ether_addr da;	/* Destination Multicast address */
	uint8	nchannels;	/* number of channels */
	uint8	channels[CCX_RMPATHLOSS_CHANNELMAX];	/* channel list */

	/* local data */
	uint16	cur_burst;	/* actvie burst */
	uint16	cur_burstlen;	/* active burst len */
	uint16	cur_chanidx;	/* current channel idx */
	uint16	seq_number;	/* seq number */

	/* cache the internal rm request */
	wlc_rm_req_t	req;
} ccx_rm_pl_data_t;

#include <wlc_types.h>
#include <wlc_scan_utils.h>

/* radio measurement book-keeping data structure */
struct wlc_ccx_rm {
	/* Beacon measurements */
	bool	scan_active;		/* true if measurement in progress */
	int	scan_dur;		/* TU, specified duration */
	wlc_bss_list_t	scan_results;
	/* Frame measurements */
	bool	frame_active;		/* true if frame measurement in progress */
	int	frame_dur;		/* TU, specified frame measurement duration */
	bool	promisc_org;		/* saved promisc. mode */
	bool	promisc_off;		/* promisc mode off req. pending during test */
	uint32	frm_elts;		/* total frame elements */
	wlc_ccx_rm_frm_elt_t	*frmhash[WLC_NRMFRAMEHASH];

	/* Pathloss Measurement */
	bool	pathloss_active; /* true if pathloss measurement active now */
	bool	pathloss_longrun; /* true if pathloss measurement will run in future */
	bool	cur_txpowerctrl;	/* true if the txpower control is done in hw */
	uint	cur_txpower; 		/* current transmit power */
	bool	cur_txpoweroverride;	/* current transmit power override */
	ccx_rm_pl_data_t pathloss_data;	/* pathloss request data */
	struct wl_timer *plm_burst_timer; /* pathloss measurment burst interval timer */
	struct wl_timer *plm_dur_timer;	/* pathloss measurment burst duration timer */
};
#endif /* BCMCCX */


#define CCX_ROAM_SCAN_CHANNELS	30	/* max roaming scan channels */

/* general ccx data structure per wlc */
struct wlc_ccx {
	bool		ccx_v4_only;	/* v4 only flag */
	bool		rm;		/* CCX Radio Management enable */
	int			cfgh;	/* bsscfg cubby handle */
};


/* Traffic Stream Metrics (TSM) parameters per AC use by CAC in CCX TSM
 * protocol and TSM reporting
 */
struct ccx_ts_metrics {
	uint16 msdu_lifetime;		/* msdu lifetime (TU) */
	bool ts_metrics;		/* TRUE = enable */
	uint16 ts_metrics_interval;	/* interval between IAPP message (TU) */
	uint16 ts_metrics_count;	/* count number of watchdog timeout period */
	uint16 cnt_delay10;		/* # of packet < 10 ms delay */
	uint16 cnt_delay20;		/* # of packet > 10 ms < 20 ms delay */
	uint16 cnt_delay40;		/* # of packet > 20 ms < 40 ms delay */
	uint16 cnt_delay;		/* # of packet > 40 ms delay */
	uint32 total_media_delay;	/* total media delay in usec */
	uint32 total_pkt_delay;		/* total packet delay usec */
	uint32 pkt_tx;			/* packet tx count */
	uint32 pkt_tx_ok;		/* packet tx successful count */
	uint32 last_tsf_timestamp;	/* buffer last packet tsf timestamp usec */
	uint8 tid;				/* traffic stream id */
	uint16 total_used_time;	/* total used time in measurement interval */
};

/* Traffic Stream Metrics constant defined by CCX spec */
#define CCX_CAC_VOICE_TID		0	/* Default for Voice TID */
#define CCX_CAC_VOICE_USER_PRIO		6	/* Default for Voice User Priority */
#define CCX_CAC_SIGNAL_TID		1	/* Default for Signal TID */
#define CCX_CAC_SIGNAL_USER_PRIO	4	/* Default for Signal User Priority */

/* defines used for quick compare */
#define CCX_CAC_TS_METRICS_IE		(CISCO_AIRONET_OUI"\x07")
#define CCX_CAC_TS_RATESET_IE		(CISCO_AIRONET_OUI"\x08")
#define CCX_CAC_MSDU_LIFETIME_IE	(CISCO_AIRONET_OUI"\x09")
#define CCX_CAC_OUI_TYPE_LEN		(DOT11_OUI_LEN + 1)

/* Traffic Stream Metrics interval */
#define CCX_TSMETRIC_INTERVAL_MIN	977	/* minimum interval TU (1 sec) */
#define CCX_TSMETRIC_INTERVAL_MAX	9770	/* maximum interval TU (10sec) */
#define CCX_TSMETRIC_INTERVAL_DEFAULT	4883	/* default interval TU (5 sec) */

/* Traffic Stream Metrics delay constants */
#define CCX_TSM_10MS_DELAY	10000	/* 10 ms delay */
#define CCX_TSM_20MS_DELAY	20000	/* 20 ms delay */
#define CCX_TSM_40MS_DELAY	40000	/* 40 ms delay */

#define CCX_ASSOCIATION(ccx, cfg) \
(!BSS_P2P_ENAB(((cfg)->wlc), (cfg)) && BSSCFG_STA((cfg)) && \
	wlc_ccx_get_ccx_network(ccx, cfg))

#define BSS_CCX_ENAB(wlc, cfg) (CCX_ENAB((wlc->pub)) && BSSCFG_STA((cfg)) && \
	!BSS_P2P_ENAB((wlc), (cfg)))

/* exported externs */
#ifdef BCMCCX
extern ccx_t *wlc_ccx_attach(wlc_info_t *wlc);
extern void wlc_ccx_detach(ccx_t *ccx);
extern void wlc_ccx_on_join_adopt_bss(ccx_t *ccx, wlc_bsscfg_t *cfg);
extern void wlc_ccx_on_leave_bss(ccx_t *ccx, wlc_bsscfg_t *cfg);
extern void wlc_ccx_on_roam_start(ccx_t *ccx, wlc_bsscfg_t *cfg, uint reason);
extern int wlc_ccx_set_auth(ccx_t *ccx, wlc_bsscfg_t *cfg);
extern void wlc_ccx_iapp_roam_rpt(ccx_t *ccx, wlc_bsscfg_t *cfg);
extern void wlc_ccx_tx_pwr_ctl(ccx_t *ccx, uint8 *tlvs, int tlvs_len, wlc_bsscfg_t *cfg);
extern int wlc_ccx_chk_iapp_frm(ccx_t *ccxh, struct wlc_frminfo *f, wlc_bsscfg_t *cfg);
extern void wlc_ccx_update_BSS(ccx_t *ccx, uint rx_band, wlc_bss_info_t *BSS);
extern uint wlc_ccx_prune(ccx_t *ccx, wlc_bss_info_t *bi, wlc_bsscfg_t *cfg);
extern uint wlc_ccx_roam(ccx_t *ccx, uint roam_reason_code, wlc_bsscfg_t *cfg);
extern uint8 *wlc_ccx_append_ies(ccx_t *ccxh, uint8 *pbody, wlc_bss_info_t *bi, bool reassoc,
	uint32 wpa_auth, struct ether_addr *curea, wpa_ie_fixed_t* wpa_ie);

/* radio measurement functions */
extern int wlc_ccx_rm_validate(ccx_t *ccx, chanspec_t cur_chanspec, wlc_rm_req_t *req);
extern int wlc_ccx_rm_begin(ccx_t *ccx, wlc_rm_req_t *req);
extern void wlc_ccx_rm_frm(ccx_t *ccx, wlc_d11rxhdr_t *wrxh, struct dot11_header *h);
extern void wlc_ccx_rm_frm_complete(ccx_t *ccx);
extern void wlc_ccx_rm_scan_complete(ccx_t *ccx);
extern void wlc_ccx_rm_pathloss_complete(ccx_t *ccx, bool force_done);

extern void wlc_ccx_rm_report(ccx_t *ccx, wlc_rm_req_t *req_block, int count);
extern void wlc_ccx_rm_end(ccx_t *ccx);
extern void wlc_ccx_rm_free(ccx_t *ccx);

extern bool wlc_ccx_is_droam(ccx_t *ccxh, wlc_bsscfg_t* cfg);

extern void wlc_cckm_calc_reassocreq_MIC(cckm_reassoc_req_ie_t *cckmie,
	struct ether_addr *bssid, wpa_ie_fixed_t *rsnie, struct ether_addr *cur_ea,
	uint32 rn, uint8 *key_refresh_key, uint32 WPA_auth);

extern uint wlc_ccx_get_orig_reason(ccx_t *ci, wlc_bsscfg_t *cfg);
extern void wlc_ccx_set_orig_reason(ccx_t *ci, wlc_bsscfg_t *cfg, uint orig_reason);

extern chanspec_t* wlc_ccx_get_ap_chanspec_list(ccx_t *ci, wlc_bsscfg_t *cfg);
extern uint wlc_ccx_get_ap_channel_num(ccx_t *ci, wlc_bsscfg_t *cfg);
extern uint wlc_ccx_get_leap_start(ccx_t *ci, wlc_bsscfg_t *cfg);
extern bool wlc_ccx_get_leap_on(ccx_t *ci, wlc_bsscfg_t *cfg);
extern void wlc_ccx_set_leap_start(ccx_t *ci, wlc_bsscfg_t *cfg, uint leap_start);
extern bool wlc_ccx_set_leap_on(ccx_t *ci, wlc_bsscfg_t *cfg, bool leap_on);
extern uint32 wlc_ccx_get_roaming_start_time(ccx_t *ci, wlc_bsscfg_t *cfg);
extern uint32* wlc_ccx_get_roam_start_time(ccx_t *ci, wlc_bsscfg_t *cfg);
extern void wlc_ccx_set_roaming_delay_tu(ccx_t *ci, wlc_bsscfg_t *cfg, uint32 roaming_delay_tu);
extern void wlc_ccx_increment_roaming_count(ccx_t *ci, wlc_bsscfg_t *cfg);
extern bool wlc_ccx_get_ccx_network(ccx_t *ci, wlc_bsscfg_t *cfg);
extern void wlc_ccx_set_ccx_network(ccx_t *ci, wlc_bsscfg_t *cfg, bool ccx_network);
extern bool wlc_ccx_get_fast_roam(ccx_t *ci, wlc_bsscfg_t *cfg);
extern void wlc_ccx_set_fast_roam(ccx_t *ci, wlc_bsscfg_t *cfg, bool fast_roam);
extern void wlc_ccx_update_roaming_delay_tu(ccx_t *ci, wlc_bsscfg_t *cfg);

/* exported data */
extern const uint8 ccx_rm_capability_ie[];
extern const uint8 ccx_version_ie[];
#endif /* BCMCCX */

/* CCX radio measurement */
#define WLC_RM_TYPE_BEACON_TABLE	0x10 /* Beacon table RM */
#define WLC_RM_TYPE_ACTIVE_SCAN		0x20 /* Active scan RM */
#define WLC_RM_TYPE_PASSIVE_SCAN	0x30 /* Passic scan RM */
#define WLC_RM_TYPE_FRAME		0x40 /* Frame measurement RM */
#define WLC_RM_TYPE_PATHLOSS		0x50 /* Pathloss measurement RM */

#define CCX_LEAP_ROGUE_DURATION	30	/* seconds */

/* CCXv4 S51 - L2 Roaming */
#define WLC_QBSS_LOAD_AAC_LEGACY_AP 	0 	/* AAC for AP without QBSS Load IE */
#define WLC_QBSS_LOAD_CHAN_FREE_LEGACY_AP 0	/* channel free scroe for AP withotu QBSS Load Ie */
#define WLC_CHANNEL_FREE_SORTING_RANGE	7 	/* pref score range for Channel Free Sorting */
#endif	/* _wlc_ccx_h_ */
