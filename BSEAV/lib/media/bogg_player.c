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
 * BMedia library, generic(no index) player
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bmedia_player.h"
#include "bogg_player.h"
#include "bkni.h"
#include "bogg_probe.h"

BDBG_MODULE(bogg_player);
typedef struct bogg_player *bogg_player_t;

BDBG_OBJECT_ID(bogg_player);

/* 256 bytes of filling */
static const uint8_t bogg_player_stuffing[256] = {0};

struct bogg_player_saved_packet {
    size_t length;
    void *buffer;
};

struct bogg_player {
    BDBG_OBJECT(bogg_player)
    off_t offset;
    size_t  length;
    enum {bmedia_player_ogg_mode_file, bmedia_player_ogg_mode_header} mode;
    bfile_io_read_t fd;
    bfile_buffer_t index_buffer;  /* buffer that is used to access OGG file */
    batom_factory_t factory;
    off_t first, last; /* offset of the file */
    bmedia_player_status status;
    bmedia_player_config config;
    bmedia_player_stream stream;
    bmedia_player_step direction;
    bmedia_time_scale time_scale;
    bfile_segment init_page;
    struct {
        baudio_format audio_codec;
        union {
            struct {
                bvorbis_frame_header frame_header;
                bvorbis_indentification_header indentification_header;
                struct {
                    uint8_t buffer[16384];
                    size_t length;
                } spanning;
                struct bogg_player_saved_packet type3, type5;
            } vorbis;
            struct {
                bopus_indentification_header indentification_header;
            } opus;
        } codec_data;
    } track;
    uint8_t index_buffer_data[BIO_BLOCK_SIZE*7 /* 28672 */ ];
};

static int
b_ogg_player_read_page(bogg_player_t player, bogg_file_header_state *header_parser, const bfile_segment *range, bfile_segment *page,bool (*handler)(bogg_player_t , const bogg_page_header *, const batom_cursor *, unsigned , bool, size_t ))
{
    int rc;


    rc = bogg_file_header_read(header_parser, player->factory, player->index_buffer, range);
    if(rc<0) {goto err_header;}

    for(;;) {
        if(handler && !handler(player, &header_parser->header, &header_parser->segment.cursor, header_parser->parser.payload-1, header_parser->spanning, header_parser->payload_size)) {
            goto err_no_data;
        }
        if(batom_cursor_skip(&header_parser->segment.cursor, header_parser->payload_size)!=(size_t)header_parser->payload_size) {
            goto err_no_data;
        }
        if(header_parser->parser.payload >= header_parser->header.page_segments) {
            break;
        }
        if(!bfile_cached_segment_reserve(&header_parser->segment, 16)) {
            goto err_no_data;
        }
        header_parser->payload_size = bogg_page_payload_parser_next(&header_parser->parser, &header_parser->header, &header_parser->spanning);
        if(header_parser->payload_size<=0) { goto err_no_data;}

        if(!bfile_cached_segment_reserve(&header_parser->segment, header_parser->payload_size)) { goto err_no_data; }
    }
    bfile_segment_set(page, range->start, bfile_cached_segment_tell(&header_parser->segment));

    bogg_file_header_shutdown(header_parser);
    return 0;

err_no_data:
    bogg_file_header_shutdown(header_parser);
err_header:
    return -1;
}

static bool
b_ogg_player_parser_first_page(bogg_player_t player, const bogg_page_header *page_header, const batom_cursor *payload, unsigned packet_no, bool spanning, size_t payload_size)
{
    batom_cursor cursor;

    BSTD_UNUSED(payload_size);

    if(page_header->header_type!=BOGG_HEADER_TYPE_FIRST_PAGE || spanning || packet_no!=0) {
        return false;
    }
    BATOM_CLONE(&cursor, payload);
    if(bvorbis_parse_frame_header(&cursor, &player->track.codec_data.vorbis.frame_header) && player->track.codec_data.vorbis.frame_header.packet_type==BVORBIS_IDENTIFICATION_HEADER) {
        if(!bvorbis_parse_indentification_header(&cursor, &player->track.codec_data.vorbis.indentification_header)) {goto err_stream_no_data;}
        player->track.audio_codec = baudio_format_vorbis;
        player->track.codec_data.vorbis.type3.length = 0;
        player->track.codec_data.vorbis.type5.length = 0;
        player->track.codec_data.vorbis.spanning.length = 0;
        return true;
    }
    BATOM_CLONE(&cursor, payload);
    if(bopus_parse_indentification_header(&cursor, &player->track.codec_data.opus.indentification_header)) {
        player->track.audio_codec = baudio_format_opus;
        return true;
    }

err_stream_no_data:
    return false;
}

