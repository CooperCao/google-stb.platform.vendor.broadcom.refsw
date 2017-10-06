/*
 * @file
 * @brief
 *
 * Pool reorg and revert to support offloaded packet forwarding
 * in D3 sleep state (powersave state)
 *
 * Broadcom 802.11abg Networking Device Driver
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
#ifndef BCMPOOLRECLAIM
#error "BCMPOOLRECLAIM must be defined for this module"
#endif

#include <typedefs.h>
#include <osl.h>
#include <osl_ext.h>
#include <bcmutils.h>
#include <hnd_pktpool.h>
#include <hnd_poolreorg.h>


poolreorg_info_t *frwd_poolreorg_info = NULL;

enum {
	POOLREORG_IDLE = 0,
	POOLREORG_INPROGRESS = 1,
	POOLREORG_DONE = 2,
	POOLREORG_REVERTING = 3
};

/* pool reorg state */
typedef struct poolreorg_state_s {
	poolreorg_info_t *poolreorg_info;
	int state;
	uint16 target_frag_bufcnt;
	uint16 cur_frag_bufcnt;
	uint16 shared_buf_alloc_cnt;
	uint16 buf_size;
} poolreorg_state_t;

/* pool reorg info */
struct poolreorg_info_s {
	osl_t *osh;
	pktpool_t *pktpool_lfrag;
	pktpool_t *pktpool_rxlfrag;
	pktpool_t *pktpool;
	pktpool_cb_t cb_frag;
	pktpool_cb_t cb_sharedbuf;
	poolreorg_state_t *txfrag_state;
	poolreorg_state_t *rcvfrag_state;
	uint16 refcnt;
	/* For debug purposes */
	uint16 poolreorg_cnt;
	uint16 poolrevert_cnt;
};

static void poolreorg_fragpool_notify(pktpool_t *pktp, void *arg);
static void poolreorg_sharedpool_notify(pktpool_t *pktp, void *arg);

static int frag_to_sharedpool_convert(poolreorg_state_t *poolreorg_state,
		poolreorg_info_t *poolreorg_info, pktpool_t *pktp_frag);
static int sharedpool_to_frag_convert(poolreorg_state_t *poolreorg_state,
		poolreorg_info_t *poolreorg_info, pktpool_t *pktp_frag);

poolreorg_info_t *
BCMATTACHFN(poolreorg_attach)(osl_t *osh,
		pktpool_t *txlfrag, pktpool_t *rxlfrag, pktpool_t *pktpool)
{
	poolreorg_info_t *poolreorg_info;
	uint32 malloc_size;

	malloc_size = sizeof(*poolreorg_info)
			+ sizeof(*poolreorg_info->txfrag_state)
			+ sizeof(*poolreorg_info->rcvfrag_state);

	if (!(poolreorg_info = (poolreorg_info_t *)MALLOCZ(osh, malloc_size))) {
		return NULL;
	}

	poolreorg_info->osh = osh;
	poolreorg_info->pktpool_lfrag = txlfrag;
	poolreorg_info->pktpool_rxlfrag = rxlfrag;
	poolreorg_info->pktpool = pktpool;
	poolreorg_info->cb_frag = poolreorg_fragpool_notify;
	poolreorg_info->cb_sharedbuf = poolreorg_sharedpool_notify;

	poolreorg_info->txfrag_state =
			(poolreorg_state_t *)
			((uint8 *)poolreorg_info + sizeof(*poolreorg_info));
	poolreorg_info->txfrag_state->state = POOLREORG_IDLE;
	poolreorg_info->txfrag_state->buf_size = MAXPKTFRAGSZ;
	poolreorg_info->txfrag_state->poolreorg_info = poolreorg_info;

	poolreorg_info->rcvfrag_state =
			(poolreorg_state_t *)((uint8 *)poolreorg_info->txfrag_state
			+ sizeof(*poolreorg_info->txfrag_state));
	poolreorg_info->rcvfrag_state->state = POOLREORG_IDLE;
	poolreorg_info->rcvfrag_state->buf_size = MAXPKTRXFRAGSZ;
	poolreorg_info->rcvfrag_state->poolreorg_info = poolreorg_info;

	return poolreorg_info;
}

