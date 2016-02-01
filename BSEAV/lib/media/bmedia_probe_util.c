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
#include "bstd.h"
#include "biobits.h"
#include "bvlc.h"
#include "bmedia_probe_util.h"
#include "bmedia_probe.h"
#include "bkni.h"

BDBG_MODULE(bmedia_probe_util);

#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */


void
bmedia_vlc_decode_init(bmedia_vlc_decode *vlc, const uint8_t *buf, size_t buf_size)
{
    BDBG_ASSERT(vlc);
    vlc->buf = buf;
    vlc->size = buf_size;
    vlc->index = 0;
    vlc->bit = 7;
    return;
}

int
bmedia_vlc_decode_read(bmedia_vlc_decode *vlc)
{
    int bit;
    BDBG_ASSERT(vlc->index < vlc->size);
    bit = b_vlc_decode(vlc->buf, vlc->size, vlc->index, vlc->bit, &vlc->index, &vlc->bit);
    BDBG_ASSERT(vlc->index < vlc->size);
    return bit;
}

int
bmedia_vlc_decode_skip(bmedia_vlc_decode *vlc)
{
    int eof ;
    BDBG_ASSERT(vlc->index < vlc->size);
    eof = b_vlc_skip(vlc->buf, vlc->size, vlc->index, vlc->bit, &vlc->index, &vlc->bit);
    BDBG_ASSERT(vlc->index < vlc->size);
    return eof;
}


int
bmedia_vlc_decode_bit(bmedia_vlc_decode *vlc)
{
    int result;

    result = (vlc->buf[vlc->index] >> vlc->bit)&1;
    if(vlc->bit) {
        vlc->bit--;
    } else {
        if(vlc->index<vlc->size) {
            vlc->index++;
            vlc->bit = 7;
        } else {
            return -1;
        }
    }
    return result;
}

int
bmedia_vlc_decode_bits(bmedia_vlc_decode *vlc, unsigned bits)
{
    int result;

    for(result=0;bits>0;bits--) {
        int val = bmedia_vlc_decode_bit(vlc);
        if(val<0) { return val;}
        result = (result << 1) | (val&1);
    }
    return result;
}

bool
b_h264_video_sps_parse(b_h264_video_sps *sps, const uint8_t *buf, size_t size)
{
    BDBG_ASSERT(buf);
    BDBG_ASSERT(sps);
    sps->compliant.baseline = false;
    sps->compliant.main = false;
    sps->compliant.extended = false;
    sps->compliant.high = false;
    sps->valid = false;
    if(size<3) {
        return false;
    }
    sps->valid = true;
    sps->profile = buf[0];
    sps->level = buf[2];
    if(B_GET_BIT(buf[1],7) && sps->profile != 66) {
        sps->compliant.baseline = true;
    }
    if(B_GET_BIT(buf[1],6) && sps->profile != 77) {
        sps->compliant.main = true;
    }
    if(B_GET_BIT(buf[1],5) && sps->profile != 88) {
        sps->compliant.extended = true;
    }
    if(B_GET_BIT(buf[1],5) && sps->profile != 100) {
        sps->compliant.high = true;
    }
    return true;
}

