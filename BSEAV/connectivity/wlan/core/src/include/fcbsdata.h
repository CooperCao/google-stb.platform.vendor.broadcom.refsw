/*
 * FCBS data interface
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: fcbsutils.h 615664 2016-01-28 08:21:07Z $
 */

#ifndef	_FCBSDATA_H_
#define	_FCBSDATA_H_

#include <typedefs.h>

extern int fcbsdata_sdio_populate(si_t *sih);
extern int fcbsdata_pmuuptrigger_populate(si_t *sih);
extern int fcbsdata_d11_populate(si_t *sih, void *wlc_hw, void *wlc, bool mac_clkgating);

#endif	/* _FCBSDATA_H_ */
