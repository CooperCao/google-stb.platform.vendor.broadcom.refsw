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
#ifndef _BMPEG_AUDIO_PROBE_H__
#define _BMPEG_AUDIO_PROBE_H__

#include "bmedia_probe_es.h"
#include "bmpeg_audio_util.h"

#ifdef __cplusplus
extern "C"
{
#endif


extern const bmedia_probe_es_desc bmpeg_audio_probe;
/* custom version of feed function that also parses VBR information */
bmedia_probe_track *bmpeg_audio_probe_feed(bmedia_probe_base_es_t probe, batom_t atom, bmedia_mpeg_audio_info *mpeg_info, bmp3_vbr_frame_info *vbr_info);

#ifdef __cplusplus
}
#endif


#endif /* _BMPEG_AUDIO_PROBE_H__ */

