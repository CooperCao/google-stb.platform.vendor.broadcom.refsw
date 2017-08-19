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
 * $Id: wlc_stf_arb.c $
 */

/*
 * http://hwnbu-twiki.sj.broadcom.com/bin/view/Mwgroup/STFArbitratorModule
 */

#include <wlc_cfg.h>
#include <wlc_types.h>
#include <typedefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <wlc_log.h>
#include <wl_dbg.h>
#include <proto/ethernet.h>
#include <proto/802.11.h>
#include <wlioctl.h>

#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_stf.h>
#include <wlc_assoc.h>

#include <wlc_scandb.h>
#include <wlc_scan_priv.h>
#include <wlc_ht.h>
#ifdef WL11AC
#include <wlc_vht.h>
#endif
#include <wlc_scb_ratesel.h>


static void wlc_stf_update_rate_info(wlc_info_t *wlc);
static int32 wlc_stf_set_arbitrated_chains(wlc_info_t *wlc, uint8 ntxchains, uint8 nrxchains);
static bool wlc_stf_nss_arbitrator(wlc_info_t *wlc);
static int wlc_stf_arb_hwcfg_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_stf_arb_hwcfg_deinit(void *ctx, wlc_bsscfg_t *cfg);
static int wlc_stf_arb_bss_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_stf_arb_bss_deinit(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_arb_bss_state_upd(void *ctx, bsscfg_state_upd_data_t *evt);
static int
wlc_stf_arb_hwcfg_cubby_get(void *ctx, wlc_bsscfg_t *cfg, uint8 *data, int *len);
static int
wlc_stf_arb_hwcfg_cubby_set(void *ctx, wlc_bsscfg_t *cfg, const uint8 *data, int len);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void wlc_stf_arb_hwcfg_cubby_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);
#else
#define wlc_stf_arb_hwcfg_cubby_dump NULL
#endif

#define STF_ARB_BSSCFG_CUBBY_LOC(sm, cfg) \
	((wlc_stf_nss_request_t **)BSSCFG_CUBBY((cfg), (sm)->cfgh_arb))
#define STF_ARB_BSSCFG_CUBBY(sm, cfg) (*(STF_ARB_BSSCFG_CUBBY_LOC(sm, cfg)))

#define STF_HWCFG_BSSCFG_CUBBY_LOC(sm, cfg) \
	((wlc_hw_config_t **)BSSCFG_CUBBY((cfg), (sm)->cfgh_hwcfg))
#define STF_HWCFG_BSSCFG_CUBBY(sm, cfg) (*(STF_HWCFG_BSSCFG_CUBBY_LOC(sm, cfg)))
#define WLC_STF_ARB_HWCFG_CUBBY_SIZE	(sizeof(wlc_hw_config_t))


/*
 * attach and initialize STA Arbitrtaor requets
 */
