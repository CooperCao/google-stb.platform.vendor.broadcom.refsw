/**
 * @file
 * @brief
 * Code that controls the MIMOPS config
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * http://hwnbu-twiki.sj.broadcom.com/bin/view/Mwgroup/MIMOPSPOwerSave
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: wlc_stf_mimops.c $
 */


#include <wlc_cfg.h>
#include <wlc_types.h>
#include <typedefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <wlc_log.h>
#include <wl_dbg.h>
#include <wlioctl.h>
#include <d11.h>

#include <wlc_event_utils.h>
#include <wl_export.h>
#include <wlc_bsscfg.h>
#include <wlc_assoc.h>
#include <wlc.h>
#include <wlc_scb.h>
#include <wlc_stf.h>

#include <wlc_scandb.h>
#include <wlc_scan_priv.h>
#include <wlc_scan.h>
#include <wlc_ht.h>
#ifdef WL11AC
#include <wlc_vht.h>
#endif
#include <wlc_scb_ratesel.h>
#include <wlc_lq.h>

#define MIMO_PS_VHT_BW_VALUE_20M 0
#define MIMO_PS_VHT_BW_VALUE_NON_20M 1
#define WLC_STF_VT_MAX_RX_MCS_OCTETS 10
#define MIMO_PS_UPDATE 0
#define MIMO_PS_UPDATE_AP 1
#define WL_RX_NSS_1 1


#define MIMO_PS_SET_LEARNING_FLAG(mimo_ps_cfg) \
		mimo_ps_cfg->mimo_ps_learning_running = TRUE; \

#define MIMO_PS_CLEAR_LEARNING_FLAG(mimo_ps_cfg) \
		mimo_ps_cfg->mimo_ps_learning_running = FALSE; \

#define MIMO_PS_RESET_LEARNING_PARAMS(mimo_ps_cfg) \
	mimo_ps_cfg->mimo_ps_learning_data.totalSISO_above_rssi_threshold = 0; \
	mimo_ps_cfg->mimo_ps_learning_data.totalSISO_below_rssi_threshold = 0; \
	mimo_ps_cfg->mimo_ps_learning_data.totalMIMO_above_rssi_threshold = 0; \
	mimo_ps_cfg->mimo_ps_learning_data.totalMIMO_below_rssi_threshold = 0; \


#if defined(WL11AC) || defined(WL_MIMOPS_CFG)
static int8 wlc_stf_mimops_get(wlc_info_t* wlc);
#endif /* WL11AC || WL_MIMOPS_CFG */

static void wlc_stf_mimo_ps_cfg_convert_params(wlc_bsscfg_t *bsscfg,
	wl_mimops_cfg_t *mimo_ps_cfg, uint8 *mode, int *rx_chains, uint16 *chanspec);
static int wlc_stf_mimo_ps_handle_identical_config(wlc_bsscfg_t *bsscfg,
	wl_mimops_cfg_t *mimo_ps_cfg);
static void wlc_stf_start_learning(wlc_bsscfg_t *bsscfg);
#define MIMO_PS_STATUS_REQ_TO_HW_STATE(req_id) (0x1 << ((req_id)-1))
static int wlc_stf_mimo_ps_status_get_hw_state(wlc_info_t *wlc, uint16 *hw_state);
static int wlc_stf_mimo_ps_status_get_apcap(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint8 *ap_cap);
static int wlc_stf_mimo_ps_status_get_assoc_status(wlc_info_t *wlc,
	wlc_bsscfg_t *bsscfg, uint8 *as);
static bool mimo_ps_status_if_vht_siso_rates(uint16 vht_mcsmap);
static bool mimo_ps_status_if_ht_siso_rates(struct scb *scb);
static uint8 wlc_stf_mimo_ps_chanspec2iovarbw(chanspec_t chanspec);
static void wlc_stf_mrc_clean_up(wlc_bsscfg_t *bsscfg);
static void wlc_stf_set_mimo_ps_cfg_to_hw(wlc_bsscfg_t *bsscfg);
static int wlc_stf_mimops_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_stf_mimops_deinit(void *ctx, wlc_bsscfg_t *cfg);
static int
wlc_stf_mimops_cfg_cubby_get(void *ctx, wlc_bsscfg_t *cfg, uint8 *data, int *len);
static int
wlc_stf_mimops_cfg_cubby_set(void *ctx, wlc_bsscfg_t *cfg, const uint8 *data, int len);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void wlc_stf_mimops_cfg_cubby_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);
#else
#define wlc_stf_mimops_cfg_cubby_dump NULL
#endif
static void wlc_stf_mimops_assoc_state_cb(void *ctx, bss_assoc_state_data_t *notif_data);
static void wlc_stf_mimops_disassoc_state_cb(void *ctx, bss_disassoc_notif_data_t *notif_data);
static void wlc_stf_mimops_rx_bcn_notif(void *ctx, bss_rx_bcn_notif_data_t *notif_data);
static void wlc_stf_update_bmac_if_siso_rx_rates(wlc_bsscfg_t *bsscfg,
	ht_cap_ie_t *cap_ie, vht_cap_ie_t *vht_cap_ie_p,
	bcm_tlv_t *sup_rates, bcm_tlv_t *ext_rates);

#define STF_MIMOPS_BSSCFG_CUBBY_LOC(sm, cfg) \
	((wlc_mimo_ps_cfg_t **)BSSCFG_CUBBY((cfg), (sm)->cfgh_mps))
#define STF_MIMOPS_BSSCFG_CUBBY(sm, cfg) (*(STF_MIMOPS_BSSCFG_CUBBY_LOC(sm, cfg)))
#define WLC_STF_MIMOPS_BSSCFG_CUBBY_SIZE	(sizeof(wlc_mimo_ps_cfg_t))

#define WLC_STF_NSS_1RX 1

int
BCMATTACHFN(wlc_stf_mimops_attach)(wlc_info_t* wlc, void *ctx)
{
	wlc_stf_arb_mps_info_t *stf_arb_mps_info = (wlc_stf_arb_mps_info_t *)ctx;
	bsscfg_cubby_params_t bsscfg_cubby_params;
	int err = BCME_OK;

	/* reserve cubby space in the bsscfg container for per-bsscfg private data */
	memset(&bsscfg_cubby_params, 0, sizeof(bsscfg_cubby_params));
	bsscfg_cubby_params.context = stf_arb_mps_info;
	bsscfg_cubby_params.fn_deinit = wlc_stf_mimops_deinit;
	bsscfg_cubby_params.fn_init	= wlc_stf_mimops_init;
	bsscfg_cubby_params.fn_get = wlc_stf_mimops_cfg_cubby_get;
	bsscfg_cubby_params.fn_set = wlc_stf_mimops_cfg_cubby_set;
	bsscfg_cubby_params.config_size = WLC_STF_MIMOPS_BSSCFG_CUBBY_SIZE;
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	bsscfg_cubby_params.fn_dump = wlc_stf_mimops_cfg_cubby_dump;
#endif
	/* reserve cubby */
	if ((stf_arb_mps_info->cfgh_mps = wlc_bsscfg_cubby_reserve_ext(wlc,
	                                  sizeof(wlc_mimo_ps_cfg_t *),
	                                  &bsscfg_cubby_params)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve failed\n",
		         WLCWLUNIT(wlc), __FUNCTION__));
		err = BCME_NOMEM;
		goto fail;
	}

	/* register assoc and disassoc state change notification callback */
	if ((err = wlc_bss_assoc_state_register(wlc, wlc_stf_mimops_assoc_state_cb,
	                                        stf_arb_mps_info)) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bss_assoc_state_register failed\n",
		         WLCWLUNIT(wlc), __FUNCTION__));
		goto fail;
	}
	if ((err = wlc_bss_disassoc_notif_register(wlc, wlc_stf_mimops_disassoc_state_cb,
	                                           stf_arb_mps_info)) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bss_disassoc_state_register failed\n",
		         WLCWLUNIT(wlc), __FUNCTION__));
		goto fail;
	}
	/* register bcn notify */
	if (wlc_bss_rx_bcn_register(wlc, (bss_rx_bcn_notif_fn_t)wlc_stf_mimops_rx_bcn_notif,
	                            stf_arb_mps_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_rx_bcn_notif_register() failed\n",
		         WLCWLUNIT(wlc), __FUNCTION__));
		goto fail;
	}
	return BCME_OK;

fail:
	wlc_stf_mimops_detach(wlc, ctx);
	return err;

}


void
BCMATTACHFN(wlc_stf_mimops_detach)(wlc_info_t* wlc, void *ctx)
{
	wlc_stf_arb_mps_info_t *stf_arb_mps_info = (wlc_stf_arb_mps_info_t *)ctx;

	/* unregister assoc state change notification callback */
	(void)wlc_bss_assoc_state_unregister(wlc, wlc_stf_mimops_assoc_state_cb,
	                                     stf_arb_mps_info);
	/* unregister disassoc state change notification callback */
	(void)wlc_bss_disassoc_notif_unregister(wlc, wlc_stf_mimops_disassoc_state_cb,
	                                        stf_arb_mps_info);
	/* unregister bcn notify */
	wlc_bss_rx_bcn_unregister(wlc, (bss_rx_bcn_notif_fn_t)wlc_stf_mimops_rx_bcn_notif,
	                          stf_arb_mps_info);
}

static void
wlc_stf_mimops_assoc_state_cb(void *ctx, bss_assoc_state_data_t *notif_data)
{
	wlc_stf_arb_mps_info_t *stf_arb_mps_info = (wlc_stf_arb_mps_info_t *)ctx;
	wlc_bsscfg_t *bsscfg;
	wlc_bss_info_t *bi;
	wlc_info_t *wlc;

	ASSERT(stf_arb_mps_info != NULL);
	ASSERT(notif_data != NULL);

	wlc = stf_arb_mps_info->wlc;
	ASSERT(wlc != NULL);

	bsscfg = notif_data->cfg;
	ASSERT(bsscfg != NULL);

	switch (notif_data->state) {
		case AS_JOIN_START:
			/* ToDo: could adjust all targets as well */
			bi = wlc_assoc_get_join_target(wlc, 0);
			if (bi == NULL)
				return;
			if (WLC_MIMOPS_ENAB(wlc->pub)) {
				wlc_stf_set_host_configured_bw(bsscfg, bi);
			}
			break;
		case AS_JOIN_ADOPT:
			if (WLC_MIMOPS_ENAB(wlc->pub)) {
				/* Set the default rx and txchain */
				wlc_stf_mimo_ps_clean_up(bsscfg, MIMOPS_CLEAN_UP_JOIN_ADOPT);
			}
			break;
		default:
			/* slected case are handled yet */
			break;
	}
}

static void
wlc_stf_mimops_disassoc_state_cb(void *ctx, bss_disassoc_notif_data_t *notif_data)
{
	wlc_stf_arb_mps_info_t *stf_arb_mps_info = (wlc_stf_arb_mps_info_t *)ctx;
	wlc_bsscfg_t *bsscfg;
	wlc_info_t *wlc;

	BCM_REFERENCE(wlc);
	ASSERT(stf_arb_mps_info != NULL);
	ASSERT(notif_data != NULL);

	bsscfg = notif_data->cfg;
	ASSERT(bsscfg != NULL);

	wlc = stf_arb_mps_info->wlc;
	ASSERT(wlc != NULL);

	/* currently just returning, enable this menthod after testing */
	if (1) return;

	if (notif_data->type != DAN_TYPE_LOCAL_DISASSOC)
		return;

	if (WLC_MIMOPS_ENAB(wlc->pub)) {
		/* Set the default rx and txchain */
		wlc_stf_mimo_ps_clean_up(bsscfg, MIMOPS_CLEAN_UP_DISASSOC);
	}
}

