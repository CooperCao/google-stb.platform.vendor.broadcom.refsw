/*
 * Assoc Mgr interfaces
 *
 * This file contains the code specific to the NDIS WDI Driver model.
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
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
 */


#if !defined(WL_ASSOC_MGR)
#error "WL_ASSOC_MGR must be defined"
#endif /*  !defined(WL_ASSOC_MGR) */

#include <wlc_cfg.h>

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <pcicfg.h>
#include <siutils.h>
#include <bcmendian.h>
#include <nicpci.h>
#include <wlioctl.h>
#include <pcie_core.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_key.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_hw.h>
#include <wlc_bmac.h>
#include <wlc_scb.h>
#include <wl_export.h>
#include <wl_dbg.h>
#include <wlc_assoc.h>
#include <wlc_assoc_mgr.h>
#include <wlc_event_utils.h>

#if (defined(BCMDBG) || defined(WLTEST))
#define WL_AM_DBG_IOVARS_ENAB 1
#else
#define WL_AM_DBG_IOVARS_ENAB 0
#endif 

#define ASSOC_MGR_MODULE_NAME "assoc_mgr"

struct wlc_assoc_mgr_info {
	/* references to driver `common' things */
	wlc_info_t *wlc;		 /* pointer to main wlc structure */
	osl_t *osh;
	uint8 unit;
	int cfgh;

#if WL_AM_DBG_IOVARS_ENAB
	uint32 msg_level; /* control debug output */
#endif /* WL_AM_DBG_IOVARS_ENAB */
};

typedef struct bss_am_info {
	bool pause_on_auth_resp;
	bool paused;
	struct ether_addr scb_addr;
	uint8 *auth_resp;
	uint auth_resp_len;
} bss_am_info_t;

#define AM_BSSCFG_CUBBY_LOC(am, cfg) ((bss_am_info_t **)BSSCFG_CUBBY(cfg, (am)->cfgh))
#define AM_BSSCFG_CUBBY(am, cfg) (*AM_BSSCFG_CUBBY_LOC(am, cfg))

#define UNIT(am) ((am)->unit)

#if WL_AM_DBG_IOVARS_ENAB

#define AM_MSG_LVL_IMPORTANT 1
#define AM_MSG_LVL_CHATTY 2
#define AM_MSG_LVL_ENTRY 4

#define WL_AM_MSG(am, args) \
		 do { \
			 if ((am) && \
				((am)->msg_level & AM_MSG_LVL_IMPORTANT) != 0) { \
				 WL_PRINT(args); \
			 } \
		 } while (0)

#define WL_AM_CHATTY(am, args) \
		 do { \
			 if ((am) && ((am)->msg_level & AM_MSG_LVL_CHATTY) != 0) { \
				 WL_PRINT(args); \
			 } \
		 } while (0)

#define WL_AM_ENTRY(am, args) \
		 do { \
			 if ((am) && ((am)->msg_level & AM_MSG_LVL_ENTRY) != 0) { \
				 WL_PRINT(args); \
			 } \
		 } while (0)

#else

#define WL_AM_MSG(am, args)
#define WL_AM_CHATTY(am, args)
#define WL_AM_ENTRY(am, args)
#endif /* defined(BCMDBG) || defined(WLTEST) */

enum {
	IOV_ASSOC_MGR_CMD,
#if WL_AM_DBG_IOVARS_ENAB
	IOV_ASSOC_MGR_MSGLEVEL,
#endif /* WL_AM_DBG_IOVARS_ENAB */
	IOV_LAST
};

static const bcm_iovar_t am_iovars[] = {
	{"assoc_mgr_cmd", IOV_ASSOC_MGR_CMD, (0), 0, IOVT_BUFFER, sizeof(wl_assoc_mgr_cmd_t)},
#if WL_AM_DBG_IOVARS_ENAB
	{"assoc_mgr_msglevel", IOV_ASSOC_MGR_MSGLEVEL, (0), 0, IOVT_UINT32, 0},
#endif /* WL_AM_DBG_IOVARS_ENAB */
	{NULL, 0, 0, 0, 0, 0}
};

