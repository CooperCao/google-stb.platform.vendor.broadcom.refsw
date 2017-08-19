/**
 * @file
 * @brief
 * Keymanagement related ULP source
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
 * $Id: km_ulp.c 564097 2015-06-16 14:00:38Z $
 */

/**
 * @file
 * @brief
 * Contains all the routines to store and restore back security keys while alternating
 * between active and low power modes.
 */

#ifdef BCMULP
#include <km_pvt.h>
#include <ulp.h>
#include <km_hw_impl.h>
#include <bcmcrypto/tkhash.h>
#include <wlc_ulp.h>

/* address of key in shm for a given hw idx */
#define KM_ULP_HW_KEY_ADDR(_hw, _idx, _p2_handle) ((wlc_bmac_read_shm(_hw, \
			M_SECKEYS_PTR(_p2_handle)) << 1) + D11_MAX_KEY_SIZE * (_idx))

/* address of rx seq shm for given hw idx - non WAPI, non-MFP */
#define KM_ULP_HW_RX_SEQ_ADDR(_idx, _ins, _p2_handle) (M_TSCPN_BLK(_p2_handle) + \
	(KM_HW_RX_SEQ_BLOCK_SIZE * (_idx)) + ((_ins) * KEY_SEQ_SIZE))

/* address in shm for a tkip mic key */
#define KM_ULP_HW_TKIP_MIC_KEY_ADDR(_hw, _idx, _p2_handle) \
	((wlc_bmac_read_shm(_hw, M_TKMICKEYS_PTR(_p2_handle)) << 1) + \
		KM_HW_TKIP_MIC_KEY_SIZE * ((_idx) << 1))
#define KM_ULP_HW_TKIP_TX_MIC_KEY_ADDR(_hw, _idx, _p2_handle) \
	KM_ULP_HW_TKIP_MIC_KEY_ADDR(_hw, _idx, _p2_handle)
#define KM_ULP_HW_TKIP_RX_MIC_KEY_ADDR(_hw, _idx, _p2_handle) \
	(KM_ULP_HW_TKIP_MIC_KEY_ADDR(_hw, _idx, _p2_handle) + KM_HW_TKIP_MIC_KEY_SIZE)

struct ulp_iv {
uint8 buf[WOWL_TSCPN_SIZE];
};
typedef struct ulp_iv ulp_iv_t;

struct ulp_key_info {
	uint8		data[DOT11_MAX_KEY_SIZE];	/* key data */
	ulp_iv_t	txiv;				/* Tx IV */
	ulp_iv_t	rxiv[WLC_KEY_BASE_RX_SEQ];	/* Rx IV (one per TID) */
	bool retrieved;
};
typedef struct ulp_key_info ulp_key_info_t;

struct km_ulp_p2_cubby_info {
	ulp_key_info_t scb_key;
	ulp_key_info_t bss_keys[4];
	uint16 rot_key_idx;
	uint8 hw_algo;
};
typedef struct km_ulp_p2_cubby_info km_ulp_p2_cubby_info_t;

struct km_p1_cubby_info {
	wlc_key_id_t tx_key_id;
	km_bsscfg_flags_t flags;
};
typedef struct km_p1_cubby_info km_p1_cubby_info_t;

static uint km_ulp_get_retention_size_cb(void *handle, ulp_ext_info_t *einfo);
static int km_ulp_enter_cb(void *handle, ulp_ext_info_t *einfo, uint8 *cache_data);
static int km_ulp_recreate_cb(void *handle, ulp_ext_info_t *einfo, uint8 *cache_data,
	uint8 *p2_cache_data);
static void km_ulp_retrieve_fromshm(p2_handle_t *p2_handle,
	km_ulp_p2_cubby_info_t *km_ulp_p2_cubby_info);
static int km_ulp_init(km_p1_cubby_info_t* p1_cubby_info, km_ulp_p2_cubby_info_t *p2_cubby_info,
	wlc_bsscfg_t *bsscfg);
static int km_ulp_p2_retrieve_cb(void *handle, osl_t *osh, uint8 *p2_cache_data);

static const ulp_p2_module_pubctx_t km_p2_retrieve_reg_cb = {
	sizeof(km_ulp_p2_cubby_info_t),
	km_ulp_p2_retrieve_cb
};

