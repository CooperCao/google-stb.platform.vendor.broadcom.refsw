/*
 * STF MIMOPS module source,
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
 * $Id: wlc_stf_mimops.h $
 */

#ifndef _wlc_stf_mimops_h_
#define _wlc_stf_mimops_h_

#define DEFAULT_DELTA_MIMOPS_THRESHOLD 5 /* MRC additional thresold to avoid ping pongs */
#define MIMO_PS_CFG_IOVAR_BW_FULL 0
#define MIMO_PS_CFG_IOVAR_BW_20M  1
#define MIMO_PS_CFG_IOVAR_BW_40M  2
#define MIMO_PS_CFG_IOVAR_BW_80M  3
#define MIMO_PS_CFG_IOVAR_BW_160M  4
#define MIMO_PS_CFG_IOVAR_BW_8080M 5

#define WLC_STF_LEARNING_ENABLED(mimo_ps_cfg) \
((mimo_ps_cfg->mimo_ps_learning_running == TRUE)&& \
	(mimo_ps_cfg->mimo_ps_smps_no_of_packets > 0))


#define SEND_OPER_MODE_NOTIFICATION 1
#define SEND_CH_WIDTH_ACTION_NOTIFICATION 2
#define SEND_SMPS_ACTION_NOTIFICATION 4


#define MIMO_PS_CFG_STATE_NONE 0
#define MIMO_PS_CFG_STATE_INFORM_AP_INPROGRESS 1
#define MIMO_PS_CFG_STATE_INFORM_AP_DONE 2
#define MIMO_PS_CFG_STATE_LEARNING 3
#define MIMO_PS_CFG_STATE_HW_CONFIGURE 4
#define MIMO_PS_CFG_STATE_INFORM_AP_PENDING 5

#define MIMOPS_CLEAN_UP_JOIN_ADOPT	0
#define MIMOPS_CLEAN_UP_DISASSOC	1
#define MIMOPS_CLEAN_UP_LINKDOWN	2


#if defined(EVENT_LOG_COMPILE)
#define WL_STF_MIMO_PS_TRACE(args)  \
    EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_MIMO_PS_TRACE, args)
#define WL_STF_MIMO_PS_INFO(args)   \
    EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_MIMO_PS_INFO, args)
#define WL_STF_MIMO_PS_ERROR(args)  \
    EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_MIMO_PS_ERROR, args)
#else
#define WL_STF_MIMO_PS_TRACE(args)  WL_TRACE(args)
#define WL_STF_MIMO_PS_INFO(args)   WL_INFORM(args)
#define WL_STF_MIMO_PS_ERROR(args)  WL_ERROR(args)
#endif /* EVENT_LOG_COMPILE */

struct wlc_mimo_ps_cfg {
	/* MIMO PS related Parameters for this bsscfg */
	/* SMPS and  pm_bcnrx are disabled if they are already enabled to benefit */
	/* in range from Maximal ratio combining (MRC) */
	int8    mrc_rssi_threshold;
	/* specifies the time STA will wait before receiving packets */
	/* when moving from higher configuration to lower configuration */
	uint8   cfg_update_in_progress;
    int8    mrc_delta_threshold;
    bool    mrc_override;
    bool    mrc_chains_changed;
    bool    cfg_change_timer_active;        /* Is cfg change timer active? */
    uint    mimo_ps_cfg_change_wait_time;
    uint    mimo_ps_smps_no_of_packets;
    bool    mimo_ps_learning_running;
    bool    mimo_ps_cfg_host_params_valid;
    wl_mimops_cfg_t *mimo_ps_cfg_host_params;
    struct  wl_timer *cfg_change_timer; /* timer to wait after cfg change */
    wl_mimo_ps_learning_event_data_t mimo_ps_learning_data;
    uint    configure_hw_active_chains;
    uint16  configure_hw_chanspec_bw;
    uint8   opermode_to_inform;
    uint8   chwidth_to_inform;
    uint8   mimo_ps_mode_to_inform;
    uint8   message_map_to_inform;
    int8    mimo_ps_learning_rssi_threshold;
	bool	mimops_dynamic_disabled;	/* state of dynamic mimops feature if disabled */
};

