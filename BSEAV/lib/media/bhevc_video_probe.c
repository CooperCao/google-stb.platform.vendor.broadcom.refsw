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
#include "bstd.h"
#include "bhevc_video_probe.h"
#include "bkni.h"
#include "biobits.h"
#include "bmedia_util.h"

BDBG_MODULE(bhevc_video_probe);

#define BDBG_MSG_TRACE(x) BDBG_MSG(x)

typedef struct bmedia_probe_video_hevc_video *bhevc_video_probe_t;
BDBG_OBJECT_ID(bhevc_video_probe_t);
typedef struct bh265_video_pps {
    bool valid;
    uint8_t pic_parameter_set_id;
    uint8_t seq_parameter_set_id;
    bool dependent_slice_segments_enabled_flag;
    uint8_t num_extra_slice_header_bits;
} h265_video_pps;

struct bmedia_probe_video_hevc_video {
    BDBG_OBJECT(bhevc_video_probe_t)
    batom_accum_t acc;
    unsigned sps;       /* counts all occurrences within stream */
    unsigned pps;       /* counts all occurrences within stream */
    unsigned ssps;      /* counts all occurrences within stream */
    unsigned sh;        /* counts only those occurrences with matching SPS and PPS */
    unsigned islice;    /* counts only those occurrences within slice header */
    int last_sps;
    int last_pps;
    bmedia_probe_video info;
    bvideo_codec codec;
    union {
        bh265_video_sps sps;
    } data;
    struct {
        h265_video_pps pps;
    } state;
    uint8_t emulation_prention_buf[200];
};

void
b_hevc_video_probe_reset(bmedia_probe_base_es_t probe)
{
    bhevc_video_probe_t hevc_video = (bhevc_video_probe_t)probe;
    BDBG_OBJECT_ASSERT(hevc_video, bhevc_video_probe_t);
    batom_accum_clear(hevc_video->acc);
    hevc_video->sps = 0;
    hevc_video->ssps = 0;
    hevc_video->pps = 0;
    hevc_video->sh = 0;
    hevc_video->islice = 0;
    hevc_video->last_sps = -1;
    hevc_video->last_pps = -1;
    hevc_video->codec = bvideo_codec_h265;
    hevc_video->state.pps.valid = false;
    bmedia_probe_video_init(&hevc_video->info);
    ((bmedia_probe_h265_video *)&hevc_video->info.codec_specific)->sps.valid = false;
    return;
}

