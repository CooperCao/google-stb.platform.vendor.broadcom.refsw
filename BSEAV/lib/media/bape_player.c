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
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bape_player.h"
#include "bape_util.h"
#include "bfile_cache.h"

BDBG_MODULE(bape_player);
#define BDBG_MSG_TRACE(x) BDBG_MSG(x)

BDBG_OBJECT_ID(bape_player_t);

typedef enum b_ape_player_state {
    b_ape_player_state_payload,
    b_ape_player_state_end_of_stream
} b_ape_player_state;

struct bape_player {
    BDBG_OBJECT(bape_player_t)
    bfile_io_read_t  fd;
    bmedia_player_stream stream;
    bfile_cache_t  frame_offset_cache;
    batom_accum_t accum_src;
    batom_accum_t accum_dest;
    batom_accum_t accum_frame;
    bmedia_time_scale time_scale;
    struct {
        b_ape_player_state state;
        uint32_t next_frame_offset;
        uint32_t first_frame_offset;
        unsigned frame_no;
        uint64_t sample_no;
        bool first_frame;
        struct {
            bool valid;
            uint32_t offset;
            size_t size;
            unsigned nsamples;
        } frame;
    } state;
    bmedia_player_status status;
    bmedia_player_config config;
    bape_file_descriptor file_descriptor;
    bape_frame_header frame_descriptor;
    struct {
       unsigned offset;
       uint8_t data[16384];
    } frame_buf;
    uint8_t pes_buf[4096];
};


static void 
b_ape_player_reset_state(bape_player_t player)
{
    int result;
    player->state.state = b_ape_player_state_payload;
    player->state.frame_no = 0;
    player->state.sample_no = 0;
    player->state.frame.valid = false;
    player->state.frame.offset = 0;
    player->state.frame.size = 0;
    player->state.first_frame_offset = 0;
    result = bfile_cache_seek(player->frame_offset_cache, 0);
    if (result != 0)
    {
        BDBG_ERR(("b_ape_player_reset_state:%#lx seek to 0 failed with result %d", (unsigned long)player, result));
    }
}


static int
b_ape_player_open_file(bape_player_t player)
{
    uint8_t buf[BAPE_FILE_DESCRIPTOR_LENGTH]; 
    ssize_t read_result;
    batom_cursor cursor;
    batom_vec vec;
    bfile_segment frame_idx;

    BDBG_OBJECT_ASSERT(player, bape_player_t);
    /* 1. read  file_descriptor */
    player->fd->seek(player->fd, 0, SEEK_SET);
    read_result =  player->fd->read(player->fd, buf, BAPE_FILE_DESCRIPTOR_LENGTH);
    BDBG_MSG(("b_ape_player_open_file:%p read %d bytes", (void *)player, (int)read_result));
    if(read_result!=BAPE_FILE_DESCRIPTOR_LENGTH) {
        goto err_read;
    }
    BATOM_VEC_INIT(&vec, buf, read_result);
    batom_cursor_from_vec(&cursor, &vec, 1);
    if(!bape_parse_file_descriptor(&cursor, &player->file_descriptor)) {
        goto err_open;
    }

    /* 2. read  frame_header */
    read_result = player->fd->read(player->fd, buf, BAPE_FRAME_HEADER_LENGTH);
    BDBG_MSG(("b_ape_player_open_file:%p read %d bytes", (void *)player, (int)read_result));
    if(read_result!=BAPE_FRAME_HEADER_LENGTH) {
        goto err_read;
    }
    BATOM_VEC_INIT(&vec, buf, read_result);
    batom_cursor_from_vec(&cursor, &vec, 1);
    if(!bape_parse_frame_header(&cursor, &player->frame_descriptor) || player->frame_descriptor.sample_rate==0) {
        goto err_open;
    }

    player->status.bounds.first = 0;
    player->status.bounds.last = ((player->frame_descriptor.total_frames * player->frame_descriptor.blocks_per_frame + player->frame_descriptor.final_frame_blocks) * (uint64_t)1000)/player->frame_descriptor.sample_rate;

    bfile_segment_clear(&frame_idx);
    bfile_segment_set(&frame_idx, BAPE_FILE_DESCRIPTOR_LENGTH+BAPE_FRAME_HEADER_LENGTH, player->file_descriptor.seektable_length);


    player->frame_offset_cache = bfile_cache_create(player->fd, &frame_idx, 256*sizeof(uint32_t), sizeof(uint32_t));
    if(!player->frame_offset_cache) {
        goto err_cache;
    }
    b_ape_player_reset_state(player);

    return 0;

err_cache:
err_read:
err_open:
    return -1;
}



