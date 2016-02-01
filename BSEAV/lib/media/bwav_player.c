/***************************************************************************
 *     Copyright (c) 2007-2011, Broadcom Corporation
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
 * WAV player library
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "briff_parser.h"
#include "bwav_player.h"
#include "bfile_cache.h"
#include "blst_squeue.h"
#include "bmedia_index.h"

BDBG_MODULE(bwav_player);
#define BDBG_MSG_TRACE(x) BDBG_MSG(x)

BDBG_OBJECT_ID(bwav_player_t);

enum b_wav_player_state {
    b_wav_player_state_header=0, 
    b_wav_player_state_data
};

typedef struct b_wav_player_object_handler {
	briff_parser_handler handler; /* must be first member */
	bwav_player_t player;
} b_wav_player_object_handler;

struct bwav_player {
	BDBG_OBJECT(bwav_player_t)
	bfile_io_read_t  fd;
	BLST_SQ_HEAD(b_wav_player_streams, b_wav_player_stream) streams;
	bmedia_player_stream stream;
	size_t  chunk_size;
	briff_off_t data_offset;
	unsigned data_size;
	enum b_wav_player_state player_state;
    briff_off_t play_offset;
    bmedia_player_status status;
    bmedia_player_config config;
    bmedia_waveformatex fmt;
	b_wav_player_object_handler fmt_handler;
    bool fmt_valid;
    bool open_error;
};

static void
b_wav_player_atom_free(batom_t atom, void *user)
{
	void *ptr = *(void **)user;
	BDBG_MSG(("free_wav %#lx", (unsigned long)atom));
	BSTD_UNUSED(atom);
	BKNI_Free(ptr);
	return;
}

static const batom_user b_wav_player_atom = {
	b_wav_player_atom_free,
	sizeof(void **)
};

static briff_parser_action
b_wav_player_fmt(briff_parser_handler *handler, briff_chunk_id chunk_id, unsigned object_size, unsigned object_offset, batom_t object)
{
    briff_parser_action  action = briff_parser_action_none;
	bwav_player_t player = ((b_wav_player_object_handler*)handler)->player;
	BDBG_MSG(("b_wav_player_fmt: %p " BMEDIA_FOURCC_FORMAT "[" BMEDIA_FOURCC_FORMAT "] %u bytes", (void *)player, BMEDIA_FOURCC_ARG(handler->chunk_id), BMEDIA_FOURCC_ARG(chunk_id), object?(unsigned)batom_len(object):0));
	BDBG_OBJECT_ASSERT(player, bwav_player_t);
    if(chunk_id==BMEDIA_FOURCC('f','m','t',' ') && object && object_offset==0 && object_size==batom_len(object)) {
        batom_cursor cursor;

        batom_cursor_from_atom(&cursor, object);

        player->fmt_valid = bmedia_read_waveformatex(&player->fmt, &cursor);
        if(!player->fmt_valid) { goto error; }
        if(player->fmt.nAvgBytesPerSec==0) {
	        BDBG_MSG(("b_wav_player_fmt: %p " BMEDIA_FOURCC_FORMAT "[" BMEDIA_FOURCC_FORMAT "] %u bytes", (void *)player, BMEDIA_FOURCC_ARG(handler->chunk_id), BMEDIA_FOURCC_ARG(chunk_id), object?(unsigned)batom_len(object):0));
            goto error;
        }

	}
done:
	if(object) {
		batom_release(object);
	}
	return action;

error:
    player->open_error = true;
    action = briff_parser_action_return;
    goto done;
}

