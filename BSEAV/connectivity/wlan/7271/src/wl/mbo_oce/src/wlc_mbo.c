/*
 * MBO implementation for
 * Broadcom 802.11bang Networking Device Driver
 *
 * $Copyright (C) 2016 Broadcom Corporation$
 * $Id$
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 */

/**
 * @file
 * @brief
 * This file implements a part of WFA MBO features
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <osl.h>
#include <wl_dbg.h>
#include <wlioctl.h>
#include <wlc_bsscfg.h>

#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_types.h>
#include <wlc_ie_mgmt_types.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_vs.h>
#include <mbo.h>
#include <mbo_oce.h>
#include <wlc_mbo.h>
#ifdef ANQPO
#include <wl_anqpo.h>
#endif /* ANQPO */
#include <wlc_wnm.h>
#include <wlc_ie_mgmt_ft.h>
#include "wlc_mbo_oce_priv.h"
#include <wlc_iocv.h>
#include <bcmendian.h>
#ifndef WLWNM
#error "WNM is required for MBO"
#endif /* WNM */

#include <wlc_scb.h>

/* macro to control alloc and dealloc for sta only entity */
#define MBO_BSSCFG_STA(cfg) (BSSCFG_INFRA_STA(cfg) && ((cfg)->type == BSSCFG_TYPE_GENERIC))

/* 255 - ( OUI + OUI Type ) */
#define MBO_ELEM_MAX_ATTRS_LEN 251
/* 255 - ( OUI + OUI Type ) - sizeof(wifi_mbo_cell_data_cap_attr_t) */
#define MBO_MAX_NON_PREF_CHAN_ATTRS_LEN 248
#define MBO_MAX_NON_PREF_CHAN_SE_LEN 248
#define MAX_NON_PREF_CHAN_BODY_LEN  32
#define MBO_MAX_CELL_DATA_CAP_SE_LEN 7

#define MBO_BTQ_TRIGGER_START_OFFSET_MAX  60u  /* in seconds */
#define MBO_BTQ_TRIGGER_START_OFFSET_MIN  5u  /* in seconds */
#define MBO_BTQ_TRIGGER_START_OFFSET_DFLT 10u   /* in seconds */

#define MBO_BTQ_TRIGGER_RSSI_DELTA_MAX     10u /* in db */
#define MBO_BTQ_TRIGGER_RSSI_DELTA_MIN     5u  /* in db */
#define MBO_BTQ_TRIGGER_RSSI_DELTA_DFLT    10u /* in db */

/* flag handling macros */
#define MBO_FLAG_BIT_SET(flags, bit) ((flags) |= (bit))
#define MBO_FLAG_IS_BIT_SET(flags, bit) ((flags) & (bit))
#define MBO_FLAG_BIT_RESET(flags, bit) ((flags) &= ~(bit))

#define MBO_ASSOC_DISALLOWED_REASON_UNSPECIFIED			0x01
#define MBO_ASSOC_DISALLOWED_REASON_MAX_STA_LIMIT_REACHED	0x02
#define MBO_ASSOC_DISALLOWED_REASON_AIR_INTERFACE_OVERLOAD	0x03
#define MBO_ASSOC_DISALLOWED_REASON_AUTH_SERVER_OVERLOAD	0x04
#define MBO_ASSOC_DISALLOWED_REASON_INSUFFICIENT_RSSI		0x05

#define MBO_NP_CHAN_ATTR_OPCLASS_LEN		1 /* 1 byte */
#define MBO_NP_CHAN_ATTR_PREF_LEN		1 /* 1 byte */
#define MBO_NP_CHAN_ATTR_REASON_LEN		1 /* 1 byte */
#define MBO_NP_ATTR_FIXED_LEN	(MBO_NP_CHAN_ATTR_OPCLASS_LEN + \
						MBO_NP_CHAN_ATTR_PREF_LEN + \
						MBO_NP_CHAN_ATTR_REASON_LEN)

#define MBO_NP_SUBELEMENT_OPCLASS_OFFSET		5
#define MBO_OUI_LEN					3
#define MBO_OUI_TYPE_LEN				1
#define MBO_NP_SUBELEMENT_FIXED_LEN	 (MBO_NP_ATTR_FIXED_LEN + \
						MBO_OUI_LEN + MBO_OUI_TYPE_LEN)

#define MBO_NP_SUBELEMENT_CHAN_OUI_TYPE			2
#define MBO_NP_SUBELEMENT_CELL_OUI_TYPE			3

#define MBO_ATTRIBUTE_ID_OFFSET				0
#define MBO_ATTRIBUTE_LEN_OFFSET			1
#define MBO_ATTRIBUTE_OUI_OFFSET			2
#define MBO_WNM_SUBELEMENT_ID_AND_LEN			2
#define MBO_EMPTY_SUBELEMENT_LIST_LEN			4
#define MBO_NO_CHAN_LIST_SUBELEMENT_LEN			6
#define MBO_WNM_NOTIFICATION_MIN_SUBELEMENT_LEN		2
#define MBO_EMPTY_CELL_SUBELEMENT_LEN			0
#define MBO_WNM_CELL_SUBELEMENT_DATA_OFFSET		6

#define MBO_STA_MARKED_CHANNEL_NON_OPERABLE		0
#define MBO_STA_MARKED_CHANNEL_NON_PREFERABLE		1
#define MBO_STA_MARKED_CHANNEL_RANGE_RESERVED		2 /* 2- 254 */
#define MBO_STA_MARKED_CHANNEL_PREFERABLE		255

#define MBO_NON_PREF_CHAN_REPORT_ATTR 1
#define MBO_NON_PREF_CHAN_REPORT_SUBELEM 2

typedef struct np_chan_entry {
	uint8 chan;
	uint8 ref_cnt;
} np_chan_entry_t;

typedef struct mbo_np_chan_list mbo_np_chan_list_t;
struct mbo_np_chan_list {
	mbo_np_chan_list_t *next;
	uint8 opclass;
	uint8 pref;
	uint8 reason;
	uint8 list_len;
	uint8 list[16];	/* Max channels present in opclass eg: E-4 opclass - 81 */
};

typedef struct mbo_chan_pref_list mbo_chan_pref_list_t;
struct mbo_chan_pref_list {
	mbo_chan_pref_list_t *next;
	uint8	opclass;
	uint8	chan;
	uint8	pref;
	uint8	reason;
};

typedef struct wlc_mbo_data {
	/* configured cellular data capability of device */
	uint8 cell_data_cap;
	uint8 max_chan_pref_entries;
	wlc_mbo_oce_ie_build_hndl_t build_ie_hndl;
	wlc_mbo_oce_ie_parse_hndl_t parse_ie_hndl;
} wlc_mbo_data_t;

struct wlc_mbo_info {
	wlc_info_t *wlc;
	/* shared data */
	wlc_mbo_data_t *mbo_data;
	int      cfgh;    /* mbo bsscfg cubby handle */
	int      scbh;    /* mbo scb cubby handle */
	uint8	 fwd_gas_rqst_to_app;
};

typedef struct wlc_mbo_bsscfg_cubby {
	uint8 mbo_assoc_disallowed;
	uint8 mbo_ap_attr;
	/* counters */
	wl_mbo_counters_t *cntrs;
	/* flags for associated bss capability etc. */
	uint32 flags;
	/* configured non pref chan list for this bss */
	mbo_chan_pref_list_t *chan_pref_list_head;
	uint8 *np_chan_attr_buf;
	uint16 np_chan_attr_buf_len;
	/* bss transition reject reason */
	uint8 bsstrans_reject_reason;
	/* Time offset in seconds for a sending a BTM query on a new join */
	uint16 btq_trigger_offset;
	/* stored last BTM query token */
	uint8 btq_token;
	/* stored last BTM query reason */
	uint8 btq_reason;
	/* delta(in db, positive) w.r.t. roam trigger to decide when to send BTQ */
	uint8 btq_trigger_rssi_delta;
	/* stored roam trigger value when a BTM query was sent due to RSSI variation */
	int prev_roam_trigger;
} wlc_mbo_bsscfg_cubby_t;

typedef struct wlc_mbo_scb_cubby {
	/* flags for associated bss capability etc. */
	uint8 flags[1];
	/* configured non pref chan list for this bss */
	mbo_np_chan_list_t *np_chan_list_head;
	/* bss transition reject reason */
	uint8 bsstrans_reject_reason;
} wlc_mbo_scb_cubby_t;

#define MBO_SCB_CUBBY_LOC(mbo, scb) ((wlc_mbo_scb_cubby_t **)SCB_CUBBY((scb), (mbo)->scbh))
#define MBO_SCB_CUBBY(mbo, scb) (*(MBO_SCB_CUBBY_LOC(mbo, scb)))

#define MBO_BSSCFG_CUBBY_LOC(mbo, cfg) ((wlc_mbo_bsscfg_cubby_t **)BSSCFG_CUBBY(cfg, (mbo)->cfgh))
#define MBO_BSSCFG_CUBBY(mbo, cfg) ((wlc_mbo_bsscfg_cubby_t*)BSSCFG_CUBBY(cfg, ((mbo)->cfgh)))
#define MBO_CUBBY_CFG_SIZE  sizeof(wlc_mbo_bsscfg_cubby_t)

#define SUBCMD_TBL_SZ(_cmd_tbl)  (sizeof(_cmd_tbl)/sizeof(*_cmd_tbl))

enum wlc_mbo_iov {
	IOV_MBO = 0,
	IOV_MBO_ENABLE = 1,
	IOV_LAST
};

static const bcm_iovar_t mbo_iovars[] = {
	{"mbo", IOV_MBO, 0, 0, IOVT_BUFFER, 0},
	{"mbo_enable", IOV_MBO_ENABLE, 0, 0, IOVT_INT32, 0},
	{NULL, 0, 0, 0, 0, 0}
};

static void wlc_mbo_watchdog(void *ctx);
static int wlc_mbo_wlc_up(void *ctx);
static int wlc_mbo_wlc_down(void *ctx);

static int
wlc_mbo_doiovar(void *hdl, uint32 actionid,
	void *params, uint p_len, void *arg, uint len, uint val_size, struct wlc_if *wlcif);

static int wlc_mbo_iov_get_mbo_ap_attr(void *hndl, const uint8 *ibuf, size_t ilen, uint8 *obuf,
	size_t *olen, wlc_bsscfg_t *bsscfg);

static int wlc_mbo_iov_get_ap_attr_assoc_disallowed(void *hndl, const uint8 *ibuf, size_t ilen,
	uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg);

static int wlc_mbo_iov_set_ap_attr_assoc_disallowed(void *hndl, const uint8 *ibuf, size_t ilen,
	uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg);

static int wlc_mbo_iov_set_fwd_gas_rqst_to_app(void *hndl, const uint8 *ibuf, size_t ilen,
	uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg);

static int wlc_mbo_iov_get_fwd_gas_rqst_to_app(void *hndl, const uint8 *ibuf, size_t ilen,
	uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg);

static int wlc_mbo_scb_init(void *ctx, struct scb *scb);
static void wlc_mbo_scb_deinit(void *ctx, struct scb *scb);

static int wlc_mbo_process_scb_np_chan_list(wlc_mbo_info_t* mbo, wlc_mbo_scb_cubby_t *mbo_scb,
	uint8* ibuf, bool attr);
static int wlc_mbo_update_scb_np_chan_list(wlc_mbo_info_t* mbo, wlc_mbo_scb_cubby_t *mbo_scb,
	uint8 *ibuf, bool attr,	uint8 chan_list_len);

static int wlc_mbo_free_np_chan_list(wlc_mbo_info_t* mbo, wlc_mbo_scb_cubby_t *mbo_scb);

static uint8 wlc_mbo_get_count_chan_from_non_pref_list(uint8* ibuf, bool attr);

static int wlc_mbo_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_mbo_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg);
static int
wlc_mbo_iov_add_chan_pref(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg);
static int
wlc_mbo_iov_del_chan_pref(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg);
static int
wlc_mbo_iov_list_chan_pref(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg);
static int
wlc_mbo_iov_set_cellular_data_cap(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg);
static int
wlc_mbo_iov_get_cellular_data_cap(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg);
static int
wlc_mbo_iov_dump_counters(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg);
static int
wlc_mbo_iov_clear_counters(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg);
#ifdef WL_MBO_TB
static int
wlc_mbo_iov_set_force_assoc(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg);
static int
wlc_mbo_iov_get_force_assoc(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg);
static int
wlc_mbo_iov_set_bsstrans_reject(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg);
static int
wlc_mbo_iov_get_bsstrans_reject(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg);
#endif /* WL_MBO_TB */
static int
wlc_mbo_send_wnm_notif(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	uint8 sub_elem_type);
static int
wlc_mbo_iov_send_notif(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg);
static int
wlc_mbo_iov_set_nbr_info_cache(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg);
static int
wlc_mbo_iov_get_nbr_info_cache(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg);
static int
wlc_mbo_iov_set_anqpo_support(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg);
static int
wlc_mbo_iov_get_anqpo_support(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg);

static int wlc_mbo_ie_build_fn(void *ctx, wlc_mbo_oce_attr_build_data_t *data);
static uint wlc_mbo_ie_supp_opclass_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_mbo_ie_supp_opclass_build_fn(void *ctx, wlc_iem_build_data_t *data);
static int wlc_mbo_ie_parse_fn(void *ctx, wlc_mbo_oce_attr_parse_data_t *data);
static void wlc_mbo_free_chan_pref_list(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg);
static void wlc_mbo_add_non_pref_chan_attr_header(uint8 *cp, uint8 buf_len, uint8 len);
static void wlc_mbo_add_non_pref_chan_subelem_header(uint8 *cp, uint8 buf_len, uint8 len);
static int
wlc_mbo_validate_chan_pref(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	wlc_mbo_chan_pref_t *ch_pref);
static int
wlc_mbo_add_chan_pref_to_list(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	wlc_mbo_chan_pref_t *ch_pref);
static int
wlc_mbo_del_chan_pref_from_list(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	wlc_mbo_chan_pref_t *ch_pref);
static int
wlc_mbo_prep_non_pref_chan_report(wlc_mbo_info_t *mbo,
	wlc_bsscfg_t *bsscfg, uint8 *report_buf,
	uint16 *report_buf_len, uint8 report_type);
static void
wlc_mbo_prep_cell_data_cap_subelement(uint8 cap, uint8 *buf, uint16 *buf_len);
static int
wlc_mbo_prep_non_pref_chan_report_body(mbo_chan_pref_list_t *start,
	mbo_chan_pref_list_t *end, uint8 chan_list_len, uint8* buf, uint8 *buf_len);
static uint16
wlc_mbo_count_pref_chan_list_entry(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg);
static int
wlc_mbo_bsscfg_get(void *ctx, wlc_bsscfg_t *cfg, uint8 *data, int *len);
static int
wlc_mbo_bsscfg_set(void *ctx, wlc_bsscfg_t *cfg, const uint8 *data, int len);
static void
wlc_mbo_update_assoc_disallowed(wlc_bss_info_t *bi,
	wifi_mbo_assoc_disallowed_attr_t *assoc_dis);
static void
wlc_mbo_update_ap_cap(wlc_bss_info_t *bi,
	wifi_mbo_ap_cap_ind_attr_t *ap_cap);
static int
wlc_mbo_chan_pref_cbfn(void *ctx, const uint8 *data, uint16 type, uint16 len);
#ifdef WL_MBO_TB
static int
wlc_mbo_set_force_assoc(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	uint8 value);
static int
wlc_mbo_get_force_assoc(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	uint8 *value);
#endif /* WL_MBO_TB */


#define MAX_SET_GET_DATA_CAP_SIZE  8
#define MAX_ADD_CHAN_PREF_CFG_SIZE  32
#define MAX_DEL_CHAN_PREF_CFG_SIZE  16
#define MAX_CELLULAR_DATA_CAP_SIZE  8
#define MAX_FORCE_ASSOC_CFG_SIZE  8
#define MIN_BSSTRANS_REJECT_CFG_SIZE 8
#define MAX_BSSTRANS_REJECT_CFG_SIZE 16
#define MAX_SEND_NOTIF_CFG_SIZE  8
#define MIN_NBR_INFO_CACHE_CFG_SIZE 8
#define MAX_NBR_INFO_CACHE_CFG_SIZE 24
#define MIN_ANQPO_SUPPORT_CFG_SIZE 8
#define MAX_ANQPO_SUPPORT_CFG_SIZE 16

