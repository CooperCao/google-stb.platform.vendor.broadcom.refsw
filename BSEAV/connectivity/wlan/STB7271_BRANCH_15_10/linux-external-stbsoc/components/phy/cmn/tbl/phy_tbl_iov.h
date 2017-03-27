/*
 * PHYTblInit module internal interface - iovar table/handlers
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

#ifndef _phy_tbl_iov_h_
#define _phy_tbl_iov_h_

enum {
	IOV_PHYTABLE = 1
};

/* iovar table */
#include <typedefs.h>
#include <bcmutils.h>
extern const bcm_iovar_t phy_tbl_iovars[];
#endif /* _phy_tbl_iov_h_ */