void
BCMATTACHFN(poolreorg_detach)(poolreorg_info_t *poolreorg_info)
{
	uint32 malloc_size;

	if (poolreorg_info == NULL) {
		return;
	}

	malloc_size = sizeof(*poolreorg_info)
			+ sizeof(*poolreorg_info->txfrag_state)
			+ sizeof(*poolreorg_info->rcvfrag_state);

	if (poolreorg_info->cb_sharedbuf) {
		/* deregister shared buf pool callback */
		pktpool_avail_deregister(poolreorg_info->pktpool, poolreorg_info->cb_sharedbuf,
				poolreorg_info);
	}

	if (poolreorg_info->cb_frag) {
		/* deregister tx frag pool callback */
		pktpool_avail_deregister(poolreorg_info->pktpool_lfrag,
				poolreorg_info->cb_frag, poolreorg_info->txfrag_state);

		/* deregister rx frag pool callback */
		pktpool_avail_deregister(poolreorg_info->pktpool_rxlfrag,
				poolreorg_info->cb_frag, poolreorg_info->rcvfrag_state);
	}

	MFREE(poolreorg_info->osh, poolreorg_info, malloc_size);
}

static void
poolreorg_fragpool_notify(pktpool_t *pktp, void *arg)
{
	poolreorg_state_t *pktp_state = (poolreorg_state_t *)arg;

	/* converting frag pool to shared pool */
	frag_to_sharedpool_convert(pktp_state, pktp_state->poolreorg_info, pktp);

	/* deregister callback once pool conversion is done. */
	if (pktp_state->state == POOLREORG_DONE) {
		pktpool_avail_deregister(pktp, pktp_state->poolreorg_info->cb_frag, arg);
	}
}

static void
poolreorg_sharedpool_notify(pktpool_t *pktp, void *arg)
{
	poolreorg_info_t *poolreorg_info = (poolreorg_info_t *)arg;

	/* First fill rx frag pool */
	sharedpool_to_frag_convert(poolreorg_info->rcvfrag_state,
			poolreorg_info, poolreorg_info->pktpool_rxlfrag);

	/* then fill tx frag pool, as tx frag pool already have few buffers. */
	sharedpool_to_frag_convert(poolreorg_info->txfrag_state,
			poolreorg_info, poolreorg_info->pktpool_lfrag);

	/* deregister callback once both tx and rx frag pool are reverted */
	if ((poolreorg_info->rcvfrag_state->state == POOLREORG_IDLE) &&
			(poolreorg_info->txfrag_state->state == POOLREORG_IDLE)) {
		pktpool_avail_deregister(pktp, poolreorg_info->cb_sharedbuf, arg);
	}
}

static int
frag_to_sharedpool_convert(poolreorg_state_t *poolreorg_state,
		poolreorg_info_t *poolreorg_info, pktpool_t *pktp_frag)
{
	pktpool_t *pktpool;
	uint16 shared_pool_len_add;
	uint16 shared_pool_len_exp;
	uint16 shared_pool_maxlen;
	uint16 free_cnt;
	uint16 avail_bufs;

	if (poolreorg_state->state == POOLREORG_DONE) {
		return BCME_OK;
	}

	/* Keep pool state in POOLREORG_INPROGRESS */
	poolreorg_state->state = POOLREORG_INPROGRESS;

	pktpool = poolreorg_info->pktpool;
	avail_bufs = pktpool_avail(pktp_frag);

	/* Reorganising pool, only if atleast one buffer more than required is available.
	 * Gradually reorganising is ensuring that atleast one buffer is available
	 * in the pool.
	 */
	free_cnt = (poolreorg_state->target_frag_bufcnt - poolreorg_state->cur_frag_bufcnt);

	if (avail_bufs) {
		if (free_cnt >= avail_bufs) {
			free_cnt = avail_bufs - 1;
		}
	} else {
		free_cnt = 0;
	}

	if (free_cnt) {
#ifdef BCMPOOLRECLAIM
		if (BCMPOOLRECLAIM_ENAB()) {
			/* reclaim lfrag pool */
			free_cnt = pktpool_reclaim(poolreorg_info->osh, pktp_frag,
					free_cnt);
		} else
#endif /* BCMPOOLRECLAIM */
		{
			free_cnt = 0;
		}

		/* increment cur_frag_bufcnt */
		poolreorg_state->cur_frag_bufcnt += free_cnt;
	}

	/* calculate number of shared pool buffers that can
	 * be allocated from freed lfrag memory
	 */
	shared_pool_len_add = ((poolreorg_state->cur_frag_bufcnt) *
			poolreorg_state->buf_size) / MAXPKTBUFSZ;

	/* subtract already allocated shared buffer count */
	shared_pool_len_add -= poolreorg_state->shared_buf_alloc_cnt;

	if (shared_pool_len_add) {
		/* Take maxlen value backup */
		shared_pool_maxlen = pktpool_max_pkts(pktpool);

		/* increase the maxlen to len + extra buffers for use in pktpool_fill() */
		pktpool_setmaxlen(pktpool, pktpool_tot_pkts(pktpool) + shared_pool_len_add);

		/* Fill the pool with buffers till new maxlen */
		pktpool_fill(poolreorg_info->osh, pktpool, FALSE);

		/* update frag_pool_len_add with number of buffers added by pktpool_fill().
		 */
		shared_pool_len_add -= (pktpool_max_pkts(pktpool) - pktpool_tot_pkts(pktpool));

		/* increase maxlen value */
		pktpool_setmaxlen(pktpool, shared_pool_maxlen + shared_pool_len_add);

		/* update shared_buf_alloc_cnt */
		poolreorg_state->shared_buf_alloc_cnt += shared_pool_len_add;
	}

	shared_pool_len_exp = ((poolreorg_state->target_frag_bufcnt) *
			poolreorg_state->buf_size) / MAXPKTBUFSZ;

	if (shared_pool_len_exp == poolreorg_state->shared_buf_alloc_cnt) {
		/* expected buf cnt and freed cnt matched, hence pool state is
		 * POOLREORG_DONE
		 */
		poolreorg_state->state = POOLREORG_DONE;
	}

	return BCME_OK;
}

