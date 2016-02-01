/***************************************************************************
 *     Copyright (c) 2007-2013, Broadcom Corporation
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
 * BMedia library, misc utilities
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BMEDIA_PROBE_UTIL_H__
#define _BMEDIA_PROBE_UTIL_H__

#include "bioatom.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct bmedia_vlc_decode {
    const uint8_t *buf;
    size_t size;
    unsigned index;
    unsigned bit;
} bmedia_vlc_decode;

void bmedia_vlc_decode_init(bmedia_vlc_decode *vlc, const uint8_t *buf, size_t buf_size);
int bmedia_vlc_decode_read(bmedia_vlc_decode *vlc);
int bmedia_vlc_decode_skip(bmedia_vlc_decode *vlc);
int bmedia_vlc_decode_bit(bmedia_vlc_decode *vlc);
int bmedia_vlc_decode_bits(bmedia_vlc_decode *vlc, unsigned bits);

typedef enum b_h264_profile {
    b_h264_profile_baseline=66,
    b_h264_profile_main=77,
    b_h264_profile_extended=88,
    b_h264_profile_high=100,
    b_h264_profile_high10=110,
    b_h264_profile_highd422=122,
    b_h264_profile_highd444=144
} b_h264_profile;

typedef struct b_h264_video_sps {
    bool valid;
    uint8_t level;
    b_h264_profile profile;
    struct {
        bool baseline;
        bool main;
        bool extended;
        bool high;
    } compliant;
} b_h264_video_sps;

int b_h264_video_sps_to_string(const b_h264_video_sps *sps, char *buf, size_t size);
bool b_h264_video_sps_parse(b_h264_video_sps *sps, const uint8_t *buf, size_t size);


typedef struct bh265_video_sps {
    bool valid;
    bool general_tier_flag;
    uint8_t sps_video_parameter_set_id;
    uint8_t general_level_idc;
    uint8_t general_profile_idc;
    uint8_t bit_depth_luma;
    uint8_t bit_depth_chroma;
} bh265_video_sps;

struct bmedia_probe_video;

int bh265_video_sps_to_string(const bh265_video_sps *sps, char *buf, size_t size);
bool bh265_video_sps_parse(bh265_video_sps *sps, struct bmedia_probe_video *video, batom_cursor *cursor);

#ifdef __cplusplus
}
#endif

#endif /* _BMEDIA_PROBE_UTIL_H__ */

