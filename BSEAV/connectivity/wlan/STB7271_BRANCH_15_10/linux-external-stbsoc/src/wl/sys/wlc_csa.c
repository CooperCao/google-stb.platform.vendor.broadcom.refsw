/**
 * @file
 * @brief
 * 802.11h Channel Switch Announcement and Extended Channel Switch Announcement module
 * Broadcom 802.11abgn Networking Device Driver
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
 * Related to radar avoidance.
 * After a valid channel has been selected, AP sends out Channel Switch Announcement (CSA) at same
 * time the beacon and probe response are updated with CSA IE. The channel switching IE gets update
 * on each dpc. When this counter in the IE reaches zero, the AP switch the channel and same time
 * the probe is updated with new channel and offset info in the additional IE. All the STA in the
 * BSS synchronize to the beacon to switch their channel at same time.
 */


#include <wlc_cfg.h>

#ifdef WLCSA

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scb.h>
#include <wlc_hw.h>
#include <wlc_ap.h>
#include <wlc_assoc.h>
#ifdef WLMCHAN
#include <wlc_mchan.h>
#endif
#include <wl_export.h>
#include <wlc_11h.h>
#include <wlc_csa.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_mgmt_vs.h>
#include <wlc_ie_reg.h>
#include <wlc_pcb.h>
#include <wlc_ht.h>
#include <wlc_txc.h>
#include <wlc_ampdu.h>
#include <wlc_tx.h>
#ifdef EVENT_LOG_COMPILE
#include <event_log.h>
#endif
#include <event_trace.h>
#include <wlc_pspretend.h>
#include <wlc_csrestrict.h>
#include <wlc_event_utils.h>
#include <wlc_scan_utils.h>
#include <wlc_dfs.h>
#include <wlc_rsdb.h>
#include <phy_misc_api.h>

/* IOVar table */
/* No ordering is imposed */
enum {
	IOV_CHANSPEC_SWITCH,	/* send CSA with chanspec as input */
	IOV_CHANSPEC_SWITCH_RESTRICT_TXWIN
};

static const bcm_iovar_t wlc_csa_iovars[] = {
#ifdef AP
	{"csa", IOV_CHANSPEC_SWITCH, (IOVF_SET_UP), 0, IOVT_BUFFER, sizeof(wl_chan_switch_t)},
#endif /* AP */
	{NULL, 0, 0, 0, 0, 0}
};

/* CSA module info */
struct wlc_csa_info {
	wlc_info_t *wlc;
	int cfgh;			/* bsscfg cubby handle */
	int scbh;			/* scb cubby handle */
	bcm_notif_h csa_notif_hdl;
};

/* local functions */
/* module */
static int wlc_csa_up(void *ctx);
static int wlc_csa_doiovar(void *ctx, uint32 actionid,
	void *params, uint p_len, void *arg, uint len, uint val_size, struct wlc_if *wlcif);

/* cubby */
static int wlc_csa_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_csa_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg);
static int wlc_csa_cubby_set(void *ctx, wlc_bsscfg_t *bsscfg, const uint8 *data, int len);
static int wlc_csa_cubby_get(void *ctx, wlc_bsscfg_t *bsscfg, uint8 *data, int *len);
static int wlc_csa_scb_init(void *ctx, struct scb *scb);
static void wlc_csa_scb_deinit(void *ctx, struct scb *scb);
static uint wlc_csa_scb_secsz(void *ctx, struct scb *scb);
#ifdef BCMDBG
static void wlc_csa_bsscfg_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);
static void wlc_csa_scb_dump(void *ctx, struct scb *scb, struct bcmstrbuf *b);
#else
#define wlc_csa_bsscfg_dump NULL
#define wlc_csa_scb_dump NULL
#endif

/* up/down */
static void wlc_csa_bsscfg_up_down(void *ctx, bsscfg_up_down_event_data_t *evt_data);

/* timer */
static void wlc_csa_timeout(void *arg);

/* action frame */
static int wlc_send_action_switch_channel_ex(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
	const struct ether_addr *dst, wl_chan_switch_t *csa, uint8 action_id);
static void wlc_send_public_action_switch_channel(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
  const struct ether_addr *dst, wl_chan_switch_t *csa);

/* channel switch */
#ifdef AP
static int wlc_csa_apply_channel_switch(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg);
#endif

/* RSDB module callback register for handling clone events */
#ifdef WLRSDB
static void wlc_csa_rsdb_clone_handler(void *ctx, rsdb_cfg_clone_upd_t *notif_data);
#endif

#ifdef STA
static void wlc_csa_process_channel_switch(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg);
#endif

/* unicast csa action ack */
static void wlc_csa_unicast_tx_complete(wlc_info_t *wlc, uint txstatus, void *arg);

/* IE mgmt */
#ifdef AP
static uint wlc_csa_calc_csa_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_csa_write_csa_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_csa_calc_ext_csa_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_csa_write_ext_csa_ie(void *ctx, wlc_iem_build_data_t *data);
#ifdef WL11AC
static uint wlc_csa_calc_csw_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_csa_write_csw_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_csa_csw_calc_wide_bw_ie_len(void *ctx, wlc_iem_calc_data_t *calc);
static int wlc_csa_csw_write_wide_bw_ie(void *ctx, wlc_iem_build_data_t *build);
#endif
#endif /* AP */
#ifdef STA
static int wlc_csa_bcn_parse_csa_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_csa_bcn_parse_ext_csa_ie(void *ctx, wlc_iem_parse_data_t *data);
#endif /* STA */
static void wlc_csa_obss_dynbw_notif_cb_notif(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
	int status, int signal, chanspec_t chanspec);

/* cubby structure and access macros */
typedef struct {
	struct wl_timer *csa_timer;	/* time to switch channel after last beacon */
	wl_chan_switch_t csa;
	struct {
		chanspec_t chanspec;		/* target chanspec */
		uint32	secs;			/* seconds until channel switch */
	} channel_sw;
} wlc_csa_t;
#define CSA_BSSCFG_CUBBY_LOC(csa, cfg) ((wlc_csa_t **)BSSCFG_CUBBY(cfg, (csa)->cfgh))
#define CSA_BSSCFG_CUBBY(csa, cfg) (*CSA_BSSCFG_CUBBY_LOC(csa, cfg))

#define CSA_UNICAST_RELOCATION_NONE	0x0
#define CSA_UNICAST_RELOCATION_PENDING	0x1
#define CSA_UNICAST_RELOCATION_SUCCESS	0x2
#define CSA_UNICAST_ACK_COUNTER	3
#define WLC_CSA_BSSCFG_CUBBY_SIZE	(sizeof(wlc_csa_t))
typedef struct {
	uint8	dcs_relocation_state; /* unicast CSA state */
	uint8	dcs_ack_counter; /* unicast CSA ACK counter */
} wlc_csa_scb_cubby_t;
#define CSA_SCB_CUBBY_LOC(csa, scb) ((wlc_csa_scb_cubby_t **)SCB_CUBBY((scb), (csa)->scbh))
#define CSA_SCB_CUBBY(csa, scb) (*(CSA_SCB_CUBBY_LOC(csa, scb)))

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>


