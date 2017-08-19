/***************************************************************************
 * Copyright (C) 2007-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 * MPEG-2 PES Parser/Demux library
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bmpeg2pes_probe.h"
#include "bmpeg2pes_parser.h"
#include "bmedia_probe_demux.h"
#include "bkni.h"
#include "biobits.h"

BDBG_MODULE(bmpeg2pes_probe);
#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */

BDBG_OBJECT_ID(bmpeg2pes_probe_t);

typedef struct bmpeg2pes_probe *bmpeg2pes_probe_t;

struct bmpeg2pes_probe {
    BDBG_OBJECT(bmpeg2pes_probe_t)
    bmedia_probe_stream *stream;
    batom_factory_t factory;
    bmedia_probe_demux demux;
    uint32_t substream_id; /* substream_id for currently parsed private PES packet */
    bmpeg2pes_demux_t pes_demux;
    off_t file_offset;
    struct {
        bool valid;
        uint8_t pes_id;
        uint32_t first_pts;
        uint32_t last_pts;
        off_t first_offset;
        off_t last_offset;
    } file_pts;
};

typedef struct b_mpeg2pes_packet_info {
    uint8_t stream_id;
    size_t length;
} b_mpeg2pes_packet_info;

static bool
b_mpeg2pes_probe_packet(batom_cursor *cursor, b_mpeg2pes_packet_info *info )
{
    uint32_t scode;
    uint16_t word;

    scode = batom_cursor_uint32_be(cursor);
    word = batom_cursor_uint16_be(cursor);
    if(BATOM_IS_EOF(cursor)) {
        return false;
    }

    if( (scode&0xFFFFFF00) != 0x100) {
        return false;
    }
    info->stream_id = scode&0xFF;
    info->length = word;
    if(bmpeg2pes_decode_stream_id(info->stream_id)==bmpeg2pes_stream_id_type_invalid) {
        if(scode==0x1BB) {
            /* do nothing */
        } else if(scode==0x1BA && B_GET_BITS(word, 15, 14)==0x01) { /* ISO/IEC 13818-1 : 2000 (E) Table 2-33 . Program Stream pack header */
            unsigned header = (2 + 3 + 1 + 15 + 1 + 15 + 1 + 9 + 1 + 22 + 1 + 1 + 5 + 3)/8;
            batom_cursor_skip(cursor, header - (16 + 8 + 8)/8);
            word = batom_cursor_byte(cursor);
            if(BATOM_IS_EOF(cursor)) {
                return false;
            }
            if(B_GET_BITS(word,1,0)!=0x03) { /* marker bit  and marker bit */
                return false;
            }
            word = batom_cursor_byte(cursor);
            if(BATOM_IS_EOF(cursor)) {
                return false;
            }
            info->length = (header-2) + B_GET_BITS(word, 2, 0);
        } else {
            return false;
        }
    }
    BDBG_MSG(("b_mpeg2pes_probe_packet: stream_id:%#x length:%u", info->stream_id, (unsigned)info->length));
    return true;
}

static bool
b_mpeg2pes_probe_header_match(batom_cursor *cursor)
{
    b_mpeg2pes_packet_info info;
    bool match = b_mpeg2pes_probe_packet(cursor, &info);
    BDBG_MSG(("header_match:%u", (unsigned)match));
    return match;
}


