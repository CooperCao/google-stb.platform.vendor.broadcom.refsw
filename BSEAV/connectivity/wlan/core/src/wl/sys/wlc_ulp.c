/**
 * @file
 * @brief
 * ULP Functionality module source
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

/**
 * @file
 * @brief
 * Contains all the routines to store and restore back software states while alternating
 * between active and low power modes.
 */



#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <siutils.h>
#include <wlioctl.h>
#include <bcmwpa.h>
#include <d11.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_bmac.h>
#include <wlc_bsscfg.h>
#include <wlc_scb.h>
#include <wlc_assoc.h>
#include <wlc_ulp.h>
#include <ulp.h>
#include <hndpmu.h>
#include <wl_export.h>
#include <wlc_mcnx.h>
#include <wlc_pm.h>
#include <wlc_event.h>
#include <hndd11.h>
#include <wlc_iocv.h>
#ifdef ULP_DUMP
#include <wlc_dump.h>
#endif /* ULP_DUMP */
#include <sbgci.h>
#include <wlc_hw_priv.h>

/* iovars */
enum {
	IOV_ULP_WLC_UP = 1,
	IOV_ULP_MAC_FEATURES = 2,
	IOV_ULP = 3,
	IOV_ULP_WAIT = 4,
	IOV_ULP_ALGO = 5,
	IOV_HUDI = 6,
	IOV_ULP_WAKEIND = 7,
	IOV_ULP_SDIOCTRL = 8,
	IOV_LAST
};

/* iovar table */
static const bcm_iovar_t ulp_iovars[] = {
	{"ulp_wlc_up", IOV_ULP_WLC_UP, (0), 0, IOVT_INT32, 0},
	{"ulp_mac_features", IOV_ULP_MAC_FEATURES, (0), 0, IOVT_INT32, 0},
	{"ulp", IOV_ULP, (0), 0, IOVT_BOOL, 0},
	{"hudi", IOV_HUDI, (0), 0, IOVT_BOOL, 0},
	{"ulp_wait", IOV_ULP_WAIT, (0), 0, IOVT_INT32, 0},
	{"ulp_algo", IOV_ULP_ALGO, (0), 0, IOVT_UINT32, 0},
	{"ulp_wakeind", IOV_ULP_WAKEIND, (0), 0, IOVT_UINT32, 0},
	{"ulp_sdioctrl", IOV_ULP_SDIOCTRL, (0), 0, IOVT_BUFFER, sizeof(ulp_shm_info_t)},
	{NULL, 0, 0, 0, 0, 0}
};

/* module private states */
#define ULP_AID_MASK			0xC000
#define ULP_FEATURES_SIZE		2

/* Wake conditions for which we need to do assoc recreate */
#define WOWL_WAKEIND_PROCESS_MASK	(WL_WOWL_MAGIC | WL_WOWL_BCAST |\
					WL_WOWL_DIS | WL_WOWL_NET | WL_WOWL_TST | \
					WL_WOWL_ULP_BAILOUT | WL_WOWL_BCN)
#define ULP_WAKEIND_PROCESS_MASK	(C_HOST_WAKEUP | C_HUDI_DS1_EXIT)

typedef enum {
	ULP_ALGO_INVALID = 0,
	ULP_ALGO_TIMER = 1,
	ULP_ALGO_MAX = 2
} wlc_ulp_algo_t;

/* external iovar interface for ulp mode. used for wlc_ulp_params_t->ulp_mode */
typedef enum {
	ULP_MODE_DISABLE = 0,
	ULP_MODE_ENABLE = 1
} wlc_ulp_dngl_ulp_mode_t;

/* internal ulp state based on wlc_ulp_dngl_ulp_mode_t set by user.
 * Used for wlc_ulp_info_t->ulp_state
 */
typedef enum {
	DNGL_ULP_STATE_IDLE = 0,
	DNGL_ULP_STATE_COUNTING = 1,
	DNGL_ULP_STATE_ENTERING = 2,
	DNGL_ULP_STATE_ASSOC_RECR_PENDING = 3
} wlc_ulp_dngl_ulp_state_t;

/* Main structure part for phase1 store/retrieve. In addition to this following is also saved
 * as part of p1 cache.
 *	- wlc_ulp_params_t
 *
 */
typedef struct wlc_ulp_cache_p1 {
	int	wowl_cfg_ID;			/* idx of the cfg going to wowl mode */
	uint16	ssid_len;			/* length of the ssid */
	uint8	SSID[DOT11_MAX_SSID_LEN];	/* SSID */
	uint8	wlfc_flags;			/* tlv flags from wlc */
	int32	sup_wpa;			/* sup_wpa */
	uint8	PM;				/* PM of the cfg going into wowl */
} wlc_ulp_cache_p1_t;

/* structure for phase2 store/retrieve [the fields of] which is shared between
 * wowl ucode and driver.
 */
typedef struct wlc_ulp_cache_p2 {
	uint32 wowl_wakeind;		/* DS1 wake indication */
	uint16 ulp_corerev;		/* corerev */
	uint16 chanspec;		/* chanspec of the assoc */
	uint16	aid;			/* aid of the association */
	uint8 *wake_data;		/* pkt which caused DS1 exit */
	uint32 retain_wowl_flags;	/* wowl flags to be retained on DS1 exit */
	uint8 arp_hostipaddr[4];	/* tbd for ipv6? */
	uint16 seq_num;			/* scb sequence num */
	uint32 ulp_wakeind;		/* ds1 wakeind */
	uint32 ulp_rxbcnmbss;		/* ds1 rxbeaconmbss */
	uint32 ulp_rxbcnloss;		/* ds1 rxbeacon loss */
} wlc_ulp_cache_p2_t;

#ifdef ULP_DUMP
/* ULP switch time statistics */
typedef enum {
	ULP_ENTER = 0,
	ULP_EXIT = 1
} wlc_ulp_modes_t;

typedef enum {
	ULP_NA = 0,
	ULP_HOSTWAKEUP = 1,
	ULP_HUDIEXIT = 2,
	ULP_UCASTMAGICPACKET = 3,
	ULP_BCASTMAGICPACKET = 4,
	ULP_NETPATTERN = 5,
	ULP_LOSSOFBEACON = 6,
	ULP_DISASSOC = 7,
	ULP_WAKEUP_TST = 8,
	ULP_UNKNOWNPACKET = 9
} wlc_ulp_exitreason_t;

static const char *ulpexit_reason[] = {
	"N/A\t",
	"HOSTWAKEUP",
	"HUDIEXIT",
	"UCASTMGPKT",
	"BCASTMGPKT",
	"NET PATTERN",
	"LOSSOFBCN",
	"DISASSOC",
	"WAKEUP TST",
	"UNKNWN PKT",
};

/* This is to make sure latest stats are displayed in the last column fo DS1 statistics print */
#define DS1_TIMECAL_LOOP(b, str, val, cdt, val1, val2, val3) \
	bcm_bprintf(b, str); \
	val1 = cdt->index; \
	val3 = ULPSWITCH_TIMES_HISTORY; \
	for (val2 = 0; val2 < 2; val2++, val3 = val1, val1 = 0) \
		for (val = val1; val < val3 && cdt->entry[val].starttime; val++)

#endif /* ULP_DUMP */

#define ULPSWITCH_TIMES_HISTORY	8
typedef struct ulp_switchtimes {
	struct {
		uint8 wakereason;
		uint32 starttime;
		uint32 endtime;
		uint32 rxbcnmbss;
		uint32 rxbcnloss;
		uint32 ulp_wait;
		uint32 DS0time;
	} entry[ULPSWITCH_TIMES_HISTORY];
	uint32 index;
	uint32 totalDS1Count;
} ulp_switchtimes_t;

typedef struct wlc_ulp_params {
	uint32 ulp_mode;	/* ready enabled by host */
	int ulp_wait;		/* time in millisec to elapse before we goto ulp mode once
				 * everything is ready [eg: NO tx activity, etc]. Note that this is
				 * now in watchdog context and ticks only if ulp_mode is ULP_READY.
				 */
	bool pm_set_by_ulp;	/* PM mode is set by ulp iovar. used to reset PM mode when user
				 * invokes "wl ulp 0"
				 */
	uint32 ulp_algo;	/* ulp algorithm */
	bool hudi;		/* HUDI (Host Ucode DS1 Interface) */
	uint32 ulp_wakeind;	/* ULP wake indication reason */
	ulp_switchtimes_t ulp_swtime;
	uint32 ds0time;
} wlc_ulp_params_t;

/* structure to store any ucode related data [shm/ihr/etc] while in ds0 and
 * applied after switching to ds1. ref: wlc_ulp_pre_DS1UC_switch
 */
typedef struct wlc_ulp_ds0_dat {
	uint16 ilp_per_h; /* ilp high */
	uint16 ilp_per_l; /* ilp low */
} wlc_ulp_ds0_dat_t;

/* info structure for this module */
struct wlc_ulp_info {
	wlc_info_t *wlc;
	osl_t *osh;
	wlc_hw_info_t *wlc_hw;
	wlc_ulp_cache_p1_t *cp1;	/* p1 cubby ptr */
	wlc_ulp_cache_p2_t *cp2;	/* p2 cubby ptr */
	wlc_ulp_ds0_dat_t *ds0_dat;	/* ptr to data while in ds0 and applied in ds1 */
	uint32 user_ulp_features;	/* M_ULP_FEATURES */
	uint32 ulp_features;
	wlc_ulp_params_t *ulp_params;	/* ulp params */
	struct wl_timer *ulp_timer;
	wlc_ulp_dngl_ulp_state_t ulp_state;
	struct wlc_if *wlcif;	/* IF for which ulp is enabled */
};

static uint wlc_ulp_get_retention_size_cb(void *handle, ulp_ext_info_t *einfo);
static int wlc_ulp_enter_pre_ulpucode_cb(void *handle, ulp_ext_info_t *einfo, uint8 *cache_data);
static int wlc_ulp_enter_post_ulpucode_cb(void *handle, ulp_ext_info_t *einfo);
static int wlc_ulp_exit_cb(void *handle, uint8 *cache_data, uint8 *p2_cache_data);
static int wlc_ulp_recreate_cb(void *handle, ulp_ext_info_t *einfo,
	uint8 *cache_data, uint8 *p2_cache_data);
