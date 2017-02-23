/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2007-2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bmpeg2pes_parser.h"
#include "bmpeg2pes_player.h"
#include "btime_indexer.h"
#include "botf_marker.h"
#include "biobits.h"

BDBG_MODULE(bmpeg2pes_player);
#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x) */

BDBG_OBJECT_ID(bmpeg2pes_player_t);

typedef enum b_mpeg2pes_player_file_mode {
    b_mpeg2pes_player_file_mode_sequential, 
    b_mpeg2pes_player_file_mode_forward,
    b_mpeg2pes_player_file_mode_backward
} b_mpeg2pes_player_file_mode;

typedef enum b_mpeg2pes_player_decoder_mode {
    b_mpeg2pes_player_decoder_mode_normal,
    b_mpeg2pes_player_decoder_mode_otf,
    b_mpeg2pes_player_decoder_mode_fragmented
} b_mpeg2pes_player_decoder_mode;

typedef enum b_file_feed_mode {
    b_file_feed_mode_sequential, 
    b_file_feed_mode_forward,
    b_file_feed_mode_backward
} b_file_feed_mode;

typedef enum b_file_feed_state {
    b_file_feed_state_header,
    /* b_file_feed_state_tail, */
    b_file_feed_state_eof_tail, /* after reading last byte of data we need to prepend trailer */
    b_file_feed_state_data
} b_file_feed_state;

struct b_mpeg2pes_feed_config {
    b_file_feed_mode mode; /* mode of operation */
    size_t chunk; /* size of contignous data chunk */
    size_t stride; /* distance between chunks */
};

struct bmpeg2pes_player {       
       BDBG_OBJECT(bmpeg2pes_player_t)
       struct b_mpeg2pes_feed_config feed_config;
       bmedia_player_status status;
       off_t read_offset;
       off_t feed_stop_offset; /* current location of end of the feed block, reset when advanced to new feed */
       off_t feed_start_offset; /* current location of end of the feed block, reset when advanced to new feed */ 
       size_t async_read_size; /* size of the last async read */
       unsigned feed_count;
       b_mpeg2pes_player_decoder_mode decoder_mode;
       b_file_feed_state feed_state;
       bmpeg2pes_demux_t pes_demux;
       batom_pipe_t pes_pipe;
       btime_indexer_t index;
       bmedia_time_scale time_scale;
       bmedia_player_pos  pts_position;
       bfile_io_read_t fd;
       unsigned nindexentries;
       uint32_t last_pts;
       bool last_pts_valid;
       bool index_error;
       bmedia_player_config config;
       bmedia_player_stream stream;
       uint8_t packet_buf[1024]; 
};


#define B_MPEG2PES_PLAYER_MIN_BLOCK (BIO_BLOCK_SIZE*64)
#define B_MPEG2PES_PLAYER_READ_SIZE (BIO_BLOCK_SIZE*16)

#define B_MPEG2PES_PLAYER_ADVANCE_BRACKETS  (2*BMEDIA_PLAYER_POS_SCALE)

#define B_MPEG2PES_FILLER_SIZE 256
#define B_MPEG2PES_BPP_LENGTH 184