/* MIMO PS related cubby */
static int
wlc_stf_mimops_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_stf_arb_mps_info_t *arb_mps_info = (wlc_stf_arb_mps_info_t *)ctx;
	wlc_info_t *wlc = arb_mps_info->wlc;
	wlc_mimo_ps_cfg_t **pmimo_ps_cfg = STF_MIMOPS_BSSCFG_CUBBY_LOC(arb_mps_info, cfg);
	wlc_mimo_ps_cfg_t *mimo_ps_cfg;
	int err = BCME_OK;

	if ((mimo_ps_cfg =
	    (wlc_mimo_ps_cfg_t*)MALLOCZ(wlc->osh, (uint)sizeof(wlc_mimo_ps_cfg_t))) == NULL) {

		WL_ERROR(("wl%d: %s: wlc_mimo_ps_cfg_t_alloc failed\n",
		         wlc->pub->unit, __FUNCTION__));
		err = BCME_NOMEM;
		goto fail;
	}

	mimo_ps_cfg->mrc_rssi_threshold = WLC_RSSI_MINVAL_INT8;
	mimo_ps_cfg->mimo_ps_learning_rssi_threshold = WLC_RSSI_MINVAL_INT8;
	mimo_ps_cfg->mrc_delta_threshold = DEFAULT_DELTA_MIMOPS_THRESHOLD;

	/* currently dynamic mimops feature such as sending smps, learning, etc. is disabled */
	mimo_ps_cfg->mimops_dynamic_disabled = 1;

	if ((mimo_ps_cfg->mimo_ps_cfg_host_params =
	    (wl_mimops_cfg_t*)MALLOCZ(wlc->osh, (uint)sizeof(wl_mimops_cfg_t))) == NULL) {

		WL_ERROR(("wl%d: %s: wlc_mimo_host_param_alloc failed\n",
		         wlc->pub->unit, __FUNCTION__));
		err = BCME_NOMEM;
		goto fail;
	}

	/* create cfg change timer if its not initalized */
	if ((mimo_ps_cfg->cfg_change_timer =
		wl_init_timer(wlc->wl, wlc_stf_cfg_change_timer_expiry,
			(void*)cfg, "mimo_ps_cfg_timer")) == NULL) {
		err = BCME_NOMEM;
		goto fail;
	}

	*pmimo_ps_cfg = mimo_ps_cfg;
	return BCME_OK;

fail:
	wlc_stf_mimops_deinit(arb_mps_info, cfg);
	return err;

}
static void
wlc_stf_mimops_deinit(void *ctx, wlc_bsscfg_t *cfg)
{

	wlc_stf_arb_mps_info_t *arb_mps_info = (wlc_stf_arb_mps_info_t *)ctx;
	wlc_info_t *wlc = arb_mps_info->wlc;
	wlc_mimo_ps_cfg_t **pmimo_ps_cfg = STF_MIMOPS_BSSCFG_CUBBY_LOC(arb_mps_info, cfg);
	wlc_mimo_ps_cfg_t *mimo_ps_cfg = *pmimo_ps_cfg;

	if (mimo_ps_cfg) {
		if (mimo_ps_cfg->cfg_change_timer) {
			wl_free_timer(wlc->wl, mimo_ps_cfg->cfg_change_timer);
			mimo_ps_cfg->cfg_change_timer = NULL;
		}
		if (mimo_ps_cfg->mimo_ps_cfg_host_params) {
			MFREE(wlc->osh, mimo_ps_cfg->mimo_ps_cfg_host_params,
			      (uint)sizeof(wl_mimops_cfg_t));
			mimo_ps_cfg->mimo_ps_cfg_host_params = NULL;
			mimo_ps_cfg->mimo_ps_cfg_host_params_valid = FALSE;
		}
		MFREE(wlc->osh, mimo_ps_cfg,
		      (uint)sizeof(wlc_mimo_ps_cfg_t));
		*pmimo_ps_cfg = NULL;
	}
}

static int
wlc_stf_mimops_cfg_cubby_get(void *ctx, wlc_bsscfg_t *cfg, uint8 *data, int *len)
{
	wlc_stf_arb_mps_info_t *arb_mps_info = (wlc_stf_arb_mps_info_t *)ctx;
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg = NULL;

	ASSERT(cfg != NULL);
	ASSERT(data != NULL);
	ASSERT(arb_mps_info != NULL);
	if (*len < WLC_STF_MIMOPS_BSSCFG_CUBBY_SIZE) {
		WL_ERROR(("wl%d: %s: Buffer too short\n", arb_mps_info->wlc->pub->unit,
		         __FUNCTION__));
		*len = WLC_STF_MIMOPS_BSSCFG_CUBBY_SIZE;
		return BCME_BUFTOOSHORT;
	}
	bsscfg_mimo_ps_cfg = STF_MIMOPS_BSSCFG_CUBBY(arb_mps_info, cfg);
	ASSERT(bsscfg_mimo_ps_cfg != NULL);
	memcpy(data, (uint8*)bsscfg_mimo_ps_cfg, WLC_STF_MIMOPS_BSSCFG_CUBBY_SIZE);
	*len = WLC_STF_MIMOPS_BSSCFG_CUBBY_SIZE;
	return BCME_OK;
}

static int
wlc_stf_mimops_cfg_cubby_set(void *ctx, wlc_bsscfg_t *cfg, const uint8 *data, int len)
{
	wlc_stf_arb_mps_info_t *arb_mps_info = (wlc_stf_arb_mps_info_t *)ctx;
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg = NULL;
	const wlc_mimo_ps_cfg_t *from_bsscfg_mimo_ps_cfg = (const wlc_mimo_ps_cfg_t *)data;

	ASSERT(cfg != NULL);
	ASSERT(arb_mps_info != NULL);
	ASSERT(len == WLC_STF_MIMOPS_BSSCFG_CUBBY_SIZE);
	/* get the cubby info */
	bsscfg_mimo_ps_cfg = STF_MIMOPS_BSSCFG_CUBBY(arb_mps_info, cfg);
	ASSERT(bsscfg_mimo_ps_cfg != NULL);
	ASSERT(bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params != NULL);
	ASSERT(from_bsscfg_mimo_ps_cfg != NULL);
	ASSERT(from_bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params != NULL);

	bsscfg_mimo_ps_cfg->mrc_rssi_threshold =
		from_bsscfg_mimo_ps_cfg->mrc_rssi_threshold;
	bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params_valid =
		from_bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params_valid;
	*bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params =
	    *from_bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params;

	WL_STF_MIMO_PS_INFO(("wl%d.%d Set mimo_ps: info/cubby %p/%p, host_param_valid %d\n",
	                    arb_mps_info->wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
	                    arb_mps_info, bsscfg_mimo_ps_cfg,
	                    bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params_valid));

	return BCME_OK;

}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void
wlc_stf_mimops_cfg_cubby_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	wlc_stf_arb_mps_info_t *arb_mps_info = (wlc_stf_arb_mps_info_t *)ctx;
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg = STF_MIMOPS_BSSCFG_CUBBY(arb_mps_info, cfg);

	ASSERT(bsscfg_mimo_ps_cfg != NULL);
	ASSERT(bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params != NULL);

	bcm_bprintf(b, "============= bsscfg_mimo_ps_cfg =============\n");
	bcm_bprintf(b, "\tmimo_ps_cfg_host_params_valid %d\n",
	            bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params_valid);
	bcm_bprintf(b, "\tactive_chains %d mode %d bandwidth %d\n",
	            bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params->active_chains,
	            bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params->mode,
	            bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params->bandwidth);
	bcm_bprintf(b, "\tmrc: thrsh %d ovrride %d ch_cg %d, cfg_update %d\n",
	            bsscfg_mimo_ps_cfg->mrc_rssi_threshold,
	            bsscfg_mimo_ps_cfg->mrc_override,
	            bsscfg_mimo_ps_cfg->mrc_chains_changed,
	            bsscfg_mimo_ps_cfg->cfg_update_in_progress);

}
#endif /* BCMDBG || BCMDBG_DUMP */

static void
wlc_stf_mimops_rx_bcn_notif(void *ctx, bss_rx_bcn_notif_data_t *notif_data)
{
	wlc_stf_arb_mps_info_t *arb_mps_info = (wlc_stf_arb_mps_info_t *)ctx;

	ASSERT(arb_mps_info && notif_data);
	BCM_REFERENCE(arb_mps_info);

	if (notif_data->scb == NULL ||
		notif_data->cfg == NULL ||
		notif_data->data_ext == NULL)
		return;

#ifdef WL11AC
	if (BSSCFG_STA(notif_data->cfg) &&
	    notif_data->cfg->BSS &&
	    notif_data->cfg->assoc->state == AS_WAIT_RCV_BCN) {
		bcn_notif_data_ext_t *data_ext =  notif_data->data_ext;
		wlc_stf_update_bmac_if_siso_rx_rates(notif_data->cfg, data_ext->cap_ie,
			(vht_cap_ie_t *)data_ext->vht_cap_ie_p, data_ext->sup_rates,
			data_ext->ext_rates);
	}
#endif
}

void
wlc_stf_mimo_ps_config_get(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,  void *a)
{
	uint8 mimops_mode = 0;
	wl_mimops_cfg_t* cfg_ptr = (wl_mimops_cfg_t*)a;
	int rxchains = 0x1;
	uint16 chanspecbw = 0;
	uint8 applychangesafterlearning = 0;
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg = NULL;
	wlc_hw_config_t *bsscfg_hw_cfg = NULL;

	bsscfg_hw_cfg = wlc_stf_bss_hw_cfg_get(bsscfg);
	bsscfg_mimo_ps_cfg = wlc_stf_mimo_ps_cfg_get(bsscfg);

	if (!bsscfg_mimo_ps_cfg) {
		WL_ERROR(("wl%d: %s: wlc_mimops_cubby null\n",
		         WLCWLUNIT(wlc), __FUNCTION__));
		return;
	}
	if (!bsscfg_hw_cfg) {
		WL_ERROR(("wl%d: %s: wlc_hw_cfg_cubby null\n",
		         WLCWLUNIT(wlc), __FUNCTION__));
		return;
	}

	if (bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params_valid) {
		memcpy(cfg_ptr, bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params,
		       sizeof(*cfg_ptr));
	} else {
		chanspecbw = bsscfg_hw_cfg->current_chanspec_bw & WL_CHANSPEC_BW_MASK;
		rxchains = bsscfg_hw_cfg->current_rxchains;
		mimops_mode =  wlc_stf_mimops_get(wlc);

		if (chanspecbw == WL_CHANSPEC_BW_20) {
			cfg_ptr->bandwidth = MIMO_PS_CFG_IOVAR_BW_20M;
		} else if (chanspecbw == WL_CHANSPEC_BW_40) {
			cfg_ptr->bandwidth = MIMO_PS_CFG_IOVAR_BW_40M;
		} else if (chanspecbw == WL_CHANSPEC_BW_80) {
			cfg_ptr->bandwidth = MIMO_PS_CFG_IOVAR_BW_80M;
		} else {
			cfg_ptr->bandwidth = MIMO_PS_CFG_IOVAR_BW_FULL;
		}
		cfg_ptr->active_chains = (uint8)rxchains;
		cfg_ptr->mode = mimops_mode;
		cfg_ptr->applychangesafterlearning = applychangesafterlearning;
	}
	cfg_ptr->version = WL_MIMO_PS_CFG_VERSION_1;

	return;
}