const ulp_p1_module_pubctx_t km_ulp_ctx = {
	MODCBFL_CTYPE_DYNAMIC,
	km_ulp_enter_cb,
	NULL,
	km_ulp_get_retention_size_cb,
	km_ulp_recreate_cb,
	NULL
};
/* phase2 data retrieval callback */
static int
km_ulp_p2_retrieve_cb(void *handle, osl_t *osh, uint8 *p2_cache_data)
{
	p2_handle_t *p2_handle = (p2_handle_t *)handle;
	km_ulp_p2_cubby_info_t *km_ulp_p2_cubby_info = (km_ulp_p2_cubby_info_t *)p2_cache_data;
	int err = BCME_OK;
	if (!p2_cache_data) {
		err = BCME_ERROR;
		goto done;
	}
	km_ulp_retrieve_fromshm(p2_handle, km_ulp_p2_cubby_info);
done:
	KM_LOG(("%s completed with status %d\n", __FUNCTION__, err));
	return err;
}

/* phase1 enter callback */
static int
km_ulp_enter_cb(void *handle, ulp_ext_info_t *einfo, uint8 *cache_data)
{
	keymgmt_t *km = (keymgmt_t *)handle;
	wlc_bsscfg_t *cfg = wlc_bsscfg_find_by_ID(km->wlc, einfo->wowl_cfg_ID);
	km_p1_cubby_info_t *km_p1_cubby_info = (km_p1_cubby_info_t *)cache_data;
	int err = BCME_OK;
	if (!cache_data || !cfg) {
		err = BCME_ERROR;
		goto done;
	}
	km_p1_cubby_info->tx_key_id = wlc_keymgmt_get_bss_tx_key_id(km, cfg, FALSE);
	km_p1_cubby_info->flags = km_get_bsscfg_flags(km, cfg);
done:
	KM_LOG(("%s completed with status %d\n", __FUNCTION__, err));
	return err;

}

/* restore back the retrieved keys from shm into the key management structures */
static int
km_ulp_recreate_cb(void *handle, ulp_ext_info_t *einfo, uint8 *cache_data,
	uint8 *p2_cache_data)
{
	keymgmt_t *km = (keymgmt_t *)handle;
	wlc_bsscfg_t *cfg = wlc_bsscfg_find_by_ID(km->wlc, einfo->wowl_cfg_ID);
	int err = BCME_OK;
	if (!cfg) {
		err = BCME_ERROR;
		goto done;
	}
	if (!p2_cache_data) {
		err = BCME_ERROR;
		goto done;
	}
	err = km_ulp_init((km_p1_cubby_info_t *)cache_data,
		(km_ulp_p2_cubby_info_t *)p2_cache_data, cfg);
done:
	KM_LOG(("%s completed with status %d\n", __FUNCTION__, err));
	return err;
}

/* used on phase1 store/retrieval to return size of cubby required by this module */
static uint
km_ulp_get_retention_size_cb(void *handle, ulp_ext_info_t *einfo)
{
	KM_LOG(("%s:sz:%d\n", __FUNCTION__,
		(sizeof(km_p1_cubby_info_t))));
		return (sizeof(km_p1_cubby_info_t));
}