wlc_stf_arb_t *
BCMATTACHFN(wlc_stf_arb_attach)(wlc_info_t* wlc, void *ctx)
{
	wlc_stf_arb_t *arb;
	wlc_stf_arb_mps_info_t *stf_arb_mps_info = (wlc_stf_arb_mps_info_t *)ctx;
	bsscfg_cubby_params_t bsscfg_cubby_params;

	if ((arb = MALLOCZ(wlc->osh, sizeof(wlc_stf_arb_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			WLCWLUNIT(wlc), __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	/* Register iovar req with STF Aribitrator for configuring chains */
	if ((arb->iovar_req = wlc_stf_nss_request_register(wlc,
			WLC_STF_ARBITRATOR_REQ_ID_IOVAR,
			WLC_STF_ARBITRATOR_REQ_PRIO_IOVAR,
			NULL, NULL)) == NULL) {
		WL_ERROR(("wl%d: %s: iovar_req failed\n", WLCWLUNIT(wlc), __FUNCTION__));
		goto fail;
	}
	/* Register lte req with STF Aribitrator for configuring chains */
	if ((arb->lte_req = wlc_stf_nss_request_register(wlc,
			WLC_STF_ARBITRATOR_REQ_ID_LTE_COEX,
			WLC_STF_ARBITRATOR_REQ_PRIO_LTE_COEX,
			NULL, NULL)) == NULL) {
		WL_ERROR(("wl%d: %s: lte_req failed\n", WLCWLUNIT(wlc), __FUNCTION__));
		goto fail;
	}
	/* Register txppr offset req with STF Aribitrator for configuring chains */
	if ((arb->txppr_req = wlc_stf_nss_request_register(wlc,
			WLC_STF_ARBITRATOR_REQ_ID_TXPPR,
			WLC_STF_ARBITRATOR_REQ_PRIO_TXPPR,
			NULL, NULL)) == NULL) {
		WL_ERROR(("wl%d: %s: txppr_req failed\n", WLCWLUNIT(wlc), __FUNCTION__));
		goto fail;
	}
	/* Register pwr throttlereq with STF Aribitrator for configuring chains */
	if ((arb->pwrthrottle_req = wlc_stf_nss_request_register(wlc,
			WLC_STF_ARBITRATOR_REQ_ID_PWRTHOTTLE,
			WLC_STF_ARBITRATOR_REQ_PRIO_PWRTHOTTLE,
			NULL, NULL)) == NULL) {
		WL_ERROR(("wl%d: %s: pwrthrottle_req failed\n", WLCWLUNIT(wlc), __FUNCTION__));
		goto fail;
	}
	/* Register temp sense req with STF Aribitrator for configuring chains */
	if ((arb->tmpsense_req = wlc_stf_nss_request_register(wlc,
			WLC_STF_ARBITRATOR_REQ_ID_TMPSENSE,
			WLC_STF_ARBITRATOR_REQ_PRIO_TMPSENSE,
			NULL, NULL)) == NULL) {
		WL_ERROR(("wl%d: %s: tmpsense_req failed\n", WLCWLUNIT(wlc), __FUNCTION__));
		goto fail;
	}
	arb->iovar_req_txchain = WL_STF_CHAINS_NOT_CONFIGURED;
	arb->iovar_req_rxchain = WL_STF_CHAINS_NOT_CONFIGURED;

	/* reserve cubby space in the bsscfg container for per-bsscfg private data */
	memset(&bsscfg_cubby_params, 0, sizeof(bsscfg_cubby_params));
	bsscfg_cubby_params.context = stf_arb_mps_info;
	bsscfg_cubby_params.fn_deinit = wlc_stf_arb_hwcfg_deinit;
	bsscfg_cubby_params.fn_init = wlc_stf_arb_hwcfg_init;
	bsscfg_cubby_params.fn_get = wlc_stf_arb_hwcfg_cubby_get;
	bsscfg_cubby_params.fn_set = wlc_stf_arb_hwcfg_cubby_set;
	bsscfg_cubby_params.config_size = WLC_STF_ARB_HWCFG_CUBBY_SIZE;
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	bsscfg_cubby_params.fn_dump = wlc_stf_arb_hwcfg_cubby_dump;
#endif

	if ((stf_arb_mps_info->cfgh_hwcfg = wlc_bsscfg_cubby_reserve_ext(wlc,
	                                    sizeof(wlc_hw_config_t *),
	                                    &bsscfg_cubby_params)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve hwcfg failed\n",
		         WLCWLUNIT(wlc), __FUNCTION__));
		goto fail;
	}
	if ((stf_arb_mps_info->cfgh_arb = wlc_bsscfg_cubby_reserve(wlc,
	                                  sizeof(wlc_stf_nss_request_t *),
	                                  wlc_stf_arb_bss_init,
	                                  wlc_stf_arb_bss_deinit,
	                                  NULL,
	                                  stf_arb_mps_info)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve failed\n",
		         WLCWLUNIT(wlc), __FUNCTION__));
		goto fail;
	}

	 /* register bsscfg state change callback */
	if (wlc_bsscfg_state_upd_register(wlc, wlc_arb_bss_state_upd,
	                                  stf_arb_mps_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_state_upd_register failed\n",
		         WLCWLUNIT(wlc), __FUNCTION__));
		goto fail;
	}
	return arb;

fail:
	wlc_stf_arb_detach(wlc, ctx);
	return NULL;

}

int
BCMATTACHFN(wlc_stf_arb_mimops_info_attach)(wlc_info_t* wlc)
{
	wlc_stf_arb_mps_info_t *stf_arb_mps_info;

	/* module info */
	if ((stf_arb_mps_info = (wlc_stf_arb_mps_info_t *)MALLOCZ(wlc->osh,
	                            sizeof(wlc_stf_arb_mps_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
		           WLCWLUNIT(wlc), __FUNCTION__, MALLOCED(wlc->osh)));
		return BCME_NOMEM;
	}
	stf_arb_mps_info->wlc = wlc;
	wlc->stf->arb_mps_info_hndl = stf_arb_mps_info;

	return BCME_OK;

}

/* detach STF Arbitrator */
void
BCMATTACHFN(wlc_stf_arb_detach)(wlc_info_t* wlc, void *ctx)
{
	wlc_stf_arb_mps_info_t *stf_arb_mps_info = (wlc_stf_arb_mps_info_t *)ctx;

	if (wlc->stf->arb == NULL)
		return;

	if (wlc_stf_nss_request_unregister(wlc, wlc->stf->arb->iovar_req))
		WL_STF_ARBITRATOR_ERROR(("ARBI: Error at Unregister iovar req\n"));
	if (wlc_stf_nss_request_unregister(wlc, wlc->stf->arb->lte_req))
		WL_STF_ARBITRATOR_ERROR(("ARBI: Error at Unregister lte req\n"));
	if (wlc_stf_nss_request_unregister(wlc, wlc->stf->arb->txppr_req))
		WL_STF_ARBITRATOR_ERROR(("ARBI: Error at Unregister txppr req\n"));
	if (wlc_stf_nss_request_unregister(wlc, wlc->stf->arb->tmpsense_req))
		WL_STF_ARBITRATOR_ERROR(("ARBI: Error at Unregister tmp sense req\n"));
	wlc->stf->arb->iovar_req = NULL;
	wlc->stf->arb->lte_req = NULL;
	wlc->stf->arb->txppr_req = NULL;
	wlc->stf->arb->tmpsense_req = NULL;

	/* unregister bsscfg state change callback */
	(void)wlc_bsscfg_state_upd_unregister(wlc, wlc_arb_bss_state_upd,
	                                      stf_arb_mps_info);

	MFREE(wlc->osh, wlc->stf->arb, sizeof(wlc_stf_arb_t));
	wlc->stf->arb = NULL;


}

void
BCMATTACHFN(wlc_stf_arb_mimops_info_detach)(wlc_info_t* wlc)
{
	MFREE(wlc->osh, wlc->stf->arb_mps_info_hndl, sizeof(wlc_stf_arb_mps_info_t));
	wlc->stf->arb_mps_info_hndl = NULL;
}

static int
wlc_stf_arb_hwcfg_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_stf_arb_mps_info_t *arb_mps_info = (wlc_stf_arb_mps_info_t *)ctx;
	wlc_info_t *wlc = arb_mps_info->wlc;
	wlc_hw_config_t **phw_cfg = STF_HWCFG_BSSCFG_CUBBY_LOC(arb_mps_info, cfg);
	wlc_hw_config_t *hw_cfg;
	int err = BCME_OK;

	if ((hw_cfg = MALLOCZ(wlc->osh, sizeof(wlc_hw_config_t))) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_hw_config_t_alloc failed\n",
		         wlc->pub->unit, __FUNCTION__));
		err = BCME_NOMEM;
		goto fail;
	}
	*phw_cfg = hw_cfg;

	/* Assign default max hw tx/rx chains */
	hw_cfg->current_txchains = wlc->stf->hw_txchain;
	hw_cfg->current_rxchains = wlc->stf->hw_rxchain;
	hw_cfg->default_txchains = wlc->stf->hw_txchain;
	hw_cfg->default_rxchains = wlc->stf->hw_rxchain;
	hw_cfg->last_ntxchains = 0;

	return BCME_OK;

fail:
	wlc_stf_arb_hwcfg_deinit(arb_mps_info, cfg);
	return err;
}

