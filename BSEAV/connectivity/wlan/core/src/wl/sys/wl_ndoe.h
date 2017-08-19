/*
 * Neighbor Advertisement Offload interface
 *
 *   Broadcom Proprietary and Confidential. Copyright (C) 2017,
 *   All Rights Reserved.
 *   
 *   This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 *   the contents of this file may not be disclosed to third parties, copied
 *   or duplicated in any form, in whole or in part, without the prior
 *   written permission of Broadcom.
 *
 * $Id$
 */


#ifndef _wl_ndoe_h_
#define _wl_ndoe_h_

#include <proto/bcmipv6.h>

/* Forward declaration */
typedef struct wl_nd_info wl_nd_info_t;

#ifdef WLNDOE
extern wl_nd_info_t * wl_nd_attach(wlc_info_t *wlc);
extern void wl_nd_detach(wl_nd_info_t *ndi);
extern int wl_nd_recv_proc(wl_nd_info_t *ndi, void *sdu);
extern wl_nd_info_t * wl_nd_alloc_ifndi(wl_nd_info_t *ndi_p, wlc_if_t *wlcif);
extern void wl_nd_clone_ifndi(wl_nd_info_t *from_ndi, wl_nd_info_t *to_ndi);
extern int wl_nd_send_proc(wl_nd_info_t *ndi, void *sdu);
extern wl_nd_info_t * wl_nd_alloc_ifndi(wl_nd_info_t *ndi_p, wlc_if_t *wlcif);
extern void wl_nd_free_ifndi(wl_nd_info_t *ndi);
#ifdef WLNDOE_RA
extern int wl_nd_ra_filter_clear_cache(wl_nd_info_t *ndi);
#endif /* WLNDOE_RA */
#endif /* WLNDOE */

#endif /* _WL_NDOE_H_ */
