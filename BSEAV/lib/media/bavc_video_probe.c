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
#include "bstd.h"
#include "bavc_video_probe.h"
#include "bkni.h"
#include "biobits.h"
#include "bmedia_util.h"

BDBG_MODULE(bavc_video_probe);

#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x) */


typedef struct bmedia_probe_video_avc_video *bavc_video_probe_t;
BDBG_OBJECT_ID(bavc_video_probe_t);

struct bmedia_probe_video_avc_video {
    BDBG_OBJECT(bavc_video_probe_t)
    batom_accum_t acc;
    unsigned sps;       /* counts all occurrences within stream */
    unsigned pps;       /* counts all occurrences within stream */
    unsigned ssps;      /* counts all occurrences within stream */
    unsigned sh;        /* counts only those occurrences with matching SPS and PPS */
    unsigned iframe;    /* counts only those occurrences within slice header */
    int last_sps;
    int last_pps;
    bmedia_probe_video info;
    bvideo_codec codec;
    union {
        struct {
            uint8_t buf[200];
        } sps;
        struct {
            uint8_t packet[128];
            uint8_t message[128];
        } sei;
    } data;
};

void 
b_avc_video_probe_reset(bmedia_probe_base_es_t probe)
{
    bavc_video_probe_t avc_video = (bavc_video_probe_t)probe;
    BDBG_OBJECT_ASSERT(avc_video, bavc_video_probe_t);
    batom_accum_clear(avc_video->acc);
    avc_video->sps = 0;
    avc_video->ssps = 0;
    avc_video->pps = 0;
    avc_video->sh = 0;
    avc_video->iframe = 0;    
    avc_video->last_sps = -1;
    avc_video->last_pps = -1;    
    avc_video->codec = bvideo_codec_h264;
    bmedia_probe_video_init(&avc_video->info);
    ((bmedia_probe_h264_video *)&avc_video->info.codec_specific)->sps.valid = false;
    return;
}

