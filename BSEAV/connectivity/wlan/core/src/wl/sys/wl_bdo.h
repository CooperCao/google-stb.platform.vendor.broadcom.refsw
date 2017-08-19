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
 */

#ifndef _wl_bdo_h_
#define _wl_bdo_h_

#include <wlc_types.h>

typedef struct wl_bdo_info wl_bdo_info_t;

/*
 * Initialize bdo private context.
 * Returns a pointer to the bdo private context, NULL on failure.
 */
extern wl_bdo_info_t *wl_bdo_attach(wlc_info_t *wlc);

/* Cleanup bdo private context */
extern void wl_bdo_detach(wl_bdo_info_t *bdo);

/* Wrapper function for mdns_rx */
extern bool wl_bdo_rx(wl_bdo_info_t *bdo, void *pkt, uint16 len);

#endif /* _wl_bdo_h_ */