static const bcm_iov_cmd_info_t mbo_sub_cmds[] = {
	{WL_MBO_CMD_AP_ATTRIBUTE, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32,
	NULL, wlc_mbo_iov_get_mbo_ap_attr,
	NULL, 0,
	MAX_SET_GET_DATA_CAP_SIZE, MAX_SET_GET_DATA_CAP_SIZE, 0, 0
	},
	{WL_MBO_CMD_AP_ASSOC_DISALLOWED, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32,
	NULL, wlc_mbo_iov_get_ap_attr_assoc_disallowed,
	wlc_mbo_iov_set_ap_attr_assoc_disallowed, 0,
	MAX_SET_GET_DATA_CAP_SIZE, MAX_SET_GET_DATA_CAP_SIZE, 0, 0
	},
	{WL_MBO_CMD_AP_FWD_GAS_RQST_TO_APP, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32,
	NULL, wlc_mbo_iov_get_fwd_gas_rqst_to_app,
	wlc_mbo_iov_set_fwd_gas_rqst_to_app, 0,
	MAX_SET_GET_DATA_CAP_SIZE, MAX_SET_GET_DATA_CAP_SIZE, 0, 0
	},
	{WL_MBO_CMD_ADD_CHAN_PREF, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32,
	NULL, NULL, wlc_mbo_iov_add_chan_pref, 0,
	MAX_ADD_CHAN_PREF_CFG_SIZE, MAX_ADD_CHAN_PREF_CFG_SIZE, 0, 0
	},
	{WL_MBO_CMD_DEL_CHAN_PREF, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32,
	NULL, NULL, wlc_mbo_iov_del_chan_pref, 0,
	MAX_DEL_CHAN_PREF_CFG_SIZE, MAX_DEL_CHAN_PREF_CFG_SIZE, 0, 0
	},
	{WL_MBO_CMD_LIST_CHAN_PREF, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32,
	NULL, wlc_mbo_iov_list_chan_pref, NULL, 0,
	0, 0, 0, 0
	},
	{WL_MBO_CMD_CELLULAR_DATA_CAP, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32,
	NULL, wlc_mbo_iov_get_cellular_data_cap,
	wlc_mbo_iov_set_cellular_data_cap, 0,
	MAX_CELLULAR_DATA_CAP_SIZE, MAX_CELLULAR_DATA_CAP_SIZE, 0, 0
	},
	{WL_MBO_CMD_DUMP_COUNTERS, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32,
	NULL, wlc_mbo_iov_dump_counters, NULL, 0,
	0, 0, 0, 0
	},
	{WL_MBO_CMD_CLEAR_COUNTERS, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32,
	NULL, NULL, wlc_mbo_iov_clear_counters, 0,
	0, 0, 0, 0
	},
#ifdef WL_MBO_TB
	{WL_MBO_CMD_FORCE_ASSOC, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32,
	NULL, wlc_mbo_iov_get_force_assoc,
	wlc_mbo_iov_set_force_assoc, 0,
	MAX_FORCE_ASSOC_CFG_SIZE, MAX_FORCE_ASSOC_CFG_SIZE, 0, 0
	},
	{WL_MBO_CMD_BSSTRANS_REJECT, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32,
	NULL, wlc_mbo_iov_get_bsstrans_reject,
	wlc_mbo_iov_set_bsstrans_reject, 0,
	MIN_BSSTRANS_REJECT_CFG_SIZE, MAX_BSSTRANS_REJECT_CFG_SIZE, 0, 0
	},
#endif /* WL_MBO_TB */
	{WL_MBO_CMD_SEND_NOTIF, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32,
	NULL, NULL, wlc_mbo_iov_send_notif, 0,
	MAX_SEND_NOTIF_CFG_SIZE, MAX_SEND_NOTIF_CFG_SIZE, 0, 0
	},
	{WL_MBO_CMD_NBR_INFO_CACHE, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32,
	NULL, wlc_mbo_iov_get_nbr_info_cache,
	wlc_mbo_iov_set_nbr_info_cache, 0,
	MIN_NBR_INFO_CACHE_CFG_SIZE, MAX_NBR_INFO_CACHE_CFG_SIZE, 0, 0
	},
	{WL_MBO_CMD_ANQPO_SUPPORT, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32,
	NULL, wlc_mbo_iov_get_anqpo_support,
	wlc_mbo_iov_set_anqpo_support, 0,
	MIN_ANQPO_SUPPORT_CFG_SIZE, MAX_ANQPO_SUPPORT_CFG_SIZE, 0, 0
	},
};

static int
wlc_mbo_doiovar(void *hdl, uint32 actionid,
	void *params, uint p_len, void *arg, uint len, uint val_size, struct wlc_if *wlcif)
{
	int err = BCME_OK;
	uint16 version;
	uint8 *p_buf = NULL;
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t*)hdl;
	wlc_bsscfg_t *bsscfg;
	bcm_iov_buf_t *p_resp;
	bcm_iov_buf_t *p_cmd;
	const bcm_iov_cmd_info_t *cmd_arg = NULL;
	const uint8* cmd_data;
	uint16 i;
	uint16 cmd_len, min_len = 0, max_len = 0;
	bcm_iov_buf_t *p_subcmd;
	void* result;
	int res_len;
	int avail;
	uint8 *res = NULL;
	bool cmd_id_match = FALSE;
	int index;
	uint16 cmd_id;

	ASSERT(params != NULL);
	ASSERT(mbo != NULL);
	ASSERT(arg != NULL);

	if (p_len < OFFSETOF(bcm_iov_buf_t, len)) {
		err = BCME_BUFTOOSHORT;
		goto done;
	}

	/*
	 * Get version
	 */
	memcpy(&version, params, sizeof(version));
	version = ltoh16_ua(&version);

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(mbo->wlc, wlcif);
	ASSERT(bsscfg != NULL);

	/* update wlcif pointer */
	if (wlcif == NULL)
		wlcif = bsscfg->wlcif;
	ASSERT(wlcif != NULL);

	if (actionid == IOV_GVAL(IOV_MBO_ENABLE)) {
		int32 *ret_int_ptr;

		ret_int_ptr = (int32 *)arg;
		*ret_int_ptr = MBO_ENAB(mbo->wlc->pub);
		return err;
	} else if (actionid == IOV_SVAL(IOV_MBO_ENABLE)) {
		int32 int_val = 0;
		bool bool_val = FALSE;

		/* convenience int and bool vals for first 8 bytes of buffer */
		if (p_len >= (int)sizeof(int_val))
			bcopy(params, &int_val, sizeof(int_val));

		bool_val = (int_val != 0) ? TRUE : FALSE;
		mbo->wlc->pub->cmn->_mbo = bool_val;
		return err;
	}

	if (!MBO_ENAB(mbo->wlc->pub)) {
		WL_MBO_ERR(("wl%d: %s: MBO unsupported\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		err = BCME_UNSUPPORTED;
		goto done;
	}

	/*
	 * Input buffer does not overlap with output. Just point
	 * p_buf to input buffer
	 */
	p_buf = params;
	p_cmd = (bcm_iov_buf_t *)p_buf;
	p_resp = (bcm_iov_buf_t *)arg;
	cmd_id = ltoh16(p_cmd->id);

	cmd_data = ((uint8 *)p_cmd + OFFSETOF(bcm_iov_buf_t, data));
	cmd_len = ltoh16(p_cmd->len);
	result = arg;
	avail = (int)len;
	/*
	 * Set up resp buffer and its max len and ensure space for resp hdr
	 */
	res = (uint8 *)result + OFFSETOF(bcm_iov_buf_t, data);
	if (avail < (res - (uint8 *)result)) {
		err = BCME_BUFTOOSHORT;
		goto done; /* note: no space to return command status */
	}
	res_len = avail - (int)(res - (uint8 *)result);

	for (i = 0; i < (size_t)SUBCMD_TBL_SZ(mbo_sub_cmds); i++) {
		if (cmd_id == mbo_sub_cmds[i].cmd) {
			index = i;
			cmd_id_match = TRUE;
			cmd_arg = &mbo_sub_cmds[index];
			break;
		}
	}

	if (!cmd_id_match) {
		err = BCME_NOTFOUND;
		goto done;
	}

	/*
	 * Validate lengths.
	 */
	if (IOV_ISSET(actionid)) {
		min_len = cmd_arg->min_len_set;
		max_len = cmd_arg->max_len_set;
	} else {
		min_len = cmd_arg->min_len_get;
		max_len = cmd_arg->max_len_get;
	}

	if (cmd_len < min_len) {
		err = BCME_BADLEN;
		goto have_result;
	}

	/* process only supported len and allow future extension */
	cmd_len = MIN(cmd_len, max_len);
	/*
	 * Dispatch get/set
	 */
	if (IOV_ISSET(actionid)) {
		if (cmd_arg->set_h) {
			err = (cmd_arg->set_h)(mbo, cmd_data, (size_t)cmd_len,
				res, (size_t *)&res_len, bsscfg);
		} else {
			err = BCME_UNSUPPORTED;
		}
	} else {
		if (cmd_arg->get_h) {
			err = (cmd_arg->get_h)(mbo, cmd_data, (size_t)cmd_len,
				res, (size_t *)&res_len, bsscfg);
		} else {
			err = BCME_UNSUPPORTED;
		}
	}

	p_resp->version = htol16(version);

have_result:
	/* upon error, return only status - return values/buffer not deterministic */
	if (err != BCME_OK) {
		res_len = 0;
	}

	/*
	 * Done with non-batched command. handlers fill the data
	 * framework fills the version, id and len. Non batched
	 * commands get return status in bcm_iovar_t
	 */
	p_subcmd = (bcm_iov_buf_t *)result;
	p_subcmd->version = htol16((uint16)version);
	p_subcmd->id = htol16(cmd_id);
	if (cmd_arg->flags & BCM_IOV_CMD_FLAG_HDR_IN_LEN) {
		res_len += OFFSETOF(bcm_iov_buf_t, data);
	}
	p_subcmd->len = htol16((uint16)res_len);

done:
	return err;
}

wlc_mbo_info_t *
BCMATTACHFN(wlc_mbo_attach)(wlc_info_t *wlc)
{
	wlc_mbo_info_t *mbo = NULL;
	wlc_mbo_data_t *mbo_data = NULL;
	int ret = BCME_OK;
	bsscfg_cubby_params_t cubby_params;
	uint16 mbo_ie_build_fstbmp =
		FT2BMP(FC_BEACON) |
		FT2BMP(FC_PROBE_RESP) |
		FT2BMP(FC_ASSOC_RESP) |
		FT2BMP(FC_REASSOC_RESP) |
		FT2BMP(FC_ASSOC_REQ) |
		FT2BMP(FC_REASSOC_REQ) |
		FT2BMP(FC_PROBE_REQ);
	uint16 supp_opcls_fstbmp = FT2BMP(FC_ASSOC_REQ) |
		FT2BMP(FC_REASSOC_REQ);
	uint16 mbo_ie_parse_fstbmp =
		FT2BMP(FC_ASSOC_REQ) |
		FT2BMP(FC_REASSOC_REQ) |
		FT2BMP(FC_BEACON) |
		FT2BMP(FC_PROBE_RESP) |
		FT2BMP(FC_ASSOC_RESP) |
		FT2BMP(FC_REASSOC_RESP) |
		FT2BMP(WLC_IEM_FC_SCAN_BCN) |
		FT2BMP(WLC_IEM_FC_SCAN_PRBRSP);
	wlc_mbo_oce_ie_build_data_t build_data;
	wlc_mbo_oce_ie_parse_data_t parse_data;

	mbo = (wlc_mbo_info_t *)MALLOCZ(wlc->osh, sizeof(*mbo));
	if (mbo == NULL) {
		WL_ERROR(("wl%d: %s:out of mem. alloced %u bytes\n",
			wlc->pub->unit, __FUNCTION__,  MALLOCED(wlc->osh)));
		goto fail;
	}

	mbo_data = (wlc_mbo_data_t *) obj_registry_get(wlc->objr, OBJR_MBO_CMN_DATA);
	if (mbo_data == NULL) {
		mbo_data = (wlc_mbo_data_t *)MALLOCZ(wlc->osh, sizeof(*mbo_data));
		if (mbo_data == NULL) {
			WL_ERROR(("wl%d: %s:out of mem. alloced %u bytes\n",
				wlc->pub->unit, __FUNCTION__,  MALLOCED(wlc->osh)));
			goto fail;
		}
		obj_registry_set(wlc->objr, OBJR_MBO_CMN_DATA, mbo_data);

		/* register MBO IE attr build callback */
		memset(&build_data, 0, sizeof(build_data));
		build_data.ctx = mbo;
		build_data.fstbmp = mbo_ie_build_fstbmp;
		build_data.build_fn = wlc_mbo_ie_build_fn;
		mbo_data->build_ie_hndl =
			wlc_mbo_oce_register_ie_build_cb(wlc->mbo_oce, &build_data);
		if (mbo_data->build_ie_hndl == NULL) {
			WL_ERROR(("wl%d: %s:MBO IE build callback registration failed\n",
				wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
		/* register MBO IE attr parse callback */
		memset(&parse_data, 0, sizeof(parse_data));
		parse_data.ctx = mbo;
		parse_data.fstbmp = mbo_ie_parse_fstbmp;
		parse_data.parse_fn = wlc_mbo_ie_parse_fn;
		mbo_data->parse_ie_hndl =
			wlc_mbo_oce_register_ie_parse_cb(wlc->mbo_oce, &parse_data);
		if (mbo_data->parse_ie_hndl == NULL) {
			WL_ERROR(("wl%d: %s:MBO IE parse callback registration failed\n",
				wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
		/* set default value for cellular data capability of device */
		mbo_data->cell_data_cap = MBO_CELL_DATA_CONN_NOT_CAPABLE;
	}

	mbo->mbo_data = mbo_data;
	(void)obj_registry_ref(wlc->objr, OBJR_MBO_CMN_DATA);
	mbo->wlc = wlc;
	mbo->mbo_data->max_chan_pref_entries = MBO_MAX_CHAN_PREF_ENTRIES;

	/* register module */
	ret = wlc_module_register(wlc->pub, mbo_iovars, "mbo", mbo,
		wlc_mbo_doiovar, wlc_mbo_watchdog,
		wlc_mbo_wlc_up, wlc_mbo_wlc_down);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* reserve cubby space in the bsscfg container for per-bsscfg private data */
	bzero(&cubby_params, sizeof(cubby_params));

	cubby_params.context = mbo;
	cubby_params.fn_init = wlc_mbo_bsscfg_init;
	cubby_params.fn_deinit = wlc_mbo_bsscfg_deinit;
	cubby_params.fn_get = wlc_mbo_bsscfg_get;
	cubby_params.fn_set = wlc_mbo_bsscfg_set;
	cubby_params.config_size = MBO_CUBBY_CFG_SIZE;

	mbo->cfgh = wlc_bsscfg_cubby_reserve_ext(wlc, sizeof(wlc_mbo_bsscfg_cubby_t *),
											 &cubby_params);
	if (mbo->cfgh < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
			wlc->pub->unit, __FUNCTION__));
			goto fail;
	}

	/* register supported opclass element build callback */
	ret = wlc_iem_add_build_fn_mft(wlc->iemi, supp_opcls_fstbmp, DOT11_MNG_REGCLASS_ID,
		wlc_mbo_ie_supp_opclass_len, wlc_mbo_ie_supp_opclass_build_fn, mbo);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn_mft(supp opclass) failed %d\n",
			wlc->pub->unit, __FUNCTION__, ret));
		goto fail;
	}

	/* reserve cubby in the scb container for per-scb private data */
	if ((mbo->scbh = wlc_scb_cubby_reserve(wlc, sizeof(wlc_mbo_scb_cubby_t *),
	                wlc_mbo_scb_init, wlc_mbo_scb_deinit, NULL /* for dump routine */,
	                (void *)mbo)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_scb_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	wlc->pub->cmn->_mbo = TRUE;
	return mbo;
fail:

	wlc_mbo_detach(mbo);
	return NULL;
}

void
BCMATTACHFN(wlc_mbo_detach)(wlc_mbo_info_t* mbo)
{
	int ret = BCME_OK;
	wlc_info_t *wlc = NULL;
	if (mbo) {
		wlc = mbo->wlc;
		mbo->wlc->pub->cmn->_mbo = FALSE;

		if (mbo->mbo_data && obj_registry_unref(mbo->wlc->objr, OBJR_MBO_CMN_DATA) == 0) {
			if (mbo->mbo_data->build_ie_hndl) {
				ret = wlc_mbo_oce_unregister_ie_build_cb(wlc->mbo_oce,
					mbo->mbo_data->build_ie_hndl);
				if (ret != BCME_OK) {
					WL_ERROR(("wl%d: %s build cb unregistration failed %d\n",
						wlc->pub->unit, __FUNCTION__, ret));
				} else {
					mbo->mbo_data->build_ie_hndl = NULL;
				}
			}
			if (mbo->mbo_data->parse_ie_hndl) {
				ret = wlc_mbo_oce_unregister_ie_parse_cb(wlc->mbo_oce,
					mbo->mbo_data->parse_ie_hndl);
				if (ret != BCME_OK) {
					WL_ERROR(("wl%d: %s parse cb unregisteration failed %d\n",
						wlc->pub->unit, __FUNCTION__, ret));
				} else {
					mbo->mbo_data->parse_ie_hndl = NULL;
				}
			}
			obj_registry_set(wlc->objr, OBJR_MBO_CMN_DATA, NULL);
			MFREE(wlc->osh, mbo->mbo_data, sizeof(*mbo->mbo_data));
			mbo->mbo_data = NULL;
		}
		wlc_module_unregister(wlc->pub, "mbo", mbo);
		MFREE(wlc->osh, mbo, sizeof(*mbo));
		mbo = NULL;
	}
}

static void
wlc_mbo_watchdog(void *ctx)
{

}

static int
wlc_mbo_wlc_up(void *ctx)
{
	return BCME_OK;
}

static int
wlc_mbo_wlc_down(void *ctx)
{
	return BCME_OK;
}

/* "wl mbo cell_data_cap" handler */
static int
wlc_mbo_iov_get_mbo_ap_attr(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg)
{
	int ret = BCME_OK;
	uint16 xtlv_size = 0;
	uint16 nbytes = 0;
	wlc_mbo_bsscfg_cubby_t *mbc;

	uint16 buflen = 0;
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t*)hndl;

	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	ASSERT(mbc);
	nbytes = sizeof(mbc->mbo_ap_attr);

	xtlv_size = bcm_xtlv_size_for_data(sizeof(uint8), BCM_XTLV_OPTION_ALIGN32);
	if (xtlv_size > *olen) {
		WL_MBO_ERR(("wl%d: %s: short buffer length %d expected %u\n",
			mbo->wlc->pub->unit, __FUNCTION__, (int)(*olen), xtlv_size));
		*olen = 0;
		ret = BCME_BUFTOOSHORT;
		goto fail;
	}
	buflen = *olen;

	ret = bcm_pack_xtlv_entry(&obuf, &buflen, WL_MBO_XTLV_AP_ATTR,
			nbytes, &mbc->mbo_ap_attr, BCM_XTLV_OPTION_ALIGN32);
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: packing xtlv failed\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	*olen = *olen - buflen;

fail:
	return ret;
}

/* "wl mbo cell_data_cap" handler */
static int
wlc_mbo_iov_get_ap_attr_assoc_disallowed(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg)
{
	int ret = BCME_OK;
	uint16 xtlv_size = 0;
	wlc_mbo_bsscfg_cubby_t *mbc;

	uint16 buflen = 0;
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t*)hndl;
	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	ASSERT(mbc);

	xtlv_size = bcm_xtlv_size_for_data(sizeof(mbc->mbo_assoc_disallowed),
		BCM_XTLV_OPTION_ALIGN32);
	if (xtlv_size > *olen) {
		WL_MBO_ERR(("wl%d: %s: short buffer length %d expected %u\n",
			mbo->wlc->pub->unit, __FUNCTION__, (int)(*olen), xtlv_size));
		*olen = 0;
		ret = BCME_BUFTOOSHORT;
		goto fail;
	}
	buflen = *olen;

	ret = bcm_pack_xtlv_entry(&obuf, &buflen, WL_MBO_XTLV_AP_ASSOC_DISALLOWED,
			sizeof(mbc->mbo_assoc_disallowed), &mbc->mbo_assoc_disallowed,
			BCM_XTLV_OPTION_ALIGN32);
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: packing xtlv failed\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	*olen = *olen - buflen;

fail:
	return ret;
}

/* "wl mbo cell_data_cap <>" handler */
static int
wlc_mbo_iov_set_ap_attr_assoc_disallowed(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg)
{
	int ret = BCME_OK;
	uint8 data = 0;
	uint8 prev_val = 0;
	wlc_mbo_bsscfg_cubby_t *mbc;
	uint16 nbytes = 0;

	wlc_mbo_info_t *mbo = (wlc_mbo_info_t*)hndl;

	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	ASSERT(mbc);
	nbytes = sizeof(mbc->mbo_assoc_disallowed);

#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
	ret = bcm_unpack_xtlv_entry((uint8 **)&ibuf, WL_MBO_XTLV_AP_ASSOC_DISALLOWED, nbytes,
		&data, BCM_XTLV_OPTION_ALIGN32);
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: unpacking xtlv failed\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	prev_val = mbc->mbo_assoc_disallowed;
	mbc->mbo_assoc_disallowed = data;
	if (prev_val != mbc->mbo_assoc_disallowed) {
		/* update AP or IBSS beacons */
		wlc_bss_update_beacon(mbo->wlc, bsscfg);
		/* update AP or IBSS probe responses */
		wlc_bss_update_probe_resp(mbo->wlc, bsscfg, TRUE);
	}
fail:
	return ret;
}

static int
wlc_mbo_iov_get_fwd_gas_rqst_to_app(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg)
{
	int ret = BCME_OK;
	uint16 xtlv_size = 0;

	uint16 buflen = 0;
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t*)hndl;

	xtlv_size = bcm_xtlv_size_for_data(sizeof(mbo->fwd_gas_rqst_to_app),
		BCM_XTLV_OPTION_ALIGN32);
	if (xtlv_size > *olen) {
		WL_MBO_ERR(("wl%d: %s: short buffer length %d expected %u\n",
			mbo->wlc->pub->unit, __FUNCTION__, (int)(*olen), xtlv_size));
		*olen = 0;
		ret = BCME_BUFTOOSHORT;
		goto fail;
	}
	buflen = *olen;

	ret = bcm_pack_xtlv_entry(&obuf, &buflen, WL_MBO_XTLV_AP_FWD_GAS_RQST_TO_APP,
			sizeof(mbo->fwd_gas_rqst_to_app), &mbo->fwd_gas_rqst_to_app,
			BCM_XTLV_OPTION_ALIGN32);
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: packing xtlv failed\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	*olen = *olen - buflen;

fail:
	return ret;
}

static int
wlc_mbo_iov_set_fwd_gas_rqst_to_app(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg)
{
	int ret = BCME_OK;
	uint8 data = 0;
	uint16 nbytes = 0;

	wlc_mbo_info_t *mbo = (wlc_mbo_info_t*)hndl;

	nbytes = sizeof(mbo->fwd_gas_rqst_to_app);

#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
	ret = bcm_unpack_xtlv_entry((uint8 **)&ibuf, WL_MBO_XTLV_AP_FWD_GAS_RQST_TO_APP, nbytes,
		&data, BCM_XTLV_OPTION_ALIGN32);
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: unpacking xtlv failed\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	mbo->fwd_gas_rqst_to_app = data;
fail:
	return ret;
}

static int
wlc_mbo_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg)
{
	int ret = BCME_OK;
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)ctx;
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;
	wlc_info_t *wlc;
	uint32 wnm_cap = 0;

	/* Initialize only sta */
	if (!MBO_BSSCFG_STA(cfg)) {
		return ret;
	}
	ASSERT(mbo != NULL);

	wlc = cfg->wlc;
	mbc = MBO_BSSCFG_CUBBY(mbo, cfg);

	if (mbc == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		ret = BCME_NOMEM;
		goto fail;
	}

	/* during clone re-use counters of old cfg */
	if (BSSCFG_IS_RSDB_CLONE(cfg)) {
		return BCME_OK;
	}

	/* set the initial values */
	mbc->flags = 0;
	mbc->chan_pref_list_head = NULL;
	mbc->np_chan_attr_buf = NULL;
	mbc->np_chan_attr_buf_len = 0;
	mbc->bsstrans_reject_reason = 0;
	mbc->cntrs = MALLOCZ(wlc->osh, sizeof(*mbc->cntrs));
	if (mbc->cntrs == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		ret = BCME_NOMEM;
		goto fail;
	}

	/* Enabling WNM Notification capability bit by default */
	wnm_cap = wlc_wnm_get_cap(wlc, cfg);
	wnm_cap |= WL_WNM_NOTIF;
	wlc_wnm_set_cap(wlc, cfg, wnm_cap);

	mbc->btq_trigger_offset = MBO_BTQ_TRIGGER_START_OFFSET_DFLT;
	mbc->btq_trigger_rssi_delta = MBO_BTQ_TRIGGER_RSSI_DELTA_DFLT;
	return ret;