static int
sharedpool_to_frag_convert(poolreorg_state_t *poolreorg_state,
		poolreorg_info_t *poolreorg_info, pktpool_t *pktp_frag)
{
	pktpool_t *pktpool;
	uint16 frag_pool_len_add;
	uint16 pool_maxlen;
	uint16 free_cnt;
	uint16 avail_bufs;

	if (poolreorg_state->state == POOLREORG_IDLE) {
		return BCME_OK;
	}

	/* Keep pool state in POOLREORG_REVERTING */
	poolreorg_state->state = POOLREORG_REVERTING;

	pktpool = poolreorg_info->pktpool;
	avail_bufs = pktpool_avail(pktpool);

	/* Reverting pool, only if atleast one buffer more than required is available.
	 * Gradually reverting is ensuring that atleast one buffer is available
	 * in the pool.
	 */
	if ((poolreorg_state->shared_buf_alloc_cnt) && (avail_bufs)) {
		if (poolreorg_state->shared_buf_alloc_cnt >= avail_bufs) {
			free_cnt = avail_bufs - 1;
		} else {
			free_cnt = poolreorg_state->shared_buf_alloc_cnt;
		}
	} else {
		free_cnt = 0;
	}

	if (free_cnt) {
#ifdef BCMPOOLRECLAIM
		if (BCMPOOLRECLAIM_ENAB()) {
			/* reclaim shared pool and decrement shared_buf_alloc_cnt */
			free_cnt = pktpool_reclaim(poolreorg_info->osh, pktpool,
					free_cnt);
		} else
#endif /* BCMPOOLRECLAIM */
		{
			free_cnt = 0;
		}
		poolreorg_state->shared_buf_alloc_cnt -= free_cnt;

		/* Take maxlen value backup */
		pool_maxlen = pktpool_max_pkts(pktpool);

		/* decrease maxlen value */
		pktpool_setmaxlen(pktpool, pool_maxlen - free_cnt);
	}

	/* calculate number of frag pool buffers that can
	 * be allocated from freed shared pool memory
	 */
	if (poolreorg_state->shared_buf_alloc_cnt) {
		frag_pool_len_add = ((free_cnt * MAXPKTBUFSZ) / poolreorg_state->buf_size);
	} else {
		/* all shared pool buffers are freed
		 * so allocat frag pool by cur_frag_bufcnt number.
		 */
		frag_pool_len_add = poolreorg_state->cur_frag_bufcnt;
	}

	if (frag_pool_len_add) {
		/* Take maxlen value backup */
		pool_maxlen = pktpool_max_pkts(pktp_frag);

		/* assign the maxlen to len + extra buffers for use in pktpool_fill() */
		pktpool_setmaxlen(pktp_frag, pktpool_tot_pkts(pktp_frag) + frag_pool_len_add);

		/* Fill the pool with buffers till new maxlen */
		pktpool_fill(poolreorg_info->osh, pktp_frag, FALSE);

		/* update frag_pool_len_add with number of buffers added by pktpool_fill().
		 */
		frag_pool_len_add -= (pktpool_max_pkts(pktp_frag) - pktpool_tot_pkts(pktp_frag));

		/* restore maxlen value */
		pktpool_setmaxlen(pktp_frag, pool_maxlen);

		poolreorg_state->cur_frag_bufcnt -= frag_pool_len_add;
	}

	if (!(poolreorg_state->cur_frag_bufcnt)) {
		/* Pool reverting complete, so state is POOLREORG_IDLE */
		poolreorg_state->state = POOLREORG_IDLE;
	}

	return BCME_OK;
}