bape_player_t
bape_player_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream)
{
    bape_player_t player;

    BDBG_ASSERT(fd);
    BDBG_ASSERT(config);
    BDBG_ASSERT(stream && stream->format == bstream_mpeg_type_ape);

    player = BKNI_Malloc(sizeof(*player));
    if (!player) {
        BDBG_ERR(("can't allocated %u bytes", (unsigned)sizeof(*player)));
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(player, bape_player_t);
    player->fd = fd;
    player->config = *config;
    player->stream = *stream;
    player->time_scale = BMEDIA_TIME_SCALE_BASE;
    bmedia_player_init_status(&player->status);

    BDBG_ASSERT(player->config.factory);
    player->accum_src = batom_accum_create(player->config.factory);
    if(!player->accum_src) {
        BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_accum_src;
    }
    player->accum_dest = batom_accum_create(player->config.factory);
    if(!player->accum_dest) {
        BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_accum_dest;
    }
    player->accum_frame = batom_accum_create(player->config.factory);
    if(!player->accum_frame) {
        BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_accum_frame;
    }


    if(b_ape_player_open_file(player)<0) {
        BDBG_WRN(("no index"));
        goto err_index;
    }

    return player;

err_index:
    batom_accum_destroy(player->accum_frame);
err_accum_frame:
    batom_accum_destroy(player->accum_dest);
err_accum_dest:
    batom_accum_destroy(player->accum_src);
err_accum_src:
    BKNI_Free(player);
err_alloc:
    return NULL;
}

void
bape_player_destroy(bape_player_t player)
{
    BDBG_OBJECT_ASSERT(player, bape_player_t);

    bfile_cache_destroy(player->frame_offset_cache);
    batom_accum_destroy(player->accum_frame);
    batom_accum_destroy(player->accum_dest);
    batom_accum_destroy(player->accum_src);
    BDBG_OBJECT_DESTROY(player, bape_player_t);
    BKNI_Free(player);
    return;
}

static void
b_ape_player_frame_advance(bape_player_t player)
{
    player->state.frame_no ++;
    player->state.sample_no += player->state.frame.nsamples;
    player->state.frame.valid = false;
    return;
}


static int
b_ape_player_frame_locate(bape_player_t player, bool *end_of_stream)
{
    int rc=0;
    const uint8_t *buf;
    uint32_t frame_offset;
    size_t frame_size;

    *end_of_stream = false;
    if(player->state.frame.valid) {
        frame_offset = player->state.frame.offset;
        frame_size = player->state.frame.size;
        goto done;
    } else if(player->state.frame_no == 0) {
        buf = bfile_cache_next(player->frame_offset_cache);
        if(!buf) {
            BDBG_WRN(("b_ape_player_frame_locate:%p reached end of index for first frame:%u(%u,%u)",  (void *)player, player->state.frame_no, player->file_descriptor.seektable_length/4, player->frame_descriptor.total_frames));
            if(bfile_cache_is_file_error(player->frame_offset_cache)) {
                goto error;
            }
            *end_of_stream = true;
            goto done;
        }
        player->state.next_frame_offset = B_MEDIA_LOAD_UINT32_LE(buf,0);
        player->state.first_frame_offset = player->state.next_frame_offset;
        BDBG_MSG_TRACE(("b_ape_player_frame_locate:%p first frame offset:%u", (void *)player, (unsigned)player->state.next_frame_offset));
    } else if(player->state.state == b_ape_player_state_end_of_stream) {
        *end_of_stream = true;
        goto done;
    }
    frame_offset = player->state.next_frame_offset;
    buf = bfile_cache_next(player->frame_offset_cache);
    if(!buf) {
        BDBG_MSG_TRACE(("b_ape_player_frame_locate:%p reached end of index frame_no:%u(%u,%u)",  (void *)player, player->state.frame_no, player->file_descriptor.seektable_length/4, player->frame_descriptor.total_frames));
        if(bfile_cache_is_file_error(player->frame_offset_cache)) {
            goto error;
        }
        player->state.state = b_ape_player_state_end_of_stream;
        frame_size = (player->file_descriptor.audiodata_length+player->state.first_frame_offset)-frame_offset;
        BDBG_MSG_TRACE(("b_ape_player_frame_locate:%p last frame size:%u offset:%u audiodata_length:%u first_frame_offset:%u", (void *)player, (unsigned)frame_size, (unsigned)frame_offset, (unsigned)player->file_descriptor.audiodata_length, (unsigned)player->state.first_frame_offset));
    } else {
        player->state.next_frame_offset = B_MEDIA_LOAD_UINT32_LE(buf,0);
        BDBG_MSG_TRACE(("b_ape_player_frame_locate:%p frame:%u offset:%u", (void *)player, player->state.frame_no+1, (unsigned)player->state.next_frame_offset));
        if(player->state.next_frame_offset <= frame_offset) {
            BDBG_WRN(("b_ape_player_frame_locate:%p invalid frame offset %u:%u", (void *)player, frame_offset, player->state.next_frame_offset));
            goto error;
        }
        frame_size = player->state.next_frame_offset -  frame_offset;
    }
    player->state.frame.valid = true;
    player->state.frame.offset = frame_offset;
    player->state.frame.size = frame_size;
    player->state.frame.nsamples = (player->state.frame_no+1) == player->frame_descriptor.total_frames ? player->frame_descriptor.final_frame_blocks : player->frame_descriptor.blocks_per_frame;

done:
    return rc;

error:
    rc = -1;
    goto done;
}

static void
b_ape_player_data_error(bape_player_t player)
{
    player->status.data_error_cnt++;
    BDBG_MSG_TRACE(("b_ape_player_data_error:%p data_error:%u", (void *)player, (unsigned)player->status.data_error_cnt));
    player->config.error_detected(player->config.cntx);
    return;
}

static void *
b_ape_player_frame_buf_alloc(bape_player_t player, size_t size)
{
    void *result = &player->frame_buf.data[player->frame_buf.offset];

    if(player->frame_buf.offset + size < sizeof(player->frame_buf.data)) {
        player->frame_buf.offset += size;
        return result;
    }
    BDBG_WRN(("b_ape_player_frame_buf_alloc:%p out of memory %u,%u,%u", (void *)player, (unsigned)size, player->frame_buf.offset, (unsigned)sizeof(player->frame_buf.data)));
    return NULL;
}

static void
b_ape_player_frame_buf_clear(bape_player_t player)
{
    BDBG_MSG_TRACE(("b_ape_player_frame_buf_clear:%p used %u[%u]", (void *)player, player->frame_buf.offset, (unsigned)sizeof(player->frame_buf.data)));
    player->frame_buf.offset = 0;
    return;
}

static batom_t
b_ape_player_process_frame(bape_player_t player, batom_t atom)
{
    unsigned nsamples;
    uint8_t pes_id;
    batom_t pes_atom;
    unsigned subframe_offset;
    batom_cursor cursor;

    BDBG_OBJECT_ASSERT(player, bape_player_t);
    nsamples = player->state.frame.nsamples;
    pes_id = bmedia_player_stream_get_id(&player->stream, 1);

    BDBG_MSG(("b_ape_player_process_frame:%p frame_no:%u frame_size:%u sample_no:%u nsamples:%u", (void *)player, player->state.frame_no, (unsigned)player->state.frame.size, (unsigned)player->state.sample_no, nsamples));
    BDBG_ASSERT(bmedia_frame_bcma.len==4);
    b_ape_player_frame_buf_clear(player);
    batom_accum_clear(player->accum_src);
    batom_accum_clear(player->accum_frame);
    batom_accum_add_atom(player->accum_frame, atom);
    batom_release(atom);
    batom_cursor_from_accum(&cursor, player->accum_frame);
    for(subframe_offset=0;subframe_offset<player->state.frame.size;) {
        size_t subframe_size = 16384;
        size_t header_size = BMEDIA_FIELD_LEN(BCMA, uint32_t)+BMEDIA_FIELD_LEN(FrameSize, uint32_t)+BMEDIA_FIELD_LEN(OffsetIntoFrame, uint32_t)+BMEDIA_FIELD_LEN(FrameSize, uint32_t);
        batom_cursor subframe;
        uint8_t *frame_header;

        if(subframe_offset + subframe_size > player->state.frame.size) {
            subframe_size = player->state.frame.size - subframe_offset;
        }
        header_size =
            BMEDIA_FIELD_LEN(BCMA, uint32_t)+BMEDIA_FIELD_LEN(FrameSize, uint32_t)+BMEDIA_FIELD_LEN(OffsetIntoFrame, uint32_t)+BMEDIA_FIELD_LEN(ApeFrameSize, uint32_t)+

            BMEDIA_FIELD_LEN(FirstFrameOffsetInFile, uint32_t)+ BMEDIA_FIELD_LEN(FrameOffsetInFile, uint32_t)+ BMEDIA_FIELD_LEN(blocks_per_frame, uint32_t)+ BMEDIA_FIELD_LEN(samplerate, uint32_t)+ BMEDIA_FIELD_LEN(compression_type, uint16_t)+
            BMEDIA_FIELD_LEN(bits_per_sample, uint16_t)+ BMEDIA_FIELD_LEN(channels, uint16_t)+ BMEDIA_FIELD_LEN(file_version, uint16_t);
        frame_header = b_ape_player_frame_buf_alloc(player, header_size);
        if(frame_header==NULL) {
            break;
        }
        BKNI_Memcpy(frame_header+0, bmedia_frame_bcma.base, bmedia_frame_bcma.len);
        B_MEDIA_SAVE_UINT32_BE((frame_header+4), subframe_size);

        B_MEDIA_SAVE_UINT32_BE((frame_header+8), subframe_offset);
        B_MEDIA_SAVE_UINT32_BE((frame_header+12), player->state.frame.size);
        B_MEDIA_SAVE_UINT32_BE((frame_header+16), player->state.first_frame_offset);
        B_MEDIA_SAVE_UINT32_BE((frame_header+20), player->state.frame.offset);
        B_MEDIA_SAVE_UINT32_BE((frame_header+24), nsamples);
        B_MEDIA_SAVE_UINT32_BE((frame_header+28), player->frame_descriptor.sample_rate);
        B_MEDIA_SAVE_UINT16_BE((frame_header+32), player->frame_descriptor.compression_type);
        B_MEDIA_SAVE_UINT16_BE((frame_header+34), player->frame_descriptor.bits_per_sample);
        B_MEDIA_SAVE_UINT16_BE((frame_header+36), player->frame_descriptor.channels);
        B_MEDIA_SAVE_UINT16_BE((frame_header+38), player->file_descriptor.file_version);
        if(pes_id!=1) {
            batom_accum_add_range(player->accum_src, frame_header, header_size);
        }
        BATOM_CLONE(&subframe, &cursor);
        batom_cursor_skip(&cursor, subframe_size);
        BDBG_MSG(("b_ape_player_process_frame:%p subframe_size:%u subframe_offset:%u", (void *)player, (unsigned)subframe_size, subframe_offset));

        batom_accum_append(player->accum_src, player->accum_frame, &subframe, &cursor );
        subframe_offset += subframe_size;
    }

    if(pes_id>=2) { /* not ES */
        bmedia_pes_info pes_info;
        BMEDIA_PES_INFO_INIT(&pes_info, pes_id);
        BMEDIA_PES_SET_PTS(&pes_info, bmedia_time2pts((player->state.sample_no*1000)/player->frame_descriptor.sample_rate, player->time_scale));
        pes_atom = bmedia_pes_subdivide_packet(player->accum_src, &pes_info, player->accum_dest, player->pes_buf, sizeof(player->pes_buf), player->config.max_pes_size?player->config.max_pes_size:B_MEDIA_MAX_PES_SIZE);
    } else {
        pes_atom = batom_from_accum(player->accum_src, NULL, NULL);
    }
    b_ape_player_frame_advance(player);
    return pes_atom;
}

static void 
b_ape_player_read_cont(void *cntx, batom_t atom, bfile_buffer_result result)
{
    bape_player_t player = cntx;
    bmedia_player_entry entry;

    BDBG_OBJECT_ASSERT(player, bape_player_t);

    bmedia_player_init_entry(&entry);
    if(atom && result==bfile_buffer_result_ok) {
        entry.atom = b_ape_player_process_frame(player, atom);
    } else {
        if(atom) {
            batom_release(atom);
        }
        entry.atom = NULL;
        if(result!=bfile_buffer_result_no_data) {
            BDBG_WRN(("b_ape_player_read_cont: read failed %p (%u:%u)", (void *)player, (unsigned)player->state.frame.offset, (unsigned)player->state.frame.size));
        }
    } 
    if(entry.atom) {
        entry.type = bmedia_player_entry_type_atom;
        entry.content = bmedia_player_entry_content_payload;
        entry.length = batom_len(entry.atom);
    } else {
        switch(result) {
        case bfile_buffer_result_no_data:
            entry.type = bmedia_player_entry_type_no_data;
            break;
        case bfile_buffer_result_buffer_overflow:
            entry.type = bmedia_player_entry_type_file;
            b_ape_player_data_error(player);
            break;
        default:
            entry.type = bmedia_player_entry_type_error;
            b_ape_player_data_error(player);
            break;
        }
        entry.length = 0;
    }
    if(entry.type != bmedia_player_entry_type_no_data) {
        player->state.frame.valid = false;
    }
    BDBG_ASSERT(player->config.atom_ready);
    player->config.atom_ready(player->config.cntx, &entry);
    return;

}


int
bape_player_next(bape_player_t player, bmedia_player_entry *entry)
{
    int rc=0;
    uint32_t frame_offset;
    size_t frame_size;
    bool end_of_stream;

    BDBG_OBJECT_ASSERT(player, bape_player_t);
    BDBG_MSG_TRACE(("bape_player_next:> %#lx %u %u", (unsigned long)player, (unsigned)player->status.position, (unsigned)player->state.state));
    bmedia_player_init_entry(entry);
   
    rc = b_ape_player_frame_locate(player, &end_of_stream);
    if(rc!=0) {goto done;}
    if(end_of_stream) {
        entry->type = bmedia_player_entry_type_end_of_stream;
        goto done;
    }

    frame_offset = player->state.frame.offset;
    frame_size = player->state.frame.size;

    BDBG_MSG_TRACE(("bape_player_next:%p frame:%u reading from:%u,%u", (void *)player, player->state.frame_no, frame_offset, (unsigned)frame_size));
    if(player->config.atom_ready) {
        bfile_buffer_result result;
        entry->atom = bfile_buffer_async_read(player->config.buffer, frame_offset, frame_size, &result, b_ape_player_read_cont, player);
        if(result==bfile_buffer_result_ok && entry->atom) {
            entry->atom = b_ape_player_process_frame(player, entry->atom);
        } else {
            if(entry->atom) {
                batom_release(entry->atom);
                entry->atom=NULL;
            }
            switch(result) {
            case bfile_buffer_result_async:
                entry->type = bmedia_player_entry_type_async;
                break;
            case bfile_buffer_result_no_data:
                entry->type = bmedia_player_entry_type_no_data;
                break;
            default:
                player->state.frame.valid = false;
                entry->type = bmedia_player_entry_type_error;
                break;
            }
            goto done; /* asynchronous read */
        }
    } else {
        bfile_buffer_result result;
        entry->atom = bfile_buffer_read(player->config.buffer, frame_offset, frame_size, &result);
        if(result == bfile_buffer_result_ok && entry->atom) {
            entry->atom = b_ape_player_process_frame(player, entry->atom);
            if(entry->atom) {
                entry->type = bmedia_player_entry_type_atom;
            }
        } else {
            if(entry->atom!=NULL) {
                batom_release(entry->atom);
                entry->atom=NULL;
            }
            switch(result) {
            case bfile_buffer_result_no_data:
                entry->type = bmedia_player_entry_type_no_data;
                break;
            default:
                entry->type = bmedia_player_entry_type_error;
                break;
            }
            goto done;
        }
    }
    if(entry->atom) {
        entry->type = bmedia_player_entry_type_atom;
        entry->content = bmedia_player_entry_content_payload;
        entry->length = batom_len(entry->atom);
    }

done:
    BDBG_MSG(("bape_player_next: %#lx %u (%lu:%u)", (unsigned long)player, (unsigned)player->status.position, (unsigned long)entry->start, (unsigned)entry->length));
    return rc;
}

void
bape_player_tell(bape_player_t player, bmedia_player_pos *pos)
{
    BDBG_OBJECT_ASSERT(player, bape_player_t);
    *pos = player->status.position;

    return;
}

void
bape_player_get_status(bape_player_t player, bmedia_player_status *status)
{
    BDBG_OBJECT_ASSERT(player, bape_player_t);
    BDBG_MSG_TRACE(("bape_player_get_status:> %#lx", (unsigned long)player));
    *status = player->status;
    BDBG_MSG_TRACE(("bape_player_get_status:< %#lx %ld:%ld", (unsigned long)player, (long)player->status.bounds.first, (long)player->status.bounds.last));
    return;
}

int
bape_player_set_direction(bape_player_t player, bmedia_player_step direction, bmedia_time_scale time_scale, bmedia_player_decoder_mode *mode)
{
    BDBG_OBJECT_ASSERT(player, bape_player_t);
    BSTD_UNUSED(mode);
    BDBG_MSG_TRACE(("bape_player_set_direction:> %#lx %ld", (unsigned long)player, (long)direction));
    if(direction!=0) {
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        return -1;
    }
    player->time_scale = time_scale;
    BDBG_MSG_TRACE(("bape_player_set_direction:< %#lx", (unsigned long)player));
    return 0;
}

int
bape_player_seek(bape_player_t player, bmedia_player_pos pos)
{
    int rc=0;
    BDBG_MSG_TRACE(("bape_player_seek:> %p %ld", (void *)player, (long)pos));
    BDBG_OBJECT_ASSERT(player, bape_player_t);
    if (pos >= player->status.bounds.last || pos < player->status.bounds.first) {
        BDBG_WRN(("bape_player_seek:< %p out of bounds %u<%u>%u %ld", (void *)player, (unsigned)player->status.bounds.first, (unsigned)pos, (unsigned)player->status.bounds.last, (long)-1));
        pos = player->status.bounds.first;
    }
    b_ape_player_reset_state(player);
    if(pos!=player->status.bounds.first) {
        /* since all (except the last) frames are of  the same length, instead of reading all index entries we could just find frame for desired location */
        for(;;) {
            bool end_of_stream;
            bmedia_player_pos frame_end;
            rc = b_ape_player_frame_locate(player, &end_of_stream);
            if(rc!=0) {
                break;
            }
            if(end_of_stream) {
                rc = -1;
                break;
            }
            frame_end = ((player->state.sample_no+player->state.frame.nsamples)*1000)/player->frame_descriptor.sample_rate;
            if(frame_end >= pos) {
                BDBG_MSG(("bape_player_seek:< %p %u(%u) frame:%u", (void *)player, (unsigned)pos, (unsigned)frame_end, player->state.frame_no));
                break;
            }
            b_ape_player_frame_advance(player);
        }
    }
    BDBG_MSG_TRACE(("bape_player_seek:< %#lx %u(%d)", (unsigned long)player, (unsigned)pos, rc));
    return rc;
}

static void *
b_ape_player_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream)
{
    return bape_player_create(fd, config, stream);
}

static void
b_ape_player_destroy(void *player)
{
    bape_player_destroy(player);
}

static int
b_ape_player_next(void *player, bmedia_player_entry *entry)
{
    return bape_player_next(player, entry);
}

static void
b_ape_player_tell(void *player, bmedia_player_pos *pos)
{
    bape_player_tell(player, pos);
    return;
}

static void
b_ape_player_get_status(void *player, bmedia_player_status *status)
{
    bape_player_get_status(player, status);
    return;
}

static int
b_ape_player_set_direction(void *player, bmedia_player_step direction, bmedia_time_scale time_scale, bmedia_player_decoder_mode *mode)
{
    return bape_player_set_direction(player, direction, time_scale, mode);
}

static int
b_ape_player_seek(void *player, bmedia_player_pos pos)
{
    return bape_player_seek(player, pos);
}

const bmedia_player_methods bape_player_methods = {
    b_ape_player_create,
    b_ape_player_destroy,
    b_ape_player_next,
    b_ape_player_tell,
    b_ape_player_get_status,
    b_ape_player_set_direction,
    b_ape_player_seek
};

    