int
b_h264_video_sps_to_string(const b_h264_video_sps *sps, char *buf, size_t size)
{
    const char *profile;
    char profiles[64];
    size_t profiles_offset=1;
    int rc;
    char profile_number[8];

    if(!sps->valid) {
        *buf = '\0';
        return 0;
    }
    switch(sps->profile) {
    case b_h264_profile_baseline: profile="Baseline";break;
    case b_h264_profile_main: profile="Main";break;
    case b_h264_profile_extended: profile="Extended";break;
    case b_h264_profile_high: profile="High";break;
    case b_h264_profile_high10: profile="High 10";break;
    case b_h264_profile_highd422: profile="High 422";break;
    case b_h264_profile_highd444: profile="High 444";break;
    default:
        BKNI_Snprintf(profile_number, sizeof(profile_number), "%d", sps->profile);
        profile = profile_number;
        break;
    }
    if(sps->compliant.baseline && profiles_offset<sizeof(profiles)) {
       rc = BKNI_Snprintf(profiles+profiles_offset, sizeof(profiles)-profiles_offset, "%s ", "Baseline");
       if(rc>0) {
           profiles_offset+=rc;
       }
    }
    if(sps->compliant.main && profiles_offset<sizeof(profiles)) {
       rc = BKNI_Snprintf(profiles+profiles_offset, sizeof(profiles)-profiles_offset, "%s ", "Main");
       if(rc>0) {
           profiles_offset+=rc;
       }
    }
    if(sps->compliant.extended && profiles_offset<sizeof(profiles)) {
       rc = BKNI_Snprintf(profiles+profiles_offset, sizeof(profiles)-profiles_offset, "%s ", "Extended");
       if(rc>0) {
           profiles_offset+=rc;
       }
    }
    if(sps->compliant.high && profiles_offset<sizeof(profiles)) {
       rc = BKNI_Snprintf(profiles+profiles_offset, sizeof(profiles)-profiles_offset, "%s ", "High");
       if(rc>0) {
           profiles_offset+=rc;
       }
    }
    if(profiles_offset>1) {
       profiles[0]='[';
        if(profiles_offset<sizeof(profiles)) {
            profiles[profiles_offset-1]=']';
        }
    } else {
        profiles[0]='\0';
    }
    return BKNI_Snprintf(buf, size, "%s%s %u.%u", profile, profiles, sps->level/10, sps->level%10);
}

/* H.265 (04/2013) */

#define B_H265_UNUSED_FIELD(name, bits) batom_bitstream_drop_bits(bs, bits)
#define B_H265_UNUSED_VLC(name) batom_bitstream_exp_golomb(bs)

/* 7.3.3 Profile, tier and level syntax */
static bool b_h265_profile_tier_level(bh265_video_sps *sps, batom_bitstream *bs, unsigned sps_max_sub_layers_minus1)
{
    unsigned i;
    unsigned j;
    unsigned data;
    unsigned general_profile_space;
    bool sub_layer_profile_present_flag[8];
    bool sub_layer_level_present_flag[8];

    if(sps_max_sub_layers_minus1>=8) {
        BDBG_WRN(("b_h265_profile_tier_level: sps_max_sub_layers_minus1 %u not supported", sps_max_sub_layers_minus1));
        return false;
    }
    general_profile_space = batom_bitstream_bits(bs, 2);
    if(batom_bitstream_eof(bs) || general_profile_space != 0) {
        return false;
    }

    sps->general_tier_flag = batom_bitstream_bit(bs);
    sps->general_profile_idc = batom_bitstream_bits(bs, 5);
    for(j=0;j<32;j++) {
        B_H265_UNUSED_FIELD(general_profile_compatibility_flag[j], 1);
    }
    B_H265_UNUSED_FIELD(general_progressive_source_flag, 1);
    B_H265_UNUSED_FIELD(general_interlaced_source_flag, 1);
    B_H265_UNUSED_FIELD(general_non_packed_constraint_flag, 1);
    B_H265_UNUSED_FIELD(general_frame_only_constraint_flag, 1);
    /* general_reserved_zero_44bits */
    data = batom_bitstream_bits(bs, 22);
    if(batom_bitstream_eof(bs) /* || data != 0 */) {
        return false;
    }
    data = batom_bitstream_bits(bs, 22);
    if(batom_bitstream_eof(bs) /* || data != 0 */) {
        return false;
    }
    sps->general_level_idc = batom_bitstream_bits(bs, 8);
    for(i=0;i<sps_max_sub_layers_minus1;i++) {
        sub_layer_profile_present_flag[i] = batom_bitstream_bit(bs);
        sub_layer_level_present_flag[i] = batom_bitstream_bit(bs);
    }
    if(sps_max_sub_layers_minus1>0) {
        for(i=sps_max_sub_layers_minus1;i<8;i++) {
            unsigned reserved_zero_2bits = batom_bitstream_bits(bs,2);
            if(batom_bitstream_eof(bs) || reserved_zero_2bits != 0) {
                return false;
            }
        }
    }
    for(i=0;i<sps_max_sub_layers_minus1;i++) {
        if(sub_layer_profile_present_flag[i]) {
            B_H265_UNUSED_FIELD(sub_layer_profile_space[i], 2);
            B_H265_UNUSED_FIELD(sub_layer_tier_flag[i],1);
            B_H265_UNUSED_FIELD(sub_layer_profile_idc[i],5);
            for(j=0;j<32;j++) {
                B_H265_UNUSED_FIELD(sub_layer_profile_compatibility_flag[i][j], 1);
            }
            B_H265_UNUSED_FIELD(sub_layer_progressive_source_flag[ i ], 1);
            B_H265_UNUSED_FIELD(sub_layer_interlaced_source_flag[ i ], 1);
            B_H265_UNUSED_FIELD(sub_layer_non_packed_constraint_flag[ i ], 1);
            B_H265_UNUSED_FIELD(sub_layer_frame_only_constraint_flag[ i ], 1);
            /* sub_layer_reserved_zero_44bits */
            data = batom_bitstream_bits(bs, 22);
            if(batom_bitstream_eof(bs) || data != 0) {
                return false;
            }
            data = batom_bitstream_bits(bs, 22);
            if(batom_bitstream_eof(bs) || data != 0) {
                return false;
            }
        }
        if(sub_layer_level_present_flag[ i ]) {
            B_H265_UNUSED_FIELD(sub_layer_level_idc[ i ], 8);
        }
    }
    if(batom_bitstream_eof(bs)) {
        return false;
    }
    return true;
}

