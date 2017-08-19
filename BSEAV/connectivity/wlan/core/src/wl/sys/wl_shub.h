/*
 * sensor hub header file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wl_shub.h $
 */


#ifndef _wl_shub_h_
#define _wl_shub_h_

#define WLC_SHUB_SCANFREQ_UPDATE_REQ	1
#define WLC_SHUB_SCAN_REQ		2

extern wl_shub_info_t * wl_shub_attach(wlc_info_t * wlc);
extern void wl_shub_detach(wl_shub_info_t * shp);

extern bool wl_shub_scan_in_progress(wl_shub_info_t *shp);

#endif /* _wl_shub_h_ */
