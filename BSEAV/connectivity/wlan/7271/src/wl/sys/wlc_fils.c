/*
 * FILS Authentication for OCE implementation for
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
*	OCE uses the FILS shared key authentication 11.11.2.3.1 and
*	FILS Higher Layer Setup with Higher Layer Protocol Encapsulation (10.47.3)
*	from 802.11ai D6.0

*	Place holder for fils authentication module.

*	For latest design proposal, please refer to
*	http://confluence.broadcom.com/display/WLAN/FILS+Authentication+for+OCE

*	The file names are kept as wlc_fils.c / wlc_fils.h.
*	802.11ai in full is not a requirement for OCE.
*	OCE has a pre-requisite for FILS authentication and FILS HLS.

*	In future, if 802.11ai full implementation is needed, we can have a wrapper file
*	like wlc_fils.c / wlc_fils.h which will include wlc_fils.c/.h file to cover
*	up the FILS authentication. Having wlc_fils.c/.h for now will be misleading.
*/

#include <wlc_cfg.h>
#include <typedefs.h>
#include <osl.h>
#include <bcmendian.h>
#include <wl_dbg.h>
#include <wlioctl.h>
#include <wlc_bsscfg.h>

#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_types.h>
#include <wlc_ie_mgmt_types.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_vs.h>
#include <wlc_ie_mgmt_ft.h>
#include <proto/802.11.h>
#include <proto/fils.h>
#include <wlc_fils.h>
#include <wlc_wnm.h>
#include <bcmiov.h>

static const bcm_iovar_t fils_iovars[] = {
	{ "fils", 0, 0, 0, IOVT_BUFFER, 0 },
	{ NULL, 0, 0, 0, 0, 0 }
};

typedef struct wlc_fils_cmn {
	fils_indication_element_t *fils_ind;
	uint16 fils_ind_len;
	fils_hlp_container_element_t *fils_hlp;
	uint16 fils_hlp_len;
} wlc_fils_cmn_t;

typedef struct wlc_fils_info {
	wlc_info_t *wlc;
	wlc_fils_cmn_t* fils_cmn;
	int cfgh;    /* fils bsscfg cubby handle */
	bcm_iov_parse_context_t *iov_parse_ctx;
} wlc_fils_info_t;

/* NOTE: This whole struct is being cloned across with its contents and
 * original memeory allocation intact.
 */
typedef struct wlc_fils_bsscfg_cubby {
	uint8 *fils_ies_data;
	uint16 fils_ies_data_len;
} wlc_fils_bsscfg_cubby_t;

#define FILS_BSSCFG_CUBBY_LOC(fils, cfg) \
	((wlc_fils_bsscfg_cubby_t **)BSSCFG_CUBBY(cfg, (fils)->cfgh))
#define FILS_BSSCFG_CUBBY(fauth, cfg) (*FILS_BSSCFG_CUBBY_LOC(fils, cfg))
#define FILS_CUBBY_CFG_SIZE  sizeof(wlc_fils_bsscfg_cubby_t)

/* module registration functionality */
static int wlc_fils_wlc_up(void *ctx);
static int wlc_fils_wlc_down(void *ctx);
static void wlc_fils_watchdog(void *ctx);

/* cubby registration functionality */
static int wlc_fils_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_fils_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg);
static int wlc_fils_bsscfg_get(void *ctx,
	wlc_bsscfg_t *cfg, uint8 *data, int *len);
static int wlc_fils_bsscfg_set(void *ctx,
	wlc_bsscfg_t *cfg, const uint8 *data, int len);

/* iovar handlers */
static void *
fils_iov_context_alloc(void *ctx, uint size);
static void
fils_iov_context_free(void *ctx, void *iov_ctx, uint size);
static int
BCMATTACHFN(fils_iov_get_digest_cb)(void *ctx, bcm_iov_cmd_digest_t **dig);
static int
wlc_fils_iov_cmd_validate(const bcm_iov_cmd_digest_t *dig, uint32 actionid,
	const uint8 *ibuf, size_t ilen, uint8 *obuf, size_t *olen);
static int wlc_fils_iov_add_hlp_ie(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen);
#ifdef AP
static int wlc_fils_iov_add_ind_ie(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen);
#endif
static int wlc_fils_iov_add_auth_data(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen);

