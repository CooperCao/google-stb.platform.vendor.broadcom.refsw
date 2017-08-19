/*
 * Definition of SAR limits table. Table itself is dynamically generated to
 * wlc_sar_tbl.c file. See src/wl/sar/README.txt for more details
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

#ifndef _WLC_SAR_TBL_H_
#define _WLC_SAR_TBL_H_

#include <typedefs.h>
#include <wlioctl.h>

#ifdef WL_SARLIMIT
/* Type for Board's SAR limits definition: board ID and board limits */
typedef struct wlc_board_sar_limit {
	uint16	boardtype;
	sar_limit_t sar;
} wlc_board_sar_limit_t;

/* Table of boards' SAR limits */
extern const wlc_board_sar_limit_t wlc_sar_tbl[];

/* Number of entries in table of boards' SAR limits */
extern const size_t wlc_sar_tbl_len;

#endif /* WL_SARLIMIT */

#endif /* _WLC_SAR_TBL_H_ */
