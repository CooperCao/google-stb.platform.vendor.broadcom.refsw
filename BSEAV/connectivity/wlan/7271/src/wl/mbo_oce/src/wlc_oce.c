/*
 * OCE implementation for
 * Broadcom 802.11bang Networking Device Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id$
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 */

/**
 * @file
 * @brief
 * This file implements a part of WFA OCE features
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <osl.h>
#include <wl_dbg.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <wlc_bsscfg.h>
#include <wl_export.h>
#include <wlc_bmac.h>

#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_types.h>
#include <wlc_ie_mgmt_types.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_vs.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_mgmt_lib.h>
#include <wlc_ie_helper.h>
#include <wlc_ap.h>
#include <wlc_rspec.h>
#include <wlc_assoc.h>
#include <proto/oce.h>
#include <proto/fils.h>
#include <wlc_oce.h>
#include "wlc_mbo_oce_priv.h"
#include <bcmwpa.h>
#include <wlc_scan_utils.h>
#include <wlc_lq.h>
#include <phy_noise_api.h>
#include <wlc_rx.h>
#include <wlc_tx.h>
#include <wlc_mcnx.h>
#include <wlc_esp.h>
#include <wlc_ie_reg.h>
#include <wlc_wnm.h>
#include <bcmiov.h>
#include <wlc_dump.h>
#include <wlc_hrt.h>
#include <wlc_chanctxt.h>
#include <wlc_scb_ratesel.h>

#define OCE_PROBE_DEFFERAL_TIME		15U
#define OCE_DEF_RSSI_DELTA		7U
#define OCE_DEF_RWAN_UPLINK_DELTA	13U /* change of 4 times */
#define OCE_DEF_RWAN_DOWNLINK_DELTA	13U /* change of 4 times */
#define OCE_DEF_RWAN_WINDOW_DUR		10U /* window duration to test rwan metrics */

#define	OCE_PRB_REQ_RATE_DISABLE	0x2 /* disable oce rates for probe request */

static const uint8 oce_pref_channels[] = {1, 6, 11};

typedef struct oce_delayed_bssid_list oce_delayed_bssid_list_t;
struct oce_delayed_bssid_list {
	oce_delayed_bssid_list_t *next;
	oce_delayed_bssid_list_t *prev;
	struct ether_addr bssid;		/* reject bssid */
	uint32 release_tm;			/* timestamp to release from the list (ms) */
	int8 release_rssi;			/* min rssi value to release from the list */
};

#define MAX_PROBE_SUPPRESS_BSS	10U
#define TBTT_WAIT_EXTRA_TIME	5000U
#define TBTT_WAIT_MAX_TIME	102000UL

#define MAX_TSF_L	0xffffffffUL

typedef struct prb_suppress_bss_item {
	dll_t	node;
	struct ether_addr bssid;		/* suppression bssid */
	uint32	short_ssid;			/* suppression short-ssid */
	uint32	bcn_prb;
	uint32	ts;
	uint32	time_to_tbtt;
} prb_suppress_bss_item_t;

typedef struct wlc_oce_rwan_info {
	uint8 uplink_trigger;
	uint8 downlink_trigger;
	uint8 window_time;
	uint8 pad;
} wlc_oce_rwan_info_t;

typedef struct wlc_oce_rwan_statcs {
	uint32 next_window; /* next windows in secs */
	bool roam_trigger;
	uint8 pad;
} wlc_oce_rwan_statcs_t;

/* Per 802.11-2016 Annex E-4 global opclass and channels */
/* include only subset */
#define OPCLASS_MAX_CHANNELS	13U
#define NO_OF_OPCLASS_CHLIST	19U

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>
BWL_PRE_PACKED_STRUCT struct wlc_oce_opcls_chlist {
	uint8 cnt;
	uint8 opclass;
	uint8 chlist[OPCLASS_MAX_CHANNELS];
} BWL_POST_PACKED_STRUCT;
typedef struct wlc_oce_opcls_chlist wlc_oce_opcls_chlist_t;
/* This marks the end of a packed structure section. */
#include <packed_section_end.h>


typedef struct wlc_oce_cmn_info {
	bool env_inc_non_oce_ap; /* indicates non-oce AP(s) presence */
	uint8 probe_defer_time;	/* probe request deferral time */
	uint8 max_channel_time; /* probe response listen time */
	uint8 fd_tx_period;	/* FD frame TX period */
	uint32 fd_tx_duration;	/* FD frame TX duration */
	uint8 rej_retry_delay;	/* Retry delay time (secs) for RSSI Assoc Rejection */
	int8 ass_rej_rssi_thd;	/* Assoc rejection RSSI Threashold */
	uint16 rssi_delta;	/* RSSI delta */
	uint8 red_wan_links; /* links values from reduced wan metrics */
	wlc_oce_rwan_info_t rwan_info;
	/* MBO-OCE IE attributes handlers */
	wlc_mbo_oce_ie_build_hndl_t mbo_oce_build_h;
	wlc_mbo_oce_ie_build_data_t	mbo_oce_build_data;
	wlc_mbo_oce_ie_parse_hndl_t mbo_oce_parse_h;
	wlc_mbo_oce_ie_parse_data_t	mbo_oce_parse_data;
	wlc_esp_ie_parse_hndl_t	esp_parse_h;
	wlc_esp_ie_parse_data_t	esp_parse_data;
	wlc_ier_reg_t *ier_fd_act_frame;
	uint8 disable_oce_prb_req_rate;
	uint8 fd_frame_count;
	uint32 rnr_scan_period;	/* rnr scan timer period */
	wlc_oce_opcls_chlist_t opcls_chlist[NO_OF_OPCLASS_CHLIST];
} wlc_oce_cmn_info_t;

struct wlc_oce_info {
	wlc_info_t *wlc;
	/* shared info */
	wlc_oce_cmn_info_t* oce_cmn_info;
	bcm_iov_parse_context_t *iov_parse_ctx;
	uint32 fils_req_params_bitmap;
	wlc_hrt_to_t *fd_timer;
	uint32	up_tick;
	int      cfgh;    /* oce bsscfg cubby handle */
	uint8	scan_ssid[DOT11_MAX_SSID_LEN];
	uint8	scan_ssid_len;
	uint32	short_ssid;
	struct wl_timer *rnr_scan_timer;
	dll_t prb_suppress_bss_list;
	uint16	prb_suppress_bss_count;
	dll_pool_t* prb_suppress_bss_pool;
	wifi_oce_probe_suppress_bssid_attr_t *prb_suppress_bss_attr;
	wifi_oce_probe_suppress_ssid_attr_t *prb_suppress_ssid_attr;
};

/* NOTE: This whole struct is being cloned across with its contents and
 * original memeory allocation intact.
 */
typedef struct wlc_oce_bsscfg_cubby {
	oce_delayed_bssid_list_t *list_head;
	uint8 rwan_capacity; /* reduced wan metrics - available capacity */
	wlc_oce_rwan_statcs_t rwan_statcs;
	uint8	*rnr_nbr_ap_info;
	uint8	rnr_nbr_ap_info_size;
	uint8	*rnrc_sssid_list;
	uint8	rnrc_sssid_list_size;
} wlc_oce_bsscfg_cubby_t;

#define OCE_BSSCFG_CUBBY_LOC(oce, cfg) ((wlc_oce_bsscfg_cubby_t **)BSSCFG_CUBBY(cfg, (oce)->cfgh))
#define OCE_BSSCFG_CUBBY(oce, cfg) (*OCE_BSSCFG_CUBBY_LOC(oce, cfg))
#define OCE_CUBBY_CFG_SIZE  sizeof(wlc_oce_bsscfg_cubby_t)

#define MAX_CHANNEL_TIME	10

/* FILS Discovery frame transmission timer */
#define FD_TX_PERIOD		20 /* ms */
#define FD_TX_DURATION		180000 /* ms (3 minutes) */
#define MAX_BEFORE_TBTT		(10 * DOT11_TU_TO_US) /* us */

#define FD_TX_ADD_TIMER(oce, cb)	wlc_hrt_add_timeout((oce)->fd_timer, \
	 (oce)->oce_cmn_info->fd_tx_period * DOT11_TU_TO_US, cb, oce)
#define FD_TX_ADD_TIMER_ADD(oce, add, cb)	wlc_hrt_add_timeout((oce)->fd_timer, \
	 ((oce)->oce_cmn_info->fd_tx_period * DOT11_TU_TO_US)+ add, cb, oce)
#define FD_TX_DEL_TIMER(oce)	wlc_hrt_del_timeout((oce)->fd_timer)
#define FD_TX_FREE_TIMER(oce)	wlc_hrt_free_timeout((oce)->fd_timer)

/* Reduced Neibor Report scan timer */
#define RNR_SCAN_START_PERIOD	1000 /* Initial scan period right after wl up in ms */
#define RNR_SCAN_PERIOD		600000 /* ms */
#define RNR_SCAN_ADD_TIMER(oce, to)	wl_add_timer((oce)->wlc->wl, (oce)->rnr_scan_timer, (to), 0)
#define RNR_SCAN_DEL_TIMER(oce)		wl_del_timer((oce)->wlc->wl, (oce)->rnr_scan_timer)
#define RNR_SCAN_FREE_TIMER(oce)	wl_free_timer((oce)->wlc->wl, (oce)->rnr_scan_timer)
#define IEM_OCE_FD_FRAME_PARSE_CB_MAX	1

/* iovar table */
enum {
	IOV_OCE = 0,
	IOV_OCE_LAST
};

static const bcm_iovar_t oce_iovars[] = {
	{"oce", IOV_OCE, 0, 0, IOVT_BUFFER, 0},
	{NULL, 0, 0, 0, 0, 0}
};

/* module registration functionality */
static void wlc_oce_watchdog(void *ctx);
static int wlc_oce_wlc_up(void *ctx);
static int wlc_oce_wlc_down(void *ctx);

/* cubby registration functionality */
static int wlc_oce_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_oce_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg);
static int wlc_oce_bsscfg_get(void *ctx, wlc_bsscfg_t *cfg, uint8 *data,
	int *len);
static int wlc_oce_bsscfg_set(void *ctx, wlc_bsscfg_t *cfg,
	const uint8 *data, int len);

/* iovar handlers */
static void *
oce_iov_context_alloc(void *ctx, uint size);
static void
oce_iov_context_free(void *ctx, void *iov_ctx, uint size);
static int
BCMATTACHFN(oce_iov_get_digest_cb)(void *ctx, bcm_iov_cmd_digest_t **dig);
static int
wlc_oce_iov_cmd_validate(const bcm_iov_cmd_digest_t *dig, uint32 actionid,
	const uint8 *ibuf, size_t ilen, uint8 *obuf, size_t *olen);
static int wlc_oce_iov_set_enable(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen);
static int wlc_oce_iov_get_enable(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen);
static int wlc_oce_iov_set_probe_def_time(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen);
static int wlc_oce_iov_get_probe_def_time(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen);
#ifdef WL_OCE_AP
static int wlc_oce_iov_set_fd_tx_period(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen);
static int wlc_oce_iov_get_fd_tx_period(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen);
static int wlc_oce_iov_set_fd_tx_duration(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen);
static int wlc_oce_iov_get_fd_tx_duration(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen);
static int wlc_oce_iov_set_ass_rej_rssi_thd(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen);
static int wlc_oce_iov_get_ass_rej_rssi_thd(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen);
static int wlc_oce_iov_set_redwan_links(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen);
static int wlc_oce_iov_get_redwan_links(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen);
static int wlc_oce_iov_get_cu_trigger(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen);
static int wlc_oce_iov_set_cu_trigger(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen);
static int wlc_oce_iov_set_retry_delay(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen);
static int wlc_oce_iov_get_retry_delay(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen);
#endif /* WL_OCE_AP */

static void wlc_oce_scan_rx_callback(void *ctx, scan_utl_scan_data_t *sd);
static void wlc_oce_detect_environment(wlc_oce_info_t *oce, uint8* body,
	uint16 body_len);
static void wlc_oce_reset_environment(wlc_oce_info_t* oce);
static void wlc_oce_scan_start_callback(void *ctx, wlc_ssid_t *ssid);
static ratespec_t wlc_oce_get_oce_compat_rspec(wlc_oce_info_t *oce);
static uint16 wlc_oce_write_oce_cap_ind_attr(uint8 *cp, uint8 buf_len, bool ap);
static uint16 wlc_oce_write_fils_req_param_element(wlc_oce_info_t *oce,
	uint8 *cp, uint8 buf_len);
static uint16 wlc_oce_get_fils_req_params_size(wlc_oce_info_t *oce);
static void wlc_oce_add_cap_ind(wlc_mbo_oce_attr_build_data_t *data, uint16 *attr_len,
	uint16 *total_len, uint8 *cp, bool ap);
