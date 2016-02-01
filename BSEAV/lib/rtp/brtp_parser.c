/******************************************************************************
 * (c) 2006-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "brtp.h"
#include "brtp_parser.h"
#include "brtp_packet.h"

BDBG_MODULE(brtp_parser);

BDBG_OBJECT_ID(brtp_parser_t);

BDBG_OBJECT_ID(brtp_parser_mux_t);

struct brtp_parser_mux {
    brtp_io_block io_pool;
    brtp_parser_mux_cfg cfg;
    BDBG_OBJECT(brtp_parser_mux_t)
    brtp_io_vec *io_vecs;
};

static void
b_rtp_null_data_sink(void *data_sink_cntx, brtp_io_block *block, brtp_parser_t parser, uint32_t pts)
{
    BSTD_UNUSED(data_sink_cntx);
    BSTD_UNUSED(block);
    BSTD_UNUSED(parser);
    BSTD_UNUSED(pts);
    return;
}

brtp_parser_mux_cfg b_rtp_parser_mux_default_cfg = {
    brtp_mux_mode_pes, /* mode */
    NULL, /* data_sink_cntt */
    b_rtp_null_data_sink, /* data_sink */
    1024, /* n_iovec */
    64,   /* n_ioblock */
    64*32, /* meta_len */
    NULL    /* meta */
};

void
brtp_parser_mux_default_cfg(brtp_parser_mux_cfg *cfg)
{
    *cfg = b_rtp_parser_mux_default_cfg;
    return;
}

brtp_parser_mux_t
brtp_parser_mux_create(const brtp_parser_mux_cfg *cfg)
{
    brtp_parser_mux_t  mux;
    unsigned i;

    BDBG_ASSERT(cfg);
    BDBG_ASSERT(cfg->meta);
    BDBG_ASSERT(cfg->meta_len>0);
    BDBG_ASSERT(cfg->n_iovec>0);
    BDBG_ASSERT(cfg->n_ioblock>0);

    mux = BKNI_Malloc(sizeof(*mux));
    if (!mux) {
        BDBG_ERR(("brtp_parser_mux_create: out of memory"));
        goto err_alloc_mux;
    }
    BDBG_OBJECT_INIT(mux, brtp_parser_mux_t);
    mux->cfg = *cfg;
    mux->io_vecs = BKNI_Malloc(sizeof(*mux->io_vecs)*cfg->n_iovec);
    if (!mux->io_vecs) {
        BDBG_ERR(("brtp_parser_mux_create: out of memory"));
        goto err_alloc_io_blocks;
    }
    BLST_SQ_INIT(&mux->io_pool);
    for(i=0;i<cfg->n_iovec;i++) {
        BLST_SQ_INSERT_TAIL(&mux->io_pool, mux->io_vecs+i, list);
    }
    return mux;

err_alloc_io_blocks:
    BKNI_Free(mux);
err_alloc_mux:
    return NULL;
}

void
brtp_parser_mux_reset(brtp_parser_mux_t mux)
{
    BSTD_UNUSED(mux);

    BDBG_OBJECT_ASSERT(mux, brtp_parser_mux_t);
    return;
}

void
brtp_parser_mux_destroy(brtp_parser_mux_t mux)
{
    BDBG_OBJECT_ASSERT(mux, brtp_parser_mux_t);
    BKNI_Free(mux->io_vecs);
    BDBG_OBJECT_DESTROY(mux, brtp_parser_mux_t);
    BKNI_Free(mux);
    return;
}

void
brtp_parser_mux_recycle(brtp_parser_mux_t mux, brtp_io_block *io_block)
{
    brtp_io_vec *vec;
    brtp_io_vec *vec_next;

    BDBG_OBJECT_ASSERT(mux, brtp_parser_mux_t);

    for(vec = BLST_SQ_FIRST(io_block);vec;vec = vec_next) {
        if(vec->packet) {
            BDBG_ASSERT(vec->packet->ref_cnt>0);
            vec->packet->ref_cnt--;
        }
        vec_next = BLST_SQ_NEXT(vec,list);
        BLST_SQ_INSERT_HEAD(&mux->io_pool, vec, list);
    }
    BLST_SQ_INIT(io_block);
    return;
}

void
brtp_parser_mux_stream_default_cfg(brtp_parser_mux_stream_cfg *cfg)
{
    cfg->pes_id = 0;
    cfg->clock_rate = 90000;
    cfg->clock_offset = 0;
    cfg->sink_stream_cntx = NULL;
    return;
}

