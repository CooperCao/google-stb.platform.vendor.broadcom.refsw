/*
 * Bonjour Dongle Offload
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
 *
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <osl.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_key.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wl_bdo.h>
#include <wl_mdns.h>

/* max based on customer requirement */
#define BDO_DOWNLOAD_SIZE_MAX	2048

/* bdo private info structure */
struct wl_bdo_info {
	wlc_info_t *wlc;	/* pointer back to wlc structure */

	uint8 enable;

	/* flattened database */
	uint16 total_size;
	uint16 current_size;
	uint16 next_frag_num;
	uint8 *database;

	wlc_mdns_info_t *mdns;
};

/* wlc_pub_t struct access macros */
#define WLCUNIT(x)	((x)->wlc->pub->unit)
#define WLCOSH(x)	((x)->wlc->osh)

enum {
	IOV_BDO
};

static const bcm_iovar_t bdo_iovars[] = {
	{"bdo", IOV_BDO, 0, 0, IOVT_BUFFER, OFFSETOF(wl_bdo_t, data)},
	{NULL, 0, 0, 0, 0, 0}
};

static int bdo_get(wl_bdo_info_t *, void *, uint, void *, int);
static int bdo_set(wl_bdo_info_t *bdo_info, void *a, int alen);

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/* bdo GET iovar */
static int
bdo_get(wl_bdo_info_t *bdo_info, void *p, uint plen, void *a, int alen)
{
	int err = BCME_OK;
	wl_bdo_t *bdo = p;
	wl_bdo_t *bdo_out = a;

	/* verify length */
	if (plen < OFFSETOF(wl_bdo_t, data) ||
		bdo->len > plen - OFFSETOF(wl_bdo_t, data)) {
		return BCME_BUFTOOSHORT;
	}

	/* copy subcommand to output */
	bdo_out->subcmd_id = bdo->subcmd_id;

	/* process subcommand */
	switch (bdo->subcmd_id) {
	case WL_BDO_SUBCMD_ENABLE:
	{
		wl_bdo_enable_t *bdo_enable = (wl_bdo_enable_t *)bdo_out->data;
		bdo_out->len = sizeof(*bdo_enable);
		bdo_enable->enable = bdo_info->enable;
		break;
	}
	case WL_BDO_SUBCMD_MAX_DOWNLOAD:
	{
		wl_bdo_max_download_t *bdo_max_download = (wl_bdo_max_download_t *)bdo_out->data;
		bdo_out->len = sizeof(*bdo_max_download);
		bdo_max_download->size = BDO_DOWNLOAD_SIZE_MAX;
		break;
	}
	default:
		err = BCME_UNSUPPORTED;
		break;
	}
	return err;
}

/* free current database */
static void
bdo_free_database(wl_bdo_info_t *bdo_info)
{
	if (bdo_info->database) {
		wlc_info_t *wlc = bdo_info->wlc;

		MFREE(wlc->osh, bdo_info->database, bdo_info->total_size);
		bdo_info->database = NULL;
		bdo_info->total_size = 0;
		bdo_info->current_size = 0;
		bdo_info->next_frag_num = 0;
	}
}

/* database download */
static int
bdo_database_download(wl_bdo_info_t *bdo_info, wl_bdo_download_t *bdo_download)
{
	int err = BCME_OK;
	wlc_info_t *wlc = bdo_info->wlc;

	if (bdo_info->enable || bdo_download->total_size > BDO_DOWNLOAD_SIZE_MAX) {
		/* cannot download while enabled or download size exceeds the max */
		return BCME_ERROR;
	}

	/* free current database and initialize for new database */
	if (bdo_download->frag_num == 0) {
		bdo_free_database(bdo_info);

		bdo_info->database = MALLOC(wlc->osh, bdo_download->total_size);
		if (bdo_info->database == NULL) {
			return BCME_NOMEM;
		}
		bdo_info->total_size = bdo_download->total_size;
		bdo_info->current_size = 0;
		bdo_info->next_frag_num = 0;
	}

	/* check fragment and save fragment */
	if (bdo_download->frag_num == bdo_info->next_frag_num &&
		bdo_download->total_size == bdo_info->total_size &&
		bdo_download->frag_size <= (bdo_info->total_size - bdo_info->current_size)) {
		memcpy(&bdo_info->database[bdo_info->current_size], bdo_download->fragment,
			bdo_download->frag_size);
		bdo_info->current_size += bdo_download->frag_size;
		bdo_info->next_frag_num++;
	} else {
		/* something gone wrong */
		bdo_free_database(bdo_info);
		err = BCME_ERROR;
	}

	return err;
}

/* returns TRUE if valid database */
static bool
bdo_is_database_valid(wl_bdo_info_t *bdo_info)
{
	if (bdo_info->database && bdo_info->total_size > 0 &&
		bdo_info->current_size == bdo_info->total_size) {
		return TRUE;
	}
	return FALSE;
}