extern void wlc_update_mimops_cfg_transition(wlc_info_t *wlc);
extern uint16 wlc_stf_mimo_ps_iovarbw2chanspecbw(wlc_bsscfg_t *bsscfg, uint16 iovarbw);
void wlc_stf_update_ht_cap_for_bw(uint16 *cap, uint16 bw);
void wlc_stf_set_host_configured_bw(wlc_bsscfg_t *cfg, wlc_bss_info_t *bi);
void wlc_stf_mimops_handle_csa_chanspec(wlc_bsscfg_t *cfg);
void wlc_stf_mimops_handle_txfifo_complete(wlc_info_t *wlc);
bool wlc_stf_mimops_handle_bw_upd(wlc_info_t *wlc, wlc_bsscfg_t *cfg, chanspec_t new_chspec);
extern void wlc_stf_mimops_set_bw_upd_in_progress(wlc_info_t *wlc, bool in_progress);
void wlc_stf_mimops_set_txchain_upd_in_progress(wlc_info_t *wlc, bool in_progress);
void wlc_stf_mimops_handle_rxchain_set(wlc_info_t *wlc, uint8 rxchain, uint8 rxchain_cnt);
extern void wlc_stf_update_mimo_ps_learning_update(wlc_bsscfg_t *bsscfg, uint8 type);
extern int wlc_stf_mimo_ps_learning_config_set(wlc_info_t *wlc,
                                               wlc_bsscfg_t *bsscfg,  void *p);
extern void wlc_stf_mimo_ps_config_get(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,  void *a);
extern int wlc_stf_mimo_ps_status_get(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,  void *a);
extern int wlc_stf_mimo_ps_cfg(wlc_bsscfg_t *bsscfg, wl_mimops_cfg_t *mimo_ps_cfg);
extern void wlc_stf_mimo_ps_clean_up(wlc_bsscfg_t *bsscfg, uint8 reason);
extern void wlc_stf_cfg_timer_start(wlc_bsscfg_t *bsscfg);

extern void wlc_stf_cfg_change_timer_expiry(void  *arg);
extern void wlc_stf_mimo_ps_mrc_handling(wlc_bsscfg_t *bsscfg);
extern int wlc_stf_mrc_thresh_handling(wlc_bsscfg_t *bsscfg);
extern void wlc_stf_config_siso(wlc_bsscfg_t *bsscfg, bool config_siso);
extern void wlc_stf_cfg_timer_start(wlc_bsscfg_t *bsscfg);
extern void wlc_stf_update_mimo_ps_cfg_data(wlc_bsscfg_t *bsscfg, ratespec_t rspec);
extern void wlc_stf_wl_up(wlc_info_t *wlc);
void wlc_stf_handle_csa_chanspec(wlc_bsscfg_t *cfg);
void wlc_stf_handle_mimo_ps_action_frames(wlc_bsscfg_t *bsscfg);
extern void wlc_ch_width_action_ht_send(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint8 ch_width);
#if defined(WL11AC)
extern bool wlc_stf_mimops_set(wlc_info_t* wlc, wlc_bsscfg_t *cfg, int32 int_val);
#endif /* WL11AC */


extern int BCMATTACHFN(wlc_stf_mimops_attach)(wlc_info_t* wlc, void *stf_arb_mps_info);
extern void BCMATTACHFN(wlc_stf_mimops_detach)(wlc_info_t* wlc,
    void *stf_arb_mps_info);
extern wlc_mimo_ps_cfg_t * wlc_stf_mimo_ps_cfg_get(wlc_bsscfg_t *bsscfg);
extern bool wlc_stf_mimops_check_mrc_overrride(wlc_info_t *wlc);

#endif /* _wlc_stf_mimops_h_ */
