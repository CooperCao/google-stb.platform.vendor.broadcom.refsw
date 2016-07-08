/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2007-2016 Broadcom. All rights reserved.
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
 * BMedia library, stream probe module
 * 
 *******************************************************************************/
#ifndef _BMEDIA_PROBE_H__
#define _BMEDIA_PROBE_H__

#include "bfile_buffer.h"
#include "bmedia_types.h"
#include "blst_squeue.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*================== Module Overview =====================================
This module is used to extract (probe) program information from the multimedia container.
Information that could be extracted from the multimedia container depends on the
format, however this type of data would be extracted for all formats:
    o type of container
	o number of tracks 
	o number of programs
	o for each track, track number (also known as PID or stream id), track type (audio or video) and codec type, would be extracted as well

At this type library supports following container formats:
    o MPEG-2 TS
	o MPEG-2 PES (also known as MPEG-2 Program, DVD VOB)
	o MPEG-1 System
	o AVI (also know as DivX)
	o ASF
	o MP4

Sample code:
	const char *file = "test.mpg"
	bmedia_probe_t probe = bmedia_probe_create();
	FILE *fin = fopen(file,"rb");
	bfile_io_read_t  fd=bfile_stdio_read_attach(fin);
	const bmedia_probe_stream *stream;
	const bmedia_probe_track *track;
	bmedia_probe_config probe_config;
	char stream_info[512];

	bmedia_probe_default_cfg(&probe_config);
	probe_config.file_name = file;
	stream = bmedia_probe_parse(probe, fd, &probe_config);
	if(stream) {
		bmedia_stream_to_string(stream, stream_info, sizeof(stream_info));
		printf("%s: %s", file, stream_info);
		for(track=BLST_SQ_FIRST(&stream->tracks);track;track=BLST_SQ_NEXT(track, link)) {
			if(track->type==bmedia_track_type_video) {
			    printf("video track %u codec:%u\n", track->number, track->info.video.codec);
			}
		}
		bmedia_probe_stream_free(stream);
	}
	bfile_stdio_read_detach(fd);
	fclose(fin);
	bmedia_probe_destroy(probe);

========================================================================*/


/*
Summary:
  bmedia_probe_t is the descriptror for media probe library
Description:
  Multiple instances of bmedia_probe_t could exist at any given time, 
  however single instance of bmedia_probe_t couls parse(probe) only single
  file at the time 
*/
typedef struct bmedia_probe *bmedia_probe_t;

/*
Summary:
   This enum is used to indicate timestamp ordering in the stream
Description:
   Historically dIfferent formats use different ordering of timestamps. Timestamp could be either linked
   to the particular frame, and reordered together with frames (this is most often used configuration)
   or timestamps could be not associated with the frames and just stored in the sequentail order.
*/
typedef enum bmedia_timestamp_order {
    bmedia_timestamp_order_display, /* stream timestamps are in the display order (PTS), timestamp linked to each frame, and if stream has B frames timestamps saved in the stream aren't in sequential order */
    bmedia_timestamp_order_decode  /* stream timestamps are in the decode order (DTS),  timestamps aren't linked to each frame and even if stream has B frames, timestamps saved in the stream are still sequential */
} bmedia_timestamp_order;

/*
Summary:
   This structure defines common properties of video track
*/
typedef struct bmedia_probe_video {
	bvideo_codec codec; /* type  of video codec */
    bmedia_timestamp_order timestamp_order; /* order of timestamps in the stream */
    uint16_t width; /* coded video width, or 0 if unknown */
	uint16_t height; /* coded video height, or 0 if unknown  */
	unsigned bitrate; /* video bitrate in Kbps, or 0 if unknown */
    struct {
        uint8_t spare[32];
    } codec_specific; /* codec specific data */
} bmedia_probe_video;