bmedia_probe_base_es_t 
b_avc_video_probe_create(batom_factory_t factory)
{
    bavc_video_probe_t avc_video;
    BDBG_ASSERT(factory);

    avc_video = BKNI_Malloc(sizeof(*avc_video));
    if(!avc_video) {
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(avc_video, bavc_video_probe_t);
    avc_video->acc = batom_accum_create(factory);
    if(!avc_video->acc) {
        goto err_acc;
    }
    b_avc_video_probe_reset((bmedia_probe_base_es_t)avc_video);
    return (bmedia_probe_base_es_t)avc_video;
err_acc:
    BKNI_Free(avc_video);
err_alloc:
    return NULL;
}

void 
b_avc_video_probe_destroy(bmedia_probe_base_es_t probe)
{
    bavc_video_probe_t avc_video = (bavc_video_probe_t)probe;
    BDBG_OBJECT_ASSERT(avc_video, bavc_video_probe_t);
    batom_accum_destroy(avc_video->acc);
    BDBG_OBJECT_DESTROY(avc_video, bavc_video_probe_t);
    BKNI_Free(avc_video);
    return;
}

/* static */ void b_avc_video_set_frame_mbs_only(bmedia_probe_h264_video *info, bool frame_mbs_only)
{
    info->frame_mbs_only = frame_mbs_only;
    return;
}

static bool
b_avc_video_parse_sps(bavc_video_probe_t avc_video, const batom_cursor *data, bmedia_probe_video *info, bool *eof, int *sps)
{
    int val;
    unsigned profile_idc;
    int frame_mbs_only;
    batom_cursor cursor;
    unsigned chroma_format_idc = 1;
    bmedia_vlc_decode vlc;


    BATOM_CLONE(&cursor, data);
    *eof = false;
    if (sizeof(avc_video->data.sps.buf)!=batom_cursor_copy(&cursor, avc_video->data.sps.buf, sizeof(avc_video->data.sps.buf))) { 
        *eof = true;
        return false;
    }
        
    BDBG_CASSERT(sizeof(b_h264_video_sps)<= sizeof(info->codec_specific));
    b_h264_video_sps_parse(&((bmedia_probe_h264_video *)&info->codec_specific)->sps, avc_video->data.sps.buf, sizeof(avc_video->data.sps.buf));


    /* ITU-T Rec. H.264 | ISO/IEC 14496-10: 2007, Table 7.3.2.1.1 Sequence parameter set data syntax */
    bmedia_vlc_decode_init(&vlc, avc_video->data.sps.buf, sizeof(avc_video->data.sps.buf));
    profile_idc = avc_video->data.sps.buf[0];
    BDBG_MSG_TRACE(("b_avc_video_parse_sps: profile_idc = %d", profile_idc));

    /* skip over profile_idc, constraint_set?_flag's, reserved_zero_4bits and level_idc */
    vlc.index = 3;
    vlc.bit = 7;
    
    val = bmedia_vlc_decode_read(&vlc); /* seq_parameter_set_id */
    if(val<0) { goto err_sps;}
    BDBG_MSG_TRACE(("b_avc_video_parse_sps: seq_parameter_set_id = %d", val));
    *sps = val;
    if (profile_idc == 100 || profile_idc == 110 || profile_idc == 122 || 
        profile_idc == 244 || profile_idc == 44 || profile_idc == 83 || profile_idc == 86) {
        chroma_format_idc = bmedia_vlc_decode_read(&vlc); /* chroma_format_idc */
        if (chroma_format_idc == 3) {
            bmedia_vlc_decode_bit(&vlc);
        }
        bmedia_vlc_decode_skip(&vlc); /* bit_depth_luma_minus8 */
        bmedia_vlc_decode_skip(&vlc); /* bit_depth_chroma_minus8 */
        bmedia_vlc_decode_bit(&vlc);  /* qpprime_y_zero_transform_bypass_flag */
        val = bmedia_vlc_decode_bit(&vlc); /* seq_scaling_matrix_present_flag */
        if(val<0) { goto err_sps;}
        if (val) {
            unsigned listIdx,listSize,coeffIdx;
            int lastScale,nextScale;
            for (listIdx=0; listIdx<((chroma_format_idc!=3)?8:12); listIdx++) {
                val = bmedia_vlc_decode_bit(&vlc); /* seq_scaling_list_present_flag */
                if(val<0) { goto err_sps;}
                if (val) {
                    listSize = (listIdx<6)?16:64;
                    lastScale = nextScale = 8;
                    for (coeffIdx=0; coeffIdx<listSize; coeffIdx++) {
                        if (nextScale != 0) {      
                            val = bmedia_vlc_decode_read(&vlc); /* delta_scale */
                            if(val<0) { goto err_sps;}
                            val = ((val%2)?1:-1) * ((val+1)>>1); /* convert val from ue(v) to se(v) */
                            nextScale = (lastScale + val + 256) % 256;
                        }
                        lastScale = (nextScale == 0) ? lastScale : nextScale;
                    }   
                }
            }
        }
    }
    bmedia_vlc_decode_skip(&vlc);  /* log2_max_frame_num_minus4 */
    val = bmedia_vlc_decode_read(&vlc); /* pic_order_cnt_type */
    if(val<0) { goto err_sps;}
    if (val == 0) {
        bmedia_vlc_decode_skip(&vlc);  /* log2_max_pic_order_cnt_lsb_minus4 */
    } else if (val == 1) {
        int i;
        bmedia_vlc_decode_bit(&vlc); /* delta_pic_order_always_zero_flag */
        bmedia_vlc_decode_skip(&vlc); /* offset_for_non_ref_pic */
        bmedia_vlc_decode_skip(&vlc); /* offset_for_top_to_bottom_field */
        val = bmedia_vlc_decode_read(&vlc); /* num_ref_frames_in_pic_order_cnt_cycle */
        if(val<0) { goto err_sps;}
        for (i = 0; i < val; i++) {
            int is_eof = bmedia_vlc_decode_skip(&vlc); /* offset_for_ref_frame[i] */
            if(is_eof) { goto err_sps;}
        }
    }
    
    bmedia_vlc_decode_skip(&vlc);  /* num_ref_frames */
    bmedia_vlc_decode_bit(&vlc); /* gaps_in_frame_num_value_allowed_flag */
    val = bmedia_vlc_decode_read(&vlc); /* pic_width_in_mbs_minus1 */
    if(val<0) { goto err_sps;}
    BDBG_MSG_TRACE(("b_avc_video_parse_sps: pic_width_in_mbs_minus1 = %u", val));
    info->width = (val+1) * 16;
    val = bmedia_vlc_decode_read(&vlc); /* pic_height_in_map_units_minus1 */
    if(val<0) { goto err_sps;}
    BDBG_MSG_TRACE(("b_avc_video_parse_sps: pic_height_in_map_units_minus1 = %u", val));

    frame_mbs_only = bmedia_vlc_decode_bit(&vlc); /* frame_mbs_only_flag */
    if(frame_mbs_only<0) { goto err_sps;}
    b_avc_video_set_frame_mbs_only((void *)&info->codec_specific, frame_mbs_only!=0);

    BDBG_MSG_TRACE(("b_avc_video_parse_sps: frame_mbs_only_flag = %u", frame_mbs_only));
    info->height = 16 * (2 - frame_mbs_only) * (val+1);
    if (!frame_mbs_only) {
        bmedia_vlc_decode_bit(&vlc); /* mb_adaptive_frame_field_flag */
    }
    bmedia_vlc_decode_bit(&vlc); /* direct_8x8_inference_flag */
    val = bmedia_vlc_decode_bit(&vlc); /* frame_cropping_flag */
    if(val<0) { goto err_sps;}
    if (val) {
        unsigned CropUnitX, CropUnitY;
        unsigned right_offset, left_offset;
        unsigned bottom_offset, top_offset;    
       
        if (chroma_format_idc==1) {
          CropUnitX = 2;
          CropUnitY = 2*(2-frame_mbs_only);
        } else if (chroma_format_idc==2) {
          CropUnitX = 2;
          CropUnitY = (2-frame_mbs_only);
        }
        else {
          CropUnitX = 1;
          CropUnitY = (2-frame_mbs_only);
        }
            
        left_offset   = bmedia_vlc_decode_read(&vlc); /* frame_crop_left_offset */
        right_offset  = bmedia_vlc_decode_read(&vlc); /* frame_crop_right_offset */
        top_offset    = bmedia_vlc_decode_read(&vlc); /* frame_crop_top_offset */
        bottom_offset = bmedia_vlc_decode_read(&vlc); /* frame_crop_bottom_offset */
        info->width  -= CropUnitX * (right_offset  + left_offset);
        info->height -= CropUnitY * (bottom_offset + top_offset );
    }

    BDBG_MSG(("b_avc_video_parse_sps: final picture dimensions = %ux%u", info->width, info->height));
    return true;

err_sps:
    return false;
}

static bmedia_probe_track *
b_avc_video_probe_make_track(bavc_video_probe_t avc_video)
{
    bmedia_probe_track *track;
    track = BKNI_Malloc(sizeof(*track));
    batom_accum_clear(avc_video->acc);
    /* found all frames */
    if(track) {
        bmedia_probe_track_init(track);
        avc_video->info.codec = avc_video->codec;
        track->type = bmedia_track_type_video;
        track->info.video = avc_video->info;
    }
    return track;
}

static uint32_t
b_avc_video_probe_scan_nal_header(batom_cursor *cursor)
{
    return bmedia_video_scan_scode(cursor, 0xFFFFFFFFul);
}

static bmedia_probe_track *
b_avc_video_probe_feed(bmedia_probe_base_es_t probe, batom_t atom, bool *done)
{
    bavc_video_probe_t avc_video = (bavc_video_probe_t)probe;
    batom_accum_t acc;
    batom_cursor cursor;
    size_t pos = 0;
    bmedia_probe_video info;
    uint8_t bytes[7]; /* worst case is Slice Header with 51 bits */
    uint32_t word;
    int val1, val2;
    bmedia_vlc_decode vlc;

    BDBG_OBJECT_ASSERT(avc_video, bavc_video_probe_t);

    bmedia_probe_video_init(&info);
    val1 = val2 = 0;
    acc = avc_video->acc;
    batom_accum_add_atom(acc, atom);
    batom_cursor_from_accum(&cursor, acc);
    
    for(;;) {
        uint32_t nal_header;
        bool valid;
        bool eof = false;

        nal_header = b_avc_video_probe_scan_nal_header(&cursor);
        pos = batom_cursor_pos(&cursor);
        BDBG_MSG_TRACE(("%s:%#lx nal_header %#x (%x %x %u) at %u", "b_avc_video_probe_feed", (unsigned long)avc_video, nal_header, B_GET_BIT(nal_header, 7), B_GET_BITS(nal_header, 6,5), B_GET_BITS(nal_header, 4,0), pos));
        if(nal_header==0) {
            goto eof;
        }

        if(B_GET_BIT(nal_header,7)) { /* forbidden_zero_bit */
            *done = true;
            goto flush;
        } 	
        switch(B_GET_BITS(nal_header, 4, 0)) { /* nal_unit_type */
        case 7: /* SPS */
            if (B_GET_BITS(nal_header, 6, 5) == 0) { /* nal_ref_idc */
                *done = true;
                goto flush;
            }
            valid = b_avc_video_parse_sps(avc_video, &cursor, &info, &eof, &val1);
            if(eof) {
                goto eof;
            }
            if(!valid) {
                *done = true;
                goto flush;
            }
#if BDBG_DEBUG_BUILD
            {
                char str[64];
                b_h264_video_sps_to_string(&((bmedia_probe_h264_video *)&info.codec_specific)->sps,  str, sizeof(str));
                BDBG_MSG(("%s:%#lx found SPS:%s, sps_id=%u", "b_avc_video_probe_feed", (unsigned long)avc_video, str, val1));
            }
#endif

            avc_video->sps++;
            if (avc_video->last_sps != val1 && !avc_video->sh) { /* reset */
                avc_video->last_sps = val1;
                avc_video->last_pps = -1;
                avc_video->sh = 0;
                avc_video->iframe = 0;
                avc_video->info = info;
            }
            else if (avc_video->last_sps == val1) {
                if (info.width != avc_video->info.width || info.height != avc_video->info.height) { /* reset */
                    avc_video->last_sps = -1;
                    avc_video->last_pps = -1;
                    avc_video->sh = 0;
                    avc_video->iframe = 0;
                }
            }
            break;
        case 8: /* PPS */
            if (B_GET_BITS(nal_header, 6, 5) == 0) {
                *done = true;
                goto flush;
            }
            if(4!=batom_cursor_copy(&cursor, bytes, 4)) { 
                goto eof;
            }
            bmedia_vlc_decode_init(&vlc, bytes, 4);
            val1 = bmedia_vlc_decode_read(&vlc); /* pic_parameter_set_id */
            val2 = bmedia_vlc_decode_read(&vlc); /* seq_parameter_set_id */
            BDBG_MSG(("%s:%#lx found PPS, pps_id=%u, sps_id=%u", "b_avc_video_probe_feed", (unsigned long)avc_video, (unsigned)val1, (unsigned)val2));
            avc_video->pps++;
            if (avc_video->last_sps==val2 && !avc_video->sh){
                avc_video->last_pps = val1;
            }
            break;
        case 5: /* SH for IDR picture */
            if (B_GET_BITS(nal_header, 6, 5) == 0) {
                *done = true;
                goto flush;
            }
        /* fall through */
        case 1: /* SH for non-IDR picture */            
            if(sizeof(bytes)!=batom_cursor_copy(&cursor, bytes, sizeof(bytes))) { 
                goto eof;
            }
            bmedia_vlc_decode_init(&vlc, bytes, 7);
            bmedia_vlc_decode_skip(&vlc);
            val1 = bmedia_vlc_decode_read(&vlc); /* slice_type */
            val2 = bmedia_vlc_decode_read(&vlc); /* pic_parameter_set_id */
            BDBG_MSG(("%s:%#lx found SH, pps_id=%u", "b_avc_video_probe_feed", (unsigned long)avc_video, (unsigned)val2));
            if (avc_video->last_pps == val2) {
                avc_video->sh++;
                if (val1 == 2 || val1 == 7) { /* I frame */
                    avc_video->iframe++;
                }
            }
            if ((avc_video->sps >= 2) && avc_video->pps && avc_video->iframe) {
                BDBG_MSG(("%s:%#lx SPS:%u PPS:%u, SH:%u, I-Frame:%u", "b_avc_video_probe_feed", (unsigned long)avc_video, avc_video->sps, avc_video->pps, avc_video->sh, avc_video->iframe));
                return b_avc_video_probe_make_track(avc_video);
            }
            break;
#if 0
	    case 14: /* SVC or MVC extension Header */
            byte = batom_cursor_byte(&cursor);
            if(BATOM_IS_EOF(&cursor)) {
                goto eof;
            }
	        avc_video->svc_extension_flag = B_GET_BIT(byte, 7);
	        break;
#endif
        case 15: /* SSPS */
            if(sizeof(bytes)!=batom_cursor_copy(&cursor, bytes, sizeof(bytes))) { 
                goto eof;
            }
            avc_video->ssps++;
            break;
        case 20: /* coded slice extension */
            word = batom_cursor_uint24_be(&cursor);
            if(BATOM_IS_EOF(&cursor)) {
                goto eof;
            }
            avc_video->codec = B_GET_BIT(word, 23) ? bvideo_codec_h264_svc : bvideo_codec_h264_mvc;
            break;
        default:
            break;
        }
    }
eof:
    BDBG_MSG_TRACE(("%s:%#lx trim %u", "b_avc_video_probe_feed", (unsigned long)avc_video, pos));
    if(pos>B_MEDIA_VIDEO_SCODE_LEN) {
        batom_cursor_from_accum(&cursor, acc);
        batom_cursor_skip(&cursor, pos-B_MEDIA_VIDEO_SCODE_LEN);
        batom_accum_trim(acc, &cursor);
    }
done:
    return NULL;
flush:
    batom_accum_clear(acc);
    goto done;
}

static bmedia_probe_track *
b_avc_video_probe_last(bmedia_probe_base_es_t probe, unsigned *probability)
{
    bavc_video_probe_t avc_video = (bavc_video_probe_t)probe;

    BDBG_OBJECT_ASSERT(avc_video, bavc_video_probe_t);
    BDBG_MSG(("b_avc_video_probe_last:%#lx sps:%u:%d pps:%u:%d ssps:%u iframe:%u %s", (unsigned long)avc_video, avc_video->sps, avc_video->last_sps, avc_video->pps, avc_video->last_pps, avc_video->ssps, avc_video->iframe, avc_video->codec!=bvideo_codec_h264?"extension":""));
    if (avc_video->sps && avc_video->pps) {
        char str[64];
        const bmedia_probe_h264_video *h264_video = (bmedia_probe_h264_video *)&avc_video->info.codec_specific;
        b_h264_video_sps_to_string(&h264_video->sps,  str, sizeof(str));
        BDBG_MSG(("b_avc_video_probe_last: SPS:%s", str));
        if (avc_video->iframe) {
            *probability = 90;
        }
        else {
            *probability = 50;
        }
        return b_avc_video_probe_make_track(avc_video);
    } else if(avc_video->ssps && avc_video->codec!=bvideo_codec_h264) {
       *probability = 50;
        return b_avc_video_probe_make_track(avc_video);
    }
    else {
        *probability = 0;
        return NULL;
    }
}

static const bmedia_probe_file_ext b_avc_video_ext[] =  {
    {"es"}, {"ves"}, {"mpg"}, {"avc"}, {"26l"}, {"264"}, 
    {""}
};

const bmedia_probe_es_desc bavc_video_probe = {
    bmedia_track_type_video,
    {
        bvideo_codec_h264
    },
    b_avc_video_ext,
    ((20*(1000*1000))/8)*2, /* parse 2 seconds of 20MBps stream */
    b_avc_video_probe_create,
    b_avc_video_probe_destroy,
    b_avc_video_probe_feed,
    b_avc_video_probe_reset,
    b_avc_video_probe_last
};