/* module */
wlc_csa_info_t *
BCMATTACHFN(wlc_csa_attach)(wlc_info_t *wlc)
{
	wlc_csa_info_t *csam;
	bcm_notif_module_t *notif;
	bsscfg_cubby_params_t bsscfg_cubby_params;

#ifdef AP
	uint16 bcnfstbmp = FT2BMP(FC_BEACON) | FT2BMP(FC_PROBE_RESP);
#endif
	scb_cubby_params_t cubby_params;

	if ((csam = MALLOCZ(wlc->osh, sizeof(wlc_csa_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	csam->wlc = wlc;

	/* reserve cubby space in the bsscfg container for per-bsscfg private data */
	bzero(&bsscfg_cubby_params, sizeof(bsscfg_cubby_params));
	bsscfg_cubby_params.context = csam;
	bsscfg_cubby_params.fn_deinit = wlc_csa_bsscfg_deinit;
	bsscfg_cubby_params.fn_init = wlc_csa_bsscfg_init;
	bsscfg_cubby_params.fn_get = wlc_csa_cubby_get;
	bsscfg_cubby_params.fn_set = wlc_csa_cubby_set;
	bsscfg_cubby_params.config_size = WLC_CSA_BSSCFG_CUBBY_SIZE;
#if defined(BCMDBG) || defined(WLDUMP)
	bsscfg_cubby_params.fn_dump = wlc_csa_bsscfg_dump;
#endif
	if ((csam->cfgh = wlc_bsscfg_cubby_reserve_ext(wlc, sizeof(wlc_csa_t *),
	                                                      &bsscfg_cubby_params)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;

	}

	bzero(&cubby_params, sizeof(cubby_params));
	cubby_params.context = (void *)csam;
	cubby_params.fn_init = wlc_csa_scb_init;
	cubby_params.fn_deinit = wlc_csa_scb_deinit;
	cubby_params.fn_dump = wlc_csa_scb_dump;
	cubby_params.fn_secsz = wlc_csa_scb_secsz;
	csam->scbh = wlc_scb_cubby_reserve_ext(wlc, sizeof(wlc_csa_scb_cubby_t *), &cubby_params);
	if (csam->scbh < 0) {
		WL_ERROR(("wl%d: %s: wlc_scb_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register bsscfg up/down callbacks */
	if (wlc_bsscfg_updown_register(wlc, wlc_csa_bsscfg_up_down, csam) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_updown_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register IE mgmt callback */
#ifdef AP
	/* bcn/prbrsp */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, bcnfstbmp, DOT11_MNG_CHANNEL_SWITCH_ID,
	      wlc_csa_calc_csa_ie_len, wlc_csa_write_csa_ie, csam) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, csa in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_add_build_fn_mft(wlc->iemi, bcnfstbmp, DOT11_MNG_EXT_CSA_ID,
	      wlc_csa_calc_ext_csa_ie_len, wlc_csa_write_ext_csa_ie, csam) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, ext csa in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#ifdef WL11AC
	if (wlc_iem_add_build_fn_mft(wlc->iemi, bcnfstbmp, DOT11_MNG_CHANNEL_SWITCH_WRAPPER_ID,
	      wlc_csa_calc_csw_ie_len, wlc_csa_write_csw_ie, csam) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, csw in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_csw, DOT11_MNG_WIDE_BW_CHANNEL_SWITCH_ID,
	      wlc_csa_csw_calc_wide_bw_ie_len, wlc_csa_csw_write_wide_bw_ie, csam) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, wbw in csw ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* WL11AC */
#endif /* AP */
#ifdef STA
	if (wlc_iem_add_parse_fn(wlc->iemi, FC_BEACON, DOT11_MNG_CHANNEL_SWITCH_ID,
	                         wlc_csa_bcn_parse_csa_ie, csam) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, csa in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_add_parse_fn(wlc->iemi, FC_BEACON, DOT11_MNG_EXT_CSA_ID,
	                         wlc_csa_bcn_parse_ext_csa_ie, csam) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, ext csa in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* STA */

	/* keep the module registration the last other add module unregistratin
	 * in the error handling code below...
	 */
	if (wlc_module_register(wlc->pub, wlc_csa_iovars, "csa", csam, wlc_csa_doiovar,
	                        NULL, wlc_csa_up, NULL) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	};

	notif = wlc->notif;
	ASSERT(notif != NULL);
	if (bcm_notif_create_list(notif, &csam->csa_notif_hdl) != BCME_OK) {
		WL_ERROR(("wl%d: %s: csa bcm_notif_create_list() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#ifdef WLRSDB
	if (RSDB_ENAB(wlc->pub)) {
		if (wlc_rsdb_cfg_clone_register(wlc->rsdbinfo,
			wlc_csa_rsdb_clone_handler, csam) != BCME_OK) {
			WL_ERROR(("wl%d: %s: wlc_rsdb_cfg_clone_register failed\n",
				wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
	}
#endif
	return csam;

	/* error handling */
fail:
	if (csam != NULL)
		MFREE(wlc->osh, csam, sizeof(wlc_csa_info_t));
	return NULL;
}

void
BCMATTACHFN(wlc_csa_detach)(wlc_csa_info_t *csam)
{
	wlc_info_t *wlc = csam->wlc;

#ifdef WLRSDB
	if (RSDB_ENAB(wlc->pub)) {
		(void)wlc_rsdb_cfg_clone_unregister(wlc->rsdbinfo,
			wlc_csa_rsdb_clone_handler, csam);
	}
#endif

	wlc_module_unregister(wlc->pub, "csa", csam);

	/* unregister bsscfg up/down callbacks */
	wlc_bsscfg_updown_unregister(wlc, wlc_csa_bsscfg_up_down, csam);
	if (csam->csa_notif_hdl != NULL) {
		bcm_notif_delete_list(&csam->csa_notif_hdl);
	}
	MFREE(wlc->osh, csam, sizeof(wlc_csa_info_t));
}

static const wlc_iem_tag_t BCMINITDATA(csw_ie_tags)[] = {
	DOT11_MNG_WIDE_BW_CHANNEL_SWITCH_ID,
	DOT11_MNG_VHT_TRANSMIT_POWER_ENVELOPE_ID,
};

static int
wlc_csa_up(void *ctx)
{
#ifdef WL11AC
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_info_t *wlc = csam->wlc;

	/* ignore the return code */
	(void)wlc_ier_sort_cbtbl(wlc->ier_csw, csw_ie_tags, ARRAYSIZE(csw_ie_tags));
#endif

	return BCME_OK;
}

static int
wlc_csa_doiovar(void *ctx, uint32 actionid,
	void *params, uint p_len, void *arg, uint len, uint val_size, struct wlc_if *wlcif)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_info_t *wlc = csam->wlc;
	wlc_bsscfg_t *bsscfg;
	int err = 0;
	int32 int_val = 0;
	int32 int_val2 = 0;
	int32 *ret_int_ptr;
	bool bool_val;
	bool bool_val2;
#ifdef AP
	wlc_bsscfg_t *apcfg;
	int idx;
#endif

	BCM_REFERENCE(len);
	BCM_REFERENCE(val_size);

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	if (p_len >= (int)sizeof(int_val) * 2)
		bcopy((void*)((uintptr)params + sizeof(int_val)), &int_val2, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;
	BCM_REFERENCE(ret_int_ptr);

	bool_val = (int_val != 0) ? TRUE : FALSE;
	bool_val2 = (int_val2 != 0) ? TRUE : FALSE;
	BCM_REFERENCE(bool_val);
	BCM_REFERENCE(bool_val2);

	/* update wlcif pointer */
	if (wlcif == NULL)
		wlcif = bsscfg->wlcif;
	ASSERT(wlcif != NULL);

	/* Do the actual parameter implementation */
	switch (actionid) {
#ifdef AP
	case IOV_SVAL(IOV_CHANSPEC_SWITCH): {
		wl_chan_switch_t *csa = (wl_chan_switch_t *)arg;

		if (wlc_dfs_scan_in_progress(wlc->dfs)) {
			return BCME_BUSY;
		}

		if (wf_chspec_malformed(csa->chspec)) {
			err = BCME_BADCHAN;
			break;
		}

#ifdef BCMDBG
		if (BSSCFG_STA(bsscfg)) {
			if (!bsscfg->up) {
				err = BCME_NOTREADY;
				break;
			}

			wlc_send_action_switch_channel_ex(csam, bsscfg, &bsscfg->BSSID,
				csa, DOT11_SM_ACTION_CHANNEL_SWITCH);
			break;
		}
#endif /* BCMDBG */

		FOREACH_UP_AP(wlc, idx, apcfg) {
			err = wlc_csa_do_channel_switch(csam, apcfg,
			  csa->chspec, csa->mode, csa->count, csa->reg, csa->frame_type);
		}

		break;
	}
#endif /* AP */
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/* bsscfg cubby */
static int
wlc_csa_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_info_t *wlc = csam->wlc;
	wlc_csa_t **pcsa = CSA_BSSCFG_CUBBY_LOC(csam, cfg);
	wlc_csa_t *csa;
	int err;

	if ((csa = MALLOCZ(wlc->osh, sizeof(wlc_csa_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		err = BCME_NOMEM;
		goto fail;
	}
	*pcsa = csa;

	/* init CSA timer */
	if ((csa->csa_timer =
	     wl_init_timer(wlc->wl, wlc_csa_timeout, (void *)cfg, "csa")) == NULL) {
		WL_ERROR(("wl%d: %s: wl_init_timer failed\n",
		          wlc->pub->unit, __FUNCTION__));
		err = BCME_NORESOURCE;
		goto fail;
	}
	/* Extended Channel Switch set */
	if (BSSCFG_AP(cfg)) {
		wlc_bsscfg_set_ext_cap(cfg, DOT11_EXT_CAP_EXT_CHAN_SWITCHING, TRUE);
	}
	return BCME_OK;
fail:
	if (csa != NULL)
		wlc_csa_bsscfg_deinit(ctx, cfg);
	return err;
}

static void
wlc_csa_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_info_t *wlc = csam->wlc;
	wlc_csa_t **pcsa = CSA_BSSCFG_CUBBY_LOC(csam, cfg);
	wlc_csa_t *csa = *pcsa;

	if (csa == NULL) {
		WL_ERROR(("wl%d: %s: CSA info not found\n", wlc->pub->unit, __FUNCTION__));
		return;
	}

	/* If datafifo was blocked, it should be released. */
	if (csa->csa.count &&
		(wlc->block_datafifo & DATA_BLOCK_QUIET)) {
		wlc_block_datafifo(wlc, DATA_BLOCK_QUIET, 0);
	}

	/* delete CSA timer */
	if (csa->csa_timer != NULL) {
		wl_free_timer(wlc->wl, csa->csa_timer);
		csa->csa_timer = NULL;
	}
	/* Extended Channel Switch Clear */
	if (BSSCFG_AP(cfg)) {
		wlc_bsscfg_set_ext_cap(cfg, DOT11_EXT_CAP_EXT_CHAN_SWITCHING, FALSE);
	}
	MFREE(wlc->osh, csa, sizeof(wlc_csa_t));
	*pcsa = NULL;
}

/* bsscfg cubby copy */
static int
wlc_csa_cubby_get(void *ctx, wlc_bsscfg_t *bsscfg, uint8 *data, int *len)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_csa_t *csa_cubby = NULL;

	ASSERT(bsscfg != NULL);
	ASSERT(data != NULL);
	if (*len < WLC_CSA_BSSCFG_CUBBY_SIZE) {
		WL_ERROR(("wl%d: %s: Buffer too short\n", csam->wlc->pub->unit, __FUNCTION__));
		*len = WLC_CSA_BSSCFG_CUBBY_SIZE;
		return BCME_BUFTOOSHORT;
	}
	csa_cubby = CSA_BSSCFG_CUBBY(csam, bsscfg);
	ASSERT(csa_cubby != NULL);
	memcpy(data, (uint8*)csa_cubby, WLC_CSA_BSSCFG_CUBBY_SIZE);
	*len = WLC_CSA_BSSCFG_CUBBY_SIZE;
	return BCME_OK;
}

/* bsscfg cubby copy */
static int
wlc_csa_cubby_set(void *ctx, wlc_bsscfg_t *bsscfg, const uint8 *data, int len)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_csa_t *csa_cubby = NULL;
	struct wl_timer *csa_timer_cp = NULL;

	ASSERT(bsscfg != NULL);
	ASSERT(data != NULL);
	ASSERT(len <= WLC_CSA_BSSCFG_CUBBY_SIZE);
	csa_cubby = CSA_BSSCFG_CUBBY(csam, bsscfg);
	ASSERT(csa_cubby != NULL);
	/* Here we take a backup of the csa_timer
	* pointer, because the data buffer contains
	* the complete struct take from old wlc.
	* Once we copy the buffer, we will overwrite
	* the csa_timer pointer from backup value.
	* New wlc has new pointer for csa_timer.
	*/
	csa_timer_cp = csa_cubby->csa_timer;
	memcpy((uint8*)csa_cubby, data, len);
	/* Restore the back up value */
	csa_cubby->csa_timer = csa_timer_cp;
	return BCME_OK;
}

static int
wlc_csa_scb_init(void *ctx, struct scb *scb)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_info_t *wlc = csam->wlc;
	wlc_csa_scb_cubby_t **pcsa_scb = CSA_SCB_CUBBY_LOC(csam, scb);
	wlc_csa_scb_cubby_t *csa_scb;

	if ((csa_scb = wlc_scb_sec_cubby_alloc(wlc, scb, sizeof(*csa_scb))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return BCME_NOMEM;
	}

	*pcsa_scb = csa_scb;

	return BCME_OK;
}

static void
wlc_csa_scb_deinit(void *ctx, struct scb *scb)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_info_t *wlc = csam->wlc;
	wlc_csa_scb_cubby_t **pcsa_scb = CSA_SCB_CUBBY_LOC(csam, scb);
	wlc_csa_scb_cubby_t *csa_scb = *pcsa_scb;

	if (csa_scb != NULL) {
		wlc_scb_sec_cubby_free(wlc, scb, csa_scb);
	}

	*pcsa_scb = NULL;
}

/**
 * Callback returning allocation size of a single csa (secondary) cubby within an scb. Is called
 * when a new scb is created, in the process of wlc_cubby_init().
 */
static uint wlc_csa_scb_secsz(void *ctx, struct scb *scb)
{
	/* TODO: is csa used in STA mode ?: if not: by returning 0, memory can be saved */
	return sizeof(wlc_csa_scb_cubby_t);
}

#ifdef BCMDBG
static void
wlc_csa_bsscfg_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);

	ASSERT(csa != NULL);

	/* CSA info */
	bcm_bprintf(b, "\tcsa->csa_timer %p\n", OSL_OBFUSCATE_BUF(csa->csa_timer));
	bcm_bprintf(b, "\tcsa->csa.mode %d, csa->csa.count %d\n",
	            csa->csa.mode, csa->csa.count);
	bcm_bprintf(b, "\tcsa->csa.chspec 0x%x, csa->csa.reg %d \n",
	            csa->csa.chspec, csa->csa.reg);
}

static void
wlc_csa_scb_dump(void *ctx, struct scb *scb, struct bcmstrbuf *b)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_csa_scb_cubby_t *csa_scb = CSA_SCB_CUBBY(csam, scb);

	ASSERT(csa_scb != NULL);

	/* CSA info */
	bcm_bprintf(b, "     csa_scb->dcs_relocation_state %d, csa_scb->dcs_ack_counter %d\n",
	            csa_scb->dcs_relocation_state, csa_scb->dcs_ack_counter);
}
#endif /* BCMDBG */

static void
wlc_csa_bsscfg_up_down(void *ctx, bsscfg_up_down_event_data_t *evt_data)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_info_t *wlc = csam->wlc;
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, evt_data->bsscfg);

	/* Only process bsscfg down events. */
	if (!evt_data->up) {
		ASSERT(csa != NULL);

		/* cancel any csa timer */
		evt_data->callbacks_pending =
		   (wl_del_timer(wlc->wl, csa->csa_timer) ? 0 : 1);
	}
}

static void
wlc_csa_unicast_tx_complete(wlc_info_t *wlc, uint txstatus, void *pkt)
{
	struct scb *scb;
	wlc_csa_info_t *csam = wlc->csa;
	wlc_csa_scb_cubby_t *csa_scb = NULL;

	BCM_REFERENCE(txstatus);

	/* make sure the scb still exists */
	if ((scb = WLPKTTAGSCBGET(pkt)) == NULL) {
		WL_ERROR(("wl%d: %s: unable to find scb from the pkt %p\n",
		          wlc->pub->unit, __FUNCTION__, OSL_OBFUSCATE_BUF(pkt)));
		return;
	}

	csa_scb = CSA_SCB_CUBBY(csam, scb);
	ASSERT(csa_scb != NULL);

	if (csa_scb->dcs_relocation_state == CSA_UNICAST_RELOCATION_PENDING) {
		csa_scb->dcs_ack_counter++;
		if (csa_scb->dcs_ack_counter == CSA_UNICAST_ACK_COUNTER)
			csa_scb->dcs_relocation_state = CSA_UNICAST_RELOCATION_SUCCESS;
	}
}

#ifdef WL_CS_RESTRICT_RELEASE
#define WL_CS_TRAIN_TXPOWER_PKTSNUM	16
static bool
wlc_csa_train_txpower_sendpkt(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct ether_addr *da)
{
	return wlc_sendnulldata(wlc, cfg, da, 0, 0, PRIO_8021D_VO, NULL, NULL);
}

static void
wlc_csa_train_txpower(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	int count = WL_CS_TRAIN_TXPOWER_PKTSNUM;

	STATIC_ASSERT(WL_CS_TRAIN_TXPOWER_PKTSNUM > 0);


	if (BSSCFG_STA(cfg)) {
		if (!BSSCFG_PSTA(cfg) && cfg->BSS) {
			while (wlc_csa_train_txpower_sendpkt(wlc, cfg, &cfg->BSSID) && --count);
		}
	} else if (BSSCFG_AP(cfg)) {
		bool can_again;
		do {
			struct scb *scb;
			struct scb_iter scbiter;

			can_again = FALSE;

			FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
				if (SCB_ASSOCIATED(scb)) {
					can_again = wlc_csa_train_txpower_sendpkt(wlc,
						cfg, &scb->ea) ? (--count != 0) : FALSE;
					if (!can_again) {
						break;
					}
				}
			}
		} while (can_again);
	}
}
#else
#define wlc_csa_train_txpower(csam, cfg)
#endif /* WL_CS_RESTRICT_RELEASE */

static void
wlc_csa_timeout(void *arg)
{
	wlc_bsscfg_t *cfg = (wlc_bsscfg_t *)arg;
	wlc_info_t *wlc = cfg->wlc;
	wlc_csa_info_t *csam = wlc->csa;
#ifdef STA
	wlc_bsscfg_t *active_assoc_cfg = AS_IN_PROGRESS_CFG(wlc);
#endif
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);
	struct scb_iter scbiter;
	struct scb *scb;
	bool switch_chnl = TRUE;
#ifdef WL_RESTRICTED_APSTA
	uint8 apsta_restrict = FALSE;
#endif
	ASSERT(csa != NULL);

	if (!wlc->pub->up)
		return;

	if (DEVICEREMOVED(wlc)) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc->pub->unit, __FUNCTION__));
		wl_down(wlc->wl);
		return;
	}

	csa->csa.count = 0;
	wlc_mute(wlc, OFF, PHY_MUTE_ALL);

	/*  Reset the csa count to unblock the TX path */
	csa->csa.count = 0;
	csa->csa.mode = DOT11_CSA_MODE_ADVISORY;

	if (BSSCFG_AP(cfg) && (csa->csa.frame_type == CSA_UNICAST_ACTION_FRAME)) {
		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
			struct scb *psta_prim = wlc_ap_get_psta_prim(wlc->ap, scb);
			wlc_csa_scb_cubby_t *csa_scb = CSA_SCB_CUBBY(csam, scb);
			ASSERT(csa_scb != NULL);

			if (!SCB_ISMULTI(scb) && SCB_ASSOCIATED(scb) &&
				(psta_prim == NULL) &&
				(csa_scb->dcs_relocation_state != CSA_UNICAST_RELOCATION_SUCCESS)) {
#if defined(BCMDBG)
				char eabuf[ETHER_ADDR_STR_LEN];
#endif
				WL_CHANINT(("dcs: csa ack pending from %s\n",
				            bcm_ether_ntoa(&scb->ea, eabuf)));
				switch_chnl = FALSE;
				break;
			}
		}
	}
#ifdef WLRSDB
	if (RSDB_ENAB(wlc->pub) && BSSCFG_STA(cfg) && (wlc_11h_get_spect_state(wlc->m11h, cfg) &
		NEED_TO_SWITCH_CHANNEL) &&
		wlc_rsdb_handle_sta_csa(wlc->rsdbinfo, cfg, csa->csa.chspec)) {
		/* It seems that channel switch needs a clone to other wlc.
		* Complete cloning and handle the channel switch in the RSDB callback.
		*/
		csa->channel_sw.secs = 0;
		if (wlc->block_datafifo & DATA_BLOCK_QUIET) {
			wlc_block_datafifo(wlc, DATA_BLOCK_QUIET, 0);
		}
		WL_REGULATORY(("wl%d.%d Defer channel switch on this wlc .."
			"try on wlc%d after clone..\n", WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg),
			WLCWLUNIT(wlc_rsdb_get_other_wlc(wlc))));
		return;
	}
#endif /* WLRSDB */
	if (BSSCFG_AP(cfg) && (csa->csa.frame_type == CSA_UNICAST_ACTION_FRAME)) {
		/* When multiple STAs assoc to AP, due to interference, some STA
		 * maynot ack csa. checking all the STAs acking csa would cause
		 * AP cannot change channel. In order to avoid this case, skip
		 * checking if all STA acks csa, to have a quick fix. Checking
		 * all STAs ack csa will be used for csa retry to make csa
		 * more robust (further enhancement)
		 */
		/* if (switch_chnl) */
		{
			/* time to switch channels... */
			wlc_do_chanswitch(cfg, csa->csa.chspec);
			wlc_11h_set_spect_state(wlc->m11h, cfg, NEED_TO_SWITCH_CHANNEL, 0);
			FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
				struct scb *psta_prim = wlc_ap_get_psta_prim(wlc->ap, scb);
				wlc_csa_scb_cubby_t *csa_scb = CSA_SCB_CUBBY(csam, scb);
				ASSERT(csa_scb != NULL);

				if (!SCB_ISMULTI(scb) && SCB_ASSOCIATED(scb) &&
					(psta_prim == NULL)) {
					csa_scb->dcs_relocation_state = CSA_UNICAST_RELOCATION_NONE;
					csa_scb->dcs_ack_counter = 0;
				}
			}
			wlc_csa_train_txpower(csam, cfg);
		}
	}
	else if (((BSSCFG_AP(cfg) || csa->csa.count == 0 || csa->channel_sw.secs) ||
		(PSTA_ENAB(wlc->pub) && BSSCFG_STA(cfg))) &&
	    (wlc_11h_get_spect_state(wlc->m11h, cfg) & NEED_TO_SWITCH_CHANNEL)) {
		if (BSSCFG_STA(cfg)) {
			csa->channel_sw.secs = 0;
			csa->csa.count = 0;
		}
		/* time to switch channels... */
		wlc_do_chanswitch(cfg, csa->csa.chspec);
		wlc_11h_set_spect_state(wlc->m11h, cfg, NEED_TO_SWITCH_CHANNEL, 0);
		wlc_csa_train_txpower(csam, cfg);
	}

	if ((BSSCFG_AP(cfg) && (csa->csa.frame_type == CSA_UNICAST_ACTION_FRAME)) && !switch_chnl) {
		wlc_11h_set_spect_state(wlc->m11h, cfg, NEED_TO_SWITCH_CHANNEL, 0);
		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
			struct scb *psta_prim = wlc_ap_get_psta_prim(wlc->ap, scb);
			wlc_csa_scb_cubby_t *csa_scb = CSA_SCB_CUBBY(csam, scb);
			ASSERT(csa_scb != NULL);

			if (!SCB_ISMULTI(scb) && SCB_ASSOCIATED(scb) &&
				(psta_prim == NULL)) {
				csa_scb->dcs_relocation_state = CSA_UNICAST_RELOCATION_NONE;
				csa_scb->dcs_ack_counter = 0;
			}
		}
	}

	/* Send Channel Switch Indication to upper (OS) layer. */
	if (BSSCFG_STA(cfg) || (BSSCFG_AP(cfg) && switch_chnl))
		wlc_bss_mac_event(wlc, cfg, WLC_E_CSA_COMPLETE_IND, NULL,
			WLC_E_STATUS_SUCCESS, 0, 0, &csa->csa.mode, sizeof(csa->csa.mode));
	wlc_csa_obss_dynbw_notif_cb_notif(csam, cfg,
		BCME_OK, CSA_CHANNEL_CHANGE_END, csa->csa.chspec);

	csa->csa.count = 0;
	csa->csa.mode = DOT11_CSA_MODE_ADVISORY;
	if (wlc->block_datafifo & DATA_BLOCK_QUIET) {
		wlc_block_datafifo(wlc, DATA_BLOCK_QUIET, 0);
	}

#ifdef STA
#ifdef WL_RESTRICTED_APSTA
	if (RAPSTA_ENAB(wlc->pub) && !P2P_GO(wlc, cfg) && (active_assoc_cfg != NULL)) {
		wlc_bss_info_t *bi;
		bi = wlc_assoc_get_join_target(wlc, 0);
		apsta_restrict = wlc_channel_apsta_restriction(wlc->cmi, bi->chanspec,
			wlc->chanspec);
	}
#endif
	if ((active_assoc_cfg != NULL) &&
#ifdef WL_RESTRICTED_APSTA
		(RAPSTA_ENAB(wlc->pub) && !apsta_restrict) &&
#endif
		((active_assoc_cfg->assoc->state == AS_WAIT_FOR_AP_CSA) ||
		(active_assoc_cfg->assoc->state == AS_WAIT_FOR_AP_CSA_ROAM_FAIL))) {
		wlc_11h_set_spect_state(wlc->m11h, active_assoc_cfg, NEED_TO_SWITCH_CHANNEL, 0);
		wlc_ap_mute(wlc, TRUE, active_assoc_cfg, -1);
		if (active_assoc_cfg->assoc->state == AS_WAIT_FOR_AP_CSA) {
			wlc_join_bss(active_assoc_cfg);
		}
		else {
			wlc_roam_complete(active_assoc_cfg, WLC_E_STATUS_FAIL,
				&active_assoc_cfg->BSSID,
				active_assoc_cfg->target_bss->bss_type);
		}
		return;
	}
#endif /* STA */
}

#ifdef WLRSDB
static void
wlc_csa_rsdb_clone_handler(void *ctx, rsdb_cfg_clone_upd_t *notif_data)
{
	wlc_bsscfg_t *cfg = notif_data->cfg;
	wlc_info_t *to_wlc = notif_data->to_wlc;
	wlc_csa_info_t *to_csam = to_wlc->csa;

	ASSERT(notif_data->cfg != NULL);

	BCM_REFERENCE(cfg);

	switch (notif_data->type) {
		case CFG_CLONE_END: {
			if (BSSCFG_STA(cfg) && (wlc_11h_get_spect_state(to_wlc->m11h, cfg)
				& NEED_TO_SWITCH_CHANNEL)) {
				WL_REGULATORY(("wl%d: Handle CSA clone callbck. Process pending"
					" STA channel switch\n", WLCWLUNIT(to_wlc)));
				wlc_11h_set_spect_state(to_wlc->m11h, cfg, NEED_TO_SWITCH_CHANNEL,
					0);
				wlc_csa_process_channel_switch(to_csam, cfg);
			}
			break;
		}
		default:
			break;
	}
}
#endif /* WLRSDB */

#ifdef STA
/* STA: Handle incoming Channel Switch Anouncement */
static void
wlc_csa_process_channel_switch(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = csam->wlc;
	wlc_bss_info_t *current_bss = cfg->current_bss;
	uint32 bi_us = (uint32)current_bss->beacon_period * DOT11_TU_TO_US;
	uint32 secs;
	wlcband_t *band;
	uint chanswitch_time;
	char chanbuf[CHANSPEC_STR_LEN];

	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);
	BCM_REFERENCE(chanbuf);

	ASSERT(csa != NULL);
	band = wlc->bandstate[CHSPEC_IS2G(csa->csa.chspec) ? BAND_2G_INDEX : BAND_5G_INDEX];

	/* add 160 and 80+80 */
	if (band && CHSPEC_IS8080(csa->csa.chspec) &&
	    (!VHT_ENAB_BAND(wlc->pub, band->bandtype) ||
	     (wlc_channel_locale_flags_in_band(wlc->cmi, band->bandunit) & WLC_NO_160MHZ) ||
	     !WL_BW_CAP_160MHZ(band->bw_cap))) {
		csa->csa.chspec = wf_chspec_primary80_chspec(csa->csa.chspec);
	}

	if (band && CHSPEC_IS160(csa->csa.chspec) &&
	    (!VHT_ENAB_BAND(wlc->pub, band->bandtype) ||
	     (wlc_channel_locale_flags_in_band(wlc->cmi, band->bandunit) & WLC_NO_160MHZ) ||
	     !WL_BW_CAP_160MHZ(band->bw_cap))) {
		csa->csa.chspec = wf_chspec_primary80_chspec(csa->csa.chspec);
	}

	/* Sanitize user setting for 80MHz against current settings
	* Reduce an 80MHz chanspec to 40MHz if needed.
	*/
	if (wf_chspec_malformed(csa->csa.chspec)) {
		WL_REGULATORY(("wl%d: %s: malformed csa channel ignore %x\n",
			wlc->pub->unit, __FUNCTION__, csa->csa.chspec));
		/* if malformed, below calls to change into lower bw won't work */
		goto ignore_csa;
	}

	if (CHSPEC_IS80(csa->csa.chspec) &&
		!VALID_80CHANSPEC(wlc, csa->csa.chspec)) {
		csa->csa.chspec = wf_chspec_primary40_chspec(csa->csa.chspec);
	}

	/* Convert a 40MHz AP channel to a 20MHz channel if we are not in NMODE or
	 * the locale does not allow 40MHz
	 * or the band is not configured for 40MHz operation
	 */
	if (band && CHSPEC_IS40(csa->csa.chspec) &&
	    (!N_ENAB(wlc->pub) ||
	     (wlc_channel_locale_flags_in_band(wlc->cmi, band->bandunit) & WLC_NO_40MHZ) ||
	     !WL_BW_CAP_40MHZ(band->bw_cap))) {
		csa->csa.chspec = CH20MHZ_CHSPEC(wf_chspec_ctlchan((csa->csa.chspec)));
	}

	if (!wlc_valid_chanspec_db(wlc->cmi, csa->csa.chspec)) {
		WL_REGULATORY(("wl%d: %s: Received invalid chanspec: %s\n",
			wlc->pub->unit, __FUNCTION__, wf_chspec_ntoa_ex(csa->csa.chspec, chanbuf)));
		/* Received bogus, malformed or otherwise screwed CSA.
		 * Err on the side of caution and goto an active A band channel now
		 */
		csa->csa.chspec = wlc_next_chanspec(wlc->cmi,
			CH20MHZ_CHSPEC((CH_MAX_2G_CHANNEL+1)), CHAN_TYPE_CHATTY, 0);
		if (csa->csa.chspec == wlc->home_chanspec)
			csa->csa.chspec =
				wlc_next_chanspec(wlc->cmi, csa->csa.chspec, CHAN_TYPE_CHATTY, 0);
		csa->csa.count = 1;

		if (csa->csa.chspec == INVCHANSPEC) {
			WL_REGULATORY(("wl%d: %s: compute new channel failed\n",
				wlc->pub->unit, __FUNCTION__));
			/* exhausted options to accomodate csa - ignore as it is bogus */
			goto ignore_csa;
		}
	}
	if ((csa->csa.chspec == WLC_BAND_PI_RADIO_CHANSPEC) &&
		(csa->csa.chspec == current_bss->chanspec)) {
		WL_REGULATORY(("wl%d: %s: Already on chan 0x%x, skip switching\n",
			wlc->pub->unit, __FUNCTION__, csa->csa.chspec));
		goto ignore_csa;
	}
	secs = (bi_us * csa->csa.count)/1000000;

	if (csa->csa.mode && WL11H_ENAB(wlc) && !CHSPEC_IS2G(wlc->home_chanspec)) {
		wlc_set_quiet_chanspec(wlc->cmi, wlc->home_chanspec);
		if (WLC_BAND_PI_RADIO_CHANSPEC == wlc->home_chanspec) {
			WL_REGULATORY(("%s: Muting now\n", __FUNCTION__));
			wlc_mute(wlc, ON, 0);
		}
	}

	if (csa->channel_sw.secs != 0) {
		WL_REGULATORY(("%s: ignoring csa: mode %d, chanspec %s, count %d, secs %d\n",
			__FUNCTION__, csa->csa.mode, wf_chspec_ntoa_ex(csa->csa.chspec, chanbuf),
			csa->csa.count, secs));
		return;
	}

	wlc_csa_obss_dynbw_notif_cb_notif(csam, cfg,
		BCME_OK, CSA_CHANNEL_CHANGE_START, csa->csa.chspec);

	/* Are we already on the required channel ? */
	if (csa->csa.chspec == cfg->current_bss->chanspec) {
		goto ignore_csa;
	}

#ifdef WLRSDB
	if (RSDB_ENAB(wlc->pub) && switch_now &&
		wlc_rsdb_handle_sta_csa(wlc->rsdbinfo, cfg, csa->csa.chspec)) {
		/* It seems that channel switch needs a clone to other wlc.
		* Complete cloning and handle the channel switch in the RSDB callback.
		*/
		WL_REGULATORY(("wl%d.%d Defer channel switch on this wlc .. "
			"try on wlc%d after clone..\n", WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg),
			WLCWLUNIT(wlc_rsdb_get_other_wlc(wlc))));
		wlc_11h_set_spect_state(wlc->m11h, cfg, NEED_TO_SWITCH_CHANNEL,
			NEED_TO_SWITCH_CHANNEL);
		return;
	}