static void
wlc_stf_arb_hwcfg_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_stf_arb_mps_info_t *arb_mps_info = (wlc_stf_arb_mps_info_t *)ctx;
	wlc_info_t *wlc = arb_mps_info->wlc;
	wlc_hw_config_t **phw_cfg = STF_HWCFG_BSSCFG_CUBBY_LOC(arb_mps_info, cfg);
	wlc_hw_config_t *hw_cfg = *phw_cfg;

	MFREE(wlc->osh, hw_cfg, sizeof(wlc_hw_config_t));
	*phw_cfg = NULL;
}

static int
wlc_stf_arb_hwcfg_cubby_get(void *ctx, wlc_bsscfg_t *cfg, uint8 *data, int *len)
{
	wlc_stf_arb_mps_info_t *arb_mps_info = (wlc_stf_arb_mps_info_t *)ctx;
	wlc_hw_config_t *bsscfg_hw_cfg = NULL;

	ASSERT(cfg != NULL);
	ASSERT(data != NULL);
	ASSERT(arb_mps_info != NULL);
	if (*len < WLC_STF_ARB_HWCFG_CUBBY_SIZE) {
		WL_ERROR(("wl%d: %s: Buffer too short\n", arb_mps_info->wlc->pub->unit,
		         __FUNCTION__));
		*len = WLC_STF_ARB_HWCFG_CUBBY_SIZE;
		return BCME_BUFTOOSHORT;
	}
	bsscfg_hw_cfg = STF_HWCFG_BSSCFG_CUBBY(arb_mps_info, cfg);
	ASSERT(bsscfg_hw_cfg != NULL);
	memcpy(data, (uint8*)bsscfg_hw_cfg, WLC_STF_ARB_HWCFG_CUBBY_SIZE);
	*len = WLC_STF_ARB_HWCFG_CUBBY_SIZE;
	return BCME_OK;
}

static int
wlc_stf_arb_hwcfg_cubby_set(void *ctx, wlc_bsscfg_t *cfg, const uint8 *data, int len)
{
	wlc_stf_arb_mps_info_t *arb_mps_info = (wlc_stf_arb_mps_info_t *)ctx;
	wlc_hw_config_t *bsscfg_hw_cfg = NULL;
	const wlc_hw_config_t *from_bsscfg_hw_cfg = (const wlc_hw_config_t *)data;

	ASSERT(cfg != NULL);
	ASSERT(data != NULL);
	ASSERT(arb_mps_info != NULL);
	ASSERT(len == WLC_STF_ARB_HWCFG_CUBBY_SIZE);
	/* get the cubby info */
	bsscfg_hw_cfg = STF_HWCFG_BSSCFG_CUBBY(arb_mps_info, cfg);
	ASSERT(bsscfg_hw_cfg != NULL);
	bsscfg_hw_cfg->default_rxchains = from_bsscfg_hw_cfg->default_rxchains;
	bsscfg_hw_cfg->default_txchains = from_bsscfg_hw_cfg->default_txchains;
	bsscfg_hw_cfg->current_rxchains = from_bsscfg_hw_cfg->current_rxchains;
	bsscfg_hw_cfg->current_txchains = from_bsscfg_hw_cfg->current_txchains;
	bsscfg_hw_cfg->default_chanspec_bw = from_bsscfg_hw_cfg->default_chanspec_bw;
	if (bsscfg_hw_cfg->default_chanspec_bw == MIMO_PS_CFG_IOVAR_BW_FULL) {
		bsscfg_hw_cfg->chanspec_override = FALSE;
	} else {
		bsscfg_hw_cfg->chanspec_override = TRUE;
	}

	WL_STF_ARBITRATOR_TRACE(("wl%d.%d set hwcfg: info/cubby %p/%p tx_rx_chains 0x%x",
		arb_mps_info->wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
		arb_mps_info, bsscfg_hw_cfg,
		((bsscfg_hw_cfg->default_txchains << 24) |
		(bsscfg_hw_cfg->current_txchains << 16) |
		(bsscfg_hw_cfg->default_rxchains << 8) |
		(bsscfg_hw_cfg->current_rxchains))));
	WL_STF_ARBITRATOR_TRACE((" default_chanspec_bw %d chanspec_override %d\n",
	     bsscfg_hw_cfg->default_chanspec_bw, bsscfg_hw_cfg->chanspec_override));

	return BCME_OK;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void
wlc_stf_arb_hwcfg_cubby_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	wlc_stf_arb_mps_info_t *arb_mps_info = (wlc_stf_arb_mps_info_t *)ctx;
	wlc_hw_config_t *bsscfg_hw_cfg = STF_HWCFG_BSSCFG_CUBBY(arb_mps_info, cfg);

	ASSERT(bsscfg_hw_cfg != NULL);

	bcm_bprintf(b, "============= bsscfg_hw_cfg =============\n");
	bcm_bprintf(b, "\tTx_Rx_default_current_chains 0x%x, last_ntxchains %d\n",
	            ((bsscfg_hw_cfg->default_txchains << 24) |
	            (bsscfg_hw_cfg->current_txchains << 16) |
	            (bsscfg_hw_cfg->default_rxchains << 8) |
	            (bsscfg_hw_cfg->current_rxchains)),
	            bsscfg_hw_cfg->last_ntxchains);
	bcm_bprintf(b, "\tchspec bw cur/org/dflt: %x/%x/%x, chspec override %d\n",
	            bsscfg_hw_cfg->current_chanspec_bw,
	            bsscfg_hw_cfg->original_chanspec_bw,
	            bsscfg_hw_cfg->default_chanspec_bw,
	            bsscfg_hw_cfg->chanspec_override);
	bcm_bprintf(b, "\tonlySISO %d, basic_rate %d\n",
	            bsscfg_hw_cfg->onlySISO,
	            bsscfg_hw_cfg->basic_rates_present);
}
#endif /* BCMDBG || BCMDBG_DUMP */