#ifdef AP
static uint wlc_fils_fils_ind_elem_calc_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_fils_fils_ind_elem_build_fn(void *ctx, wlc_iem_build_data_t *data);
static int wlc_fils_write_fils_ind_ie(wlc_fils_info_t *fils,
	const void *arg, uint len);
#endif /* AP */
static int wlc_fils_write_fils_auth_data(wlc_fils_info_t *fils,
	wlc_bsscfg_t *bsscfg, const void *arg, uint len);
/* FILS IEs */
static uint wlc_fils_fils_ies_calc_len(void *ctx,
	wlc_iem_calc_data_t *data);
static int wlc_fils_fils_ies_build_fn(void *ctx,
	wlc_iem_build_data_t *data);
static uint wlc_fils_hlp_elem_calc_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_fils_hlp_elem_build_fn(void *ctx, wlc_iem_build_data_t *data);
static uint16 wlc_fils_get_hlp_elem_size(wlc_fils_info_t *fils);
static int wlc_fils_write_hlp_ie(wlc_fils_info_t *fils,
	const void *arg, uint len);

#define MAX_ADD_IND_IE_SIZE		64
#define MIN_ADD_IND_IE_SIZE		4
#define MAX_ADD_HLP_IE_SIZE		576
#define MIN_ADD_HLP_IE_SIZE		32
#define MAX_ADD_AUTH_DATA_SIZE	256
#define MIN_ADD_AUTH_DATA_SIZE	7

static const bcm_iov_cmd_info_t fils_sub_cmds[] = {
#ifdef AP
	{WL_FILS_CMD_ADD_IND_IE, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32,
	wlc_fils_iov_cmd_validate, NULL, wlc_fils_iov_add_ind_ie, 0,
	MIN_ADD_IND_IE_SIZE, MAX_ADD_IND_IE_SIZE, 0, 0
	},
#endif /* AP */
	{WL_FILS_CMD_ADD_HLP_IE, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32,
	wlc_fils_iov_cmd_validate, NULL, wlc_fils_iov_add_hlp_ie, 0,
	MIN_ADD_HLP_IE_SIZE, MAX_ADD_HLP_IE_SIZE, 0, 0
	},
	{WL_FILS_CMD_ADD_AUTH_DATA, BCM_IOV_CMD_FLAG_NONE,
	0, BCM_XTLV_OPTION_ALIGN32,
	wlc_fils_iov_cmd_validate, NULL, wlc_fils_iov_add_auth_data, 0,
	MIN_ADD_AUTH_DATA_SIZE, MAX_ADD_AUTH_DATA_SIZE, 0, 0
	}
};

#define SUBCMD_TBL_SZ(_cmd_tbl)  (sizeof(_cmd_tbl)/sizeof(*_cmd_tbl))

wlc_fils_info_t *
BCMATTACHFN(wlc_fils_attach)(wlc_info_t *wlc)
{
	bcm_iov_parse_context_t *parse_ctx = NULL;
	bcm_iov_parse_config_t parse_cfg;
	wlc_fils_info_t *fils = NULL;
	wlc_fils_cmn_t *fils_cmn = NULL;
	bsscfg_cubby_params_t cubby_params;
	uint16 fils_auth_bld_fstbmp = FT2BMP(FC_AUTH);
	int ret = BCME_OK;
#ifdef AP
	uint16 fils_ind_elm_build_fstbmp = FT2BMP(FC_BEACON) |
		FT2BMP(FC_PROBE_RESP);
#endif /* AP */
	uint16 fils_hlp_elm_build_fstbmp = FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ);
#ifdef AP
	fils_hlp_elm_build_fstbmp |= (FT2BMP(FC_ASSOC_RESP) | FT2BMP(FC_REASSOC_RESP));
