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
#include "bmedia_player_es.h"
#include "bkni.h"
#include "bmpeg_audio_probe.h"
#include "bmpeg_audio_util.h"
#include "biobits.h"
#include "bid3_parser.h"
#include "bmedia_index.h"
#include "baac_adts_probe.h"
#include "bac3_probe.h"

BDBG_MODULE(bmedia_player_es);

#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */

#define BMEDIA_PLAYER_ES_BLOCK (15*BIO_BLOCK_SIZE)
/* when advancing through the file don't read more then said amount, instead  'blindly' jump to a file */
#define BMEDIA_PLAYER_ES_BLOCK_THRESHOLD  (128*1024)
#define BMEDIA_PLAYER_ES_SYNC_BLOCK (64*1024)


typedef struct bmedia_player_es *bmedia_player_es_t;

typedef struct b_media_es_entry {
    off_t   offset; /* position in the file */
} b_media_es_entry;

BDBG_OBJECT_ID(bmedia_player_es);

/* 256 bytes of filling */
static const uint8_t bmedia_player_es_stuffing[256] = {0};
struct bmedia_player_es;


typedef struct b_media_player_es_frame_info {
    bmedia_probe_audio info;
    unsigned samples;
    union {
        struct {
            bmedia_mpeg_audio_info mpeg_info;
            bmp3_vbr_frame_info vbr_info;
        } mpeg;
    } codec;
} b_media_player_es_frame_info;


typedef struct b_audio_player_es_codec_info {
    unsigned sync_len;
    bool (*test_sync)(uint32_t sync_word);
    size_t (*parse_header)(batom_cursor *cursor, uint32_t sync_word, b_media_player_es_frame_info *frame_info, bool first_frame);
    void (*calibrate)(struct bmedia_player_es *player, const b_media_player_es_frame_info *frame_info);
} b_audio_player_es_codec_info;


struct bmedia_player_es {
    BDBG_OBJECT(bmedia_player_es)
    batom_accum_t accum_src;
    batom_accum_t accum_pes;
    batom_factory_t factory;
    bmedia_player_config config;
    bmedia_player_stream stream;
    bmedia_time_scale time_scale;
    off_t offset;
    batom_cursor cursor;
    bmedia_player_pos index_wait_time;
    bmedia_index_t index;
    bmedia_player_status status;
    struct {
        bool vbr;
        unsigned bitrate;
        unsigned duration;
        unsigned frame_duration; /* frame duration in us */
        off_t first,last;
    } file;
    struct {
        batom_t atom;
        unsigned duration;
        unsigned time_units;
        b_media_es_entry index_entry;
    } frame;
    struct {
        bmedia_player_pos last;
        bmedia_player_pos base;
        uint64_t time;
        unsigned time_units;
    } position;
    struct {
        off_t off_first, off_last;
        bmedia_player_pos pos_first, pos_last;
    } auto_bitrate;
    bfile_buffer_t index_buffer;  /* buffer that is used to access an index file */
    const b_audio_player_es_codec_info *codec;
    uint8_t pes_header[BMEDIA_PES_HEADER_MAX_SIZE];
    uint8_t index_buffer_data[BIO_BLOCK_SIZE*17 /* 69632 */];
};

static void
b_media_player_es_reset_position(bmedia_player_es_t player, bmedia_player_pos pos, off_t offset)
{
    player->position.base = pos;
    player->position.last = pos;
    player->position.time_units = 0;
    player->position.time = 0;

    player->auto_bitrate.off_first=-1;
    player->auto_bitrate.off_last =-1;
    player->auto_bitrate.pos_first = 0;
    player->auto_bitrate.pos_last = 0;

    player->offset = offset;
    player->frame.index_entry.offset = player->offset;
    batom_accum_clear(player->accum_src);
    batom_cursor_from_accum(&player->cursor, player->accum_src);

    return;
}

static unsigned 
b_media_player_get_auto_bitrate(bmedia_player_es_t player)
{
    off_t bytes = player->auto_bitrate.off_last - player->auto_bitrate.off_first;
    bmedia_player_pos time = player->auto_bitrate.pos_last - player->auto_bitrate.pos_first;
    unsigned bitrate = 0;

    if(time>0) {
        bitrate = (bytes * 8 * BMEDIA_PLAYER_POS_SCALE) / time;
    }
    if(bitrate<=0) {
        bitrate = 1024;
    }
    return bitrate;
}

