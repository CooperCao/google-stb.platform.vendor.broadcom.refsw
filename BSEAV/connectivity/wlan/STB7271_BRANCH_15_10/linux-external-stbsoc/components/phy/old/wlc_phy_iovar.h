/*
 * PHY iovar's interface
 *
 * Registers an iovar's handler, handles phy's iovar's/strings and translates them
 * to enums defined in wlc_phy_hal.h and ultimately calls wlc_phy_iovar_dispatch() in
 * the wlc_phy_hal.h
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


#ifndef _wlc_phy_iovar_h_
#define _wlc_phy_iovar_h_

extern int  wlc_phy_iovar_attach(void *pub);
extern void wlc_phy_iovar_detach(void *pub);
extern int wlc_phy_iovar_txpwrindex_set(phy_info_t *pi, void *p);
#endif  /* _wlc_phy_iovar_h_ */
