/***************************************************************************
 *     Copyright (c) 2007-2011, Broadcom Corporation
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
 * Media filter library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/

#ifndef _BMEDIA_FILTER_H__
#define _BMEDIA_FILTER_H__

#include "bmedia_types.h"
#include "bioatom.h"
#include "bmedia_util.h"
#if B_HAS_ASF
#include "basf_stream.h"
#endif
#if B_HAS_AVI
#include "bavi_parser.h"
#include "bavi_stream.h"
#endif
#include "bmp4_fragment_demux.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*================== Module Overview =====================================
This module provides multi-format stream processing library. 
Function of this module is to convert the various multimedia container formats (ASF, AVI, MPEG-1 System, etc) into the MPEG-2 PES stream
This module used batom framwework and accomplished the following goals:
 1.	In-place conversion (e.g. zero memory copy parsing), where stream is separated in pieces and rejoined into a MPEG-2 PES stream
 2.	Asynchronous non-blocking model of operation

Purpose of thio module to provide unified stream processing for multiple stream parsers library.
For example AVI parsing is done in two different modules basf_parser and basf_stream, and trickmodes are managed by basf_player . This module knows
intrinsic details of those modules and does connect them together in a way that provides unified interface.

========================================================================*/

/**
Summary:
  bmedia_filter_t is a element that encapsulates control under stream processing
Description:
  bmedia_filter_t is a file level processing element and it's used for file level operations, for example
    bmedia_filter_stop, bmedia_filter_start or bmedia_filter_seek
**/
typedef struct bmedia_filter *bmedia_filter_t;

/**
Summary:
  bmedia_stream_t is a element that abstracts single stream (e.g. track) in the file
Description:
 Usually one file has multiple sub-streams (tracks), for example AVI file would have
 audio and video streams, such tracks are indentified by unique integer. 
 The bmedia_stream_t is used to control such sub-streams (tracks).
**/
typedef struct bmedia_stream *bmedia_stream_t;

/** 
Summary:
  Application specific configuration 
**/
typedef struct bmedia_filter_cfg  {
	  const void *eos_data; /* this is pointer to the data which is appended at the end of stream, primary function of this data that it would propagate data in the decoder's pipeline */
	  size_t eos_len;  /* size of the end-of-stream data */
      bool sync_streams_after_seek; /* sync audio and video of streams after the a seek has been done in the stream */
	  void *application_cnxt; /* application specific context */
#if B_HAS_ASF
	  void (*update_asf_stream_cfg)(void *application_cnxt, unsigned stream_id, basf_stream_cfg *cfg); /* this callback could be used by application to provide custom configuration for the basf_stream object */
#endif
#if B_HAS_AVI
	  struct {
		    void (*activate_avi_stream)(void *application_cnxt, unsigned stream_id, bavi_parser_t parser, bavi_stream_t stream); /* this callback could be used by application to provide custom processing for AVI streams */
		    void (*deactivate_avi_stream)(void *application_cnxt, unsigned stream_id, bavi_parser_t parser, bavi_stream_t stream); /* this callback could be used by application to release resources used for custom processing for AVI streams */
	  } avi[2];
#endif
      void (*update_mp4_fragment_cfg)(void *application_cnxt, bmp4_fragment_demux_cfg *cfg);
      void (*stream_error)(void *cntx); /* callback that was called when error detectect, this callback can't call back into the filter */
} bmedia_filter_cfg;

/** 
Summary:
  Application specific per-stream configuration 
**/
typedef struct bmedia_stream_cfg {
    bool single_pts; /* send only single pts */
    bool reorder_timestamps; /* reorder timestamps from display into the decode order */
} bmedia_stream_cfg;

/** 
Summary:
 	Status of the stream processing.  
**/
typedef struct bmedia_filter_status {
	uint64_t offset; /* current offset in a stream */
	size_t acc_length; /* number of accumulated and not parsed bytes */
	size_t obj_length; /* size of the object that parser expects */
	const char *state; /* state of the parser */
    bmedia_parsing_errors errors; /* parsing errors */
    bool last_pts_valid;
    uint32_t last_pts;  /* last PTS (in 45KHz units) that was produced by the filter */
} bmedia_filter_status;