/* advances player to desired location, this function builds index by creating new index entry every BINDEX_TIME_STEP */
static int
b_mpeg2pes_player_advance(bmpeg2pes_player_t player, bmedia_player_pos destination)
{
	batom_pipe_t pes_pipe;
	int rc=0;
	bmedia_player_pos destination_min, destination_max;
	bmedia_player_pos step;
    bmedia_player_pos step_size = BINDEX_SEEK_STEP;


	pes_pipe = player->pes_pipe;
    if (player->config.force_seek_step != 0) {
        step_size = player->config.force_seek_step;
    }
	for(step=step_size;;) {
		btime_indexer_position position;
		uint64_t offset;
		bmedia_player_pos delta;

		if(destination>=step) {
			destination_min = destination-step;
		} else {
			destination_min = 0;
		}
		destination_max = destination+step;
        if(btime_indexer_position_by_time(player->index, destination, &position)!=true) {
            BDBG_WRN(("b_mpeg2pes_player_advance: %p can't get position for %u", (void *)player, (unsigned)destination));
            rc = -1;
            break;
        }
        BDBG_MSG(("b_mpeg2pes_player_advance: %p range %u:%u %u:%u", (void *)player, (unsigned)position.prev.time, (unsigned)destination_min, (unsigned)position.next.time, (unsigned)destination_max));
        while(position.prev.time >= destination_min && position.next.time <= destination_max ) {
            BDBG_MSG(("b_mpeg2pes_player_advance: %p step %u -> %u", (void *)player, (unsigned)step, (unsigned)step/2));
            step /= 2;
            if(step<B_MPEG2PES_PLAYER_ADVANCE_BRACKETS) {
                goto done;
            }
        }
		if(position.next.time < destination ) {
			if(destination >= position.next.time + step) {
				delta = step;
			} else  if (destination > position.next.time) {
				delta = destination - position.next.time;
			} else {
				delta = 0;
			}
			btime_indexer_seek(player->index, position.next.time+delta);
			offset = position.next.offset + ((uint64_t)position.byterate * delta)/BMEDIA_PLAYER_POS_SCALE;
            BDBG_MSG(("b_mpeg2pes_player_advance: %p forward time:%u(%u) offset:" B_OFFT_FMT "(" B_OFFT_FMT ")", (void *)player, (unsigned)(position.next.time+delta), (unsigned)(position.next.time), B_OFFT_ARG(offset), B_OFFT_ARG(position.next.offset)));
		} else {
			if(destination >= position.prev.time && destination <= position.next.time) {
				if(position.next.time - destination >= destination - position.prev.time) {
                    delta = destination - position.prev.time;
                    btime_indexer_seek(player->index, position.prev.time+delta);
                    offset = position.prev.offset + ((uint64_t)position.byterate * delta)/BMEDIA_PLAYER_POS_SCALE;
                    BDBG_MSG(("b_mpeg2pes_player_advance: %p search forward time:%u(%u) offset:" B_OFFT_FMT "(" B_OFFT_FMT ")", (void *)player, (unsigned)(position.prev.time+delta), (unsigned)position.prev.time, B_OFFT_ARG(offset), B_OFFT_ARG(position.prev.offset)));
				} else {
                    delta = position.next.time - destination;
                    if(delta<B_MPEG2PES_PLAYER_ADVANCE_BRACKETS) {
                        delta = B_MPEG2PES_PLAYER_ADVANCE_BRACKETS;
                        if(delta > position.next.time) {
                            delta = position.next.time;
                        }
                    }
                    btime_indexer_seek(player->index, position.next.time-delta);
                    offset = position.next.offset - ((uint64_t)position.byterate * delta)/BMEDIA_PLAYER_POS_SCALE;
                    BDBG_MSG(("b_mpeg2pes_player_advance: %p search reverse time:%u(%u) offset:" B_OFFT_FMT "(" B_OFFT_FMT ")", (void *)player, (unsigned)(position.next.time-delta), (unsigned)position.next.time, B_OFFT_ARG(offset), B_OFFT_ARG(position.next.offset)));
				}
			} else {
				continue;
			}
		}
		batom_pipe_flush(pes_pipe);
		bmpeg2pes_demux_seek(player->pes_demux, offset);

		for(;;) {
			bfile_buffer_result result;
			size_t atom_len;
			unsigned nindexentries = player->nindexentries;
			size_t feed_len;
			batom_t atom = bfile_buffer_read(player->config.buffer, offset, B_MPEG2PES_PLAYER_READ_SIZE, &result);
			if(atom==NULL) {
				BDBG_MSG(("b_mpeg2pes_player_advance: %p read completed %u at %u", (void *)player, result, (unsigned)offset));
				goto done;
			}
			atom_len = batom_len(atom);
			BDBG_MSG_TRACE(("b_mpeg2pes_player_advance: %p read %u:%u -> %p", (void *)player, (unsigned)offset, atom_len, (void *)atom));
			offset += atom_len;
			batom_pipe_push(pes_pipe, atom);
			feed_len = bmpeg2pes_demux_feed(player->pes_demux, pes_pipe);
			BDBG_MSG_TRACE(("b_mpeg2pes_player_advance: %p %u:%u time:%u ", (void *)player, (unsigned)offset, atom_len, player->pts_position));
			if(nindexentries!=player->nindexentries) {
				break;
			}
			if(feed_len!=atom_len ) {
				BDBG_WRN(("b_mpeg2pes_player_advance: %p parser didn't consume all data %u:%u", (void *)player, (unsigned)feed_len, (unsigned)atom_len));
				rc = -1;
				goto done;
			}
		}
	}
 done:
	/* bmpeg2pes_parser_flush(player->parser); */
	bmpeg2pes_demux_reset(player->pes_demux);
	batom_pipe_flush(pes_pipe);
	return rc;
}

static int
b_mpeg2pes_player_open_file(bmpeg2pes_player_t player)
{
	off_t off=0;
	bfile_buffer_t buf;
	bmpeg2pes_demux_t demux;
	batom_pipe_t pipe;
	BDBG_OBJECT_ASSERT(player, bmpeg2pes_player_t);

	demux = player->pes_demux;
	buf = player->config.buffer;
	pipe = player->pes_pipe;
	bmpeg2pes_demux_reset(demux);
	batom_pipe_flush(pipe);
	for(;off<8*1024*1024;) { /* don't reat more than  8MBytes worth of data */
		batom_t atom;
		bfile_buffer_result result;
		size_t feed_len;
		size_t atom_len;
		const size_t read_len = B_MPEG2PES_PLAYER_READ_SIZE;
		unsigned nindexentries = player->nindexentries;

		BDBG_MSG_TRACE(("b_mpeg2pes_player_open_file: %p reading %u:%u", (void *)player, (unsigned)off, read_len));
		atom = bfile_buffer_read(buf, off, read_len, &result);
		if(atom==NULL) {
			BDBG_MSG(("b_mpeg2pes_player_open_file: %p read completed %u at %u", (void *)player, result, (unsigned)off));
			break;
		}
		BDBG_ASSERT(atom);
		atom_len = batom_len(atom);
		BDBG_MSG_TRACE(("b_mpeg2pes_player_open_file: %p read %u:%u -> %#lx", (void *)player, (unsigned)off, atom_len, (unsigned long)atom));
		off += atom_len;
		batom_pipe_push(pipe, atom);
		feed_len = bmpeg2pes_demux_feed(demux, pipe);
		if(feed_len!=atom_len) {
			break;
		}
		if(player->index_error) {
			break;
		}
		if(nindexentries!=player->nindexentries) {
			btime_indexer_status status;
			btime_indexer_get_status(player->index, &status);
			if(status.byterate_valid) {
				break;
			}
		}
	}
	bmpeg2pes_demux_reset(player->pes_demux);
	batom_pipe_flush(pipe);

    if(player->index_error || player->nindexentries<2) {
        BDBG_WRN(("b_mpeg2pes_player_open_file:%p can't get index entries", (void *)player));
        return -1;
    } else {
        return 0;
    }
}

