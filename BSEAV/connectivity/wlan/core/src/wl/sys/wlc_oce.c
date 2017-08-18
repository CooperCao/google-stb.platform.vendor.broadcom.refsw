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
#include <wl_rte.h>
#include <wl_dbg.h>
#include <wlioctl.h>
#include <wlc_bsscfg.h>

#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_types.h>
#include <wlc_ie_mgmt_types.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_vs.h>
#include <proto/oce.h>
#include <wlc_oce.h>
#include <bcmwpa.h>
#include <wlc_scan_utils.h>

#define OCE_PROBE_DEFFERAL_TIME		20

static const uint8 oce_pref_channels[] = {1, 6, 11};

typedef struct wlc_oce_cmn_info {
	bool env_inc_non_oce_ap; /* indicates non-oce AP(s) presence */
	uint8 probe_defer_time;	/* probe request deferral time */
} wlc_oce_cmn_info_t;

struct wlc_oce_info {
	wlc_info_t *wlc;
	/* shared info */
	wlc_oce_cmn_info_t* oce_cmn_info;
	int      cfgh;    /* oce bsscfg cubby handle */
};

/* NOTE: This whole struct is being cloned across with its contents and
 * original memeory allocation intact.
 */
typedef struct wlc_oce_bsscfg_cubby {
} wlc_oce_bsscfg_cubby_t;

#define OCE_BSSCFG_CUBBY_LOC(oce, cfg) ((wlc_oce_bsscfg_cubby_t **)BSSCFG_CUBBY(cfg, (oce)->cfgh))
#define OCE_BSSCFG_CUBBY(oce, cfg) (*OCE_BSSCFG_CUBBY_LOC(oce, cfg))
#define OCE_CUBBY_CFG_SIZE  sizeof(wlc_oce_bsscfg_cubby_t)

/* iovar table */
enum {
	IOV_OCE, /* enable/dsiable OCE */
#if defined(STA) && defined(BCMDBG)
	IOV_OCE_PROBE_DELAY, /* WFA OCE speci v5 3.2 Probe Request Deferral And Suppression */
#endif
	IOV_LAST
};

static const bcm_iovar_t oce_iovars[] = {
	{"oce", 0, 0, 0, IOVT_BUFFER, 0},
#if defined(STA) && defined(BCMDBG)
	{"oce_probe_delay", IOV_OCE_PROBE_DELAY, IOVF_BSSCFG_STA_ONLY, 0, IOVT_UINT16, 0},
#endif
	{NULL, 0, 0, 0, 0, 0}
};

static int
wlc_oce_doiovar(void *hdl, uint32 actionid, void *params, uint p_len,
	void *arg, uint len, uint val_size, struct wlc_if *wlcif);
static void wlc_oce_watchdog(void *ctx);
static int wlc_oce_wlc_up(void *ctx);
static int wlc_oce_wlc_down(void *ctx);
static int wlc_oce_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_oce_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg);
static int
wlc_oce_bsscfg_get(void *ctx, wlc_bsscfg_t *cfg, uint8 *data, int *len);
static int
wlc_oce_bsscfg_set(void *ctx, wlc_bsscfg_t *cfg, const uint8 *data, int len);
static wifi_oce_cap_ind_attr_t*
wlc_oce_find_cap_ind_attr(uint8 *parse, uint len);
static void wlc_oce_detect_environment(void *ctx, scan_utl_scan_data_t *sd);
static void wlc_oce_reset_environment(void *ctx);
static ratespec_t
wlc_oce_get_oce_compat_rspec(wlc_oce_info_t *oce);
static uint16
wlc_oce_write_oce_cap_ind_attr(uint8 *cp, uint8 buf_len);
static int
wlc_mbo_oce_ie_build_fn(void *ctx, wlc_iem_build_data_t *data);
static uint
wlc_mbo_oce_ie_calc_len(void *ctx, wlc_iem_calc_data_t *data);
static int
wlc_mbo_oce_ie_parse_fn(void *ctx, wlc_iem_parse_data_t *data);