#endif /* AP */

	fils = (wlc_fils_info_t *)MALLOCZ(wlc->osh, sizeof(*fils));
	if (fils == NULL) {
		WL_ERROR(("wl%d: %s:out of mem. alloced %u bytes\n",
			wlc->pub->unit, __FUNCTION__,  MALLOCED(wlc->osh)));
		goto fail;
	}

	fils->wlc = wlc;

	fils_cmn = (wlc_fils_cmn_t *) obj_registry_get(wlc->objr, OBJR_FILS_CMN_INFO);
	if (fils_cmn == NULL) {
		fils_cmn = (wlc_fils_cmn_t *)MALLOCZ(wlc->osh,
			sizeof(*fils_cmn));
		if (fils_cmn == NULL) {
			WL_ERROR(("wl%d: %s:out of mem. alloced %u bytes\n",
				wlc->pub->unit, __FUNCTION__,  MALLOCED(wlc->osh)));
			goto fail;
		}
		obj_registry_set(wlc->objr, OBJR_FILS_CMN_INFO, fils_cmn);
	}
	fils->fils_cmn = fils_cmn;
	BCM_REFERENCE(obj_registry_ref(wlc->objr, OBJR_FILS_CMN_INFO));

	/* parse config */
	parse_cfg.alloc_fn = (bcm_iov_malloc_t)fils_iov_context_alloc;
	parse_cfg.free_fn = (bcm_iov_free_t)fils_iov_context_free;
	parse_cfg.dig_fn = (bcm_iov_get_digest_t)fils_iov_get_digest_cb;
	parse_cfg.max_regs = 1;
	parse_cfg.alloc_ctx = (void *)fils;

	/* parse context */
	ret = bcm_iov_create_parse_context((const bcm_iov_parse_config_t *)&parse_cfg,
		&parse_ctx);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s parse context creation failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	fils->iov_parse_ctx = parse_ctx;

	/* register module */
	ret = wlc_module_register(wlc->pub, fils_iovars, "FILS", (void *)parse_ctx,
		bcm_iov_doiovar, wlc_fils_watchdog,
		wlc_fils_wlc_up, wlc_fils_wlc_down);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register fils subcommands */
	ret = bcm_iov_register_commands(fils->iov_parse_ctx, (void *)fils,
		&fils_sub_cmds[0], (size_t)SUBCMD_TBL_SZ(fils_sub_cmds), NULL, 0);

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	memset(&cubby_params, 0, sizeof(cubby_params));

	cubby_params.context = fils;
	cubby_params.fn_init = wlc_fils_bsscfg_init;
	cubby_params.fn_deinit = wlc_fils_bsscfg_deinit;
	cubby_params.fn_get = wlc_fils_bsscfg_get;
	cubby_params.fn_set = wlc_fils_bsscfg_set;
	cubby_params.config_size = FILS_CUBBY_CFG_SIZE;

	fils->cfgh = wlc_bsscfg_cubby_reserve_ext(wlc,
		sizeof(wlc_fils_bsscfg_cubby_t *), &cubby_params);
	if (fils->cfgh < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	wlc->pub->cmn->_fils = TRUE;

#ifdef AP
	/* register FILS Indicator IE build callback fn */
	ret = wlc_iem_add_build_fn_mft(wlc->iemi,
			fils_ind_elm_build_fstbmp, DOT11_MNG_FILS_IND_ID,
			wlc_fils_fils_ind_elem_calc_len,
			wlc_fils_fils_ind_elem_build_fn, fils);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn_mft failed %d, \n",
			wlc->pub->unit, __FUNCTION__, ret));
		goto fail;
	}
#endif /* AP */

	/* register RSN IE calc/build/ callbacks */
	ret = wlc_iem_add_build_fn_mft(wlc->iemi,
			fils_auth_bld_fstbmp, DOT11_MNG_RSN_ID,
			wlc_fils_fils_ies_calc_len,
			wlc_fils_fils_ies_build_fn, fils);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn_mft failed %d, \n",
			wlc->pub->unit, __FUNCTION__, ret));
		goto fail;
	}

	/* register FILS HLP Container IE build/calc callback fn */
	ret = wlc_iem_add_build_fn_mft(wlc->iemi,
			fils_hlp_elm_build_fstbmp, DOT11_MNG_FILS_HLP_CONTAINER,
			wlc_fils_hlp_elem_calc_len,
			wlc_fils_hlp_elem_build_fn, fils);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn_mft failed %d, \n",
			wlc->pub->unit, __FUNCTION__, ret));
		goto fail;
	}

	return fils;

fail:
	if (fils) {
		MODULE_DETACH(fils, wlc_fils_detach);
	}

	return NULL;
}

