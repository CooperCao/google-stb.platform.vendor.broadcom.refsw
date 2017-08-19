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

*	The file names are kept as wlc_filsauth.c / wlc_filsauth.h.
*	802.11ai in full is not a requirement for OCE.
*	OCE has a pre-requisite for FILS authentication.

*	In future, if 802.11ai full implementation is needed, we can have a wrapper file
*	like wlc_fils.c / wlc_fils.h which will include wlc_filsauth.c/.h file to cover
*	up the FILS authentication. Having wlc_fils.c/.h for now will be misleading.
*/

#include <wlc_cfg.h>
#include <typedefs.h>
#include <osl.h>
#include <wl_dbg.h>
#include <wlioctl.h>
#include <wlc_bsscfg.h>

#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_types.h>
#include <wlc_ie_mgmt_types.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_vs.h>
#include <proto/filsauth.h>
#include <wlc_filsauth.h>
#include <wlc_wnm.h>
#include <bcmiov.h>
#include <wlc_ie_mgmt_ft.h>


static const bcm_iovar_t filsauth_iovars[] = {
	{ "oce_filsauth_config", 0, 0, 0, IOVT_BUFFER, 0 },
	{ NULL, 0, 0, 0, 0, 0 }
};

typedef struct wlc_filsauth_info {
	wlc_info_t *wlc;
} wlc_filsauth_info_t;

static int wlc_filsauth_wlc_up(void *ctx);
static int wlc_filsauth_wlc_down(void *ctx);
static void wlc_filsauth_watchdog(void *ctx);
static int wlc_filsauth_doiovar(void *hdl, uint32 actionid, void *params, uint p_len,
	void *arg, uint len, uint val_size, struct wlc_if *wlcif);

extern wlc_filsauth_info_t * wlc_filsauth_attach(wlc_info_t *wlc);
extern void wlc_filsauth_detach(wlc_filsauth_info_t *filsauth);

wlc_filsauth_info_t *
BCMATTACHFN(wlc_filsauth_attach)(wlc_info_t *wlc)
{
	wlc_filsauth_info_t *filsauth = NULL;
	int ret = BCME_OK;

	filsauth = (wlc_filsauth_info_t *)MALLOCZ(wlc->osh, sizeof(*filsauth));
	if (filsauth == NULL) {
		WL_ERROR(("wl%d: %s:out of mem. alloced %u bytes\n",
			wlc->pub->unit, __FUNCTION__,  MALLOCED(wlc->osh)));
		goto fail;
	}

	filsauth->wlc = wlc;

	/* register module */
	ret = wlc_module_register(wlc->pub, filsauth_iovars, "FILSAUTH", (void *)filsauth,
		wlc_filsauth_doiovar, wlc_filsauth_watchdog,
		wlc_filsauth_wlc_up, wlc_filsauth_wlc_down);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	wlc->pub->cmn->_filsauth = TRUE;
	return filsauth;

fail:
	if (filsauth) {
		MFREE(filsauth->wlc->osh, filsauth, sizeof(*filsauth));
	}
	return NULL;
}

void
BCMATTACHFN(wlc_filsauth_detach)(wlc_filsauth_info_t* filsauth)
{
	filsauth->wlc->pub->cmn->_filsauth = FALSE;
	wlc_module_unregister(filsauth->wlc->pub, "FILSAUTH", filsauth);
	MFREE(filsauth->wlc->osh, filsauth, sizeof(*filsauth));
}

static void
wlc_filsauth_watchdog(void *ctx)
{

}

static int
wlc_filsauth_wlc_up(void *ctx)
{
	return BCME_OK;
}

static int
wlc_filsauth_wlc_down(void *ctx)
{
	return BCME_OK;
}

static int
wlc_filsauth_doiovar(void *hdl, uint32 actionid, void *params, uint p_len,
	void *arg, uint len, uint val_size, struct wlc_if *wlcif)
{
	int ret = BCME_OK;

	return ret;
}