static briff_parser_action
b_wav_player_object_begin(void *cntx, briff_chunk_id chunk_id, unsigned size, briff_off_t offset)
{
    bwav_player_t player = cntx;
    briff_parser_action  action = briff_parser_action_none;
    BDBG_MSG(("b_wav_player_object_begin:%p player " BMEDIA_FOURCC_FORMAT " %u:(%u)",  (void *)player, BMEDIA_FOURCC_ARG(chunk_id), size, (unsigned)offset));
    BDBG_OBJECT_ASSERT(player, bwav_player_t);

    if(chunk_id != BMEDIA_FOURCC('d','a','t','a')) {
        goto done;
    }
    if(!player->fmt_valid) {
        /* no FMT record */
        player->open_error = true;
        goto done;
    }
    BDBG_ASSERT(player->fmt.nAvgBytesPerSec);
    player->status.bounds.last = (BMEDIA_PLAYER_POS_SCALE*(uint64_t)size / player->fmt.nAvgBytesPerSec);
    player->data_size = size;
    player->data_offset = offset;
    action = briff_parser_action_return;

done:
    return action;
}

static briff_parser_action 
b_wav_player_file_type(void *context, briff_chunk_id file_type)
{
    bwav_player_t    player  = context;
    BSTD_UNUSED(player);
    BDBG_OBJECT_ASSERT(player, bwav_player_t);

    player->open_error = file_type!=BMEDIA_FOURCC('W','A','V','E');
    BDBG_MSG(("b_wav_player_file_type:%#lx player " BMEDIA_FOURCC_FORMAT " %ssupported",  (unsigned long)player, BMEDIA_FOURCC_ARG(file_type), player->open_error?"not ":""));

    return player->open_error ? briff_parser_action_return : briff_parser_action_none ;
}

static int
b_wav_player_open_file(bwav_player_t player)
{
	batom_factory_t factory;
	batom_pipe_t pipe_wav;
	batom_t atom;
	bfile_io_read_t fd;
	briff_parser_cfg cfg;
	briff_parser_t  wav;
	int rc=-1;
    size_t parsed_size;

	BDBG_OBJECT_ASSERT(player, bwav_player_t);
	fd = player->fd;
	player->data_offset = 0;
	player->data_size = 0;
    player->fmt_valid = false;
    player->open_error = false;
	factory = batom_factory_create(bkni_alloc, 64);
	if(!factory) {
		goto err_factory;
	}

	briff_parser_default_cfg(&cfg);
	cfg.user_cntx = player;
    cfg.object_begin = b_wav_player_object_begin;
    cfg.file_type = b_wav_player_file_type;
	wav = briff_parser_create(factory, &cfg);
	if(!wav) {
		goto err_wav;
	}
	player->fmt_handler.player = player;

	briff_parser_install_handler(wav, &player->fmt_handler.handler, BMEDIA_FOURCC('f','m','t',' '), b_wav_player_fmt); 

	pipe_wav = batom_pipe_create(factory);
	if (!pipe_wav) {
		goto err_pipe;
	}
	fd->seek(fd, 0, SEEK_SET);
	for(parsed_size=0;parsed_size<=player->config.max_data_parsed;) {
		void *buf;
		ssize_t read_result;
		size_t wav_result;
		const size_t len = player->config.prefered_read_size;

		buf = BKNI_Malloc(len);
		if (!buf) {
			goto err_alloc;
		}
		parsed_size += len;
		read_result =  fd->read(fd, buf, len);
        BDBG_MSG(("b_wav_player_open_file:%p read %d bytes", (void *)player, (int)read_result));
		if(read_result<0) {
			BKNI_Free(buf);
			goto err_read;
		}
		atom = batom_from_range(factory, buf, read_result, &b_wav_player_atom, &buf);
		if(!atom) {
			BKNI_Free(buf);
			goto err_atom;
		}
		batom_pipe_push(pipe_wav, atom);
		wav_result = briff_parser_feed(wav, pipe_wav);
        BDBG_MSG(("b_wav_player_open_file:%p parsed %d bytes %s", (void *)player, (unsigned)wav_result, player->open_error?"error":""));
		if(wav_result==0 || player->open_error) {
			goto err_parser;
		}
        if(player->data_offset != 0) {
	        BDBG_MSG(("b_wav_player_open_file: %#lx bytrate:%u bounds:%u..%u", (unsigned long)player, player->fmt.nAvgBytesPerSec, (unsigned)player->status.bounds.first, (unsigned)player->status.bounds.last));
            rc = 0;
            break;
        }
	}

	if(rc!=0) {
		BDBG_WRN(("b_wav_player_open_file: %#lx no valid data found", (unsigned long)player));
	}


err_parser:
err_atom:
err_read:
err_alloc:
	batom_pipe_flush(pipe_wav);
	batom_pipe_destroy(pipe_wav);
err_pipe:
	briff_parser_remove_handler(wav, &player->fmt_handler.handler);
	briff_parser_destroy(wav);
err_wav:
	batom_factory_destroy(factory);
err_factory:
	return rc;
}