wlc_oce_info_t *
BCMATTACHFN(wlc_oce_attach)(wlc_info_t *wlc)
{
	wlc_oce_info_t *oce = NULL;
	wlc_oce_cmn_info_t *oce_cmn = NULL;
	bsscfg_cubby_params_t cubby_params;
	int ret = BCME_OK;
	uint16 mbo_oce_ie_build_fstbmp = FT2BMP(FC_ASSOC_REQ) |
		FT2BMP(FC_REASSOC_REQ) |
		FT2BMP(FC_PROBE_REQ);
	uint16 mbo_oce_ie_parse_fstbmp = FT2BMP(FC_BEACON) |
		FT2BMP(FC_PROBE_RESP) |
		FT2BMP(FC_ASSOC_RESP) |
		FT2BMP(FC_REASSOC_RESP);

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
	}
	oce->oce_cmn_info = oce_cmn;
	(void)obj_registry_ref(wlc->objr, OBJR_OCE_CMN_INFO);

	oce->wlc = wlc;

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

	wlc->pub->cmn->_oce = TRUE;

	ret = wlc_scan_utils_rx_scan_register(wlc, wlc_oce_detect_environment, oce);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_scan_utils_rx_scan_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	ret = wlc_scan_utils_scan_start_register(wlc, wlc_oce_reset_environment, oce);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_scan_utils_scan_start_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	oce->oce_cmn_info->probe_defer_time = OCE_PROBE_DEFFERAL_TIME;

	/* register MBO-OCE IE build callbacks */
	ret = wlc_iem_vs_add_build_fn_mft(wlc->iemi,
			mbo_oce_ie_build_fstbmp, WLC_IEM_VS_IE_PRIO_MBO,
		wlc_mbo_oce_ie_calc_len, wlc_mbo_oce_ie_build_fn, oce);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_vs_add_build_fn_mft(cell data) failed %d, \n",
			wlc->pub->unit, __FUNCTION__, ret));
		goto fail;
	}

	/* register MBO-OCE IE parse callbacks */
	ret = wlc_iem_vs_add_parse_fn_mft(wlc->iemi,
			mbo_oce_ie_parse_fstbmp, WLC_IEM_VS_IE_PRIO_MBO,
		wlc_mbo_oce_ie_parse_fn, oce);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_vs_add_parse_fn failed:mbo ap cap ind\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

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
			MFREE(oce->wlc->osh, oce->oce_cmn_info, sizeof(*oce->oce_cmn_info));
			oce->oce_cmn_info = NULL;
		}

		wlc_scan_utils_rx_scan_unregister(oce->wlc, wlc_oce_detect_environment, oce);
		wlc_scan_utils_scan_start_unregister(oce->wlc, wlc_oce_reset_environment, oce);

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
	return BCME_OK;
}

static int
wlc_oce_wlc_down(void *ctx)
{
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
	ASSERT(obc != NULL);
	MFREE(wlc->osh, obc, sizeof(*obc));
	pobc = NULL;
	return;
}

