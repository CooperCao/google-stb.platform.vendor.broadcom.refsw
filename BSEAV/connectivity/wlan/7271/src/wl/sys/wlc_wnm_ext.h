/*
 * 802.11v definitions for
 * Broadcom 802.11abgn Networking Device Driver
 *
 * $ Copyright Broadcom $
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: wlc_wnm_ext.h 777371 2018-08-20 15:43:59Z ah951552 $
 */

/**
 * XXX
 * header file between WNM and its submodules.
 */

#ifndef _wnm_ext_h_
#define _wnm_ext_h_

typedef struct join_pref join_pref_t;

typedef enum wlc_wnm_bsstrans_proc_id {
	WLC_WNM_BSSTRANS_PROC_DFLT = 1,
	WLC_WNM_BSSTRANS_PROC_MBO = 2,
	WLC_WNM_BSSTRANS_PROC_WBTEXT = 3,
	WLC_WNM_BSSTRANS_PROC_WNM_PDT = 4,
	/* ADD before this line and update MAX */
	WLC_WNM_BSSTRANS_PROC_MAX = 5
} wlc_wnm_bsstrans_proc_id_e;

/* BSS Transition Req processing decision */
typedef enum {
	WLC_WNM_BTM_REQ_PROC_DFLT = 0,
	WLC_WNM_BTM_REQ_PROC_ACCEPT = 1,
	WLC_WNM_BTM_REQ_PROC_REJECT = 2
} wlc_wnm_bss_trans_proc_decision_e;

/* BSS Transition flags:
 * Valid combinations for ACCEPT :
 * a) RESPONSE | ROAM : send BTM response first, then roam
 * b) RESPONSE | DISASSOC : send BTM response first, then disassoc
 * c) RESPONSE | REASSOC : send BTM response first, then Reassoc
 * d) ROAM: Only roam first, dont send BTM resp
 * e) DISASSOC: Disassoc without sending any BTM response
 * f) ROAM | POST_RESP: Send BTM response post roam
 * g) REASSOC | POST_RESP: Send BTM response post reassoc
 * h) REASSOC : Only reassoc
 * i) RESPONSE | WAIT : send BTM response and wait
 * Valid combinations for REJECT :
 * a) RESPONSE : Send reject response.
 * b) no flag set.
 * Invalid combinations:
 * all other combinations are invalid
 */
#define WLC_WNM_BSS_TRANS_ACTN_ROAM          0x0001
#define WLC_WNM_BSS_TRANS_ACTN_DISASSOC      0x0002
#define WLC_WNM_BSS_TRANS_ACTN_RESPONSE      0x0004
#define WLC_WNM_BSS_TRANS_ACTN_REASSOC       0x0008
#define WLC_WNM_BSS_TRANS_ACTN_POST_RESP     0x0010
#define WLC_WNM_BSS_TRANS_ACTN_WAIT          0x0020
#define WLC_WNM_BSS_TRANS_ACTN_ROAMSCAN_WAIT          0x0040

/* in cases when the duration mentioned by AP is not valid, used to reject the frames */
#define DURATION_INVALID		0xFFFFFFFFu

typedef uint16 wlc_wnm_bsstrans_act_flags_t;
typedef struct wlc_wnm_bss_trans_roam_data {
	uint16 nbrlist_size;              /* num of nbr in Preferred Candidate List */
	struct ether_addr bssid;           /* BSSID of most preferred candidate */
	chanspec_t chsp;                   /* chanspec of most pref candidate */
	uint8  pref;                       /* preference value of most pref candidate */
	uint8 max_chsp;                    /* max num of chan spec */
	uint8 num_chsp;                    /* num of chan spec present */
	chanspec_t chsp_list[];            /* list of chan spec */
} wlc_wnm_bss_trans_roam_data_t;

typedef struct wnm_ngbr_rep wnm_ngbr_rep_t;
/* Neighbor Report element */
struct wnm_ngbr_rep {
	struct wnm_ngbr_rep *next;
	nbr_rpt_elem_t nbr_elt;
};

typedef struct wlc_wnm_bsstrans_parse_ctx {
	wlc_wnm_info_t *wnm;
	wlc_bsscfg_t *cfg;
	uint8 req_mode;
	wnm_ngbr_rep_t *nbr_list_head;
	uint16 nbrlist_size;
	uint32 disassoc_dur;
	uint32 validity_dur;
	uint8 token;
	uint32 bss_term_tsf_l;
	uint32 bss_term_tsf_h;
	uint32 bss_term_dur;
	uint32 bss_term_blacklist_dur;
} wlc_wnm_bsstrans_parse_ctx_t;