static int wlc_ulp_p2_retrieve_cb(void *handle, osl_t *osh, uint8 *p2_cache_data);
static int wlc_ulp_retrieve_wakeup_causing_pkt_fromshm(p2_handle_t *p2_handle,
	osl_t *osh, wlc_ulp_cache_p2_t *cp2);
static int wlc_ulp_restore_wowl_settings(wlc_ulp_info_t *ulp_info,
	wlc_bsscfg_t *bsscfg, wlc_ulp_cache_p2_t *cp2);
static void wlc_ulp_retrieve_wowl_settings(osl_t *osh, p2_handle_t *p2_handle,
	wlc_ulp_cache_p2_t *cp2);
static int wlc_ulp_recreate_assoc(wlc_ulp_info_t *ulp_info, struct scb *scb);
static int wlc_ulp_save_ds0_shms(wlc_ulp_info_t *ulp_info);
static void wlc_ulp_timeout(void *arg);
static void wlc_ulp_dngl_set_ulp_state(wlc_ulp_info_t *ulp_info, wlc_ulp_dngl_ulp_state_t state);
static wlc_ulp_dngl_ulp_state_t wlc_ulp_dngl_get_ulp_state(wlc_ulp_info_t *ulp_info);
static void wlc_ulp_as_upd_cb(void *ctx, bss_assoc_state_data_t *notif_data);

static int wlc_ulp_entry_timer_start(wlc_ulp_info_t *ulp_info);
static int wlc_ulp_entry_timer_stop(wlc_ulp_info_t *ulp_info);
static void wlc_ulp_shm_info(wlc_ulp_info_t *ui, ulp_shm_info_t *ulp_shm);
#ifdef ULP_DUMP
static int wlc_ulp_switchtime_update(wlc_ulp_info_t *ulp_info, wlc_ulp_params_t *upa,
	wlc_ulp_cache_p2_t *cp2, wlc_ulp_modes_t ulpmode);
static int wlc_ulp_dumpstats(void *ctx, struct bcmstrbuf *b);
static int wlc_ulp_dumpstats_clr(void *ctx);
static int wlc_ulp_fcbsstats(void *ctx, struct bcmstrbuf *b);
static void wlc_ulp_fcbsstats_readblock(wlc_ulp_info_t *ulp_info,
	struct bcmstrbuf *b, uint32 offset);
#endif /* ULP_DUMP */
static const char BCMATTACHDATA(rstr_ulp)[] = "ulp";

/* p1 context for phase1 store/retrieve/restore. This is used to register to
 * the ulp baselevel [ulp.c/h] for phase1. In phase1 storage, the
 * values/iovar's gets stored in shm separate from wowl/ulp-ucode-shm space
 * and dedicated to the driver, [called "driver-cache-shm"] which is NOT used
 * by wowl/ulp-ucode.
 */
static const ulp_p1_module_pubctx_t wlc_ulp_p1_ctx = {
	MODCBFL_CTYPE_DYNAMIC,
	wlc_ulp_enter_pre_ulpucode_cb,
	wlc_ulp_exit_cb,
	wlc_ulp_get_retention_size_cb,
	wlc_ulp_recreate_cb,
	wlc_ulp_enter_post_ulpucode_cb
};

/* p2 context for phase2 store/retrieve/restore.  This is used to register to
 * the ulp baselevel [ulp.c/h] for phase2. In phase2, the values retrieved are
 * shared between the wowl/ulp-ucode and driver. So we need to retrieve them
 * from the shared shm's.
 */