static int
wlc_stf_arb_bss_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_stf_arb_mps_info_t *arb_mps_info = (wlc_stf_arb_mps_info_t *)ctx;
	wlc_info_t *wlc = arb_mps_info->wlc;
	wlc_stf_nss_request_t **pstf_arb_req = STF_ARB_BSSCFG_CUBBY_LOC(arb_mps_info, cfg);
	wlc_stf_nss_request_t *stf_arb_req = NULL;
	uint8 prio, req_id;
	bool register_call = FALSE;
	int err = BCME_OK;

	if (BSSCFG_AP(cfg)) {
		req_id = WLC_STF_ARBITRATOR_REQ_ID_AP_BSS;
		prio = WLC_STF_ARBITRATOR_REQ_PRIO_AP_BSS;
		register_call = TRUE;
	} else if (BSSCFG_AWDL(wlc, cfg)) {
		req_id = WLC_STF_ARBITRATOR_REQ_ID_AWDL_BSS;
		prio = WLC_STF_ARBITRATOR_REQ_PRIO_AWDL_BSS;
		register_call = TRUE;
	} else if (cfg == cfg->wlc->cfg) {
		req_id = WLC_STF_ARBITRATOR_REQ_ID_MIMOPS_BSS;
		prio = WLC_STF_ARBITRATOR_REQ_PRIO_MIMOPS_BSS;
		register_call = TRUE;
	}
	if (register_call) {
		stf_arb_req = wlc_stf_nss_request_register(wlc,
		                  req_id, prio, wlc_stf_arb_call_back,
		                  (void*)cfg);
		if (!stf_arb_req) {
			WL_ERROR(("wl%d: %s: stf_arb_req_bss failed\n",
			         WLCWLUNIT(wlc), __FUNCTION__));
			err = BCME_NOMEM;
			goto fail;
		}
	}
	*pstf_arb_req = stf_arb_req;
	return BCME_OK;

fail:
	wlc_stf_arb_bss_deinit(arb_mps_info, cfg);
	return err;
}

static void
wlc_stf_arb_bss_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_stf_arb_mps_info_t *arb_mps_info = (wlc_stf_arb_mps_info_t *)ctx;
	wlc_info_t *wlc = arb_mps_info->wlc;
	wlc_stf_nss_request_t **pstf_arb_req = STF_ARB_BSSCFG_CUBBY_LOC(arb_mps_info, cfg);
	wlc_stf_nss_request_t *stf_arb_req = *pstf_arb_req;

	if (stf_arb_req) {
		if (wlc_stf_nss_request_unregister(wlc, stf_arb_req))
			 WL_STF_ARBITRATOR_ERROR(("ARBI: Error at Unregister bsscfg req\n"));
		*pstf_arb_req = NULL;
	}
}

/* handle bsscfg state change */
static void
wlc_arb_bss_state_upd(void *ctx, bsscfg_state_upd_data_t *evt)
{
	wlc_stf_arb_mps_info_t *arb_mps_info = (wlc_stf_arb_mps_info_t *)ctx;
	wlc_info_t *wlc = arb_mps_info->wlc;
	wlc_bsscfg_t *bsscfg = evt->cfg;
	wlc_hw_config_t *bsscfg_hw_cfg = NULL;
	wlc_stf_nss_request_t *stf_arb_req = NULL;
	uint8 state;
	bool arb_update = FALSE;

	//do need to check ARB_ENAB, was checked during registration??

	ASSERT(arb_mps_info != NULL);
	ASSERT(bsscfg != NULL);

	bsscfg_hw_cfg = STF_HWCFG_BSSCFG_CUBBY(arb_mps_info, bsscfg);
	ASSERT(bsscfg_hw_cfg != NULL);

	stf_arb_req = STF_ARB_BSSCFG_CUBBY(arb_mps_info, bsscfg);
	if (stf_arb_req == NULL) {
		WL_STF_ARBITRATOR_TRACE(("ARB_REQ is NULL, bsscfg %d\n", bsscfg->_idx));
		return;
	}

	if (BSSCFG_AP(bsscfg) && bsscfg->enable && !evt->old_enable) {
		state = WLC_STF_ARBITRATOR_REQ_STATE_RXTX_ACTIVE;
		arb_update = TRUE;
	} else if (!bsscfg->enable && evt->old_enable) {
		state = WLC_STF_ARBITRATOR_REQ_STATE_RXTX_INACTIVE;
		arb_update = TRUE;
	}

	if (stf_arb_req && arb_update) {
		WL_STF_ARBITRATOR_TRACE(("wlc_bsscfg %s Setting default tx_rx chains 0x%x",
		    (state ? "enable" : "disable"),
		    (bsscfg_hw_cfg->default_txchains << 8 | bsscfg_hw_cfg->default_rxchains)));

		wlc_stf_nss_request_update(wlc, stf_arb_req,
		                           state,
		                           bsscfg_hw_cfg->default_txchains,
		                           WLC_BITSCNT(bsscfg_hw_cfg->default_txchains),
		                           bsscfg_hw_cfg->default_rxchains,
		                           WLC_BITSCNT(bsscfg_hw_cfg->default_rxchains));
	}
}


/*				Arbitrator design
*----------------------------------------------------------------------------------
* - All requests will be inserted in singly linked priority list;
* - Higher the priority requests will be placed at front.
* - Any requester which wants to set tx/rx chains has to register using below API's.
* - Requester can update its active/inactive status, chains using a given API call.
* - Arbitrator will judge based on current condition which is best applicable.
* - Upon change in hwchain configuration, all active requests will receive callbacks.
*-----------------------------------------------------------------------------------
*/
/* Add an request entry in STF request q; priority single linked list  */
void *wlc_stf_nss_request_register(wlc_info_t *wlc,
	uint8 req_id, uint8 prio, void *cb, void *ctx)
{
	wlc_stf_nss_request_q_t *new_req_q, *cur, *prev = NULL;
	wlc_stf_nss_request_t *request = NULL;

	if ((request = MALLOCZ(wlc->osh, sizeof(*request))) == NULL) {
		goto handle_error;
	}
	if ((new_req_q = MALLOCZ(wlc->osh, sizeof(*new_req_q))) == NULL) {
		MFREE(wlc->osh, request, sizeof(*request));
		goto handle_error;
	}
	/* Store the given information in the req */
	request->state = WLC_STF_ARBITRATOR_REQ_STATE_RXTX_INACTIVE;
	request->txchains = wlc->stf->hw_txchain;
	request->rxchains = wlc->stf->hw_rxchain;
	request->prio = prio;
	request->req_id = req_id;
	request->call_back_fn = cb;
	request->priv_ctx = ctx;
	request->flags = 0;

	new_req_q->req = request;
	new_req_q->next = NULL;
	/* Insert in priority list */
	if (!wlc->stf->arb_req_q) {
		wlc->stf->arb_req_q = new_req_q;
	} else {
		cur = wlc->stf->arb_req_q;
		while (cur && (cur->req->prio < request->prio)) {
			prev = cur;
			cur = cur->next;
		}
		new_req_q->next = cur;
		if (!prev) {
			wlc->stf->arb_req_q = new_req_q;
		} else {
			prev->next = new_req_q;
		}
	}
	WL_STF_ARBITRATOR_TRACE(("ARBI: Added Request for ID %d prio %d\n", req_id, prio));
	return ((void *)request);
handle_error:
	WL_STF_ARBITRATOR_ERROR(("Failed to alloc memory for req %d\n", req_id));
	return NULL;

}