static int wlc_oce_ie_build_fn(void *ctx, wlc_mbo_oce_attr_build_data_t *data);
static int wlc_oce_ie_parse_fn(void *ctx, wlc_mbo_oce_attr_parse_data_t *data);
static int wlc_oce_fils_element_build_fn(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_oce_fils_element_calc_len(void *ctx, wlc_iem_calc_data_t *data);
#ifdef WL_OCE_AP
static int wlc_oce_rnr_element_build_fn(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_oce_rnr_element_calc_len(void *ctx, wlc_iem_calc_data_t *data);
static uint16 wlc_oce_write_rnr_element(wlc_oce_bsscfg_cubby_t *obc,
	uint8 *cp, uint buf_len);
static int wlc_oce_ap_chrep_element_build_fn(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_oce_ap_chrep_element_calc_len(void *ctx, wlc_iem_calc_data_t *data);
static uint16 wlc_oce_write_ap_chrep_element(wlc_oce_info_t *oce,
	uint8 *cp, uint buf_len);
static uint wlc_oce_get_ap_chrep_size(wlc_oce_info_t *oce);
static void wlc_oce_update_ap_chrep(wlc_oce_info_t* oce, wlc_bsscfg_t *bsscfg);
#endif /* WL_OCE_AP */
static int wlc_oce_rnr_element_parse_fn(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_oce_parse_rnr_element(wlc_oce_info_t *oce,
	 uint8* buf, uint16 buflen);
static int wlc_oce_parse_fd_action_frame(wlc_oce_info_t *oce, wlc_d11rxhdr_t *wrxh,
	struct dot11_management_header *hdr, uint8 *body, uint body_len);
#ifdef WL_ESP
static int wlc_oce_esp_ie_parse_fn(void *ctx, wlc_esp_attr_parse_data_t *data);
static void wlc_oce_parse_esp_element(wlc_oce_info_t *oce, wlc_bss_info_t *bi,
	uint8* buf, uint16 buflen);
#endif
static int wlc_oce_bssload_ie_parse_fn(void *ctx, wlc_iem_parse_data_t *data);
static void wlc_oce_parse_bssload_ie(wlc_oce_info_t *oce, wlc_bss_info_t *bi,
        uint8* buf, uint16 buflen);
/* delayed list functionality */
static oce_delayed_bssid_list_t* wlc_oce_find_in_delayed_list(
	wlc_oce_info_t *oce, wlc_bsscfg_t *bsscfg, struct ether_addr *bssid);
static int wlc_oce_add_to_delayed_list(wlc_oce_info_t *oce, wlc_bsscfg_t *bsscfg,
	wlc_bss_info_t *bss, uint8 delay, uint8 delta);
static int wlc_oce_rm_from_delayed_list(wlc_oce_info_t *oce, wlc_bsscfg_t *bsscfg,
	oce_delayed_bssid_list_t *mmbr);
static int wlc_oce_rm_delayed_list(wlc_oce_info_t *oce, wlc_bsscfg_t *bsscfg);

/* probe suppress bss list functionality */
static prb_suppress_bss_item_t* wlc_oce_find_in_prb_suppress_bss_list(wlc_oce_info_t *oce,
	struct ether_addr *bssid);
static int wlc_oce_add_to_prb_suppress_bss_list(wlc_oce_info_t *oce,
	struct ether_addr *bssid, uint32 short_ssid, uint32 ts, uint32 time_to_tbtt, bool bcn_prb);
#ifdef WL_OCE_AP
static void wlc_oce_fd_tx_timer(void* ctx);
static void wlc_oce_rnr_scan_timer(void *ctx);
#ifdef WLMCNX
static void wlc_oce_pretbtt_callback(void *ctx, wlc_mcnx_intr_data_t *notif_data);
#endif
static void wlc_oce_set_bcn_rate(wlc_oce_info_t *oce);
uint8 wlc_chanspec_ac2opclass(chanspec_t chanspec);
#endif /* WL_OCE_AP */

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int wlc_oce_dump(void *ctx, struct bcmstrbuf *b);
#endif /* BCMDBG || BCMDBG_DUMP */
#define MAX_SET_ENABLE		8
#define MIN_SET_ENABLE		MAX_SET_ENABLE
#define MAX_SET_PROBE_DEF_TIME		8
#define MIN_SET_PROBE_DEF_TIME		MAX_SET_PROBE_DEF_TIME
#ifdef WL_OCE_AP
#define MAX_SET_FD_TX_PERIOD		8
#define MIN_SET_FD_TX_PERIOD		MAX_SET_FD_TX_PERIOD
#define MAX_SET_FD_TX_DURATION		8
#define MIN_SET_FD_TX_DURATION		MAX_SET_FD_TX_DURATION
#define MAX_SET_RSSI_TH		8
#define MIN_SET_RSSI_TH		MAX_SET_RSSI_TH
#define MAX_SET_CU_TRIGGER	8
#define MIN_SET_CU_TRIGGER	MAX_SET_CU_TRIGGER
#define MAX_SET_REJ_RETRY_DELAY	8
#define MIN_SET_REJ_RETRY_DELAY	MAX_SET_REJ_RETRY_DELAY
#endif /* WL_OCE_AP */

static const bcm_iov_cmd_info_t oce_sub_cmds[] = {
	{WL_OCE_CMD_ENABLE, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32, wlc_oce_iov_cmd_validate,
	wlc_oce_iov_get_enable, wlc_oce_iov_set_enable,
	0, MIN_SET_ENABLE, MAX_SET_ENABLE, 0, 0
	},
	{WL_OCE_CMD_PROBE_DEF_TIME, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32, wlc_oce_iov_cmd_validate,
	wlc_oce_iov_get_probe_def_time, wlc_oce_iov_set_probe_def_time,
	0, MIN_SET_PROBE_DEF_TIME, MAX_SET_PROBE_DEF_TIME, 0, 0
	},
#ifdef WL_OCE_AP
	{WL_OCE_CMD_FD_TX_PERIOD, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32, wlc_oce_iov_cmd_validate,
	wlc_oce_iov_get_fd_tx_period, wlc_oce_iov_set_fd_tx_period, 0,
	MIN_SET_FD_TX_PERIOD, MAX_SET_FD_TX_PERIOD, 0, 0
	},
	{WL_OCE_CMD_FD_TX_DURATION, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32, wlc_oce_iov_cmd_validate,
	wlc_oce_iov_get_fd_tx_duration, wlc_oce_iov_set_fd_tx_duration, 0,
	MIN_SET_FD_TX_DURATION, MAX_SET_FD_TX_DURATION, 0, 0
	},
	{WL_OCE_CMD_RSSI_TH, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32, wlc_oce_iov_cmd_validate,
	wlc_oce_iov_get_ass_rej_rssi_thd, wlc_oce_iov_set_ass_rej_rssi_thd, 0,
	MIN_SET_RSSI_TH, MAX_SET_RSSI_TH, 0, 0
	},
	{WL_OCE_CMD_RWAN_LINKS, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32, wlc_oce_iov_cmd_validate,
	wlc_oce_iov_get_redwan_links, wlc_oce_iov_set_redwan_links, 0,
	MIN_SET_RSSI_TH, MAX_SET_RSSI_TH, 0, 0
	},
	{WL_OCE_CMD_CU_TRIGGER, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32, wlc_oce_iov_cmd_validate,
	wlc_oce_iov_get_cu_trigger, wlc_oce_iov_set_cu_trigger, 0,
	MIN_SET_CU_TRIGGER, MAX_SET_CU_TRIGGER, 0, 0
	},
	{WL_OCE_CMD_REJ_RETRY_DELAY, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32, wlc_oce_iov_cmd_validate,
	wlc_oce_iov_get_retry_delay, wlc_oce_iov_set_retry_delay, 0,
	MIN_SET_REJ_RETRY_DELAY, MAX_SET_REJ_RETRY_DELAY, 0, 0
	}
#endif /* WL_OCE_AP */
};

#define SUBCMD_TBL_SZ(_cmd_tbl)  (sizeof(_cmd_tbl)/sizeof(*_cmd_tbl))

static int
wlc_oce_doiovar(void *hdl, uint32 actionid,
	void *p, uint plen, void *a, uint alen, uint vsize, struct wlc_if *wlcif)
{
	wlc_oce_info_t *oce = (wlc_oce_info_t *)hdl;
	int32 int_val = 0;
	int err = BCME_OK;

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	switch (actionid) {
		case IOV_GVAL(IOV_OCE):
		case IOV_SVAL(IOV_OCE):
		{
			err = bcm_iov_doiovar(oce->iov_parse_ctx, actionid, p, plen, a, alen,
				vsize, wlcif);
			break;
		}
		default:
			err = BCME_UNSUPPORTED;
			break;
	}
	return err;
}

wlc_oce_info_t *
BCMATTACHFN(wlc_oce_attach)(wlc_info_t *wlc)
{
	wlc_oce_info_t *oce = NULL;
	bcm_iov_parse_context_t *parse_ctx = NULL;
	bcm_iov_parse_config_t parse_cfg;
	wlc_oce_cmn_info_t *oce_cmn = NULL;
	bsscfg_cubby_params_t cubby_params;
	int ret = BCME_OK;
	uint16 fils_elm_build_fstbmp = FT2BMP(FC_PROBE_REQ);
	uint16 rnr_elm_build_fstbmp = FT2BMP(FC_PROBE_RESP) | FT2BMP(FC_BEACON);
	uint16 bssload_parse_fstbmp = FT2BMP(WLC_IEM_FC_SCAN_BCN) |
			FT2BMP(WLC_IEM_FC_SCAN_PRBRSP);
	uint16 ap_chrep_elm_build_fstbmp = FT2BMP(FC_PROBE_RESP) | FT2BMP(FC_BEACON);
	uint16 rnr_elm_parse_fstbmp = FT2BMP(FC_PROBE_RESP) |
		FT2BMP(FC_BEACON) |
		FT2BMP(WLC_IEM_FC_SCAN_BCN) |
		FT2BMP(WLC_IEM_FC_SCAN_PRBRSP);

	oce = (wlc_oce_info_t *)MALLOCZ(wlc->osh, sizeof(*oce));
	if (oce == NULL) {
		WL_ERROR(("wl%d: %s:out of mem. alloced %u bytes\n",
			wlc->pub->unit, __FUNCTION__,  MALLOCED(wlc->osh)));
		goto fail;
	}

	oce_cmn = (wlc_oce_cmn_info_t *) obj_registry_get(wlc->objr, OBJR_OCE_CMN_INFO);
	if (oce_cmn == NULL) {
		oce_cmn = (wlc_oce_cmn_info_t *)MALLOCZ(wlc->osh, sizeof(*oce_cmn));
		if (oce_cmn == NULL) {
			WL_ERROR(("wl%d: %s:out of mem. alloced %u bytes\n",
				wlc->pub->unit, __FUNCTION__,  MALLOCED(wlc->osh)));
			goto fail;
		}
		obj_registry_set(wlc->objr, OBJR_OCE_CMN_INFO, oce_cmn);

		oce_cmn->probe_defer_time = OCE_PROBE_DEFFERAL_TIME;
		oce_cmn->rssi_delta = OCE_DEF_RSSI_DELTA;
		oce_cmn->fd_tx_period = FD_TX_PERIOD;
		oce_cmn->rnr_scan_period = RNR_SCAN_PERIOD;
		oce_cmn->rwan_info.uplink_trigger = OCE_DEF_RWAN_UPLINK_DELTA;
		oce_cmn->rwan_info.downlink_trigger = OCE_DEF_RWAN_DOWNLINK_DELTA;
		oce_cmn->rwan_info.window_time = OCE_DEF_RWAN_WINDOW_DUR;
#ifdef WL_OCE_AP
		oce_cmn->ass_rej_rssi_thd = OCE_ASS_REJ_DEF_RSSI_THD;
		oce_cmn->red_wan_links = (OCE_REDUCED_WAN_METR_DEF_UPLINK_CAP << 4) +
			OCE_REDUCED_WAN_METR_DEF_DOWNLINK_CAP;
#endif /* WL_OCE_AP */
		/* register OCE attributes build/parse callbacks */
		oce_cmn->mbo_oce_build_data.build_fn = wlc_oce_ie_build_fn;
		oce_cmn->mbo_oce_build_data.fstbmp = FT2BMP(FC_ASSOC_REQ) |
#ifdef WL_OCE_AP
			FT2BMP(FC_BEACON) |
#endif /* WL_OCE_AP */
			FT2BMP(FC_REASSOC_REQ) |
			FT2BMP(FC_ASSOC_RESP) |
			FT2BMP(FC_REASSOC_RESP) |
			FT2BMP(FC_PROBE_RESP) |
			FT2BMP(FC_PROBE_REQ);
		oce_cmn->mbo_oce_build_data.ctx = oce;

		oce_cmn->mbo_oce_build_h =
			wlc_mbo_oce_register_ie_build_cb(wlc->mbo_oce,
				&oce_cmn->mbo_oce_build_data);

		oce_cmn->mbo_oce_parse_data.parse_fn = wlc_oce_ie_parse_fn;
		oce_cmn->mbo_oce_parse_data.fstbmp = FT2BMP(FC_BEACON) |
			FT2BMP(WLC_IEM_FC_SCAN_BCN) |
			FT2BMP(WLC_IEM_FC_SCAN_PRBRSP) |
#ifdef WL_OCE_AP
			FT2BMP(FC_ASSOC_REQ) |
			FT2BMP(FC_REASSOC_REQ) |
#endif /* WL_OCE_AP */
			FT2BMP(FC_PROBE_RESP) |
			FT2BMP(FC_ASSOC_RESP) |
			FT2BMP(FC_REASSOC_RESP);
		oce_cmn->mbo_oce_parse_data.ctx = oce;
		oce_cmn->mbo_oce_parse_h =
			wlc_mbo_oce_register_ie_parse_cb(wlc->mbo_oce,
				&oce_cmn->mbo_oce_parse_data);
#ifdef WL_ESP
		/* register ESP IE parse callbacks */
		oce_cmn->esp_parse_data.parse_fn = wlc_oce_esp_ie_parse_fn;
		oce_cmn->esp_parse_data.fstbmp = FT2BMP(FC_BEACON) |
			FT2BMP(WLC_IEM_FC_SCAN_BCN) |
			FT2BMP(WLC_IEM_FC_SCAN_PRBRSP) |
			FT2BMP(FC_PROBE_RESP);
		oce_cmn->esp_parse_data.ctx = oce;
		oce_cmn->esp_parse_h =
			wlc_esp_register_ie_parse_cb(wlc->esp,
				&oce_cmn->esp_parse_data);
#endif /* WL_ESP */
	}
	oce->oce_cmn_info = oce_cmn;
	(void)obj_registry_ref(wlc->objr, OBJR_OCE_CMN_INFO);

	oce->wlc = wlc;

	/* parse config */
	memset(&parse_cfg, 0, sizeof(parse_cfg));
	parse_cfg.alloc_fn = (bcm_iov_malloc_t)oce_iov_context_alloc;
	parse_cfg.free_fn = (bcm_iov_free_t)oce_iov_context_free;
	parse_cfg.dig_fn = (bcm_iov_get_digest_t)oce_iov_get_digest_cb;
	parse_cfg.max_regs = 1;
	parse_cfg.alloc_ctx = (void *)oce;

	/* parse context */
	ret = bcm_iov_create_parse_context((const bcm_iov_parse_config_t *)&parse_cfg,
		&parse_ctx);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s parse context creation failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	oce->iov_parse_ctx = parse_ctx;
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	if (wlc_dump_register(wlc->pub, "oce", wlc_oce_dump, oce) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_dumpe_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* BCMDBG || BCMDBG_DUMP */
	/* register module */
	ret = wlc_module_register(wlc->pub, oce_iovars, "oce", oce,
		wlc_oce_doiovar, wlc_oce_watchdog,
		wlc_oce_wlc_up, wlc_oce_wlc_down);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	memset(&cubby_params, 0, sizeof(cubby_params));

	cubby_params.context = oce;
	cubby_params.fn_init = wlc_oce_bsscfg_init;
	cubby_params.fn_deinit = wlc_oce_bsscfg_deinit;
	cubby_params.fn_get = wlc_oce_bsscfg_get;
	cubby_params.fn_set = wlc_oce_bsscfg_set;
	cubby_params.config_size = OCE_CUBBY_CFG_SIZE;

	oce->cfgh = wlc_bsscfg_cubby_reserve_ext(wlc, sizeof(wlc_oce_bsscfg_cubby_t *),
		&cubby_params);
	if (oce->cfgh < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
			wlc->pub->unit, __FUNCTION__));
			goto fail;
	}

	/* register oce subcommands */
	ret = bcm_iov_register_commands(oce->iov_parse_ctx, (void *)oce,
		&oce_sub_cmds[0], (size_t)SUBCMD_TBL_SZ(oce_sub_cmds), NULL, 0);

#ifdef WL_OCE_AP
	oce->oce_cmn_info->rej_retry_delay = OCE_ASSOC_REJECT_DEF_RETRY_DELAY;
	oce->fd_timer =wlc_hrt_alloc_timeout(oce->wlc->hrti);
	if (oce->fd_timer == NULL) {
		WL_ERROR(("wl%d: %s: wl_init_timer for fd timer failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	oce->rnr_scan_timer = wl_init_timer((struct wl_info *)oce->wlc->wl,
		wlc_oce_rnr_scan_timer, oce, "rnr_scan_timer");

	if (oce->rnr_scan_timer == NULL) {
		WL_ERROR(("wl%d: %s: wl_init_timer for rnr scan timer failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#ifdef WLMCNX
	oce->oce_cmn_info->fd_tx_duration = FD_TX_DURATION;
	if (MCNX_ENAB(oce->wlc->pub)) {
	ret = wlc_mcnx_intr_register(wlc->mcnx, wlc_oce_pretbtt_callback, oce);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: wlc_mcnx_intr_register failed (tbtt)\n",
		          wlc->pub->unit));
		goto fail;
		}
	}
#endif /* WLMCNX */
#endif /* WL_OCE_AP */

	ret = wlc_scan_utils_rx_scan_register(wlc, wlc_oce_scan_rx_callback, oce);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_scan_utils_rx_scan_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	ret = wlc_scan_start_register(wlc, wlc_oce_scan_start_callback, oce);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_scan_utils_scan_start_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register FILS elements build callbacks */
	ret = wlc_iem_add_build_fn_mft(wlc->iemi,
			fils_elm_build_fstbmp, DOT11_MNG_FILS_REQ_PARAMS,
			wlc_oce_fils_element_calc_len, wlc_oce_fils_element_build_fn, oce);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn_mft failed %d, \n",
			wlc->pub->unit, __FUNCTION__, ret));
		goto fail;
	}
#ifdef WL_OCE_AP
	/* register RNR element build callbacks */
	ret = wlc_iem_add_build_fn_mft(wlc->iemi,
			rnr_elm_build_fstbmp, DOT11_MNG_RNR_ID,
			wlc_oce_rnr_element_calc_len, wlc_oce_rnr_element_build_fn, oce);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn_mft failed %d, \n",
			wlc->pub->unit, __FUNCTION__, ret));
		goto fail;
	}
	/* ap channel report build callbacks */
	ret = wlc_iem_add_build_fn_mft(wlc->iemi,
			ap_chrep_elm_build_fstbmp, DOT11_MNG_AP_CHREP_ID,
			wlc_oce_ap_chrep_element_calc_len, wlc_oce_ap_chrep_element_build_fn, oce);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn_mft failed %d, \n",
			wlc->pub->unit, __FUNCTION__, ret));
		goto fail;
	}
