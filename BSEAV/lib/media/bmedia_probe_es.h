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
 * BMedia library, stream probe module
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BMEDIA_PROBE_ES_H__
#define _BMEDIA_PROBE_ES_H__

#include "bmedia_probe_impl.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct bmedia_probe_es *bmedia_probe_es_t;

typedef struct bmedia_probe_base_es *bmedia_probe_base_es_t;

typedef struct bmedia_probe_es_desc {
	bmedia_track_type type;
	union {
		baudio_format audio;
		bvideo_codec video;
	} codec;
	const bmedia_probe_file_ext *ext_list; /* list of extensions */
	size_t max_parsing_size;
	bmedia_probe_base_es_t (*create)(batom_factory_t factory);
	void (*destroy)(bmedia_probe_base_es_t probe);
	bmedia_probe_track *(*feed)(bmedia_probe_base_es_t probe, batom_t atom, bool *done);
	void (*reset)(bmedia_probe_base_es_t probe);
	bmedia_probe_track *(*last)(bmedia_probe_base_es_t probe, unsigned *probability);
} bmedia_probe_es_desc;

bmedia_probe_es_t bmedia_probe_es_create(batom_factory_t factory);
void bmedia_probe_es_destroy(bmedia_probe_es_t probe);
bmedia_probe_track * bmedia_probe_es_feed(bmedia_probe_es_t probe, batom_t atom, unsigned *nactive);
bmedia_probe_track * bmedia_probe_es_last(bmedia_probe_es_t probe);
void bmedia_probe_es_filter_ext(bmedia_probe_es_t probe, const char *ext);
void bmedia_probe_es_filter_type(bmedia_probe_es_t probe, bmedia_track_type track_type);
void bmedia_probe_es_reset(bmedia_probe_es_t probe);
bmedia_probe_track *bmedia_probe_es_nolast(bmedia_probe_base_es_t probe, unsigned *probability);
#ifdef __cplusplus
}
#endif


#endif /* _BMEDIA_PROBE_ES_H__ */