/* Remove a request entry from STF request q */
int32 wlc_stf_nss_request_unregister(wlc_info_t *wlc, void *req)
{
	wlc_stf_nss_request_q_t *cur, *prev = NULL;
	wlc_stf_nss_request_t *request = (wlc_stf_nss_request_t *)req;
	cur = wlc->stf->arb_req_q;
	while (cur && (cur->req != request)) {
		prev = cur;
		cur = cur->next;
	}
	if (!cur)
		return BCME_NOTFOUND;
	if (!prev) {
		wlc->stf->arb_req_q = cur->next;
	} else {
		prev->next = cur->next;
	}
	WL_STF_ARBITRATOR_TRACE(("ARBI: Del Request ID %d\n", cur->req->req_id));
	MFREE(wlc->osh, cur->req, sizeof(*(cur->req)));
	MFREE(wlc->osh, cur, sizeof(*cur));
	return BCME_OK;
}

/* DUMP request entries from STF request q */
void wlc_stf_nss_request_dump(wlc_info_t *wlc)
{
	wlc_stf_nss_request_q_t *cur;
	if (!wlc->stf->arb_req_q)
		return;
	cur = wlc->stf->arb_req_q;
	while (cur) {
		WL_PRINT(("ID %d, Prio %d, state %d, ntxx %d, nrxc %d\n",
			cur->req->req_id,
			cur->req->prio,
			cur->req->state,
			cur->req->txchains,
			cur->req->rxchains));
		cur = cur->next;
	}
	WL_PRINT(("Current chains RX %d TX %d\n",
		wlc->stf->rxchain, wlc->stf->txchain));
}

/* External API call for requesters to update their state and chains */
int32 wlc_stf_nss_request_update(wlc_info_t *wlc, void *req,
	uint8 state, uint8 txchain, uint8 tx_nss, uint8 rxchain, uint8 rx_nss)
{
	wlc_stf_nss_request_t *request = (wlc_stf_nss_request_t *)req;
	bool sync_arbitrator = FALSE;
	if (request == NULL) {
		WL_STF_ARBITRATOR_ERROR(("ARBI: request is null\n"));
		return BCME_ERROR;
	}
	/* streams and chains cannot be zero */
	ASSERT((txchain && rxchain && tx_nss && rx_nss));
	if ((!txchain || !rxchain || !tx_nss || !rx_nss)) {
		WL_ERROR(("ARBI INVALID request by %d, %d, %d, %d, %d\n",
			request->req_id, txchain, rxchain, tx_nss, rx_nss));
		return BCME_ERROR;
	}
	/* If state has changed or its in Active state with changed chains then,
	 * call arbitrator for re-evalution of states and chains to hw.
	 */
	if ((request->state != state) ||
		(WLC_STF_ARBI_REQ_STATE_TXACTIVE(request) &&
		((request->txchains != txchain) || (request->tx_nss != tx_nss))) ||
		(WLC_STF_ARBI_REQ_STATE_RXACTIVE(request) &&
		((request->rxchains != rxchain) || (request->rx_nss != rx_nss)))) {
		   sync_arbitrator = TRUE;
	}
	request->state = state;
	request->txchains = txchain;
	request->rxchains = rxchain;
	request->tx_nss = tx_nss;
	request->rx_nss = rx_nss;
	if (sync_arbitrator) {
		/* Call arbitrator, If it return false, means it did not change settings
		 * Call its call back if needed in active state.
		 */
		WL_STF_ARBITRATOR_TRACE(("ARBI: request update %d %d TX = %d RX = %d\n",
			request->req_id, request->state, txchain, rxchain));
		if (!wlc_stf_nss_arbitrator(wlc) &&
			WLC_STF_ARBI_REQ_STATE_ACTIVE(request)) {
			if (request->call_back_fn)
				(request->call_back_fn)(request->priv_ctx, wlc->stf->txchain,
				wlc->stf->rxchain);
		}
	}
	return BCME_OK;
}


/* Evaluate chains from starems  */
/* Will work for 4350 for now, This needs to be looked in for other chips */
/* Table for 2 streams (4350) Input and evaluated output show below.
*----------------------------------
*	Input	    |	Output	   |
*----------------------------------
*	Txc	Rxc |	TXC    RXC |
*----------------------------------
*	1	1	1	1
*	1	2	1	1
*	1	3	1	3
*	2	1	2	2
*	2	2	2	2
*	2	3	2	3
*	3	1	1	1
*	3	2	2	2
*	3	3	3	3
*----------------------------------
*/

static void wlc_stf_arbitrator_evaluate(wlc_info_t *wlc, uint8 *txchain,
	uint8 *tx_nss, uint8 *rxchain, uint8 *rx_nss)
{
	/* Below is temparory optimizer chosing core 0 */
	if (WLC_BITSCNT(*txchain) > *tx_nss) {
		if (*tx_nss == 1)
			*txchain = 1;
	}

	/* Rxchain follows txchain if rx_nss = tx_nss = 1.
	 * If tx_nss = 2 and rx_nss = 1, then txchain follows rxchain.
	 * If tx_nss = 1 and rx_nss = 2, rxchain can remain 0x3
	 * and txchain can retain its value.
	 */
	if (*rx_nss == 1) {
		if (*tx_nss == 1) {
			*rxchain = *txchain;
		} else {
			*txchain = *rxchain;
		}
	}
	/* Update nss from chain info */
	*tx_nss = WLC_BITSCNT(*txchain);
	*rx_nss = WLC_BITSCNT(*rxchain);
}