#define BSSTRANS_IS_ESS_TERM_SET(req_mode) (req_mode & DOT11_BSSTRANS_REQMODE_ESS_DISASSOC_IMNT)
#define BSSTRANS_IS_BSS_TERM_SET(req_mode) (req_mode & DOT11_BSSTRANS_REQMODE_BSS_TERM_INCL)
#define BSSTRANS_IS_DISASSOC_IMMINENT(req_mode) \
	(req_mode & DOT11_BSSTRANS_REQMODE_DISASSOC_IMMINENT)
#define BSSTRANS_IS_ABRIDED_SET(req_mode) (req_mode & DOT11_BSSTRANS_REQMODE_ABRIDGED)
#define BSSTRANS_IS_PREF_LIST_INCL(req_mode) (req_mode & DOT11_BSSTRANS_REQMODE_PREF_LIST_INCL)

/* Macro is used for checking if AP is going down. This will unify the implementation and behavior
 * of these two bits for now. Can be changed in future based on the requirement.
 */
#define BSSTRANS_IS_BSS_GOING_DOWN(req_mode)	BSSTRANS_IS_DISASSOC_IMMINENT(req_mode) || \
	BSSTRANS_IS_BSS_TERM_SET(req_mode)
/* Input data */
typedef struct wbrq_parse_data {
	wlc_bsscfg_t *cfg;
	/* already parsed btm req data context */
	wlc_wnm_bsstrans_parse_ctx_t *parse_ctx;
	/* frame body */
	uint8 *body;
	/* frame body len */
	uint16 body_len;
} wbrq_parse_data_t;

/* Output data */
typedef struct wbrq_parse_res {
	/* bss trans extn parsed data */
	void * extn_parse_ctx;
} wbrq_parse_res_t;

/* BTM Request parsing callback function for Extension sub-module.
 * Extension sub-module parses any extension specific IE here.
 * Extension sub-module can return a handle to parsed data.
 */
typedef int (*wlc_wnm_bsstrans_req_parse_cb) (void *ctx, wbrq_parse_data_t *req_data,
	wbrq_parse_res_t *res_data);

/* Input data */
typedef struct wbrq_process_data {
	wlc_bsscfg_t *cfg;
	/* already parsed btm req data context */
	wlc_wnm_bsstrans_parse_ctx_t *parse_ctx;
	/* bss trans extn parsed data context */
	void * extn_parse_ctx;
} wbrq_process_data_t;

/* Output data */
typedef struct wbrq_process_res {
	/* bss trans roam data */
	wlc_wnm_bss_trans_roam_data_t *roam_data;
	/* decision after processing */
	wlc_wnm_bss_trans_proc_decision_e decision;
	/* action flags */
	wlc_wnm_bsstrans_act_flags_t act_flags;
	/* extn specific btm resp status.
	 */
	uint8 resp_status;
	/* reason in case of reject */
	uint8 reason_code;
} wbrq_process_res_t;

/* BTM Request process callback function for Extension sub-module.
 * Extension sub-module take a decision to ACCEPT/REJECT the BTM req
 * here and decide on subseuqent actions (like: to roam, to reassoc,
 * to disassoc, send BTM Resp before roam or post roam).
 * Extension sub-module can return a handle to parsed data.
 */
typedef int (*wlc_wnm_bsstrans_req_process_cb) (void *ctx, wbrq_process_data_t *req_data,
	wbrq_process_res_t *res_data);

typedef struct wbrs_ie_data_t {
	wlc_bsscfg_t *cfg;
	/* already parsed btm req data context */
	wlc_wnm_bsstrans_parse_ctx_t *parse_ctx;
	/* bss trans extn parsed data context */
	void * extn_parse_ctx;
	/* status to be sent in BTM resp */
	uint8 resp_status;
	/* reason in case of reject */
	uint8 reason_code;
	/* buf = buffer to be written, NULL in case length cb */
	uint8 *buf;
	/* buf_len = buffer length, zero in case of length cb */
	uint16 buf_len;
} wbrs_ie_data_t;

/* BTM Resp IE len calculation callback function for Extension sub-module.
 * WNM calls this before allocating buffer for BTM Response to know how
 * many bytes extension moudule needs.
 * This callback will return number of bytes it adds to BTM Response for
 * adding any IE etc. if any. If no processing to be done, this returns zero
 */
typedef uint16 (*wlc_wnm_bsstrans_resp_ie_len_cb)
	(void *ctx, wbrs_ie_data_t *data);

/* BTM Resp IE build callback function for Extension sub-module.
 * This callback adds IE etc. to BTM Response if any.
 */
typedef int (*wlc_wnm_bsstrans_resp_ie_build_cb)
	(void *ctx, wbrs_ie_data_t *data);

