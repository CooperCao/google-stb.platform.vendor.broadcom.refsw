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

#ifndef _HND_POOLREORG_H_
#define _HND_POOLREORG_H_

typedef struct poolreorg_info_s poolreorg_info_t;

extern poolreorg_info_t *frwd_poolreorg_info;

#define FRWD_POOLREORG_INFO     (frwd_poolreorg_info)

poolreorg_info_t *poolreorg_attach(osl_t *osh,
		pktpool_t *txlfrag, pktpool_t *rxlfrag, pktpool_t *pktpool);
void poolreorg_detach(poolreorg_info_t *poolreorg_info);
int frwd_pools_reorg(poolreorg_info_t *poolreorg_info);
int frwd_pools_revert(poolreorg_info_t *poolreorg_info);
int dump_poolreorg_state(poolreorg_info_t *poolreorg_info, struct bcmstrbuf *b);
int frwd_pools_refcnt(poolreorg_info_t *poolreorg_info, int reorg);
#endif /* _HND_POOLREORG_H_ */
