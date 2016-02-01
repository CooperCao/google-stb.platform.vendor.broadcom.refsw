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
#include "brtp_parser_mpeg4.h"
#include "brtp_packet.h"
#include "biobits.h"
#include "brtp_util.h"

BDBG_MODULE(brtp_parser_mpeg4);

BDBG_OBJECT_ID(brtp_parser_mpeg4_t);

struct brtp_parser_mpeg4 {
    struct brtp_parser parent; /* must be the first element */
    BDBG_OBJECT(brtp_parser_mpeg4_t)
    brtp_parser_mpeg4_stream_cfg stream_cfg;
    unsigned pkt_cnt;
    unsigned pkt_len;
    brtp_parser_mpeg4_cfg cfg;
    union {
        struct {
            uint8_t adts_header[7];
            uint8_t adts_header_3;
        } aac;
        struct {
            uint8_t wav_header[8/*BCMA header*/+18/*WAVEFORMATEX*/];
        } g711;
    } codec;
};

typedef struct brtp_parser_mpeg4 *brtp_parser_mpeg4_t;

static const brtp_parser_type b_rtp_parser_mpeg4= {
    "MPEG4/AAC",
    brtp_stream_audio
};

const brtp_parser_type *brtp_parser_mpeg4 = &b_rtp_parser_mpeg4;

static const brtp_parser_type b_rtp_parser_g711= {
    "MPEG4/G711",
    brtp_stream_audio
};
const brtp_parser_type *brtp_parser_g711 = &b_rtp_parser_g711;

void
brtp_parser_mpeg4_default_cfg(brtp_parser_mpeg4_cfg *cfg)
{
    *cfg = 0;
    return;
}

static const brtp_parser_mpeg4_stream_cfg b_rtp_parser_mpeg4_aac_hbr_cfg = {
    brtp_parser_mpeg4_aac_hbr, /* mode */
    13, /* sizelength */
    3, /* indexlength */
    2, /* headerlength */
    {0}, /* config */
    0, /* config_len */
    15 /* profile */
};

static const brtp_parser_mpeg4_stream_cfg b_rtp_parser_mpeg4_g711_cfg = {
    brtp_parser_mpeg4_g711, /* mode */
    13, /* sizelength */
    0, /* indexlength */
    0, /* headerlength */
    {0}, /* config */
    0, /* config_len */
    15 /* profile */
};

void
brtp_parser_mpeg4_default_stream_cfg(brtp_parser_mpeg4_mode mode, brtp_parser_mpeg4_stream_cfg *cfg)
{
    BDBG_ASSERT(cfg);
    switch(mode) {
    default:
        BDBG_WRN(("brtp_parser_mpeg4_default_stream_cfg: not supported mode %u", mode));
        cfg->mode = mode;
        break;
    case brtp_parser_mpeg4_aac_hbr:
        *cfg = b_rtp_parser_mpeg4_aac_hbr_cfg;
        break;
    case brtp_parser_mpeg4_g711:
        *cfg = b_rtp_parser_mpeg4_g711_cfg;
        break;
    }
    return;
}

static void
b_rtp_parser_mpeg4_begin(brtp_parser_mpeg4_t parser)
{
    brtp_parser_begin(&parser->parent);
    parser->pkt_cnt = 0;
    parser->pkt_len = 0;
    return;
}