int
wlc_stf_mimo_ps_learning_config_set(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,  void *p)
{
	int err = BCME_OK;
	wl_mimops_learning_cfg_t *cfg_ptr = (wl_mimops_learning_cfg_t*)p;
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg = wlc_stf_mimo_ps_cfg_get(bsscfg);

	if (bsscfg_mimo_ps_cfg->mimops_dynamic_disabled) {
		err = BCME_UNSUPPORTED;
		return err;
	}

	if (cfg_ptr->flag == WL_MIMO_PS_PS_LEARNING_CFG_CONFIG) {
		if (cfg_ptr->version > WL_MIMO_PS_PS_LEARNING_CFG_V1) {
			err = BCME_VERSION;
			return err;
		}
		/* Option to configure the no of packets */
		if (cfg_ptr->no_of_packets_for_learning == 0)
			wlc_stf_update_mimo_ps_learning_update(bsscfg,
			    WL_MIMO_PS_PS_LEARNING_CFG_ABORT);

		bsscfg_mimo_ps_cfg->mimo_ps_smps_no_of_packets =
			cfg_ptr->no_of_packets_for_learning;

		WL_STF_MIMO_PS_INFO((" WLC STF MIMO PS :"
			" SMPS learingiovar no Pkt %d",
			bsscfg_mimo_ps_cfg->mimo_ps_smps_no_of_packets));

		if (cfg_ptr->version == WL_MIMO_PS_PS_LEARNING_CFG_V1) {
			if (cfg_ptr->learning_rssi_threshold > 0) {
				err = BCME_BADARG;
				return err;
			}
			if (cfg_ptr->learning_rssi_threshold == 0) {
				WL_STF_MIMO_PS_INFO(("setting RSSI=0"
				        " causes no effect on"
				        "learning rssi threshold"));
				return err;
			}
			bsscfg_mimo_ps_cfg->mimo_ps_learning_rssi_threshold =
			    cfg_ptr->learning_rssi_threshold;
			WL_STF_MIMO_PS_INFO((" WLC STF MIMO PS :"
			        " SMPS learingiovar RSSI threshold %d",
			        bsscfg_mimo_ps_cfg->
			        mimo_ps_learning_rssi_threshold));
		} else {
			/* for version 0 rssi threshold is set from mrc threshold */
			bsscfg_mimo_ps_cfg->mimo_ps_learning_rssi_threshold =
			    bsscfg_mimo_ps_cfg->mrc_rssi_threshold;
		}
	} else {
		/* In case of ABORT or status request call the following */
		WL_STF_MIMO_PS_INFO((" WLC STF MIMO PS : SMPS learning - "
			"IOVAR rcvd - ABORT"));
		wlc_stf_update_mimo_ps_learning_update(bsscfg,
			(uint8)cfg_ptr->flag);
	}
	return err;
}

int
wlc_stf_mimo_ps_status_get(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,  void *a)
{
	int err = BCME_OK;
	wl_mimo_ps_status_t *status_ptr = (wl_mimo_ps_status_t *)a;
	uint16 mhfval = 0;
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg = wlc_stf_mimo_ps_cfg_get(bsscfg);
	wlc_hw_config_t *bsscfg_hw_cfg = wlc_stf_bss_hw_cfg_get(bsscfg);


	if (!bsscfg_mimo_ps_cfg || !bsscfg_hw_cfg) {
		WL_ERROR(("wl%d: %s: mimops_hw_cfg_cubby null\n",
		         WLCWLUNIT(wlc), __FUNCTION__));
		return BCME_ERROR;
	}
	status_ptr->version = WL_MIMO_PS_STATUS_VERSION_3;
	err = wlc_stf_mimo_ps_status_get_apcap(wlc, bsscfg,
		&status_ptr->ap_cap);
	err = wlc_stf_mimo_ps_status_get_assoc_status(wlc, bsscfg,
		&status_ptr->association_status);
	if ((status_ptr->association_status != 0xFF) &&
		(bsscfg->current_bss->flags2 & WLC_BSS_VHT) &&
		!(bsscfg->current_bss->flags2 & WLC_BSS_OPER_MODE))
		status_ptr->association_status |=
			WL_MIMO_PS_STATUS_ASSOC_STATUS_VHT_WITHOUT_OMN;
	status_ptr->mimo_ps_state = bsscfg_mimo_ps_cfg->cfg_update_in_progress;
	status_ptr->mrc_state = (bsscfg_mimo_ps_cfg->mrc_override) ?
			WL_MIMO_PS_STATUS_MRC_ACTIVE : WL_MIMO_PS_STATUS_MRC_NONE;
	status_ptr->bss_rxchain = bsscfg_hw_cfg->current_rxchains;
	status_ptr->bss_txchain = bsscfg_hw_cfg->current_txchains;
	status_ptr->bss_bw =
		wlc_stf_mimo_ps_chanspec2iovarbw(bsscfg_hw_cfg->current_chanspec_bw);
	err = wlc_stf_mimo_ps_status_get_hw_state(wlc, &status_ptr->hw_state);
	status_ptr->hw_rxchain = wlc->stf->rxchain;
	status_ptr->hw_txchain = wlc->stf->txchain;
	status_ptr->hw_bw = wlc_stf_mimo_ps_chanspec2iovarbw(wlc->chanspec);

	if (!D11REV_GE(wlc->pub->corerev, 43)) {
		status_ptr->pm_bcnrx_state =
				WL_MIMO_PS_STATUS_MHF_FLAG_INVALID;
	} else if (!si_iscoreup(wlc->pub->sih)) {
		status_ptr->pm_bcnrx_state =
				WL_MIMO_PS_STATUS_MHF_FLAG_COREDOWN;
	} else {
		mhfval = wlc_mhf_get(wlc, MHF3, WLC_BAND_AUTO);

		if (mhfval & MHF3_PM_BCNRX) {
			status_ptr->pm_bcnrx_state =
					WL_MIMO_PS_STATUS_MHF_FLAG_ACTIVE;
		} else {
			status_ptr->pm_bcnrx_state =
					WL_MIMO_PS_STATUS_MHF_FLAG_NONE;
		}

		if (mhfval & MHF3_SISO_BCMC_RX) {
			status_ptr->siso_bcmc_rx_state =
					WL_MIMO_PS_STATUS_MHF_FLAG_ACTIVE;
		} else {
			status_ptr->siso_bcmc_rx_state =
					WL_MIMO_PS_STATUS_MHF_FLAG_NONE;
		}
	}
	status_ptr->basic_rates_present = bsscfg_hw_cfg->basic_rates_present;
	return err;
}

static
void wlc_stf_mimo_ps_cfg_convert_params(wlc_bsscfg_t *bsscfg,
	wl_mimops_cfg_t *mimo_ps_cfg, uint8 *mode, int *rx_chains, uint16 *chanspec)
{
	wlc_info_t* wlc = bsscfg->wlc;
	wlc_hw_config_t *bsscfg_hw_cfg = wlc_stf_bss_hw_cfg_get(bsscfg);

	/* convert the no of rx chain based on input */
	if (mimo_ps_cfg->active_chains == 1)
		*rx_chains = 1;
	else
		*rx_chains = WLC_BITSCNT(wlc->stf->hw_rxchain); /* set to maximum */

	if (mimo_ps_cfg->bandwidth == MIMO_PS_CFG_IOVAR_BW_FULL)
		*chanspec = bsscfg_hw_cfg->original_chanspec_bw & WL_CHANSPEC_BW_MASK;
	else if (mimo_ps_cfg->bandwidth == MIMO_PS_CFG_IOVAR_BW_20M)
		*chanspec = WL_CHANSPEC_BW_20;
	else if (mimo_ps_cfg->bandwidth == MIMO_PS_CFG_IOVAR_BW_40M)
		*chanspec = WL_CHANSPEC_BW_40;
	else if (mimo_ps_cfg->bandwidth == MIMO_PS_CFG_IOVAR_BW_80M)
		*chanspec = WL_CHANSPEC_BW_80;
	else if (mimo_ps_cfg->bandwidth == MIMO_PS_CFG_IOVAR_BW_8080M)
		*chanspec = WL_CHANSPEC_BW_8080;
	else if (mimo_ps_cfg->bandwidth == MIMO_PS_CFG_IOVAR_BW_160M)
		*chanspec = WL_CHANSPEC_BW_160;
	/* Convert the input mimo ps mode to internal interpretation */
	if (mimo_ps_cfg->mode == HT_CAP_MIMO_PS_ON)
		*mode = HT_CAP_MIMO_PS_ON;
	else if (mimo_ps_cfg->mode == HT_CAP_MIMO_PS_RTS)
		*mode = HT_CAP_MIMO_PS_RTS;
	else
		*mode = HT_CAP_MIMO_PS_OFF;
}

static int
wlc_stf_mimo_ps_handle_identical_config(wlc_bsscfg_t *bsscfg,
	wl_mimops_cfg_t *mimo_ps_cfg)
{
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg = wlc_stf_mimo_ps_cfg_get(bsscfg);

	WL_STF_MIMO_PS_TRACE((" WLC STF MIMO PS : identical update\n"));
	if (mimo_ps_cfg->applychangesafterlearning == 0) {
		/* Start learning again */
		wlc_stf_update_mimo_ps_learning_update(bsscfg,
			WL_MIMO_PS_PS_LEARNING_CFG_ABORT);
		wlc_stf_start_learning(bsscfg);
		return	BCME_OK;
	}
	if (bsscfg_mimo_ps_cfg->cfg_update_in_progress == MIMO_PS_CFG_STATE_NONE) {
		wlc_stf_set_mimo_ps_cfg_to_hw(bsscfg);
	} else if (bsscfg_mimo_ps_cfg->cfg_update_in_progress ==
		MIMO_PS_CFG_STATE_INFORM_AP_INPROGRESS) {
		/* Just update the parameters and apply changes */
		/* In this case we update the parameters and wait for AP ACK */
	} else if (bsscfg_mimo_ps_cfg->cfg_update_in_progress == MIMO_PS_CFG_STATE_LEARNING) {
		wlc_stf_update_mimo_ps_learning_update(bsscfg,
			WL_MIMO_PS_PS_LEARNING_CFG_ABORT);
		wlc_stf_set_mimo_ps_cfg_to_hw(bsscfg);
	}
	return BCME_OK;
}
/* The following function handles the mimo power save config */
/* 1>> validation of parameters */
/* 2>> Send the mimo_ps mode to AP */
/* 3>> if 11 ac AP send operating mode notification based on */
/*     RX chain and bandwidth changed */
/* 4>> If bandwidth changed then send the bandwidth notification */
/* If the mode is Disabled then these rxchain and bw needs to be */
/* used for attach and disassoc */

