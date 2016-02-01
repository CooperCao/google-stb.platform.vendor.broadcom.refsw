/***************************************************************************
 *     Copyright (c) 2007-2010, Broadcom Corporation
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
 * AVI parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/

#ifndef _BAVI_STREAM_H___
#define _BAVI_STREAM_H___

#include "bavi_parser.h"
#include "bmedia_util.h"
#include "bmedia_pes.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum bavi_stream_type {
	bavi_stream_type_unknown=0,
	bavi_stream_type_video, 
	bavi_stream_type_audio, 
	bavi_stream_type_invalid
} bavi_stream_type;

typedef struct bavi_stream_config {
    bool reorder_timestamps;
} bavi_stream_config;

typedef struct bavi_stream_status {
	bavi_streamheader header;
	bavi_stream_type stream_type;
	union {
		bmedia_waveformatex audio;
		bmedia_bitmapinfo video;
	} stream_info;

	uint8_t seq_header[256];
	unsigned seq_header_len;
	union {
		bmedia_vc1sm_info vc1_sm;
		bmedia_vc1ap_info vc1_ap;
	} seq_hdr;
} bavi_stream_status;

typedef struct bavi_file_status {
	bool header_valid;
	bavi_mainheader mainheader;
} bavi_file_status;

typedef struct bavi_stream *bavi_stream_t;
typedef struct bavi_demux  *bavi_demux_t;

#define BAVI_STREAMD_ID_RAW		1
#define BAVI_STREAMD_ID_ES		0

typedef struct bavi_demux_cfg {
	void *user_cntx;
	unsigned pes_hdr_size;
	unsigned max_streams;
	unsigned preallocated_streams;
} bavi_demux_cfg;

void bavi_demux_default_cfg(bavi_demux_cfg *cfg);
bavi_demux_t bavi_demux_create(bavi_parser_t parser, batom_factory_t factory, balloc_iface_t alloc, const bavi_demux_cfg *cfg);
void bavi_demux_reset(bavi_demux_t demux);
void bavi_demux_flush(bavi_demux_t demux);
void bavi_demux_destroy(bavi_demux_t demux);
bool bavi_demux_step(bavi_demux_t demux);
void bavi_demux_set_offset(bavi_stream_t stream, uint32_t timestamp);
void bavi_demux_set_timescale(bavi_demux_t demux, bmedia_time_scale time_scale);
void bavi_demux_movi_end(bavi_demux_t demux);
void bavi_demux_default_stream_cfg(bavi_stream_config *stream_config);

bavi_stream_t bavi_demux_get_stream(bavi_demux_t demux, unsigned n);
int bavi_stream_activate(bavi_stream_t stream, batom_pipe_t pipe, const bavi_stream_config *stream_config);
void bavi_stream_get_stream_cfg(bavi_stream_t stream, bmedia_pes_stream_cfg *cfg);
void bavi_stream_deactivate(bavi_stream_t stream);
void bavi_stream_get_status(bavi_stream_t stream, bavi_stream_status *status);

#ifdef __cplusplus
}
#endif

#endif /* _BAVI_STREAM_H__ */