#endif /* WLRSDB */

	wlc_sync_txfifo(wlc, cfg->wlcif->qi, BITMAP_SYNC_ALL_TX_FIFOS, FLUSHFIFO);

	WL_REGULATORY(("wl%d.%d: %s: Recved CSA: mode %d, chanspec %s, count %d, secs %d\n",
	               wlc->pub->unit, cfg->_idx, __FUNCTION__,
	               csa->csa.mode, wf_chspec_ntoa_ex(csa->csa.chspec, chanbuf), csa->csa.count,
	               secs));

	if (csa->csa.count <= 1) {
		wl_del_timer(wlc->wl, csa->csa_timer);
		WL_REGULATORY(("%s: switch is only %d secs, switching now\n", __FUNCTION__, secs));
		csa->csa.count = 0;
		wlc_do_chanswitch(cfg, csa->csa.chspec);

		/* Send Channel Switch Indication to upper (OS) layer. */
		wlc_bss_mac_event(wlc, cfg, WLC_E_CSA_COMPLETE_IND, NULL,
			WLC_E_STATUS_SUCCESS, 0, 0, &csa->csa.mode, sizeof(csa->csa.mode));
			csa->csa.count = 0;
			csa->csa.mode = DOT11_CSA_MODE_ADVISORY;
		wlc_csa_train_txpower(csam, cfg);
		wlc_csa_obss_dynbw_notif_cb_notif(csam, cfg,
			BCME_OK, CSA_CHANNEL_CHANGE_END, csa->csa.chspec);
		return;
	}

	WL_REGULATORY(("%s: Scheduling channel switch in %d tbtts\n", __FUNCTION__,
	               csa->csa.count));

	/* Now we only use cfg->channel_sw.secs as a flag */
	csa->channel_sw.secs = secs;
	csa->channel_sw.chanspec = csa->csa.chspec;