typedef struct wlc_wnm_join_target_proc_data {
	wlc_bsscfg_t *cfg;
	/* already parsed btm req data context(BTM trigerred roam) */
	wlc_wnm_bsstrans_parse_ctx_t *parse_ctx;
	/* Evaluating BSS info */
	wlc_bss_info_t *bip;
	/* RSSI, Score of the evaluating BSS */
	join_pref_t *bip_pref;
	/* RSSI, Score of the referenced BSS */
	join_pref_t *cur_pref;
	uint roam_delta;
} wlc_wnm_join_target_proc_data_t;

typedef struct wlc_wnm_join_target_proc_res {
	/* calculated score based on policy */
	uint32 *target_score;
} wlc_wnm_join_target_proc_res_t;

/* During roam/join process join targets processing callback function for Extension sub-module.
 * WNM calls this so that sub-module extenstion can process join targets scores.
 */
typedef int (*wlc_wnm_proc_join_target_cb) (void *ctx, wlc_wnm_join_target_proc_data_t *data,
	wlc_wnm_join_target_proc_res_t *res_data);

/* input */
typedef struct wlc_wnm_roamscan_complete_data {
	wlc_bsscfg_t *cfg;
	/* already parsed btm req data context */
	wlc_wnm_bsstrans_parse_ctx_t *parse_ctx;
	/* status of roamscan join */
	uint8 status;
	/* target bssid */
	struct ether_addr *trgt_bssid;
} wlc_wnm_roamscan_complete_data_t;

/* output */
typedef struct wlc_wnm_roamscan_complete_res {
	bool delay_join;
} wlc_wnm_roamscan_complete_res_t;

/* Post roamscan, status processing callback function for Extension sub-module.
 * WNM calls this at end of roamscan complete to notify sub-module extenstion
 * to process status of roamscan like switching to cellular network etc.
 */
typedef int (*wlc_wnm_roamscan_complete_cb)
	(void *ctx, wlc_wnm_roamscan_complete_data_t *data,
	wlc_wnm_roamscan_complete_res_t *res_data);

typedef struct wlc_wnm_join_target_filter_data {
	wlc_bsscfg_t *cfg;
	/* already parsed btm req data context */
	wlc_wnm_bsstrans_parse_ctx_t *parse_ctx;
	wlc_bss_info_t *bi;
	bool for_roam;
} wlc_wnm_join_target_filter_data_t;

/* During roamjoin process join targets filtering callback function for Extension sub-module.
 * WNM calls this so that sub-module extenstion can filter join targets.
 * Ex: MBO can filter based on "Assoc Retry delay attr"  etc.
 */
typedef bool (*wlc_wnm_filter_join_target_cb)
	(void *ctx, wlc_wnm_join_target_filter_data_t *data);

/* timer callback for sub module like disassoc and validity timers
 * These timers are maintained in wnm module but on expiry it will
 * notify using callback for subsequent processing
 */
typedef void (*wlc_wnm_timer_cb)
	(void *ctx, wlc_bsscfg_t *cfg, uint8 timer_idx);

/* scan results filter callback - After scan results are processed by cook join routine
 * a list of valid ones is identified. Incase no valid ones are found any subsequent
 * action can be directed with this callback
 */
typedef bool (*wlc_wnm_filterscanresults_cb)
	(void *ctx, wlc_bsscfg_t *cfg, uint bss_count, bool for_roam);

/* join list sort callback - For any additional criteria. During roam, candidates are
 * sorted in asc order. While sorting if any additional parameter needs to be checked
 * ... do it using this callback .. ex., for wbtext, if scores are matching preference is
 * given to band, rssi and cu in that order
 */
typedef bool (*wlc_wnm_sort_joinlist_cb)
	(void *ctx, wlc_bsscfg_t *cfg, join_pref_t *join_pref,
		wlc_bss_info_t **bip, int idx);

/* sub-module scoring callback function */
typedef void (*wlc_wnm_scoring_cb)
	(wlc_bsscfg_t *cfg, wlc_bss_info_t **bip, uint bss_cnt, join_pref_t *join_pref,
		join_pref_t *current_join_pref, int assoc_apidx, int32 *score_delta,
		bool band_rssi_boost);

