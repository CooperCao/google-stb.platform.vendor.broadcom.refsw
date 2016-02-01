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
 * Media parser library,  FLV (Flash Video) format
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bflv_parser.h"
#include "biobits.h"
#include "bmedia_util.h"
#include "bmpeg4_util.h"

BDBG_MODULE(bflv_parser);

BDBG_OBJECT_ID(bflv_parser_t);

#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x) */


typedef enum b_flv_parser_state {
    b_flv_parser_state_capture,
    b_flv_parser_state_tag,
    b_flv_parser_state_discard,
    b_flv_parser_state_header,
    b_flv_parser_state_format_error
} b_flv_parser_state;

#define B_FLV_VEC_VP6_FRAME 0
#define B_FLV_VEC_BCMV_EOS  1
#define B_FLV_VEC_H264_EOS  2

struct bflv_parser {
    BDBG_OBJECT(bflv_parser_t)
    BLST_S_HEAD(b_flv_handlers, bflv_parser_handler) handlers;
    b_flv_parser_state state;
    batom_accum_t acc;
    batom_accum_t dst;
    batom_factory_t factory;
    const batom_vec *vecs[3];
    struct {
        bflv_parser_handler *handler;
        uint32_t timestamp;
        size_t size; /* either size of object to capture or number of bytes to drop */
        uint8_t tag_type;
        uint8_t meta;
        struct {
            struct {
                uint8_t soundFormat;
            } audio;
            struct {
                uint8_t codecId;
            } video;
        } parsed_meta;
        struct {
            size_t nalu_len;
            void *meta;
            batom_vec nal;
            batom_vec rai;
        } avc;
        struct {
            bmedia_adts_header adts;
        } aac;
    } tag;
    uint64_t acc_off;
    bflv_parser_cfg cfg;
};

static bflv_parser_handler *
b_flv_test_tag(bflv_parser_t parser, unsigned tag_type)
{
    bflv_parser_handler *handler;

    for(handler=BLST_S_FIRST(&parser->handlers);handler;handler=BLST_S_NEXT(handler, link)) {
        if(handler->tag_type==tag_type) {
            return handler;
        }
    }
    return NULL;
}

    static bool
b_flv_parser_format_error(bflv_parser_t parser)
{
    BDBG_WRN(("b_flv_parser_format_error: %#lx format error at %#lx", (unsigned long)parser, (unsigned long)parser->acc_off));
    parser->state = b_flv_parser_state_format_error;
    return true;
}

/*
 * Macromedia Flash (SWF) and Flash  Video (FLV) File Format Specification  Version 8
 */

    static bool
b_flv_parser_header(bflv_parser_t parser, batom_cursor *cursor)
{
    uint32_t header;
    int version;
    uint32_t data_offset;
    unsigned header_size;
    batom_checkpoint chk;

    BDBG_MSG_TRACE(("b_flv_parser_header: %#lx", (unsigned long)parser));

    BATOM_SAVE(cursor, &chk);

    /* page The FLV Header, p. 258 */
    header = batom_cursor_uint24_be(cursor);
    version = batom_cursor_next(cursor);
    batom_cursor_skip(cursor,1); /* skip flags audio/flags video */
    data_offset = batom_cursor_uint32_be(cursor);
    if(BATOM_IS_EOF(cursor)) {
        goto rollback;
    }
    BDBG_MSG_TRACE(("b_flv_parser_header: %#lx header:%#x version:%u data_offset:%u", (unsigned long)parser, (unsigned)header, (unsigned)version, (unsigned)data_offset));
    if (header != 0x464C56) {
        goto invalid_format;
    }
    if (version != 1) {
        BDBG_WRN(("b_flv_parser_header: %#lx unsupported version %d", (unsigned long)parser, version));
        goto invalid_format;
    }
    header_size = 3 /* header */+ 1/* version */+ 1/* flags */ + 4 /* data_offset */;
    if(data_offset< 3 + 1 + 1 + 4) {
        /* data offset shall be larger or equal to the header size */
        goto invalid_format;
    }
    parser->state = b_flv_parser_state_discard;
    parser->tag.size = data_offset - header_size;
    return true;

rollback:
    batom_cursor_rollback(cursor, &chk);
    return false;
invalid_format:
    return b_flv_parser_format_error(parser);
}