/* Retrieve the key management phase2 data from shm */
static void
km_ulp_retrieve_fromshm(p2_handle_t *p2_handle, km_ulp_p2_cubby_info_t *km_ulp_p2_cubby_info)
{
	int i = 0, j = 0;
	uint16 shm_addr;
	uint16 skl_val = 0;
	uint8 skl_idx = 0;
	wlc_hw_info_t *wlc_hw = (wlc_hw_info_t *)p2_handle->wlc_hw;

	/* Retrieve key rotation index */
	km_ulp_p2_cubby_info->rot_key_idx = wlc_bmac_read_shm(wlc_hw, M_GROUPKEY_UPBM(p2_handle));

	for (skl_idx = 0; skl_idx <= WLC_KEYMGMT_NUM_GROUP_KEYS; ++skl_idx) {
		skl_val = wlc_bmac_read_shm(wlc_hw, M_SECKINDXALGO_BLK(p2_handle) + (skl_idx << 1));
		km_ulp_p2_cubby_info->hw_algo = (skl_val >> SKL_ALGO_SHIFT) & SKL_ALGO_MASK;
		if (km_ulp_p2_cubby_info->hw_algo == WSEC_ALGO_OFF)
			continue;
		else
			break;
	}

	KM_LOG(("%s - start hw_algo = %d \n", __FUNCTION__, km_ulp_p2_cubby_info->hw_algo));

	switch (km_ulp_p2_cubby_info->hw_algo) {

		case WSEC_ALGO_WEP1:
		case WSEC_ALGO_WEP128:
			/* reading WEP key from shm.... WEP uses the same key */
			/* for both unicast and BC/MC */
			for (i = 0; i < WLC_KEYMGMT_NUM_GROUP_KEYS; i++) {
				skl_val = wlc_bmac_read_shm(wlc_hw,
					M_SECKINDXALGO_BLK(p2_handle) + (i << 1));
				if (((skl_val >> SKL_ALGO_SHIFT) & SKL_ALGO_MASK) == WSEC_ALGO_OFF)
					continue;
				else {
					wlc_bmac_copyfrom_objmem(wlc_hw,
						KM_ULP_HW_KEY_ADDR(wlc_hw, i, p2_handle),
						km_ulp_p2_cubby_info->bss_keys[i].data,
						D11_MAX_KEY_SIZE, OBJADDR_SHM_SEL);
					km_ulp_p2_cubby_info->bss_keys[i].retrieved = TRUE;
				}
			}
			break;
		case WSEC_ALGO_AES:
		case WSEC_ALGO_TKIP:
			/* reading AES scb key used for unicast traffic */
			wlc_bmac_copyfrom_objmem(wlc_hw,
				KM_ULP_HW_KEY_ADDR(wlc_hw, WLC_KEYMGMT_NUM_GROUP_KEYS, p2_handle),
				km_ulp_p2_cubby_info->scb_key.data, D11_MAX_KEY_SIZE,
				OBJADDR_SHM_SEL);
			/* reading seq */
			/* tx iv */
			shm_addr = M_CTX_GTKMSG2(p2_handle) +
				OFFSETOF(wowl_templ_ctxt_t, seciv) + TKHASH_P1_KEY_SIZE;
			wlc_bmac_copyfrom_objmem(wlc_hw, shm_addr,
				km_ulp_p2_cubby_info->scb_key.txiv.buf, KEY_SEQ_SIZE,
				OBJADDR_SHM_SEL);
			/* rx iv */
			for (i = 0; i < WLC_KEY_BASE_RX_SEQ; i++) {
				wlc_bmac_copyfrom_objmem(wlc_hw,
					KM_ULP_HW_RX_SEQ_ADDR(WLC_KEYMGMT_NUM_GROUP_KEYS, i,
						p2_handle),
					km_ulp_p2_cubby_info->scb_key.rxiv[i].buf, KEY_SEQ_SIZE,
					OBJADDR_SHM_SEL);
			}
			/* reading AES bss key used for BC/MCt traffic */
			for (i = 0; i < WLC_KEYMGMT_NUM_GROUP_KEYS; i++) {
				if (km_ulp_p2_cubby_info->rot_key_idx & (1 << i) ||
					(wlc_bmac_read_shm(wlc_hw,
					M_SECKINDXALGO_BLK(p2_handle) + (i << 1)))) {
					wlc_bmac_copyfrom_objmem(wlc_hw,
						KM_ULP_HW_KEY_ADDR(wlc_hw, i, p2_handle),
						km_ulp_p2_cubby_info->bss_keys[i].data,
						D11_MAX_KEY_SIZE, OBJADDR_SHM_SEL);
				}
				/* reading bss seq */
				/* tx iv */
				wlc_bmac_copyfrom_objmem(wlc_hw,
				shm_addr, km_ulp_p2_cubby_info->bss_keys[i].txiv.buf,
				KEY_SEQ_SIZE, OBJADDR_SHM_SEL);
				/* rx iv */
				for (j = 0; j < WLC_KEY_BASE_RX_SEQ; j++) {
					wlc_bmac_copyfrom_objmem(wlc_hw,
						KM_ULP_HW_RX_SEQ_ADDR(i, j, p2_handle),
						km_ulp_p2_cubby_info->bss_keys[i].rxiv[j].buf,
						KEY_SEQ_SIZE, OBJADDR_SHM_SEL);
				}
				km_ulp_p2_cubby_info->bss_keys[i].retrieved = TRUE;
			}
			if (km_ulp_p2_cubby_info->hw_algo == WSEC_ALGO_TKIP) {
				/* scb to ds key */
				wlc_bmac_copyfrom_objmem(wlc_hw,
					KM_ULP_HW_TKIP_TX_MIC_KEY_ADDR(wlc_hw,
					WLC_KEYMGMT_NUM_GROUP_KEYS, p2_handle),
					km_ulp_p2_cubby_info->scb_key.data +
					D11_MAX_KEY_SIZE + KM_HW_TKIP_MIC_KEY_SIZE,
					KM_HW_TKIP_MIC_KEY_SIZE, OBJADDR_SHM_SEL);
				/* scb from ds key */
				wlc_bmac_copyfrom_objmem(wlc_hw,
					KM_ULP_HW_TKIP_RX_MIC_KEY_ADDR(wlc_hw,
					WLC_KEYMGMT_NUM_GROUP_KEYS, p2_handle),
					km_ulp_p2_cubby_info->scb_key.data + D11_MAX_KEY_SIZE,
					KM_HW_TKIP_MIC_KEY_SIZE, OBJADDR_SHM_SEL);
				/* bss to ds and from ds keys */
				for (i = 0; i < WLC_KEYMGMT_NUM_GROUP_KEYS; i++) {
					if (km_ulp_p2_cubby_info->bss_keys[i].retrieved ==
						TRUE) {
						wlc_bmac_copyfrom_objmem(wlc_hw,
							KM_ULP_HW_TKIP_TX_MIC_KEY_ADDR(wlc_hw, i,
								p2_handle),
							km_ulp_p2_cubby_info->bss_keys[i].data +
							D11_MAX_KEY_SIZE + KM_HW_TKIP_MIC_KEY_SIZE,
							KM_HW_TKIP_MIC_KEY_SIZE, OBJADDR_SHM_SEL);
						wlc_bmac_copyfrom_objmem(wlc_hw,
							KM_ULP_HW_TKIP_RX_MIC_KEY_ADDR(wlc_hw, i,
								p2_handle),
							km_ulp_p2_cubby_info->bss_keys[i].data +
							D11_MAX_KEY_SIZE,
							KM_HW_TKIP_MIC_KEY_SIZE, OBJADDR_SHM_SEL);
					}
				}
			}
			break;
		case WSEC_ALGO_OFF:
			/* non security case */
			break;
		default:
			WL_ERROR(("Unknown algo type"));
			ASSERT(0);
			break;
		}

}