int
frwd_pools_reorg(poolreorg_info_t *poolreorg_info)
{
	pktpool_t *pktp;
	poolreorg_state_t *pktp_state;

	BCM_REFERENCE(pktp);
	BCM_REFERENCE(pktp_state);

	if (!BCMSPLITRX_ENAB()) {
		return BCME_OK;
	}

	/* Status variable for debug purpose */
	poolreorg_info->poolreorg_cnt++;

	/* check if both the pools are already converted to shared pool */
	if ((poolreorg_info->rcvfrag_state->state == POOLREORG_DONE) &&
			(poolreorg_info->txfrag_state->state == POOLREORG_DONE)) {
		return BCME_OK;
	}

	/* deregister poolreorg_sharedpool_notify() callback if any of the tx or rx frag pool
	 * are still in the process of reverting.
	 */
	if ((poolreorg_info->rcvfrag_state->state == POOLREORG_REVERTING) ||
			(poolreorg_info->txfrag_state->state == POOLREORG_REVERTING)) {
		pktpool_avail_deregister(poolreorg_info->pktpool, poolreorg_info->cb_sharedbuf,
				poolreorg_info);
	}

#if defined(BCMRXFRAGPOOL)
	pktp = poolreorg_info->pktpool_rxlfrag;
	if (pktp && POOL_ENAB(pktp)) {
		pktp_state = poolreorg_info->rcvfrag_state;

		if (pktp_state->state == POOLREORG_IDLE) {
			/* Init target_frag_bufcnt of rx frag pool */
			pktp_state->target_frag_bufcnt = pktpool_tot_pkts(pktp);
		}

		/* converting rx frag pool to shared pool */
		frag_to_sharedpool_convert(pktp_state, poolreorg_info, pktp);

		/* register callback if pool is still in process of conversion. */
		if (pktp_state->state == POOLREORG_INPROGRESS) {
			pktpool_avail_register(pktp, poolreorg_info->cb_frag, pktp_state);
		}
	}
#endif /* defined(BCMRXFRAGPOOL) */

#if defined(BCMFRAGPOOL)
	pktp = poolreorg_info->pktpool_lfrag;
	if (pktp && POOL_ENAB(pktp)) {
		pktp_state = poolreorg_info->txfrag_state;

		if (pktp_state->state == POOLREORG_IDLE) {
			/* Init target_frag_bufcnt of tx frag pool */
			pktp_state->target_frag_bufcnt = pktpool_tot_pkts(pktp);

			/* Keep aside few tx frags from getting reclaim for
			 * resuming tx traffic on D3 exit immediately.
			 */
			pktp_state->target_frag_bufcnt -= (pktp_state->target_frag_bufcnt >> 3);
		}

		/* converting tx frag pool to shared pool. */
		frag_to_sharedpool_convert(pktp_state, poolreorg_info, pktp);

		/* register callback if pool is still in process of conversion. */
		if (pktp_state->state == POOLREORG_INPROGRESS) {
			pktpool_avail_register(pktp, poolreorg_info->cb_frag, pktp_state);
		}
	}
#endif /* defined(BCMFRAGPOOL) */

	return BCME_OK;
}