#endif /* WL_OCE_AP */
	/* register RNR element parse callbacks */
	ret = wlc_iem_add_parse_fn_mft(wlc->iemi,
			rnr_elm_parse_fstbmp, DOT11_MNG_RNR_ID,
			wlc_oce_rnr_element_parse_fn, oce);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn_mft failed %d, \n",
			wlc->pub->unit, __FUNCTION__, ret));
		goto fail;
	}

	if ((oce->oce_cmn_info->ier_fd_act_frame = wlc_ier_create_registry(wlc->ieri,
		0, IEM_OCE_FD_FRAME_PARSE_CB_MAX)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_ier_create_registry failed, FILS Discovery Action frame\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	ret = wlc_ier_add_parse_fn(oce->oce_cmn_info->ier_fd_act_frame, DOT11_MNG_RNR_ID,
			wlc_oce_rnr_element_parse_fn, oce);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, RNR IE in FD Action frame\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	dll_init(&oce->prb_suppress_bss_list);
	oce->prb_suppress_bss_pool = dll_pool_init(wlc->osh,
		MAX_PROBE_SUPPRESS_BSS, sizeof(prb_suppress_bss_item_t));

	if (!oce->prb_suppress_bss_pool) {
		WL_ERROR(("wl%d: %s: pool init failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	oce->prb_suppress_bss_attr = MALLOCZ(wlc->osh,
		OCE_PROBE_SUPPRESS_BSSID_ATTR_SIZE + (ETHER_ADDR_LEN * MAX_PROBE_SUPPRESS_BSS));

	if (!oce->prb_suppress_bss_attr) {
		WL_ERROR(("wl%d: %s: prb_suppress_bss_attr alloc failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	oce->prb_suppress_ssid_attr = MALLOCZ(wlc->osh,
			OCE_PROBE_SUPPRESS_SSID_ATTR_SIZE + (SHORT_SSID_LEN * MAX_PROBE_SUPPRESS_BSS));

	if (!oce->prb_suppress_ssid_attr) {
		WL_ERROR(("wl%d: %s: prb_suppress_ssid_attr alloc failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register BSSLOAD IE parse callbacks */
	ret = wlc_iem_add_parse_fn_mft(wlc->iemi,
			bssload_parse_fstbmp, DOT11_MNG_QBSS_LOAD_ID,
			wlc_oce_bssload_ie_parse_fn, oce);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn_mft failed %d, \n",
			wlc->pub->unit, __FUNCTION__, ret));
		goto fail;
	}

	wlc->pub->cmn->_oce = TRUE;
	return oce;

fail:
	MODULE_DETACH(oce, wlc_oce_detach);
	return NULL;
}

void
BCMATTACHFN(wlc_oce_detach)(wlc_oce_info_t* oce)
{
	if (oce) {
		if (oce->oce_cmn_info &&
			obj_registry_unref(oce->wlc->objr, OBJR_OCE_CMN_INFO) == 0) {
			obj_registry_set(oce->wlc->objr, OBJR_OCE_CMN_INFO, NULL);

			if (oce->oce_cmn_info->ier_fd_act_frame != NULL) {
				wlc_ier_destroy_registry(oce->oce_cmn_info->ier_fd_act_frame);
			}

			wlc_oce_flush_prb_suppress_bss_list(oce);
			dll_pool_detach(oce->wlc->osh, oce->prb_suppress_bss_pool,
				MAX_PROBE_SUPPRESS_BSS, sizeof(prb_suppress_bss_item_t));

			if (oce->prb_suppress_ssid_attr) {
				MFREE(oce->wlc->osh, oce->prb_suppress_ssid_attr,
					OCE_PROBE_SUPPRESS_SSID_ATTR_SIZE
						 + (SHORT_SSID_LEN * MAX_PROBE_SUPPRESS_BSS));
			}

			if (oce->prb_suppress_bss_attr) {
				MFREE(oce->wlc->osh, oce->prb_suppress_bss_attr,
					OCE_PROBE_SUPPRESS_BSSID_ATTR_SIZE
						 + (ETHER_ADDR_LEN * MAX_PROBE_SUPPRESS_BSS));
			}

			wlc_mbo_oce_unregister_ie_build_cb(oce->wlc->mbo_oce,
				oce->oce_cmn_info->mbo_oce_build_h);
			wlc_mbo_oce_unregister_ie_parse_cb(oce->wlc->mbo_oce,
				oce->oce_cmn_info->mbo_oce_parse_h);
#ifdef WL_ESP
			wlc_esp_unregister_ie_parse_cb(oce->wlc->esp,
				oce->oce_cmn_info->esp_parse_h);
#endif
			MFREE(oce->wlc->osh, oce->oce_cmn_info, sizeof(*oce->oce_cmn_info));
			oce->oce_cmn_info = NULL;
		}

#ifdef WL_OCE_AP
		FD_TX_FREE_TIMER(oce);
		RNR_SCAN_FREE_TIMER(oce);
#ifdef WLMCNX
		if (MCNX_ENAB(oce->wlc->pub)) {
			wlc_mcnx_intr_unregister(oce->wlc->mcnx, wlc_oce_pretbtt_callback, oce);
		}
#endif
#endif /* WL_OCE_AP */

		wlc_scan_utils_rx_scan_unregister(oce->wlc, wlc_oce_scan_rx_callback, oce);
		wlc_scan_start_unregister(oce->wlc, wlc_oce_scan_start_callback, oce);

		(void)bcm_iov_free_parse_context(&oce->iov_parse_ctx,
			(bcm_iov_free_t)oce_iov_context_free);

		wlc_module_unregister(oce->wlc->pub, "oce", oce);
		MFREE(oce->wlc->osh, oce, sizeof(*oce));
		oce = NULL;
	}
}

static void
wlc_oce_watchdog(void *ctx)
{

}

static int
wlc_oce_wlc_up(void *ctx)
{
#ifdef WL_OCE_AP
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;

	oce->up_tick = OSL_SYSUPTIME();
	if (!OCE_ENAB(oce->wlc->pub)) {
		return BCME_OK;
	}

	/* let first scan happen as early as possible */
	RNR_SCAN_ADD_TIMER(oce, RNR_SCAN_START_PERIOD);
	wlc_oce_set_bcn_rate(oce);
#endif /* WL_OCE_AP */

	return BCME_OK;
}

static int
wlc_oce_wlc_down(void *ctx)
{
#ifdef WL_OCE_AP
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;

	FD_TX_DEL_TIMER(oce);
	RNR_SCAN_DEL_TIMER(oce);
#endif /* WL_OCE_AP */

	return BCME_OK;
}

static int
wlc_oce_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg)
{
	int ret = BCME_OK;
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;
	wlc_oce_bsscfg_cubby_t *obc = NULL;
	wlc_oce_bsscfg_cubby_t **pobc = NULL;
	wlc_info_t *wlc;
	ASSERT(oce != NULL);

	wlc = cfg->wlc;
	pobc = OCE_BSSCFG_CUBBY_LOC(oce, cfg);
	obc = MALLOCZ(wlc->osh, sizeof(*obc));
	if (obc == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		ret = BCME_NOMEM;
		goto fail;
	}
	*pobc = obc;
#ifdef WL_OCE_AP
	obc->rnr_nbr_ap_info = MALLOCZ(oce->wlc->osh, BCM_TLV_MAX_DATA_SIZE);
	if (obc->rnr_nbr_ap_info == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		ret = BCME_NOMEM;
		goto fail;
	}

	obc->rnrc_sssid_list = MALLOCZ(oce->wlc->osh, BCM_TLV_MAX_DATA_SIZE);
	if (obc->rnrc_sssid_list == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		ret = BCME_NOMEM;
		goto fail;
	}
#endif /* WL_OCE_AP */
	return ret;
fail:
	if (obc) {
		wlc_oce_bsscfg_deinit(ctx, cfg);
	}
	return ret;
}

static void
wlc_oce_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;
	wlc_oce_bsscfg_cubby_t *obc = NULL;
	wlc_oce_bsscfg_cubby_t **pobc = NULL;
	wlc_info_t *wlc;

	ASSERT(oce != NULL);
	wlc = cfg->wlc;
	pobc = OCE_BSSCFG_CUBBY_LOC(oce, cfg);
	obc = *pobc;
	if (obc != NULL) {
#ifdef WL_OCE_AP
		if (obc->rnr_nbr_ap_info) {
			MFREE(oce->wlc->osh, obc->rnr_nbr_ap_info, BCM_TLV_MAX_DATA_SIZE);
			obc->rnr_nbr_ap_info = NULL;
		}
		if (obc->rnrc_sssid_list) {
			MFREE(oce->wlc->osh, obc->rnrc_sssid_list, BCM_TLV_MAX_DATA_SIZE);
			obc->rnrc_sssid_list = NULL;
		}
#endif /* WL_OCE_AP */
		MFREE(wlc->osh, obc, sizeof(*obc));
	}
	pobc = NULL;
	return;
}

/* bsscfg copy :get function */
static int
wlc_oce_bsscfg_get(void *ctx, wlc_bsscfg_t *cfg, uint8 *data, int *len)
{
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;
	wlc_oce_bsscfg_cubby_t *obc = NULL;

	if (data == NULL || *len < (int)OCE_CUBBY_CFG_SIZE) {
		*len  = OCE_CUBBY_CFG_SIZE;
		return BCME_BUFTOOSHORT;
	}
	obc = OCE_BSSCFG_CUBBY(oce, cfg);
	if (obc == NULL) {
		*len = 0;
		return BCME_OK;
	}
	memcpy(data, obc, OCE_CUBBY_CFG_SIZE);
	*len = OCE_CUBBY_CFG_SIZE;
	/* reset the data pointers to NULL so that wlc_oce_bsscfg_deinit()
	 * doesn't free up allocated data
	 */
	memset(obc, 0, sizeof(*obc));

	return BCME_OK;
}

/* bsscfg copy: set function */
static int
wlc_oce_bsscfg_set(void *ctx, wlc_bsscfg_t *cfg, const uint8 *data, int len)
{
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;
	wlc_oce_bsscfg_cubby_t *obc = NULL;
	obc = OCE_BSSCFG_CUBBY(oce, cfg);
	if (obc == NULL) {
		return BCME_OK;
	}
	if (data == NULL || len < (int)OCE_CUBBY_CFG_SIZE) {
		return BCME_BADARG;
	}
	memcpy(obc, data, OCE_CUBBY_CFG_SIZE);
	return BCME_OK;
}

/* Return found linked list member equals to given bssid or null */
static oce_delayed_bssid_list_t*
wlc_oce_find_in_delayed_list(wlc_oce_info_t *oce, wlc_bsscfg_t *bsscfg,
	struct ether_addr *bssid)
{
	wlc_oce_bsscfg_cubby_t *occ = NULL;
	oce_delayed_bssid_list_t *cur = NULL;

	ASSERT(oce);
	ASSERT(bsscfg);

	occ = OCE_BSSCFG_CUBBY(oce, bsscfg);

	cur = occ->list_head;

	while (cur != NULL) {

		if (eacmp(bssid, &cur->bssid) == 0) {
			return cur;
		}
		cur = cur->next;
	}

	return NULL;
}

static int
wlc_oce_add_to_delayed_list(wlc_oce_info_t *oce, wlc_bsscfg_t *bsscfg,
	wlc_bss_info_t *bss, uint8 delay, uint8 delta)
{
	wlc_oce_bsscfg_cubby_t *occ = NULL;
	wlc_info_t *wlc = NULL;
	oce_delayed_bssid_list_t *new = NULL;
	struct ether_addr *bssid = &bss->BSSID;

	ASSERT(oce);
	ASSERT(bsscfg);

	wlc = oce->wlc;
	occ = OCE_BSSCFG_CUBBY(oce, bsscfg);

	new = wlc_oce_find_in_delayed_list(oce, bsscfg, bssid);
	if (new == NULL) {

		/* allocate */
		new = MALLOCZ(wlc->osh, sizeof(*new));
		if (new == NULL) {
			WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
				wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
			return BCME_NOMEM;
		}
		eacopy(bssid, &new->bssid);

		if (occ->list_head) {
			new->next = occ->list_head;
			occ->list_head->prev = new;
		}
		else {
			new->next = NULL;
		}

		new->prev = NULL;
		occ->list_head = new;
	}

	new->release_rssi = bss->RSSI + delta;
	new->release_tm = OSL_SYSUPTIME() + delay * 1000;

	return BCME_OK;
}

static int
wlc_oce_rm_from_delayed_list(wlc_oce_info_t *oce, wlc_bsscfg_t *bsscfg,
	oce_delayed_bssid_list_t *mmbr)
{
	ASSERT(oce);
	ASSERT(bsscfg);
	ASSERT(mmbr);

	if (mmbr->prev)
		mmbr->prev->next = mmbr->next;

	if (mmbr->next)
		mmbr->next->prev = mmbr->prev;

	if (mmbr->prev == NULL) {
		wlc_oce_bsscfg_cubby_t * occ = OCE_BSSCFG_CUBBY(oce, bsscfg);

		if (mmbr->next == NULL) {
			occ->list_head = NULL;
		} else {
			occ->list_head = mmbr->next;
		}
	}

	MFREE(oce->wlc->osh, mmbr, sizeof(*mmbr));

	return BCME_OK;
}

static int
wlc_oce_rm_delayed_list(wlc_oce_info_t *oce, wlc_bsscfg_t *bsscfg)
{
	oce_delayed_bssid_list_t *mmbr = NULL;
	wlc_info_t *wlc = NULL;
	wlc_oce_bsscfg_cubby_t *occ = NULL;

	ASSERT(oce);
	ASSERT(bsscfg);

	occ = OCE_BSSCFG_CUBBY(oce, bsscfg);
	mmbr = occ->list_head;
	if (mmbr == NULL)
		return BCME_OK;

	wlc = oce->wlc;

	while (mmbr != NULL) {
		oce_delayed_bssid_list_t *tmp = mmbr->next;

		MFREE(wlc->osh, mmbr, sizeof(*mmbr));

		mmbr = tmp;
	}

	occ->list_head = NULL;

	return BCME_OK;
}

/* Return found linked list member equals to given bssid or null */
static prb_suppress_bss_item_t*
wlc_oce_find_in_prb_suppress_bss_list(wlc_oce_info_t *oce,
	struct ether_addr *bssid)
{
	dll_t *item_p, *next_p;
	ASSERT(oce);

	for (item_p = dll_head_p(&oce->prb_suppress_bss_list);
		!dll_end(&oce->prb_suppress_bss_list, item_p);
		item_p = next_p)
	{
		prb_suppress_bss_item_t *p = (prb_suppress_bss_item_t*)item_p;

		if (eacmp(bssid, &p->bssid) == 0) {
			return p;
		}

		next_p = dll_next_p(item_p);
	}

	return NULL;
}

static int
wlc_oce_add_to_prb_suppress_bss_list(wlc_oce_info_t *oce,
	struct ether_addr *bssid, uint32 short_ssid, uint32 ts, uint32 time_to_tbtt, bool bcn_prb)
{
	prb_suppress_bss_item_t *new = NULL;
	ASSERT(oce);

	new = wlc_oce_find_in_prb_suppress_bss_list(oce, bssid);

	if (new == NULL) {
		/* allocate */
		new = dll_pool_alloc(oce->prb_suppress_bss_pool);
		if (new == NULL) {
			WL_ERROR(("wl%d: %s: alloc failed. out of pool memory\n",
				oce->wlc->pub->unit, __FUNCTION__));
			return BCME_NOMEM;
		}
		eacopy(bssid, &new->bssid);
		new->short_ssid = short_ssid;

		dll_append(&oce->prb_suppress_bss_list, (dll_t *)new);
		oce->prb_suppress_bss_count++;
	}

	/* override FILS DF with beacon and probe response */
	if (bcn_prb) {
		new->bcn_prb = bcn_prb;
	} else {
		new->ts = ts;
		new->time_to_tbtt = time_to_tbtt;
	}

	return BCME_OK;
}

void
wlc_oce_flush_prb_suppress_bss_list(wlc_oce_info_t *oce)
{
	dll_t *item_p, *next_p;

	ASSERT(oce);

	for (item_p = dll_head_p(&oce->prb_suppress_bss_list);
		!dll_end(&oce->prb_suppress_bss_list, item_p);
		item_p = next_p)
	{
		prb_suppress_bss_item_t *p;
		next_p = dll_next_p(item_p);

		p = (prb_suppress_bss_item_t*)item_p;

		memset(&p->bssid, 0, sizeof(p->bssid));
		p->short_ssid = 0;
		p->bcn_prb = 0;

		dll_delete(item_p);
		dll_pool_free(oce->prb_suppress_bss_pool, item_p);

		oce->prb_suppress_bss_count--;
	}
}

static prb_suppress_bss_item_t*
wlc_oce_get_highest_tbtt_from_suppressed_list(wlc_oce_info_t *oce)
{
	prb_suppress_bss_item_t* bss = NULL;
	uint32 tbtt = 0;
	dll_t *item_p, *next_p;

	ASSERT(oce);

	for (item_p = dll_head_p(&oce->prb_suppress_bss_list);
		!dll_end(&oce->prb_suppress_bss_list, item_p);
		item_p = next_p)
	{
		prb_suppress_bss_item_t *p = (prb_suppress_bss_item_t*)item_p;

		if (!p->bcn_prb && tbtt < p->time_to_tbtt) {
			tbtt = p->time_to_tbtt;
			bss = p;
		}

		next_p = dll_next_p(item_p);
	}

	return bss;
}

static void
wlc_oce_update_current_slot_duration(wlc_oce_info_t *oce)
{
	prb_suppress_bss_item_t* bss;

	bss = wlc_oce_get_highest_tbtt_from_suppressed_list(oce);

	if (bss) {
		wlc_info_t *wlc = oce->wlc;
		uint32 tsf_l, tsf_h, duration, d = 0;

		wlc_read_tsf(wlc, &tsf_l, &tsf_h);

		/* time since fils df reception */
		if (tsf_l > bss->ts) {
			d = tsf_l - bss->ts;
		} else {
			/* wrap around */
			d = MAX_TSF_L - (bss->ts - tsf_l);
		}

		if (d >= bss->time_to_tbtt) {
			WL_INFORM(("OCE skip bcn time update, d %d >= tbtt %d\n",
				d, bss->time_to_tbtt));
			return;
		}

		duration = (bss->time_to_tbtt - d) + TBTT_WAIT_EXTRA_TIME;

		if (duration > TBTT_WAIT_MAX_TIME) {
			WL_INFORM(("OCE: required slot duration is too long %d us\n", duration));
			return;
		}

		WL_INFORM(("OCE update current slot duration to %d us\n", duration));

		wlc_scan_update_current_slot_duration(wlc, duration);
	}

	return;
}

/* Check whether pointed-to IE looks like MBO_OCE. */
#define bcm_is_mbo_oce_ie(ie, tlvs, len)	bcm_has_ie(ie, tlvs, len, \
	(const uint8 *)MBO_OCE_OUI, WFA_OUI_LEN, MBO_OCE_OUI_TYPE)

/* Return OCE Capabilities Indication Attribute or NULL */
wifi_oce_cap_ind_attr_t*
wlc_oce_find_cap_ind_attr(uint8 *parse, uint16 buf_len)
{
	bcm_tlv_t *ie;
	uint len = buf_len;

	while ((ie = bcm_parse_tlvs(parse, (int)len, MBO_OCE_IE_ID))) {
		if (bcm_is_mbo_oce_ie((uint8 *)ie, &parse, &len)) {

			wifi_oce_cap_ind_attr_t *oce_cap_ind_attr;
			uint16 len_tmp = ie->len - MBO_OCE_IE_NO_ATTR_LEN;
			uint8 *ie_buf = (uint8*)ie + MBO_OCE_IE_HDR_SIZE;

			if (ie->len < (MBO_OCE_IE_NO_ATTR_LEN + OCE_CAP_INDICATION_ATTR_SIZE)) {
				break;
			}

			oce_cap_ind_attr = (wifi_oce_cap_ind_attr_t*)bcm_parse_tlvs(ie_buf,
				len_tmp, OCE_ATTR_OCE_CAPABILITY_INDICATION);

			if (oce_cap_ind_attr) {
				return oce_cap_ind_attr;
			} else if (len > (ie->len + (uint)TLV_HDR_LEN)) {
				parse += (ie->len + TLV_HDR_LEN);
				len -= (ie->len + TLV_HDR_LEN);
			} else {
				break;
			}
		}
	}

	return NULL;
}

/* Return OCE BSSID Attribute or NULL */
wifi_oce_probe_suppress_bssid_attr_t*
wlc_oce_get_prb_suppr_bssid_attr(uint8 *parse, uint16 buf_len)
{
	bcm_tlv_t *ie;
	uint len = buf_len;

	while ((ie = bcm_parse_tlvs(parse, (int)len, MBO_OCE_IE_ID))) {
		if (bcm_is_mbo_oce_ie((uint8 *)ie, &parse, &len)) {

			wifi_oce_probe_suppress_bssid_attr_t *oce_bssid_suppr_attr;
			uint16 len_tmp = ie->len - MBO_OCE_IE_NO_ATTR_LEN;
			uint8 *ie_buf = (uint8*)ie + MBO_OCE_IE_HDR_SIZE;

			if (ie->len < (MBO_OCE_IE_NO_ATTR_LEN + OCE_CAP_INDICATION_ATTR_SIZE + OCE_PROBE_SUPPRESS_BSSID_ATTR_SIZE)) {
				break;
			}

			oce_bssid_suppr_attr = (wifi_oce_probe_suppress_bssid_attr_t *)bcm_parse_tlvs(ie_buf,
				len_tmp, OCE_ATTR_PROBE_SUPPRESS_BSSID);

			if (oce_bssid_suppr_attr) {
				return oce_bssid_suppr_attr;
			} else if (len > (ie->len + (uint)TLV_HDR_LEN)) {
				parse += (ie->len + TLV_HDR_LEN);
				len -= (ie->len + TLV_HDR_LEN);
			} else {
				break;
			}
		}
	}

	return NULL;
}

/* Return OCE SSID Attribute or NULL */
wifi_oce_probe_suppress_ssid_attr_t*
wlc_oce_get_prb_suppr_ssid_attr(uint8 *parse, uint16 buf_len)
{
	bcm_tlv_t *ie;
	uint len = buf_len;

	while ((ie = bcm_parse_tlvs(parse, (int)len, MBO_OCE_IE_ID))) {
		if (bcm_is_mbo_oce_ie((uint8 *)ie, &parse, &len)) {

			wifi_oce_probe_suppress_ssid_attr_t *oce_ssid_suppr_attr;
			uint16 len_tmp = ie->len - MBO_OCE_IE_NO_ATTR_LEN;
			uint8 *ie_buf = (uint8*)ie + MBO_OCE_IE_HDR_SIZE;

			if (ie->len < (MBO_OCE_IE_NO_ATTR_LEN + OCE_CAP_INDICATION_ATTR_SIZE +
				OCE_PROBE_SUPPRESS_SSID_ATTR_SIZE)) {
				break;
			}

			oce_ssid_suppr_attr = (wifi_oce_probe_suppress_ssid_attr_t *)
				bcm_parse_tlvs(ie_buf, len_tmp, OCE_ATTR_PROBE_SUPPRESS_SSID);

			if (oce_ssid_suppr_attr) {
				return oce_ssid_suppr_attr;
			} else if (len > (ie->len + (uint)TLV_HDR_LEN)) {
				parse += (ie->len + TLV_HDR_LEN);
				len -= (ie->len + TLV_HDR_LEN);
			} else {
				break;
			}
		}
	}

	return NULL;
}


static bool
wlc_oce_check_oce_capability(scan_utl_scan_data_t *sd)
{
	if (wlc_oce_find_cap_ind_attr(sd->body + DOT11_BCN_PRB_LEN,
			sd->body_len - DOT11_BCN_PRB_LEN)) {
		return TRUE;
	}

	return FALSE;
}

static void
wlc_oce_scan_rx_callback(void* ctx, scan_utl_scan_data_t *sd)
{
	wlc_oce_info_t *oce;
	wlc_info_t *wlc;

	ASSERT(ctx && sd);

	oce = (wlc_oce_info_t *)ctx;
	wlc = oce->wlc;

	if (!ETHER_ISMULTI(&wlc->scan->bssid) &&
	    !ETHER_ISNULLADDR(&wlc->scan->bssid)) {
		wlc_scan_probe_suppress(wlc->scan);
	}

	if (wlc_oce_check_oce_capability(sd)) {
#if defined(BCMDBG) || defined(WLMSG_INFORM)
		char eabuf[ETHER_ADDR_STR_LEN];
#endif
		uint32 sup_short_ssid = ~hndcrc32(sd->bi->SSID, sd->bi->SSID_len, CRC32_INIT_VALUE);

		WL_OCE_INFO(("prb/bcn: add bssid %s ssid %s short_ssid %x to the list\n",
			bcm_ether_ntoa(&sd->bi->BSSID, eabuf),
			oce->scan_ssid, sup_short_ssid));

		wlc_oce_add_to_prb_suppress_bss_list(oce, &sd->bi->BSSID, sup_short_ssid,
			0, 0, TRUE);
	}

	wlc_oce_detect_environment(oce, sd->body, sd->body_len);
}

static void
wlc_oce_detect_environment(wlc_oce_info_t *oce, uint8* body, uint16 body_len)
{
	if (!oce->oce_cmn_info->env_inc_non_oce_ap) {
		wifi_oce_cap_ind_attr_t* oce_cap_ind_attr;
			oce_cap_ind_attr = wlc_oce_find_cap_ind_attr(body + DOT11_BCN_PRB_LEN,
				body_len - DOT11_BCN_PRB_LEN);
		if (!oce_cap_ind_attr) {
			oce->oce_cmn_info->env_inc_non_oce_ap = TRUE;
		}
	}
}

static void
wlc_oce_reset_environment(wlc_oce_info_t* oce)
{
	oce->oce_cmn_info->env_inc_non_oce_ap = FALSE;
}

static void
wlc_oce_scan_start_callback(void *ctx, wlc_ssid_t *ssid)
{
	wlc_oce_info_t *oce;

	ASSERT(ctx);
	oce = (wlc_oce_info_t *)ctx;

	if ((ssid != NULL) && ssid->SSID_len) {
		if (ssid->SSID_len > DOT11_MAX_SSID_LEN) {
			WL_ERROR(("wl%d: %s: bad ssid length %d\n",
				oce->wlc->pub->unit, __FUNCTION__, ssid->SSID_len));
			return;
		}

		memcpy(oce->scan_ssid, ssid->SSID, ssid->SSID_len);

		oce->scan_ssid_len = ssid->SSID_len;

		oce->short_ssid = ~hndcrc32(ssid->SSID,
			ssid->SSID_len, CRC32_INIT_VALUE);
	} else {
		oce->scan_ssid_len = 0;
		oce->short_ssid = 0;
	}

	WL_OCE_INFO(("wl%d: %s:  scan ssid %s short_ssid %x len %d\n",
		oce->wlc->pub->unit, __FUNCTION__, oce->scan_ssid,
		oce->short_ssid, ssid->SSID_len));

	wlc_oce_reset_environment(oce);
}

/* Pick up rate compatible with OCE requirements (minimum 5.5 mbps) */
static ratespec_t
wlc_oce_get_oce_compat_rspec(wlc_oce_info_t *oce)
{
	wlc_info_t *wlc = oce->wlc;
	wlc_rateset_t default_rateset;
	wlc_rateset_t basic_rateset;
	uint i;

	wlc_default_rateset(wlc, &default_rateset);

	/* filter basic rates */
	wlc_rateset_filter(&default_rateset, &basic_rateset,
			TRUE, WLC_RATES_CCK_OFDM, RATE_MASK_FULL, FALSE);

	for (i = 0; i < basic_rateset.count; i++) {
		uint8 r = basic_rateset.rates[i] & RATE_MASK;
		if (r >= WLC_RATE_5M5) {
			return OFDM_RSPEC(r);
		}
	}

	return OFDM_RSPEC(WLC_RATE_1M);
}

/* OCE probe request. */
/* Transmit rate is 5.5 Mbps minimum (WFA OCE spec v5 3.3) */
bool
wlc_oce_send_probe(wlc_oce_info_t *oce, void *p)
{
	wlc_info_t *wlc = oce->wlc;
	ratespec_t rate_override;

	if (wlc_scan_get_num_passes_left(wlc) == 1) {
		wlc_oce_update_current_slot_duration(wlc->oce);
	}

	wlc->scan->state &= ~SCAN_STATE_PRB_DEFERRED;

	if ((BAND_5G(wlc->band->bandtype))) {
		return wlc_sendmgmt(wlc, p, wlc->active_queue, NULL);
	}


	if (oce->oce_cmn_info->disable_oce_prb_req_rate == TRUE)
		rate_override = WLC_LOWEST_BAND_RSPEC(wlc->band);
	else
		rate_override = wlc_oce_get_oce_compat_rspec(oce);

	return wlc_queue_80211_frag(wlc, p, wlc->active_queue,
		NULL, NULL, FALSE, NULL, rate_override);
}

/* Return OCE Probe Request Deferral time (WFA OCE spec v5 3.2) */
uint8
wlc_oce_get_probe_defer_time(wlc_oce_info_t *oce)
{
	ASSERT(oce);

	return oce->oce_cmn_info->probe_defer_time;
}

bool
wlc_oce_is_oce_environment(wlc_oce_info_t *oce)
{
	ASSERT(oce);

	return (oce->oce_cmn_info->env_inc_non_oce_ap ? FALSE : TRUE);
}

/* Write OCE Capabilities Attribute,
 * OCE Tech Spec v5 section 4.2.1.
 */
static uint16
wlc_oce_write_oce_cap_ind_attr(uint8 *cp, uint8 buf_len, bool ap)
{
	uint8 data = OCE_RELEASE;

#ifdef WLMCNX
/* Only for CFON*/
	if (ap) {
		data |= STA_CFON;
	}
#endif

	ASSERT(buf_len >= OCE_CAP_INDICATION_ATTR_SIZE);

	bcm_write_tlv(OCE_ATTR_OCE_CAPABILITY_INDICATION, &data, sizeof(data), cp);

	return OCE_CAP_INDICATION_ATTR_SIZE;
}

static uint16
wlc_oce_get_prb_suppress_bssid_attr_len(wlc_oce_info_t *oce)
{
	uint16 attr_size = 0;

	ASSERT(oce);

	if (!oce->prb_suppress_bss_count) {
		return 0;
	}

	attr_size = OCE_PROBE_SUPPRESS_BSSID_ATTR_SIZE
		+ (ETHER_ADDR_LEN * oce->prb_suppress_bss_count);

	return attr_size;
}

/* Write Probe Suppress BSSID Attribute,
 * OCE Tech Spec v0.0.11 section 4.2.5.
 */
static uint16
wlc_oce_write_prb_suppress_bssid_attr(wlc_oce_info_t *oce, uint8 *cp, uint8 buf_len)
{
	uint16 attr_size = 0;
	uint8* ptr;
	dll_t *item_p, *next_p;

	ASSERT(oce);

	attr_size = wlc_oce_get_prb_suppress_bssid_attr_len(oce);

	if (!attr_size) {
		return 0;
	}

	if (buf_len < attr_size) {
		ASSERT(0);
		return 0;
	}

	ptr = oce->prb_suppress_bss_attr->bssid_list;

	for (item_p = dll_head_p(&oce->prb_suppress_bss_list);
		!dll_end(&oce->prb_suppress_bss_list, item_p);
		item_p = next_p)
	{
		prb_suppress_bss_item_t *p = (prb_suppress_bss_item_t*)item_p;

		eacopy((uint8*)&p->bssid, ptr);
		ptr += ETHER_ADDR_LEN;

		next_p = dll_next_p(item_p);
	}

	bcm_write_tlv(OCE_ATTR_PROBE_SUPPRESS_BSSID,
		oce->prb_suppress_bss_attr->bssid_list,
		attr_size - OCE_PROBE_SUPPRESS_BSSID_ATTR_SIZE, cp);
	return attr_size;
}

static uint16
wlc_oce_get_prb_suppress_ssid_attr_len(wlc_oce_info_t *oce)
{
	uint8 attr_size = 0, len = 0;
	dll_t *item_p;

	ASSERT(oce);

	if (!oce->prb_suppress_bss_count) {
		return 0;
	}

	for (item_p = dll_head_p(&oce->prb_suppress_bss_list);
		!dll_end(&oce->prb_suppress_bss_list, item_p);
		item_p = dll_next_p(item_p))
	{
		dll_t *rnr_p;
		prb_suppress_bss_item_t *p = (prb_suppress_bss_item_t*)item_p;

		if (p->short_ssid == 0) {
			continue;
		}

		/* XXX: Iterate thru the list and clear dups since our dll is
		 * small as MAX_PROBE_SUPPRESS_BSS (10).
		 */
		for (rnr_p = dll_next_p(item_p);
			!dll_end(&oce->prb_suppress_bss_list, rnr_p);
			rnr_p = dll_next_p(rnr_p))
		{
			prb_suppress_bss_item_t *ip = (prb_suppress_bss_item_t*)rnr_p;

			if (ip->short_ssid == p->short_ssid) {
				ip->short_ssid = 0;
			}
		}

		/* valid short_ssid */
		len += SHORT_SSID_LEN;
	}

	attr_size = OCE_PROBE_SUPPRESS_SSID_ATTR_SIZE + len;

	return attr_size;
}

/* Write Probe Suppress SSID Attribute, ********
 * OCE Tech Spec v0.0.11 section 4.2.6.
 */
static uint16
wlc_oce_write_prb_suppress_ssid_attr(wlc_oce_info_t *oce, uint8 *cp, uint8 buf_len)
{
	uint16 attr_size = 0;
	dll_t *item_p;
	uint8 *ptr;

	ASSERT(oce);

	attr_size = wlc_oce_get_prb_suppress_ssid_attr_len(oce);

	if (!attr_size) {
		return 0;
	}

	if (buf_len < attr_size) {
		ASSERT(0);
		return 0;
	}

	ptr = oce->prb_suppress_ssid_attr->ssid_list;
	bzero(ptr, SHORT_SSID_LEN * MAX_PROBE_SUPPRESS_BSS);

	/* iterate thru the list and copy non-zero short ssids */
	for (item_p = dll_head_p(&oce->prb_suppress_bss_list);
		!dll_end(&oce->prb_suppress_bss_list, item_p);
		item_p = dll_next_p(item_p))
	{
		prb_suppress_bss_item_t *p = (prb_suppress_bss_item_t*)item_p;

		if (p->short_ssid) {
			htol32_ua_store(p->short_ssid, ptr);
			ptr += SHORT_SSID_LEN;
		}
	}

	bcm_write_tlv(OCE_ATTR_PROBE_SUPPRESS_SSID,
		oce->prb_suppress_ssid_attr->ssid_list,
		attr_size - OCE_PROBE_SUPPRESS_SSID_ATTR_SIZE, cp);
	return attr_size;
}


#ifdef WL_OCE_AP
/* Write OCE Association Rejection Attribute,
 * OCE Tech Spec v5 section 4.2.2.
 */
static uint16
wlc_oce_write_oce_ass_rej_attr(uint8 *cp, uint8 buf_len, uint8 rssi_delta, uint8 rej_retry_delay)
{
	/* Attribute data - delta RSSI, retry delay (secs) */
	uint16 data = (rej_retry_delay << 8) + rssi_delta;

	ASSERT(buf_len >= OCE_RSSI_ASSOC_REJ_ATTR_SIZE);

	bcm_write_tlv(OCE_ATTR_RSSI_BASED_ASSOC_REJECTION, &data, sizeof(data), cp);

	return OCE_RSSI_ASSOC_REJ_ATTR_SIZE;
}

/* Write OCE Reduced WAN Metrics attribute,
 * OCE Tech Spec v5 section 4.2.3.
 */
static uint16
wlc_oce_write_reduced_wan_metr_attr(wlc_oce_cmn_info_t *oce_cmn, uint8 *cp, uint8 buf_len)
{
	uint8 data;

	ASSERT(buf_len >= OCE_REDUCED_WAN_METR_ATTR_SIZE);

	/* Attribute data -  Uplink Available Capacity (4 bits),
	 * Downlink Available Capacity (4 bits)
	 */
	data = oce_cmn->red_wan_links;

	bcm_write_tlv(OCE_ATTR_REDUCED_WAN_METRICS, &data, sizeof(data), cp);

	return OCE_REDUCED_WAN_METR_ATTR_SIZE;
}

uint16
wlc_oce_if_valid_assoc(wlc_oce_info_t *oce, int8 *rssi, uint8 *rej_rssi_delta)
{
	ASSERT(oce);

	if (oce->oce_cmn_info->ass_rej_rssi_thd &&
		(*rssi < oce->oce_cmn_info->ass_rej_rssi_thd)) {

		*rej_rssi_delta = oce->oce_cmn_info->ass_rej_rssi_thd - *rssi;
		/* XXX: delta cannot be higher than diff of max and min */
		ASSERT(*rej_rssi_delta <= (OCE_MAX_RSSI_TH - OCE_MIN_RSSI_TH));
		return OCE_ASSOC_REJECT_RC_INSUFFICIENT_RSSI;
	}

	return DOT11_SC_SUCCESS;
}

/* Write OCE RNR completeness attribute,
 * OCE Tech Spec v18 section 4.2.4.
 */
static uint16
wlc_oce_get_rnr_completeness_attr_len(wlc_oce_info_t *oce, wlc_bsscfg_t *cfg)
{
	uint16 attr_size = 0;
	wlc_oce_bsscfg_cubby_t *obc;

	ASSERT(oce);
	ASSERT(cfg);

	obc = OCE_BSSCFG_CUBBY(oce, cfg);
	ASSERT(obc);

	if (!obc->rnrc_sssid_list_size) {
		return 0;
	}

	attr_size = OCE_RNR_COMPLETENESS_ATTR_SIZE + obc->rnrc_sssid_list_size;

	return attr_size;
}

/* Write OCE RNR completeness attribute,
 * OCE Tech Spec v18 section 4.2.4.
 */
static uint16
wlc_oce_write_rnr_completeness_attr(wlc_oce_info_t *oce, wlc_bsscfg_t *cfg, uint8 *cp,
	uint8 buf_len)
{
	uint16 attr_size = 0;
	wlc_oce_bsscfg_cubby_t *obc;

	ASSERT(oce);
	ASSERT(cfg);

	obc = OCE_BSSCFG_CUBBY(oce, cfg);
	if (!(attr_size = wlc_oce_get_rnr_completeness_attr_len(oce, cfg))) {
		return 0;
	}

	if (buf_len < attr_size) {
		ASSERT(0);
		return 0;
	}

	/* attr_size - OCE_RNR_COMPLETENESS_ATTR_SIZE */
	bcm_write_tlv(OCE_ATTR_RNR_COMPLETENESS,
		obc->rnrc_sssid_list,
		attr_size - OCE_RNR_COMPLETENESS_ATTR_SIZE, cp);

	return attr_size;
}
#endif /* WL_OCE_AP */

static void
wlc_oce_add_cap_ind(wlc_mbo_oce_attr_build_data_t *data, uint16 *attr_len,
	uint16 *total_len, uint8 *cp, bool ap)
{
	uint16 buf_len = data->buf_len;

	if (!data->buf_len) {
		/* return attributes length if buffer len is 0 */
		*total_len += OCE_CAP_INDICATION_ATTR_SIZE;
	} else {
		/* write OCE Capability Indication attr */
		*attr_len = wlc_oce_write_oce_cap_ind_attr(cp, buf_len, ap);
		cp += *attr_len;
		buf_len -= *attr_len;
		*total_len += *attr_len;
	}
}

/* Add OCE attributes to MBO_OCE IE */
static int
wlc_oce_ie_build_fn(void *ctx, wlc_mbo_oce_attr_build_data_t *data)
{
	uint8 *cp = NULL;

	uint16 attr_len = 0;
	uint16 total_len = 0;
	wlc_iem_ft_cbparm_t *ft;
	uint16 buf_len = data->buf_len;
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;

	if (!OCE_ENAB(oce->wlc->pub)) {
		return 0;
	}
	cp = data->buf;
	ft = data->cbparm->ft;

	switch (data->ft) {
		case FC_ASSOC_RESP:
		case FC_REASSOC_RESP:
#ifdef WL_OCE_AP
		{
			if (ft->assocresp.status == OCE_ASSOC_REJECT_RC_INSUFFICIENT_RSSI) {
				if (!data->buf_len) {
					/* return attributes length if buffer len is 0 */
					total_len += OCE_RSSI_ASSOC_REJ_ATTR_SIZE;
				} else {
					/* write OCE Association Rejection attr */
					attr_len = wlc_oce_write_oce_ass_rej_attr(cp, buf_len,
						ft->assocresp.status_data, oce->oce_cmn_info->rej_retry_delay);
					cp += attr_len;
					buf_len += attr_len;
					total_len += attr_len;
				}
			}

				wlc_oce_add_cap_ind(data, &attr_len, &total_len, cp, TRUE);
			}
		break;
#endif /* WL_OCE_AP */
		case FC_BEACON:
		case FC_PROBE_RESP:
#ifdef WL_OCE_AP
		{
			if (!data->buf_len) {
				/* return attributes length if buffer len is 0 */
				total_len += OCE_REDUCED_WAN_METR_ATTR_SIZE;
				total_len += wlc_oce_get_rnr_completeness_attr_len(oce, data->cfg);
			} else {
				/* write OCE Reduced WAN Metrics attr */
				attr_len = wlc_oce_write_reduced_wan_metr_attr(
								oce->oce_cmn_info,
								cp, buf_len);
				cp += attr_len;
				buf_len -= attr_len;
				total_len += attr_len;
				/* write RNR completeness attribute */
				attr_len = wlc_oce_write_rnr_completeness_attr(oce, data->cfg, cp,
					buf_len);
				cp += attr_len;
				buf_len -= attr_len;
				total_len += attr_len;
			}

			wlc_oce_add_cap_ind(data, &attr_len, &total_len, cp, TRUE);
		}
		break;
#endif /* WL_OCE_AP */
		case FC_ASSOC_REQ:
		case FC_REASSOC_REQ:
			wlc_oce_add_cap_ind(data, &attr_len, &total_len, cp, FALSE);
			break;
		case FC_PROBE_REQ:
		{
			if (!ft->prbreq.da || ETHER_ISBCAST(ft->prbreq.da)) {
				if (!data->buf_len) {
					/* return attributes length if buffer len is 0 */
					total_len += wlc_oce_get_prb_suppress_bssid_attr_len(oce);
					/* add ssid attribute if wildcard SSID */
					if (!ft->prbreq.ssid_len || !ft->prbreq.ssid) {
						total_len +=
							wlc_oce_get_prb_suppress_ssid_attr_len(oce);
					}
				} else {
					/* write Probe Suppression BSSID attr */
					attr_len = wlc_oce_write_prb_suppress_bssid_attr(oce,
							cp, buf_len);
					cp += attr_len;
					buf_len -= attr_len;
					total_len += attr_len;

					/* add ssid attribute if wildcard SSID */
					if (!ft->prbreq.ssid_len || !ft->prbreq.ssid) {
						/* write Probe Suppression SSID attr */
						attr_len = wlc_oce_write_prb_suppress_ssid_attr(oce,
							cp, buf_len);
						cp += attr_len;
						buf_len -= attr_len;
						total_len += attr_len;
					}
				}
			}
			wlc_oce_add_cap_ind(data, &attr_len, &total_len, cp, FALSE);
		}
			break;
		default:
			ASSERT(0);
	}

	return total_len;
}

static void
wlc_oce_parse_bcnprb_at_scan(wlc_oce_info_t *oce,
	wlc_mbo_oce_attr_parse_data_t *data)
{
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->scan.result;
	oce_delayed_bssid_list_t *to_find = NULL;
	wlc_bsscfg_t *bsscfg = NULL;
	wifi_oce_reduced_wan_metrics_attr_t *rwan_met = NULL;

	bsscfg = data->cfg;

	to_find = wlc_oce_find_in_delayed_list(oce, bsscfg, &bi->BSSID);
	if (to_find) {
		uint32 cur_tm = OSL_SYSUPTIME();

		/* Updated due to OCE TechSpec v0.0.11
		 * BSSID found in delayed list.
		 * If it's delay period expired OR
		 * RSSI level gets min needed, so
		 * remove this BSSID from delyed list.
		 */
		if ((cur_tm >= to_find->release_tm) ||
		    (bi->RSSI >= to_find->release_rssi)) {
			wlc_oce_rm_from_delayed_list(oce, bsscfg, to_find);
		}
		else
			bi->flags2 |= WLC_BSS_OCE_ASSOC_REJD;
	}

	/* OCE Reduced WAN Metrics attribute */
	rwan_met = (wifi_oce_reduced_wan_metrics_attr_t *)
		bcm_parse_tlvs(data->ie, data->ie_len, OCE_ATTR_REDUCED_WAN_METRICS);

	bi->rwan_links = rwan_met->avail_capacity;
}

bool
wlc_oce_get_trigger_rwan_roam(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	wlc_oce_bsscfg_cubby_t *occ = OCE_BSSCFG_CUBBY(wlc->oce, cfg);

	return occ->rwan_statcs.roam_trigger;
}

void
wlc_oce_reset_rwan_statcs(wlc_bsscfg_t *cfg)
{
	wlc_oce_bsscfg_cubby_t *occ = NULL;
	wlc_oce_rwan_statcs_t *rwan_statcs = NULL;

	occ = OCE_BSSCFG_CUBBY(cfg->wlc->oce, cfg);
	rwan_statcs = &occ->rwan_statcs;

	rwan_statcs->roam_trigger = FALSE;
	rwan_statcs->next_window = 0;
}

static void
wlc_oce_parse_bcnprb(wlc_oce_info_t *oce, wlc_mbo_oce_attr_parse_data_t *data)
{
	wlc_bsscfg_t *bsscfg = NULL;
	wifi_oce_reduced_wan_metrics_attr_t *rwan_met = NULL;
	wlc_oce_bsscfg_cubby_t *occ = NULL;
	wlc_oce_rwan_info_t *rwan_info = NULL;
	wlc_oce_rwan_statcs_t *rwan_statcs = NULL;
	uint8 uplink, downlink;

	bsscfg = data->cfg;
	occ = OCE_BSSCFG_CUBBY(oce, bsscfg);
	rwan_info = &oce->oce_cmn_info->rwan_info;
	rwan_statcs = &occ->rwan_statcs;

	/* OCE Reduced WAN Metrics attribute */
	rwan_met = (wifi_oce_reduced_wan_metrics_attr_t *)
		bcm_parse_tlvs(data->ie, data->ie_len,
		OCE_ATTR_REDUCED_WAN_METRICS);

	if (rwan_met == NULL) {
		return;
	}

	uplink = (rwan_met->avail_capacity >> 4) & 0xf;
	downlink = rwan_met->avail_capacity & 0xf;

	if (((uplink >= rwan_info->uplink_trigger) &&
	    downlink >= rwan_info->downlink_trigger)) {
		wlc_oce_reset_rwan_statcs(bsscfg);

		return;
	}

	if (!rwan_statcs->next_window)
		rwan_statcs->next_window = bsscfg->wlc->pub->now + rwan_info->window_time;

	if (rwan_statcs->next_window <= bsscfg->wlc->pub->now)
		rwan_statcs->roam_trigger = TRUE;
}

static int
wlc_oce_parse_assoc(wlc_oce_info_t *oce, wlc_mbo_oce_attr_parse_data_t *data)
{
	wlc_bss_info_t *target_bss = NULL;
	wlc_bsscfg_t *bsscfg = NULL;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wifi_oce_rssi_assoc_rej_attr_t *assoc_rej = NULL;
	int ret = BCME_OK;

	bsscfg = data->cfg;
	target_bss = bsscfg->target_bss;

	/* OCE Association Rejection attribute */
	assoc_rej = (wifi_oce_rssi_assoc_rej_attr_t *)
		bcm_parse_tlvs(data->ie, data->ie_len,
		OCE_ATTR_RSSI_BASED_ASSOC_REJECTION);

	if ((ftpparm->assocresp.status == OCE_ASSOC_REJECT_RC_INSUFFICIENT_RSSI) &&
		assoc_rej) {
		ret = wlc_oce_add_to_delayed_list(oce, bsscfg, target_bss,
				assoc_rej->retry_delay,
				assoc_rej->delta_rssi);
		if (ret != BCME_OK) {
			WL_ERROR(("wl%d: %s: failed to add reject bssid to the list\n",
				oce->wlc->pub->unit, __FUNCTION__));
			return BCME_NOTFOUND;
		}

		target_bss->flags2 |= WLC_BSS_OCE_ASSOC_REJD;
	} else {
		wlc_oce_rm_delayed_list(oce, bsscfg);
	}

	return BCME_OK;
}

static int
wlc_oce_ie_parse_fn(void *ctx, wlc_mbo_oce_attr_parse_data_t *data)
{
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;
	wifi_oce_cap_ind_attr_t *ap_cap_attr = NULL;
	int ret = BCME_OK;

	if (data->ie) {
		/* OCE Capability Indication attribute */
		ap_cap_attr = (wifi_oce_cap_ind_attr_t *)
			bcm_parse_tlvs(data->ie, data->ie_len,
			OCE_ATTR_OCE_CAPABILITY_INDICATION);
	}

	if (ap_cap_attr == NULL) {
		return BCME_OK;
	}

	switch (data->ft) {
		case WLC_IEM_FC_SCAN_BCN:
		case WLC_IEM_FC_SCAN_PRBRSP:
			wlc_oce_parse_bcnprb_at_scan(oce, data);
			break;
#ifdef WL_OCE_AP
		case FC_ASSOC_REQ:
		case FC_REASSOC_REQ:
			break;
#endif
		case FC_ASSOC_RESP:
		case FC_REASSOC_RESP:
			ret = wlc_oce_parse_assoc(oce, data);
			break;
		case FC_BEACON:
		case FC_PROBE_RESP:
			wlc_oce_parse_bcnprb(oce, data);
			break;
		default:
		{
			ASSERT(0);
	}
	}

	return ret;
}

/* Write FILS Request Parameters element for probe req,
 * OCE Tech Spec v5 section  3.7,
 * 802.11ai draft 6.2 section 8.4.2.173
 */
static int
wlc_oce_fils_element_build_fn(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;
	uint8 *cp = NULL;

	if (!OCE_ENAB(oce->wlc->pub)) {
		return 0;
	}

	cp = data->buf;

	switch (data->ft) {
		case FC_PROBE_REQ:
			(void)wlc_oce_write_fils_req_param_element(oce, cp, data->buf_len);
			break;
		default:
			break;
	}

	return BCME_OK;
}

/* Calculate bytes needed to write FILS Request Parameters element.
 * OCE Tech Spec v5 section 3.7,
 * 802.11ai section 9.4.2.178
 */
static uint
wlc_oce_fils_element_calc_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;

	if (!OCE_ENAB(oce->wlc->pub)) {
		return 0;
	}

	return wlc_oce_get_fils_req_params_size(oce) + BCM_TLV_EXT_HDR_SIZE;
}

/* Write FILS Request Parameters Element,
 * OCE Tech Spec v5 section 3.7,
 * 802.11ai section 9.4.2.178
 */
static uint16
wlc_oce_write_fils_req_param_element(wlc_oce_info_t *oce, uint8 *cp, uint8 buf_len)
{
	uint16 size = wlc_oce_get_fils_req_params_size(oce);
	uint8* fils_params = MALLOCZ(oce->wlc->osh, size);

	if (!fils_params) {
		WL_ERROR(("wl%d: %s:out of mem. alloced %u bytes\n",
			oce->wlc->pub->unit, __FUNCTION__,  MALLOCED(oce->wlc->osh)));
		return 0;
	}

	ASSERT(buf_len >=
		(BCM_TLV_EXT_HDR_SIZE + size));

	fils_params[0] = oce->fils_req_params_bitmap;
	fils_params[1] = oce->oce_cmn_info->max_channel_time;

	bcm_write_tlv_ext(DOT11_MNG_ID_EXT_ID, FILS_EXTID_MNG_REQ_PARAMS, fils_params, size, cp);

	MFREE(oce->wlc->osh, fils_params, size);

	return size + BCM_TLV_EXT_HDR_SIZE;
}

static uint16 wlc_oce_get_fils_req_params_size(wlc_oce_info_t *oce)
{
	uint16 size = 2;

	/* additional parameters size added here... */

	return size;
}

uint8
wlc_oce_get_pref_channels(chanspec_t *chanspec_list)
{
	uint i;

	for (i = 0; i < sizeof(oce_pref_channels); i++) {
		chanspec_list[i] = CH20MHZ_CHSPEC(oce_pref_channels[i]);
	}

	return sizeof(oce_pref_channels);
}

bool wlc_oce_is_pref_channel(chanspec_t chanspec)
{
	uint i;

	for (i = 0; i < sizeof(oce_pref_channels); i++) {
		if (CHSPEC_CHANNEL(chanspec) == oce_pref_channels[i]) {
			return TRUE;
	}
	}

	return FALSE;
}

/* Set Max Channel Time Indication in units of 200us
 * OCE Tech Spec v5 section 3.7,
 * 802.11ai draft 6.2 section 8.4.2.173
 */
void
wlc_oce_set_max_channel_time(wlc_oce_info_t *oce, uint16 time)
{
	uint8 channel_time;

	ASSERT(oce);

	channel_time = MAX_CHANNEL_TIME;

	if (time < MAX_CHANNEL_TIME) {
		channel_time = time;
	}

	oce->oce_cmn_info->max_channel_time = channel_time;
}

static int
wlc_oce_parse_fd_action_frame(wlc_oce_info_t *oce, wlc_d11rxhdr_t *wrxh,
	struct dot11_management_header *hdr, uint8 *body, uint body_len)
{
	wlc_info_t *wlc;
	wlc_iem_upp_t upp;
	wlc_iem_ft_pparm_t ftpparm;
	wlc_iem_pparm_t pparm;
	wlc_bss_info_t bi;

	ASSERT(oce && body);

	if (body_len < (DOT11_ACTION_HDR_LEN + FD_INFO_FIELD_HDR_LEN)) {
		return BCME_BADLEN;
	}

	(void)wlc_oce_parse_fils_discovery(oce, wrxh, &hdr->bssid,
			body, body_len, &bi);

	wlc = oce->wlc;

	body += DOT11_ACTION_HDR_LEN;
	body_len -= DOT11_ACTION_HDR_LEN;

	/* prepare IE mgmt calls */
	wlc_iem_parse_upp_init(wlc->iemi, &upp);
	bzero(&ftpparm, sizeof(ftpparm));
	bzero(&pparm, sizeof(pparm));
	pparm.ft = &ftpparm;

	/* parse IEs */
	return wlc_ier_parse_frame(oce->oce_cmn_info->ier_fd_act_frame,
		NULL, WLC_IEM_FC_IER, &upp, &pparm, body, body_len);
}

int
wlc_oce_recv_fils(wlc_oce_info_t *oce, wlc_bsscfg_t *bsscfg,
	uint action_id, wlc_d11rxhdr_t *wrxh, uint8 *plcp,
	struct dot11_management_header *hdr, uint8 *body, int body_len)
{
	switch (action_id) {
	case DOT11_FILS_ACTION_DISCOVERY: {
		(void)wlc_oce_parse_fd_action_frame(oce, wrxh,
			hdr, body, body_len);
		break;
	}
	default:
		return BCME_UNSUPPORTED;
	};

	return BCME_OK;

}

static int wlc_oce_parse_fd_capability(wlc_oce_info_t *oce,
	fils_discovery_info_field_t	*fd_info, wlc_bss_info_t *bi)
{
	uint16 fc = ltoh16_ua(&fd_info->framecontrol);
	uint16 fd_cap;
	uint8 fd_cap_offset = 0;

	if (!FD_INFO_IS_CAP_PRESENT(fc)) {
		return BCME_NOTFOUND;
	}

	fd_cap_offset += FD_INFO_SSID_LENGTH(fc) + 1;

	if (FD_INFO_IS_LENGTH_PRESENT(fc)) {
		fd_cap_offset += FD_INFO_LENGTH_FIELD_SIZE;
	}

	fd_cap = ltoh16_ua(fd_info->disc_info + fd_cap_offset);

	if (FD_CAP_ESS(fd_cap)) {
		bi->capability |= DOT11_CAP_ESS;
	}

	/* farther FD capability fields parsed here */

	return BCME_OK;
}

static int wlc_oce_parse_ssid(wlc_oce_info_t *oce,
	fils_discovery_info_field_t	*fd_info, wlc_bss_info_t *bi, uint32 *short_ssid)
{
	uint16 fc = ltoh16_ua(&fd_info->framecontrol);
	uint8 ssid_len = 0;

	ssid_len = FD_INFO_SSID_LENGTH(fc) + 1;

	if (!ssid_len) {
		return BCME_NOTFOUND;
	}
	else if (ssid_len > DOT11_MAX_SSID_LEN) {
		return BCME_BADSSIDLEN;
	}

	if (FD_INFO_IS_SHORT_SSID_PRESENT(fc)) {
		if (ssid_len != SHORT_SSID_LEN) {
			WL_ERROR(("%s: Wrong short ssid length %d\n", __FUNCTION__, ssid_len));
			return BCME_BADSSIDLEN;
		}

		memcpy(short_ssid, fd_info->disc_info, ssid_len);
		bi->SSID_len = 0;

		return BCME_OK;
	}


	memcpy(bi->SSID, fd_info->disc_info, ssid_len);
	bi->SSID_len = ssid_len;

	return BCME_OK;
}

/*
 * Parse FILS Discovery Frame.
 * OCE Tech Spec v.0.0.7, 3.3. 802.11ai, 8.6.8.36.
 */
int
wlc_oce_parse_fils_discovery(wlc_oce_info_t *oce, wlc_d11rxhdr_t *wrxh,
	struct ether_addr *bssid, uint8 *body, uint body_len, wlc_bss_info_t *bi)
{
	wlc_info_t *wlc = oce->wlc;
	fils_discovery_info_field_t	*fd_info =
		(fils_discovery_info_field_t*) (body + DOT11_ACTION_HDR_LEN);
	uint32 short_ssid = 0;
	uint32 tsf_l, tsf_h, next_tbtt, ts;
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif
	memset(bi, 0, sizeof(wlc_bss_info_t));
	/* get (short)ssid */
	(void)wlc_oce_parse_ssid(oce, fd_info, bi, &short_ssid);

	bi->beacon_period = ltoh16_ua(&fd_info->bcninterval);

	/* calc next tbtt info */
	ts = ltoh32_ua(&fd_info->timestamp);

	next_tbtt = CEIL(ts, bi->beacon_period * 1024) * (bi->beacon_period * 1024);

	wlc_read_tsf(wlc, &tsf_l, &tsf_h);

#if defined(BCMDBG) || defined(WLMSG_INFORM)
	WL_OCE_INFO(("fd frame: add bssid %s short ssid %x %x to the list\n",
		bcm_ether_ntoa(bssid, eabuf), oce->short_ssid, short_ssid));
#endif

	wlc_oce_add_to_prb_suppress_bss_list(oce, bssid, short_ssid, tsf_l, next_tbtt - ts, FALSE);

	/* get BSS capabilities */
	(void)wlc_oce_parse_fd_capability(oce, fd_info, bi);

	memcpy((char *)&bi->BSSID, (char *)bssid, ETHER_ADDR_LEN);

	bi->RSSI = wlc_lq_recv_rssi_compute(wlc, wrxh);

	bi->flags2 |= (bi->RSSI == WLC_RSSI_INVALID) ? WLC_BSS_RSSI_INVALID : 0;

	bi->phy_noise = phy_noise_avg(WLC_PI(wlc));

	bi->SNR = (int16)wlc_lq_recv_snr_compute(wlc, wrxh, (int8)bi->RSSI, bi->phy_noise);

	bi->chanspec = CH20MHZ_CHSPEC(wlc_recv_mgmt_rx_channel_get(wlc, wrxh));

	bi->bss_type = DOT11_BSSTYPE_INFRASTRUCTURE;

	bi->flags = 0;
	bi->flags2 = (bi->RSSI == WLC_RSSI_INVALID) ? WLC_BSS_RSSI_INVALID : 0;

	wlc_default_rateset(wlc, &bi->rateset);

	return BCME_OK;
}

void
wlc_oce_process_assoc_reject(wlc_bsscfg_t *cfg, struct scb *scb, uint16 fk,
	uint8 *body, uint body_len)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_iem_upp_t upp;
	wlc_iem_ft_pparm_t ftpparm;
	wlc_iem_pparm_t pparm;

	body += sizeof(struct dot11_assoc_resp);
	body_len -= sizeof(struct dot11_assoc_resp);

	/* prepare IE mgmt calls */
	wlc_iem_parse_upp_init(wlc->iemi, &upp);
	bzero(&ftpparm, sizeof(ftpparm));
	ftpparm.assocresp.scb = scb;
	ftpparm.assocresp.status = OCE_ASSOC_REJECT_RC_INSUFFICIENT_RSSI;
	bzero(&pparm, sizeof(pparm));
	pparm.ft = &ftpparm;

	/* parse IEs */
	wlc_iem_parse_frame(wlc->iemi, cfg, fk, &upp, &pparm,
		body, body_len);
}

