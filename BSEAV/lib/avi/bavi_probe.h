/***************************************************************************
 *     Copyright (c) 2007-2009, Broadcom Corporation
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
#ifndef _BAVI_PROBE_H__
#define _BAVI_PROBE_H__

#include "bmedia_probe_impl.h"
#include "bavi_util.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum bavi_subtitle_type {
	bavi_subtitle_type_dxsa,
	bavi_subtitle_type_dxsb,
	bavi_subtitle_type_none
} bavi_subtitle_type;

typedef struct bavi_probe_stream {
	  bmedia_probe_stream media;
      unsigned video_framerate; /* framerate of video stream, in 1000 FPS, derived from the dwMicroSecPerFrame in AVI mainframe */
      unsigned suggestedBufferSize; /* value of dwSuggestedBufferSize from  the AVI header */
      off_t max_av_byte_delta; /* the maximum byte difference between a single audio and video frame of the same PTS. -1 if unknown */ 
} bavi_probe_stream;

typedef struct bavi_probe_track {
	  bmedia_probe_track media;
	  uint8_t *private_data;
	  unsigned private_data_length;
	  bavi_subtitle_type subtitle_type;
	  unsigned duration; /* duration of stream, in milliseconds or 0 if unknown */
      unsigned suggestedBufferSize; /* value of dwSuggestedBufferSize from  the stream header */
      unsigned averageBitrate; /* for audio streams, this is WAVEFORMATEX.nAvgBytesPerSec*8, otherwise it's 0 */
      uint32_t media_type; /* FOURCC for video track or wFormatTag for audio tracks */
	  bool encryptedContentFlag;
	  char language[16]; 
} bavi_probe_track;
		

extern const bmedia_probe_format_desc bavi_probe;

#ifdef __cplusplus
}
#endif


#endif /* _BAVI_PROBE_H__ */