fail:
	if (mbc) {
		wlc_mbo_bsscfg_deinit(ctx, cfg);
	}
	return ret;
}

static void
wlc_mbo_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)ctx;
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;
	wlc_info_t *wlc;

	/* deinitialize only sta */
	if (!MBO_BSSCFG_STA(cfg)) {
		return;
	}
	ASSERT(mbo != NULL);
	wlc = cfg->wlc;
	mbc = MBO_BSSCFG_CUBBY(mbo, cfg);
	ASSERT(mbc != NULL);
	if (mbc->cntrs) {
		MFREE(wlc->osh, mbc->cntrs, sizeof(*mbc->cntrs));
		mbc->cntrs = NULL;
	}
	if (mbc->np_chan_attr_buf) {
		MFREE(wlc->osh, mbc->np_chan_attr_buf, MBO_MAX_NON_PREF_CHAN_ATTRS_LEN);
		mbc->np_chan_attr_buf = NULL;
		mbc->np_chan_attr_buf_len = 0;
	}
	if (mbc->chan_pref_list_head) {
		wlc_mbo_free_chan_pref_list(mbo, cfg);
	}

	return;
}

static int
wlc_mbo_iov_add_chan_pref(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg)
{
	int ret = BCME_OK;
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)hndl;
	wlc_mbo_chan_pref_t ch_pref;

	memset(&ch_pref, 0, sizeof(ch_pref));
	ret = bcm_unpack_xtlv_buf(&ch_pref, ibuf, ilen, BCM_XTLV_OPTION_ALIGN32,
		wlc_mbo_chan_pref_cbfn);
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: unpacking xtlv buf failed %d\n",
			mbo->wlc->pub->unit, __FUNCTION__, ret));
		goto fail;
	}
	/* handle chan pref data */
	ret = wlc_mbo_add_chan_pref(mbo, bsscfg, &ch_pref);
fail:
	return ret;
}

static int
wlc_mbo_iov_del_chan_pref(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg)
{
	int ret = BCME_OK;
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)hndl;
	wlc_mbo_chan_pref_t ch_pref;

	memset(&ch_pref, 0, sizeof(ch_pref));
	ret = bcm_unpack_xtlv_buf(&ch_pref, ibuf, ilen, BCM_XTLV_OPTION_ALIGN32,
		wlc_mbo_chan_pref_cbfn);
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: unpacking xtlv buf failed %d\n",
			mbo->wlc->pub->unit, __FUNCTION__, ret));
		goto fail;
	}
	/* handle chan pref data */
	ret = wlc_mbo_del_chan_pref(mbo, bsscfg, &ch_pref);
fail:
	return ret;
}

static int
wlc_mbo_iov_list_chan_pref(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg)
{
	int ret = BCME_OK;
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)hndl;
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;
	mbo_chan_pref_list_t *cur = NULL;
	uint16 buflen = 0, count = 0, xtlv_size = 0;

	count = wlc_mbo_count_pref_chan_list_entry(mbo, bsscfg);
	xtlv_size = bcm_xtlv_size_for_data(sizeof(uint8), BCM_XTLV_OPTION_ALIGN32);
	if ((count * xtlv_size) > *olen) {
		WL_MBO_ERR(("wl%d: %s: short o/p buffer %zu, expected %u\n",
			mbo->wlc->pub->unit, __FUNCTION__, *olen, (count * xtlv_size)));
		*olen = 0;
		ret = BCME_BUFTOOSHORT;
		goto fail;
	}
	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	cur = mbc->chan_pref_list_head;
	buflen = *olen;
	while (cur) {
		xtlv_desc_t xtlv_ar[] = {
			{WL_MBO_XTLV_OPCLASS,  sizeof(uint8), &cur->opclass},
			{WL_MBO_XTLV_CHAN,  sizeof(uint8), &cur->chan},
			{WL_MBO_XTLV_PREFERENCE,  sizeof(uint8), &cur->pref},
			{WL_MBO_XTLV_REASON_CODE,  sizeof(uint8), &cur->reason},
			{0, 0, NULL}
		};
		ret = bcm_pack_xtlv_buf_from_mem(&obuf, &buflen, xtlv_ar,
			BCM_XTLV_OPTION_ALIGN32);
		if (ret != BCME_OK) {
			WL_MBO_ERR(("wl%d: %s: packing xtlvs failed\n",
				mbo->wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
		cur = cur->next;
	}
	*olen = *olen - buflen;
fail:
	return ret;
}

static int
wlc_mbo_iov_set_cellular_data_cap(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg)
{
	int ret = BCME_OK;
	uint8 data = 0;
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)hndl;

#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
	ret = bcm_unpack_xtlv_entry((uint8 **)&ibuf, WL_MBO_XTLV_CELL_DATA_CAP, sizeof(uint8),
			&data, BCM_XTLV_OPTION_ALIGN32);
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: unpacking xtlv failed\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	ret = wlc_mbo_set_cellular_data_cap(mbo, bsscfg, data);
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: set cellular data capability failed\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
fail:
	return ret;
}

static int
wlc_mbo_iov_get_cellular_data_cap(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg)
{
	int ret = BCME_OK;
	uint16 xtlv_size = 0;
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)hndl;
	uint16 buflen = 0;

	xtlv_size = bcm_xtlv_size_for_data(sizeof(mbo->mbo_data->cell_data_cap),
		BCM_XTLV_OPTION_ALIGN32);
	if (xtlv_size > *olen) {
		WL_MBO_ERR(("wl%d: %s: short buffer length %zu expected %u\n",
			mbo->wlc->pub->unit, __FUNCTION__, *olen, xtlv_size));
		*olen = 0;
		ret = BCME_BUFTOOSHORT;
		goto fail;
	}
	buflen = *olen;
	ret = bcm_pack_xtlv_entry(&obuf, &buflen, WL_MBO_XTLV_CELL_DATA_CAP,
			sizeof(mbo->mbo_data->cell_data_cap), &mbo->mbo_data->cell_data_cap,
			BCM_XTLV_OPTION_ALIGN32);
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: packing xtlv failed\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	*olen = *olen - buflen;

fail:
	return ret;
}

static int
wlc_mbo_iov_dump_counters(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg)
{
	int ret = BCME_OK;
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)hndl;
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;
	uint16 buflen = 0, xtlv_size = 0;

	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	if (mbc->cntrs) {
		xtlv_size = bcm_xtlv_size_for_data(sizeof(*mbc->cntrs), BCM_XTLV_OPTION_ALIGN32);
		if (xtlv_size > *olen) {
			WL_MBO_ERR(("wl%d: %s: short buffer length %zu expected %u\n",
				mbo->wlc->pub->unit, __FUNCTION__, *olen, xtlv_size));
			*olen = 0;
			ret = BCME_BUFTOOSHORT;
			goto fail;
		}
		buflen = *olen;
		ret = bcm_pack_xtlv_entry(&obuf, &buflen, WL_MBO_XTLV_COUNTERS,
			sizeof(*mbc->cntrs), (uint8 *)mbc->cntrs, BCM_XTLV_OPTION_ALIGN32);
		if (ret != BCME_OK) {
			WL_MBO_ERR(("wl%d: %s: packing xtlvs failed\n",
				mbo->wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
		*olen = *olen - buflen;
	} else {
		*olen = 0;
	}
fail:
	return ret;
}

static int
wlc_mbo_iov_clear_counters(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg)
{
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)hndl;
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;

	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	if (mbc->cntrs) {
		memset(mbc->cntrs, 0, sizeof(*mbc->cntrs));
	}
	return BCME_OK;
}

#ifdef WL_MBO_TB
/* "wl mbo force_assoc <>" handler */
static int
wlc_mbo_iov_set_force_assoc(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg)
{
	int ret = BCME_OK;
	uint8 data = 0;
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)hndl;

	UNUSED_PARAMETER(mbo);
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
	ret = bcm_unpack_xtlv_entry((uint8 **)&ibuf, WL_MBO_XTLV_ENABLE, sizeof(data),
			&data, BCM_XTLV_OPTION_ALIGN32);
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: unpacking xtlv failed\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	ret = wlc_mbo_set_force_assoc(mbo, bsscfg, data);
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: set force assoc failed\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
fail:
	return ret;
}