#ifdef RADAR
	/* If AP indicates switching to radar channel, take AP CAC period
	 * into account before switching channel. If roam_off is set STA
	 * will never try to associate to upstream AP.
	 */
	if (RADAR_ENAB(wlc->pub) && BSSCFG_AP(cfg) &&
		wlc_radar_chanspec(wlc->cmi, csa->csa.chspec) == TRUE) {
		uint cactime = wlc_is_european_weather_radar_channel(wlc,
			csa->csa.chspec) ? WLC_DFS_CAC_TIME_SEC_DEF_EUWR:
			WLC_DFS_CAC_TIME_SEC_DEFAULT;
		WL_REGULATORY(("%s: Scheduled to switch to radar channel, wait for additional"
			"CAC time(%d) sec\n", __FUNCTION__, cactime));
		chanswitch_time = ((cactime + 5) * 1000000)/DOT11_TU_TO_US
				+ (csa->csa.count * current_bss->beacon_period);
	} else
#endif /* RADAR */
	if (PSTA_ENAB(wlc->pub)) {
		chanswitch_time =
			(current_bss->beacon_period * (csa->csa.count - 1) * DOT11_TU_TO_US) / 1000;
	} else {
		chanswitch_time = csa->csa.count * current_bss->beacon_period;
	}
	wlc_mute(wlc, ON, PHY_MUTE_ALL);
	wlc_11h_set_spect_state(wlc->m11h, cfg, NEED_TO_SWITCH_CHANNEL, NEED_TO_SWITCH_CHANNEL);
	wl_del_timer(wlc->wl, csa->csa_timer);
	wl_add_timer(wlc->wl, csa->csa_timer, chanswitch_time, 0);

#ifdef AP
	/* If this STA connected to up-stream AP and has an overlaping chanspec
	 * with a local AP, propagate csa to downstream STA's (for local AP).
	 */
	if (
#ifdef WLMCHAN
		(!MCHAN_ENAB(wlc->pub) || wlc_mchan_stago_is_disabled(wlc->mchan)) &&
#endif
		WL11H_AP_ENAB(wlc))
	{
		wlc_bsscfg_t *ap_bsscfg;
		int idx;
		FOREACH_UP_AP(wlc, idx, ap_bsscfg) {

			if (ap_bsscfg->current_bss->chanspec == cfg->current_bss->chanspec) {
				wlc_csa_t *apcsa = CSA_BSSCFG_CUBBY(csam, ap_bsscfg);

				/* copy over the csa info */
				bcopy(&csa->csa, &apcsa->csa, sizeof(wl_chan_switch_t));
				/* decrement by 1 since this is for our next beacon */
				apcsa->csa.count--;
				/* send csa action frame and update bcn, prb rsp */
				wlc_csa_apply_channel_switch(csam, ap_bsscfg);
			}
		}
	}
#endif /* AP */
	return;
ignore_csa:
	csa->csa.count = 0;
	return;
}
#endif /* STA */

#ifdef WL11AC
/* return bw number given channel width
 */
static uint
channel_width_to_bw(uint8 channel_width)
{
	uint bw;

	if (channel_width == VHT_OP_CHAN_WIDTH_80)
		bw = WL_CHANSPEC_BW_80;
	else if (channel_width == VHT_OP_CHAN_WIDTH_160)
		bw = WL_CHANSPEC_BW_160;
	else if (channel_width == VHT_OP_CHAN_WIDTH_80_80)
		bw = WL_CHANSPEC_BW_8080;
	else
		bw = WL_CHANSPEC_BW_40;

	return bw;
}

/* Given the band and control channel construct the vht chanspec */
static chanspec_t
wlc_get_vht_chanspec(wlc_info_t *wlc, uint8 ctl_chan, uint bw)
{
	chanspec_t chanspec = 0;

	BCM_REFERENCE(wlc);

	chanspec = wf_channel2chspec(ctl_chan, bw);
	WL_REGULATORY(("wl%d: %s: chanspec = 0x%x\n", wlc->pub->unit, __FUNCTION__, chanspec));

	return chanspec;
}
#endif /* WL11AC */