void
brtp_parser_mux_attach(brtp_parser_mux_t mux, brtp_parser_t parser,  const brtp_parser_mux_stream_cfg *cfg)
{
    brtp_parser_mux_stream_t stream = &parser->stream;

    BDBG_OBJECT_ASSERT(parser, brtp_parser_t);
    BDBG_OBJECT_ASSERT(mux, brtp_parser_mux_t);
    stream->mux = mux;
    stream->cfg = *cfg;
    stream->last_timestamp = 0;
    stream->header_len = 0;
    stream->header = NULL;
    BLST_SQ_INIT(&stream->acc);
    btimestamp_init(&stream->timestamp);
    return;
}


brtp_io_vec *
brtp_parser_next(brtp_parser_t parser)
{
    brtp_parser_mux_t mux = parser->stream.mux;
    brtp_parser_mux_stream_t stream = &parser->stream;
    brtp_io_vec *vec;
    BDBG_OBJECT_ASSERT(parser, brtp_parser_t);
    BDBG_OBJECT_ASSERT(mux, brtp_parser_mux_t);

    vec = BLST_SQ_FIRST(&mux->io_pool);

    if (vec) {
        BLST_SQ_REMOVE_HEAD(&mux->io_pool, list);
        BLST_SQ_INSERT_TAIL(&stream->acc, vec, list);
    }
    return vec;
}

bool
brtp_parser_begin(brtp_parser_t parser)
{
    brtp_parser_mux_stream_t stream = &parser->stream;
    brtp_parser_mux_t mux = parser->stream.mux;

    BDBG_OBJECT_ASSERT(parser, brtp_parser_t);
    BDBG_OBJECT_ASSERT(mux, brtp_parser_mux_t);

    BLST_SQ_INIT(&stream->acc);
    if (mux->cfg.mode == brtp_mux_mode_pes || parser->stream.header_len>0) {
        brtp_io_vec *vec = brtp_parser_next(parser); /* reserve vector for the PES header */
        if (!vec) {
            return true;
        }
        vec->len = 0;
        vec->data = NULL;
        vec->packet = NULL;
    }
    return false;
}


static size_t
b_rtp_pes_header_init(uint8_t *pes_header, uint8_t stream_id, uint32_t pts, bool pts_valid, size_t length)
{
    unsigned off;
    const bool frame_boundary=false;
    const bool extension=false;

    BDBG_MSG(("b_rtp_pes_header_init>: %#lx %#x %u %u, %08x", (unsigned long)pes_header, stream_id, length, pts*2, pts));

    pes_header[0] = 0;
    pes_header[1] = 0;
    pes_header[2] = 1;
    pes_header[3] = stream_id;

    pes_header[6] =
        B_SET_BITS( "10", 2, 7, 6 ) |
        B_SET_BITS(PES_scrambling_control, 0, 5, 4) |
        B_SET_BIT(PES_priority, 0, 3) |
        B_SET_BIT(data_alignment_indicator, frame_boundary, 2) |
        B_SET_BIT(copyright, 0, 1) |
        B_SET_BIT(original_or_copy, 1, 0);
    pes_header[7] =
        B_SET_BITS(PTS_DTS_flags, pts_valid?2:0, 7, 6) |
        B_SET_BIT(ESCR_flag, 0, 5) |
        B_SET_BIT(ES_rate_flag, 0, 4) |
        B_SET_BIT(DSM_trick_mode_flag, 0, 3) |
        B_SET_BIT(additional_copy_info_flag, 0, 2) |
        B_SET_BIT(PES_CRC_flag, 0, 1) |
        B_SET_BIT(PES_extension_flag, extension, 0);
    if (pts_valid) {
        pes_header[9] =
            B_SET_BITS("0010", 0x2, 7, 4) |
            B_SET_BITS("PTS [32..30]", B_GET_BITS(pts,31,29), 3, 1) |
            B_SET_BIT(marker_bit, 1, 0);
        pes_header[10] = B_GET_BITS(pts,28,21); /* PTS [29..15] -> PTS [29..22] */
        pes_header[11] =
            B_SET_BITS("PTS [29..15] -> PTS [21..15]", B_GET_BITS(pts,20,14), 7, 1) |
            B_SET_BIT(marker_bit, 1, 0);
        pes_header[12] = B_GET_BITS(pts,13,6); /* PTS [14..0] -> PTS [14..7]  */
        pes_header[13] =
                        B_SET_BITS("PTS [14..0] -> PTS [6..0]", B_GET_BITS(pts,5,0), 7, 2) |
                        B_SET_BIT("PTS[0]", 0, 1) |
                        B_SET_BIT(marker_bit, 1, 0);
        off = 14;
    } else {
        off = 9;
    }

    pes_header[8] = off - 9;
    /* length */
    if (length==0) {
        pes_header[4] = 0;
        pes_header[5] = 0;
    } else {
        length += (off-6);
        if (length>65536) {
            BDBG_WRN(("unbounded pes %02x %u", stream_id, length));
            length = 0;
        }
        pes_header[4] = B_GET_BITS(length, 15, 8);
        pes_header[5] = B_GET_BITS(length, 7, 0);
    }

    BDBG_MSG(("b_rtp_pes_header_init<: %#lx %#x %u %u", (unsigned long)pes_header, stream_id, length, off));

    return off;
}

