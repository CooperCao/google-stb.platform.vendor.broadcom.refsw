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
 * BMedia library, stream probe module
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bflv_probe.h"
#include "bflv_parser.h"
#include "bkni.h"
#include "biobits.h"


BDBG_MODULE(bflv_probe);

typedef struct bflv_probe  *bflv_probe_t;

typedef struct b_flv_probe_object_handler {
    bflv_parser_handler handler; /* must be first */
    bflv_probe_t probe; /* pointer to probe */
    bool found;
    uint8_t meta;
} b_flv_probe_object_handler;

struct bflv_probe {
    BDBG_OBJECT(bflv_probe_t)
    bflv_parser_t parser;
    b_flv_probe_object_handler video;
    b_flv_probe_object_handler audio;
    b_flv_probe_object_handler script;
    struct {
        bool valid;
        unsigned value;
    } duration;
    struct {
        bool valid;
        unsigned value;
    } videoWidth;
    struct {
        bool valid;
        unsigned value;
    } videoHeight;
};

BDBG_OBJECT_ID(bflv_probe_t);
typedef enum bflv_script_datavalue_type {
    bflv_script_datavalue_type_Number = 0,
    bflv_script_datavalue_type_Boolean = 1,
    bflv_script_datavalue_type_String = 2,
    bflv_script_datavalue_type_Object = 3,
    bflv_script_datavalue_type_MovieClip = 4,
    bflv_script_datavalue_type_Null = 5,
    bflv_script_datavalue_type_Undefined = 6,
    bflv_script_datavalue_type_Reference = 7,
    bflv_script_datavalue_type_ECMA_array = 8,
    bflv_script_datavalue_type_Object_end_marker = 9,
    bflv_script_datavalue_type_Strict_array = 10,
    bflv_script_datavalue_type_Date = 11,
    bflv_script_datavalue_type_Long_string = 12
} bflv_script_datavalue_type;

union bflv_script_double {
    double v_double;
    uint64_t v_uint64_t;
};

struct bflv_script_string {
    size_t len;
    char  buf[128];
};

struct bflv_script_datavalue {
    bflv_script_datavalue_type type;
    union {
        struct bflv_script_string string;
        union bflv_script_double number;
        bool v_bool;
        struct {
            uint32_t approximateLength;
        } ECMA_array;
        uint16_t reference;
        struct {
            uint32_t length;
        } Strict_array;
        struct {
            union bflv_script_double dateTime;
            int32_t localDateTimeOffset;
        } date;
    } data;
};

static bool b_flv_read_string(batom_cursor *cursor, bflv_script_datavalue_type type, struct bflv_script_string *string)
{
    unsigned data;
    if(type==bflv_script_datavalue_type_String) {
        data = batom_cursor_uint16_be(cursor);
    } else {
        data = batom_cursor_uint32_be(cursor);
    }
    if(BATOM_IS_EOF(cursor)) {
        return false;
    }
    string->len = data;
    if(data >= sizeof(string->buf)) {
        size_t skip = data - (sizeof(string->buf) -1);

        BDBG_MSG(("truncating %u string to %u", data, (unsigned)sizeof(string->buf)));
        data = sizeof(string->buf)-1;
        string->buf[data] = '\0';
        if(batom_cursor_copy(cursor, string->buf, data) != data) {
            return false;
        }
        if(batom_cursor_skip(cursor,skip) != skip) {
            return false;
        }
    } else {
        string->buf[data] = '\0';
        if(batom_cursor_copy(cursor, string->buf, data) != data) {
            return false;
        }
    }
    return true;
}