/* Parse RNR element in beacon/probe resp/action frame,
 * WFA OCE Tech Spec v5 3.4, 802.11ai D11 9.4.2.171
 */
static int
wlc_oce_rnr_element_parse_fn(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;

	ASSERT(ctx && data);

	switch (data->ft) {
		case FC_ACTION:
		case FC_BEACON:
		case FC_PROBE_RESP:
		case WLC_IEM_FC_SCAN_PRBRSP:
		case WLC_IEM_FC_SCAN_BCN:
			if (data->ie) {
				WL_OCE_DBG(("Received RNR ie\n"));
				(void)wlc_oce_parse_rnr_element(oce, data->ie, data->ie_len);
			}
			break;
		default:
			break;
	}

	return BCME_OK;
}

static int
wlc_oce_parse_rnr_element(wlc_oce_info_t *oce, uint8* buf, uint16 buflen)
{
	neighbor_ap_info_field_t ap_info;
	uint16 tbtt_count;
	uint8 tbtt_info_length;
	uint8* cp;
	int16 len;
	uint16 i;

	ASSERT(oce && buf);

	len = buflen - FILS_RNR_ELEM_HDR_LEN;
	cp = buf + FILS_RNR_ELEM_HDR_LEN;

	while (len >= (uint16)NEIGHBOR_AP_INFO_FIELD_HDR_LEN) {
		memcpy((uint8*)&ap_info, cp, NEIGHBOR_AP_INFO_FIELD_HDR_LEN);

		ap_info.tbtt_info_header = ltoh16_ua(&ap_info.tbtt_info_header);
		tbtt_info_length = TBTT_INFO_HDR_LENGTH(ap_info.tbtt_info_header);

		tbtt_count = TBTT_INFO_HDR_COUNT(ap_info.tbtt_info_header);

		cp += NEIGHBOR_AP_INFO_FIELD_HDR_LEN;
		len -= NEIGHBOR_AP_INFO_FIELD_HDR_LEN;

		WL_OCE_DBG(("Neighbor AP Info Field: channel %d tbtt info count %d "
			"tbtt info len %d\n",
			ap_info.channel, tbtt_count, tbtt_info_length));

		for (i = 0; i <= tbtt_count; i++) {
			tbtt_info_field_t tbtt_info;
#if defined(BCMDBG) || defined(WLMSG_OCE)
			char eabuf[ETHER_ADDR_STR_LEN];
#endif
			if (len < tbtt_info_length) {
				WL_ERROR(("%s: Wrong RNR ie length!\n", __FUNCTION__));
				return BCME_ERROR;
			}

			memcpy((uint8*)&tbtt_info, cp, tbtt_info_length);

			if (tbtt_info_length > 1) {
				tbtt_info.short_ssid = ltoh32(tbtt_info.short_ssid);
			}

			WL_OCE_DBG(("TBTT Info Field: short ssid %x tbtt offset %d BSSID %s\n",
				tbtt_info.short_ssid, tbtt_info.tbtt_offset,
				bcm_ether_ntoa((struct ether_addr*)tbtt_info.bssid, eabuf)));

			cp += tbtt_info_length;
			len -= tbtt_info_length;
		}
	}

	return BCME_OK;
}