bwav_player_t
bwav_player_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream)
{
	bwav_player_t player;


	BDBG_ASSERT(fd);
	BDBG_ASSERT(config);
	BDBG_ASSERT(stream && stream->format == bstream_mpeg_type_wav);

	player = BKNI_Malloc(sizeof(*player));
	if (!player) {
		BDBG_ERR(("can't allocated %u bytes", (unsigned)sizeof(*player)));
		goto err_alloc;
	}
	BDBG_OBJECT_INIT(player, bwav_player_t);
	player->fd = fd;
	player->config = *config;
	player->stream = *stream;
    bmedia_player_init_status(&player->status);

	if(b_wav_player_open_file(player)<0) {
		BDBG_WRN(("no index"));
		goto err_index;
	}

	player->player_state = b_wav_player_state_header;

	player->chunk_size = 60 * 1024;
    player->play_offset = 0;

	return player;

err_index:
	BKNI_Free(player);
err_alloc:
	return NULL;
}

void
bwav_player_destroy(bwav_player_t player)
{
	BDBG_OBJECT_ASSERT(player, bwav_player_t);

	BDBG_OBJECT_DESTROY(player, bwav_player_t);
	BKNI_Free(player);
	return;
}

int
bwav_player_next(bwav_player_t player, bmedia_player_entry *entry)
{
    size_t read_length;

	BDBG_OBJECT_ASSERT(player, bwav_player_t);
	BDBG_MSG_TRACE(("bwav_player_next:> %#lx %u %u", (unsigned long)player, (unsigned)player->status.position, (unsigned)player->player_state));
    bmedia_player_init_entry(entry);

	entry->type = bmedia_player_entry_type_file;
    entry->embedded = NULL;
	/* send a haader first */
	switch(player->player_state) {
	case b_wav_player_state_data:
        entry->start = player->data_offset + player->play_offset;
	    entry->content = bmedia_player_entry_content_payload;
        read_length =  player->chunk_size;
        if(player->play_offset + read_length > player->data_size) {
            read_length = player->data_size - player->play_offset;
        }
        player->play_offset += read_length;
        entry->length = read_length;
        BDBG_ASSERT(player->fmt.nAvgBytesPerSec);
        player->status.position = (BMEDIA_PLAYER_POS_SCALE * (uint64_t)player->play_offset) / player->fmt.nAvgBytesPerSec;
        if(read_length==0) {
	        entry->type = bmedia_player_entry_type_end_of_stream;
        }
		break;
	case b_wav_player_state_header:
		entry->start =  0 ;
		entry->length = player->data_offset;
        entry->content =  bmedia_player_entry_content_header;
		player->player_state = b_wav_player_state_data;
        break;
	}
	/* now send a data payload */


	BDBG_MSG(("bwav_player_next: %#lx %u (%lu:%u)", (unsigned long)player, (unsigned)player->status.position, (unsigned long)entry->start, (unsigned)entry->length));
	return 0;
}

void
bwav_player_tell(bwav_player_t player, bmedia_player_pos *pos)
{
	BDBG_OBJECT_ASSERT(player, bwav_player_t);
    *pos = player->status.position;

	return;
}