static bool
b_flv_parser_tag(bflv_parser_t parser, batom_cursor *cursor)
{
    uint8_t tag_type;
    uint32_t stream_id;
    uint32_t timestamp;
    bflv_parser_handler *handler;
    uint32_t size;
    int meta;
    batom_checkpoint chk;

    BDBG_MSG_TRACE(("b_flv_parser_tag: %#lx", (unsigned long)parser));
    /* The FLV File Body, p. 258 */
    BATOM_SAVE(cursor, &chk);
    batom_cursor_skip(cursor, sizeof(uint32_t) /* PreviousTagSize0 */);
    /* FLV Tags,  p. 259 */
    tag_type = batom_cursor_byte(cursor);
    size = batom_cursor_uint24_be(cursor);
    timestamp = batom_cursor_uint24_be(cursor);
    timestamp |= ((uint32_t)batom_cursor_byte(cursor))<<24;
    stream_id = batom_cursor_uint24_be(cursor);
    if(BATOM_IS_EOF(cursor)) {
        goto rollback;
    }
    BDBG_MSG_TRACE(("b_flv_parser_tag: %#lx tag_type:%u size:%u timestamp:%u", (unsigned long)parser, (unsigned)tag_type, (unsigned)size, (unsigned)timestamp));
    if(stream_id!=0) {
        BDBG_WRN(("b_flv_parser_tag: %#lx StreamID shall be 0 %#x", (unsigned long)parser, (unsigned)stream_id));
        goto invalid_format;
    }
    handler = b_flv_test_tag(parser, tag_type);
    parser->tag.handler = handler;
    parser->tag.tag_type = tag_type;
    parser->tag.size = size;
    if(handler && size>0) {
        switch(tag_type) {
        case B_FLV_TAG_AUDIO: /* audio tag */
        case B_FLV_TAG_VIDEO: /* video tag */
            meta = batom_cursor_next(cursor);
            if(meta==BATOM_EOF) {
                goto rollback;
            }
            size--;
            parser->tag.size = size;
            parser->tag.meta = (uint8_t)meta;
            if(tag_type==B_FLV_TAG_AUDIO) {
                parser->tag.parsed_meta.audio.soundFormat = B_GET_BITS(meta,7,4);
#if BDBG_DEBUG_BUILD
                {
                    static const char * const sound_format[16]={"uncompressed","ADPCM","MP3","Linear PCM","Nellymoser 16kHz mono","Nellymoser 8kHz mono","Nellymoser","G.711 A-law","G.711 mu-law","-9-","AAC","-11-","-12-","-13-","MP3 8kHz","-15-"};
                    static const uint16_t sound_rate[4]={5513, 11025, 22050, 44100};
                    BDBG_MSG(("b_flv_parser_tag: %#lx len:%u time:%u audio format:%s rate:%u size:%ubit type:%s", (unsigned long)parser, (unsigned)size, (unsigned)timestamp, sound_format[parser->tag.parsed_meta.audio.soundFormat], (unsigned)sound_rate[B_GET_BITS(meta,3,2)], B_GET_BIT(meta,1)?16:8, B_GET_BIT(meta,0)?"stereo":"mono"));
                }
#endif
            } else { /* B_FLV_TAG_VIDEO */
                parser->tag.parsed_meta.video.codecId = B_GET_BITS(meta,3,0);
#if BDBG_DEBUG_BUILD
                {
                    static const char * const frame_type[16]={"","key","inter","disposable inter","", "","","","", "","","",""};
                    static const char * const codec_id[16]={"-0-", "-1-", "Sorenson H.263", "Screen video", "On2 VP6", "On2 VP6 with alpha channel", "Screen video version 2", "AVC", "-8-","-9-","-10-","-11-", "-12-", "-13-", "-14-", "-15"};
                    BDBG_MSG(("b_flv_parser_tag: %#lx len:%u time:%u video frame:'%s' codec:'%s'", (unsigned long)parser, (unsigned)size, (unsigned)timestamp, frame_type[B_GET_BITS(meta,7,4)], codec_id[parser->tag.parsed_meta.video.codecId]));
                }
#endif
            }
            break;

        default:
            parser->tag.meta = 0;
            parser->tag.size = size;
            break;
        }
        if(parser->state != b_flv_parser_state_discard) {
            parser->state = b_flv_parser_state_capture;
            parser->tag.timestamp = timestamp;
        }
    } else {
        parser->state = b_flv_parser_state_discard;
    }
    return true;
rollback:
    batom_cursor_rollback(cursor, &chk);
    return false;
invalid_format:
    return b_flv_parser_format_error(parser);
}