static bss_am_info_t *
wlc_am_bsscfg_cubby_init(wlc_assoc_mgr_info_t *am_info, wlc_bsscfg_t *cfg);

static bss_am_info_t *
wlc_am_get_bsscfg(wlc_assoc_mgr_info_t *am_info, wlc_bsscfg_t *cfg);

static void
wlc_am_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg);

static int
wlc_am_doiovar(void *handle, uint32 action_id,
	void *params, uint param_len, void *arg, uint arg_len, uint var_size, struct wlc_if *wlcif);


/* Allocate context, squirrel away the passed values,
 * and return the context handle.
 */
wlc_assoc_mgr_info_t *
BCMATTACHFN(wlc_assoc_mgr_attach)(wlc_info_t *wlc)
{
	int err = BCME_OK;
	wlc_assoc_mgr_info_t *am_info = NULL;

	WL_TRACE(("wl%d: %s\n", wlc->pub->unit, __FUNCTION__));

	if (!(am_info = (wlc_assoc_mgr_info_t *)MALLOCZ(wlc->osh,
			sizeof(*am_info)))) {

		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		err = BCME_NOMEM;
		goto exit;
	}

	am_info->wlc = wlc;
	am_info->osh = wlc->osh;
	am_info->unit = (uint8)wlc->pub->unit;

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((am_info->cfgh =
			wlc_bsscfg_cubby_reserve(wlc, sizeof(bss_am_info_t *),
			NULL, wlc_am_bsscfg_deinit, NULL,
			(void *)am_info)) < 0) {

		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          UNIT(am_info), __FUNCTION__));
		err = BCME_NOMEM;
		goto exit;
	}

	/* register module */
	if (wlc_module_register(wlc->pub, am_iovars, ASSOC_MGR_MODULE_NAME,
		am_info, wlc_am_doiovar, NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: assoc mgr: wlc_module_register() failed\n",
			UNIT(am_info)));
		err = BCME_ERROR;
		goto exit;
	}
exit:
	if (err != BCME_OK && am_info) {
		wlc_assoc_mgr_detach(am_info);
		am_info = NULL;
	} else {
		wlc->pub->_assoc_mgr = TRUE;
	}

	return am_info;
}

void
BCMATTACHFN(wlc_assoc_mgr_detach)(wlc_assoc_mgr_info_t *am_info)
{
	if (am_info == NULL) {
		WL_INFORM(("%s: detach called w/ NULL\n",
			__FUNCTION__));
		goto exit;
	}

	WL_TRACE(("wl%d: %s\n", UNIT(am_info), __FUNCTION__));

	wlc_module_unregister(am_info->wlc->pub, ASSOC_MGR_MODULE_NAME, am_info);

	MFREE(am_info->osh, am_info, sizeof(*am_info));
exit:
	return;
}

static void
wlc_am_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_assoc_mgr_info_t *am_info = (wlc_assoc_mgr_info_t *)ctx;

	bss_am_info_t *am_bss = AM_BSSCFG_CUBBY(am_info, cfg);
	bss_am_info_t **pam_cfg = AM_BSSCFG_CUBBY_LOC(am_info, cfg);

	if (!am_bss) {
		/* may happen */
		goto exit;
	}

	MFREE(am_info->osh, am_bss, sizeof(*am_bss));

	*pam_cfg = NULL;
exit:
	return;
}

void
wlc_assoc_mgr_save_auth_resp(wlc_assoc_mgr_info_t *am_info, wlc_bsscfg_t *bsscfg,
	uint8 *auth_resp, uint auth_resp_len)
{
	bss_am_info_t *am_bss_info = AM_BSSCFG_CUBBY(am_info, bsscfg);

	if (am_bss_info) {
		am_bss_info->auth_resp = auth_resp;
		am_bss_info->auth_resp_len = auth_resp_len;
	}
}