static void
b_rtp_parser_mpeg4_commit(brtp_parser_mpeg4_t parser)
{
    unsigned len;

    switch(parser->stream_cfg.mode) {
    case brtp_parser_mpeg4_aac_hbr:
        len = parser->pkt_len + sizeof(parser->codec.aac.adts_header);
        /* IS 13818-7 1.2.2 Variable Header of ADTS */
        parser->codec.aac.adts_header[3] = parser->codec.aac.adts_header_3 |
            B_SET_BITS(aac_frame_length[12..11], B_GET_BITS(len, 12, 11), 1, 0);
        parser->codec.aac.adts_header[4] =
            B_SET_BITS(aac_frame_length[10..3], B_GET_BITS(len, 10, 3), 7, 0);
        parser->codec.aac.adts_header[5] =
            B_SET_BITS(aac_frame_length[2..0], B_GET_BITS(len, 2, 0), 7, 5) |
            B_SET_BITS(adts_buffer_fullness[10..6], B_GET_BITS( 0x1FF /* VBR */, 10, 6), 4, 0);
        parser->codec.aac.adts_header[6] =
                B_SET_BITS(adts_buffer_fullness[5..0], B_GET_BITS( 0x1FF /* VBR */, 5, 0), 7, 2) |
                B_SET_BITS(no_raw_data_blocks_in_frame, parser->pkt_cnt - 1, 2, 0);
        break;
    case brtp_parser_mpeg4_g711:
        len = parser->pkt_len;
        parser->codec.g711.wav_header[4] = (len >> 24) & 0xff;
        parser->codec.g711.wav_header[5] = (len >> 16) & 0xff;
        parser->codec.g711.wav_header[6] = (len >> 8) & 0xff;
        parser->codec.g711.wav_header[7] = len & 0xff;
        break;
    }
    brtp_parser_commit(&parser->parent);
    return;
}

static void
b_rtp_parser_mpeg4_packet(brtp_parser_t parser_, brtp_packet_t pkt, const void *data, size_t len)
{
    brtp_parser_mpeg4_t parser = (brtp_parser_mpeg4_t)parser_;
    bio_bitstream bs;
    bio_cursor cursor;
    bio_array array;
    unsigned size, index;
    unsigned header_len;
    unsigned off=0;
    bool overflow=false;

    BSTD_UNUSED(len);

    BDBG_OBJECT_ASSERT(parser, brtp_parser_mpeg4_t);
    BDBG_ASSERT(parser->parent.stream.mux);
    BDBG_MSG(("b_rtp_parser_mpeg4_packet: %#lx %lx:%u %s %u", (unsigned long)parser, (unsigned long)pkt, len, B_RTP_PKT_MARKER(pkt)?"M":"", B_RTP_PKT_TIMESTAMP(pkt)));
    if (parser->stream_cfg.headerlength) {
        /* read AU header */
        header_len = B_RTP_LOAD16(data,0);
        /* skip the header */
        off = 2 + (header_len+7)/8; /* number of bytes */
        bio_cursor_from_range(&cursor, &array, (uint8_t *)data+2, len-2);
        bio_bitstream_init(&bs, &cursor);
        while(header_len>0) {
            size = bio_bitstream_bits(&bs, parser->stream_cfg.sizelength);
            index = bio_bitstream_bits(&bs, parser->stream_cfg.indexlength);
            BDBG_MSG(("b_rtp_parser_mpeg4_packet: %#lx header_len %u size %u index %u", (unsigned long)parser, header_len, size, index));
            header_len -= parser->stream_cfg.indexlength + parser->stream_cfg.sizelength;
            overflow = brtp_parser_add_packet(&parser->parent, pkt, (uint8_t *)data+off, size);
            if (overflow) {
                brtp_parser_abort(&parser->parent);
                break;
            }
            off += size;
            parser->pkt_len += size;
            parser->pkt_cnt++;
            if (B_RTP_PKT_MARKER(pkt)) {
                b_rtp_parser_mpeg4_commit(parser);
                b_rtp_parser_mpeg4_begin(parser);
            }
        }
    } else {
        /* just add data as is */
        overflow = brtp_parser_add_packet(&parser->parent, pkt, data, len);
        if (overflow) {
            brtp_parser_abort(&parser->parent);
        } else {
            parser->pkt_len += len;
            parser->pkt_cnt++;
            b_rtp_parser_mpeg4_commit(parser);
            b_rtp_parser_mpeg4_begin(parser);
        }
    }
    BDBG_ASSERT(len > 2);

    return;
}

