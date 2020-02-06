/*
 * MBO declarations/definitions for
 * Broadcom 802.11abgn Networking Device Driver
 *
 * $Copyright (C) 2016 Broadcom Corporation$
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

#include <wlc_types.h>

#define WL_MBO_CNT_INR(_m, _ctr) (++((_m)->cntrs->_ctr))
#define MBO_MAX_CHAN_PREF_ENTRIES 16

/* flags to mark MBO ap capability */
#define MBO_FLAG_AP_CELLULAR_AWARE  0x1
/* flag to association attempt even AP is not accepting connection */
#define MBO_FLAG_FORCE_ASSOC_TO_AP  0x2
/* flag to forcefully reject bss transition request from AP */
#define MBO_FLAG_FORCE_BSSTRANS_REJECT  0x4
/* flag to enable/disble nbr info caching */
#define MBO_FLAG_NBR_INFO_CACHE  0x8

/* flag to anqpo support request from host */
#define MBO_FLAG_ANQPO_SUPPORT  0x8
/* flag to cellular pref to support anqpo */
#define MBO_FLAG_ANQP_CELL_PREF 0x10

/* Forward declarations */
typedef uint16 bcm_iov_cmd_id_t;
typedef uint16 bcm_iov_cmd_flags_t;
typedef uint16 bcm_iov_cmd_mflags_t;
typedef struct bcm_iov_cmd_info bcm_iov_cmd_info_t;
typedef struct bcm_iov_buf bcm_iov_buf_t;
typedef struct bcm_iov_batch_buf bcm_iov_batch_buf_t;

typedef struct wlc_mbo_chan_pref {
	uint8 opclass;
	uint8 chan;
	uint8 pref;
	uint8 reason;
} wlc_mbo_chan_pref_t;

/*
 * iov validation handler - All the common checks that are required
 * for processing of iovars for any given command.
 */
typedef int (*bcm_iov_cmd_validate_t)(void *ptr,
	uint32 actionid, const uint8* ibuf, size_t ilen, uint8 *obuf, size_t *olen);

/* iov get handler - process subcommand specific input and return output.
 * input and output may overlap, so the callee needs to check if
 * that is supported. For xtlv data a tlv digest is provided to make
 * parsing simpler. Output tlvs may be packed into output buffer using
 * bcm xtlv support. olen is input/output parameter. On input contains
 * max available obuf length and callee must fill the correct length
 * to represent the length of output returned.
 */
typedef int (*bcm_iov_cmd_get_t)(void *ptr,
	const uint8* ibuf, size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg);

/* iov set handler - process subcommand specific input and return output
 * input and output may overlap, so the callee needs to check if
 * that is supported. olen is input/output parameter. On input contains
 * max available obuf length and callee must fill the correct length
 * to represent the length of output returned.
 */
typedef int (*bcm_iov_cmd_set_t)(void *ptr,
	const uint8* ibuf, size_t ilen, uint8 *obuf, size_t *olen, wlc_bsscfg_t *bsscfg);

/*
 * Batched commands will have the following memory layout
 * +--------+---------+--------+-------+
 * |version |count    | is_set |sub-cmd|
 * +--------+---------+--------+-------+
 * version >= 0x8000
 * count = number of sub-commands encoded in the iov buf
 * sub-cmd one or more sub-commands for processing
 * Where sub-cmd is padded byte buffer with memory layout as follows
 * +--------+---------+-----------------------+-------------+------
 * |cmd-id  |length   |IN(options) OUT(status)|command data |......
 * +--------+---------+-----------------------+-------------+------
 * cmd-id =sub-command ID
 * length = length of this sub-command
 * IN(options) = On input processing options/flags for this command
 * OUT(status) on output processing status for this command
 * command data = encapsulated IOVAR data as a single structure or packed TLVs for each
 * individual sub-command.
 */
struct bcm_iov_batch_subcmd {
	uint16 id;
	uint16 len;
	union {
		uint32 options;
		uint32 status;
	} u;
	uint8 data[1];
};

struct bcm_iov_batch_buf {
	uint16 version;
	uint8 count;
	uint8 is_set; /* to differentiate set or get */
	struct bcm_iov_batch_subcmd cmds[0];
};
/* information about the command, xtlv options and xtlvs_off are meaningful
 * only if XTLV_DATA cmd flag is selected
 */
struct bcm_iov_cmd_info {
	bcm_iov_cmd_id_t	cmd;		/* the (sub)command - module specific */
	bcm_iov_cmd_flags_t	flags;		/* checked by bcmiov but set by module */
	bcm_iov_cmd_mflags_t	mflags;		/* owned and checked by module */
	bcm_xtlv_opts_t		xtlv_opts;
	bcm_iov_cmd_validate_t	validate_h;	/* command validation handler */
	bcm_iov_cmd_get_t	get_h;
	bcm_iov_cmd_set_t	set_h;
	uint16			xtlvs_off;	/* offset to beginning of xtlvs in cmd data */
	uint16			min_len_set;
	uint16			max_len_set;
	uint16			min_len_get;
	uint16			max_len_get;
};