static int b_ogg_player_copy_packet( struct bogg_player_saved_packet *packet, batom_cursor *cursor, size_t payload_size)
{
    packet->buffer = BKNI_Malloc(payload_size);
    BDBG_MSG(("b_ogg_player_copy_packet: %p %u bytes -> %p", (void *)packet, (unsigned)payload_size, packet->buffer));
    if(packet->buffer==NULL) {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return -1;
    }
    packet->length = payload_size;
    batom_cursor_copy(cursor, packet->buffer, payload_size);
    return  0;
}

static void b_ogg_player_free_packet( struct bogg_player_saved_packet *packet)
{
    if(packet->buffer) {
        BKNI_Free(packet->buffer);
        packet->buffer = NULL;
        packet->length = 0;
    }
    return;
}

static void b_ogg_player_free_codec_data(bogg_player_t player)
{
    switch(player->track.audio_codec) {
    case baudio_format_vorbis:
        b_ogg_player_free_packet(&player->track.codec_data.vorbis.type3);
        b_ogg_player_free_packet(&player->track.codec_data.vorbis.type5);
        break;
    default:
        break;
    }
    return;
}

static bool
b_ogg_player_parser_vorbis_meta(bogg_player_t player, const bogg_page_header *page_header, const batom_cursor *payload, unsigned packet_no, bool spanning, size_t payload_size)
{
    batom_cursor cursor;
    batom_cursor packet;
    bvorbis_frame_header vorbis_frame_header;
    batom_vec spanning_vec;
    int rc;

    BSTD_UNUSED(page_header);
    BSTD_UNUSED(packet_no);
    if(player->track.codec_data.vorbis.type3.length && player->track.codec_data.vorbis.type5.length) {
        return true;
    }

    if(spanning || player->track.codec_data.vorbis.spanning.length ) { /* copy payload to the buffer */
        if(player->track.codec_data.vorbis.spanning.length + payload_size > sizeof(player->track.codec_data.vorbis.spanning.buffer)) {
            BDBG_ERR(("%p: overflow of spanning buffer %u/%u", (void *)player, (unsigned)(player->track.codec_data.vorbis.spanning.length + payload_size), (unsigned)sizeof(player->track.codec_data.vorbis.spanning.buffer)));
            return false;
        }
        BATOM_CLONE(&cursor, payload);
        batom_cursor_copy(&cursor, player->track.codec_data.vorbis.spanning.buffer + player->track.codec_data.vorbis.spanning.length, payload_size);
        player->track.codec_data.vorbis.spanning.length += payload_size;
        if(spanning) {
            return true;
        }
        BATOM_VEC_INIT(&spanning_vec, player->track.codec_data.vorbis.spanning.buffer, player->track.codec_data.vorbis.spanning.length);
        payload_size = player->track.codec_data.vorbis.spanning.length ;
        player->track.codec_data.vorbis.spanning.length = 0;
        batom_cursor_from_vec(&cursor, &spanning_vec, 1);
    } else {
        BATOM_CLONE(&cursor, payload);
    }
    BATOM_CLONE(&packet, &cursor);

    if(!bvorbis_parse_frame_header(&cursor, &vorbis_frame_header)) {
        goto stream_error;
    }

    switch(vorbis_frame_header.packet_type) {
    case 3:
        rc = b_ogg_player_copy_packet(&player->track.codec_data.vorbis.type3, &packet, payload_size);
        if(rc!=0) { return rc; }
        break;
    case 5:
        rc = b_ogg_player_copy_packet(&player->track.codec_data.vorbis.type5, &packet, payload_size);
        if(rc!=0) { return rc; }
        break;
    default:
        break;
    }
    return true;

stream_error:
    return false;
}

