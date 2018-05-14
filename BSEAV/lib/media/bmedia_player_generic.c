/***************************************************************************
 * Copyright (C) 2007-2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 * BMedia library, generic(no index) player
 *
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bmedia_player.h"
#include "bmedia_player_generic.h"
#include "bkni.h"

BDBG_MODULE(bmedia_player_generic);
typedef struct bmedia_player_generic *bmedia_player_generic_t;

BDBG_OBJECT_ID(bmedia_player_generic);

/* 256 bytes of filling */
static const uint8_t bmedia_player_generic_stuffing[256] = {0};

struct bmedia_player_generic {
    BDBG_OBJECT(bmedia_player_generic)
    enum {bmedia_player_generic_mode_file, bmedia_player_generic_mode_stuffing} mode;
    off_t offset;
    size_t  length;
    size_t  stuffing_bytes;
    size_t  stuffing_offset;
    bfile_io_read_t fd;
    bmedia_player_config config;
    bmedia_player_stream stream;
    bmedia_player_step direction;
    bmedia_time_scale time_scale;
    off_t first, last; /* offset of the file */
    uint8_t last_block[BIO_BLOCK_SIZE+BIO_BLOCK_SIZE]; /* we need to get BIO_BLOCK_SIZE alligned block in memory */
};

static bmedia_player_generic_t
bmedia_player_generic_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream)
{
    BERR_Code rc;
    bmedia_player_generic_t  player;

    BSTD_UNUSED(rc);
    BDBG_ASSERT(fd);
    BDBG_ASSERT(config);
    BDBG_ASSERT(stream);

    if(stream->stream.noindex.bitrate==0) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_stream;
    }
    player = BKNI_Malloc(sizeof(*player));
    if(!player) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(player, bmedia_player_generic);
    player->config = *config;
    player->stream = *stream;
    player->length = (188/4) * BIO_BLOCK_SIZE;
    player->stuffing_bytes = 16384;
    player->stuffing_offset = 0;
    player->fd = fd;
    player->mode = bmedia_player_generic_mode_file;
    if(player->fd->bounds(player->fd, &player->first, &player->last)<0) {
        player->first = 0;
        player->last = ((uint64_t) 1)<<62; /* some very large number */
    }
    player->offset = player->first;

    return player;

err_alloc:
err_stream:
    return NULL;
}

static void
bmedia_player_generic_destroy(bmedia_player_generic_t player)
{
    BDBG_OBJECT_DESTROY(player, bmedia_player_generic);
    BKNI_Free(player);
    return;
}

static int
bmedia_player_generic_next(bmedia_player_generic_t player, bmedia_player_entry *entry)
{
    BDBG_OBJECT_ASSERT(player, bmedia_player_generic);
    bmedia_player_init_entry(entry);
    switch(player->mode) {
    default:
        BDBG_ASSERT(0);
        /* keep going */
    case bmedia_player_generic_mode_file:
        entry->start = player->offset;
        entry->type = bmedia_player_entry_type_file;
        entry->embedded = NULL;

        if((off_t)(player->offset + player->length) <= player->last || player->config.timeshifting) {
            entry->length = player->length;
            player->offset += player->length;
            break;
        }
        if(player->offset < player->last) { /* sending last packet */
            entry->length = player->last - player->offset;
            entry->length -= entry->length%BIO_BLOCK_SIZE; /* ensure that block is alligned */
            player->offset += entry->length;
            if(entry->length>0) {
                break;
            }
            if(player->offset!=player->last) { /* we need to read last bytes, since playback rejects not BIO_BLOCK_SIZE reads, try to read it here  */
                ssize_t rc;
                uint8_t *last_block = player->last_block;
                off_t offset;
                off_t seek_rc;

                offset = player->offset;
                player->offset = player->last;
                last_block += BIO_BLOCK_SIZE - 1;
                last_block -= (((unsigned long)last_block) % BIO_BLOCK_SIZE);
                BDBG_MSG(("bmedia_player_generic_next:%p alligning addr %#lx to %#lx", (void *)player, (unsigned long)player->last_block, (unsigned long)last_block));
                seek_rc = player->fd->seek(player->fd, offset, SEEK_SET);
                if(seek_rc==offset) {
                    rc = player->fd->read(player->fd, last_block, BIO_BLOCK_SIZE);
                    BDBG_MSG(("bmedia_player_generic_next:%p read %lu bytes from %lu to %#lx[%02x,%02x,%02x,%02x]", (void *)player, (unsigned long)rc, (unsigned long)offset, (unsigned long)last_block, last_block[0], last_block[1], last_block[2], last_block[3]));
                    seek_rc = player->fd->seek(player->fd, offset, SEEK_SET);
                    if(rc>=0 && seek_rc == offset) {
                        entry->start = player->offset;
                        entry->type  = bmedia_player_entry_type_embedded;
                        entry->embedded = last_block;
                        entry->length = (size_t)rc;
                        break;
                    }
                }
            }
        }
        player->mode = bmedia_player_generic_mode_stuffing;
        player->stuffing_offset = 0;
        /* keep going */
    case bmedia_player_generic_mode_stuffing:
        if(player->stuffing_offset >= player->stuffing_bytes) {
            entry->type = bmedia_player_entry_type_end_of_stream;
            return 0; /* sent all data */
        }
        switch(player->stream.format) {
        default:
        case bstream_mpeg_type_es:
        case bstream_mpeg_type_ts:
        case bstream_mpeg_type_vob:
        case bstream_mpeg_type_pes:
        case bstream_mpeg_type_dss_es:
        case bstream_mpeg_type_dss_pes:
        case bstream_mpeg_type_mpeg1:
            entry->start = 0;
            entry->type  = bmedia_player_entry_type_embedded;
            entry->embedded = bmedia_player_generic_stuffing;
            entry->length = sizeof(bmedia_player_generic_stuffing);
            BDBG_MSG(("bmedia_player_generic_next: %p stuffing_offset %u:%u", (void *)player, (unsigned)player->stuffing_offset, (unsigned)player->stuffing_bytes));
            player->stuffing_offset += sizeof(bmedia_player_generic_stuffing);
            break;
        case bstream_mpeg_type_asf:
        case bstream_mpeg_type_avi:
        case bstream_mpeg_type_flv:
        case bstream_mpeg_type_wav:
            /* for media files we can't just add 0, since this would greatly upset parsers, however we need to indicate EOS, so we send empty packet here */
            entry->start = 0;
            entry->type  = bmedia_player_entry_type_embedded;
            entry->embedded = bmedia_player_generic_stuffing;
            entry->length = 0;
            BDBG_MSG(("bmedia_player_generic_next: %p media EOS", (void *)player));
            player->stuffing_offset = player->stuffing_bytes;
            break;
        }
        break;
    }
    return 0;
}

