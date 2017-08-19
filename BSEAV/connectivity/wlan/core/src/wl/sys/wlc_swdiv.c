/*
 * Software diversity module .c file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */

#include <typedefs.h>
#include <wlc_cfg.h>
#include <osl.h>

#include <bcmutils.h>
#include <bcmendian.h>
#include <siutils.h>
#include <wlioctl.h>
#include <hndd11.h>
#include <d11.h>

#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc_scb.h>
#include <wlc_phy_hal.h>
#include <wlc_assoc.h>

#include <wlc_swdiv.h>
#include <wlc_swdiv_priv.h>

#include <wlc_types.h>
#include <wlc_bmac.h>
#include <event_log.h>
#include <wlc_utils.h>
#include <bcmdevs.h>
#include <wlc.h>
#include <wlc_hw.h>
#include <wlc_hw_priv.h>
#include <wl_export.h>
#include <wlc_scan.h>
#ifdef WLC_TXPWRCAP
#include <phy_txpwrcap_api.h>
#endif
#include <phy_antdiv_api.h>


#ifdef WLC_SW_DIVERSITY

static const char BCMATTACHDATA(rstr_swdiv)[] = "swdiv";
static const char BCMATTACHDATA(rstr_swdiv_en)[] = "swdiv_en";

/* sw diversity capable core bit map info
 * format : uint8, 0x[core7]...[core0]
 * Parameter names:
 * antmap2g_main	D11MAIN MAC 2G band specific bitmap
 * antmap5g_main	D11MAIN MAC 5G band specific bitmap
 * antmap2g_aux		D11AUX MAC 2G band specific bitmap
 * antmap5g_aux		D11AUX MAC 5G band specific bitmap
 */
static const char BCMATTACHDATA(rstr_swdiv_antmap2g_main)[] = "swdiv_antmap2g_main";
static const char BCMATTACHDATA(rstr_swdiv_antmap5g_main)[] = "swdiv_antmap5g_main";
static const char BCMATTACHDATA(rstr_swdiv_antmap2g_aux)[] = "swdiv_antmap2g_aux";
static const char BCMATTACHDATA(rstr_swdiv_antmap5g_aux)[] = "swdiv_antmap5g_aux";
/* HW configuration */
static const char BCMATTACHDATA(rstr_swdiv_gpio)[] = "swdiv_gpio";
static const char BCMATTACHDATA(rstr_swdiv_gpioctrl)[] = "swdiv_gpioctrl";

static const char BCMATTACHDATA(rstr_swdiv_swctrl_en)[] = "swdiv_swctrl_en";
static const char BCMATTACHDATA(rstr_swdiv_swctrl_mask)[] = "swdiv_swctrl_mask";
static const char BCMATTACHDATA(rstr_swdiv_swctrl_ant0)[] = "swdiv_swctrl_ant0";
static const char BCMATTACHDATA(rstr_swdiv_swctrl_ant1)[] = "swdiv_swctrl_ant1";
static const char BCMATTACHDATA(rstr_swdiv_log2avg)[] = "swdiv_log2avg";
/* BSSCFG Tunables */
static const char BCMATTACHDATA(rstr_swdiv_snrthresh)[] = "swdiv_snrthresh";
static const char BCMATTACHDATA(rstr_swdiv_thresh)[] = "swdiv_thresh";
static const char BCMATTACHDATA(rstr_swdiv_settle)[] = "swdiv_settle";
static const char BCMATTACHDATA(rstr_swdiv_snrlim)[] = "swdiv_snrlim";
static const char BCMATTACHDATA(rstr_swdiv_ccksnrcorr)[] = "swdiv_ccksnrcorr";	/* obsoleted */
static const char BCMATTACHDATA(rstr_swdiv_timeout)[] = "swdiv_timeout";
static const char BCMATTACHDATA(rstr_swdiv_weight)[] = "swdiv_weight";
static const char BCMATTACHDATA(rstr_swdiv_tx_weight)[] = "swdiv_tx_weight";
static const char BCMATTACHDATA(rstr_swdiv_snrdrop_delta)[] = "swdiv_snrdrop_delta";
static const char BCMATTACHDATA(rstr_swdiv_snrdrop_prd)[] = "swdiv_snrdrop_prd";
static const char BCMATTACHDATA(rstr_swdiv_snrdrop_lmt)[] = "swdiv_snrdrop_lmt";
static const char BCMATTACHDATA(rstr_swdiv_poll_prd)[] = "swdiv_poll_prd";
static const char BCMATTACHDATA(rstr_swdiv_snrdrop_txfail)[] = "swdiv_snrdrop_txfail";

/* iovar table */
enum {
	IOV_SWDIV_RX_POLICY =	0,
	IOV_SWDIV_TX_POLICY =	1,
	IOV_SWDIV_STATS =		2,
	IOV_SWDIV_RESET_STATS =	3,
	IOV_SWDIV_TX_WEIGHT =	4,
	IOV_SWDIV_CELL_POLICY =	5,
	IOV_SWDIV_LAST			/* end */
};

/* sw diversity module IOVARs */
static const bcm_iovar_t swdiv_iovars[] = {
	{"swdiv_rx_policy", IOV_SWDIV_RX_POLICY,
	(IOVF_SET_CLK), 0, IOVT_UINT32, 0
	},
	{"swdiv_tx_policy", IOV_SWDIV_TX_POLICY,
	(IOVF_SET_CLK), 0, IOVT_UINT32, 0
	},
	{"swdiv_stats", IOV_SWDIV_STATS,
	(IOVF_OPEN_ALLOW | IOVF_GET_UP), 0, IOVT_BUFFER, sizeof(wlc_swdiv_stats_t)
	},
	{"swdiv_reset_stats", IOV_SWDIV_RESET_STATS,
	(0), 0, IOVT_INT32, 0
	},
	{"swdiv_tx_weight", IOV_SWDIV_TX_WEIGHT,		/* test purpose - can be removed */
	(0), 0, IOVT_INT32, 0
	},
	{"swdiv_cell_policy", IOV_SWDIV_CELL_POLICY,
	(IOVF_SET_CLK), 0, IOVT_UINT32, 0
	},
	{NULL, 0, 0, 0, 0, 0}
};

#ifdef BCMDBG
static swdiv_evtname_tbl_t evttbl[] = {
		{"EVT_IDLE"},
		{"EVT_RXCNT"},
		{"EVT_SNRTHRSH"},
		{"EVT_TXFAIL"},
		{"EVT_TIMER"},
		{"EVT_WATCHDOG"},
		{"EVT_SNRDROP"}
};
#endif /* BCMDBG */

/* this includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/* Internal function prototypes */
static int wlc_swdiv_up(void *ctx);
static void wlc_swdiv_params_init(wlc_swdiv_info_t * swdiv);
static int wlc_swdiv_corelist_init(wlc_swdiv_info_t *swdiv);
static void wlc_swdiv_corelist_deinit(wlc_swdiv_info_t *swdiv);
static void wlc_swdiv_plcytype_init(wlc_swdiv_info_t *swdiv);
static void wlc_swdiv_watchdog(void *hdl);
static void wlc_swdiv_gpio_nvram_get(wlc_swdiv_info_t *swdiv);
static void wlc_swdiv_timer(void *arg);
static int wlc_swdiv_doiovar(void *hdl, uint32 actionid,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);
static void wlc_swdiv_bsscfg_up_down(void *ctx,
	bsscfg_up_down_event_data_t *evt_data);
static void wlc_swdiv_assoc_state_change_cb(void *ctx,
	bss_assoc_state_data_t *notif_data);
static void wlc_swdiv_rxbcn_recv_cb(void *ctx, bss_rx_bcn_notif_data_t *data);
static void wlc_swdiv_corelist_append(swdiv_core_entity_t **pphead,
	swdiv_core_entity_t *pinput);
static uint32 wlc_swdiv_coremap_select(wlc_swdiv_info_t *swdiv, uint8 reqtype);
static void wlc_swdiv_snr_compute(wlc_swdiv_info_t *swdiv, d11rxhdr_t *rxh,
	swdiv_scb_cubby_t * scb,  int8 *rssi, int32 *snr_out, bool isCCK);
static void wlc_swdiv_corelist_coremap_sync(wlc_swdiv_info_t *swdiv,
	uint16 activemap);
static void wlc_swdiv_corelist_antmap_sync(wlc_swdiv_info_t *swdiv,
	uint16 ucode_antmap);
static void wlc_swdiv_antmap_shmem_update(wlc_swdiv_info_t *swdiv,
	swdiv_scb_cubby_t * scbcb, uint16 rxantmap);
static uint16 wlc_swdiv_core_mapping(
	wlc_swdiv_info_t *swdiv, swdiv_bsscfg_cubby_t *cfgcb,
	swdiv_scb_cubby_t *scbcb, uint16 coremap, uint16 antmap, uint8 band);
static bool wlc_swdiv_diversity_engine(swdiv_bsscfg_cubby_t *cfgcb,
	swdiv_scb_cubby_t * scbcb, int idx, int sidx, int antidx);
static int32 wlc_swdiv_get_stats(wlc_swdiv_info_t *swdiv,
	wlc_swdiv_stats_t *swdiv_stats);
static void wlc_swdiv_scb_stats_reset(wlc_swdiv_info_t *swdiv, struct scb *scb);
static int wlc_swdiv_bitpos_get(uint8 input);
static bool wlc_swdiv_snrbased_swap_check(swdiv_bsscfg_cubby_t *cfgcb,
	swdiv_scb_cubby_t *scbcb, int sidx);
static bool wlc_swdiv_rxpktbased_swap_check(swdiv_scb_cubby_t *scbcb, int sidx);
static int32 wlc_swdiv_policy_mapping(int coreidx,
	uint32 rx_policy,
	uint32 tx_policy,
	uint32 cell_policy,
	uint8 *rxp,
	uint8 *txp,
	uint8 *cellp_on,
	uint8 *cellp_off);
static void wlc_swdiv_antpref_conf_set(wlc_swdiv_info_t *swdiv,
	swdiv_requester_type_t reqid, uint16 rxantpref2g, uint16 rxantpref5g,
	uint16 txantpref2g, uint16 txantpref5g, bool isSet);
static uint32 wlc_swdiv_ltecoex_antpref_integrate(uint16 rxantpref2g,
	uint16 rxantpref5g, uint16 txantpref2g, uint16 txantpref5g);
static int32 wlc_swdiv_corelist_policy_priotize(swdiv_core_entity_t *clist,
	bool cellstatus,
	uint8 band, uint8 *rxp, uint8 *txp, uint8 *cellp_on, uint8 *cellp_off);
static void wlc_swdiv_swctrl_type_set(wlc_hw_info_t *wlc_hw, uint type);
static void wlc_swdiv_gpio_init(wlc_hw_info_t *wlc_hw,
	bool ucodectrl, uint16 defenable);
static uint32 wlc_swdiv_antmap_to_gpio_convert(wlc_info_t *wlc,
	uint16 antmap, uint32 gpiomask);
static bool wlc_swdiv_recvpkt_valid_check(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh);
static int32 wlc_swdiv_policy_validchk(
	uint8 rxplcy, uint8 txplcy, uint8 cellplcy_on, uint8 cellplcy_off);
static uint16 wlc_swdiv_plcy_to_antmap_conv(wlc_swdiv_info_t *swdiv, int band);
static int32 wlc_swdiv_antsel_select(swdiv_core_entity_t *clist,
	uint8 band, int idx, bool isswap, uint16 *rxantmap);
static void wlc_swdiv_rxpkt_scbstats_update(wlc_swdiv_info_t *swdiv,
	swdiv_scb_cubby_t *scbcb,
	uint16 antmap, int32 *snr, int8 *rssi, bool isCCK);
static void wlc_swdiv_current_ant_get(wlc_info_t *wlc, uint16 *rxantmap);
static swdiv_core_entity_t * wlc_swdiv_corelist_byidx_get(wlc_swdiv_info_t *swdiv,
	int coreidx);
static uint8 wlc_swdiv_antpref_to_plcy_conv(uint8 pref);
static int32 wlc_swdiv_rx_policy_get(wlc_swdiv_info_t *swdiv, int32 *retval);
static int32 wlc_swdiv_cell_policy_get(wlc_swdiv_info_t *swdiv, int32 *retval);
static uint16 wlc_swdiv_linkmon_dur_get(wlc_info_t *wlc, wlc_bsscfg_t* bsscfg);
static bool wlc_swdiv_band_valid_chk(wlc_bsscfg_t *cfg);