/**
Summary:
  Initializes  bmedia_filter_cfg structure with default values.
Description:
  To allow extention of API, application must call bmedia_filter_default_cfg function before accessing members 
  of the bmedia_filter_cfg structure.
**/
void bmedia_filter_default_cfg(bmedia_filter_cfg *cfg);


/**
Summary:
  Initializes  bmedia_stream_cfg structure with default values.
Description:
  To allow extention of API, application must call bmedia_stream_default_cfg function before accessing members 
  of the bmedia_stream_default_cfg structure.
**/
void bmedia_stream_default_cfg(bmedia_stream_cfg *cfg);

/**
Summary:
  Creates instance of the bmedia_filter_t and allocates all necessary resources.
Description:
  This function creates new instance of bmedia_filter_t or NULL, if resources can't be allocated
**/
bmedia_filter_t bmedia_filter_create(
		batom_factory_t factory,  /* instance of batom_factory_t that is used to allocate data processing elements */
		balloc_iface_t data_alloc,  /* memory allocation interface that is used to allocate memory for stream elements, e.g. for PES headers */
		const bmedia_filter_cfg *cfg	/* application specific configuration */
		);

/**
Summary:
  Releases resources allocation for the bmedia_filter_t
**/
void bmedia_filter_destroy(
		bmedia_filter_t filter /* instance of the bmedia_filter_t */
		);


/*
Summary:
  Prepares bmedia_filter for data processing 
Description:
  Function bmedia_filter_start sets up internal state of the bmedia_filter_t instance such it could take media stream and produce MPEG-2 PES stream.
  In case of success this function returns instance of batom_pipe_t object that application shall use to retrieve MPEG-2 PES stream, in case of error
  NULL is returned.
*/
batom_pipe_t 
bmedia_filter_start(
		bmedia_filter_t filter, /* instance of the bmedia_filter_t */
		bstream_mpeg_type type  /* stream type */
		);

/**
Summary:
  Adds new sub-stream(track) for further processing 
Description:
  The bmedia_filter module only constructs MPEG-2 PES stream for sub-streams(tracks) that are registered.
  This function returns instance of bmedia_stream_t or NULL is new sub-stream(track) can't be registered.
**/
bmedia_stream_t 
bmedia_filter_stream_open(
		bmedia_filter_t filter,  /* instance of the bmedia_filter_t */
		unsigned stream_id, 	 /* number that uniquely identifies sub-stream(track) */
		uint16_t pes_id,	     /* MPEG-2 PES stream id that would be used to PES packets that corrspont to the selected sub-stream(track) */
        const bmedia_stream_cfg *stream_cfg /* optional extra configuration for the stream, NULL if default should be used */
		);

/**
Summary:
   Removes sub-stream(track) from further processing
**/
void 
bmedia_filter_stream_close(
		bmedia_filter_t filter,	/* instance of the bmedia_filter_t */
		bmedia_stream_t stream	/* instance of the bmedia_stream_t */
		);

/**
Summary:
   Returns current configuation for the sub-stream(track) processing
**/
void 
bmedia_filter_stream_get_config(
		bmedia_stream_t stream,	/* instance of the bmedia_stream_t */
        bmedia_stream_cfg *cfg
		);

/**
Summary:
   Sets configuation used fore the sub-stream(track) processing
**/
int
bmedia_filter_stream_set_config(
		bmedia_stream_t stream,	/* instance of the bmedia_stream_t */
        const bmedia_stream_cfg *cfg
		);

/**
Summary:
  Feeds a data into the stream processor
Description:
  The bmedia_filter_feed function feeds a data into the stream processor. The bmedia_filter_feed (or internal processing elements) extract 
  data from the user provided instance of batom_pipe_t.
  This function returns 'true' if some data was processed and false if for some resonse (e.g. resource starvation) no data was processed.
**/
bool 
bmedia_filter_feed(
		bmedia_filter_t filter, /* instance of the bmedia_filter_t */
		batom_pipe_t pipe		/* instance of the batom_pipe_t with the stream data*/
		);