#ifdef WL_OCE_AP

/* Add RNR element in beacon/probe resp per 802.11ai-2016 amendment-1,
 * 9.4.2.171.1 and WFA OCE Spec v18, 3.4.
 */
static uint
wlc_oce_rnr_element_calc_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_oce_bsscfg_cubby_t *obc;

	ASSERT(oce);
	ASSERT(cfg);

	obc = OCE_BSSCFG_CUBBY(oce, cfg);
	ASSERT(obc);

	if (!obc->rnr_nbr_ap_info_size || !OCE_ENAB(oce->wlc->pub)) {
		return 0;
	}

	return obc->rnr_nbr_ap_info_size + BCM_TLV_HDR_SIZE;
}

/* Add RNR element in beacon/probe resp per 802.11ai-2016 amendment-1,
 * 9.4.2.171.1 and WFA OCE Spec v18, 3.4.
 */
static int
wlc_oce_rnr_element_build_fn(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_oce_bsscfg_cubby_t *obc;
	uint8 *cp = NULL;

	ASSERT(oce);
	ASSERT(cfg);
	obc = OCE_BSSCFG_CUBBY(oce, cfg);
	ASSERT(obc);

	if (!obc->rnr_nbr_ap_info_size || !OCE_ENAB(oce->wlc->pub)) {
		return BCME_OK;
	}

	cp = data->buf;

	switch (data->ft) {
		case FC_BEACON:
		case FC_PROBE_RESP:
			(void)wlc_oce_write_rnr_element(obc, cp, data->buf_len);
			break;
		default:
			break;
	}

	return BCME_OK;
}