/* "wl mbo force_assoc" handler */
static int
wlc_mbo_iov_get_force_assoc(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg)
{
	int ret = BCME_OK;
	uint16 xtlv_size = 0;
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)hndl;
	uint16 buflen = 0;
	uint8 value = 0;

	xtlv_size = bcm_xtlv_size_for_data(sizeof(value),
		BCM_XTLV_OPTION_ALIGN32);
	if (xtlv_size > *olen) {
		WL_MBO_ERR(("wl%d: %s: short buffer length %zu expected %u\n",
			mbo->wlc->pub->unit, __FUNCTION__, *olen, xtlv_size));
		*olen = 0;
		ret = BCME_BUFTOOSHORT;
		goto fail;
	}
	ret = wlc_mbo_get_force_assoc(mbo, bsscfg, &value);
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: failed to get force assoc value\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		return ret;
	}
	buflen = *olen;
	ret = bcm_pack_xtlv_entry(&obuf, &buflen, WL_MBO_XTLV_ENABLE,
			sizeof(value), &value, BCM_XTLV_OPTION_ALIGN32);
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: packing xtlv failed\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	*olen = *olen - buflen;

fail:
	return ret;
}

/* "wl mbo bsstrans_reject <>" handler */
static int
wlc_mbo_iov_set_bsstrans_reject(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg)
{
	int ret = BCME_OK;
	uint8 enable = 0, reason = 0;
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)hndl;

	UNUSED_PARAMETER(mbo);
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
	ret = bcm_unpack_xtlv_entry((uint8 **)&ibuf, WL_MBO_XTLV_ENABLE, sizeof(enable),
			&enable, BCM_XTLV_OPTION_ALIGN32);
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: unpacking xtlv failed\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (enable) {
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
		ret = bcm_unpack_xtlv_entry((uint8 **)&ibuf,
			WL_MBO_XTLV_REASON_CODE, sizeof(reason),
			&reason, BCM_XTLV_OPTION_ALIGN32);
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
		if (ret != BCME_OK) {
			WL_MBO_ERR(("wl%d: %s: unpacking xtlv failed\n",
				mbo->wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
	}
	ret = wlc_mbo_set_bsstrans_reject(mbo, bsscfg, enable, reason);
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: set bsstrans reject failed\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
fail:
	return ret;
}

/* "wl mbo bsstrans_reject" handler */
static int
wlc_mbo_iov_get_bsstrans_reject(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg)
{
	int ret = BCME_OK;
	uint16 xtlv_size = 0;
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)hndl;
	uint16 buflen = 0;
	uint8 enable = 0, reason = 0;

	xtlv_size = bcm_xtlv_size_for_data(sizeof(enable),
		BCM_XTLV_OPTION_ALIGN32);
	if (enable) {
		xtlv_size = bcm_xtlv_size_for_data(sizeof(reason),
			BCM_XTLV_OPTION_ALIGN32);
	}
	if (xtlv_size > *olen) {
		WL_MBO_ERR(("wl%d: %s: short buffer length %d expected %u\n",
			mbo->wlc->pub->unit, __FUNCTION__, *olen, xtlv_size));
		*olen = 0;
		ret = BCME_BUFTOOSHORT;
		goto fail;
	}
	ret = wlc_mbo_get_bsstrans_reject(mbo, bsscfg, &enable, &reason);
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: failed to get basstrans reject values\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		return ret;
	}
	buflen = *olen;
	ret = bcm_pack_xtlv_entry(&obuf, &buflen, WL_MBO_XTLV_ENABLE,
			sizeof(enable), &enable, BCM_XTLV_OPTION_ALIGN32);
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: packing xtlv failed\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (enable) {
		ret = bcm_pack_xtlv_entry(&obuf, &buflen, WL_MBO_XTLV_REASON_CODE,
				sizeof(reason), &reason, BCM_XTLV_OPTION_ALIGN32);
		if (ret != BCME_OK) {
			WL_MBO_ERR(("wl%d: %s: packing xtlv failed\n",
				mbo->wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
	}
	*olen = *olen - buflen;

fail:
	return ret;
}
#endif /* WL_MBO_TB */

/* API to send WNM Notification req to AP */
static int
wlc_mbo_send_wnm_notif(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	uint8 sub_elem_type)
{
	int ret = BCME_OK;
	uint8 *se_buf = NULL;
	uint16 se_buf_len = 0;
	wlc_info_t *wlc = bsscfg->wlc;

	WL_MBO_DBG(("%s:Sub element type %d\n", __FUNCTION__, sub_elem_type));
	/* if joined, then send WNM Notification to AP */
	if (!WLC_BSS_CONNECTED(bsscfg) || !WLWNM_ENAB(wlc->pub)) {
		WL_MBO_ERR(("wl%d.%d: %s:Not associated or WNM disabled\n",
			wlc->pub->unit, bsscfg->_idx, __FUNCTION__));
		ret = BCME_NOTASSOCIATED;
		goto fail;
	}

	ASSERT(MBO_MAX_NON_PREF_CHAN_SE_LEN >= MBO_MAX_CELL_DATA_CAP_SE_LEN);
	/* allocate buffer for np-chan report sub-element */
	se_buf = MALLOCZ(wlc->osh, MBO_MAX_NON_PREF_CHAN_SE_LEN);
	if (se_buf == NULL) {
		WL_ERROR(("wl%d.%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, bsscfg->_idx, __FUNCTION__, MALLOCED(wlc->osh)));
		ret = BCME_NOMEM;
		goto fail;
	}
	se_buf_len = MBO_MAX_NON_PREF_CHAN_SE_LEN;
	switch (sub_elem_type) {
		case MBO_ATTR_CELL_DATA_CAP:
		{
			wlc_mbo_prep_cell_data_cap_subelement(mbo->mbo_data->cell_data_cap,
				se_buf, &se_buf_len);
			if (se_buf_len == 0) {
				ret = BCME_BADLEN;
				goto fail;
			}
#if defined(BCMDBG) || defined(WLMSG_MBO)
			prhex("cell data se", se_buf, se_buf_len);
#endif /* BCMDBG || WLMSG_MBO */
		}
		break;
		case MBO_ATTR_NON_PREF_CHAN_REPORT:
		{
			/* prepare non pref chan sub-elem buffer */
			ret = wlc_mbo_prep_non_pref_chan_report(mbo, bsscfg,
				se_buf, &se_buf_len, MBO_NON_PREF_CHAN_REPORT_SUBELEM);
			if (ret != BCME_OK) {
				WL_MBO_ERR(("wl%d.%d: %s: non pref chan report "
					"preparation failed %d\n",
					wlc->pub->unit, bsscfg->_idx, __FUNCTION__, ret));
				goto fail;
			}
#if defined(BCMDBG) || defined(WLMSG_MBO)
			prhex("np chan se:", se_buf, se_buf_len);
#endif /* BCMDBG || WLMSG_MBO */
		}
		break;
		default:
			WL_MBO_ERR(("%s:wrong sub element type %u\n", __FUNCTION__, sub_elem_type));
			goto fail;
	}
	ret = wlc_wnm_notif_req_send(wlc->wnm_info, bsscfg,
		NULL, MBO_SUBELEM_ID, se_buf, se_buf_len);
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d.%d: %s:WNM-Notif send failed %d\n",
			wlc->pub->unit, bsscfg->_idx, __FUNCTION__, ret));
		goto fail;
	}
fail:
	if (se_buf) {
		MFREE(bsscfg->wlc->osh, se_buf, MBO_MAX_NON_PREF_CHAN_SE_LEN);
		se_buf_len = 0;
	}
	return ret;
}

/* "wl mbo send_notif <>" handler */
static int
wlc_mbo_iov_send_notif(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg)
{
	int ret = BCME_OK;
	uint8 data = 0;
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)hndl;

	UNUSED_PARAMETER(mbo);
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
	ret = bcm_unpack_xtlv_entry((uint8 **)&ibuf, WL_MBO_XTLV_SUB_ELEM_TYPE, sizeof(uint8),
			&data, BCM_XTLV_OPTION_ALIGN32);
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: unpacking xtlv failed\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	ret = wlc_mbo_send_wnm_notif(mbo, bsscfg, data);
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: send WNM notification failed\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	*olen = 0;
fail:
	return ret;
}

static int
wlc_mbo_set_nbr_info_cache(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	uint8 enable, uint16 start_offset, uint8 rssi_trig_delta)
{
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;

	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	ASSERT(mbc != NULL);

	/* update nbr info config value */
	if (enable) {
		MBO_FLAG_BIT_SET(mbc->flags, MBO_FLAG_NBR_INFO_CACHE);
		if (start_offset) {
			mbc->btq_trigger_offset = start_offset;
		}
		if (rssi_trig_delta) {
			mbc->btq_trigger_rssi_delta = rssi_trig_delta;
		}
	} else {
		MBO_FLAG_BIT_RESET(mbc->flags, MBO_FLAG_NBR_INFO_CACHE);
	}
	WL_MBO_DBG(("New join BTQ trig offset %u BTQ trig RSSI delta %u\n",
		mbc->btq_trigger_offset, mbc->btq_trigger_rssi_delta));

	return BCME_OK;
}

/* "wl mbo nbr_info_cache <>" handler */
static int
wlc_mbo_iov_set_nbr_info_cache(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg)
{
	int ret = BCME_OK;
	uint8 enable = 0, rssi_trig_delta = 0, start_offset = 0;
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)hndl;
	xtlv_desc_t xtlv_ar[] = {
		{WL_MBO_XTLV_ENABLE,  sizeof(uint8), &enable},
		{WL_MBO_XTLV_BTQ_TRIG_START_OFFSET,  sizeof(uint8), &start_offset},
		{WL_MBO_XTLV_BTQ_TRIG_RSSI_DELTA,  sizeof(uint8), &rssi_trig_delta},
		{0, 0, NULL}
	};
	uint ip_len = ilen;
	UNUSED_PARAMETER(mbo);
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
	ret = bcm_unpack_xtlv_buf_to_mem((uint8 *)ibuf, (int *)&ip_len, xtlv_ar,
			BCM_XTLV_OPTION_ALIGN32);
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: unpacking xtlv failed\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	/* validate arguments */
	if (((enable) && (((start_offset != 0) &&
		((start_offset < MBO_BTQ_TRIGGER_START_OFFSET_MIN) ||
		(start_offset > MBO_BTQ_TRIGGER_START_OFFSET_MAX))) ||
		((rssi_trig_delta != 0) && ((rssi_trig_delta < MBO_BTQ_TRIGGER_RSSI_DELTA_MIN) ||
		(rssi_trig_delta > MBO_BTQ_TRIGGER_RSSI_DELTA_MAX)))))) {
		ret = BCME_BADARG;
		WL_MBO_ERR(("wl%d: %s: Bad arguments\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	ret = wlc_mbo_set_nbr_info_cache(mbo, bsscfg, enable,
		start_offset, rssi_trig_delta);
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: set nbr chan cache failed\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
fail:
	return ret;
}

/* "wl mbo nbr_info_cache" handler */
static int
wlc_mbo_iov_get_nbr_info_cache(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg)
{
	int ret = BCME_OK;
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)hndl;
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;
	uint16 buflen = 0, xtlv_size = 0;
	uint8 enable = 0, rssi_trig_delta = 0, start_offset = 0;
	xtlv_desc_t xtlv_ar[] = {
		{WL_MBO_XTLV_ENABLE,  sizeof(uint8), &enable},
		{WL_MBO_XTLV_BTQ_TRIG_START_OFFSET,  sizeof(uint8), &start_offset},
		{WL_MBO_XTLV_BTQ_TRIG_RSSI_DELTA,  sizeof(uint8), &rssi_trig_delta},
		{0, 0, NULL}
	};

	xtlv_size = (ARRAYSIZE(xtlv_ar) - 1) *
		bcm_xtlv_size_for_data(sizeof(uint8), BCM_XTLV_OPTION_ALIGN32);

	if (xtlv_size > *olen) {
		WL_MBO_ERR(("wl%d: %s: short o/p buffer %zu, expected %u\n",
			mbo->wlc->pub->unit, __FUNCTION__, *olen, xtlv_size));
		*olen = 0;
		ret = BCME_BUFTOOSHORT;
		goto fail;
	}
	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	enable = MBO_FLAG_IS_BIT_SET(mbc->flags, MBO_FLAG_NBR_INFO_CACHE);
	start_offset = mbc->btq_trigger_offset;
	rssi_trig_delta = mbc->btq_trigger_rssi_delta;

	buflen = *olen;
	ret = bcm_pack_xtlv_buf_from_mem(&obuf, &buflen, xtlv_ar,
		BCM_XTLV_OPTION_ALIGN32);
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: packing xtlvs failed\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	*olen = *olen - buflen;
fail:
	return ret;
}

static int
wlc_mbo_set_anqpo_support(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	uint8 enable, uint8 cellular)
{
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;
	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	ASSERT(mbc != NULL);

	/* update anqpo support and cellular value
	 * Note:If anqpo support is enabled, by default
	 * neighbor report id will be add to anqp offload query
	 */
	if (enable) {
		MBO_FLAG_BIT_SET(mbc->flags, MBO_FLAG_ANQPO_SUPPORT);
		if (cellular) {
			MBO_FLAG_BIT_SET(mbc->flags, MBO_FLAG_ANQP_CELL_PREF);
		}
		else if (MBO_FLAG_IS_BIT_SET(mbc->flags,
				MBO_FLAG_ANQP_CELL_PREF)) {
			MBO_FLAG_BIT_RESET(mbc->flags, MBO_FLAG_ANQP_CELL_PREF);
		}
	} else {
		MBO_FLAG_BIT_RESET(mbc->flags, MBO_FLAG_ANQPO_SUPPORT);
		MBO_FLAG_BIT_RESET(mbc->flags, MBO_FLAG_ANQP_CELL_PREF);
	}
	return BCME_OK;
}

/* "wl mbo anqpo_support <>" handler */
static int
wlc_mbo_iov_set_anqpo_support(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg)
{
	int ret = BCME_OK;
	uint8 enable = 0, cellular = 0;
	uint ip_len = ilen;
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)hndl;
#ifdef ANQPO
	wl_anqpo_info_t *anqpo = (wl_anqpo_info_t *)bsscfg->wlc->anqpo;
#endif /* ANQPO */
	xtlv_desc_t xtlv_ar[] = {
		{WL_MBO_XTLV_ENABLE,  sizeof(uint8), &enable},
		{WL_MBO_XTLV_ANQP_CELL_SUPP,  sizeof(uint8), &cellular},
		{0, 0, NULL}
	};

	if (!ANQPO_ENAB(bsscfg->wlc->pub)) {
		return BCME_UNSUPPORTED;
	}

	BCM_REFERENCE(mbo);
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
	ret = bcm_unpack_xtlv_buf_to_mem((uint8 *)ibuf, (int *)&ip_len, xtlv_ar,
		BCM_XTLV_OPTION_ALIGN32);
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: unpacking xtlv failed\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#ifdef ANQPO
	/* validate range */
	if ((wl_anqpo_auto_hotspot_enabled(anqpo)) && (!cellular)) {
		ret = BCME_BADARG;
		WL_MBO_ERR(("wl%d: %s: Bad Arguments\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* ANQPO */
	ret = wlc_mbo_set_anqpo_support(mbo, bsscfg, enable, cellular);
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: set anqpo support failed\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
fail:
	return ret;
}

/* "wl mbo anqpo_support <>" handler */
static int
wlc_mbo_iov_get_anqpo_support(void *hndl, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg)
{
	int ret = BCME_OK;
	uint16 xtlv_size = 0;
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)hndl;
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;
	uint16 buflen = 0;
	uint8 enable = 0, cellular = 0;
	xtlv_desc_t xtlv_ar[] = {
		{WL_MBO_XTLV_ENABLE,  sizeof(uint8), &enable},
		{WL_MBO_XTLV_ANQP_CELL_SUPP,  sizeof(uint8), &cellular},
		{0, 0, NULL}
	};

	xtlv_size = (ARRAYSIZE(xtlv_ar) - 1) *
		bcm_xtlv_size_for_data(sizeof(uint8), BCM_XTLV_OPTION_ALIGN32);
	if (xtlv_size > *olen) {
		WL_MBO_ERR(("wl%d: %s: short buffer length %zu expected %u\n",
			mbo->wlc->pub->unit, __FUNCTION__, *olen, xtlv_size));
		*olen = 0;
		ret = BCME_BUFTOOSHORT;
		goto fail;
	}

	/* bsscfg */
	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	if (!mbc) {
		return BCME_UNSUPPORTED;
	}
	enable = MBO_FLAG_IS_BIT_SET(mbc->flags, MBO_FLAG_ANQPO_SUPPORT);
	cellular = MBO_FLAG_IS_BIT_SET(mbc->flags, MBO_FLAG_ANQP_CELL_PREF);
	buflen = *olen;
	ret = bcm_pack_xtlv_buf_from_mem(&obuf, &buflen, xtlv_ar,
		BCM_XTLV_OPTION_ALIGN32);
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d: %s: packing xtlv failed\n",
			mbo->wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	*olen = *olen - buflen;

fail:
	return ret;
}

/* Return num of bytes require for  "Supported Operaing Classes" element. */
static uint
wlc_mbo_ie_supp_opclass_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)ctx;
	chanspec_t chanspec;
	uint8 rclen;                       /* regulatory class length */
	uint8 rclist[MAXRCLISTSIZE];       /* regulatory class list */

	ASSERT(mbo != NULL);
	ASSERT(data != NULL && data->cfg != NULL && data->cfg->target_bss);
	chanspec = data->cfg->target_bss->chanspec;
	rclen = wlc_get_regclass_list(mbo->wlc->cmi, rclist, MAXRCLISTSIZE, chanspec, TRUE);
	return TLV_HDR_LEN + rclen;
}