#ifdef WL11N
void
wlc_recv_public_csa_action(wlc_csa_info_t *csam, struct dot11_management_header *hdr,
	uint8 *body, int body_len)
{
	wlc_info_t *wlc = csam->wlc;
	wlc_bsscfg_t *cfg;
	struct dot11y_action_ext_csa *action_hdr;
	uint8 extch;
	wlc_csa_t *csa;
#ifdef WL11AC
	dot11_wide_bw_chan_switch_ie_t *wide_bw_chan_switch_ie;
	bcm_tlv_t *ies;
	uint ies_len;
	uint bw = WL_CHANSPEC_BW_20;
#endif /* WL11AC */

	if ((cfg = wlc_bsscfg_find_by_bssid(wlc, &hdr->bssid)) == NULL &&
	    (cfg = wlc_bsscfg_find_by_hwaddr(wlc, &hdr->da)) == NULL) {
		char eabuf[ETHER_ADDR_STR_LEN];
		BCM_REFERENCE(eabuf);
		WL_ERROR(("wl%d: %s: Unable to find bsscfg for %s\n",
		          wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&hdr->sa, eabuf)));
		return;
	}

	if (!BSSCFG_STA(cfg)) {
		WL_ERROR(("wl%d.%d: %s: not a STA\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));
		return;
	}

	if (body_len < (int)(sizeof(struct dot11y_action_ext_csa))) {
		WL_ERROR(("wl%d: %s: Invalid len %d < %d\n",	wlc->pub->unit, __FUNCTION__,
			body_len, (int)(TLV_HDR_LEN + sizeof(struct dot11_csa_body))));
		return;
	}

	action_hdr = (struct dot11y_action_ext_csa *)body;
	/* valid the IE in this action frame */
	WL_INFORM(("wl%d: %s: mode %d, reg %d, channel %d, count %d\n",
		wlc->pub->unit, __FUNCTION__, action_hdr->b.mode, action_hdr->b.reg,
		action_hdr->b.channel, action_hdr->b.count));

	csa = CSA_BSSCFG_CUBBY(csam, cfg);
	ASSERT(csa != NULL);

	csa->csa.mode = action_hdr->b.mode;
	csa->csa.count = action_hdr->b.count;
#ifdef WL11AC
	if (body_len > (int)(sizeof(struct dot11y_action_ext_csa))) {
		ies = (bcm_tlv_t*) (body + (sizeof(struct dot11y_action_ext_csa)));
		ies_len = body_len - (sizeof(struct dot11y_action_ext_csa));
		wide_bw_chan_switch_ie = (dot11_wide_bw_chan_switch_ie_t*)
			bcm_parse_tlvs_min_bodylen(ies, ies_len,
			DOT11_MNG_WIDE_BW_CHANNEL_SWITCH_ID,
			DOT11_WIDE_BW_SWITCH_IE_LEN);

		if (wide_bw_chan_switch_ie == NULL) {
			WL_REGULATORY(("wl%d: %s: Bad CSA Mngmt Action frame"
			               "(Wide Bandwidth Channel Switch IE)\n",
			               wlc->pub->unit, __FUNCTION__));
			WLCNTINCR(wlc->pub->_cnt->rxbadproto);
			wlc_send_action_err(wlc, hdr, body, body_len);
			return;
		}
		WL_REGULATORY(("wl%d: %s: channel width %d, control channel %d\n", wlc->pub->unit,
		  __FUNCTION__, wide_bw_chan_switch_ie->channel_width, action_hdr->b.channel));
		bw = channel_width_to_bw(wide_bw_chan_switch_ie->channel_width);
		csa->csa.chspec = wlc_get_vht_chanspec(wlc, action_hdr->b.channel, bw);
		BCM_REFERENCE(extch);
	} else
#endif /* WL11AC */
	{
		extch = wlc_rclass_extch_get(wlc->cmi, action_hdr->b.reg);
		csa->csa.chspec = wlc_ht_chanspec(wlc, action_hdr->b.channel, extch, cfg);
	}

#ifdef WL_EVENT_LOG_COMPILE
	{
		wl_event_log_tlv_hdr_t tlv_log = {{0, 0}};
		uint32 tlv_log_tmp = 0;
		wl_event_log_csa_t sts = {{0, 0, 0}};

		sts.ch = csa->csa.chspec;
		sts.count = csa->csa.count;
		sts.mode = csa->csa.mode;

		tlv_log.tag = TRACE_TAG_VENDOR_SPECIFIC;
		tlv_log.length = sizeof(wl_event_log_csa_t);
		tlv_log_tmp = tlv_log.t;

		tlv_log.tag = TRACE_TAG_CHANNEL_SPEC;
		tlv_log.length = sizeof(uint16);

		WL_EVENT_LOG((EVENT_LOG_TAG_TRACE_WL_INFO, WLC_E_CSA_START_IND,
			tlv_log_tmp, sts.t, tlv_log.t, csa->csa.chspec));
	}
#endif /* WL_EVENT_LOG_COMPILE */

	csa->csa.reg = action_hdr->b.reg;

#ifdef STA
	if (cfg->associated &&
	    bcmp(&hdr->bssid, &cfg->BSSID, ETHER_ADDR_LEN) == 0) {
		wlc_csa_process_channel_switch(csam, cfg);
	}
#ifdef PSTA
	{
	int32 idx;
	wl_chan_switch_t pcsa;
	pcsa = csa->csa;
	/* Process public csa action for each associated proxysta bsscfg */
	FOREACH_PSTA(wlc, idx, cfg) {
		if (cfg->associated &&
			bcmp(&hdr->bssid, &cfg->BSSID, ETHER_ADDR_LEN) == 0) {
			CSA_BSSCFG_CUBBY(csam, cfg)->csa = pcsa;
			wlc_csa_process_channel_switch(csam, cfg);
		}
	}
	}
#endif /* PSTA */
#endif /* STA */
}
#endif /* WL11N */

static uint8 *
wlc_write_csa_body(wl_chan_switch_t *cs, uint8 *cp)
{
	struct dot11_csa_body *b = (struct dot11_csa_body *)cp;

	b->mode = cs->mode;
	b->reg = cs->reg;
	b->channel = wf_chspec_ctlchan(cs->chspec);
	b->count = cs->count;
	cp += sizeof(struct dot11_csa_body);

	return cp;
}

static uint8 *
wlc_write_ext_csa_ie(wl_chan_switch_t *cs, uint8 *cp)
{
	dot11_ext_csa_ie_t *chan_switch_ie = (dot11_ext_csa_ie_t *)cp;

	chan_switch_ie->id = DOT11_MNG_EXT_CSA_ID;
	chan_switch_ie->len = DOT11_EXT_CSA_IE_LEN;
	cp += TLV_HDR_LEN;
	cp = wlc_write_csa_body(cs, cp);

	return cp;
}

static uint8 *
wlc_write_csa_ie(wl_chan_switch_t *cs, uint8 *cp, int buflen)
{
	dot11_chan_switch_ie_t *chan_switch_ie;

	/* perform buffer length check. */
	/* if not big enough, return buffer untouched */
	BUFLEN_CHECK_AND_RETURN((TLV_HDR_LEN + DOT11_SWITCH_IE_LEN), buflen, cp);

	chan_switch_ie = (dot11_chan_switch_ie_t *)cp;
	chan_switch_ie->id = DOT11_MNG_CHANNEL_SWITCH_ID;
	chan_switch_ie->len = DOT11_SWITCH_IE_LEN;
	chan_switch_ie->mode = cs->mode;
	chan_switch_ie->channel = wf_chspec_ctlchan(cs->chspec);
	chan_switch_ie->count = cs->count;
	cp += (TLV_HDR_LEN + DOT11_SWITCH_IE_LEN);

	return cp;
}

#ifdef WL11AC
#ifdef AP
static uint
wlc_calc_chan_switch_wrapper_ie_len(wlc_csa_info_t *csam, wl_chan_switch_t *cs,
	wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = csam->wlc;
	wlc_iem_ft_cbparm_t ftcbparm;
	wlc_iem_cbparm_t cbparm;

	/* prepare IE mgmt calls */
	bzero(&ftcbparm, sizeof(ftcbparm));
	ftcbparm.csw.chspec = cs->chspec;
	bzero(&cbparm, sizeof(cbparm));
	cbparm.ft = &ftcbparm;

	/* calc IEs' length */
	return TLV_HDR_LEN + wlc_ier_calc_len(wlc->ier_csw, cfg, 0, &cbparm);
}
#endif /* AP */

static uint8 *
wlc_write_chan_switch_wrapper_ie(wlc_csa_info_t *csam, wl_chan_switch_t *cs,
	wlc_bsscfg_t *cfg, uint8 *cp, int buflen)
{
	wlc_info_t *wlc = csam->wlc;
	dot11_chan_switch_wrapper_ie_t *chan_switch_wrapper;
	uint cp_len;
	wlc_iem_ft_cbparm_t ftcbparm;
	wlc_iem_cbparm_t cbparm;

	/* prepare IE mgmt calls */
	bzero(&ftcbparm, sizeof(ftcbparm));
	ftcbparm.csw.chspec = cs->chspec;
	bzero(&cbparm, sizeof(cbparm));
	cbparm.ft = &ftcbparm;

	/* calc IEs' length */
	cp_len = wlc_ier_calc_len(wlc->ier_csw, cfg, 0, &cbparm);
	if (TLV_HDR_LEN + cp_len >= (uint)buflen)
		return cp;

	chan_switch_wrapper = (dot11_chan_switch_wrapper_ie_t *)cp;
	chan_switch_wrapper->id = DOT11_MNG_CHANNEL_SWITCH_WRAPPER_ID;
	chan_switch_wrapper->len = (uint8)cp_len;

	cp += TLV_HDR_LEN;

	/* build IEs */
	if (wlc_ier_build_frame(wlc->ier_csw, cfg, WLC_IEM_FC_IER,
	                        &cbparm, cp, cp_len) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_build_frame failed\n",
		          wlc->pub->unit, __FUNCTION__));
		return cp;
	}

	return cp + cp_len;
}


uint8 *
wlc_csa_write_chan_switch_wrapper_ie(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg, uint8 *cp, int buflen)
{
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);
	ASSERT(csa != NULL);

	return wlc_write_chan_switch_wrapper_ie(csam, &csa->csa, cfg, cp, buflen);
}
#endif /* WL11AC */

static uint8 *
wlc_write_extch_ie(chanspec_t chspec, uint8 *cp, int buflen)
{
	dot11_extch_ie_t *extch_ie = (dot11_extch_ie_t *)cp;
	chanspec_t chspec40;

	/* length check */
	/* if buffer too small, return untouched buffer */
	BUFLEN_CHECK_AND_RETURN((TLV_HDR_LEN + DOT11_EXTCH_IE_LEN), buflen, cp);

	extch_ie->id = DOT11_MNG_EXT_CHANNEL_OFFSET;
	extch_ie->len = DOT11_EXTCH_IE_LEN;

	if (CHSPEC_IS8080(chspec) || CHSPEC_IS160(chspec)) {
		chspec40 = wf_chspec_primary80_chspec(chspec);
		chspec40 = wf_chspec_primary40_chspec(chspec);
		extch_ie->extch = CHSPEC_SB_UPPER(chspec40) ?
		                  DOT11_EXT_CH_LOWER : DOT11_EXT_CH_UPPER;
	}
	else if (CHSPEC_IS80(chspec)) {
		chspec40 = wf_chspec_primary40_chspec(chspec);
		extch_ie->extch = CHSPEC_SB_UPPER(chspec40) ?
		                  DOT11_EXT_CH_LOWER : DOT11_EXT_CH_UPPER;
	} else if (CHSPEC_IS40(chspec)) {
		extch_ie->extch = CHSPEC_SB_UPPER(chspec) ? DOT11_EXT_CH_LOWER : DOT11_EXT_CH_UPPER;
	} else {
		ASSERT(CHSPEC_IS20(chspec));
		extch_ie->extch = DOT11_EXT_CH_NONE;
	}
	cp += (TLV_HDR_LEN + DOT11_EXTCH_IE_LEN);

	return cp;
}

#ifdef WL11AC
static uint8 *
wlc_write_wide_bw_csa_ie(wl_chan_switch_t *cs, uint8 *cp, int buflen)
{
	dot11_wide_bw_chan_switch_ie_t *wide_bw_chan_switch_ie;
	uint8 center_chan;

	/* perform buffer length check. */
	/* if not big enough, return buffer untouched */
	BUFLEN_CHECK_AND_RETURN((TLV_HDR_LEN + DOT11_WIDE_BW_SWITCH_IE_LEN), buflen, cp);

	wide_bw_chan_switch_ie = (dot11_wide_bw_chan_switch_ie_t *)cp;
	wide_bw_chan_switch_ie->id = DOT11_MNG_WIDE_BW_CHANNEL_SWITCH_ID;
	wide_bw_chan_switch_ie->len = DOT11_WIDE_BW_SWITCH_IE_LEN;

	if (CHSPEC_IS80(cs->chspec))
		wide_bw_chan_switch_ie->channel_width = VHT_OP_CHAN_WIDTH_80;
	else if (CHSPEC_IS160(cs->chspec))
		wide_bw_chan_switch_ie->channel_width = VHT_OP_CHAN_WIDTH_160;
	else if (CHSPEC_IS8080(cs->chspec))
		wide_bw_chan_switch_ie->channel_width = VHT_OP_CHAN_WIDTH_80_80;
	else
		wide_bw_chan_switch_ie->channel_width = VHT_OP_CHAN_WIDTH_20_40;

	if (CHSPEC_IS8080(cs->chspec)) {
		wide_bw_chan_switch_ie->center_frequency_segment_0 =
			wf_chspec_primary80_channel(cs->chspec);
		wide_bw_chan_switch_ie->center_frequency_segment_1 =
			wf_chspec_secondary80_channel(cs->chspec);
	}
	else {
		center_chan = CHSPEC_CHANNEL(cs->chspec) >> WL_CHANSPEC_CHAN_SHIFT;
		wide_bw_chan_switch_ie->center_frequency_segment_0 = center_chan;
		wide_bw_chan_switch_ie->center_frequency_segment_1 = 0;
	}

	cp += (TLV_HDR_LEN + DOT11_WIDE_BW_SWITCH_IE_LEN);

	return cp;
}
#endif /* WL11AC */

