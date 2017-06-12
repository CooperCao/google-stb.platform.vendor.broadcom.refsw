/*
 * ACPHY ACPHY VCO CAL module implementation - iovar handlers & registration
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

#include <phy_vcocal_iov.h>
#include <phy_vcocal.h>
#include <wlc_iocv_reg.h>

static const bcm_iovar_t phy_vcocal_iovars[] = {
#if defined(BCMINTPHYDBG)
	{"phy_vcocal", IOV_PHY_VCOCAL, (IOVF_SET_UP | IOVF_MFG), 0, IOVT_UINT8, 0},
#endif 
	{NULL, 0, 0, 0, 0, 0}
};

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

static int
phy_vcocal_doiovar(void *ctx, uint32 aid,
	void *p, uint plen, void *a, uint alen, uint vsize, struct wlc_if *wlcif)
{
	phy_info_t *pi = (phy_info_t *)ctx;
	int err = BCME_OK;

	BCM_REFERENCE(pi);

	switch (aid) {
#if defined(BCMINTPHYDBG)
	case IOV_GVAL(IOV_PHY_VCOCAL):
	case IOV_SVAL(IOV_PHY_VCOCAL):
		phy_vcocal_force(pi);
		break;
#endif 
	default:
#if defined(BCMINTPHYDBG)
		err = BCME_UNSUPPORTED;
#else
		err = BCME_OK;
#endif
		break;
	}

	return err;
}

/* register iovar table to the system */
int
BCMATTACHFN(phy_vcocal_register_iovt)(phy_info_t *pi, wlc_iocv_info_t *ii)
{
	wlc_iovt_desc_t iovd;

	ASSERT(ii != NULL);

	wlc_iocv_init_iovd(phy_vcocal_iovars,
	                   NULL, NULL,
	                   phy_vcocal_doiovar, NULL, NULL, pi,
	                   &iovd);

	return wlc_iocv_register_iovt(ii, &iovd);
}
