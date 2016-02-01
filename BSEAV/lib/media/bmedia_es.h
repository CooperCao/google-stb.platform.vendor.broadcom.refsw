/***************************************************************************
 *     Copyright (c) 2007, Broadcom Corporation
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
 * BMedia library, ES producer
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BMEDIA_ES_H__
#define _BMEDIA_ES_H__

#include "bmedia_pes.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct bmedia_es *bmedia_es_t;

void bmedia_es_default_cfg(bmedia_pes_cfg *cfg);
bmedia_es_t bmedia_es_create(batom_factory_t factory, balloc_iface_t alloc, const bmedia_pes_cfg *cfg);
void bmedia_es_destroy(bmedia_es_t  stream);
bool bmedia_es_feed(bmedia_es_t stream, batom_pipe_t pipe);
void bmedia_es_start(bmedia_es_t stream, batom_pipe_t pipe, const bmedia_pes_stream_cfg *stream_cfg, bool raw);
void bmedia_es_stop(bmedia_es_t stream);


#ifdef __cplusplus
}
#endif


#endif /* _BMEDIA_ES_H__ */

