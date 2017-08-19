/*
 * Common CCX function header
 *
 * Copyright (C) 2017, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_ccx.h,v 1.2 2009-10-07 21:14:02 $
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


#define CCX_ROAM_SCAN_CHANNELS	30	/* max roaming scan channels */
/* general ccx data structure */
struct wlc_ccx {
	uint		leap_start;	/* time instance of leap LEAP starting point */
	bool		leap_on;	/* flag for CCX leap */
	bool		fast_roam;	/* flag signalling use of AP channel list */
	uint		orig_reason;	/* original roam reason code(before fast roaming fails) */

	bool		ccx_enabled;	/* CCX feature enable */
	bool		rm;		/* CCX Radio Management enable */

	chanspec_t	ap_chanspec_list[CCX_ROAM_SCAN_CHANNELS];	/* List of channels provided
						 * by AP for fast roaming
						 */
	uint		ap_channel_num;		/* Number of valid channels in list */
	/* roaming stats - S56.4.3, S56.5.2.6 */
	uint32		roaming_start_time;	/* roam start time. low 32 bits of tsf */
	uint32		roaming_delay_tu;	/* time in TU from roam_start_time till
						 * association completion for the last
						 * successful association.
						 */
	uint32		roaming_count;		/* number of successful roams. */

};

#define CCX_ENAB(ccx) ((ccx)->ccx_enabled)


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
#define CCX_CAC_TS_METRICS_TYPE		7	/* Traffic Stream Metrics type */
#define CCX_CAC_TS_RATESET_TYPE		8	/* Traffic Stream Rateset type */
#define CCX_CAC_MSDU_LIFETIME_TYPE	9	/* MSUD Lifetime type */
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

/* exported externs */

/* CCX radio measurement */
#define WLC_RM_TYPE_BEACON_TABLE	0x10 /* Beacon table RM */
#define WLC_RM_TYPE_ACTIVE_SCAN		0x20 /* Active scan RM */
#define WLC_RM_TYPE_PASSIVE_SCAN	0x30 /* Passic scan RM */
#define WLC_RM_TYPE_FRAME		0x40 /* Frame measurement RM */
#define WLC_RM_TYPE_PATHLOSS		0x50 /* Pathloss measurement RM */

#define CCX_LEAP_ROGUE_DURATION	30	/* seconds */

/* CCXv4 S51 - L2 Roaming */
#define WLC_QBSS_LOAD_AAC_LEGACY_AP 	0 	/* AAC for AP without QBSS Load IE */
#define WLC_QBSS_LOAD_CHAN_FREE_MAX	0xff	/* max for channel free score */
#define WLC_QBSS_LOAD_CHAN_FREE_LEGACY_AP 0	/* channel free scroe for AP withotu QBSS Load Ie */
#define WLC_QBSS_LOAD_CHAN_FREE_MAX 	0xff	/* channel free scroe for AP withotu QBSS Load Ie */
#define WLC_CHANNEL_FREE_SORTING_RANGE	7 	/* pref score range for Channel Free Sorting */
#endif	/* _wlc_ccx_h_ */