void
BCMATTACHFN(wlc_fils_detach)(wlc_fils_info_t* fils)
{
	wlc_info_t *wlc;

	if (!fils)
		return;

	wlc = fils->wlc;

	if (obj_registry_unref(fils->wlc->objr, OBJR_FILS_CMN_INFO) == 0) {
		wlc_fils_cmn_t *fils_cmn = fils->fils_cmn;
		if (fils_cmn->fils_ind) {
			MFREE(wlc->osh, fils_cmn->fils_ind, fils_cmn->fils_ind_len);
		}

		if (fils_cmn->fils_hlp) {
			MFREE(wlc->osh, fils_cmn->fils_hlp, fils_cmn->fils_hlp_len);
		}

		MFREE(wlc->osh, fils_cmn, sizeof(*fils_cmn));
	}
	wlc->pub->cmn->_fils = FALSE;
	wlc_module_unregister(wlc->pub, "FILS", fils);
	MFREE(wlc->osh, fils, sizeof(*fils));
}

static void
wlc_fils_watchdog(void *ctx)
{

}

static int
wlc_fils_wlc_up(void *ctx)
{
	return BCME_OK;
}

static int
wlc_fils_wlc_down(void *ctx)
{
	return BCME_OK;
}

static int
wlc_fils_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg)
{
	int ret = BCME_OK;
	wlc_fils_info_t *fils = (wlc_fils_info_t *)ctx;
	wlc_fils_bsscfg_cubby_t *obc = NULL;
	wlc_fils_bsscfg_cubby_t **pobc = NULL;
	wlc_info_t *wlc;

	ASSERT(fils != NULL);

	wlc = cfg->wlc;
	pobc = FILS_BSSCFG_CUBBY_LOC(fils, cfg);
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
		wlc_fils_bsscfg_deinit(ctx, cfg);
	}
	return ret;
}

static void
wlc_fils_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_fils_info_t *fils = (wlc_fils_info_t *)ctx;
	wlc_fils_bsscfg_cubby_t *obc = NULL;
	wlc_fils_bsscfg_cubby_t **pobc = NULL;
	wlc_info_t *wlc;

	ASSERT(fils != NULL);

	wlc = cfg->wlc;

	wlc_fils_free_ies(wlc, cfg);

	pobc = FILS_BSSCFG_CUBBY_LOC(fils, cfg);
	obc = *pobc;
	if (obc != NULL) {
		MFREE(wlc->osh, obc, sizeof(*obc));
	}
	pobc = NULL;
	return;
}

/* bsscfg copy :get function */
static int
wlc_fils_bsscfg_get(void *ctx, wlc_bsscfg_t *cfg, uint8 *data, int *len)
{
	wlc_fils_info_t *fils = (wlc_fils_info_t *)ctx;
	wlc_fils_bsscfg_cubby_t *obc = NULL;

	if (data == NULL || *len < (int)FILS_CUBBY_CFG_SIZE) {
		*len  = FILS_CUBBY_CFG_SIZE;
		return BCME_BUFTOOSHORT;
	}
	obc = FILS_BSSCFG_CUBBY(fils, cfg);
	if (obc == NULL) {
		*len = 0;
		return BCME_OK;
	}
	memcpy(data, obc, FILS_CUBBY_CFG_SIZE);
	*len = FILS_CUBBY_CFG_SIZE;
	/* reset the data pointers to NULL so that wlc_fils_bsscfg_deinit()
	 * doesn't free up allocated data
	 */
	memset(obc, 0, sizeof(*obc));

	return BCME_OK;
}

/* bsscfg copy: set function */
static int
wlc_fils_bsscfg_set(void *ctx, wlc_bsscfg_t *cfg, const uint8 *data, int len)
{
	wlc_fils_info_t *fils = (wlc_fils_info_t *)ctx;
	wlc_fils_bsscfg_cubby_t *obc = NULL;
	obc = FILS_BSSCFG_CUBBY(fils, cfg);
	if (obc == NULL) {
		return BCME_OK;
	}
	if (data == NULL || len < (int)FILS_CUBBY_CFG_SIZE) {
		return BCME_BADARG;
	}
	memcpy(obc, data, FILS_CUBBY_CFG_SIZE);
	return BCME_OK;
}