#ifdef AP
#ifdef WL11AC
/* Wide Bandwidth IE in CS Wrapper IE */
static uint
wlc_csa_csw_calc_wide_bw_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	wlc_iem_ft_cbparm_t *ftcbparm = calc->cbparm->ft;

	BCM_REFERENCE(ftcbparm);

	BCM_REFERENCE(ctx);

	/* wb_csa_ie doesn't present in 20MHz channels */
	if (!CHSPEC_IS20(ftcbparm->csw.chspec))
		return TLV_HDR_LEN + DOT11_WIDE_BW_SWITCH_IE_LEN;

	return 0;
}

static int
wlc_csa_csw_write_wide_bw_ie(void *ctx, wlc_iem_build_data_t *build)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_csa_t *csa;
	wlc_iem_ft_cbparm_t *ftcbparm = build->cbparm->ft;

	csa = CSA_BSSCFG_CUBBY(csam, build->cfg);
	ASSERT(csa != NULL);

	BCM_REFERENCE(ftcbparm);

	/* wb_csa_ie doesn't present in 20MHz channels */
	if (!CHSPEC_IS20(ftcbparm->csw.chspec))
		wlc_write_wide_bw_csa_ie(&csa->csa, build->buf, build->buf_len);

	return BCME_OK;
}

/* CS wrapper */
static uint
wlc_csa_calc_csw_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_info_t *wlc = csam->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_csa_t *csa;

	if (!data->cbparm->vht)
		return 0;

	if (BAND_2G(wlc->band->bandtype))
		return 0;

	if (!(BSSCFG_AP(cfg) &&
	      wlc_csa_get_csa_count(csam, cfg) > 0))
		return 0;

	csa = CSA_BSSCFG_CUBBY(csam, cfg);
	ASSERT(csa != NULL);

	return wlc_calc_chan_switch_wrapper_ie_len(csam, &csa->csa, cfg);
}

static int
wlc_csa_write_csw_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_info_t *wlc = csam->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_csa_t *csa;

	if (!data->cbparm->vht)
		return BCME_OK;

	if (BAND_2G(wlc->band->bandtype))
		return BCME_OK;

	if (!(BSSCFG_AP(cfg) &&
	      wlc_csa_get_csa_count(csam, cfg) > 0))
		return BCME_OK;

	csa = CSA_BSSCFG_CUBBY(csam, cfg);
	ASSERT(csa != NULL);

	wlc_write_chan_switch_wrapper_ie(csam, &csa->csa, cfg, data->buf, data->buf_len);
	return BCME_OK;
}
#endif /* WL11AC */

/* Extended CSA IE */
static uint
wlc_csa_calc_ext_csa_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;

	if (data->cbparm->ht &&
	    BSSCFG_AP(cfg) &&
	    wlc_csa_get_csa_count(csam, cfg) > 0)
		return TLV_HDR_LEN + DOT11_EXT_CSA_IE_LEN;

	return 0;
}

static int
wlc_csa_write_ext_csa_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;

	if (data->cbparm->ht &&
	    BSSCFG_AP(cfg) &&
	    wlc_csa_get_csa_count(csam, cfg) > 0) {
		wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);
		ASSERT(csa != NULL);
		wlc_write_ext_csa_ie(&csa->csa, data->buf);
	}

	return BCME_OK;
}
#endif /* AP */

#ifdef STA
static bool
wlc_parse_csa_ie(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg, uint8 *params, int len)
{
	wlc_info_t *wlc = cfg->wlc;
	bcm_tlv_t *tag = (bcm_tlv_t*)params;
	uint8 *end = params + len;
	uint8 extch = DOT11_EXT_CH_NONE;
	uint8 type;
	bool csa_ie_found = FALSE;
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);
#ifdef WL11AC
	uint bw = WL_CHANSPEC_BW_20;
	dot11_wide_bw_chan_switch_ie_t *wide_bw_ie = NULL;
#endif /* WL11AC */

	ASSERT(csa != NULL);

	/* 11n & 11y csa takes precedence */
	if ((params < end) &&
	    (tag = bcm_parse_tlvs(params, len, DOT11_MNG_EXT_CSA_ID)) != NULL) {
		dot11_ext_csa_ie_t *csa_ie = (dot11_ext_csa_ie_t *)tag;
		if (tag->len >= DOT11_EXT_CSA_IE_LEN) {
			csa_ie_found = TRUE;
			csa->csa.mode = csa_ie->b.mode;
			csa->csa.count = csa_ie->b.count;
#ifdef WL11AC
			/* 11ac csa takes precedence */
			/* Look for the Channel Switch Wrapper IE for additional CSA info */
			tag = bcm_parse_tlvs(params, len, DOT11_MNG_CHANNEL_SWITCH_WRAPPER_ID);

			/* search for the Wide Bandwidth Channel Switch sub-element for
			 * wide channel width info
			 */
			if (tag != NULL) {
				wide_bw_ie = (dot11_wide_bw_chan_switch_ie_t *)
					bcm_parse_tlvs_min_bodylen(tag->data, tag->len,
					DOT11_MNG_WIDE_BW_CHANNEL_SWITCH_ID,
					DOT11_WIDE_BW_SWITCH_IE_LEN);
			}

			/* determine the wide bw chanspec if the wide_bw_ie
			 * was found (and valid len)
			 */
			if (wide_bw_ie != NULL) {
				bw = channel_width_to_bw(wide_bw_ie->channel_width);
				csa->csa.chspec = wlc_get_vht_chanspec(wlc, csa_ie->b.channel, bw);
			}
			else
#endif /* WL11AC */
			{
				extch = wlc_rclass_extch_get(wlc->cmi, csa_ie->b.reg);
				csa->csa.chspec = wlc_ht_chanspec(wlc, csa_ie->b.channel, extch,
					cfg);
			}
			csa->csa.reg = csa_ie->b.reg;
		} else {
			WL_REGULATORY(("wl%d: %s: CSA IE length != 4\n",
				wlc->pub->unit, __FUNCTION__));
		}
	}

	if (!csa_ie_found && (params < end) &&
	    (tag = bcm_parse_tlvs(params, len, DOT11_MNG_CHANNEL_SWITCH_ID)) != NULL) {
		bool err = FALSE;
		dot11_chan_switch_ie_t *csa_ie = (dot11_chan_switch_ie_t *)tag;
		/* look for brcm extch IE first, if exist, use it,
		 * otherwise look for IEEE extch IE
		 */
		if (tag->len < DOT11_SWITCH_IE_LEN)
			err = TRUE;

		csa_ie_found = TRUE;
		type = BRCM_EXTCH_IE_TYPE;
		tag = (bcm_tlv_t *)bcm_find_vendor_ie((uchar *)tag, (uint)(end-(uint8 *)tag),
			BRCM_PROP_OUI, &type, sizeof(type));
		if (tag && tag->len == BRCM_EXTCH_IE_LEN)
			extch = ((dot11_brcm_extch_ie_t *)tag)->extch;
		else {
			tag = bcm_parse_tlvs(params, len, DOT11_MNG_EXT_CHANNEL_OFFSET);
			if (tag) {
				if (tag->len >= DOT11_EXTCH_IE_LEN) {
					extch = ((dot11_extch_ie_t *)tag)->extch;
					if (extch != DOT11_EXT_CH_LOWER &&
					    extch != DOT11_EXT_CH_UPPER &&
					    extch != DOT11_EXT_CH_NONE)
						extch = DOT11_EXT_CH_NONE;
				} else {
					WL_ERROR(("wl%d: wlc_parse_11h: extension channel offset"
						" len %d length too short\n",
						wlc->pub->unit, tag->len));
					csa_ie_found = FALSE;
					err = TRUE;
				}
			}
		}

		if (!err) {
			csa->csa.mode = csa_ie->mode;
			csa->csa.count = csa_ie->count;
			csa->csa.chspec = wlc_ht_chanspec(wlc, csa_ie->channel, extch, cfg);
			csa->csa.reg = 0;
		}
	}

	/* Should reset the csa.count back to zero if csa_ie not found */
	if (!csa_ie_found && csa->csa.count) {
		WL_REGULATORY(("wl%d.%d: %s: no csa ie found, reset csa.count %d to 0\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, csa->csa.count));
		csa->csa.count = 0;
	}

	if (csa_ie_found)
		WL_REGULATORY(("%s: Found a CSA, count = %d\n", __FUNCTION__, csa->csa.count));

	return (csa_ie_found);
}

static int
wlc_csa_bcn_parse_ext_csa_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;

	if (wlc_parse_csa_ie(csam, cfg, data->buf, data->buf_len)) {
		wlc_csa_process_channel_switch(csam, cfg);
	}

	return BCME_OK;
}
#endif /* STA */

#ifdef AP
/* 802.11h Channel Switch Announcement */
static uint
wlc_csa_calc_csa_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_info_t *wlc = csam->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	if (WL11H_ENAB(wlc) && BSSCFG_AP(cfg) &&
	    wlc_csa_get_csa_count(csam, cfg) > 0)
		return TLV_HDR_LEN + DOT11_SWITCH_IE_LEN;

	return 0;
}

static int
wlc_csa_write_csa_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_info_t *wlc = csam->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	if (WL11H_ENAB(wlc) && BSSCFG_AP(cfg) &&
	    wlc_csa_get_csa_count(csam, cfg) > 0) {
		wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);
		ASSERT(csa != NULL);
		wlc_write_csa_ie(&csa->csa, data->buf, data->buf_len);
	}

	return BCME_OK;
}
#endif /* AP */

#ifdef STA
static int
wlc_csa_bcn_parse_csa_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;

	if (wlc_parse_csa_ie(csam, cfg, data->buf, data->buf_len)) {
		wlc_csa_process_channel_switch(csam, cfg);
	}

	return BCME_OK;
}
#endif /* STA */