static bool
b_mpeg2pes_probe_parse_lpcm(bmpeg2pes_probe_t probe, unsigned stream_id, batom_cursor *payload, size_t len)
{
    static const uint8_t b_lpcm_sample_size[] = {16, 20, 24, 0};
    static const unsigned b_lpcm_sample_rate[] ={48000, 96000, 0, 0};
    /* LPCM audio */
    if(len>=3) { /* we haven't read enough data just drop this packet */
        bmedia_probe_track *track;
        uint32_t word;
        word = batom_cursor_uint24_be(payload);
        BDBG_ASSERT(!BATOM_IS_EOF(payload));
        track = BKNI_Malloc(sizeof(*track));
        if(track) {
            bmedia_probe_track_init(track);
            track->type = bmedia_track_type_audio;
            track->number = stream_id;
            track->info.audio.codec = baudio_format_lpcm_dvd;
            track->info.audio.sample_size = b_lpcm_sample_size[B_GET_BITS(word, 15, 14)];
            track->info.audio.sample_rate = b_lpcm_sample_rate[B_GET_BITS(word, 13, 12)];
            track->info.audio.channel_count = B_GET_BITS(word, 10, 8)+1;
            track->info.audio.bitrate = (track->info.audio.sample_size*track->info.audio.sample_rate*track->info.audio.channel_count)/1024;
            bmedia_probe_add_track(probe->stream, track);
            return true;
        }
    }
    return false;
}

static void
b_mpeg2pes_probe_pes_packet(void *packet_cnxt, batom_accum_t src, batom_cursor *payload, size_t len, const bmpeg2pes_atom_info *info)
{
    bmpeg2pes_probe_t probe  = packet_cnxt;
    batom_cursor first;
    batom_t packet;
    unsigned stream_id;
    bmedia_track_type track_type;
    bmedia_probe_track *track;

    BDBG_OBJECT_ASSERT(probe, bmpeg2pes_probe_t);
    BDBG_MSG_TRACE(("b_mpeg2pes_probe_pes_packet: %#lx stream %#x pes data %u:%u", (unsigned long)probe, (unsigned)info->pes_id, info->data_offset, len));
    BDBG_ASSERT(probe->stream);
    BATOM_CLONE(&first, payload);
    batom_cursor_skip(payload, len);
    stream_id = info->pes_id;
    if(info->flags & BMPEG2PES_PTS_VALID) {
        if(probe->file_pts.valid) {
            if(probe->file_pts.pes_id == info->pes_id) {
                probe->file_pts.last_pts = info->pts;
                probe->file_pts.last_offset = probe->file_offset;
            }
        } else {
            probe->file_pts.valid = true;
            probe->file_pts.pes_id = info->pes_id;
            probe->file_pts.first_pts = info->pts;
            probe->file_pts.first_offset = probe->file_offset;
            probe->file_pts.last_pts = info->pts;
            probe->file_pts.last_offset = probe->file_offset;
        }
    }
    probe->file_offset += len;
    if((stream_id&0xE0)==0xC0) { /* audio stream */
        track_type = bmedia_track_type_audio;
    } else if( (stream_id&0xF0)==0xE0) { /* video stream */
        track_type = bmedia_track_type_video;
    } else if(stream_id == 0xBD) { /* private_stream_1, used in DVD */
        uint32_t substream_id;
        track_type = bmedia_track_type_audio;
        probe->stream->type = bstream_mpeg_type_vob;
        if(info->data_offset!=0) {
            substream_id = probe->substream_id;
            if(substream_id==0) {
                goto drop;
            }
            stream_id |= (substream_id<<8); /* combine PES id and sub stream id */
        } else { /* start of payload */
            probe->substream_id = 0; /* clear substream_id */
            if(len<=4) { /* XXX we haven't read enough data just drop this packet */
                goto drop;
            }
            substream_id = batom_cursor_uint32_be(&first);
            substream_id = (substream_id>>24);
            BDBG_MSG(("b_mpeg2pes_probe_pes_packet: %#lx stream_id:%#x substream_id:%#x", (unsigned long)probe, stream_id, substream_id));
            stream_id |= (substream_id<<8);
            for(track=BLST_SQ_FIRST(&probe->stream->tracks);track;track=BLST_SQ_NEXT(track, link)) {
                if(track->number == stream_id) {
                    goto drop; /* this track is already known */
                }
            }
            if(
                  (substream_id&0xF0) == 0x80  /* AC3 audio */
               || (substream_id&0xF0) == 0x90  /* DTS audio */
               || (substream_id&0xF0) == 0xC0  /* AC3+ audio */
              ) {
                probe->substream_id = substream_id; /* save substream_id */
            } else if((substream_id&0xF0) == 0xA0) {  /* PCM audio */
                if(b_mpeg2pes_probe_parse_lpcm(probe, (substream_id<<8) | stream_id, &first, len-4)) {
                    goto done;
                }
            } else {
                goto drop; /* something else */
            }
        }
    } else {
        if(stream_id == 0xBF) { /* 0xBF is the private dsta, index for DVD VOB */
            probe->stream->type = bstream_mpeg_type_vob;
        }
        goto drop; /* some other stream */
    }
    packet = batom_accum_extract(src, &first, payload, NULL, NULL);

    if(packet) {
        BDBG_MSG(("b_mpeg2pes_probe_pes_packet: %p stream %x pes payload %p:%u", (void *)probe, (unsigned)stream_id, (void *)packet, (unsigned)batom_len(packet)));
        track = bmedia_probe_demux_data(&probe->demux, probe->factory, stream_id, track_type, packet);
        if(track) {
            bmedia_probe_add_track(probe->stream, track);
        }
    }
done:
    return;
drop:
    BDBG_MSG(("b_mpeg2pes_probe_pes_packet: %p dropping id:%#x %u", (void *)probe, info->pes_id, (unsigned)len));
    goto done;
}