static void
b_mpeg2pes_player_payload(void *packet_cnxt, batom_accum_t src, batom_cursor *payload_start, size_t len, const bmpeg2pes_atom_info *info)
{	
	bmpeg2pes_player_t player = (bmpeg2pes_player_t)packet_cnxt;
	BDBG_OBJECT_ASSERT(player, bmpeg2pes_player_t);


	BSTD_UNUSED(src);
	BSTD_UNUSED(payload_start);
	BSTD_UNUSED(len);	

	if(info->flags & BMPEG2PES_PTS_VALID && info->pes_id == player->stream.master) {
		int rc;
		bmpeg2pes_demux_status demux_status;

		bmpeg2pes_demux_get_status(player->pes_demux, &demux_status);	
		BDBG_MSG_TRACE(("b_mpeg2pes_player_pcr_payload: %p PTS:%#u offset:" BDBG_UINT64_FMT "", (void *)player, (unsigned)info->pts, BDBG_UINT64_ARG((uint64_t)demux_status.data_offset)));
		rc = btime_indexer_add(player->index, info->pts, demux_status.data_offset, &player->pts_position);
		if(rc>=0) {
			if((unsigned)rc!=player->nindexentries) {
				/* TODO : What do we do here? */
				/* action = bmpeg2pes_parser_action_hold; */
			}
			player->nindexentries = (unsigned)rc;
        } else {
            BDBG_WRN(("b_mpeg2pes_player_payload: %p index error:%d", (void *)player, rc));
            player->index_error = true;
        }
    }
    return;

}

static void
b_mpeg2pes_player_update_feed(bmpeg2pes_player_t player, off_t start_offset)
{
	BDBG_MSG(("b_mpeg2pes_player_update_feed:%p " B_OFFT_FMT "", (void *)player, B_OFFT_ARG(start_offset)));
	start_offset -= start_offset%BIO_BLOCK_SIZE;
	switch(player->feed_config.mode) {
	case b_mpeg2pes_player_file_mode_sequential:
		/* fall through */
	case b_mpeg2pes_player_file_mode_forward:
		player->feed_start_offset = start_offset;
		player->feed_stop_offset = player->feed_start_offset + player->feed_config.chunk;
		break;
	case b_mpeg2pes_player_file_mode_backward:
		if(start_offset >= 0) {
			player->feed_start_offset = start_offset;
			player->feed_stop_offset = player->feed_start_offset + player->feed_config.chunk;
		} else {
			size_t chunk;
			start_offset += player->feed_config.stride;
			if(start_offset >= (off_t)player->feed_config.chunk) {
				chunk = player->feed_config.chunk;
			} else {
				chunk = start_offset;
			}
			player->feed_start_offset = 0;
			player->feed_stop_offset = chunk;
		}
		break;
	}
	player->read_offset = player->feed_start_offset;
	player->feed_state = b_file_feed_state_header;
	batom_pipe_flush(player->pes_pipe);
	bmpeg2pes_demux_seek(player->pes_demux, player->feed_start_offset);
	return;
}

bmpeg2pes_player_t
bmpeg2pes_player_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream)
{
	bmpeg2pes_player_t player;
	bmpeg2pes_demux_config demux_cfg;
	BERR_Code rc;

	BDBG_ASSERT(fd);
	BDBG_ASSERT(config);
	BDBG_ASSERT(stream && (stream->format == bstream_mpeg_type_pes || stream->format == bstream_mpeg_type_vob));
	BSTD_UNUSED(fd);

	if(!config->buffer) {
		BDBG_ERR(("bmpeg2pes_player_create: buffer has to be provided"));
		rc = BERR_TRACE(BERR_NOT_SUPPORTED);
		goto err_config;
	}
	if(!config->factory) {
		BDBG_ERR(("bmpeg2pes_player_create: atom_factory has to be provided"));
		rc = BERR_TRACE(BERR_NOT_SUPPORTED);
		goto err_config;
	}
	
	player = BKNI_Malloc(sizeof(*player));
	if (!player) {
		BDBG_ERR(("bmpeg2pes_player_create: can't allocate %u bytes", (unsigned)sizeof(*player)));
		rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto err_alloc;
	}
	BDBG_OBJECT_INIT(player, bmpeg2pes_player_t);

	player->config = *config;
	player->stream = *stream;

	bmedia_player_init_status(&player->status);
	player->status.format = bstream_mpeg_type_pes;
	player->time_scale = BMEDIA_TIME_SCALE_BASE;
	player->pts_position = 0;
	player->nindexentries = 0;
	player->index_error = false;
	player->feed_state = b_file_feed_state_header;
	player->decoder_mode = b_mpeg2pes_player_decoder_mode_normal;
	player->read_offset = 0;
	player->feed_config.mode = b_mpeg2pes_player_file_mode_sequential;
	player->feed_config.chunk = B_MPEG2PES_PLAYER_MIN_BLOCK;
	player->feed_config.stride = B_MPEG2PES_PLAYER_MIN_BLOCK;
	player->fd = fd;
	player->last_pts = 0;
	player->last_pts_valid = false;
	player->feed_count = 0;
	player->async_read_size = 0;
        
	bmpeg2pes_demux_default_config(&demux_cfg);
	demux_cfg.packet_cnxt = player;
	demux_cfg.packet = b_mpeg2pes_player_payload;
	player->pes_demux = bmpeg2pes_demux_create(player->config.factory, &demux_cfg);
	if(!player->pes_demux) {
		goto err_parser;
	}	

	player->pes_pipe = batom_pipe_create(player->config.factory);
	if(!player->pes_pipe) {
		goto err_pipe;
	}
	player->index = btime_indexer_create();
	if(!player->index) {
		goto err_index;
	}
	b_mpeg2pes_player_update_feed(player, 0);

	return player;
	
 err_index:
	batom_pipe_destroy(player->pes_pipe);
 err_pipe:	
	bmpeg2pes_demux_destroy(player->pes_demux);
 err_parser:
	BKNI_Free(player);
 err_alloc:
 err_config:
	return NULL;
}

