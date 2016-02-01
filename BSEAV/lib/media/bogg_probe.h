/***************************************************************************
 *     Copyright (c) 2011 Broadcom Corporation
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
#ifndef _BOGG_PROBE_H__
#define _BOGG_PROBE_H__

#include "bmedia_probe_impl.h"
#include "bfile_cache.h"
#include "bogg_util.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct bogg_probe_stream {
      bmedia_probe_stream media;
} bogg_probe_stream;

typedef struct bogg_probe_track {
      bmedia_probe_track media;
} bogg_probe_track;

extern const bmedia_probe_format_desc bogg_probe;

typedef struct bogg_file_header_state {
    bfile_cached_segment segment;
    bogg_page_payload_parser parser;
    bogg_page_header header;
    int payload_size;
    bool spanning;
    bool segment_valid;
} bogg_file_header_state;

int bogg_file_header_read(bogg_file_header_state *state, batom_factory_t factory, bfile_buffer_t buf, const bfile_segment *segment);
void bogg_file_header_shutdown(bogg_file_header_state *state);

#ifdef __cplusplus
}
#endif


#endif /* _BOGG_PROBE_H__ */