static uint16
wlc_oce_write_rnr_element(wlc_oce_bsscfg_cubby_t *obc, uint8 *cp, uint buf_len)
{
	ASSERT(buf_len >=
		(obc->rnr_nbr_ap_info_size + BCM_TLV_HDR_SIZE));

	bcm_write_tlv(DOT11_MNG_RNR_ID, obc->rnr_nbr_ap_info, obc->rnr_nbr_ap_info_size, cp);

	return obc->rnr_nbr_ap_info_size + BCM_TLV_HDR_SIZE;
}

static int
wlc_oce_write_tbtt_info_field(wlc_oce_info_t *oce, wlc_bss_info_t *bi, uint8* buf, uint16 buflen)
{
	tbtt_info_field_t info;

	ASSERT(buflen >= TBTT_INFO_FIELD_HDR_LEN);

	info.tbtt_offset = 0xff;

	memcpy(info.bssid, &bi->BSSID, ETHER_ADDR_LEN);

	info.short_ssid = ~hndcrc32(bi->SSID, bi->SSID_len, CRC32_INIT_VALUE);
	htol32_ua_store(info.short_ssid, &info.short_ssid);

	memcpy(buf, (uint8*)&info, TBTT_INFO_FIELD_HDR_LEN);

	return TBTT_INFO_FIELD_HDR_LEN;
}

/*
 * Channel spec to Global op class conversion routine per 802.11-2016 Annex-E
 * table E-4. RNR IE operating class field specifically mentiones E-4. There
 * is per region op class implemention in wlc_channel.c but that cannot be
 * used RNR IE.
 */
static struct _global_opclass {
	uint8 global_opclass_20m[6];
	uint8 global_opclass_40m[10];
	uint8 global_opclass_80m[2];
	uint8 global_opclass_160m[1];
} global_opclass = {
	{81, 115, 118, 121, 124, 125},
	{83, 84, 116, 117, 119, 120, 122, 123, 126, 127},
	{128, 130},
	{129}};

static uint8 country_info_global_opclass_chan[] = {13, 48, 64, 144, 161, 169};

uint8
wlc_chanspec_ac2opclass(chanspec_t chanspec)
{
	uint16 primary_chan, bw, sb;
	uint i;

	primary_chan = wf_chspec_ctlchan(chanspec);
	bw = chanspec & WL_CHANSPEC_BW_MASK;
	sb = chanspec & WL_CHANSPEC_CTL_SB_MASK;

	for (i = 0; i < sizeof(country_info_global_opclass_chan); i++)
		if (primary_chan <= country_info_global_opclass_chan[i]) {
			if (bw == WL_CHANSPEC_BW_20)
				return global_opclass.global_opclass_20m[i];
			if (bw == WL_CHANSPEC_BW_40) {
				if (sb == WL_CHANSPEC_CTL_SB_L)
					return global_opclass.global_opclass_40m[i * 2];
				return global_opclass.global_opclass_40m[i * 2 + 1];
			}
		}

	if (bw == WL_CHANSPEC_BW_80)
		return 128;
	if (bw == WL_CHANSPEC_BW_160)
		return 129;
	return 0;
}

static int
wlc_oce_opclass2index(uint8 opclass)
{
	int i = 0;
	uint8 *ptr = (uint8 *)&global_opclass;

	for (i = 0; i < sizeof(global_opclass); i++) {
		if (opclass == *ptr++) {
			break;
		}
	}

	return i;
}

static int
wlc_oce_write_neighbor_ap_info_field(wlc_oce_info_t *oce, uint8 tbtt_count,
	bool filter_neighbor_ap, chanspec_t chspec, uint8* buf, uint16 buflen)
{
	neighbor_ap_info_field_t info;
	uint16 tbtt_info_hdr = 0;

	ASSERT(oce);
	info.channel = wf_chspec_ctlchan(chspec);
	info.op_class = wlc_chanspec_ac2opclass(chspec);

	TBTT_INFO_HDR_SET_LENGTH(tbtt_info_hdr, TBTT_INFO_FIELD_HDR_LEN);

	if (filter_neighbor_ap) {
		TBTT_INFO_HDR_SET_FN_AP(tbtt_info_hdr, 1);
	}

	ASSERT(tbtt_count >= 1);
	TBTT_INFO_HDR_SET_COUNT(tbtt_info_hdr, (tbtt_count - 1));

	htol16_ua_store(tbtt_info_hdr, &info.tbtt_info_header);
	ASSERT(buflen >= NEIGHBOR_AP_INFO_FIELD_HDR_LEN);
	memcpy(buf, (uint8*)&info, NEIGHBOR_AP_INFO_FIELD_HDR_LEN);

	return NEIGHBOR_AP_INFO_FIELD_HDR_LEN;
}

static void
wlc_oce_update_rnr_nbr_ap_info(wlc_oce_info_t* oce, wlc_bsscfg_t *bsscfg)
{
	wlc_oce_bsscfg_cubby_t *obc;
	wlc_bsscfg_t *cfg;
	uint8 *cp = NULL, *cp_ap = NULL, *sscp;
	uint8 len, i, j;
	uint8 channel = 0, upd_sssid = 0;
	uint16 bss_count;
	chanvec_t channels;
	bool filter_neighbor_ap = FALSE, home_channel;

	ASSERT(oce);
	ASSERT(bsscfg);

	bss_count = oce->wlc->scan_results->count;
	obc = OCE_BSSCFG_CUBBY(oce, bsscfg);
	ASSERT(obc);

	obc->rnr_nbr_ap_info_size = 0;
	obc->rnrc_sssid_list_size = 0;

	if (!oce->oce_cmn_info->rnr_scan_period || !OCE_ENAB(oce->wlc->pub)) {
		RNR_SCAN_DEL_TIMER(oce);
		return;
	}

	/* count channels with BSS presence */
	memset(&channels, 0, sizeof(chanvec_t));
	for (i = 0; i < bss_count; i++) {
		wlc_bss_info_t *bi = oce->wlc->scan_results->ptrs[i];

		if (isclr(channels.vec, CHSPEC_CHANNEL(bi->chanspec))) {
			setbit(channels.vec, CHSPEC_CHANNEL(bi->chanspec));
		}
	}
	/* make sure home channel is set */
	setbit(channels.vec, CHSPEC_CHANNEL(bsscfg->current_bss->chanspec));
	home_channel = TRUE;

	/* clear the old one */
	bzero(obc->rnr_nbr_ap_info, BCM_TLV_MAX_DATA_SIZE);
	obc->rnr_nbr_ap_info_size = BCM_TLV_MAX_DATA_SIZE;

	bzero(obc->rnrc_sssid_list, BCM_TLV_MAX_DATA_SIZE);

	cp = obc->rnr_nbr_ap_info;
	len = obc->rnr_nbr_ap_info_size;
	sscp = obc->rnrc_sssid_list;

	for (j = 0; (j < MAXCHANNEL) &&
		(len >= (NEIGHBOR_AP_INFO_FIELD_HDR_LEN + TBTT_INFO_FIELD_HDR_LEN)); j++) {
		tbtt_info_field_t *tbtt_info;
		uint8 tbtt_info_count;
		chanspec_t chspec = bsscfg->current_bss->chanspec;
		uint8 bi_bmp[bss_count];

		if (home_channel) {
			channel = CHSPEC_CHANNEL(chspec);
		} else if (isset(channels.vec, j)) {
			channel = j;
		} else {
			continue;
		}

		clrbit(channels.vec, channel);
		home_channel = FALSE;

		/* adjust for info header field */
		cp_ap = cp;
		cp += NEIGHBOR_AP_INFO_FIELD_HDR_LEN;
		len -= NEIGHBOR_AP_INFO_FIELD_HDR_LEN;

		tbtt_info_count = 0;

		/* Write any My-BSSes */
		FOREACH_UP_AP(oce->wlc, i, cfg) {
			if (cfg == bsscfg)
				continue;

			if (cfg->current_bss->SSID_len == 0) {
				/* update current_bss SSID */
				cfg->current_bss->SSID_len = cfg->SSID_len;
				bcopy(cfg->SSID, cfg->current_bss->SSID, cfg->SSID_len);
			}

			if ((CHSPEC_CHANNEL(cfg->current_bss->chanspec) == channel) &&
				(len >= TBTT_INFO_FIELD_HDR_LEN) &&
				(tbtt_info_count < MAX_TBTT_INFO_FIELDS)) {

				/* add tbtt info field */
				wlc_oce_write_tbtt_info_field(oce, cfg->current_bss, cp, len);

				/* copy short ssid  for rnr completeness
				 * attribute.
				 */
				tbtt_info = (tbtt_info_field_t *)cp;
				memcpy(sscp, (uint8 *)&tbtt_info->short_ssid, SHORT_SSID_LEN);
				sscp += SHORT_SSID_LEN;

				cp += TBTT_INFO_FIELD_HDR_LEN;
				len -= TBTT_INFO_FIELD_HDR_LEN;
				tbtt_info_count++;
			}
		}

		memset(&bi_bmp, 0, sizeof(bi_bmp));

		/* write BSSes with same as my SSID */
		for (i = 0; (i < bss_count) && (len >= TBTT_INFO_FIELD_HDR_LEN) &&
			(tbtt_info_count < MAX_TBTT_INFO_FIELDS); i++) {
			wlc_bss_info_t *obi = oce->wlc->scan_results->ptrs[i];
			uint8 obi_channel = CHSPEC_CHANNEL(obi->chanspec);

			if ((obi_channel == channel) && ((obi->SSID_len == bsscfg->SSID_len) &&
				!memcmp(obi->SSID, bsscfg->SSID, bsscfg->SSID_len))) {
				wlc_oce_write_tbtt_info_field(oce, obi, cp, len);

				/* Just one of the S-SSID is needed in RNR
				 * Completeness attribute.
				 */
				if (!upd_sssid) {
					tbtt_info = (tbtt_info_field_t *)cp;
					memcpy(sscp, (uint8 *)&tbtt_info->short_ssid,
						SHORT_SSID_LEN);
					sscp += SHORT_SSID_LEN;
					upd_sssid = 1;
				}


				cp += TBTT_INFO_FIELD_HDR_LEN;
				len -= TBTT_INFO_FIELD_HDR_LEN;
				tbtt_info_count++;

				setbit(bi_bmp, i);
				chspec = obi->chanspec;
			}
		}

#if 0
		/* XXX: Adding other APs info increases beacon/prs size since
		 * RNR IE and RNR completeness attribute both get bigger.
		 */
		for (i = 0; (i < bss_count) && (len >= TBTT_INFO_FIELD_HDR_LEN) &&
			(tbtt_info_count < MAX_TBTT_INFO_FIELDS); i++) {
			wlc_bss_info_t *obi = oce->wlc->scan_results->ptrs[i];
			uint8 obi_channel = CHSPEC_CHANNEL(obi->chanspec);

			if ((obi_channel == channel) && isclr(bi_bmp, i)) {
				wlc_oce_write_tbtt_info_field(oce, obi, cp, len);

				/* copy short ssid  for rnr completeness
				 * attribute.
				 */
				tbtt_info = (tbtt_info_field_t *)cp;
				memcpy(sscp, (uint8 *)&tbtt_info->short_ssid, SHORT_SSID_LEN);
				sscp += SHORT_SSID_LEN;

				cp += TBTT_INFO_FIELD_HDR_LEN;
				len -= TBTT_INFO_FIELD_HDR_LEN;
				tbtt_info_count++;

				setbit(bi_bmp, i);
				chspec = obi->chanspec;
			}
		}
#endif

		if (tbtt_info_count) {
			wlc_oce_write_neighbor_ap_info_field(oce,
				tbtt_info_count, filter_neighbor_ap, chspec, cp_ap, len);
		} else {
			/* No need to update Neighbor info field, adjust for info header field */
			cp = cp_ap;
			len += NEIGHBOR_AP_INFO_FIELD_HDR_LEN;
		}

	}

	obc->rnrc_sssid_list_size = sscp - obc->rnrc_sssid_list;
	obc->rnr_nbr_ap_info_size = cp - obc->rnr_nbr_ap_info;

	return;
}

static void
wlc_oce_scan_complete_cb(void *ctx, int status, wlc_bsscfg_t *bsscfg)
{
	wlc_oce_info_t *oce = (wlc_oce_info_t*) ctx;
	wlc_bsscfg_t *cfg;
	uint8 idx;

	ASSERT(oce);
	ASSERT(bsscfg);

	/* XXX: just one scanresults are sufficient to prepare neighbor info
	 * for all other BSSes.
	 */
	FOREACH_UP_AP(oce->wlc, idx, cfg) {
		wlc_oce_update_rnr_nbr_ap_info(oce, cfg);
		wlc_oce_update_ap_chrep(oce, cfg);
	}

	wlc_suspend_mac_and_wait(oce->wlc);
	wlc_bss_update_beacon(oce->wlc, bsscfg);
	wlc_bss_update_probe_resp(oce->wlc, bsscfg, FALSE);
	wlc_enable_mac(oce->wlc);
}


static void
wlc_oce_rnr_scan_timer(void *ctx)
{
	wlc_oce_info_t *oce = (wlc_oce_info_t*) ctx;
	wlc_bsscfg_t *bsscfg;
	wlc_ssid_t ssid = {0};
	int err;
	int i;

	ASSERT(oce);
	ASSERT(oce->oce_cmn_info);
	if (!oce->oce_cmn_info->rnr_scan_period || !OCE_ENAB(oce->wlc->pub)) {
		RNR_SCAN_DEL_TIMER(oce);
		return;
	}

	FOREACH_UP_AP(oce->wlc, i, bsscfg) {
		bzero(&ssid, sizeof(ssid));
		err = wlc_scan_request_ex(oce->wlc, DOT11_BSSTYPE_ANY, &ether_bcast, 1, &ssid,
				DOT11_SCANTYPE_PASSIVE, -1,
				-1, -1, -1,
				NULL, 0, 0,
				FALSE,
				wlc_oce_scan_complete_cb, oce,
				WLC_ACTION_SCAN, FALSE, bsscfg, NULL, NULL);

		if (err) {
			WL_ERROR(("wl%d: %s: RNR scan failed [%d]\n",
				oce->wlc->pub->unit, __FUNCTION__, err));
		}

		/* just one bss scan is ok */
		break;
	}

	RNR_SCAN_ADD_TIMER(oce, oce->oce_cmn_info->rnr_scan_period);
}


static int
wlc_oce_get_fd_frame_len(wlc_oce_info_t *oce, wlc_bsscfg_t *bsscfg)
{
	uint16 frame_len;

	frame_len = DOT11_ACTION_HDR_LEN +
			FD_INFO_FIELD_HDR_LEN +
			SHORT_SSID_LEN +
			FD_INFO_CAP_SUBFIELD_SIZE;

	return frame_len;
}

static int wlc_oce_write_fd_capability(wlc_oce_info_t *oce,
	fils_discovery_info_field_t	*fd_info, uint8* fd_cap)
{
	uint16 cap = 0;

	FD_CAP_SET_ESS(cap);
	/* farther FD capability bits written below this line */

	htol16_ua_store(cap, fd_cap);
	FD_INFO_SET_CAP_PRESENT(fd_info->framecontrol);

	return BCME_OK;
}

/* Short-SSID calculation:
 * 802.11ai 9.4.2.171.2 Calculating the Short-SSID
 */
static int wlc_oce_write_fd_ssid(wlc_oce_info_t *oce,
	fils_discovery_info_field_t	*fd_info, uint8* ssid, uint8 ssid_len)
{
	uint32 short_ssid;

	short_ssid = ~hndcrc32(ssid, ssid_len, CRC32_INIT_VALUE);
	htol32_ua_store(short_ssid, fd_info->disc_info);

	FD_INFO_SET_SHORT_SSID_PRESENT(fd_info->framecontrol);
	FD_INFO_SET_SSID_LENGTH(fd_info->framecontrol, SHORT_SSID_LEN - 1);

	return BCME_OK;
}

static int
wlc_oce_write_fd_info_field(wlc_oce_info_t *oce, wlc_bsscfg_t *bsscfg,
	uint8 *buf, uint16 buf_len)
{
	fils_discovery_info_field_t *fd_info =
		(fils_discovery_info_field_t*) buf;

	uint32 tsf_l, tsf_h;

	memset(buf, 0, buf_len);

	/* beacon interval */
	fd_info->bcninterval = htol16(bsscfg->current_bss->beacon_period);

	/* TSF */
	wlc_read_tsf(oce->wlc, &tsf_l, &tsf_h);

	fd_info->timestamp[0] = htol32(tsf_l);
	fd_info->timestamp[1] = htol32(tsf_h);

	/* FILS Discovery info */
	wlc_oce_write_fd_ssid(oce, fd_info, bsscfg->SSID, bsscfg->SSID_len);
	wlc_oce_write_fd_capability(oce, fd_info, fd_info->disc_info + SHORT_SSID_LEN);

	fd_info->framecontrol = htol16(fd_info->framecontrol);

	return BCME_OK;
}

/* FILS Discovery Frame: 802.11ai D11, 9.6.8.36 */
static wlc_pkt_t
wlc_oce_init_fd_frame(wlc_oce_info_t *oce, wlc_bsscfg_t *bsscfg)
{
	dot11_action_frmhdr_t* fd_action_frame;
	uint8 *pbody;
	wlc_pkt_t pkt;

	uint16 frame_len = wlc_oce_get_fd_frame_len(oce, bsscfg);

	pkt = wlc_frame_get_action(oce->wlc, &ether_bcast, &bsscfg->cur_etheraddr,
		&bsscfg->BSSID, frame_len, &pbody, DOT11_ACTION_CAT_PUBLIC);

	if (!pkt) {
		WL_ERROR(("%s, Could not allocate fils dicovery frame\n", __FUNCTION__));
		return NULL;
	}

	fd_action_frame = (dot11_action_frmhdr_t*) pbody;

	fd_action_frame->category = DOT11_ACTION_CAT_PUBLIC;
	fd_action_frame->action = DOT11_FILS_ACTION_DISCOVERY;

	wlc_oce_write_fd_info_field(oce, bsscfg,
		pbody + DOT11_ACTION_HDR_LEN, frame_len - DOT11_ACTION_HDR_LEN);

	return pkt;
}

/* FILS Discovery Frame transmission,
 * WFA-OCE spec v0.0.7, 3.3
 */
static int
wlc_oce_send_fd_frame(wlc_oce_info_t *oce, wlc_bsscfg_t *bsscfg)
{
	wlc_pkt_t pkt;
	ratespec_t rate_override = 0;

	if (!(pkt = wlc_oce_init_fd_frame(oce, bsscfg))) {
		return BCME_NOMEM;
	}

	rate_override = WLC_RATE_6M;

	if (!wlc_queue_80211_frag(oce->wlc, pkt,
		oce->wlc->active_queue, NULL, bsscfg,
		FALSE, NULL, rate_override)) {
		WL_ERROR(("%s, wlc_queue_80211_frag failed\n", __FUNCTION__));
		return BCME_ERROR;
	}

	return BCME_OK;
}