/* function declaration */
wlc_swdiv_info_t *
BCMATTACHFN(wlc_swdiv_attach)(wlc_info_t* wlc)
{
	wlc_swdiv_info_t *swdiv;
	scb_cubby_params_t cubby_params;
	osl_t *osh = wlc->osh;
	wlc_pub_t *pub = wlc->pub;

	WL_SWDIV(("wl%d: %s\n", wlc->pub->unit, __FUNCTION__));

	/* no need to make swdiv module as a singleton type
	 * each wlc instance will get its own swdiv corelist based on
	 * antmap2g/5g_main, antmap2g/5g_aux NVRAM
	 */
	if (!(swdiv = (wlc_swdiv_info_t *)MALLOCZ(osh, sizeof(wlc_swdiv_info_t)))) {
		WL_ERROR(("wl%d: %s: MALLOC failed; total mallocs %d bytes\n",
		          WLCWLUNIT(wlc), __FUNCTION__, MALLOCED(osh)));
		return NULL;
	}
	swdiv->wlc = wlc;
	swdiv->pub = pub;
	swdiv->wlc->osh = osh;

	/* register module */
	if (wlc_module_register(wlc->pub, swdiv_iovars, rstr_swdiv, swdiv,
		(wlc_iov_disp_fn_t)wlc_swdiv_doiovar,
		(watchdog_fn_t)wlc_swdiv_watchdog,
		(up_fn_t)wlc_swdiv_up, NULL)) {
		WL_ERROR(("wl%d: swdiv wlc_swdiv_iovar_attach failed\n", wlc->pub->unit));
		goto fail;
	}

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((swdiv->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(swdiv_bsscfg_cubby_t *),
	                wlc_swdiv_bsscfg_init, wlc_swdiv_bsscfg_deinit, wlc_swdiv_bsscfg_dump,
	                (void *)swdiv)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* reserve cubby in the scb container for per-scb private data */
	bzero(&cubby_params, sizeof(cubby_params));

	cubby_params.context = swdiv;
	cubby_params.fn_init = wlc_swdiv_scb_init;
	cubby_params.fn_deinit = wlc_swdiv_scb_deinit;
	cubby_params.fn_update = wlc_swdiv_scb_update;
	cubby_params.fn_secsz = wlc_swdiv_scb_secsz;
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	cubby_params.fn_dump = wlc_swdiv_scb_dump;
#endif /* BCMDBG || BCMDBG_DUMP */

	if ((swdiv->scbh = wlc_scb_cubby_reserve_ext(wlc, sizeof(swdiv_scb_cubby_t *),
	                &cubby_params)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_scb_cubby_reserve_ext() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	/* register bsscfg up/down callbacks */
	if (wlc_bsscfg_updown_register(wlc, wlc_swdiv_bsscfg_up_down, swdiv) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_updown_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	/* register assoc state change callback */
	if (wlc_bss_assoc_state_register(wlc, wlc_swdiv_assoc_state_change_cb, swdiv)
		!= BCME_OK) {
		WL_ERROR(("wl%d: wlc_bss_assoc_state_register() failed\n", wlc->pub->unit));
		goto fail;
	}
	/* register beacon rx notif. callback, no need to handover swdiv here */
	if (wlc_bss_rx_bcn_register(wlc, wlc_swdiv_rxbcn_recv_cb, wlc)
		!= BCME_OK) {
		WL_ERROR(("wl%d: wlc_bss_rx_bcn_register() failed\n", wlc->pub->unit));
		goto fail;
	}
	/* Set enab flag for phy as well */
	wlc_swdiv_params_init(swdiv);
	wlc_swdiv_corelist_init(swdiv);
	wlc_swdiv_plcytype_init(swdiv);
	wlc->pub->_swdiv = TRUE;

	return swdiv;
fail:
	wlc_swdiv_detach(swdiv);
	return NULL;
}

void
BCMATTACHFN(wlc_swdiv_detach)(wlc_swdiv_info_t* swdiv)
{
	wlc_info_t* wlc;

	if (swdiv == NULL)
		return;
	wlc = swdiv->wlc;

	WL_SWDIV(("wl%d: %s\n", wlc->pub->unit, __FUNCTION__));

	/* un-register beacon rx notif. callback */
	wlc_bss_rx_bcn_unregister(wlc, wlc_swdiv_rxbcn_recv_cb, wlc);
	/* unregister assoc state change callback */
	wlc_bss_assoc_state_unregister(wlc, wlc_swdiv_assoc_state_change_cb, swdiv);
	/* unregister bsscfg up/down callbacks */
	wlc_bsscfg_updown_unregister(wlc, wlc_swdiv_bsscfg_up_down, swdiv);
	/* clean up the corechain list */
	wlc_swdiv_corelist_deinit(swdiv);
	/* module unregister */
	wlc_module_unregister(wlc->pub, rstr_swdiv, swdiv);
	MFREE(wlc->osh, swdiv, sizeof(wlc_swdiv_info_t));
}

static int
wlc_swdiv_doiovar(void *hdl, uint32 actionid,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_info_t *wlc = ((wlc_swdiv_info_t *)hdl)->wlc;
	int32 int_val = 0;
	int32 int_val2 = 0;
	int32 *ret_int_ptr;
	int err = 0;

	ret_int_ptr = (int32 *)a;
	if (plen >= (int)sizeof(int_val)) {
		bcopy(p, &int_val, sizeof(int_val));
	}
	if (plen >= (int)sizeof(int_val) * 2) {
		bcopy((void*)((uintptr)p + sizeof(int_val)), &int_val2, sizeof(int_val));
	}
	switch (actionid) {
		case IOV_GVAL(IOV_SWDIV_STATS):
		{
			wlc_swdiv_stats_t stats;
			err = wlc_swdiv_get_stats(wlc->swdiv, &stats);
			if (err == BCME_OK) {
				bcopy(&stats, a, sizeof(wlc_swdiv_stats_t));
			}
			break;
		}
		case IOV_GVAL(IOV_SWDIV_RESET_STATS):
		{
			wlc_swdiv_reset_stats(wlc->swdiv, NULL);
			*ret_int_ptr = 0;
			break;
		}
		case IOV_SVAL(IOV_SWDIV_TX_WEIGHT):
		{
			err = BCME_UNSUPPORTED;
			break;
		}
		case IOV_GVAL(IOV_SWDIV_TX_WEIGHT):
		{
			*ret_int_ptr = 0;
			break;
		}
		case IOV_SVAL(IOV_SWDIV_CELL_POLICY):
		{
			err = wlc_swdiv_policy_req(wlc->swdiv, wlc->swdiv->policy.rx,
				SWDIV_POLICY_INVALID,
				SWDIV_POLICY_INVALID, SWDIV_POLICY_INVALID,
				(uint32)int_val, SWDIV_POLICY_INVALID);
			break;
		}
		case IOV_GVAL(IOV_SWDIV_CELL_POLICY):
		{
			err = wlc_swdiv_cell_policy_get(wlc->swdiv, ret_int_ptr);
			break;
		}
		case IOV_SVAL(IOV_SWDIV_RX_POLICY):
		{
			err = wlc_swdiv_policy_req(wlc->swdiv, (uint32)int_val,
				SWDIV_POLICY_INVALID,
				SWDIV_POLICY_INVALID, SWDIV_POLICY_INVALID,
				wlc->swdiv->policy.cell, SWDIV_POLICY_INVALID);
			break;
		}
		case IOV_GVAL(IOV_SWDIV_RX_POLICY):
		{
			err = wlc_swdiv_rx_policy_get(wlc->swdiv, ret_int_ptr);
			break;
		}
		case IOV_SVAL(IOV_SWDIV_TX_POLICY):
		{
			err = BCME_UNSUPPORTED;
			break;
		}
		case IOV_GVAL(IOV_SWDIV_TX_POLICY):
		{
			err = BCME_UNSUPPORTED;
			break;
		}
		default:
			err = BCME_UNSUPPORTED;
	}
	return err;
}

int
wlc_swdiv_up(void *ctx)
{
	int err = 0;
	wlc_swdiv_info_t *swdiv = (wlc_swdiv_info_t *)ctx;
	wlc_info_t *wlc = swdiv->wlc;
	if (swdiv->enable) {
		if (swdiv->swctrl_en == SWDIV_SWCTRL_0) {
			wlc_swdiv_swctrl_type_set(wlc->hw, SWDIV_SWCTRL_SHMEM_GPIO);
			wlc_swdiv_gpio_init(wlc->hw, TRUE, swdiv->gpio_ctrl);
		} else if (swdiv->swctrl_en == SWDIV_SWCTRL_1 ||
			swdiv->swctrl_en == SWDIV_SWCTRL_2) {
			wlc_swdiv_swctrl_type_set(wlc->hw, SWDIV_SWCTRL_SHMEM_SWCTRL);
		}
	} else {
		wlc_swdiv_swctrl_type_set(wlc->hw, SWDIV_SWCTRL_SHMEM_OFF);
	}
	return err;
}

/* --------------------------------------------
 * External API Function declaration
 * ---------------------------------------------
 */

void
wlc_swdiv_enable_set(wlc_swdiv_info_t *swdiv, bool enable)
{
	ASSERT(swdiv != NULL);
	swdiv->enable = enable;
}

void
wlc_swdiv_rxpkt_recv(wlc_swdiv_info_t *swdiv, struct scb *scb, wlc_d11rxhdr_t *wrxh, bool isCCK)
{
	wlc_bsscfg_t * bsscfg;
	d11rxhdr_t *rxh;
	wlc_info_t *wlc;
	swdiv_bsscfg_cubby_t *cfgcb;
	swdiv_scb_cubby_t *scbcb;
	int32 snr[MAX_UCODE_ANTBITS];
	int8 rssi[MAX_UCODE_ANTBITS];
	int sidx = 0, last_antbit = 0;
	int idx = 0;
	bool psm_update = FALSE;
	uint16 coremap;
	uint16 antmap = 0, new_antmap;
	chanspec_t chspec_info;
	uint8 band;
	swdiv_core_entity_t *clist;

	ASSERT(swdiv);
	ASSERT(scb);
	ASSERT(wrxh);

	wlc = swdiv->wlc;
	bsscfg = scb->bsscfg;
	rxh = &wrxh->rxhdr;
	ASSERT(wlc);
	ASSERT(bsscfg);

	cfgcb = SWDIV_BSSCFG_CUBBY(swdiv, bsscfg);
	scbcb = SWDIV_SCB_CUBBY(swdiv, scb);
	ASSERT(cfgcb);
	ASSERT(scbcb);
	/* chip limitation exception  */
	ASSERT(WLCISACPHY(wlc->band));

	clist = swdiv->corelist;
	chspec_info = scb->bsscfg->current_bss->chanspec;
	coremap = ACPHY_COREMAP(rxh);
	antmap = ACPHY_ANTMAP(rxh);

	/* pkt validity check */
	if (!wlc_swdiv_recvpkt_valid_check(wlc, wrxh)) {
			return;
		}
	/* Timer kill if any rxpkt is updated */
	if (scbcb->linkmon->ptimer) {
		wl_del_timer(wlc->wl, scbcb->linkmon->ptimer);
		scbcb->linkmon->is_timerset = FALSE;
	}
	/* top control flag check */
	if (!SWDIV_IS_ENABLED(swdiv)) {
		return;
	}
	/* exception handling for antmap update */
	if (scbcb->stats->lastantmap == SWDIV_NULL_ANTMAP) {
		/* initial recv pkt */
		scbcb->stats->lastantmap = antmap;
	}
	/* antmap sync check */
	if ((swdiv->userantmap != SWDIV_NULL_ANTMAP) &&
		(swdiv->userantmap == antmap)) {
		psm_update = TRUE;
		swdiv->userantmap = SWDIV_NULL_ANTMAP;
		/* OK user request now synced. any action? */
	} else if ((scbcb->stats->reqantmap != SWDIV_NULL_ANTMAP) &&
		(scbcb->stats->reqantmap == antmap)) {
		psm_update = TRUE;
		scbcb->stats->reqantmap = SWDIV_NULL_ANTMAP;
	} else if (scbcb->stats->lastantmap != antmap) {
		psm_update = TRUE;
	}
	/* update snr info */
	bzero(snr, MAX_UCODE_ANTBITS);
	bzero(rssi, MAX_UCODE_ANTBITS);
	FOREACH_SWDIV_COREMAP(coremap, idx) {
		if (SWDIV_ISBIT_SET_ON_MAP(coremap, idx)) {
			/* RSSI gathering */
			rssi[idx] = wrxh->rxpwr[idx];
			if (rssi[idx] == WLC_RSSI_INVALID) {
				WL_ERROR(("wl%d: %s: Invalid RSSI on active core%d\n",
					wlc->pub->unit, __FUNCTION__, idx));
				return;
			}
			rssi[idx] = LIMIT_TO_MAX(rssi[idx], MAX_RSSI_CAP);
			rssi[idx] = LIMIT_TO_MIN(rssi[idx], MIN_RSSI_CAP);
		} else {
			rssi[idx] = WLC_RSSI_INVALID;
		}
	}
	wlc_swdiv_snr_compute(swdiv, rxh, scbcb, rssi, snr, isCCK);

	/* sync uCode change */
	if (coremap != scbcb->stats->lastcoremap) {
		wlc_swdiv_corelist_coremap_sync(swdiv, coremap);
	}
	if (psm_update) {
		wlc_swdiv_corelist_antmap_sync(swdiv, antmap);
		/* reset rxpkt count only if any update on the core */
		FOREACH_SWDIV_ANTMAP(antmap, idx) {
			last_antbit = SWDIV_GETBIT_BY_MAP(scbcb->stats->lastantmap, idx);
			if (SWDIV_GETBIT_BY_MAP(antmap, idx) != last_antbit) {
				sidx = SWDIV_GETBASE_SIDX(idx);
				scbcb->rxpktcnt_per_ant[sidx] = 0;
				scbcb->rxpktcnt_prev_per_ant[sidx] = 0;
				scbcb->rxpktcnt_per_ant[sidx + 1] = 0;
				scbcb->rxpktcnt_prev_per_ant[sidx + 1] = 0;
			}
		}
	}
	/* update statistics per-scb */
	wlc_swdiv_rxpkt_scbstats_update(swdiv, scbcb, antmap, snr, rssi, isCCK);

	band = (CHSPEC_IS2G(scb->bsscfg->current_bss->chanspec)) ?
		SWDIV_BANDSEL_2G : SWDIV_BANDSEL_5G;
	/* start swdiv algo */
	new_antmap = wlc_swdiv_core_mapping(swdiv, cfgcb, scbcb, coremap, antmap, band);
	if (new_antmap != antmap) {
		scbcb->stats->reqantmap = new_antmap;
	}
	scbcb->stats->lastcoremap = coremap;
	scbcb->stats->lastantmap = antmap;
	scbcb->stats->lastchanspec = chspec_info;
	if (scbcb->settlecnt_rx <= cfgcb->tune.settle_rxpktcnt_limit) {
		scbcb->settlecnt_rx++;
	}
	clist = swdiv->corelist;
	FOREACH_SWDIV_ACTCORE(clist) {
		idx = wlc_swdiv_bitpos_get(clist->coreid);
		if (idx < 0) {
			return;
		}
		scbcb->polltime_elapsed[idx] = 0;
	}
	scbcb->linkmon->is_snrdropped = FALSE;
	wlc_swdiv_antmap_shmem_update(swdiv, scbcb, new_antmap);
}

void
wlc_swdiv_reset_stats(wlc_swdiv_info_t *swdiv, struct scb *scb)
{
	wlc_info_t *wlc;
	wlc_bsscfg_t *cfgiter, *cfg;
	swdiv_scb_cubby_t *scbcb;
	struct scb_iter scbiter;
	int idx = 0;

	ASSERT(swdiv);
	wlc = swdiv->wlc;
	ASSERT(wlc);

	if (scb == NULL) {
		/* coming from IOVAR */
		FOREACH_BSS(wlc, idx, cfgiter) {
			if (WLSWDIV_ENAB(wlc) && wlc_swdiv_bss_is_supported(cfgiter)) {
				FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfgiter, scb) {
					/* currently DVT only support single link */
					/* TODO: need to switch to multi-scb stats soon */
					if (scb) {
						wlc_swdiv_scb_stats_reset(swdiv, scb);
					}
				}
			}
		}
	} else {
		/* link creation */
		cfg = SCB_BSSCFG(scb);
		scbcb = SWDIV_SCB_CUBBY(swdiv, scb);
		wlc_swdiv_scb_stats_reset(swdiv, scb);
		scbcb->linkmon_dur = wlc_swdiv_linkmon_dur_get(wlc, cfg);
	}
}

/* each antpref consists of :  0x[core7,15:14]...[core1,3:2][core0,1:0]
 * each 2bits indicates:
 * 00 : turn off both ants (ALLOFF)
 * 01 : turn on 1st ant only (FORCE_0)
 * 10 : turn on 2nd ant only (FORCE_1)
 * 11 : turn on both ants (AUTO)
 */
int32
wlc_swdiv_antpref_update(wlc_swdiv_info_t *swdiv, swdiv_requester_type_t reqid,
	uint16 rxantpref2g,
	uint16 rxantpref5g,
	uint16 txantpref2g,
	uint16 txantpref5g)
{
	wlc_info_t *wlc;
	swdiv_core_entity_t *clist;
	uint8 rxplcy_type[MAX_BAND_TYPES];
	uint8 txplcy_type[MAX_BAND_TYPES];
	int bandidx, idx, tempidx;
	uint16 rxantmap_new = 0;
	int32 ret = BCME_OK;

	ASSERT(swdiv);
	wlc = swdiv->wlc;
	ASSERT(wlc);
	clist = swdiv->corelist;
	bandidx = (BAND_2G(wlc->band->bandtype)) ? SWDIV_BANDSEL_2G : SWDIV_BANDSEL_5G;

	wlc_swdiv_antpref_conf_set(swdiv, reqid, rxantpref2g, rxantpref5g,
		txantpref2g, txantpref5g, TRUE);

	FOREACH_SWDIV_CORE(clist) {
		idx = wlc_swdiv_bitpos_get(clist->coreid);
		if (idx < 0) {
			ret = BCME_NOTFOUND;
			goto failcase;
		}
		rxplcy_type[SWDIV_BANDSEL_2G] =
			wlc_swdiv_antpref_to_plcy_conv(SWDIV_GET_ANTPREF(rxantpref2g, idx));
		txplcy_type[SWDIV_BANDSEL_2G] =
			wlc_swdiv_antpref_to_plcy_conv(SWDIV_GET_ANTPREF(txantpref2g, idx));
		rxplcy_type[SWDIV_BANDSEL_5G] =
			wlc_swdiv_antpref_to_plcy_conv(SWDIV_GET_ANTPREF(rxantpref5g, idx));
		txplcy_type[SWDIV_BANDSEL_5G] =
			wlc_swdiv_antpref_to_plcy_conv(SWDIV_GET_ANTPREF(txantpref5g, idx));

		for (tempidx = 0; tempidx < MAX_BAND_TYPES; tempidx++) {
			if (((clist->support_2g) && (tempidx == SWDIV_BANDSEL_2G)) ||
				((clist->support_5g) && (tempidx == SWDIV_BANDSEL_5G))) {
				ret = wlc_swdiv_policy_validchk(rxplcy_type[tempidx],
					txplcy_type[tempidx],
					clist->cellplcy_on[tempidx],
					clist->cellplcy_off[tempidx]);
				if (ret != BCME_OK) {
					WL_ERROR(("%s: invalid policy err(%d)\n",
						__FUNCTION__, ret));
					goto failcase;
				}
				/* mws subset calculation. assume that no conflict between two.
				 * eg., RX:FORCE_0 and TX:FORCE_1 cannot be handled
				 */
				clist->mwsplcy[bandidx] =
						rxplcy_type[bandidx] & txplcy_type[bandidx];
				if (clist->mwsplcy[bandidx] == SWDIV_POLICY_INVALID) {
					WL_ERROR(("swdiv_antpref_update(): input param invalid\n"));
					ret = BCME_RANGE;
					goto failcase;
				} else if (clist->mwsplcy[bandidx] == SWDIV_POLICY_AUTO) {
					/* mws unset the antpref, follow swdiv policy info */
					clist->mwsplcy[bandidx] = SWDIV_POLICY_INVALID;
					/* go back to default set */
					rxplcy_type[bandidx] = SWDIV_POLICY_AUTO;
				}
			}
		}

		ret = wlc_swdiv_corelist_policy_priotize(clist, swdiv->cellstatus, bandidx,
			rxplcy_type, txplcy_type, clist->cellplcy_on, clist->cellplcy_off);
		if (ret != BCME_OK) {
			return ret;
		}
		ret = wlc_swdiv_antsel_select(clist, bandidx, idx, FALSE, &rxantmap_new);
		if (ret != BCME_OK) {
			return ret;
		}
	}
	swdiv->userantmap = rxantmap_new;
	wlc_swdiv_scbcubby_stat_reset(swdiv);
	wlc_swdiv_antmap_shmem_update(swdiv, NULL, rxantmap_new);
	return ret;
failcase:
	wlc_swdiv_antpref_conf_set(swdiv, reqid, 0, 0, 0, 0, FALSE);
	for (tempidx = 0; tempidx < MAX_BAND_TYPES; tempidx++) {
		clist->mwsplcy[bandidx] = SWDIV_POLICY_INVALID;
	}
	return ret;
}

void
wlc_swdiv_txfail(wlc_swdiv_info_t *swdiv, struct scb *scb)
{
	wlc_info_t *wlc;
	wlc_bsscfg_t *bsscfg = NULL;
	swdiv_bsscfg_cubby_t *cfgcb;
	swdiv_scb_cubby_t *scbcb;
	swdiv_core_entity_t *clist;
	int idx = 0, sidx = 0;
	uint16 rxantmap = 0;
	uint8 band;

	ASSERT(swdiv);
	ASSERT(scb);
	wlc = swdiv->wlc;
	bsscfg = SCB_BSSCFG(scb);
	ASSERT(wlc);
	ASSERT(bsscfg);

	if (!SWDIV_IS_ENABLED(swdiv)) {
		return;
	}
	cfgcb = SWDIV_BSSCFG_CUBBY(swdiv, bsscfg);
	scbcb = SWDIV_SCB_CUBBY(swdiv, scb);
	clist = swdiv->corelist;
	band = (CHSPEC_IS2G(bsscfg->current_bss->chanspec)) ?
		SWDIV_BANDSEL_2G : SWDIV_BANDSEL_5G;
	wlc_swdiv_current_ant_get(wlc, &rxantmap);
	FOREACH_SWDIV_CORE(clist) {
		idx = wlc_swdiv_bitpos_get(clist->coreid);
		if (idx < 0) {
			break;
		}
		/* syncup first with the current uCode update */
		clist->rxsel.cur = SWDIV_GETBIT_BY_MAP(rxantmap, idx);
		scbcb->stats->lastantmap = rxantmap;
		sidx = SWDIV_GETSIDX(clist->rxsel.cur, idx);
		scbcb->stats->swaprsn_cur = SWDIV_EVENT_TXFAIL;
		wlc_swdiv_antsel_select(clist, band, idx, TRUE, &rxantmap);
		/* reduce SNR */
		scbcb->snr_avg_per_ant[sidx] -=
			cfgcb->tune.snrdec_upon_txfail;
		scbcb->snr_sum_per_ant[sidx] -=
			(cfgcb->tune.snrdec_upon_txfail
				<< swdiv->log2_num_pkt_avg);
		if (scbcb->snr_avg_per_ant[sidx] < 0) {
			scbcb->snr_sum_per_ant[sidx] = 0;
			scbcb->snr_avg_per_ant[sidx] = 0;
		}
		scbcb->stats->swap_txfail[sidx]++;
		if (clist->active) {
			scbcb->polltime_elapsed[idx] = 0;
		}
	} /* end of SWDIV_CORE loop */
	if (scbcb->linkmon->ptimer) {
		if (scbcb->linkmon->is_timerset) {
			wl_del_timer(wlc->wl, scbcb->linkmon->ptimer);
		}
		wl_add_timer(wlc->wl, scbcb->linkmon->ptimer, scbcb->linkmon_dur, FALSE);
		scbcb->linkmon->is_timerset = TRUE;
	}
	scbcb->linkmon->is_snrdropped = FALSE;
	if (rxantmap != scbcb->stats->lastantmap) {
		wlc_swdiv_antmap_shmem_update(swdiv, scbcb, rxantmap);
		WL_SWDIV(("%s: swap from lastantmap=0x%x to newant_map=0x%x\n", __FUNCTION__,
			scbcb->stats->lastantmap, rxantmap));
	}
}

/* ant selection policy control api
 * format : each core has 8bits 0x[core3]...[core0]
 * rx_policy/tx_policy has 8bits definitions per-core bits like as :
 * [3:0] - 2g policy type id
 * [7:4] - 5g policy type id
 * e.g., 0x11 val sets policy type 0x1 to both band for core0.
 * cell_policy has 8bits definitions per-core bits like as :
 * [1:0] - cell on, 2g policy type id
 * [3:2] - cell on, 5g policy type id
 * [5:4] - cell off, 2g policy type id
 * [7:6] - cell off, 5g policy type id
 * e.g., 0x44 val sets type 0x0 on 2G, and set type 0x1 on 5G
 *  to core0 and core1 respectively in both cell on and off condition.
 * xx_policy_ext params are reserved for core4~core7
 */
int32
wlc_swdiv_policy_req(wlc_swdiv_info_t *swdiv,
	uint32 rx_policy,
	uint32 rx_policy_ext,
	uint32 tx_policy,
	uint32 tx_policy_ext,
	uint32 cell_policy,
	uint32 cell_policy_ext)
{
	wlc_info_t *wlc;
	swdiv_core_entity_t *clist;
	uint8 band;
	int idx = 0;
	int32 ret = BCME_OK;
	uint8 rxp[MAX_BAND_TYPES];
	uint8 cellp_on[MAX_BAND_TYPES];
	uint8 cellp_off[MAX_BAND_TYPES];
	uint16 rxantmap_new = 0;
	int32 cellstatus;

	UNUSED_PARAMETER(tx_policy);
	ASSERT(swdiv);
	wlc = swdiv->wlc;
	ASSERT(wlc);
	if (!WLCISACPHY(wlc->band)) {
		return BCME_UNSUPPORTED;
	}
	clist = swdiv->corelist;
	cellstatus = swdiv->cellstatus;
	band = (BAND_2G(wlc->band->bandtype)) ? SWDIV_BANDSEL_2G : SWDIV_BANDSEL_5G;

	WL_SWDIV(("%s: rx_policy=0x%x, cell_policy=0x%x\n", __FUNCTION__,
		rx_policy, cell_policy));
	if ((rx_policy != SWDIV_POLICY_INVALID) &&
		(swdiv->policy.rx == rx_policy) &&
		(cell_policy != SWDIV_POLICY_INVALID) &&
		(swdiv->policy.cell == cell_policy)) {
		/* nothing need to be updated */
		return BCME_OK;
	}

	FOREACH_SWDIV_CORE(clist) {
		idx = wlc_swdiv_bitpos_get(clist->coreid);
		if (idx < 0) {
			return BCME_NOTFOUND;
		}
		ret = wlc_swdiv_policy_mapping(idx, rx_policy, 0, cell_policy,
			rxp, 0, cellp_on, cellp_off);
		if (ret != BCME_OK) {
			WL_ERROR(("%s: invalid policy err(%d)\n", __FUNCTION__, ret));
			return ret;
		}
		ret = wlc_swdiv_corelist_policy_priotize(clist,
			cellstatus, band, rxp, 0, cellp_on, cellp_off);
		if (ret != BCME_OK) {
			return ret;
		}
		ret = wlc_swdiv_antsel_select(clist, band, idx, FALSE, &rxantmap_new);
		if (ret != BCME_OK) {
			return ret;
		}
	}
	swdiv->policy.rx = rx_policy;
	swdiv->policy.cell = cell_policy;
	swdiv->userantmap = rxantmap_new;
	wlc_swdiv_scbcubby_stat_reset(swdiv);
	wlc_swdiv_antmap_shmem_update(swdiv, NULL, rxantmap_new);

	return ret;
}

void
wlc_swdiv_cellstatus_notif(wlc_swdiv_info_t *swdiv, int cellstatus)
{
	ASSERT(swdiv);
	WL_SWDIV(("%s: cellstatus=%d\n", __FUNCTION__, cellstatus));
	if (swdiv->cellstatus != cellstatus) {
		swdiv->cellstatus = cellstatus;
		wlc_swdiv_antmap_init(swdiv);
	}
}

uint16
wlc_swdiv_antmap_init(wlc_swdiv_info_t *swdiv)
{
	uint16 initantmap = 0;
	wlc_info_t *wlc;
	int band = 0;

	ASSERT(swdiv);
	if (swdiv) {
		wlc = swdiv->wlc;
		ASSERT(wlc);
		band = (BAND_2G(wlc->band->bandtype)) ? SWDIV_BANDSEL_2G : SWDIV_BANDSEL_5G;
		initantmap = wlc_swdiv_plcy_to_antmap_conv(swdiv, band);
		wlc_swdiv_scbcubby_stat_reset(swdiv);
		wlc_swdiv_antmap_shmem_update(swdiv, NULL, initantmap);
	}
	return initantmap;
}

void
wlc_swdiv_gpio_info_get(wlc_swdiv_info_t *swdiv, uint8 *offset, uint16 *enablebits)
{
	ASSERT(swdiv);
	if (swdiv) {
		if (offset) {
			*offset = swdiv->gpio_offset;
		}
		if (enablebits) {
			*enablebits = swdiv->gpio_ctrl;
		}
	}
}

wlc_swdiv_swctrl_t
wlc_swdiv_swctrl_en_get(wlc_swdiv_info_t *swdiv)
{
	uint8 swctrl_en = SWDIV_SWCTRL_0;	/* ACPHY default */
	if (swdiv) {
		swctrl_en = swdiv->swctrl_en;
	}
	return swctrl_en;
}

uint8
wlc_swdiv_plcybased_ant_get(wlc_swdiv_info_t *swdiv, int coreidx)
{
	wlc_info_t *wlc;
	swdiv_core_entity_t *clist;
	int band;
	uint8 ret_ant = 0;

	ASSERT(swdiv);
	wlc = swdiv->wlc;
	ASSERT(wlc);
	if (coreidx < 0 || coreidx >= MAX_PHYCORES_PER_SLICE) {
		WL_ERROR(("%s: invalid coreidx=%d\n", __FUNCTION__, coreidx));
		return ret_ant;
	}
	band = (BAND_2G(wlc->band->bandtype)) ? SWDIV_BANDSEL_2G : SWDIV_BANDSEL_5G;
	clist = wlc_swdiv_corelist_byidx_get(swdiv, coreidx);
	if (clist) {
		if (clist->rx_curplcy[band] == SWDIV_POLICY_AUTO) {
			ret_ant = SWDIV_ANT0_INUSE;
		} else {
			ret_ant = (clist->rx_curplcy[band] == SWDIV_POLICY_FORCE_0) ?
				SWDIV_ANT0_INUSE : SWDIV_ANT1_INUSE;
		}
	}
	return ret_ant;
}

bool
wlc_swdiv_supported(wlc_swdiv_info_t *swdiv, int coreidx, bool is2g, bool isokanyband)
{
	swdiv_core_entity_t *clist;
	bool ret = FALSE;
	ASSERT(swdiv);

	if (coreidx < 0 || coreidx >= MAX_PHYCORES_PER_SLICE) {
		WL_ERROR(("%s: invalid coreidx=%d\n", __FUNCTION__, coreidx));
		return 0;
	}
	clist = wlc_swdiv_corelist_byidx_get(swdiv, coreidx);
	if (clist) {
		if (isokanyband) {
			ret = (clist->support_2g || clist->support_5g) ? TRUE : FALSE;
		} else {
			ret = (is2g) ? clist->support_2g : clist->support_5g;
		}
	}
	return ret;
}

/* user request doesn't have specific scb info so need to clean it all */
void
wlc_swdiv_scbcubby_stat_reset(wlc_swdiv_info_t *swdiv)
{
	wlc_info_t *wlc;
	wlc_bsscfg_t *cfgiter;
	swdiv_scb_cubby_t *scbcb = NULL;
	struct scb_iter scbiter;
	struct scb *scb;
	int idx = 0, loop = 0;
	ASSERT(swdiv);
	wlc = swdiv->wlc;
	ASSERT(wlc);

	FOREACH_BSS(wlc, idx, cfgiter) {
		if (WLSWDIV_ENAB(wlc) && wlc_swdiv_bss_is_supported(cfgiter)) {
			FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfgiter, scb) {
				scbcb = SWDIV_SCB_CUBBY(swdiv, scb);
				if (scbcb) {
					for (loop = 0; loop < MAX_UCODE_COREBITS; loop++) {
						scbcb->polltime_elapsed[loop] = 0;
					}
					scbcb->linkmon->is_snrdropped = FALSE;
					if (scbcb->linkmon->ptimer) {
						wl_del_timer(wlc->wl, scbcb->linkmon->ptimer);
						scbcb->linkmon->is_timerset = FALSE;
					}
				}
			}
		}
	}
}

int32
wlc_swdiv_ant_plcy_override(wlc_swdiv_info_t *swdiv, uint core,
	uint8 rxovr_val, uint8 txovr_val, uint8 cellonovr_val, uint8 celloffovr_val)
{
	int ret = BCME_OK;
	int shift = 0;
	int coreidx = 0;
	uint32 ovr_rxplcy = 0, ovr_cellplcy = 0;
	swdiv_core_entity_t *clist;
	uint8 band;

	UNUSED_PARAMETER(txovr_val);
	ASSERT(swdiv);

	clist = swdiv->corelist;
	ovr_rxplcy = swdiv->policy.rx;
	ovr_cellplcy = swdiv->policy.cell;
	FOREACH_SWDIV_CORE(clist) {
		coreidx = wlc_swdiv_bitpos_get(clist->coreid);
		if (coreidx != core) {
			continue;
		}
		shift = coreidx * SWDIV_PLCY_BITS_PER_CORE;
		band = (clist->support_2g) ? SWDIV_BANDSEL_2G : SWDIV_BANDSEL_5G;
		SWDIV_PLCY_SET(ovr_rxplcy, band, shift, rxovr_val);
		SWDIV_CPLCY_SET(ovr_cellplcy, band, SWDIV_CELL_OFF, shift,
			celloffovr_val);
		SWDIV_CPLCY_SET(ovr_cellplcy, band, SWDIV_CELL_ON, shift,
			cellonovr_val);
	}
	ret = wlc_swdiv_policy_req(swdiv, ovr_rxplcy, SWDIV_POLICY_INVALID,
		SWDIV_POLICY_INVALID, SWDIV_POLICY_INVALID,
		ovr_cellplcy, SWDIV_POLICY_INVALID);
	return ret;
}

bool
wlc_swdiv_bss_is_supported(wlc_bsscfg_t *cfg)
{
	bool ret = FALSE;
	ASSERT(cfg);
	if (BSSCFG_INFRA_STA(cfg) && cfg->associated) {
		ret = TRUE;
	} else {
		/* disable others */
		ret = FALSE;
	}
	return ret;
}

/* --------------------------------------------
 * Internal API Function declaration
 * ---------------------------------------------
 */

/* initilize default params */
static void
BCMATTACHFN(wlc_swdiv_params_init)(wlc_swdiv_info_t * swdiv)
{
	wlc_pub_t *pub = swdiv->pub;

	/* all swdiv params are set with 0 at the module attach */
	swdiv->enable = (uint8)getintvar(pub->vars, rstr_swdiv_en);
	wlc_swdiv_gpio_nvram_get(swdiv);
	if (getintvar(pub->vars, rstr_swdiv_swctrl_en)) {
		swdiv->swctrl_en =
			(wlc_swdiv_swctrl_t)getintvar(pub->vars, rstr_swdiv_swctrl_en);
	} else {
		swdiv->swctrl_en = SWDIV_SWCTRL_0;	/* ACPHY default */
	}
	swdiv->swctrl_mask = (uint16)getintvar(pub->vars, rstr_swdiv_swctrl_mask);
	swdiv->swctrl_ant0 = (uint16)getintvar(pub->vars, rstr_swdiv_swctrl_ant0);
	swdiv->swctrl_ant1 = (uint16)getintvar(pub->vars, rstr_swdiv_swctrl_ant1);
	swdiv->log2_num_pkt_avg =
		(uint8)getintvar(pub->vars, rstr_swdiv_log2avg);
	if (!swdiv->log2_num_pkt_avg) {
		swdiv->log2_num_pkt_avg = SWDIV_LOG2_NUM_PKT_AVG;
	}
	swdiv->cck_snr_corr = (int16)getintvar(pub->vars, rstr_swdiv_ccksnrcorr);
	swdiv->antmap2g_main = (uint8)getintvar(pub->vars, rstr_swdiv_antmap2g_main);
	swdiv->antmap2g_aux = (uint8)getintvar(pub->vars, rstr_swdiv_antmap2g_aux);
	swdiv->antmap5g_main = (uint8)getintvar(pub->vars, rstr_swdiv_antmap5g_main);
	swdiv->antmap5g_aux = (uint8)getintvar(pub->vars, rstr_swdiv_antmap5g_aux);
	swdiv->deftune.snr_diff_thresh =
		(int16)getintvar(pub->vars, rstr_swdiv_snrthresh);
	if (!swdiv->deftune.snr_diff_thresh) {
		swdiv->deftune.snr_diff_thresh = SWDIV_SNR_DIFF_THRESHOLD_ACPHY;
	}
	swdiv->deftune.rxpktcnt_thresh =
		(uint32)getintvar(pub->vars, rstr_swdiv_thresh);
	if (!swdiv->deftune.rxpktcnt_thresh) {
		swdiv->deftune.rxpktcnt_thresh = SWDIV_RXPKTCNT_THRESHOLD_DEFAULT;
	}
	if ((1 << swdiv->log2_num_pkt_avg) > swdiv->deftune.rxpktcnt_thresh) {
		swdiv->deftune.rxpktcnt_thresh = 1 << swdiv->log2_num_pkt_avg;
	}
	swdiv->deftune.settle_rxpktcnt_limit =
		(uint32)getintvar(pub->vars, rstr_swdiv_settle);
	if (!swdiv->deftune.settle_rxpktcnt_limit) {
		swdiv->deftune.settle_rxpktcnt_limit = SWDIV_SETTLE_RX_COUNT_LIMIT;
	}
	swdiv->deftune.settle_rxpktcnt_thresh = (1 << swdiv->log2_num_pkt_avg)*2;
	swdiv->deftune.snr_healthy_limit =
		(int16)getintvar(pub->vars, rstr_swdiv_snrlim);
	if (!swdiv->deftune.snr_healthy_limit) {
		swdiv->deftune.snr_healthy_limit = SWDIV_SNR_LIMIT_ACPHY;
	}
	swdiv->deftune.snr_weight_firstant = (int8)getintvar(pub->vars, rstr_swdiv_weight);
	if (!swdiv->deftune.snr_weight_firstant) {
		swdiv->deftune.snr_weight_firstant = SWDIV_SNR_WEIGHT_1STANT;
	}
	swdiv->deftune.snrdrop_delta = (int16)getintvar(pub->vars, rstr_swdiv_snrdrop_delta);
	if (!swdiv->deftune.snrdrop_delta) {
		swdiv->deftune.snrdrop_delta = SWDIV_SNRDROP_THRESH_DEFAULT;
	}
	swdiv->deftune.snrdrop_period = (uint8)getintvar(pub->vars, rstr_swdiv_snrdrop_prd);
	if (!swdiv->deftune.snrdrop_period) {
		swdiv->deftune.snrdrop_period = SWDIV_SNRDROP_TICK_DUR_SEC;
	}
	swdiv->deftune.snrdrop_valid_limit = (int16)getintvar(pub->vars, rstr_swdiv_snrdrop_lmt);
	if (!swdiv->deftune.snrdrop_valid_limit) {
		swdiv->deftune.snrdrop_valid_limit = SWDIV_SNRDROP_MINSNR_VAL;
	}
	swdiv->deftune.snrdec_upon_txfail =
		(int16)getintvar(pub->vars, rstr_swdiv_snrdrop_txfail);
	if (!swdiv->deftune.snrdec_upon_txfail) {
		swdiv->deftune.snrdec_upon_txfail = SWDIV_SNRDROP_TXFAIL_DEFAULT;
	}
	swdiv->deftune.snr_poll_prd =
		(uint)getintvar(pub->vars, rstr_swdiv_poll_prd);
	if (!swdiv->deftune.snr_poll_prd) {
		swdiv->deftune.snr_poll_prd = (uint8)SWDIV_SNRDROP_POLL_PRD_DEFAULT;
	}
	swdiv->userantmap = SWDIV_NULL_ANTMAP;
	swdiv->policy.rx = 0x0;
	swdiv->policy.tx = 0x0;
	swdiv->policy.cell = 0x0;
	/* not used yet */
	swdiv->deftune.cfgmon_dur = 0;
}

/* iovar getter for sw diversity statistics */
static int32
wlc_swdiv_get_stats(wlc_swdiv_info_t *swdiv, wlc_swdiv_stats_t *swdiv_stats)
{
	wlc_info_t *wlc;
	wlc_bsscfg_t *cfgiter;
	swdiv_scb_cubby_t *scbcb = NULL;
	struct scb_iter scbiter;
	struct scb *scb;
	uint16 rxantmap_new;
	int idx = 0;
	int32 err = BCME_OK;
	ASSERT(swdiv);
	wlc = swdiv->wlc;

	FOREACH_BSS(wlc, idx, cfgiter) {
		if (WLSWDIV_ENAB(wlc) && wlc_swdiv_bss_is_supported(cfgiter)) {
			FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfgiter, scb) {
				scbcb = SWDIV_SCB_CUBBY(swdiv, scb);
				break;
			}
		}
	}
	if (scbcb) {
		bzero((char *)swdiv_stats, sizeof(wlc_swdiv_stats_t));
		wlc_swdiv_current_ant_get(wlc, &rxantmap_new);
		swdiv_stats->version = SWDIV_STATS_CURRENT_VERSION;
		swdiv_stats->length = sizeof(wlc_swdiv_stats_t);
		swdiv_stats->auto_en = (swdiv->policy.rx == SWDIV_POLICY_AUTO) ? 1 : 0;
		swdiv_stats->rx_policy = swdiv->policy.rx;
		swdiv_stats->cell_policy = swdiv->policy.cell;
		swdiv_stats->mws_antsel_ovr_tx = swdiv->mws_antsel_ovr_tx;
		swdiv_stats->mws_antsel_ovr_rx = swdiv->mws_antsel_ovr_rx;
		/* ignore tx_xxx params, and set them with '0' */

		swdiv_stats->active_ant = scbcb->stats->lastantmap & 0x1;
		swdiv_stats->acc_rxcount = scbcb->stats->rxpktcnt_accumulated;
		swdiv_stats->swap_trig_event_id =
			scbcb->stats->swaprsn_prev[swdiv_stats->active_ant];
		swdiv_stats->rxcount =
			scbcb->rxpktcnt_per_ant[swdiv_stats->active_ant];
		/* ant0 stats */
		swdiv_stats->avg_snr_per_ant0 =
			scbcb->snr_avg_per_ant[0] >> SWDIV_SNR_QFORMAT;
		swdiv_stats->swap_ge_rxcount0 =
			scbcb->stats->swap_ge_rxcnt[0];
		swdiv_stats->swap_ge_snrthresh0 =
			scbcb->stats->swap_ge_snrthresh[0];
		swdiv_stats->swap_txfail0 =
			scbcb->stats->swap_txfail[0];
		swdiv_stats->swap_timer0 =
			scbcb->stats->swapcnt_linkmon[0];
		swdiv_stats->swap_alivecheck0 =
			scbcb->stats->swap_alivechk[0];
		swdiv_stats->rxcount_per_ant0 =
			scbcb->rxpktcnt_per_ant[0];
		swdiv_stats->acc_rxcount_per_ant0 =
			scbcb->stats->rxpktcnt_total_per_ant[0];
		swdiv_stats->swap_snrdrop0 =
			scbcb->stats->swapcnt_snrdrop[0];
		/* ant1 stats */
		swdiv_stats->avg_snr_per_ant1 =
			scbcb->snr_avg_per_ant[1]  >> SWDIV_SNR_QFORMAT;
		swdiv_stats->swap_ge_rxcount1 =
			scbcb->stats->swap_ge_rxcnt[1];
		swdiv_stats->swap_ge_snrthresh1 =
			scbcb->stats->swap_ge_snrthresh[1];
		swdiv_stats->swap_txfail1 =
			scbcb->stats->swap_txfail[1];
		swdiv_stats->swap_timer1 =
			scbcb->stats->swapcnt_linkmon[1];
		swdiv_stats->swap_alivecheck1 =
			scbcb->stats->swap_alivechk[1];
		swdiv_stats->rxcount_per_ant1 =
			scbcb->rxpktcnt_per_ant[1];
		swdiv_stats->acc_rxcount_per_ant1 =
			scbcb->stats->rxpktcnt_total_per_ant[1];
		swdiv_stats->swap_snrdrop1 =
			scbcb->stats->swapcnt_snrdrop[1];
		/* ant2 stats */
		swdiv_stats->avg_snr_per_ant2 = 0;
	} else {
		err = BCME_NOTASSOCIATED;
	}
	return err;
}

static void
wlc_swdiv_gpio_shmem_init(wlc_hw_info_t *wlc_hw, uint32 mask)
{
	uint16 swdiv_shm_addr, gpio_mask_addr, gpio_num_addr;
	wlc_swdiv_info_t *swdiv;
	swdiv = wlc_hw->wlc->swdiv;

	swdiv_shm_addr = 2 * wlc_bmac_read_shm(wlc_hw, M_ACPHY_SWDIV_BLK_PTR);
	gpio_mask_addr = swdiv_shm_addr + M_ACPHY_SWDIV_GPIO_MASK;
	gpio_num_addr = swdiv_shm_addr + M_ACPHY_SWDIV_GPIO_NUM;

	wlc_bmac_write_shm(wlc_hw, gpio_num_addr, swdiv->gpio_offset);
	wlc_bmac_write_shm(wlc_hw, gpio_mask_addr, mask);
}

/* select dedicated switch ctrl lines
 *                disable	GPIO	RF_CTRL
 * M_SWDIV_EN		0		1		2
 *   swdiv_en		0		1		1
 * swdiv_swctrl_en	-		0		1or2(LCN40 or legacy chips)
 */
static void
wlc_swdiv_swctrl_type_set(wlc_hw_info_t *wlc_hw, uint type)
{
	uint16 swdiv_shm_addr, swdiv_en_addr;
	swdiv_shm_addr = 2 * wlc_bmac_read_shm(wlc_hw, M_ACPHY_SWDIV_BLK_PTR);
	swdiv_en_addr = swdiv_shm_addr + M_ACPHY_SWDIV_EN;

	wlc_bmac_write_shm(wlc_hw, swdiv_en_addr, type);
}

static void
wlc_swdiv_gpio_init(wlc_hw_info_t *wlc_hw, bool ucodectrl, uint16 defenable)
{
	uint32 gpiomask = 0x0;
	uint32 gpioctrl = 0x0;

	gpioctrl = wlc_swdiv_antmap_to_gpio_convert(wlc_hw->wlc, defenable, gpioctrl);
	if (ucodectrl) {
		/* enable gpio ctrl by other cores not by cc */
		si_gpiocontrol(wlc_hw->sih, gpioctrl, gpioctrl, GPIO_DRV_PRIORITY);
		/* set psm_gpio_oe for uCode gpio mask */
		gpiomask = wlc_swdiv_antmap_to_gpio_convert(wlc_hw->wlc, defenable, gpiomask);
		wlc_swdiv_gpio_shmem_init(wlc_hw, gpiomask);
	} else {
		/* general debugging purpose */
		gpiomask = si_gpioouten(wlc_hw->sih, 0, 0, GPIO_DRV_PRIORITY);
		gpiomask |= gpioctrl;
		/* disable any other core ctrl on gpio */
		si_gpiocontrol(wlc_hw->sih, gpiomask, gpioctrl, GPIO_DRV_PRIORITY);
		si_gpioouten(wlc_hw->sih, gpiomask, gpiomask, GPIO_DRV_PRIORITY);
	}
	WL_SWDIV(("%s:	[cc] gpioctrl=0x%x gpioouten=0x%x, \n", __FUNCTION__,
		si_gpiocontrol(wlc_hw->sih, 0, 0, GPIO_DRV_PRIORITY),
		si_gpioouten(wlc_hw->sih, 0, 0, GPIO_DRV_PRIORITY)));
	WL_SWDIV(("%s:	[psm] gpio_oe=0x%x \n", __FUNCTION__,
		R_REG(wlc_hw->osh, &wlc_hw->regs->psm_gpio_oe)));
}

static uint32
wlc_swdiv_antmap_to_gpio_convert(wlc_info_t *wlc, uint16 antmap, uint32 gpiomask)
{
	uint32 gpioout = gpiomask;
	int cnt = 0;

	FOREACH_SWDIV_ANTMAP(antmap, cnt) {
		if (wlc->swdiv->gpio_ctrl & (1 << cnt)) {
			if (SWDIV_ISBIT_SET_ON_MAP(antmap, cnt)) {
				gpioout |= 1 << (cnt + wlc->swdiv->gpio_offset);
			} else {
				gpioout &= ~(1 << (cnt + wlc->swdiv->gpio_offset));
			}
		}
	}
	return gpioout;
}

/* BSSCFG state update handling */
static void
wlc_swdiv_bsscfg_up_down(void *ctx, bsscfg_up_down_event_data_t *evt_data)
{
	wlc_swdiv_info_t *swdiv = (wlc_swdiv_info_t*)ctx;
	wlc_info_t *wlc = swdiv->wlc;
	swdiv_bsscfg_cubby_t *swdiv_bsscubby = SWDIV_BSSCFG_CUBBY(swdiv, evt_data->bsscfg);

	WL_SWDIV(("wl%d: %s(isup=%d)\n", wlc->pub->unit, __FUNCTION__, evt_data->up));

	if (!evt_data->up) {
		ASSERT(swdiv_bsscubby != NULL);
		/* cancel any link monitor timer */
		if (swdiv_bsscubby->cfgmon && swdiv_bsscubby->cfgmon->ptimer) {
			evt_data->callbacks_pending =
			   (wl_del_timer(wlc->wl, swdiv_bsscubby->cfgmon->ptimer) ? 0 : 1);
		}
	} else {	/* When bsscfg is up */
		swdiv_bsscubby->defantmap = wlc_swdiv_antmap_init(swdiv);
	}
}

static int
wlc_swdiv_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg)
{
	int err = 0;
	wlc_swdiv_info_t *swdiv = (wlc_swdiv_info_t *)ctx;
	wlc_info_t *wlc = swdiv->wlc;
	swdiv_bsscfg_cubby_t *pcubby;
	swdiv_bsscfg_cubby_t **pswdiv_cubby = SWDIV_BSSCFG_CUBBY_LOC(swdiv, cfg);

	WL_SWDIV(("wl%d: %s\n", wlc->pub->unit, __FUNCTION__));

	/* Init bsscfg cubby manually */
	if ((pcubby = MALLOCZ(wlc->osh, sizeof(swdiv_bsscfg_cubby_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		err = BCME_NOMEM;
		goto fail;
	}
	/* copy the whole setting from default set */
	memcpy((void*)&pcubby->tune, (void*)&swdiv->deftune,
		sizeof(swdiv_algo_tune_info_t));
	pcubby->auto_en = swdiv->enable;
	pcubby->defantmap = SWDIV_NULL_ANTMAP;
	pcubby->cfgmon = NULL;

	*pswdiv_cubby = pcubby;
	return BCME_OK;

fail:
	wlc_swdiv_bsscfg_deinit((void *)ctx, cfg);

	return err;
}

static void
wlc_swdiv_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_swdiv_info_t *swdiv = (wlc_swdiv_info_t *)ctx;
	wlc_info_t *wlc = swdiv->wlc;
	swdiv_bsscfg_cubby_t *cfgcb = SWDIV_BSSCFG_CUBBY(swdiv, cfg);

	WL_SWDIV(("wl%d: %s\n", wlc->pub->unit, __FUNCTION__));

	if (cfgcb) {
		if (cfgcb->cfgmon) {
			MFREE(wlc->osh, cfgcb->cfgmon, sizeof(swdiv_link_mon_t));
			cfgcb->cfgmon = NULL;
		}
		MFREE(wlc->osh, cfgcb, sizeof(swdiv_bsscfg_cubby_t));
		cfgcb = NULL;
	}
}

/* scb cubby functions */
static uint
wlc_swdiv_scb_secsz(void *ctx, struct scb *scb)
{
	wlc_swdiv_info_t *swdiv = (wlc_swdiv_info_t*)ctx;
	uint extra = 0;
	UNUSED_PARAMETER(swdiv);

	extra = sizeof(wlc_swdiv_status_t) + sizeof(swdiv_link_mon_t);
	return (uint)(sizeof(swdiv_scb_cubby_t) + extra);
}

static int
wlc_swdiv_scb_init(void *ctx, struct scb *scb)
{
	wlc_swdiv_info_t *swdiv = (wlc_swdiv_info_t *)ctx;
	wlc_info_t *wlc = swdiv->wlc;
	wlc_bsscfg_t *bsscfg = SCB_BSSCFG(scb);
	swdiv_bsscfg_cubby_t *cfgcb = SWDIV_BSSCFG_CUBBY(swdiv, bsscfg);
	swdiv_scb_cubby_t **pswdiv_scb = SWDIV_SCB_CUBBY_LOC(swdiv, scb);
	swdiv_scb_cubby_t *scbcb = NULL;
	uint secsz;
	int cnt = 0;

	WL_SWDIV(("wl%d: %s\n", wlc->pub->unit, __FUNCTION__));

	 /* alloc the cubby space and clear SCBs with init vals */
	 secsz = wlc_swdiv_scb_secsz(swdiv, scb);
	if ((scbcb = wlc_scb_sec_cubby_alloc(wlc, scb, secsz)) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return BCME_NOMEM;
	}
	/* assign statistics space */
	scbcb->stats = (wlc_swdiv_status_t*)&scbcb[1];
	/* assign link monitor space */
	scbcb->linkmon = (swdiv_link_mon_t*)&scbcb->stats[1];
	ASSERT(sizeof(*scbcb) + sizeof(*scbcb->stats) + sizeof(*scbcb->linkmon) == secsz);

	scbcb->stats->reqantmap = cfgcb->defantmap;
	scbcb->stats->lastantmap = SWDIV_NULL_ANTMAP;
	scbcb->linkmon->ptimer =
		wl_init_timer(wlc->wl, wlc_swdiv_timer, scb, "swdiv_link");
	/* calculate minimal wakeup interval to recv */
	scbcb->linkmon_dur = wlc_swdiv_linkmon_dur_get(wlc, bsscfg);
	for (cnt = 0; cnt < MAX_HW_ANTNUM; cnt++) {
		scbcb->noisest_per_ant[cnt] = SWDIV_NOISE_INIT;
	}
	scbcb->swdiv = swdiv;

	WL_SWDIV(("%s : scbcb=%p, stats=%p, linkmon=%p, ptimer=%p\n", __FUNCTION__,
		scbcb, scbcb->stats, scbcb->linkmon, scbcb->linkmon->ptimer));

	*pswdiv_scb = scbcb;

	return BCME_OK;
}

static void
wlc_swdiv_scb_deinit(void *ctx, struct scb *scb)
{
	wlc_swdiv_info_t *swdiv = (wlc_swdiv_info_t *)ctx;
	wlc_info_t *wlc = swdiv->wlc;
	swdiv_scb_cubby_t **pswdiv_scb = SWDIV_SCB_CUBBY_LOC(swdiv, scb);
	swdiv_scb_cubby_t *scbcb = *pswdiv_scb;

	WL_SWDIV(("wl%d: %s\n", 0, __FUNCTION__));

	wlc_scb_sec_cubby_free(wlc, scb, scbcb);
	*pswdiv_scb = NULL;
}

/* scb cubby update */
static int
wlc_swdiv_scb_update(void *ctx, struct scb *scb, wlc_bsscfg_t* new_cfg)
{
	wlc_swdiv_info_t *swdiv = (wlc_swdiv_info_t *)ctx;
	wlc_info_t *new_wlc = new_cfg->wlc;
	swdiv_scb_cubby_t *scbcb;
	scbcb = SWDIV_SCB_CUBBY(swdiv, scb);
	if (SWDIV_IS_ENABLED(swdiv) && wlc_swdiv_bss_is_supported(new_cfg)) {
		scbcb->swdiv = new_wlc->swdiv;
	}
	return BCME_OK;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void
wlc_swdiv_bsscfg_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	wlc_swdiv_info_t *swdiv = (wlc_swdiv_info_t *)ctx;
	swdiv_bsscfg_cubby_t * cfgcb = SWDIV_BSSCFG_CUBBY(swdiv, cfg);

	bcm_bprintf(b, "  snr_healthy_limit: %d\n", cfgcb->tune.snr_healthy_limit);
	bcm_bprintf(b, "  snr_diff_thresh: %d\n", cfgcb->tune.snr_diff_thresh);
	bcm_bprintf(b, "  snrdrop_valid_limit: %d\n", cfgcb->tune.snrdrop_valid_limit);
	bcm_bprintf(b, "  snrdec_upon_txfail: %d\n",
		cfgcb->tune.snrdec_upon_txfail);
	bcm_bprintf(b, "  snrdrop_delta: %d\n", cfgcb->tune.snrdrop_delta);
	bcm_bprintf(b, "  snrdrop_period: %d\n", cfgcb->tune.snrdrop_period);
	bcm_bprintf(b, "  snr_poll_prd: %d\n", cfgcb->tune.snr_poll_prd);
	bcm_bprintf(b, "  settle_rxpktcnt_thresh: %d\n", cfgcb->tune.settle_rxpktcnt_thresh);
	bcm_bprintf(b, "  rxpktcnt_thresh: %d\n", cfgcb->tune.rxpktcnt_thresh);
	bcm_bprintf(b, "  settlecnt_rxlimit %d\n", cfgcb->tune.settle_rxpktcnt_limit);
}

static void
wlc_swdiv_scb_dump(void *ctx, struct scb *scb, struct bcmstrbuf *b)
{
	wlc_swdiv_info_t *swdiv = (wlc_swdiv_info_t *)ctx;
	swdiv_scb_cubby_t *scbcb = SWDIV_SCB_CUBBY(swdiv, scb);
	char eabuf[ETHER_ADDR_STR_LEN];
	int idx = 0;
	swdiv_core_entity_t *clist = swdiv->corelist;

	wlc_swdiv_corelist_dump(clist, b);
	bcm_bprintf(b, "  SCB: %s BAND: %d\n",
	            bcm_ether_ntoa(&scb->ea, eabuf), scb->bandunit);
	bcm_bprintf(b, "   settlecnt_rx %d\n", scbcb->settlecnt_rx);
	bcm_bprintf(b, "   linkmon_dur %d\n", scbcb->linkmon_dur);
	bcm_bprintf(b, "   snrdrop_tick_wdog %d\n", scbcb->snrdrop_tick_wdog);
	bcm_bprintf(b, "   --per-ant--\n");
	bcm_bprintf(b, "   pollt_elspd: [");
	for (idx = 0; idx < 4; idx++) {
		bcm_bprintf(b, " %d", scbcb->polltime_elapsed[idx]);
	}
	bcm_bprintf(b, " ]\n   snr_avg: [");
	for (idx = 0; idx < 8; idx++) {
		bcm_bprintf(b, " %d", scbcb->snr_avg_per_ant[idx]);
	}
	bcm_bprintf(b, " ]\n   snr_sum: [");
	for (idx = 0; idx < 8; idx++) {
		bcm_bprintf(b, " %d", scbcb->snr_sum_per_ant[idx]);
	}
	bcm_bprintf(b, " ]\n  noiseest: [");
	for (idx = 0; idx < 8; idx++) {
		bcm_bprintf(b, " %d", scbcb->noisest_per_ant[idx]);
	}
	bcm_bprintf(b, " ]\n  rxpktcnt: [");
	for (idx = 0; idx < 8; idx++) {
		bcm_bprintf(b, " %d", scbcb->rxpktcnt_per_ant[idx]);
	}
	bcm_bprintf(b, " ]\n rxcnt_prv: [");
	for (idx = 0; idx < 8; idx++) {
		bcm_bprintf(b, " %d", scbcb->rxpktcnt_prev_per_ant[idx]);
	}

	bcm_bprintf(b, " ]\n   --stats struct--\n");
	bcm_bprintf(b, "   swaprsn_cur %d\n", scbcb->stats->swaprsn_cur);
	bcm_bprintf(b, "   rxpktcnt_accumulated %d\n", scbcb->stats->rxpktcnt_accumulated);
	bcm_bprintf(b, "   lastcoremap 0x%x\n", scbcb->stats->lastcoremap);
	bcm_bprintf(b, "   lastantmap 0x%x\n", scbcb->stats->lastantmap);
	bcm_bprintf(b, "   lastchanspec 0x%x\n", scbcb->stats->lastchanspec);
	bcm_bprintf(b, "   --stats: per-ant--\n");
	bcm_bprintf(b, "   swaprsn_prev: [");
	for (idx = 0; idx < 8; idx++) {
		bcm_bprintf(b, " %d", scbcb->stats->swaprsn_prev[idx]);
	}
	bcm_bprintf(b, " ]\n   snr_prev_avg: [");
	for (idx = 0; idx < 8; idx++) {
		bcm_bprintf(b, " %d", scbcb->stats->snr_prev_avg_per_ant[idx]);
	}
	bcm_bprintf(b, " ]\n rxpktcnt_total: [");
	for (idx = 0; idx < 8; idx++) {
		bcm_bprintf(b, " %d", scbcb->stats->rxpktcnt_total_per_ant[idx]);
	}
	bcm_bprintf(b, " ]\n swapcnt_snrdrp: [");
	for (idx = 0; idx < 8; idx++) {
		bcm_bprintf(b, " %d", scbcb->stats->swapcnt_snrdrop[idx]);
	}
	bcm_bprintf(b, " ]\n  swapcnt_linkm: [");
	for (idx = 0; idx < 8; idx++) {
		bcm_bprintf(b, " %d", scbcb->stats->swapcnt_linkmon[idx]);
	}
	bcm_bprintf(b, " ]\n    swap_txfail: [");
	for (idx = 0; idx < 8; idx++) {
		bcm_bprintf(b, " %d", scbcb->stats->swap_txfail[idx]);
	}
	bcm_bprintf(b, " ]\n  swap_alivechk: [");
	for (idx = 0; idx < 8; idx++) {
		bcm_bprintf(b, " %d", scbcb->stats->swap_alivechk[idx]);
	}
	bcm_bprintf(b, " ]\n ----END of SWDIV SCB----\n");
}
#endif /* BCMDBG || BCMDBG_DUMP */

static void
wlc_swdiv_watchdog(void *hdl)
{
	wlc_swdiv_info_t *swdiv = (wlc_swdiv_info_t *)hdl;
	wlc_info_t *wlc = swdiv->wlc;
	swdiv_core_entity_t * clist;
	wlc_bsscfg_t *cfgiter;
	swdiv_bsscfg_cubby_t *cfgcb;
	swdiv_scb_cubby_t *scbcb;
	struct scb_iter scbiter;
	struct scb *scb;

	uint8 band;
	uint16 rxantmap_new;
	int idx = 0;
	int sidx = 0;

	/* top control flag check */
	if (!SWDIV_IS_ENABLED(swdiv)) {
		WL_INFORM(("%s: enable flag set FALSE\n", __FUNCTION__));
		return;
	}
	clist = swdiv->corelist;
	wlc_swdiv_current_ant_get(wlc, &rxantmap_new);

	/* multi-scbs cases need to calculate their own status */
	/* check scb rxpktcnt for each one */
	/* TODO: per-band scb preference introduced. which scb on the same band
	 * will be selected to decide swdiv criteria. need a new IOVAR.
	 * e.g., AWDL + STA on the same ch case is problematic
	 */
	FOREACH_BSS(wlc, idx, cfgiter) {
		WL_INFORM(("%s rxantmap=0x%x / current_bss(0x%x)\n",
			__FUNCTION__, rxantmap_new, cfgiter->current_bss->chanspec));
		if (WLSWDIV_ENAB(wlc) && wlc_swdiv_bss_is_supported(cfgiter) &&
			wlc_swdiv_band_valid_chk(cfgiter)) {
			cfgcb = SWDIV_BSSCFG_CUBBY(swdiv, cfgiter);
			band =
				(CHSPEC_IS2G(cfgiter->current_bss->chanspec)) ?
					SWDIV_BANDSEL_2G : SWDIV_BANDSEL_5G;
			FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfgiter, scb) {
				scbcb = SWDIV_SCB_CUBBY(swdiv, scb);
				if (!scbcb) {
					continue;
				}
				scbcb->snrdrop_tick_wdog++;

				FOREACH_SWDIV_CORE(clist) {
					idx = wlc_swdiv_bitpos_get(clist->coreid);
					if (idx	< 0) {
						break;
					}
					/* syncup first with the current uCode update */
					clist->rxsel.cur = SWDIV_GETBIT_BY_MAP(rxantmap_new, idx);
					scbcb->stats->lastantmap = rxantmap_new;
					sidx = SWDIV_GETSIDX(clist->rxsel.cur, idx);
					WL_SWDIV(("%s: scb=0x%p, rxsel.cur=%d, sidx=%d\n",
						__FUNCTION__, scb, clist->rxsel.cur, sidx));
					if (wlc_swdiv_rxpktbased_swap_check(scbcb, sidx)) {
						scbcb->stats->swaprsn_cur = SWDIV_EVENT_WATCHDOG;
						scbcb->stats->swap_alivechk[sidx]++;
						wlc_swdiv_antsel_select(clist, band, idx, TRUE,
							&rxantmap_new);
					} else if ((clist->rx_curplcy[band] == SWDIV_POLICY_AUTO) &&
						(scbcb->snrdrop_tick_wdog >=
							cfgcb->tune.snrdrop_period)) {
						if (wlc_swdiv_snrbased_swap_check(
								cfgcb, scbcb, sidx)) {
							scbcb->linkmon->is_snrdropped = TRUE;
							/* later on, let do rxpkt based chk */
						}
					}
					scbcb->rxpktcnt_prev_per_ant[sidx] =
						scbcb->rxpktcnt_per_ant[sidx];
					if (clist->active) {
						scbcb->polltime_elapsed[idx]++;
					}
					/* reset polltime if ant is changed */
					if (SWDIV_GETBIT_BY_MAP(rxantmap_new, idx)
						!= clist->rxsel.cur) {
						scbcb->polltime_elapsed[idx] = 0;
					}
				} /* End of ACTBAND_CORE loop */
				if (rxantmap_new != scbcb->stats->lastantmap) {
					scbcb->linkmon->is_snrdropped = FALSE;
					wlc_swdiv_antmap_shmem_update(swdiv, scbcb, rxantmap_new);
				}
			} /* END of SCB loop */
		}
	}
	return;
}

static bool
wlc_swdiv_band_valid_chk(wlc_bsscfg_t *cfg)
{
	bool ret = FALSE;
	wlc_info_t *wlc = cfg->wlc;
	UNUSED_PARAMETER(wlc);

	if (BSSCFG_INFRA_STA(cfg)) {
		if (wf_chspec_ctlchan(WLC_BAND_PI_RADIO_CHANSPEC) ==
			wf_chspec_ctlchan(cfg->current_bss->chanspec)) {
			ret = TRUE;
		}
	} else {
		/* put other bsscfg types valid condition */
		ret = FALSE;
	}
	return ret;
}

static void
wlc_swdiv_assoc_state_change_cb(void *ctx, bss_assoc_state_data_t *notif_data)
{
	wlc_swdiv_info_t *swdiv = (wlc_swdiv_info_t*)ctx;
	wlc_info_t *wlc = swdiv->wlc;
	wlc_bsscfg_t *cfg = notif_data->cfg;
	struct scb *scb;

	if (!SWDIV_IS_ENABLED(swdiv)) {
		return;
	}
	if (WLSWDIV_BSSCFG_SUPPORT(cfg) &&
		notif_data->type == AS_ASSOCIATION && notif_data->state == AS_IDLE) {
		scb = wlc_scbfind(wlc, cfg, &cfg->BSSID);
		wlc_swdiv_reset_stats(wlc->swdiv, scb);
	}
}

static void
wlc_swdiv_gpio_nvram_get(wlc_swdiv_info_t *swdiv)
{
	wlc_info_t *wlc;
	uint8 gpio_offset = 0xFF;
	uint16 gpio_ctrl = 0xFFFF;
	if (swdiv == NULL) {
		WL_ERROR(("%s err: swdiv is null.\n", __FUNCTION__));
		return;
	}
	wlc = swdiv->wlc;
	if (getintvar(wlc->pub->vars, rstr_swdiv_gpio)) {
		gpio_offset = (uint8)getintvar(wlc->pub->vars, rstr_swdiv_gpio);
	}
	if (getintvar(wlc->pub->vars, rstr_swdiv_gpioctrl)) {
		gpio_ctrl = (uint16)getintvar(wlc->pub->vars, rstr_swdiv_gpioctrl);
	}
	gpio_offset = (gpio_offset == 0xFF) ?
		WLC_SWDIV_GPIO_OFFSET : gpio_offset;
	gpio_ctrl = (gpio_ctrl == 0xFFFF) ?
		WLC_SWDIV_GPIO_CTRLBITS : gpio_ctrl;
	swdiv->gpio_offset = gpio_offset;
	swdiv->gpio_ctrl = gpio_ctrl;
}

static uint16
wlc_swdiv_core_mapping(
	wlc_swdiv_info_t *swdiv, swdiv_bsscfg_cubby_t *cfgcb,
	swdiv_scb_cubby_t *scbcb, uint16 coremap, uint16 antmap, uint8 band)
{
	swdiv_core_entity_t *clist;
	int idx = 0;
	int sidx = 0;
	bool swap = TRUE, start_timer = FALSE;
	uint16 antmap_updated = antmap;

	clist = swdiv->corelist;

	if (coremap == 0) {
		/* TODO: do we have to get from other place like stf info? */
		/* or just regarded this as all rxchains are off..? */
		return antmap_updated;
	}
	/* swap decision logic */
	FOREACH_SWDIV_ACTBAND_CORE(clist, band) {
		idx = wlc_swdiv_bitpos_get(clist->coreid);
		if (idx < 0) {
			return antmap;
		}
		if (clist->rx_curplcy[band] == SWDIV_POLICY_AUTO) {
			sidx = SWDIV_GETSIDX(clist->rxsel.cur, idx);
			swap = wlc_swdiv_diversity_engine(cfgcb, scbcb,
				idx, sidx, clist->rxsel.cur);
			if (swap || (scbcb->rxpktcnt_per_ant[sidx] <
				(1 << scbcb->swdiv->log2_num_pkt_avg))) {
				start_timer = TRUE;
			}
			if (swap) {
				clist->rxsel.requested =
					(clist->rxsel.cur == SWDIV_ANT0_INUSE) ?
						SWDIV_ANT1_INUSE : SWDIV_ANT0_INUSE;
				SWDIV_SETBIT_ON_MAP(antmap_updated, idx, clist->rxsel.requested);
				WL_SWDIV(("%s: rxsel.req=%d, rxsel.cur=%d\n", __FUNCTION__,
					clist->rxsel.requested, clist->rxsel.cur));
			} else {
				continue;
			}
		}
	}
	/* assume that the running timer is already killed
	 * at the rxpkt_recv enterance
	 */
	if (start_timer) {
		scbcb->linkmon->is_timerset = TRUE;
		wl_add_timer(swdiv->wlc->wl, scbcb->linkmon->ptimer,
			scbcb->linkmon_dur, FALSE);
	}
	return antmap_updated;
}

/* 'idx' indicates each core index coming from corelist */
/* 'sidx' indicates array index of per-ant stats */
/* antidx indicates the selection between ANT0 and ANT1 */
static bool
wlc_swdiv_diversity_engine(swdiv_bsscfg_cubby_t *cfgcb,
	swdiv_scb_cubby_t * scbcb, int idx, int sidx, int antidx)
{
	bool swap_ant = FALSE;
	uint32 num_pkt_avg = 0;
	uint8 swap_event = SWDIV_EVENT_IDLE;
	int32 snr_diff = 12345;

	uint32 rxpkt_thresh;
	int32 snrdiff_thresh;

	/* pick creteria */
	num_pkt_avg = 1 << scbcb->swdiv->log2_num_pkt_avg;
	rxpkt_thresh = cfgcb->tune.rxpktcnt_thresh;
	snrdiff_thresh = cfgcb->tune.snr_diff_thresh;

	if (scbcb->settlecnt_rx <= cfgcb->tune.settle_rxpktcnt_limit) {
		rxpkt_thresh = cfgcb->tune.settle_rxpktcnt_thresh;		//settle_threshold
		snrdiff_thresh = SWDIV_SNR_DIFF_THRESHOLD_DURING_SETTLING;	//snr_thresh_dB
	}
	/* priotized checking */
	if (scbcb->snr_avg_per_ant[sidx] > cfgcb->tune.snr_healthy_limit) {
		swap_ant = FALSE;
	} else if (scbcb->linkmon->is_snrdropped) {
		/* 1nd priority : SNR DROP */
		swap_ant = TRUE;
		swap_event = SWDIV_EVENT_SNRDROP;
		scbcb->stats->swapcnt_snrdrop[sidx]++;
		scbcb->stats->swap_ge_rxcnt[sidx]++;
		scbcb->polltime_elapsed[idx] = 0;
		scbcb->linkmon->is_snrdropped = FALSE;
	} else if ((scbcb->rxpktcnt_per_ant[sidx] > rxpkt_thresh) ||
		(scbcb->polltime_elapsed[idx] > cfgcb->tune.snr_poll_prd)) {
		/* 2rd priority : RXPKT COUNT or polltime by watchdog */
		swap_ant = TRUE;
		swap_event = SWDIV_EVENT_RXCOUNT;
	} else {
		/* 3rd proirity : SNR THRSH */
		if (antidx) {
			/* ant1 is selected, ant0 is pos-1 */
			snr_diff =
				((scbcb->snr_avg_per_ant[sidx - 1])
					+ cfgcb->tune.snr_weight_firstant)
						- scbcb->snr_avg_per_ant[sidx];
		} else {
			/* ant0 is selected, ant1 is pos+1 */
			snr_diff =
				scbcb->snr_avg_per_ant[sidx + 1]
				- ((scbcb->snr_avg_per_ant[sidx])
					+ cfgcb->tune.snr_weight_firstant);
		}
		if ((snr_diff >= snrdiff_thresh) &&
			(scbcb->rxpktcnt_per_ant[sidx] >= num_pkt_avg)) {
				swap_ant = TRUE;
				swap_event = SWDIV_EVENT_SNRTHRSH;
				scbcb->stats->swap_ge_snrthresh[idx]++;
		}
	}
	/* stat update */
	scbcb->stats->swaprsn_prev[sidx] = swap_event;
	scbcb->stats->swaprsn_cur = swap_event;
	if (swap_ant || scbcb->rxpktcnt_per_ant[sidx] < num_pkt_avg) {
		scbcb->stats->snr_prev_avg_per_ant[sidx] = scbcb->snr_avg_per_ant[sidx];
	}
#ifdef BCMDBG
	if (swap_ant) {
		WL_SWDIV(("%s: swap rsn=%s,avgsnr=%d,snrdrop=%d,rxpkt(%d|%d),snrdiff(%d|%d),"
			"pollt(%d|%d)\n",
			__FUNCTION__, evttbl[swap_event].str, scbcb->snr_avg_per_ant[sidx],
			scbcb->linkmon->is_snrdropped,
			scbcb->rxpktcnt_per_ant[sidx], rxpkt_thresh,
			snr_diff, snrdiff_thresh, scbcb->polltime_elapsed[idx],
			cfgcb->tune.snr_poll_prd));
	} else {
		WL_INFORM(("%s: noswap rsn=%s,avgsnr=%d,snrdrop=%d,rxpkt(%d|%d),"
			"snrdiff(%d|%d), pollt(%d|%d)\n",
			__FUNCTION__, evttbl[swap_event].str, scbcb->snr_avg_per_ant[sidx],
			scbcb->linkmon->is_snrdropped,
			scbcb->rxpktcnt_per_ant[sidx], rxpkt_thresh,
			snr_diff, snrdiff_thresh, scbcb->polltime_elapsed[idx],
			cfgcb->tune.snr_poll_prd));
	}
#endif /* BCMDBG */
	return swap_ant;
}

static bool
wlc_swdiv_rxpktbased_swap_check(swdiv_scb_cubby_t *scbcb, int sidx)
{
	bool do_swap = FALSE;
	if (scbcb->rxpktcnt_per_ant[sidx] == scbcb->rxpktcnt_prev_per_ant[sidx]) {
		scbcb->stats->swaprsn_prev[sidx] = scbcb->stats->swaprsn_cur;
		do_swap = TRUE;
		WL_SWDIV(("%s: TRUE\n", __FUNCTION__));
	}
	return do_swap;
}

static bool
wlc_swdiv_snrbased_swap_check(swdiv_bsscfg_cubby_t *cfgcb, swdiv_scb_cubby_t *scbcb, int sidx)
{
	bool do_swap = FALSE;
	scbcb->snrdrop_tick_wdog = 0;

	/* Two conditions to switch for SNR drop :
	 * 1. SNR drop is greater than the delta threshold
	 * 2. SNR is greater than SNR drop limit
	 */
	if (((scbcb->stats->snr_prev_avg_per_ant[sidx] - scbcb->snr_avg_per_ant[sidx])
		> cfgcb->tune.snrdrop_delta) &&
		(scbcb->stats->snr_prev_avg_per_ant[sidx]
		> cfgcb->tune.snrdrop_valid_limit)) {
		WL_SWDIV(("%s: TRUE, algo will decide. snrdiff=%d, sntprev_avg=%d\n", __FUNCTION__,
			(scbcb->stats->snr_prev_avg_per_ant[sidx] - scbcb->snr_avg_per_ant[sidx]),
			scbcb->stats->snr_prev_avg_per_ant[sidx]));
		do_swap = TRUE;
	}

	scbcb->stats->snr_prev_avg_per_ant[sidx] = scbcb->snr_avg_per_ant[sidx];
	return do_swap;
}

static void
wlc_swdiv_rxbcn_recv_cb(void *ctx, bss_rx_bcn_notif_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t*)ctx;
	bss_rx_bcn_notif_data_t *bcn = data;
	wlc_bsscfg_t *cfg = bcn->cfg;
	ratespec_t rspec;
	struct scb *scb = wlc_scbfind(wlc, cfg, &cfg->BSSID);

	rspec = wlc_recv_compute_rspec(wlc->d11_info, bcn->wrxh, bcn->plcp);
	if (scb && WLSWDIV_BSSCFG_SUPPORT(cfg)) {
		wlc_swdiv_rxpkt_recv(wlc->swdiv, scb, bcn->wrxh, RSPEC_ISCCK(rspec));
	}
}

