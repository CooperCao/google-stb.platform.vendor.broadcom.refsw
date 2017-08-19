/*
 * wlc_sfd.h
 *
 * Short frame descriptor cache for Tx
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


#ifndef _wlc_sfd_h_
#define _wlc_sfd_h_

#include <wlc_rate.h>
#include <wlc_channel.h>
#include <wlc.h>
#include <d11.h>
#include <hnddma.h>

#define SFD_TMR_ACTIVE		0x1
#define WLC_SFD_SET_SFDID(val, sfd_id) \
		val |= htol16((sfd_id << D11AC_TXC_CACHE_IDX_SHIFT) & D11AC_TXC_CACHE_IDX_MASK)
#define WLC_SFD_GET_SFDID(sfd_id) \
		ltoh16(((sfd_id & D11AC_TXC_CACHE_IDX_MASK) >> D11AC_TXC_CACHE_IDX_SHIFT))


void wlc_sfd_get_desc(void *ctx, void *pkt, uint8 **desc0, uint16 *len0,
		uint8 **desc1, uint16 *len1);
uint32 wlc_sfd_entry_avail(wlc_sfd_cache_t *sfd_cache);
d11actxh_cache_t *wlc_sfd_get_cache_info(wlc_sfd_cache_t *sfd_cache, d11actxh_t *txh);
d11actxh_rate_t *wlc_sfd_get_rate_info(wlc_sfd_cache_t *sfd_cache, d11actxh_t *txh);
void wlc_sfd_inc_refcnt(wlc_sfd_cache_t *sfd_cache, uint32 sfd_id);
uint32 wlc_sfd_dec_refcnt(wlc_sfd_cache_t *sfd_cache, uint32 sfd_id);
int wlc_sfd_entry_alloc(wlc_sfd_cache_t *sfd_cache, d11actxh_rate_t **sfd_rate_info,
		d11actxh_cache_t **sfd_cache_info);
void wlc_sfd_entry_free(wlc_sfd_cache_t *sfd_cache, uint32 sfd_id);
wlc_sfd_cache_t* wlc_sfd_cache_attach(wlc_info_t *wlc);
void wlc_sfd_cache_detach(wlc_sfd_cache_t *sfd);

#endif /* _wlc_sfd_h_ */