void 
bmpeg2pes_player_destroy(bmpeg2pes_player_t player)
{
	BDBG_OBJECT_ASSERT(player, bmpeg2pes_player_t);
	btime_indexer_destroy(player->index);
	batom_pipe_destroy(player->pes_pipe);
	bmpeg2pes_demux_destroy(player->pes_demux);
	BDBG_OBJECT_DESTROY(player, bmpeg2pes_player_t);
	BKNI_Free(player);
	return;
}

static void 
b_mpeg2pes_player_data_error(bmpeg2pes_player_t player)
{
    BSTD_UNUSED(player);
    return;
}


static int
b_mpeg2pes_player_process_data(bmpeg2pes_player_t player, batom_t atom, bmedia_player_entry *entry)
{
	batom_pipe_t pipe;
	size_t atom_len;
	size_t feed_len;

	BDBG_ASSERT(atom);

	entry->type = bmedia_player_entry_type_atom;

	pipe = player->pes_pipe;
	atom_len = batom_len(atom);
	BDBG_MSG_TRACE(("bmpeg2pes_player_next:%p read from %lu:%u", (void *)player, (unsigned long)player->read_offset, atom_len));
	entry->length = atom_len;
	entry->atom = atom;
	player->read_offset += atom_len;
	batom_pipe_push(pipe, atom);
	batom_lock(atom);
	feed_len = bmpeg2pes_demux_feed(player->pes_demux, pipe);
	if(feed_len!=atom_len) {
		goto err_parser;
	}
	player->status.position = player->pts_position;
	entry->timestamp = player->pts_position;
	if(player->status.bounds.last<player->status.position) {
		player->status.bounds.last = player->status.position;
	}
	return 0;

 err_parser:
    entry->type = bmedia_player_entry_type_error;
    b_mpeg2pes_player_data_error(player);
    batom_release(atom);
    batom_pipe_flush(pipe);
    BDBG_MSG(("b_mpeg2pes_player_process_data: %p EOF reached", (void *)player));
    return -1;
}

static int
b_mpeg2pes_player_send_header_otf(bmpeg2pes_player_t player, bmedia_player_entry *entry, bool eof)
{
    BOTF_MarkerParams marker;
    BERR_Code rc;

    BKNI_Memset(&marker, 0, sizeof(marker));
    marker.Pid = player->stream.master;
    marker.ChunkCntr = eof? B_MARKER_CHUNK_EOF : player->feed_count ;
    marker.OffsetHi = (player->read_offset >> 32);
    marker.OffsetLow = player->read_offset;
    rc = BOTF_BuildMarkerPES(&marker, player->packet_buf, sizeof(player->packet_buf), &entry->length);
    if(rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);entry->length = 1024;
    }
    entry->type = bmedia_player_entry_type_embedded;
    entry->embedded = player->packet_buf;
    return 0;
}

/**
Summary:
BPP mode values
Description:
These are the supported values in word[0] of a BPP
**/
#define TT_MODE_PLAY                0x00
#define TT_MODE_DISPLAY             0x01
#define TT_MODE_BUILD_REFERENCE     0x02
#define TT_MODE_PROCESS             0x03
#define TT_MODE_DISCARD_TILL_BTP    0x04
#define TT_MODE_DISPLAY_FORWARD     0x05
#define TT_MODE_DISPLAY_REWIND      0x06
#define TT_MODE_DISPLAY_PAST_BUFFER 0x07
#define TT_MODE_PIC_OUTPUT_COUNT         0x09    /* picture output count */
#define TT_MODE_INLINE_FLUSH              0x0A        /* also known as Trick Play Discontinuity (TPD) */
#define TT_MODE_TIMING_MARKER       0x0C
#define TT_MODE_PICTURE_TAG         0x0D

static void 
b_mpeg2pes_ts_make_filler(uint16_t stream_id, uint8_t *pkt)
{
    unsigned i;

    BKNI_Memset(pkt, 0, B_MPEG2PES_FILLER_SIZE);
    pkt[0] = 0x00;
    pkt[1] = 0x00;
    pkt[2] = 0x00;
    pkt[3] = (uint8_t)stream_id;
    /* create dummy startcodes */
    for(i=4;i<B_MPEG2PES_FILLER_SIZE;i+=4) {
        pkt[i] =   0x00;
        pkt[i+1] = 0x00;
        pkt[i+2] = 0x01;
        pkt[i+3] = 0xFF;        
    }
    return;
}

static void
b_mpeg2pes_bpp_make(uint16_t stream_id, uint8_t *pkt, const bmedia_bpp_pkt *bpp)
{
    (void)bmedia_make_bpp_pkt(stream_id, bpp, pkt, B_MPEG2PES_BPP_LENGTH);
    return;
}

static int
b_mpeg2pes_player_send_header_fragmented(bmpeg2pes_player_t player, bmedia_player_entry *entry)
{
    bmedia_bpp_pkt bpp;
    BKNI_Memset(&bpp, 0, sizeof(bpp));
    bpp.data[0] = TT_MODE_PROCESS;
    bpp.data[7] = 0;
    bpp.data[8] = 0;
    b_mpeg2pes_bpp_make(player->stream.master, player->packet_buf, &bpp);
    entry->type = bmedia_player_entry_type_embedded;
    entry->length = B_MPEG2PES_BPP_LENGTH;
    entry->embedded = player->packet_buf;

    return 0;
}

