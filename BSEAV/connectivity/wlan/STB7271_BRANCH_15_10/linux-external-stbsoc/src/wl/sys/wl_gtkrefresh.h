/*
* similar supplicant functions for GTK refresh in EXT_STA
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
*
*/

#ifndef _WL_GTKREFRESH_H_
#define _WL_GTKREFRESH_H_

#ifdef BCMULP
#include <wl_gtkrefresh_ulp.h>
#endif

extern bool wlc_gtk_refresh(wlc_info_t *wlc, struct scb *scb, struct wlc_frminfo *f);
extern wlc_sup_info_t * wlc_gtk_attach(wlc_info_t *wlc);
extern void wlc_gtk_detach(wlc_sup_info_t *sup_info);
extern void * wlc_gtkoe_hw_wowl_init(wlc_sup_info_t *sup_info, struct scb *scb);
extern int wlc_gtk_init(wlc_sup_info_t *sup_info,  wlc_bsscfg_t *cfg);

#endif /* _WL_GTKREFRESH_H_ */