static uint32
wlc_swdiv_coremap_select(wlc_swdiv_info_t *swdiv, uint8 reqtype)
{
	wlc_info_t* wlc;
	uint32 coremap = 0;
	uint8 map2g = 0;
	uint8 map5g = 0;

	ASSERT(swdiv != NULL);

	wlc = swdiv->wlc;
	/* multiple calling from each wlc instance case is also fine here
	 * corelist will be genereated by its own d11MAC info
	 */
	if (wlc->cmn->wlc[0] == wlc) {
		map2g = swdiv->antmap2g_main;
		map5g = swdiv->antmap5g_main;
	} else {
		map2g = swdiv->antmap2g_aux;
		map5g = swdiv->antmap5g_aux;
	}

	if (reqtype == SWDIV_BANDSEL_BOTH) {
		coremap = map2g | map5g;
	} else if (reqtype == SWDIV_BANDSEL_2G) {
		coremap = map2g;
	} else {
		coremap = map5g;
	}
	return coremap;
}

/* if static core configuration is OK, don't need to generalize with list control funcs */
static int
BCMATTACHFN(wlc_swdiv_corelist_init)(wlc_swdiv_info_t *swdiv)
{
	int corecnts = 0;
	uint8 bitstamp = 0;
	uint32 coremap = 0;
	uint32 bandmap2g = 0;
	uint32 bandmap5g = 0;
	int err = 0;
	swdiv_core_entity_t *corealloc, *headlist;
	wlc_info_t *wlc;

	ASSERT(swdiv != NULL);

	WL_SWDIV(("wl%d: %s\n", 0, __FUNCTION__));

	wlc = swdiv->wlc;
	corealloc = headlist = NULL;
	/* coremap picked from current d11MAC wlc */
	coremap = wlc_swdiv_coremap_select(swdiv, SWDIV_BANDSEL_BOTH);
	bandmap2g = wlc_swdiv_coremap_select(swdiv, SWDIV_BANDSEL_2G);
	bandmap5g = wlc_swdiv_coremap_select(swdiv, SWDIV_BANDSEL_5G);

	if (coremap) {
		FOREACH_SWDIV_COREMAP(coremap, corecnts) {
			if (SWDIV_GETBIT_BY_MAP(coremap, corecnts)) {
				/* allocate a core entity space */
				corealloc = MALLOCZ(wlc->osh, sizeof(swdiv_core_entity_t));
				if (corealloc == NULL) {
					WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
						wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
					err = BCME_NOMEM;
					return err;
				}
				/* Fill in the core entity contents */
				bitstamp = (1 << corecnts);
				corealloc->coreid = bitstamp;
				corealloc->support_2g =
					(bandmap2g & bitstamp) ? TRUE : FALSE;
				corealloc->support_5g =
					(bandmap5g & bitstamp) ? TRUE : FALSE;
				corealloc->active = FALSE;
				/* Propagate any specific shared info from swdiv_info struct */
				corealloc->rxsel.cur = SWDIV_ANT0_INUSE;
				corealloc->rxsel.requested = corealloc->rxsel.cur;

				/* append on the list */
				wlc_swdiv_corelist_append(&headlist, corealloc);
			}
		}
		swdiv->corelist = headlist;
	} else {
		swdiv->corelist = NULL;
		err = BCME_NOTFOUND;
		WL_ERROR(("wl%d: %s: corelist=0\n", 0, __FUNCTION__));
	}
	return err;
}