static int
b_mpeg2pes_player_send_tail_fragmented(bmpeg2pes_player_t player, bmedia_player_entry *entry)
{
    bmedia_bpp_pkt bpp;
    BKNI_Memset(&bpp, 0, sizeof(bpp));
    bpp.data[0] = TT_MODE_INLINE_FLUSH;
    b_mpeg2pes_bpp_make(player->stream.master,player->packet_buf, &bpp);
    entry->type = bmedia_player_entry_type_embedded;
    entry->length = B_MPEG2PES_BPP_LENGTH;
    entry->embedded = player->packet_buf;
    return 0;
}

static int
b_mpeg2pes_player_send_header(bmpeg2pes_player_t player, bmedia_player_entry *entry, bool eof)
{
	switch(player->decoder_mode) {
	default:
	case b_mpeg2pes_player_decoder_mode_normal:
		BDBG_ASSERT(0);
		entry->type = bmedia_player_entry_type_no_data;
		return 0;
	case b_mpeg2pes_player_decoder_mode_otf:
		return b_mpeg2pes_player_send_header_otf(player, entry, eof);
	case b_mpeg2pes_player_decoder_mode_fragmented:
		return b_mpeg2pes_player_send_header_fragmented(player, entry);
	}
}


static int
b_mpeg2pes_player_start_eof(bmpeg2pes_player_t player, bmedia_player_entry *entry)
{
	BDBG_MSG(("b_mpeg2pes_player_start_eof: %#lx", (unsigned long) player));
	if(player->decoder_mode == b_mpeg2pes_player_decoder_mode_otf ||
	   !(player->feed_config.mode==b_file_feed_mode_sequential || player->decoder_mode==b_mpeg2pes_player_decoder_mode_normal)) {
		int rc = b_mpeg2pes_player_send_header(player, entry, true);
		player->feed_state = b_file_feed_state_eof_tail;
		if(rc==0) {
			BDBG_ASSERT(player->packet_buf == entry->embedded);
			BDBG_ASSERT(sizeof(player->packet_buf)>= entry->length + 2*B_MPEG2PES_FILLER_SIZE);
			b_mpeg2pes_ts_make_filler(player->stream.master, player->packet_buf + entry->length);
			entry->length += B_MPEG2PES_FILLER_SIZE;
			b_mpeg2pes_ts_make_filler(player->stream.master, player->packet_buf + entry->length);
			entry->length += B_MPEG2PES_FILLER_SIZE;
		}
		return rc;
	} else {
        entry->type = bmedia_player_entry_type_end_of_stream;
	    return 0; /* just EOF */
    }
}

static int
b_mpeg2pes_player_handle_read_error(bmpeg2pes_player_t player, bfile_buffer_result read_result, bmedia_player_entry *entry)
{
	int rc=0;
	switch(read_result) {
	case bfile_buffer_result_async:
		BDBG_ASSERT(0);
		break;
	case bfile_buffer_result_no_data:
		entry->type = bmedia_player_entry_type_no_data;
		break;
	case bfile_buffer_result_eof:
		if(player->feed_config.mode==(b_file_feed_mode)b_mpeg2pes_player_file_mode_backward) {
			entry->type = bmedia_player_entry_type_no_data;
			player->read_offset = player->feed_stop_offset;
		} else {
			rc = b_mpeg2pes_player_start_eof(player, entry);
		} 
		break;
	default:
		b_mpeg2pes_player_data_error(player);
		rc =b_mpeg2pes_player_start_eof(player, entry);
		break;
	}
	return rc;
}

static void 
b_mpeg2pes_player_read_cont(void *cntx, batom_t atom, bfile_buffer_result result)
{
	bmpeg2pes_player_t player = cntx;
	bmedia_player_entry entry;
	int rc=0;

	BDBG_OBJECT_ASSERT(player, bmpeg2pes_player_t);

	bmedia_player_init_entry(&entry);
	if(atom) {
		b_mpeg2pes_player_process_data(player, atom, &entry);
	} else {
        entry.atom = NULL;
        if(!(result==bfile_buffer_result_no_data || result==bfile_buffer_result_eof)) {
            BDBG_WRN(("b_mpeg2pes_player_read_cont: read failed %p (" B_OFFT_FMT ":%u)", (void *)player, B_OFFT_ARG(player->read_offset), (unsigned)player->async_read_size));
        }
        rc = b_mpeg2pes_player_handle_read_error(player, result, &entry);
	}
	if(rc<0) {
		entry.type = bmedia_player_entry_type_error;
	}
	BDBG_ASSERT(player->config.atom_ready);
	player->async_read_size = 0;
	player->config.atom_ready(player->config.cntx, &entry);
	return;
}



static int
b_mpeg2pes_player_read_data(bmpeg2pes_player_t player, bmedia_player_entry *entry)
{
	batom_t atom;
	bfile_buffer_result read_result;
	size_t read_size;
	int rc = 0;

	if(player->read_offset + B_MPEG2PES_PLAYER_READ_SIZE <= player->feed_stop_offset) {
		read_size = B_MPEG2PES_PLAYER_READ_SIZE;
	} else {
		BDBG_ASSERT(player->feed_stop_offset - player->read_offset);
		read_size = player->feed_stop_offset - player->read_offset;
	}

	if(player->config.atom_ready) {		
		atom = bfile_buffer_async_read(player->config.buffer, player->read_offset, read_size, &read_result, b_mpeg2pes_player_read_cont, player);
		player->async_read_size = 0;
		if(atom==NULL) {
			switch(read_result) {
			case bfile_buffer_result_async:
			  player->async_read_size = read_size;
			  entry->type = bmedia_player_entry_type_async;
			  break;
			default:
			  rc = b_mpeg2pes_player_handle_read_error(player, read_result, entry);
			  break;
			}
			goto done;
		}
	} else {
		atom = bfile_buffer_read(player->config.buffer, player->read_offset, read_size, &read_result);
		if(atom==NULL) {
			rc = b_mpeg2pes_player_handle_read_error(player, read_result, entry);
			goto done;
		}
	}
	b_mpeg2pes_player_process_data(player, atom, entry);
 done:
	return rc;
}