int
frwd_pools_revert(poolreorg_info_t *poolreorg_info)
{
	pktpool_t *pktp;
	poolreorg_state_t *pktp_state;

	BCM_REFERENCE(pktp);
	BCM_REFERENCE(pktp_state);

	if (!BCMSPLITRX_ENAB()) {
		return BCME_OK;
	}

	/* Status variable for debug purpose */
	poolreorg_info->poolrevert_cnt++;

	/* check if both the pools are already reverted */
	if ((poolreorg_info->rcvfrag_state->state == POOLREORG_IDLE) &&
			(poolreorg_info->txfrag_state->state == POOLREORG_IDLE)) {
		return BCME_OK;
	}

#if defined(BCMRXFRAGPOOL)
	pktp = poolreorg_info->pktpool_rxlfrag;
	if (pktp && POOL_ENAB(pktp)) {
		pktp_state = poolreorg_info->rcvfrag_state;
		/* deregister callback if pool is still in process of conversion. */
		if (pktp_state->state == POOLREORG_INPROGRESS) {
			pktpool_avail_deregister(pktp, poolreorg_info->cb_frag, pktp_state);
		}

		/* First fill rx frag pool */
		sharedpool_to_frag_convert(pktp_state, poolreorg_info, pktp);
	}

#endif /* defined(BCMRXFRAGPOOL) */

#if defined(BCMFRAGPOOL)
	pktp = poolreorg_info->pktpool_lfrag;
	if (pktp && POOL_ENAB(pktp)) {
		pktp_state = poolreorg_info->txfrag_state;
		/* deregister callback if pool is still in process of conversion. */
		if (pktp_state->state == POOLREORG_INPROGRESS) {
			pktpool_avail_deregister(pktp, poolreorg_info->cb_frag, pktp_state);
		}

		/* then fill tx frag pool, as tx frag pool already have few buffers. */
		sharedpool_to_frag_convert(pktp_state, poolreorg_info, pktp);
	}
#endif /* defined(BCMFRAGPOOL) */

	/* register poolreorg_sharedpool_notify() callback if any of the tx or rx frag pool
	 * are not converted completely.
	 */
	if ((poolreorg_info->rcvfrag_state->state == POOLREORG_REVERTING) ||
			(poolreorg_info->txfrag_state->state == POOLREORG_REVERTING)) {
		pktpool_avail_register(poolreorg_info->pktpool, poolreorg_info->cb_sharedbuf,
				poolreorg_info);
	}

	return BCME_OK;
}

int
frwd_pools_refcnt(poolreorg_info_t *poolreorg_info, int reorg)
{
	int ret = FALSE;
	if (reorg) {
		if (!poolreorg_info->refcnt) {
			/* No existing reference. so proceed to pool reorg */
			ret = TRUE;
		}
		/* Increment refcnt */
		poolreorg_info->refcnt++;
	} else {
		if (poolreorg_info->refcnt == 1) {
			/* last reference. so proceed to pool revert */
			ret = TRUE;
		}
		/* Decrement refcnt */
		if (poolreorg_info->refcnt) {
			poolreorg_info->refcnt--;
		}
	}
	return ret;
}

int
dump_poolreorg_state(poolreorg_info_t *poolreorg_info, struct bcmstrbuf *b)
{
	poolreorg_state_t *poolreorg_state;

	bcm_bprintf(b, "POOL REORG info:\n");
	bcm_bprintf(b, "poolreorg_cnt %d poolrevert_cnt %d\n",
			poolreorg_info->poolreorg_cnt,
			poolreorg_info->poolrevert_cnt);

	poolreorg_state = poolreorg_info->txfrag_state;
	bcm_bprintf(b, "txfrag_state %d expected %d freed %d shared bufs %d "
			"size %d %d\n", poolreorg_state->state,
			poolreorg_state->target_frag_bufcnt,
			poolreorg_state->cur_frag_bufcnt,
			poolreorg_state->shared_buf_alloc_cnt,
			poolreorg_state->buf_size,
			MAXPKTBUFSZ);

	poolreorg_state = poolreorg_info->rcvfrag_state;
	bcm_bprintf(b, "rcvfrag_state %d expected %d freed %d shared bufs %d "
			"size %d %d\n", poolreorg_state->state,
			poolreorg_state->target_frag_bufcnt,
			poolreorg_state->cur_frag_bufcnt,
			poolreorg_state->shared_buf_alloc_cnt,
			poolreorg_state->buf_size,
			MAXPKTBUFSZ);

	bcm_bprintf(b, "\n");

	return BCME_OK;
}