static void
wlc_swdiv_corelist_deinit(wlc_swdiv_info_t *swdiv)
{
	swdiv_core_entity_t *pnext;
	swdiv_core_entity_t *pitr;
	wlc_info_t *wlc = swdiv->wlc;
	pitr = swdiv->corelist;

	while (pitr != NULL) {
		pnext = pitr->next;
		MFREE(wlc->osh, pitr, sizeof(swdiv_core_entity_t));
		pitr = pnext;
	}
	swdiv->corelist = NULL;
}

static void
wlc_swdiv_corelist_append(swdiv_core_entity_t **pphead, swdiv_core_entity_t *pinput)
{
	swdiv_core_entity_t *clist;
	clist = *pphead;
	pinput->next = NULL;

	WL_INFORM(("wl%d: %s, alloc ptr=0x%p\n", 0, __FUNCTION__, pinput));
	if (clist) {
		while (clist->next) {
			clist = clist->next;
		}
		clist->next = pinput;
	} else { /* first allocation case */
		*pphead = pinput;
	}
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void
wlc_swdiv_corelist_dump(swdiv_core_entity_t *clist, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "   ---CORELIST---\n");
	FOREACH_SWDIV_CORE(clist) {
		bcm_bprintf(b, "   [COREID %p 0x%x]\n", clist, clist->coreid);
		bcm_bprintf(b, "     active %d\n", clist->active);
		bcm_bprintf(b, "     support_2g %d\n", clist->support_2g);
		bcm_bprintf(b, "     support_5g %d\n", clist->support_5g);
		bcm_bprintf(b, "     rx_curplcy (%d,%d)\n",
			clist->rx_curplcy[0], clist->rx_curplcy[1]);
		bcm_bprintf(b, "     rxsel: cur=0x%x, reqested=0x%x\n",
			clist->rxsel.cur, clist->rxsel.requested);
		bcm_bprintf(b, "     policyRX: %d, %d \n",
			clist->rxplcy[0], clist->rxplcy[1]);
		bcm_bprintf(b, "     policy_cellon: %d, %d \n",
			clist->cellplcy_on[0], clist->cellplcy_on[1]);
		bcm_bprintf(b, "     policy_celloff: %d, %d \n",
			clist->cellplcy_off[0], clist->cellplcy_off[1]);
	}
	bcm_bprintf(b, "   ---END of CORELIST---\n");
}
#endif /* BCMDBG || BCMDBG_DUMP */

