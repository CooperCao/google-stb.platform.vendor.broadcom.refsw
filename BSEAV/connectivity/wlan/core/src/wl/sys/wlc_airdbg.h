/*
 * Additional driver debug functionalities
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#ifndef _wlc_dbg_h_
#define _wlc_dbg_h_

extern void wlc_airdbg_set(wlc_info_t *wlc, bool val);
extern bool wlc_airdbg_get(wlc_info_t *wlc);
extern void wlc_airdbg_printf(const char *fmt, ...);
#endif /* _wlc_dbg_h_ */