int
wlc_stf_mimo_ps_cfg(wlc_bsscfg_t *bsscfg, wl_mimops_cfg_t *mimo_ps_cfg)
{
	wlc_info_t* wlc = bsscfg->wlc;
	int new_rx_chains;
	uint8 new_mimops_mode;
	uint16 new_chanspec_bw;
	int old_rx_chains;
	uint8 old_mimops_mode = wlc_stf_mimops_get(wlc);
	uint16 old_chanspec_bw;
	int8 bw = MIMO_PS_CFG_IOVAR_BW_FULL;
	struct scb *scb;
	bool transmit_mimo_ps_frame = TRUE;
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg = wlc_stf_mimo_ps_cfg_get(bsscfg);
	wlc_hw_config_t *bsscfg_hw_cfg = wlc_stf_bss_hw_cfg_get(bsscfg);

	if (!BSSCFG_STA(bsscfg)) {
		return BCME_NOTSTA;
	}
	if (bsscfg->associated || bsscfg->assoc->state != AS_IDLE) {
		return BCME_ASSOCIATED;
	}

	WL_STF_MIMO_PS_INFO((
		" WLC STF MIMO PS : calling mimo_ps IOvar chains %d mode %d\n",
		mimo_ps_cfg->active_chains, mimo_ps_cfg->mode));

	old_rx_chains = bsscfg_hw_cfg->default_rxchains;
	old_chanspec_bw = bsscfg_hw_cfg->original_chanspec_bw & WL_CHANSPEC_BW_MASK;

	/* AP is in SISO MODE know from beacon
	* After waking up if the host configures
	* mimo_ps_cfg to be in 3 chain mode return OK
	*/
	if ((bsscfg_hw_cfg->onlySISO == TRUE) && (mimo_ps_cfg->active_chains > 1)) {
		return BCME_OK;
	}
	/* Reset the mrc_override when setting mimo_ps_cfg iovar */
	if (bsscfg_mimo_ps_cfg->mrc_override == TRUE) {
		bsscfg_mimo_ps_cfg->mrc_override = FALSE;
		wlc_stf_mimo_ps_mrc_handling(bsscfg);
		WL_STF_MIMO_PS_INFO(
			(" WLC STF MIMO PS : mimo_ps_cfg override MRC is disabled"));
	}

	/* Get the new parameters converted to internal driver representation */
	wlc_stf_mimo_ps_cfg_convert_params(bsscfg, mimo_ps_cfg, &new_mimops_mode,
		&new_rx_chains, &new_chanspec_bw);

	/* Get the old parameters converted to internal driver representation */
	/* If there is no old configuration we use the defaults */
	if (bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params_valid == TRUE) {
		wlc_stf_mimo_ps_cfg_convert_params(bsscfg,
			bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params,
			&old_mimops_mode, &old_rx_chains, &old_chanspec_bw);
	}

	/*  Send Action frames AP if in associated state */
	if (bsscfg->associated) {
		/* Find the SCB for the current BSS */
		scb = wlc_scbfindband(wlc, bsscfg, &bsscfg->BSSID,
			CHSPEC_WLCBANDUNIT(bsscfg->current_bss->chanspec));

		/* If not a VHT AP, cannot handle bandwidth change -- override such request */
		if (!(SCB_OPER_MODE_NOTIF_CAP(scb) && SCB_VHT_CAP(scb))) {
			mimo_ps_cfg->bandwidth = MIMO_PS_CFG_IOVAR_BW_FULL;
			new_chanspec_bw = bsscfg_hw_cfg->original_chanspec_bw
			                                 & WL_CHANSPEC_BW_MASK;
		}

		/* Check for reconfig of identical parameters (force immediate restart) */
		if ((old_rx_chains == new_rx_chains) && (new_chanspec_bw == old_chanspec_bw) &&
		    (old_mimops_mode == new_mimops_mode)) {
			/* backup configured parameters to use on return from MRC */
			memcpy(bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params,
			       mimo_ps_cfg, sizeof(wl_mimops_cfg_t));
			bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params_valid = TRUE;
			/* This can be to apply changes or re-run smps learning */
			return wlc_stf_mimo_ps_handle_identical_config(bsscfg, mimo_ps_cfg);
		}

		/* If a command is already in progress ignore */
		if (bsscfg_mimo_ps_cfg->cfg_update_in_progress != MIMO_PS_CFG_STATE_NONE)
			return BCME_BUSY;

		/* backup configured parameters to use on return from MRC */
		memcpy(bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params,
		       mimo_ps_cfg, sizeof(wl_mimops_cfg_t));
		bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params_valid = TRUE;

		/* Update the flag if mimo ps mode is changed */
		transmit_mimo_ps_frame = (old_mimops_mode != new_mimops_mode);

		/* If AP does not support HT or VHT donot handle */
		if (!(SCB_HT_CAP(scb) || SCB_VHT_CAP(scb))) {
			/* This can be legacy AP just set the */
			/* parameters as desired */
			WL_STF_MIMO_PS_TRACE((" WLC STF MIMO PS : No HT/VHT cap %x(hex)\n",
				bsscfg->oper_mode));
			wlc_stf_set_mimo_ps_cfg_to_hw(bsscfg);
			return BCME_OK;
		}
		bsscfg_mimo_ps_cfg->message_map_to_inform = 0;
#if defined(WL11AC)
		if (SCB_OPER_MODE_NOTIF_CAP(scb) && SCB_VHT_CAP(scb)) {
			/* Handle bandwidth related changes */
			/* Send VHT operation IE in case its a 11AC AP */
			if ((old_rx_chains != new_rx_chains) ||
				(old_chanspec_bw != new_chanspec_bw)) {
				/* Convert BW to the correct format */
				if (new_chanspec_bw == WL_CHANSPEC_BW_20)
					bw = DOT11_OPER_MODE_20MHZ;
				else if (new_chanspec_bw == WL_CHANSPEC_BW_40)
					bw = DOT11_OPER_MODE_40MHZ;
				else if (new_chanspec_bw == WL_CHANSPEC_BW_80)
					bw = DOT11_OPER_MODE_80MHZ;
				else if (new_chanspec_bw == WL_CHANSPEC_BW_8080)
					bw = DOT11_OPER_MODE_8080MHZ;
				else if (new_chanspec_bw == WL_CHANSPEC_BW_160)
					bw = DOT11_OPER_MODE_160MHZ;
				/* create the oper mode based on the values */
				bsscfg->oper_mode =
					DOT11_OPER_MODE(0, new_rx_chains, bw);
				bsscfg->oper_mode_enabled = TRUE;
				/* VHT send oper mode sets the correct BW CAP from PHY */
				bsscfg_mimo_ps_cfg->opermode_to_inform = bsscfg->oper_mode;
				bsscfg_mimo_ps_cfg->cfg_update_in_progress =
					MIMO_PS_CFG_STATE_INFORM_AP_PENDING;
				bsscfg_mimo_ps_cfg->message_map_to_inform |=
					SEND_OPER_MODE_NOTIFICATION;
				/* If rx chain did not change tranmit mimops if changed */
				if (old_rx_chains != new_rx_chains)
					transmit_mimo_ps_frame = FALSE;
			}
		} else
#endif /* defined(WL11AC) */
		{ /* Handle a non 11 ac ap */
		}
		/* Handle mimo powersave related action frame  */
		if (transmit_mimo_ps_frame == TRUE) {
			/* Send the action frame with the MIMO PS mode */
#ifndef WLC_NET80211
			wlc_ht_set_cfg_mimops_PM(wlc->hti, wlc->cfg, new_mimops_mode);
			/* wlc->cfg->mimops_PM = new_mimops_mode; */
#endif /* WLC_NET80211 */
			bsscfg_mimo_ps_cfg->mimo_ps_mode_to_inform = bw;
			bsscfg_mimo_ps_cfg->cfg_update_in_progress =
				MIMO_PS_CFG_STATE_INFORM_AP_PENDING;
			bsscfg_mimo_ps_cfg->message_map_to_inform |= SEND_SMPS_ACTION_NOTIFICATION;
		}
		wlc_stf_handle_mimo_ps_action_frames(bsscfg);
	} else {
		bsscfg_hw_cfg->default_rxchains = mimo_ps_cfg->active_chains;
		bsscfg_hw_cfg->default_txchains = mimo_ps_cfg->active_chains;
		bsscfg_hw_cfg->current_rxchains = mimo_ps_cfg->active_chains;
		bsscfg_hw_cfg->current_txchains = mimo_ps_cfg->active_chains;
		bsscfg_hw_cfg->last_ntxchains = 0;
		WL_STF_MIMO_PS_INFO(
			("\n wlc_stf_mimo_ps_cfg -> arbitrator rxchain %d txchains %d \n",
				bsscfg_hw_cfg->current_rxchains,
			    bsscfg_hw_cfg->current_txchains));
		bsscfg_hw_cfg->default_chanspec_bw = mimo_ps_cfg->bandwidth;
		if (mimo_ps_cfg->bandwidth == MIMO_PS_CFG_IOVAR_BW_FULL) {
			bsscfg_hw_cfg->chanspec_override = FALSE;
		} else {
			bsscfg_hw_cfg->chanspec_override = TRUE;
		}
		wlc_update_mimops_cfg_transition(wlc);
	}
	/* backup the configured parameters so that we can use when we come back from MRC */
	memcpy(bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params,
		mimo_ps_cfg, sizeof(wl_mimops_cfg_t));
	bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params_valid = TRUE;
	return BCME_OK;
}

void wlc_stf_handle_mimo_ps_action_frames(wlc_bsscfg_t *bsscfg)
{
	wlc_info_t* wlc = bsscfg->wlc;
	bool currChanspecActive = FALSE;
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg = wlc_stf_mimo_ps_cfg_get(bsscfg);

	if ((wf_chspec_ctlchan(WLC_BAND_PI_RADIO_CHANSPEC)) ==
		wf_chspec_ctlchan(wlc->cfg->current_bss->chanspec))
			currChanspecActive = TRUE;
	WL_STF_MIMO_PS_TRACE((" WLC STF MIMO PS : coming to %x(hex)\n",
		bsscfg->oper_mode));

	if (!bsscfg->associated) {
		WL_STF_MIMO_PS_TRACE((" WLC STF MIMO PS : bail out 1 %x(hex)\n",
			bsscfg->oper_mode));
		return;
	}
	if (bsscfg_mimo_ps_cfg->cfg_update_in_progress != MIMO_PS_CFG_STATE_INFORM_AP_PENDING) {
		WL_STF_MIMO_PS_TRACE((" WLC STF MIMO PS : bail out 2 %x(hex)\n",
			bsscfg->oper_mode));
		return;
	}
	if (currChanspecActive == TRUE &&
		bsscfg_mimo_ps_cfg->cfg_update_in_progress ==
			MIMO_PS_CFG_STATE_INFORM_AP_PENDING) {

#ifdef WL11AC
		if ((bsscfg_mimo_ps_cfg->message_map_to_inform & SEND_OPER_MODE_NOTIFICATION) ==
			SEND_OPER_MODE_NOTIFICATION) {
			bsscfg->oper_mode = bsscfg_mimo_ps_cfg->opermode_to_inform;
			bsscfg->oper_mode_enabled = TRUE;
			WL_STF_MIMO_PS_TRACE(
				(" WLC STF MIMO PS : Sending VHT Operation Mode: Value %x(hex)",
				bsscfg->oper_mode));
			wlc_send_action_vht_oper_mode(wlc->vhti,
				bsscfg, &bsscfg->BSSID, TRUE);
			bsscfg_mimo_ps_cfg->message_map_to_inform &= ~SEND_OPER_MODE_NOTIFICATION;
		}
#endif /* WL11AC */
		if ((bsscfg_mimo_ps_cfg->message_map_to_inform &
			SEND_CH_WIDTH_ACTION_NOTIFICATION) == SEND_CH_WIDTH_ACTION_NOTIFICATION) {
			WL_STF_MIMO_PS_TRACE(
			(" WLC STF MIMO PS : Sending Channel width notification: Value %x(hex)",
				bsscfg_mimo_ps_cfg->chwidth_to_inform));
			wlc_ch_width_action_ht_send(wlc, wlc->cfg,
			                            bsscfg_mimo_ps_cfg->chwidth_to_inform);
			bsscfg_mimo_ps_cfg->message_map_to_inform &=
			                            ~SEND_CH_WIDTH_ACTION_NOTIFICATION;
		}
		if ((bsscfg_mimo_ps_cfg->message_map_to_inform & SEND_SMPS_ACTION_NOTIFICATION) ==
		SEND_SMPS_ACTION_NOTIFICATION) {
#ifndef WLC_NET80211
			wlc_ht_set_cfg_mimops_PM(wlc->hti, wlc->cfg,
				bsscfg_mimo_ps_cfg->mimo_ps_mode_to_inform);
			/* wlc->cfg->mimops_PM = bsscfg->mimo_ps_cfg->mimo_ps_mode_to_inform; */
#endif /* WLC_NET80211 */
			WL_STF_MIMO_PS_TRACE(
				(" WLC STF MIMO PS : Sending SMPS action frame: Value %x(hex)",
				bsscfg_mimo_ps_cfg->mimo_ps_mode_to_inform));
			wlc_mimops_action_ht_send(wlc->hti, wlc->cfg,
				bsscfg_mimo_ps_cfg->mimo_ps_mode_to_inform);
			bsscfg_mimo_ps_cfg->message_map_to_inform &=
				~SEND_SMPS_ACTION_NOTIFICATION;
		}
	}
	if (bsscfg_mimo_ps_cfg->message_map_to_inform == 0) {
		WL_STF_MIMO_PS_TRACE(
			(" WLC STF MIMO PS : Message map is NULL so start timer"));
		bsscfg_mimo_ps_cfg->cfg_update_in_progress =
			MIMO_PS_CFG_STATE_INFORM_AP_INPROGRESS;
		wlc_stf_cfg_timer_start(bsscfg);
	}
	WL_STF_MIMO_PS_TRACE((" WLC STF MIMO PS : bail out 3 %x(hex)\n",
		bsscfg->oper_mode));
}