int
bmpeg2pes_player_next(bmpeg2pes_player_t player, bmedia_player_entry *entry)
{
	int rc = 0;

	BDBG_OBJECT_ASSERT(player, bmpeg2pes_player_t);
	BDBG_ASSERT(entry);

	bmedia_player_init_entry(entry);

	BDBG_MSG_TRACE(("bmpeg2pes_player_next:%#lx reading position %lu(%lu)", (unsigned long)player, (unsigned long)player->read_offset, (unsigned long)player->feed_stop_offset));
	BDBG_ASSERT(player->async_read_size==0); /* we can't allow multiple outstanding calls into the async I/O */

	if(player->feed_state == b_file_feed_state_eof_tail) {
        entry->type = bmedia_player_entry_type_end_of_stream;
		rc = 0;
		goto done;
	} else if(player->feed_config.mode!=(b_file_feed_mode)b_mpeg2pes_player_file_mode_sequential && player->feed_state==b_file_feed_state_header) {
		player->feed_state = b_file_feed_state_data;
		rc = b_mpeg2pes_player_send_header(player, entry, false);
		goto done;
	} else {
		player->feed_state = b_file_feed_state_data; 
		if(player->read_offset >= player->feed_stop_offset) {
			off_t stride;
			BDBG_MSG(("bmpeg2pes_player_next:%#lx end of block:%u at %lu", (unsigned long)player, player->feed_count, (unsigned long)player->read_offset));
			player->feed_count ++;
			stride = player->feed_config.mode==(b_file_feed_mode)b_mpeg2pes_player_file_mode_backward? -(off_t)player->feed_config.stride : (off_t)player->feed_config.stride;
			b_mpeg2pes_player_update_feed(player, player->feed_start_offset + stride);
			if(player->feed_start_offset == player->feed_stop_offset) {
				/* reached begining of the file */
				rc = b_mpeg2pes_player_start_eof(player, entry);
				goto done;
			}
			BDBG_MSG(("bmpeg2pes_player_next:%#lx new block:%u at %lu..%lu", (unsigned long)player, player->feed_count, (unsigned long)player->feed_start_offset, (unsigned long)player->feed_stop_offset));
			if(player->feed_config.mode==(b_file_feed_mode)b_mpeg2pes_player_file_mode_sequential) {
				rc = b_mpeg2pes_player_read_data(player, entry);
			} else {
				if(player->decoder_mode==b_mpeg2pes_player_decoder_mode_fragmented) {
					player->feed_state = b_file_feed_state_header;
					rc = b_mpeg2pes_player_send_tail_fragmented(player, entry);
				} else {
					player->feed_state = b_file_feed_state_data;
					rc = b_mpeg2pes_player_send_header(player, entry, false);
				}
			}
		} else {
			rc = b_mpeg2pes_player_read_data(player, entry);
		}
	}

 done:
	BDBG_MSG_TRACE(("bmpeg2pes_player_next:%#lx position:%u", (unsigned long)player, (unsigned)player->status.position));
	return rc;
}

void 
bmpeg2pes_player_tell(bmpeg2pes_player_t player, bmedia_player_pos *pos)
{
	BDBG_OBJECT_ASSERT(player, bmpeg2pes_player_t);
	*pos = player->status.position;
	if(player->last_pts_valid) {
		int rc;
		btime_indexer_location location;
		location.timestamp = player->last_pts;
		location.offset = player->read_offset;
		location.direction = player->time_scale<0 ? btime_indexer_direction_backward:btime_indexer_direction_forward;
		/* btime_indexer_dump(player->index); */
		rc = btime_indexer_get_time_by_location(player->index, &location, pos);
	}
    BDBG_MSG(("bmpeg2pes_player_tell:%p %s %u:%u", (void *)player, player->last_pts_valid?"WITH PTS":"", player->last_pts, (unsigned)*pos));
    return;
}

void
bmpeg2pes_player_get_status(bmpeg2pes_player_t player, bmedia_player_status *status)
{
	btime_indexer_status indexer_status;
	BDBG_OBJECT_ASSERT(player, bmpeg2pes_player_t);
    BDBG_MSG_TRACE(("bmpeg2pes_player_get_status:> %p", (void *)player));
	*status = player->status;
	btime_indexer_get_status(player->index, &indexer_status);
	if(indexer_status.byterate==0 && player->async_read_size == 0) {
		int rc = b_mpeg2pes_player_open_file(player);
		if(rc==0) {
			btime_indexer_get_status(player->index, &indexer_status);
			bmpeg2pes_demux_seek(player->pes_demux, player->read_offset);
			btime_indexer_seek(player->index, player->status.position);
		}
	} 
	if(indexer_status.byterate>0) {
		off_t first, last;
		int rc = player->fd->bounds(player->fd, &first, &last);
		if(rc>=0) {
			status->bounds.last = (last*BMEDIA_PLAYER_POS_SCALE) / indexer_status.byterate;
		} 
	} 
    BDBG_MSG_TRACE(("bmpeg2pes_player_get_status:< %p %ld:%ld", (void *)player, (long)status->bounds.first, (long)status->bounds.last));
    return;
}

