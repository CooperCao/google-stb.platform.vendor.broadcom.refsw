/*
 * wlc_sfd.c
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

/**
 * @file
 * @brief
 * Confluence: http://confluence.broadcom.com/display/WLAN/Short+Frame+Descriptor
 */

#include <typedefs.h>
#include <wlc_cfg.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <wlc_rate.h>
#include <wlc_channel.h>
#include <wlc_pub.h>
#include <wl_export.h>
#include <wlc.h>
#include <d11.h>
#include <wlc_rsdb.h>
#include <wlc_hw.h>
#include <wlc_sfd.h>

typedef struct wlc_sfd_entry {
	d11actxh_rate_t		rate_info[D11AC_TXH_NUM_RATES];	/* RateInfo */
	d11actxh_cache_t	cache_info;			/* CacheInfo */
	uint32			refcnt;				/* Reference count, lazy delete */
	uint32			access_time;			/* Last access time */
	uint8			valid;				/* Active */
} wlc_sfd_entry_t;

typedef struct wlc_sfd_cache {
	wlc_info_t		*wlc;				/* WLC reference */
	struct wl_timer		*sync_tmr;			/* Debug only */
	uint32			flags;				/* General purpose */
	uint32			alloced;			/* Number alloced in cache */
	uint8			*cache_map;			/* Bitmap of active cache entry */
	uint8			*delete_map;			/* Bitmap of marked for delete */
	wlc_sfd_entry_t		*sfd_head;			/* Head of sfd cache */
} wlc_sfd_cache_t;

static void wlc_sfd_sync_tmr(void *context);
static wlc_sfd_entry_t *wlc_sfd_entry_from_sfdid(wlc_sfd_cache_t *sfd_cache, uint32 sfd_id);

/*
 * Returns number of available cache entry
 */
uint32
wlc_sfd_entry_avail(wlc_sfd_cache_t *sfd_cache)
{
	return (MAX_SFD_FLOWS - sfd_cache->alloced);
}

/*
 * Returns address/len of desc0 containing d11actxh_pkt_t and
 * desc1 containing d11actxh_rate_t + d11actxh_cache_t
 */
void
wlc_sfd_get_desc(void *ctx, void *pkt, uint8 **desc0, uint16 *len0, uint8 **desc1, uint16 *len1)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_sfd_cache_t *sfd_cache;
	wlc_sfd_entry_t *sfd_entry;
	uint16 sfd_id, toehdrsz;
	d11actxh_t *vhtHdr;

	sfd_cache = wlc->sfd;

	toehdrsz = wlc_pkt_get_vht_hdr(wlc, pkt, &vhtHdr);
	sfd_id = WLC_SFD_GET_SFDID(vhtHdr->PktInfo.MacTxControlLow);
	sfd_entry = wlc_sfd_entry_from_sfdid(sfd_cache, sfd_id);

	*desc0 = PKTDATA(wlc->osh, pkt);
	*len0 = sizeof(d11actxh_pkt_t) + toehdrsz;

	*desc1 = (uint8 *)sfd_entry;
	*len1 = sizeof(d11actxh_rate_t) * D11AC_TXH_NUM_RATES + sizeof(d11actxh_cache_t);

	WL_NONE(("%s wlc = %p sfd_cache = %p *desc0 = 0x%p len0 = %d *desc1 = 0x%p len1 = %d\n",
		__FUNCTION__, wlc, sfd_cache, *desc0, *len0, *desc1, *len1));
}

/*
 * Rate_info from sfd_cache if sfd_id != 0
 * else rate_info from Tx_lfrag
 */
d11actxh_rate_t *
wlc_sfd_get_rate_info(wlc_sfd_cache_t *sfd_cache, d11actxh_t *txh)
{
	d11actxh_rate_t *sfd_rate_info;
	wlc_sfd_entry_t *sfd_entry;
	int sfd_id;

	sfd_id = WLC_SFD_GET_SFDID(txh->PktInfo.MacTxControlLow);
	if (sfd_id != 0) {
		sfd_entry = wlc_sfd_entry_from_sfdid(sfd_cache, sfd_id);
		sfd_rate_info = sfd_entry->rate_info;
	} else {
		sfd_rate_info = WLC_TXD_RATE_INFO_GET(txh, sfd_cache->wlc->pub->corerev);
	}

	return sfd_rate_info;
}

/*
 * Cache_info from sfd_cache if sfd_id != 0
 * else cache_info from Tx_lfrag
 */