static void wlc_stf_mrc_clean_up(wlc_bsscfg_t *bsscfg)
{
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg = wlc_stf_mimo_ps_cfg_get(bsscfg);

	WL_STF_MIMO_PS_INFO(("\n wlc_stf_mrc_clean_up -> Calling cleanup \n"));
	bsscfg_mimo_ps_cfg->mrc_override = FALSE;
	bsscfg_mimo_ps_cfg->mrc_chains_changed = FALSE;
}

void wlc_stf_mimo_ps_clean_up(wlc_bsscfg_t *bsscfg, uint8 reason)
{
	wlc_info_t* wlc;
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg;
	wlc_hw_config_t *bsscfg_hw_cfg;
	int rx_chains;
	int tx_chains;

	if (bsscfg == NULL)
		return;
	wlc = bsscfg->wlc;

	bsscfg_hw_cfg = wlc_stf_bss_hw_cfg_get(bsscfg);
	bsscfg_mimo_ps_cfg = wlc_stf_mimo_ps_cfg_get(bsscfg);

	if (bsscfg_mimo_ps_cfg == NULL || bsscfg_hw_cfg == NULL)
		return;

	WL_STF_MIMO_PS_TRACE(("MIMO_PS_Clean_Up, Reason : %d\n",
		reason));

	rx_chains = bsscfg_hw_cfg->default_rxchains;
	tx_chains = bsscfg_hw_cfg->default_txchains;
	bsscfg_hw_cfg->onlySISO = FALSE;
	bsscfg_hw_cfg->current_rxchains = rx_chains;
	bsscfg_hw_cfg->current_txchains = tx_chains;
	bsscfg_hw_cfg->last_ntxchains = 0;
	wlc_update_mimops_cfg_transition(wlc);
	bsscfg_mimo_ps_cfg->cfg_update_in_progress = MIMO_PS_CFG_STATE_NONE;
	wlc_stf_update_mimo_ps_learning_update(bsscfg, WL_MIMO_PS_PS_LEARNING_CFG_ABORT);
	bsscfg_mimo_ps_cfg->mimo_ps_learning_running = FALSE;
	wlc_stf_mrc_clean_up(bsscfg);
}

static
void wlc_stf_set_mimo_ps_cfg_to_hw(wlc_bsscfg_t *bsscfg)
{
	wlc_info_t* wlc = bsscfg->wlc;
	uint16 new_chspec;
	uint16 currchspec_bw = WL_CHANSPEC_BW_80;
	uint16 newchspec_bw = WL_CHANSPEC_BW_80;
	uint16 mimo_ps_bw;
	int rx_chains;
	int tx_chains;
	uint8 mode;
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg;
	wlc_hw_config_t *wlc_cfg_hw_cfg;
	wlc_hw_config_t *bsscfg_hw_cfg;

	bsscfg_mimo_ps_cfg = wlc_stf_mimo_ps_cfg_get(bsscfg);
	wlc_cfg_hw_cfg = wlc_stf_bss_hw_cfg_get(wlc->cfg);
	bsscfg_hw_cfg = wlc_stf_bss_hw_cfg_get(bsscfg);

	mimo_ps_bw = bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params->bandwidth;

	bsscfg_mimo_ps_cfg->cfg_update_in_progress = MIMO_PS_CFG_STATE_HW_CONFIGURE;
	WL_STF_MIMO_PS_INFO(
		("\n MIMO PS - MIMO_PS_CFG_STATE_HW_CONFIGURE \n"));
	/* Reset the learning flag if set */
	MIMO_PS_CLEAR_LEARNING_FLAG(bsscfg_mimo_ps_cfg);
	/* Apply bandwidth related change */
	if (!wlc->cfg->associated)
		return;
	currchspec_bw = wlc->cfg->current_bss->chanspec & WL_CHANSPEC_BW_MASK;
	newchspec_bw = wlc_cfg_hw_cfg->original_chanspec_bw & WL_CHANSPEC_BW_MASK;
	WL_STF_MIMO_PS_INFO((
	" WLC STF MIMO PS : Applying changes as configured in mimo_ps_Cfg IOVAR"));
	/* Update the mimo ps mode */
	/* Convert the input mimo ps mode to internal interpretation */
	if (bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params->mode == HT_CAP_MIMO_PS_ON) {
		mode = HT_CAP_MIMO_PS_ON;
	} else if (bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params->mode == HT_CAP_MIMO_PS_RTS)
		mode = HT_CAP_MIMO_PS_RTS;
	else
		mode = HT_CAP_MIMO_PS_OFF;

#ifndef WLC_NET80211
	wlc_ht_set_cfg_mimops_PM(wlc->hti, wlc->cfg, mode);
	/* wlc->cfg->mimops_PM = mode; */
#endif /* WLC_NET80211 */
	/* Update the MIMO PS capabilites */
	wlc_ht_mimops_cap_update(wlc->hti, mode);
	/* convert the no of rx chain based on input */
	if (bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params->active_chains == 1) {
		rx_chains = 1;
		tx_chains = 1;
	}
	else {
		rx_chains = wlc->stf->hw_rxchain;
		tx_chains = wlc->stf->hw_txchain;
	}
	bsscfg_hw_cfg->current_rxchains = rx_chains;
	bsscfg_hw_cfg->current_txchains = tx_chains;

	newchspec_bw = wlc_stf_mimo_ps_iovarbw2chanspecbw(wlc->cfg, mimo_ps_bw);
	if (currchspec_bw != newchspec_bw) {
		new_chspec = wf_channel2chspec(wf_chspec_ctlchan(wlc->cfg->current_bss->chanspec),
			newchspec_bw);
		bsscfg_hw_cfg->current_chanspec_bw = new_chspec;
		wlc_cfg_hw_cfg->chanspec_override = TRUE;
	}
	if (mimo_ps_bw == MIMO_PS_CFG_IOVAR_BW_FULL) {
		wlc_cfg_hw_cfg->chanspec_override = FALSE;
	}
	bsscfg_mimo_ps_cfg->cfg_update_in_progress = MIMO_PS_CFG_STATE_NONE;
	WL_STF_MIMO_PS_INFO(
		("\n wlc_stf_set_mimo_ps_cfg_to_hw -> arbitrator rxchain %d txchains %d \n",
			bsscfg_hw_cfg->current_rxchains, bsscfg_hw_cfg->current_txchains));
	wlc_update_mimops_cfg_transition(wlc);
}

static void wlc_stf_start_learning(wlc_bsscfg_t *bsscfg)
{
	wlc_info_t* wlc = bsscfg->wlc;
	uint16 new_chspec = WL_CHANSPEC_BW_80;
	uint16 newchspec_bw = WL_CHANSPEC_BW_80;
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg = wlc_stf_mimo_ps_cfg_get(bsscfg);
	uint16 mimo_ps_bw = bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params->bandwidth;
	wlc_hw_config_t *bsscfg_hw_cfg = wlc_stf_bss_hw_cfg_get(bsscfg);
	wlc_hw_config_t *wlc_cfg_hw_cfg = wlc_stf_bss_hw_cfg_get(wlc->cfg);


	/* If learning is configured start it when we downgrade to 1 chain */

	/* Set the flag that learning is started; reset counters */
	MIMO_PS_SET_LEARNING_FLAG(bsscfg_mimo_ps_cfg);
	MIMO_PS_RESET_LEARNING_PARAMS(bsscfg_mimo_ps_cfg);

	/* Store the current BSSID (that we're learning from) */
	memset(&bsscfg_mimo_ps_cfg->mimo_ps_learning_data.BSSID, 0, sizeof(struct ether_addr));
	if (BSSCFG_STA(bsscfg) && bsscfg->associated && bsscfg->BSS) {
		memcpy(&bsscfg_mimo_ps_cfg->mimo_ps_learning_data.BSSID,
		       &bsscfg->BSSID, sizeof(struct ether_addr));
	}

	/* Set the time marking the start of the learning */
	bsscfg_mimo_ps_cfg->mimo_ps_learning_data.startTimeStamp = OSL_SYSUPTIME();
	bsscfg_mimo_ps_cfg->cfg_update_in_progress = MIMO_PS_CFG_STATE_LEARNING;
	bsscfg_hw_cfg->current_txchains = 1;
	WL_STF_MIMO_PS_INFO(
		("\n wlc_stf_start_learning -> arbitrator rxchain %d txchains %d \n",
		bsscfg_hw_cfg->current_rxchains, bsscfg_hw_cfg->current_txchains));
	wlc_update_mimops_cfg_transition(wlc);
	newchspec_bw = wlc_stf_mimo_ps_iovarbw2chanspecbw(wlc->cfg, mimo_ps_bw);
	new_chspec = wf_channel2chspec(wf_chspec_ctlchan(wlc->cfg->current_bss->chanspec),
		newchspec_bw);
	bsscfg_hw_cfg->current_chanspec_bw = new_chspec;
	wlc_cfg_hw_cfg->chanspec_override = TRUE;
	wlc_update_bandwidth(wlc, wlc->cfg, new_chspec);
	WL_STF_MIMO_PS_INFO(
		("\n MIMO PS - MIMO_PS_CFG_STATE_LEARNING - Started \n"));
}