/**
Summary:
   Stops further data processing
**/
void bmedia_filter_stop(
		bmedia_filter_t filter /* instance of the bmedia_filter_t */
		);

/**
Summary:
   Returns status of stream processing
**/
void bmedia_filter_get_status(
		bmedia_filter_t filter, 		/* instance of the bmedia_filter_t */ 
		bmedia_filter_status *status	/* pointer to structure to be filled */
		);

/**
Summary:
   This function is used to notify parser that new data would come with that file offset
Description:
   Due to nature of many mutlimedia containers in-order to correcly parse container, it's necessary
   to know from what offset data was originated. This function allows application to perform random
   navigation in the container and still allow parsing of multimedia container.
Result:
   0 - on success
   megative - on error
**/
int 
bmedia_filter_seek(
		bmedia_filter_t filter, /* instance of the bmedia_filter_t */ 
		int64_t off				/* new offset into data */
		);

/**
Summary:
   This function is used to notify parser about new offset in the stream presentation time
Description:
   In some stream presentation time of object (e.g. audio or video frame) depend on it's position
   in the stream. This function  is usually used after bmedia_filter_seek to notify parser
   about changes in the stream presentation time.
**/
void bmedia_filter_set_offset(
		bmedia_filter_t filter, /* instance of the bmedia_filter_t */ 
		uint32_t timestamp,		/* new timestamp offset in 45KHz units */
		uint16_t  stream_id
		);

/**
Summary:
   This function flushes (e.g. removes) all data accumulated in the stream processing unit
**/
void bmedia_filter_flush(
        bmedia_filter_t filter
        );

/**
Summary:
   This function clears (e.g. frees) all data queued in the stream processing unit, without affecting it's state
**/
void bmedia_filter_clear(
        bmedia_filter_t filter
        );

/**
Summary:
   This function is used to control presentation rate of the stream
Description:
   The bmedia_filter_set_timescale function  is used to control how relation
   between rate of MPEG-2 PES timestamps and timestamps in the multimedia container.
   This function is intended to be used in trickmodes in order to speed-up or slow-down
   visual decoding speed.
**/
void bmedia_filter_set_timescale(
        bmedia_filter_t filter, 
        bmedia_time_scale time_scale
        );

/**
Summary:
   This function controls filter for key(e.g. I frames)
Description:
   For certain media formats (e.g. ASF) exact location of key frame is unknown, for this formats
   this function is used to activate a filter that is causes to pass only single I frame (and only
   I frame) between consecutive calls to bmedia_filter_seek. The bmedia_filter_set_keyframeonly
   is indentded to be used in trickmodes.
**/
void bmedia_filter_set_keyframeonly(
        bmedia_filter_t filter, 
        bool key_frame_only
        );

/**
Summary:
   This function determines which stream is the master stream when resyncing
Description:
   This API is use to inform the filter which stream should be considered the 
   master for operations such as resyncing the streams.
**/
void bmedia_filter_set_master_stream(
        bmedia_filter_t filter, 
        uint32_t stream_id
        );

/**
Summary:
   This function controls filter for syncing to a specific time.
Description:
   After performing a seek or jump in the stream, we cannot be certin what the A/V muxing 
   in the stream will look like. In certin streams, at file offset X, audio and video packets 
   might have PTS values that are easily off by more than a second.  If we send all data packets 
   at file offset X to the decoder we run the risk of confusing it with audio and video from 
   very different times. To solve this we can sync all streams to a specifc time and drop all
   packets from before that time. the time we are syncing to will be the earliest PTS in all 
   open streams which is:
      1) After the time provided by the application through this API
      2) After the next key frame of the master stream which is set by bmedia_filter_set_master_stream().
**/
void bmedia_filter_sync_after_time(
        bmedia_filter_t filter, 
        uint32_t time
        );

#ifdef __cplusplus
}
#endif

#endif /* _BMEDIA_FILTER_H__ */