static bool
b_flv_parser_copy_avc_meta(bflv_parser_t parser, batom_accum_t src, batom_cursor *cursor, batom_accum_t dst, unsigned sets)
{
    unsigned i;
    batom_cursor nal;

    for(i=0;i<sets;i++) {
        unsigned len = batom_cursor_uint16_be(cursor);
        if(BATOM_IS_EOF(cursor)) {
            return false;
        }
        BATOM_CLONE(&nal, cursor);
        if(len!=batom_cursor_skip(cursor, len)) {
            return false;
        }
        batom_accum_add_vec(dst, &parser->tag.avc.nal);
        if(!batom_accum_append(dst, src, &nal, cursor)) {
            return false;
        }
    }
    return true;
}

static batom_t
b_flv_parser_handle_avc_meta(bflv_parser_t parser, batom_cursor *cursor, bmedia_packet_header *header)
{
    int word;
    unsigned sets;
    batom_t atom;

    batom_cursor_skip(cursor, 1/*Reserved*/+1/*Profile*/+1/*Reserved*/+1/*Level*/);
    word = batom_cursor_next(cursor);

    /* get the NALU len */
    parser->tag.avc.nalu_len = 1+B_GET_BITS(word, 1, 0);

    word = batom_cursor_next(cursor);
    if(word==BATOM_EOF) { goto error; }
    /* Determine the number of SPS sets and set a cursor to the start of the SPS data */
    sets = B_GET_BITS(word,4,0);
    if(!b_flv_parser_copy_avc_meta(parser, parser->acc, cursor, parser->dst, sets)) { goto error;}

    word = batom_cursor_next(cursor);
    if(word==BATOM_EOF) { goto error; }

    /* Determine the number of PPS sets and set a cursor to the start of the PPS data */
    sets = B_GET_BITS(word,4,0);
    if(!b_flv_parser_copy_avc_meta(parser, parser->acc, cursor, parser->dst, sets)) { goto error;}

    batom_accum_add_vec(parser->dst, &parser->tag.avc.rai);

    header->pts_valid = false;
    atom = batom_from_accum(parser->dst, bmedia_atom, header);
    return atom;

error:
    batom_accum_clear(parser->dst);
    return NULL;
}

