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
 * BMedia library, MPEG Video elementary stream probe
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BMPEG_VIDEO_PROBE_H__
#define _BMPEG_VIDEO_PROBE_H__

#include "bmedia_probe_es.h"

#ifdef __cplusplus
extern "C"
{
#endif
	
extern const bmedia_probe_es_desc bmpeg_video_probe;
typedef struct bmedia_probe_mpeg_video {
    bool valid; /* fields below are valid, only if this is set to true */
    unsigned framerate; /* framerate of video stream, in 1000 FPS, derived from the frame_rate_code, 0 if unknown */
	uint16_t seq_scode_count; /* number of sequence start codes seen when parsing payload */
	uint16_t pic_scode_count; /* number of picture start codes seen when parsing payload */
    uint8_t profile;  /* ISO/IEC 13818-2 , Table 8-2 . Profile identification */
    uint8_t level; /* ISO/IEC 13818-2 , Table 8-3 . Level identification */
    bool progressive_sequence; /*  ISO/IEC 13818-2, Sequence extension, progressive_sequence */ 
    bool low_delay;  /*  ISO/IEC 13818-2, Sequence extension, low_delay */ 
} bmedia_probe_mpeg_video;


#ifdef __cplusplus
}
#endif


#endif /* _BMPEG_VIDEO_PROBE_H__ */

