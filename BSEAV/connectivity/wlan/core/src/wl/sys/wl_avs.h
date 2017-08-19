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

#ifndef _wl_avs_h_
#define _wl_avs_h_

#include <typedefs.h>
#include <wlioctl.h>
#include <wlc.h>

typedef struct wl_avs wl_avs_t;

wl_avs_t *wl_avs_attach(wlc_info_t *wlc);
void wl_avs_detach(wl_avs_t *avs_info);

#endif /* _wl_avs_h_ */