void wlc_stf_cfg_change_timer_expiry(void *arg)
{
	wlc_bsscfg_t* bsscfg = (wlc_bsscfg_t*)arg;
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg = wlc_stf_mimo_ps_cfg_get(bsscfg);


	if (bsscfg_mimo_ps_cfg->mimo_ps_smps_no_of_packets != 0 &&
		bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params->active_chains == 1 &&
			bsscfg_mimo_ps_cfg->mimo_ps_learning_running == FALSE) {
				wlc_stf_start_learning(bsscfg);
				return;
	}
	/* Reset the learning flag if set */
	MIMO_PS_CLEAR_LEARNING_FLAG(bsscfg_mimo_ps_cfg);
	if (bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params_valid &&
		bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params->applychangesafterlearning == 1)
			wlc_stf_set_mimo_ps_cfg_to_hw(bsscfg);
	bsscfg_mimo_ps_cfg->cfg_change_timer_active = FALSE;
	bsscfg_mimo_ps_cfg->cfg_update_in_progress = MIMO_PS_CFG_STATE_NONE;
	WL_STF_MIMO_PS_INFO(
		("\n MIMO PS - MIMO_PS_CFG_STATE_NONE - Completed \n"));
	WL_STF_MIMO_PS_TRACE((" WLC STF MIMO PS : mimo_ps_cfg iovar SUCCESSFUL "));
}

void wlc_stf_cfg_timer_start(wlc_bsscfg_t *bsscfg)
{
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg = wlc_stf_mimo_ps_cfg_get(bsscfg);

	if (bsscfg_mimo_ps_cfg->cfg_change_timer_active == FALSE &&
		bsscfg_mimo_ps_cfg->cfg_update_in_progress != MIMO_PS_CFG_STATE_NONE) {


			if (bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params->active_chains != 1) {
				/* donot start timer if we are moving to 1 chain */
				/* Apply the configuration immediately */
				WL_STF_MIMO_PS_INFO((
				"\n TIMER NOT STARTED as active chains is not 1 \n"));
				wlc_stf_cfg_change_timer_expiry(bsscfg);
				return;
			}
			wl_add_timer(bsscfg->wlc->wl, bsscfg_mimo_ps_cfg->cfg_change_timer,
			bsscfg_mimo_ps_cfg->mimo_ps_cfg_change_wait_time, FALSE);
			bsscfg_mimo_ps_cfg->cfg_change_timer_active = TRUE;
			WL_STF_MIMO_PS_TRACE(
				(" WLC STF MIMO PS : Starting Guard interval timer: timer value %d",
				bsscfg_mimo_ps_cfg->mimo_ps_cfg_change_wait_time));
			bsscfg_mimo_ps_cfg->cfg_update_in_progress =
				MIMO_PS_CFG_STATE_INFORM_AP_DONE;
			WL_STF_MIMO_PS_INFO(
				("\n MIMO PS - MIMO_PS_CFG_STATE_INFORM_AP_DONE \n"));

	}
}

/* The follwing function handles the packet counting for SMPS learning */
/* It derives the no of spatial streams from the rspec and count */
/* If the learning is complete then either apply the configured changes or end learning */
void wlc_stf_update_mimo_ps_cfg_data(wlc_bsscfg_t *bsscfg, ratespec_t rspec)
{
	int nss = WL_RX_NSS_1;
	uint totalrx = 0;
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg = wlc_stf_mimo_ps_cfg_get(bsscfg);


	if (!WLC_STF_LEARNING_ENABLED(bsscfg_mimo_ps_cfg))
		return;
	/* Get the number of Spatial streams based on the rspec of the packet */
	nss = wlc_ratespec_nss(rspec);
	/* Update the relevant fields based on the RSSI and NSS */
	if (bsscfg->link->rssi >=
			(bsscfg_mimo_ps_cfg->mimo_ps_learning_rssi_threshold)) {
		if (nss == WL_RX_NSS_1)
		       bsscfg_mimo_ps_cfg->mimo_ps_learning_data.totalSISO_above_rssi_threshold ++;
		else
		       bsscfg_mimo_ps_cfg->mimo_ps_learning_data.totalMIMO_above_rssi_threshold ++;
	} else {
		if (nss == WL_RX_NSS_1)
		       bsscfg_mimo_ps_cfg->mimo_ps_learning_data.totalSISO_below_rssi_threshold ++;
		else
		       bsscfg_mimo_ps_cfg->mimo_ps_learning_data.totalMIMO_below_rssi_threshold ++;
	}

	/* If the total rx reaches the number of packets to be tracked ?? */
	totalrx = bsscfg_mimo_ps_cfg->mimo_ps_learning_data.totalSISO_above_rssi_threshold +
			bsscfg_mimo_ps_cfg->mimo_ps_learning_data.totalMIMO_above_rssi_threshold +
			bsscfg_mimo_ps_cfg->mimo_ps_learning_data.totalSISO_below_rssi_threshold +
			bsscfg_mimo_ps_cfg->mimo_ps_learning_data.totalMIMO_below_rssi_threshold;

	/* If total requested packets have been learnt */
	/* then apply or stop learning and send the event */
	if (bsscfg_mimo_ps_cfg->mimo_ps_smps_no_of_packets <= totalrx) {
		WL_STF_MIMO_PS_INFO((
			" WLC STF MIMO PS : SMPS learning - configured packets learnt "));
		/* Mark the end time of SMPS learning */
		bsscfg_mimo_ps_cfg->mimo_ps_learning_data.endTimeStamp = OSL_SYSUPTIME();
		/* Send the event to the host that the learning is completed */
		bsscfg_mimo_ps_cfg->mimo_ps_learning_data.reason =
			WL_MIMO_PS_PS_LEARNING_COMPLETED;
		wlc_stf_update_mimo_ps_learning_update(bsscfg, WL_MIMO_PS_PS_LEARNING_COMPLETED);
		/* If host has asked to apply the configuration after learning ??? */
		if (bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params->applychangesafterlearning) {
			WL_STF_MIMO_PS_INFO((
			" WLC STF MIMO PS : SMPS learning - Applying changes after learning "));
			/* Apply changes after learning by calling the exipry routine */
			wlc_stf_cfg_change_timer_expiry(bsscfg);
			wlc_stf_set_mimo_ps_cfg_to_hw(bsscfg);
		} else {
			WL_STF_MIMO_PS_INFO((
			" WLC STF MIMO PS : SMPS learning - NOT Applying changes after learning "));
			/* Reset flags if the changes are not requested */
			WL_STF_MIMO_PS_INFO(
				("\n MIMO PS - MIMO_PS_CFG_STATE_LEARNING - Completed \n"));
		}
		wl_del_timer(bsscfg->wlc->wl, bsscfg_mimo_ps_cfg->cfg_change_timer);
		bsscfg_mimo_ps_cfg->cfg_change_timer_active = FALSE;
		bsscfg_mimo_ps_cfg->cfg_update_in_progress = MIMO_PS_CFG_STATE_NONE;
		MIMO_PS_CLEAR_LEARNING_FLAG(bsscfg_mimo_ps_cfg);
	}
}

/* The following function Will update the learning mode */
/* will abort if requested for */
/* And then send the mac event to the host */
void wlc_stf_update_mimo_ps_learning_update(wlc_bsscfg_t *bsscfg, uint8 reason)
{
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg = wlc_stf_mimo_ps_cfg_get(bsscfg);

	if (reason == WL_MIMO_PS_PS_LEARNING_CFG_ABORT) {
		WL_STF_MIMO_PS_INFO((
		" WLC STF MIMO PS : SMPS learning - ABORT recieved"));
		if (bsscfg_mimo_ps_cfg->mimo_ps_learning_running == TRUE) {
			bsscfg_mimo_ps_cfg->mimo_ps_learning_data.reason = reason;
			bsscfg_mimo_ps_cfg->mimo_ps_learning_data.endTimeStamp = OSL_SYSUPTIME();
		}
		wl_del_timer(bsscfg->wlc->wl, bsscfg_mimo_ps_cfg->cfg_change_timer);
		MIMO_PS_CLEAR_LEARNING_FLAG(bsscfg_mimo_ps_cfg);
		bsscfg_mimo_ps_cfg->cfg_change_timer_active = FALSE;
		bsscfg_mimo_ps_cfg->cfg_update_in_progress = MIMO_PS_CFG_STATE_NONE;
		WL_STF_MIMO_PS_INFO(
			("\n MIMO PS - MIMO_PS_CFG_STATE_LEARNING - Aborted \n"));
	}
	memcpy(&bsscfg_mimo_ps_cfg->mimo_ps_learning_data.BSSID,
		&bsscfg->BSSID, sizeof(struct ether_addr));
	WL_STF_MIMO_PS_INFO((
		" WLC STF MIMO PS : SMPS learning - Sending WLC_E_MIMO_PWR_SAVE event "));
	wlc_mac_event(bsscfg->wlc, WLC_E_MIMO_PWR_SAVE, NULL, 0, 0, 0,
		&bsscfg_mimo_ps_cfg->mimo_ps_learning_data,
			sizeof(wl_mimo_ps_learning_event_data_t));
}


void wlc_stf_mimo_ps_mrc_handling(wlc_bsscfg_t *bsscfg)
{
	wlc_info_t *wlc = bsscfg->wlc;
	uint8 mimo_ps_mode = wlc_ht_get_cfg_mimops_PM(wlc->hti, wlc->cfg);
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg = wlc_stf_mimo_ps_cfg_get(bsscfg);
	wlc_hw_config_t *bsscfg_hw_cfg = wlc_stf_bss_hw_cfg_get(bsscfg);


	if (bsscfg_mimo_ps_cfg->mrc_override == TRUE) {
		/* If the rx chain is 1 then make it max chains to gain from MRC */
		if (bsscfg_hw_cfg->current_rxchains == 1) {
			bsscfg_hw_cfg->current_rxchains = wlc->stf->hw_rxchain;
			bsscfg_hw_cfg->current_txchains = wlc->stf->hw_txchain;
			bsscfg_mimo_ps_cfg->mrc_chains_changed = TRUE;
		}
	}
	else if (bsscfg_mimo_ps_cfg->mrc_chains_changed) {
		bsscfg_hw_cfg->current_rxchains = 1;
		bsscfg_hw_cfg->current_txchains = 1;
		bsscfg_mimo_ps_cfg->mrc_chains_changed = FALSE;
	}
	WL_STF_MIMO_PS_INFO(
		("\n wlc_stf_mimo_ps_mrc_handling -> arbitrator rxchain %d txchains %d \n",
			bsscfg_hw_cfg->current_rxchains, bsscfg_hw_cfg->current_txchains));
	wlc_update_mimops_cfg_transition(wlc);

	/* If there was no chain change, make sure we don't miss bcnrx change */
	if (!bsscfg_mimo_ps_cfg->mrc_chains_changed) {
		wlc_info_t *wlc = bsscfg->wlc;

		if (PM_BCNRX_ENAB(wlc->pub)) {
			/* Counters depend on flag, so sync */
			wlc_mimo_siso_metrics_snapshot(wlc, FALSE,
				WL_MIMOPS_METRICS_SNAPSHOT_PMBCNRX);
			if (!wlc_pm_bcnrx_allowed(wlc))
				wlc_pm_bcnrx_set(wlc, FALSE); /* Disable */
			else
				wlc_pm_bcnrx_set(wlc, TRUE); /* Enable */
		}
	}

	/* Convert the input mimo ps mode to internal interpretation */
	if (bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params->mode == 0)
		mimo_ps_mode = HT_CAP_MIMO_PS_ON;
	else if (bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params->mode == 1)
		mimo_ps_mode = HT_CAP_MIMO_PS_RTS;
	else
		mimo_ps_mode = HT_CAP_MIMO_PS_OFF;
	/* Restore the mimo_ps config modified by wlc_stf_rxchain_cfg */
	wlc_ht_set_mimops_ActionPM(wlc, wlc->cfg, mimo_ps_mode);
	wlc_ht_set_cfg_mimops_PM(wlc->hti, wlc->cfg, mimo_ps_mode);
	wlc_ht_mimops_cap_update(wlc->hti, (uint8)mimo_ps_mode);
}