static void
b_rtp_parser_mpeg4_discontinuity(brtp_parser_t parser_)
{
    brtp_parser_mpeg4_t parser = (brtp_parser_mpeg4_t)parser_;
    BDBG_OBJECT_ASSERT(parser, brtp_parser_mpeg4_t);
    BDBG_ASSERT(parser->parent.stream.mux);
    brtp_parser_abort(&parser->parent);
    b_rtp_parser_mpeg4_begin(parser);
    return;
}



static brtp_parser_mux_stream_t
b_rtp_parser_mpeg4_start(brtp_parser_t parser_, brtp_parser_mux_t mux, const brtp_parser_mux_stream_cfg *cfg, const void *mpeg4_cfg_, size_t mpeg4_cfg_len)
{
    brtp_parser_mpeg4_t parser = (brtp_parser_mpeg4_t)parser_;
    bio_bitstream bs;
    bio_cursor cursor;
    bio_array array;
    unsigned stream_type;
    unsigned sampling_frequency_index;
    unsigned channel_configuration;
    unsigned profile;
    const brtp_parser_mpeg4_stream_cfg *mpeg4_cfg = mpeg4_cfg_;
    uint8_t *waveheader;

    BDBG_OBJECT_ASSERT(parser, brtp_parser_mpeg4_t);
    BDBG_ASSERT(parser->parent.stream.mux==NULL);
    BDBG_ASSERT(cfg);
    BDBG_ASSERT(mpeg4_cfg);
    BDBG_ASSERT(sizeof(*mpeg4_cfg)==mpeg4_cfg_len);

    parser->stream_cfg = *mpeg4_cfg;
    brtp_parser_mux_attach(mux, parser_, cfg);

    switch(mpeg4_cfg->mode) {
    case brtp_parser_mpeg4_aac_hbr:
        bio_cursor_from_range(&cursor, &array, mpeg4_cfg->config, mpeg4_cfg->config_len);
        bio_bitstream_init(&bs, &cursor);
        /*  ISO/IEC 14496-3: 6.2.1 */
        stream_type = bio_bitstream_bits(&bs, 5);
        sampling_frequency_index = bio_bitstream_bits(&bs, 4);
        if (sampling_frequency_index==0xF) {
            BDBG_WRN(("brtp_parser_mpeg4_start: %#lx unsupported sampling_frequency_index  %u", (unsigned long)parser, sampling_frequency_index));
            bio_bitstream_drop_bits(&bs, 24);
        }
        channel_configuration = bio_bitstream_bits(&bs, 4);
        switch(mpeg4_cfg->profile) {
        case 0x15:
        case 15:
        case 41:
        case 0x41:
            profile = 1; break; /* AAC low complexity profile */
        default:
            BDBG_WRN(("brtp_parser_mpeg4_start: %#lx unknown profile %u", (unsigned long)parser, mpeg4_cfg->profile));
            profile = 0; break;
        }
        /* IS 13818-7  1.2.1    Fixed Header of ADTS  */
        parser->codec.aac.adts_header[0] =
            0xFF; /* sync word */
        parser->codec.aac.adts_header[1] =
            0xF0 |  /* sync word */
            B_SET_BIT( ID, 1, 3) | /* MPEG-2 AAC */
            B_SET_BITS( "00", 0, 2, 1) |
            B_SET_BIT( protection_absent, 1, 0);
        parser->codec.aac.adts_header[2] =
            B_SET_BITS( profile, profile /* low-complexity profile */ , 7, 6) |
            B_SET_BITS( sampling_frequency_index, sampling_frequency_index, 5, 2 ) |
            B_SET_BIT( private_bit, 0, 1) |
            B_SET_BIT( channel_configuration[2], B_GET_BIT(channel_configuration, 2), 0);

        parser->codec.aac.adts_header_3 = /* 4'th byte is shared */
            B_SET_BITS( channel_configuration[2], B_GET_BITS(channel_configuration, 1, 0), 7, 6) |
            B_SET_BIT( original_copy, 0, 5) |
            B_SET_BIT( home, 0, 4) |
        /* IS 13818-7 1.2.2 Variable Header of ADTS */
            B_SET_BIT( copyright_identification_bit, 0, 3) |
            B_SET_BIT( copyright_identification_start, 0, 2);

        /* bytes 5,6 and 7 depend on the frame length */

        parser->parent.stream.header_len = sizeof(parser->codec.aac.adts_header);
        parser->parent.stream.header = parser->codec.aac.adts_header;
        break;

    case brtp_parser_mpeg4_g711:
        BDBG_WRN(("%s: brtp_parser_mpeg4_g711", __func__));
        waveheader = parser->codec.g711.wav_header;
        waveheader[0] = 0x42;
        waveheader[1] = 0x43;
        waveheader[2] = 0x4d;
        waveheader[3] = 0x41;
        waveheader[4] = 0x00;
        waveheader[5] = 0x00;
        waveheader[6] = 0x01;
        waveheader[7] = 0x40;

        B_RTP_SAVE_UINT16_LE(waveheader+8, 1);
        B_RTP_SAVE_UINT16_LE(waveheader+10, 1);
        B_RTP_SAVE_UINT32_LE(waveheader+12, 8000);
        B_RTP_SAVE_UINT32_LE(waveheader+16, 16000);
        B_RTP_SAVE_UINT16_LE(waveheader+20, 1);
        B_RTP_SAVE_UINT16_LE(waveheader+22, 16);
        B_RTP_SAVE_UINT16_LE(waveheader+24, 0);

        parser->parent.stream.header_len = sizeof(parser->codec.g711.wav_header);
        parser->parent.stream.header = (void *) &(parser->codec.g711.wav_header);
        // bFeedRtpPayloadToPlaypump of b_playback_ip_rtsp_es.c
        // uses parser type to decide whether to do PCMU to PCM conversion
        parser->parent.type = brtp_parser_g711;
        break;

    default:
        BDBG_WRN(("brtp_parser_mpeg4_start: %#lx not supported mode %u", (unsigned long)parser, mpeg4_cfg->mode));
        break;
    }
    b_rtp_parser_mpeg4_begin(parser);
    return  &parser->parent.stream;
}