static void
wlc_oce_fd_tx_timer(void *ctx)
{
	wlc_bsscfg_t *bsscfg;
	wlc_oce_info_t *oce = (wlc_oce_info_t*) ctx;
	wlc_info_t *wlc = oce->wlc;
	int i;

	ASSERT(ctx);
	wlc_gptimer_wake_upd(wlc, WLC_GPTIMER_AWAKE_OCE, FALSE);

	if (!oce->oce_cmn_info->fd_tx_period) {
		/* if fd disabled */
		FD_TX_DEL_TIMER(oce);
		return;
	}

	FOREACH_UP_AP(wlc, i, bsscfg) {
		wlc_oce_send_fd_frame(oce, bsscfg);
		ASSERT(oce->oce_cmn_info->fd_frame_count > 0);
		oce->oce_cmn_info->fd_frame_count--;
		/* just one bss is ok since rnr ie is sent */
		if (oce->oce_cmn_info->rnr_scan_period)
			break;
	}

	if (oce->oce_cmn_info->fd_frame_count) {
		FD_TX_ADD_TIMER(oce, wlc_oce_fd_tx_timer);
		wlc_gptimer_wake_upd(wlc, WLC_GPTIMER_AWAKE_OCE, TRUE);
	}
}

#ifdef WLMCNX
static void
wlc_oce_pretbtt_callback(void *ctx, wlc_mcnx_intr_data_t *notif_data)
{
	wlc_oce_info_t *oce = (wlc_oce_info_t*) ctx;
	wlc_info_t *wlc = oce->wlc;
	wlc_bsscfg_t *bsscfg;
	int i;

	ASSERT(ctx);

	if (!oce->oce_cmn_info->fd_tx_period) {
		/* if fd disabled */
		return;
	}

	if (notif_data->intr == M_P2P_I_PRE_TBTT) {
		/* cancel pending FILS Discovery frame */
		/* to stay aligned with tbtt */
		wlc_gptimer_wake_upd(wlc, WLC_GPTIMER_AWAKE_OCE, FALSE);
		FD_TX_DEL_TIMER(oce);

		oce->oce_cmn_info->fd_frame_count = 0;
		/* 3min wait time is needed only for CFON. */
		if ((OSL_SYSUPTIME() - oce->up_tick) < oce->oce_cmn_info->fd_tx_duration) {
			/* calculate number of FD frames per beacon interval */
			FOREACH_UP_AP(wlc, i, bsscfg) {
				oce->oce_cmn_info->fd_frame_count +=
				(bsscfg->current_bss->beacon_period / oce->oce_cmn_info->fd_tx_period) - 1;
				/* just one bss is ok since rnr ie is sent */
				if (oce->oce_cmn_info->rnr_scan_period)
					break;
			}
			/* initiate FD frame transmission */
			FD_TX_ADD_TIMER(oce, wlc_oce_fd_tx_timer);
			wlc_gptimer_wake_upd(wlc, WLC_GPTIMER_AWAKE_OCE, TRUE);
		}
	}
}

#else
void
wlc_oce_pretbtt_fd_callback(wlc_oce_info_t *oce)
{
	wlc_info_t *wlc;
	wlc_bsscfg_t *bsscfg;
	int i;
	uint32 tbtt_h, tbtt_l;
	uint32 tsf_h, tsf_l;
	int32 tt_next_tbtt;
	int32 addtime = 0;

	ASSERT(oce);
	wlc = oce->wlc;

	if (!oce->oce_cmn_info->fd_tx_period) {
		/* if fd disabled */
		return;
	}

	/* cancel pending FILS Discovery frame */
	/* to stay aligned with tbtt */
	wlc_gptimer_wake_upd(wlc, WLC_GPTIMER_AWAKE_OCE, FALSE);
	FD_TX_DEL_TIMER(oce);
	oce->oce_cmn_info->fd_frame_count = 0;
	FOREACH_UP_AP(wlc, i, bsscfg) {
		/* BIs in AP (MBSS) are same; use default. */
		oce->oce_cmn_info->fd_frame_count +=
			(wlc->default_bss->beacon_period / oce->oce_cmn_info->fd_tx_period) - 1;

		/* just one bss is ok since rnr ie is sent */
		if (oce->oce_cmn_info->rnr_scan_period)
			break;
	}
	wlc_read_tsf(wlc, &tbtt_l, &tbtt_h);
	tsf_l = tbtt_l;
	tsf_h = tbtt_h;
	wlc_tsf64_to_next_tbtt64(wlc->default_bss->beacon_period, &tbtt_h, &tbtt_l);
	tt_next_tbtt = U32_DUR(tsf_l, tbtt_l);

	if (tt_next_tbtt <= MAX_BEFORE_TBTT) /*TBTT intr comes earlier than actual TBTT, mbss enabled case*/
		addtime += tt_next_tbtt;
	else /*TBTT intr comes later than actual TBTT*/
		addtime = tt_next_tbtt - (wlc->default_bss->beacon_period*DOT11_TU_TO_US);

	/* initiate FD frame transmission */
	FD_TX_ADD_TIMER_ADD(oce, addtime, wlc_oce_fd_tx_timer);
	wlc_gptimer_wake_upd(wlc, WLC_GPTIMER_AWAKE_OCE, TRUE);
}

#endif /* WLMCNX */

/* Add AP channel report element in beacon/probe resp per 802.11-2016,
 * 9.4.2.36 and WFA OCE spec v18, 3.4.
 */
static uint
wlc_oce_ap_chrep_element_calc_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_oce_bsscfg_cubby_t *obc;

	ASSERT(oce);
	ASSERT(cfg);

	obc = OCE_BSSCFG_CUBBY(oce, cfg);
	ASSERT(obc);

	if (!OCE_ENAB(oce->wlc->pub)) {
		return 0;
	}

	return wlc_oce_get_ap_chrep_size(oce);

}

static int
wlc_oce_ap_chrep_element_build_fn(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_oce_bsscfg_cubby_t *obc;
	uint8 *cp = NULL;

	ASSERT(oce);
	ASSERT(cfg);

	obc = OCE_BSSCFG_CUBBY(oce, cfg);
	ASSERT(obc);

	if (!OCE_ENAB(oce->wlc->pub)) {
		return BCME_OK;
	}

	cp = data->buf;

	switch (data->ft) {
		case FC_BEACON:
		case FC_PROBE_RESP:
			(void)wlc_oce_write_ap_chrep_element(oce, cp, data->buf_len);
			break;
		default:
			break;
	}

	return BCME_OK;
}

static uint16
wlc_oce_write_ap_chrep_element(wlc_oce_info_t *oce, uint8 *cp, uint buf_len)
{
	uint8 i, attr_size = 0;

	ASSERT(oce);

	attr_size = wlc_oce_get_ap_chrep_size(oce);

	if (!attr_size) {
		return 0;
	}

	if (buf_len < attr_size) {
		ASSERT(0);
		return 0;
	}

	for (i = 0; i < NO_OF_OPCLASS_CHLIST; i++) {
		if (oce->oce_cmn_info->opcls_chlist[i].cnt) {
			bcm_write_tlv(DOT11_MNG_AP_CHREP_ID,
				&oce->oce_cmn_info->opcls_chlist[i].opclass,
				oce->oce_cmn_info->opcls_chlist[i].cnt + 1, cp);
			cp += (oce->oce_cmn_info->opcls_chlist[i].cnt + 1 + BCM_TLV_HDR_SIZE);
		}
	}

	return attr_size;
}

static uint
wlc_oce_get_ap_chrep_size(wlc_oce_info_t *oce)
{
	uint8 i, size = 0;

	ASSERT(oce);

	if (!OCE_ENAB(oce->wlc->pub)) {
		return 0;
	}

	for (i = 0; i < NO_OF_OPCLASS_CHLIST; i++) {
		if (oce->oce_cmn_info->opcls_chlist[i].cnt) {
			/* no. of chans + opclass + tlv hdr size */
			size += (oce->oce_cmn_info->opcls_chlist[i].cnt + 1 + BCM_TLV_HDR_SIZE);
		}
	}

	return size;
}
static void
wlc_oce_update_ap_chrep(wlc_oce_info_t* oce, wlc_bsscfg_t *bsscfg)
{
	wlc_oce_bsscfg_cubby_t *obc;
	wlc_oce_cmn_info_t *oce_cmn;
	uint8 i;
	uint16 bss_count;
	chanvec_t channels;
	ASSERT(oce);
	ASSERT(bsscfg);

	bss_count = oce->wlc->scan_results->count;
	obc = OCE_BSSCFG_CUBBY(oce, bsscfg);
	ASSERT(obc);
	oce_cmn = oce->oce_cmn_info;
	ASSERT(oce_cmn);

	if (!oce_cmn->rnr_scan_period || !OCE_ENAB(oce->wlc->pub)) {
		RNR_SCAN_DEL_TIMER(oce);
		return;
	}

	/* reset */
	memset(&oce_cmn->opcls_chlist, 0, sizeof(oce_cmn->opcls_chlist));

	/* channels found in scan */
	memset(&channels, 0, sizeof(chanvec_t));
	for (i = 0; i < bss_count; i++) {
		wlc_bss_info_t *bi = oce->wlc->scan_results->ptrs[i];
		uint16 primary_chan = wf_chspec_ctlchan(bi->chanspec);
		if (isclr(channels.vec, primary_chan)) {
			uint8 opclass, opi;

			setbit(channels.vec, primary_chan);

			/* convert all the chanspecs to 20MHz chanspec and
			 * report their primary channels in AP channel report
			 * to save space.
			 */
			opclass = wlc_chanspec_ac2opclass(CH20MHZ_CHSPEC(primary_chan));
			opi = wlc_oce_opclass2index(opclass);

			ASSERT(oce_cmn->opcls_chlist[opi].cnt < OPCLASS_MAX_CHANNELS);
			oce_cmn->opcls_chlist[opi].opclass = opclass;
			oce_cmn->opcls_chlist[opi].chlist[oce_cmn->opcls_chlist[opi].cnt++] =
				primary_chan;
		}
	}
	return;
}

#endif /* WL_OCE_AP */

#ifdef WL_ESP
/* Parse ESP element in beacon/probe resp frames,
 * WFA OCE Tech Spec v0.0.13 3.15, 802.11-2016 9.4.2.174
 */
static int
wlc_oce_esp_ie_parse_fn(void *ctx, wlc_esp_attr_parse_data_t *data)
{
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;

	ASSERT(ctx && data);

	if (data->ie_len < DOT11_ESP_IE_INFO_LIST_SIZE)
		return BCME_OK;

	switch (data->ft) {
		case WLC_IEM_FC_SCAN_PRBRSP:
		case WLC_IEM_FC_SCAN_BCN:
		{
			wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
			wlc_bss_info_t *bi = ftpparm->scan.result;

			if (data->ie) {
				(void)wlc_oce_parse_esp_element(oce, bi, data->ie, data->ie_len);
			}
		}
			break;
		case FC_BEACON:
		case FC_PROBE_RESP:
		default:
			break;
	}

	return BCME_OK;
}

static void
wlc_oce_parse_esp_element(wlc_oce_info_t *oce, wlc_bss_info_t *bi,
	uint8* buf, uint16 buflen)
{
	uint16 i = 0;
	dot11_esp_ie_info_list_t *list = (dot11_esp_ie_info_list_t *)buf;
	uint16 nbr_of_lists = buflen/DOT11_ESP_IE_INFO_LIST_SIZE;

	/* looking only for Estimated AirTime Fraction of Best Effort */
	for (i = 0; i < nbr_of_lists; i++) {
		uint8 ac_df_baws = list->ac_df_baws;
		if (ac_df_baws & DOT11_ESP_INFO_LIST_AC_BE) {
			bi->eat_frac = list->eat_frac;
			break;
		}
		list++;
	}
}
#endif /* WL_ESP */
/* validation function for IOVAR */
static int
wlc_oce_iov_cmd_validate(const bcm_iov_cmd_digest_t *dig, uint32 actionid,
	const uint8 *ibuf, size_t ilen, uint8 *obuf, size_t *olen)
{
	int ret = BCME_OK;
	wlc_info_t *wlc = NULL;
	wlc_oce_info_t *oce = NULL;

	ASSERT(dig);
	oce = (wlc_oce_info_t *)dig->cmd_ctx;
	ASSERT(oce);
	wlc = oce->wlc;

	UNUSED_PARAMETER(wlc);

	if (!OCE_ENAB(wlc->pub) &&
		(dig->cmd_info->cmd != WL_OCE_CMD_ENABLE)) {
		WL_ERROR(("wl%d: %s: Command unsupported\n",
			wlc->pub->unit, __FUNCTION__));
		ret = BCME_UNSUPPORTED;
		goto fail;
	}
fail:
	return ret;
}

/* iovar context alloc */
static void *
oce_iov_context_alloc(void *ctx, uint size)
{
	uint8 *iov_ctx = NULL;
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;

	ASSERT(oce != NULL);

	iov_ctx = MALLOCZ(oce->wlc->osh, size);
	if (iov_ctx == NULL) {
		WL_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
			oce->wlc->pub->unit, __FUNCTION__, MALLOCED(oce->wlc->osh)));
	}

	return iov_ctx;
}

/* iovar context free */
static void
oce_iov_context_free(void *ctx, void *iov_ctx, uint size)
{
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;

	ASSERT(oce != NULL);
	if (iov_ctx) {
		MFREE(oce->wlc->osh, iov_ctx, size);
	}
}

/* command digest alloc function */
static int
BCMATTACHFN(oce_iov_get_digest_cb)(void *ctx, bcm_iov_cmd_digest_t **dig)
{
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;
	int ret = BCME_OK;
	uint8 *iov_cmd_dig = NULL;

	ASSERT(oce != NULL);
	iov_cmd_dig = MALLOCZ(oce->wlc->osh, sizeof(bcm_iov_cmd_digest_t));
	if (iov_cmd_dig == NULL) {
		WL_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
			oce->wlc->pub->unit, __FUNCTION__, MALLOCED(oce->wlc->osh)));
		ret = BCME_NOMEM;
		goto fail;
	}
	*dig = (bcm_iov_cmd_digest_t *)iov_cmd_dig;
fail:
	return ret;
}

/* "wl oce enable <>" handler */
static int
wlc_oce_iov_set_enable(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen)
{
	int ret = BCME_OK;
	uint16 len;
	uint16 type;
	wlc_oce_info_t *oce = (wlc_oce_info_t *)dig->cmd_ctx;
	const bcm_xtlv_t *ptlv = (const bcm_xtlv_t *)ibuf;
	const uint8 *data;

	if (!ibuf || !ilen) {
		return BCME_BADARG;
	}
	bcm_xtlv_unpack_xtlv(ptlv, &type, &len, &data, BCM_XTLV_OPTION_ALIGN32);

	if (type != WL_OCE_XTLV_ENABLE || !len) {
		WL_ERROR(("wl%d: %s: wrong xtlv: id %d len %d\n",
			oce->wlc->pub->unit, __FUNCTION__, type, len));

		return BCME_BADARG;
	}

	if (*data & OCE_PRB_REQ_RATE_DISABLE) {
		oce->oce_cmn_info->disable_oce_prb_req_rate = TRUE;
	} else {
		oce->oce_cmn_info->disable_oce_prb_req_rate = FALSE;
	}

	oce->wlc->pub->cmn->_oce = *data;
#ifdef WL_OCE_AP
	/* set/reset beacon rate */
	wlc_oce_set_bcn_rate(oce);

	if (OCE_ENAB(oce->wlc->pub)) {
		oce->oce_cmn_info->fd_tx_period = FD_TX_PERIOD;
		oce->oce_cmn_info->rnr_scan_period = RNR_SCAN_PERIOD;
		RNR_SCAN_DEL_TIMER(oce);
		/* do the scan right away */
		RNR_SCAN_ADD_TIMER(oce, RNR_SCAN_START_PERIOD);
	} else {
		oce->oce_cmn_info->fd_tx_period = 0;
		oce->oce_cmn_info->rnr_scan_period = 0;
	}
#endif
	wlc_update_beacon(oce->wlc);
	wlc_update_probe_resp(oce->wlc, FALSE);

	return ret;
}