d11actxh_cache_t *
wlc_sfd_get_cache_info(wlc_sfd_cache_t *sfd_cache, d11actxh_t *txh)
{
	d11actxh_cache_t *sfd_cache_info;
	wlc_sfd_entry_t *sfd_entry;
	int sfd_id;

	sfd_id = WLC_SFD_GET_SFDID(txh->PktInfo.MacTxControlLow);
	if (sfd_id != 0) {
		sfd_entry = wlc_sfd_entry_from_sfdid(sfd_cache, sfd_id);
		sfd_cache_info = &sfd_entry->cache_info;
	} else {
		sfd_cache_info = WLC_TXD_CACHE_INFO_GET(txh, sfd_cache->wlc->pub->corerev);
	}

	return sfd_cache_info;
}

/*
 * Reference count decremented on a tx status received,
 * returns the value after decrement
 */
uint32
wlc_sfd_dec_refcnt(wlc_sfd_cache_t *sfd_cache, uint32 sfd_id)
{
	wlc_sfd_entry_t *sfd_entry;

	ASSERT(sfd_id < MAX_SFD_FLOWS);

	sfd_entry = (wlc_sfd_entry_t *)&(sfd_cache->sfd_head[sfd_id]);

	ASSERT((uint)sfd_entry != 0xdeadbeef);
	sfd_entry->refcnt--;

	if (sfd_entry->refcnt == 0) {
		if (isset(sfd_cache->delete_map, sfd_id)) {
			clrbit(sfd_cache->delete_map, sfd_id);
			wlc_sfd_entry_free(sfd_cache, sfd_id);
		}
	}

	WL_NONE(("wl%d: %s: sfd_cache = 0x%p sfd_entry = 0x%p\n"
			"\t\tsfd_id = 0x%x refcnt = 0x%x\n",
			sfd_cache->wlc->pub->unit, __FUNCTION__, sfd_cache,
			sfd_entry, sfd_id, sfd_entry->refcnt));

	return (sfd_entry->refcnt);
}

/*
 * Reference count incremented on a txc cache hit
 */
void
wlc_sfd_inc_refcnt(wlc_sfd_cache_t *sfd_cache, uint32 sfd_id)
{
	wlc_sfd_entry_t *sfd_entry;

	ASSERT(sfd_id < MAX_SFD_FLOWS);

	sfd_entry = (wlc_sfd_entry_t *)&(sfd_cache->sfd_head[sfd_id]);

	ASSERT((uint)sfd_entry != 0xdeadbeef);
	sfd_entry->refcnt++;
	sfd_entry->access_time = OSL_SYSUPTIME();

	WL_NONE(("wl%d: %s: sfd_cache = 0x%p sfd_entry = 0x%p\n"
			"sfd_id = 0x%x refcnt = 0x%x\n",
			sfd_cache->wlc->pub->unit, __FUNCTION__, sfd_cache,
			sfd_entry, sfd_id, sfd_entry->refcnt));

	return;
}

/*
 * Translates sfd id to sfd entry
 */
wlc_sfd_entry_t *
wlc_sfd_entry_from_sfdid(wlc_sfd_cache_t *sfd_cache, uint32 sfd_id)
{
	wlc_sfd_entry_t *sfd_entry;

	ASSERT(sfd_id < MAX_SFD_FLOWS);

	sfd_entry = (wlc_sfd_entry_t *)&(sfd_cache->sfd_head[sfd_id]);

	ASSERT((uint)sfd_entry != 0xdeadbeef);

	WL_NONE(("wl%d: %s: sfd_cache = 0x%p sfd_entry = 0x%p sfd_id = 0x%x\n",
			sfd_cache->wlc->pub->unit, __FUNCTION__, sfd_cache, sfd_entry,
			sfd_id));

	return sfd_entry;
}

/*
 * Allocation of a cache entry containing rate_info and cache_info.
 * Reference count is initialized to 1
 */