/* This function writes "Supported Operaing Classes" element.
 * Refer section 3.2 "Channel and Band Indication and preference"
 * of MBO Tech spec
 */
static int
wlc_mbo_ie_supp_opclass_build_fn(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)ctx;
	chanspec_t chanspec;
	uint8 rclen;                       /* regulatory class length */
	uint8 rclist[MAXRCLISTSIZE];       /* regulatory class list */

	ASSERT(data != NULL && data->cfg != NULL && data->cfg->target_bss);
	chanspec = data->cfg->target_bss->chanspec;
	rclen = wlc_get_regclass_list(mbo->wlc->cmi, rclist, MAXRCLISTSIZE, chanspec, TRUE);
	if (rclen <= data->buf_len) {
		bcm_write_tlv_safe(DOT11_MNG_REGCLASS_ID, rclist, rclen, data->buf, data->buf_len);
	} else {
		ASSERT(0);
	}

	return BCME_OK;
}

/* This function builds MBO IE with "Non-preferred Channel report Attribute"
 * and "Cellular data Capability Attribute" for (re)assoc req and probe req.
 * Refer section 3.2, 3.3, 4.2.2 and 4.2.3 of MBO Tech Spec.
 */
static int
wlc_mbo_ie_build_fn(void *ctx, wlc_mbo_oce_attr_build_data_t *data)
{
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)ctx;
	uint8 *cp = NULL;

	int total_len = 0;
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;
	wlc_bsscfg_t *bsscfg = NULL;
	wifi_mbo_cell_data_cap_attr_t *cdc_attr = NULL;

	ASSERT(mbo != NULL);
	ASSERT(data != NULL);
	bsscfg = data->cfg;
	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	ASSERT(mbc != NULL);

	if (!MBO_ENAB(mbo->wlc->pub)) {
		goto done;
	}

	if (data->buf == NULL && data->buf_len == 0) {
		switch (data->ft) {
			case FC_BEACON:
			case FC_PROBE_RESP:
			case FC_ASSOC_RESP:
			case FC_REASSOC_RESP:
			{
				/* add for MBO ap attribute indicate IE */
				total_len += sizeof(wifi_mbo_ap_cap_ind_attr_t);
				if ((mbc->mbo_assoc_disallowed >= MBO_ASSOC_DISALLOWED_REASON_UNSPECIFIED) &&
					(mbc->mbo_assoc_disallowed <=
						MBO_ASSOC_DISALLOWED_REASON_INSUFFICIENT_RSSI)) {
					/* add for MBO ap assoc disallowed attribute */
					total_len += sizeof(wifi_mbo_assoc_disallowed_attr_t);
				}
			}
			break;
			case FC_ASSOC_REQ:
			case FC_REASSOC_REQ:
			{
				if (mbc->np_chan_attr_buf_len) {
					total_len += mbc->np_chan_attr_buf_len;
				}
			}
			/* Fall through */
			case FC_PROBE_REQ:
			{
				if (mbo->mbo_data->cell_data_cap) {
					total_len += sizeof(wifi_mbo_cell_data_cap_attr_t);
				}
			}
			break;
			default:
			/* we should not be here !! check mbo_ie_build_fstbmp */
			ASSERT(0);
		}
	} else {
		ASSERT(data->buf);
		cp = data->buf;
		switch (data->ft) {
			case FC_BEACON:
			case FC_PROBE_RESP:
			case FC_ASSOC_RESP:
			case FC_REASSOC_RESP:
			{
				wifi_mbo_ap_cap_ind_attr_t *ap_attr = NULL;
				wifi_mbo_assoc_disallowed_attr_t *ap_assoc_attr = NULL;

				/* fill in MBO AP attribute */
				ap_attr = (wifi_mbo_ap_cap_ind_attr_t *)cp;
				ap_attr->id = MBO_ATTR_MBO_AP_CAPABILITY;
				ap_attr->len = sizeof(*ap_attr) - MBO_ATTR_HDR_LEN;
				ap_attr->cap_ind = mbc->mbo_ap_attr;

				cp += sizeof(*ap_attr);
				total_len += sizeof(*ap_attr);
				/* MBO standard possible values:
				 * 1: Unspecified reason
				 * 2: Max number of sta association reahed
				 * 3: Air interface is overloaded
				 * 4: Authentication server overloaded
				 * 5: Insufficient RSSI
				 * 6 - 255: Reserved
				 */
				if ((mbc->mbo_assoc_disallowed >= MBO_ASSOC_DISALLOWED_REASON_UNSPECIFIED) &&
					(mbc->mbo_assoc_disallowed <= MBO_ASSOC_DISALLOWED_REASON_INSUFFICIENT_RSSI)) {
					/* fill in MBO AP association disallowed */
					ap_assoc_attr = (wifi_mbo_assoc_disallowed_attr_t *)cp;
					ap_assoc_attr->id = MBO_ATTR_ASSOC_DISALLOWED;
					ap_assoc_attr->len = sizeof(*ap_assoc_attr) - MBO_ATTR_HDR_LEN;
					ap_assoc_attr->reason_code = mbc->mbo_assoc_disallowed;

					cp += sizeof(*ap_assoc_attr);
					total_len += sizeof(*ap_assoc_attr);
				}
			}
			break;
			case FC_ASSOC_REQ:
			case FC_REASSOC_REQ:
			{
				if (mbc->np_chan_attr_buf_len) {
					ASSERT(mbc->np_chan_attr_buf);
					/* np chan report attr(s) */
					memcpy(cp, mbc->np_chan_attr_buf,
						mbc->np_chan_attr_buf_len);
					cp += mbc->np_chan_attr_buf_len;
					total_len += mbc->np_chan_attr_buf_len;
				}
			}
			/* Fall through */
			case FC_PROBE_REQ:
			{
				if (mbo->mbo_data->cell_data_cap) {
					cdc_attr = (wifi_mbo_cell_data_cap_attr_t *)cp;
					/* fill in Cellular Data Capability Attr */
					cdc_attr->id = MBO_ATTR_CELL_DATA_CAP;
					cdc_attr->len = sizeof(*cdc_attr) - MBO_ATTR_HDR_LEN;
					cdc_attr->cell_conn = mbo->mbo_data->cell_data_cap;

					cp += sizeof(*cdc_attr);
					total_len += sizeof(*cdc_attr);
				}
			}
			break;
			default:
				/* we should not be here !! check mbo_ie_build_fstbmp */
				ASSERT(0);
		}
	}

done:
	return total_len;
}

/* From WFA MBO tech Spec v0.22: Section 3.6
 * "If an MBO AP cannot accept new associations, it shall include
 * the MBO IE with the Association Disallowed Attribute in Beacon,
 * Probe Response and (Re)Association Response frames and shall set
 * the value in the Status Code field in (Re)Association Response
 * frames to a value of seventeen (17) or thirty-one (31).
 * When an MBO STA receives a Beacon, Probe Response or
 * (Re)Association Response frame from an MBO AP containing an
 * MBO IE with the Association Disallowed Attribute, that MBO STA
 * shall not send a (Re)Association Request frame from that AP
 * without the Association Disallowed Attribute. The presence of the
 * MBO IE with the Association Disallowed Attribute in any of the Beacon,
 * Probe Response or (Re)Association Response frames shall be interpreted
 * as an indication that the MBO AP is currently not accepting associations."
 *
 * To support above requirements:
 * 1) (Re)Assoc Response: We don't process(parse IEs present in the frame)
 * Assoc Response if status is not success. Hence don't want to change this
 * behavior as we can handle this requirement via processing of scan beacon and
 * probe response without processing MBO IE in Assoc Response.
 * 2) Beacon and Probe response: MBO module will process Association Disallowed
 * Attr in beacon and probe response to set some flags. Later while pruning
 * join targets we prune the AP from join target list.
 * 3) As we go for assoc scan before initial assoc or reassoc steps taken in #2 above
 * take cares of not sending Assoc Req or Reassoc Req.
 * 4) Targeted or normal scan uses Probe Request with broadcast address for scan.
 * Hence, we dont have to take care Assoc Disallowed attr in case of normal probe request.
 * But, if we do unicast probe request  to associated AP we drop sending probe req
 * at wlc_sendprobe()
 */
static int
wlc_mbo_ie_parse_fn(void *ctx, wlc_mbo_oce_attr_parse_data_t *data)
{
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)ctx;
	wlc_bsscfg_t *bsscfg = data->cfg;
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;
	wifi_mbo_ap_cap_ind_attr_t *ap_cap_attr = NULL;
	wifi_mbo_assoc_disallowed_attr_t *assoc_dis = NULL;

	ASSERT(mbo != NULL);
	ASSERT(data != NULL);

	if (!MBO_ENAB(mbo->wlc->pub)) {
		WL_MBO_DBG(("wlc_mbo_ie_parse_fn: MBO disabled.\n"));
		goto done;
	}

	if (BSSCFG_STA(data->cfg)) {
		/* There can be one/more attribute within same MBO IE. To be safe parse */
		if (data->ie) {
			/* AP capability indication attribute */
			ap_cap_attr = (wifi_mbo_ap_cap_ind_attr_t *)
				bcm_parse_tlvs(data->ie, data->ie_len,
				MBO_ATTR_MBO_AP_CAPABILITY);

			/* Association Disallowed attribute */
			assoc_dis = (wifi_mbo_assoc_disallowed_attr_t *)
				bcm_parse_tlvs(data->ie, data->ie_len,
				MBO_ATTR_ASSOC_DISALLOWED);
		}
		mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
		ASSERT(mbc != NULL);
	}

	switch (data->ft) {
		case FC_ASSOC_REQ:
		case FC_REASSOC_REQ: {
			wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
			uint8 *ptr = NULL;
			uint8 *local = NULL;
			wlc_mbo_scb_cubby_t *mbo_scb = NULL;
			struct scb *scb =  ftpparm->assocreq.scb;
			int ret = BCME_OK;

			/* WFA MBO standard(3.2) Tech spec
			 * On (re)association an MBO STA shall indicate non-preferred channels by
			 * including zero (0) or more Non-preferred Channel Report Attributes
			 * in the (Re)Association Request frame. Every time an MBO STA informs
			 * an MBO AP of its channel and band preferences, either via the inclusion
			 * of at least one Non-preferred Channel Report Attribute in a
			 * (Re)Association frame or the inclusion of at least one Non-preferred
			 * Channel Report Subelement in a WNM-Notification Request frame, the MBO
			 * AP shall replace all (if any) previously stored information
			 * (irrespective of Operating Class) with the most current information
			 * as indicated by the MBO STA.
			 */
			mbo_scb = MBO_SCB_CUBBY(mbo, scb);
			wlc_mbo_free_np_chan_list(mbo, mbo_scb);
			local = data->ie;

			/* check for Non preferred channel report attribute */
			while ((ptr = (uint8*)bcm_parse_tlvs(local,
					(data->ie_len - MBO_OCE_IE_HDR_SIZE),
					MBO_ATTR_NON_PREF_CHAN_REPORT)) != NULL) {
				bcm_tlv_t *elt = (bcm_tlv_t*)ptr;

				ret = wlc_mbo_process_scb_np_chan_list(mbo, mbo_scb, ptr, TRUE);
				if (ret != BCME_OK) {
					return ret;
				}
				local = local + (elt->len + TLV_HDR_LEN);
			}

			/* Update SCB3 flag for MBO capable Station */
			scb->flags3 |= SCB3_MBO;
			break;
		}
		case WLC_IEM_FC_SCAN_BCN:
		case WLC_IEM_FC_SCAN_PRBRSP:
		{
			wlc_iem_ft_pparm_t *ftpparm;
			wlc_bss_info_t *bi;

			if (BSSCFG_AP(data->cfg)) {
				WL_MBO_DBG(("wlc_mbo_ie_parse_fn: Not a STA, skip to parse FC Scan BCN and ProbeResp\n"));
				goto done;
			}

			/* Pass it back in data->pparm->ft->scan.result */
			ftpparm = data->pparm->ft;
			bi = ftpparm->scan.result;

			/* As per WFA MBO Interoperability Test Plan v0.0.26, section 2.3.2
			 * STA Test Bed Requirements, the test bed STA must be able to trigger
			 * an association request even when the AP is disallowing
			 * association request.
			 */
			if (!MBO_FLAG_IS_BIT_SET(mbc->flags, MBO_FLAG_FORCE_ASSOC_TO_AP)) {
				wlc_mbo_update_assoc_disallowed(bi, assoc_dis);
			}
			wlc_mbo_update_ap_cap(bi, ap_cap_attr);
		}
		break;
		case FC_ASSOC_RESP:
		case FC_REASSOC_RESP:
		{
			if (ap_cap_attr &&
				(ap_cap_attr->cap_ind & MBO_AP_CAP_IND_CELLULAR_AWARE)) {
				mbc->flags |= MBO_FLAG_AP_CELLULAR_AWARE;
			}
		}
		break;
		case FC_BEACON:
		case FC_PROBE_RESP:
		{
			wlc_bss_info_t *bi;

			if (BSSCFG_AP(data->cfg)) {
				WL_MBO_DBG(("wlc_mbo_ie_parse_fn: Not a STA, skip to parse BCN and ProbeResp\n"));
				goto done;
			}

			bi = bsscfg->current_bss;

			/* As per WFA MBO Interoperability Test Plan v0.0.26, section 2.3.2
			 * STA Test Bed Requirements, the test bed STA must be able to trigger
			 * an association request even when the AP is disallowing
			 * association request.
			 */
			if (!MBO_FLAG_IS_BIT_SET(mbc->flags, MBO_FLAG_FORCE_ASSOC_TO_AP)) {
				wlc_mbo_update_assoc_disallowed(bi, assoc_dis);
			}
			wlc_mbo_update_ap_cap(bi, ap_cap_attr);

		}
		break;
		default:
			/* we should not be here */
			ASSERT(0);
	}
done:
	return BCME_OK;
}

/* Free up channel preference list */
static void
wlc_mbo_free_chan_pref_list(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg)
{
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;
	mbo_chan_pref_list_t *cur = NULL;
	mbo_chan_pref_list_t *temp = NULL;

	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	cur = mbc->chan_pref_list_head;
	while (cur) {
		temp = cur;
		cur = cur->next;
		MFREE(mbo->wlc->osh, temp, sizeof(*temp));
	}
	mbc->chan_pref_list_head = NULL;
}

/* Non-preferred Channel Report Attribute header.
 * Refer: section 4.2.2 of MBO Tech Spec
 * cp = buffer to be filled
 * buf_len = length of the buffer
 * len = length value to be assigined in attribute header
 */
static void
wlc_mbo_add_non_pref_chan_attr_header(uint8 *cp, uint8 buf_len, uint8 len)
{
	ASSERT(buf_len >= (MBO_ATTR_ID_LEN + MBO_ATTR_LEN_LEN));
	/* attr id */
	*cp = MBO_ATTR_NON_PREF_CHAN_REPORT;
	cp += MBO_ATTR_ID_LEN;

	/* attr len */
	*cp = len;
	cp += MBO_ATTR_LEN_LEN;
}

/* Non-preferred Channel Report subelement header.
 * Refer: section 4.4.1 of MBO Tech Spec
 * cp = buffer to be filled
 * buf_len = length of the buffer
 * len = length value to be assigined in attribute header
 */
static void
wlc_mbo_add_non_pref_chan_subelem_header(uint8 *cp, uint8 buf_len, uint8 len)
{
	ASSERT(buf_len >= (MBO_SUBELEM_ID_LEN + MBO_SUBELEM_LEN_LEN +
		WFA_OUI_LEN + MBO_ATTR_ID_LEN));
	/* subelement id */
	*cp = MBO_SUBELEM_ID;
	cp += MBO_SUBELEM_ID_LEN;

	/* subelem len */
	*cp = len;
	cp += MBO_SUBELEM_LEN_LEN;

	/* subelem oui */
	memcpy(cp, MBO_SUBELEM_OUI, WFA_OUI_LEN);
	cp += WFA_OUI_LEN;

	/* subelem oui type */
	*cp = MBO_ATTR_NON_PREF_CHAN_REPORT;
	cp += MBO_ATTR_ID_LEN;
}