/* bsscfg copy :get function */
static int
wlc_oce_bsscfg_get(void *ctx, wlc_bsscfg_t *cfg, uint8 *data, int *len)
{
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;
	wlc_oce_bsscfg_cubby_t *obc = NULL;

	if (data == NULL || *len < OCE_CUBBY_CFG_SIZE) {
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
	memset(obc, 0, OCE_CUBBY_CFG_SIZE);

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
	if (data == NULL || len < OCE_CUBBY_CFG_SIZE) {
		return BCME_BADARG;
	}
	memcpy(obc, (wlc_oce_bsscfg_cubby_t *)data, OCE_CUBBY_CFG_SIZE);
	return BCME_OK;
}

static int
wlc_oce_doiovar(void *hdl, uint32 actionid, void *params, uint p_len,
	void *arg, uint len, uint val_size, struct wlc_if *wlcif)
{
	int ret = BCME_OK;
	int32 int_val = 0;
#if defined(STA) && defined(BCMDBG)
	wlc_oce_info_t *oce = (wlc_oce_info_t *)hdl;
	wlc_info_t *wlc = oce->wlc;
	int32 *ret_int_ptr;
	ret_int_ptr = (int32 *)arg;
#endif /* STA && BCMDBG */

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		memcpy(&int_val, params, sizeof(int_val));

	switch (actionid) {

#if defined(STA) && defined(BCMDBG)
	case IOV_GVAL(IOV_OCE_PROBE_DELAY):
		*ret_int_ptr = (int32)oce->oce_cmn_info->probe_defer_time;

		break;
	case IOV_SVAL(IOV_OCE_PROBE_DELAY):
		oce->oce_cmn_info->probe_defer_time = (uint8)int_val;

		break;
#endif /* STA && BCMDBG */
	default:
		ret = BCME_UNSUPPORTED;
		break;
	};

	return ret;
}

/* Check whether pointed-to IE looks like MBO_OCE. */
#define bcm_is_mbo_oce_ie(ie, tlvs, len)	bcm_has_ie(ie, tlvs, len, \
	(const uint8 *)MBO_OCE_OUI, WFA_OUI_LEN, MBO_OCE_OUI_TYPE)

/* return OCE Capabilities Indication Attribute or NULL */
static wifi_oce_cap_ind_attr_t*
wlc_oce_find_cap_ind_attr(uint8 *parse, uint len)
{
	bcm_tlv_t *ie;

	while ((ie = bcm_parse_tlvs(parse, (int)len, MBO_OCE_IE_ID))) {
		if (bcm_is_mbo_oce_ie((uint8 *)ie, &parse, &len)) {
			wifi_oce_cap_ind_attr_t* oce_cap_ind_attr;

			oce_cap_ind_attr = (wifi_oce_cap_ind_attr_t*)
				bcm_parse_tlvs((uint8*)ie + MBO_OCE_IE_HDR_SIZE,
					ie->len - MBO_OCE_IE_NO_ATTR_LEN,
					OCE_ATTR_OCE_CAPABILITY_INDICATION);

			if (oce_cap_ind_attr) {
				return oce_cap_ind_attr;
			} else {
				parse += (ie->len + TLV_HDR_LEN);
				len -= (ie->len + TLV_HDR_LEN);
			}
		}
	}

	return NULL;
}

static void
wlc_oce_detect_environment(void* ctx, scan_utl_scan_data_t *sd)
{
	ASSERT(ctx && sd);
	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;
	if (!oce->oce_cmn_info->env_inc_non_oce_ap) {
		wifi_oce_cap_ind_attr_t* oce_cap_ind_attr;
			oce_cap_ind_attr = wlc_oce_find_cap_ind_attr(sd->body + DOT11_BCN_PRB_LEN,
				sd->body_len - DOT11_BCN_PRB_LEN);
		if (!oce_cap_ind_attr) {
			oce->oce_cmn_info->env_inc_non_oce_ap = TRUE;
		}
	}
}

static void
wlc_oce_reset_environment(void *ctx)
{
	ASSERT(ctx);

	wlc_oce_info_t *oce = (wlc_oce_info_t *)ctx;

	oce->oce_cmn_info->env_inc_non_oce_ap = FALSE;
}