void
wlc_stf_arb_set_txchain_upd_in_progress(wlc_info_t *wlc, bool in_progress)
{
	wlc->stf->arb->txchain_update_in_progress = in_progress;
}

/* STF aribitrator function to handle context update
 * Looks for all active request states, find tx and rx chains on given highest priority seperately.
 * Re-evaluate chains if there are any constraints.
 * Commit the tx-rx chain settings if required.
 * Call active requesters call backs if they have any cb's.
 * Call only when requesters think change in its state/chain reduce code execuation.
 * Return TRUE once its done calling cb's, else FALSE.
 */
static bool wlc_stf_nss_arbitrator(wlc_info_t *wlc)
{
	wlc_stf_nss_request_q_t *cur;
	uint8 arb_txchain, arb_rxchain, arb_tx_nss, arb_rx_nss;
	bool tx_set = FALSE, rx_set = FALSE;

	if (!wlc->stf->arb_req_q)
		return FALSE;
	cur = wlc->stf->arb_req_q;
	arb_txchain = wlc->stf->txchain;
	arb_rxchain = wlc->stf->rxchain;
	arb_tx_nss = WLC_BITSCNT(arb_txchain);
	arb_rx_nss = WLC_BITSCNT(arb_rxchain);

	/* Run though all active requests and find final tx and rx chains */
	for (cur = wlc->stf->arb_req_q; cur; cur = cur->next) {
		/* Skip in-active once */
		if (WLC_STF_ARBI_REQ_STATE_INACTIVE(cur->req))
			continue;
		/* Pick highest priority once, tx and rx individually as some may have don't care.
		 * This can be optimized/enhanced, as we get more constrains and requirements.
		 * our current constraints are very clear.
		 */
		/* Pick tx */
		if (!tx_set && WLC_STF_ARBI_REQ_STATE_TXACTIVE(cur->req)) {
			arb_txchain = cur->req->txchains;
			arb_tx_nss = cur->req->tx_nss;
			tx_set = TRUE;
		}
		/* Pick rx */
		if (!rx_set && WLC_STF_ARBI_REQ_STATE_RXACTIVE(cur->req)) {
			arb_rxchain = cur->req->rxchains;
			arb_rx_nss = cur->req->rx_nss;
			rx_set = TRUE;
		}
		/* If we have picked both from priority list then, exit loop */
		if (tx_set && rx_set)
			break;
	}
	/* Run re-evaluation for constraints */
	wlc_stf_arbitrator_evaluate(wlc, &arb_txchain, &arb_tx_nss, &arb_rxchain, &arb_rx_nss);

	if ((arb_rxchain == wlc->stf->rxchain) && (arb_txchain == wlc->stf->txchain)) {
		WL_STF_ARBITRATOR_TRACE(("ARBI: no change  RXC = %d, TXC = %d \n",
			arb_rxchain, arb_txchain));
		/* Call callbacks anyways as they need to be updated if we re-evluated
		 * on next call not to change chains.
		 */
		if (wlc->stf->arb->txchain_update_in_progress)
			goto call_backs;
		return FALSE;
	}
	/* Let pkts drain, we will come back to re-evaluated chains */
	if (TXPKTPENDTOT(wlc)) {
		wlc->block_datafifo |= DATA_BLOCK_TXCHAIN;
		wlc->stf->arb->txchain_update_in_progress = 1;
		WL_STF_ARBITRATOR_TRACE(("ARBI: pkts pending delay chain settings\n"));
		return TRUE;
	}
	wlc_mimo_siso_metrics_snapshot(wlc, FALSE, WL_MIMOPS_METRICS_SNAPSHOT_ARBI);
	/* Set chains */
	wlc_stf_set_arbitrated_chains(wlc, arb_txchain, arb_rxchain);
call_backs:
	WL_STF_ARBITRATOR_TRACE(("ARBI: Calling callbacks\n"));
	/* Call callbacks for active requests */
	for (cur = wlc->stf->arb_req_q; cur; cur = cur->next) {
		if (WLC_STF_ARBI_REQ_STATE_INACTIVE(cur->req))
			continue;
		if (cur->req->call_back_fn) {
			(cur->req->call_back_fn)(cur->req->priv_ctx, wlc->stf->txchain,
				wlc->stf->rxchain);
		}
	}
	return TRUE;
}

