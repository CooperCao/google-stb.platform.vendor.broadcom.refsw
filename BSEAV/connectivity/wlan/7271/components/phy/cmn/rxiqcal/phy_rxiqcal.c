/*
 * RXIQ CAL module implementation.
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

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_rxiqcal.h"
#include <phy_rstr.h>
#include <phy_rxiqcal.h>

/* forward declaration */
typedef struct phy_rxiqcal_mem phy_rxiqcal_mem_t;

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_PHYDUMP) || \
	defined(WLTEST)
static int phy_rxiq_mismatch_dump(void *ctx, struct bcmstrbuf *b);
static int phy_rxiqcal_dump(void *ctx, struct bcmstrbuf *b);
static int phy_rxiqcal_dump_clear(void *ctx);

#endif /* BCMDBG || BCMDBG_DUMP ||  BCMDBG_PHYDUMP || WLTEST */

/* module private states */
struct phy_rxiqcal_priv_info {
	phy_info_t 				*pi;		/* PHY info ptr */
	phy_type_rxiqcal_fns_t	*fns;		/* Function ptr */
};

/* module private states memory layout */
struct phy_rxiqcal_mem {
	phy_rxiqcal_info_t		cmn_info;
	phy_type_rxiqcal_fns_t	fns;
	phy_rxiqcal_priv_info_t priv;
	phy_rxiqcal_data_t data;
/* add other variable size variables here at the end */
};

/* local function declaration */

/* attach/detach */
phy_rxiqcal_info_t *
BCMATTACHFN(phy_rxiqcal_attach)(phy_info_t *pi)
{
	phy_rxiqcal_info_t	*cmn_info = NULL;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((cmn_info = phy_malloc(pi, sizeof(phy_rxiqcal_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize params */
	cmn_info->priv = &((phy_rxiqcal_mem_t *)cmn_info)->priv;
	cmn_info->priv->pi = pi;
	cmn_info->priv->fns = &((phy_rxiqcal_mem_t *)cmn_info)->fns;
	cmn_info->data = &((phy_rxiqcal_mem_t *)cmn_info)->data;

	/* init the rxiqcal states */
	/* pi->phy_rx_diglpf_default_coeffs are not set yet */
	cmn_info->data->phy_rx_diglpf_default_coeffs_valid = FALSE;

	/* Register callbacks */

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_PHYDUMP) || \
	defined(WLTEST)
	/* register dump callback */
	phy_dbg_add_dump_fn(pi, "rxiq_mismatch", phy_rxiq_mismatch_dump, pi);
	phy_dbg_add_dump_fns(pi, "rxiqcal", phy_rxiqcal_dump, phy_rxiqcal_dump_clear, pi);
#endif /* BCMDBG || BCMDBG_DUMP ||  BCMDBG_PHYDUMP || WLTEST */

	return cmn_info;

	/* error */
fail:
	phy_rxiqcal_detach(cmn_info);
	return NULL;
}

void
BCMATTACHFN(phy_rxiqcal_detach)(phy_rxiqcal_info_t *cmn_info)
{
	PHY_CAL(("%s\n", __FUNCTION__));

	if (cmn_info == NULL) {
		PHY_INFORM(("%s: null rxiqcal module\n", __FUNCTION__));
		return;
	}

	phy_mfree(cmn_info->priv->pi, cmn_info, sizeof(phy_rxiqcal_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_rxiqcal_register_impl)(phy_rxiqcal_info_t *cmn_info, phy_type_rxiqcal_fns_t *fns)
{
	PHY_CAL(("%s\n", __FUNCTION__));

	ASSERT(cmn_info);

	*cmn_info->priv->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_rxiqcal_unregister_impl)(phy_rxiqcal_info_t *cmn_info)
{
	PHY_CAL(("%s\n", __FUNCTION__));

	ASSERT(cmn_info);

	cmn_info->priv->fns = NULL;
}

void phy_rxiqcal_scanroam_cache(phy_info_t *pi, bool set)
{
	phy_type_rxiqcal_fns_t *fns = pi->rxiqcali->priv->fns;

	if (fns->scanroam_cache != NULL)
		(fns->scanroam_cache)(fns->ctx, set);
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_PHYDUMP) || \
	defined(WLTEST)
static int
phy_rxiq_mismatch_dump(void *ctx, struct bcmstrbuf *b)
{
	phy_info_t *pi = ctx;
	phy_rxiqcal_info_t *rxiqcali = pi->rxiqcali;
	phy_type_rxiqcal_fns_t *fns = rxiqcali->priv->fns;
	int ret = BCME_UNSUPPORTED;

	if (!pi->sh->clk) {
		return BCME_NOCLK;
	}

	if (fns->rxiq_mismatch_dump) {
		ret = (fns->rxiq_mismatch_dump)(fns->ctx, b);
	}

	return ret;
}

static int
phy_rxiqcal_dump(void *ctx, struct bcmstrbuf *b)
{
	phy_info_t *pi = ctx;
	phy_rxiqcal_info_t *rxiqcali = pi->rxiqcali;
	phy_type_rxiqcal_fns_t *fns = rxiqcali->priv->fns;
	int ret = BCME_UNSUPPORTED;

	if (!pi->sh->clk) {
		return BCME_NOCLK;
	}

	if (fns->rxiqcal_dump) {
		ret = (fns->rxiqcal_dump)(fns->ctx, b);
	}

	return ret;
}

static int
phy_rxiqcal_dump_clear(void *ctx)
{
	phy_info_t *pi = ctx;
	phy_rxiqcal_info_t *rxiqcali = pi->rxiqcali;
	phy_type_rxiqcal_fns_t *fns = rxiqcali->priv->fns;
	int ret = BCME_UNSUPPORTED;

	printf("rxiqcal_dump_clear\n");
	if (!pi->sh->clk) {
		return BCME_NOCLK;
	}

	if (fns->rxiqcal_dump_clear) {
		ret = (fns->rxiqcal_dump_clear)(fns->ctx);
	}

	return ret;
}
#endif /* BCMDBG || BCMDBG_DUMP ||  BCMDBG_PHYDUMP || WLTEST */