static size_t
b_mpeg2pes_player_get_chunk_size(bmpeg2pes_player_t player)
{
	btime_indexer_position entry;
	size_t stream_based_size;
	size_t cdb_based_size;
	size_t size;

	if(btime_indexer_position_by_time(player->index, player->status.position, &entry)==true) {
		stream_based_size = 1*entry.byterate; /* assuming gop is less then 1 second */
	} else {
		stream_based_size = B_MPEG2PES_PLAYER_MIN_BLOCK;
	}
	cdb_based_size = (3*player->config.decoder_features.video_buffer_size) / 4; /* video buffer should be able to keep at least 2 chunks */
	if(stream_based_size <= cdb_based_size) {
		size = stream_based_size;
	} else {
		size = cdb_based_size;
		BDBG_WRN(("Limiting size of the trickmode chunk based on the CDB size %u -> %u", (unsigned)stream_based_size, (unsigned)cdb_based_size));
	}
	if(size>B_MPEG2PES_PLAYER_MIN_BLOCK) {
		size += BIO_BLOCK_SIZE - (size%BIO_BLOCK_SIZE);
	} else {
		size = B_MPEG2PES_PLAYER_MIN_BLOCK;
	}

	return size;
}

static bmedia_time_scale
b_mpeg2pes_time_scale_abs(bmedia_time_scale time_scale)
{
	return time_scale>=0?time_scale:-time_scale;
}

static void
b_mpeg2pes_player_I_only_mode(bmpeg2pes_player_t player, bmedia_time_scale time_scale, bmedia_player_decoder_mode *mode)
{
	mode->display_frames = bmedia_player_decoder_frames_I;
	mode->reordering_mode = time_scale >= 0 ? bmedia_player_reordering_mode_forward : bmedia_player_reordering_mode_backward;
	time_scale = b_mpeg2pes_time_scale_abs(time_scale);
	player->feed_config.stride = player->feed_config.chunk * ((time_scale / (4*BMEDIA_TIME_SCALE_BASE))+1);
	mode->time_scale = time_scale/16;
	if(mode->time_scale>BMEDIA_TIME_SCALE_BASE) {
		mode->time_scale = BMEDIA_TIME_SCALE_BASE;
	}
	return;
}

void 
bmpeg2pes_player_set_direction(bmpeg2pes_player_t player, bmedia_player_step direction, bmedia_time_scale time_scale, bmedia_player_decoder_mode *mode)
{
	BDBG_OBJECT_ASSERT(player, bmpeg2pes_player_t);
	BSTD_UNUSED(mode);

	BDBG_MSG_TRACE(("bmpeg2pes_player_set_direction:> %#lx %ld", (unsigned long)player, (long)direction));
	BDBG_ASSERT(mode);
	mode->brcm = false;
	mode->dqt = false;
	mode->host_paced = false;
	mode->display_frames = bmedia_player_decoder_frames_all;
	player->feed_count = 0;
	if(time_scale >= 0 && time_scale <= BMEDIA_TIME_SCALE_BASE) {
		/* normal play */
		mode->continuous = true;
		mode->tsm = true;
		mode->reordering_mode = bmedia_player_reordering_mode_none;
		mode->time_scale = time_scale;
		if(player->time_scale!=time_scale) {
			player->feed_config.mode = b_mpeg2pes_player_file_mode_sequential;
			player->feed_config.chunk = B_MPEG2PES_PLAYER_MIN_BLOCK;
			player->feed_config.stride = B_MPEG2PES_PLAYER_MIN_BLOCK;
			b_mpeg2pes_player_update_feed(player, player->read_offset);
		}
	} else {
		/* trick mode */
		mode->continuous = false;
		mode->tsm = false;
		if(player->config.decoder_features.otf && player->stream.stream.es.video_codec == bvideo_codec_mpeg2) {
			/* MPEG-2 OTF PVR */
			player->feed_state = b_file_feed_state_header;
			player->decoder_mode = b_mpeg2pes_player_decoder_mode_otf;
			if(time_scale >= 0 && time_scale <= 8 * BMEDIA_TIME_SCALE_BASE) {
				mode->reordering_mode = bmedia_player_reordering_mode_sequential;
				mode->time_scale = time_scale;
				player->feed_config.mode = b_mpeg2pes_player_file_mode_sequential;
				player->feed_config.chunk = B_MPEG2PES_PLAYER_MIN_BLOCK;
				player->feed_config.stride = B_MPEG2PES_PLAYER_MIN_BLOCK;
			} else {
				size_t overlap_chunk;
				player->feed_config.chunk = b_mpeg2pes_player_get_chunk_size(player);
				player->feed_config.mode = time_scale >= 0 ? b_mpeg2pes_player_file_mode_forward : b_mpeg2pes_player_file_mode_backward;
				overlap_chunk = (player->feed_config.chunk * 5)/2; /* overlap chunk should be 2x GOP size */
				if(time_scale < 0 && time_scale >= -4 * BMEDIA_TIME_SCALE_BASE && 
				   overlap_chunk <= player->config.decoder_features.video_buffer_size/2 /* and decoder buffer is large enough */ ) {
					mode->reordering_mode = bmedia_player_reordering_mode_interleaved;
					player->feed_config.stride = player->feed_config.chunk;
					player->feed_config.chunk = overlap_chunk;
					mode->time_scale = b_mpeg2pes_time_scale_abs(time_scale);
				} else {
					if(time_scale < 0 && time_scale >= -4 * BMEDIA_TIME_SCALE_BASE) {
						BDBG_WRN(("not enough CBD size for real 1x reverse %u %u", (unsigned)(2*overlap_chunk), (unsigned)player->config.decoder_features.video_buffer_size));
					}
					mode->reordering_mode = time_scale >= 0 ? bmedia_player_reordering_mode_forward : bmedia_player_reordering_mode_backward;
					mode->time_scale = b_mpeg2pes_time_scale_abs(time_scale);
					if(mode->time_scale>10*BMEDIA_TIME_SCALE_BASE) {
						mode->time_scale = 10*BMEDIA_TIME_SCALE_BASE;
					}
					player->feed_config.stride = ((uint64_t)player->feed_config.chunk * b_mpeg2pes_time_scale_abs(time_scale))/mode->time_scale;
				}
			}
			b_mpeg2pes_player_update_feed(player, player->read_offset);
			BDBG_MSG(("bmpeg2pes_player_set_direction:%p OTF feeding data %u:%u rate:%u", (void *)player, (unsigned)player->feed_config.chunk, (unsigned)player->feed_config.stride, (unsigned)mode->time_scale));
		} else if(player->config.decoder_features.fragmented) {
			/* fragmented decode */
		    player->feed_config.mode = time_scale >= 0 ? b_mpeg2pes_player_file_mode_forward : b_mpeg2pes_player_file_mode_backward;
			player->feed_state = b_file_feed_state_header;
			player->decoder_mode = b_mpeg2pes_player_decoder_mode_fragmented;
			mode->display_frames = bmedia_player_decoder_frames_I;
			mode->fragmented = true;
			player->feed_config.chunk = b_mpeg2pes_player_get_chunk_size(player);
			b_mpeg2pes_player_I_only_mode(player, time_scale, mode);
			b_mpeg2pes_player_update_feed(player, player->read_offset);
			BDBG_MSG(("bmpeg2pes_player_set_direction:%p FRAGMENTED feeding data %u:%u", (void *)player, (unsigned)player->feed_config.chunk, (unsigned)player->feed_config.stride));
		} else {
			BDBG_ERR(("bmpeg2pes_player_set_direction:%p trickmodes aren't supported", (void *)player));
			goto done;
		}
	}
	player->status.direction = direction;
	player->time_scale = time_scale;
 done:
	BDBG_MSG_TRACE(("bmpeg2pes_player_set_direction:< %#lx %ld", (unsigned long)player, (long)0));
	return;
}