/* Call setting tx and rx chains; careful caller responsible to do any cleanups before this */
static int32 wlc_stf_set_arbitrated_chains(wlc_info_t *wlc, uint8 txchains, uint8 rxchains)
{
	uint8 rxchain_cnt;
	uint8 txstreams = (uint8)WLC_BITSCNT(txchains);
	bool force_clk = FALSE, mpc = wlc->mpc_out;

	/* Do not expect any pending packets at this time */
	if (TXPKTPENDTOT(wlc)) {
		ASSERT(0);
	}
	WL_STF_ARBITRATOR_TRACE(("Before Set tx %d rx %d chains\n", txchains, rxchains));

	if (!wlc->clk) {
		wlc->mpc_out = TRUE;
		wlc_radio_mpc_upd(wlc);
		force_clk  = TRUE;
	}

	wlc->stf->arb->txchain_update_in_progress = 0;

	/* Disable STBC if we are switching to one txchain */
	if ((wlc->stf->txchain > 1) && (txchains == 1) &&
		(wlc_stf_stbc_tx_get(wlc) == ON)) {
		wlc->bandstate[BAND_2G_INDEX]->band_stf_stbc_tx = OFF;
		wlc->bandstate[BAND_5G_INDEX]->band_stf_stbc_tx = OFF;
	}

	rxchain_cnt = (uint8)WLC_BITSCNT(rxchains);
	/* save the rxchain value */
	//ToDo
	//wlc->stf->rxchain_save = rxchains;
	wlc->stf->rxchain = rxchains;
	wlc->stf->rxstreams = rxchain_cnt;
	wlc->stf->txchain = txchains;
	/* Update op_rxstreams */
	wlc->stf->op_rxstreams = wlc->stf->rxstreams;


	/* if we're not turning everything off, ensure CCK and OFDM overrides conform to txchain */
	/* otherwise, remove overrides */
	if (wlc->stf->txchain) {
		if (wlc->stf->txcore_override[OFDM_IDX]) {
			if ((wlc->stf->txcore_override[OFDM_IDX] & wlc->stf->txchain) !=
				wlc->stf->txcore_override[OFDM_IDX]) {
				WL_NONE(("ofdm override removed chain=%x val=%x\n",
					wlc->stf->txcore_override[OFDM_IDX], wlc->stf->txchain));
				wlc->stf->txcore_override[OFDM_IDX] = 0;

				if (wlc->stf->txchain_perrate_state_modify) {
					wlc->stf->txchain_perrate_state_modify(wlc);
				}
			}
		}
		if (wlc->stf->txcore_override[CCK_IDX]) {
			if ((wlc->stf->txcore_override[CCK_IDX] & wlc->stf->txchain) !=
				wlc->stf->txcore_override[CCK_IDX]) {
				WL_NONE(("cck override removed chain=%x val=%x\n",
					wlc->stf->txcore_override[OFDM_IDX], wlc->stf->txchain));

				wlc->stf->txcore_override[CCK_IDX] = 0;
				if (wlc->stf->txchain_perrate_state_modify) {
					wlc->stf->txchain_perrate_state_modify(wlc);
				}
			}
		}
	}
	wlc->stf->txchain_pending = 0;
	wlc->stf->txstreams = txstreams;
	wlc_ht_stbc_tx_set(wlc->hti, wlc->band->band_stf_stbc_tx);
	wlc_stf_ss_update(wlc, wlc->bandstate[BAND_2G_INDEX]);
	wlc_stf_ss_update(wlc, wlc->bandstate[BAND_5G_INDEX]);

	if ((wlc->stf->txstreams == 1) &&
		(!WLCISHTPHY(wlc->band) && !WLCISACPHY(wlc->band))) {
		if (wlc->stf->txchain == 1) {
			wlc->stf->txant = ANT_TX_FORCE_0;
		} else if (wlc->stf->txchain == 2) {
			wlc->stf->txant = ANT_TX_FORCE_1;
		} else {
			ASSERT(0);
		}
	} else {
		wlc->stf->txant = ANT_TX_DEF;
	}
	if (wlc->pub->hw_up)
		wlc_suspend_mac_and_wait(wlc);
	/* actually sets the negotiated chains */
	wlc_stf_set_arbitrated_chains_complete(wlc);

#ifdef WLPM_BCNRX
	/* Do at end after phy */
	if (PM_BCNRX_ENAB(wlc->pub)) {
		if (!wlc_pm_bcnrx_allowed(wlc))
			wlc_pm_bcnrx_set(wlc, FALSE); /* Disable */
		else
			wlc_pm_bcnrx_set(wlc, TRUE); /* Enable */
	}
#endif /* WLPM_BCNRX */

#ifdef WL_MIMOPS_CFG
	if (WLC_MIMOPS_ENAB(wlc->pub)) {
		/* Disable SISO rx of mcast packets; active clients may enable in callback below */
		wlc_stf_siso_bcmc_rx(wlc, FALSE);
	}
#endif
	wlc_stf_update_rate_info(wlc);

	if (force_clk) {
		wlc->mpc_out = mpc;
		wlc_radio_mpc_upd(wlc);
	}
	if (wlc->pub->hw_up)
		wlc_enable_mac(wlc);
	return BCME_OK;
}

static void wlc_stf_update_rate_info(wlc_info_t *wlc)
{
	/* Update rateset */
	wlc_rateset_mcs_build(&wlc->default_bss->rateset,
		wlc->stf->rxstreams);

#ifdef WL11N
	if (N_ENAB(wlc->pub))
		wlc_set_nmode(wlc->hti, ON);
#endif
#ifdef WL11AC
	if (VHT_ENAB(wlc->pub)) {
		wlc_vht_set_ac_mode(wlc->vhti);
		wlc_vht_update_mcs_cap(wlc->vhti);
	}
#endif
}