static bmedia_probe_base_t
b_mpeg2pes_probe_create(batom_factory_t factory)
{
    bmpeg2pes_probe_t probe;
    bmpeg2pes_demux_config pes_cfg;;

    probe = BKNI_Malloc(sizeof(*probe));
    if(!probe) {
        BDBG_ERR(("b_mpeg2pes_probe_create: can't allocate %u bytes", (unsigned)sizeof(*probe)));
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(probe, bmpeg2pes_probe_t);
    probe->stream = NULL;
    probe->factory = factory;
    bmpeg2pes_demux_default_config(&pes_cfg);
    pes_cfg.packet_cnxt = probe;
    pes_cfg.packet = b_mpeg2pes_probe_pes_packet;
    probe->pes_demux = bmpeg2pes_demux_create(factory, &pes_cfg);
    if(!probe->pes_demux) {
        goto err_pes_demux;
    }
    return (bmedia_probe_base_t)probe;

err_pes_demux:
    BKNI_Free(probe);
err_alloc:
    return NULL;
}

static void
b_mpeg2pes_probe_destroy(bmedia_probe_base_t probe_)
{
    bmpeg2pes_probe_t probe = (bmpeg2pes_probe_t)probe_;

    BDBG_OBJECT_ASSERT(probe, bmpeg2pes_probe_t);
    BDBG_ASSERT(probe->stream==NULL); /* can't destroy probe in middle of parsing */

    bmpeg2pes_demux_destroy(probe->pes_demux);
    BDBG_OBJECT_DESTROY(probe, bmpeg2pes_probe_t);
    BKNI_Free(probe);
    return;
}

static bool b_mpeg2pes_verify_stream(bfile_buffer_t buf, batom_t atom, const bmedia_probe_parser_config *config)
{
    b_mpeg2pes_packet_info info;
    batom_cursor cursor;
    bool result;
    bfile_buffer_result file_result;

    batom_cursor_from_atom(&cursor, atom);
    if(!b_mpeg2pes_probe_packet(&cursor, &info)) {
        return false;
    }
    if(info.length==0) {
        return B_GET_BITS(info.stream_id, 7, 4)==0x0E; /* allow unbounded packets for video only */
    }
    BDBG_MSG(("b_mpeg2pes_verify_stream: reading %u:%u", (unsigned)(info.length + (6-1)+config->parse_offset), BMEDIA_PROBE_FEED_SIZE));
    atom = bfile_buffer_read(buf, config->parse_offset+info.length + (6-1), BMEDIA_PROBE_FEED_SIZE, &file_result );
    if(atom==NULL) { /* if first packet bounded then, it should be possible to read last byte of it */
        return false;
    }
    batom_cursor_from_atom(&cursor, atom);
    batom_cursor_skip(&cursor, 1);
    if(batom_cursor_reserve(&cursor, 4)==0) {
        result = true; /* if this is single packet let it in */
    } else { /* otherwise it should be a valid PES packet */
        result = b_mpeg2pes_probe_packet(&cursor, &info);
    }
    batom_release(atom);
    return result;
}

static const bmedia_probe_stream *
b_mpeg2pes_probe_parse(bmedia_probe_base_t probe_, bfile_buffer_t buf, batom_pipe_t pipe, const bmedia_probe_parser_config *config)
{
    bmpeg2pes_probe_t probe = (bmpeg2pes_probe_t)probe_;
    bmedia_probe_stream *stream;
    off_t off;
    bmpeg2pes_demux_t pes_demux;


    BDBG_OBJECT_ASSERT(probe, bmpeg2pes_probe_t);

    BDBG_ASSERT(probe->stream==NULL);

    stream = BKNI_Malloc(sizeof(*stream));
    if(!stream) {
        goto done;
    }
    bmedia_probe_stream_init(stream, bstream_mpeg_type_pes);
    probe->stream = stream;
    probe->substream_id = 0;
    pes_demux = probe->pes_demux;
    bmedia_probe_demux_init(&probe->demux);
    probe->file_offset = 0;
    probe->file_pts.valid = false;

    for(off=0;;) {
        batom_t atom;
        bfile_buffer_result result;
        size_t feed_len;
        size_t atom_len;
        bmpeg2pes_demux_status status;
        const size_t read_len = BMEDIA_PROBE_FEED_SIZE;

        BDBG_MSG(("b_mpeg2pes_probe_parse: %p reading %u:%u", (void *)probe, (unsigned)(off+config->parse_offset), (unsigned)read_len));
        atom = bfile_buffer_read(buf, config->parse_offset+off, read_len, &result);
        if(!atom) {
            BDBG_MSG(("b_mpeg2pes_probe_parse: %p read completed %u at %u", (void *)probe, result, (unsigned)(off+config->parse_offset)));
            break;
        }
        atom_len = batom_len(atom);
        BDBG_MSG(("b_mpeg2pes_probe_parse: %p read %u:%u -> %p", (void *)probe, (unsigned)(off+config->parse_offset), (unsigned)atom_len, (void *)atom));
        if(off==0) { /* confirm that stream is indeed resembles valid MPEG-2 PES */
            if(!b_mpeg2pes_verify_stream(buf, atom, config)) {
                batom_release(atom);
                BKNI_Free(stream);
                stream = NULL;
                goto done;
            }
        }
        probe->file_offset = off;
        off += atom_len;
        batom_pipe_push(pipe, atom);
        feed_len = bmpeg2pes_demux_feed(pes_demux, pipe);
        if(feed_len!=atom_len) {
            break;
        }
        bmpeg2pes_demux_get_status(pes_demux, &status);
        if(status.nresyncs>8) {
            BDBG_MSG(("b_mpeg2pes_probe_parse: %#lx too much(%u) resyncs ",  (unsigned long)probe, status.nresyncs));
            break;
        }
        BDBG_MSG(("b_mpeg2pes_probe_parse: %#lx parsed:%u tracks:%u completed:%u", (unsigned long)probe, (unsigned)off, probe->demux.tracks, probe->demux.completed));
        if(
           (off>512*1024 && probe->demux.tracks == probe->demux.completed) ||/* parsed 512KBytes and probed all tracks */
           (off>3*1024*1024) /* parsed >3MBytes */
          ) {
            break;
        }
    }
    BDBG_MSG(("b_mpeg2pes_probe_parse: %#lx parsed %u bytes", (unsigned long)probe, (unsigned)off));
    if(probe->file_pts.valid && probe->file_pts.first_pts < probe->file_pts.last_pts) {
        stream->max_bitrate = (8*45000*(probe->file_pts.last_offset - probe->file_pts.first_offset))/(probe->file_pts.last_pts - probe->file_pts.first_pts);
    }
    bmedia_probe_demux_add_unknown(&probe->demux, stream, NULL);
    bmedia_probe_demux_shutdown(&probe->demux);
    bmpeg2pes_demux_reset(pes_demux);

done:
    probe->stream = NULL;
    return stream;
}


static const bmedia_probe_file_ext bmpeg2pes_probe_ext[] =  {
    {"pes"},{"mpg"},{"vob"},{"mpeg"},
    {""}
};

const bmedia_probe_format_desc bmpeg2pes_probe = {
    bstream_mpeg_type_pes,
    bmpeg2pes_probe_ext, /* ext_list */
    sizeof(uint32_t)+8, /* meed first start code, and program stream pack header*/
    b_mpeg2pes_probe_header_match, /* header_match */
    b_mpeg2pes_probe_create, /* create */
    b_mpeg2pes_probe_destroy, /* destroy */
    b_mpeg2pes_probe_parse, /* parse */
    bmedia_probe_basic_stream_free /* stream free */
};


typedef struct b_mpeg2pes_stream_id {
    bmpeg2pes_parser_stream pes; /* must be first */
    struct b_mpeg2pes_pts_parser *parser;
    struct bmedia_timestamp *timestamp;
    off_t sync_offset;
    off_t data_offset;
} b_mpeg2pes_stream_id;


BDBG_OBJECT_ID(b_mpeg2pes_pts_parser);

typedef struct b_mpeg2pes_pts_parser {
    bmedia_timestamp_parser parent;
    BDBG_OBJECT(b_mpeg2pes_pts_parser)
        size_t packet_len;
    b_mpeg2pes_stream_id stream;
} b_mpeg2pes_pts_parser;


static void
b_mpeg2pes_pts_parser_destroy(bmedia_timestamp_parser_t parser_)
{
    b_mpeg2pes_pts_parser *parser = (b_mpeg2pes_pts_parser *)parser_;

    BDBG_OBJECT_DESTROY(parser, b_mpeg2pes_pts_parser);
    BKNI_Free(parser);
    return;
}


static int
b_mpeg2pes_pts_parser_parse(bmedia_timestamp_parser_t parser_, batom_cursor *cursor, bmedia_timestamp *timestamp)
{
    b_mpeg2pes_pts_parser *parser = (b_mpeg2pes_pts_parser *)parser_;
    off_t next_data;
    int rc;

    BDBG_OBJECT_ASSERT(parser, b_mpeg2pes_pts_parser);
    BDBG_ASSERT(cursor);
    BDBG_ASSERT(timestamp);

    next_data = parser->stream.data_offset + batom_cursor_size(cursor);
    parser->stream.timestamp = timestamp;

    for(;;) {
        rc = bmpeg2pes_parser_stream_feed(&parser->stream.pes, NULL, cursor);
        if(rc>=0) { /* some other pid */
            parser->stream.sync_offset = parser->stream.data_offset + batom_cursor_pos(cursor);
        } else if(rc==BMPEG2TS_RESULT_HOLD) {
            BDBG_ASSERT(parser->stream.timestamp==NULL); /* verify that payload handler handled timestamp */
            parser->stream.sync_offset = parser->stream.data_offset + batom_cursor_pos(cursor);
            rc = 1;
            break;
        } else if(rc==BMPEG2TS_RESULT_EOS) {
            rc = 0;
            break;
        } else if(rc==BMPEG2TS_RESULT_SYNC_ERROR) {

            parser->stream.pes.flags = 0;
            parser->stream.pes.npackets = 0;

            for(;;) {
                b_mpeg2pes_packet_info info;
                batom_cursor packet_start;
                BATOM_CLONE(&packet_start, cursor);

                if(b_mpeg2pes_probe_packet(&packet_start, &info)) {
                    if(bmpeg2pes_decode_stream_id(info.stream_id)==bmpeg2pes_stream_id_type_data) {
                        break;
                    } else {
                        batom_cursor_skip(cursor, 6+info.length);
                        if(BATOM_IS_EOF(cursor)) {
                            rc = 0;
                            goto done;
                        }
                        continue;
                    }
                }
                if(BATOM_IS_EOF(&packet_start)) {
                    rc = 0;
                    goto done;
                }
                batom_cursor_skip(cursor, 1);
            }
        }
    }
done:
    parser->stream.data_offset = next_data;
    BDBG_MSG_TRACE(("b_mpeg2pes_pts_parser_parse: %#lx %u %u -> %u:%u", (unsigned long)parser, (unsigned)parser->stream.sync_offset, (unsigned)parser->stream.data_offset, (unsigned)timestamp->offset, (unsigned)timestamp->timestamp));
    return rc;
}


static void
b_mpeg2pes_pts_parser_seek(bmedia_timestamp_parser_t parser_, off_t offset)
{
    b_mpeg2pes_pts_parser *parser = (b_mpeg2pes_pts_parser *)parser_;
    BDBG_OBJECT_ASSERT(parser, b_mpeg2pes_pts_parser);

    BDBG_ASSERT(offset>=0);

    parser->stream.data_offset = offset;
    parser->stream.pes.flags = 0;
    parser->stream.pes.npackets = 0;

    return;
}


static const bmedia_timestamp_parser_methods  b_mpeg2pes_pts_parser_methods = {
    b_mpeg2pes_pts_parser_seek,
    b_mpeg2pes_pts_parser_parse,
    b_mpeg2pes_pts_parser_destroy
};


static bmpeg2ts_parser_action
b_mpeg2pes_pts_parser_pes_payload(bmpeg2pes_parser_stream *stream, unsigned flags, batom_accum_t src, batom_cursor *cursor, size_t len)
{
    b_mpeg2pes_stream_id *stream_id = (b_mpeg2pes_stream_id*)stream;

    BSTD_UNUSED(src);
    BSTD_UNUSED(len);
    BDBG_MSG_TRACE(("b_mpeg2pes_pts_parser_pes_payload: %#lx id:%#x  flags %#x", stream_id->parser, stream_id->pes.stream_id, (unsigned)flags));
    if(flags&BMPEG2PES_PTS_VALID) {
        BDBG_ASSERT(stream_id->timestamp);
        stream_id->timestamp->timestamp = stream->pts;
        stream_id->timestamp->offset = stream_id->data_offset + batom_cursor_pos(cursor);
        stream_id->timestamp = NULL; /* clear timestamp, it signals that timestamp was handled and is useful to detect multiple PTS detections */
        return bmpeg2ts_parser_action_hold;
    }
    return bmpeg2ts_parser_action_skip;
}

bmedia_timestamp_parser_t
bmpeg2pes_pts_parser_create(uint16_t stream_id, size_t packet_len)
{
    b_mpeg2pes_pts_parser *parser;

    parser = BKNI_Malloc(sizeof(*parser));

    if(!parser) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_alloc;}

    BDBG_OBJECT_INIT(parser, b_mpeg2pes_pts_parser);
    BDBG_MSG(("bmpeg2pes_pts_parser_create: %p stream id:%#x packet_len:%u", (void *)parser, (unsigned)stream_id, (unsigned)packet_len));
    parser->parent.methods = &b_mpeg2pes_pts_parser_methods;
    parser->packet_len = packet_len;
    bmpeg2pes_parser_stream_init(&parser->stream.pes, stream_id);
    parser->stream.parser = parser;
    parser->stream.pes.payload = b_mpeg2pes_pts_parser_pes_payload;
    parser->stream.sync_offset = 0;
    parser->stream.data_offset = 0;
    parser->stream.timestamp = NULL;

    return &parser->parent;

err_alloc:
    return NULL;
}