/* An operating class with no channel number means preference is applicable
 * for all channels of that operating class. Operating class with channel
 * number means preference applicable to that channel only. These two type
 * of configuration is mutually exclusive i.e if any one type of configuration
 * is present the other one wont be allowed.
 */
static int
wlc_mbo_validate_chan_pref(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	wlc_mbo_chan_pref_t *ch_pref)
{
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;
	mbo_chan_pref_list_t *cur = NULL;

	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	cur = mbc->chan_pref_list_head;
	while (cur != NULL) {
		if ((cur->opclass == ch_pref->opclass) &&
			(((cur->chan == 0) && (ch_pref->chan != 0)) ||
			((cur->chan != 0) && (ch_pref->chan == 0)))) {
			WL_MBO_ERR(("wl%d.%d: %s:either all channel of band or individual channel "
				"under the band\n", mbo->wlc->pub->unit, bsscfg->_idx, __FUNCTION__));
			return BCME_BADOPTION;
		}
		cur = cur->next;
	}
	return BCME_OK;
}

/* Add chan preference to the list */
static int
wlc_mbo_add_chan_pref_to_list(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	wlc_mbo_chan_pref_t *ch_pref)
{
	wlc_info_t *wlc = mbo->wlc;
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;
	mbo_chan_pref_list_t *cur = NULL, *loc = NULL, *new = NULL, *prev = NULL;

	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	/* remove from list if already exist. Ignore return value */
	wlc_mbo_del_chan_pref_from_list(mbo, bsscfg, ch_pref);
	/* we should not cross max limit */
	if (wlc_mbo_count_pref_chan_list_entry(mbo, bsscfg) >=
			mbo->mbo_data->max_chan_pref_entries) {
		WL_MBO_ERR(("wl%d.%d: %s:Max limit reached\n",
			mbo->wlc->pub->unit, bsscfg->_idx, __FUNCTION__));
		return BCME_NORESOURCE;
	}
	/* Now, find the right place */
	cur = mbc->chan_pref_list_head;
	while (cur != NULL) {
		if ((cur->opclass == ch_pref->opclass) &&
			(cur->pref == ch_pref->pref) &&
			(cur->reason == ch_pref->reason)) {
				loc = cur;
		}
		prev = cur;
		/* go for next */
		cur = cur->next;
	}
	/* allocate */
	new = MALLOCZ(wlc->osh, sizeof(*new));
	if (new == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return BCME_NOMEM;
	}
	/* fill it up */
	new->opclass = ch_pref->opclass;
	new->chan = ch_pref->chan;
	new->pref = ch_pref->pref;
	new->reason = ch_pref->reason;
	/* link the node */
	if ((mbc->chan_pref_list_head == NULL)) {
		mbc->chan_pref_list_head = new;
	} else {
		if (loc) {
			/* add at right position */
			new->next = loc->next;
			loc->next = new;
		} else {
			ASSERT(prev != NULL);
			/* add at the end */
			prev->next = new;
		}
	}
	return BCME_OK;
}

/* delete chan preference from the list */
static int
wlc_mbo_del_chan_pref_from_list(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	wlc_mbo_chan_pref_t *ch_pref)
{
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;
	mbo_chan_pref_list_t *cur = NULL, *prev = NULL;

	ASSERT(mbo);
	ASSERT(bsscfg);
	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	cur = prev = mbc->chan_pref_list_head;

	while (cur) {
		if ((cur->opclass == ch_pref->opclass) &&
			(cur->chan == ch_pref->chan)) {
			if (cur ==  mbc->chan_pref_list_head) {
				mbc->chan_pref_list_head = cur->next;
			} else {
				prev->next = cur->next;
			}
			MFREE(mbo->wlc->osh, cur, sizeof(*cur));
			return BCME_OK;
		}
		prev = cur;
		cur = cur->next;
	}
	return BCME_NOTFOUND;
}

/* This function prepares Non-preferred Channel Report Attribute/Subelement
 * and return them in report_buf.
 * report_type: Type of the report i.e Non-preferred Channel Report Attribute
 *    or Non-preferred Channel Report Subelement
 * report_buf: Buffer where the report will be written
 * report_buf_len: in param: length of the buffer,
 *    out param: num of bytes written.
 */
static int
wlc_mbo_prep_non_pref_chan_report(wlc_mbo_info_t *mbo,
	wlc_bsscfg_t *bsscfg, uint8 *report_buf, uint16 *report_buf_len,
	uint8 report_type)
{
	int ret = BCME_OK;
	mbo_chan_pref_list_t *cur = NULL, *start = NULL, *end = NULL;
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;
	uint8 tot_len = 0;
	uint8 *cp = NULL;
	uint8 chan_list_len = 0;
	uint8 buf[MAX_NON_PREF_CHAN_BODY_LEN];
	uint8 buf_len = 0;
	uint16 cur_ent_len = 0;

	WL_MBO_DBG(("%s:Enter: Non Preferred Chan Attr buf len %u\n",
		__FUNCTION__, *report_buf_len));
	ASSERT(report_buf);

	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);

	cp = report_buf;
	cur = mbc->chan_pref_list_head;
	if (cur == NULL) {
		/* Empty list i.e entries have been deleted just now
		 * Prepare a no-body Non-preferred Channel report subelemet only
		 */
		if (report_type == MBO_NON_PREF_CHAN_REPORT_SUBELEM) {
			wlc_mbo_add_non_pref_chan_subelem_header(cp,
				MBO_MAX_NON_PREF_CHAN_SE_LEN, 4);
			*report_buf_len = MBO_SUBELEM_HDR_LEN;
		} else {
			*report_buf_len = 0;
		}
		return BCME_OK;
	}
	start = cur;
	do {
		if ((cur->next == NULL) || ((cur->next != NULL) &&
			((cur->opclass != cur->next->opclass) ||
			(cur->pref != cur->next->pref) || (cur->reason != cur->next->reason)))) {
			/* NOTE: we are stopping on non pref chan buffer len consumed */
			/* validate sufficient space for another attr */
			if (report_type == MBO_NON_PREF_CHAN_REPORT_ATTR) {
				cur_ent_len = MBO_NON_PREF_CHAN_ATTR_TOT_LEN(++chan_list_len);
			} else {
				cur_ent_len = MBO_NON_PREF_CHAN_SUBELEM_TOT_LEN(++chan_list_len);
			}
			WL_MBO_DBG(("%s:Num chan %u, Cur entry len %u left len %u\n",
				__FUNCTION__, chan_list_len, cur_ent_len,
				(*report_buf_len - tot_len)));
			if (cur_ent_len > (*report_buf_len - tot_len)) {
				ret = BCME_BUFTOOSHORT;
				break;
			}
			end = cur;
			/* create non pref chan body */
			bzero(buf, sizeof(buf));
			buf_len = MAX_NON_PREF_CHAN_BODY_LEN;
			ret = wlc_mbo_prep_non_pref_chan_report_body(start, end,
				chan_list_len, buf, &buf_len);
			if (ret != BCME_OK) {
				return ret;
			}
			if (report_type == MBO_NON_PREF_CHAN_REPORT_ATTR) {
				/* add non pref chan attr header */
				wlc_mbo_add_non_pref_chan_attr_header
					(cp, MBO_MAX_NON_PREF_CHAN_ATTRS_LEN,
					MBO_NON_PREF_CHAN_ATTR_LEN(chan_list_len));
				cp += MBO_ATTR_HDR_LEN;
				tot_len += MBO_NON_PREF_CHAN_ATTR_TOT_LEN(chan_list_len);
			} else {
				/* add non pref chan subelement header */
				wlc_mbo_add_non_pref_chan_subelem_header
					(cp, MBO_MAX_NON_PREF_CHAN_SE_LEN,
					MBO_NON_PREF_CHAN_SUBELEM_LEN_LEN(chan_list_len));
				cp += MBO_SUBELEM_HDR_LEN;
				tot_len += MBO_NON_PREF_CHAN_SUBELEM_TOT_LEN(chan_list_len);
			}
			/* append  non pref chan body */
			memcpy(cp, buf, buf_len);
			cp += buf_len;

			/* update for next report */
			start = cur->next;
			chan_list_len = 0;
		} else {
			chan_list_len++;
		}
		cur = cur->next;
	} while (cur != NULL);
	/* update num of bytes used */
	*report_buf_len = tot_len;
	WL_MBO_DBG(("%s:Exit: Non Preferred Chan Attr buf len %u\n",
		__FUNCTION__, *report_buf_len));
	return ret;
}

/* prepare non-preferred channel report body for attribute/sub-element */
static int
wlc_mbo_prep_non_pref_chan_report_body(mbo_chan_pref_list_t *start,
	mbo_chan_pref_list_t *end, uint8 chan_list_len, uint8* buf, uint8 *buf_len)
{
	mbo_chan_pref_list_t *cur = start;
	uint8 *cp = buf;

	if (*buf_len < MBO_NON_PREF_CHAN_ATTR_LEN(chan_list_len)) {
		WL_MBO_ERR(("%s: short buffer len %u expected %u \n",
			__FUNCTION__, *buf_len,
			MBO_NON_PREF_CHAN_ATTR_LEN(chan_list_len)));
		return BCME_BUFTOOSHORT;
	}
	/* op class */
	*cp = start->opclass;
	cp += MBO_NON_PREF_CHAN_ATTR_OPCALSS_LEN;

	/* chan list */
	while (cur != end) {
		*cp = cur->chan;
		cp++;
		/* move to next */
		cur = cur->next;
	}
	/* This takes care of last node and start euqals to end case */
	if (end) {
		*cp = cur->chan;
		cp++;
	}

	/* preference */
	*cp = start->pref;
	cp += MBO_NON_PREF_CHAN_ATTR_PREF_LEN;

	/* Reason Code */
	*cp = start->reason;
	cp += MBO_NON_PREF_CHAN_ATTR_REASON_LEN;

	*buf_len = MBO_NON_PREF_CHAN_ATTR_LEN(chan_list_len);
	return BCME_OK;
}

/* returns count of chan preference entry present in the list */
static uint16
wlc_mbo_count_pref_chan_list_entry(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg)
{
	mbo_chan_pref_list_t *cur = NULL;
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;
	uint16 count = 0;
	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	cur = mbc->chan_pref_list_head;
	while (cur) {
		count++;
		cur = cur->next;
	}
	return count;
}

/* bsscfg copy :get function */
static int
wlc_mbo_bsscfg_get(void *ctx, wlc_bsscfg_t *cfg, uint8 *data, int *len)
{
	if (MBO_BSSCFG_STA(cfg)) {
		wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)ctx;
		wlc_mbo_bsscfg_cubby_t *mbc = NULL;

		if (data == NULL || *len < MBO_CUBBY_CFG_SIZE) {
			WL_MBO_ERR(("wl%d.%d: %s: short buffer size %d expected %zu\n",
				mbo->wlc->pub->unit, cfg->_idx, __FUNCTION__, *len, MBO_CUBBY_CFG_SIZE));
			*len  = MBO_CUBBY_CFG_SIZE;
			return BCME_BUFTOOSHORT;
		}
		mbc = MBO_BSSCFG_CUBBY(mbo, cfg);
		if (mbc == NULL) {
			*len = 0;
			return BCME_OK;
		}
		memcpy(data, mbc, MBO_CUBBY_CFG_SIZE);
		*len = MBO_CUBBY_CFG_SIZE;
		/* reset the data pointers to NULL so that wlc_mbo_bsscfg_deinit()
		 * doesn't free up allocated data
		 */
		memset(mbc, 0, MBO_CUBBY_CFG_SIZE);
	}
	return BCME_OK;
}

/* bsscfg copy: set function */
static int
wlc_mbo_bsscfg_set(void *ctx, wlc_bsscfg_t *cfg, const uint8 *data, int len)
{
	if (MBO_BSSCFG_STA(cfg)) {
		wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)ctx;
		wlc_mbo_bsscfg_cubby_t *mbc = NULL;
		mbc = MBO_BSSCFG_CUBBY(mbo, cfg);
		if (mbc == NULL) {
			return BCME_OK;
		}
		if (data == NULL || len < MBO_CUBBY_CFG_SIZE) {
			WL_MBO_ERR(("wl%d.%d: %s:  bad argument(NULL data or bad len), "
				"len %d expected %zu\n", mbo->wlc->pub->unit, cfg->_idx, __FUNCTION__,
				len, MBO_CUBBY_CFG_SIZE));
			return BCME_BADARG;
		}
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
		memcpy(mbc, (wlc_mbo_bsscfg_cubby_t *)data, MBO_CUBBY_CFG_SIZE);
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
	}
	return BCME_OK;
}

static void
wlc_mbo_update_assoc_disallowed(wlc_bss_info_t *bi,
	wifi_mbo_assoc_disallowed_attr_t *assoc_dis)
{
	ASSERT(bi != NULL);
	/* If assoc disallwed attr is present with a valid reason */
	if (assoc_dis &&
		(assoc_dis->reason_code >= MBO_ASSOC_DISALLOWED_RC_UNSPECIFIED) &&
		(assoc_dis->reason_code <= MBO_ASSOC_DISALLOWED_RC_INSUFFIC_RSSI)) {
		bi->bcnflags |= WLC_BSS_MBO_ASSOC_DISALLOWED;
	} else if ((assoc_dis == NULL) &&
		(bi->bcnflags & WLC_BSS_MBO_ASSOC_DISALLOWED)) {
		/* if assoc disallowed attr is not present and flag is set,
		* clear
		*/
		bi->bcnflags &= ~WLC_BSS_MBO_ASSOC_DISALLOWED;
	}
}

static void
wlc_mbo_update_ap_cap(wlc_bss_info_t *bi,
	wifi_mbo_ap_cap_ind_attr_t *ap_cap_attr)
{
	/* set MBO ap capability bit */
	if (ap_cap_attr) {
		bi->bcnflags |= WLC_BSS_MBO_CAPABLE;
	} else if (ap_cap_attr == NULL && (bi->bcnflags & WLC_BSS_MBO_CAPABLE)) {
		/* if MBO ap cap attr is not present and flag is set, clear */
		bi->bcnflags &= ~WLC_BSS_MBO_CAPABLE;
	}
}

static int
wlc_mbo_chan_pref_cbfn(void *ctx, const uint8 *data, uint16 type, uint16 len)
{
	wlc_mbo_chan_pref_t *ch_pref = (wlc_mbo_chan_pref_t *)ctx;
	if (ctx == NULL || data == NULL) {
		return BCME_BADARG;
	}
	switch (type) {
		case WL_MBO_XTLV_OPCLASS:
			ch_pref->opclass = *data;
			break;
		case WL_MBO_XTLV_CHAN:
			ch_pref->chan = *data;
			break;
		case WL_MBO_XTLV_PREFERENCE:
			ch_pref->pref = *data;
			break;
		case WL_MBO_XTLV_REASON_CODE:
			ch_pref->reason = *data;
			break;
		default:
			WL_MBO_ERR(("%s: Unknown tlv type %u\n", __FUNCTION__, type));
	}
	return BCME_OK;
}
#ifdef WL_MBO_TB
/* Function to update forceful assoc attempt to AP
 * which is not accepting new connection
 */
static int
wlc_mbo_set_force_assoc(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	uint8 value)
{
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;

	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	ASSERT(mbc != NULL);

	/* update force association value */
	if (value) {
		MBO_FLAG_BIT_SET(mbc->flags, MBO_FLAG_FORCE_ASSOC_TO_AP);
	} else {
		MBO_FLAG_BIT_RESET(mbc->flags, MBO_FLAG_FORCE_ASSOC_TO_AP);
	}
	/* reset already set flag in current BSS if we are going for force attempt */
	if (value) {
		bsscfg->current_bss->bcnflags &= ~WLC_BSS_MBO_ASSOC_DISALLOWED;
	}
	return BCME_OK;
}

/* Function to get force assoc value */
static int
wlc_mbo_get_force_assoc(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	uint8 *value)
{
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;

	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	ASSERT(mbc != NULL);

	/* return force association value */
	*value = MBO_FLAG_IS_BIT_SET(mbc->flags, MBO_FLAG_FORCE_ASSOC_TO_AP);

	return BCME_OK;
}
#endif /* WL_MBO_TB */

/* Process Non preferred chan attribute or subelemnt from (Re)Association request or WNM
 * notification request frame from STA.
 */