static int
b_ogg_player_open_file(bogg_player_t player)
{
    bogg_file_header_state header_parser;
    bfile_segment segment;
    int rc;

    player->track.audio_codec = baudio_format_unknown;
    bfile_segment_set(&segment, 0, 1*1024*1024);
    rc = b_ogg_player_read_page(player, &header_parser, &segment, &player->init_page, b_ogg_player_parser_first_page);
    if(rc<0) {goto err_page;}
    if(header_parser.header.header_type!=BOGG_HEADER_TYPE_FIRST_PAGE || header_parser.spanning) { goto err_page;}
    switch(player->track.audio_codec) {
    case baudio_format_opus:
    case baudio_format_unknown:
    default:
        break;
    case baudio_format_vorbis:
        {
            unsigned i;
            bool found=false;
            for(i=0;i<3;i++) {
                segment.start += player->init_page.len;
                rc = b_ogg_player_read_page(player, &header_parser, &segment, &player->init_page, b_ogg_player_parser_vorbis_meta);
                if(rc<0) {goto err_page;}
                if(player->track.codec_data.vorbis.type3.length && player->track.codec_data.vorbis.type5.length) {
                    found = true;
                    break;
                }
            }
            if(!found) {
                goto err_page;
            }
            player->init_page.len += segment.start;
            player->init_page.start = 0;
            break;
        }
    }
    BDBG_MSG(("b_ogg_player_open_file: %p init_page: %u bytes at %u ", (void *)player, (unsigned)player->init_page.len, (unsigned)player->init_page.start));

    if(player->fd->bounds(player->fd, &player->first, &player->last)<0) {
        player->first = 0;
        player->last = ((uint64_t) 1)<<62; /* some very large number */
    }
    player->offset = player->first;
    player->length = (188/4) * BIO_BLOCK_SIZE;
    player->mode = bmedia_player_ogg_mode_file;

    return 0;

err_page:
    b_ogg_player_free_codec_data(player);
    return rc;
}

static bogg_player_t
bogg_player_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream)
{
    int rc;
    bogg_player_t  player;
    bfile_buffer_cfg buffer_cfg;

    BSTD_UNUSED(rc);
    BDBG_ASSERT(fd);
    BDBG_ASSERT(config);
    BDBG_ASSERT(stream);

    player = BKNI_Malloc(sizeof(*player));
    if(!player) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc;
    }
    BDBG_MSG(("bogg_player_create:%p", (void *)player));
    BDBG_OBJECT_INIT(player, bogg_player);
    player->config = *config;
    player->stream = *stream;
    player->fd = fd;
    bmedia_player_init_status(&player->status);
    player->factory = batom_factory_create(bkni_alloc, 64);
    if(!player->factory) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_factory;
    }
    bfile_buffer_default_cfg(&buffer_cfg);
    buffer_cfg.buf = player->index_buffer_data;
    buffer_cfg.buf_len = sizeof(player->index_buffer_data);
    buffer_cfg.nsegs = sizeof(player->index_buffer_data)/BIO_BLOCK_SIZE;
    buffer_cfg.fd = fd;
    player->index_buffer = bfile_buffer_create(player->factory, &buffer_cfg);
    if(!player->index_buffer) {
        goto err_index_buffer;
    }
    rc =  b_ogg_player_open_file(player);
    if(rc<0) {goto err_open_file;}

    return player;

err_open_file:
    bfile_buffer_destroy(player->index_buffer);
err_index_buffer:
    batom_factory_destroy(player->factory);
err_factory:
    BKNI_Free(player);
err_alloc:
    return NULL;
}

static void
bogg_player_destroy(bogg_player_t player)
{
    b_ogg_player_free_codec_data(player);
    bfile_buffer_destroy(player->index_buffer);
    batom_factory_destroy(player->factory);
    BDBG_OBJECT_DESTROY(player, bogg_player);
    BKNI_Free(player);
    return;
}