int
wlc_sfd_entry_alloc(wlc_sfd_cache_t *sfd_cache, d11actxh_rate_t **sfd_rate_info,
		d11actxh_cache_t **sfd_cache_info)
{
	wlc_sfd_entry_t *sfd_entry = NULL;
	wlc_info_t *wlc;
	int sfd_idx = 0;

	ASSERT(sfd_cache->alloced < MAX_SFD_FLOWS);

	wlc = sfd_cache->wlc;

	for (; sfd_idx < MAX_SFD_FLOWS; sfd_idx++) {
		if (isclr(sfd_cache->cache_map, (uint)sfd_idx)) {
			setbit(sfd_cache->cache_map, (uint)sfd_idx);
			break;
		}
	}

	if (sfd_idx == MAX_SFD_FLOWS) {
		sfd_idx = -1;
		goto end;
	}

	sfd_entry = (wlc_sfd_entry_t *)&(sfd_cache->sfd_head[sfd_idx]);
	memset(sfd_entry, 0, sizeof(wlc_sfd_entry_t));
	sfd_entry->valid = 1;
	sfd_entry->refcnt = 1;
	sfd_entry->access_time = OSL_SYSUPTIME();

	sfd_cache->alloced += 1;

	if ((sfd_cache->flags & SFD_TMR_ACTIVE) == 0) {
		wl_add_timer(wlc->wl, sfd_cache->sync_tmr, 15 * 1000, 0);
		sfd_cache->flags |= SFD_TMR_ACTIVE;
	}

	if (sfd_rate_info)
		*sfd_rate_info = sfd_entry->rate_info;
	if (sfd_cache_info)
		*sfd_cache_info = &sfd_entry->cache_info;

end:
	WL_NONE(("wl%d: %s: sfd_cache = 0x%p sfd_entry = 0x%p\n"
			"\t\tsfd_id = 0x%x cache_map = 0x%x delete_map = 0x%x\n"
			"\t\talloced = 0x%x flags = 0x%x\n\n",
			wlc->pub->unit, __FUNCTION__, sfd_cache, sfd_entry,
			sfd_idx, *(uint16 *)sfd_cache->cache_map, *(uint16 *)sfd_cache->delete_map,
			sfd_cache->alloced, sfd_cache->flags));

	return sfd_idx;
}

/*
 * De-Allocation of a cache entry containing rate_info and cache_info.
 * If the reference count is nonzero, deletion is marked for later
 */
void
wlc_sfd_entry_free(wlc_sfd_cache_t *sfd_cache, uint32 sfd_id)
{
	wlc_sfd_entry_t *sfd_entry = NULL;

	ASSERT(sfd_id < MAX_SFD_FLOWS);

	sfd_entry = (wlc_sfd_entry_t *)&(sfd_cache->sfd_head[sfd_id]);
	if (sfd_entry->refcnt) {
		ASSERT(isclr(sfd_cache->delete_map, (uint)sfd_id));
		setbit(sfd_cache->delete_map, (uint)sfd_id);
		goto end;
	}

	ASSERT(isset(sfd_cache->cache_map, (uint)sfd_id));
	clrbit(sfd_cache->cache_map, (uint)sfd_id);
	sfd_entry->valid = 0;

	ASSERT(sfd_cache->alloced > 0);
	sfd_cache->alloced -= 1;

end:
	WL_NONE(("wl%d: %s: sfd_cache = 0x%p sfd_id = 0x%x\n"
			"\t\tcache_map = 0x%x delete_map = 0x%x alloced = 0x%x\n\n",
			sfd_cache->wlc->pub->unit, __FUNCTION__, sfd_cache,
			sfd_id, *(uint16 *)sfd_cache->cache_map, *(uint16 *)sfd_cache->delete_map,
			sfd_cache->alloced));

	return;
}

/*
 * Dumps sfd cache periodically
 */
static void
wlc_sfd_sync_tmr(void *context)
{
	wlc_sfd_cache_t *sfd_cache = (wlc_sfd_cache_t *)context;
	wlc_sfd_entry_t *sfd_entry = NULL;
	int sfd_idx = 0;

	for (; sfd_idx < MAX_SFD_FLOWS; sfd_idx++) {
		if (isset(sfd_cache->cache_map, sfd_idx)) {
			sfd_entry = (wlc_sfd_entry_t *)&(sfd_cache->sfd_head[sfd_idx]);

			WL_PRINT(("wl%d: %s: sfd_cache = 0x%p sfd_entry = 0x%p\n"
				"\t\tsfd_id = 0x%x refcnt = 0x%x valid = 0x%x\n"
				"\t\tcache_map = 0x%x delete_map = 0x%x alloced = 0x%x\n\n",
				sfd_cache->wlc->pub->unit, __FUNCTION__, sfd_cache,
				sfd_entry, sfd_idx, sfd_entry->refcnt, sfd_entry->valid,
				*(uint16 *)sfd_cache->cache_map, *(uint16 *)sfd_cache->delete_map,
				sfd_cache->alloced));
		}
	}

	return;
}