static batom_t
b_flv_parser_handle_h264(bflv_parser_t parser, batom_cursor *cursor, size_t payload_length, bmedia_packet_header *header)
{
    size_t left;
    batom_t atom;
    uint8_t avcPacketType;
    int32_t compositionTimeOffset;

    batom_accum_clear(parser->dst);

    /* AVCPacketType + CompositionTime = 4 bytes */
    avcPacketType = batom_cursor_byte(cursor);
    compositionTimeOffset = batom_cursor_uint24_be(cursor);
    /* sign extend 24 bit to 32 bit */
    compositionTimeOffset = compositionTimeOffset << 8; /* shift left, fill a sign bit  with bit 23 */
    compositionTimeOffset = compositionTimeOffset >> 8; /* shift right, bits 31..24 would stick with a sign bit */

    if(payload_length<4) {
        return NULL;
    }
    payload_length -= 4;

    if(avcPacketType == 0) {
        BDBG_MSG(("b_flv_parser_handle_avc:%p pps/sps:%u", (void *)parser, (unsigned)payload_length));
        /* AVCPacketType of 0 indicates AVCDecoderConfigurationRecord */
        return b_flv_parser_handle_avc_meta(parser, cursor, header);
    }
    else if (avcPacketType != 1) {
        goto error;
    }
    BDBG_MSG(("b_flv_parser_handle_avc:%p frame:%u", (void *)parser, (unsigned)payload_length));
    BMEDIA_PES_SET_PTS(header, bmedia_time2pts(parser->tag.timestamp+compositionTimeOffset, BMEDIA_TIME_SCALE_BASE ));
    if(parser->tag.avc.nalu_len==0) {
        return NULL;
    }

    for(left = payload_length; left > parser->tag.avc.nalu_len;) {
        size_t nal_len;
        batom_cursor nal_cursor;

        nal_len = batom_cursor_vword_be(cursor, parser->tag.avc.nalu_len);
        left -= parser->tag.avc.nalu_len;
        if(nal_len>left) {
            goto error;
        }

        BATOM_CLONE(&nal_cursor, cursor);
        if(batom_cursor_skip(cursor, nal_len) !=  nal_len) {
            goto error;
        }

        left -= nal_len;
        batom_accum_add_vec(parser->dst, &parser->tag.avc.nal);
        batom_accum_append(parser->dst, parser->acc, &nal_cursor, cursor);
    }
    atom = batom_from_accum(parser->dst, bmedia_atom, header);
    return atom;

error:
    batom_accum_clear(parser->dst);
    return NULL;
}

typedef struct b_flv_aac_hdr {
    bmedia_packet_header pes;
    uint8_t header[BMEDIA_ADTS_HEADER_SIZE];
} b_flv_aac_hdr;

static const batom_user b_flv_aac_pkt = {
    NULL,
    sizeof(b_flv_aac_hdr)
};

static batom_t
b_flv_parser_handle_aac(bflv_parser_t parser, batom_cursor *cursor, const batom_cursor *last, size_t payload_length, bmedia_packet_header *header)
{
    uint8_t aacPacketType;

    batom_accum_clear(parser->dst);
    if(payload_length<=1) {
        goto error;
    }

    /* AVCPacketType + CompositionTime = 4 bytes */
    aacPacketType = batom_cursor_byte(cursor);
    if(aacPacketType==0) {
        bmedia_info_aac aac;
        if(!bmedia_info_probe_aac_info(cursor, &aac)) {
            goto error;
        }
        bmedia_adts_header_init_aac(&parser->tag.aac.adts, &aac);
    } else if (aacPacketType==1) {
        size_t len;
        b_flv_aac_hdr aac_hdr;

        len = bmedia_adts_header_fill(aac_hdr.header, &parser->tag.aac.adts, payload_length-1);
        BDBG_ASSERT(len<=sizeof(aac_hdr.header));
        aac_hdr.pes = *header;
        aac_hdr.pes.header_off = 0;
        aac_hdr.pes.header_len = len;
        return batom_accum_extract(parser->acc, cursor, last, &b_flv_aac_pkt, &aac_hdr);
    }

error:
    batom_accum_clear(parser->dst);
    return NULL;
}

static batom_t
b_flv_parser_handle_bcmv(bflv_parser_t parser, batom_cursor *first, const batom_cursor *last, size_t payload_length, bmedia_packet_header *header)
{
    batom_t atom;
    bmedia_bcmv_hdr hdr;

    hdr.pes = *header;
    hdr.pes.header_type = B_FLV_VEC_VP6_FRAME;
    bmedia_bcmv_hdr_init(&hdr, payload_length);
    atom = batom_accum_extract(parser->acc, first, last, &bmedia_bcmv_atom, &hdr);
    return atom;
}

