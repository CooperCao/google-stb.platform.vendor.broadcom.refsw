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
 * BMedia library, H.265/HEVC video elementary stream probe
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BEHVC_VIDEO_PROBE_H__
#define _BEHVC_VIDEO_PROBE_H__

#include "bmedia_probe_es.h"
#include "bmedia_probe_util.h"

#ifdef __cplusplus
extern "C"
{
#endif
/*
Summary:
   This structure defines H265 specific properties of video track
*/
typedef struct bmedia_probe_h265_video {
    bh265_video_sps sps;
    bool frame_mbs_only;  /* value of  frame_mbs_only_flag from ITU-T Rec. H.265 */
} bmedia_probe_h265_video;

extern const bmedia_probe_es_desc bhevc_video_probe;

#ifdef __cplusplus
}
#endif


#endif /* _BHEVC_VIDEO_PROBE_H__ */