/* bdo SET iovar */
static int
bdo_set(wl_bdo_info_t *bdo_info, void *a, int alen)
{
	int err = BCME_OK;
	wl_bdo_t *bdo = a;

	/* verify length */
	if (alen < OFFSETOF(wl_bdo_t, data) ||
		bdo->len > alen - OFFSETOF(wl_bdo_t, data)) {
		return BCME_BUFTOOSHORT;
	}

	/* process subcommand */
	switch (bdo->subcmd_id) {
	case WL_BDO_SUBCMD_DOWNLOAD:
	{
		wl_bdo_download_t *bdo_download = (wl_bdo_download_t *)bdo->data;
		if (bdo->len >= OFFSETOF(wl_bdo_download_t, fragment)) {
			err = bdo_database_download(bdo_info, bdo_download);
		} else  {
			err = BCME_BADLEN;
		}
		break;
	}
	case WL_BDO_SUBCMD_ENABLE:
	{
		wl_bdo_enable_t *bdo_enable = (wl_bdo_enable_t *)bdo->data;
		if (bdo->len >= sizeof(*bdo_enable)) {
			if (bdo_enable->enable != bdo_info->enable) {
				if (bdo_enable->enable && !bdo_is_database_valid(bdo_info)) {
					/* database must be valid to enable */
					err = BCME_ERROR;
				} else {
					bdo_info->enable = bdo_enable->enable;
					if (bdo_info->enable) {
						WL_INFORM(("database size: %d\n",
							bdo_info->total_size));
						if (!wl_mDNS_Init(bdo_info->mdns,
							bdo_info->database, bdo_info->total_size)) {
							/* mDNS failed to initialize */
							bdo_info->enable = FALSE;
							err = BCME_ERROR;
						}
					}
					/* disable or failed to initialize */
					if (!bdo_info->enable) {
						/* free memory and database */
						wl_mDNS_Exit(bdo_info->mdns);
						bdo_free_database(bdo_info);
					}
				}
			}
		} else  {
			err = BCME_BADLEN;
		}
		break;
	}
	default:
		err = BCME_UNSUPPORTED;
		break;
	}
	return err;
}

/* handling bdo related iovars */
static int
bdo_doiovar(void *hdl, uint32 actionid,
            void *p, uint plen, void *a, uint alen, uint vsize, struct wlc_if *wlcif)
{
	wl_bdo_info_t *bdo_info = hdl;
	int32 int_val = 0;
	int err = BCME_OK;
	ASSERT(bdo_info);

	WL_INFORM(("wl%d: bdo_doiovar()\n", WLCUNIT(bdo_info)));

	/* Do nothing if is not supported */
	if (!BDO_SUPPORT(bdo_info->wlc->pub)) {
		return BCME_UNSUPPORTED;
	}

	if (plen >= (int)sizeof(int_val)) {
		bcopy(p, &int_val, sizeof(int_val));
	}

	switch (actionid) {
	case IOV_GVAL(IOV_BDO):
		err = bdo_get(bdo_info, p, plen, a, alen);
		break;
	case IOV_SVAL(IOV_BDO):
		err = bdo_set(bdo_info, a, alen);
		break;
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/* Wrapper function for mdns_rx */
bool
wl_bdo_rx(wl_bdo_info_t *bdo, void *pkt, uint16 len)
{
	if (mdns_rx(bdo->mdns, pkt, len)) {
		return FALSE;
	} else {
		return TRUE;
	}
}

/*
 * initialize bdo private context.
 * returns a pointer to the bdo private context, NULL on failure.
 */
wl_bdo_info_t *
BCMATTACHFN(wl_bdo_attach)(wlc_info_t *wlc)
{
	wl_bdo_info_t *bdo;

	/* allocate bdo private info struct */
	bdo = MALLOCZ(wlc->osh, sizeof(wl_bdo_info_t));
	if (!bdo) {
		WL_ERROR(("wl%d: %s: MALLOC failed; total mallocs %d bytes\n",
			WLCWLUNIT(wlc), __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	bdo->wlc = wlc;

	/* attach mdns */
	if ((bdo->mdns = wlc_mdns_attach(wlc)) == NULL) {
		WL_ERROR(("wlc_mdns_attach failed\n"));
		wl_bdo_detach(bdo);
		return NULL;
	}

	/* register module */
	if (wlc_module_register(wlc->pub, bdo_iovars, "bdo",
		bdo, bdo_doiovar, NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
			WLCWLUNIT(wlc), __FUNCTION__));
		wl_bdo_detach(bdo);
		return NULL;
	}

	wlc->pub->_bdo_support = TRUE;
	return bdo;
}

/* cleanup bdo private context */
void
BCMATTACHFN(wl_bdo_detach)(wl_bdo_info_t *bdo)
{
	WL_INFORM(("wl%d: bdo_detach()\n", WLCUNIT(bdo)));

	if (!bdo) {
		return;
	}

	/* disable if currently running */
	if (bdo->enable) {
		wl_mDNS_Exit(bdo->mdns);
		bdo_free_database(bdo);
	}

	if (bdo->mdns) {
		wl_mdns_detach(bdo->mdns);
	}

	bdo->wlc->pub->_bdo = FALSE;

	wlc_module_unregister(bdo->wlc->pub, "bdo", bdo);
	MFREE(WLCOSH(bdo), bdo, sizeof(wl_bdo_info_t));
}