static void
b_flv_parser_eos(bflv_parser_t parser)
{
    batom_t pes_atom;
    bmedia_packet_header hdr;
    bflv_parser_handler *handler;

    BDBG_MSG(("b_flv_parser_eos:%p", (void *)parser));

    BDBG_OBJECT_ASSERT(parser, bflv_parser_t);

    handler = b_flv_test_tag(parser, B_FLV_TAG_VIDEO);
    if(handler) {
        BMEDIA_PACKET_HEADER_INIT(&hdr);
        switch(parser->tag.parsed_meta.video.codecId) {
        case B_FLV_CODECID_ON2_VP6:
        case B_FLV_CODECID_ON2_VP6_ALPHA:
        case B_FLV_CODECID_S263:
             hdr.header_type = B_FLV_VEC_BCMV_EOS;
             break;
        case B_FLV_CODECID_H264:
             hdr.header_type = B_FLV_VEC_H264_EOS;
             break;
        }
        if(hdr.header_type) {
            hdr.header_type |= B_MEDIA_PACKET_FLAG_EOS;
            BMEDIA_PES_SET_PTS(&hdr, bmedia_time2pts(parser->tag.timestamp, BMEDIA_TIME_SCALE_BASE ));
            pes_atom = batom_empty(parser->factory, bmedia_atom, &hdr);
            if(pes_atom) {
                handler->handler(handler, pes_atom, parser->tag.parsed_meta.video.codecId);
            }
            pes_atom = batom_empty(parser->factory, bmedia_atom, &hdr);
            if(pes_atom) {
                handler->handler(handler, pes_atom, parser->tag.parsed_meta.video.codecId);
            }
        }
    }
    return;
}