static BERR_Code
b_media_player_es_mp3_probe(bmedia_player_es_t player)
{
    batom_t atom;
    bfile_buffer_result result;
    batom_cursor cursor;
    bid3v2_header header;

    atom = bfile_buffer_read(player->config.buffer, 0, 128, &result);
    if(atom==NULL) {
        goto error;
    }
    batom_cursor_from_atom(&cursor, atom);
    if(!bid3v2_parser_probe(&cursor)) {
        goto done;
    }
    batom_cursor_from_atom(&cursor, atom);
    if(!bid3v2_parse_header(&cursor, &header)) {
        goto done;
    }
    BDBG_MSG(("b_media_player_es_mp3_probe:%#lx ID3V2 starting from %u", (unsigned long)player, (unsigned)header.size));
    player->file.first = header.size;
done:
    batom_release(atom);
    return BERR_SUCCESS;
error:
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

static BERR_Code
b_media_player_es_aac_probe(bmedia_player_es_t player)
{
    batom_t atom;
    bfile_buffer_result result;
    batom_cursor cursor;
    batom_cursor start;
    uint32_t temp = 0;
    int byte;
    
    atom = bfile_buffer_read(player->config.buffer, 0, 128, &result);
    if(atom==NULL) {
        goto error;
    }
    batom_cursor_from_atom(&cursor, atom);
    batom_cursor_clone(&start, &cursor);
    
    for(;;) {        
        byte = batom_cursor_next(&cursor);
        if(byte==BATOM_EOF) {
            goto done;
        }    
        temp = ((temp&0xFF)<<8)|byte;
        if(B_GET_BITS(temp,15, 4)==B_AAC_ADTS_SYNC) {
            break;
        }
    }
    player->file.first = batom_cursor_distance(&start, &cursor) - B_AAC_ADTS_SYNC_LEN;

done:
    batom_release(atom);
    return BERR_SUCCESS;
error:
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

static BERR_Code
b_media_player_es_ac3_probe(bmedia_player_es_t player)
{
    batom_t atom;
    bfile_buffer_result result;
    batom_cursor cursor;
    batom_cursor start;
    uint32_t temp = 0;
    int byte;

    atom = bfile_buffer_read(player->config.buffer, 0, 128, &result);
    if(atom==NULL) {
        goto error;
    }
    batom_cursor_from_atom(&cursor, atom);
    batom_cursor_clone(&start, &cursor);

    for(;;) {
        byte = batom_cursor_next(&cursor);
        if(byte==BATOM_EOF) {
            goto done;
        }
        temp = ((temp&0xFF)<<8)|byte;
        if(temp==B_AC3_SYNC) {
            break;
        }
    }
    player->file.first = batom_cursor_distance(&start, &cursor) - B_AC3_SYNC_LEN;

done:
    batom_release(atom);
    return BERR_SUCCESS;
error:
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}
static void 
bmedia_player_es_destroy(bmedia_player_es_t player)
{
    BDBG_OBJECT_ASSERT(player, bmedia_player_es);
    batom_accum_destroy(player->accum_src);
    batom_accum_destroy(player->accum_pes);
    bmedia_index_destroy(player->index);
    bfile_buffer_destroy(player->index_buffer);
    BDBG_OBJECT_DESTROY(player, bmedia_player_es);
    BKNI_Free(player);
    return;
}

static void
b_media_player_es_trim(bmedia_player_es_t player, size_t keep)
{
    size_t pos = batom_cursor_pos(&player->cursor);
    batom_accum_t acc = player->accum_src;

    if(pos>keep) {
        size_t advance = pos - keep;
        player->offset += advance;
        batom_cursor_from_accum(&player->cursor, acc);
        batom_cursor_skip(&player->cursor, advance);
        batom_accum_trim(acc, &player->cursor);
    }
    batom_cursor_from_accum(&player->cursor, acc);
    return;
}

static bool
b_media_player_es_load(bmedia_player_es_t player, size_t reserve, bfile_buffer_t buffer, bfile_buffer_result *buffer_result)
{
    batom_accum_t acc = player->accum_src;

    if(buffer_result) {
        *buffer_result = bfile_buffer_result_ok;
    }
    for(;;) {
        size_t acc_len;
        size_t cursor_left = batom_cursor_reserve(&player->cursor, reserve);
        bfile_buffer_result result;
        batom_t atom;
        size_t read_len = BMEDIA_PLAYER_ES_BLOCK;
        off_t offset;
        size_t cursor_pos;

        if(reserve <= cursor_left) { break;}
        cursor_pos = batom_cursor_pos(&player->cursor);
        acc_len = batom_accum_len(acc);

        offset = player->offset+acc_len;
        BDBG_MSG_TRACE(("b_media_player_es_load: %#lx loading %u:%u", (unsigned long)player, (unsigned)(player->offset+acc_len), (unsigned)read_len));
        if(player->file.last>0 && offset+(int)read_len >= player->file.last) {
            if(offset>=player->file.last) {
                goto error;
            }
            read_len = player->file.last - offset;
        }

        atom = bfile_buffer_read(buffer, player->offset+acc_len, read_len, &result);
        if(buffer_result) {
            *buffer_result = result;
        }
        if(!atom) {
            goto error;
        }
        batom_accum_add_atom(acc, atom);
        batom_release(atom);
        batom_cursor_from_accum(&player->cursor, acc);
        batom_cursor_skip(&player->cursor, cursor_pos);
    }
    return true;
error:
    return false;
}

static int
b_media_player_es_audio_resync(bmedia_player_es_t player, bfile_buffer_result *buffer_result)
{
    uint32_t sync = 0;
    unsigned frame_syncs=0;
    unsigned offset=0;
    unsigned first_sync_offset=0;
    batom_accum_t acc = player->accum_src;
    unsigned sample_rate = 0;
    unsigned restarts=0;

    BDBG_MSG_TRACE(("b_media_player_es_audio_resync: %#lx at %u", (unsigned long)player, (unsigned)player->offset));
    batom_cursor_from_accum(&player->cursor, acc);
    if(!b_media_player_es_load(player, BMEDIA_PLAYER_ES_SYNC_BLOCK , player->index_buffer, buffer_result)) {
        return -1;
    }
    for(;;) {
        int byte = batom_cursor_next(&player->cursor);
        if(byte==BATOM_EOF) {
            if(frame_syncs==0) {
                break; /* no syncs found */
            }
            goto restart;
        }
        sync = ((sync&0xFF)<<8)|byte;
        if(frame_syncs==0) {
            first_sync_offset = batom_cursor_pos(&player->cursor)-player->codec->sync_len;
        }
        if(player->codec->test_sync(sync)) {
            size_t frame_len;
            b_media_player_es_frame_info frame_info;

            BKNI_Memset(&frame_info, 0, sizeof(frame_info));

            frame_len = player->codec->parse_header(&player->cursor, sync, &frame_info, false);
            BDBG_MSG_TRACE(("b_media_player_es_audio_resync: %#lx found frame %u(%u):%u at %u:%u", (unsigned long)player, (unsigned)frame_info.info.sample_rate,  sample_rate, (unsigned)frame_len, batom_cursor_pos(&player->cursor), player->offset+batom_cursor_pos(&player->cursor))); 
            if(frame_syncs==0) {
                sample_rate = frame_info.info.sample_rate;
            }
            if(frame_len==0 || frame_info.info.sample_rate==0 || frame_info.info.sample_rate != sample_rate ) {
                goto restart;
            }
            sync=0;
            frame_syncs++;
            BDBG_MSG_TRACE(("b_media_player_es_audio_resync: %#lx true sync %u at %u", (unsigned long)player, frame_syncs, (unsigned)batom_cursor_pos(&player->cursor)));
            if(frame_syncs>8) {
                break;
            }
            batom_cursor_skip(&player->cursor, frame_len);
        }
        continue;
restart:
        offset = first_sync_offset+1;
        frame_syncs = 0;
        sync = 0;
        first_sync_offset = offset;
        restarts++;
        if(restarts>64) {
            break;
        }
        batom_cursor_from_accum(&player->cursor, acc);
        batom_cursor_skip(&player->cursor, offset);
        BDBG_MSG_TRACE(("b_media_player_es_audio_resync: %#lx restarting at %u:%u", (unsigned long)player, offset, player->offset+offset));
    }
    return frame_syncs>0?(int)first_sync_offset:-1;
}


static bool b_audio_player_es_test_mpeg_sync(uint32_t sync_word)
{
    return B_GET_BITS(sync_word, 15,5)==B_MPEG_AUDIO_SYNC;
}

static size_t
b_audio_player_es_parse_mpeg_header(batom_cursor *cursor, uint32_t sync_word, b_media_player_es_frame_info *frame_info, bool first_frame)
{
    size_t frame_len;

    frame_len = bmpeg_audio_probe_parse_header(cursor, sync_word, &frame_info->info, &frame_info->codec.mpeg.mpeg_info );
    if(frame_len>0) {
        frame_info->samples = frame_info->codec.mpeg.mpeg_info.samples;
        if (first_frame) {
            batom_cursor vbr_cursor;
            BATOM_CLONE(&vbr_cursor, cursor);
            bmp3_parse_vbr_frame_info(&vbr_cursor, &frame_info->codec.mpeg.mpeg_info, &frame_info->codec.mpeg.vbr_info);
        }
    }
    return frame_len;
}

static void
b_audio_player_es_calibrate_mpeg(bmedia_player_es_t player, const b_media_player_es_frame_info *frame_info)
{
    if(frame_info->codec.mpeg.vbr_info.stream_type != bmp3_stream_type_unknown) {
        if(frame_info->info.sample_rate>0) {
            player->file.vbr = frame_info->codec.mpeg.vbr_info.stream_type!=bmp3_stream_type_cbr;
            player->status.bounds.last = ((BMEDIA_PLAYER_POS_SCALE * frame_info->codec.mpeg.mpeg_info.samples * (uint64_t)frame_info->codec.mpeg.vbr_info.num_frames) / frame_info->info.sample_rate);
            if (player->status.bounds.last) {
                player->file.bitrate = ((BMEDIA_PLAYER_POS_SCALE*8)*(uint64_t)frame_info->codec.mpeg.vbr_info.stream_bytes)/player->status.bounds.last;
            }
        }
    }
}

static const b_audio_player_es_codec_info b_audio_player_es_mpeg_info = {
    B_MPEG_AUDIO_SYNC_LEN,
    b_audio_player_es_test_mpeg_sync,
    b_audio_player_es_parse_mpeg_header,
    b_audio_player_es_calibrate_mpeg
};

static bool b_audio_player_es_test_aac_sync(uint32_t sync_word)
{
    return (B_GET_BITS(sync_word, 15,4)==B_AAC_ADTS_SYNC);
}

static size_t
b_audio_player_es_parse_aac_header(batom_cursor *cursor, uint32_t sync_word, b_media_player_es_frame_info *frame_info, bool first_frame)
{
    BSTD_UNUSED(first_frame);
    frame_info->samples = 1024;
    return b_aac_adts_probe_parse_header(cursor, &frame_info->info, sync_word );
}

static void
b_audio_player_es_calibrate_none(bmedia_player_es_t player, const b_media_player_es_frame_info *frame_info)
{
    BSTD_UNUSED(player);
    BSTD_UNUSED(frame_info);
    return;
}

static const b_audio_player_es_codec_info b_audio_player_es_aac_info = {
    B_AAC_ADTS_SYNC_LEN,
    b_audio_player_es_test_aac_sync,
    b_audio_player_es_parse_aac_header,
    b_audio_player_es_calibrate_none
};


static bool b_audio_player_es_test_ac3_sync(uint32_t sync_word)
{
    return sync_word == B_AC3_SYNC;
}

static size_t
b_audio_player_es_parse_ac3_header(batom_cursor *cursor, uint32_t sync_word, b_media_player_es_frame_info *frame_info, bool first_frame)
{
    unsigned nblocks;
    size_t frame_size;
    BSTD_UNUSED(first_frame);
    BSTD_UNUSED(sync_word);
    frame_size = b_ac3_probe_parse_header(cursor, &frame_info->info, &nblocks);
    frame_info->samples = nblocks*256;
    return frame_size;
}

static const b_audio_player_es_codec_info b_audio_player_es_ac3_info = {
    B_AC3_SYNC_LEN,
    b_audio_player_es_test_ac3_sync,
    b_audio_player_es_parse_ac3_header,
    b_audio_player_es_calibrate_none
};


static bool
b_media_player_es_load_audio_frame(bmedia_player_es_t player, b_media_player_es_frame_info *frame_info, bool first_frame, bfile_buffer_result *buffer_result)
{
    uint16_t sync = 0;
    batom_accum_t acc = player->accum_src;

    BDBG_MSG_TRACE(("b_media_player_es_load_audio_frame: %#lx loading frame at:%u", (unsigned long)player, (unsigned)player->offset));
    BDBG_ASSERT(frame_info);
    /* vbr_info is optional */

    if(buffer_result) {
        *buffer_result = bfile_buffer_result_ok;
    }

    batom_cursor_from_accum(&player->cursor, acc);
    for(;;) {
        int byte = batom_cursor_next(&player->cursor);
        bool frame_completed;

        if(byte!=BATOM_EOF) {
            sync = ((sync&0xFF)<<8)|byte;
            if(player->codec->test_sync(sync)) {
                size_t frame_len;
                unsigned sync_len = player->codec->sync_len;

                b_media_player_es_trim(player, sync_len);
                batom_cursor_skip(&player->cursor, sync_len);
                if(buffer_result==NULL) { /* read shouldn't fail and we are using index */
                    frame_completed = b_media_player_es_load(player, 16, player->index_buffer, NULL);
                } else { /* reading frame as data, it could fail (as return no data) */
                    frame_completed = b_media_player_es_load(player, 16, player->config.buffer, buffer_result);
                }
                if(!frame_completed) {
                    return false;
                }
                frame_len = player->codec->parse_header(&player->cursor, sync, frame_info, first_frame);
                if(frame_len>0) {
                    batom_cursor last;

                    first_frame = false;
                    /* BDBG_MSG_TRACE(("b_media_player_es_load_audio_frame: %#lx found frame %u:%u:%u at %u", (unsigned long)player, (unsigned)info->sample_rate, (unsigned)info->bitrate, (unsigned)frame_len, (unsigned)player->offset)); */
                    if(buffer_result==NULL) { /* read shouldn't fail and we are using index */
                        frame_completed = b_media_player_es_load(player, frame_len+sync_len, player->index_buffer, NULL);
                    } else { /* reading frame as data, it could fail (as return no data) */
                        frame_completed = b_media_player_es_load(player, frame_len+sync_len, player->config.buffer, buffer_result);
                        if(!frame_completed && *buffer_result==bfile_buffer_result_no_data) {
                            batom_cursor_from_accum(&player->cursor, acc); /* reset cursor */
                            return false;
                        }
                    }
                    if(frame_completed) {
                        batom_cursor_skip(&player->cursor, frame_len);
                        BATOM_CLONE(&last, &player->cursor);
                        sync = batom_cursor_vword_be(&player->cursor, sync_len);
                        BDBG_ASSERT(!BATOM_IS_EOF(&player->cursor));
                        if(!player->codec->test_sync(sync)) {
                            BDBG_WRN(("b_media_player_es_load_audio_frame: %#lx invalid frame at %u", (unsigned long)player, (unsigned)player->offset));
                            goto resync;
                        }
                    } else { /* just send all data that is left */
                        batom_cursor_skip(&player->cursor, frame_len);
                        BATOM_CLONE(&last, &player->cursor);
                    }
                    batom_cursor_from_accum(&player->cursor, acc);
                    player->frame.atom = batom_accum_extract(acc, &player->cursor, &last, NULL, NULL);
                    if(player->frame.atom) {
                        player->frame.time_units = frame_info->info.sample_rate;
                        player->frame.index_entry.offset = player->offset;
                        player->frame.duration = frame_info->samples;
                        BDBG_MSG_TRACE(("b_media_player_es_load_audio_frame: %#lx extracted frame %u:%u:%u(%u:%u) at %u", (unsigned long)player, (unsigned)frame_info->info.sample_rate, (unsigned)frame_info->info.bitrate, (unsigned)batom_len(player->frame.atom), (unsigned)(frame_len+sync_len), (unsigned)player->frame.duration, (unsigned)player->offset));
                        BATOM_CLONE(&player->cursor, &last);
                        b_media_player_es_trim(player, 0); 
                        return true;
                    } else {
                        return false;
                    }
                } 
                BDBG_MSG_TRACE(("b_media_player_es_load_audio_frame: %#lx invalid sync at %u", (unsigned long)player, (unsigned)player->offset));
resync:
                b_media_player_es_trim(player, batom_cursor_pos(&player->cursor)-1); /* invalid frame  resync */
                sync = 0;
            }
        } else {
            b_media_player_es_trim(player, 1);
            if(buffer_result==NULL) { /* read shouldn't fail and we are using index */
                frame_completed = b_media_player_es_load(player, B_MPEG_AUDIO_SYNC_LEN, player->index_buffer, NULL);
            } else { /* reading frame as data, it could fail (as return no data) */
                frame_completed = b_media_player_es_load(player, B_MPEG_AUDIO_SYNC_LEN, player->config.buffer, buffer_result);
            }
            if(!frame_completed) {
                if(buffer_result) {
                    if(*buffer_result == bfile_buffer_result_ok) {
                        *buffer_result = bfile_buffer_result_eof;
                    }
                }
                BDBG_MSG_TRACE(("b_media_player_es_load_audio_frame: %#lx end of stream at %u", (unsigned long)player, (unsigned)player->offset));
                return false;
            }
            sync = 0;
        }
    }
    /* not reached */
}

static void
b_media_player_es_index_point(bmedia_player_es_t player, bmedia_player_pos time)
{
    if(player->index_wait_time != BMEDIA_PLAYER_INVALID && time >= player->index_wait_time) {
        BDBG_MSG(("b_media_player_es_index_point: %#lx saving index (%u:%u) off:%u", (unsigned long)player, (unsigned)time, (unsigned)player->index_wait_time, (unsigned)player->frame.index_entry.offset));
        bmedia_index_add(player->index, time, &player->frame.index_entry);
        if(time>0 && player->frame.index_entry.offset>player->file.first) {
            player->file.bitrate = ((BMEDIA_PLAYER_POS_SCALE*8)*(player->frame.index_entry.offset - player->file.first))/time;
        }
        player->index_wait_time = bmedia_index_next(player->index);
    }
    return;
}



static bool
b_media_player_es_load_next_frame(bmedia_player_es_t player, bfile_buffer_result *buffer_result, bool add_index)
{
    b_media_player_es_frame_info info;

    player->frame.atom = NULL;
    player->frame.time_units = 0;
    player->frame.duration = 0;
    player->frame.index_entry.offset = player->offset;
    if(!b_media_player_es_load_audio_frame(player, &info, false, buffer_result) ) {
        return false;
    }
    if(player->frame.atom==NULL) {
        return false;
    }
    if(player->frame.time_units!=player->position.time_units) {
        player->position.time_units = player->frame.time_units;
        player->position.base = player->position.last;
        player->position.time = 0;
    }
    if(player->auto_bitrate.off_first==-1) {
        player->auto_bitrate.off_first = player->frame.index_entry.offset;
        player->auto_bitrate.pos_first = player->position.last;
    }
    if(player->position.time_units!=0) {
        player->position.time += player->frame.duration;
        player->position.last = player->position.base + (player->position.time * BMEDIA_PLAYER_POS_SCALE)/player->position.time_units;
    } 
    player->auto_bitrate.off_last = player->offset;
    player->auto_bitrate.pos_last = player->position.last;
    player->status.position = player->position.last;
    if(add_index) {
        b_media_player_es_index_point(player, player->position.last);
    }
    BDBG_MSG_TRACE(("b_media_player_es_load_next: %#lx time:%u", (unsigned long)player, (unsigned)player->position.last));

    return true;
}

static batom_t
b_media_player_es_make_pes_packet(bmedia_player_es_t player)
{
    uint8_t pes_id = player->stream.stream.id.master;
    batom_t atom;

    if(pes_id>=2) {
        bmedia_pes_info pes_info;
        size_t hdr_len;
        uint32_t pts = bmedia_time2pts(player->position.last, player->time_scale);

        BMEDIA_PES_INFO_INIT(&pes_info,  pes_id);
        BMEDIA_PES_SET_PTS(&pes_info,  pts);
        hdr_len = bmedia_pes_header_init(player->pes_header, batom_len(player->frame.atom), &pes_info);
        atom = batom_from_range_and_atom(player->pes_header, hdr_len, player->frame.atom, NULL, NULL);
        batom_release(player->frame.atom);
    } else {
        atom = player->frame.atom;
    }
    player->frame.atom = NULL;
    return atom;
}

static int 
bmedia_player_es_next(bmedia_player_es_t player, bmedia_player_entry *entry)
{
    bfile_buffer_result result;

    BDBG_OBJECT_ASSERT(player, bmedia_player_es);
    BDBG_ASSERT(entry);

    bmedia_player_init_entry(entry);
    entry->start = player->offset;
    if(!b_media_player_es_load_next_frame(player, &result, true)) { 
        if(result == bfile_buffer_result_no_data) {
            entry->type = bmedia_player_entry_type_no_data;
            goto done;
        } else if(result == bfile_buffer_result_eof) {
            entry->type = bmedia_player_entry_type_end_of_stream;
            goto done;
        }
        goto error;
    }
    entry->type = bmedia_player_entry_type_atom;
    entry->content = bmedia_player_entry_content_payload;
    entry->timestamp = player->position.last;
    entry->atom = b_media_player_es_make_pes_packet(player);
    if(!entry->atom) { goto error;}
    entry->length = batom_len(entry->atom);
done:
    return 0;
error:
    return -1;
}

static void 
bmedia_player_es_tell(bmedia_player_es_t player, bmedia_player_pos *pos)
{
    BDBG_OBJECT_ASSERT(player, bmedia_player_es);
    *pos = player->position.last;
    return;
}

static void
b_media_player_es_update_bounds(bmedia_player_es_t player)
{
    if(player->stream.stream.es.audio_codec != baudio_format_mp3) {
        if(player->file.bitrate) {
            player->status.bounds.last = (BMEDIA_PLAYER_POS_SCALE*8*(player->file.last - player->file.first))/player->file.bitrate;
        }
    }
    return;
}

static void 
bmedia_player_es_get_status(bmedia_player_es_t player, bmedia_player_status *status)
{
    BDBG_OBJECT_ASSERT(player, bmedia_player_es);

    b_media_player_es_update_bounds(player);
    *status = player->status;
    return;
}

static void 
bmedia_player_es_set_direction(bmedia_player_es_t player, bmedia_player_step direction, bmedia_time_scale time_scale, bmedia_player_decoder_mode *mode)
{
    BDBG_OBJECT_ASSERT(player, bmedia_player_es);
    BSTD_UNUSED(mode);
    if(direction==0) {
        player->status.direction = direction;
    } else {
        BDBG_ERR(("bmedia_player_es_set_direction: %#lx direction %d not supported", (unsigned long)player, (int)direction));
    }
    player->time_scale = time_scale;
    return;
}

static int
b_media_player_es_advance_audio(bmedia_player_es_t player, off_t current_offset, bmedia_player_pos current_pos, bmedia_player_pos target, unsigned bitrate)
{
    bmedia_player_pos time_distance;
    off_t byte_distance;
    off_t byte_skipped = 0;
    bmedia_player_pos time_skipped;
    int rc;

    BDBG_ASSERT(target >= current_pos);
    if(bitrate<1024) {bitrate=1024;}
    time_distance = target - current_pos;
    byte_distance = ((uint64_t)time_distance * bitrate)/(8*BMEDIA_PLAYER_POS_SCALE);
    BDBG_MSG(("b_media_player_es_advance_audio:%#lx estimating jump at %u:%u", (unsigned long)player, (unsigned)time_distance, (unsigned)byte_distance));
    if(byte_distance > BMEDIA_PLAYER_ES_BLOCK_THRESHOLD) { /* limit amount of stream data read */
        int resync_offset;
        unsigned i;

        byte_skipped = byte_distance - BMEDIA_PLAYER_ES_BLOCK_THRESHOLD;
        for(i=0;i<10;i++) {
            bfile_buffer_result buffer_result;

            time_skipped = (8 * BMEDIA_PLAYER_POS_SCALE * byte_skipped)/bitrate;
            BDBG_MSG(("b_media_player_es_advance_audio:%#lx %u estimating seek at %u:%u %u", (unsigned long)player, i, (unsigned)time_skipped, (unsigned)byte_skipped, (unsigned)(current_offset+byte_skipped)));
            b_media_player_es_reset_position(player, current_pos+time_skipped, current_offset+byte_skipped);
            resync_offset = b_media_player_es_audio_resync(player, &buffer_result);
            if(resync_offset>=0) {
                byte_skipped += resync_offset;
                break;
            } else {
                if(buffer_result==bfile_buffer_result_buffer_overflow) {
                    byte_skipped += BMEDIA_PLAYER_ES_BLOCK_THRESHOLD; /* try to skip junk data */
                } else {
                    BDBG_WRN(("b_media_player_es_advance_audio:%#lx %u can't resync with a strwam at %u:%u", (unsigned long)player, i, (unsigned)time_skipped, (unsigned)byte_skipped));
                    return -1;
                }
            }
        }
        time_skipped = (8 * BMEDIA_PLAYER_POS_SCALE * byte_skipped)/bitrate;
        BDBG_MSG(("b_media_player_es_advance_audio:%#lx resynced seek at %u:%u", (unsigned long)player, (unsigned)time_skipped, (unsigned)byte_skipped));
        b_media_player_es_reset_position(player, current_pos+time_skipped, current_offset+byte_skipped);
    } else {
        b_media_player_es_reset_position(player, current_pos, current_offset);
    }
    do {
        bool success;
        success = b_media_player_es_load_next_frame(player, NULL, false);
        BDBG_MSG_TRACE(("b_media_player_es_advance_audio:%#lx parsing position:%u(%u) offset:%u %s", (unsigned long)player, (unsigned)player->position.last, (unsigned)target, (unsigned)player->offset, success?"success":""));
        if(!success) { rc = -1; goto error;}
        batom_release(player->frame.atom);
    } while(player->position.last<target);
    if(byte_skipped) {
        unsigned new_bitrate;
        bmedia_player_pos time_parsed = player->auto_bitrate.pos_last - player->auto_bitrate.pos_first;
        
        new_bitrate = b_media_player_get_auto_bitrate(player);
        if(new_bitrate>0) {
            time_skipped = (8 * BMEDIA_PLAYER_POS_SCALE * byte_skipped)/new_bitrate;
            BDBG_MSG(("b_media_player_es_advance_audio:%#lx reavaluated seek at %u:%u target:%u,%u(%u) bitrate:%u(%u)", (unsigned long)player, (unsigned)time_skipped, (unsigned)byte_skipped, (unsigned)player->offset, (unsigned)(current_pos+time_skipped+time_parsed), (unsigned)target, new_bitrate, bitrate));
            b_media_player_es_reset_position(player, current_pos+time_skipped+time_parsed, player->offset);
        }
    }
    b_media_player_es_update_bounds(player);

    return 0;
error:
    return rc;
}

static int 
b_media_player_es_seek_audio(bmedia_player_es_t player, bmedia_player_pos pos)
{
    bmedia_player_pos time;
    off_t offset;
    int rc=0;
    BDBG_OBJECT_ASSERT(player, bmedia_player_es);

    for(;;) {
        BDBG_MSG(("b_media_player_es_seek_audio: %#lx index advance pos:%u:%u:%u off:%u", (unsigned long)player, (unsigned)player->position.last, (unsigned)pos, (unsigned)player->index_wait_time, (unsigned)player->offset));
        if(!(player->position.last<player->index_wait_time && pos>player->index_wait_time)) {
            break;
        }
        rc = b_media_player_es_advance_audio(player, player->offset, player->position.last, player->index_wait_time, player->file.bitrate);
        if(rc<0) {
            BDBG_WRN(("b_media_player_es_seek_audio: %#lx index advance failed pos:%u:%u:%u off:%u", (unsigned long)player, (unsigned)player->position.last, (unsigned)pos, (unsigned)player->index_wait_time, (unsigned)player->offset));
            break;
        }
        b_media_player_es_index_point(player, player->position.last);
    }
    time = bmedia_index_get(player->index, pos, &player->frame.index_entry);
    if(time!=BMEDIA_PLAYER_INVALID) {
        offset = player->frame.index_entry.offset;
    } else {
        offset = player->file.first;
        time = 0;
    } 
    for(;;) {
        BDBG_MSG(("b_media_player_es_seek_audio: %#lx pos:%u off:%u", (unsigned long)player, (unsigned)time, (unsigned)player->offset));
        rc = b_media_player_es_advance_audio(player, offset, time, pos, player->file.bitrate);
        if(rc<0) {
            break;
        }
        if(player->position.last>=pos) {
            break;
        }
        offset = player->offset;
        time = player->position.last;
    }
    BDBG_MSG(("b_media_player_es_seek_audio: %#lx seek pos:%u off:%u rc:%d", (unsigned long)player, (unsigned)player->position.last, (unsigned)player->offset, rc));
    return rc ;
}


static int 
bmedia_player_es_seek(bmedia_player_es_t player, bmedia_player_pos pos)
{
    b_media_player_es_update_bounds(player);
    return b_media_player_es_seek_audio(player, pos);
}

static void
b_media_player_es_calibrate_audio(bmedia_player_es_t  player)
{
    unsigned i;
    unsigned prev_bitrate = 0;
    uint64_t duration=0; /* in us */

    player->file.vbr = false;
    player->file.bitrate = 0;
    for(i=0;i<64 && player->file.bitrate==0;i++) {
        b_media_player_es_frame_info info;
        bool success;

        player->frame.atom = NULL;
        player->frame.time_units = 0;
        player->frame.duration = 0;
        success=b_media_player_es_load_audio_frame(player, &info, i==0, NULL);
        if(!success) {
            break;
        }
        batom_release(player->frame.atom);

        if(prev_bitrate==0) {
            prev_bitrate = info.info.bitrate;
        } else {
            if(prev_bitrate != info.info.bitrate) {
                player->file.vbr = true;
            }
        }
        player->file.frame_duration = (player->frame.duration * 1000000)/player->frame.time_units;
        duration += player->file.frame_duration;

        if(i==0) {
            player->codec->calibrate(player, &info);
        }
    }

    if (player->file.bitrate==0) {
        player->file.bitrate = prev_bitrate*1000;
    }
    if (player->file.bitrate==0 && player->file.vbr && duration!=0) {
        player->file.bitrate = (8*(player->offset-player->file.first)*1000000)/duration;
    }
    if(player->file.bitrate==0) {
        player->file.bitrate=1;
    }
    player->status.bounds.last = (BMEDIA_PLAYER_POS_SCALE*8*(player->file.last - player->file.first))/player->file.bitrate;
    BDBG_MSG(("b_media_player_es_calibrate_audio: %p %s %u:%u:%u:%u", (void *)player, player->file.vbr?"VBR":"CBR", (unsigned)duration, (unsigned)(player->offset - player->file.first), player->file.bitrate, (unsigned)player->status.bounds.last));
    return;
}


static bmedia_player_es_t
bmedia_player_es_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream)
{
    BERR_Code rc;
    bmedia_player_es_t  player;
    bfile_buffer_cfg buffer_cfg;

    BSTD_UNUSED(rc);
    BDBG_ASSERT(fd);
    BDBG_ASSERT(config);
    BDBG_ASSERT(stream);

    BSTD_UNUSED(fd);

    
    if(!config->buffer || !config->factory) {
        BDBG_ERR(("bmedia_player_es_create: buffer has to be provided"));
        goto err_config;
    }
    if(stream->stream.es.video_codec != bvideo_codec_unknown || 
            !(stream->stream.es.audio_codec == baudio_format_mp3 || stream->stream.es.audio_codec == baudio_format_mpeg ||
            stream->stream.es.audio_codec == baudio_format_aac ||
            stream->stream.es.audio_codec == baudio_format_ac3 ||
            stream->stream.es.audio_codec == baudio_format_ac3_plus
            )
            ) {
        BDBG_ERR(("bmedia_player_es_create: not supported codec"));
        goto err_config;
    }
    player = BKNI_Malloc(sizeof(*player));
    if(!player) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(player, bmedia_player_es);
    player->time_scale = BMEDIA_TIME_SCALE_BASE;
    player->config = *config;
    player->stream = *stream;
    bmedia_player_init_status(&player->status);
    if(stream->stream.id.master>=2) {
        player->status.format = bstream_mpeg_type_pes;
    } else {
        player->status.format = bstream_mpeg_type_es;
    }
    bfile_buffer_default_cfg(&buffer_cfg);
    buffer_cfg.buf = player->index_buffer_data;
    buffer_cfg.buf_len = sizeof(player->index_buffer_data);
    buffer_cfg.nsegs = sizeof(player->index_buffer_data)/BIO_BLOCK_SIZE;
    buffer_cfg.fd = fd;
    player->index_buffer = bfile_buffer_create(config->factory, &buffer_cfg);
    if(!player->index_buffer) {
        goto err_index_buffer;
    }

    if(bfile_buffer_get_bounds(config->buffer, &player->file.first, &player->file.last)<0) { 
        player->file.first = 0;
        player->file.last = -1;
    }
    player->accum_src = batom_accum_create(config->factory);
    if(!player->accum_src) {
        goto err_accum_src;
    }
    player->accum_pes = batom_accum_create(config->factory);
    if(!player->accum_pes) {
        goto err_accum_pes;
    }
   
    switch(stream->stream.es.audio_codec) {
    case baudio_format_mp3: 
    case baudio_format_mpeg:
        rc = b_media_player_es_mp3_probe(player);
        if(rc!=BERR_SUCCESS) { goto err_probe; }
        player->codec = &b_audio_player_es_mpeg_info;
        break;
    case baudio_format_aac:
        rc = b_media_player_es_aac_probe(player);
        if(rc!=BERR_SUCCESS) { goto err_probe; }
        player->codec = &b_audio_player_es_aac_info;
        break;
    case baudio_format_ac3:
    case baudio_format_ac3_plus:
        rc = b_media_player_es_ac3_probe(player);
        if(rc!=BERR_SUCCESS) { goto err_probe; }
        player->codec = &b_audio_player_es_ac3_info;
        break;
    default:
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_codec;
    }
    b_media_player_es_reset_position(player, 0, player->file.first);
    b_media_player_es_calibrate_audio(player);
    b_media_player_es_reset_position(player, 0, player->file.first);

    player->index = bmedia_index_create(player->status.bounds.last, sizeof(b_media_es_entry), 100);
    if(!player->index) { 
        goto err_index;
    }
    player->index_wait_time = bmedia_index_next(player->index);
    b_media_player_es_index_point(player, 0);

    return player;

err_codec:
err_probe:
err_index:
    batom_accum_destroy(player->accum_pes);
err_accum_pes:
    batom_accum_destroy(player->accum_src);
err_accum_src:
    bfile_buffer_destroy(player->index_buffer);
err_index_buffer:
    BKNI_Free(player);
err_alloc:
err_config:
    return NULL;
}

static void *
b_media_player_es_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream)
{
    return bmedia_player_es_create(fd, config, stream);
}

static void 
b_media_player_es_destroy(void *player)
{
    bmedia_player_es_destroy(player);
    return;
}

static int  
b_media_player_es_next(void *player, bmedia_player_entry *entry)
{
    return bmedia_player_es_next(player, entry);
}

static void 
b_media_player_es_tell(void *player, bmedia_player_pos *pos)
{
    bmedia_player_es_tell(player, pos);
    return;
}

static void 
b_media_player_es_get_status(void *player, bmedia_player_status *status)
{
    bmedia_player_es_get_status(player, status);
    return;
}

static int
b_media_player_es_set_direction(void *player, bmedia_player_step direction, bmedia_time_scale time_scale, bmedia_player_decoder_mode *mode)
{
    bmedia_player_es_set_direction(player, direction, time_scale, mode);
    return 0;
}

static int
b_media_player_es_seek(void *player, bmedia_player_pos pos)
{
    return bmedia_player_es_seek(player, pos);
}

const bmedia_player_methods bmedia_player_es = {
    b_media_player_es_create,
    b_media_player_es_destroy,
    b_media_player_es_next,
    b_media_player_es_tell,
    b_media_player_es_get_status,
    b_media_player_es_set_direction,
    b_media_player_es_seek
};