/* This function fills the key information on km_ulp_recreate_cb */
static int
km_ulp_init(km_p1_cubby_info_t* p1_cubby_info, km_ulp_p2_cubby_info_t *p2c,
	wlc_bsscfg_t *bsscfg)
{
	wlc_key_t *key = NULL;
	wlc_key_info_t key_info;
	wlc_info_t *wlc = bsscfg->wlc;
	keymgmt_t *km = wlc->keymgmt;
	int i = 0, j = 0;
	scb_t *scb;
	int err = BCME_OK;
	scb = wlc_scblookupband(wlc, bsscfg, &bsscfg->current_bss->BSSID,
		CHSPEC_WLCBANDUNIT(bsscfg->current_bss->chanspec));
	wlc_key_algo_t algo = km_hw_hw_algo_to_algo(km->hw, p2c->hw_algo);
	KM_LOG(("%s - start \n", __FUNCTION__));

	memset(&key_info, 0, sizeof(key_info));

	if (p2c->hw_algo == WSEC_ALGO_OFF) {
		goto done;
	}

	km_bsscfg_ulp_init(km, bsscfg, p1_cubby_info->flags);

	switch (algo) {
		case CRYPTO_ALGO_WEP1:
		case CRYPTO_ALGO_WEP128:
			/* Set the WEP key or bss key */
			for (i = 0; i < WLC_KEYMGMT_NUM_GROUP_KEYS; i++) {
				if (p2c->bss_keys[i].retrieved == TRUE) {
					key = wlc_keymgmt_get_bss_key(km, bsscfg, i, &key_info);

					if (key_info.key_idx == WLC_KEY_INDEX_INVALID) {
						err = BCME_BADKEYIDX;
						goto done;
					}

					err = wlc_key_set_data(key, algo,
					p2c->bss_keys[i].data, D11_MAX_KEY_SIZE);

					if (err != BCME_OK) {
						goto done;
					}
				}
			}
			wlc_keymgmt_set_bss_tx_key_id(km, bsscfg,
				p1_cubby_info->tx_key_id, FALSE);
			break;
		case CRYPTO_ALGO_AES_CCM:
		case CRYPTO_ALGO_TKIP:
			/* Get the scb key */
			key = wlc_keymgmt_get_scb_key(km, scb, WLC_KEY_ID_PAIRWISE,
				WLC_KEY_FLAG_NONE, &key_info);
			if (key_info.key_idx == WLC_KEY_INDEX_INVALID) {
				err = BCME_BADKEYIDX;
				goto done;
			}

			err = wlc_key_set_data(key, algo,
				p2c->scb_key.data, DOT11_MAX_KEY_SIZE);

			if (err != BCME_OK)
				goto done;

			/* setting scb key tx iv */
			err = wlc_key_set_seq(key, p2c->scb_key.txiv.buf,
				sizeof(p2c->scb_key.txiv.buf), 0, TRUE);
			if (err != BCME_OK)
				goto done;

			/* setting scb key rx iv */
			for (i = 0; i < WLC_KEY_BASE_RX_SEQ; ++i) {
				int err = wlc_key_set_seq(key,
					p2c->scb_key.rxiv[i].buf,
					sizeof(p2c->scb_key.rxiv[i].buf),
					(wlc_key_seq_id_t)i, FALSE);
				if (err != BCME_OK)
					goto done;
			}
			/* Group key */
			/* Get the bss key */
			for (i = 0; i < WLC_KEYMGMT_NUM_GROUP_KEYS; i++) {
				if (p2c->bss_keys[i].retrieved == TRUE) {
					key = wlc_keymgmt_get_bss_key(km, bsscfg, i, &key_info);
					if (key_info.key_idx == WLC_KEY_INDEX_INVALID) {
						err = BCME_BADKEYIDX;
						goto done;
					}

					err = wlc_key_set_data(key, algo,
						p2c->bss_keys[i].data, DOT11_MAX_KEY_SIZE);

					if (err != BCME_OK)
						goto done;

					/* setting bss key tx iv */
					err = wlc_key_set_seq(key,
						p2c->bss_keys[i].txiv.buf,
						sizeof(p2c->bss_keys[i].txiv.buf),
						0, TRUE);
					if (err != BCME_OK)
						goto done;

					/* setting bss key rx iv */
					for (j = 0; j < WLC_KEY_BASE_RX_SEQ; ++j) {
						int err = wlc_key_set_seq(key,
							p2c->bss_keys[i].rxiv[j].buf,
							sizeof(p2c->bss_keys[i].rxiv[j].buf),
							(wlc_key_seq_id_t)j, FALSE);
						if (err != BCME_OK)
							goto done;
					}
				}
			}
		case CRYPTO_ALGO_OFF:
			/* non security case */
			break;
		default:
			WL_ERROR(("Unknown algo type"));
			ASSERT(0);
			break;
	}
done:
	KM_LOG(("%s completed with status %d\n", __FUNCTION__, err));
	return err;

}

int
km_ulp_p1_module_register(keymgmt_t *km)
{
	return ulp_p1_module_register(ULP_MODULE_ID_KM, &km_ulp_ctx, km);
}

/* This function registers for phase2 data retrieval. */
int
km_ulp_preattach(void)
{
	int err = BCME_OK;
	KM_LOG(("%s enter\n", __FUNCTION__));
	err = ulp_p2_module_register(ULP_MODULE_ID_KM, &km_p2_retrieve_reg_cb);
	return err;
}
#endif /* BCMULP */