int 
bmpeg2pes_player_seek(bmpeg2pes_player_t player, bmedia_player_pos pos)
{
	int rc;
	btime_indexer_position position;
	off_t read_offset;
	
	BDBG_OBJECT_ASSERT(player, bmpeg2pes_player_t);
	BDBG_MSG_TRACE(("bmpeg2pes_player_seek:> %#lx %u(%u)", (unsigned long)player, (unsigned)pos, (unsigned)player->status.position));
	player->feed_count = 0;
	if(pos > player->status.bounds.last) {
		rc = b_mpeg2pes_player_advance(player, pos);
		if(rc<0) { 
			rc = b_mpeg2pes_player_open_file(player);
			if(rc<0) { rc=BERR_TRACE(rc);goto error;}
			rc = b_mpeg2pes_player_advance(player, pos);
			if(rc<0) { rc=BERR_TRACE(rc);goto error;}
		}
	}
	if(btime_indexer_position_by_time(player->index, pos, &position)!=true) {
		rc = b_mpeg2pes_player_open_file(player);
		if(rc<0) { rc=BERR_TRACE(rc);goto error;}
		if(!btime_indexer_position_by_time(player->index, pos, &position)) {
			rc=BERR_TRACE(rc);goto error;
		}
	}
	player->status.position = position.time;
	player->last_pts_valid = false;
	player->pts_position = pos;
	{
		off_t first, last;
		int rc = player->fd->bounds(player->fd, &first, &last);
		if(rc>=0 && (off_t)position.offset > last) {
			position.offset = last;
		}
	}
	read_offset = position.offset;
	if(player->feed_config.mode==(b_file_feed_mode)b_mpeg2pes_player_file_mode_backward) {
		read_offset -= player->feed_config.stride;
	}
	b_mpeg2pes_player_update_feed(player, read_offset);
	btime_indexer_seek(player->index, position.time);

	BDBG_MSG_TRACE(("bmpeg2pes_player_seek:< %#lx %u:%u(%lu)", (unsigned long)player, (unsigned)pos, (unsigned)player->status.position, (unsigned long)position.offset));
	return 0;
 error:
	BDBG_WRN(("bmpeg2pes_player_seek:%#lx seek error", (unsigned long)player));
	return -1;
}

void 
bmpeg2pes_player_update_position(bmpeg2pes_player_t player, uint32_t pts)
{
	BDBG_OBJECT_ASSERT(player, bmpeg2pes_player_t);

    player->last_pts = pts;
    player->last_pts_valid = true;
    return;
}


static void *
b_mpeg2pes_player_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream)
{
	return bmpeg2pes_player_create(fd, config, stream);
}

static void
b_mpeg2pes_player_destroy(void *player)
{
	bmpeg2pes_player_destroy(player);
}

static int
b_mpeg2pes_player_next(void *player, bmedia_player_entry *entry)
{
	return bmpeg2pes_player_next(player, entry);
}

static void 
b_mpeg2pes_player_tell(void *player, bmedia_player_pos *pos)
{
	bmpeg2pes_player_tell(player, pos);
	return;
}

static void 
b_mpeg2pes_player_get_status(void *player, bmedia_player_status *status)
{
	bmpeg2pes_player_get_status(player, status);
	return;
}

static int
b_mpeg2pes_player_set_direction(void *player, bmedia_player_step direction, bmedia_time_scale time_scale, bmedia_player_decoder_mode *mode)
{
	bmpeg2pes_player_set_direction(player, direction, time_scale, mode);
	return 0;
}

static int  
b_mpeg2pes_player_seek(void *player, bmedia_player_pos pos)
{
	return bmpeg2pes_player_seek(player, pos);
}

const bmedia_player_methods bmpeg2pes_player_methods = {
	b_mpeg2pes_player_create,
	b_mpeg2pes_player_destroy,
	b_mpeg2pes_player_next,
	b_mpeg2pes_player_tell,
	b_mpeg2pes_player_get_status,
	b_mpeg2pes_player_set_direction,
	b_mpeg2pes_player_seek
};

