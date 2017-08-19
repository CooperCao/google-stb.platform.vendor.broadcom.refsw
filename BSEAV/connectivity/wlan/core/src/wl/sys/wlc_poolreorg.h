/*
 * Pool reorg to support offloaded packet forwarding
 * in D3 sleep state (powersave state)
 *
 * Broadcom 802.11abgn Networking Device Driver
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

#ifndef _WLC_POOLREORG_H_
#define _WLC_POOLREORG_H_

int wlc_poolreorg_config(wlc_info_t *wlc, int reorg);
void wlc_poolreorg_rx_hostmem_access(wlc_info_t *wlc, bool hostmem_access_enabled);
int wlc_poolreorg_devpwrstchg(wlc_info_t *wlc, bool hostmem_access_enabled);

#endif /* _WLC_POOLREORG_H_ */
