/*
* Tunnel BT Traffic Over Wlan public header file
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
#ifndef _wlc_tbow_h_
#define _wlc_tbow_h_

#define ETHER_TYPE_TBOW 0xC022
#define WLC_TBOW_ETHER_TYPE_MATCH(eth_type)	(eth_type == ETHER_TYPE_TBOW)

extern tbow_info_t *wlc_tbow_attach(wlc_info_t* wlc);
extern void wlc_tbow_detach(tbow_info_t *tbow_info);

bool tbow_recv_wlandata(tbow_info_t *tbow_info, void *sdu);

uint32 tbow_ho_connection_done(tbow_info_t *tbow_info, wlc_bsscfg_t *bsscfg, wl_wsec_key_t *key);

void tbow_ho_bt_send_status(tbow_info_t *tbow_info, wlc_bsscfg_t *bsscfg);

int tbow_start_go(tbow_info_t *tbow_info, wlc_bsscfg_t *bsscfg,
		tbow_setup_netinfo_t *netinfo);
int tbow_join_go(tbow_info_t *tbow_info, wlc_bsscfg_t *bsscfg,
		tbow_setup_netinfo_t *netinfo);
int tbow_teardown_link(tbow_info_t *tbow_info, wlc_bsscfg_t *bsscfg);
#endif /* _wlc_tbow_h_ */