void
b_rtp_parser_mpeg4_stop(brtp_parser_t parser_)
{
    brtp_parser_mpeg4_t parser = (brtp_parser_mpeg4_t)parser_;
    BDBG_OBJECT_ASSERT(parser, brtp_parser_mpeg4_t);
    brtp_parser_stop(&parser->parent);
    return;
}

static void
b_rtp_parser_mpeg4_destroy(brtp_parser_t parser_)
{
    brtp_parser_mpeg4_t parser = (brtp_parser_mpeg4_t)parser_;

    BDBG_OBJECT_ASSERT(parser, brtp_parser_mpeg4_t);
    b_rtp_parser_mpeg4_stop(parser_);

    BDBG_OBJECT_DESTROY(parser, brtp_parser_mpeg4_t);
    BKNI_Free(parser);
    return;
}

brtp_parser_t
brtp_parser_mpeg4_create(const brtp_parser_mpeg4_cfg *cfg)
{
    brtp_parser_mpeg4_t parser;

    BDBG_ASSERT(cfg);
    parser = BKNI_Malloc(sizeof(*parser));
    if (!parser) {
        BDBG_ERR(("brtp_parser_mpeg4_create: out of memory"));
        return NULL;
    }
    BDBG_OBJECT_INIT(parser, brtp_parser_mpeg4_t);
    brtp_parser_init(&parser->parent);
    parser->parent.packet = b_rtp_parser_mpeg4_packet;
    parser->parent.discontinuity = b_rtp_parser_mpeg4_discontinuity;
    parser->parent.start = b_rtp_parser_mpeg4_start;
    parser->parent.stop = b_rtp_parser_mpeg4_stop;
    parser->parent.destroy = b_rtp_parser_mpeg4_destroy;
    parser->parent.type = brtp_parser_mpeg4;
    parser->cfg = *cfg;
    return &parser->parent;
}