/* non-batched command version = major|minor w/ major <= 127 */
struct bcm_iov_buf {
	uint16 version;
	uint16 len;
	bcm_iov_cmd_id_t id;
	uint16 data[1]; /* 32 bit alignment may be repurposed by the command */
	/* command specific data follows */
};

/* iov options flags */
enum {
	BCM_IOV_CMD_OPT_ALIGN_NONE = 0x0000,
	BCM_IOV_CMD_OPT_ALIGN32 = 0x0001,
	BCM_IOV_CMD_OPT_TERMINATE_SUB_CMDS = 0x0002
};

/* iov command flags */
enum {
	BCM_IOV_CMD_FLAG_NONE = 0,
	BCM_IOV_CMD_FLAG_STATUS_PRESENT = (1 << 0), /* status present at data start - output only */
	BCM_IOV_CMD_FLAG_XTLV_DATA = (1 << 1),  /* data is a set of xtlvs */
	BCM_IOV_CMD_FLAG_HDR_IN_LEN = (1 << 2), /* length starts at version - non-bacthed only */
	BCM_IOV_CMD_FLAG_NOPAD = (1 << 3) /* No padding needed after iov_buf */
};
wlc_mbo_info_t * wlc_mbo_attach(wlc_info_t *wlc);
void wlc_mbo_detach(wlc_mbo_info_t *mbo);
#ifdef WL_MBO_OCE
wlc_mbo_oce_info_t *wlc_init_mbo_oce(wlc_info_t* wlc);
#endif /* WL_MBO_OCE */
int wlc_mbo_process_wnm_notif(wlc_info_t *wlc, struct scb *scb, uint8 *body, int body_len);
void wlc_mbo_update_scb_band_cap(wlc_info_t* wlc, struct scb* scb, uint8* data);
int wlc_mbo_process_bsstrans_resp(wlc_info_t* wlc, struct scb* scb, uint8* body, int body_len);
void wlc_mbo_add_mbo_ie_bsstrans_req(wlc_info_t* wlc, uint8* data, bool assoc_retry_attr,
	uint8 retry_delay, uint8 transition_reason);
int wlc_mbo_calc_len_mbo_ie_bsstrans_req(uint8 reqmode, bool* assoc_retry_attr);
bool wlc_mbo_reject_assoc_req(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg);
bool wlc_mbo_is_channel_non_preferred(wlc_info_t* wlc, struct scb* scb, uint8 channel,
	uint8 opclass);
int32 wlc_mbo_get_gas_support(wlc_info_t* wlc);
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
uint8
wlc_is_mbo_association(wlc_bsscfg_t *cfg);
int
wlc_mbo_handle_rssi_variation(wlc_bsscfg_t *cfg, int prev_rssi, int rssi);
void
wlc_mbo_update_btq_info(wlc_mbo_info_t *mbo, wlc_bsscfg_t *cfg,
	uint8 token, uint8 reason);
#ifdef WL_MBO_TB
int
wlc_mbo_set_bsstrans_reject(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	uint8 enable, uint8 reason);
int
wlc_mbo_get_bsstrans_reject(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	uint8 *enable, uint8 *reason);
#endif /* WL_MBO_TB */

int
wlc_mbo_add_mbo_ie_bsstrans_reject(wlc_info_t* wlc, uint8* data, uint8 resp_status,
	uint8 reason_code);
int
wlc_mbo_roamscan_complete(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	uint8 status, uint8 req_mode, uint32 disassoc_dur,
	uint32 bss_term_tsf_h, uint32 bss_term_tsf_l);

#ifdef ANQPO
uint
wlc_mbo_calc_anqp_elem_len(wlc_bsscfg_t *cfg, uint8 *query, uint16 query_len,
	uint8 cellular_aware);
int
wlc_mbo_build_anqp_elem(wlc_bsscfg_t *cfg, uint8 *query, uint16 *query_len,
	uint8 cellular_aware, uint16 total_len);
int
wlc_mbo_process_bsstrans_req(wlc_info_t* wlc, wlc_bsscfg_t *bsscfg,
	dot11_bsstrans_req_t* req, int body_len, wnm_bsstrans_policy_type_t policy,
	void *roam_data, uint16 *act_flags,
	uint8 btq_token, uint8 *decision);
bool
wlc_mbo_is_reassoc_delay_timer_active(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	struct ether_addr *bssid);
#endif /* ANQPO */
#endif	/* _wlc_mbo_h_ */