/* uCode antmap update is asyncronous. have to be updated with the best effort */
static void
wlc_swdiv_corelist_antmap_sync(wlc_swdiv_info_t *swdiv, uint16 ucode_antmap)
{
	int idx = 0;
	swdiv_core_entity_t *clist;
	clist = swdiv->corelist;

	if (clist != NULL) {
		FOREACH_SWDIV_CORE(clist) {
			idx = wlc_swdiv_bitpos_get(clist->coreid);
			if (idx < 0) {
				break;
			}
			clist->rxsel.cur = SWDIV_ISBIT_SET_ON_MAP(ucode_antmap, idx);
			clist->rxsel.requested = SWDIV_ANT_INVALID;
		}
	}
}

/* PARAM: activemap - each bit position indicates each phycore sequencially */
static void
wlc_swdiv_corelist_coremap_sync(wlc_swdiv_info_t *swdiv, uint16 activemap)
{
	int idx = 0;
	swdiv_core_entity_t *clist;
	clist = swdiv->corelist;

	if (clist != NULL) {
		FOREACH_SWDIV_CORE(clist) {
			idx = wlc_swdiv_bitpos_get(clist->coreid);
			if (idx < 0) {
				break;
			}
			clist->active =
				(SWDIV_ISBIT_SET_ON_MAP(activemap, idx)) ? TRUE : FALSE;
		}
	}
}

