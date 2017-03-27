/*
 * HTPHY Core module interface (to PHY Core module).
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

#ifndef _phy_type_ht_high_h_
#define _phy_type_ht_high_h_

#include <wlc_iocv_types.h>

/* register HTPHY specific iovar tables/handlers to system */
int phy_ht_high_register_iovt(wlc_iocv_info_t *ii);

#endif /* _phy_type_ht_high_h_ */