#ifdef AP
static uint
wlc_fils_fils_ind_elem_calc_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_fils_info_t *fils = (wlc_fils_info_t *)ctx;
	wlc_fils_cmn_t *fils_cmn = fils->fils_cmn;

	if (fils_cmn->fils_ind == NULL)
		return 0;

	return fils_cmn->fils_ind_len;
}

static int
wlc_fils_fils_ind_elem_build_fn(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_fils_info_t *fils = (wlc_fils_info_t *)ctx;
	wlc_fils_cmn_t *fils_cmn = fils->fils_cmn;

	memcpy(data->buf, fils_cmn->fils_ind, fils_cmn->fils_ind_len);

	return BCME_OK;
}

static int
wlc_fils_write_fils_ind_ie(wlc_fils_info_t *fils, const void *arg, uint len)
{
	wlc_info_t *wlc = fils->wlc;
	wlc_fils_cmn_t *fils_cmn = fils->fils_cmn;

	if ((fils_cmn->fils_ind) && (fils_cmn->fils_ind->length != len)) {
		MFREE(wlc->osh, fils_cmn->fils_ind, fils_cmn->fils_ind_len);
		fils_cmn->fils_ind = NULL;
	}

	if (fils_cmn->fils_ind == NULL) {
		fils_cmn->fils_ind = MALLOCZ(wlc->osh, len+TLV_HDR_LEN);
		if (fils_cmn->fils_ind == NULL) {
			fils_cmn->fils_ind_len = 0;
			WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
				wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
			return BCME_NOMEM;
		}
	}

	fils_cmn->fils_ind_len = len + TLV_HDR_LEN;

	bcm_write_tlv(DOT11_MNG_FILS_IND_ID, arg, len, (uint8 *)fils_cmn->fils_ind);
	return BCME_OK;
}
#endif /* AP */

void
wlc_fils_free_ies(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	wlc_fils_info_t *fils = wlc->fils;
	wlc_fils_bsscfg_cubby_t *fbc;

	fbc = FILS_BSSCFG_CUBBY(fils, bsscfg);

	if (fbc->fils_ies_data) {
		MFREE(wlc->osh, fbc->fils_ies_data, fbc->fils_ies_data_len);
		fbc->fils_ies_data = NULL;
		fbc->fils_ies_data_len = 0;

		WL_INFORM(("wl%d: %s: free auth data\n",
		           wlc->pub->unit, __FUNCTION__));
	}
}

/* Parse and save authentication data from wpa_supplicant. Assume to
 * find each of FILS RSN IE, FILS Nonce, FILS Session and FILS Wrapped Data
 */
static int
wlc_fils_write_fils_auth_data(wlc_fils_info_t *fils,
	wlc_bsscfg_t *bsscfg, const void *arg, uint len)
{
	wlc_info_t *wlc = fils->wlc;
	wlc_fils_bsscfg_cubby_t *fbc;
	const uint8 *start_data;
	uint length = 0;

	start_data = (const uint8*)arg + DOT11_AUTH_SEQ_STATUS_LEN;
	length = len - DOT11_AUTH_SEQ_STATUS_LEN;

	fbc = FILS_BSSCFG_CUBBY(fils, bsscfg);

	/* allocate space for all auth data */
	if (fbc->fils_ies_data_len)
		MFREE(wlc->osh, fbc->fils_ies_data, fbc->fils_ies_data_len);

	fbc->fils_ies_data = MALLOCZ(wlc->osh, length);
	if (fbc->fils_ies_data == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return BCME_NOMEM;
	}

	memcpy(fbc->fils_ies_data, start_data, length);
	fbc->fils_ies_data_len = length;

	return BCME_OK;
}

static uint
wlc_fils_fils_ies_calc_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_fils_info_t *fils = (wlc_fils_info_t *)ctx;
	wlc_bsscfg_t *bsscfg = data->cfg;
	wlc_fils_bsscfg_cubby_t *fbc;

	fbc = FILS_BSSCFG_CUBBY(fils, bsscfg);

	return fbc->fils_ies_data_len;
}