bmedia_probe_base_es_t
b_hevc_video_probe_create(batom_factory_t factory)
{
    bhevc_video_probe_t hevc_video;
    BDBG_ASSERT(factory);

    hevc_video = BKNI_Malloc(sizeof(*hevc_video));
    if(!hevc_video) {
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(hevc_video, bhevc_video_probe_t);
    hevc_video->acc = batom_accum_create(factory);
    if(!hevc_video->acc) {
        goto err_acc;
    }
    b_hevc_video_probe_reset((bmedia_probe_base_es_t)hevc_video);
    return (bmedia_probe_base_es_t)hevc_video;
err_acc:
    BKNI_Free(hevc_video);
err_alloc:
    return NULL;
}

void
b_hevc_video_probe_destroy(bmedia_probe_base_es_t probe)
{
    bhevc_video_probe_t hevc_video = (bhevc_video_probe_t)probe;
    BDBG_OBJECT_ASSERT(hevc_video, bhevc_video_probe_t);
    batom_accum_destroy(hevc_video->acc);
    BDBG_OBJECT_DESTROY(hevc_video, bhevc_video_probe_t);
    BKNI_Free(hevc_video);
    return;
}

/* static */ void b_hevc_video_set_frame_mbs_only(bmedia_probe_h265_video *info, bool frame_mbs_only)
{
    info->frame_mbs_only = frame_mbs_only;
    return;
}

struct b_hevc_bitstream {
    size_t len;
    batom_cursor cursor;
    batom_vec vec;
    batom_bitstream bs;
};

static void
b_hevc_prepare_bitstream(bhevc_video_probe_t hevc_video, const batom_cursor *data, struct b_hevc_bitstream *hevc_bs)
{
    BATOM_CLONE(&hevc_bs->cursor, data);
    hevc_bs->len = bmedia_strip_emulation_prevention(&hevc_bs->cursor, hevc_video->emulation_prention_buf, sizeof(hevc_video->emulation_prention_buf));
    BATOM_VEC_INIT(&hevc_bs->vec, hevc_video->emulation_prention_buf, hevc_bs->len);
    batom_cursor_from_vec(&hevc_bs->cursor, &hevc_bs->vec, 1);
    batom_bitstream_init(&hevc_bs->bs, &hevc_bs->cursor);
    return ;
}

static bool
b_hevc_video_parse_sps(bhevc_video_probe_t hevc_video, const batom_cursor *data, bmedia_probe_video *info, bool *eof)
{
    struct b_hevc_bitstream hevc_bs;

    *eof = false;
    b_hevc_prepare_bitstream(hevc_video, data, &hevc_bs);
    if(!bh265_video_sps_parse(&((bmedia_probe_h265_video *)&info->codec_specific)->sps, info, &hevc_bs.cursor)) {
        if(batom_bitstream_eof(&hevc_bs.bs)) {
            if(hevc_bs.len == sizeof(hevc_video->emulation_prention_buf)) {
                BDBG_WRN(("b_hevc_video_parse_sps:%p to large SPS record", (void *)hevc_video));
            } else {
                *eof = true;
            }
        }
        goto err_sps;
    }

    return true;

err_sps:
    return false;
}

static bool
b_hevc_video_parse_pps(bhevc_video_probe_t hevc_video, const batom_cursor *data, bool *eof)
{
    struct b_hevc_bitstream hevc_bs;

    *eof = false;
    b_hevc_prepare_bitstream(hevc_video, data, &hevc_bs);
    /*
     * 7.3.2.3 Picture parameter set RBSP syntax
     *
     * pic_parameter_set_rbsp( ) {
     */
    hevc_video->state.pps.pic_parameter_set_id = batom_bitstream_exp_golomb(&hevc_bs.bs);
    hevc_video->state.pps.seq_parameter_set_id = batom_bitstream_exp_golomb(&hevc_bs.bs);
    hevc_video->state.pps.dependent_slice_segments_enabled_flag = batom_bitstream_bit(&hevc_bs.bs);
    /* output_flag_present_flag */ batom_bitstream_drop_bits(&hevc_bs.bs,1);
    hevc_video->state.pps.num_extra_slice_header_bits = batom_bitstream_bits(&hevc_bs.bs,3);
    if(batom_bitstream_eof(&hevc_bs.bs)) {
        *eof = true;
        return false;
    }
    hevc_video->state.pps.valid = true;
    BDBG_MSG(("%s:%p found PPS, pps_id=%u, sps_id=%u", "b_hevc_video_probe_feed", (void *)hevc_video, hevc_video->state.pps.pic_parameter_set_id, hevc_video->state.pps.seq_parameter_set_id));
    return true;
}

static bool
b_hevc_video_parse_slice(bhevc_video_probe_t hevc_video, unsigned nal_unit_type, const batom_cursor *data, bool *eof)
{
    struct b_hevc_bitstream hevc_bs;
    unsigned i;
    unsigned first_slice_segment_in_pic_flag;
    unsigned slice_type;
    unsigned slice_pic_parameter_set_id;

    *eof = false;
    b_hevc_prepare_bitstream(hevc_video, data, &hevc_bs);

    if(!hevc_video->state.pps.valid) {
        return true;
    }
    /*
     * 7.3.2.9 Slice segment layer RBSP syntax
     *
     * slice_segment_layer_rbsp( ) { Descriptor
     * slice_segment_header( )
     *
     * 7.3.6.1 General slice segment header syntax
     * slice_segment_header( ) {
     */
    first_slice_segment_in_pic_flag = batom_bitstream_bit(&hevc_bs.bs);
    if(nal_unit_type>=16 /*BLA_W_LP*/ && nal_unit_type<=23 /* RSV_IRAP_VCL23 */) {
        batom_bitstream_drop_bits(&hevc_bs.bs,1); /* no_output_of_prior_pics_flag */
    }
    slice_pic_parameter_set_id = batom_bitstream_exp_golomb(&hevc_bs.bs);
    if(batom_bitstream_eof(&hevc_bs.bs)) {
        *eof = true;
        return false;
    }
    if(slice_pic_parameter_set_id != hevc_video->state.pps.pic_parameter_set_id) {
        BDBG_MSG(("%p:Not matching slice_pic_parameter_set_id:%u and pps.pic_parameter_set_id:%u", (void *)hevc_video, slice_pic_parameter_set_id, hevc_video->state.pps.pic_parameter_set_id));
        return true;
    }
    if(hevc_video->state.pps.dependent_slice_segments_enabled_flag) {
        return true;
    }
    if(batom_bitstream_eof(&hevc_bs.bs)) {
        *eof = true;
        return false;
    }
    if(!first_slice_segment_in_pic_flag) {
        return true; /* only looking at first_slice_segment_in_pic_flag */
    }
    for(i=0;i<hevc_video->state.pps.num_extra_slice_header_bits;i++) {
        batom_bitstream_drop_bits(&hevc_bs.bs, 1); /* slice_reserved_flag[i] */
    }
    slice_type = batom_bitstream_exp_golomb(&hevc_bs.bs);
    if(batom_bitstream_eof(&hevc_bs.bs)) {
        *eof = true;
        return false;
    }
    BDBG_MSG(("%s:%p found SH, slice_type=%u", "b_avc_video_probe_feed", (void *)hevc_video, slice_type));
    hevc_video->sh++;
    if (slice_type == 2) { /* I slice */
        hevc_video->islice++;
    }
    return true;
}

static bmedia_probe_track *
b_hevc_video_probe_make_track(bhevc_video_probe_t hevc_video)
{
    bmedia_probe_track *track;
    track = BKNI_Malloc(sizeof(*track));
    batom_accum_clear(hevc_video->acc);
    /* found all frames */
    if(track) {
        bmedia_probe_track_init(track);
        hevc_video->info.codec = hevc_video->codec;
        track->type = bmedia_track_type_video;
        track->info.video = hevc_video->info;
    }
    return track;
}

static uint32_t
b_hevc_video_probe_scan_nal_header(batom_cursor *cursor)
{
    return bmedia_video_scan_scode(cursor, 0xFFFFFFFFul);
}

static bmedia_probe_track *
b_hevc_video_probe_feed(bmedia_probe_base_es_t probe, batom_t atom, bool *done)
{
    bhevc_video_probe_t hevc_video = (bhevc_video_probe_t)probe;
    batom_accum_t acc;
    batom_cursor cursor;
    size_t pos = 0;
    bmedia_probe_video info;

    BDBG_OBJECT_ASSERT(hevc_video, bhevc_video_probe_t);

    bmedia_probe_video_init(&info);
    acc = hevc_video->acc;
    batom_accum_add_atom(acc, atom);
    batom_cursor_from_accum(&cursor, acc);

    for(;;) {
        uint32_t nal_header;
        bool valid;
        bool eof = false;
        bool forbidden_zero_bit;
        unsigned nal_unit_type;

        nal_header = b_hevc_video_probe_scan_nal_header(&cursor);
        /* H.265 NAL is 16 bits, and with NAL header is 40 bytes, so drop most significant 8 bits (00 in 00 00 01 .. ..) */
        nal_header = nal_header << 8;
        nal_header = nal_header | (batom_cursor_byte(&cursor));
        pos = batom_cursor_pos(&cursor);
        if(BATOM_IS_EOF(&cursor)) {
            goto eof;
        }
        forbidden_zero_bit = B_GET_BIT(nal_header, 15);
        nal_unit_type = B_GET_BITS(nal_header, 14, 9);

        BDBG_MSG_TRACE(("%s:%p nal_header %#x (forbidden_zero_bit:%u nal_unit_type:%u %u,%u) at %u", "b_hevc_video_probe_feed", (void *)hevc_video, nal_header, forbidden_zero_bit, nal_unit_type, B_GET_BITS(nal_header, 8,3), B_GET_BITS(nal_header, 2, 0), (unsigned)pos));

        if(forbidden_zero_bit) { /* forbidden_zero_bit */
            *done = true;
            goto flush;
        }
        switch(nal_unit_type) {
        case 33: /* SPS_NUT */

            valid = b_hevc_video_parse_sps(hevc_video, &cursor, &info, &eof);
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
                bh265_video_sps_to_string(&hevc_video->data.sps,  str, sizeof(str));
                BDBG_MSG(("%s:%p found SPS:%s, sps_id=%u", "b_hevc_video_probe_feed", (void *)hevc_video, str, hevc_video->data.sps.sps_video_parameter_set_id));
            }
#endif

            hevc_video->sps++;
            if (hevc_video->last_sps != hevc_video->data.sps.sps_video_parameter_set_id ) { /* reset */
                hevc_video->last_sps = hevc_video->data.sps.sps_video_parameter_set_id;
                hevc_video->last_pps = -1;
                hevc_video->sh = 0;
                hevc_video->islice = 0;
                hevc_video->info = info;
            } else if (hevc_video->last_sps == hevc_video->data.sps.sps_video_parameter_set_id) {
                if (info.width != hevc_video->info.width || info.height != hevc_video->info.height) { /* reset */
                    hevc_video->last_sps = -1;
                    hevc_video->last_pps = -1;
                    hevc_video->sh = 0;
                    hevc_video->islice = 0;
                }
            }
            break;
        case 34: /* PPS_NUT */
            valid = b_hevc_video_parse_pps(hevc_video, &cursor, &eof);
            if(eof) {
                goto eof;
            }
            if(!valid) {
                *done = true;
                goto flush;
            }
            hevc_video->pps++;
            if (hevc_video->last_sps==hevc_video->state.pps.seq_parameter_set_id && !hevc_video->sh){
                hevc_video->last_pps = hevc_video->state.pps.seq_parameter_set_id;
            }
            break;
        /* 3.15 broken link access (BLA) picture */
        /* 3.23 clean random access (CRA) picture */
        /* 3.59 instantaneous decoding refresh (IDR) picture */
        /* 3.109 random access decodable leading (RADL) picture */
        /* 3.111 random access skipped leading (RASL) picture */
        /* 3.138 step-wise temporal sub-layer access (STSA) picture */
        /* 3.151 temporal sub-layer access (TSA) picture */
        case 0: case 1: /* Coded slice segment of a non-TSA, non-STSA trailing picture */
        case 2: case 3: /* Coded slice segment of a TSA picture */
        case 4: case 5: /* Coded slice segment of an STSA picture */
        case 6: case 7: /* Coded slice segment of a RADL picture */
        case 8: case 9: /* Coded slice segment of a RASL picture */
        /* case 10: case 12: case 14: */ /* Reserved non-IRAP sub-layer non-reference VCL NAL unit types */
        case 16: case 17: case 18: /* Coded slice segment of a BLA picture */
        case 19: case 20: /* Coded slice segment of an IDR picture */
        case 21:  /* Coded slice segment of a CRA picture */
            valid = b_hevc_video_parse_slice(hevc_video, nal_unit_type, &cursor, &eof);
            if(eof) {
                goto eof;
            }

            if(!valid) {
                *done = true;
                goto flush;
            }
            if ((hevc_video->sps >= 2) && hevc_video->pps && hevc_video->islice) {
                BDBG_MSG(("%s:%p SPS:%u PPS:%u, SH:%u, I-Slice:%u", "b_hevc_video_probe_feed", (void *)hevc_video, hevc_video->sps, hevc_video->pps, hevc_video->sh, hevc_video->islice));
                return b_hevc_video_probe_make_track(hevc_video);
            }

            break;

        default:
            break;
        }
    }
eof:
    BDBG_MSG_TRACE(("%s:%p trim %u", "b_hevc_video_probe_feed", (void *)hevc_video, (unsigned)pos));
    if(pos>B_MEDIA_VIDEO_SCODE_LEN+1) {
        batom_cursor_from_accum(&cursor, acc);
        batom_cursor_skip(&cursor, pos-(B_MEDIA_VIDEO_SCODE_LEN+1));
        batom_accum_trim(acc, &cursor);
    }
done:
    return NULL;
flush:
    batom_accum_clear(acc);
    goto done;
}