void
wlc_recv_csa_action(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len)
{
	wlc_info_t *wlc = csam->wlc;
	struct dot11_action_frmhdr *action_hdr;
	dot11_chan_switch_ie_t *csa_ie;
	bcm_tlv_t *ext_ie;
	bcm_tlv_t *ies;
	uint ies_len;
	uint8 extch = DOT11_EXT_CH_NONE;
#if defined(BCMDBG) && defined(AP) || defined(CLIENT_CSA)
#ifdef WL11AC
	bcm_tlv_t *wide_bw_ie;
	uint8 channel_width = VHT_OP_CHAN_WIDTH_20_40;
	uint bw = WL_CHANSPEC_BW_20;
#endif /* WL11AC */
#endif /* BCMDBG && AP || CLIENT_CSA */

	ASSERT(cfg != NULL);

	action_hdr = (struct dot11_action_frmhdr *)body;

	ies = (bcm_tlv_t*)action_hdr->data;
	ies_len = body_len - DOT11_ACTION_HDR_LEN;

	csa_ie = (dot11_chan_switch_ie_t*)
		bcm_parse_tlvs_min_bodylen(ies, ies_len,
		DOT11_MNG_CHANNEL_SWITCH_ID,
		DOT11_SWITCH_IE_LEN);

	if (csa_ie == NULL) {
		WL_REGULATORY(("wl%d:%s: Bad CSA Spectrum Mngmt Action frame\n",
		               wlc->pub->unit, __FUNCTION__));
		WLCNTINCR(wlc->pub->_cnt->rxbadproto);
		wlc_send_action_err(wlc, hdr, body, body_len);
		return;
	}

	/* check if we have an extension channel ie */
	if (N_ENAB(wlc->pub)) {
		/* Check for 11n spec IE first */
		ext_ie = bcm_parse_tlvs(ies, ies_len, DOT11_MNG_EXT_CHANNEL_OFFSET);
		if (ext_ie != NULL &&
		    ext_ie->len == DOT11_EXTCH_IE_LEN) {
			extch = ((dot11_extch_ie_t *)ext_ie)->extch;
		} else {
			uint8 extch_subtype = BRCM_EXTCH_IE_TYPE;

			/* Check for BRCM OUI format */
			ext_ie = bcm_find_vendor_ie(ies, ies_len, BRCM_PROP_OUI,
			                            &extch_subtype, 1);
			if (ext_ie != NULL &&
			    ext_ie->len == BRCM_EXTCH_IE_LEN) {
				extch = ((dot11_brcm_extch_ie_t *)ext_ie)->extch;
			}
		}
	}
#ifdef WL_EVENT_LOG_COMPILE
	{
		wl_event_log_tlv_hdr_t tlv_log = {{0, 0}};
		wl_event_log_csa_t sts = {{0, 0, 0}};

		sts.ch = csa_ie->channel;
		sts.count =  csa_ie->count;
		sts.mode = csa_ie->mode;

		tlv_log.tag = TRACE_TAG_VENDOR_SPECIFIC;
		tlv_log.length = sizeof(wl_event_log_csa_t);

		WL_EVENT_LOG((EVENT_LOG_TAG_TRACE_WL_INFO, WLC_E_CSA_START_IND,
			tlv_log.t, sts.t));
	}
#endif /* WL_EVENT_LOG_COMPILE */
#if defined(BCMDBG) && defined(AP) || defined(CLIENT_CSA)
#ifdef WL11AC
	/* check if we have an wide bandwidth channel switch ie */
	if (VHT_ENAB(wlc->pub)) {
		/* Check for 11ac spec IE */
		wide_bw_ie = bcm_parse_tlvs_min_bodylen(ies, ies_len,
			DOT11_MNG_WIDE_BW_CHANNEL_SWITCH_ID, DOT11_WIDE_BW_SWITCH_IE_LEN);
		if (wide_bw_ie != NULL) {
			channel_width = ((dot11_wide_bw_chan_switch_ie_t *)
			                 wide_bw_ie)->channel_width;
			bw = channel_width_to_bw(((dot11_wide_bw_chan_switch_ie_t *)
			                 wide_bw_ie)->channel_width);
			WL_REGULATORY(("wl%d: wlc_recv_csa_action: mode %d, channel %d, count %d,"
			   "channel width %d\n", wlc->pub->unit, csa_ie->mode,
			   csa_ie->channel, csa_ie->count, channel_width));
			}
	}
#endif /* WL11AC */
#endif /* BCMDBG && AP || CLIENT_CSA */

	WL_REGULATORY(("wl%d: wlc_recv_csa_action: mode %d, channel %d, count %d, extension %d\n",
	               wlc->pub->unit, csa_ie->mode, csa_ie->channel, csa_ie->count, extch));
	BCM_REFERENCE(extch);

#if defined(BCMDBG) && defined(AP) || defined(CLIENT_CSA)
	if (BSSCFG_AP(cfg)) {
		chanspec_t chspec;
#ifdef WL11AC
		if (channel_width != VHT_OP_CHAN_WIDTH_20_40) {
			chspec = wlc_get_vht_chanspec(wlc, csa_ie->channel, bw);
		} else
#endif /* WL11AC */
		if (extch == DOT11_EXT_CH_NONE) {
			chspec = CH20MHZ_CHSPEC(csa_ie->channel);
		} else if (extch == DOT11_EXT_CH_UPPER) {
			chspec = CH40MHZ_CHSPEC(csa_ie->channel, WL_CHANSPEC_CTL_SB_UPPER);
		} else {
			chspec = CH40MHZ_CHSPEC(csa_ie->channel, WL_CHANSPEC_CTL_SB_LOWER);
		}

		wlc_csa_do_channel_switch(csam, cfg, chspec, csa_ie->mode, csa_ie->count, 0,
			CSA_BROADCAST_ACTION_FRAME);
	}
#endif /* BCMDBG && AP || CLIENT_CSA */

	return;
}

void
wlc_recv_ext_csa_action(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len)
{
	wlc_info_t *wlc = csam->wlc;
	struct dot11_action_ext_csa *action_hdr;
	dot11_ext_csa_ie_t *req_ie;

	BCM_REFERENCE(hdr);

	(void)wlc;

	ASSERT(cfg != NULL);

	action_hdr = (struct dot11_action_ext_csa *)body;
	req_ie = &action_hdr->chan_switch_ie;

	/* valid the IE in this action frame */
	if (N_ENAB(wlc->pub) &&
	    body_len >= (int)(sizeof(struct dot11_action_ext_csa))) {
		if (req_ie->id == DOT11_MNG_EXT_CSA_ID) {
			WL_REGULATORY(("wl%d: wlc_recv_ext_csa_action: mode %d, reg %d, channel %d,"
				"count %d\n", wlc->pub->unit, req_ie->b.mode,
				req_ie->b.reg, req_ie->b.channel, req_ie->b.count));
			return;
		}
	}
	WL_REGULATORY(("wl%d: wlc_recv_ext_csa_action: unknown ID %d", wlc->pub->unit, req_ie->id));
}

bool
wlc_csa_quiet_mode(wlc_csa_info_t *csam, uint8 *tag, uint tag_len)
{
	bool quiet = FALSE;
	dot11_chan_switch_ie_t *csa_ie;

	BCM_REFERENCE(csam);

	if (!tag || tag_len <= DOT11_BCN_PRB_LEN)
		return quiet;

	tag_len = tag_len - DOT11_BCN_PRB_LEN;
	tag = tag + DOT11_BCN_PRB_LEN;

	csa_ie = (dot11_chan_switch_ie_t *)
	        bcm_parse_tlvs(tag, tag_len, DOT11_MNG_CHANNEL_SWITCH_ID);
	if (csa_ie && csa_ie->mode)
		quiet = TRUE;

	return quiet;
}

void
wlc_csa_reset_all(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = csam->wlc;
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);

	ASSERT(csa != NULL);

	if (csa->channel_sw.secs != 0) {
		wl_del_timer(wlc->wl, csa->csa_timer);
		csa->channel_sw.secs = 0;
	}
	csa->csa.count = 0;
}

#ifdef AP
int
wlc_csa_do_channel_switch(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
	chanspec_t chanspec, uint8 mode, uint8 count, uint8 reg_class, uint8 frame_type)
{
	wlc_info_t *wlc = csam->wlc;
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);
	int bcmerror;

	ASSERT(csa != NULL);

	if (!BSSCFG_AP(cfg))
		return BCME_NOTAP;

	if (!cfg->up) {
		return BCME_NOTREADY;
	}

	ASSERT(!wf_chspec_malformed(chanspec));

	if (!wlc_valid_chanspec_db(wlc->cmi, chanspec)) {
		return BCME_BADCHAN;
	}
	wlc_csa_obss_dynbw_notif_cb_notif(csam, cfg,
		BCME_OK, CSA_CHANNEL_CHANGE_START, chanspec);
	csa->csa.mode = (mode != 0) ? 1 : 0;
	csa->csa.count = count;
	csa->csa.chspec = chanspec;
	if (reg_class != 0)
		csa->csa.reg = reg_class;
	else
		csa->csa.reg = wlc_get_regclass(wlc->cmi, chanspec);

	csa->csa.frame_type = (frame_type != CSA_UNICAST_ACTION_FRAME) ?
	  CSA_BROADCAST_ACTION_FRAME : CSA_UNICAST_ACTION_FRAME;

	/* and update beacon and probe response for the specified bsscfg */
	bcmerror = wlc_csa_apply_channel_switch(csam, cfg);

	/* adds NEED_TO_UPDATE_BCN to wlc->spect_state, send csa action frames, */
	if (!bcmerror) {
		wlc_11h_set_spect_state(wlc->m11h, cfg, NEED_TO_SWITCH_CHANNEL,
		                        NEED_TO_SWITCH_CHANNEL);
#ifdef WLMCHAN
		/* If don't update chanspec of mchan-structure,
		 * P2P/GO goes back previous channel when become P2P/GO-only from STA+P2P/GO.
		 * It cause P2P disconnect by beacon loss.
		 */
		if (P2P_GO(wlc, cfg)) {
			WL_ERROR(("wl%d: %s: Update mchan chanspec due to P2P ECSA operation. "
				"(0x%X->0x%X)\n", wlc->pub->unit, __FUNCTION__,
				wlc_mchan_get_chanspec(wlc->mchan, cfg), chanspec));
			wlc_mchan_config_go_chanspec(wlc->mchan, cfg, chanspec);
		}
#endif /* WLMCHAN */
		if (mode != 0) {
			/* CSA Mode 0, Data transmission can take place till the time of
			 * actual channel switch. So, block fifo here only if Mode is 1
			 */
			wlc_block_datafifo(wlc, DATA_BLOCK_QUIET, DATA_BLOCK_QUIET);
		}
		BSSCFG_SET_CSA_IN_PROGRESS(cfg);
	}

	return BCME_OK;
}

/* This function applies the parameters of channel switch set else where */
/* It is assumed that the csa parameters have been set else where */
/* We send out the necessary csa action frames */
/* We mark wlc->spect_state with NEED_TO_UPDATE_BCN flag */
/* We only want the beacons and probe responses updated for the specified bss */
/* The actual channel switch will be initiated else where */
static int
wlc_csa_apply_channel_switch(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = csam->wlc;
	int bcmerror;

	if (!BSSCFG_AP(cfg))
		return BCME_NOTAP;

	if (!cfg->up) {
		return BCME_NOTREADY;
	}

	bcmerror = wlc_send_action_switch_channel(csam, cfg);
	if (bcmerror == BCME_OK) {
		wlc_11h_set_spect_state(wlc->m11h, cfg, NEED_TO_UPDATE_BCN, NEED_TO_UPDATE_BCN);
		wlc_bss_update_beacon(wlc, cfg);
		wlc_bss_update_probe_resp(wlc, cfg, TRUE);
	}

	return bcmerror;
}

void
wlc_csa_do_switch(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg, chanspec_t chspec)
{
	wlc_info_t *wlc = csam->wlc;
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);

	ASSERT(csa != NULL);

	/* Stop the current queue with flow control */
	wlc_txflowcontrol_override(wlc, cfg->wlcif->qi, ON, TXQ_STOP_FOR_PKT_DRAIN);

	csa->csa.mode = DOT11_CSA_MODE_ADVISORY;
	csa->csa.chspec = chspec;
	csa->csa.reg = wlc_get_regclass(wlc->cmi, chspec);
	csa->csa.count = cfg->current_bss->dtim_period + 1;

	wlc_csa_obss_dynbw_notif_cb_notif(csam, cfg,
		BCME_OK, CSA_CHANNEL_CHANGE_START, csa->csa.chspec);

	wlc_csa_apply_channel_switch(csam, cfg);
}
#endif /* AP */

#ifdef CLIENT_CSA
int wlc_send_unicast_action_switch_channel(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
	const struct ether_addr *dst, wl_chan_switch_t *csa, uint8 action_id)
{
	int ret;
	ret = wlc_send_action_switch_channel_ex(csam, cfg, dst, csa, action_id);
	return ret;
}
#endif /* CLIENT_CSA */