static int
wlc_fils_fils_ies_build_fn(void *ctx, wlc_iem_build_data_t *data)
{
	uint8 *cp = data->buf;
	wlc_fils_info_t *fils = (wlc_fils_info_t *)ctx;
	wlc_bsscfg_t *bsscfg = data->cfg;
	wlc_fils_bsscfg_cubby_t *fbc;

	fbc = FILS_BSSCFG_CUBBY(fils, bsscfg);

	memcpy(cp, fbc->fils_ies_data, fbc->fils_ies_data_len);

	return BCME_OK;
}

static uint
wlc_fils_hlp_elem_calc_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_fils_info_t *fils = (wlc_fils_info_t *)ctx;

	return wlc_fils_get_hlp_elem_size(fils);
}

static int
wlc_fils_hlp_elem_build_fn(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_fils_info_t *fils = (wlc_fils_info_t *)ctx;
	wlc_info_t *wlc = fils->wlc;
	wlc_fils_cmn_t *fils_cmn = fils->fils_cmn;

	if (fils_cmn->fils_hlp && fils_cmn->fils_hlp_len) {
		memcpy(data->buf, fils_cmn->fils_hlp, fils_cmn->fils_hlp_len);

		MFREE(wlc->osh, fils_cmn->fils_hlp, fils_cmn->fils_hlp_len);
		fils_cmn->fils_hlp = NULL;
		fils_cmn->fils_hlp_len = 0;
	}

	return BCME_OK;
}

static uint16 wlc_fils_get_hlp_elem_size(wlc_fils_info_t *fils)
{
	wlc_fils_cmn_t *fils_cmn = fils->fils_cmn;

	ASSERT(fils);

	if (fils_cmn->fils_hlp == NULL)
		return 0;

	return fils_cmn->fils_hlp_len;
}

static int wlc_fils_write_hlp_ie(wlc_fils_info_t *fils, const void *arg, uint len)
{
	wlc_info_t *wlc = fils->wlc;
	wlc_fils_cmn_t *fils_cmn = fils->fils_cmn;

	if ((fils_cmn->fils_hlp) && (fils_cmn->fils_hlp->length != len)) {
		MFREE(wlc->osh, fils_cmn->fils_hlp, fils_cmn->fils_hlp_len);
		fils_cmn->fils_hlp = NULL;
	}

	if (fils_cmn->fils_hlp == NULL) {
		fils_cmn->fils_hlp = MALLOCZ(wlc->osh, len);
		if (fils_cmn->fils_hlp == NULL) {
			fils_cmn->fils_hlp_len = 0;
			WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
				wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
			return BCME_NOMEM;
		}
	}

	memcpy((uint8*)fils_cmn->fils_hlp, (const uint8*)arg, len);
	fils_cmn->fils_hlp_len = len;

	return BCME_OK;
}

