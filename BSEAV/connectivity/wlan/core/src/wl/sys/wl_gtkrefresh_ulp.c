/*
 * ulp functions plumb GTK key_info and restore it on ULP enter/exit
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 *
 */
#include <wlc_cfg.h>
#include <bcmendian.h>
#include <bcmwpa.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_bmac.h>
#include <wl_gtkrefresh.h>
#include <wl_gtkrefresh_ulp.h>
#include <wlc_ulp.h>
#include <ulp.h>
#include <wlc_scb.h>
#include <wlc_sup.h>
#include <wlc_sup_priv.h>

struct wlc_gtkoe_p1_cubby_info {
	uint8 eapol_encr_key[WPA_ENCR_KEY_LEN];
};
typedef struct wlc_gtkoe_p1_cubby_info wlc_gtkoe_p1_cubby_info_t;

struct wlc_gtkoe_p2_cubby_info {
	uint8 replay[EAPOL_KEY_REPLAY_LEN];
	uint8 eapol_mic_key[WPA_MIC_KEY_LEN];
};
typedef struct wlc_gtkoe_p2_cubby_info wlc_gtkoe_p2_cubby_info_t;

static int wlc_gtkoe_ulp_enter_cb(void *handle, ulp_ext_info_t *einfo, uint8 *cache_data);
static uint wlc_gtkoe_ulp_get_retention_size_cb(void *handle, ulp_ext_info_t *einfo);

static int wlc_gtkoe_ulp_recreate_cb(void * handle, ulp_ext_info_t * einfo, uint8 * cache_data,
	uint8 * p2_cache_data);
static void wlc_gtkoe_cache_ulp(void *ctx, wlc_bsscfg_t *cfg, bool cache,
	wlc_gtkoe_p1_cubby_info_t *wlc_gtkoe_p1_cubby_info,
	wlc_gtkoe_p2_cubby_info_t *wlc_gtkoe_p2_cubby_info);
static int wlc_gtkoe_p2_retrieve_cb(void *handle, osl_t *osh, uint8 *p2_cache_data);


static const ulp_p1_module_pubctx_t ulp_gtkoe_ctx = {
	MODCBFL_CTYPE_DYNAMIC,  /* because for non secure this will be 0 */
	wlc_gtkoe_ulp_enter_cb,
	NULL,
	wlc_gtkoe_ulp_get_retention_size_cb,
	wlc_gtkoe_ulp_recreate_cb,
	NULL
};

static const ulp_p2_module_pubctx_t gtkoe_p2_retrieve_reg_cb = {
	sizeof(wlc_gtkoe_p2_cubby_info_t),
	wlc_gtkoe_p2_retrieve_cb
};

int wlc_gtkoe_ulp_p1_register(wlc_sup_info_t *sup_info)
{
	if (BCME_OK != ulp_p1_module_register(ULP_MODULE_ID_WLC_SUP,
		&ulp_gtkoe_ctx, (void*)sup_info)) {
		return -1;
	}
	return 0;
}

static int
wlc_gtkoe_ulp_enter_cb(void *handle, ulp_ext_info_t *einfo, uint8 *cache_data)
{
	wlc_sup_info_t *sup_info = (wlc_sup_info_t *)handle;
	wlc_bsscfg_t *cfg;
	int err = BCME_OK;

	if (!sup_info) {
		WL_WSEC(("%s non secure., returning\n", __FUNCTION__));
		err = BCME_ERROR;
		goto done;
	}

	cfg = wlc_bsscfg_find_by_ID(sup_info->wlc, einfo->wowl_cfg_ID);
	if (!cfg) {
		WL_WSEC(("%s cfg NULL., returning\n", __FUNCTION__));
		err = BCME_ERROR;
		goto done;
	}

	if (!cache_data) {
		WL_WSEC(("%s no cachedat ., returning\n", __FUNCTION__));
		err = BCME_BADARG;
		goto done;
	}
	WL_WSEC(("%s: cache_shm: %p\n", __FUNCTION__,
		OSL_OBFUSCATE_BUF((wlc_sup_p1_cubby_info_t *)cache_data)));
	wlc_gtkoe_cache_ulp(sup_info, cfg, TRUE, (wlc_gtkoe_p1_cubby_info_t *)cache_data, NULL);

done:
	return err;
}

static uint
wlc_gtkoe_ulp_get_retention_size_cb(void *handle, ulp_ext_info_t *einfo)
{
	wlc_sup_info_t *sup_info = (wlc_sup_info_t *)handle;
	if (!sup_info) {
		WL_WSEC(("%s non secure., sz: 0 bytes\n", __FUNCTION__));
		return 0;
	}
	WL_WSEC(("%s: sz: %d\n", __FUNCTION__, (sizeof(wlc_gtkoe_p1_cubby_info_t))));
	return (sizeof(wlc_gtkoe_p1_cubby_info_t));
}

