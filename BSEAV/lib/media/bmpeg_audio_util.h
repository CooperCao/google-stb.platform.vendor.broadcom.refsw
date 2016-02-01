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
 * BMedia library, MPEG Audio elementary stream probe
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BMPEG_AUDIO_UTIL_H__
#define _BMPEG_AUDIO_UTIL_H__

#include "bmedia_probe_es.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define B_MPEG_AUDIO_SYNC	        0x7FF
#define B_MPEG_AUDIO_SYNC_LEN       2
#define B_MPEG_AUDIO_HEADER         (B_MPEG_AUDIO_SYNC_LEN+2)

#define B_MPEG_AUDIO_LAYER_I        0x03
#define B_MPEG_AUDIO_LAYER_II       0x02
#define B_MPEG_AUDIO_LAYER_III      0x01

#define B_MPEG_AUDIO_VERSION_1      0x03
#define B_MPEG_AUDIO_VERSION_2      0x02
#define B_MPEG_AUDIO_VERSION_2_5    0x00

#define B_MPEG_AUDIO_CHANNEL_MODE_STEREO            0x00
#define B_MPEG_AUDIO_CHANNEL_MODE_JOINT_STEREO      0x01
#define B_MPEG_AUDIO_CHANNEL_MODE_DUAL_MONO         0x02
#define B_MPEG_AUDIO_CHANNEL_MODE_MONO              0x03


/*
Summary:
   This enumeration defines the type of bitrate encoding used to encode the stream
*/
typedef enum bmp3_stream_type {
    bmp3_stream_type_unknown = 0,
    bmp3_stream_type_cbr,
    bmp3_stream_type_vbr,
    bmp3_stream_type_abr
} bmp3_stream_type;

/*
Summary:
   This structure defines MP3 audio specific properties of audio track
*/
typedef struct bmp3_vbr_frame_info {
    bmp3_stream_type stream_type;
    unsigned num_frames;
    unsigned stream_bytes;
    unsigned stream_quality;
} bmp3_vbr_frame_info;

typedef struct bmedia_mpeg_audio_info {
    uint8_t layer;
    uint8_t version;
    uint8_t channel_mode;
    uint16_t samples;
} bmedia_mpeg_audio_info;

void
bmp3_parse_vbr_frame_info(batom_cursor *cursor, const bmedia_mpeg_audio_info *mpeg_info, bmp3_vbr_frame_info *vbr_info);

size_t
bmpeg_audio_probe_parse_header(batom_cursor *cursor, uint16_t sync_word, bmedia_probe_audio *info, bmedia_mpeg_audio_info *mpeg_info);

/*
Summary:
   This structure defines MPEG audio specific properties of audio track
*/

extern const bmedia_probe_es_desc bmpeg_audio_probe;

#ifdef __cplusplus
}
#endif


#endif /* _BMPEG_AUDIO_UTIL_H__ */