static bool
b_flv_read_datavalue(batom_cursor *cursor, struct bflv_script_datavalue *v)
{
    int byte;

    byte = batom_cursor_next(cursor);
    if(byte==BATOM_EOF) {
        return false;
    }
    BDBG_MSG(("b_flv_read_datavalue: type %u", (unsigned)byte));
    v->type = byte;
    switch(byte) {
    case bflv_script_datavalue_type_Number:
        v->data.number.v_uint64_t = batom_cursor_uint64_be(cursor);
        break;
    case bflv_script_datavalue_type_Boolean:
        byte = batom_cursor_next(cursor);
        v->data.v_bool = byte != 0;
        break;
    case bflv_script_datavalue_type_String:
    case bflv_script_datavalue_type_Long_string:
        if(!b_flv_read_string(cursor, byte, &v->data.string)) {
            return false;
        }
        break;
    case bflv_script_datavalue_type_Object:
    case bflv_script_datavalue_type_MovieClip:
    case bflv_script_datavalue_type_Null:
    case bflv_script_datavalue_type_Undefined:
    case bflv_script_datavalue_type_Object_end_marker:
        break;
    case bflv_script_datavalue_type_Reference:
        v->data.reference = batom_cursor_uint16_be(cursor);
        break;
    case bflv_script_datavalue_type_ECMA_array:
        v->data.ECMA_array.approximateLength = batom_cursor_uint32_be(cursor);
        break;
    case bflv_script_datavalue_type_Strict_array:
        v->data.Strict_array.length = batom_cursor_uint32_be(cursor);
        break;
    case bflv_script_datavalue_type_Date:
        v->data.date.dateTime.v_uint64_t = batom_cursor_uint64_be(cursor);
        v->data.date.localDateTimeOffset = (int16_t)batom_cursor_uint16_be(cursor);
        break;
    default:
        return false;
    }
    return !BATOM_IS_EOF(cursor);
}

static bool
b_flv_parse_is_primitive_type(bflv_script_datavalue_type type)
{
    switch(type) {
    case bflv_script_datavalue_type_Number:
    case bflv_script_datavalue_type_Boolean:
    case bflv_script_datavalue_type_String:
    case bflv_script_datavalue_type_Null:
    case bflv_script_datavalue_type_Undefined:
    case bflv_script_datavalue_type_Reference:
    case bflv_script_datavalue_type_Date:
    case bflv_script_datavalue_type_Long_string:
        return true;
    default:
        break;
    }
    return false;
}


static void
b_flv_parse_script_data(bflv_probe_t probe, batom_t data)
{
    batom_cursor cursor;
    batom_cursor_from_atom(&cursor, data);
    /*
      ADOBE FLASH VIDEO FILE FORMAT SPECIFICATION VERSION 10.1 74
        The FLV File Format

      E.4.4.1 SCRIPTDATA
    */
    for(;;) {
        struct bflv_script_datavalue n,v;
        if(!b_flv_read_datavalue(&cursor, &n)) {
            break;
        }
        if(n.type != bflv_script_datavalue_type_String) {
            BDBG_WRN(("%p:Unsupported top level type:%#x", (void *)probe, n.type));
            break;
        }
        BDBG_MSG(("variable:%s", n.data.string.buf));
        if(!b_flv_read_datavalue(&cursor, &v)) {
            break;
        }
        switch(v.type) {
        case bflv_script_datavalue_type_Object:
        case bflv_script_datavalue_type_ECMA_array:
            for(;;) {
                struct bflv_script_string string;
                struct bflv_script_datavalue d;
                static const char onMetaData[]="onMetaData";

                if(!b_flv_read_string(&cursor, bflv_script_datavalue_type_String, &string)) {
                    goto done;
                }
                if(string.len) {
                    BDBG_MSG(("property:%s.%s", n.data.string.buf, string.buf));
                }
                if(!b_flv_read_datavalue(&cursor, &d)) {
                    break;
                }
                if(d.type == bflv_script_datavalue_type_Object_end_marker) {
                    break;
                }
                if(!b_flv_parse_is_primitive_type(d.type)) {
                    BDBG_WRN(("%p:Unsupported Property type:%u", (void *)probe,d.type));
                    goto done;
                }
                if(BKNI_Memcmp(n.data.string.buf, onMetaData, sizeof(onMetaData))==0) {
                    static const char duration[]="duration";
                    static const char width[]="width";
                    static const char height[]="height";

                    if(d.type == bflv_script_datavalue_type_Number && BKNI_Memcmp(string.buf, duration, sizeof(duration))==0) {
                        probe->duration.valid = true;
                        probe->duration.value = (unsigned)(1000*d.data.number.v_double);
                        BDBG_MSG(("duration %u", probe->duration.value));
                    } else if(d.type == bflv_script_datavalue_type_Number && BKNI_Memcmp(string.buf, width, sizeof(width))==0) {
                        probe->videoWidth.valid = true;
                        probe->videoWidth.value = (unsigned)(d.data.number.v_double);
                        BDBG_MSG(("width %u", probe->videoWidth.value));
                    } else if(d.type == bflv_script_datavalue_type_Number && BKNI_Memcmp(string.buf, height, sizeof(height))==0) {
                        probe->videoHeight.valid = true;
                        probe->videoHeight.value = (unsigned)(d.data.number.v_double);
                        BDBG_MSG(("height %u", probe->videoHeight.value));
                    }
                }
            }
            break;
        case bflv_script_datavalue_type_Strict_array:
            {
                unsigned i;
                for(i=0;i<v.data.Strict_array.length;i++) {
                    struct bflv_script_datavalue d;
                    if(!b_flv_read_datavalue(&cursor, &d)) {
                        break;
                    }
                    if(!b_flv_parse_is_primitive_type(d.type)) {
                        BDBG_WRN(("%p:Unsupported Strict array type:%u", (void *)probe,d.type));
                        goto done;
                    }
                }
            }
        default:
            break;
        }
    }
done:
    return;
}