int wlc_stf_mrc_thresh_handling(wlc_bsscfg_t *bsscfg)
{
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg = wlc_stf_mimo_ps_cfg_get(bsscfg);

	/* Handle MIMO PS related changes here */
	if (bsscfg_mimo_ps_cfg->mrc_override == TRUE) {
		if (bsscfg->link->rssi >=
			(bsscfg_mimo_ps_cfg->mrc_rssi_threshold +
				bsscfg_mimo_ps_cfg->mrc_delta_threshold)) {
			bsscfg_mimo_ps_cfg->mrc_override = FALSE;
			wlc_stf_mimo_ps_mrc_handling(bsscfg);
			WL_STF_MIMO_PS_INFO(
				(" WLC STF MIMO PS : MRC is disabled RSSI %d MRC threshold %d",
				bsscfg->link->rssi, bsscfg_mimo_ps_cfg->mrc_rssi_threshold));
		}
	} else if (bsscfg->link->rssi < bsscfg_mimo_ps_cfg->mrc_rssi_threshold) {
		/* trigger MRC */
		bsscfg_mimo_ps_cfg->mrc_override = TRUE;
		wlc_stf_mimo_ps_mrc_handling(bsscfg);
		WL_STF_MIMO_PS_INFO(
			(" WLC STF MIMO PS : MRC is enabled RSSI %d MRC threshold %d",
			bsscfg->link->rssi, bsscfg_mimo_ps_cfg->mrc_rssi_threshold));
	}
	return BCME_OK;
}

#if defined(WL11AC) || defined(WL_MIMOPS_CFG)
static int8
wlc_stf_mimops_get(wlc_info_t* wlc)
{
	return (int8)((wlc_ht_get_cap(wlc->hti) & HT_CAP_MIMO_PS_MASK) >> HT_CAP_MIMO_PS_SHIFT);
}
#endif /* WL11AC || WL_MIMOPS_CFG */

#if defined(WL11AC)
bool
wlc_stf_mimops_set(wlc_info_t* wlc, wlc_bsscfg_t *cfg, int32 int_val)
{
	if ((int_val < 0) || (int_val > HT_CAP_MIMO_PS_OFF) || (int_val == 2)) {
		return FALSE;
	}
	wlc_ht_mimops_cap_update(wlc->hti, (uint8)int_val);
#ifndef WLC_NET80211
	if (cfg && cfg->associated)
		wlc_mimops_action_ht_send(wlc->hti, cfg, (uint8)int_val);
#endif /* !WLC_NET80211 */
	return TRUE;
}
#endif /* WL11AC */

void
wlc_stf_config_siso(wlc_bsscfg_t *bsscfg, bool config_siso)
{
	wlc_info_t* wlc = bsscfg->wlc;
	wlc_hw_config_t *bsscfg_hw_cfg = wlc_stf_bss_hw_cfg_get(bsscfg);

	if (config_siso == FALSE)
		return;
	/* restore the original rx chain */
	bsscfg_hw_cfg->current_txchains = 1;
	bsscfg_hw_cfg->current_rxchains = 1;
	WL_STF_MIMO_PS_INFO(
		("\n wlc_stf_config_siso -> arbitrator rxchain %d txchains %d \n",
			bsscfg_hw_cfg->current_rxchains, bsscfg_hw_cfg->current_txchains));
	wlc_update_mimops_cfg_transition(wlc);
}


/* Handle cfg's channel in and out transitions  */
void wlc_update_mimops_cfg_transition(wlc_info_t *wlc)
{
#ifdef WL_STF_ARBITRATOR
	uint8 state = WLC_STF_ARBITRATOR_REQ_STATE_RXTX_INACTIVE;
	wlc_hw_config_t *wlc_cfg_hw_cfg = wlc_stf_bss_hw_cfg_get(wlc->cfg);
	wlc_stf_nss_request_t *wlc_cfg_stf_arb_req = wlc_stf_bss_arb_req_get(wlc->cfg);

	if (wlc->cfg->enable && wlc_cfg_hw_cfg && wlc_cfg_stf_arb_req &&
		((wlc->cfg->associated) || (wlc->cfg->assoc->state != AS_IDLE))) {
		chanspec_t chan_spec = wlc->cfg->current_bss->chanspec;
		if (!wlc->cfg->associated)
			chan_spec = wlc->cfg->target_bss->chanspec;
		WL_STF_ARBITRATOR_TRACE(("cfg transition %d %x %x %d\n",
			wlc->cfg->associated, wlc->chanspec, chan_spec, wlc->cfg->assoc->state));
		/* Check if primary bss is going in and out of its operating channel */
		if ((wf_chspec_ctlchan(WLC_BAND_PI_RADIO_CHANSPEC)) ==
			wf_chspec_ctlchan(chan_spec))
			state = WLC_STF_ARBITRATOR_REQ_STATE_RXTX_ACTIVE;
		WL_STF_ARBITRATOR_TRACE(("cfg transition state %d; tx %d rx %d chains \n", state,
			wlc_cfg_hw_cfg->current_txchains, wlc_cfg_hw_cfg->current_rxchains));
		wlc_stf_nss_request_update(wlc, wlc_cfg_stf_arb_req, state,
			wlc_cfg_hw_cfg->current_txchains,
		    WLC_BITSCNT(wlc_cfg_hw_cfg->current_txchains),
			wlc_cfg_hw_cfg->current_rxchains,
		    WLC_BITSCNT(wlc_cfg_hw_cfg->current_rxchains));
	}
#endif /* WL_STF_ARBITRATOR */
}

static int
wlc_stf_mimo_ps_status_get_hw_state(wlc_info_t *wlc, uint16 *hw_state)
{
	int err = BCME_OK;
#if defined(WL_STF_ARBITRATOR)
	wlc_stf_nss_request_q_t *cur;
	*hw_state = WL_MIMO_PS_STATUS_HW_STATE_NONE;

	if (!wlc->stf->arb_req_q)
		return BCME_OK;

	/* iterate through the requests */
	for (cur = wlc->stf->arb_req_q; cur; cur = cur->next) {
		if (WLC_STF_ARBI_REQ_STATE_ACTIVE(cur->req)) {
			WL_STF_ARBITRATOR_TRACE(("MIMO_PS_STATUS: active req_id=%u\n",
				cur->req->req_id));
			*hw_state |= MIMO_PS_STATUS_REQ_TO_HW_STATE(cur->req->req_id);
		}
	}
	err = BCME_OK;
#else
	err = BCME_UNSUPPORTED;
#endif /* WL_STF_ARBITRATOR */
	return err;
}

static int
wlc_stf_mimo_ps_status_get_apcap(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint8 *ap_cap)
{
	int err = BCME_OK;
	struct scb *scb;
	struct ether_addr *bssid;
	wlc_hw_config_t *bsscfg_hw_cfg = wlc_stf_bss_hw_cfg_get(bsscfg);


	*ap_cap = WL_MIMO_PS_STATUS_ASSOC_NONE;
	if ((bsscfg != wlc->cfg) || (!bsscfg->BSS) || (!bsscfg->associated))
		return BCME_OK;

	/* determine MIMO/SISO (low nibble) */
	if (bsscfg_hw_cfg->onlySISO) {
		*ap_cap = WL_MIMO_PS_STATUS_ASSOC_SISO;
		bssid = (!ETHER_ISNULLADDR(&bsscfg->BSSID)) ?
			&bsscfg->BSSID : &bsscfg->prev_BSSID;
		scb = wlc_scbfindband(wlc, bsscfg, bssid,
			CHSPEC_WLCBANDUNIT(bsscfg->current_bss->chanspec));
		if (scb != NULL) {
			if (!(SCB_VHT_CAP(scb) || SCB_HT_CAP(scb)))
				*ap_cap = WL_MIMO_PS_STATUS_ASSOC_LEGACY;
		}
	} else
		*ap_cap = WL_MIMO_PS_STATUS_ASSOC_MIMO;

	/* determine BW (high nibble) */
	*ap_cap |= (wlc_stf_mimo_ps_chanspec2iovarbw(bsscfg_hw_cfg->original_chanspec_bw) <<
			WL_MIMO_PS_STATUS_ASSOC_BW_SHIFT);

	return err;
}

static int
wlc_stf_mimo_ps_status_get_assoc_status(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint8 *as)
{
	struct scb *scb;
	struct ether_addr *bssid;
	*as = WL_MIMO_PS_STATUS_ASSOC_NONE;

	if ((bsscfg != wlc->cfg) || (!bsscfg->BSS) || (!bsscfg->associated))
		return BCME_OK;

	/* The bsscfg is associated */
	bssid = (!ETHER_ISNULLADDR(&bsscfg->BSSID)) ?
		&bsscfg->BSSID : &bsscfg->prev_BSSID;
	scb = wlc_scbfindband(wlc, bsscfg, bssid,
		CHSPEC_WLCBANDUNIT(bsscfg->current_bss->chanspec));
	if (scb == NULL) {
		/* scb not found but still allow other information
		 * of mimo_ps_status to return
		 */
		*as = 0xff;
		return BCME_OK;
	}

	if (SCB_VHT_CAP(scb)) {
		if (mimo_ps_status_if_vht_siso_rates(scb->rateset.vht_mcsmap)) {
			*as = WL_MIMO_PS_STATUS_ASSOC_SISO;
		} else {
			*as = WL_MIMO_PS_STATUS_ASSOC_MIMO;
		}
	} else if (SCB_HT_CAP(scb)) {
		if (mimo_ps_status_if_ht_siso_rates(scb)) {
			*as = WL_MIMO_PS_STATUS_ASSOC_SISO;
		} else {
			*as = WL_MIMO_PS_STATUS_ASSOC_MIMO;
		}
	} else {
		/* Legacy */
		*as = WL_MIMO_PS_STATUS_ASSOC_LEGACY;
	}
	return BCME_OK;
}

static bool
mimo_ps_status_if_vht_siso_rates(uint16 vht_mcsmap)
{
#if defined(WL11AC)
	uint8 i;
	uint8 mcs;
	for (i = 1; i <= VHT_CAP_MCS_MAP_NSS_MAX; i++) {
		mcs = VHT_MCS_MAP_GET_MCS_PER_SS(i, vht_mcsmap);
		if (mcs != VHT_CAP_MCS_MAP_NONE && i != 1)
			return FALSE;
	}
#endif /* WL11AC */
	return TRUE;
}

static bool
mimo_ps_status_if_ht_siso_rates(struct scb *scb)
{
	uint8 i;
	uint8 mask = 0xff;
	bool isSISO = TRUE;
	ASSERT(scb);
	for (i = 0; i < ARRAYSIZE(scb->rateset.mcs); i++) {
		mask = (i == 4) ? 0xfe : 0xff; /* MSC32 lowest bit set is SISO */
		if ((scb->rateset.mcs[i] & mask) != 0 && i != 0)
			isSISO = FALSE;
	}
	return isSISO;
}

