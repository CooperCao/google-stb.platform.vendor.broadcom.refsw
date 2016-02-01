/***************************************************************************
 *
 *     Copyright (c) 2009-2012, Broadcom Corporation
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
 * BMedia library, AIFF stream filter module
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BAIFF_FILTER_H__
#define _BAIFF_FILTER_H__

#include "bioatom.h"
#include "bmedia_pes.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct baiff_filter *baiff_filter_t;
typedef struct baiff_filter_cfg  {
    batom_factory_t factory;
    balloc_iface_t alloc;
    void *user_context;
    void (*error)(void *user_context);
} baiff_filter_cfg;

void baiff_filter_default_cfg(baiff_filter_cfg *cfg);

baiff_filter_t baiff_filter_create(const baiff_filter_cfg *config);

void baiff_filter_destroy(baiff_filter_t filter);

void baiff_filter_reset(baiff_filter_t filter);

size_t baiff_filter_feed(baiff_filter_t filter, batom_pipe_t pipe, batom_pipe_t pipe_out);

int baiff_filter_seek(baiff_filter_t filter, uint64_t off);

void baiff_filter_flush(baiff_filter_t filter);

void baiff_filter_get_stream_cfg(baiff_filter_t filter, bmedia_pes_stream_cfg *cfg);


#ifdef __cplusplus
}
#endif


#endif /* _BAIFF_FILTER_H__ */