void
brtp_parser_commit(brtp_parser_t parser)
{
    brtp_parser_mux_t mux = parser->stream.mux;
    brtp_parser_mux_stream_t stream = &parser->stream;
    uint32_t pts=0;

    BDBG_OBJECT_ASSERT(parser, brtp_parser_t);
    BDBG_OBJECT_ASSERT(mux, brtp_parser_mux_t);

    if (mux->cfg.mode == brtp_mux_mode_pes || parser->stream.header_len>0) {
        size_t pkt_len;
        brtp_io_vec *pes_hdr;
        const brtp_io_vec *vec;
        uint32_t timestamp=0;
        bool has_timestamp=false;
        uint8_t *header;

        BDBG_ASSERT(BLST_SQ_FIRST(&stream->acc));
        pes_hdr = BLST_SQ_FIRST(&stream->acc);
        BDBG_ASSERT(pes_hdr->len==0 && pes_hdr->data==NULL && pes_hdr->packet==NULL);
        header = mux->cfg.meta;

        if (mux->cfg.mode == brtp_mux_mode_pes) {
            for(pkt_len=parser->stream.header_len, vec = BLST_SQ_NEXT(pes_hdr,list);vec;vec = BLST_SQ_NEXT(vec,list)) {
                    if (vec->packet) {
                        timestamp = B_RTP_PKT_TIMESTAMP(vec->packet);
                        has_timestamp = true;
                    }
                    pkt_len += vec->len;
            }
            if (has_timestamp) {
                if (timestamp == stream->last_timestamp) {
                    has_timestamp = false;
                } else {
                    BDBG_MSG(("brtp_parser_commit: %#lx source:%#lx(0x%02X) timestamp %lu(%ld)", (unsigned long)mux, (unsigned long)parser, stream->cfg.pes_id, (unsigned long)timestamp,((long)timestamp - (long)stream->last_timestamp)));
                    stream->last_timestamp = timestamp; /* save last timestamp */
                    btimestamp_add(&stream->timestamp, timestamp-parser->stream.cfg.clock_offset);
                    /* convert timestamp to 45KHz */
                    pts = (btimestamp_get(&stream->timestamp)*45000)/(parser->stream.cfg.clock_rate);
                    BDBG_MSG(("brtp_parser_commit: %#lx source:%#lx(0x%02X) pts %lu(%lu)", (unsigned long)mux, (unsigned long)parser, stream->cfg.pes_id, (unsigned long)pts, (unsigned long)timestamp));
                }
            }
            pes_hdr->len = b_rtp_pes_header_init(header, stream->cfg.pes_id, pts, has_timestamp, pkt_len);
            BDBG_MSG(("pkt_len %u pes_len %u PTS: %u", pkt_len, pes_hdr->len, pts));
        }
        if (parser->stream.header_len) {
            BDBG_ASSERT(parser->stream.header);
            BKNI_Memcpy(header+pes_hdr->len, parser->stream.header, parser->stream.header_len);
            pes_hdr->len += parser->stream.header_len;
        }
        pes_hdr->data = header;
    }
    mux->cfg.data_sink(mux->cfg.data_sink_cntx, &stream->acc, parser, pts);
    return;
}