uint16
wlc_stf_mimo_ps_iovarbw2chanspecbw(wlc_bsscfg_t *bsscfg, uint16 iovarbw)
{
	wlc_hw_config_t *bsscfg_hw_cfg = wlc_stf_bss_hw_cfg_get(bsscfg);
	uint16 result_chanspec_bw =
		(bsscfg_hw_cfg->original_chanspec_bw & WL_CHANSPEC_BW_MASK);

	if (iovarbw == MIMO_PS_CFG_IOVAR_BW_20M) {
		result_chanspec_bw = WL_CHANSPEC_BW_20;
	} else if (iovarbw == MIMO_PS_CFG_IOVAR_BW_40M) {
		result_chanspec_bw = WL_CHANSPEC_BW_40;
	} else if (iovarbw == MIMO_PS_CFG_IOVAR_BW_80M) {
		result_chanspec_bw = WL_CHANSPEC_BW_80;
	} else if (iovarbw == MIMO_PS_CFG_IOVAR_BW_8080M) {
		result_chanspec_bw = WL_CHANSPEC_BW_8080;
	} else if (iovarbw == MIMO_PS_CFG_IOVAR_BW_160M) {
		result_chanspec_bw = WL_CHANSPEC_BW_160;
	}
	return result_chanspec_bw;
}

static uint8
wlc_stf_mimo_ps_chanspec2iovarbw(chanspec_t chanspec)
{
	uint8 result_bw = MIMO_PS_CFG_IOVAR_BW_FULL;

	chanspec = chanspec & WL_CHANSPEC_BW_MASK;
	if (chanspec == WL_CHANSPEC_BW_20) {
		result_bw = MIMO_PS_CFG_IOVAR_BW_20M;
	} else if (chanspec == WL_CHANSPEC_BW_40) {
		result_bw = MIMO_PS_CFG_IOVAR_BW_40M;
	} else if (chanspec == WL_CHANSPEC_BW_80) {
		result_bw = MIMO_PS_CFG_IOVAR_BW_80M;
	} else if (chanspec == WL_CHANSPEC_BW_8080) {
		result_bw = MIMO_PS_CFG_IOVAR_BW_8080M;
	} else if (chanspec == WL_CHANSPEC_BW_160) {
		result_bw = MIMO_PS_CFG_IOVAR_BW_160M;
	}
	return result_bw;
}


void wlc_stf_update_ht_cap_for_bw(uint16 *cap, uint16 bw)
{
	if (bw == MIMO_PS_CFG_IOVAR_BW_20M) {
		*cap &= ~HT_CAP_40MHZ;
		*cap &= ~HT_CAP_SHORT_GI_40;
		*cap |= HT_CAP_40MHZ_INTOLERANT;
	}
}

void wlc_stf_set_host_configured_bw(wlc_bsscfg_t *cfg, wlc_bss_info_t *bi)
{
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg = wlc_stf_mimo_ps_cfg_get(cfg);
	wlc_hw_config_t *bsscfg_hw_cfg = wlc_stf_bss_hw_cfg_get(cfg);


	if ((bi->flags2 & WLC_BSS_VHT) && !(bi->flags2 & WLC_BSS_OPER_MODE)) {
		WL_STF_MIMO_PS_INFO(("AP without OMN, skipped setting host configured BW"));
		return;
	}
	if (bsscfg_mimo_ps_cfg && bsscfg_hw_cfg->default_chanspec_bw != MIMO_PS_CFG_IOVAR_BW_FULL) {
		bi->chanspec = wf_channel2chspec(wf_chspec_ctlchan(bi->chanspec),
		                   wlc_stf_mimo_ps_iovarbw2chanspecbw(cfg,
		                   bsscfg_hw_cfg->default_chanspec_bw));
		bsscfg_hw_cfg->chanspec_override = TRUE;
		if (bsscfg_hw_cfg->default_chanspec_bw == MIMO_PS_CFG_IOVAR_BW_20M) {
			bi->flags &= ~(WLC_BSS_SGI_40 | WLC_BSS_40MHZ);
			bi->flags |= WLC_BSS_40INTOL;
			bi->flags2 &= ~(WLC_BSS_SGI_80);
		}
		WL_STF_MIMO_PS_INFO(("wl%d: JOIN: MIMO PS set chanspec(0x%x):",
			WLCWLUNIT(cfg->wlc), bi->chanspec));
	}
}

void wlc_stf_mimops_handle_csa_chanspec(wlc_bsscfg_t *bsscfg)
{
	wlc_info_t* wlc = bsscfg->wlc;
	uint16 newchspec_bw = WL_CHANSPEC_BW_80;
	uint16 currchpec_bw = CHSPEC_BW(bsscfg->current_bss->chanspec);
	uint16 new_chspec;
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg = wlc_stf_mimo_ps_cfg_get(bsscfg);
	wlc_hw_config_t *bsscfg_hw_cfg = wlc_stf_bss_hw_cfg_get(bsscfg);
	wlc_hw_config_t *wlc_cfg_hw_cfg = wlc_stf_bss_hw_cfg_get(wlc->cfg);


	bsscfg_hw_cfg->original_chanspec_bw = bsscfg->current_bss->chanspec;
	bsscfg_hw_cfg->current_chanspec_bw = bsscfg->current_bss->chanspec;

	if (bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params_valid) {
		newchspec_bw = wlc_stf_mimo_ps_iovarbw2chanspecbw(wlc->cfg,
			bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params->bandwidth);
		/* Override iovar mimo_ps_bw since AP does not support oper mode notification */
		if ((bsscfg->current_bss->flags2 & WLC_BSS_VHT) &&
			!(bsscfg->current_bss->flags2 & WLC_BSS_OPER_MODE)) {
			newchspec_bw = MIMO_PS_CFG_IOVAR_BW_FULL;
			WL_STF_MIMO_PS_INFO((
			"wl%d: ARBIT:MIMO PS override host configured BW (AP without OMN)",
			WLCWLUNIT(wlc)));
		}
		if (newchspec_bw < currchpec_bw) {
			new_chspec = wf_channel2chspec(wf_chspec_ctlchan
				(wlc->cfg->current_bss->chanspec),
					newchspec_bw);
			bsscfg_hw_cfg->current_chanspec_bw = new_chspec;
			wlc_cfg_hw_cfg->chanspec_override = TRUE;
			if (WLC_BAND_PI_RADIO_CHANSPEC != bsscfg->current_bss->chanspec)
				wlc_update_bandwidth(wlc, wlc->cfg, new_chspec);
		} else {
			wlc_cfg_hw_cfg->chanspec_override = FALSE;
		}
	}
}

void wlc_stf_mimops_set_bw_upd_in_progress(wlc_info_t *wlc, bool in_progress)
{
	wlc->stf->bw_update_in_progress = in_progress;
}


bool wlc_stf_mimops_handle_bw_upd(wlc_info_t *wlc, wlc_bsscfg_t* cfg, chanspec_t new_chspec)
{
	BCM_REFERENCE(cfg);

	if (TXPKTPENDTOT(wlc)) {
		wlc->block_datafifo |= DATA_BLOCK_TXCHAIN;
		wlc_stf_mimops_set_bw_upd_in_progress(wlc, TRUE);
		wlc->stf->pending_bw = new_chspec;
		return TRUE;
	}
	wlc_stf_mimops_set_bw_upd_in_progress(wlc, FALSE);
	return FALSE;
}

wlc_mimo_ps_cfg_t *
wlc_stf_mimo_ps_cfg_get(wlc_bsscfg_t *bsscfg)
{
	wlc_info_t* wlc = bsscfg->wlc;
	wlc_stf_arb_mps_info_t *arb_mps_info = wlc->stf->arb_mps_info_hndl;
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg = NULL;

	ASSERT(arb_mps_info != NULL);

	bsscfg_mimo_ps_cfg = STF_MIMOPS_BSSCFG_CUBBY(arb_mps_info, bsscfg);

	ASSERT(bsscfg_mimo_ps_cfg != NULL);

	return bsscfg_mimo_ps_cfg;
}

bool wlc_stf_mimops_check_mrc_overrride(wlc_info_t *wlc)
{
	int idx;
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg;
	wlc_bsscfg_t *bsscfg;

	FOREACH_STA(wlc, idx, bsscfg) {
		bsscfg_mimo_ps_cfg = wlc_stf_mimo_ps_cfg_get(bsscfg);
		if (bsscfg_mimo_ps_cfg && bsscfg_mimo_ps_cfg->mrc_override)
			return FALSE;
	}
	return TRUE;
}

static void
wlc_stf_update_bmac_if_siso_rx_rates(wlc_bsscfg_t *bsscfg,
	ht_cap_ie_t *cap_ie, vht_cap_ie_t *vht_cap_ie_p,
	bcm_tlv_t *sup_rates, bcm_tlv_t *ext_rates)
{
	wlc_info_t* wlc = bsscfg->wlc;
	uint8 mcs;
	uint8 i;
	bool onlySISO = TRUE;
	bool basic_rates_present = FALSE;
	uint8 mask = 0xff;
	wlc_hw_config_t *bsscfg_hw_cfg = wlc_stf_bss_hw_cfg_get(bsscfg);
	wlc_hw_config_t *wlc_cfg_hw_cfg = wlc_stf_bss_hw_cfg_get(wlc->cfg);

	if (bsscfg->associated) {
		if (BSSCFG_STA(bsscfg)) {
			if (vht_cap_ie_p) {
				for (i = 1; i <= VHT_CAP_MCS_MAP_NSS_MAX; i++) {
					mcs = VHT_MCS_MAP_GET_MCS_PER_SS(i,
						vht_cap_ie_p->rx_mcs_map);
					if (mcs != VHT_CAP_MCS_MAP_NONE && i != WLC_STF_NSS_1RX) {
						onlySISO = FALSE;
						break;
					}
				}
			}

			if (cap_ie) {
				for (i = 1; i < ARRAYSIZE(cap_ie->supp_mcs); i++) {
						/* ignore mcs 32 */
						mask = (i == 4) ? 0xfe : 0xff;
						mcs = cap_ie->supp_mcs[i] & mask;
						if (mcs != 0) {
							onlySISO = FALSE;
							break;
						}
				}
			}

			for (i = 0; i < sup_rates->len; i++) {
				if (sup_rates->data[i] & WLC_RATE_FLAG) {
					basic_rates_present = TRUE;
					break;
				}
			}

			for (i = 0; i < ext_rates->len; i++) {
				if (ext_rates->data[i] & WLC_RATE_FLAG) {
					basic_rates_present = TRUE;
					break;
				}
			}

			/* This flag will be checked every time in aribrator callback */
			bsscfg_hw_cfg->basic_rates_present = basic_rates_present;
			if ((wf_chspec_ctlchan(WLC_BAND_PI_RADIO_CHANSPEC)) ==
			    wf_chspec_ctlchan(wlc->cfg->current_bss->chanspec))
				wlc_stf_siso_bcmc_rx(wlc, basic_rates_present);

			if (bsscfg_hw_cfg->onlySISO == onlySISO)
				return;

			bsscfg_hw_cfg->onlySISO = onlySISO;

			if (onlySISO == TRUE) {
				if (wlc->stf->rxchain != ONE_CHAIN_CORE0) {
#ifdef WL_MIMOPS_CFG
				if (WLC_MIMOPS_ENAB(wlc->pub)) {
					wlc_cfg_hw_cfg->current_txchains = ONE_CHAIN_CORE0;
					wlc_cfg_hw_cfg->current_rxchains = ONE_CHAIN_CORE0;
					wlc_update_mimops_cfg_transition(wlc);
					WL_STF_MIMO_PS_INFO((
						" WLC STF MIMO PS : Setting the chain to"
						"SISO value %d",
						onlySISO));
				}
#else
//we are in this fn by checking MIMOPS ENAB, we should not call non-mimops
#endif /* WL_MIMOPS_CFG */
				}
			}
		}
	}
}