static int
wlc_mbo_process_scb_np_chan_list(wlc_mbo_info_t* mbo, wlc_mbo_scb_cubby_t *mbo_scb, uint8* ibuf,
	bool attr)
{
	uint8 chan_list_len = 0;
	int ret = BCME_OK;

	chan_list_len = wlc_mbo_get_count_chan_from_non_pref_list(ibuf, attr);
	if (!chan_list_len) {
		/* It is possible to receive Attribute or subelement
		 * having zero chan list, skip further processing
		 */
		return BCME_OK;
	}
#ifdef BCMDBG
	/* Include ID and len of element for debug */
	prhex(" MBO Non preferred element data  ==>", ibuf, (ibuf[MBO_ATTRIBUTE_LEN_OFFSET] + 2));
#endif	/* BCMDBG */

	ret = wlc_mbo_update_scb_np_chan_list(mbo, mbo_scb, ibuf, attr, chan_list_len);
	return ret;
}

/* Returns the number of channels in list
 * Argument - attr:
 *	True for Non preferred chan report attribute
 *	False for Non preferred chan subelement in WNM Notification request frame
 */
static uint8
wlc_mbo_get_count_chan_from_non_pref_list(uint8* ibuf, bool attr)
{
	uint8 *ptr = ibuf;
	uint8 len = 0;
	uint8 empty_list_len = 0;

	ptr++;
	len = *ptr;

	if (attr) {
		/* if len == 0, No chan list if present in this Attribute
		 * return
		 */
		if (len != 0) {
			return (len - MBO_NP_ATTR_FIXED_LEN);
		}
	} else {
		/* possible values in length byte:
		 * 0x04 or variable
		 * 0x04 - No chan list provided, empty element return with 0 len
		 */
		if (ibuf[MBO_ATTRIBUTE_LEN_OFFSET] > MBO_EMPTY_SUBELEMENT_LIST_LEN) {
			return (len - MBO_NP_SUBELEMENT_FIXED_LEN);
		}
	}
	return empty_list_len;
}

static int
wlc_mbo_free_np_chan_list(wlc_mbo_info_t* mbo, wlc_mbo_scb_cubby_t *mbo_scb)
{
	mbo_np_chan_list_t *cur = NULL;
	mbo_np_chan_list_t *list = NULL;

	if (mbo_scb != NULL) {
		list = mbo_scb->np_chan_list_head;
		cur = list;
		while (cur != NULL) {
			list = cur->next;
			cur->next = NULL;
			MFREE(mbo->wlc->osh, cur, sizeof(*cur));
			cur = list;
		}
		mbo_scb->np_chan_list_head = NULL;
	}
	return BCME_OK;
}

static int
wlc_mbo_update_scb_np_chan_list(wlc_mbo_info_t* mbo, wlc_mbo_scb_cubby_t *mbo_scb, uint8 *ibuf,
	bool attr, uint8 chan_list_len)
{
	mbo_np_chan_list_t *cur, *list;
	uint8 *ptr = ibuf;
	uint8 i;

	list = (mbo_np_chan_list_t*)MALLOCZ(mbo->wlc->osh, sizeof(mbo_np_chan_list_t));
	if (!list) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			mbo->wlc->pub->unit, __FUNCTION__, MALLOCED(mbo->wlc->osh)));
		return BCME_NOMEM;
	}

	cur = mbo_scb->np_chan_list_head;

	if (!cur) {
		mbo_scb->np_chan_list_head = list;
	} else {
		while (cur->next != NULL) {
			cur = cur->next;
		}
		cur->next = list;
	}

	/*  Non preferred chan attribute report information
	 *  --------------------------------------------------------------------------------
	 *  attribute_id | length | operating class | channel list | preference | reason code
	 *   1 byte        1 byte    1 byte              variable     1 byte       1 byte
	 *  --------------------------------------------------------------------------------
	 */
	/*  Non preferred subelement information (*number of bytes)
	 *  --------------------------------------------------------------------------------
	 *    id(1) | length(1) | OUI(3)  | OUI TYPE(1) | operating class(1) | channel list(VAR)
	 *	| preference(1) | reason code(1)
	 *  --------------------------------------------------------------------------------
	 */
	ptr++;
	if (attr) {
		/* Update Opclass from Non preferred chan Attribute */
		ptr++;
		list->opclass = *ptr;
	} else {
		/* update Opclass from Non preferred chan Subelement */
		ptr = ptr + MBO_NP_SUBELEMENT_OPCLASS_OFFSET;
		list->opclass = *ptr;
	}

	ptr++;
	/* Load chan list for the corresponding opclass */
	for (i = 0; i < chan_list_len; i++) {
		list->list[i] = *ptr++;
	}

	list->pref = *ptr++;
	list->reason = *ptr;

	return BCME_OK;
}

static int
wlc_mbo_scb_init(void *ctx, struct scb *scb)
{
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)ctx;
	wlc_info_t *wlc = mbo->wlc;
	wlc_mbo_scb_cubby_t **pmbo_scb = MBO_SCB_CUBBY_LOC(mbo, scb);
	wlc_mbo_scb_cubby_t *mbo_scb;

	if ((mbo_scb = MALLOCZ(wlc->osh, sizeof(wlc_mbo_scb_cubby_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return BCME_NOMEM;
	}

	*pmbo_scb = mbo_scb;

	return BCME_OK;
}

static void
wlc_mbo_scb_deinit(void *ctx, struct scb *scb)
{
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)ctx;
	wlc_info_t *wlc = mbo->wlc;
	wlc_mbo_scb_cubby_t **pmbo_scb = MBO_SCB_CUBBY_LOC(mbo, scb);
	wlc_mbo_scb_cubby_t *mbo_scb = *pmbo_scb;

	if (mbo_scb != NULL) {
		wlc_mbo_free_np_chan_list(mbo, mbo_scb);
		MFREE(wlc->osh, mbo_scb, sizeof(wlc_mbo_scb_cubby_t));
	}
	*pmbo_scb = NULL;
}

/* Parse Non preferred chan subelement or cellular data capability subelement in
 * WNM notification request frame from STA. For every WNM notification request
 * Frame with Non preferred chan subelement free earlier scb's non pref chan list
 * and allocate new one with the contents provided in Non preferred chan subelement.
 * For Cellular data capability subelement update scb's cell capability with new
 * value provided in Cellular data subelement
 */
int
wlc_mbo_process_wnm_notif(wlc_info_t* wlc, struct scb *scb, uint8 *body, int body_len)
{
	dot11_wnm_notif_req_t *wnm_notif;
	wlc_mbo_info_t *mbo = NULL;
	wlc_mbo_scb_cubby_t *mbo_scb = NULL;
	uint8* ptr = NULL;
	uint8 bytes_rd = 0, nbytes = 0, len = 0;
	bool do_scb_chan_list_cleanup = TRUE;
	uint8 oui_type_offset = 0;
	int ret = BCME_OK;

	mbo = wlc->mbo;
	ASSERT(mbo);
	mbo_scb = MBO_SCB_CUBBY(mbo, scb);

	ASSERT(mbo_scb);

	if (body_len < DOT11_WNM_NOTIF_REQ_LEN) {
		WL_ERROR(("WNM notification request frame with invalid length\n"));
		return BCME_BADARG;
	}

	wnm_notif = (dot11_wnm_notif_req_t*)body;
	ptr = &(wnm_notif->data[MBO_ATTRIBUTE_ID_OFFSET]);
	oui_type_offset = MBO_ATTRIBUTE_OUI_OFFSET + WFA_OUI_LEN;
	/* body_len is whole notification request frame len, update body_len to
	 * have only MBO specific number of bytes
	 */
	body_len = body_len - DOT11_WNM_NOTIF_REQ_LEN;

	/* parse number of subelements if present */
	while ((body_len - bytes_rd) > MBO_WNM_NOTIFICATION_MIN_SUBELEMENT_LEN) {
		/* Confirm WFA OUI tag 0X50,0X6F,0X9A */
		if (memcmp(&ptr[MBO_ATTRIBUTE_OUI_OFFSET], WFA_OUI, WFA_OUI_LEN) != 0) {
			return BCME_IE_NOTFOUND;
		}
		if (ptr[oui_type_offset] == MBO_NP_SUBELEMENT_CHAN_OUI_TYPE) {
			/* WFA MBO standard(3.2) Tech spec
			 * Every time an MBO STA informs an MBO AP of its channel and
			 * band preferences, either via the inclusion of at least one
			 * Non-preferred Channel Report Attribute in a (Re)Association
			 * frame or the inclusion of at least one Non-preferred Channel
			 * Report Subelement in a WNM-Notification Request frame, the MBO
			 * AP shall replace all (if any) previously stored information
			 * (irrespective of Operating Class) with the most current
			 * information as indicated by the MBO STA
			 */
			if (do_scb_chan_list_cleanup) {
				wlc_mbo_free_np_chan_list(mbo, mbo_scb);
				do_scb_chan_list_cleanup = FALSE;
			}
			ret = wlc_mbo_process_scb_np_chan_list(mbo, mbo_scb, ptr, FALSE);
			if (ret != BCME_OK) {
				return ret;
			}
		}
		/* continue to look out for more subelements if any in WNM notification frame */
		len = ptr[MBO_ATTRIBUTE_LEN_OFFSET];
		nbytes = (len + MBO_WNM_SUBELEMENT_ID_AND_LEN);
		ptr += nbytes;
		bytes_rd += nbytes;
	}
	return BCME_OK;
}
/* Update 2G, 5G band capability of scb */
void
wlc_mbo_update_scb_band_cap(wlc_info_t* wlc, struct scb* scb, uint8* data)
{
	wlc_mbo_info_t* mbo = NULL;
	wlc_mbo_scb_cubby_t *mbo_scb = NULL;

	mbo = wlc->mbo;
	ASSERT(mbo);

	mbo_scb = MBO_SCB_CUBBY(mbo, scb);
	mbo_scb->flags[0] = data[0];
}

int
wlc_mbo_process_bsstrans_resp(wlc_info_t* wlc, struct scb* scb, uint8* body, int body_len)
{
	dot11_bsstrans_resp_t *rsp = NULL;
	uint16 data_len = 0;
	uint8 mbotype = WFA_OUI_TYPE_MBO;
	wifi_mbo_ie_t* mbo_ie;
	wlc_mbo_info_t *mbo = NULL;
	wifi_mbo_trans_reason_code_attr_t* reject_attr = NULL;
	wlc_mbo_scb_cubby_t *mbo_scb = NULL;
	uint8 attr_len = 0;

	mbo = wlc->mbo;
	ASSERT(mbo);
	mbo_scb = MBO_SCB_CUBBY(mbo, scb);

	ASSERT(mbo_scb);
	/* Parse and store MBO related data */
	rsp = (dot11_bsstrans_resp_t *)body;
	data_len = body_len - DOT11_BSSTRANS_RESP_LEN;
	mbo_ie = (wifi_mbo_ie_t *) bcm_find_vendor_ie(rsp->data, data_len,
		MBO_OUI, &mbotype, 1);

	if (!mbo_ie) {
		return BCME_NOTFOUND;
	}
	if (mbo_ie && (mbo_ie->len > (MBO_OUI_LEN + sizeof(mbo_ie->oui_type)))) {
		attr_len = mbo_ie->len - (MBO_OUI_LEN + sizeof(mbo_ie->oui_type));

		/* Transition rejection reason Attribute */
		reject_attr = (wifi_mbo_trans_reason_code_attr_t *)
			bcm_parse_tlvs(mbo_ie->attr, attr_len,
			MBO_ATTR_TRANS_REJ_REASON_CODE);

		if (reject_attr) {
			mbo_scb->bsstrans_reject_reason = reject_attr->trans_reason_code;
		}
	}
	return BCME_OK;
}

int
wlc_mbo_calc_len_mbo_ie_bsstrans_req(uint8 reqmode, bool* assoc_retry_attr)
{
	uint8 len = 0;

	len += MBO_OCE_IE_HDR_SIZE;
	/* add transition reassoc code attr */
	len += sizeof(wifi_mbo_trans_reason_code_attr_t);

	if (reqmode & DOT11_BSSTRANS_REQMODE_DISASSOC_IMMINENT) {
		*assoc_retry_attr = TRUE;
		len += sizeof(wifi_mbo_assoc_retry_delay_attr_t);
	}
	return len;
}

void
wlc_mbo_add_mbo_ie_bsstrans_req(wlc_info_t* wlc, uint8* data, bool assoc_retry_attr,
	uint8 retry_delay, uint8 transition_reason)
{
	uint8 *cp = NULL;
	wifi_mbo_oce_ie_t *ie_hdr = NULL;
	uint total_len = 0; /* len to be put in IE header */
	int len = 0;
	wifi_mbo_trans_reason_code_attr_t* trans_rc_attr = NULL;
	wifi_mbo_assoc_retry_delay_attr_t* retry_delay_attr = NULL;

	cp = data;
	ie_hdr = (wifi_mbo_oce_ie_t *)cp;

	/* fill in MBO-OCE IE header */
	ie_hdr->id = MBO_OCE_IE_ID;
	memcpy(ie_hdr->oui, MBO_OCE_OUI, WFA_OUI_LEN);
	ie_hdr->oui_type = MBO_OCE_OUI_TYPE;
	len = MBO_OCE_IE_HDR_SIZE;
	cp += len;
	total_len = MBO_OCE_IE_NO_ATTR_LEN;

	/* fill transition reason code */
	trans_rc_attr = (wifi_mbo_trans_reason_code_attr_t *)cp;
	trans_rc_attr->id = MBO_ATTR_TRANS_REASON_CODE;
	trans_rc_attr->len = sizeof(*trans_rc_attr) - MBO_ATTR_HDR_LEN;
	trans_rc_attr->trans_reason_code = transition_reason;

	cp += sizeof(*trans_rc_attr);
	total_len += sizeof(*trans_rc_attr);

	if (assoc_retry_attr) {
		/* fill Association retry delay attribute */
		retry_delay_attr = (wifi_mbo_assoc_retry_delay_attr_t *)cp;
		retry_delay_attr->id = MBO_ATTR_ASSOC_RETRY_DELAY;
		retry_delay_attr->len = sizeof(*retry_delay_attr) - MBO_ATTR_HDR_LEN;
		retry_delay_attr->reassoc_delay = retry_delay;

		cp += sizeof(*retry_delay_attr);
		total_len += sizeof(*retry_delay_attr);
	}

	ie_hdr->len = total_len;
}

bool
wlc_mbo_reject_assoc_req(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	wlc_mbo_info_t *mbo = NULL;
	wlc_mbo_bsscfg_cubby_t *mbo_bsscfg = NULL;

	mbo = wlc->mbo;
	ASSERT(mbo);

	mbo_bsscfg = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	ASSERT(mbo_bsscfg);

	if ((mbo_bsscfg->mbo_assoc_disallowed >= MBO_ASSOC_DISALLOWED_REASON_UNSPECIFIED) &&
		(mbo_bsscfg->mbo_assoc_disallowed <=
			MBO_ASSOC_DISALLOWED_REASON_INSUFFICIENT_RSSI)) {

		return TRUE;
	}
	return FALSE;
}

bool
wlc_mbo_is_channel_non_preferred(wlc_info_t* wlc, struct scb* scb, uint8 channel, uint8 opclass)
{
	wlc_mbo_info_t *mbo = NULL;
	wlc_mbo_scb_cubby_t *mbo_scb = NULL;
	mbo_np_chan_list_t *cur;
	uint8 list_cnt;
	bool ret = FALSE;

	mbo = wlc->mbo;
	ASSERT(mbo);
	if (!scb) {
		/* No need to check */
		return ret;
	}
	mbo_scb = MBO_SCB_CUBBY(mbo, scb);

	if (!(mbo_scb->np_chan_list_head)) {
		return ret;
	}

	cur = mbo_scb->np_chan_list_head;
	while (cur) {
		if (cur->opclass != opclass) {
			cur = cur->next;
			continue;
		}
		for (list_cnt = 0; list_cnt < sizeof(cur->list); list_cnt++) {
			if (cur->list[list_cnt] == channel) {
				/* possible preference:
				 * 0: Non operable
				 * 1: Non preferable
				 * 2-254: Reserved
				 * 255: preferable
				 */
				if (cur->pref == MBO_STA_MARKED_CHANNEL_PREFERABLE) {
					/* Mark this as preferred */
					return FALSE;
				} else {
					return TRUE;
				}
			}
		}
		cur = cur->next;
	}
	return ret;
}

int32
wlc_mbo_get_gas_support(wlc_info_t* wlc)
{
	wlc_mbo_info_t *mbo = NULL;

	mbo = wlc->mbo;
	ASSERT(mbo);
	return (int32)(mbo->fwd_gas_rqst_to_app);
}

/* API to add channel preference for MBO */
int
wlc_mbo_add_chan_pref(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	wlc_mbo_chan_pref_t *ch_pref)
{
	int ret = BCME_OK;
	wlc_info_t *wlc = mbo->wlc;
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;

	/* add validation check */
	ret = wlc_mbo_validate_chan_pref(mbo, bsscfg, ch_pref);
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d.%d: %s: chan pref data validation failed %d\n",
			wlc->pub->unit, bsscfg->_idx, __FUNCTION__, ret));
		goto fail;
	}
	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	/* add to the ordered list */
	ret = wlc_mbo_add_chan_pref_to_list(mbo, bsscfg, ch_pref);
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d.%d: %s: non pref chan add failed %d\n",
			wlc->pub->unit, bsscfg->_idx, __FUNCTION__, ret));
		goto fail;
	}
	/* allocate buffer for pre-built non-preferred chan report attr */
	if (mbc->np_chan_attr_buf == NULL) {
		mbc->np_chan_attr_buf = MALLOCZ(wlc->osh, MBO_MAX_NON_PREF_CHAN_ATTRS_LEN);
		if (mbc->np_chan_attr_buf == NULL) {
			WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
				wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
			ret = BCME_NOMEM;
			goto fail;
		}
	}
	/* reset old report */
	bzero(mbc->np_chan_attr_buf, MBO_MAX_NON_PREF_CHAN_ATTRS_LEN);
	mbc->np_chan_attr_buf_len = MBO_MAX_NON_PREF_CHAN_ATTRS_LEN;
	/* update pre-built non preferred chan report */
	ret = wlc_mbo_prep_non_pref_chan_report(mbo, bsscfg,
		mbc->np_chan_attr_buf, &mbc->np_chan_attr_buf_len,
		MBO_NON_PREF_CHAN_REPORT_ATTR);
	if (ret != BCME_OK) {
		WL_MBO_ERR(("wl%d.%d: %s: non pref chan report preparation failed %d\n",
			wlc->pub->unit, bsscfg->_idx, __FUNCTION__, ret));
		goto fail;
	}