bool
wlc_assoc_mgr_pause_on_auth_resp(wlc_assoc_mgr_info_t *am_info,
	wlc_bsscfg_t *bsscfg, struct ether_addr* addr, uint auth_status, uint auth_type)
{
	bool paused = FALSE;
	bss_am_info_t *am_bss_info = AM_BSSCFG_CUBBY(am_info, bsscfg);

	WL_AM_ENTRY(am_info, ("Entering: %s\n", __FUNCTION__));

	if (am_bss_info == NULL) {
		WL_AM_CHATTY(am_info,
			("wl%d.%d: %s: no assoc mgr bsscfg cubby; no pause\n",
			bsscfg->wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), __FUNCTION__));
		goto exit;
	}

	/* should the assoc be paused here? if so indicate event and return true */
	if (am_bss_info->pause_on_auth_resp) {

		/* if so save info and return TRUE */
		am_bss_info->paused = TRUE;

		paused = TRUE;

		/* save scb addr */
		memcpy(&am_bss_info->scb_addr, addr, sizeof(*addr));

		/* also indicate event */
		wlc_bss_mac_event(am_info->wlc, bsscfg, WLC_E_AUTH, addr,
			WLC_E_STATUS_SUCCESS, auth_status, auth_type,
			am_bss_info->auth_resp, am_bss_info->auth_resp_len);

	}
exit:
	WL_AM_CHATTY(am_info, ("pause request %s successful\n",
		paused ? "" : "not"));

	WL_AM_ENTRY(am_info, ("Exiting: %s\n", __FUNCTION__));

	return paused;
}

static int
wlc_am_assoc_abort(wlc_assoc_mgr_info_t *am_info, wlc_bsscfg_t *bsscfg)
{
	int bcmerr = BCME_OK;

	WL_AM_MSG(am_info, ("process abort assoc cmd\n"));

	if ((BSSCFG_STA(bsscfg) && bsscfg->assoc->state != AS_IDLE)) {

		/* Return val is for timers not cancelled
		* Ignore for now and return OK, as the abort
		* process was kicked off.
		*/
		(void)wlc_assoc_abort(bsscfg);
	} else {

		WL_AM_MSG(am_info, ("wl%d.%d: %s: Couldn't abort assoc; %s\n",
			bsscfg->wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), __FUNCTION__,
			BSSCFG_STA(bsscfg) ? "idle state" : "non-STA"));
		bcmerr = BCME_UNSUPPORTED;
	}

	if (bcmerr != BCME_OK) {
		WL_ASSOC_ERROR(("%s: got error: Returning %s\n",
			__FUNCTION__, bcmerrorstr(bcmerr)));
	}

	return bcmerr;
}

static int
wlc_am_do_cmd(wlc_assoc_mgr_info_t *am_info, wlc_bsscfg_t *bsscfg,
	wl_assoc_mgr_cmd_t *cmd)
{
	int bcmerr = BCME_OK;
	bss_am_info_t *am_bss_info = NULL;


	am_bss_info = AM_BSSCFG_CUBBY(am_info, bsscfg);

	switch (cmd->cmd) {

		case WL_ASSOC_MGR_CMD_PAUSE_ON_EVT:

			/* cubby handling for pause/unpause */
			if (cmd->params != WL_ASSOC_MGR_PARAMS_EVENT_NONE) {
				/* WL_ASSOC_MGR_CMD_PAUSE_ON_EVT - create cubby if needed */
				am_bss_info = wlc_am_get_bsscfg(am_info, bsscfg);

				if (am_bss_info == NULL) {

					WL_ASSOC_ERROR(("%s: err: assocmgr cfg cubby not exist\n",
						__FUNCTION__));
					bcmerr = BCME_NOMEM;
					break;
				}
				am_bss_info->pause_on_auth_resp = TRUE;
			} else {

				if (am_bss_info == NULL) {
					/* can't unpause if not already paused */
					bcmerr = BCME_NOTREADY;
					break;
				}

				if (am_bss_info->pause_on_auth_resp &&
					am_bss_info->paused) {

					/* try to continue */
					bcmerr = wlc_assoc_continue(bsscfg,
						&am_bss_info->scb_addr);

					/* even if continue fails, not in paused mode */
					am_bss_info->paused = FALSE;

					/* try abort if there's a failure */
					if (bcmerr != BCME_OK) {
						(void)wlc_am_assoc_abort(am_info,
							bsscfg);
					}
				}
				am_bss_info->pause_on_auth_resp = FALSE;
			}
			break;

		case WL_ASSOC_MGR_CMD_ABORT_ASSOC:
			bcmerr = wlc_am_assoc_abort(am_info, bsscfg);
			break;

		default:
			bcmerr = BCME_UNSUPPORTED;
			break;
	}

	if (bcmerr != BCME_OK) {
		WL_ASSOC_ERROR(("%s: got error: Returning %s\n",
			__FUNCTION__, bcmerrorstr(bcmerr)));
	}

	return bcmerr;
}