static bflv_parser_action
b_flv_probe_data(bflv_parser_handler *handler_, batom_t object, uint8_t meta )
{
    b_flv_probe_object_handler *handler = ( b_flv_probe_object_handler *)handler_;

    BDBG_OBJECT_ASSERT(handler->probe, bflv_probe_t);
    switch(handler->handler.tag_type) {
    case B_FLV_TAG_SCRIPT:
        b_flv_parse_script_data(handler->probe, object);
        break;
    default:
        break;
    }
    batom_release(object);
    handler->found = true;
    handler->meta = meta;
    return bflv_parser_action_none;
}

static bmedia_probe_base_t
b_flv_probe_create(batom_factory_t factory)
{
    bflv_probe_t    probe;
    bflv_parser_cfg cfg;

    probe = BKNI_Malloc(sizeof(*probe));
    if(!probe) {
        BDBG_ERR(("b_flv_probe_create: can't allocate %u bytes", (unsigned)sizeof(*probe)));
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(probe, bflv_probe_t);
    probe->video.probe = probe;
    probe->audio.probe = probe;
    probe->script.probe = probe;
    bflv_parser_default_cfg(&cfg);
    probe->parser = bflv_parser_create(factory, &cfg);
    if(!probe->parser) {
        goto err_parser;
    }
    bflv_parser_install_handler(probe->parser, &probe->audio.handler, B_FLV_TAG_AUDIO, b_flv_probe_data);
    bflv_parser_install_handler(probe->parser, &probe->video.handler, B_FLV_TAG_VIDEO, b_flv_probe_data);
    bflv_parser_install_handler(probe->parser, &probe->script.handler, B_FLV_TAG_SCRIPT, b_flv_probe_data);
    return (bmedia_probe_base_t)probe;
err_parser:
    BKNI_Free(probe);
err_alloc:
    return NULL;
}

static void
b_flv_probe_destroy(bmedia_probe_base_t probe_)
{
    bflv_probe_t probe = (bflv_probe_t) probe_;

    BDBG_OBJECT_ASSERT(probe, bflv_probe_t);
    bflv_parser_remove_handler(probe->parser, &probe->video.handler);
    bflv_parser_remove_handler(probe->parser, &probe->audio.handler);
    bflv_parser_destroy(probe->parser);
    BDBG_OBJECT_DESTROY(probe, bflv_probe_t);
    BKNI_Free(probe);
    return;
}

static unsigned
b_media_probe_aquire_flv_duration(bfile_buffer_t buf)
{
    int rc;
    off_t off_first, off_last;
    batom_t atom;
    bfile_buffer_result result;
    batom_cursor cursor;
    uint32_t last_tag;
    uint32_t timestamp = 0;

    rc = bfile_buffer_get_bounds(buf, &off_first, &off_last);
    if(rc!=0) { goto error;}

    /* The last 4 bytes of an FLV file specify the size of the previous tag,
       We can use this to locate the last tag in the file and determine the timestamp */
    atom = bfile_buffer_read(buf, off_last-4, 4, &result);
    if(atom==NULL) { goto error;}

    batom_cursor_from_atom(&cursor, atom);
    last_tag = batom_cursor_uint32_be(&cursor);
    batom_release(atom);
    if(BATOM_IS_EOF(&cursor)) {goto error;}
    if(last_tag+4 >= off_last) {goto error;}

    atom = bfile_buffer_read(buf, off_last-(last_tag+4), last_tag, &result);
    if(atom==NULL) { goto error;}
    batom_cursor_from_atom(&cursor, atom);

    /* TagType(1byte) + DataSize(3bytes) */
    batom_cursor_skip(&cursor, 4);
    timestamp = batom_cursor_uint24_be(&cursor);
    timestamp |= ((uint32_t)batom_cursor_byte(&cursor))<<24;
    batom_release(atom);
    if(BATOM_IS_EOF(&cursor)) {goto error;}

    return timestamp;
error:
    return 0;
}

static const bmedia_probe_stream *
b_flv_probe_parse(bmedia_probe_base_t probe_, bfile_buffer_t buf, batom_pipe_t pipe, const bmedia_probe_parser_config *config)
{
    bflv_probe_t probe = (bflv_probe_t)probe_;
    off_t off;
    size_t read_len = 16384;
    bflv_probe_stream *stream;
    bflv_probe_track *track;

    BDBG_OBJECT_ASSERT(probe, bflv_probe_t);
    probe->audio.found = false;
    probe->video.found = false;
    probe->duration.valid = false;
    probe->videoWidth.valid = false;
    probe->videoHeight.valid = false;
    for(off=0;off<1*1024*1024;) {
        batom_t atom;
        bfile_buffer_result result;
        size_t feed_len;
        size_t atom_len;

        BDBG_MSG(("b_flv_probe_parse: %p reading %u:%u", (void *)probe, (unsigned)off, (unsigned)read_len));
        atom = bfile_buffer_read(buf, off+config->parse_offset, read_len, &result);
        if(result!=bfile_buffer_result_ok) {
            break;
        }
        if(!atom) {
            break;
        }
        atom_len = batom_len(atom);
        BDBG_MSG(("b_flv_probe_parse: %p read %u:%u -> %p", (void *)probe, (unsigned)off, (unsigned)atom_len, (void *)atom));
        off += atom_len;
        batom_pipe_push(pipe, atom);
        feed_len = bflv_parser_feed(probe->parser, pipe);
        if(feed_len!=atom_len) {
            break;
        }
        if(probe->video.found && probe->audio.found) {
            break;
        }
    }
    bflv_parser_reset(probe->parser);
    if(!probe->video.found && !probe->audio.found) { goto err_no_data;}

    /* return result of parsing */
    stream = BKNI_Malloc(sizeof(*stream));
    if(!stream) { goto err_stream; }
    bmedia_probe_stream_init(&stream->media, bstream_mpeg_type_flv);
    if(probe->video.found) {
        track = BKNI_Malloc(sizeof(*track));
        if(!track) { goto err_track_video; }
        bmedia_probe_track_init(&track->media);
        track->media.type = bmedia_track_type_video;
        track->media.number = probe->video.handler.tag_type;
        switch(B_GET_BITS(probe->video.meta,3,0)) {
            case B_FLV_CODECID_S263: track->media.info.video.codec = bvideo_codec_spark; break;
            case B_FLV_CODECID_ON2_VP6_ALPHA: /* VP6 with alpha channel */
            case B_FLV_CODECID_ON2_VP6: track->media.info.video.codec = bvideo_codec_vp6;  break;
            case B_FLV_CODECID_H264: track->media.info.video.codec = bvideo_codec_h264;  break;

            case 3: /* Screen video */
            case 6: /* Screen video V2 */
            default:
                track->media.info.video.codec = bvideo_codec_unknown;  break;
        }
        track->media.info.video.width = 0;
        track->media.info.video.height = 0;
        if(probe->videoWidth.valid) {
            track->media.info.video.width = probe->videoWidth.value;
        }
        if(probe->videoHeight.valid) {
            track->media.info.video.height = probe->videoHeight.value;
        }
        BLST_SQ_INSERT_TAIL(&stream->media.tracks, &track->media, link);
    }
    if(probe->audio.found) {
        static const uint16_t sound_rate[4]={5513, 11025, 22050, 44100};
        track = BKNI_Malloc(sizeof(*track));
        if(!track) { goto err_track_audio; }
        bmedia_probe_track_init(&track->media);
        track->media.type = bmedia_track_type_audio;
        track->media.number = probe->audio.handler.tag_type;
        track->media.info.audio.channel_count = B_GET_BIT(probe->audio.meta,0)?2:1;
        track->media.info.audio.sample_size = B_GET_BIT(probe->audio.meta,1)?16:8;
        track->media.info.audio.sample_rate = sound_rate[B_GET_BITS(probe->audio.meta,3,2)];
        switch(B_GET_BITS(probe->audio.meta,7,4)) {
            case B_FLV_SOUNDFORMAT_MP3:
            case B_FLV_SOUNDFORMAT_MP3_8KHZ: track->media.info.audio.codec = baudio_format_mp3; break;
            case B_FLV_SOUNDFORMAT_AAC: track->media.info.audio.codec = baudio_format_aac; break;
            case 0: /* Linear PCM */ case 1: /* ADPCM */
            case 3: /* Linear PCM, little endian */
            case 4: /* Nellymoser 16kHz mono */
            case 5: /* Nellymoser 8kHz mono */
            case 6: /* Nellymoser */
            case 7: /* G.711 A-law */
            case 8: /* G.711 mu-law */
            default:
                track->media.info.audio.codec = baudio_format_unknown; break;
        }
        BLST_SQ_INSERT_TAIL(&stream->media.tracks, &track->media, link);
    }

    if(probe->duration.valid) {
        stream->media.duration = probe->duration.value;
    } else {
        stream->media.duration = b_media_probe_aquire_flv_duration(buf);
    }
    return &stream->media;

err_track_audio:
err_track_video:
    bmedia_probe_basic_stream_free(probe_, &stream->media);
err_stream:
err_no_data:
    return NULL;
}

static bool
b_flv_probe_header_match(batom_cursor *header)
{
    uint8_t byte0, byte1, byte2;

    byte0 = batom_cursor_byte(header);
    byte1 = batom_cursor_byte(header);
    byte2 = batom_cursor_byte(header);
    if(byte0 != 'F' || byte1 != 'L' || byte2 != 'V') {
        return false;
    }

    byte0 = batom_cursor_byte(header);
    if(byte0 != 1) {
        return false;
    }

    byte0 = batom_cursor_byte(header);
    if(B_GET_BITS(byte0,7,4) != 0) {
        return false;
    }

    if(BATOM_IS_EOF(header)) {
        return false;
    }
    return true;
}

static const bmedia_probe_file_ext b_flv_ext[] =  {
    {"flv"},
    {""}
};

const bmedia_probe_format_desc bflv_probe = {
    bstream_mpeg_type_flv,
    b_flv_ext, /* ext_list */
    /* FLV version type */
    3 +  1  +   1 , /* header_size */
    b_flv_probe_header_match, /* header_match */
    b_flv_probe_create, /* create */
    b_flv_probe_destroy, /* destroy */
    b_flv_probe_parse, /* parse */
    bmedia_probe_basic_stream_free /* stream free */
};