static const ulp_p2_module_pubctx_t p2_retrieve_reg_cb = {
	sizeof(wlc_ulp_cache_p2_t),
	wlc_ulp_p2_retrieve_cb
};

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/* module entries */
static int
wlc_ulp_doiovar(void *ctx, uint32 actionid,
	void *params, uint p_len, void *arg, uint len, uint val_size, struct wlc_if *wlcif)
{
	wlc_ulp_info_t *ulp_info = (wlc_ulp_info_t *)ctx;
	int err = BCME_OK;
	int32 int_val = 0;
	int32 *ret_int_ptr = (int32 *)arg;
	wlc_bsscfg_t *bsscfg = NULL;
	wlc_ulp_params_t *upa = ulp_info->ulp_params;
	bool bool_val;

	WL_ULP_DBG(("%s: %d\n", __FUNCTION__, actionid));

	bsscfg = wlc_bsscfg_find_by_wlcif(ulp_info->wlc, wlcif);
	ASSERT(bsscfg != NULL);
	if (p_len >= (int)sizeof(int_val))
		memcpy(&int_val, params, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;

	switch (actionid) {
		case IOV_SVAL(IOV_ULP_WLC_UP) :
			if ((err = wlc_ioctl(ulp_info->wlc, WLC_UP, NULL, 0, NULL)))
				WL_ERROR(("%s: IOV_ULP_WLC_UP failed: err: %d\n",
					__FUNCTION__, err));
			break;

		case IOV_SVAL(IOV_ULP_MAC_FEATURES) :
			/* check if all user requested features are present in chip. */
			if ((int_val & ulp_mac_ulp_features_support(ulp_info->wlc->pub->sih))
					!= int_val) {
				err = BCME_UNSUPPORTED;
				goto done;
			}
			ulp_info->user_ulp_features = int_val;
			break;

		case IOV_GVAL(IOV_ULP_MAC_FEATURES) :
			*ret_int_ptr = ulp_info->user_ulp_features;
			break;

		case IOV_SVAL(IOV_ULP_WAKEIND):
			ulp_info->ulp_params->ulp_wakeind = int_val;
			break;

		case IOV_GVAL(IOV_ULP_WAKEIND):
			*ret_int_ptr = ulp_info->ulp_params->ulp_wakeind;
			break;

		case IOV_GVAL(IOV_ULP_SDIOCTRL):
			wlc_ulp_shm_info(ulp_info, (ulp_shm_info_t *)arg);
			break;

		case IOV_SVAL(IOV_ULP) :
		{
			int pm = PM_MAX;
			/* check if ulp is already enabled for any interface. If so, prevent */
			if ((bool_val == (bool)ULP_MODE_ENABLE) &&
					(upa->ulp_mode == ULP_MODE_ENABLE)) {
				WL_ERROR(("Error: dup enabling ulp/for multi IF's prohibited!\n"));
				err = BCME_USAGE_ERROR;
				goto done;
			}

			/* check if ulp is enabled for any interface. If so, disable */
			if ((bool_val == (bool)ULP_MODE_DISABLE) &&
					(upa->ulp_mode == ULP_MODE_ENABLE)) {
				/* for ULP_ALGO_TIMER, stop timer and update mode */
				if (upa->ulp_algo == ULP_ALGO_TIMER) {
					wlc_ulp_entry_timer_stop(ulp_info);
				}
				upa->ulp_mode = (uint32)ULP_MODE_DISABLE;
				ulp_info->wlcif = NULL;
				if (upa->pm_set_by_ulp == TRUE) {
					/* move PM to OFF and reset */
					upa->pm_set_by_ulp = FALSE;
					pm = PM_OFF;
					wlc_ioctl(ulp_info->wlc, WLC_SET_PM, (void *)&pm,
						sizeof(pm), bsscfg->wlcif);
				}
				goto done;
			}

			if (bool_val == (bool)ULP_MODE_ENABLE) {
				/* prevent ulp mode entry if ulp_wait is <= 0 */
				if (upa->ulp_wait <= 0) {
					WL_ERROR(("Error: ulp mode needs ulp_wait which is "
						"null!\n"));
					err = BCME_BADOPTION;
					goto done;
				}
				ulp_info->wlcif = wlcif;
				upa->ulp_mode = (uint32)ULP_MODE_ENABLE;
				/* check if cfg is in any PM. if not set to PM 1 */
				if (wlc_get_pm_state(bsscfg) == PM_OFF) {
					WL_ULP_DBG(("%s: forcing PM\n", __FUNCTION__));
					/* force PM1 & timer will be started after ps_ctrl */
					wlc_ioctl(ulp_info->wlc, WLC_SET_PM, (void *)&pm,
						sizeof(pm), bsscfg->wlcif);
					upa->pm_set_by_ulp = TRUE;
				} else {
					/* if this request comes when we are already in PM1/2,
					 * we need to restart timer straightaway. Otherwise,
					 * timer will be started just before
					 * actual entering into PM1/2 state by wlc_pm2_enter_ps
					 */
					WL_ULP_DBG(("%s:val %d, pmen:%d\n", __FUNCTION__, bool_val,
						bsscfg->pm->PMenabled));
					/* start timer */
					wlc_ulp_perform_sleep_ctrl_action(ulp_info,
						ULP_TIMER_START);
				}
			}
		}
			break;

		case IOV_GVAL(IOV_ULP) :
			*ret_int_ptr = ulp_info->ulp_params->ulp_mode;
			break;

		case IOV_SVAL(IOV_ULP_WAIT) :
			ulp_info->ulp_params->ulp_wait = int_val;
			break;
		case IOV_GVAL(IOV_ULP_WAIT) :
			*ret_int_ptr = ulp_info->ulp_params->ulp_wait;
			break;

		case IOV_SVAL(IOV_HUDI) :
			ulp_info->ulp_params->hudi = bool_val;
			break;

		case IOV_SVAL(IOV_ULP_ALGO) :
			if (((uint32)int_val <= ULP_ALGO_INVALID) &&
					((uint32)int_val >= ULP_ALGO_MAX)) {
				err = BCME_USAGE_ERROR;
				goto done;
			}
			ulp_info->ulp_params->ulp_algo = (uint32)int_val;
			break;
		case IOV_GVAL(IOV_ULP_ALGO) :
			*ret_int_ptr = ulp_info->ulp_params->ulp_algo;
			break;
		default:
			ASSERT(0);
	}
done:
	return err;
}

/* This function attaches the ULP module */
wlc_ulp_info_t *
BCMATTACHFN(wlc_ulp_attach)(wlc_info_t *wlc)
{
	int err = 0;
	wlc_ulp_info_t *ulp_info;

	if ((ulp_info = MALLOCZ(wlc->osh, sizeof(*ulp_info) + sizeof(wlc_ulp_params_t))) == NULL) {
		WL_ERROR(("wl: %s: out of mem, malloced %d bytes\n",
			__FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}
	ulp_info->osh = wlc->osh;

	/* Point to wlc ->ulp pointer */
	wlc->ulp = ulp_info;

	/* ulp params are allocate along with ulp_info and placed at its end */
	ulp_info->ulp_params = (wlc_ulp_params_t *)(ulp_info + 1);

	/* default algorithm */
	ulp_info->ulp_params->ulp_algo = ULP_ALGO_TIMER;

	/* Enable HUDI by default */
	ulp_info->ulp_params->hudi = TRUE;

	/* register wlc module */
	if ((err = wlc_module_register(wlc->pub, ulp_iovars, rstr_ulp, wlc->ulp,
			wlc_ulp_doiovar, NULL, NULL, NULL)) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	ulp_info->wlc = wlc;

	ulp_info->ulp_timer = wl_init_timer(wlc->wl, wlc_ulp_timeout, ulp_info, "wlc_ulp_tmo");

	if (ulp_info->ulp_timer == NULL) {
		WL_ERROR(("wl%d: %s: wl_init_timer failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	wlc->pub->_ulp = TRUE;

	if ((err = ulp_p1_module_register(ULP_MODULE_ID_WLC_ULP,
			&wlc_ulp_p1_ctx, (void*)ulp_info)) != BCME_OK) {
		WL_ERROR(("%s: ulp_p1_module_register failed\n", __FUNCTION__));
		goto fail;
	}
	/* register assoc state notification callback */
	if (wlc_bss_assoc_state_register(wlc, wlc_ulp_as_upd_cb, ulp_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bss_assoc_state_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		ASSERT(0);
		goto fail;
	}
#ifdef ULP_DUMP
	wlc_dump_add_fns(wlc->pub, "ulpstats", (dump_fn_t)wlc_ulp_dumpstats,
		wlc_ulp_dumpstats_clr, (void *)ulp_info);
	wlc_dump_add_fns(wlc->pub, "fcbsstats", (dump_fn_t)wlc_ulp_fcbsstats,
		NULL, (void *)ulp_info);
	/* Read LHL timer reg during bootup and use the same to calculate DS0 time */
	ulp_info->ulp_params->ds0time = LHL_REG(ulp_info->wlc->pub->sih, lhl_hibtim_adr, 0, 0)/32;
#endif /* ULP_DUMP */
	return ulp_info;
fail:
	MODULE_DETACH(ulp_info, wlc_ulp_detach);
	return NULL;
}

void
BCMATTACHFN(wlc_ulp_detach)(wlc_ulp_info_t *ulp_info)
{
	wlc_info_t *wlc;

	if (ulp_info == NULL)
		return;

	wlc = ulp_info->wlc;
	wl_free_timer(wlc->wl, ulp_info->ulp_timer);
	wlc_module_unregister(wlc->pub, rstr_ulp, ulp_info);
	MFREE(wlc->osh, ulp_info, sizeof(wlc_ulp_info_t));

	return;
}

/* This function registers for phase2 data retrieval. */
int
wlc_ulp_preattach(void)
{
	int err = BCME_OK;
	ULP_DBG(("%s enter\n", __FUNCTION__));
	err = ulp_p2_module_register(ULP_MODULE_ID_WLC_ULP, &p2_retrieve_reg_cb);
	return err;
}

/* phase2 data retrieval callback */
int
wlc_ulp_p2_retrieve_cb(void *handle, osl_t *osh, uint8 *p2_cache_data)
{
	wlc_ulp_cache_p2_t *cp2 =
		(wlc_ulp_cache_p2_t *)p2_cache_data;
	p2_handle_t *p2_handle = (p2_handle_t *)handle;
	wlc_hw_info_t *wlc_hw = (wlc_hw_info_t *)p2_handle->wlc_hw;

	ULP_DBG(("%s enter\n", __FUNCTION__));
	WL_ULP_DBG(("%s: enter: p2cd:%p\n", __FUNCTION__, OSL_OBFUSCATE_BUF(p2_cache_data)));

	/* Read corerev from SHM */
	cp2->ulp_corerev = wlc_bmac_read_shm(wlc_hw, M_MACHW_VER(p2_handle));

	/* Read Wake event from SHM */
	cp2->wowl_wakeind = (uint32)wlc_bmac_read_shm(wlc_hw, M_WAKEEVENT_IND(p2_handle));

	cp2->ulp_wakeind = (uint32)wlc_bmac_read_shm(wlc_hw, M_ULP_WAKE_IND(p2_handle));

	/* Read MBSS Beacons received during ULP mode */
	cp2->ulp_rxbcnmbss = (uint32)wlc_bmac_read_shm(wlc_hw, M_RXBEACONMBSS_CNT(p2_handle));

	/* Read no of Beacons lost count during ULP mode */
	cp2->ulp_rxbcnloss = (uint32)wlc_bmac_read_shm(wlc_hw, M_FRRUN_LBCN_CNT(p2_handle));

	WL_ULP_DBG(("%s - start corerev = %d wowl_wakeind = %x ulp_wakeind = %x\n",
			__FUNCTION__, cp2->ulp_corerev, cp2->wowl_wakeind, cp2->ulp_wakeind));

	if ((cp2->wowl_wakeind & WOWL_WAKEIND_PROCESS_MASK) ||
			(cp2->ulp_wakeind & ULP_WAKEIND_PROCESS_MASK)) {
		WL_ULP_DBG(("%s - wakeind match\n", __FUNCTION__));

		/* Retrieve current channel */
		cp2->chanspec = wlc_bmac_read_shm(wlc_hw, M_CURCHANNEL(p2_handle));
		/* Retrieve SCB sequence number */
		wlc_bmac_copyfrom_objmem(wlc_hw,
			M_CTX_GTKMSG2(p2_handle) +
			OFFSETOF(wowl_templ_ctxt_t, seqnum),
			&cp2->seq_num, sizeof(cp2->seq_num),
			OBJADDR_SHM_SEL);

		cp2->aid = wlc_bmac_read_shm(wlc_hw, M_AID_NBIT(p2_handle));
		/* retrieve data which caused wakeup */
		(void) wlc_ulp_retrieve_wakeup_causing_pkt_fromshm(p2_handle, osh, cp2);

		/* retrieve iovar data set into SHM for wowl to input to wowl_iovar later */
		wlc_ulp_retrieve_wowl_settings(osh, p2_handle, cp2);
	}
	return BCME_OK;
}

/* used on phase1 store/retrieval to return size of cubby required by this module */
static uint
wlc_ulp_get_retention_size_cb(void *handle, ulp_ext_info_t *einfo)
{
	ULP_DBG(("%s:sz:%d\n", __FUNCTION__,
		(sizeof(wlc_ulp_cache_p1_t) + sizeof(wlc_ulp_params_t))));
	return (sizeof(wlc_ulp_cache_p1_t) + sizeof(wlc_ulp_params_t));
}

/* ulp enter callback before switching to ulp ucode */
static int
wlc_ulp_enter_pre_ulpucode_cb(void *handle, ulp_ext_info_t *einfo, uint8 *cache_data)
{
	wlc_ulp_info_t *ulp_info = (wlc_ulp_info_t *)handle;
	wlc_ulp_cache_p1_t *cp1 = (wlc_ulp_cache_p1_t *)cache_data;
	wlc_ulp_params_t *upa = (wlc_ulp_params_t *)(cp1 + 1);
	wlc_bsscfg_t *cfg = wlc_bsscfg_find_by_ID(ulp_info->wlc, einfo->wowl_cfg_ID);
	int err = BCME_OK;

	if (!cfg) {
		err = BCME_ERROR;
		ULP_ERR(("%s: NULL cfg pointer %d\n", __FUNCTION__, err));
		goto done;
	}

	BCM_REFERENCE(ulp_info);
	ULP_DBG(("%s: cd: %p, cache_shm: %p\n",
		__FUNCTION__, OSL_OBFUSCATE_BUF(cache_data), OSL_OBFUSCATE_BUF(cp1)));

	cp1->ssid_len = (uint16)cfg->SSID_len;
	/* Writing SSID */
	memset(cp1->SSID, 0, DOT11_MAX_SSID_LEN);
	memcpy(cp1->SSID, cfg->SSID, cfg->SSID_len);

	cp1->wowl_cfg_ID = cfg->ID;
	cp1->wlfc_flags = ulp_info->wlc->wlfc_flags;
	cp1->PM = cfg->pm->PM;

	upa->ulp_mode = ulp_info->ulp_params->ulp_mode;
	upa->ulp_wait = ulp_info->ulp_params->ulp_wait;
	upa->pm_set_by_ulp = ulp_info->ulp_params->pm_set_by_ulp;
#ifdef ULP_DUMP
	if ((err = wlc_ulp_switchtime_update(ulp_info, upa, NULL, ULP_ENTER)) != BCME_OK) {
		ULP_ERR(("%s: wlc_ulp_switchtime_update failed! %d\n", __FUNCTION__, err));
	}
#endif /* ULP_DUMP */
	/* store shm values used by ds0 ucode and transferred to ds1 ucode
	 * This block is kept intentionally here before wl_down. Saved shm's will be
	 * applied in ulp ucode in wlc_ulp_enter_post_ulpucode_cb()
	 */
	err = wlc_ulp_save_ds0_shms(ulp_info);

	if (err) {
		ULP_ERR(("%s: wlc_ulp_save_ds0_shms failed! %d\n", __FUNCTION__, err));
		goto done;
	}

done:
	ULP_DBG(("REC: infoptr:%p, finalcfg:%d\n",
		OSL_OBFUSCATE_BUF(cp1), cp1->wowl_cfg_ID));

#ifdef ULP_DBG_ON
	prhex("enter: wlc_ulp", (uchar *)cp1, wlc_ulp_get_retention_size_cb(ulp_info, NULL));
#endif /* ULP_DBG_ON */

	return err;
}

/* ulp enter callback after switching to ulp ucode
 * see documentation of wlc_ulp_save_DS0_shms also
 */
static int
wlc_ulp_enter_post_ulpucode_cb(void *handle, ulp_ext_info_t *einfo)
{
	wlc_ulp_info_t *ulp_info = (wlc_ulp_info_t *)handle;
	wlc_ulp_ds0_dat_t *ds0_dat = ulp_info->ds0_dat;
	uint32 ilp_period;
	uint16 data;
	int err = BCME_OK;

	ULP_DBG(("%s enter\n", __FUNCTION__));
	if (ds0_dat) {
		if (ulp_info->user_ulp_features & C_ULP_SLOWCAL_SKIP) {
#ifdef BCMQT
				/* ILP low and high values are populated as a result of slow cal
				 * and slow cal takes very long time in QT. So, hardcoded the ILP
				 * low and high values for 32KHz clock. Used ds1calskip tcl proc
				 * to get these values.
				 */
				ds0_dat->ilp_per_l = 0x4000;
				ds0_dat->ilp_per_h = 0x001F;
#endif /* BCMQT */
			if ((ds0_dat->ilp_per_l  != 0) && (ds0_dat->ilp_per_h  != 0)) {
				/* slowcal related shm's */
				wlc_bmac_write_shm(ulp_info->wlc->hw,
					M_ILP_PER_H(ulp_info->wlc), ds0_dat->ilp_per_h);
				wlc_bmac_write_shm(ulp_info->wlc->hw,
					M_ILP_PER_L(ulp_info->wlc), ds0_dat->ilp_per_l);
				ilp_period = ((ds0_dat->ilp_per_h << 16) | ds0_dat->ilp_per_l);
				WL_ULP_DBG(("%s:save ulp ilp_prd ilp_per_h = %x ilp_per_l = %x\n",
					__FUNCTION__, ds0_dat->ilp_per_h, ds0_dat->ilp_per_l));
				si_pmu_ulp_ilp_config(ulp_info->wlc->pub->sih,
					ulp_info->wlc->osh, ilp_period);
				wlc_ulp_configure_ulp_features(ulp_info, C_ULP_SLOWCAL_SKIP, TRUE);
			}
		}

		if (ulp_info->ulp_params->hudi) {
			wlc_ulp_configure_ulp_features(ulp_info, C_HUDI_ENABLE, TRUE);

			/* Tell ucode which gpio is used for SDIO CMD */
			data = (1 << ULP_SDIO_CMD_PIN);
			hndd11_write_shm(ulp_info->wlc->pub->sih, 0,
					M_DS1_CTRL_SDIO_PIN(ulp_info->wlc), &data);
		}

		/* write ulp features shm */
		if (ulp_info->ulp_features) {
			/* Currently only the lower 16 bits of ulp_info->ulp_features is used */
			ulp_write_driver_shmblk(ulp_info->wlc->pub->sih, 0,
				M_ULP_FEATURES_OFFSET(ulp_info->wlc),
				&ulp_info->ulp_features, ULP_FEATURES_SIZE);
		}

		/* free the dat */
		MFREE(ulp_info->osh, ds0_dat, sizeof(*ds0_dat));
		ulp_info->ds0_dat = NULL;
	}

	return err;
}

/* ulp exit callbk called in the context of wlc_ulp_attach(), when
 * ulp_p1_module_register is called and condition is warmboot
 */
static int
wlc_ulp_exit_cb(void *handle, uint8 *cache_data, uint8 *p2_cache_data)
{
	wlc_ulp_info_t *ulp_info = (wlc_ulp_info_t *)handle;
	ULP_DBG(("%s: h:%p, cd:%p, p2cd:%p\n",
		__FUNCTION__, OSL_OBFUSCATE_BUF(ulp_info),
		OSL_OBFUSCATE_BUF(cache_data),
		OSL_OBFUSCATE_BUF(p2_cache_data)));

	/* assoc recreate will take care of rest. donot free the pointer. */
	ulp_info->cp1 = (wlc_ulp_cache_p1_t *)cache_data;
	ulp_info->cp2 = (wlc_ulp_cache_p2_t *) p2_cache_data;

	return BCME_OK;
}

/* restore callbk called in the context of assoc recreate() and condition is
 * warmboot
 */
static int
wlc_ulp_recreate_cb(void *handle, ulp_ext_info_t *einfo, uint8 *cache_data,
	uint8 *p2_cache_data)
{
	wlc_ulp_info_t *ulp_info = (wlc_ulp_info_t *)handle;
	wlc_bsscfg_t *cfg = wlc_bsscfg_find_by_ID(ulp_info->wlc, einfo->wowl_cfg_ID);
	wlc_info_t *wlc = ulp_info->wlc;
	struct scb *scb;
	/* note that, we can use same as cache_data and p2_cache_data, but
	 * they are already stored in cp1/cp2 resptly
	 */
	wlc_ulp_cache_p2_t *cp2 = ulp_info->cp2;
	wlc_ulp_cache_p1_t *cp1 = ulp_info->cp1;
	wlc_ulp_params_t *upa = (wlc_ulp_params_t *)(cp1 + 1);
	int err = BCME_OK;

	WL_ULP_DBG(("%s enter: einfo: %p, cfg: %p\n", __FUNCTION__,
		OSL_OBFUSCATE_BUF(einfo), OSL_OBFUSCATE_BUF(cfg)));
	if (!cfg) {
		ULP_DBG(("%s cfg NULL., returning\n", __FUNCTION__));
		err = BCME_ERROR;
		goto done;
	}

	/* restore BSSID and SSID */
	cfg->SSID_len = cp1->ssid_len;
	memcpy(cfg->SSID, cp1->SSID, DOT11_MAX_SSID_LEN);
	cfg->current_bss->SSID_len = cfg->SSID_len;
	memcpy(&cfg->current_bss->SSID, &cfg->SSID, DOT11_MAX_SSID_LEN);

	/* restore AID */
	cfg->AID = (cp2->aid | ULP_AID_MASK);
	cfg->current_bss->bss_type = DOT11_BSSTYPE_INFRASTRUCTURE;
	cfg->BSS = TRUE;
	wlc->pub->_assoc_recreate = TRUE;

	/* restore other flags */
	wlc->wlfc_flags = cp1->wlfc_flags;
	ulp_info->ulp_params->ulp_mode = upa->ulp_mode;
	ulp_info->ulp_params->ulp_wait = upa->ulp_wait;
	ulp_info->ulp_params->pm_set_by_ulp = upa->pm_set_by_ulp;
	ulp_info->wlcif = cfg->wlcif;
#ifdef ULP_DUMP
	if ((err = wlc_ulp_switchtime_update(ulp_info, upa, cp2, ULP_EXIT)) != BCME_OK) {
		ULP_ERR(("%s: wlc_ulp_switchtime_update failed! %d\n", __FUNCTION__, err));
	}
#endif /* ULP_DUMP */
	/* restore chanspec */
	cfg->current_bss->chanspec = cp2->chanspec;
	memcpy(cfg->target_bss, cfg->current_bss, sizeof(wlc_bss_info_t));

	/* scb lookup */
	scb = wlc_scblookupband(wlc, cfg, &cfg->current_bss->BSSID,
		CHSPEC_WLCBANDUNIT(cfg->current_bss->chanspec));

	ulp_info->ulp_state = DNGL_ULP_STATE_ASSOC_RECR_PENDING;
	if (scb != NULL) {
		/* re-establish connection */
		wlc_ulp_recreate_assoc(ulp_info, scb);
	}
	else {
		err = BCME_ERROR;
		WL_ERROR(("%s:SCB is NULL.. assoc recreate failure!\n", __FUNCTION__));
		goto done;
	}
	/* process data which caused wakeup */
	if (cp2->wake_data) {
#ifdef WLC_ULP_DBG_ON
		prhex("retrdat", PKTDATA(ulp_info->osh, cp2->wake_data),
			PKTLEN(ulp_info->osh, cp2->wake_data));
#endif
		wlc_recv(wlc, cp2->wake_data);
	}
	wlc_bsscfg_enable(wlc, cfg);

	/* restore wowl settings back */
	err = wlc_ulp_restore_wowl_settings(ulp_info, cfg, cp2);

done:
	return err;
}

static int
wlc_ulp_recreate_assoc(wlc_ulp_info_t *ulp_info, struct scb *scb)
{
	int err = BCME_OK;
	wlc_bsscfg_t *cfg = scb->bsscfg;
	wlc_info_t *wlc = ulp_info->wlc;
	wlc_ulp_cache_p2_t *cp2 = ulp_info->cp2;
	scb->state = AUTHENTICATED|ASSOCIATED|AUTHORIZED;
	/* TBD retreive/restore the flag */
	scb->flags |= SCB_MYAP | SCB_WMECAP | SCB_HTCAP;
	wlc_sta_assoc_upd(cfg, TRUE);
	cfg->assoc->type = AS_RECREATE;
	scb->WPA_auth = cfg->WPA_auth;
	/* assoc recreate */
	wlc_join_recreate(wlc, cfg);
	/* SCB sequence number */
	SCB_SEQNUM(scb, PRIO_8021D_BE) = cp2->seq_num + 1;

	return err;
}

/* This function is called by wowl module and passes it on to ulp base
 * to start ulp_enter callbk and saving the data.
 * NOTE: this function is called BEFORE wl_down and BEFORE switching to DS1 ucode.
*/
int
wlc_ulp_pre_ulpucode_switch(wlc_ulp_info_t *ulp_info, wlc_bsscfg_t *cfg)
{
	int err = BCME_OK;
	ulp_ext_info_t *einfo = NULL;
	if ((einfo = MALLOCZ(ulp_info->osh, sizeof(ulp_ext_info_t))) == NULL) {
		WL_ERROR(("wl: %s: out of mem, malloced %d bytes\n",
			__FUNCTION__, MALLOCED(ulp_info->wlc->osh)));
		err = BCME_NOMEM;
		goto done;
	}
	einfo->wowl_cfg_ID = cfg->ID;

	/* collect data to be stored in shm/bm and store. Note that this does't transfer
	 * data to shm or bm.
	 */
	err = ulp_enter_pre_ulpucode_switch(einfo);

	if (err) {
		WL_ERROR(("wl: %s: error in p1_enter! %d\n",
			__FUNCTION__, err));
		goto done;
	}

done:
	if (einfo) {
		ULP_DBG(("%s: freeing entry: %p\n",
			__FUNCTION__, OSL_OBFUSCATE_BUF(einfo)));
		MFREE(ulp_info->osh, einfo, sizeof(ulp_ext_info_t));
	}
	return err;
}

#ifdef ULP_EVENT
/* This function is for triggering an event to host on complete DS1 entry */
static void
wlc_ulp_post_ulpdone_event(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	wl_ulp_event_t ulp_evt;

	ulp_evt.ulp_dongle_action = WL_ULP_DISABLE_CONSOLE;
	ulp_evt.version = WL_ULP_EVENT_VERSION;
	wlc_bss_mac_event(wlc, cfg, WLC_E_ULP, NULL, 0, 0, 0,
		&ulp_evt, sizeof(wl_ulp_event_t));
}
#endif /* ULP_EVENT */

/* This function is called by wowl module and passes it on to ulp base
 * to start really going to ULP mode by calling ulp_enter callbk and transferring the data
 * stored by wlc_ulp_pre_DS1UC_switch to SHM/BM.
 * NOTE: this function is called AFTER switching to DS1 ucode.
*/
int
wlc_ulp_post_ulpucode_switch(wlc_ulp_info_t *ulp_info, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = ulp_info->wlc;
	ulp_ext_info_t *einfo = NULL;
	int err = BCME_OK;
	uint16 host_flags = 0;

	if ((einfo = MALLOCZ(ulp_info->osh, sizeof(ulp_ext_info_t))) == NULL) {
		WL_ERROR(("wl: %s: out of mem, malloced %d bytes\n",
			__FUNCTION__, MALLOCED(ulp_info->wlc->osh)));
		err = BCME_NOMEM;
		goto done;
	}
	einfo->wowl_cfg_ID = cfg->ID;

	ulp_enter(wlc->pub->sih, wlc->osh, einfo);

	hndd11_read_shm(wlc->pub->sih, 0, M_HOST_FLAGS2(wlc), &host_flags);
	host_flags |= MHF2_HIB_FEATURE_ENABLE;
	hndd11_write_shm(wlc->pub->sih, 0, M_HOST_FLAGS2(wlc), &host_flags);

#ifdef ULP_EVENT
	/* Trigger host event indicating complete DS1 entry */
	wlc_ulp_post_ulpdone_event(wlc, cfg);
#endif /* ULP_EVENT */

	if (einfo) {
		ULP_DBG(("%s: freeing entry: %p\n",
			__FUNCTION__, OSL_OBFUSCATE_BUF(einfo)));
		MFREE(ulp_info->osh, einfo, sizeof(ulp_ext_info_t));
	}
done:
	return err;
}

/* This function is used to do assoc recreate
*  This will be called during wl up functionality.
*/
int
wlc_ulp_assoc_recreate(wlc_ulp_info_t *ulp_info)
{
	int err = BCME_OK;
	wlc_ulp_cache_p1_t *cp1 = ulp_info->cp1;
	ulp_ext_info_t *einfo = NULL;
	ULP_DBG(("%s enter\n", __FUNCTION__));

	if (!cp1) {
		WL_ERROR(("%s: No valid cp1\n",	__FUNCTION__));
		err = BCME_ERROR;
		goto error;
	}
	if (cp1->wowl_cfg_ID == ULP_CFG_IDX_INVALID) {
		WL_ERROR(("%s: No valid bsscfg\n", __FUNCTION__));
		err = BCME_NOTASSOCIATED;
		goto error;
	}

	if ((einfo = MALLOCZ(ulp_info->osh, sizeof(ulp_ext_info_t))) == NULL) {
		WL_ERROR(("wl: %s: out of mem, malloced %d bytes\n",
			__FUNCTION__, MALLOCED(ulp_info->wlc->osh)));
		err = BCME_NOMEM;
		goto error;
	}

	/* note that, we are NOT attempting to store bsscfg pointer here because, it would
	 * not have initialized [esp for p2p case] until recreate_cb for bsscfg is called in
	 * the context of ulp_recreate() below.
	 */
	einfo->wowl_cfg_ID = cp1->wowl_cfg_ID;

	if ((err = ulp_recreate(einfo)) != BCME_OK) {
		WL_ERROR(("%s: err %d returned by ulp_recreate, but invoking cleanup!\n",
			__FUNCTION__, err));
		/* no "goto error" here because anyway cleanup is required */
	}
error:
	if (einfo) {
		ULP_DBG(("%s: freeing einfo: %p\n",
			__FUNCTION__, OSL_OBFUSCATE_BUF(einfo)));
		MFREE(ulp_info->osh, einfo, sizeof(ulp_ext_info_t));
	}
	return err;
}

/* retrieve data which caused wakeup */
static int
wlc_ulp_retrieve_wakeup_causing_pkt_fromshm(p2_handle_t *p2_handle,
	osl_t *osh,
	wlc_ulp_cache_p2_t *cp2)
{
	int err = BCME_OK;
	wlc_hw_info_t *wlc_hw = (wlc_hw_info_t *)p2_handle->wlc_hw;
	uint32 retrlen = wlc_bmac_read_shm(wlc_hw, M_RXCNT(wlc_hw));
	uint8 *pdat = NULL;
	uint8 *wake_data = NULL;
	wlc_info_t *wlc = wlc_hw->wlc;

	ULP_DBG(("%s: enter sz: %d\n", __FUNCTION__, retrlen));

	if (!retrlen)
		goto done;

	wake_data = PKTGET(osh,
		LBUFSZ +
		wlc->hwrxoff + /* note that this could be sizeof(*pwhdr),
				* which is 39 bytes, but wlc_recv pulls WL_HWRXOFF
				* which is 40 bytes
				*/
		retrlen,
		TRUE);

	if (wake_data == NULL) {
		WL_ERROR(("%s:malloc failure in retreive\n", __FUNCTION__));
		err = BCME_ERROR;
		goto done;
	}
	pdat = PKTDATA(osh, wake_data);

	/* get header */
	wlc_bmac_copyfrom_objmem(wlc_hw,
		M_CRX_BLK(p2_handle),
		pdat, wlc->hwrxoff,
		OBJADDR_SHM_SEL);

	/* fill any missing data in header. Currently nothing */

	/* get data */
	wlc_bmac_copyfrom_objmem(wlc_hw,
		M_RXFRM_BASE_ADDR(p2_handle),
		pdat + wlc->hwrxoff, retrlen,
		OBJADDR_SHM_SEL);

	PKTSETLEN(osh, wake_data, retrlen + wlc->hwrxoff);
#ifdef WLC_ULP_DBG_ON
	prhex("retrdat-retr", pdat, retrlen);
#endif
	/* now assoc recreate will do the rest */
	cp2->wake_data = wake_data;

done:
	return err;
}

/* restore wowl settings back */
static int
wlc_ulp_restore_wowl_settings(wlc_ulp_info_t *ulp_info,
	wlc_bsscfg_t *bsscfg,
	wlc_ulp_cache_p2_t *cp2)
{
	int err = BCME_OK;
	wlc_info_t *wlc = ulp_info->wlc;

	WL_ULP_DBG(("%s: RESTORING FLAGS: 0x%x\n", __FUNCTION__,
		cp2->retain_wowl_flags));

	if (cp2->retain_wowl_flags) {
		err = wlc_iovar_op(wlc, "wowl", NULL, 0,
			&cp2->retain_wowl_flags,
			sizeof(cp2->retain_wowl_flags),
			IOV_SET, bsscfg->wlcif);
		if (err) {
			WL_ERROR(("%s: wowl iovar restore failed %d!\n",
				__FUNCTION__, err));
			goto done;
		}
		err = wlc_iovar_op(wlc, "wowl_os", NULL, 0,
			&cp2->retain_wowl_flags,
			sizeof(cp2->retain_wowl_flags),
			IOV_SET, bsscfg->wlcif);

		if (err) {
			WL_ERROR(("%s: wowl_os iovar restore failed %d!\n",
				__FUNCTION__, err));
			goto done;
		}

		err = wlc_iovar_op(wlc, "wowl_wakeind", NULL, 0, &cp2->wowl_wakeind,
				sizeof(cp2->wowl_wakeind), IOV_SET, bsscfg->wlcif);

		err = wlc_iovar_op(wlc, "ulp_wakeind", NULL, 0, &cp2->ulp_wakeind,
				sizeof(cp2->ulp_wakeind), IOV_SET, bsscfg->wlcif);

		if (err) {
			WL_ERROR(("%s: wowl_wakeind iovar restore failed %d!\n",
				__FUNCTION__, err));
			goto done;
		}
		if (cp2->retain_wowl_flags & WL_WOWL_ARPOFFLOAD) {
			/* set ip addr */
			WL_ULP_DBG(("RESTR:ip:%d:%d:%d:%d\n",
				cp2->arp_hostipaddr[0],
				cp2->arp_hostipaddr[1],
				cp2->arp_hostipaddr[2],
				cp2->arp_hostipaddr[3]));
			err = wlc_iovar_op(wlc, "wowl_arp_hostip", NULL, 0,
				cp2->arp_hostipaddr, 4,
				IOV_SET, bsscfg->wlcif);
			if (err) {
				WL_ERROR(("%s: wowl_arp_hostip iovar restore failed %d!\n",
					__FUNCTION__, err));
				goto done;
			}
		}
	}
done:
	return err;
}

/* before going to wowl, this is called to see the readiness to move to wowl. */
int
wlc_ulp_wowl_check(wlc_ulp_info_t *ulp_info, uint16 id)
{
	int err = BCME_OK;
	ulp_ext_info_t *einfo = NULL;
	uint csz = 0;

	ULP_DBG(("%s enter\n", __FUNCTION__));

	if ((einfo = MALLOCZ(ulp_info->osh, sizeof(ulp_ext_info_t))) == NULL) {
		WL_ERROR(("wl: %s: out of mem, malloced %d bytes\n",
			__FUNCTION__, MALLOCED(ulp_info->wlc->osh)));
		err = BCME_NOMEM;
		goto done;
	}
	einfo->wowl_cfg_ID = id;

	csz = ulp_p1_get_max_box_size(einfo);
	/* if there is no space avilable for what we want to store
	 * then error and return
	 */
	if (csz > M_WOWL_ULP_SW_DAT_BLK_MAX_SZ) {
		WL_ERROR(("%s:Error,reqd_size:%d,avail:%d\n", __FUNCTION__,
			csz, M_WOWL_ULP_SW_DAT_BLK_MAX_SZ));
		err = BCME_NORESOURCE;
		goto done;
	}
done:
	if (einfo) {
		ULP_DBG(("%s: freeing einfo: %p\n",
			__FUNCTION__, OSL_OBFUSCATE_BUF(einfo)));
		MFREE(ulp_info->osh, einfo, sizeof(ulp_ext_info_t));
	}
	return err;
}

static void
wlc_ulp_retrieve_wowl_settings(osl_t *osh, p2_handle_t *p2_handle,
	wlc_ulp_cache_p2_t *cp2)
{
	wlc_hw_info_t *wlc_hw = (wlc_hw_info_t *)p2_handle->wlc_hw;
	uint32 wowl_flags = wlc_bmac_read_shm(wlc_hw, M_HOST_WOWLBM(p2_handle));
	uint32 pattern_count = wlc_bmac_read_shm(wlc_hw, M_NETPAT_NUM(p2_handle));
	uint32 offload_cfg_ptr = wlc_bmac_read_shm(wlc_hw, M_WOWL_OFFLOADCFG_PTR(p2_handle)) * 2;
	uint32 shm_pat = wlc_bmac_read_shm(wlc_hw, M_NETPAT_BLK_PTR(p2_handle)) * 2;
	uint32 arp_pattern_idx = 0xffff;
	uint8 *arp_hostipaddr = cp2->arp_hostipaddr;
	uint16 ipaddroffset = 0;
	uint16 ip1 = 0;
	uint32 patsize = 0;
	int i = 0;

	if (wowl_flags & WL_WOWL_NET) {
		WL_ULP_DBG(("retr:offsetofIdx:0x%x\n",
			offload_cfg_ptr + M_NPAT_ARPIDX_OFFSET));
		arp_pattern_idx = wlc_bmac_read_shm(wlc_hw,
			offload_cfg_ptr + M_NPAT_ARPIDX_OFFSET);

		/* TBD: at present ONLY arp pattern supported to retreive */
		if (arp_pattern_idx != 0xffff) {
			WL_ULP_DBG(("patix: %d\n", arp_pattern_idx));
			/* means arp offload enabled */
			for (i = 0; i < pattern_count; i++) {
				/* TBD: skip - this is supported for arp only for now */
				if (i != arp_pattern_idx) {
					/* skip offset, patternsize, masksz, patsz */
					shm_pat += (sizeof(uint16) * 2) +
						MAXMASKSIZE + MAXPATTERNSIZE;
					continue;
				}

				WL_ULP_DBG(("found patix: %d, shmpat: %x\n", i, shm_pat));
				/* skip offset */
				shm_pat += sizeof(uint16);
				patsize = wlc_bmac_read_shm(wlc_hw, shm_pat);
				/* skip patternsize, masksz */
				shm_pat += sizeof(uint16) + MAXMASKSIZE;
				/* ipaddress is at the last 4 bytes of pattern */
				ipaddroffset = shm_pat +
					patsize - /* patsz */
					IPV4_ADDR_LEN;
				/* need to figure out if ipaddr is in odd bounday shm */
				if (ipaddroffset % 2 == 0) {
					*((uint16*)arp_hostipaddr) =
						wlc_bmac_read_shm(wlc_hw, ipaddroffset);
					*((uint16*)(arp_hostipaddr + 2)) =
						wlc_bmac_read_shm(wlc_hw, ipaddroffset + 2);
				} else {
					ip1 = wlc_bmac_read_shm(wlc_hw, ipaddroffset - 1);
					arp_hostipaddr[0] = (uint8)(ip1 >> 8);
					ip1 = wlc_bmac_read_shm(wlc_hw, ipaddroffset + 1);
					arp_hostipaddr[1] = (uint8)ip1;
					arp_hostipaddr[2] = (uint8)(ip1 >> 8);
					ip1 = wlc_bmac_read_shm(wlc_hw, ipaddroffset + 3);
					arp_hostipaddr[3] = (uint8)ip1;
				}
				WL_ULP_DBG(("RETR1: ip: %d:%d:%d:%d\n",
					cp2->arp_hostipaddr[0],
					cp2->arp_hostipaddr[1],
					cp2->arp_hostipaddr[2],
					cp2->arp_hostipaddr[3]));
			}
		}
	}

	/* TBD keepalive retreival */

	/* TBD	ipv6 NS */
	WL_ULP_DBG(("%s: retain_wowl_flags: 0x%x\n", __FUNCTION__, wowl_flags));

	cp2->retain_wowl_flags = wowl_flags;
}

/* functionality done before ucode switch to DS1. This will be useful in the below scenario
 *	a) There is a shm used by DS0 ucode which is also used by DS1 ucode in
 *		same or different address
 *	b) This function is called when ds0 ucode is executing.
 *	c) wlc_ulp_enter_post_ulpucode_cb is called after ulp_enter which is after
 *		switching to ds1 ucode.
 */
static int
wlc_ulp_save_ds0_shms(wlc_ulp_info_t *ulp_info)
{
	int err = BCME_OK;
	wlc_ulp_ds0_dat_t *ds0_dat = NULL;
	uint32 default_mac_ulp_feat = 0;
	if ((ds0_dat = MALLOCZ(ulp_info->osh, sizeof(*ds0_dat))) == NULL) {
		WL_ERROR(("wl: %s: out of mem, malloced %d bytes\n",
			__FUNCTION__, MALLOCED(ulp_info->wlc->osh)));
		err = BCME_NOMEM;
		goto done;
	}
	/* check the default MAC ULP features enabled */
	default_mac_ulp_feat = ulp_mac_ulp_features_default(ulp_info->wlc->pub->sih);
	ulp_info->user_ulp_features |= default_mac_ulp_feat;

	/* store ds0 shm's, which will be set to respective shm's in ds1 ucode in
	 * wlc_ulp_enter_post_ulpucode_cb
	 */
	/* slowcal related shm's */
	if (ulp_info->user_ulp_features & C_ULP_SLOWCAL_SKIP) {
		ds0_dat->ilp_per_h = wlc_bmac_read_shm(ulp_info->wlc->hw,
			M_ILP_PER_H(ulp_info->wlc));
		ds0_dat->ilp_per_l = wlc_bmac_read_shm(ulp_info->wlc->hw,
			M_ILP_PER_L(ulp_info->wlc));
		WL_ULP_DBG(("%s: save ulp ilp_period ilp_per_h = %x ilp_per_l = %x\n",
			__FUNCTION__, ds0_dat->ilp_per_h, ds0_dat->ilp_per_l));
	}
	ulp_info->ds0_dat = ds0_dat;
done:
	return err;
}

/* Note: this routine sets M_ULP_FEATURES shm. enable: TRUE: means set, else reset */
void
wlc_ulp_configure_ulp_features(wlc_ulp_info_t *ulp_info, uint32 val, uint32 enable)
{
	if (enable)
		ulp_info->ulp_features |= val;
	else
		ulp_info->ulp_features &= ~val;
}

/* **** START: Following code is part of ULP_ALGO_TIMER algorithm **** */

static void
wlc_ulp_dngl_set_ulp_state(wlc_ulp_info_t *ulp_info, wlc_ulp_dngl_ulp_state_t state)
{
	WL_ULP_DBG(("ULPST-change: from: %d to %d\n", ulp_info->ulp_state, state));
	ulp_info->ulp_state = state;
}

static wlc_ulp_dngl_ulp_state_t
wlc_ulp_dngl_get_ulp_state(wlc_ulp_info_t *ulp_info)
{
	return ulp_info->ulp_state;
}

/* ULP_ALGO_TIMER algorithm : this is the function which starts ulp mode. */
static void
wlc_ulp_timeout(void *arg)
{
	int err = BCME_OK;
	wlc_ulp_info_t *ui = (wlc_ulp_info_t *)arg;
	wlc_ulp_params_t *upa = ui->ulp_params;
	WL_ULP_DBG(("%s: enter \n", __FUNCTION__));

	/* check for ulp entry: to make sure nothing happened when this timer was running. */
	if ((upa->ulp_wait > 0) && (upa->ulp_mode == ULP_MODE_ENABLE)) {
		int cmd_arg = 1;	/* arg to wowl force to be 1 to enable it */
		WL_ULP_ERR(("%s: going to ULP!\n", __FUNCTION__));
		ui->ulp_state = DNGL_ULP_STATE_ENTERING;
		/* call wowl force */
		if ((err = wlc_iovar_op(ui->wlc, "wowl_force", NULL, 0,
				(void *)&cmd_arg, sizeof(cmd_arg), IOV_SET, ui->wlcif))
				!= BCME_OK) {
			WL_ERROR(("failed to enter ULP!err = [%d]\n", err));
			ui->ulp_state = DNGL_ULP_STATE_IDLE;
			goto done;
		}
	} else {
		WL_ULP_DBG(("%s: enter, IGNORED! \n", __FUNCTION__));
	}
done:
	return;
}

/* Main starting point of ULP_ALGO_TIMER algorithm */
/* hook called by wlc_pm2_enter_ps. Note that for ulp to work, PM2 should be enabled.
 *	Note: expects ulp_cfg to be NON-NULL and ULP to be enabled.
 */
static int
wlc_ulp_entry_timer_start(wlc_ulp_info_t *ui)
{
	int err = BCME_OK;
	wlc_ulp_params_t *upa = ui->ulp_params;
	WL_ULP_DBG(("%s: enter\n", __FUNCTION__));

	/* start timer */
	if (wlc_ulp_dngl_get_ulp_state(ui) == DNGL_ULP_STATE_IDLE) {
		WL_ULP_DBG(("%s: starting timer\n", __FUNCTION__));
		wl_add_timer(ui->wlc->wl, ui->ulp_timer, (upa->ulp_wait), 0);
		wlc_ulp_dngl_set_ulp_state(ui, DNGL_ULP_STATE_COUNTING);
	} else if (wlc_ulp_dngl_get_ulp_state(ui) == DNGL_ULP_STATE_COUNTING) {
		WL_ULP_DBG(("%s: duplicate st change req:ignore: state: %d\n", __FUNCTION__,
			wlc_ulp_dngl_get_ulp_state(ui)));
		/* Timer is already running.. so delete and restart again */
		wl_del_timer(ui->wlc->wl, ui->ulp_timer);
		wl_add_timer(ui->wlc->wl, ui->ulp_timer, (upa->ulp_wait), 0);
		/* Set ULP State to counting */
		wlc_ulp_dngl_set_ulp_state(ui, DNGL_ULP_STATE_COUNTING);
	}
	else {
		/* Skip the Timer start/ Restart as Driver is already in ULP mode */
		WL_ULP_DBG(("%s: Skipping Timer Start/Stop\n", __FUNCTION__));
	}
	return err;
}

/* called by PM2 timer disruptions, tx pkt, etc.
 *	Note: expects ulp_cfg to be NON-NULL and ULP to be enabled.
 */
static int
wlc_ulp_entry_timer_stop(wlc_ulp_info_t *ui)
{
	int err = BCME_OK;

	WL_ULP_DBG(("%s: enter\n", __FUNCTION__));

	/* stop timer if it was already running */
	if (wlc_ulp_dngl_get_ulp_state(ui) == DNGL_ULP_STATE_COUNTING) {
		wl_del_timer(ui->wlc->wl, ui->ulp_timer);
		wlc_ulp_dngl_set_ulp_state(ui, DNGL_ULP_STATE_IDLE);
	} else {
		WL_ULP_DBG(("%s: duplicate st change req:ignore: state: %d\n", __FUNCTION__,
			wlc_ulp_dngl_get_ulp_state(ui)));
	}
	return err;
}

/* This will make sure that warmboot is still in progress */
bool
wlc_is_ulp_pending(wlc_ulp_info_t *ulp_info)
{
	return (wlc_ulp_dngl_get_ulp_state(ulp_info) == DNGL_ULP_STATE_ASSOC_RECR_PENDING);
}

/* assoc/reassoc/roam/disassoc notification callback */
static void
wlc_ulp_as_upd_cb(void *ctx, bss_assoc_state_data_t *notif_data)
{
	wlc_ulp_info_t *ui = (wlc_ulp_info_t *)ctx;
	wlc_bsscfg_t *cfg = notif_data->cfg;
	int pm;

	WL_ULP_DBG(("%s: AS_ASSOC_VERIFY in ulp\n", __FUNCTION__));
	if ((notif_data->state == AS_IDLE) &&
			wlc_is_ulp_pending(ui)) {
		/* perform here every thing required after assoc recreate
		 * - restore PM
		 */
		if (ui->cp1)
		{
			pm = ui->cp1->PM;
			wlc_ioctl(ui->wlc, WLC_SET_PM, (void *)&pm, sizeof(pm),
				cfg->wlcif);
		}

		/* fake the PM transition */
		wlc_update_pmstate(cfg, TX_STATUS_BE);
		wlc_ulp_dngl_set_ulp_state(ui, DNGL_ULP_STATE_IDLE);
		/* Start the ulp algo timer after PM mode is set during assoc recreate
		* Reason: Timer start is called during assoc recreate time and it is ignored,
		* causing delay in re enter of DS1 again and DS1 entry will happen only when
		* TIM bit is set in the beacon. So it is better to start timer during PM restore and
		* it is cleared depending on TX/RX Traffic
		*/
		wlc_ulp_perform_sleep_ctrl_action(ui, ULP_TIMER_START);

		ulp_cleanup_cache();
		ui->cp1 = NULL;
		ui->cp2 = NULL;
	}
}

/* ULP_ALGO_TIMER algorithm: starts timer */
int
wlc_ulp_perform_sleep_ctrl_action(wlc_ulp_info_t *ui, uint8 action)
{
	wlc_info_t *wlc = ui->wlc;
	wlc_ulp_params_t *upa = ui->ulp_params;
	wlc_bsscfg_t *ulp_cfg = wlc_bsscfg_find_by_wlcif(ui->wlc, ui->wlcif);
	int err = BCME_OK;

	if (upa->ulp_algo != ULP_ALGO_TIMER) {
		/* Note: "algo is NOT ULP_ALGO_TIMER " is NOT an error condition */
		goto done;
	}

	if ((upa->ulp_mode != ULP_MODE_ENABLE) || (ulp_cfg == NULL)) {
		err = BCME_NODEVICE;
		goto done;
	}

	WL_ULP_DBG(("%s enter from:%p\n",
		__FUNCTION__, OSL_OBFUSCATE_BUF(__builtin_return_address(0))));

	if (action == ULP_TIMER_START) {
		wlc_ulp_entry_timer_start(wlc->ulp);
	} else if (action == ULP_TIMER_STOP) {
		wlc_ulp_entry_timer_stop(wlc->ulp);
	}

done:
	WL_ULP_DBG(("%s exit: %d\n", __FUNCTION__, err));
	return err;
}

bool
wlc_ulp_allow_wowl_force(wlc_ulp_info_t *ui)
{
	wlc_ulp_params_t *upa = ui->ulp_params;

	/* In ULP builds, "wowl_force 1" IOVAR works only when "wl ulp 1" command is issued and when
	 * the ULP state is DNGL_ULP_STATE_ENTERING. This means, "wl wowl_force 1" command cannot
	 * be issued by the user for ULP builds.
	 */
	if ((upa->ulp_mode == ULP_MODE_ENABLE) && (ui->ulp_state == DNGL_ULP_STATE_ENTERING))
		return TRUE;

	return FALSE;
}

/* This Function is used to populate ULP SHM offsets required for DHD for DS1 exit
* Processing.
*/
static void
wlc_ulp_shm_info(wlc_ulp_info_t *ui, ulp_shm_info_t *ulp_shm)
{
	wlc_info_t *wlc = ui ->wlc;
	/* Switch to ULP SHM to get the SDIOCTRL SHM offset */
	wlc_bmac_autod11_shm_upd(wlc->hw, D11_IF_SHM_ULP);
	/* get the SHM offset info and sent back */
	ulp_shm->m_ulp_ctrl_sdio = M_DS1_CTRL_SDIO(wlc);
	ulp_shm->m_ulp_wakeevt_ind = M_WAKEEVENT_IND(wlc);
	ulp_shm->m_ulp_wakeind = M_ULP_WAKE_IND(wlc);
	/* Switch back to Standard ucode SHM offset mapping */
	wlc_bmac_autod11_shm_upd(wlc->hw, D11_IF_SHM_STD);
}

#ifdef ULP_DUMP
/* This function is used to clear the ulpstats dump structures and can be called using
* iovar wl dump_clear ulpstats
*/
static int
wlc_ulp_dumpstats_clr(void *ctx)
{
	wlc_ulp_info_t *ulp_info = (wlc_ulp_info_t *)ctx;
	wlc_ulp_params_t *upa = ulp_info->ulp_params;
	if (!upa) {
		WL_ULP_ERR(("%s upa is NULL\n", __FUNCTION__));
		return BCME_ERROR;
	}
	ulp_switchtimes_t *ulp_info_swtime = &upa->ulp_swtime;

	if (!ulp_info_swtime) {
		WL_ULP_ERR(("%s ulp_info_swtime is NULL\n", __FUNCTION__));
		return BCME_ERROR;
	}
	/* Reset/Clear the ULP statistics Structure */
	bzero(ulp_info_swtime, sizeof(*(ulp_info_swtime)));
	return BCME_OK;
}

/* Function to dump DS1 statistics
* DS1 Duration :- Time Duration chip was in DS1 state
* RX BCN MBSS :- Number of Beacons received during DS1
* Wakeup Reason :- DS1 exit reason : HOST Wakeup or MAGIC PKT etc
*/
static int
wlc_ulp_dumpstats(void *ctx, struct bcmstrbuf *b)
{
	int val, val1, val2, val3 = 0;
	wlc_ulp_info_t *ulp_info = (wlc_ulp_info_t *)ctx;
	wlc_ulp_params_t *upa = ulp_info->ulp_params;
	ulp_switchtimes_t *ulp_info_swtime = &upa->ulp_swtime;

	bcm_bprintf(b, "\n\t\t\t DS1 Statistics \n");

	/* DS0 Duration */
	DS1_TIMECAL_LOOP(b, "\n DS0 Duration (msec) :", val, ulp_info_swtime,
		val1, val2, val3)
		bcm_bprintf(b, "\t %8d", (ulp_info_swtime->entry[val].DS0time));

	/* DS1 Duration */
	DS1_TIMECAL_LOOP(b, "\n DS1 Duration (msec) :", val, ulp_info_swtime,
		val1, val2, val3)
		bcm_bprintf(b, "\t %8d",
		(ulp_info_swtime->entry[val].endtime -
		ulp_info_swtime->entry[val].starttime));

	DS1_TIMECAL_LOOP(b, "\n RX BCN MBSS :\t", val, ulp_info_swtime, val1, val2, val3)
		bcm_bprintf(b, "\t %8d", ulp_info_swtime->entry[val].rxbcnmbss);

	DS1_TIMECAL_LOOP(b, "\n RX BCN LOSS :\t", val, ulp_info_swtime, val1, val2, val3)
		bcm_bprintf(b, "\t %8d", ulp_info_swtime->entry[val].rxbcnloss);

	DS1_TIMECAL_LOOP(b, "\n ULP WAIT TIME :", val, ulp_info_swtime, val1, val2, val3)
		bcm_bprintf(b, "\t %8d ", ulp_info_swtime->entry[val].ulp_wait);

	DS1_TIMECAL_LOOP(b, "\n WAKEUP REASON :", val, ulp_info_swtime, val1, val2, val3)
		bcm_bprintf(b, "\t %s", ulpexit_reason[ulp_info_swtime->entry[val].wakereason]);
	bcm_bprintf(b, "\n \n Chip Entered DS1 : '%d' times",
		(ULPSWITCH_TIMES_HISTORY * ulp_info_swtime->totalDS1Count) +
		ulp_info_swtime->index);
	bcm_bprintf(b, "\n \n");
	return BCME_OK;
}

/* Function to handle ULP Wakeup reason depending on
* ulp wakeind and wowl_wakeind
*/
static void
wlc_ulp_switchtime_setreason(wlc_ulp_info_t *ulp_info, wlc_ulp_cache_p2_t *cp2, uint32 index)
{
	uint8 *reason = &ulp_info->ulp_params->ulp_swtime.entry[index].wakereason;

	/* Get the ULP wakeup reason depending on ulp_wakeind & wowl_wakeind
	* wowl_wakeind takes precedence in this case and can override ulp_wakeind settings.
	* As per the design, only one of this variables will be set based on Wake cdts by ucode
	*/
	switch (cp2->ulp_wakeind)
	{
		case C_HOST_WAKEUP :
			 *reason = ULP_HOSTWAKEUP;
			break;
		case C_HUDI_DS1_EXIT :
			 *reason = ULP_HUDIEXIT;
			break;
		default:
			WL_ULP_DBG(("%s cp2->wakeind is NULL\n", __FUNCTION__));
			break;
	}
	switch (cp2->wowl_wakeind)
	{
		case WL_WOWL_MAGIC :
			 *reason = ULP_UCASTMAGICPACKET;
			break;
		case WL_WOWL_BCAST | WL_WOWL_MAGIC:
			 *reason = ULP_BCASTMAGICPACKET;
			break;
		case WL_WOWL_NET :
			 *reason = ULP_NETPATTERN;
			break;
		case WL_WOWL_BCN :
			 *reason = ULP_LOSSOFBEACON;
			break;
		case WL_WOWL_DIS :
			 *reason = ULP_DISASSOC;
			break;
		case WL_WOWL_TST :
			 *reason = ULP_WAKEUP_TST;
			break;
		case WL_WOWL_ULP_BAILOUT :
			 *reason = ULP_UNKNOWNPACKET;
			break;
		default:
			WL_ULP_DBG(("%s cp2->wowl_wakeind is NULL\n", __FUNCTION__));
			break;
	}
}
/* Function to handle ULP Switch times computation depending on
* ULP Enter/ ULP Exit states
*/
static int
wlc_ulp_switchtime_update(wlc_ulp_info_t *ulp_info, wlc_ulp_params_t *upa, wlc_ulp_cache_p2_t *cp2,
	wlc_ulp_modes_t ulpmode)
{
	int err = BCME_OK;
	ulp_switchtimes_t *ulp_info_swtime = NULL;
	ulp_switchtimes_t *upa_swtime = NULL;
	uint32 index = 0;

	ulp_info_swtime = &ulp_info->ulp_params->ulp_swtime;
	upa_swtime = &upa->ulp_swtime;

	if (upa_swtime == NULL) {
		err = BCME_BADADDR;
		WL_ULP_ERR(("%s upa_swtime is NULL\n", __FUNCTION__));
		goto done;
	}

	if (ulp_info_swtime == NULL) {
		err = BCME_BADADDR;
		WL_ULP_ERR(("%s ulp_info_swtime is NULL\n", __FUNCTION__));
		goto done;
	}
	index = ulp_info_swtime->index;

	/* If index exceeds the max value, cycle back to index 0 */
	if (index >= ULPSWITCH_TIMES_HISTORY) {
		WL_ULP_DBG(("[%s] Reset the index to point to first index\n", __FUNCTION__));
		ulp_info_swtime->totalDS1Count++;
		index = 0;
		ulp_info_swtime->index = 0;
	}
	/* ULP_ENTER case is applicable during DS1 enter */
	if (ulpmode == ULP_ENTER) {
		/* Update the starttime in ms  to startime variable which will be stored in BM
		* during DS1
		*/
		ulp_info_swtime->entry[index].starttime =
			LHL_REG(ulp_info->wlc->pub->sih, lhl_hibtim_adr, 0, 0)/32;

		ulp_info_swtime->entry[index].DS0time =
			(LHL_REG(ulp_info->wlc->pub->sih, lhl_hibtim_adr, 0, 0)/32 -
			ulp_info->ulp_params->ds0time);

		/* store ulp_wait configuration value before entering DS1 */
		ulp_info_swtime->entry[index].ulp_wait = (uint32)upa->ulp_wait;

		/* Copy the ULP statistics info to cache structure during DS1 enter phase */
		memcpy(upa_swtime, ulp_info_swtime, sizeof(*ulp_info_swtime));
	} else if (ulpmode == ULP_EXIT) {
		/* This is applicable for DS1 exit */
		/* Copy the cache statistics data back to ulp_info structure */
		memcpy(ulp_info_swtime, upa_swtime, sizeof(*upa_swtime));

		/* map index variable to restore index again */
		index = ulp_info_swtime->index;

		if (!cp2) {
			err = BCME_BADADDR;
			goto done;
		}
		/* Save the RX Beacon count which is programmed during DS1 to
		* ulp_info_swtime structure
		*/
		ulp_info_swtime->entry[index].rxbcnmbss = cp2->ulp_rxbcnmbss;

		/* save the RX Beacon loss count which is programmed during DS1 to
		* ulp_info_swtime structure
		*/
		ulp_info_swtime->entry[index].rxbcnloss = cp2->ulp_rxbcnloss;

		/* Update the DS1 Wakeup depending on ulp_wakeind and wowl_wakeind variables */
		wlc_ulp_switchtime_setreason(ulp_info, cp2, index);

		/* Get current time using LHL free running timer which is running in both
		* DS0 and DS1 state
		*/
		ulp_info_swtime->entry[index].endtime =
			LHL_REG(ulp_info->wlc->pub->sih, lhl_hibtim_adr, 0, 0)/32;
		/* Increment  index so that next DS1 entry/Exit will be populated on next index */
		ulp_info_swtime->index++;
	}
done:
	return err;
}
/* Function is used to dump FCBS Sequence populated by Driver for DS0
* RADIO PU/PD. FCBS sequence are in the form of Command and Data pointers
* http://hwnbu-twiki.sj.broadcom.com/twiki/pub/Mwgroup/SoftwareProject43012/MAC_FCBS.pptx
*/
static void
wlc_ulp_fcbsstats_readblock(wlc_ulp_info_t *ulp_info, struct bcmstrbuf *b, uint32 offset)
{
	uint32 numseq, val, counter = 0;
	int index = 0;
	numseq = wlc_bmac_read_shm(ulp_info->wlc->hw, offset);
	if (!numseq) {
		bcm_bprintf(b, " \t \t Block is empty \n");
	} else {
		bcm_bprintf(b, " *********** Number of sequences = [%d] **************\n", numseq);
		for (index = 1; index <= numseq; index++) {
			counter++;

			val = wlc_bmac_read_shm(ulp_info->wlc->hw, offset + counter * 2);
			bcm_bprintf(b, " Command Pointer for sequence        [0%x] (0%x)\n",
				val, val << 2);
			counter++;

			val = wlc_bmac_read_shm(ulp_info->wlc->hw, offset + counter * 2);
			bcm_bprintf(b, " Data Pointer for sequence           [0%x] (0%x)\n",
				val, val << 2);
			counter++;

			bcm_bprintf(b, " Control Word for sequence           [0%x] \n",
				wlc_bmac_read_shm(ulp_info->wlc->hw, offset + counter * 2));
			counter++;

			bcm_bprintf(b, " Wait Time for sequence              [0%x] \n",
				wlc_bmac_read_shm(ulp_info->wlc->hw, offset + counter * 2));
			counter++;

			bcm_bprintf(b, " Control Time Output for sequence    [0%x]\n",
				wlc_bmac_read_shm(ulp_info->wlc->hw, offset + counter * 2));
			counter++;

			bcm_bprintf(b, " Completion Word Output for sequence [0%x]\n",
				wlc_bmac_read_shm(ulp_info->wlc->hw, offset + counter * 2));
			bcm_bprintf(b, "\n");
		}
	}
}
/* This function is used to dump DS0 FCBS RADIO PU/PD Sequence which are
* required for PM1/PM2 mode.Ucode uses this FCBS sequence Power On/Off RADIO
*/
static int
wlc_ulp_fcbsstats(void *ctx, struct bcmstrbuf *b)
{
	wlc_ulp_info_t *ulp_info = (wlc_ulp_info_t *)ctx;

	bcm_bprintf(b, "\n\t\t\t *DS0 FCBS Stats*  \n\n");

	/* This dumps the FCBS DS0 RADIO POWER UP SEQUENCE */
	bcm_bprintf(b, " \t *M_FCBS_DS0_RADIO_PU_BLOCK* \n");
	wlc_ulp_fcbsstats_readblock(ulp_info, b, M_FCBS_DS0_RADIO_PU_BLOCK(ulp_info->wlc));

	/* This dumps the FCBS DS0 RADIO POWER DOWN SEQUENCE */
	bcm_bprintf(b, " \t *M_FCBS_DS0_RADIO_PD_BLOCK* \n");
	wlc_ulp_fcbsstats_readblock(ulp_info, b, M_FCBS_DS0_RADIO_PD_BLOCK(ulp_info->wlc));

	bcm_bprintf(b, "\n");
	return BCME_OK;
}
#endif /* ULP_DUMP */

/* **** END: --- part of ULP_ALGO_TIMER algorithm **** */