static int
wlc_am_doiovar(void *handle, uint32 action_id,
	void *params, uint param_len, void *arg, uint arg_len, uint var_size, struct wlc_if *wlcif)
{
	wlc_assoc_mgr_info_t *am_info = (wlc_assoc_mgr_info_t *)handle;
	wlc_info_t *wlc = am_info->wlc;
	wlc_bsscfg_t *bsscfg;
	int err = BCME_OK;
	int32 int_val = 0;
#if WL_AM_DBG_IOVARS_ENAB
	int32 *ret_int_ptr = (int32 *)arg;
#endif /* WL_AM_DBG_IOVARS_ENAB */

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	if (param_len >= (int)sizeof(int_val)) {
		memcpy(&int_val, params, sizeof(int_val));
	}


	switch (action_id) {

		case IOV_SVAL(IOV_ASSOC_MGR_CMD):
		{
			wl_assoc_mgr_cmd_t *cmd	= (wl_assoc_mgr_cmd_t *)arg;
			err = wlc_am_do_cmd(am_info, bsscfg, cmd);
			break;
		}
#if WL_AM_DBG_IOVARS_ENAB
		case IOV_SVAL(IOV_ASSOC_MGR_MSGLEVEL):
			if (param_len < (int)sizeof(uint8)) {
				err = BCME_BADARG;
			} else {
				am_info->msg_level = (uint8)int_val;
			}
			break;

		case IOV_GVAL(IOV_ASSOC_MGR_MSGLEVEL):
			*ret_int_ptr = am_info->msg_level;
			break;
#endif /* WL_AM_DBG_IOVARS_ENAB */
		default:
			err = BCME_UNSUPPORTED;
			break;
	}

	if (err != BCME_OK) {
		WL_ASSOC_ERROR(("%s failed: return %s\n",
			__FUNCTION__, bcmerrorstr(err)));
	}

	return err;
}

static bss_am_info_t*
wlc_am_get_bsscfg(wlc_assoc_mgr_info_t *am_info, wlc_bsscfg_t *cfg)
{
	bss_am_info_t *am_bss_priv = AM_BSSCFG_CUBBY(am_info, cfg);

	if (am_bss_priv == NULL) {
		am_bss_priv = wlc_am_bsscfg_cubby_init(am_info, cfg);
	}

	return am_bss_priv;
}

static bss_am_info_t*
wlc_am_bsscfg_cubby_init(wlc_assoc_mgr_info_t *am_info, wlc_bsscfg_t *cfg)
{
	bss_am_info_t **pam_cfg = AM_BSSCFG_CUBBY_LOC(am_info, cfg);
	bss_am_info_t *am_bss = NULL;

	if (!(am_bss = (bss_am_info_t *)MALLOCZ(am_info->osh, sizeof(*am_bss)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			UNIT(am_info), __FUNCTION__, MALLOCED(am_info->osh)));
	}

	*pam_cfg = am_bss;
	return am_bss;
}