void
brtp_parser_abort(brtp_parser_t parser)
{
    brtp_parser_mux_t mux = parser->stream.mux;
    brtp_parser_mux_stream_t stream = &parser->stream;
    brtp_io_vec *vec;
    brtp_io_vec *vec_next;

    BDBG_OBJECT_ASSERT(parser, brtp_parser_t);
    BDBG_OBJECT_ASSERT(mux, brtp_parser_mux_t);

    for(vec = BLST_SQ_FIRST(&stream->acc); vec; vec=vec_next ) {
        if(vec->packet) {
            BDBG_MSG(("b_rtp_parser_mux_abort: %#lx recycling  %#lx(%u)", (unsigned long)mux, (unsigned long)vec->packet, vec->packet->ref_cnt));
            //BDBG_ASSERT(vec->packet->ref_cnt>0);
            // We got assert failure after *** 00:20:43.103 brtp_parser_h264: b_rtp_parser_h264_discontinuity: 0x50aa98 dropping accumulated data
            // Disable it for now.
            if ( vec->packet->ref_cnt>0)
                vec->packet->ref_cnt--;
            else {
                BDBG_WRN(("%s: ref_cnt (%d) <= 0!!! Set to 0", vec->packet->ref_cnt));
                vec->packet->ref_cnt = 0;
            }
        }
        vec_next = BLST_SQ_NEXT(vec,list);
        BLST_SQ_INSERT_HEAD(&mux->io_pool, vec, list);
    }
    BLST_SQ_INIT(&stream->acc);
    return;
}


static void
b_rtp_parser_packet(brtp_parser_t parser, brtp_packet_t pkt, const void *data, size_t len)
{
    BSTD_UNUSED(parser);
    BSTD_UNUSED(pkt);
    BSTD_UNUSED(data);
    BSTD_UNUSED(len);
    BDBG_ASSERT(0); /* this function shall never be called */
    return;
}

static void
b_rtp_parser_discontinuity(brtp_parser_t parser)
{
    BSTD_UNUSED(parser);
    /* do nothing */
    return;
}

static brtp_parser_mux_stream_t
b_rtp_parser_start(brtp_parser_t parser, brtp_parser_mux_t mux, const brtp_parser_mux_stream_cfg *cfg, const void *parser_cfg, size_t parser_cfg_len)
{
    BSTD_UNUSED(parser);
    BSTD_UNUSED(mux);
    BSTD_UNUSED(cfg);
    BSTD_UNUSED(parser_cfg);
    BSTD_UNUSED(parser_cfg_len);

    BDBG_ASSERT(0); /* this function shall never be called */
    return NULL;
}

static void
b_rtp_parser_stop(brtp_parser_t parser)
{
    brtp_parser_stop(parser);
    return;
}

static void
b_rtp_parser_destroy(brtp_parser_t parser)
{
    BSTD_UNUSED(parser);
    BDBG_ASSERT(0); /* this function shall never be called */

    return;
}

static const brtp_parser_type brtp_parser_invalid = {
    "invalid",
    brtp_stream_invalid
};

void
brtp_parser_init(brtp_parser_t parser)
{
    BKNI_Memset(parser, 0, sizeof(*parser));
    BDBG_OBJECT_SET(parser, brtp_parser_t);
    parser->stream.mux = NULL;
    parser->packet = b_rtp_parser_packet;
    parser->discontinuity = b_rtp_parser_discontinuity;
    parser->start =  b_rtp_parser_start;
    parser->stop =  b_rtp_parser_stop;
    parser->destroy =  b_rtp_parser_destroy;
    parser->type =  &brtp_parser_invalid;
    return;
}

void
brtp_parser_stop(brtp_parser_t parser)
{
    BDBG_OBJECT_ASSERT(parser, brtp_parser_t);
    if (parser->stream.mux) {
        brtp_parser_abort(parser);
        parser->stream.mux = NULL;
    }
}

bool
brtp_parser_add_packet(brtp_parser_t parser, brtp_packet_t packet, const void *data, size_t len)
{
    brtp_io_vec *vec = brtp_parser_next(parser);
    BDBG_OBJECT_ASSERT(parser, brtp_parser_t);
    //BDBG_ASSERT( (uint8_t*)data >= (uint8_t *)packet->data && len <= packet->len); /* check that data belongs to the packet */
    //BDBG_ASSERT( (uint8_t*)data+len <= (uint8_t*)packet->data+packet->len); /* check that data is in bounds */
    //BDBG_ASSERT(len <= packet->len); /* check that data is in bounds */
    if (!vec) {return true;}
    vec->packet = packet;
    vec->data = data;
    vec->len = len;
    packet->ref_cnt++;
    return false;
}