static bmedia_probe_track *
b_hevc_video_probe_last(bmedia_probe_base_es_t probe, unsigned *probability)
{
    bhevc_video_probe_t hevc_video = (bhevc_video_probe_t)probe;

    BDBG_OBJECT_ASSERT(hevc_video, bhevc_video_probe_t);
    BDBG_MSG(("b_hevc_video_probe_last:%#lx sps:%u:%d pps:%u:%d ssps:%u islice:%u %s", (unsigned long)hevc_video, hevc_video->sps, hevc_video->last_sps, hevc_video->pps, hevc_video->last_pps, hevc_video->ssps, hevc_video->islice, hevc_video->codec!=bvideo_codec_h265?"extension":""));
    if (hevc_video->sps && hevc_video->pps) {
        char str[64];
        const bmedia_probe_h265_video *h265_video = (bmedia_probe_h265_video *)&hevc_video->info.codec_specific;
        bh265_video_sps_to_string(&h265_video->sps,  str, sizeof(str));
        BDBG_MSG(("b_hevc_video_probe_last: SPS:%s", str));
        if (hevc_video->islice) {
            *probability = 90;
        }
        else {
            *probability = 50;
        }
        return b_hevc_video_probe_make_track(hevc_video);
    } else if(hevc_video->ssps && hevc_video->codec!=bvideo_codec_h265) {
       *probability = 50;
        return b_hevc_video_probe_make_track(hevc_video);
    }
    else {
        *probability = 0;
        return NULL;
    }
}

static const bmedia_probe_file_ext b_hevc_video_ext[] =  {
    {"es"}, {"ves"}, {"mpg"}, {"hevc"}, {"26l"}, {"265"},
    {""}
};

const bmedia_probe_es_desc bhevc_video_probe = {
    bmedia_track_type_video,
    {
        bvideo_codec_h265
    },
    b_hevc_video_ext,
    ((20*(1000*1000))/8)*2, /* parse 2 seconds of 20MBps stream */
    b_hevc_video_probe_create,
    b_hevc_video_probe_destroy,
    b_hevc_video_probe_feed,
    b_hevc_video_probe_reset,
    b_hevc_video_probe_last
};