static int
wlc_send_action_switch_channel_ex(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
	const struct ether_addr *dst, wl_chan_switch_t *csa, uint8 action_id)
{
	wlc_info_t *wlc = csam->wlc;
	void *p;
	uint8* pbody;
	uint8* cp;
	uint body_len;
	struct dot11_action_frmhdr *action_hdr;
	bool ext_ie = FALSE;
	uint8 *bufend;
	bool ret;
#ifdef WL11AC
	bool wide_bw_ie = FALSE;
#endif /* WL11AC */

	/* Action switch_channel */
	body_len = DOT11_ACTION_HDR_LEN + TLV_HDR_LEN;
	if (action_id == DOT11_SM_ACTION_CHANNEL_SWITCH) {
		body_len += DOT11_SWITCH_IE_LEN;
		/* account for extension channel IE if operate in 40MHz */
		if ((N_ENAB(wlc->pub) && (CHSPEC_IS40(csa->chspec))) ||
		    (VHT_ENAB(wlc->pub) && CHSPEC_BW_GE(csa->chspec, WL_CHANSPEC_BW_80))) {
			body_len += (TLV_HDR_LEN + DOT11_EXTCH_IE_LEN);
			ext_ie = TRUE;
		}
#ifdef WL11AC
		/* account for wide bandwidth channel switch IE if operate in 80MHz */
		if (VHT_ENAB(wlc->pub) && CHSPEC_BW_GE(csa->chspec, WL_CHANSPEC_BW_80)) {
			body_len += (TLV_HDR_LEN + DOT11_WIDE_BW_SWITCH_IE_LEN);
			wide_bw_ie = TRUE;
		}
#endif /* WL11AC */
	} else
		body_len += DOT11_EXT_CSA_IE_LEN;

	if ((p = wlc_frame_get_action(wlc, dst, &cfg->cur_etheraddr,
			&cfg->BSSID, body_len, &pbody, DOT11_ACTION_CAT_SPECT_MNG)) == NULL) {
		return BCME_NOMEM;
	}

	/* mark the end of buffer */
	bufend = pbody + body_len;

	action_hdr = (struct dot11_action_frmhdr *)pbody;
	action_hdr->category = DOT11_ACTION_CAT_SPECT_MNG;
	action_hdr->action = action_id;

	if (action_id == DOT11_SM_ACTION_CHANNEL_SWITCH) {
		cp = wlc_write_csa_ie(csa, action_hdr->data, (body_len - DOT11_ACTION_HDR_LEN));
		if (ext_ie)
			cp = wlc_write_extch_ie(csa->chspec, cp, BUFLEN(cp, bufend));
#ifdef WL11AC
		if (wide_bw_ie)
			cp = wlc_write_wide_bw_csa_ie(csa, cp, BUFLEN(cp, bufend));
#endif /* WL11AC */
	} else
		cp = wlc_write_ext_csa_ie(csa, action_hdr->data);

	ASSERT((cp - pbody) == (int)body_len);

	WL_COEX(("wl%d: %s: Send CSA (id=%d) Action frame\n",
		wlc->pub->unit, __FUNCTION__, action_id));

	if (csa->frame_type == CSA_UNICAST_ACTION_FRAME)
		wlc_pcb_fn_register(wlc->pcb, wlc_csa_unicast_tx_complete, (void *)p, p);

	ret = wlc_sendmgmt(wlc, p, cfg->wlcif->qi, NULL);

	return (ret ? BCME_OK : BCME_ERROR);
}

static void
wlc_send_public_action_switch_channel(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
  const struct ether_addr *dst, wl_chan_switch_t *csa)
{
	wlc_info_t *wlc = csam->wlc;
	void *p;
	uint8* pbody;
	uint8* cp;
	uint body_len;
	struct dot11_action_frmhdr *action_hdr;
#ifdef WL11AC
	uint8 *bufend;
	bool wide_bw_ie = FALSE;
#endif /* WL11AC */

	/* Action switch_channel */
	body_len = sizeof(struct dot11y_action_ext_csa);
#ifdef WL11AC
	if (VHT_ENAB(wlc->pub) && (CHSPEC_IS80(csa->chspec))) {
		wide_bw_ie = TRUE;
		body_len += (TLV_HDR_LEN + DOT11_WIDE_BW_SWITCH_IE_LEN);
	}
#endif /* WL11AC */

	if ((p = wlc_frame_get_action(wlc, dst, &cfg->cur_etheraddr,
			&cfg->BSSID, body_len, &pbody, DOT11_ACTION_CAT_PUBLIC)) == NULL) {
		return;
	}

#ifdef WL11AC
	/* mark the end of buffer */
	bufend = pbody + body_len;
#endif /* WL11AC */
	action_hdr = (struct dot11_action_frmhdr *)pbody;
	action_hdr->category = DOT11_ACTION_CAT_PUBLIC;
	action_hdr->action = DOT11_PUB_ACTION_CHANNEL_SWITCH;

	cp = wlc_write_csa_body(csa, action_hdr->data);
#ifdef WL11AC
	if (wide_bw_ie)
		cp = wlc_write_wide_bw_csa_ie(csa, cp, BUFLEN(cp, bufend));
#endif /* WL11AC */

	ASSERT((cp - pbody) == (int)body_len);
	BCM_REFERENCE(cp);

	WL_COEX(("wl%d: %s: Send CSA Public Action frame\n", wlc->pub->unit, __FUNCTION__));

	if (csa->frame_type == CSA_UNICAST_ACTION_FRAME)
		wlc_pcb_fn_register(wlc->pcb, wlc_csa_unicast_tx_complete, (void *)p, p);

	wlc_sendmgmt(wlc, p, cfg->wlcif->qi, NULL);
}

int
wlc_send_action_switch_channel(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = csam->wlc;
	wlc_csa_t *csa;
	int bcmerror = BCME_OK;

	ASSERT(cfg != NULL);

#ifdef PSPRETEND
	/* When sending the CSA, packets are going to be dropped somewhere in the process as
	 * the radio channel changes and the packets in transit are still set up for the
	 * original channel.
	 * PS pretend would try to save these packets but this is difficult to coordinate.
	 * For now, it is better to disable ps pretend in the short time window around CSA.
	 * It is useful to 'fake' a ps pretend event hitting the ps pretend retry limit,
	 * as the ps pretend will be reactivated when the channel conditions become
	 * normal again.
	 */
	if (PS_PRETEND_ENABLED(cfg)) {
		wlc_pspretend_csa_upd(wlc->pps_info, cfg);
	}
#endif /* PSPRETEND */

	csa = CSA_BSSCFG_CUBBY(csam, cfg);
	ASSERT(csa != NULL);
	if (csa->csa.frame_type == CSA_UNICAST_ACTION_FRAME) {
		struct scb *scb;
		struct scb_iter scbiter;

		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
			struct scb *psta_prim = wlc_ap_get_psta_prim(wlc->ap, scb);
			wlc_csa_scb_cubby_t *csa_scb = CSA_SCB_CUBBY(csam, scb);
			ASSERT(csa_scb != NULL);

			if (SCB_ASSOCIATED(scb) && !SCB_ISMULTI(scb) &&
				(psta_prim == NULL)) {
				bcmerror = wlc_send_action_switch_channel_ex(csam, cfg, &scb->ea,
					&csa->csa, DOT11_SM_ACTION_CHANNEL_SWITCH);

				if (bcmerror < 0) {
					break;
				} else {
					if (N_ENAB(wlc->pub)) {
						wlc_send_public_action_switch_channel(csam,
						  scb->bsscfg, &scb->ea, &csa->csa);
						csa_scb->dcs_relocation_state =
							CSA_UNICAST_RELOCATION_PENDING;
					}
				}
			} else {
				csa_scb->dcs_relocation_state = CSA_UNICAST_RELOCATION_NONE;
			}
		}
	} else {
		wlc_send_action_switch_channel_ex(csam, cfg, &ether_bcast,
			&csa->csa, DOT11_SM_ACTION_CHANNEL_SWITCH);
		wlc_send_public_action_switch_channel(csam, cfg, &ether_bcast, &csa->csa);
	}

	return bcmerror;
}

#define CSA_PRE_SWITCH_TIME	10	/* pre-csa switch time, in unit of ms */

void
wlc_csa_count_down(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = csam->wlc;
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);
	wlc_bss_info_t *current_bss = cfg->current_bss;

	ASSERT(csa != NULL);

	/* Resume tx */
	if (csa->csa.count == 0) {
#ifdef WL_CS_RESTRICT_RELEASE
		bool start = BSSCFG_AP(cfg) &&
		        TRUE;
#endif /* WL_CS_RESTRICT_RELEASE */
		BSSCFG_CLR_CSA_IN_PROGRESS(cfg);
		wlc_block_datafifo(wlc, DATA_BLOCK_QUIET, 0);
#ifdef WL_CS_RESTRICT_RELEASE
		wlc_restrict_csa_start(wlc, cfg, start);
#endif
#ifdef PSPRETEND
		if (PS_PRETEND_ENABLED(cfg)) {

			wlc_pspretend_csa_start(wlc->pps_info, cfg);
		}
#endif /* PSPRETEND */
		return;
	}

	/* to updated channel switch count of csa ie. */
	if (--csa->csa.count == 0) {
		/* set up time to switch channel after beacon is sent */
		wl_del_timer(wlc->wl, csa->csa_timer);
		wl_add_timer(wlc->wl, csa->csa_timer,
		             (current_bss->beacon_period < CSA_PRE_SWITCH_TIME ?
		              current_bss->beacon_period : CSA_PRE_SWITCH_TIME), 0);
		/* block data traffic but allow control */
		wlc_block_datafifo(wlc, DATA_BLOCK_QUIET, DATA_BLOCK_QUIET);
	}
}

void
wlc_csa_do_csa(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg, wl_chan_switch_t *cs, bool docs)
{
	wlc_info_t *wlc = csam->wlc;
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);

	ASSERT(csa != NULL);

	csa->csa = *cs;

	if (csa->csa.mode != 0) {
		/* CSA Mode 0, Data transmission can take place till the time of
		 * actual channel switch. So, block fifo here only if Mode is 1
		 */
		wlc_block_datafifo(wlc, DATA_BLOCK_QUIET, DATA_BLOCK_QUIET);
	}
	/* need to send legacy CSA and new 11n Ext-CSA if is n-enabled */
	wlc_send_action_switch_channel(csam, cfg);

	if (docs) {
		wlc_do_chanswitch(cfg, cs->chspec);
		wlc_csa_obss_dynbw_notif_cb_notif(csam, cfg,
			BCME_OK, CSA_CHANNEL_CHANGE_END, csa->csa.chspec);
	}
	else {
		/* dpc handles NEED_TO_UPDATE_BCN, NEED_TO_SWITCH_CHANNEL */
		wlc_11h_set_spect_state(wlc->m11h, cfg,
		                         NEED_TO_UPDATE_BCN | NEED_TO_SWITCH_CHANNEL,
		                         NEED_TO_UPDATE_BCN | NEED_TO_SWITCH_CHANNEL);
	}

	wlc_bss_update_beacon(wlc, cfg);
	wlc_bss_update_probe_resp(wlc, cfg, TRUE);
}

/* accessor functions */
uint8
wlc_csa_get_csa_count(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg)
{
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);

	ASSERT(csa != NULL);

	return csa->csa.count;
}

/* These functions register/unregister a callback that wlc_prot obss_notif_cb_notif may invoke. */
int
wlc_csa_obss_dynbw_notif_cb_register(wlc_csa_info_t *csam,
	wlc_csa_notif_cb_fn_t cb, void *arg)
{
	bcm_notif_h hdl = csam->csa_notif_hdl;
	return bcm_notif_add_interest(hdl, (bcm_notif_client_callback)cb, arg);
}

int
wlc_csa_obss_dynbw_notif_cb_unregister(wlc_csa_info_t *csam,
	wlc_csa_notif_cb_fn_t cb, void *arg)
{
	bcm_notif_h hdl = csam->csa_notif_hdl;
	return bcm_notif_remove_interest(hdl, (bcm_notif_client_callback)cb, arg);
}

static void
wlc_csa_obss_dynbw_notif_cb_notif(wlc_csa_info_t *csam,
	wlc_bsscfg_t *cfg, int status,
	int signal, chanspec_t new_chanspec)
{
	wlc_csa_notif_cb_data_t notif_data;
	bcm_notif_h hdl = csam->csa_notif_hdl;
	notif_data.cfg = cfg;
	notif_data.status = status;
	notif_data.signal = signal;
	notif_data.chanspec = new_chanspec;
	bcm_notif_signal(hdl, &notif_data);
	return;
}
#endif /* WLCSA */