/*
 * Registration of sfd module. Timers and memory allocation for managing
 * sfd cache
 */
wlc_sfd_cache_t*
BCMATTACHFN(wlc_sfd_cache_attach)(wlc_info_t *wlc)
{
	wlc_sfd_cache_t *sfd_cache = NULL;
	uint32 alloc_sz;
	hnddma_t* di;
	int fifo_idx;

	if (!wlc) {
		WL_ERROR(("%s - Null wlc\n", __FUNCTION__));
		goto fail;
	}

	alloc_sz = sizeof(wlc_sfd_cache_t);
	if ((sfd_cache = (wlc_sfd_cache_t *)MALLOCZ(wlc->osh, alloc_sz))
		== NULL) {
		WL_ERROR(("wl%d: %s: Out of mem, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	alloc_sz = sizeof(wlc_sfd_entry_t) * MAX_SFD_FLOWS;
	if ((sfd_cache->sfd_head = (wlc_sfd_entry_t *)MALLOCZ(wlc->osh, alloc_sz))
		== NULL) {
		WL_ERROR(("wl%d: %s: Out of mem, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	ASSERT(MAX_SFD_FLOWS != 0);
	ASSERT((MAX_SFD_FLOWS % NBBY) == 0);

	alloc_sz = sizeof(uint8) * MAX_SFD_FLOWS / NBBY;
	if ((sfd_cache->cache_map = (uint8 *)MALLOCZ(wlc->osh, alloc_sz))
		== NULL) {
		WL_ERROR(("wl%d: %s: Out of mem, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	if ((sfd_cache->delete_map = (uint8 *)MALLOCZ(wlc->osh, alloc_sz))
		== NULL) {
		WL_ERROR(("wl%d: %s: Out of mem, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	sfd_cache->wlc = wlc;
	wlc->sfd = sfd_cache;

	if (wlc_module_register(wlc->pub, NULL, "sfd", sfd_cache, NULL,
	                        NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	if ((sfd_cache->sync_tmr = wl_init_timer(wlc->wl, wlc_sfd_sync_tmr, sfd_cache,
		"sfd_sync_timer")) == NULL) {
		WL_ERROR(("wl%d: %s: sfd_sync_timer init failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	wlc_sfd_entry_alloc(sfd_cache, NULL, NULL);

	for (fifo_idx = 0; fifo_idx < NFIFO; fifo_idx++) {
		di = WLC_HW_DI(wlc, fifo_idx);
		if (di != NULL)
			dma_context(di, wlc_sfd_get_desc, (void *)wlc);
	}

	WL_NONE(("wl%d: %s: sfd_cache = 0x%x wlc = 0x%x\n",
			wlc->pub->unit, __FUNCTION__, (uint32)sfd_cache, (uint32)wlc));

	return sfd_cache;

fail:
	wlc_sfd_cache_detach(sfd_cache);
	return NULL;
}

/*
 * De-Registration of sfd module. Timers and packet pool de-allocation
 */
void
BCMATTACHFN(wlc_sfd_cache_detach)(wlc_sfd_cache_t *sfd_cache)
{
	wlc_info_t *wlc;
	uint32 alloc_sz;

	if (sfd_cache == NULL)
		return;

	wlc = sfd_cache->wlc;

	if (sfd_cache->sync_tmr)
		wl_free_timer(wlc->wl, sfd_cache->sync_tmr);

	wlc_module_unregister(wlc->pub, "sfd", sfd_cache);

	alloc_sz = sizeof(uint8) * MAX_SFD_FLOWS / NBBY;
	MFREE(wlc->osh, sfd_cache->delete_map, alloc_sz);
	MFREE(wlc->osh, sfd_cache->cache_map, alloc_sz);

	alloc_sz = sizeof(wlc_sfd_entry_t) * MAX_SFD_FLOWS;
	MFREE(wlc->osh, sfd_cache->sfd_head, alloc_sz);

	alloc_sz = sizeof(wlc_sfd_entry_t);
	MFREE(wlc->osh, sfd_cache, alloc_sz);

	WL_NONE(("wl%d: %s: sfd_cache = 0x%x wlc = 0x%x\n",
			wlc->pub->unit, __FUNCTION__, (uint32)sfd_cache, (uint32)wlc));
}