/*
Summary:
   This structure defines common properties of audio track
*/
typedef struct bmedia_probe_audio {
	baudio_format codec; /* type of audio codec */
	uint8_t channel_count;  /* number of channels, or 0 if unknown  */
	uint8_t sample_size; /* number of bits in the each sample, or 0 if unknown */
	uint16_t bitrate; /* audio bitrate in Kbps, or 0 if unknown */
	unsigned sample_rate; /* audio sampling rate in Hz, or 0 if unknown */
    struct {
        uint8_t spare[32];
    } codec_specific; /* codec specific data */
} bmedia_probe_audio;


/*
Summary:
   This enum is used to indicate track type
*/
typedef	enum bmedia_track_type {
	bmedia_track_type_video, /* video track */
	bmedia_track_type_audio, /* audio track */
	bmedia_track_type_pcr,   /* track with PCR information */
	bmedia_track_type_other /* track type other than listed above, it could be video or audio track with unknown codec type */
} bmedia_track_type;

#define BMEDIA_PROBE_INVALID_PROGRAM    0xFFFFFFFF
/*
Summary:
   This structure is used to describe single track 
*/
typedef struct bmedia_probe_track {
    BLST_SQ_ENTRY(bmedia_probe_track) link; /* this field is used to link tracks together */
	bmedia_track_type type; /* type of track */
	unsigned number; /* unique track ID */
	unsigned program; /* program number that track belongs to */
	union {
		bmedia_probe_audio audio; /* information for audio track */
		bmedia_probe_video video; /* information for video track */
	} info;        
} bmedia_probe_track;

/*
Summary:
   Type of the index avaliable in the media file
*/
typedef enum bmedia_probe_index {
    bmedia_probe_index_unknown, /* information about index file is unknown */
    bmedia_probe_index_available, /* there is an index object and media file could be used with or without an index file */
    bmedia_probe_index_missing, /* there is no index object, and media file could be used without an index */
    bmedia_probe_index_required, /* there is an index object, and media file should be used with the index */
    bmedia_probe_index_unusable, /* there is an index object, but media file should be used without the index */
    bmedia_probe_index_self /* content of the file could be uses as an index */
} bmedia_probe_index;

/*
Summary:
   This structure is used to describe entire multimedia stream
*/
typedef struct bmedia_probe_stream {
	BLST_SQ_HEAD(bmedia_probe_track_list, bmedia_probe_track) tracks; /* linked list of tracks */
	bstream_mpeg_type type; /* type of multimedia stream */
    bmedia_probe_index index; /* type of the index data */
	unsigned max_bitrate; /* maximum stream bitreate, bps or 0 if unknown  */
	unsigned duration; /* duration of stream, in milliseconds or 0 if unknown */
	unsigned nprograms; /* total number of programs */
	unsigned ntracks; /* total number of tracks */
	unsigned probe_id; /* unique id of the probe */
} bmedia_probe_stream;

typedef struct bmedia_probe_stream_specific_cfg {
    struct {
        bool probe_attachments; /* allowing probing of extra metadata attached to the container */
        bool probe_next_volume; /* causes read of top-level objects until found either end-of-file or next volume */
    } mkv; /* MKV (Matroska) specific options */
} bmedia_probe_stream_specific_cfg;

/* 
Summary:
    This structure is used to configure the duration calcuation settings
*/
typedef struct bmedia_probe_duration_config {
    int32_t max_bitrate_probe_depth; /* number of times to retry binary probe */
    int32_t max_bitrate_percentage_difference; /* how close the two halves of the bitrate should be */
    int32_t max_bitrate_n_segments; /* number of segments to break a section into */
    uint32_t min_bitrate_segment_size; /* smallest size of a section before we break it into segments */
} bmedia_probe_duration_config;