size_t
bflv_parser_feed(bflv_parser_t parser, batom_pipe_t pipe)
{
    size_t len=0;
    batom_t atom;
    bflv_parser_action action = bflv_parser_action_none;
    bool eos_detected = false;

    BDBG_OBJECT_ASSERT(parser, bflv_parser_t);
    BDBG_MSG_TRACE(("bflv_parser_feed>:%#lx %#lx %#lu", (unsigned long)parser, (unsigned long)pipe, (unsigned long)batom_accum_len(parser->acc)));
    atom=batom_pipe_pop(pipe);
    do {
        bool want_continue;
        BDBG_MSG(("bflv_parser_feed:%#lx atom:%#lx", (unsigned long)parser, (unsigned long)atom));
        if (atom) {
            size_t atom_len = batom_len(atom);
            eos_detected = atom_len==0;
            len += atom_len;
            batom_accum_add_atom(parser->acc, atom);
            batom_release(atom);
        }
        do {
            size_t acc_len = batom_accum_len(parser->acc);
            batom_cursor cursor;
            want_continue = false;

            BDBG_MSG_TRACE(("bflv_parser_feed:%#lx acc_len:%u", (unsigned long)parser, acc_len));
            batom_cursor_from_accum(&cursor, parser->acc);
            switch(parser->state) {
            case b_flv_parser_state_tag:
                want_continue = b_flv_parser_tag(parser, &cursor);
                break;
            case b_flv_parser_state_capture:
                if(acc_len>=parser->tag.size) {
                    size_t payload_length = parser->tag.size;
                    batom_t atom;
                    batom_cursor first;
                    bmedia_packet_header header;

                    BATOM_CLONE(&first,&cursor);
                    batom_cursor_skip(&cursor, payload_length);
                    BMEDIA_PACKET_HEADER_INIT(&header);
                    BMEDIA_PES_SET_PTS(&header, bmedia_time2pts(parser->tag.timestamp, BMEDIA_TIME_SCALE_BASE ));

                    switch(parser->tag.tag_type) {
                    case B_FLV_TAG_AUDIO: /* audio tag */
                        switch(parser->tag.parsed_meta.audio.soundFormat) {
                        case B_FLV_SOUNDFORMAT_AAC:
                            atom = b_flv_parser_handle_aac(parser, &first, &cursor, payload_length, &header);
                            break;
                        default:
                            atom = batom_accum_extract(parser->acc, &first, &cursor, bmedia_atom, &header);
                            break;
                        }
                        break;
                    case B_FLV_TAG_VIDEO: /* video tag */
                        switch(parser->tag.parsed_meta.video.codecId) {
                        case B_FLV_CODECID_ON2_VP6:
                        case B_FLV_CODECID_ON2_VP6_ALPHA:
                        case B_FLV_CODECID_S263:
                            atom = b_flv_parser_handle_bcmv(parser, &first, &cursor, payload_length, &header);
                            break;
                        case B_FLV_CODECID_H264:
                            if(parser->tag.avc.meta) {
                                atom = b_flv_parser_handle_h264(parser, &first, payload_length, &header);
                                break;
                            }
                            /* keep going */
                        default:
                            atom = batom_accum_extract(parser->acc, &first, &cursor, bmedia_atom, &header);
                            break;
                        }
                        break;
                    case B_FLV_TAG_SCRIPT:
                        atom = batom_accum_extract(parser->acc, &first, &cursor, bmedia_atom, &header);
                        break;
                    default:
                        atom = NULL;
                        break;
                    }

                    if (atom) {
                        BDBG_ASSERT(parser->tag.handler);
                        BDBG_MSG_TRACE(("bflv_parser_feed: %#lx payload %#lx:%#lx:%u", (unsigned long)parser, (unsigned long)parser->tag.handler, (unsigned long)atom, parser->tag.size));
                        /* batom_dump(atom, "data"); */
                        action = parser->tag.handler->handler(parser->tag.handler, atom, parser->tag.meta);
                    }
                    parser->tag.handler = NULL;
                    parser->tag.size = 0;
                    parser->state = b_flv_parser_state_tag;
                    want_continue = true;
                }
                break;
            case b_flv_parser_state_discard:
                acc_len = batom_cursor_skip(&cursor, parser->tag.size);
                BDBG_ASSERT(acc_len<=parser->tag.size);
                parser->tag.size -= acc_len;
                if (parser->tag.size==0) {
                    parser->state = b_flv_parser_state_tag;
                    want_continue = true;
                }
                break;
            case b_flv_parser_state_header:
                want_continue = b_flv_parser_header(parser, &cursor);
                break;
            case b_flv_parser_state_format_error:
                BDBG_MSG_TRACE(("bflv_parser_feed: %lx dropping %u bytes", (unsigned long)parser, acc_len));
                batom_cursor_skip(&cursor, acc_len);
                break;
            }
            parser->acc_off += batom_cursor_pos(&cursor);
            batom_accum_trim(parser->acc, &cursor);
            if(action == bflv_parser_action_return) {
                goto done;
            }
        } while(want_continue);
    } while(NULL!=(atom=batom_pipe_pop(pipe)));
    if(eos_detected) {
        b_flv_parser_eos(parser);
    }
done:
    return len;
}

void
bflv_parser_default_cfg(bflv_parser_cfg *cfg)
{
    BDBG_ASSERT(cfg);
    BKNI_Memset(cfg, 0, sizeof(*cfg));
    cfg->alloc = NULL;
    return;
}