static void
bmedia_player_generic_tell(bmedia_player_generic_t player, bmedia_player_pos *pos)
{
    BDBG_OBJECT_ASSERT(player, bmedia_player_generic);
    *pos = (bmedia_player_pos)((BMEDIA_PLAYER_POS_SCALE*8*(uint64_t)player->offset)/player->stream.stream.noindex.bitrate);
    return;
}

static void
bmedia_player_generic_get_status(bmedia_player_generic_t player, bmedia_player_status *status)
{
    off_t first, last;
    BDBG_OBJECT_ASSERT(player, bmedia_player_generic);
    status->direction = player->direction;
    status->bounds.first = 0;
    status->bounds.last = 0;
    if(player->fd->bounds(player->fd, &first, &last)<0) { goto error; }
    status->bounds.first = (bmedia_player_pos) ((BMEDIA_PLAYER_POS_SCALE*8*(uint64_t)first)/player->stream.stream.noindex.bitrate);
    status->bounds.last = (bmedia_player_pos) ((BMEDIA_PLAYER_POS_SCALE*8*(uint64_t)last)/player->stream.stream.noindex.bitrate);
error:
    return;
}

static void
bmedia_player_generic_set_direction(bmedia_player_generic_t player, bmedia_player_step direction, bmedia_time_scale time_scale, bmedia_player_decoder_mode *mode)
{
    BDBG_OBJECT_ASSERT(player, bmedia_player_generic);
    BSTD_UNUSED(mode);
    player->direction = direction;
    player->time_scale = time_scale;
    return;
}

static int
bmedia_player_generic_seek(bmedia_player_generic_t player, bmedia_player_pos pos)
{
    BDBG_OBJECT_ASSERT(player, bmedia_player_generic);
    player->offset = (off_t)((pos*(uint64_t)player->stream.stream.noindex.bitrate)/(BMEDIA_PLAYER_POS_SCALE*8));
    player->offset -= player->offset%BIO_BLOCK_SIZE; /* keep offset alligned */
    BDBG_MSG(("bmedia_player_generic:%#lx pos %u bitrate %u offset %u", (unsigned long)player, (unsigned)pos, (unsigned)player->stream.stream.noindex.bitrate, (unsigned)player->offset));
    player->mode = bmedia_player_generic_mode_file;
    return 0;
}

static void *
b_media_player_generic_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream)
{
    return bmedia_player_generic_create(fd, config, stream);
}

static void
b_media_player_generic_destroy(void *player)
{
    bmedia_player_generic_destroy(player);
    return;
}

static int
b_media_player_generic_next(void *player, bmedia_player_entry *entry)
{
    return bmedia_player_generic_next(player, entry);
}

static void
b_media_player_generic_tell(void *player, bmedia_player_pos *pos)
{
    bmedia_player_generic_tell(player, pos);
    return;
}

static void
b_media_player_generic_get_status(void *player, bmedia_player_status *status)
{
    bmedia_player_generic_get_status(player, status);
    return;
}

static int
b_media_player_generic_set_direction(void *player, bmedia_player_step direction, bmedia_time_scale time_scale, bmedia_player_decoder_mode *mode)
{
    bmedia_player_generic_set_direction(player, direction, time_scale, mode);
    return 0;
}

static int
b_media_player_generic_seek(void *player, bmedia_player_pos pos)
{
    return bmedia_player_generic_seek(player, pos);
}

const bmedia_player_methods bmedia_player_generic = {
    b_media_player_generic_create,
    b_media_player_generic_destroy,
    b_media_player_generic_next,
    b_media_player_generic_tell,
    b_media_player_generic_get_status,
    b_media_player_generic_set_direction,
    b_media_player_generic_seek
};