/* Pick up rate compatible with OCE requirements (minimum 5.5 mbps) */
static ratespec_t
wlc_oce_get_oce_compat_rspec(wlc_oce_info_t *oce)
{
	wlc_info_t *wlc = oce->wlc;
	wlc_rateset_t default_rateset;
	wlc_rateset_t basic_rateset;
	int i;

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

	if ((BAND_5G(wlc->band->bandtype))) {
		return wlc_sendmgmt(wlc, p, wlc->active_queue, NULL);
	}

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
wlc_oce_write_oce_cap_ind_attr(uint8 *cp, uint8 buf_len)
{
	ASSERT(buf_len >= sizeof(wifi_oce_cap_ind_attr_t));

	/* Attribute id */
	*cp = OCE_ATTR_OCE_CAPABILITY_INDICATION;
	cp += OCE_ATTR_ID_LEN;

	/* Attribute len */
	*cp = sizeof(uint8);
	cp += OCE_ATTR_LEN_LEN;

	/* Attribute data */
	*cp = OCE_RELEASE;

	return sizeof(wifi_oce_cap_ind_attr_t);
}

/* Build MBO-OCE IE with:
 * "OCE Capabilities Attribute" for (re)assoc req and probe req,
 * "FILS Request Parameters IE" for probe req.
 * OCE Tech Spec v5 section 4.1, 4.2.1, 3.7.
 */
static int
wlc_mbo_oce_ie_build_fn(void *ctx, wlc_iem_build_data_t *data)
{
	uint8 *cp = NULL;
	wifi_mbo_oce_ie_t *ie_hdr = NULL;

	uint16 attr_len = 0;
	uint total_len = 0;
	uint buf_len = 0;

	cp = data->buf;
	ie_hdr = (wifi_mbo_oce_ie_t *)cp;

	ASSERT(data->buf_len >= MBO_OCE_IE_HDR_SIZE);

	/* fill in MBO-OCE IE header */
	ie_hdr->id = MBO_OCE_IE_ID;
	memcpy(ie_hdr->oui, MBO_OCE_OUI, WFA_OUI_LEN);
	ie_hdr->oui_type = MBO_OCE_OUI_TYPE;
	cp += MBO_OCE_IE_HDR_SIZE;

	total_len = MBO_OCE_IE_NO_ATTR_LEN;
	buf_len = MBO_OCE_IE_HDR_SIZE;

	switch (data->ft) {
		case FC_ASSOC_REQ:
		case FC_REASSOC_REQ:
		case FC_PROBE_REQ:
			/* fill in Cellular Data Capability Attr */
			attr_len = wlc_oce_write_oce_cap_ind_attr(cp, buf_len);
			cp += attr_len;
			buf_len += attr_len;
			total_len += attr_len;
			break;
		default:
			/* we should not be here !! */
			ASSERT(0);
	}

	/* update MBO-OCE IE len */
	ie_hdr->len = total_len;

	return BCME_OK;
}

/* Calculate bytes needed to write MBO_OCE IE with:
 * "OCE Capabilities Indication Attribute" for (re)assoc req and probe req,
 * "FILS Request Parameters IE" for probe req.
 * OCE Tech Spec v5 section 4.1, 4.2.1, 3.7.
 */
static uint
wlc_mbo_oce_ie_calc_len(void *ctx, wlc_iem_calc_data_t *data)
{
	uint total_len = 0;

	total_len = MBO_OCE_IE_HDR_SIZE;

	switch (data->ft) {
		case FC_ASSOC_REQ:
		case FC_REASSOC_REQ:
		case FC_PROBE_REQ:
			total_len += sizeof(wifi_oce_cap_ind_attr_t);
			break;
		default:
			ASSERT(0);
	}

	return total_len;
}

static int
wlc_mbo_oce_ie_parse_fn(void *ctx, wlc_iem_parse_data_t *data)
{
	/* validate minimum IE length */
	if (data->ie_len <= MBO_OCE_IE_HDR_SIZE) {
		return BCME_OK;
	}

	switch (data->ft) {
		case FC_ASSOC_RESP:
		case FC_REASSOC_RESP:
			break;
		case FC_BEACON:
		case FC_PROBE_RESP:
			break;
		default:
			ASSERT(0);
	}

	return BCME_OK;
}

uint8
wlc_oce_get_pref_channels(chanspec_t *chanspec_list)
{
	int i;

	for (i = 0; i < sizeof(oce_pref_channels); i++) {
		chanspec_list[i] = CH20MHZ_CHSPEC(oce_pref_channels[i]);
	}

	return sizeof(oce_pref_channels);
}

bool wlc_oce_is_pref_channel(chanspec_t chanspec)
{
	int i;

	for (i = 0; i < sizeof(oce_pref_channels); i++) {
		if (CHSPEC_CHANNEL(chanspec) == oce_pref_channels[i])
			return TRUE;
	}

	return FALSE;
}