static int
bogg_player_next(bogg_player_t player, bmedia_player_entry *entry)
{
    BDBG_OBJECT_ASSERT(player, bogg_player);
    bmedia_player_init_entry(entry);

    switch(player->mode) {
    case bmedia_player_ogg_mode_header:
        entry->start = player->init_page.start;
        entry->length = (size_t)player->init_page.len;
        entry->type = bmedia_player_entry_type_file;
        player->mode = bmedia_player_ogg_mode_file;
        break;
    case bmedia_player_ogg_mode_file:
        entry->start = player->offset;
        entry->type = bmedia_player_entry_type_file;
        entry->embedded = NULL;
        if((off_t)(player->offset + player->length) <= player->last) {
            entry->length = player->length;
            player->offset += player->length;
            break;
        }
        if(player->offset < player->last) { /* sending last packet */
            entry->length = player->last - player->offset;
            entry->length -= entry->length%BIO_BLOCK_SIZE; /* ensure that block is aligned */
            player->offset += entry->length;
            if(entry->length<=0) {
                entry->type = bmedia_player_entry_type_end_of_stream;
            }
        }
        break;
    default:
        break;
    }
    BDBG_MSG(("bogg_player_next: %p type:%u at %u(%u)", (void *)player, entry->type, (unsigned)entry->start, (unsigned)entry->length));
    return 0;
}

static void
bogg_player_tell(bogg_player_t player, bmedia_player_pos *pos)
{
    BDBG_OBJECT_ASSERT(player, bogg_player);
    *pos = (bmedia_player_pos)((BMEDIA_PLAYER_POS_SCALE*8*(uint64_t)player->offset)/player->stream.stream.noindex.bitrate);
    return;
}

static void
bogg_player_set_direction(bogg_player_t player, bmedia_player_step direction, bmedia_time_scale time_scale, bmedia_player_decoder_mode *mode)
{
    BDBG_OBJECT_ASSERT(player, bogg_player);
    BSTD_UNUSED(mode);
    player->direction = direction;
    player->time_scale = time_scale;
    return;
}

static int
bogg_player_seek(bogg_player_t player, bmedia_player_pos pos)
{
    BDBG_OBJECT_ASSERT(player, bogg_player);
    player->offset = (off_t)((pos*(uint64_t)player->stream.stream.noindex.bitrate)/(BMEDIA_PLAYER_POS_SCALE*8));
    player->offset -= player->offset%BIO_BLOCK_SIZE; /* keep offset alligned */
    if(player->offset > player->init_page.start) {
        player->mode = bmedia_player_ogg_mode_header;
    } else {
        player->mode = bmedia_player_ogg_mode_file;
        player->offset = 0;
    }
    BDBG_MSG(("bogg_player_seek:%#lx pos %u bitrate %u offset %u", (unsigned long)player, (unsigned)pos, (unsigned)player->stream.stream.noindex.bitrate, (unsigned)player->offset));
    return 0;
}

static void *
b_ogg_player_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream)
{
    return bogg_player_create(fd, config, stream);
}

static void
b_ogg_player_destroy(void *player)
{
    bogg_player_destroy(player);
    return;
}

static int
b_ogg_player_next(void *player, bmedia_player_entry *entry)
{
    return bogg_player_next(player, entry);
}

static void
b_ogg_player_tell(void *player, bmedia_player_pos *pos)
{
    bogg_player_tell(player, pos);
    return;
}

static void
b_ogg_player_get_status(void *player_, bmedia_player_status *status)
{
    bogg_player_t player = player_;
    BDBG_OBJECT_ASSERT(player, bogg_player);

    *status = player->status;
    return;
}

static int
b_ogg_player_set_direction(void *player, bmedia_player_step direction, bmedia_time_scale time_scale, bmedia_player_decoder_mode *mode)
{
    bogg_player_set_direction(player, direction, time_scale, mode);
    return 0;
}

static int
b_ogg_player_seek(void *player, bmedia_player_pos pos)
{
    return bogg_player_seek(player, pos);
}

const bmedia_player_methods bogg_player_methods = {
    b_ogg_player_create,
    b_ogg_player_destroy,
    b_ogg_player_next,
    b_ogg_player_tell,
    b_ogg_player_get_status,
    b_ogg_player_set_direction,
    b_ogg_player_seek
};