static void
wlc_swdiv_current_ant_get(wlc_info_t *wlc, uint16 *rxantmap)
{
	uint16 swdiv_shm_addr;
	uint16 rx_pref_ant_addr;
	wlc_hw_info_t *wlc_hw = wlc->hw;

	swdiv_shm_addr = 2 * wlc_bmac_read_shm(wlc_hw, M_ACPHY_SWDIV_BLK_PTR);
	rx_pref_ant_addr = swdiv_shm_addr + M_ACPHY_SWDIV_PREF_ANT;
	*rxantmap = wlc_bmac_read_shm(wlc_hw, rx_pref_ant_addr);
	WL_SWDIV(("%s pref_ant_addr,val=0x%x, retval=0x%x,rxantmap=0x%x\n",
		__FUNCTION__, rx_pref_ant_addr, *rxantmap,
		*rxantmap & SWDIV_PREF_ANT_SHMEM_MASK));
	*rxantmap &= SWDIV_PREF_ANT_SHMEM_MASK;
}

/* update phycore activation and band info */
static void
wlc_swdiv_antmap_shmem_update(wlc_swdiv_info_t * swdiv,
	swdiv_scb_cubby_t * scbcb, uint16 rxantmap)
{
	uint16 swdiv_shm_addr, pref_ant_addr;
	uint16 curantmap = 0;
	wlc_info_t *wlc = swdiv->wlc;
	wlc_hw_info_t *wlc_hw = wlc->hw;

	if (!wlc->clk) {
		WL_ERROR(("wl%d: %s: No CLK\n", wlc->pub->unit, __FUNCTION__));
		return;
	}
	if (scbcb) {
		if ((rxantmap != SWDIV_NULL_ANTMAP) &&
			(rxantmap == scbcb->stats->lastantmap)) {
			return;
		}
	}
	/* notify the selection info to uCode shmem */
	if (rxantmap != SWDIV_NULL_ANTMAP) {
		/* pref_ant value in detail
		 *  uint32 object: [31:8] 16bits TXPWRCAP + [7:0] ant selection info per core
		 *  [7:0] bits : 0x[phycore7][phycore8]...[phycore0]
		 * each bit position indicates each phychain ant selection status:
		 * bit'0' mean 1st ant, bit '1' means 2nd ant
		 */
#ifdef WLC_TXPWRCAP
		if (WLTXPWRCAP_ENAB(wlc)) {
			wlc_phy_txpwrcap_to_shm(wlc->hw->band->pi, rxantmap, rxantmap);
		} else
#endif /* WLC_TXPWRCAP */
		{
			swdiv_shm_addr =
				2 * wlc_bmac_read_shm(wlc_hw, M_ACPHY_SWDIV_BLK_PTR);
			pref_ant_addr = swdiv_shm_addr + M_ACPHY_SWDIV_PREF_ANT;
			curantmap = wlc_bmac_read_shm(wlc_hw, pref_ant_addr);
			curantmap = (curantmap & ~SWDIV_PREF_ANT_SHMEM_MASK) |
#ifdef WLC_SWDIV_MULTI_CORE_ENABLE
				(rxantmap & SWDIV_PREF_ANT_SHMEM_MASK);
#else
				(rxantmap & 0x1);
#endif /* WLC_SWDIV_MULTI_CORE_ENABLE */
			wlc_bmac_write_shm(wlc_hw, pref_ant_addr, curantmap);
		}
		WL_SWDIV(("%s rxantmap=0x%x\n", __FUNCTION__, rxantmap));
	}
}