bflv_parser_t
bflv_parser_create(batom_factory_t factory, const bflv_parser_cfg *cfg)
{
    bflv_parser_t parser;
    size_t meta_size;

    BDBG_ASSERT(cfg);
    BDBG_ASSERT(factory);

    parser = BKNI_Malloc(sizeof(*parser));
    if (!parser) {
        BDBG_ERR(("bflv_parser_create: out of memory %u", (unsigned)sizeof(*parser)));
        goto err_alloc_parser;
    }
    BDBG_OBJECT_INIT(parser, bflv_parser_t);
    parser->cfg = *cfg;
    meta_size = bmedia_rai_h264.len +  bmedia_nal_vec.len;
    parser->tag.avc.meta = NULL;
    if(parser->cfg.alloc) {
        parser->tag.avc.meta = parser->cfg.alloc->bmem_alloc(parser->cfg.alloc, meta_size);
        if(parser->tag.avc.meta == NULL) {
            (void)BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);goto err_meta_alloc;
        }
        BATOM_VEC_INIT(&parser->tag.avc.nal, parser->tag.avc.meta, bmedia_nal_vec.len);
        BATOM_VEC_INIT(&parser->tag.avc.rai, (const uint8_t *)parser->tag.avc.nal.base + parser->tag.avc.nal.len, bmedia_rai_h264.len);
        BKNI_Memcpy(parser->tag.avc.nal.base, bmedia_nal_vec.base, parser->tag.avc.nal.len);
        BKNI_Memcpy(parser->tag.avc.rai.base, bmedia_rai_h264.base, parser->tag.avc.rai.len);
    }

    BLST_S_INIT(&parser->handlers);
    parser->factory = factory;
    parser->vecs[B_FLV_VEC_VP6_FRAME] = &bmedia_frame_bcmv;
    parser->vecs[B_FLV_VEC_BCMV_EOS] = &bmedia_eos_bcmv;
    parser->vecs[B_FLV_VEC_H264_EOS] = &bmedia_eos_h264;
    parser->acc = batom_accum_create(factory);
    if(!parser->acc) {
        goto err_alloc_accum;
    }
    parser->dst = batom_accum_create(factory);
    if(!parser->dst) {
        goto err_alloc_accum_dst;
    }
    bflv_parser_reset(parser);
    return parser;

err_alloc_accum_dst:
    batom_accum_destroy(parser->acc);
err_alloc_accum:
    if(parser->cfg.alloc) {
        parser->cfg.alloc->bmem_free(parser->cfg.alloc, parser->tag.avc.meta);
    }
err_meta_alloc:
    BKNI_Free(parser);
err_alloc_parser:
    return NULL;
}


void
bflv_parser_destroy(bflv_parser_t parser)
{
    BDBG_OBJECT_ASSERT(parser, bflv_parser_t);
    batom_accum_destroy(parser->acc);
    batom_accum_destroy(parser->dst);
    if(parser->cfg.alloc) {
        parser->cfg.alloc->bmem_free(parser->cfg.alloc, parser->tag.avc.meta);
    }
    BDBG_OBJECT_DESTROY(parser, bflv_parser_t);
    BKNI_Free(parser);
    return;
}

void
bflv_parser_remove_handler(bflv_parser_t parser, bflv_parser_handler *handler)
{
    BDBG_OBJECT_ASSERT(parser, bflv_parser_t);
    BDBG_ASSERT(handler);
    BLST_S_REMOVE(&parser->handlers, handler, bflv_parser_handler, link);
    return;
}

void
bflv_parser_install_handler(bflv_parser_t parser, bflv_parser_handler *handler, uint8_t tag_type, bflv_parser_handler_cb tag_handler)
{
    BDBG_OBJECT_ASSERT(parser, bflv_parser_t);
    BDBG_ASSERT(handler);
    handler->tag_type = tag_type;
    handler->handler = tag_handler;
    BLST_S_INSERT_HEAD(&parser->handlers, handler, link);
    return;
}

void
bflv_parser_reset(bflv_parser_t parser)
{
    BDBG_OBJECT_ASSERT(parser, bflv_parser_t);
    batom_accum_clear(parser->acc);
    batom_accum_clear(parser->dst);
    parser->state = b_flv_parser_state_header;
    parser->tag.handler = NULL;
    parser->tag.size = 0;
    parser->acc_off = 0;
    parser->tag.avc.nalu_len = 0;

    return;
}

void
bflv_parser_get_stream_cfg(bflv_parser_t parser, bmedia_pes_stream_cfg *cfg)
{
    BDBG_OBJECT_ASSERT(parser, bflv_parser_t);
    BDBG_ASSERT(cfg);
    bmedia_pes_default_stream_cfg(cfg);
    cfg->nvecs = sizeof(parser->vecs)/sizeof(parser->vecs[0]);
    cfg->vecs = parser->vecs;
    return;
}
