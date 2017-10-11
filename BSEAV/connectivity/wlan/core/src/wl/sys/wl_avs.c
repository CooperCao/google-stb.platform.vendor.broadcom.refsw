/*
 * Adaptive Voltage Scaling
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
 * $Id$
 */

#include "wl_avs.h"
#include "avs.h"

#include <typedefs.h>
#include <bcmdefs.h>
#include <wlioctl.h>
#include <bcmutils.h>
#include <wlc.h>
#include <wl_rte.h>
#include <rte.h>

#define AVS_TIMER_INTERVAL_MS		100		/* Default tracking interval */
#define AVS_MIN_TIMER_INTERVAL_MS	100		/* Minimum tracking interval */
#define AVS_STACK_SIZE			2048

static void avs_timer_cb(hnd_timer_t *t);
static int avs_doiovar(void *hdl, uint32 actionid,
        void *params, uint p_len, void *arg, uint len, uint val_size, struct wlc_if *wlcif);

/*
 * Globals
 */

static CHAR avs_thread_stack[AVS_STACK_SIZE] DECLSPEC_ALIGN(16);

/*
 * Local prototypes
 */

enum {
	IOV_AVS_ITERATIONS = 0,
	IOV_AVS_INTERVAL   = 1,
	IOV_AVS_TRIGGER    = 2,
	IOV_AVS_TEMP       = 3
};

static const bcm_iovar_t avs_iovars[] = {
	{"avs_iterations", IOV_AVS_ITERATIONS, 0, 0, IOVT_UINT32, sizeof(uint32) },
	{"avs_interval",   IOV_AVS_INTERVAL,   0, 0, IOVT_UINT32, sizeof(uint32) },
	{"avs_trigger",    IOV_AVS_TRIGGER,    0, 0, IOVT_UINT32, sizeof(uint32) },
	{"avs_temp",	   IOV_AVS_TEMP,       0, 0, IOVT_UINT32, sizeof(uint32) },
	{NULL, 0, 0, 0, 0, 0}
};

/*
 * Private data structures
 */

struct wl_avs {
	wlc_info_t		*wlc;

	avs_context_t		*avs;
	osl_ext_task_t		thread;
	hnd_timer_t		*timer;

	uint32			iterations;		/* For debugging */
	uint32			interval;		/* AVS tracking interval, in ms. */
};

/*
 * Export functions
 */

wl_avs_t*
BCMATTACHFN(wl_avs_attach)(wlc_info_t *wlc)
{
	wl_avs_t *avs;

	ASSERT(wlc && wlc->pub);

	avs = (wl_avs_t*)MALLOCZ(wlc->osh, sizeof(wl_avs_t));
	if (avs == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto exit;
	}

	avs->wlc      = wlc;
	avs->interval = AVS_TIMER_INTERVAL_MS;

	/* Attach to AVS */
	avs->avs = avs_attach(wlc->osh);
	if (avs->avs == NULL) {
		goto exit;
	}

	/* Register module */
	if (wlc_module_register(wlc->pub, avs_iovars, "avs", avs, avs_doiovar,
	    NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s: event wlc_module_register() failed",
			wlc->pub->unit, __FUNCTION__));
		goto exit;
	}

	/*
	 * Create a thread for handling the timer callbacks, using the default thread loop
	 * implementation.
	 */
	if (hnd_thread_create("avs", &avs_thread_stack, sizeof(avs_thread_stack),
	    OSL_EXT_TASK_LOW_NORMAL_PRIORITY, hnd_thread_default_implementation, avs,
	    OSL_DEFAULT_EVENT_DEPTH, &avs->thread) != OSL_EXT_SUCCESS) {
		WL_ERROR(("wl%d: %s: thread creation failed\n", wlc->pub->unit, __FUNCTION__));
		goto exit;
	}

	/*
	 * Create and start a timer to run the callback in the context
	 * of the newly created thread.
	 */
	avs->timer = hnd_timer_create(NULL, avs, avs_timer_cb, NULL, &avs->thread);
	if (avs->timer == NULL) {
		WL_ERROR(("wl%d: %s: timer failed\n", wlc->pub->unit, __FUNCTION__));
		goto exit;
	}

	if (avs->interval != 0) {
		/* Use 0ms timer interval to start the convergence process a.s.a.p. */
		hnd_timer_start(avs->timer, 0, FALSE);
	}

	return avs;

exit:
	MODULE_DETACH(avs, wl_avs_detach);
	return NULL;
}

void
BCMATTACHFN(wl_avs_detach)(wl_avs_t *avs)
{
	if (avs == NULL)
		return;

	ASSERT(avs->wlc != NULL);

	if (avs->timer != NULL) {
		hnd_timer_stop(avs->timer);
		hnd_timer_free(avs->timer);
	}

	wlc_module_unregister(avs->wlc->pub, "avs", avs);

	if (avs->avs != NULL) {
		avs_detach(avs->avs);
	}

	MFREE(avs->wlc->osh, avs, sizeof(wl_avs_t));
}

/*
 * Local Functions
 */

static void
avs_timer_cb(hnd_timer_t *t)
{
	wl_avs_t *avs = (wl_avs_t*)hnd_timer_get_data(t);

	ASSERT_THREAD("avs");
	ASSERT(avs != NULL && avs->avs != NULL);

	avs->iterations++;

	if (avs_track(avs->avs) != BCME_OK) {
		avs->interval = 0;
	}

	if (avs->interval != 0) {
		ASSERT(avs->interval >= AVS_MIN_TIMER_INTERVAL_MS);
		hnd_timer_start(avs->timer, avs->interval, FALSE);
	}
}

static int
avs_doiovar(void *hdl, uint32 actionid,
        void *params, uint p_len, void *arg, uint len, uint val_size, struct wlc_if *wlcif)
{

	wl_avs_t *avs = (wl_avs_t*)hdl;
	int err = BCME_OK;
	uint32 *pval32 = (uint32*)arg;

	BCM_REFERENCE(wlcif);
	BCM_REFERENCE(val_size);
	BCM_REFERENCE(p_len);
	BCM_REFERENCE(avs);

	ASSERT(avs != NULL);

	switch (actionid) {
	case IOV_GVAL(IOV_AVS_ITERATIONS):
		*pval32 = avs->iterations;
		break;
	case IOV_SVAL(IOV_AVS_INTERVAL):
		if (*pval32 == 0) {
			avs->interval = 0;
			hnd_timer_stop(avs->timer);
		} else {
			avs->interval = MAX(*pval32, AVS_MIN_TIMER_INTERVAL_MS);
			hnd_timer_start(avs->timer, avs->interval, TRUE);
		}
		break;
	case IOV_GVAL(IOV_AVS_INTERVAL):
		*pval32 = avs->interval;
		break;
	case IOV_GVAL(IOV_AVS_TRIGGER):
		*pval32 = 1;
		avs->interval = 0;
		hnd_timer_start(avs->timer, avs->interval, FALSE);
		break;
#ifdef AVS_ENABLE_SIMULATION
	case IOV_SVAL(IOV_AVS_TEMP):
		avs_set_temperature(avs->avs, *pval32);
		break;
#endif /* AVS_ENABLE_SIMULATION */

	default:
		err = BCME_UNSUPPORTED;
		break;
	}
	return err;
}