void
bwav_player_get_status(bwav_player_t player, bmedia_player_status *status)
{
	BDBG_OBJECT_ASSERT(player, bwav_player_t);
	BDBG_MSG_TRACE(("bwav_player_get_status:> %#lx", (unsigned long)player));
    *status = player->status;
	BDBG_MSG_TRACE(("bwav_player_get_status:< %#lx %ld:%ld", (unsigned long)player, (long)player->status.bounds.first, (long)player->status.bounds.last));
	return;
}

int
bwav_player_set_direction(bwav_player_t player, bmedia_player_step direction, bmedia_time_scale time_scale, bmedia_player_decoder_mode *mode)
{
	BDBG_OBJECT_ASSERT(player, bwav_player_t);
    BSTD_UNUSED(time_scale);
    BSTD_UNUSED(mode);
	BDBG_MSG_TRACE(("bwav_player_set_direction:> %#lx %ld", (unsigned long)player, (long)direction));
    if(direction!=0) {
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        return -1;
    }
	player->player_state = b_wav_player_state_header; /* set state to header in order to resent the RIFF header */
	BDBG_MSG_TRACE(("bwav_player_set_direction:< %#lx", (unsigned long)player));
	return 0;
}

int
bwav_player_seek(bwav_player_t player, bmedia_player_pos pos)
{
    BDBG_MSG_TRACE(("bwav_player_seek:>%p %ld", (void *)player, (long)pos));
    BDBG_OBJECT_ASSERT(player, bwav_player_t);
    if (pos >= player->status.bounds.last || pos < player->status.bounds.first) {
        BDBG_WRN(("bwav_player_seek:<%p out of bounds %u<%u>%u %ld", (void *)player, (unsigned)player->status.bounds.first, (unsigned)pos, (unsigned)player->status.bounds.last, (long)-1));
        pos = player->status.bounds.first;
    }
    player->play_offset = (player->fmt.nAvgBytesPerSec * (uint64_t)pos) / BMEDIA_PLAYER_POS_SCALE;
    if(player->fmt.nBlockAlign) {
        player->play_offset -= player->play_offset % player->fmt.nBlockAlign;
    }
    player->status.position = (BMEDIA_PLAYER_POS_SCALE * (uint64_t)player->play_offset) / player->fmt.nAvgBytesPerSec;
    player->player_state = b_wav_player_state_header; /* set state to header in order to resent the RIFF header */
    BDBG_MSG_TRACE(("bwav_player_seek:<%p %u->%u", (void *)player, (unsigned)pos, (unsigned)player->play_offset));
    return 0;
}

static void *
b_wav_player_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream)
{
    return bwav_player_create(fd, config, stream);
}

static void
b_wav_player_destroy(void *player)
{
	bwav_player_destroy(player);
}

static int
b_wav_player_next(void *player, bmedia_player_entry *entry)
{
	return bwav_player_next(player, entry);
}

static void
b_wav_player_tell(void *player, bmedia_player_pos *pos)
{
	bwav_player_tell(player, pos);
	return;
}

static void
b_wav_player_get_status(void *player, bmedia_player_status *status)
{
	bwav_player_get_status(player, status);
	return;
}

static int
b_wav_player_set_direction(void *player, bmedia_player_step direction, bmedia_time_scale time_scale, bmedia_player_decoder_mode *mode)
{
	return bwav_player_set_direction(player, direction, time_scale, mode);
}

static int
b_wav_player_seek(void *player, bmedia_player_pos pos)
{
	return bwav_player_seek(player, pos);
}

const bmedia_player_methods bwav_player_methods = {
	b_wav_player_create,
	b_wav_player_destroy,
	b_wav_player_next,
	b_wav_player_tell,
	b_wav_player_get_status,
	b_wav_player_set_direction,
	b_wav_player_seek
};

    