/* "wl oce enable" handler */
static int
wlc_oce_iov_get_enable(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen)
{
	int ret = BCME_OK;
	uint16 xtlv_size = 0;
	wlc_oce_info_t *oce = (wlc_oce_info_t *)dig->cmd_ctx;
	uint16 buflen = 0;
	uint8 enable = oce->wlc->pub->cmn->_oce;

	if (!obuf || !*olen) {
		ret = BCME_BADARG;
		goto fail;
	}

	xtlv_size = bcm_xtlv_size_for_data(sizeof(enable),
		BCM_XTLV_OPTION_ALIGN32);
	if (xtlv_size > *olen) {
		WL_OCE_ERR(("wl%d: %s: short buffer length %d expected %u\n",
			oce->wlc->pub->unit, __FUNCTION__, (int)*olen, xtlv_size));
		*olen = 0;
		ret = BCME_BUFTOOSHORT;
		goto fail;
	}
	buflen = *olen;
	ret = bcm_pack_xtlv_entry(&obuf, &buflen, WL_OCE_XTLV_ENABLE,
			sizeof(oce->oce_cmn_info->probe_defer_time), &enable,
			BCM_XTLV_OPTION_ALIGN32);
	if (ret != BCME_OK) {
		WL_OCE_ERR(("wl%d: %s: packing xtlv failed\n",
			oce->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	*olen = *olen - buflen;

fail:
	return ret;
}

/* "wl oce probe_def_time <>" handler */
static int
wlc_oce_iov_set_probe_def_time(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen)
{
	int ret = BCME_OK;
	uint16 len;
	uint16 type;
	const bcm_xtlv_t *ptlv = (const bcm_xtlv_t *)ibuf;
	const uint8 *data;
	wlc_oce_info_t *oce = (wlc_oce_info_t *)dig->cmd_ctx;

	if (!ibuf || !ilen) {
		return BCME_BADARG;
	}

	bcm_xtlv_unpack_xtlv(ptlv, &type, &len, &data, BCM_XTLV_OPTION_ALIGN32);

	if (type != WL_OCE_XTLV_PROBE_DEF_TIME || !len) {
		WL_ERROR(("wl%d: %s: wrong xtlv: id %d len %d\n",
			oce->wlc->pub->unit, __FUNCTION__, type, len));

		return BCME_BADARG;
	}

	oce->oce_cmn_info->probe_defer_time = *data;

	return ret;
}

/* "wl oce probe_def_time" handler */
static int
wlc_oce_iov_get_probe_def_time(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen)
{
	int ret = BCME_OK;
	uint16 xtlv_size = 0;
	wlc_oce_info_t *oce = (wlc_oce_info_t *)dig->cmd_ctx;
	uint16 buflen = 0;

	if (!obuf || !*olen) {
		ret = BCME_BADARG;
		goto fail;
	}

	xtlv_size = bcm_xtlv_size_for_data(sizeof(oce->oce_cmn_info->probe_defer_time),
		BCM_XTLV_OPTION_ALIGN32);
	if (xtlv_size > *olen) {
		WL_OCE_ERR(("wl%d: %s: short buffer length %d expected %u\n",
			oce->wlc->pub->unit, __FUNCTION__, (int)*olen, xtlv_size));
		*olen = 0;
		ret = BCME_BUFTOOSHORT;
		goto fail;
	}
	buflen = *olen;
	ret = bcm_pack_xtlv_entry(&obuf, &buflen, WL_OCE_XTLV_PROBE_DEF_TIME,
			sizeof(oce->oce_cmn_info->probe_defer_time),
			&oce->oce_cmn_info->probe_defer_time,
			BCM_XTLV_OPTION_ALIGN32);
	if (ret != BCME_OK) {
		WL_OCE_ERR(("wl%d: %s: packing xtlv failed\n",
			oce->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	*olen = *olen - buflen;

fail:
	return ret;
}

static int
wlc_oce_bssload_ie_parse_fn(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;

	ASSERT(ctx && data);

	if (data->ie == NULL || data->ie_len < BSS_LOAD_IE_SIZE)
		return BCME_OK;

	switch (data->ft) {
		case WLC_IEM_FC_SCAN_PRBRSP:
		case WLC_IEM_FC_SCAN_BCN:
		{
			wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
			wlc_bss_info_t *bi = ftpparm->scan.result;

			if (data->ie) {
				(void)wlc_oce_parse_bssload_ie(oce, bi, data->ie, data->ie_len);
			}
		}
			break;
		default:
			break;
	}

	return BCME_OK;
}

static void
wlc_oce_parse_bssload_ie(wlc_oce_info_t *oce, wlc_bss_info_t *bi,
        uint8* buf, uint16 buflen)
{
	dot11_qbss_load_ie_t *data;

	ASSERT(oce && buf);

	data = (dot11_qbss_load_ie_t *)buf;

	bi->qbss_load_chan_free = (uint8)WLC_QBSS_LOAD_CHAN_FREE_MAX -
		data->channel_utilization;
}

void
oce_calc_join_pref(wlc_bsscfg_t *cfg, wlc_bss_info_t **bip, uint bss_cnt,
	join_pref_t *join_pref)
{
	uint j;

	/*
	 * Recalculate new score as sum of
	 * 40% prev score,
	 * 20% chan free from BSSLOAD IE,
	 * 20% Estimated Airtime Fraction from ESP IE,
	 * 20% Reduced WAN metrics from OCE IE
	 */
	for (j = 0; j < bss_cnt; j++) {
		join_pref[j].score = ((join_pref[j].score / 5 * 2) +
			(bip[j]->qbss_load_chan_free / 5) +
			(bip[j]->eat_frac / 5) +
			(bip[j]->rwan_links / 5));

		WL_OCE_INFO(("OCE: candidate to assoc,  chan_free %d, esp %d, rwan %d, score %d\n",
			bip[j]->qbss_load_chan_free, bip[j]->eat_frac,
			bip[j]->rwan_links, join_pref[j].score));
	}
}

#ifdef WL_OCE_AP
/* "wl oce fd_tx_period <>" handler */
static int
wlc_oce_iov_set_fd_tx_period(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen)
{
	int ret = BCME_OK;
	uint16 len;
	uint16 type;
	const bcm_xtlv_t *ptlv = (const bcm_xtlv_t *)ibuf;
	const uint8 *data;
	wlc_oce_info_t *oce = (wlc_oce_info_t *)dig->cmd_ctx;

	if (!ibuf || !ilen) {
		return BCME_BADARG;
	}

	bcm_xtlv_unpack_xtlv(ptlv, &type, &len, &data, BCM_XTLV_OPTION_ALIGN32);

	if (type != WL_OCE_XTLV_FD_TX_PERIOD || !len) {
		WL_ERROR(("wl%d: %s: wrong xtlv: id %d len %d\n",
			oce->wlc->pub->unit, __FUNCTION__, type, len));

		return BCME_BADARG;
	}

	oce->oce_cmn_info->fd_tx_period = *data;

	if (!oce->oce_cmn_info->fd_tx_period) {
		/* if fd disabled */
		wlc_gptimer_wake_upd(oce->wlc, WLC_GPTIMER_AWAKE_OCE, FALSE);
		FD_TX_DEL_TIMER(oce);
		return ret;
	}

	return ret;
}

/* "wl oce fd_tx_period" handler */
static int
wlc_oce_iov_get_fd_tx_period(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen)
{
	int ret = BCME_OK;
	uint16 xtlv_size = 0;
	wlc_oce_info_t *oce = (wlc_oce_info_t *)dig->cmd_ctx;
	uint16 buflen = 0;

	if (!obuf || !*olen) {
		ret = BCME_BADARG;
		goto fail;
	}

	xtlv_size = bcm_xtlv_size_for_data(sizeof(oce->oce_cmn_info->fd_tx_period),
		BCM_XTLV_OPTION_ALIGN32);
	if (xtlv_size > *olen) {
		WL_OCE_ERR(("wl%d: %s: short buffer length %d expected %u\n",
			oce->wlc->pub->unit, __FUNCTION__, (int)*olen, xtlv_size));
		*olen = 0;
		ret = BCME_BUFTOOSHORT;
		goto fail;
	}
	buflen = *olen;
	ret = bcm_pack_xtlv_entry(&obuf, &buflen, WL_OCE_XTLV_FD_TX_PERIOD,
			sizeof(oce->oce_cmn_info->fd_tx_period), &oce->oce_cmn_info->fd_tx_period,
			BCM_XTLV_OPTION_ALIGN32);
	if (ret != BCME_OK) {
		WL_OCE_ERR(("wl%d: %s: packing xtlv failed\n",
			oce->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	*olen = *olen - buflen;

fail:
	return ret;
}

/* "wl oce rej_retry_delay <>" handler */
static int
wlc_oce_iov_set_retry_delay(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen)
{
	int ret = BCME_OK;
	uint16 len;
	uint16 type;
	const bcm_xtlv_t *ptlv = (const bcm_xtlv_t *)ibuf;
	const uint8 *data;
	wlc_oce_info_t *oce = (wlc_oce_info_t *)dig->cmd_ctx;

	if (!ibuf || !ilen) {
		return BCME_BADARG;
	}

	bcm_xtlv_unpack_xtlv(ptlv, &type, &len, &data, BCM_XTLV_OPTION_ALIGN32);

	if (type != WL_OCE_XTLV_REJ_RETRY_DELAY || !len) {
		WL_ERROR(("wl%d: %s: wrong xtlv: id %d len %d\n",
			oce->wlc->pub->unit, __FUNCTION__, type, len));

		return BCME_BADARG;
	}
	oce->oce_cmn_info->rej_retry_delay = *data;

	return ret;
}

/* "wl oce rej_retry_delay" handler */
static int
wlc_oce_iov_get_retry_delay(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen)
{
	int ret = BCME_OK;
	uint16 xtlv_size = 0;
	wlc_oce_info_t *oce = (wlc_oce_info_t *)dig->cmd_ctx;
	uint16 buflen = 0;

	if (!obuf || !*olen) {
		ret = BCME_BADARG;
		goto fail;
	}

	xtlv_size = bcm_xtlv_size_for_data(sizeof(oce->oce_cmn_info->rej_retry_delay),
		BCM_XTLV_OPTION_ALIGN32);
	if (xtlv_size > *olen) {
		WL_OCE_ERR(("wl%d: %s: short buffer length %d expected %u\n",
			oce->wlc->pub->unit, __FUNCTION__, (int)*olen, xtlv_size));
		*olen = 0;
		ret = BCME_BUFTOOSHORT;
		goto fail;
	}
	buflen = *olen;

	ret = bcm_pack_xtlv_entry(&obuf, &buflen, WL_OCE_XTLV_REJ_RETRY_DELAY,
			sizeof(oce->oce_cmn_info->rej_retry_delay),
			&oce->oce_cmn_info->rej_retry_delay,
			BCM_XTLV_OPTION_ALIGN32);

	if (ret != BCME_OK) {
		WL_OCE_ERR(("wl%d: %s: packing xtlv failed\n",
			oce->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	*olen = *olen - buflen;

fail:
	return ret;
}

/* "wl oce fd_tx_duration <>" handler */
static int
wlc_oce_iov_set_fd_tx_duration(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen)
{
	int ret = BCME_OK;
	uint16 len;
	uint16 type;
	const bcm_xtlv_t *ptlv = (const bcm_xtlv_t *)ibuf;
	const uint8 *data;
	wlc_oce_info_t *oce = (wlc_oce_info_t *)dig->cmd_ctx;

	if (!ibuf || !ilen) {
		return BCME_BADARG;
	}

	bcm_xtlv_unpack_xtlv(ptlv, &type, &len, &data, BCM_XTLV_OPTION_ALIGN32);

	if (type != WL_OCE_XTLV_FD_TX_DURATION || !len) {
		WL_ERROR(("wl%d: %s: wrong xtlv: id %d len %d\n",
			oce->wlc->pub->unit, __FUNCTION__, type, len));

		return BCME_BADARG;
	}
	oce->oce_cmn_info->fd_tx_duration = load32_ua(data);
	return ret;
}

/* "wl oce fd_tx_duration" handler */
static int
wlc_oce_iov_get_fd_tx_duration(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen)
{
	int ret = BCME_OK;
	uint16 xtlv_size = 0;
	wlc_oce_info_t *oce = (wlc_oce_info_t *)dig->cmd_ctx;
	uint16 buflen = 0;

	if (!obuf || !*olen) {
		ret = BCME_BADARG;
		goto fail;
	}

	xtlv_size = bcm_xtlv_size_for_data(sizeof(oce->oce_cmn_info->fd_tx_duration),
		BCM_XTLV_OPTION_ALIGN32);
	if (xtlv_size > *olen) {
		WL_OCE_ERR(("wl%d: %s: short buffer length %d expected %u\n",
			oce->wlc->pub->unit, __FUNCTION__, (int)*olen, xtlv_size));
		*olen = 0;
		ret = BCME_BUFTOOSHORT;
		goto fail;
	}
	buflen = *olen;
	ret = bcm_pack_xtlv_entry(&obuf, &buflen, WL_OCE_XTLV_FD_TX_DURATION,
		sizeof(oce->oce_cmn_info->fd_tx_duration),
		(void*)&oce->oce_cmn_info->fd_tx_duration,
		BCM_XTLV_OPTION_ALIGN32);

	if (ret != BCME_OK) {
		WL_OCE_ERR(("wl%d: %s: packing xtlv failed\n",
			oce->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	*olen = *olen - buflen;

fail:
	return ret;
}


/* "wl oce rssi_th <>" handler */
static int
wlc_oce_iov_set_ass_rej_rssi_thd(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen)
{
	int ret = BCME_OK;
	uint16 len;
	uint16 type;
	const bcm_xtlv_t *ptlv = (const bcm_xtlv_t *)ibuf;
	const uint8 *data;
	wlc_oce_info_t *oce = (wlc_oce_info_t *)dig->cmd_ctx;

	bcm_xtlv_unpack_xtlv(ptlv, &type, &len, &data, BCM_XTLV_OPTION_ALIGN32);

	if (type != WL_OCE_XTLV_RSSI_TH || !len) {
		WL_ERROR(("wl%d: %s: wrong xtlv: id %d len %d\n",
			oce->wlc->pub->unit, __FUNCTION__, type, len));

		return BCME_BADARG;
	}

	oce->oce_cmn_info->ass_rej_rssi_thd = *data;

	return ret;
}

/* "wl oce rssi_th" handler */
static int
wlc_oce_iov_get_ass_rej_rssi_thd(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen)
{
	int ret = BCME_OK;
	uint16 xtlv_size = 0;
	wlc_oce_info_t *oce = (wlc_oce_info_t *)dig->cmd_ctx;
	uint16 buflen = 0;

	if (!obuf || !*olen) {
		ret = BCME_BADARG;
		goto fail;
	}

	xtlv_size = bcm_xtlv_size_for_data(sizeof(oce->oce_cmn_info->ass_rej_rssi_thd),
		BCM_XTLV_OPTION_ALIGN32);
	if (xtlv_size > *olen) {
		WL_OCE_ERR(("wl%d: %s: short buffer length %d expected %u\n",
			oce->wlc->pub->unit, __FUNCTION__, (int)*olen, xtlv_size));
		*olen = 0;
		ret = BCME_BUFTOOSHORT;
		goto fail;
	}

	buflen = *olen;
	ret = bcm_pack_xtlv_entry(&obuf, &buflen, WL_OCE_XTLV_RSSI_TH,
			sizeof(oce->oce_cmn_info->ass_rej_rssi_thd),
			(const uint8*)&oce->oce_cmn_info->ass_rej_rssi_thd,
			BCM_XTLV_OPTION_ALIGN32);
	if (ret != BCME_OK) {
		WL_OCE_ERR(("wl%d: %s: packing xtlv failed\n",
			oce->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	*olen = *olen - buflen;

fail:
	return ret;
}

/* "wl oce redwan_links" handler */
static int
wlc_oce_iov_get_redwan_links(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen)
{
	int ret = BCME_OK;
	uint16 xtlv_size = 0;
	wlc_oce_info_t *oce = (wlc_oce_info_t *)dig->cmd_ctx;
	uint16 buflen = 0;

	if (!obuf || !*olen) {
		ret = BCME_BADARG;
		goto fail;
	}

	xtlv_size = bcm_xtlv_size_for_data(sizeof(oce->oce_cmn_info->red_wan_links),
		BCM_XTLV_OPTION_ALIGN32);
	if (xtlv_size > *olen) {
		WL_OCE_ERR(("wl%d: %s: short buffer length %d expected %u\n",
			oce->wlc->pub->unit, __FUNCTION__, (int)*olen, xtlv_size));
		*olen = 0;
		ret = BCME_BUFTOOSHORT;
		goto fail;
	}

	buflen = *olen;
	ret = bcm_pack_xtlv_entry(&obuf, &buflen, WL_OCE_XTLV_RWAN_LINKS,
			sizeof(oce->oce_cmn_info->red_wan_links),
			(const uint8*)&oce->oce_cmn_info->red_wan_links,
			BCM_XTLV_OPTION_ALIGN32);
	if (ret != BCME_OK) {
		WL_OCE_ERR(("wl%d: %s: packing xtlv failed\n",
			oce->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	*olen = *olen - buflen;

fail:
	return ret;
}

/* "wl oce redwan_links <>" handler */
static int
wlc_oce_iov_set_redwan_links(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen)
{
	int ret = BCME_OK;
	uint16 len;
	uint16 type;
	const bcm_xtlv_t *ptlv = (const bcm_xtlv_t *)ibuf;
	const uint8 *data;
	wlc_oce_info_t *oce = (wlc_oce_info_t *)dig->cmd_ctx;

	bcm_xtlv_unpack_xtlv(ptlv, &type, &len, &data, BCM_XTLV_OPTION_ALIGN32);

	if (type != WL_OCE_XTLV_RWAN_LINKS || !len) {
		WL_ERROR(("wl%d: %s: wrong xtlv: id %d len %d\n",
			oce->wlc->pub->unit, __FUNCTION__, type, len));

		return BCME_BADARG;
	}

	oce->oce_cmn_info->red_wan_links = *data;

	wlc_update_beacon(oce->wlc);
	wlc_update_probe_resp(oce->wlc, FALSE);

	return ret;
}

/* "wl oce cu_trigger" handler */
static int
wlc_oce_iov_get_cu_trigger(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen)
{
	int ret = BCME_OK;
	uint16 xtlv_size = 0;
	wlc_oce_info_t *oce = (wlc_oce_info_t *)dig->cmd_ctx;
	uint16 buflen = 0;
	wlc_bsscfg_t *bsscfg;
	wlc_info_t *wlc;
	int i;
	uint8 roam_cu_trigger = 0; /* channel utilization roam trigger (%) */

	if (!obuf || !*olen) {
		ret = BCME_BADARG;
		goto fail;
	}

	wlc = oce->wlc;

	FOREACH_BSS(wlc, i, bsscfg) {
		if (BSSCFG_INFRA_STA(bsscfg) &&
			WLC_BSS_CONNECTED(bsscfg)) {
				roam_cu_trigger =
					wlc_wnm_get_cu_trigger_percent(wlc, bsscfg);
		}
	}

	if (!roam_cu_trigger) {
		WL_OCE_ERR(("wl%d: %s: bss not found\n",
			oce->wlc->pub->unit, __FUNCTION__));
		*olen = 0;
		ret = BCME_NOTFOUND;
		goto fail;
	}

	xtlv_size = bcm_xtlv_size_for_data(sizeof(roam_cu_trigger),
		BCM_XTLV_OPTION_ALIGN32);
	if (xtlv_size > *olen) {
		WL_OCE_ERR(("wl%d: %s: short buffer length %d expected %u\n",
			oce->wlc->pub->unit, __FUNCTION__, (int)*olen, xtlv_size));
		*olen = 0;
		ret = BCME_BUFTOOSHORT;
		goto fail;
	}

	buflen = *olen;
	ret = bcm_pack_xtlv_entry(&obuf, &buflen, WL_OCE_XTLV_CU_TRIGGER,
			sizeof(roam_cu_trigger),
			(const uint8*)&roam_cu_trigger,
			BCM_XTLV_OPTION_ALIGN32);
	if (ret != BCME_OK) {
		WL_OCE_ERR(("wl%d: %s: packing xtlv failed\n",
			oce->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	*olen = *olen - buflen;

fail:
	return ret;
}

/* "wl oce cu_trigger <>" handler */
static int
wlc_oce_iov_set_cu_trigger(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen)
{
	int ret = BCME_OK;
	uint16 len;
	uint16 type;
	const bcm_xtlv_t *ptlv = (const bcm_xtlv_t *)ibuf;
	const uint8 *data;
	wlc_oce_info_t *oce = (wlc_oce_info_t *)dig->cmd_ctx;
	wlc_bsscfg_t *bsscfg;
	wlc_info_t *wlc;
	int i;

	bcm_xtlv_unpack_xtlv(ptlv, &type, &len, &data, BCM_XTLV_OPTION_ALIGN32);

	if (type != WL_OCE_XTLV_CU_TRIGGER || !len) {
		WL_ERROR(("wl%d: %s: wrong xtlv: id %d len %d\n",
			oce->wlc->pub->unit, __FUNCTION__, type, len));

		return BCME_BADARG;
	}

	wlc = oce->wlc;

	FOREACH_BSS(wlc, i, bsscfg) {
		if (BSSCFG_INFRA_STA(bsscfg) &&
			WLC_BSS_CONNECTED(bsscfg)) {
				wlc_wnm_set_cu_trigger_percent(wlc, bsscfg, *data);
		}
	}

	return ret;
}

static void
wlc_oce_set_bcn_rate(wlc_oce_info_t *oce)
{
	if (OCE_ENAB(oce->wlc->pub) && BAND_2G(oce->wlc->band->bandtype)) {
		/* OCE demands min beacon transmit rate to be 5.5 Mbps OOB (WFA OCE spec v5 3.10) */
		wlc_set_force_bcn_rspec(oce->wlc, LEGACY_RSPEC(WLC_RATE_5M5));
	} else {
		/* reset beacon rate */
		wlc_set_force_bcn_rspec(oce->wlc, 0);
	}
}
#endif /* WL_OCE_AP */

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_oce_dump(void *ctx, struct bcmstrbuf *b)
{
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;

	bcm_bprintf(b, "OCE: %d \n", OCE_ENAB(oce->wlc->pub));
	bcm_bprintf(b, "     up_tick %u sys_up_time:%u\n", oce->up_tick, OSL_SYSUPTIME());
	bcm_bprintf(b, "OCE AP :\n");
	bcm_bprintf(b, "     FD frame tx period	%d \n", oce->oce_cmn_info->fd_tx_period);
	bcm_bprintf(b, "     FD frame tx duration %d \n", oce->oce_cmn_info->fd_tx_duration);
	bcm_bprintf(b, "     FD frame tx count %d \n", oce->oce_cmn_info->fd_frame_count);
	bcm_bprintf(b, "     assoc rejection rssi thrshld %d \n",
		oce->oce_cmn_info->ass_rej_rssi_thd);
	bcm_bprintf(b, "     rssi delta %d \n", oce->oce_cmn_info->rssi_delta);
	bcm_bprintf(b, "     RNR scan period	%d \n", oce->oce_cmn_info->rnr_scan_period);
	bcm_bprintf(b, "     Assoc retry delay	%d \n", oce->oce_cmn_info->rej_retry_delay);
	bcm_bprintf(b, "OCE STA :\n");
	bcm_bprintf(b, "     env_inc_non_oce_ap	%d \n", oce->oce_cmn_info->env_inc_non_oce_ap);
	bcm_bprintf(b, "     probe req deferral time %d \n",
		oce->oce_cmn_info->probe_defer_time);
	bcm_bprintf(b, "     probe resp listen time %d \n", oce->oce_cmn_info->max_channel_time);

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */
