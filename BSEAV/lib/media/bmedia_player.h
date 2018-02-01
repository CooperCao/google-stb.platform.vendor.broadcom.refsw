/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * BMedia library, player interface
 *
 *******************************************************************************/
#ifndef _BMEDIA_PLAYER_H__
#define _BMEDIA_PLAYER_H__

#include "bfile_io.h"
#include "bmedia_types.h"
#include "bmedia_util.h"
#include "bfile_buffer.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define BMEDIA_PLAYER_MAX_TRACKS	16

/*================== Module Overview =====================================
This module is a helper moduie that provides uniform interface to playback with trickmodes for
the various multimedia container formats (ASF, AVI, MP4).

Result of this module is an action that shall be performed by actual playback module. There is a list of some actions:
    * read XXX bytes from offsett YYYY and send the to bmedia_filter
	* send XXX bytes from the buffer YYY and send the  to bmedia_filter

This module is just a wrapper for the stream specific modules, e.g. it unifies functionality
of bavi_player, basf_player, bmp4_player with a single interface.
========================================================================*/


/**
Summary:
  bmedia_player_t is the object that encapsulates state if the library.
**/
typedef struct bmedia_player *bmedia_player_t;



/**
Summary:
 Number of bmedia_player_pos units in one second
**/
#define BMEDIA_PLAYER_POS_SCALE	(1000)

/**
Summary:
 Interval (in msec) on how often bmedia_player_update_position shall be called
**/
#define BMEDIA_UPDATE_POSITION_INTERVAL (1000)


/**
Summary:
 Max size of block to send to playback. Truncate any blocks larger than that size.
**/
#define BMEDIA_PLAYER_MAX_BLOCK (8*1024*1024)


/**
Summary:
	position increment in msec
**/
typedef signed long bmedia_player_step;

#define BMEDIA_PLAYER_INVALID ((bmedia_player_pos)(-1))

/**
Summary:
  This enum specifies type of action requested by the player
**/
typedef enum bmedia_player_entry_type {
	bmedia_player_entry_type_file, /* player requested to read data from the file */
	bmedia_player_entry_type_embedded, /* player has provided data */
	bmedia_player_entry_type_noop, /* player was not able to provide any information at this time, however it should be able to at a later time */
	bmedia_player_entry_type_atom, /* player has provided  batom_t with a data */
	bmedia_player_entry_type_async, /* player requested asyncronoush action, application must wait fo asynchronous action to complete before calling into the hmedia_player */
	bmedia_player_entry_type_no_data, /* player temporarily wasn't able to read data from the file */
	bmedia_player_entry_type_sleep_10msec, /* player was not able to provide any information at this time, call back after 10 msec sleep */
	bmedia_player_entry_type_error, /* player encountered error */
	bmedia_player_entry_type_end_of_stream /* player reached end of stream (or begining of the stream if running in reverse) */
} bmedia_player_entry_type;

/**
Summary:
  This enum specifies kind of data is sent with the entry
**/
typedef enum bmedia_player_entry_content {
    bmedia_player_entry_content_unknown, /* type of content is unknown */
    bmedia_player_entry_content_header, /* content is a type of a header */
    bmedia_player_entry_content_payload /* content is a data payload */
} bmedia_player_entry_content;

/**
Summary:
  This structure is used to set timestamp when resync is required
**/
typedef struct bmedia_player_sync_entry {
    struct {
        uint16_t stream_id;
	    bmedia_player_pos timestamp;
    } resync[BMEDIA_PLAYER_MAX_TRACKS+1];  /* same size media_player_stream other */
} bmedia_player_sync_entry;


/**
Summary:
  This structure defined action that requested by player from an application
**/
typedef struct bmedia_player_entry {
	bmedia_player_pos timestamp; /* if not 0, then timestamp  of the current entry, application shall call function bmedia_filter_set_offset function */
  	const bmedia_player_sync_entry *entry; /* if not NULL, then timestamp  of the current entry, application shall call function bmedia_filter_set_offset function for each stream */
	off_t start;	/* start offset to the block */
	size_t length;  /* length of the block */
	bmedia_player_entry_type type; /* type of the entry */
	const void *embedded; /* if type=bmedia_player_entry_type_embedded  than this pointer is not NULL, then application shall use given data, instead of trying read data from the file */
	batom_t atom; /* if type=bmedia_player_entry_type_atom than this pointer is not NULL, and application shall use given data, instead of trying read data from the file */
    bmedia_player_entry_content content; /* type of content is pointed by the  media_player_entry */
    uint16_t filteredPid; /* if not zero, then payload of only this pid should be handed down */
} bmedia_player_entry;


/**
Summary:
  This structure is used to return bounds (limits) of the media container
**/
typedef struct bmedia_player_bounds {
	bmedia_player_pos first; /* first valid location in the file, in ms */
	bmedia_player_pos last;  /* last location in the file, in ms */
} bmedia_player_bounds;

/**
Summary:
  This structure is used to return status of the player
**/
typedef struct bmedia_player_status {
	  bmedia_player_bounds bounds; /* bounds of the current container */
	  bmedia_player_step direction; /* current direction */
	  unsigned index_error_cnt; /* number of errors detected when processing index data */
	  unsigned data_error_cnt; /* bumber of errors detected when processing data payload */
	  bstream_mpeg_type format; /* formats of the streeam that is produced by the player, it's usually the same as input format of MPEG-2 PES, format remains constant for the players lifecycle */
      bmedia_player_pos position; /* current position player position,  and it's different from results of bmedia_player_tell which often returns currently decoded/displayed position */
} bmedia_player_status;

/**
Summary:
  Enumerates the host trick modes
**/
typedef enum bmedia_player_host_trick_mode
{
	bmedia_player_host_trick_mode_auto,
	bmedia_player_host_trick_mode_normal,
	bmedia_player_host_trick_mode_I,
	bmedia_player_host_trick_mode_skipB,
	bmedia_player_host_trick_mode_IP,
	bmedia_player_host_trick_mode_skipP,
	bmedia_player_host_trick_mode_brcm,
	bmedia_player_host_trick_mode_gop,
	bmedia_player_host_trick_mode_gop_IP,
	bmedia_player_host_trick_mode_mdqt,
	bmedia_player_host_trick_mode_mdqt_IP,
	bmedia_player_host_trick_mode_time_skip,
	bmedia_player_host_trick_mode_max
} bmedia_player_host_trick_mode;

/**
Summary:
  This structure is used to specify decoder configuration
**/
typedef struct bmedia_player_decoder_config {
	  bool brcm; /* Broadcom's Mpeg Trick Mode playback algorithm. */
	  bool dqt; /* Broadcom's display queue trickmode playback algorithm. */
      bool fragmented; /* Decoder supports decoding of fragmented data */
      bool otf; /* Decoder supports on-the-fly trickmodes (rewind and fast forward) of fragmented data */
      bool stc; /* Whether decoder supports STC based trickmodes (e.g. decoder could run faster then rate specified in the stream) */
	  bmedia_time_scale max_decoder_rate; /* maximum fast forward rate supported by the decoder */
	  bmedia_player_host_trick_mode host_mode; /* requires mode host mode of operation */
	  int mode_modifier; /* modified for host_mode */
      unsigned video_buffer_size; /* size of the video decoder compressed buffer, bytes */
} bmedia_player_decoder_config;

typedef struct bmedia_player_dqt_data {
    unsigned index;
    unsigned openGopPictures;
} bmedia_player_dqt_data;

/**
Summary:
  This structure is used to specify application specific information
**/
typedef struct bmedia_player_config {
	  bfile_buffer_t buffer;  /* buffer that is used to access a file, this buffer shall be configured to support asynchronous operations */
	  batom_factory_t factory;
	  void *cntx; /* context that is passed into the user callback */
	  void (*error_detected)(void *cntx); /* callback is called when error detected */
	  void (*atom_ready)(void *cntx, bmedia_player_entry *entry); /* callback is called after completion of asynchronous action */
	  int (*get_dqt_index)(void *cntx, bmedia_player_dqt_data *data);
	  bool timeshifting; /* true if playback file is used as timeshifting buffer */
      bool reorder_timestamps; /* reorder timestamps from display into the decode order */
      bool autoselect_player; /* auto select which player to use */
      size_t prefered_read_size; /* prefered block size  for read from the index file */
      size_t max_data_parsed; /* maximum amount of data player could read when discovering the stream properties */
	  bmedia_player_decoder_config decoder_features; /* decoders mode of operation */
      size_t max_pes_size; /* maximum amount of data to be placed in a pes packet.  0 is unbounded */
      size_t key_frame_distance; /* distance (bytes) in the source stream between key frames, if 0 software guesses distance between key frames */
	  off_t data_file_size; /* fixed size of the data file. used to trim an index that extends past the data file. */
      bmedia_player_pos force_seek_step; /* if the seek step is not 0, use that for the size of seeks */
      struct {
        struct {
            size_t fragmentBufferSize; /* maximum size of single MP4 fragment, used only for fragmented files, 0 - default */
        } mp4;
      } format;
} bmedia_player_config;

/**
Summary:
  This structure is used to define stream that player shall manage
**/
typedef struct bmedia_player_stream {
	bstream_mpeg_type format; /* formats of the streeam */
	uint16_t master; /* master sub-stream(track), must be video track */
	uint16_t other[BMEDIA_PLAYER_MAX_TRACKS]; /* other sub-streams(tracks), 0 means empty slot */
	uint16_t drop[BMEDIA_PLAYER_MAX_TRACKS];  /* list of tracks which the player should ignore during output. All streams default to output unless specified here */
    bool without_index;
    struct {
        struct {
            unsigned packet_size; /* 188 or 192 bytes */
        } mpeg2ts;
        struct {
            unsigned bitrate; /* stream bitrate, in bits per second */
            bool auto_rate; /* use adaptive algorithm to measure the stream rate */
        } noindex;
		struct { /* ID's used to packetize a stream */
			uint8_t master;
			uint8_t other[BMEDIA_PLAYER_MAX_TRACKS];
		} id;
        struct {
            baudio_format audio_codec; /* audio codec for the master track */
            bvideo_codec video_codec; /* video codec for the master track */
	        baudio_format other_audio[BMEDIA_PLAYER_MAX_TRACKS]; /* audio types for other tracks, baudio_format_unknown means track is not audio tracke  */
	        bvideo_codec other_video[BMEDIA_PLAYER_MAX_TRACKS]; /* video codec for other tracks, bvideo_codec_unknown means track is not video track */
        } es;
    } stream;
	void *cntx;
	void (*decrypt_callback)(void *cntx, batom_cursor *cursor, size_t *length, void *drm_info, unsigned track_no);
} bmedia_player_stream;

/**
Summary:
  This enum is used to select what frames should be displayed by the decoder
**/
typedef enum bmedia_player_decoder_frames {
    bmedia_player_decoder_frames_all, /* all decodable frame should be displayed */
    bmedia_player_decoder_frames_IP,  /* display I(reference) and P frames only */
    bmedia_player_decoder_frames_I,   /* display I(reference) frames only */
    bmedia_player_decoder_frames_max
} bmedia_player_decoder_frames;

/**
Summary:
  This enum is used to select on the fly PVR mode of operation
**/
typedef enum bmedia_player_reordering_mode {
    bmedia_player_reordering_mode_none,         /* no reordering */
    bmedia_player_reordering_mode_sequential,   /* data sent without discontinuities */
    bmedia_player_reordering_mode_gop,          /* unused */
    bmedia_player_reordering_mode_interleaved,  /* data sent in interleaved chunks */
    bmedia_player_reordering_mode_forward,      /* data sent in forward direction */
    bmedia_player_reordering_mode_backward,     /* data sent in forward direction */
    bmedia_player_reordering_mode_max
} bmedia_player_reordering_mode;

/**
Summary:
  This structure is used to communicate back to the application desired state of the decoder
**/
typedef struct bmedia_player_decoder_mode {
    bmedia_time_scale time_scale; /* time factor of decode */
    bool discontinuity; /* player would introduce discontinuity into the stream */
    bool brcm; /* If true player uses Broadcom's Mpeg Trick Mode playback algorithm. */
    bool dqt; /* If true player uses Broadcom's display trickmode playback algorithm. */
    bool tsm; /* If true player generates stream, that requires decoder to perform timestamp management */
    bool continuous; /* If true player would send continuous data, otherwise player sends only parts of the stream */
    bool host_paced; /* player would change timestamps in the stream */
    bool otf; /* player would set random fragments of data */
    bool fragmented; /* player would set random fragments of data */
    unsigned force_source_frame_rate; /* forced source frame rate, in units of 1/1000 Hz */
    bmedia_player_decoder_frames display_frames; /* what frames should be displayed by the decoder, decoder free to discard other frames */
    bmedia_player_reordering_mode reordering_mode; /* reordering mode for on the fly PVR */
    bool simulated_tsm;
} bmedia_player_decoder_mode;

/**
Summary:
  This function initializes bmedia_player_stream structure
**/
void bmedia_player_init_stream(
		bmedia_player_stream *stream /* pointer to structure to initialize */
		);

/**
Summary:
  This function initializes bmedia_player_config structure
**/
void bmedia_player_init_config(
		bmedia_player_config *config /* pointer to structure to initialize */
		);

/**
Summary:
  This function creates new instance of bmedia_player_t
**/
bmedia_player_t
bmedia_player_create(
		bfile_io_read_t fd,  /* instance of read_only file descriptor */
		const bmedia_player_config *config,  /* pointer to the application specific configuration */
		const bmedia_player_stream *stream   /* pointer to the stream description */
		);

/**
Summary:
  This function releases resources allocated for the instance of bmedia_player
**/
void bmedia_player_destroy(
		bmedia_player_t player	/* instance of bmedia_player_t */
		);

/**
Summary:
  This function lets player to return next action
Description:
  The bmedia_player_next function fills out bmedia_player_entry structure with
  instructions on what application shall do.
Returns:
  0 - Success
  negative - if out of bound data was reached or error occured
**/
int bmedia_player_next(
		bmedia_player_t player, 	/* instance of bmedia_player_t */
		bmedia_player_entry *entry  /* pointer to the next action */
		);

/**
Summary:
  This function returns current position of the player
**/
void bmedia_player_tell(
		bmedia_player_t player,		/* instance of bmedia_player_t */
		bmedia_player_pos *pos		/* pointer to the location */
		);

/**
Summary:
  This function returns status of the bmedia_player
**/
void bmedia_player_get_status(
		bmedia_player_t player, 		/* instance of bmedia_player_t */
		bmedia_player_status *status
		);

/**
Summary:
  This function sets new direction for the player
Description:
  The bmedia_player_set_status function is used to set desired direction for the player. Player could operate if two distinctive modes of operation:
    - normal play, in this mode player would instruct application in a way that it sends all data to the stream processoer (e.g. audio and video)
	- truck modde, in this mode player would provide actions to application that it would only send key(I) frames for the selected master track. Distance between frames would not exceed value of direction, e.g. if direction is set to 10000, then application would send key(I) frames that at least 10 seconds appart
Returns:
  0 - Success
  Error code - can't change direction
**/
int bmedia_player_set_direction(
		bmedia_player_t player, /* instance of bmedia_player_t */
		bmedia_player_step direction, /* new direction, 0 is used for normal play, negative values for rewind and positive for fast forward */
		bmedia_time_scale time_scale, /* timescale factor that is currently used */
        bmedia_player_decoder_mode *mode /* new decoder mode */
		);

/**
Summary:
  This function sets new location for the player.
Description:
  Location shall be inside player bounds. After seek data would be delivered from the nearest next key(I) frames.
Returns:
  0 - success
  -1 - error during seek (e.g. seek out of bounds)
**/
int bmedia_player_seek(
		bmedia_player_t player, /* instance of bmedia_player_t */
		bmedia_player_pos pos   /* new location for the player, in ms */
		);

/**
Summary:
  This function updates player with latest presentation time, obtained from the decoder
**/
void bmedia_player_update_position(bmedia_player_t player, uint32_t pts);

/**
Summary:
  This function verifies whether given sub-stream listed in the stream descriptor
**/
bool bmedia_player_stream_test(
		const bmedia_player_stream *stream,
		unsigned sub_stream
		);

/**
Summary:
  This function returns id for media sub_stream (track)
**/
uint8_t bmedia_player_stream_get_id(
		const bmedia_player_stream *stream,
		unsigned sub_stream
		);

/**
Summary:
  This function returns delta between two timestamps
**/
int bmedia_player_pos_delta(bmedia_player_pos future, bmedia_player_pos past);

/**
Summary:
  This function checks whether to timestamps are within required range
**/
bool bmedia_player_pos_in_range(bmedia_player_pos future, bmedia_player_pos past, bmedia_player_pos range);


/**
Summary:
Lookup a PTS value based on a position
**/
int bmedia_player_lookup_pts(
		bmedia_player_t player,		/* instance of bmedia_player_t */
		bmedia_player_pos pos,		/* pointer to the location */
		uint32_t *p_pts /* [out] PTS for that location */
		);


/**
Summary:
Initialize bmedia_player_status structure
**/
void bmedia_player_init_status(bmedia_player_status *status);

/**
Summary:
Set media player decoder config
**/
int bmedia_player_set_decoder_config (bmedia_player_t player, const bmedia_player_decoder_config *config);

/**
Summary:
Get media player decoder config
**/
void  bmedia_player_get_decoder_config (bmedia_player_t player, bmedia_player_decoder_config *config);

/**
Summary:
  This function initializes bmedia_player_entry structure
**/
void bmedia_player_init_entry(
		bmedia_player_entry *entry /* pointer to structure to initialize */
		);

/**
Summary:
  This function initializes bmedia_player_sync_entry structure
**/
void bmedia_player_init_sync_entry(
		bmedia_player_sync_entry *entry /* pointer to structure to initialize */
		);


/**
Summary:
  This structure defines interface that each compatible media container shall implement
**/
typedef struct bmedia_player_methods {
	void *(*create)(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream);
	void (*destroy)(void *player);
	int  (*next)(void *player, bmedia_player_entry *entry);
	void (*tell)(void *player, bmedia_player_pos *pos);
	void (*get_status)(void *player, bmedia_player_status *status);
	int (*set_direction)(void *player, bmedia_player_step direction, bmedia_time_scale time_scale, bmedia_player_decoder_mode *mode);
	int  (*seek)(void *player, bmedia_player_pos pos);
} bmedia_player_methods;


/* when reached EOS without finding right index, step back by 1000ms (e.g. 1sec) */
#define BMEDIA_PLAYER_SEEK_STEP (1000)
/* try 10 times and then fail */
#define BMEDIA_PLAYER_SEEK_TRIES    10


#ifdef __cplusplus
}
#endif


#endif /* _BMEDIA_PLAYER_H__ */