/*
Summary:
   This structure is used to instruce module on how to parse a stream
*/
typedef struct bmedia_probe_config {
	const char *file_name; /* name of the file, it's used as hint in what stream type is, e.g. if file has extension of .mp4, then this is MP4 container and other parsers wouldn't be used for this file. If this member is NULL, then heuristic based on file name wouldn't be used */
	bstream_mpeg_type type; /* type of the stream, of bstream_mpeg_type_unknown if stream type is unknown. Limits probing of the file to specified type of container. Knowing stream type could speed-up parsing and remove a lot of guess work. */
	bool probe_es; /* if stream wasn't detected as any valid containter, setting this member to true, would cause stream to be tested as any type of elementary streams */
    bool probe_all_formats; /* if heuristics based on extension didn't work then probe stream using all avaliable probes */
    bool probe_payload; /* allows probing payload for some container formats, it allows to extract more track specific information */
    bool probe_duration; /* alllows payload scanning and random access to detect more accurate stream duration */
    bool probe_index; /* allows probing for presence of the index data, usually it causes the media probe to read data toward end of the file */
    bool parse_index; /* allows parsing of the index data, The entire contents of the index table may be read (perhaps more than once) and analyzed to provide more details about the layout of the file */
    bmedia_probe_stream_specific_cfg stream_specific;
    bfile_buffer_t  buffer; /* optional buffer that shall be used to access file data */
    off_t probe_offset; /* first position in the file that should be use for probing, for certain type of files in might be beneficial to start parsing from the middle of file */
    bmedia_probe_duration_config duration_config; /* settings to configure how accurate the duration calcuation should be */
    off_t min_probe_request; /* Minimum amount of data to probe to find all tracks */
    struct {
        struct {
            bool reprobe_codec; /* if set to true, then information from PSI data overwritten by information from the ES layer probes */
        } mpeg2ts_psi;
    } probe_config;
} bmedia_probe_config;

/*
Summary:
   This function is used to create new instance of bmedia_probe_t 
Result:
   o NULL - if there isn't enough system resources
   o new instance of bmedia_probe_t
*/
bmedia_probe_t bmedia_probe_create(void);

/*
Summary:
   This function is used to release all resources acquired by bmedia_probe_t
*/
void bmedia_probe_destroy(
		bmedia_probe_t probe /* instance of bmedia_probe_t */
		);

/*
Summary:
   This function is used to initializes bmedia_probe_config structure with default values. 
See also:
    bmedia_probe_parse
*/
void bmedia_probe_default_cfg(
		bmedia_probe_config *config /* pointer to bmedia_probe_config */
		);

/*
Summary:
   This function is used to parse multimedia container
Description:
   The bmedia_probe_parser function parsed multimedia container and returns discovered information.
   Prior calling this function user must provide means of reading (accessing media data) and initialize
   bmedia_probe_config structure.
See also:
	bmedia_probe_default_cfg
	bmedia_probe_stream_free
Result:
   o NULL - unknown or unsupported stream format
   o read-only pointer to stream descriptor
*/
const bmedia_probe_stream *
bmedia_probe_parse(
		bmedia_probe_t probe,  /* instance of bmedia_probe_t */
		bfile_io_read_t fd,    /* interface to access (read) data from multimedia container */
		const bmedia_probe_config *config /* pointer to bmedia_probe_config */
		);

/*
Summary:
   This function is used to convert stream descriptor into textual form
Result:
   o number of characters printed
*/
int 
bmedia_stream_to_string(
		const bmedia_probe_stream *stream,  /* read-only pointer to stream descriptor */
		char *buf,  /* pointer to the character buffer */
		size_t size /* size of character buffer */
		);

/*
Summary:
   This function is used to free resources associated with tbe stream descriptor
*/
void bmedia_probe_stream_free(
		bmedia_probe_t probe,  /* instance of bmedia_probe_t */
		const bmedia_probe_stream *stream /* pointer to the stream descriptor */
		);

/*
Summary:
   This function is used to get the color depth of a probed video track
   NOTE: This function takes into account not only what is in the stream, but
   also what is the most likely default color depth for each codec, if the
   info is missing from the stream.
*/
unsigned bmedia_probe_get_video_color_depth(const bmedia_probe_track *track);

#ifdef __cplusplus
}
#endif


#endif /* _BMEDIA_PROBE_H__ */

