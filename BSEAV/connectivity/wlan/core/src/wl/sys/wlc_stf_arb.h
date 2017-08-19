/*
 * STF Arbitrator module source,
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
 * $Id: wlc_stf_arb.h $
 */

#ifndef _wlc_stf_arb_h_
#define _wlc_stf_arb_h_

#if defined(EVENT_LOG_COMPILE)
#define WL_STF_ARBITRATOR_ERROR(args) \
    EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_STF_ARBITRATOR_ERROR, args)
#define WL_STF_ARBITRATOR_TRACE(args) \
    EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_STF_ARBITRATOR_TRACE, args)
#define WL_STF_ARBITRATOR_WARN(args) \
    EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_STF_ARBITRATOR_WARN, args)
#else
#define WL_STF_ARBITRATOR_TRACE(args)   WL_TRACE(args)
#define WL_STF_ARBITRATOR_INFO(args)    WL_INFORM(args)
#define WL_STF_ARBITRATOR_ERROR(args)   WL_ERROR(args)
#endif /* EVENT_LOG_COMPILE */

/* Defines for request ID's; use only defines once */
#define WLC_STF_ARBITRATOR_REQ_ID_LTE_COEX  1
#define WLC_STF_ARBITRATOR_REQ_ID_MIMOPS_BSS    2
#define WLC_STF_ARBITRATOR_REQ_ID_AWDL_BSS  3
#define WLC_STF_ARBITRATOR_REQ_ID_SCAN      4
#define WLC_STF_ARBITRATOR_REQ_ID_TXPPR     5
#define WLC_STF_ARBITRATOR_REQ_ID_PWRTHOTTLE    6
#define WLC_STF_ARBITRATOR_REQ_ID_TMPSENSE  7
#define WLC_STF_ARBITRATOR_REQ_ID_IOVAR     8
#define WLC_STF_ARBITRATOR_REQ_ID_AP_BSS    9


/* Defines for request entry priority; use only defines once */
/* Lower the higher in priority */
#define WLC_STF_ARBITRATOR_REQ_PRIO_IOVAR   0
#define WLC_STF_ARBITRATOR_REQ_PRIO_PWRTHOTTLE  3
#define WLC_STF_ARBITRATOR_REQ_PRIO_TMPSENSE    10
#define WLC_STF_ARBITRATOR_REQ_PRIO_TXPPR   5
#define WLC_STF_ARBITRATOR_REQ_PRIO_LTE_COEX    4 /* Swapped with Temp sense for now */
#define WLC_STF_ARBITRATOR_REQ_PRIO_AWDL_BSS    15
#define WLC_STF_ARBITRATOR_REQ_PRIO_MIMOPS_BSS  20
#define WLC_STF_ARBITRATOR_REQ_PRIO_AP_BSS  20
#define WLC_STF_ARBITRATOR_REQ_PRIO_SCAN    30

#define WLC_STF_ARBITRATOR_REQ_DEFAULT_TXSTREAM 0xff
#define WLC_STF_ARBITRATOR_REQ_DEFAULT_RXSTREAM 0xff

/* Macros active inactive states */
#define WLC_STF_ARBI_REQ_STATE_ACTIVE(req)  ((req)->state)
#define WLC_STF_ARBI_REQ_STATE_INACTIVE(req)    (!((req)->state))
#define WLC_STF_ARBI_REQ_STATE_TXACTIVE(req) \
	      ((req)->state & WLC_STF_ARBITRATOR_REQ_STATE_TX_ACTIVE)
#define WLC_STF_ARBI_REQ_STATE_RXACTIVE(req) \
	      ((req)->state & WLC_STF_ARBITRATOR_REQ_STATE_RX_ACTIVE)

/* These are chains and bw config per bss */
struct wlc_hw_config {
	uint8   default_rxchains;
	uint8   default_txchains;
	/* Current configuration for hw to be applied */
	uint8   current_rxchains;
	uint8   current_txchains;
	/* Last tx chain count used to set up rates */
	uint8   last_ntxchains;
	uint8   ch_width_val;
	uint8   ch_width_valPM;
	uint8   ch_width_ActionRetry;
	uint8   ch_width_ActionPending;
	bool    chanspec_override;
	chanspec_t  current_chanspec_bw;
	chanspec_t  original_chanspec_bw;
	chanspec_t  default_chanspec_bw;
	bool    onlySISO;
	bool    basic_rates_present;
};

typedef void (wlc_stf_arb_cb_t)(void *ctx, uint8 txc_active, uint8 rxc_active);
/* STF request entry */
struct wlc_stf_nss_request_st {
    uint8   req_id; /* Unique Request ID */
    uint8   prio;   /* Priority of Request */
    uint8   state;  /* State of request, active or inactive */
    uint8   rxchains;       /* Requested rxchains */
    uint8   rx_nss;         /* Requested rx streams */
    uint8   txchains;       /* Requested txchains */
    uint8   tx_nss; /* Requested tx streams */
    uint8   flags;  /* Internal flags */
    wlc_stf_arb_cb_t        *call_back_fn; /* Call back when chain config changes */
    void *priv_ctx; /* Private pointer for internal to requester */
};

/* STF request entry queue */
struct wlc_stf_nss_request_q_st {
    wlc_stf_nss_request_t *req;
    struct wlc_stf_nss_request_q_st *next;
};

struct wlc_stf_arb {
    void    *iovar_req;
    void    *lte_req;
    void    *txppr_req;
    void    *pwrthrottle_req;
    void    *tmpsense_req;
    uint8   iovar_rxtxchain_active;
    int8    iovar_req_txchain;
    int8	iovar_req_rxchain;
    uint8	txchain_update_in_progress;
};

extern wlc_stf_arb_t * BCMATTACHFN(wlc_stf_arb_attach)(wlc_info_t* wlc, void *ctx);
extern int BCMATTACHFN(wlc_stf_arb_mimops_info_attach)(wlc_info_t* wlc);
extern void BCMATTACHFN(wlc_stf_arb_detach)(wlc_info_t* wlc, void *ctx);
extern void BCMATTACHFN(wlc_stf_arb_mimops_info_detach)(wlc_info_t* wlc);

extern void *wlc_stf_nss_request_register(wlc_info_t *wlc,
        uint8 req_id, uint8 prio, void *cb, void *ctx);
extern int32 wlc_stf_nss_request_unregister(wlc_info_t *wlc, void *request);
extern int32 wlc_stf_nss_request_update(wlc_info_t *wlc, void *req,
        uint8 state, uint8 txcmap, uint8 ntxstreams, uint8 rxcmap, uint8 nrxstreams);
extern void wlc_stf_nss_request_dump(wlc_info_t *wlc);
extern void wlc_stf_arb_handle_txfifo_complete(wlc_info_t *wlc);
extern void wlc_stf_arb_call_back(void *ctx, uint8 txc_active, uint8 rxc_active);
extern void wlc_stf_arb_set_txchain_upd_in_progress(wlc_info_t *wlc, bool in_progress);
extern int wlc_stf_siso_bcmc_rx(wlc_info_t *wlc,  bool enable);

extern wlc_hw_config_t * wlc_stf_bss_hw_cfg_get(wlc_bsscfg_t *bsscfg);

extern wlc_stf_nss_request_t * wlc_stf_bss_arb_req_get(wlc_bsscfg_t *bsscfg);

#endif /* _wlc_stf_arb_h_ */