static int
wlc_gtkoe_ulp_recreate_cb(void *handle, ulp_ext_info_t *einfo, uint8 *cache_data,
	uint8 *p2_cache_data)
{
	wlc_sup_info_t *sup_info = (wlc_sup_info_t *)handle;
	wlc_bsscfg_t *cfg = NULL;
	int err = BCME_OK;
	if (!sup_info) {
		WL_WSEC(("%s non secure., returning\n", __FUNCTION__));
		err = BCME_ERROR;
		goto done;
	}
	cfg = wlc_bsscfg_find_by_ID(sup_info->wlc, einfo->wowl_cfg_ID);

	if (!cfg) {
		WL_WSEC(("%s cfg NULL., returning\n", __FUNCTION__));
		err = BCME_ERROR;
		goto done;
	}
	if (!cache_data) {
		WL_WSEC(("%s no cachedat ., returning\n", __FUNCTION__));
		err = BCME_BADARG;
		goto done;
	}
	wlc_gtkoe_cache_ulp(sup_info, cfg, FALSE,
		(wlc_gtkoe_p1_cubby_info_t *)cache_data,
		(wlc_gtkoe_p2_cubby_info_t *)p2_cache_data);
done:
	return err;
}

static void
wlc_gtkoe_cache_ulp(void *ctx, wlc_bsscfg_t *cfg, bool cache,
	wlc_gtkoe_p1_cubby_info_t *wlc_gtkoe_p1_cubby_info,
	wlc_gtkoe_p2_cubby_info_t *wlc_gtkoe_p2_cubby_info)
{
	wlc_sup_info_t *sup_info = (wlc_sup_info_t *)ctx;
	supplicant_t *sup = NULL;

	if (!sup_info || !cfg) {
		WL_WSEC(("%s sup_info or cfg is NULL., returning\n", __FUNCTION__));
		return;
	}

	/* this code requires caching of ONLY TKIP and AES params now. */
	if (!(WSEC_AES_ENABLED(cfg->wsec) || WSEC_TKIP_ENABLED(cfg->wsec))) {
		return;
	}
	/* ULP entry ...Cache the necessary sup data used after ULP exit */
	if (cache) {
		sup = SUP_BSSCFG_CUBBY(sup_info, cfg);
		memcpy(wlc_gtkoe_p1_cubby_info->eapol_encr_key,
			sup->wpa->eapol_encr_key, WPA_ENCR_KEY_LEN);
	}
	/* ULP exit ... Populate the sup structures with the data retrieved
	* from shmem after ULP exit
	*/
	else {
		if (wlc_gtk_init(sup_info, cfg) != BCME_OK) {
			return;
		}
		sup = SUP_BSSCFG_CUBBY(sup_info, cfg);
		memcpy(sup->wpa->eapol_encr_key,
			wlc_gtkoe_p1_cubby_info->eapol_encr_key, WPA_ENCR_KEY_LEN);
		memcpy(sup->wpa->eapol_mic_key,
			wlc_gtkoe_p2_cubby_info->eapol_mic_key, WPA_MIC_KEY_LEN);
		memcpy(sup->wpa->replay,
			wlc_gtkoe_p2_cubby_info->replay, EAPOL_KEY_REPLAY_LEN);
		memcpy(sup->wpa->last_replay,
			sup->wpa->replay, EAPOL_KEY_REPLAY_LEN);

		if (WSEC_AES_ENABLED(cfg->wsec)) {
			sup->wpa->ucipher = CRYPTO_ALGO_AES_CCM;
			sup->wpa->mcipher = CRYPTO_ALGO_AES_CCM;
		}
		else {
			sup->wpa->ucipher = CRYPTO_ALGO_TKIP;
			sup->wpa->mcipher = CRYPTO_ALGO_TKIP;
		}
		memcpy(&sup->peer_ea, &cfg->BSSID, ETHER_ADDR_LEN);
	}
}

int
wlc_gtkoe_p2_retrieve_cb(void *handle, osl_t *osh, uint8 *p2_cache_data)
{
	WL_WSEC(("%s enter\n", __FUNCTION__));
	wlc_hw_info_t *wlc_hw = (wlc_hw_info_t *)handle;
	wlc_gtkoe_p2_cubby_info_t *wlc_gtkoe_p2_cubby_info =
		(wlc_gtkoe_p2_cubby_info_t *)p2_cache_data;
	/* Retrieve replay counter */
	wlc_bmac_copyfrom_shm(wlc_hw, M_KEYRC_LAST(wlc_hw),
		wlc_gtkoe_p2_cubby_info->replay, EAPOL_KEY_REPLAY_LEN);
	/* Retrieve kck */
	wlc_bmac_copyfrom_shm(wlc_hw, M_KCK(wlc_hw),
		wlc_gtkoe_p2_cubby_info->eapol_mic_key, WPA_MIC_KEY_LEN);
	return BCME_OK;
}

int
wlc_gtkoe_preattach(void)
{
	int err = BCME_OK;
	WL_WSEC(("%s enter\n", __FUNCTION__));
	err = ulp_p2_module_register(ULP_MODULE_ID_WLC_SUP, &gtkoe_p2_retrieve_reg_cb);
	return err;
}