void wlc_stf_arb_call_back(void *ctx, uint8 txc_active, uint8 rxc_active)
{
	wlc_bsscfg_t *bsscfg = (wlc_bsscfg_t*)ctx;
	wlc_info_t	*wlc = bsscfg->wlc;
	wlc_hw_config_t *bsscfg_hw_cfg = wlc_stf_bss_hw_cfg_get(bsscfg);
#ifdef WL_MIMOPS_CFG
	wlc_mimo_ps_cfg_t *bsscfg_mimo_ps_cfg = NULL;
	wlc_hw_config_t *wlc_cfg_hw_cfg = wlc_stf_bss_hw_cfg_get(wlc->cfg);
	uint16 new_chspec;
	uint16 newchspec_bw = WL_CHANSPEC_BW_80;
	uint16 mimo_ps_bw;
#endif
#ifdef WLPM_BCNRX
	bool    enableBCNrx = FALSE;
#endif
	uint8	ntxchains = WLC_BITSCNT(txc_active);

#ifdef WL_MIMOPS_CFG
	if (WLC_MIMOPS_ENAB(wlc->pub)) {
		bsscfg_mimo_ps_cfg = wlc_stf_mimo_ps_cfg_get(bsscfg);
		ASSERT(bsscfg_mimo_ps_cfg);
	}
#endif

#ifdef WL_MIMOPS_CFG
	if (WLC_MIMOPS_ENAB(wlc->pub)) {
		if (bsscfg_mimo_ps_cfg->mrc_chains_changed)
			ntxchains = 1;
	}
	else
#endif /* WL_MIMOPS_CFG */
	{
		if  (WLC_BITSCNT(bsscfg_hw_cfg->current_txchains) == 1)
			ntxchains = 1;
	}

#ifdef WL_MIMOPS_CFG
	if (WLC_MIMOPS_ENAB(wlc->pub))
		WL_STF_ARBITRATOR_TRACE(("ARBI: bsscfg call back idx %d tx_rx_cur_mrc|last %08x\n",
		                         WLC_BSSCFG_IDX(bsscfg),
		                         ((txc_active << 24) | (rxc_active << 16) |
		                         (bsscfg_hw_cfg->current_txchains << 8) |
		                         ((bsscfg_mimo_ps_cfg->mrc_chains_changed ? 1 : 0) << 7) |
		                         bsscfg_hw_cfg->last_ntxchains)));
#endif

	if (ntxchains != bsscfg_hw_cfg->last_ntxchains) {
		wlc_scb_ratesel_init_bss(wlc, bsscfg);
		bsscfg_hw_cfg->last_ntxchains = ntxchains;
	}

#ifdef WLPM_BCNRX
	if (PM_BCNRX_ENAB(wlc->pub)) {
		/* Handle beacon based reception setting */
		if (WLC_BITSCNT(rxc_active) > 1 && wlc_pm_bcnrx_allowed(wlc)) {
			enableBCNrx = TRUE;
		}
		/* Enable or disable BCN RX optimization - if mrc is on disable */
		WL_STF_ARBITRATOR_TRACE(("ARBI: bsscfg call back Setting"
			" wlc_pm_bcnrx_set = %d\n", enableBCNrx));
		/* Do at end after phy */
		wlc_pm_bcnrx_set(wlc, enableBCNrx);
	}

	wlc_stf_siso_bcmc_rx(wlc, bsscfg_hw_cfg->basic_rates_present);

#endif /* WLPM_BCNRX */
	if (BSSCFG_AP(bsscfg)) {
		return;
	}
	if (!bsscfg->associated)
		return;

#if defined(WL_MIMOPS_CFG)
	if (WLC_MIMOPS_ENAB(wlc->pub)) {
		if (bsscfg_mimo_ps_cfg->cfg_update_in_progress ==
		        MIMO_PS_CFG_STATE_INFORM_AP_PENDING) {
			WL_STF_ARBITRATOR_TRACE(("ARBI:Rate init and send AF \n"));
			wlc_stf_handle_mimo_ps_action_frames(bsscfg);
			return;
		}
		if (bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params_valid) {
			mimo_ps_bw = bsscfg_mimo_ps_cfg->mimo_ps_cfg_host_params->bandwidth;
			/* Need to update the bw with curr_bss chanspec for roam case */
			if (bsscfg->assoc->type == AS_ROAM &&
				mimo_ps_bw == MIMO_PS_CFG_IOVAR_BW_FULL) {
				bsscfg_hw_cfg->original_chanspec_bw
					= bsscfg->current_bss->chanspec;
			}
			/* Override iovar mimo_ps_bw since AP does not
			 * support oper mode notification
			 */
			if ((bsscfg->current_bss->flags2 & WLC_BSS_VHT) &&
				!(bsscfg->current_bss->flags2 & WLC_BSS_OPER_MODE)) {
				mimo_ps_bw = MIMO_PS_CFG_IOVAR_BW_FULL;
				WL_STF_MIMO_PS_INFO((
				"wl%d: ARBIT:MIMO PS override host configured BW (AP without OMN)",
				WLCWLUNIT(wlc)));
			}
			newchspec_bw = wlc_stf_mimo_ps_iovarbw2chanspecbw(bsscfg, mimo_ps_bw);
			new_chspec =
			    wf_channel2chspec(wf_chspec_ctlchan(bsscfg->current_bss->chanspec),
			                      newchspec_bw);
			/* No need to change BW if are roaming to same channel */
			if (new_chspec == bsscfg->current_bss->chanspec) {
				WL_STF_MIMO_PS_TRACE(("Skip BW update\n"));
				return;
			}
			bsscfg_hw_cfg->current_chanspec_bw = new_chspec;
			wlc_cfg_hw_cfg->chanspec_override = TRUE;
			wlc_update_bandwidth(wlc, bsscfg, new_chspec);
		}
	}
#endif /* WL_MIMOPS_CFG */
}

void wlc_stf_arb_handle_txfifo_complete(wlc_info_t *wlc)
{
#ifdef WL_MIMOPS_CFG
	if ((WLC_MIMOPS_ENAB(wlc->pub)) &&
		(wlc->stf->bw_update_in_progress)) {
			wlc_update_bandwidth(wlc, wlc->cfg, wlc->stf->pending_bw);
	}
#endif
	if (wlc->stf->arb->txchain_update_in_progress) {
		/* context to avoid recursion */
		wlc_stf_nss_arbitrator(wlc);
	}
}

void
wlc_stf_arb_req_update(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint8 state)
{
	wlc_stf_nss_request_t *bsscfg_arb_req = NULL;
	wlc_hw_config_t *bsscfg_hw_cfg;

	if (WLC_STF_ARB_ENAB(wlc->pub)) {
		bsscfg_arb_req =  wlc_stf_bss_arb_req_get(bsscfg);
		if (bsscfg_arb_req == NULL) {
			WL_STF_ARBITRATOR_TRACE(("ARB_REQ is NULL, state %d\n", state));
			return;
		}

		bsscfg_hw_cfg = wlc_stf_bss_hw_cfg_get(bsscfg);

		wlc_stf_nss_request_update(wlc, bsscfg_arb_req,
		                           state,
		                           bsscfg_hw_cfg->default_txchains,
		                           WLC_BITSCNT(bsscfg_hw_cfg->default_txchains),
		                           bsscfg_hw_cfg->default_rxchains,
		                           WLC_BITSCNT(bsscfg_hw_cfg->default_rxchains));
	}
}

wlc_stf_nss_request_t *
wlc_stf_bss_arb_req_get(wlc_bsscfg_t *bsscfg)
{
	wlc_info_t	*wlc = bsscfg->wlc;
	wlc_stf_arb_mps_info_t *arb_mps_info = wlc->stf->arb_mps_info_hndl;
	wlc_stf_nss_request_t *stf_arb_req = NULL;

	ASSERT(arb_mps_info != NULL);
	stf_arb_req = STF_ARB_BSSCFG_CUBBY(arb_mps_info, bsscfg);
	return stf_arb_req;
}

wlc_hw_config_t *
wlc_stf_bss_hw_cfg_get(wlc_bsscfg_t *bsscfg)
{
	wlc_info_t	*wlc = bsscfg->wlc;
	wlc_stf_arb_mps_info_t *arb_mps_info = wlc->stf->arb_mps_info_hndl;
	wlc_hw_config_t *hw_cfg = NULL;

	ASSERT(arb_mps_info != NULL);

	hw_cfg = STF_HWCFG_BSSCFG_CUBBY(arb_mps_info, bsscfg);
	ASSERT(hw_cfg != NULL);

	return hw_cfg;

}
