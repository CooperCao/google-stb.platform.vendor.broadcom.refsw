/***************************************************************************
 *
 *     Copyright (c) 2009, Broadcom Corporation
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
 * BMedia library, wav stream filter module
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BWAV_FILTER_H__
#define _BWAV_FILTER_H__

#include "bioatom.h"
#include "bmedia_pes.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct bwav_filter *bwav_filter_t;
typedef struct bwav_filter_cfg  {
    batom_factory_t factory;
	balloc_iface_t alloc;
    void *user_context;
    void (*error)(void *user_context);
} bwav_filter_cfg;

void bwav_filter_default_cfg(bwav_filter_cfg *cfg);

bwav_filter_t bwav_filter_create(const bwav_filter_cfg *config);

void bwav_filter_destroy(bwav_filter_t filter);

void bwav_filter_reset(bwav_filter_t filter);

size_t bwav_filter_feed(bwav_filter_t filter, batom_pipe_t pipe, batom_pipe_t pipe_out);

int bwav_filter_seek(bwav_filter_t filter, uint64_t off);

void bwav_filter_flush(bwav_filter_t filter);

void bwav_filter_get_stream_cfg(bwav_filter_t filter, bmedia_pes_stream_cfg *cfg);


#ifdef __cplusplus
}
#endif


#endif /* _BWAV_FILTER_H__ */