static void
wlc_swdiv_timer(void *arg)
{
	struct scb *scb = (struct scb*)arg;
	wlc_info_t *wlc;
	wlc_swdiv_info_t *swdiv;
	wlc_bsscfg_t *bsscfg;
	swdiv_scb_cubby_t * scbcb;
	swdiv_bsscfg_cubby_t *cfgcb;
	uint16 rx_map;
	int sidx = 0;
	int idx = 0;
	bool update_snr = TRUE;
	uint8 band;
	swdiv_core_entity_t *clist;

	ASSERT(scb);
	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg);
	wlc = bsscfg->wlc;
	ASSERT(wlc);
	swdiv = wlc->swdiv;
	ASSERT(swdiv);
	cfgcb = SWDIV_BSSCFG_CUBBY(swdiv, bsscfg);
	scbcb = SWDIV_SCB_CUBBY(swdiv, scb);
	clist = swdiv->corelist;

	if (!wlc->clk) {
		return;
	}
	/* top control flag check */
	if (!SWDIV_IS_ENABLED(swdiv)) {
		WL_INFORM(("%s: enable flag set FALSE\n", __FUNCTION__));
		return;
	}
	/* TODO: if the current channel is moved. do we need to return? */
	wlc_swdiv_current_ant_get(wlc, &rx_map);
	band = (CHSPEC_IS2G(bsscfg->current_bss->chanspec)) ?
		SWDIV_BANDSEL_2G : SWDIV_BANDSEL_5G;
	WL_SWDIV(("%s: rx_map=0x%x ?= lastantmap=0x%x, chanspec=0x%x\n", __FUNCTION__,
		rx_map, scbcb->stats->lastantmap, bsscfg->current_bss->chanspec));

	/* Update necessary stats */
	for (idx = 0; idx < MAX_HW_ANTNUM; idx++) {
		scbcb->rxpktcnt_per_ant[idx] = 0;
		scbcb->rxpktcnt_prev_per_ant[idx] = 0;
	}
	scbcb->stats->swaprsn_cur = SWDIV_EVENT_TIMER;

	/* Update SNR only in case of data traffic */
	/* TODO: if uCode ant is changed, do we need to update? */
	if (SCAN_IN_PROGRESS(wlc->scan) ||
		(scbcb->settlecnt_rx <= cfgcb->tune.settle_rxpktcnt_limit)) {
		update_snr = FALSE;
	}

	FOREACH_SWDIV_ACTCORE(clist) {
		idx = wlc_swdiv_bitpos_get(clist->coreid);
		if (idx < 0) {
			break;
		}
		/* swap ant selection per-band specific */
		wlc_swdiv_antsel_select(clist, band, idx, TRUE, &rx_map);
		WL_SWDIV(("%s: (core%d) swap rxsel.cur=%d \n",
			__FUNCTION__, idx, clist->rxsel.cur));
		/* stat update */
		sidx = SWDIV_GETSIDX(clist->rxsel.cur, idx);
		scbcb->stats->swaprsn_prev[sidx] = SWDIV_EVENT_TIMER;
		scbcb->stats->swapcnt_linkmon[sidx]++;

		if (update_snr) {
			scbcb->snr_sum_per_ant[sidx] >>= 1;	/* cut half */
			WL_SWDIV(("%s: (core%d) [sidx=%d] Reduce SNR in Q3 %d to",
				__FUNCTION__, idx, sidx, scbcb->snr_avg_per_ant[sidx]));
			scbcb->snr_avg_per_ant[sidx] =
				scbcb->snr_sum_per_ant[sidx] >>
					swdiv->log2_num_pkt_avg;
			WL_SWDIV((" %d\n", scbcb->snr_avg_per_ant[sidx]));
		}
		scbcb->polltime_elapsed[idx] = 0;
	}
	scbcb->linkmon->is_snrdropped = FALSE;
	scbcb->linkmon->is_timerset = FALSE;
	wlc_swdiv_antmap_shmem_update(swdiv, scbcb, rx_map);

	return;
}

/* snr computing calculation */
static void
wlc_swdiv_snr_compute(wlc_swdiv_info_t *swdiv, d11rxhdr_t *rxh, swdiv_scb_cubby_t *scb,
	int8 *rssi, int32 *snr_out, bool isCCK)
{
	int idx = 0;
	int32 tmpsnr = 0;

	WL_INFORM(("%s : SNR [", __FUNCTION__));
	/* SNR calculation */
	FOREACH_SWDIV_ANTMAP(ACPHY_ANTMAP(rxh), idx) {
#if defined(WLC_SWDIV_ACPHY_QDOT1SQ_SNR_SUPPORT)
		/* SQ in dBm Q.1 format is used as SNR */
		if (idx == 0) {
			tmpsnr = PHY_RXPWR_ANT0(swdiv->wlc->pub->corerev, rxh);
		} else if (idx == 1) {
			tmpsnr = PHY_RXPWR_ANT1(swdiv->wlc->pub->corerev, rxh);
		} else if (idx == 2) {
			tmpsnr = PHY_RXPWR_ANT2(swdiv->wlc->pub->corerev, rxh);
		} else if (idx == 3) {
			tmpsnr = PHY_RXPWR_ANT3(swdiv->wlc->pub->corerev, rxh);
		}
#elif defined(WLC_SWDIV_ACPHY_LOG2SQ_SNR_SUPPORT)
		/* SQ for CORE 0 in log2 is used as SNR : BCM4355 */
		if (idx == 0 || idx == 1) {
			tmpsnr = LOG10_CONV((ltoh16(rxh->lt80.PhyRxStatus_5) >> (idx * 8)) & 0xFF);
		}
#else /* WLC_SWDIV_ACPHY_LOG2SQ_SNR_SUPPORT */
		/* compute snr based on rssi signal : 43162 and others */
		if (rssi[idx] == WLC_RSSI_INVALID) {
			tmpsnr = SWDIV_SNR_MIN_ACPHY;
		} else {
			tmpsnr = rssi[idx] - SWDIV_NOISE_INIT;
		}
		/* skip CCK comp because noisest is invalid */
		isCCK = FALSE;
#endif /* !WLC_SWDIV_ACPHY_QDOT1SQ_SNR_SUPPORT && !WLC_SWDIV_ACPHY_LOG2SQ_SNR_SUPPORT */
		tmpsnr = (tmpsnr < 0) ? 0 : tmpsnr;
		snr_out[idx] = tmpsnr;

		/* Additional compansation for CCK */
		if (isCCK) {
			snr_out[idx] =
				rssi[idx] - scb->noisest_per_ant[idx] - swdiv->cck_snr_corr;
		}
		snr_out[idx] = LIMIT_TO_MAX(snr_out[idx], SWDIV_SNR_MAX_ACPHY);
		snr_out[idx] = LIMIT_TO_MIN(snr_out[idx], SWDIV_SNR_MIN_ACPHY);
		WL_INFORM((" %d", snr_out[idx]));
	} /* END of FOREACH LOOP */
	WL_INFORM((" ]\n"));
}

static void
wlc_swdiv_rxpkt_scbstats_update(wlc_swdiv_info_t *swdiv, swdiv_scb_cubby_t *scbcb,
	uint16 antmap, int32 *snr, int8 *rssi, bool isCCK)
{
	int idx = 0, sidx = 0;
	swdiv_core_entity_t *clist;

	clist = swdiv->corelist;
	FOREACH_SWDIV_ACTCORE(clist) {
		idx = wlc_swdiv_bitpos_get(clist->coreid);
		if (idx < 0) {
			return;
		}
		sidx =  SWDIV_GETSIDX(SWDIV_GETBIT_BY_MAP(antmap, idx), idx);
		if (!isCCK) {
			scbcb->noisest_per_ant[sidx] = rssi[idx] - (int8)snr[idx];
		}
		/* stats updating */
		scbcb->snr_sum_per_ant[sidx] =
			scbcb->snr_sum_per_ant[sidx] -
			scbcb->snr_avg_per_ant[sidx] + (snr[idx] * SWDIV_1BY8_DB);
		/* init the buf with snr for fast convergence */
		if (scbcb->stats->swaprsn_prev[sidx] == SWDIV_EVENT_TIMER ||
			scbcb->stats->swaprsn_prev[sidx] == SWDIV_EVENT_WATCHDOG ||
			scbcb->stats->swaprsn_prev[sidx] == SWDIV_EVENT_TXFAIL) {
			scbcb->stats->swaprsn_prev[sidx] = SWDIV_EVENT_IDLE;
			scbcb->snr_sum_per_ant[sidx] =
				(snr[idx] * SWDIV_1BY8_DB) << swdiv->log2_num_pkt_avg;
		}
		/* both average and sum snr values are in Q.3 format */
		scbcb->snr_avg_per_ant[sidx] =
			scbcb->snr_sum_per_ant[sidx] >> swdiv->log2_num_pkt_avg;
		scbcb->rxpktcnt_per_ant[sidx]++;
		scbcb->stats->rxpktcnt_total_per_ant[sidx]++;
	}
	/* update per-scb base statistics  */
	scbcb->stats->rxpktcnt_accumulated++;
}

static int32
wlc_swdiv_policy_mapping(int coreidx,
	uint32 rx_policy,
	uint32 tx_policy,
	uint32 cell_policy,
	uint8 *rxp,
	uint8 *txp,
	uint8 *cellp_on,
	uint8 *cellp_off)
{
	int coreshift_bits = coreidx * SWDIV_PLCY_BITS_PER_CORE;
	int32 ret = BCME_OK;
	int idx = 0;
	int bandsel;

	UNUSED_PARAMETER(txp);

	/* split the mixed info core entity object */
	for (idx = 0; idx < MAX_BAND_TYPES; idx++) {
		bandsel = (idx == 0) ? SWDIV_BANDSEL_2G : SWDIV_BANDSEL_5G;
		cellp_on[idx] =
			SWDIV_CPLCY_GET(cell_policy, bandsel, SWDIV_CELL_ON, coreshift_bits);
		cellp_off[idx] =
			SWDIV_CPLCY_GET(cell_policy, bandsel, SWDIV_CELL_OFF, coreshift_bits);
		rxp[idx] =
			SWDIV_PLCY_GET(rx_policy, bandsel, coreshift_bits);
		/* validation */
		ret = wlc_swdiv_policy_validchk(rxp[idx], SWDIV_POLICY_INVALID,
			cellp_on[idx], cellp_off[idx]);
		if (ret != BCME_OK) {
			break;
		}
	}
	WL_SWDIV(("%s, (coreidx=%d|shift=%d): rx(%d,%d)|cellon(%d,%d)|celloff(%d,%d)\n",
		__FUNCTION__, coreidx, coreshift_bits, rxp[0], rxp[1],
		cellp_on[0], cellp_on[1], cellp_off[0], cellp_off[1]));

	return ret;
}

