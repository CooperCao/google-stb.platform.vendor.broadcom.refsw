/***************************************************************************
 *     Copyright (c) 2007-2012, Broadcom Corporation
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
 * MP4 library, media track interface
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BMP4_TRACK_H__
#define _BMP4_TRACK_H__

#include "bmp4_util.h"
#include "bfile_cache.h"
#include "bmedia_player.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct bmp4_track_info {
	bmp4_trackheaderbox trackheader;
	bmp4_mediaheaderbox mediaheader;
	bmp4_handlerbox handler;
	bmp4_sample_info	sample_info;
	bfile_segment decode_t2s; /* (decoding) time-to-sample */
	bfile_segment composition_t2s; /* (composition) time-to-sample */
	bfile_segment samplesize;
	bfile_segment usamplesize;
	bfile_segment sampletochunk; /* sample-to-chunk, partial data-offset information */
	bfile_segment chunkoffset; /* chunk offset, partial data-offset information */ 
	bfile_segment chunkoffset64; 
	bfile_segment syncsample;
    bfile_segment edit;
    const bmp4_movieheaderbox *movieheader;
} bmp4_track_info;

typedef struct bmp4_sample {
	off_t offset;
	size_t len;
	bmedia_player_pos time; /* composition time, in msec */
    unsigned sample_count;
	bool	syncsample; 
    bool    endofstream;
} bmp4_sample;

typedef struct bmp4_stream_sample_status {
    unsigned sample_count; /* number of samples immediattly avaliable in the track */
} bmp4_stream_sample_status;

typedef struct bmp4_stream_sample *bmp4_stream_sample_t;

void bmp4_track_info_init(bmp4_track_info *track);
bmp4_stream_sample_t bmp4_stream_sample_create(bfile_io_read_t fd, const bmp4_track_info *track);
void bmp4_stream_sample_destroy(bmp4_stream_sample_t stream);
int bmp4_stream_sample_next(bmp4_stream_sample_t stream, bmp4_sample *sample, size_t max_sample_count);
bmedia_player_pos bmp4_stream_sample_seek(bmp4_stream_sample_t stream, bmedia_player_pos pos, bool *endofstream);
void bmp4_stream_sample_get_status(bmp4_stream_sample_t stream, bmp4_stream_sample_status *status);

#ifdef __cplusplus
}
#endif

#endif /* _BMP4_TRACK_H__ */