typedef struct wlc_wnm_bsstrans_proc_cbs {
	wlc_wnm_bsstrans_proc_id_e id;
	/* pointer to the registration time context */
	void *ctx;
	/* pointer to the extension parsed data context
	 * which is passed in wlc_wnm_bsstrans_req_parse_cb()
	 * and will be subsequently passed in other callbacks.
	 */
	void *extn_parse_ctx;
	wlc_wnm_bsstrans_req_parse_cb btm_req_parse;
	wlc_wnm_bsstrans_req_process_cb btm_req_process;
	wlc_wnm_bsstrans_resp_ie_len_cb btm_resp_ie_len;
	wlc_wnm_bsstrans_resp_ie_build_cb btm_resp_ie_build;
	wlc_wnm_proc_join_target_cb join_tgt_proc;
	wlc_wnm_filter_join_target_cb join_tgt_filter;
	wlc_wnm_roamscan_complete_cb roamscan_cmplt;
	/* Upon timer expiry.. sub module intimation */
	wlc_wnm_timer_cb timer_cb;
	/* For candidate scoring, policy specific callback */
	wlc_wnm_scoring_cb scoring_cb;
	/* For zero valid APs .. any action? */
	wlc_wnm_filterscanresults_cb filterscanresults_cb;
	/* sorting callback using additional criteria */
	wlc_wnm_sort_joinlist_cb joinlist_sort_cb;
} wlc_wnm_bsstrans_proc_cbs_t;

typedef void* wlc_wnm_bsstrans_proc_cbs_hndl_t;

/* used for maintaining timer running status */
enum {
	RESP_TMR_IDX = 1,
	DISASSOC_TMR_IDX = 2,
	NBRLIST_VALID_TMR_IDX = 3,
	MAX_TMR_IDX = 4
};

typedef struct timer_callback_ctx {
	wlc_wnm_info_t *wnm;		/* used to reach the correct wlc */
	int join_cfgidx;			/* cfg idx linked to the timer */
	uint32 timer_to;			/* configured timeout in ms */
	uint32 timer_added_at;	/* time when timer was added */
	uint32 timer_pending_to;	/* time for pending timeout .. used during clone */
	struct wl_timer *timer;	/* timer to which this context belongs */
	bool timer_active;		/* is the timer is running currently */
} timer_cb_ctx_t;

wlc_wnm_bsstrans_proc_cbs_hndl_t
wlc_wnm_register_bsstrans_proc_cbs(wlc_wnm_info_t *wnm,
	wlc_wnm_bsstrans_proc_cbs_t *cbs);
int wlc_wnm_unregister_bsstrans_proc_cbs(wlc_wnm_info_t *wnm,
	wlc_wnm_bsstrans_proc_cbs_hndl_t hndl);
int wlc_wnm_update_bsstrans_req_block(wlc_wnm_bsstrans_proc_cbs_hndl_t hndl,
	wlc_bsscfg_t *cfg, bool block);

/* bsstrans utility API */
int wlc_wnm_bsstrans_get_pref_bss_data(wlc_wnm_info_t *wnm, wlc_bsscfg_t *cfg,
	wlc_wnm_bss_trans_roam_data_t *roam_data);
bool wlc_wnm_bsstrans_is_bss_pref_zero(wlc_wnm_info_t *wnm, wlc_bsscfg_t *cfg,
	struct ether_addr *ea, uint8 channel);
void wlc_wnm_bsstrans_del_all_nbr(wlc_wnm_info_t *wnm, wlc_bsscfg_t *cfg);
wnm_bsstrans_policy_type_t wlc_wnm_bsstrans_get_policy(wlc_wnm_info_t *wnm);
void wlc_wnm_del_timer(wlc_bsscfg_t *bsscfg, uint8 idx);
bool wlc_wnm_get_timerstatus(wlc_bsscfg_t *bsscfg, uint8 idx);
void wlc_wnm_add_timer(wlc_bsscfg_t *bsscfg, uint32 duration, uint8 idx);
void wlc_wnm_reset_timerinfo(wlc_bsscfg_t *bsscfg, uint8 idx);
struct wl_timer * wlc_wnm_init_timer(wlc_bsscfg_t *bsscfg, void (*fn)(void* arg), uint8 idx);
uint32 wlc_wnm_get_pending_time(wlc_bsscfg_t *bsscfg, uint8 idx);
wnm_ngbr_rep_t *wlc_wnm_bsstrans_get_ngbr_rep(wlc_wnm_info_t *wnm, wlc_bsscfg_t *cfg,
	struct ether_addr *ea, uint8 channel);
uint8 wlc_wnm_bsstrans_get_reqmode(wlc_wnm_info_t *wnm, wlc_bsscfg_t *cfg);

bool wlc_wnm_check_roam_throttle(wlc_wnm_info_t *wnm);
void wlc_wnm_update_roam_throttle(wlc_wnm_info_t *wnm);
void wlc_wnm_bsstrans_set_policy(wlc_wnm_info_t *wnm, wnm_bsstrans_policy_type_t policy);
uint32 wlc_wnm_bsstrans_get_disassoc_dur(wlc_wnm_info_t *wnm, wlc_bsscfg_t *cfg,
	uint8 req_mode);
void wlc_wnm_clear_timers(wlc_bsscfg_t * bsscfg);
#endif	/* _wnm_ext_h_ */
