/***************************************************************************
 *     Copyright (c) 2007 Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * ATOM playback data sink
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef __BSINK_PLAYBACK_H__
#define __BSINK_PLAYBACK_H__

#include "bioatom.h"
#include "bpvrlib_feed.h"
#include "bmma.h"
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct bsink_playback *bsink_playback_t;

typedef struct bsink_playback_cfg {
    BMMA_DeviceOffset (*addr_to_offset)(const void *ptr);
    void (*cache_flush)(const void *addr, size_t len);
    BPVRlib_Feed_Handle feed;
} bsink_playback_cfg;

typedef struct bsink_playback_status {
	bool idle; /* set to true if no data is accumulated in the sink */
} bsink_playback_status;

void bsink_playback_default_cfg(bsink_playback_cfg *cfg);
bsink_playback_t bsink_playback_create(batom_factory_t factory, const bsink_playback_cfg *cfg);
void bsink_playback_destroy(bsink_playback_t sink);
BERR_Code bsink_playback_start(bsink_playback_t sink);
void bsink_playback_stop(bsink_playback_t sink);
void bsink_playback_get_status(bsink_playback_t sink, bsink_playback_status *status);
size_t bsink_playback_feed(bsink_playback_t sink, batom_pipe_t in_pipe);

#ifdef __cplusplus
}
#endif

#endif /* __BSINK_PLAYBACK_H__ */