/* autoswap indicates caller want to swap the cur ant in automode */
static int32
wlc_swdiv_antsel_select(swdiv_core_entity_t *clist, uint8 band, int idx, bool isswap,
	uint16 *rxantmap)
{
	if ((!clist->support_2g && band == SWDIV_BANDSEL_2G) ||
		(!clist->support_5g && band == SWDIV_BANDSEL_5G)) {
		WL_SWDIV(("%s: band mismatch (coreid=0x%x, 2g=%d, 5g=%d, band=%d\n",
			__FUNCTION__, clist->coreid, clist->support_2g, clist->support_5g, band));
		return BCME_BADBAND;
	}
	if (clist->rx_curplcy[band] != SWDIV_POLICY_AUTO) {
		/* policy has the top priority to check than isswap flag */
		clist->rxsel.requested =
			(clist->rx_curplcy[band] == SWDIV_POLICY_FORCE_0) ?
			SWDIV_ANT0_INUSE : SWDIV_ANT1_INUSE;
	} else {
		/* any non-swap case in auto mode, ant will start with ANT0 */
		clist->rxsel.requested =
			(isswap) ? !clist->rxsel.cur : SWDIV_ANT0_INUSE;
	}
	WL_SWDIV(("%s coreid=0x%x : rxsel.reqtd=%d, bandidx=%d\n", __FUNCTION__,
		clist->coreid, clist->rxsel.requested, band));
	if (rxantmap && (*rxantmap != SWDIV_NULL_ANTMAP) &&
		(clist->rxsel.requested != SWDIV_ANT_INVALID)) {
		SWDIV_SETBIT_ON_MAP(*rxantmap, idx, clist->rxsel.requested);
		/* rxsel.cur will be updated by uCode antmap */
	}
	return BCME_OK;
}

static int32
wlc_swdiv_corelist_policy_priotize(swdiv_core_entity_t *clist, bool cellstatus,
	uint8 band, uint8 *rxp, uint8 *txp, uint8 *cellp_on, uint8 *cellp_off)
{
	int bandidx = 0;

	UNUSED_PARAMETER(txp);

	/* apply each band info */
	for (bandidx = 0; bandidx < MAX_BAND_TYPES; bandidx++) {
		if ((!clist->support_2g && bandidx == SWDIV_BANDSEL_2G) ||
			(!clist->support_5g && bandidx == SWDIV_BANDSEL_5G)) {
			if (rxp[bandidx] != 0x0) {
				WL_ERROR(("%s, coreid=0x%x : bandidx=%d, rxp(%d)|txp(%d)\n",
					__FUNCTION__, clist->coreid, bandidx,
					rxp[bandidx], txp[bandidx]));
				return BCME_UNSUPPORTED;
			}
		}
		if (cellstatus &&
			(clist->mwsplcy[bandidx] != SWDIV_POLICY_INVALID)) {
			/* this is a kind of top priority decision.
			 * mws integration already made at this point.
			 */
			clist->rx_curplcy[bandidx] = clist->mwsplcy[bandidx];
		} else {
			/* if cellstatus off or no mws IOVAR set even in cellon */
			if (rxp[bandidx] == SWDIV_POLICY_FOLLOW_CELL) {
				clist->rx_curplcy[bandidx] =
					(cellstatus) ? cellp_on[bandidx] : cellp_off[bandidx];
			} else {
				clist->rx_curplcy[bandidx] = rxp[bandidx];
			}
		}
		clist->rxplcy[bandidx] = rxp[bandidx];
		clist->cellplcy_on[bandidx] = cellp_on[bandidx];
		clist->cellplcy_off[bandidx] = cellp_off[bandidx];
	}
	return BCME_OK;
}

static uint32
wlc_swdiv_ltecoex_antpref_integrate(uint16 rxantpref2g, uint16 rxantpref5g,
	uint16 txantpref2g, uint16 txantpref5g)
{
	uint16 rx2g_mws_plcy = 0;
	uint16 rx5g_mws_plcy = 0;

	/* AND op is ok for minimal subset calculation */
	rx2g_mws_plcy = rxantpref2g & txantpref2g;
	rx5g_mws_plcy = rxantpref5g & txantpref5g;

	/* pick the minimal set between rx and tx */
	return ((uint32)(rx5g_mws_plcy << 16) | rx2g_mws_plcy);
}

static void
wlc_swdiv_antpref_conf_set(wlc_swdiv_info_t *swdiv,
	swdiv_requester_type_t reqid, uint16 rxantpref2g, uint16 rxantpref5g,
	uint16 txantpref2g, uint16 txantpref5g, bool isSet)
{
	switch (reqid) {
		case SWDIV_REQ_FROM_LTE:
		{
			if (isSet) {
				swdiv->mws_antsel_ovr_rx = 1;
				swdiv->mws_antpref = wlc_swdiv_ltecoex_antpref_integrate(
					rxantpref2g, rxantpref5g, txantpref2g, txantpref5g);
			} else {
				swdiv->mws_antsel_ovr_rx = 0;
				swdiv->mws_antpref =
					(uint32)(SWDIV_NULL_ANTMAP << 16) | SWDIV_NULL_ANTMAP;
			}
		}
			break;
		case SWDIV_REQ_FROM_IOVAR:
			break;
		default:
			break;
	}
}

static int
wlc_swdiv_bitpos_get(uint8 input)
{
	int i = 0;
	int cnt = MAX_UCODE_COREBITS;
	for (i = 0; i < cnt; i++) {
		if ((input >> i) & 1)
			return i;
	}
	return -1;
}

static uint16
wlc_swdiv_plcy_to_antmap_conv(wlc_swdiv_info_t *swdiv, int band)
{
	swdiv_core_entity_t *clist = swdiv->corelist;
	int coreshift = 0, idx = 0;
	int bandidx = 0;
	uint8 rxplcy_type[MAX_BAND_TYPES];
	uint8 cellplcy_on_type[MAX_BAND_TYPES];
	uint8 cellplcy_off_type[MAX_BAND_TYPES];
	uint16 antmap = 0;
	int32 ret = BCME_OK;

	FOREACH_SWDIV_CORE(clist) {
		idx = wlc_swdiv_bitpos_get(clist->coreid);
		coreshift = idx * SWDIV_PLCY_BITS_PER_CORE;

		for (bandidx = 0; bandidx < MAX_BAND_TYPES; bandidx++) {
			if ((clist->support_2g && bandidx == SWDIV_BANDSEL_2G) ||
				(clist->support_5g && bandidx == SWDIV_BANDSEL_5G)) {
				rxplcy_type[bandidx] =
					SWDIV_PLCY_GET(swdiv->policy.rx, bandidx, coreshift);
				cellplcy_on_type[bandidx] =
					SWDIV_CPLCY_GET(swdiv->policy.cell, bandidx, 1, coreshift);
				cellplcy_off_type[bandidx] =
					SWDIV_CPLCY_GET(swdiv->policy.cell, bandidx, 0, coreshift);
			} else {
				rxplcy_type[bandidx] = 0x0;
				cellplcy_on_type[bandidx] = 0x0;
				cellplcy_off_type[bandidx] = 0x0;
			}
		}
		/* txplcy is obsoleated */
		ret = wlc_swdiv_corelist_policy_priotize(clist, swdiv->cellstatus, band,
			rxplcy_type, 0, cellplcy_on_type, cellplcy_off_type);
		if (ret != BCME_OK) {
			WL_ERROR(("%s: policy initialization fails!\n", __FUNCTION__));
		}
		wlc_swdiv_antsel_select(clist, band, idx, FALSE, &antmap);
	}
	return antmap;
}

static void
BCMATTACHFN(wlc_swdiv_plcytype_init)(wlc_swdiv_info_t *swdiv)
{
	int bandidx = 0;
	int shift = 0;
	uint8 defplcy_type;
	uint32 init_rxplcy = 0, init_cellplcy = 0;
	swdiv_core_entity_t *clist = swdiv->corelist;

	for (bandidx = 0; bandidx < MAX_BAND_TYPES; bandidx++) {
		FOREACH_SWDIV_CORE(clist) {
			shift =
				wlc_swdiv_bitpos_get(clist->coreid) * SWDIV_PLCY_BITS_PER_CORE;
			if ((clist->support_2g && bandidx == SWDIV_BANDSEL_2G) ||
				(clist->support_5g && bandidx == SWDIV_BANDSEL_5G)) {
				defplcy_type = SWDIV_POLICY_AUTO;
			} else {
				defplcy_type = 0x0;
			}
			init_rxplcy =
				SET_TYPE_IN_PLCY(defplcy_type, init_rxplcy, bandidx, shift);
			init_cellplcy =
				SET_TYPE_IN_CPLCY(0x0, init_cellplcy, bandidx, 0, shift);
			init_cellplcy =
				SET_TYPE_IN_CPLCY(0x0, init_cellplcy, bandidx, 1, shift);
		}
	}
	swdiv->policy.rx = init_rxplcy;
	swdiv->policy.cell = init_cellplcy;
}

static bool
wlc_swdiv_recvpkt_valid_check(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh)
{
	bool ret = TRUE;
	d11rxhdr_t *rxh;
	rxh = &wrxh->rxhdr;
	if (!(rxh->lt80.RxStatus2 & RXS_PHYRXST_VALID)) {
		if (rxh->lt80.RxStatus2 & RXS_AMSDU_MASK) {
			WL_SWDIV(("%s: Invalid AMSDU pkt (rxstat2=0x%x, inampdu=%d, aggtype=%d)\n",
				__FUNCTION__, rxh->lt80.RxStatus2, wlc->_rx_amsdu_in_ampdu,
				((rxh->lt80.RxStatus2 & RXS_AGGTYPE_MASK) >> RXS_AGGTYPE_SHIFT)));
			ret = TRUE;
		} else {
			/* if it is not AMSDU with invalid status, drop */
			ret = FALSE;
		}
	}
	return ret;
}

static int32
wlc_swdiv_policy_validchk(uint8 rxplcy, uint8 txplcy, uint8 cellplcy_on, uint8 cellplcy_off)
{
	int32 ret = BCME_OK;
	if ((rxplcy == SWDIV_POLICY_ALLOFF) ||
		(rxplcy > SWDIV_POLICY_FOLLOW_CELL) ||
		(cellplcy_on == SWDIV_POLICY_ALLOFF) ||
		(cellplcy_on > SWDIV_POLICY_AUTO) ||
		(cellplcy_off == SWDIV_POLICY_ALLOFF) ||
		(cellplcy_off > SWDIV_POLICY_FOLLOW_CELL)) {
		WL_ERROR(("%s: INVALID policy set (rx=0x%x, cellon=0x%x, celloff=0x%x) !!!\n",
			__FUNCTION__, rxplcy, cellplcy_on, cellplcy_off));
		ret = BCME_RANGE;
	}
	return ret;
}

static swdiv_core_entity_t *
wlc_swdiv_corelist_byidx_get(wlc_swdiv_info_t *swdiv, int coreidx)
{
	swdiv_core_entity_t *clist;
	int idx;
	swdiv_core_entity_t *ret = NULL;

	clist = swdiv->corelist;
	FOREACH_SWDIV_CORE(clist) {
		idx = wlc_swdiv_bitpos_get(clist->coreid);
		if (idx == coreidx) {
			ret = clist;
			return ret;
		}
	}
	return ret;
}

/* return : policy id */
static uint8
wlc_swdiv_antpref_to_plcy_conv(uint8 pref)
{
	uint8 ret_plcy;
	/* antpref value has a different id, need to covert it to policy id */
	if (pref == 0x0) {
		/* mws don't care */
		ret_plcy = SWDIV_POLICY_AUTO;
	} else if (pref == 0x1) {
		ret_plcy = SWDIV_POLICY_FORCE_0;
	} else if (pref == 0x2) {
		ret_plcy = SWDIV_POLICY_FORCE_1;
	} else if (pref == 0x3) {
		ret_plcy = SWDIV_POLICY_AUTO;
	} else {
		ret_plcy = SWDIV_POLICY_INVALID;
	}
	return ret_plcy;
}

static void
wlc_swdiv_scb_stats_reset(wlc_swdiv_info_t *swdiv, struct scb *scb)
{
	swdiv_scb_cubby_t *scbcb = NULL;
	int idx = 0;
	scbcb = SWDIV_SCB_CUBBY(swdiv, scb);
	if (scbcb) {
		/* any used val has to be reset */
		for (idx = 0; idx < MAX_HW_ANTNUM; idx++) {
			scbcb->noisest_per_ant[idx] = SWDIV_NOISE_INIT;
			scbcb->stats->swaprsn_prev[idx] = SWDIV_EVENT_IDLE;
			scbcb->stats->swapcnt_linkmon[idx] = 0;
			scbcb->snr_sum_per_ant[idx] = 0 * (1 << swdiv->log2_num_pkt_avg);
			scbcb->snr_avg_per_ant[idx] = 0;
			scbcb->rxpktcnt_per_ant[idx] = 0;
			scbcb->rxpktcnt_prev_per_ant[idx] = 0;
			scbcb->stats->rxpktcnt_total_per_ant[idx] = 0;
			scbcb->stats->swapcnt_snrdrop[idx] = 0;
			scbcb->stats->snr_prev_avg_per_ant[idx] = 0;
			scbcb->stats->swap_ge_rxcnt[idx] = 0;
			scbcb->stats->swap_ge_snrthresh[idx] = 0;
			scbcb->stats->swap_txfail[idx] = 0;
			scbcb->stats->swap_alivechk[idx] = 0;
		}
		for (idx = 0; idx < MAX_UCODE_COREBITS; idx++) {
			scbcb->polltime_elapsed[idx] = 0;
		}
		scbcb->settlecnt_rx = 0;
		scbcb->snrdrop_tick_wdog = 0;
		scbcb->stats->rxpktcnt_accumulated = 0;
		scbcb->stats->lastcoremap = 0;
		scbcb->stats->lastantmap = SWDIV_NULL_ANTMAP;
		scbcb->stats->reqantmap = SWDIV_NULL_ANTMAP;
		scbcb->stats->lastchanspec = 0;
	}
}

static int32
wlc_swdiv_rx_policy_get(wlc_swdiv_info_t *swdiv, int32 *retval)
{
	wlc_info_t *wlc;

	ASSERT(swdiv);
	wlc = swdiv->wlc;
	ASSERT(wlc);

	if (WLCISACPHY(wlc->band)) {
		*retval = (int32)swdiv->policy.rx;
		return BCME_OK;
	}
	return BCME_UNSUPPORTED;
}

static int32
wlc_swdiv_cell_policy_get(wlc_swdiv_info_t *swdiv, int32 *retval)
{
	wlc_info_t *wlc;
	ASSERT(swdiv);
	wlc = swdiv->wlc;
	ASSERT(wlc);

	if (WLCISACPHY(wlc->band)) {
		*retval = (int32)swdiv->policy.cell;
		return BCME_OK;
	}
	return BCME_UNSUPPORTED;
}

static uint16
wlc_swdiv_linkmon_dur_get(wlc_info_t *wlc, wlc_bsscfg_t* bsscfg)
{
	uint16 duration = 0;
	uint16 bcnrecv_dur_ms;
	int dtim_interval;

	/* calculate minimal wakeup interval to recv */
	if (bsscfg->associated) {
		if (wlc->bcn_li_dtim) {
			dtim_interval =
				LIMIT_TO_MIN(bsscfg->current_bss->dtim_period, wlc->bcn_li_dtim);
		} else {
			dtim_interval = bsscfg->current_bss->dtim_period;
		}
		bcnrecv_dur_ms =
			dtim_interval * bsscfg->current_bss->beacon_period;
		WL_SWDIV(("wl%d: %s: picked_dur=%d, beacon_per=%d, dtim=%d, bcn_li_dtim=%d\n",
			wlc->pub->unit, __FUNCTION__, bcnrecv_dur_ms,
			bsscfg->current_bss->beacon_period,
			bsscfg->current_bss->dtim_period,
			wlc->bcn_li_dtim));
	} else {
		/* unexpected situation, assume DTIM=3 */
		bcnrecv_dur_ms = SWDIV_LINKMON_DUR_DEFAULT;
		WL_SWDIV(("wl%d: %s: scb link is not associated. use def=%d\n",
			wlc->pub->unit, __FUNCTION__, bcnrecv_dur_ms));
	}
	duration = bcnrecv_dur_ms + SWDIV_LINKMON_EXTRA_DUR;
	return duration;
}

/* will be unlocked sooner or later */

#endif /* WLC_SW_DIVERSITY */
