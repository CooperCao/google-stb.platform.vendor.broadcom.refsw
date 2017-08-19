/*
 * NAN SHIM Layer for API compatiability
 * with  wlc layer
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
 * $Id:$
 */

#ifndef _wlc_nan_awdl_shim_h_
#define _wlc_nan_awdl_shim_h_

#define SCAN_DB_CREATE(osh, unit, max_cache)	wlc_scandb_create((osh), (unit), (max_cache))
#define WLC_IBSS_START(wlc, cfg, type) wlc_join_start_ibss((wlc), (cfg), (type))
#define WLC_PKTTAG_INFO_MOVE(wlc, pkt, dup) wlc_pkttag_info_move((wlc), (pkt), (dup))

#define WLC_GET_ACT_FRAME(wlc, dst, src, bssid, len, pbody, cat)\
				wlc_frame_get_action((wlc), (dst), (src), (bssid), \
						(len), (pbody), (cat))
#define WLC_WLFC_MCHAN_INTERFACE_STATE_UPDATE(wlc, bsscfg, open_close, flag) \
	wlc_wlfc_mchan_interface_state_update((wlc), (bsscfg), (open_close), (flag))

#define WLC_ALLOC_BSSCFG(wlc, idx, type, flags, addr)	wlc_bsscfg_alloc((wlc), \
							(idx), (type), (flags), (addr))
#define IOV_FLAGS_NAN  0,
#define WLC_NAN_DOIOVAR nan_cmn_iov_doiovar
#endif /* _wlc_nan_awdl_shim_h */
