/***************************************************************************
 *     Copyright (c) 2008 Broadcom Corporation
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
 * BMedia library, H.264/MPEG-4 Part 10 video elementary stream probe
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BAVC_VIDEO_PROBE_H__
#define _BAVC_VIDEO_PROBE_H__

#include "bmedia_probe_es.h"
#include "bmedia_probe_util.h"

#ifdef __cplusplus
extern "C"
{
#endif
/*
Summary:
   This structure defines H264 specific properties of video track
*/
typedef struct bmedia_probe_h264_video {
    b_h264_video_sps sps;
    bool frame_mbs_only;  /* value of  frame_mbs_only_flag from ITU-T Rec. H.264 */
} bmedia_probe_h264_video;

	
extern const bmedia_probe_es_desc bavc_video_probe;

#ifdef __cplusplus
}
#endif


#endif /* _BAVC_VIDEO_PROBE_H__ */