static bool
b_h265_seq_parameter_set_rbsp(bh265_video_sps *sps, struct bmedia_probe_video *video, batom_bitstream *bs)
{
    unsigned sps_max_sub_layers_minus1;
    unsigned chroma_format_idc;
    bool conformance_window_flag;

    /* 7.3.2.2 Sequence parameter set RBSP syntax */
    sps->sps_video_parameter_set_id = batom_bitstream_bits(bs, 4);
    sps_max_sub_layers_minus1 = batom_bitstream_bits(bs, 3);
    B_H265_UNUSED_FIELD(sps_temporal_id_nesting_flag, 1);
    if(batom_bitstream_eof(bs)) {
        return false;
    }

    if(!b_h265_profile_tier_level(sps, bs, sps_max_sub_layers_minus1)) {
        return false;
    }

    B_H265_UNUSED_VLC(sps_seq_parameter_set_id);
    chroma_format_idc = batom_bitstream_exp_golomb(bs);
    if(batom_bitstream_eof(bs)) {
        return false;
    }
    if(chroma_format_idc == 3) {
        B_H265_UNUSED_FIELD(separate_colour_plane_flag, 1);
    }
    video->width = batom_bitstream_exp_golomb(bs); /* pic_width_in_luma_samples */
    video->height = batom_bitstream_exp_golomb(bs); /* pic_height_in_luma_samples */
    conformance_window_flag = batom_bitstream_bit(bs);
    if(batom_bitstream_eof(bs)) {
        return false;
    }
    BDBG_MSG(("b_h265_seq_parameter_set_rbsp: width:%u,height:%u", video->width, video->height));
    if(conformance_window_flag) {
        B_H265_UNUSED_VLC(conf_win_left_offset);
        B_H265_UNUSED_VLC(conf_win_right_offset);
        B_H265_UNUSED_VLC(conf_win_top_offset);
        B_H265_UNUSED_VLC(conf_win_bottom_offset);
    }
    sps->bit_depth_luma = 8 + batom_bitstream_exp_golomb(bs);
    sps->bit_depth_chroma = 8 + batom_bitstream_exp_golomb(bs);
    if(batom_bitstream_eof(bs)) {
        return false;
    }

    return true;
}

bool
bh265_video_sps_parse(bh265_video_sps *sps, struct bmedia_probe_video *video, batom_cursor *cursor)
{
    batom_bitstream bs;

    BDBG_ASSERT(sps);
    BDBG_ASSERT(cursor);

    batom_bitstream_init(&bs, cursor);

    if(!b_h265_seq_parameter_set_rbsp(sps, video, &bs)) {
        return false;
    }

    sps->valid = true;
    return true;
}

int
bh265_video_sps_to_string(const bh265_video_sps *sps, char *buf, size_t size)
{
    char profile_number[8];
    const char *profile;
    switch(sps->general_profile_idc) {
    case 0: profile = "Main";break;
    case 1: profile = "Main 10";break;
    default:
        profile = profile_number;
        BKNI_Snprintf(profile_number, sizeof(profile_number),"%u", sps->general_profile_idc);
        break;
    }
    return BKNI_Snprintf(buf, size, "%s %u.%u %s", sps->general_tier_flag?"High":"Main", sps->general_level_idc/30, (sps->general_level_idc/3)%10, profile);
}
