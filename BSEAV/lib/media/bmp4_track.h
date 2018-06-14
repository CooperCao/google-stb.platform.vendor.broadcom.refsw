/***************************************************************************
 * Copyright (C) 2007-2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 * MP4 library, media track interface
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

#define B_MP4_MAX_AUXILIARY_INFO    1
#define B_MP4_MAX_SAMPLE_GROUP  1

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
    struct {
        bfile_segment sizes;
        bfile_segment offsets;
    } sampleAuxiliaryInformation[B_MP4_MAX_AUXILIARY_INFO];
    struct {
        bfile_segment sampleToGroup;
        bfile_segment description;
    } sampleGroup[B_MP4_MAX_SAMPLE_GROUP];
    const bmp4_movieheaderbox *movieheader;
} bmp4_track_info;

typedef struct bmp4_sample_auxiliary {
    bool valid;
    uint8_t size;
    uint64_t offset;
    struct bmp4_SampleAuxiliaryInformation aux_info;
} bmp4_sample_auxiliary;

typedef struct bmp4_sample_group {
    bool valid;
    uint32_t type;
    unsigned length;
    uint64_t offset;
} bmp4_sample_group;

typedef struct bmp4_sample {
	off_t offset;
	size_t len;
	bmedia_player_pos time; /* composition time, in msec */
    unsigned sample_count;
	bool	syncsample; 
    bool    endofstream;

    bmp4_sample_auxiliary auxiliaryInformation[B_MP4_MAX_AUXILIARY_INFO];
    bmp4_sample_group sampleGroup[B_MP4_MAX_SAMPLE_GROUP];
} bmp4_sample;

typedef struct bmp4_stream_sample_status {
    unsigned sample_count; /* number of samples immediately available in the track */
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