#if defined(BCMDBG) || defined(WLMSG_MBO)
	prhex("np chan attr:", mbc->np_chan_attr_buf, mbc->np_chan_attr_buf_len);
#endif /* BCMDBG || WLMSG_MBO */
	return BCME_OK;
fail:
	/* no need to free mbc->np_chan_attr_buf on fail case. will be free bsscfg_deinit
	 * or all channel preference deletion case
	 */
	return ret;
}

/* API to delete chan/band preference from list */
int
wlc_mbo_del_chan_pref(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	wlc_mbo_chan_pref_t *ch_pref)
{
	int ret = BCME_OK;
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;

	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	/* delete from list */
	if (mbc->chan_pref_list_head) {
		ret = wlc_mbo_del_chan_pref_from_list(mbo, bsscfg, ch_pref);
		if (ret != BCME_OK) {
			WL_MBO_ERR(("wl%d.%d:%s:pref chan del failed %d\n",
				mbo->wlc->pub->unit, bsscfg->_idx, __FUNCTION__, ret));
			goto fail;
		}
		/* update pre-built non preferred chan report */
		mbc->np_chan_attr_buf_len = MBO_MAX_NON_PREF_CHAN_ATTRS_LEN;
		ret = wlc_mbo_prep_non_pref_chan_report(mbo, bsscfg,
			mbc->np_chan_attr_buf, &mbc->np_chan_attr_buf_len,
			MBO_ATTR_NON_PREF_CHAN_REPORT);
		if (ret != BCME_OK) {
			WL_MBO_ERR(("wl%d.%d: %s: non pref chan attr report updation failed %d\n",
				mbo->wlc->pub->unit, bsscfg->_idx, __FUNCTION__, ret));
			goto fail;
		}
	}
fail:
	if (mbc->chan_pref_list_head == NULL) {
		/* entries are not there, free up pre-built buffer for np-chan report if-any */
		if (mbc->np_chan_attr_buf) {
			MFREE(mbo->wlc->osh, mbc->np_chan_attr_buf,
				MBO_MAX_NON_PREF_CHAN_ATTRS_LEN);
			mbc->np_chan_attr_buf_len = 0;
		}
	}
	return ret;
}

/* Build "Cellular Capabilities Subelement".
 * Refer section 4.4.2 of MBO Tech spec
 */
static void
wlc_mbo_prep_cell_data_cap_subelement(uint8 cap, uint8 *buf, uint16 *buf_len)
{
	wifi_mbo_cell_cap_subelem_t *se = NULL;
	if (*buf_len != MBO_MAX_CELL_DATA_CAP_SE_LEN) {
		WL_MBO_ERR(("%s: bad buf len %u\n",
			__FUNCTION__, *buf_len));
		*buf_len = 0;
		return;
	}
	se = (wifi_mbo_cell_cap_subelem_t *)buf;
	se->sub_elem_id = MBO_SUBELEM_ID;
	se->len = sizeof(*se) - (MBO_SUBELEM_ID_LEN + MBO_SUBELEM_LEN_LEN);
	/* subelem oui */
	memcpy(se->oui, MBO_SUBELEM_OUI, WFA_OUI_LEN);
	se->oui_type = MBO_ATTR_CELL_DATA_CAP;
	se->cell_conn = cap;
}

/* API to set cellular data capability */
int
wlc_mbo_set_cellular_data_cap(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	uint8 cell_data_cap)
{
	uint8 se_buf[MBO_MAX_CELL_DATA_CAP_SE_LEN];
	uint16 se_buf_len = MBO_MAX_CELL_DATA_CAP_SE_LEN;

	/* set cellular data capability */
	mbo->mbo_data->cell_data_cap = cell_data_cap;

	wlc_mbo_prep_cell_data_cap_subelement(cell_data_cap, se_buf, &se_buf_len);
	if (se_buf_len != MBO_MAX_CELL_DATA_CAP_SE_LEN) {
		return BCME_BADLEN;
	}
#ifdef BCMDBG
	prhex("cell data se", se_buf, se_buf_len);
#endif /* BCMDBG */
#ifdef WLWNM
	/* if joined, send WNM Notification to AP */
	if (WLC_BSS_CONNECTED(bsscfg) && WLWNM_ENAB(mbo->wlc->pub)) {
		int ret = wlc_wnm_notif_req_send(mbo->wlc->wnm_info, bsscfg,
			NULL, MBO_SUBELEM_ID, se_buf, se_buf_len);
		if (ret != BCME_OK) {
			WL_MBO_ERR(("wl%d.%d: %s:WNM-Notif send failed %d\n",
				mbo->wlc->pub->unit, bsscfg->_idx, __FUNCTION__, ret));
			return ret;
		}
	}
#endif /* WLWNM */
	return BCME_OK;
}

/* API to get cellular data capability */
int
wlc_mbo_get_cellular_data_cap(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	uint8 *cell_data_cap)
{
	/* set cellular data capability */
	*cell_data_cap = mbo->mbo_data->cell_data_cap;
	return BCME_OK;
}

uint8
wlc_is_mbo_association(wlc_bsscfg_t *cfg)
{
	wlc_bss_info_t *bi = cfg->current_bss;
	if (!BSSCFG_INFRA_STA(cfg) && !WLC_BSS_CONNECTED(cfg)) {
		return FALSE;
	}
	return (bi->bcnflags & WLC_BSS_MBO_CAPABLE);
}

int
wlc_mbo_handle_rssi_variation(wlc_bsscfg_t *cfg, int roam_trigger, int rssi)
{
	int ret = BCME_OK;
	wlc_mbo_bsscfg_cubby_t *mbc;
	wlc_info_t *wlc = cfg->wlc;

	if (!BSSCFG_INFRA_STA(cfg)) {
		return BCME_OK;
	}

	mbc = MBO_BSSCFG_CUBBY(wlc->mbo, cfg);
	/* NBR info caching not enabled */
	if (mbc && !MBO_FLAG_IS_BIT_SET(mbc->flags, MBO_FLAG_NBR_INFO_CACHE)) {
		return BCME_OK;
	}
	WL_MBO_DBG(("%s: Prev roam trig %d roam trig %d RSSI %d dela %u\n",
		__FUNCTION__, mbc->prev_roam_trigger, roam_trigger, rssi,
		mbc->btq_trigger_rssi_delta));
	if ((mbc->prev_roam_trigger != roam_trigger) && (rssi > roam_trigger) &&
		((rssi - roam_trigger) <= mbc->btq_trigger_rssi_delta)) {
		mbc->prev_roam_trigger = roam_trigger;
		WL_MBO_INFO(("%s: Triggering BTM query\n", __FUNCTION__));
		ret = wlc_wnm_bsstrans_query_send(wlc->wnm_info,
			cfg, NULL, DOT11_BSSTRANS_REASON_BETTER_AP_FOUND);
		if (ret != BCME_OK) {
			WL_MBO_ERR(("%s: failed to send BTM query: %d\n",
				__FUNCTION__, ret));
		}
	}
	return ret;
}

void
wlc_mbo_update_btq_info(wlc_mbo_info_t *mbo, wlc_bsscfg_t *cfg,
	uint8 token, uint8 reason)
{
	wlc_mbo_bsscfg_cubby_t *mbc = MBO_BSSCFG_CUBBY(mbo, cfg);

	if (MBO_FLAG_IS_BIT_SET(mbc->flags, MBO_FLAG_NBR_INFO_CACHE)) {
		mbc->btq_token = token;
		mbc->btq_reason = reason;
	}
}

#ifdef WL_MBO_TB
int
wlc_mbo_set_bsstrans_reject(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	uint8 enable, uint8 reason)
{
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;

	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	ASSERT(mbc != NULL);

	/* update bsstrans reject and reason value */
	if (enable) {
		MBO_FLAG_BIT_SET(mbc->flags, MBO_FLAG_FORCE_BSSTRANS_REJECT);
	} else {
		MBO_FLAG_BIT_RESET(mbc->flags, MBO_FLAG_FORCE_BSSTRANS_REJECT);
	}
	mbc->bsstrans_reject_reason = reason;

	return BCME_OK;
}

int
wlc_mbo_get_bsstrans_reject(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	uint8 *enable, uint8 *reason)
{
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;

	mbc = MBO_BSSCFG_CUBBY(mbo, bsscfg);
	ASSERT(mbc != NULL);

	/* return bsstrans reject and reason value */
	*enable = MBO_FLAG_IS_BIT_SET(mbc->flags, MBO_FLAG_FORCE_BSSTRANS_REJECT);
	*reason = mbc->bsstrans_reject_reason;

	return BCME_OK;
}
#endif /* WL_MBO_TB */

#ifdef ANQPO
static int
wlc_mbo_write_cell_con_pref_query_elem(wlc_bsscfg_t *cfg, uint8 *cp,
	uint16 *mbo_buf_len)
{
	wifi_mbo_anqp_elem_t *anqp_ie = NULL;

	if (*mbo_buf_len < MBO_ANQP_ELEM_HDR_SIZE) {
		return BCME_BADLEN;
	}
	anqp_ie = (wifi_mbo_anqp_elem_t *)cp;

	/* fill in anqp IE header */
	anqp_ie->info_id = ANQP_ID_VENDOR_SPECIFIC_LIST;
	memcpy(anqp_ie->oui, WFA_OUI, WFA_OUI_LEN);
	anqp_ie->oui_type = MBO_ANQP_OUI_TYPE;
	anqp_ie->sub_type = MBO_ANQP_ELEM_CELL_DATA_CONN_PREF;

	/* update MBO ANQP elem len */
	anqp_ie->len = MBO_ANQP_ELEM_NO_PAYLOAD_LEN;

	return BCME_OK;
}

/* Parse MBO ANQP elements to anqp offload query
 * query: General query data set from host
 * query_len: General query set from host through anqpo_set IOVAR command
 * cellular_aware: MBO AP cellular aware bit flag use for hotspot scan
 */
uint
wlc_mbo_calc_anqp_elem_len(wlc_bsscfg_t *cfg, uint8 *query, uint16 query_len,
	uint8 cellular_aware)
{
	uint16 total_len = 0;
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)cfg->wlc->mbo;
	wl_anqpo_info_t *anqpo = (wl_anqpo_info_t *)cfg->wlc->anqpo;
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;

	if (!ANQPO_ENAB(cfg->wlc->pub)) {
		return total_len;
	}
	/* bsscfg */
	mbc = MBO_BSSCFG_CUBBY(mbo, cfg);
	if (!mbc) {
		return total_len;
	}

	/* check MBO IOVAR ANQPO support is enabled */
	if (MBO_FLAG_IS_BIT_SET(mbc->flags, MBO_FLAG_ANQPO_SUPPORT)) {
		total_len = ANQP_INFORMATION_ID_LEN;
		if (((wl_anqpo_auto_hotspot_enabled(anqpo)) && (cellular_aware)) ||
			((!wl_anqpo_auto_hotspot_enabled(anqpo)) &&
			(MBO_FLAG_IS_BIT_SET(mbc->flags, MBO_FLAG_ANQP_CELL_PREF) != 0))) {
			/* calculate total len of the MBO ANQP elem */
			total_len += MBO_ANQP_ELEM_HDR_SIZE;
		}
	} else {
		WL_MBO_INFO(("%s: MBO FLAG for ANQPO support IOVAR is not enabled\n",
			__FUNCTION__));
	}
	return total_len;
}

/* MBO module prepare the MBO ANQP elements(based on the MBO
 * IOVAR configuration) to pass ANQP Offload Query
 * query: General query data set from host
 * query_len: General query set from host through anqpo_set IOVAR command
 * cellular_aware: MBO AP cellular aware bit flag use for hotspot scan
 * total_len: General anqpo query len + MBO ANQP elements length
 */
int
wlc_mbo_build_anqp_elem(wlc_bsscfg_t *cfg, uint8 *query, uint16 *query_len,
	uint8 cellular_aware, uint16 total_buf_len)
{
	int ret = BCME_OK;
	wlc_mbo_info_t *mbo = (wlc_mbo_info_t *)cfg->wlc->mbo;
	wl_anqpo_info_t *anqpo = (wl_anqpo_info_t *)cfg->wlc->anqpo;
	wlc_mbo_bsscfg_cubby_t *mbc = NULL;
	uint8 *cp = NULL;
	uint16 mbo_buf_len = 0, anqp_nbr_id;
	uint16 anqp_query_len = 0;
	uint16 anqp_query_id = ltoh16_ua(query);

	if (!ANQPO_ENAB(cfg->wlc->pub)) {
		return BCME_UNSUPPORTED;
	}
	anqp_nbr_id = ANQP_ID_NEIGHBOR_REPORT;

	/* check ANQP ID query list is there in the anqpo query (or)
	 * query len is less than total buf len
	 */
	if ((anqp_query_id != ANQP_ID_QUERY_LIST) || (*query_len > total_buf_len)) {
		return BCME_BADARG;
	}
	/* bsscfg */
	mbc = MBO_BSSCFG_CUBBY(mbo, cfg);
	if (!mbc) {
		return ret;
	}

	cp = query + *query_len;
	if (MBO_FLAG_IS_BIT_SET(mbc->flags, MBO_FLAG_ANQPO_SUPPORT)) {
		mbo_buf_len = total_buf_len - *query_len;
		if (mbo_buf_len >= ANQP_INFORMATION_ID_LEN) {
			memcpy(cp, &anqp_nbr_id, ANQP_INFORMATION_ID_LEN);
			cp += ANQP_INFORMATION_ID_LEN;
			mbo_buf_len -= ANQP_INFORMATION_ID_LEN;

			/* Updating ANQP INFO len */
			anqp_query_len = ltoh16_ua(query + ANQP_INFORMATION_ID_LEN);
			anqp_query_len += ANQP_INFORMATION_ID_LEN;
			htol16_ua_store(anqp_query_len, (query + ANQP_INFORMATION_ID_LEN));
		} else {
			WL_MBO_ERR(("%s: short buffer length:%u\n",
				__FUNCTION__, mbo_buf_len));
			return BCME_BADLEN;
		}

		/* check SCAN (or) HOST initiated is enabled, if enabled then
		 * add MBO ANQP elements
		 */
		if ((wl_anqpo_auto_hotspot_enabled(anqpo) && (cellular_aware)) ||
			((!wl_anqpo_auto_hotspot_enabled(anqpo)) &&
			(MBO_FLAG_IS_BIT_SET(mbc->flags, MBO_FLAG_ANQP_CELL_PREF)))) {
				ret = wlc_mbo_write_cell_con_pref_query_elem(cfg, cp, &mbo_buf_len);
			if (ret != BCME_OK) {
				WL_MBO_ERR(("%s: short buffer length:%u\n",
					__FUNCTION__, mbo_buf_len));
				return ret;
			}
			*query_len += (mbo_buf_len + ANQP_INFORMATION_ID_LEN);
			return ret;
		}
		*query_len += ANQP_INFORMATION_ID_LEN;
	} else {
		WL_MBO_INFO(("%s: Parse general query, MBO fields are not enabled",
			__FUNCTION__));
	}
	return ret;
}
#endif /* ANQPO */