/* validation function for IOVAR */
static int
wlc_fils_iov_cmd_validate(const bcm_iov_cmd_digest_t *dig, uint32 actionid,
	const uint8 *ibuf, size_t ilen, uint8 *obuf, size_t *olen)
{
	int ret = BCME_OK;
	wlc_info_t *wlc = NULL;
	wlc_fils_info_t *fils = NULL;

	ASSERT(dig);
	fils = (wlc_fils_info_t *)dig->cmd_ctx;
	ASSERT(fils);
	wlc = fils->wlc;

	UNUSED_PARAMETER(wlc);

	if (!FILS_ENAB(wlc->pub) &&
		(dig->cmd_info->cmd != WL_FILS_CMD_ADD_HLP_IE)) {
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
fils_iov_context_alloc(void *ctx, uint size)
{
	uint8 *iov_ctx = NULL;
	wlc_fils_info_t *fils = (wlc_fils_info_t *)ctx;

	ASSERT(fils != NULL);

	iov_ctx = MALLOCZ(fils->wlc->osh, size);
	if (iov_ctx == NULL) {
		WL_ERROR(("wl%d: %s: MALLOC failed, size %d malloced %d bytes\n",
			fils->wlc->pub->unit, __FUNCTION__, size, MALLOCED(fils->wlc->osh)));
	}

	return iov_ctx;
}

/* iovar context free */
static void
fils_iov_context_free(void *ctx, void *iov_ctx, uint size)
{
	wlc_fils_info_t *fils = (wlc_fils_info_t *)ctx;

	ASSERT(fils != NULL);
	if (iov_ctx) {
		MFREE(fils->wlc->osh, iov_ctx, size);
	}
}

/* command digest alloc function */
static int
BCMATTACHFN(fils_iov_get_digest_cb)(void *ctx, bcm_iov_cmd_digest_t **dig)
{
	wlc_fils_info_t *fils = (wlc_fils_info_t *)ctx;
	int ret = BCME_OK;
	uint8 *iov_cmd_dig = NULL;

	ASSERT(fils != NULL);
	iov_cmd_dig = MALLOCZ(fils->wlc->osh, sizeof(bcm_iov_cmd_digest_t));
	if (iov_cmd_dig == NULL) {
		WL_ERROR(("wl%d: %s: MALLOC failed, size %d malloced %d bytes\n",
			fils->wlc->pub->unit, __FUNCTION__,  sizeof(bcm_iov_cmd_digest_t), MALLOCED(fils->wlc->osh)));
		ret = BCME_NOMEM;
		goto fail;
	}
	*dig = (bcm_iov_cmd_digest_t *)iov_cmd_dig;
fail:
	return ret;
}

/* "wl fils add_hlp_ie <>" handler */
static int
wlc_fils_iov_add_hlp_ie(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen)
{
	int ret = BCME_OK;
	uint16 len;
	uint16 type;
	const bcm_xtlv_t *ptlv = (const bcm_xtlv_t *)ibuf;
	const uint8 *data;
	wlc_fils_info_t *fils = (wlc_fils_info_t *)dig->cmd_ctx;

	if (!ibuf || !ilen) {
		return BCME_BADARG;
	}

	bcm_xtlv_unpack_xtlv(ptlv, &type, &len, &data, BCM_XTLV_OPTION_ALIGN32);

	if (type != WL_FILS_XTLV_HLP_IE || !len) {
		WL_ERROR(("wl%d: %s: wrong xtlv: id %d len %d\n",
			fils->wlc->pub->unit, __FUNCTION__, type, len));

		return BCME_BADARG;
	}

	ret = wlc_fils_write_hlp_ie(fils, data, ptlv->len);

	return ret;
}

#ifdef AP
/* "wl fils add_ind_ie <>" handler */
static int
wlc_fils_iov_add_ind_ie(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen)
{
	int ret = BCME_OK;
	uint16 len;
	uint16 type;
	const bcm_xtlv_t *ptlv = (const bcm_xtlv_t *)ibuf;
	const uint8 *data;
	wlc_fils_info_t *fils = (wlc_fils_info_t *)dig->cmd_ctx;

	if (!ibuf || !ilen) {
		return BCME_BADARG;
	}

	bcm_xtlv_unpack_xtlv(ptlv, &type, &len, &data, BCM_XTLV_OPTION_ALIGN32);

	if (type != WL_FILS_XTLV_IND_IE || !len) {
		WL_ERROR(("wl%d: %s: wrong xtlv: id %d len %d\n",
			fils->wlc->pub->unit, __FUNCTION__, type, len));

		return BCME_BADARG;
	}

	ret = wlc_fils_write_fils_ind_ie(fils, data, ptlv->len);

	return ret;
}
#endif /* AP */

/* "wl fils add_auth_data <>" handler */
static int
wlc_fils_iov_add_auth_data(const bcm_iov_cmd_digest_t *dig, const uint8 *ibuf,
	size_t ilen, uint8 *obuf, size_t *olen)
{
	int ret = BCME_OK;
	uint16 len;
	uint16 type;
	const bcm_xtlv_t *ptlv = (const bcm_xtlv_t *)ibuf;
	const uint8 *data;
	wlc_fils_info_t *fils = (wlc_fils_info_t *)dig->cmd_ctx;

	if (!ibuf || !ilen) {
		return BCME_BADARG;
	}

	bcm_xtlv_unpack_xtlv(ptlv, &type, &len, &data, BCM_XTLV_OPTION_ALIGN32);

	if (type != WL_FILS_XTLV_AUTH_DATA || !len) {
		WL_ERROR(("wl%d: %s: wrong xtlv: id %d len %d\n",
			fils->wlc->pub->unit, __FUNCTION__, type, len));

		return BCME_BADARG;
	}

	ret = wlc_fils_write_fils_auth_data(fils, dig->bsscfg, data, ptlv->len);

	return ret;
}
